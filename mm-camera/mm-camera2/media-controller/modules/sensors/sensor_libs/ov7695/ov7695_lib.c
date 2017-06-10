/* ov7695_lib.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor_lib.h"

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

static sensor_lens_info_t default_lens_info = {
  .focal_length = 1.52,
  .pix_size = 0.3,
  .f_number = 2.8,
  .total_f_dist = 1.97,
  .hor_view_angle = 60,
  .ver_view_angle = 60,
};

static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x3,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 1,
};
//#endif

static struct msm_camera_csid_vc_cfg ov7695_cid_cfg[] = {
  {0, CSI_YUV422_8, CSI_DECODE_8BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov7695_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov7695_cid_cfg),
      .vc_cfg = {
         &ov7695_cid_cfg[0],
         &ov7695_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x1b,
  },
};

static struct sensor_pix_fmt_info_t ov7695_pix_fmt0_fourcc[] = {
  { V4L2_MBUS_FMT_YUYV8_2X8 },
};

static struct sensor_pix_fmt_info_t ov7695_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov7695_stream_info[] = {
  {1, &ov7695_cid_cfg[0], ov7695_pix_fmt0_fourcc},
  {1, &ov7695_cid_cfg[1], ov7695_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov7695_stream_info_array = {
  .sensor_stream_info = ov7695_stream_info,
  .size = ARRAY_SIZE(ov7695_stream_info),
};

static struct msm_camera_csi2_params *csi_params[] = {
  &ov7695_csi_params, /* RES 0*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
    .x_output = 1280,
    .y_output = 480,
    .line_length_pclk = 746*2,
    .frame_length_lines = 536,
    .vt_pixel_clk = 24000000,
    .op_pixel_clk = 24000000,
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

static sensor_res_cfg_type_t ov7695_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov7695_res_table = {
  .res_cfg_type = ov7695_res_cfg,
  .size = ARRAY_SIZE(ov7695_res_cfg),
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 3,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 3,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 1,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov7695_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov7695_cid_cfg),
  /* resolution cfg table */
  .sensor_res_cfg_table = &ov7695_res_table,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov7695_stream_info_array,
};

/*===========================================================================
 * FUNCTION    - ov7695_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov7695_open_lib(void)
{
  return &sensor_lib_ptr;
}
