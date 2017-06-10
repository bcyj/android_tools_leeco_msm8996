/*============================================================================
 Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/

#include "module_hdr_dbg.h"
#include "module_imglib_common.h"
#include "module_hdr_lib.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "hdr.h"
#include "hdr_chromatix.h"

/** MODULE_HDR_LIB_SINGLE_INSTANCE:
 *
 * Defines whether hdr library is single instance
 *
 * Returns TRUE if hdr library is single instance
 **/
#define MODULE_HDR_LIB_SINGLE_INSTANCE (TRUE)

/** hdr_lib_handle_t:
 *   @core_ops: core ops
 *   @mutex: used protecting single instance library
 *
 *  This structure defines hdr library handle
 **/

static hdr_chromatix_t g_hdr_chromatix = {
  #include "hdr_chromatix_data.h"
};

typedef struct
{
  img_core_ops_t core_ops;
  pthread_mutex_t mutex;
  hdr_chromatix_t *hdr_chromatix;
} hdr_lib_handle_t;

/** hdr_lib_t:
 *    @lib_handle: hdr library handle
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @user_data: user data
 *    @cb: notification cb
 *    @core_ops: function table for the operation
 *    @comp: pointer to the component ops
 *    @gamma: Regular light gamma table
 *
 *  This structure defines hdr library instance
 **/
typedef struct
{
  hdr_lib_handle_t* lib_handle;
  module_hdr_buf_t* out_buff[HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS];
  module_hdr_buf_t* in_buff[HDR_LIB_IN_BUFFS];
  void* user_data;
  module_hdr_lib_notify_cb cb;
  img_core_ops_t core_ops;
  img_component_ops_t comp;
  img_gamma_t gamma;
} hdr_lib_t;

#define HDR_MODULE_DEFAULT_GAMMA_TABLE_SIZE 1024

