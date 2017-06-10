/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_MCE_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - display_mce_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
void display_mce_config(mce_mod_t *mce_mod)
{
  CDBG("%s:MCE Configurations\n",__func__);
  CDBG("mce enable %d\n", mce_mod->mce_cmd.enable);
  CDBG("qk %d\n", mce_mod->mce_cmd.qk);
  CDBG("red:y1 %d, y2 %d, y3 %d, y4 %d, yM1 %d, yM3 %d, yS1 %d, yS3 %d,"
    " width %d, trunc %d, CR %d, CB %d, slope %d, K %d\n",
    mce_mod->mce_cmd.redCfg.y1, mce_mod->mce_cmd.redCfg.y2,
    mce_mod->mce_cmd.redCfg.y3, mce_mod->mce_cmd.redCfg.y4,
    mce_mod->mce_cmd.redCfg.yM1, mce_mod->mce_cmd.redCfg.yM3,
    mce_mod->mce_cmd.redCfg.yS1, mce_mod->mce_cmd.redCfg.yS3,
    mce_mod->mce_cmd.redCfg.transWidth, mce_mod->mce_cmd.redCfg.transTrunc,
    mce_mod->mce_cmd.redCfg.CRZone, mce_mod->mce_cmd.redCfg.CBZone,
    mce_mod->mce_cmd.redCfg.transSlope, mce_mod->mce_cmd.redCfg.K);
  CDBG("green:y1 %d, y2 %d, y3 %d, y4 %d, yM1 %d, yM3 %d, yS1 %d, yS3 %d,"
    " width %d, trunc %d, CR %d, CB %d, slope %d, K %d\n",
    mce_mod->mce_cmd.greenCfg.y1, mce_mod->mce_cmd.greenCfg.y2,
    mce_mod->mce_cmd.greenCfg.y3, mce_mod->mce_cmd.greenCfg.y4,
    mce_mod->mce_cmd.greenCfg.yM1, mce_mod->mce_cmd.greenCfg.yM3,
    mce_mod->mce_cmd.greenCfg.yS1, mce_mod->mce_cmd.greenCfg.yS3,
    mce_mod->mce_cmd.greenCfg.transWidth, mce_mod->mce_cmd.greenCfg.transTrunc,
    mce_mod->mce_cmd.greenCfg.CRZone, mce_mod->mce_cmd.greenCfg.CBZone,
    mce_mod->mce_cmd.greenCfg.transSlope, mce_mod->mce_cmd.greenCfg.K);
  CDBG("blue:y1 %d, y2 %d, y3 %d, y4 %d, yM1 %d, yM3 %d, yS1 %d, yS3 %d,"
    " width %d, trunc %d, CR %d, CB %d, slope %d, K %d\n",
    mce_mod->mce_cmd.blueCfg.y1, mce_mod->mce_cmd.blueCfg.y2,
    mce_mod->mce_cmd.blueCfg.y3, mce_mod->mce_cmd.blueCfg.y4,
    mce_mod->mce_cmd.blueCfg.yM1, mce_mod->mce_cmd.blueCfg.yM3,
    mce_mod->mce_cmd.blueCfg.yS1, mce_mod->mce_cmd.blueCfg.yS3,
    mce_mod->mce_cmd.blueCfg.transWidth, mce_mod->mce_cmd.blueCfg.transTrunc,
    mce_mod->mce_cmd.blueCfg.CRZone, mce_mod->mce_cmd.blueCfg.CBZone,
    mce_mod->mce_cmd.blueCfg.transSlope, mce_mod->mce_cmd.blueCfg.K);
}

