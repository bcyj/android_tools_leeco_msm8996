/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "demosaic32.h"
#include "isp_log.h"

#ifdef ENABLE_DEMOSAIC_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif


static uint16_t wInterpDefault[ISP_DEMOSAIC32_CLASSIFIER_CNT] =
    { 137, 91, 91, 1023, 922, 93, 195, 99, 64, 319, 197, 88, 84, 109, 151, 98,
       66, 76 };
static uint16_t bInterpDefault[ISP_DEMOSAIC32_CLASSIFIER_CNT] =
    { 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1 };
static uint16_t lInterpDefault[ISP_DEMOSAIC32_CLASSIFIER_CNT] =
    { 0, 0, 1, 2, 2, 3, 9, 9, 9, 4, 4, 5, 6, 7, 8, 8, 10, 10 };
static int16_t tInterpDefault[ISP_DEMOSAIC32_CLASSIFIER_CNT] =
    { 2, 1, 0, 0, -1, 2, 0, -1, 1, 0, -1, 2, 0, 2, 2, 1, 0, 100 };


/*===========================================================================
 * FUNCTION    - demosaic_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void demosaic_debug(void *cmd, uint8_t update)
{
  ISP_Demosaic32ConfigCmdType* pcmd = (ISP_Demosaic32ConfigCmdType *)cmd;

  ISP_DBG(ISP_MOD_DEMOSAIC, "VFE_Demosaic32 config = %d or update = %d", !update, update);

  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType rgWbGain %d", pcmd->rgWbGain);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType bgWbGain %d", pcmd->bgWbGain);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType grWbGain %d", pcmd->grWbGain);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType gbWbGain %d", pcmd->gbWbGain);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType bl %d", pcmd->bl);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType bu %d", pcmd->bu);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType dblu %d", pcmd->dblu);
  ISP_DBG(ISP_MOD_DEMOSAIC, "ISP_Demosaic32CmdType a %d", pcmd->a);

}/*demosaic_debug*/

