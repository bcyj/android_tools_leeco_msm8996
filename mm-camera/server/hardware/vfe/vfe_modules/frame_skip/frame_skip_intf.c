/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_frame_skip_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_ops_init(void *mod)
{
  frame_skip_mod_t *pmod = (frame_skip_mod_t *)mod;
  memset(pmod, 0x0, sizeof(frame_skip_mod_t));
  pmod->ops.init = vfe_frame_skip_init;
  pmod->ops.update = vfe_frame_skip_config_pattern;
  pmod->ops.trigger_update = NULL;
  pmod->ops.config = vfe_frame_skip_config;
  pmod->ops.enable = vfe_frame_skip_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = NULL;
  pmod->ops.test_vector_validate = NULL;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_frame_skip_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_frame_skip_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_ops_deinit(void *mod)
{
  frame_skip_mod_t *pmod = (frame_skip_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}

