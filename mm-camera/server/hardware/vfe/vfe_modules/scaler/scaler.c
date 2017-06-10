/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
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

/*===========================================================================
 * FUNCTION    - vfe_main_scaler_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_main_scaler_config(VFE_Main_Scaler_ConfigCmdType *main_scaler_cmd,
  vfe_params_t *vfe_params)
{
  vfe_status_t rc = VFE_SUCCESS;
  uint32_t input_width, input_height,output_width, output_height;
  unsigned int scale_factor_horiz, scale_factor_vert;

  input_width = vfe_params->crop_info.crop_last_pixel - vfe_params->crop_info.crop_first_pixel + 1;
  input_height = vfe_params->crop_info.crop_last_line - vfe_params->crop_info.crop_first_line + 1;
  output_width = vfe_params->output2w;
  output_height = vfe_params->output2h;
  //vfe_cmd_id cmd = vfe_util_is_vfe_started() ?
  //  VFE_CMD_ID_MAIN_SCALER_UPDATE : VFE_CMD_ID_MAIN_SCALER_CONFIG;
  int hFactor = 0;
  int vFactor = 0;

  scale_factor_horiz = input_width / output_width;
  scale_factor_vert = input_height / output_height;

  /* Save this value for sharpness control */
  vfe_params->sharpness_info.downscale_factor =
    1.0 * ((float)input_width / (float)output_width);

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    CDBG("Output2 is configure larger than camsensor FOV.\n");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
    vfe_params->sharpness_info.downscale_factor = 1.0;
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
    CDBG("scale_factor_horiz is greater than 32, which is more than "
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
    CDBG("scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  main_scaler_cmd->vertInterResolution    = vFactor;
  main_scaler_cmd->vertPhaseMult          =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + vFactor,
    (int32_t)input_height) / output_height;

  CDBG("%s: main_scaler_cmd.inWidth = %d\n", __func__,
    main_scaler_cmd->inWidth);
  CDBG("%s: main_scaler_cmd.outWidth = %d\n", __func__,
    main_scaler_cmd->outWidth);
  CDBG("%s: main_scaler_cmd.inHeight = %d\n", __func__,
    main_scaler_cmd->inHeight);
  CDBG("%s: main_scaler_cmd.outHeight = %d\n", __func__,
    main_scaler_cmd->outHeight);
  CDBG("%s: main_scaler_cmd.horizInterResolution = %d\n", __func__,
    main_scaler_cmd->horizInterResolution);
  CDBG("%s main_scaler_cmd.horizPhaseMult = %u\n", __func__,
    main_scaler_cmd->horizPhaseMult);
  CDBG("%s main_scaler_cmd.vertInterResolution = %d\n", __func__,
    main_scaler_cmd->vertInterResolution);
  CDBG("%s: main_scaler_cmd.vertPhaseMult = %u\n", __func__,
    main_scaler_cmd->vertPhaseMult);

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL, (void *)main_scaler_cmd,
    sizeof(*main_scaler_cmd), VFE_CMD_MAIN_SCALER_CFG);

  return rc;
} /* vfe_main_scaler_config */

