/*============================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "imx135_lib.h"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VDIG,
    .config_val = 0,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VANA,
    .config_val = 0,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VIO,
    .config_val = 0,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VAF,
    .config_val = 0,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_LOW,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_HIGH,
    .delay = 30,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_LOW,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_HIGH,
    .delay = 30,
  },
  {
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 24000000,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_I2C_MUX,
    .seq_val = 0,
    .config_val = 0,
    .delay = 0,
  },
};

static struct msm_camera_sensor_slave_info sensor_slave_info = {
  /* Camera slot where this camera is mounted */
  .camera_id = CAMERA_0,
  /* sensor slave address */
  .slave_addr = 0x20,
  /*sensor i2c frequency*/
  .i2c_freq_mode = I2C_FAST_MODE,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x0016,
    /* sensor id */
    .sensor_id = 0x0135,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = CAMERA_MODE_2D_B,
  .position = BACK_CAMERA_B,
  .sensor_mount_angle = SENSOR_MOUNTANGLE_360,
};

static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_10_BIT_DIRECT,
};

static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0x034C,
  .y_output = 0x034E,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0205,
  .vert_offset = 4,
};

static sensor_manual_exposure_info_t manual_exp_info = {
  .min_exposure_time = 10587,/*in nano sec = 1line*/
  .max_exposure_time = 693863262,/*in nano sec = FFFF lines*/
  .min_iso = 100,
  .max_iso = 1554,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 8.0,
  .max_linecount = 65525,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 4.6,
  .pix_size = 1.4,
  .f_number = 2.65,
  .total_f_dist = 1.97,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
  .near_end_distance = 7,/*in cm*/
   .sensing_method = SENSOR_SMETHOD_NOT_DEFINED,
   .crop_factor = 1.33, //(4:3) its sensor's physical dimension dependent factor
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xE4,
  .csi_lane_mask = 0xF,
  .csi_if = 1,
  .csid_core = { 0 },
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x1F,
  .csi_if = 1,
  .csid_core = { 0 },
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array,
    .size = ARRAY_SIZE(init_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = init_common_reg_array,
    .size = ARRAY_SIZE(init_common_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 2,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
  { 0x0100, 0x01 },
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  { 0x0100, 0x00 },
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
  { 0x0104, 0x01 },
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  { 0x0104, 0x00 },
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg imx135_cid_cfg[] = {
  { 0, CSI_RAW10, CSI_DECODE_10BIT },
  { 1, 0x35, CSI_DECODE_8BIT },
  { 2, CSI_EMBED_DATA, CSI_DECODE_8BIT}
};

static struct msm_camera_csi2_params imx135_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = ARRAY_SIZE(imx135_cid_cfg),
      .vc_cfg = {
        &imx135_cid_cfg[0],
        &imx135_cid_cfg[1],
        &imx135_cid_cfg[2],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 0x1B,
  },
  .csi_clk_scale_enable = 1,
};

static struct sensor_pix_fmt_info_t imx135_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SRGGB10 },
  { MSM_V4L2_PIX_FMT_META },
};

static struct sensor_pix_fmt_info_t imx135_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t imx135_stream_info[] = {
  { 2, &imx135_cid_cfg[0], imx135_pix_fmt0_fourcc },
  { 1, &imx135_cid_cfg[1], imx135_pix_fmt1_fourcc },
};

static sensor_stream_info_array_t imx135_stream_info_array = {
  .sensor_stream_info = imx135_stream_info,
  .size = ARRAY_SIZE(imx135_stream_info),
};

