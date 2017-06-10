/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define MAX_CC_GAIN 3.9
#define CC_START_REG 0x388

#define GET_CC_MATRIX(CC, M) ({ \
  M[0][0] = CC->c0; \
  M[0][1] = CC->c1; \
  M[0][2] = CC->c2; \
  M[1][0] = CC->c3; \
  M[1][1] = CC->c4; \
  M[1][2] = CC->c5; \
  M[2][0] = CC->c6; \
  M[2][1] = CC->c7; \
  M[2][2] = CC->c8; })

#define SET_VFE_CC_MATRIX(CC, M, q) ({ \
  CC->C0 = M[1][1]; \
  CC->C1 = M[1][2]; \
  CC->C2 = M[1][0]; \
  CC->C3 = M[2][1]; \
  CC->C4 = M[2][2]; \
  CC->C5 = M[2][0]; \
  CC->C6 = M[0][1]; \
  CC->C7 = M[0][2]; \
  CC->C8 = M[0][0]; })

#define SET_VFE_CC_MATRIX1(CC, M, q) ({ \
  CC->C0 = M[0][0]; \
  CC->C1 = M[0][1]; \
  CC->C2 = M[0][2]; \
  CC->C3 = M[1][0]; \
  CC->C4 = M[1][1]; \
  CC->C5 = M[1][2]; \
  CC->C6 = M[2][0]; \
  CC->C7 = M[2][1]; \
  CC->C8 = M[2][2]; })

#define CC_APPLY_GAIN(cc, gain) ({ \
  cc->c0 *= gain; \
  cc->c1 *= gain; \
  cc->c2 *= gain; \
  cc->c3 *= gain; \
  cc->c4 *= gain; \
  cc->c5 *= gain; \
  cc->c6 *= gain; \
  cc->c7 *= gain; \
  cc->c8 *= gain; \
  cc->k0 *= gain; \
  cc->k1 *= gain; \
  cc->k2 *= gain; \
})

/*===========================================================================
 * Function:           vfe_color_correct_convert_table
 *
 * Description:
===========================================================================*/
void vfe_color_correct_convert_table(
chromatix_color_correction_type *pInCC,
  color_correct_type* pOutCC)
{
  pOutCC->c0 = CC_COEFF(pInCC->c0, pInCC->q_factor+7);
  pOutCC->c1 = CC_COEFF(pInCC->c1, pInCC->q_factor+7);
  pOutCC->c2 = CC_COEFF(pInCC->c2, pInCC->q_factor+7);
  pOutCC->c3 = CC_COEFF(pInCC->c3, pInCC->q_factor+7);
  pOutCC->c4 = CC_COEFF(pInCC->c4, pInCC->q_factor+7);
  pOutCC->c5 = CC_COEFF(pInCC->c5, pInCC->q_factor+7);
  pOutCC->c6 = CC_COEFF(pInCC->c6, pInCC->q_factor+7);
  pOutCC->c7 = CC_COEFF(pInCC->c7, pInCC->q_factor+7);
  pOutCC->c8 = CC_COEFF(pInCC->c8, pInCC->q_factor+7);
  pOutCC->k0 = pInCC->k0;
  pOutCC->k1 = pInCC->k1;
  pOutCC->k2 = pInCC->k2;
  pOutCC->q_factor = pInCC->q_factor+7;
}

/*===========================================================================
 * Function:           vfe_color_correct_convert_table_all
 *
 * Description:
===========================================================================*/
void vfe_color_correct_convert_table_all(color_correct_mod_t *mod,
  vfe_params_t *params)
{
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_TL84_color_correction_snapshot,
    &mod->table.chromatix_TL84_color_correction_snapshot);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_yhi_ylo_color_correction_snapshot,
    &mod->table.chromatix_yhi_ylo_color_correction_snapshot);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_D65_color_correction_snapshot,
    &mod->table.chromatix_D65_color_correction_snapshot);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_A_color_correction_snapshot,
    &mod->table.chromatix_A_color_correction_snapshot);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_LED_color_correction_snapshot,
    &mod->table.chromatix_LED_color_correction_snapshot);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_STROBE_color_correction,
    &mod->table.chromatix_STROBE_color_correction);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_TL84_color_correction,
    &mod->table.chromatix_TL84_color_correction);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_yhi_ylo_color_correction,
    &mod->table.chromatix_yhi_ylo_color_correction);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_LED_color_correction_VF,
    &mod->table.chromatix_LED_color_correction_VF);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_D65_color_correction_VF,
    &mod->table.chromatix_D65_color_correction_VF);
  vfe_color_correct_convert_table(
    &params->chroma3a->chromatix_A_color_correction_VF,
    &mod->table.chromatix_A_color_correction_VF);

}/*vfe_color_correct_convert_table_all*/

