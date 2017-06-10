/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_rs_stats_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rs_stats_ops_init(void *mod)
{
  rs_stats_t *pmod = (rs_stats_t *)mod;
  memset(pmod, 0x0, sizeof(rs_stats_t));
  pmod->ops.init = NULL;
  pmod->ops.update = NULL;
  pmod->ops.trigger_update = NULL;
  pmod->ops.config = vfe_rs_stats_config;
  pmod->ops.enable = vfe_rs_stats_enable;
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
 * FUNCTION    - vfe_rs_stats_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rs_stats_ops_deinit(void *mod)
{
  rs_stats_t *pmod = (rs_stats_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_cs_stats_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_cs_stats_ops_init(void *mod)
{
  cs_stats_t *pmod = (cs_stats_t *)mod;
  memset(pmod, 0x0, sizeof(cs_stats_t));
  pmod->ops.init = NULL;
  pmod->ops.update = NULL;
  pmod->ops.trigger_update = NULL;
  pmod->ops.config = vfe_cs_stats_config;
  pmod->ops.enable = vfe_cs_stats_enable;
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
 * FUNCTION    - vfe_cs_stats_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_cs_stats_ops_deinit(void *mod)
{
  cs_stats_t *pmod = (cs_stats_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
