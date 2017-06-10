/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "vfe.h"
#include "gamma.h"
#include "rolloff.h"
#include "black_level_v1.h"
#include "vfe_tgtcommon.h"

/*===========================================================================
 * FUNCTION    - ez_vfe_operation_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void ez_vfe_set_operation_enable(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfemodule_t item, int32_t enable)
{
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);
  vfe_params_t *params = &(vfe_ctrl_obj->vfe_params);
  int mod_id = 0;

  switch(item) {
    case VFE_MODULE_COLORCORRECTION:
      diagnostics->control_colorcorr.enable = enable;
      vfe_color_correct_enable(mod_id, &(vfe_module->color_proc_mod.cc_mod),
        params, enable, TRUE);
      break;
    case VFE_MODULE_COLORCONVERSION:
      diagnostics->control_colorconv.enable = enable;
      ez_vfe_color_conversion_enable(&(vfe_module->color_proc_mod.cv_mod),
        params, enable, TRUE);
      break;
    case VFE_MODULE_GAMMA:
      diagnostics->control_gamma.enable = enable;
      ez_vfe_gamma_enable(&(vfe_module->gamma_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_BLACKLEVEL:
      diagnostics->control_blacklevel.enable = enable;
      ez_vfe_black_level_enable(&(vfe_module->linear_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_ASF5X5:
      diagnostics->control_asfsharp.enable = enable;
      ez_vfe_asf_enable( &(vfe_module->asf_mod), &(vfe_ctrl_obj->vfe_params),
        enable, TRUE);
      break;
    case VFE_MODULE_ROLLOFF:
      diagnostics->control_rolloff.enable = enable;
      ez_vfe_rolloff_enable(&(vfe_module->rolloff_mod), &(vfe_ctrl_obj->vfe_params),
        enable, TRUE);
      break;
    case VFE_MODULE_BCC:
    case VFE_MODULE_BPC:
      diagnostics->control_bpc.enable = enable;
      vfe_demosaic_bpc_enable(&(vfe_module->bpc_mod),
        &(vfe_ctrl_obj->vfe_params), enable, TRUE);
      break;
    case VFE_MODULE_ABF:
      vfe_abf_enable(mod_id, &(vfe_module->abf_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_DEMUX:
      diagnostics->control_demux.enable = enable;
      vfe_demux_enable(mod_id, &(vfe_module->demux_mod), params, enable, TRUE);
      break;
    default:
      CDBG_ERROR("%s: invalid item %d\n", __func__, item);
      break;
  }
}

/*===========================================================================
 * Function:           ez_vfe_asf_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t ez_vfe_asf_enable(asf_mod_t* asf_ctrl, vfe_params_t* vfe_params,
  int8_t enable, int8_t hw_write)
{
  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;

  CDBG("%s: enable=%d, hw_write=%d asf_enable %d\n", __func__,
    enable, hw_write, asf_ctrl->asf_enable);
  vfe_params->moduleCfg->asfEnable = enable;

  if (hw_write && (asf_ctrl->asf_enable == enable))
    return VFE_SUCCESS;

  asf_ctrl->asf_enable = enable;
  asf_ctrl->hw_enable_cmd = hw_write;
  asf_ctrl->vfe_reconfig = 1;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_ASF)
      : (vfe_params->current_config & ~VFE_MOD_ASF);
  }

  return VFE_SUCCESS;
} /* ez_vfe_asf_enable */

/*===========================================================================
 * FUNCTION    - ez_vfe_black_level_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t ez_vfe_black_level_enable(black_level_mod_t* mod,
  vfe_params_t* params, int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  params->moduleCfg->blackLevelCorrectionEnable = enable;

  if (hw_write && (mod->enable == enable))
    return VFE_SUCCESS;

  mod->enable = enable;
  mod->hw_enable = hw_write;
  mod->vfe_reconfig = 1;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_LINEARIZATION)
      : (params->current_config & ~VFE_MOD_LINEARIZATION);
  }
  return status;
}/*ez_vfe_black_level_enable*/

/*===========================================================================
 * Function:           ez_vfe_gamma_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t ez_vfe_gamma_enable(gamma_mod_t* mod,
  vfe_params_t *params, int8_t g_enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  if (!IS_BAYER_FORMAT(params))
    g_enable = FALSE;
  params->moduleCfg->rgbLUTEnable = g_enable;
  if (hw_write && (mod->enable == g_enable))
    return VFE_SUCCESS;

  mod->vfe_reconfig = 1;
  mod->enable = g_enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (g_enable) ?
      (params->current_config | VFE_MOD_GAMMA)
      : (params->current_config & ~VFE_MOD_GAMMA);
  }

  return status;
} /* vfe_gamma_set */

