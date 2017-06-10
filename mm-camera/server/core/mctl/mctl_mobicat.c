/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "mm_camera_interface.h"
#include "mctl.h"

#define MAX_MCB_BUF_SIZE 5000

#define update_mbc_string(buffer, buf_size, mbc_str, val, \
  len, filled_len, p_meta_str) ({ \
  snprintf(buffer, buf_size, mbc_str, val); \
  len = strlen(buffer); \
  if ((filled_len + len) > MAX_MOBICAT_SIZE) { \
    CDBG_ERROR("%s:%d] Error", __func__, __LINE__); \
    return -1; \
  } \
  memcpy(p_meta_str, buffer, len); \
  CDBG("%s:%d] buffer %s", __func__, __LINE__, buffer); \
  p_meta_str += len; \
  filled_len += len; \
  })

#define update_mbc_string2(buffer, buf_size, mbc_str, i, val, \
  len, filled_len, p_meta_str) ({ \
  snprintf(buffer, buf_size, mbc_str, i, val); \
  len = strlen(buffer); \
  if ((filled_len + len) > MAX_MOBICAT_SIZE) { \
    CDBG_ERROR("%s:%d] Error", __func__, __LINE__); \
    return -1; \
  } \
  memcpy(p_meta_str, buffer, len); \
  CDBG("%s:%d] buffer %s", __func__, __LINE__, buffer); \
  p_meta_str += len; \
  filled_len += len; \
  })

#define flatten_int_arr(buffer, buf_size, val, cnt) ({ \
  int k = 0; \
  const int t_sz = 32; \
  char temp[t_sz]; \
  buffer[0] = '\0'; \
  for (k = 0; k < cnt-1; k++) { \
    snprintf(temp, t_sz, "%d,", val[k]); \
    strncat(buffer, temp, strlen(temp)); \
  } \
  snprintf(temp, t_sz, "%d", val[cnt-1]); \
  strncat(buffer, temp, strlen(temp)); \
})

