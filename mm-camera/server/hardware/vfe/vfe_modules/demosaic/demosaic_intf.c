/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_demosaic_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_ops_init(void *mod)
{
  demosaic_mod_t *pmod = (demosaic_mod_t *)mod;
  memset(pmod, 0x0, sizeof(demosaic_mod_t));
  pmod->ops.init = vfe_demosaic_init;
  pmod->ops.update = vfe_demosaic_update;
  pmod->ops.trigger_update = vfe_demosaic_trigger_update;
  pmod->ops.config = vfe_demosaic_config;
  pmod->ops.enable = vfe_demosaic_enable;
  pmod->ops.reload_params = vfe_demosaic_reload_params;
  pmod->ops.trigger_enable = vfe_demosaic_trigger_enable;
  pmod->ops.test_vector_validate = vfe_demosaic_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_demosaic_deinit;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_demosaic_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demosaic_ops_deinit(void *mod)
{
  demosaic_mod_t *pmod = (demosaic_mod_t *)mod;

  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
