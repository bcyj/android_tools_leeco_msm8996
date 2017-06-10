/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "scaler40.h"
#include "isp_log.h"

#ifdef ENABLE_SCALER_LOGGING
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define VFE_DOWNSCALER_MN_FACTOR_OFFSET 13

#define VFE_MN_INIT 0
#define SCALE_RATIO_LIMIT 105

/** scaler_cmd_debug
 *    @cmd: bcc config cmd
 *    @index: pix path index
 *
 * This function dumps the scaler module register settings set
 * to hw
 *
 * Return: nothing
 **/
static void scaler_cmd_debug(ISP_ScaleCfgCmdType* cmd, uint8_t index)
{

  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d]\n", __func__, index);
  /* Y config */
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y horiz_input = %d\n", __func__,
    index, cmd->Y_ScaleCfg.hIn);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y horiz_output = %d\n", __func__,
    index, cmd->Y_ScaleCfg.hOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y vert_input = %d\n", __func__,
    index, cmd->Y_ScaleCfg.vIn);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y vert_output = %d\n", __func__,
    index, cmd->Y_ScaleCfg.vOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y horiz_interp_Resolution = %d\n",
    __func__, index, cmd->Y_ScaleCfg.horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y horiz_phase_Multipli_Factor = %u\n",
    __func__, index, cmd->Y_ScaleCfg.horizPhaseMult);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y vert_interp_Resolution = %d\n",
    __func__, index, cmd->Y_ScaleCfg.vertInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y vert_phase_Multipli_Factor = %u\n",
    __func__, index, cmd->Y_ScaleCfg.vertPhaseMult);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y horiz_MNInit = %d\n",
    __func__, index, cmd->Y_ScaleCfg.horizMNInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y horiz_PhaseInit = %d\n",
    __func__, index, cmd->Y_ScaleCfg.horizPhaseInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y vert_MNInit = %d\n",
    __func__, index, cmd->Y_ScaleCfg.vertMNInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] Y vert_PhaseInit = %d\n",
    __func__, index, cmd->Y_ScaleCfg.vertPhaseInit);

  /* cbcr config*/
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr horiz_input = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.hIn);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr horiz_output = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.hOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr vert_input = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.vIn);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr vert_output = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.vOut);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr horiz_interp_Resolution = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.horizInterResolution);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr horiz_phase_Multipli_Factor = %u\n",
    __func__, index, cmd->CbCr_ScaleCfg.horizPhaseMult);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr vert_interp_Resolution = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.vertInterResolution );
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr vert_phase_Multipli_Factor = %u\n",
     __func__, index, cmd->CbCr_ScaleCfg.vertPhaseMult );
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr horiz_MNInit = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.horizMNInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr horiz_PhaseInit = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.horizPhaseInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr vert_MNInit = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.vertMNInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr vert_PhaseInit = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.vertPhaseInit);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr CbCr_In_Width = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.ScaleCbCrInWidth);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr Horiz_Skip_Count = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.HSkipCount);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr Right_Pad_Enable = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.RightPadEnable);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr CbCr_In_Height = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.ScaleCbCrInHeight);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr Vert_Skip_Count = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.VSkipCount);
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler[%d] CbCr Bottom_Pad_Enable = %d\n",
    __func__, index, cmd->CbCr_ScaleCfg.BottomPadEnable);

} /* scaler_cmd_debug */

/** calculate_scaler_factor
 *    @scale_factor_horiz: horizontal scale factor
 *    @scale_factor_vert:  vertical scale factor
 *    @hFactor: output
 *    @vFactor: output
 *
 * calculate scaler factor
 *
 * Return: Scaler putput value
 **/
static  int calculate_scaler_factor(int scale_factor_horiz,
  int scale_factor_vert, int* hFactor, int* vFactor)
{
  int rc = 0;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(ISP_MOD_SCALER, "%s: Output1 scale larger than camsensor FOV, set scale_factor =1\n",
         __func__);
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
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_horiz is greater than 32, which is more than "
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
    ISP_DBG(ISP_MOD_SCALER, "scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.\n");
  }

  return rc;
}/* calculate_scaler_factor */

/** scaler_trigger_enable
 *    @mod: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  scaler module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler_trigger_enable(isp_scaler_mod_t *scaler,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
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

/** scaler_enable
 *    @mod: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input struct size
 *
 *  scaler module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler_enable(isp_scaler_mod_t *scaler, isp_mod_set_enable_t *enable,
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
   /* set all scaler entries to not used */
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    scaler->scalers[i].hw_update_pending = 0;
    scaler->scalers[i].is_used = enable->enable; /* now we enable both scalers */
  }
  return 0;
}

