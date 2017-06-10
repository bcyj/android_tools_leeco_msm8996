/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BCC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bcc_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_demosaic_bcc_debug(VFE_DemosaicDBCC_CmdType *cmd)
{
  CDBG("%s: cmd->fminThreshold = %d \n", __func__, cmd->fminThreshold);
  CDBG("%s: cmd->fmaxThreshold = %d \n", __func__, cmd->fmaxThreshold);
  CDBG("%s: cmd->rOffsetHi = %d \n", __func__, cmd->rOffsetHi);
  CDBG("%s: cmd->rOffsetLo = %d \n", __func__, cmd->rOffsetLo);
  CDBG("%s: cmd->bOffsetHi = %d \n", __func__, cmd->bOffsetHi);
  CDBG("%s: cmd->bOffsetLo = %d \n", __func__, cmd->bOffsetLo);
  CDBG("%s: cmd->grOffsetHi = %d \n", __func__, cmd->grOffsetHi);
  CDBG("%s: cmd->grOffsetLo = %d \n", __func__, cmd->grOffsetLo);
  CDBG("%s: cmd->gbOffsetHi = %d \n", __func__, cmd->gbOffsetHi);
  CDBG("%s: cmd->gbOffsetLo = %d \n", __func__, cmd->gbOffsetLo);
}/*vfe_demosaic_bcc_debug*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bcc_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bcc_update(int mod_id, void *bcc_mod, void *params)
{
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  VFE_DemosaicDBCC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bcc_ctrl->bcc_snap_cmd) : &(bcc_ctrl->bcc_prev_cmd);

  CDBG("%s: %d %d", __func__, bcc_ctrl->hw_enable_cmd,
    bcc_ctrl->bcc_trigger_update);

  vfe_demosaic_bcc_debug(cmd);
  if (bcc_ctrl->hw_enable_cmd) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *) cmd, sizeof(VFE_DemosaicDBCC_CmdType),
      VFE_CMD_DEMOSAICV3_DBCC_UPDATE)) {
      CDBG_ERROR("%s: DBPC update for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
    vfe_params->update |= VFE_MOD_BCC;
    bcc_ctrl->hw_enable_cmd = FALSE;
  }

  if (!bcc_ctrl->bcc_enable) {
    CDBG("%s: bcc is not enabled. Skip the update\n", __func__);
    return VFE_SUCCESS;
  }

  if (!bcc_ctrl->bcc_trigger_update) {
    CDBG("%s: Trigger is not be valid. Skip the update", __func__);
    return VFE_SUCCESS;
  }

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(VFE_DemosaicDBCC_CmdType),
    VFE_CMD_DEMOSAICV3_DBCC_UPDATE)) {
    CDBG_ERROR("%s: DBCC update for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  bcc_ctrl->bcc_trigger_update = FALSE;
  vfe_params->update |= VFE_MOD_BCC;

  return VFE_SUCCESS;
} /* vfe_demosaic_bcc_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bcc_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bcc_trigger_update(int mod_id, void *bcc_mod,
  void *params)
{
  uint8_t Fmin, Fmin_lowlight, Fmax, Fmax_lowlight;
  tuning_control_type tunning_control;
  float new_trigger_ratio;
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  VFE_DemosaicDBCC_CmdType *cmd = NULL;
  bpc_4_offset_type *bcc_normal_input_offset = NULL;
  bpc_4_offset_type *bcc_lowlight_input_offset = NULL;
  trigger_point_type *trigger_point = NULL;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  if (!bcc_ctrl->bcc_enable) {
    CDBG("%s: bcc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  bcc_ctrl->bcc_trigger_update = FALSE;
  tunning_control = chrPtr->control_bcc;

  if (!bcc_ctrl->bcc_trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    trigger_point = &(chrPtr->bcc_snapshot_lowlight_trigger);
    Fmin = chrPtr->bcc_Fmin_snapshot;
    Fmax = chrPtr->bcc_Fmax_snapshot;
    Fmin_lowlight = chrPtr->bcc_Fmin_snapshot_lowlight;
    Fmax_lowlight = chrPtr->bcc_Fmax_snapshot_lowlight;
    bcc_normal_input_offset = &(chrPtr->bcc_4_offset_snapshot[BPC_NORMAL_LIGHT]);
    bcc_lowlight_input_offset = &(chrPtr->bcc_4_offset_snapshot[BPC_LOW_LIGHT]);
    cmd = &(bcc_ctrl->bcc_snap_cmd);
  } else {
    if (!vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
      if (!bcc_ctrl->bcc_reload_params) {
        CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
        return VFE_SUCCESS;
      }
    }

    trigger_point = &(chrPtr->bcc_lowlight_trigger);
    Fmin = chrPtr->bcc_Fmin_preview;
    Fmax = chrPtr->bcc_Fmax_preview;
    Fmin_lowlight = chrPtr->bcc_Fmin_preview_lowlight;
    Fmax_lowlight = chrPtr->bcc_Fmax_preview_lowlight;
    bcc_normal_input_offset = &(chrPtr->bcc_4_offset[BPC_NORMAL_LIGHT]);
    bcc_lowlight_input_offset = &(chrPtr->bcc_4_offset[BPC_LOW_LIGHT]);
    cmd = &(bcc_ctrl->bcc_prev_cmd);
  }

  new_trigger_ratio = vfe_util_get_aec_ratio(tunning_control, trigger_point,
    vfe_params);
 
  if (bcc_ctrl->cur_mode != vfe_params->vfe_op_mode ||
    bcc_ctrl->bcc_reload_params ||
    !F_EQUAL(new_trigger_ratio, bcc_ctrl->bcc_trigger_ratio)) {

    cmd->fminThreshold = (uint8_t)LINEAR_INTERPOLATION(Fmin, Fmin_lowlight,
      new_trigger_ratio);
    cmd->fmaxThreshold = (uint8_t)LINEAR_INTERPOLATION(Fmax, Fmax_lowlight,
      new_trigger_ratio);

    cmd->rOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_r_hi,
      bcc_lowlight_input_offset->bpc_4_offset_r_hi, new_trigger_ratio);
    cmd->rOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_r_lo,
      bcc_lowlight_input_offset->bpc_4_offset_r_lo, new_trigger_ratio);

    cmd->bOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_b_hi,
      bcc_lowlight_input_offset->bpc_4_offset_b_hi, new_trigger_ratio);
    cmd->bOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_b_lo,
      bcc_lowlight_input_offset->bpc_4_offset_b_lo, new_trigger_ratio);

    cmd->grOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gr_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gr_hi, new_trigger_ratio);
    cmd->grOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gr_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gr_lo, new_trigger_ratio);

    cmd->gbOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gb_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gb_hi, new_trigger_ratio);
    cmd->gbOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gb_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gb_lo, new_trigger_ratio);

    bcc_ctrl->cur_mode = vfe_params->vfe_op_mode;
    bcc_ctrl->bcc_trigger_ratio = new_trigger_ratio;
    bcc_ctrl->bcc_trigger_update = TRUE;
    bcc_ctrl->bcc_reload_params = FALSE;
  } else {
    CDBG("%s: No update required.\n", __func__);
  }

  return VFE_SUCCESS;
} /* vfe_demosaic_bcc_trigger_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bcc_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bcc_config(int mod_id, void *bcc_mod, void *params)
{
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  if (!bcc_ctrl->bcc_enable) {
    CDBG("%s: bcc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  VFE_DemosaicDBCC_CmdType *cmd =
    (IS_SNAP_MODE(vfe_params) && bcc_ctrl->bcc_trigger_enable) ?
    &(bcc_ctrl->bcc_snap_cmd) : &(bcc_ctrl->bcc_prev_cmd);
  vfe_demosaic_bcc_debug(cmd);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(VFE_DemosaicDBCC_CmdType),
    VFE_CMD_DEMOSAICV3_DBCC_CFG)) {
    CDBG_ERROR("%s: DBCC config for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_demosaic_bcc_config */

