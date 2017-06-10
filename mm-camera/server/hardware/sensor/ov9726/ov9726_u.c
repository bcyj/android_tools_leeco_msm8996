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
#include "ov9726_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_OV9726 "ov9726"
#define OV9726_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV9726"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *ov9726_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  OV9726_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  OV9726_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL, /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  NULL,/* AR */
  NULL,/* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t ov9726_inputformat[] = {
    CAMIF_BAYER_G_B,/*RES0*/
    CAMIF_BAYER_G_B,/*RES1*/
    CAMIF_BAYER_G_B,/*RES2*/
};

static sensor_crop_parms_t ov9726_cropinfo[] = {
  {0, 0, 0, 0},/*RES0*/
  {0, 0, 0, 0},/*RES1*/
  {0, 0, 0, 0},/*RES2*/
};

static uint32_t ov9726_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_ZSL*/
};

static struct msm_camera_csi_params ov9726_csi_params = {
  .data_format = CSI_10BIT,
  .lane_cnt    = 1,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 7,
};

static struct msm_camera_csi_params *ov9726_csi_params_array[] = {
  &ov9726_csi_params,
};

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov9726_real_to_register_gain(float gain)
{
  uint16_t reg_gain, multiply_factor;
  uint16_t reg_gain_first=0;
  uint16_t reg_gain_last = 0;

  if (gain < 1) {
    gain =1;
    reg_gain = 0x00;
  } else if (gain >= 32) {
    gain =32;
    reg_gain = 0xFF;
  } else {
    multiply_factor = 1;
    while (gain >= 2) {
      reg_gain_first += multiply_factor;
      multiply_factor *=2;
      gain /= 2.0;
    }

    reg_gain_last = (uint16_t)((gain-1.0)*16.0);

    if (reg_gain_last == 16) {
      if(multiply_factor < 16)
        reg_gain_first += multiply_factor;
      else
        reg_gain_last = 0xf;
    }

    reg_gain = (reg_gain_first << 4) | reg_gain_last;
  }

  return reg_gain;

} /* ov9726_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov9726_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;
  real_gain = (float)((((float)(reg_gain >> 4) + 1.0) * \
    (16.0 + (float)(reg_gain % 16))) / 16.0);

  return real_gain;

} /* ov9726_register_to_real_gain */

static sensor_function_table_t ov9726_func_tbl = {
  .sensor_set_op_mode = sensor_util_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_get_dim_info,

  .sensor_set_frame_rate = sensor_util_set_frame_rate,
  .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

  .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
  .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
  .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
  .sensor_register_to_real_gain = ov9726_register_to_real_gain,
  .sensor_real_to_register_gain = ov9726_real_to_register_gain,
  .sensor_get_lens_info = sensor_get_lens_info,
  .sensor_get_max_supported_hfr_mode = sensor_util_get_max_supported_hfr_mode,
  .sensor_set_start_stream = sensor_util_set_start_stream,
  .sensor_set_stop_stream = sensor_util_set_stop_stream,
  .sensor_get_csi_params = sensor_util_get_csi_params,
  .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
  .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
  .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
  .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

int8_t ov9726_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &ov9726_func_tbl;
  sctrl->sensor.inputformat = ov9726_inputformat;
  sctrl->sensor.crop_info = ov9726_cropinfo;
  sctrl->sensor.mode_res = ov9726_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &ov9726_csi_params_array[0];

  sensor_util_get_output_info(sctrl);

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI_1;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_10_BIT_DIRECT;

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
