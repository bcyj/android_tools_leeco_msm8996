/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_WB_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define WB_START_REG 0x384

#define INIT_WB_GAIN(p,g,b,r) ({ \
  p.g_gain = g; \
  p.b_gain = b; \
  p.r_gain = r; \
})

/*===========================================================================
 * FUNCTION    - vfe_wb_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_wb_debug(VFE_WhiteBalanceConfigCmdType* p_cmd)
{
  CDBG("VFE_WhiteBalanceCfgCmd.ch0Gain = %d\n",
    p_cmd->ch0Gain);
  CDBG("VFE_WhiteBalanceCfgCmd.ch1Gain = %d\n",
    p_cmd->ch1Gain);
  CDBG("VFE_WhiteBalanceCfgCmd.ch2Gain = %d\n",
    p_cmd->ch2Gain);
}/*vfe_wb_debug*/


/*===========================================================================
 * FUNCTION      vfe_wb_config
 *
 * DESCRIPTION
 *==========================================================================*/
vfe_status_t vfe_wb_config(int mod_id, void* mod_wb, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  wb_mod_t* mod = (wb_mod_t *)mod_wb;
  vfe_params_t *params = (vfe_params_t *)vparams;

  int is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;
  if (!mod->enable) {
    CDBG("%s: WB not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s: %s dig_gain %5.3f", __func__,
    (is_snap) ? "Snapshot" : "Preview", mod->dig_gain[index]);
  mod->VFE_WhiteBalanceCfgCmd.ch0Gain =
    WB_GAIN(mod->awb_gain[index].g_gain * mod->dig_gain[index]);
  mod->VFE_WhiteBalanceCfgCmd.ch1Gain =
    WB_GAIN(mod->awb_gain[index].b_gain * mod->dig_gain[index]);
  mod->VFE_WhiteBalanceCfgCmd.ch2Gain =
    WB_GAIN(mod->awb_gain[index].r_gain * mod->dig_gain[index]);
  vfe_wb_debug(&mod->VFE_WhiteBalanceCfgCmd);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    (void *) &(mod->VFE_WhiteBalanceCfgCmd),
    sizeof(mod->VFE_WhiteBalanceCfgCmd), VFE_CMD_WB_CFG);

  if (VFE_SUCCESS == status)
    mod->update = FALSE;
  return status;
} /* vfe_wb_config */

/*===========================================================================
 * FUNCTION      vfe_wb_set_manual_wb
 *
 * DESCRIPTION
 *==========================================================================*/
vfe_status_t vfe_wb_set_manual_wb(int mod_id, void *mod_wb, void *parms)
{
  vfe_status_t status = VFE_SUCCESS;
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)parms;

  int is_snap = IS_SNAP_MODE(params);
  chromatix_parms_type *chroma_ptr = params->chroma3a;
  int index = (is_snap) ? SNAP : PREV;
  CDBG("%s: old %f %f %f new %f %f %f", __func__,
    mod->awb_gain[index].g_gain,
    mod->awb_gain[index].b_gain,
    mod->awb_gain[index].r_gain,
    params->awb_params.gain.g_gain,
    params->awb_params.gain.b_gain,
    params->awb_params.gain.r_gain);
  if (IS_MANUAL_WB(params)) {
    mod->awb_gain[index].g_gain = params->awb_params.gain.g_gain;
    mod->awb_gain[index].b_gain = params->awb_params.gain.b_gain;
    mod->awb_gain[index].r_gain = params->awb_params.gain.r_gain;
  } else { /*switch to AWB*/
#ifdef RESET_WB_VALUES
    float b_gain = chroma_ptr->chromatix_tl84_white_balance.b_gain *
      chroma_ptr->awb_reference_hw_rolloff[AGW_AWB_INDOOR_WARM_FLO].
      blue_gain_adj;
    float r_gain = chroma_ptr->chromatix_tl84_white_balance.r_gain *
      chroma_ptr->awb_reference_hw_rolloff[AGW_AWB_INDOOR_WARM_FLO].
      red_gain_adj;
    float g_gain = chroma_ptr->chromatix_tl84_white_balance.g_gain;
    INIT_WB_GAIN(mod->awb_gain[PREV], g_gain, b_gain, r_gain);
    INIT_WB_GAIN(mod->awb_gain[SNAP], g_gain, b_gain, r_gain);
    params->awb_params.gain.g_gain = g_gain;
    params->awb_params.gain.b_gain = b_gain;
    params->awb_params.gain.r_gain = r_gain;
#endif
  }
  mod->update = TRUE;
  return status;
} /*vfe_wb_set_manual_wb*/

/*===========================================================================
 * FUNCTION      vfe_wb_set_bestshot
 *
 * DESCRIPTION
 *==========================================================================*/
