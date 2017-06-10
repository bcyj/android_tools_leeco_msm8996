/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_linearization_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_ops_init(void *mod)
{
  linear_mod_t *pmod = (linear_mod_t *)mod;
  memset(pmod, 0x0, sizeof(gamma_mod_t));
  pmod->ops.init = vfe_linearization_init;
  pmod->ops.update = vfe_linearization_update;
  pmod->ops.trigger_update = vfe_linearization_trigger_update;
  pmod->ops.config = vfe_linearization_config;
  pmod->ops.enable = vfe_linearization_enable;
  pmod->ops.reload_params = vfe_linearization_reload_params;
  pmod->ops.trigger_enable = vfe_linearization_trigger_enable;
  pmod->ops.test_vector_validate = vfe_linearization_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_linearization_deinit;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_linearization_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_ops_deinit(void *mod)
{
  linear_mod_t *pmod = (linear_mod_t *)mod;

  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
