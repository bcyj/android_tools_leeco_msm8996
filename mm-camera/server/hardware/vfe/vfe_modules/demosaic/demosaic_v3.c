
/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_DEMOSAIC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

static uint16_t wInterpDefault[VFE_DEMOSAICV3_CLASSIFIER_CNT] =
    { 137, 91, 91, 1023, 922, 93, 195, 99, 64, 319, 197, 88, 84, 109, 151, 98,
       66, 76 };
static uint16_t bInterpDefault[VFE_DEMOSAICV3_CLASSIFIER_CNT] =
    { 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1 };
static uint16_t lInterpDefault[VFE_DEMOSAICV3_CLASSIFIER_CNT] =
    { 0, 0, 1, 2, 2, 3, 9, 9, 9, 4, 4, 5, 6, 7, 8, 8, 10, 10 };
static int16_t tInterpDefault[VFE_DEMOSAICV3_CLASSIFIER_CNT] =
    { 2, 1, 0, 0, -1, 2, 0, -1, 1, 0, -1, 2, 0, 2, 2, 1, 0, 100 };


/*===========================================================================
 * FUNCTION    - vfe_demosaic_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_demosaic_debug(void *cmd, int8_t update)
{
#ifndef VFE_2X
  CDBG("VFE_DemosaicV3 config = %d or update = %d",
    !update, update);

  if (update) {
    VFE_DemosaicV3UpdateCmdType* pcmd =
      (VFE_DemosaicV3UpdateCmdType *)cmd;
	CDBG("VFE_DemosaicV3ConfigCmdType cositedRgbEnable %d",
	  pcmd->cositedRgbEnable);
	CDBG("VFE_DemosaicV3CmdType pipeFlushCount %d", pcmd->pipeFlushCount);
	CDBG("VFE_DemosaicV3CmdType pipeFlushOvd %d", pcmd->pipeFlushOvd);
	CDBG("VFE_DemosaicV3CmdType flushHaltOvd %d", pcmd->flushHaltOvd);
	CDBG("VFE_DemosaicV3CmdType rgWbGain %d", pcmd->rgWbGain);
	CDBG("VFE_DemosaicV3CmdType bgWbGain %d", pcmd->bgWbGain);
	CDBG("VFE_DemosaicV3CmdType grWbGain %d", pcmd->grWbGain);
	CDBG("VFE_DemosaicV3CmdType gbWbGain %d", pcmd->gbWbGain);
	CDBG("VFE_DemosaicV3CmdType bl %d", pcmd->bl);
	CDBG("VFE_DemosaicV3CmdType bu %d", pcmd->bu);
	CDBG("VFE_DemosaicV3CmdType dblu %d", pcmd->dblu);
	CDBG("VFE_DemosaicV3CmdType a %d", pcmd->a);
  } else {
    VFE_DemosaicV3ConfigCmdType* pcmd =
      (VFE_DemosaicV3ConfigCmdType *)cmd;
	CDBG("VFE_DemosaicV3ConfigCmdType cositedRgbEnable %d",
	  pcmd->cositedRgbEnable);
	CDBG("VFE_DemosaicV3CmdType pipeFlushCount %d", pcmd->pipeFlushCount);
	CDBG("VFE_DemosaicV3CmdType pipeFlushOvd %d", pcmd->pipeFlushOvd);
	CDBG("VFE_DemosaicV3CmdType flushHaltOvd %d", pcmd->flushHaltOvd);
	CDBG("VFE_DemosaicV3CmdType rgWbGain %d", pcmd->rgWbGain);
	CDBG("VFE_DemosaicV3CmdType bgWbGain %d", pcmd->bgWbGain);
	CDBG("VFE_DemosaicV3CmdType grWbGain %d", pcmd->grWbGain);
	CDBG("VFE_DemosaicV3CmdType gbWbGain %d", pcmd->gbWbGain);
	CDBG("VFE_DemosaicV3CmdType bl %d", pcmd->bl);
	CDBG("VFE_DemosaicV3CmdType bu %d", pcmd->bu);
	CDBG("VFE_DemosaicV3CmdType dblu %d", pcmd->dblu);
	CDBG("VFE_DemosaicV3CmdType a %d", pcmd->a);
  }
#else
  VFE_DemosaicV3ConfigCmdType* pcmd =
      (VFE_DemosaicV3ConfigCmdType *)cmd;
  CDBG("VFE_DemosaicV3ConfigCmdType cositedRgbEnable %d",
    pcmd->cositedRgbEnable);
  CDBG("VFE_DemosaicV3CmdType rgWbGain %d", pcmd->rgWbGain);
  CDBG("VFE_DemosaicV3CmdType bgWbGain %d", pcmd->bgWbGain);
  CDBG("VFE_DemosaicV3CmdType grWbGain %d", pcmd->grWbGain);
  CDBG("VFE_DemosaicV3CmdType gbWbGain %d", pcmd->gbWbGain);
  CDBG("VFE_DemosaicV3CmdType bl %d", pcmd->bl);
  CDBG("VFE_DemosaicV3CmdType bu %d", pcmd->bu);
  CDBG("VFE_DemosaicV3CmdType dblu %d", pcmd->dblu);
  CDBG("VFE_DemosaicV3CmdType a %d", pcmd->a);
#endif
}/*vfe_demosaic_debug*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_set_params(demosaic_mod_t *mod, vfe_params_t *params,
  int is_snap)
{
  int i;
  int temp;
  chromatix_parms_type *chromatix_ptr = params->chroma3a;
  demosaic3_LUT_type *classifier = NULL;
  VFE_DemosaicV3ConfigCmdType *p_cmd = NULL;
  float bL = 0.0;
  float aG = 0.0;
  if (!is_snap) {
    classifier = &(chromatix_ptr->demosaic3_LUT_preview);
    bL = chromatix_ptr->demosaic3_bL_preview[1];
    aG = chromatix_ptr->demosaic3_aG_preview[1];
    p_cmd = &mod->demosaic_cmd;
  } else {
    classifier = &(chromatix_ptr->demosaic3_LUT_snapshot);
    bL = chromatix_ptr->demosaic3_bL_snapshot[1];
    aG = chromatix_ptr->demosaic3_aG_snapshot[1];
    p_cmd = &mod->demosaic_snapshot_cmd;
  }

  /* default values */
