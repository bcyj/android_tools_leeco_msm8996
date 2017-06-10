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
  0,/*num_meta*/
  1,/*inplace_algo*/
};

/**
 * Function: module_llvd_client_created
 *
 * Description: function called after the client creation
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_created(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);
  p_client->rate_control = TRUE;
  p_client->exp_frame_delay = 33000LL;
  p_client->ion_fd = open("/dev/ion", O_RDONLY);
  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_client_destroy
 *
 * Description: function called before client is
 *                          destroyed
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);
  if (p_client->ion_fd > 0) {
    close(p_client->ion_fd);
    p_client->ion_fd = -1;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_client_process_done
 *
 * Description: function to indicate after the
 *                               frame is processed
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_frame: output frame
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_process_done(imgbase_client_t *p_client,
  img_frame_t *p_frame)
{
  void *v_addr = IMG_ADDR(p_frame);
  int32_t fd = IMG_FD(p_frame);
  int32_t buffer_size = IMG_FRAME_LEN(p_frame);
  IDBG_MED("%s:%d] addr %p fd %d size %d ion %d", __func__, __LINE__,
    v_addr, fd, buffer_size, p_client->ion_fd);
  int rc = img_cache_ops_external(v_addr, buffer_size, 0, fd,
    CACHE_CLEAN_INVALIDATE, p_client->ion_fd);
  return rc;
}

/**
 * Function: module_llvd_crop_support
 *
 * Description: function to indicate if
 *                               crop is supported
 *
 * Arguments:
 *
 * Return values:
 *   TRUE
 *   FALSE
 *
 * Notes: none
 **/
boolean module_llvd_crop_support()
{
  return FALSE;
}

/**
 * Function: module_llvd_query_mod
 *
 * Description: This function is used to query the
 *                   LLVD module info
 *
 * Arguments:
 *   @p_mct_cap - MCT capability struct
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_query_mod(mct_pipeline_cap_t *p_mct_cap)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);
  mct_pipeline_pp_cap_t *p_cap = &p_mct_cap->pp_cap;

  if (p_mct_cap->sensor_cap.sensor_format == FORMAT_YCBCR) {
    IDBG_ERROR("%s %d: Disable LLVD for YUV sensor", __func__, __LINE__);
    p_cap->feature_mask &= ~CAM_QCOM_FEATURE_LLVD;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_deinit
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
void module_llvd_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/** module_llvd_init:
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
mct_module_t *module_llvd_init(const char *name)
{
  module_imgbase_params_t params;
  memset(&params, 0x0, sizeof(module_imgbase_params_t));
  params.imgbase_client_created = module_llvd_client_created;
  params.imgbase_client_destroy = module_llvd_client_destroy;
  params.imgbase_client_process_done = module_llvd_client_process_done;
  params.imgbase_query_mod = module_llvd_query_mod;
  params.imgbase_crop_support = module_llvd_crop_support;

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_llvd.so",
    CAM_QCOM_FEATURE_LLVD, /* feature mask */
    &params);
}


/** module_llvd_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 * of the LLVD module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_llvd_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  return module_imgbase_set_parent(p_mct_mod, p_parent);

}

