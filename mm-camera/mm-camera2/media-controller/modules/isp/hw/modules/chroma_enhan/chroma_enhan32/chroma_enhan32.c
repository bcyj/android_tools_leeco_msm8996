/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "chroma_enhan32.h"
#include "isp_log.h"

#ifdef ENABLE_CV_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#define SET_SAT_MATRIX(IN, S) ({\
  IN[0][0] = IN[1][1] = S; \
  IN[0][1] = IN[1][0] = 0; \
  })

#define SET_HUE_MATRIX(IN, H) ({\
  IN[0][0] = cos(H); \
  IN[0][1] = -sin(H); \
  IN[1][0] = sin(H); \
  IN[1][1] = cos(H); \
  })

/** util_color_conversion_cmd_debug:
 *
 *    @mod:
 *
 **/
static void util_color_conversion_cmd_debug(isp_color_conversion_mod_t *mod)
{
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: E", __func__);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: am = %d, bm = %d, cm = %d, dm = %d\n", __func__,
    mod->RegCmd.am, mod->RegCmd.bm,
    mod->RegCmd.cm, mod->RegCmd.dm);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: ap = %d, bp = %d, cp = %d, dp = %d\n", __func__,
    mod->RegCmd.ap, mod->RegCmd.bp,
    mod->RegCmd.cp, mod->RegCmd.dp);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: RGBtoYConversionV0 = %d\n", __func__,
    mod->RegCmd.RGBtoYConversionV0);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: RGBtoYConversionV1 = %d\n", __func__,
    mod->RegCmd.RGBtoYConversionV1);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: RGBtoYConversionV2 = %d\n", __func__,
    mod->RegCmd.RGBtoYConversionV2);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: RGBtoYConversionOffset = %d\n", __func__,
    mod->RegCmd.RGBtoYConversionOffset);
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: kcb = %d, kcr = %d\n", __func__,
    mod->RegCmd.kcb, mod->RegCmd.kcr);
} /* util_color_conversion_cmd_debug */

/** util_color_conversion_cmd_config:
 *
 *    @mod:
 *
 * copy from mod->threshold to reg cmd then configure
 *
 **/
static void util_color_conversion_cmd_config(isp_color_conversion_mod_t *mod)
{
  chromatix_color_conversion_type *cc = &(mod->cv_data);
  double am_new, bm_new, cm_new, dm_new;
  double ap_new, bp_new, cp_new, dp_new;

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: effects_matrix: %f, %f; %f, %f\n", __func__,
    mod->effects_matrix[0][0], mod->effects_matrix[0][1],
    mod->effects_matrix[1][0], mod->effects_matrix[1][1]);

  /*config 1st set of matrix for HW to select, am, bm, cm, dm*/
  am_new = cc->chroma.a_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[0][1];
  bm_new = cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * mod->effects_matrix[0][1];
  cm_new = cc->chroma.c_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[1][0];
  dm_new = cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * mod->effects_matrix[1][0];

  if (am_new)
    bm_new /= am_new;

  if (cm_new)
    dm_new /= cm_new;

  if (fabs(am_new) >= 4)
    CDBG_HIGH("%s: error overflow a_m_new = %f\n", __func__, am_new);
  if (fabs(bm_new) >= 4)
    CDBG_HIGH("%s: error overflow b_m_new = %f\n", __func__, bm_new);
  if (fabs(cm_new) >= 4)
    CDBG_HIGH("%s: error overflow c_m_new = %f\n", __func__, cm_new);
  if (fabs(dm_new) >= 4)
    CDBG_HIGH("%s: error overflow d_m_new = %f\n", __func__, dm_new);

  mod->RegCmd.am = FLOAT_TO_Q(8, am_new);
  mod->RegCmd.bm = FLOAT_TO_Q(8, bm_new);
  mod->RegCmd.cm = FLOAT_TO_Q(8, cm_new);
  mod->RegCmd.dm = FLOAT_TO_Q(8, dm_new);

  /*config 2nd set of matrix for HW to select, ap, bp, cp, dp*/
  ap_new = cc->chroma.a_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[0][1];
  bp_new = cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * mod->effects_matrix[0][1];
  cp_new = cc->chroma.c_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[1][0];
  dp_new = cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * mod->effects_matrix[1][0];

  if (ap_new)
    bp_new /= ap_new;

  if (cp_new)
    dp_new /= cp_new;

  if (fabs(ap_new) >= 4)
    CDBG_HIGH("%s: error overflow a_p_new = %f\n", __func__, ap_new);
  if (fabs(bp_new) >= 4)
    CDBG_HIGH("%s: error overflow b_p_new = %f\n", __func__, bp_new);
  if (fabs(cp_new) >= 4)
    CDBG_HIGH("%s: error overflow c_p_new = %f\n", __func__, cp_new);
  if (fabs(dp_new) >= 4)
    CDBG_HIGH("%s: error overflow d_p_new = %f\n", __func__, dp_new);

  mod->RegCmd.ap = FLOAT_TO_Q(8, ap_new);
  mod->RegCmd.bp = FLOAT_TO_Q(8, bp_new);
  mod->RegCmd.cp = FLOAT_TO_Q(8, cp_new);
  mod->RegCmd.dp = FLOAT_TO_Q(8, dp_new);

  /*constant offset for matrix conversion: Cb, Cr*/
  mod->RegCmd.kcb = cc->chroma.k_cb;
  mod->RegCmd.kcr = cc->chroma.k_cr;

  /*constant offset for matrix conversion: Y*/
  mod->RegCmd.RGBtoYConversionOffset = cc->luma.k;
  /*Coeff for R, G, B for calculating Y*/
  mod->RegCmd.RGBtoYConversionV0 = FLOAT_TO_Q(8, cc->luma.v0);
  mod->RegCmd.RGBtoYConversionV1 = FLOAT_TO_Q(8, cc->luma.v1);
  mod->RegCmd.RGBtoYConversionV2 = FLOAT_TO_Q(8, cc->luma.v2);
} /* util_color_conversion_cmd_config */

