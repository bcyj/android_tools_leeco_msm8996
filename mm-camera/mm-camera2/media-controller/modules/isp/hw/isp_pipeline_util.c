/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "camera_dbg.h"
#include "q3a_stats_hw.h"
#include "isp_log.h"
#include <stats/isp_stats.h>

#ifdef ENABLE_VFE_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#if 0
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif
/*===========================================================================
 * FUNCTION    -  isp_util_calculate_ceil_log_2 -
 *
 * DESCRIPTION: calculate the y = ceil(log2(x)),
 * if x == 0, then y =0;
 * if 2^n <= x < 2^(n+1), then y= n+1;
 * .
 *==========================================================================*/
static uint32_t isp_util_calculate_ceil_log_2(uint32_t pixels_in_ae_rgn)
{
  uint32_t val = 0;
  while (pixels_in_ae_rgn) {
    val++;
    pixels_in_ae_rgn = pixels_in_ae_rgn >>1;
  }
  return val;
} /* vfe_util_calculate_ceil_log_2 */

/*===========================================================================
 * FUNCTION    - isp_util_get_awb_cct_type -
 *
 * DESCRIPTION:
 *==========================================================================*/
awb_cct_type isp_util_get_awb_cct_type(void *pipeline_ptr, cct_trigger_info* trigger,
  void *chromatix_ptr)
{
  isp_pipeline_t *pipeline = pipeline_ptr;
  return pipeline->dep.util_get_awb_cct_type(trigger, chromatix_ptr);
}

/** isp_util_get_streaming_mode:
 *    @pipeline: pipeline
 *
 * Gets current streaming mode
 *
 * Return:  CAM_STREAMING_MODE_CONTINUOUS - continuous mode
 *          CAM_STREAMING_MODE_BURST      - burst mode
 **/
cam_streaming_mode_t isp_util_get_streaming_mode(isp_pipeline_t *pipeline)
{
  int rc = 0;
  uint32_t i = 0;
  isp_hw_pix_setting_params_t *pix_params = &pipeline->pix_params.cfg_and_3a_params.cfg;

  for (i = 0; i < ISP_PIX_PATH_MAX; i ++) {
    /*check if the stream is available
      any continuous stream will set the streaming mode to be continuos*/
    if (pix_params->outputs[i].stream_param.streaming_mode ==
      CAM_STREAMING_MODE_CONTINUOUS && pix_params->outputs[i].stream_param.width > 0) {
      return CAM_STREAMING_MODE_CONTINUOUS;
    }
  }

  return CAM_STREAMING_MODE_BURST;

} /* isp_util_get_streaming_mode */

/*===========================================================================
 * FUNCTION    - isp_util_aec_check_settled -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t isp_util_aec_check_settled (aec_update_t* aec_params)
{
  /* check whether aec is settled or not */
  return (aec_params->led_state != Q3A_LED_OFF) ? 1 : aec_params->settled;

} /* isp_trigger_aec_check_settled */

/*===========================================================================
 * FUNCTION    - isp_util_awb_restore_gains -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t isp_util_awb_restore_gains (awb_update_t* awb_params)
{
  /* check whether gains need to be restored */
  return awb_params->gains_restored;

} /* isp_util_awb_restore_gains */

/*===========================================================================
 * FUNCTION    - isp_util_calc_interpolation_weight -
 *
 * DESCRIPTION:
 *==========================================================================*/
float isp_util_calc_interpolation_weight(float value,
  float start, float end)
{
  /* return value is a ratio to the start point,
    "start" point is always the smaller gain/lux one.
    thus,
    "start" could be lowlight trigger start, and bright light trigger end*/
  if (start != end) {
    if (value  <= start)
      return 0.0;
    else if (value  >= end)
      return 1.0;
    else
      return(value  - start) / (end - start);
  } else {
    ISP_DBG(ISP_MOD_COM,"Trigger Warning: same value %f\n", start);
    return 0.0;
  }
} /* isp_trigger_calc_interpolation_weight */

