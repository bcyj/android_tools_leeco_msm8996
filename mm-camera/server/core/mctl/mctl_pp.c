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
#include <sys/socket.h>
#include <linux/un.h>
#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#include "config_proc.h"
#include "mctl.h"
#include "mctl_pp.h"
#include "mctl_divert.h"
#include "vpe_api.h"
#include "eztune_preview.h"

#define BUFF_SIZE_128 128
#define DUMP_YUV 0

#if 0
#undef CDBG
#define CDBG LOGE
#endif

static int mctl_pp_dump(struct msm_frame *frame, uint32_t len)
{
  char bufp[BUFF_SIZE_128];
  int file_fdp;
  int rc = 0;
  snprintf(bufp, BUFF_SIZE_128, "/data/divert_pp.yuv");
  file_fdp = open(bufp, O_RDWR | O_CREAT, 0777);

  if (file_fdp < 0) {
    CDBG("cannot open file %s\n", bufp);
    rc = -1;
    goto end;
  }
  CDBG("%s:dump frame to '%s', len=0x%x, addr=0x%x\n",
    __func__, bufp, len, (uint32_t)frame->buffer);
  write(file_fdp,
    (const void *)frame->buffer, len);

  close(file_fdp);

  end:
  return rc;
}

static void mctl_pp_proc_pp_frame(mctl_pp_t *poll_cb,
  struct msm_cam_evt_divert_frame *div_frame)
{
  int i;

  CDBG("%s: divert frame path = %d, frame_id = %d %d", __func__,
    div_frame->frame.path, div_frame->frame.frame_id, div_frame->image_mode);

  for(i = 0; i < poll_cb->data.pp_ctrl.num_src; i++) {
    if(div_frame->frame.path == poll_cb->data.pp_ctrl.src[i].data.path) {
      mctl_pp_divert_src_t *src = &poll_cb->data.pp_ctrl.src[i];
      src->ops->divert((void *)poll_cb, div_frame, i);
      break;
    }
  }
}

static void mctl_pp_poll_sig_done(mctl_pp_t *poll_cb)
{
  CDBG("%s\n", __func__);

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return;
  }

  pthread_mutex_lock(&poll_cb->mutex);
  poll_cb->status = TRUE;
  pthread_cond_signal(&poll_cb->cond_v);
  pthread_mutex_unlock(&poll_cb->mutex);
}

int32_t mctl_pp_cmd(mctl_pp_t *poll_cb, mctl_pp_cmd_t *cmd)
{
  CDBG("%s\n", __func__);

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return -EINVAL;
  }
  if (!cmd) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return -EINVAL;
  }
  if (poll_cb->data.pfds[1] == 0) {
    CDBG_HIGH("%s: pipe for mctl pp thread is already closed.", __func__);
    return 0;
  }

  pthread_mutex_lock(&poll_cb->mutex);
  poll_cb->status = FALSE;
  pthread_mutex_unlock(&poll_cb->mutex);

  write(poll_cb->data.pfds[1], cmd, sizeof(mctl_pp_cmd_t));

  pthread_mutex_lock(&poll_cb->mutex);
  if (FALSE == poll_cb->status)
    pthread_cond_wait(&poll_cb->cond_v, &poll_cb->mutex);
  pthread_mutex_unlock(&poll_cb->mutex);

  return 0;
}

static int mctl_pp_pipeline_init(mctl_pp_t *poll_cb, void *cfg_ctrl)
{
  CDBG("%s\n", __func__);

  pthread_mutex_init(&poll_cb->mutex, NULL);
  pthread_cond_init(&poll_cb->cond_v, NULL);
  memset(&poll_cb->data, 0, sizeof(poll_cb->data));
  poll_cb->data.cfg_ctrl = cfg_ctrl;
  mctl_pp_src_init(poll_cb);

  return 0;
}
static int mctl_pp_send_mctl_cmd(mctl_pp_t *poll_cb,
  int id, void *ptr, uint32_t length)
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

static int mctl_pp_proc_pp_event(mctl_pp_t *poll_cb)
{
  int rc = 0, i;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;

  if (poll_cb->data.cmd.pp_event.event == MCTL_PP_EVENT_CMD_ACK) {
    for(i= 0; i < pp_ctrl->num_src; i++)
      pp_ctrl->src[i].ops->handle_ack((void *)poll_cb, i);
  }
  return rc;
}

