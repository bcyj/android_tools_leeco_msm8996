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

static int mctl_pp_cmp_timestamp(struct timeval *t1, struct timeval *t2)
{
  if(t1->tv_sec > t2->tv_sec ||
     (t1->tv_sec == t2->tv_sec &&
      t1->tv_usec > t2->tv_usec)) {
    return 1;
  } else if(t1->tv_sec == t2->tv_sec && t1->tv_usec > t2->tv_usec) {
    return 0;
  } else
    return -1;
}

static mctl_pp_src_node_t *mctl_pp_get_free_src_node(mctl_pp_t *poll_cb,
  mctl_pp_divert_src_t *src)
{
  int i;
  for (i = 0; i < MCTL_PP_MAX_EVENT; i++) {
    if (src->data.free_node[i].state == MCTL_PP_FRAME_NULL) {
      src->data.free_node[i].state = MCTL_PP_FRAME_QUEUED;
      return &src->data.free_node[i];
    }
  }
  return NULL;
}

static void mctl_pp_put_src_node(mctl_pp_t *poll_cb, mctl_pp_src_node_t *node)
{
  memset(node,  0,  sizeof(mctl_pp_src_node_t));
}

static int mctl_pp_src_return_user_buffer(mctl_pp_t *poll_cb,
  struct msm_cam_evt_divert_frame *div_frame)
{
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  int rc = 0;

   /* Source buffer was allocated by HAL.
    * Just return it back to the video node instance.*/
   CDBG("%s Returning frame %x idx %d back to mctl pp node ",
         __func__, (int)div_frame->frame.handle, div_frame->frame.buf_idx);
   rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_RELEASE_FREE_FRAME, div_frame);
   if (rc < 0)
     CDBG_ERROR("%s Error returning frame to mctl pp node", __func__);

   return rc;
}

static int mctl_pp_src_return_mctl_buffer(mctl_pp_t *poll_cb,
  struct msm_cam_evt_divert_frame *div_frame)
{
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  int rc = 0;
#ifdef USE_ION
   struct ion_flush_data cache_data;
   int ion_fd;
   mctl_pp_node_buffer_alloc_t *buf_alloc = NULL;

   /* invalidate cache before sending back to hardware for write */
   memset(&cache_data, 0, sizeof(struct ion_flush_data));
   buf_alloc = &ctrl->pp_node.buf_alloc[div_frame->frame.buf_idx];
   cache_data.vaddr  = (void*)buf_alloc->buf;
   cache_data.fd     = buf_alloc->fd;
   cache_data.handle = buf_alloc->ion_alloc.handle;
   cache_data.length = buf_alloc->ion_alloc.len;

   if (mctl_pp_cache_ops(&cache_data, ION_IOC_INV_CACHES,
                         ctrl->ion_dev_fd) < 0)
     CDBG_ERROR("%s: mctl_pp_cache_ops failed", __func__);
#endif

   /* Source buffer was sent by daemon.
    * Just return it back to the mctl pp node instance.*/
   CDBG("%s Returning frame %x idx %d back to mctl pp node ",
         __func__, (int)div_frame->frame.handle, div_frame->frame.buf_idx);
   rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_MCTL_DIVERT_DONE, div_frame);
   if (rc < 0)
     CDBG_ERROR("%s Error returning frame to mctl pp node", __func__);

   return rc;
}

