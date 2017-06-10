/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "abf44.h"
#include "isp_log.h"

#ifdef ENABLE_ABF_LOGGING
  #undef ISP_DBG
  #define ISP_DBG ALOGE
#endif

#define ABF_START_REG 0x02a4

/** abf_reset:
 *
 *    @mod:
 *
 **/
static void abf_reset(isp_abf_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  memset(&mod->aec_ratio, 0, sizeof(mod->aec_ratio));
  memset(&mod->abf2_parms, 0, sizeof(mod->abf2_parms));
  mod->hw_update_pending = 0;
  mod->trigger_enable = 0; /* enable trigger update feature flag from PIX*/
  mod->skip_trigger = 0; /* skip the trigger update */
  mod->enable = 0;         /* enable flag from PIX */
}

/** abf_set_params_common:
 *
 *    @p_cmd:
 *    @abf2_data:
 *
 * common part in set abf cfg cmd
 *
 **/
static void abf_set_params_common(ISP_DemosaicABF_CmdType* p_cmd,
  chromatix_adaptive_bayer_filter_data_type2* abf2_data)
{
  p_cmd->gCfg.Cutoff1 = ABF2_CUTOFF1(abf2_data->threshold_green[0]);
  p_cmd->bCfg.Cutoff1 = ABF2_CUTOFF1(abf2_data->threshold_blue[0]);
  p_cmd->rCfg.Cutoff1 = ABF2_CUTOFF1(abf2_data->threshold_red[0]);

  p_cmd->gCfg.Cutoff2 = ABF2_CUTOFF2(p_cmd->gCfg.Cutoff1,
    abf2_data->threshold_green[1]);
  p_cmd->bCfg.Cutoff2 = ABF2_CUTOFF2(p_cmd->bCfg.Cutoff1,
    abf2_data->threshold_blue[1]);
  p_cmd->rCfg.Cutoff2 = ABF2_CUTOFF2(p_cmd->rCfg.Cutoff1,
    abf2_data->threshold_red[1]);

  p_cmd->gCfg.Cutoff3 = ABF2_CUTOFF3(p_cmd->gCfg.Cutoff2,
    abf2_data->threshold_green[2]);
  p_cmd->bCfg.Cutoff3 = ABF2_CUTOFF3(p_cmd->bCfg.Cutoff2,
    abf2_data->threshold_blue[2]);
  p_cmd->rCfg.Cutoff3 = ABF2_CUTOFF3(p_cmd->rCfg.Cutoff2,
    abf2_data->threshold_red[2]);

  p_cmd->gCfg.MultNegative = ABF2_MULT_NEG(p_cmd->gCfg.Cutoff2,
    p_cmd->gCfg.Cutoff3);
  p_cmd->bCfg.MultNegative = ABF2_MULT_NEG(p_cmd->bCfg.Cutoff2,
    p_cmd->bCfg.Cutoff3);
  p_cmd->rCfg.MultNegative = ABF2_MULT_NEG(p_cmd->rCfg.Cutoff2,
    p_cmd->rCfg.Cutoff3);

  p_cmd->gCfg.MultPositive = ABF2_MULT_POS(p_cmd->gCfg.Cutoff1);
  p_cmd->bCfg.MultPositive = ABF2_MULT_POS(p_cmd->bCfg.Cutoff1);
  p_cmd->rCfg.MultPositive = ABF2_MULT_POS(p_cmd->rCfg.Cutoff1);

  p_cmd->gCfg.SpatialKernelA0 = ABF2_SP_KERNEL(abf2_data->a[0]);
  p_cmd->gCfg.SpatialKernelA1 = ABF2_SP_KERNEL(abf2_data->a[1]);
} /* abf_set_params_common */

/** abf_set_cmd_params1:
 *
 *    @p_cmd:
 *    @abf2_data:
 *
 * common part in set abf cfg cmd
 *
 **/