static void mctl_pp_proc_data(mctl_pp_t *poll_cb)
{
  struct msm_cam_evt_divert_frame *div_frame;
  switch (poll_cb->data.cmd.evt_type) {
    case MSM_CAM_RESP_DIV_FRAME_EVT_MSG:
      CDBG("%s div frame\n", __func__);
      div_frame = &poll_cb->data.cmd.div_frame;
      mctl_pp_proc_pp_frame(poll_cb, div_frame);
      break;
    case MSM_CAM_RESP_MCTL_PP_EVENT:
      mctl_pp_proc_pp_event(poll_cb);
      break;
    default:
      break;
  }
}

static void mctl_pp_clear_buf(mctl_pp_t *poll_cb)
{
  int i, j;
  mctl_pp_buf_info_t *buf_info = NULL;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;

  CDBG("%s: E", __func__);

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return;
  }

  for (i = 0; i < MSM_V4L2_EXT_CAPTURE_MODE_MAX; i++) {
    buf_info = &(ctrl->video_ctrl.user_buf_info[i]);
    if (buf_info->active_mapping_count > 0) {
      for (j = 0; j < MCTL_PP_MAX_FRAME_NUM; j++) {
        if (buf_info->local_frame_info[j].fd > 0) {
          CDBG_HIGH("%s:  unmap buf_type=%d with idx=%d\n", __func__, i, j);

          munmap((void *)buf_info->local_frame_info[j].local_vaddr,
            buf_info->remote_frame_info[j].size);

          close(buf_info->local_frame_info[j].fd);

          buf_info->local_frame_info[j].fd = 0;
          buf_info->local_frame_info[j].local_vaddr = NULL;
          buf_info->active_mapping_count--;

          memset(&buf_info->remote_frame_info[j], 0,
            sizeof(mm_camera_frame_map_type));
        }
      }
    }
  }
}

static int mctl_pp_stream_on_off_notify(mctl_pp_t *poll_cb, int streamon)
{
  int rc = 0;
  int i, j;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;


  CDBG("%s\n", __func__);
  for(i= 0; i < pp_ctrl->num_src; i++) {
    for(j = 0; j < pp_ctrl->src[i].data.num_dest; j++) {

      CDBG("%s: Image mode: %d poll %d Path %d poll %d\n", __func__,
        pp_ctrl->src[i].dest[j].data.image_mode,
        poll_cb->data.cmd.stream_info.image_mode,
        pp_ctrl->src[i].dest[j].data.path,
        poll_cb->data.cmd.stream_info.path);

      if(pp_ctrl->src[i].dest[j].data.image_mode ==
        poll_cb->data.cmd.stream_info.image_mode &&
        pp_ctrl->src[i].dest[j].data.path ==
        poll_cb->data.cmd.stream_info.path) {
        if(streamon)
          rc = pp_ctrl->src[i].ops->streamon((void *)poll_cb, i, j);
        else
          rc = pp_ctrl->src[i].ops->streamoff((void *)poll_cb, i, j);
        break;
      }
    }
  }
  CDBG("%s: rc = %d, image_mode = %d, path = %d, streamon=%d",
    __func__, rc, poll_cb->data.cmd.stream_info.image_mode,
       poll_cb->data.cmd.stream_info.path, streamon);
  return rc;
}

static void mctl_pp_do_shutdown(mctl_pp_t *poll_cb)
{
  int rc = 0;
  int i, j;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;

  /* Process was killed. Shutdown all the sources */
  for(i= 0; i < pp_ctrl->num_src; i++) {
    for(j = 0; j < pp_ctrl->src[i].data.num_dest; j++) {
      CDBG_HIGH("%s: SHUT DOWN src %d dest %d image_mode = %d",
        __func__, i, j, pp_ctrl->src[i].dest[j].data.image_mode);
      rc = pp_ctrl->src[i].ops->streamoff((void *)poll_cb, i, j);
      CDBG("%s Src %d streamoff rc = %d ", __func__, i, rc);
    }
  }
  mctl_pp_clear_buf(poll_cb);
}

