/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "module_denoise_dbg.h"
#include "module_imglib_common.h"
#include "module_denoise_lib.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "denoise.h"
#include "chromatix.h"

/** MODULE_DENOISE_LIB_SINGLE_INSTANCE:
 *
 * Defines whether denoise library is single instance
 *
 * Returns TRUE if denoise library is single instance
 **/
#define MODULE_DENOISE_LIB_SINGLE_INSTANCE (TRUE)

/** MODULE_DENOISE_LIB_CHROMATIX_VERSION:
 *
 * Defines supported version of chromatix
 *
 * Returns TRUE supported version of chromatix
 **/
#define MODULE_DENOISE_LIB_CHROMATIX_VERSION (0x301)

/** wd_lib_handle_t:
 *   @core_ops: core ops
 *   @mutex: used protecting single instance library
 *
 *  This structure defines denoise library handle
 **/
typedef struct {
  img_core_ops_t core_ops;
  pthread_mutex_t mutex;
} wd_lib_handle_t;

/** wd_lib_t:
 *    @lib_handle: denoise library handle
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @user_data: user data
 *    @cb: notification cb
 *    @info_3a: 3A information
 *    @core_ops: function table for the operation
 *    @comp: pointer to the component ops
 *    @mode: Wavelet denoise mode
 *    @chromatix: chromatics table
 *    @gamma: Regular light gamma table
 *    @low_gamma: Low light gamma table
 *
 *  This structure defines denoise library instance
 **/
typedef struct {
  wd_lib_handle_t* lib_handle;
  module_denoise_buf_t *out_buff;
  module_denoise_buf_t *in_buff;
  void* user_data;
  module_denoise_lib_notify_cb cb;
  wd_3a_info_t info_3a;
  img_core_ops_t core_ops;
  img_component_ops_t comp;
  wd_mode_t mode;
  chromatix_parms_type* chromatix;
  img_gamma_t gamma;
  img_gamma_t low_gamma;
} wd_lib_t;

/** module_denoise_lib_event_handler
 *    @appdata: denoise library instance
 *    @event: pointer to the event
 *
 * Event handler for denoise library
 *
 * Returns IMG_SUCCESS in case of success or IMG_ERR_GENERAL
 **/
