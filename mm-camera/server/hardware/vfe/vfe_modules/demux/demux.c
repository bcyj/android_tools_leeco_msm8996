/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe_util_common.h"
#include "vfe.h"
#include "demux.h"
#include "vfe_tgtcommon.h"

#ifdef ENABLE_DEMUX_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define VFE_CMD_DEMUX_R_CHANNEL_GAIN_CONFIG 200

#define MAX_DEMUX_GAIN 7.8

#define INIT_GAIN(p_gain, val) ( {\
  p_gain.blue = val; \
  p_gain.red = val; \
  p_gain.green_even = val; \
  p_gain.green_odd = val; \
})

/*===========================================================================
 * FUNCTION    - vfe_demux_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_demux_debug(VFE_DemuxConfigCmdType* pcmd)
{
  CDBG("VFE_DemuxConfigCmd.ch0OddGain = %d\n",
    pcmd->ch0OddGain);
  CDBG("VFE_DemuxConfigCmd.ch0EvenGain = %d\n",
    pcmd->ch0EvenGain);
  CDBG("VFE_DemuxConfigCmd.ch1Gain = %d\n",
    pcmd->ch1Gain);
  CDBG("VFE_DemuxConfigCmd.ch2Gain = %d\n",
    pcmd->ch2Gain);
#ifndef VFE_2X
  CDBG("VFE_DemuxConfigCmd.period = %d\n",
    pcmd->period);
  CDBG("VFE_DemuxConfigCmd.evenCfg = %d\n",
    pcmd->evenCfg);
  CDBG("VFE_DemuxConfigCmd.oddCfg = %d\n",
    pcmd->oddCfg);
#else
  CDBG("VFE_DemuxConfigCmd.inputFormatFirstRowPattern = %d\n",
    pcmd->inputFormatFirstRowPattern);
  CDBG("VFE_DemuxConfigCmd.chromaCositingForYCbCrInputs = %d\n",
    pcmd->chromaCositingForYCbCrInputs);
#endif
}

/*===========================================================================
 * FUNCTION    - vfe_demux_gain_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_demux_gain_debug(VFE_DemuxGainCfgCmdType* pcmd)
{
  CDBG("VFE_DemuxGainCfgCmdType.ch0OddGain = %d\n",
    pcmd->ch0OddGain);
  CDBG("VFE_DemuxGainCfgCmdType.ch0EvenGain = %d\n",
    pcmd->ch0EvenGain);
  CDBG("VFE_DemuxGainCfgCmdType.ch1Gain = %d\n",
    pcmd->ch1Gain);
  CDBG("VFE_DemuxGainCfgCmdType.ch2Gain = %d\n",
    pcmd->ch2Gain);
}

/*===========================================================================
 * FUNCTION    - vfe_demux_set_cfg_parms -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t vfe_demux_set_cfg_parms(VFE_DemuxConfigCmdType* pcmd,
  sensor_camif_inputformat_t format)
{
  int rc = TRUE;
  /* Configure VFE input format, Send Input format command */
  CDBG("%s: format %d", __func__, format);
#ifndef VFE_2X
  switch (format) {
    /* bayer patterns */
    case CAMIF_BAYER_G_B:
      pcmd->period = 1;
      pcmd->evenCfg = 0xAC;
      pcmd->oddCfg = 0xC9;
      break;
    case CAMIF_BAYER_B_G:
      pcmd->period = 1;
      pcmd->evenCfg = 0xCA;
      pcmd->oddCfg = 0x9C;
      break;
    case CAMIF_BAYER_G_R:
      pcmd->period = 1;
      pcmd->evenCfg = 0x9C;
      pcmd->oddCfg = 0xCA;
      break;
    case CAMIF_BAYER_R_G:
      pcmd->period = 1;
      pcmd->evenCfg = 0xC9;
      pcmd->oddCfg = 0xAC;
      break;
      /* YCbCr Patterns */
    case CAMIF_YCBCR_Y_CB_Y_CR:
      pcmd->period = 3;
      pcmd->evenCfg = 0x9CAC;
      pcmd->oddCfg = 0x9CAC;
      break;
    case CAMIF_YCBCR_Y_CR_Y_CB:
      pcmd->period = 3;
      pcmd->evenCfg = 0xAC9C;
      pcmd->oddCfg = 0xAC9C;
      break;
    case CAMIF_YCBCR_CB_Y_CR_Y:
      pcmd->period = 3;
      pcmd->evenCfg =0xC9CA;
      pcmd->oddCfg = 0xC9CA;
      break;
    case CAMIF_YCBCR_CR_Y_CB_Y:
      pcmd->period = 3;
      pcmd->evenCfg =0xCAC9;
      pcmd->oddCfg =0xCAC9;
      break;
    default:
      CDBG_ERROR("Error VFE input not configured!!!\n");
      rc = FALSE;
      break;
  }
