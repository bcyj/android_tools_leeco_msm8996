/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_ABF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define ABF_START_REG 0x02a4

/*===========================================================================
 * Function:               vfe_abf_set_params_common
 *
 * Description:
 *=========================================================================*/
void vfe_abf_set_params_common(VFE_DemosaicABF_CmdType* p_cmd,
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

  //TODO : need to check it a[0] same as Q6_A0 for 7x27a
  p_cmd->gCfg.SpatialKernelA0 = ABF2_SP_KERNEL(abf2_data->a[0]);
  p_cmd->gCfg.SpatialKernelA1 = ABF2_SP_KERNEL(abf2_data->a[1]);
} /* vfe_abf_set_params_common */

/*===========================================================================
 * Function:               vfe_abf_set_params
 *
 * Description:
 *=========================================================================*/
int8_t vfe_abf_set_params1(VFE_DemosaicABF_CmdType* p_cmd,
  chromatix_adaptive_bayer_filter_data_type2* abf2_data)
{
  int i = 0;
  uint32_t temp;
  int8_t rc = TRUE;
  vfe_abf_set_params_common(p_cmd, abf2_data);

  for (i=0; i<8; i++) {
    p_cmd->gPosLut[i].PostiveLUT0 =
      ABF2_LUT(abf2_data->scale_factor_green[0]*abf2_data->table_pos[2*i]);
    p_cmd->bPosLut[i].PostiveLUT0 =
      ABF2_LUT(abf2_data->scale_factor_blue[0]*abf2_data->table_pos[2*i]);
    p_cmd->rPosLut[i].PostiveLUT0 =
      ABF2_LUT(abf2_data->scale_factor_red[0]*abf2_data->table_pos[2*i]);
    p_cmd->gPosLut[i].PostiveLUT1 =
      ABF2_LUT(abf2_data->scale_factor_green[0]*abf2_data->table_pos[2*i+1]);
    p_cmd->bPosLut[i].PostiveLUT1 =
      ABF2_LUT(abf2_data->scale_factor_blue[0]*abf2_data->table_pos[2*i+1]);
    p_cmd->rPosLut[i].PostiveLUT1 =
      ABF2_LUT(abf2_data->scale_factor_red[0]*abf2_data->table_pos[2*i+1]);
  }

  for (i=0; i<4; i++) {
    p_cmd->gNegLut[i].NegativeLUT0 =
      ABF2_LUT(abf2_data->scale_factor_green[1]*abf2_data->table_neg[2*i]);
    p_cmd->bNegLut[i].NegativeLUT0 =
      ABF2_LUT(abf2_data->scale_factor_blue[1]*abf2_data->table_neg[2*i]);
    p_cmd->rNegLut[i].NegativeLUT0 =
      ABF2_LUT(abf2_data->scale_factor_red[1]*abf2_data->table_neg[2*i]);
    p_cmd->gNegLut[i].NegativeLUT1 =
      ABF2_LUT(abf2_data->scale_factor_green[1]*abf2_data->table_neg[2*i+1]);
    p_cmd->bNegLut[i].NegativeLUT1 =
      ABF2_LUT(abf2_data->scale_factor_blue[1]*abf2_data->table_neg[2*i+1]);
    p_cmd->rNegLut[i].NegativeLUT1 =
      ABF2_LUT(abf2_data->scale_factor_red[1]*abf2_data->table_neg[2*i+1]);
  }
  return rc;
} /* vfe_abf_set_params1 */

/*===========================================================================
 * Function:               vfe_abf_set_params2
 *
 * Description:
 *=========================================================================*/