static const uint8_t hdr_module_default_gamma[HDR_MODULE_DEFAULT_GAMMA_TABLE_SIZE] =
  { 2, 2, 3, 4, 4, 5, 6, 6, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 17,
    18, 19, 20, 21, 21, 22, 23, 24, 25, 26, 26, 27, 28, 29, 30, 30, 31, 32, 33,
    34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 42, 43, 44, 44, 45, 46, 47, 48,
    49, 49, 50, 51, 51, 52, 53, 53, 54, 55, 55, 56, 56, 57, 58, 59, 59, 60, 61,
    61, 62, 63, 63, 64, 64, 65, 66, 66, 67, 67, 68, 69, 69, 70, 70, 71, 71, 72,
    73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 80, 80, 81, 81, 82, 82,
    83, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 87, 88, 88, 89, 89, 90, 90, 90,
    91, 91, 92, 92, 93, 93, 94, 94, 95, 95, 96, 96, 96, 97, 97, 98, 98, 99, 99,
    99, 100, 100, 100, 101, 101, 102, 102, 102, 103, 103, 104, 104, 105, 105,
    105, 106, 106, 107, 107, 107, 108, 108, 109, 109, 109, 110, 110, 110, 111,
    111, 111, 112, 112, 113, 113, 114, 114, 114, 115, 115, 115, 116, 116, 116,
    117, 117, 117, 118, 118, 118, 119, 119, 119, 120, 120, 121, 121, 121, 122,
    122, 123, 123, 123, 124, 124, 124, 124, 125, 125, 125, 126, 126, 126, 127,
    127, 127, 128, 128, 128, 129, 129, 129, 130, 130, 130, 131, 131, 132, 132,
    132, 132, 133, 133, 133, 134, 134, 134, 134, 135, 135, 135, 136, 136, 136,
    136, 137, 137, 137, 138, 138, 138, 139, 139, 139, 140, 140, 140, 141, 141,
    141, 142, 142, 142, 142, 143, 143, 143, 143, 144, 144, 144, 144, 145, 145,
    145, 145, 146, 146, 146, 146, 147, 147, 147, 148, 148, 148, 148, 149, 149,
    149, 150, 150, 150, 150, 151, 151, 151, 152, 152, 152, 152, 153, 153, 153,
    153, 154, 154, 154, 154, 155, 155, 155, 155, 155, 156, 156, 156, 156, 157,
    157, 157, 157, 157, 158, 158, 158, 158, 159, 159, 159, 159, 160, 160, 160,
    160, 161, 161, 161, 161, 162, 162, 162, 162, 163, 163, 163, 163, 163, 164,
    164, 164, 164, 165, 165, 165, 165, 166, 166, 166, 166, 167, 167, 167, 167,
    167, 168, 168, 168, 168, 168, 169, 169, 169, 169, 169, 170, 170, 170, 170,
    170, 171, 171, 171, 171, 171, 172, 172, 172, 172, 172, 173, 173, 173, 173,
    174, 174, 174, 174, 174, 174, 175, 175, 175, 175, 175, 176, 176, 176, 176,
    176, 177, 177, 177, 177, 177, 178, 178, 178, 178, 178, 178, 179, 179, 179,
    179, 179, 180, 180, 180, 180, 180, 180, 181, 181, 181, 181, 181, 182, 182,
    182, 182, 182, 183, 183, 183, 183, 183, 184, 184, 184, 184, 184, 185, 185,
    185, 185, 185, 185, 186, 186, 186, 186, 186, 187, 187, 187, 187, 187, 187,
    188, 188, 188, 188, 188, 188, 189, 189, 189, 189, 189, 189, 190, 190, 190,
    190, 190, 190, 191, 191, 191, 191, 191, 191, 191, 192, 192, 192, 192, 192,
    192, 193, 193, 193, 193, 193, 193, 194, 194, 194, 194, 194, 194, 195, 195,
    195, 195, 195, 195, 196, 196, 196, 196, 196, 196, 197, 197, 197, 197, 197,
    197, 198, 198, 198, 198, 198, 198, 199, 199, 199, 199, 199, 200, 200, 200,
    200, 200, 200, 201, 201, 201, 201, 201, 201, 202, 202, 202, 202, 202, 202,
    203, 203, 203, 203, 203, 203, 204, 204, 204, 204, 204, 204, 204, 205, 205,
    205, 205, 205, 205, 205, 205, 206, 206, 206, 206, 206, 206, 206, 206, 207,
    207, 207, 207, 207, 207, 207, 208, 208, 208, 208, 208, 208, 208, 209, 209,
    209, 209, 209, 209, 210, 210, 210, 210, 210, 210, 211, 211, 211, 211, 211,
    211, 211, 212, 212, 212, 212, 212, 212, 213, 213, 213, 213, 213, 213, 213,
    213, 214, 214, 214, 214, 214, 214, 214, 214, 215, 215, 215, 215, 215, 215,
    215, 216, 216, 216, 216, 216, 216, 216, 216, 217, 217, 217, 217, 217, 217,
    217, 218, 218, 218, 218, 218, 218, 218, 218, 219, 219, 219, 219, 219, 219,
    219, 219, 220, 220, 220, 220, 220, 220, 220, 220, 221, 221, 221, 221, 221,
    221, 221, 221, 222, 222, 222, 222, 222, 222, 222, 222, 223, 223, 223, 223,
    223, 223, 223, 223, 223, 224, 224, 224, 224, 224, 224, 224, 224, 225, 225,
    225, 225, 225, 225, 225, 225, 226, 226, 226, 226, 226, 226, 226, 226, 227,
    227, 227, 227, 227, 227, 227, 227, 228, 228, 228, 228, 228, 228, 228, 228,
    228, 229, 229, 229, 229, 229, 229, 229, 229, 229, 230, 230, 230, 230, 230,
    230, 230, 230, 230, 231, 231, 231, 231, 231, 231, 231, 231, 231, 232, 232,
    232, 232, 232, 232, 232, 232, 232, 232, 233, 233, 233, 233, 233, 233, 233,
    233, 233, 234, 234, 234, 234, 234, 234, 234, 234, 235, 235, 235, 235, 235,
    235, 235, 235, 235, 236, 236, 236, 236, 236, 236, 236, 236, 236, 237, 237,
    237, 237, 237, 237, 237, 237, 237, 238, 238, 238, 238, 238, 238, 238, 238,
    238, 239, 239, 239, 239, 239, 239, 239, 239, 239, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241,
    242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 243, 243, 243, 243, 243,
    243, 243, 243, 243, 243, 243, 244, 244, 244, 244, 244, 244, 244, 244, 244,
    245, 245, 245, 245, 245, 245, 245, 245, 245, 245, 246, 246, 246, 246, 246,
    246, 246, 246, 246, 247, 247, 247, 247, 247, 247, 247, 247, 247, 248, 248,
    248, 248, 248, 248, 248, 248, 248, 249, 249, 249, 249, 249, 249, 249, 249,
    249, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 251, 251, 251,
    251, 251, 251, 251, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252, 252,
    252, 252, 252, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 254,
    254, 254, 254, 254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255 };

