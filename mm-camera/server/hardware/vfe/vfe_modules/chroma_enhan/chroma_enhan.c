/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CV_LOGGING
  #undef CDBG
  #define CDBG LOGE
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

#define CV_START_REG 0x3C4

/*===========================================================================
 * FUNCTION    - vfe_chroma_enhan_init -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_chroma_enhan_init(int mod_id, void *mod_cv,
  void *vparms)
{
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *parms = (vfe_params_t *)vparms;
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  mod->cv_data = parms->chroma3a->chromatix_tl84_color_conversion;
  mod->p_cv = &parms->chroma3a->chromatix_tl84_color_conversion;
  mod->trigger_enable = TRUE;
  return VFE_SUCCESS;
} /* vfe_chroma_enhan_init */

/*===========================================================================
 * FUNCTION.   - vfe_chroma_enhan_config_debug -
 *
 * DESCRIPTION
 *========================================================================*/
void vfe_chroma_enhan_config_debug(chroma_enhan_mod_t *mod)
{
  CDBG("mod->chroma_enhan_cmd.am = %d\n",
    mod->chroma_enhan_cmd.am);
  CDBG("mod->chroma_enhan_cmd.bm = %d\n",
    mod->chroma_enhan_cmd.bm);
  CDBG("mod->chroma_enhan_cmd.cm = %d\n",
    mod->chroma_enhan_cmd.cm);
  CDBG("mod->chroma_enhan_cmd.dm = %d\n",
    mod->chroma_enhan_cmd.dm);
  CDBG("mod->chroma_enhan_cmd.ap = %d\n",
    mod->chroma_enhan_cmd.ap);
  CDBG("mod->chroma_enhan_cmd.bp = %d\n",
    mod->chroma_enhan_cmd.bp);
  CDBG("mod->chroma_enhan_cmd.cp = %d\n",
    mod->chroma_enhan_cmd.cp);
  CDBG("mod->chroma_enhan_cmd.dp = %d\n",
    mod->chroma_enhan_cmd.dp);
  CDBG("mod->chroma_enhan_cmd.RGBtoYConversionV0 = %d\n",
    mod->chroma_enhan_cmd.RGBtoYConversionV0);
  CDBG("mod->chroma_enhan_cmd.RGBtoYConversionV1 = %d\n",
    mod->chroma_enhan_cmd.RGBtoYConversionV1);
  CDBG("mod->chroma_enhan_cmd.RGBtoYConversionV2 = %d\n",
    mod->chroma_enhan_cmd.RGBtoYConversionV2);
  CDBG("mod->chroma_enhan_cmd.RGBtoYConversionOffset = %d\n",
    mod->chroma_enhan_cmd.RGBtoYConversionOffset);
  CDBG("mod->chroma_enhan_cmd.kcb = %d\n",
    mod->chroma_enhan_cmd.kcb);
  CDBG("mod->chroma_enhan_cmd.kcr = %d\n",
    mod->chroma_enhan_cmd.kcr);
} /* vfe_chroma_enhan_config_debug */

/*===========================================================================
 * FUNCTION.   - vfe_color_conversion_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_color_conversion_enable(int mod_id, void *mod_cv, void *vparms,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *params = (vfe_params_t *)vparms;

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
  return status;
} /* vfe_color_conversion_enable */

