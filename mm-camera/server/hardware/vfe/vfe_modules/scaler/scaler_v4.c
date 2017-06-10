/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_SCALER_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define VFE_DOWNSCALER_MN_FACTOR_OFFSET 13

#define VFE_MN_INIT 0

#define VEF_DOWNSCALER_MAX_SCALE_FACTOR 15

/*==============================================================================
 * FUNCTION    - scaler_cmd_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void scaler_cmd_debug(VFE_ScaleCfgCmdType* cmd)
{
  /* Y config */
  CDBG("y_scaler_cmd.hinputSize = %d\n", cmd->Y_ScaleCfg.hIn);
  CDBG("y_scaler_cmd.houtputSize = %d\n", cmd->Y_ScaleCfg.hOut);
  CDBG("y_scaler_cmd.vinputSize = %d\n", cmd->Y_ScaleCfg.vIn);
  CDBG("y_scaler_cmd.voutputSize = %d\n", cmd->Y_ScaleCfg.vOut);
  CDBG("y_scaler_cmd.hinterpolationResolution = %d\n",
    cmd->Y_ScaleCfg.horizInterResolution);
  CDBG("y_scaler_cmd.hphaseMultiplicationFactor = %u\n",
    cmd->Y_ScaleCfg.horizPhaseMult);
  CDBG("y_scaler_cmd.vinterpolationResolution = %d\n",
    cmd->Y_ScaleCfg.vertInterResolution);
  CDBG("y_scaler_cmd.vphaseMultiplicationFactor = %u\n",
    cmd->Y_ScaleCfg.vertPhaseMult);
  CDBG("y_scaler_cmd.horizMNInit = %d\n", cmd->Y_ScaleCfg.horizMNInit);
  CDBG("y_scaler_cmd.horizPhaseInit = %d\n", cmd->Y_ScaleCfg.horizPhaseInit);

  CDBG("y_scaler_cmd.vertMNInit = %d\n", cmd->Y_ScaleCfg.vertMNInit);
  CDBG("y_scaler_cmd.vertPhaseInit = %d\n", cmd->Y_ScaleCfg.vertPhaseInit);

  /* cbcr config*/
  CDBG("cbcr_scaler_cmd.hinputSize = %d\n", cmd->CbCr_ScaleCfg.hIn);
  CDBG("cbcr_scaler_cmd.houtputSize = %d\n", cmd->CbCr_ScaleCfg.hOut);
  CDBG("cbcr_scaler_cmd.vinputSize = %d\n", cmd->CbCr_ScaleCfg.vIn);
  CDBG("cbcr_scaler_cmd.voutputSize = %d\n", cmd->CbCr_ScaleCfg.vOut);
  CDBG("cbcr_scaler_cmd.hinterpolationResolution = %d\n",
    cmd->CbCr_ScaleCfg.horizInterResolution);
  CDBG("cbcr_scaler_cmd.hphaseMultiplicationFactor = %u\n",
    cmd->CbCr_ScaleCfg.horizPhaseMult);
  CDBG("cbcr_scaler_cmd.vinterpolationResolution = %d\n",
    cmd->CbCr_ScaleCfg.vertInterResolution );
  CDBG("cbcr_scaler_cmd.vphaseMultiplicationFactor = %u\n",
    cmd->CbCr_ScaleCfg.vertPhaseMult );

  CDBG("cbcr_scaler_cmd.horizMNInit = %d\n", cmd->CbCr_ScaleCfg.horizMNInit);
  CDBG("cbcr_scaler_cmd.horizPhaseInit = %d\n", cmd->CbCr_ScaleCfg.horizPhaseInit);

  CDBG("cbcr_scaler_cmd.vertMNInit = %d\n", cmd->CbCr_ScaleCfg.vertMNInit);
  CDBG("cbcr_scaler_cmd.vertPhaseInit = %d\n", cmd->CbCr_ScaleCfg.vertPhaseInit);

  CDBG("cbcr_scaler_cmd.ScaleCbCrInWidth = %d\n", cmd->CbCr_ScaleCfg.ScaleCbCrInWidth);
  CDBG("cbcr_scaler_cmd.HSkipCount = %d\n", cmd->CbCr_ScaleCfg.HSkipCount);
  CDBG("cbcr_scaler_cmd.RightPadEnable = %d\n", cmd->CbCr_ScaleCfg.RightPadEnable);

  CDBG("cbcr_scaler_cmd.ScaleCbCrInHeight = %d\n", cmd->CbCr_ScaleCfg.ScaleCbCrInHeight);
  CDBG("cbcr_scaler_cmd.VSkipCount = %d\n", cmd->CbCr_ScaleCfg.VSkipCount);
  CDBG("cbcr_scaler_cmd.BottomPadEnable = %d\n", cmd->CbCr_ScaleCfg.BottomPadEnable);

} /* scaler_cmd_debug */

