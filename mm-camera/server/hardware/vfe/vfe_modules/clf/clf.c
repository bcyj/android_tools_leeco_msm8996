/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CLF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define CLF_START_REG 0x06B4

#define IS_CLF_ENABLED(mod) (mod->cf_enable || mod->lf_enable)

/*===========================================================================
 * Function:           vfe_disable_chroma
 *
 * Description:
 *=========================================================================*/
void vfe_disable_chroma(VFE_CLF_Chroma_Update_CmdType* chroma)
{
  chroma->chroma_coeff.h_coeff0 = chroma->chroma_coeff.v_coeff0 = 1;
  chroma->chroma_coeff.h_coeff1 = chroma->chroma_coeff.h_coeff2 =
    chroma->chroma_coeff.h_coeff3 = chroma->chroma_coeff.v_coeff1 = 0;
}

/*===========================================================================
 * Function:           vfe_disable_luma
 *
 * Description:
 *=========================================================================*/
void vfe_disable_luma(VFE_CLF_Luma_Update_CmdType* luma)
{
  memset(&luma->luma_pos_LUT, 0x0, sizeof(VFE_CLF_Luma_Pos_Lut) * 8);
  memset(&luma->luma_neg_LUT, 0x0, sizeof(VFE_CLF_Luma_Neg_Lut) * 4);
}

/*===========================================================================
 * Function:           vfe_clf_chroma_debug
 *
 * Description:
 *=========================================================================*/
void vfe_clf_chroma_debug(VFE_CLF_Chroma_Update_CmdType* p_cmd)
{
  int rc = TRUE;
  CDBG("v_coeff0 %d v_coeff1 %d", p_cmd->chroma_coeff.v_coeff0,
     p_cmd->chroma_coeff.v_coeff1);

  CDBG("h_coeff0 %d h_coeff1 %d h_coeff2 %d h_coeff3 %d",
       p_cmd->chroma_coeff.h_coeff0, p_cmd->chroma_coeff.h_coeff1,
       p_cmd->chroma_coeff.h_coeff2, p_cmd->chroma_coeff.h_coeff3);

}

/*===========================================================================
 * Function:           vfe_clf_luma_debug
 *
 * Description:
 *=========================================================================*/
void vfe_clf_luma_debug(VFE_CLF_Luma_Update_CmdType* p_cmd)
{
  int i = 0;
  CDBG("cutoff_1 %d cutoff_2 %d cutoff_3 %d",
    p_cmd->luma_Cfg.cutoff_1, p_cmd->luma_Cfg.cutoff_2,
    p_cmd->luma_Cfg.cutoff_3);
  CDBG("mult_neg %d mult_pos %d",
    p_cmd->luma_Cfg.mult_neg, p_cmd->luma_Cfg.mult_pos);

  for (i=0; i<8; i++)
    CDBG("posLUT%d %d posLUT%d %d", 2*i, p_cmd->luma_pos_LUT[i].pos_lut0,
      2*i+1, p_cmd->luma_pos_LUT[i].pos_lut1);

  for (i=0; i<4; i++)
    CDBG("negLUT%d %d negLUT%d %d", 2*i, p_cmd->luma_neg_LUT[i].neg_lut0,
      2*i+1, p_cmd->luma_neg_LUT[i].neg_lut1);
}

/*===========================================================================
 * Function:           vfe_clf_luma_update
 *
 * Description:
 *=========================================================================*/
