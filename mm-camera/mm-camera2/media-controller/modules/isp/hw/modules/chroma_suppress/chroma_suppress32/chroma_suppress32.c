/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "chroma_suppress32.h"
#include "isp_log.h"

#ifdef ENABLE_CHROMA_SUPP_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#define CHROMA_SUPPRESS_DIFF_MIN 4
#define CHROMA_SUPPRESS_DIFF_MAX 127

/** util_chroma_suppress_cmd_debug:
 *    @cmd: Pointer to chroma suppress configuration.
 *
 * Print chroma suppress configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void util_chroma_suppress_cmd_debug(ISP_ChromaSuppress_ConfigCmdType *cmd,
   ISP_ChromaSuppress_Mix1_ConfigCmdType *mix1_cmd, ISP_ChromaSuppress_Mix2_ConfigCmdType *mix2_cmd)
{
  ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s:\n", __func__);
  ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: ySup1 : %d ySup2 : %d ySup3 : %d ySup4 : %d\n", __func__,
    cmd->ySup1, cmd->ySup2, cmd->ySup3, cmd->ySup4);

  ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: ySupM1 : %d ySupM3 : %d ySupS1 : %d ySupS3 : %d\n", __func__,
    mix1_cmd->ySupM1, mix1_cmd->ySupM3, mix1_cmd->ySupS1, mix1_cmd->ySupS3);

  ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: chromaSuppressEn : %d", __func__, mix1_cmd->chromaSuppressEn);
  ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: cSup1 : %d cSup2 : %d cSupM1 : %d cSupS1 : %d", __func__,
    mix2_cmd->cSup1, mix2_cmd->cSup2, mix2_cmd->cSupM1, mix2_cmd->cSupS1);
} /* util_chroma_suppress_cmd_debug *//* util_chroma_suppress_cmd_debug */

/** util_chroma_suppress_cmd_config:
 *    @mod: pointer to instance private data
 *
 * Prepare hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void util_chroma_suppress_cmd_config(isp_chroma_suppress_mod_t *mod)
{
  int Diff, Q_slope;

  /* copy from local thres to cmd */
  mod->reg_cmd.ySup1 = mod->thresholds.cs_luma_threshold1;
  mod->reg_cmd.ySup2 = mod->thresholds.cs_luma_threshold2;
  mod->reg_cmd.ySup3 = mod->thresholds.cs_luma_threshold3;
  mod->reg_cmd.ySup4 = mod->thresholds.cs_luma_threshold4;
  mod->reg_mix_cmd_2.reg_cmd.cSup1 = mod->thresholds.cs_chroma_threshold1;
  mod->reg_mix_cmd_2.reg_cmd.cSup2 = mod->thresholds.cs_chroma_threshold2;

  Diff = mod->reg_cmd.ySup2 - mod->reg_cmd.ySup1;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  mod->reg_cmd.ySup2 = mod->reg_cmd.ySup1 + Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->reg_mix_cmd_1.reg_cmd.ySupM1 = (1 << Q_slope) / Diff;
  mod->reg_mix_cmd_1.reg_cmd.ySupS1 = Q_slope - 7;

  Diff = mod->reg_cmd.ySup4 - mod->reg_cmd.ySup3;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  mod->reg_cmd.ySup3 = mod->reg_cmd.ySup4 - Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->reg_mix_cmd_1.reg_cmd.ySupM3 = (1 << Q_slope) / Diff;
  mod->reg_mix_cmd_1.reg_cmd.ySupS3 = Q_slope - 7;

  Diff = mod->reg_mix_cmd_2.reg_cmd.cSup2 - mod->reg_mix_cmd_2.reg_cmd.cSup1;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  mod->reg_mix_cmd_2.reg_cmd.cSup2 = mod->reg_mix_cmd_2.reg_cmd.cSup1 + Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->reg_mix_cmd_2.reg_cmd.cSupM1 = (1 << Q_slope) / Diff;
  mod->reg_mix_cmd_2.reg_cmd.cSupS1 = Q_slope - 7;
} /* util_chroma_suppress_cmd_config */