/*===========================================================================
 * FUNCTION    - vfe_calculate_scaler_factor -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_calculate_scaler_factor(int scale_factor_horiz, int scale_factor_vert,
  int* hFactor, int* vFactor){
  vfe_status_t rc = VFE_SUCCESS;

  /* TODO : ZOOM needs to be taken care of*/
  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    CDBG("%s: Output1 scale larger than camsensor FOV, set scale_factor =1\n", __func__);
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }

  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    *hFactor = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    *hFactor = 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    *hFactor = 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz < 32) {
    *hFactor = 0;
  } else {
    CDBG("scale_factor_horiz is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }

  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    *vFactor = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    *vFactor = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    *vFactor = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert < 32) {
    *vFactor = 0;
  } else {
    CDBG("scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }

  return rc;
}/* vfe_calculate_scaler_factor */

/*===========================================================================
 * FUNCTION    - vfe_scaler_view_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_view_config(scaler_mod_t* scaler_mod, vfe_params_t* vfe_params)
{
  vfe_status_t rc = VFE_SUCCESS;
  VFE_Y_ScaleCfgCmdType* y_cfg = &scaler_mod->View_scaler_cmd.Y_ScaleCfg;
  VFE_CbCr_ScaleCfgCmdType* cbcr_cfg = &scaler_mod->View_scaler_cmd.CbCr_ScaleCfg;

  unsigned int cbcr_scale_factor_horiz, cbcr_scale_factor_vert;
  unsigned int y_scale_factor_horiz, y_scale_factor_vert;
  int cbcr_hFactor, cbcr_vFactor, y_hFactor, y_vFactor;
  int input_ratio, output_ratio;
  int is_cosited = 0;

  if (!scaler_mod->scaler_view_enable) {
    CDBG("%s: SCALER_view not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s: scaler_view start Y config \n", __func__);

  y_cfg->hEnable = TRUE;
  y_cfg->vEnable = TRUE;

  if (IS_BAYER_FORMAT(vfe_params))
    y_cfg->hIn =
      vfe_params->demosaic_op_params.last_pixel -
        vfe_params->demosaic_op_params.first_pixel + 1;
  else
    y_cfg->hIn =
      (vfe_params->demosaic_op_params.last_pixel -
        vfe_params->demosaic_op_params.first_pixel + 1) >> 1;

  y_cfg->vIn =
    vfe_params->demosaic_op_params.last_line -
      vfe_params->demosaic_op_params.first_line + 1;

  input_ratio = y_cfg->hIn * Q12 / y_cfg->vIn;
  output_ratio = vfe_params->output1w * Q12 / vfe_params->output1h;
  if (output_ratio == input_ratio) {
    y_cfg->hOut = vfe_params->output1w;
    y_cfg->vOut = vfe_params->output1h;
  } else if (output_ratio > input_ratio) {
    y_cfg->hOut = vfe_params->output1w;
    y_cfg->vOut = vfe_params->output1w * Q12 / input_ratio;
  } else {
    y_cfg->hOut = vfe_params->output1h * input_ratio / Q12;
    y_cfg->vOut = vfe_params->output1h;
  }

  /* make sure its even number so CbCr will match Y instead of rounding donw*/
  y_cfg->hOut = (y_cfg->hOut >> 1) << 1;
  y_cfg->vOut = (y_cfg->vOut >> 1) << 1;

  /* check for hw limitation, up to 32x for luma*/
  if ((y_cfg->vIn / (float) y_cfg->vOut) > VEF_DOWNSCALER_MAX_SCALE_FACTOR) {
    y_cfg->hOut = (uint32_t)y_cfg->hIn / VEF_DOWNSCALER_MAX_SCALE_FACTOR;
    y_cfg->vOut = (uint32_t)y_cfg->vIn / VEF_DOWNSCALER_MAX_SCALE_FACTOR;
  }

  y_scale_factor_horiz = y_cfg->hIn / y_cfg->hOut;
  y_scale_factor_vert = y_cfg->vIn / y_cfg->vOut;

  rc = vfe_calculate_scaler_factor(y_scale_factor_horiz,
    y_scale_factor_vert, &y_hFactor, &y_vFactor);

  y_cfg->horizInterResolution = y_hFactor;
  y_cfg->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_hFactor,
    (int32_t)y_cfg->hIn) / (int) y_cfg->hOut;

  y_cfg->vertInterResolution = y_vFactor;
  y_cfg->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->vIn) / (int) y_cfg->vOut;

  /*TODO: implement MN_INIT properly afterward*/
  y_cfg->horizMNInit = VFE_MN_INIT;
  y_cfg->horizPhaseInit =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_hFactor,
    (int32_t)y_cfg->horizMNInit) / (int) y_cfg->hOut;

  y_cfg->vertMNInit = VFE_MN_INIT;
  y_cfg->vertPhaseInit =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->vertMNInit) / (int) y_cfg->vOut;

    /* Output crop info for FOV_CROP module*/
  vfe_params->scaler_op_params[OUT_PREVIEW].first_pixel =
    vfe_params->demosaic_op_params.first_pixel;
  vfe_params->scaler_op_params[OUT_PREVIEW].last_pixel  =
    vfe_params->demosaic_op_params.first_pixel + y_cfg->hOut -1;
  vfe_params->scaler_op_params[OUT_PREVIEW].first_line =
    vfe_params->demosaic_op_params.first_line;
  vfe_params->scaler_op_params[OUT_PREVIEW].last_line =
    vfe_params->demosaic_op_params.first_line + y_cfg->vOut - 1;

  /*TODO: RIGHT_PAD_EN, H_SKIP_CNT and SCALE_CBCR_IN_WIDTH need to be taken care*/
  CDBG("%s: scaler_1 start CbCr config \n", __func__);

  cbcr_cfg->hEnable = TRUE;
  cbcr_cfg->vEnable = TRUE;
  cbcr_cfg->hIn = y_cfg->hIn;
  cbcr_cfg->vIn = y_cfg->vIn;

  if ((vfe_params->vfe_op_mode == VFE_OP_MODE_SNAPSHOT ||
    vfe_params->vfe_op_mode == VFE_OP_MODE_ZSL) &&
    (vfe_params->enc_format == CAMERA_YUV_422_NV61 ||
    vfe_params->enc_format == CAMERA_YUV_422_NV16)) {
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    is_cosited = 1;
  }else {
    cbcr_cfg->vOut = y_cfg->vOut / 2;
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    is_cosited = 0;
  }

  cbcr_scale_factor_horiz = cbcr_cfg->hIn / cbcr_cfg->hOut;
  cbcr_scale_factor_vert = cbcr_cfg->vIn / cbcr_cfg->vOut;

  rc = vfe_calculate_scaler_factor(cbcr_scale_factor_horiz,
    cbcr_scale_factor_vert, &cbcr_hFactor, &cbcr_vFactor);

  cbcr_cfg->horizInterResolution = cbcr_hFactor;
  cbcr_cfg->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_hFactor,
    (int32_t) cbcr_cfg->hIn) / (int) cbcr_cfg->hOut;

  cbcr_cfg->vertInterResolution = cbcr_vFactor;
  cbcr_cfg->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_vFactor,
    (int32_t) cbcr_cfg->vIn) / (int) cbcr_cfg->vOut;

   /* peter chroma subsampling config*/
  /* support even cosited phase (left padding) for now*/
  cbcr_cfg->RightPadEnable = 0;
  cbcr_cfg->HSkipCount = 0;
  cbcr_cfg->ScaleCbCrInWidth = cbcr_cfg->hIn;
  cbcr_cfg->BottomPadEnable = 0;
  cbcr_cfg->VSkipCount = 0;
  cbcr_cfg->ScaleCbCrInHeight = cbcr_cfg->vIn;

  /* if not cosited, CbCr init count  = luma init count*/
  /*TODO: implement MN_INIT when cosited to support NV16 and NV61*/
  if (is_cosited == 0) {
      cbcr_cfg->vertMNInit = VFE_MN_INIT;
      cbcr_cfg->vertPhaseInit =
        FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_vFactor,
        (int32_t)cbcr_cfg->vertMNInit) / (int) cbcr_cfg->vOut;
      cbcr_cfg->horizMNInit = VFE_MN_INIT;
      cbcr_cfg->horizPhaseInit =
        FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_hFactor,
        (int32_t)cbcr_cfg->horizMNInit) / (int) cbcr_cfg->hOut;
  } else {
    CDBG_HIGH("%s: not support cosited format yet", __func__);
  }

  scaler_cmd_debug(&scaler_mod->View_scaler_cmd);
  /* TODO: need to take care the hardware write after vfe40 kernel is done*/

  vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&scaler_mod->View_scaler_cmd, sizeof(scaler_mod->View_scaler_cmd),
    VFE_CMD_SCALER_VIEW_CFG);
  return rc;
} /* vfe_scaler_view_config */

