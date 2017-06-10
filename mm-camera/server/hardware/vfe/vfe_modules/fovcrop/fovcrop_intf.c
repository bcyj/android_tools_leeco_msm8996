/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_FOV_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_fov_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_ops_init(void *mod)
{
  fov_mod_t *pmod = (fov_mod_t *)mod;
  memset(pmod, 0x0, sizeof(fov_mod_t));
  pmod->ops.init = vfe_fov_init;
  pmod->ops.update = vfe_fov_update;
  pmod->ops.trigger_update = NULL;
  pmod->ops.config = vfe_fov_config;
  pmod->ops.enable = vfe_fov_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = NULL;
  pmod->ops.test_vector_validate = NULL;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_fov_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_fov_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_ops_deinit(void *mod)
{
  fov_mod_t *pmod = (fov_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
