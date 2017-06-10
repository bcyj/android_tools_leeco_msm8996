/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#include "config_proc.h"
#include "mctl.h"
#include "mctl_pp.h"
#include "c2d_client_intf.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

/*==============================================================================
 * FUNCTION    - mctl_pp_gen_c2d_config_parm -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_pp_gen_c2d_config_parm(mctl_pp_t *poll_cb,
  mctl_pp_divert_src_t *src, mctl_pp_dest_t *dest)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  c2d_dimension_cfg_t input_cfg_parm, op_cfg_parm;
  module_ops_t *c2d_mod_ops = NULL;

  CDBG("%s: E\n", __func__);

  switch (dest->data.path) {
    case OUTPUT_TYPE_P:
    case OUTPUT_TYPE_V:
      input_cfg_parm.width = src->data.image_width;
      input_cfg_parm.height = src->data.image_height;
      input_cfg_parm.plane[0] = src->data.plane[0];
      input_cfg_parm.plane[1] = src->data.plane[1];
      input_cfg_parm.cam_fmt = src->data.format;

      op_cfg_parm.width = dest->data.image_width;
      op_cfg_parm.height = dest->data.image_height;
      op_cfg_parm.plane[0] = dest->data.plane[0];
      op_cfg_parm.plane[1] = dest->data.plane[1];
      op_cfg_parm.cam_fmt = dest->data.format;
      break;
    default:
      CDBG_ERROR("%s: Invalid Data Path %d\n", __func__, dest->data.path);
      return -1;
  }

  c2d_mod_ops = &dest->hw_ops;
  if (c2d_mod_ops->set_params == NULL) {
    CDBG_ERROR("%s: c2d ops not initialized\n", __func__);
    return -1;
  }

  rc = c2d_mod_ops->set_params(c2d_mod_ops->handle, C2D_SET_INPUT_CFG,
  (void *)&input_cfg_parm, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: c2d C2D_SET_INPUT_CFG failed", __func__);
    return -1;
  }

  rc = c2d_mod_ops->set_params(c2d_mod_ops->handle, C2D_SET_OUTPUT_CFG,
  (void *)&op_cfg_parm, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: c2d C2D_SET_OUTPUT_CFG failed", __func__);
    return -1;
  }

  CDBG("%s: X\n", __func__);
  return 0;
} /* mctl_pp_gen_c2d_config_parm */

/*==============================================================================
 * FUNCTION    - mctl_pp_dest_c2d_resend_cmd -
 *
 * DESCRIPTION:
 *============================================================================*/
