/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "wb32.h"
#include "isp_log.h"

#ifdef ENABLE_WB_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

/** wb_debug:
 *    @mce_mod: Pointer to wb configuration.
 *
 * Print wb configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void wb_debug(ISP_WhiteBalanceConfigCmdType* p_cmd)
{
  ISP_DBG(ISP_MOD_WB, "ISP_WhiteBalanceCfgCmd.ch0Gain = %d\n", p_cmd->ch0Gain);
  ISP_DBG(ISP_MOD_WB, "ISP_WhiteBalanceCfgCmd.ch1Gain = %d\n", p_cmd->ch1Gain);
  ISP_DBG(ISP_MOD_WB, "ISP_WhiteBalanceCfgCmd.ch2Gain = %d\n", p_cmd->ch2Gain);
} /*wb_debug*/

/** wb_update_hw_gain_reg:
 *    @wb_mod: pointer to instance private data
 *
 * Update wb coefficients.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void wb_update_hw_gain_reg(isp_wb_mod_t *wb_mod)
{
  wb_mod->ISP_WhiteBalanceCfgCmd.ch0Gain =
    WB_GAIN(wb_mod->awb_gain.g_gain * wb_mod->dig_gain);
  wb_mod->ISP_WhiteBalanceCfgCmd.ch1Gain =
    WB_GAIN(wb_mod->awb_gain.b_gain * wb_mod->dig_gain);
  wb_mod->ISP_WhiteBalanceCfgCmd.ch2Gain =
    WB_GAIN(wb_mod->awb_gain.r_gain * wb_mod->dig_gain);
}

/** wb_set_manual_wb:
 *    @mod: pointer to instance private data
 *    @trigger_params: input data
 *    @in_params_size: size of input data
 *
 * Set manual wb.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_set_manual_wb(isp_wb_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;

  awb_gain_t *awb_gain =
    (awb_gain_t*)&trigger_params->trigger_input.stats_update.awb_update.gain;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  /* Agreed sequence with pix pipeline before stream started is that,
   * (1) config wb
   * (2) set manual WB
   * (3) action_hw_update.
   * After stream started, set_wb uses trigger_update the wb gain. */

  if (IS_MANUAL_WB((trigger_params))) {
    mod->awb_gain.g_gain = awb_gain->g_gain;
    mod->awb_gain.b_gain = awb_gain->b_gain;
    mod->awb_gain.r_gain = awb_gain->r_gain;
    wb_update_hw_gain_reg(mod);
    mod->hw_update_pending = TRUE;
  }

  return rc;
} /*wb_set_manual_wb*/

/** wb_trigger_update:
 *    @wb_mod: pointer to instance private data
 *    @trigger_params: input data
 *    @in_params_size: size of input data
 *
 * Update configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_trigger_update(isp_wb_mod_t *wb_mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  awb_gain_t *awb_gain =
    (awb_gain_t*)&trigger_params->trigger_input.stats_update.awb_update.gain;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  if (!wb_mod->enable || !wb_mod->trigger_enable) {
    ISP_DBG(ISP_MOD_WB, "%s: enable = %d, trigger_enable = %d, manual_wb = %d", __func__,
      wb_mod->enable, wb_mod->trigger_enable,
      trigger_params->trigger_input.stats_update.awb_update.wb_mode);
    return 0;
  }

  if (!WB_GAIN_EQUAL(awb_gain, wb_mod->awb_gain) && !WB_GAIN_EQ_ZERO(awb_gain)) {
    wb_mod->awb_gain.g_gain = awb_gain->g_gain;
    wb_mod->awb_gain.b_gain = awb_gain->b_gain;
    wb_mod->awb_gain.r_gain = awb_gain->r_gain;
    wb_update_hw_gain_reg(wb_mod);
    wb_mod->hw_update_pending = TRUE;
  }

  return rc;
} /* wb_trigger_update */

/** wb_set_bestshot:
 *    @mod: pointer to instance private data
 *    @pix_settings: input data
 *    @in_params_size: size of input data
 *
 * Set wb scene mode.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_set_bestshot(isp_wb_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int rc = 0;
  chromatix_parms_type *chromatix_ptr =
    (chromatix_parms_type *)pix_settings->chromatix_ptrs.chromatixPtr;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  switch (pix_settings->bestshot_mode) {
  case CAM_SCENE_MODE_FIREWORKS:
    pix_settings->wb_mode = CAM_WB_MODE_CLOUDY_DAYLIGHT;
    break;
  case CAM_SCENE_MODE_SUNSET:
  case CAM_SCENE_MODE_CANDLELIGHT:
    pix_settings->wb_mode = CAM_WB_MODE_INCANDESCENT;
    break;
  default:
    pix_settings->wb_mode = CAM_WB_MODE_AUTO;
    break;
  }
  return rc;
} /* isp_wb_set_effect */

