/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_la_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_la_ops_init(void *mod)
{
  la_mod_t *pmod = (la_mod_t *)mod;
  memset(pmod, 0x0, sizeof(la_mod_t));
  pmod->ops.init = vfe_la_init;
  pmod->ops.update = vfe_la_update;
  pmod->ops.trigger_update = vfe_la_trigger_update;
  pmod->ops.config = vfe_la_config;
  pmod->ops.enable = vfe_la_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = vfe_la_trigger_enable;
  pmod->ops.test_vector_validate = NULL;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = vfe_la_set_spl_effect;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = vfe_la_set_bestshot;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_la_deinit;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_la_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_la_ops_deinit(void *mod)
{
  la_mod_t *pmod = (la_mod_t *)mod;

  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