/*===========================================================================
 * Function:           vfe_color_correct_debug
 *
 * Description:
===========================================================================*/
void vfe_color_correct_debug(VFE_ColorCorrectionCfgCmdType* p_cmd)
{
  CDBG("Color correction coefQFactor = %d\n",
    p_cmd->coefQFactor);
  CDBG("Color correction  C[0-8] = %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
    p_cmd->C0,
    p_cmd->C1,
    p_cmd->C2,
    p_cmd->C3,
    p_cmd->C4,
    p_cmd->C5,
    p_cmd->C6,
    p_cmd->C7,
    p_cmd->C8);
  CDBG("Color correction K[0-2] = %d, %d, %d\n",
    p_cmd->K0,
    p_cmd->K1,
    p_cmd->K2);
}/*vfe_color_correct_debug*/

/*===========================================================================
 * Function:           vfe_set_color_correction_params
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_set_color_correction_params(
  VFE_ColorCorrectionCfgCmdType* p_cmd,
  float effects_matrix[3][3],
  color_correct_type* p_cc,
  float dig_gain)
{
  int i, j;
  float coeff[3][3], out_coeff[3][3];
  uint32_t q_factor;
#ifdef ENABLE_CC_LOGGING
  PRINT_2D_MATRIX(3, 3, effects_matrix);
#endif
  if (IS_UNITY_MATRIX(effects_matrix, 3)) {
    CDBG("%s: No effects enabled", __func__);
    GET_CC_MATRIX(p_cc, out_coeff);
  } else {
    CDBG("%s: Effects enabled", __func__);
    GET_CC_MATRIX(p_cc, coeff);
    MATRIX_MULT(effects_matrix, coeff, out_coeff, 3, 3, 3);
  }
#ifdef ENABLE_CC_LOGGING
  PRINT_2D_MATRIX(3, 3, out_coeff);
#endif
#ifdef VFE_2X
  SET_VFE_CC_MATRIX1(p_cmd, out_coeff, (p_cc->q_factor));
#else
  SET_VFE_CC_MATRIX(p_cmd, out_coeff, (p_cc->q_factor));
#endif
  CDBG("%s: dig_gain %5.3f", __func__, dig_gain);

  p_cmd->C2 = (int32_t)(128*dig_gain) - ( p_cmd->C0 + p_cmd->C1);
  p_cmd->C5 = (int32_t)(128*dig_gain) - ( p_cmd->C3 + p_cmd->C4);
  p_cmd->C6 = (int32_t)(128*dig_gain) - ( p_cmd->C7 + p_cmd->C8);

#ifdef VFE_2X
  p_cmd->K0 = p_cc->k0;
  p_cmd->K1 = p_cc->k1;
  p_cmd->K2 = p_cc->k2;
#else
  p_cmd->K0 = p_cc->k1;
  p_cmd->K1 = p_cc->k2;
  p_cmd->K2 = p_cc->k0;
#endif

  p_cmd->coefQFactor = p_cc->q_factor-7;
  return VFE_SUCCESS;
}/*vfe_set_color_correction_params*/

/*===========================================================================
 * Function:           vfe_color_correct_set_effect
 *
 * Description:
 *=========================================================================*/
