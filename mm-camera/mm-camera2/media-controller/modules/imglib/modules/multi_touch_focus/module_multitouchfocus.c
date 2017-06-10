/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "module_imgbase.h"

/** MULTITOUCH_FOCUS_BURST_CNT:
 *
 *  default burst count
 **/
#define MULTI_TOUCH_FOCUS_BURST_CNT 5

/** :
 *
 *  indicate if refocus is enabled
 **/
static int g_c_mtf_refocus = 1;

/**
 *  Static functions
 **/
static boolean module_multitouch_focus_query_mod(mct_pipeline_cap_t *p_mct_cap);
static boolean module_multitouch_focus_init_params(img_init_params_t *p_params);

/** g_mtf_focus_steps:
 *
 *  Focus steps
 **/
uint8_t g_mtf_focus_steps[MAX_AF_BRACKETING_VALUES] =
  {21, 16, 11, 6, 1};

/** g_caps:
 *
 *  Set the capabilities for multi-touch focus module
 **/
static img_caps_t g_caps = {
  MULTI_TOUCH_FOCUS_BURST_CNT,/*num_input*/
  1,/*num_output*/
  1,/*num_meta*/
  0,/*inplace_algo*/
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  module_multitouch_focus_query_mod,
  module_multitouch_focus_init_params,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};


/**
 * Function: module_multitouch_focus_get_refocus
 *
 * Description: This function is used to check whether refocus
 *            is enabled
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_multitouch_focus_get_refocus()
{
  char prop[PROPERTY_VALUE_MAX];
  memset(prop, 0, sizeof(prop));
  property_get("persist.camera.imglib.refocus", prop, "0");
  int enable_refocus = atoi(prop);
  return enable_refocus == 2;
}


/**
 * Function: module_multitouch_focus_init_params
 *
 * Description: This function is used to init parameters
 *
 * Arguments:
 *   p_params - multi-touch focus init params
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_multitouch_focus_init_params(img_init_params_t *p_params)
{
  boolean ret = FALSE;
  if (p_params) {
    IDBG_HIGH("%s:%d] refocus %d", __func__, __LINE__,
      g_c_mtf_refocus);
    p_params->refocus_encode = g_c_mtf_refocus;
    ret = TRUE;
  }
  return ret;
}

/**
 * Function: module_multitouch_focus_deinit
 *
 * Description: This function is used to deinit multi-touch
 *               focus module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_multitouch_focus_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_multitouch_focus_query_mod
 *
 * Description: This function is used to query multi-touch focus
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
boolean module_multitouch_focus_query_mod(mct_pipeline_cap_t *p_mct_cap)
{
  mct_pipeline_imaging_cap_t *buf;

  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  buf = &p_mct_cap->imaging_cap;
  buf->mtf_af_bracketing.burst_count = MULTI_TOUCH_FOCUS_BURST_CNT;
  buf->mtf_af_bracketing.enable = TRUE;
  buf->mtf_af_bracketing.output_count =
    (g_c_mtf_refocus) ? MULTI_TOUCH_FOCUS_BURST_CNT + 1 : 1;
  memcpy(&buf->mtf_af_bracketing.focus_steps, &g_mtf_focus_steps,
    sizeof(g_mtf_focus_steps));
  return TRUE;
}

/** module_multitouch_focus_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the ubifocus
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_multitouch_focus_init(const char *name)
{
  //Get RAM size and disable features which are memory rich
  struct sysinfo info;
  sysinfo(&info);

  IDBG_MED("%s: totalram = %ld, freeram = %ld ", __func__, info.totalram,
    info.freeram);
  if (info.totalram > RAM_SIZE_THRESHOLD_FOR_AOST) {
    g_c_mtf_refocus = module_multitouch_focus_get_refocus();
    return module_imgbase_init(name,
      IMG_COMP_GEN_FRAME_PROC,
      "qcom.gen_frameproc",
      NULL,
      &g_caps,
      "libmmcamera_multitouchfocus_lib.so",
      CAM_QCOM_FEATURE_MULTI_TOUCH_FOCUS,
      &g_params);
  } else {
    return NULL;
  }
}

