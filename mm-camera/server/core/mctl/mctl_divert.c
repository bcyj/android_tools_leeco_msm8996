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
#include <sys/socket.h>
#include <linux/un.h>
#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#include "config_proc.h"
#include "mctl.h"
#include "vpe_api.h"
#include "eztune_preview.h"
#include "mctl_divert.h"
#include "mctl_pp.h"

#define BUFF_SIZE_128 128
#define DUMP_YUV 0

#if 0
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - mctl_divert_set_key -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_divert_set_key(void *cctrl, frame_proc_key_t fp_key)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  switch (fp_key) {
    case FP_PREVIEW_SET:
      if (ctrl->ppkey & PP_PREV)
        return TRUE;
      ctrl->ppkey |= PP_PREV;
      break;
    case FP_PREVIEW_RESET:
      if (ctrl->eztune_preview_flag)
        return TRUE;
      ctrl->ppkey &= ~PP_PREV;
      break;
    case FP_SNAPSHOT_SET:
      if (ctrl->ppkey & PP_SNAP && ctrl->ppkey & PP_THUMB)
        return TRUE;
      ctrl->ppkey |= PP_SNAP;
      ctrl->ppkey |= PP_THUMB;
      break;
    case FP_SNAPSHOT_RESET:
      ctrl->ppkey &= ~PP_SNAP;
      ctrl->ppkey &= ~PP_THUMB;
      break;
    case FP_RESET:
      if (ctrl->eztune_preview_flag)
        return TRUE;
      ctrl->ppkey &= ~PP_SNAP;
      ctrl->ppkey &= ~PP_PREV;
      break;
    default:
      CDBG_ERROR("%s Invalid FP_KEY",__func__);
      return FALSE;
      break;
  }
  CDBG("%s: Sending IOCTL_PICT_PP: PP_KEY = %d %d\n",
    __func__, ctrl->ppkey, fp_key);

  if (ioctl(ctrl->camfd, MSM_CAM_IOCTL_PICT_PP, &(ctrl->ppkey)) < 0) {
    CDBG_ERROR("%s: IOCTL_PICT_PP is failed...\n", __func__);
    return FALSE;
  }
  return TRUE;
} /* mctl_pp_set_key */

static int mctl_divert_send_fd_roi_data(mctl_config_ctrl_t *ctrl,
                                     uint32_t frame_id,
                                     uint8_t idx,
                                     frame_proc_fd_data_t *fd_data)
{
  int rc = 0;
  struct v4l2_event_and_payload evt_and_payload;
  memset(&evt_and_payload, 0, sizeof(evt_and_payload));
  frame_proc_fd_roi_t *roi = &(fd_data->roi[idx]);
  mm_camera_event_t *cam_event = (mm_camera_event_t *)evt_and_payload.evt.u.data;
  evt_and_payload.evt.type = V4L2_EVENT_PRIVATE_START+MSM_CAM_APP_NOTIFY_EVENT;
  cam_event->event_type = MM_CAMERA_EVT_TYPE_INFO;
  cam_event->e.info.event_id = MM_CAMERA_INFO_EVT_ROI;
  cam_event->e.info.e.roi.type = FD_ROI_TYPE_DATA;
  cam_event->e.info.e.roi.d.data.frame_id = frame_id;
  cam_event->e.info.e.roi.d.data.idx = idx;

  cam_event->e.info.e.roi.d.data.face.id = roi->unique_id;
  cam_event->e.info.e.roi.d.data.face.score = (roi->fd_confidence + 5)/ 10;

  // Set face boundary
  memcpy(&(cam_event->e.info.e.roi.d.data.face.face_boundary),
         &(roi->face_boundary),
         sizeof(struct fd_rect_t));
  // Set left eye cetner (PT_POINT_LEFT_EYE = 0)
  cam_event->e.info.e.roi.d.data.face.left_eye_center[0] =
    roi->fp.facePt[0].x;
  cam_event->e.info.e.roi.d.data.face.left_eye_center[1] =
    roi->fp.facePt[0].y;

  // Set right eye center (PT_POINT_RIGHT_EYE = 1)
  cam_event->e.info.e.roi.d.data.face.right_eye_center[0] =
    roi->fp.facePt[1].x;
  cam_event->e.info.e.roi.d.data.face.right_eye_center[1] =
    roi->fp.facePt[1].y;

  // Set mouth center (PT_POINT_MOUTH = 2)
  cam_event->e.info.e.roi.d.data.face.mouth_center[0] =
    roi->fp.facePt[2].x;
  cam_event->e.info.e.roi.d.data.face.mouth_center[1] =
    roi->fp.facePt[2].y;

  // Set smile detection
  cam_event->e.info.e.roi.d.data.face.smile_degree =
    roi->sm.smile_degree;
  cam_event->e.info.e.roi.d.data.face.smile_confidence =
    roi->sm.confidence/10;
  // Set gaze angle
  cam_event->e.info.e.roi.d.data.face.gaze_angle =
    roi->gaze_angle;
  // Set blink detection
  cam_event->e.info.e.roi.d.data.face.blink_detected =
    roi->blink_detected;
  // Set blink detection
  cam_event->e.info.e.roi.d.data.face.updown_dir =
    roi->fp.faceDirectionUpDown/2;
  cam_event->e.info.e.roi.d.data.face.leftright_dir =
    roi->fp.faceDirectionLeftRight/2;
  cam_event->e.info.e.roi.d.data.face.roll_dir =
    roi->fp.faceDirectionRoll/2;
  cam_event->e.info.e.roi.d.data.face.left_blink =
    roi->left_blink/10;
  cam_event->e.info.e.roi.d.data.face.right_blink =
    roi->right_blink/10;
  // Set Gaze detection angle
  cam_event->e.info.e.roi.d.data.face.left_right_gaze =
    roi->left_right_gaze;
  cam_event->e.info.e.roi.d.data.face.top_bottom_gaze =
    roi->top_bottom_gaze;
  // Set face recognition flag
  cam_event->e.info.e.roi.d.data.face.is_face_recognised =
    roi->is_face_recognised;

  if ((rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &evt_and_payload)) < 0)
    CDBG_ERROR("%s: MM_CAMERA_INFO_EVT_ROI failed, rc = %d", __func__, rc);
  return rc;
}

static int mctl_divert_send_fd_roi_hdr(mctl_config_ctrl_t *ctrl, uint32_t frame_id, int16_t num_faces_detected)
{
  int rc = 0;
  struct v4l2_event_and_payload evt_and_payload;

  memset(&evt_and_payload, 0, sizeof(evt_and_payload));
  mm_camera_event_t *cam_event = (mm_camera_event_t *)evt_and_payload.evt.u.data;
  evt_and_payload.evt.type = V4L2_EVENT_PRIVATE_START+MSM_CAM_APP_NOTIFY_EVENT;
  cam_event->event_type = MM_CAMERA_EVT_TYPE_INFO;
  cam_event->e.info.event_id = MM_CAMERA_INFO_EVT_ROI;
  cam_event->e.info.e.roi.type = FD_ROI_TYPE_HEADER;
  cam_event->e.info.e.roi.d.hdr.frame_id = frame_id;
  cam_event->e.info.e.roi.d.hdr.num_face_detected = num_faces_detected;
  if ( (rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &evt_and_payload)) < 0)
    CDBG_ERROR("%s: MM_CAMERA_INFO_EVT_ROI failed, rc = %d", __func__, rc);
  return rc;
}

