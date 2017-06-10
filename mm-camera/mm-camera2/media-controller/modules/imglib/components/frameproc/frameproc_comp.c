/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "frameproc_comp.h"

/**
 * CONSTANTS and MACROS
 **/

 /**
 * Macro: FRAME_PROC_Q_MSG
 *
 * Description: Queues a message to the frameproc message queue
 *
 * Input parameters:
 *   @p_comp - The pointer to the component handle.
 *   @msg_type: message type
 *   @val_type: data type
 *   @data: data payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
#define FRAME_PROC_Q_MSG(p_comp, msg_type, val_type, data) ({ \
  int status = IMG_SUCCESS; \
  img_msg_t *p_msg; \
  p_msg = malloc(sizeof(img_msg_t)); \
  if (!p_msg) { \
    IDBG_ERROR("%s:%d] cannot alloc message", __func__, __LINE__); \
    status = IMG_ERR_NO_MEMORY; \
  } else { \
    memset(p_msg, 0x0, sizeof(img_msg_t)); \
    p_msg->type = msg_type; \
    p_msg->val_type = data; \
    status = img_q_enqueue(&p_comp->msgQ, p_msg); \
    if (IMG_ERROR(status)) { \
      IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__); \
      free(p_msg); \
    } else { \
      img_q_signal(&p_comp->msgQ); \
    } \
  } \
  status; \
})

/**
 * Function: frameproc_comp_init
 *
 * Description: Initializes the Qualcomm frameproc component
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
int frameproc_comp_init(void *handle, void* p_userdata, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;
  img_init_params_t *p_params = (img_init_params_t *)p_data;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return status;
  }

  if (p_comp->p_lib->img_algo_init)
    status = p_comp->p_lib->img_algo_init(&p_comp->p_algocontext, p_params);
  return status;
}

/**
 * Function: frameproc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to frameproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int frameproc_can_wait(void *p_userdata)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  return !((p_base->state == IMG_STATE_STOP_REQUESTED)
    || (p_base->state == IMG_STATE_STOPPED));
}

/**
 * Function: frameproc_flush_buffers
 *
 * Description: Function to send the buffers to client
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   p_bundle - frame bundle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void frameproc_flush_buffers(frameproc_comp_t *p_comp)
{
  img_component_t *p_base = &p_comp->b;
  int i = 0;
  img_frame_t *p_frame;
  img_meta_t *p_meta;

  while ((p_frame = img_q_dequeue(&p_base->inputQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_frame, p_frame);
  }

  while ((p_frame = img_q_dequeue(&p_base->outBufQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
      p_frame, p_frame);
  }

  while ((p_meta = img_q_dequeue(&p_base->metaQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_meta, p_meta);
  }
}

/**
 * Function: frameproc_send_buffers
 *
 * Description: Function to send the buffers to client
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   p_bundle - frame bundle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void frameproc_send_buffers(frameproc_comp_t *p_comp,
  img_frame_bundle_t *p_bundle)
{
  img_component_t *p_base = &p_comp->b;
  int i = 0;
  for (i = 0; i < p_base->caps.num_input; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_frame, p_bundle->p_input[i]);
  }
  for (i = 0; i < p_base->caps.num_output; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
      p_frame, p_bundle->p_output[i]);
  }
  for (i = 0; i < p_base->caps.num_meta; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_META_BUF_DONE,
      p_meta, p_bundle->p_meta[i]);
  }
}

/**
 * Function: frameproc_thread_loop
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
void *frameproc_thread_loop(void *handle)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  int i = 0, count;
  img_msg_t *p_msg;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  count = img_q_count(&p_comp->msgQ);
  IDBG_MED("%s:%d] buf count %d", __func__, __LINE__, count);

  while (TRUE) {
    p_msg = img_q_wait(&p_comp->msgQ, frameproc_can_wait, p_comp);

    if (!frameproc_can_wait(p_comp))
      break;
    else if (!p_msg)
      continue;

    switch (p_msg->type) {
    case IMG_MSG_BUNDLE:
      if (p_comp->p_lib->img_algo_process) {
        p_comp->p_lib->img_algo_process(p_comp->p_algocontext,
          p_msg->bundle.p_input,
          p_base->caps.num_input,
          (!p_base->caps.inplace_algo) ? p_msg->bundle.p_output :
            p_msg->bundle.p_input,
          (!p_base->caps.inplace_algo) ? p_base->caps.num_output :
            p_base->caps.num_input,
          p_msg->bundle.p_meta,
          p_base->caps.num_meta);
      }
      frameproc_send_buffers(p_comp, &p_msg->bundle);
      break;
    case IMG_MSG_FRAME:
      if (p_comp->p_lib->img_algo_frame_ind) {
        p_comp->p_lib->img_algo_frame_ind(p_comp->p_algocontext,
          p_msg->p_frame);
      }
      break;
    case IMG_MSG_META:
      if (p_comp->p_lib->img_algo_meta_ind)
        p_comp->p_lib->img_algo_meta_ind(p_comp->p_algocontext,
          p_msg->p_meta);
      break;
    default:;
    }
    if (p_msg)
      free(p_msg);
  }

  /* flush rest of the buffers */
  while ((p_msg = img_q_dequeue(&p_comp->msgQ)) != NULL) {
    switch (p_msg->type) {
    case IMG_MSG_BUNDLE:
      frameproc_send_buffers(p_comp, &p_msg->bundle);
      break;
    default:;
    }
    free(p_msg);
  }
  frameproc_flush_buffers(p_comp);
  return NULL;
}