/*===========================================================================
 * Function:           vfe_rolloff_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t ez_vfe_rolloff_enable(rolloff_mod_t* rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable, int8_t hw_write)
{
  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;
  vfe_params->moduleCfg->lensRollOffEnable = enable;

  CDBG("%s: enable=%d, hw_write=%d\n", __func__, enable, hw_write);

  if (hw_write && (rolloff_ctrl->rolloff_enable == enable))
    return VFE_SUCCESS;

  rolloff_ctrl->rolloff_enable = enable;
  rolloff_ctrl->vfe_reconfig = 1;

  if (hw_write) {
    rolloff_ctrl->hw_enable_cmd = TRUE;
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_ROLLOFF)
      : (vfe_params->current_config & ~VFE_MOD_ROLLOFF);
  }

  return VFE_SUCCESS;
} /* vfe_rolloff_enable */

/*===========================================================================
 * FUNCTION.   - ez_vfe_color_conversion_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t ez_vfe_color_conversion_enable(chroma_enhan_mod_t *mod,
  vfe_params_t *params, int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;

  CDBG("%s, enable/disable CV module = %d",__func__, enable);
  params->moduleCfg->chromaEnhanEnable = enable;

  if (hw_write && (mod->cv_enable == enable))
    return VFE_SUCCESS;

  mod->cv_enable = enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_COLOR_CONV)
      : (params->current_config & ~VFE_MOD_COLOR_CONV);
  }

  if(!enable) {
   mod->chroma_enhan_cmd.am  = FLOAT_TO_Q(8, 0.5000);
   mod->chroma_enhan_cmd.ap  = FLOAT_TO_Q(8, 0.5000);
   mod->chroma_enhan_cmd.bm  = FLOAT_TO_Q(8, -0.3380);
   mod->chroma_enhan_cmd.bp  = FLOAT_TO_Q(8, -0.3380);
   mod->chroma_enhan_cmd.cm  = FLOAT_TO_Q(8, 0.5000);
   mod->chroma_enhan_cmd.cp  = FLOAT_TO_Q(8, 0.5000);
   mod->chroma_enhan_cmd.dm  = FLOAT_TO_Q(8, -0.1620);
   mod->chroma_enhan_cmd.dp  = FLOAT_TO_Q(8, -0.1620);

   mod->chroma_enhan_cmd.kcb  = 128;
   mod->chroma_enhan_cmd.kcr  = 128;

   mod->chroma_enhan_cmd.RGBtoYConversionV0 = FLOAT_TO_Q(8, 0.2990);
   mod->chroma_enhan_cmd.RGBtoYConversionV1 = FLOAT_TO_Q(8, 0.5870);
   mod->chroma_enhan_cmd.RGBtoYConversionV2 = FLOAT_TO_Q(8, 0.1140);

   mod->chroma_enhan_cmd.RGBtoYConversionOffset = 0;

   mod->cv_update = TRUE;
  }

  return status;
} /* vfe_color_conversion_enable */

/*===========================================================================
 * FUNCTION    - ez_vfe_operation_controlenable -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void ez_vfe_set_operation_controlenable(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfemodule_t item, int32_t cntrlenable)
{
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);
  vfe_params_t *params = &(vfe_ctrl_obj->vfe_params);
  int mod_id = 0;

  switch(item) {
    case VFE_MODULE_COLORCORRECTION:
      diagnostics->control_colorcorr.cntrlenable = cntrlenable;
      vfe_color_correct_trigger_enable(mod_id, &vfe_module->color_proc_mod.cc_mod,
        params, cntrlenable);
      break;
    case VFE_MODULE_COLORCONVERSION:
      diagnostics->control_colorconv.cntrlenable = cntrlenable;
      vfe_color_conversion_trigger_enable(mod_id, &vfe_module->color_proc_mod.cv_mod,
        params, cntrlenable);
      break;
    case VFE_MODULE_GAMMA:
      diagnostics->control_gamma.cntrlenable = cntrlenable;
      vfe_gamma_trigger_enable(mod_id, &vfe_module->gamma_mod, params, cntrlenable);
      break;
    case VFE_MODULE_BLACKLEVEL:
      diagnostics->control_blacklevel.cntrlenable = cntrlenable;
      //triggers->ez_blacklevel_update = enable;
      break;
    case VFE_MODULE_ASF5X5:
      diagnostics->control_asfsharp.cntrlenable = cntrlenable;
      vfe_asf_trigger_enable(mod_id, &(vfe_module->asf_mod),
        &(vfe_ctrl_obj->vfe_params), cntrlenable);
      break;
    case VFE_MODULE_ROLLOFF:
      diagnostics->control_rolloff.cntrlenable = cntrlenable;
      vfe_rolloff_trigger_enable(mod_id, &(vfe_module->rolloff_mod),
        &(vfe_ctrl_obj->vfe_params), cntrlenable);
      break;
    case VFE_MODULE_BPC:
      diagnostics->control_bpc.cntrlenable = cntrlenable;
      vfe_bpc_trigger_enable(&(vfe_module->bpc_mod),
        &(vfe_ctrl_obj->vfe_params), cntrlenable);
      break;
#ifdef VFE32 /*Need to confirm presence of bcc_mod in vfe2x structure*/
    case VFE_MODULE_BCC:
      diagnostics->control_bcc.cntrlenable = cntrlenable;
      vfe_bcc_trigger_enable(&(vfe_module->bcc_mod),
        &(vfe_ctrl_obj->vfe_params), cntrlenable);
      break;
