/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <media/msm_isp.h>

#include "mctl_divert.h"
#include "mctl.h"
#include "mctl_af.h"
#include "camera_dbg.h"
#include "config_proc.h"
#include "cam_mmap.h"
#include "camera.h"
#include <media/msm_media_info.h>

#if 0
#undef CDBG
#define CDBG LOGE
#endif

camera_iso_mode_type max_camera_iso_type = CAMERA_ISO_MAX - 1;
int release_cam_conf_thread(void){return -1;}
void set_config_start_params(config_params_t* p_start_params){;}
int launch_cam_conf_thread(){return -1;}
int wait_cam_conf_ready() {return -1;}
extern int config_MSG_ID_STOP_ACK(void *parm1,  void *parm2);

/*===========================================================================
 * FUNCTION    - cal_video_buf_size -
 *
 * DESCRIPTION:
 *==========================================================================*/
void cal_video_buf_size(uint16_t width, uint16_t height,
     uint32_t *luma_size, uint32_t *chroma_size, uint32_t *frame_size)
{
  int luma_stride, luma_scanl;

  luma_stride = VENUS_Y_STRIDE(COLOR_FMT_NV12, width);
  luma_scanl = VENUS_Y_SCANLINES(COLOR_FMT_NV12, height);

  *frame_size = VENUS_BUFFER_SIZE(COLOR_FMT_NV12, width, height);
  *luma_size = luma_stride * luma_scanl;
  *chroma_size = *frame_size - *luma_size;
}

/*===========================================================================
 * FUNCTION    - config_process_MSG_ID_SNAPSHOT_DONE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_process_MSG_ID_SNAPSHOT_DONE(mctl_config_ctrl_t *cfg_ctrl, void *parm2)
{
  int rc = 0;
  mod_cmd_t cmd;
  uint8_t sync_abort = 1;
  int channel_interface = *((int *)parm2);
  struct msm_camera_vfe_params_t mod_params;
  CDBG("%s: cam ctrl->state is %d\n", __func__, cfg_ctrl->state);
  if (cfg_ctrl->state != CAMERA_STATE_STARTED) {
    CDBG_ERROR("%s: ctrl->state is not CAMERA_STATE_STARTED\n", __func__);
    return -EINVAL;
  }

  if (channel_interface == AXI_INTF_PIXEL_0 ||
    channel_interface == AXI_INTF_PIXEL_1) {
    flash_led_get_t led_get_parm;
    memset(&mod_params, 0, sizeof(struct msm_camera_vfe_params_t));
    cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
     cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
     FLASH_GET_MODE, &led_get_parm, sizeof(led_get_parm));

    stats_proc_ctrl_t *sp_ctrl = &(cfg_ctrl->stats_proc_ctrl);
    if (led_get_parm.data.led_mode != LED_MODE_OFF &&
        led_get_parm.data.led_mode != LED_MODE_TORCH &&
        sp_ctrl->intf.output.aec_d.use_led_estimation) {
      flash_led_set_t led_set_parm;
      led_set_parm.data.led_state = MSM_CAMERA_LED_OFF;
      sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_OFF;
      cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
        cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
        FLASH_SET_STATE, &led_set_parm, NULL);
      stats_proc_set_t sp_set_param;
      sp_set_param.type = STATS_PROC_AEC_TYPE;
      sp_set_param.d.set_aec.type = AEC_LED_RESET;
      cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
        sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
    }
  }
  if (cfg_ctrl->state != CAMERA_STATE_SENT_STOP) {
    if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
      if (cfg_ctrl->concurrent_enabled)
        rc = cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
            cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
            ISPIF_PROCESS_STOP_ON_FRAME_BOUNDARY, NULL);
      else
        rc = cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
            cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
            ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
        return rc;
      }
    }

  /* stop isp */
    cmd.mod_cmd_ops = MOD_CMD_STOP;
    mod_params.operation_mode = cfg_ctrl->curr_output_info.vfe_operation_mode;
    cmd.cmd_data = &mod_params;
    cmd.length = sizeof(struct msm_camera_vfe_params_t);
    if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
      rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].process(
                         cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                         VFE_CMD_OPS, &cmd);
      if (0 != rc) {
        CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
        return rc;
      }
    }

    /* stop camif: */
    if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
      rc = cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
               cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
               CAMIF_PROC_CMD_OPS, &cmd);
      if (0 != rc) {
        CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
        return rc;
      }
    }

    cmd.mod_cmd_ops = MOD_CMD_ABORT;
    cmd.cmd_data = &sync_abort;
    cmd.length = sizeof(uint8_t);
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_AXI].process(
             cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);

    if (rc < 0) {
      CDBG_ERROR("%s: config AXI_ABORT failed, rc = %d \n", __func__, rc);
      return rc;
    }
    rc = 0;
    cfg_ctrl->state = CAMERA_STATE_SENT_STOP;
  }
  return rc;
} /* config_process_MSG_ID_SNAPSHOT_DONE */