void mctl_pp_dest_c2d_resend_cmd(void *p_poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node, int *del_node)
{
  int rc = 0;
  mctl_pp_t *poll_cb = (mctl_pp_t *)p_poll_cb;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  pp_frame_data_t c2d_frame;
  int local_fd;
  void *local_vaddr;
  uint32_t c2d_cmd_handle;
  mctl_pp_local_buf_info_t *src_buf_data, *dst_buf_data;
  module_ops_t *c2d_ops = NULL;
  c2d_process_mode_t c2d_process;
  struct msm_pp_crop final_crop;

  CDBG("%s: E\n", __func__);

  *del_node = 0;

  node->dest_frame = *dest->data.p_free_frame;
  dest->data.p_free_frame = NULL;
  node->dest_frame.frame.frame_id = node->src_frame.frame.frame_id;
  node->dest_frame.frame.timestamp = node->src_frame.frame.timestamp;
  node->state = MCTL_PP_FRAME_DOING;

  /* we have free frame. send to c2d */
  memset(&c2d_frame,  0,  sizeof(c2d_frame));
  c2d_frame.src_format =
    ((mctl_pp_divert_src_t *)dest->data.src_ptr)->data.format;
  c2d_frame.dest_format = dest->data.format;//c2d_frame.src_format;
  CDBG("%s: Src format: %d Dest format: %d\n", __func__,c2d_frame.src_format,
    c2d_frame.dest_format);

  c2d_frame.action_flag = dest->data.c2d_action_flag;
  c2d_frame.cookie = (void *)dest;

  if (dest->data.dis_enable) {
    if (node->src_frame.frame.frame_id != dest->data.dis_info.frame_id) {
      CDBG_HIGH("%s: DIS info frame_id=%d and Diverted frame_id=%d differ",
        __func__, dest->data.dis_info.frame_id,
        node->src_frame.frame.frame_id);
    }
    mctl_pp_merge_crop_dis_offset(&dest->data.crop_info.crop,
      &dest->data.dis_info, &final_crop);
  } else {
    final_crop = dest->data.crop_info.crop;
  }
  c2d_frame.crop = final_crop;

  c2d_frame.dest = &node->dest_frame;
  c2d_frame.frame_id = node->src_frame.frame.frame_id;
  c2d_frame.path = dest->data.path;
  c2d_frame.src = &node->src_frame;
  c2d_frame.dest->frame.path = dest->data.path;

  CDBG("%s: src->img_mode=%d buf_idx=%d", __func__, c2d_frame.src->image_mode,
    c2d_frame.src->frame.buf_idx);
  CDBG("%s: dest->img_mode=%d buf_idx=%d", __func__, c2d_frame.dest->image_mode,
    c2d_frame.dest->frame.buf_idx);

  if (ctrl->enableLowPowerMode) {
    src_buf_data = &(ctrl->video_ctrl.user_buf_info[c2d_frame.src->image_mode].
      local_frame_info[c2d_frame.src->frame.buf_idx]);
  } else {
    src_buf_data = &(ctrl->video_ctrl.mctl_buf_info[c2d_frame.src->image_mode].
      local_frame_info[c2d_frame.src->frame.buf_idx]);
  }
  dst_buf_data = &(ctrl->video_ctrl.user_buf_info[c2d_frame.dest->image_mode].
    local_frame_info[c2d_frame.dest->frame.buf_idx]);

  CDBG("%s: src num_planes = %d", __func__, c2d_frame.src->frame.num_planes);
  if (c2d_frame.src->frame.num_planes == 1) {
    c2d_frame.src->frame.mp[0].fd = src_buf_data->fd;
    c2d_frame.src->frame.mp[0].vaddr = (uint32_t)src_buf_data->local_vaddr;
  } else if (c2d_frame.src->frame.num_planes > 1) {
    CDBG("%s: Src Plane0: fd=%d, vaddr=0x%x Plane1: fd=%d, vaddr=0x%x\n",
      __func__, src_buf_data->fd, (uint32_t)src_buf_data->local_vaddr +
      c2d_frame.src->frame.mp[0].addr_offset, src_buf_data->fd,
      (uint32_t)src_buf_data->local_vaddr +
      c2d_frame.src->frame.mp[1].addr_offset);

    c2d_frame.src->frame.mp[0].fd = src_buf_data->fd;
    c2d_frame.src->frame.mp[1].fd = src_buf_data->fd;
    c2d_frame.src->frame.mp[0].vaddr = (uint32_t)src_buf_data->local_vaddr +
      c2d_frame.src->frame.mp[0].addr_offset;
    c2d_frame.src->frame.mp[1].vaddr = (uint32_t)src_buf_data->local_vaddr +
      c2d_frame.src->frame.mp[1].addr_offset;
  } else {
    CDBG_ERROR("%s: Src Buf num_planes is invalid", __func__);
    goto error;
  }

  CDBG("%s: dst num_planes = %d", __func__, c2d_frame.dest->frame.num_planes);
  if (c2d_frame.dest->frame.num_planes == 1) {
    c2d_frame.dest->frame.mp[0].fd = dst_buf_data->fd;
    c2d_frame.dest->frame.mp[0].vaddr = (uint32_t)dst_buf_data->local_vaddr;
  } else if (c2d_frame.dest->frame.num_planes > 1) {
    CDBG("%s: Dst Plane0: fd=%d, vaddr=0x%x Plane1: fd=%d, vaddr=0x%x\n",
      __func__, dst_buf_data->fd, (uint32_t)dst_buf_data->local_vaddr +
      c2d_frame.dest->frame.mp[0].addr_offset, dst_buf_data->fd,
      (uint32_t)dst_buf_data->local_vaddr +
      c2d_frame.dest->frame.mp[1].addr_offset);

    c2d_frame.dest->frame.mp[0].fd = dst_buf_data->fd;
    c2d_frame.dest->frame.mp[1].fd = dst_buf_data->fd;
    c2d_frame.dest->frame.mp[0].vaddr = (uint32_t)dst_buf_data->local_vaddr +
      c2d_frame.dest->frame.mp[0].addr_offset;
    c2d_frame.dest->frame.mp[1].vaddr = (uint32_t)dst_buf_data->local_vaddr +
      c2d_frame.dest->frame.mp[1].addr_offset;
  } else {
    CDBG_ERROR("%s: Dst Buf num_planes is invalid", __func__);
    goto error;
  }

  if (dest->data.frame_cnt > 1)
    CDBG_HIGH("%s Mutliple frames received", __func__);

  c2d_ops = &dest->hw_ops;

  /* ToDo: During S3D, this should be C2D_IMAGE_GEOCORRECT */
  c2d_process = C2D_IMAGE_DRAW;

  rc = c2d_ops->set_params(c2d_ops->handle, C2D_SET_PROCESS_MODE, &c2d_process,
    &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_SET_PROCESS_MODE failed\n", __func__);
    goto error;
  }

  CDBG("%s: c2d_cmd_handle = 0x%x", __func__, c2d_cmd_handle);

  /* Override set_param's param_out to send second param */
  rc = c2d_ops->set_params(c2d_ops->handle, C2D_SET_ROTATION_CFG,
    (void *)&dest->data.rotation, &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_SET_ROTATION_CFG failed\n", __func__);
    goto error;
  }

  /* Override set_param's param_out to send second param */
  rc = c2d_ops->set_params(c2d_ops->handle, C2D_SET_INPUT_BUF_CFG,
    (void *)&c2d_frame.src->frame, &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_SET_INPUT_BUF_CFG failed\n", __func__);
    goto error;
  }

  /* Override set_param's param_out to send second param */
  rc = c2d_ops->set_params(c2d_ops->handle, C2D_SET_OUTPUT_BUF_CFG,
    (void *)&c2d_frame.dest->frame, &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_SET_OUTPUT_BUF_CFG failed\n", __func__);
    goto error;
  }

  /* Override set_param's param_out to send second param */
  rc = c2d_ops->set_params(c2d_ops->handle, C2D_SET_CROP_CFG,
    (void *)&c2d_frame.crop, &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_SET_CROP_CFG failed\n", __func__);
    goto error;
  }

  /* Override set_param's param_out to send second param */
  rc = c2d_ops->set_params(c2d_ops->handle, C2D_SET_CB_DATA, (void *)&c2d_frame,
    &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_SET_CB_DATA failed\n", __func__);
    goto error;
  }

  /* If LowPowerMode and no Color Conversion, no cropping and no rotation
   * then send this frame back to app directly.
   */
  CDBG("%s: srcFormat=%d destFormat=%d Rotation=%d", __func__,
    poll_cb->data.pp_ctrl.src->data.format, dest->data.format,
    dest->data.rotation);
  CDBG("%s: srcWxH=%dx%d srcXxY=%dx%d dstWxH=%dx%d dstXxY=%dx%d", __func__,
    c2d_frame.crop.src_w, c2d_frame.crop.src_h, c2d_frame.crop.src_x,
    c2d_frame.crop.src_y, c2d_frame.crop.dst_w, c2d_frame.crop.dst_h,
    c2d_frame.crop.dst_x, c2d_frame.crop.dst_y);

  if (ctrl->enableLowPowerMode) {
    if ((c2d_frame.crop.src_w == c2d_frame.crop.dst_w) &&
      (c2d_frame.crop.src_h == c2d_frame.crop.dst_h) &&
      (c2d_frame.crop.src_x == 0) && (c2d_frame.crop.src_y == 0) &&
      (poll_cb->data.pp_ctrl.src->data.format == dest->data.format) &&
      (dest->data.rotation == ROT_NONE)) {
      CDBG_HIGH("%s: Skip C2D pass & send the frame directly to App", __func__);
      goto skip_pp;
    }
  }

  rc = c2d_ops->process(c2d_ops->handle, C2D_EVENT_DO_PP, &c2d_cmd_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: C2D_EVENT_DO_PP failed\n", __func__);
    goto error;
  }

  CDBG("%s: X\n", __func__);
  return;

skip_pp:
  /* no crop & dis needed. send src frame back
   * to app and keep orig free frame
   */
  c2d_frame.not_pp_flag = 1;
  CDBG("%s not_pp_flag set \n", __func__);
  dest->src_cb_ops->pp_no_op(dest->data.src_ptr, c2d_frame.not_pp_flag,
    c2d_frame.frame_id, dest->my_idx);
  dest->data.free_frame = node->dest_frame;
  dest->data.p_free_frame = &dest->data.free_frame;
  *del_node = 1;

  CDBG("%s: X\n", __func__);
  return;

error:
  dest->data.frame_cnt--;
  dest->src_cb_ops->pp_error(dest->data.src_ptr,
  c2d_frame.frame_id,  dest->my_idx);
  CDBG_ERROR("%s: c2d error = %d",  __func__,  rc);
  dest->data.free_frame = node->dest_frame;
  dest->data.p_free_frame = &dest->data.free_frame;
  *del_node = 1;
  CDBG("%s: X\n", __func__);
} /* mctl_pp_dest_c2d_resend_cmd */

