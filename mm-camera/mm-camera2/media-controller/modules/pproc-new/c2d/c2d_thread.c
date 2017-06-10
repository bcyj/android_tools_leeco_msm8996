/*============================================================================

  Copyright (c) 2013 - 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_thread.h"
#include "c2d_log.h"
#include "pp_buf_mgr.h"
#include <poll.h>
#include <unistd.h>
#include "c2d_hardware.h"
#include <sys/syscall.h>
#include <sys/prctl.h>

#define PIPE_FD_IDX   0
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
//#define SUBDEV_FD_IDX 1
extern c2d_hardware_stream_status_t*
  c2d_hardware_get_stream_status(c2d_hardware_t* c2dhw, uint32_t identity);
/** c2d_thread_func:
 *
 * Description:
 *   Entry point for c2d_thread. Polls over pipe read fd and c2d
 *   hw subdev fd. If there is any new pipe message or hardware
 *   event, it is processed.
 **/
static void* c2d_thread_func(void* data)
{
  int rc;
  mct_module_t *module = (mct_module_t *) data;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  PTHREAD_MUTEX_LOCK(&(ctrl->c2d_mutex));
  ctrl->c2d_thread_started = TRUE;
  pthread_cond_signal(&(ctrl->th_start_cond));
  PTHREAD_MUTEX_UNLOCK(&(ctrl->c2d_mutex));
  /* poll on the pipe readfd and subdev fd */
  struct pollfd pollfds;
  int num_fds = 1;
  int ready=0, i=0;
  prctl(PR_SET_NAME, "c2d_thread", 0, 0, 0);
  pollfds.fd = ctrl->pfd[READ_FD];
  pollfds.events = POLLIN|POLLPRI;
  CDBG_HIGH("%s:%d: c2d_thread entering the polling loop... thread_id is %d\n",
    __func__, __LINE__,syscall(SYS_gettid));
  while(1) {
    /* poll on the fds with no timeout */
    ready = poll(&pollfds, (nfds_t)num_fds, -1);
    if (ready > 0) {
      if (pollfds.revents & (POLLIN|POLLPRI)) {
        int num_read=0;
        c2d_thread_msg_t pipe_msg;
        num_read = read(pollfds.fd, &(pipe_msg),
                     sizeof(c2d_thread_msg_t));
        if(num_read < 0) {
          CDBG_ERROR("%s:%d, read() failed, rc=%d",
                      __func__, __LINE__, num_read);
          c2d_thread_fatal_exit(ctrl, TRUE);
        } else if(num_read != sizeof(c2d_thread_msg_t)) {
          CDBG_ERROR("%s:%d, failed, in read(), num_read=%d, msg_size=%d",
            __func__, __LINE__, num_read, sizeof(c2d_thread_msg_t));
          c2d_thread_fatal_exit(ctrl, FALSE);
        }
        rc = c2d_thread_process_pipe_message(ctrl, pipe_msg);
        if (rc < 0) {
          CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
          c2d_thread_fatal_exit(ctrl, FALSE);
        }
      } /* if */
    } else if(ready == 0){
      CDBG_ERROR("%s:%d, error: poll() timed out", __func__, __LINE__);
      c2d_thread_fatal_exit(ctrl, FALSE);
    } else {
      CDBG_ERROR("%s:%d, error: poll() failed", __func__, __LINE__);
      c2d_thread_fatal_exit(ctrl, FALSE);
    }
  } /* while(1) */
  return NULL;
}

/* c2d_thread_handle_divert_buf_event:
 *
 *   send a buf divert event to downstream module, if the piggy-backed ACK
 *   is received, we can update the ACK from ack_list, otherwise, the ACK will
 *   be updated when buf_divert_ack event comes from downstream module.
 *
 **/
static int32_t c2d_thread_handle_divert_buf_event(c2d_module_ctrl_t* ctrl,
  c2d_module_event_t* c2d_event)
{
  int rc;
  mct_event_t event;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = c2d_event->u.divert_buf_data.div_identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data =
    &(c2d_event->u.divert_buf_data.isp_buf_divert);

  c2d_event->u.divert_buf_data.isp_buf_divert.ack_flag = FALSE;

  CDBG("%s:%d, sending unproc_div, identity=0x%x", __func__, __LINE__,
    event.identity);
  rc = c2d_module_send_event_downstream(ctrl->p_module, &event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, unprocessed divert ack = %d", __func__, __LINE__,
    c2d_event->u.divert_buf_data.isp_buf_divert.ack_flag);

  /* if ack is piggy backed, we can safely send ack to upstream */
  if (c2d_event->u.divert_buf_data.isp_buf_divert.ack_flag == TRUE) {
    CDBG_LOW("%s:%d, doing ack for divert event", __func__, __LINE__);
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
  }
  return 0;
}