/*===========================================================================
 * FUNCTION.   - vfe_color_conversion_config -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_color_conversion_config(int mod_id, void *mod_cv,
  void *vparms)
{
  double a_new, b_new, c_new, d_new;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *p_obj = (vfe_params_t *)vparms;

  chromatix_color_conversion_type *cc = &(mod->cv_data);

  if (!mod->cv_enable) {
    CDBG("%s: CV not enabled", __func__);
    return VFE_SUCCESS;
  }
  CDBG("vfe_color_conversion_config: %f, %f; %f, %f\n",
    mod->effects_matrix[0][0], mod->effects_matrix[0][1], mod->effects_matrix[1][0],
    mod->effects_matrix[1][1]);

  a_new = cc->chroma.a_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[0][1];
  b_new = cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * mod->effects_matrix[0][1];
  c_new = cc->chroma.c_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[1][0];
  d_new = cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * mod->effects_matrix[1][0];

  if (a_new)
    b_new /= a_new;
  if (c_new)
    d_new /= c_new;

  if (fabs(a_new) >= 4)
    CDBG("overflow a_p_new = %f\n", a_new);
  if (fabs(b_new) >= 4)
    CDBG("overflow b_p_new = %f\n", b_new);
  if (fabs(c_new) >= 4)
    CDBG("overflow c_p_new = %f\n", c_new);
  if (fabs(d_new) >= 4)
    CDBG("overflow d_p_new = %f\n", d_new);

  mod->chroma_enhan_cmd.am = FLOAT_TO_Q(8, a_new);
  mod->chroma_enhan_cmd.bm = FLOAT_TO_Q(8, b_new);
  mod->chroma_enhan_cmd.cm = FLOAT_TO_Q(8, c_new);
  mod->chroma_enhan_cmd.dm = FLOAT_TO_Q(8, d_new);

  a_new = cc->chroma.a_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[0][1];
  b_new = cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * mod->effects_matrix[0][1];
  c_new = cc->chroma.c_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[1][0];
  d_new = cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * mod->effects_matrix[1][0];

  if (a_new)
    b_new /= a_new;
  if (c_new)
    d_new /= c_new;

  if (fabs(a_new) >= 4) {
    CDBG("overflow a_m_new = %f\n", a_new);
  }
  if (fabs(b_new) >= 4) {
    CDBG("overflow b_m_new = %f\n", b_new);
  }
  if (fabs(c_new) >= 4) {
    CDBG("overflow c_m_new = %f\n", c_new);
  }
  if (fabs(d_new) >= 4) {
    CDBG("overflow d_m_new = %f\n", d_new);
  }

  mod->chroma_enhan_cmd.ap = FLOAT_TO_Q(8, a_new);
  mod->chroma_enhan_cmd.bp = FLOAT_TO_Q(8, b_new);
  mod->chroma_enhan_cmd.cp = FLOAT_TO_Q(8, c_new);
  mod->chroma_enhan_cmd.dp = FLOAT_TO_Q(8, d_new);

  mod->chroma_enhan_cmd.kcb = cc->chroma.k_cb;
  mod->chroma_enhan_cmd.kcr = cc->chroma.k_cr;
  mod->chroma_enhan_cmd.RGBtoYConversionV0 =
    FLOAT_TO_Q(8, cc->luma.v0);

  mod->chroma_enhan_cmd.RGBtoYConversionV1 =
    FLOAT_TO_Q(8, cc->luma.v1);

  mod->chroma_enhan_cmd.RGBtoYConversionV2 =
    FLOAT_TO_Q(8, cc->luma.v2);

  mod->chroma_enhan_cmd.RGBtoYConversionOffset =
    cc->luma.k;

  vfe_chroma_enhan_config_debug(mod);

#ifndef VFE_2X
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd ,CMD_GENERAL,
    (void *) &(mod->chroma_enhan_cmd), sizeof(mod->chroma_enhan_cmd),
    VFE_CMD_CHROMA_EN_CFG)) {
    CDBG_HIGH("%s: Color Conv config for operation mode = %d failed\n",
      __func__, p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
#endif

  mod->cv_update = FALSE;
  mod->cv_data = *cc;
  return VFE_SUCCESS;
} /* vfe_color_conversion_config */