int8_t vfe_abf_set_params2(VFE_DemosaicABF_CmdType* p_cmd,
  abf2_parms_t* abf2_parms)
{
  int i = 0;
  int8_t rc = TRUE;
  uint32_t temp;

  if (!abf2_parms->table_updated)
    return vfe_abf_set_params1(p_cmd, &abf2_parms->data);

  vfe_abf_set_params_common(p_cmd, &abf2_parms->data);

  for (i=0; i<8; i++) {
    p_cmd->gPosLut[i].PostiveLUT0 =
      ABF2_LUT(abf2_parms->g_table.table_pos[2*i]);
    p_cmd->bPosLut[i].PostiveLUT0 =
      ABF2_LUT(abf2_parms->b_table.table_pos[2*i]);
    p_cmd->rPosLut[i].PostiveLUT0 =
      ABF2_LUT(abf2_parms->r_table.table_pos[2*i]);
    p_cmd->gPosLut[i].PostiveLUT1 =
      ABF2_LUT(abf2_parms->g_table.table_pos[2*i+1]);
    p_cmd->bPosLut[i].PostiveLUT1 =
      ABF2_LUT(abf2_parms->b_table.table_pos[2*i+1]);
    p_cmd->rPosLut[i].PostiveLUT1 =
      ABF2_LUT(abf2_parms->r_table.table_pos[2*i+1]);
  }

  for (i=0; i<4; i++) {
    p_cmd->gNegLut[i].NegativeLUT0 =
      ABF2_LUT(abf2_parms->g_table.table_neg[2*i]);
    p_cmd->bNegLut[i].NegativeLUT0 =
      ABF2_LUT(abf2_parms->b_table.table_neg[2*i]);
    p_cmd->rNegLut[i].NegativeLUT0 =
      ABF2_LUT(abf2_parms->r_table.table_neg[2*i]);
    p_cmd->gNegLut[i].NegativeLUT1 =
      ABF2_LUT(abf2_parms->g_table.table_neg[2*i+1]);
    p_cmd->bNegLut[i].NegativeLUT1 =
      ABF2_LUT(abf2_parms->b_table.table_neg[2*i+1]);
    p_cmd->rNegLut[i].NegativeLUT1 =
      ABF2_LUT(abf2_parms->r_table.table_neg[2*i+1]);
  }
  return rc;
} /* vfe_abf_set_params2 */

/*===========================================================================
 * Function:               vfe_abf_debug
 *
 * Description:
 *=========================================================================*/
void vfe_abf_debug(VFE_DemosaicABF_CmdType* pCmd)
{
  int i = 0;
  VFE_DemosaicABF_gCfg* gCfg = &(pCmd->gCfg);
  VFE_DemosaicABF_RBCfg* rCfg = &(pCmd->rCfg);
  VFE_DemosaicABF_RBCfg* bCfg = &(pCmd->bCfg);

  VFE_DemosaicABF_Neg_Lut* gNLut = (pCmd->gNegLut);
  VFE_DemosaicABF_Neg_Lut* rNLut = (pCmd->rNegLut);
  VFE_DemosaicABF_Neg_Lut* bNLut = (pCmd->bNegLut);

  VFE_DemosaicABF_Pos_Lut* gPLut = (pCmd->gPosLut);
  VFE_DemosaicABF_Pos_Lut* rPLut = (pCmd->rPosLut);
  VFE_DemosaicABF_Pos_Lut* bPLut = (pCmd->bPosLut);

#ifndef VFE_2X
  CDBG("abf2 enable=%d\n", pCmd->abf_Cfg.enable);
#else
  CDBG("abf2 enable=%d\n", pCmd->gCfg.enable);
#endif

#ifdef VFE_31
  CDBG("forceOn=%d.\n", pCmd->abf_Cfg.forceOn);
#endif

  CDBG("=====Green parametets ===============\n");
  CDBG("abf2 green: cutoff1=0x%x, cutoff2=0x%x,cutoff3=0x%x.\n",
    gCfg->Cutoff1, gCfg->Cutoff2, gCfg->Cutoff3);
  CDBG("abf2 green: multiPositive=0x%x, multiNegative=0x%x.\n",
    gCfg->MultPositive, gCfg->MultNegative);
  CDBG("abf2 green: A0 = 0x%x, A1 = 0x%x.\n",
    gCfg->SpatialKernelA0,gCfg->SpatialKernelA1);

  for (i=0; i<8; i++)
    CDBG("Green PosLUT: coef%d=0x%x,coef%d=0x%x\n",
      2*i, gPLut[i].PostiveLUT0, 2*i+1, gPLut[i].PostiveLUT1);

  for (i=0; i<4; i++)
    CDBG("Green NegLUT: coef%d=0x%x,coef%d=0x%x\n",
      2*i, gNLut[i].NegativeLUT0, 2*i+1, gNLut[i].NegativeLUT1);

  CDBG("=====Red parametets ===============\n");
  CDBG("abf2 red: cutoff1=0x%x, cutoff2=0x%x,cutoff3=0x%x.\n",
    rCfg->Cutoff1, rCfg->Cutoff2, rCfg->Cutoff3);
  CDBG("abf2 red: multiPositive=0x%x, multiNegative=0x%x.\n",
    rCfg->MultPositive, rCfg->MultNegative);

  for (i=0; i<8; i++)
    CDBG("Red PosLUT: coef%d=0x%x,coef%d=0x%x\n",
      2*i, rPLut[i].PostiveLUT0, 2*i+1, rPLut[i].PostiveLUT1);

  for (i=0; i<4; i++)
    CDBG("Red NegLUT: coef%d=0x%x,coef%d=0x%x\n",
      2*i, rNLut[i].NegativeLUT0, 2*i+1, rNLut[i].NegativeLUT1);

  CDBG("=====Blue parametets ===============\n");
  CDBG("abf2 blue: cutoff1=0x%x, cutoff2=0x%x,cutoff3=0x%x.\n",
    bCfg->Cutoff1, bCfg->Cutoff2, bCfg->Cutoff3);
  CDBG("abf2 blue: multiPositive=0x%x, multiNegative=0x%x.\n",
    bCfg->MultPositive, bCfg->MultNegative);

  for (i=0; i<8; i++)
    CDBG("Blue PosLUT: coef%d=0x%x,coef%d=0x%x\n",
      2*i, bPLut[i].PostiveLUT0, 2*i+1, bPLut[i].PostiveLUT1);

  for (i=0; i<4; i++)
    CDBG("Blue NegLUT: coef%d=0x%x,coef%d=0x%x\n",
      2*i, bNLut[i].NegativeLUT0, 2*i+1, bNLut[i].NegativeLUT1);
} /* vfe_abf_debug */