/** scaler_calculate_zoom_scaling
 *    @pix_setting: pix path settings
 *    @entry_idx: pix path index
 *    @y_cfg: output config
 *    @crop_factor: crop factor
 *
 *  Based on input/output aspect ratio, calculate width, height
 *
 * Return: nothing
 **/
static void scaler_calculate_zoom_scaling(
  isp_hw_pix_setting_params_t *pix_setting, int entry_idx,
  ISP_Y_ScaleCfgCmdType* y_cfg, uint32_t crop_factor)
{
  uint32_t hout_factor = y_cfg->hOut * crop_factor;
  uint32_t vout_factor = y_cfg->vOut * crop_factor;
  uint32_t tmp1 = y_cfg->hIn *
    pix_setting->outputs[entry_idx].stream_param.height;
  uint32_t tmp2 = y_cfg->vIn *
    pix_setting->outputs[entry_idx].stream_param.width;
  uint32_t hOut, vOut;

  if (tmp1 == tmp2) {
    /* same aspect ratio */
    hOut = hout_factor / Q12;
    vOut = vout_factor / Q12;
    /* bounded by camif box */
    if (hOut > y_cfg->hIn || vOut > y_cfg->vIn) {
      hOut = y_cfg->hIn;
      vOut = y_cfg->vIn;
    }
  } else if (tmp1 < tmp2) {
    /* input aspect < output aspect ratio: output width bigger */
    hOut = hout_factor / Q12;
    if (hOut > y_cfg->hIn)
      hOut = y_cfg->hIn;
    vOut = hOut * y_cfg->vIn / y_cfg->hIn;
  } else {
    /* input aspect ratio > output aspect ratio: output width shorted */
    vOut = vout_factor / Q12;
    if (vOut > y_cfg->vIn)
      vOut = y_cfg->vIn;
    hOut = vOut * y_cfg->hIn / y_cfg->vIn;
  }
  y_cfg->hOut = hOut;
  y_cfg->vOut = vOut;
}

/** scaler_calculate_phase
 *    @M:       output
 *    @N:       input
 *    @offset:  offset
 *    @interp_reso: actual input width
 *    @mn_init:
 *    @phase_init:
 *    @phase_mult:
 *    @y_scaler: luma scaler
 *
 * TODO
 *
 * Return: nothing
 **/
void scaler_calculate_phase(uint32_t  M, uint32_t  N, uint32_t  offset,
  uint32_t *interp_reso, uint32_t *mn_init, uint32_t *phase_init,
  uint32_t *phase_mult, boolean   y_scaler)
{
  uint32_t ratio = N / M;
  *interp_reso = 3;
  if (ratio >= 16)     *interp_reso = 0;
  else if (ratio >= 8) *interp_reso = 1;
  else if (ratio >= 4) *interp_reso = 2;

  *mn_init = offset * M % N;
  *phase_init = (*mn_init << (13 + *interp_reso)) / M;
  if (y_scaler && ((*phase_init >> 13) != 0)) {
    *mn_init = (offset + 1) * M % N;
    *phase_init = (*mn_init << (13 + *interp_reso)) / M;
  }
  *phase_mult = (N << (13 + *interp_reso)) / M;
}

/** scaler_check_hw_limit
 *    @entry_idx: pix path index
 *    @y_cfg:
 *    @in_aspect_ratio:
 *
 * Check of output dimensions are withing the limit, if not
 * clamp to maximum
 *
 * Return: nothing
 **/
static void scaler_check_hw_limit(uint32_t entry_idx,
  ISP_Y_ScaleCfgCmdType* y_cfg, int in_aspect_ratio,
  uint32_t max_scaler_out_width)
{

  if ((entry_idx == ISP_PIX_PATH_VIEWFINDER) &&
    (y_cfg->hOut > max_scaler_out_width)) {
    ISP_DBG(ISP_MOD_SCALER,"%s: hw_limit hout = %d\n", __func__,
               max_scaler_out_width);
    y_cfg->hOut = max_scaler_out_width;
    y_cfg->vOut = y_cfg->hOut * Q12 / in_aspect_ratio;
  }

  if ((y_cfg->hOut * ISP_SCALER40_MAX_SCALER_FACTOR) < y_cfg->hIn) {
    ISP_DBG(ISP_MOD_SCALER, "%s: hOut / hIn < 1/16\n", __func__);
    y_cfg->hOut = (y_cfg->hIn + ISP_SCALER40_LIMIT_SCALER_FACTOR -1) /
      ISP_SCALER40_LIMIT_SCALER_FACTOR;
    y_cfg->vOut = y_cfg->hOut * Q12 / in_aspect_ratio;
  }

  if ((y_cfg->vOut * ISP_SCALER40_MAX_SCALER_FACTOR) < y_cfg->vIn) {
    ISP_DBG(ISP_MOD_SCALER, "%s: vOut / vIn < 1/16\n", __func__);
    y_cfg->vOut = (y_cfg->vIn + ISP_SCALER40_LIMIT_SCALER_FACTOR -1)/
      ISP_SCALER40_LIMIT_SCALER_FACTOR;
    y_cfg->hOut = y_cfg->vOut * in_aspect_ratio / Q12;
  }

  return;
}


