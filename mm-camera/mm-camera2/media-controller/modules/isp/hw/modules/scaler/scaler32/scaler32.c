/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "scaler32.h"
#include "isp_log.h"

#ifdef ENABLE_SCALER_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#define VFE_DOWNSCALER_MN_FACTOR_OFFSET 13

/*==============================================================================
 * FUNCTION    - main_scaler_cmd_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void main_scaler_cmd_debug(ISP_Main_Scaler_ConfigCmdType* cmd)
{
  ISP_DBG(ISP_MOD_SCALER, "%s: hEnable = %d vEnable = %d", __func__, cmd->hEnable, cmd->vEnable);
  ISP_DBG(ISP_MOD_SCALER, "%s: inWidth = %d outWidth = %d", __func__, cmd->inWidth, cmd->outWidth);
  ISP_DBG(ISP_MOD_SCALER, "%s: horizPhaseMult = %d horizInterResolution = %d", __func__,
    cmd->horizPhaseMult, cmd->horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: horizMNInit = %d horizPhaseInit = %d", __func__,
    cmd->horizMNInit, cmd->horizPhaseInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: inHeight = %d outHeight = %d", __func__,
    cmd->inHeight, cmd->outHeight);
  ISP_DBG(ISP_MOD_SCALER, "%s: vertPhaseMult = %d vertInterResolution = %d", __func__,
    cmd->vertPhaseMult, cmd->vertInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: vertMNInit = %d vertPhaseInit = %d", __func__,
    cmd->vertMNInit, cmd->vertPhaseInit);
} /* main_scaler_cmd_debug */

/*==============================================================================
 * FUNCTION    - y_scaler_cmd_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void y_scaler_cmd_debug(ISP_Output_YScaleCfgCmdType *cmd)
{
  ISP_DBG(ISP_MOD_SCALER, "%s: hEnable = %d vEnable = %d", __func__, cmd->hEnable, cmd->vEnable);
  ISP_DBG(ISP_MOD_SCALER, "%s: hIn = %d hOut = %d", __func__, cmd->hIn, cmd->hOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: horizPhaseMult = %d horizInterResolution = %d", __func__,
    cmd->horizPhaseMult, cmd->horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: vIn = %d vOut = %d", __func__, cmd->vIn, cmd->vOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: vertPhaseMult = %d vertInterResolution = %d", __func__,
    cmd->vertPhaseMult, cmd->vertInterResolution);
} /* y_scaler_cmd_debug */

/*==============================================================================
 * FUNCTION    - cbcr_scaler_cmd_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void cbcr_scaler_cmd_debug(ISP_Output_CbCrScaleCfgCmdType *cmd)
{
  ISP_DBG(ISP_MOD_SCALER, "%s: hEnable = %d vEnable = %d", __func__, cmd->hEnable, cmd->vEnable);
  ISP_DBG(ISP_MOD_SCALER, "%s: hIn = %d hOut = %d", __func__, cmd->hIn, cmd->hOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: horizPhaseMult = %d horizInterResolution = %d", __func__,
    cmd->horizPhaseMult, cmd->horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: vIn = %d vOut = %d", __func__, cmd->vIn, cmd->vOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: vertPhaseMult = %d vertInterResolution = %d", __func__,
    cmd->vertPhaseMult, cmd->vertInterResolution);
} /* cbcr_scaler_cmd_debug */