/*===========================================================================
 * FUNCTION    - config_process_MSG_ID_PREV_STOP_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_process_MSG_ID_PREV_STOP_ACK(mctl_config_ctrl_t *cfg_ctrl, void *parm2)
{
  int rc = 0;
  mod_cmd_t cmd;
  uint8_t sync_abort = 1;
  struct msm_camera_vfe_params_t mod_params;
  CDBG("%s: cam ctrl->state is %d and %d\n", __func__, cfg_ctrl->state,
    cfg_ctrl->concurrent_enabled);
  if (cfg_ctrl->state != CAMERA_STATE_STARTED) {
    CDBG_ERROR("%s: ctrl->state is not CAMERA_STATE_STARTED\n", __func__);
    return -EINVAL;
  }

  if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    if (cfg_ctrl->concurrent_enabled)
      rc = cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_STOP_ON_FRAME_BOUNDARY, NULL);
    else
      rc = cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
      return rc;
    }
  }
   /* stop isp */
  cmd.mod_cmd_ops = MOD_CMD_STOP;
  mod_params.operation_mode = cfg_ctrl->curr_output_info.vfe_operation_mode;
  cmd.cmd_data = &mod_params;
  cmd.length = sizeof(struct msm_camera_vfe_params_t);
  if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
      return rc;
    }
  }

   /* stop camif: */
  if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
      return rc;
    }
  }

  cmd.mod_cmd_ops = MOD_CMD_ABORT;
  cmd.cmd_data = &sync_abort;
  cmd.length = sizeof(uint8_t);
  rc = cfg_ctrl->comp_ops[MCTL_COMPID_AXI].process(
           cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
           AXI_PROC_CMD_OPS, &cmd);

  if (rc < 0) {
    CDBG_ERROR("%s: config AXI_ABORT failed, rc = %d \n", __func__, rc);
    return rc;
  }
  rc = 0;

    /* stop AF */
  if (cfg_ctrl->afCtrl.af_cont_enable) {
    /* Stop AF STATS */
    mctl_af_stop(cfg_ctrl);
  }
  cfg_ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = FALSE;

  cfg_ctrl->state = CAMERA_STATE_SENT_STOP;
  rc = config_MSG_ID_STOP_ACK(cfg_ctrl,  parm2);
  return rc;
} /* config_process_MSG_ID_PREV_STOP_ACK */

/*===========================================================================
 * FUNCTION    - config_proc_write_sensor_gain -
 *
 * DESCRIPTION: Write the gain to the sensor interface.
 *==========================================================================*/
int config_proc_write_sensor_gain(void *cctrl)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);
  sensor_set_t sensor_set;
  stats_proc_set_t sp_set_param;
  sensor_get_t sensor_get;
  int sensor_update_ok, rc = 0;

  sensor_update_ok = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
	     ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	     SENSOR_SET_EXPOSURE, NULL, NULL);
  if (sensor_update_ok < 0) {
    CDBG_ERROR("%s Sensor gain update failed ", __func__);
    rc = -1;
    sensor_update_ok = FALSE;
  } else
    sensor_update_ok = TRUE;

  CDBG("%s: Sensor update=%d \n",__func__, sensor_update_ok);

  sp_set_param.type = STATS_PROC_AEC_TYPE;
  sp_set_param.d.set_aec.type = AEC_SOF;
  sp_set_param.d.set_aec.d.aec_sensor_update_ok = sensor_update_ok;
  if((ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
    sp_set_param.type, &sp_set_param, &(sp_ctrl->intf))) < 0) {
    CDBG_ERROR("%s Stats proc set params failed ", __func__);
    rc =-1;
  }

  if(ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
		 ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
		 SENSOR_GET_DIGITAL_GAIN, &sensor_get, sizeof(sensor_get))) {
    CDBG_ERROR("%s Error getting digital gain from sensor ", __func__);
    rc =-1;
  }
  sp_ctrl->digital_gain = sensor_get.data.aec_info.digital_gain;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_CUR_FPS, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s Error getting current fps from sensor ", __func__);
    rc =-1;
  }
  sp_ctrl->intf.input.sensor_info.current_fps = sensor_get.data.fps;

  CDBG("%s: Digital Gain: %f Current FPS: %d", __func__,
    sp_ctrl->digital_gain, sp_ctrl->intf.input.sensor_info.current_fps);

  return rc;
}