/** chroma_suppress_init:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @notify_ops: notify
 *
 * Open and initialize all required submodules
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_chroma_suppress_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->reg_mix_cmd_1.hw_mask = 0x01777F7F;
  mod->reg_mix_cmd_2.hw_mask = 0x077FFFFF;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->aec_ratio.ratio = 1.0f;

  return 0;
} /* chroma_suppress_init */

/** chroma_suppress_config:
 *    @mod: pointer to instance private data
 *    @pix_setting: input data
 *    @in_params_size: size of input data
 *
 * Configure module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_config(isp_chroma_suppress_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  chromatix_parms_type *chromatix_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CS_MCE_type *chromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;
  cs_luma_threshold_type *p_thresholds;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  if (!mod->enable) {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Mod not Enable.", __func__);
    return rc;
  }

  /* set old cfg to invalid value to trigger the first trigger update */
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  p_thresholds = &chromatix_CS_MCE->cs_normal;

  /* default values */
  mod->thresholds.cs_luma_threshold1 = p_thresholds->cs_luma_threshold1;
  mod->thresholds.cs_luma_threshold2 = p_thresholds->cs_luma_threshold2;
  mod->thresholds.cs_luma_threshold3 = p_thresholds->cs_luma_threshold3;
  mod->thresholds.cs_luma_threshold4 = p_thresholds->cs_luma_threshold4;
  mod->thresholds.cs_chroma_threshold1 = p_thresholds->cs_chroma_threshold1;
  mod->thresholds.cs_chroma_threshold2 = p_thresholds->cs_chroma_threshold2;

  util_chroma_suppress_cmd_config(mod);

  mod->skip_trigger = FALSE;
  mod->hw_update_pending = TRUE;

  return rc;
} /* chroma_suppress_config */

/** chroma_suppress_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_destroy(void *mod_ctrl)
{
  isp_chroma_suppress_mod_t *mod = mod_ctrl;

  memset(mod, 0, sizeof(isp_chroma_suppress_mod_t));
  free(mod);

  return 0;
} /* chroma_suppress_destroy */

/** chroma_suppress_enable:
 *    @mod: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Enable module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_enable(isp_chroma_suppress_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->enable = enable->enable;
  mod->reg_mix_cmd_1.reg_cmd.chromaSuppressEn = mod->enable;

  if (!mod->enable)
    mod->hw_update_pending = 0;

  return 0;
} /* chroma_suppress_enable */

/** chroma_suppress_trigger_enable:
 *    @mod: pointer to instance private data
 *    @enable: input data
 *    @in_params_size: size of input data
 *
 * Trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_trigger_enable(isp_chroma_suppress_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->trigger_enable = enable->enable;

  return 0;
} /* chroma_suppress_trigger_enable */

