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
#include "s5k4e1_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_S5K4E1 "s5k4e1"
#define S5K4E1_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_S5K4E1"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *s5k4e1_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  S5K4E1_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  S5K4E1_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL, /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  NULL,/* AR */
  S5K4E1_LOAD_CHROMATIX(preview),/* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t s5k4e1_inputformat[] = {
    CAMIF_BAYER_G_R,/*RES0*/
    CAMIF_BAYER_G_R,/*RES1*/
};

static sensor_crop_parms_t s5k4e1_cropinfo[] = {
/*top, down, left, right*/
#if 0
  {24, 24, 56, 56},/*FULL*/
  {12, 12, 28, 28},/*QTR*/
#endif
  {0, 0, 0, 0},/*FULL*/
  {0, 0, 0, 0},/*QTR*/

};

static uint32_t s5k4e1_mode_res[SENSOR_MODE_INVALID] = {
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

static struct msm_camera_csi_params s5k4e1_csi_params = {
  .data_format = CSI_10BIT,
  .lane_cnt    = 1,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 24,
};

static struct msm_camera_csi_params *s5k4e1_csi_params_array[] = {
  &s5k4e1_csi_params,/*FULL*/
  &s5k4e1_csi_params,/*QTR*/
};

/*===========================================================================
 * FUNCTION    - s5k4e1_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t s5k4e1_real_to_register_gain(float gain)
{
    uint16_t reg_gain;

    if (gain < 1.0)
      gain = 1.0;

    if (gain > 16.0)
      gain = 16.0;

    reg_gain = (uint16_t)(gain * 32.0f);

    return reg_gain;
}/* s5k4e1_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - s5k4e1_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float s5k4e1_register_to_real_gain(uint16_t reg_gain)
{
    float gain;

    if (reg_gain > 0x0200)
      reg_gain = 0x0200;
   gain = (float)(reg_gain/32.0f);

    return gain;
}/* s5k4e1_register_to_real_gain */

static sensor_function_table_t s5k4e1_func_tbl = {
  .sensor_set_op_mode = sensor_util_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_get_dim_info,

  .sensor_set_frame_rate = sensor_util_set_frame_rate,
  .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

  .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
  .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
  .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
  .sensor_register_to_real_gain = s5k4e1_register_to_real_gain,
  .sensor_real_to_register_gain = s5k4e1_real_to_register_gain,
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

int8_t s5k4e1_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &s5k4e1_func_tbl;
  sctrl->sensor.inputformat = s5k4e1_inputformat;
  sctrl->sensor.crop_info = s5k4e1_cropinfo;
  sctrl->sensor.mode_res = s5k4e1_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &s5k4e1_csi_params_array[0];

  sensor_util_get_output_info(sctrl);

  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_10_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.max_gain = 16.0;
  sctrl->sensor.out_data.aec_info.max_linecount =
    sctrl->sensor.output_info[sctrl->sensor.
    mode_res[SENSOR_MODE_PREVIEW]].frame_length_lines * 24;
  sctrl->sensor.snapshot_exp_wait_frames = 1;

  sctrl->sensor.out_data.lens_info.focal_length = 3.49;
  sctrl->sensor.out_data.lens_info.pix_size = 1.4;
  sctrl->sensor.out_data.lens_info.f_number = 2.2;
  sctrl->sensor.out_data.lens_info.total_f_dist = 1.97;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 54.8;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 42.5;

  sensor_util_config(sctrl);
  return TRUE;
}
