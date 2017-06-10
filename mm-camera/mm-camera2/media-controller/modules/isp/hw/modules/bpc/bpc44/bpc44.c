/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "bpc44.h"
#include "isp_log.h"

#ifdef ENABLE_BPC_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

/** util_bpc_cmd_debug
 *    @cmd: bpc config cmd
 *
 * This function dumps the bpc module configuration set to hw
 *
 * Return: nothing
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

/** util_bpc_cmd_config
 *    @mod: bpc module struct data
 *
 * Copy from mod->chromatix params to reg cmd then configure
 *
 * Return: nothing
 **/
static void util_bpc_cmd_config(isp_bpc_mod_t *mod)
{
  mod->RegCfgCmd.enable = mod->enable;
  mod->RegCmd.fminThreshold = *mod->p_params.p_Fmin;
  mod->RegCmd.fmaxThreshold = *mod->p_params.p_Fmax;
  mod->RegCmd.rOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_r_hi;
  mod->RegCmd.rOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_r_lo;
  mod->RegCmd.bOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_b_hi;
  mod->RegCmd.bOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_b_lo;
  mod->RegCmd.grOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_gr_lo;
  mod->RegCmd.grOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_gr_hi;
  mod->RegCmd.gbOffsetLo = mod->p_params.p_input_offset->bpc_4_offset_gb_lo;
  mod->RegCmd.gbOffsetHi = mod->p_params.p_input_offset->bpc_4_offset_gb_hi;
} /* util_bpc_cmd_config */

/** bpc_reset
 *    @mod: bpc module struct data
 *
 * BPC module disable,release reg settings and strcuts
 *
 * Return: nothing
 **/
static void bpc_reset(isp_bpc_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->RegCfgCmd, 0, sizeof(mod->RegCfgCmd));
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  mod->aec_ratio = 0;
  memset(&mod->p_params, 0, sizeof(mod->p_params));
  mod->hw_update_pending = 0;
  mod->trigger_enable = 0; /* enable trigger update feature flag from PIX */
  mod->skip_trigger = 0;
  mod->enable = 0;         /* enable flag from PIX */
}

/** bpc_do_hw_update
 *    @bpc_mod: bpc module struct data
 *
 * update BPC module register to kernel
 *
 * Return: nothing
 **/
