/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "demux32.h"
#include "isp_log.h"

#ifdef ENABLE_DEMUX_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif


#if 0
#undef ISP_DBG
#define ISP_DBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

#define VFE_CMD_DEMUX_R_CHANNEL_GAIN_CONFIG 200 /* TODO: need to clean this, 3D used.*/

#define MAX_DEMUX_GAIN 31.9  /* max 32x, should we use 32? */

#define INIT_GAIN(p_gain, val) ( {\
  p_gain.blue = val; \
  p_gain.red = val; \
  p_gain.green_even = val; \
  p_gain.green_odd = val; \
})

/*===========================================================================
 * FUNCTION    - demux_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void demux_debug(ISP_DemuxConfigCmdType* pcmd)
{
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.ch0OddGain = %d\n",
    pcmd->ch0OddGain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.ch0EvenGain = %d\n",
    pcmd->ch0EvenGain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.ch1Gain = %d\n",
    pcmd->ch1Gain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.ch2Gain = %d\n",
    pcmd->ch2Gain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.period = %d\n",
    pcmd->period);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.evenCfg = %d\n",
    pcmd->evenCfg);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxConfigCmd.oddCfg = %d\n",
    pcmd->oddCfg);
}

/*===========================================================================
 * FUNCTION    - demux_gain_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void demux_gain_debug(ISP_DemuxGainCfgCmdType* pcmd)
{
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxGainCfgCmdType.ch0OddGain = %d\n",
    pcmd->ch0OddGain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxGainCfgCmdType.ch0EvenGain = %d\n",
    pcmd->ch0EvenGain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxGainCfgCmdType.ch1Gain = %d\n",
    pcmd->ch1Gain);
  ISP_DBG(ISP_MOD_DEMUX, "ISP_DemuxGainCfgCmdType.ch2Gain = %d\n",
    pcmd->ch2Gain);
}
/*===========================================================================
 * FUNCTION    - demux_r_image_gain_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int demux_r_image_gain_update(isp_demux_mod_t *demux)
{
  ISP_DemuxGainCfgCmdType* p_gaincmd = &demux->ISP_RImageGainConfigCmd;

  p_gaincmd->ch0OddGain = DEMUX_GAIN(demux->r_gain.green_odd);
  p_gaincmd->ch0EvenGain = DEMUX_GAIN(demux->r_gain.green_even);
  p_gaincmd->ch1Gain = DEMUX_GAIN(demux->r_gain.blue);
  p_gaincmd->ch2Gain = DEMUX_GAIN(demux->r_gain.red);

  return 0;
}

/*===========================================================================
 * FUNCTION    - demux_set_cfg_parms -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int demux_set_cfg_parms(ISP_DemuxConfigCmdType* pcmd,
  cam_format_t fmt)
{
  int rc = TRUE;
  enum ISP_START_PIXEL_PATTERN pix_pattern;

  /* Configure VFE input format, Send Input format command */
  ISP_DBG(ISP_MOD_DEMUX, "%s: format %d", __func__, fmt);

  pix_pattern = isp_fmt_to_pix_pattern(fmt);
  switch (pix_pattern) {
    /* bayer patterns */
    case ISP_BAYER_GBGBGB:
      pcmd->period = 1;
      pcmd->evenCfg = 0xAC;
      pcmd->oddCfg = 0xC9;
      break;
    case ISP_BAYER_BGBGBG:
      pcmd->period = 1;
      pcmd->evenCfg = 0xCA;
      pcmd->oddCfg = 0x9C;
      break;
    case ISP_BAYER_GRGRGR:
      pcmd->period = 1;
      pcmd->evenCfg = 0x9C;
      pcmd->oddCfg = 0xCA;
      break;
    case ISP_BAYER_RGRGRG:
      pcmd->period = 1;
      pcmd->evenCfg = 0xC9;
      pcmd->oddCfg = 0xAC;
      break;
      /* YCbCr Patterns */
    case ISP_YUV_YCbYCr:
      pcmd->period = 3;
      pcmd->evenCfg = 0x9CAC;
      pcmd->oddCfg = 0x9CAC;
      break;
    case ISP_YUV_YCrYCb:
      pcmd->period = 3;
      pcmd->evenCfg = 0xAC9C;
      pcmd->oddCfg = 0xAC9C;
      break;
    case ISP_YUV_CbYCrY:
      pcmd->period = 3;
      pcmd->evenCfg =0xC9CA;
      pcmd->oddCfg = 0xC9CA;
      break;
    case ISP_YUV_CrYCbY:
      pcmd->period = 3;
      pcmd->evenCfg =0xCAC9;
      pcmd->oddCfg =0xCAC9;
      break;
    default:
      CDBG_ERROR("Error ISP input not configured!!!\n");
      rc = FALSE;
      break;
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - demux_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int demux_trigger_update(isp_demux_mod_t *demux,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  isp_hw_pix_setting_params_t *pix_setting = &trigger_params->cfg;
  chromatix_parms_type *chromatix_ptr;
  chromatix_channel_balance_gains_type *chromatix_channel_balance_gains;
  AEC_algo_struct_type *AEC_algo_data;
  ISP_DemuxConfigCmdType* p_cmd = &demux->ISP_DemuxConfigCmd;
  float max_gain = 0.0, gain = 0.0, new_dig_gain = 1.0;
  float max_ch_gain = 0.0;
  demux->remaining_digital_gain = 1.0; /* set to unitity */

  if(!demux->enable || !demux->trigger_enable) {
    ISP_DBG(ISP_MOD_DEMUX, "%s: no trigger update for DEMUX, trigger_enable = %d, enable =%d\n",
         __func__, demux->trigger_enable, demux->enable);
    return 0;
  }

  if (trigger_params->cfg.camif_cfg.is_bayer_sensor) {
    chromatix_ptr = pix_setting->chromatix_ptrs.chromatixPtr;
    chromatix_channel_balance_gains =
        &chromatix_ptr->chromatix_VFE.chromatix_channel_balance_gains;
    AEC_algo_data = &chromatix_ptr->AEC_algo_data;

    ISP_DBG(ISP_MOD_DEMUX, "%s: dig gain %5.3f", __func__,
      trigger_params->trigger_input.digital_gain);
    if (trigger_params->trigger_input.digital_gain < 1.0)
      trigger_params->trigger_input.digital_gain = 1.0;

    max_ch_gain = MAX(chromatix_channel_balance_gains->green_odd,
        MAX(chromatix_channel_balance_gains->green_even,
        MAX(chromatix_channel_balance_gains->red,
          chromatix_channel_balance_gains->blue)));

    ISP_DBG(ISP_MOD_DEMUX, "%s: max_ch_gain %5.3f glob %5.3f", __func__, max_ch_gain,
      AEC_algo_data->color_correction_global_gain);
    max_gain = max_ch_gain * AEC_algo_data->color_correction_global_gain *
        trigger_params->trigger_input.digital_gain;

    ISP_DBG(ISP_MOD_DEMUX, "%s: max_gain_final %5.3f", __func__, max_gain);
    if (max_gain > MAX_DEMUX_GAIN) {
      new_dig_gain = MAX_DEMUX_GAIN/
        (AEC_algo_data->color_correction_global_gain * max_ch_gain);
      demux->remaining_digital_gain = max_gain/MAX_DEMUX_GAIN;
    } else {
      new_dig_gain = max_gain/
        (AEC_algo_data->color_correction_global_gain * max_ch_gain);
    }

    ISP_DBG(ISP_MOD_DEMUX, "%s: dig_gain_old %5.3f new %5.3f", __func__, demux->dig_gain,
      new_dig_gain);
    if (F_EQUAL(demux->dig_gain, new_dig_gain)
      && (pix_setting->streaming_mode == demux->old_streaming_mode)) {
      ISP_DBG(ISP_MOD_DEMUX, "%s: No update required", __func__);
      return 0;
    }
    demux->old_streaming_mode = pix_setting->streaming_mode;
    demux->dig_gain = new_dig_gain;

    gain = AEC_algo_data->color_correction_global_gain * demux->dig_gain;
    demux->gain.green_odd = gain *
      chromatix_channel_balance_gains->green_odd;
    demux->gain.green_even= gain *
      chromatix_channel_balance_gains->green_even;
    demux->gain.red = gain *
      chromatix_channel_balance_gains->red;
    demux->gain.blue = gain *
      chromatix_channel_balance_gains->blue;
  }

  p_cmd->ch0EvenGain = DEMUX_GAIN(demux->gain.green_even);
  p_cmd->ch0OddGain = DEMUX_GAIN(demux->gain.green_odd);
  p_cmd->ch1Gain = DEMUX_GAIN(demux->gain.blue);
  p_cmd->ch2Gain = DEMUX_GAIN(demux->gain.red);

  /* 3D is not supported. */
  if (demux->is_3d)
    demux_r_image_gain_update(demux);

  demux->hw_update_pending = TRUE;

  return 0;
}/*demux_trigger_update*/
/*===========================================================================
 * FUNCTION    - demux_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int demux_config(isp_demux_mod_t *demux, isp_hw_pix_setting_params_t *pix_settings,
                     uint32_t in_param_size)
{
  chromatix_parms_type *chromatix_ptr = pix_settings->chromatix_ptrs.chromatixPtr;
  AEC_algo_struct_type *AEC_algo_data = &chromatix_ptr->AEC_algo_data;
  ISP_DemuxConfigCmdType* p_cmd = &demux->ISP_DemuxConfigCmd;
  demux->dig_gain = 1.0;
  demux->remaining_digital_gain = 1.0;
  if (pix_settings->camif_cfg.is_bayer_sensor) {
    INIT_GAIN(demux->gain, AEC_algo_data->color_correction_global_gain);
  } else {
    INIT_GAIN(demux->gain, 1.0);
  }
  INIT_GAIN(demux->r_gain, 1.0);

  demux_set_cfg_parms(p_cmd, pix_settings->camif_cfg.sensor_output_fmt);
  ISP_DBG(ISP_MOD_DEMUX, "%s: sensor input format : %d \n", __func__,
    pix_settings->camif_cfg.sensor_output_fmt);

  ISP_DBG(ISP_MOD_DEMUX, "%s %5.2f %5.2f %5.2f %5.2f ", __func__,
    demux->gain.green_odd,
    demux->gain.green_even,
    demux->gain.red,
    demux->gain.blue);

  p_cmd->ch0EvenGain = DEMUX_GAIN(demux->gain.green_even);
  p_cmd->ch0OddGain = DEMUX_GAIN(demux->gain.green_odd);
  p_cmd->ch1Gain = DEMUX_GAIN(demux->gain.blue);
  p_cmd->ch2Gain = DEMUX_GAIN(demux->gain.red);

  demux_debug(p_cmd);

  /* 3D is not supported. */
  if (demux->is_3d) {
    demux_r_image_gain_update(demux);
  }
  demux->hw_update_pending = TRUE;
  return 0;
}
/* ============================================================
 * function name: demux_enable
 * description: enable demux
 * ============================================================*/
