/* ============================================================================
  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "config_proc.h"
#include "stereocam.h"
#include "camera_dbg.h"

int st_analysis_launched = FALSE;
int st_dispatch_launched = FALSE;
int st_stop_video = FALSE;
int stop_ack_holding = FALSE;
#if 0
/*===========================================================================
 * FUNCTION    - setup_stereo_thread_pipes -
 *
 * DESCRIPTION: Setup communication pipes for config and Stereo threads.
 *==========================================================================*/
static int setup_stereo_thread_pipes(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;

  if (pipe(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN])< 0) {
    CDBG_HIGH("%s: conf_to_stcam_analysis pipe creation failed\n", __func__);
    return FALSE;
  }

  if (pipe(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_OUT])< 0) {
    CDBG_HIGH("%s: stcam_analysis_to_conf pipe creation failed\n", __func__);
    return FALSE;
  }

  if (pipe(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN])< 0) {
    CDBG_HIGH("%s: conf_to_stcam_dispatch pipe creation failed\n", __func__);
    return FALSE;
  }

  if (pipe(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT])< 0) {
    CDBG_HIGH("%s: stcam_dispatch_to_conf pipe creation failed\n", __func__);
    return FALSE;
  }
  return TRUE;
} /* setup_stereo_thread_pipes */

/*===========================================================================
 * FUNCTION    - stereo_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
void stereocam_init(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;

#ifdef MM_STEREO_CAM
  ctrl->stereoCtrl.st_analysis_launch = launch_stereocam_analysis_thread;
  ctrl->stereoCtrl.st_analysis_wait_ready = wait_stereocam_analysis_ready;
  ctrl->stereoCtrl.st_analysis_release = release_stereocam_analysis_thread;

  ctrl->stereoCtrl.st_dispatch_launch = launch_stereocam_dispatch_thread;
  ctrl->stereoCtrl.st_dispatch_wait_ready = wait_stereocam_dispatch_ready;
  ctrl->stereoCtrl.st_dispatch_release = release_stereocam_dispatch_thread;

  ctrl->stereoCtrl.lib3d.pop_out_ratio = 0.3;
#else
  ctrl->stereoCtrl.st_analysis_launch = NULL;
  ctrl->stereoCtrl.st_analysis_wait_ready = NULL;
  ctrl->stereoCtrl.st_analysis_release = NULL;

  ctrl->stereoCtrl.st_dispatch_launch = NULL;
  ctrl->stereoCtrl.st_dispatch_wait_ready = NULL;
  ctrl->stereoCtrl.st_dispatch_release = NULL;
#endif

  if (ctrl->current_mode == CAMERA_MODE_3D) {
    setup_stereo_thread_pipes(ctrl);
    ctrl->stereoCtrl.procFrame.currentFrameProgress = ST_ANALYSIS_END;
    ctrl->stereoCtrl.videoFrame.currentFrameProgress = ST_FRAME_R_DONE;
  }
} /* stereo_init */

/*===========================================================================
 * FUNCTION    - close_stereo_thread_pipes -
 *
 * DESCRIPTION: Close communication pipes for config and stereo threads.
 *==========================================================================*/
static int close_stereo_thread_pipes(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;

  if (ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][READ_END] >= 0) {
    close(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][READ_END]);
    close(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][WRITE_END]);
  }

  if (ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_OUT][READ_END] >= 0) {
    close(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_OUT][READ_END]);
    close(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_OUT][WRITE_END]);
  }

  if (ctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][READ_END] >= 0) {
    close(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][READ_END]);
    close(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][WRITE_END]);
  }

  if (ctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT][READ_END] >= 0) {
    close(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT][READ_END]);
    close(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT][WRITE_END]);
  }
  return TRUE;
} /* close_stereo_thread_pipes */

/*===========================================================================
 * FUNCTION    - start_stereo_threads -
 *
 * DESCRIPTION: Start stereo thread based on input.
 *==========================================================================*/