/**
 * Function: frameproc_comp_get_param
 *
 * Description: Gets frameproc parameters
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
int frameproc_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);

  return status;
}

/**
 * Function: frameproc_comp_set_param
 *
 * Description: Set frameproc parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in frameproc.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int frameproc_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);

  IDBG_LOW("%s:%d] param 0x%x", __func__, __LINE__, param);
  return status;
}

/**
 * Function: frameproc_comp_deinit
 *
 * Description: Un-initializes the frameproc component
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
int frameproc_comp_deinit(void *handle)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.abort(handle, NULL);
  if (status < 0)
    return status;

  if (p_comp->p_lib->img_algo_deinit)
    status = p_comp->p_lib->img_algo_deinit(p_comp->p_algocontext);

  if (IMG_ERROR(status)) {
    IDBG_MED("%s:%d] Error status %d", __func__, __LINE__, status);
  }

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  img_q_deinit(&p_comp->msgQ);
  free(p_comp);
  return status;
}

/**
 * Function: frameproc_comp_check_create_bundle
 *
 * Description: This function is used to check and create the
 *            bundle if needed
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *
 * Return values:
 *     Imaging error values
 *
 * Notes: none
 **/
int frameproc_comp_check_create_bundle(frameproc_comp_t *p_comp)
{
  int8_t create_bundle = FALSE;
  int status = TRUE;
  img_component_t *p_base = &p_comp->b;
  int inputQcnt = img_q_count(&p_base->inputQ);
  int outputQcnt = img_q_count(&p_base->outBufQ);
  int metaQcount = img_q_count(&p_base->metaQ);
  img_msg_t *p_msg = NULL;
  int i = 0;

  if ((inputQcnt >= p_base->caps.num_input) &&
    (outputQcnt >= p_base->caps.num_output) &&
    (metaQcount >= p_base->caps.num_meta)) {
    p_msg = malloc(sizeof(img_msg_t));
    if (!p_msg) {
      IDBG_ERROR("%s:%d] Cannot create bundle", __func__, __LINE__);
      return IMG_ERR_NO_MEMORY;
    }
    memset(p_msg, 0x0, sizeof(img_msg_t));

    /* input */
    for (i = 0; i < p_base->caps.num_input; i++) {
      p_msg->bundle.p_input[i] = img_q_dequeue(&p_base->inputQ);
      if (!p_msg->bundle.p_input[i]) {
        IDBG_ERROR("%s:%d] Cannot dequeue in frame", __func__, __LINE__);
        goto error;
      }
    }
    /* output */
    for (i = 0; i < p_base->caps.num_output; i++) {
      p_msg->bundle.p_output[i] = img_q_dequeue(&p_base->outBufQ);
      if (!p_msg->bundle.p_output[i]) {
        IDBG_ERROR("%s:%d] Cannot dequeue out frame", __func__, __LINE__);
        goto error;
      }
    }
    /* meta */
    for (i = 0; i < p_base->caps.num_meta; i++) {
      p_msg->bundle.p_meta[i] = img_q_dequeue(&p_base->metaQ);
      if (!p_msg->bundle.p_meta[i]) {
        IDBG_ERROR("%s:%d] Cannot dequeue meta frame", __func__, __LINE__);
        goto error;
      }
    }

    status = img_q_enqueue(&p_comp->msgQ, p_msg);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__);
      return status;
    }
    create_bundle = TRUE;
  }
  IDBG_MED("%s:%d] (%d %d) (%d %d) (%d %d) flag %d",
    __func__, __LINE__,
    inputQcnt,
    p_base->caps.num_input,
    outputQcnt,
    p_base->caps.num_output,
    metaQcount,
    p_base->caps.num_meta,
    create_bundle);

  /* signal the component */
  if (create_bundle) {
    img_q_signal(&p_comp->msgQ);
  }
  return IMG_SUCCESS;