void vfe_color_correct_populate_matrix(float m[3][3], float s)
{
  m[0][0] = 0.2990 + 1.4075 * 0.498 * s;
  m[0][1] = 0.5870 - 1.4075 * 0.417 * s;
  m[0][2] = 0.1140 - 1.4075 * 0.081 * s;
  m[1][0] =
    0.2990 + 0.3455 * 0.168 * s - 0.7169 * 0.498 * s;
  m[1][1] =
    0.5870 + 0.3455 * 0.330 * s + 0.7169 * 0.417 * s;
  m[1][2] =
    0.1140 - 0.3455 * 0.498 * s + 0.7169 * 0.081 * s;
  m[2][0] = 0.2990 - 1.7790 * 0.168 * s;
  m[2][1] = 0.5870 - 1.7790 * 0.330 * s;
  m[2][2] = 0.1140 + 1.7790 * 0.498 * s;
}/*vfe_color_correct_populate_matrix*/

/*===========================================================================
 * Function:           vfe_color_correct_set_effect
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_color_correct_set_effect(int mod_id, void *mod_cc,
  void* vparms, vfe_effects_type_t type)
{
  vfe_status_t status = VFE_SUCCESS;
  color_correct_mod_t* mod = (color_correct_mod_t* )mod_cc;
  vfe_params_t* params = (vfe_params_t* )vparms;
  if (!mod->cc_enable) {
    CDBG("%s: CC not enabled", __func__);
    return VFE_SUCCESS;
  }

  SET_UNITY_MATRIX(mod->effects_matrix, 3);
  switch(type) {
    case VFE_SATURATION: {
      float s = 2.0 * params->effects_params.saturation;
      if (!F_EQUAL(params->effects_params.hue, 0))
        vfe_color_correct_populate_matrix(mod->effects_matrix, s);
      break;
    }
    case VFE_HUE: {
      float s = 2.0 * params->effects_params.saturation;
      if (!F_EQUAL(params->effects_params.saturation, .5))
        vfe_color_correct_populate_matrix(mod->effects_matrix, s);
      break;
    }
    default:
      SET_UNITY_MATRIX(mod->effects_matrix, 3);
      break;
  }
  mod->update = TRUE;
  return status;
}/*vfe_color_correct_set_effect*/

/*===========================================================================
 * Function:           vfe_color_correct_update
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_correct_update(int mod_id, void *module, void* vparams)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t* )vparams;
  int8_t is_snap = IS_SNAP_MODE(params);
  float (*p_effects_matrix)[3] = NULL, effects_matrix[3][3];
  VFE_ColorCorrectionCfgCmdType* p_cmd = (!is_snap) ?
    &mod->VFE_PrevColorCorrectionCmd :
    &mod->VFE_SnapColorCorrectionCmd;
  int index = (is_snap) ? SNAP : PREV;
  vfe_status_t status = VFE_SUCCESS;

#ifndef VFE_2X
  if (mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(params->camfd,
      CMD_GENERAL, params->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    params->update |= VFE_MOD_COLOR_CORRECT;
    mod->hw_enable = FALSE;
  }
#endif

  if (!mod->cc_enable) {
    CDBG("%s: CC not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->update) {
    CDBG("%s: CC not updated", __func__);
    return VFE_SUCCESS;
  }

  if (params->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: Best shot enabled", __func__);
    SET_UNITY_MATRIX(effects_matrix, 3);
    p_effects_matrix = effects_matrix;
  } else
    p_effects_matrix = mod->effects_matrix;

  vfe_set_color_correction_params (p_cmd,
    p_effects_matrix, &mod->cc[index], mod->dig_gain[index]);

  CDBG("%s: Color correction update",__func__);
  vfe_color_correct_debug(p_cmd);

#ifndef VFE_2X
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, p_cmd,
    sizeof(VFE_ColorCorrectionCfgCmdType),
    VFE_CMD_COLOR_COR_UPDATE);
#endif
  if (VFE_SUCCESS == status) {
    mod->update = FALSE;
    params->update |= VFE_MOD_COLOR_CORRECT;
  }
  return status;
}/*vfe_color_correct_update*/

