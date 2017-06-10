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

static mctl_pp_dest_cmd_node_t *mctl_pp_get_free_dest_cmd_node(mctl_pp_t *poll_cb,
  mctl_pp_divert_src_t *src, mctl_pp_dest_t *dest)
{
  int i;
  for (i = 0; i < MCTL_PP_MAX_EVENT; i++) {
    if (dest->data.free_cmd_node[i].state == MCTL_PP_FRAME_NULL) {
      dest->data.free_cmd_node[i].state = MCTL_PP_FRAME_QUEUED;
      return &dest->data.free_cmd_node[i];
    }
  }
  return NULL;
}

static void mctl_pp_print_cmd_node_list(mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *cmd_list)
{
  int i;
  CDBG("%s Dest %d free cmd node list contains: ", __func__, dest->my_idx);
  CDBG(" (Index, FrameId, State) State_NULL = %d, State_QUEUED = %d"
    " State_QUEUED_NO_FREEFRAME = %d, State_DOING = %d\n",
    MCTL_PP_FRAME_NULL, MCTL_PP_FRAME_QUEUED,
    MCTL_PP_FRAME_QUEUED_NO_FREE_FRAME, MCTL_PP_FRAME_DOING);
  for (i = 0; i < MCTL_PP_MAX_EVENT; i++) {
    CDBG(" (%d, %d, %d) -> ", i, cmd_list[i].src_frame.frame.frame_id,
      cmd_list[i].state);
  }
  CDBG(" X ");
}

static void mctl_pp_put_dest_cmd_node(mctl_pp_t *poll_cb,
  mctl_pp_dest_cmd_node_t *node)
{
  memset(node,  0,  sizeof(mctl_pp_dest_cmd_node_t));
  node->state = MCTL_PP_FRAME_NULL;
}

static int mctl_pp_dest_config(void *p_poll_cb,
  mctl_pp_dest_cfg_cmd_t *dest_cfg)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[dest_cfg->src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_cfg->dest_idx];

  CDBG("%s src %d dest %d\n", __func__, dest_cfg->src_idx, dest_cfg->dest_idx);

  dest->data.proc_type = dest_cfg->parms.proc_type;
  dest->data.format = dest_cfg->parms.format;
  dest->data.image_mode = dest_cfg->parms.image_mode;
  dest->data.path = dest_cfg->parms.path;
  dest->data.dis_enable = dest_cfg->parms.dis_enable;
  dest->data.image_width = dest_cfg->parms.image_width;
  dest->data.image_height = dest_cfg->parms.image_height;
  dest->data.plane[0] = dest_cfg->parms.plane[0];
  dest->data.plane[1] = dest_cfg->parms.plane[1];
  dest->data.rotation = dest_cfg->parms.rotation;
  dest->data.hw_type = dest_cfg->parms.hw_type;

  switch(dest->data.proc_type) {
  case MCTL_PP_VPE_CROP_AND_DIS:
    dest->data.vpe_action_flag = dest_cfg->parms.action_flag;
    break;
  case MCTL_PP_C2D_CROP_2D:
    break;
  case MCTL_PP_S3D_3D_CORE:
  case MCTL_PP_S3D_C2D_VPE:
  default:
    break;
  }
  cam_list_init(&dest->data.cmd_list.list);
  dest->data.src_ptr = (void *)src;
  dest->data.stream_on = 0;
  CDBG("%s: dest_idx = %d, proc_type = %d, image_mode = %d, "
    "path = %d, act_flag = 0x%x, dis = %d, streamon = %d",
    __func__, dest_cfg->dest_idx,
    dest->data.proc_type, dest->data.image_mode, dest->data.path,
    dest->data.vpe_action_flag, dest->data.dis_enable, dest->data.stream_on);
  return 0;
}