#endif
    case VFE_MODULE_ABF:
      diagnostics->control_abfilter.cntrlenable = cntrlenable;
      vfe_abf_trigger_enable(mod_id, &vfe_module->abf_mod, params, cntrlenable);
      break;
    case VFE_MODULE_DEMUX:
      diagnostics->control_demux.cntrlenable = cntrlenable;
      vfe_demux_trigger_enable(mod_id, &vfe_module->demux_mod, params, cntrlenable);
      break;
    default:
      CDBG_ERROR("%s: invalid item %d\n", __func__, item);
      break;
  }
}

/*===========================================================================
 * FUNCTION    - ez_vfe_update_colorcorrection -
 *
 * DESCRIPTION: updates color correction coefficients.
 *==========================================================================*/
static void ez_vfe_update_colorcorrection(
  color_correct_mod_t *cc_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_ColorCorrectionCfgCmdType *colorCorrectionCfg;
  colorcorrection_t *colorCorrectionDiag;
  if (mode == PREVIEW) {
    colorCorrectionDiag =
      &(diagnostics->prev_colorcorr);
    colorCorrectionCfg =
      &(cc_module->VFE_PrevColorCorrectionCmd);
  } else {
    colorCorrectionDiag =
      &(diagnostics->snap_colorcorr);
    colorCorrectionCfg =
      &(cc_module->VFE_SnapColorCorrectionCmd);
  }
  colorCorrectionDiag->coef_rtor = colorCorrectionCfg->C8;
  colorCorrectionDiag->coef_gtor = colorCorrectionCfg->C6;
  colorCorrectionDiag->coef_btor = colorCorrectionCfg->C7;
  colorCorrectionDiag->coef_rtog = colorCorrectionCfg->C2;
  colorCorrectionDiag->coef_gtog = colorCorrectionCfg->C0;
  colorCorrectionDiag->coef_btog = colorCorrectionCfg->C1;
  colorCorrectionDiag->coef_rtob = colorCorrectionCfg->C5;
  colorCorrectionDiag->coef_gtob = colorCorrectionCfg->C3;
  colorCorrectionDiag->coef_btob = colorCorrectionCfg->C4;
  colorCorrectionDiag->roffset = colorCorrectionCfg->K0;
  colorCorrectionDiag->goffset = colorCorrectionCfg->K1;
  colorCorrectionDiag->boffset = colorCorrectionCfg->K2;
  colorCorrectionDiag->coef_qfactor = colorCorrectionCfg->coefQFactor;
}

/*===========================================================================
 * FUNCTION    - ez_vfe_update_colorconversion -
 *
 * DESCRIPTION: update chroma enhancement matrix parameters
 *==========================================================================*/
static void ez_vfe_update_colorconversion(chroma_enhan_mod_t *ce_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_Chroma_Enhance_CfgCmdType *colorConversionCfg;
  chromaenhancement_t *colorConversionDiag;
  colorConversionDiag =
    &(diagnostics->colorconv);
  colorConversionCfg =
    &(ce_module->chroma_enhan_cmd);
  colorConversionDiag->param_ap = colorConversionCfg->ap;
  colorConversionDiag->param_am = colorConversionCfg->am;
  colorConversionDiag->param_bp = colorConversionCfg->bp;
  colorConversionDiag->param_bm = colorConversionCfg->bm;
  colorConversionDiag->param_cp = colorConversionCfg->cp;
  colorConversionDiag->param_cm = colorConversionCfg->cm;
  colorConversionDiag->param_dp = colorConversionCfg->dp;
  colorConversionDiag->param_dm = colorConversionCfg->dm;
  colorConversionDiag->param_kcb = colorConversionCfg->kcb;
  colorConversionDiag->param_kcr = colorConversionCfg->kcr;
  colorConversionDiag->param_rtoy = colorConversionCfg->RGBtoYConversionV0;
  colorConversionDiag->param_gtoy = colorConversionCfg->RGBtoYConversionV1;
  colorConversionDiag->param_btoy = colorConversionCfg->RGBtoYConversionV2;
  colorConversionDiag->param_yoffset =
    colorConversionCfg->RGBtoYConversionOffset;
}