/** scaler_config_entry
 *    @mod:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler_config_entry(isp_scaler_mod_t *scaler, int entry_idx,
   isp_hw_pix_setting_params_t *pix_setting)
{
  int rc = 0;
  isp_scaler_entry_t *entry = &scaler->scalers[entry_idx];
  ISP_Y_ScaleCfgCmdType* y_cfg = &entry->reg_cmd.Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType* cbcr_cfg = &entry->reg_cmd.CbCr_ScaleCfg;

  isp_pix_camif_cfg_t *camif_cfg = &pix_setting->camif_cfg;
  unsigned int cbcr_scale_factor_horiz, cbcr_scale_factor_vert;
  unsigned int y_scale_factor_horiz, y_scale_factor_vert;
  int cbcr_hFactor, cbcr_vFactor, y_hFactor, y_vFactor;
  int input_ratio, output_ratio;
  int is_cosited = 0;
  float zoom_scaling = 0.0;
  int  down_scaling_factor = 0;

  /* TODO: need to add cropping into scaler */
  if (pix_setting->outputs[entry_idx].stream_param.width == 0) {
    entry->is_used = 0;
    entry->hw_update_pending = 0;
    ISP_DBG(ISP_MOD_SCALER, "%s: Scaler entry %d not used", __func__, entry_idx);
    return 0;
  }
  entry->is_used = 1;
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler %d, start Y config \n", __func__, entry_idx);

  y_cfg->hEnable = TRUE;
  y_cfg->vEnable = TRUE;

  if (pix_setting->camif_cfg.is_bayer_sensor)
    y_cfg->hIn = camif_cfg->sensor_out_info.request_crop.last_pixel -
      camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
  else
    y_cfg->hIn =
      (camif_cfg->sensor_out_info.request_crop.last_pixel -
       camif_cfg->sensor_out_info.request_crop.first_pixel + 1) >> 1;

  y_cfg->vIn =
    camif_cfg->sensor_out_info.request_crop.last_line -
      camif_cfg->sensor_out_info.request_crop.first_line + 1;

  input_ratio = y_cfg->hIn * Q12 / y_cfg->vIn;
  output_ratio = pix_setting->outputs[entry_idx].stream_param.width * Q12 /
    pix_setting->outputs[entry_idx].stream_param.height;
  if (output_ratio == input_ratio) {
    y_cfg->hOut = pix_setting->outputs[entry_idx].stream_param.width;
    y_cfg->vOut = pix_setting->outputs[entry_idx].stream_param.height;
  } else if (output_ratio > input_ratio) {
    y_cfg->hOut = pix_setting->outputs[entry_idx].stream_param.width;
    y_cfg->vOut = pix_setting->outputs[entry_idx].stream_param.width *
      Q12 / input_ratio;
  } else {
    y_cfg->hOut = pix_setting->outputs[entry_idx].stream_param.height *
      input_ratio / Q12;
    y_cfg->vOut = pix_setting->outputs[entry_idx].stream_param.height;
  }
  down_scaling_factor = (int)(((float) y_cfg->hIn/ (float)y_cfg->hOut) * 100);
  if (down_scaling_factor < SCALE_RATIO_LIMIT) {
    y_cfg->hOut = y_cfg->hIn;
    y_cfg->vOut = y_cfg->vIn;
  }

  scaler_calculate_zoom_scaling(pix_setting, entry_idx, y_cfg,
    pix_setting->crop_factor);
  scaler_check_hw_limit(entry_idx, y_cfg, input_ratio,
                        scaler->max_scaler_out_width);

  /* make sure its even number so CbCr will match Y instead of rounding donw*/
  y_cfg->hOut = (y_cfg->hOut >> 1) << 1;
  y_cfg->vOut = (y_cfg->vOut >> 1) << 1;

  entry->scaling_factor = (float) y_cfg->hIn/ (float)y_cfg->hOut;
  y_scale_factor_horiz = y_cfg->hIn / y_cfg->hOut;
  y_scale_factor_vert = y_cfg->vIn / y_cfg->vOut;

  rc = calculate_scaler_factor(y_scale_factor_horiz,
    y_scale_factor_vert, &y_hFactor, &y_vFactor);

  y_cfg->horizInterResolution = y_hFactor;
  y_cfg->horizPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_hFactor,
    (int32_t)y_cfg->hIn) / (int) y_cfg->hOut;

  y_cfg->vertInterResolution = y_vFactor;
  y_cfg->vertPhaseMult =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->vIn) / (int) y_cfg->vOut;

  y_cfg->horizMNInit = VFE_MN_INIT;
  y_cfg->horizPhaseInit =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_hFactor,
    (int32_t)y_cfg->horizMNInit) / (int) y_cfg->hOut;

  y_cfg->vertMNInit = VFE_MN_INIT;
  y_cfg->vertPhaseInit =
    FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + y_vFactor,
    (int32_t)y_cfg->vertMNInit) / (int) y_cfg->vOut;

  /*TODO: RIGHT_PAD_EN, H_SKIP_CNT and SCALE_CBCR_IN_WIDTH
    need to be taken care*/
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler_enc start CbCr config \n", __func__);

  cbcr_cfg->hEnable  = TRUE;
  cbcr_cfg->vEnable = TRUE;
  cbcr_cfg->hIn = y_cfg->hIn;
  cbcr_cfg->vIn = y_cfg->vIn;

  switch (pix_setting->outputs[entry_idx].stream_param.fmt) {
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16: {
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    cbcr_cfg->vOut = y_cfg->vOut;
    is_cosited = 1;
  }
    break;

  default: {
    cbcr_cfg->vOut = y_cfg->vOut / 2;
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    is_cosited = 0;
  }
    break;
  }

  if (pix_setting->outputs[entry_idx].need_uv_subsample) {
    cbcr_cfg->vOut /= 2;
    cbcr_cfg->hOut /= 2;
  }

  cbcr_scale_factor_horiz = cbcr_cfg->hIn / cbcr_cfg->hOut;
  cbcr_scale_factor_vert = cbcr_cfg->vIn / cbcr_cfg->vOut;

  rc = calculate_scaler_factor(cbcr_scale_factor_horiz,
    cbcr_scale_factor_vert, &cbcr_hFactor, &cbcr_vFactor);

  cbcr_cfg->horizInterResolution = cbcr_hFactor;
  cbcr_cfg->horizPhaseMult = FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET +
    cbcr_hFactor, (int32_t) cbcr_cfg->hIn) / (int) cbcr_cfg->hOut;

  cbcr_cfg->vertInterResolution = cbcr_vFactor;
  cbcr_cfg->vertPhaseMult = FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET +
    cbcr_vFactor, (int32_t) cbcr_cfg->vIn) / (int) cbcr_cfg->vOut;

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
    cbcr_cfg->horizMNInit = VFE_MN_INIT;
    cbcr_cfg->horizPhaseInit =
      FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_hFactor,
      (int32_t)cbcr_cfg->horizMNInit) / (int) cbcr_cfg->hOut;
    cbcr_cfg->vertMNInit = VFE_MN_INIT;
    cbcr_cfg->vertPhaseInit =
      FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_vFactor,
      (int32_t)cbcr_cfg->vertMNInit) / (int) cbcr_cfg->vOut;
  } else {
    cbcr_cfg->vertMNInit = VFE_MN_INIT;
    cbcr_cfg->vertPhaseInit =
      FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_vFactor,
      (int32_t)cbcr_cfg->vertMNInit) / (int) cbcr_cfg->vOut;
    cbcr_cfg->horizMNInit = VFE_MN_INIT;
    cbcr_cfg->horizPhaseInit =
      FLOAT_TO_Q(VFE_DOWNSCALER_MN_FACTOR_OFFSET + cbcr_hFactor,
      (int32_t)cbcr_cfg->horizMNInit) / (int) cbcr_cfg->hOut;
    CDBG_HIGH("%s: not support cosited format yet", __func__);
  }

  entry->hw_update_pending = TRUE;
  return rc;
}

