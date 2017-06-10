/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "mce32.h"
#include "isp_log.h"

#ifdef ENABLE_MCE_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

/** display_mce_config:
 *    @mce_mod: Pointer to mce configuration.
 *
 * Print mce configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void display_mce_config(isp_mce_mod_t *mce_mod)
{
  ISP_DBG(ISP_MOD_MCE, "%s:MCE Configurations\n",__func__);
  ISP_DBG(ISP_MOD_MCE, "%s: mce enable %d\n", __func__, mce_mod->mce_mix_cmd_1.enable);
  ISP_DBG(ISP_MOD_MCE, "%s: qk %d\n", __func__,mce_mod->mce_mix_cmd_2.qk);
  ISP_DBG(ISP_MOD_MCE, "red:y1 %d, y2 %d, y3 %d, y4 %d, yM1 %d, yM3 %d, yS1 %d, yS3 %d,"
    " width %d, trunc %d, CR %d, CB %d, slope %d, K %d\n",
    mce_mod->mce_cmd.redCfg.y1, mce_mod->mce_cmd.redCfg.y2,
    mce_mod->mce_cmd.redCfg.y3, mce_mod->mce_cmd.redCfg.y4,
    mce_mod->mce_cmd.redCfg.yM1, mce_mod->mce_cmd.redCfg.yM3,
    mce_mod->mce_cmd.redCfg.yS1, mce_mod->mce_cmd.redCfg.yS3,
    mce_mod->mce_cmd.redCfg.transWidth, mce_mod->mce_cmd.redCfg.transTrunc,
    mce_mod->mce_cmd.redCfg.CRZone, mce_mod->mce_cmd.redCfg.CBZone,
    mce_mod->mce_cmd.redCfg.transSlope, mce_mod->mce_cmd.redCfg.K);
  ISP_DBG(ISP_MOD_MCE, "green:y1 %d, y2 %d, y3 %d, y4 %d, yM1 %d, yM3 %d, yS1 %d, yS3 %d,"
    " width %d, trunc %d, CR %d, CB %d, slope %d, K %d\n",
    mce_mod->mce_cmd.greenCfg.y1, mce_mod->mce_cmd.greenCfg.y2,
    mce_mod->mce_cmd.greenCfg.y3, mce_mod->mce_cmd.greenCfg.y4,
    mce_mod->mce_cmd.greenCfg.yM1, mce_mod->mce_cmd.greenCfg.yM3,
    mce_mod->mce_cmd.greenCfg.yS1, mce_mod->mce_cmd.greenCfg.yS3,
    mce_mod->mce_cmd.greenCfg.transWidth, mce_mod->mce_cmd.greenCfg.transTrunc,
    mce_mod->mce_cmd.greenCfg.CRZone, mce_mod->mce_cmd.greenCfg.CBZone,
    mce_mod->mce_cmd.greenCfg.transSlope, mce_mod->mce_cmd.greenCfg.K);
  ISP_DBG(ISP_MOD_MCE, "blue:y1 %d, y2 %d, y3 %d, y4 %d, yM1 %d, yM3 %d, yS1 %d, yS3 %d,"
    " width %d, trunc %d, CR %d, CB %d, slope %d, K %d\n",
    mce_mod->mce_cmd.blueCfg.y1, mce_mod->mce_cmd.blueCfg.y2,
    mce_mod->mce_cmd.blueCfg.y3, mce_mod->mce_cmd.blueCfg.y4,
    mce_mod->mce_cmd.blueCfg.yM1, mce_mod->mce_cmd.blueCfg.yM3,
    mce_mod->mce_cmd.blueCfg.yS1, mce_mod->mce_cmd.blueCfg.yS3,
    mce_mod->mce_cmd.blueCfg.transWidth, mce_mod->mce_cmd.blueCfg.transTrunc,
    mce_mod->mce_cmd.blueCfg.CRZone, mce_mod->mce_cmd.blueCfg.CBZone,
    mce_mod->mce_cmd.blueCfg.transSlope, mce_mod->mce_cmd.blueCfg.K);
}

/** mce_config:
 *    @mce_mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Configure module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int mce_config(isp_mce_mod_t *mce_mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  ISP_MCE_ConfigCmdType *mce_cmd = &(mce_mod->mce_cmd);
  chromatix_parms_type *pchromatix =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CS_MCE_type *pchromatix_CS_MCE =
    &pchromatix->chromatix_VFE.chromatix_CS_MCE;
  uint32_t Q_s1, Q_s3;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  if (!mce_mod->mce_enable) {
    ISP_DBG(ISP_MOD_MCE, "%s: MCE not enabled", __func__);
    return 0;
  }

  /*
   Assuming aec ratio = 0, landscape severity = 0
   All boost factors = 0, and qk = 15
   */

  mce_mod->mce_mix_cmd_1.enable = mce_mod->mce_enable;
  /* Overall Q_K and gains */
  mce_mod->mce_mix_cmd_2.qk = 15;
  mce_cmd->redCfg.K = 0;
  mce_cmd->greenCfg.K = 0;
  mce_cmd->blueCfg.K = 0;

  /* Green */
  mce_cmd->greenCfg.y1 = pchromatix_CS_MCE->mce_config.green_y[0];
  mce_cmd->greenCfg.y2 = pchromatix_CS_MCE->mce_config.green_y[1];
  mce_cmd->greenCfg.y3 = pchromatix_CS_MCE->mce_config.green_y[2];
  mce_cmd->greenCfg.y4 = pchromatix_CS_MCE->mce_config.green_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->greenCfg.yM1 = 0;
  mce_cmd->greenCfg.yS1 = Q_s1 - mce_mod->mce_mix_cmd_2.qk;
  Q_s3 = 20;
  mce_cmd->greenCfg.yM3 = 0;
  mce_cmd->greenCfg.yS3 = Q_s3 - mce_mod->mce_mix_cmd_2.qk;

  /* Blue */
  mce_cmd->blueCfg.y1 = pchromatix_CS_MCE->mce_config.blue_y[0];
  mce_cmd->blueCfg.y2 = pchromatix_CS_MCE->mce_config.blue_y[1];
  mce_cmd->blueCfg.y3 = pchromatix_CS_MCE->mce_config.blue_y[2];
  mce_cmd->blueCfg.y4 = pchromatix_CS_MCE->mce_config.blue_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->blueCfg.yM1 = 0;
  mce_cmd->blueCfg.yS1 = Q_s1 - mce_mod->mce_mix_cmd_2.qk;
  Q_s3 = 20;
  mce_cmd->blueCfg.yM3 = 0;
  mce_cmd->blueCfg.yS3 = Q_s3 - mce_mod->mce_mix_cmd_2.qk;

  /* Red */
  mce_cmd->redCfg.y1 = pchromatix_CS_MCE->mce_config.red_y[0];
  mce_cmd->redCfg.y2 = pchromatix_CS_MCE->mce_config.red_y[1];
  mce_cmd->redCfg.y3 = pchromatix_CS_MCE->mce_config.red_y[2];
  mce_cmd->redCfg.y4 = pchromatix_CS_MCE->mce_config.red_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->redCfg.yM1 = 0;
  mce_cmd->redCfg.yS1 = Q_s1 - mce_mod->mce_mix_cmd_2.qk;
  Q_s3 = 20;
  mce_cmd->redCfg.yM3 = 0;
  mce_cmd->redCfg.yS3 = Q_s3 - mce_mod->mce_mix_cmd_2.qk;

  /* Compute C slopes */
  /* Green */
  mce_cmd->greenCfg.CBZone = pchromatix_CS_MCE->mce_config.green_cb_boundary;
  mce_cmd->greenCfg.CRZone = pchromatix_CS_MCE->mce_config.green_cr_boundary;
  mce_cmd->greenCfg.transWidth =
   (pchromatix_CS_MCE->mce_config.green_cb_transition_width +
    pchromatix_CS_MCE->mce_config.green_cr_transition_width) / 2;
  mce_cmd->greenCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->greenCfg.transWidth)) / log(2.0f) ) + 4;
  mce_cmd->greenCfg.transTrunc =
  Clamp(mce_cmd->greenCfg.transTrunc, 6, 9);
  mce_cmd->greenCfg.transSlope =
  (1 << mce_cmd->greenCfg.transTrunc) /
  mce_cmd->greenCfg.transWidth;

  /* Blue */
  mce_cmd->blueCfg.CBZone = pchromatix_CS_MCE->mce_config.blue_cb_boundary;
  mce_cmd->blueCfg.CRZone = pchromatix_CS_MCE->mce_config.blue_cr_boundary;
  mce_cmd->blueCfg.transWidth =
  (pchromatix_CS_MCE->mce_config.blue_cb_transition_width +
   pchromatix_CS_MCE->mce_config.blue_cr_transition_width) / 2;
  mce_cmd->blueCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->blueCfg.transWidth)) / log(2.0f)) + 4;
  mce_cmd->blueCfg.transTrunc =
  Clamp(mce_cmd->blueCfg.transTrunc, 6, 9);
  mce_cmd->blueCfg.transSlope =
  (1 << mce_cmd->blueCfg.transTrunc) /
  mce_cmd->blueCfg.transWidth;

  /* Red */
  mce_cmd->redCfg.CBZone =
    pchromatix_CS_MCE->mce_config.red_cb_boundary;
  mce_cmd->redCfg.CRZone =
    pchromatix_CS_MCE->mce_config.red_cr_boundary;
  mce_cmd->redCfg.transWidth =
    (pchromatix_CS_MCE->mce_config.red_cb_transition_width +
    pchromatix_CS_MCE->mce_config.red_cr_transition_width) / 2;
  mce_cmd->redCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->redCfg.transWidth)) / log(2.0f)) + 4;
  mce_cmd->redCfg.transTrunc =
  Clamp(mce_cmd->redCfg.transTrunc, 6, 9);
  mce_cmd->redCfg.transSlope =
    (1 << mce_cmd->redCfg.transTrunc) / mce_cmd->redCfg.transWidth;

  mce_mod->prev_lux_idx = 0.0;

  mce_mod->hw_update_pending = TRUE;
  return 0;
} /* mce_config */