#else
  switch (format) {
    /* bayer patterns */
    case CAMIF_BAYER_G_B:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_BAYER_GBGB_PATTERN;
      break;
    case CAMIF_BAYER_B_G:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_BAYER_BGBG_PATTERN;
      break;
    case CAMIF_BAYER_G_R:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_BAYER_GRGR_PATTERN;
      break;
    case CAMIF_BAYER_R_G:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_BAYER_RGRG_PATTERN;
      break;
      /* YCbCr Patterns */
    case CAMIF_YCBCR_Y_CB_Y_CR:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_422_YCBYCR_PATTERN;
      break;
    case CAMIF_YCBCR_Y_CR_Y_CB:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_422_YCRYCB_PATTERN;
      break;
    case CAMIF_YCBCR_CB_Y_CR_Y:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_422_CBYCRY_PATTERN;
      break;
    case CAMIF_YCBCR_CR_Y_CB_Y:
      pcmd->inputFormatFirstRowPattern =
        VFE_INPUT_FORMAT_FIRST_ROW_422_CRYCBY_PATTERN;
      break;
    default:
      CDBG_ERROR("Error VFE input not configured!!!\n");
      rc = FALSE;
      break;
  }

  //TODO: disabling for now, need to check for YCBCR
  pcmd->chromaCositingForYCbCrInputs =
    VFE_DISABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES;
#endif
  return rc;
}

/*===========================================================================
 * FUNCTION    - vfe_demux_r_image_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_r_image_update(int mod_id, void *demux_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  vfe_params_t *parms = (vfe_params_t *) vparms;

  int is_snap = IS_SNAP_MODE(parms);
  int index = (is_snap) ? SNAP : PREV;
  VFE_DemuxGainCfgCmdType* p_gaincmd = (is_snap) ?
    &mod->VFE_SnapRImageGainConfigCmd : &mod->VFE_PreviewRImageGainConfigCmd;

  p_gaincmd->ch0OddGain = DEMUX_GAIN(mod->r_gain[index].green_odd);
  p_gaincmd->ch0EvenGain = DEMUX_GAIN(mod->r_gain[index].green_even);
  p_gaincmd->ch1Gain = DEMUX_GAIN(mod->r_gain[index].blue);
  p_gaincmd->ch2Gain = DEMUX_GAIN(mod->r_gain[index].red);

  status = vfe_util_write_hw_cmd(parms->camfd,
    CMD_GENERAL, (void *)p_gaincmd,
    sizeof(VFE_DemuxGainCfgCmdType),
    VFE_CMD_DEMUX_R_CHANNEL_GAIN_CONFIG);
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_demux_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_config(int mod_id, void *demux_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  vfe_params_t *parms = (vfe_params_t *) vparms;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;

  int is_snap = IS_SNAP_MODE(parms);
  int index = (is_snap) ? SNAP : PREV;
  VFE_DemuxConfigCmdType* p_cmd = (is_snap) ? &mod->VFE_SnapDemuxConfigCmd :
    &mod->VFE_PreviewDemuxConfigCmd;
  int is_vfe33 = FALSE, is_3d_mode = FALSE; /* for V2 */

  vfe_demux_set_cfg_parms(p_cmd,
    parms->sensor_parms.vfe_camif_in_fmt);
  CDBG("%s: sensor input format : %d \n", __func__, parms->sensor_parms.vfe_camif_in_fmt);

  CDBG("%s %5.2f %5.2f %5.2f %5.2f ", __func__,
    mod->gain[index].green_odd,
    mod->gain[index].green_even,
    mod->gain[index].red,
    mod->gain[index].blue);