static int bpc_do_hw_update(isp_bpc_mod_t *bpc_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[2];
  ISP_DemosaicDBPCCfg_CmdType cfg_mask;

  if (bpc_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &bpc_mod->RegCmd;
    cfg_cmd.cmd_len = sizeof(bpc_mod->RegCmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 2;

    cfg_mask.enable = 1;

    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_DBPC_DEMOSAIC_MIX_CFG_OFF;
    reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.cfg_reg;
    reg_cfg_cmd[0].u.mask_info.val = bpc_mod->RegCfgCmd.cfg_reg;

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DBPC40_CFG_OFF;
    reg_cfg_cmd[1].u.rw_info.len = ISP_DBPC40_LEN * sizeof(uint32_t);

    rc = ioctl(bpc_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    bpc_mod->hw_update_pending = 0;
  }

  /* TODO: update hw reg */
  return rc;
} /* bpc_do_hw_update */

/** bpc_init
 *    @mod_ctrl: bpc module control strcut
 *    @in_params: BPC hw module init params
 *    @notify_ops: fn pointer to notify other modules
 *
 *  BPC module data struct initialization
 *
 * Return: 0 always
 **/
static int bpc_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_bpc_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  bpc_reset(mod);
  return 0;
} /* bpc_init */

/** bpc_config
 *    @mod: bpc module control strcut
 *    @in_params: contains chromatix ptr
 *    @in_param_size: in params struct size
 *
 *  BPC module configuration initial settings
 *
 * Return: 0 - success and negative value - failure
 **/
static int bpc_config(isp_bpc_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int  rc = 0;
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

  if ((chromatix_BPC->bpc_Fmin > chromatix_BPC->bpc_Fmax) ||
      (chromatix_BPC->bpc_Fmin_lowlight > chromatix_BPC->bpc_Fmax_lowlight)) {
    CDBG_ERROR("%s: Error min>max: %d/%d; %d/%d\n", __func__,
      chromatix_BPC->bpc_Fmin, chromatix_BPC->bpc_Fmax,
      chromatix_BPC->bpc_Fmin_lowlight, chromatix_BPC->bpc_Fmax_lowlight);
    return -1;
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

/** bpc_destroy
 *    @mod_ctrl: bpc module control strcut
 *
 *  Close BPC module
 *
 * Return: 0 always
 **/
static int bpc_destroy (void *mod_ctrl)
{
  isp_bpc_mod_t *mod = mod_ctrl;

  memset(mod, 0, sizeof(isp_bpc_mod_t));
  free(mod);
  return 0;
} /* bpc_destroy */

/** bpc_enable
 *    @mod: bpc module control struct
 *    @enable: module enable/disable flag
 *
 *  BPC module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int bpc_enable(isp_bpc_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
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

/** bpc_trigger_enable
 *    @mod: bpc module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  BPC module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
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

/** bpc_trigger_update
 *    @mod: bpc module control struct
 *    @in_params: input config params including chromatix ptr
 *    @in_param_size: input params struct size
 *
 *  BPC module modify reg settings as per new input params and
 *  trigger hw update
 *
 * Return: 0 - success and negative value - failure
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
  if (!is_snapmode && !isp_util_aec_check_settled(
         &(in_params->trigger_input.stats_update.aec_update))) {
    ISP_DBG(ISP_MOD_BPC, "%s: AEC not settled", __func__);
    return rc;
  }

  ISP_DBG(ISP_MOD_BPC, "%s: Calculate table with AEC", __func__);
  if ((chromatix_BPC->bpc_Fmin > chromatix_BPC->bpc_Fmax) ||
      (chromatix_BPC->bpc_Fmin_lowlight > chromatix_BPC->bpc_Fmax_lowlight)) {
    CDBG_ERROR("%s: Error min>max: %d/%d; %d/%d\n", __func__,
      chromatix_BPC->bpc_Fmin, chromatix_BPC->bpc_Fmax,
      chromatix_BPC->bpc_Fmin_lowlight, chromatix_BPC->bpc_Fmax_lowlight);
    return -1;
  }

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
                &(in_params->trigger_input.stats_update.aec_update),
                is_snapmode);
  update_cs = ((mod->old_streaming_mode != in_params->cfg.streaming_mode) ||
    !F_EQUAL(mod->aec_ratio, aec_ratio));

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
    Fmin = (uint8_t)LINEAR_INTERPOLATION(Fmin, Fmin_lowlight, aec_ratio);
    mod->RegCmd.fminThreshold = Fmin;

    Fmax = (uint8_t)LINEAR_INTERPOLATION(Fmax, Fmax_lowlight, aec_ratio);
    mod->RegCmd.fmaxThreshold = Fmax;

    mod->RegCmd.rOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_r_hi,
      bpc_lowlight_input_offset->bpc_4_offset_r_hi, aec_ratio);
    mod->RegCmd.rOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_r_lo,
      bpc_lowlight_input_offset->bpc_4_offset_r_lo, aec_ratio);

    mod->RegCmd.bOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_b_hi,
      bpc_lowlight_input_offset->bpc_4_offset_b_hi, aec_ratio);
    mod->RegCmd.bOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_b_lo,
      bpc_lowlight_input_offset->bpc_4_offset_b_lo, aec_ratio);

    mod->RegCmd.grOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gr_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gr_hi, aec_ratio);
    mod->RegCmd.grOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gr_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gr_lo, aec_ratio);

    mod->RegCmd.gbOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gb_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gb_hi, aec_ratio);
    mod->RegCmd.gbOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gb_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gb_lo, aec_ratio);
  }
  mod->aec_ratio = aec_ratio;

  util_bpc_cmd_debug(&(mod->RegCmd));
  mod->hw_update_pending = TRUE;
  return rc;
} /* bpc_trigger_update */

/** bpc_set_params
 *    @mod_ctrl: bpc module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *
 *  set config params utility to update BPC module
 *
 * Return: 0 - success and negative value - failure
 **/
static int bpc_set_params (void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_bpc_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = bpc_enable(mod, (isp_mod_set_enable_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = bpc_config(mod, (isp_hw_pix_setting_params_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = bpc_trigger_enable(mod, (isp_mod_set_enable_t *)in_params,
         in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = bpc_trigger_update(mod,(isp_pix_trigger_update_input_t *)in_params,
         in_param_size);
  }
    break;

  default: {
    rc = -EAGAIN;
  }
    break;
  }
  return rc;
} /* bpc_set_params */

/** bpc_get_params
 *    @mod_ctrl: bpc module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *    @out_params: struct to return out params
 *    @out_param_size: output params struct size
 *
 *  Get config params utility to fetch config of BPC module
 *
 * Return: 0 - success and negative value - failure
 **/
static int bpc_get_params (void *mod_ctrl, uint32_t param_id, void *in_params,
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
  }
    break;

  default:{
    rc = -1;
  }
    break;
  }
  return rc;
} /* bpc_get_params */

/** bpc_action
 *    @mod_ctrl: bpc module control struct
 *    @action_code : action code
 *    @data: not used
 *    @data_size: not used
 *
 *  processing the hw action like update or reset
 *
 * Return: 0 - success and negative value - failure
 **/
static int bpc_action (void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_bpc_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = bpc_do_hw_update(mod);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET: {
    bpc_reset(mod);
  }
    break;

  default: {
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
  }
    break;
  }
  return rc;
} /* bpc_action */

/** bpc44_open
 *    @version: hw version
 *
 *  BPC 40 module open and create func table
 *
 * Return: BPC module ops struct pointer
 **/
isp_ops_t *bpc44_open(uint32_t version)
{
  isp_bpc_mod_t *mod = malloc(sizeof(isp_bpc_mod_t));

  ISP_DBG(ISP_MOD_BPC, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }
  memset(mod,  0,  sizeof(isp_bpc_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = bpc_init;
  mod->ops.destroy = bpc_destroy;
  mod->ops.set_params = bpc_set_params;
  mod->ops.get_params = bpc_get_params;
  mod->ops.action = bpc_action;

  return &mod->ops;
} /* bpc44_open */