/** chroma_suppress_trigger_update:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Update configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_trigger_update(isp_chroma_suppress_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  ISP_ChromaSuppress_ConfigCmdType *reg_cmd = &mod->reg_cmd;
  chromatix_parms_type *chromatix_ptr =
    in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_CS_MCE_type *chromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;
  trigger_point_type *p_trigger_point;
  float ratio;
  cs_luma_threshold_type *cs_luma_threshold, *cs_luma_threshold_lowlight;
  uint8_t update_cs = FALSE;
  int is_burst;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  if (!mod->enable || !mod->trigger_enable || mod->skip_trigger) {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Skip Trigger update. enable %d, trig_enable %d, skip_trigger %d",
      __func__, mod->enable, mod->trigger_enable, mod->skip_trigger);
    return rc;
  }

  is_burst = IS_BURST_STREAMING(in_params);

  if (!is_burst && !isp_util_aec_check_settled(
                      &in_params->trigger_input.stats_update.aec_update)) {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: AEC not settled", __func__);
    return rc;
  }

  p_trigger_point = &(chromatix_CS_MCE->cs_lowlight_trigger);
  cs_luma_threshold = &(chromatix_CS_MCE->cs_normal);
  cs_luma_threshold_lowlight = &(chromatix_CS_MCE->cs_lowlight);

  ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
    chromatix_CS_MCE->control_cs, p_trigger_point,
    &(in_params->trigger_input.stats_update.aec_update), is_burst);

  update_cs = !F_EQUAL(mod->aec_ratio.ratio, ratio);
  mod->aec_ratio.ratio = ratio;
  if (!update_cs) {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Chroma Suppression update not required", __func__);
    return 0;
  }

  if (F_EQUAL(ratio, 0.0)) {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Low Light \n", __func__);
    mod->thresholds = *(cs_luma_threshold_lowlight);
  } else if (F_EQUAL(ratio, 1.0)) {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Normal Light \n", __func__);
    mod->thresholds = *(cs_luma_threshold);
  } else {
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Interpolate between Normal and Low Light \n", __func__);
    mod->thresholds.cs_luma_threshold1 = LINEAR_INTERPOLATION_INT(
        cs_luma_threshold->cs_luma_threshold1,
        cs_luma_threshold_lowlight->cs_luma_threshold1, ratio);
    mod->thresholds.cs_luma_threshold2 = LINEAR_INTERPOLATION_INT(
        cs_luma_threshold->cs_luma_threshold2,
        cs_luma_threshold_lowlight->cs_luma_threshold2, ratio);
    mod->thresholds.cs_luma_threshold3 = LINEAR_INTERPOLATION_INT(
        cs_luma_threshold->cs_luma_threshold3,
        cs_luma_threshold_lowlight->cs_luma_threshold3, ratio);
    mod->thresholds.cs_luma_threshold4 = LINEAR_INTERPOLATION_INT(
        cs_luma_threshold->cs_luma_threshold4,
        cs_luma_threshold_lowlight->cs_luma_threshold4, ratio);
    mod->thresholds.cs_chroma_threshold1 = LINEAR_INTERPOLATION_INT(
        cs_luma_threshold->cs_chroma_threshold1,
        cs_luma_threshold_lowlight->cs_chroma_threshold1, ratio);
    mod->thresholds.cs_chroma_threshold2 = LINEAR_INTERPOLATION_INT(
        cs_luma_threshold->cs_chroma_threshold2,
        cs_luma_threshold_lowlight->cs_chroma_threshold2, ratio);
  }

  util_chroma_suppress_cmd_config(mod);

  mod->hw_update_pending = TRUE;

  return 0;
} /* chroma_suppress_trigger_update */

