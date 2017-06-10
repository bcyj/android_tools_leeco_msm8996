/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>

#include "camera_dbg.h"
#include "camera_defs_i.h"
#include "bpc_v3_7x27a.h"

#ifdef ENABLE_BPC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bpc_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bpc_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params)
{
  VFE_DemosaicDBPC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  CDBG("%s: %d %d", __func__, bpc_ctrl->hw_enable_cmd,
    bpc_ctrl->trigger_update);
  memcpy(&(bpc_ctrl->bpc_update_cmd), cmd,
    sizeof(VFE_DemosaicDBPC_CmdType) -  (sizeof(VFE_abccLutType) * 512));

  if (bpc_ctrl->hw_enable_cmd) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *) &(bpc_ctrl->bpc_update_cmd), sizeof(VFE_DemosaicBPCUpdate_CmdType),
      VFE_CMD_DEMOSAICV3_DBPC_UPDATE)) {
      CDBG_ERROR("%s: DBPC update for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
    vfe_params->update |= VFE_MOD_BPC;
    bpc_ctrl->hw_enable_cmd = FALSE;
  }

  if (!bpc_ctrl->enable) {
    CDBG("%s: bpc is not enabled. Skip the update\n", __func__);
    return VFE_SUCCESS;
  }

  if (!bpc_ctrl->trigger_update) {
    CDBG("%s: Trigger is not valid. Skip the update", __func__);
    return VFE_SUCCESS;
  }

  CDBG("dbpcEnable %d", cmd->dbpcEnable);
  CDBG("dbccenable %d", cmd->dbccenable);
  CDBG("abccEnable %d", cmd->abccEnable);
  CDBG("abccLutSel %d", cmd->abccLutSel);
  CDBG("BPC bpc fminThreshold %d", cmd->dbpcFmin);
  CDBG("BPC bpc fmaxThreshold %d", cmd->dbpcFmax);
  CDBG("BPC RdbpcOffLo %d", cmd->RdbpcOffLo);
  CDBG("BPC RdbpcOffHi %d", cmd->RdbpcOffHi);
  CDBG("BPC GRdbpcOffLo %d", cmd->GRdbpcOffLo);
  CDBG("BPC GRdbpcOffHi %d", cmd->GRdbpcOffHi);
  CDBG("BPC GBdbpcOffLo %d", cmd->GBdbpcOffLo);
  CDBG("BPC GBdbpcOffHi %d", cmd->GBdbpcOffHi);
  CDBG("BPC BdbpcOffLo %d", cmd->BdbpcOffLo);
  CDBG("BPC BdbpcOffHi %d", cmd->BdbpcOffHi);

  CDBG("BPC bcc fminThreshold %d", cmd->dbccFmin);
  CDBG("BPC bcc fmaxThreshold %d", cmd->dbccFmax);
  CDBG("BPC RdbccOffLo %d", cmd->RdbccOffLo);
  CDBG("BPC RdbccOffHi %d", cmd->RdbccOffHi);
  CDBG("BPC GRdbccOffLo %d", cmd->GRdbccOffLo);
  CDBG("BPC GRdbccOffHi %d", cmd->GRdbccOffHi);
  CDBG("BPC GBdbccOffLo %d", cmd->GBdbccOffLo);
  CDBG("BPC GBdbccOffHi %d", cmd->GBdbccOffHi);
  CDBG("BPC BdbccOffLo %d", cmd->BdbccOffLo);
  CDBG("BPC BdbccOffHi %d", cmd->BdbccOffHi);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) &(bpc_ctrl->bpc_update_cmd), sizeof(VFE_DemosaicBPCUpdate_CmdType),
    VFE_CMD_DEMOSAICV3_DBPC_UPDATE)) {
    CDBG_ERROR("%s: DBPC update for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  bpc_ctrl->trigger_update = FALSE;
  vfe_params->update |= VFE_MOD_BPC;

  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_bpc_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_bpc_trigger_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params)
{

  uint8_t dbpcFmin, dbpcFmin_lowlight, dbpcFmax, dbpcFmax_lowlight;
  uint8_t dbccFmin, dbccFmin_lowlight, dbccFmax, dbccFmax_lowlight;
  tuning_control_type bpc_tuning_control, bcc_tuning_control;
  float new_bpc_trigger_ratio, new_bcc_trigger_ratio;
  VFE_DemosaicDBPC_CmdType *cmd = NULL;
  bpc_4_offset_type *bpc_normal_input_offset = NULL,
    *bcc_normal_input_offset = NULL;
  bpc_4_offset_type *bpc_lowlight_input_offset = NULL,
    *bcc_lowlight_input_offset = NULL;
  trigger_point_type *bpc_trigger_point = NULL, *bcc_trigger_point = NULL;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  static vfe_op_mode_t cur_mode = VFE_OP_MODE_INVALID;

  if (!bpc_ctrl->enable) {
    CDBG("%s: bpc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  bpc_ctrl->trigger_update = FALSE;
  bpc_tuning_control = chrPtr->control_bpc;
  bcc_tuning_control = chrPtr->control_bcc;

  if (!bpc_ctrl->trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    bpc_trigger_point = &(chrPtr->bpc_snapshot_lowlight_trigger);
    bcc_trigger_point = &(chrPtr->bcc_snapshot_lowlight_trigger);

    dbpcFmin = chrPtr->bpc_Fmin_snapshot;
    dbpcFmax = chrPtr->bpc_Fmax_snapshot;
    dbpcFmin_lowlight = chrPtr->bpc_Fmin_snapshot_lowlight;
    dbpcFmax_lowlight = chrPtr->bpc_Fmax_snapshot_lowlight;

    dbccFmin = chrPtr->bcc_Fmin_snapshot;
    dbccFmax = chrPtr->bcc_Fmax_snapshot;
    dbccFmin_lowlight = chrPtr->bcc_Fmin_snapshot_lowlight;
    dbccFmax_lowlight = chrPtr->bcc_Fmax_snapshot_lowlight;

    bpc_normal_input_offset = &(chrPtr->bpc_4_offset_snapshot[BPC_NORMAL_LIGHT]);
    bpc_lowlight_input_offset = &(chrPtr->bpc_4_offset_snapshot[BPC_LOW_LIGHT]);
    bcc_normal_input_offset = &(chrPtr->bcc_4_offset_snapshot[BPC_NORMAL_LIGHT]);
    bcc_lowlight_input_offset = &(chrPtr->bcc_4_offset_snapshot[BPC_LOW_LIGHT]);
    cmd = &(bpc_ctrl->bpc_snap_cmd);
  } else {
    if (!vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
      if (!bpc_ctrl->reload_params) {
        CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
        return VFE_SUCCESS;
      }
    }

    bpc_trigger_point = &(chrPtr->bpc_lowlight_trigger);
    bcc_trigger_point = &(chrPtr->bcc_lowlight_trigger);

    dbpcFmin = chrPtr->bpc_Fmin_preview;
    dbpcFmax = chrPtr->bpc_Fmax_preview;
    dbpcFmin_lowlight = chrPtr->bpc_Fmin_preview_lowlight;
    dbpcFmax_lowlight = chrPtr->bpc_Fmax_preview_lowlight;

    dbccFmin = chrPtr->bcc_Fmin_preview;
    dbccFmax = chrPtr->bcc_Fmax_preview;
    dbccFmin_lowlight = chrPtr->bcc_Fmin_preview_lowlight;
    dbccFmax_lowlight = chrPtr->bcc_Fmax_preview_lowlight;

    bpc_normal_input_offset = &(chrPtr->bpc_4_offset[BPC_NORMAL_LIGHT]);
    bpc_lowlight_input_offset = &(chrPtr->bpc_4_offset[BPC_LOW_LIGHT]);
    bcc_normal_input_offset = &(chrPtr->bcc_4_offset[BPC_NORMAL_LIGHT]);
    bcc_lowlight_input_offset = &(chrPtr->bcc_4_offset[BPC_LOW_LIGHT]);
    cmd = &(bpc_ctrl->bpc_prev_cmd);
  }

  new_bpc_trigger_ratio = vfe_util_get_aec_ratio(bpc_tuning_control, bpc_trigger_point,
    vfe_params);
  new_bcc_trigger_ratio = vfe_util_get_aec_ratio(bcc_tuning_control, bcc_trigger_point,
    vfe_params);

  if (cur_mode != vfe_params->vfe_op_mode || bpc_ctrl->reload_params ||
    !F_EQUAL(new_bpc_trigger_ratio, bpc_ctrl->bpc_trigger_ratio) ||
    !F_EQUAL(new_bcc_trigger_ratio, bpc_ctrl->bcc_trigger_ratio)) {

    cmd->dbpcFmin = (uint8_t)LINEAR_INTERPOLATION(dbpcFmin, dbpcFmin_lowlight,
      new_bpc_trigger_ratio);
    cmd->dbpcFmax = (uint8_t)LINEAR_INTERPOLATION(dbpcFmax, dbpcFmax_lowlight,
      new_bpc_trigger_ratio);

    cmd->RdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_r_hi,
      bpc_lowlight_input_offset->bpc_4_offset_r_hi, new_bpc_trigger_ratio);
    cmd->RdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_r_lo,
      bpc_lowlight_input_offset->bpc_4_offset_r_lo, new_bpc_trigger_ratio);

    cmd->BdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_b_hi,
      bpc_lowlight_input_offset->bpc_4_offset_b_hi, new_bpc_trigger_ratio);
    cmd->BdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_b_lo,
      bpc_lowlight_input_offset->bpc_4_offset_b_lo, new_bpc_trigger_ratio);

    cmd->GRdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gr_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gr_hi, new_bpc_trigger_ratio);
    cmd->GRdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gr_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gr_lo, new_bpc_trigger_ratio);

    cmd->GBdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gb_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gb_hi, new_bpc_trigger_ratio);
    cmd->GBdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bpc_normal_input_offset->bpc_4_offset_gb_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gb_lo, new_bpc_trigger_ratio);

    cmd->dbccFmin = (uint8_t)LINEAR_INTERPOLATION(dbccFmin, dbccFmin_lowlight,
      new_bcc_trigger_ratio);
    cmd->dbccFmax = (uint8_t)LINEAR_INTERPOLATION(dbccFmax, dbccFmax_lowlight,
      new_bcc_trigger_ratio);

    cmd->RdbccOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_r_hi,
      bcc_lowlight_input_offset->bpc_4_offset_r_hi, new_bcc_trigger_ratio);
    cmd->RdbccOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_r_lo,
      bcc_lowlight_input_offset->bpc_4_offset_r_lo, new_bcc_trigger_ratio);

    cmd->BdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_b_hi,
      bcc_lowlight_input_offset->bpc_4_offset_b_hi, new_bcc_trigger_ratio);
    cmd->BdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_b_lo,
      bcc_lowlight_input_offset->bpc_4_offset_b_lo, new_bcc_trigger_ratio);

    cmd->GRdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gr_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gr_hi, new_bcc_trigger_ratio);
    cmd->GRdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gr_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gr_lo, new_bcc_trigger_ratio);

    cmd->GBdbpcOffHi = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gb_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gb_hi, new_bcc_trigger_ratio);
    cmd->GBdbpcOffLo = (uint16_t)LINEAR_INTERPOLATION(
      bcc_normal_input_offset->bpc_4_offset_gb_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gb_lo, new_bcc_trigger_ratio);

    cur_mode = vfe_params->vfe_op_mode;
    bpc_ctrl->bpc_trigger_ratio = new_bpc_trigger_ratio;
    bpc_ctrl->bcc_trigger_ratio = new_bcc_trigger_ratio;
    bpc_ctrl->trigger_update = TRUE;
    bpc_ctrl->reload_params = FALSE;
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
vfe_status_t vfe_demosaic_bpc_config(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params)
{
  if (!bpc_ctrl->enable) {
    CDBG("%s: bpc is not enabled. Skip the config\n", __func__);
    return VFE_SUCCESS;
  }

  VFE_DemosaicDBPC_CmdType *cmd =
    (IS_SNAP_MODE(vfe_params) && bpc_ctrl->trigger_enable) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  CDBG("dbpcEnable %d", cmd->dbpcEnable);
  CDBG("dbccenable %d", cmd->dbccenable);
  CDBG("abccEnable %d", cmd->abccEnable);
  CDBG("abccLutSel %d", cmd->abccLutSel);
  CDBG("BPC bpc fminThreshold %d", cmd->dbpcFmin);
  CDBG("BPC bpc fmaxThreshold %d", cmd->dbpcFmax);
  CDBG("BPC RdbpcOffLo %d", cmd->RdbpcOffLo);
  CDBG("BPC RdbpcOffHi %d", cmd->RdbpcOffHi);
  CDBG("BPC GRdbpcOffLo %d", cmd->GRdbpcOffLo);
  CDBG("BPC GRdbpcOffHi %d", cmd->GRdbpcOffHi);
  CDBG("BPC GBdbpcOffLo %d", cmd->GBdbpcOffLo);
  CDBG("BPC GBdbpcOffHi %d", cmd->GBdbpcOffHi);
  CDBG("BPC BdbpcOffLo %d", cmd->BdbpcOffLo);
  CDBG("BPC BdbpcOffHi %d", cmd->BdbpcOffHi);

  CDBG("BPC bcc fminThreshold %d", cmd->dbccFmin);
  CDBG("BPC bcc fmaxThreshold %d", cmd->dbccFmax);
  CDBG("BPC RdbccOffLo %d", cmd->RdbccOffLo);
  CDBG("BPC RdbccOffHi %d", cmd->RdbccOffHi);
  CDBG("BPC GRdbccOffLo %d", cmd->GRdbccOffLo);
  CDBG("BPC GRdbccOffHi %d", cmd->GRdbccOffHi);
  CDBG("BPC GBdbccOffLo %d", cmd->GBdbccOffLo);
  CDBG("BPC GBdbccOffHi %d", cmd->GBdbccOffHi);
  CDBG("BPC BdbccOffLo %d", cmd->BdbccOffLo);
  CDBG("BPC BdbccOffHi %d", cmd->BdbccOffHi);

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
vfe_status_t vfe_demosaic_bpc_enable(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params, int8_t enable, int8_t hw_write)
{
  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;

  VFE_DemosaicDBPC_CmdType *cmd = IS_SNAP_MODE(vfe_params) ?
    &(bpc_ctrl->bpc_snap_cmd) : &(bpc_ctrl->bpc_prev_cmd);

  CDBG("%s: enable=%d, hw_write=%d\n", __func__, enable, hw_write);

  if (hw_write && (bpc_ctrl->enable == enable))
    return VFE_SUCCESS;

  bpc_ctrl->enable = enable;
  cmd->dbpcEnable = enable;
  cmd->dbccenable = enable;
  cmd->abccEnable = 0;//enable; //TODO: check again

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
vfe_status_t vfe_demosaic_bpc_init(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params)
{
  int i;
  VFE_DemosaicDBPC_CmdType *cmd = NULL;
  bpc_4_offset_type *bpc_input_offset = NULL, *bcc_input_offset = NULL;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;
  CDBG("%s: Enter\n", __func__);

  bpc_ctrl->enable = TRUE;
  bpc_ctrl->trigger_update = FALSE;
  bpc_ctrl->trigger_enable = TRUE;
  bpc_ctrl->reload_params = FALSE;
  bpc_ctrl->hw_enable_cmd = FALSE;

  bpc_ctrl->bpc_trigger_ratio = 0.0;
  bpc_ctrl->bcc_trigger_ratio = 0.0;
  for (i = 0; i < 2; i++) {
    if (i == 0) {
      /* Preview Init */
      cmd = &(bpc_ctrl->bpc_prev_cmd);
      bpc_input_offset = &(chrPtr->bpc_4_offset[BPC_NORMAL_LIGHT]);

      /* Make sure that we doesnt exceed the boundaries */
      cmd->dbpcFmin = MIN(63, chrPtr->bpc_Fmin_preview);
      cmd->dbpcFmax = MIN(127, chrPtr->bpc_Fmax_preview);

      bcc_input_offset = &(chrPtr->bcc_4_offset[BPC_NORMAL_LIGHT]);

      cmd->dbccFmin = MIN(63, chrPtr->bcc_Fmin_preview);
      cmd->dbccFmax = MIN(127, chrPtr->bcc_Fmax_preview);

    } else {
      /* Snapshot Init */
      cmd = &(bpc_ctrl->bpc_snap_cmd);
      bpc_input_offset = &(chrPtr->bpc_4_offset_snapshot[BPC_NORMAL_LIGHT]);

      /* Make sure that we doesnt exceed the boundaries */
      cmd->dbpcFmin = MIN(63, chrPtr->bpc_Fmin_snapshot);
      cmd->dbpcFmax = MIN(127, chrPtr->bpc_Fmax_snapshot);

      bcc_input_offset = &(chrPtr->bcc_4_offset_snapshot[BPC_NORMAL_LIGHT]);

      cmd->dbccFmin = MIN(63, chrPtr->bcc_Fmin_snapshot);
      cmd->dbccFmax = MIN(127, chrPtr->bcc_Fmax_snapshot);
    }

    cmd->RdbpcOffHi = bpc_input_offset->bpc_4_offset_r_hi;
    cmd->RdbpcOffLo = bpc_input_offset->bpc_4_offset_r_lo;
    cmd->BdbpcOffHi = bpc_input_offset->bpc_4_offset_b_hi;
    cmd->BdbpcOffLo = bpc_input_offset->bpc_4_offset_b_lo;
    cmd->GRdbpcOffLo = bpc_input_offset->bpc_4_offset_gr_lo;
    cmd->GRdbpcOffHi = bpc_input_offset->bpc_4_offset_gr_hi;
    cmd->GBdbpcOffLo = bpc_input_offset->bpc_4_offset_gb_lo;
    cmd->GBdbpcOffHi = bpc_input_offset->bpc_4_offset_gb_hi;

    cmd->RdbccOffHi = bcc_input_offset->bpc_4_offset_r_hi;
    cmd->RdbccOffLo = bcc_input_offset->bpc_4_offset_r_lo;
    cmd->BdbccOffHi = bcc_input_offset->bpc_4_offset_b_hi;
    cmd->BdbccOffLo = bcc_input_offset->bpc_4_offset_b_lo;
    cmd->GRdbccOffLo = bcc_input_offset->bpc_4_offset_gr_lo;
    cmd->GRdbccOffHi = bcc_input_offset->bpc_4_offset_gr_hi;
    cmd->GBdbccOffLo = bcc_input_offset->bpc_4_offset_gb_lo;
    cmd->GBdbccOffHi = bcc_input_offset->bpc_4_offset_gb_hi;

    for (i = 0; i < 512; i++) {
      cmd->abccLUT[i].kernelIdx = 0;
      cmd->abccLUT[i].skip0 = 0;
      cmd->abccLUT[i].skip1 = 0;
      cmd->abccLUT[i].pixelIdx = 0;
    }
    cmd->dbpcEnable = 1;
    cmd->dbccenable = 1;
    cmd->abccEnable = 0;
    cmd->abccLutSel = 1;
  }

  CDBG("%s: Exit\n", __func__);
  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_init */

/*=============================================================================
 * Function:               vfe_demosaic_bpc_deinit
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_demosaic_bpc_deinit(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params)
{
  return VFE_SUCCESS;
} /* vfe_demosaic_bpc_deinit */

/*=============================================================================
 * Function:               vfe_bpc_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_bpc_trigger_enable(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params, int8_t enable)
{
  CDBG("%s: new trigger enable value = %d\n", __func__, enable);
  bpc_ctrl->trigger_enable = enable;

  return VFE_SUCCESS;
} /* vfe_bpc_trigger_enable */

/*===========================================================================
 * FUNCTION    - vfe_bpc_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bpc_reload_params(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params)
{
  CDBG("%s: reload the chromatix\n", __func__);
  bpc_ctrl->reload_params = TRUE;

  return VFE_SUCCESS;
} /* vfe_bpc_reload_params */
