/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/


#include "tp_faceproc.h"
#include "module_imgbase.h"
/**
 * CONSTANTS and MACROS
 **/
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1088

#define MAX_FD_COUNT 50

/** g_fd_chromatix_tp chromatic:
 *
 *  Chromatix for Face detection test
 **/
static fd_chromatix_t g_fd_chromatix_tp = {
  #include "fd_chromatix_tp.h"
};

/**
 * Function: faceproc_test_event_handler
 *
 * Description: event handler for FaceProc test case
 *
 * Input parameters:
 *   p_appdata - FaceProc test object
 *   p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_event_handler(void* p_appdata, img_event_t *p_event)
{
  faceproc_tp_t *p_faceproc = (faceproc_tp_t *)p_appdata;
  img_component_ops_t *p_comp = &p_faceproc->base->comp;
  img_core_ops_t *p_core_ops = &p_faceproc->base->core_ops;
  int rc = IMG_SUCCESS;
  img_frame_t *p_frame;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return 0;
  }
  IDBG_MED("%s:%d] type %d", __func__, __LINE__, p_event->type);
  switch (p_event->type) {
  case QIMG_EVT_FACE_PROC:
    /* get the result*/
    rc = IMG_COMP_GET_PARAM(p_comp, QWD_FACEPROC_RESULT,
      (void *)&p_faceproc->result);
    if (rc != IMG_SUCCESS) {
      IDBG_MED("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
    pthread_cond_signal(&p_faceproc->base->cond);
    break;
  case QIMG_EVT_BUF_DONE:
    /*send the buffer back*/
    rc = IMG_COMP_DQ_BUF(p_comp, &p_frame);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
    IDBG_MED("%s:%d] buffer idx %d", __func__, __LINE__, p_frame->idx);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
  default:;
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_test_init
 *
 * Description: init FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_init(faceproc_tp_t *p_faceproc)
{
  int rc = IMG_SUCCESS;
  img_param_type param;
  img_component_ops_t *p_comp = &p_faceproc->base->comp;
  img_core_ops_t *p_core_ops = &p_faceproc->base->core_ops;
  p_faceproc->mode = FACE_DETECT;
  p_faceproc->config.frame_cfg.max_width = p_faceproc->base->frame[0].info.width;
  p_faceproc->config.frame_cfg.max_height = p_faceproc->base->frame[0].info.height;
  p_faceproc->config.histogram_enable = FALSE;
  p_faceproc->config.face_cfg.min_face_size = MIN_FACE_SIZE;
  p_faceproc->config.face_cfg.max_face_size = 1500;
  p_faceproc->config.face_cfg.max_num_face_to_detect = 2;
  p_faceproc->config.face_cfg.face_orientation_hint = FD_FACE_ORIENTATION_UNKNOWN;
  p_faceproc->config.face_cfg.rotation_range = 45;
  p_faceproc->config.histogram_enable = 0;
  p_faceproc->config.fd_feature_mask = FACE_PROP_DEFAULT;
  p_faceproc->test_chromatix = &g_fd_chromatix_tp;

  rc = img_core_get_comp(IMG_COMP_FACE_PROC, "qcom.faceproc", p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_INIT(p_comp, (void *)p_faceproc, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_CB(p_comp, faceproc_test_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_MODE,
    (void *)&p_faceproc->mode);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_CFG,
    (void *)&p_faceproc->config);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_CHROMATIX,
    (void *)p_faceproc->test_chromatix);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  return 0;
}

/**
 * Function: faceproc_test_process_frame
 *
 * Description: start FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_process_frame(faceproc_tp_t *p_faceproc)
{
  int i = 0;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_faceproc->base->comp;
  img_core_ops_t *p_core_ops = &p_faceproc->base->core_ops;

  for (i = 0; i < p_faceproc->base->in_count; i++) {
    IDBG_MED("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
      p_faceproc->base->frame[i].info.width,
      p_faceproc->base->frame[i].info.height,
      &p_faceproc->base->frame[i]);
    rc = IMG_COMP_Q_BUF(p_comp, &p_faceproc->base->frame[i], IMG_IN);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
  }
  return 0;
}

/**
 * Function: faceproc_test_start
 *
 * Description: start FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - fd object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_start(faceproc_tp_t *p_faceproc)
{
  int i = 0;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_faceproc->base->comp;
  img_core_ops_t *p_core_ops = &p_faceproc->base->core_ops;

  rc = IMG_COMP_START(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  /* wait for the result */
  pthread_mutex_lock(&p_faceproc->base->mutex);
  rc = faceproc_test_process_frame(p_faceproc);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_faceproc->base->mutex);
    return rc;
  }
  IDBG_ERROR("%s:%d] before wait rc %d", __func__, __LINE__, rc);
  rc = img_wait_for_completion(&p_faceproc->base->cond, &p_faceproc->base->mutex,
    10000);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_faceproc->base->mutex);
    return rc;
  }
  IDBG_MED("%s:%d] after wait rc %d", __func__, __LINE__, rc);
  pthread_mutex_unlock(&p_faceproc->base->mutex);
  return 0;
}