void vfe_clf_debug(VFE_CLF_CmdType* p_cmd)
{
  CDBG("colorconv_enable %d pipe_flush_cnt %d",
   p_cmd->clf_cfg.colorconv_enable,
   p_cmd->clf_cfg.pipe_flush_cnt);
  CDBG("pipe_flush_ovd %d flush_halt_ovd %d",
   p_cmd->clf_cfg.pipe_flush_ovd,
   p_cmd->clf_cfg.flush_halt_ovd);

  vfe_clf_luma_debug(&p_cmd->lumaUpdateCmd);
  vfe_clf_chroma_debug(&p_cmd->chromaUpdateCmd);
}
/*===========================================================================
 * Function:           vfe_clf_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_enable(int mod_id, void *module, void *vparams,
  int8_t enable, int8_t hw_write)
{
  clf_mod_t *mod = (clf_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_clf_enable_type_t flag = (vfe_clf_enable_type_t)enable;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    flag = VFE_CLF_LUMA_CHROMA_DISABLE;
  int8_t is_snap = IS_SNAP_MODE(params);
  VFE_CLF_CmdType* VFE_CLF_Cmd = (is_snap) ?
    &mod->VFE_SnapshotCLF_Cmd : &mod->VFE_PrevCLF_Cmd;

  int clf_enable = IS_CLF_ENABLED(mod);
  CDBG("%s: flag %d clf_enable %d", __func__, flag, clf_enable);

  if (hw_write &&
    (((flag == VFE_CLF_LUMA_CHROMA_ENABLE) && clf_enable) ||
    ((flag == VFE_CLF_LUMA_CHROMA_DISABLE) && !clf_enable) ||
    ((flag == VFE_CLF_CHROMA_ENABLE) && mod->cf_enable) ||
    ((flag == VFE_CLF_LUMA_ENABLE) && mod->lf_enable))) {
    CDBG("%s: Enable not required", __func__);
    return VFE_SUCCESS;
  }

  if (flag == VFE_CLF_LUMA_CHROMA_DISABLE) {
    /* disable both modules */
    params->moduleCfg->clfEnable = FALSE;
    mod->lf_enable = mod->cf_enable = FALSE;
  } else {
    params->moduleCfg->clfEnable = TRUE;
    mod->lf_enable = mod->cf_enable = TRUE;
    if (flag == VFE_CLF_CHROMA_ENABLE) {
      vfe_disable_luma(&VFE_CLF_Cmd->lumaUpdateCmd);
      mod->lf_enable = FALSE;
    }
    if (flag == VFE_CLF_LUMA_ENABLE) {
      vfe_disable_chroma(&VFE_CLF_Cmd->chromaUpdateCmd);
      mod->cf_enable = FALSE;
    }
  }
  if (hw_write) {
    CDBG("%s: Update hardware flasg %d", __func__, flag);
    if ((flag == VFE_CLF_LUMA_CHROMA_DISABLE) ||
        (flag == VFE_CLF_LUMA_CHROMA_ENABLE)) {
      mod->hw_enable = TRUE;
    } else {
      mod->lf_update = mod->lf_enable;
      mod->cf_update = mod->cf_enable;
    }
    params->current_config = (flag != VFE_CLF_LUMA_CHROMA_DISABLE) ?
      (params->current_config | VFE_MOD_CLF)
      : (params->current_config & ~VFE_MOD_CLF);
  }
  return status;
}

/*===========================================================================
 * Function:           vfe_clf_set_chroma_parms
 *
 * Description:
 *=========================================================================*/
void vfe_clf_set_chroma_parms(clf_mod_t* mod,
  Chroma_filter_type *parm, int8_t is_snap)
{
  VFE_CLF_CmdType* VFE_CLF_Cmd = (is_snap) ?
    &mod->VFE_SnapshotCLF_Cmd : &mod->VFE_PrevCLF_Cmd;

  if (!mod->cf_enable)
    return;

  VFE_CLF_Cmd->chromaUpdateCmd.chroma_coeff.h_coeff0 =
    CLF_CF_COEFF(parm->h[0]);
  VFE_CLF_Cmd->chromaUpdateCmd.chroma_coeff.h_coeff1 =
    CLF_CF_COEFF(parm->h[1]);
  VFE_CLF_Cmd->chromaUpdateCmd.chroma_coeff.h_coeff2 =
    CLF_CF_COEFF(parm->h[2]);
  VFE_CLF_Cmd->chromaUpdateCmd.chroma_coeff.h_coeff3 =
    CLF_CF_COEFF(parm->h[3]);

  VFE_CLF_Cmd->chromaUpdateCmd.chroma_coeff.v_coeff0 =
    CLF_CF_COEFF(parm->v[0]);
  VFE_CLF_Cmd->chromaUpdateCmd.chroma_coeff.v_coeff1 =
    CLF_CF_COEFF(parm->v[1]);
}

/*===========================================================================
 * Function:           vfe_clf_chroma_interpolate
 *
 * Description:
 *=========================================================================*/
