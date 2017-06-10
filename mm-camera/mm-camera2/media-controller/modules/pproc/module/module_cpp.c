/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <linux/media.h>
#include "camera_dbg.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "cam_intf.h"
#include "pproc_caps.h"
#include "pproc_interface.h"
#include "module_pproc.h"
#include "module_cpp.h"
#include "port_cpp.h"
#include "mct_stream.h"
#include "module_pproc_common.h"
#include "port_pproc_common.h"
#include "pproc_common_buff_mgr.h"

#define STR_SIZE_OF_SRC 3
#define STR_SIZE_OF_SINK 4

//#define CONFIG_PPROC_DBG
#ifdef CONFIG_PPROC_DBG
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

#undef CPP_DUMP_FRAME
//#define CPP_DUMP_FRAME

static uint32_t gcpp_count = 0;

boolean module_cpp_fill_input_params(mct_module_t *module,
  pproc_frame_input_params_t *frame_params, void *buffer,
  module_pproc_common_frame_params_t *port_frame_params,
  uint32_t event_identity, mct_port_t *port)
{
  isp_buf_divert_t                        *isp_buff =
    (isp_buf_divert_t *)buffer;
  module_pproc_common_ctrl_t              *mod_priv =
    (module_pproc_common_ctrl_t *)module->module_private;
  module_pproc_common_session_params_t    *session_params = NULL;
  uint32_t                                 x = 0, y = 0, j, k, offset;
  mct_bus_msg_stream_crop_t               *stream_crop = NULL;
  is_update_t                             *is_crop = NULL;
  mct_list_t                              *p_list;
  uint32_t                                 sessionid =
    (event_identity >> 16) & 0xFFFF;
  module_pproc_common_port_private_t      *port_private;
  module_pproc_common_frame_params_t      *buff_port_frame_params;
  module_pproc_common_frame_params_t      *event_port_frame_params;
  module_pproc_chromatix_denoise_params_t *chrmtx_denoise_params;

  if (!module || !frame_params || !buffer) {
    CDBG_ERROR("%s:%d invalid pamrams module %p frame params %p buffer %p\n",
      __func__, __LINE__, module, frame_params, buffer);
    return FALSE;
  }

  port_private = (module_pproc_common_port_private_t *)port->port_private;
  p_list = mct_list_find_custom(port_private->frame_params,
    &isp_buff->identity, module_pproc_common_find_identity);
  if (!p_list) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  buff_port_frame_params =
    (module_pproc_common_frame_params_t *)p_list->data;
  if (!buff_port_frame_params) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  p_list = mct_list_find_custom(port_private->frame_params,
    &event_identity, module_pproc_common_find_identity);
  if (!p_list) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  event_port_frame_params =
    (module_pproc_common_frame_params_t *)p_list->data;
  if (!event_port_frame_params) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  p_list = mct_list_find_custom(mod_priv->session_params, &sessionid,
    module_pproc_common_match_mod_params_by_session);
  if (!p_list) {
    CDBG_ERROR("%s:%d] Error in finding feature params\n", __func__,
      __LINE__);
    return FALSE;
  }
  session_params = (module_pproc_common_session_params_t *)p_list->data;

  if (!port_frame_params->src_width || !port_frame_params->src_height ||
    (port_frame_params->stream_crop.crop_out_x == 0) ||
    (port_frame_params->is_crop.width == 0) ||
    (port_frame_params->stream_crop.crop_out_y == 0) ||
    (port_frame_params->is_crop.height == 0)) {
    ALOGE("%s:%d] ERROR!!! iden: %d crop_out_x:%d, is_crop.width:%d, crop_out_y:%d is_crop.height:%d src_width:%d src_height:%d",
      __func__, __LINE__, event_identity, port_frame_params->stream_crop.crop_out_x,
      port_frame_params->is_crop.width, port_frame_params->stream_crop.crop_out_y,
      port_frame_params->is_crop.height, port_frame_params->src_width,
      port_frame_params->src_height);
    port_frame_params->src_width = port_frame_params->dst_width;
    port_frame_params->src_height = port_frame_params->dst_height;
    port_frame_params->src_stride = port_frame_params->dst_stride;
    port_frame_params->src_scanline = port_frame_params->dst_scanline;
    return FALSE;
  }

  frame_params->src_width = port_frame_params->src_width;
  frame_params->src_height = port_frame_params->src_height;
  frame_params->src_stride = port_frame_params->src_stride;
  frame_params->src_scanline = port_frame_params->src_scanline;
  frame_params->dst_stride = port_frame_params->dst_stride;
  frame_params->dst_scanline = port_frame_params->dst_scanline;

  stream_crop = &port_frame_params->stream_crop;
  is_crop = &port_frame_params->is_crop;
  CDBG("%s:%d isp crop %d %d %d %d\n", __func__, __LINE__,
    stream_crop->x, stream_crop->y, stream_crop->crop_out_x,
    stream_crop->crop_out_y);
  CDBG("%s:%d dis crop %d %d %d %d\n", __func__, __LINE__,
    is_crop->x, is_crop->y, is_crop->width, is_crop->height);
  CDBG("%s:%d src %d %d st sc %d %d dst %d %d\n", __func__, __LINE__,
    port_frame_params->src_width, port_frame_params->src_height,
    port_frame_params->src_stride, port_frame_params->src_scanline,
    port_frame_params->dst_stride, port_frame_params->dst_scanline);
  x = (stream_crop->x * port_frame_params->dst_width) /
    port_frame_params->src_width;
  CDBG("%s:%d %d = (%d * %d) / %d\n", __func__, __LINE__, x, stream_crop->x,
    port_frame_params->dst_width, port_frame_params->src_width);
  frame_params->process_window_first_pixel = x + port_frame_params->is_crop.x;
  CDBG("%s:%d %d = %d + %d\n", __func__, __LINE__,
    frame_params->process_window_first_pixel, x, port_frame_params->is_crop.x);
  y = (stream_crop->y * port_frame_params->dst_height) /
    port_frame_params->src_height;
  CDBG("%s:%d %d = (%d * %d) / %d\n", __func__, __LINE__, y, stream_crop->y,
    port_frame_params->dst_height, port_frame_params->src_height);
  frame_params->process_window_first_line = y + port_frame_params->is_crop.y;
  CDBG("%s:%d %d = %d + %d\n", __func__, __LINE__,
    frame_params->process_window_first_line, y, port_frame_params->is_crop.y);
  frame_params->process_window_width =
    (stream_crop->crop_out_x * is_crop->width) / port_frame_params->src_width;
  CDBG("%s:%d %d = (%d * %d) / %d\n", __func__, __LINE__,
    frame_params->process_window_width, stream_crop->crop_out_x,
    is_crop->width, port_frame_params->src_width);
  frame_params->process_window_height =
    (stream_crop->crop_out_y * is_crop->height) / port_frame_params->src_height;
  CDBG("%s:%d %d = (%d * %d) / %d\n", __func__, __LINE__,
    frame_params->process_window_height, stream_crop->crop_out_y,
    is_crop->height, port_frame_params->src_height);
  frame_params->rotation = 0;
  frame_params->mirror = port_frame_params->flip;
  frame_params->h_scale_ratio =
    (double)port_frame_params->dst_width / frame_params->process_window_width;
  frame_params->v_scale_ratio =
    (double)port_frame_params->dst_height / frame_params->process_window_height;

  chrmtx_denoise_params = &event_port_frame_params->chrmatix_denoise_params;
  for (k = 0; k < PPROC_WDN_FILTER_LEVEL; k++) {
    for (j = 0; j < PPROC_MAX_PLANES; j++) {
      frame_params->noise_profile[j][k] =
        chrmtx_denoise_params->noise_profile[j][k];
      frame_params->weight[j][k] = chrmtx_denoise_params->weight[j][k];
      frame_params->denoise_ratio[j][k] =
        chrmtx_denoise_params->denoise_ratio[j][k];
      frame_params->edge_softness[j][k] =
        chrmtx_denoise_params->edge_softness[j][k];
    }
  }

  frame_params->asf_mode = session_params->asf_mode;
  frame_params->sharpness = session_params->sharpness;
  frame_params->asf_info = port_private->asf_info;
  frame_params->denoise_enable =
    session_params->denoise_params.denoise_enable;
  frame_params->in_plane_fmt = buff_port_frame_params->plane_fmt;
  frame_params->out_plane_fmt = port_frame_params->plane_fmt;

  if (isp_buff->native_buf) {
    frame_params->in_frame_fd = isp_buff->fd;
  } else {
    frame_params->in_frame_fd = isp_buff->buffer.m.planes[0].m.userptr;
  }
  CDBG("%s:%d out x %d y %d w %d h %d hsc %f vsc %f\n", __func__, __LINE__,
    frame_params->process_window_first_pixel,
    frame_params->process_window_first_line,
    frame_params->process_window_width,
    frame_params->process_window_height,
    frame_params->h_scale_ratio,
    frame_params->v_scale_ratio);
  return TRUE;
}