/*===========================================================================
 * FUNCTION.   - vfe_color_conversion_update -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_color_conversion_update(int mod_id, void *mod_cv,
  void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  double a_new, b_new, c_new, d_new;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *p_obj = (vfe_params_t *)vparms;
  chromatix_color_conversion_type *cc = &(mod->cv_data);

#ifndef VFE_2X
  if (mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(p_obj->camfd,
      CMD_GENERAL, p_obj->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    p_obj->update |= VFE_MOD_COLOR_CONV;
    mod->hw_enable = FALSE;
  }
#endif

  if (!mod->cv_enable) {
    CDBG("%s: CV not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!mod->cv_update) {
    CDBG("%s: CV not updated", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s: %f, %f; %f, %f\n", __func__,
    mod->effects_matrix[0][0], mod->effects_matrix[0][1], mod->effects_matrix[1][0],
    mod->effects_matrix[1][1]);

  a_new = cc->chroma.a_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[0][1];
  c_new = cc->chroma.c_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[1][0];

  b_new = cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[0][0] +
   cc->chroma.c_m * mod->effects_matrix[0][1];
  if (a_new)
   b_new /= a_new;

  d_new = cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * mod->effects_matrix[1][0];
  if (c_new)
    d_new /= c_new;

  if (fabs(a_new) >= 4) {
    CDBG("overflow a_p_new = %f\n", a_new);
  }
  if (fabs(b_new) >= 4) {
    CDBG("overflow b_p_new = %f\n", b_new);
  }
  if (fabs(c_new) >= 4) {
    CDBG("overflow c_p_new = %f\n", c_new);
  }
  if (fabs(d_new) >= 4) {
    CDBG("overflow d_p_new = %f\n", d_new);
  }

  mod->chroma_enhan_cmd.am = FLOAT_TO_Q(8, a_new);
  mod->chroma_enhan_cmd.bm = FLOAT_TO_Q(8, b_new);
  mod->chroma_enhan_cmd.cm = FLOAT_TO_Q(8, c_new);
  mod->chroma_enhan_cmd.dm = FLOAT_TO_Q(8, d_new);

  a_new = cc->chroma.a_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[0][1];
  c_new = cc->chroma.c_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[1][0];

  b_new = cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * mod->effects_matrix[0][1];
  if (a_new)
    b_new /= a_new;

  d_new = cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * mod->effects_matrix[1][0];
  if (c_new)
    d_new /= c_new;

  if (fabs(a_new) >= 4)
    CDBG("overflow a_m_new = %f\n", a_new);
  if (fabs(b_new) >= 4)
    CDBG("overflow b_m_new = %f\n", b_new);
  if (fabs(c_new) >= 4)
    CDBG("overflow c_m_new = %f\n", c_new);
  if (fabs(d_new) >= 4)
    CDBG("overflow d_m_new = %f\n", d_new);

  mod->chroma_enhan_cmd.ap = FLOAT_TO_Q(8, a_new);
  mod->chroma_enhan_cmd.bp = FLOAT_TO_Q(8, b_new);
  mod->chroma_enhan_cmd.cp = FLOAT_TO_Q(8, c_new);
  mod->chroma_enhan_cmd.dp = FLOAT_TO_Q(8, d_new);

  mod->chroma_enhan_cmd.kcb = cc->chroma.k_cb;
  mod->chroma_enhan_cmd.kcr = cc->chroma.k_cr;
  mod->chroma_enhan_cmd.RGBtoYConversionV0 = FLOAT_TO_Q(8, cc->luma.v0);
  mod->chroma_enhan_cmd.RGBtoYConversionV1 = FLOAT_TO_Q(8, cc->luma.v1);
  mod->chroma_enhan_cmd.RGBtoYConversionV2 = FLOAT_TO_Q(8, cc->luma.v2);
  mod->chroma_enhan_cmd.RGBtoYConversionOffset = cc->luma.k;

  vfe_chroma_enhan_config_debug(mod);

#ifndef VFE_2X
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd, CMD_GENERAL,
    (void *) &(mod->chroma_enhan_cmd), sizeof(mod->chroma_enhan_cmd),
    VFE_CMD_CHROMA_EN_UPDATE)) {
    CDBG_ERROR("%s: Color Conv update for operation mode = %d failed\n",
    __func__, p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
#endif
  if (VFE_SUCCESS == status) {
    mod->cv_update = FALSE;
    p_obj->update |= VFE_MOD_COLOR_CONV;
  }
  return status;
} /* vfe_color_conversion_update */