void vfe_clf_chroma_interpolate(Chroma_filter_type *in1,
  Chroma_filter_type *in2,
  Chroma_filter_type *out, float ratio)
{
  int i = 0;
  TBL_INTERPOLATE(in1->h, in2->h, out->h, ratio, 4, i);
  TBL_INTERPOLATE(in1->v, in2->v, out->v, ratio, 2, i);
}

/*===========================================================================
 * Function:           vfe_clf_chroma_trigger_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_chroma_trigger_update(clf_mod_t* mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  trigger_point_type  *p_trigger_point = NULL;
  int8_t is_snapmode = IS_SNAP_MODE(parms);
  tuning_control_type tuning_type;
  int8_t update_cf = FALSE;
  float ratio;
  Chroma_filter_type *p_cf_new = &(mod->clf_params.cf_param);
  Chroma_filter_type *p_cf = NULL;
  int8_t cf_enabled = mod->cf_enable && mod->cf_enable_trig;

  if (!cf_enabled) {
    CDBG("%s: Chroma filter trigger not enabled %d %d", __func__,
      mod->cf_enable, mod->cf_enable_trig);
    return VFE_SUCCESS;
  }

  mod->cf_update = FALSE;
  if (!is_snapmode) {
    tuning_type = chromatix_ptr->control_chroma_filter_preview;
    p_trigger_point = &chromatix_ptr->chroma_filter_trigger_lowlight_preview;
    p_cf = chromatix_ptr->chroma_filter_preview;

  } else {
    tuning_type = chromatix_ptr->control_chroma_filter_snapshot;
    p_trigger_point = &chromatix_ptr->chroma_filter_trigger_lowlight_snapshot;
    p_cf = chromatix_ptr->chroma_filter_snapshot;
  }

  ratio = vfe_util_get_aec_ratio(tuning_type, p_trigger_point, parms);

  update_cf = (mod->cur_mode_chroma != parms->vfe_op_mode) ||
    !F_EQUAL(ratio, mod->cur_cf_ratio) ||
    mod->reload_params;

  if (update_cf) {
    vfe_clf_chroma_interpolate(&p_cf[1], &p_cf[0], p_cf_new, ratio);
    mod->cur_mode_chroma = parms->vfe_op_mode;
    mod->cur_cf_ratio = ratio;
    mod->cf_update = TRUE;
  }
  return status;
}

/*===========================================================================
 * Function:           vfe_clf_set_luma_parms
 *
 * Description:
 *=========================================================================*/
void vfe_clf_set_luma_parms(clf_mod_t *mod,
  chromatix_adaptive_bayer_filter_data_type2 *parm,
  int8_t is_snap)
{
  int i = 0;
  int32_t temp;
  VFE_CLF_CmdType* VFE_CLF_Cmd = (is_snap) ?
    &mod->VFE_SnapshotCLF_Cmd : &mod->VFE_PrevCLF_Cmd;

  if (!mod->lf_enable)
    return;

  VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_1 =
    ABF2_CUTOFF1(parm->threshold_red[0]);

  VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_2 =
    ABF2_CUTOFF2(VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_1,
      parm->threshold_red[1]);
  VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_3 =
    ABF2_CUTOFF3(VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_2,
      parm->threshold_red[2]);
  VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.mult_neg =
    ABF2_MULT_NEG(VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_2,
      VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_3);
  VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.mult_pos =
    ABF2_MULT_POS(VFE_CLF_Cmd->lumaUpdateCmd.luma_Cfg.cutoff_1);

  for (i=0; i<8; i++) {
    VFE_CLF_Cmd->lumaUpdateCmd.luma_pos_LUT[i].pos_lut0 =
      ABF2_LUT(parm->table_pos[2*i] * parm->scale_factor_red[0]);
    VFE_CLF_Cmd->lumaUpdateCmd.luma_pos_LUT[i].pos_lut1 =
      ABF2_LUT(parm->table_pos[2*i+1] * parm->scale_factor_red[0]);
  }

  for (i=0; i<4; i++) {
    VFE_CLF_Cmd->lumaUpdateCmd.luma_neg_LUT[i].neg_lut0 =
      ABF2_LUT(parm->table_neg[2*i] * parm->scale_factor_red[1]);
    VFE_CLF_Cmd->lumaUpdateCmd.luma_neg_LUT[i].neg_lut1 =
      ABF2_LUT(parm->table_neg[2*i+1] * parm->scale_factor_red[1]);
  }
}