/** fov_config_entry_split
 *    @mod:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler_config_entry_split(isp_scaler_mod_t *scaler, int entry_idx,
   isp_hw_pix_setting_params_t *pix_setting)
{
  int rc = 0;
  isp_scaler_entry_t *entry = &scaler->scalers[entry_idx];
  ISP_Y_ScaleCfgCmdType* y_cfg = &entry->reg_cmd.Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType* cbcr_cfg = &entry->reg_cmd.CbCr_ScaleCfg;

  unsigned int cbcr_scale_factor_horiz, cbcr_scale_factor_vert;
  unsigned int y_scale_factor_horiz, y_scale_factor_vert;
  int cbcr_hFactor, cbcr_vFactor, y_hFactor, y_vFactor;
  int input_ratio, output_ratio;
  int is_cosited = 0;
  float zoom_scaling = 0.0;
  uint32_t interp_reso, mn_init, phase_init, phase_mult;

  isp_out_info_t* isp_out_info =
    &pix_setting->outputs[entry_idx].isp_out_info;
  uint32_t offset = (isp_out_info->stripe_id == ISP_STRIPE_RIGHT) ?
    isp_out_info->right_stripe_offset : 0;

  /* TODO: need to add cropping into scaler */
  if (pix_setting->outputs[entry_idx].stream_param.width == 0) {
    entry->is_used = 0;
    entry->hw_update_pending = 0;
    ISP_DBG(ISP_MOD_SCALER, "%s: Scaler entry %d not used", __func__, entry_idx);
    return 0;
  }

  entry->is_used = 1;
  ISP_DBG(ISP_MOD_SCALER, "%s: scaler %d, start Y config \n", __func__, entry_idx);

  y_cfg->hEnable = TRUE;
  y_cfg->vEnable = TRUE;
  cbcr_cfg->hEnable = TRUE;
  cbcr_cfg->vEnable = TRUE;

  /* dual vfe scaler need use sensor output window to calculate config*/
  y_cfg->hIn = pix_setting->camif_cfg.sensor_out_info.request_crop.last_pixel -
    pix_setting->camif_cfg.sensor_out_info.request_crop.first_pixel + 1;
  y_cfg->vIn = pix_setting->camif_cfg.sensor_out_info.request_crop.last_line -
    pix_setting->camif_cfg.sensor_out_info.request_crop.first_line + 1;

  if (!pix_setting->camif_cfg.is_bayer_sensor)
    y_cfg->hIn /= 2;

  /* from now on, hIn and vIn are adjusted, it's imaginary as if
     FOV crop is performed before to alter the input to the scaler */
  /* adjust hIn, vIn based on zoom level (crop_factor) */
  y_cfg->hIn    = y_cfg->hIn * Q12 / pix_setting->crop_factor;
  y_cfg->vIn    = y_cfg->vIn * Q12 / pix_setting->crop_factor;
  y_cfg->hOut    = pix_setting->outputs[entry_idx].stream_param.width;
  y_cfg->vOut    = pix_setting->outputs[entry_idx].stream_param.height;

  /* adjust to match input to output aspect ratio */
  /* This might not seem easy to understand why we set horizontal
      and vertical input and output to be the same initially.
      But due to luma scaler implementation where right stripe needs to start
      at roll-over points, the crop factor is chosen very carefully earlier so
      that the resulting scaling ratio ensures right stripe starts at a
      roll-over point. The crop factor was chosen based on which dimension is
      the non-cropping side, therefore, the non-cropping side M and N should be
      used precisely on both dimensions. Please do not change the value of hIn
      hOut vIn and vOut without understanding why it is set this way. */
  if (y_cfg->hOut * y_cfg->vIn < y_cfg->vOut * y_cfg->hIn) {
    y_cfg->hIn  = y_cfg->vIn;
    y_cfg->hOut = y_cfg->vOut;
  }
  else {
    y_cfg->vIn  = y_cfg->hIn;
    y_cfg->vOut = y_cfg->hOut;
  }

  /* maximum zoom by ISP reached */
  if (y_cfg->hIn < y_cfg->hOut) {
    y_cfg->hIn = y_cfg->hOut;
    y_cfg->vIn = y_cfg->vOut;
  }

  /* derive cbcr information based on y */
  cbcr_cfg->hIn  = y_cfg->hIn;
  cbcr_cfg->vIn  = y_cfg->vIn;

  switch (pix_setting->outputs[entry_idx].stream_param.fmt) {
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16: {
    cbcr_cfg->hOut = y_cfg->hOut >>1;
    cbcr_cfg->vOut = y_cfg->vOut;
    is_cosited = 1;
  }
    break;

  default: {
    cbcr_cfg->vOut = y_cfg->vOut >> 1;
    cbcr_cfg->hOut = y_cfg->hOut >> 1;
    is_cosited = 0;
  }
    break;
  }

  if (pix_setting->outputs[entry_idx].need_uv_subsample) {
    cbcr_cfg->vOut /= 2;
    cbcr_cfg->hOut /= 2;
  }

  entry->scaling_factor = (float) y_cfg->hIn/ (float)y_cfg->hOut;

  /* calculate phase related values */
  scaler_calculate_phase(y_cfg->hOut, y_cfg->hIn, offset,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 1);
  y_cfg->horizInterResolution = interp_reso;
  if (isp_out_info->stripe_id == ISP_STRIPE_LEFT ||
      entry->is_right_stripe_config == 0) {
    y_cfg->horizMNInit          = mn_init;
    y_cfg->horizPhaseInit       = phase_init;
  }
  entry->reg_cmd.VFE1_Y_stripe_workaround_cfg.vfe1_horizMNInit = mn_init;
  entry->reg_cmd.VFE1_Y_stripe_workaround_cfg.vfe1_horizPhaseInit = phase_init;
  entry->is_right_stripe_config = 1;

  y_cfg->horizPhaseMult       = phase_mult;
  scaler_calculate_phase(y_cfg->vOut, y_cfg->vIn, 0,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 1);
  y_cfg->vertInterResolution  = interp_reso;
  y_cfg->vertMNInit           = mn_init;
  y_cfg->vertPhaseInit        = phase_init;
  y_cfg->vertPhaseMult        = phase_mult;
  scaler_calculate_phase(cbcr_cfg->hOut, cbcr_cfg->hIn, offset,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 0);
  cbcr_cfg->horizInterResolution = interp_reso;
  cbcr_cfg->horizMNInit          = mn_init;
  cbcr_cfg->horizPhaseInit       = phase_init;
  cbcr_cfg->horizPhaseMult       = phase_mult;
  scaler_calculate_phase(cbcr_cfg->vOut, cbcr_cfg->vIn, 0,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 0);
  cbcr_cfg->vertInterResolution  = interp_reso;
  cbcr_cfg->vertMNInit           = mn_init;
  cbcr_cfg->vertPhaseInit        = phase_init;
  cbcr_cfg->vertPhaseMult        = phase_mult;

  cbcr_cfg->ScaleCbCrInWidth  = cbcr_cfg->hIn;
  cbcr_cfg->ScaleCbCrInHeight = cbcr_cfg->vIn;
  cbcr_cfg->HSkipCount        = 0;
  cbcr_cfg->VSkipCount        = 0;
  cbcr_cfg->RightPadEnable    = 0;
  cbcr_cfg->BottomPadEnable   = 0;

  entry->hw_update_pending = TRUE;
  return rc;
}