/*===========================================================================
 * FUNCTION    - cv_interpolate -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void cv_interpolate(chromatix_color_conversion_type* in1,
  chromatix_color_conversion_type *in2, chromatix_color_conversion_type* out,
  float ratio)
{
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
} /* cv_interpolate */

/*===========================================================================
 * FUNCTION. ..- color_conversion_aec_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void color_conversion_aec_update(chroma_enhan_mod_t *mod,
  vfe_params_t *params,  chromatix_color_conversion_type* p_cv_data,
  trigger_ratio_t *p_trigger_ratio)
{
  chromatix_parms_type *chroma_ptr = params->chroma3a;

  switch (p_trigger_ratio->lighting) {
   case TRIGGER_LOWLIGHT:
     cv_interpolate(p_cv_data, &chroma_ptr->chromatix_yhi_ylo_color_conversion,
       &mod->cv_data, p_trigger_ratio->ratio);
     break;
   case TRIGGER_OUTDOOR:
     cv_interpolate(p_cv_data, &chroma_ptr->chromatix_outdoor_color_conversion,
       &mod->cv_data, p_trigger_ratio->ratio);
     break;
   default:
   case TRIGGER_NORMAL:
     mod->cv_data = *p_cv_data;
     break;
  }
} /* color_conversion_aec_update */


/*===========================================================================
 * FUNCTION    - color_conversion_awb_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void color_conversion_awb_update(chroma_enhan_mod_t *mod,
  vfe_params_t* params, chromatix_color_conversion_type *cv_data)
{
  vfe_status_t status = VFE_SUCCESS;
  cct_trigger_info trigger_info;
  float ratio = 0.0;
  chromatix_parms_type *chroma_ptr = params->chroma3a;

  trigger_info.mired_color_temp = MIRED(params->awb_params.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    chroma_ptr->CV_Daylight_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
    chroma_ptr->CV_A_trigger);
  awb_cct_type cct_type = vfe_util_get_awb_cct_type(&trigger_info, params);

  CDBG("%s: CCT type %d", __func__, cct_type);

  chromatix_color_conversion_type *cv_table_A, *cv_table_TL84, *cv_table_D65;
  if (params->bs_mode == CAMERA_BESTSHOT_PORTRAIT) {
    cv_table_A = &chroma_ptr->skintone_color_conversion_a;
    cv_table_TL84 = &chroma_ptr->skintone_color_conversion;
    cv_table_D65 = &chroma_ptr->skintone_color_conversion_d65;
  }
  else {
    cv_table_A = &chroma_ptr->chromatix_incandescent_color_conversion;
    cv_table_TL84 = &chroma_ptr->chromatix_tl84_color_conversion;
    cv_table_D65 =   &chroma_ptr->chromatix_daylight_color_conversion;
  }

  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      *cv_data = *cv_table_A;
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
      cv_interpolate(cv_table_TL84, cv_table_A, cv_data, ratio);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_start,
        trigger_info.trigger_d65.mired_end);
      cv_interpolate(cv_table_D65,cv_table_TL84, cv_data, ratio);
      break;
    case AWB_CCT_TYPE_D65:
      *cv_data = *cv_table_D65;
      break;
    default:
    case AWB_CCT_TYPE_TL84:
      *cv_data = *cv_table_TL84;
      break;
  }
}

/*===========================================================================
 * FUNCTION    - vfe_color_conversion_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_conversion_trigger_update(int mod_id, void *mod_cv,
  void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t update_cv = FALSE;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *params = (vfe_params_t *)vparams;

  vfe_aec_parms_t *aec_obj = &(params->aec_params);
  chromatix_parms_type *chroma_ptr = params->chroma3a;
  trigger_ratio_t trigger_ratio;

  CDBG("%s: trig_enable %d", __func__, mod->trigger_enable);
  if (!mod->cv_enable || !vfe_util_aec_check_settled(aec_obj)
    || !mod->trigger_enable) {
    CDBG("%s: CV no trigger update required", __func__);
    return status;
  }

  /* Decide the trigger ratio for current lighting condition */
  trigger_ratio = vfe_util_get_aec_ratio2(chroma_ptr->control_cv,
    &(chroma_ptr->gamma_outdoor_trigger), &(chroma_ptr->cv_trigger), params);

  update_cv = ((mod->cur_mode != params->vfe_op_mode)
    || !F_EQUAL(trigger_ratio.ratio, mod->curr_ratio.ratio)
    || (trigger_ratio.lighting != mod->curr_ratio.lighting)
    || (mod->color_temp != params->awb_params.color_temp)
    || mod->reload_params);

  CDBG("%s: update_cv %d ratio %f lighting %d colortemp %u", __func__,
    update_cv, trigger_ratio.ratio, trigger_ratio.lighting,
    params->awb_params.color_temp);

  if (update_cv) {
    mod->cv_data = *(mod->p_cv);
    if(!IS_MANUAL_WB(params) && !F_EQUAL(trigger_ratio.ratio, 0.0))
      color_conversion_awb_update(mod, params, &mod->cv_data);

    color_conversion_aec_update(mod, params, &mod->cv_data, &trigger_ratio);
    mod->curr_ratio = trigger_ratio;
    mod->color_temp = params->awb_params.color_temp;
    mod->reload_params = FALSE;
    mod->cv_update = TRUE;
  }
  mod->cur_mode = params->vfe_op_mode;
  return status;
} /* vfe_color_conversion_trigger_update */


