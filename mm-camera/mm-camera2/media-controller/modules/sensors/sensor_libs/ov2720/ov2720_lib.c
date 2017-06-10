/* ov2720_lib.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_OV2720 "ov2720"
#define OV2720_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV2720"_"#n".so"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VIO,
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
    .seq_val = CAM_VDIG,
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
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 0,
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
  .camera_id = CAMERA_2,
  /* sensor slave address */
  .slave_addr = 0x6c,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x300A,
    /* sensor id */
    .sensor_id = 0x2720,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_NOT_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = CAMERA_MODE_2D_B,
  .position = FRONT_CAMERA_B,
  .sensor_mount_angle = SENSOR_MOUNTANGLE_360,
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
  .coarse_int_time_addr = 0x3501,
  .global_gain_addr = 0x3508,
  .vert_offset = 6,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 8.0,
  .max_linecount = 26880,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.93,
  .pix_size = 1.4,
  .f_number = 2.8,
  .total_f_dist = 1.2,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
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
  .csi_lane_mask = 0x7,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 2,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array[] = {
  {0x0103, 0x01},
  {0x3718, 0x10},
  {0x3702, 0x24},
  {0x373a, 0x60},
  {0x3715, 0x01},
  {0x3703, 0x2e},
  {0x3705, 0x10},
  {0x3730, 0x30},
  {0x3704, 0x62},
  {0x3f06, 0x3a},
  {0x371c, 0x00},
  {0x371d, 0xc4},
  {0x371e, 0x01},
  {0x371f, 0x0d},
  {0x3708, 0x61},
  {0x3709, 0x12},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array,
    .size = ARRAY_SIZE(init_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 1,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
  {0x0100, 0x01},
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
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
  {0x3208, 0x00},
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

static struct msm_camera_csid_vc_cfg ov2720_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov2720_csi_params = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov2720_cid_cfg),
      .vc_cfg = {
         &ov2720_cid_cfg[0],
         &ov2720_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x1B,
  },
};