/*===========================================================================
 * Function:           vfe_clf_luma_interpolate
 *
 * Description:
 *=========================================================================*/
void vfe_clf_luma_interpolate(chromatix_adaptive_bayer_filter_data_type2 *in1,
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

}

/*===========================================================================
 * Function:           vfe_clf_luma_trigger_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_luma_trigger_update(clf_mod_t* mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  trigger_point_type  *p_trigger_low = NULL, *p_trigger_bright = NULL;
  int8_t is_snapmode = IS_SNAP_MODE(parms);
  tuning_control_type tuning_type;
  int8_t update_lf = FALSE;
  float ratio;
  trigger_ratio_t trig_ratio;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  int8_t lf_enabled = mod->lf_enable && mod->lf_enable_trig;

  if (!lf_enabled) {
    CDBG("%s: Luma filter trigger not enabled %d %d", __func__,
       mod->lf_enable, mod->lf_enable_trig);
    return status;
  }

  chromatix_adaptive_bayer_filter_data_type2 *p_lf_new =
    &(mod->clf_params.lf_param);
  chromatix_adaptive_bayer_filter_data_type2 *p_lf1 = NULL, *p_lf2 = NULL;

  mod->lf_update = FALSE;
  if (!is_snapmode) {
    tuning_type = chromatix_ptr->control_ABF;
    p_trigger_low = &chromatix_ptr->LF_low_light_trigger_preview;
    p_trigger_bright = &chromatix_ptr->LF_bright_light_trigger_preview;
  } else {
    tuning_type = chromatix_ptr->control_ABF;
    p_trigger_low = &chromatix_ptr->LF_low_light_trigger_snapshot;
    p_trigger_bright = &chromatix_ptr->LF_bright_light_trigger_snapshot;
  }

  trig_ratio =
    vfe_util_get_aec_ratio2(tuning_type, p_trigger_bright, p_trigger_low,
    parms);

  switch (trig_ratio.lighting) {
  case TRIGGER_LOWLIGHT:
    if (!is_snapmode) {
      p_lf1 = &chromatix_ptr->LF_config_low_light_preview;
      p_lf2 = &chromatix_ptr->LF_config_normal_light_preview;
    } else {
      p_lf1 = &chromatix_ptr->LF_config_low_light_snapshot;
      p_lf2 = &chromatix_ptr->LF_config_normal_light_snapshot;
    }
    break;
  case TRIGGER_OUTDOOR:
    if (!is_snapmode) {
      p_lf1 = &chromatix_ptr->LF_config_bright_light_preview;
      p_lf2 = &chromatix_ptr->LF_config_normal_light_preview;
    } else {
      p_lf1 = &chromatix_ptr->LF_config_bright_light_snapshot;
      p_lf2 = &chromatix_ptr->LF_config_normal_light_snapshot;
    }
    break;
  default:
  case TRIGGER_NORMAL:
    p_lf1 = p_lf2 = (!is_snapmode) ?
      &chromatix_ptr->LF_config_normal_light_preview :
      &chromatix_ptr->LF_config_normal_light_snapshot;
    break;
  }

  update_lf =
   (trig_ratio.lighting != mod->cur_lf_trig_ratio.lighting)
   || (trig_ratio.ratio != mod->cur_lf_trig_ratio.ratio)
   || (parms->vfe_op_mode != mod->cur_mode_luma)
   || mod->reload_params;

  CDBG("%s: update_lf %d ratio %f lighting %d", __func__, update_lf,
    trig_ratio.ratio, trig_ratio.lighting);
  if (update_lf) {
    if(F_EQUAL(trig_ratio.ratio, 0.0) || F_EQUAL(trig_ratio.ratio, 1.0))
      *p_lf_new = *p_lf1;
    else
      vfe_clf_luma_interpolate(p_lf2, p_lf1, p_lf_new, trig_ratio.ratio);

    mod->cur_lf_trig_ratio = trig_ratio;
    mod->cur_mode_luma = parms->vfe_op_mode;
    mod->lf_update = TRUE;
  }
  return status;
}

/*===========================================================================
 * Function:           vfe_clf_trigger_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_trigger_update(int module_id, void *module,
  void* vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  clf_mod_t *mod = (clf_mod_t *)module;
  vfe_params_t *parms = (vfe_params_t *)vparams;

  if (!mod->trigger_enable) {
    CDBG("%s: CLF trigger not enabled", __func__);
    return status;
  }

  status = vfe_clf_luma_trigger_update(mod, parms);
  if (VFE_SUCCESS != status) {
    CDBG("%s: vfe_clf_luma_trigger_update failed", __func__);
    return status;
  }
  status = vfe_clf_chroma_trigger_update(mod, parms);
  if (VFE_SUCCESS != status) {
    CDBG("%s: vfe_clf_luma_trigger_update failed", __func__);
    return status;
  }

  if (mod->reload_params)
    mod->reload_params = FALSE;

  return status;
}/* vfe_clf_trigger_update */

