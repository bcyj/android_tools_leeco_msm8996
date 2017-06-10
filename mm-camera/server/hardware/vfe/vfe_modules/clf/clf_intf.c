/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_clf_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_ops_init(void *mod)
{
  clf_mod_t *pmod = (clf_mod_t *)mod;
  memset(pmod, 0x0, sizeof(clf_mod_t));
  pmod->ops.init = vfe_clf_init;
  pmod->ops.update = vfe_clf_update;
  pmod->ops.trigger_update = vfe_clf_trigger_update;
  pmod->ops.config = vfe_clf_config;
  pmod->ops.enable = vfe_clf_enable;
  pmod->ops.reload_params = vfe_clf_reload_params;
  pmod->ops.trigger_enable = vfe_clf_trigger_enable;
  pmod->ops.test_vector_validate = vfe_clf_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = vfe_clf_set_bestshot;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_clf_deinit;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_clf_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_clf_ops_deinit(void *mod)
{
  clf_mod_t *pmod = (clf_mod_t *)mod;

  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
