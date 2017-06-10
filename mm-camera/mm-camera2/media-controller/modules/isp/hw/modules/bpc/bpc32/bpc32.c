/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "bpc32.h"
#include "isp_log.h"

#ifdef ENABLE_BPC_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#ifndef sign
#define sign(x) (((x) < 0) ? (-1) : (1))
#endif

#ifndef Round
#define Round(x) (int)((x) + sign(x)*0.5)
#endif

/** util_bpc_cmd_debug:
 *    @cmd: Pointer to statistic configuration.
 *
 * Print mce configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void util_bpc_cmd_debug(ISP_DemosaicDBPC_CmdType *cmd)
{
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->fminThreshold = %d \n", __func__, cmd->fminThreshold);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->fmaxThreshold = %d \n", __func__, cmd->fmaxThreshold);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->rOffsetHi = %d \n", __func__, cmd->rOffsetHi);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->rOffsetLo = %d \n", __func__, cmd->rOffsetLo);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->bOffsetHi = %d \n", __func__, cmd->bOffsetHi);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->bOffsetLo = %d \n", __func__, cmd->bOffsetLo);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->grOffsetHi = %d \n", __func__, cmd->grOffsetHi);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->grOffsetLo = %d \n", __func__, cmd->grOffsetLo);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->gbOffsetHi = %d \n", __func__, cmd->gbOffsetHi);
  ISP_DBG(ISP_MOD_BPC, "%s: cmd->gbOffsetLo = %d \n", __func__, cmd->gbOffsetLo);
} /* util_bpc_cmd_debug */

/** util_bpc_cmd_config:
 *    @mod: pointer to instance private data
 *
 * Update hardware configuration structures.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void util_bpc_cmd_config(isp_bpc_mod_t *mod)
{
  mod->RegCfgCmd.enable = mod->enable;
  mod->RegCmd.fminThreshold = MIN(63, *(mod->p_params.p_Fmin));
  mod->RegCmd.fmaxThreshold = MIN(127, *(mod->p_params.p_Fmax));
  mod->RegCmd.rOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_r_hi;
  mod->RegCmd.rOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_r_lo;
  mod->RegCmd.bOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_b_hi;
  mod->RegCmd.bOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_b_lo;
  mod->RegCmd.grOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_gr_lo;
  mod->RegCmd.grOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_gr_hi;
  mod->RegCmd.gbOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_gb_lo;
  mod->RegCmd.gbOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_gb_hi;
} /* util_bpc_cmd_config */

/** bpc_do_hw_update:
 *    @bpc_mod: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bpc_do_hw_update(isp_bpc_mod_t *bpc_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[3];
  ISP_DemosaicDBPCCfg_CmdType cfg_mask;

  if (bpc_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)&bpc_mod->RegCmd;
    cfg_cmd.cmd_len = sizeof(bpc_mod->RegCmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 3;

    cfg_mask.enable = 1;
    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_DBPC32_DEMOSAIC_MIX_CFG_OFF;
    reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.cfg_reg;
    reg_cfg_cmd[0].u.mask_info.val = bpc_mod->RegCfgCmd.cfg_reg;

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DBPC32_CFG_OFF;
    reg_cfg_cmd[1].u.rw_info.len = ISP_DBPC32_LEN * sizeof(uint32_t);

    reg_cfg_cmd[2].u.rw_info.cmd_data_offset = 8;
    reg_cfg_cmd[2].cmd_type = VFE_WRITE;
    reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_DBPC32_CFG_OFF_1;
    reg_cfg_cmd[2].u.rw_info.len = ISP_DBPC32_LEN_1 * sizeof(uint32_t);

    util_bpc_cmd_debug(&(bpc_mod->RegCmd));

    rc = ioctl(bpc_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    memcpy(&(bpc_mod->applied_RegCmd), &(bpc_mod->RegCmd),
      sizeof(ISP_DemosaicDBPC_CmdType));
    bpc_mod->hw_update_pending = 0;
  }

  /* TODO: update hw reg */
  return rc;
} /* bpc_do_hw_update */

/** bpc_init:
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
static int bpc_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_bpc_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
} /* bpc_init */