static void mctl_divert_send_fd_roi_result(mctl_config_ctrl_t *ctrl,
                                     frame_proc_fd_data_t *fd_data)
{
    frame_proc_fd_roi_t *roi;
    int16_t num_faces_detected = fd_data->num_faces_detected;
    int16_t dup_num_faces_detected = 0;
    int i;
    int rc = 0;

    if (num_faces_detected > MAX_ROI)
      num_faces_detected = MAX_ROI;
    dup_num_faces_detected = num_faces_detected;
    //validate number of faces detected based on face boundaries
    for (i = 0; i < num_faces_detected; i++)
    {
      roi = &(fd_data->roi[i]);
      if(roi->face_boundary.x == 0 && roi->face_boundary.y == 0
        && roi->face_boundary.dx == 0 && roi->face_boundary.dy ==0)
           num_faces_detected--;
    }
    fd_data->num_faces_detected = num_faces_detected;
    /* Send event of face detection header. */
    rc = mctl_divert_send_fd_roi_hdr(
      ctrl,
      fd_data->frame_id,
      num_faces_detected);
    if (rc == 0) {
      for (i = 0; i < dup_num_faces_detected; i++) {
        roi = &(fd_data->roi[i]);
        if (roi->face_boundary.x != 0 && roi->face_boundary.y != 0
          && roi->face_boundary.dx != 0 && roi->face_boundary.dy !=0)
        /* Send event of face detection roi data. */
            rc = mctl_divert_send_fd_roi_data(
              ctrl,
              fd_data->frame_id,
              i,
              fd_data);
      }
    } else
      CDBG_ERROR("%s: error, rc = %d",  __func__,  rc);
}

static void mctl_divert_fill_input_info(mctl_config_ctrl_t *ctrl)
{
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  sensor_get_t sensor_get;
  stats_proc_get_t sp_get_param;

  /* Sensor info */
  if (ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_CAMIF_CFG, &sensor_get, sizeof(sensor_get)) < 0)
    CDBG_ERROR("%s: sensor_get_params failed\n", __func__);

  fp_ctrl->intf.input.sensor_info.max_preview_fps =
    ctrl->stats_proc_ctrl.intf.input.sensor_info.max_preview_fps;
  fp_ctrl->intf.input.sensor_info.preview_fps =
    ctrl->stats_proc_ctrl.intf.input.sensor_info.preview_fps;

  fp_ctrl->intf.input.isp_info.afd_zoom_xScale =
    (sensor_get.data.camif_setting.last_pixel -
    sensor_get.data.camif_setting.first_pixel + 1) * Q12 /
    ctrl->dimInfo.orig_picture_dx;
  fp_ctrl->intf.input.isp_info.afd_zoom_yScale =
    (sensor_get.data.camif_setting.last_line -
    sensor_get.data.camif_setting.first_line + 1) * Q12 /
    ctrl->dimInfo.orig_picture_dy;

  /* Misc info */
  fp_ctrl->intf.input.mctl_info.crop_factor = ctrl->zoomCtrl.resize_factor;
  fp_ctrl->intf.input.mctl_info.display_dim.width = ctrl->dimInfo.display_width;
  fp_ctrl->intf.input.mctl_info.display_dim.height = ctrl->dimInfo.display_height;

  /* Stats proc info */
  fp_ctrl->intf.input.statsproc_info.aec_d.band_50hz_gap =
    ctrl->stats_proc_ctrl.intf.output.aec_d.band_50hz_gap;
  fp_ctrl->intf.input.statsproc_info.aec_d.max_line_cnt =
    ctrl->stats_proc_ctrl.intf.output.aec_d.max_line_cnt;
  fp_ctrl->intf.input.statsproc_info.aec_d.cur_line_cnt =
    ctrl->stats_proc_ctrl.intf.output.aec_d.cur_line_cnt;
  fp_ctrl->intf.input.statsproc_info.aec_d.aec_settled =
    ctrl->stats_proc_ctrl.intf.output.aec_d.aec_settled;
  fp_ctrl->intf.input.statsproc_info.aec_d.lux_idx =
    ctrl->stats_proc_ctrl.intf.output.aec_d.lux_idx;
}

void mctl_divert_frame_done_check_eztune_fd(
  mctl_config_ctrl_t *ctrl, struct msm_cam_evt_divert_frame *div_frame)
{
  int rc = 0, i;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  int ops_mode = 0, found = 0;
  struct v4l2_control s_ctrl;
  pp_parms postproc;
  mctl_pp_buf_info_t *map_buf_info = NULL;

  if (!ctrl->eztune_preview_flag && !ctrl->is_fd_on)
    return;

  map_buf_info = &ctrl->video_ctrl.user_buf_info[div_frame->image_mode];
  if (div_frame->frame.num_planes == 1) {
    div_frame->frame.sp.vaddr =
      (uint32_t)map_buf_info->local_frame_info[div_frame->frame.buf_idx].local_vaddr +
      div_frame->frame.sp.addr_offset;
  } else if (div_frame->frame.num_planes > 1) {
    div_frame->frame.mp[0].vaddr =
      (uint32_t)map_buf_info->local_frame_info[div_frame->frame.buf_idx].local_vaddr +
      div_frame->frame.mp[0].addr_offset;
    div_frame->frame.mp[1].vaddr =
      (uint32_t)map_buf_info->local_frame_info[div_frame->frame.buf_idx].local_vaddr +
      div_frame->frame.mp[1].addr_offset;
  } else {
    CDBG_ERROR("%s: Buf num_planes (%d) is invalid",
      __func__, div_frame->frame.num_planes);
    return;
  }

  if (ctrl->eztune_preview_flag == 1) {
    eztune_copy_preview_frame(&div_frame->frame);
  }
  if(ctrl->is_fd_on) {
    memcpy(&(fp_ctrl->intf.input.mctl_info.frame), &div_frame->frame,
      sizeof(struct msm_pp_frame));

    mctl_divert_fill_input_info(ctrl);
    if( ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
      ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
      fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0)
      CDBG_ERROR("%s Error executing frame proc for preview ", __func__);

    if (fp_ctrl->intf.output.fd_d.fd_enable) {
      mctl_divert_send_fd_roi_result(ctrl, &(fp_ctrl->intf.output.fd_d));

      stats_proc_set_t set_param;
      set_param.type = STATS_PROC_AEC_TYPE;
      set_param.d.set_aec.type = AEC_SET_FD_ROI;
      set_param.d.set_aec.d.fd_roi.frm_width =ctrl->dimInfo.display_width;
      set_param.d.set_aec.d.fd_roi.frm_height =ctrl->dimInfo.display_height;
      set_param.d.set_aec.d.fd_roi.num_roi =
        fp_ctrl->intf.output.fd_d.num_faces_detected;
      set_param.d.set_aec.d.fd_roi.frm_id =
        fp_ctrl->intf.output.fd_d.frame_id;

      for (i = 0; i < (int)set_param.d.set_aec.d.fd_roi.num_roi ; i++) {
        set_param.d.set_aec.d.fd_roi.roi[i].x =
          fp_ctrl->intf.output.fd_d.roi[i].face_boundary.x;
        set_param.d.set_aec.d.fd_roi.roi[i].y =
          fp_ctrl->intf.output.fd_d.roi[i].face_boundary.y;
        set_param.d.set_aec.d.fd_roi.roi[i].dx =
          fp_ctrl->intf.output.fd_d.roi[i].face_boundary.dx;
        set_param.d.set_aec.d.fd_roi.roi[i].dy =
          fp_ctrl->intf.output.fd_d.roi[i].face_boundary.dy;
        set_param.d.set_aec.d.fd_roi.hist[i].bin =
          &(fp_ctrl->intf.output.fd_d.roi[i].histogram.bin[0]);
        set_param.d.set_aec.d.fd_roi.hist[i].roi_pixels =
          fp_ctrl->intf.output.fd_d.roi[i].histogram.num_samples;
      }
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
        set_param.type, &set_param, &(sp_ctrl->intf));
    }
  }
}

static int mctl_divert_adjust_crop( mctl_config_ctrl_t *ctrl)
{
  struct v4l2_crop *v4l2_crp = &ctrl->crop_info;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  frame_proc_set_t fp_set_param;
  if(v4l2_crp->c.width==0 && v4l2_crp->c.height== 0)
    return 0;
  fp_set_param.type = FRAME_PROC_SHARE;
  fp_set_param.d.set_share.type = FRAME_PROC_ADJUST_ZOOM;
  fp_set_param.d.set_share.d.zoom_crp.x = v4l2_crp->c.left;
  fp_set_param.d.set_share.d.zoom_crp.y  = v4l2_crp->c.top;
  fp_set_param.d.set_share.d.zoom_crp.dx = v4l2_crp->c.width;
  fp_set_param.d.set_share.d.zoom_crp.dy = v4l2_crp->c.height;
  if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, fp_set_param.type,
    &fp_set_param, &(fp_ctrl->intf))<0 ){
    CDBG_ERROR("%s Error while adjusting FD coordinates", __func__);
    return -1;
  }
  return 0;
}