error:
  if (p_msg) {
    /* input */
    for (i = 0; i < p_base->caps.num_input; i++) {
      if (p_msg->bundle.p_input[i])
        free(p_msg->bundle.p_input[i]);
    }
    /* output */
    for (i = 0; i < p_base->caps.num_output; i++) {
      if (p_msg->bundle.p_output[i])
        free(p_msg->bundle.p_output[i]);
    }
    /* meta */
    for (i = 0; i < p_base->caps.num_meta; i++) {
      if (p_msg->bundle.p_meta[i])
        free(p_msg->bundle.p_meta[i]);
    }
    free(p_msg);
  }
  return IMG_ERR_GENERAL;
}

/**
 * Function: frameproc_comp_queue_buffer
 *
 * Description: This function is used to handle buffers from the
 *             client
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_frame - The frame buffer which needs to be processed by
 *             the imaging library
 *   @type: image type (main image or thumbnail image)
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int frameproc_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  img_queue_t *queue = (type == IMG_OUT) ? &p_base->outBufQ : &p_base->inputQ;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  if (type & IMG_IN) {
    FRAME_PROC_Q_MSG(p_comp, IMG_MSG_FRAME, p_frame, p_frame);
  }

  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  frameproc_comp_check_create_bundle(p_comp);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_queue_buffer
 *
 * Description: This function is used to handle input meta
 *            buffers from the client
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_metabuffer - The meta buffer which needs to be
 *             processed by the imaging library
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int frameproc_comp_queue_metabuffer(void *handle, img_meta_t *p_metabuffer)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  img_queue_t *queue = &p_base->metaQ;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_metabuffer);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  FRAME_PROC_Q_MSG(p_comp, IMG_MSG_META, p_meta, p_metabuffer);

  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  frameproc_comp_check_create_bundle(p_comp);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_abort
 *
 * Description: Aborts the execution of the base imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in the corresponding
 *            include file.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int frameproc_comp_abort(void *handle, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;

  IDBG_ERROR("%s:%d] state %d", __func__, __LINE__, p_base->state);
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOP_REQUESTED;
  pthread_mutex_unlock(&p_base->mutex);

  img_q_signal(&p_comp->msgQ);
  if (!pthread_equal(pthread_self(), p_base->threadid)) {
    IDBG_MED("%s:%d] thread id %d", __func__, __LINE__,
      (uint32_t)p_base->threadid);
    pthread_join(p_base->threadid, NULL);
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_start
 *
 * Description: Start the execution of frameproc
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int frameproc_comp_start(void *handle, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  if (p_data != NULL) {
     img_caps_t* update_caps = p_data;
     p_base->caps.num_input = update_caps->num_input;
     p_base->caps.num_output = update_caps->num_output;
     IDBG_HIGH("%s: updated # input = %d, # output = %d",
       __func__, p_base->caps.num_input, p_base->caps.num_output);
  }

  if ((p_base->caps.num_input <= 0) ||
    (p_base->caps.num_output < 0) ||
    (p_base->caps.num_meta < 0) ||
    (p_base->caps.num_input > IMG_MAX_INPUT_FRAME) ||
    (p_base->caps.num_output > IMG_MAX_OUTPUT_FRAME) ||
    (p_base->caps.num_meta > IMG_MAX_META_FRAME)) {
    IDBG_ERROR("%s:%d] Error caps not set", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (p_comp->p_lib->img_algo_set_frame_ops)
    p_comp->p_lib->img_algo_set_frame_ops(p_comp->p_algocontext,
      &p_base->frame_ops);

  /* flush the queues */
  img_q_flush(&p_base->inputQ);
  img_q_flush(&p_base->outBufQ);
  img_q_flush(&p_base->metaQ);
  img_q_flush(&p_comp->msgQ);

  pthread_mutex_unlock(&p_base->mutex);

  status = p_comp->b.ops.start(&p_comp->b, p_data);

  return status;
}

