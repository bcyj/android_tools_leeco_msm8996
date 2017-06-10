/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include "faceproc_comp.h"
#include <math.h>

/**
 * CONSTANTS and MACROS
 **/

#define FD_DOWN_SAMPLE
static faceproc_lib_t g_faceproc_lib;

int faceproc_comp_abort(void *handle, void *p_data);

/**
 * Function: faceproc_comp_init
 *
 * Description: Initializes the Qualcomm faceproc component
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
int faceproc_comp_init(void *handle, void* p_userdata, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0)
    return status;

  p_comp->mode = FACE_DETECT;
  p_comp->trans_info.h_scale = 1.0;
  p_comp->trans_info.v_scale = 1.0;
  p_comp->trans_info.h_offset = 0;
  p_comp->trans_info.v_offset = 0;
  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_deinit
 *
 * Description: Un-initializes the face processing component
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
int faceproc_comp_deinit(void *handle)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = faceproc_comp_abort(handle, NULL);
  if (status < 0)
    return status;

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  status = faceproc_comp_eng_destroy(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return status;
  }
  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_set_param
 *
 * Description: Set faceproc parameters
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
int faceproc_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  IDBG_MED("%s:%d] param 0x%x", __func__, __LINE__, param);
  switch (param) {
  case QWD_FACEPROC_CFG: {
    faceproc_config_t *p_config = (faceproc_config_t *)p_data;

    if (NULL == p_config) {
      IDBG_ERROR("%s:%d] invalid faceproc config", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->config = *p_config;
    p_comp->config_set = TRUE;

  }
    break;
  case QWD_FACEPROC_MODE: {
    faceproc_mode_t *p_mode = (faceproc_mode_t *)p_data;

    if (NULL == p_mode) {
      IDBG_ERROR("%s:%d] invalid faceproc mode", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->mode = *p_mode;
    IDBG_MED("%s:%d] mode %d", __func__, __LINE__, p_comp->mode);
  }
    break;
  case QWD_FACEPROC_CHROMATIX: {
    fd_chromatix_t *p_chromatix = (fd_chromatix_t *)p_data;

    if (NULL == p_chromatix) {
      IDBG_ERROR("%s:%d] invalid faceproc chromatix", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->fd_chromatix = *p_chromatix;
  }
    break;
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_get_param
 *
 * Description: Gets faceproc parameters
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
int faceproc_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  case QWD_FACEPROC_RESULT: {
    faceproc_result_t *p_result = (faceproc_result_t *)p_data;

    if (NULL == p_result) {
      IDBG_ERROR("%s:%d] invalid faceproc result", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    if (!p_comp->width || !p_comp->height) {
      IDBG_ERROR("%s:%d] frame not processed", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    status = faceproc_comp_eng_get_output(p_comp, p_result);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s:%d] invalid faceproc result", __func__, __LINE__);
      return status;
    }
    p_result->trans_info.h_scale = p_comp->trans_info.h_scale;
    p_result->trans_info.v_scale = p_comp->trans_info.v_scale;
    p_result->trans_info.h_offset = p_comp->trans_info.h_offset;
    p_result->trans_info.v_offset = p_comp->trans_info.v_offset;
    break;
  }
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_cfg_debug
 *
 * Description: Debug params for faceproc configuration
 *
 * Input parameters:
 *   p_denoiselib - library instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void faceproc_comp_cfg_debug(faceproc_config_t *p_config)
{
  IDBG_LOW("%s:%d] FaceProc cfg hist enable %d", __func__, __LINE__,
    p_config->histogram_enable);
  IDBG_LOW("%s:%d] FaceProc cfg max_height %d",
    __func__, __LINE__,
    p_config->frame_cfg.max_height);
  IDBG_LOW("%s:%d] FaceProc cfg max_width %d",
    __func__, __LINE__,
    p_config->frame_cfg.max_width);
  IDBG_LOW("%s:%d] FaceProc cfg face_orientation_hint %d",
    __func__, __LINE__,
    p_config->face_cfg.face_orientation_hint);
  IDBG_LOW("%s:%d] FaceProc cfg max_face_size %d",
    __func__, __LINE__,
    p_config->face_cfg.max_face_size);
  IDBG_LOW("%s:%d] FaceProc cfg max_num_face_to_detect %d",
    __func__, __LINE__,
    p_config->face_cfg.max_num_face_to_detect);
  IDBG_LOW("%s:%d] FaceProc cfg min_face_size %d",
    __func__, __LINE__,
    p_config->face_cfg.min_face_size);
  IDBG_LOW("%s:%d] FaceProc cfg rotation_range %d",
    __func__, __LINE__,
    p_config->face_cfg.rotation_range);
}

/**
 * Function: faceproc_comp_chromaitix_debug
 *
 * Description: Debug params for faceproc chromatix
 *
 * Input parameters:
 *   p_chromatix - FD chromatix pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void faceproc_comp_chromaitix_debug(fd_chromatix_t *p_chromatix)
{
  IDBG_MED("%s:%d] FaceProc angle_front %d", __func__, __LINE__,
    p_chromatix->angle_front);
  IDBG_MED("%s:%d] FaceProc angle_front_bitmask %x", __func__, __LINE__,
    p_chromatix->angle_front_bitmask);
  IDBG_MED("%s:%d] FaceProc angle_front %d", __func__, __LINE__,
    p_chromatix->angle_full_profile);
  IDBG_MED("%s:%d] FaceProc angle_full_profile %d", __func__, __LINE__,
    p_chromatix->angle_full_profile_bitmask);
  IDBG_MED("%s:%d] FaceProc angle_full_profile_bitmask %x", __func__, __LINE__,
    p_chromatix->angle_half_profile);
  IDBG_MED("%s:%d] FaceProc angle_half_profile %d", __func__, __LINE__,
    p_chromatix->angle_half_profile);
  IDBG_MED("%s:%d] FaceProc angle_front_bitmask %x", __func__, __LINE__,
    p_chromatix->angle_front_bitmask);
  IDBG_MED("%s:%d] FaceProc detection_mode %d", __func__, __LINE__,
    p_chromatix->detection_mode);
  IDBG_MED("%s:%d] FaceProc enable_blink_detection %d", __func__, __LINE__,
    p_chromatix->enable_blink_detection);
  IDBG_MED("%s:%d] FaceProc enable_gaze_detection %d", __func__, __LINE__,
    p_chromatix->enable_gaze_detection);
  IDBG_MED("%s:%d] FaceProc enable_smile_detection %d", __func__, __LINE__,
    p_chromatix->enable_smile_detection);
  IDBG_MED("%s:%d] FaceProc frame_skip %d", __func__, __LINE__,
    p_chromatix->frame_skip);
  IDBG_MED("%s:%d] FaceProc max_face_size %d", __func__, __LINE__,
    p_chromatix->max_face_size);
  IDBG_MED("%s:%d] FaceProc max_num_face_to_detect %d", __func__, __LINE__,
    p_chromatix->max_num_face_to_detect);
  IDBG_MED("%s:%d] FaceProc min_face_adj_type %d", __func__, __LINE__,
    p_chromatix->min_face_adj_type);
  IDBG_MED("%s:%d] FaceProc min_face_size %d", __func__, __LINE__,
    p_chromatix->min_face_size);
  IDBG_MED("%s:%d] FaceProc min_face_size_ratio %f", __func__, __LINE__,
    p_chromatix->min_face_size_ratio);
}

/**
 * Function: face_proc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to faceproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int face_proc_can_wait(void *p_userdata)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  return !((p_base->state == IMG_STATE_STOP_REQUESTED)
    || (p_base->state == IMG_STATE_STOPPED));
}

/**
 * Function: face_proc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to faceproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int face_proc_release_frame(void *data, void *p_userdata)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = (img_frame_t *)data;

  status = img_q_enqueue(&p_base->outputQ, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
  } else {
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }
  p_comp->facedrop_cnt++;
  return status;
}

/**
 * Function: face_proc_get_scaled_frame
 *
 * Description: Function to get the downscaled frame
 *
 * Input parameters:
 *   p_comp - The pointer to faceproc component
 *   p_frame - poiinter to the frame
 *   p_ds_frame - pointer to the downscaled frame
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int face_proc_get_scaled_frame(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  img_frame_t *p_ds_frame)
{
  uint8_t *p_scaledframe = NULL;
  int size = FD_WIDTH(p_frame) * FD_HEIGHT(p_frame);
  p_scaledframe = (uint8_t *)malloc(size/2);
  if (NULL == p_scaledframe) {
    IDBG_ERROR("%s:%d] cannot allocate scaled img buf", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  p_ds_frame->frame[0].plane[0].addr = p_scaledframe;
  p_ds_frame->frame[0].plane[0].width = FD_WIDTH(p_frame)/2;
  p_ds_frame->frame[0].plane[0].height = FD_HEIGHT(p_frame)/2;
#ifdef FD_DOWNSCALE_ASSEMBLY
  ds_2by2_asm(FD_ADDR(p_frame), FD_WIDTH(p_frame), FD_HEIGHT(p_frame),
    FD_WIDTH(p_frame), FD_ADDR(p_ds_frame), FD_WIDTH(p_ds_frame));
#endif
  p_comp->trans_info.h_offset = (int)p_comp->width/4;
  p_comp->trans_info.v_offset = (int)p_comp->height/4;
  return IMG_SUCCESS;
}


/**
 * Function: face_proc_thread_loop
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
void *face_proc_thread_loop(void *handle)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;
  img_event_t event;
  int i = 0, count;
  img_frame_t ds_frame;
  img_frame_t *p_ds_frame = &ds_frame;
  IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
    p_base->state, p_comp->abort_flag);

#ifdef FD_DROP_FRAME
  while (1) {

    /* wait for frame */
    img_q_wait_for_signal(&p_base->inputQ, face_proc_can_wait, p_comp);

    if (!face_proc_can_wait(p_comp)) {
      IDBG_HIGH("%s:%d] Exit the thread", __func__, __LINE__);
      break;
    }
    p_comp->facedrop_cnt = 0;
    p_frame = (img_frame_t *)img_q_get_last_entry(&p_base->inputQ,
      face_proc_release_frame, p_comp);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] No more elements.", __func__, __LINE__);
      continue;
    }
    IDBG_MED("%s:%d] frame drop cnt %d q_cnt %d buf_idx %d",
      __func__, __LINE__,
      p_comp->facedrop_cnt, img_q_count(&p_base->inputQ), p_frame->idx);