static int dump_frames(frame_proc_ctrl_t *fp_ctrl, int i)
{
  frame_proc_mctl_info_t *fp_mctl_info;
  char buf[BUFF_SIZE_128];
  int koff = 0;
  snprintf(buf, BUFF_SIZE_128, "/data/main_before_%d.yuv", i);
  int main_file_fdt = open(buf, O_RDWR | O_CREAT, 0777);
  if (main_file_fdt < 0) {
    CDBG("%s:cannot open file %s\n", __func__, buf);
  }
  fp_mctl_info = &fp_ctrl->intf.input.mctl_info;
  CDBG("plane 0 pffset %d, plane 1 offset %d\n",
    fp_mctl_info->main_img_frame[i].mp[0].data_offset,
    fp_mctl_info->main_img_frame[i].mp[1].data_offset);
  CDBG("plane 0 vaddr %lx, plane 1 vaddr %lx\n",
    fp_mctl_info->main_img_frame[i].mp[0].vaddr,
    fp_mctl_info->main_img_frame[i].mp[1].vaddr);
  CDBG("plane 0 length %d, plane 1 lenth %d\n",
    fp_mctl_info->main_img_frame[i].mp[0].length,
    fp_mctl_info->main_img_frame[i].mp[1].length);

  write(main_file_fdt, (const void *)
    (fp_mctl_info->main_img_frame[i].mp[0].vaddr +
      fp_mctl_info->main_img_frame[i].mp[0].data_offset),
    fp_mctl_info->picture_dim.width * fp_mctl_info->picture_dim.height);

  koff = lseek(main_file_fdt, (fp_mctl_info->picture_dim.width *
           fp_mctl_info->picture_dim.height), SEEK_SET);

  write(main_file_fdt, (const void *)
    (fp_mctl_info->main_img_frame[i].mp[1].vaddr +
      fp_mctl_info->main_img_frame[i].mp[1].data_offset),
    fp_mctl_info->picture_dim.width * fp_mctl_info->picture_dim.height/2);

  koff = 0;
  snprintf(buf, BUFF_SIZE_128, "/data/thumb_before_%d.yuv", i);
  int thumb_file_fdt = open(buf, O_RDWR | O_CREAT, 0777);
  if (thumb_file_fdt < 0) {
    CDBG("%s:cannot open file %s\n", __func__, buf);
  }
  write(thumb_file_fdt, (const void *)
    (fp_mctl_info->thumb_img_frame[i].mp[0].vaddr +
      fp_mctl_info->thumb_img_frame[i].mp[0].data_offset),
    fp_mctl_info->thumbnail_dim.width * fp_mctl_info->thumbnail_dim.height);

  koff = lseek(thumb_file_fdt, fp_mctl_info->thumbnail_dim.width *
           fp_mctl_info->thumbnail_dim.height, SEEK_SET);

  write(thumb_file_fdt, (const void *)
    (fp_mctl_info->thumb_img_frame[i].mp[1].vaddr +
      fp_mctl_info->thumb_img_frame[i].mp[1].data_offset),
    fp_mctl_info->thumbnail_dim.width * fp_mctl_info->thumbnail_dim.height/2);

  close(main_file_fdt);
  close(thumb_file_fdt);
  return 0;
}

static int mctl_divert_find_buf(mctl_config_ctrl_t *ctrl,
        struct msm_cam_evt_divert_frame *div_frame)
{
  int i;

  mctl_pp_local_buf_info_t* user_frame =
    &ctrl->video_ctrl.user_buf_info[div_frame->image_mode].
    local_frame_info[div_frame->frame.buf_idx];
  mctl_pp_local_buf_info_t* mctl_frame =
    &ctrl->video_ctrl.mctl_buf_info[div_frame->image_mode].
    local_frame_info[div_frame->frame.buf_idx];

  if (div_frame->frame.node_type == VIDEO_NODE) {
    if (user_frame->local_vaddr) {
      if (div_frame->frame.num_planes == 1) {
        div_frame->frame.sp.vaddr =
          (unsigned long)(user_frame->local_vaddr);
      } else {
        div_frame->frame.mp[0].vaddr =
          (unsigned long)(user_frame->local_vaddr);
        for (i = 1; i < div_frame->frame.num_planes; i++) {
          div_frame->frame.mp[i].vaddr =
            (unsigned long)(user_frame->local_vaddr)
            + div_frame->frame.mp[i-1].length;
        }
      }
    } else{
      CDBG_ERROR("%s: video node buffer has not been registered\n", __func__);
      return -1;
    }
  } else if (div_frame->frame.node_type == MCTL_NODE) {
    if (mctl_frame->local_vaddr) {
      if (div_frame->frame.num_planes == 1) {
        div_frame->frame.sp.vaddr =
          (unsigned long)(mctl_frame->local_vaddr);
      } else {
        div_frame->frame.mp[0].vaddr =
          (unsigned long)(mctl_frame->local_vaddr);
        for (i = 1; i < div_frame->frame.num_planes; i++) {
          div_frame->frame.mp[i].vaddr =
            (unsigned long)(mctl_frame->local_vaddr)
            + div_frame->frame.mp[i-1].length;
        }
      }
    } else{
      CDBG_ERROR("%s: mctl node buffer has not been registered\n", __func__);
      return -1;
    }
  } else {
    CDBG_ERROR("%s: Invalid node type for buffer\n", __func__);
    return -1;
  }
  return 0;
}

static void mctl_proc_divert_frame(
  mctl_config_ctrl_t *ctrl,
  struct msm_cam_evt_divert_frame *div_frame)
{
  int rc = FALSE, i;
  int snapshot_divert, preview_divert;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  struct msm_pp_frame frame;
  int ops_mode = 0, found = 0;
  struct v4l2_control s_ctrl;
  pp_parms postproc;
  char filename[128];
  frame_proc_key_t fp_key;

  CDBG("%s: do_pp=%d,path=0x%x, mode=%d, vb = 0x%x, id = 0x%x, ts=0x%x:0x%x\n",
    __func__, div_frame->do_pp, div_frame->frame.path,
    div_frame->image_mode, div_frame->frame.handle, div_frame->frame.frame_id,
    (uint32_t)div_frame->frame.timestamp.tv_sec,
    (uint32_t)div_frame->frame.timestamp.tv_usec);
  memset(&frame, 0x0, sizeof(struct msm_frame));

  if (ctrl->eztune_preview_flag == 1) {
    rc = mctl_divert_find_buf(ctrl, div_frame);
    if(rc < 0) {
      CDBG_ERROR("%s Error finding buffer ", __func__);
      return;
    }
    eztune_copy_preview_frame(&div_frame->frame);
  }

  if (ctrl->eztune_preview_flag == 0) {
    rc = mctl_divert_find_buf(ctrl, div_frame);
    if(rc < 0) {
      CDBG_ERROR("%s Error finding buffer ", __func__);
      return;
    }
  }

  frame = div_frame->frame;
  snapshot_divert = fp_ctrl->intf.output.share_d.divert_snapshot;
  preview_divert = fp_ctrl->intf.output.share_d.divert_preview;