#ifndef VFE_2X
  p_cmd->ch0EvenGain = DEMUX_GAIN(mod->gain[index].green_even);
  p_cmd->ch0OddGain = DEMUX_GAIN(mod->gain[index].green_odd);
  p_cmd->ch1Gain = DEMUX_GAIN(mod->gain[index].blue);
  p_cmd->ch2Gain = DEMUX_GAIN(mod->gain[index].red);
#else
  p_cmd->ch0EvenGain = 128;
  p_cmd->ch0OddGain = 128;
  p_cmd->ch1Gain = 128;
  p_cmd->ch2Gain = 128;
#endif

  CDBG("%s Demux config", (is_snap) ? "Snapshot" : "Preview");
  vfe_demux_debug(p_cmd);

  /* 3d config for V2 */
  if (is_vfe33 && is_3d_mode)
    vfe_demux_r_image_update(mod_id, mod, parms);

  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL,
    (void *) p_cmd,
    sizeof(VFE_DemuxConfigCmdType),
    VFE_CMD_DEMUX_CFG);

  if (VFE_SUCCESS == status)
    mod->update = FALSE;

  return status;
} /*vfe_demux_config*/

/*===========================================================================
 * Function:           vfe_demux_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_demux_enable(int mod_id, void *demux_mod, void *vparms,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  vfe_params_t *params = (vfe_params_t *) vparms;
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  params->moduleCfg->demuxEnable = enable;

  if (hw_write && (mod->enable == enable))
    return VFE_SUCCESS;

  mod->enable = enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_DEMUX)
      : (params->current_config & ~VFE_MOD_DEMUX);
  }
  return status;
} /* vfe_demux_enable */

/*===========================================================================
 * FUNCTION    - vfe_demux_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_trigger_update(int mod_id, void *demux_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  vfe_params_t *parms = (vfe_params_t *) vparms;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;

  int is_snap = IS_SNAP_MODE(parms);
  int index = (is_snap) ? SNAP : PREV;
  float max_gain = 0.0, gain = 0.0, new_dig_gain = 1.0;
  float max_ch_gain = 0.0;

  if(!mod->enable) {
    CDBG("%s: Demux not enabled", __func__);
    return VFE_SUCCESS;
  }

  if(!mod->trigger_enable) {
    CDBG("%s: Demux trigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s: dig gain %5.3f", __func__, parms->digital_gain);
  if (parms->digital_gain < 1.0)
    parms->digital_gain = 1.0;

  if (!parms->use_cc_for_dig_gain) {
    max_ch_gain = MAX(chromatix_ptr->chromatix_channel_balance_gains.green_odd,
      MAX(chromatix_ptr->chromatix_channel_balance_gains.green_even,
      MAX(chromatix_ptr->chromatix_channel_balance_gains.red,
      chromatix_ptr->chromatix_channel_balance_gains.blue)));

    CDBG("%s: max_ch_gain %5.3f glob %5.3f", __func__, max_ch_gain,
      chromatix_ptr->color_correction_global_gain);
    max_gain = max_ch_gain * chromatix_ptr->color_correction_global_gain *
      parms->digital_gain;

    CDBG("%s: max_gain_final %5.3f", __func__, max_gain);
    if (max_gain > MAX_DEMUX_GAIN) {
      new_dig_gain = MAX_DEMUX_GAIN/
        (chromatix_ptr->color_correction_global_gain * max_ch_gain);
      parms->aec_gain_adj.cc_gain_adj = max_gain/MAX_DEMUX_GAIN;
    } else {
      new_dig_gain = max_gain/
        (chromatix_ptr->color_correction_global_gain * max_ch_gain);
      parms->aec_gain_adj.cc_gain_adj = 1.0;
    }
  } else {
    parms->aec_gain_adj.cc_gain_adj = parms->digital_gain;
  }

  CDBG("%s: dig_gain_old %5.3f new %5.3f", __func__, mod->dig_gain[index],
    new_dig_gain);
  if (F_EQUAL(mod->dig_gain[index], new_dig_gain)
    && (mod->mode == parms->vfe_op_mode)
    && !mod->reload_params) {
    CDBG("%s: No update required", __func__);
    return VFE_SUCCESS;
  }
  mod->mode = parms->vfe_op_mode;
  mod->reload_params = FALSE;
  mod->dig_gain[index] = new_dig_gain;
  parms->aec_gain_adj.total_dig_gain = parms->digital_gain;

  gain = chromatix_ptr->color_correction_global_gain * mod->dig_gain[index];
  mod->gain[index].green_odd = gain *
    chromatix_ptr->chromatix_channel_balance_gains.green_odd;
  mod->gain[index].green_even= gain *
    chromatix_ptr->chromatix_channel_balance_gains.green_even;
  mod->gain[index].red = gain *
    chromatix_ptr->chromatix_channel_balance_gains.red;
  mod->gain[index].blue = gain *
    chromatix_ptr->chromatix_channel_balance_gains.blue;

  if (parms->demosaic_wb_not_present && is_snap) {
    CDBG("%s: AWB gain g %f b %f r %f", __func__,
      parms->awb_params.gain.g_gain,
      parms->awb_params.gain.b_gain,
      parms->awb_params.gain.r_gain);
    mod->gain[SNAP].blue *= parms->awb_params.gain.b_gain;
    mod->gain[SNAP].green_even *= parms->awb_params.gain.g_gain;
    mod->gain[SNAP].green_odd *= parms->awb_params.gain.g_gain;
    mod->gain[SNAP].red *= parms->awb_params.gain.r_gain;
  }
  mod->update = TRUE;

  return status;
}/*vfe_demux_trigger_update*/

