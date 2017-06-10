/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_SKUF_OV5648_P5V23C "skuf_ov5648_p5v23c"
#define SKUF_OV5648_P5V23C_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_SKUF_OV5648_P5V23C"_"#n".so"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VIO,
    .config_val = 0,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VDIG,
    .config_val = GPIO_OUT_LOW,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VDIG,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VANA,
    .config_val = 0,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_LOW,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_HIGH,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_LOW,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_HIGH,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 24000000,
    .delay = 10,
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
  .camera_id = CAMERA_1,
  /* sensor slave address */
  .slave_addr = 0x6a,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x300a,
    /* sensor id */
    .sensor_id = 0x5648,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_NOT_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 0,
  .position = 1,
  .sensor_mount_angle = 270,
};
static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_10_BIT_DIRECT,
};

static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0x3808,
  .y_output = 0x380a,
  .line_length_pclk = 0x380c,
  .frame_length_lines = 0x380e,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x3500,
  .global_gain_addr = 0x350a,
  .vert_offset = 4,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 8.0,
  .max_linecount = 29760,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.99,
  .pix_size = 1.4,
  .f_number = 2.4,
  .total_f_dist = 1.97,
  .hor_view_angle = 74.3,
  .ver_view_angle = 74.3,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xE4,
  .csi_lane_mask = 0x3,
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
  .csi_phy_sel = 1,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103, 0x01},
};

static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x3001, 0x00},
  {0x3002, 0x00},
  {0x3011, 0x02},
  {0x3017, 0x05},
  {0x3018, 0x2c},
  {0x301c, 0xd2},
  {0x3022, 0x00},
  {0x3034, 0x1a},
  {0x3035, 0x21},
  {0x3036, 0x69},
  {0x3037, 0x13},
  {0x3038, 0x00},
  {0x3039, 0x00},
  {0x303a, 0x00},
  {0x303b, 0x19},
  {0x303c, 0x11},
  {0x303d, 0x30},
  {0x3105, 0x11},
  {0x3106, 0x05},
  {0x3304, 0x28},
  {0x3305, 0x41},
  {0x3306, 0x30},
  {0x3308, 0x00},
  {0x3309, 0xc8},
  {0x330a, 0x01},
  {0x330b, 0x90},
  {0x330c, 0x02},
  {0x330d, 0x58},
  {0x330e, 0x03},
  {0x330f, 0x20},
  {0x3300, 0x00},
  {0x3500, 0x00},
  {0x3501, 0x3d},
  {0x3502, 0x00},
  {0x3503, 0x07},
  {0x350a, 0x00},
  {0x350b, 0x40},
  {0x3601, 0x33},
  {0x3602, 0x00},
  {0x3611, 0x0e},
  {0x3612, 0x2b},
  {0x3614, 0x50},
  {0x3620, 0x33},
  {0x3622, 0x00},
  {0x3630, 0xad},
  {0x3631, 0x00},
  {0x3632, 0x94},
  {0x3633, 0x17},
  {0x3634, 0x14},
  {0x3704, 0xc0},
  {0x3705, 0x2a},
  {0x3708, 0x66},
  {0x3709, 0x52},
  {0x370b, 0x23},
  {0x370c, 0xc3},
  {0x370d, 0x00},
  {0x370e, 0x00},
  {0x371c, 0x07},
  {0x3739, 0xd2},
  {0x373c, 0x00},
  {0x3800, 0x00},
  {0x3801, 0x00},
  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3804, 0x0a},
  {0x3805, 0x3f},
  {0x3806, 0x07},
  {0x3807, 0xa3},
  {0x3808, 0x05},
  {0x3809, 0x10},
  {0x380a, 0x03},
  {0x380b, 0xcc},
  {0x380c, 0x05},
  {0x380d, 0x80},
  {0x380e, 0x03},
  {0x380f, 0xe0},
  {0x3810, 0x00},
  {0x3811, 0x08},
  {0x3812, 0x00},
  {0x3813, 0x04},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3817, 0x00},
  {0x3820, 0x08},
  {0x3821, 0x07},
  {0x3826, 0x03},
  {0x3829, 0x00},
  {0x382b, 0x0b},
  {0x3830, 0x00},
  {0x3836, 0x00},
  {0x3837, 0x00},
  {0x3838, 0x00},
  {0x3839, 0x04},
  {0x383a, 0x00},
  {0x383b, 0x01},
  {0x3b00, 0x00},
  {0x3b02, 0x08},
  {0x3b03, 0x00},
  {0x3b04, 0x04},
  {0x3b05, 0x00},
  {0x3b06, 0x04},
  {0x3b07, 0x08},
  {0x3b08, 0x00},
  {0x3b09, 0x02},
  {0x3b0a, 0x04},
  {0x3b0b, 0x00},
  {0x3b0c, 0x3d},
  {0x3f01, 0x0d},
  {0x3f0f, 0xf5},
  {0x4000, 0x89},
  {0x4001, 0x02},
  {0x4002, 0x45},
  {0x4004, 0x02},
  {0x4005, 0x18},
  {0x4006, 0x08},
  {0x4007, 0x10},
  {0x4008, 0x00},
  {0x4050, 0x6e},
  {0x4051, 0x8f},
  {0x4300, 0xf8},
  {0x4303, 0xff},
  {0x4304, 0x00},
  {0x4307, 0xff},
  {0x4520, 0x00},
  {0x4521, 0x00},
  {0x4511, 0x22},
  {0x4801, 0x0f},
  {0x4814, 0x2a},
  {0x481f, 0x3c},
  {0x4823, 0x3c},
  {0x4826, 0x00},
  {0x481b, 0x3c},
  {0x4827, 0x32},
  {0x4837, 0x0c},
  {0x4b00, 0x06},
  {0x4b01, 0x0a},
  {0x4b04, 0x10},
  {0x5000, 0xff},
  {0x5001, 0x00},
  {0x5002, 0x41},
  {0x5003, 0x0a},
  {0x5004, 0x00},
  {0x5043, 0x00},
  {0x5013, 0x00},
  {0x501f, 0x03},
  {0x503d, 0x00},
  {0x5180, 0x08},
  {0x5a00, 0x08},
  {0x5b00, 0x01},
  {0x5b01, 0x40},
  {0x5b02, 0x00},
  {0x5b03, 0xf0},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 50,
  },
  {
    .reg_setting = init_reg_array1,
    .size = ARRAY_SIZE(init_reg_array1),
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
  {0x0100, 0x01},
  {0x301c, 0xd2},
  {0x301a, 0xf0},
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x301a, 0xf1},
  {0x301c, 0xd6},
  {0x0100, 0x00},
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
  {0x3208, 0x0},
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x3208, 0x10},
  {0x3208, 0xA0},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg skuf_ov5648_p5v23c_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params skuf_ov5648_p5v23c_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &skuf_ov5648_p5v23c_cid_cfg[0],
         &skuf_ov5648_p5v23c_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x15,
  },
};