/*===========================================================================
 * FUNCTION    - vfe_mce_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_init(int module_id, void *mod, void *vparams)
{
  mce_mod_t *mce_mod = (mce_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  VFE_MCE_ConfigCmdType *mce_cmd = &(mce_mod->mce_cmd);
  chromatix_parms_type *pchromatix = vfe_params->chroma3a;
  uint32_t Q_s1, Q_s3;

  CDBG("%s\n",__func__);
  /*
  Assuming aec ratio = 0, landscape severity = 0
  All boost factors = 0, and qk = 15
  */

  /* Overall Q_K and gains */
  mce_cmd->qk = 15;
  mce_cmd->redCfg.K = 0;
  mce_cmd->greenCfg.K = 0;
  mce_cmd->blueCfg.K = 0;

  /* Green */
  mce_cmd->greenCfg.y1 = pchromatix->mce_config.green_y[0];
  mce_cmd->greenCfg.y2 = pchromatix->mce_config.green_y[1];
  mce_cmd->greenCfg.y3 = pchromatix->mce_config.green_y[2];
  mce_cmd->greenCfg.y4 = pchromatix->mce_config.green_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->greenCfg.yM1 = 0;
  mce_cmd->greenCfg.yS1 = Q_s1 - mce_cmd->qk;
  Q_s3 = 20;
  mce_cmd->greenCfg.yM3 = 0;
  mce_cmd->greenCfg.yS3 = Q_s3 - mce_cmd->qk;

  /* Blue */
  mce_cmd->blueCfg.y1 = pchromatix->mce_config.blue_y[0];
  mce_cmd->blueCfg.y2 = pchromatix->mce_config.blue_y[1];
  mce_cmd->blueCfg.y3 = pchromatix->mce_config.blue_y[2];
  mce_cmd->blueCfg.y4 = pchromatix->mce_config.blue_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->blueCfg.yM1 = 0;
  mce_cmd->blueCfg.yS1 = Q_s1 - mce_cmd->qk;
  Q_s3 = 20;
  mce_cmd->blueCfg.yM3 = 0;
  mce_cmd->blueCfg.yS3 = Q_s3 - mce_cmd->qk;

  /* Red */
  mce_cmd->redCfg.y1 = pchromatix->mce_config.red_y[0];
  mce_cmd->redCfg.y2 = pchromatix->mce_config.red_y[1];
  mce_cmd->redCfg.y3 = pchromatix->mce_config.red_y[2];
  mce_cmd->redCfg.y4 = pchromatix->mce_config.red_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->redCfg.yM1 = 0;
  mce_cmd->redCfg.yS1 = Q_s1 - mce_cmd->qk;
  Q_s3 = 20;
  mce_cmd->redCfg.yM3 = 0;
  mce_cmd->redCfg.yS3 = Q_s3 - mce_cmd->qk;

  /* Compute C slopes */
  /* Green */
  mce_cmd->greenCfg.CBZone = pchromatix->mce_config.green_cb_boundary;
  mce_cmd->greenCfg.CRZone = pchromatix->mce_config.green_cr_boundary;
  mce_cmd->greenCfg.transWidth =
   (pchromatix->mce_config.green_cb_transition_width +
   pchromatix->mce_config.green_cr_transition_width) / 2;
  mce_cmd->greenCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->greenCfg.transWidth)) / log(2.0f) ) + 4;
  mce_cmd->greenCfg.transTrunc =
  Clamp(mce_cmd->greenCfg.transTrunc, 6, 9);
  mce_cmd->greenCfg.transSlope =
  (1 << mce_cmd->greenCfg.transTrunc) /
  mce_cmd->greenCfg.transWidth;

  /* Blue */
  mce_cmd->blueCfg.CBZone = pchromatix->mce_config.blue_cb_boundary;
  mce_cmd->blueCfg.CRZone = pchromatix->mce_config.blue_cr_boundary;
  mce_cmd->blueCfg.transWidth =
  (pchromatix->mce_config.blue_cb_transition_width +
   pchromatix->mce_config.blue_cr_transition_width) / 2;
  mce_cmd->blueCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->blueCfg.transWidth)) / log(2.0f)) + 4;
  mce_cmd->blueCfg.transTrunc =
  Clamp(mce_cmd->blueCfg.transTrunc, 6, 9);
  mce_cmd->blueCfg.transSlope =
  (1 << mce_cmd->blueCfg.transTrunc) /
  mce_cmd->blueCfg.transWidth;

  /* Red */
  mce_cmd->redCfg.CBZone =
  pchromatix->mce_config.red_cb_boundary;
  mce_cmd->redCfg.CRZone =
  pchromatix->mce_config.red_cr_boundary;
  mce_cmd->redCfg.transWidth =
  (pchromatix->mce_config.red_cb_transition_width +
   pchromatix->mce_config.red_cr_transition_width) / 2;
  mce_cmd->redCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->redCfg.transWidth)) / log(2.0f)) + 4;
  mce_cmd->redCfg.transTrunc =
  Clamp(mce_cmd->redCfg.transTrunc, 6, 9);
  mce_cmd->redCfg.transSlope =
    (1 << mce_cmd->redCfg.transTrunc) / mce_cmd->redCfg.transWidth;

  mce_mod->mce_trigger = TRUE;
  mce_mod->mce_update = FALSE;
  mce_mod->hw_enable = FALSE;
  mce_mod->prev_lux_idx = 0.0;
  mce_mod->prev_mode = VFE_OP_MODE_INVALID;
  return VFE_SUCCESS;
}