#ifdef CPP_DUMP_FRAME
static boolean module_cpp_dump_frame(void *vaddr, uint32_t size, uint32_t ext)
{
  int out_file_fd;
  char out_fname[32];
  CDBG("size %d", size);
  snprintf(out_fname, sizeof(out_fname), "%s_%d.yuv", "/data/cpp_output", ext);
  out_file_fd = open(out_fname, O_RDWR | O_CREAT, 0777);
  if (out_file_fd < 0) {
    ALOGE("Cannot open file\n");
  }
  write(out_file_fd, vaddr, size);
  close(out_file_fd);
  return 0;
}
#endif

int32_t module_cpp_create_frame(void *data,
  pproc_interface_frame_divert_t *divert_frame)
{
  mct_module_t *module = (mct_module_t *)data;
  module_pproc_common_ctrl_t *pproc_ctrl =
    (module_pproc_common_ctrl_t *)module->module_private;
  pproc_buff_mgr_frmbuffer_t *frmbuffer = NULL;
  pproc_frame_input_params_t *frame_params = &divert_frame->frame_params;

  CDBG("%s: Enter\n", __func__);
  /* Get a VB2 buffer */
  if (pproc_common_buff_mgr_get_buffer(pproc_ctrl->buff_mgr_client,
    divert_frame->mct_event_identity, &frmbuffer) == FALSE) {
    CDBG_ERROR("%s: Get buffer failed.\n", __func__);
    return FALSE;
  }

  divert_frame->out_buff_idx = frmbuffer->buf_index;
  divert_frame->buf = frmbuffer->buf_planes[0].buf;

  CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%x, eveniden:%x\n",
    __func__, divert_frame->frame_params.frame_id,
    divert_frame->isp_divert_buffer.v4l2_buffer_obj.index, divert_frame->out_buff_idx,
    divert_frame->isp_divert_buffer.identity, divert_frame->mct_event_identity);

  frame_params->out_frame_fd = frmbuffer->buf_planes[0].fd;
  CDBG("%s:Exit\n", __func__);

  return TRUE;
}