/*===========================================================================
 * Function:               vfe_abf_update
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_abf_update(int mod_id, void *abf_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  abf_mod_t *mod = (abf_mod_t *)abf_mod;
  vfe_params_t* parms = (vfe_params_t *)vparms;
  abf2_parms_t *abf2_parms = &(mod->abf2_parms);
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;
  int is_snap = IS_SNAP_MODE(parms);
  VFE_DemosaicABF_CmdType* p_cmd = (is_snap) ?
    &mod->VFE_SnapshotDemosaicABFCfgCmd :
    &mod->VFE_DemosaicABFCfgCmd;

  CDBG("%s: update %d enable %d\n", __func__, mod->abf2_update,
    mod->enable);
  if (!mod->abf2_update) {
    CDBG("%s: ABF not updated", __func__);
    return VFE_SUCCESS;
  }

  vfe_abf_set_params2(p_cmd, abf2_parms);

  CDBG("Update %s config", is_snap ? "Snapshot" : "Preview");
  vfe_abf_debug(p_cmd);

  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) p_cmd,
    sizeof(mod->VFE_DemosaicABFCfgCmd), VFE_CMD_DEMOSAICV3_ABF_UPDATE);

  if (status == VFE_SUCCESS) {
    parms->update |= VFE_MOD_ABF;
    mod->abf2_update = FALSE;
  }

  return status;
} /* vfe_abf_update */

/*===========================================================================
 * FUNCTION    - vfe_abf_video_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_video_config(int mod_id, void *abf_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  abf_mod_t *mod = (abf_mod_t *) abf_mod;
  vfe_params_t* parms = (vfe_params_t*)vparms;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;

  if (!mod->enable) {
    CDBG("%s: ABF not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("Video ABF Config %d", mod->abf2_update);

  vfe_abf_debug(&(mod->VFE_DemosaicABFCfgCmd));
  vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) &(mod->VFE_DemosaicABFCfgCmd),
    sizeof(mod->VFE_DemosaicABFCfgCmd), VFE_CMD_DEMOSAICV3_ABF_CFG);

  if (status == VFE_SUCCESS)
    mod->abf2_update = FALSE;

  return status;
} /* vfe_abf_video_config */