static int mctl_pp_src_config(void *p_poll_cb, mctl_pp_src_cfg_cmd_t *src_cfg)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl;
  mctl_pp_divert_src_t *src;
  mctl_pp_dest_t *dest = NULL;
  int rc = 0;
  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return -EINVAL;
  }

  if (!src_cfg || (src_cfg->src_idx >= MCTL_PP_MAX_SRC_NUM)) {
    CDBG_ERROR("%s src_cfg ptr is invalid!!", __func__);
    return -EINVAL;
  }

  pp_ctrl = &poll_cb->data.pp_ctrl;
  src = &pp_ctrl->src[src_cfg->src_idx];

  src->data.format = src_cfg->parms.format;
  src->data.image_mode = src_cfg->parms.image_mode; /* app's image mode */
  src->data.path = src_cfg->parms.path; /* which vfe path diverted here */
  src->data.dis_waiting_node = NULL;
  src->data.num_dest = src_cfg->num_dest;
  src->data.dis_enable = src_cfg->parms.dis_enable;
  src->data.image_width = src_cfg->parms.image_width;
  src->data.image_height = src_cfg->parms.image_height;
  src->data.plane[0] = src_cfg->parms.plane[0];
  src->data.plane[1] = src_cfg->parms.plane[1];
  src->my_idx = src_cfg->src_idx;
  cam_list_init(&src->data.src_list.list);
  src->data.stream_on = 0;

  CDBG("%s: src_idx = %d, image_mode = %d, "
    "path = 0x%x, num_dest = %d, dis = %d, streamon = %d",
    __func__, src_cfg->src_idx, src->data.image_mode, src->data.path,
    src->data.num_dest, src->data.dis_enable,
    src->data.stream_on);

  return rc;
}

static int mctl_pp_src_config_dest(void *p_poll_cb,
  mctl_pp_dest_cfg_cmd_t *dest_cfg)
{
  int rc = 0;
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src;
  mctl_pp_dest_t *dest;

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return -EINVAL;
  }

  if (!dest_cfg || (dest_cfg->src_idx >= MCTL_PP_MAX_SRC_NUM)
      || (dest_cfg->dest_idx >= MCTL_PP_MAX_DEST_NUM)) {
    CDBG_ERROR("%s dest_cfg ptr is invalid!!", __func__);
    return -EINVAL;
  }
  CDBG("%s Configuring src %d, dest %d ", __func__, dest_cfg->src_idx,
    dest_cfg->dest_idx);
  src = &pp_ctrl->src[dest_cfg->src_idx];
  dest = &src->dest[dest_cfg->dest_idx];
  rc = dest->ops->config_dest(poll_cb, dest_cfg);
  return rc;
}

static void mctl_pp_src_reset(void *p_poll_cb, int src_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = NULL;
  mctl_pp_divert_src_t *src;

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return;
  }

  if (src_idx >= MCTL_PP_MAX_SRC_NUM) {
    CDBG_ERROR("%s src_idx is invalid!!", __func__);
    return;
  }
  pp_ctrl = &poll_cb->data.pp_ctrl;
  src = &pp_ctrl->src[src_idx];
  memset(&src->data, 0, sizeof(src->data));
}

static void mctl_pp_src_reset_dest(void *p_poll_cb,
  mctl_pp_dest_cfg_cmd_t *dest_cfg)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src;
  mctl_pp_dest_t *dest;

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return;
  }

  if (!dest_cfg || (dest_cfg->src_idx >= MCTL_PP_MAX_SRC_NUM)
      || (dest_cfg->dest_idx >= MCTL_PP_MAX_DEST_NUM)) {
    CDBG_ERROR("%s dest_cfg ptr is invalid!!", __func__);
    return;
  }
  CDBG("%s Resetting src %d, dest %d ", __func__, dest_cfg->src_idx,
    dest_cfg->dest_idx);
  src = &pp_ctrl->src[dest_cfg->src_idx];
  dest = &src->dest[dest_cfg->dest_idx];
  dest->ops->reset_dest(poll_cb, dest_cfg);
  return;
}

static int mctl_pp_src_acquire_hw(void *p_poll_cb, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  int rc = 0;

  rc = dest->ops->acquire_hw(p_poll_cb, src_idx, dest_idx);

  return rc;
}

static int mctl_pp_src_release_hw(void *p_poll_cb, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  int rc = 0;

  rc = dest->ops->release_hw(p_poll_cb, src_idx, dest_idx);

  return rc;
}