/*===========================================================================
 * Function:           vfe_color_correct_config
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_color_correct_config(int mod_id, void* module, void* vparams)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t* )vparams;
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;
  float (*p_effects_matrix)[3] = NULL, effects_matrix[3][3];
  VFE_ColorCorrectionCfgCmdType* p_cmd = (!is_snap) ?
    &mod->VFE_PrevColorCorrectionCmd :
    &mod->VFE_SnapColorCorrectionCmd;

  if (!mod->cc_enable) {
    CDBG("%s: CC not enabled", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s: Color correction cfg %s",__func__,
    (is_snap) ? "Snapshot" : "Preview");

  if (params->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: Best shot enabled", __func__);
    SET_UNITY_MATRIX(effects_matrix, 3);
    p_effects_matrix = effects_matrix;
  } else
    p_effects_matrix = mod->effects_matrix;

  vfe_set_color_correction_params (p_cmd,
    p_effects_matrix, &mod->cc[index], mod->dig_gain[index]);

  vfe_color_correct_debug(p_cmd);

#ifndef VFE_2X
  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_GENERAL, p_cmd,
    sizeof(VFE_ColorCorrectionCfgCmdType),
    VFE_CMD_COLOR_COR_CFG);
#endif
  if (VFE_SUCCESS == status)
    mod->update = FALSE;
  return status;
}/* vfe_color_correct_config */

/*===========================================================================
 * Function:           vfe_color_correct_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_color_correct_enable(int mod_id, void *module, void *vparams,
  int8_t enable, int8_t hw_write)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t* )vparams;
  vfe_status_t status = VFE_SUCCESS;
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  params->moduleCfg->colorCorrectionEnable = enable;

  if (hw_write && (mod->cc_enable == enable))
    return VFE_SUCCESS;

  mod->cc_enable = enable;
  mod->hw_enable = hw_write;
  if (hw_write) {

    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_COLOR_CORRECT)
      : (params->current_config & ~VFE_MOD_COLOR_CORRECT);
  }

#ifdef VFE_2X
  if(!enable){
  /*Disable CC by setting it to Default matrix */
    //mod->VFE_PrevColorCorrectionCmd.coefQFactor = 7;

    mod->VFE_PrevColorCorrectionCmd.C0 = CC_COEFF(1.0, 7);
    mod->VFE_PrevColorCorrectionCmd.C1 = 0;
    mod->VFE_PrevColorCorrectionCmd.C2 = 0;

    mod->VFE_PrevColorCorrectionCmd.C3 = 0;
    mod->VFE_PrevColorCorrectionCmd.C4 = CC_COEFF(1.0, 7);
    mod->VFE_PrevColorCorrectionCmd.C5 = 0;

    mod->VFE_PrevColorCorrectionCmd.C6 = 0;
    mod->VFE_PrevColorCorrectionCmd.C7 = 0;
    mod->VFE_PrevColorCorrectionCmd.C8 = CC_COEFF(1.0, 7);

    mod->VFE_PrevColorCorrectionCmd.K0 = 0;
    mod->VFE_PrevColorCorrectionCmd.K1 = 0;
    mod->VFE_PrevColorCorrectionCmd.K2 = 0;

  }

#endif
  return VFE_SUCCESS;
} /* vfe_util_set_color_correction */

/*===========================================================================
 * FUNCTION    - color_correct_interpolate -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void color_correct_interpolate(
  color_correct_type* in1,
  color_correct_type* in2,
  color_correct_type* out,
  float ratio)
{
  out->c0 = LINEAR_INTERPOLATION(in1->c0, in2->c0, ratio);
  out->c1 = LINEAR_INTERPOLATION(in1->c1, in2->c1, ratio);
  out->c2 = LINEAR_INTERPOLATION(in1->c2, in2->c2, ratio);
  out->c3 = LINEAR_INTERPOLATION(in1->c3, in2->c3, ratio);
  out->c4 = LINEAR_INTERPOLATION(in1->c4, in2->c4, ratio);
  out->c5 = LINEAR_INTERPOLATION(in1->c5, in2->c5, ratio);
  out->c6 = LINEAR_INTERPOLATION(in1->c6, in2->c6, ratio);
  out->c7 = LINEAR_INTERPOLATION(in1->c7, in2->c7, ratio);
  out->c8 = LINEAR_INTERPOLATION(in1->c8, in2->c8, ratio);

  out->k0 = LINEAR_INTERPOLATION(in1->k0, in2->k0, ratio);
  out->k1 = LINEAR_INTERPOLATION(in1->k1, in2->k1, ratio);
  out->k2 = LINEAR_INTERPOLATION(in1->k2, in2->k2, ratio);
  out->q_factor = in1->q_factor;
}/*color_correct_interpolate*/