static int demux_enable(isp_demux_mod_t *demux,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  ISP_DBG(ISP_MOD_DEMUX, "%s: enable=%d", __func__, demux->enable);

  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  demux->enable = enable->enable;

  return 0;
}
/* ============================================================
 * function name: demux_trigger_enable
 * description: enable trigger update feature
 * ============================================================*/
static int demux_trigger_enable(isp_demux_mod_t *demux,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  demux->trigger_enable = enable->enable;

  return 0;
}

/* ============================================================
 * function name: demux_destroy
 * description: close demux
 * ============================================================*/
static int demux_destroy (void *mod_ctrl)
{
  isp_demux_mod_t *demux = mod_ctrl;

  memset(demux,  0,  sizeof(isp_demux_mod_t));
  free(demux);
  return 0;
}
/* ============================================================
 * function name: demux_set_params
 * description: set parameters
 * ============================================================*/
static int demux_set_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_demux_mod_t *demux = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = demux_enable(demux, (isp_mod_set_enable_t *)in_params,
                     in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = demux_config(demux, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = demux_trigger_enable(demux, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = demux_trigger_update(demux, in_params, in_param_size);
    break;
  default:
    CDBG_ERROR("%s: param_id %d, is not supported in this module\n", __func__, param_id);
    break;
  }
  return rc;
}

/** demux_ez_isp_update
 *  @demux_module
 *  @gain
 *
 **/
static void demux_ez_isp_update(
  isp_demux_mod_t *demux_module,
  demuxchannelgain_t *gain)
{
  ISP_DemuxConfigCmdType *demuxchgainCfg;
  demuxchgainCfg = &(demux_module->applied_cmd);
  gain->greenEvenRow = demuxchgainCfg->ch0EvenGain;
  gain->greenOddRow = demuxchgainCfg->ch0OddGain;
  gain->blue = demuxchgainCfg->ch1Gain;
  gain->red = demuxchgainCfg->ch2Gain;
}
/* ============================================================
 * function name: demux_get_params
 * description: get parameters
 * ============================================================*/
static int demux_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_demux_mod_t *demux = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = demux->enable;
    break;
  }
  case ISP_HW_MOD_GET_APPLIED_DIG_GAIN:{
    isp_hw_mode_get_applied_dig_gain_t *demux_dig_gain = out_params;
    demux_dig_gain->applied_dig_gain = demux->dig_gain;
    break;
  }

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    demuxchannelgain_t *gain;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    gain = &(vfe_diag->prev_demuxchannelgain);
    if(demux->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
        gain = &(vfe_diag->snap_demuxchannelgain);
    }
    vfe_diag->control_demux.enable = demux->enable;
    vfe_diag->control_demux.cntrlenable = demux->trigger_enable;
    demux_ez_isp_update(demux, gain);
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_DEMUX, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default:
    rc = -EPERM;
    break;
  }
  return rc;
}
static int demux_do_hw_update(isp_demux_mod_t *demux_mod)
{
  int i, rc = 0;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  ISP_DBG(ISP_MOD_DEMUX, "%s: hw_update_pending=%d", __func__,
    demux_mod->hw_update_pending);

  if (demux_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &demux_mod->ISP_DemuxConfigCmd;
    cfg_cmd.cmd_len = sizeof(demux_mod->ISP_DemuxConfigCmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DEMUX32_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_DEMUX32_LEN * sizeof(uint32_t);

    demux_debug(&demux_mod->ISP_DemuxConfigCmd);
    rc = ioctl(demux_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    demux_mod->applied_cmd = demux_mod->ISP_DemuxConfigCmd;
    demux_mod->hw_update_pending = 0;
  }

  return rc;
}

/* ============================================================
 * function name: demux_action
 * description: processing the action
 * ============================================================*/
static int demux_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_demux_mod_t *demux = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = demux_do_hw_update(demux);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}
/* ============================================================
 * function name: demux_init
 * description: init
 * ============================================================*/
static int demux_init (void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_demux_mod_t *demux = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  demux->fd = init_params->fd;
  demux->notify_ops = notify_ops;
  demux->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
}
/* ============================================================
 * function name: demux32_open
 * description: open demux
 * ============================================================*/
isp_ops_t *demux32_open(uint32_t version)
{
  isp_demux_mod_t *demux = malloc(sizeof(isp_demux_mod_t));

  if (!demux) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(demux,  0,  sizeof(isp_demux_mod_t));
  demux->ops.ctrl = (void *)demux;
  demux->ops.init = demux_init;
  /* destroy the module object */
  demux->ops.destroy = demux_destroy;
  /* set parameter */
  demux->ops.set_params = demux_set_params;
  /* get parameter */
  demux->ops.get_params = demux_get_params;
  demux->ops.action = demux_action;
  return &demux->ops;
}