static int module_denoise_lib_event_handler(void* appdata,
  img_event_t *event)
{
  int ret_val = IMG_ERR_GENERAL;
  wd_lib_t *wd_lib = (wd_lib_t *)appdata;

  IDBG_MED("%s +", __func__);

  if (event && wd_lib && wd_lib->out_buff && wd_lib->in_buff && wd_lib->cb) {
    IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, event->type);

    if (QIMG_EVT_DONE == event->type || QIMG_EVT_ERROR == event->type) {
      wd_lib->cb(wd_lib->user_data, wd_lib->out_buff, wd_lib->in_buff);
      if (pthread_mutex_unlock(&wd_lib->lib_handle->mutex))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }

    ret_val = IMG_SUCCESS;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_query_mod
 *    @buf: querry capabilities data
 *
 * Requests module capabilities data for specified session
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_query_mod(mct_pipeline_cap_t *buf)
{
  IDBG_MED("%s +", __func__);

  IDBG_MED("%s -", __func__);

  return TRUE;
}

/** module_denoise_lib_set_denoise_lib_params:
 *    @lib_instance: library handle instance
 *    @metadata_buff: metadata buffer handler
 *    @cam_denoise_param: denoise library configuration
 *
 * Function to set denoise library input parameters
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_lib_set_denoise_lib_params(void* lib_instance,
  mct_stream_session_metadata_info* session_meta,
  cam_denoise_param_t *cam_denoise_param)
{
  boolean ret_val = FALSE;
  int rc;
  unsigned int i;
  int sum;
  wd_lib_t* wd_lib = lib_instance;
  stats_get_data_t* stats_get_data;
  aec_get_t* aec_get;
  awb_update_t* awb_update;
  chromatix_parms_type* chromatix;
  chromatix_gamma_type *chromatix_gamma;
  int16_t* isp_gamma_data;
  wd_mode_t wd_mode;

  IDBG_MED("%s +", __func__);

  if (wd_lib && session_meta) {
    if (session_meta->sensor_data.chromatix_ptr
      && session_meta->sensor_data.common_chromatix_ptr) {

        chromatix = session_meta->sensor_data.chromatix_ptr;

        if (MODULE_DENOISE_LIB_CHROMATIX_VERSION
          == chromatix->chromatix_version) {

            chromatix_gamma = &(chromatix->chromatix_VFE.chromatix_gamma);
            stats_get_data =
              (stats_get_data_t*)&session_meta->stats_aec_data.private_data;
            isp_gamma_data = (int16_t*)&session_meta->isp_gamma_data.private_data;
            aec_get = &stats_get_data->aec_get;
            awb_update =
              (awb_update_t*)&session_meta->isp_stats_awb_data.private_data;

            wd_lib->info_3a.aec_real_gain = aec_get->real_gain[0];
            wd_lib->info_3a.lux_idx = aec_get->lux_idx;
            wd_lib->info_3a.wb_g_gain = awb_update->gain.g_gain;
            wd_lib->chromatix = session_meta->sensor_data.chromatix_ptr;

            memcpy(&wd_lib->gamma.table,
              isp_gamma_data,
              sizeof(wd_lib->gamma.table));
            for (i=0; i<IMGLIB_ARRAY_SIZE(wd_lib->low_gamma.table)-1; i++)
              wd_lib->low_gamma.table[i] =
                chromatix_gamma->lowlight_gamma_table.gamma[i] +
                ((chromatix_gamma->lowlight_gamma_table.gamma[i + 1] -
                chromatix_gamma->lowlight_gamma_table.gamma[i]) << 8);
            wd_lib->low_gamma.table[i] =
              chromatix_gamma->lowlight_gamma_table.gamma[i] + 0xff00;

            rc = IMG_COMP_SET_PARAM(&wd_lib->comp, QWD_3A_INFO,
              &wd_lib->info_3a);
            if (rc == IMG_SUCCESS) {
              rc = IMG_COMP_SET_PARAM(&wd_lib->comp, QWD_CHROMATIX,
                wd_lib->chromatix);
              if (rc == IMG_SUCCESS) {

                sum = 0;
                for (i=0; i<IMGLIB_ARRAY_SIZE(wd_lib->low_gamma.table); i++)
                  sum += wd_lib->low_gamma.table[i];
                if (sum)
                  rc = IMG_COMP_SET_PARAM(&wd_lib->comp, QWD_GAMMA_TABLE,
                    &wd_lib->gamma);

                if (rc == IMG_SUCCESS) {
                  sum = 0;
                  for (i=0; i<IMGLIB_ARRAY_SIZE(wd_lib->low_gamma.table); i++)
                    sum += wd_lib->low_gamma.table[i];
                  if (sum)
                    rc = IMG_COMP_SET_PARAM(&wd_lib->comp, QWD_LOW_GAMMA_TABLE,
                      &wd_lib->low_gamma);

                  if (rc == IMG_SUCCESS) {
                    switch (cam_denoise_param->process_plates) {
                    case CAM_WAVELET_DENOISE_CBCR_ONLY:
                      wd_mode = WD_MODE_CBCR_ONLY;
                      break;
                    case CAM_WAVELET_DENOISE_STREAMLINE_YCBCR:
                      wd_mode = WD_MODE_STREAMLINE_YCBCR;
                      break;
                    case CAM_WAVELET_DENOISE_STREAMLINED_CBCR:
                      wd_mode = WD_MODE_STREAMLINED_CBCR;
                      break;
                    case CAM_WAVELET_DENOISE_YCBCR_PLANE:
                    default:
                      wd_mode = WD_MODE_YCBCR_PLANE;
                    }

                    rc = IMG_COMP_SET_PARAM(&wd_lib->comp, QWD_MODE, &wd_mode);

                    if (rc != IMG_SUCCESS)
                      CDBG_ERROR("Cannot set QWD_MODE in %s\n", __func__);
                  } else
                    CDBG_ERROR("Cannot set QWD_LOW_GAMMA_TABLE in %s\n",
                      __func__);
                } else
                  CDBG_ERROR("Cannot set QWD_GAMMA_TABLE in %s\n", __func__);
              } else
                CDBG_ERROR("Cannot set QWD_CHROMATIX in %s\n", __func__);
            } else
              CDBG_ERROR("Cannot set QWD_3A_INFO in %s\n", __func__);

            ret_val = GET_STATUS(rc);
            if (!ret_val)
              CDBG_ERROR("Cannot set denoise library input parameters in %s\n",
                __func__);
        } else
          CDBG_ERROR("Wrong chromatix version 0x%x is not 0x%x in %s\n",
            chromatix->chromatix_version,
            MODULE_DENOISE_LIB_CHROMATIX_VERSION, __func__);
    } else
      CDBG_ERROR("Null chromatix pointer detected in %s\n", __func__);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_start_denoise_filter:
 *    @lib_instance: library handle instance
 *    @buff: buffer handler for input/output image
 *
 * Function to start inplace denoise filter on the image data
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_lib_start_denoise_filter(void* lib_instance,
  module_denoise_buf_t *buff)
{
  boolean ret_val = FALSE;
  wd_lib_t* wd_lib = lib_instance;
  int rc;

  IDBG_MED("%s +", __func__);

  if (wd_lib && buff) {
    rc = IMG_COMP_Q_BUF(&wd_lib->comp, buff->img_frame, IMG_IN);
    if (rc == IMG_SUCCESS) {

      IDBG_HIGH("Start denoise processing");

      rc = IMG_COMP_START(&wd_lib->comp, NULL);

      if (rc != IMG_SUCCESS) {
        CDBG_ERROR("Cannot start denoise in %s\n", __func__);
      }

    } else
      CDBG_ERROR("Cannot queue buffer in %s\n", __func__);

    ret_val = GET_STATUS(rc);
    if (!ret_val)
      CDBG_ERROR("Cannot apply denoise filter on the image data in %s\n",
        __func__);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_process:
 *    @lib_instance: library handle instance
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @metadata_buff: metadata buffer handler
 *    @user_data: user data
 *    @cam_denoise_param: denoise library configuration
 *    @cb: notification cb
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_process(void* lib_instance,
  module_denoise_buf_t *out_buff, module_denoise_buf_t *in_buff,
  void* metadata_buff, void* user_data, cam_denoise_param_t *cam_denoise_param,
  module_denoise_lib_notify_cb cb)
{
  wd_lib_t* wd_lib = lib_instance;
  boolean ret_val = FALSE;
  int rc;

  IDBG_MED("%s +", __func__);

  if (wd_lib && wd_lib->lib_handle && out_buff && out_buff->img_frame
    && in_buff && in_buff->img_frame && user_data && cb && metadata_buff) {

      if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
        if (pthread_mutex_lock(&wd_lib->lib_handle->mutex))
          CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
      }

      wd_lib->out_buff = out_buff;
      wd_lib->in_buff = in_buff;
      wd_lib->user_data = user_data;
      wd_lib->cb = cb;
      out_buff->img_frame->timestamp = in_buff->img_frame->timestamp;

      if (module_denoise_lib_set_denoise_lib_params(wd_lib, metadata_buff,
        cam_denoise_param)) {
          img_image_copy(out_buff->img_frame, in_buff->img_frame);

          if (module_denoise_lib_start_denoise_filter(wd_lib, out_buff))
            ret_val = TRUE;
      }

      if (MODULE_DENOISE_LIB_SINGLE_INSTANCE && !ret_val) {
        if (pthread_mutex_unlock(&wd_lib->lib_handle->mutex))
          CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
      }

  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_abort
 *    @lib_instance: library handle instance
 *
 * Aborts denoise library processing
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_abort(void* lib_instance)
{
  wd_lib_t* wd_lib = lib_instance;
  boolean ret_val = FALSE;
  int rc;

  IDBG_MED("%s +", __func__);

  if (wd_lib && wd_lib->lib_handle) {

    rc = IMG_COMP_ABORT(&wd_lib->comp, NULL);

    ret_val = GET_STATUS(rc);
    if (!ret_val)
      CDBG_ERROR("Cannot abort denoise library in %s\n",
        __func__);

    if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_unlock(&wd_lib->lib_handle->mutex))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_deinit
 *    @lib_instance: library handle instance
 *
 * Deinitializes denoise library
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_deinit(void* lib_instance)
{
  boolean ret_val = FALSE;
  int rc;
  wd_lib_t* wd_lib = lib_instance;
  wd_lib_handle_t* lib_handle;

  IDBG_MED("%s +", __func__);

  if (wd_lib && wd_lib->lib_handle) {

    if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_lock(&wd_lib->lib_handle->mutex))
        CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
    }

    rc = IMG_COMP_DEINIT(&wd_lib->comp);
    if (rc != IMG_SUCCESS) {
      CDBG_ERROR("Cannot deinit denoise lib in %s\n", __func__);
    }

    lib_handle = wd_lib->lib_handle;
    free(wd_lib);

    ret_val = GET_STATUS(rc);
    if (!ret_val)
      CDBG_ERROR("Cannot deinitialize denoise library in %s\n",
        __func__);

    if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_unlock(&lib_handle->mutex))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_init
 *    @lib_handle: library handle
 *
 * Initializes denoise library
 *
 * Returns Library handle instance in case of success or NULL
 **/
void* module_denoise_lib_init(void* lib_handle)
{
  wd_lib_t* wd_lib = NULL;
  int rc;
  wd_lib_handle_t* wd_lib_handle = lib_handle;

  IDBG_MED("%s +", __func__);

  if (lib_handle) {

    if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_lock(&wd_lib_handle->mutex))
        CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
    }

    wd_lib = malloc(sizeof(wd_lib_t));

    if (wd_lib) {

      wd_lib->lib_handle = lib_handle;
      wd_lib->mode = WD_MODE_YCBCR_PLANE;
      wd_lib->core_ops = wd_lib_handle->core_ops;

      rc = IMG_COMP_CREATE(&wd_lib->core_ops, &wd_lib->comp);
      if (rc == IMG_SUCCESS) {
        rc = IMG_COMP_INIT(&wd_lib->comp, wd_lib, &wd_lib->mode);
        if (rc == IMG_SUCCESS) {
          rc = IMG_COMP_SET_CB(&wd_lib->comp,
            module_denoise_lib_event_handler);

          if (rc != IMG_SUCCESS)
            CDBG_ERROR("Cannot set cb for denoise lib in %s\n", __func__);
        } else
          CDBG_ERROR("Cannot init denoise lib in %s\n", __func__);
      } else
        CDBG_ERROR("Cannot create denoise lib in %s\n", __func__);

      if (rc != IMG_SUCCESS) {
        free(wd_lib);
        wd_lib = NULL;
      }
    } else
      CDBG_ERROR("Cannot allocate memory for denoise library interface in %s\n",
        __func__);

    if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_unlock(&wd_lib_handle->mutex))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return wd_lib;
}