static int8_t abf_set_cmd_params1(ISP_DemosaicABF_CmdType* p_cmd,
  chromatix_adaptive_bayer_filter_data_type2* abf2_data)
{
  int i = 0;
  uint32_t temp;
  int8_t rc = TRUE;
  abf_set_params_common(p_cmd, abf2_data);

  for (i=0; i<8; i++) {
    p_cmd->gPosLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_green[0]*abf2_data->table_pos[2*i]);
    p_cmd->bPosLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_blue[0]*abf2_data->table_pos[2*i]);
    p_cmd->rPosLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_red[0]*abf2_data->table_pos[2*i]);
    p_cmd->gPosLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_green[0]*abf2_data->table_pos[2*i+1]);
    p_cmd->bPosLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_blue[0]*abf2_data->table_pos[2*i+1]);
    p_cmd->rPosLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_red[0]*abf2_data->table_pos[2*i+1]);
  }

  for (i=0; i<4; i++) {
    p_cmd->gNegLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_green[1]*abf2_data->table_neg[2*i]);
    p_cmd->bNegLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_blue[1]*abf2_data->table_neg[2*i]);
    p_cmd->rNegLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_red[1]*abf2_data->table_neg[2*i]);
    p_cmd->gNegLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_green[1]*abf2_data->table_neg[2*i+1]);
    p_cmd->bNegLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_blue[1]*abf2_data->table_neg[2*i+1]);
    p_cmd->rNegLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_red[1]*abf2_data->table_neg[2*i+1]);
  }

  return rc;
} /* abf_set_cmd_params1 */

/** abf_set_cmd_params2:
 *
 *    @p_cmd:
 *    @abf2_data:
 *
 *
 **/
static int8_t abf_set_cmd_params2(ISP_DemosaicABF_CmdType* p_cmd,
  abf2_parms_t* abf2_parms)
{
  int i = 0;
  int8_t rc = TRUE;
  uint32_t temp;

  if (!abf2_parms->table_updated)
    return abf_set_cmd_params1(p_cmd, &abf2_parms->data);

  abf_set_params_common(p_cmd, &abf2_parms->data);

  for (i=0; i<8; i++) {
    p_cmd->gPosLut[i].LUT0 =
      ABF2_LUT(abf2_parms->g_table.table_pos[2*i]);
    p_cmd->bPosLut[i].LUT0 =
      ABF2_LUT(abf2_parms->b_table.table_pos[2*i]);
    p_cmd->rPosLut[i].LUT0 =
      ABF2_LUT(abf2_parms->r_table.table_pos[2*i]);
    p_cmd->gPosLut[i].LUT1 =
      ABF2_LUT(abf2_parms->g_table.table_pos[2*i+1]);
    p_cmd->bPosLut[i].LUT1 =
      ABF2_LUT(abf2_parms->b_table.table_pos[2*i+1]);
    p_cmd->rPosLut[i].LUT1 =
      ABF2_LUT(abf2_parms->r_table.table_pos[2*i+1]);
  }

  for (i=0; i<4; i++) {
    p_cmd->gNegLut[i].LUT0 =
      ABF2_LUT(abf2_parms->g_table.table_neg[2*i]);
    p_cmd->bNegLut[i].LUT0 =
      ABF2_LUT(abf2_parms->b_table.table_neg[2*i]);
    p_cmd->rNegLut[i].LUT0 =
      ABF2_LUT(abf2_parms->r_table.table_neg[2*i]);
    p_cmd->gNegLut[i].LUT1 =
      ABF2_LUT(abf2_parms->g_table.table_neg[2*i+1]);
    p_cmd->bNegLut[i].LUT1 =
      ABF2_LUT(abf2_parms->b_table.table_neg[2*i+1]);
    p_cmd->rNegLut[i].LUT1 =
      ABF2_LUT(abf2_parms->r_table.table_neg[2*i+1]);
  }

  return rc;
} /* abf_set_cmd_params2 */