static int start_stereo_threads(int thread_id, void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;

  if (thread_id == STEREO_DISPATCH) {
    CDBG("%s: Start STEREO_DISPATCH Thread \n", __func__);
    if (ctrl->stereoCtrl.st_dispatch_launch((void *)ctrl)) {
      CDBG_HIGH("%s: launch_stereocam_dispatch_thread failed!\n", __func__);
      return FALSE;
    }
    if (ctrl->stereoCtrl.st_dispatch_wait_ready()) {
      CDBG_HIGH("%s: wait_stereocam_dispatch_ready failed!\n", __func__);
      return FALSE;
    }
  } else if (thread_id == STEREO_ANALYSIS) {
    CDBG("%s: Start STEREO_ANALYSIS Thread \n", __func__);
    if (ctrl->stereoCtrl.st_analysis_launch((void *)ctrl)) {
      CDBG_HIGH("%s: launch_stereocam_analysis_thread failed!\n", __func__);
      return FALSE;
    }
    if (ctrl->stereoCtrl.st_analysis_wait_ready()) {
      CDBG_HIGH("%s: wait_stereocam_analysis_ready failed!\n", __func__);
      return FALSE;
    }
  } else {
    CDBG_HIGH("%s: Invalid thread_id = %d\n", __func__, thread_id);
    return FALSE;
  }

  return TRUE;
} /* start_stereo_threads */

/*===========================================================================
 * FUNCTION    - end_stereo_threads -
 *
 * DESCRIPTION: Close stereo thread based on input.
 *==========================================================================*/
static int end_stereo_threads(int thread_id, void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;

  if (thread_id == STEREO_DISPATCH) {
    CDBG("%s: End STEREO_DISPATCH Thread.\n", __func__);
    if (cfgctrl.stereoCtrl.st_dispatch_release()) {
      CDBG_HIGH("stereocam_dispatch_thread exit failure!\n");
      return FALSE;
    }
  } else if (thread_id == STEREO_ANALYSIS) {
    CDBG("%s: End STEREO_ANALYSIS Thread.\n", __func__);
    if (cfgctrl.stereoCtrl.st_analysis_release()) {
      CDBG_HIGH("stereocam_analysis_thread exit failure!\n");
      return FALSE;
    }
  } else {
    CDBG_HIGH("%s: Invalid thread_id = %d\n", __func__, thread_id);
    return FALSE;
  }

  return TRUE;
} /* end_stereo_threads */

/*===========================================================================
 * FUNCTION    - stereocam_config_proc_ctrl_command -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t stereocam_config_proc_ctrl_command(void *ctrlBlk, void *ctrlCmd)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  struct msm_ctrl_cmd *ctrl_cmd = (struct msm_ctrl_cmd *)ctrlCmd;

  config_proc_ctrl_command(ctrlBlk, ctrlCmd);

  if (ctrl_cmd->type == CAMERA_STOP_VIDEO) {
    CDBG("%s: VFE STOP is issued\n", __func__);
    st_stop_video = TRUE;
#ifdef MM_STEREO_CAM
    if (ctrl->stereoCtrl.procFrame.currentFrameProgress == ST_ANALYSIS_START) {
      int rc = 0;
      CDBG("%s: Abort stereo processing.\n", __func__);
      rc = ctrl->stereoCtrl.lib3d.s3d_abort(ctrl->stereoCtrl.lib3d.s3d_param);

      if (rc != S3D_RET_SUCCESS)
        CDBG_HIGH("%s: s3d_abort failed. rc=%d. STOP VIDEO may take longer\n",
          __func__, rc);
    }
#endif
  } else if ((ctrl_cmd->type == CAMERA_START_VIDEO) ||
    (ctrl_cmd->type == CAMERA_START_SNAPSHOT)) {
    CDBG("%s: START VIDEO or SNAPSHOT is issued\n", __func__);
    st_stop_video = FALSE;
  }

  return TRUE;
} /* stereocam_config_proc_ctrl_command */