static void mctl_pp_src_divert(void *p_poll_cb,
  struct msm_cam_evt_divert_frame *div_frame, int src_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_src_node_t *current_node = NULL, *temp_node = NULL;
  int dest_idx, rc = 0;
  mctl_pp_dest_t *dest = NULL;
  int divert_cnt = 0, dis_waiting_cnt = 0;
  struct timespec cur_time;

  clock_gettime(CLOCK_REALTIME, &cur_time);
  CDBG("%s: -- Iteration Begin -- frame id %d %ld %ld", __func__,
    div_frame->frame.frame_id, cur_time.tv_sec,
    (cur_time.tv_nsec/1000));
  CDBG("%s: divert frame path = %d, frame_id = 0x%x image mode %d\n",
    __func__, div_frame->frame.path, div_frame->frame.frame_id,
    div_frame->image_mode);
  /* src uses frame union for the list */
  if(mctl_pp_cmp_timestamp(&div_frame->frame.timestamp,
    &src->data.streamon_timestamp) <= 0) {
    CDBG_ERROR("%s: Drop dirty frame. divert frame path = %d, "
         "frame_id = 0x%x vb_handle: 0x%x, active_dest = %d\n",
         __func__, div_frame->frame.path, div_frame->frame.frame_id,
         div_frame->frame.handle, src->data.active_dest);
    return;
  }

  current_node = mctl_pp_get_free_src_node(poll_cb,
    &poll_cb->data.pp_ctrl.src[src_idx]);
  if(current_node == NULL) {
    CDBG_ERROR("%s node alloc null, frame, path = %d, frame_id = 0x%x dropped\n",
      __func__, div_frame->frame.path, div_frame->frame.frame_id);
    if (ctrl->enableLowPowerMode)
      mctl_pp_src_return_user_buffer(poll_cb, div_frame);
    else
      mctl_pp_src_return_mctl_buffer(poll_cb, div_frame);
    return;
  }
  CDBG("%s: node->count = %d \n", __func__, src->data.active_dest);
  current_node->div_frame = *div_frame;
  current_node->count = src->data.active_dest;

  if(src->data.dis_waiting_node) {
    CDBG("%s: There is already a frame in dis waiting mode. Dispatch waiting "
      " frame and put current frame into waiting mode", __func__);
    temp_node = current_node;
    current_node = src->data.dis_waiting_node;
    src->data.dis_waiting_node = temp_node;
    src->data.dis_waiting_node->dest_delivered_count = 0;
  } else {
    if(src->data.dis_enable) {
      if(div_frame->frame.frame_id > src->data.dis_info.frame_id) {
        CDBG("%s: dis_info for frame_id=%d is not ready. Put the current frame "
          "into dis waiting mode", __func__, div_frame->frame.frame_id);
        src->data.dis_waiting_node = current_node;
        src->data.dis_waiting_node->dest_delivered_count = 0;
        return;
      }
    }
  }

  if(current_node->count) {
    cam_list_add_tail_node(&current_node->list, &src->data.src_list.list);
    for(dest_idx = 0; dest_idx < src->data.num_dest; dest_idx++) {
      dest= &src->dest[dest_idx];
      CDBG("%s: dest idx %d streamon? %d, frame_id %d\n", __func__, dest_idx,
        dest->data.stream_on, current_node->div_frame.frame.frame_id);
      if(dest->data.stream_on) {
        dest->ops->divert(p_poll_cb, &current_node->div_frame, src_idx,
          dest_idx);
        divert_cnt++;
      }
    }
  }

  if(!divert_cnt) {
    if(current_node->count) {
      cam_list_del_node(&current_node->list);
    }
    mctl_pp_put_src_node(poll_cb, current_node);
    CDBG("%s: no active and stream on dest. drop divert frame - path = %d, "
      "frame_id = 0x%x vb_handle: 0x%x\n", __func__, div_frame->frame.path,
      div_frame->frame.frame_id, div_frame->frame.handle);
  }
  CDBG("%s X ", __func__);
} /* mctl_pp_src_divert */