/*===========================================================================
 * FUNCTION.   - vfe_demosaic_bcc_enable -
 *
 * DESCRIPTION: BCC will be enabled by HW only if Demosaic is enabled.
 *========================================================================*/
vfe_status_t vfe_demosaic_bcc_enable(int mod_id, void *bcc_mod,
  void *params, int8_t enable, int8_t hw_write)
{
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;

  VFE_DemosaicDBCC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bcc_ctrl->bcc_snap_cmd) : &(bcc_ctrl->bcc_prev_cmd);

  CDBG("%s: enable=%d, hw_write=%d\n", __func__, enable, hw_write);

  if (hw_write && (bcc_ctrl->bcc_enable == enable))
    return VFE_SUCCESS;

  bcc_ctrl->bcc_enable = enable;

  cmd->enable = enable;
  bcc_ctrl->hw_enable_cmd = hw_write;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_BCC)
      : (vfe_params->current_config & ~VFE_MOD_BCC);
  }

  return VFE_SUCCESS;
} /* vfe_demosaic_bcc_enable */

/*===========================================================================
 * FUNCTION.   - vfe_demosaic_bcc_init -
 *
 * DESCRIPTION: BCC will be enabled by HW only if Demosaic is enabled.
 *========================================================================*/
vfe_status_t vfe_demosaic_bcc_init(int mod_id, void *bcc_mod, void *params)
{
  int i;
  VFE_DemosaicDBCC_CmdType *cmd = NULL;
  bpc_4_offset_type *bcc_input_offset = NULL;
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  bcc_ctrl->bcc_enable = TRUE;
  bcc_ctrl->bcc_trigger_update = FALSE;
  bcc_ctrl->bcc_trigger_enable = TRUE;
  bcc_ctrl->bcc_reload_params = FALSE;
  bcc_ctrl->hw_enable_cmd = FALSE;
  bcc_ctrl->cur_mode = VFE_OP_MODE_INVALID;

  bcc_ctrl->bcc_trigger_ratio = 0;

  for (i = 0; i < 2; i++) {
    if (i == 0) {
      /* Preview Init */
      cmd = &(bcc_ctrl->bcc_prev_cmd);
      bcc_input_offset = &(chrPtr->bcc_4_offset[BPC_NORMAL_LIGHT]);

      /* Make sure that we doesnt exceed the boundaries */
      cmd->fminThreshold = MIN(63, chrPtr->bcc_Fmin_preview);
      cmd->fmaxThreshold = MIN(127, chrPtr->bcc_Fmax_preview);
    } else {
      /* Snapshot Init */
      cmd = &(bcc_ctrl->bcc_snap_cmd);
      bcc_input_offset = &(chrPtr->bcc_4_offset_snapshot[BPC_NORMAL_LIGHT]);

      /* Make sure that we doesnt exceed the boundaries */
      cmd->fminThreshold = MIN(63, chrPtr->bcc_Fmin_snapshot);
      cmd->fmaxThreshold = MIN(127, chrPtr->bcc_Fmax_snapshot);
    }

    cmd->rOffsetHi = bcc_input_offset->bpc_4_offset_r_hi;
    cmd->rOffsetLo = bcc_input_offset->bpc_4_offset_r_lo;
    cmd->bOffsetHi = bcc_input_offset->bpc_4_offset_b_hi;
    cmd->bOffsetLo = bcc_input_offset->bpc_4_offset_b_lo;
    cmd->grOffsetLo = bcc_input_offset->bpc_4_offset_gr_lo;
    cmd->grOffsetHi = bcc_input_offset->bpc_4_offset_gr_hi;
    cmd->gbOffsetLo = bcc_input_offset->bpc_4_offset_gb_lo;
    cmd->gbOffsetHi = bcc_input_offset->bpc_4_offset_gb_hi;
  }

  return VFE_SUCCESS;
} /* vfe_demosaic_bcc_init */

