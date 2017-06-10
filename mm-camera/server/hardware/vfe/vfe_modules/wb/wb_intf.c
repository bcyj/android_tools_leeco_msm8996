/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_WB_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_wb_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_wb_ops_init(void *mod)
{
  wb_mod_t *pmod = (wb_mod_t *)mod;
  memset(pmod, 0x0, sizeof(wb_mod_t));
  pmod->ops.init = vfe_wb_init;
  pmod->ops.update = vfe_wb_update;
  pmod->ops.trigger_update = vfe_wb_trigger_update;
  pmod->ops.config = vfe_wb_config;
  pmod->ops.enable = vfe_wb_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = vfe_wb_trigger_enable;
  pmod->ops.test_vector_validate = vfe_wb_tv_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = vfe_wb_set_manual_wb;
  pmod->ops.set_bestshot = vfe_wb_set_bestshot;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_wb_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_wb_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_wb_ops_deinit(void *mod)
{
  wb_mod_t *pmod = (wb_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