  switch (frame.path) {
    case OUTPUT_TYPE_S:
      if (ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT) {
        if (ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV61 ||
          ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV16)
          fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V1;
        else
          fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V2;
        fp_ctrl->intf.input.mctl_info.picture_dim.width =
          ctrl->dimInfo.picture_width;
        fp_ctrl->intf.input.mctl_info.picture_dim.height =
          ctrl->dimInfo.picture_height;
        if (ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV61 ||
          ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV16)
          fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V1;
        else
          fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V2;
        fp_ctrl->intf.input.mctl_info.thumbnail_dim.width =
          ctrl->dimInfo.ui_thumbnail_width;
        fp_ctrl->intf.input.mctl_info.thumbnail_dim.height =
          ctrl->dimInfo.ui_thumbnail_height;
        if (ctrl->hdrCtrl.hdr_enable || ctrl->hdrCtrl.exp_bracketing_enable ) {
          memcpy(&(fp_ctrl->intf.input.mctl_info.main_img_frame[ctrl->hdrCtrl.hdr_main_divert_count]),
              &frame, sizeof(struct msm_pp_frame));
          ctrl->hdrCtrl.hdr_main_frame[ctrl->hdrCtrl.hdr_main_divert_count] = *div_frame;
          ctrl->hdrCtrl.hdr_main_divert_count++;
          CDBG("%s: frame id = %d, num planes %d, Cookie %x \n", __func__,
              div_frame->frame.frame_id, div_frame->frame.num_planes,
              (uint32_t)div_frame->frame.handle);
          if (ctrl->hdrCtrl.hdr_main_divert_count < ctrl->hdrCtrl.total_frames
              || ctrl->hdrCtrl.hdr_thumb_divert_count < ctrl->hdrCtrl.total_frames)
            return;
          fp_ctrl->intf.input.mctl_info.num_main_img = ctrl->hdrCtrl.hdr_main_divert_count;
          fp_ctrl->intf.input.mctl_info.num_thumb_img = ctrl->hdrCtrl.hdr_thumb_divert_count;
        } else {
          memcpy(&(fp_ctrl->intf.input.mctl_info.main_img_frame[0]),
              &frame, sizeof(struct msm_pp_frame));
          CDBG("%s: frame id = %d, num planes %d, Cookie %x \n", __func__,
              div_frame->frame.frame_id, div_frame->frame.num_planes,
              (uint32_t)div_frame->frame.handle);
          fp_ctrl->intf.input.mctl_info.num_main_img = 1;
          fp_ctrl->intf.input.mctl_info.num_thumb_img = 0;
        }
        if( ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
          ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
          fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0)
          CDBG_ERROR("%s Error executing frame proc for snapshot ", __func__);
      } else {
        CDBG_HIGH("%s RAW frame diverted. Return it back to kernel ", __func__);
      }
      break;
      case OUTPUT_TYPE_T:
      if (ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT) {
        if (ctrl->hdrCtrl.hdr_enable || ctrl->hdrCtrl.exp_bracketing_enable ) {
          memcpy(&(fp_ctrl->intf.input.mctl_info.thumb_img_frame[ctrl->hdrCtrl.hdr_thumb_divert_count]),
          &frame, sizeof(struct msm_pp_frame));
          ctrl->hdrCtrl.hdr_thumb_frame[ctrl->hdrCtrl.hdr_thumb_divert_count] = *div_frame;
          ctrl->hdrCtrl.hdr_thumb_divert_count++;
          CDBG("%s: frame id = %d, num planes %d, Cookie %x \n", __func__,
             div_frame->frame.frame_id, div_frame->frame.num_planes,
             (uint32_t)div_frame->frame.handle);
          if (ctrl->hdrCtrl.hdr_main_divert_count < ctrl->hdrCtrl.total_frames
              || ctrl->hdrCtrl.hdr_thumb_divert_count < ctrl->hdrCtrl.total_frames)
            return;
          fp_ctrl->intf.input.mctl_info.num_main_img = ctrl->hdrCtrl.hdr_main_divert_count;
          fp_ctrl->intf.input.mctl_info.num_thumb_img = ctrl->hdrCtrl.hdr_thumb_divert_count;
        } else {
          memcpy(&(fp_ctrl->intf.input.mctl_info.thumb_img_frame[0]),
            &frame, sizeof(struct msm_pp_frame));
          CDBG("%s: frame id = %d, num planes %d, Cookie %x \n", __func__,
            div_frame->frame.frame_id, div_frame->frame.num_planes,
            (uint32_t)div_frame->frame.handle);
          fp_ctrl->intf.input.mctl_info.num_thumb_img = 1;
          fp_ctrl->intf.input.mctl_info.num_main_img = 0;
        }
        if (ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV61 ||
        ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV16)
          fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V1;
        else
          fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V2;
        fp_ctrl->intf.input.mctl_info.picture_dim.width =
          ctrl->dimInfo.picture_width;
        fp_ctrl->intf.input.mctl_info.picture_dim.height =
          ctrl->dimInfo.picture_height;
        if (ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV61 ||
          ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV16)
          fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V1;
        else
          fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V2;
        fp_ctrl->intf.input.mctl_info.thumbnail_dim.width =
          ctrl->dimInfo.ui_thumbnail_width;
        fp_ctrl->intf.input.mctl_info.thumbnail_dim.height =
          ctrl->dimInfo.ui_thumbnail_height;
#if DUMP_YUV
        for (i = 0; i < (int)ctrl->hdrCtrl.hdr_main_divert_count; i++) {
         dump_frames(fp_ctrl, i);
        }
#endif
      if( ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
        ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
        fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0)
          CDBG_ERROR("%s Error executing frame proc for snapshot ", __func__);
      } else {
          CDBG_HIGH("%s RAW frame diverted. Return it back to kernel ", __func__);
        }
      break;
    case OUTPUT_TYPE_P:
      if(fp_ctrl->intf.output.fd_d.fd_enable &&
        ctrl->video_ctrl.op_mode != MSM_V4L2_CAM_OP_ZSL
        && fp_ctrl->intf.output.fd_d.fd_skip_cnt++ % 3 == 0)
        break;
      memcpy(&(fp_ctrl->intf.input.mctl_info.frame), &frame,
        sizeof(struct msm_pp_frame));

      mctl_divert_fill_input_info(ctrl);
      if( ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
        ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
        fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0)
        CDBG_ERROR("%s Error executing frame proc for preview ", __func__);

#ifdef VFE_2X
        if (fp_ctrl->intf.output.afd_d.afd_enable) {
          stats_proc_set_t set_param;
          /* Send the afd data to stats proc module. */
          set_param.type = STATS_PROC_AEC_TYPE;
          set_param.d.set_aec.type = AEC_ANTIBANDING;
          set_param.d.set_aec.d.aec_atb =
            fp_ctrl->intf.output.afd_d.afd_antibanding_type;
          rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
            set_param.type, &set_param, &(sp_ctrl->intf));
          if (rc) {
            CDBG_ERROR("Failed to set AEC_SET_FD_ROI");
          }
          set_param.type = STATS_PROC_AEC_TYPE;
          set_param.d.set_aec.type = AEC_ANTIBANDING_STATUS;
          set_param.d.set_aec.d.aec_atb_status =
            fp_ctrl->intf.output.afd_d.afd_status;
          rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
            set_param.type, &set_param, &(sp_ctrl->intf));
          if (rc) {
            CDBG_ERROR("Failed to set AEC_SET_FD_ROI");
          }
        }