static int mctl_pp_notify_dis_info(mctl_pp_t *poll_cb)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  int image_mode, i, j;
  cam_dis_info_t *dis_info = &poll_cb->data.cmd.dis_cmd;

  CDBG("%s: x=%d, y=%d, frame_id=%d, extra w=%d h=%d", __func__, dis_info->x,
    dis_info->y, dis_info->frame_id, dis_info->extra_pad_w,
    dis_info->extra_pad_h);

  for(i = 0; i < poll_cb->data.pp_ctrl.num_src; i++) {
    if (poll_cb->data.pp_ctrl.src[i].data.dis_enable) {
      for(j = 0; j < poll_cb->data.pp_ctrl.src[i].data.num_dest; j++) {
        poll_cb->data.pp_ctrl.src[i].ops->dis((void *)poll_cb, dis_info, i, j);
      }
    }
  }
  return 0;
} /* mctl_pp_notify_dis_info */

static int mctl_pp_notify_crop_info(mctl_pp_t *poll_cb)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  int image_mode, i, j;
  uint32_t pp_src_w, pp_src_h;
  struct mctl_pp_crop_info crop_out;
  /* Note: crop_in is based on VFE Primary output (MainImg)*/
  struct mctl_pp_crop_cmd *crop_in = &poll_cb->data.cmd.crop_info;

  CDBG("%s: crop_in: path = %d, in_w=%d, in_height=%d, out_width=%d,"
    "out_height=%d", __func__, crop_in->src_path, crop_in->src_w,
    crop_in->src_h, crop_in->dst_w, crop_in->dst_h);

  for(i = 0; i < poll_cb->data.pp_ctrl.num_src; i++) {
    pp_src_w = poll_cb->data.pp_ctrl.src[i].data.image_width;
    pp_src_h = poll_cb->data.pp_ctrl.src[i].data.image_height;

    if (poll_cb->data.pp_ctrl.src[i].data.path == crop_in->src_path) {
      for(j = 0; j < poll_cb->data.pp_ctrl.src[i].data.num_dest; j++) {

        crop_out.crop.src_w = pp_src_w;
        crop_out.crop.src_h = pp_src_h;
        crop_out.crop.dst_w =
          poll_cb->data.pp_ctrl.src[i].dest[j].data.image_width;
        crop_out.crop.dst_h =
          poll_cb->data.pp_ctrl.src[i].dest[j].data.image_height;

        CDBG("%s: Default src[%d] w=%d: h=%d", __func__, i, crop_out.crop.src_w,
          crop_out.crop.src_h);
        CDBG("%s: Default dst[%d] w=%d: h=%d", __func__, j, crop_out.crop.dst_w,
          crop_out.crop.dst_h);

        crop_out.crop.src_x = 0;
        crop_out.crop.src_y = 0;
        crop_out.crop.dst_x = 0;
        crop_out.crop.dst_y = 0;

        if (crop_in->src_w == 0 && crop_in->src_h == 0) {
          crop_out.crop.src_w = pp_src_w;
          crop_out.crop.src_h = pp_src_h;
        } else {
          /* Normalize src crop wxh from PRIMARY to SECONDARY */
          crop_out.crop.src_w =
            (uint32_t)(((float)crop_in->src_w/(float)crop_in->dst_w)*pp_src_w);
          crop_out.crop.src_h =
            (uint32_t)(((float)crop_in->src_h/(float)crop_in->dst_h)*pp_src_h);
        }

        crop_out.crop.src_x = (pp_src_w - crop_out.crop.src_w)/2;
        if (((crop_out.crop.src_x * 2) + crop_out.crop.src_w) > pp_src_w)
          crop_out.crop.src_x = 0;
        crop_out.crop.src_y = (pp_src_h - crop_out.crop.src_h)/2;
        if (((crop_out.crop.src_y * 2) + crop_out.crop.src_h) > pp_src_h)
          crop_out.crop.src_y = 0;

        CDBG("%s: crop out src: x=%d, y=%d wxh = %dx%d ", __func__,
          crop_out.crop.src_x, crop_out.crop.src_y, crop_out.crop.src_w,
          crop_out.crop.src_h);
        CDBG("%s: crop out dst: wxh = %dx%d src = %dx%d", __func__,
          crop_out.crop.dst_w, crop_out.crop.dst_h, crop_out.crop.src_w,
          crop_out.crop.src_h);

        poll_cb->data.pp_ctrl.src[i].ops->crop((void *)poll_cb, &crop_out, i, j);
      }
    }
  }
  return 0;
}

