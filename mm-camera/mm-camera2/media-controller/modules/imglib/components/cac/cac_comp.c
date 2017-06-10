/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include "cac_comp.h"

/**
 * CONSTANTS and MACROS
 **/

typedef struct {
  void *ptr;
  int (*cac_module)(cac_arg_t  *p_lib);
  int (*cac_module_init)();
  int (*cac_module_deinit)();
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
    IDBG_ERROR("%s:%d] p_comp->b.ops.init returned %d", __func__, __LINE__,status);
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
static void cac_comp_lib_debug(cac_arg_t *p_caclib)
{
  IDBG_MED("%s:%d] CACLIB pInY %p", __func__, __LINE__,
    p_caclib->pInY);
  IDBG_MED("%s:%d] CACLIB pInC %p", __func__, __LINE__,
    p_caclib->pInC);
  IDBG_MED("%s:%d] CACLIB fdInY %d", __func__, __LINE__,
    p_caclib->fdInY);
  IDBG_MED("%s:%d] CACLIB pTempLuma %p", __func__, __LINE__,
    p_caclib->pTmpY);
  IDBG_MED("%s:%d] CACLIB pTempChroma %p", __func__, __LINE__,
    p_caclib->pTmpC);
  IDBG_MED("%s:%d] CACLIB width %d", __func__, __LINE__,
    p_caclib->imageWidth);
  IDBG_MED("%s:%d] CACLIB height %d", __func__, __LINE__,
    p_caclib->imageHeight);
  IDBG_MED("%s:%d] CACLIB Y_stride %d", __func__, __LINE__,
    p_caclib->Y_stride);
  IDBG_MED("%s:%d] CACLIB C_stride %d", __func__, __LINE__,
    p_caclib->C_stride);
  IDBG_MED("%s:%d] CACLIB numThread %d", __func__,
    __LINE__, p_caclib->numThread);
  IDBG_MED("%s:%d] CACLIB chromaorder %d", __func__, __LINE__,
    p_caclib->chromaOrder);
  IDBG_MED("%s:%d] CACLIB awbGR %d", __func__, __LINE__,
    p_caclib->awbGR);
  IDBG_MED("%s:%d] CACLIB awbGB %d", __func__, __LINE__,
    p_caclib->awbGB);
  IDBG_MED("%s:%d] CACLIB edgeTH %d", __func__, __LINE__,
    p_caclib->edgeTH);
  IDBG_MED("%s:%d] CACLIB saturatedTH %d", __func__, __LINE__,
    p_caclib->saturatedTH);
  IDBG_MED("%s:%d] CACLIB chrom0LowTH %d", __func__, __LINE__,
    p_caclib->chrom0LowTH);
  IDBG_MED("%s:%d] CACLIB chrom0HighTH %d", __func__, __LINE__,
    p_caclib->chrom0HighTH);
  IDBG_MED("%s:%d] CACLIB chrom1LowTH %d", __func__, __LINE__,
    p_caclib->chrom1LowTH);
  IDBG_MED("%s:%d] CACLIB chrom1HighTH %d", __func__, __LINE__,
    p_caclib->chrom1HighTH);
  IDBG_MED("%s:%d] CACLIB chrom0LowDiffTH %d", __func__, __LINE__,
    p_caclib->chrom0LowDiffTH);
   IDBG_MED("%s:%d] CACLIB chorm0HighDiffTH %d", __func__, __LINE__,
    p_caclib->chorm0HighDiffTH);
  IDBG_MED("%s:%d] CACLIB chrom1LowDiffTH %d", __func__, __LINE__,
    p_caclib->chrom1LowDiffTH);
  IDBG_MED("%s:%d] CACLIB chorm1HighDiffTH %d", __func__, __LINE__,
    p_caclib->chorm1HighDiffTH);

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
  cac_arg_t cac_params;
  uint8_t *p_tempLuma = NULL, *p_tempCbcr = NULL;
  uint8_t *p_tempCR = NULL;

  memset(&cac_params, 0x0, sizeof(cac_arg_t));

  pthread_mutex_lock(&p_base->mutex);
  //Copy the data to a temp buffer to pass it to the CAC algoritm
  if (NULL == p_comp->p_y_buffer) {
    p_comp->p_y_buffer = malloc(p_frame->frame[0].plane[IY].stride *
      p_frame->info.height);
    if (!p_comp->p_y_buffer) {
      IDBG_ERROR("%s:%d] Error allocating temp Y buffer", __func__, __LINE__);
      pthread_mutex_unlock(&p_base->mutex);
      return IMG_ERR_NO_MEMORY;
    }
    IDBG_ERROR("%s:%d] Allocating temp buffers", __func__, __LINE__);
    memset(p_comp->p_y_buffer, 0x0, p_frame->frame[0].plane[IY].stride *
      p_frame->info.height);
  }
  if (NULL == p_comp->p_c_buffer) {
    p_comp->p_c_buffer = malloc(p_frame->frame[0].plane[IC].stride *
      (p_frame->info.height >> 1));
    if (!p_comp->p_c_buffer) {
      IDBG_ERROR("%s:%d] Error allocating temp C buffer", __func__, __LINE__);
      free(p_comp->p_y_buffer);
      p_comp->p_y_buffer = NULL;
      pthread_mutex_unlock(&p_base->mutex);
      return IMG_ERR_NO_MEMORY;
    }
    memset(p_comp->p_c_buffer, 0x0, p_frame->frame[0].plane[IC].stride *
      (p_frame->info.height >> 1));
  }
  p_tempLuma = p_comp->p_y_buffer;
  p_tempCbcr = p_comp->p_c_buffer;

  //Fill in the CAC algorithm data
  cac_params.pInY = p_frame->frame[0].plane[IY].addr;
  cac_params.fdInY = p_frame->frame[0].plane[IY].fd;
  cac_params.pInC = p_frame->frame[0].plane[IC].addr;
  cac_params.pTmpY = p_tempLuma;
  cac_params.pTmpC = p_tempCbcr;
  cac_params.imageWidth = p_frame->info.width;
  cac_params.imageHeight = p_frame->info.height;
  cac_params.Y_stride = p_frame->frame[0].plane[IY].stride;
  cac_params.C_stride = p_frame->frame[0].plane[IC].stride;
  cac_params.numThread = 4;
  cac_params.chromaOrder = p_comp->chroma_order;
  cac_params.awbGR = p_comp->info_3a.awb_gr_gain;
  cac_params.awbGB = p_comp->info_3a.awb_gb_gain;
  cac_params.RGammaTable_16 = p_comp->r_gamma.table;
  cac_params.GGammaTable_16 = p_comp->g_gamma.table;
  cac_params.BGammaTable_16 = p_comp->b_gamma.table;
  cac_params.edgeTH = p_comp->chromatix_info.edgeTH;
  cac_params.saturatedTH = p_comp->chromatix_info.saturatedTH;
  cac_params.chrom0LowTH = p_comp->chromatix_info.chrom0LowTH;
  cac_params.chrom0HighTH = p_comp->chromatix_info.chrom0HighTH;
  cac_params.chrom1LowTH = p_comp->chromatix_info.chrom1LowTH;
  cac_params.chrom1HighTH = p_comp->chromatix_info.chrom1HighTH;
  cac_params.chrom0LowDiffTH = p_comp->chromatix_info.chrom0LowDiffTH;
  cac_params.chorm0HighDiffTH = p_comp->chromatix_info.chorm1HighDiffTH;
  cac_params.chrom1LowDiffTH = p_comp->chromatix_info.chrom1LowDiffTH;
  cac_params.chorm1HighDiffTH = p_comp->chromatix_info.chorm1HighDiffTH;

  pthread_mutex_unlock(&p_base->mutex);

  //Print Params - To debug
  cac_comp_lib_debug(&cac_params);
  IDBG_ERROR("%s:%d] Start CAC ", __func__, __LINE__);

  rc = g_cac_lib.cac_module(&cac_params);
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
  case QCAC_RGAMMA_TABLE: {
    img_gamma_t *l_rgamma = (img_gamma_t *)p_data;
    if (NULL == l_rgamma) {
      IDBG_ERROR("%s:%d] invalid R-gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->r_gamma = *l_rgamma;
  }
  break;
  case QCAC_GGAMMA_TABLE: {
    img_gamma_t *l_ggamma = (img_gamma_t *)p_data;
    if (NULL == l_ggamma) {
      IDBG_ERROR("%s:%d] invalid G-gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->g_gamma = *l_ggamma;
  }
  break;
  case QCAC_BGAMMA_TABLE: {
    img_gamma_t *l_bgamma = (img_gamma_t *)p_data;
    if (NULL == l_bgamma) {
      IDBG_ERROR("%s:%d] invalid G-gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->b_gamma = *l_bgamma;
  }
  break;
  case QCAC_3A_INFO: {
    cac_3a_info_t *l_3aInfo = (cac_3a_info_t *)p_data;
    if (NULL == l_3aInfo) {
      IDBG_ERROR("%s:%d] invalid 3A Info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->info_3a = *l_3aInfo;
  }
  break;
  case QCAC_CHROMATIX_INFO : {
    cac_chromatix_info_t *l_chromatix = (cac_chromatix_info_t *)p_data;
    if (NULL == l_chromatix) {
      IDBG_ERROR("%s:%d] invalid chromatix info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->chromatix_info = *l_chromatix;
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
 *   @p_comp - The pointer to img_component_t object. This
 *            object contains the handle and the function
 *            pointers for communicating with the imaging
 *            component.
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

  g_cac_lib.ptr = dlopen("libmmcamera_cac_lib.so", RTLD_NOW);
  if (!g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] Error opening CAC library", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac_module) =
    dlsym(g_cac_lib.ptr, "cac_module");
  if (!g_cac_lib.cac_module) {
    IDBG_ERROR("%s:%d] Error linking camera CAC module start",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac_module_init) =
    dlsym(g_cac_lib.ptr, "cac_module_init");
  if (!g_cac_lib.cac_module_init) {
    IDBG_ERROR("%s:%d] Error linking cac_module_init",
    __func__, __LINE__);
  dlclose(g_cac_lib.ptr);
  g_cac_lib.ptr = NULL;
  return IMG_ERR_NOT_FOUND;
  }
  *(void **)&(g_cac_lib.cac_module_deinit) =
    dlsym(g_cac_lib.ptr, "cac_module_deinit");
  if (!g_cac_lib.cac_module_deinit) {
    IDBG_ERROR("%s:%d] Error linking cac_module_deinit",
    __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  rc = g_cac_lib.cac_module_init();
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
    rc = g_cac_lib.cac_module_deinit();
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib module deinit failed", __func__, __LINE__);
    }
    rc = dlclose(g_cac_lib.ptr);
    if (rc < 0)
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
      g_cac_lib.ptr = NULL;
  }
}
