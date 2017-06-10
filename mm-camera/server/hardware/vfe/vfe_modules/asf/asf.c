/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_ASF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

typedef enum VFE_ASF_ModeType {
  ASF_MODE_NONE,
  ASF_MODE_SINGLE_FILTER,
  ASF_MODE_DUAL_FILTER,
  ASF_MODE_SMART_FILTER,
  ASF_MODE_ENUM = ASF_MODE_SMART_FILTER,
} VFE_ASF_ModeType;

/*===========================================================================
 * Function:               asf_config_print
 *
 * Description:
 *=========================================================================*/
static void asf_config_print(VFE_AdaptiveFilterConfigCmdType *asfCmd)
{
  CDBG("%s: smoothFilterEnabled = %d\n", __func__, asfCmd->smoothFilterEnabled);
  CDBG("%s: sharpMode = %d\n", __func__, asfCmd->sharpMode);
#ifndef VFE_2X
  CDBG("%s: lpfMode = %d smoothCoefCenter = %d smoothCoefSurr = %d\n", __func__,
    asfCmd->lpfMode, asfCmd->smoothCoefCenter, asfCmd->smoothCoefSurr);
  CDBG("%s: pipeClubCount = %d pipeClubOvd = %d flushHalt Ovd = %d\n", __func__,
    asfCmd->pipeFlushCount, asfCmd->pipeFlushOvd, asfCmd->flushHaltOvd);
  CDBG("%s: cropEnable = %d\n", __func__, asfCmd->cropEnable);
#endif
  CDBG("%s: sharpK1 = %d, sharpK2 = %d\n", __func__, asfCmd->sharpK1,
    asfCmd->sharpK2);
  CDBG("%s: normalizeFactor = %d\n", __func__, asfCmd->normalizeFactor);
  CDBG("%s: sharpThresholdE[1..5] = %d %d %d %d %d\n", __func__,
    asfCmd->sharpThreshE1, asfCmd->sharpThreshE2, asfCmd->sharpThreshE3,
    asfCmd->sharpThreshE4, asfCmd->sharpThreshE5);

  CDBG("%s: F1Coefficients: %d, %d, %d\n", __func__, asfCmd->F1Coeff0,
    asfCmd->F1Coeff1, asfCmd->F1Coeff2);
  CDBG("%s: F1Coefficients: %d, %d, %d\n", __func__, asfCmd->F1Coeff3,
    asfCmd->F1Coeff4, asfCmd->F1Coeff5);
  CDBG("%s: F1Coefficients: %d, %d, %d\n", __func__, asfCmd->F1Coeff6,
    asfCmd->F1Coeff7, asfCmd->F1Coeff8);

  CDBG("%s: F2Coefficients: %d, %d, %d\n", __func__, asfCmd->F2Coeff0,
    asfCmd->F2Coeff1, asfCmd->F2Coeff2);
  CDBG("%s: F2Coefficients: %d, %d, %d\n", __func__, asfCmd->F2Coeff3,
    asfCmd->F2Coeff4, asfCmd->F2Coeff5);
  CDBG("%s: F2Coefficients: %d, %d, %d\n", __func__, asfCmd->F2Coeff6,
    asfCmd->F2Coeff7, asfCmd->F2Coeff8);
#ifndef VFE_2X
  CDBG("%s: F3Coefficients: %d, %d, %d\n", __func__, asfCmd->F3Coeff0,
    asfCmd->F3Coeff1, asfCmd->F3Coeff2);
  CDBG("%s: F3Coefficients: %d, %d, %d\n", __func__, asfCmd->F3Coeff3,
    asfCmd->F3Coeff4, asfCmd->F3Coeff5);
  CDBG("%s: F3Coefficients: %d, %d, %d\n", __func__, asfCmd->F3Coeff6,
    asfCmd->F3Coeff7, asfCmd->F3Coeff8);
#endif
#ifdef VFE_32
  CDBG("%s: NZflag 1, 2 , 3: %d, %d, %d\n", __func__, asfCmd->nzFlag1,
    asfCmd->nzFlag2, asfCmd->nzFlag3);
  CDBG("%s: NZflag 4, 5 , 6,  7: %d, %d, %d, %d\n", __func__, asfCmd->nzFlag4,
    asfCmd->nzFlag5, asfCmd->nzFlag6, asfCmd->nzFlag7);
#endif
} /* asf_config_print */

/*===========================================================================
 * Function:               asf_settings_print
 *
 * Description:
 *=========================================================================*/
static void asf_settings_print(asf_setting_type *ip)
{
  CDBG("%s: lower_threshold = %d\n", __func__, ip->lower_threshold);
  CDBG("%s: upper_threshold = %d\n", __func__, ip->upper_threshold);
  CDBG("%s: negative_threshold = %d\n", __func__, ip->negative_threshold);
  CDBG("%s: upper_threshold_f2 = %d\n", __func__, ip->upper_threshold_f2);
  CDBG("%s: negative_threshold_f2 = %d\n", __func__, ip->negative_threshold_f2);
  CDBG("%s: sharpen_degree_f1 = %f\n", __func__, ip->sharpen_degree_f1);
  CDBG("%s: sharpen_degree_f2 = %f\n", __func__, ip->sharpen_degree_f2);
  CDBG("%s: smoothing_percent = %d\n", __func__, ip->smoothing_percent);
  CDBG("%s: smoothing_percent_5x5 = %d\n", __func__, ip->smoothing_percent_5x5);
} /* asf_settings_print */

/*===========================================================================
 * Function:               asf_calc_5x5_lpf
 *
 * Description:
 *=========================================================================*/
