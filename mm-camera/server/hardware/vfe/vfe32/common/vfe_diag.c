/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_diag_gamma -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_gamma(gamma_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  int i = 0, j = 0;
  int index = (is_snap) ? 1 : 0;
  uint8_t* p_gamma_table = p_mod->p_gamma_table[index];
  for (i = 0, j = 0; i < 1024 && j < 64; i+=16, j++) {
    p_isp_info->gamma.table[j] = p_gamma_table[i];
  }
}/*vfe_diag_gamma*/

/*===========================================================================
 * FUNCTION    - vfe_diag_linearization -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_linearization(linear_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  VFE_LinearizationLut *p_lut = is_snap ? &p_mod->linear_lut
    : &p_mod->linear_lut;
  QLinearization_t *p_lin = &p_isp_info->linearization;

  memcpy(p_lin->rlut_pl, p_lut->r_lut_p_l, sizeof(uint16_t) * 8);
  memcpy(p_lin->grlut_pl, p_lut->gr_lut_p, sizeof(uint16_t) * 8);
  memcpy(p_lin->gblut_pl, p_lut->gb_lut_p, sizeof(uint16_t) * 8);
  memcpy(p_lin->blut_pl, p_lut->b_lut_p, sizeof(uint16_t) * 8);
  memcpy(p_lin->rlut_base, p_lut->r_lut_base, sizeof(uint16_t) * 9);
  memcpy(p_lin->grlut_base, p_lut->gr_lut_base, sizeof(uint16_t) * 9);
  memcpy(p_lin->gblut_base, p_lut->gb_lut_base, sizeof(uint16_t) * 9);
  memcpy(p_lin->blut_base, p_lut->b_lut_base, sizeof(uint16_t) * 9);
  memcpy(p_lin->rlut_delta, p_lut->r_lut_delta, sizeof(uint32_t) * 9);
  memcpy(p_lin->grlut_delta, p_lut->gr_lut_delta, sizeof(uint32_t) * 9);
  memcpy(p_lin->gblut_delta, p_lut->gb_lut_delta, sizeof(uint32_t) * 9);
  memcpy(p_lin->blut_delta, p_lut->b_lut_delta, sizeof(uint32_t) * 9);
}/*vfe_diag_linearization*/

/*===========================================================================
 * FUNCTION    - vfe_diag_mce -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_mce(mce_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  VFE_MCE_ConfigCmdType *mceCfg = &p_mod->mce_cmd;
  QMemoryColor_t *p_mce = &p_isp_info->mce;

  p_mce->qk = mceCfg->qk;
  p_mce->blue.y1 = mceCfg->blueCfg.y1;
  p_mce->blue.y2 = mceCfg->blueCfg.y2;
  p_mce->blue.y3 = mceCfg->blueCfg.y3;
  p_mce->blue.y4 = mceCfg->blueCfg.y4;
  p_mce->blue.yM1 = mceCfg->blueCfg.yM1;
  p_mce->blue.yM3 = mceCfg->blueCfg.yM3;
  p_mce->blue.yS1 = mceCfg->blueCfg.yS1;
  p_mce->blue.yS3 = mceCfg->blueCfg.yS3;
  p_mce->blue.transWidth = mceCfg->blueCfg.transWidth;
  p_mce->blue.transTrunc = mceCfg->blueCfg.transTrunc;
  p_mce->blue.crZone = mceCfg->blueCfg.CRZone;
  p_mce->blue.cbZone = mceCfg->blueCfg.CBZone;
  p_mce->blue.translope = mceCfg->blueCfg.transSlope;
  p_mce->blue.k = mceCfg->blueCfg.K;

  p_mce->red.y1 = mceCfg->redCfg.y1;
  p_mce->red.y2 = mceCfg->redCfg.y2;
  p_mce->red.y3 = mceCfg->redCfg.y3;
  p_mce->red.y4 = mceCfg->redCfg.y4;
  p_mce->red.yM1 = mceCfg->redCfg.yM1;
  p_mce->red.yM3 = mceCfg->redCfg.yM3;
  p_mce->red.yS1 = mceCfg->redCfg.yS1;
  p_mce->red.yS3 = mceCfg->redCfg.yS3;
  p_mce->red.transWidth = mceCfg->redCfg.transWidth;
  p_mce->red.transTrunc = mceCfg->redCfg.transTrunc;
  p_mce->red.crZone = mceCfg->redCfg.CRZone;
  p_mce->red.cbZone = mceCfg->redCfg.CBZone;
  p_mce->red.translope = mceCfg->redCfg.transSlope;
  p_mce->red.k = mceCfg->redCfg.K;

  p_mce->green.y1 = mceCfg->greenCfg.y1;
  p_mce->green.y2 = mceCfg->greenCfg.y2;
  p_mce->green.y3 = mceCfg->greenCfg.y3;
  p_mce->green.y4 = mceCfg->greenCfg.y4;
  p_mce->green.yM1 = mceCfg->greenCfg.yM1;
  p_mce->green.yM3 = mceCfg->greenCfg.yM3;
  p_mce->green.yS1 = mceCfg->greenCfg.yS1;
  p_mce->green.yS3 = mceCfg->greenCfg.yS3;
  p_mce->green.transWidth = mceCfg->greenCfg.transWidth;
  p_mce->green.transTrunc = mceCfg->greenCfg.transTrunc;
  p_mce->green.crZone = mceCfg->greenCfg.CRZone;
  p_mce->green.cbZone = mceCfg->greenCfg.CBZone;
  p_mce->green.translope = mceCfg->greenCfg.transSlope;
  p_mce->green.k = mceCfg->greenCfg.K;
}/*vfe_diag_mce*/