/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_UPDATE_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_proc_MSG_ID_UPDATE_ACK(mctl_config_ctrl_t *cfg_ctrl,
  void *parm2)
{
  int rc = 0;
  stats_proc_ctrl_t *sp_ctrl = &(cfg_ctrl->stats_proc_ctrl);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);

  CDBG("====Update ACK: ctrl->state = %d ===\n",
    cfg_ctrl->state);

  if (cfg_ctrl->vfe_reg_updated) {
    uint8_t vfe_reg_updated;
    if (((cfg_ctrl->vfeMode == VFE_OP_MODE_PREVIEW) ||
      (cfg_ctrl->vfeMode == VFE_OP_MODE_VIDEO) ||
      (cfg_ctrl->vfeMode == VFE_OP_MODE_ZSL)) &&
      (cfg_ctrl->curr_output_info.vfe_operation_mode !=
      VFE_OUTPUTS_RDI0)) {
      vfe_flash_parms_t vfe_flash_parms;
      if (sp_output->aec_d.strobe_cfg_st == STROBE_TO_BE_CONFIGURED) {
        stats_proc_set_t set_param;
        set_param.type = STATS_PROC_AEC_TYPE;
        set_param.d.set_aec.type = AEC_STROBE_CFG_ST;
        set_param.d.set_aec.d.aec_strobe_cfg_st = STROBE_PRE_CONFIGURED;
        cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
          cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
          set_param.type, &set_param,
          &(sp_ctrl->intf));
        vfe_flash_parms.flash_mode = VFE_FLASH_STROBE;
      } else if (sp_output->aec_d.use_led_estimation) {
        vfe_flash_parms.flash_mode = VFE_FLASH_LED;
      } else {
        vfe_flash_parms.flash_mode = VFE_FLASH_NONE;
      }
      if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
        if ((int)vfe_flash_parms.flash_mode != VFE_FLASH_NONE) {
          vfe_flash_parms.sensitivity_led_off = sp_output->aec_d.flash_si.off;
          vfe_flash_parms.sensitivity_led_low = sp_output->aec_d.flash_si.low;
          vfe_flash_parms.sensitivity_led_hi = sp_output->aec_d.flash_si.high;
          vfe_flash_parms.strobe_duration =
            cfg_ctrl->stats_proc_ctrl.intf.input.chromatix->AEC_strobe_flash.strobe_min_time;
          rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
            cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_FLASH_PARMS,
            (void *)&vfe_flash_parms, NULL);
          if (rc != VFE_SUCCESS)
            CDBG_ERROR("%s VFE Set FLASH params failed ", __func__);
        }
      }
    } else {
      CDBG_HIGH("%s: In-Correct Mode", __func__);
    }
    if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
      rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].process(
        cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SOF_NOTIFY,
        &vfe_reg_updated);
      if (!rc)
        cfg_ctrl->vfe_reg_updated = vfe_reg_updated;
      else
        rc = -EFAULT;
    }
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_INTF_UPDATE_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_proc_INTF_UPDATE_ACK(void *parm,
  int channel_interface)
{
  int rc = 0;
  mctl_config_ctrl_t *cfg_ctrl = (mctl_config_ctrl_t *)parm;
  CDBG("Received Update ACK!!! %d\n",channel_interface);
  CDBG("====Update ACK: ctrl->state = %d ===\n",
    cfg_ctrl->state);

  rc = cfg_ctrl->comp_ops[MCTL_COMPID_AXI].process(
    cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
    AXI_PROC_EVENT_REG_UPDATE, (void *)channel_interface);
  if (rc < 0) {
    CDBG_ERROR("%s: AXI_PROC_EVENT_REG_UPDATE failed\n", __func__);
    goto error;
  }

  if (cfg_ctrl->state == CAMERA_STATE_STARTED ||
    cfg_ctrl->state == CAMERA_STATE_SENT_STOP) {
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_AXI].process(
      cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
      AXI_PROC_EVENT_UNREGISTER_WMS, (void *)channel_interface);
    if (rc < 0) {
      CDBG_ERROR("%s: AXI_PROC_EVENT_UNREGISTER_WMS failed\n", __func__);
    }
  }

  switch (channel_interface) {
  case AXI_INTF_PIXEL_0:
    cfg_ctrl->curr_output_info.pending_ports &=
      ~(VFE_OUTPUT_PRIMARY|VFE_OUTPUT_SECONDARY);
    break;
  case AXI_INTF_RDI_0:
    cfg_ctrl->curr_output_info.pending_ports &=
      ~VFE_OUTPUT_TERTIARY1;
    break;
  case AXI_INTF_RDI_1:
    cfg_ctrl->curr_output_info.pending_ports &=
      ~VFE_OUTPUT_TERTIARY2;
    break;
  default:
    break;
  }
  if (!cfg_ctrl->curr_output_info.pending_ports) {
    struct msm_cam_evt_msg adsp;
    memset(&adsp, 0, sizeof(struct msm_cam_evt_msg));
    if (cfg_ctrl->state == CAMERA_STATE_SENT_START) {
      adsp.msg_id = MSG_ID_START_ACK;
      rc = cfg_ctrl->config_intf->config_proc_event_message(cfg_ctrl, &adsp);
    } else if (cfg_ctrl->state == CAMERA_STATE_STARTED){
        if(cfg_ctrl->ops_mode != CAM_OP_MODE_SNAPSHOT &&
         cfg_ctrl->ops_mode != CAM_OP_MODE_RAW_SNAPSHOT &&
         cfg_ctrl->ops_mode != CAM_OP_MODE_JPEG_SNAPSHOT) {
          adsp.msg_id = MSG_ID_PREV_STOP_ACK;
          rc = config_proc_event_message_1(cfg_ctrl, &adsp);
        } else if (cfg_ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT ||
           cfg_ctrl->ops_mode == CAM_OP_MODE_JPEG_SNAPSHOT)
           rc = config_process_MSG_ID_SNAPSHOT_DONE(cfg_ctrl,
                   &channel_interface);
    }
  }
error:
  return rc;
}


/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_CAMIF_ERROR -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_proc_MSG_ID_CAMIF_ERROR(mctl_config_ctrl_t *cfg_ctrl,
  void *parm2)
{
  int rc = 0;
  uint32_t error_code;
  struct msm_camera_vfe_params_t mod_params;
  mod_cmd_t cmd;

#ifndef VFE_2X
  cfg_ctrl->state = CAMERA_STATE_ERROR;
  /* Something wrong! Just stop VFE */
  /* 1. stop vfe: */
  cmd.mod_cmd_ops = MOD_CMD_STOP;
  mod_params.operation_mode =
    cfg_ctrl->curr_output_info.vfe_operation_mode;
  mod_params.port_info = cfg_ctrl->curr_output_info.active_ports;
  mod_params.cmd_type = AXI_CMD_PREVIEW;
  cmd.cmd_data = &mod_params;
  cmd.length = sizeof(struct msm_camera_vfe_params_t);

  rc = cfg_ctrl->comp_ops[MCTL_COMPID_AXI].process(
             cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_STOP failed %d\n", __func__, rc);
    return rc;
  }

  if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
      return rc;
    }
  }

   /* stop camif: */
  if (cfg_ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
      return rc;
    }
  }

  cfg_ctrl->curr_output_info.pending_ports = mod_params.port_info;