/*===========================================================================
 * FUNCTION    - stereocam_config_proc_vfe_event_message -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t stereocam_config_proc_vfe_event_message(void *ctrlBlk, void *camMsg)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  struct msm_cam_evt_msg *msg = (struct msm_cam_evt_msg *)camMsg;

#ifdef MM_STEREO_CAM
  if (msg->msg_id == VFE_ID_STOP_ACK) {
    CDBG("%s: VFE STOP ACK\n", __func__);
    if (((ctrl->stereoCtrl.videoFrame.currentFrameProgress !=
      ST_FRAME_R_DONE) ||
      (ctrl->stereoCtrl.procFrame.currentFrameProgress !=
      ST_ANALYSIS_END)) && st_stop_video) {
      CDBG("%s: Current frame is still in progress\n", __func__);
      memcpy(&ctrl->stereoCtrl.bkupEvtMsg, msg, sizeof(struct msm_cam_evt_msg));
      stop_ack_holding = TRUE;
      return TRUE;
    }
  }
#endif
  config_proc_vfe_event_message(ctrlBlk, camMsg);

  return TRUE;
} /* stereocam_config_proc_vfe_event_message */

/*===========================================================================
 * FUNCTION    - streocam_send_msg -
 *
 * DESCRIPTION: This routine sends messages to stereo threads.
 *==========================================================================*/
static int streocam_send_msg(int thread_id, void *ctrlBlk, void *camMsg)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  static struct msm_cam_evt_msg analysis_msg, dispatch_msg;
  static struct msm_st_frame analysis_frame_t, dispatch_frame_t;
  struct msm_cam_evt_msg *msg = (struct msm_cam_evt_msg *)camMsg;
  int rc;

  CDBG("%s: thread id %d msg type %d", __func__, thread_id, msg->type);

  /* Copy message in local data because if next wake of config thread
   * happens before 1st thread reads the data then it will be overwritten. */
  if (thread_id == STEREO_ANALYSIS) {
    if (st_stop_video) {
      CDBG("%s: VFE is stopped, no need to send frame to 3D Analysis\n",
        __func__);
      return TRUE;
    }

    memcpy(&analysis_msg, msg, sizeof(analysis_msg));
    if (msg->data == NULL) {
      CDBG_ERROR("%s: Error...Analysis Msg data pointer is NULL.\n", __func__);
      return FALSE;
    } else {
      memcpy(&analysis_frame_t, msg->data, sizeof(analysis_frame_t));
    }
    analysis_msg.data = (void *)&(analysis_frame_t);

    ctrl->stereoCtrl.procFrame.currentFrameProgress = ST_ANALYSIS_START;

    rc = write(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][WRITE_END],
               &analysis_msg, sizeof(analysis_msg));
    if (rc < 0) {
      CDBG_HIGH("%s: Stereocam Analysis wake up failed\n", __func__);
      return FALSE;
    }
  } else if (thread_id == STEREO_DISPATCH) {
    memcpy(&dispatch_msg, msg, sizeof(dispatch_msg));
    if (msg->data == NULL) {
      CDBG_ERROR("%s: Error...Dispatch Msg data pointer is NULL.\n", __func__);
      return FALSE;
    } else {
      memcpy(&dispatch_frame_t, msg->data, sizeof(dispatch_frame_t));
    }
    dispatch_msg.data = (void *)&(dispatch_frame_t);

    if (dispatch_frame_t.type == OUTPUT_TYPE_ST_D) {
      if (dispatch_frame_t.buf_info.path == OUTPUT_TYPE_T)
        ctrl->vfeCtrl.vfeVpeCfgFn(ctrl, FALSE);
      else if (dispatch_frame_t.buf_info.path == OUTPUT_TYPE_S)
        ctrl->vfeCtrl.vfeVpeCfgFn(ctrl, TRUE);
      else {
        CDBG("%s: D-done\n", __func__);
        ctrl->stereoCtrl.videoFrame.currentFrameProgress = ST_FRAME_D_DONE;
        if (st_stop_video){
          CDBG("%s: VFE is stopped, no need to send frame to 3D Dispatch\n",
            __func__);
          ctrl->stereoCtrl.videoFrame.currentFrameProgress = ST_FRAME_R_DONE;
          return TRUE;
        }
      }
    } else if (dispatch_frame_t.type == OUTPUT_TYPE_ST_L) {
      CDBG("%s: L-done\n", __func__);
      if (ctrl->vfeCtrl.vfeMode != VFE_MODE_SNAPSHOT)
        ctrl->stereoCtrl.videoFrame.currentFrameProgress = ST_FRAME_L_DONE;
    } else if (dispatch_frame_t.type == OUTPUT_TYPE_ST_R) {
      CDBG("%s: R-done\n", __func__);
      ctrl->stereoCtrl.videoFrame.currentFrameProgress = ST_FRAME_R_DONE;
      if (stop_ack_holding &&
        ctrl->stereoCtrl.procFrame.currentFrameProgress == ST_ANALYSIS_END) {
        CDBG("%s: release the stop ack\n", __func__);
        config_proc_vfe_event_message(ctrl, &ctrl->stereoCtrl.bkupEvtMsg);
        stop_ack_holding = FALSE;
      }
      return TRUE;
    } else
      CDBG_HIGH("%s: Invalid dispatch frame type\n", __func__);

    rc = write(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][WRITE_END],
               &dispatch_msg, sizeof(dispatch_msg));
    if (rc < 0) {
      CDBG_HIGH("%s: Stereocam Dispatch wake up failed\n", __func__);
      return FALSE;
    }
  } else {
    CDBG_HIGH("%s: Invalid thread id %d\n", __func__, thread_id);
    return FALSE;
  }
  return TRUE;
} /* streocam_send_msg */