/*===========================================================================
 * FUNCTION    -  isp_util_calculate_shift_bits -
 *
 * DESCRIPTION: Calculate the shift bits.
 *   AE_SHIFT_BITS = ceil(log2[(ae_rgn_width*ae_rgn_height)<<8])-16
 *     = ceil(log2[ae_rgn_width*ae_rgn_height]-8);
 *  if (log2[ae_rgn_width*ae_rgn_height] ==
 *    ceil(log2[ae_rgn_width*ae_rgn_height])) AE_SHIFT_BITS++;
 *   if (AE_SHIFT_BITS < 0) AE_SHIFT_BITS = 0;
 *==========================================================================*/
uint32_t isp_util_calculate_shift_bits(uint32_t pixels_in_ae_rgn)
{
  uint32_t log2_val;
  uint32_t shift_bits;

  log2_val = isp_util_calculate_ceil_log_2(pixels_in_ae_rgn);
    ISP_DBG(ISP_MOD_COM,"%s: line %d\n", __func__, __LINE__);
  if (log2_val > 8) {
    shift_bits = log2_val - 8;
  } else {
    shift_bits = 0;
  }
  return shift_bits;
} /* isp_util_calculate_shift_bits */

/** isp_cam_fmt_to_v4l2_fmt:
 *
 * Map cam format to v4l2 format
 *
 * Return:
 **/
uint32_t isp_cam_fmt_to_v4l2_fmt(cam_format_t fmt, uint32_t uv_subsample)
{

  switch (fmt) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
    return (uv_subsample) ? V4L2_PIX_FMT_NV14 : V4L2_PIX_FMT_NV12;
  case CAM_FORMAT_YUV_420_NV21:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV21;
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV21;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR8;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG8;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG8;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB8;
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR8;
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG8;
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG8;
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB8;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR10;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG10;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG10;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB10;
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR10;
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG10;
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG10;
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB10;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR12;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG12;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG12;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB12;
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR12;
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG12;
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG12;
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB12;
  case CAM_FORMAT_YUV_420_YV12:
    return V4L2_PIX_FMT_YVU420;
  case CAM_FORMAT_YUV_422_NV16:
    return (uv_subsample) ? V4L2_PIX_FMT_NV14 : V4L2_PIX_FMT_NV16;
  case CAM_FORMAT_YUV_422_NV61:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV61;
  case CAM_FORMAT_JPEG_RAW_8BIT:
    return V4L2_PIX_FMT_JPEG;
  case CAM_FORMAT_META_RAW_8BIT:
  case CAM_FORMAT_META_RAW_10BIT:
    return V4L2_PIX_FMT_META;
  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    return V4L2_PIX_FMT_YUYV;
  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    return V4L2_PIX_FMT_YVYU;
  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    return V4L2_PIX_FMT_UYVY;
  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    return V4L2_PIX_FMT_VYUY;
  default:
    return 0;
  }
}

/*===========================================================================
 * FUNCTION    - isp_fmt_to_pix_pattern -
 *
 * DESCRIPTION:
 *==========================================================================*/
enum ISP_START_PIXEL_PATTERN isp_fmt_to_pix_pattern(cam_format_t fmt)
{
  CDBG_HIGH("%s: format %d", __func__, fmt);

  switch (fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    return ISP_BAYER_BGBGBG;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    return ISP_BAYER_GBGBGB;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    return ISP_BAYER_GRGRGR;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    return ISP_BAYER_RGRGRG;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    return ISP_BAYER_GBGBGB;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    return ISP_BAYER_GRGRGR;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    return ISP_BAYER_RGRGRG;
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    return ISP_BAYER_BGBGBG;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    return ISP_BAYER_GBGBGB;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    return ISP_BAYER_GRGRGR;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    return ISP_BAYER_RGRGRG;
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    return ISP_BAYER_BGBGBG;
  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    return ISP_YUV_YCbYCr;
  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    return ISP_YUV_YCrYCb;
  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    return ISP_YUV_CbYCrY;
  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    return ISP_YUV_CrYCbY;
  case CAM_FORMAT_YUV_422_NV16:
    return ISP_YUV_YCbYCr;
  case CAM_FORMAT_YUV_422_NV61:
    return ISP_YUV_YCrYCb;
  default:
    CDBG_ERROR("%s: invalid fmt=%d", __func__, fmt);
    return ISP_PIX_PATTERN_MAX;
  }
  return ISP_PIX_PATTERN_MAX;
}

/* ============================================================
 * function name: isp_pipeline_util_is_bayer_fmt
 * description:
 * ============================================================*/
