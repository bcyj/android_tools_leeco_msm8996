/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "img_comp_priv.h"

#include <sys/syscall.h>
#include <sys/prctl.h>
#ifdef _ANDROID_
  #include <cutils/properties.h>
#endif

/**
 *  dynamic loglevel
 **/
volatile uint32_t g_imgloglevel;

/**
 * Function: img_comp_init
 *
 * Description: Initializes the base imaging component
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
int img_comp_init(void *handle, void* p_userdata, void *p_data)
{
  img_component_t *p_comp = (img_component_t *)handle;

  if (p_comp->state != IMG_STATE_IDLE) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }
  p_comp->threadid = -1;
  p_comp->p_userdata = p_userdata;
  pthread_mutex_init(&p_comp->mutex, NULL);
  pthread_cond_init(&p_comp->cond, NULL);
  img_q_init(&p_comp->inputQ, "inputQ");
  img_q_init(&p_comp->outputQ, "outputQ");
  img_q_init(&p_comp->outBufQ, "outBufQ");
  img_q_init(&p_comp->metaQ, "metaQ");
  p_comp->state = IMG_STATE_INIT;
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_deinit
 *
 * Description: Un-initializes the base imaging component
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
int img_comp_deinit(void *handle)
{
  img_component_t *p_comp = (img_component_t *)handle;

  if (p_comp->state == IMG_STATE_IDLE) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  img_q_deinit(&p_comp->inputQ);
  img_q_deinit(&p_comp->outputQ);
  img_q_deinit(&p_comp->outBufQ);
  img_q_deinit(&p_comp->metaQ);
  pthread_mutex_destroy(&p_comp->mutex);
  pthread_cond_destroy(&p_comp->cond);
  p_comp->state = IMG_STATE_IDLE;
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_set_param
 *
 * Description: Set common parameters for the base imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in the corresponding
 *            include file.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  img_component_t *p_comp = (img_component_t *)handle;

  IDBG_MED("%s:%d] param %d", __func__, __LINE__, param);
  pthread_mutex_lock(&p_comp->mutex);
  if (p_comp->state == IMG_STATE_IDLE) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  switch (param) {
  case QIMG_PARAM_FRAME_INFO: {
    img_frame_info_t *p_frame_info = (img_frame_info_t *)p_data;

    if (NULL == p_frame_info) {
      IDBG_ERROR("%s:%d] invalid frame data", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->frame_info = *p_frame_info;

    /* validate the parameters */
    if (!p_comp->frame_info.height || !p_comp->frame_info.width) {
      IDBG_ERROR("%s:%d] invalid frame size", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
  }
  break;
  case QIMG_PARAM_MODE: {
    img_comp_mode_t *l_mode = (img_comp_mode_t *) p_data;
    if (NULL == l_mode) {
      IDBG_ERROR("%s:%d] invalid mode", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->mode = *l_mode;
  }
  break;
  case QIMG_CAMERA_DUMP: {
    img_debug_info_t *l_debug_info = (img_debug_info_t*)p_data;
    if (NULL == l_debug_info) {
      IDBG_ERROR("%s:%d] invalid dump status", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->debug_info = *l_debug_info;
  }
  break;
  case QIMG_PARAM_CAPS: {
    img_caps_t *l_caps = (img_caps_t *) p_data;
    if (NULL == l_caps) {
      IDBG_ERROR("%s:%d] invalid caps", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->caps = *l_caps;
  }
  break;
  case QIMG_PARAM_FRAME_OPS: {
    img_frame_ops_t *l_ops = (img_frame_ops_t *) p_data;
    if (NULL == l_ops) {
      IDBG_ERROR("%s:%d] invalid frame ops", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->frame_ops = *l_ops;
  }
  break;
  default:;
  }
  pthread_mutex_unlock(&p_comp->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_get_param
 *
 * Description: Gets common parameters from the base imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in the corresponding
 *            include file.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  img_component_t *p_comp = (img_component_t *)handle;

  IDBG_MED("%s:%d] param %d", __func__, __LINE__, param);
  pthread_mutex_lock(&p_comp->mutex);
  if (p_comp->state == IMG_STATE_IDLE) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  switch (param) {
  case QIMG_PARAM_FRAME_INFO: {
    img_frame_info_t *p_frame_info = (img_frame_info_t *)p_data;

    /* validate the parameters */
    if (!p_comp->frame_info.height || !p_comp->frame_info.width) {
      IDBG_ERROR("%s:%d] invalid frame info", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_INPUT;
    }
    *p_frame_info = p_comp->frame_info;
  }
    break;
  case QIMG_PARAM_MODE: {
    img_comp_mode_t *l_mode = (img_comp_mode_t *) p_data;
    *l_mode = p_comp->mode;
  }
  break;
  default:;
  }
  pthread_mutex_unlock(&p_comp->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_thread_loop
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
void *img_thread_loop(void *data)
{
  img_component_t *p_comp = (img_component_t *)data;

  IDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "imgcomp_thread", 0, 0, 0);
  /*signal the base class*/
  pthread_mutex_lock(&p_comp->mutex);
  p_comp->is_ready = TRUE;
  pthread_cond_signal(&p_comp->cond);
  pthread_mutex_unlock(&p_comp->mutex);

  return p_comp->thread_loop(p_comp->p_core);
}

/**
 * Function: img_comp_start
 *
 * Description: Start the execution of the base imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in the corresponding
 *            include file.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int img_comp_start(void *handle, void *p_data)
{
  int status = IMG_SUCCESS;
  img_component_t *p_comp = (img_component_t *)handle;

  pthread_mutex_lock(&p_comp->mutex);
  if ((p_comp->state != IMG_STATE_INIT) ||
    (NULL == p_comp->thread_loop) ||
    (NULL == p_comp->p_core)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_comp->state);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  p_comp->is_ready = FALSE;
  status = pthread_create(&p_comp->threadid, NULL, img_thread_loop,
    (void *)p_comp);
  pthread_setname_np(p_comp->threadid, "CAM_img");
  if (status < 0) {
    IDBG_ERROR("%s:%d] pthread creation failed %d",
      __func__, __LINE__, status);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_GENERAL;
  }

  if (FALSE == p_comp->is_ready) {
    IDBG_MED("%s: before wait", __func__);
    pthread_cond_wait(&p_comp->cond, &p_comp->mutex);
  }
  IDBG_MED("%s: after wait", __func__);
  p_comp->state = IMG_STATE_STARTED;
  pthread_mutex_unlock(&p_comp->mutex);

  return IMG_SUCCESS;
}

/**
 * Function: img_comp_abort
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
int img_comp_abort(void *handle, void *p_data)
{
  img_component_t *p_comp = (img_component_t *)handle;

  IDBG_ERROR("%s:%d] state %d", __func__, __LINE__, p_comp->state);

  pthread_mutex_lock(&p_comp->mutex);
  /* Thread is not present in init state */
  if (IMG_STATE_INIT == p_comp->state) {
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_SUCCESS;
  }
  p_comp->state = IMG_STATE_STOP_REQUESTED;
  pthread_mutex_unlock(&p_comp->mutex);

  if (!pthread_equal(pthread_self(), p_comp->threadid)) {
    IDBG_MED("%s:%d] thread id %d", __func__, __LINE__,
      (uint32_t)p_comp->threadid);
    pthread_join(p_comp->threadid, NULL);
  }
  pthread_mutex_lock(&p_comp->mutex);
  p_comp->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_comp->mutex);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_set_callback
 *
 * Description: Sets the callback for the imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   notify - The function pointer for the event callback
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int img_comp_set_callback(void *handle, notify_cb notify)
{
  img_component_t *p_comp = (img_component_t *)handle;

  pthread_mutex_lock(&p_comp->mutex);
  if ((p_comp->state != IMG_STATE_INIT)
    && (p_comp->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  p_comp->p_cb = notify;
  pthread_mutex_unlock(&p_comp->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              imaging component
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
int img_comp_process(void *handle, img_cmd_type cmd, void *p_data)
{
  img_component_t *p_comp = (img_component_t *)handle;

  pthread_mutex_lock(&p_comp->mutex);
  if (p_comp->state == IMG_STATE_IDLE) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }
  pthread_mutex_unlock(&p_comp->mutex);
  /* This function needs to be implemented by the core component*/
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_queue_buffer
 *
 * Description: This function is used to send buffers to the component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_frame - The frame buffer which needs to be processed by the imaging
 *             library
 *   @type: image type (main image or thumbnail image)
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int img_comp_queue_buffer(void *handle, img_frame_t *p_frame, img_type_t type)
{
  img_component_t *p_comp = (img_component_t *)handle;
  int status = IMG_SUCCESS;
  img_queue_t *queue = (type == IMG_OUT) ?
    &p_comp->outBufQ : &p_comp->inputQ;

  pthread_mutex_lock(&p_comp->mutex);
  if ((p_comp->state != IMG_STATE_INIT)
    && (p_comp->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_comp->state);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return status;
  }
  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  img_q_signal(queue);

  pthread_mutex_unlock(&p_comp->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_deque_buffer
 *
 * Description: This function is used to get the buffers back from the
 *              imaging component.
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   pp_frame - The double pointer to hold the buffer from the component.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: This function can be called only after the client receives the
 *        buf done or error notification.
 **/
int img_comp_deque_buffer(void *handle, img_frame_t **pp_frame)
{
  img_component_t *p_comp = (img_component_t *)handle;
  int status = IMG_SUCCESS;

  pthread_mutex_lock(&p_comp->mutex);
  if ((p_comp->state == IMG_STATE_INIT) ||
    (p_comp->state == IMG_STATE_IDLE)) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  *pp_frame = (img_frame_t *)img_q_dequeue(&p_comp->outputQ);
  if (NULL == *pp_frame) {
    IDBG_ERROR("%s:%d] Error dequeue", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_NO_MEMORY;
  }

  pthread_mutex_unlock(&p_comp->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_comp_queue_metabuffer
 *
 * Description: This function is used to send buffers to the component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   @p_metabuffer - The meta buffer which needs to be
 *             processed by the imaging library
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int img_comp_queue_metabuffer(void *handle, img_meta_t *p_metabuffer)
{
  img_component_t *p_comp = (img_component_t *)handle;
  int status = IMG_SUCCESS;
  img_queue_t *queue = &p_comp->metaQ;

  pthread_mutex_lock(&p_comp->mutex);
  if ((p_comp->state != IMG_STATE_INIT)
    && (p_comp->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_comp->state);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_metabuffer);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return status;
  }
  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  img_q_signal(queue);

  pthread_mutex_unlock(&p_comp->mutex);
  return status;
}

/**
 * Function: img_comp_get_state
 *
 * Description: Get the state of the component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     component states
 *
 * Notes: none
 **/
comp_state_t img_comp_get_state(void *handle)
{
  img_component_t *p_comp = (img_component_t *)handle;
  return p_comp->state;
}

/**
 * Function: img_comp_create
 *
 * Description: This function is used to create the imaging component
 *
 * Input parameters:
 *   p_comp - The pointer to img_component_t object. This object contains
 *            the handle and the function pointers for communicating with
 *            the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int img_comp_create(img_component_t *p_comp)
{
  memset(p_comp, 0x0, sizeof(img_component_t));
  p_comp->ops.init            = img_comp_init;
  p_comp->ops.deinit          = img_comp_deinit;
  p_comp->ops.set_parm        = img_comp_set_param;
  p_comp->ops.get_parm        = img_comp_get_param;
  p_comp->ops.start           = img_comp_start;
  p_comp->ops.abort           = img_comp_abort;
  p_comp->ops.set_callback    = img_comp_set_callback;
  p_comp->ops.process         = img_comp_process;
  p_comp->ops.queue_buffer    = img_comp_queue_buffer;
  p_comp->ops.deque_buffer    = img_comp_deque_buffer;
  p_comp->ops.get_state       = img_comp_get_state;
  p_comp->ops.queue_metabuffer = img_comp_queue_metabuffer;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}
