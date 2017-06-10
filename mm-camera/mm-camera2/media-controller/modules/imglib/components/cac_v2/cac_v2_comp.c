/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>
#include "cac_v2_comp.h"

/**
 * CONSTANTS and MACROS
 **/


#define ION_HEAP_ID 0
typedef struct {
  void *ptr;
  int (*cac2_process)(cac2_args_t  *args);
  int (*cac2_init)();
  int (*cac2_deinit)();
} cac_lib_info_t;

static cac_lib_info_t g_cac_lib;

int cac_comp_abort(void *handle, void *p_data);

/**
 * Function: cac_comp_init
 *
 * Description: Initializes the Qualcomm CAC component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_userdata - the handle which is passed by the client
 *   p_data - The pointer to the parameter which is required during the
 *            init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int cac_comp_init(void *handle, void* p_userdata, void *p_data)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0)
  {
    IDBG_ERROR("%s:%d] p_comp->b.ops.init returned %d",
      __func__, __LINE__,status);
    return status;
  }
 return status;
}

/**
 * Function: cac_comp_lib_debug
 *
 * Description: Debug params for cac library
 *
 * Input parameters:
 *   p_caclib - library instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void cac_comp_lib_debug(cac2_args_t *p_caclib)
{
  int i =0;
  IDBG_MED("%s:%d] CACLIB pInY %p", __func__, __LINE__,
    p_caclib->p_y);
  IDBG_MED("%s:%d] CACLIB pInC %p", __func__, __LINE__,
    p_caclib->p_crcb);
  IDBG_MED("%s:%d] CACLIB fdInY %d", __func__, __LINE__,
    p_caclib->fd);
  IDBG_MED("%s:%d] CACLIB Ion Heap ID %d", __func__, __LINE__,
    p_caclib->ion_heap_id);
  IDBG_MED("%s:%d] CACLIB width %d", __func__, __LINE__,
    p_caclib->image_width);
  IDBG_MED("%s:%d] CACLIB height %d", __func__, __LINE__,
    p_caclib->image_height);
  IDBG_MED("%s:%d] CACLIB Y_stride %d", __func__, __LINE__,
    p_caclib->y_stride);
  IDBG_MED("%s:%d] CACLIB C_stride %d", __func__, __LINE__,
    p_caclib->cbcr_stride);
  IDBG_MED("%s:%d] CACLIB chromaorder %d", __func__, __LINE__,
    p_caclib->image_format);
  IDBG_MED("%s:%d] CACLIB Bright_Spot_HighTH %d", __func__, __LINE__,
    p_caclib->detection_th1);
  IDBG_MED("%s:%d] CACLIB Bright_Spot_LowTH %d", __func__, __LINE__,
    p_caclib->detection_th2);
  IDBG_MED("%s:%d] CACLIB Saturation_TH %d", __func__, __LINE__,
    p_caclib->detection_th3);
  IDBG_MED("%s:%d] CACLIB Color_Cb_TH %d", __func__, __LINE__,
    p_caclib->verification_th1);
  IDBG_MED("%s:%d] CACLIB CorrRatio_TH %d", __func__, __LINE__,
    p_caclib->correction_strength);
  IDBG_MED("%s:%d] CACLIB sampling_factor %d", __func__, __LINE__,
    p_caclib->sampling_factor);
   IDBG_MED("%s:%d] CACLIB lut_size %d", __func__, __LINE__,
    p_caclib->lut_size);
  IDBG_MED("%s:%d] CACLIB sigma_lut %p", __func__, __LINE__,
    p_caclib->sigma_lut);
  for(i =0; i < p_caclib->lut_size; i++){
    IDBG_MED("%s:%d] CACLIB sigma_lut[%d] = %f",  __func__, __LINE__,
      i, p_caclib->sigma_lut[i]);
  }
  IDBG_MED("%s:%d] CACLIB center_noise_sigma %f", __func__, __LINE__,
    p_caclib->center_noise_sigma);
  IDBG_MED("%s:%d] CACLIB center_noise_weight %f", __func__, __LINE__,
    p_caclib->center_noise_weight);
  IDBG_MED("%s:%d] CACLIB weight_order %f", __func__, __LINE__,
    p_caclib->weight_order);
  IDBG_MED("%s:%d] CACLIB scale_factor %f", __func__, __LINE__,
    p_caclib->scale_factor);
  IDBG_MED("%s:%d] CACLIB cac2_enabled %d", __func__, __LINE__,
    p_caclib->cac2_enable_flag);
  IDBG_MED("%s:%d] CACLIB rnr_enabled %d", __func__, __LINE__,
    p_caclib->rnr_enable_flag);
}

/**
 * Function: cac_comp_process_frame
 *
 * Description: Run the denoise algorithm on the given frame
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_frame - Frame which needs to be processed
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/

int cac_comp_process_frame(cac_comp_t *p_comp, img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;
  img_component_t *p_base = &p_comp->b;
  cac2_args_t cac_rnr_params;

  memset(&cac_rnr_params, 0x0, sizeof(cac2_args_t));

  pthread_mutex_lock(&p_base->mutex);

  //Fill in the CAC parameters
    cac_rnr_params.p_y = p_frame->frame[0].plane[IY].addr;
    cac_rnr_params.fd = p_frame->frame[0].plane[IY].fd;
    cac_rnr_params.p_crcb = p_frame->frame[0].plane[IC].addr;
    cac_rnr_params.image_width = p_frame->info.width;
    cac_rnr_params.image_height = p_frame->info.height;
    cac_rnr_params.y_stride = p_frame->frame[0].plane[IY].stride;
    cac_rnr_params.cbcr_stride = p_frame->frame[0].plane[IC].stride;
    cac_rnr_params.image_format = p_comp->chroma_order;
    cac_rnr_params.ion_heap_id = ION_HEAP_ID;
  //Fill in the CAC parameters
  if(p_comp->cac2_enable_flag ) {
    cac_rnr_params.detection_th1 =
      p_comp->cac_chromatix_info.detection_th1;
    cac_rnr_params.detection_th2 =
      p_comp->cac_chromatix_info.detection_th2;
    cac_rnr_params.detection_th3 = p_comp->cac_chromatix_info.detection_th3;
    cac_rnr_params.verification_th1 =
      p_comp->cac_chromatix_info.verification_th1;
    cac_rnr_params.correction_strength =
      p_comp->cac_chromatix_info.correction_strength;
  }
  cac_rnr_params.cac2_enable_flag = p_comp->cac2_enable_flag;

  //Fill in the RNR parameters
  if(p_comp->rnr_enable_flag) {
  cac_rnr_params.sampling_factor = p_comp->rnr_chromatix_info.sampling_factor;
  cac_rnr_params.sigma_lut = p_comp->rnr_chromatix_info.sigma_lut;
  cac_rnr_params.lut_size = p_comp->rnr_chromatix_info.lut_size;
  cac_rnr_params.scale_factor = p_comp->rnr_chromatix_info.scale_factor;
  cac_rnr_params.center_noise_sigma =
    p_comp->rnr_chromatix_info.center_noise_sigma;
  cac_rnr_params.center_noise_weight =
    p_comp->rnr_chromatix_info.center_noise_weight;
  cac_rnr_params.weight_order = p_comp->rnr_chromatix_info.weight_order;
  }
  cac_rnr_params.rnr_enable_flag = p_comp->rnr_enable_flag;;
  pthread_mutex_unlock(&p_base->mutex);

  //Print Params - To debug
  cac_comp_lib_debug(&cac_rnr_params);
  IDBG_ERROR("%s:%d] Start CAC ", __func__, __LINE__);

  ATRACE_BEGIN("Camera:CAC");
  rc = g_cac_lib.cac2_process(&cac_rnr_params);
  ATRACE_END();
  p_comp->b.state = IMG_STATE_IDLE;

  if (rc) {
    IDBG_ERROR("%s:%d] CAC failed", __func__, __LINE__);
  } else {
    IDBG_ERROR("%s:%d] CAC Successfull", __func__, __LINE__);
  }

  goto Error;

Error:
  return rc;
}


/**
 * Function: cac_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   data - The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *cac_thread_loop(void *handle)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;
  img_event_t event;
  int i = 0, count;
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] num buffers %d", __func__, __LINE__, count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      goto error;
    }
    /*process the frame*/
    status = cac_comp_process_frame(p_comp, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] process error %d", __func__, __LINE__, status);
      goto error;
    }

    /*enque the frame to the output queue*/
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      goto error;
    }
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT(p_base, QIMG_EVT_DONE);
  return IMG_SUCCESS;