/** util_color_conversion_interpolate:
 *
 *    @in1:
 *    @in2:
 *    @out:
 *    @ratio:
 *
 **/
static void util_color_conversion_interpolate(
  chromatix_color_conversion_type* in1, chromatix_color_conversion_type *in2,
  chromatix_color_conversion_type* out, float ratio)
{
   if (!in1 || !in2 || !out) {
      CDBG_ERROR("%s:%d failed %p %p %p\n", __func__, __LINE__, in1, in2, out);
      return;
   }

  out->chroma.a_p = LINEAR_INTERPOLATION (in1->chroma.a_p,
    in2->chroma.a_p, ratio);
  out->chroma.a_m = LINEAR_INTERPOLATION(in1->chroma.a_m,
    in2->chroma.a_m, ratio);
  out->chroma.b_p = LINEAR_INTERPOLATION(in1->chroma.b_p,
    in2->chroma.b_p, ratio);
  out->chroma.b_m = LINEAR_INTERPOLATION(in1->chroma.b_m,
    in2->chroma.b_m, ratio);
  out->chroma.c_p = LINEAR_INTERPOLATION(in1->chroma.c_p,
    in2->chroma.c_p, ratio);
  out->chroma.c_m = LINEAR_INTERPOLATION(in1->chroma.c_m,
    in2->chroma.c_m, ratio);
  out->chroma.d_p = LINEAR_INTERPOLATION(in1->chroma.d_p,
    in2->chroma.d_p, ratio);
  out->chroma.d_m = LINEAR_INTERPOLATION(in1->chroma.d_m,
    in2->chroma.d_m, ratio);
  out->chroma.k_cb = LINEAR_INTERPOLATION(in1->chroma.k_cb,
    in2->chroma.k_cb, ratio);
  out->chroma.k_cr = LINEAR_INTERPOLATION(in1->chroma.k_cr,
    in2->chroma.k_cr, ratio);
  out->luma.k = LINEAR_INTERPOLATION(in1->luma.k, in2->luma.k, ratio);
  out->luma.v0 = LINEAR_INTERPOLATION(in1->luma.v0, in2->luma.v0, ratio);
  out->luma.v1 = LINEAR_INTERPOLATION(in1->luma.v1, in2->luma.v1, ratio);
  out->luma.v2 = LINEAR_INTERPOLATION(in1->luma.v2, in2->luma.v2, ratio);
} /* util_color_conversion_interpolate */

/** util_color_conversion_aec_update:
 *
 *    @mod:
 *    @in_params:
 *    @p_cv_data:
 *    @p_trigger_ratio:
 *
 **/