#endif

      if (fp_ctrl->intf.output.fd_d.fd_enable) {
        mctl_divert_adjust_crop(ctrl);
        mctl_divert_send_fd_roi_result(ctrl, &(fp_ctrl->intf.output.fd_d));
        CDBG("%s: mctl_divert_send_fd_roi_result", __func__);
        if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
          stats_proc_set_t set_param;
          set_param.type = STATS_PROC_AEC_TYPE;
          set_param.d.set_aec.type = AEC_SET_FD_ROI;
          set_param.d.set_aec.d.fd_roi.frm_width =ctrl->dimInfo.display_width;
          set_param.d.set_aec.d.fd_roi.frm_height =ctrl->dimInfo.display_height;
          set_param.d.set_aec.d.fd_roi.num_roi =
            fp_ctrl->intf.output.fd_d.num_faces_detected;
          set_param.d.set_aec.d.fd_roi.frm_id =
            fp_ctrl->intf.output.fd_d.frame_id;
          for (i = 0; i < (int)set_param.d.set_aec.d.fd_roi.num_roi ; i++) {
            set_param.d.set_aec.d.fd_roi.roi[i].x =
              fp_ctrl->intf.output.fd_d.roi[i].face_boundary.x;
            set_param.d.set_aec.d.fd_roi.roi[i].y =
              fp_ctrl->intf.output.fd_d.roi[i].face_boundary.y;
            set_param.d.set_aec.d.fd_roi.roi[i].dx =
              fp_ctrl->intf.output.fd_d.roi[i].face_boundary.dx;
            set_param.d.set_aec.d.fd_roi.roi[i].dy =
              fp_ctrl->intf.output.fd_d.roi[i].face_boundary.dy;
            set_param.d.set_aec.d.fd_roi.hist[i].bin =
              &(fp_ctrl->intf.output.fd_d.roi[i].histogram.bin[0]);
            set_param.d.set_aec.d.fd_roi.hist[i].roi_pixels =
              fp_ctrl->intf.output.fd_d.roi[i].histogram.num_samples;
          }
          rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
            set_param.type, &set_param, &(sp_ctrl->intf));
          if (rc) {
            CDBG_ERROR("Failed to set AEC_SET_FD_ROI");
          }
        }
      }
      break;
    default:
      CDBG("%s Default: Return back the frame %d ", __func__, frame.path);
      break;
  }
  if (fp_ctrl->intf.output.share_d.divert_snapshot != snapshot_divert
    && ctrl->video_ctrl.op_mode != MSM_V4L2_CAM_OP_ZSL) {
    if (fp_ctrl->intf.output.share_d.divert_snapshot)
      fp_key = FP_SNAPSHOT_SET;
    else
      fp_key = FP_SNAPSHOT_RESET;

    rc = mctl_divert_set_key(ctrl, fp_key);
    if (!rc)
      CDBG_ERROR("%s Error setting SNAPSHOT KEY", __func__);
  }

  if (fp_ctrl->intf.output.share_d.divert_preview != preview_divert) {
    if (fp_ctrl->intf.output.share_d.divert_preview)
      fp_key = FP_PREVIEW_SET;
    else
      fp_key = FP_PREVIEW_RESET;

    rc = mctl_divert_set_key(ctrl, fp_key);
    if (!rc)
      CDBG_ERROR("%s Error setting PREVIEW KEY", __func__);
  }
  if ((ctrl->hdrCtrl.hdr_enable || ctrl->hdrCtrl.exp_bracketing_enable)
      && (ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT)) {
    for (i = 0; i < (int)ctrl->hdrCtrl.total_frames; i++) {
      rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_PICT_PP_DIVERT_DONE, &ctrl->hdrCtrl.hdr_main_frame[i].frame);
      rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_PICT_PP_DIVERT_DONE, &ctrl->hdrCtrl.hdr_thumb_frame[i].frame);
    }
  } else {
    /* Release the frame back to the kernel */
    rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_PICT_PP_DONE, &frame);
  }
  CDBG("%s: End", __func__);
} /* mctl_proc_divert_frame */

/*===========================================================================
 * FUNCTION    - mctl_divert_frame -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t mctl_divert_frame(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_cam_evt_divert_frame *div_frame =
    (struct msm_cam_evt_divert_frame *)parm2;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  CDBG("%s:msg_len=%d,phy=0x%x,length=%d,image_mode=%d\n", __func__,
    (int)sizeof(struct msm_cam_evt_divert_frame),
    (uint32_t)div_frame->frame.sp.phy_addr, div_frame->frame.sp.length,
    div_frame->image_mode);
  if (div_frame->do_pp) {
    mctl_pp_cmd_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.cmd_type = QCAM_MCTL_CMD_DATA;
    cmd.evt_type = MSM_CAM_RESP_DIV_FRAME_EVT_MSG;
    memcpy(&cmd.div_frame, div_frame, sizeof(struct msm_cam_evt_divert_frame));
    if (pipeline_idx >= 0)
      mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);
    else
      CDBG_ERROR("%s Default pipeline closed. Cannot divert frame ", __func__);
  } else {
    mctl_proc_divert_frame(ctrl, div_frame);
  }
  return TRUE;
}

int mctl_divert_socket_recvmsg(int fd, mctl_config_ctrl_t *ctrl)
{
    struct msghdr msgh;
    struct iovec iov[1];
    struct cmsghdr *cmsghp = NULL;
    char control[CMSG_SPACE(sizeof(int))];
    int rcvd_fd = -1;
    int rcvd_len = 0;
    cam_sock_packet_t *buf_packet = &ctrl->video_ctrl.socket_info.buf_packet;

    memset(&msgh, 0, sizeof(msgh));
    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;
    msgh.msg_control = control;
    msgh.msg_controllen = sizeof(control);

    memset(buf_packet, 0, sizeof(cam_sock_packet_t));
    iov[0].iov_base = buf_packet;
    iov[0].iov_len = sizeof(cam_sock_packet_t);
    msgh.msg_iov = iov;
    msgh.msg_iovlen = 1;

    if ((rcvd_len = recvmsg(fd, &(msgh), 0)) <= 0) {
      CDBG_ERROR(" %s: recvmsg failed", __func__);
      return rcvd_len;
    }

    CDBG("%s:  msg_ctrl %p len %d", __func__, msgh.msg_control,
      msgh.msg_controllen);

    if(((cmsghp = CMSG_FIRSTHDR(&msgh)) != NULL) &&
      (cmsghp->cmsg_len == CMSG_LEN(sizeof(int)))) {
      if (cmsghp->cmsg_level == SOL_SOCKET &&
        cmsghp->cmsg_type == SCM_RIGHTS) {
        CDBG("%s:  CtrlMsg is valid", __func__);
        rcvd_fd = *((int *) CMSG_DATA(cmsghp));
        CDBG("%s:  Receieved fd=%d", __func__, rcvd_fd);
      } else {
        CDBG_ERROR("%s: Unexpected Control Msg", __func__);
      }
    }

    CDBG_HIGH("%s: Receieved msg_type=%d", __func__, buf_packet->msg_type);
    if (buf_packet->msg_type == CAM_SOCK_MSG_TYPE_FD_MAPPING ||
      buf_packet->msg_type == CAM_SOCK_MSG_TYPE_HIST_MAPPING) {
      buf_packet->payload.frame_fd_map.fd = rcvd_fd;
    }

    return rcvd_len;
}

static void mctl_divert_domain_socket_reg_hist_buf(mctl_config_ctrl_t *ctrl, mm_camera_frame_map_type *map_buf)
{
  mctl_pp_buf_info_t *map_buf_info = NULL;

  map_buf_info = &ctrl->video_ctrl.user_hist_buf_info;
  if(map_buf_info->remote_frame_info[map_buf->frame_idx].fd > 0) {
    CDBG("%s: index in use. idx = %d", __func__, map_buf->frame_idx);
    return;
  }
  map_buf_info->remote_frame_info[map_buf->frame_idx] = *map_buf;
  map_buf_info->local_frame_info[map_buf->frame_idx].fd = map_buf->fd;
  map_buf_info->local_frame_info[map_buf->frame_idx].local_vaddr = mmap(NULL,
    map_buf_info->remote_frame_info[map_buf->frame_idx].size,
    PROT_READ|PROT_WRITE, MAP_SHARED, map_buf_info->local_frame_info[map_buf->frame_idx].fd, 0);
  if (map_buf_info->local_frame_info[map_buf->frame_idx].local_vaddr == MAP_FAILED){
    CDBG_ERROR("%s:  mmap failed\n", __func__);
    map_buf_info->local_frame_info[map_buf->frame_idx].local_vaddr = NULL;
    map_buf_info->local_frame_info[map_buf->frame_idx].fd = 0;
    memset(&map_buf_info->remote_frame_info[map_buf->frame_idx], 0,
           sizeof(mm_camera_frame_map_type));
  } else {
    CDBG("%s:  mmap fd(%d), vaddr(%p)\n", __func__,
               map_buf_info->local_frame_info[map_buf->frame_idx].fd, map_buf_info->local_frame_info[map_buf->frame_idx].local_vaddr);
    map_buf_info->active_mapping_count++;
  }
} /* mctl_pp_reg_buf */

static void mctl_divert_domain_socket_unreg_hist_buf(mctl_config_ctrl_t *ctrl, mm_camera_frame_unmap_type *buf_unmap)
{
  mctl_pp_buf_info_t *unmap_buf_info = NULL;

  unmap_buf_info = &ctrl->video_ctrl.user_hist_buf_info;
  if(unmap_buf_info->local_frame_info[buf_unmap->frame_idx].fd > 0) {
    munmap((void *)unmap_buf_info->local_frame_info[buf_unmap->frame_idx].local_vaddr,
           unmap_buf_info->remote_frame_info[buf_unmap->frame_idx].size);
    close(unmap_buf_info->local_frame_info[buf_unmap->frame_idx].fd);
    unmap_buf_info->active_mapping_count--;
    unmap_buf_info->local_frame_info[buf_unmap->frame_idx].local_vaddr = NULL;
    unmap_buf_info->local_frame_info[buf_unmap->frame_idx].fd = 0;
    memset(&unmap_buf_info->remote_frame_info[buf_unmap->frame_idx], 0,
           sizeof(mm_camera_frame_map_type));
  }
} /* mctl_pp_unreg_buf */