/*===========================================================================
 * FUNCTION    - vfe_color_conversion_set_effect -
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_color_conversion_set_effect(int mod_id, void *mod_cv,
  void *vparms, vfe_effects_type_t type)
{
  int i = 0, j = 0;
  vfe_status_t status = VFE_SUCCESS;
  CDBG("%s",__func__);
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *params = (vfe_params_t *)vparms;
  float hue_matrix[2][2];
  float sat_matrix[2][2];
  float s, hue_in_radian;

  s = 2.0 * params->effects_params.saturation;
  hue_in_radian = DEGREE_TO_RADIAN(params->effects_params.hue * 10);

  switch(type) {
    case VFE_SATURATION: {
       if (!params->use_cv_for_hue_sat &&
         F_EQUAL(params->effects_params.hue, 0))
         SET_SAT_MATRIX(mod->effects_matrix, s);
       break;
    }
    case VFE_HUE:
      if (!params->use_cv_for_hue_sat)
        SET_HUE_MATRIX(mod->effects_matrix, hue_in_radian);
      break;
    default:
      return VFE_SUCCESS;
  }

  if (params->use_cv_for_hue_sat) {
    SET_UNITY_MATRIX(mod->effects_matrix, 2);
    SET_SAT_MATRIX(sat_matrix, s);

    if (F_EQUAL(params->effects_params.hue, 0))
      SET_UNITY_MATRIX(hue_matrix, 2);
    else
      SET_HUE_MATRIX(hue_matrix, hue_in_radian);

    MATRIX_MULT(sat_matrix, hue_matrix, mod->effects_matrix, 2, 2, 2);
  }

#ifdef ENABLE_CV_LOGGING
  PRINT_2D_MATRIX(2, 2, mod->effects_matrix);
#endif

  mod->cv_update = TRUE;
  return status;
}/*vfe_color_conversion_set_effect*/