#ifndef VFE_2X
  p_cmd->dbpcEnable = FALSE;
  p_cmd->dbccEnable = FALSE;
  p_cmd->abccEnable = FALSE;
  p_cmd->abfEnable = FALSE;
  p_cmd->abccLutBankSel = 0;
  p_cmd->pipeFlushCount = 0;
  p_cmd->pipeFlushOvd = 0;
  p_cmd->flushHaltOvd = 0;
#endif
  p_cmd->cositedRgbEnable = FALSE;

  /* WB gain */
  p_cmd->rgWbGain = FLOAT_TO_Q(7,
    (params->awb_params.gain.r_gain/params->awb_params.gain.g_gain));
  p_cmd->bgWbGain = FLOAT_TO_Q(7,
    (params->awb_params.gain.b_gain/params->awb_params.gain.g_gain));
  p_cmd->grWbGain = FLOAT_TO_Q(7,
    (params->awb_params.gain.g_gain/params->awb_params.gain.r_gain));
  p_cmd->gbWbGain = FLOAT_TO_Q(7,
    (params->awb_params.gain.g_gain/params->awb_params.gain.b_gain));

  /* Classifier */
  for (i=0 ; i<VFE_DEMOSAICV3_CLASSIFIER_CNT ; ++i) {
    p_cmd->interpClassifier[i].w_n = FLOAT_TO_Q(10,classifier->wk[i]);
    p_cmd->interpClassifier[i].t_n = classifier->Tk[i];
    p_cmd->interpClassifier[i].l_n = classifier->lk[i];
    p_cmd->interpClassifier[i].b_n = classifier->bk[i];
  }

  /* Interp G */
  temp = FLOAT_TO_Q(8, bL);
  CDBG("%s: bl %d", __func__, temp);
  p_cmd->bl = MIN(MAX(0, temp), 118);
  temp = FLOAT_TO_Q(8, (1.0-bL));
  p_cmd->bu = MIN(MAX(138, temp), 255);
  temp = FLOAT_TO_Q(5, (1.0/(1.0-2*bL)));
  p_cmd->dblu = MIN(MAX(0, temp), 511);
  temp = FLOAT_TO_Q(6, aG);
  p_cmd->a = MIN(MAX(0, temp), 63);
  return VFE_SUCCESS;
} /* vfe_demosaic_set_params */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_init(int mod_id, void *module, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  mod->trigger_enable = TRUE;
  status = vfe_demosaic_set_params(mod, params, FALSE);
  if (VFE_SUCCESS != status)
    return status;

  status = vfe_demosaic_set_params(mod, params, TRUE);
  return status;
} /* vfe_demosaic_init */

