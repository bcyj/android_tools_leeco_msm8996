/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "clf44.h"
#include "isp_log.h"


#ifdef ENABLE_CLF_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#if 0
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#define CLF_START_REG 0x06B4

#define IS_CLF_ENABLED(mod) (mod->cf_enable || mod->lf_enable)

/** util_clf_luma_debug:
 *    @p_cmd: Luma Filter configuration
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps the CLF luma configuration
 *
 *  Return: None
 **/
static void util_clf_luma_debug(ISP_CLF_Luma_Update_CmdType* p_cmd)
{
  int i = 0;
  ISP_DBG(ISP_MOD_CLF, "%s: CLF Luma cutoff1: %d cutoff2: %d cutoff3: %d\n",
    __func__, p_cmd->Cfg.cutoff_1, p_cmd->Cfg.cutoff_2, p_cmd->Cfg.cutoff_3);
  ISP_DBG(ISP_MOD_CLF, "%s: mult_neg %d mult_pos %d\n",
    __func__, p_cmd->Cfg.mult_neg, p_cmd->Cfg.mult_pos);

  for (i=0; i<8; i++)
    ISP_DBG(ISP_MOD_CLF, "%s: posLUT%d %d posLUT%d %d", __func__,
      2*i, p_cmd->pos_LUT[i].lut0,
      2*i+1, p_cmd->pos_LUT[i].lut1);

  for (i=0; i<4; i++)
    ISP_DBG(ISP_MOD_CLF, "%s: negLUT%d %d negLUT%d %d", __func__,
      2*i, p_cmd->neg_LUT[i].lut0,
      2*i+1, p_cmd->neg_LUT[i].lut1);
} /* util_clf_luma_debug */

/** util_clf_chroma_debug:
 *    @p_cmd: Chroma Filter configuration
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps the CLF chroma configuration
 *
 *  Return: None
 **/
static void util_clf_chroma_debug(ISP_CLF_Chroma_Update_CmdType* p_cmd)
{
  int rc = TRUE;
  ISP_DBG(ISP_MOD_CLF, "s: v_coeff0 %d v_coeff1 %d", p_cmd->chroma_coeff.v_coeff0,
     p_cmd->chroma_coeff.v_coeff1);

  ISP_DBG(ISP_MOD_CLF, "s: h_coeff0 %d h_coeff1 %d h_coeff2 %d h_coeff3 %d",
       p_cmd->chroma_coeff.h_coeff0, p_cmd->chroma_coeff.h_coeff1,
       p_cmd->chroma_coeff.h_coeff2, p_cmd->chroma_coeff.h_coeff3);
} /* util_clf_chroma_debug */

/** util_clf_luma_debug:
 *    @p_cmd: CLF Filter configuration
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps the CLF configuration
 *
 *  Return: None
 **/
static void util_clf_debug(ISP_CLF_CmdType* p_cmd)
{
  ISP_DBG(ISP_MOD_CLF, "%s: colorconv_enable %d pipe_flush_cnt %d\n", __func__,
   p_cmd->clf_cfg.colorconv_enable,
   p_cmd->clf_cfg.pipe_flush_cnt);
  ISP_DBG(ISP_MOD_CLF, "%s: pipe_flush_ovd %d flush_halt_ovd %d\n", __func__,
   p_cmd->clf_cfg.pipe_flush_ovd,
   p_cmd->clf_cfg.flush_halt_ovd);

  util_clf_luma_debug(&p_cmd->lumaUpdateCmd);
  util_clf_chroma_debug(&p_cmd->chromaUpdateCmd);
} /* util_clf_debug */

/** util_clf_luma_interpolate:
 *    @in1: start input bayer filter data
 *    @in2: end input bayer filter data
 *    @out: interpolated data
 *    @ratio: ratio
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function interpolates CLF luma
 *
 *  Return: None
 **/
