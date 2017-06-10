

/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BG_STATS_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_bg_stats_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bg_stats_ops_init(void *mod)
{
  bg_stats_t *pmod = (bg_stats_t *)mod;
  memset(pmod, 0x0, sizeof(bg_stats_t));
  pmod->ops.init = vfe_bg_stats_init;
  pmod->ops.update = NULL;
  pmod->ops.trigger_update = NULL;
  pmod->ops.config = vfe_bg_stats_config;
  pmod->ops.enable = vfe_bg_stats_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = NULL;
  pmod->ops.test_vector_validate = NULL;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = NULL;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bg_stats_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bg_stats_ops_deinit(void *mod)
{
  bg_stats_t *pmod = (bg_stats_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
