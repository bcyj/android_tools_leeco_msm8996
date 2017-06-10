/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include "img_test.h"

/**
 * CONSTANTS and MACROS
 **/
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1088

#define MAX_FD_COUNT 50

/** g_test_chromatix chromatic:
 *
 *  Chromatix for Face detection test
 **/
static fd_chromatix_t g_test_chromatix = {
  #include "fd_chromatix_test_qc.h"
};

/**
 * Function: faceproc_print_result
 *
 * Description: print the faceproc result
 *
 * Input parameters:
 *   p_result - Face detection result
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
void faceproc_print_result(faceproc_result_t *p_result)
{
  uint32_t i = 0;
  IDBG_MED("Faceproc result num_faces_detected %d",
    p_result->num_faces_detected);

  for (i = 0; i < p_result->num_faces_detected; i++) {
    IDBG_MED("Faceproc face[%d] blink_detected %d", i,
      p_result->roi[i].blink_detected);
    IDBG_MED("Faceproc face[%d] face_boundary.x %d", i,
      p_result->roi[i].face_boundary.x);
    IDBG_MED("Faceproc face[%d] face_boundary.y %d", i,
      p_result->roi[i].face_boundary.y);
    IDBG_MED("Faceproc face[%d] face_boundary.dx %d", i,
      p_result->roi[i].face_boundary.dx);
    IDBG_MED("Faceproc face[%d] face_boundary.dy %d", i,
      p_result->roi[i].face_boundary.dy);
    IDBG_MED("Faceproc face[%d] fd_confidence %d", i,
      p_result->roi[i].fd_confidence);
    IDBG_MED("Faceproc face[%d] fp.direction_left_right %d", i,
      p_result->roi[i].fp.direction_left_right);
    IDBG_MED("Faceproc face[%d] fp.direction_up_down %d", i,
      p_result->roi[i].fp.direction_up_down);
    IDBG_MED("Faceproc face[%d] fp.direction_roll %d", i,
      p_result->roi[i].fp.direction_roll);
    IDBG_MED("Faceproc face[%d] gaze_angle %d", i,
      p_result->roi[i].gaze_angle);
    IDBG_MED("Faceproc face[%d] is_face_recognised %d", i,
      p_result->roi[i].is_face_recognised);
    IDBG_MED("Faceproc face[%d] left_blink %d", i,
      p_result->roi[i].left_blink);
    IDBG_MED("Faceproc face[%d] right_blink %d", i,
      p_result->roi[i].right_blink);
    IDBG_MED("Faceproc face[%d] left_right_gaze %d", i,
      p_result->roi[i].left_right_gaze);
    IDBG_MED("Faceproc face[%d] top_bottom_gaze %d", i,
      p_result->roi[i].top_bottom_gaze);
    IDBG_MED("Faceproc face[%d] sm.confidence %d", i,
      p_result->roi[i].sm.confidence);
    IDBG_MED("Faceproc face[%d] sm.smile_degree %d", i,
      p_result->roi[i].sm.smile_degree);
    IDBG_MED("Faceproc face[%d] sm.unique_id %d", i,
      p_result->roi[i].unique_id);
  }
}

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
  static int count = 0;
  faceproc_test_t *p_test = (faceproc_test_t *)p_appdata;
  img_component_ops_t *p_comp = &p_test->base->comp;
  img_core_ops_t *p_core_ops = &p_test->base->core_ops;
  int rc = IMG_SUCCESS;
  img_frame_t *p_frame;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return 0;
  }
  IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, p_event->type);
  switch (p_event->type) {
  case QIMG_EVT_FACE_PROC:
    /* get the result*/
    rc = IMG_COMP_GET_PARAM(p_comp, QWD_FACEPROC_RESULT,
      (void *)&p_test->result);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
    count++;
    IDBG_MED("%s:%d] count %d %d", __func__, __LINE__,
      p_test->base->in_count, count);
    if (MAX_FD_COUNT <= count) {
      pthread_cond_signal(&p_test->base->cond);
    }
    faceproc_print_result(&p_test->result);
    break;
  case QIMG_EVT_BUF_DONE:
    /*send the buffer back*/
    rc = IMG_COMP_DQ_BUF(p_comp, &p_frame);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
    IDBG_HIGH("%s:%d] buffer idx %d", __func__, __LINE__, p_frame->idx);
    /*send the buffer back*/
    rc = IMG_COMP_Q_BUF(p_comp, &p_test->base->frame[p_frame->idx],
      IMG_IN);
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
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_init(faceproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_param_type param;
  img_component_ops_t *p_comp = &p_test->base->comp;
  img_core_ops_t *p_core_ops = &p_test->base->core_ops;
  p_test->mode = FACE_DETECT;
  p_test->config.frame_cfg.max_width = MAX_WIDTH;
  p_test->config.frame_cfg.max_height = MAX_HEIGHT;
  p_test->config.histogram_enable = FALSE;
  p_test->config.face_cfg.min_face_size = MIN_FACE_SIZE;
  p_test->config.face_cfg.max_face_size = 1500;
  p_test->config.face_cfg.max_num_face_to_detect = 2;
  p_test->config.face_cfg.face_orientation_hint = FD_FACE_ORIENTATION_UNKNOWN;
  p_test->config.face_cfg.rotation_range = 45;
  p_test->config.histogram_enable = 0;
  p_test->config.fd_feature_mask = FACE_PROP_DEFAULT;
  p_test->test_chromatix = &g_test_chromatix;

  rc = img_core_get_comp(IMG_COMP_FACE_PROC, "qcom.faceproc", p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_LOAD(p_core_ops, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_INIT(p_comp, (void *)p_test, NULL);
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
    (void *)&p_test->mode);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_CFG,
    (void *)&p_test->config);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_CHROMATIX,
    (void *)p_test->test_chromatix);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  return 0;
}

