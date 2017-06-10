/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "black_level.h"

/*===========================================================================
 * Function:               vfe_black_level_debug
 *
 * Description:
 *=========================================================================*/
void vfe_black_level_debug(VFE_BlackLevelConfigCmdType *pcmd)
{
  CDBG("VFE_BlackLevelCfgCmd:\n");
  CDBG("eventEvenAdjustment = %d\n", pcmd->evenEvenAdjustment);
  CDBG("evenOddAdjustment = %d\n", pcmd->evenOddAdjustment);
  CDBG("oddEvenAdjustment = %d\n", pcmd->oddEvenAdjustment);
  CDBG("oddOddAdjustment = %d\n", pcmd->oddOddAdjustment);
} /* vfe_black_level_debug */

/*===========================================================================
 * Function:               black_level_set_params
 *
 * Description:
 *=========================================================================*/
void black_level_set_params(chromatix_4_channel_black_level *params,
  VFE_BlackLevelConfigCmdType *pcmd)
{
  pcmd->oddOddAdjustment = params->black_odd_row_odd_col;
  pcmd->oddEvenAdjustment = params->black_odd_row_even_col;
  pcmd->evenOddAdjustment = params->black_even_row_odd_col;
  pcmd->evenEvenAdjustment = params->black_even_row_even_col;
} /* black_level_set_params */

/*===========================================================================
 * FUNCTION    - vfe_black_level_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_black_level_init(black_level_mod_t *mod,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    return VFE_SUCCESS;

  chromatix_4_channel_black_level *normal_light_4_channel = NULL;
  chromatix_parms_type *chroma_ptr = params->chroma3a;
  memset(mod, 0x0, sizeof(black_level_mod_t));
  mod->trigger_enable = TRUE;

  mod->bl_info[PREV] = chroma_ptr->normal_light_4_channel;
  mod->bl_info[SNAP] = chroma_ptr->normal_light_4_channel_snapshot;
  return status;
} /*vfe_black_level_init*/

/*===========================================================================
 * FUNCTION    - vfe_black_level_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_black_level_config(black_level_mod_t *mod,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;
  VFE_BlackLevelConfigCmdType* p_cmd = (!is_snap) ?
    &mod->preview_cmd : &mod->snapshot_cmd;

  if (!mod->enable) {
    CDBG("%s: Black_Level not enabled", __func__);
    return VFE_SUCCESS;
  }
  CDBG("Black Level cfg %s",
    (is_snap) ? "Snapshot" : "Preview");

  black_level_set_params(&mod->bl_info[index], p_cmd);

  vfe_black_level_debug(p_cmd);

  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_GENERAL, p_cmd,
    sizeof(VFE_BlackLevelConfigCmdType),
    VFE_CMD_BLACK_LEVEL_CFG);

  if(status != VFE_SUCCESS) {
    CDBG_ERROR("%s: failed \n",__func__);
  }

  if (VFE_SUCCESS == status)
    mod->update = FALSE;
  return status;
}/*vfe_black_level_config*/

/*===========================================================================
 * FUNCTION    - vfe_black_level_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_black_level_trigger_update(black_level_mod_t *mod,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;
  chromatix_parms_type *chroma_ptr = params->chroma3a;
  tuning_control_type tuning_type = chroma_ptr->control_blk;
  trigger_point_type  *p_trigger_point = NULL;
  float ratio;
  uint16_t max_blk_increase;
  chromatix_4_channel_black_level *normal_light_4_channel = NULL;
  int update_blc = FALSE;

  if (!mod->enable) {
    CDBG("%s: Black_Level not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->trigger_enable) {
    CDBG("%s: Black_Level trigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!is_snap && !vfe_util_aec_check_settled(&params->aec_params)) {
    CDBG("%s: AEC not settled", __func__);
    return VFE_SUCCESS;
  }

  if (!is_snap) {
    p_trigger_point = &(chroma_ptr->blk_lowlight_trigger);
    max_blk_increase = chroma_ptr->max_blk_increase;
    normal_light_4_channel = &(chroma_ptr->normal_light_4_channel);
  } else {
    p_trigger_point = &(chroma_ptr->blk_snapshot_lowlight_trigger);
    max_blk_increase = chroma_ptr->max_blk_increase_snapshot;
    normal_light_4_channel = &(chroma_ptr->normal_light_4_channel_snapshot);
  }

  ratio = vfe_util_get_aec_ratio(tuning_type, p_trigger_point, params);

  update_blc =
    (mod->op_mode != params->vfe_op_mode) ||
    !F_EQUAL(mod->ratio, ratio);

  CDBG("%s: update_blc %d", __func__, update_blc);

  if (update_blc) {
    normal_light_4_channel->black_even_row_even_col = (uint16_t)
      (normal_light_4_channel->black_even_row_even_col -
      (1.0-ratio) * max_blk_increase);
    normal_light_4_channel->black_even_row_odd_col = (uint16_t)
      (normal_light_4_channel->black_even_row_odd_col -
      (1.0-ratio) * max_blk_increase);
    normal_light_4_channel->black_odd_row_even_col = (uint16_t)
      (normal_light_4_channel->black_odd_row_even_col -
      (1.0-ratio) * max_blk_increase);
    normal_light_4_channel->black_odd_row_odd_col = (uint16_t)
      (normal_light_4_channel->black_odd_row_odd_col -
      (1.0-ratio) * max_blk_increase);
    mod->op_mode = params->vfe_op_mode;
    mod->ratio = ratio;
    mod->update = TRUE;
  }
  return status;
}/*vfe_black_level_trigger_update*/

/*===========================================================================
 * FUNCTION    - vfe_black_level_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_black_level_update(black_level_mod_t *mod,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(params);
  int index = (is_snap) ? SNAP : PREV;
  VFE_BlackLevelConfigCmdType* p_cmd = (!is_snap) ?
    &mod->preview_cmd : &mod->snapshot_cmd;

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
    params->update |= VFE_MOD_LINEARIZATION;
    mod->hw_enable = FALSE;
  }

  if (!mod->enable) {
    CDBG("%s: Black_Level not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->update) {
    CDBG("%s: Black_Level not updated", __func__);
    return VFE_SUCCESS;
  }

  CDBG("Black_Level update");
  black_level_set_params(&mod->bl_info[index], p_cmd);

  vfe_black_level_debug(p_cmd);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, p_cmd,
    sizeof(VFE_BlackLevelConfigCmdType),
    VFE_CMD_BLACK_LEVEL_CFG);

  if (VFE_SUCCESS == status) {
    mod->update = FALSE;
    params->update |= VFE_MOD_LINEARIZATION;
  }

  return status;
}/*vfe_black_level_update*/

/*===========================================================================
 * FUNCTION    - vfe_black_level_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_black_level_enable(black_level_mod_t* mod,
  vfe_params_t* params, int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  params->moduleCfg->blackLevelCorrectionEnable = enable;

  if (hw_write && (mod->enable == enable))
    return VFE_SUCCESS;

  mod->enable = enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_LINEARIZATION)
      : (params->current_config & ~VFE_MOD_LINEARIZATION);
  }
  return status;
}/*vfe_black_level_enable*/

/*===========================================================================
 * FUNCTION    - vfe_black_level_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_black_level_trigger_enable(black_level_mod_t *mod,
  vfe_params_t *params, int enable)
{
  vfe_status_t status = VFE_SUCCESS;
  mod->trigger_enable = enable;
  return status;
}/*vfe_black_level_trigger_enable*/
