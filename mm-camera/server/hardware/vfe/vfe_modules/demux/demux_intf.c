/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_DEMUX_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_demux_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_ops_init(void *mod)
{
  demux_mod_t *pmod = (demux_mod_t *)mod;
  memset(pmod, 0x0, sizeof(demux_mod_t));
  pmod->ops.init = vfe_demux_init;
  pmod->ops.update = vfe_demux_update;
  pmod->ops.trigger_update = vfe_demux_trigger_update;
  pmod->ops.config = vfe_demux_config;
  pmod->ops.enable = vfe_demux_enable;
  pmod->ops.reload_params = vfe_demux_reload_params;
  pmod->ops.trigger_enable = vfe_demux_trigger_enable;
  pmod->ops.test_vector_validate = vfe_demux_tv_vaidate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_demux_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_demux_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_demux_ops_deinit(void *mod)
{
  demux_mod_t *pmod = (demux_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