/*===========================================================================
 * FUNCTION    - vfe_scaler_enc_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_enc_config(scaler_mod_t* scaler_mod, vfe_params_t* vfe_params)
{
  vfe_status_t rc = VFE_SUCCESS;
  VFE_Y_ScaleCfgCmdType* y_cfg = &scaler_mod->Enc_scaler_cmd.Y_ScaleCfg;
  VFE_CbCr_ScaleCfgCmdType* cbcr_cfg = &scaler_mod->Enc_scaler_cmd.CbCr_ScaleCfg;

  unsigned int cbcr_scale_factor_horiz, cbcr_scale_factor_vert;
  unsigned int y_scale_factor_horiz, y_scale_factor_vert;
  int cbcr_hFactor, cbcr_vFactor, y_hFactor, y_vFactor;
  int input_ratio, output_ratio;
  int is_cosited = 0;

  if (!scaler_mod->scaler_enc_enable) {
    CDBG("%s: SCALER_enc not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s: scaler_enc start Y config \n", __func__);

  y_cfg->hEnable = TRUE;
  y_cfg->vEnable = TRUE;

  if (IS_BAYER_FORMAT(vfe_params))
    y_cfg->hIn =
      vfe_params->demosaic_op_params.last_pixel -
        vfe_params->demosaic_op_params.first_pixel + 1;
  else
    y_cfg->hIn =
      (vfe_params->demosaic_op_params.last_pixel -
        vfe_params->demosaic_op_params.first_pixel + 1) >> 1;

  y_cfg->vIn =
    vfe_params->demosaic_op_params.last_line -
      vfe_params->demosaic_op_params.first_line + 1;

  input_ratio = y_cfg->hIn * Q12 / y_cfg->vIn;
  output_ratio = vfe_params->output2w * Q12 / vfe_params->output2h;
  if (output_ratio == input_ratio) {
    y_cfg->hOut = vfe_params->output2w;
    y_cfg->vOut = vfe_params->output2h;
  } else if (output_ratio > input_ratio) {
    y_cfg->hOut = vfe_params->output2w;
    y_cfg->vOut = vfe_params->output2w * Q12 / input_ratio;
  } else {
    y_cfg->hOut = vfe_params->output2h * input_ratio / Q12;
    y_cfg->vOut = vfe_params->output2h;
  }

    /* make sure its even number so CbCr will match Y instead of rounding donw*/
  y_cfg->hOut = (y_cfg->hOut >> 1) << 1;
  y_cfg->vOut = (y_cfg->vOut >> 1) << 1;

  /* check for hw limitation, up to 32x for luma*/
  if ((y_cfg->vIn / (float) y_cfg->vOut) > VEF_DOWNSCALER_MAX_SCALE_FACTOR) {
    y_cfg->hOut = (uint32_t)y_cfg->hIn / VEF_DOWNSCALER_MAX_SCALE_FACTOR;
    y_cfg->vOut = (uint32_t)y_cfg->vIn / VEF_DOWNSCALER_MAX_SCALE_FACTOR;
  }

  y_scale_factor_horiz = y_cfg->hIn / y_cfg->hOut;
  y_scale_factor_vert = y_cfg->vIn / y_cfg->vOut;

  rc = vfe_calculate_scaler_factor(y_scale_factor_horiz,
    y_scale_factor_vert, &y_hFactor, &y_vFactor);

  y_cfg->horizInterResolution = y_hFactor;
  y_cfg->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_hFactor,
    (int32_t)y_cfg->hIn) / (int) y_cfg->hOut;

  y_cfg->vertInterResolution = y_vFactor;
  y_cfg->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->vIn) / (int) y_cfg->vOut;

    /*TODO: implement MN_INIT properly afterward*/
  y_cfg->horizMNInit = VFE_MN_INIT;
  y_cfg->horizPhaseInit =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->horizMNInit) / (int) y_cfg->vOut;

  y_cfg->vertMNInit = VFE_MN_INIT;
  y_cfg->vertPhaseInit =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->vertMNInit) / (int) y_cfg->vOut;

  /* Output crop info for FOV_CROP module*/
  vfe_params->scaler_op_params[OUT_ENCODER].first_pixel =
    vfe_params->demosaic_op_params.first_pixel;
  vfe_params->scaler_op_params[OUT_ENCODER].last_pixel  =
    vfe_params->demosaic_op_params.first_pixel + y_cfg->hOut - 1;
  vfe_params->scaler_op_params[OUT_ENCODER].first_line =
    vfe_params->demosaic_op_params.first_line;
  vfe_params->scaler_op_params[OUT_ENCODER].last_line =
    vfe_params->demosaic_op_params.first_line + y_cfg->vOut - 1;

  /*TODO: RIGHT_PAD_EN, H_SKIP_CNT and SCALE_CBCR_IN_WIDTH need to be taken care*/
  CDBG("%s: scaler_enc start CbCr config \n", __func__);
  CDBG("%s: enc format = %d", __func__, vfe_params->enc_format);

  cbcr_cfg->hEnable  = TRUE;
  cbcr_cfg->vEnable = TRUE;
  cbcr_cfg->hIn = y_cfg->hIn;
  cbcr_cfg->vIn = y_cfg->vIn;

  if ((vfe_params->vfe_op_mode == VFE_OP_MODE_SNAPSHOT ||
    vfe_params->vfe_op_mode == VFE_OP_MODE_ZSL) &&
    (vfe_params->enc_format == CAMERA_YUV_422_NV61 ||
    vfe_params->enc_format == CAMERA_YUV_422_NV16)) {
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    is_cosited = 1;
  }else {
    cbcr_cfg->vOut = y_cfg->vOut / 2;
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    is_cosited = 0;
  }

  cbcr_scale_factor_horiz = cbcr_cfg->hIn / cbcr_cfg->hOut;
  cbcr_scale_factor_vert = cbcr_cfg->vIn / cbcr_cfg->vOut;

  rc = vfe_calculate_scaler_factor(cbcr_scale_factor_horiz,
    cbcr_scale_factor_vert, &cbcr_hFactor, &cbcr_vFactor);

  cbcr_cfg->horizInterResolution = cbcr_hFactor;
  cbcr_cfg->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_hFactor,
    (int32_t) cbcr_cfg->hIn) / (int) cbcr_cfg->hOut;

  cbcr_cfg->vertInterResolution = cbcr_vFactor;
  cbcr_cfg->vertPhaseMult = FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_vFactor,
    (int32_t) cbcr_cfg->vIn) / (int) cbcr_cfg->vOut;

  /* peter chroma subsampling config*/
  /* support even cosited phase (left padding) for now*/
  cbcr_cfg->RightPadEnable = 0;
  cbcr_cfg->HSkipCount = 0;
  cbcr_cfg->ScaleCbCrInWidth = cbcr_cfg->hIn;
  cbcr_cfg->BottomPadEnable = 0;
  cbcr_cfg->VSkipCount = 0;
  cbcr_cfg->ScaleCbCrInHeight = cbcr_cfg->vIn;

  /* if not cosited, CbCr init count  = luma init count*/
  /*TODO: implement MN_INIT when cosited to support NV16 and NV61*/
  if (is_cosited == 0) {
      cbcr_cfg->vertMNInit = VFE_MN_INIT;
      cbcr_cfg->vertPhaseInit =
        FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_vFactor,
        (int32_t)cbcr_cfg->vertMNInit) / (int) cbcr_cfg->vOut;
      cbcr_cfg->horizMNInit = VFE_MN_INIT;
      cbcr_cfg->horizPhaseInit =
        FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_hFactor,
        (int32_t)cbcr_cfg->horizMNInit) / (int) cbcr_cfg->hOut;
  } else {
    CDBG_HIGH("%s: not support cosited format yet", __func__);
  }
  scaler_cmd_debug(&scaler_mod->Enc_scaler_cmd);

  /* TODO: need to take care the hardware write after vfe40 kernel is done*/
  vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&scaler_mod->Enc_scaler_cmd, sizeof(scaler_mod->Enc_scaler_cmd),
    VFE_CMD_SCALER_ENC_CFG);

  return rc;
} /* vfe_scaler_enc_config */