static void util_clf_luma_interpolate(
  chromatix_adaptive_bayer_filter_data_type2 *in1,
  chromatix_adaptive_bayer_filter_data_type2 *in2,
  chromatix_adaptive_bayer_filter_data_type2 *out, float ratio)
{
  int i = 0;
  TBL_INTERPOLATE_INT(in1->threshold_red, in2->threshold_red, out->threshold_red,
    ratio, 3, i);

  for (i=0; i<16; i++)
    out->table_pos[i] =
      LINEAR_INTERPOLATION(in1->scale_factor_red[0] * in1->table_pos[i],
        in2->scale_factor_red[0] * in2->table_pos[i], ratio);

  for (i=0; i<8; i++)
    out->table_neg[i] =
      LINEAR_INTERPOLATION(in1->scale_factor_red[1] * in1->table_neg[i],
        in2->scale_factor_red[1] * in2->table_neg[i], ratio);

  /* setting scale factors to 1.0 since the curves are already calculated */
  for (i=0; i<2; i++)
    out->scale_factor_red[i] = 1.0;

} /* util_clf_luma_interpolate */

/** util_clf_chroma_interpolate:
 *    @in1: start input bayer filter data
 *    @in2: end input bayer filter data
 *    @out: interpolated data
 *    @ratio: ratio
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function interpolates CLF chroma
 *
 *  Return: None
 **/
static void util_clf_chroma_interpolate(Chroma_filter_type *in1,
  Chroma_filter_type *in2,
  Chroma_filter_type *out, float ratio)
{
  int i = 0;
  TBL_INTERPOLATE(in1->h, in2->h, out->h, ratio, 4, i);
  TBL_INTERPOLATE(in1->v, in2->v, out->v, ratio, 2, i);
} /* util_clf_chroma_interpolate */

/** util_disable_luma:
 *    @luma: luma filter configuration
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets configuration to disabla luma filtering
 *
 *  Return: None
 **/
static void util_disable_luma(ISP_CLF_Luma_Update_CmdType* luma)
{
  memset(&luma->pos_LUT, 0x0, sizeof(ISP_CLF_Luma_Lut) * 8);
  memset(&luma->neg_LUT, 0x0, sizeof(ISP_CLF_Luma_Lut) * 4);
} /* util_disable_luma */

/** util_disable_chroma:
 *    @luma: start input bayer filter data
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets configuration to disabla chroma filtering
 *
 *  Return: None
 **/
static void util_disable_chroma(ISP_CLF_Chroma_Update_CmdType* chroma)
{
  chroma->chroma_coeff.h_coeff0 = chroma->chroma_coeff.v_coeff0 = 1;
  chroma->chroma_coeff.h_coeff1 = chroma->chroma_coeff.h_coeff2 =
  chroma->chroma_coeff.h_coeff3 = chroma->chroma_coeff.v_coeff1 = 0;
} /* util_disable_chroma */

/** util_clf_set_luma_params:
 *    @mod: CLF module instance
 *    @parm: luma parameters
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function sets luma parameters
 *
 *  Return: None
 **/
static void util_clf_set_luma_params(isp_clf_mod_t *mod,
  chromatix_adaptive_bayer_filter_data_type2 *parm)
{
  int i = 0;
  int32_t temp;
  ISP_CLF_Luma_Update_CmdType* p_luma_cmd = &mod->reg_cmd.lumaUpdateCmd;

  if (!mod->lf_enable)
    return;

  p_luma_cmd->Cfg.cutoff_1 = ABF2_CUTOFF1(parm->threshold_red[0]);
  p_luma_cmd->Cfg.cutoff_2 = ABF2_CUTOFF2(p_luma_cmd->Cfg.cutoff_1,
    parm->threshold_red[1]);
  p_luma_cmd->Cfg.cutoff_3 = ABF2_CUTOFF3(p_luma_cmd->Cfg.cutoff_2,
    parm->threshold_red[2]);
  p_luma_cmd->Cfg.mult_neg = ABF2_MULT_NEG(p_luma_cmd->Cfg.cutoff_2,
    p_luma_cmd->Cfg.cutoff_3);
  p_luma_cmd->Cfg.mult_pos = ABF2_MULT_POS(p_luma_cmd->Cfg.cutoff_1);

  for (i = 0; i < 8; i++) {
    p_luma_cmd->pos_LUT[i].lut0 =
      ABF2_LUT(parm->table_pos[2*i] * parm->scale_factor_red[0]);
    p_luma_cmd->pos_LUT[i].lut1 =
      ABF2_LUT(parm->table_pos[2*i+1] * parm->scale_factor_red[0]);
  }

  for (i = 0; i < 4; i++) {
    p_luma_cmd->neg_LUT[i].lut0 =
      ABF2_LUT(parm->table_neg[2*i] * parm->scale_factor_red[1]);
    p_luma_cmd->neg_LUT[i].lut1 =
      ABF2_LUT(parm->table_neg[2*i+1] * parm->scale_factor_red[1]);
  }
} /* util_clf_set_luma_params */

