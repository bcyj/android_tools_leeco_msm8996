/**********************************************************************
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
 * Qualcomm Technologies Proprietary and Confidential.                 *
 **********************************************************************/

#include "img_test.h"

static const uint8_t g_gamma_table[1024] = { 2, 2, 3, 4, 4, 5, 6, 6, 8, 8, 9,
  10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24,
  25, 26, 26, 27, 28, 29, 30, 30, 31, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39,
  40, 41, 42, 42, 43, 44, 44, 45, 46, 47, 48, 49, 49, 50, 51, 51, 52, 53, 53,
  54, 55, 55, 56, 56, 57, 58, 59, 59, 60, 61, 61, 62, 63, 63, 64, 64, 65, 66,
  66, 67, 67, 68, 69, 69, 70, 70, 71, 71, 72, 73, 73, 74, 74, 75, 75, 76, 76,
  77, 77, 78, 78, 79, 80, 80, 81, 81, 82, 82, 83, 83, 83, 84, 84, 85, 85, 86,
  86, 87, 87, 87, 88, 88, 89, 89, 90, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94,
  95, 95, 96, 96, 96, 97, 97, 98, 98, 99, 99, 99, 100, 100, 100, 101, 101, 102,
  102, 102, 103, 103, 104, 104, 105, 105, 105, 106, 106, 107, 107, 107, 108,
  108, 109, 109, 109, 110, 110, 110, 111, 111, 111, 112, 112, 113, 113, 114,
  114, 114, 115, 115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 119,
  119, 119, 120, 120, 121, 121, 121, 122, 122, 123, 123, 123, 124, 124, 124,
  124, 125, 125, 125, 126, 126, 126, 127, 127, 127, 128, 128, 128, 129, 129,
  129, 130, 130, 130, 131, 131, 132, 132, 132, 132, 133, 133, 133, 134, 134,
  134, 134, 135, 135, 135, 136, 136, 136, 136, 137, 137, 137, 138, 138, 138,
  139, 139, 139, 140, 140, 140, 141, 141, 141, 142, 142, 142, 142, 143, 143,
  143, 143, 144, 144, 144, 144, 145, 145, 145, 145, 146, 146, 146, 146, 147,
  147, 147, 148, 148, 148, 148, 149, 149, 149, 150, 150, 150, 150, 151, 151,
  151, 152, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 154, 155, 155,
  155, 155, 155, 156, 156, 156, 156, 157, 157, 157, 157, 157, 158, 158, 158,
  158, 159, 159, 159, 159, 160, 160, 160, 160, 161, 161, 161, 161, 162, 162,
  162, 162, 163, 163, 163, 163, 163, 164, 164, 164, 164, 165, 165, 165, 165,
  166, 166, 166, 166, 167, 167, 167, 167, 167, 168, 168, 168, 168, 168, 169,
  169, 169, 169, 169, 170, 170, 170, 170, 170, 171, 171, 171, 171, 171, 172,
  172, 172, 172, 172, 173, 173, 173, 173, 174, 174, 174, 174, 174, 174, 175,
  175, 175, 175, 175, 176, 176, 176, 176, 176, 177, 177, 177, 177, 177, 178,
  178, 178, 178, 178, 178, 179, 179, 179, 179, 179, 180, 180, 180, 180, 180,
  180, 181, 181, 181, 181, 181, 182, 182, 182, 182, 182, 183, 183, 183, 183,
  183, 184, 184, 184, 184, 184, 185, 185, 185, 185, 185, 185, 186, 186, 186,
  186, 186, 187, 187, 187, 187, 187, 187, 188, 188, 188, 188, 188, 188, 189,
  189, 189, 189, 189, 189, 190, 190, 190, 190, 190, 190, 191, 191, 191, 191,
  191, 191, 191, 192, 192, 192, 192, 192, 192, 193, 193, 193, 193, 193, 193,
  194, 194, 194, 194, 194, 194, 195, 195, 195, 195, 195, 195, 196, 196, 196,
  196, 196, 196, 197, 197, 197, 197, 197, 197, 198, 198, 198, 198, 198, 198,
  199, 199, 199, 199, 199, 200, 200, 200, 200, 200, 200, 201, 201, 201, 201,
  201, 201, 202, 202, 202, 202, 202, 202, 203, 203, 203, 203, 203, 203, 204,
  204, 204, 204, 204, 204, 204, 205, 205, 205, 205, 205, 205, 205, 205, 206,
  206, 206, 206, 206, 206, 206, 206, 207, 207, 207, 207, 207, 207, 207, 208,
  208, 208, 208, 208, 208, 208, 209, 209, 209, 209, 209, 209, 210, 210, 210,
  210, 210, 210, 211, 211, 211, 211, 211, 211, 211, 212, 212, 212, 212, 212,
  212, 213, 213, 213, 213, 213, 213, 213, 213, 214, 214, 214, 214, 214, 214,
  214, 214, 215, 215, 215, 215, 215, 215, 215, 216, 216, 216, 216, 216, 216,
  216, 216, 217, 217, 217, 217, 217, 217, 217, 218, 218, 218, 218, 218, 218,
  218, 218, 219, 219, 219, 219, 219, 219, 219, 219, 220, 220, 220, 220, 220,
  220, 220, 220, 221, 221, 221, 221, 221, 221, 221, 221, 222, 222, 222, 222,
  222, 222, 222, 222, 223, 223, 223, 223, 223, 223, 223, 223, 223, 224, 224,
  224, 224, 224, 224, 224, 224, 225, 225, 225, 225, 225, 225, 225, 225, 226,
  226, 226, 226, 226, 226, 226, 226, 227, 227, 227, 227, 227, 227, 227, 227,
  228, 228, 228, 228, 228, 228, 228, 228, 228, 229, 229, 229, 229, 229, 229,
  229, 229, 229, 230, 230, 230, 230, 230, 230, 230, 230, 230, 231, 231, 231,
  231, 231, 231, 231, 231, 231, 232, 232, 232, 232, 232, 232, 232, 232, 232,
  232, 233, 233, 233, 233, 233, 233, 233, 233, 233, 234, 234, 234, 234, 234,
  234, 234, 234, 235, 235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236,
  236, 236, 236, 236, 236, 236, 237, 237, 237, 237, 237, 237, 237, 237, 237,
  238, 238, 238, 238, 238, 238, 238, 238, 238, 239, 239, 239, 239, 239, 239,
  239, 239, 239, 240, 240, 240, 240, 240, 240, 240, 240, 240, 241, 241, 241,
  241, 241, 241, 241, 241, 241, 241, 241, 242, 242, 242, 242, 242, 242, 242,
  242, 242, 242, 243, 243, 243, 243, 243, 243, 243, 243, 243, 243, 243, 244,
  244, 244, 244, 244, 244, 244, 244, 244, 245, 245, 245, 245, 245, 245, 245,
  245, 245, 245, 246, 246, 246, 246, 246, 246, 246, 246, 246, 247, 247, 247,
  247, 247, 247, 247, 247, 247, 248, 248, 248, 248, 248, 248, 248, 248, 248,
  249, 249, 249, 249, 249, 249, 249, 249, 249, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 250, 251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
  251, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 253, 253, 253, 253,
  253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254,
  254, 254, 255, 255, 255, 255, 255 };