/*===========================================================================
 * FUNCTION    - vfe_diag_cc -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_cc(color_correct_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  VFE_ColorCorrectionCfgCmdType *colorCorrectionCfg =
    (is_snap) ? &(p_mod->VFE_SnapColorCorrectionCmd)
    : &(p_mod->VFE_PrevColorCorrectionCmd);
  QColorCorrect_t *p_cc = &p_isp_info->cc;

  p_cc->coef_rtor = colorCorrectionCfg->C8;
  p_cc->coef_gtor = colorCorrectionCfg->C6;
  p_cc->coef_btor = colorCorrectionCfg->C7;
  p_cc->coef_rtog = colorCorrectionCfg->C2;
  p_cc->coef_gtog = colorCorrectionCfg->C0;
  p_cc->coef_btog = colorCorrectionCfg->C1;
  p_cc->coef_rtob = colorCorrectionCfg->C5;
  p_cc->coef_gtob = colorCorrectionCfg->C3;
  p_cc->coef_btob = colorCorrectionCfg->C4;
  p_cc->roffset = colorCorrectionCfg->K0;
  p_cc->goffset = colorCorrectionCfg->K1;
  p_cc->boffset = colorCorrectionCfg->K2;
  p_cc->coef_qfactor = colorCorrectionCfg->coefQFactor;
}/*vfe_diag_cc*/

/*===========================================================================
 * FUNCTION    - vfe_diag_demosaic -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_demosaic(demosaic_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap, int config)
{
  VFE_DemosaicV3ConfigCmdType *demosaicCfg =
    is_snap ? &(p_mod->demosaic_snapshot_cmd)
    : &(p_mod->demosaic_cmd);
  VFE_DemosaicV3UpdateCmdType *demosaicUpdate =
    (is_snap) ? &(p_mod->demosaic_snap_up_cmd)
    : &(p_mod->demosaic_vf_up_cmd);
  QDemosaic_t *p_dem = &p_isp_info->demosaic;
  int index;

  if (config) {
    p_dem->aG = demosaicCfg->a;
    p_dem->bL = demosaicCfg->bl;
    for(index = 0; index < 18; index++) {
      p_dem->lut[index].bk = demosaicCfg->interpClassifier[index].b_n;
      p_dem->lut[index].wk = demosaicCfg->interpClassifier[index].w_n;
      p_dem->lut[index].lk = demosaicCfg->interpClassifier[index].l_n;
      p_dem->lut[index].tk = demosaicCfg->interpClassifier[index].t_n;
    }
  } else {
    p_dem->aG = demosaicUpdate->a;
    p_dem->bL = demosaicUpdate->bl;
  }
}/*vfe_diag_demosaic*/

