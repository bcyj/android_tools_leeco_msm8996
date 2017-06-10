/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
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
#include "vpe_api.h"

#define BUFF_SIZE_128 128

#if 0
#undef CDBG
#define CDBG LOGE
#endif

int mctl_pp_gen_vpe_config_parm(mctl_pp_t *poll_cb, mctl_pp_divert_src_t *src,
  mctl_pp_dest_t *dest)
{
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mm_vpe_pipe_config_parm_type cfg_parm;
  CDBG("%s\n", __func__);

  switch (dest->data.path) {
    case OUTPUT_TYPE_P:
    case OUTPUT_TYPE_V:
      cfg_parm.input.width = src->data.image_width;
      cfg_parm.input.height = src->data.image_height;
      cfg_parm.input.stride0 = src->data.image_width;
      cfg_parm.input.stride1 = src->data.image_width;
      cfg_parm.input.fmt = src->data.format;

      cfg_parm.output.width = dest->data.image_width;
      cfg_parm.output.height = dest->data.image_height;
      cfg_parm.output.stride0 = dest->data.image_width;
      cfg_parm.output.stride1 = dest->data.image_width;
      cfg_parm.output.fmt = dest->data.format;
      break;
    default:
      CDBG_ERROR("%s: Invalid data path %d", __func__, dest->data.path);
      return -1;
  }
  cfg_parm.rot = dest->data.rotation;
  CDBG("%s: VPE cfg: src_w=%d,src_h=%d,src_stride0=%d,src_stride1=%d,src_fmt=%d,"
   "dest_w=%d,dest_h=%d,dest_stride0=%d,dest_stride1=%d,dest_fmt=%d,rot=%d",
    __func__, cfg_parm.input.width,
    cfg_parm.input.height, cfg_parm.input.stride0,
    cfg_parm.input.stride1, cfg_parm.input.fmt,
    cfg_parm.output.width, cfg_parm.output.height,
    cfg_parm.output.stride0,  cfg_parm.output.stride1,
    cfg_parm.output.fmt, cfg_parm.rot);
  return dest->hw_ops.set_params(dest->hw_ops.handle, VPE_PARM_PIPELINE_CFG,
    (void *)&cfg_parm, NULL);
}

int mctl_pp_dest_vpe_ack_notify(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  struct timespec cur_time;

  CDBG("%s: ack vpe frame_id = %d, timestamp = 0x%x:0x%x mode: %d\n",
    __func__, node->dest_frame.frame.frame_id,
    (uint32_t)node->dest_frame.frame.timestamp.tv_sec,
    (uint32_t)node->dest_frame.frame.timestamp.tv_usec,
    node->dest_frame.image_mode);
  if (!(dest->data.vpe_action_flag & MSM_MCTL_PP_VPE_FRAME_TO_APP)) {
    CDBG("%s: VPE Send frame to app\n", __func__);
    rc = mctl_pp_divert_done((void *)poll_cb, &node->dest_frame);
    CDBG("%s: -----------DestIdx %d Divert End------------",
      __func__, dest->my_idx);
    clock_gettime(CLOCK_REALTIME, &cur_time);
    CDBG("%s: -- VPE End -- frame id %d %ld %ld", __func__,
      node->dest_frame.frame.frame_id, cur_time.tv_sec,
      (cur_time.tv_nsec/1000));
  }
  dest->src_cb_ops->ack(dest->data.src_ptr,
    node->dest_frame.frame.frame_id, dest->my_idx);
  return rc;
}