static void asf_calc_5x5_lpf(filter_smoothing_degree_type *smoothing_percent,
  VFE_AdaptiveFilterConfigCmdType *asfCmd)
{
#ifndef VFE_2X
  const int32_t LPFLen = 5;
  const int32_t center = 2;

  const double turningPt = 50.0; /* 50% */
  const double twoPi = 2.0 * 3.14159f;

  double F[LPFLen][LPFLen], sigma, radius2;
  double sumF = 0;

  int32_t F64[LPFLen][LPFLen], Diff64, sgn, aDiff, qDiff;
  int32_t sumF64 = 0;
  int32_t lpf_coeff[9] = {0};

  int32_t i, j, k, l, rc = TRUE;

  CDBG("%s: smoothening value = %d\n", __func__, *smoothing_percent);
  if (*smoothing_percent <= 0) {
    CDBG("%s: smoothening value <= 0, disable 5x5 LSF \n", __func__);
    rc = FALSE;
    goto end;
  } else if (*smoothing_percent > 100) {
    CDBG("%s: smoothening value > 100, cap it. \n", __func__);
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
  sgn = (Diff64 >= 0)? 1: -1;
  aDiff  = sgn * Diff64;
  qDiff  = sgn * (aDiff >> 2);

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
    CDBG("%s: sum of the lsf co-efficients is incorrect\n", __func__);

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
#endif
} /* asf_calc_5x5_lpf */

/*===========================================================================
 * FUNCTION    - asf_calc_sharpness_scale -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float asf_calc_sharpness_scale(vfe_sharpness_info_t *sharp_info,
  chromatix_parms_type *chrPtr)
{
  /* default sharpness = 0, for upscale ratio great than MIN_DS_FACTOR */
  float hw_sharp_ctrl_factor = 0;
  float downscale_factor_from_scaler = sharp_info->downscale_factor;

  CDBG("%s: input sharp_ctrl_factor = %f, downscale_factor_from_scaler = %f",
    __func__, sharp_info->ui_sharp_ctrl_factor, downscale_factor_from_scaler);
  CDBG("%s: asd_soft_focus_dgr = %f, bst_soft_focus_dgr = %f\n", __func__,
    sharp_info->asd_soft_focus_dgr, sharp_info->bst_soft_focus_dgr);

  /* Note 1: Clamp the downscale_factor */
  if (downscale_factor_from_scaler > chrPtr->asf_5_5_sharp_max_ds_factor)
    downscale_factor_from_scaler = chrPtr->asf_5_5_sharp_max_ds_factor;

  if (downscale_factor_from_scaler < chrPtr->asf_5_5_sharp_min_ds_factor)
    downscale_factor_from_scaler = chrPtr->asf_5_5_sharp_min_ds_factor;

  /* Note 2: Now do the interpolate */
  if (downscale_factor_from_scaler >= 1.0)
    hw_sharp_ctrl_factor = chrPtr->asf_5_5_sharp_max_factor *
      (chrPtr->asf_5_5_sharp_max_ds_factor - downscale_factor_from_scaler) /
      (chrPtr->asf_5_5_sharp_max_ds_factor - 1.0);
  else
    /* (asf_5_5_sharp_min_ds_factor) <= (vfectrl->downscale_factor) < 1 */
    /* if downscale ratio is smaller than 1,
     * interpolate between sharp_min_ds_factor */
    hw_sharp_ctrl_factor =
      (downscale_factor_from_scaler - chrPtr->asf_5_5_sharp_min_ds_factor) /
      (1.0 - chrPtr->asf_5_5_sharp_min_ds_factor);

  hw_sharp_ctrl_factor *= sharp_info->ui_sharp_ctrl_factor *
    sharp_info->asd_soft_focus_dgr * sharp_info->bst_soft_focus_dgr;
  CDBG("%s: downscale_sharp_ctrl_factor = %f.\n", __func__,
    hw_sharp_ctrl_factor);

  return hw_sharp_ctrl_factor;
} /* asf_calc_sharpness_scale */

/*===========================================================================
 * FUNCTION    - asf_threshold_and_degree_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void asf_threshold_and_degree_update(chromatix_parms_type *chrPtr,
  asf_setting_type *asfSetting, VFE_AdaptiveFilterConfigCmdType *asfCmd,
  chromatix_asf_5_5_type *asfPtr, asf_mod_t* asf_ctrl)
{
  unsigned int ps = 0;
  float sharp_ctrl_factor;

  asf_settings_print(asfSetting);

  asfCmd->smoothFilterEnabled = TRUE;
  asfCmd->sharpMode = (VFE_ASF_ModeType)asfPtr->filter_mode;
  /* Adjust thresholds according to sharp filter mode */
  if (asfCmd->sharpMode == ASF_MODE_SMART_FILTER)
    asfCmd->normalizeFactor = 127;
  else
    asfCmd->normalizeFactor = 0;

  /* Convert chromatix smoothing_percent (0..100) to VFE ps (0..14),
   * chromatix smoothing_percent 100 is the smoothiest, whereas
   * VFE ps 0 is the smoothiest.
   */
  CDBG("%s: smoothing_percent = %d\n", __func__, asfSetting->smoothing_percent);
  ps =
    (unsigned int)((1.0 - (float)asfSetting->smoothing_percent / 100.0) * 14.0);
  asfCmd->smoothCoefCenter = 16 + ( ps << 3 );
  asfCmd->smoothCoefSurr = 14 - ps;
#ifndef VFE_2X
  asfCmd->pipeFlushOvd = 1;
  asfCmd->pipeFlushCount = 0x400;
#endif

  asfCmd->sharpThreshE1 = asfSetting->lower_threshold;
  asfCmd->sharpThreshE2 = asfSetting->upper_threshold;
  asfCmd->sharpThreshE3 = asfSetting->negative_threshold;
  asfCmd->sharpThreshE4 = asfSetting->upper_threshold_f2;
  asfCmd->sharpThreshE5 = asfSetting->negative_threshold_f2;

  sharp_ctrl_factor =
    asf_calc_sharpness_scale(&(asf_ctrl->asf_sharpness_data), chrPtr);

  asfCmd->sharpK1 =
    FLOAT_TO_Q(3, asfSetting->sharpen_degree_f1 * sharp_ctrl_factor);
  asfCmd->sharpK2 =
    FLOAT_TO_Q(3, asfSetting->sharpen_degree_f2 * sharp_ctrl_factor);
} /* asf_threshold_and_degree_update */

/*===========================================================================
 * FUNCTION    - asf_parm_interpolate -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void asf_parm_interpolate(asf_setting_type *ip1, asf_setting_type *ip2,
  asf_setting_type *out, float ratio)
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
} /* asf_parm_interpolate */

/*===========================================================================
 * FUNCTION    - asf_set_sharpness_parm -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int asf_set_sharpness_parm(vfe_sharpness_info_t *in,
  vfe_sharpness_info_t *out)
{
  int rc = FALSE;

  CDBG("%s: In asd=%f, bst=%f, dcf=%f, port_sev=%f, ui_sharp=%f\n", __func__,
    in->asd_soft_focus_dgr, in->bst_soft_focus_dgr, in->downscale_factor,
    in->portrait_severity, in->ui_sharp_ctrl_factor * 3.9);
  CDBG("%s: Out asd=%f, bst=%f, dcf=%f, port_sev=%f, ui_sharp=%f\n", __func__,
    out->asd_soft_focus_dgr, out->bst_soft_focus_dgr, out->downscale_factor,
    out->portrait_severity, out->ui_sharp_ctrl_factor);

  if (!F_EQUAL(in->asd_soft_focus_dgr, out->asd_soft_focus_dgr) ||
    !F_EQUAL(in->bst_soft_focus_dgr, out->bst_soft_focus_dgr) ||
    !F_EQUAL(in->downscale_factor, out->downscale_factor) ||
    !F_EQUAL(in->portrait_severity, out->portrait_severity) ||
    !F_EQUAL(3.9*in->ui_sharp_ctrl_factor, out->ui_sharp_ctrl_factor)) {
    CDBG("%s: Update required\n", __func__);

    memcpy(out, in, sizeof(vfe_sharpness_info_t));
    out->ui_sharp_ctrl_factor *= 3.9;
    out->ui_sharp_ctrl_factor = MIN(out->ui_sharp_ctrl_factor, 3.9);
    out->ui_sharp_ctrl_factor = MAX(out->ui_sharp_ctrl_factor, 0);
    rc = TRUE;
  }

  return rc;
} /* asf_set_sharpness_parm */