static int mctl_pp_proc_config_src(mctl_pp_t *poll_cb,
  mctl_pp_src_cfg_cmd_t *src_cfg)
{
  int rc = 0;
  int i, j;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;

  pp_ctrl->dimInfo = src_cfg->dimInfo;
  pp_ctrl->num_src = src_cfg->num_src;
  pp_ctrl->op_mode = src_cfg->op_mode;
  pp_ctrl->src[src_cfg->src_idx].data.num_dest_configured = 0;

  if(pp_ctrl->src[src_cfg->src_idx].ops)
    pp_ctrl->src[src_cfg->src_idx].ops->config_src(poll_cb, src_cfg);
  else
    CDBG_ERROR("%s src %d ops null", __func__, src_cfg->src_idx);

  return rc;
}

static int mctl_pp_proc_config_dest(mctl_pp_t *poll_cb,
  mctl_pp_dest_cfg_cmd_t *dest_cfg)
{
  int rc = 0;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;

  CDBG("%s Sending config_dest %d for src %d ", __func__,
    dest_cfg->dest_idx, dest_cfg->src_idx);

  if(pp_ctrl->src[dest_cfg->src_idx].ops) {
    pp_ctrl->src[dest_cfg->src_idx].ops->config_dest(poll_cb, dest_cfg);
    pp_ctrl->src[dest_cfg->src_idx].data.num_dest_configured++;
    CDBG("%s # of Dest configured for src %d = %d ", __func__,
      dest_cfg->src_idx,
      pp_ctrl->src[dest_cfg->src_idx].data.num_dest_configured);
  } else
    CDBG_ERROR("%s src %d ops null", __func__, dest_cfg->src_idx);

  return rc;
}

static void mctl_pp_proc_reset_src(mctl_pp_t *poll_cb, int src_idx)
{
  mctl_config_ctrl_t *ctrl = NULL;
  mctl_pp_ctrl_t *pp_ctrl = NULL;

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return;
  }

  ctrl = poll_cb->data.cfg_ctrl;
  pp_ctrl = &poll_cb->data.pp_ctrl;

  if (pp_ctrl->src[src_idx].data.num_dest_configured) {
    CDBG_ERROR("%s Error: Src %d still has %d dest configured! ", __func__,
      src_idx, pp_ctrl->src[src_idx].data.num_dest_configured);
    return;
  }
  if (pp_ctrl->src[src_idx].ops)
    pp_ctrl->src[src_idx].ops->reset_src((void *)poll_cb, src_idx);
  else
    CDBG_ERROR("%s src %d ops null", __func__, src_idx);

  pp_ctrl->num_src = 0;
  pp_ctrl->op_mode = 0;
  memset(&pp_ctrl->dimInfo, 0, sizeof(pp_ctrl->dimInfo));
}

static void mctl_pp_proc_reset_dest(mctl_pp_t *poll_cb,
  mctl_pp_dest_cfg_cmd_t *dest_cfg)
{
  mctl_pp_ctrl_t *pp_ctrl = NULL;

  if (!poll_cb) {
    CDBG_ERROR("%s: error: poll_cb is NULL", __func__);
    return;
  }

  pp_ctrl = &poll_cb->data.pp_ctrl;
  if (pp_ctrl->src[dest_cfg->src_idx].ops &&
      pp_ctrl->src[dest_cfg->src_idx].data.num_dest_configured) {
    pp_ctrl->src[dest_cfg->src_idx].ops->reset_dest(poll_cb, dest_cfg);
    pp_ctrl->src[dest_cfg->src_idx].data.num_dest_configured--;
  } else {
    CDBG_ERROR("%s src %d dont need reset dest", __func__, dest_cfg->src_idx);
    return;
  }
  CDBG("%s # of Dest configured for src %d = %d ", __func__, dest_cfg->src_idx,
    pp_ctrl->src[dest_cfg->src_idx].data.num_dest_configured);
}