/** util_clf_set_chroma_params:
 *    @mod: CLF module instance
 *    @parm: chroma parameters
 *    @is_snap: is snapshot - obsolete
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function sets chroma parameters
 *
 *  Return: None
 **/
static void util_clf_set_chroma_params(isp_clf_mod_t* mod,
  Chroma_filter_type *parm, int8_t is_snap)
{
  ISP_CLF_Chroma_Update_CmdType* p_chroma_cmd = &mod->reg_cmd.chromaUpdateCmd;

  if (!mod->cf_enable) {
    ISP_DBG(ISP_MOD_CLF, "%s: CF not enabled\n", __func__);
    return;
  }

  p_chroma_cmd->chroma_coeff.h_coeff0 = CLF_CF_COEFF(parm->h[0]);
  p_chroma_cmd->chroma_coeff.h_coeff1 = CLF_CF_COEFF(parm->h[1]);
  p_chroma_cmd->chroma_coeff.h_coeff2 = CLF_CF_COEFF(parm->h[2]);
  p_chroma_cmd->chroma_coeff.h_coeff3 = CLF_CF_COEFF(parm->h[3]);

  p_chroma_cmd->chroma_coeff.v_coeff0 = CLF_CF_COEFF(parm->v[0]);
  p_chroma_cmd->chroma_coeff.v_coeff1 = CLF_CF_COEFF(parm->v[1]);
} /* util_clf_set_chroma_parms */

/** clf_chroma_trigger_update:
 *    @mod: CLF module instance
 *    @in_params: triger update data
 *    @in_param_size: update data size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function updates parametes and sets flag for hw update if needed
 *  for chroma filtering
 *
 *  Return:  0 - Success
 *          -1 - parameter size mismatch
 **/
static int clf_chroma_trigger_update(isp_clf_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_CL_filter_type *chromatix_CL_filter =
    &chroma_ptr->chromatix_VFE.chromatix_CL_filter;

  trigger_point_type  *p_trigger_point = NULL;
  int8_t is_burst = IS_BURST_STREAMING(&(in_params->cfg));
  tuning_control_type tuning_type;
  int8_t update_cf = FALSE;
  float ratio;
  Chroma_filter_type *p_cf_new = &(mod->clf_params.cf_param);
  Chroma_filter_type *p_cf = NULL;
  int8_t cf_enabled = mod->cf_enable && mod->cf_enable_trig;
  int status = 0;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  if (!cf_enabled) {
    ISP_DBG(ISP_MOD_CLF, "%s: Chroma filter trigger not enabled %d %d", __func__,
      mod->cf_enable, mod->cf_enable_trig);
    return 0;
  }

  mod->cf_update = FALSE;

  tuning_type = chromatix_CL_filter->control_chroma_filter;
  p_trigger_point = &chromatix_CL_filter->chroma_filter_trigger_lowlight;
  p_cf = chromatix_CL_filter->chroma_filter;

  ratio = isp_util_get_aec_ratio(mod->notify_ops->parent, tuning_type,
    p_trigger_point, &(in_params->trigger_input.stats_update.aec_update),
    is_burst);

  update_cf = (mod->old_streaming_mode != in_params->cfg.streaming_mode) ||
    !F_EQUAL(ratio, mod->cur_cf_aec_ratio);

  if (update_cf) {
    util_clf_chroma_interpolate(&p_cf[1], &p_cf[0], p_cf_new, ratio);
    mod->old_streaming_mode = in_params->cfg.streaming_mode;
    mod->cur_cf_aec_ratio = ratio;
    mod->cf_update = TRUE;
  }

  return status;
} /* clf_chroma_trigger_update */