/*===========================================================================
 * FUNCTION    - vfe_abf_snapshot_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_snapshot_config(abf_mod_t *mod, vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;

  CDBG("Snapshot Config ABF2.\n");
  chromatix_adaptive_bayer_filter_data_type2 *abf2_data;
  chromatix_parms_type *chroma_ptr = parms->chroma3a;

  if (!mod->enable) {
    CDBG("%s: ABF not enabled", __func__);
    return VFE_SUCCESS;
  }
  abf2_data = &(chroma_ptr->abf2_config_normal_light_snapshot);

  CDBG("Snapshot ABF Config %d", mod->abf2_update);
  if (mod->abf2_update)
    vfe_abf_set_params2(&mod->VFE_SnapshotDemosaicABFCfgCmd, &mod->abf2_parms);
  else
    vfe_abf_set_params1(&mod->VFE_SnapshotDemosaicABFCfgCmd, abf2_data);

  vfe_abf_debug(&(mod->VFE_SnapshotDemosaicABFCfgCmd));
  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) &(mod->VFE_SnapshotDemosaicABFCfgCmd),
    sizeof(mod->VFE_SnapshotDemosaicABFCfgCmd),
    VFE_CMD_DEMOSAICV3_ABF_CFG);

  if (status == VFE_SUCCESS)
    mod->abf2_update = FALSE;

  return status;
} /* vfe_abf_snapshot_config */

/*===========================================================================
 * FUNCTION    - vfe_abf_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_config(int mod_id, void *abf_mod, void *vparams)
{
  abf_mod_t *mod = (abf_mod_t *) abf_mod;
  vfe_params_t* params = (vfe_params_t*)vparams;
  int8_t is_snap = IS_SNAP_MODE(params);

  return is_snap ? vfe_abf_snapshot_config(mod, params) :
  vfe_abf_video_config(mod_id, mod, params);
} /* vfe_abf_config */

/*===========================================================================
 * FUNCTION    - vfe_abf_interpolate -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_abf_interpolate(chromatix_adaptive_bayer_filter_data_type2* pv1,
  chromatix_adaptive_bayer_filter_data_type2* pv2,
  abf2_parms_t* pv_out, float ratio)
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
} /* vfe_abf_interpolate */

/*===========================================================================
 * FUNCTION    - vfe_abf_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_trigger_update(int mod_id, void *abf_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  trigger_point_type      *outdoor, *lowlight;
  trigger_ratio_t         trigger_ratio;
  int i = 0;
  abf_mod_t *mod = (abf_mod_t *) abf_mod;
  vfe_params_t* parms = (vfe_params_t*)vparms;
  chromatix_parms_type *chroma_ptr = parms->chroma3a;

  int is_snap = IS_SNAP_MODE(parms);
  abf2_parms_t *abf2_parms = &(mod->abf2_parms);
  chromatix_adaptive_bayer_filter_data_type2 *p1 = NULL, *p2 = NULL,
    *abf2_normal, *abf2_outdoor, *abf2_lowlight;
  tuning_control_type tunning_control = chroma_ptr->control_ABF;

  if (!mod->enable) {
    CDBG("%s: ABF not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->trigger_enable) {
    CDBG("%s: ABF trigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!vfe_util_aec_check_settled(&parms->aec_params)) {
    CDBG("%s: AEC not settled", __func__);
    return status;
  }

  if (!is_snap) {
    outdoor = &(chroma_ptr->abf2_bright_light_trigger_preview);
    lowlight = &(chroma_ptr->abf2_low_light_trigger_preview);
    abf2_normal = &(chroma_ptr->abf2_config_normal_light_preview);
    abf2_outdoor = &(chroma_ptr->abf2_config_bright_light_preview);
    abf2_lowlight = &(chroma_ptr->abf2_config_low_light_preview);
  } else {
    outdoor = &(chroma_ptr->abf2_bright_light_trigger_snapshot);
    lowlight = &(chroma_ptr->abf2_low_light_trigger_snapshot);
    abf2_normal = &(chroma_ptr->abf2_config_normal_light_snapshot);
    abf2_outdoor = &(chroma_ptr->abf2_config_bright_light_snapshot);
    abf2_lowlight = &(chroma_ptr->abf2_config_low_light_snapshot);
  }

  /* Decide the trigger ratio for current lighting condition */
  trigger_ratio = vfe_util_get_aec_ratio2(tunning_control,
    outdoor, lowlight, parms);
  CDBG("In VFE trigger ABF2 trigger_ratio: lighting = %d, ratio = %f.\n",
    trigger_ratio.lighting, trigger_ratio.ratio);

  /* this condition is:  if first time, or either ratio/lighting
     != previous ones, then we need update. */
  CDBG("In VFE trigger abf2 old ratio: lighting = %d, ratio = %f.\n",
    mod->abf2_ratio.lighting, mod->abf2_ratio.ratio);

  switch (trigger_ratio.lighting) {
    case TRIGGER_LOWLIGHT:
      p1 = abf2_lowlight;
      p2 = abf2_normal;
      break;
    case TRIGGER_OUTDOOR:
      p1 = abf2_outdoor;
      p2 = abf2_normal;
      break;
    default:
    case TRIGGER_NORMAL:
      p1 = p2 = abf2_normal;
      break;
  }

  mod->abf2_update = (mod->cur_mode != parms->vfe_op_mode) ||
    (trigger_ratio.lighting != mod->abf2_ratio.lighting) ||
    !F_EQUAL(trigger_ratio.ratio, mod->abf2_ratio.ratio) ||
    mod->reload_params;

  if (mod->abf2_update) {
    /* Update abf2 */
    mod->cur_mode = parms->vfe_op_mode;
    mod->abf2_ratio = trigger_ratio;
    abf2_parms->table_updated = FALSE;
    if (!F_EQUAL(trigger_ratio.ratio, 0.0) &&
      !F_EQUAL(trigger_ratio.ratio, 1.0))
      vfe_abf_interpolate(p2, p1, abf2_parms, trigger_ratio.ratio);
    else
      memcpy(&abf2_parms->data, p1,
        sizeof(chromatix_adaptive_bayer_filter_data_type2));
  }
  CDBG("In vfe trigger ABF2. Need ABF2 update = %d.\n",
    mod->abf2_update);
  return status;
} /* vfe_abf_trigger_update */