/*===========================================================================
 * FUNCTION    - demosaic_set_cfg_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void demosaic_set_cfg_params(isp_demosaic_mod_t *demosaic,
                           isp_hw_pix_setting_params_t *in_params,
                           uint8_t is_burst)
{
  int i;
  int temp;
  chromatix_parms_type *chromatix_ptr = in_params->chromatix_ptrs.chromatixPtr;
  demosaic3_LUT_type *classifier = NULL;
  chromatix_demosaic_type *chromatix_demosaic;
  ISP_Demosaic32ConfigCmdType *p_cmd = &demosaic->reg_cmd;
  ISP_Demosaic32MixConfigCmdType *mix_cmd = &demosaic->mix_reg_cmd;
  float bL = 0.0;
  float aG = 0.0;

  chromatix_demosaic = &(chromatix_ptr->chromatix_VFE.chromatix_demosaic);
  classifier = &(chromatix_demosaic->demosaic3_LUT);
  bL = chromatix_demosaic->demosaic3_bL[1];
  aG = chromatix_demosaic->demosaic3_aG[1];

  /* default values */
  mix_cmd->abccLutBankSel = 0;
  mix_cmd->pipeFlushCount = 0;
  mix_cmd->pipeFlushOvd = 0;
  mix_cmd->flushHaltOvd = 0;
  mix_cmd->cositedRgbEnable = FALSE;

  /* Classifier */
  for (i=0 ; i<ISP_DEMOSAIC32_CLASSIFIER_CNT ; ++i) {
    p_cmd->interpClassifier[i].w_n = FLOAT_TO_Q(10,classifier->wk[i]);
    p_cmd->interpClassifier[i].t_n = classifier->Tk[i];
    p_cmd->interpClassifier[i].l_n = classifier->lk[i];
    p_cmd->interpClassifier[i].b_n = classifier->bk[i];
  }

  /* Interp G */
  temp = FLOAT_TO_Q(8, bL);
  ISP_DBG(ISP_MOD_DEMOSAIC, "%s: bl %d", __func__, temp);
  p_cmd->bl = MIN(MAX(0, temp), 118);
  temp = FLOAT_TO_Q(8, (1.0 - bL));
  p_cmd->bu = MIN(MAX(138, temp), 255);
  temp = FLOAT_TO_Q(5, (1.0/(1.0 - 2 * bL)));
  p_cmd->dblu = MIN(MAX(0, temp), 511);
  temp = FLOAT_TO_Q(6, aG);
  p_cmd->a = MIN(MAX(0, temp), 63);
} /* demosaic_set_cfg_params */
/*===========================================================================
 * FUNCTION    - demosaic_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int demosaic_trigger_update(isp_demosaic_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  int temp, is_burst;
  float ratio = 0.0, aG = 0.0, bL = 0.0, aG_lowlight = 0.0, bL_lowlight = 0.0;
  ISP_Demosaic32ConfigCmdType *p_cmd = &mod->reg_cmd;
  chromatix_parms_type *chromatix_ptr =
    trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_demosaic_type *chromatix_demosaic =
    &chromatix_ptr->chromatix_VFE.chromatix_demosaic;
  awb_gain_t* wb_gain =
    (awb_gain_t *)&trigger_params->trigger_input.stats_update.awb_update.gain;

  aec_update_t *aec_output = &trigger_params->trigger_input.stats_update.aec_update;
  tuning_control_type *tc = NULL;
  trigger_point_type  *tp = NULL;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
  return -1;
  }

  if (!mod->enable || !mod->trigger_enable) {
    ISP_DBG(ISP_MOD_DEMOSAIC, "%s:No trigger update for Demosaic: enable = %d, trigger_en = %d.\n",
         __func__, mod->enable, mod->trigger_enable);
    return 0;
  }

  is_burst = IS_BURST_STREAMING(&trigger_params->cfg);
  if (!is_burst) {
    if (!isp_util_aec_check_settled(aec_output)) {
      ISP_DBG(ISP_MOD_DEMOSAIC, "%s: aec not settled, skip trigger\n", __func__);
      return 0;
    }
  }

  tc = &(chromatix_demosaic->control_demosaic3);
  tp = &(chromatix_demosaic->demosaic3_trigger_lowlight);
  aG = chromatix_demosaic->demosaic3_aG[1];
  bL = chromatix_demosaic->demosaic3_bL[1];
  aG_lowlight = chromatix_demosaic->demosaic3_aG[0];
  bL_lowlight = chromatix_demosaic->demosaic3_bL[0];

  /*Do interpolation by the aec ratio*/
  ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
                                 *tc, tp, aec_output, is_burst);
  ISP_DBG(ISP_MOD_DEMOSAIC, "%s: aec ratio %f", __func__, ratio);

  if (trigger_params->cfg.streaming_mode != mod->old_streaming_mode ||
    !F_EQUAL(ratio, mod->ratio.ratio)) {
    float new_aG, new_bL;

    new_aG = LINEAR_INTERPOLATION(aG, aG_lowlight, ratio);
    new_bL = LINEAR_INTERPOLATION(bL, bL_lowlight, ratio);
    /*calculate bl*/
    temp = FLOAT_TO_Q(8, new_bL);
    p_cmd->bl = MIN(MAX(0, temp), 118);
    /*calculate bu*/
    temp = FLOAT_TO_Q(8, (1.0-new_bL));
    p_cmd->bu = MIN(MAX(138, temp), 255);
    /*calculate dblu*/
    temp = FLOAT_TO_Q(5, (1.0/(1.0-2*new_bL)));
    p_cmd->dblu = MIN(MAX(0, temp), 511);
    /*calculate a*/
    temp = FLOAT_TO_Q(6, new_aG);
    p_cmd->a = MIN(MAX(0, temp), 63);

    mod->ratio.ratio = ratio;
    mod->old_streaming_mode = trigger_params->cfg.streaming_mode;
  }

  /* update wb gains */
  ISP_DBG(ISP_MOD_DEMOSAIC, "%s: gains r %f g %f b %f", __func__,
    wb_gain->r_gain, wb_gain->g_gain, wb_gain->b_gain);
  p_cmd->rgWbGain = FLOAT_TO_Q(7, (wb_gain->r_gain / wb_gain->g_gain));
  p_cmd->bgWbGain = FLOAT_TO_Q(7, (wb_gain->b_gain / wb_gain->g_gain));
  p_cmd->grWbGain = FLOAT_TO_Q(7, (wb_gain->g_gain / wb_gain->r_gain));
  p_cmd->gbWbGain = FLOAT_TO_Q(7, (wb_gain->g_gain / wb_gain->b_gain));
  mod->hw_update_pending = TRUE;

  return 0;
} /* demosaic_trigger_update */