/** abf_interpolate:
 *
 *    @pv1:
 *    @pv2:
 *    @pv_out:
 *    @ration
 *
 **/
static void abf_interpolate(chromatix_adaptive_bayer_filter_data_type2* pv1,
  chromatix_adaptive_bayer_filter_data_type2* pv2, abf2_parms_t* pv_out,
  float ratio)
{
  int32_t i;

  TBL_INTERPOLATE_INT(pv1->threshold_red, pv2->threshold_red,
    pv_out->data.threshold_red, ratio, 3, i);
  TBL_INTERPOLATE_INT(pv1->threshold_green, pv2->threshold_green,
    pv_out->data.threshold_green, ratio, 3, i);
  TBL_INTERPOLATE_INT(pv1->threshold_blue, pv2->threshold_blue,
    pv_out->data.threshold_blue, ratio, 3, i);

  for (i=0; i<16; i++) {
    pv_out->g_table.table_pos[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_green[0] * pv1->table_pos[i],
      pv2->scale_factor_green[0] * pv2->table_pos[i], ratio);
    pv_out->b_table.table_pos[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_blue[0] * pv1->table_pos[i],
      pv2->scale_factor_blue[0] * pv2->table_pos[i], ratio);
    pv_out->r_table.table_pos[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_red[0] * pv1->table_pos[i],
      pv2->scale_factor_red[0] * pv2->table_pos[i], ratio);
  }

  for (i=0; i<8; i++) {
    pv_out->g_table.table_neg[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_green[1] * pv1->table_neg[i],
      pv2->scale_factor_green[1] * pv2->table_neg[i], ratio);
    pv_out->b_table.table_neg[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_blue[1] * pv1->table_neg[i],
      pv2->scale_factor_blue[1] * pv2->table_neg[i], ratio);
    pv_out->r_table.table_neg[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_red[1] * pv1->table_neg[i],
      pv2->scale_factor_red[1] * pv2->table_neg[i], ratio);
  }

  TBL_INTERPOLATE(pv1->a, pv2->a, pv_out->data.a, ratio, 2, i);
  pv_out->table_updated = TRUE;
} /* abf_interpolate */

/** abf_debug:
 *
 *    @pCmd:
 *
 * print out params in config cmd
 *
 **/