/*===========================================================================
 * FUNCTION    - main_scaler_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int main_scaler_config(ISP_Main_Scaler_ConfigCmdType *main_scaler_cmd,
  isp_hw_pix_setting_params_t *pix_setting)
{
  int rc = 0;
  uint32_t input_width, input_height,output_width, output_height;
  unsigned int scale_factor_horiz, scale_factor_vert;

  input_width = pix_setting->crop_info.y.pix_line.last_pixel -
    pix_setting->crop_info.y.pix_line.first_pixel + 1;

  input_height = pix_setting->crop_info.y.pix_line.last_line -
    pix_setting->crop_info.y.pix_line.first_line + 1;

  /* todo_bug_fix: hardcoding to index 0 or 1 is wrong and will not work */
  output_width = pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.width;
  output_height = pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.height;

  int hFactor = 0;
  int vFactor = 0;

  CDBG_HIGH("%s: ip_w=%d, ip_h=%d", __func__, input_width, input_height);
  CDBG_HIGH("%s: op_w=%d, op_h=%d", __func__, output_width, output_height);

  scale_factor_horiz = input_width / output_width;
  scale_factor_vert = input_height / output_height;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(ISP_MOD_SCALER, "Output2 is configure larger than camsensor FOV.\n");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }

  main_scaler_cmd->hEnable = TRUE;
  main_scaler_cmd->vEnable = TRUE;
  main_scaler_cmd->inWidth   = input_width;
  main_scaler_cmd->inHeight  = input_height;
  main_scaler_cmd->outWidth  = output_width;
  main_scaler_cmd->outHeight = output_height;

  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    hFactor = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    hFactor = 2;
    main_scaler_cmd->horizPhaseMult += 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    hFactor = 1;
    main_scaler_cmd->horizPhaseMult += 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz < 32) {
    hFactor = 0;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_horiz is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  main_scaler_cmd->horizInterResolution  = hFactor;
  main_scaler_cmd->horizPhaseMult        =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + hFactor,
    (int32_t)input_width) / output_width;


  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    vFactor = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    vFactor = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    vFactor = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert < 32) {
    vFactor = 0;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  main_scaler_cmd->vertInterResolution    = vFactor;
  main_scaler_cmd->vertPhaseMult          =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + vFactor,
    (int32_t)input_height) / output_height;

  ISP_DBG(ISP_MOD_SCALER, "%s: main_scaler_cmd.inWidth = %d\n", __func__,
    main_scaler_cmd->inWidth);
  ISP_DBG(ISP_MOD_SCALER, "%s: main_scaler_cmd.outWidth = %d\n", __func__,
    main_scaler_cmd->outWidth);
  ISP_DBG(ISP_MOD_SCALER, "%s: main_scaler_cmd.inHeight = %d\n", __func__,
    main_scaler_cmd->inHeight);
  ISP_DBG(ISP_MOD_SCALER, "%s: main_scaler_cmd.outHeight = %d\n", __func__,
    main_scaler_cmd->outHeight);
  ISP_DBG(ISP_MOD_SCALER, "%s: main_scaler_cmd.horizInterResolution = %d\n", __func__,
    main_scaler_cmd->horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s main_scaler_cmd.horizPhaseMult = %u\n", __func__,
    main_scaler_cmd->horizPhaseMult);
  ISP_DBG(ISP_MOD_SCALER, "%s main_scaler_cmd.vertInterResolution = %d\n", __func__,
    main_scaler_cmd->vertInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: main_scaler_cmd.vertPhaseMult = %u\n", __func__,
    main_scaler_cmd->vertPhaseMult);

  return rc;
} /* main_scaler_config */

/*===========================================================================
 * FUNCTION    - Output1_YScaleCfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int y_scaler_config(ISP_Output_YScaleCfgCmdType *y_scaler_cmd,
  isp_hw_pix_setting_params_t *pix_setting)
{
  //vfe_cmd_id cmd = vfe_util_is_vfe_started() ?
  //  VFE_CMD_ID_SCALER2Y_UPDATE : VFE_CMD_ID_SCALER2Y_CONFIG;
  int rc = 0;
  uint32_t output1_width,output1_height,output2_width,output2_height;
  unsigned int scale_factor_horiz,scale_factor_vert;
  int hFactor = 0;
  int vFactor = 0;

  output1_width = pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.width;
  output1_height = pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.height;
  output2_width = pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.width;
  output2_height = pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.height;

  scale_factor_horiz = output2_width / output1_width;
  scale_factor_vert = output2_height / output1_height;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(ISP_MOD_SCALER, "Output2 is configure larger than camsensor FOV.\n");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }

  y_scaler_cmd->hEnable = TRUE;
  y_scaler_cmd->vEnable = TRUE;
  y_scaler_cmd->hIn = output2_width;
  y_scaler_cmd->vIn = output2_height;
  y_scaler_cmd->hOut = output1_width;
  y_scaler_cmd->vOut = output1_height;

  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    hFactor = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    hFactor = 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    hFactor = 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz < 32) {
    hFactor = 0;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_horiz is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  y_scaler_cmd->horizInterResolution = hFactor;
  y_scaler_cmd->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + hFactor,
    (int32_t)output2_width) / output1_width;

  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    vFactor = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    vFactor = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    vFactor = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert < 32) {
    vFactor = 0;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  y_scaler_cmd->vertInterResolution = vFactor;
  y_scaler_cmd->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + vFactor,
    (int32_t)output2_height) / output1_height;

  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.hinputSize = %d\n",y_scaler_cmd->hIn);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.houtputSize = %d\n",y_scaler_cmd->hOut);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.vinputSize = %d\n",y_scaler_cmd->vIn);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.voutputSize = %d\n",y_scaler_cmd->vOut);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.hinterpolationResolution = %d\n",
    y_scaler_cmd->horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.hphaseMultiplicationFactor = %u\n",
    y_scaler_cmd->horizPhaseMult);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.vinterpolationResolution = %d\n",
    y_scaler_cmd->vertInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "y_scaler_cmd.vphaseMultiplicationFactor = %u\n",
    y_scaler_cmd->vertPhaseMult);

  return rc;
} /* vfe_y_scaler_config */

