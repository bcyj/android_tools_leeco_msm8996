/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "bcc32.h"
#include "isp_log.h"

#ifdef ENABLE_BCC_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#ifndef sign
#define sign(x) (((x) < 0) ? (-1) : (1))
#endif

#ifndef Round
#define Round(x) (int)((x) + sign(x)*0.5)
#endif

/** util_bcc_cmd_debug:
 *    @cmd: Pointer to statistic configuration.
 *
 * Print mce configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void util_bcc_cmd_debug(ISP_DemosaicDBCC_CmdType *cmd)
{
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->fminThreshold = %d \n", __func__, cmd->fminThreshold);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->fmaxThreshold = %d \n", __func__, cmd->fmaxThreshold);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->rOffsetHi = %d \n", __func__, cmd->rOffsetHi);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->rOffsetLo = %d \n", __func__, cmd->rOffsetLo);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->bOffsetHi = %d \n", __func__, cmd->bOffsetHi);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->bOffsetLo = %d \n", __func__, cmd->bOffsetLo);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->grOffsetHi = %d \n", __func__, cmd->grOffsetHi);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->grOffsetLo = %d \n", __func__, cmd->grOffsetLo);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->gbOffsetHi = %d \n", __func__, cmd->gbOffsetHi);
  ISP_DBG(ISP_MOD_BCC, "%s: cmd->gbOffsetLo = %d \n", __func__, cmd->gbOffsetLo);
} /* util_bcc_cmd_debug */

/** util_bcc_cmd_config:
 *    @mod: pointer to instance private data
 *
 * Update hardware configuration structures.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void util_bcc_cmd_config(isp_bcc_mod_t *mod)
{
  mod->RegCfgCmd.enable = mod->enable;
  mod->RegCmd.fminThreshold = MIN(63, mod->p_params.Fmin);
  mod->RegCmd.fmaxThreshold = MIN(127, mod->p_params.Fmax);
  mod->RegCmd.rOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_r_hi;
  mod->RegCmd.rOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_r_lo;
  mod->RegCmd.bOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_b_hi;
  mod->RegCmd.bOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_b_lo;
  mod->RegCmd.grOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_gr_lo;
  mod->RegCmd.grOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_gr_hi;
  mod->RegCmd.gbOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_gb_lo;
  mod->RegCmd.gbOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_gb_hi;
} /* util_bcc_cmd_config */

/** bcc_config:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Configure module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bcc_config(isp_bcc_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  chromatix_parms_type *chromatix_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_BPC_type *chromatix_BPC =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  if (!mod->enable) {
    ISP_DBG(ISP_MOD_BCC, "%s: Mod not Enable.", __func__);
    return rc;
  }

  /* set old cfg to invalid value to trigger the first trigger update */
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  mod->p_params.p_input_offset =
    &(chromatix_BPC->bcc_4_offset[BPC_NORMAL_LIGHT]);
  mod->p_params.Fmin = chromatix_BPC->bcc_Fmin;
  mod->p_params.Fmax = chromatix_BPC->bcc_Fmax;

  util_bcc_cmd_config(mod);

  mod->enable = TRUE;
  mod->trigger_enable = TRUE;
  mod->aec_ratio = 0;
  mod->skip_trigger = FALSE;
  mod->hw_update_pending = TRUE;

  return rc;
} /* bcc_config */

/** bcc_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bcc_destroy(void *mod_ctrl)
{
  isp_bcc_mod_t *mod = mod_ctrl;

  memset(mod, 0, sizeof(isp_bcc_mod_t));
  free(mod);
  return 0;
} /* bcc_destroy */

/** bcc_enable:
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
static int bcc_enable(isp_bcc_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->enable = enable->enable;
  if (!mod->enable)
    mod->hw_update_pending = 0;
  return 0;
} /* bcc_enable */

/** bcc_trigger_enable:
 *    @mod: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bcc_trigger_enable(isp_bcc_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->trigger_enable = enable->enable;
  return 0;
} /* bcc_trigger_enable */