#ifdef HDR_LIB_GHOSTBUSTER

/** MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX:
 *
 * Defines hdr library inplace output buffer index
 *
 * Returns hdr library inplace output buffer index
 **/
#define MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX (2)
static const int hdr_mod_frame_exposure_vals[HDR_LIB_IN_BUFFS] = { 0, -6, 6 };
static const int hdr_mod_frame_exposure_sequence[HDR_LIB_IN_BUFFS] = { 2, 1, 0 };

#else

/** MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX:
 *
 * Defines hdr library inplace output buffer index
 *
 * Returns hdr library inplace output buffer index
 **/
#define MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX (1)
static const int hdr_mod_frame_exposure_vals[HDR_LIB_IN_BUFFS] = { -6, 6 };
static const int hdr_mod_frame_exposure_sequence[HDR_LIB_IN_BUFFS] = { 1, 0 };

#endif

/** module_hdr_lib_event_handler
 *    @appdata: hdr library instance
 *    @event: pointer to the event
 *
 * Event handler for hdr library
 *
 * Returns IMG_SUCCESS in case of success or IMG_ERR_GENERAL
 **/
static int module_hdr_lib_event_handler(void* appdata, img_event_t *event)
{
  int ret_val = IMG_ERR_GENERAL;
  hdr_lib_t *hdr_lib = (hdr_lib_t *)appdata;
  img_frame_t *p_frame = NULL;
  hdr_crop_t out_crop;
  module_hdr_crop_t module_hdr_out_crop;
  int rc;

  IDBG_MED("%s +", __func__);

  if (event && hdr_lib && hdr_lib->cb) {
    IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, event->type);

    switch (event->type) {
    case QIMG_EVT_BUF_DONE: {
      IMG_COMP_DQ_BUF(&hdr_lib->comp, &p_frame);
      break;
    }

    case QIMG_EVT_ERROR:
    case QIMG_EVT_DONE: {
      rc = IMG_COMP_GET_PARAM(&hdr_lib->comp, QHDR_OUT_CROP, (void *)&out_crop);
      if (IMG_SUCCESS != rc) {
        IDBG_ERROR("Cannot get QHDR_OUT_CROP in %s:%d \n", __func__, __LINE__);
        break;
      }

      module_hdr_out_crop.start_x = out_crop.start_x;
      module_hdr_out_crop.start_y = out_crop.start_y;
      module_hdr_out_crop.width = out_crop.width;
      module_hdr_out_crop.height = out_crop.height;

      hdr_lib->cb(hdr_lib->user_data, hdr_lib->out_buff, hdr_lib->in_buff,
        &module_hdr_out_crop);

      if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
        if (pthread_mutex_unlock(&hdr_lib->lib_handle->mutex))
          IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
      }
    }
    default:
      break;
    }

    ret_val = IMG_SUCCESS;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_query_mod
 *    @buf: query capabilities data
 *
 * Fills capability and requirements for enabling this module
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_query_mod(mct_pipeline_imaging_cap_t *buf)
{
  boolean ret_val = FALSE;
  int i;

  IDBG_MED("%s +", __func__);

  if (NULL != buf) {
    //specify number of frames needed for HDR to work
    buf->hdr_bracketing_setting.num_frames = HDR_LIB_IN_BUFFS;

    //specify exposure bracketing to turn on
    buf->hdr_bracketing_setting.exp_val.mode =
      CAM_EXP_BRACKETING_ON;

    //specify exposure values for each of the frames
    for (i = 0; i < HDR_LIB_IN_BUFFS; i++)
      buf->hdr_bracketing_setting.exp_val.values[i] =
        hdr_mod_frame_exposure_vals[i];

    /* set HDR capability bit in imaging feature mask */
    buf->feature_mask |= CAM_QCOM_FEATURE_HDR;

    ret_val = TRUE;

  } else
    IDBG_ERROR("%s: Null pointer detected", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_set_hdr_lib_params:
 *    @lib_instance: library handle instance
 *    @session_meta: metadata buffer handler
 *    @cam_hdr_param: hdr library configuration
 *
 * Function to set hdr library input parameters
 *
 * Returns TRUE in case of success
 **/

static boolean module_hdr_lib_set_hdr_lib_params(void* lib_instance,
  mct_stream_session_metadata_info* session_meta,
  cam_hdr_param_t *cam_hdr_param)
{
  boolean ret_val = TRUE;
  int rc;
  unsigned int i;
  int sum;
  hdr_lib_t* hdr_lib = lib_instance;
  int16_t* isp_gamma_table;
  int out_index = 0;
  hdr_mode_t mode = MULTI_FRAME;

  IDBG_MED("%s +", __func__);

  if (NULL == hdr_lib) {
    IDBG_ERROR("%s:%d] Null pointer detected", __func__, __LINE__);
    ret_val = FALSE;
    goto end;
  }

  //set library mode
  rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_MODE,
      (void *)&mode)
  ;
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    ret_val = FALSE;
    goto end;
  }

  rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_HDR_CHROMATIX,
    (void *)hdr_lib->lib_handle->hdr_chromatix)
  ;
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    ret_val = FALSE;
    goto end;
  }

  /* Set gamma table :
   *
   * If session meta is NULL, pick default gamma table, else get
   * from session meta info
   */
  if (session_meta == NULL) {
    SUBSAMPLE_TABLE(hdr_module_default_gamma,
      HDR_MODULE_DEFAULT_GAMMA_TABLE_SIZE, hdr_lib->gamma.table,
      GAMMA_TABLE_ENTRIES, 0);

  } else {
    uint8_t* isp_gamma_data =
      (uint8_t*)session_meta->isp_gamma_data.private_data;
    memcpy(&hdr_lib->gamma.table, isp_gamma_data, sizeof(hdr_lib->gamma.table));
  }

  rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_GAMMA_TABLE,
      (void *)&hdr_lib->gamma)
  ;
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    ret_val = FALSE;
    goto end;
  }

  end:
  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_start_hdr_filter:
 *    @lib_instance: library handle instance
 *    @buff: buffer handler for input/output image
 *
 * Function to start inplace hdr filter on the image data
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_lib_start_hdr_filter(void* lib_instance)
{
  boolean ret_val = FALSE;
  hdr_lib_t* hdr_lib = lib_instance;
  int rc = IMG_ERR_INVALID_INPUT;
  int i;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->out_buff && hdr_lib->in_buff) {

    rc = IMG_SUCCESS;

    for (i = 0; i < HDR_LIB_OUT_BUFFS; i++) {
      rc = IMG_COMP_Q_BUF(&hdr_lib->comp, hdr_lib->out_buff[i]->img_frame,
          IMG_IN);
      if (IMG_SUCCESS != rc)
        break;
    }

    if (IMG_SUCCESS == rc) {
      for (i = 0; i < HDR_LIB_IN_BUFFS; i++) {
        rc = IMG_COMP_Q_BUF(&hdr_lib->comp, hdr_lib->in_buff[i]->img_frame,
            IMG_IN);
        if (IMG_SUCCESS != rc)
          break;
      }
    }

    //if buffers enqueued successfully start processing
    if (IMG_SUCCESS == rc) {
      IDBG_HIGH("Start hdr processing");

      rc = IMG_COMP_START(&hdr_lib->comp, NULL);

      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("Cannot start hdr in %s\n", __func__);
      }
    } else
      IDBG_ERROR("Cannot queue buffer in %s\n", __func__);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  ret_val = GET_STATUS(rc);
  if (!ret_val)
    IDBG_ERROR("Cannot apply hdr filter on the image data in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_get_output_inplace_index:
 *    @number: sequential number for inplace buffers
 *    @index: output inplace index
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_get_output_inplace_index(uint32_t number,
  uint32_t* index)
{
  boolean ret_val = FALSE;

  if (index) {
    if (number < IMGLIB_ARRAY_SIZE(hdr_mod_frame_exposure_sequence)) {

      *index = MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX;
      if (*index > 0) {
        ret_val = TRUE;
      }

    } else
      IDBG_ERROR("Input sequence number %d is more than array size %d %s\n",
        number, IMGLIB_ARRAY_SIZE(hdr_mod_frame_exposure_sequence)-1, __func__);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  return ret_val;
}

/** module_hdr_lib_process:
 *    @lib_instance: library handle instance
 *    @out_buff: output buffer handler array
 *    @in_buff: input buffer handler array
 *    @metadata_buff: metadata buffer handler
 *    @user_data: user data
 *    @cam_hdr_param: hdr library configuration
 *    @cb: notification cb
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_process(void* lib_instance, module_hdr_buf_t **out_buff,
  module_hdr_buf_t **in_buff, void* metadata_buff, void* user_data,
  cam_hdr_param_t *cam_hdr_param, module_hdr_lib_notify_cb cb)
{
  hdr_lib_t* hdr_lib = lib_instance;
  boolean ret_val = FALSE;
  int i;
  int rc;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->lib_handle && out_buff && in_buff && user_data
    && cb) {

    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_lock(&hdr_lib->lib_handle->mutex))
        IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
    }

    if (module_hdr_lib_set_hdr_lib_params(hdr_lib, metadata_buff,
      cam_hdr_param)) {
      hdr_lib->user_data = user_data;
      hdr_lib->cb = cb;

      for (i = 0; i < HDR_LIB_IN_BUFFS; i++) {
        hdr_lib->in_buff[i] = in_buff[hdr_mod_frame_exposure_sequence[i]];
      }

      for (i = 0; i < HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS; i++) {
        hdr_lib->out_buff[i] = out_buff[i];
      }

      if (module_hdr_lib_start_hdr_filter(hdr_lib))
        ret_val = TRUE;
    } else
      IDBG_ERROR(" %s: HDR library setparams failed\n", __func__);

    if (MODULE_HDR_LIB_SINGLE_INSTANCE && !ret_val) {
      if (pthread_mutex_unlock(&hdr_lib->lib_handle->mutex))
        IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }

  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_abort
 *    @lib_instance: library handle instance
 *
 * Aborts hdr library processing
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_abort(void* lib_instance)
{
  hdr_lib_t* hdr_lib = lib_instance;
  boolean ret_val = FALSE;
  int rc;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->lib_handle) {

    rc = IMG_COMP_ABORT(&hdr_lib->comp, NULL)
    ;

    ret_val = GET_STATUS(rc);
    if (!ret_val)
      IDBG_ERROR("Cannot abort hdr library in %s\n", __func__);

    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_unlock(&hdr_lib->lib_handle->mutex))
        IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_deinit
 *    @lib_instance: library handle instance
 *
 * Deinitializes hdr library
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_deinit(void* lib_instance)
{
  boolean ret_val = FALSE;
  int rc;
  hdr_lib_t* hdr_lib = lib_instance;
  hdr_lib_handle_t* lib_handle;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->lib_handle) {

    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_lock(&hdr_lib->lib_handle->mutex))
        IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
    }

    rc = IMG_COMP_DEINIT(&hdr_lib->comp)
    ;
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Cannot deinit hdr lib in %s\n", __func__);
    }

    lib_handle = hdr_lib->lib_handle;
    free(hdr_lib);

    ret_val = GET_STATUS(rc);
    if (!ret_val)
      IDBG_ERROR("Cannot deinitialize hdr library in %s\n", __func__);

    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_unlock(&lib_handle->mutex))
        IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_init
 *    @lib_handle: library handle
 *
 * Initializes hdr library
 *
 * Returns Library handle instance in case of success or NULL
 **/
void* module_hdr_lib_init(void* lib_handle)
{
  hdr_lib_t* hdr_lib = NULL;
  int rc;
  hdr_lib_handle_t* hdr_lib_handle = lib_handle;
  hdr_mode_t mode = MULTI_FRAME;

  IDBG_MED("%s +", __func__);

  if (lib_handle) {

    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_lock(&hdr_lib_handle->mutex))
        IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
    }

    hdr_lib = malloc(sizeof(hdr_lib_t));

    if (hdr_lib) {

      hdr_lib->lib_handle = lib_handle;
      hdr_lib->core_ops = hdr_lib_handle->core_ops;

      rc = IMG_COMP_CREATE(&hdr_lib->core_ops, &hdr_lib->comp);
      if (rc == IMG_SUCCESS) {
        rc = IMG_COMP_INIT(&hdr_lib->comp, hdr_lib, &mode);
        if (rc == IMG_SUCCESS) {
          rc = IMG_COMP_SET_CB(&hdr_lib->comp,
              module_hdr_lib_event_handler)
          ;

          if (rc != IMG_SUCCESS)
            IDBG_ERROR("Cannot set cb for hdr lib in %s\n", __func__);
        } else
          IDBG_ERROR("Cannot init hdr lib in %s\n", __func__);
      } else
        IDBG_ERROR("Cannot create hdr lib in %s\n", __func__);

      if (rc != IMG_SUCCESS) {
        free(hdr_lib);
        hdr_lib = NULL;
      }
    } else
      IDBG_ERROR("Cannot allocate memory for hdr library interface in %s\n",
        __func__);

    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_unlock(&hdr_lib_handle->mutex))
        IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return hdr_lib;
}

