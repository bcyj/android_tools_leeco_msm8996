/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "black_level_v1.h"
#include "vfe.h"

#ifdef ENABLE_BL_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * Function:               vfe_black_level_debug
 *
 * Description:
 *=========================================================================*/
void vfe_black_level_debug(VFE_BlackLevelConfigCmdType *pcmd)
{
  CDBG("VFE_BlackLevelCfgCmd:\n");
  CDBG("lastPixel = %d\n", pcmd->lastPixel);
  CDBG("firstPixel = %d\n", pcmd->firstPixel);
  CDBG("lastLine = %d\n", pcmd->lastLine);
  CDBG("firstLine = %d\n", pcmd->firstLine);
  CDBG("log2PixelsInBlackRegion = %d\n", pcmd->log2PixelsInBlackRegion);
  CDBG("oddColManualBlackCorrection = %d\n",
    pcmd->oddColManualBlackCorrection);
  CDBG("evenColManualBlackCorrection = %d\n",
    pcmd->evenColManualBlackCorrection);
} /* vfe_black_level_debug */

/*===========================================================================
 * Function:               black_level_set_params
 *
 * Description:
 *=========================================================================*/
void black_level_set_params(chromatix_black_level_offset_type *chr_params,
  VFE_BlackLevelConfigCmdType *pcmd, vfe_params_t *params)
{
  pcmd->blackLevelMode = VFE_MANUAL_BLACK_LEVEL_CORRECTION;
#if 0
  /* In the target tree the values are zeroes. Hence keeping the same
    until the modules are enabled */
  pcmd->lastPixel = params->sensor_parms.lastPixel;
  pcmd->firstPixel = params->sensor_parms.firstPixel;
  pcmd->lastLine = params->sensor_parms.lastLine;
  pcmd->firstLine = params->sensor_parms.firstLine;
  pcmd->log2PixelsInBlackRegion = 0; /*TODO*/
#endif
  pcmd->oddColManualBlackCorrection = chr_params->odd_columns;
  pcmd->evenColManualBlackCorrection = chr_params->even_columns;
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

  chromatix_black_level_offset_type *normal_light_black_level_offset = NULL;
  chromatix_parms_type *chroma_ptr = params->chroma3a;
  memset(mod, 0x0, sizeof(black_level_mod_t));
  mod->trigger_enable = TRUE;

  mod->bl_info[PREV] = chroma_ptr->normal_light_black_level_offset;
  mod->bl_info[SNAP] = chroma_ptr->normal_light_black_level_offset_snapshot;
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

  black_level_set_params(&mod->bl_info[index], p_cmd, params);

  vfe_black_level_debug(p_cmd);

  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_GENERAL, p_cmd,
    sizeof(VFE_BlackLevelConfigCmdType),
    VFE_CMD_BLACK_LEVEL_CFG);

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
  chromatix_black_level_offset_type *p_bl_offset = NULL;
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
    p_bl_offset = &(chroma_ptr->normal_light_black_level_offset);
  } else {
    p_trigger_point = &(chroma_ptr->blk_snapshot_lowlight_trigger);
    max_blk_increase = chroma_ptr->max_blk_increase_snapshot;
    p_bl_offset = &(chroma_ptr->normal_light_black_level_offset_snapshot);
  }

  ratio = vfe_util_get_aec_ratio(tuning_type, p_trigger_point, params);

  update_blc =
    (mod->op_mode != params->vfe_op_mode) ||
    !F_EQUAL(mod->ratio, ratio);

  CDBG("%s: update_blc %d :: ratio : %f", __func__, update_blc, ratio);

  if (update_blc) {
    p_bl_offset->even_columns = (uint16_t)(p_bl_offset->even_columns -
      (1.0-ratio) * max_blk_increase);
    p_bl_offset->odd_columns = (uint16_t)(p_bl_offset->odd_columns -
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

  if (!mod->enable) {
    CDBG("%s: Black_Level not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->update) {
    CDBG("%s: Black_Level not updated", __func__);
    return VFE_SUCCESS;
  }

  CDBG("Black_Level update");
  black_level_set_params(&mod->bl_info[index], p_cmd, params);

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

vfe_status_t is_vfe_black_level_reconfig(vfe_ctrl_info_t *p_obj, void *parm)
{
  uint32_t *ptr = (uint32_t *)parm;

  *ptr = p_obj->vfe_module.linear_mod.vfe_reconfig;
  p_obj->vfe_module.linear_mod.vfe_reconfig = 0;
  return VFE_SUCCESS;
}

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