/*===========================================================================
 * FUNCTION    - vfe_mce_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_update(int module_id, void *mod, void *vparams)
{
  mce_mod_t *mce_mod = (mce_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t status;

  if (mce_mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(vfe_params->camfd,
      CMD_GENERAL, vfe_params->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    vfe_params->update |= VFE_MOD_MCE;
    mce_mod->hw_enable = FALSE;
  }

  if (!mce_mod->mce_enable) {
    CDBG("%s: MCE not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s:\n",__func__);

  if(mce_mod->mce_update) {
    mce_mod->mce_update = FALSE;
    status = vfe_mce_config(module_id, mce_mod,vfe_params);
    if (status != VFE_SUCCESS)
      CDBG_HIGH("%s: Failed\n",__func__);
    else
      vfe_params->update |= VFE_MOD_MCE;
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_mce_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_config(int module_id, void *mod, void *vparams)
{
  mce_mod_t *mce_mod = (mce_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (!mce_mod->mce_enable) {
    CDBG("%s: MCE not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s:\n",__func__);

  vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) &(mce_mod->mce_cmd),
    sizeof(mce_mod->mce_cmd), VFE_CMD_MCE_CFG);

  display_mce_config(mce_mod);

  return VFE_SUCCESS;
} /* vfe_mce_config */

