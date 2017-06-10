/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CX_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_color_xform_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_ops_init(void *mod_cx)
{
  CDBG("%s: E", __func__);
  color_xform_mod_t *pmod = (color_xform_mod_t *)mod;
  memset(pmod, 0 , sizeof(color_xform_mod_t));
  pmod->ops.init = vfe_color_xform_init;
  pmod->ops.update = vfe_color_xform_update;
  pmod->ops.trigger_update = vfe_color_xform_trigger_update;
  pmod->ops.config = vfe_color_xform_config;
  pmod->ops.enable = vfe_color_xform_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = vfe_color_xform_trigger_enable;
  pmod->ops.test_vector_validate = NULL;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = NULL;
  CDBG("%s: X", __func__);
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_color_xform_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_ops_deinit(void *mod_cx)
{
  color_xform_mod_t *pmod = (color_xform_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}