#endif
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_SYNC_TIMER1_DONE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_proc_MSG_ID_SYNC_TIMER1_DONE(void *cctrl,
  void *parm2)
{
  CDBG("SYNC_TIMER 1 DONE\n");
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_SYNC_TIMER2_DONE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_proc_MSG_ID_SYNC_TIMER2_DONE(void *cctrl,
  void *parm2)
{
  CDBG("SYNC_TIMER 2 DONE\n");
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_STOP_REC_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_proc_MSG_ID_STOP_REC_ACK(mctl_config_ctrl_t *cfg_ctrl,
  void *parm2)
{
  int rc = 0;
  struct msm_ctrl_cmd *ctrlCmd = cfg_ctrl->pendingCtrlCmd;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  CDBG("%s: Received VFE STOP RECORDING ACK!!!\n", __func__);
  CDBG("%s: ctrl->pendingCtrlCmd=%x \n", __func__, (int)cfg_ctrl->pendingCtrlCmd);


  if (ctrlCmd) {
    config_pp_release_hw(cfg_ctrl, ctrlCmd);

    /* Sends the CTRL_CMD_DONE to kernel for pending start vfe command. */
    CDBG("sending IOCTL_CTRL_CMD_DONE to kernel!, length=%d status=%d\n",
      ctrlCmd->length, ctrlCmd->status);

    v4l2_ioctl.ioctl_ptr = ctrlCmd;
    rc = ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE, &v4l2_ioctl);

    if (rc < 0) {
      CDBG_ERROR("%s: sending IOCTL_CTRL_CMD_DONE to kernel failed\n",
        __func__);
      return rc;
    }
    /* Clears pendingCtrlCmd in ctrl */
    if (cfg_ctrl->pendingCtrlCmd) {
      free(cfg_ctrl->pendingCtrlCmd);
      cfg_ctrl->pendingCtrlCmd = NULL;
    }
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - config_proc_MSG_ID_STOP_LS_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_proc_MSG_ID_STOP_LS_ACK(mctl_config_ctrl_t *cfg_ctrl,
  void *parm2)
{
  int rc = 0;
  struct msm_ctrl_cmd *ctrlCmd = cfg_ctrl->pendingCtrlCmd;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  CDBG_HIGH("%s: Received VFE STOP LIVESHOT ACK\n", __func__);

  if (ctrlCmd) {
    /* Sends the CTRL_CMD_DONE to kernel for pending start vfe command. */
    CDBG("sending IOCTL_CTRL_CMD_DONE to kernel!, length=%d status=%d\n",
      ctrlCmd->length, ctrlCmd->status);

    v4l2_ioctl.ioctl_ptr = ctrlCmd;
    rc = ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE, &v4l2_ioctl);

    if (rc < 0) {
      CDBG_ERROR("%s: sending IOCTL_CTRL_CMD_DONE to kernel failed\n",
        __func__);
      return rc;
    }
    /* Clears pendingCtrlCmd in ctrl */
    if (cfg_ctrl->pendingCtrlCmd) {
      free(cfg_ctrl->pendingCtrlCmd);
      cfg_ctrl->pendingCtrlCmd = NULL;
    }
  }

  return rc;
}

/*===========================================================================
FUNCTION     config_proc_event_message_1

DESCRIPTION
===========================================================================*/
extern int config_CAMERA_STOP_VIDEO(void *parm1, void *parm2, int *cmd);
int config_proc_event_message_1(void *parm1,  void *parm2)
{
  int rc = 0;
#ifdef VFE_2X
  uint32_t vfe_reconfig = 0,vfe_gamma_reconfig = 0, vfe_rolloff_reconfig = 0;
  uint32_t vfe_blklvl_reconfig = 0, vfe_5x5asf_reconfig = 0;
#endif
  mctl_config_ctrl_t *cfg_ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_cam_evt_msg *adsp =  (struct msm_cam_evt_msg *)parm2;
  assert(adsp);
  assert(adsp->type == 0);
  switch (adsp->msg_id) {
  case MSG_ID_SNAPSHOT_DONE: {
    int interface = AXI_INTF_PIXEL_0;
    rc = config_process_MSG_ID_SNAPSHOT_DONE(cfg_ctrl, &interface);
    }
    break;
  case MSG_ID_PREV_STOP_ACK:
    rc = config_process_MSG_ID_PREV_STOP_ACK(cfg_ctrl, parm2);
    break;
  case MSG_ID_UPDATE_ACK:
    rc = config_proc_MSG_ID_UPDATE_ACK(cfg_ctrl, parm2);
    break;
  case MSG_ID_OUTPUT_PRIMARY:
  case MSG_ID_OUTPUT_SECONDARY:
  case MSG_ID_OUTPUT_TERTIARY1:
  case MSG_ID_OUTPUT_TERTIARY2:
    break;
  case MSG_ID_CAMIF_ERROR:
    rc = config_proc_MSG_ID_CAMIF_ERROR(cfg_ctrl, parm2);
    break;
  case MSG_ID_STOP_REC_ACK:
    rc = config_proc_MSG_ID_STOP_REC_ACK(cfg_ctrl, parm2);
    break;
  case MSG_ID_STOP_LS_ACK:
    rc = config_proc_MSG_ID_STOP_LS_ACK(cfg_ctrl, parm2);
    break;
  case MSG_ID_SYNC_TIMER1_DONE:
    rc = config_proc_MSG_ID_SYNC_TIMER1_DONE(cfg_ctrl, parm2);
    break;
  case MSG_ID_SYNC_TIMER2_DONE:
    rc = config_proc_MSG_ID_SYNC_TIMER2_DONE(cfg_ctrl, parm2);
    break;
  default:
    CDBG_ERROR("%s: Unsupported message id: %d\n", __func__,
      adsp->msg_id);
    rc = -EINVAL;
    break;
  }
#ifdef VFE_2X
  rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_GAMMA_RECONFIG_VFE,
    (void *)&vfe_gamma_reconfig, sizeof(vfe_gamma_reconfig));
  if (rc != VFE_SUCCESS)
    CDBG_ERROR("%s VFE_GET_GAMMA_RECONFIG_VFE failed", __func__);

  rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_ROLLOFF_RECONFIG_VFE,
    (void *)&vfe_rolloff_reconfig, sizeof(vfe_rolloff_reconfig));
  if (rc != VFE_SUCCESS)
    CDBG_ERROR("%s VFE_GET_ROLLOFF_RECONFIG_VFE failed", __func__);

  rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_BLACK_LEVEL_RECONFIG_VFE,
    (void *)&vfe_blklvl_reconfig, sizeof(vfe_blklvl_reconfig));
  if (rc != VFE_SUCCESS)
    CDBG_ERROR("%s VFE_GET_BLACK_LEVEL_RECONFIG_VFE failed", __func__);

  rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_5X5ASF_RECONFIG_VFE,
    (void *)&vfe_5x5asf_reconfig, sizeof(vfe_5x5asf_reconfig));
  if (rc != VFE_SUCCESS)
    CDBG_ERROR("%s VFE_GET_5X5ASF_RECONFIG_VFE failed", __func__);

  vfe_reconfig = (vfe_gamma_reconfig || vfe_rolloff_reconfig) ||
                 (vfe_blklvl_reconfig || vfe_5x5asf_reconfig);

  CDBG("%s vfe_reconfig = %d\n", __func__, vfe_reconfig);
  if (vfe_reconfig && rc == VFE_SUCCESS) {
    CDBG("%s Reconfigure VFE\n", __func__);
    struct msm_ctrl_cmd ctrl_cmd;
    int cmd;
  rc = isp_sendcmd(cfg_ctrl->camfd, CMD_GENERAL, NULL, 0, VFE_CMD_RECONFIG_VFE);
  if (rc < 0) {
    CDBG_ERROR("vfeResetFn failed!\n");
    return rc;
  }
    cfg_ctrl->reconfig_vfe = vfe_reconfig;
   rc = config_CAMERA_STOP_VIDEO(parm1, &ctrl_cmd, &cmd);
  }