/** mce_trigger_update:
 *    @mce_mod: pointer to instance private data
 *    @trigger_params: input data
 *    @in_params_size: size of input data
 *
 * Update configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int mce_trigger_update(isp_mce_mod_t *mce_mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  float ratio, fKg, fKb, fKr, max_boost;
  uint32_t dS1, dS3, QKg, QKb, QKr, Q_s1, Q_s3;
  ISP_MCE_ConfigCmdType *mce_config = &(mce_mod->mce_cmd);
  chromatix_parms_type *chromatix_ptr = trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_CS_MCE_type *pchromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;
  ASD_struct_type *ASD_algo_data_ptr = &chromatix_ptr->ASD_algo_data;

  float landscape_green_boost_factor =
    ASD_algo_data_ptr->landscape_scene_detect.landscape_green_boost_factor;
  float landscape_blue_boost_factor =
    ASD_algo_data_ptr->landscape_scene_detect.landscape_blue_boost_factor;
  float landscape_red_boost_factor =
    ASD_algo_data_ptr->landscape_scene_detect.landscape_red_boost_factor;
  uint32_t landscape_severity = 0;
  float lux_idx;
  uint8_t update_mce = FALSE;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  lux_idx = trigger_params->trigger_input.stats_update.aec_update.lux_idx;

  /* check for trigger updation */
  update_mce = ((mce_mod->old_streaming_mode!= trigger_params->cfg.streaming_mode) ||
    !F_EQUAL(mce_mod->prev_lux_idx, lux_idx));
  if (!update_mce) {
    ISP_DBG(ISP_MOD_MCE, "%s: MCE update not required", __func__);
    return 0;
  } else {
    mce_mod->prev_lux_idx = lux_idx;
    mce_mod->old_streaming_mode = trigger_params->cfg.streaming_mode;
  }

  if (!mce_mod->mce_enable) {
    ISP_DBG(ISP_MOD_MCE, "%s: MCE not enabled", __func__);
    return 0;
  }
  if (mce_mod->mce_trigger_enable != TRUE) {
    ISP_DBG(ISP_MOD_MCE, "%s: MCE trigger not enabled\n", __func__);
    return 0;
  }
  if (!isp_util_aec_check_settled(
    &trigger_params->trigger_input.stats_update.aec_update)) {
    ISP_DBG(ISP_MOD_MCE, "%s: AEC not settled", __func__);
    return 0;
  }

  if (trigger_params->cfg.bestshot_mode == CAM_SCENE_MODE_LANDSCAPE)
    landscape_severity = 255; // 100 % severity
  else
    landscape_severity =
      MIN(255, trigger_params->trigger_input.stats_update.asd_update.landscape_severity);

  /* Compute MCE gains and Q_K first */
  /* Green */
  ratio = 1.0f - isp_util_calc_interpolation_weight(lux_idx,
      pchromatix_CS_MCE->mce_config.green_bright_index,
      pchromatix_CS_MCE->mce_config.green_dark_index);
  ISP_DBG(ISP_MOD_MCE, "ratio = %f, lux_idx %f\n", ratio, lux_idx);

  fKg = ratio * (pchromatix_CS_MCE->mce_config.green_boost_factor - 1.0f);
  ISP_DBG(ISP_MOD_MCE, "pre fKg =%f", fKg);
  if (fKg > 0) {
    /*add ratio for landscape severity  */
    fKg = ((fKg + 1) * (((float)landscape_severity / 255.0 *
    (landscape_green_boost_factor - 1)) + 1));

    max_boost = MAX(pchromatix_CS_MCE->mce_config.green_boost_factor,
      landscape_green_boost_factor);
    if (fKg > max_boost)
      fKg = max_boost;

    fKg = fKg - 1;
    QKg = (uint8_t)ceil(log(4.0f / fKg) / log(2.0f)) + 6;
    while ((int32_t)(fKg * (1 << QKg)) > 383)
      QKg--;
  } else {
    fKg = 0;
    QKg = 15;
  }
  ISP_DBG(ISP_MOD_MCE, "post fKg =%f", fKg);
  QKg = Clamp(QKg, 7, 15);

  /* Blue */
  ratio = 1.0f - isp_util_calc_interpolation_weight(lux_idx,
      pchromatix_CS_MCE->mce_config.blue_bright_index,
      pchromatix_CS_MCE->mce_config.blue_dark_index);
  fKb = ratio * (pchromatix_CS_MCE->mce_config.blue_boost_factor - 1.0f);
  ISP_DBG(ISP_MOD_MCE, "pre fKb =%f", fKb);
  if (fKb > 0) {
    /*add ratio for landscape severity */
    fKb = ((fKb + 1) * (((float)landscape_severity / 255.0 *
    (landscape_blue_boost_factor - 1)) + 1));

    max_boost = MAX(pchromatix_CS_MCE->mce_config.blue_boost_factor,
      landscape_blue_boost_factor);
    if (fKb > max_boost)
      fKb = max_boost;

    fKb = fKb - 1;
    QKb = (uint8_t)ceil(log(4.0f / fKb) / log(2.0f)) + 6;
    while ((int32_t)(fKb * (1 << QKb)) > 383)
      QKb--;
  } else {
    fKb = 0;
    QKb = 15;
  }
  ISP_DBG(ISP_MOD_MCE, "post fKb =%f", fKb);
  QKb = Clamp(QKb, 7, 15);

  /* Red */
  ratio = 1.0f - isp_util_calc_interpolation_weight(lux_idx,
      pchromatix_CS_MCE->mce_config.red_bright_index,
      pchromatix_CS_MCE->mce_config.red_dark_index);
  fKr = ratio * (pchromatix_CS_MCE->mce_config.red_boost_factor - 1.0f);
  ISP_DBG(ISP_MOD_MCE, "pre fKr =%f", fKr);
  if (fKr > 0) {
    /* add ratio for landscape severity */
    fKr = ((fKr + 1) * (((float)landscape_severity / 255.0 *
    (landscape_red_boost_factor - 1)) + 1));
    max_boost = MAX(pchromatix_CS_MCE->mce_config.red_boost_factor,
      landscape_red_boost_factor);
    if (fKr > max_boost)
      fKr = max_boost;

    fKr = fKr - 1;
    QKr = (uint8_t)ceil(log(4.0f / fKr) / log(2.0f)) + 6;
    while ((int32_t)(fKr * (1 << QKr)) > 383)
      QKr--;
  } else {
    fKr = 0;
    QKr = 15;
  }
  ISP_DBG(ISP_MOD_MCE, "post fKr =%f", fKr);
  QKr = Clamp(QKr, 7, 15);

  mce_mod->mce_mix_cmd_1.enable = mce_mod->mce_enable;
  /* Overall Q_K and gains */
  mce_mod->mce_mix_cmd_2.qk = MIN(MIN(QKg, QKb), QKr);
  mce_config->redCfg.K =
  (int32_t)(fKr * (1 << mce_mod->mce_mix_cmd_2.qk));
  mce_config->greenCfg.K =
  (int32_t)(fKg * (1 << mce_mod->mce_mix_cmd_2.qk));
  mce_config->blueCfg.K =
  (int32_t)(fKb * (1 << mce_mod->mce_mix_cmd_2.qk));

  /* Compute Y slopes */
  /* Green */

  dS1 = mce_config->greenCfg.y2 - mce_config->greenCfg.y1;
  dS3 = mce_config->greenCfg.y4 - mce_config->greenCfg.y3;

  if ((fKg > 0) && (dS1 > 0)) {
    Q_s1 = (int32_t)ceil(log(dS1 / fKg) / log(2.0f)) + 6;
    Q_s1 = Clamp(Q_s1, 7, 20);
    mce_config->greenCfg.yM1 = mce_config->greenCfg.K *
      (1 << (Q_s1 - mce_mod->mce_mix_cmd_2.qk)) / dS1;
  } else {
    Q_s1 = 20;
    mce_config->greenCfg.yM1 = 0;
  }

  mce_config->greenCfg.yS1 = Q_s1 - mce_mod->mce_mix_cmd_2.qk;
  if ((fKg > 0) && (dS3 > 0)) {
    Q_s3 = (int32_t)ceil(log(dS3 / fKg) / log(2.0f)) + 6;
    Q_s3 = Clamp(Q_s3, 7, 20);
    mce_config->greenCfg.yM3 = mce_config->greenCfg.K *
      (1 << (Q_s3 - mce_mod->mce_mix_cmd_2.qk)) / dS3;
  } else {
    Q_s3 = 20;
    mce_config->greenCfg.yM3 = 0;
  }

  mce_config->greenCfg.yS3 = Q_s3 - mce_mod->mce_mix_cmd_2.qk;
  /* Blue */
  dS1 = mce_config->blueCfg.y2 - mce_config->blueCfg.y1;
  dS3 = mce_config->blueCfg.y4 - mce_config->blueCfg.y3;

  if ((fKb > 0) && (dS1 > 0)) {
    Q_s1 = (int32_t)ceil(log(dS1 / fKb) / log(2.0f)) + 6;
    Q_s1 = Clamp(Q_s1, 7, 20);
    mce_config->blueCfg.yM1 = mce_config->blueCfg.K *
      (1 << (Q_s1 - mce_mod->mce_mix_cmd_2.qk)) / dS1;
  } else {
    Q_s1 = 20;
    mce_config->blueCfg.yM1 = 0;
  }
  mce_config->blueCfg.yS1 = Q_s1 - mce_mod->mce_mix_cmd_2.qk;

  if ((fKb > 0) && (dS3 > 0)) {
    Q_s3 = (int32_t)ceil(log(dS3 / fKb) / log(2.0f)) + 6;
    Q_s3 = Clamp(Q_s3, 7, 20);
    mce_config->blueCfg.yM3 = mce_config->blueCfg.K *
      (1 << (Q_s3 - mce_mod->mce_mix_cmd_2.qk)) / dS3;
  } else {
    Q_s3 = 20;
    mce_config->blueCfg.yM3 = 0;
  }

  mce_config->blueCfg.yS3 = Q_s3 - mce_mod->mce_mix_cmd_2.qk;
  /* Red */
  dS1 = mce_config->redCfg.y2 - mce_config->redCfg.y1;
  dS3 = mce_config->redCfg.y4 - mce_config->redCfg.y3;

  if ((fKr > 0) && (dS1 > 0)) {
    Q_s1 = (int32_t)ceil(log(dS1 / fKr) / log(2.0f)) + 6;
    Q_s1 = Clamp(Q_s1, 7, 20);
    mce_config->redCfg.yM1 = mce_config->redCfg.K *
      (1 << (Q_s1 - mce_mod->mce_mix_cmd_2.qk)) / dS1;
  } else {
    Q_s1 = 20;
    mce_config->redCfg.yM1 = 0;
  }
  mce_config->redCfg.yS1 = Q_s1 - mce_mod->mce_mix_cmd_2.qk;

  if ((fKr > 0) && (dS3 > 0)) {
    Q_s3 = (int32_t)ceil(log(dS3 / fKr) / log(2.0f)) + 6;
    Q_s3 = Clamp(Q_s3, 7, 20);
    mce_config->redCfg.yM3 = mce_config->redCfg.K *
      (1 << (Q_s3 - mce_mod->mce_mix_cmd_2.qk)) / dS3;
  } else {
    Q_s3 = 20;
    mce_config->redCfg.yM3 = 0;
  }
  mce_config->redCfg.yS3 = Q_s3 - mce_mod->mce_mix_cmd_2.qk;
  /* Compute C slopes */
  /* Green */
  mce_config->greenCfg.CBZone =
    pchromatix_CS_MCE->mce_config.green_cb_boundary;
  mce_config->greenCfg.CRZone =
    pchromatix_CS_MCE->mce_config.green_cr_boundary;
  mce_config->greenCfg.transWidth =
   (pchromatix_CS_MCE->mce_config.green_cb_transition_width +
   pchromatix_CS_MCE->mce_config.green_cr_transition_width) / 2;
  mce_config->greenCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_config->greenCfg.transWidth)) / log(2.0f) ) + 4;
  mce_config->greenCfg.transTrunc =
    Clamp(mce_config->greenCfg.transTrunc, 6, 9);
  mce_config->greenCfg.transSlope =
  (1 << mce_config->greenCfg.transTrunc) /
  mce_config->greenCfg.transWidth;
  /* Blue */
  mce_config->blueCfg.CBZone = pchromatix_CS_MCE->mce_config.blue_cb_boundary;
  mce_config->blueCfg.CRZone = pchromatix_CS_MCE->mce_config.blue_cr_boundary;
  mce_config->blueCfg.transWidth =
    (pchromatix_CS_MCE->mce_config.blue_cb_transition_width +
    pchromatix_CS_MCE->mce_config.blue_cr_transition_width) / 2;
  mce_config->blueCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_config->blueCfg.transWidth)) / log(2.0f)) + 4;
  mce_config->blueCfg.transTrunc =
  Clamp(mce_config->blueCfg.transTrunc, 6, 9);
  mce_config->blueCfg.transSlope =
  (1 << mce_config->blueCfg.transTrunc) /
  mce_config->blueCfg.transWidth;
  /* Red */
  mce_config->redCfg.CBZone =
    pchromatix_CS_MCE->mce_config.red_cb_boundary;
  mce_config->redCfg.CRZone =
  pchromatix_CS_MCE->mce_config.red_cr_boundary;
  mce_config->redCfg.transWidth =
    (pchromatix_CS_MCE->mce_config.red_cb_transition_width +
     pchromatix_CS_MCE->mce_config.red_cr_transition_width) / 2;
  mce_config->redCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_config->redCfg.transWidth)) / log(2.0f)) + 4;
  mce_config->redCfg.transTrunc =
  Clamp(mce_config->redCfg.transTrunc, 6, 9);
  mce_config->redCfg.transSlope =
    (1 << mce_config->redCfg.transTrunc) / mce_config->redCfg.transWidth;

  mce_mod->mce_update = TRUE;

  if (mce_mod->cnt == 0) {
    ISP_DBG(ISP_MOD_MCE, "MCE_landscape_severity = %d\n", landscape_severity);
    ISP_DBG(ISP_MOD_MCE, "MCE aec ratio = %f, aec_out->lux_idx %f\n", ratio, lux_idx);
    mce_mod->hw_update_pending = TRUE;
  }
  mce_mod->cnt++;
  if (mce_mod->cnt == 6)
    mce_mod->cnt = 0;
  return 0;
} /* mce_trigger_update */

