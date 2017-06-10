/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "module_imgbase.h"

/** TP_META_SIZE:
 *
 *  Tp max meta/body mask mask size
 **/
#define TP_FULL_META_SIZE (800 * 800)
#define BODY_MASK_META_SZ 25
#define TP_BODYMASK_WIDTH 800


/**
 *  Static functions
 **/
static boolean module_trueportrait_query_mod(mct_pipeline_cap_t *p_mct_cap);
static boolean module_trueportrait_init_params(img_init_params_t *p_params);


/** g_caps:
 *
 *  Set the capabilities for trueportrait module
 **/
static img_caps_t g_caps = {
  1,/*num_input*/
  0,/*num_output*/
  1,/*num_meta*/
  1,/*inplace_algo*/
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  module_trueportrait_query_mod,
  module_trueportrait_init_params,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

/**
 * Function: module_trueportrait_init_params
 *
 * Description: This function is used to init parameters
 *
 * Arguments:
 *   p_params - trueportrait init params
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_trueportrait_init_params(img_init_params_t *p_params)
{
  boolean ret = FALSE;
  if (p_params) {
    ret = TRUE;
  }
  return ret;
}

/**
 * Function: module_trueportrait_deinit
 *
 * Description: This function is used to deinit trueportrait
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
void module_trueportrait_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_trueportrait_query_mod
 *
 * Description: This function is used to query trueportrait
 *               caps
 *
 * Arguments:
 *   p_mct_cap - capababilities
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_trueportrait_query_mod(mct_pipeline_cap_t *p_mct_cap)
{
  mct_pipeline_imaging_cap_t *buf;


  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }
  buf = &p_mct_cap->imaging_cap;

  buf->true_portrait_settings.enable = TRUE;
  buf->true_portrait_settings.meta_max_size = TP_FULL_META_SIZE;
  buf->true_portrait_settings.meta_header_size = BODY_MASK_META_SZ;
  buf->true_portrait_settings.body_mask_width = TP_BODYMASK_WIDTH;

  return TRUE;
}

/** module_trueportrait_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the trueportrait
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_trueportrait_init(const char *name)
{
  //Get RAM size and disable features which are memory rich
  struct sysinfo info;
  sysinfo(&info);

  IDBG_MED("%s: totalram = %ld, freeram = %ld ", __func__, info.totalram,
    info.freeram);
  if (info.totalram > RAM_SIZE_THRESHOLD_FOR_AOST) {
    return module_imgbase_init(name,
      IMG_COMP_GEN_FRAME_PROC,
      "qcom.gen_frameproc",
      NULL,
      &g_caps,
      "libmmcamera_trueportrait_lib.so",
      CAM_QCOM_FEATURE_TRUEPORTRAIT,
      &g_params);
  } else {
    return NULL;
  }
}

