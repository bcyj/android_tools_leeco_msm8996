/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BPC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bpc_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_demosaic_bpc_debug(VFE_DemosaicDBPC_CmdType *cmd)
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
}/*vfe_demosaic_bpc_debug*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bpc_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bpc_update(int mod_id, void *bpc_mod, void *params)
{
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  VFE_DemosaicDBPC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  CDBG("%s: %d %d", __func__, bpc_ctrl->hw_enable_cmd,
    bpc_ctrl->bpc_trigger_update);

  vfe_demosaic_bpc_debug(cmd);
  if (bpc_ctrl->hw_enable_cmd) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *) cmd, sizeof(VFE_DemosaicDBPC_CmdType),
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
    (void *) cmd, sizeof(VFE_DemosaicDBPC_CmdType),
    VFE_CMD_DEMOSAICV3_DBPC_UPDATE)) {
    CDBG_ERROR("%s: DBPC update for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  bpc_ctrl->bpc_trigger_update = FALSE;
  vfe_params->update |= VFE_MOD_BPC;

  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bpc_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bpc_trigger_update(int mod_id, void *bpc_mod,
  void *params)
{
  uint8_t Fmin, Fmin_lowlight, Fmax, Fmax_lowlight;
  tuning_control_type tunning_control;
  float new_trigger_ratio;
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  VFE_DemosaicDBPC_CmdType *cmd = NULL;
  bpc_4_offset_type *bpc_normal_input_offset = NULL;
  bpc_4_offset_type *bpc_lowlight_input_offset = NULL;
  trigger_point_type *trigger_point = NULL;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  if (!bpc_ctrl->bpc_enable) {
    CDBG("%s: bpc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  bpc_ctrl->bpc_trigger_update = FALSE;
  tunning_control = chrPtr->control_bpc;

  if (!bpc_ctrl->bpc_trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    trigger_point = &(chrPtr->bpc_snapshot_lowlight_trigger);
    Fmin = chrPtr->bpc_Fmin_snapshot;
    Fmax = chrPtr->bpc_Fmax_snapshot;
    Fmin_lowlight = chrPtr->bpc_Fmin_snapshot_lowlight;
    Fmax_lowlight = chrPtr->bpc_Fmax_snapshot_lowlight;
    bpc_normal_input_offset = &(chrPtr->bpc_4_offset_snapshot[BPC_NORMAL_LIGHT]);
    bpc_lowlight_input_offset = &(chrPtr->bpc_4_offset_snapshot[BPC_LOW_LIGHT]);
    cmd = &(bpc_ctrl->bpc_snap_cmd);
  } else {
    if (!vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
      if (!bpc_ctrl->bpc_reload_params) {
        CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
        return VFE_SUCCESS;
      }
    }

    trigger_point = &(chrPtr->bpc_lowlight_trigger);
    Fmin = chrPtr->bpc_Fmin_preview;
    Fmax = chrPtr->bpc_Fmax_preview;
    Fmin_lowlight = chrPtr->bpc_Fmin_preview_lowlight;
    Fmax_lowlight = chrPtr->bpc_Fmax_preview_lowlight;
    bpc_normal_input_offset = &(chrPtr->bpc_4_offset[BPC_NORMAL_LIGHT]);
    bpc_lowlight_input_offset = &(chrPtr->bpc_4_offset[BPC_LOW_LIGHT]);
    cmd = &(bpc_ctrl->bpc_prev_cmd);
  }

  new_trigger_ratio = vfe_util_get_aec_ratio(tunning_control, trigger_point,
    vfe_params);

  if ((bpc_ctrl->cur_mode != vfe_params->vfe_op_mode) ||
    bpc_ctrl->bpc_reload_params ||
    !F_EQUAL(new_trigger_ratio, bpc_ctrl->bpc_trigger_ratio)) {

    cmd->fminThreshold = (uint8_t)LINEAR_INTERPOLATION(Fmin, Fmin_lowlight,
      new_trigger_ratio);
    cmd->fmaxThreshold = (uint8_t)LINEAR_INTERPOLATION(Fmax, Fmax_lowlight,
      new_trigger_ratio);

    cmd->rOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_r_hi,
      bpc_lowlight_input_offset->bpc_4_offset_r_hi, new_trigger_ratio);
    cmd->rOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_r_lo,
      bpc_lowlight_input_offset->bpc_4_offset_r_lo, new_trigger_ratio);

    cmd->bOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_b_hi,
      bpc_lowlight_input_offset->bpc_4_offset_b_hi, new_trigger_ratio);
    cmd->bOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_b_lo,
      bpc_lowlight_input_offset->bpc_4_offset_b_lo, new_trigger_ratio);

    cmd->grOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gr_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gr_hi, new_trigger_ratio);
    cmd->grOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gr_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gr_lo, new_trigger_ratio);

    cmd->gbOffsetHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gb_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gb_hi, new_trigger_ratio);
    cmd->gbOffsetLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gb_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gb_lo, new_trigger_ratio);

    bpc_ctrl->cur_mode = vfe_params->vfe_op_mode;
    bpc_ctrl->bpc_trigger_ratio = new_trigger_ratio;
    bpc_ctrl->bpc_trigger_update = TRUE;
    bpc_ctrl->bpc_reload_params = FALSE;
  } else {
    CDBG("%s: No update required.\n", __func__);
  }

  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_trigger_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bpc_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bpc_config(int mod_id, void *bpc_mod, void *params)
{
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  if (!bpc_ctrl->bpc_enable) {
    CDBG("%s: bpc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  VFE_DemosaicDBPC_CmdType *cmd =
    (IS_SNAP_MODE(vfe_params) && bpc_ctrl->bpc_trigger_enable) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  vfe_demosaic_bpc_debug(cmd);
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(VFE_DemosaicDBPC_CmdType),
    VFE_CMD_DEMOSAICV3_DBPC_CFG)) {
    CDBG_ERROR("%s: DBPC config for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_config */

