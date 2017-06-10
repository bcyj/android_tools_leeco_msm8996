/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include "img_test.h"
#include "chromatix.h"

static const uint16_t g_gamma_table[64] =
{
  1280, 2309, 2830, 3097, 2853, 2608, 2874, 2373, 2382, 2391, 1888, 1895, 1902,
  1653, 1659, 1409, 1414, 1419, 1168, 1428, 921, 1180, 1184, 932, 935, 938, 941,
  944, 947, 694, 952, 699, 957, 960, 707, 965, 712, 970, 717, 719, 977, 724,
  726, 984, 731, 733, 735, 737, 739, 741, 743, 745, 491, 748, 750, 752, 498,
  755, 757, 503, 760, 762, 508, 765
};

/**
 * Function: cac_test_event_handler
 *
 * Description: event handler for HDR test case
 *
 * Input parameters:
 *   p_appdata - CAC test object
 *   p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int cac_test_event_handler(void* p_appdata, img_event_t *p_event)
{
  cac_test_t *p_test = (cac_test_t *)p_appdata;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return 0;
  }
  IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, p_event->type);
  switch (p_event->type) {
  case QIMG_EVT_DONE:
    pthread_cond_signal(&p_test->base->cond);
    break;
  default:;
  }
  return IMG_SUCCESS;
}


/**
 * Function: cac_test_init
 *
 * Description: init CAC test case
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
int cac_test_init(cac_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  p_test->base->p_comp = &p_test->base->comp;
  p_test->base->p_core_ops = &p_test->base->core_ops;
  p_test->info_3a.awb_gb_gain = 127;
  p_test->info_3a.awb_gr_gain = 127;
  p_test->chroma_order = CAC_CHROMA_ORDER_CRCB;
  p_test->chromatix_info.edgeTH = 20;
  p_test->chromatix_info.saturatedTH = 120;
  p_test->chromatix_info.chrom0LowTH = 8;
  p_test->chromatix_info.chrom0HighTH = 448;
  p_test->chromatix_info.chrom1LowTH = 8;
  p_test->chromatix_info.chrom1HighTH = 448;
  p_test->chromatix_info.chrom0LowDiffTH = 192;
  p_test->chromatix_info.chorm0HighDiffTH = 320;
  p_test->chromatix_info.chrom1LowDiffTH = 192;
  p_test->chromatix_info.chorm1HighDiffTH = 320;



  rc = img_core_get_comp(IMG_COMP_CAC, "qcom.cac",
    p_test->base->p_core_ops);
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

  rc = IMG_COMP_SET_CB(p_test->base->p_comp, cac_test_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QCAC_3A_INFO,
    (void *)&p_test->info_3a);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QCAC_CHROMATIX_INFO,
    (void *)&p_test->chromatix_info);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QCAC_CHROMA_ORDER,
    (void *)&p_test->chroma_order);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QCAC_RGAMMA_TABLE,
    &g_gamma_table);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QCAC_GGAMMA_TABLE,
    &g_gamma_table);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QCAC_BGAMMA_TABLE,
    &g_gamma_table);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  return 0;

}

/**
 * Function: cac_test_start
 *
 * Description: start CAC test case
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
int cac_test_start(cac_test_t *p_test)
{
  int i = 0;
  int rc = IMG_SUCCESS;

  for (i = 0; i < p_test->base->in_count; i++) {
    IDBG_HIGH("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
      p_test->base->frame[i].info.width,
      p_test->base->frame[i].info.height,
      &p_test->base->frame[i]);
    rc = IMG_COMP_Q_BUF(p_test->base->p_comp, &p_test->base->frame[i],
      IMG_IN_OUT);
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
 * Function: cac_test_stop
 *
 * Description: stop CAC test case
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
int cac_test_stop(cac_test_t *p_test)
{
  return IMG_SUCCESS;
}

/**
 * Function: cac_test_deinit
 *
 * Description: CAC test case
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
int cac_test_deinit(cac_test_t *p_test)
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
 * Function: cac_test_finish
 *
 * Description: finish executing CAC test case
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
int cac_test_finish(cac_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  IDBG_ERROR("%s:%d] ", __func__, __LINE__);
  rc = img_test_write(p_test->base, p_test->base->out_fn, 0);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = cac_test_deinit(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  return IMG_SUCCESS;
}

/**
 * Function: cac_test_execute
 *
 * Description: execute CAC test case
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
int cac_test_execute(cac_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  rc = cac_test_init(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = cac_test_start(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = cac_test_finish(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }
  IDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return rc;
}