/* c2d_hardware_validate_params:
 *
 * Description:
 *
 *
 **/
static boolean c2d_hardware_validate_params(c2d_hardware_params_t *hw_params)
{

  CDBG_LOW("%s:%d, inw=%d, inh=%d, outw=%d, outh=%d", __func__, __LINE__,
    hw_params->input_info.width, hw_params->input_info.height,
    hw_params->output_info.width, hw_params->output_info.height);
  CDBG_LOW("%s:%d, inst=%d, insc=%d, outst=%d, outsc=%d", __func__, __LINE__,
    hw_params->input_info.stride, hw_params->input_info.scanline,
    hw_params->output_info.stride, hw_params->output_info.scanline);

  if (hw_params->input_info.width <= 0 || hw_params->input_info.height <= 0) {
    CDBG_ERROR("%s:%d, invalid input dim", __func__, __LINE__);
    return FALSE;
  }
  /* TODO: add mode sanity checks */
  return TRUE;
}

/* c2d_thread_handle_process_buf_event:
 *
 * Description:
 *
 *
 **/
static int32_t c2d_thread_handle_process_buf_event(c2d_module_ctrl_t* ctrl,
  c2d_module_event_t* c2d_event)
{
  int rc;
  boolean bool_ret = TRUE;
  int in_frame_fd;
  mct_event_t event;
  c2d_hardware_cmd_t cmd;
  mct_stream_map_buf_t *output_buf;
  c2d_frame c2d_input_buffer, c2d_output_buffer;
  c2d_process_frame_buffer c2d_process_buffer;
  uint32 x,y;

  if(!ctrl || !c2d_event) {
    CDBG_ERROR("%s:%d, failed, ctrl=%p, c2d_event=%p", __func__, __LINE__,
      ctrl, c2d_event);
    return -EINVAL;
  }
  /* if C2D processing is not needed, just ack input buffer*/
  if (c2d_event->u.process_buf_data.isp_buf_divert.is_skip_pproc) {
    CDBG("%s, C2D processing not required for frame:%d,Iden:0x%x,stream type:%d",
      __func__,
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.sequence,
      c2d_event->u.process_buf_data.proc_identity,
      c2d_event->u.process_buf_data.stream_info->stream_type);
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return 0;
  }

  c2d_hardware_params_t* hw_params;
  hw_params = &(c2d_event->u.process_buf_data.hw_params);
  if (c2d_event->u.process_buf_data.isp_buf_divert.native_buf) {
    in_frame_fd = c2d_event->u.process_buf_data.isp_buf_divert.fd;
    /* Get virtual address from isp divert structure */
    hw_params->input_buffer_info.vaddr =
      (unsigned long)c2d_event->u.process_buf_data.isp_buf_divert.vaddr;
  } else {
    in_frame_fd =
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.m.planes[0].m.userptr;
    CDBG_ERROR("%s:%d input buf index %d", __func__, __LINE__,
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.index);
    /* Get virtual address for input buffer */
    bool_ret = pp_buf_mgr_get_vaddr(ctrl->buf_mgr,
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.index,
      c2d_event->u.process_buf_data.input_stream_info,
      &hw_params->input_buffer_info.vaddr);
    if (bool_ret == FALSE) {
      CDBG_ERROR("%s:%d failed: pp_buf_mgr_get_vaddr() for frame_id:%d"
      "identity:0x%x,stream type: %d\n",
      __func__, __LINE__,
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.sequence,
      c2d_event->u.process_buf_data.proc_identity,
      c2d_event->u.process_buf_data.stream_info->stream_type);
      c2d_module_do_ack(ctrl, c2d_event->ack_key);
      return -EINVAL;
    }
    CDBG_ERROR("%s:%d input vaddr %lx\n", __func__, __LINE__,
      hw_params->input_buffer_info.vaddr);
  }
  hw_params->frame_id =
    c2d_event->u.process_buf_data.isp_buf_divert.buffer.sequence;
  hw_params->timestamp =
    c2d_event->u.process_buf_data.isp_buf_divert.buffer.timestamp;
  hw_params->identity = c2d_event->u.process_buf_data.proc_identity;
  hw_params->input_buffer_info.fd = in_frame_fd;
  hw_params->input_buffer_info.index =
    c2d_event->u.process_buf_data.isp_buf_divert.buffer.index;
  hw_params->input_buffer_info.native_buff =
    c2d_event->u.process_buf_data.isp_buf_divert.native_buf;

  if (!c2d_event->u.process_buf_data.stream_info) {
    CDBG_ERROR("%s:%d failed: stream_info NULL\n", __func__, __LINE__);
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return -EINVAL;
  }

  if (!ctrl->c2d || !ctrl->c2d_ctrl) {
    CDBG_ERROR("%s:%d failed: c2d / c2d ctrl NULL\n", __func__, __LINE__);
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return -EINVAL;
  }

  /* Get output buffer from buffer manager */
  output_buf = pp_buf_mgr_get_buf(ctrl->buf_mgr,
    c2d_event->u.process_buf_data.stream_info);
  if (!output_buf) {
    CDBG_ERROR("%s:%d failed: pp_buf_mgr_get_buf() for frame_id:%d "
      "identity:0x%x,stream type: %d\n",
      __func__, __LINE__,hw_params->frame_id,hw_params->identity,
      c2d_event->u.process_buf_data.stream_info->stream_type);
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return 0;
  }

  /* before giving the frame to hw, make sure the parameters are good */
  if(FALSE == c2d_hardware_validate_params(hw_params)) {
    CDBG_ERROR("%s:%d, hw_params invalid, dropping frame.", __func__, __LINE__);
    return c2d_module_do_ack(ctrl, c2d_event->ack_key);
  }
  cmd.type = C2D_HW_CMD_PROCESS_FRAME;
  cmd.u.hw_params = hw_params;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  if (rc == -EAGAIN) {
     CDBG_ERROR("%s:%d] get buffer fail. drop frame id:%d identity:0x%x"
      " stream type:%d",
       __func__, __LINE__, hw_params->frame_id, hw_params->identity,
       c2d_event->u.process_buf_data.stream_info->stream_type);
     pp_buf_mgr_put_buf(ctrl->buf_mgr,
         c2d_event->u.process_buf_data.stream_info->identity,
         output_buf->buf_index, hw_params->frame_id,
         hw_params->timestamp);
     return c2d_module_do_ack(ctrl, c2d_event->ack_key);
  }

  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, hw_params->identity,
    &session_params, &stream_params);
  if (!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  CDBG("%s:%d frame_id:%d,identity:0x%x,stream type: %d output buf index %d,"
    "buf size %d\n",
    __func__, __LINE__,hw_params->frame_id,hw_params->identity,
    c2d_event->u.process_buf_data.stream_info->stream_type,
    output_buf->buf_index, output_buf->buf_size);

  /* Initialize input frame */
  memset(&c2d_input_buffer, 0, sizeof(c2d_input_buffer));
  if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_420_NV12;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_420_NV21;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_422_NV16;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_422_NV61;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_YCRYCB422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_YUYV;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_YCBYCR422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_YVYU;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRYCBY422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_UYVY;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBYCRY422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_VYUY;
  }
  if ((c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV61) ||
       (c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV16)) {
  /* Copy input buffer to output buffer */
  memcpy(output_buf->buf_planes[0].buf,
    (const void *)hw_params->input_buffer_info.vaddr,
    output_buf->buf_size);
  } else {
  c2d_input_buffer.width = hw_params->input_info.width;
  c2d_input_buffer.height = hw_params->input_info.height;
  c2d_input_buffer.stride = hw_params->input_info.stride;
  c2d_input_buffer.num_planes = 2;
  if (stream_params->stream_info->is_type == IS_TYPE_EIS_2_0) {
    uint32_t biger_dim;

    biger_dim = hw_params->output_info.width >
      stream_params->linked_stream_params->hw_params.output_info.width ?
      hw_params->output_info.width :
      stream_params->linked_stream_params->hw_params.output_info.width;
    c2d_input_buffer.x_border = c2d_input_buffer.width - biger_dim;

    biger_dim = hw_params->output_info.height >
      stream_params->linked_stream_params->hw_params.output_info.height ?
      hw_params->output_info.height :
      stream_params->linked_stream_params->hw_params.output_info.height;
    c2d_input_buffer.y_border = c2d_input_buffer.height - biger_dim;
  }
  /* Plane 0 */
  c2d_input_buffer.mp[0].vaddr = hw_params->input_buffer_info.vaddr;
  c2d_input_buffer.mp[0].length = hw_params->input_info.stride *
    hw_params->input_info.scanline;
  c2d_input_buffer.mp[0].fd = hw_params->input_buffer_info.fd;
  c2d_input_buffer.mp[0].addr_offset = 0;
  c2d_input_buffer.mp[0].data_offset = 0;
  if ((c2d_input_buffer.format >= CAM_FORMAT_YUV_RAW_8BIT_YUYV) &&
      (c2d_input_buffer.format <= CAM_FORMAT_YUV_RAW_8BIT_VYUY)) {
    c2d_input_buffer.num_planes = 1;
    c2d_input_buffer.mp[0].length *= 2;
  } else {
  /* Plane 1 */
    c2d_input_buffer.mp[1].vaddr = c2d_input_buffer.mp[0].vaddr +
        hw_params->input_info.stride * hw_params->input_info.scanline;
    c2d_input_buffer.mp[1].length = (hw_params->input_info.stride *
        hw_params->input_info.scanline) / 2;
    c2d_input_buffer.mp[1].fd = hw_params->input_buffer_info.fd;
    c2d_input_buffer.mp[1].addr_offset = 0;
    c2d_input_buffer.mp[1].data_offset = 0;

    if ((c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV61) ||
        (c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV16)) {
      c2d_input_buffer.mp[1].length = (hw_params->input_info.stride *
          hw_params->input_info.scanline);
    }
  }

  /* Configure c2d IS parameters */

  if (hw_params->crop_info.is_crop.use_3d && session_params->dis_enable ) {
    c2d_input_buffer.lens_correction_cfg.use_LC = TRUE;
    c2d_input_buffer.lens_correction_cfg.transform_mtx =
      &hw_params->crop_info.is_crop.transform_matrix[0];
    c2d_input_buffer.lens_correction_cfg.transform_type =
        hw_params->crop_info.is_crop.transform_type;
  } else {
    c2d_input_buffer.lens_correction_cfg.use_LC = FALSE;
  }

  /* Configure c2d for crop paramters required by the stream */


  if (((c2d_event->u.process_buf_data.input_stream_info &&
    c2d_event->u.process_buf_data.input_stream_info->is_type ==
    IS_TYPE_EIS_2_0) || stream_params->interleaved) &&
    !stream_params->single_module){

    c2d_input_buffer.roi_cfg.x = hw_params->crop_info.is_crop.x;
    c2d_input_buffer.roi_cfg.y = hw_params->crop_info.is_crop.y;
    if (!hw_params->crop_info.is_crop.dx && ! hw_params->crop_info.is_crop.dy){
      c2d_input_buffer.roi_cfg.width  = c2d_input_buffer.width;
      c2d_input_buffer.roi_cfg.height = c2d_input_buffer.height;
    }
    else {
      c2d_input_buffer.roi_cfg.width = hw_params->crop_info.is_crop.dx;
      c2d_input_buffer.roi_cfg.height = hw_params->crop_info.is_crop.dy;
    }
  } else {
    if (!hw_params->crop_info.stream_crop.dx ||
        ! hw_params->crop_info.stream_crop.dy){
      c2d_input_buffer.roi_cfg.width  =hw_params->input_info.width;
      c2d_input_buffer.roi_cfg.height = hw_params->input_info.height;
    }
    x = (hw_params->crop_info.stream_crop.x * hw_params->crop_info.is_crop.dx) /
        hw_params->input_info.width;
    y = (hw_params->crop_info.stream_crop.y * hw_params->crop_info.is_crop.dy) /
        hw_params->input_info.height;

    /* calculate the first pixel in window */
    c2d_input_buffer.roi_cfg.x =
        x + hw_params->crop_info.is_crop.x;
    /* calculate the first line in window */
    c2d_input_buffer.roi_cfg.y =
        y + hw_params->crop_info.is_crop.y;
    /* calculate the window width */
    c2d_input_buffer.roi_cfg.width =
        (hw_params->crop_info.stream_crop.dx * hw_params->crop_info.is_crop.dx)/
        hw_params->input_info.width;
    /* calculate the window height */
    c2d_input_buffer.roi_cfg.height =
        (hw_params->crop_info.stream_crop.dy * hw_params->crop_info.is_crop.dy)/
        hw_params->input_info.height;
  }
  /* Initialize output frame */

  if (c2d_input_buffer.roi_cfg.height &&
    hw_params->output_info.height && hw_params->output_info.width) {
      float dst_aspect_ratio = (float)hw_params->output_info.width /
        (float)hw_params->output_info.height;
      float src_aspect_ratio = (float)c2d_input_buffer.roi_cfg.width /
        (float)c2d_input_buffer.roi_cfg.height;
      uint32_t process_window_first_pixel, process_window_first_line;
      uint32_t process_window_width, process_window_height;

      if((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
        dst_aspect_ratio = (float)hw_params->output_info.height /
        (float)hw_params->output_info.width;
      }

    if (dst_aspect_ratio > src_aspect_ratio) {
      process_window_height =
        (float)c2d_input_buffer.roi_cfg.width / dst_aspect_ratio;
      process_window_width = c2d_input_buffer.roi_cfg.width;
      process_window_first_pixel =
        c2d_input_buffer.roi_cfg.x;
      process_window_first_line =
        c2d_input_buffer.roi_cfg.y +
        ((float)(c2d_input_buffer.roi_cfg.height -
        process_window_height)) / 2.0;
    } else {
      process_window_width =
        (float)c2d_input_buffer.roi_cfg.height * dst_aspect_ratio;
      process_window_height = c2d_input_buffer.roi_cfg.height;
      process_window_first_line =
        c2d_input_buffer.roi_cfg.y;
      process_window_first_pixel =
        c2d_input_buffer.roi_cfg.x +
        ((float)(c2d_input_buffer.roi_cfg.width-
        process_window_width)) / 2.0;
    }

    c2d_input_buffer.roi_cfg.width = process_window_width;
    c2d_input_buffer.roi_cfg.height = process_window_height;
    c2d_input_buffer.roi_cfg.x =
      process_window_first_pixel;
    c2d_input_buffer.roi_cfg.y =
      process_window_first_line;
  }
  CDBG("%s c2d_input_buffer.roi_cfg.x %d c2d_input_buffer.roi_cfg.y %d c2d_input_buffer.roi_cfg.width %d \
         c2d_input_buffer.roi_cfg.height %d",__func__, c2d_input_buffer.roi_cfg.x,c2d_input_buffer.roi_cfg.y,
         c2d_input_buffer.roi_cfg.width,c2d_input_buffer.roi_cfg.height);

  memset(&c2d_output_buffer, 0, sizeof(c2d_output_buffer));
  if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_420_NV12;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_420_NV21;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR422) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_422_NV16;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB422) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_422_NV61;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB420) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_420_YV12;
  }
  c2d_output_buffer.width = hw_params->output_info.width;
  c2d_output_buffer.height = hw_params->output_info.height;
  c2d_output_buffer.stride = hw_params->output_info.stride;

  if (c2d_output_buffer.format == CAM_FORMAT_YUV_420_YV12)
    c2d_output_buffer.num_planes = 3;
  else
    c2d_output_buffer.num_planes = 2;

  c2d_output_buffer.roi_cfg.x = 0;
  c2d_output_buffer.roi_cfg.y = 0;
  c2d_output_buffer.roi_cfg.width = hw_params->output_info.width;
  c2d_output_buffer.roi_cfg.height = hw_params->output_info.height;
  /* Plane 0 */
  c2d_output_buffer.mp[0].vaddr = (unsigned long)output_buf->buf_planes[0].buf;
  c2d_output_buffer.mp[0].length = hw_params->output_info.stride *
    hw_params->output_info.scanline;
  c2d_output_buffer.mp[0].fd = output_buf->buf_planes[0].fd;
  c2d_output_buffer.mp[0].addr_offset = 0;
  c2d_output_buffer.mp[0].data_offset = 0;

  if (c2d_output_buffer.format != CAM_FORMAT_YUV_420_YV12) {
    /* Plane 1 */
    c2d_output_buffer.mp[1].vaddr = c2d_output_buffer.mp[0].vaddr +
      hw_params->output_info.stride * hw_params->output_info.scanline;
    c2d_output_buffer.mp[1].length = (hw_params->output_info.stride *
	hw_params->output_info.scanline) / 2;
    c2d_output_buffer.mp[1].fd = output_buf->buf_planes[0].fd;
    c2d_output_buffer.mp[1].addr_offset = 0;
    c2d_output_buffer.mp[1].data_offset = 0;
  } else {
  /* Plane 1 */
    c2d_output_buffer.mp[1].vaddr = c2d_output_buffer.mp[0].vaddr +
      hw_params->output_info.stride * hw_params->output_info.scanline;
    c2d_output_buffer.mp[1].length = CEILING16(hw_params->output_info.stride /2) *
	hw_params->output_info.scanline / 2;
    c2d_output_buffer.mp[1].fd = output_buf->buf_planes[0].fd;
    c2d_output_buffer.mp[1].addr_offset = 0;
    c2d_output_buffer.mp[1].data_offset = 0;

  /* Plane 2 */
     c2d_output_buffer.mp[2].length = CEILING16(hw_params->output_info.stride /2) *
        hw_params->output_info.scanline / 2;
    c2d_output_buffer.mp[2].vaddr = c2d_output_buffer.mp[1].vaddr +
      c2d_output_buffer.mp[2].length ;
    c2d_output_buffer.mp[2].fd = output_buf->buf_planes[0].fd;
    c2d_output_buffer.mp[2].addr_offset = 0;
    c2d_output_buffer.mp[2].data_offset = 0;
  }

  if ((c2d_output_buffer.format == CAM_FORMAT_YUV_422_NV61) ||
       (c2d_output_buffer.format == CAM_FORMAT_YUV_422_NV16)) {
    c2d_output_buffer.mp[1].length = (hw_params->output_info.stride *
      hw_params->output_info.scanline);
  }

  c2d_process_buffer.c2d_input_buffer = &c2d_input_buffer;
  c2d_process_buffer.c2d_output_buffer = &c2d_output_buffer;
  /* Hardcode 180 rotation for testing */
  //c2d_process_buffer.rotation = (0x00000001 << 1);
  int32_t   swap_dim;

  if((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
    swap_dim = hw_params->output_info.width;
    hw_params->output_info.width = hw_params->output_info.height;
    hw_params->output_info.height = swap_dim;
  }

  switch(hw_params->rotation) {
  case 0:
    c2d_process_buffer.rotation = 0;
  break;
  case 1:
   c2d_process_buffer.rotation = (0x00000001 << 0);
   break;
  case 2:
   c2d_process_buffer.rotation = (0x00000001 << 1);
   break;
  case 3:
   c2d_process_buffer.rotation = (0x00000001 << 2);
   break;
  default:
  ALOGE("Not a valid Rotation");
 }

  c2d_process_buffer.flip = hw_params->mirror;
  c2d_process_buffer.c2d_input_lib_params = stream_params->c2d_input_lib_params;
  c2d_process_buffer.c2d_output_lib_params = stream_params->c2d_output_lib_params;

  /* Call process frame on c2d */
  rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl, PPROC_IFACE_PROCESS_FRAME,
    &c2d_process_buffer);
  if (rc < FALSE) {
    CDBG_ERROR("%s:%d failed: ctrl->c2d->func_tbl->process() for frame_id:%d"
      "iden:0x%x,stream_type:%d\n", __func__,
      __LINE__,
      hw_params->frame_id,
      hw_params->identity,
      c2d_event->u.process_buf_data.stream_info->stream_type);
  }
  }

  /*
   * Invalidate output buffer
   */
  if (session_params->ion_fd) {
    struct ion_flush_data cache_inv_data;
    struct ion_custom_data custom_data;
    struct ion_fd_data     fd_data;

    memset(&cache_inv_data, 0, sizeof(cache_inv_data));
    memset(&custom_data, 0, sizeof(custom_data));
    cache_inv_data.vaddr = output_buf->buf_planes[0].buf;
    cache_inv_data.fd = output_buf->buf_planes[0].fd;

    cache_inv_data.handle = NULL;
    cache_inv_data.length = output_buf->buf_size;
    custom_data.cmd = ION_IOC_CLEAN_INV_CACHES;
    custom_data.arg = (unsigned long)&cache_inv_data;

    rc = ioctl(session_params->ion_fd, ION_IOC_CUSTOM, &custom_data);
    if(rc < 0)
       CDBG_ERROR("%s - WARNING Cache invalidation on output buffer failed",__func__);
  }
  if (((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
    stream_params->interleaved) && !stream_params->single_module) ||
    hw_params->processed_divert) {

    if  (stream_params->is_stream_on) {
      mct_event_t event;
      isp_buf_divert_t  isp_buf;
      struct v4l2_plane plane;

      memset(&isp_buf, 0, sizeof(isp_buf));
      memset(&plane, 0, sizeof(plane));

      isp_buf.native_buf = 0;
      isp_buf.buffer.sequence =
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.sequence;
      isp_buf.buffer.index = output_buf->buf_index;
      isp_buf.buffer.length =
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.length;
      isp_buf.buffer.m.planes = &plane;
      isp_buf.buffer.m.planes[0].m.userptr = output_buf->buf_planes[0].fd;
      isp_buf.fd = output_buf->buf_planes[0].fd;
      isp_buf.vaddr = output_buf->buf_planes[0].buf;
      isp_buf.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      isp_buf.buffer.memory = V4L2_MEMORY_USERPTR;
      isp_buf.is_uv_subsampled =
        c2d_event->u.divert_buf_data.isp_buf_divert.is_uv_subsampled;

      isp_buf.buffer.timestamp =
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.timestamp;
      isp_buf.identity = hw_params->identity;

      event.direction = MCT_EVENT_DOWNSTREAM;
      event.identity = hw_params->identity;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
      event.u.module_event.module_event_data = &isp_buf;

      rc = c2d_module_send_event_downstream(ctrl->p_module, &event);
      if (rc < 0) {
        CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
          __func__, __LINE__, event.u.module_event.type, hw_params->identity);
        pp_buf_mgr_put_buf(ctrl->buf_mgr,hw_params->identity,output_buf->buf_index,
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.sequence,
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.timestamp);
        return -EFAULT;
      }
      if (isp_buf.ack_flag == 1) {
          pp_buf_mgr_put_buf(ctrl->buf_mgr,hw_params->identity,output_buf->buf_index,
            c2d_event->u.divert_buf_data.isp_buf_divert.buffer.sequence,
            c2d_event->u.divert_buf_data.isp_buf_divert.buffer.timestamp);
      }
    } else {
      pp_buf_mgr_put_buf(ctrl->buf_mgr,hw_params->identity,output_buf->buf_index,
        c2d_event->u.divert_buf_data.isp_buf_divert.buffer.sequence,
        c2d_event->u.divert_buf_data.isp_buf_divert.buffer.timestamp);
    }
  } else {
    /* Put buffer */
      bool_ret = pp_buf_mgr_buf_done(ctrl->buf_mgr,
      c2d_event->u.process_buf_data.stream_info->identity,
      output_buf->buf_index, hw_params->frame_id,
      hw_params->timestamp);
  }
    cmd.type = C2D_HW_CMD_RELEASE_FRAME;
    cmd.u.hw_params = hw_params;
    c2d_hardware_process_command(ctrl->c2dhw, cmd);
    c2d_module_do_ack(ctrl, c2d_event->ack_key);

  return rc;
}

