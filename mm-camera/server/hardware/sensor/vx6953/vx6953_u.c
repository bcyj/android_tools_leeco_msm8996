/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util.h"
#include "sensor_util_bayer.h"
#include "vx6953_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_VX6953 "vx6953"
#define VX6953_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_VX6953"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *vx6953_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  VX6953_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  VX6953_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL, /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  VX6953_LOAD_CHROMATIX(ar), /* AR */
  NULL,/* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t vx6953_inputformat[] = {
    CAMIF_BAYER_G_R,/*RES0*/
	CAMIF_BAYER_G_R,/*RES0*/
};

static sensor_crop_parms_t vx6953_cropinfo[] = {
/*top, down, left, right*/
  {0, 0, 0, 0},/*FULL*/
  {0, 0, 0, 0},/*FULL*/
};

static uint32_t vx6953_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_ZSL*/
};

static struct msm_camera_csi_params vx6953_csi_params = {
  .data_format = CSI_8BIT,
  .lane_cnt    = 1,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 7,
};

static struct msm_camera_csi_params *vx6953_csi_params_array[] = {
  &vx6953_csi_params,
  &vx6953_csi_params,
};

/*===========================================================================
 * FUNCTION    - vx6953_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t vx6953_real_to_register_gain(float gain)
{
	uint16_t reg_anlg_gain, reg_digital_gain;
	float real_anlg_gain;
	float real_digital_gain;

	reg_anlg_gain = (uint16_t)(256.0 - 256.0 / gain) & 0xF0;
	real_anlg_gain = 256.0 / (256.0 - (float)reg_anlg_gain);
	real_digital_gain = gain / real_anlg_gain;
	reg_digital_gain = (uint16_t)((real_digital_gain - 1) * 32.0) * 8;

	return ((reg_digital_gain << 8) | reg_anlg_gain);
}/* vx6953_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - vx6953_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float vx6953_register_to_real_gain(uint16_t reg_gain)
{
	float real_gain;
	real_gain = (256.0 / (256.0 - (float)(reg_gain & 0xFF)));
	real_gain = (float)(1.0 + (reg_gain >> 8) / (32.0 * 8.0)) * real_gain;

	return real_gain;
}/* vx6953_register_to_real_gain */

static sensor_function_table_t vx6953_func_tbl = {
    .sensor_set_op_mode = sensor_util_set_op_mode,
    .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
    .sensor_get_dim_info = sensor_util_get_dim_info,

    .sensor_set_frame_rate = sensor_util_set_frame_rate,
    .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

    .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
    .sensor_register_to_real_gain = vx6953_register_to_real_gain,
    .sensor_real_to_register_gain = vx6953_real_to_register_gain,
    .sensor_get_max_supported_hfr_mode = sensor_util_get_max_supported_hfr_mode,
    .sensor_get_lens_info = sensor_get_lens_info,
    .sensor_set_start_stream = sensor_util_set_start_stream,
    .sensor_set_stop_stream = sensor_util_set_stop_stream,
    .sensor_get_csi_params = sensor_util_get_csi_params,
    .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
    .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
    .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
    .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

int8_t vx6953_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &vx6953_func_tbl;
  sctrl->sensor.inputformat = vx6953_inputformat;
  sctrl->sensor.crop_info = vx6953_cropinfo;
  sctrl->sensor.mode_res = vx6953_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &vx6953_csi_params_array[0];

  sensor_util_get_output_info(sctrl);
  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_10_BIT_DIRECT;

#if 0
  sctrl->sensor.output_info[0].x_output = 2608;
  sctrl->sensor.output_info[0].y_output = 1960;
  sctrl->sensor.output_info[0].line_length_pclk = 0xb8c;
  sctrl->sensor.output_info[0].frame_length_lines = 0x7d0;
  sctrl->sensor.output_info[0].vt_pixel_clk = 88666666;
  sctrl->sensor.output_info[0].op_pixel_clk = 192000000;
  sctrl->sensor.output_info[0].binning_factor = 1;

  sctrl->sensor.output_info[1].x_output = 1304;
  sctrl->sensor.output_info[1].y_output = 980;
  sctrl->sensor.output_info[1].line_length_pclk = 0xb74;
  sctrl->sensor.output_info[1].frame_length_lines = 0x3f0;
  sctrl->sensor.output_info[1].vt_pixel_clk = 88666666;
  sctrl->sensor.output_info[1].op_pixel_clk = 192000000;
  sctrl->sensor.output_info[1].binning_factor = 1;
#endif

  sctrl->sensor.out_data.aec_info.max_gain = 8.0;
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