static int mctl_pp_dest_acquire_hw(void *p_poll_cb, int src_idx, int dest_idx)
{
  int rc = 0;
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  uint32_t handle = 0;

  CDBG("%s Acquiring hardware %d ", __func__, dest->data.hw_type);
  switch(dest->data.hw_type) {
    case PP_HW_TYPE_VPE:
      handle = mctl_pp_acquire_vpe(poll_cb, dest);
      break;
    case PP_HW_TYPE_C2D:
      handle = mctl_pp_acquire_c2d(poll_cb, dest);
      break;
    case PP_HW_TYPE_S3D:
    default:
      handle = 0;
      break;
  }

  if (handle) {
    dest->data.dest_handle = handle;
    CDBG("%s Successfully acquired hw %d, handle %d", __func__,
      dest->data.hw_type, dest->data.dest_handle);
  } else {
    CDBG_ERROR("%s: Invalid hw type OR max instances reached for %d",
      __func__, dest->data.hw_type);
    rc = -EINVAL;
  }

  CDBG("%s X rc %d ", __func__, rc);
  return rc;
}

static void mctl_pp_dest_reset(void *p_poll_cb, mctl_pp_dest_cfg_cmd_t *dest_cfg)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[dest_cfg->src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_cfg->dest_idx];

  CDBG("%s src %d dest %d\n", __func__, dest_cfg->src_idx, dest_cfg->dest_idx);
  memset(&dest->data, 0, sizeof(dest->data));
  return;
}

static int mctl_pp_dest_release_hw(void *p_poll_cb, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  int rc = 0;

  CDBG("%s Releasing hardware %d ", __func__, dest->data.hw_type);
  switch(dest->data.hw_type) {
    case PP_HW_TYPE_VPE:
      rc = mctl_pp_release_vpe(poll_cb, dest);
      break;
    case PP_HW_TYPE_C2D:
      rc = mctl_pp_release_c2d(poll_cb, dest);
      break;
    case PP_HW_TYPE_S3D:
    default:
      rc = -1;
      break;
  }
  memset(&dest->hw_ops, 0, sizeof(module_ops_t));
  CDBG("%s X rc %d ", __func__, rc);
  return rc;
}