#if 1
/*===========================================================================
 * FUNCTION    - ez_vfe_update_blacklevel -
 *
 * DESCRIPTION: update  the blacklevel correction values
 *==========================================================================*/
void ez_vfe_update_blacklevel(black_level_mod_t *bl_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  int32_t rc = INT32_MAX;
  VFE_BlackLevelConfigCmdType *blacklevelCfg;
  blacklevelcorrection_t *blacklevelDiag;
  if (mode == PREVIEW) {
    blacklevelCfg =
      &(bl_module->preview_cmd);
    blacklevelDiag =
      &(diagnostics->prev_blacklevel);
  } else {
    blacklevelCfg =
      &(bl_module->snapshot_cmd);
    blacklevelDiag =
      &(diagnostics->snap_blacklevel);
  }

  blacklevelDiag->evenRevenC = blacklevelCfg->evenColManualBlackCorrection;
  blacklevelDiag->evenRoddC = blacklevelCfg->evenColManualBlackCorrection;
  blacklevelDiag->oddRevenC = blacklevelCfg->oddColManualBlackCorrection;
  blacklevelDiag->oddRoddC = blacklevelCfg->oddColManualBlackCorrection;
}
#endif

/*===========================================================================
 * FUNCTION    - ez_vfe_update_rolloff -
 *
 * DESCRIPTION: update pca rolloff  table
 *==========================================================================*/
static void ez_vfe_update_rolloff(rolloff_mod_t *rolloff_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  pca_rolloff_params_t *rolloffCfg;
  rolloff_t *rolloffDiag;
  int row, col;

  if (mode == PREVIEW) {
    rolloffDiag =
      &(diagnostics->prev_rolloff);
    rolloffCfg =
      &(rolloff_module->pca_ctrl.pca_rolloff_prev_param);
  } else {
    rolloffDiag =
      &(diagnostics->snap_rolloff);
    rolloffCfg =
      &(rolloff_module->pca_ctrl.pca_rolloff_snap_param);
  }
  for (row = 0; row < ROLLOFF_NUM_ROWS; row++)
    for (col = 0; col < ROLLOFF_NUM_BASE; col++) {
      rolloffDiag->coefftable_R[row][col] =
        rolloffCfg->left_input_table.coeff_table_R[row][col];
      rolloffDiag->coefftable_Gr[row][col] =
        rolloffCfg->left_input_table.coeff_table_Gr[row][col];
      rolloffDiag->coefftable_Gb[row][col] =
        rolloffCfg->left_input_table.coeff_table_Gb[row][col];
      rolloffDiag->coefftable_B[row][col] =
        rolloffCfg->left_input_table.coeff_table_B[row][col];
    }
  for (row = 0; row < ROLLOFF_NUM_BASE; row++)
    for (col = 0; col < ROLLOFF_NUM_COLS; col++)
      rolloffDiag->basistable[row][col] =
        rolloffCfg->left_input_table.PCA_basis_table[row][col];

}


/*===========================================================================
 * FUNCTION    - eztune_vfe_get_asf -
 *
 * DESCRIPTION: update asf sharpness parameters
 *==========================================================================*/
static void ez_vfe_update_asf(asf_mod_t *asf_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_AdaptiveFilterConfigCmdType *asfCfg;
  asfsharpness_t *asfDiag;

  if (mode == PREVIEW) {
    asfDiag = &(diagnostics->prev_asfsharp);
    asfCfg = &(asf_module->asf_prev_cmd);
  } else {
    asfDiag = &(diagnostics->snap_asfsharp);
    asfCfg = &(asf_module->asf_snap_cmd);
  }
  asfDiag->smoothfilterEnabled = asfCfg->smoothFilterEnabled;
  asfDiag->sharpMode = asfCfg->sharpMode;
  asfDiag->smoothcoefCenter = asfCfg->smoothCoefCenter;
  asfDiag->smoothcoefSurr = asfCfg->smoothCoefSurr;
  asfDiag->normalizeFactor = asfCfg->normalizeFactor;
  asfDiag->sharpthreshE1 = asfCfg->sharpThreshE1;
  asfDiag->sharpthreshE2 = asfCfg->sharpThreshE2;
  asfDiag->sharpthreshE3 = asfCfg->sharpThreshE3;
  asfDiag->sharpthreshE4 = asfCfg->sharpThreshE4;
  asfDiag->sharpthreshE5 = asfCfg->sharpThreshE5;
  asfDiag->sharpK1 = asfCfg->sharpK1;
  asfDiag->sharpK2 = asfCfg->sharpK2;
  asfDiag->f1coef0 = asfCfg->F1Coeff0;
  asfDiag->f1coef1 = asfCfg->F1Coeff1;
  asfDiag->f1coef2 = asfCfg->F1Coeff2;
  asfDiag->f1coef3 = asfCfg->F1Coeff3;
  asfDiag->f1coef4 = asfCfg->F1Coeff4;
  asfDiag->f1coef5 = asfCfg->F1Coeff5;
  asfDiag->f1coef6 = asfCfg->F1Coeff6;
  asfDiag->f1coef7 = asfCfg->F1Coeff7;
  asfDiag->f1coef8 = asfCfg->F1Coeff8;
  asfDiag->f2coef0 = asfCfg->F2Coeff0;
  asfDiag->f2coef1 = asfCfg->F2Coeff1;
  asfDiag->f2coef2 = asfCfg->F2Coeff2;
  asfDiag->f2coef3 = asfCfg->F2Coeff3;
  asfDiag->f2coef4 = asfCfg->F2Coeff4;
  asfDiag->f2coef5 = asfCfg->F2Coeff5;
  asfDiag->f2coef6 = asfCfg->F2Coeff6;
  asfDiag->f2coef7 = asfCfg->F2Coeff7;
  asfDiag->f2coef8 = asfCfg->F2Coeff8;
}