/** clf_luma_trigger_update:
 *    @mod: CLF module instance
 *    @in_params: triger update data
 *    @in_param_size: update data size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function updates parametes and sets flag for hw update if needed
 *  for luma filtering
 *
 *  Return:  0 - Success
 *           Otherwise - get AEC ratio failed
 **/
static int clf_luma_trigger_update(isp_clf_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  trigger_point_type  *p_trigger_low = NULL, *p_trigger_bright = NULL;
  int8_t is_burst = IS_BURST_STREAMING(&(in_params->cfg));
  tuning_control_type tuning_type;
  int8_t update_lf = FALSE;
  float ratio;
  trigger_ratio_t trig_ratio;
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_ABF2_type *chromatix_ABF2 =
    &chroma_ptr->chromatix_VFE.chromatix_ABF2;
  int8_t lf_enabled = mod->lf_enable && mod->lf_enable_trig;
  int status = 0;

  if (!lf_enabled) {
    ISP_DBG(ISP_MOD_CLF, "%s: Luma filter trigger not enabled %d %d", __func__,
       mod->lf_enable, mod->lf_enable_trig);

    return status;
  }

  chromatix_adaptive_bayer_filter_data_type2 *p_lf_new =
    &(mod->clf_params.lf_param);
  chromatix_adaptive_bayer_filter_data_type2 *p_lf1 = NULL, *p_lf2 = NULL;

  mod->lf_update = FALSE;

  tuning_type = chromatix_ABF2->control_abf2;
  p_trigger_low = &chromatix_ABF2->abf2_low_light_trigger;
  p_trigger_bright = &chromatix_ABF2->abf2_bright_light_trigger;

  status = isp_util_get_aec_ratio2(mod->notify_ops->parent, tuning_type,
    p_trigger_bright, p_trigger_low,
    &(in_params->trigger_input.stats_update.aec_update), is_burst, &trig_ratio);

  if (status != 0)
    CDBG_HIGH("%s: get aec ratio failed", __func__);

  switch (trig_ratio.lighting) {
  case TRIGGER_LOWLIGHT:
    p_lf1 = &chromatix_ABF2->abf2_config_low_light;
    p_lf2 = &chromatix_ABF2->abf2_config_normal_light;
    break;

  case TRIGGER_OUTDOOR:
    p_lf1 = &chromatix_ABF2->abf2_config_bright_light;
    p_lf2 = &chromatix_ABF2->abf2_config_normal_light;
    break;

  case TRIGGER_NORMAL:
  default:
    p_lf1 = p_lf2 = &chromatix_ABF2->abf2_config_normal_light;
    break;
  }

  update_lf =
   (trig_ratio.lighting != mod->cur_lf_aec_ratio.lighting) ||
   (trig_ratio.ratio != mod->cur_lf_aec_ratio.ratio) ||
   (in_params->cfg.streaming_mode != mod->old_streaming_mode);

  ISP_DBG(ISP_MOD_CLF, "%s: update_lf %d ratio %f lighting %d", __func__, update_lf,
    trig_ratio.ratio, trig_ratio.lighting);
  if (update_lf) {
    if(F_EQUAL(trig_ratio.ratio, 0.0) || F_EQUAL(trig_ratio.ratio, 1.0))
      *p_lf_new = *p_lf1;
    else
      util_clf_luma_interpolate(p_lf2, p_lf1, p_lf_new, trig_ratio.ratio);

    mod->cur_lf_aec_ratio = trig_ratio;
    /* need to handle for luma case */
    mod->old_streaming_mode = in_params->cfg.streaming_mode;
    mod->lf_update = TRUE;
  }

  return status;
} /* clf_luma_trigger_update */