static void abf_debug(ISP_DemosaicABF_CmdType* pCmd)
{
  int i = 0;
  ISP_DemosaicABF_gCfg* gCfg = &(pCmd->gCfg);
  ISP_DemosaicABF_RBCfg* rCfg = &(pCmd->rCfg);
  ISP_DemosaicABF_RBCfg* bCfg = &(pCmd->bCfg);

  ISP_DemosaicABF_Lut* gNLut = (pCmd->gNegLut);
  ISP_DemosaicABF_Lut* rNLut = (pCmd->rNegLut);
  ISP_DemosaicABF_Lut* bNLut = (pCmd->bNegLut);

  ISP_DemosaicABF_Lut* gPLut = (pCmd->gPosLut);
  ISP_DemosaicABF_Lut* rPLut = (pCmd->rPosLut);
  ISP_DemosaicABF_Lut* bPLut = (pCmd->bPosLut);

  ISP_DBG(ISP_MOD_ABF, "%s: =====Green parametets ===============\n", __func__);
  ISP_DBG(ISP_MOD_ABF, "%s: abf2 green: cutoff1=%d, cutoff2=%d,cutoff3=%d.\n", __func__,
    gCfg->Cutoff1, gCfg->Cutoff2, gCfg->Cutoff3);
  ISP_DBG(ISP_MOD_ABF, "%s: abf2 green: multiPositive=%d, multiNegative=%d.\n", __func__,
    gCfg->MultPositive, gCfg->MultNegative);
  ISP_DBG(ISP_MOD_ABF, "%s: abf2 green: A0 = %d, A1 = %d.\n", __func__,
    gCfg->SpatialKernelA0,gCfg->SpatialKernelA1);

  for (i = 0; i < 8; i++)
    ISP_DBG(ISP_MOD_ABF, "%s: Green PosLUT: coef%d=%d,coef%d=%d\n", __func__,
      2*i, gPLut[i].LUT0, 2*i+1, gPLut[i].LUT1);

  for (i = 0; i < 4; i++)
    ISP_DBG(ISP_MOD_ABF, "%s: Green NegLUT: coef%d=%d,coef%d=%d\n", __func__,
      2*i, gNLut[i].LUT0, 2*i+1, gNLut[i].LUT1);

  ISP_DBG(ISP_MOD_ABF, "%s:=====Red parametets ===============\n", __func__);
  ISP_DBG(ISP_MOD_ABF, "%s: abf2 red: cutoff1=%d, cutoff2=%d,cutoff3=%d.\n", __func__,
    rCfg->Cutoff1, rCfg->Cutoff2, rCfg->Cutoff3);
  ISP_DBG(ISP_MOD_ABF, "%s: abf2 red: multiPositive=%d, multiNegative=%d.\n", __func__,
    rCfg->MultPositive, rCfg->MultNegative);

  for (i = 0; i < 8; i++)
    ISP_DBG(ISP_MOD_ABF, "%s: Red PosLUT: coef%d=%d,coef%d=%d\n", __func__,
      2*i, rPLut[i].LUT0, 2*i+1, rPLut[i].LUT1);

  for (i = 0; i < 4; i++)
    ISP_DBG(ISP_MOD_ABF, "%s: Red NegLUT: coef%d=%d,coef%d=%d\n", __func__,
      2*i, rNLut[i].LUT0, 2*i+1, rNLut[i].LUT1);

  ISP_DBG(ISP_MOD_ABF, "%s:=====Blue parametets ===============\n", __func__);
  ISP_DBG(ISP_MOD_ABF, "%s: abf2 blue: cutoff1=%d, cutoff2=%d,cutoff3=%d.\n", __func__,
    bCfg->Cutoff1, bCfg->Cutoff2, bCfg->Cutoff3);
  ISP_DBG(ISP_MOD_ABF, "%s:abf2 blue: multiPositive=%d, multiNegative=%d.\n", __func__,
    bCfg->MultPositive, bCfg->MultNegative);

  for (i = 0; i < 8; i++)
    ISP_DBG(ISP_MOD_ABF, "%s: Blue PosLUT: coef%d=%d,coef%d=%d\n", __func__,
      2*i, bPLut[i].LUT0, 2*i+1, bPLut[i].LUT1);

  for (i = 0; i < 4; i++)
    ISP_DBG(ISP_MOD_ABF, "%s: Blue NegLUT: coef%d=%d,coef%d=%d\n", __func__,
      2*i, bNLut[i].LUT0, 2*i+1, bNLut[i].LUT1);
} /* abf_debug */

/** abf_init:
 *
 *    @mod_ctrl:
 *    @in_params:
 *    @notify_ops:
 *
 * init abf module
 *
 **/
static int abf_init(void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_abf_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  abf_reset(mod);

  return 0;
} /* abf_init */

/** abf_config:
 *
 *    @mod:
 *    @pix_setting:
 *    @in_param_size:
 *
 **/
static int abf_config(isp_abf_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t in_param_size)
{
  chromatix_adaptive_bayer_filter_data_type2 *abf2_data = NULL;
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)pix_setting->chromatix_ptrs.chromatixPtr;
  chromatix_ABF2_type *chromatix_ABF2 =
    &chroma_ptr->chromatix_VFE.chromatix_ABF2;

  ISP_DBG(ISP_MOD_ABF, "%s: enter", __func__);

  if (!mod->enable) {
    CDBG_HIGH("%s: abf not enabled, returns\n", __func__);
    return 0;
  }

  memset(&mod->abf2_parms, 0x0, sizeof(abf2_parms_t));
  abf2_data = &(chromatix_ABF2->abf2_config_normal_light);
  abf_set_cmd_params1(&mod->RegCmd, abf2_data);

  mod->hw_update_pending = TRUE;

  return 0;
} /* abf_config */