/*===========================================================================
 * FUNCTION    - vfe_scaler_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_config(int mod_id, void* mod_sc,
  void* vparams)
{
  vfe_status_t rc = VFE_SUCCESS;
  scaler_mod_t* scaler_mod = (scaler_mod_t *)mod_sc;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;
  if (!scaler_mod->scaler_enc_enable && !scaler_mod->scaler_view_enable) {
    CDBG("%s: SCALER not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (scaler_mod->scaler_view_enable) {
    /* output 1 for preview, scaler 1 for preview*/
    CDBG("config view scaler");
    rc = vfe_scaler_view_config(scaler_mod, vfe_params);
    if (rc != VFE_SUCCESS) {
      CDBG("%s:failed",__func__);
      return rc;
    }
  }

  if (scaler_mod->scaler_enc_enable) {
    /* output 2 for encoder, scaler for encoder*/
    CDBG("config enc scaler \n");
    rc = vfe_scaler_enc_config(scaler_mod, vfe_params);
    if (rc != VFE_SUCCESS) {
      CDBG("%s:failed",__func__);
      return rc;
    }
  }
  return rc;
} /* vfe_scaler_config */

/*===========================================================================
 * FUNCTION    - vfe_scaler_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_update(int mod_id, void* mod_sc,
  void* vparams)
{
  vfe_status_t status;
  scaler_mod_t* scaler_mod = (scaler_mod_t *)mod_sc;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;

  if (!scaler_mod->scaler_enc_enable && !scaler_mod->scaler_view_enable) {
    CDBG("%s: SCALER not enabled", __func__);
    return VFE_SUCCESS;
  }

  if(scaler_mod->scaler_update) {
    status = vfe_scaler_config(mod_id, scaler_mod,vfe_params);
    if (status != VFE_SUCCESS)
      CDBG("%s: Failed\n",__func__);
    else
      vfe_params->update |= VFE_MOD_SCALER;
  }
  scaler_mod->scaler_update = FALSE;
  return VFE_SUCCESS;
}

/*===========================================================================
 * Function:           vfe_scaler_enc_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_scaler_enc_enable(int mod_id, void* mod_sc,
  void* vparams, int8_t enable, int8_t hw_write)
{
  /* TODO: need to add function call for this func*/
  scaler_mod_t* scaler_mod = (scaler_mod_t *)mod_sc;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;
  CDBG("%s: enable = %d\n", __func__, enable);
  vfe_params->moduleCfg->scalerEncEnable = enable;

  if (hw_write && (scaler_mod->scaler_enc_enable == enable))
    return VFE_SUCCESS;

  scaler_mod->scaler_enc_enable = enable;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_SCALER_ENC)
      : (vfe_params->current_config & ~VFE_MOD_SCALER_ENC);
  }
  return VFE_SUCCESS;
} /* vfe_scaler_enc_enable */