/*===========================================================================
 * FUNCTION    - ez_vfe_update_bcc -
 *
 * DESCRIPTION: update bpc parameters
 *==========================================================================*/
static void ez_vfe_update_bcc(
  bpc_mod_t *bcc_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_DemosaicDBPC_CmdType *bccCfg;
  badcorrection_t *bccDiag;

  if (mode == PREVIEW) {
    bccDiag = &(diagnostics->prev_bcc);
    bccCfg = &(bcc_module->bpc_prev_cmd);
  } else {
    bccDiag = &(diagnostics->snap_bcc);
    bccCfg = &(bcc_module->bpc_snap_cmd);
  }
  bccDiag->fminThreshold = bccCfg->dbpcFmin;
  bccDiag->fmaxThreshold = bccCfg->dbpcFmax;
  bccDiag->gbOffsetLo = bccCfg->GBdbpcOffLo;
  bccDiag->gbOffsetHi = bccCfg->GBdbpcOffHi;
  bccDiag->grOffsetLo = bccCfg->GRdbpcOffLo;
  bccDiag->grOffsetHi = bccCfg->GRdbpcOffHi;
  bccDiag->rOffsetLo = bccCfg->RdbpcOffLo;
  bccDiag->rOffsetHi = bccCfg->RdbpcOffHi;
  bccDiag->bOffsetLo = bccCfg->BdbpcOffLo;
  bccDiag->bOffsetHi = bccCfg->BdbpcOffHi;
}

/*===========================================================================
 * FUNCTION    - ez_vfe_update_bpc -
 *
 * DESCRIPTION: update bpc parameters
 *==========================================================================*/
static void ez_vfe_update_bpc(
  bpc_mod_t *bpc_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_DemosaicDBPC_CmdType *bpcCfg;
  badcorrection_t *bpcDiag;

  if (mode == PREVIEW) {
    bpcDiag = &(diagnostics->prev_bpc);
    bpcCfg = &(bpc_module->bpc_prev_cmd);
  } else {
    bpcDiag = &(diagnostics->snap_bpc);
    bpcCfg = &(bpc_module->bpc_snap_cmd);
  }

  bpcDiag->fminThreshold = bpcCfg->dbpcFmin;
  bpcDiag->fmaxThreshold = bpcCfg->dbpcFmax;
  bpcDiag->gbOffsetLo = bpcCfg->GBdbpcOffLo;
  bpcDiag->gbOffsetHi = bpcCfg->GBdbpcOffHi;
  bpcDiag->grOffsetLo = bpcCfg->GRdbpcOffLo;
  bpcDiag->grOffsetHi = bpcCfg->GRdbpcOffHi;
  bpcDiag->rOffsetLo = bpcCfg->RdbpcOffLo;
  bpcDiag->rOffsetHi = bpcCfg->RdbpcOffHi;
  bpcDiag->bOffsetLo = bpcCfg->BdbpcOffLo;
  bpcDiag->bOffsetHi = bpcCfg->BdbpcOffHi;
}


/*===========================================================================
 * FUNCTION    - eztune_vfe_update_abf -
 *
 * DESCRIPTION: update ABF parameters
 *==========================================================================*/