/*===========================================================================
 * FUNCTION    - vfe_demux_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_set_gains(demux_mod_t *mod, vfe_params_t* parms,
  float g_odd_gain, float g_even_gain, float b_gain, float r_gain)
{
  vfe_status_t status = VFE_SUCCESS;
  int is_snap = IS_SNAP_MODE(parms);
  int index = (is_snap) ? SNAP : PREV;
  mod->gain[index].green_odd = g_odd_gain;
  mod->gain[index].green_even= g_even_gain;
  mod->gain[index].red = r_gain;
  mod->gain[index].blue = b_gain;
  return status;
}
/*===========================================================================
 * FUNCTION    - vfe_demux_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_update(int mod_id, void *demux_mod, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  vfe_params_t *parms = (vfe_params_t *) vparms;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;

  int is_snap = IS_SNAP_MODE(parms);
  int index = (is_snap) ? SNAP : PREV;
  VFE_DemuxConfigCmdType* p_cmd = (is_snap) ? &mod->VFE_SnapDemuxConfigCmd :
    &mod->VFE_PreviewDemuxConfigCmd;

#ifndef VFE_2X
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
    parms->update |= VFE_MOD_DEMUX;
    mod->hw_enable = FALSE;
  }
#endif

  if(!mod->enable) {
    CDBG("%s: Demux not enabled", __func__);
    return VFE_SUCCESS;
  }
  if(!mod->update) {
    CDBG("%s: Demux not updated", __func__);
    return VFE_SUCCESS;
  }

  p_cmd->ch0EvenGain = DEMUX_GAIN(mod->gain[index].green_even);
  p_cmd->ch0OddGain = DEMUX_GAIN(mod->gain[index].green_odd);
  p_cmd->ch1Gain = DEMUX_GAIN(mod->gain[index].blue);
  p_cmd->ch2Gain = DEMUX_GAIN(mod->gain[index].red);
  CDBG("Demux update");
  vfe_demux_debug(p_cmd);
  status = vfe_util_write_hw_cmd(parms->camfd, CMD_GENERAL, (void *) p_cmd,
    sizeof(VFE_DemuxConfigCmdType),
    VFE_CMD_DEMUX_UPDATE);

  if (VFE_SUCCESS == status) {
    mod->update = FALSE;
    parms->update |= VFE_MOD_DEMUX;
  }

  return status;
}/*vfe_demux_update*/