static int mctl_pp_acquire_hw(mctl_pp_t *poll_cb)
{
  int i, j, rc = 0;
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;

  for(i= 0; i < pp_ctrl->num_src; i++) {
    for(j = 0; j < pp_ctrl->src[i].data.num_dest; j++) {

      CDBG_HIGH("%s: Image mode: %d poll %d Path %d poll %d\n", __func__,
        pp_ctrl->src[i].dest[j].data.image_mode,
        poll_cb->data.cmd.stream_info.image_mode,
        pp_ctrl->src[i].dest[j].data.path,
        poll_cb->data.cmd.stream_info.path);

      if ((pp_ctrl->src[i].dest[j].data.image_mode ==
             poll_cb->data.cmd.stream_info.image_mode) &&
          (pp_ctrl->src[i].dest[j].data.path ==
             poll_cb->data.cmd.stream_info.path)) {
        rc = pp_ctrl->src[i].ops->acquire_hw((void *)poll_cb, i, j);
        break;
      }
    }
  }
  return rc;
}

static int mctl_pp_release_hw(mctl_pp_t *poll_cb)
{
  int i, j, rc = 0;
#ifndef VFE_2X
  mctl_pp_ctrl_t *pp_ctrl = &poll_cb->data.pp_ctrl;

  for(i= 0; i < pp_ctrl->num_src; i++) {
    for(j = 0; j < pp_ctrl->src[i].data.num_dest; j++) {

      CDBG("%s: Image mode: %d poll %d Path %d poll %d\n", __func__,
        pp_ctrl->src[i].dest[j].data.image_mode,
        poll_cb->data.cmd.stream_info.image_mode,
        pp_ctrl->src[i].dest[j].data.path,
        poll_cb->data.cmd.stream_info.path);

      if ((pp_ctrl->src[i].dest[j].data.image_mode ==
             poll_cb->data.cmd.stream_info.image_mode) &&
          (pp_ctrl->src[i].dest[j].data.path ==
             poll_cb->data.cmd.stream_info.path)) {
        rc = pp_ctrl->src[i].ops->release_hw((void *)poll_cb, i, j);
        break;
      }
    }
  }
#endif
  return rc;
}

static void mctl_pp_read_pipe(mctl_pp_t *poll_cb, int fd)
{
  ssize_t read_len;
  int rc = 0;

  read_len = read(fd, &poll_cb->data.cmd, sizeof(poll_cb->data.cmd));

  switch (poll_cb->data.cmd.cmd_type) {
    case QCAM_MCTL_CMD_ACQUIRE_HW:
      mctl_pp_acquire_hw(poll_cb);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_RELEASE_HW:
      mctl_pp_release_hw(poll_cb);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_DATA:
      mctl_pp_poll_sig_done(poll_cb);
      mctl_pp_proc_data(poll_cb);
      break;
    case QCAM_MCTL_CMD_STREAMON:
      mctl_pp_stream_on_off_notify(poll_cb, 1);
      CDBG("%s: QCAM_MCTL_CMD_STREAMON", __func__);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_STREAMOFF:
      CDBG("%s: QCAM_MCTL_CMD_STREAMOFF", __func__);
      rc = mctl_pp_stream_on_off_notify(poll_cb, 0);
      if (rc == 0)
        mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_SET_CROP:
      CDBG(" %s: QCAM_MCTL_CMD_SET_CROP", __func__);
      mctl_pp_notify_crop_info(poll_cb);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_SET_DIS:
      CDBG(" %s: QCAM_MCTL_CMD_SET_DIS", __func__);
      mctl_pp_notify_dis_info(poll_cb);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_CONFIG_SRC:
      mctl_pp_proc_config_src(poll_cb, &poll_cb->data.cmd.src_cfg);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_CONFIG_DEST:
      mctl_pp_proc_config_dest(poll_cb, &poll_cb->data.cmd.dest_cfg);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_RESET_SRC:
      mctl_pp_proc_reset_src(poll_cb, poll_cb->data.cmd.src_cfg.src_idx);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_RESET_DEST:
      mctl_pp_proc_reset_dest(poll_cb, &poll_cb->data.cmd.dest_cfg);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_SHUTDOWN:
      CDBG_HIGH("%s Got CMD_SHUTDOWN ", __func__);
      mctl_pp_do_shutdown(poll_cb);
      CDBG_HIGH("%s Sending SIG_DONE ", __func__);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    case QCAM_MCTL_CMD_EXIT:
      poll_cb->data.release = 1;
      mctl_pp_clear_buf(poll_cb);
      mctl_pp_poll_sig_done(poll_cb);
      break;
    default:
      CDBG_ERROR("%s Unrecognized cmd %d ", __func__,
        poll_cb->data.cmd.cmd_type);
      break;
  }
}