/** chroma_suppress_set_chromatix:
 *    @mod: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set chromatix.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_set_chromatix(isp_chroma_suppress_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  uint32_t Diff;
  int Q_slope;
  chromatix_parms_type * chromatix_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_CS_MCE_type *chromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;

  mod->reg_cmd.ySup1 = chromatix_CS_MCE->cs_normal.cs_luma_threshold1;
  mod->reg_cmd.ySup2 = chromatix_CS_MCE->cs_normal.cs_luma_threshold2;
  mod->reg_cmd.ySup3 = chromatix_CS_MCE->cs_normal.cs_luma_threshold3;
  mod->reg_cmd.ySup4 = chromatix_CS_MCE->cs_normal.cs_luma_threshold4;
  mod->reg_mix_cmd_2.reg_cmd.cSup1 =
    chromatix_CS_MCE->cs_normal.cs_chroma_threshold1;
  mod->reg_mix_cmd_2.reg_cmd.cSup2 =
    chromatix_CS_MCE->cs_normal.cs_chroma_threshold2;

  Diff = mod->reg_cmd.ySup2 - mod->reg_cmd.ySup1;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  mod->reg_cmd.ySup2 = mod->reg_cmd.ySup1 + Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->reg_mix_cmd_1.reg_cmd.ySupM1 = (1 << Q_slope) / Diff;
  mod->reg_mix_cmd_1.reg_cmd.ySupS1 = Q_slope - 7;

  Diff = mod->reg_cmd.ySup4 - mod->reg_cmd.ySup3;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  mod->reg_cmd.ySup3 = mod->reg_cmd.ySup4 - Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->reg_mix_cmd_1.reg_cmd.ySupM3 = (1 << Q_slope) / Diff;
  mod->reg_mix_cmd_1.reg_cmd.ySupS3 = Q_slope - 7;

  Diff = mod->reg_mix_cmd_2.reg_cmd.cSup2 - mod->reg_mix_cmd_2.reg_cmd.cSup1;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  mod->reg_mix_cmd_2.reg_cmd.cSup2 = mod->reg_mix_cmd_2.reg_cmd.cSup1 + Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->reg_mix_cmd_2.reg_cmd.cSupM1 = (1 << Q_slope) / Diff;
  mod->reg_mix_cmd_2.reg_cmd.cSupS1 = Q_slope - 7;

  mod->skip_trigger = FALSE;

  return 0;
} /* chroma_suppress_set_chromatix */

/** chroma_suppress_do_hw_update:
 *    @chroma_supp_mod: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_do_hw_update(
  isp_chroma_suppress_mod_t *chroma_supp_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[3];

  if (chroma_supp_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)&chroma_supp_mod->reg_cmd;
    cfg_cmd.cmd_len = sizeof(chroma_supp_mod->reg_cmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 3;

    /*write value into mixed register 1*/
    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_CHROMA32_SUP_MIX_OFF_1;
    reg_cfg_cmd[0].u.mask_info.mask = chroma_supp_mod->reg_mix_cmd_1.hw_mask;
    reg_cfg_cmd[0].u.mask_info.val = chroma_supp_mod->reg_mix_cmd_1.reg_cmd.cfg;

    /*write value into mixed register 2*/
    reg_cfg_cmd[1].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[1].u.mask_info.reg_offset = ISP_CHROMA32_SUP_MIX_OFF_2;
    reg_cfg_cmd[1].u.mask_info.mask = chroma_supp_mod->reg_mix_cmd_2.hw_mask;
    reg_cfg_cmd[1].u.mask_info.val = chroma_supp_mod->reg_mix_cmd_2.reg_cmd.cfg;

    /*write regular chroma SS cmd*/
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[2].cmd_type = VFE_WRITE;
    reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_CHROMA32_SUP_OFF;
    reg_cfg_cmd[2].u.rw_info.len = ISP_CHROMA32_SUP_LEN * sizeof(uint32_t);

    util_chroma_suppress_cmd_debug(&(chroma_supp_mod->reg_cmd),
        &(chroma_supp_mod->reg_mix_cmd_1.reg_cmd), &(chroma_supp_mod->reg_mix_cmd_2.reg_cmd));

    rc = ioctl(chroma_supp_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    /*Store the applied hw reg config*/
    chroma_supp_mod->applied_reg_cmd = chroma_supp_mod->reg_cmd;
    chroma_supp_mod->reg_mix_cmd_1.applied_reg_cmd =
      chroma_supp_mod->reg_mix_cmd_1.reg_cmd;
    chroma_supp_mod->reg_mix_cmd_2.applied_reg_cmd =
      chroma_supp_mod->reg_mix_cmd_2.reg_cmd;
    chroma_supp_mod->hw_update_pending = 0;
  }

  /* TODO: update hw reg */
  return rc;
} /* chroma_suppress_hw_reg_update */