error:
    /* flush rest of the buffers */
  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] Error buf count %d", __func__, __LINE__, count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      continue;
    }
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      continue;
    }
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  return NULL;

}


/**
 * Function: cac_comp_abort
 *
 * Description: Aborts the execution of CAC
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac_comp_abort(void *handle, void *p_data)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status;

  if (p_base->mode == IMG_ASYNC_MODE) {
    status = p_comp->b.ops.abort(&p_comp->b, p_data);
    if (status < 0)
      return status;
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);

  return 0;
}

/**
 * Function: cac_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              CAC component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   cmd - The command type which needs to be processed
 *   p_data - The pointer to the command payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int cac_comp_process (void *handle, img_cmd_type cmd, void *p_data)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: cac_comp_start
 *
 * Description: Start the execution of CAC
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in cac.h
 *   sync_mode: Indicates if the componnt is executed in
 *   syncronous mode.In this mode it will be executed in the
 *   calling threads context and there will be no callback.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int cac_comp_start(void *handle, void *p_data)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  p_base->state = IMG_STATE_STARTED;
  pthread_mutex_unlock(&p_base->mutex);

  if (p_base->mode == IMG_SYNC_MODE) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    status = cac_comp_process_frame(p_comp, p_frame);
  } else {
    status = p_comp->b.ops.start(&p_comp->b, p_data);
  }

  return status;
}


/**
 * Function: cac_comp_get_param
 *
 * Description: Gets CAC parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: cac_comp_set_param
 *
 * Description: Set CAC parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  IDBG_MED("%s:%d] param 0x%x", __func__, __LINE__, param);
  switch (param) {
  case QCAC_CHROMATIX_INFO : {
    cac_v2_chromatix_info_t *l_chromatix = (cac_v2_chromatix_info_t *)p_data;
    if (NULL == l_chromatix) {
      IDBG_ERROR("%s:%d] invalid chromatix info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->cac_chromatix_info = *l_chromatix;
  }
  break;
  case QCAC_CHROMA_ORDER: {
    cac_chroma_order *l_chroma_order = (cac_chroma_order *)p_data;
    if (NULL == l_chroma_order) {
      IDBG_ERROR("%s:%d] invalid chroma order info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->chroma_order = *l_chroma_order;
  }
  break;
  case QRNR_CHROMATIX_INFO: {
    rnr_chromatix_info_t *l_rnr_chromatix = (rnr_chromatix_info_t *)p_data;
    if (NULL == l_rnr_chromatix) {
      IDBG_ERROR("%s:%d] invalid RNR chromatix info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->rnr_chromatix_info = *l_rnr_chromatix;
  }
  break;
  case QCAC_ENABLED: {
    uint8_t *l_cac2_enabled = (uint8_t *)p_data;
    if (NULL == l_cac2_enabled) {
      IDBG_ERROR("%s:%d] invalid cac enabled info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->cac2_enable_flag = *l_cac2_enabled;
  }
  break;
  case QRNR_ENABLED: {
    uint8_t *l_rnr_enabled = (uint8_t *)p_data;
    if (NULL == l_rnr_enabled) {
      IDBG_ERROR("%s:%d] invalid rnr enabled info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->rnr_enable_flag = *l_rnr_enabled;
   }
   break;
  case QIMG_PARAM_MODE:
  break;
  default: {
    IDBG_ERROR("%s:%d] invalid parameter %d", __func__, __LINE__, param);
    return IMG_ERR_INVALID_INPUT;
  }
  }
  return status;
}

/**
 * Function: cac_comp_deinit
 *
 * Description: Un-initializes the CAC component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int cac_comp_deinit(void *handle)
{
  cac_comp_t *p_comp = (cac_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] \n", __func__, __LINE__);
  status = cac_comp_abort(handle, NULL);
  if (status < 0)
    return status;

  if (p_comp->p_y_buffer) {
    free(p_comp->p_y_buffer);
    p_comp->p_y_buffer = NULL;
  }
  if (p_comp->p_c_buffer) {
    free(p_comp->p_c_buffer);
    p_comp->p_c_buffer = NULL;
  }
  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: cac_comp_create
 *
 * Description: This function is used to create Qualcomm CAC
 *              denoise component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component_t object. This object
 *            contains the handle and the function pointers for
 *            communicating with the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac_comp_create(void* handle, img_component_ops_t *p_ops)
{
  cac_comp_t *p_comp = NULL;
  int status;

  if (NULL == g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (cac_comp_t *)malloc(sizeof(cac_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(cac_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0)
  {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = cac_thread_loop;
  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = cac_comp_init;
  p_ops->deinit          = cac_comp_deinit;
  p_ops->set_parm        = cac_comp_set_param;
  p_ops->get_parm        = cac_comp_get_param;
  p_ops->start           = cac_comp_start;
  p_ops->abort           = cac_comp_abort;
  p_ops->process         = cac_comp_process;


  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: cac_comp_load
 *
 * Description: This function is used to load Qualcomm CAC
 * library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
int cac_comp_load(const char* name, void** handle)
{
  int rc = IMG_SUCCESS;
  if (g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  g_cac_lib.ptr = dlopen("libmmcamera_cac2_lib.so", RTLD_NOW);
  if (!g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] Error opening CAC library", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac2_process) =
    dlsym(g_cac_lib.ptr, "cac2_process");
  if (!g_cac_lib.cac2_process) {
    IDBG_ERROR("%s:%d] Error linking camera CAC module start",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac2_init) =
    dlsym(g_cac_lib.ptr, "cac2_init");
  if (!g_cac_lib.cac2_init) {
    IDBG_ERROR("%s:%d] Error linking cac_module_init",
    __func__, __LINE__);
  dlclose(g_cac_lib.ptr);
  g_cac_lib.ptr = NULL;
  return IMG_ERR_NOT_FOUND;
  }
  *(void **)&(g_cac_lib.cac2_deinit) =
    dlsym(g_cac_lib.ptr, "cac2_deinit");
  if (!g_cac_lib.cac2_deinit) {
    IDBG_ERROR("%s:%d] Error linking cac_module_deinit",
    __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  rc = g_cac_lib.cac2_init(ION_HEAP_ID);
  if (rc) {
    IDBG_ERROR("%s:%d] CAC lib module init failed", __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  IDBG_HIGH("%s:%d] CAC library loaded successfully", __func__, __LINE__);

  return rc;
}

/**
 * Function: cac_comp_unload
 *
 * Description: This function is used to unload Qualcomm CAC
 * library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void cac_comp_unload(void* handle)
{
  int rc = 0;
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
    rc = g_cac_lib.cac2_deinit();
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib module deinit failed", __func__, __LINE__);
    }
    rc = dlclose(g_cac_lib.ptr);
    if (rc < 0)
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
      g_cac_lib.ptr = NULL;
  }
}
