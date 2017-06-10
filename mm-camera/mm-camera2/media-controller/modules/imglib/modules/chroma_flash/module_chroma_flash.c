/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#include "module_imgbase.h"

/** g_caps:
 *
 *  Set the capabilities for chroma flash module
*/
static img_caps_t g_caps = {
  2,/*num_input*/
  1,/*num_output*/
  1,/*num_meta*/
  0,/*inplace_algo*/
};

/**
 * Function: module_chroma_flash_deinit
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
void module_chroma_flash_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/** module_chroma_flash_init:
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
mct_module_t *module_chroma_flash_init(const char *name)
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
      "libmmcamera_chromaflash_lib.so",
      CAM_QCOM_FEATURE_CHROMA_FLASH,
      NULL);
  } else {
    return NULL;
  }
}