/*===========================================================================
 * FUNCTION    - vfe_abf_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_init(int mod_id, void *abf_mod, void *vparms)
{
  chromatix_adaptive_bayer_filter_data_type2 *abf2_data = NULL;
    CDBG("%s: enter", __func__);
  abf_mod_t *mod = (abf_mod_t *) abf_mod;
  vfe_params_t* parms = (vfe_params_t*)vparms;
  chromatix_parms_type *chroma_ptr = parms->chroma3a;

  memset(&mod->abf2_parms, 0x0, sizeof(abf2_parms_t));
  mod->cur_mode = VFE_OP_MODE_INVALID;
  mod->trigger_enable = TRUE;
  abf2_data = &(chroma_ptr->abf2_config_normal_light_preview);
  vfe_abf_set_params1(&mod->VFE_DemosaicABFCfgCmd, abf2_data);
  abf2_data = &(chroma_ptr->abf2_config_normal_light_snapshot);
  vfe_abf_set_params1(&mod->VFE_SnapshotDemosaicABFCfgCmd, abf2_data);
  return VFE_SUCCESS;
} /* vfe_abf_init */

/*===========================================================================
 * Function:           vfe_abf_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_abf_enable(int mod_id, void *abf_mod, void *vparams,
  int8_t enable, int8_t hw_write)
{
  abf_mod_t *abf_ctrl = (abf_mod_t *) abf_mod;
  vfe_params_t* params = (vfe_params_t*)vparams;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  int8_t is_snap = IS_SNAP_MODE(params);
  VFE_DemosaicABF_CmdType* p_cmd = (is_snap) ?
    &abf_ctrl->VFE_SnapshotDemosaicABFCfgCmd :
    &abf_ctrl->VFE_DemosaicABFCfgCmd;
#ifndef VFE_2X
  p_cmd->abf_Cfg.enable = enable;
#else
  p_cmd->gCfg.enable = enable;
#endif

  if (hw_write && (abf_ctrl->enable == enable))
    return VFE_SUCCESS;

  abf_ctrl->enable = enable;
  if (hw_write) {
    abf_ctrl->abf2_update = TRUE;
    params->current_config = (enable) ? (params->current_config | VFE_MOD_ABF)
      : (params->current_config & ~VFE_MOD_ABF);
  }
  return VFE_SUCCESS;
} /* vfe_abf_set */

/*===========================================================================
 * FUNCTION    - vfe_abf_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_trigger_enable(int mod_id, void* abf_mod, void* vparams,
  int enable)
{
  abf_mod_t *mod = (abf_mod_t *) abf_mod;
  vfe_params_t* params = (vfe_params_t*)vparams;

  CDBG("%s: %d", __func__, enable);
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_abf_trigger_enable*/