/**
 * Function: frameproc_comp_create
 *
 * Description: This function is used to create Qualcomm frameproc
 *              denoise component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component ops. This
 *            object contains the handle and the function
 *            pointers for communicating with the imaging
 *            component.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
int frameproc_comp_create(void* handle, img_component_ops_t *p_ops)
{
  frameproc_comp_t *p_comp = NULL;
  int status;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;

  if (!handle) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_frameproc_lib->ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (frameproc_comp_t *)malloc(sizeof(frameproc_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(frameproc_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = frameproc_thread_loop;
  p_comp->b.p_core = p_comp;
  p_comp->p_lib = p_frameproc_lib;
  img_q_init(&p_comp->msgQ, "msgQ");

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = frameproc_comp_init;
  p_ops->deinit          = frameproc_comp_deinit;
  p_ops->set_parm        = frameproc_comp_set_param;
  p_ops->get_parm        = frameproc_comp_get_param;
  p_ops->start           = frameproc_comp_start;
  p_ops->queue_buffer    = frameproc_comp_queue_buffer;
  p_ops->queue_metabuffer = frameproc_comp_queue_metabuffer;
  p_ops->abort           = frameproc_comp_abort;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_load
 *
 * Description: This function is used to load Qualcomm frameproc
 * library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int frameproc_comp_load(const char* name, void** handle)
{
  int rc = IMG_SUCCESS;
  frameproc_lib_info_t *p_frameproc_lib;

  if (!name || !handle) {
    IDBG_ERROR("%s:%d] invalid input %p %p",
      __func__, __LINE__, name, handle);
    return IMG_ERR_INVALID_INPUT;
  }

  p_frameproc_lib = malloc(sizeof(frameproc_lib_info_t));
  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] cannot alloc p_frameproc_lib %s",
      __func__, __LINE__, name);
    return IMG_ERR_NOT_FOUND;
  }

  p_frameproc_lib->ptr = dlopen(name, RTLD_NOW);
  if (!p_frameproc_lib->ptr) {
    IDBG_ERROR("%s:%d] Error opening frameproc library %s",
      __func__, __LINE__, name);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_frameproc_lib->img_algo_init) =
    dlsym(p_frameproc_lib->ptr, "img_algo_init");
  if (!p_frameproc_lib->img_algo_init) {
    IDBG_ERROR("%s:%d] Error linking camera img_algo_init",
      __func__, __LINE__);
    dlclose(p_frameproc_lib->ptr);
    p_frameproc_lib->ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_frameproc_lib->img_algo_deinit) =
    dlsym(p_frameproc_lib->ptr, "img_algo_deinit");
  if (!p_frameproc_lib->img_algo_deinit) {
    IDBG_ERROR("%s:%d] Error linking img_algo_deinit",
    __func__, __LINE__);
    dlclose(p_frameproc_lib->ptr);
    p_frameproc_lib->ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_frameproc_lib->img_algo_process) =
    dlsym(p_frameproc_lib->ptr, "img_algo_process");
  if (!p_frameproc_lib->img_algo_process) {
    IDBG_ERROR("%s:%d] Error linking img_algo_process",
    __func__, __LINE__);
    dlclose(p_frameproc_lib->ptr);
    p_frameproc_lib->ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_frameproc_lib->img_algo_frame_ind) =
    dlsym(p_frameproc_lib->ptr, "img_algo_frame_ind");
  if (!p_frameproc_lib->img_algo_frame_ind) {
    IDBG_ERROR("%s:%d] Warning linking img_algo_frame_ind",
    __func__, __LINE__);
  }

  *(void **)&(p_frameproc_lib->img_algo_meta_ind) =
    dlsym(p_frameproc_lib->ptr, "img_algo_meta_ind");
  if (!p_frameproc_lib->img_algo_meta_ind) {
    IDBG_ERROR("%s:%d] Warning linking img_algo_meta_ind",
    __func__, __LINE__);
  }

  *(void **)&(p_frameproc_lib->img_algo_set_frame_ops) =
    dlsym(p_frameproc_lib->ptr, "img_algo_set_frame_ops");
  if (!p_frameproc_lib->img_algo_set_frame_ops) {
    IDBG_ERROR("%s:%d] Warning linking img_algo_set_frame_ops",
    __func__, __LINE__);
  }

  *handle = p_frameproc_lib;
  IDBG_HIGH("%s:%d] %s loaded successfully", __func__, __LINE__, name);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_unload
 *
 * Description: This function is used to unload Qualcomm frameproc
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
void frameproc_comp_unload(void* handle)
{
  int rc = 0;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;
  IDBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error unloading library", __func__, __LINE__);
    return;
  }
  if (p_frameproc_lib->ptr) {
    rc = dlclose(p_frameproc_lib->ptr);
    if (rc < 0)
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
    p_frameproc_lib->ptr = NULL;
  }
  free(p_frameproc_lib);
}