/*===========================================================================
 * FUNCTION    - color_correct_awb_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void color_correct_awb_trigger_update(
  color_correct_mod_t* mod,
  color_correct_type* cc_data,
  int8_t is_snapmode, vfe_params_t* params)
{
  chromatix_parms_type *chromatix_ptr = params->chroma3a;
  color_correct_type *cc1 = NULL, *cc2 = NULL;
  cct_trigger_info trigger_info;
  float ratio = 0.0;

  trigger_info.mired_color_temp = MIRED(params->awb_params.color_temp);
  if (!is_snapmode) {
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
      chromatix_ptr->CC_A_trigger_VF);
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
      chromatix_ptr->CC_Daylight_trigger_VF);
  } else {
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
      chromatix_ptr->CC_A_trigger_snapshot);
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
      chromatix_ptr->CC_Daylight_trigger_snapshot);
  }

  awb_cct_type cct_type = vfe_util_get_awb_cct_type(&trigger_info,
    params);

  CDBG("%s: cct type %d", __func__, cct_type);
  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      *cc_data = (!is_snapmode) ?
        mod->table.chromatix_A_color_correction_VF :
        mod->table.chromatix_A_color_correction_snapshot;
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
      CDBG("%s: AWB_CCT_TYPE_TL84_A ratio %f", __func__, ratio);
      if (!is_snapmode)
        color_correct_interpolate(
          &mod->table.chromatix_TL84_color_correction,
          &mod->table.chromatix_A_color_correction_VF,
          cc_data, ratio);
      else
        color_correct_interpolate(
          &mod->table.chromatix_TL84_color_correction_snapshot,
          &mod->table.chromatix_A_color_correction_snapshot,
          cc_data, ratio);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_end,
        trigger_info.trigger_d65.mired_start);
      CDBG("%s: AWB_CCT_TYPE_D65_TL84 ratio %f", __func__, ratio);
      if (!is_snapmode)
        color_correct_interpolate(
          &mod->table.chromatix_D65_color_correction_VF,
          &mod->table.chromatix_TL84_color_correction,
          cc_data, ratio);
      else
        color_correct_interpolate(
          &mod->table.chromatix_D65_color_correction_snapshot,
          &mod->table.chromatix_TL84_color_correction_snapshot,
          cc_data, ratio);
      break;
    case AWB_CCT_TYPE_D65:
      *cc_data = (!is_snapmode) ?
        mod->table.chromatix_D65_color_correction_VF :
        mod->table.chromatix_D65_color_correction_snapshot;
      break;
    default:
    case AWB_CCT_TYPE_TL84:
      *cc_data = (!is_snapmode) ?
        mod->table.chromatix_TL84_color_correction :
        mod->table.chromatix_TL84_color_correction_snapshot;
      break;
  }
}/*color_correct_awb_trigger_update*/

/*===========================================================================
 * FUNCTION    - color_correction_check_flash -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t color_correction_check_flash(
  color_correct_mod_t* mod,
  vfe_params_t* params, float* p_ratio,
  color_correct_type **pp_input_cc,
  cc_mode_t *cc_mode)
{
  int rc;
  float ratio;
  chromatix_parms_type *chromatix_ptr = params->chroma3a;
  int8_t is_snapmode = IS_SNAP_MODE(params);

  if ((int)params->flash_params.flash_mode != VFE_FLASH_NONE) {
    float flash_start, flash_end;
    ratio = params->flash_params.sensitivity_led_off /
      params->flash_params.sensitivity_led_hi;
    if (((int)params->flash_params.flash_mode == VFE_FLASH_STROBE) &&
      is_snapmode) {
      *pp_input_cc = &(mod->table.chromatix_STROBE_color_correction);
      flash_start = chromatix_ptr->rolloff_Strobe_start;
      flash_end = chromatix_ptr->rolloff_Strobe_end;
      *cc_mode = CC_LED_STROBE;
    } else { /* led */
      *pp_input_cc = (!is_snapmode) ?
        &(mod->table.chromatix_LED_color_correction_VF) :
        &(mod->table.chromatix_LED_color_correction_snapshot);
      flash_start = chromatix_ptr->rolloff_LED_start;
      flash_end = chromatix_ptr->rolloff_LED_end;
      *cc_mode = CC_MODE_LED;
    }
    CDBG("%s: cc_mode %d flash_start %5.2f flash_end %5.2f",
       __func__, *cc_mode, flash_start, flash_end);
    if (ratio >= flash_end)
      *p_ratio = 0.0;
    else if (ratio <= flash_start)
      *p_ratio = 1.0;
    else
      *p_ratio = GET_INTERPOLATION_RATIO(ratio, flash_start, flash_end);
    return TRUE;
  }
  return FALSE;
}/*color_correction_check_flash*/