/* c2d_thread_get_event_from_queue
 *
 * Description:
 * - dq event from the queue based on priority. if there is any event in
 *   realtime queue, return it. Only when there is nothing in realtime queue,
 *   get event from offline queue.
 * - Get hardware related event only if the hardware is ready to process.
 **/
static c2d_module_event_t* c2d_thread_get_event_from_queue(
  c2d_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return NULL;
  }
  c2d_module_event_t *c2d_event = NULL;
  /* TODO: see if this hardware related logic is suitable in this function
     or need to put it somewhere else */
  if (c2d_hardware_get_status(ctrl->c2dhw) == C2D_HW_STATUS_READY) {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                  mct_queue_pop_head(ctrl->realtime_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
      return c2d_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                  mct_queue_pop_head(ctrl->offline_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
      return c2d_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  } else {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                    mct_queue_look_at_head(ctrl->realtime_queue.q);
      if (NULL==c2d_event) {
        PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
        return c2d_event;
      }
      if (c2d_event->hw_process_flag == FALSE) {
        c2d_event = (c2d_module_event_t *)
                      mct_queue_pop_head(ctrl->realtime_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
        return c2d_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                    mct_queue_look_at_head(ctrl->offline_queue.q);
      if (NULL==c2d_event) {
        PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
        return c2d_event;
      }
      if (c2d_event->hw_process_flag == FALSE) {
        c2d_event = (c2d_module_event_t *)
                      mct_queue_pop_head(ctrl->offline_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
        return c2d_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  }
  return NULL;
}

/* c2d_thread_process_queue_event:
 *
 * Description:
 *
 **/
static int32_t c2d_thread_process_queue_event(c2d_module_ctrl_t *ctrl,
  c2d_module_event_t* c2d_event)
{
  int32_t rc = 0;
  if(!ctrl || !c2d_event) {
    CDBG_ERROR("%s:%d, failed, ctrl=%p, c2d_event=%p", __func__, __LINE__,
      ctrl, c2d_event);
    return -EINVAL;
  }
  /* if the event is invalid, no need to process, just free the memory */
  if(c2d_event->invalid == TRUE) {
    CDBG("%s:%d, invalidated event received.", __func__, __LINE__);
    free(c2d_event);
    return 0;
  }
  switch(c2d_event->type) {
  case C2D_MODULE_EVENT_DIVERT_BUF:
    CDBG_LOW("%s:%d, C2D_MODULE_EVENT_DIVERT_BUF", __func__, __LINE__);
    rc = c2d_thread_handle_divert_buf_event(ctrl, c2d_event);
    break;
  case C2D_MODULE_EVENT_PROCESS_BUF:
    CDBG_LOW("%s:%d, C2D_MODULE_EVENT_PROCESS_BUF", __func__, __LINE__);
    rc = c2d_thread_handle_process_buf_event(ctrl, c2d_event);
    break;
  default:
    CDBG_ERROR("%s:%d, failed, bad event type=%d", __func__, __LINE__,
      c2d_event->type);
    free(c2d_event);
    return -EINVAL;
  }
  /* free the event memory */
  free(c2d_event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, failed, rc=%d", __func__, __LINE__, rc);
  }
  return rc;
}

/* c2d_thread_process_pipe_message:
 *
 * Description:
 *
 **/
int32_t c2d_thread_process_pipe_message(c2d_module_ctrl_t *ctrl,
  c2d_thread_msg_t msg)
{
  int rc = 0;
  c2d_hardware_cmd_t cmd;
  switch(msg.type) {
  case C2D_THREAD_MSG_ABORT: {
    CDBG_HIGH("%s:%d, C2D_THREAD_MSG_ABORT: c2d_thread exiting..",
      __func__, __LINE__);
    ctrl->c2d_thread_started = FALSE;
#if 0
    cmd.type = C2D_HW_CMD_UNSUBSCRIBE_EVENT;
    c2d_hardware_process_command(ctrl->c2dhw, cmd);
#endif
    pthread_exit(NULL);
  }
  case C2D_THREAD_MSG_NEW_EVENT_IN_Q: {
    CDBG_LOW("%s:%d, C2D_THREAD_MSG_NEW_EVENT_IN_Q:", __func__, __LINE__);
    c2d_module_event_t* c2d_event;
    /* while there is some valid event in queue process it */
    while(1) {
      c2d_event = c2d_thread_get_event_from_queue(ctrl);
      if(!c2d_event) {
        break;
      }
      rc = c2d_thread_process_queue_event(ctrl, c2d_event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, c2d_thread_process_queue_event() failed",
          __func__, __LINE__);
      }
    }
    break;
  }
  default:
    CDBG_ERROR("%s:%d, error: bad msg type=%d",
      __func__, __LINE__, msg.type);
    return -EINVAL;
  }
  return rc;
}

/* c2d_thread_fatal_exit:
 *
 * Description:
 *
 **/
void c2d_thread_fatal_exit(c2d_module_ctrl_t *ctrl, boolean post_to_bus)
{
  CDBG_ERROR("%s:%d, fatal error: killing c2d_thread....!", __func__, __LINE__);
  if(post_to_bus) {
    CDBG_ERROR("%s:%d, posting error to MCT BUS!", __func__, __LINE__);
    /* TODO: add code to post error on mct bus */
  }
  ctrl->c2d_thread_started = FALSE;
  pthread_exit(NULL);
}

/* c2d_thread_create:
 *
 * Description:
 *
 **/
int32_t c2d_thread_create(mct_module_t *module)
{
  int rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(ctrl->c2d_thread_started == TRUE) {
    CDBG_ERROR("%s:%d, failed, thread already started, "
               "can't create the thread again!", __func__, __LINE__);
    return -EFAULT;
  }
  ctrl->c2d_thread_started = FALSE;
  rc = pthread_create(&(ctrl->c2d_thread), NULL, c2d_thread_func, module);
  pthread_setname_np(ctrl->c2d_thread, "CAM_c2d");
  if(rc < 0) {
    CDBG_ERROR("%s:%d, pthread_create() failed, rc= ", __func__, __LINE__);
    return rc;
  }
  /* wait to confirm if the thread is started */
  PTHREAD_MUTEX_LOCK(&(ctrl->c2d_mutex));
  while(ctrl->c2d_thread_started == FALSE) {
    pthread_cond_wait(&(ctrl->th_start_cond), &(ctrl->c2d_mutex));
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->c2d_mutex));
  return 0;
}