/*===========================================================================
 * Function:           vfe_scaler_view_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_scaler_view_enable(int mod_id, void* mod_sc,
  void* vparams, int8_t enable, int8_t hw_write)
{
  scaler_mod_t* scaler_mod = (scaler_mod_t *)mod_sc;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;

  CDBG("%s: enable = %d\n", __func__, enable);
  vfe_params->moduleCfg->scalerViewEnable = enable;

  if (hw_write && (scaler_mod->scaler_view_enable == enable))
    return VFE_SUCCESS;

  scaler_mod->scaler_view_enable = enable;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_SCALER)
      : (vfe_params->current_config & ~VFE_MOD_SCALER);
  }
  return VFE_SUCCESS;
} /* vfe_scaler_view_enable */

/*===========================================================================
 * Function:           vfe_scaler_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_scaler_enable(int mod_id, void* mod_sc,
  void* vparams, int8_t enable, int8_t hw_write)
{
  scaler_mod_t* scaler_mod = (scaler_mod_t *)mod_sc;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;

  /* the first scaler always config encoder scaler to match Write Master setting*/
  if (vfe_params->vfe_op_mode == VFE_OP_MODE_PREVIEW) {
    vfe_scaler_enc_enable(mod_id, mod_sc, vparams, enable, hw_write);
  } else if (vfe_params->vfe_op_mode == VFE_OP_MODE_RAW_SNAPSHOT) {
    vfe_scaler_enc_enable(mod_id, mod_sc, vparams, enable, hw_write);
  }else {
    vfe_scaler_view_enable(mod_id, mod_sc, vparams, enable, hw_write);
    vfe_scaler_enc_enable(mod_id, mod_sc, vparams, enable, hw_write);
  }

  return VFE_SUCCESS;
} /* vfe_scaler_enable */


/*===========================================================================
 * Function:           vfe_set_scaler
 *
 * Description:   TO DO: to implement this for zoom
 *=========================================================================*/
vfe_status_t vfe_set_scaler(void *params)
{
  return VFE_SUCCESS;
} /* vfe_set_scaler */
/*===========================================================================
 * FUNCTION    - vfe_scaler_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_scaler_deinit(int mod_id, void *module, void *params)
{
  scaler_mod_t *scaler_mod = (scaler_mod_t *)module;
  vfe_params_t* vfe_params = (vfe_params_t *)params;
  memset(scaler_mod, 0 , sizeof(scaler_mod_t));
  return VFE_SUCCESS;
} /*vfe_scaler_deinit*/