#else
  while ((p_frame = img_q_wait(&p_base->inputQ,
    face_proc_can_wait, p_comp)) != NULL) {
#endif

    p_comp->width = FD_WIDTH(p_frame);
    p_comp->height = FD_HEIGHT(p_frame);

    if (!FD_DS_ENABLE(p_comp)) {
      status = faceproc_comp_eng_exec(p_comp, p_frame);
    } else {
      status = face_proc_get_scaled_frame(p_comp, p_frame, p_ds_frame);
      if (IMG_SUCCEEDED(status)) {
        status = faceproc_comp_eng_exec(p_comp, p_ds_frame);
        free(FD_ADDR(p_ds_frame));
      }
    }

    if (status != 0) {
      IDBG_ERROR("%s:%d] frameproc exec error %d", __func__, __LINE__,
        status);
      status = img_q_enqueue(&p_base->outputQ, p_frame);
      if (status < 0) {
        IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      } else {
        IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
      }
      IMG_SEND_EVENT(p_base, QIMG_EVT_ERROR);
      continue;
    }
    IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
      p_base->state, p_comp->abort_flag);

    IMG_CHK_ABORT_RET_LOCKED(p_base, &p_base->mutex);

    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status != 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
    } else {
      IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
    }
    IMG_SEND_EVENT(p_base, QIMG_EVT_FACE_PROC);
  }

  IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
    p_base->state, p_comp->abort_flag);
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT(p_base, QIMG_EVT_DONE);
  return IMG_SUCCESS;

