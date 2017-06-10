/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "vfe.h"

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
  int module_id = 0;

  switch(item) {
    case VFE_MODULE_LINEARIZATION:
      diagnostics->control_linear.enable = enable;
      vfe_module->linear_mod.ops.enable(module_id, &(vfe_module->linear_mod),
        params, enable, TRUE);
      break;
    case VFE_MODULE_COLORCORRECTION:
      diagnostics->control_colorcorr.enable = enable;
      vfe_module->color_correct_mod.ops.enable(module_id,
        &(vfe_module->color_correct_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_COLORCONVERSION:
      diagnostics->control_colorconv.enable = enable;
      vfe_module->chroma_enhan_mod.ops.enable(module_id,
        &(vfe_module->chroma_enhan_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_GAMMA:
      diagnostics->control_gamma.enable = enable;
      vfe_module->gamma_mod.ops.enable(module_id,
        &(vfe_module->gamma_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_BLACKLEVEL:
      diagnostics->control_blacklevel.enable = enable;
      //vfe_util_blacklevel_enable(enable);
      break;
    /*TODO: no more asf in VFE*/
    /*case VFE_MODULE_ASF5X5:
      diagnostics->control_asfsharp.enable = enable;
      vfe_module->asf_mod.ops.enable(module_id,
        &(vfe_module->asf_mod), params, enable, TRUE);
      break;*/
    case VFE_MODULE_LUMAADAPTATION:
      diagnostics->control_lumaadaptation.enable = enable;
      vfe_module->la_mod.ops.enable(module_id, &(vfe_module->la_mod),
        params, enable, TRUE);
      break;
    case VFE_MODULE_ROLLOFF:
      diagnostics->control_rolloff.enable = enable;
      vfe_module->rolloff_mod.ops.enable(module_id,
        &(vfe_module->rolloff_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_BPC:
      diagnostics->control_bpc.enable = enable;
      vfe_module->bpc_mod.ops.enable(module_id,
        &(vfe_module->bpc_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_BCC:
      diagnostics->control_bcc.enable = enable;
      vfe_module->bcc_mod.ops.enable(module_id,
        &(vfe_module->bcc_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_ABF:
      vfe_module->abf_mod.ops.enable(module_id,
        &(vfe_module->abf_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_CHROMASUPPRESSION:
      diagnostics->control_chromasupp.enable = enable;
      vfe_module->chroma_supp_mod.ops.enable(module_id,
        &(vfe_module->chroma_supp_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_MCE:
      diagnostics->control_memcolorenhan.enable = enable;
      vfe_module->mce_mod.ops.enable(module_id,
        &(vfe_module->mce_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_SCE:
      diagnostics->control_skincolorenhan.enable = enable;
      vfe_module->sce_mod.ops.enable(module_id,
        &(vfe_module->sce_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_DEMUX:
      diagnostics->control_demux.enable = enable;
      vfe_module->demux_mod.ops.enable(module_id,
        &(vfe_module->demux_mod), params, enable, TRUE);
      break;
    case VFE_MODULE_CLFILTER: {
      vfe_clf_enable_type_t clf_enable;
      diagnostics->control_clfilter.enable = enable;
      if(enable)
        clf_enable = VFE_CLF_LUMA_CHROMA_ENABLE;
      else
        clf_enable = VFE_CLF_LUMA_CHROMA_DISABLE;
      vfe_module->clf_mod.ops.enable(module_id,
        &(vfe_module->clf_mod), params, enable, TRUE);
      break;}
    default:
      CDBG_ERROR("%s: invalid item %d\n", __func__, item);
      break;
  }
}

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
  int module_id = 0;

  switch(item) {
    case VFE_MODULE_LINEARIZATION:
      diagnostics->control_linear.cntrlenable = cntrlenable;
      vfe_module->linear_mod.ops.trigger_enable(module_id, &vfe_module->linear_mod,
        params, cntrlenable);
      break;
    case VFE_MODULE_COLORCORRECTION:
      diagnostics->control_colorcorr.cntrlenable = cntrlenable;
      vfe_module->color_correct_mod.ops.trigger_enable(module_id,
       &vfe_module->color_correct_mod, params, cntrlenable);
      break;
    case VFE_MODULE_COLORCONVERSION:
      diagnostics->control_colorconv.cntrlenable = cntrlenable;
      vfe_module->chroma_enhan_mod.ops.trigger_enable(module_id,
       &vfe_module->chroma_enhan_mod, params, cntrlenable);
      break;
    case VFE_MODULE_GAMMA:
      diagnostics->control_gamma.cntrlenable = cntrlenable;
      vfe_module->gamma_mod.ops.trigger_enable(module_id,
       &vfe_module->gamma_mod, params, cntrlenable);
      break;
    case VFE_MODULE_BLACKLEVEL:
      diagnostics->control_blacklevel.cntrlenable = cntrlenable;
      //triggers->ez_blacklevel_update = enable;
      break;
    /*case VFE_MODULE_ASF5X5:
      diagnostics->control_asfsharp.cntrlenable = cntrlenable;
      vfe_module->asf_mod.ops.trigger_enable(module_id,
       &vfe_module->asf_mod, params, cntrlenable);
      break;*/
    case VFE_MODULE_ROLLOFF:
      diagnostics->control_rolloff.cntrlenable = cntrlenable;
      vfe_module->rolloff_mod.ops.trigger_enable(module_id,
       &vfe_module->rolloff_mod, params, cntrlenable);
      break;
    case VFE_MODULE_BPC:
      diagnostics->control_bpc.cntrlenable = cntrlenable;
      vfe_module->bpc_mod.ops.trigger_enable(module_id,
       &vfe_module->bpc_mod, params, cntrlenable);
      break;
    case VFE_MODULE_BCC:
      diagnostics->control_bcc.cntrlenable = cntrlenable;
      vfe_module->bcc_mod.ops.trigger_enable(module_id,
       &vfe_module->bcc_mod, params, cntrlenable);
      break;
    case VFE_MODULE_ABF:
      diagnostics->control_abfilter.cntrlenable = cntrlenable;
      vfe_module->abf_mod.ops.trigger_enable(module_id,
       &vfe_module->abf_mod, params, cntrlenable);
      break;
    case VFE_MODULE_LUMAADAPTATION:
      diagnostics->control_lumaadaptation.cntrlenable = cntrlenable;
      vfe_module->la_mod.ops.trigger_enable(module_id,
       &vfe_module->la_mod, params, cntrlenable);
      break;
    case VFE_MODULE_CHROMASUPPRESSION:
      diagnostics->control_chromasupp.cntrlenable = cntrlenable;
      vfe_module->chroma_supp_mod.ops.trigger_enable(module_id,
       &vfe_module->chroma_supp_mod, params, cntrlenable);
      break;
    case VFE_MODULE_MCE:
      diagnostics->control_memcolorenhan.cntrlenable = cntrlenable;
      vfe_module->mce_mod.ops.trigger_enable(module_id, &vfe_module->mce_mod,
        params, cntrlenable);
      break;
    case VFE_MODULE_SCE:
      diagnostics->control_skincolorenhan.cntrlenable = cntrlenable;
      vfe_module->sce_mod.ops.trigger_enable(module_id, &vfe_module->sce_mod,
        params, cntrlenable);
      break;
    case VFE_MODULE_DEMUX:
      diagnostics->control_demux.cntrlenable = cntrlenable;
      vfe_module->demux_mod.ops.trigger_enable(module_id,
        &vfe_module->demux_mod, params, cntrlenable);
      break;
    case VFE_MODULE_CLFILTER:
      diagnostics->control_clfilter.cntrlenable = cntrlenable;
      vfe_module->clf_mod.ops.trigger_enable(module_id, &vfe_module->clf_mod,
        params, cntrlenable);
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

#if 0
/*===========================================================================
 * FUNCTION    - ez_vfe_update_blacklevel -
 *
 * DESCRIPTION: update  the blacklevel correction values
 *==========================================================================*/
void ez_vfe_update_blacklevel(/*black_level_mod_t *bl_module,*/
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  int32_t rc = INT32_MAX;
  VFE_BlackLevelConfigCmdType *blacklevelCfg;
  blacklevelcorrection_t blacklevelDiag;
  if (mode == PREVIEW) {
    blacklevelDiag =
      &(diagnostics->prev_blacklevel);
//    blacklevelCfg =
//      &(bl_module->chroma_enhan_cmd);
  } else {
    blacklevelDiag =
      &(diagnostics->snap_blacklevel);
//    blacklevelCfg =
//      &(bl_module->chroma_enhan_cmd);
  }

  blacklevelDiag->evenRevenC = blacklevelCfg->evenEvenAdjustment;
  blacklevelDiag->evenRoddC = blacklevelCfg->evenOddAdjustment;
  blacklevelDiag->oddRevenC = blacklevelCfg->oddEvenAdjustment;
  blacklevelDiag->oddRoddC = blacklevelCfg->oddOddAdjustment;
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
/*static void ez_vfe_update_asf(asf_mod_t *asf_module,
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
  asfDiag->lpfMode = asfCfg->lpfMode;
  asfDiag->smoothcoefCenter = asfCfg->smoothCoefCenter;
  asfDiag->smoothcoefSurr = asfCfg->smoothCoefSurr;
  asfDiag->pipeflushCount = asfCfg->pipeFlushCount;
  asfDiag->pipeflushOvd = asfCfg->pipeFlushOvd;
  asfDiag->flushhaltOvd = asfCfg->flushHaltOvd;
  asfDiag->cropEnable = asfCfg->cropEnable;
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
  asfDiag->f3coef0 = asfCfg->F3Coeff0;
  asfDiag->f3coef1 = asfCfg->F3Coeff1;
  asfDiag->f3coef2 = asfCfg->F3Coeff2;
  asfDiag->f3coef3 = asfCfg->F3Coeff3;
  asfDiag->f3coef4 = asfCfg->F3Coeff4;
  asfDiag->f3coef5 = asfCfg->F3Coeff5;
  asfDiag->f3coef6 = asfCfg->F3Coeff6;
  asfDiag->f3coef7 = asfCfg->F3Coeff7;
  asfDiag->f3coef8 = asfCfg->F3Coeff8;
}
*/
/*===========================================================================
 * FUNCTION    - ez_vfe_update_lumaadaptation -
 *
 * DESCRIPTION: update lumaadaptation lut
 *==========================================================================*/
static void ez_vfe_update_lumaadaptation(
  la_mod_t *la_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode){

  la_mod_t *laCfg;
  lumaadaptation_t *laDiag;
  int idx;

  if (mode == PREVIEW) {
    laDiag = &(diagnostics->prev_lumaadaptation);
    laCfg = la_module;
  } else {
    laDiag = &(diagnostics->snap_lumaadaptation);
    laCfg = la_module;
  }
  for (idx = 0; idx < LA_LUT_SIZE; idx++)
    laDiag->lut_yratio[idx] = laCfg->LUT_Yratio[idx];

}

/*===========================================================================
 * FUNCTION    - ez_vfe_update_chromasuppression -
 *
 * DESCRIPTION: update chroma suppression parameters
 *==========================================================================*/
static void ez_vfe_update_chromasuppression(
  chroma_supp_mod_t *cs_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode){

  VFE_ChromaSuppress_ConfigCmdType *chromaSuppressionCfg;
  chromasuppression_t *chromaSuppressionDiag;

  if (mode == PREVIEW) {
    chromaSuppressionDiag =
      &(diagnostics->prev_chromasupp);
    chromaSuppressionCfg =
      &(cs_module->chroma_supp_video_cmd);
  } else {
    chromaSuppressionDiag =
      &(diagnostics->snap_chromasupp);
    chromaSuppressionCfg =
      &(cs_module->chroma_supp_snap_cmd);
  }

  chromaSuppressionDiag->ysup1 = chromaSuppressionCfg->ySup1;
  chromaSuppressionDiag->ysup2 = chromaSuppressionCfg->ySup2;
  chromaSuppressionDiag->ysup3 = chromaSuppressionCfg->ySup3;
  chromaSuppressionDiag->ysup4 = chromaSuppressionCfg->ySup4;
  chromaSuppressionDiag->ysupM1 = chromaSuppressionCfg->ySupM1;
  chromaSuppressionDiag->ysupM3 = chromaSuppressionCfg->ySupM3;
  chromaSuppressionDiag->ysupS1 = chromaSuppressionCfg->ySupS1;
  chromaSuppressionDiag->ysupS3 = chromaSuppressionCfg->ySupS3;
  chromaSuppressionDiag->csup1 = chromaSuppressionCfg->cSup1;
  chromaSuppressionDiag->csup2 = chromaSuppressionCfg->cSup2;
  chromaSuppressionDiag->csupM1 = chromaSuppressionCfg->cSupM1;
  chromaSuppressionDiag->csupS1 = chromaSuppressionCfg->cSupS1;
}

/*===========================================================================
 * FUNCTION    - ez_vfe_update_bcc -
 *
 * DESCRIPTION: update bpc parameters
 *==========================================================================*/
static void ez_vfe_update_bcc(
  bcc_mod_t *bcc_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_DemosaicDBCC_CmdType *bccCfg;
  badcorrection_t *bccDiag;

  if (mode == PREVIEW) {
    bccDiag = &(diagnostics->prev_bcc);
    bccCfg = &(bcc_module->bcc_prev_cmd);
  } else {
    bccDiag = &(diagnostics->snap_bcc);
    bccCfg = &(bcc_module->bcc_snap_cmd);
  }

  bccDiag->fminThreshold = bccCfg->fminThreshold;
  bccDiag->fmaxThreshold = bccCfg->fmaxThreshold;
  bccDiag->gbOffsetLo = bccCfg->gbOffsetLo;
  bccDiag->gbOffsetHi = bccCfg->gbOffsetHi;
  bccDiag->grOffsetLo = bccCfg->grOffsetLo;
  bccDiag->grOffsetHi = bccCfg->grOffsetHi;
  bccDiag->rOffsetLo = bccCfg->rOffsetLo;
  bccDiag->rOffsetHi = bccCfg->rOffsetHi;
  bccDiag->bOffsetLo = bccCfg->bOffsetLo;
  bccDiag->bOffsetHi = bccCfg->bOffsetHi;
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

/*===========================================================================
 * FUNCTION    - eztune_vfe_update_mce -
 *
 * DESCRIPTION: update MCE parameters
 *==========================================================================*/
static void ez_vfe_update_mce(
  mce_mod_t *mce_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_MCE_ConfigCmdType *mceCfg;
  memorycolorenhancement_t *mceDiag;

  if (mode == PREVIEW) {
    mceDiag = &(diagnostics->prev_memcolorenhan);
    mceCfg = &(mce_module->mce_cmd);
  } else {
    mceDiag = &(diagnostics->snap_memcolorenhan);
    mceCfg = &(mce_module->mce_cmd);
  }

  mceDiag->qk = mceCfg->qk;
  mceDiag->blue.y1 = mceCfg->blueCfg.y1;
  mceDiag->blue.y2 = mceCfg->blueCfg.y2;
  mceDiag->blue.y3 = mceCfg->blueCfg.y3;
  mceDiag->blue.y4 = mceCfg->blueCfg.y4;
  mceDiag->blue.yM1 = mceCfg->blueCfg.yM1;
  mceDiag->blue.yM3 = mceCfg->blueCfg.yM3;
  mceDiag->blue.yS1 = mceCfg->blueCfg.yS1;
  mceDiag->blue.yS3 = mceCfg->blueCfg.yS3;
  mceDiag->blue.transWidth = mceCfg->blueCfg.transWidth;
  mceDiag->blue.transTrunc = mceCfg->blueCfg.transTrunc;
  mceDiag->blue.crZone = mceCfg->blueCfg.CRZone;
  mceDiag->blue.cbZone = mceCfg->blueCfg.CBZone;
  mceDiag->blue.translope = mceCfg->blueCfg.transSlope;
  mceDiag->blue.k = mceCfg->blueCfg.K;

  mceDiag->red.y1 = mceCfg->redCfg.y1;
  mceDiag->red.y2 = mceCfg->redCfg.y2;
  mceDiag->red.y3 = mceCfg->redCfg.y3;
  mceDiag->red.y4 = mceCfg->redCfg.y4;
  mceDiag->red.yM1 = mceCfg->redCfg.yM1;
  mceDiag->red.yM3 = mceCfg->redCfg.yM3;
  mceDiag->red.yS1 = mceCfg->redCfg.yS1;
  mceDiag->red.yS3 = mceCfg->redCfg.yS3;
  mceDiag->red.transWidth = mceCfg->redCfg.transWidth;
  mceDiag->red.transTrunc = mceCfg->redCfg.transTrunc;
  mceDiag->red.crZone = mceCfg->redCfg.CRZone;
  mceDiag->red.cbZone = mceCfg->redCfg.CBZone;
  mceDiag->red.translope = mceCfg->redCfg.transSlope;
  mceDiag->red.k = mceCfg->redCfg.K;

  mceDiag->green.y1 = mceCfg->greenCfg.y1;
  mceDiag->green.y2 = mceCfg->greenCfg.y2;
  mceDiag->green.y3 = mceCfg->greenCfg.y3;
  mceDiag->green.y4 = mceCfg->greenCfg.y4;
  mceDiag->green.yM1 = mceCfg->greenCfg.yM1;
  mceDiag->green.yM3 = mceCfg->greenCfg.yM3;
  mceDiag->green.yS1 = mceCfg->greenCfg.yS1;
  mceDiag->green.yS3 = mceCfg->greenCfg.yS3;
  mceDiag->green.transWidth = mceCfg->greenCfg.transWidth;
  mceDiag->green.transTrunc = mceCfg->greenCfg.transTrunc;
  mceDiag->green.crZone = mceCfg->greenCfg.CRZone;
  mceDiag->green.cbZone = mceCfg->greenCfg.CBZone;
  mceDiag->green.translope = mceCfg->greenCfg.transSlope;
  mceDiag->green.k = mceCfg->greenCfg.K;

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
  VFE_DemosaicABF_CmdType *abfCfg;
  abffilterdata_t *abfDiag;
  int idx;

  if (mode == PREVIEW) {
    abfDiag = &(diagnostics->prev_abfilter);
    abfCfg = &(abf_module->VFE_DemosaicABFCfgCmd);
  } else {
    abfDiag = &(diagnostics->snap_abfilter);
    abfCfg = &(abf_module->VFE_SnapshotDemosaicABFCfgCmd);
  }

  abfDiag->red.threshold[0] = abfCfg->rCfg.Cutoff1;
  abfDiag->green.threshold[0] = abfCfg->gCfg.Cutoff1;
  abfDiag->blue.threshold[0] = abfCfg->bCfg.Cutoff1;

  abfDiag->red.threshold[1] = abfCfg->rCfg.Cutoff2;
  abfDiag->green.threshold[1] = abfCfg->gCfg.Cutoff2;
  abfDiag->blue.threshold[1] = abfCfg->bCfg.Cutoff2;

  abfDiag->red.threshold[2] = abfCfg->rCfg.Cutoff3;
  abfDiag->green.threshold[2] = abfCfg->gCfg.Cutoff3;
  abfDiag->blue.threshold[2] = abfCfg->bCfg.Cutoff3;

  for(idx = 0; idx < 8; idx++) {
    abfDiag->red.pos[2*idx] = abfCfg->rPosLut[idx].PostiveLUT0;
    abfDiag->red.pos[2*idx+1] = abfCfg->rPosLut[idx].PostiveLUT1;

    abfDiag->blue.pos[2*idx] = abfCfg->bPosLut[idx].PostiveLUT0;
    abfDiag->blue.pos[2*idx+1] = abfCfg->bPosLut[idx].PostiveLUT1;

    abfDiag->green.pos[2*idx] =  abfCfg->gPosLut[idx].PostiveLUT0;
    abfDiag->green.pos[2*idx+1] =  abfCfg->gPosLut[idx].PostiveLUT1;
  }

  for(idx = 0; idx < 4; idx++) {
    abfDiag->red.neg[2*idx] = abfCfg->rNegLut[idx].NegativeLUT0;
    abfDiag->red.neg[2*idx+1] = abfCfg->rNegLut[idx].NegativeLUT1;

    abfDiag->blue.neg[2*idx] = abfCfg->bNegLut[idx].NegativeLUT0;
    abfDiag->blue.neg[2*idx+1] = abfCfg->bNegLut[idx].NegativeLUT1;

    abfDiag->green.neg[2*idx] =  abfCfg->gNegLut[idx].NegativeLUT0;
    abfDiag->green.neg[2*idx+1] =  abfCfg->gNegLut[idx].NegativeLUT1;
  }
}

/*===========================================================================
 * FUNCTION    - eztune_vfe_update_sce -
 *
 * DESCRIPTION: update SCE parameters
 *==========================================================================*/
static void ez_vfe_update_sce(
  sce_mod_t *sce_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_Skin_enhan_ConfigCmdType *sceCfg;
  skincolorenhancement_t *sceDiag;
  if (mode == PREVIEW) {
    sceDiag = &(diagnostics->prev_skincolorenhan);
    sceCfg = &(sce_module->sce_cmd);
  } else {
    sceDiag = &(diagnostics->snap_skincolorenhan);
    sceCfg = &(sce_module->sce_cmd);
  }

  sceDiag->crcoord.vertex00 = sceCfg->crcoord.vertex00;
  sceDiag->crcoord.vertex01 = sceCfg->crcoord.vertex01;
  sceDiag->crcoord.vertex02 = sceCfg->crcoord.vertex02;
  sceDiag->cbcoord.vertex00 = sceCfg->cbcoord.vertex00;
  sceDiag->cbcoord.vertex01 = sceCfg->cbcoord.vertex01;
  sceDiag->cbcoord.vertex02 = sceCfg->cbcoord.vertex02;
  sceDiag->crcoeff.coef00 = sceCfg->crcoeff.coef00;     /*coef A*/
  sceDiag->crcoeff.coef01 = sceCfg->crcoeff.coef01;     /*coef B*/
  sceDiag->croffset.offset0 = sceCfg->croffset.offset0; /*coef C*/
  sceDiag->croffset.shift0 = sceCfg->croffset.shift0;   /*matrix shift*/
  sceDiag->cbcoeff.coef00 = sceCfg->cbcoeff.coef00;     /*coef D*/
  sceDiag->cbcoeff.coef01 = sceCfg->cbcoeff.coef01;     /*coef E*/
  sceDiag->cboffset.offset0 = sceCfg->cboffset.offset0; /*coef F*/
  sceDiag->cboffset.shift0 = sceCfg->cboffset.shift0;   /*offset shift*/

  sceDiag->crcoord.vertex10 = sceCfg->crcoord.vertex10;
  sceDiag->crcoord.vertex11 = sceCfg->crcoord.vertex11;
  sceDiag->crcoord.vertex12 = sceCfg->crcoord.vertex12;
  sceDiag->cbcoord.vertex10 = sceCfg->cbcoord.vertex10;
  sceDiag->cbcoord.vertex11 = sceCfg->cbcoord.vertex11;
  sceDiag->cbcoord.vertex12 = sceCfg->cbcoord.vertex12;
  sceDiag->crcoeff.coef10 = sceCfg->crcoeff.coef10;     /*coef A*/
  sceDiag->crcoeff.coef11 = sceCfg->crcoeff.coef11;     /*coef B*/
  sceDiag->croffset.offset1 = sceCfg->croffset.offset1; /*coef C*/
  sceDiag->croffset.shift1 = sceCfg->croffset.shift1;   /*matrix shift*/
  sceDiag->cbcoeff.coef10 = sceCfg->cbcoeff.coef10;     /*coef D*/
  sceDiag->cbcoeff.coef11 = sceCfg->cbcoeff.coef11;     /*coef E*/
  sceDiag->cboffset.offset1 = sceCfg->cboffset.offset1; /*coef F*/
  sceDiag->cboffset.shift1 = sceCfg->cboffset.shift1;   /*offset shift*/

  sceDiag->crcoord.vertex20 = sceCfg->crcoord.vertex20;
  sceDiag->crcoord.vertex21 = sceCfg->crcoord.vertex21;
  sceDiag->crcoord.vertex22 = sceCfg->crcoord.vertex22;
  sceDiag->cbcoord.vertex20 = sceCfg->cbcoord.vertex20;
  sceDiag->cbcoord.vertex21 = sceCfg->cbcoord.vertex21;
  sceDiag->cbcoord.vertex22 = sceCfg->cbcoord.vertex22;
  sceDiag->crcoeff.coef20 = sceCfg->crcoeff.coef20;     /*coef A*/
  sceDiag->crcoeff.coef21 = sceCfg->crcoeff.coef21;     /*coef B*/
  sceDiag->croffset.offset2 = sceCfg->croffset.offset2; /*coef C*/
  sceDiag->croffset.shift2 = sceCfg->croffset.shift2;   /*matrix shift*/
  sceDiag->cbcoeff.coef20 = sceCfg->cbcoeff.coef20;     /*coef D*/
  sceDiag->cbcoeff.coef21 = sceCfg->cbcoeff.coef21;     /*coef E*/
  sceDiag->cboffset.offset2 = sceCfg->cboffset.offset2; /*coef F*/
  sceDiag->cboffset.shift2 = sceCfg->cboffset.shift2;   /*offset shift*/

  sceDiag->crcoord.vertex30 = sceCfg->crcoord.vertex30;
  sceDiag->crcoord.vertex31 = sceCfg->crcoord.vertex31;
  sceDiag->crcoord.vertex32 = sceCfg->crcoord.vertex32;
  sceDiag->cbcoord.vertex30 = sceCfg->cbcoord.vertex30;
  sceDiag->cbcoord.vertex31 = sceCfg->cbcoord.vertex31;
  sceDiag->cbcoord.vertex32 = sceCfg->cbcoord.vertex32;
  sceDiag->crcoeff.coef30 = sceCfg->crcoeff.coef30;     /*coef A*/
  sceDiag->crcoeff.coef31 = sceCfg->crcoeff.coef31;     /*coef B*/
  sceDiag->croffset.offset3 = sceCfg->croffset.offset3; /*coef C*/
  sceDiag->croffset.shift3 = sceCfg->croffset.shift3;   /*matrix shift*/
  sceDiag->cbcoeff.coef30 = sceCfg->cbcoeff.coef30;     /*coef D*/
  sceDiag->cbcoeff.coef31 = sceCfg->cbcoeff.coef31;     /*coef E*/
  sceDiag->cboffset.offset3 = sceCfg->cboffset.offset3; /*coef F*/
  sceDiag->cboffset.shift3 = sceCfg->cboffset.shift3;   /*offset shift*/

  sceDiag->crcoord.vertex40 = sceCfg->crcoord.vertex40;
  sceDiag->crcoord.vertex41 = sceCfg->crcoord.vertex41;
  sceDiag->crcoord.vertex42 = sceCfg->crcoord.vertex42;
  sceDiag->cbcoord.vertex40 = sceCfg->cbcoord.vertex40;
  sceDiag->cbcoord.vertex41 = sceCfg->cbcoord.vertex41;
  sceDiag->cbcoord.vertex42 = sceCfg->cbcoord.vertex42;
  sceDiag->crcoeff.coef40 = sceCfg->crcoeff.coef40;     /*coef A*/
  sceDiag->crcoeff.coef41 = sceCfg->crcoeff.coef41;     /*coef B*/
  sceDiag->croffset.offset4 = sceCfg->croffset.offset4; /*coef C*/
  sceDiag->croffset.shift4 = sceCfg->croffset.shift4;   /*matrix shift*/
  sceDiag->cbcoeff.coef40 = sceCfg->cbcoeff.coef40;     /*coef D*/
  sceDiag->cbcoeff.coef41 = sceCfg->cbcoeff.coef41;     /*coef E*/
  sceDiag->cboffset.offset4 = sceCfg->cboffset.offset4; /*coef F*/
  sceDiag->cboffset.shift4 = sceCfg->cboffset.shift4;   /*offset shift*/
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
 * FUNCTION    - eztune_vfe_update_linearization -
 *
 * DESCRIPTION: update linearization lut
 *==========================================================================*/
static void ez_vfe_update_linearization(
  linear_mod_t *clf_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  VFE_LinearizationLut *linCfg;
  linearization_t *linDiag;
  int idx;

  if (mode == PREVIEW) {
    linDiag = &(diagnostics->prev_linear);
    linCfg = &(clf_module->linear_lut);
  } else {
    linDiag = &(diagnostics->snap_linear);
    linCfg = &(clf_module->linear_lut);
  }
  memcpy(linDiag, linCfg, sizeof(linearization_t));
}

/*===========================================================================
 * FUNCTION    - eztune_vfe_update_chromalumafilter -
 *
 * DESCRIPTION: update chromalumafilter
 *==========================================================================*/
static void ez_vfe_update_chromalumafilter(
  clf_mod_t *clf_module,
  vfe_diagnostics_t *diagnostics, opmode_t mode)
{
  clf_params_t *clfCfg;
  chromalumafiltercoeff_t *clfDiag;
  int idx;

  if (mode == PREVIEW) {
    clfDiag = &(diagnostics->prev_chromalumafilter);
    clfCfg = &(clf_module->clf_params);
  } else {
    clfDiag = &(diagnostics->snap_chromalumafilter);
    clfCfg = &(clf_module->clf_params);
  }
  memcpy(&(clfDiag->chromafilter), &(clfCfg->cf_param),
    sizeof(chromafiltercoeff_t));

  memcpy(&(clfDiag->lumafilter), &(clfCfg->lf_param),
    sizeof(lumafiltercoeff_t));
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

    ez_vfe_update_linearization(&(vfe_module->linear_mod),
      diagnostics, opmode);
    ez_vfe_update_mce(&(vfe_module->mce_mod),
      diagnostics, opmode);
    ez_vfe_update_colorconversion(&(vfe_module->chroma_enhan_mod),
      diagnostics, opmode);
    ez_vfe_update_chromasuppression(&(vfe_module->chroma_supp_mod),
      diagnostics, opmode);
    ez_vfe_update_demosaic(&(vfe_module->demosaic_mod),
      diagnostics, opmode);
    ez_vfe_update_bpc(&(vfe_module->bpc_mod),
      diagnostics, opmode);
    ez_vfe_update_bcc(&(vfe_module->bcc_mod),
      diagnostics, opmode);
    /*TODO: no more asf in VFE*/
    /*ez_vfe_update_asf(&(vfe_module->asf_mod),
      diagnostics, opmode);*/
    ez_vfe_update_demuxchannelgain(&(vfe_module->demux_mod),
      diagnostics, opmode);
    ez_vfe_update_colorcorrection(&(vfe_module->color_correct_mod),
      diagnostics, opmode);
    ez_vfe_update_abf(&(vfe_module->abf_mod),
      diagnostics, opmode);
    ez_vfe_update_chromalumafilter(&(vfe_module->clf_mod),
      diagnostics, opmode);
    ez_vfe_update_rolloff(&(vfe_module->rolloff_mod),
      diagnostics, opmode);
    ez_vfe_update_lumaadaptation(&(vfe_module->la_mod),
      diagnostics, opmode);
    ez_vfe_update_sce(&(vfe_module->sce_mod),
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
    ez_vfe_update_linearization(&(vfe_module->linear_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_MCE))
    ez_vfe_update_mce(&(vfe_module->mce_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_COLOR_CONV))
    ez_vfe_update_colorconversion(&(vfe_module->chroma_enhan_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_CHROMA_SUPPRESS))
    ez_vfe_update_chromasuppression(&(vfe_module->chroma_supp_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_DEMOSAIC))
    ez_vfe_update_demosaic(&(vfe_module->demosaic_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_BPC))
    ez_vfe_update_bpc(&(vfe_module->bpc_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_BCC))
    ez_vfe_update_bcc(&(vfe_module->bcc_mod),
      diagnostics, opmode);

  /*TODO: no more asf in VFE*/
  /*if ((params->update & VFE_MOD_ASF))
    ez_vfe_update_asf(&(vfe_module->asf_mod),
      diagnostics, opmode);*/

  if ((params->update & VFE_MOD_DEMUX))
    ez_vfe_update_demuxchannelgain(&(vfe_module->demux_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_COLOR_CORRECT))
    ez_vfe_update_colorcorrection(&(vfe_module->color_correct_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_ABF))
    ez_vfe_update_abf(&(vfe_module->abf_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_CLF))
    ez_vfe_update_chromalumafilter(&(vfe_module->clf_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_ROLLOFF))
    ez_vfe_update_rolloff(&(vfe_module->rolloff_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_LA))
    ez_vfe_update_lumaadaptation(&(vfe_module->la_mod),
      diagnostics, opmode);

  if ((params->update & VFE_MOD_SCE))
    ez_vfe_update_sce(&(vfe_module->sce_mod),
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
    &(vfe_ctrl_obj->vfe_op_cfg.op_cmd.moduleCfg);
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);

  diagnostics->control_linear.enable =
    vfe_mcfg->blackLevelCorrectionEnable;
  diagnostics->control_colorcorr.enable =
    vfe_mcfg->colorCorrectionEnable;
  diagnostics->control_colorconv.enable = vfe_mcfg->chromaEnhanEnable;
  diagnostics->control_gamma.enable = vfe_mcfg->rgbLUTEnable;
  diagnostics->control_blacklevel.enable =
    vfe_mcfg->blackLevelCorrectionEnable;
  //diagnostics->control_asfsharp.enable = vfe_mcfg->asfEnable;
  diagnostics->control_lumaadaptation.enable =
    vfe_mcfg->lumaAdaptationEnable;
  diagnostics->control_rolloff.enable = vfe_mcfg->lensRollOffEnable;
  diagnostics->control_bcc.enable = vfe_module->bcc_mod.bcc_enable;
  diagnostics->control_bpc.enable = vfe_module->bpc_mod.bpc_enable;
  diagnostics->control_abfilter.enable = vfe_module->abf_mod.enable;
  diagnostics->control_chromasupp.enable =
    (vfe_mcfg->chromaSuppressionMceEnable &
    vfe_module->chroma_supp_mod.chroma_supp_video_cmd.chromaSuppressEn);
  diagnostics->control_memcolorenhan.enable =
    (vfe_mcfg->chromaSuppressionMceEnable &
     vfe_module->mce_mod.mce_cmd.enable);
  diagnostics->control_skincolorenhan.enable =
    vfe_mcfg->skinEnhancementEnable;
  diagnostics->control_demux.enable =
    vfe_mcfg->demuxEnable;
  diagnostics->control_clfilter.enable =
    vfe_mcfg->clfEnable;

  diagnostics->control_linear.cntrlenable =
    vfe_module->linear_mod.linear_trigger;
  diagnostics->control_colorcorr.cntrlenable =
    vfe_module->color_correct_mod.trigger_enable;
  diagnostics->control_colorconv.cntrlenable =
    vfe_module->chroma_enhan_mod.trigger_enable;
  diagnostics->control_gamma.cntrlenable =
    vfe_module->gamma_mod.trigger_enable;;
  diagnostics->control_blacklevel.cntrlenable =
    vfe_mcfg->blackLevelCorrectionEnable;
  /*TODO: no more asf in VFE*/
  /*diagnostics->control_asfsharp.cntrlenable =
    vfe_module->asf_mod.asf_trigger_enable;*/
  diagnostics->control_lumaadaptation.cntrlenable =
    vfe_module->la_mod.la_enable;
  if (vfe_ctrl_obj->vfe_params.vfe_version == MSM8960V2)
    diagnostics->control_rolloff.cntrlenable =
      vfe_module->rolloff_mod.pca_ctrl.pca_rolloff_trigger_enable;
  else
    diagnostics->control_rolloff.cntrlenable =
      vfe_module->rolloff_mod.mesh_ctrl.mesh_rolloff_trigger_enable;
  diagnostics->control_bcc.cntrlenable =
    vfe_module->bcc_mod.bcc_trigger_enable;
  diagnostics->control_bpc.cntrlenable =
    vfe_module->bpc_mod.bpc_trigger_enable;
  diagnostics->control_abfilter.cntrlenable =
    vfe_module->abf_mod.trigger_enable;
  diagnostics->control_chromasupp.cntrlenable =
    vfe_module->chroma_supp_mod.chroma_supp_trigger;
  diagnostics->control_memcolorenhan.cntrlenable =
    vfe_module->mce_mod.mce_trigger;
  diagnostics->control_demux.cntrlenable =
    vfe_module->demux_mod.trigger_enable;
  diagnostics->control_clfilter.cntrlenable =
    vfe_module->clf_mod.trigger_enable;
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

  status = vfe_module->rolloff_mod.ops.reload_params(mod_id,
    &vfe_module->rolloff_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: rolloff_mod %d", __func__, status);
    return;
  }
  status = vfe_module->bpc_mod.ops.reload_params(mod_id,
    &vfe_module->bpc_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: bpc_mod %d", __func__, status);
    return;
  }
  status = vfe_module->bcc_mod.ops.reload_params(mod_id,
    &vfe_module->bcc_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: bcc_mod %d", __func__, status);
    return;
  }
  /*TODO: no more asf in VFE*/
  /*status = vfe_module->asf_mod.ops.reload_params(mod_id,
    &vfe_module->asf_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: bcc_mod %d", __func__, status);
    return;
  }*/
  status = vfe_module->color_correct_mod.ops.reload_params(mod_id,
    &vfe_module->color_correct_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_color_correct_reload_params %d", __func__, status);
    return;
  }

  status = vfe_module->chroma_enhan_mod.ops.reload_params(mod_id,
    &vfe_module->chroma_enhan_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_color_conversion_reload_params %d", __func__, status);
    return;
  }

  status = vfe_module->gamma_mod.ops.reload_params(mod_id,
    &vfe_module->gamma_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_gamma_reload_params %d", __func__, status);
    return;
  }

  status = vfe_module->abf_mod.ops.reload_params(mod_id,
    &vfe_module->abf_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_abf2_reload_params %d", __func__, status);
    return;
  }

  status = vfe_module->demosaic_mod.ops.reload_params(mod_id,
    &vfe_module->demosaic_mod, params);
  if (status != VFE_SUCCESS) {
    CDBG_ERROR("%s: vfe_demosaic_reload_params %d", __func__, status);
    return;
  }

  status = vfe_module->demux_mod.ops.reload_params(mod_id,
    &vfe_module->demux_mod, params);
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