/*===========================================================================
 * Function:           vfe_clf_video_config
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_video_config(clf_mod_t* mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  VFE_CLF_CmdType* VFE_CLF_Cmd = &mod->VFE_PrevCLF_Cmd;
  int rc = TRUE;

  CDBG("CLF Video config");
  vfe_clf_debug(VFE_CLF_Cmd);

  status = vfe_util_write_hw_cmd(parms->camfd,
    CMD_GENERAL, (void *) VFE_CLF_Cmd,
    sizeof(VFE_CLF_CmdType), VFE_CMD_CLF_CFG);

  if (VFE_SUCCESS != status)
    CDBG_HIGH("%s: failed", __func__);
  else
    mod->cf_update = mod->lf_update = FALSE;

  return status;
}

/*===========================================================================
 * Function:           vfe_clf_snapshot_config
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_snapshot_config(clf_mod_t* mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  VFE_CLF_CmdType* VFE_CLF_Cmd = &mod->VFE_SnapshotCLF_Cmd;
  chromatix_adaptive_bayer_filter_data_type2* p_lf =
    (mod->lf_update) ? &mod->clf_params.lf_param
    : &chromatix_ptr->LF_config_normal_light_snapshot;
  Chroma_filter_type* p_cf = (mod->cf_update) ?
    &mod->clf_params.cf_param :
    &chromatix_ptr->chroma_filter_snapshot[1];

  CDBG("%s: lf_update %d cf_update %d", __func__, mod->lf_update,
     mod->cf_update);

  vfe_clf_set_luma_parms(mod, p_lf, TRUE);

  vfe_clf_set_chroma_parms(mod, p_cf, TRUE);

  CDBG("CLF Snapshot config");
  vfe_clf_debug(VFE_CLF_Cmd);

  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) VFE_CLF_Cmd,
    sizeof(VFE_CLF_CmdType), VFE_CMD_CLF_CFG);

  if (VFE_SUCCESS != status)
    CDBG_HIGH("%s: failed", __func__);
  else
    mod->cf_update = mod->lf_update = FALSE;

  return status;
}

/*===========================================================================
 * Function:           vfe_clf_snapshot_config
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_config(int mod_id, void* module, void* vparams)
{
  clf_mod_t *mod = (clf_mod_t *)module;
  vfe_params_t *parms = (vfe_params_t *)vparams;
  int is_snap = IS_SNAP_MODE(parms);

  if (!IS_CLF_ENABLED(mod)) {
    CDBG("%s: CLF not enabled", __func__);
    return VFE_SUCCESS;
  }
  return is_snap ? vfe_clf_snapshot_config(mod, parms) :
    vfe_clf_video_config(mod, parms);
}

/*===========================================================================
 * Function:           vfe_clf_chroma_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_chroma_update(clf_mod_t* mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(parms);
  VFE_CLF_CmdType* VFE_CLF_Cmd = (is_snap) ?
    &mod->VFE_SnapshotCLF_Cmd : &mod->VFE_PrevCLF_Cmd;
  int rc = TRUE;

  if (!mod->cf_update || !mod->cf_enable) {
    CDBG("%s: update not required", __func__);
    return FALSE;
  }

  vfe_clf_set_chroma_parms(mod, &mod->clf_params.cf_param, is_snap);

  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) &VFE_CLF_Cmd->chromaUpdateCmd,
    sizeof(VFE_CLF_Cmd->chromaUpdateCmd), VFE_CMD_CLF_CHROMA_UPDATE);

  if (VFE_SUCCESS != status)
    CDBG_HIGH("%s: failed", __func__);
  else
    mod->cf_update = FALSE;
  return status;
}

/*===========================================================================
 * Function:           vfe_clf_luma_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_luma_update(clf_mod_t* mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(parms);
  VFE_CLF_CmdType* VFE_CLF_Cmd = (is_snap) ?
    &mod->VFE_SnapshotCLF_Cmd : &mod->VFE_PrevCLF_Cmd;

  if (!mod->lf_update || !mod->lf_enable) {
    CDBG("%s: update not required", __func__);
    return status;
  }

  vfe_clf_set_luma_parms(mod, &mod->clf_params.lf_param, is_snap);
  CDBG("CLF update");
  vfe_clf_debug(VFE_CLF_Cmd);

  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) &VFE_CLF_Cmd->lumaUpdateCmd,
    sizeof(VFE_CLF_Cmd->lumaUpdateCmd), VFE_CMD_CLF_LUMA_UPDATE);

  if (VFE_SUCCESS != status)
    CDBG_HIGH("%s: failed", __func__);
  else
    mod->lf_update = FALSE;

  return status;
}

/*===========================================================================
 * Function:           vfe_clf_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_update(int mod_id, void* module, void* vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  clf_mod_t *mod = (clf_mod_t *)module;
  vfe_params_t *parms = (vfe_params_t *)vparams;

  if (mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(parms->camfd,
      CMD_GENERAL, parms->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    parms->update |= VFE_MOD_CLF;
    mod->hw_enable = FALSE;
  }

  status = vfe_clf_luma_update(mod, parms);
  if (VFE_SUCCESS != status) {
    CDBG("%s: vfe_clf_luma_trigger_update failed", __func__);
    return status;
  }
  status = vfe_clf_chroma_update(mod, parms);
  if (VFE_SUCCESS != status) {
    CDBG("%s: vfe_clf_luma_trigger_update failed", __func__);
    return status;
  } else {
    if (mod->cf_update || mod->lf_update)
      parms->update |= VFE_MOD_CLF;
  }
  return status;
}/* vfe_clf_update */

