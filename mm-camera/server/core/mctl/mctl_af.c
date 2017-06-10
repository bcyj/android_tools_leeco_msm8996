/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file implements the media/module/master controller's focus logic in the                                                              .
   mm-camera server. The functionalities of this modules are:

   1. process/parse af stats events
   2. control the actuator interface

============================================================================*/
#include <errno.h>
#include <string.h>
#include "camera.h"

#include "mctl.h"
#include "mctl_ez.h"
#include "config_proc.h"
#include "camera_dbg.h"
#include "intf_comm_data.h"

#ifdef ENABLE_MCTL_AF
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - mctl_af_send_focus_done_event -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_af_send_focus_done_event(mctl_config_ctrl_t *ctrl, int status)
{
    struct v4l2_event_and_payload v4l2_ev;
    mm_camera_event_t *cam_event;
    int rc = 0;

    v4l2_ev.payload_length = 0;
    v4l2_ev.transaction_id = -1;
    v4l2_ev.payload = NULL;
    cam_event = (mm_camera_event_t *)v4l2_ev.evt.u.data;
    cam_event->event_type = MM_CAMERA_EVT_TYPE_CTRL;
    cam_event->e.ctrl.evt = MM_CAMERA_CTRL_EVT_AUTO_FOCUS_DONE;
    /* for 3A: success: 0  & failure < 0
     * Upper layer expects: success: 1  failure: 0*/
    cam_event->e.ctrl.status = !(uint16_t)status;
    v4l2_ev.evt.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_APP_NOTIFY_EVENT;
    /* Sends the AUTO_FOCUS_DONE event to kernel. */
    rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &v4l2_ev);
    CDBG("%s: Done MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, event = 0x%x, rc = %d\n",
      __func__, v4l2_ev.evt.type, rc);

    return rc;
}

/*===========================================================================
 * FUNCTION    - mctl_af_get_caf_status -
 *
 * DESCRIPTION:
 *==========================================================================*/

