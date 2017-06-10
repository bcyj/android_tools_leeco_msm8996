/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_ABF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif


/*===========================================================================
 * FUNCTION    - vfe_abf_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_ops_init(void *mod)
{
  abf_mod_t *pmod = (abf_mod_t *)mod;
  memset(pmod, 0 , sizeof(abf_mod_t));
  pmod->ops.init = vfe_abf_init;
  pmod->ops.update = vfe_abf_update;
  pmod->ops.trigger_update = vfe_abf_trigger_update;
  pmod->ops.config = vfe_abf_config;
  pmod->ops.enable = vfe_abf_enable;
  pmod->ops.reload_params = vfe_abf_reload_params;
  pmod->ops.trigger_enable = vfe_abf_trigger_enable;
  pmod->ops.test_vector_validate = vfe_abf_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_abf_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_abf_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_abf_ops_deinit(void *mod)
{
  abf_mod_t *pmod = (abf_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