/*===========================================================================
 * FUNCTION    - cbcr_scaler_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int cbcr_scaler_config(ISP_Output_CbCrScaleCfgCmdType *cbcr_scaler_cmd,
  isp_hw_pix_setting_params_t *pix_setting)
{
  //vfe_cmd_id cmd = vfe_util_is_vfe_started() ?
  //  VFE_CMD_ID_SCALER2CbCr_UPDATE : VFE_CMD_ID_SCALER2CbCr_CONFIG;
  int rc = 0;
  unsigned int scale_factor_horiz, scale_factor_vert;
  int hFactor = 0;
  int vFactor = 0;
  int nViewfinderOutWidth;
  int nViewfinderOutHeight;

  cbcr_scaler_cmd->hEnable = TRUE;
  cbcr_scaler_cmd->vEnable = TRUE;
  cbcr_scaler_cmd->hIn = pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.width;
  cbcr_scaler_cmd->vIn = pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.height;
  nViewfinderOutWidth  = pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.width;
  nViewfinderOutHeight = pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.height;

  switch (pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.fmt) {
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16:
    cbcr_scaler_cmd->hOut = (nViewfinderOutWidth + 1) / 2;
    cbcr_scaler_cmd->vOut = nViewfinderOutHeight;
    break;
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV12_VENUS:
    cbcr_scaler_cmd->hOut = (nViewfinderOutWidth + 1) / 2;
    cbcr_scaler_cmd->vOut = (nViewfinderOutHeight + 1) / 2;
    break;
  default:
    CDBG_ERROR("%s: ERROR: Not supported image format: %d",
      __func__, pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.fmt);
    break;
  }

  scale_factor_horiz = cbcr_scaler_cmd->hIn / cbcr_scaler_cmd->hOut;
  scale_factor_vert = cbcr_scaler_cmd->vIn / cbcr_scaler_cmd->vOut;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(ISP_MOD_SCALER, "Output2 is configure larger than camsensor FOV.\n");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }
  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    hFactor = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    hFactor = 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    hFactor = 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz < 32) {
    hFactor = 0;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_horiz is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }

  cbcr_scaler_cmd->horizInterResolution = hFactor;
  cbcr_scaler_cmd->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + hFactor,
    (int32_t) cbcr_scaler_cmd->hIn) / (int) cbcr_scaler_cmd->hOut;

  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    vFactor = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    vFactor = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    vFactor = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert < 32) {
    vFactor = 0;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  cbcr_scaler_cmd->vertInterResolution = vFactor;
  cbcr_scaler_cmd->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + vFactor,
    (int32_t) cbcr_scaler_cmd->vIn) / (int) cbcr_scaler_cmd->vOut;

  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.hinputSize = %d\n", cbcr_scaler_cmd->hIn);
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.houtputSize = %d\n", cbcr_scaler_cmd->hOut);
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.vinputSize = %d\n", cbcr_scaler_cmd->vIn);
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.voutputSize = %d\n", cbcr_scaler_cmd->vOut);
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.hinterpolationResolution = %d\n",
    cbcr_scaler_cmd->horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.hphaseMultiplicationFactor = %u\n",
    cbcr_scaler_cmd->horizPhaseMult);
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.vinterpolationResolution = %d\n",
    cbcr_scaler_cmd->vertInterResolution );
  ISP_DBG(ISP_MOD_SCALER, "cbcr_scaler_cmd.vphaseMultiplicationFactor = %u\n",
    cbcr_scaler_cmd->vertPhaseMult );

  return rc;
} /* cbcr_scaler_config */

/*==================================================*/

/* ============================================================
 * function name: scaler_trigger_enable
 * description: enable scaler trigger update feature
 * ============================================================*/
static int scaler_trigger_enable(isp_scaler_mod_t *scaler,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  scaler->trigger_enable = enable->enable;
  return 0;
}
/* ============================================================
 * function name: scaler_enable
 * description: enable scaler
 * ============================================================*/
static int scaler_enable(isp_scaler_mod_t *scaler,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  int i;

  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  scaler->enable = enable->enable;

  return 0;
}

/* ============================================================
 * function name: scaler_config
 * description: config scaler
 * ============================================================*/