int8_t mctl_af_get_caf_status(mctl_config_ctrl_t *ctrl)
{

  mm_camera_event_t *cam_event;
  struct v4l2_event_and_payload v4l2_ev;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_get_t stats_get;
  int ret = 0;
  int send_event = FALSE, status = 0;

  stats_get.type = STATS_PROC_AF_TYPE;
  stats_get.d.get_af.type = AF_STATUS;
  ret = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
    stats_get.type, &stats_get, sizeof(stats_get));

  switch(stats_get.d.get_af.d.af_status) {
    case CAF_FOCUSED:
      CDBG("%s: Image is focused.", __func__);
      status = TRUE;
      send_event = TRUE;
      break;
    case CAF_UNKNOWN:
      CDBG("%s: Image may or may not be focused.", __func__);
      status = FALSE;
      send_event = TRUE;
      break;
    case CAF_NOT_FOCUSED:
    case CAF_FOCUSING:
      /* As of now we'll just return from this call.
         We'll send event later on. */
      CDBG("%s: CAF is Focusing. Wait!!!", __func__);
      stats_proc_set_t stats_af_set_data;
      stats_af_set_data.type = STATS_PROC_AF_TYPE;
      stats_af_set_data.d.set_af.type = AF_SEND_CAF_DONE_EVT_LTR;
      stats_af_set_data.d.set_af.d.af_send_evt_ltr = TRUE;
      ret = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
        stats_af_set_data.type, &stats_af_set_data,
        &(ctrl->stats_proc_ctrl.intf));
      if (ret < 0) {
        CDBG_ERROR("%s: Requesting SEND_EVENT_LATER failed!", __func__);
        status = FALSE;
        send_event = TRUE;
      }
      break;
    default:
      CDBG_ERROR("%s: Invalid Status: %d", __func__,
        stats_get.d.get_af.d.af_status);
      ret = -1;
      break;
  }

  /* send event to upper layer regarding CAF status */
  if (send_event) {
    mctl_af_send_focus_done_event(ctrl, status);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - mctl_af_start -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t mctl_af_start(mctl_config_ctrl_t *ctrl,
  cam_af_focusrect_t focusrect_sel)
{

  int rc = TRUE;
  int i = 0;
  stats_proc_set_t stats_set_data;
  stats_proc_get_t stats_get_data;
  roi_info_t *touch_roi_info = &ctrl->afCtrl.roiInfo;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  vfe_stats_af_params_t vfe_af_params;
  sensor_get_t sensor_get;

  CDBG("%s: E\n", __func__);
  memset(&vfe_af_params, 0, sizeof(vfe_stats_af_params_t));

  if (!ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled) {
    stats_set_data.type = STATS_PROC_AEC_TYPE;
    stats_set_data.d.set_aec.type = AEC_HJR_AF;
    stats_set_data.d.set_aec.d.aec_af_hjr = TRUE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
      stats_set_data.type, &stats_set_data,
      &(ctrl->stats_proc_ctrl.intf));
    if (rc < 0) {
      CDBG_ERROR("%s: stats_proc_set_params failed %d\n", __func__, rc);
      return rc;
    }
    ctrl->stats_proc_ctrl.sof_update_needed = TRUE;
  }

  /* If AF isn't enabled, we'll just return */
  if (!ctrl->afCtrl.af_enable)
    return FALSE;

  /* set focus rectangle type - AUTO/SPOT/AVERAGE/CENTER_WEIGHTED */
  stats_set_data.type = STATS_PROC_AF_TYPE;
  stats_set_data.d.set_af.type = AF_FOCUSRECT;
  stats_set_data.d.set_af.d.cur_focusrect_value = focusrect_sel;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
      stats_set_data.type, &stats_set_data,
      &(ctrl->stats_proc_ctrl.intf));
  if (rc < 0) {
    CDBG_ERROR("%s: Setting AF_FOCUSRECT failed %d\n", __func__, rc);
    return rc;
  }

  /* if face-detection is enabled we need to let 3A know face ROIs */
  if (fp_ctrl->intf.output.fd_d.fd_enable) {
    memset(&stats_set_data, 0, sizeof(stats_proc_set_t));
    stats_set_data.d.set_af.d.roiInfo.num_roi =
      fp_ctrl->intf.output.fd_d.num_faces_detected;
    stats_set_data.d.set_af.d.roiInfo.roi_updated = TRUE;
    stats_set_data.d.set_af.d.roiInfo.frm_height =
      ctrl->dimInfo.display_height;
    stats_set_data.d.set_af.d.roiInfo.frm_width =
      ctrl->dimInfo.display_width;

    for (i = 0;
      i < (int)fp_ctrl->intf.output.fd_d.num_faces_detected;
      i++) {
      stats_set_data.d.set_af.d.roiInfo.roi[i].x =
        fp_ctrl->intf.output.fd_d.roi[i].face_boundary.x;
      stats_set_data.d.set_af.d.roiInfo.roi[i].dx =
        fp_ctrl->intf.output.fd_d.roi[i].face_boundary.dx;
      stats_set_data.d.set_af.d.roiInfo.roi[i].y =
        fp_ctrl->intf.output.fd_d.roi[i].face_boundary.y;
      stats_set_data.d.set_af.d.roiInfo.roi[i].dy =
        fp_ctrl->intf.output.fd_d.roi[i].face_boundary.dy;

      CDBG("%s: Face ROI: x=%d, y=%d, dx=%d, dy=%d\n", __func__,
        stats_set_data.d.set_af.d.roiInfo.roi[i].x,
        stats_set_data.d.set_af.d.roiInfo.roi[i].y,
        stats_set_data.d.set_af.d.roiInfo.roi[i].dx,
        stats_set_data.d.set_af.d.roiInfo.roi[i].dy);
    }

    stats_set_data.type = STATS_PROC_AF_TYPE;
    stats_set_data.d.set_af.type = AF_FACE_DETECTION_ROI;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
      stats_set_data.type, &stats_set_data,
      &(ctrl->stats_proc_ctrl.intf));
    if (rc < 0) {
      CDBG_ERROR("%s: Setting parm AF_FACE_DETECTION_ROI failed\n", __func__);
    }
  }

  /* Get the mode to configure VFE - DEFAULT/SINGLE/MULTIPLE */
  memset(&stats_get_data, 0, sizeof(stats_proc_get_t));
  stats_get_data.type = STATS_PROC_AF_TYPE;
  stats_get_data.d.get_af.type = AF_STATS_CONFIG_MODE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_get_data.type,
    &stats_get_data, sizeof(stats_get_data));
  if (rc < 0) {
    CDBG_ERROR("%s: Getting VFE CONFIG MODE failed\n", __func__);
    return rc;
  }
  CDBG("%s: Received VFE CONFIG MODE: %d", __func__,
    stats_get_data.d.get_af.d.af_stats_config.mode);

  /* get camif info from the sensor */
  if (ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_CAMIF_CFG, &sensor_get, sizeof(sensor_get)) < 0) {
    CDBG_ERROR("%s: sensor_get_params failed\n", __func__);
    return 0;
  }

  /* Update VFE params */
  vfe_af_params.af_mult_window =
    (uint8_t *)stats_get_data.d.get_af.d.af_stats_config.af_multi_roi_window;
  vfe_af_params.multi_roi_nfocus =
    stats_get_data.d.get_af.d.af_stats_config.af_multi_nfocus;
  vfe_af_params.roi_type =
    stats_get_data.d.get_af.d.af_stats_config.mode;
  /* Currently vfe params only accept single ROI. Anyway, in case of multiple
     ROI we pass a window grid table to select right grids from. So VFE will
     not need to know different ROIs. */
  memcpy(&vfe_af_params.region,
    stats_get_data.d.get_af.d.af_stats_config.region,
    sizeof(roi_t));
  vfe_af_params.frame_width = ctrl->dimInfo.display_width;
  vfe_af_params.frame_height = ctrl->dimInfo.display_height;
  vfe_af_params.camif_width = sensor_get.data.camif_setting.last_pixel -
    sensor_get.data.camif_setting.first_pixel + 1;
  vfe_af_params.camif_height = sensor_get.data.camif_setting.last_line -
    sensor_get.data.camif_setting.first_line + 1;

  /* Configure VFE */
  CDBG("%s: Configuring VFE with: config mode: %d frame_size: %d x %d "
    "camif_info: %d x %d", __func__, vfe_af_params.roi_type,
    vfe_af_params.frame_width, vfe_af_params.frame_height,
    vfe_af_params.camif_width, vfe_af_params.camif_height);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_CONFIG_AF,
    &vfe_af_params);
  if (rc) {
    CDBG_ERROR("%s: VFE_CONFIG_AF failed\n", __func__);
    return FALSE;
  }

  CDBG("%s: X\n", __func__);
  return TRUE;
} /*mctl_af_start*/

