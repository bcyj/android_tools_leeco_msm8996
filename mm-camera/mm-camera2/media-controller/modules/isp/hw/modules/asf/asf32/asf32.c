/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "asf32.h"
#include "isp_log.h"

#ifdef ENABLE_ASF_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#ifndef sign
#define sign(x) (((x) < 0) ? (-1) : (1))
#endif

#ifndef Round
#define Round(x) (int)((x) + sign(x)*0.5)
#endif

/** util_asf_cmd_debug:
 *    @asfCmd: asf configuration
 *
 * Print ASF configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void util_asf_cmd_debug(ISP_AdaptiveFilterConfigCmdType *asfCmd)
{
  ISP_DBG(ISP_MOD_ASF, "%s: smoothFilterEnabled = %d", __func__, asfCmd->smoothFilterEnabled);
  ISP_DBG(ISP_MOD_ASF, "%s: sharpMode = %d, lpfMode = %d\n", __func__,
    asfCmd->sharpMode, asfCmd->lpfMode);
  ISP_DBG(ISP_MOD_ASF, "%s: smoothCoefCenter = %d smoothCoefSurr = %d\n", __func__,
    asfCmd->smoothCoefCenter, asfCmd->smoothCoefSurr);
  ISP_DBG(ISP_MOD_ASF, "%s: pipeClubCount = %d pipeClubOvd = %d flushHalt Ovd = %d\n", __func__,
    asfCmd->pipeFlushCount, asfCmd->pipeFlushOvd, asfCmd->flushHaltOvd);
  ISP_DBG(ISP_MOD_ASF, "%s: cropEnable = %d\n", __func__, asfCmd->cropEnable);
  ISP_DBG(ISP_MOD_ASF, "%s: sharpK1 = %d, sharpK2 = %d\n", __func__, asfCmd->sharpK1,
    asfCmd->sharpK2);
  ISP_DBG(ISP_MOD_ASF, "%s: normalizeFactor = %d\n", __func__, asfCmd->normalizeFactor);
  ISP_DBG(ISP_MOD_ASF, "%s: sharpThresholdE[1..5] = %d %d %d %d %d\n", __func__,
    asfCmd->sharpThreshE1, asfCmd->sharpThreshE2, asfCmd->sharpThreshE3,
    asfCmd->sharpThreshE4, asfCmd->sharpThreshE5);

  ISP_DBG(ISP_MOD_ASF, "%s: F1Coefficients[0-8]: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
    __func__, asfCmd->F1Coeff0, asfCmd->F1Coeff1, asfCmd->F1Coeff2,
    asfCmd->F1Coeff3, asfCmd->F1Coeff4, asfCmd->F1Coeff5,
    asfCmd->F1Coeff6, asfCmd->F1Coeff7, asfCmd->F1Coeff8);

  ISP_DBG(ISP_MOD_ASF, "%s: F2Coefficients[0-8]: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
    __func__, asfCmd->F2Coeff0, asfCmd->F2Coeff1, asfCmd->F2Coeff2,
    asfCmd->F2Coeff3, asfCmd->F2Coeff4, asfCmd->F2Coeff5,
    asfCmd->F2Coeff6, asfCmd->F2Coeff7, asfCmd->F2Coeff8);

  ISP_DBG(ISP_MOD_ASF, "%s: F3Coefficients[0-8]: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
    __func__, asfCmd->F3Coeff0, asfCmd->F3Coeff1, asfCmd->F3Coeff2,
  asfCmd->F3Coeff3, asfCmd->F3Coeff4, asfCmd->F3Coeff5,
    asfCmd->F3Coeff6, asfCmd->F3Coeff7, asfCmd->F3Coeff8);

  ISP_DBG(ISP_MOD_ASF, "%s: NZflag[0-7]: %d, %d, %d, %d, %d, %d, %d\n", __func__,
    asfCmd->nzFlag1, asfCmd->nzFlag2, asfCmd->nzFlag3, asfCmd->nzFlag4,
    asfCmd->nzFlag5, asfCmd->nzFlag6, asfCmd->nzFlag7);

} /* util_asf_config_print */

