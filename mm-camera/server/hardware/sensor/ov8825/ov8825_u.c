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
#include "ov8825_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_OV8825 "ov8825"
#define OV8825_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV8825"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *ov8825_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  OV8825_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  OV8825_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL, /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL,	/* HFR 150 fps video recording */
  NULL, /* AR */
  OV8825_LOAD_CHROMATIX(preview), /* camera / camcorder preview for ZSL*/
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t ov8825_inputformat[] = {
    CAMIF_BAYER_B_G,/*RES0*/
    CAMIF_BAYER_B_G,/*RES0*/
};

static sensor_crop_parms_t ov8825_cropinfo[] = {
/*top, down, left, right*/
  {0, 0, 0, 0},/*FULL*/
  {0, 0, 0, 0},/*FULL*/
};

static uint32_t ov8825_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES, /* SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_INVALID_RES, /*ZSL*/
};

static struct msm_camera_csi_params ov8825_csi_params = {
  .data_format = CSI_10BIT,
  .lane_cnt    = 2,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 14,
};

static struct msm_camera_csi_params *ov8825_csi_params_array[] = {
  &ov8825_csi_params,
  &ov8825_csi_params,
};

/*===========================================================================
 * FUNCTION    - ov8820_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov8825_real_to_register_gain(float gain)
{
    uint16_t reg_gain;
    uint8_t gain_mult = 1;

    if (gain < 1.0)
      gain = 1.0;

    if (gain > 64.0)
      gain = 64.0;

    while (gain >= 2.0)
    {
        gain = gain / 2;
        gain_mult *= 2;
    }
    reg_gain = (uint16_t) ((gain - 1) * 16.0);
    gain_mult -=1;
    reg_gain = gain_mult << 4 | reg_gain;
    return reg_gain;
}/* ov8820_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - ov8820_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float ov8825_register_to_real_gain(uint16_t reg_gain)
{
    float gain;
    float gain_mult;

    if (reg_gain > 0x1FF)
      reg_gain = 0x1FF;

    gain_mult = (reg_gain >> 4) + 1;
    gain = ((float)(reg_gain & 0xF)/16 + 1) * gain_mult;
    return gain;
}/* ov8820_register_to_real_gain */

static sensor_function_table_t ov8825_func_tbl = {
    .sensor_set_op_mode = sensor_util_set_op_mode,
    .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
    .sensor_get_dim_info = sensor_util_get_dim_info,

    .sensor_set_frame_rate = sensor_util_set_frame_rate,
    .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

    .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
    .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
    .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
    .sensor_register_to_real_gain = ov8825_register_to_real_gain,
    .sensor_real_to_register_gain = ov8825_real_to_register_gain,
    .sensor_get_cur_fps = sensor_util_get_cur_fps,
    .sensor_get_lens_info = sensor_get_lens_info,
    .sensor_set_start_stream = sensor_util_set_start_stream,
    .sensor_set_stop_stream = sensor_util_set_stop_stream,
    .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
    .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
    .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
    .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

int8_t ov8825_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &ov8825_func_tbl;
  sctrl->sensor.inputformat = ov8825_inputformat;
  sctrl->sensor.crop_info = ov8825_cropinfo;
  sctrl->sensor.mode_res = ov8825_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &ov8825_csi_params_array[0];

  CDBG("ov8825_process_start\n");
  sensor_util_get_output_info(sctrl);

  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_10_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.max_gain = 8.0;
  sctrl->sensor.out_data.aec_info.max_linecount =
  sctrl->sensor.output_info[sctrl->sensor.
  mode_res[SENSOR_MODE_PREVIEW]].frame_length_lines * 24;
  sctrl->sensor.snapshot_exp_wait_frames = 1;
  sensor_util_config(sctrl);
  CDBG("ov8825_process_start xxx\n");

  return TRUE;
}
