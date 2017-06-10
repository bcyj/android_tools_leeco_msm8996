/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util.h"
#include "imx135_u.h"
#include "sensor_util_bayer.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_IMX135 "imx135"
#define IMX135_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_IMX135"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *imx135_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  IMX135_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  IMX135_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  IMX135_LOAD_CHROMATIX(video_hd), /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  NULL, /* AR */
  IMX135_LOAD_CHROMATIX(zsl), /* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t imx135_inputformat[] = {
    CAMIF_BAYER_R_G,/*RES0*/
    CAMIF_BAYER_R_G,/*RES1*/
    CAMIF_BAYER_R_G,/*RES2*/
};

static sensor_crop_parms_t imx135_cropinfo[] = {
/*top, down, left, right*/
  {0, 0, 0, 0},/*FULL*/
  {0, 0, 0, 0},/*QTR*/
  {0, 0, 0, 0},/*RES2*/
};

static uint32_t imx135_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_ZSL*/
};

static struct msm_camera_csid_vc_cfg imx135_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx135_csi_params = {
	.csid_params = {
		.lane_cnt = 4,
		.lut_params = {
			.num_cid = ARRAY_SIZE(imx135_cid_cfg),
			.vc_cfg = imx135_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 4,
		.settle_cnt = 0x1B,
	},
};

static struct msm_camera_csi2_params *imx135_csi_params_array[] = {
	&imx135_csi_params,
	&imx135_csi_params,
	&imx135_csi_params,
};

/*===========================================================================
 * FUNCTION    - imx135_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t imx135_real_to_register_gain(float gain)
{
    uint16_t reg_gain;

    if (gain < 1.0)
      gain = 1.0;

    if (gain > 8.0)
      gain = 8.0;

    reg_gain = (uint16_t)(256.0 - 205.0 / gain);

    return reg_gain;
}/* imx135_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - imx135_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float imx135_register_to_real_gain(uint16_t reg_gain)
{
    float gain;
	//reg value correspoding to 8x gain
    if (reg_gain > 231)
      reg_gain = 231;

    gain = 205.0 /(256.0 - reg_gain);

    return gain;
}/* imx135_register_to_real_gain */

static sensor_function_table_t imx135_func_tbl = {
  .sensor_set_op_mode = sensor_util_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_get_dim_info,
  .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
  .sensor_set_frame_rate = sensor_util_set_frame_rate,
  .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

  .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
  .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
  .sensor_register_to_real_gain = imx135_register_to_real_gain,
  .sensor_real_to_register_gain = imx135_real_to_register_gain,
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

static struct sensor_calib_data imx135_calib_data;

int8_t imx135_process_start(void *ctrl)
{
  CDBG("imx135_process_start");
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &imx135_func_tbl;
  sctrl->sensor.inputformat = imx135_inputformat;
  sctrl->sensor.crop_info = imx135_cropinfo;
  sctrl->sensor.mode_res = imx135_mode_res;
  sctrl->sensor.sensor_csi_params.csi2_params = &imx135_csi_params_array[0];

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

  sctrl->sensor.out_data.lens_info.focal_length = 4.6;
  sctrl->sensor.out_data.lens_info.pix_size = 1.4;
  sctrl->sensor.out_data.lens_info.f_number = 2.65;
  sctrl->sensor.out_data.lens_info.total_f_dist = 1.97;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 54.8;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 42.5;

  sensor_util_config(sctrl);
  return TRUE;
}