/*===========================================================================
 * FUNCTION    - vfe_mce_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_trigger_update(int module_id, void *mod, void *vparams)
{
  mce_mod_t *mce_mod = (mce_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  float ratio, fKg, fKb, fKr, max_boost;
  uint32_t dS1, dS3, QKg, QKb, QKr, Q_s1, Q_s3;
  static int cnt = 0;
  VFE_MCE_ConfigCmdType *mce_config = &(mce_mod->mce_cmd);
  float landscape_green_boost_factor =
    vfe_params->chroma3a->landscape_scene_detect.landscape_green_boost_factor;
  float landscape_blue_boost_factor =
    vfe_params->chroma3a->landscape_scene_detect.landscape_blue_boost_factor;
  float landscape_red_boost_factor =
    vfe_params->chroma3a->landscape_scene_detect.landscape_red_boost_factor;
  uint32_t landscape_severity = 0;
  chromatix_parms_type *chromatix_ptr = vfe_params->chroma3a;
  float lux_idx;
  uint8_t update_mce = FALSE;

  lux_idx = vfe_params->aec_params.lux_idx;

  /* check for trigger updation */
  update_mce = ((mce_mod->prev_mode != vfe_params->vfe_op_mode) ||
    !F_EQUAL(mce_mod->prev_lux_idx, lux_idx));
  if(!update_mce) {
    CDBG("%s: MCE update not required", __func__);
    return VFE_SUCCESS;
  } else {
    mce_mod->prev_lux_idx = lux_idx;
    mce_mod->prev_mode = vfe_params->vfe_op_mode;
  }

  if (!mce_mod->mce_enable) {
    CDBG("%s: MCE not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (mce_mod->mce_trigger != TRUE) {
    CDBG("%s: MCE trigger not enabled\n",__func__);
    return VFE_SUCCESS;
  }
  if (!vfe_util_aec_check_settled(&vfe_params->aec_params)) {
    CDBG("%s: AEC not settled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s : lux : %f\n",__func__,lux_idx);

  if (vfe_params->bs_mode == CAMERA_BESTSHOT_LANDSCAPE)
    landscape_severity = 255; // 100 % severity
  else
    landscape_severity = MIN(255, vfe_params->asd_params.landscape_severity);

  /* Compute MCE gains and Q_K first */
  /* Green */
  ratio = 1.0f - vfe_util_calc_interpolation_weight(lux_idx,
    chromatix_ptr->mce_config.green_bright_index,
    chromatix_ptr->mce_config.green_dark_index);
  CDBG("ratio = %f, lux_idx %f\n", ratio, lux_idx);

  fKg = ratio * (chromatix_ptr->mce_config.green_boost_factor - 1.0f);
  /*add ratio for landscape severity  */
  CDBG("pre fKg =%f", fKg);
  fKg = ((fKg + 1) * (((float)landscape_severity / 255.0 *
    (landscape_green_boost_factor - 1)) + 1));

  max_boost = MAX(chromatix_ptr->mce_config.green_boost_factor,
    landscape_green_boost_factor);
  if (fKg > max_boost)
    fKg = max_boost;

  fKg = fKg-1;
  CDBG("post fKg =%f", fKg);
  if (fKg > 0) {
    QKg = (uint8_t)ceil(log(4.0f / fKg) / log(2.0f)) + 6;
    while ((int32_t)(fKg * (1 << QKg)) > 383)
      QKg--;
  } else {
    fKg = 0;
    QKg = 15;
  }
  QKg = Clamp(QKg, 7, 15);

  /* Blue */
  ratio = 1.0f - vfe_util_calc_interpolation_weight(lux_idx,
    chromatix_ptr->mce_config.blue_bright_index,
    chromatix_ptr->mce_config.blue_dark_index);
  fKb = ratio * (chromatix_ptr->mce_config.blue_boost_factor - 1.0f);
  /*add ratio for landscape severity */
  CDBG("pre fKb =%f", fKb);
  fKb = ((fKb + 1) * (((float)landscape_severity / 255.0 *
    (landscape_blue_boost_factor - 1)) + 1));

  max_boost = MAX(chromatix_ptr->mce_config.blue_boost_factor,
    landscape_blue_boost_factor);
  if (fKb > max_boost)
    fKb = max_boost;

  fKb = fKb-1;
  CDBG("post fKb =%f", fKb);
  if (fKb > 0) {
    QKb = (uint8_t)ceil(log(4.0f / fKb) / log(2.0f)) + 6;
    while ((int32_t)(fKb * (1 << QKb)) > 383)
      QKb--;
  } else {
    fKb = 0;
    QKb = 15;
  }
  QKb = Clamp(QKb, 7, 15);

  /* Red */
  ratio = 1.0f - vfe_util_calc_interpolation_weight(lux_idx,
    chromatix_ptr->mce_config.red_bright_index,
    chromatix_ptr->mce_config.red_dark_index);
  fKr = ratio * (chromatix_ptr->mce_config.red_boost_factor - 1.0f);
  CDBG("pre fKr =%f", fKr);
  /* add ratio for landscape severity */
  fKr = ((fKr + 1) * (((float)landscape_severity / 255.0 *
    (landscape_red_boost_factor - 1)) + 1));
  max_boost = MAX(chromatix_ptr->mce_config.red_boost_factor,
    landscape_red_boost_factor);
  if (fKr > max_boost)
    fKr = max_boost;

  fKr = fKr-1;
  CDBG("post fKr =%f", fKr);
  if (fKr > 0) {
    QKr = (uint8_t)ceil(log(4.0f / fKr) / log(2.0f)) + 6;
    while ((int32_t)(fKr * (1 << QKr)) > 383)
      QKr--;
  } else {
    fKr = 0;
    QKr = 15;
  }
  QKr = Clamp(QKr, 7, 15);

  /* Overall Q_K and gains */
  mce_config->qk = MIN(MIN(QKg, QKb), QKr);
  mce_config->redCfg.K =
  (int32_t)(fKr * (1 << mce_config->qk));
  mce_config->greenCfg.K =
  (int32_t)(fKg * (1 << mce_config->qk));
  mce_config->blueCfg.K =
  (int32_t)(fKb * (1 << mce_config->qk));

  /* Compute Y slopes */
  /* Green */

  dS1 = mce_config->greenCfg.y2 - mce_config->greenCfg.y1;
  dS3 = mce_config->greenCfg.y4 - mce_config->greenCfg.y3;

  if ((fKg > 0) && (dS1 > 0)) {
    Q_s1 = (int32_t)ceil(log(dS1 / fKg) / log(2.0f)) + 6;
    Q_s1 = Clamp(Q_s1, 7, 20);
    mce_config->greenCfg.yM1 = mce_config->greenCfg.K *
      (1 << (Q_s1 - mce_config->qk)) / dS1;
  } else {
    Q_s1 = 20;
    mce_config->greenCfg.yM1 = 0;
  }

  mce_config->greenCfg.yS1 = Q_s1 - mce_config->qk;
  if ((fKg > 0) && (dS3 > 0)) {
    Q_s3 = (int32_t)ceil(log(dS3/fKg)/log(2.0f)) + 6;
    Q_s3 = Clamp(Q_s3, 7, 20);
    mce_config->greenCfg.yM3 = mce_config->greenCfg.K *
      (1 << (Q_s3 - mce_config->qk)) / dS3;
  } else {
    Q_s3 = 20;
    mce_config->greenCfg.yM3 = 0;
  }

  mce_config->greenCfg.yS3 = Q_s3 - mce_config->qk;
  /* Blue */
  dS1 = mce_config->blueCfg.y2 - mce_config->blueCfg.y1;
  dS3 = mce_config->blueCfg.y4 - mce_config->blueCfg.y3;

  if ((fKb > 0) && (dS1 > 0)) {
    Q_s1 = (int32_t)ceil(log(dS1 / fKb) / log(2.0f)) + 6;
    Q_s1 = Clamp(Q_s1, 7, 20);
    mce_config->blueCfg.yM1 = mce_config->blueCfg.K *
      (1 << (Q_s1 - mce_config->qk)) / dS1;
  } else {
    Q_s1 = 20;
    mce_config->blueCfg.yM1 = 0;
  }
  mce_config->blueCfg.yS1 = Q_s1 - mce_config->qk;

  if ((fKb > 0) && (dS3 > 0)) {
    Q_s3 = (int32_t)ceil(log(dS3 / fKb) / log(2.0f)) + 6;
    Q_s3 = Clamp(Q_s3, 7, 20);
    mce_config->blueCfg.yM3 = mce_config->blueCfg.K *
      (1 << (Q_s3 - mce_config->qk)) / dS3;
  } else {
    Q_s3 = 20;
    mce_config->blueCfg.yM3 = 0;
  }

  mce_config->blueCfg.yS3 = Q_s3 - mce_config->qk;
  /* Red */
  dS1 = mce_config->redCfg.y2 - mce_config->redCfg.y1;
  dS3 = mce_config->redCfg.y4 - mce_config->redCfg.y3;

  if ((fKr > 0) && (dS1 > 0)) {
    Q_s1 = (int32_t)ceil(log(dS1 / fKr) / log(2.0f)) + 6;
    Q_s1 = Clamp(Q_s1, 7, 20);
    mce_config->redCfg.yM1 = mce_config->redCfg.K *
      (1 << (Q_s1 - mce_config->qk)) / dS1;
  } else {
    Q_s1 = 20;
    mce_config->redCfg.yM1 = 0;
  }
  mce_config->redCfg.yS1 = Q_s1 - mce_config->qk;

  if ((fKr > 0) && (dS3 > 0)) {
    Q_s3 = (int32_t)ceil(log(dS3 / fKr) / log(2.0f)) + 6;
    Q_s3 = Clamp(Q_s3, 7, 20);
    mce_config->redCfg.yM3 = mce_config->redCfg.K *
      (1 << (Q_s3 - mce_config->qk)) / dS3;
  } else {
    Q_s3 = 20;
    mce_config->redCfg.yM3 = 0;
  }
  mce_config->redCfg.yS3 = Q_s3 - mce_config->qk;
  /* Compute C slopes */
  /* Green */
  mce_config->greenCfg.CBZone =
    chromatix_ptr->mce_config.green_cb_boundary;
  mce_config->greenCfg.CRZone =
    chromatix_ptr->mce_config.green_cr_boundary;
  mce_config->greenCfg.transWidth =
   (chromatix_ptr->mce_config.green_cb_transition_width +
   chromatix_ptr->mce_config.green_cr_transition_width) / 2;
  mce_config->greenCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_config->greenCfg.transWidth)) / log(2.0f) ) + 4;
  mce_config->greenCfg.transTrunc =
  Clamp(mce_config->greenCfg.transTrunc, 6, 9);
  mce_config->greenCfg.transSlope =
  (1 << mce_config->greenCfg.transTrunc) /
  mce_config->greenCfg.transWidth;
  /* Blue */
  mce_config->blueCfg.CBZone = chromatix_ptr->mce_config.blue_cb_boundary;
  mce_config->blueCfg.CRZone = chromatix_ptr->mce_config.blue_cr_boundary;
  mce_config->blueCfg.transWidth =
  (chromatix_ptr->mce_config.blue_cb_transition_width +
   chromatix_ptr->mce_config.blue_cr_transition_width) / 2;
  mce_config->blueCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_config->blueCfg.transWidth)) / log(2.0f)) + 4;
  mce_config->blueCfg.transTrunc =
  Clamp(mce_config->blueCfg.transTrunc, 6, 9);
  mce_config->blueCfg.transSlope =
  (1 << mce_config->blueCfg.transTrunc) /
  mce_config->blueCfg.transWidth;
  /* Red */
  mce_config->redCfg.CBZone =
  chromatix_ptr->mce_config.red_cb_boundary;
  mce_config->redCfg.CRZone =
  chromatix_ptr->mce_config.red_cr_boundary;
  mce_config->redCfg.transWidth =
  (chromatix_ptr->mce_config.red_cb_transition_width +
   chromatix_ptr->mce_config.red_cr_transition_width) / 2;
  mce_config->redCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_config->redCfg.transWidth)) / log(2.0f)) + 4;
  mce_config->redCfg.transTrunc =
  Clamp(mce_config->redCfg.transTrunc, 6, 9);
  mce_config->redCfg.transSlope =
    (1 << mce_config->redCfg.transTrunc) / mce_config->redCfg.transWidth;

  mce_mod->mce_update = TRUE;

  if (cnt == 0) {
    CDBG("MCE_landscape_severity = %d\n", landscape_severity);
    CDBG("MCE aec ratio = %f, aec_out->lux_idx %f\n", ratio, lux_idx);
    display_mce_config(mce_mod);
  }
  cnt++;
  if (cnt == 6)
    cnt = 0;
  return VFE_SUCCESS;
}/* vfe_mce_trigger_update */