/*===========================================================================
 * FUNCTION    - vfe_abf_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_reload_params(int mod_id, void *abf_mod, void *vparms)
{
  abf_mod_t *mod = (abf_mod_t *) abf_mod;
  vfe_params_t* parms = (vfe_params_t*) vparms;
  chromatix_adaptive_bayer_filter_data_type2 *abf2_data = NULL;

  CDBG("%s:", __func__);
  chromatix_parms_type *chroma_ptr = parms->chroma3a;
  abf2_data = &(chroma_ptr->abf2_config_normal_light_preview);
  vfe_abf_set_params1(&mod->VFE_DemosaicABFCfgCmd, abf2_data);
  abf2_data = &(chroma_ptr->abf2_config_normal_light_snapshot);
  vfe_abf_set_params1(&mod->VFE_SnapshotDemosaicABFCfgCmd, abf2_data);
  mod->reload_params = TRUE;
  return VFE_SUCCESS;
} /*vfe_abf_reload_params*/

/*===========================================================================
 * FUNCTION    - abf_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_populate_data(uint32_t *reg, VFE_DemosaicABF_CmdType *pcmd)
{
  int size = sizeof(VFE_DemosaicABF_CmdType) - sizeof(VFE_DemosaicABF_Cfg);
  uint8_t *ptr = (uint8_t *)pcmd + sizeof(VFE_DemosaicABF_Cfg);
  reg += (ABF_START_REG/4);
  memcpy((void *)ptr, (void *)reg, size);
}/*abf_populate_data*/

/*===========================================================================
 * FUNCTION    - abf_validate_g_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_g_cfg(VFE_DemosaicABF_gCfg *in, VFE_DemosaicABF_gCfg
  *out)
{
  if (!MATCH(in->Cutoff1, out->Cutoff1, 0))
    CDBG_TV("%s: gCfg Cutoff1 in %d out %d doesnt match", __func__,
    in->Cutoff1, out->Cutoff1);

  if (!MATCH(in->Cutoff2, out->Cutoff2, 0))
    CDBG_TV("%s: gCfg Cutoff2 in %d out %d doesnt match", __func__,
    in->Cutoff2, out->Cutoff2);

  if (!MATCH(in->Cutoff3, out->Cutoff3, 0))
    CDBG_TV("%s: gCfg Cutoff3 in %d out %d doesnt match", __func__,
    in->Cutoff3, out->Cutoff3);

  if (!MATCH(in->SpatialKernelA0, out->SpatialKernelA0, 0))
    CDBG_TV("%s: gCfg SpatialKernelA0 in %d out %d doesnt match", __func__,
    in->SpatialKernelA0, out->SpatialKernelA0);

  if (!MATCH(in->SpatialKernelA1, out->SpatialKernelA1, 0))
    CDBG_TV("%s: gCfg SpatialKernelA1 in %d out %d doesnt match", __func__,
    in->SpatialKernelA1, out->SpatialKernelA1);

  if (!MATCH(in->MultNegative, out->MultNegative, 2))
    CDBG_TV("%s: gCfg MultNegative in %d out %d doesnt match", __func__,
    in->MultNegative, out->MultNegative);

  if (!MATCH(in->MultPositive, out->MultPositive, 2))
    CDBG_TV("%s: gCfg MultPositive in %d out %d doesnt match", __func__,
    in->MultPositive, out->MultPositive);
}/*abf_validate_g_cfg*/

/*===========================================================================
 * FUNCTION    - abf_validate_rb_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_pos_lut(VFE_DemosaicABF_Pos_Lut *in, VFE_DemosaicABF_Pos_Lut
  *out, char* channel)
{
  int i=0;
  for (i=0; i<8; i++) {
    if (!MATCH(in[i].PostiveLUT0, out[i].PostiveLUT0, 0))
      CDBG_TV("%s: %s PostiveLUT%d in %d out %d doesnt match", __func__,
      channel, 2*i, in[i].PostiveLUT0, out[i].PostiveLUT0);

    if (!MATCH(in[i].PostiveLUT1, out[i].PostiveLUT1, 0))
      CDBG_TV("%s: %s PostiveLUT%d in %d out %d doesnt match", __func__,
      channel, 2*i+1, in[i].PostiveLUT1, out[i].PostiveLUT1);
  }
}/*abf_validate_rb_cfg*/

