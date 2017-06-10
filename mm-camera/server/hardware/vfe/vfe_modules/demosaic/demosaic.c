/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "demosaic.h"

#ifdef ENABLE_DEMOSAIC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * Function:               vfe_demosaic_debug
 *
 * Description:
 *=========================================================================*/
void vfe_demosaic_debug(VFE_DemosaicConfigCmdType *pcmd)
{
  CDBG("VFE_DemosaicConfigCmdType:\n");
  CDBG("slopeShift = %d\n", pcmd->slopeShift);
} /* vfe_demosaic_debug */

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_init(demosaic_mod_t *mod, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  memset(mod, 0x0, sizeof(demosaic_mod_t));

  if (!IS_BAYER_FORMAT(params))
    return VFE_SUCCESS;

  mod->cmd.slopeShift = 0;
  return status;
}/*vfe_demosaic_init*/

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_enable(demosaic_mod_t *mod, vfe_params_t *params,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  CDBG("%s, enable/disable demosaic module = %d",__func__, enable);
  params->moduleCfg->demosaicEnable = enable;

  if (hw_write && (mod->enable == enable))
    return VFE_SUCCESS;

  mod->enable = enable;
  mod->hw_enable = hw_write;
  if (hw_write) {
    params->current_config = (enable) ?
      (params->current_config | VFE_MOD_DEMOSAIC)
      : (params->current_config & ~VFE_MOD_DEMOSAIC);
  }
  return status;
}/*vfe_demosaic_enable*/

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_config(demosaic_mod_t *mod, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int is_snap = IS_SNAP_MODE(params);
  VFE_DemosaicConfigCmdType *pcmd =  &mod->cmd;

  CDBG("%s: %s", __func__, (is_snap) ? "Snapshot" : "Preview");
  vfe_demosaic_debug(pcmd);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    (void *) pcmd, sizeof(VFE_DemosaicConfigCmdType),
    VFE_CMD_DEMOSAICV3)) {
    CDBG_HIGH("%s: demosaic config for operation mode = %d failed\n", __func__,
      params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return status;
}/*vfe_demosaic_config*/

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_update(demosaic_mod_t *mod, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;

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
    params->update |= VFE_MOD_DEMOSAIC;
    mod->hw_enable = FALSE;
  }
  return status;
}/*vfe_demosaic_update*/

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_trigger_update(demosaic_mod_t *mod,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  return status;
}/*vfe_demosaic_trigger_update*/

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_trigger_enable(demosaic_mod_t* mod,
  vfe_params_t* params, int enable)
{
  vfe_status_t status = VFE_SUCCESS;
  return status;
}/*vfe_demosaic_trigger_enable*/

/*===========================================================================
 * FUNCTION    -  vfe_demosaic_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_reload_params(demosaic_mod_t* mod,
  vfe_params_t* params)
{
  vfe_status_t status = VFE_SUCCESS;
  return status;
}/*vfe_demosaic_reload_params*/