/*==============================================================================
 * FUNCTION    - mctl_pp_dest_c2d_ack_notify -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_pp_dest_c2d_ack_notify(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  struct timespec cur_time;

  CDBG("%s: ack c2d frame_id = %d, timestamp = 0x%x:0x%x", __func__,
    node->dest_frame.frame.frame_id,
    (uint32_t)node->dest_frame.frame.timestamp.tv_sec,
    (uint32_t)node->dest_frame.frame.timestamp.tv_usec);

  if (!(dest->data.c2d_action_flag & MSM_MCTL_PP_VPE_FRAME_TO_APP)) {
    CDBG("%s: Send Frame to App Dest idx %d", __func__, dest->my_idx);
    rc = mctl_pp_divert_done((void *)poll_cb, &node->dest_frame);
    CDBG("%s: -----------DestIdx %d Divert End------------",
              __func__, dest->my_idx);
    clock_gettime(CLOCK_REALTIME, &cur_time);
    CDBG("%s: -- C2D End -- frame id %d %ld %ld", __func__,
                 node->dest_frame.frame.frame_id, cur_time.tv_sec,
                 (cur_time.tv_nsec/1000));
  }

  dest->src_cb_ops->ack(dest->data.src_ptr, node->dest_frame.frame.frame_id,
    dest->my_idx);
  return rc;
} /* mctl_pp_dest_c2d_ack_notify */