vfe_status_t vfe_wb_set_bestshot(int mod_id, void *mod_wb, void *parms,
  camera_bestshot_mode_type mode)
{
  vfe_status_t status = VFE_SUCCESS;
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)parms;

  int8_t is_snapmode = IS_SNAP_MODE(params);
  int index = (is_snapmode) ? SNAP : PREV;

  CDBG("%s: mode %d", __func__, mode);
  switch(mode) {
    case CAMERA_BESTSHOT_FIREWORKS:
      params->wb = CAMERA_WB_CLOUDY_DAYLIGHT;
      break;
    case CAMERA_BESTSHOT_SUNSET:
    case CAMERA_BESTSHOT_CANDLELIGHT:
      params->wb = CAMERA_WB_INCANDESCENT;
      break;
    default:
      params->wb = CAMERA_WB_AUTO;
      break;
  }
  status = vfe_wb_set_manual_wb(mod_id, mod, params);
  if (VFE_SUCCESS == status)
    mod->update = TRUE;
  return status;
}/*vfe_wb_set_bestshot*/

/*===========================================================================
 * FUNCTION      vfe_wb_update
 *
 * DESCRIPTION
 *==========================================================================*/
vfe_status_t vfe_wb_update(int mod_id, void *mod_wb, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)vparms;

  int is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;

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
    params->update |= VFE_MOD_WB;
    mod->hw_enable = FALSE;
  }

  if (!mod->enable) {
    CDBG("%s: WB not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!mod->update) {
    CDBG("%s: WB not updated", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s: %s dig_gain %5.3f", __func__,
    (is_snap) ? "Snapshot" : "Preview", mod->dig_gain[index]);
  mod->VFE_WhiteBalanceCfgCmd.ch0Gain =
    WB_GAIN(mod->awb_gain[index].g_gain * mod->dig_gain[index]);
  mod->VFE_WhiteBalanceCfgCmd.ch1Gain =
    WB_GAIN(mod->awb_gain[index].b_gain * mod->dig_gain[index]);
  mod->VFE_WhiteBalanceCfgCmd.ch2Gain =
    WB_GAIN(mod->awb_gain[index].r_gain * mod->dig_gain[index]);
  CDBG("White balance update");
  vfe_wb_debug(&mod->VFE_WhiteBalanceCfgCmd);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    (void *) &(mod->VFE_WhiteBalanceCfgCmd),
    sizeof(mod->VFE_WhiteBalanceCfgCmd), VFE_CMD_WB_UPDATE);

  if (VFE_SUCCESS == status) {
    mod->update = FALSE;
    params->update |= VFE_MOD_WB;
  }
  return status;
} /* vfe_wb_update */

/*===========================================================================
 * FUNCTION      vfe_wb_trigger_update
 *
 * DESCRIPTION
 *==========================================================================*/