int32_t module_cpp_frame_done(uint32_t skip, void *params)
{
  mct_event_t mct_event;
  mct_port_t *mct_port;
  mct_module_t               *module = NULL;
  module_pproc_common_ctrl_t *pproc_ctrl = NULL;
  struct cpp_library_params_t *cpp_lib_ctrl = NULL;
  isp_buf_divert_ack_t buff_divert_ack;
  pproc_interface_callback_params_t *callback_params =
    (pproc_interface_callback_params_t *)params;
  uint32_t frameid = callback_params->divert_frame.frame_params.frame_id;
  uint32_t in_buff_idx =
    callback_params->divert_frame.isp_divert_buffer.v4l2_buffer_obj.index;
  uint32_t out_buff_idx = callback_params->divert_frame.out_buff_idx;
  uint32_t buffer_skip = skip & 0x1;
  uint32_t framedone_skip = skip & 0x02;
  mct_list_t *p_list = NULL;
  module_pproc_common_port_private_t *port_private = NULL;
  module_pproc_common_frame_params_t *frame_params = NULL;

  module = (mct_module_t *)callback_params->module;
  /* Send buffer divert ack event */
  CDBG("%s:Receive buffer divert Ack\n", __func__);
  memset(&mct_event,  0,  sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
  mct_event.u.module_event.module_event_data = (void *)&buff_divert_ack;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = callback_params->divert_frame.mct_event_identity;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  memset(&buff_divert_ack,  0,  sizeof(buff_divert_ack));
  buff_divert_ack.buf_idx = in_buff_idx;
  buff_divert_ack.is_buf_dirty = 0;
  /* Attach buffer identity back to acknowledge event */
  buff_divert_ack.identity =
    callback_params->divert_frame.isp_divert_buffer.identity;

  if (!framedone_skip) {
    buff_divert_ack.is_buf_dirty = 1;
    pproc_ctrl = (module_pproc_common_ctrl_t *)module->module_private;
    cpp_lib_ctrl = pproc_ctrl->pproc_iface->lib_params->lib_private_data;
    pproc_ctrl->pproc_iface->lib_params->func_tbl->process
      (pproc_ctrl->pproc_iface->lib_params->lib_private_data,
        PPROC_IFACE_FRAME_DONE, (void *)frameid);
  }

  mct_port = module_pproc_common_find_port_using_identity(module,
    &mct_event.identity);
  if (!mct_port) {
    CDBG("%s:Error getting sink port\n", __func__);
    return FALSE;
  }

  CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d\n",
    __func__, callback_params->divert_frame.frame_params.frame_id,
    callback_params->divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
    callback_params->divert_frame.out_buff_idx,
    callback_params->divert_frame.isp_divert_buffer.identity,
    callback_params->divert_frame.mct_event_identity);

  port_private = (module_pproc_common_port_private_t *)mct_port->port_private;
  p_list = mct_list_find_custom(port_private->frame_params, &mct_event.identity,
    module_pproc_common_find_identity);
  frame_params = (module_pproc_common_frame_params_t *)p_list->data;
  if (frame_params->stream_info->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    mct_port_send_event_to_peer(mct_port, &mct_event);
  }
  if (!buffer_skip ||
      (frame_params->stream_info->stream_type ==
       CAM_STREAM_TYPE_OFFLINE_PROC)) {
    /* Need to give link identity and not buffer identity */
    //pproc_common_buff_mgr_put_buffer(pproc_ctrl->buff_mgr_client,
    //  callback_params->mct_event_identity, out_buff_idx);
    pproc_common_buff_mgr_buffer_done(pproc_ctrl->buff_mgr_client,
      mct_event.identity, out_buff_idx,
      callback_params->divert_frame.isp_divert_buffer.v4l2_buffer_obj.sequence,
      callback_params->divert_frame.isp_divert_buffer.v4l2_buffer_obj.timestamp);
  }

  if (frame_params->stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    uint32_t buf_size = (callback_params->divert_frame.frame_params.src_width *
      callback_params->divert_frame.frame_params.src_height * 3) / 2;
    CDBG_ERROR("%s:%d offline output ide %x buf id %d frame id %d\n", __func__,
      __LINE__, mct_event.identity, in_buff_idx, frameid);

#ifdef CPP_DUMP_FRAME
    module_cpp_dump_frame(callback_params->divert_frame.buf, 18051072,
      gcpp_count++);
#endif
  }
  CDBG("%s:Exit\n", __func__);
  return TRUE;
}