/** bpc_config:
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
static int bpc_config(isp_bpc_mod_t *mod,
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

  ISP_DBG(ISP_MOD_BPC, "%s: enter", __func__);

  if (!mod->enable) {
    ISP_DBG(ISP_MOD_BPC, "%s: Mod not Enable.", __func__);
    return rc;
  }

  /* set old cfg to invalid value to trigger the first trigger update */
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  mod->p_params.p_input_offset =
    &(chromatix_BPC->bpc_4_offset[BPC_NORMAL_LIGHT]);
  mod->p_params.p_Fmin = &(chromatix_BPC->bpc_Fmin);
  mod->p_params.p_Fmax = &(chromatix_BPC->bpc_Fmax);

  util_bpc_cmd_config(mod);

  mod->enable = TRUE;
  mod->trigger_enable = TRUE;
  mod->aec_ratio = 0;
  mod->skip_trigger = FALSE;
  mod->hw_update_pending = TRUE;

  return rc;
} /* bpc_config */

/** bpc_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bpc_destroy(void *mod_ctrl)
{
  isp_bpc_mod_t *mod = mod_ctrl;

  memset(mod, 0, sizeof(isp_bpc_mod_t));
  free(mod);
  return 0;
} /* bpc_destroy */

/** bpc_enable:
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
static int bpc_enable(isp_bpc_mod_t *mod, isp_mod_set_enable_t *enable,
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
} /* bpc_enable */

/** bpc_trigger_enable:
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
static int bpc_trigger_enable(isp_bpc_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->trigger_enable = enable->enable;
  return 0;
} /* bpc_trigger_enable */

/** bpc_trigger_update:
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
static int bpc_trigger_update(isp_bpc_mod_t *mod,
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
  uint8_t update_bpc = TRUE;
  uint8_t Fmin, Fmin_lowlight, Fmax, Fmax_lowlight;
  tuning_control_type tunning_control;
  bpc_4_offset_type *bpc_normal_input_offset = NULL;
  bpc_4_offset_type *bpc_lowlight_input_offset = NULL;
  trigger_point_type *trigger_point = NULL;
  int is_snapmode, update_cs;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  if (!mod->enable || !mod->trigger_enable || mod->skip_trigger) {
    ISP_DBG(ISP_MOD_BPC, "%s: Skip Trigger update. enable %d, trig_enable %d, skip_trigger %d",
      __func__, mod->enable, mod->trigger_enable, mod->skip_trigger);
    return rc;
  }

  is_snapmode = IS_BURST_STREAMING(&in_params->cfg);
  if (!is_snapmode
    && !isp_util_aec_check_settled(
      &(in_params->trigger_input.stats_update.aec_update))) {
    ISP_DBG(ISP_MOD_BPC, "%s: AEC not settled", __func__);
    return rc;
  }

  ISP_DBG(ISP_MOD_BPC, "%s: Calculate table with AEC", __func__);

  trigger_point = &(chromatix_BPC->bpc_lowlight_trigger);
  Fmin = chromatix_BPC->bpc_Fmin;
  Fmax = chromatix_BPC->bpc_Fmax;
  Fmin_lowlight = chromatix_BPC->bpc_Fmin_lowlight;
  Fmax_lowlight = chromatix_BPC->bpc_Fmax_lowlight;
  bpc_normal_input_offset = &(chromatix_BPC->bpc_4_offset[BPC_NORMAL_LIGHT]);
  bpc_lowlight_input_offset = &(chromatix_BPC->bpc_4_offset[BPC_LOW_LIGHT]);

  tunning_control = chromatix_BPC->control_bpc;
  aec_ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
    chromatix_BPC->control_bpc, trigger_point,
    &(in_params->trigger_input.stats_update.aec_update), is_snapmode);

  update_cs = ((mod->old_streaming_mode != in_params->cfg.streaming_mode)
    || !F_EQUAL(mod->aec_ratio, aec_ratio));

  if (!update_cs) {
    ISP_DBG(ISP_MOD_BPC, "%s: update not required", __func__);
    return 0;
  }

  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG(ISP_MOD_BPC, "%s: Low Light \n", __func__);
    mod->p_params.p_input_offset = bpc_lowlight_input_offset;
    mod->p_params.p_Fmin = &(Fmin_lowlight);
    mod->p_params.p_Fmax = &(Fmax_lowlight);
    util_bpc_cmd_config(mod);
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG(ISP_MOD_BPC, "%s: Normal Light \n", __func__);
    mod->p_params.p_input_offset = bpc_normal_input_offset;
    mod->p_params.p_Fmin = &(Fmin);
    mod->p_params.p_Fmax = &(Fmax);
    util_bpc_cmd_config(mod);
  } else {
    ISP_DBG(ISP_MOD_BPC, "%s: Interpolate between Nomal and Low Light \n", __func__);
    /* Directly configure reg cmd.*/
    Fmin = (uint8_t)Round(LINEAR_INTERPOLATION(Fmin, Fmin_lowlight, aec_ratio));
    mod->RegCmd.fminThreshold = MIN(63, Fmin);

    Fmax = (uint8_t)Round(LINEAR_INTERPOLATION(Fmax, Fmax_lowlight, aec_ratio));
    mod->RegCmd.fmaxThreshold = MIN(127, Fmax);

    mod->RegCmd.rOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_r_hi,
      bpc_lowlight_input_offset->bpc_4_offset_r_hi, aec_ratio));
    mod->RegCmd.rOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_r_lo,
      bpc_lowlight_input_offset->bpc_4_offset_r_lo, aec_ratio));

    mod->RegCmd.bOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_b_hi,
      bpc_lowlight_input_offset->bpc_4_offset_b_hi, aec_ratio));
    mod->RegCmd.bOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_b_lo,
      bpc_lowlight_input_offset->bpc_4_offset_b_lo, aec_ratio));

    mod->RegCmd.grOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_gr_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gr_hi, aec_ratio));
    mod->RegCmd.grOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_gr_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gr_lo, aec_ratio));

    mod->RegCmd.gbOffsetHi = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_gb_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gb_hi, aec_ratio));
    mod->RegCmd.gbOffsetLo = (uint16_t)Round(LINEAR_INTERPOLATION(
        bpc_normal_input_offset->bpc_4_offset_gb_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gb_lo, aec_ratio));
  }
  mod->aec_ratio = aec_ratio;

  util_bpc_cmd_debug(&(mod->RegCmd));
  mod->hw_update_pending = TRUE;
  return rc;
} /* bpc_trigger_update */