static void util_color_conversion_aec_update(isp_color_conversion_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params,
  chromatix_color_conversion_type* p_cv_data, trigger_ratio_t *p_trigger_ratio)
{
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr = &chroma_ptr->chromatix_VFE.chromatix_CV;

  switch (p_trigger_ratio->lighting) {
  case TRIGGER_LOWLIGHT: {
    util_color_conversion_interpolate(p_cv_data,
      &chromatix_CV_ptr->lowlight_color_conversion,
      &mod->cv_data, p_trigger_ratio->ratio);
  }
    break;

  case TRIGGER_OUTDOOR: {
    util_color_conversion_interpolate(p_cv_data,
      &chromatix_CV_ptr->outdoor_color_conversion,
      &mod->cv_data, p_trigger_ratio->ratio);
  }
    break;

   default:
   case TRIGGER_NORMAL: {
     mod->cv_data = *p_cv_data;
   }
     break;
  }
} /* util_color_conversion_aec_update */

/** util_color_conversion_awb_update:
 *
 *    @mod:
 *    @in_params:
 *    @cv_data:
 *
 **/
static void util_color_conversion_awb_update(isp_color_conversion_mod_t *mod,
  isp_pix_trigger_update_input_t* in_params,
  chromatix_color_conversion_type *cv_data)
{
  chromatix_parms_type *chroma_ptr;
  chromatix_CV_type *chromatix_CV_ptr;
  ASD_struct_type *ASD_algo_ptr;
  cct_trigger_info trigger_info;
  float ratio = 0.0;
  float color_temp = 0.0f;

  if (!mod || !in_params || !cv_data) {
    CDBG_ERROR("%s:%d failed: %p %p %p\n", __func__, __LINE__, mod, in_params,
      cv_data);
    return;
  }
  chroma_ptr = (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_CV_ptr = &chroma_ptr->chromatix_VFE.chromatix_CV;
  ASD_algo_ptr = &chroma_ptr->ASD_algo_data;

  color_temp = in_params->trigger_input.stats_update.awb_update.color_temp;
  trigger_info.mired_color_temp =
    MIRED(in_params->trigger_input.stats_update.awb_update.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    chromatix_CV_ptr->CV_Daylight_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
    chromatix_CV_ptr->CV_A_trigger);
  awb_cct_type cct_type = isp_util_get_awb_cct_type(mod->notify_ops->parent,
	                                                  &trigger_info, chroma_ptr);

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: CCT type %d", __func__, cct_type);

  chromatix_color_conversion_type *cv_table_A, *cv_table_TL84, *cv_table_D65;

  if (in_params->cfg.bestshot_mode == CAM_SCENE_MODE_PORTRAIT) {
    cv_table_A = &ASD_algo_ptr->skintone_color_conversion_a;
    cv_table_TL84 = &ASD_algo_ptr->skintone_color_conversion;
    cv_table_D65 = &ASD_algo_ptr->skintone_color_conversion_d65;
  } else {
    cv_table_A = &chromatix_CV_ptr->A_color_conversion;
    cv_table_TL84 = &chromatix_CV_ptr->TL84_color_conversion;
    cv_table_D65 =   &chromatix_CV_ptr->daylight_color_conversion;
  }

  switch (cct_type) {
  case AWB_CCT_TYPE_A: {
    *cv_data = *cv_table_A;
  }
    break;
  case AWB_CCT_TYPE_TL84_A: {
    ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
      1.0f / (float)chromatix_CV_ptr->CV_A_trigger.CCT_start,
      1.0f / (float)chromatix_CV_ptr->CV_A_trigger.CCT_end);
    util_color_conversion_interpolate(cv_table_TL84, cv_table_A, cv_data,
      ratio);
  }
    break;

  case AWB_CCT_TYPE_D65_TL84: {
    ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
      1.0f / (float)chromatix_CV_ptr->CV_Daylight_trigger.CCT_end,
      1.0f / (float)chromatix_CV_ptr->CV_Daylight_trigger.CCT_start);
    util_color_conversion_interpolate(cv_table_D65,cv_table_TL84, cv_data,
      ratio);
  }
    break;

  case AWB_CCT_TYPE_D65: {
    *cv_data = *cv_table_D65;
  }
    break;

  default:
  case AWB_CCT_TYPE_TL84: {
    *cv_data = *cv_table_TL84;
  }
    break;
  }
} /* util_color_conversion_awb_update */