/*===========================================================================
 * FUNCTION    - mctl_af_stop -
 *
 * DESCRIPTION:
 *==========================================================================*/
void mctl_af_stop(mctl_config_ctrl_t *ctrl)
{
  int rc;
  stats_proc_set_t stats_set_data;
  CDBG("%s:Calling AF stats stop\n", __func__);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_STOP_AF,
    NULL);
  if (rc) {
    CDBG_ERROR("%s: VFE_STOP_AF failed\n", __func__);
  }

  if (!ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled) {
    stats_set_data.type = STATS_PROC_AEC_TYPE;
    stats_set_data.d.set_aec.type = AEC_HJR_AF;
    stats_set_data.d.set_aec.d.aec_af_hjr = FALSE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
      stats_set_data.type, &stats_set_data,
      &(ctrl->stats_proc_ctrl.intf));
    if (rc < 0) {
      CDBG_ERROR("%s: stats_proc_set_params failed %d\n", __func__, rc);
      return;
    }
    ctrl->stats_proc_ctrl.sof_update_needed = TRUE;
  }
} /* mctl_af_stop */

/*===========================================================================
 * FUNCTION    -  mctl_do_af -
 *
 * DESCRIPTION:
 *
 *==========================================================================*/
static void mctl_do_af(mctl_config_ctrl_t *ctrl)
{
  int rc;
  mm_camera_event_t *cam_event;
  struct v4l2_event_and_payload v4l2_ev;
  stats_proc_set_t stats_proc_move_done;
  stats_proc_get_t stats_get;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
#ifdef FEATURE_GYRO
  dsps_get_data_t dsps_get;

  dsps_get.type = GET_DATA_LAST;
  dsps_get.format = TYPE_FLOAT;
  if (dsps_get_params(&dsps_get)) {
    /* Gyro data unavailable */
    sp_ctrl->intf.input.gyro_info.float_ready = 0;
    sp_ctrl->intf.input.gyro_info.flt[0] = 0.0;
    sp_ctrl->intf.input.gyro_info.flt[1] = 0.0;
    sp_ctrl->intf.input.gyro_info.flt[2] = 0.0;
  } else {
    sp_ctrl->intf.input.gyro_info.float_ready = 1;
    sp_ctrl->intf.input.gyro_info.flt[0] = dsps_get.output_data.flt[0];
    sp_ctrl->intf.input.gyro_info.flt[1] = dsps_get.output_data.flt[1];
    sp_ctrl->intf.input.gyro_info.flt[2] = dsps_get.output_data.flt[2];
  }
#endif

  stats_get.type = STATS_PROC_AF_TYPE;
  stats_get.d.get_af.type = AF_FOCUS_MODE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_get.type,
    &stats_get, sizeof(stats_get));
  if (rc < 0) {
    CDBG_ERROR("%s: Getting AF MODE failed\n", __func__);
    return;
  }
  CDBG("%s: Received AF MODE: %d", __func__, stats_get.d.get_af.d.af_mode);

  if (stats_get.d.get_af.d.af_mode == AF_MODE_INFINITY) {
    CDBG_ERROR("%s: Focus mode INFINITY. Ignore Stats.!!!", __func__);
    return;
  }

  sp_ctrl->intf.input.mctl_info.type = STATS_PROC_AF_TYPE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
    sp_ctrl->intf.input.mctl_info.type, &sp_ctrl->intf);
  if (rc < 0) {
    flash_led_get_t led_get_parm;
    ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
      FLASH_GET_MODE, &led_get_parm, sizeof(led_get_parm));
    if (led_get_parm.data.led_mode != LED_MODE_OFF &&
      led_get_parm.data.led_mode != LED_MODE_TORCH) {
      flash_led_set_t led_set_parm;
      led_set_parm.data.led_state = MSM_CAMERA_LED_OFF;
      stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
      sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_OFF;
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
      FLASH_SET_STATE, &led_set_parm, NULL);
    }
    /* send af failed event */
    mctl_af_send_focus_done_event(ctrl, CAMERA_EXIT_CB_FAILED);
  }

  /* Update sensor */
  CDBG("%s:Before: move_lens_status=%d,reset_lens=%d,\
    stop_af=%d,done_status=%d, af_done_flag=%d\n",
    __func__, sp_ctrl->intf.output.af_d.move_lens_status,
    sp_ctrl->intf.output.af_d.reset_lens,
    sp_ctrl->intf.output.af_d.stop_af,
    sp_ctrl->intf.output.af_d.done_status,
    sp_ctrl->intf.output.af_d.done_flag);

  /* Move Lens */
  if (sp_ctrl->intf.output.af_d.move_lens_status == TRUE) {
    if (sp_ctrl->intf.output.af_d.num_of_steps > 0) {
      actuator_set_t actuator_move_lens;
      actuator_move_lens.data.move.direction =
        sp_ctrl->intf.output.af_d.direction;
      actuator_move_lens.data.move.num_steps =
        sp_ctrl->intf.output.af_d.num_of_steps;
      CDBG("%s:num_steps=%d direction=%d\n",
        __func__, sp_ctrl->intf.output.af_d.num_of_steps,
        sp_ctrl->intf.output.af_d.direction);
      if (ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
        rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
          ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle, ACTUATOR_MOVE_FOCUS,
            (void *)&actuator_move_lens, NULL);
      }

      stats_proc_move_done.type = STATS_PROC_AF_TYPE;
      stats_proc_move_done.d.set_af.type = AF_LENS_MOVE_DONE;
      if (rc != 0) {
        CDBG_ERROR("%s: ACTUATOR_MOVE_FOCUS failed %d\n", __func__, rc);
        stats_proc_move_done.d.set_af.d.af_lens_move_status = FALSE;
        CDBG_ERROR("%s:d.af_lens_move_status=%d\n", __func__,
          stats_proc_move_done.d.set_af.d.af_lens_move_status);
      } else {
        stats_proc_move_done.d.set_af.d.af_lens_move_status = TRUE;
        CDBG("%s:d.af_lens_move_status=%d\n", __func__,
          stats_proc_move_done.d.set_af.d.af_lens_move_status);
      }
    } else {
      stats_proc_move_done.type = STATS_PROC_AF_TYPE;
      stats_proc_move_done.d.set_af.type = AF_LENS_MOVE_DONE;
      stats_proc_move_done.d.set_af.d.af_lens_move_status = 2;
      CDBG("%s:d.af_lens_move_status=%d\n", __func__,
        stats_proc_move_done.d.set_af.d.af_lens_move_status);
    }
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
      stats_proc_move_done.type, &stats_proc_move_done,
      &(sp_ctrl->intf));
    if (rc < 0) {
      CDBG_ERROR("%s: AF_LENS_MOVE_DONE failed %d\n", __func__, rc);
    }
  }

  /* Reset Lens */
  if (sp_ctrl->intf.output.af_d.reset_lens == TRUE &&
    (ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle)) {
    CDBG("%s ACTUATOR_DEF_FOCUS\n", __func__);
    rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
        ACTUATOR_DEF_FOCUS, NULL, NULL);
    if (rc != 0) {
      CDBG("File Name:%s, Line#%d,Failure:Reset lens failed\n",
        __FILE__, __LINE__);
      rc = FALSE;
    } else {
      CDBG("Reset lens succeeded\n");
      rc = TRUE;
    }
  }

  /* Stop AF */
  if (sp_ctrl->intf.output.af_d.stop_af == TRUE) {
    mctl_af_stop(ctrl);
    sp_ctrl->intf.output.af_d.stop_af = FALSE;
    /* If CAF, pre-flash is currently disabled. So no need to set it to OFF.
    TBD: It might change - especially in case of ZSL */
    if (!sp_ctrl->intf.output.af_d.cont_af_enabled) {
      flash_led_get_t led_get_parm;
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
        ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
        FLASH_GET_MODE, &led_get_parm, sizeof(led_get_parm));
      if (led_get_parm.data.led_mode != LED_MODE_OFF &&
        led_get_parm.data.led_mode != LED_MODE_TORCH) {
        flash_led_set_t led_set_parm;
        led_set_parm.data.led_state = MSM_CAMERA_LED_OFF;
        stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
        sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_OFF;
        ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
          ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
          FLASH_SET_STATE, &led_set_parm, NULL);
      }
    }
  }
  /* AF Done */
  if (sp_ctrl->intf.output.af_d.done_flag) {
    sp_ctrl->intf.output.af_d.done_flag = FALSE;
    mctl_af_send_focus_done_event(ctrl, sp_ctrl->intf.output.af_d.done_status);
  }

  CDBG("%s:After: num_steps=%d,reset_lens=%d,stop_af=%d,done_status=%d\n",
    __func__,
    sp_ctrl->intf.output.af_d.num_of_steps,
    sp_ctrl->intf.output.af_d.reset_lens,
    sp_ctrl->intf.output.af_d.stop_af,
    sp_ctrl->intf.output.af_d.done_status);

} /* mctl_do_af */
/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_AF -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_AF(void *parm1, void *parm2)
{
  int rc = 0;
  int i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;
  CDBG("%s: E, rc = %d\n", __func__, rc);

  vfe_stats_out->af_bf_done = FALSE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_YUV, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->af_bf_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }
  if(parse_stats)
    mctl_do_af(ctrl);
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AF_CMD);
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_AF */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_BF -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_BF(void *parm1, void *parm2)
{
  int rc = 0;
  int i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;
  CDBG("%s: E, rc = %d\n", __func__, rc);

  vfe_stats_out->af_bf_done = FALSE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_BAYER, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->af_bf_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }
  if(parse_stats)
    mctl_do_af(ctrl);
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AF_CMD);
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_BF */

