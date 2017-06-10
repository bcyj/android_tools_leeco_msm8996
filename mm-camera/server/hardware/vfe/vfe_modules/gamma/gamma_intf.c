/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_GAMMA_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_gamma_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_gamma_ops_init(void *mod)
{
  gamma_mod_t *pmod = (gamma_mod_t *)mod;
  memset(pmod, 0x0, sizeof(gamma_mod_t));
  pmod->ops.init = vfe_gamma_init;
  pmod->ops.update = vfe_gamma_update;
  pmod->ops.trigger_update = vfe_gamma_trigger_update;
  pmod->ops.config = vfe_gamma_config;
  pmod->ops.enable = vfe_gamma_enable;
  pmod->ops.reload_params = vfe_gamma_reload_params;
  pmod->ops.trigger_enable = vfe_gamma_trigger_enable;
  pmod->ops.test_vector_validate = vfe_gamma_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = vfe_gamma_set_spl_effect;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = vfe_gamma_set_bestshot;
  pmod->ops.set_contrast = vfe_gamma_set_contrast;
  pmod->ops.deinit = vfe_gamma_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_gamma_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_gamma_ops_deinit(void *mod)
{
  gamma_mod_t *pmod = (gamma_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}