static void mctl_pp_dest_divert(void *p_poll_cb,
  struct msm_cam_evt_divert_frame *div_frame, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  int rc = 0;
  mctl_pp_dest_cmd_node_t *node = NULL;
  int del_node = 0;
  struct timespec cur_time;

  CDBG(" %s: frame_id = %d, src_idx = %d, dest_idx = %d", __func__,
    div_frame->frame.frame_id, src_idx, dest_idx);

  node= mctl_pp_get_free_dest_cmd_node(poll_cb, src, dest);
  if(!node) {
    CDBG_ERROR(" %s: no free dest cmd node.", __func__);
    if (dest->src_cb_ops->pp_error(dest->data.src_ptr, div_frame->frame.frame_id,
      dest->my_idx) < 0)
      CDBG_ERROR("%s: error reporting failed", __func__);
    return;
  }
  CDBG("%s: -----------DestIdx %d Divert Begin------------",
    __func__, dest_idx);

  node->src_frame = *div_frame;
  node->state = MCTL_PP_FRAME_QUEUED;
  cam_list_add_tail_node(&node->list, &dest->data.cmd_list.list);

  if (ctrl->videoHint || ctrl->channel_stream_info & STREAM_RAW) {
    if (dest->data.p_free_frame) {
      CDBG_HIGH("%s: dest idx %d has frame %d reserved already,"
        " no need to reserve another now\n", __func__, dest_idx,
        dest->data.p_free_frame->frame.buf_idx);
    } else {
      /* need to fetch one free frame from kernel mctl buf mgr */
      dest->data.free_frame.frame.path = dest->data.path;
      dest->data.free_frame.image_mode = dest->data.image_mode;
      CDBG(" dest divert reserve free frame\n");
      rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_RESERVE_FREE_FRAME,
        &dest->data.free_frame);
      if(rc < 0) {
        CDBG_ERROR(" %s: cannot reserve frame from kernel, path = 0x%x",
          __func__, dest->data.free_frame.frame.path);
          dest->src_cb_ops->pp_error(dest->data.src_ptr,
          div_frame->frame.frame_id, dest->my_idx);
        cam_list_del_node(&node->list);
        mctl_pp_put_dest_cmd_node(poll_cb, node);
        return;
      }
      CDBG("%s: Reserved frame for dest idx: %d rframe idx = %d "
        "dest image mode = %d\n", __func__, dest_idx,
        dest->data.free_frame.frame.buf_idx, dest->data.free_frame.image_mode);
      dest->data.p_free_frame = &dest->data.free_frame;
    }
  } else {
    /* We got an unexpected diverted frame.
     * This should not happen. Just put back the dest node into
     * the free cmd node list and return the diverted frame back.*/
    CDBG_HIGH("%s Frame diverted in camera mode. Sending back. ", __func__);
    dest->src_cb_ops->pp_error(dest->data.src_ptr, div_frame->frame.frame_id,
      dest->my_idx);
    cam_list_del_node(&node->list);
    mctl_pp_put_dest_cmd_node(poll_cb, node);
    return;
  }

  dest->data.frame_cnt++;
  CDBG(" %s proc_type: %d frame cnt = %d\n", __func__, dest->data.proc_type,
    dest->data.frame_cnt);

  switch(dest->data.proc_type) {
  case MCTL_PP_VPE_CROP_AND_DIS:
    CDBG("%s: VPE resend cmd", __func__);
    clock_gettime(CLOCK_REALTIME, &cur_time);
    CDBG("%s: -- VPE Start -- frame id %d %ld %ld", __func__,
                 div_frame->frame.frame_id, cur_time.tv_sec,
                 (cur_time.tv_nsec/1000));
    mctl_pp_dest_vpe_resend_cmd(p_poll_cb, dest, node, &del_node);
    if(del_node == 1) {
      cam_list_del_node(&node->list);
      mctl_pp_put_dest_cmd_node(poll_cb, node);
    }
    break;
  case MCTL_PP_C2D_CROP_2D:
    CDBG("%s: C2D resend cmd", __func__);
    clock_gettime(CLOCK_REALTIME, &cur_time);
    CDBG("%s: -- C2D Start -- frame id %d %ld %ld", __func__,
                 div_frame->frame.frame_id, cur_time.tv_sec,
                 (cur_time.tv_nsec/1000));
    mctl_pp_dest_c2d_resend_cmd(p_poll_cb, dest, node, &del_node);
    if(del_node == 1) {
      cam_list_del_node(&node->list);
      mctl_pp_put_dest_cmd_node(poll_cb, node);
    }
    break;
  case MCTL_PP_S3D_C2D_VPE:
    CDBG_ERROR("%s: need implementation.", __func__);
    dest->src_cb_ops->pp_error(dest->data.src_ptr, div_frame->frame.frame_id, dest->my_idx);
    rc = -1;
    break;
  case MCTL_PP_S3D_3D_CORE:
    CDBG_ERROR("%s: need implementation.", __func__);
    dest->src_cb_ops->pp_error(dest->data.src_ptr, div_frame->frame.frame_id, dest->my_idx);
    rc = -1;
    break;
  default:
    dest->src_cb_ops->pp_error(dest->data.src_ptr, div_frame->frame.frame_id, dest->my_idx);
    CDBG_ERROR("%s: proc_type = %d not supported",
      __func__, dest->data.proc_type);
    return;
  }
  CDBG("%s X ", __func__);
  return;
}