/** module_cpp_init:
 *    @name: module name("cpp")
 *
 *  This function creates mct_module_t for cpp module, creates
 *  port, fills capabilities and add them to the cpp module
 *
 *  This function executes in callers' context
 *
 *  Return:
 *  Success - mct_module_t pointer corresponding to cpp
 *  Failure - NULL in case of error / cpp library is not found
 **/
mct_module_t *module_cpp_init(const char *name)
{
  CDBG("%s:%d E\n", __func__, __LINE__);
  if (strcmp(name, "cpp")) {
    CDBG("%s:%d invalid name\n", __func__, __LINE__);
    return NULL;
  }

  return module_pproc_common_create_submod(name);
}

/** module_cpp_deinit:
 *    @module: pointer to cpp mct module
 *
 *  This function destroys mct_module_t for cpp module
 *
 *  This function executes in callers' context
 *
 *  Return: void
 **/
void module_cpp_deinit(mct_module_t *module)
{
  module_pproc_common_ctrl_t *mod_ctrl = NULL;
  CDBG("%s:%d E\n", __func__, __LINE__);
  if (!module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return;
  }

  mod_ctrl = (module_pproc_common_ctrl_t *)module->module_private;
  if (!mod_ctrl) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return;
  }

  mct_list_free_all(module->sinkports, module_pproc_destroy_port);
  mct_list_free_all(module->srcports, module_pproc_destroy_port);
  free(mod_ctrl->pproc_iface);
  free(mod_ctrl);
  mct_module_destroy(module);
  CDBG("%s:%d X\n", __func__, __LINE__);
  return;
}
