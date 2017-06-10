/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CV_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_chroma_enhan_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_enhan_ops_init(void *mod)
{
  CDBG("%s: E", __func__);
  chroma_enhan_mod_t *pmod = (chroma_enhan_mod_t *)mod;
  memset(pmod, 0 , sizeof(chroma_enhan_mod_t));
  pmod->ops.init = vfe_chroma_enhan_init;
  pmod->ops.update = vfe_color_conversion_update;
  pmod->ops.trigger_update = vfe_color_conversion_trigger_update;
  pmod->ops.config = vfe_color_conversion_config;
  pmod->ops.enable = vfe_color_conversion_enable;
  pmod->ops.reload_params = vfe_color_conversion_reload_params;
  pmod->ops.trigger_enable = vfe_color_conversion_trigger_enable;
  pmod->ops.test_vector_validate = vfe_color_conversion_tv_validate;
  pmod->ops.set_effect = vfe_color_conversion_set_effect;
  pmod->ops.set_spl_effect = vfe_color_conversion_set_spl_effect;
  pmod->ops.set_manual_wb = vfe_color_conversion_set_manual_wb;
  pmod->ops.set_bestshot = vfe_color_conversion_set_bestshot;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_chroma_enhan_deinit;
  CDBG("%s: X", __func__);
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_chroma_enhan_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_enhan_ops_deinit(void *mod)
{
  chroma_enhan_mod_t *pmod = (chroma_enhan_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