/*===========================================================================
 * FUNCTION.   - vfe_color_conversion_set_manual_wb -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_color_conversion_set_manual_wb(int mod_id, void *mod_cv,
  void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *parms = (vfe_params_t *)vparms;

  chromatix_parms_type *chroma_ptr= parms->chroma3a;
  CDBG("%s: wb %d", __func__, parms->wb);

  switch(parms->wb) {
    case CAMERA_WB_INCANDESCENT:
      mod->p_cv = &chroma_ptr->chromatix_incandescent_color_conversion;
      break;
    case CAMERA_WB_CLOUDY_DAYLIGHT:
    case CAMERA_WB_DAYLIGHT:
      mod->p_cv = &chroma_ptr->chromatix_daylight_color_conversion;
      break;
    default:
      mod->p_cv = &chroma_ptr->chromatix_tl84_color_conversion;
      break;
  }
  mod->cv_data = *(mod->p_cv);
  mod->cv_update = TRUE;
  return status;
} /* vfe_color_conversion_set_manual_wb */

/*===========================================================================
 * FUNCTION.   - vfe_color_conversion_set_effect -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_color_conversion_set_spl_effect(int mod_id, void *mod_cv,
  void *vparms, vfe_spl_effects_type effects)
{
  vfe_status_t status = VFE_SUCCESS;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *parms = (vfe_params_t *)vparms;
  chromatix_color_conversion_type chromatix_spl_effect_color_conversion;

  chromatix_parms_type *chroma_ptr= parms->chroma3a;
  mod->cv_update = TRUE;
  CDBG("%s: apply %d", __func__, effects);
  /* reset hue/saturation */
  SET_UNITY_MATRIX(mod->effects_matrix, 2);

  switch(effects) {
    case CAMERA_EFFECT_MONO:
      mod->cv_data = chroma_ptr->chromatix_mono_color_conversion;
      mod->trigger_enable = FALSE;
      break;
    case CAMERA_EFFECT_NEGATIVE:
      mod->cv_data = chroma_ptr->chromatix_negative_color_conversion;
#ifdef VFE_2X
      mod->cv_data.luma.k = 4095;
#endif
      mod->trigger_enable = FALSE;
      break;
    case CAMERA_EFFECT_SEPIA:
      mod->cv_data = chroma_ptr->chromatix_sepia_color_conversion;
      mod->trigger_enable = FALSE;
      break;
    case CAMERA_EFFECT_AQUA:
      mod->cv_data = chroma_ptr->chromatix_aqua_color_conversion;
      mod->trigger_enable = FALSE;
      break;
    case CAMERA_EFFECT_VINTAGECOOL:
      memcpy(&chromatix_spl_effect_color_conversion, &mod->cv_data,
        sizeof(chromatix_color_conversion_type));
      mod->cv_data = chromatix_spl_effect_color_conversion;
      mod->trigger_enable = FALSE;
      mod->cv_data.chroma.k_cb = 137;
      mod->cv_data.chroma.k_cr = 128;
      break;
    case CAMERA_EFFECT_VINTAGEWARM:
      memcpy(&chromatix_spl_effect_color_conversion, &mod->cv_data,
        sizeof(chromatix_color_conversion_type));
      mod->cv_data = chromatix_spl_effect_color_conversion;
      mod->trigger_enable = FALSE;
      mod->cv_data.chroma.k_cb = 118;
      mod->cv_data.chroma.k_cr = 140;
      break;
    case CAMERA_EFFECT_FADED:
      memcpy(&chromatix_spl_effect_color_conversion, &mod->cv_data,
        sizeof(chromatix_color_conversion_type));
      mod->cv_data = chromatix_spl_effect_color_conversion;
      mod->trigger_enable = FALSE;
      mod->cv_data.chroma.a_m *= 0.6;
      mod->cv_data.chroma.a_p *= 0.6;
      mod->cv_data.chroma.c_m *= 0.6;
      mod->cv_data.chroma.c_p *= 0.6;
      break;
    default:
      mod->cv_data = chroma_ptr->chromatix_tl84_color_conversion;
      mod->trigger_enable = TRUE;
      break;
  }

  mod->cv_update = TRUE;
  return status;
} /* vfe_color_conversion_set_effect */