/** scaler_config
 *    @mod: fov module struct data
 *    @pix_setting: hw pixel settings
 *    @size: input params struct size
 *
 * TODO
 *
 * Return: 0 - sucess and negative value - failure
 **/
static int scaler_config(isp_scaler_mod_t *scaler,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t size)
{
  int i, rc = 0;

  if (sizeof(isp_hw_pix_setting_params_t) != size) {
    CDBG_ERROR("%s: in_params size mismatch\n", __func__);
    return -1;
  }
  if (!scaler->enable) {
    /* not enabled no op */
    return rc;
  }
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    if (!pix_setting->camif_cfg.ispif_out_info.is_split)
      rc = scaler_config_entry(scaler, i, pix_setting);
    else
      rc = scaler_config_entry_split(scaler, i, pix_setting);

    if (rc < 0) {
      /* scaler entry configuration error */
      CDBG_ERROR("%s: scaler_config_entry error, idx = %d, rc = %d",
                 __func__, i, rc);
      return rc;
    }
  }
  return rc;
}

/** scaler_set_zoom_ratio
 *    @scaler:
 *    @in_params:
 *    @size:
 *
 *  TODO
 *
 * Return: nothing
 **/
static int scaler_set_zoom_ratio(isp_scaler_mod_t *scaler,
  isp_hw_pix_setting_params_t *in_params, uint32_t size)
{
  return scaler_config(scaler, in_params, size);

}