static struct msm_camera_i2c_reg_setting res_settings[] = {
#ifdef _FULL_RES_30FPS
  {
    .reg_setting = res0_2vfe_reg_array,
    .size = ARRAY_SIZE(res0_2vfe_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#else
  {
    .reg_setting = res0_1vfe_reg_array,
    .size = ARRAY_SIZE(res0_1vfe_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
  {
    .reg_setting = res7_reg_array,
    .size = ARRAY_SIZE(res7_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res2_reg_array,
    .size = ARRAY_SIZE(res2_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res3_reg_array,
    .size = ARRAY_SIZE(res3_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res4_reg_array,
    .size = ARRAY_SIZE(res4_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res5_reg_array,
    .size = ARRAY_SIZE(res5_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res6_reg_array,
    .size = ARRAY_SIZE(res6_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct msm_camera_csi2_params* csi_params[] = {
#ifdef _FULL_RES_30FPS
  &imx135_csi_params, /* RES 0 */
#else
  &imx135_csi_params, /* RES 0 */
#endif
  &imx135_csi_params, /* RES 1 */
  &imx135_csi_params, /* RES 2 */
  &imx135_csi_params, /* RES 3 */
  &imx135_csi_params, /* RES 4 */
  &imx135_csi_params, /* RES 5 */
  &imx135_csi_params, /* RES 6 */
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
#ifdef _FULL_RES_30FPS
  { 0, 0, 0, 0 }, /* RES 0 */
#else
  { 0, 0, 0, 0 }, /* RES 0 */
#endif
  { 0, 0, 0, 0 }, /* RES 1 */
  { 0, 0, 0, 0 }, /* RES 2 */
  { 0, 0, 0, 0 }, /* RES 3 */
  { 0, 0, 0, 0 }, /* RES 4 */
  { 0, 0, 0, 0 }, /* RES 5 */
  { 0, 0, 0, 0 }, /* RES 6 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
#ifdef _FULL_RES_30FPS
  {
    /* full size 4:3 @ 30 fps*/
    .x_output = 4208,
    .y_output = 3120,
    .line_length_pclk = 4572,
    .frame_length_lines = 3148,
    .vt_pixel_clk = 432000000,
    .op_pixel_clk = 432000000,
    .binning_factor = 1,
    .max_fps = 30,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#else
  {
    /* full size @ 22.27 fps*/
    .x_output = 4208,
    .y_output = 3120,
    .line_length_pclk = 4572,
    .frame_length_lines = 3142,
    .vt_pixel_clk = 320000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 22.27,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if 0
  {
    /* full size 16:9 @ 30 fps */
    .x_output = 4208,
    .y_output = 2368,
    .line_length_pclk = 4572,
    .frame_length_lines = 2390,
    .vt_pixel_clk = 320000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 29.285,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#else
  {
    /*1080P 16:9 @ 30 fps */
    .x_output = 1920,
    .y_output = 1080,
    .line_length_pclk = 4572,
    .frame_length_lines = 1312,
    .vt_pixel_clk = 180000000,
    .op_pixel_clk = 180000000,//90000000, 
    .binning_factor = 1,
    .max_fps = 30,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  #endif
  {
    /* 1/2 HV @ 30 fps */
    .x_output = 2104,
    .y_output = 1560,
    .line_length_pclk = 4572,
    .frame_length_lines = 2624,
    .vt_pixel_clk = 360000000,
    .op_pixel_clk = 180000000,
    .binning_factor = 1,
    .max_fps = 30.00,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#ifndef _MSM_BEAR
  {
     /* 1080p at 60 fps */
    .x_output = 2104,
    .y_output = 1184,
    .line_length_pclk = 4572,
    .frame_length_lines = 1312,
    .vt_pixel_clk = 360000000,
    .op_pixel_clk = 180000000,
    .binning_factor = 1,
    .max_fps = 60.00,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
#else
  {
     /* 1080p at 60 fps */
    .x_output = 2104,
    .y_output = 1184,
    .line_length_pclk = 4572,
    .frame_length_lines = 1312,
    .vt_pixel_clk = 360000000,
    .op_pixel_clk = 360000000,
    .binning_factor = 1,
    .max_fps = 60.00,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
#endif
  {
    /* 720p at 90 fps */
    .x_output = 1296,
    .y_output = 730,
    .line_length_pclk = 4572,
    .frame_length_lines = 1044,
    .vt_pixel_clk = 430000000,
    .op_pixel_clk = 216000000,
    .binning_factor = 1,
    .max_fps = 90.00,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    /* 720p at 120 fps */
    .x_output = 1296,
    .y_output = 730,
    .line_length_pclk = 4572,
    .frame_length_lines = 786,
    .vt_pixel_clk = 432000000,
    .op_pixel_clk = 216000000,
    .binning_factor = 1,
    .max_fps = 120.00,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    /* RES2 4:3 HDR movie mode */
    .x_output = 2096,
    .y_output = 1560,
    .line_length_pclk = 4572,
    .frame_length_lines = 3290,
    .vt_pixel_clk = 360000000,
    .op_pixel_clk = 180000000,
    .binning_factor = 1,
    .max_fps = 23.93,
    .min_fps = 7.5,
    .mode = SENSOR_HDR_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static struct sensor_meta_data_out_info_t sensor_meta_data_out_info[] = {
  {
    /* meta data info */
    .width  = 2096,
    .height = 1,
    .stats_type = YRGB,
  },
};

static struct sensor_lib_meta_data_info_array meta_data_out_info_array = {
  .meta_data_out_info = sensor_meta_data_out_info,
  .size = ARRAY_SIZE(sensor_meta_data_out_info),
};

static sensor_res_cfg_type_t imx135_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX,  /* set chromatix prt */
  SENSOR_SEND_EVENT,      /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t imx135_res_table = {
  .res_cfg_type = imx135_res_cfg,
  .size = ARRAY_SIZE(imx135_res_cfg),
};

static struct sensor_lib_chromatix_t imx135_chromatix[] = {
#ifdef _FULL_RES_30FPS
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES0 */

  },
#else
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
#endif
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(default_video), /* RES1 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(preview), /* RES2 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(preview), /* RES2 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(default_video), /* RES2 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES2 */
  },
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(hfr_60), /* RES3 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(hfr_60), /* RES3 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(hfr_60), /* RES3 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES3 */
  },
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(hfr_90), /* RES4 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(hfr_90), /* RES4 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(hfr_90), /* RES4 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES4 */
  },
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(hfr_120), /* RES5 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(hfr_120), /* RES5 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(hfr_120), /* RES5 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES5 */
  },
  {
    .common_chromatix = IMX135_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX135_LOAD_CHROMATIX(video_hd), /* RES6 */
    .camera_snapshot_chromatix = IMX135_LOAD_CHROMATIX(video_hd), /* RES6 */
    .camcorder_chromatix = IMX135_LOAD_CHROMATIX(video_hd), /* RES6 */
    .liveshot_chromatix = IMX135_LOAD_CHROMATIX(liveshot), /* RES6 */
  },
};

static struct sensor_lib_chromatix_array imx135_lib_chromatix_array = {
  .sensor_lib_chromatix = imx135_chromatix,
  .size = ARRAY_SIZE(imx135_chromatix),
};

/*===========================================================================
 * FUNCTION    - imx135_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t imx135_real_to_register_gain(float gain) {
  uint16_t reg_gain;
  if (gain < 1.0)
    gain = 1.0;
  if (gain > 8.0)
    gain = 8.0;
  reg_gain = (uint16_t)(256.0 - 256.0 / gain);
  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - imx135_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float imx135_register_to_real_gain(uint16_t reg_gain) {
  float gain;
  if (reg_gain > 224)
    reg_gain = 224;
  gain = 256.0 / (256.0 - reg_gain);
  return gain;
}

/*===========================================================================
 * FUNCTION    - imx135_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx135_calculate_exposure(float real_gain,
                                         uint32_t line_count, sensor_exposure_info_t* exp_info) {
  if (!exp_info) {
    return -1;
  }
  if(exp_info->use_long_exposure == 1) {
    exp_info->long_line_count = line_count;
  }
  else {
    exp_info->line_count = line_count & 0xFFFF;
  }
  exp_info->reg_gain = imx135_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = imx135_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;

  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - imx135_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx135_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting* reg_setting) {

  uint16_t shortshutter_gain = 0;
  uint32_t shortshutter = 0;
  uint16_t shortshutter_expratio = 8;
  uint16_t atr_out_noise = 0;
  uint16_t atr_out_mid = 0;
  uint16_t atr_noise_brate = 0;
  uint16_t atr_mid_brate = 0;
  uint16_t reg_count = 0;
  uint16_t i = 0;
  uint16_t luma_delta = 0;
  uint16_t luma_delta_h = 0;
  uint16_t luma_delta_l = 0;
  static uint16_t luma_delta_hys = 0;
  float longshutter_real_gain = 0.0;

  if (!reg_setting) {
    return -1;
  }

  for (i = 0; i < sensor_lib_ptr.groupon_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupon_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupon_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }

  luma_delta = fgain;
  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF00) >> 8;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF);
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info->global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_count = reg_count + 1;

  /* For video HDR mode */
  if ((luma_avg != 0) || (fgain != 0)) {
    /* Short shutter update */
    reg_setting->reg_setting[reg_count].reg_addr = SHORT_GAIN_BYTE_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = shortshutter_gain;
    reg_count = reg_count + 1;

    longshutter_real_gain = imx135_register_to_real_gain(gain);
    shortshutter = line * longshutter_real_gain / shortshutter_expratio;

    reg_setting->reg_setting[reg_count].reg_addr = SHORT_SHUTTER_WORD_ADDR;
    reg_setting->reg_setting[reg_count].reg_data =
      (shortshutter & 0xFF00) >> 8;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
      SHORT_SHUTTER_WORD_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (shortshutter & 0xFF);
    reg_count = reg_count + 1;

    /* Adaptive tone curve parameters update */
    if (luma_avg < THRESHOLD_DEAD_ZONE) {
    /* change to fixed tone curve */
      reg_setting->reg_setting[reg_count].reg_addr = TC_SWITCH_BYTE_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = 0x01;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr = DBG_SEL;
      reg_setting->reg_setting[reg_count].reg_data = 0x00;
      reg_count = reg_count + 1;
    } else {
      /* extract average of max 20 and min 20*/
      luma_delta_h = (luma_delta >> 8) & 0xFF;
      luma_delta_l = luma_delta & 0xFF;

      if (luma_avg < THRESHOLD_0) {
        atr_out_noise = 0;
        atr_out_mid = 0;
      } else if (luma_avg < THRESHOLD_1) {
        atr_out_noise = INIT_ATR_OUT_NOISE * (luma_avg - THRESHOLD_0) /
          (THRESHOLD_1 - THRESHOLD_0);
        atr_out_mid = INIT_ATR_OUT_MID * (luma_avg - THRESHOLD_0) /
          (THRESHOLD_1 - THRESHOLD_0);
      } else {
        atr_out_noise = INIT_ATR_OUT_NOISE;
        atr_out_mid = INIT_ATR_OUT_MID;
      }
      atr_out_noise += ATR_OFFSET;
      atr_out_mid += ATR_OFFSET;

      atr_noise_brate = 0x18 * 1 + ATR_OFFSET;
      atr_mid_brate = 0x18 * 1 + ATR_OFFSET;

      reg_setting->reg_setting[reg_count].reg_addr = TC_OUT_NOISE_WORD_ADDR;
      reg_setting->reg_setting[reg_count].reg_data =
        (atr_out_noise & 0xFF00) >> 8;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
        TC_OUT_NOISE_WORD_ADDR + 1;
      reg_setting->reg_setting[reg_count].reg_data =
        (atr_out_noise & 0xFF);
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr = TC_OUT_MID_WORD_ADDR;
      reg_setting->reg_setting[reg_count].reg_data =
        (atr_out_mid & 0xFF00) >> 8;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
        TC_OUT_MID_WORD_ADDR + 1;
      reg_setting->reg_setting[reg_count].reg_data =
        (atr_out_mid & 0xFF);
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr = TC_NOISE_BRATE_REGADDR;
      reg_setting->reg_setting[reg_count].reg_data = atr_noise_brate;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr = TC_MID_BRATE_REGADDR;
      reg_setting->reg_setting[reg_count].reg_data = atr_mid_brate;
      reg_count = reg_count + 1;

      /* change to adaptive tone curve */
      reg_setting->reg_setting[reg_count].reg_addr = TC_SWITCH_BYTE_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = 0x00;
      reg_count = reg_count + 1;

      /*right-shift 2 bits to get 10-bit values*/
      luma_delta_h *= 4;
      luma_delta_l *= 4;
      luma_avg *= 4;
      /* Over exposure control*/
      if(((luma_delta_h - luma_avg) < (200 + luma_delta_hys)) &&
        (luma_delta_h < 700)) {
        luma_delta_hys = 30;
        if(luma_delta_h < 959)
          luma_delta_h = 959;
        if(luma_delta_l > 60)
          luma_delta_l = 60;
        if(luma_delta_l < 8)
          luma_delta_l = 8;
        if(luma_avg <= luma_delta_l)
          luma_avg = luma_delta_l + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_AVE_H_REGADDR;
         reg_setting->reg_setting[reg_count].reg_data =
           (luma_avg & 0xFF00) >> 8;
         reg_count = reg_count + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_AVE_H_REGADDR + 1;
         reg_setting->reg_setting[reg_count].reg_data = (luma_avg & 0xFF);
         reg_count = reg_count + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_MAX_H_REGADDR;
         reg_setting->reg_setting[reg_count].reg_data =
           (luma_delta_h & 0xFF00) >> 8;
         reg_count = reg_count + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_MAX_H_REGADDR + 1;
         reg_setting->reg_setting[reg_count].reg_data =
           (luma_delta_h & 0xFF);
         reg_count = reg_count + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_MIN_H_REGADDR;
         reg_setting->reg_setting[reg_count].reg_data =
           (luma_delta_l & 0xFF00) >> 8;
         reg_count = reg_count + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_MIN_H_REGADDR + 1;
         reg_setting->reg_setting[reg_count].reg_data =
           (luma_delta_l & 0xFF);
         reg_count = reg_count + 1;

         reg_setting->reg_setting[reg_count].reg_addr = DBG_SEL;
         reg_setting->reg_setting[reg_count].reg_data = 0x01;
         reg_count = reg_count + 1;
        } else {
          luma_delta_hys = 0;
          reg_setting->reg_setting[reg_count].reg_addr = DBG_SEL;
          reg_setting->reg_setting[reg_count].reg_data = 0x00;
          reg_count = reg_count + 1;
        }
     }
  }

  for (i = 0; i < sensor_lib_ptr.groupoff_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;
  return 0;
}

static int32_t imx135_fill_awb_hdr_array(uint16_t awb_gain_r, uint16_t awb_gain_b,
  struct msm_camera_i2c_seq_reg_setting* reg_setting) {

  reg_setting->reg_setting[0].reg_addr = ABS_GAIN_R_WORD_ADDR;
  reg_setting->reg_setting[0].reg_data[0] = (awb_gain_r & 0xFF00) >> 8;
  reg_setting->reg_setting[0].reg_data[1] = (awb_gain_r & 0xFF);
  reg_setting->reg_setting[0].reg_data_size = 2;

  reg_setting->reg_setting[1].reg_addr = ABS_GAIN_B_WORD_ADDR;
  reg_setting->reg_setting[1].reg_data[0] = (awb_gain_b & 0xFF00) >> 8;
  reg_setting->reg_setting[1].reg_data[1] = (awb_gain_b & 0xFF);
  reg_setting->reg_setting[1].reg_data_size = 2;

  reg_setting->size = 2;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->delay = 0;

  return 0;
}

static sensor_exposure_table_t imx135_expsoure_tbl = {
  .sensor_calculate_exposure = imx135_calculate_exposure,
  .sensor_fill_exposure_array = imx135_fill_exposure_array,
};

static sensor_video_hdr_table_t imx135_video_hdr_tbl = {
  .sensor_fill_awb_array = imx135_fill_awb_hdr_array,
  .awb_table_size = 2,
  .video_hdr_capability = 0x100, /* (1<<8) */
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
  .actuator_name = "dw9714",
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* sensor output register address */
  .output_reg_addr = &output_reg_addr,
  /* sensor exposure gain register address */
  .exp_gain_info = &exp_gain_info,
  /* manual_exp_info */
  .manual_exp_info = &manual_exp_info,
  /* sensor aec info */
  .aec_info = &aec_info,
  /* sensor snapshot exposure wait frames info */
  .snapshot_exp_wait_frames = 1,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 2,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 1,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 1,
  /* sensor exposure table size */
  .exposure_table_size = 27,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = imx135_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(imx135_cid_cfg),
  /* init settings */
  .init_settings_array = &init_settings_array,
  /* start settings */
  .start_settings = &start_settings,
  /* stop settings */
  .stop_settings = &stop_settings,
  /* group on settings */
  .groupon_settings = &groupon_settings,
  /* group off settings */
  .groupoff_settings = &groupoff_settings,
  /* resolution cfg table */
  .sensor_res_cfg_table = &imx135_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &imx135_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &imx135_expsoure_tbl,
  /* video hdr func table */
  .video_hdr_awb_lsc_func_table = &imx135_video_hdr_tbl,
  /* chromatix array */
  .chromatix_array = &imx135_lib_chromatix_array,
  /* meta data info */
  .meta_data_out_info_array = &meta_data_out_info_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - imx135_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void* imx135_open_lib(void) {
  return &sensor_lib_ptr;
}