/*===========================================================================
 * FUNCTION    - VFE_Output1_YScaleCfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_y_scaler_config(VFE_Output_YScaleCfgCmdType *y_scaler_cmd,
  vfe_params_t *vfe_params)
{
  //vfe_cmd_id cmd = vfe_util_is_vfe_started() ?
  //  VFE_CMD_ID_SCALER2Y_UPDATE : VFE_CMD_ID_SCALER2Y_CONFIG;
  vfe_status_t rc = VFE_SUCCESS;
  uint32_t output1_width,output1_height,output2_width,output2_height;
  unsigned int scale_factor_horiz,scale_factor_vert;
  int hFactor = 0;
  int vFactor = 0;
  output1_width = vfe_params->output1w;
  output1_height = vfe_params->output1h;
  output2_width = vfe_params->output2w;
  output2_height = vfe_params->output2h;

  scale_factor_horiz = output2_width / output1_width;
  scale_factor_vert = output2_height / output1_height;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    CDBG("Output2 is configure larger than camsensor FOV.\n");
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
    CDBG("scale_factor_horiz is greater than 32, which is more than "
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
    CDBG("scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  y_scaler_cmd->vertInterResolution = vFactor;
  y_scaler_cmd->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + vFactor,
    (int32_t)output2_height) / output1_height;

  CDBG("y_scaler_cmd.hinputSize = %d\n",y_scaler_cmd->hIn);
  CDBG("y_scaler_cmd.houtputSize = %d\n",y_scaler_cmd->hOut);
  CDBG("y_scaler_cmd.vinputSize = %d\n",y_scaler_cmd->vIn);
  CDBG("y_scaler_cmd.voutputSize = %d\n",y_scaler_cmd->vOut);
  CDBG("y_scaler_cmd.hinterpolationResolution = %d\n",
    y_scaler_cmd->horizInterResolution);
  CDBG("y_scaler_cmd.hphaseMultiplicationFactor = %u\n",
    y_scaler_cmd->horizPhaseMult);
  CDBG("y_scaler_cmd.vinterpolationResolution = %d\n",
    y_scaler_cmd->vertInterResolution);
  CDBG("y_scaler_cmd.vphaseMultiplicationFactor = %u\n",
    y_scaler_cmd->vertPhaseMult);

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL, (void *)y_scaler_cmd,
    sizeof(*y_scaler_cmd), VFE_CMD_S2Y_CFG);

  return rc;
} /* vfe_y_scaler_config */

/*===========================================================================
 * FUNCTION    - vfe_cbcr_scaler_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_cbcr_scaler_config(VFE_Output_CbCrScaleCfgCmdType *cbcr_scaler_cmd,
  vfe_params_t *vfe_params)
{
  //vfe_cmd_id cmd = vfe_util_is_vfe_started() ?
  //  VFE_CMD_ID_SCALER2CbCr_UPDATE : VFE_CMD_ID_SCALER2CbCr_CONFIG;
  vfe_status_t rc = VFE_SUCCESS;
  unsigned int scale_factor_horiz, scale_factor_vert;
  int hFactor = 0;
  int vFactor = 0;

  cbcr_scaler_cmd->hEnable  = TRUE;
  cbcr_scaler_cmd->vEnable = TRUE;
  cbcr_scaler_cmd->hIn = vfe_params->output2w;
  cbcr_scaler_cmd->vIn = vfe_params->output2h;
  cbcr_scaler_cmd->hOut = (vfe_params->output1w + 1) / 2;
  cbcr_scaler_cmd->vOut = (vfe_params->output1h + 1) / 2;

  scale_factor_horiz = cbcr_scaler_cmd->hIn / cbcr_scaler_cmd->hOut;
  scale_factor_vert = cbcr_scaler_cmd->vIn / cbcr_scaler_cmd->vOut;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    CDBG("Output2 is configure larger than camsensor FOV.\n");
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
    CDBG("scale_factor_horiz is greater than 32, which is more than "
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
    CDBG("scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }
  cbcr_scaler_cmd->vertInterResolution = vFactor;
  cbcr_scaler_cmd->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + vFactor,
    (int32_t) cbcr_scaler_cmd->vIn) / (int) cbcr_scaler_cmd->vOut;

  CDBG("cbcr_scaler_cmd.hinputSize = %d\n", cbcr_scaler_cmd->hIn);
  CDBG("cbcr_scaler_cmd.houtputSize = %d\n", cbcr_scaler_cmd->hOut);
  CDBG("cbcr_scaler_cmd.vinputSize = %d\n", cbcr_scaler_cmd->vIn);
  CDBG("cbcr_scaler_cmd.voutputSize = %d\n", cbcr_scaler_cmd->vOut);
  CDBG("cbcr_scaler_cmd.hinterpolationResolution = %d\n",
    cbcr_scaler_cmd->horizInterResolution);
  CDBG("cbcr_scaler_cmd.hphaseMultiplicationFactor = %u\n",
    cbcr_scaler_cmd->horizPhaseMult);
  CDBG("cbcr_scaler_cmd.vinterpolationResolution = %d\n",
    cbcr_scaler_cmd->vertInterResolution );
  CDBG("cbcr_scaler_cmd.vphaseMultiplicationFactor = %u\n",
    cbcr_scaler_cmd->vertPhaseMult );

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL, (void *)cbcr_scaler_cmd,
    sizeof(*cbcr_scaler_cmd), VFE_CMD_S2CbCr_CFG);

  return rc;
} /* vfe_cbcr_scaler_config */