static int mctl_pp_send_mctl_cmd(mctl_pp_t *poll_cb, int id, void *ptr,
  uint32_t length)
{
  struct msm_mctl_post_proc_cmd pp_cmd;
  struct msm_mctl_pp_cmd cmd;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  memset(&pp_cmd, 0, sizeof(pp_cmd));
  memset(&cmd, 0, sizeof(cmd));
  pp_cmd.type = MSM_PP_CMD_TYPE_MCTL;
  pp_cmd.cmd.id = id;
  pp_cmd.cmd.length = length;
  pp_cmd.cmd.value = ptr;
  if (ioctl(ctrl->camfd, MSM_CAM_IOCTL_MCTL_POST_PROC, &pp_cmd) < 0) {
    CDBG_ERROR("vpe_util_sendcmd: vpe_cmd failed...\n");
    return -1;
  }
  return 0;
}

static int mctl_pp_src_enable(mctl_pp_t *poll_cb, int enable, int path)
{
  int rc = 0;
  struct msm_mctl_pp_divert_pp divert_pp;

  divert_pp.path = path;
  divert_pp.enable = enable;
  rc = mctl_pp_send_mctl_cmd(poll_cb, MCTL_CMD_DIVERT_FRAME_PP_PATH,
    (void *)&divert_pp, sizeof(divert_pp));

  CDBG("%s: enable = %d, path %d done, rc = %d", __func__, divert_pp.enable,
    divert_pp.path, rc);

  return rc;
}

static int mctl_pp_src_streamon(void *p_poll_cb, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;

  CDBG("%s: image_mode = %d, path = %d", __func__,
   poll_cb->data.cmd.stream_info.image_mode, poll_cb->data.cmd.stream_info.path);

  if(src->data.active_dest == 0)
    src->data.stream_on = 1;

  dest->ops->streamon(p_poll_cb, src_idx, dest_idx);
  if(dest->data.stream_on) {
    src->data.active_dest++;
  }
  if(src->data.active_dest == 1) {
    if (ioctl(ctrl->camfd, MSM_CAM_IOCTL_GET_KERNEL_SYSTEM_TIME,
      &src->data.streamon_timestamp) < 0) {
      CDBG_ERROR("%s: cannot get system timestamp\n", __func__);
      return -1;
    }
    /* Start diverting the frames if the frames are coming from
     * Kernel. i.e input type is MCTL_PP_INPUT_FROM_KERNEL.  */
    if (poll_cb->input_type == MCTL_PP_INPUT_FROM_KERNEL) {
      CDBG_HIGH("%s Enable frame divert from kernel ", __func__);
      mctl_pp_src_enable(poll_cb, 1, src->data.path);
    } else {
      CDBG_HIGH("%s Frame divert not enabled. ", __func__);
    }
  }
  return 0;
}

static int mctl_pp_src_streamoff(void *p_poll_cb, int src_idx, int dest_idx)
{
  int rc = 0;
  int j, cnt = 0;
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  struct msm_cam_evt_divert_frame div_frame;

  if((src->data.dis_waiting_node) &&
    (!src->data.dis_waiting_node->dest_delivered_count)) {

    CDBG("%s: Remove the waiting node first", __func__);
    /* Source buffer was sent by daemon. Hence dont recycle the buffer.
     * Just return it back to the mctl pp node instance.*/
    div_frame = src->data.dis_waiting_node->div_frame;
    CDBG("%s Returning frame %x idx %d back to mctl pp node ", __func__,
      (int)div_frame.frame.handle, div_frame.frame.buf_idx);
    mctl_pp_src_return_mctl_buffer(poll_cb, &div_frame);
    mctl_pp_put_src_node(poll_cb, src->data.dis_waiting_node);
  }

  CDBG("%s Issuing streamoff on dest %d ", __func__, dest_idx);
  if(dest->data.stream_on) {
    dest->ops->streamoff(p_poll_cb, src_idx, dest_idx);
    src->data.active_dest--;
  }
  /*TODO: fix me, need to add streamoff count, do -1 for each streamoff*/
  for(j = 0; j < src->data.num_dest; j++)
    if(src->dest[j].data.stream_on == 0)
      cnt++;

  if(src->data.active_dest == 0) {
    src->data.stream_on = 0;
    /* Start diverting the frames if the frames are coming from
     * Kernel. i.e input type is MCTL_PP_INPUT_FROM_KERNEL.  */
    if (poll_cb->input_type == MCTL_PP_INPUT_FROM_KERNEL) {
      CDBG("%s: Disable Frame divert from kernel for %d", __func__,
        src->data.path);
      mctl_pp_src_enable(poll_cb, 0, src->data.path);
    } else {
      CDBG_HIGH("%s Need not disable frame divert. ", __func__);
    }
  }
  return rc;
}