/** abf_destroy:
 *
 *    @mod_ctrl:
 *
 **/
static int abf_destroy(void *mod_ctrl)
{
  isp_abf_mod_t *mod = mod_ctrl;

  memset(mod,  0,  sizeof(isp_abf_mod_t));
  free(mod);

  return 0;
} /* abf_destroy */

/** abf_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 **/
static int abf_enable(isp_abf_mod_t *mod,
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
} /* abf_enable */

/** abf_trigger_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 * enable abf trigger update feature
 *
 **/
static int abf_trigger_enable(isp_abf_mod_t *mod,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->trigger_enable = enable->enable;

  return 0;
} /* abf_trigger_enable */

/** abf_trigger_update:
 *
 *    @mod:
 *    @trigger_params:
 *    @in_param_size:
 *
 **/
static int abf_trigger_update(isp_abf_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params,
  uint32_t in_param_size)
{
  int i = 0;
  chromatix_parms_type *chroma_ptr = trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_ABF2_type *chromatix_ABF2 =
    &chroma_ptr->chromatix_VFE.chromatix_ABF2;
  abf2_parms_t *abf2_parms = &(mod->abf2_parms);

  trigger_point_type *outdoor = NULL, *lowlight = NULL;
  trigger_ratio_t trigger_ratio;
  chromatix_adaptive_bayer_filter_data_type2 *p1 = NULL, *p2 = NULL,
    *abf2_normal, *abf2_outdoor, *abf2_lowlight;
  tuning_control_type tunning_control = chromatix_ABF2->control_abf2;
  uint8_t is_burst = IS_BURST_STREAMING((&trigger_params->cfg));

  if (!mod->enable || !mod->trigger_enable) {
    ISP_DBG(ISP_MOD_ABF, "%s: no trigger for ABF, enable = %d, trigger_ena = %d\n",
      __func__, mod->enable, mod->trigger_enable);
    return 0;
  }

  if (!isp_util_aec_check_settled(
         &trigger_params->trigger_input.stats_update.aec_update)) {
    ISP_DBG(ISP_MOD_ABF, "%s: AEC not settled", __func__);
    return 0;
  }

  outdoor = &(chromatix_ABF2->abf2_bright_light_trigger);
  lowlight = &(chromatix_ABF2->abf2_low_light_trigger);
  abf2_normal = &(chromatix_ABF2->abf2_config_normal_light);
  abf2_outdoor = &(chromatix_ABF2->abf2_config_bright_light);
  abf2_lowlight = &(chromatix_ABF2->abf2_config_low_light);

  /* Decide the trigger ratio for current lighting condition */
  if(isp_util_get_aec_ratio2(mod->notify_ops->parent, tunning_control, outdoor,
       lowlight, &trigger_params->trigger_input.stats_update.aec_update,
       is_burst, &trigger_ratio) != 0){
    CDBG_HIGH("%s: get aec ratio error", __func__);
    return -1;
  }

  /* this condition is:  if first time, or either ratio/lighting
     != previous ones, then we need update. */
  ISP_DBG(ISP_MOD_ABF, "%s: OLD: lighting = %d, ratio = %f. NEW: lighting = %d, ratio %f\n",
    __func__, mod->aec_ratio.lighting, mod->aec_ratio.ratio,
    trigger_ratio.lighting, trigger_ratio.ratio);

  switch (trigger_ratio.lighting) {
  case TRIGGER_LOWLIGHT: {
    p1 = abf2_lowlight;
    p2 = abf2_normal;
  }
    break;

  case TRIGGER_OUTDOOR: {
    p1 = abf2_outdoor;
    p2 = abf2_normal;
  }
    break;

  default:
  case TRIGGER_NORMAL: {
    p1 = p2 = abf2_normal;
  }
    break;
  }

  if ((trigger_params->cfg.streaming_mode != mod->old_streaming_mode) ||
      (trigger_ratio.lighting != mod->aec_ratio.lighting) ||
      !F_EQUAL(trigger_ratio.ratio, mod->aec_ratio.ratio)) {
    /* Update abf2 */
    mod->hw_update_pending = 1;
    mod->old_streaming_mode = trigger_params->cfg.streaming_mode;
    mod->aec_ratio = trigger_ratio;

    if (!F_EQUAL(trigger_ratio.ratio, 0.0) &&
        !F_EQUAL(trigger_ratio.ratio, 1.0)) {
      abf_interpolate(p2, p1, abf2_parms, trigger_ratio.ratio);
    } else {
      memcpy(&abf2_parms->data, p1,
        sizeof(chromatix_adaptive_bayer_filter_data_type2));
    }

    abf_set_cmd_params2(&mod->RegCmd, abf2_parms);
  }

  return 0;
} /* abf_trigger_update */