#endif
  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_sendcmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_sendcmd(int fd, int type,
  void *pCmdData, unsigned int messageSize, int cmd_id)
{
  int rc;
  struct msm_vfe_cfg_cmd cfgCmd;
  struct msm_isp_cmd ispcmd;

  ispcmd.id = cmd_id;
  ispcmd.length = messageSize;
  ispcmd.value = pCmdData;

  cfgCmd.cmd_type = type;
  cfgCmd.length = sizeof(struct msm_isp_cmd);
  cfgCmd.value = &ispcmd;

  rc = ioctl(fd, MSM_CAM_IOCTL_CONFIG_VFE, &cfgCmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_CAM_IOCTL_CONFIG_VFE failed %d\n",
      __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_plane_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_plane_info(void *cfg_ctrl , void *data)
{
  int8_t rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cfg_ctrl;
  struct img_plane_info *plane_info = (struct img_plane_info *)data;
  CDBG("%s: plane_info: buffer_type=%d, ext_mode=%d, pixelformat=%d",__func__,
      plane_info->buffer_type, plane_info->ext_mode, plane_info->pixelformat);
  if (plane_info->buffer_type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
    switch (plane_info->pixelformat) {
      case V4L2_PIX_FMT_NV21:
      case V4L2_PIX_FMT_NV12:
        if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
          plane_info->sp_y_offset = 0;
          plane_info->plane[V4L2_SINGLE_PLANE].offset =
            PAD_TO_2K(plane_info->width * plane_info->height);
          plane_info->plane[V4L2_SINGLE_PLANE].size =
            plane_info->plane[V4L2_SINGLE_PLANE].offset +
            PAD_TO_2K(plane_info->width * plane_info->height/2);
        } else if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
          plane_info->sp_y_offset= 0;
          plane_info->plane[V4L2_SINGLE_PLANE].offset =
            PAD_TO_WORD(plane_info->width * plane_info->height);
          plane_info->plane[V4L2_SINGLE_PLANE].size =
            plane_info->plane[V4L2_SINGLE_PLANE].offset +
            PAD_TO_WORD(plane_info->width * plane_info->height/2);
        } else { // Snapshot/Thumbnail
          plane_info->sp_y_offset = 0;
          plane_info->plane[V4L2_SINGLE_PLANE].offset =
              PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height));
          plane_info->plane[V4L2_SINGLE_PLANE].size =
              plane_info->plane[V4L2_SINGLE_PLANE].offset +
              PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height)/2);
        }
        break;
      case V4L2_PIX_FMT_NV61:
      case V4L2_PIX_FMT_NV16:
        if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
          plane_info->sp_y_offset = 0;
          plane_info->plane[V4L2_SINGLE_PLANE].offset =
            PAD_TO_2K(plane_info->width * plane_info->height);
          plane_info->plane[V4L2_SINGLE_PLANE].size =
            plane_info->plane[V4L2_SINGLE_PLANE].offset +
            PAD_TO_2K(plane_info->width * plane_info->height);
        } else if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
          plane_info->sp_y_offset= 0;
          plane_info->plane[V4L2_SINGLE_PLANE].offset =
            PAD_TO_WORD(plane_info->width * plane_info->height);
          plane_info->plane[V4L2_SINGLE_PLANE].size =
            plane_info->plane[V4L2_SINGLE_PLANE].offset +
            PAD_TO_WORD(plane_info->width * plane_info->height);
        } else { // Snapshot/Thumbnail
          plane_info->sp_y_offset = 0;
          plane_info->plane[V4L2_SINGLE_PLANE].offset =
              PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height));
          plane_info->plane[V4L2_SINGLE_PLANE].size =
              plane_info->plane[V4L2_SINGLE_PLANE].offset +
              PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height));
        }
        break;
      case V4L2_PIX_FMT_SBGGR10:
      case V4L2_PIX_FMT_SGBRG10:
      case V4L2_PIX_FMT_SGRBG10:
      case V4L2_PIX_FMT_SRGGB10:
      case V4L2_PIX_FMT_YUYV:
        plane_info->sp_y_offset = 0;
        plane_info->plane[V4L2_SINGLE_PLANE].offset = 0;
        plane_info->plane[V4L2_SINGLE_PLANE].size =
          PAD_TO_WORD(plane_info->width * plane_info->height);
        break;
      default:
        CDBG_ERROR("%s: pixelformat %d not supported.\n",
          __func__, plane_info->pixelformat);
        plane_info->plane[V4L2_SINGLE_PLANE].size = 0;
        rc = -1;
        break;
    }
  } else {// Multi plane buffer
    switch (plane_info->pixelformat) {
      case V4L2_PIX_FMT_NV21:
      case V4L2_PIX_FMT_NV12:
        if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
          uint32_t size, luma_size, chroma_size;
#ifdef VFE_40
          cal_video_buf_size(plane_info->width, plane_info->height,
                             &luma_size, &chroma_size, &size);
#else
          luma_size = PAD_TO_2K(plane_info->width * plane_info->height);
          chroma_size = PAD_TO_2K(plane_info->width * plane_info->height/2);
#endif
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size = luma_size;

          plane_info->plane[V4L2_MULTI_PLANE_CBCR].plane_id = V4L2_MULTI_PLANE_CBCR;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].size = chroma_size;
        } else if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
          if (ctrl->dimInfo.prev_format == CAMERA_YUV_420_YV12) {
            int chrom_stride = ((plane_info->width/2+15)/16)*16;
            int luma_stride = ((plane_info->width+15)/16)*16;
            int luma_size = luma_stride * plane_info->height;
            int chrom_size = chrom_stride *  plane_info->height / 2;

            plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
            plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
            plane_info->plane[V4L2_MULTI_PLANE_Y].size =
              PAD_TO_WORD(luma_size);

            plane_info->plane[V4L2_MULTI_PLANE_CB].plane_id = V4L2_MULTI_PLANE_CBCR;
            plane_info->plane[V4L2_MULTI_PLANE_CB].offset = 0;
            plane_info->plane[V4L2_MULTI_PLANE_CB].size =
              PAD_TO_WORD(chrom_size);

            plane_info->plane[V4L2_MULTI_PLANE_CR].plane_id = V4L2_MULTI_PLANE_CBCR;
            plane_info->plane[V4L2_MULTI_PLANE_CR].offset = 0;
            plane_info->plane[V4L2_MULTI_PLANE_CR].size =
              PAD_TO_WORD(chrom_size);
          } else {
            plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
            plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
            plane_info->plane[V4L2_MULTI_PLANE_Y].size =
              PAD_TO_WORD(plane_info->width * plane_info->height);

            plane_info->plane[V4L2_MULTI_PLANE_CBCR].plane_id = V4L2_MULTI_PLANE_CBCR;
            plane_info->plane[V4L2_MULTI_PLANE_CBCR].offset = 0;
            plane_info->plane[V4L2_MULTI_PLANE_CBCR].size =
              PAD_TO_WORD(plane_info->width * plane_info->height/2);
          }
        } else { // Snapshot/Thumbnail
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size = PAD_TO_WORD(plane_info->width *
            CEILING16(plane_info->height));
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].plane_id = V4L2_MULTI_PLANE_CBCR;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].size =
            PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height)/2);
        }
        break;
      case V4L2_PIX_FMT_NV61:
      case V4L2_PIX_FMT_NV16:
        if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size =
            PAD_TO_2K(plane_info->width * plane_info->height);

          plane_info->plane[V4L2_MULTI_PLANE_CBCR].plane_id = V4L2_MULTI_PLANE_CBCR;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].size =
            PAD_TO_2K(plane_info->width * plane_info->height);
        } else if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size =
            PAD_TO_WORD(plane_info->width * plane_info->height);

          plane_info->plane[V4L2_MULTI_PLANE_CBCR].plane_id = V4L2_MULTI_PLANE_CBCR;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].size =
            PAD_TO_WORD(plane_info->width * plane_info->height);
        } else { // Snapshot/Thumbnail
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size = PAD_TO_WORD(plane_info->width *
            CEILING16(plane_info->height));
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].plane_id = V4L2_MULTI_PLANE_CBCR;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CBCR].size =
            PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height));
        }
        break;
      case V4L2_PIX_FMT_YUV420M:
        if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size =
            PAD_TO_2K(plane_info->width * plane_info->height);

          plane_info->plane[V4L2_MULTI_PLANE_CB].plane_id = V4L2_MULTI_PLANE_CB;
          plane_info->plane[V4L2_MULTI_PLANE_CB].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CB].size =
            PAD_TO_2K(plane_info->width * plane_info->height/4);
          plane_info->plane[V4L2_MULTI_PLANE_CR].plane_id = V4L2_MULTI_PLANE_CR;
          plane_info->plane[V4L2_MULTI_PLANE_CR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CR].size =
            PAD_TO_2K(plane_info->width * plane_info->height/4);
        } else if (plane_info->ext_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size =
            PAD_TO_WORD(plane_info->width * plane_info->height);

          plane_info->plane[V4L2_MULTI_PLANE_CB].plane_id = V4L2_MULTI_PLANE_CB;
          plane_info->plane[V4L2_MULTI_PLANE_CB].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CB].size =
            PAD_TO_WORD(plane_info->width * plane_info->height/4);
          plane_info->plane[V4L2_MULTI_PLANE_CR].plane_id = V4L2_MULTI_PLANE_CR;
          plane_info->plane[V4L2_MULTI_PLANE_CR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CR].size =
            PAD_TO_WORD(plane_info->width * plane_info->height/4);
        } else { // Snapshot/Thumbnail
          plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
          plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_Y].size =
            PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height));
          plane_info->plane[V4L2_MULTI_PLANE_CB].plane_id = V4L2_MULTI_PLANE_CB;
          plane_info->plane[V4L2_MULTI_PLANE_CB].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CB].size =
            PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height)/4);
          plane_info->plane[V4L2_MULTI_PLANE_CR].plane_id = V4L2_MULTI_PLANE_CR;
          plane_info->plane[V4L2_MULTI_PLANE_CR].offset = 0;
          plane_info->plane[V4L2_MULTI_PLANE_CR].size =
            PAD_TO_WORD(plane_info->width * CEILING16(plane_info->height)/4);
        }
        break;
      case V4L2_PIX_FMT_SBGGR10:
      case V4L2_PIX_FMT_SGBRG10:
      case V4L2_PIX_FMT_SGRBG10:
      case V4L2_PIX_FMT_SRGGB10:
      case V4L2_PIX_FMT_YUYV:
        plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
        plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
        plane_info->plane[V4L2_MULTI_PLANE_Y].size =
          PAD_TO_WORD(plane_info->width * plane_info->height);
        break;
      case V4L2_PIX_FMT_STATS_AE:
      case V4L2_PIX_FMT_STATS_AWB:
      case V4L2_PIX_FMT_STATS_AF:
      case V4L2_PIX_FMT_STATS_IHST:
        plane_info->plane[V4L2_MULTI_PLANE_Y].plane_id = V4L2_MULTI_PLANE_Y;
        plane_info->plane[V4L2_MULTI_PLANE_Y].offset = 0;
        plane_info->plane[V4L2_MULTI_PLANE_Y].size =
          PAD_TO_WORD(plane_info->width * plane_info->height);
        break;
      default:
        CDBG_ERROR("%s: pixelformat %d not supported.\n", __FUNCTION__, plane_info->pixelformat);
        rc = -1;
        break;
    }
  }
  return rc;
}