/** bpc_set_params:
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
static int bpc_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_bpc_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = bpc_enable(mod, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = bpc_config(mod, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = bpc_trigger_enable(mod, (isp_mod_set_enable_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = bpc_trigger_update(mod, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
    break;
  default:
    return -EAGAIN;
    break;
  }
  return rc;
} /* bpc_set_params */

/** bpc_ez_isp_update
 *  @bpc_module: pointer to the bpc module
 *  @bpcDiag: pointer to the bpc diagonstics
 *
 **/
static void bpc_ez_isp_update(
  isp_bpc_mod_t *bpc_module,
  badcorrection_t *bpcDiag)
{
  ISP_DemosaicDBPC_CmdType *bpcCfg = &(bpc_module->applied_RegCmd);

  bpcDiag->fminThreshold = bpcCfg->fminThreshold;
  bpcDiag->fmaxThreshold = bpcCfg->fmaxThreshold;
  bpcDiag->gbOffsetLo = bpcCfg->gbOffsetLo;
  bpcDiag->gbOffsetHi = bpcCfg->gbOffsetHi;
  bpcDiag->grOffsetLo = bpcCfg->grOffsetLo;
  bpcDiag->grOffsetHi = bpcCfg->grOffsetHi;
  bpcDiag->rOffsetLo = bpcCfg->rOffsetLo;
  bpcDiag->rOffsetHi = bpcCfg->rOffsetHi;
  bpcDiag->bOffsetLo = bpcCfg->bOffsetLo;
  bpcDiag->bOffsetHi = bpcCfg->bOffsetHi;
}

/** bpc_get_params:
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
static int bpc_get_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_bpc_mod_t *mod = mod_ctrl;
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
    badcorrection_t *bpcDiag;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    bpcDiag = &(vfe_diag->prev_bpc);
    if (mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
      bpcDiag = &(vfe_diag->snap_bpc);
    }
    vfe_diag->control_bpc.enable = mod->enable;
    vfe_diag->control_bpc.cntrlenable = mod->trigger_enable;
    bpc_ez_isp_update(mod, bpcDiag);
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_BPC, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default:
    rc = -1;
    break;
  }
  return rc;
} /* bpc_get_params */

/** bpc_action:
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
static int bpc_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_bpc_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = bpc_do_hw_update(mod);
    break;

  default:
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
    break;
  }
  return rc;
} /* bpc_action */

/** bpc32_open:
 *    @version: version of isp
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *bpc32_open(uint32_t version)
{
  isp_bpc_mod_t *mod;

  mod = malloc(sizeof(isp_bpc_mod_t));
  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory", __func__);
    return NULL;
  }
  memset(mod, 0, sizeof(isp_bpc_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = bpc_init;
  mod->ops.destroy = bpc_destroy;
  mod->ops.set_params = bpc_set_params;
  mod->ops.get_params = bpc_get_params;
  mod->ops.action = bpc_action;

  return &mod->ops;
} /* bpc32_open */