static void mctl_pp_src_deinit(void *p_poll_cb, int src_idx)
{
}

int mctl_pp_src_frame_done(mctl_pp_t *poll_cb,
  struct msm_cam_evt_divert_frame *frame)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  CDBG("%s: rc = %d, fid = 0x%x, path=0x%x, mode=0x%x, vb=0x%x, ts=0x%x:0x%x",
    __func__, rc, frame->frame.frame_id, frame->frame.path, frame->image_mode,
    frame->frame.handle, (uint32_t)frame->frame.timestamp.tv_sec,
    (uint32_t)frame->frame.timestamp.tv_usec);
  rc = mctl_pp_divert_done((void *)poll_cb, frame);
  return rc;
}

static int mctl_pp_src_cb_def(void *src_ptr, uint32_t frame_id, int dest_idx,
  int b_stop)
{
  int rc = 0;
  mctl_pp_divert_src_t *src = src_ptr;
  mctl_pp_src_node_t *node = NULL;
  struct cam_list *head = &src->data.src_list.list;
  struct cam_list *pos;
  struct msm_cam_evt_divert_frame div_frame;
  struct timespec cur_time;
  mctl_pp_t *poll_cb = (mctl_pp_t *)src->p_poll_cb;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;

  CDBG("%s: frame_id = %d, src_idx = %d, dest_idx = %d", __func__, frame_id,
    src->my_idx, dest_idx);
  for (pos = head->next; pos != head; pos = pos->next) {
    node = member_of(pos, mctl_pp_src_node_t, list);
    if(node->div_frame.frame.frame_id == frame_id) {
      /* found the match */
      node->count--;
      CDBG("%s: frameid=0x%x, node count=%d, src idx=%d, dest idx=%d,"
            "send frame back %d stop %d", __func__, frame_id, node->count,
            src->my_idx, dest_idx, node->send_frame_back, b_stop);
      if(!node->count) {
        /* last dest done with this frame */
        cam_list_del_node(&node->list);
        div_frame = node->div_frame;
        mctl_pp_put_src_node(poll_cb, node);
        if (ctrl->videoHint && !ctrl->enableLowPowerMode) {
          /* VFE buffer allocated on mctl_pp node. Just return
           * it back to kernel and mark it as QUEUED. */
          CDBG("%s Returning mctl pp buffer %d back to kernel free queue",
            __func__, div_frame.frame.buf_idx);
          mctl_pp_src_return_mctl_buffer(poll_cb, &div_frame);
        } else {
          /* VFE buffer allocated by HAL. Just return
           * it back to kernel and mark it as QUEUED. */
          CDBG("%s Returning HAL buffer %d back to kernel free queue",
            __func__, div_frame.frame.buf_idx);
          mctl_pp_src_return_user_buffer(poll_cb, &div_frame);
        }
        clock_gettime(CLOCK_REALTIME, &cur_time);
        CDBG("%s: -- Iteration End -- frame id %d %ld %ld", __func__,
                 div_frame.frame.frame_id, cur_time.tv_sec,
                 (cur_time.tv_nsec/1000));
      }
      return rc;
    } else {
      CDBG_HIGH("%s NOT MATCHING Frame ID = %d src_list frame id = %d ",
        __func__, frame_id, node->div_frame.frame.frame_id);
    }
  }
  return -1;
}

