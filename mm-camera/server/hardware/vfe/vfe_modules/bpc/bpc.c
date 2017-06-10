/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "bpc.h"

#ifdef ENABLE_BPC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * Function:           bpc_debug
 *
 * Description:
===========================================================================*/
void bpc_debug(bpc_mod_t *mod, int is_snap)
{
  vfe_status_t status = VFE_SUCCESS;
  VFE_DemosaicBPC_CmdType *p_cmd = (is_snap) ? &mod->bpc_snap_cmd :
    &mod->bpc_prev_cmd;

  CDBG("BPC fminThreshold %d", p_cmd->fminThreshold);
  CDBG("BPC fmaxThreshold %d", p_cmd->fmaxThreshold);
  CDBG("BPC rOffsetHi %d", p_cmd->rOffsetHi);
  CDBG("BPC rOffsetLo %d", p_cmd->rOffsetLo);
  CDBG("BPC bOffsetHi %d", p_cmd->bOffsetHi);
  CDBG("BPC bOffsetLo %d", p_cmd->bOffsetLo);
  CDBG("BPC gOffsetHi %d", p_cmd->gOffsetHi);
  CDBG("BPC gOffsetLo %d", p_cmd->gOffsetLo);
}/*bpc_debug*/

/*===========================================================================
 * Function:           bpc_set_params
 *
 * Description:
===========================================================================*/
vfe_status_t bpc_set_params(bpc_mod_t *mod, vfe_params_t *params,
  int is_snap)
{
  vfe_status_t status = VFE_SUCCESS;
  VFE_DemosaicBPC_CmdType *p_cmd = (is_snap) ? &mod->bpc_snap_cmd :
    &mod->bpc_prev_cmd;
  chromatix_parms_type *chroma3a = params->chroma3a;
  bpc_diff_threshold_type *bpc_diff_threshold = NULL;

  if (!is_snap) {
    p_cmd->fminThreshold = MIN(63, chroma3a->bpc_Fmin_preview);
    p_cmd->fmaxThreshold = MIN(127, chroma3a->bpc_Fmax_preview);
    bpc_diff_threshold = chroma3a->bpc_diff_threshold;
  } else {
    p_cmd->fminThreshold = MIN(63, chroma3a->bpc_Fmin_snapshot);
    p_cmd->fmaxThreshold = MIN(127, chroma3a->bpc_Fmax_snapshot);
    bpc_diff_threshold = chroma3a->bpc_diff_threshold_snapshot;
  }

  p_cmd->rOffsetHi = p_cmd->rOffsetLo =
    bpc_diff_threshold[BPC_NORMAL_LIGHT].bpc_diff_threshold_r;

  p_cmd->bOffsetHi = p_cmd->bOffsetLo =
    bpc_diff_threshold[BPC_NORMAL_LIGHT].bpc_diff_threshold_b;

  p_cmd->gOffsetHi = p_cmd->gOffsetLo =
    bpc_diff_threshold[BPC_NORMAL_LIGHT].bpc_diff_threshold_g;

  return status;
}/*bpc_set_params*/

/*===========================================================================
 * Function:           vfe_demosaic_bpc_update
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_demosaic_bpc_update(bpc_mod_t *bpc_ctrl,
vfe_params_t *vfe_params)
{
  vfe_status_t status = VFE_SUCCESS;
  VFE_DemosaicBPC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  CDBG("%s: %d %d", __func__, bpc_ctrl->hw_enable_cmd,
    bpc_ctrl->bpc_trigger_update);
  if (bpc_ctrl->hw_enable_cmd) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *) cmd, sizeof(VFE_DemosaicBPC_CmdType),
      VFE_CMD_DEMOSAICV3_DBPC_UPDATE)) {
      CDBG_ERROR("%s: DBPC update for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
    vfe_params->update |= VFE_MOD_BPC;
    bpc_ctrl->hw_enable_cmd = FALSE;
  }

  if (!bpc_ctrl->bpc_enable) {
    CDBG("%s: bpc is not enabled. Skip the update\n", __func__);
    return VFE_SUCCESS;
  }

  if (!bpc_ctrl->bpc_trigger_update) {
    CDBG("%s: Trigger is not be valid. Skip the update", __func__);
    return VFE_SUCCESS;
  }

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(VFE_DemosaicBPC_CmdType),
    VFE_CMD_DEMOSAICV3_DBPC_UPDATE)) {
    CDBG_ERROR("%s: DBPC update for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  bpc_ctrl->bpc_trigger_update = FALSE;
  vfe_params->update |= VFE_MOD_BPC;

  return status;
}/*vfe_demosaic_bpc_update*/