/*===========================================================================
 * FUNCTION.   - vfe_demosaic_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_demosaic_enable(int mod_id, void *module, void *vparams,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  CDBG("%s, enable/disable demosaic v3 module = %d",__func__, enable);
  params->moduleCfg->demosaicEnable = enable;

  if (hw_write && (mod->enable == enable))
    return VFE_SUCCESS;

  mod->enable = enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_DEMOSAIC)
      : (params->current_config & ~VFE_MOD_DEMOSAIC);
  }
  return status;
} /* vfe_demosaic_enable */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_config(int mod_id, void *module, void *vparams)
{
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  int is_snap = IS_SNAP_MODE(params);
  VFE_DemosaicV3ConfigCmdType *pcmd = is_snap ? &mod->demosaic_snapshot_cmd :
    &mod->demosaic_cmd;

  CDBG("%s: %s", __func__, (is_snap) ? "Snapshot" : "Preview");
  vfe_demosaic_debug(pcmd, FALSE);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(params->camfd,CMD_GENERAL,
    (void *) pcmd, sizeof(VFE_DemosaicV3ConfigCmdType),
    VFE_CMD_DEMOSAICV3)) {
    CDBG_HIGH("%s: demosaic config for operation mode = %d failed\n", __func__,
      params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  } else
    mod->update = FALSE;

  params->awb_gain_update = TRUE;
  return VFE_SUCCESS;
} /* vfe_demosaic_config */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_update(int mod_id, void *module, void *vparams)
{
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;
  int is_snap = IS_SNAP_MODE(params);
  VFE_DemosaicV3UpdateCmdType *pcmd = is_snap ? &mod->demosaic_snap_up_cmd :
    &mod->demosaic_vf_up_cmd;

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
    params->update |= VFE_MOD_DEMOSAIC;
    mod->hw_enable = FALSE;
  }
#endif

#ifdef VFE_2X
  pcmd = is_snap ? &mod->demosaic_snap_up_cmd : &mod->demosaic_cmd;
#endif
  if (!mod->update) {
    CDBG("%s: No update required", __func__);
    return status;
  }
  CDBG("%s: %s", __func__, (is_snap) ? "Snapshot" : "Preview");
  vfe_demosaic_debug(pcmd, TRUE);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    (void *) pcmd, sizeof(VFE_DemosaicV3UpdateCmdType),
    VFE_CMD_DEMOSAICV3_UPDATE);
  CDBG("%s: demosaic update %d\n", __func__, status);

  if (VFE_SUCCESS == status) {
    mod->update = FALSE;
    params->update |= VFE_MOD_DEMOSAIC;
  }
  return status;
} /* vfe_demosaic_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_trigger_update(int mod_id, void *module,
  void *vparams)
{
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  float ratio = 0.0, aG = 0.0, bL = 0.0, aG_lowlight = 0.0, bL_lowlight = 0.0;
  tuning_control_type *tc = NULL;
  trigger_point_type  *tp = NULL;
  VFE_DemosaicV3UpdateCmdType *p_cmd = NULL;
  vfe_status_t status = VFE_SUCCESS;
  int temp;

  vfe_aec_parms_t *aec_obj = &(params->aec_params);
  int is_snap = IS_SNAP_MODE(params);
  mod->update = FALSE;

  if (!mod->trigger_enable) {
    CDBG("%s: Demosaic trigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (is_snap) {
    tc = &(params->chroma3a->control_demosaic3_snapshot);
    tp = &(params->chroma3a->demosaic3_trigger_lowlight_snapshot);
    aG = params->chroma3a->demosaic3_aG_snapshot[1];
    bL = params->chroma3a->demosaic3_bL_snapshot[1];
    aG_lowlight = params->chroma3a->demosaic3_aG_snapshot[0];
    bL_lowlight = params->chroma3a->demosaic3_bL_snapshot[0];
    p_cmd = &mod->demosaic_snap_up_cmd;
  } else {
    /* preview */
    if (!vfe_util_aec_check_settled(aec_obj))
      return VFE_SUCCESS;

    tc = &(params->chroma3a->control_demosaic3_preview);
    tp = &(params->chroma3a->demosaic3_trigger_lowlight_preview);
    aG = params->chroma3a->demosaic3_aG_preview[1];
    bL = params->chroma3a->demosaic3_bL_preview[1];
    aG_lowlight = params->chroma3a->demosaic3_aG_preview[0];
    bL_lowlight = params->chroma3a->demosaic3_bL_preview[0];
    p_cmd = &mod->demosaic_vf_up_cmd;
    //todo: need to check again