/*=============================================================================
 * Function:               vfe_asf_config
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_asf_config(int mod_id, void *asf_mod, void *params)
{
  int32_t lpf_coeff[9] = {0};
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;
  chromatix_parms_type *chrPtr = NULL;
  VFE_AdaptiveFilterConfigCmdType *asfCmd = NULL;
  chromatix_asf_5_5_type *asfPtr = NULL;
  asf_setting_type *asfSetting = NULL;

  if (!asf_ctrl->asf_enable) {
    CDBG("%s: ASF module is disabled. Skip the config.\n", __func__);
    return VFE_SUCCESS;
  }
  asf_ctrl->asf_sharpness_update =
    asf_set_sharpness_parm(&(vfe_params->sharpness_info),
      &(asf_ctrl->asf_sharpness_data));

  chrPtr = vfe_params->chroma3a;
  if (IS_SNAP_MODE(vfe_params)) {
    if (!asf_ctrl->asf_trigger_enable) {
      CDBG("%s: Snapshot should have the same config as Preview\n", __func__);
      asfCmd = &(asf_ctrl->asf_prev_cmd);
      goto hw_cmd_send;
    }
#ifndef VFE_2X
    if (!asf_ctrl->asf_trigger_update)
      CDBG_HIGH("%s: Trigger should be valid before snapshot config is called."
        " Disabling ASF for snapshot\n", __func__);
#endif
    asfCmd = &(asf_ctrl->asf_snap_cmd);
    asfPtr = &(asf_ctrl->asf_snap_param.data);
    asfSetting = &(asf_ctrl->asf_snap_param.settings);
  } else {
    asfCmd = &(asf_ctrl->asf_prev_cmd);
    asfPtr = &(asf_ctrl->asf_prev_param.data);
    asfSetting = &(asf_ctrl->asf_prev_param.settings);
  }

  if (vfe_params->effects_params.spl_effect != CAMERA_EFFECT_EMBOSS &&
    vfe_params->effects_params.spl_effect != CAMERA_EFFECT_SKETCH &&
    vfe_params->effects_params.spl_effect != CAMERA_EFFECT_NEON) {

    asf_threshold_and_degree_update(chrPtr, asfSetting, asfCmd, asfPtr, asf_ctrl);

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
      (1 - (asf_ctrl->asf_sharpness_data.portrait_severity / 255.0)) +
      (asf_ctrl->asf_sharpness_data.asd_soft_focus_dgr *
      (asf_ctrl->asf_sharpness_data.portrait_severity / 255.0)));
    /* calculate 5x5 LPF co-efficients*/
    asf_calc_5x5_lpf(&(asfSetting->smoothing_percent_5x5), asfCmd);
  }else {
    if (VFE_SUCCESS != vfe_asf_SP_effect_snapshot_adjust(mod_id, asf_ctrl,
      vfe_params, vfe_params->effects_params.spl_effect)){
      CDBG_HIGH("%s: asf adjust K1 by special effect failed",__func__);
      return VFE_ERROR_GENERAL;
    }
    if(IS_SNAP_MODE(vfe_params))
      asfCmd->sharpK1 = asf_ctrl->asf_snap_cmd.sharpK1;
    CDBG("%s: linear scaled K1 by SP_EFFECT= %d", __func__, asfCmd->sharpK1);
  }