/*===========================================================================
 * Function:           vfe_demosaic_bpc_trigger_update
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_demosaic_bpc_trigger_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params)
{
  vfe_status_t status = VFE_SUCCESS;
  uint8_t Fmin, Fmin_lowlight, Fmax, Fmax_lowlight;
  tuning_control_type tunning_control;
  float new_trigger_ratio;
  VFE_DemosaicBPC_CmdType *cmd = NULL;
  bpc_diff_threshold_type *diff_threshold;
  trigger_point_type *trigger_point = NULL;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;
  int is_snap = IS_SNAP_MODE(vfe_params);

  if (!bpc_ctrl->bpc_enable) {
    CDBG("%s: bpc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  tunning_control = chrPtr->control_bpc;

  if (!bpc_ctrl->bpc_trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (!is_snap && !vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
    if (!bpc_ctrl->bpc_reload_params) {
      CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
      return VFE_SUCCESS;
    }
  }

  if (is_snap) {
    trigger_point = &(chrPtr->bpc_snapshot_lowlight_trigger);
    Fmin = chrPtr->bpc_Fmin_snapshot;
    Fmax = chrPtr->bpc_Fmax_snapshot;
    Fmin_lowlight = chrPtr->bpc_Fmin_snapshot_lowlight;
    Fmax_lowlight = chrPtr->bpc_Fmax_snapshot_lowlight;
    diff_threshold = chrPtr->bpc_diff_threshold;
    cmd = &(bpc_ctrl->bpc_snap_cmd);
  } else {
    trigger_point = &(chrPtr->bpc_lowlight_trigger);
    Fmin = chrPtr->bpc_Fmin_preview;
    Fmax = chrPtr->bpc_Fmax_preview;
    Fmin_lowlight = chrPtr->bpc_Fmin_preview_lowlight;
    Fmax_lowlight = chrPtr->bpc_Fmax_preview_lowlight;
    diff_threshold = chrPtr->bpc_diff_threshold_snapshot;
    cmd = &(bpc_ctrl->bpc_prev_cmd);
  }

  new_trigger_ratio = vfe_util_get_aec_ratio(tunning_control, trigger_point,
    vfe_params);

  if (bpc_ctrl->cur_mode != vfe_params->vfe_op_mode ||
     bpc_ctrl->bpc_reload_params ||
    !F_EQUAL(new_trigger_ratio, bpc_ctrl->bpc_trigger_ratio)) {

    cmd->fminThreshold = (uint8_t)LINEAR_INTERPOLATION(Fmin, Fmin_lowlight,
      new_trigger_ratio);
    cmd->fmaxThreshold = (uint8_t)LINEAR_INTERPOLATION(Fmax, Fmax_lowlight,
      new_trigger_ratio);

    cmd->rOffsetHi = cmd->rOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      diff_threshold[BPC_NORMAL_LIGHT].bpc_diff_threshold_r,
      diff_threshold[BPC_LOW_LIGHT].bpc_diff_threshold_r, new_trigger_ratio);

    cmd->bOffsetHi = cmd->bOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      diff_threshold[BPC_NORMAL_LIGHT].bpc_diff_threshold_b,
      diff_threshold[BPC_LOW_LIGHT].bpc_diff_threshold_b, new_trigger_ratio);

    cmd->gOffsetHi = cmd->gOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      diff_threshold[BPC_NORMAL_LIGHT].bpc_diff_threshold_g,
      diff_threshold[BPC_LOW_LIGHT].bpc_diff_threshold_g, new_trigger_ratio);

    bpc_ctrl->cur_mode = vfe_params->vfe_op_mode;
    bpc_ctrl->bpc_trigger_ratio = new_trigger_ratio;
    bpc_ctrl->bpc_trigger_update = TRUE;
    bpc_ctrl->bpc_reload_params = FALSE;
  } else {
    CDBG("%s: No update required.\n", __func__);
  }
  return status;
}/*vfe_demosaic_bpc_trigger_update*/