/** color_conversion_set_spl_effect:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 **/
static int color_conversion_set_spl_effect(isp_color_conversion_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  chromatix_parms_type *chroma_ptr =
    in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr = &chroma_ptr->chromatix_VFE.chromatix_CV;
  int status= 0;
  cam_effect_mode_type effects =
    (cam_effect_mode_type) in_params->effects.spl_effect;
  mod->trigger_enable = FALSE;
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: apply %d", __func__, effects);
  /* reset hue/saturation */
  SET_UNITY_MATRIX(mod->effects_matrix, 2);

  switch(effects) {
  case CAM_EFFECT_MODE_EMBOSS:
  case CAM_EFFECT_MODE_MONO: {
    mod->cv_data = chromatix_CV_ptr->mono_color_conversion;
  }
    break;

  case CAM_EFFECT_MODE_NEGATIVE: {
    mod->cv_data = chromatix_CV_ptr->negative_color_conversion;
  }
    break;

  case CAM_EFFECT_MODE_SEPIA: {
    mod->cv_data = chromatix_CV_ptr->sepia_color_conversion;
  }
    break;

  case CAM_EFFECT_MODE_AQUA: {
    mod->cv_data = chromatix_CV_ptr->aqua_color_conversion;
  }
    break;

  default: {
    mod->cv_data = chromatix_CV_ptr->TL84_color_conversion;
    mod->trigger_enable = TRUE;
    mod->aec_ratio.lighting = TRIGGER_NORMAL;
    mod->aec_ratio.ratio = 0;
  }
    break;
  }

  util_color_conversion_cmd_config(mod);
  mod->hw_update_pending = TRUE;

  return status;
} /* util_color_conversion_set_spl_effect */

/** color_conversion_set_effect:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 * Set special effect
 *
 **/
static int color_conversion_set_effect(isp_color_conversion_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  int i = 0, j = 0;
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s",__func__);
  float hue_matrix[2][2];
  float sat_matrix[2][2];
  float s, hue_in_radian;
  int type;
  int status= 0;

  if (in_params->bestshot_mode != CAM_SCENE_MODE_OFF) {
    ISP_DBG(ISP_MOD_COLOR_CONV, "%s: Best shot enabled, skip set effect", __func__);
    return 0;
  }

  s = 2.0 * in_params->effects.saturation;
  hue_in_radian = DEGREE_TO_RADIAN(in_params->effects.hue * 10); //ryan need to define in pix_common.h

  type = in_params->effects.effect_type_mask;

  if(type & (1 << ISP_EFFECT_SPECIAL)) {
    status = color_conversion_set_spl_effect(mod, in_params, in_param_size);
    if (status)
      goto END;
  }

  if(type & (1 << ISP_EFFECT_SATURATION)) {
    SET_UNITY_MATRIX(mod->effects_matrix, 2);
    SET_SAT_MATRIX(sat_matrix, s);
  }

  if(type & (1 << ISP_EFFECT_HUE)) {
    if (F_EQUAL(in_params->effects.hue, 0)) {
      SET_UNITY_MATRIX(hue_matrix, 2);
    } else {
      SET_HUE_MATRIX(hue_matrix, hue_in_radian);
    }

    MATRIX_MULT(sat_matrix, hue_matrix, mod->effects_matrix, 2, 2, 2);
  }

#ifdef ENABLE_CV_LOGGING
  PRINT_2D_MATRIX(2, 2, mod->effects_matrix);
#endif

  mod->hw_update_pending = TRUE;

END:
  return status;
}/*color_conversion_set_effect*/

/** color_conversion_set_manual_wb:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 **/
static int color_conversion_set_manual_wb(isp_color_conversion_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  int status= 0;
  chromatix_parms_type *chromatix_ptr =
    in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr =
    &chromatix_ptr->chromatix_VFE.chromatix_CV;

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: wb %d", __func__, in_params->wb_mode);

  switch(in_params->wb_mode) {
  case CAM_WB_MODE_INCANDESCENT: {
    mod->p_cv = &chromatix_CV_ptr->A_color_conversion;
  }
    break;

  case CAM_WB_MODE_CLOUDY_DAYLIGHT:
  case CAM_WB_MODE_DAYLIGHT: {
    mod->p_cv = &chromatix_CV_ptr->daylight_color_conversion;
  }
    break;

  default: {
    mod->p_cv = &chromatix_CV_ptr->TL84_color_conversion;
  }
    break;
  }

  mod->cv_data = *(mod->p_cv);
  util_color_conversion_cmd_config(mod);
  mod->hw_update_pending = TRUE;

  return status;
} /* color_conversion_set_manual_wb */