uint8_t isp_pipeline_util_is_bayer_fmt(isp_pipeline_t *pix)
{
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;

  switch (cfg->camif_cfg.sensor_output_fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    return 1;
  default:
    return 0;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_dump_stream_dim -
 *
 * DESCRIPTION:
 *==========================================================================*/
void isp_pipeline_util_dump_stream_dim(isp_pipeline_t *pix)
{
  isp_hwif_output_cfg_t *outputs = pix->pix_params.cfg_and_3a_params.cfg.outputs;

  if (outputs[0].stream_param.num_cids > 0) {
    ISP_DBG(ISP_MOD_COM,"%s: Encoder path: width = %d, heght = %d, fmt = %d, "
         "burst = %d, session id = %d, stream id = %d\n",
         __func__,
         outputs[0].stream_param.width, outputs[0].stream_param.height,
         outputs[0].stream_param.fmt, outputs[0].stream_param.num_burst,
         outputs[0].stream_param.session_id, outputs[0].stream_param.stream_id);
  }
  if (outputs[1].stream_param.num_cids > 0) {
    ISP_DBG(ISP_MOD_COM,"%s: Viewfinder path: width = %d, heght = %d, fmt = %d, "
         "burst = %d, session id = %d, stream id = %d\n",
         __func__,
         outputs[1].stream_param.width, outputs[1].stream_param.height,
         outputs[1].stream_param.fmt, outputs[1].stream_param.num_burst,
         outputs[1].stream_param.session_id, outputs[1].stream_param.stream_id);
  }
}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_stats_buf_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_pipeline_util_stats_buf_cfg(isp_pipeline_t *pix, uint8_t is_cfg)
{
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *config = &params->cfg_and_3a_params.cfg;
  uint32_t stats_mask = params->cur_stats_mask;
  uint32_t cmd;
  int rc = 0;

  if (is_cfg)
    cmd = ISP_HW_MOD_ACTION_BUF_CFG;
  else
    cmd = ISP_HW_MOD_ACTION_BUF_UNCFG;

  if (params->cur_module_mask & (1 << ISP_MOD_STATS)) {
    rc = pix->mod_ops[ISP_MOD_STATS]->action(
      pix->mod_ops[ISP_MOD_STATS]->ctrl, cmd,
      &stats_mask, sizeof(stats_mask));
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_stats_start -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_pipeline_util_stats_start(isp_pipeline_t *pix, uint8_t start)
{
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  uint32_t stats_mask = pix->pix_params.cur_stats_mask;
  uint32_t cmd;
  int rc = 0;

  if (start)
    cmd = ISP_HW_MOD_ACTION_STREAMON;
  else
    cmd = ISP_HW_MOD_ACTION_STREAMOFF;

  if (params->cur_module_mask & (1 << ISP_MOD_STATS)) {
    ((isp_stats_mod_t *)pix->mod_ops[ISP_MOD_STATS]->ctrl)->stats_burst_len
      = pix->stats_burst_len;
    rc = pix->mod_ops[ISP_MOD_STATS]->action(
      pix->mod_ops[ISP_MOD_STATS]->ctrl, cmd,
      &stats_mask, sizeof(stats_mask));
  }
  return rc;

}

/*===========================================================================
 * FUNCTION    - isp_pix_pipeline_update_af-
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
int isp_pipeline_util_update_af(isp_pipeline_t *pix)
{
  int i, num, rc = 0;
  isp_pix_params_t *params = &pix->pix_params;
  isp_pix_trigger_update_input_t *trigger_update_params =
    &pix->pix_params.cfg_and_3a_params;
  af_config_t *af_config =
    &trigger_update_params->cfg.stats_cfg.af_config;

  rc = pix->mod_ops[ISP_MOD_STATS]->set_params(
    pix->mod_ops[ISP_MOD_STATS]->ctrl,
    ISP_HW_MOD_SET_TRIGGER_UPDATE,
    trigger_update_params,
    sizeof(isp_pix_trigger_update_input_t));

  return rc;
}


/** isp_pix_pipeline_trigger_start:
 *    @pix: pointer to pipeline
 *
 *  Perform all tasks required to start pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
int isp_pipeline_util_trigger_start(isp_pipeline_t *pix)
{
  int i, num, rc = 0;
  uint16_t *module_ids = NULL;
  isp_pix_params_t *params = &pix->pix_params;
  isp_pix_trigger_update_input_t *trigger_update_params =
    &params->cfg_and_3a_params;
  isp_stats_udpate_t *stats_update =
    &trigger_update_params->trigger_input.stats_update;

  module_ids = pix->dep.mod_trigger_update_order_bayer;
  num = pix->dep.num_mod_trigger_update_order_bayer;
  if (stats_update->awb_update.color_temp == 0) {
    /* zero color temperature. no update needed */
    CDBG_ERROR("%s: zero color temperture. No update needed\n", __func__);
    return 0;
  }

  for (i = 0; i < num; i++) {
    if (((1 << module_ids[i]) & pix->pix_params.cur_module_mask) &&
        pix->mod_ops[module_ids[i]] && (module_ids[i] != ISP_MOD_STATS)) {
      rc = pix->mod_ops[module_ids[i]]->set_params(
             pix->mod_ops[module_ids[i]]->ctrl,
             ISP_HW_MOD_SET_TRIGGER_UPDATE,
             trigger_update_params,
             sizeof(isp_pix_trigger_update_input_t));
      if (rc < 0) {
        CDBG_ERROR("%s: module %d config failed\n", __func__, i);
        return rc;
      }
    }
  }

  if (((1 << ISP_MOD_LA) & pix->pix_params.cur_module_mask) &&
    pix->mod_ops[ISP_MOD_LA]) {
    /* trigger update LA by LA ihist algo*/
    rc = pix->mod_ops[ISP_MOD_LA]->set_params(
      pix->mod_ops[ISP_MOD_LA]->ctrl, ISP_HW_MOD_SET_LA_HIST_UPDATE,
      (void *)trigger_update_params, sizeof(isp_pix_trigger_update_input_t));

    if (rc < 0) {
      CDBG_ERROR("%s: trigger update Luma Adaptation failed, rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return 0;

}
/*===========================================================================
 * FUNCTION    - isp_pix_pipeline_trigger_update -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
int isp_pipeline_util_trigger_update(isp_pipeline_t *pix)
{
  int i, num, rc = 0;
  uint16_t *module_ids = NULL;
  isp_pix_params_t *params = &pix->pix_params;
  isp_pix_trigger_update_input_t *trigger_update_params =
    &params->cfg_and_3a_params;
  isp_stats_udpate_t *stats_update =
    &trigger_update_params->trigger_input.stats_update;
   uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pix);

  if (!is_bayer_input) {
    return rc;
  }

  module_ids = pix->dep.mod_trigger_update_order_bayer;
  num = pix->dep.num_mod_trigger_update_order_bayer;
  if (stats_update->awb_update.color_temp == 0) {
    /* zero color temperature. no update needed */
    CDBG_ERROR("%s: zero color temperture. No update needed\n", __func__);
    return 0;
  }
  for (i = 0; i < num; i++) {
    if (((1 << module_ids[i]) & pix->pix_params.cur_module_mask) &&
        pix->mod_ops[module_ids[i]] && (module_ids[i] != ISP_MOD_STATS)) {
      rc = pix->mod_ops[module_ids[i]]->set_params(
             pix->mod_ops[module_ids[i]]->ctrl,
             ISP_HW_MOD_SET_TRIGGER_UPDATE,
             trigger_update_params,
             sizeof(isp_pix_trigger_update_input_t));
      if (rc < 0) {
        CDBG_ERROR("%s: module %d config failed\n", __func__, i);
        return rc;
      }
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_get_stream_path -
 *
 * DESCRIPTION:
 *==========================================================================*/
enum msm_vfe_axi_stream_src isp_pipeline_util_get_stream_path(
   isp_pipeline_t *pipeline,
   uint32_t session_id, uint32_t stream_id)
{
  int i;
  isp_hwif_output_cfg_t *outputs;
  isp_hwif_output_cfg_t *raw_output;

  outputs = pipeline->pix_params.cfg_and_3a_params.cfg.outputs;
  raw_output = &pipeline->pix_params.cfg_and_3a_params.cfg.raw_output;
   for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
     if (outputs[i].stream_param.width > 0 &&
       outputs[i].stream_param.session_id == session_id &&
       outputs[i].stream_param.stream_id == stream_id)
         return outputs[i].axi_path;
   }

    if (raw_output->stream_param.width > 0 &&
      raw_output->stream_param.session_id == session_id &&
      raw_output->stream_param.stream_id == stream_id) {
      return raw_output->axi_path;
    }

   return VFE_AXI_SRC_MAX; /* invalid path */
}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_single_HW_write -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_pipeline_util_single_HW_write(uint32_t fd, void* cmd_offset, uint32_t cmd_len,
                                      uint32_t hw_reg_offset, uint32_t reg_num, uint32_t cmd_type){

  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  cfg_cmd.cfg_data = cmd_offset;
  cfg_cmd.cmd_len = cmd_len;
  cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = cmd_type;
  reg_cfg_cmd[0].u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd[0].u.rw_info.len = reg_num * sizeof(uint32_t);

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0){
    CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
    return rc;
  }

  return rc;

}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_pack_cfgcmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
void isp_pipeline_util_pack_cfg_cmd(uint32_t fd,
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t cmd_offset, uint32_t cmd_len,
  uint32_t cmd_type, uint32_t hw_reg_offset)
{
  reg_cfg_cmd->u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd->cmd_type = cmd_type;
  reg_cfg_cmd->u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd->u.rw_info.len = cmd_len;
}

/*===========================================================================
 * FUNCTION    - isp_pipeline_util_pack_dmi_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
void isp_pipeline_util_pack_dmi_cmd(uint32_t fd,
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t hi_tbl_offset, uint32_t lo_tbl_offset,uint32_t cmd_len,
  uint32_t cmd_type)
{
  reg_cfg_cmd->u.dmi_info.hi_tbl_offset = hi_tbl_offset;
  reg_cfg_cmd->u.dmi_info.lo_tbl_offset = lo_tbl_offset;
  reg_cfg_cmd->cmd_type = cmd_type;
  reg_cfg_cmd->u.dmi_info.len = cmd_len;
}

/** isp_util_get_aec_ratio
 *
 *    get aec trigger ratio for lowlight case.
 **/
float isp_util_get_aec_ratio(void *pipeline_ptr,
                            unsigned char tunning,
                            void *trigger,
                            aec_update_t* aec_out,
                            int8_t is_snap_mode)
{
  isp_pipeline_t *pipeline = pipeline_ptr;
  return pipeline->dep.util_get_aec_ratio_lowlight(tunning,
                            trigger,
                            aec_out,
                            is_snap_mode);
}

/** isp_util_get_aec_ratio2
 *
 *
 *    get aec trigger ratio for bright light/lowlight mixing
 *    case.
 **/
int isp_util_get_aec_ratio2(void *pipeline_ptr,
                            unsigned char tuning,
                            void *outdoor_trigger,
                            void *lowlight_trigger,
                            aec_update_t* aec_out,
                            int8_t is_snap_mode,
                            trigger_ratio_t *rt)
{
  isp_pipeline_t *pipeline = pipeline_ptr;
  return pipeline->dep.util_get_aec_ratio_bright_low(tuning,
                            outdoor_trigger,
                            lowlight_trigger,
                            aec_out,
                            is_snap_mode,
                            rt);
}

/** isp_util_get_aec_ratio3
 *
 *
 *    get aec trigger ratio for bright light case.
 **/
float isp_util_get_aec_ratio3(void *pipeline_ptr,
                            unsigned char tunning,
                            void *trigger,
                            aec_update_t* aec_out,
                            int8_t is_snap_mode)
{
  isp_pipeline_t *pipeline = pipeline_ptr;
  return pipeline->dep.util_get_aec_ratio_bright(tunning,
                            trigger,
                            aec_out,
                            is_snap_mode);
}

void isp_pipeline_util_reset(isp_pipeline_t *pipeline)
{
  int i;
  int rc = 0;

  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    if (pipeline->mod_ops[i] && pipeline->mod_ops[i]->action) {
      ISP_DBG(ISP_MOD_COM,"%s: module id = %d, action = %p\n",
        __func__, i, pipeline->mod_ops[i]->action);
      rc = pipeline->mod_ops[i]->action(
        pipeline->mod_ops[i]->ctrl,
        ISP_HW_MOD_ACTION_RESET, NULL, 0);
    }
  }
}
