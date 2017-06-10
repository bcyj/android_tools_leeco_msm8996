/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_PIX_UTIL_H__
#define __ISP_PIX_UTIL_H__
/* Function declaration */

int8_t isp_util_aec_check_settled (aec_update_t* aec_params);
int8_t isp_util_awb_restore_gains (awb_update_t* awb_params);
float isp_util_get_aec_ratio(void *pipeline_ptr, unsigned char tunning,
  void *trigger, aec_update_t* aec_out, int8_t is_snap_mode);
int isp_util_get_aec_ratio2(void *pipeline_ptr, unsigned char tuning,
  void *outdoor_trigger, void *lowlight_trigger,
  aec_update_t* aec_out, int8_t is_snap_mode, trigger_ratio_t *rt);
float isp_util_get_aec_ratio3(void *pipeline_ptr, unsigned char tunning,
  void *trigger, aec_update_t* aec_out, int8_t is_snap_mode);
uint32_t isp_util_calculate_shift_bits(uint32_t pixels_in_ae_rgn);
float isp_util_calc_interpolation_weight(float value,
  float start, float end);
awb_cct_type isp_util_get_awb_cct_type(void *pipeline_ptr, cct_trigger_info* trigger,
  void *chromatix_ptr);
static uint32_t isp_util_calculate_ceil_log_2(uint32_t pixels_in_ae_rgn);
enum ISP_START_PIXEL_PATTERN isp_fmt_to_pix_pattern(cam_format_t fmt);
uint32_t isp_cam_fmt_to_v4l2_fmt(cam_format_t fmt, uint32_t uv_subsample);

uint8_t isp_pipeline_util_is_bayer_fmt(isp_pipeline_t *pipeline);
void isp_pipeline_util_dump_camif_dim(isp_pipeline_t *pipeline);
void isp_pipeline_util_dump_stream_dim(isp_pipeline_t *pipeline);
int isp_pipeline_util_stats_buf_cfg(isp_pipeline_t *pipeline, uint8_t is_cfg);
int isp_pipeline_util_stats_start(isp_pipeline_t *pipeline, uint8_t start);
int isp_pipeline_util_trigger_update(isp_pipeline_t *pipeline);
int isp_pipeline_util_trigger_start(isp_pipeline_t *pipeline);
enum msm_vfe_axi_stream_src isp_pipeline_util_get_stream_path(
   isp_pipeline_t *pipeline,
   uint32_t session_id, uint32_t stream_id);
int isp_pipeline_util_update_af(isp_pipeline_t *pix);
int isp_pipeline_util_single_HW_write(uint32_t fd, void* cmd_offset,
  uint32_t cmd_len, uint32_t hw_reg_offset, uint32_t reg_num,
  uint32_t cmd_type);
void isp_pipeline_util_pack_cfg_cmd(uint32_t fd,
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t cmd_offset, uint32_t cmd_len,
  uint32_t cmd_type, uint32_t hw_reg_offset);
void isp_pipeline_util_pack_dmi_cmd(uint32_t fd,
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t hi_tbl_offset, uint32_t lo_tbl_offset,uint32_t cmd_len,
  uint32_t cmd_type);
enum ISP_START_PIXEL_PATTERN isp_fmt_to_pix_pattern(cam_format_t fmt);

/** isp_util_get_streaming_mode:
 *    @pipeline: pipeline
 *
 * Gets current streaming mode
 *
 * Return:  CAM_STREAMING_MODE_CONTINUOUS - continuous mode
 *          CAM_STREAMING_MODE_BURST      - burst mode
 **/
cam_streaming_mode_t isp_util_get_streaming_mode(isp_pipeline_t *pipeline);
void isp_pipeline_util_reset(isp_pipeline_t *pix);

#endif /* __ISP_PIX_UTIL_H__ */