/** clf_reset:
 *    @mod: CLF module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets CLF module
 *
 *  Return:  None
 **/
static void clf_reset(isp_clf_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->cur_lf_aec_ratio, 0, sizeof(mod->cur_lf_aec_ratio));
  memset(&mod->clf_params, 0, sizeof(mod->clf_params));
  mod->cur_cf_aec_ratio = 0;
  mod->hw_update_pending = 0;
  mod->trigger_enable = 0; /* enable trigger update feature flag from PIX */
  mod->skip_trigger = 0;
  mod->enable = 0;         /* enable flag from PIX */

  mod->cf_enable = 0;
  mod->lf_enable = 0;
  mod->cf_update = 0;
  mod->lf_update = 0;
  mod->cf_enable_trig = 0;
  mod->lf_enable_trig = 0;
}

/** clf_init:
 *    @mod: CLF module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function inits CLF module
 *
 *  Return:  0
 **/
static int clf_init (void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_clf_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  clf_reset(mod);

  return 0;
} /* clf_init */

/** clf_config:
 *    @mod: CLF module instance
 *    @in_params: configuration parameters
 *    @in_param_size: configuration parameters size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial configuration of CLF module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static int clf_config(isp_clf_mod_t *mod, isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int  rc = 0;
  chromatix_parms_type *chromatix_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_ABF2_type *chromatix_ABF2 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF2;
  chromatix_CL_filter_type *chromatix_CL_filter =
    &chromatix_ptr->chromatix_VFE.chromatix_CL_filter;
  int is_burst;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  ISP_DBG(ISP_MOD_CLF, "%s: enter", __func__);

  if (!mod->enable) {
    ISP_DBG(ISP_MOD_CLF, "%s: Mod not Enable.", __func__);
    return rc;
  }

  if (!IS_CLF_ENABLED(mod)) {
    ISP_DBG(ISP_MOD_CLF, "%s: CLF not enabled", __func__);
    return 0;
  }

  is_burst = IS_BURST_STREAMING(in_params);
  /* set old cfg to invalid value to trigger the first trigger update */
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  mod->cf_enable_trig = TRUE;
  mod->lf_enable_trig = TRUE;
  mod->cf_enable = TRUE;
  mod->lf_enable = TRUE;
  mod->trigger_enable = TRUE;

  mod->reg_cmd.clf_cfg.colorconv_enable = (uint32_t) in_params->camif_cfg.is_bayer_sensor;
  mod->reg_cmd.clf_cfg.flush_halt_ovd = 0;
  mod->reg_cmd.clf_cfg.pipe_flush_cnt = 0x400;
  mod->reg_cmd.clf_cfg.pipe_flush_ovd = 1;

  /*preview*/
  util_clf_set_luma_params(mod, &chromatix_ABF2->abf2_config_normal_light);
  util_clf_set_chroma_params(mod, &chromatix_CL_filter->chroma_filter[1],
    is_burst);

  //originally config
  if (is_burst) {
    ISP_CLF_CmdType* ISP_CLF_Cmd = &mod->reg_cmd;

    chromatix_adaptive_bayer_filter_data_type2* p_lf =
      (mod->lf_update) ?
      &mod->clf_params.lf_param : &chromatix_ABF2->abf2_config_normal_light;

    Chroma_filter_type* p_cf =
      (mod->cf_update) ?
      &mod->clf_params.cf_param : &chromatix_CL_filter->chroma_filter[1];

    ISP_DBG(ISP_MOD_CLF, "%s: lf_update %d cf_update %d", __func__, mod->lf_update,
       mod->cf_update);

    util_clf_set_luma_params(mod, p_lf);
    util_clf_set_chroma_params(mod, p_cf, is_burst);
  }

  mod->skip_trigger = FALSE;
  mod->hw_update_pending = TRUE;

  return rc;
} /* clf_config */