/*===========================================================================
 * FUNCTION    - color_correct_apply_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void color_correct_apply_gain(color_correct_mod_t* mod,
  vfe_params_t* params)
{
  int8_t is_snapmode = IS_SNAP_MODE(params);
  int index = (is_snapmode) ? SNAP : PREV;
  color_correct_type *cc = &mod->cc[index];
  CDBG("%s: cc_gain %5.3f", __func__, params->aec_gain_adj.cc_gain_adj);
  if (params->aec_gain_adj.cc_gain_adj > MAX_CC_GAIN) {
    params->aec_gain_adj.wb_gain_adj =
      params->aec_gain_adj.cc_gain_adj/MAX_CC_GAIN;
    params->aec_gain_adj.cc_gain_adj = MAX_CC_GAIN;
  } else {
    params->aec_gain_adj.wb_gain_adj = 1.0;
  }
  CDBG("%s: new cc_gain %5.3f old_gain %5.3f", __func__,
    params->aec_gain_adj.cc_gain_adj,
    mod->dig_gain[index]);
  if (F_EQUAL(params->aec_gain_adj.cc_gain_adj, mod->dig_gain[index])
    && (mod->cur_vfe_mode == params->vfe_op_mode)) {
    CDBG("%s: No update required", __func__);
    return;
  }
  CDBG("%s: wb_gain %5.3f", __func__, params->aec_gain_adj.wb_gain_adj);

  mod->dig_gain[index] = params->aec_gain_adj.cc_gain_adj;
  CC_APPLY_GAIN(cc, mod->dig_gain[index]);
  mod->cur_vfe_mode = params->vfe_op_mode;
  mod->update = TRUE;
} /* color_correct_apply_gain */

/*===========================================================================
 * FUNCTION    - vfe_color_correct_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_trigger_update(int mod_id, void *module,
  void *vparams)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t* )vparams;
  color_correct_type input1_cc, *p_input2_cc;
  float ratio = 0.0;
  chromatix_parms_type *chromatix_ptr = params->chroma3a;
  tuning_control_type tuning_type = chromatix_ptr->control_cc;
  trigger_point_type  *p_trigger_point = NULL;
  int8_t update_cc = FALSE;
  cc_mode_t cc_mode = CC_MODE_NORMAL;
  int rc, flash_mode = 0;
  int8_t is_snapmode = IS_SNAP_MODE(params);
  int index = (is_snapmode) ? SNAP : PREV;
  color_correct_type *cc = &mod->cc[index];
  vfe_status_t status = VFE_SUCCESS;

  if (!mod->cc_enable) {
    CDBG("%s: CC not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->trigger_enable) {
    CDBG("%s: CC trigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!is_snapmode && !vfe_util_aec_check_settled(&params->aec_params)) {
    CDBG("%s: AEC not settled", __func__);
    return VFE_SUCCESS;
  }

  memset(&input1_cc, 0x0, sizeof(color_correct_type));
  input1_cc = (!is_snapmode) ? mod->table.chromatix_TL84_color_correction
    : mod->table.chromatix_TL84_color_correction_snapshot;

  if (FALSE == color_correction_check_flash(mod, params, &ratio,
    &p_input2_cc, &cc_mode)) { /*non led/strobe */
    if (!is_snapmode) {
      p_input2_cc = &(mod->table.chromatix_yhi_ylo_color_correction);
      p_trigger_point = &(chromatix_ptr->cc_VF_trigger);
    } else {
      p_input2_cc =
        &(mod->table.chromatix_yhi_ylo_color_correction_snapshot);
      p_trigger_point = &(chromatix_ptr->cc_snapshot_trigger);
    }

    ratio = vfe_util_get_aec_ratio(tuning_type, p_trigger_point, params);
  }

  update_cc =
    (mod->cur_vfe_mode != params->vfe_op_mode) ||
    !F_EQUAL(mod->cur_ratio, ratio) ||
    (mod->cur_cc_mode != cc_mode) ||
    (mod->color_temp != params->awb_params.color_temp) ||
    mod->reload_params;

  if (update_cc) {

    if(!F_EQUAL(ratio, 0.0))
      color_correct_awb_trigger_update(mod, &input1_cc, is_snapmode,
        params);

    color_correct_interpolate(&input1_cc, p_input2_cc, cc, ratio);
    mod->update = TRUE;

    CDBG("%s: new ratio = %f, old color_correction_ratio = %f\n",
      __func__, ratio, mod->cur_ratio);

    mod->cur_ratio = ratio;
    mod->cur_cc_mode = cc_mode;
    mod->reload_params = FALSE;
  }
  color_correct_apply_gain(mod, params);
  mod->cur_vfe_mode = params->vfe_op_mode;
  return status;
}/*vfe_color_correct_trigger_update*/