/*===========================================================================
 * FUNCTION    - vfe_demux_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_init(int mod_id, void *demux_mod, void *vparms)
{
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  vfe_params_t *parms = (vfe_params_t *) vparms;
  mod->dig_gain[PREV] = 1.0;
  mod->dig_gain[SNAP] = 1.0;
  if (IS_BAYER_FORMAT(parms)) {
    chromatix_parms_type *chromatix_ptr = parms->chroma3a;
#ifndef VFE_2X
    INIT_GAIN(mod->gain[PREV], chromatix_ptr->color_correction_global_gain);
    INIT_GAIN(mod->gain[SNAP], chromatix_ptr->color_correction_global_gain);
#else
    float glob = chromatix_ptr->color_correction_global_gain;
    int i = 0;
    for (i = 0; i < 2; i ++) {
      mod->gain[i].green_odd = glob *
        chromatix_ptr->chromatix_channel_balance_gains.green_odd;
      mod->gain[i].green_even = glob *
        chromatix_ptr->chromatix_channel_balance_gains.green_even;
      mod->gain[i].blue = glob *
        chromatix_ptr->chromatix_channel_balance_gains.blue;
      mod->gain[i].red = glob *
        chromatix_ptr->chromatix_channel_balance_gains.red;
    }
#endif
  } else {
    INIT_GAIN(mod->gain[PREV], 1.0);
    INIT_GAIN(mod->gain[SNAP], 1.0);
  }
  INIT_GAIN(mod->r_gain[PREV], 1.0);
  INIT_GAIN(mod->r_gain[SNAP], 1.0);
  CDBG("%s %5.2f %5.2f %5.2f %5.2f ", __func__,
    mod->gain[PREV].green_odd,
    mod->gain[PREV].green_even,
    mod->gain[PREV].red,
    mod->gain[PREV].blue);
  mod->trigger_enable = TRUE;
  /* update params */
  parms->digital_gain = 1.0;
  parms->aec_gain_adj.total_dig_gain = 1.0;
  parms->aec_gain_adj.cc_gain_adj = 1.0;
  parms->aec_gain_adj.wb_gain_adj = 1.0;
  mod->mode = VFE_OP_MODE_INVALID;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_demux_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_reload_params(int mod_id, void *demux_mod, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  CDBG("%s:", __func__);
  /* to trigger the update */
  mod->reload_params = TRUE;
  return status;
} /*vfe_demux_reload_params*/

/*===========================================================================
 * FUNCTION    - vfe_demux_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_trigger_enable(int mod_id, void *demux_mod,
  void *params, int enable)
{
  CDBG("%s: %d", __func__, enable);
  demux_mod_t* mod = (demux_mod_t *)demux_mod;
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_demux_trigger_enable*/

/*===========================================================================
 * FUNCTION    - vfe_demux_tv_vaidate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_tv_vaidate(int mod_id, void *in, void *out)
{
  CDBG_ERROR("%s: NOT IMPLEMENTED", __func__);
  return VFE_SUCCESS;
} /*vfe_demux_tv_vaidate*/

/*=============================================================================
 * Function:               vfe_demux_deinit
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_demux_deinit(int mod_id, void *demux_mod, void *params)
{
  return VFE_SUCCESS;
} /* vfe_demux_deinit */

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_demux_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  demux_module_t *cmd = (demux_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  status = vfe_util_write_hw_cmd(vfe_params->camfd,
    CMD_GENERAL, (void *)&(cmd->demux_GainConfigCmd),
    sizeof(VFE_DemuxGainCfgCmdType),
    VFE_CMD_DEMUX_R_CHANNEL_GAIN_CONFIG);


  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->demux_cfgCmd),
     sizeof(VFE_DemuxConfigCmdType),
     VFE_CMD_DEMUX_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_demux_plugin_update */
#endif