/** bcc_trigger_update:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Update configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bcc_trigger_update(isp_bcc_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  chromatix_parms_type *chromatix_ptr =
    in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_BPC_type *chromatix_BPC =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  chromatix_CS_MCE_type *chromatix_CS_MCE =
      &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;
  float aec_ratio;
  uint8_t update_bcc = TRUE;
  uint8_t Fmin, Fmin_lowlight, Fmax, Fmax_lowlight;
  tuning_control_type tunning_control;
  bpc_4_offset_type *bcc_normal_input_offset = NULL;
  bpc_4_offset_type *bcc_lowlight_input_offset = NULL;
  trigger_point_type *trigger_point = NULL;
  int is_snapmode, update_cs;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  if (!mod->enable || !mod->trigger_enable || mod->skip_trigger) {
    ISP_DBG(ISP_MOD_BCC, "%s: Skip Trigger update. enable %d, trig_enable %d, skip_trigger %d",
      __func__, mod->enable, mod->trigger_enable, mod->skip_trigger);
    return rc;
  }

  is_snapmode = IS_BURST_STREAMING(&in_params->cfg);
  if (!is_snapmode
    && !isp_util_aec_check_settled(
      &(in_params->trigger_input.stats_update.aec_update))) {
    ISP_DBG(ISP_MOD_BCC, "%s: AEC not settled", __func__);
    return rc;
  }

  trigger_point = &(chromatix_BPC->bcc_lowlight_trigger);
  Fmin = chromatix_BPC->bcc_Fmin;
  Fmax = chromatix_BPC->bcc_Fmax;
  Fmin_lowlight = chromatix_BPC->bcc_Fmin_lowlight;
  Fmax_lowlight = chromatix_BPC->bcc_Fmax_lowlight;
  bcc_normal_input_offset =
    &(chromatix_BPC->bcc_4_offset[BPC_NORMAL_LIGHT]);
  bcc_lowlight_input_offset =
    &(chromatix_BPC->bcc_4_offset[BPC_LOW_LIGHT]);

  tunning_control = chromatix_BPC->control_bcc;
  aec_ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
                                     chromatix_CS_MCE->control_cs,
                                     trigger_point,
                                     &(in_params->trigger_input.
                                       stats_update.aec_update),
                                     is_snapmode);

  update_cs = ((mod->old_streaming_mode != in_params->cfg.streaming_mode)
    || !F_EQUAL(mod->aec_ratio, aec_ratio));

  if (!update_cs) {
    ISP_DBG(ISP_MOD_BCC, "%s: update not required", __func__);
    return 0;
  }

  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG(ISP_MOD_BCC, "%s: Low Light \n", __func__);
    mod->p_params.p_input_offset = bcc_lowlight_input_offset;
    mod->p_params.Fmin = Fmin_lowlight;
    mod->p_params.Fmax = Fmax_lowlight;
    util_bcc_cmd_config(mod);
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG(ISP_MOD_BCC, "%s: Normal Light \n", __func__);
    mod->p_params.p_input_offset = bcc_normal_input_offset;
    mod->p_params.Fmin = Fmin;
    mod->p_params.Fmax = Fmax;
    util_bcc_cmd_config(mod);
  } else {
    ISP_DBG(ISP_MOD_BCC, "%s: Interpolate between Nomal and Low Light \n", __func__);
    /* Directly configure reg cmd.*/
    Fmin = (uint8_t)Round(LINEAR_INTERPOLATION(Fmin, Fmin_lowlight, aec_ratio));
    mod->RegCmd.fminThreshold = MIN(63, Fmin);

    Fmax = (uint8_t)Round(LINEAR_INTERPOLATION(Fmax, Fmax_lowlight, aec_ratio));
    mod->RegCmd.fmaxThreshold = MIN(127, Fmax);

    mod->RegCmd.rOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_r_hi,
      bcc_lowlight_input_offset->bpc_4_offset_r_hi, aec_ratio));
    mod->RegCmd.rOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_r_lo,
      bcc_lowlight_input_offset->bpc_4_offset_r_lo, aec_ratio));

    mod->RegCmd.bOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_b_hi,
      bcc_lowlight_input_offset->bpc_4_offset_b_hi, aec_ratio));
    mod->RegCmd.bOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_b_lo,
      bcc_lowlight_input_offset->bpc_4_offset_b_lo, aec_ratio));

    mod->RegCmd.grOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_gr_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gr_hi, aec_ratio));
    mod->RegCmd.grOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_gr_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gr_lo, aec_ratio));

    mod->RegCmd.gbOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_gb_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gb_hi, aec_ratio));
    mod->RegCmd.gbOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bcc_normal_input_offset->bpc_4_offset_gb_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gb_lo, aec_ratio));
  }
  mod->aec_ratio = aec_ratio;

  util_bcc_cmd_debug(&(mod->RegCmd));
  mod->hw_update_pending = TRUE;
  return rc;
} /* bcc_trigger_update */