static int scaler_config(isp_scaler_mod_t *scaler_mod,
                         isp_hw_pix_setting_params_t *pix_setting,
                         uint32_t size)
{
  int i, rc = 0;

  if (sizeof(isp_hw_pix_setting_params_t) != size) {
    CDBG_ERROR("%s: in_params size mismatch\n", __func__);
    return -EINVAL;
  }
  if (!scaler_mod || !pix_setting) {
    CDBG_ERROR("%s: invalid input", __func__);
    return -EINVAL;
  }

  if (!scaler_mod->enable) {
    ISP_DBG(ISP_MOD_SCALER, "%s: not enabled", __func__);
    /* not enabled no op */
    return rc;
  }

  ISP_DBG(ISP_MOD_SCALER, "%s: <<<camera>>> calling main_scaler_config", __func__);

  /* main scaler is always on */
  rc = main_scaler_config(&(scaler_mod->main_scaler_cmd), pix_setting);
  if (rc != 0) {
    CDBG_ERROR("%s:failed",__func__);
    scaler_mod->main_scaler_hw_update_pending = FALSE;
    return rc;
  }
  scaler_mod->main_scaler_hw_update_pending = TRUE;

  scaler_mod->scaling_factor =
      (float) (pix_setting->crop_info.y.pix_line.last_pixel -
      pix_setting->crop_info.y.pix_line.first_pixel + 1) /
      (float)pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.width;

  /* todo_bug_fix? */
  if (pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.width &&
    pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.height) {
    //TODO: how to get the number of vfe output??

    ISP_DBG(ISP_MOD_SCALER, "%s SECONDARY SCALAR ENABLED ", __func__);
    /* secondary scaler is only on when output path 1 is enabled. */
    //TODO: Need to check if Media controller is going to provide
    //output path and any check need to be performed for secondary scaler
    rc = y_scaler_config(&(scaler_mod->y_scaler_cmd),pix_setting);
    if (rc != 0) {
      ISP_DBG(ISP_MOD_SCALER, "%s:failed",__func__);
      return rc;
    } else {
      scaler_mod->Y_scaler_hw_update_pending = TRUE;
    }
    ISP_DBG(ISP_MOD_SCALER, "sent yscale cfg comand \n");

    rc = cbcr_scaler_config(&(scaler_mod->cbcr_scaler_cmd), pix_setting);
    if (rc != 0) {
      ISP_DBG(ISP_MOD_SCALER, "%s:failed",__func__);
      return rc;
    } else {
       scaler_mod->CbCr_scaler_hw_update_pending = TRUE;
    }
    ISP_DBG(ISP_MOD_SCALER, "sent CbCr scalar cfg comand \n");

    scaler_mod->scaling_factor *=
        (float)pix_setting->outputs[ISP_PIX_PATH_ENCODER].stream_param.width /
        (float)pix_setting->outputs[ISP_PIX_PATH_VIEWFINDER].stream_param.width;
  } else {
    ISP_DBG(ISP_MOD_SCALER, "%s SECONDARY SCALAR DISABLED ", __func__);
  }

  return rc;
}
/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_set_zoom_ratio(isp_scaler_mod_t *scaler,
  isp_hw_pix_setting_params_t *in_params, uint32_t size)
{
  return scaler_config(scaler, in_params, size);

}
/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_trigger_update(isp_scaler_mod_t *scaler,
  isp_pix_trigger_update_input_t *params, uint32_t in_param_size)
{
  int rc = 0, i;

  if (!scaler->trigger_enable || !scaler->enable) {
    ISP_DBG(ISP_MOD_SCALER, "%s: SCALER trigger update not enabled", __func__);
    return 0;
  }
  rc = scaler_config(scaler, &params->cfg, sizeof(params->cfg));
  return rc;

}

/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_destroy (void *mod_ctrl)
{
  isp_scaler_mod_t *scaler = mod_ctrl;

  if (scaler) {
    memset(scaler, 0, sizeof(isp_scaler_mod_t));
    free(scaler);
  }
  return 0;
}