void config_shutdown_pp(void *cctrl)
{
  mctl_pp_cmd_t pp_cmd;
  v4l2_video_ctrl *pvideo_ctrl;
  mctl_config_ctrl_t *ctrl = cctrl;
  int pipeline_idx;
  memset(&pp_cmd, 0, sizeof(pp_cmd));

  pvideo_ctrl = &ctrl->video_ctrl;
  if (!pvideo_ctrl->streamon_mask) {
    CDBG_HIGH("%s Camera not in streaming mode. Returning. ", __func__);
    return;
  }
  pipeline_idx = pvideo_ctrl->def_pp_idx;

  /* Shutdown all the source and destinations if active. */
  CDBG_HIGH("%s Sending QCAM_MCTL_CMD_SHUTDOWN to mctl_pp ", __func__);
  pp_cmd.cmd_type = QCAM_MCTL_CMD_SHUTDOWN;
  if (pipeline_idx >= 0)
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &pp_cmd);
  else
    CDBG_ERROR("%s Default pp pipeline is closed ", __func__);

  config_pp_end_pp_topology(ctrl, pvideo_ctrl->op_mode);
}

/*===========================================================================
 * FUNCTION     config_proc_send_zoom_done_event
 *
 * DESCRIPTION: ToDo: Don't know where to out this code.
 *===========================================================================*/
