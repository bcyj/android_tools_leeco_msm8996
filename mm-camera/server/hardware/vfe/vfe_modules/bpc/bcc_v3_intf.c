/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BCC_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_bcc_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bcc_ops_init(void *mod)
{
  bcc_mod_t *pmod = (bcc_mod_t *)mod;
  memset(pmod, 0 , sizeof(bcc_mod_t));
  pmod->ops.init = vfe_demosaic_bcc_init;
  pmod->ops.update = vfe_demosaic_bcc_update;
  pmod->ops.trigger_update = vfe_demosaic_bcc_trigger_update;
  pmod->ops.config = vfe_demosaic_bcc_config;
  pmod->ops.enable = vfe_demosaic_bcc_enable;
  pmod->ops.reload_params = vfe_bcc_reload_params;
  pmod->ops.trigger_enable = vfe_bcc_trigger_enable;
  pmod->ops.test_vector_validate = vfe_bcc_test_vector_validation;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_demosaic_bcc_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bcc_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bcc_ops_deinit(void *mod)
{
  bcc_mod_t *pmod = (bcc_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}