/** module_hdr_lib_unload
 *    @lib_handle: library handle
 *
 * Unloads hdr library
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_unload(void* lib_handle)
{
  boolean ret_val = FALSE;
  int rc;
  hdr_lib_handle_t* hdr_lib_handle = lib_handle;

  IDBG_MED("%s +", __func__);

  if (lib_handle) {
    if (MODULE_HDR_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_destroy(&hdr_lib_handle->mutex))
        IDBG_ERROR("Cannot destroy mutex\n");
    }

    rc = IMG_COMP_UNLOAD(&hdr_lib_handle->core_ops);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Cannot unload hdr lib in %s\n", __func__);
    }

    ret_val = GET_STATUS(rc);
    free(hdr_lib_handle);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_load
 *
 * Loads hdr library
 *
 * Returns library handle in case of success or NULL
 **/
void* module_hdr_lib_load()
{
  hdr_lib_handle_t* hdr_lib_handle;
  int rc = IMG_ERR_GENERAL;

  IDBG_MED("%s +", __func__);

  hdr_lib_handle = malloc(sizeof(hdr_lib_handle_t));

  if (hdr_lib_handle) {
    if (!MODULE_HDR_LIB_SINGLE_INSTANCE
      || !pthread_mutex_init(&hdr_lib_handle->mutex, NULL)) {
      rc = img_core_get_comp(IMG_COMP_HDR, "qcom.hdr",
        &hdr_lib_handle->core_ops);
      if (rc == IMG_SUCCESS) {
        rc = IMG_COMP_LOAD(&hdr_lib_handle->core_ops, NULL);
        if (rc != IMG_SUCCESS)
          IDBG_ERROR("Cannot load hdr lib in %s\n", __func__);
      } else
        IDBG_ERROR("Cannot get hdr lib component in %s\n", __func__);
      hdr_lib_handle->hdr_chromatix = &g_hdr_chromatix;

    } else
      IDBG_ERROR("Cannot create mutex\n");

    if (rc != IMG_SUCCESS) {
      free(hdr_lib_handle);
      hdr_lib_handle = NULL;
    }
  }

  IDBG_MED("%s -", __func__);

  return hdr_lib_handle;
}

