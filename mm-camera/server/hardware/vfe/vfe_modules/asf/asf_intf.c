/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <string.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_ASF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_asf_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_ops_init(void *mod)
{
  asf_mod_t *pmod = (asf_mod_t *)mod;
  memset(pmod, 0 , sizeof(asf_mod_t));
  pmod->ops.init = vfe_asf_init;
  pmod->ops.update = vfe_asf_update;
  pmod->ops.trigger_update = vfe_asf_trigger_update;
  pmod->ops.config = vfe_asf_config;
  pmod->ops.enable = vfe_asf_enable;
  pmod->ops.reload_params = vfe_asf_reload_params;
  pmod->ops.trigger_enable = vfe_asf_trigger_enable;
  pmod->ops.test_vector_validate = vfe_asf_test_vector_validation;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = vfe_asf_set_special_effect;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = vfe_asf_set_bestshot;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_asf_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_asf_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_asf_ops_deinit(void *mod)
{
  asf_mod_t *pmod = (asf_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