/** bcc_set_params:
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
static int bcc_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_bcc_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = bcc_enable(mod, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = bcc_config(mod, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = bcc_trigger_enable(mod, (isp_mod_set_enable_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = bcc_trigger_update(mod, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
    break;
  default:
    return -EAGAIN;
    break;
  }
  return rc;
} /* bcc_set_params */

/** bcc_ez_isp_update
 *  @bcc_module
 *  @bcc
 *
 **/
static void bcc_ez_isp_update(
  isp_bcc_mod_t *bcc_module,
  badcorrection_t *bcc)
{
  ISP_DemosaicDBCC_CmdType *bccCfg;
  bccCfg = &(bcc_module->applied_RegCmd);

  bcc->fminThreshold = bccCfg->fminThreshold;
  bcc->fmaxThreshold = bccCfg->fmaxThreshold;
  bcc->gbOffsetLo = bccCfg->gbOffsetLo;
  bcc->gbOffsetHi = bccCfg->gbOffsetHi;
  bcc->grOffsetLo = bccCfg->grOffsetLo;
  bcc->grOffsetHi = bccCfg->grOffsetHi;
  bcc->rOffsetLo = bccCfg->rOffsetLo;
  bcc->rOffsetHi = bccCfg->rOffsetHi;
  bcc->bOffsetLo = bccCfg->bOffsetLo;
  bcc->bOffsetHi = bccCfg->bOffsetHi;
}

/** bcc_get_params:
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
static int bcc_get_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_bcc_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    enable->enable = mod->enable;
    break;
  }

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    badcorrection_t *bcc;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    bcc = &(vfe_diag->prev_bcc);
    if (mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
      bcc = &(vfe_diag->snap_bcc);
    }
    vfe_diag->control_bcc.enable = mod->enable;
    vfe_diag->control_bcc.cntrlenable = mod->trigger_enable;
    bcc_ez_isp_update(mod, bcc);
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_BCC, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default:
    rc = -1;
    break;
  }
  return rc;
} /* bcc_get_params */

/** bcc_do_hw_update:
 *    @bcc_mod: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bcc_do_hw_update(isp_bcc_mod_t *bcc_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[2];
  ISP_DemosaicDBCCCfg_CmdType cfg_mask;

  if (bcc_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)&bcc_mod->RegCmd;
    cfg_cmd.cmd_len = sizeof(bcc_mod->RegCmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 2;

    cfg_mask.enable = 1;
    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_DBCC32_DEMOSAIC_MIX_CFG_OFF;
    reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.cfg;
    reg_cfg_cmd[0].u.mask_info.val = bcc_mod->RegCfgCmd.cfg;

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DBCC32_OFF;
    reg_cfg_cmd[1].u.rw_info.len = ISP_DBCC32_LEN * sizeof(uint32_t);

    rc = ioctl(bcc_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    bcc_mod->applied_RegCmd = bcc_mod->RegCmd;
    bcc_mod->hw_update_pending = 0;
  }

  /* TODO: update hw reg */
  return rc;
} /* bcc_do_hw_update */

/** bcc_action:
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
static int bcc_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_bcc_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = bcc_do_hw_update(mod);
    break;
  default:
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
    break;
  }
  return rc;
} /* bcc_action */

/** bcc_init:
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
static int bcc_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_bcc_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
} /* bcc_init */

/** bcc32_open:
 *    @version: version of isp
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *bcc32_open(uint32_t version)
{
  isp_bcc_mod_t *mod = malloc(sizeof(isp_bcc_mod_t));

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory", __func__);
    return NULL;
  }
  memset(mod, 0, sizeof(isp_bcc_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = bcc_init;
  mod->ops.destroy = bcc_destroy;
  mod->ops.set_params = bcc_set_params;
  mod->ops.get_params = bcc_get_params;
  mod->ops.action = bcc_action;

  return &mod->ops;
} /* bcc32_open */