error:
  /* flush rest of the buffers */
  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] Error buf count %d", __func__, __LINE__,
    count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      continue;
    }
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status != 0) {
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
 * Function: faceproc_comp_start
 *
 * Description: Start the execution of faceproc
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
int faceproc_comp_start(void *handle, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
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

  if (!p_comp->config_set) {
    IDBG_ERROR("%s:%d] error config not set", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }
  faceproc_comp_cfg_debug(&p_comp->config);

  faceproc_comp_chromaitix_debug(&p_comp->fd_chromatix);

  status = faceproc_comp_eng_config(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  IMG_CHK_ABORT_UNLK_RET(p_base, &p_base->mutex);

  /* flush the queues */
  img_q_flush(&p_base->inputQ);
  img_q_flush(&p_base->outputQ);

  pthread_mutex_unlock(&p_base->mutex);

  status = p_comp->b.ops.start(&p_comp->b, p_data);
  if (status < 0)
    return status;

  return status;
}

/**
 * Function: faceproc_comp_abort
 *
 * Description: Aborts the execution of faceproc
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int faceproc_comp_abort(void *handle, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = (img_component_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_HIGH("%s:%d] state %d", __func__, __LINE__, p_base->state);
  pthread_mutex_lock(&p_base->mutex);
  if (IMG_STATE_STARTED != p_base->state) {
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }
  p_base->state = IMG_STATE_STOP_REQUESTED;
  pthread_mutex_unlock(&p_base->mutex);
  /*signal the thread*/
  img_q_signal(&p_base->inputQ);

  if (!pthread_equal(pthread_self(), p_base->threadid)) {
    IDBG_MED("%s:%d] thread id %d %d", __func__, __LINE__,
      (uint32_t)pthread_self(), (uint32_t)p_base->threadid);
    pthread_join(p_base->threadid, NULL);
  }

  /* destroy the handle */
  status = faceproc_comp_eng_destroy(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return status;
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return status;
}

/**
 * Function: faceproc_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              faceproc component
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
int faceproc_comp_process (void *handle, img_cmd_type cmd, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: faceproc_comp_queue_buffer
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
int faceproc_comp_queue_buffer(void *handle, img_frame_t *p_frame, img_type_t type)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_queue_t *queue = &p_base->inputQ;
  int count = img_q_count(queue);

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (count > 1) {
    IDBG_MED("%s:%d] Drop the frame %d", __func__, __LINE__, count);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_BUSY;
  }

  status = img_q_enqueue(queue, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }
  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  img_q_signal(queue);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}


/**
 * Function: faceproc_comp_create
 *
 * Description: This function is used to create Qualcomm faceproc component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component_t object. This object
 *            contains the handle and the function pointers for
 *            communicating with the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NO_MEMORY
 *     IMG_ERR_INVALID_INPUT
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_comp_create(void* handle, img_component_ops_t *p_ops)
{
  faceproc_comp_t *p_comp = NULL;
  int status;

  if (NULL == g_faceproc_lib.ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  p_comp = (faceproc_comp_t *)malloc(sizeof(faceproc_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    status = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  memset(p_comp, 0x0, sizeof(faceproc_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    goto error;
  }

  p_comp->p_lib = &g_faceproc_lib;
  /*set the main thread*/
  p_comp->b.thread_loop = face_proc_thread_loop;
  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = faceproc_comp_init;
  p_ops->deinit          = faceproc_comp_deinit;
  p_ops->set_parm        = faceproc_comp_set_param;
  p_ops->get_parm        = faceproc_comp_get_param;
  p_ops->start           = faceproc_comp_start;
  p_ops->abort           = faceproc_comp_abort;
  p_ops->process         = faceproc_comp_process;
  p_ops->queue_buffer    = faceproc_comp_queue_buffer;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;

error:
  IDBG_ERROR("%s:%d] failed %d", __func__, __LINE__, status);
  if (p_comp) {
    free(p_comp);
    p_comp = NULL;
  }
  return status;
}

/**
 * Function: faceproc_comp_load
 *
 * Description: This function is used to load the faceproc library
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
int faceproc_comp_load(const char* name, void** handle)
{
  if (g_faceproc_lib.ptr) {
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  return faceproc_comp_eng_load(&g_faceproc_lib);
}

/**
 * Function: faceproc_comp_unload
 *
 * Description: This function is used to unload the faceproc library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
void faceproc_comp_unload(void* handle)
{
  faceproc_comp_eng_unload(&g_faceproc_lib);
}