static void *mctl_pp_poll_fn(mctl_pp_t *poll_cb)
{
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  int rc = 0, i;
  struct pollfd fds[PP_HW_TYPE_MAX_NUM + 1];
  int num_fds = PP_HW_TYPE_MAX_NUM + 1;
  int timeoutms;

  memset(fds, 0, sizeof(struct pollfd) * num_fds);
  do {
    fds[0].fd = poll_cb->data.pfds[0];
    fds[0].events = POLLIN|POLLRDNORM;

    for (i = 1; i <= PP_HW_TYPE_MAX_NUM; i++) {
      if (poll_cb->data.poll_fds[i-1].in_use) {
        fds[i].fd = poll_cb->data.poll_fds[i-1].poll_fd.fd;
        fds[i].events = poll_cb->data.poll_fds[i-1].poll_fd.events;
      } else
        fds[i].fd = -1;
    }

    CDBG("%s Polling on %d fds: ", __func__, num_fds);
    for (i = 0; i < PP_HW_TYPE_MAX_NUM + 1; i++)
      CDBG("\n Fd = %d | Events = %x", fds[i].fd, fds[i].events);

    timeoutms = poll_cb->data.timeoutms;
    rc = poll(fds, num_fds, timeoutms);
    CDBG("%s rc = %d num_fds = %d timeout = %d\n", __func__, rc,
      num_fds, timeoutms);
    if (rc > 0) {
      /* Events on pipe between mctl thread - mctl pp thread */
      if ((fds[0].revents & POLLIN) && (fds[0].revents & POLLRDNORM))
        mctl_pp_read_pipe(poll_cb, poll_cb->data.pfds[0]);

      for (i = 1; i < PP_HW_TYPE_MAX_NUM; i++)
        if ((fds[i].revents & POLLIN) &&
            (fds[i].revents & POLLRDNORM))
            mctl_pp_read_pipe(poll_cb, fds[i].fd);

    } else {
      usleep(10);
      continue;
    }
  } while (!poll_cb->data.release);

  return NULL;
}

static void *mctl_pp_poll_thread(void *data)
{
  int rc = 0;
  int i;
  void *ret = NULL;
  mctl_pp_t *poll_cb = data;

  mctl_pp_poll_sig_done(poll_cb);
  ret = mctl_pp_poll_fn(poll_cb);

  return ret;
}

int mctl_pp_get_free_pipeline(void *cctrl, int *free_idx)
{
  int rc = -EINVAL, i;
  mctl_config_ctrl_t *ctrl = ( mctl_config_ctrl_t *)cctrl;

  if (free_idx) {
    *free_idx = -1;
    for (i = 0; i < MCTL_MAX_PARALLEL_PP_PIPES; i++) {
      if (!ctrl->mctl_pp_ctrl[i].data.used) {
        CDBG("%s Found free pipeline idx %d ", __func__, i);
        ctrl->mctl_pp_ctrl[i].data.used = 1;
        ctrl->mctl_pp_ctrl[i].pp_idx = i;
        *free_idx = i;
        rc = 0;
        break;
      }
    }
  }
  if (!rc)
    CDBG("%s Got free pipeline %d ", __func__, *free_idx);
  else
    CDBG("%s No free pp pipeline available ", __func__);

  return rc;
}

int mctl_pp_put_free_pipeline(void *cctrl, int idx)
{
  int i, rc = -EINVAL;
  mctl_config_ctrl_t *ctrl = ( mctl_config_ctrl_t *)cctrl;

  if (idx < 0 || idx > MCTL_MAX_PARALLEL_PP_PIPES) {
    CDBG_ERROR("%s Invalid input idx %d ", __func__, idx);
    return rc;
  }
  CDBG("%s pipeline %d ", __func__, idx);
  for (i = 0; i < MCTL_MAX_PARALLEL_PP_PIPES; i++) {
    if (i == idx) {
      /* Make sure the idx is correct and the corresponding
       * pp pipeline is configured. */
      if (ctrl->mctl_pp_ctrl[i].pp_idx == idx) {
        CDBG("%s Put free pipeline idx %d ", __func__, idx);
        ctrl->mctl_pp_ctrl[i].data.used = 0;
        ctrl->mctl_pp_ctrl[i].pp_idx = -1;
        rc = 0;
      } else {
        CDBG_ERROR("%s Pipeline %d not configured ", __func__, idx);
      }
      break;
    }
  }
  return rc;
}