/**
 * Function: hdr_test_event_handler
 *
 * Description: event handler for HDR test case
 *
 * Input parameters:
 *   p_appdata - HDR test object
 *   p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int hdr_test_event_handler(void* p_appdata, img_event_t *p_event)
{
  hdr_test_t *p_test = (hdr_test_t *)p_appdata;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return 0;
  }
  IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, p_event->type);
  switch (p_event->type) {
  case QIMG_EVT_DONE:
    pthread_cond_signal(&p_test->base->cond);
    break;
  default:
    ;
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_test_init
 *
 * Description: init HDR test case
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
int hdr_test_init(hdr_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_param_type param;
  p_test->base->p_comp = &p_test->base->comp;
  p_test->base->p_core_ops = &p_test->base->core_ops;
  p_test->out_index = 0;
  p_test->mode = (p_test->base->in_count == 1) ? SINGLE_FRAME : MULTI_FRAME;
  p_test->analyse = 0;

  rc = img_core_get_comp(IMG_COMP_HDR, "qcom.hdr", p_test->base->p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_LOAD(p_test->base->p_core_ops, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_CREATE(p_test->base->p_core_ops, p_test->base->p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_INIT(p_test->base->p_comp, (void *)p_test, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_CB(p_test->base->p_comp, hdr_test_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QHDR_MODE,
      (void *)&p_test->mode);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QHDR_ANALYZE_IMAGE,
      (void *)&p_test->analyse);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  /*subsample gamma table*/
  SUBSAMPLE_TABLE(g_gamma_table, 1024, p_test->gamma.table, 64, 0);
  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QHDR_GAMMA_TABLE,
      (void *)&p_test->gamma);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QHDR_OUT_INDEX,
      (void *)&p_test->out_index);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  return 0;
}

/**
 * Function: hdr_test_start
 *
 * Description: start HDR test case
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
int hdr_test_start(hdr_test_t *p_test)
{
  int i = 0;
  int rc = IMG_SUCCESS;

  for (i = 0; i < p_test->base->in_count; i++) {
    IDBG_HIGH("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
      p_test->base->frame[i].info.width, p_test->base->frame[i].info.height,
      &p_test->base->frame[i]);
    rc = IMG_COMP_Q_BUF(p_test->base->p_comp, &p_test->base->frame[i],
        IMG_IN);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
  }

  pthread_mutex_lock(&p_test->base->mutex);
  rc = IMG_COMP_START(p_test->base->p_comp, NULL);
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
 * Function: hdr_test_stop
 *
 * Description: stop HDR test case
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
int hdr_test_stop(hdr_test_t *p_test)
{
  return IMG_SUCCESS;
}

/**
 * Function: hdr_test_deinit
 *
 * Description: deinit HDR test case
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
int hdr_test_deinit(hdr_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  rc = IMG_COMP_DEINIT(p_test->base->p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_UNLOAD(p_test->base->p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_test_finish
 *
 * Description: finish executing HDR test case
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
int hdr_test_finish(hdr_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  IDBG_ERROR("%s:%d] index %d", __func__, __LINE__, p_test->out_index);
  rc = img_test_write(p_test->base, p_test->base->out_fn, p_test->out_index);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = hdr_test_deinit(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_test_execute
 *
 * Description: execute HDR test case
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
int hdr_test_execute(hdr_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  rc = hdr_test_init(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = hdr_test_start(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = hdr_test_finish(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }
  return rc;
}