/*===========================================================================
 * Function:           vfe_clf_init
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_clf_init(int mod_id, void* module, void* vparms)
{
  clf_mod_t *mod = (clf_mod_t *)module;
  vfe_params_t *parms = (vfe_params_t *)vparms;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  vfe_status_t status = VFE_SUCCESS;

  mod->cf_enable_trig = TRUE;
  mod->lf_enable_trig = TRUE;
  mod->cf_enable = TRUE;
  mod->lf_enable = TRUE;
  mod->trigger_enable = TRUE;
  mod->cur_mode_luma = VFE_OP_MODE_INVALID;
  mod->cur_mode_chroma = VFE_OP_MODE_INVALID;

  mod->VFE_PrevCLF_Cmd.clf_cfg.colorconv_enable = IS_BAYER_FORMAT(parms);
  mod->VFE_PrevCLF_Cmd.clf_cfg.flush_halt_ovd = 0;
  mod->VFE_PrevCLF_Cmd.clf_cfg.pipe_flush_cnt = 0x400;
  mod->VFE_PrevCLF_Cmd.clf_cfg.pipe_flush_ovd = 1;
  mod->VFE_SnapshotCLF_Cmd.clf_cfg = mod->VFE_PrevCLF_Cmd.clf_cfg;

  /*preview*/
  vfe_clf_set_luma_parms(mod,
    &chromatix_ptr->LF_config_normal_light_preview,
    FALSE);
  vfe_clf_set_chroma_parms(mod,
    &chromatix_ptr->chroma_filter_preview[1], FALSE);

  /*snapshot*/
  vfe_clf_set_luma_parms(mod,
    &chromatix_ptr->LF_config_normal_light_snapshot,
    TRUE);
  vfe_clf_set_chroma_parms(mod,
    &chromatix_ptr->chroma_filter_snapshot[1], TRUE);
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_clf_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_trigger_enable(int mod_id, void* module,
  void* params, int enable)
{
  clf_mod_t *mod = (clf_mod_t *)module;
  CDBG("%s: %d", __func__, enable);
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_clf_trigger_enable*/

/*===========================================================================
 * FUNCTION    - vfe_clf_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_reload_params(int mod_id, void *module, void* vparms)
{
  clf_mod_t *mod = (clf_mod_t *)module;
  vfe_params_t *parms = (vfe_params_t *)vparms;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  vfe_status_t status = VFE_SUCCESS;
  CDBG("%s:", __func__);
  /*preview*/
  vfe_clf_set_luma_parms(mod,
    &chromatix_ptr->LF_config_normal_light_preview,
    FALSE);
  vfe_clf_set_chroma_parms(mod,
    &chromatix_ptr->chroma_filter_preview[1], FALSE);

  /*snapshot*/
  vfe_clf_set_luma_parms(mod,
    &chromatix_ptr->LF_config_normal_light_snapshot,
    TRUE);
  vfe_clf_set_chroma_parms(mod,
    &chromatix_ptr->chroma_filter_snapshot[1], TRUE);

  mod->reload_params = TRUE;
  return status;
} /*vfe_clf_reload_params*/

