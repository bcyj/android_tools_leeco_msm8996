/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_mce_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_ops_init(void *mod)
{
  mce_mod_t *pmod = (mce_mod_t *)mod;
  memset(pmod, 0 , sizeof(mce_mod_t));
  pmod->ops.init = vfe_mce_init;
  pmod->ops.update = vfe_mce_update;
  pmod->ops.trigger_update = vfe_mce_trigger_update;
  pmod->ops.config = vfe_mce_config;
  pmod->ops.enable = vfe_mce_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = vfe_mce_trigger_enable;
  pmod->ops.test_vector_validate = vfe_mce_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.deinit = vfe_mce_deinit;

  return 0;
}

/*===========================================================================
 * FUNCTION    - vfe_mce_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_mce_ops_deinit(void *mod)
{
  mce_mod_t *pmod = (mce_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