/*===========================================================================
 * FUNCTION    - stereocam_proc_message -
 *
 * DESCRIPTION: This routine processes messsages from stereo threads.
 *==========================================================================*/
void stereocam_proc_message(int thread_id, int direction, void *ctrlBlk,
  void *camMsg)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  struct msm_frame stereo_frame_p;
  struct msm_st_frame stereo_frame_v;
  struct msm_cam_evt_msg *msg = NULL;
  int rc;

  switch (thread_id) {
    case STEREO_DISPATCH: {
      if (direction == PIPE_OUT) {
        CDBG("%s: Frame from st_dispatch thread.\n", __func__);
        rc = read(ctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT][READ_END],
          &stereo_frame_v, sizeof(stereo_frame_v));
        if (rc < 0)
          CDBG_HIGH("%s: Cannot read from stcam dispatch thread\n", __func__);

        if ((rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_PUT_ST_FRAME,
          &stereo_frame_v)) < 0)
          CDBG_ERROR("%s: MSM_CAM_IOCTL_PUT_ST_FRAME is failed  rc = %d\n",
            __func__, rc);
      } else if (direction == PIPE_IN) {
        msg = (struct msm_cam_evt_msg *)camMsg;
        if (!st_dispatch_launched) {
          if (!start_stereo_threads(STEREO_DISPATCH, (void *)ctrl))
            CDBG_HIGH("%s: Stereo Thread start failed\n", __func__);
          else
            st_dispatch_launched = TRUE;
        }

        if (!streocam_send_msg(STEREO_DISPATCH, ctrl, msg))
          CDBG_HIGH("%s: send msg to dispatch thread failed\n", __func__);
      } else
        CDBG_HIGH("%s: Invalid direction for Dispatch = %d\n", __func__,
          direction);

      break;
    }
    case STEREO_ANALYSIS: {
      if (direction == PIPE_OUT) {
        CDBG("%s: Frame from st_analysis thread.\n", __func__);
        rc = read(ctrl->child_fd_set[STEREO_ANALYSIS][PIPE_OUT][READ_END],
          &stereo_frame_p, sizeof(stereo_frame_p));
        if (rc < 0)
          CDBG_HIGH("%s: Cannot read from stcam analysis thread\n", __func__);

        ctrl->stereoCtrl.procFrame.currentFrameProgress = ST_ANALYSIS_END;

        if (stop_ack_holding && (ST_FRAME_R_DONE ==
          ctrl->stereoCtrl.videoFrame.currentFrameProgress)) {
          CDBG("%s: release the stop ack\n", __func__);
          config_proc_vfe_event_message(ctrl, &ctrl->stereoCtrl.bkupEvtMsg);
          stop_ack_holding = FALSE;
          return;
        }

        if (st_stop_video) {
          CDBG("%s: VFE is stopped, no need to release 3D_P buf\n", __func__);
          return;
        }

        if ((rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_RELEASE_FRAME_BUFFER,
          &stereo_frame_p)) < 0)
          CDBG_ERROR("%s: MSM_CAM_IOCTL_RELEASE_FRAME_BUFFER failed rc = %d\n",
            __func__, rc);

      } else if (direction == PIPE_IN) {
        msg = (struct msm_cam_evt_msg *)camMsg;
        if (!st_analysis_launched) {
          if (!start_stereo_threads(STEREO_ANALYSIS, (void *)ctrl))
            CDBG_HIGH("%s: Stereo Thread start failed\n", __func__);
          else
            st_analysis_launched = TRUE;
        }

        if (!streocam_send_msg(STEREO_ANALYSIS, ctrl, msg))
          CDBG_HIGH("%s: send msg to analysis thread failed\n", __func__);
      } else
        CDBG_HIGH("%s: Invalid direction for Analysis = %d\n", __func__,
          direction);
      break;
    }
    default:
      CDBG_HIGH("%s: invalid thread id = %d\n", __func__, thread_id);
  }
} /* stereocam_proc_message */