static struct sensor_pix_fmt_info_t ov2720_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov2720_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov2720_stream_info[] = {
  {1, &ov2720_cid_cfg[0], ov2720_pix_fmt0_fourcc},
  {1, &ov2720_cid_cfg[1], ov2720_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov2720_stream_info_array = {
  .sensor_stream_info = ov2720_stream_info,
  .size = ARRAY_SIZE(ov2720_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  {0x3800, 0x00},
  {0x3801, 0x02},
  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3804, 0x07},
  {0x3805, 0xA1},
  {0x3806, 0x04},
  {0x3807, 0x47},
  {0x3810, 0x00},
  {0x3811, 0x09},
  {0x3812, 0x00},
  {0x3813, 0x02},
  {0x3820, 0x80},
  {0x3821, 0x06},
  {0x3814, 0x11},
  {0x3815, 0x11},
  {0x3612, 0x0b},
  {0x3618, 0x04},
  {0x3a08, 0x01},
  {0x3a09, 0x50},
  {0x3a0a, 0x01},
  {0x3a0b, 0x18},
  {0x3a0d, 0x03},
  {0x3a0e, 0x03},
  {0x4520, 0x00},
  {0x4837, 0x1b},
  {0x3000, 0xff},
  {0x3001, 0xff},
  {0x3002, 0xf0},
  {0x3600, 0x08},
  {0x3621, 0xc0},
  {0x3632, 0xd2},
  {0x3633, 0x23},
  {0x3634, 0x54},
  {0x3f01, 0x0c},
  {0x5001, 0xc1},
  {0x3614, 0xf0},
  {0x3630, 0x2d},
  {0x370b, 0x62},
  {0x3706, 0x61},
  {0x4000, 0x02},
  {0x4002, 0xc5},
  {0x4005, 0x08},
  {0x404f, 0x84},
  {0x4051, 0x00},
  {0x5000, 0xcf},
  {0x3a18, 0x00},
  {0x3a19, 0x80},
  {0x3503, 0x03},
  {0x4521, 0x00},
  {0x5183, 0xb0},
  {0x5184, 0xb0},
  {0x5185, 0xb0},
  {0x370c, 0x0c},
  {0x3035, 0x10},
  {0x3036, 0x1e},
  {0x3037, 0x21},
  {0x303e, 0x19},
  {0x3038, 0x06},
  {0x3018, 0x04},
  {0x3000, 0x00},
  {0x3001, 0x00},
  {0x3002, 0x00},
  {0x3a0f, 0x40},
  {0x3a10, 0x38},
  {0x3a1b, 0x48},
  {0x3a1e, 0x30},
  {0x3a11, 0x90},
  {0x3a1f, 0x10},
  {0x4800, 0x24},
  {0x3501, 0x45},
  {0x3502, 0xa0},
  {0x3508, 0x00},
  {0x3509, 0x20},
};

static struct msm_camera_i2c_reg_array res3_reg_array[] = {
  {0x3718, 0x10},
  {0x3702, 0x18},
  {0x373a, 0x3c},
  {0x3715, 0x01},
  {0x3703, 0x1d},
  {0x3705, 0x0b},
  {0x3730, 0x1f},
  {0x3704, 0x3f},
  {0x3f06, 0x1d},
  {0x371c, 0x00},
  {0x371d, 0x83},
  {0x371e, 0x00},
  {0x371f, 0xb6},
  {0x3708, 0x63},
  {0x3709, 0x52},
  {0x3800, 0x01},
  {0x3801, 0x42},
  {0x3802, 0x00},
  {0x3803, 0x40},
  {0x3804, 0x06},
  {0x3805, 0x61},
  {0x3806, 0x04},
  {0x3807, 0x08},
  {0x3808, 0x02},
  {0x3809, 0x80},
  {0x380a, 0x01},
  {0x380b, 0xe0},
  {0x380c, 0x03},
  {0x380d, 0x0c},
  {0x380e, 0x02},
  {0x380f, 0x00},
  {0x3810, 0x00},
  {0x3811, 0x0f},
  {0x3812, 0x00},
  {0x3813, 0x02},
  {0x3820, 0x80},
  {0x3821, 0x06},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3612, 0x0b},
  {0x3618, 0x04},
  {0x3a08, 0x02},
  {0x3a09, 0x67},
  {0x3a0a, 0x02},
  {0x3a0b, 0x00},
  {0x3a0d, 0x00},
  {0x3a0e, 0x00},
  {0x4520, 0x0a},
  {0x4837, 0x29},
  {0x3000, 0xff},
  {0x3001, 0xff},
  {0x3002, 0xf0},
  {0x3600, 0x08},
  {0x3621, 0xc0},
  {0x3632, 0xd2},
  {0x3633, 0x23},
  {0x3634, 0x54},
  {0x3f01, 0x0c},
  {0x5001, 0xc1},
  {0x3614, 0xf0},
  {0x3630, 0x2d},
  {0x370b, 0x62},
  {0x3706, 0x61},
  {0x4000, 0x02},
  {0x4002, 0xc5},
  {0x4005, 0x08},
  {0x404f, 0x84},
  {0x4051, 0x00},
  {0x5000, 0xcf},
  {0x3a18, 0x00},
  {0x3a19, 0x80},
  {0x3503, 0x07},
  {0x4521, 0x00},
  {0x5183, 0xb0},
  {0x5184, 0xb0},
  {0x5185, 0xb0},
  {0x370c, 0x0c},
  {0x3035, 0x30},
  {0x3036, 0x14},
  {0x3037, 0x21},
  {0x303e, 0x19},
  {0x3038, 0x06},
  {0x3018, 0x04},
  {0x3000, 0x00},
  {0x3001, 0x00},
  {0x3002, 0x00},
  {0x3a0f, 0x40},
  {0x3a10, 0x38},
  {0x3a1b, 0x48},
  {0x3a1e, 0x30},
  {0x3a11, 0x90},
  {0x3a1f, 0x10},
  {0x3011, 0x22},
  {0x3a00, 0x58},
};

static struct msm_camera_i2c_reg_array res4_reg_array[] = {
  {0x3718, 0x10},
  {0x3702, 0x18},
  {0x373a, 0x3c},
  {0x3715, 0x01},
  {0x3703, 0x1d},
  {0x3705, 0x0b},
  {0x3730, 0x1f},
  {0x3704, 0x3f},
  {0x3f06, 0x1d},
  {0x371c, 0x00},
  {0x371d, 0x83},
  {0x371e, 0x00},
  {0x371f, 0xb6},
  {0x3708, 0x63},
  {0x3709, 0x52},
  {0x3800, 0x01},
  {0x3801, 0x42},
  {0x3802, 0x00},
  {0x3803, 0x40},
  {0x3804, 0x06},
  {0x3805, 0x61},
  {0x3806, 0x04},
  {0x3807, 0x08},
  {0x3808, 0x02},
  {0x3809, 0x80},
  {0x380a, 0x01},
  {0x380b, 0xe0},
  {0x380c, 0x03},
  {0x380d, 0x0c},
  {0x380e, 0x02},
  {0x380f, 0x00},
  {0x3810, 0x00},
  {0x3811, 0x0f},
  {0x3812, 0x00},
  {0x3813, 0x02},
  {0x3820, 0x80},
  {0x3821, 0x06},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3612, 0x0b},
  {0x3618, 0x04},
  {0x3a08, 0x02},
  {0x3a09, 0x67},
  {0x3a0a, 0x02},
  {0x3a0b, 0x00},
  {0x3a0d, 0x00},
  {0x3a0e, 0x00},
  {0x4520, 0x0a},
  {0x4837, 0x29},
  {0x3000, 0xff},
  {0x3001, 0xff},
  {0x3002, 0xf0},
  {0x3600, 0x08},
  {0x3621, 0xc0},
  {0x3632, 0xd2},
  {0x3633, 0x23},
  {0x3634, 0x54},
  {0x3f01, 0x0c},
  {0x5001, 0xc1},
  {0x3614, 0xf0},
  {0x3630, 0x2d},
  {0x370b, 0x62},
  {0x3706, 0x61},
  {0x4000, 0x02},
  {0x4002, 0xc5},
  {0x4005, 0x08},
  {0x404f, 0x84},
  {0x4051, 0x00},
  {0x5000, 0xcf},
  {0x3a18, 0x00},
  {0x3a19, 0x80},
  {0x3503, 0x07},
  {0x4521, 0x00},
  {0x5183, 0xb0},
  {0x5184, 0xb0},
  {0x5185, 0xb0},
  {0x370c, 0x0c},
  {0x3035, 0x30},
  {0x3036, 0x1e},
  {0x3037, 0x21},
  {0x303e, 0x19},
  {0x3038, 0x06},
  {0x3018, 0x04},
  {0x3000, 0x00},
  {0x3001, 0x00},
  {0x3002, 0x00},
  {0x3a0f, 0x40},
  {0x3a10, 0x38},
  {0x3a1b, 0x48},
  {0x3a1e, 0x30},
  {0x3a11, 0x90},
  {0x3a1f, 0x10},
  {0x3011, 0x22},
  {0x3a00, 0x58},
};

