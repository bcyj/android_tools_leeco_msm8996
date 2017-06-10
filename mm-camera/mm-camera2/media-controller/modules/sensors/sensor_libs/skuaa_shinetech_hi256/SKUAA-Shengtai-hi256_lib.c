/* hi256_lib.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor_lib.h"
#include "cam_types.h"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 1,
  .position = 1,
  .sensor_mount_angle = 90,
};

static sensor_output_t sensor_output = {
  .output_format = SENSOR_YCBCR,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_8_BIT_DIRECT,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xe4,
  .csi_lane_mask = 0x03,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xE4,
  .csi_lane_mask = 0x1,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 1,
};
#endif

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.93,
  .pix_size = 1.4,
  .f_number = 2.8,
  .total_f_dist = 1.2,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
};

static struct msm_camera_csid_vc_cfg hi256_cid_cfg[] = {
  {0, CSI_YUV422_8, CSI_DECODE_8BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params hi256_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(hi256_cid_cfg),
      .vc_cfg = {
         &hi256_cid_cfg[0],
         &hi256_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x20,
  },
};

static struct sensor_pix_fmt_info_t hi256_pix_fmt0_fourcc[] = {
  { V4L2_MBUS_FMT_YUYV8_2X8 },
};

static struct sensor_pix_fmt_info_t hi256_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t hi256_stream_info[] = {
  {1, &hi256_cid_cfg[0], hi256_pix_fmt0_fourcc},
  {1, &hi256_cid_cfg[1], hi256_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t hi256_stream_info_array = {
  .sensor_stream_info = hi256_stream_info,
  .size = ARRAY_SIZE(hi256_stream_info),
};

static struct msm_camera_csi2_params *csi_params[] = {
  &hi256_csi_params, /* RES 0*/
  &hi256_csi_params, /* RES 1*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
  {0, 0, 0, 0}, /* RES 1 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
    /* full size */
    .x_output = 0x320*2*2,
    .y_output = 0x258*2,
    .line_length_pclk = 0x320*2*2, /* 1600 */
    .frame_length_lines = 0x258*2, /* 1200 */
    .vt_pixel_clk = 28800000,
    .op_pixel_clk = 80000000,
    .binning_factor = 1,
    .max_fps = 7.5,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    /* QTR size */
    .x_output = 0x320*2,
    .y_output = 0x258,
    .line_length_pclk = 0x320*2, /* 1600 */
    .frame_length_lines = 0x258, /* 1200 */
    .vt_pixel_clk = 28800000,
    .op_pixel_clk = 80000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t hi256_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t hi256_res_table = {
  .res_cfg_type = hi256_res_cfg,
  .size = ARRAY_SIZE(hi256_res_cfg),
};

static uint32_t sensor_supported_scene_mode =
  1 << CAM_SCENE_MODE_OFF |
  0 << CAM_SCENE_MODE_AUTO |
  1 << CAM_SCENE_MODE_LANDSCAPE |
  0 << CAM_SCENE_MODE_SNOW |
  0 << CAM_SCENE_MODE_BEACH |
  0 << CAM_SCENE_MODE_SUNSET |
  1 << CAM_SCENE_MODE_NIGHT |
  1 << CAM_SCENE_MODE_PORTRAIT |
  0 << CAM_SCENE_MODE_BACKLIGHT |
  0 << CAM_SCENE_MODE_SPORTS |
  0 << CAM_SCENE_MODE_ANTISHAKE |
  0 << CAM_SCENE_MODE_FLOWERS |
  0 << CAM_SCENE_MODE_CANDLELIGHT |
  0 << CAM_SCENE_MODE_FIREWORKS |
  0 << CAM_SCENE_MODE_PARTY |
  0 << CAM_SCENE_MODE_NIGHT_PORTRAIT |
  0 << CAM_SCENE_MODE_THEATRE |
  0 << CAM_SCENE_MODE_ACTION |
  0 << CAM_SCENE_MODE_AR |
  0 >> CAM_SCENE_MODE_FACE_PRIORITY |
  0 >> CAM_SCENE_MODE_BARCODE;

static uint32_t sensor_supported_effect_mode =
  1 << CAM_EFFECT_MODE_OFF |
  1 << CAM_EFFECT_MODE_MONO |
  1 << CAM_EFFECT_MODE_NEGATIVE |
  1 << CAM_EFFECT_MODE_SOLARIZE |
  1 << CAM_EFFECT_MODE_SEPIA |
  0 << CAM_EFFECT_MODE_POSTERIZE |
  0  << CAM_EFFECT_MODE_WHITEBOARD |
  0 << CAM_EFFECT_MODE_BLACKBOARD |
  0 << CAM_EFFECT_MODE_AQUA |
  0 << CAM_EFFECT_MODE_EMBOSS |
  0 << CAM_EFFECT_MODE_SKETCH |
  0 << CAM_EFFECT_MODE_NEON;

static sensor_lib_t sensor_lib_ptr = {
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 1,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 1,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 1,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = hi256_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(hi256_cid_cfg),
  /* resolution cfg table */
  .sensor_res_cfg_table = &hi256_res_table,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &hi256_stream_info_array,
  /* sensor supported scene mode, optional */
  .sensor_supported_scene_mode = &sensor_supported_scene_mode,
  /* sensor supported effect mode, optional */
  .sensor_supported_effect_mode = &sensor_supported_effect_mode,
};

/*===========================================================================
 * FUNCTION    - hi256_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *hi256_open_lib(void)
{
  return &sensor_lib_ptr;
}