/** scaler_trigger_update
 *    @scaler: scaler module control struct
 *    @params: input params
 *    @in_param_size: input params struct size
 *
 *  scaler module modify reg settings as per new input params
 *  and trigger hw update
 *
 * Return: 0 - success and negative value - failure
 **/
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

/** scaler_destroy
 *    @mod_ctrl: scaler module control strcut
 *
 *  Close scaler module
 *
 * Return: 0 always
 **/
static int scaler_destroy (void *mod_ctrl)
{
  isp_scaler_mod_t *scaler = mod_ctrl;

  memset(scaler,  0,  sizeof(isp_scaler_mod_t));
  free(scaler);
  return 0;
}

/** scaler_set_params
 *    @mod_ctrl: scaler module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *
 *  set config params like mod enable, zoom, trigger update, mod
 *  config etc. utility to update scaler module
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler_set_params (void *mod_ctrl, uint32_t params_id,
  void *in_params, uint32_t in_params_size)
{
  isp_scaler_mod_t *scaler = mod_ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = scaler_enable(scaler, (isp_mod_set_enable_t *)in_params,
                     in_params_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = scaler_config(scaler,
           (isp_hw_pix_setting_params_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = scaler_trigger_enable(scaler,
           (isp_mod_set_enable_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = scaler_trigger_update(scaler,
           (isp_pix_trigger_update_input_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_MOD_SET_ZOOM_RATIO: {
    rc = scaler_set_zoom_ratio(scaler,
           (isp_hw_pix_setting_params_t *)in_params, in_params_size);
  }
    break;

  default: {
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
  }
    break;
  }
  return rc;
}

/** scaler_get_params
 *    @mod_ctrl: scaler module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *    @out_params: struct to return out params
 *    @out_param_size: output params struct size
 *
 *  Get config params utility to fetch config of scaler module
 *
 * Return: 0 - success and negative value - failure
 **/
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
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, params_id);
      break;
    }
    enable->enable = scaler->enable;
  }
    break;

  case ISP_PIX_GET_UV_SUBSAMPLE_SUPPORTED:{
    int i;
    float max_scale_factor;
    uint32_t camif_width, camif_height, scale_factor_w, scale_factor_h;
    isp_hw_pix_setting_params_t *pix_setting = in_params;
    isp_hwif_output_cfg_t *output = out_params;

    camif_width =
      pix_setting->camif_cfg.sensor_out_info.request_crop.last_pixel -
        pix_setting->camif_cfg.sensor_out_info.request_crop.first_pixel + 1;
    camif_height =
      pix_setting->camif_cfg.sensor_out_info.request_crop.last_line -
        pix_setting->camif_cfg.sensor_out_info.request_crop.first_line + 1;

    /* only got Y scaling factor from get param,
       check if accept extra subsample chroma
       consider the MAX scaling factor without ZOOM
       to get the max possible scaling factor*/
    scale_factor_w =
      (float)camif_width / (float)output->stream_param.width;
    scale_factor_h =
      (float)camif_height / (float)output->stream_param.height;
    max_scale_factor =
      (scale_factor_w > scale_factor_h) ? scale_factor_w : scale_factor_h;

    output->need_uv_subsample =
      (max_scale_factor <= ISP_SCALER40_MAX_SCALER_FACTOR / 2);
  }
    break;

  case ISP_PIX_GET_SCALER_CROP_REQUEST:{
    int i;
    uint32_t *scaler_crop_request = out_params;
    ISP_Y_ScaleCfgCmdType y_cfg;

    for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
      y_cfg = scaler->scalers[i].reg_cmd.Y_ScaleCfg;
      if (scaler->scalers[i].is_used) {
        if ((y_cfg.hOut >= y_cfg.hIn) || (y_cfg.vOut >= y_cfg.vIn)) {
          scaler_crop_request[i] = 1;
        } else{
         if(i == ISP_PIX_PATH_VIEWFINDER &&
           y_cfg.hOut >= scaler->max_scaler_out_width)
           scaler_crop_request[i] = 1;
         else
           scaler_crop_request[i] = 0;
        }
      }
    }
  }
    break;

  case ISP_PIX_GET_SCALER_OUTPUT: {
    int i;
    isp_pixel_window_info_t *scaler_output = out_params;
    for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
      if (scaler->scalers[i].is_used) {
        scaler_output[i].height =
          scaler->scalers[i].reg_cmd.Y_ScaleCfg.vOut;
        scaler_output[i].width =
          scaler->scalers[i].reg_cmd.Y_ScaleCfg.hOut;
        scaler_output[i].scaling_factor = scaler->scalers[i].scaling_factor;
        ISP_DBG(ISP_MOD_SCALER, "%s: scaler_output[%d]: Width %d, Height %d, scalefactor %f\n",
          __func__, i, scaler_output[i].width, scaler_output[i].height,
          scaler_output[i].scaling_factor);
      }
    }
  }
    break;

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    if (sizeof(vfe_diagnostics_t) != out_params_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, params_id);
      break;
    }
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_SCALER, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default: {
    rc = -EAGAIN;
  }
    break;
  }
  return rc;
}