/* ============================================================
 * function name: demosaic_config
 * description: config demosaic
 * ============================================================*/
static int demosaic_config(isp_demosaic_mod_t *mod, isp_hw_pix_setting_params_t *in_params, uint32_t size)
{
  int i, rc = 0;
  uint8_t is_burst = IS_BURST_STREAMING(in_params);

  if (sizeof(isp_hw_pix_setting_params_t) != size) {
    CDBG_ERROR("%s: in_params size mismatch\n", __func__);
    return -1;
  }
  if (!mod->enable) {
    ISP_DBG(ISP_MOD_DEMOSAIC, "%s: demosaic enable = %d\n", __func__, mod->enable);
    return rc;
  }

  demosaic_set_cfg_params(mod, in_params, is_burst);
  mod->hw_update_pending = TRUE;
  return rc;
}
/* ============================================================
 * function name: demosaic_trigger_enable
 * description: enable trigger update feature
 * ============================================================*/
static int demosaic_trigger_enable(isp_demosaic_mod_t *demosaic,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  demosaic->trigger_enable = enable->enable;
  return 0;
}
/* ============================================================
 * function name: demosaic_enable
 * description: enable demosaic
 * ============================================================*/
static int demosaic_enable(isp_demosaic_mod_t *demosaic,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  demosaic->enable = enable->enable;

  return 0;
}
/* ============================================================
 * function name: demosaic_destroy
 * description: close demosaic
 * ============================================================*/
static int demosaic_destroy (void *mod_ctrl)
{
  isp_demosaic_mod_t *demosaic = mod_ctrl;

  memset(demosaic,  0,  sizeof(isp_demosaic_mod_t));
  free(demosaic);
  return 0;
}
/* ============================================================
 * function name: demosaic_set_params
 * description: set parameters
 * ============================================================*/