int mctl_pp_launch(mctl_pp_t *poll_cb, void *cfg_ctrl, int input_type)
{
  int rc = 0, i;
  mctl_pp_cmd_t cmd;

  mctl_config_ctrl_t *ctrl = ( mctl_config_ctrl_t *)cfg_ctrl;
  rc = mctl_pp_pipeline_init(poll_cb, cfg_ctrl);
  if (rc < 0) {
    CDBG_ERROR("%s: qcam_pp_init failed", __func__);
    goto end;
  }

  rc = pipe(poll_cb->data.pfds);
  if (rc < 0) {
    CDBG_ERROR("%s: pipe open err=%d, mctl_pp_ctrl=%p", __func__, rc, poll_cb);
    rc = -1;
    goto qcam_pp_deinit;
  }
  poll_cb->input_type = input_type;
  poll_cb->data.timeoutms = 5000; /* 5 seconds */
  pthread_mutex_lock(&poll_cb->mutex);
  poll_cb->status = 0;

  rc = pthread_create(&poll_cb->data.pid, NULL, mctl_pp_poll_thread,
    (void *)poll_cb);
  if (!rc) {
    if (!poll_cb->status)
      pthread_cond_wait(&poll_cb->cond_v, &poll_cb->mutex);
  } else {
    pthread_mutex_unlock(&poll_cb->mutex);
    CDBG_ERROR("%s: mctl pp thread create failed (%s) rc = %d", __func__,
      strerror(errno), rc);
    rc = -rc;
    goto close_pipe;
  }
  pthread_mutex_unlock(&poll_cb->mutex);

  return rc;

close_pipe:
  if (poll_cb->data.pfds[0] > 0) {
    close(poll_cb->data.pfds[0]);
    poll_cb->data.pfds[0] = 0;
  }
  if (poll_cb->data.pfds[1] > 0) {
    close(poll_cb->data.pfds[1]);
    poll_cb->data.pfds[1] = 0;
  }
qcam_pp_deinit:
  pthread_mutex_destroy(&poll_cb->mutex);
  pthread_cond_destroy(&poll_cb->cond_v);
end:
  return rc;
}

int mctl_pp_release(mctl_pp_t *poll_cb)
{
  int rc = 0;
  mctl_pp_cmd_t cmd;

  CDBG("%s: mctl_pp_ctrl=%p\n", __func__, poll_cb);

  memset(&cmd, 0, sizeof(mctl_pp_cmd_t));

  /* All the pp hw should be released by now */
  cmd.cmd_type = QCAM_MCTL_CMD_EXIT;
  mctl_pp_cmd(poll_cb, &cmd);


  if (pthread_join(poll_cb->data.pid, NULL) != 0)
    CDBG("%s: pthread dead already\n", __func__);

  if (poll_cb->data.pfds[0]) {
    close(poll_cb->data.pfds[0]);
    poll_cb->data.pfds[0] = 0;
  }

  if (poll_cb->data.pfds[1]) {
    close(poll_cb->data.pfds[1]);
    poll_cb->data.pfds[1] = 0;
  }

  pthread_mutex_destroy(&poll_cb->mutex);
  pthread_cond_destroy(&poll_cb->cond_v);
  memset(&poll_cb->data, 0, sizeof(poll_cb->data));

  return rc;
}

/*===========================================================================
 * FUNCTION    - mctl_pp_proc_event -
 *
 * DESCRIPTION: process post processing events
 *==========================================================================*/
int8_t mctl_pp_proc_event(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_mctl_pp_event_info *pp_event =
    (struct msm_mctl_pp_event_info *)parm2;
  mctl_pp_cmd_t cmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  CDBG("%s:evt_type=%d, msg_len=%d\n", __func__, MSM_CAM_RESP_MCTL_PP_EVENT,
    (int)sizeof(struct msm_mctl_pp_event_info));
  memset(&cmd, 0, sizeof(cmd));
  cmd.cmd_type = QCAM_MCTL_CMD_DATA;
  cmd.evt_type = MSM_CAM_RESP_MCTL_PP_EVENT;
  memcpy(&cmd.pp_event, pp_event, sizeof(struct msm_mctl_pp_event_info));

  if (pipeline_idx >=0 ) {
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);
    rc = TRUE;
  } else {
    CDBG_ERROR("%s Default pp pipeline is closed ", __func__);
  }
  return rc;
}