/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_set_params (void *mod_ctrl, uint32_t params_id,
  void *in_params, uint32_t in_params_size)
{
  isp_scaler_mod_t *scaler = mod_ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = scaler_enable(scaler, (isp_mod_set_enable_t *)in_params,
      in_params_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = scaler_config(scaler, (isp_hw_pix_setting_params_t *)in_params,
      in_params_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = scaler_trigger_enable(scaler, (isp_mod_set_enable_t *)in_params,
      in_params_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = scaler_trigger_update(scaler,
      (isp_pix_trigger_update_input_t *)in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_ZOOM_RATIO:
    rc = scaler_set_zoom_ratio(scaler,
      (isp_hw_pix_setting_params_t *)in_params, in_params_size);
    break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    rc = -EAGAIN;
    break;
  }
  return rc;
}

/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_get_params (void *mod_ctrl, uint32_t params_id,
  void *in_params, uint32_t in_params_size, void *out_params,
  uint32_t out_params_size)
{
  isp_scaler_mod_t *scaler = mod_ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = (isp_mod_get_enable_t *)out_params;

    if (sizeof(isp_mod_get_enable_t) != out_params_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d", __func__,
        params_id);
      break;
    }
    enable->enable = scaler->enable;
    break;
  }
  case ISP_PIX_GET_SCALER_OUTPUT: {
    isp_pixel_window_info_t *scaler_output = out_params;
    scaler_output->height = scaler->main_scaler_cmd.outWidth;
    scaler_output->width = scaler->main_scaler_cmd.outHeight;
    scaler_output->scaling_factor = scaler->scaling_factor;
    break;
  }
  default:
    rc = -EAGAIN;
    break;
  }
  return rc;
}

/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_do_hw_update(isp_scaler_mod_t *scaler_mod)
{
  int i, rc = 0;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  ISP_DBG(ISP_MOD_SCALER, "%s: HW_update = %d\n", __func__,
    scaler_mod->main_scaler_hw_update_pending);

  if (scaler_mod->main_scaler_hw_update_pending == TRUE) {
    main_scaler_cmd_debug(&scaler_mod->main_scaler_cmd);

    cfg_cmd.cfg_data = (void *) &scaler_mod->main_scaler_cmd;
    cfg_cmd.cmd_len = sizeof(scaler_mod->main_scaler_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_SCALER32_MAIN_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_SCALER32_MAIN_LEN * sizeof(uint32_t);

    rc = ioctl(scaler_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: main scaler HW update error, rc = %d", __func__, rc);
      return rc;
    }

    scaler_mod->main_scaler_hw_update_pending = 0;
  }

  if (scaler_mod->Y_scaler_hw_update_pending == TRUE) {
    y_scaler_cmd_debug(&scaler_mod->y_scaler_cmd);

    cfg_cmd.cfg_data = (void *) &scaler_mod->y_scaler_cmd;
    cfg_cmd.cmd_len = sizeof(scaler_mod->y_scaler_cmd);
    cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_SCALER32_Y_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_SCALER32_Y_LEN * sizeof(uint32_t);

    rc = ioctl(scaler_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
       CDBG_ERROR("%s: Y scaler HW update error, rc = %d", __func__, rc);
       return rc;
    }

    scaler_mod->Y_scaler_hw_update_pending = 0;
  }

  if (scaler_mod->CbCr_scaler_hw_update_pending == TRUE) {
    cbcr_scaler_cmd_debug(&scaler_mod->cbcr_scaler_cmd);

    cfg_cmd.cfg_data = (void *) &scaler_mod->cbcr_scaler_cmd;
    cfg_cmd.cmd_len = sizeof(scaler_mod->cbcr_scaler_cmd);
    cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_SCALER32_CBCR_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_SCALER32_CBCR_LEN * sizeof(uint32_t);

    rc = ioctl(scaler_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: cbcr scaler HW update error, rc = %d", __func__, rc);
      return rc;
    }

    scaler_mod->CbCr_scaler_hw_update_pending = 0;
  }

  return rc;
}

/* ============================================================================
 * function name: scaler_action
 * description: processing the action
 * ==========================================================================*/
static int scaler_action (void *mod_ctrl, uint32_t action_code,
                          void *action_data, uint32_t action_data_size)
{
  int rc = 0;
  isp_scaler_mod_t *scaler = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = scaler_do_hw_update(scaler);
    break;
  default:
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported", __func__, action_code);
    break;
  }
  return rc;
}

/* ============================================================
 * function name: scaler_init
 * description: init scaler
 * ============================================================*/
static int scaler_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_scaler_mod_t *scaler = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  scaler->fd = init_params->fd;
  scaler->notify_ops = notify_ops;
  return 0;
}

/* ============================================================
 * function name: scaler32_open
 * description: open scaler
 * ============================================================*/
isp_ops_t *scaler32_open(uint32_t version)
{
  isp_scaler_mod_t *scaler = malloc(sizeof(isp_scaler_mod_t));

  if (!scaler) {
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }

  memset(scaler,  0,  sizeof(isp_scaler_mod_t));
  scaler->ops.ctrl = (void *)scaler;
  scaler->ops.init = scaler_init;
  scaler->ops.destroy = scaler_destroy;
  scaler->ops.set_params = scaler_set_params;
  scaler->ops.get_params = scaler_get_params;
  scaler->ops.action = scaler_action;

  return &scaler->ops;
}