static struct msm_camera_csi2_params *csi_params[] = {
  &skuf_ov5648_p5v23c_csi_params, /* RES 0*/
  &skuf_ov5648_p5v23c_csi_params, /* RES 1*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t skuf_ov5648_p5v23c_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t skuf_ov5648_p5v23c_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t skuf_ov5648_p5v23c_stream_info[] = {
  {1, &skuf_ov5648_p5v23c_cid_cfg[0], skuf_ov5648_p5v23c_pix_fmt0_fourcc},
  {1, &skuf_ov5648_p5v23c_cid_cfg[1], skuf_ov5648_p5v23c_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t skuf_ov5648_p5v23c_stream_info_array = {
  .sensor_stream_info = skuf_ov5648_p5v23c_stream_info,
  .size = ARRAY_SIZE(skuf_ov5648_p5v23c_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  /* Full size 2592x1944 @12.5 fps */
  {0x3018, 0x2c},
  {0x3501, 0x7b},
  {0x3502, 0x00},
  {0x3708, 0x63},
  {0x3709, 0x12},
  {0x370c, 0xc0},
  {0x3800, 0x00},
  {0x3801, 0x00},
  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3804, 0x0a},
  {0x3805, 0x3f},
  {0x3806, 0x07},
  {0x3807, 0xa3},
  {0x3808, 0x0a},
  {0x3809, 0x20},
  {0x380a, 0x07},
  {0x380b, 0x98},
  {0x380c, 0x0b},
  {0x380d, 0x00},
  {0x380e, 0x09},
  {0x380f, 0x4c},
  {0x3810, 0x00},
  {0x3811, 0x10},
  {0x3812, 0x00},
  {0x3813, 0x06},
  {0x3814, 0x11},
  {0x3815, 0x11},
  {0x3817, 0x00},
  {0x3820, 0x56},
  {0x3821, 0x00},
  {0x4004, 0x04},
  {0x4005, 0x1a},
  {0x350b, 0x40},
  {0x3034, 0x1a},
  {0x3035, 0x11},
  {0x3036, 0x69},
  {0x3037, 0x13},
  {0x3038, 0x00},
  {0x3039, 0x00},
  {0x3105, 0x11},
  {0x3106, 0x05},
  {0x4837, 0x17},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  /* Quarter size 1296x972 @30 fps */
  {0x3018, 0x2c},
  {0x3501, 0x3d},
  {0x3502, 0x00},
  {0x3708, 0x66},
  {0x3709, 0x52},
  {0x370c, 0xc3},
  {0x3800, 0x00},
  {0x3801, 0x00},
  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3804, 0x0a},
  {0x3805, 0x3f},
  {0x3806, 0x07},
  {0x3807, 0xa3},
  {0x3808, 0x05},
  {0x3809, 0x10},
  {0x380a, 0x03},
  {0x380b, 0xcc},
  {0x380c, 0x05},
  {0x380d, 0x80},
  {0x380e, 0x03},
  {0x380f, 0xe0},
  {0x3810, 0x00},
  {0x3811, 0x08},
  {0x3812, 0x00},
  {0x3813, 0x04},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3817, 0x00},
  {0x3820, 0x1e},
  {0x3821, 0x01},
  {0x4004, 0x02},
  {0x4005, 0x18},
  {0x350b, 0x80},
  {0x3034, 0x1a},
  {0x3035, 0x21},
  {0x3036, 0x69},
  {0x3037, 0x13},
  {0x4837, 0x2f},
};