/*=============================================================================
 * Function:               vfe_demosaic_bcc_deinit
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_demosaic_bcc_deinit(int mod_id, void *bcc_mod, void *params)
{
  return VFE_SUCCESS;
} /* vfe_demosaic_bcc_deinit */

/*=============================================================================
 * Function:               vfe_bcc_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_bcc_trigger_enable(int mod_id, void *bcc_mod,
  void *params, int enable)
{
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;

  CDBG("%s: new trigger enable value = %d\n", __func__, enable);
  bcc_ctrl->bcc_trigger_enable = enable;

  return VFE_SUCCESS;
} /* vfe_bpc_trigger_enable */

/*===========================================================================
 * FUNCTION    - vfe_bcc_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bcc_reload_params(int mod_id, void *bcc_mod, void *params)
{
  bcc_mod_t *bcc_ctrl = (bcc_mod_t *)bcc_mod;

  CDBG("%s: reload the chromatix\n", __func__);
  bcc_ctrl->bcc_reload_params = TRUE;

  return VFE_SUCCESS;
} /* vfe_bcc_reload_params */

/*===========================================================================
 * FUNCTION    - vfe_bcc_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bcc_test_vector_validation(int mod_id, void *in, void *op)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

  VFE_DemosaicDBCC_CmdType *InCmd =
    (VFE_DemosaicDBCC_CmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_0_OFF/4));

  VFE_DemosaicDBCC_CmdType *OutCmd =
    (VFE_DemosaicDBCC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_0_OFF/4));

  VALIDATE_TST_VEC(InCmd->enable, OutCmd->enable, 0,
    "enable");

  //4 bytes back to set the correct location
  InCmd = (VFE_DemosaicDBCC_CmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_DBCC_OFF/4) - 1);

  //4 bytes back to set the correct location
  OutCmd = (VFE_DemosaicDBCC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_DBCC_OFF/4) - 1);

  VALIDATE_TST_VEC(InCmd->fminThreshold, OutCmd->fminThreshold, 0,
    "fminThreshold");
  VALIDATE_TST_VEC(InCmd->fmaxThreshold, OutCmd->fmaxThreshold, 0,
    "fmaxThreshold");

  VALIDATE_TST_VEC(InCmd->rOffsetLo, OutCmd->rOffsetLo, 0,
    "rOffsetLo");
  VALIDATE_TST_VEC(InCmd->rOffsetHi, OutCmd->rOffsetHi, 0,
    "rOffsetHi");
  VALIDATE_TST_VEC(InCmd->grOffsetLo, OutCmd->grOffsetLo, 0,
    "grOffsetLo");

  VALIDATE_TST_VEC(InCmd->gbOffsetLo, OutCmd->gbOffsetLo, 0,
    "gbOffsetLo");
  VALIDATE_TST_VEC(InCmd->gbOffsetHi, OutCmd->gbOffsetHi, 0,
    "gbOffsetHi");
  VALIDATE_TST_VEC(InCmd->grOffsetHi, OutCmd->grOffsetHi, 0,
    "grOffsetHi");

  VALIDATE_TST_VEC(InCmd->bOffsetLo, OutCmd->bOffsetLo, 0,
    "bOffsetLo");
  VALIDATE_TST_VEC(InCmd->bOffsetHi, OutCmd->bOffsetHi, 0,
    "bOffsetHi");

  return VFE_SUCCESS;
} /*vfe_bcc_test_vector_validation*/

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_bcc_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bcc_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  bcc_module_t *cmd = (bcc_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&(cmd->bcc_cmd), sizeof(VFE_DemosaicDBCC_CmdType),
     VFE_CMD_DEMOSAICV3_DBCC_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_bcc_plugin_update */
#endif