/** clf_destroy:
 *    @mod: CLF module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function destroys CLF module
 *
 *  Return:   0 - Success
 **/
static int clf_destroy (void *mod_ctrl)
{
  isp_clf_mod_t *mod = mod_ctrl;

  memset(mod,  0,  sizeof(isp_clf_mod_t));
  free(mod);
  return 0;
} /* clf_destroy */

/** clf_enable:
 *    @mod: CLF module instance
 *    @enable: true if module is to be enabled
 *    @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function enables CLF module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static int clf_enable(isp_clf_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);

    return -1;
  }

  mod->lf_enable = mod->cf_enable = mod->enable = enable->enable;

  int clf_enable = IS_CLF_ENABLED(mod);
  ISP_DBG(ISP_MOD_CLF, "%s: clf_enable %d", __func__, clf_enable);

  if (!clf_enable)
    /* disable both modules */
    mod->lf_enable = mod->cf_enable = FALSE;
  else
    mod->lf_enable = mod->cf_enable = TRUE;

  if (!mod->enable)
      mod->hw_update_pending = 0;

  return 0;
} /* clf_enable */

/** clf_trigger_enable:
 *    @mod: CLF module instance
 *    @enable: true if triger update is enabled
 *    @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function enables triger update of CLF module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static int clf_trigger_enable(isp_clf_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->trigger_enable = enable->enable;
  return 0;
} /* clf_trigger_enable */

/** clf_trigger_update:
 *    @mod: CLF module instance
 *    @in_params: module trigger update params
 *    @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static int clf_trigger_update(isp_clf_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  ISP_CLF_CmdType *reg_cmd = &mod->reg_cmd;
  chromatix_parms_type *chromatix_ptr =
    in_params->cfg.chromatix_ptrs.chromatixPtr;
  trigger_point_type *p_trigger_point;
  float aec_ratio;
  cs_luma_threshold_type *cs_luma_threshold, *cs_luma_threshold_lowlight;
  uint8_t update_clf = FALSE;
  int is_burst;
  int status = 0;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
  return -1;
  }

  if (!mod->enable || !mod->trigger_enable || mod->skip_trigger) {
    ISP_DBG(ISP_MOD_CLF, "%s: Skip Trigger update. enable %d, trig_enable %d, skip_trigger %d",
      __func__, mod->enable, mod->trigger_enable, mod->skip_trigger);
    return rc;
  }

  is_burst = IS_BURST_STREAMING(&in_params->cfg);

  if (!is_burst && !isp_util_aec_check_settled(&in_params->trigger_input.stats_update.aec_update)) {
    ISP_DBG(ISP_MOD_CLF, "%s: AEC not settled", __func__);
    return rc;
  }

  status = clf_luma_trigger_update(mod, in_params, in_param_size);
  if (0 != status) {
    ISP_DBG(ISP_MOD_CLF, "%s: isp_clf_luma_trigger_update failed", __func__);
    return status;
  }
  status = clf_chroma_trigger_update(mod, in_params, in_param_size);
  if (0 != status) {
    ISP_DBG(ISP_MOD_CLF, "%s: isp_clf_luma_trigger_update failed", __func__);
    return status;
  }

  //update:
  if (mod->cf_update) {
    ISP_DBG(ISP_MOD_CLF, "%s: update required", __func__);

    util_clf_set_chroma_params(mod, &mod->clf_params.cf_param, is_burst);
    mod->hw_update_pending = TRUE;
  }

  return 0;
} /* clf_trigger_update */