/** util_asf_settings_debug:
 *    @ip: asf settings
 *
 * Print ASF settings.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void util_asf_settings_debug(asf_setting_type *ip)
{
  ISP_DBG(ISP_MOD_ASF, "%s: lower_threshold = %d\n", __func__, ip->lower_threshold);
  ISP_DBG(ISP_MOD_ASF, "%s: upper_threshold = %d\n", __func__, ip->upper_threshold);
  ISP_DBG(ISP_MOD_ASF, "%s: negative_threshold = %d\n", __func__, ip->negative_threshold);
  ISP_DBG(ISP_MOD_ASF, "%s: upper_threshold_f2 = %d\n", __func__, ip->upper_threshold_f2);
  ISP_DBG(ISP_MOD_ASF, "%s: negative_threshold_f2 = %d\n", __func__, ip->negative_threshold_f2);
  ISP_DBG(ISP_MOD_ASF, "%s: sharpen_degree_f1 = %f\n", __func__, ip->sharpen_degree_f1);
  ISP_DBG(ISP_MOD_ASF, "%s: sharpen_degree_f2 = %f\n", __func__, ip->sharpen_degree_f2);
  ISP_DBG(ISP_MOD_ASF, "%s: smoothing_percent = %d\n", __func__, ip->smoothing_percent);
  ISP_DBG(ISP_MOD_ASF, "%s: smoothing_percent_5x5 = %d\n", __func__, ip->smoothing_percent_5x5);
} /* util_asf_settings_print */

/** util_asf_parm_interpolate:
 *    @ip1: asf settings 1
 *    @ip2: asf settings 2
 *    @out: outpu asf settings
 *    @ratio: interpolation ratio
 *
 * Interpolate ASF settings.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void util_asf_parm_interpolate(asf_setting_type *ip1,
  asf_setting_type *ip2, asf_setting_type *out, float ratio)
{
  out->lower_threshold =
    LINEAR_INTERPOLATION_INT(ip1->lower_threshold,
    ip2->lower_threshold, ratio);
  out->upper_threshold =
    LINEAR_INTERPOLATION_INT(ip1->upper_threshold,
    ip2->upper_threshold, ratio);
  out->negative_threshold =
    LINEAR_INTERPOLATION_INT(ip1->negative_threshold,
    ip2->negative_threshold, ratio);
  out->upper_threshold_f2 =
    LINEAR_INTERPOLATION_INT(ip1->upper_threshold_f2,
    ip2->upper_threshold_f2, ratio);
  out->negative_threshold_f2 =
    LINEAR_INTERPOLATION_INT(ip1->negative_threshold_f2,
    ip2->negative_threshold_f2, ratio);
  out->sharpen_degree_f1 =
    LINEAR_INTERPOLATION(ip1->sharpen_degree_f1,
    ip2->sharpen_degree_f1, ratio);
  out->sharpen_degree_f2 =
    LINEAR_INTERPOLATION(ip1->sharpen_degree_f2,
    ip2->sharpen_degree_f2, ratio);
  out->smoothing_percent =
    LINEAR_INTERPOLATION_INT(ip1->smoothing_percent,
    ip2->smoothing_percent, ratio);
  out->smoothing_percent_5x5 =
    LINEAR_INTERPOLATION_INT(ip1->smoothing_percent_5x5,
    ip2->smoothing_percent_5x5, ratio);
} /* util_asf_parm_interpolate */

