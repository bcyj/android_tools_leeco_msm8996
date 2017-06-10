/*============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include <stdio.h>
#include "sensor_lib.h"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 1,
  .position = 1,
  .sensor_mount_angle = 270,
};

static sensor_output_t sensor_output = {
  .output_format = SENSOR_YCBCR,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_8_BIT_DIRECT,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 4.6,
  .pix_size = 1.4,
  .f_number = 2.65,
  .total_f_dist = 1.97,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
};


#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x0004,
  .csi_lane_mask = 0x18,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x3,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_csid_vc_cfg gc0310_cid_cfg[] = {
  {0, CSI_YUV422_8, CSI_DECODE_8BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params gc0310_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(gc0310_cid_cfg),
      .vc_cfg = {
         &gc0310_cid_cfg[0],
         &gc0310_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 29,
#ifndef VFE_40
    .combo_mode = 1,
#endif
 },
};

static struct sensor_pix_fmt_info_t gc0310_pix_fmt0_fourcc[] = {
  { V4L2_MBUS_FMT_YUYV8_2X8 },
};

static struct sensor_pix_fmt_info_t gc0310_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t gc0310_stream_info[] = {
  {1, &gc0310_cid_cfg[0], gc0310_pix_fmt0_fourcc},
  {1, &gc0310_cid_cfg[1], gc0310_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t gc0310_stream_info_array = {
  .sensor_stream_info = gc0310_stream_info,
  .size = ARRAY_SIZE(gc0310_stream_info),
};

static struct msm_camera_csi2_params *csi_params[] = {
  &gc0310_csi_params, /* RES 0*/
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
    .x_output = 640*2,
    .y_output = 480,
    .line_length_pclk = 648,
    .frame_length_lines = 488,
    .vt_pixel_clk = 12000000,
    .op_pixel_clk = 12000000,
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

static sensor_res_cfg_type_t gc0310_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t gc0310_res_table = {
  .res_cfg_type = gc0310_res_cfg,
  .size = ARRAY_SIZE(gc0310_res_cfg),
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
  .sensor_max_pipeline_frame_delay = 0,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = gc0310_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(gc0310_cid_cfg),
  /* resolution cfg table */
  .sensor_res_cfg_table = &gc0310_res_table,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &gc0310_stream_info_array,
};

/*===========================================================================
 * FUNCTION    - gc0310_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *gc0310_open_lib(void)
{
  return &sensor_lib_ptr;
}