#ifdef MM_STEREO_CAM
/*===========================================================================
 * FUNCTION    - stereocam_get_lib3d_format -
 *
 * DESCRIPTION: This routine maps the mm-camera 3D image formats to
 *              lib3D image formats.
 *==========================================================================*/
int stereocam_get_lib3d_format(int cam_packing)
{
  switch (cam_packing) {
    case SIDE_BY_SIDE_FULL:
      return S3D_IMAGE_FORMAT_SIDE_BY_SIDE_FULL;
      break;
    case SIDE_BY_SIDE_HALF:
      return S3D_IMAGE_FORMAT_SIDE_BY_SIDE_HALF;
      break;
    case TOP_DOWN_FULL:
      return S3D_IMAGE_FORMAT_TOP_DOWN_FULL;
      break;
    case TOP_DOWN_HALF:
      return S3D_IMAGE_FORMAT_TOP_DOWN_HALF;
      break;
    default:
      CDBG_HIGH("%s: Invalid packing type = %d\n", __func__, cam_packing);
      return -1;
  }
} /* stereocam_get_lib3d_format */

/*===========================================================================
 * FUNCTION    - stereocam_get_correction_matrix -
 *
 * DESCRIPTION: This routine gets the right frame geo correction matrix
 *              from the lib3d library. Frame path could be Video,
 *              Snapshot or Thumbnail.
 *==========================================================================*/
int stereocam_get_correction_matrix(stereo_ctrl_t *stCtrl,
  stereo_frame_t *pStereoFrame)
{
  int rc;
  uint32_t mono_w, mono_h;

  if (pStereoFrame->non_zoom_upscale) {
    mono_w = pStereoFrame->right_pack_dim.orig_w;
    mono_h = pStereoFrame->right_pack_dim.orig_h;
  } else {
    mono_w = pStereoFrame->right_pack_dim.modified_w;
    mono_h = pStereoFrame->right_pack_dim.modified_h;
  }

  CDBG("%s: Requested w = %d, h = %d\n", __func__, mono_w, mono_h);

  rc = stCtrl->lib3d.s3d_get_correction_matrix(stCtrl->lib3d.s3d_param,
    stereocam_get_lib3d_format(pStereoFrame->packing),
    S3D_CORRECTION_ORIGIN_TOP_LEFT, mono_w, mono_h,
    pStereoFrame->geo_corr_matrix);

  if (rc != S3D_RET_SUCCESS) {
    CDBG_HIGH("%s: s3d_get_correction_matrix failed with rc = %d\n",
      __func__, rc);
    return FALSE;
  }

  pStereoFrame->geo_corr_matrix[2] += 0.5;
  pStereoFrame->geo_corr_matrix[5] += 0.5;

#ifdef PRINT_STEREO_MATRIX
  PRINT_1D_MATRIX(3, 3, pStereoFrame->geo_corr_matrix);
#endif
  return TRUE;
} /* stereocam_get_correction_matrix */
#endif