/** mce_enable:
 *    @mce: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Enable module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int mce_enable(isp_mce_mod_t *mce, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mce->mce_enable = enable->enable;

  return 0;
}

/** mce_trigger_enable:
 *    @mce: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int mce_trigger_enable(isp_mce_mod_t *mce, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mce->mce_trigger_enable = enable->enable;

  return 0;
}

/** mce_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int mce_destroy(void *mod_ctrl)
{
  isp_mce_mod_t *mce = mod_ctrl;

  memset(mce, 0, sizeof(isp_mce_mod_t));
  free(mce);
  return 0;
}

/** mce_set_params:
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
static int mce_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_mce_mod_t *mce = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = mce_enable(mce, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = mce_config(mce, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = mce_trigger_enable(mce, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = mce_trigger_update(mce, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_BESTSHOT:
    rc = mce_config(mce, in_params, in_param_size);
    break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/** mce_ez_vfe_update
 *
 * description: update vfe params
 *
 **/
static void mce_ez_vfe_update(memorycolorenhancement_t *memcolorenhan,
  ISP_MCE_ConfigCmdType *mceCfg)
{
  memcolorenhan->red.y1           = mceCfg->redCfg.y1;
  memcolorenhan->red.y2           = mceCfg->redCfg.y2;
  memcolorenhan->red.y3           = mceCfg->redCfg.y3;
  memcolorenhan->red.y4           = mceCfg->redCfg.y4;
  memcolorenhan->red.yM1          = mceCfg->redCfg.yM1;
  memcolorenhan->red.yM3          = mceCfg->redCfg.yM3;
  memcolorenhan->red.yS1          = mceCfg->redCfg.yS1;
  memcolorenhan->red.yS3          = mceCfg->redCfg.yS3;
  memcolorenhan->red.transWidth   = mceCfg->redCfg.transWidth;
  memcolorenhan->red.transTrunc   = mceCfg->redCfg.transTrunc;
  memcolorenhan->red.crZone       = mceCfg->redCfg.CRZone;
  memcolorenhan->red.cbZone       = mceCfg->redCfg.CBZone;
  memcolorenhan->red.translope    = mceCfg->redCfg.transSlope;
  memcolorenhan->red.k            = mceCfg->redCfg.K;

  memcolorenhan->green.y1          = mceCfg->greenCfg.y1;
  memcolorenhan->green.y2          = mceCfg->greenCfg.y2;
  memcolorenhan->green.y3          = mceCfg->greenCfg.y3;
  memcolorenhan->green.y4          = mceCfg->greenCfg.y4;
  memcolorenhan->green.yM1         = mceCfg->greenCfg.yM1;
  memcolorenhan->green.yM3         = mceCfg->greenCfg.yM3;
  memcolorenhan->green.yS1         = mceCfg->greenCfg.yS1;
  memcolorenhan->green.yS3         = mceCfg->greenCfg.yS3;
  memcolorenhan->green.transWidth  = mceCfg->greenCfg.transWidth;
  memcolorenhan->green.transTrunc  = mceCfg->greenCfg.transTrunc;
  memcolorenhan->green.crZone      = mceCfg->greenCfg.CRZone;
  memcolorenhan->green.cbZone      = mceCfg->greenCfg.CBZone;
  memcolorenhan->green.translope   = mceCfg->greenCfg.transSlope;
  memcolorenhan->green.k           = mceCfg->greenCfg.K;

  memcolorenhan->blue.y1           = mceCfg->blueCfg.y1;
  memcolorenhan->blue.y2           = mceCfg->blueCfg.y2;
  memcolorenhan->blue.y3           = mceCfg->blueCfg.y3;
  memcolorenhan->blue.y4           = mceCfg->blueCfg.y4;
  memcolorenhan->blue.yM1          = mceCfg->blueCfg.yM1;
  memcolorenhan->blue.yM3          = mceCfg->blueCfg.yM3;
  memcolorenhan->blue.yS1          = mceCfg->blueCfg.yS1;
  memcolorenhan->blue.yS3          = mceCfg->blueCfg.yS3;
  memcolorenhan->blue.transWidth   = mceCfg->blueCfg.transWidth;
  memcolorenhan->blue.transTrunc   = mceCfg->blueCfg.transTrunc;
  memcolorenhan->blue.crZone       = mceCfg->blueCfg.CRZone;
  memcolorenhan->blue.cbZone       = mceCfg->blueCfg.CBZone;
  memcolorenhan->blue.translope    = mceCfg->blueCfg.transSlope;
  memcolorenhan->blue.k            = mceCfg->blueCfg.K;
}