static void ez_vfe_update_abf(
  abf_mod_t *abf_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  abf2_parms_t *abfCfg;
  abffilterdata_t *abfDiag;
  int idx;

  if (mode == PREVIEW) {
    abfDiag = &(diagnostics->prev_abfilter);
    abfCfg = &(abf_module->abf2_parms);
  } else {
    abfDiag = &(diagnostics->snap_abfilter);
    abfCfg = &(abf_module->abf2_parms);
  }
  for(idx = 0; idx < 3; idx++) {
    abfDiag->red.threshold[idx] = abfCfg->data.threshold_red[idx];
    abfDiag->green.gr.threshold[idx] = abfCfg->data.threshold_green[idx];
    abfDiag->blue.threshold[idx] = abfCfg->data.threshold_blue[idx];
  }
  for(idx = 0; idx < 2; idx++) {
    abfDiag->red.scalefactor[idx] = abfCfg->data.scale_factor_red[idx];
    abfDiag->green.gr.scalefactor[idx] = abfCfg->data.scale_factor_green[idx];
    abfDiag->blue.scalefactor[idx] = abfCfg->data.scale_factor_blue[idx];
    abfDiag->green.a[idx] = abfCfg->data.a[idx];

  }
  for(idx = 0; idx < 8; idx++) {
    abfDiag->red.pos[idx] = abfCfg->r_table.table_pos[idx];
    abfDiag->red.pos[idx + 8] = abfCfg->r_table.table_pos[idx + 8];
    abfDiag->red.neg[idx] = abfCfg->r_table.table_neg[idx];

    abfDiag->blue.pos[idx] = abfCfg->b_table.table_pos[idx];
    abfDiag->blue.pos[idx + 8] = abfCfg->b_table.table_pos[idx + 8];
    abfDiag->blue.neg[idx] = abfCfg->b_table.table_neg[idx];

    abfDiag->green.gr.pos[idx] = abfCfg->g_table.table_pos[idx];
    abfDiag->green.gr.pos[idx + 8] = abfCfg->g_table.table_pos[idx + 8];
    abfDiag->green.gr.neg[idx] = abfCfg->g_table.table_neg[idx];

  }

}


/*===========================================================================
 * FUNCTION    - eztune_vfe_update_demosaic -
 *
 * DESCRIPTION: update demosaic parameters
 *==========================================================================*/
static void ez_vfe_update_demosaic(
  demosaic_mod_t *demosaic_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_DemosaicV3ConfigCmdType *demosaicCfg;
  demosaic3_t *demosaicDiag;
  int index;

  if (mode == PREVIEW) {
    demosaicDiag = &(diagnostics->prev_demosaic);
    demosaicCfg = &(demosaic_module->demosaic_cmd);
  } else {
    demosaicDiag = &(diagnostics->snap_demosaic);
    demosaicCfg = &(demosaic_module->demosaic_cmd);
  }
  demosaicDiag->aG = demosaicCfg->a;
  demosaicDiag->bL = demosaicCfg->bl;

  for(index = 0; index < 18; index++) {
    demosaicDiag->lut[index].bk = demosaicCfg->interpClassifier[index].b_n;
    demosaicDiag->lut[index].wk = demosaicCfg->interpClassifier[index].w_n;
    demosaicDiag->lut[index].lk = demosaicCfg->interpClassifier[index].l_n;
    demosaicDiag->lut[index].tk = demosaicCfg->interpClassifier[index].t_n;
  }

}

/*===========================================================================
 * FUNCTION    - eztune_vfe_update_demuxchannelgain -
 *
 * DESCRIPTION: update demuxchannelgain
 *==========================================================================*/
static void ez_vfe_update_demuxchannelgain(
  demux_mod_t *demux_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_DemuxConfigCmdType *demuxchgainCfg;
  demuxchannelgain_t *demuxchgainDiag;

  if (mode == PREVIEW) {
    demuxchgainDiag = &(diagnostics->prev_demuxchannelgain);
    demuxchgainCfg = &(demux_module->VFE_PreviewDemuxConfigCmd);
  } else {
    demuxchgainDiag = &(diagnostics->snap_demuxchannelgain);
    demuxchgainCfg = &(demux_module->VFE_SnapDemuxConfigCmd);
  }
  demuxchgainDiag->greenEvenRow = demuxchgainCfg->ch0EvenGain;
  demuxchgainDiag->greenOddRow = demuxchgainCfg->ch0OddGain;
  demuxchgainDiag->blue = demuxchgainCfg->ch1Gain;
  demuxchgainDiag->red = demuxchgainCfg->ch2Gain;
}