/** module_denoise_lib_unload
 *    @lib_handle: library handle
 *
 * Unloads denoise library
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_unload(void* lib_handle)
{
  boolean ret_val = FALSE;
  int rc;
  wd_lib_handle_t* wd_lib_handle = lib_handle;

  IDBG_MED("%s +", __func__);

  if (lib_handle) {
    if (MODULE_DENOISE_LIB_SINGLE_INSTANCE) {
      if (pthread_mutex_destroy(&wd_lib_handle->mutex))
        CDBG_ERROR("Cannot destroy mutex\n");
    }

    rc = IMG_COMP_UNLOAD(&wd_lib_handle->core_ops);
    if (rc != IMG_SUCCESS) {
      CDBG_ERROR("Cannot unload denoise lib in %s\n", __func__);
    }

    ret_val = GET_STATUS(rc);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_lib_load
 *
 * Loads denoise library
 *
 * Returns library handle in case of success or NULL
 **/
void* module_denoise_lib_load()
{
  wd_lib_handle_t* wd_lib_handle;
  int rc = IMG_ERR_GENERAL;

  IDBG_MED("%s +", __func__);

  wd_lib_handle = malloc(sizeof(wd_lib_handle_t));

  if (wd_lib_handle)
  {
    if (!MODULE_DENOISE_LIB_SINGLE_INSTANCE
      || !pthread_mutex_init(&wd_lib_handle->mutex, NULL)) {
        rc = img_core_get_comp(IMG_COMP_DENOISE, "qcom.wavelet",
          &wd_lib_handle->core_ops);
        if (rc == IMG_SUCCESS) {
          rc = IMG_COMP_LOAD(&wd_lib_handle->core_ops, NULL);
          if (rc != IMG_SUCCESS)
            CDBG_ERROR("Cannot load denoise lib in %s\n", __func__);
        } else
          CDBG_ERROR("Cannot get denoise lib component in %s\n", __func__);

      if (rc != IMG_SUCCESS && MODULE_DENOISE_LIB_SINGLE_INSTANCE
        && pthread_mutex_destroy(&wd_lib_handle->mutex))
          CDBG_ERROR("Cannot destroy mutex\n");

    } else
      CDBG_ERROR("Cannot create mutex\n");

    if (rc != IMG_SUCCESS) {
      free(wd_lib_handle);
      wd_lib_handle = NULL;
    }
  }

  IDBG_MED("%s -", __func__);

  return wd_lib_handle;
}