static int mctl_pp_src_handle_ack(void *p_poll_cb, int src_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  int dest_idx;
  mctl_pp_dest_t *dest = NULL;

  for(dest_idx = 0; dest_idx < src->data.num_dest; dest_idx++) {
    dest = &src->dest[dest_idx];
    dest->ops->handle_ack(p_poll_cb, src_idx, dest_idx);
  }

  return 0;
}

static int mctl_pp_src_cb_err(void *src_ptr, uint32_t frame_id, int dest_idx)
{
  return mctl_pp_src_cb_def(src_ptr, frame_id, dest_idx, 0);
}

static void mctl_pp_src_cb_stop(void *src_ptr, uint32_t frame_id, int dest_idx)
{
  mctl_pp_src_cb_def(src_ptr, frame_id, dest_idx, 1);
}

static int mctl_pp_src_cb_no_op(void *src_ptr, int send_to_app,
  uint32_t frame_id, int dest_idx)
{
  mctl_pp_divert_src_t *src = src_ptr;
  mctl_pp_src_node_t *node = NULL;
  struct cam_list *head = &src->data.src_list.list;
  struct cam_list *pos;
  struct msm_cam_evt_divert_frame div_frame;

  for (pos = head->next; pos != head; pos = pos->next) {
    node = member_of(pos, mctl_pp_src_node_t, list);
    if(node->div_frame.frame.frame_id == frame_id) {
      /* found the match */
      node->count--;
      node->send_frame_back = send_to_app;
      CDBG("%s node count: %d\n", __func__, node->count);
      if(!node->count) {
        /* last dest done with this frame */
        if(node->send_frame_back) {
          CDBG("%s: no pp needed, send src frame_id = 0x%x back to HAL",
            __func__, node->div_frame.frame.frame_id);
          cam_list_del_node(&node->list);
          mctl_pp_src_frame_done((mctl_pp_t *)src->p_poll_cb, &node->div_frame);
          mctl_pp_put_src_node((mctl_pp_t *)src->p_poll_cb, node);
        } else {
          div_frame = node->div_frame;
          cam_list_del_node(&node->list);
          mctl_pp_put_src_node((mctl_pp_t *)src->p_poll_cb, node);

          /* VFE buffer allocated by HAL. Just return
           * it back to kernel and mark it as QUEUED. */
          CDBG("%s Returning source buffer %d back to kernel free queue",
            __func__, div_frame.frame.buf_idx);
          mctl_pp_src_return_user_buffer((mctl_pp_t *)src->p_poll_cb,
            &div_frame);
        }
      }
      return 0;
    }
  }
  return -1;
}

static void mctl_pp_src_set_crop(void *p_poll_cb,
  struct mctl_pp_crop_info *crop_info, int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];

  dest->ops->crop(p_poll_cb, crop_info, src_idx, dest_idx);
}