/*==============================================================================
 * FUNCTION    - mctl_pp_release_c2d -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_pp_release_c2d(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest)
{
  int rc = 0;
  mctl_pp_divert_src_t *src;
  module_ops_t *c2d_mod_ops = NULL;

  CDBG("%s: E\n", __func__);

  c2d_mod_ops = &dest->hw_ops;

  if( !c2d_mod_ops || c2d_mod_ops->handle <= 0) {
    CDBG("%s: C2D interface was not opened", __func__);
    return 0;
  }
  if(c2d_mod_ops == NULL || c2d_mod_ops->destroy == NULL) {
    CDBG_ERROR("%s: c2d_mod_ops is NULL", __func__);
    return -1;
  }
  if (c2d_mod_ops->destroy(c2d_mod_ops->handle) < 0) {
    CDBG_ERROR("%s: c2d_destroy failed", __func__);
    return -1;
  }

  if(dest->data.pipe_fds[0])
    close(dest->data.pipe_fds[0]);
  if(dest->data.pipe_fds[1])
    close(dest->data.pipe_fds[1]);

  dest->data.pipe_fds[0] = 0;
  dest->data.pipe_fds[1] = 0;

  src = dest->data.src_ptr;
  /* Notify MCTL PP to stop polling for ACK on this hw.*/
  rc = mctl_pp_remove_poll_fd(poll_cb, src->my_idx, dest->my_idx);

  CDBG("%s: X\n", __func__);
  return rc;
} /* mctl_pp_release_c2d */