/*===========================================================================
 * FUNCTION    - vfe_color_correct_set_bestshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t*)vparams;
  int8_t is_snapmode = IS_SNAP_MODE(params);
  int index = (is_snapmode) ? SNAP : PREV;

  mod->trigger_enable = FALSE;

  CDBG("%s: mode %d", __func__, mode);
  switch(mode) {
    case CAMERA_BESTSHOT_NIGHT:
      mod->cc[PREV] = mod->table.chromatix_yhi_ylo_color_correction;
      mod->cc[SNAP] = mod->table.chromatix_yhi_ylo_color_correction_snapshot;
      break;
    default:
      mod->cc[PREV] = mod->table.chromatix_TL84_color_correction;
      mod->cc[SNAP] = mod->table.chromatix_TL84_color_correction_snapshot;
      mod->trigger_enable = TRUE;
      break;
  }
  mod->update = TRUE;
  return VFE_SUCCESS;
} /* vfe_color_correct_set_bestshot */

/*===========================================================================
 * FUNCTION    - vfe_color_correct_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_trigger_enable(int mod_id, void* module, void* params,
  int enable)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  CDBG("%s: %d", __func__, enable);
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_color_correct_trigger_enable*/

/*===========================================================================
 * FUNCTION    - vfe_color_correct_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_reload_params(int mod_id, void *module,
  void *vparams)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t* )vparams;
  CDBG("%s:", __func__);
  mod->reload_params = TRUE;
  vfe_status_t status = VFE_SUCCESS;
  vfe_color_correct_convert_table_all(mod, params);

  if (params->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: update BSM in reload for mode %d",__func__,params->bs_mode);
    status = vfe_color_correct_set_bestshot(mod_id, mod, params,
               params->bs_mode);
  }
  else {
    mod->cc[PREV] = mod->table.chromatix_TL84_color_correction;
    mod->cc[SNAP] = mod->table.chromatix_TL84_color_correction_snapshot;
  }

  return status;
} /*vfe_color_correct_reload_params*/

/*===========================================================================
 * FUNCTION    - vfe_color_correct_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_init(int mod_id, void *module, void *vparams)
{
  color_correct_mod_t* mod = (color_correct_mod_t*) module;
  vfe_params_t* params = (vfe_params_t* )vparams;
  chromatix_parms_type *chromatix_ptr = params->chroma3a;

  SET_UNITY_MATRIX(mod->effects_matrix, 3);
  mod->dig_gain[PREV] = 1.0;
  mod->dig_gain[SNAP] = 1.0;
  mod->cur_cc_mode = CC_MODE_NONE;
  mod->cur_vfe_mode = VFE_OP_MODE_INVALID;
  mod->trigger_enable = TRUE;
  vfe_color_correct_convert_table_all(mod, params);
  mod->cc[PREV] = mod->table.chromatix_TL84_color_correction;
  mod->cc[SNAP] = mod->table.chromatix_TL84_color_correction_snapshot;
  return VFE_SUCCESS;
} /*vfe_color_correct_init*/