/** chroma_suppress_set_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set parameter function. It handle all input parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_set_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_chroma_suppress_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_CHROMATIX_RELOAD: {
    rc = chroma_suppress_set_chromatix(mod,
           (isp_hw_pix_setting_params_t *)in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = chroma_suppress_enable(mod, (isp_mod_set_enable_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = chroma_suppress_config(mod, (isp_hw_pix_setting_params_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = chroma_suppress_trigger_enable(mod, (isp_mod_set_enable_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = chroma_suppress_trigger_update(mod,
           (isp_pix_trigger_update_input_t *)in_params, in_param_size);
  }
    break;

  default:
    return -EAGAIN;
    break;
  }

  return rc;
} /* chroma_suppress_set_params */

/** chroma_suppress_get_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *    @out_params: output data
 *    @out_params_size: size of output data
 *
 * Get parameter function. It handle all parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
{
  isp_chroma_suppress_mod_t *mod = mod_ctrl;
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
    chromasuppression_t *chromasupp = &vfe_diag->prev_chromasupp;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_CHROMA_SUPPRESS, "%s: Populating vfe_diag data", __func__);
    if (NULL == chromasupp || NULL == mod ) {
      CDBG_ERROR("%s: NULL chromasupp %x mod %x", __func__,
        (unsigned int)chromasupp, (unsigned int)mod);
      break;
    }
    vfe_diag->control_chromasupp.enable = mod->enable;
    vfe_diag->control_chromasupp.cntrlenable = mod->trigger_enable;
    chromasupp->ysup1  = mod->applied_reg_cmd.ySup1;
    chromasupp->ysup2  = mod->applied_reg_cmd.ySup2;
    chromasupp->ysup3  = mod->applied_reg_cmd.ySup3;
    chromasupp->ysup4  = mod->applied_reg_cmd.ySup4;
    chromasupp->ysupM1 = mod->reg_mix_cmd_1.applied_reg_cmd.ySupM1;
    chromasupp->ysupM3 = mod->reg_mix_cmd_1.applied_reg_cmd.ySupM3;
    chromasupp->ysupS1 = mod->reg_mix_cmd_1.applied_reg_cmd.ySupS1;
    chromasupp->ysupS3 = mod->reg_mix_cmd_1.applied_reg_cmd.ySupS3;
    chromasupp->csup1  = mod->reg_mix_cmd_2.applied_reg_cmd.cSup1;
    chromasupp->csup2  = mod->reg_mix_cmd_2.applied_reg_cmd.cSup2;
    chromasupp->csupM1 = mod->reg_mix_cmd_2.applied_reg_cmd.cSupM1;
    chromasupp->csupS1 = mod->reg_mix_cmd_2.applied_reg_cmd.cSupS1;
  }
    break;

  default: {
    rc = -1;
  }
    break;
  }

  return rc;
} /* chroma_suppress_get_params */

/** chroma_suppress_action:
 *    @mod_ctrl: pointer to instance private data
 *    @action_code: action id
 *    @action_data: action data
 *    @action_data_size: action data size
 *
 * Handle all actions.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int chroma_suppress_action(void *mod_ctrl, uint32_t action_code,
  void *data, uint32_t data_size)
{
  int rc = 0;
  isp_chroma_suppress_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = chroma_suppress_do_hw_update(mod);
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
} /* chroma_suppress_action */

/** chroma_suppress32_open:
 *    @version: version of isp
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *chroma_suppress32_open(uint32_t version)
{
  isp_chroma_suppress_mod_t *mod = malloc(sizeof(isp_chroma_suppress_mod_t));

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory", __func__);
    return NULL;
  }

  memset(mod, 0, sizeof(isp_chroma_suppress_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = chroma_suppress_init;
  mod->ops.destroy = chroma_suppress_destroy;
  mod->ops.set_params = chroma_suppress_set_params;
  mod->ops.get_params = chroma_suppress_get_params;
  mod->ops.action = chroma_suppress_action;

  return &mod->ops;
} /* chroma_suppress32_open */