/** wb_init:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @notify_ops: notify
 *
 * Open and initialize all required submodules
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_wb_mod_t *wb = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  wb->fd = init_params->fd;
  wb->notify_ops = notify_ops;
  wb->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
} /* wb_init */

/** wb_config:
 *    @mod: pointer to instance private data
 *    @pix_setting: input data
 *    @in_params_size: size of input data
 *
 * Configure module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_config(isp_wb_mod_t *mod, isp_hw_pix_setting_params_t *in_params,
  uint32_t in_param_size)
{
  int rc = 0;
  uint32_t i;
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_MWB_type *chromatix_MWB = &chroma_ptr->chromatix_MWB;
  Bayer_AWB_parameters_type *AWB_bayer_algo_data =
    &chroma_ptr->AWB_bayer_algo_data;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  mod->awb_gain.b_gain = chromatix_MWB->MWB_tl84.b_gain *
    AWB_bayer_algo_data->gain_adj[AGW_AWB_WARM_FLO].blue_gain_adj;
  mod->awb_gain.r_gain = chromatix_MWB->MWB_tl84.b_gain *
    AWB_bayer_algo_data->gain_adj[AGW_AWB_WARM_FLO].red_gain_adj;
  mod->awb_gain.g_gain = chromatix_MWB->MWB_tl84.b_gain;

  mod->dig_gain = 1.0; //no apply aec gain for now, could be used in the future

  wb_update_hw_gain_reg(mod);

  mod->hw_update_pending = TRUE;

  return rc;
}

/** wb_enable:
 *    @mod: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Enable module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_enable(isp_wb_mod_t *wb, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  wb->enable = enable->enable;

  return 0;
}

/** wb_trigger_update:
 *    @mod: pointer to instance private data
 *    @trigger_params: input data
 *    @in_params_size: size of input data
 *
 * Enable hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_trigger_enable(isp_wb_mod_t *wb, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  wb->trigger_enable = enable->enable;

  return 0;
}

/** wb_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_destroy(void *mod_ctrl)
{
  isp_wb_mod_t *wb = mod_ctrl;

  memset(wb, 0, sizeof(isp_wb_mod_t));
  free(wb);
  return 0;
}

/** wb_set_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set parameter function. It handle all input parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_wb_mod_t *wb = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = wb_enable(wb, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = wb_config(wb, (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = wb_trigger_enable(wb, (isp_mod_set_enable_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = wb_trigger_update(wb, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_BESTSHOT:
    rc = wb_set_bestshot(wb, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_MANUAL_WB:
    rc = wb_set_manual_wb(wb, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/** wb_get_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *    @out_params: output data
 *    @out_params_size: size of output data
 *
 * Get parameter function. It handle all parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_get_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  int rc = 0;
  isp_wb_mod_t *wb = mod_ctrl;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    enable->enable = wb->enable;
    break;
  }

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    vfe_diag->control_wb.enable = wb->enable;
    vfe_diag->control_wb.cntrlenable = wb->trigger_enable;

    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_WB, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/** wb_do_hw_update:
 *    @wb_mod: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_do_hw_update(isp_wb_mod_t *wb_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (wb_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)&wb_mod->ISP_WhiteBalanceCfgCmd;
    cfg_cmd.cmd_len = sizeof(wb_mod->ISP_WhiteBalanceCfgCmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_WB32_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_WB32_LEN * sizeof(uint32_t);

    wb_debug(&wb_mod->ISP_WhiteBalanceCfgCmd);
    rc = ioctl(wb_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    wb_mod->hw_update_pending = 0;
  }

  return rc;
}

/** wb_action:
 *    @mod_ctrl: pointer to instance private data
 *    @action_code: action id
 *    @action_data: action data
 *    @action_data_size: action data size
 *
 * Handle all actions.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int wb_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_wb_mod_t *wb = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = wb_do_hw_update(wb);
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

/** wb32_open:
 *    @version: version of isp
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *wb32_open(uint32_t version)
{
  isp_wb_mod_t *wb = malloc(sizeof(isp_wb_mod_t));

  if (!wb) {
    /* no memory */
    CDBG_ERROR("%s: no mem", __func__);
    return NULL;
  }
  memset(wb, 0, sizeof(isp_wb_mod_t));
  wb->ops.ctrl = (void *)wb;
  wb->ops.init = wb_init;
  wb->ops.destroy = wb_destroy;
  wb->ops.set_params = wb_set_params;
  wb->ops.get_params = wb_get_params;
  wb->ops.action = wb_action;
  return &wb->ops;
}