/*===========================================================================
 * FUNCTION    - parse_mobicat_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
int parse_mobicat_info(QCameraInfo_t *p_mobicat_info,
  cam_exif_tags_t *p_metadata)
{
  char *p_meta_str = p_metadata->tags;
  const int buf_size = MAX_MCB_BUF_SIZE;
  uint32_t len = 0;
  int i = 0;
  uint32_t filled_len = 0;

  char *buffer = (char *)malloc(buf_size * sizeof(char));
  if (buffer == NULL) {
      CDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
      return -1;
  }

  char *buffer1 = (char *)malloc(buf_size * sizeof(char));
  if (buffer1 == NULL) {
      CDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
      free(buffer);
      return -1;
  }
  memset(buffer, 0x0, buf_size * sizeof(char));
  memset(buffer1, 0x0, buf_size * sizeof(char));
  if (p_mobicat_info == NULL) {
      CDBG_ERROR("%s:%d] Error Invalid mobicat info", __func__, __LINE__);
      return -1;
  }
  QISPInfo_t isp = p_mobicat_info->isp_info;
  Q3AInfo_t stats = p_mobicat_info->stats_proc_info;

  p_metadata->data_len = 0;
  //isp info
  QColorCorrect_t color_correct = isp.cc;
  QLinearization_t linearization = isp.linearization;
  QDemosaic_t demosaic = isp.demosaic;
  QLensCorrection_t lens_correct = isp.lc;
  QGamma_t gamma = isp.gamma;
  QABF_t abf = isp.abf;
  QMemoryColor_t mce = isp.mce;
  QASF_t asf = isp.asf;

  //stats info
  QAWBInfo_t awb = stats.awb_info;
  QAECInfo_t aec = stats.aec_info;
  QAFInfo_t af = stats.af_info;

  //Fill in QWB_Info_t info in APP5 marker payload.
  update_mbc_string(buffer, buf_size, "awb-colorTemp=%d\n", awb.colorTemp,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "awb-rGain=%f\n", awb.wbGain.rGain,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "awb-gGain=%f\n", awb.wbGain.gGain,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "awb-bGain=%f\n", awb.wbGain.bGain,
    len, filled_len, p_meta_str);

  //Fill in QAEC_Info_t info in APP5 marker payload.
  update_mbc_string(buffer, buf_size, "aec-expIndex=%d\n", aec.expIndex,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "aec-lightCond=%d\n", aec.lightCond,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "aec-expMode=%d\n", aec.expMode,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "aec-expBias=%d\n", aec.expBias,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "aec-analogGain=%f\n", aec.analogGain,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "aec-expTime=%f\n", aec.expTime,
    len, filled_len, p_meta_str);

  //Fill in QAF_Info_t info in APP5 marker payload.
  update_mbc_string(buffer, buf_size, "af-fcsMode=%d\n", af.focusMode,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "af-cafStatus=%d\n", af.cafStatus,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "af-startLensPos=%d\n", af.startLensPos,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "af-finalLensPos=%d\n", af.finalLensPos,
    len, filled_len, p_meta_str);
  update_mbc_string(buffer, buf_size, "af-numOfFcsArea=%d\n", af.numOfFocusArea,
    len, filled_len, p_meta_str);

  for(i=0; i<MAX_ROI; i++) {
    update_mbc_string2(buffer, buf_size, "af-fcsArea[%d].x=%d\n", i,
      af.focusArea[i].x,
      len, filled_len, p_meta_str);
    update_mbc_string2(buffer, buf_size, "af-fcsArea[%d].y=%d\n", i,
      af.focusArea[i].y,
      len, filled_len, p_meta_str);
    update_mbc_string2(buffer, buf_size, "af-fcsArea[%d].dx=%d\n", i,
      af.focusArea[i].dx,
      len, filled_len, p_meta_str);
    update_mbc_string2(buffer, buf_size, "af-fcsArea[%d].dy=%d\n", i,
      af.focusArea[i].dy,
      len, filled_len, p_meta_str);
  }

  flatten_int_arr(buffer1, buf_size, af.focusValue, 50);
  update_mbc_string(buffer, buf_size, "af-fcsValue=%s\n", buffer1,
    len, filled_len, p_meta_str);

  flatten_int_arr(buffer1, buf_size, af.focusSteps, 50);
  update_mbc_string(buffer, buf_size, "af-fcsSteps=%s\n", buffer1,
    len, filled_len, p_meta_str);

  update_mbc_string(buffer, buf_size, "af-index=%d\n", af.index,
    len, filled_len, p_meta_str);

  //Fill in QColorCorrect_t info in APP5 marker payload.
  if (color_correct.enable) {
    update_mbc_string(buffer, buf_size, "ccm-coef_rtor=%d\n",
      color_correct.coef_rtor,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_gtor=%d\n",
      color_correct.coef_gtor,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_btor=%d\n",
      color_correct.coef_btor,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_rtog=%d\n",
      color_correct.coef_rtog,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_btog=%d\n",
      color_correct.coef_btog,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_gtog=%d\n",
      color_correct.coef_gtog,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_rtob=%d\n",
      color_correct.coef_rtob,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_gtob=%d\n",
      color_correct.coef_gtob,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_btob=%d\n",
      color_correct.coef_btob,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-roffset=%d\n",
      color_correct.roffset,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-boffset=%d\n",
      color_correct.boffset,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-goffset=%d\n",
      color_correct.goffset,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "ccm-coef_qfactor=%d\n",
      color_correct.coef_qfactor,
      len, filled_len, p_meta_str);
  }

  //Fill in QDemosaic_t info in APP5 marker payload.
  if (demosaic.enable) {
    update_mbc_string(buffer, buf_size, "demosaic-aG=%d\n", demosaic.aG,
      len, filled_len, p_meta_str);
    update_mbc_string(buffer, buf_size, "demosaic-bL=%d\n", demosaic.bL,
      len, filled_len, p_meta_str);
  }

  //Fill in QGamma_t info in APP5 marker payload.
  if (gamma.enable) {
    flatten_int_arr(buffer1, buf_size, gamma.table, 64);
    update_mbc_string(buffer, buf_size, "gamma-table=%s\n", buffer1,
      len, filled_len, p_meta_str);
  }

  //Fill in QABF_t info in APP5 marker payload.
  if (abf.enable) {
    flatten_int_arr(buffer1, buf_size, abf.red.threshold, 3);
    update_mbc_string(buffer, buf_size, "abf-red-threshold=%s\n", buffer1,
      len, filled_len, p_meta_str);
    flatten_int_arr(buffer1, buf_size, abf.red.pos, 16);
    update_mbc_string(buffer, buf_size, "abf-red-pos=%s\n", buffer1,
      len, filled_len, p_meta_str);
    flatten_int_arr(buffer1, buf_size, abf.red.neg, 8);
    update_mbc_string(buffer, buf_size, "abf-red-neg=%s\n", buffer1,
      len, filled_len, p_meta_str);

    flatten_int_arr(buffer1, buf_size, abf.green.threshold, 3);
    update_mbc_string(buffer, buf_size, "abf-green-threshold=%s\n", buffer1,
      len, filled_len, p_meta_str);
    flatten_int_arr(buffer1, buf_size, abf.green.pos, 16);
    update_mbc_string(buffer, buf_size, "abf-green-pos=%s\n", buffer1,
      len, filled_len, p_meta_str);
    flatten_int_arr(buffer1, buf_size, abf.green.neg, 8);
    update_mbc_string(buffer, buf_size, "abf-green-neg=%s\n", buffer1,
      len, filled_len, p_meta_str);

    flatten_int_arr(buffer1, buf_size, abf.blue.threshold, 3);
    update_mbc_string(buffer, buf_size, "abf-blue-threshold=%s\n", buffer1,
      len, filled_len, p_meta_str);
    flatten_int_arr(buffer1, buf_size, abf.blue.pos, 16);
    update_mbc_string(buffer, buf_size, "abf-blue-pos=%s\n", buffer1,
      len, filled_len, p_meta_str);
    flatten_int_arr(buffer1, buf_size, abf.blue.neg, 8);
    update_mbc_string(buffer, buf_size, "abf-blue-neg=%s\n", buffer1,
      len, filled_len, p_meta_str);
  }

  *p_meta_str = '\0';
  filled_len++;
  CDBG_HIGH("%s:%d] filled_len %d", __func__, __LINE__, filled_len);
  p_metadata->data_len = filled_len;
  if (buffer)
    free(buffer);
  if (buffer1)
    free(buffer1);
  return 0;
}
