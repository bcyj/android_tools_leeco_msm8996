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
#include "mt9m114_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_MT9M114 "mt9m114"
#define MT9M114_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_MT9M114"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *mt9m114_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  NULL, /* camera / camcorder preview */
  NULL, /* Non HD Video recording */
  NULL, /* HD video recording */
  NULL, /* HFR 60 fps video recording */
  NULL, /* HFR 90 fps video recording */
  NULL, /* HFR 120 fps video recording */
  NULL, /* HFR 150 fps video recording */
  NULL, /* AR */
  NULL, /* ZSL */
};

static struct msm_camera_csid_vc_cfg mt9m114_cid_cfg[] = {
  {0, CSI_YUV422_8, CSI_DECODE_8BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params mt9m114_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(mt9m114_cid_cfg),
      .vc_cfg = mt9m114_cid_cfg,
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x14,
  },
};

static struct msm_camera_csi2_params *mt9m114_csi_params_array[] = {
  &mt9m114_csi_params,/*FULL*/
  &mt9m114_csi_params,/*QTR*/
};

/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t mt9m114_inputformat[] = {
    CAMIF_YCBCR_CB_Y_CR_Y,/*RES0*/
    CAMIF_YCBCR_CB_Y_CR_Y,/*RES1*/
};

static sensor_crop_parms_t mt9m114_cropinfo[] = {
/*top, down, left, right*/
  {0, 0, 0, 0},/*FULL*/
};

static uint32_t mt9m114_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_ZSL*/
};

static sensor_function_table_t mt9m114_func_tbl = {
  .sensor_set_op_mode = sensor_util_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_get_dim_info,
  .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
  .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,
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

int8_t mt9m114_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &mt9m114_func_tbl;
  sctrl->sensor.inputformat = mt9m114_inputformat;
  sctrl->sensor.crop_info = mt9m114_cropinfo;
  sctrl->sensor.mode_res = mt9m114_mode_res;
  sctrl->sensor.sensor_csi_params.csi2_params = &mt9m114_csi_params_array[0];

  sensor_util_get_output_info(sctrl);

  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_YCBCR;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_8_BIT_DIRECT;

  sctrl->sensor.out_data.lens_info.focal_length = 4.6;
  sctrl->sensor.out_data.lens_info.pix_size = 1.4;
  sctrl->sensor.out_data.lens_info.f_number = 2.65;
  sctrl->sensor.out_data.lens_info.total_f_dist = 1.97;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 54.8;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 42.5;

  sensor_util_config(sctrl);
  return TRUE;
}