void mctl_pp_dest_vpe_resend_cmd(void *p_poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node, int *del_node)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  pp_frame_data_t vpe_frame;
  int rc = 0;
  struct msm_pp_crop final_crop;

  *del_node = 0;
  node->dest_frame = *dest->data.p_free_frame;
  dest->data.p_free_frame = NULL;
  node->dest_frame.frame.frame_id = node->src_frame.frame.frame_id;
  node->dest_frame.frame.timestamp = node->src_frame.frame.timestamp;
  memset(&vpe_frame,  0,  sizeof(vpe_frame));
  node->state = MCTL_PP_FRAME_DOING;
  /* we have free frame. send to VPE */
  vpe_frame.dest_format = dest->data.format;
  vpe_frame.src_format = ((mctl_pp_divert_src_t *)dest->data.src_ptr)->data.format;
  CDBG("%s: Src format: %d Dest format: %d\n", __func__,
    vpe_frame.src_format, dest->data.format);
  vpe_frame.action_flag = dest->data.vpe_action_flag;
  vpe_frame.cookie = (void *)dest;

  if (dest->data.dis_enable) {
    /* ToDo: Queuing mechanism if dis data is old. */
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
  vpe_frame.crop = final_crop;

  vpe_frame.dest = &node->dest_frame;
  CDBG("%s: Dest frame info: planes: %d %lu\n", __func__,
    vpe_frame.dest->frame.num_planes, vpe_frame.dest->frame.mp->vaddr);
  vpe_frame.frame_id = node->src_frame.frame.frame_id;
  vpe_frame.path = dest->data.path;
  vpe_frame.src = &node->src_frame;

  CDBG("%s: Low power mode %d src img mode %d dest img mode %d\n", __func__,
    ctrl->enableLowPowerMode, vpe_frame.src->image_mode, vpe_frame.src->image_mode);
  if (ctrl->enableLowPowerMode) {
    vpe_frame.src_buf_data =
      ctrl->video_ctrl.user_buf_info[vpe_frame.src->image_mode].
      local_frame_info[vpe_frame.src->frame.buf_idx];
  } else {
    vpe_frame.src_buf_data =
      ctrl->video_ctrl.mctl_buf_info[vpe_frame.src->image_mode].
      local_frame_info[vpe_frame.src->frame.buf_idx];
  }
  vpe_frame.dst_buf_data =
    ctrl->video_ctrl.user_buf_info[vpe_frame.dest->image_mode].
    local_frame_info[vpe_frame.dest->frame.buf_idx];

  CDBG("%s Got source and destination buffer fds as %d and %d ",
    __func__, vpe_frame.src_buf_data.fd, vpe_frame.dst_buf_data.fd);
  vpe_frame.dest->frame.path = dest->data.path;

  rc = dest->hw_ops.process(dest->hw_ops.handle, VPE_EVENT_DO_PP,
    (void *)&vpe_frame);
  if (rc < 0) {
    dest->src_cb_ops->pp_error(dest->data.src_ptr,
    vpe_frame.frame_id,  dest->my_idx);
    CDBG_ERROR("%s: VPE error = %d",  __func__,  rc);
    dest->data.free_frame = node->dest_frame;
    dest->data.p_free_frame = &dest->data.free_frame;
    *del_node = 1;
    return;
  }
  if(vpe_frame.not_pp_flag) {
    /* no crop & dfis needed. send src frame back
       to app and keep orig free frame */
    dest->src_cb_ops->pp_no_op(dest->data.src_ptr, vpe_frame.not_pp_flag,
      vpe_frame.frame_id, dest->my_idx);
    dest->data.free_frame = node->dest_frame;
    dest->data.p_free_frame = &dest->data.free_frame;
    *del_node = 1;
  }
}

uint32_t mctl_pp_acquire_vpe(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest)
{
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_ops_t hw_ops;
  module_ops_t *vpe_mod_ops = NULL;
  comp_res_req_info_t res_req_info;
  mctl_pp_divert_src_t *src;
  struct pollfd poll_fd;
  int rc = 0, fds[2] = {0,0};

  CDBG("%s E\n", __func__);
  vpe_mod_ops = &dest->hw_ops;
  if (vpe_mod_ops->handle) {
    CDBG_ERROR("%s: VPE already acquired, handle = %d", __func__,
      vpe_mod_ops->handle);
    return (uint32_t)NULL;
  }

  memset(&hw_ops, 0, sizeof(mctl_ops_t));
  hw_ops.fd = ctrl->camfd;
  hw_ops.parent = (void *)poll_cb;
  hw_ops.buf_done = mctl_pp_dest_done_notify;

  CDBG("%s call reserve and create\n", __func__);
  res_req_info.comp_id = MCTL_COMPID_VPE;
  rc = qcamsvr_reserve_res(ctrl->cfg_arg.vnode_id, &res_req_info,
    NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: reserve resource failed for comp id %d ", __func__,
      res_req_info.comp_id);
    return -1;
  }
  if(0 == vpe_interface_create(vpe_mod_ops, res_req_info.sdev_revision)) {
    CDBG_ERROR("%s: cannot create VPE",  __func__);
    return (uint32_t)NULL;
  }

  rc = pipe(fds);
  if (rc < 0) {
    CDBG_ERROR("%s: pipe open err=%d", __func__, rc);
    goto error;
  }

  dest->data.pipe_fds[0] = fds[0];
  dest->data.pipe_fds[1] = fds[1];

  CDBG("%s: Sending FD[0] = %d FD[1] = %d", __func__, fds[0], fds[1]);
  rc = vpe_mod_ops->init(vpe_mod_ops->handle, &hw_ops, fds);
  if (rc < 0) {
    CDBG_ERROR("%s Error initializing VPE interface ", __func__);
    goto error;
  }

  src = dest->data.src_ptr;
  poll_fd.fd = fds[0];
  poll_fd.events = POLLIN|POLLRDNORM;
  /* Notify MCTL PP to start polling for ACK on this hw.*/
  mctl_pp_add_poll_fd(poll_cb, &poll_fd, src->my_idx, dest->my_idx);

  CDBG("%s: X\n", __func__);
  return vpe_mod_ops->handle;

error:
  if (fds[0]) {
    close(fds[0]);
    dest->data.pipe_fds[0] = fds[0] = 0;
  }

  if (fds[1]) {
    close(fds[1]);
    dest->data.pipe_fds[1] = fds[1] = 0;
  }

  if (vpe_mod_ops->destroy(vpe_mod_ops->handle) < 0)
    CDBG_ERROR("%s: vpe_destroy failed", __func__);

  return (uint32_t)NULL;
}

int mctl_pp_release_vpe(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest)
{
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  int rc = 0;
  module_ops_t *vpe_mod_ops = NULL;
  mctl_pp_divert_src_t *src;

  CDBG("%s E\n", __func__);
  vpe_mod_ops = &dest->hw_ops;

  if(vpe_mod_ops->handle) {
    rc = vpe_mod_ops->destroy(vpe_mod_ops->handle);
  }
  qcamsvr_release_res(ctrl->cfg_arg.vnode_id, MCTL_COMPID_VPE,
    NULL);

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
}