static int mctl_pp_dest_streamon(void *p_poll_cb, int src_idx, int dest_idx)
{
  int rc = 0;
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  module_ops_t *hw_ops = NULL;
  mctl_pp_hw_init_data init_data;

  CDBG(" %s dest idx: %d\n", __func__, dest_idx);

  dest->data.stream_on = 1;
  switch(dest->data.proc_type) {
  case MCTL_PP_VPE_CROP_AND_DIS: {
    hw_ops = &dest->hw_ops;
    int clk_rate = 0; /* TBD: make it configurable and move to vpe_cmd.c */

    if (0 != mctl_pp_gen_vpe_config_parm(poll_cb, src, dest)) {
      /* vpe pipe line config error */
      CDBG_ERROR("%s: cannot set VPE pipeline cfg",  __func__);
      return -1;
    }
    if(0 != hw_ops->set_params(hw_ops->handle, VPE_PARM_CLK_RATE,
      (void *)&clk_rate, NULL)) {
      CDBG_ERROR("%s: cannot set vpe clk rate %d",  __func__,  clk_rate);
      return -1;
    }
    if (0 != hw_ops->set_params(hw_ops->handle, VPE_PARM_LOW_POWER_MODE,
      (void *)&ctrl->enableLowPowerMode, NULL)) {
      CDBG_ERROR("%s: cannot set low power mode %d",  __func__,  clk_rate);
      return -1;
    }

    hw_ops->process(hw_ops->handle, VPE_EVENT_START, NULL);
    break;
  }
  case MCTL_PP_C2D_CROP_2D:
    hw_ops = &dest->hw_ops;
    init_data.src_format = src->data.format;
    init_data.dest_format = dest->data.format;
    rc = hw_ops->process(hw_ops->handle, C2D_EVENT_START, &init_data);
    if(rc < 0) {
      CDBG_ERROR("%s: Cannot Start C2D", __func__);
      return rc;
    }

    if (0 != mctl_pp_gen_c2d_config_parm(poll_cb, src, dest)) {
      /* C2D pipe line config error */
      CDBG_ERROR("%s: cannot set C2D cfg",  __func__);
      return -1;
    }

    break;
  default:
    rc = -1;
    break;
  }
  CDBG("%s: X", __func__);
  return rc;
}

static int mctl_pp_dest_streamoff(void *p_poll_cb, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  mctl_pp_dest_cmd_node_t *node;
  struct cam_list *head = &dest->data.cmd_list.list;
  struct cam_list *pos;
  module_ops_t *hw_ops = &dest->hw_ops;;

  CDBG("%s: Stopping dest %d proc_type %d", __func__, dest_idx,
    dest->data.proc_type);
  dest->data.stream_on = 0;
  switch(dest->data.proc_type) {
  case MCTL_PP_VPE_CROP_AND_DIS:
    CDBG("%s: Sending STOP to VPE", __func__);
    hw_ops->process(hw_ops->handle, VPE_EVENT_STOP, NULL);
    break;
  case MCTL_PP_C2D_CROP_2D:
    hw_ops->process(hw_ops->handle, C2D_EVENT_STOP, NULL);
    break;
  default:
    break;
  }
  CDBG("%s: Stopped dest %d image_mode = %d", __func__, dest_idx,
    dest->data.image_mode);
  /* delete all cmd_list */
  pos = head->next;
  while(pos != head) {
    CDBG("%s: , pos = %p, head = %p, pos->next = %p", __func__, pos, head, pos->next);
    node = member_of(pos, mctl_pp_dest_cmd_node_t, list);
    pos = pos->next;
    cam_list_del_node(&node->list);
    dest->src_cb_ops->stop(dest->data.src_ptr, node->src_frame.frame.frame_id,
      dest->my_idx);
    mctl_pp_put_dest_cmd_node(poll_cb, node);
  }
  memset(dest->data.free_cmd_node, 0, sizeof(mctl_pp_dest_cmd_node_t) *
    MCTL_PP_MAX_EVENT);
  dest->data.p_free_frame = NULL;
  CDBG("%s X ", __func__);
  return 0;
}

static void mctl_pp_dest_deinit(void *p_poll_cb, int src_idx, int dest_idx)
{
  return;
}