/*===========================================================================
 * FUNCTION    - clf_validate_pos_lut -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_pos_lut(VFE_CLF_Luma_Pos_Lut *in, VFE_CLF_Luma_Pos_Lut
  *out)
{
  int i=0;
  for (i=0; i<8; i++) {
    if (!MATCH(in[i].pos_lut0, out[i].pos_lut0, 0))
      CDBG_TV("%s: luma_pos_LUT pos_lut%d in %d out %d doesnt match", __func__,
      2*i, in[i].pos_lut0, out[i].pos_lut0);

    if (!MATCH(in[i].pos_lut1, out[i].pos_lut1, 0))
      CDBG_TV("%s: luma_pos_LUT pos_lut%d in %d out %d doesnt match", __func__,
      2*i+1, in[i].pos_lut1, out[i].pos_lut1);
  }
}/*clf_validate_pos_lut*/

/*===========================================================================
 * FUNCTION    - clf_validate_neg_lut -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_neg_lut(VFE_CLF_Luma_Neg_Lut *in, VFE_CLF_Luma_Neg_Lut
  *out)
{
  int i=0;
  for (i=0; i<4; i++) {
    if (!MATCH(in[i].neg_lut0, out[i].neg_lut0, 0))
      CDBG_TV("%s: luma_neg_LUT neg_lut%d in %d out %d doesnt match", __func__,
      2*i, in[i].neg_lut0, out[i].neg_lut0);

    if (!MATCH(in[i].neg_lut1, out[i].neg_lut1, 0))
      CDBG_TV("%s: luma_neg_LUT neg_lut%d in %d out %d doesnt match", __func__,
      2*i+1, in[i].neg_lut1, out[i].neg_lut1);
  }
}/*clf_validate_neg_lut*/

/*===========================================================================
 * FUNCTION    - clf_validate_luma_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_luma_cfg(VFE_CLF_Luma_Cfg *in, VFE_CLF_Luma_Cfg
  *out)
{
  if (!MATCH(in->cutoff_1, out->cutoff_1, 0))
    CDBG_TV("%s: lumaCfg cutoff_1 in %d out %d doesnt match", __func__,
    in->cutoff_1, out->cutoff_1);

  if (!MATCH(in->cutoff_2, out->cutoff_2, 0))
    CDBG_TV("%s: lumaCfg cutoff_2 in %d out %d doesnt match", __func__,
    in->cutoff_2, out->cutoff_2);

  if (!MATCH(in->cutoff_3, out->cutoff_3, 0))
    CDBG_TV("%s: lumaCfg cutoff_3 in %d out %d doesnt match", __func__,
    in->cutoff_3, out->cutoff_3);

  if (!MATCH(in->mult_neg, out->mult_neg, 2))
    CDBG_TV("%s: lumaCfg mult_neg in %d out %d doesnt match", __func__,
    in->mult_neg, out->mult_neg);

  if (!MATCH(in->mult_pos, out->mult_pos, 2))
    CDBG_TV("%s: lumaCfg mult_pos in %d out %d doesnt match", __func__,
    in->mult_pos, out->mult_pos);
}/*clf_validate_luma_cfg*/