/*===========================================================================
 * FUNCTION    - vfe_mce_enable -
 *
 * DESCRIPTION: This function is called from UI
 *==========================================================================*/
vfe_status_t vfe_mce_enable(int module_id, void *mod, void *params,
  int8_t enable, int8_t hw_write)
{
  mce_mod_t *mce_mod = (mce_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  CDBG("%s: enable: %d\n", __func__, enable);
  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;
  vfe_params->moduleCfg->chromaSuppressionMceEnable |= enable;

  mce_mod->mce_cmd.enable = enable;  /*enable/disable the MCE in VFE HW*/

  if (hw_write && (mce_mod->mce_enable == enable))
    return VFE_SUCCESS;

  mce_mod->mce_enable = enable;
  mce_mod->hw_enable = hw_write;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_MCE)
      : (vfe_params->current_config & ~VFE_MOD_MCE);
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_mce_trigger_enable -
 *
 * DESCRIPTION: This function updates the mce trigger enable flag
 *==========================================================================*/
vfe_status_t vfe_mce_trigger_enable(int module_id, void *mod, void *vfe_params,
  int enable)
{
  mce_mod_t *mce_mod = (mce_mod_t *)mod;

  CDBG("%s:enable :%d\n",__func__, enable);
  mce_mod->mce_trigger = enable;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_mce_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
vfe_status_t vfe_mce_tv_validate(int module_id, void *input, void *output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  VFE_MCE_ConfigCmdType *in, *out;

  CDBG("%s:\n", __func__);
  in = (VFE_MCE_ConfigCmdType *)(mod_in->reg_dump + (V32_MCE_REG_OFF/4));
  out = (VFE_MCE_ConfigCmdType *)(mod_op->reg_dump_data + (V32_MCE_REG_OFF/4));

  VALIDATE_TST_VEC(in->qk, out->qk, 0, "qk");

  // Red Color
  VALIDATE_TST_VEC(in->redCfg.y1, out->redCfg.y1, 0, "redCfg.y1");
  VALIDATE_TST_VEC(in->redCfg.y2, out->redCfg.y2, 0, "redCfg.y2");
  VALIDATE_TST_VEC(in->redCfg.y3, out->redCfg.y3, 0, "redCfg.y3");
  VALIDATE_TST_VEC(in->redCfg.y4, out->redCfg.y4, 0, "redCfg.y4");
  VALIDATE_TST_VEC(in->redCfg.yM1, out->redCfg.yM1, 0, "redCfg.yM1");
  VALIDATE_TST_VEC(in->redCfg.yM3, out->redCfg.yM3, 0, "redCfg.yM3");
  VALIDATE_TST_VEC(in->redCfg.yS1, out->redCfg.yS1, 0, "redCfg.yS1");
  VALIDATE_TST_VEC(in->redCfg.yS3, out->redCfg.yS3, 0, "redCfg.yS3");
  VALIDATE_TST_VEC(in->redCfg.transWidth, out->redCfg.transWidth, 0, "redCfg.transWidth");
  VALIDATE_TST_VEC(in->redCfg.transTrunc, out->redCfg.transTrunc, 0, "redCfg.transTrunc");
  VALIDATE_TST_VEC(in->redCfg.CRZone, out->redCfg.CRZone, 0, "redCfg.CRZone");
  VALIDATE_TST_VEC(in->redCfg.CBZone, out->redCfg.CBZone, 0, "redCfg.CBZone");
  VALIDATE_TST_VEC(in->redCfg.transSlope, out->redCfg.transSlope, 0, "redCfg.transSlope");
  VALIDATE_TST_VEC(in->redCfg.K, out->redCfg.K, 0, "redCfg.K");

  // Green Color
  VALIDATE_TST_VEC(in->greenCfg.y1, out->greenCfg.y1, 0, "greenCfg.y1");
  VALIDATE_TST_VEC(in->greenCfg.y2, out->greenCfg.y2, 0, "greenCfg.y2");
  VALIDATE_TST_VEC(in->greenCfg.y3, out->greenCfg.y3, 0, "greenCfg.y3");
  VALIDATE_TST_VEC(in->greenCfg.y4, out->greenCfg.y4, 0, "greenCfg.y4");
  VALIDATE_TST_VEC(in->greenCfg.yM1, out->greenCfg.yM1, 0, "greenCfg.yM1");
  VALIDATE_TST_VEC(in->greenCfg.yM3, out->greenCfg.yM3, 0, "greenCfg.yM3");
  VALIDATE_TST_VEC(in->greenCfg.yS1, out->greenCfg.yS1, 0, "greenCfg.yS1");
  VALIDATE_TST_VEC(in->greenCfg.yS3, out->greenCfg.yS3, 0, "greenCfg.yS3");
  VALIDATE_TST_VEC(in->greenCfg.transWidth, out->greenCfg.transWidth, 0, "greenCfg.transWidth");
  VALIDATE_TST_VEC(in->greenCfg.transTrunc, out->greenCfg.transTrunc, 0, "greenCfg.transTrunc");
  VALIDATE_TST_VEC(in->greenCfg.CRZone, out->greenCfg.CRZone, 0, "greenCfg.CRZone");
  VALIDATE_TST_VEC(in->greenCfg.CBZone, out->greenCfg.CBZone, 0, "greenCfg.CBZone");
  VALIDATE_TST_VEC(in->greenCfg.transSlope, out->greenCfg.transSlope, 0, "greenCfg.transSlope");

  // Blue Color
  VALIDATE_TST_VEC(in->blueCfg.y1, out->blueCfg.y1, 0, "blueCfg.y1");
  VALIDATE_TST_VEC(in->blueCfg.y2, out->blueCfg.y2, 0, "blueCfg.y2");
  VALIDATE_TST_VEC(in->blueCfg.y3, out->blueCfg.y3, 0, "blueCfg.y3");
  VALIDATE_TST_VEC(in->blueCfg.y4, out->blueCfg.y4, 0, "blueCfg.y4");
  VALIDATE_TST_VEC(in->blueCfg.yM1, out->blueCfg.yM1, 0, "blueCfg.yM1");
  VALIDATE_TST_VEC(in->blueCfg.yM3, out->blueCfg.yM3, 0, "blueCfg.yM3");
  VALIDATE_TST_VEC(in->blueCfg.yS1, out->blueCfg.yS1, 0, "blueCfg.yS1");
  VALIDATE_TST_VEC(in->blueCfg.yS3, out->blueCfg.yS3, 0, "blueCfg.yS3");
  VALIDATE_TST_VEC(in->blueCfg.transWidth, out->blueCfg.transWidth, 0, "blueCfg.transWidth");
  VALIDATE_TST_VEC(in->blueCfg.transTrunc, out->blueCfg.transTrunc, 0, "blueCfg.transTrunc");
  VALIDATE_TST_VEC(in->blueCfg.CRZone, out->blueCfg.CRZone, 0, "blueCfg.CRZone");
  VALIDATE_TST_VEC(in->blueCfg.CBZone, out->blueCfg.CBZone, 0, "blueCfg.CBZone");
  VALIDATE_TST_VEC(in->blueCfg.transSlope, out->blueCfg.transSlope, 0, "blueCfg.transSlope");

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_mce_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_mce_deinit(int mod_id, void *module, void *params)
{
  mce_mod_t *mce_mod = (mce_mod_t *)module;
  memset(mce_mod, 0 , sizeof(mce_mod_t));
  return VFE_SUCCESS;
}
#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_mce_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  mce_module_t *cmd = (mce_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->mce_cmd),
     sizeof(VFE_MCE_ConfigCmdType),
     VFE_CMD_MCE_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_mce_plugin_update */
#endif