int mctl_pp_dest_done_notify(void *userdata, pp_frame_data_t *frame)
{
  int rc = 0;
  mctl_pp_t *poll_cb = userdata;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_dest_t *dest = (mctl_pp_dest_t *)frame->cookie;
  mctl_pp_dest_cmd_node_t *node;
  struct cam_list *head = &dest->data.cmd_list.list;
  struct cam_list *pos;

  CDBG(" %s: buf_done, frame_id = %d", __func__, frame->frame_id);
  for (pos = head->next; pos != head; pos = pos->next) {
    node = member_of(pos, mctl_pp_dest_cmd_node_t, list);
    CDBG(" loop frames:  src fid: %d fid: %d\n",
         node->src_frame.frame.frame_id, frame->frame_id);
    if(node->src_frame.frame.frame_id == frame->frame_id) {
      cam_list_del_node(&node->list);
      if(frame->status < 0) {
        CDBG_ERROR("%s: err notify, frame_id = 0x%x", __func__, frame->frame_id);
        dest->src_cb_ops->pp_error(dest->data.src_ptr, frame->frame_id, dest->my_idx);
      } else if(frame->not_pp_flag) {
        CDBG("%s dest set not_pp_flag, send no_op \n", __func__);
        dest->src_cb_ops->pp_no_op(dest->data.src_ptr, frame->not_pp_flag,
                                   frame->frame_id, dest->my_idx);
        dest->data.free_frame = node->dest_frame;
        dest->data.p_free_frame = &dest->data.free_frame;
      } else {
        dest->data.frame_cnt--;
        if(dest->data.proc_type == MCTL_PP_C2D_CROP_2D) {
          CDBG("%s: c2d ack frame cnt %d\n", __func__, dest->data.frame_cnt);
          mctl_pp_dest_c2d_ack_notify(poll_cb, dest, node);
        } else if(dest->data.proc_type == MCTL_PP_VPE_CROP_AND_DIS) {
          CDBG("%s: vpe ack frame cnt %d\n", __func__, dest->data.frame_cnt);
          mctl_pp_dest_vpe_ack_notify(poll_cb, dest, node);
        } else {
          CDBG_ERROR("%s: proc type error\n", __func__);
          break;
        }
      }
      mctl_pp_put_dest_cmd_node(poll_cb, node);
      break;
    } else {
      CDBG_HIGH("%s Warning! src node frame id %d doesnt match finished "
        "frame id %d of dest %d", __func__, node->src_frame.frame.frame_id,
        frame->frame_id, dest->my_idx);
    }
  }
  CDBG(" %s X\n", __func__);
  return 0;
}

static void mctl_pp_dest_set_crop(void *p_poll_cb,
  struct mctl_pp_crop_info *crop_info, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  CDBG("%s\n", __func__);

  dest->data.crop_info = *crop_info;
}

static void mctl_pp_dest_set_dis(void *p_poll_cb, cam_dis_info_t *dis_info,
  int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];

  dest->data.dis_info = *dis_info;

  CDBG("%s: x=%d, y=%d, frame_id=%d, extra w=%d h=%d", __func__, dis_info->x,
    dis_info->y, dis_info->frame_id, dis_info->extra_pad_w,
    dis_info->extra_pad_h);
}

