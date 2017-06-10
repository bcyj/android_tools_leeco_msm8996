/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_CHROMA_SUPP_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_ops_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_ops_init(void *mod)
{
  chroma_supp_mod_t *pmod = (chroma_supp_mod_t *)mod;
  memset(pmod, 0 , sizeof(chroma_supp_mod_t));
  pmod->ops.init = vfe_chroma_suppression_init;
  pmod->ops.update = vfe_chroma_suppression_update;
  pmod->ops.trigger_update = vfe_chroma_suppresion_trigger_update;
  pmod->ops.config = vfe_chroma_suppression_config;
  pmod->ops.enable = vfe_chroma_suppression_enable;
  pmod->ops.reload_params = NULL;
  pmod->ops.trigger_enable = vfe_chroma_suppression_trigger_enable;
  pmod->ops.test_vector_validate =
      vfe_chroma_suppression_test_vector_validate;
  pmod->ops.set_effect = NULL;
  pmod->ops.set_spl_effect = NULL;
  pmod->ops.set_manual_wb = NULL;
  pmod->ops.set_bestshot = NULL;
  pmod->ops.set_contrast = NULL;
  pmod->ops.deinit = vfe_chroma_suppression_deinit;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_ops_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_suppression_ops_deinit(void *mod)
{
  chroma_supp_mod_t *pmod = (chroma_supp_mod_t *)mod;
  memset(&(pmod->ops), 0, sizeof(vfe_module_ops_t));
  return VFE_SUCCESS;
}