int config_proc_send_zoom_done_event(void *parm1,  void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *cfg_ctrl = (mctl_config_ctrl_t *)parm1;
  struct v4l2_event_and_payload event;
  mm_camera_event_t *cam_event = (mm_camera_event_t *)event.evt.u.data;

  /* Send events of zoom success. */
  CDBG("%s: E", __func__);
  event.payload_length = 0;
  event.transaction_id = -1;
  event.payload = NULL;
  event.evt.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_APP_NOTIFY_EVENT;
  cam_event->event_type = MM_CAMERA_EVT_TYPE_CTRL;
  cam_event->e.ctrl.evt = MM_CAMERA_CTRL_EVT_ZOOM_DONE;
  cam_event->e.ctrl.status = CAM_CTRL_SUCCESS;
  rc = ioctl(cfg_ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &event);
  if (rc < 0)
    CDBG_ERROR("%s: MM_CAMERA_CTRL_EVT_ZOOM_DONE failed w/ rc=%d", __func__, rc);

  return rc;
} /* config_proc_send_zoom_done_event */

/*===========================================================================
 * FUNCTION    - config_CAMERA_SET_PARM_ZOOM -
 *
 * DESCRIPTION:
 *==========================================================================*/
int32_t config_proc_zoom(void *parm1, int32_t zoom_val)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  sensor_get_t sensor_get;
  v4l2_video_ctrl *pvideo_ctrl = &ctrl->video_ctrl;

  CDBG("%s: E, zoom_val = %d, streamon_mask = 0x%x",
       __func__, zoom_val, ctrl->video_ctrl.streamon_mask);

  ctrl->zoomCtrl.zoom_val = zoom_val;
  ctrl->zoom_done_pending = 1;
  if ((ctrl->video_ctrl.streamon_mask
    || ctrl->video_ctrl.streamon_bundle_mask) &&
      ((ctrl->state == CAMERA_STATE_SENT_START)
    || (ctrl->state == CAMERA_STATE_STARTED))) {
    uint32_t crop_factor = 0;
    zoom_scaling_params_t zoomscaling;
    vfe_zoom_crop_info_t zoom_crop_info;
    rc = zoom_get_parms(&ctrl->zoomCtrl, ZOOM_PARM_GET_CROP_FACTOR,
                        (void *)&zoom_val, (void *)&crop_factor);
    if(rc < 0) {
      CDBG_ERROR("%s: ZOOM_PARM_GET_CROP_FACTOR err = %d", __func__, rc);
      goto end;
    }
    if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
      rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_FOV_CROP_FACTOR, &crop_factor, NULL);
      if(rc < 0) {
        CDBG_ERROR("%s: VFE_SET_FOV_CROP_FACTOR err = %d", __func__, rc);
        goto end;
      }

      rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
                 ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                 VFE_GET_ZOOM_CROP_INFO, (void *)&zoom_crop_info,
                 sizeof(zoom_crop_info));
      if(rc < 0) {
        CDBG_ERROR("%s: VFE_GET_ZOOM_CROP_INFO err = %d", __func__, rc);
        goto end;
      }
    }
    rc = zoom_get_parms(&ctrl->zoomCtrl, ZOOM_PARM_GET_SCALING_INFO,
                        (void *)&zoom_crop_info, (void *)&zoomscaling);
    if(rc < 0) {
      CDBG_ERROR("%s: ZOOM_PARM_GET_SCALING_INFO err = %d", __func__, rc);
      goto end;
    }
    config_send_crop_to_mctl_pp(ctrl, &ctrl->video_ctrl, &zoomscaling);
    ctrl->zoom_done_pending = 0;
    if (config_proc_send_zoom_done_event(ctrl, NULL) < 0) {
      CDBG_ERROR("%s: config_proc_send_zoom_done_event failed w/ rc=%d",
        __func__, rc);
    }
  }
  end:
  CDBG("%s: X", __func__);
  return rc;
}