/** mce_get_params:
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
static int mce_get_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_mce_mod_t *mce = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    enable->enable = mce->mce_enable;
    break;
  }

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    memorycolorenhancement_t *memcolorenhan = &vfe_diag->prev_memcolorenhan;
    if (mce->old_streaming_mode == CAM_STREAMING_MODE_BURST)
      memcolorenhan = &vfe_diag->snap_memcolorenhan;

    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    /*Populate vfe_diag data for example*/
    ISP_DBG(ISP_MOD_MCE, "%s: Populating vfe_diag data", __func__);
    vfe_diag->control_memcolorenhan.enable = mce->mce_enable;
    vfe_diag->control_memcolorenhan.cntrlenable = mce->mce_trigger_enable;
    memcolorenhan->qk = mce->mce_mix_cmd_2.qk;
    mce_ez_vfe_update(memcolorenhan, &mce->applied_mce_cmd);
  }
    break;

  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/** mce_do_hw_update:
 *    @mce_mod: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int mce_do_hw_update(isp_mce_mod_t *mce_mod)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[3];

  ISP_DBG(ISP_MOD_MCE, "%s: E: hw_update = %d\n", __func__, mce_mod->hw_update_pending);
  if (mce_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)&mce_mod->mce_cmd;
    cfg_cmd.cmd_len = sizeof(mce_mod->mce_cmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 3;

    /*write value into mixed register 1*/
    reg_cfg_cmd[0].u.mask_info.mask = mce_mod->mce_mix_cmd_1.mask;
    reg_cfg_cmd[0].u.mask_info.val = mce_mod->mce_mix_cmd_1.cfg;
    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_MCE32_CHORMA_SUPP_MIX_OFF1;

    /*write value into mixed register 2*/
    reg_cfg_cmd[1].u.mask_info.mask = mce_mod->mce_mix_cmd_2.mask;
    reg_cfg_cmd[1].u.mask_info.val = mce_mod->mce_mix_cmd_2.cfg;
    reg_cfg_cmd[1].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_MCE32_CHORMA_SUPP_MIX_OFF2;

    /* write value into regular register*/
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[2].cmd_type = VFE_WRITE;
    reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_MCE32_OFF;
    reg_cfg_cmd[2].u.rw_info.len = ISP_MCE32_LEN * sizeof(uint32_t);

    display_mce_config(mce_mod);
    rc = ioctl(mce_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    mce_mod->applied_mce_cmd = mce_mod->mce_cmd;
    mce_mod->hw_update_pending = 0;
  }

  return rc;
}

/** mce_action:
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
static int mce_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_mce_mod_t *mce = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = mce_do_hw_update(mce);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    break;
  }
  return rc;
}

/** mce_init:
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
static int mce_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_mce_mod_t *mce = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mce->fd = init_params->fd;
  mce->notify_ops = notify_ops;
  mce->mce_mix_cmd_1.hw_wr_mask = 0x1;
  mce->mce_mix_cmd_2.hw_wr_mask = 0xF;
  mce->cnt = 0;
  mce->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
}

/** mce32_open:
 *    @version: version of isp
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *mce32_open(uint32_t version)
{
  isp_mce_mod_t *mce = malloc(sizeof(isp_mce_mod_t));

  if (!mce) {
    /* no memory */
    CDBG_ERROR("%s: no mem", __func__);
    return NULL;
  }
  memset(mce, 0, sizeof(isp_mce_mod_t));
  mce->ops.ctrl = (void *)mce;
  mce->ops.init = mce_init;
  mce->ops.destroy = mce_destroy;
  mce->ops.set_params = mce_set_params;
  mce->ops.get_params = mce_get_params;
  mce->ops.action = mce_action;
  return &mce->ops;
}