static struct msm_camera_i2c_reg_array res5_reg_array[] = {
  {0x3718, 0x10},
  {0x3702, 0x18},
  {0x373a, 0x3c},
  {0x3715, 0x01},
  {0x3703, 0x1d},
  {0x3705, 0x0b},
  {0x3730, 0x1f},
  {0x3704, 0x3f},
  {0x3f06, 0x1d},
  {0x371c, 0x00},
  {0x371d, 0x83},
  {0x371e, 0x00},
  {0x371f, 0xb6},
  {0x3708, 0x63},
  {0x3709, 0x52},
  {0x3800, 0x01},
  {0x3801, 0x42},
  {0x3802, 0x00},
  {0x3803, 0x40},
  {0x3804, 0x06},
  {0x3805, 0x61},
  {0x3806, 0x04},
  {0x3807, 0x08},
  {0x3808, 0x02},
  {0x3809, 0x80},
  {0x380a, 0x01},
  {0x380b, 0xe0},
  {0x380c, 0x03},
  {0x380d, 0x0c},
  {0x380e, 0x02},
  {0x380f, 0x00},
  {0x3810, 0x00},
  {0x3811, 0x0f},
  {0x3812, 0x00},
  {0x3813, 0x02},
  {0x3820, 0x80},
  {0x3821, 0x06},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3612, 0x0b},
  {0x3618, 0x04},
  {0x3a08, 0x02},
  {0x3a09, 0x67},
  {0x3a0a, 0x02},
  {0x3a0b, 0x00},
  {0x3a0d, 0x00},
  {0x3a0e, 0x00},
  {0x4520, 0x0a},
  {0x4837, 0x29},
  {0x3000, 0xff},
  {0x3001, 0xff},
  {0x3002, 0xf0},
  {0x3600, 0x08},
  {0x3621, 0xc0},
  {0x3632, 0xd2},
  {0x3633, 0x23},
  {0x3634, 0x54},
  {0x3f01, 0x0c},
  {0x5001, 0xc1},
  {0x3614, 0xf0},
  {0x3630, 0x2d},
  {0x370b, 0x62},
  {0x3706, 0x61},
  {0x4000, 0x02},
  {0x4002, 0xc5},
  {0x4005, 0x08},
  {0x404f, 0x84},
  {0x4051, 0x00},
  {0x5000, 0xcf},
  {0x3a18, 0x00},
  {0x3a19, 0x80},
  {0x3503, 0x07},
  {0x4521, 0x00},
  {0x5183, 0xb0},
  {0x5184, 0xb0},
  {0x5185, 0xb0},
  {0x370c, 0x0c},
  {0x3035, 0x10},
  {0x3036, 0x14},
  {0x3037, 0x21},
  {0x303e, 0x19},
  {0x3038, 0x06},
  {0x3018, 0x04},
  {0x3000, 0x00},
  {0x3001, 0x00},
  {0x3002, 0x00},
  {0x3a0f, 0x40},
  {0x3a10, 0x38},
  {0x3a1b, 0x48},
  {0x3a1e, 0x30},
  {0x3a11, 0x90},
  {0x3a1f, 0x10},
  {0x3011, 0x22},
  {0x3a00, 0x58},
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
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct msm_camera_csi2_params *csi_params[] = {
  &ov2720_csi_params, /* RES 0*/
  &ov2720_csi_params, /* RES 1*/
  &ov2720_csi_params, /* RES 2*/
  &ov2720_csi_params, /* RES 3*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
  {0, 0, 0, 0}, /* RES 1 */
  {0, 0, 0, 0}, /* RES 2 */
  {0, 0, 0, 0}, /* RES 3 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
    .x_output = 0x78C, /* 1932 */
    .y_output = 0x444, /* 1092 */
    .line_length_pclk = 0x85c, /* 2140 */
    .frame_length_lines = 0x460, /* 1120 */
    .vt_pixel_clk = 72000000,
    .op_pixel_clk = 72000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 0x280, /* 640 */
    .y_output = 0x1E0, /* 480 */
    .line_length_pclk = 0x30C, /* 780 */
    .frame_length_lines = 0x200, /* 512 */
    .vt_pixel_clk = 24000000,
    .op_pixel_clk = 24000000,
    .binning_factor = 1,
    .max_fps = 60.09,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    .x_output = 0x280, /* 640 */
    .y_output = 0x1E0, /* 480 */
    .line_length_pclk = 0x30C, /* 780 */
    .frame_length_lines = 0x200, /* 512 */
    .vt_pixel_clk = 36000000,
    .op_pixel_clk = 36000000,
    .binning_factor = 1,
    .max_fps = 90.14,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    .x_output = 0x280, /* 640 */
    .y_output = 0x1E0, /* 480 */
    .line_length_pclk = 0x30C, /* 780 */
    .frame_length_lines = 0x200, /* 512 */
    .vt_pixel_clk = 48000000,
    .op_pixel_clk = 48000000,
    .binning_factor = 1,
    .max_fps = 120.19,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov2720_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov2720_res_table = {
  .res_cfg_type = ov2720_res_cfg,
  .size = ARRAY_SIZE(ov2720_res_cfg),
};

static struct sensor_lib_chromatix_t ov2720_chromatix[] = {
  {
    .common_chromatix = OV2720_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV2720_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = OV2720_LOAD_CHROMATIX(preview), /* RES0 */
    .camcorder_chromatix = OV2720_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = OV2720_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
  {
    .common_chromatix = OV2720_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES1 */
    .camera_snapshot_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES1 */
    .camcorder_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES1 */
    .liveshot_chromatix = OV2720_LOAD_CHROMATIX(liveshot), /* RES31 */
  },
  {
    .common_chromatix = OV2720_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES2 */
    .camera_snapshot_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES2 */
    .camcorder_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES2 */
    .liveshot_chromatix = OV2720_LOAD_CHROMATIX(liveshot), /* RES2 */
  },
  {
    .common_chromatix = OV2720_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES31 */
    .camera_snapshot_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES3 */
    .camcorder_chromatix = OV2720_LOAD_CHROMATIX(hfr), /* RES3 */
    .liveshot_chromatix = OV2720_LOAD_CHROMATIX(liveshot), /* RES3 */
  },
};

static struct sensor_lib_chromatix_array ov2720_lib_chromatix_array = {
  .sensor_lib_chromatix = ov2720_chromatix,
  .size = ARRAY_SIZE(ov2720_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov2720_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov2720_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if(gain < 1.0) {
      gain = 1.0;
  } else if (gain > 16.0) {
      gain = 16.0;
  }
  gain = (gain) * 16.0;
  reg_gain = (uint16_t) gain;

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - ov2720_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov2720_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  if (reg_gain < 0x10) {
      reg_gain = 0x10;
  } else if (reg_gain > 0x100) {
      reg_gain = 0x100;
  }
  real_gain = (float) reg_gain / 16.0;

  return real_gain;
}

/*===========================================================================
 * FUNCTION    - ov2720_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov2720_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov2720_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = ov2720_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov2720_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov2720_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting* reg_setting)
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

static sensor_exposure_table_t ov2720_expsoure_tbl = {
  .sensor_calculate_exposure = ov2720_calculate_exposure,
  .sensor_fill_exposure_array = ov2720_fill_exposure_array,
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
  .sensor_num_frame_skip = 2,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 2,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 2,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov2720_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov2720_cid_cfg),
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
  .sensor_res_cfg_table = &ov2720_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov2720_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov2720_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov2720_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - ov2720_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov2720_open_lib(void)
{
  return &sensor_lib_ptr;
}