/*===========================================================================
 * Function:           vfe_demosaic_bpc_config
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_demosaic_bpc_config(bpc_mod_t *mod, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int is_snap = IS_SNAP_MODE(params);
  if (!mod->bpc_enable) {
    CDBG("%s: bpc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  CDBG("BPC cfg %s", (is_snap) ? "Snapshot" : "Preview");
  VFE_DemosaicBPC_CmdType *cmd = (is_snap) ?
    &(mod->bpc_snap_cmd) : &(mod->bpc_prev_cmd);

  bpc_debug(mod, is_snap);
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(VFE_DemosaicBPC_CmdType),
    VFE_CMD_DEMOSAICV3_DBPC_CFG)) {
    CDBG_ERROR("%s: DBPC config for operation mode = %d failed\n", __func__,
      params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  mod->bpc_trigger_update = FALSE;
  return status;
}/*vfe_demosaic_bpc_config*/

/*===========================================================================
 * Function:           vfe_demosaic_bpc_enable
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_demosaic_bpc_enable(bpc_mod_t *mod, vfe_params_t *params,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;

  VFE_DemosaicBPC_CmdType *cmd = IS_SNAP_MODE(params) ?
    &(mod->bpc_snap_cmd) : &(mod->bpc_prev_cmd);

  CDBG("%s: enable=%d, hw_write=%d\n", __func__, enable, hw_write);

  if (hw_write && (mod->bpc_enable == enable))
    return VFE_SUCCESS;

  mod->bpc_enable = enable;
  cmd->enable = enable;

  mod->hw_enable_cmd = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_BPC)
      : (params->current_config & ~VFE_MOD_BPC);
  }

  return status;
}/*vfe_demosaic_bpc_enable*/

/*===========================================================================
 * Function:           vfe_demosaic_bpc_init
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_demosaic_bpc_init(bpc_mod_t *mod, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  bpc_4_offset_type *bpc_input_offset = NULL;
  chromatix_parms_type *chroma3a = params->chroma3a;
  memset(mod, 0x0, sizeof(bpc_mod_t));

  if (!IS_BAYER_FORMAT(params))
    return VFE_SUCCESS;

  mod->bpc_trigger_enable = TRUE;
  bpc_set_params(mod, params, TRUE);
  bpc_set_params(mod, params, FALSE);
  return status;
}/*vfe_demosaic_bpc_init*/

/*===========================================================================
 * Function:           vfe_bpc_trigger_enable
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_bpc_trigger_enable(bpc_mod_t* bpc_ctrl,
  vfe_params_t* params, int8_t enable)
{
  vfe_status_t status = VFE_SUCCESS;
  bpc_ctrl->bpc_trigger_enable = enable;
  return status;
}/*vfe_bpc_trigger_enable*/

/*===========================================================================
 * Function:           vfe_bpc_reload_params
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_bpc_reload_params(bpc_mod_t* bpc_ctrl,
  vfe_params_t* params)
{
  vfe_status_t status = VFE_SUCCESS;
  bpc_ctrl->bpc_reload_params = TRUE;
  return status;
}/*vfe_bpc_reload_params*/