static int mctl_pp_frame_done_ex(mctl_pp_t *poll_cb, struct msm_pp_frame *frame)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;

  rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_PICT_PP_DIVERT_DONE, frame);
  return rc;
}

int mctl_pp_divert_done(void *parent, struct msm_cam_evt_divert_frame *div_frame)
{
  int rc = 0;
  mctl_pp_t *poll_cb = parent;
  mctl_config_ctrl_t *ctrl = poll_cb->data.cfg_ctrl;
  struct msm_pp_frame tmp_frame = div_frame->frame;

  /* for two output solution, need to send preview frame
     to face detection/eztune.
     We do not support FD in camcorder yet.
     use videoHint to filter out camcorder case */
  if(!ctrl->videoHint &&
     div_frame->image_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW)
    mctl_divert_frame_done_check_eztune_fd(ctrl, div_frame);

  rc = mctl_pp_frame_done_ex(poll_cb, &tmp_frame);

  CDBG("%s: frame id= %d, image path %d rc = %d", __func__,
    (uint32_t)tmp_frame.frame_id, tmp_frame.path, rc);
  return rc;
}

int mctl_pp_cache_ops(struct ion_flush_data *cache_data, int type,
                      int ion_dev_fd)
{
  int rc = 0;
  struct ion_custom_data custom_data;

  CDBG("%s: vAddr=0x%x, fd=%d, handle=0x%x, len=0x%x", __func__,
    (unsigned int)cache_data->vaddr, cache_data->fd,
    (unsigned int)cache_data->handle, cache_data->length);

  custom_data.cmd = type;
  custom_data.arg = (unsigned long)cache_data;
  if(ioctl(ion_dev_fd, ION_IOC_CUSTOM, &custom_data) < 0) {
    CDBG_ERROR("%s: Cache ops(0x%x) failed\n", __func__, (uint32_t) type);
    rc = -EPERM;
  } else {
    CDBG("%s: Cache ops(0x%x) success\n", __func__, (uint32_t) type);
  }

  return rc;
}

int mctl_pp_add_poll_fd(mctl_pp_t *poll_cb, struct pollfd *poll_fd,
  int src_idx, int dest_idx)
{
  int i, rc = -EINVAL;

  if (!poll_fd) {
    CDBG_ERROR("%s Invalid argument poll_fd ", __func__);
    return -EINVAL;
  }

  for (i = 0; i < PP_HW_TYPE_MAX_NUM; i++) {
    if (!poll_cb->data.poll_fds[i].in_use) {
      poll_cb->data.poll_fds[i].poll_fd = *poll_fd;
      poll_cb->data.poll_fds[i].src_idx = src_idx;
      poll_cb->data.poll_fds[i].dest_idx = dest_idx;
      poll_cb->data.poll_fds[i].in_use = 1;
      poll_cb->data.num_poll_fds++;
      CDBG("%s Adding fd %d src %d, dest %d ", __func__,
        poll_cb->data.poll_fds[i].poll_fd.fd, src_idx, dest_idx);
      rc = 0;
      break;
    }
  }
  if (rc)
    CDBG_ERROR("%s Cannot find free entry in poll_fds", __func__);

  return rc;
}

int mctl_pp_remove_poll_fd(mctl_pp_t *poll_cb, int src_idx, int dest_idx)
{
  int i, rc = -EINVAL;

  for (i = 0; i < PP_HW_TYPE_MAX_NUM; i++) {
    if ((poll_cb->data.poll_fds[i].src_idx == src_idx)
     && (poll_cb->data.poll_fds[i].dest_idx == dest_idx)) {
      CDBG("%s Removing fd %d src %d, dest %d ", __func__,
        poll_cb->data.poll_fds[i].poll_fd.fd, src_idx, dest_idx);
      poll_cb->data.poll_fds[i].poll_fd.fd = -1;
      poll_cb->data.poll_fds[i].src_idx = -1;
      poll_cb->data.poll_fds[i].dest_idx = -1;
      poll_cb->data.poll_fds[i].in_use = 0;
      poll_cb->data.num_poll_fds--;
      rc = 0;
      break;
    }
  }
  if (rc)
    CDBG_ERROR("%s Cannot find src %d dest %d in poll_fds", __func__,
      src_idx, dest_idx);

  return rc;
}