/** abf_set_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *
 **/
static int abf_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_abf_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = abf_enable(mod, (isp_mod_set_enable_t *)in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = abf_config(mod, in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = abf_trigger_enable(mod, in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = abf_trigger_update(mod, in_params, in_param_size);
  }
    break;

  default: {
    return -EAGAIN;
  }
    break;
  }
  return rc;
} /* abf_set_params */

/** abf_get_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *    @in_param_size:
 *    @out_params:
 *    @out_param_size:
 *
 **/
static int abf_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size,
  void *out_params, uint32_t out_param_size)
{
  isp_abf_mod_t *mod = mod_ctrl;
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

  default: {
    rc = -1;
  }
    break;
  }

  return rc;
} /* abf_get_params */

/** abf_do_hw_update:
 *
 *    @abf_mod:
 *
 * update module register to kernel
 *
 **/
static int abf_do_hw_update(isp_abf_mod_t *abf_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[2];
  ISP_DemosaicABF_Cfg cfg_enable;
  ISP_DemosaicABF_Cfg cfg_mask;

  ISP_DBG(ISP_MOD_ABF, "%s: do hw update: %d, abf_enable = %d\n", __func__,
    abf_mod->hw_update_pending,abf_mod->enable);

  if (abf_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &abf_mod->RegCmd;
    cfg_cmd.cmd_len = sizeof(abf_mod->RegCmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 2;

    cfg_mask.enable = 1;
    cfg_enable.enable = abf_mod->enable;

    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_ABF_DEMOSAIC_MIX_CFG_OFF;
    reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.cfg;
    reg_cfg_cmd[0].u.mask_info.val = cfg_enable.cfg;

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_ABF40_OFF;
    reg_cfg_cmd[1].u.rw_info.len = ISP_ABF40_LEN * sizeof(uint32_t);

    abf_debug(&abf_mod->RegCmd);
    rc = ioctl(abf_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    abf_mod->hw_update_pending = 0;
  }

  /* TODO: update hw reg */
  return rc;
} /* abf_hw_reg_update */

/** abf_action:
 *
 *    @mod_ctrl:
 *    @action_code
 *    @data:
 *    @data_size:
 *
 * processing the action
 *
 **/
static int abf_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_abf_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = abf_do_hw_update(mod);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET: {
    abf_reset(mod);
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
} /* abf_action */

/** abf44_open:
 *
 *    @version:
 *
 * open abf40 mod, create func table
 *
 **/
isp_ops_t *abf44_open(uint32_t version)
{
  isp_abf_mod_t *mod = malloc(sizeof(isp_abf_mod_t));

  ISP_DBG(ISP_MOD_ABF, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }

  memset(mod,  0,  sizeof(isp_abf_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = abf_init;
  mod->ops.destroy = abf_destroy;
  mod->ops.set_params = abf_set_params;
  mod->ops.get_params = abf_get_params;
  mod->ops.action = abf_action;

  return &mod->ops;
} /* abf44_open */
