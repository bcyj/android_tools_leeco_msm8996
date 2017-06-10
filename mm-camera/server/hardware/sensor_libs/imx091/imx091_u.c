/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util_bayer.h"
#include "sensor_util_bayer.h"
#include "imx091_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_IMX091 "imx091"
#define IMX091_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_IMX091"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *imx091_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  IMX091_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  IMX091_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  IMX091_LOAD_CHROMATIX(video_hd), /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  NULL, /* AR */
  IMX091_LOAD_CHROMATIX(zsl), /* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t imx091_inputformat[] = {
  CAMIF_BAYER_R_G,/*RES0*/
  CAMIF_BAYER_R_G,/*RES1*/
  CAMIF_BAYER_R_G,/*RES2*/
};

static sensor_crop_parms_t imx091_cropinfo[] = {
  {0, 0, 0, 0},/*RES0*/
  {0, 0, 0, 0},/*RES1*/
  {0, 0, 0, 0},/*RES2*/
};

static uint32_t imx091_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_RES_FULL, /*SENSOR_MODE_ZSL*/
};

static struct msm_camera_csid_vc_cfg imx091_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
  {2, CSI_RESERVED_DATA_0, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx091_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
       .num_cid = ARRAY_SIZE(imx091_cid_cfg),
       .vc_cfg = imx091_cid_cfg,
    },
  },
  .csiphy_params = {
     .lane_cnt = 4,
     .settle_cnt = 0x12,
  },
};

static struct msm_camera_csi2_params *imx091_csi_params_array[] = {
  &imx091_csi_params,
  &imx091_csi_params,
};

static struct msm_sensor_output_reg_addr_t imx091_reg_addr = {
  .x_output = 0x034C,
  .y_output = 0x034E,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t imx091_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if (gain < 1.0)
  gain = 1.0;

  if (gain > 8.0)
  gain = 8.0;

  reg_gain = (uint16_t)(256.0 - 256.0 / gain);

  return reg_gain;
}/* imx091_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float imx091_register_to_real_gain(uint16_t reg_gain)
{
  float gain;

  if (reg_gain > 0x00E0)
    reg_gain = 0x00E0;

  gain = 256.0 /(256.0 - reg_gain);

  return gain;
} /* imx091_register_to_real_gain */

static sensor_function_table_t imx091_func_tbl = {
  .sensor_set_op_mode = sensor_util_bayer_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_bayer_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_bayer_get_dim_info,
  .sensor_get_preview_fps_range = sensor_util_bayer_get_preview_fps_range,
  .sensor_set_frame_rate = sensor_util_bayer_set_frame_rate,
  .sensor_set_exposure_gain = sensor_util_bayer_set_exposure_gain,
  .sensor_set_snapshot_exposure_gain =
    sensor_util_bayer_set_snapshot_exposure_gain,
  .sensor_write_exp_gain = sensor_util_bayer_write_exp_gain,
  .sensor_register_to_real_gain = imx091_register_to_real_gain,
  .sensor_real_to_register_gain = imx091_real_to_register_gain,
  .sensor_get_max_supported_hfr_mode =
    sensor_util_bayer_get_max_supported_hfr_mode,
  .sensor_get_cur_fps = sensor_util_bayer_get_cur_fps,
  .sensor_get_lens_info = sensor_get_bayer_lens_info,
  .sensor_set_start_stream = sensor_util_bayer_set_start_stream,
  .sensor_set_stop_stream = sensor_util_bayer_set_stop_stream,
  .sensor_get_csi_params = sensor_util_bayer_get_csi_params,
  .sensor_set_config_setting = sensor_util_bayer_config_setting,
  .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
  .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
  .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
  .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t imx091_process_start(void *ctrl)
{
  int32_t rc = 0;
  int index = 0;
  LOGE("%s:%d library process_start\n", __func__, __LINE__);
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &imx091_func_tbl;
  sctrl->sensor.inputformat = imx091_inputformat;
  sctrl->sensor.crop_info = imx091_cropinfo;
  sctrl->sensor.mode_res = imx091_mode_res;
  sctrl->sensor.sensor_csi_params.csi2_params = &imx091_csi_params_array[0];
  sctrl->sensor.sensor_load_chromatixfile = &imx091_load_chromatix[0];

  load_driver_params(sctrl);

  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_10_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.max_gain = 8.0;
  sctrl->sensor.out_data.aec_info.max_linecount =
  sctrl->driver_params->output_info[sctrl->sensor.
    mode_res[SENSOR_MODE_PREVIEW]].frame_length_lines * 24;
  sctrl->sensor.snapshot_exp_wait_frames = 1;

  sctrl->sensor.out_data.lens_info.focal_length = 3.79;
  sctrl->sensor.out_data.lens_info.pix_size = 1.12;
  sctrl->sensor.out_data.lens_info.f_number = 2.2;
  sctrl->sensor.out_data.lens_info.total_f_dist = 2.91;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 64.1;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 50.1;

  rc = sensor_util_bayer_power_up(sctrl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }

  rc = sensor_util_bayer_get_output_csi_info(sctrl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }

  rc = sensor_util_bayer_config(sctrl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
  }
  return rc;
}
