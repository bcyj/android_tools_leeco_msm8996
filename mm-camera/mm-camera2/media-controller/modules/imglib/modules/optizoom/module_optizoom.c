/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "module_imgbase.h"

/** OPTIZOOM_ZOOM_THRESHOLD:
 *
 *  Zoom threshold. 2.0f represents 2x
 **/
#define OPTIZOOM_ZOOM_THRESHOLD (2.0f)

/** OPTIZOOM_BURST_CNT:
 *
 *  Burst count
 **/
#define OPTIZOOM_BURST_CNT      8

/**
 *  Static functions
 **/
static boolean module_optizoom_query_mod(mct_pipeline_cap_t *p_mct_cap);

/** g_caps:
 *
 *  Set the capabilities for optizoom module
*/
static img_caps_t g_caps = {
  OPTIZOOM_BURST_CNT,/*num_input*/
  1,/*num_output*/
  1,/*num_meta*/
  0,/*inplace_algo*/
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  module_optizoom_query_mod,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

/**
 * Function: module_optizoom_deinit
 *
 * Description: This function is used to deinit optizoom
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
void module_optizoom_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_optizoom_query_mod
 *
 * Description: This function is used to query optizoom
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
static boolean module_optizoom_query_mod(mct_pipeline_cap_t *p_mct_cap)
{
  mct_pipeline_imaging_cap_t *buf;

  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  buf = &p_mct_cap->imaging_cap;
  buf->opti_zoom_settings.zoom_threshold =
    module_imglib_common_get_zoom_level(p_mct_cap, OPTIZOOM_ZOOM_THRESHOLD);
  buf->opti_zoom_settings.burst_count = OPTIZOOM_BURST_CNT;
  buf->opti_zoom_settings.enable = TRUE;

  return TRUE;
}

/** module_optizoom_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the optizoom
 *             module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_optizoom_init(const char *name)
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
      "libmmcamera_optizoom_lib.so",
      CAM_QCOM_FEATURE_OPTIZOOM,
      &g_params);
  } else {
    return NULL;
  }
}