/*===========================================================================
 * FUNCTION    - stereocam_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void stereocam_deinit(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  if (st_analysis_launched) {
    if (!end_stereo_threads(STEREO_ANALYSIS, ctrl))
      CDBG_HIGH("%s: end_stereo_threads failed\n", __func__);
    else
      st_analysis_launched = FALSE;
  }
  if (st_dispatch_launched) {
    if (!end_stereo_threads(STEREO_DISPATCH, ctrl))
      CDBG_HIGH("%s: end_stereo_threads failed\n", __func__);
    else
      st_dispatch_launched = FALSE;
  }
  close_stereo_thread_pipes(ctrl);
  ctrl->stereoCtrl.procFrame.currentFrameProgress = ST_ANALYSIS_END;
  ctrl->stereoCtrl.videoFrame.currentFrameProgress = ST_FRAME_R_DONE;
} /* stereocam_deinit */

/*===========================================================================
 * FUNCTION    - stereocam_set_display_distance -
 *
 * DESCRIPTION: Set the display distance from viewer.
 *              Value will not be set while in the middle of the 3D session.
 *              Range   : 1cm - 1000cm
 *              Default : 200 cm
 *              Sample  : 3D over HDMI 200cm
 *                        3D Panel 30cm
 *==========================================================================*/
int stereocam_set_display_distance(void *ctrlBlk, void *value)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  float distance_in_meteres = *(uint32_t *)value;

  if (distance_in_meteres <= 0 && distance_in_meteres > 1000) {
    CDBG_HIGH("%s: Error ... Requested Range = %f, Valid Range = %d - %d\n",
      __func__, distance_in_meteres, 1, 1000);
    return FALSE;
  }

#ifdef MM_STEREO_CAM
  stCtrl->lib3d.display_distance = (float)distance_in_meteres / 100;
#endif
  return TRUE;
} /* stereocam_set_display_distance */

/*===========================================================================
 * FUNCTION    - stereocam_get_display_distance -
 *
 * DESCRIPTION: Get the current display distance set.
 *==========================================================================*/
uint32_t stereocam_get_display_distance(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;

#ifdef MM_STEREO_CAM
  if (!stCtrl->lib3d.display_distance) {
    CDBG_HIGH("%s: display_distance is not set. Return default 200 cm\n",
      __func__);
    return 200;
  } else
    return (uint32_t)(stCtrl->lib3d.display_distance * 100);
#endif
  return FALSE;
} /* stereocam_get_display_distance */

/*===========================================================================
 * FUNCTION    - stereocam_set_display_view_angle -
 *
 * DESCRIPTION: Set the display view angle from viewer.
 *              Value will not be set while in the middle of the 3D session.
 *              Range   : 1 - 179 degress
 *              Default : 30 degrees
 *==========================================================================*/
int stereocam_set_display_view_angle(void *ctrlBlk, void *value)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  uint32_t view_angle = *(uint32_t *)value;

  if (view_angle <= 0 && view_angle >= 180) {
    CDBG_HIGH("%s: Error ... Requested Range = %d, Valid Range = %d - %d\n",
      __func__, view_angle, 1, 179);
    return FALSE;
  }

#ifdef MM_STEREO_CAM
  stCtrl->lib3d.view_angle = view_angle;
#endif
  return TRUE;
} /* stereocam_set_display_view_angle */

/*===========================================================================
 * FUNCTION    - stereocam_get_display_view_angle -
 *
 * DESCRIPTION: Get the current view angle set.
 *==========================================================================*/