/*===========================================================================
 * FUNCTION    - clf_validate_chroma_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_chroma_cfg(VFE_CLF_Chroma_Coeff *in, VFE_CLF_Chroma_Coeff
  *out)
{
  if (!MATCH(in->h_coeff0, out->h_coeff0, 0))
    CDBG_TV("%s: chromaCfg h_coeff0 in %d out %d doesnt match", __func__,
    in->h_coeff0, out->h_coeff0);

  if (!MATCH(in->h_coeff1, out->h_coeff1, 0))
    CDBG_TV("%s: chromaCfg h_coeff1 in %d out %d doesnt match", __func__,
    in->h_coeff1, out->h_coeff1);

  if (!MATCH(in->h_coeff2, out->h_coeff2, 0))
    CDBG_TV("%s: chromaCfg h_coeff2 in %d out %d doesnt match", __func__,
    in->h_coeff2, out->h_coeff2);

  if (!MATCH(in->h_coeff3, out->h_coeff3, 0))
    CDBG_TV("%s: chromaCfg h_coeff3 in %d out %d doesnt match", __func__,
    in->h_coeff3, out->h_coeff3);

  if (!MATCH(in->v_coeff0, out->v_coeff0, 0))
    CDBG_TV("%s: chromaCfg v_coeff0 in %d out %d doesnt match", __func__,
    in->v_coeff0, out->v_coeff0);

  if (!MATCH(in->v_coeff1, out->v_coeff1, 0))
    CDBG_TV("%s: chromaCfg v_coeff1 in %d out %d doesnt match", __func__,
    in->v_coeff1, out->v_coeff1);

}/*clf_validate_chroma_cfg*/

/*===========================================================================
 * FUNCTION    - clf_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_populate_data(uint32_t *reg, VFE_CLF_CmdType *pcmd)
{
  int size = sizeof(VFE_CLF_CmdType) - sizeof(VFE_CLF_Cfg);
  uint8_t *ptr = (uint8_t *)pcmd + sizeof(VFE_CLF_Cfg);
  reg += (CLF_START_REG/4);
  CDBG("%s: size %d", __func__, size);
  memcpy((void *)ptr, (void *)reg, size);
}

/*===========================================================================
 * FUNCTION    - vfe_clf_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_tv_validate(int mod_id, void* test_input,
  void* test_output)
{
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_output;
  VFE_CLF_CmdType in, out;
  clf_populate_data(input->reg_dump, &in);
  clf_populate_data(output->reg_dump_data, &out);

  /*luma*/
  clf_validate_luma_cfg(&in.lumaUpdateCmd.luma_Cfg,
    &out.lumaUpdateCmd.luma_Cfg);
  clf_validate_pos_lut(in.lumaUpdateCmd.luma_pos_LUT,
    out.lumaUpdateCmd.luma_pos_LUT);
  clf_validate_neg_lut(in.lumaUpdateCmd.luma_neg_LUT,
    out.lumaUpdateCmd.luma_neg_LUT);

  /*chroma*/
  clf_validate_chroma_cfg(&in.chromaUpdateCmd.chroma_coeff,
    &out.chromaUpdateCmd.chroma_coeff);

  return VFE_SUCCESS;
}/*vfe_clf_tv_validate*/

/*===========================================================================
 * FUNCTION    - vfe_clf_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_deinit(int mod_id, void *module, void *params)
{
  clf_mod_t *clf_mod = (clf_mod_t *)module;
  memset(clf_mod, 0 , sizeof(clf_mod_t));
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_clf_set_bestshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode)
{
  CDBG_ERROR("%s: Not implemented\n", __func__);
  return VFE_SUCCESS;
}
#ifndef VFE_40
/*===========================================================================
 * FUNCTION    - vfe_clf_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  clf_module_t *cmd = (clf_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->clf_Cmd),
     sizeof(VFE_CLF_CmdType),
     VFE_CMD_CLF_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_clf_plugin_update */
#endif