int config_proc_face_detection_cmd (void *parm1, int fd_mode)
{
  int32_t is_fd_on = 0, rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;

  CDBG("%s: value: %d", __func__, fd_mode);
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  frame_proc_set_t fd_set_parm;
  frame_proc_key_t fp_key;
  if(ctrl->video_ctrl.op_mode == MSM_V4L2_CAM_OP_VIDEO ||
     ctrl->video_ctrl.op_mode ==  MSM_V4L2_CAM_OP_ZSL) {
    if(!ctrl->is_fd_on ||
       (ctrl->video_ctrl.streamon_mask & V4L2_DEV_STRAEMON_BIT_P)) {
      if (fd_mode > 0)
        is_fd_on = 1;
      else
        is_fd_on = 0;
      fp_ctrl->intf.input.mctl_info.display_dim.width =
        ctrl->dimInfo.display_width;
      fp_ctrl->intf.input.mctl_info.display_dim.height =
        ctrl->dimInfo.display_height;
      if (ctrl->comp_mask & (1 << MCTL_COMPID_FRAMEPROC)) {
          fd_set_parm.type = FRAME_PROC_FACE_DETECT;
          fd_set_parm.d.set_fd.type = FACE_DETECT_ENABLE;
          fd_set_parm.d.set_fd.fd_enable = is_fd_on;
          ctrl->is_fd_on = is_fd_on;
          fd_set_parm.d.set_fd.mode = FACE_DETECT;
          fd_set_parm.d.set_fd.num_fd = ctrl->num_fd;
          if (ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, fd_set_parm.type,
            &fd_set_parm, &(fp_ctrl->intf))<0) {
            CDBG_ERROR("%s  Frame proc set param failed for Face Detect",
              __func__);
            return -1;
          }
          if (fd_mode == FACE_REGISTER) {
            fp_key = FP_SNAPSHOT_SET;
            rc = mctl_divert_set_key(ctrl, fp_key);
          } else if (fd_mode == FACE_DETECT || fd_mode == FACE_RECOGNIZE) {
            fp_key = FP_PREVIEW_SET;
            rc = mctl_divert_set_key(ctrl, fp_key);
          } else if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
            stats_proc_set_t set_param;
            set_param.type = STATS_PROC_AEC_TYPE;
            set_param.d.set_aec.type = AEC_SET_FD_ROI;
            set_param.d.set_aec.d.fd_roi.num_roi = 0;
            set_param.d.set_aec.d.fd_roi.frm_id = 0;
            rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
              ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
              set_param.type, &set_param,
              &(sp_ctrl->intf));
            if (rc) {
              CDBG_ERROR("Failed to set AEC_SET_FD_ROI");
            }
          }
          /* Ensure to disable diverts for all non active FACE MODE */
          if (!fp_ctrl->intf.output.share_d.divert_snapshot &&
            !fp_ctrl->intf.output.share_d.divert_preview) {
            fp_key = FP_RESET;
            rc = mctl_divert_set_key(ctrl, fp_key);
          } else if (!fp_ctrl->intf.output.share_d.divert_preview) {
            fp_key = FP_PREVIEW_RESET;
            rc = mctl_divert_set_key(ctrl, fp_key);
          } else if (!fp_ctrl->intf.output.share_d.divert_snapshot) {
            fp_key = FP_SNAPSHOT_RESET;
            rc = mctl_divert_set_key(ctrl, fp_key);
          }
      }
    }
  }
  return 0;
}

int config_proc_v4l2fmt_to_camfmt(uint32_t v4l2_pixfmt)
{
  cam_format_t camfmt = CAMERA_YUV_420_NV21;

  switch(v4l2_pixfmt) {
  case V4L2_PIX_FMT_NV12:
    camfmt = CAMERA_YUV_420_NV12;
    break;
  case V4L2_PIX_FMT_NV21:
    camfmt = CAMERA_YUV_420_NV21;
    break;
  case V4L2_PIX_FMT_SBGGR10:
    camfmt = CAMERA_BAYER_SBGGR10;
    break;
  case V4L2_PIX_FMT_NV61:
    camfmt = CAMERA_YUV_422_NV61;
    break;
  case V4L2_PIX_FMT_YUV420M:
    camfmt = CAMERA_YUV_420_YV12;
    break;
  case V4L2_PIX_FMT_STATS_AE:
    camfmt = CAMERA_SAEC;
  break;
  case V4L2_PIX_FMT_STATS_AWB:
    camfmt = CAMERA_SAWB;
  break;
  case V4L2_PIX_FMT_STATS_AF:
    camfmt = CAMERA_SAFC;
  break;
  case V4L2_PIX_FMT_STATS_IHST:
    camfmt = CAMERA_SHST;
  break;
  default:
    break;
  }
  return camfmt;
}

uint32_t config_get_inst_handle(cam_stream_info_t *strm_info,
  int search, uint32_t image_mode)
{
  int i;
  uint32_t inst_handle = 0x0;

  switch(search) {
  case STRM_INFO_USER:
    for (i = 0; i < MSM_MAX_DEV_INST; i++) {
      if (strm_info->user[i].image_mode == image_mode) {
        inst_handle = strm_info->user[i].inst_handle;
        break;
      }
    }
  break;
  case STRM_INFO_MCTL:
    for (i = 0; i < MSM_MAX_DEV_INST; i++) {
      if (strm_info->mctl[i].image_mode == image_mode) {
        inst_handle = strm_info->mctl[i].inst_handle;
        break;
      }
    }
  break;
  case STRM_INFO_STATS:
    for (i = 0; i < MSM_MAX_DEV_INST; i++) {
      if (strm_info->stats[i].image_mode == image_mode) {
        inst_handle = strm_info->stats[i].inst_handle;
        break;
      }
    }
  break;
  default:
    CDBG_ERROR("%s: Invalid Stream type : %d\n",
      __func__, search);
  break;
  }

  return inst_handle;
}