static void mctl_divert_domain_socket_reg_buf(mctl_config_ctrl_t *ctrl,
  mm_camera_frame_map_type *map_buf)
{
  mctl_pp_buf_info_t *map_buf_info = NULL;
  mm_camera_frame_map_type *remote_frame_info = NULL;
  mctl_pp_local_buf_info_t *local_frame_info = NULL;

  CDBG("%s: map_buf->ext_mode = %d", __func__, map_buf->ext_mode);
  if(map_buf->ext_mode >= MSM_V4L2_EXT_CAPTURE_MODE_MAX) {
    CDBG_ERROR("%s: invalid ext_mode %d", __func__, map_buf->ext_mode);
    return;
  }

  map_buf_info = &ctrl->video_ctrl.user_buf_info[map_buf->ext_mode];
  remote_frame_info = &map_buf_info->remote_frame_info[map_buf->frame_idx];
  local_frame_info = &map_buf_info->local_frame_info[map_buf->frame_idx];

  if (remote_frame_info->fd > 0) {
    CDBG_HIGH("%s: index in use. image_mode = %d, idx = %d", __func__,
      map_buf->ext_mode, map_buf->frame_idx);
    return;
  }

  *remote_frame_info = *map_buf;
  local_frame_info->fd = map_buf->fd;
  local_frame_info->local_vaddr = mmap(NULL, remote_frame_info->size,
    PROT_READ | PROT_WRITE, MAP_SHARED, local_frame_info->fd, 0);

  if (local_frame_info->local_vaddr == MAP_FAILED) {
    CDBG_ERROR("%s: mmap failed\n", __func__);
    local_frame_info->local_vaddr = NULL;
    local_frame_info->fd = 0;
    memset(remote_frame_info, 0, sizeof(mm_camera_frame_map_type));
  } else {
    CDBG("%s: mmap fd(%d), vaddr(%p)\n", __func__, local_frame_info->fd,
      local_frame_info->local_vaddr);
    map_buf_info->active_mapping_count++;
  }
  CDBG("%s: Mapped fd = %d image mode %d, buf_idx %d", __func__,
    local_frame_info->fd, map_buf->ext_mode, map_buf->frame_idx);
} /* mctl_pp_reg_buf */

static void mctl_divert_domain_socket_unreg_buf(mctl_config_ctrl_t *ctrl,
  mm_camera_frame_unmap_type *buf_unmap)
{
  mctl_pp_buf_info_t *unmap_buf_info = NULL;
  mm_camera_frame_map_type *remote_frame_info = NULL;
  mctl_pp_local_buf_info_t *local_frame_info = NULL;

  CDBG("%s: buf unmap->ext_mode = %d", __func__, buf_unmap->ext_mode);
  if (buf_unmap->ext_mode >= MSM_V4L2_EXT_CAPTURE_MODE_MAX) {
    CDBG_ERROR("%s: Invalid ext_mode = %d", __func__, buf_unmap->ext_mode);
    return;
  }

  unmap_buf_info = &ctrl->video_ctrl.user_buf_info[buf_unmap->ext_mode];
  remote_frame_info = &unmap_buf_info->remote_frame_info[buf_unmap->frame_idx];
  local_frame_info = &unmap_buf_info->local_frame_info[buf_unmap->frame_idx];

  if(local_frame_info->fd > 0) {
    CDBG("%s: munmap idx=%d vaddr=%p fd=%d", __func__, buf_unmap->frame_idx,
      local_frame_info->local_vaddr, local_frame_info->fd);

    munmap((void *)local_frame_info->local_vaddr, remote_frame_info->size);
    close(local_frame_info->fd);
    unmap_buf_info->active_mapping_count--;
    local_frame_info->local_vaddr = NULL;
    local_frame_info->fd = 0;
    memset(remote_frame_info, 0, sizeof(mm_camera_frame_map_type));
  }
} /* mctl_pp_unreg_buf */

static void mctl_divert_send_wdn_done(mctl_config_ctrl_t *ctrl,
  cam_ctrl_status_t status, unsigned long cookie)
{
  mm_camera_event_t *cam_event;
  struct v4l2_event_and_payload v4l2_ev;

  v4l2_ev.payload_length = 0;
  v4l2_ev.transaction_id = -1;
  v4l2_ev.payload = NULL;
  cam_event = (mm_camera_event_t *)v4l2_ev.evt.u.data;
  cam_event->event_type = MM_CAMERA_EVT_TYPE_CTRL;
  cam_event->e.ctrl.evt = MM_CAMERA_CTRL_EVT_WDN_DONE;
  cam_event->e.ctrl.status = status;
  cam_event->e.ctrl.cookie = cookie;
  v4l2_ev.evt.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_APP_NOTIFY_EVENT;
  ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &v4l2_ev);
}