/** util_asf_calc_5x5_lpf:
 *    @smoothing_percent: smoothing coefficient in percents
 *    @asfCmd: adf configuration
 *
 * Calculate 5x5 LPF coefficients
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void util_asf_calc_5x5_lpf(
  filter_smoothing_degree_type *smoothing_percent,
  ISP_AdaptiveFilterConfigCmdType *asfCmd)
{
  const int32_t LPFLen = 5;
  const int32_t center = 2;

  const double turningPt = 50.0; /* 50% */
  const double twoPi = 2.0 * 3.14159f;

  double F[LPFLen][LPFLen], sigma, radius2;
  double sumF = 0;

  int32_t F64[LPFLen][LPFLen], Diff64, sgn, aDiff, qDiff;
  int32_t sumF64 = 0;
  int32_t lpf_coeff[9] = { 0 };

  int32_t i, j, k, l, rc = TRUE;

  ISP_DBG(ISP_MOD_ASF, "%s: smoothening value = %d\n", __func__, *smoothing_percent);
  if (*smoothing_percent <= 0) {
    ISP_DBG(ISP_MOD_ASF, "%s: smoothening value <= 0, disable 5x5 LSF \n", __func__);
    rc = FALSE;
  } else if (*smoothing_percent > 100) {
    ISP_DBG(ISP_MOD_ASF, "%s: smoothening value > 100, cap it. \n", __func__);
    *smoothing_percent = 100;
  }

  /* Gaussian shaped low-pass filter */
  sigma = CameraExp((double)(*smoothing_percent)/turningPt) / CameraExp(1.0);

  for (i = 0; i < LPFLen; i++) {
    for (j = 0; j < LPFLen; j++) {
      k = i - center;
      l = j - center;
      radius2 = (double)(k * k + l * l);
      F[i][j] = CameraExp(-0.5 * radius2 / (sigma * sigma)) /
          (CameraSquareRoot(twoPi) * sigma);
      sumF += F[i][j];
    }
  }

  /*  Scale the sum to ~64 (Q6) */
  for (i = 0; i < LPFLen; i++) {
    for (j = 0; j < LPFLen; j++) {
      F64[i][j] = (int32_t)(FLOAT_TO_Q(6, F[i][j]) / sumF + 0.5);
      sumF64 += F64[i][j];
    }
  }

  /*  Tweak coefficients to make the sum 64 (Q6) */
  Diff64 = Q6 - sumF64;
  sgn = (Diff64 >= 0) ? 1 : -1;
  aDiff = sgn * Diff64;
  qDiff = sgn * (aDiff >> 2);

  if (aDiff >= 4) {
    if ((F64[1][1] + qDiff <= F64[2][1]) && (F64[1][1] + qDiff >= F64[2][0])) {
      F64[1][1] += qDiff;
      F64[1][3] += qDiff;
      F64[3][1] += qDiff;
      F64[3][3] += qDiff;
    } else if (F64[2][0] + qDiff >= F64[1][0]) {
      F64[2][0] += qDiff;
      F64[2][4] += qDiff;
      F64[0][2] += qDiff;
      F64[4][2] += qDiff;
    } else {
      F64[2][1] += qDiff;
      F64[2][3] += qDiff;
      F64[1][2] += qDiff;
      F64[3][2] += qDiff;
    }
    sumF64 += (qDiff << 2);
  }

  Diff64 -= (qDiff << 2);
  F64[2][2] += Diff64;
  sumF64 += Diff64;

  if (sumF64 != 64)
    ISP_DBG(ISP_MOD_ASF, "%s: sum of the lsf co-efficients is incorrect\n", __func__);
  if (*smoothing_percent == 0) {
    F64[0][0] = F64[0][1] = F64[0][2] = 0;
    F64[1][0] = F64[1][1] = F64[1][2] = 0;
    F64[2][0] = F64[2][1] = F64[2][2] = 0;
    F64[2][2] = Q6 - 1;  // 8uQ7
  }
  /* Update/Configure ASF input configuration */
  lpf_coeff[0] = MIN(31, MAX(-32, F64[0][0]));
  lpf_coeff[1] = MIN(31, MAX(-32, F64[0][1]));
  lpf_coeff[2] = MIN(31, MAX(-32, F64[0][2]));
  lpf_coeff[3] = MIN(31, MAX(-32, F64[1][0]));
  lpf_coeff[4] = MIN(31, MAX(-32, F64[1][1]));
  lpf_coeff[5] = MIN(31, MAX(-32, F64[1][2]));
  lpf_coeff[6] = MIN(31, MAX(-32, F64[2][0]));
  lpf_coeff[7] = MIN(31, MAX(-32, F64[2][1]));
  lpf_coeff[8] = MIN(63, MAX(-64, F64[2][2]));

end:
  if (rc != TRUE)
    asfCmd->lpfMode = FALSE;
  else
    asfCmd->lpfMode = TRUE;

  asfCmd->F3Coeff0 = lpf_coeff[0];
  asfCmd->F3Coeff1 = lpf_coeff[1];
  asfCmd->F3Coeff2 = lpf_coeff[2];
  asfCmd->F3Coeff3 = lpf_coeff[3];
  asfCmd->F3Coeff4 = lpf_coeff[4];
  asfCmd->F3Coeff5 = lpf_coeff[5];
  asfCmd->F3Coeff6 = lpf_coeff[6];
  asfCmd->F3Coeff7 = lpf_coeff[7];
  asfCmd->F3Coeff8 = lpf_coeff[8];
} /* util_asf_calc_5x5_lpf */