/*===========================================================================
 * FUNCTION    - vfe_color_conversion_set_bestshot -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_color_conversion_set_bestshot(int mod_id, void *mod_cv,
  void *vparms, camera_bestshot_mode_type mode)
{
  vfe_status_t status = VFE_SUCCESS;
  float s = 0.0;
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *params = (vfe_params_t *)vparms;

  chromatix_parms_type *chroma_ptr= params->chroma3a;

  mod->trigger_enable = FALSE;
  CDBG("%s: mode %d", __func__, mode);
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  switch (mode) {
    case CAMERA_BESTSHOT_THEATRE:
    case CAMERA_BESTSHOT_CANDLELIGHT:
    case CAMERA_BESTSHOT_SUNSET:
      mod->cv_data = chroma_ptr->sunset_color_conversion;
      break;
    case CAMERA_BESTSHOT_FIREWORKS:
      /* from WB need to check */
      mod->cv_data = chroma_ptr->chromatix_daylight_color_conversion;
      break;
    case CAMERA_BESTSHOT_FLOWERS:
    case CAMERA_BESTSHOT_PARTY:
      /* apply required saturation */
      s = chroma_ptr->saturated_color_conversion_factor;
      /* Just update color conversion matrix in this case */
      SET_UNITY_MATRIX(mod->effects_matrix, 2);
      SET_SAT_MATRIX(mod->effects_matrix, s);
      mod->trigger_enable = TRUE;
      break;
    case CAMERA_BESTSHOT_PORTRAIT:
    case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
      //dynamically update in awb update
    case BESTSHOT_COLOR_CONVERSION_NORMAL:
    default:
      mod->cv_data = chroma_ptr->chromatix_tl84_color_conversion;
      mod->trigger_enable = TRUE;
      break;
  }
  mod->cv_update = TRUE;
  return status;
} /* vfe_color_conv_set_bestshot */

/*===========================================================================
 * FUNCTION    - vfe_color_conversion_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_conversion_trigger_enable(int mod_id, void* mod_cv,
  void* params, int enable)
{
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  CDBG("%s: %d", __func__, enable);
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_color_conversion_trigger_enable*/

/*===========================================================================
 * FUNCTION    - vfe_color_conversion_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_conversion_reload_params(int mod_id, void *mod_cv,
  void *vparms)
{
  chroma_enhan_mod_t *mod = (chroma_enhan_mod_t *)mod_cv;
  vfe_params_t *params = (vfe_params_t *)vparms;
  mod->reload_params = TRUE;
  CDBG("%s:", __func__);
  vfe_status_t status = VFE_SUCCESS;
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  if (params->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: update BSM in reload for mode %d",__func__,params->bs_mode);
    status = vfe_color_conversion_set_bestshot(mod_id, mod, params,
               params->bs_mode);
  }
  else {
  mod->cv_data = params->chroma3a->chromatix_tl84_color_conversion;
  mod->p_cv = &params->chroma3a->chromatix_tl84_color_conversion;
  }
  return status;
} /*vfe_color_conversion_reload_params*/