/*===========================================================================
 * FUNCTION    - vfe_diag_asf -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_asf(asf_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  VFE_AdaptiveFilterConfigCmdType *asfCfg =
    (is_snap) ? &(p_mod->asf_snap_cmd) :
    &p_mod->asf_prev_cmd;
  QASF_t *p_asf = &p_isp_info->asf;

  p_asf->smoothfilterEnabled = asfCfg->smoothFilterEnabled;
  p_asf->sharpMode = asfCfg->sharpMode;
  p_asf->lpfMode = asfCfg->lpfMode;
  p_asf->smoothcoefCenter = asfCfg->smoothCoefCenter;
  p_asf->smoothcoefSurr = asfCfg->smoothCoefSurr;
  p_asf->pipeflushCount = asfCfg->pipeFlushCount;
  p_asf->pipeflushOvd = asfCfg->pipeFlushOvd;
  p_asf->flushhaltOvd = asfCfg->flushHaltOvd;
  p_asf->cropEnable = asfCfg->cropEnable;
  p_asf->normalizeFactor = asfCfg->normalizeFactor;
  p_asf->sharpthreshE1 = asfCfg->sharpThreshE1;
  p_asf->sharpthreshE2 = asfCfg->sharpThreshE2;
  p_asf->sharpthreshE3 = asfCfg->sharpThreshE3;
  p_asf->sharpthreshE4 = asfCfg->sharpThreshE4;
  p_asf->sharpthreshE5 = asfCfg->sharpThreshE5;
  p_asf->sharpK1 = asfCfg->sharpK1;
  p_asf->sharpK2 = asfCfg->sharpK2;
  p_asf->f1coef0 = asfCfg->F1Coeff0;
  p_asf->f1coef1 = asfCfg->F1Coeff1;
  p_asf->f1coef2 = asfCfg->F1Coeff2;
  p_asf->f1coef3 = asfCfg->F1Coeff3;
  p_asf->f1coef4 = asfCfg->F1Coeff4;
  p_asf->f1coef5 = asfCfg->F1Coeff5;
  p_asf->f1coef6 = asfCfg->F1Coeff6;
  p_asf->f1coef7 = asfCfg->F1Coeff7;
  p_asf->f1coef8 = asfCfg->F1Coeff8;
  p_asf->f2coef0 = asfCfg->F2Coeff0;
  p_asf->f2coef1 = asfCfg->F2Coeff1;
  p_asf->f2coef2 = asfCfg->F2Coeff2;
  p_asf->f2coef3 = asfCfg->F2Coeff3;
  p_asf->f2coef4 = asfCfg->F2Coeff4;
  p_asf->f2coef5 = asfCfg->F2Coeff5;
  p_asf->f2coef6 = asfCfg->F2Coeff6;
  p_asf->f2coef7 = asfCfg->F2Coeff7;
  p_asf->f2coef8 = asfCfg->F2Coeff8;
  p_asf->f3coef0 = asfCfg->F3Coeff0;
  p_asf->f3coef1 = asfCfg->F3Coeff1;
  p_asf->f3coef2 = asfCfg->F3Coeff2;
  p_asf->f3coef3 = asfCfg->F3Coeff3;
  p_asf->f3coef4 = asfCfg->F3Coeff4;
  p_asf->f3coef5 = asfCfg->F3Coeff5;
  p_asf->f3coef6 = asfCfg->F3Coeff6;
  p_asf->f3coef7 = asfCfg->F3Coeff7;
  p_asf->f3coef8 = asfCfg->F3Coeff8;

}/*vfe_diag_asf*/

/*===========================================================================
 * FUNCTION    - vfe_diag_abf -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_abf(abf_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  VFE_DemosaicABF_CmdType *abfCfg =
    (is_snap) ? &p_mod->VFE_DemosaicABFCfgCmd :
    &p_mod->VFE_SnapshotDemosaicABFCfgCmd;
  QABF_t *p_abf = &p_isp_info->abf;
  int idx;

  p_abf->enable = p_mod->enable;
  p_abf->red.threshold[0] = abfCfg->rCfg.Cutoff1;
  p_abf->green.threshold[0] = abfCfg->gCfg.Cutoff1;
  p_abf->blue.threshold[0] = abfCfg->bCfg.Cutoff1;
  p_abf->red.threshold[1] = abfCfg->rCfg.Cutoff2;
  p_abf->green.threshold[1] = abfCfg->gCfg.Cutoff2;
  p_abf->blue.threshold[1] = abfCfg->bCfg.Cutoff2;
  p_abf->red.threshold[2] = abfCfg->rCfg.Cutoff3;
  p_abf->green.threshold[2] = abfCfg->gCfg.Cutoff3;
  p_abf->blue.threshold[2] = abfCfg->bCfg.Cutoff3;

  for(idx = 0; idx < 8; idx++) {
    p_abf->red.pos[2*idx] = abfCfg->rPosLut[idx].PostiveLUT0;
    p_abf->red.pos[2*idx+1] = abfCfg->rPosLut[idx].PostiveLUT1;
    p_abf->blue.pos[2*idx] = abfCfg->bPosLut[idx].PostiveLUT0;
    p_abf->blue.pos[2*idx+1] = abfCfg->bPosLut[idx].PostiveLUT1;
    p_abf->green.pos[2*idx] =  abfCfg->gPosLut[idx].PostiveLUT0;
    p_abf->green.pos[2*idx+1] =  abfCfg->gPosLut[idx].PostiveLUT1;
  }

  for(idx = 0; idx < 4; idx++) {
    p_abf->red.neg[2*idx] = abfCfg->rNegLut[idx].NegativeLUT0;
    p_abf->red.neg[2*idx+1] = abfCfg->rNegLut[idx].NegativeLUT1;
    p_abf->blue.neg[2*idx] = abfCfg->bNegLut[idx].NegativeLUT0;
    p_abf->blue.neg[2*idx+1] = abfCfg->bNegLut[idx].NegativeLUT1;
    p_abf->green.neg[2*idx] =  abfCfg->gNegLut[idx].NegativeLUT0;
    p_abf->green.neg[2*idx+1] =  abfCfg->gNegLut[idx].NegativeLUT1;
  }
}/*vfe_diag_abf*/