/** util_asf_calc_sharpness_scale:
 *    @sharp_info: sharpness info
 *    @chrPtr: pointer to chromatix data
 *
 * Calculate sharpness scale
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static float util_asf_calc_sharpness_scale(isp_sharpness_info_t *sharp_info,
  chromatix_parms_type *chrPtr)
{
  chromatix_ASF_5x5_type *ASF_5x5 = &chrPtr->chromatix_VFE.chromatix_ASF_5x5;
  /* default sharpness = 0, for upscale ratio great than MIN_DS_FACTOR */
  float hw_sharp_ctrl_factor = 0;
  float downscale_factor_from_scaler = sharp_info->downscale_factor;

  ISP_DBG(ISP_MOD_ASF, "%s: input sharp_ctrl_factor = %f, downscale_factor_from_scaler = %f",
    __func__, sharp_info->ui_sharp_ctrl_factor, downscale_factor_from_scaler);
  ISP_DBG(ISP_MOD_ASF, "%s: asd_soft_focus_dgr = %f, bst_soft_focus_dgr = %f\n",
    __func__, sharp_info->asd_soft_focus_dgr, sharp_info->bst_soft_focus_dgr);

  /* Note 1: Clamp the downscale_factor */
  if (downscale_factor_from_scaler > ASF_5x5->asf_5_5_sharp_max_ds_factor)
    downscale_factor_from_scaler = ASF_5x5->asf_5_5_sharp_max_ds_factor;

  if (downscale_factor_from_scaler < ASF_5x5->asf_5_5_sharp_min_ds_factor)
    downscale_factor_from_scaler = ASF_5x5->asf_5_5_sharp_min_ds_factor;

  /* Note 2: Now do the interpolate */
  if (downscale_factor_from_scaler >= 1.0) {
    hw_sharp_ctrl_factor = ASF_5x5->asf_5_5_sharp_max_factor
      * (ASF_5x5->asf_5_5_sharp_max_ds_factor - downscale_factor_from_scaler)
      / (ASF_5x5->asf_5_5_sharp_max_ds_factor - 1.0);
  } else {
    /* (asf_5_5_sharp_min_ds_factor) <= (vfectrl->downscale_factor) < 1 */
    /* if downscale ratio is smaller than 1,
     * interpolate between sharp_min_ds_factor */
    hw_sharp_ctrl_factor = (downscale_factor_from_scaler
      - ASF_5x5->asf_5_5_sharp_min_ds_factor)
      / (1.0 - ASF_5x5->asf_5_5_sharp_min_ds_factor);
  }

  hw_sharp_ctrl_factor *= sharp_info->ui_sharp_ctrl_factor *
      sharp_info->asd_soft_focus_dgr * sharp_info->bst_soft_focus_dgr;
  ISP_DBG(ISP_MOD_ASF, "%s: downscale_sharp_ctrl_factor = %f.\n",
    __func__, hw_sharp_ctrl_factor);

  return hw_sharp_ctrl_factor;
} /* util_asf_calc_sharpness_scale */

/** util_asf_threshold_and_degree_cmd_config:
 *    @mod: pointer to instance private data
 *    @chrPtr: pointer to chromatix data
 *    @asfSetting: asf settings
 *    @asfCmd: asf configuration (output)
 *    @asfPtr: asf chromatix data
 *
 * Set threshold and degree configuration
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void util_asf_threshold_and_degree_cmd_config(isp_asf_mod_t* mod,
  chromatix_parms_type *chrPtr, asf_setting_type *asfSetting,
  ISP_AdaptiveFilterConfigCmdType *asfCmd, chromatix_asf_5_5_type *asfPtr)
{
  unsigned int ps = 0;
  float sharp_ctrl_factor;

  util_asf_settings_debug(asfSetting);

  asfCmd->smoothFilterEnabled = TRUE;
  asfCmd->sharpMode = (ISP_ASF_ModeType)asfPtr->filter_mode;
  /* Adjust thresholds according to sharp filter mode */
  if (asfCmd->sharpMode == ASF_MODE_SMART_FILTER)
    asfCmd->normalizeFactor = 127;
  else
    asfCmd->normalizeFactor = 0;

  /* Convert chromatix smoothing_percent (0..100) to VFE ps (0..14),
   * chromatix smoothing_percent 100 is the smoothiest, whereas
   * VFE ps 0 is the smoothiest.
   */
  ISP_DBG(ISP_MOD_ASF, "%s: smoothing_percent = %d\n", __func__, asfSetting->smoothing_percent);
  ps = (unsigned int)((1.0 - (float)asfSetting->smoothing_percent / 100.0)
    * 14.0);
  asfCmd->smoothCoefCenter = 16 + (ps << 3);
  asfCmd->smoothCoefSurr = 14 - ps;
  asfCmd->pipeFlushOvd = 1;
  asfCmd->pipeFlushCount = 0x400;

  asfCmd->sharpThreshE1 = asfSetting->lower_threshold;
  asfCmd->sharpThreshE2 = asfSetting->upper_threshold;
  asfCmd->sharpThreshE3 = asfSetting->negative_threshold;
  asfCmd->sharpThreshE4 = asfSetting->upper_threshold_f2;
  asfCmd->sharpThreshE5 = asfSetting->negative_threshold_f2;

  sharp_ctrl_factor = Round(util_asf_calc_sharpness_scale(&(mod->out_sharpness_info),
    chrPtr));

  asfCmd->sharpK1 =
    FLOAT_TO_Q(3, asfSetting->sharpen_degree_f1 * sharp_ctrl_factor);
  asfCmd->sharpK2 =
    FLOAT_TO_Q(3, asfSetting->sharpen_degree_f2 * sharp_ctrl_factor);
} /* util_asf_threshold_and_degree_cmd_config */