/** color_conversion_set_bestshot:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 * Set BestShot mode
 *
 **/
static int color_conversion_set_bestshot(isp_color_conversion_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size )
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  chromatix_parms_type *chroma_ptr =
    in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr = &chroma_ptr->chromatix_VFE.chromatix_CV;
  ASD_struct_type *ASD_algo_ptr = &chroma_ptr->ASD_algo_data;
  cam_scene_mode_type bestshot_mode = in_params->bestshot_mode;
  float s = 0.0;
  int status= 0;

  mod->trigger_enable = TRUE;
  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: mode %d\n", __func__, bestshot_mode);
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  switch (bestshot_mode) {
  case CAM_SCENE_MODE_THEATRE:
  case CAM_SCENE_MODE_CANDLELIGHT:
  case CAM_SCENE_MODE_SUNSET: {
    mod->cv_data = ASD_algo_ptr->sunset_color_conversion;
  }
    break;

  case CAM_SCENE_MODE_FIREWORKS: {
    /* from WB need to check */
    mod->cv_data = chromatix_CV_ptr->daylight_color_conversion;
  }
    break;

  case CAM_SCENE_MODE_FLOWERS:
  case CAM_SCENE_MODE_PARTY: {
    /* apply required saturation */
    s = chromatix_CV_ptr->saturated_color_conversion_factor;
    /* Just update color conversion matrix in this case */
    SET_UNITY_MATRIX(mod->effects_matrix, 2);
    SET_SAT_MATRIX(mod->effects_matrix, s);
  }
    break;

  case CAM_SCENE_MODE_PORTRAIT:
  case CAM_SCENE_MODE_NIGHT_PORTRAIT:
  default: {
    mod->cv_data = chromatix_CV_ptr->TL84_color_conversion;
    mod->trigger_enable = FALSE;
  }
    break;
  }

  util_color_conversion_cmd_config(mod);
  util_color_conversion_cmd_debug(mod);
  mod->hw_update_pending = TRUE;

  return status;
} /* color_conversion_set_bestshot */

/** color_conversion_init:
 *
 *    @mod_ctrl:
 *    @in_params:
 *    @notify_ops:
 *
 * init color_conversionion module
 *
 **/
static int color_conversion_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_color_conversion_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  return 0;
} /* color_conversion_init */

/** color_conversion_config:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 * configure initial settings
 *
 **/
static int color_conversion_config(isp_color_conversion_mod_t *mod, isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int  rc = 0;
  chromatix_parms_type *chromatix_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr =
      &chromatix_ptr->chromatix_VFE.chromatix_CV;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: enter", __func__);

  if (!mod->enable) {
    CDBG_HIGH("%s: Mod not Enable.", __func__);
    return rc;
  }

  /* set old cfg to invalid value to trigger the first trigger update */
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  mod->cv_data = chromatix_CV_ptr->TL84_color_conversion;
  mod->p_cv = &(chromatix_CV_ptr->TL84_color_conversion);

  util_color_conversion_cmd_config(mod);
  mod->hw_update_pending = TRUE;

  return rc;
} /* color_conversion_config */

/** color_conversion_destroy:
 *
 *    @mod_ctrl:
 *
 * close color_conversion mod
 *
 **/
static int color_conversion_destroy(void *mod_ctrl)
{
  isp_color_conversion_mod_t *mod = mod_ctrl;

  memset(mod,  0,  sizeof(isp_color_conversion_mod_t));
  free(mod);

  return 0;
} /* color_conversion_destroy */

/** color_conversion_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 * enable module
 *
 **/
static int color_conversion_enable(isp_color_conversion_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
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
} /* color_conversion_enable */

/** color_conversion_trigger_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 * enable color_conversion trigger update feature
 *
 **/
static int color_conversion_trigger_enable(isp_color_conversion_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->trigger_enable = enable->enable;

  return 0;
} /* color_conversion_trigger_enable */

/** color_conversion_trigger_update:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 * enable color_conversion trigger update feature
 *
 **/