#ifdef VFE_2X
    p_cmd = &mod->demosaic_cmd;
#endif
  }

  ratio = vfe_util_get_aec_ratio(*tc, tp, params);
  CDBG("%s: ratio %f", __func__, ratio);

  if (mod->cur_mode != params->vfe_op_mode || !F_EQUAL(ratio, mod->ratio)
    || mod->reload_params) {
    float new_aG, new_bL;

    mod->cur_mode = params->vfe_op_mode;

    new_aG = LINEAR_INTERPOLATION(aG, aG_lowlight, ratio);
    new_bL = LINEAR_INTERPOLATION(bL, bL_lowlight, ratio);
    temp = FLOAT_TO_Q(8, new_bL);
    CDBG("%s: bl %d", __func__, temp);
    p_cmd->bl = MIN(MAX(0, temp), 118);
    temp = FLOAT_TO_Q(8, (1.0-new_bL));
    p_cmd->bu = MIN(MAX(138, temp), 255);
    temp = FLOAT_TO_Q(5, (1.0/(1.0-2*new_bL)));
    p_cmd->dblu = MIN(MAX(0, temp), 511);
    temp = FLOAT_TO_Q(6, new_aG);
    p_cmd->a = MIN(MAX(0, temp), 63);
    mod->update = TRUE;
    mod->ratio = ratio;
    mod->reload_params = FALSE;
  }

  /* update wb gains */
  CDBG("%s: awb update %d", __func__, params->awb_gain_update);
  if (params->awb_gain_update) {
    CDBG("%s: gains r %f g %f b %f", __func__,
      params->awb_params.gain.r_gain,
      params->awb_params.gain.g_gain,
      params->awb_params.gain.b_gain);
    p_cmd->rgWbGain = FLOAT_TO_Q(7,
      (params->awb_params.gain.r_gain/params->awb_params.gain.g_gain));
    p_cmd->bgWbGain = FLOAT_TO_Q(7,
      (params->awb_params.gain.b_gain/params->awb_params.gain.g_gain));
    p_cmd->grWbGain = FLOAT_TO_Q(7,
      (params->awb_params.gain.g_gain/params->awb_params.gain.r_gain));
    p_cmd->gbWbGain = FLOAT_TO_Q(7,
      (params->awb_params.gain.g_gain/params->awb_params.gain.b_gain));
    mod->update = TRUE;
  }

  /* config the demosaicConfigCmd for snapshot*/
  if (is_snap) {
    mod->demosaic_snapshot_cmd.bl = p_cmd->bl;
    mod->demosaic_snapshot_cmd.bu = p_cmd->bu;
    mod->demosaic_snapshot_cmd.dblu = p_cmd->dblu;
    mod->demosaic_snapshot_cmd.a = p_cmd->a;
    if (params->awb_gain_update == TRUE) {
        mod->demosaic_snapshot_cmd.rgWbGain = p_cmd->rgWbGain;
        mod->demosaic_snapshot_cmd.bgWbGain = p_cmd->bgWbGain;
        mod->demosaic_snapshot_cmd.grWbGain = p_cmd->grWbGain;
        mod->demosaic_snapshot_cmd.gbWbGain = p_cmd->gbWbGain;
     }
  }

  params->awb_gain_update = FALSE;
  return VFE_SUCCESS;
} /* vfe_demosaic_trigger_update */