/*===========================================================================
 * FUNCTION    - color_conversion_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void color_conversion_populate_data(uint32_t *reg,
  VFE_Chroma_Enhance_CfgCmdType *pcmd)
{
  reg += (CV_START_REG/4);
  memcpy((void *)pcmd, (void *)reg, sizeof(VFE_Chroma_Enhance_CfgCmdType));
}/*color_conversion_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_color_conversion_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_conversion_tv_validate(int mod_id, void *in, void *op)
{
  VFE_Chroma_Enhance_CfgCmdType in_cv, out_cv;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)op;
  color_conversion_populate_data(input->reg_dump, &in_cv);
  color_conversion_populate_data(output->reg_dump_data, &out_cv);

  if (!MATCH(in_cv.RGBtoYConversionV0, out_cv.RGBtoYConversionV0, 0))
    CDBG_TV("%s: RGBtoYConversionV0 in %d out %d doesnt match", __func__,
    in_cv.RGBtoYConversionV0, out_cv.RGBtoYConversionV0);

  if (!MATCH(in_cv.RGBtoYConversionV1, out_cv.RGBtoYConversionV1, 0))
    CDBG_TV("%s: RGBtoYConversionV1 in %d out %d doesnt match", __func__,
    in_cv.RGBtoYConversionV1, out_cv.RGBtoYConversionV1);

  if (!MATCH(in_cv.RGBtoYConversionV2, out_cv.RGBtoYConversionV2, 0))
    CDBG_TV("%s: RGBtoYConversionV2 in %d out %d doesnt match", __func__,
    in_cv.RGBtoYConversionV2, out_cv.RGBtoYConversionV2);

  if (!MATCH(in_cv.RGBtoYConversionOffset, out_cv.RGBtoYConversionOffset, 0))
    CDBG_TV("%s: RGBtoYConversionOffset in %d out %d doesnt match", __func__,
    in_cv.RGBtoYConversionOffset, out_cv.RGBtoYConversionOffset);

  if (!MATCH(in_cv.ap, out_cv.ap, 0))
    CDBG_TV("%s: ap in %d out %d doesnt match", __func__,
    in_cv.ap, out_cv.ap);

  if (!MATCH(in_cv.am, out_cv.am, 0))
    CDBG_TV("%s: am in %d out %d doesnt match", __func__,
    in_cv.am, out_cv.am);

  if (!MATCH(in_cv.bp, out_cv.bp, 0))
    CDBG_TV("%s: bp in %d out %d doesnt match", __func__,
    in_cv.bp, out_cv.bp);

  if (!MATCH(in_cv.bm, out_cv.bm, 0))
    CDBG_TV("%s: bm in %d out %d doesnt match", __func__,
    in_cv.bm, out_cv.bm);

  if (!MATCH(in_cv.cp, out_cv.cp, 0))
    CDBG_TV("%s: cp in %d out %d doesnt match", __func__,
    in_cv.cp, out_cv.cp);

  if (!MATCH(in_cv.cm, out_cv.cm, 0))
    CDBG_TV("%s: cm in %d out %d doesnt match", __func__,
    in_cv.cm, out_cv.cm);

  if (!MATCH(in_cv.dp, out_cv.dp, 0))
    CDBG_TV("%s: dp in %d out %d doesnt match", __func__,
    in_cv.dp, out_cv.dp);

  if (!MATCH(in_cv.dm, out_cv.dm, 0))
    CDBG_TV("%s: dm in %d out %d doesnt match", __func__,
    in_cv.dm, out_cv.dm);

  if (!MATCH(in_cv.kcb, out_cv.kcb, 0))
    CDBG_TV("%s: kcb in %d out %d doesnt match", __func__,
    in_cv.kcb, out_cv.kcb);

  if (!MATCH(in_cv.kcr, out_cv.kcr, 0))
    CDBG_TV("%s: kcr in %d out %d doesnt match", __func__,
    in_cv.kcr, out_cv.kcr);

  return VFE_SUCCESS;
}/*vfe_color_conversion_tv_validate*/

/*===========================================================================
 * FUNCTION    - vfe_chroma_enhan_deinit -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_chroma_enhan_deinit(int mod_id, void *mod_cv,
  void *vparms)
{
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_chroma_enhan_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_conv_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  chroma_enhan_module_t *cmd =
    (chroma_enhan_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&(cmd->chroma_enhan_cmd), sizeof(VFE_Chroma_Enhance_CfgCmdType),
     VFE_CMD_CHROMA_EN_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
#endif