/*===========================================================================
 * FUNCTION    - vfe_diag_rolloff -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_rolloff(rolloff_mod_t *p_mod, QISPInfo_t *p_isp_info,
  int is_snap)
{
  pca_rolloff_params_t *rolloffCfg = is_snap ?
    &(p_mod->pca_ctrl.pca_rolloff_snap_param) :
    &(p_mod->pca_ctrl.pca_rolloff_prev_param);
  int row, col;
  QLensCorrection_t *p_lc = &p_isp_info->lc;

  for (row = 0; row < ROLLOFF_NUM_ROWS; row++)
    for (col = 0; col < ROLLOFF_NUM_BASE; col++) {
      p_lc->coefftable_R[row][col] =
        rolloffCfg->left_input_table.coeff_table_R[row][col];
      p_lc->coefftable_Gr[row][col] =
        rolloffCfg->left_input_table.coeff_table_Gr[row][col];
      p_lc->coefftable_Gb[row][col] =
        rolloffCfg->left_input_table.coeff_table_Gb[row][col];
      p_lc->coefftable_B[row][col] =
        rolloffCfg->left_input_table.coeff_table_B[row][col];
    }
  for (row = 0; row < ROLLOFF_NUM_BASE; row++)
    for (col = 0; col < ROLLOFF_NUM_COLS; col++)
      p_lc->basistable[row][col] =
        rolloffCfg->left_input_table.PCA_basis_table[row][col];
}/*vfe_diag_rolloff*/

/*===========================================================================
 * FUNCTION    - vfe_diag_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_diag_update(void *ctrl_obj, int config)
{
  vfe_ctrl_info_t *vfe_ctrl_obj = (vfe_ctrl_info_t *)ctrl_obj;
  vfe_module_t *vfe_module = &(vfe_ctrl_obj->vfe_module);
  vfe_params_t *params = &(vfe_ctrl_obj->vfe_params);
  vfe_op_mode_t vfe_mode = (vfe_op_mode_t)(params->vfe_op_mode);
  QISPInfo_t *p_isp_info = &vfe_ctrl_obj->isp_info;
  int is_snap = IS_SNAP_MODE(params);

  if (config || (params->update & VFE_MOD_LINEARIZATION)) {
    p_isp_info->linearization.enable =
      params->moduleCfg->blackLevelCorrectionEnable;
    vfe_diag_linearization(&(vfe_module->linear_mod),
      p_isp_info, is_snap);
  }

  if (config || (params->update & VFE_MOD_MCE)) {
    p_isp_info->mce.enable =
      params->moduleCfg->chromaSuppressionMceEnable;
    vfe_diag_mce(&(vfe_module->mce_mod),
      p_isp_info, is_snap);
  }

  if (config || (params->update & VFE_MOD_COLOR_CONV)) {
    p_isp_info->cc.enable =
      params->moduleCfg->colorCorrectionEnable;
    vfe_diag_cc(&(vfe_module->color_correct_mod),
      p_isp_info, is_snap);
  }

  if (config || (params->update & VFE_MOD_DEMOSAIC)) {
    p_isp_info->demosaic.enable =
      params->moduleCfg->demosaicEnable;
    vfe_diag_demosaic(&(vfe_module->demosaic_mod),
      p_isp_info, is_snap, config);
  }

  if (config || (params->update & VFE_MOD_ASF)) {
    p_isp_info->asf.enable =
      params->moduleCfg->asfEnable;
    vfe_diag_asf(&(vfe_module->asf_mod),
      p_isp_info, is_snap);
  }

  if (config || (params->update & VFE_MOD_ABF)) {
    vfe_diag_abf(&(vfe_module->abf_mod),
      p_isp_info, is_snap);
  }

  if (config || (params->update & VFE_MOD_ROLLOFF)) {
    p_isp_info->lc.enable =
      params->moduleCfg->lensRollOffEnable;
    vfe_diag_rolloff(&(vfe_module->rolloff_mod),
      p_isp_info, is_snap);
  }

  if (config || (params->update & VFE_MOD_GAMMA)) {
    p_isp_info->gamma.enable =
      params->moduleCfg->rgbLUTEnable;
    vfe_diag_gamma(&(vfe_module->gamma_mod),
      p_isp_info, is_snap);
  }
}/*vfe_diag_update*/