uint32_t stereocam_get_display_view_angle(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;

#ifdef MM_STEREO_CAM
  if (!stCtrl->lib3d.view_angle) {
    CDBG_HIGH("%s: view_angle is not set. Return default 30 degrees\n",
      __func__);
    return 30;
  } else
    return stCtrl->lib3d.view_angle;
#endif
  return FALSE;
} /* stereocam_get_display_view_angle */

/*===========================================================================
 * FUNCTION    - stereocam_set_3d_effect -
 *
 * DESCRIPTION: Set the stereo converge plane.
 *              Value can be set anytime during the 3D session.
 *              Range     : 0 - 10
 *              Step Size : 1
 *              Default   : 3
 *==========================================================================*/
int stereocam_set_3d_effect(void *ctrlBlk, void *value)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  int val = *(uint32_t *)value;

  if (val < 0 && val > 10) {
    CDBG_HIGH("%s: Error ... Requested Range = %d, Valid Range = %d - %d\n",
      __func__, val, 0, 10);
    return FALSE;
  }

#ifdef MM_STEREO_CAM
  stCtrl->lib3d.pop_out_ratio = (float)val / 10;
#endif
  return TRUE;
} /* stereocam_set_3d_effect */

/*===========================================================================
 * FUNCTION    - stereocam_get_3d_effect -
 *
 * DESCRIPTION: Get the current pop out ratio.
 *==========================================================================*/
uint32_t stereocam_get_3d_effect(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;

#ifdef MM_STEREO_CAM
  if (!stCtrl->lib3d.pop_out_ratio) {
    CDBG_HIGH("%s: pop_out_ratio is not set. Return default 3\n",
      __func__);
    return 3;
  } else
    return (uint32_t)(stCtrl->lib3d.pop_out_ratio * 10);
#endif
  return FALSE;
} /* stereocam_get_3d_effect */

/*===========================================================================
 * FUNCTION    - stereocam_set_manual_conv_value -
 *
 * DESCRIPTION: Set the stereo convergence value.
 *              Value can be set anytime during the 3D session.
 *              Range     : 0 - manualConvRange
 *              Step Size : 1
 *              Default   : autoConvValue
 *==========================================================================*/
int stereocam_set_manual_conv_value(void *ctrlBlk, void *value)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  int val = *(uint32_t *)value;

  if ((stCtrl->convMode != ST_CONV_MANUAL) &&
    (stCtrl->convMode != ST_CONV_MANUAL_INIT)) {
    CDBG_HIGH("%s: Manual Convergence is not enabled. First enable it and "
      "then set the value of it.\n", __func__);
    return FALSE;
  }

  if ((val < 0) || (val > (int)stCtrl->manualConvRange)) {
    CDBG_HIGH("%s: User conv value, %d, is not in range. Rang = 0 - %d\n",
      __func__, val, stCtrl->manualConvRange);
    return FALSE;
  } else {
    CDBG("%s: new value set by user = %d\n", __func__, val);
    stCtrl->manualConvValue = val;
    stCtrl->convMode = ST_CONV_MANUAL;
  }

  return TRUE;
} /* stereocam_set_manual_conv_value */

/*===========================================================================
 * FUNCTION    - stereocam_get_manual_conv_range -
 *
 * DESCRIPTION: Get the current pop out ratio.
 *==========================================================================*/
uint32_t stereocam_get_manual_conv_range(void *ctrlBlk)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;

  return stCtrl->manualConvRange;
} /* stereocam_get_manual_conv_range */

/*===========================================================================
 * FUNCTION    - stereocam_enable_manual_convergence -
 *
 * DESCRIPTION: Turn on/off manual convergence
 *==========================================================================*/
int stereocam_enable_manual_convergence(void *ctrlBlk, void *value)
{
  config_ctrl_t *ctrl = (config_ctrl_t *)ctrlBlk;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  int val = *(uint32_t *)value;

  /* Manual Mode will be set only when user gives the value. So
   * mode will be set in stereocam_set_manual_conv_value.
   */
  if (!val)
    stCtrl->convMode = ST_CONV_AUTO;
  else
    stCtrl->convMode = ST_CONV_MANUAL_INIT;

  return TRUE;
} /* stereocam_enable_manual_convergence */
#endif