/** util_asf_set_sharpness_parm:
 *    @in: input data
 *    @out: output data
 *
 * Fill sharpness parameters
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int util_asf_set_sharpness_parm(isp_sharpness_info_t *in,
  isp_sharpness_info_t *out)
{
  int rc = FALSE;

  ISP_DBG(ISP_MOD_ASF, "%s: In asd=%f, bst=%f, dcf=%f, port_sev=%f, ui_sharp=%f\n",
    __func__, in->asd_soft_focus_dgr, in->bst_soft_focus_dgr,
    in->downscale_factor, in->portrait_severity, in->ui_sharp_ctrl_factor * 3.9);
  ISP_DBG(ISP_MOD_ASF, "%s: Out asd=%f, bst=%f, dcf=%f, port_sev=%f, ui_sharp=%f\n",
    __func__, out->asd_soft_focus_dgr, out->bst_soft_focus_dgr,
    out->downscale_factor, out->portrait_severity, out->ui_sharp_ctrl_factor);

  if (!F_EQUAL(in->asd_soft_focus_dgr, out->asd_soft_focus_dgr)
    || !F_EQUAL(in->bst_soft_focus_dgr, out->bst_soft_focus_dgr)
    || !F_EQUAL(in->downscale_factor, out->downscale_factor)
    || !F_EQUAL(in->portrait_severity, out->portrait_severity)
    || !F_EQUAL(3.9*in->ui_sharp_ctrl_factor, out->ui_sharp_ctrl_factor)) {
    ISP_DBG(ISP_MOD_ASF, "%s: Update required\n", __func__);

    memcpy(out, in, sizeof(isp_sharpness_info_t));
    out->ui_sharp_ctrl_factor *= 3.9;
    out->ui_sharp_ctrl_factor = MIN(out->ui_sharp_ctrl_factor, 3.9);
    out->ui_sharp_ctrl_factor = MAX(out->ui_sharp_ctrl_factor, 0);
    rc = TRUE;
  }
  return rc;
} /* util_asf_set_sharpness_parm */

/** asf_init:
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
static int asf_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_asf_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
} /* asf_init */