/*===========================================================================
 * FUNCTION.   - vfe_demosaic_bpc_enable -
 *
 * DESCRIPTION: BPC will be enabled by HW only if Demosaic is enabled.
 *========================================================================*/
vfe_status_t vfe_demosaic_bpc_enable(int mod_id, void *bpc_mod,
  void *params, int8_t enable, int8_t hw_write)
{
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;

  VFE_DemosaicDBPC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  CDBG("%s: enable=%d, hw_write=%d\n", __func__, enable, hw_write);

  if (hw_write && (bpc_ctrl->bpc_enable == enable))
    return VFE_SUCCESS;

  bpc_ctrl->bpc_enable = enable;
  cmd->enable = enable;

  bpc_ctrl->hw_enable_cmd = hw_write;
  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_BPC)
      : (vfe_params->current_config & ~VFE_MOD_BPC);
  }

  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_enable */

/*===========================================================================
 * FUNCTION.   - vfe_demosaic_bpc_init -
 *
 * DESCRIPTION: BPC will be enabled by HW only if Demosaic is enabled.
 *========================================================================*/
vfe_status_t vfe_demosaic_bpc_init(int mod_id, void *bpc_mod,
  void *params)
{
  int i;
  VFE_DemosaicDBPC_CmdType *cmd = NULL;
  bpc_4_offset_type *bpc_input_offset = NULL;
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  bpc_ctrl->bpc_enable = TRUE;
  bpc_ctrl->bpc_trigger_update = FALSE;
  bpc_ctrl->bpc_trigger_enable = TRUE;
  bpc_ctrl->bpc_reload_params = FALSE;
  bpc_ctrl->hw_enable_cmd = FALSE;
  bpc_ctrl->cur_mode = VFE_OP_MODE_INVALID;

  bpc_ctrl->bpc_trigger_ratio = 0;

  for (i = 0; i < 2; i++) {
    if (i == 0) {
      /* Preview Init */
      cmd = &(bpc_ctrl->bpc_prev_cmd);
      bpc_input_offset = &(chrPtr->bpc_4_offset[BPC_NORMAL_LIGHT]);

      /* Make sure that we doesnt exceed the boundaries */
      cmd->fminThreshold = MIN(63, chrPtr->bpc_Fmin_preview);
      cmd->fmaxThreshold = MIN(127, chrPtr->bpc_Fmax_preview);
    } else {
      /* Snapshot Init */
      cmd = &(bpc_ctrl->bpc_snap_cmd);
      bpc_input_offset = &(chrPtr->bpc_4_offset_snapshot[BPC_NORMAL_LIGHT]);

      /* Make sure that we doesnt exceed the boundaries */
      cmd->fminThreshold = MIN(63, chrPtr->bpc_Fmin_snapshot);
      cmd->fmaxThreshold = MIN(127, chrPtr->bpc_Fmax_snapshot);
    }

    cmd->rOffsetHi = bpc_input_offset->bpc_4_offset_r_hi;
    cmd->rOffsetLo = bpc_input_offset->bpc_4_offset_r_lo;
    cmd->bOffsetHi = bpc_input_offset->bpc_4_offset_b_hi;
    cmd->bOffsetLo = bpc_input_offset->bpc_4_offset_b_lo;
    cmd->grOffsetLo = bpc_input_offset->bpc_4_offset_gr_lo;
    cmd->grOffsetHi = bpc_input_offset->bpc_4_offset_gr_hi;
    cmd->gbOffsetLo = bpc_input_offset->bpc_4_offset_gb_lo;
    cmd->gbOffsetHi = bpc_input_offset->bpc_4_offset_gb_hi;
  }

  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_init */

/*=============================================================================
 * Function:               vfe_demosaic_bpc_deinit
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_demosaic_bpc_deinit(int mod_id, void *bpc_mod,
  void* params)
{
  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_deinit */

/*=============================================================================
 * Function:               vfe_bpc_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_bpc_trigger_enable(int mod_id, void* bpc_mod,
  void *params, int enable)
{
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  CDBG("%s: new trigger enable value = %d\n", __func__, enable);
  bpc_ctrl->bpc_trigger_enable = enable;

  return VFE_SUCCESS;
} /* vfe_bpc_trigger_enable */

/*===========================================================================
 * FUNCTION    - vfe_bpc_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bpc_reload_params(int mod_id, void *bpc_mod,
  void *params)
{
  bpc_mod_t *bpc_ctrl = (bpc_mod_t *)bpc_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  CDBG("%s: reload the chromatix\n", __func__);
  bpc_ctrl->bpc_reload_params = TRUE;

  return VFE_SUCCESS;
} /* vfe_bpc_reload_params */

/*===========================================================================
 * FUNCTION    - vfe_bpc_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bpc_test_vector_validation(int mod_id, void *in, void *op)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

  VFE_DemosaicDBPC_CmdType *InCmd =
    (VFE_DemosaicDBPC_CmdType *)(mod_in->reg_dump + (VFE_DEMOSAICV3_0_OFF/4));

  VFE_DemosaicDBPC_CmdType *OutCmd =
    (VFE_DemosaicDBPC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_0_OFF/4));

  VALIDATE_TST_VEC(InCmd->enable, OutCmd->enable, 0,
    "enable");

  //4 bytes back to set the correct location
  InCmd = (VFE_DemosaicDBPC_CmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF/4) - 1);

  //4 bytes back to set the correct location
  OutCmd = (VFE_DemosaicDBPC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF/4) - 1);

  VALIDATE_TST_VEC(InCmd->fminThreshold, OutCmd->fminThreshold, 0,
    "fminThreshold");
  VALIDATE_TST_VEC(InCmd->fmaxThreshold, OutCmd->fmaxThreshold, 0,
    "fmaxThreshold");

  //8 bytes back to set the correct location
  InCmd = (VFE_DemosaicDBPC_CmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF0/4) - 2);

  //8 bytes back to set the correct location
  OutCmd = (VFE_DemosaicDBPC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF0/4) - 2);

  VALIDATE_TST_VEC(InCmd->rOffsetLo, OutCmd->rOffsetLo, 0,
    "rOffsetLo");
  VALIDATE_TST_VEC(InCmd->rOffsetHi, OutCmd->rOffsetHi, 0,
    "rOffsetHi");
  VALIDATE_TST_VEC(InCmd->grOffsetLo, OutCmd->grOffsetLo, 0,
    "grOffsetLo");

  InCmd = (VFE_DemosaicDBPC_CmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF1/4) - 3);
  //8 bytes back to set the correct location

  OutCmd = (VFE_DemosaicDBPC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF1/4) - 3);
  //8 bytes back to set the correct location

  VALIDATE_TST_VEC(InCmd->gbOffsetLo, OutCmd->gbOffsetLo, 0,
    "gbOffsetLo");
  VALIDATE_TST_VEC(InCmd->gbOffsetHi, OutCmd->gbOffsetHi, 0,
    "gbOffsetHi");
  VALIDATE_TST_VEC(InCmd->grOffsetHi, OutCmd->grOffsetHi, 0,
    "grOffsetHi");

  InCmd = (VFE_DemosaicDBPC_CmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF2/4) - 4);
  //8 bytes back to set the correct location

  OutCmd = (VFE_DemosaicDBPC_CmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_DBPC_CFG_OFF2/4) - 4);
  //8 bytes back to set the correct location

  VALIDATE_TST_VEC(InCmd->bOffsetLo, OutCmd->bOffsetLo, 0,
    "bOffsetLo");
  VALIDATE_TST_VEC(InCmd->bOffsetHi, OutCmd->bOffsetHi, 0,
    "bOffsetHi");

  return VFE_SUCCESS;
} /*vfe_bpc_test_vector_validation*/

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_bpc_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bpc_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  bpc_module_t *cmd = (bpc_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&(cmd->bpc_cmd), sizeof(VFE_DemosaicDBPC_CmdType),
     VFE_CMD_DEMOSAICV3_DBPC_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_bcc_plugin_update */

#endif