/*===========================================================================
 * FUNCTION    - abf_validate_neg_lut -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_neg_lut(VFE_DemosaicABF_Neg_Lut *in, VFE_DemosaicABF_Neg_Lut
  *out, char* channel)
{
  int i=0;
  for (i=0; i<4; i++) {
    if (!MATCH(in[i].NegativeLUT0, out[i].NegativeLUT0, 0))
      CDBG_TV("%s: %s NegativeLUT%d in %d out %d doesnt match", __func__,
      channel, 2*i, in[i].NegativeLUT0, out[i].NegativeLUT0);

    if (!MATCH(in[i].NegativeLUT1, out[i].NegativeLUT1, 0))
      CDBG_TV("%s: %s NegativeLUT%d in %d out %d doesnt match", __func__,
      channel, 2*i+1, in[i].NegativeLUT1, out[i].NegativeLUT1);
  }
}/*abf_validate_neg_lut*/

/*===========================================================================
 * FUNCTION    - abf_validate_rb_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_rb_cfg(VFE_DemosaicABF_RBCfg *in, VFE_DemosaicABF_RBCfg
  *out, const char* channel)
{
  if (!MATCH(in->Cutoff1, out->Cutoff1, 0))
    CDBG_TV("%s: %s Cutoff1 in %d out %d doesnt match", __func__, channel,
    in->Cutoff1, out->Cutoff1);

  if (!MATCH(in->Cutoff2, out->Cutoff2, 0))
    CDBG_TV("%s: %s Cutoff2 in %d out %d doesnt match", __func__, channel,
    in->Cutoff2, out->Cutoff2);

  if (!MATCH(in->Cutoff3, out->Cutoff3, 0))
    CDBG_TV("%s: %s Cutoff3 in %d out %d doesnt match", __func__, channel,
    in->Cutoff3, out->Cutoff3);

  if (!MATCH(in->MultNegative, out->MultNegative, 2))
    CDBG_TV("%s: %s MultNegative in %d out %d doesnt match", __func__,
    channel, in->MultNegative, out->MultNegative);

  if (!MATCH(in->MultPositive, out->MultPositive, 2))
    CDBG_TV("%s: %s MultPositive in %d out %d doesnt match", __func__,
    channel, in->MultPositive, out->MultPositive);
}/*abf_validate_rb_cfg*/

/*===========================================================================
 * FUNCTION    - vfe_abf_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_tv_validate(int mod_id, void *test_input,
  void *test_output)
{
  VFE_DemosaicABF_CmdType in, out;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *) test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;
  abf_populate_data(input->reg_dump, &in);
  abf_populate_data(output->reg_dump_data, &out);

  /*green*/
  abf_validate_g_cfg(&in.gCfg, &out.gCfg);
  abf_validate_pos_lut(in.gPosLut, out.gPosLut, "gPosLut");
  abf_validate_neg_lut(in.gNegLut, out.gNegLut, "gNegLut");

  /*blue*/
  abf_validate_rb_cfg(&in.bCfg, &out.bCfg, "bCfg");
  abf_validate_pos_lut(in.bPosLut, out.bPosLut, "bPosLut");
  abf_validate_neg_lut(in.bNegLut, out.bNegLut, "bNegLut");

  /*red*/
  abf_validate_rb_cfg(&in.rCfg, &out.rCfg, "rCfg");
  abf_validate_pos_lut(in.rPosLut, out.rPosLut, "rPosLut");
  abf_validate_neg_lut(in.rNegLut, out.rNegLut, "rNegLut");

  return VFE_SUCCESS;
}/*vfe_abf_tv_validate*/

/*=============================================================================
 * Function:               vfe_abf_deinit
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_abf_deinit(int mod_id, void *abf_mod, void *vfe_params)
{
  return VFE_SUCCESS;
} /* vfe_abf_deinit */
#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_abf_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  abf_module_t *cmd = (abf_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&(cmd->demosaicABFCfgCmd), sizeof(VFE_DemosaicABF_CmdType),
    VFE_CMD_DEMOSAICV3_ABF_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_abf_plugin_update */
#endif