/** scaler_do_hw_update
 *    @scaler_mod: scaler module struct data
 *
 * update scaler module register to kernel
 *
 * Return: nothing
 **/
static int scaler_do_hw_update(isp_scaler_mod_t *scaler_mod)
{
  int i, rc = 0;

  ISP_DBG(ISP_MOD_SCALER, "%s: HW_update, scaler[0] = %d, scaler[1] = %d\n", __func__,
    scaler_mod->scalers[0].hw_update_pending,
    scaler_mod->scalers[1].hw_update_pending);

  for (i = 0; i< ISP_PIX_PATH_MAX; i++) {
    if (scaler_mod->scalers[i].hw_update_pending == TRUE) {
      scaler_cmd_debug(&scaler_mod->scalers[i].reg_cmd, i);

      if (i == ISP_PIX_PATH_ENCODER) {
         rc = isp_pipeline_util_single_HW_write(scaler_mod->fd,
           (void *)&scaler_mod->scalers[i].reg_cmd,
           sizeof(scaler_mod->scalers[i].reg_cmd),
           ISP_SCALER40_ENC_OFF, ISP_SCALER40_ENC_LEN, VFE_WRITE);

         /* system workaround for vfe1_y_enc_stripe_H_cfg */
         rc = isp_pipeline_util_single_HW_write(scaler_mod->fd,
           (void *)&scaler_mod->scalers[i].reg_cmd.VFE1_Y_stripe_workaround_cfg,
           sizeof(ISP_Y_Stripe_WorkAround_CfgCmdType),
           ISP_SCALER40_SYSTEM_WORKAROUND_OFF_1,
           ISP_SCALER40_SYSTEM_WORKAROUND_LEN_1, VFE_WRITE);

      } else{
         rc = isp_pipeline_util_single_HW_write(scaler_mod->fd,
           (void *)&scaler_mod->scalers[i].reg_cmd,
           sizeof(scaler_mod->scalers[i].reg_cmd),
           ISP_SCALER40_VIEW_OFF, ISP_SCALER40_VIEW_LEN, VFE_WRITE);

          /* system workaround for vfe1_y_view_stripe_H_cfg */
         rc = isp_pipeline_util_single_HW_write(scaler_mod->fd,
           (void *)&scaler_mod->scalers[i].reg_cmd.VFE1_Y_stripe_workaround_cfg,
           sizeof(ISP_Y_Stripe_WorkAround_CfgCmdType),
           ISP_SCALER40_SYSTEM_WORKAROUND_OFF_2,
           ISP_SCALER40_SYSTEM_WORKAROUND_LEN_2, VFE_WRITE);
      }

      scaler_mod->scalers[i].reg_cmd.Y_ScaleCfg.horizMNInit =
        scaler_mod->scalers[i].reg_cmd.VFE1_Y_stripe_workaround_cfg.
          vfe1_horizMNInit;
      scaler_mod->scalers[i].reg_cmd.Y_ScaleCfg.horizPhaseInit =
        scaler_mod->scalers[i].reg_cmd.VFE1_Y_stripe_workaround_cfg.
          vfe1_horizPhaseInit;
      scaler_mod->scalers[i].hw_update_pending = 0;
    }
  }

  return rc;
}