/**
 * Function: faceproc_test_stop
 *
 * Description: stop FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_stop(faceproc_tp_t *p_faceproc)
{
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_test_deinit
 *
 * Description: deinit FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_deinit(faceproc_tp_t *p_faceproc)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_faceproc->base->comp;
  img_core_ops_t *p_core_ops = &p_faceproc->base->core_ops;

  rc = IMG_COMP_DEINIT(p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_test_finish
 *
 * Description: finish executing FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_finish(faceproc_tp_t *p_faceproc)
{
  int rc = IMG_SUCCESS;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  rc = faceproc_test_deinit(p_faceproc);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_test_execute
 *
 * Description: execute FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_execute(faceproc_tp_t *p_faceproc)
{
  int rc = IMG_SUCCESS;

  rc = faceproc_test_init(p_faceproc);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = faceproc_test_start(p_faceproc);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = faceproc_test_finish(p_faceproc);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }
  return rc;
}

/**
 * Function: process_fd_on_frame
 *
 * Description: Appliies FD on a given frame
 *
 * Input parameters:
 *   p_frame - input image frame
 *   p_meta - imglib meta data structure
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int process_fd_on_frame(img_frame_t *p_frame, img_meta_t *p_meta)
{
  int rc = IMG_SUCCESS, c, i, val = 0;
  int lrc = IMG_SUCCESS;
  uint32_t num = 0;
  imglib_data_t img_data;
  faceproc_tp_t faceproc_data;

  /* Initialize the structures */
  memset(&img_data, 0x0, sizeof(img_data));
  memset(&faceproc_data, 0x0, sizeof(faceproc_tp_t));
  pthread_mutex_init(&img_data.mutex, NULL);
  pthread_cond_init(&img_data.cond, NULL);
  memcpy(&img_data.frame[0], p_frame, sizeof (img_frame_t));

  img_data.in_count = 1;

  faceproc_data.base = &img_data;
  faceproc_result_t *fd_result = &faceproc_data.result;
  img_fd_info_t *fd_info = &p_meta->fd_info;

  p_meta->fd_frame_dim.width  = faceproc_data.base->frame[0].info.width;
  p_meta->fd_frame_dim.height = faceproc_data.base->frame[0].info.height;
  p_meta->rotation = 0;

  faceproc_data.base->frame[0].frame[0].plane[0].width = p_frame->frame[0].plane[0].stride;

  IDBG_HIGH("%s:%d] start FD %d x %d",  __func__, __LINE__,
    faceproc_data.base->frame[0].frame[0].plane[0].width,
    faceproc_data.base->frame[0].frame[0].plane[0].height);

  rc = faceproc_test_execute(&faceproc_data);

  IDBG_HIGH("%s:%d]end FD %d", __func__, __LINE__, rc);

  p_meta->valid_faces_detected = 0;

  for (num = 0; num < fd_result->num_faces_detected; num++) {
    /* check orientation of the face; if it is in gravity direction,
            then only it should be passed to trueportrait; if not filter
            that face alone */
    IDBG_HIGH("%s:%d] roll_dir before sensor mount angle comp off %d, %d",
      __func__, __LINE__, fd_result->roi[num].fp.direction_roll,
      ABS(fd_result->roi[num].fp.direction_roll + p_meta->rotation));

    if ( (ABS(fd_result->roi[num].fp.direction_roll) < FACE_TILT_CUTOFF_FOR_TP) &&
      (p_meta->valid_faces_detected < MAX_FACES_SUPPORTED_BY_TP) ) {
      fd_info->faceROIx[p_meta->valid_faces_detected] =
      fd_result->roi[num].face_boundary.x;
      fd_info->faceROIy[p_meta->valid_faces_detected] =
      fd_result->roi[num].face_boundary.y;
      fd_info->faceROIWidth[p_meta->valid_faces_detected] =
      fd_result->roi[num].face_boundary.dx;
      fd_info->faceROIHeight[p_meta->valid_faces_detected] =
      fd_result->roi[num].face_boundary.dy;

      p_meta->valid_faces_detected++;
    }

    IDBG_MED("%s:%d] Face info: %d %d, %d, %d", __func__, __LINE__,
    fd_result->roi[num].left_right_gaze, fd_result->roi[num].top_bottom_gaze,
    fd_result->roi[num].fp.direction_up_down,fd_result->roi[num].fp.direction_roll);
  }

  IDBG_ERROR("%s:%d] Final number of faces filtered for TP is %d outof %d, %d x %d; %d",
    __func__, __LINE__, p_meta->valid_faces_detected, fd_result->num_faces_detected,
    p_meta->fd_frame_dim.width, p_meta->fd_frame_dim.height, p_frame->frame[0].plane[0].stride);


  if (rc != IMG_SUCCESS) {
    fprintf(stderr, "Error rc %d", rc);
    goto error;
  }

  goto exit;

  error: for (i = 0; i < img_data.mem_cnt; i++) {
    if (img_data.addr[i] != NULL) {
      free(img_data.addr[i]);
      img_data.addr[i] = NULL;
    }
  }

exit:
  pthread_mutex_destroy(&img_data.mutex);
  pthread_cond_destroy(&img_data.cond);

  return val;
}