static int demosaic_set_params (void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_demosaic_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = demosaic_enable(mod, (isp_mod_set_enable_t *)in_params,
                     in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = demosaic_config(mod, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = demosaic_trigger_enable(mod, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = demosaic_trigger_update(mod, in_params, in_param_size);
    break;
  default:
    CDBG_ERROR("%s: param_id %d, is not supported in this module\n", __func__, param_id);
    break;
  }
  return rc;
}

/** demosaic_ez_isp_update
 *  @demosaic_module
 *  @demosaicDiag
 *
 **/
static void demosaic_ez_isp_update(
  isp_demosaic_mod_t *demosaic_module,
  demosaic3_t *demosaicDiag)
{
  ISP_Demosaic32ConfigCmdType *demosaicCfg;
  int index;
  demosaicCfg = &(demosaic_module->applied_RegCmd);
  demosaicDiag->aG = demosaicCfg->a;
  demosaicDiag->bL = demosaicCfg->bl;

  for(index = 0; index < ISP_DEMOSAIC32_CLASSIFIER_CNT; index++) {
    demosaicDiag->lut[index].bk = demosaicCfg->interpClassifier[index].b_n;
    demosaicDiag->lut[index].wk = demosaicCfg->interpClassifier[index].w_n;
    demosaicDiag->lut[index].lk = demosaicCfg->interpClassifier[index].l_n;
    demosaicDiag->lut[index].tk = demosaicCfg->interpClassifier[index].t_n;
  }

}
/* ============================================================
 * function name: demosaic_get_params
 * description: get parameters
 * ============================================================*/
static int demosaic_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_demosaic_mod_t *mod = mod_ctrl;
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
    demosaic3_t *demosaicDiag;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    demosaicDiag = &(vfe_diag->prev_demosaic);
    if (mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
      demosaicDiag = &(vfe_diag->snap_demosaic);
    }
    vfe_diag->control_demosaic.enable = mod->enable;
    vfe_diag->control_demosaic.cntrlenable = mod->trigger_enable;
    demosaic_ez_isp_update(mod, demosaicDiag);
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_DEMOSAIC, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default:
    rc = -EPERM;
    break;
  }
  return rc;
}
static int demosaic_do_hw_update(isp_demosaic_mod_t *demosaic_mod)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[4];
  ISP_Demosaic32MixConfigCmdType cfg_mask;

  /*regular hw wirte for Demosaic*/
  if (demosaic_mod->hw_update_pending) {
    demosaic_debug(&demosaic_mod->reg_cmd,
                          demosaic_mod->classifier_cfg_done);
    cfg_cmd.cfg_data = (void *) &demosaic_mod->reg_cmd;
    cfg_cmd.cmd_len = sizeof(demosaic_mod->reg_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    if (demosaic_mod->classifier_cfg_done == 0)
      cfg_cmd.num_cfg = 4;
    else
      cfg_cmd.num_cfg = 3;

    /* Set bits for update */
    cfg_mask.cositedRgbEnable = 1;
    cfg_mask.abccLutBankSel = 1;
    cfg_mask.pipeFlushCount = 1;
    cfg_mask.pipeFlushOvd = 1;
    cfg_mask.flushHaltOvd = 1;

    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_DEMOSAIC_MIX_CFG_OFF;
    reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.cfg;
    reg_cfg_cmd[0].u.mask_info.val = *(uint32_t *)&demosaic_mod->mix_reg_cmd;

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DEMOSAIC32_WB_GAIN_OFF;
    reg_cfg_cmd[1].u.rw_info.len = ISP_DEMOSAIC32_WB_GAIN_LEN * sizeof(uint32_t);

     /* 20st reg in demosaic_mod->reg_cmd*/
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset = 20 * sizeof(uint32_t);
    reg_cfg_cmd[2].cmd_type = VFE_WRITE;
    reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_DEMOSAIC_INTERP_GAIN_OFF;
    reg_cfg_cmd[2].u.rw_info.len = ISP_DEMOSAIC_INTERP_GAIN_LEN * sizeof(uint32_t);

    if (demosaic_mod->classifier_cfg_done == 0) {
      /* 2nd register in demosaic_mod->reg_cmd */
      reg_cfg_cmd[3].u.rw_info.cmd_data_offset = 2 * sizeof(uint32_t);
      reg_cfg_cmd[3].cmd_type = VFE_WRITE;
      reg_cfg_cmd[3].u.rw_info.reg_offset = ISP_DEMOSAIC_CLASSIFIER_OFF;
      reg_cfg_cmd[3].u.rw_info.len = ISP_DEMOSAIC_CLASSIFIER_LEN * sizeof(uint32_t);
    }

    rc = ioctl(demosaic_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
       CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
       return rc;
     }
     memcpy(&(demosaic_mod->applied_RegCmd), &(demosaic_mod->reg_cmd),
       sizeof(ISP_Demosaic32ConfigCmdType));
     demosaic_mod->classifier_cfg_done = 1;
     demosaic_mod->hw_update_pending = 0;
  }

  return rc;
}

/* ============================================================
 * function name: demosaic_action
 * description: processing the action
 * ============================================================*/
static int demosaic_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_demosaic_mod_t *demosaic = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = demosaic_do_hw_update(demosaic);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}
/* ============================================================
 * function name: demosaic_init
 * description: init demosaic
 * ============================================================*/
static int demosaic_init (void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_demosaic_mod_t *demosaic = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  demosaic->fd = init_params->fd;
  demosaic->notify_ops = notify_ops;
  demosaic->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  return 0;
}
/* ============================================================
 * function name: demosaic32_open
 * description: open scaler
 * ============================================================*/
isp_ops_t *demosaic32_open(uint32_t version)
{
  isp_demosaic_mod_t *mod = malloc(sizeof(isp_demosaic_mod_t));

  if (!mod) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(mod,  0,  sizeof(isp_demosaic_mod_t));
  mod->ops.ctrl = (void *)mod;
  mod->ops.init = demosaic_init;
  /* destroy the module object */
  mod->ops.destroy = demosaic_destroy;
  /* set parameter */
  mod->ops.set_params = demosaic_set_params;
  /* get parameter */
  mod->ops.get_params = demosaic_get_params;
  mod->ops.action = demosaic_action;
  return &mod->ops;
}


