/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CHROMA_SUPP_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_chrom_supp_config_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_chrom_supp_config_debug(VFE_ChromaSuppress_ConfigCmdType *cmd)
{
  CDBG("%s:\n",__func__);
  CDBG("ySup1 : %d", cmd->ySup1);
  CDBG("ySup2 : %d", cmd->ySup2);
  CDBG("ySup3 : %d", cmd->ySup3);
  CDBG("ySup4 : %d", cmd->ySup4);
  CDBG("ySupM1 : %d", cmd->ySupM1);
  CDBG("ySupM3 : %d", cmd->ySupM3);
  CDBG("ySupS1 : %d", cmd->ySupS1);
  CDBG("ySupS3 : %d", cmd->ySupS3);
  CDBG("chromaSuppressEn : %d",
    cmd->chromaSuppressEn);
  CDBG("cSup1 : %d",cmd->cSup1);
  CDBG("cSup2 : %d",cmd->cSup2);
  CDBG("cSupM1 : %d",cmd->cSupM1);
  CDBG("cSupS1 : %d",cmd->cSupS1);

} /* vfe_chrom_supp_config_debug */
/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_init(int mod_id, void *mod_csupp,
  void *vparms)
{
  chroma_supp_mod_t *mod = (chroma_supp_mod_t *)mod_csupp;
  vfe_params_t *parms = (vfe_params_t *)vparms;
  chromatix_parms_type *pchromatix = parms->chroma3a;
  int Diff, Q_slope;

  CDBG("%s\n",__func__);
  mod->chroma_supp_video_cmd.ySup1 =
    pchromatix->cs_luma_threshold.cs_luma_threshold1;
  mod->chroma_supp_video_cmd.ySup2 =
    pchromatix->cs_luma_threshold.cs_luma_threshold2;
  mod->chroma_supp_video_cmd.ySup3 =
    pchromatix->cs_luma_threshold.cs_luma_threshold3;
  mod->chroma_supp_video_cmd.ySup4 =
    pchromatix->cs_luma_threshold.cs_luma_threshold4;
  mod->chroma_supp_video_cmd.cSup1 =
    pchromatix->cs_luma_threshold.cs_chroma_threshold1;
  mod->chroma_supp_video_cmd.cSup2 =
    pchromatix->cs_luma_threshold.cs_chroma_threshold2;

  Diff = mod->chroma_supp_video_cmd.ySup2 - mod->chroma_supp_video_cmd.ySup1;
  Diff = Clamp(Diff, 4, 127);
  mod->chroma_supp_video_cmd.ySup2 = mod->chroma_supp_video_cmd.ySup1 + Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->chroma_supp_video_cmd.ySupM1 = (1 << Q_slope) / Diff;
  mod->chroma_supp_video_cmd.ySupS1 = Q_slope - 7;

  Diff = mod->chroma_supp_video_cmd.ySup4 - mod->chroma_supp_video_cmd.ySup3;
  Diff = Clamp(Diff, 4, 127);
  mod->chroma_supp_video_cmd.ySup3 = mod->chroma_supp_video_cmd.ySup4 - Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->chroma_supp_video_cmd.ySupM3 = (1 << Q_slope) / Diff;
  mod->chroma_supp_video_cmd.ySupS3 = Q_slope - 7;

  Diff = mod->chroma_supp_video_cmd.cSup2 - mod->chroma_supp_video_cmd.cSup1;
  Diff = mod->chroma_supp_video_cmd.cSup2 - mod->chroma_supp_video_cmd.cSup1;
  Diff = Clamp(Diff, 4, 127);
  mod->chroma_supp_video_cmd.cSup2 = mod->chroma_supp_video_cmd.cSup1 + Diff;
  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  mod->chroma_supp_video_cmd.cSupM1 = (1 << Q_slope) / Diff;
  mod->chroma_supp_video_cmd.cSupS1 = Q_slope - 7;

  mod->chroma_supp_trigger = TRUE;
  mod->chroma_supp_update = FALSE;
  mod->prev_mode = VFE_OP_MODE_INVALID;
  mod->prev_aec_ratio = 0.0;
  mod->hw_enable = FALSE;

  return VFE_SUCCESS;
} /* vfe_chroma_suppression_init */

