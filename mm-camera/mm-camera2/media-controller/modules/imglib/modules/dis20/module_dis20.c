/***************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.      *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#include "module_imgbase.h"

/** g_caps:
 *
 *  Set the capabilities for chroma flash module
*/
static img_caps_t g_caps = {
  1,/*num_input*/
  0,/*num_output*/
  1,/*num_meta*/
  1,/*inplace_algo*/
};

/**
 * Function: module_dis20_deinit
 *
 * Description: This function is used to deinit chroma flash
 *               module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_dis20_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/** module_dis20_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the chroma
 *            flash module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_dis20_init(const char *name)
{
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_dummyalgo.so",
    CAM_QCOM_FEATURE_DIS20, /* feature mask */
    NULL);
}


/** module_dis20_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 * of the dis 2.0 module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_dis20_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  return module_imgbase_set_parent(p_mct_mod, p_parent);

}