/*===========================================================================
 * FUNCTION    - vfe_demosaic_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_reload_params(int mod_id, void* module,
  void* vparams)
{
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;
  CDBG("%s:", __func__);
  status = vfe_demosaic_set_params(mod, params, FALSE);
  if (VFE_SUCCESS != status)
    return status;

  status = vfe_demosaic_set_params(mod, params, TRUE);
  if (VFE_SUCCESS != status)
    return status;
  mod->reload_params = TRUE;
  return status;
} /*vfe_demosaic_reload_params*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_trigger_enable(int mod_id, void* module,
  void* params, int enable)
{
  demosaic_mod_t *mod = (demosaic_mod_t *)module;
  CDBG("%s: %d", __func__, enable);
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_demosaic_trigger_enable*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_test_vector_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_tv_validate(int mod_id, void *test_input,
  void *test_output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;

  VFE_DemosaicV3ConfigCmdType *InCmd =
    (VFE_DemosaicV3ConfigCmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_0_OFF/4));

  VFE_DemosaicV3ConfigCmdType *OutCmd =
    (VFE_DemosaicV3ConfigCmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_0_OFF/4));

  /* demosaic cfg need not be verified. Keeping the code so as to
     enable once trigger is there */
#if 0
  VALIDATE_TST_VEC(InCmd->dbpcEnable, OutCmd->dbpcEnable, 0,
    "dbpcEnable");
  VALIDATE_TST_VEC(InCmd->dbccEnable, OutCmd->dbccEnable, 0,
    "dbccEnable");
  VALIDATE_TST_VEC(InCmd->abccEnable, OutCmd->abccEnable, 0,
    "abccEnable");
  VALIDATE_TST_VEC(InCmd->abfEnable, OutCmd->abfEnable, 0,
    "abfEnable");
  VALIDATE_TST_VEC(InCmd->cositedRgbEnable, OutCmd->cositedRgbEnable, 0,
   "cositedRgbEnable");
  VALIDATE_TST_VEC(InCmd->abccLutBankSel, OutCmd->abccLutBankSel, 0,
   "cositedRgbEnable");
  VALIDATE_TST_VEC(InCmd->pipeFlushCount, OutCmd->pipeFlushCount, 0,
    "pipeFlushCount");
  VALIDATE_TST_VEC(InCmd->pipeFlushOvd, OutCmd->pipeFlushOvd, 0,
    "pipeFlushOvd");
  VALIDATE_TST_VEC(InCmd->flushHaltOvd, OutCmd->flushHaltOvd, 0,
    "flushHaltOvd");
#endif

  InCmd = (VFE_DemosaicV3ConfigCmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_1_OFF/4) - 1); //4 bytes back to set the correct location

  OutCmd = (VFE_DemosaicV3ConfigCmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_1_OFF/4) - 1); //4 bytes back to set the correct location

  VALIDATE_TST_VEC(InCmd->rgWbGain, OutCmd->rgWbGain, 0,
    "rgWbGain");
  VALIDATE_TST_VEC(InCmd->bgWbGain, OutCmd->bgWbGain, 0,
    "bgWbGain");
  VALIDATE_TST_VEC(InCmd->grWbGain, OutCmd->grWbGain, 0,
    "grWbGain");
  VALIDATE_TST_VEC(InCmd->gbWbGain, OutCmd->gbWbGain, 0,
    "gbWbGain");

  InCmd = (VFE_DemosaicV3ConfigCmdType *)(mod_in->reg_dump +
    (VFE_DEMOSAICV3_2_OFF/4) - 21);
  //4*21 bytes back to set the correct location

  OutCmd = (VFE_DemosaicV3ConfigCmdType *)(mod_op->reg_dump_data +
    (VFE_DEMOSAICV3_2_OFF/4) - 21);
  //4*21 bytes back to set the correct location

  VALIDATE_TST_VEC(InCmd->bl, OutCmd->bl, 0,
    "bl");
  VALIDATE_TST_VEC(InCmd->bu, OutCmd->bu, 0,
    "bu");
  VALIDATE_TST_VEC(InCmd->dblu, OutCmd->dblu, 0,
    "dblu");
  VALIDATE_TST_VEC(InCmd->a, OutCmd->a, 0,
    "a");

  return VFE_SUCCESS;
} /*vfe_demosaic_test_vector_validation*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_set_bestshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode)
{
  CDBG_ERROR("%s: Not implemented\n", __func__);
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_demosaic_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_demosaic_deinit(int mod_id, void *module, void *params)
{
  demosaic_mod_t *d_mod = (demosaic_mod_t *)module;
  memset(d_mod, 0 , sizeof(demosaic_mod_t));
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_demosaic_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  demosaic_module_t *cmd = (demosaic_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->demosaic_up_cmd),
     sizeof(VFE_DemosaicV3UpdateCmdType),
     VFE_CMD_DEMOSAICV3_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_demosaic_plugin_update */
#endif