/*===========================================================================
 * FUNCTION.   - vfe_chroma_suppression_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_chroma_suppression_enable(int mod_id, void *mod_csupp,
  void *vparms, int8_t enable, int8_t hw_write)
{
  CDBG("%s: enable: %d\n", __func__, enable);
  chroma_supp_mod_t *mod = (chroma_supp_mod_t *)mod_csupp;
  vfe_params_t *params = (vfe_params_t *)vparms;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  params->moduleCfg->chromaSuppressionMceEnable |= enable;

  if(IS_SNAP_MODE(params)) {
    mod->chroma_supp_snap_cmd.chromaSuppressEn = enable;
  } else
    mod->chroma_supp_video_cmd.chromaSuppressEn = enable;

  mod->chroma_supp_enable = enable;

  if (hw_write && (mod->chroma_supp_enable == enable))
    return VFE_SUCCESS;

  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_CHROMA_SUPPRESS)
      : (params->current_config & ~VFE_MOD_CHROMA_SUPPRESS);
  }
  return VFE_SUCCESS;
} /* vfe_chroma_suppression_enable */

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_config(int mod_id, void *mod_csupp,
  void *parms)
{
  VFE_ChromaSuppress_ConfigCmdType *chroma_supp_cmd;
  chroma_supp_mod_t *mod = (chroma_supp_mod_t *)mod_csupp;
  vfe_params_t *p_obj = (vfe_params_t *)parms;

  if (!mod->chroma_supp_enable) {
    CDBG("%s: Chroma Suppression not enabled", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s: mode: %d\n", __func__, p_obj->vfe_op_mode);

  if(IS_SNAP_MODE(p_obj)) {
    chroma_supp_cmd = &(mod->chroma_supp_snap_cmd);
  } else
    chroma_supp_cmd = &(mod->chroma_supp_video_cmd);

  vfe_chrom_supp_config_debug(chroma_supp_cmd);
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd, CMD_GENERAL,
    (void *)chroma_supp_cmd, sizeof(*chroma_supp_cmd),
     VFE_CMD_CHROMA_SUP_CFG)) {
    CDBG_HIGH("%s: chroma sup config for op mode = %d failed\n", __func__,
      p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  return VFE_SUCCESS;
} /* vfe_chroma_suppression_config */

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_update(int mod_id, void *mod_csupp,
  void *parms)
{
  vfe_status_t status;
  VFE_ChromaSuppress_ConfigCmdType *chroma_supp_cmd;
  chroma_supp_mod_t *mod = (chroma_supp_mod_t *)mod_csupp;
  vfe_params_t *p_obj = (vfe_params_t *)parms;

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
    p_obj->update |= VFE_MOD_CHROMA_SUPPRESS;
    mod->hw_enable = FALSE;
  }
  if (!mod->chroma_supp_enable) {
    CDBG("%s: Chroma Suppression not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!mod->chroma_supp_update) {
    CDBG("%s: Chroma Suppression trigger not updated", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s: mode: %d\n", __func__, p_obj->vfe_op_mode);

  if(IS_SNAP_MODE(p_obj)) {
    chroma_supp_cmd = &(mod->chroma_supp_snap_cmd);
  } else
    chroma_supp_cmd = &(mod->chroma_supp_video_cmd);

  vfe_chrom_supp_config_debug(chroma_supp_cmd);
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd, CMD_GENERAL,
    (void *) chroma_supp_cmd, sizeof(*chroma_supp_cmd),
    VFE_CMD_CHROMA_SUP_UPDATE)) {
    CDBG_HIGH("%s: chroma sup update for op mode = %d failed\n", __func__,
      p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  p_obj->update |= VFE_MOD_CHROMA_SUPPRESS;
  mod->chroma_supp_update = FALSE;
  return VFE_SUCCESS;
} /* vfe_chroma_suppression_update */

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppresion_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppresion_trigger_update(int mod_id, void *mod_csupp,
  void *parms)
{
  float lux_idx, ratio;
  int Diff, Q_slope;
  tuning_control_type *tc;
  trigger_point_type  *tp;
  cs_luma_threshold_type *cs_luma_threshold, *cs_luma_threshold_lowlight;
  VFE_ChromaSuppress_ConfigCmdType *chroma_supp_cmd;
  uint8_t update_cs = FALSE;
  chroma_supp_mod_t *mod = (chroma_supp_mod_t *)mod_csupp;
  vfe_params_t *p_obj = (vfe_params_t *)parms;

  tc = &(p_obj->chroma3a->control_cs);

  if (!mod->chroma_supp_enable) {
    CDBG("%s: Chroma Suppression not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!mod->chroma_supp_trigger) {
    CDBG("%s: Chroma Suppression trigger not enabled", __func__);
    return VFE_SUCCESS;
  }
  switch (p_obj->vfe_op_mode) {
    case VFE_OP_MODE_PREVIEW:
    case VFE_OP_MODE_VIDEO:
    case VFE_OP_MODE_ZSL:
      tp = &(p_obj->chroma3a->cs_lowlight_trigger);
      cs_luma_threshold = &(p_obj->chroma3a->cs_luma_threshold);
      cs_luma_threshold_lowlight =
        &(p_obj->chroma3a->cs_luma_threshold_lowlight);
      chroma_supp_cmd = &(mod->chroma_supp_video_cmd);
      break;
    case VFE_OP_MODE_SNAPSHOT:
    case VFE_OP_MODE_JPEG_SNAPSHOT:
      tp = &(p_obj->chroma3a->cs_snapshot_lowlight_trigger);
      cs_luma_threshold = &(p_obj->chroma3a->cs_luma_threshold_snapshot);
      cs_luma_threshold_lowlight =
        &(p_obj->chroma3a->cs_luma_threshold_snapshot_lowlight);
      chroma_supp_cmd = &(mod->chroma_supp_snap_cmd);
      break;
    default:
      CDBG_ERROR("%s, invalid mode = %d",__func__, p_obj->vfe_op_mode);
      return VFE_ERROR_GENERAL;
  }
  ratio = vfe_util_get_aec_ratio(*tc, tp, p_obj);
  if (ratio > 1.0)
    ratio = 1.0;
  else if (ratio < 0.0)
    ratio = 0.0;

  update_cs = ((mod->prev_mode != p_obj->vfe_op_mode) ||
    !F_EQUAL(mod->prev_aec_ratio, ratio));

  CDBG("%s: update_cs : %d\n", __func__, update_cs);
  if (!update_cs) {
    CDBG("%s: Chroma Suppression update not required", __func__);
    return VFE_SUCCESS;
  }

  mod->prev_mode = p_obj->vfe_op_mode;
  mod->prev_aec_ratio = ratio;

  /* luma threshold */
  chroma_supp_cmd->ySup1 = LINEAR_INTERPOLATION_INT(cs_luma_threshold->
    cs_luma_threshold1, cs_luma_threshold_lowlight->cs_luma_threshold1, ratio);
  chroma_supp_cmd->ySup2 = LINEAR_INTERPOLATION_INT(cs_luma_threshold->
    cs_luma_threshold2, cs_luma_threshold_lowlight->cs_luma_threshold2, ratio);

  Diff = chroma_supp_cmd->ySup2 - chroma_supp_cmd->ySup1;
  Diff = Clamp(Diff, 4, 127);
  chroma_supp_cmd->ySup2 = chroma_supp_cmd->ySup1 + Diff;

  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  chroma_supp_cmd->ySupM1 = (1 << Q_slope) / Diff;
  chroma_supp_cmd->ySupS1 = Q_slope - 7;

  chroma_supp_cmd->ySup3 = LINEAR_INTERPOLATION_INT(cs_luma_threshold->
    cs_luma_threshold3, cs_luma_threshold_lowlight->cs_luma_threshold3, ratio);
  chroma_supp_cmd->ySup4 = LINEAR_INTERPOLATION_INT(cs_luma_threshold->
    cs_luma_threshold4, cs_luma_threshold_lowlight->cs_luma_threshold4, ratio);

  Diff = chroma_supp_cmd->ySup4 - chroma_supp_cmd->ySup3;
  Diff = Clamp(Diff, 4, 127);
  chroma_supp_cmd->ySup3 = chroma_supp_cmd->ySup4 - Diff;

  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  chroma_supp_cmd->ySupM3 = (1 << Q_slope) / Diff;
  chroma_supp_cmd->ySupS3 = Q_slope - 7;

  /* chroma threshold */
  chroma_supp_cmd->cSup1 = LINEAR_INTERPOLATION_INT(cs_luma_threshold->
    cs_chroma_threshold1,
    cs_luma_threshold_lowlight->cs_chroma_threshold1, ratio);
  chroma_supp_cmd->cSup2 = LINEAR_INTERPOLATION_INT(cs_luma_threshold->
    cs_chroma_threshold2,
    cs_luma_threshold_lowlight->cs_chroma_threshold2, ratio);
  Diff = chroma_supp_cmd->cSup2 - chroma_supp_cmd->cSup1;
  Diff = Clamp(Diff, 4, 127);
  chroma_supp_cmd->cSup2 = chroma_supp_cmd->cSup1 + Diff;

  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  chroma_supp_cmd->cSupM1 = (1 << Q_slope) / Diff;
  chroma_supp_cmd->cSupS1 = Q_slope - 7;

  mod->chroma_supp_update = TRUE;

  return VFE_SUCCESS;
} /* vfe_trigger_update_chroma_suppresion */
/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_trigger_enable -
 *
 * DESCRIPTION: This function updates the mce trigger enable flag
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_trigger_enable(int mod_id, void *mod_csupp,
  void *parms, int enable)
{
  chroma_supp_mod_t *mod = (chroma_supp_mod_t *)mod_csupp;
  vfe_params_t *vfe_params = (vfe_params_t *)parms;
  CDBG("%s:enable :%d\n",__func__, enable);
  mod->chroma_supp_trigger = enable;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_test_vector_validate(
  int mod_id,void *test_input, void *test_output)
{
  VFE_ChromaSuppress_ConfigCmdType *in, *out;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;

  CDBG("%s:\n", __func__);
  in = (VFE_ChromaSuppress_ConfigCmdType *)(mod_in->reg_dump +
    (V32_CHROMA_SUP_OFF/4));
  out = (VFE_ChromaSuppress_ConfigCmdType *)(mod_op->reg_dump_data +
    (V32_CHROMA_SUP_OFF/4));

  VALIDATE_TST_VEC(in->ySup1, out->ySup1, 0, "ySup1");
  VALIDATE_TST_VEC(in->ySup2, out->ySup2, 0, "ySup2");
  VALIDATE_TST_VEC(in->ySup3, out->ySup3, 0, "ySup3");
  VALIDATE_TST_VEC(in->ySup4, out->ySup4, 0, "ySup4");
  VALIDATE_TST_VEC(in->ySupM1, out->ySupM1, 0, "ySupM1");
  VALIDATE_TST_VEC(in->ySupM3, out->ySupM3, 0, "ySupM3");
  VALIDATE_TST_VEC(in->ySupS1, out->ySupS1, 0, "ySupS1");
  VALIDATE_TST_VEC(in->ySupS3, out->ySupS3, 0, "ySupS3");
  VALIDATE_TST_VEC(in->cSup1, out->cSup1, 0, "cSup1");
  VALIDATE_TST_VEC(in->cSup2, out->cSup2, 0, "cSup2");
  VALIDATE_TST_VEC(in->cSupM1, out->cSupM1, 0, "cSupM1");
  VALIDATE_TST_VEC(in->cSupS1, out->cSupS1, 0, "cSupS1");

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_deinit(int mod_id, void *mod_csupp,
  void *vparms)
{
    return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_chroma_subsample_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppress_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  chroma_supp_module_t *cmd = (chroma_supp_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->chroma_supp_cmd),
     sizeof(VFE_ChromaSuppress_ConfigCmdType),
     VFE_CMD_CHROMA_SUP_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
#endif