/*===========================================================================
 * FUNCTION    - color_correct_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void color_correct_populate_data(uint32_t *reg, VFE_ColorCorrectionCfgCmdType
  *pcmd)
{
  CDBG("%s: size %d", __func__, sizeof(VFE_ColorCorrectionCfgCmdType));
  reg += (CC_START_REG/4);
  memcpy((void *)pcmd, reg, sizeof(VFE_ColorCorrectionCfgCmdType));
}/*color_correct_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_color_correct_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_tv_validate(int mod_id, void *test_ip, void *test_op)
{
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_ip;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_op;
  VFE_ColorCorrectionCfgCmdType in_cc, out_cc;
  color_correct_populate_data(input->reg_dump, &in_cc);
  color_correct_populate_data(output->reg_dump_data, &out_cc);

  if (!MATCH(in_cc.C0,out_cc.C0, 2))
    CDBG_TV("%s: C0 in %d out %d doesnt match", __func__, in_cc.C0, out_cc.C0);

  if (!MATCH(in_cc.C1,out_cc.C1, 2))
    CDBG_TV("%s: C1 in %d out %d doesnt match", __func__, in_cc.C1, out_cc.C1);

  if (!MATCH(in_cc.C2,out_cc.C2, 2))
    CDBG_TV("%s: C2 in %d out %d doesnt match", __func__, in_cc.C2, out_cc.C2);

  if (!MATCH(in_cc.C3,out_cc.C3, 2))
    CDBG_TV("%s: C3 in %d out %d doesnt match", __func__, in_cc.C3, out_cc.C3);

  if (!MATCH(in_cc.C4,out_cc.C4, 2))
    CDBG_TV("%s: C4 in %d out %d doesnt match", __func__, in_cc.C4, out_cc.C4);

  if (!MATCH(in_cc.C5,out_cc.C5, 2))
    CDBG_TV("%s: C5 in %d out %d doesnt match", __func__, in_cc.C5, out_cc.C5);

  if (!MATCH(in_cc.C6,out_cc.C6, 2))
    CDBG_TV("%s: C6 in %d out %d doesnt match", __func__, in_cc.C6, out_cc.C6);

  if (!MATCH(in_cc.C7,out_cc.C7, 2))
    CDBG_TV("%s: C7 in %d out %d doesnt match", __func__, in_cc.C7, out_cc.C7);

  if (!MATCH(in_cc.C8,out_cc.C8, 2))
    CDBG_TV("%s: C8 in %d out %d doesnt match", __func__, in_cc.C8, out_cc.C8);

  if (!MATCH(in_cc.K0,out_cc.K0, 0))
    CDBG_TV("%s: K0 in %d out %d doesnt match", __func__, in_cc.K0, out_cc.K0);

  if (!MATCH(in_cc.K1,out_cc.K1, 0))
    CDBG_TV("%s: K1 in %d out %d doesnt match", __func__, in_cc.K1, out_cc.K1);

  if (!MATCH(in_cc.K2,out_cc.K2, 0))
    CDBG_TV("%s: K2 in %d out %d doesnt match", __func__, in_cc.K2, out_cc.K2);

  if (!MATCH(in_cc.coefQFactor,out_cc.coefQFactor, 0))
    CDBG_TV("%s: coefQFactor in %d out %d doesnt match", __func__,
    in_cc.coefQFactor, out_cc.coefQFactor);

  return VFE_SUCCESS;
}/*vfe_color_correct_tv_validate*/

/*===========================================================================
 * FUNCTION    - vfe_color_correct_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_color_correct_deinit(int mod_id, void *module, void *params)
{
  color_correct_mod_t *cc_mod = (color_correct_mod_t *)module;
  memset(cc_mod, 0 , sizeof(color_correct_mod_t));
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_color_correct_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  color_correct_module_t *cmd = (color_correct_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->cc_cfg_Cmd),
     sizeof(VFE_ColorCorrectionCfgCmdType),
     VFE_CMD_COLOR_COR_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_color_correct_plugin_update */
#endif