/*==============================================================================
 * FUNCTION    - mctl_pp_acquire_c2d -
 *
 * DESCRIPTION:
 *============================================================================*/
uint32_t mctl_pp_acquire_c2d(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest)
{
  int rc = 0, fds[2] = {0,0};
  mctl_ops_t mctl_ops;
  module_ops_t *c2d_mod_ops = NULL;
  mctl_pp_divert_src_t *src;
  struct pollfd poll_fd;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;

  CDBG("%s: E\n", __func__);

  c2d_mod_ops = &dest->hw_ops;
  if (c2d_mod_ops->handle) {
    CDBG_HIGH("%s: C2d already acquired, handle = %d", __func__,
      c2d_mod_ops->handle);
    return (uint32_t)NULL;
  }

  memset(&mctl_ops, 0, sizeof(mctl_ops_t));

  mctl_ops.fd = ctrl->camfd;
  mctl_ops.parent = (void *)poll_cb;
  mctl_ops.buf_done = mctl_pp_dest_done_notify;

  if(!c2d_interface_create(c2d_mod_ops)) {
    CDBG_HIGH("%s: cannot create C2D",  __func__);
    return (uint32_t)NULL;
  }

  rc = pipe(fds);
  if (rc < 0) {
    CDBG("%s: pipe open err=%d", __func__, rc);
    goto error;
  }

  dest->data.pipe_fds[0] = fds[0];
  dest->data.pipe_fds[1] = fds[1];

  CDBG("%s: Sending FD[0] = %d FD[1] = %d", __func__, fds[0], fds[1]);

  if (c2d_mod_ops->init(c2d_mod_ops->handle, &mctl_ops, fds) < 0) {
    CDBG_ERROR("%s: c2d_init failed", __func__);
    goto error;
  }

  src = dest->data.src_ptr;
  poll_fd.fd = fds[0];
  poll_fd.events = POLLIN|POLLRDNORM;
  /* Notify MCTL PP to start polling for ACK on this hw.*/
  rc = mctl_pp_add_poll_fd(poll_cb, &poll_fd, src->my_idx, dest->my_idx);
  if (rc < 0) {
    CDBG_ERROR("%s Couldnt add poll fd", __func__);
    goto error;
  }

  CDBG("%s: X\n", __func__);
  return c2d_mod_ops->handle;

error:
  if (fds[0]) {
    close(fds[0]);
    dest->data.pipe_fds[0] = fds[0] = 0;
  }

  if (fds[1]) {
    close(fds[1]);
    dest->data.pipe_fds[1] = fds[1] = 0;
  }

  if (c2d_mod_ops->destroy(c2d_mod_ops->handle) < 0)
    CDBG_HIGH("%s: c2d_destroy failed", __func__);
  return (uint32_t)NULL;
} /* mctl_pp_acquire_c2d */