/*===========================================================================
 * FUNCTION    - ez_vfe_diagnostics -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ez_vfe_diagnostics(void *ctrl_obj)
{
  vfe_ctrl_info_t *vfe_ctrl_obj = (vfe_ctrl_info_t *)ctrl_obj;
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  vfe_params_t *params = &(vfe_ctrl_obj->vfe_params);
   opmode_t opmode;
  vfe_op_mode_t vfe_mode = (vfe_op_mode_t)(params->vfe_op_mode);
  if (vfe_mode == VFE_OP_MODE_VIDEO)
    opmode = PREVIEW;
  else if (vfe_mode == VFE_OP_MODE_SNAPSHOT)
    opmode = SNAPSHOT;
  else
    return;

    ez_vfe_update_blacklevel(&(vfe_module->linear_mod),
      diagnostics, opmode);
    ez_vfe_update_colorconversion(&(vfe_module->color_proc_mod.cv_mod),
      diagnostics, opmode);
    ez_vfe_update_demosaic(&(vfe_module->demosaic_mod),
      diagnostics, opmode);
    ez_vfe_update_bpc(&(vfe_module->bpc_mod),
      diagnostics, opmode);
    ez_vfe_update_bcc(&(vfe_module->bpc_mod),
      diagnostics, opmode);
    ez_vfe_update_asf(&(vfe_module->asf_mod),
      diagnostics, opmode);
    ez_vfe_update_demuxchannelgain(&(vfe_module->demux_mod),
      diagnostics, opmode);
    ez_vfe_update_colorcorrection(&(vfe_module->color_proc_mod.cc_mod),
      diagnostics, opmode);
    ez_vfe_update_abf(&(vfe_module->abf_mod),
      diagnostics, opmode);
    ez_vfe_update_rolloff(&(vfe_module->rolloff_mod),
      diagnostics, opmode);
}

/*===========================================================================
 * FUNCTION    - ez_vfe_diagnostics_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ez_vfe_diagnostics_update(void *ctrl_obj)
{
  vfe_ctrl_info_t *vfe_ctrl_obj = (vfe_ctrl_info_t *)ctrl_obj;
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  vfe_params_t *params = &(vfe_ctrl_obj->vfe_params);
  opmode_t opmode;
  vfe_op_mode_t vfe_mode = (vfe_op_mode_t)(params->vfe_op_mode);
  if (vfe_mode == VFE_OP_MODE_VIDEO)
    opmode = PREVIEW;
  else if (vfe_mode == VFE_OP_MODE_SNAPSHOT)
    opmode = SNAPSHOT;
  else
    return;
  if ((params->update & VFE_MOD_LINEARIZATION))
    ez_vfe_update_blacklevel(&(vfe_module->linear_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_COLOR_CONV))
    ez_vfe_update_colorconversion(&(vfe_module->color_proc_mod.cv_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_DEMOSAIC))
    ez_vfe_update_demosaic(&(vfe_module->demosaic_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_BPC)) {
    ez_vfe_update_bpc(&(vfe_module->bpc_mod),
      diagnostics, opmode);

    ez_vfe_update_bcc(&(vfe_module->bpc_mod),
      diagnostics, opmode);
   }

  if ((params->update & VFE_MOD_ASF))
    ez_vfe_update_asf(&(vfe_module->asf_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_DEMUX))
    ez_vfe_update_demuxchannelgain(&(vfe_module->demux_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_COLOR_PROC)) {
    ez_vfe_update_colorcorrection(&(vfe_module->color_proc_mod.cc_mod),
      diagnostics, opmode);
  }

  if ((params->update & VFE_MOD_ABF))
    ez_vfe_update_abf(&(vfe_module->abf_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_ROLLOFF))
    ez_vfe_update_rolloff(&(vfe_module->rolloff_mod),
      diagnostics, opmode);

}

/*===========================================================================
 * FUNCTION    - ez_vfe_get_moduleconfig -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void ez_vfe_get_moduleconfig(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  opmode_t opmode = VFE_OP_MODE_VIDEO;
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  VFE_ModuleCfgPacked *vfe_mcfg =
    &(vfe_ctrl_obj->vfe_op_cfg.moduleCfg);
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);

  diagnostics->control_linear.enable =
    vfe_mcfg->blackLevelCorrectionEnable;
  diagnostics->control_colorcorr.enable =
    vfe_mcfg->colorCorrectionEnable;
  diagnostics->control_colorconv.enable = vfe_mcfg->chromaEnhanEnable;
  diagnostics->control_gamma.enable = vfe_mcfg->rgbLUTEnable;
  diagnostics->control_blacklevel.enable =
    vfe_mcfg->blackLevelCorrectionEnable;
  diagnostics->control_asfsharp.enable = vfe_mcfg->asfEnable;
  diagnostics->control_rolloff.enable = vfe_mcfg->lensRollOffEnable;
#ifdef VFE32
  diagnostics->control_bcc.enable = vfe_module->bcc_mod.bcc_enable;
#endif
  diagnostics->control_bpc.enable = vfe_module->bpc_mod.enable;
  diagnostics->control_abfilter.enable = vfe_module->abf_mod.enable;
  diagnostics->control_demux.enable =
    vfe_mcfg->demuxEnable;
  diagnostics->control_colorcorr.cntrlenable =
    vfe_module->color_proc_mod.cc_mod.trigger_enable;
  diagnostics->control_colorconv.cntrlenable =
    vfe_module->color_proc_mod.cv_mod.trigger_enable;
  diagnostics->control_gamma.cntrlenable =
    vfe_module->gamma_mod.trigger_enable;;
  diagnostics->control_blacklevel.cntrlenable =
    vfe_mcfg->blackLevelCorrectionEnable;
  diagnostics->control_asfsharp.cntrlenable =
    vfe_module->asf_mod.asf_trigger_enable;
  if (vfe_ctrl_obj->vfe_params.vfe_version == MSM8960V2)
    diagnostics->control_rolloff.cntrlenable =
      vfe_module->rolloff_mod.pca_ctrl.pca_rolloff_trigger_enable;
  else
    diagnostics->control_rolloff.cntrlenable =
      vfe_module->rolloff_mod.mesh_ctrl.mesh_rolloff_trigger_enable;
#ifdef VFE32
  diagnostics->control_bcc.cntrlenable =
    vfe_module->bcc_mod.bcc_trigger_enable;
#endif
  diagnostics->control_bpc.cntrlenable =
    vfe_module->bpc_mod.trigger_enable;
  diagnostics->control_abfilter.cntrlenable =
    vfe_module->abf_mod.trigger_enable;
  diagnostics->control_demux.cntrlenable =
    vfe_module->demux_mod.trigger_enable;
  ez_vfe_diagnostics((void *)vfe_ctrl_obj);
}

/*===========================================================================
 * FUNCTION    - ez_vfe_set_status -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void ez_vfe_set_status(vfe_ctrl_info_t *vfe_ctrl_obj, int32_t enable)
{
  vfe_ctrl_obj->vfe_params.eztune_status = enable;
  if (enable)
    ez_vfe_get_moduleconfig(vfe_ctrl_obj);
}

/*===========================================================================
 * FUNCTION    - ez_vfe_set_reload_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void ez_vfe_set_reload_chromatix(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  vfe_params_t *params = &(vfe_ctrl_obj->vfe_params);
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;

  vfe_rolloff_reload_params(mod_id, &vfe_module->rolloff_mod,
    &vfe_ctrl_obj->vfe_params);
  vfe_bpc_reload_params(&vfe_module->bpc_mod, &vfe_ctrl_obj->vfe_params);
#ifdef VFE32
  vfe_bcc_reload_params(mod_id, &vfe_module->bcc_mod, &vfe_ctrl_obj->vfe_params);
#endif
  vfe_asf_reload_params(mod_id, &vfe_module->asf_mod, &vfe_ctrl_obj->vfe_params);

  status = vfe_color_correct_reload_params(mod_id, &vfe_module->color_proc_mod.cc_mod,
    params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_color_correct_reload_params %d", __func__, status);
    return;
  }

  status = vfe_color_conversion_reload_params(mod_id, &vfe_module->color_proc_mod.cv_mod,
    params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_color_conversion_reload_params %d", __func__, status);
    return;
  }

  status = vfe_gamma_reload_params(mod_id, &vfe_module->gamma_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_gamma_reload_params %d", __func__, status);
    return;
  }

  status = vfe_abf_reload_params(mod_id, &vfe_module->abf_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_abf2_reload_params %d", __func__, status);
    return;
  }

  status = vfe_demosaic_reload_params(mod_id, &vfe_module->demosaic_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_demosaic_reload_params %d", __func__, status);
    return;
  }

  status = vfe_demux_reload_params(mod_id, &vfe_module->demux_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_demosaic_reload_params %d", __func__, status);
    return;
  }
}


/*===========================================================================
 * FUNCTION    - ez_vfe_set -
 *
 * DESCRIPTION:  set  the vfe modules.
 *==========================================================================*/
vfe_status_t ez_vfe_set(
  void *ctrl_obj, void *param1, void *param2)
{
  ez_vfecmd_t *cmd = (ez_vfecmd_t *)param1;
  int32_t *value = (int32_t *)param2;
  vfe_ctrl_info_t *vfe_ctrl_obj = (vfe_ctrl_info_t *)ctrl_obj;
  if(!vfe_ctrl_obj)
    return VFE_ERROR_GENERAL;

  if (cmd) {
    switch (cmd->type) {
      case SET_ENABLE:
        ez_vfe_set_operation_enable(
          vfe_ctrl_obj, cmd->module, *value);
        break;
      case SET_CONTROLENABLE:
        ez_vfe_set_operation_controlenable(
          vfe_ctrl_obj, cmd->module, *value);
        break;
      case SET_STATUS:
        ez_vfe_set_status(vfe_ctrl_obj, *value);
        break;
      case SET_RELOAD_CHROMATIX:
        ez_vfe_set_reload_chromatix(vfe_ctrl_obj);
        break;
    }
  }
  return VFE_SUCCESS;
}
