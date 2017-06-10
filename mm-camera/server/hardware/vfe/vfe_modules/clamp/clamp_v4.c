/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_clamp_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clamp_init(int mod_id, void *mod_clamp, void* parm)
{
  clamp_mod_t *mod = (clamp_mod_t *)mod_clamp;

  mod->clamp_cfg_cmd.enc_clamp_config.cbChanMax  = 255;
  mod->clamp_cfg_cmd.enc_clamp_config.cbChanMin  = 0;
  mod->clamp_cfg_cmd.enc_clamp_config.crChanMax  = 255;
  mod->clamp_cfg_cmd.enc_clamp_config.crChanMin  = 0;
  mod->clamp_cfg_cmd.enc_clamp_config.yChanMax   = 255;
  mod->clamp_cfg_cmd.enc_clamp_config.yChanMin   = 0;

  mod->clamp_cfg_cmd.view_clamp_config.cbChanMax  = 255;
  mod->clamp_cfg_cmd.view_clamp_config.cbChanMin  = 0;
  mod->clamp_cfg_cmd.view_clamp_config.crChanMax  = 255;
  mod->clamp_cfg_cmd.view_clamp_config.crChanMin  = 0;
  mod->clamp_cfg_cmd.view_clamp_config.yChanMax   = 255;
  mod->clamp_cfg_cmd.view_clamp_config.yChanMin   = 0;

  return VFE_SUCCESS;
} /* vfe_clamp_init */

/*===========================================================================
 * FUNCTION.   - vfe_clamp_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_clamp_enable(int mod_id, void *mod_clamp, void* parm,
  int8_t enable, int8_t hw_write)
{
  clamp_mod_t *mod = (clamp_mod_t *)mod_clamp;
  vfe_params_t* p_obj = (vfe_params_t *)parm;

  mod->clamp_enable = enable;
  CDBG("%s, enable/disable clamp module = %d",__func__, enable);
  if (hw_write) {
    p_obj->current_config = (enable) ?
      (p_obj->current_config | VFE_MOD_CLAMP)
      : (p_obj->current_config & ~VFE_MOD_CLAMP);
  }
  return VFE_SUCCESS;
} /* vfe_clamp_enable */

/*===========================================================================
 * FUNCTION    - vfe_clamp_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clamp_config(int mod_id, void *mod_clamp, void* parm)
{
  clamp_mod_t *mod = (clamp_mod_t *)mod_clamp;
  vfe_params_t* p_obj = (vfe_params_t *)parm;
  if (!mod->clamp_enable) {
    CDBG("%s: clamp not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd, CMD_GENERAL,
    (void *) &(mod->clamp_cfg_cmd), sizeof(mod->clamp_cfg_cmd),
    VFE_CMD_OUT_CLAMP_CFG)) {
    CDBG_HIGH("%s: iclamp config for operation mode = %d failed\n", __func__,
      p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_clamp_config */

/*===========================================================================
 * FUNCTION    - vfe_clamp_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clamp_deinit(int mod_id, void *mod_clamp, void* parm)
{
  return VFE_SUCCESS;
} /* vfe_clamp_init */