static struct msm_camera_i2c_reg_setting res_settings[] = {
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
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
    .x_output = 2592,
    .y_output = 1944,
    .line_length_pclk = 2816,
    .frame_length_lines = 2380,
    .vt_pixel_clk = 84000000,
    .op_pixel_clk = 84000000,
    .binning_factor = 1,
    .max_fps = 12.5,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 1296,
    .y_output = 972,
    .line_length_pclk = 1408,
    .frame_length_lines = 992,
    .vt_pixel_clk = 42000000,
    .op_pixel_clk = 42000000,
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

static sensor_res_cfg_type_t skuf_ov5648_p5v23c_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t skuf_ov5648_p5v23c_res_table = {
  .res_cfg_type = skuf_ov5648_p5v23c_res_cfg,
  .size = ARRAY_SIZE(skuf_ov5648_p5v23c_res_cfg),
};

static struct sensor_lib_chromatix_t skuf_ov5648_p5v23c_chromatix[] = {
  {
    .common_chromatix = SKUF_OV5648_P5V23C_LOAD_CHROMATIX(common),
    .camera_preview_chromatix =
      SKUF_OV5648_P5V23C_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix =
      SKUF_OV5648_P5V23C_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix =
      SKUF_OV5648_P5V23C_LOAD_CHROMATIX(default_video), /* RES0 */
  },
  {
    .common_chromatix = SKUF_OV5648_P5V23C_LOAD_CHROMATIX(common),
    .camera_preview_chromatix =
      SKUF_OV5648_P5V23C_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix =
      SKUF_OV5648_P5V23C_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix =
      SKUF_OV5648_P5V23C_LOAD_CHROMATIX(default_video), /* RES1 */
  },
};

static struct sensor_lib_chromatix_array
  skuf_ov5648_p5v23c_lib_chromatix_array = {
  .sensor_lib_chromatix = skuf_ov5648_p5v23c_chromatix,
  .size = ARRAY_SIZE(skuf_ov5648_p5v23c_chromatix),
};

/*===========================================================================
 * FUNCTION    - skuf_ov5648_p5v23c_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t skuf_ov5648_p5v23c_real_to_register_gain(float gain)
{
  uint16_t reg_gain, reg_temp;
  reg_gain = (uint16_t)gain;
  reg_temp = reg_gain<<4;
  reg_gain = reg_temp | (((uint16_t)((gain - (float)reg_gain)*16.0))&0x0f);
  return reg_gain;
}


/*===========================================================================
 * FUNCTION    - skuf_ov5648_p5v23c_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float skuf_ov5648_p5v23c_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;
  real_gain = (float) ((float)(reg_gain>>4)+(((float)(reg_gain&0x0f))/16.0));
  return real_gain;
}


/*===========================================================================
 * FUNCTION    - skuf_ov5648_p5v23c_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t skuf_ov5648_p5v23c_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = skuf_ov5648_p5v23c_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain =
    skuf_ov5648_p5v23c_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - skuf_ov5648_p5v23c_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t skuf_ov5648_p5v23c_fill_exposure_array(uint16_t gain,
  uint32_t line, uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting *reg_setting)
{
  int32_t rc = 0;
  uint16_t reg_count = 0;
  uint16_t i = 0;

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

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr - 1;
  reg_setting->reg_setting[reg_count].reg_data = line >> 12;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = line >> 4;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = line << 4;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF00) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_count++;

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

  return rc;
}

static sensor_exposure_table_t skuf_ov5648_p5v23c_expsoure_tbl = {
  .sensor_calculate_exposure = skuf_ov5648_p5v23c_calculate_exposure,
  .sensor_fill_exposure_array = skuf_ov5648_p5v23c_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* sensor output register address */
  .output_reg_addr = &output_reg_addr,
  /* sensor exposure gain register address */
  .exp_gain_info = &exp_gain_info,
  /* sensor aec info */
  .aec_info = &aec_info,
  /* sensor snapshot exposure wait frames info */
  .snapshot_exp_wait_frames = 1,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 1,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 2,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = skuf_ov5648_p5v23c_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(skuf_ov5648_p5v23c_cid_cfg),
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
  .sensor_res_cfg_table = &skuf_ov5648_p5v23c_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &skuf_ov5648_p5v23c_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &skuf_ov5648_p5v23c_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &skuf_ov5648_p5v23c_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - skuf_ov5648_p5v23c_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *skuf_ov5648_p5v23c_open_lib(void)
{
  return &sensor_lib_ptr;
}