static int mctl_pp_dest_handle_ack(void *p_poll_cb, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  int rc = 0;
  uint32_t handle = 0;

  switch(dest->data.hw_type) {
  case PP_HW_TYPE_VPE:
    if (poll_cb->data.cmd.pp_event.ack.cmd == VPE_CMD_ZOOM) {
      CDBG("%s: VPE_EVENT_ACK received", __func__);
      dest->hw_ops.process(dest->hw_ops.handle, VPE_EVENT_ACK,
        &poll_cb->data.cmd.pp_event);
    } else {
      CDBG("%s Ack %d not meant for dest %d ", __func__,
        poll_cb->data.cmd.pp_event.ack.cmd, dest_idx);
    }
    break;
  case PP_HW_TYPE_C2D:
    if (poll_cb->data.cmd.pp_event.ack.cmd == C2D_CMD_IMAGE_DRAW) {
      CDBG("%s: C2D_EVENT_ACK received", __func__);
      /* The cookie sent in the C2D ACK message is the
       * handle of the c2d client instance. */
      handle = poll_cb->data.cmd.pp_event.ack.cookie;
      if (handle == dest->data.dest_handle) {
        CDBG("%s: process C2D_EVENT_ACK for %d", __func__, handle);
        dest->hw_ops.process(handle, C2D_EVENT_ACK,
          &poll_cb->data.cmd.pp_event);
      } else {
        CDBG("%s Ack %d not meant for dest %d ", __func__,
          poll_cb->data.cmd.pp_event.ack.cmd, dest_idx);
      }
    }
    break;
  default:
    CDBG_ERROR("%s Invalid hw_type", __func__);
    rc = -EINVAL;
    break;
  }
  return rc;
}

static mctl_pp_dest_ops_t dest_ops = {
  .acquire_hw = mctl_pp_dest_acquire_hw,
  .release_hw = mctl_pp_dest_release_hw,
  .config_dest = mctl_pp_dest_config,
  .reset_dest = mctl_pp_dest_reset,
  .divert = mctl_pp_dest_divert,
  .streamon = mctl_pp_dest_streamon,
  .streamoff = mctl_pp_dest_streamoff,
  .deinit = mctl_pp_dest_deinit,
  .crop = mctl_pp_dest_set_crop,
  .dis = mctl_pp_dest_set_dis,
  .handle_ack = mctl_pp_dest_handle_ack,
};

int mctl_pp_dest_init(mctl_pp_t *poll_cb, void *p_src, int src_idx,
  int dest_idx, mctl_pp_src_cb_ops_t *p_src_cb_ops)
{
  mctl_pp_divert_src_t *src = p_src;
  mctl_pp_dest_t *dest = &src->dest[dest_idx];

  dest->my_idx = dest_idx;
  dest->ops = &dest_ops;
  dest->src_cb_ops = p_src_cb_ops;
  memset(&dest->data,  0,  sizeof(dest->data));
  cam_list_init(&dest->data.cmd_list.list);

  return 0;
}

/* Note: This routine assumes that crop_in and in_dis are generated from
 *       the same base, i.e. vfe_op.
 */
void mctl_pp_merge_crop_dis_offset(struct msm_pp_crop *crop_in,
  cam_dis_info_t *in_dis, struct msm_pp_crop *final_crop)
{
  uint32_t zoom_dis_x, zoom_dis_y, padded_w, padded_h;
  float zoom_ratio_x, zoom_ratio_y;

  padded_w = crop_in->src_w + (crop_in->src_x*2);
  padded_h = crop_in->src_h + (crop_in->src_y*2);

  *final_crop = *crop_in;

  zoom_ratio_x = (float)crop_in->src_w / (float)(padded_w);
  zoom_ratio_y = (float)crop_in->src_h / (float)(padded_h);
  final_crop->src_w = (float)(padded_w - in_dis->extra_pad_w) * zoom_ratio_x;
  final_crop->src_h = (float)(padded_h - in_dis->extra_pad_h) * zoom_ratio_y;

  final_crop->src_x = in_dis->x +
    (((padded_w - in_dis->extra_pad_w) - final_crop->src_w) / 2);
  final_crop->src_y = in_dis->y +
    (((padded_h - in_dis->extra_pad_h) - final_crop->src_h) / 2);

  CDBG("%s: final crop: src: x=%d, y=%d, w=%d, h=%d", __func__,
    final_crop->src_x, final_crop->src_y, final_crop->src_w, final_crop->src_h);
  CDBG("%s: final crop: dest: x=%d, y=%d, w=%d, h=%d", __func__,
    final_crop->dst_x, final_crop->dst_y, final_crop->dst_w, final_crop->dst_h);
} /* mctl_pp_merge_crop_dis_offset */