static void mctl_pp_src_set_dis(void *p_poll_cb, cam_dis_info_t *dis_info,
  int src_idx, int dest_idx)
{
  mctl_pp_t *poll_cb = p_poll_cb;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_pp_divert_src_t *src = &pp_ctrl->src[src_idx];
  mctl_pp_dest_t *dest = &src->dest[dest_idx];
  mctl_pp_src_node_t *node = NULL;
  int divert_cnt = 0;

  /* Note: We don't need dis_info at both src and dest but for now its fine. */
  src->data.dis_info = *dis_info;
  dest->ops->dis(p_poll_cb, dis_info, src_idx, dest_idx);

  node = src->data.dis_waiting_node;
  if(node) {
    CDBG("%s: We've DIS waiting node. Dispatch it. Node Count %d", __func__,
      src->data.active_dest);
    if(node->count) {
      if(!src->data.dis_waiting_node->dest_delivered_count)
        cam_list_add_tail_node(&node->list, &src->data.src_list.list);
      if(dest->data.stream_on) {
        dest->ops->divert(p_poll_cb, &node->div_frame, src_idx, dest_idx);
        divert_cnt++;
      }
    }

    if(!divert_cnt) {
      if(node->count) {
        cam_list_del_node(&node->list);
      }
      mctl_pp_put_src_node(poll_cb, node);
      CDBG("%s: no active and stream on dest. drop divert frame - path = %d, "
        "frame_id = 0x%x vb_handle: 0x%x\n", __func__,
        node->div_frame.frame.path, node->div_frame.frame.frame_id,
        node->div_frame.frame.handle);
      src->data.dis_waiting_node = NULL;
      return;
    }
    if (src->data.dis_waiting_node->state != MCTL_PP_FRAME_NULL) {
      src->data.dis_waiting_node->dest_delivered_count++;
      CDBG("%s: Node Count %d, dest_delv_cnt %d", __func__, node->count,
	  src->data.dis_waiting_node->dest_delivered_count);
      if(src->data.dis_waiting_node->dest_delivered_count >=
                                       src->data.active_dest) {
	CDBG("%s: All dest divert is done. Remove from waiting list", __func__);
	src->data.dis_waiting_node = NULL;
      }
    } else {
      /* If the dis_waiting_node state has become NULL, it means that
       * all the destinations are done with the buffer and its returned back
       * to the kernel. In that case, there is no need to check for dest
       * delivered count. Just set the waiting node to NULL. */
       src->data.dis_waiting_node = NULL;
    }
  }

  CDBG("%s: X", __func__);
}

static void mctl_pp_src_cb_ack(void *src_ptr, uint32_t frame_id, int dest_idx)
{
  mctl_pp_src_cb_def(src_ptr, frame_id, dest_idx, 0);
}

static mctl_pp_src_ops_t src_ops = {
  .acquire_hw = mctl_pp_src_acquire_hw,
  .release_hw = mctl_pp_src_release_hw,
  .config_src = mctl_pp_src_config,
  .reset_src = mctl_pp_src_reset,
  .config_dest = mctl_pp_src_config_dest,
  .reset_dest = mctl_pp_src_reset_dest,
  .divert = mctl_pp_src_divert,
  .streamon = mctl_pp_src_streamon,
  .streamoff = mctl_pp_src_streamoff,
  .deinit = mctl_pp_src_deinit,
  .crop = mctl_pp_src_set_crop,
  .dis = mctl_pp_src_set_dis,
  .handle_ack = mctl_pp_src_handle_ack,
};

static mctl_pp_src_cb_ops_t src_cb_ops = {
  .pp_error = mctl_pp_src_cb_err,
  .pp_no_op = mctl_pp_src_cb_no_op,
  .ack = mctl_pp_src_cb_ack,
  .stop = mctl_pp_src_cb_stop,
};

int mctl_pp_src_init(mctl_pp_t *poll_cb)
{
  int i, j;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;

  for(i = 0; i < MCTL_PP_MAX_SRC_NUM; i++) {
    pp_ctrl->src[i].my_idx = i;
    pp_ctrl->src[i].ops = &src_ops;
    pp_ctrl->src[i].p_poll_cb = (void *)poll_cb;
    memset(&pp_ctrl->src[i].data, 0, sizeof(pp_ctrl->src[i].data));
    cam_list_init(&pp_ctrl->src[i].data.src_list.list);

    for(j = 0; j < MCTL_PP_MAX_DEST_NUM; j++) {
      mctl_pp_dest_init(poll_cb, (void *)&pp_ctrl->src[i], i, j, &src_cb_ops);
      CDBG("%s: init src_idx = %d, dest_idx = %d", __func__, i, j);
    }
  }
  return 0;
}