static int color_conversion_trigger_update(isp_color_conversion_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  int rc = 0;

  chromatix_parms_type *chromatix_ptr =
    in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr =
      &chromatix_ptr->chromatix_VFE.chromatix_CV;
  chromatix_gamma_type *chromatix_gamma =
    &(chromatix_ptr->chromatix_VFE.chromatix_gamma);
  int8_t update_cv = FALSE;
  aec_update_t *aec_params = &(in_params->trigger_input.stats_update.aec_update);
  trigger_ratio_t trigger_ratio;
  int is_burst = IS_BURST_STREAMING(&in_params->cfg);


  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  if (!mod->enable || !isp_util_aec_check_settled(aec_params) ||
      !mod->trigger_enable ||
      in_params->trigger_input.stats_update.awb_update.color_temp == 0) {
    ISP_DBG(ISP_MOD_COLOR_CONV, "%s: CV no trigger update required, enable = %d,"
      "trigger_enable = %d, aec_settled? %d\n", __func__, mod->enable,
      mod->trigger_enable, isp_util_aec_check_settled(aec_params));
    return 0;
  }

  /* Decide the trigger ratio for current lighting condition */
  rc = isp_util_get_aec_ratio2(mod->notify_ops->parent,
         chromatix_CV_ptr->control_cv,
         &(chromatix_gamma->gamma_outdoor_trigger),
         &(chromatix_CV_ptr->cv_trigger), aec_params, is_burst,
         &trigger_ratio);

  if (rc != 0)
    CDBG_ERROR("%s: get aec ratio error", __func__);

  update_cv = ((mod->old_streaming_mode != in_params->cfg.streaming_mode)
    || !F_EQUAL(trigger_ratio.ratio, mod->aec_ratio.ratio)
    || (trigger_ratio.lighting != mod->aec_ratio.lighting)
    || (mod->color_temp != in_params->trigger_input.stats_update.awb_update.color_temp));

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: update_cv %d ratio %f lighting %d colortemp %u", __func__,
    update_cv, trigger_ratio.ratio, trigger_ratio.lighting,
    in_params->trigger_input.stats_update.awb_update.color_temp);

  if (update_cv) {
    mod->cv_data = *(mod->p_cv);

    if(!IS_MANUAL_WB(in_params) && !F_EQUAL(trigger_ratio.ratio, 0.0))
      util_color_conversion_awb_update(mod, in_params, &mod->cv_data);

    util_color_conversion_aec_update(mod, in_params, &mod->cv_data, &trigger_ratio);
    mod->aec_ratio = trigger_ratio;
    mod->color_temp = in_params->trigger_input.stats_update.awb_update.color_temp;
    mod->old_streaming_mode = in_params->cfg.streaming_mode;
  }

  util_color_conversion_cmd_config(mod);
  mod->hw_update_pending = TRUE;

  return 0;
} /* color_conversion_trigger_update */

/** color_conversion_set_chromatix:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 * set chromatix info to modules
 *
 **/
static int color_conversion_set_chromatix(isp_color_conversion_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  chromatix_parms_type *chromatix_ptr =
    in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CV_type *chromatix_CV_ptr =
      &chromatix_ptr->chromatix_VFE.chromatix_CV;

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s:", __func__);
  SET_UNITY_MATRIX(mod->effects_matrix, 2);

  mod->cv_data = chromatix_CV_ptr->TL84_color_conversion;
  mod->p_cv = &chromatix_CV_ptr->TL84_color_conversion;

  mod->hw_update_pending = TRUE;

  return 0;
}

/** color_conversion_do_hw_update:
 *
 *    @mod:
 *
 * update module register to kernel
 *
 **/