static void mctl_divert_domain_socket_do_wdn(mctl_config_ctrl_t *ctrl,
  mm_camera_wdn_start_type *wdn_start)
{
  struct msm_pp_frame frame;
  int ext_mode;
  int idx, i, rc = 0;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  frame_proc_set_t fp_set_param;
  vfe_pp_params_t vfe_pp_params;
  frame_proc_key_t fp_key;
  int prev_opt_mode;

  ctrl->video_ctrl.wdn_status.wdn_start = *wdn_start;
  prev_opt_mode = fp_ctrl->intf.input.mctl_info.opt_mode;
  fp_ctrl->intf.input.mctl_info.opt_mode = FRAME_PROC_SNAPSHOT;
  fp_key = FP_PREVIEW_RESET;
  rc = mctl_divert_set_key(ctrl, fp_key);
  if (!rc)
    CDBG_ERROR("%s Error setting PREVIEW KEY", __func__);
  fp_ctrl->intf.input.statsproc_info.aec_d.lux_idx
    = ctrl->stats_proc_ctrl.intf.output.aec_d.lux_idx;

  if (ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT) {
    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.snap.real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.snapshot_wb.g_gain;
  }else {
    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.cur_real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.curr_gains.g_gain;
  }
  /* Now get the gamma table info from VFE interface */
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_PP_INFO,
    (void *)&vfe_pp_params, sizeof(vfe_pp_params));
  fp_ctrl->intf.input.isp_info.VFE_GAMMA_NUM_ENTRIES =
    vfe_pp_params.gamma_num_entries;
  fp_ctrl->intf.input.isp_info.RGB_gamma_table =
    vfe_pp_params.gamma_table;
  fp_ctrl->intf.input.isp_info.lumaAdaptationEnable =
    vfe_pp_params.la_enable;
  fp_ctrl->intf.input.isp_info.VFE_LA_TABLE_LENGTH =
    vfe_pp_params.luma_num_entries;
  fp_ctrl->intf.input.isp_info.LA_gamma_table =
    vfe_pp_params.luma_table;

  for(idx = 0; idx < wdn_start->num_frames; idx++) {
    memset(&frame,  0,  sizeof(frame));
    ext_mode = ctrl->video_ctrl.wdn_status.wdn_start.ext_mode[idx];
    switch (ext_mode) {
      case MSM_V4L2_EXT_CAPTURE_MODE_MAIN:
        fp_ctrl->intf.input.mctl_info.picture_dim.width = ctrl->dimInfo.picture_width;
        fp_ctrl->intf.input.mctl_info.picture_dim.height = ctrl->dimInfo.picture_height;
        fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V2;
        frame.image_type = MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
        frame.path = OUTPUT_TYPE_S;
        frame.num_planes = ctrl->dimInfo.picture_frame_offset.num_planes;
        if(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].local_vaddr == NULL){
          fp_ctrl->intf.input.mctl_info.num_main_img = 0;
          continue;
        }
        for(i = 0; i < frame.num_planes; i++) {
          frame.mp[i].data_offset = ctrl->dimInfo.picture_frame_offset.mp[i].offset;
          frame.mp[i].length = ctrl->dimInfo.picture_frame_offset.mp[i].len;
          frame.mp[i].fd = ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].fd;
          if (i > 0)
            frame.mp[i].vaddr = (unsigned long)(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].local_vaddr) + frame.mp[i-1].length;
          else
            frame.mp[i].vaddr = (unsigned long)(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].local_vaddr);
          CDBG("%s (buf,fd,length, offset) = (0x%lu, %d, %d, %d)", __func__, frame.mp[i].vaddr, frame.mp[i].fd, frame.mp[i].length, frame.mp[i].addr_offset);
        }
        memcpy(&(fp_ctrl->intf.input.mctl_info.main_img_frame[0]),
            &frame, sizeof(struct msm_pp_frame));
        fp_ctrl->intf.input.mctl_info.num_main_img = 1;
        break;
      case MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL:
        if (ctrl->video_ctrl.op_mode == MSM_V4L2_CAM_OP_ZSL){
          fp_ctrl->intf.input.mctl_info.thumbnail_dim.width = ctrl->dimInfo.display_width;
          fp_ctrl->intf.input.mctl_info.thumbnail_dim.height = ctrl->dimInfo.display_height;
          frame.num_planes = ctrl->dimInfo.display_frame_offset.num_planes;
        } else {
        fp_ctrl->intf.input.mctl_info.thumbnail_dim.width = ctrl->dimInfo.thumbnail_width;
        fp_ctrl->intf.input.mctl_info.thumbnail_dim.height = ctrl->dimInfo.thumbnail_height;
        frame.num_planes = ctrl->dimInfo.thumb_frame_offset.num_planes;
        }
        fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V2;
        frame.image_type = MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL;
        frame.path = OUTPUT_TYPE_T;
        if(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].local_vaddr == NULL){
          fp_ctrl->intf.input.mctl_info.num_thumb_img = 0;
          continue;
        }
        for(i = 0; i < frame.num_planes; i++) {
          if (ctrl->video_ctrl.op_mode == MSM_V4L2_CAM_OP_ZSL){
            frame.mp[i].data_offset = ctrl->dimInfo.display_frame_offset.mp[i].offset;
            frame.mp[i].length = ctrl->dimInfo.display_frame_offset.mp[i].len;
          } else {
            frame.mp[i].data_offset = ctrl->dimInfo.thumb_frame_offset.mp[i].offset;
            frame.mp[i].length = ctrl->dimInfo.thumb_frame_offset.mp[i].len;
          }
          frame.mp[i].fd = ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].fd;
          frame.mp[i].vaddr = (unsigned long)(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wdn_start->frame_idx[idx]].local_vaddr) + frame.mp[i-1].length;
          CDBG("%s (buf,fd,length, offset) = (0x%lu, %d, %d, %d)", __func__, frame.mp[i].vaddr, frame.mp[i].fd, frame.mp[i].length, frame.mp[i].addr_offset);
        }
        memcpy(&(fp_ctrl->intf.input.mctl_info.thumb_img_frame[0]),
            &frame, sizeof(struct msm_pp_frame));
        fp_ctrl->intf.input.mctl_info.num_thumb_img = 1;
        break;
      default:
        CDBG_ERROR("%s: invalid image mode %d", __func__, ext_mode);
        /* send event back to HAL */
        mctl_divert_send_wdn_done(ctrl, CAM_CTRL_FAILED, wdn_start->cookie);
        return;
    }
  }

  fp_set_param.type = FRAME_PROC_WAVELET_DENOISE;
  fp_set_param.d.set_wd.type = WAVELET_DENOISE_CALIBRATE;
  if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, fp_set_param.type,
    &fp_set_param, &(fp_ctrl->intf))<0 ){
    CDBG_ERROR("%s Error while calibrating Wavelet Denoise", __func__);
    mctl_divert_send_wdn_done(ctrl, CAM_CTRL_FAILED, wdn_start->cookie);
    return;
  }

  if( ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
    fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0){
      CDBG_ERROR("%s Error executing wavelet denoise for snapshot ", __func__);
      /* send event back to HAL */
      mctl_divert_send_wdn_done(ctrl, CAM_CTRL_FAILED, wdn_start->cookie);
  } else {
      CDBG("%s Success processing Wavelet Denoise", __func__);
      /* send event back to HAL */
      mctl_divert_send_wdn_done(ctrl, CAM_CTRL_SUCCESS, wdn_start->cookie);
  }
  if (fp_ctrl->intf.output.fd_d.fd_enable) {
    fp_key = FP_PREVIEW_SET;
    rc = mctl_divert_set_key(ctrl, fp_key);
    if (!rc)
      CDBG_ERROR("%s Error setting PREVIEW KEY", __func__);
  }
  fp_ctrl->intf.input.mctl_info.opt_mode = prev_opt_mode;
}

int create_camfd_receive_socket(mctl_domain_socket_t *socket_info, int cameraId)
{
  struct sockaddr_un addr;
  struct cmsghdr *cmhp;

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, UNIX_PATH_MAX, "/data/cam_socket%d", cameraId);
  CDBG("%s:  domain socket opened with name (%s)\n", __func__, addr.sun_path);

  /* remove the socket path if it already exists, otherwise bind might fail */
  unlink(addr.sun_path);

  socket_info->socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (socket_info->socket_fd == -1) {
    CDBG_ERROR("%s: socket creation failed", __func__);
    return -1;
  }

  if (bind(socket_info->socket_fd, (struct sockaddr *)&addr,
    sizeof(struct sockaddr_un)) == -1) {
    CDBG_ERROR("%s:  socket binding failed\n", __func__);
    close(socket_info->socket_fd);
    return -1;
  }

  return 0;
} /* create_camfd_receive_socket */

void close_camfd_receive_socket(mctl_domain_socket_t *socket_info, int cameraId)
{
  CDBG("%s: cameraId = %d, close domain socket fd = %d",
       __func__, cameraId, socket_info->socket_fd);
  if (socket_info->socket_fd > 0) {
    close(socket_info->socket_fd);
    socket_info->socket_fd = 0;
  }
}

static void mctl_divert_send_hdr_done(mctl_config_ctrl_t *ctrl,
  cam_ctrl_status_t status, unsigned long cookie)
{
  mm_camera_event_t *cam_event;
  struct v4l2_event_and_payload v4l2_ev;

  v4l2_ev.payload_length = 0;
  v4l2_ev.transaction_id = -1;
  v4l2_ev.payload = NULL;
  cam_event = (mm_camera_event_t *)v4l2_ev.evt.u.data;
  cam_event->event_type = MM_CAMERA_EVT_TYPE_CTRL;
  cam_event->e.ctrl.evt = MM_CAMERA_CTRL_EVT_HDR_DONE;
  cam_event->e.ctrl.status = status;
  cam_event->e.ctrl.cookie = cookie;
  v4l2_ev.evt.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_APP_NOTIFY_EVENT;
  ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &v4l2_ev);
}

