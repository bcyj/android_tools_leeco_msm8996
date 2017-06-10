/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "camera_dbg.h"
#include "vfe_util_common.h"
#include "vfe.h"
#include "colorxform.h"
#include "vfe_tgtcommon.h"

#ifdef ENABLE_GAMMA_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_color_xform_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_init(int mod_id, void* mod_cx){

    vfe_status_t status = VFE_SUCCESS;
    CDBG("%s: enter", __func__);
    color_xform_mod_t *mod = (color_xform_mod_t *)mod_cx;

    mod->trigger_update = FALSE;
    mod->trigger_enable = TRUE;
    mod->update = FALSE;

    return status;
}

/*===========================================================================
 * FUNCTION    - vfe_color_xform_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_config(int mod_id, void* mod_cx, void *vparams){

    color_xform_mod_t *mod = (color_xform_mod_t *)mod_cx;
    vfe_params_t *parms = (vfe_params_t *) vparams;
    vfe_status_t status = VFE_SUCCESS;

    return status;
}

/*===========================================================================
 * FUNCTION    - vfe_color_xform_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_trigger_update(int mod_id, void* mod_cx,
  void* *vparams){
    color_xform_mod_t *mod = (color_xform_mod_t *)mod_cx;
    vfe_params_t *parms = (vfe_params_t *) vparams;
     vfe_status_t status = VFE_SUCCESS;



    return status;
}

/*===========================================================================
 * FUNCTION    - vfe_colorxform_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_update(int* mod_id, void* mod_cx, void* vparams){
    color_xform_mod_t *mod = (color_xform_mod_t *)mod_cx;
    vfe_params_t *parms = (vfe_params_t *) vparams;
    vfe_status_t status = VFE_SUCCESS;

    return status;
}/* vfe_colorXform_update */

/*===========================================================================
 * FUNCTION    - vfe_color_xform_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_enable(int mod_id, void* mod_cx, void* vparams,
  int8_t enable, int8_t hw_write){
    color_xform_mod_t *mod = (color_xform_mod_t *)mod_cx;
    vfe_params_t *parms = (vfe_params_t *) vparams;
    vfe_status_t status = VFE_SUCCESS;

    return status;
}/* vfe_color_xform_enable */

/*===========================================================================
 * FUNCTION    - vfe_color_xform_trigger_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_color_xform_trigger_enable(int mod_id, void* mod_cx,
  void* vparams, int enable){
    color_xform_mod_t *mod = (color_xform_mod_t *)mod_cx;
    vfe_params_t *parms = (vfe_params_t *) vparams;
    vfe_status_t status = VFE_SUCCESS;

    return status;
}/* vfe_color_xform_trigger_enable */
