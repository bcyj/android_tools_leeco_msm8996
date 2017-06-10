/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_color_correct_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_ops_init(void *mod)
{
  color_correct_mod_t *pmod = (color_correct_mod_t *)mod;
  memset(pmod, 0x0, sizeof(color_correct_mod_t));
  pmod->ops.init = vfe_color_correct_init;
  pmod->ops.update = vfe_color_correct_update;
  pmod->ops.trigger_update = vfe_color_correct_trigger_update;
  pmod->ops.config = vfe_color_correct_config;
  pmod->ops.enable = vfe_color_correct_enable;
  pmod->ops.reload_params = vfe_color_correct_reload_params;
  pmod->ops.trigger_enable = vfe_color_correct_trigger_enable;
  pmod->ops.test_vector_validate = vfe_color_correct_tv_validate;
  pmod->ops.set_effect = vfe_color_correct_set_effect;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = vfe_color_correct_set_bestshot;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_color_correct_deinit;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_color_correct_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_correct_ops_deinit(void *mod)
{
  color_correct_mod_t *pmod = (color_correct_mod_t *)mod;

  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