/** scaler_reset
 *      @mod: scaler module struct data
 *
 * Scaler module disable hw updates,release reg settings and
 * structs
 *
 * Return: nothing
 **/
static void scaler_reset(isp_scaler_mod_t *mod)
{
  int i;
  mod->trigger_enable = 0;
  mod->enable = 0;
  mod->applied_crop_factor = 0;
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    mod->scalers[i].hw_update_pending = 0;
    memset(&mod->scalers[i].reg_cmd, 0, sizeof(mod->scalers[i].reg_cmd));
    mod->scalers[i].is_right_stripe_config = 0;
    mod->scalers[i].is_used = 0;
  }
}

/** scaler_action
 *    @mod_ctrl: scaler module control struct
 *    @action_code : action code
 *    @action_data: not used
 *    @action_data_size: not used
 *
 *  processing the hw action like update or reset
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler_action (void *mod_ctrl, uint32_t action_code,
  void *action_data, uint32_t action_data_size)
{
  int rc = 0;
  isp_scaler_mod_t *scaler = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = scaler_do_hw_update(scaler);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET:{
    scaler_reset(scaler);
  }
    break;

  default: {
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
  }
    break;
  }
  return rc;
}

/** scaler_init
 *    @mod_ctrl: scaler module control strcut
 *    @in_params: scaler hw module init params
 *    @notify_ops: fn pointer to notify other modules
 *
 *  scaler module data struct initialization
 *
 * Return: 0 always
 **/
static int scaler_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_scaler_mod_t *scaler = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  scaler->fd = init_params->fd;
  scaler->notify_ops = notify_ops;
  scaler->max_scaler_out_width = init_params->max_scaler_out_width;
  scaler->max_scaler_out_height = init_params->max_scaler_out_height;
  scaler_reset(scaler);
  return 0;
}

/** scaler40_open
 *    @version: hw version
 *
 *  scaler 40 module open and create func table
 *
 * Return: scaler module ops struct pointer
 **/
isp_ops_t *scaler40_open(uint32_t version)
{
  isp_scaler_mod_t *scaler = malloc(sizeof(isp_scaler_mod_t));

  if (!scaler) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(scaler,  0,  sizeof(isp_scaler_mod_t));
  scaler->ops.ctrl = (void *)scaler;
  scaler->ops.init = scaler_init;
  /* destroy the module object */
  scaler->ops.destroy = scaler_destroy;
  /* set parameter */
  scaler->ops.set_params = scaler_set_params;
  /* get parameter */
  scaler->ops.get_params = scaler_get_params;
  scaler->ops.action = scaler_action;
  return &scaler->ops;
}
