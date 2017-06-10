/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util.h"
#include "sensor_util_bayer.h"
#include "ov5647_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_OV5647 "ov5647"
#define OV5647_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV5647"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *ov5647_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  OV5647_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  OV5647_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL, /* HD video recording */
  OV5647_LOAD_CHROMATIX(video_hfr), /*HFR 60 fps video recording*/
  OV5647_LOAD_CHROMATIX(video_hfr), /*HFR 90 fps video recording*/
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  NULL,/* AR */
  OV5647_LOAD_CHROMATIX(preview), /* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t ov5647_inputformat[] = {
	CAMIF_BAYER_B_G,/*FULL*/
	CAMIF_BAYER_B_G,/*QTR*/
	CAMIF_BAYER_B_G,/*RES2*/
	CAMIF_BAYER_B_G,/*RES3*/
	CAMIF_BAYER_B_G,/*RES4*/
};

static sensor_crop_parms_t ov5647_cropinfo[] = {
/*top, down, left, right*/
  {0, 0, 0, 0},/*FULL*/
  {0, 0, 0, 0},/*QTR*/
  {0, 0, 0, 0},/*RES2*/
  {0, 0, 0, 0},/*RES3*/
  {0, 0, 0, 0},/*RES4*/
};

static uint32_t ov5647_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_RES_2, /*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_RES_3, /*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_RES_4, /*SENSOR_MODE_ZSL*/
};

static struct msm_camera_csi_params ov5647_csi_params = {
  .data_format = CSI_8BIT,
  .lane_cnt    = 2,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 10,
};

static struct msm_camera_csi_params *ov5647_csi_params_array[] = {
  &ov5647_csi_params,/* Snapshot */
  &ov5647_csi_params,/* Preview */
  &ov5647_csi_params,/* 60fps */
  &ov5647_csi_params,/* 90fps */
  &ov5647_csi_params,/* ZSL */
};

/*===========================================================================
 * FUNCTION    - ov5647_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov5647_real_to_register_gain(float gain)
{
	uint16_t reg_gain, reg_temp;
	reg_gain = (uint16_t)gain;
	reg_temp = reg_gain<<4;
	reg_gain = reg_temp | (((uint16_t)((gain - (float)reg_gain)*16.0))&0x0f);

    return reg_gain;
}/* ov5647_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - ov5647_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float ov5647_register_to_real_gain(uint16_t reg_gain)
{
	float real_gain;
	real_gain = (float) ((float)(reg_gain>>4)+(((float)(reg_gain&0x0f))/16.0));
	return real_gain;
}/* ov5647_register_to_real_gain */

static sensor_function_table_t ov5647_func_tbl = {
  .sensor_set_op_mode = sensor_util_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_get_dim_info,

  .sensor_set_frame_rate = sensor_util_set_frame_rate,
  .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

  .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
  .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
  .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
  .sensor_register_to_real_gain = ov5647_register_to_real_gain,
  .sensor_real_to_register_gain = ov5647_real_to_register_gain,
  .sensor_get_max_supported_hfr_mode = sensor_util_get_max_supported_hfr_mode,
  .sensor_get_cur_fps = sensor_util_get_cur_fps,
  .sensor_get_lens_info = sensor_get_lens_info,
  .sensor_set_start_stream = sensor_util_set_start_stream,
  .sensor_set_stop_stream = sensor_util_set_stop_stream,
  .sensor_get_csi_params = sensor_util_get_csi_params,
  .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
  .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
  .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
  .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

int8_t ov5647_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &ov5647_func_tbl;
  sctrl->sensor.inputformat = ov5647_inputformat;
  sctrl->sensor.crop_info = ov5647_cropinfo;
  sctrl->sensor.mode_res = ov5647_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &ov5647_csi_params_array[0];

  sensor_util_get_output_info(sctrl);

  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_8_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.max_gain = 16.0;
  sctrl->sensor.out_data.aec_info.max_linecount =
    sctrl->sensor.output_info[sctrl->sensor.
    mode_res[SENSOR_MODE_PREVIEW]].frame_length_lines * 24;
  sctrl->sensor.snapshot_exp_wait_frames = 1;

  sctrl->sensor.out_data.lens_info.focal_length = 4.6;
  sctrl->sensor.out_data.lens_info.pix_size = 1.4;
  sctrl->sensor.out_data.lens_info.f_number = 2.65;
  sctrl->sensor.out_data.lens_info.total_f_dist = 1.97;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 54.8;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 42.5;

  sensor_util_config(sctrl);
  return TRUE;
}