vfe_status_t vfe_wb_trigger_update(int mod_id, void *mod_wb, void *vparms)
{
  vfe_status_t status = VFE_SUCCESS;
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)vparms;

  int is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;
  int update = FALSE;
  if (!mod->enable) {
    CDBG("%s: WB not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->trigger_enable) {
    CDBG("%s: WB trigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (params->demosaic_wb_not_present && is_snap) {
    CDBG("%s: WB update not required in snapshot, hardcode the values",
      __func__);
    INIT_WB_GAIN(mod->awb_gain[SNAP], 1.0, 1.0, 1.0);
    return VFE_SUCCESS;
  }

  CDBG("%s: snap %d old %f %f %f new %f %f %f", __func__, is_snap,
    mod->awb_gain[index].g_gain,
    mod->awb_gain[index].b_gain,
    mod->awb_gain[index].r_gain,
    params->awb_params.gain.g_gain,
    params->awb_params.gain.b_gain,
    params->awb_params.gain.r_gain);

  if ((is_snap || !IS_MANUAL_WB(params)) &&
    !WB_GAIN_EQUAL(params->awb_params.gain, mod->awb_gain[index])
    && !WB_GAIN_EQ_ZERO(params->awb_params.gain)) {
    mod->awb_gain[index].g_gain = params->awb_params.gain.g_gain;
    mod->awb_gain[index].b_gain = params->awb_params.gain.b_gain;
    mod->awb_gain[index].r_gain = params->awb_params.gain.r_gain;
    params->awb_gain_update = TRUE;
    update = TRUE;
  } 

  if (!F_EQUAL(params->aec_gain_adj.wb_gain_adj,
    mod->dig_gain[index])) {
    mod->dig_gain[index] = params->aec_gain_adj.wb_gain_adj;
    update = TRUE;
  } 

  if (!update) {
    CDBG("%s: No update required", __func__);
    return VFE_SUCCESS;
  }

  mod->update = TRUE;
  CDBG("%s: updated", __func__);
  return status;
} /* vfe_wb_trigger_update */

/*===========================================================================
 * Function:           vfe_wb_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_wb_enable(int mod_id, void *mod_wb, void *vparms,
  int8_t wb_enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)vparms;

  if (!IS_BAYER_FORMAT(params))
    wb_enable = FALSE;
  params->moduleCfg->whiteBalanceEnable = wb_enable;

  CDBG("%s: enable %d", __func__, wb_enable);
  if (hw_write && (mod->enable == wb_enable))
    return VFE_SUCCESS;

  mod->enable = wb_enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (wb_enable) ?
      (params->current_config | VFE_MOD_WB)
      : (params->current_config & ~VFE_MOD_WB);
  }
  return status;
} /* vfe_wb_enable */

/*===========================================================================
 * Function:           vfe_wb_init
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_wb_init(int mod_id, void *mod_wb, void *vparms)
{
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)vparms;

  chromatix_parms_type *chroma_ptr = params->chroma3a;
  float b_gain = chroma_ptr->chromatix_tl84_white_balance.b_gain *
    chroma_ptr->awb_reference_hw_rolloff[AGW_AWB_INDOOR_WARM_FLO].
    blue_gain_adj;
  float r_gain = chroma_ptr->chromatix_tl84_white_balance.r_gain *
    chroma_ptr->awb_reference_hw_rolloff[AGW_AWB_INDOOR_WARM_FLO].
    red_gain_adj;
  float g_gain = chroma_ptr->chromatix_tl84_white_balance.g_gain;
  INIT_WB_GAIN(mod->awb_gain[PREV], g_gain, b_gain, r_gain);
  if (params->demosaic_wb_not_present)
    INIT_WB_GAIN(mod->awb_gain[SNAP], 1.0, 1.0, 1.0);
  else
    INIT_WB_GAIN(mod->awb_gain[SNAP], g_gain, b_gain, r_gain);
  mod->dig_gain[PREV] = 1.0;
  mod->dig_gain[SNAP] = 1.0;
  mod->trigger_enable = TRUE;
  /*params set*/
  params->awb_params.gain.g_gain = g_gain;
  params->awb_params.gain.b_gain = b_gain;
  params->awb_params.gain.r_gain = r_gain;
  params->aec_gain_adj.wb_gain_adj = 1.0;
  params->awb_gain_update = FALSE;

  CDBG("%s: old %f %f %f new %f %f %f", __func__,
    mod->awb_gain[0].g_gain,
    mod->awb_gain[0].b_gain,
    mod->awb_gain[0].r_gain,
    params->awb_params.gain.g_gain,
    params->awb_params.gain.b_gain,
    params->awb_params.gain.r_gain);
  return VFE_SUCCESS;
} /* vfe_wb_init */

/*===========================================================================
 * FUNCTION    - vfe_wb_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_wb_trigger_enable(int mod_id, void *mod_wb, void *vparms,
  int enable)
{
  wb_mod_t* mod = (wb_mod_t*)mod_wb;
  vfe_params_t* params = (vfe_params_t*)vparms;

  CDBG("%s: %d", __func__, enable);
  mod->trigger_enable = enable;
  return VFE_SUCCESS;
} /*vfe_wb_trigger_enable*/

/*===========================================================================
 * FUNCTION    - wb_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void wb_populate_data(uint32_t *reg, VFE_WhiteBalanceConfigCmdType *pcmd)
{
  reg += (WB_START_REG/4);
  memcpy((void *)pcmd, (void *)reg, sizeof(VFE_WhiteBalanceConfigCmdType));
}/*wb_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_wb_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_wb_tv_validate(int mod_id, void *test_input,
  void *test_output)
{
  VFE_WhiteBalanceConfigCmdType in, out;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;

  wb_populate_data(input->reg_dump, &in);
  wb_populate_data(output->reg_dump_data, &out);

  if (!MATCH(in.ch0Gain, out.ch0Gain, 0))
    CDBG_TV("%s: ch0Gain in %d out %d doesnt match", __func__,
    in.ch0Gain, out.ch0Gain);

  if (!MATCH(in.ch1Gain, out.ch1Gain, 0))
    CDBG_TV("%s: ch1Gain in %d out %d doesnt match", __func__,
    in.ch1Gain, out.ch1Gain);

  if (!MATCH(in.ch2Gain, out.ch2Gain, 0))
    CDBG_TV("%s: ch2Gain in %d out %d doesnt match", __func__,
    in.ch2Gain, out.ch2Gain);

  return VFE_SUCCESS;
}/*vfe_wb_tv_validate*/

/*===========================================================================
 * FUNCTION    - vfe_wb_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_wb_deinit(int mod_id, void *module, void *params)
{
  wb_mod_t *wb_mod = (wb_mod_t *)module;
  memset(wb_mod, 0 , sizeof(wb_mod_t));
  return VFE_SUCCESS;
}
#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_wb_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_wb_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  wb_module_t *cmd = (wb_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->WB_cfgCmd),
     sizeof(VFE_WhiteBalanceConfigCmdType),
     VFE_CMD_WB_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_wb_plugin_update */
#endif