static int color_conversion_do_hw_update(isp_color_conversion_mod_t *mod)
{
  int rc = 0;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &mod->RegCmd;
    cfg_cmd.cmd_len = sizeof(mod->RegCmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CC32_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_CC32_LEN * sizeof(uint32_t);

    util_color_conversion_cmd_debug(mod);

    rc = ioctl(mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    mod->applied_RegCmd = mod->RegCmd;
    mod->hw_update_pending = 0;
  }

  return rc;
} /* color_conversion_hw_reg_update */

/** color_conversion_set_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params,
 *    @in_param_size:
 *
 * set parameters from ISP
 *
 **/
static int color_conversion_set_params(void *mod_ctrl,
  uint32_t param_id, void *in_params, uint32_t in_param_size)
{
  isp_color_conversion_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_CHROMATIX_RELOAD: {
    rc = color_conversion_set_chromatix(mod,
           (isp_hw_pix_setting_params_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = color_conversion_enable(mod,
           (isp_mod_set_enable_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = color_conversion_config(mod,
           (isp_hw_pix_setting_params_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = color_conversion_trigger_enable(mod,
           (isp_mod_set_enable_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = color_conversion_trigger_update(mod,
           (isp_pix_trigger_update_input_t *)in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_BESTSHOT: {
    rc = color_conversion_set_bestshot(mod,
           (isp_hw_pix_setting_params_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_EFFECT: {
    rc = color_conversion_set_effect(mod,
           (isp_hw_pix_setting_params_t *) in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MANUAL_WB: {
    rc = color_conversion_set_manual_wb(mod,
           (isp_hw_pix_setting_params_t *) in_params, in_param_size);
  }
    break;

  default: {
    return -EAGAIN;
  }
    break;
  }

  return rc;
} /* color_conversion_set_params */

/** color_conversion_get_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params,
 *    @in_param_size:
 *    @iout_params,
 *    @out_param_size:
 *
 * get parameters
 *
 **/
static int color_conversion_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
{
  isp_color_conversion_mod_t *mod = mod_ctrl;
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
  }
    break;

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    chromaenhancement_t *colorconv = &vfe_diag->colorconv;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_COLOR_CONV, "%s: Populating vfe_diag data", __func__);
    if (NULL == colorconv || NULL == mod ) {
      CDBG_ERROR("%s: NULL colorconv %x mod %x", __func__,
        (unsigned int)colorconv, (unsigned int)mod);
      break;
    }
    vfe_diag->control_colorconv.enable = mod->enable;
    vfe_diag->control_colorconv.cntrlenable = mod->trigger_enable;
    colorconv->param_ap = mod->applied_RegCmd.ap;
    colorconv->param_am = mod->applied_RegCmd.am;
    colorconv->param_bp = mod->applied_RegCmd.bp;
    colorconv->param_bm = mod->applied_RegCmd.bm;
    colorconv->param_cp = mod->applied_RegCmd.cp;
    colorconv->param_cm = mod->applied_RegCmd.cm;
    colorconv->param_dp = mod->applied_RegCmd.dp;
    colorconv->param_dm = mod->applied_RegCmd.dm;
    colorconv->param_kcb = mod->applied_RegCmd.kcb;
    colorconv->param_kcr = mod->applied_RegCmd.kcr;
    colorconv->param_rtoy = mod->applied_RegCmd.RGBtoYConversionV0;
    /*TODO: confirm below register values and vfe diag params are matching*/
    colorconv->param_gtoy = mod->applied_RegCmd.RGBtoYConversionV1;
    colorconv->param_btoy = mod->applied_RegCmd.RGBtoYConversionV2;
    colorconv->param_yoffset =
      mod->applied_RegCmd.RGBtoYConversionOffset;
  }
    break;

  default: {
    rc = -1;
  }
    break;
  }

  return rc;
} /* color_conversion_get_params */

/** color_conversion_action:
 *
 *    @mod_ctrl:
 *    @action_code:
 *    @data,
 *    @data_size:
 *
 *  processing the action
 *
 **/
static int color_conversion_action(void *mod_ctrl, uint32_t action_code,
  void *data, uint32_t data_size)
{
  int rc = 0;
  isp_color_conversion_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = color_conversion_do_hw_update(mod);
  }
    break;

  default: {
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
  }
    break;
  }

  return rc;
} /* color_conversion_action */

/** chroma_enhan32_open:
 *
 *    @version:
 *
 *  open mod, create func table
 *
 **/
isp_ops_t *chroma_enhan32_open(uint32_t version)
{
  isp_color_conversion_mod_t *mod = malloc(sizeof(isp_color_conversion_mod_t));

  ISP_DBG(ISP_MOD_COLOR_CONV, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }

  memset(mod,  0,  sizeof(isp_color_conversion_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = color_conversion_init;
  mod->ops.destroy = color_conversion_destroy;
  mod->ops.set_params = color_conversion_set_params;
  mod->ops.get_params = color_conversion_get_params;
  mod->ops.action = color_conversion_action;

  return &mod->ops;
} /* chroma_enhan32_open */