static void mctl_divert_do_hdr(mctl_config_ctrl_t *ctrl,
  mm_camera_hdr_start_type *hdr_start)
{
  struct msm_pp_frame frame;
  int ext_mode;
  int idx, i, rc = 0;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  frame_proc_set_t fp_set_param;
  mctl_pp_local_buf_info_t *local_buf_info;

  if (!hdr_start) {
    CDBG_ERROR("%s: Invalid hdr_start command ", __func__);
    mctl_divert_send_hdr_done(ctrl, CAM_CTRL_FAILED, hdr_start->cookie);
    return;
  }

  ctrl->video_ctrl.hdr_status.hdr_start = *hdr_start;
  fp_ctrl->intf.input.mctl_info.opt_mode = FRAME_PROC_SNAPSHOT;

  CDBG_HIGH("%s E Exposure %d %d %d Buffers %d %d %d, %d %d %d ", __func__,
    hdr_start->exp[0], hdr_start->exp[1], hdr_start->exp[2],
    hdr_start->hdr_main_idx[0], hdr_start->hdr_main_idx[1],
    hdr_start->hdr_main_idx[2], hdr_start->hdr_thm_idx[0],
    hdr_start->hdr_thm_idx[1], hdr_start->hdr_thm_idx[2]);
  if (ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT) {
    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.snap.real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.snapshot_wb.g_gain;
  } else {
    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.cur_real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.curr_gains.g_gain;
  }

  for(idx = 0;
       idx < hdr_start->num_hdr_frames && idx < MAX_HDR_EXP_FRAME_NUM &&
       ctrl->hdrCtrl.hdr_main_divert_count < MAX_FRAMEPROC_FRAME_NUM;
       idx++) {
    memset(&frame,  0,  sizeof(frame));
    if (ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV61 ||
      ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV16)
      fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V1;
    else
      fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V2;

    fp_ctrl->intf.input.mctl_info.picture_dim.width =
      ctrl->dimInfo.picture_width;
    fp_ctrl->intf.input.mctl_info.picture_dim.height =
      ctrl->dimInfo.picture_height;

    if (ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV61 ||
      ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV16)
      fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V1;
    else
      fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V2;

    fp_ctrl->intf.input.mctl_info.thumbnail_dim.width =
      ctrl->dimInfo.ui_thumbnail_width;
    fp_ctrl->intf.input.mctl_info.thumbnail_dim.height =
      ctrl->dimInfo.ui_thumbnail_height;

    frame.image_type = ext_mode = MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
    frame.path = OUTPUT_TYPE_S;
    frame.num_planes = ctrl->dimInfo.picture_frame_offset.num_planes;
    local_buf_info = ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info;
    if(local_buf_info[hdr_start->hdr_main_idx[idx]].local_vaddr == NULL) {
      CDBG_ERROR("%s No mapping found for buffer %d ",
        __func__, hdr_start->hdr_main_idx[idx]);
      fp_ctrl->intf.input.mctl_info.num_main_img = 0;
      continue;
    }

    for(i = 0; i < frame.num_planes; i++) {
      frame.mp[i].data_offset = ctrl->dimInfo.picture_frame_offset.mp[i].offset;
      frame.mp[i].length = ctrl->dimInfo.picture_frame_offset.mp[i].len;
      frame.mp[i].fd = local_buf_info[hdr_start->hdr_main_idx[idx]].fd;
      if (i > 0)
        frame.mp[i].vaddr = (unsigned long)
          (local_buf_info[hdr_start->hdr_main_idx[idx]].local_vaddr)
          + frame.mp[i-1].length;
      else
        frame.mp[i].vaddr = (unsigned long)
          (local_buf_info[hdr_start->hdr_main_idx[idx]].local_vaddr);
      CDBG("%s (buf,fd,length, offset) = (0x%lu, %d, %d, %d)", __func__,
        frame.mp[i].vaddr, frame.mp[i].fd, frame.mp[i].length,
        frame.mp[i].addr_offset);
    }
    memcpy(&(fp_ctrl->intf.input.mctl_info.main_img_frame
      [ctrl->hdrCtrl.hdr_main_divert_count]),
      &frame, sizeof(struct msm_pp_frame));
    ctrl->hdrCtrl.hdr_main_divert_count++;

    memset(&frame,  0,  sizeof(frame));
    frame.image_type = ext_mode = MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL;
    frame.path = OUTPUT_TYPE_T;
    frame.num_planes = ctrl->dimInfo.thumb_frame_offset.num_planes;
    local_buf_info = ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info;
    if (local_buf_info[hdr_start->hdr_thm_idx[idx]].local_vaddr == NULL) {
      CDBG_ERROR("%s No mapping found for thumbnail buffer %d ",
        __func__, hdr_start->hdr_thm_idx[idx]);
      fp_ctrl->intf.input.mctl_info.num_thumb_img = 0;
      continue;
    }

    for(i = 0; i < frame.num_planes; i++) {
      frame.mp[i].data_offset = ctrl->dimInfo.thumb_frame_offset.mp[i].offset;
      frame.mp[i].length = ctrl->dimInfo.thumb_frame_offset.mp[i].len;
      frame.mp[i].fd = local_buf_info[hdr_start->hdr_thm_idx[idx]].fd;
      if (i > 0)
        frame.mp[i].vaddr = (unsigned long)
          (local_buf_info[hdr_start->hdr_thm_idx[idx]].local_vaddr)
          + frame.mp[i-1].length;
      else
        frame.mp[i].vaddr = (unsigned long)
          (local_buf_info[hdr_start->hdr_thm_idx[idx]].local_vaddr);
      CDBG("%s (buf,fd,length, offset) = (0x%lu, %d, %d, %d)", __func__,
        frame.mp[i].vaddr, frame.mp[i].fd, frame.mp[i].length,
        frame.mp[i].addr_offset);
    }

    memcpy(&(fp_ctrl->intf.input.mctl_info.thumb_img_frame
      [ctrl->hdrCtrl.hdr_thumb_divert_count]),
      &frame, sizeof(struct msm_pp_frame));
    ctrl->hdrCtrl.hdr_thumb_divert_count++;
  }
  if (ctrl->hdrCtrl.hdr_main_divert_count != ctrl->hdrCtrl.total_frames
      || ctrl->hdrCtrl.hdr_thumb_divert_count != ctrl->hdrCtrl.total_frames
      || idx < hdr_start->num_hdr_frames) {
    CDBG_ERROR("%s HDR configured for %d frames, but received only"
      " %d main image frames and %d thumbnail frames ", __func__,
      ctrl->hdrCtrl.total_frames, ctrl->hdrCtrl.hdr_main_divert_count,
      ctrl->hdrCtrl.hdr_thumb_divert_count);
    mctl_divert_send_hdr_done(ctrl, CAM_CTRL_FAILED, hdr_start->cookie);
    return;
  }

  fp_ctrl->intf.input.mctl_info.num_main_img = ctrl->hdrCtrl.hdr_main_divert_count;
  fp_ctrl->intf.input.mctl_info.num_thumb_img = ctrl->hdrCtrl.hdr_thumb_divert_count;

  if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
    fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0){
    CDBG_ERROR("%s Error executing HDR for snapshot ", __func__);
    /* send event back to HAL */
    mctl_divert_send_hdr_done(ctrl, CAM_CTRL_FAILED, hdr_start->cookie);
  } else {
    CDBG("%s Success processing HDR", __func__);
    /* send event back to HAL */
    mctl_divert_send_hdr_done(ctrl, CAM_CTRL_SUCCESS, hdr_start->cookie);
  }
  CDBG_HIGH("%s X ", __func__);
}

void mctl_divert_socket_get_buf_data(mctl_config_ctrl_t *ctrl)
{
  cam_sock_packet_t* sock_pkt;

  sock_pkt = &(ctrl->video_ctrl.socket_info.buf_packet);

  switch(sock_pkt->msg_type) {
    case CAM_SOCK_MSG_TYPE_FD_MAPPING:
      mctl_divert_domain_socket_reg_buf(ctrl, &sock_pkt->payload.frame_fd_map);
      break;
    case CAM_SOCK_MSG_TYPE_FD_UNMAPPING:
      mctl_divert_domain_socket_unreg_buf(ctrl,
        &sock_pkt->payload.frame_fd_unmap);
      break;
    case CAM_SOCK_MSG_TYPE_WDN_START:
      mctl_divert_domain_socket_do_wdn(ctrl, &sock_pkt->payload.wdn_start);
      break;
    case CAM_SOCK_MSG_TYPE_HIST_MAPPING:
      mctl_divert_domain_socket_reg_hist_buf(ctrl,
        &sock_pkt->payload.frame_fd_map);
      break;
    case CAM_SOCK_MSG_TYPE_HIST_UNMAPPING:
      mctl_divert_domain_socket_unreg_hist_buf(ctrl,
        &sock_pkt->payload.frame_fd_unmap);
      break;
    case CAM_SOCK_MSG_TYPE_HDR_START:
      mctl_divert_do_hdr(ctrl, &sock_pkt->payload.hdr_pkg);
      break;
    default:
      CDBG_HIGH("%s: Invalid MSG type = %d", __func__, sock_pkt->msg_type);
      break;
  }
} /* mctl_divert_socket_get_buf_data */