/*===========================================================================
 * FUNCTION    - vfe_config_scaler -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_config(int mod_id, void* mod_sc,
  void* vparams)
{
  vfe_status_t rc = VFE_SUCCESS;
  scaler_mod_t* scaler_mod = (scaler_mod_t *)mod_sc;
  vfe_params_t* vfe_params = (vfe_params_t *)vparams;

  if (!scaler_mod->scaler_enable) {
    CDBG("%s: SCALER not enabled", __func__);
    return VFE_SUCCESS;
  }

  /* main scaler is always on */
  rc = vfe_main_scaler_config(&(scaler_mod->main_scaler_cmd),vfe_params);
  if (rc != VFE_SUCCESS) {
    CDBG("%s:failed",__func__);
    return rc;
  }
  if (!(scaler_mod->vfe_op_mode & VFE_OUTPUTS_PREVIEW) &&
    scaler_mod->vfe_op_mode != VFE_OUTPUTS_VIDEO) {
    CDBG("%s SECONDARY SCALAR ENABLED ", __func__);
    /* secondary scaler is only on when output path 1 is enabled. */
    //TODO: Need to check if Media controller is going to provide
    //output path and any check need to be performed for secondary scaler
    rc = vfe_y_scaler_config(&(scaler_mod->y_scaler_cmd),vfe_params);
    if (rc != VFE_SUCCESS) {
      CDBG("%s:failed",__func__);
      return rc;
    }
    CDBG("sent yscale cfg comand \n");
    rc = vfe_cbcr_scaler_config(&(scaler_mod->cbcr_scaler_cmd),vfe_params);
    if (rc != VFE_SUCCESS) {
      CDBG("%s:failed",__func__);
      return rc;
    }
    CDBG("sent CbCr scalar cfg comand \n");
  } else
      CDBG("%s SECONDARY SCALAR DISABLED ", __func__);
  return rc;
} /* vfe_config_scaler */

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

  if (!scaler_mod->scaler_enable) {
    CDBG("%s: SCALER not enabled", __func__);
    return VFE_SUCCESS;
  }

  if(scaler_mod->scaler_update) {
    status = vfe_scaler_config(mod_id, scaler_mod,vfe_params);
    CDBG("%s: doing scaler_config", __func__);
    if (status != VFE_SUCCESS)
      CDBG("%s: Failed\n",__func__);
    else
      vfe_params->update |= VFE_MOD_SCALER;
  }
  scaler_mod->scaler_update = FALSE;
  return VFE_SUCCESS;
}

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

  vfe_params->moduleCfg->mainScalerEnable = enable;

  if (hw_write && (scaler_mod->scaler_enable == enable))
    return VFE_SUCCESS;

  if (!(scaler_mod->vfe_op_mode & VFE_OUTPUTS_PREVIEW) &&
    scaler_mod->vfe_op_mode != VFE_OUTPUTS_VIDEO) {
    vfe_params->moduleCfg->scaler2YEnable = enable;
    vfe_params->moduleCfg->scaler2CbcrEnable = enable;
  }
  scaler_mod->scaler_enable = enable;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_SCALER)
      : (vfe_params->current_config & ~VFE_MOD_SCALER);
  }
  return VFE_SUCCESS;
} /* vfe_scaler_enable */

/*===========================================================================
 * FUNCTION    - vfe_scaler_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_scaler_deinit(int mod_id, void *module, void *params)
{
  scaler_mod_t *scaler_mod = (scaler_mod_t *)module;
  memset(scaler_mod, 0 , sizeof(scaler_mod_t));
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_scaler_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  scaler_module_t *cmd = (scaler_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->main_scaler_cmd),
     sizeof(VFE_Main_Scaler_ConfigCmdType),
     VFE_CMD_MAIN_SCALER_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->y_scaler_cmd),
     sizeof(VFE_Output_YScaleCfgCmdType),
     VFE_CMD_S2Y_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->cbcr_scaler_cmd),
     sizeof(VFE_Output_CbCrScaleCfgCmdType),
     VFE_CMD_S2CbCr_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_scaler_plugin_update */
#endif