hw_cmd_send:
  asf_config_print(asfCmd);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) asfCmd, sizeof(VFE_AdaptiveFilterConfigCmdType),
    VFE_CMD_ASF_CFG)) {
    CDBG_HIGH("%s: asf config for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}/* vfe_asf_config */

/*=============================================================================
 * Function:               vfe_asf_trigger_update
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_asf_trigger_update(int mod_id, void *asf_mod, void *vparams)
{
  chromatix_parms_type *chrPtr = NULL;
  asf_setting_type *asf_normal, *asf_outdoor, *asf_lowlight;
  asf_setting_type *asf_final = NULL;
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;

  tuning_control_type tunning_control;
  trigger_ratio_t new_trigger_ratio;
  trigger_point_type *outdoor = NULL;
  trigger_point_type *lowlight = NULL;

  static vfe_op_mode_t cur_mode = VFE_OP_MODE_INVALID;

  asf_ctrl->asf_trigger_update = FALSE;

  if (!asf_ctrl->asf_enable) {
    CDBG("%s: ASF module is disabled. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (!asf_ctrl->asf_trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  chrPtr = vfe_params->chroma3a;

  if (IS_SNAP_MODE(vfe_params)) {
    asf_final = &(asf_ctrl->asf_snap_param.settings);

    CDBG("%s: In snapshot, aec update asf ratio.\n", __func__);
    outdoor = &(chrPtr->asf_5x5_outdoor_trigger);
    lowlight = &(chrPtr->asf_5x5_lowlight_trigger);
    asf_outdoor = &(chrPtr->asf_5_5.setting[ASF_BRIGHT_LIGHT]);
    asf_normal = &(chrPtr->asf_5_5.setting[ASF_NORMAL_LIGHT]);
    asf_lowlight = &(chrPtr->asf_5_5.setting[ASF_LOW_LIGHT]);
  } else {
    asf_final = &(asf_ctrl->asf_prev_param.settings);

    if (!vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
      if (!asf_ctrl->asf_reload_params) {
        CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
        return VFE_SUCCESS;
      }
    }
    CDBG("%s: In preview, aec update asf ratio.\n", __func__);
    outdoor = &(chrPtr->asf_3x3_outdoor_trigger);
    lowlight = &(chrPtr->asf_3x3_lowlight_trigger);
    asf_outdoor = &(chrPtr->asf_5_5_preview.setting[ASF_BRIGHT_LIGHT]);
    asf_normal = &(chrPtr->asf_5_5_preview.setting[ASF_NORMAL_LIGHT]);
    asf_lowlight = &(chrPtr->asf_5_5_preview.setting[ASF_LOW_LIGHT]);
  }

  tunning_control = chrPtr->control_asf_5X5;

  /* Decide the trigger ratio for current lighting condition */
  new_trigger_ratio = vfe_util_get_aec_ratio2(tunning_control, outdoor,
    lowlight, vfe_params);

  if (cur_mode != vfe_params->vfe_op_mode || asf_ctrl->asf_reload_params ||
    new_trigger_ratio.lighting != asf_ctrl->asf_trigger_ratio.lighting ||
    !F_EQUAL(new_trigger_ratio.ratio, asf_ctrl->asf_trigger_ratio.ratio)) {

    CDBG("%s: lighting = %d\n", __func__, new_trigger_ratio.lighting);
    switch (new_trigger_ratio.lighting) {
      case TRIGGER_NORMAL:
        *asf_final = *asf_normal;
        break;

      case TRIGGER_LOWLIGHT:
        asf_parm_interpolate(asf_normal, asf_lowlight, asf_final,
          new_trigger_ratio.ratio);
        break;

      case TRIGGER_OUTDOOR:
        asf_parm_interpolate(asf_normal, asf_outdoor, asf_final,
          new_trigger_ratio.ratio);
        break;
      default:
        CDBG_HIGH("%s: Invalid lighting condition.\n", __func__);
        return VFE_SUCCESS;
    }
    cur_mode = vfe_params->vfe_op_mode;
    asf_ctrl->asf_trigger_ratio = new_trigger_ratio;
    asf_ctrl->asf_trigger_update = TRUE;
    asf_ctrl->asf_reload_params = FALSE;
  } else {
    CDBG("%s: No update required.\n", __func__);
  }
  return VFE_SUCCESS;
} /* vfe_asf_trigger_update */

/*=============================================================================
 * Function:               vfe_asf_update
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_asf_update(int mod_id, void *asf_mod, void *params)
{
  unsigned int ps = 0;
  chromatix_parms_type *chrPtr = NULL;
  VFE_AdaptiveFilterConfigCmdType *asfCmd = NULL;
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;
  chromatix_asf_5_5_type *asfPtr = NULL;
  asf_setting_type *asfSetting = NULL;

  if (asf_ctrl->hw_enable_cmd) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)vfe_params->moduleCfg, sizeof(vfe_params->moduleCfg),
      VFE_CMD_MODULE_CFG)) {
      CDBG_HIGH("%s: Module config failed\n", __func__);
      return VFE_ERROR_GENERAL;
    }
    asf_ctrl->hw_enable_cmd = FALSE;
  }

  if (!asf_ctrl->asf_enable) {
    CDBG("%s: ASF module is disabled. Skip the config.\n", __func__);
    return VFE_SUCCESS;
  }

  if (asf_ctrl->asf_sp_effect_HW_enable) {
    asf_ctrl->asf_trigger_update = TRUE;
  }

  asf_ctrl->asf_sharpness_update =
    asf_set_sharpness_parm(&(vfe_params->sharpness_info),
      &(asf_ctrl->asf_sharpness_data));

  if (!asf_ctrl->asf_trigger_update && !asf_ctrl->asf_sharpness_update) {
    CDBG("%s: No update required.\n", __func__);
    return VFE_SUCCESS;
  }

  chrPtr = vfe_params->chroma3a;
  if (IS_SNAP_MODE(vfe_params)) {
    CDBG_HIGH("%s: Should not come here\n", __func__);
    asfCmd = &(asf_ctrl->asf_snap_cmd);
    asfPtr = &(asf_ctrl->asf_snap_param.data);
    asfSetting = &(asf_ctrl->asf_snap_param.settings);
  } else {
    asfCmd = &(asf_ctrl->asf_prev_cmd);
    asfPtr = &(asf_ctrl->asf_prev_param.data);
    asfSetting = &(asf_ctrl->asf_prev_param.settings);
  }

  if (vfe_params->effects_params.spl_effect != CAMERA_EFFECT_EMBOSS &&
    vfe_params->effects_params.spl_effect != CAMERA_EFFECT_SKETCH &&
    vfe_params->effects_params.spl_effect != CAMERA_EFFECT_NEON) {

    asf_threshold_and_degree_update(chrPtr, asfSetting, asfCmd, asfPtr, asf_ctrl);

    asfSetting->smoothing_percent_5x5 = (uint8_t)
      ((float)asfSetting->smoothing_percent_5x5 *
      (1 - (asf_ctrl->asf_sharpness_data.portrait_severity / 255.0)) +
      (asf_ctrl->asf_sharpness_data.asd_soft_focus_dgr *
      (asf_ctrl->asf_sharpness_data.portrait_severity / 255.0)));
    /* calculate 5x5 LPF co-efficients*/
    asf_calc_5x5_lpf(&(asfSetting->smoothing_percent_5x5), asfCmd);
  }

  asf_config_print(asfCmd);

  CDBG("%s: send the update to ASF HW\n", __func__);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) asfCmd, sizeof(VFE_AdaptiveFilterConfigCmdType),
    VFE_CMD_ASF_UPDATE)) {
    CDBG_HIGH("%s: asf config for operation mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  asf_ctrl->asf_sp_effect_HW_enable = FALSE;
  asf_ctrl->asf_trigger_update = FALSE;
  vfe_params->update |= VFE_MOD_ASF;

  return VFE_SUCCESS;
} /* vfe_asf_update */

/*===========================================================================
 * Function:           vfe_asf_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_asf_enable(int mod_id, void *asf_mod, void *params,
  int8_t enable, int8_t hw_write)
{
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;

  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;

  CDBG("%s: enable=%d, hw_write=%d asf_enable %d\n", __func__,
    enable, hw_write, asf_ctrl->asf_enable);
  vfe_params->moduleCfg->asfEnable = enable;

  if (hw_write && (asf_ctrl->asf_enable == enable))
    return VFE_SUCCESS;

  asf_ctrl->asf_enable = enable;
  asf_ctrl->hw_enable_cmd = hw_write;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_ASF)
      : (vfe_params->current_config & ~VFE_MOD_ASF);
  }

  return VFE_SUCCESS;
} /* vfe_asf_enable */

vfe_status_t is_vfe_asf_reconfig(vfe_ctrl_info_t *p_obj, void *parm)
{
  uint32_t *ptr = (uint32_t *)parm;

  *ptr = p_obj->vfe_module.asf_mod.vfe_reconfig;
  p_obj->vfe_module.asf_mod.vfe_reconfig = 0;
  return VFE_SUCCESS;
}

/*=============================================================================
 * Function:               vfe_asf_init
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_asf_init(int mod_id, void *asf_mod, void *params)
{
  chromatix_parms_type *chrPtr = NULL;
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;

  asf_ctrl->asf_enable = TRUE;
  asf_ctrl->asf_trigger_enable = TRUE;
  asf_ctrl->asf_sp_effect_HW_enable = FALSE;
  asf_ctrl->asf_trigger_update = FALSE;
  asf_ctrl->asf_sharpness_update = FALSE;
  asf_ctrl->asf_reload_params = FALSE;
  asf_ctrl->hw_enable_cmd = FALSE;
  chrPtr = vfe_params->chroma3a;
  asf_ctrl->asf_prev_param.data = chrPtr->asf_5_5_preview;
  asf_ctrl->asf_prev_param.settings =
    asf_ctrl->asf_prev_param.data.setting[ASF_NORMAL_LIGHT];

  asf_ctrl->asf_snap_param.data = chrPtr->asf_5_5_preview;
  asf_ctrl->asf_snap_param.settings =
    asf_ctrl->asf_prev_param.data.setting[ASF_NORMAL_LIGHT];

  asf_ctrl->asf_trigger_ratio.ratio = 0;
  asf_ctrl->asf_trigger_ratio.lighting = TRIGGER_NORMAL;
  asf_ctrl->asf_sharpness_data.asd_soft_focus_dgr = 1.0;
  asf_ctrl->asf_sharpness_data.bst_soft_focus_dgr = 1.0;
  asf_ctrl->asf_sharpness_data.ui_sharp_ctrl_factor = 1.0 / 3.9;

  vfe_asf_set_special_effect(mod_id, asf_ctrl, vfe_params, CAMERA_EFFECT_OFF);

  return VFE_SUCCESS;
} /* vfe_asf_init */

/*=============================================================================
 * Function:               vfe_asf_deinit
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_asf_deinit(int mod_id, void *asf_mod, void *vfe_params)
{
  return VFE_SUCCESS;
} /* vfe_asf_deinit */

/*===========================================================================
 * FUNCTION    - vfe_asf_set_bestshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_set_bestshot(int mod_id, void *asf_mod,
  void *params, camera_bestshot_mode_type mode)
{
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;
  chromatix_parms_type *chroma = vfe_params->chroma3a;
  asf_ctrl->asf_trigger_enable = FALSE;

  CDBG("%s: mode %d", __func__, mode);
  switch(mode) {
    case CAMERA_BESTSHOT_PORTRAIT:
    case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
      vfe_params->sharpness_info.bst_soft_focus_dgr = chroma->soft_focus_degree;
      break;
    default:
      vfe_params->sharpness_info.bst_soft_focus_dgr = 1.0;
      asf_ctrl->asf_trigger_enable = TRUE;
      break;
  }
  vfe_params->update = TRUE;
  return VFE_SUCCESS;
} /* vfe_asf_set_bestshot */

/*=============================================================================
 * Function:               vfe_asf_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_asf_trigger_enable(int mod_id, void *asf_mod,
  void *params, int enable)
{
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;

  CDBG("%s: new trigger enable value = %d\n", __func__, enable);
  asf_ctrl->asf_trigger_enable = enable;

  return VFE_SUCCESS;
} /* vfe_asf_trigger_enable */

/*===========================================================================
 * FUNCTION    - vfe_asf_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_reload_params(int mod_id, void *asf_mod,
  void *params)
{
  vfe_status_t status = VFE_SUCCESS;
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  CDBG("%s: reload the chromatix\n", __func__);
  asf_ctrl->asf_reload_params = TRUE;

  CDBG("%s: reload the chromatix\n", __func__);
  if (vfe_params->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: update BSM in reload for mode %d",__func__,vfe_params->bs_mode);
    status = vfe_asf_set_bestshot(mod_id, asf_ctrl, vfe_params, vfe_params->bs_mode);
  }

  return status;
} /* vfe_asf_reload_params */

/*===========================================================================
 * FUNCTION    - vfe_asf_set_special_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_set_special_effect(int mod_id, void *asf_mod,
  void *params, vfe_spl_effects_type effects)
{
  vfe_status_t status = VFE_SUCCESS;
  asf_mod_t* asf_ctrl = (asf_mod_t *) asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;
  VFE_AdaptiveFilterConfigCmdType *asfCmd = NULL;
#ifdef VFE_32
  CDBG("%s: apply asf special effect %d", __func__, effects);
  switch(effects) {
    case CAMERA_EFFECT_EMBOSS:
      asf_ctrl->asf_prev_cmd.sharpMode = 0x2;
      asf_ctrl->asf_prev_cmd.smoothFilterEnabled = TRUE;
      asf_ctrl->asf_prev_cmd.sharpThreshE1 = 0;
      asf_ctrl->asf_prev_cmd.sharpThreshE2 = 127;
      asf_ctrl->asf_prev_cmd.sharpThreshE3 = -127;
      asf_ctrl->asf_prev_cmd.sharpThreshE4 = 64;
      asf_ctrl->asf_prev_cmd.sharpThreshE5 = 64;
      asf_ctrl->asf_prev_cmd.sharpK1 = FLOAT_TO_Q(3, 2.0);
      asf_ctrl->asf_prev_cmd.sharpK2 = FLOAT_TO_Q(3, 2.0);
      asf_ctrl->asf_prev_cmd.cutYSmooth = 0;
      asf_ctrl->asf_prev_cmd.smoothCoefCenter = 16;
      asf_ctrl->asf_prev_cmd.smoothCoefSurr = 0;
      asf_ctrl->asf_prev_cmd.F1Coeff0 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff1 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff3 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff4 = FLOAT_TO_Q(4, -1.0);
      asf_ctrl->asf_prev_cmd.F1Coeff5 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff7 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.nzFlag7 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag6 = 1;
      asf_ctrl->asf_prev_cmd.nzFlag5 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag4 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag3 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag2 = 1;
      asf_ctrl->asf_prev_cmd.nzFlag1 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag0 = 0;
      asf_ctrl->asf_sp_effect_HW_enable = TRUE;
      break;

    case CAMERA_EFFECT_SKETCH:
      asf_ctrl->asf_prev_cmd.sharpMode = 0x3;
      asf_ctrl->asf_prev_cmd.normalizeFactor = 127;
      asf_ctrl->asf_prev_cmd.smoothFilterEnabled = TRUE;
      asf_ctrl->asf_prev_cmd.sharpThreshE1 = 0;
      asf_ctrl->asf_prev_cmd.sharpThreshE2 = 127;
      asf_ctrl->asf_prev_cmd.sharpThreshE3 = -127;
      asf_ctrl->asf_prev_cmd.sharpThreshE4 = 96;
      asf_ctrl->asf_prev_cmd.sharpThreshE5 = 96;
      asf_ctrl->asf_prev_cmd.sharpK1 = FLOAT_TO_Q(3, -4.0);
      asf_ctrl->asf_prev_cmd.sharpK2 = FLOAT_TO_Q(3, 2.0);
      asf_ctrl->asf_prev_cmd.cutYSmooth = 0;
      asf_ctrl->asf_prev_cmd.smoothCoefCenter = 32;
      asf_ctrl->asf_prev_cmd.smoothCoefSurr = 0;
      asf_ctrl->asf_prev_cmd.F1Coeff0 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff1 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff3 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff4 = FLOAT_TO_Q(4, -1.0);
      asf_ctrl->asf_prev_cmd.F1Coeff5 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff7 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.nzFlag7 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag6 = 1;
      asf_ctrl->asf_prev_cmd.nzFlag5 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag4 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag3 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag2 = 1;
      asf_ctrl->asf_prev_cmd.nzFlag1 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag0 = 0;
      asf_ctrl->asf_sp_effect_HW_enable = TRUE;
      break;

    case CAMERA_EFFECT_NEON:
      asf_ctrl->asf_prev_cmd.sharpMode = 0x3;
      asf_ctrl->asf_prev_cmd.normalizeFactor = 127;
      asf_ctrl->asf_prev_cmd.smoothFilterEnabled = TRUE;
      asf_ctrl->asf_prev_cmd.sharpThreshE1 = 0;
      asf_ctrl->asf_prev_cmd.sharpThreshE2 = 127;
      asf_ctrl->asf_prev_cmd.sharpThreshE3 = -127;
      asf_ctrl->asf_prev_cmd.sharpThreshE4 = 0;
      asf_ctrl->asf_prev_cmd.sharpThreshE5 = 0;
      asf_ctrl->asf_prev_cmd.sharpK1 = FLOAT_TO_Q(3, 1.0);
      asf_ctrl->asf_prev_cmd.sharpK2 = FLOAT_TO_Q(3, 0.0);
      asf_ctrl->asf_prev_cmd.cutYSmooth = 1;
      asf_ctrl->asf_prev_cmd.smoothCoefCenter = 128;
      asf_ctrl->asf_prev_cmd.smoothCoefSurr = 0;
      asf_ctrl->asf_prev_cmd.F1Coeff0 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff1 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff3 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff4 = FLOAT_TO_Q(4, 1.0);
      asf_ctrl->asf_prev_cmd.F1Coeff5 = FLOAT_TO_Q(4, 1.0);
      asf_ctrl->asf_prev_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.F1Coeff7 = FLOAT_TO_Q(4, 1.0);
      asf_ctrl->asf_prev_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      asf_ctrl->asf_prev_cmd.nzFlag7 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag6 = 1;
      asf_ctrl->asf_prev_cmd.nzFlag5 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag4 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag3 = 2;
      asf_ctrl->asf_prev_cmd.nzFlag2 = 1;
      asf_ctrl->asf_prev_cmd.nzFlag1 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag0 = 0;
      asf_ctrl->asf_sp_effect_HW_enable = TRUE;
      break;
    default:
      CDBG("%s: turn off effect:  %d  ", __func__, effects);
      asf_ctrl->asf_prev_cmd.cutYSmooth = 0;
      asf_ctrl->asf_prev_cmd.nzFlag7 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag6 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag5 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag4 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag3 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag2 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag1 = 0;
      asf_ctrl->asf_prev_cmd.nzFlag0 = 0;
      asf_ctrl->asf_sp_effect_HW_enable = TRUE;
      status = vfe_asf_config(mod_id, asf_ctrl, vfe_params);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: ASF reset failed after ASF special effect ", __func__);
        return VFE_ERROR_GENERAL;
      }
      CDBG("%s: ASF reset success after ASF special effect ", __func__);
      break;
  }
  asf_ctrl->asf_snap_cmd = asf_ctrl->asf_prev_cmd;
#endif
  return status;
} /*vfe_asf_set_special_effect */

/*===========================================================================
 * FUNCTION    - vfe_asf_SP_effect_snapshot_adjust -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_SP_effect_snapshot_adjust(int mod_id, void *asf_mod,
  void *params, vfe_spl_effects_type effects)
{
  float k1_Q3_emboss = 0;
  float k1_Q3_neon = 0;
  float DS_threshold1 = 4.0;
  float DS_threshold2 = 2.0;
  asf_mod_t* asf_ctrl = (asf_mod_t *)asf_mod;
  vfe_params_t* vfe_params = (vfe_params_t *)params;
#ifdef VFE_32
  if(!IS_SNAP_MODE(vfe_params)){
    CDBG("%s: no need to adjust k1 in preview mode, reset snap_cmd", __func__);
    asf_ctrl->asf_snap_cmd = asf_ctrl->asf_prev_cmd;
    return VFE_SUCCESS;
  }

  k1_Q3_emboss = 2 +
    1.9*(6.5 - asf_ctrl->asf_sharpness_data.downscale_factor) / (6.5 - 1.039);
  k1_Q3_neon = 1 +
    0.3*(6.5 - asf_ctrl->asf_sharpness_data.downscale_factor) / (6.5 - 1.039);

  switch(effects) {
    case CAMERA_EFFECT_EMBOSS:
      asf_ctrl->asf_snap_cmd.sharpK1 = FLOAT_TO_Q(3, k1_Q3_emboss);
      if (asf_ctrl->asf_sharpness_data.downscale_factor < DS_threshold1) {
        asf_ctrl->asf_snap_cmd.F1Coeff0 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff1 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff3 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff4 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff5 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff7 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      }
      break;
    case CAMERA_EFFECT_SKETCH:
     if (asf_ctrl->asf_sharpness_data.downscale_factor < DS_threshold2) {
        asf_ctrl->asf_snap_cmd.F1Coeff0 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff1 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff3 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff4 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff5 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff7 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      } else if (asf_ctrl->asf_sharpness_data.downscale_factor < DS_threshold1) {
        asf_ctrl->asf_snap_cmd.F1Coeff0 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff1 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff3 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff4 = FLOAT_TO_Q(4, -1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff5 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff7 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      }
      break;
    case CAMERA_EFFECT_NEON:
      asf_ctrl->asf_snap_cmd.sharpK1 = FLOAT_TO_Q(3, k1_Q3_neon);
      if (asf_ctrl->asf_sharpness_data.downscale_factor < 4) {
        asf_ctrl->asf_snap_cmd.F1Coeff0 = FLOAT_TO_Q(4, 1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff1 = FLOAT_TO_Q(4, 1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff2 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff3 = FLOAT_TO_Q(4, 1.0);
        asf_ctrl->asf_snap_cmd.F1Coeff4 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff5 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff6 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff7 = FLOAT_TO_Q(4, 0.0);
        asf_ctrl->asf_snap_cmd.F1Coeff8 = FLOAT_TO_Q(4, 0.0);
      }
      break;
    default:
      CDBG("%s: no need to adjust k1 by current efffect", __func__);
      break;
  }
#endif
  return VFE_SUCCESS;
}/*vfe_asf_SP_effect_snapshot_adjust*/

/*===========================================================================
 * FUNCTION    - vfe_asf_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_test_vector_validation(int mod_id, void *in, void *out)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)out;

#ifdef VFE_32
  VFE_AdaptiveFilterConfigCmdType *InCmd_local =
    (VFE_AdaptiveFilterConfigCmdType *)(mod_in->reg_dump + VFE_ASF_OFF1/4);
  VFE_AdaptiveFilterConfigCmdType *OutCmd_local =
    (VFE_AdaptiveFilterConfigCmdType *)(mod_op->reg_dump_data + VFE_ASF_OFF1/4);

  CDBG("%s: enter test vector validate \n", __func__);

  if (InCmd_local == NULL || OutCmd_local == NULL) {
    CDBG_HIGH("%s: asf test vector failed: casting failed\n", __func__);
    return VFE_ERROR_GENERAL;
  }

  asf_config_print(InCmd_local);
  asf_config_print(OutCmd_local);

  /* ASF Config Command */
  if (InCmd_local->smoothFilterEnabled != OutCmd_local->smoothFilterEnabled){
    CDBG_HIGH("%s: smoothFilterEnabled not pass: in = %d, out = %d \n", __func__,
      InCmd_local->smoothFilterEnabled, OutCmd_local->smoothFilterEnabled);
  }
  if (abs(InCmd_local->sharpMode - OutCmd_local->sharpMode) > 0){
    CDBG_HIGH("%s: sharpMode not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpMode, OutCmd_local->sharpMode);
  }
  if (abs(InCmd_local->lpfMode - OutCmd_local->lpfMode) >0){
    CDBG_HIGH("%s: lpfMode not pass: in = %d, out = %d \n", __func__,
      InCmd_local->lpfMode, OutCmd_local->lpfMode);
  }
  if (abs(InCmd_local->smoothCoefSurr - OutCmd_local->smoothCoefSurr) > 0){
    CDBG_HIGH("%s: smoothCoefSurr not pass: in = %d, out = %d \n", __func__,
      InCmd_local->smoothCoefSurr, OutCmd_local->smoothCoefSurr);
  }
  if (abs(InCmd_local->smoothCoefCenter - OutCmd_local->smoothCoefCenter) > 0){
    CDBG_HIGH("%s: smoothCoefCenter not pass: in = %d, out = %d \n", __func__,
      InCmd_local->smoothCoefCenter, OutCmd_local->smoothCoefCenter);
  }
  if (abs(InCmd_local->pipeFlushCount - OutCmd_local->pipeFlushCount) > 0){
    CDBG_HIGH("%s: pipeFlushCount not pass: in = %d, out = %d \n", __func__,
      InCmd_local->pipeFlushCount, OutCmd_local->pipeFlushCount);
  }
  if (abs(InCmd_local->pipeFlushOvd - OutCmd_local->pipeFlushOvd) > 0){
    CDBG_HIGH("%s: pipeFlushOvd not pass: in = %d, out = %d \n", __func__,
      InCmd_local->pipeFlushOvd, OutCmd_local->pipeFlushOvd);
  }
  if (abs(InCmd_local->flushHaltOvd - OutCmd_local->flushHaltOvd) > 0){
    CDBG_HIGH("%s: flushHaltOvd not pass: in = %d, out = %d \n", __func__,
      InCmd_local->flushHaltOvd, OutCmd_local->flushHaltOvd);
  }
  if (abs(InCmd_local->cropEnable - OutCmd_local->cropEnable) > 0){
    CDBG_HIGH("%s: cropEnable not pass: in = %d, out = %d \n", __func__,
      InCmd_local->cropEnable, OutCmd_local->cropEnable);
  }

  /* Sharpening Config 0 */
  if (abs(InCmd_local->sharpThreshE1 - OutCmd_local->sharpThreshE1) > 0){
    CDBG_HIGH("%s: sharpThreshE1 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpThreshE1, OutCmd_local->sharpThreshE1);
  }
  if (abs(InCmd_local->sharpK1 - OutCmd_local->sharpK1) > 0){
    CDBG_HIGH("%s: sharpK1 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpK1, OutCmd_local->sharpK1);
  }
  if (abs(InCmd_local->sharpK2 - OutCmd_local->sharpK2) > 0){
    CDBG_HIGH("%s: sharpK2 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpK2, OutCmd_local->sharpK2);
  }
  if (abs(InCmd_local->normalizeFactor - OutCmd_local->normalizeFactor) > 0){
    CDBG_HIGH("%s: normalizeFactor not pass: in = %d, out = %d \n", __func__,
      InCmd_local->normalizeFactor, OutCmd_local->normalizeFactor);
  }
  if (abs(InCmd_local->cutYSmooth - OutCmd_local->cutYSmooth) > 0){
    CDBG_HIGH("%s: cutYSmooth not pass: in = %d, out = %d \n", __func__,
      InCmd_local->cutYSmooth, OutCmd_local->cutYSmooth);
  }

  /* Sharpening Config 1 */
  if (abs(InCmd_local->sharpThreshE2 - OutCmd_local->sharpThreshE2) > 0){
    CDBG_HIGH("%s: sharpThreshE2 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpThreshE2, OutCmd_local->sharpThreshE2);
  }
  if (abs(InCmd_local->sharpThreshE3 - OutCmd_local->sharpThreshE3) > 0){
    CDBG_HIGH("%s: sharpThreshE3 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpThreshE3, OutCmd_local->sharpThreshE3);
  }
  if (abs(InCmd_local->sharpThreshE4 - OutCmd_local->sharpThreshE4) > 0){
    CDBG_HIGH("%s: sharpThreshE4 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpThreshE4, OutCmd_local->sharpThreshE4);
  }
  if (abs(InCmd_local->sharpThreshE5 - OutCmd_local->sharpThreshE5) > 0){
    CDBG_HIGH("%s: sharpThreshE5 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->sharpThreshE5, OutCmd_local->sharpThreshE5);
  }

  /* Sharpening Coefficients 0 */
  if (abs(InCmd_local->F1Coeff0 - OutCmd_local->F1Coeff0) > 0){
    CDBG_HIGH("%s: F1Coeff0 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff0, OutCmd_local->F1Coeff0);
  }
  if (abs(InCmd_local->F1Coeff1 - OutCmd_local->F1Coeff1) > 0){
    CDBG_HIGH("%s: F1Coeff1 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff1, OutCmd_local->F1Coeff1);
  }
  if (abs(InCmd_local->F1Coeff2 - OutCmd_local->F1Coeff2) > 0){
    CDBG_HIGH("%s: F1Coeff2 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff2, OutCmd_local->F1Coeff2);
  }
  if (abs(InCmd_local->F1Coeff3 - OutCmd_local->F1Coeff3) > 0){
    CDBG_HIGH("%s: F1Coeff3 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff3, OutCmd_local->F1Coeff3);
  }
  if (abs(InCmd_local->F1Coeff4 - OutCmd_local->F1Coeff4) > 0){
    CDBG_HIGH("%s: F1Coeff4 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff4, OutCmd_local->F1Coeff4);
  }

  /* Sharpening Coefficients 1 */
  if (abs(InCmd_local->F1Coeff5 - OutCmd_local->F1Coeff5) > 0){
    CDBG_HIGH("%s: F1Coeff5 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff5, OutCmd_local->F1Coeff5);
  }
  if (abs(InCmd_local->F1Coeff6 - OutCmd_local->F1Coeff6) > 0){
    CDBG_HIGH("%s: F1Coeff6 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff6, OutCmd_local->F1Coeff6);
  }
  if (abs(InCmd_local->F1Coeff7 - OutCmd_local->F1Coeff7) > 0){
    CDBG_HIGH("%s: F1Coeff7 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff7, OutCmd_local->F1Coeff7);
  }
  if (abs(InCmd_local->F1Coeff8 - OutCmd_local->F1Coeff8) > 0){
    CDBG_HIGH("%s: F1Coeff8 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F1Coeff8, OutCmd_local->F1Coeff8);
  }

  /* Sharpening Coefficients 2 */
  if (abs(InCmd_local->F2Coeff0 - OutCmd_local->F2Coeff0) > 0){
    CDBG_HIGH("%s: F2Coeff0 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff0, OutCmd_local->F2Coeff0);
  }
  if (abs(InCmd_local->F2Coeff1 - OutCmd_local->F2Coeff1) > 0){
    CDBG_HIGH("%s: F2Coeff1 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff1, OutCmd_local->F2Coeff1);
  }
  if (abs(InCmd_local->F2Coeff2 - OutCmd_local->F2Coeff2) > 0){
    CDBG_HIGH("%s: F2Coeff2 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff2, OutCmd_local->F2Coeff2);
  }
  if (abs(InCmd_local->F2Coeff3 - OutCmd_local->F2Coeff3) > 0){
    CDBG_HIGH("%s: F2Coeff3 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff3, OutCmd_local->F2Coeff3);
  }
  if (abs(InCmd_local->F2Coeff4 - OutCmd_local->F2Coeff4) > 0){
    CDBG_HIGH("%s: F2Coeff4 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff4, OutCmd_local->F2Coeff4);
  }

  /* Sharpening Coefficients 3 */
  if (abs(InCmd_local->F2Coeff5 - OutCmd_local->F2Coeff5) > 0){
    CDBG_HIGH("%s: F2Coeff5 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff5, OutCmd_local->F2Coeff5);
  }
  if (abs(InCmd_local->F2Coeff6 - OutCmd_local->F2Coeff6) > 0){
    CDBG_HIGH("%s: F2Coeff6 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff6, OutCmd_local->F2Coeff6);
  }
  if (abs(InCmd_local->F2Coeff7 - OutCmd_local->F2Coeff7) > 0){
    CDBG_HIGH("%s: F2Coeff7 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff7, OutCmd_local->F2Coeff7);
  }
  if (abs(InCmd_local->F2Coeff8 - OutCmd_local->F2Coeff8) > 0){
    CDBG_HIGH("%s: F2Coeff8 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F2Coeff8, OutCmd_local->F2Coeff8);
  }

  /* Sharpening Coefficients 4 */
  if (abs(InCmd_local->F3Coeff0 - OutCmd_local->F3Coeff0) > 0){
    CDBG_HIGH("%s: F3Coeff0 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff0, OutCmd_local->F3Coeff0);
  }
  if (abs(InCmd_local->F3Coeff1 - OutCmd_local->F3Coeff1) > 0){
    CDBG_HIGH("%s: F3Coeff1 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff1, OutCmd_local->F3Coeff1);
  }
  if (abs(InCmd_local->F3Coeff2 - OutCmd_local->F3Coeff2) > 0){
    CDBG_HIGH("%s: F3Coeff2 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff2, OutCmd_local->F3Coeff2);
  }
  if (abs(InCmd_local->F3Coeff3 - OutCmd_local->F3Coeff3) > 0){
    CDBG_HIGH("%s: F3Coeff3 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff3, OutCmd_local->F3Coeff3);
  }
  if (abs(InCmd_local->F3Coeff4 - OutCmd_local->F3Coeff4) > 0){
    CDBG_HIGH("%s: F3Coeff4 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff4, OutCmd_local->F3Coeff4);
  }

  /* Sharpening Coefficients 5 */
  if (abs(InCmd_local->F3Coeff5 - OutCmd_local->F3Coeff5) > 0){
    CDBG_HIGH("%s: F3Coeff5 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff5, OutCmd_local->F3Coeff5);
  }
  if (abs(InCmd_local->F3Coeff6 - OutCmd_local->F3Coeff6) > 0){
    CDBG_HIGH("%s: F3Coeff6 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff6, OutCmd_local->F3Coeff6);
  }
  if (abs(InCmd_local->F3Coeff7 - OutCmd_local->F3Coeff7) > 0){
    CDBG_HIGH("%s: F3Coeff7 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff7, OutCmd_local->F3Coeff7);
  }
  if (abs(InCmd_local->F3Coeff8 - OutCmd_local->F3Coeff8) > 0){
    CDBG_HIGH("%s: F3Coeff8 not pass: in = %d, out = %d \n", __func__,
      InCmd_local->F3Coeff8, OutCmd_local->F3Coeff8);
  }
#endif
  return VFE_SUCCESS;
}/*vfe_asf_test_vector_validation*/

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_asf_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  asf_module_t *cmd = (asf_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&(cmd->asf_prev_cmd), sizeof(VFE_AdaptiveFilterConfigCmdType),
    VFE_CMD_ASF_UPDATE)) {
    CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_asf_plugin_update */
#endif