/** clf_set_chromatix:
 *    @mod: CLF module instance
 *    @in_params: true if module is to be enabled
 *    @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static int clf_set_chromatix(isp_clf_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
  return -1;
  }
  chromatix_parms_type * chromatix_ptr =
    (chromatix_parms_type *) in_params->chromatix_ptrs.chromatixPtr;
  chromatix_ABF2_type *chromatix_ABF2 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF2;
  chromatix_CL_filter_type *chromatix_CL_filter =
    &chromatix_ptr->chromatix_VFE.chromatix_CL_filter;
  int is_burst = IS_BURST_STREAMING(in_params);

  /*snapshot*/
  util_clf_set_luma_params(mod, &chromatix_ABF2->abf2_config_normal_light);
  util_clf_set_chroma_params(mod, &chromatix_CL_filter->chroma_filter[1],
    is_burst);

  mod->skip_trigger = FALSE;

  return 0;
} /* clf_set_chromatix */

/** clf_do_hw_update:
 *    @clf_mod: CLF module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:   0 - Success
 *           -1 - configuration error
 **/
static int clf_do_hw_update(isp_clf_mod_t *clf_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (clf_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &clf_mod->reg_cmd;
    cfg_cmd.cmd_len = sizeof(clf_mod->reg_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CLF40_CFG_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_CLF40_CFG_LEN * sizeof(uint32_t);

    util_clf_debug(&clf_mod->reg_cmd);
    rc = ioctl(clf_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    clf_mod->hw_update_pending = 0;
  }

  /* TODO: update hw reg */
  return rc;
} /* clf_hw_reg_update */

/** clf_set_params:
 *    @mod_ctrl: CLF module instance
 *    @param_id: parameter id
 *    @in_params: parameter data
 *    @in_param_size: parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function sets a parameter to CLF module
 *
 *  Return:   0 - Success
 *            Negative - paramter set error
 **/
static int clf_set_params (void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_clf_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_CHROMATIX_RELOAD:
    rc = clf_set_chromatix(mod, (isp_hw_pix_setting_params_t *) in_params,
      in_param_size);
    break;

  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = clf_enable(mod, (isp_mod_set_enable_t *) in_params, in_param_size);
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = clf_config(mod, (isp_hw_pix_setting_params_t *) in_params,
      in_param_size);
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = clf_trigger_enable(mod, (isp_mod_set_enable_t *) in_params,
      in_param_size);
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = clf_trigger_update(mod,
        (isp_pix_trigger_update_input_t *)in_params, in_param_size);
    break;

  default:
    return -EAGAIN;
    break;
  }
  return rc;
} /* clf_set_params */

/** clf_get_params:
 *    @mod_ctrl: CLF module instance
 *    @param_id: parameter id
 *    @in_params: input parameter data
 *    @in_param_size: input parameter size
 *    @out_params: output parameter data
 *    @out_param_size: output parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function sets a parameter to CLF module
 *
 *  Return:   0 - Success
 *            Negative - paramter get error
 **/
static int clf_get_params (void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_clf_mod_t *mod = mod_ctrl;
  int rc =0;

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

  default:
    rc = -1;
    break;
  }

  return 0;
} /* clf_get_params */

/** clf_action:
 *    @mod_ctrl: CLF module instance
 *    @action_code: action id
 *    @data: input parameter data
 *    @data_size: input parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function executes an CLF module action
 *
 *  Return:   0 - Success
 *            Negative - action execution error
 **/
static int clf_action (void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_clf_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = clf_do_hw_update(mod);
    break;
  case ISP_HW_MOD_ACTION_RESET:
    clf_reset(mod);
    break;
  default:
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
    break;
  }
  return rc;
} /* clf_action */

/** clf44_open:
 *    @version: version
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function instantiates a CLF module
 *
 *  Return:   NULL - not enough memory
 *            Otherwise handle to module instance
 **/
isp_ops_t *clf44_open(uint32_t version)
{
  isp_clf_mod_t *mod = malloc(sizeof(isp_clf_mod_t));

  ISP_DBG(ISP_MOD_CLF, "%s: E\n", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }
  memset(mod,  0,  sizeof(isp_clf_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = clf_init;
  mod->ops.destroy = clf_destroy;
  mod->ops.set_params = clf_set_params;
  mod->ops.get_params = clf_get_params;
  mod->ops.action = clf_action;

  return &mod->ops;
} /* clf44_open */