/**
 * Function: faceproc_test_start
 *
 * Description: start FaceProc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_process_frame(faceproc_test_t *p_test)
{
  int i = 0;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_test->base->comp;
  img_core_ops_t *p_core_ops = &p_test->base->core_ops;

  for (i = 0; i < p_test->base->in_count; i++) {
    IDBG_HIGH("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
      p_test->base->frame[i].info.width,
      p_test->base->frame[i].info.height,
      &p_test->base->frame[i]);
    rc = IMG_COMP_Q_BUF(p_comp, &p_test->base->frame[i], IMG_IN);
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
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_start(faceproc_test_t *p_test)
{
  int i = 0;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_test->base->comp;
  img_core_ops_t *p_core_ops = &p_test->base->core_ops;

  rc = IMG_COMP_START(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  /* wait for the result */
  pthread_mutex_lock(&p_test->base->mutex);
  rc = faceproc_test_process_frame(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_test->base->mutex);
    return rc;
  }
  IDBG_HIGH("%s:%d] before wait rc %d", __func__, __LINE__, rc);
  rc = img_wait_for_completion(&p_test->base->cond, &p_test->base->mutex,
    10000);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_test->base->mutex);
    return rc;
  }
  IDBG_HIGH("%s:%d] after wait rc %d", __func__, __LINE__, rc);
  pthread_mutex_unlock(&p_test->base->mutex);
  return 0;
}

/**
 * Function: faceproc_test_stop
 *
 * Description: stop FaceProc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_stop(faceproc_test_t *p_test)
{
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_test_deinit
 *
 * Description: deinit FaceProc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_deinit(faceproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_test->base->comp;
  img_core_ops_t *p_core_ops = &p_test->base->core_ops;

  rc = IMG_COMP_DEINIT(p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  IMG_COMP_UNLOAD(p_core_ops);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_test_finish
 *
 * Description: finish executing FaceProc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_finish(faceproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  IDBG_ERROR("%s:%d] E", __func__, __LINE__);

  rc = faceproc_test_deinit(p_test);
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
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_test_execute(faceproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  rc = faceproc_test_init(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = faceproc_test_start(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = faceproc_test_finish(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }
  return rc;
}