/** asf_config:
 *    @mod: pointer to instance private data
 *    @pix_setting: input data
 *    @in_params_size: size of input data
 *
 * Configure module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_config(isp_asf_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int is_burst = IS_BURST_STREAMING(in_params);
  chromatix_parms_type *chrPtr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  ISP_AdaptiveFilterConfigCmdType *asfCmd = NULL;
  chromatix_asf_5_5_type *asfPtr = NULL;
  asf_setting_type *asfSetting = NULL;

  /* initialize module control */
  mod->enable = TRUE;
  mod->trigger_enable = TRUE;
  mod->sp_effect_HW_enable = FALSE;
  mod->sharpness_update = FALSE;

  /* calc main scaler downscale factor */
  mod->in_sharpness_info.downscale_factor = (float)
      (in_params->crop_info.y.pix_line.last_pixel -
    in_params->crop_info.y.pix_line.first_pixel + 1) /
      in_params->outputs[ISP_PIX_PATH_ENCODER].stream_param.width;

  /* initialize default settings */
  mod->param.data = chrPtr->chromatix_VFE.chromatix_ASF_5x5.asf_5_5;

  mod->param.settings = mod->param.data.setting[ASF_NORMAL_LIGHT];

  mod->aec_ratio.ratio = 0;
  mod->aec_ratio.lighting = TRIGGER_NORMAL;

  mod->in_sharpness_info.asd_soft_focus_dgr = 1.0;
  mod->in_sharpness_info.bst_soft_focus_dgr = 1.0;

  /* configure default values to the hw cmd */
  asfCmd = &(mod->RegCmd);
  asfPtr = &(mod->param.data);
  asfSetting = &(mod->param.settings);

  mod->sharpness_update =
    util_asf_set_sharpness_parm(&(mod->in_sharpness_info),
    &(mod->out_sharpness_info));

  util_asf_threshold_and_degree_cmd_config(mod, chrPtr, asfSetting, asfCmd, asfPtr);
  asfCmd->F1Coeff0 =
    FLOAT_TO_Q(4, asfPtr->filter1.a11 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff1 =
    FLOAT_TO_Q(4, asfPtr->filter1.a12 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff2 =
    FLOAT_TO_Q(4, asfPtr->filter1.a13 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff3 =
    FLOAT_TO_Q(4, asfPtr->filter1.a21 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff4 =
    FLOAT_TO_Q(4, asfPtr->filter1.a22 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff5 =
    FLOAT_TO_Q(4, asfPtr->filter1.a23 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff6 =
    FLOAT_TO_Q(4, asfPtr->filter1.a31 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff7 =
    FLOAT_TO_Q(4, asfPtr->filter1.a32 * asfPtr->normalize_factor1);
  asfCmd->F1Coeff8 =
    FLOAT_TO_Q(4, asfPtr->filter1.a33 * asfPtr->normalize_factor1);

  asfCmd->F2Coeff0 =
    FLOAT_TO_Q(4, asfPtr->filter2.a11 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff1 =
    FLOAT_TO_Q(4, asfPtr->filter2.a12 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff2 =
    FLOAT_TO_Q(4, asfPtr->filter2.a13 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff3 =
    FLOAT_TO_Q(4, asfPtr->filter2.a21 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff4 =
    FLOAT_TO_Q(4, asfPtr->filter2.a22 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff5 =
    FLOAT_TO_Q(4, asfPtr->filter2.a23 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff6 =
    FLOAT_TO_Q(4, asfPtr->filter2.a31 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff7 =
    FLOAT_TO_Q(4, asfPtr->filter2.a32 * asfPtr->normalize_factor2);
  asfCmd->F2Coeff8 =
    FLOAT_TO_Q(4, asfPtr->filter2.a33 * asfPtr->normalize_factor2);

  asfSetting->smoothing_percent_5x5 = (uint8_t)
    ((float)asfSetting->smoothing_percent_5x5 *
    (1 - (mod->out_sharpness_info.portrait_severity / 255.0)) +
    (mod->out_sharpness_info.asd_soft_focus_dgr *
    (mod->out_sharpness_info.portrait_severity / 255.0)));
  /* calculate 5x5 LPF co-efficients*/
  util_asf_calc_5x5_lpf(&(asfSetting->smoothing_percent_5x5), asfCmd);

  mod->hw_update_pending = TRUE;
  return 0;
} /* asf_config */

/** asf_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_destroy (void *mod_ctrl)
{
  isp_asf_mod_t *mod = mod_ctrl;

  memset(mod, 0, sizeof(isp_asf_mod_t));
  free(mod);
  return 0;
} /* asf_destroy */

/** asf_enable:
 *    @mod: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Enable module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_enable(isp_asf_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
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
} /* asf_enable */

/** asf_trigger_enable:
 *    @mod: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_trigger_enable(isp_asf_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->trigger_enable = enable->enable;
  return 0;
} /* asf_trigger_enable */

/** asf_trigger_update:
 *    @mod: pointer to instance private data
 *    @trigger_params: input data
 *    @in_params_size: size of input data
 *
 * Update configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_trigger_update(isp_asf_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{

  //trigger update
  chromatix_parms_type *chrPtr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_ASF_5x5_type *ASF_5x5 = &chrPtr->chromatix_VFE.chromatix_ASF_5x5;
  asf_setting_type *asf_normal, *asf_outdoor, *asf_lowlight;
  asf_setting_type *asf_final = NULL;
  ISP_AdaptiveFilterConfigCmdType *asfCmd = NULL;
  chromatix_asf_5_5_type *asfPtr = NULL;
  asf_setting_type *asfSetting = NULL;

  tuning_control_type tunning_control;
  trigger_ratio_t new_trigger_ratio;
  trigger_point_type *outdoor = NULL;
  trigger_point_type *lowlight = NULL;

  int status = 0;
  int is_burst = IS_BURST_STREAMING(&(in_params->cfg));

  if (!mod->enable) {
    ISP_DBG(ISP_MOD_ASF, "%s: ASF module is disabled. Skip the trigger update.\n", __func__);
    return 0;
  }

  if (!mod->trigger_enable) {
    ISP_DBG(ISP_MOD_ASF, "%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return 0;
  }

  mod->in_sharpness_info.asd_soft_focus_dgr = 1.0;
  mod->in_sharpness_info.bst_soft_focus_dgr = 1.0;

  asf_final = &(mod->param.settings);
  if (is_burst) {
    if (!isp_util_aec_check_settled(
      &(in_params->trigger_input.stats_update.aec_update))) {
      ISP_DBG(ISP_MOD_ASF, "%s: AEC is not setteled. Skip the trigger\n", __func__);
      return 0;
    }
  }
  ISP_DBG(ISP_MOD_ASF, "%s: aec update asf ratio.\n", __func__);
  outdoor = &(ASF_5x5->asf_5x5_outdoor_trigger);
  lowlight = &(ASF_5x5->asf_5x5_lowlight_trigger);
  asf_outdoor = &(ASF_5x5->asf_5_5.setting[ASF_BRIGHT_LIGHT]);
  asf_normal = &(ASF_5x5->asf_5_5.setting[ASF_NORMAL_LIGHT]);
  asf_lowlight = &(ASF_5x5->asf_5_5.setting[ASF_LOW_LIGHT]);

  tunning_control = ASF_5x5->control_asf_5X5;

  /* Decide the trigger ratio for current lighting condition */
  status = isp_util_get_aec_ratio2(mod->notify_ops->parent, tunning_control,
    outdoor, lowlight, &(in_params->trigger_input.stats_update.aec_update),
    is_burst, &new_trigger_ratio);

  if (status != 0) {
    CDBG_ERROR("%s: get aec ratio failed", __func__);
  }
  if ((mod->old_streaming_mode != in_params->cfg.streaming_mode) ||
      (new_trigger_ratio.lighting != mod->aec_ratio.lighting)    ||
      (!F_EQUAL(new_trigger_ratio.ratio, mod->aec_ratio.ratio))) {

    ISP_DBG(ISP_MOD_ASF, "%s: lighting = %d\n", __func__, new_trigger_ratio.lighting);
    switch (new_trigger_ratio.lighting) {
    case TRIGGER_NORMAL:
      *asf_final = *asf_normal;
      break;

    case TRIGGER_LOWLIGHT:
      util_asf_parm_interpolate(asf_normal, asf_lowlight, asf_final,
        new_trigger_ratio.ratio);
      break;

    case TRIGGER_OUTDOOR:
      util_asf_parm_interpolate(asf_normal, asf_outdoor, asf_final,
        new_trigger_ratio.ratio);
      break;
    default:
      CDBG_HIGH("%s: Invalid lighting condition.\n", __func__);
      return 0;
    }
    mod->old_streaming_mode = in_params->cfg.streaming_mode;
    mod->aec_ratio = new_trigger_ratio;
  } else {
    ISP_DBG(ISP_MOD_ASF, "%s: No update required.\n", __func__);
  }

  mod->sharpness_update = util_asf_set_sharpness_parm(&(mod->in_sharpness_info),
    &(mod->out_sharpness_info));

  if (mod->skip_trigger && !mod->sharpness_update) {
    ISP_DBG(ISP_MOD_ASF, "%s: No update required.\n", __func__);
    return 0;
  }

  asfCmd = &(mod->RegCmd);
  asfPtr = &(mod->param.data);
  asfSetting = &(mod->param.settings);

  //todo: adjust for snapshot
  util_asf_threshold_and_degree_cmd_config(mod, chrPtr, asfSetting, asfCmd, asfPtr);

  asfSetting->smoothing_percent_5x5 =
              (uint8_t)((float)asfSetting->smoothing_percent_5x5 *
              (1 - (mod->out_sharpness_info.portrait_severity / 255.0)) +
              (mod->out_sharpness_info.asd_soft_focus_dgr *
              (mod->out_sharpness_info.portrait_severity / 255.0)));

  // calculate 5x5 LPF co-efficients
  util_asf_calc_5x5_lpf(&(asfSetting->smoothing_percent_5x5), asfCmd);

  util_asf_cmd_debug(asfCmd);

  ISP_DBG(ISP_MOD_ASF, "%s: send the update to ASF HW\n", __func__);

  mod->hw_update_pending = TRUE;
  return 0;
} /* asf_trigger_update */

/** asf_set_bestshot:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set scene function. It is responsible to apply scene settings.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_set_bestshot(isp_asf_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  ISP_DBG(ISP_MOD_ASF, "%s:", __func__);

  return 0;
} /* asf_set_bestshot */

/** asf_set_effect:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set effect function. It is responsible to apply effect settings.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_set_effect(isp_asf_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  ISP_DBG(ISP_MOD_ASF, "%s:", __func__);

  return 0;
} /* asf_set_effect */

/** asf_set_sharpness:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set sharpness function. It is responsible to apply sharpness settings.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_set_sharpness(isp_asf_mod_t *mod,
  int *in_params, uint32_t in_param_size)
{
  int sharpness = *(int *)in_params;
  float sharpness_f = (float) sharpness;

  ISP_DBG(ISP_MOD_ASF, "%s: %d\n", __func__, sharpness);

  mod->in_sharpness_info.ui_sharp_ctrl_factor = sharpness_f / (36 - 0);
  ISP_DBG(ISP_MOD_ASF, "%s: %f\n", __func__, mod->in_sharpness_info.ui_sharp_ctrl_factor);
  return 0;
} /* asf_set_effect */


/** asf_set_params:
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
static int asf_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_asf_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = asf_enable(mod, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = asf_config(mod, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = asf_trigger_enable(mod, (isp_mod_set_enable_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = asf_trigger_update(mod, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_BESTSHOT:
    rc = asf_set_bestshot(mod, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_EFFECT:
    rc = asf_set_effect(mod, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_SHARPNESS_FACTOR:
    rc = asf_set_sharpness(mod, in_params, in_param_size);
    break;
  default:
    return -EAGAIN;
    break;
  }
  return rc;
} /* asf_set_params */
/** asf_ez_isp_update
 * @asf: asf module pointer
 * @asfDiag: asf_Diag pointer
 *
 **/
static void asf_ez_isp_update(
  isp_asf_mod_t *asf, asfsharpness5x5_t *asfDiag)
{
  ISP_AdaptiveFilterConfigCmdType *asfCfg =
    &asf->RegCmd;
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
/** asf_get_params:
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
static int asf_get_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_asf_mod_t *mod = mod_ctrl;
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
    break;
  }

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    asfsharpness5x5_t *asf_diag;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    asf_diag = &(vfe_diag->prev_asf5x5);
    if (mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
      asf_diag = &(vfe_diag->snap_asf5x5);
    }
    vfe_diag->control_asf5x5.enable = mod->enable;
    vfe_diag->control_asf5x5.cntrlenable = mod->trigger_enable;
    asf_ez_isp_update(mod, asf_diag);
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_ASF, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default:
    rc = -1;
    break;
  }
  return rc;
} /* asf_get_params */

/** asf_do_hw_update:
 *    @abf_mod: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int asf_do_hw_update(isp_asf_mod_t *asf_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (asf_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)&asf_mod->RegCmd;
    cfg_cmd.cmd_len = sizeof(asf_mod->RegCmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_ASF_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_ASF_LEN * sizeof(uint32_t);

    rc = ioctl(asf_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 48;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = 0x000005FC;
    reg_cfg_cmd[0].u.rw_info.len = sizeof(uint32_t);
    rc = ioctl(asf_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    memcpy(&asf_mod->applied_RegCmd, &asf_mod->RegCmd,
      sizeof(ISP_AdaptiveFilterConfigCmdType));
    asf_mod->hw_update_pending = 0;
  }

  return rc;
} /* asf_do_hw_update */

/** asf_action:
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
static int asf_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_asf_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = asf_do_hw_update(mod);
    break;
  default:
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
    break;
  }
  return rc;
} /* asf_action */

/** asf32_open:
 *    @version: version of isp
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *asf32_open(uint32_t version)
{
  isp_asf_mod_t *mod = malloc(sizeof(isp_asf_mod_t));

  ISP_DBG(ISP_MOD_ASF, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory", __func__);
    return NULL;
  }
  memset(mod, 0, sizeof(isp_asf_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = asf_init;
  mod->ops.destroy = asf_destroy;
  mod->ops.set_params = asf_set_params;
  mod->ops.get_params = asf_get_params;
  mod->ops.action = asf_action;

  return &mod->ops;
} /* asf40_open */

