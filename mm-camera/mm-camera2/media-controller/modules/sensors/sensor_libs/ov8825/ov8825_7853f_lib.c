/*============================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_OV8825 "ov8825_7853f"
#define OV8825_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV8825"_"#n".so"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VIO,
    .config_val = 0,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VANA,
    .config_val = 0,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VDIG,
    .config_val = 0,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VAF,
    .config_val = 0,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_LOW,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_LOW,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_HIGH,
    .delay = 10,
  },
#ifndef _MSM_BEAR
  {
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 24000000,
    .delay = 10,
  },
#else
  {
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 23880000,
    .delay = 10,
  },
#endif
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
  .slave_addr = 0x6c,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x300a,
    /* sensor id */
    .sensor_id = 0x8825,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 0,
  .position = 0,
  .sensor_mount_angle = 0,
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
  .global_gain_addr = 0x350a,
  .vert_offset = 6,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 8.0,
  .max_linecount = 30834,
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
  .csi_lane_assign = 0xe4,
  .csi_lane_mask = 0x3,
  .csi_if = 1,
  .csid_core = {0},
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x7,
  .csi_if = 1,
  .csid_core = {0},
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103, 0x01},
};

static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x3000, 0x16},
  {0x3001, 0x00},
  {0x3002, 0x6c},
  {0x3003, 0xce},
  {0x3004, 0xd4},
  {0x3005, 0x00},
  {0x3006, 0x10},
  {0x3007, 0x3b},
  {0x300d, 0x00},
  {0x301f, 0x09},
  {0x3020, 0x01},
  {0x3010, 0x00},
  {0x3011, 0x01},
  {0x3012, 0x80},
  {0x3013, 0x39},
  {0x3018, 0x00},
  {0x3104, 0x20},
  {0x3106, 0x15},
  {0x3300, 0x00},
  {0x3500, 0x00},
  {0x3501, 0x27},
  {0x3502, 0x00},
  {0x3503, 0x07},
  {0x3509, 0x00},
  {0x350a, 0x00},
  {0x350b, 0x0b},
  {0x3600, 0x06},
  {0x3601, 0x34},
  {0x3602, 0x42},
  {0x3603, 0x5c},
  {0x3604, 0x98},
  {0x3605, 0xf5},
  {0x3609, 0xb4},
  {0x360a, 0x7c},
  {0x360b, 0xc9},
  {0x360c, 0x0b},
  {0x3612, 0x00},
  {0x3613, 0x02},
  {0x3614, 0x0f},
  {0x3615, 0x00},
  {0x3616, 0x03},
  {0x3617, 0xa1},

  {0x3618, 0x00}, /* AF init settings */
  {0x3619, 0x00},
  {0x361a, 0xb0},
  {0x361b, 0x04},
  {0x361c, 0x07},

  {0x3700, 0x20},
  {0x3701, 0x44},
  {0x3702, 0x50},
  {0x3703, 0xcc},
  {0x3704, 0x19},
  {0x3705, 0x32},
  {0x3706, 0x4b},
  {0x3707, 0x63},
  {0x3708, 0x84},
  {0x3709, 0x40},
  {0x370a, 0x33},
  {0x370b, 0x01},
  {0x370c, 0x50},
  {0x370d, 0x00},
  {0x370e, 0x00},
  {0x3711, 0x0f},
  {0x3712, 0x9c},
  {0x3724, 0x01},
  {0x3725, 0x92},
  {0x3726, 0x01},
  {0x3727, 0xc7},
  {0x3800, 0x00},
  {0x3801, 0x00},
  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3804, 0x0c},
  {0x3805, 0xdf},
  {0x3806, 0x09},
  {0x3807, 0x9b},
  {0x3808, 0x06},
  {0x3809, 0x60},
  {0x380a, 0x04},
  {0x380b, 0xc8},
  {0x380c, 0x0d},
  {0x380d, 0xbc},
  {0x380e, 0x04},
  {0x380f, 0xf0},
  {0x3810, 0x00},
  {0x3811, 0x08},
  {0x3812, 0x00},
  {0x3813, 0x04},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3816, 0x02},
  {0x3817, 0x40},
  {0x3818, 0x00},
  {0x3819, 0x40},
  {0x3820, 0x07},
  {0x3821, 0x11},
  {0x3b1f, 0x00},
  {0x3d00, 0x00},
  {0x3d01, 0x00},
  {0x3d02, 0x00},
  {0x3d03, 0x00},
  {0x3d04, 0x00},
  {0x3d05, 0x00},
  {0x3d06, 0x00},
  {0x3d07, 0x00},
  {0x3d08, 0x00},
  {0x3d09, 0x00},
  {0x3d0a, 0x00},
  {0x3d0b, 0x00},
  {0x3d0c, 0x00},
  {0x3d0d, 0x00},
  {0x3d0e, 0x00},
  {0x3d0f, 0x00},
  {0x3d10, 0x00},
  {0x3d11, 0x00},
  {0x3d12, 0x00},
  {0x3d13, 0x00},
  {0x3d14, 0x00},
  {0x3d15, 0x00},
  {0x3d16, 0x00},
  {0x3d17, 0x00},
  {0x3d18, 0x00},
  {0x3d19, 0x00},
  {0x3d1a, 0x00},
  {0x3d1b, 0x00},
  {0x3d1c, 0x00},
  {0x3d1d, 0x00},
  {0x3d1e, 0x00},
  {0x3d1f, 0x00},
  {0x3d80, 0x00},
  {0x3d81, 0x00},
  {0x3d84, 0x00},
  {0x3f00, 0x00},
  {0x3f01, 0xfc},
  {0x3f05, 0x10},
  {0x3f06, 0x00},
  {0x3f07, 0x00},
  {0x4000, 0x29},
  {0x4001, 0x02},
  {0x4002, 0x45},
  {0x4003, 0x08},
  {0x4004, 0x04},
  {0x4005, 0x18},
  {0x404e, 0x37},
  {0x404f, 0x8f},
  {0x4300, 0xff},
  {0x4303, 0x00},
  {0x4304, 0x08},
  {0x4307, 0x00},
  {0x4600, 0x04},
  {0x4601, 0x00},
  {0x4602, 0x30},
  {0x4800, 0x04},
  {0x4801, 0x0f},
  {0x4837, 0x18},
  {0x4843, 0x02},
  {0x5000, 0x06},
  {0x5001, 0x00},
  {0x5002, 0x00},
  {0x5068, 0x00},
  {0x506a, 0x00},
  {0x501f, 0x00},
  {0x5780, 0xfc},
  {0x5c00, 0x80},
  {0x5c01, 0x00},
  {0x5c02, 0x00},
  {0x5c03, 0x00},
  {0x5c04, 0x00},
  {0x5c05, 0x00},
  {0x5c06, 0x00},
  {0x5c07, 0x80},
  {0x5c08, 0x10},
  {0x6700, 0x05},
  {0x6701, 0x19},
  {0x6702, 0xfd},
  {0x6703, 0xd7},
  {0x6704, 0xff},
  {0x6705, 0xff},
  {0x6800, 0x10},
  {0x6801, 0x02},
  {0x6802, 0x90},
  {0x6803, 0x10},
  {0x6804, 0x59},
  {0x6900, 0x60},
  {0x6901, 0x05},
  {0x5800, 0x0f},
  {0x5801, 0x0d},
  {0x5802, 0x09},
  {0x5803, 0x0a},
  {0x5804, 0x0d},
  {0x5805, 0x14},
  {0x5806, 0x0a},
  {0x5807, 0x04},
  {0x5808, 0x03},
  {0x5809, 0x03},
  {0x580a, 0x05},
  {0x580b, 0x0a},
  {0x580c, 0x05},
  {0x580d, 0x02},
  {0x580e, 0x00},
  {0x580f, 0x00},
  {0x5810, 0x03},
  {0x5811, 0x05},
  {0x5812, 0x09},
  {0x5813, 0x03},
  {0x5814, 0x01},
  {0x5815, 0x01},
  {0x5816, 0x04},
  {0x5817, 0x09},
  {0x5818, 0x09},
  {0x5819, 0x08},
  {0x581a, 0x06},
  {0x581b, 0x06},
  {0x581c, 0x08},
  {0x581d, 0x06},
  {0x581e, 0x33},
  {0x581f, 0x11},
  {0x5820, 0x0e},
  {0x5821, 0x0f},
  {0x5822, 0x11},
  {0x5823, 0x3f},
  {0x5824, 0x08},
  {0x5825, 0x46},
  {0x5826, 0x46},
  {0x5827, 0x46},
  {0x5828, 0x46},
  {0x5829, 0x46},
  {0x582a, 0x42},
  {0x582b, 0x42},
  {0x582c, 0x44},
  {0x582d, 0x46},
  {0x582e, 0x46},
  {0x582f, 0x60},
  {0x5830, 0x62},
  {0x5831, 0x42},
  {0x5832, 0x46},
  {0x5833, 0x46},
  {0x5834, 0x44},
  {0x5835, 0x44},
  {0x5836, 0x44},
  {0x5837, 0x48},
  {0x5838, 0x28},
  {0x5839, 0x46},
  {0x583a, 0x48},
  {0x583b, 0x68},
  {0x583c, 0x28},
  {0x583d, 0xae},
  {0x5842, 0x00},
  {0x5843, 0xef},
  {0x5844, 0x01},
  {0x5845, 0x3f},
  {0x5846, 0x01},
  {0x5847, 0x3f},
  {0x5848, 0x00},
  {0x5849, 0xd5},
  {0x3400, 0x04}, /* red h */
  {0x3401, 0x00}, /* red l */
  {0x3402, 0x04}, /* green h */
  {0x3403, 0x00}, /* green l */
  {0x3404, 0x04}, /* blue h */
  {0x3405, 0x00}, /* blue l */
  {0x3406, 0x01}, /* MWB manual */
  {0x5001, 0x01}, /* MWB on */
  {0x5000, 0x86}, /* LENC on, BPC on, WPC on */
  {0x301a, 0x71}, /* MIPI stream off */
  {0x301c, 0xf4}, /* clock in LP11 mode */
  {0x0100, 0x01},
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
  {0x301c, 0xf0}, /* release clock */
  {0x301a, 0x70}, /* MIPI stream on */
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x301a, 0x71}, /* MIPI stream off */
  {0x301c, 0xf4}, /* clock in LP11 mode */
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

static struct msm_camera_csid_vc_cfg ov8825_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov8825_csi_params = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &ov8825_cid_cfg[0],
         &ov8825_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x7,
  },
};

static struct msm_camera_csi2_params ov8825_csi_params_hfr = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &ov8825_cid_cfg[0],
         &ov8825_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x15,
  },
};

static struct msm_camera_csi2_params *csi_params[] = {
  &ov8825_csi_params, /* RES 0*/
  &ov8825_csi_params, /* RES 2*/
  &ov8825_csi_params_hfr, /* RES 3*/
  &ov8825_csi_params_hfr, /* RES 4*/
  &ov8825_csi_params_hfr, /* RES 5*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t ov8825_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov8825_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov8825_stream_info[] = {
  {1, &ov8825_cid_cfg[0], ov8825_pix_fmt0_fourcc},
  {1, &ov8825_cid_cfg[1], ov8825_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov8825_stream_info_array = {
  .sensor_stream_info = ov8825_stream_info,
  .size = ARRAY_SIZE(ov8825_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {

  /* 3264x2448_2lane_15fps_Sysclk=133.3M_656MBps/lane */
  {0x3004, 0xd8},
  {0x3006, 0x10},
  {0x3020, 0x81},

  {0x3500, 0x00},
  {0x3501, 0x85},
  {0x3502, 0xe0},
  {0x350a, 0x00},
  {0x350b, 0x00},

  {0x3700, 0x10},
  {0x3702, 0x28},
  {0x3703, 0x6c},
  {0x3704, 0x40},
  {0x3705, 0x19},
  {0x3706, 0x27},
  {0x3708, 0x48},
  {0x3709, 0x20},
  {0x370a, 0x31},
  {0x3711, 0x07},
  {0x3712, 0x4e},
  {0x3724, 0x00},
  {0x3725, 0xd4},
  {0x3726, 0x00},
  {0x3727, 0xf0},
  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3806, 0x09},
  {0x3807, 0x9b},
  {0x3808, 0x0c},
  {0x3809, 0xc0},
  {0x380a, 0x09},
  {0x380b, 0x90},
  {0x380c, 0x0e},
  {0x380d, 0x00},
  {0x380e, 0x09},
  {0x380f, 0xb0},
  {0x3811, 0x10},
  {0x3813, 0x06},
  {0x3814, 0x11},
  {0x3815, 0x11},
  {0x3820, 0x80},
  {0x3821, 0x16},
  {0x3f00, 0x02},

  {0x4005, 0x1a}, /*BLC triggered every frame */

  {0x4601, 0x00},
  {0x4602, 0x20},
  {0x4837, 0x1e},
  {0x5068, 0x00},
  {0x506a, 0x00},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {

  /* 1920x1080_2Lane_30fps_Sysclk=200M_656MBps/lane */
  {0x3004, 0xd8},
  {0x3005, 0x00},
  {0x3006, 0x00},
  {0x3007, 0x3b},
  {0x3011, 0x01},
  {0x3012, 0x80},
  {0x3013, 0x39},
  {0x3020, 0x01},
  {0x3106, 0x15},
  {0x3500, 0x00},
  {0x3501, 0x74},
  {0x3502, 0x60},
  {0x350a, 0x00},
  {0x350b, 0x02},
  {0x3600, 0x06},
  {0x3601, 0x34},
  {0x3602, 0x42},
  {0x3700, 0x20},
  {0x3702, 0x50},
  {0x3703, 0xcc},
  {0x3704, 0x19},
  {0x3705, 0x32},
  {0x3706, 0x4b},
  {0x3708, 0x84},
  {0x3709, 0x40},
  {0x370a, 0x31},
  {0x370e, 0x00},
  {0x3711, 0x0f},
  {0x3712, 0x9c},
  {0x3724, 0x01},
  {0x3725, 0x92},
  {0x3726, 0x01},
  {0x3727, 0xc7},
  {0x3800, 0x00},
  {0x3801, 0x00},
  {0x3802, 0x01},
  {0x3803, 0x30},
  {0x3804, 0x0c},
  {0x3805, 0xdf},
  {0x3806, 0x08},
  {0x3807, 0x67},
  {0x3808, 0x07},
  {0x3809, 0x80},
  {0x380a, 0x04},
  {0x380b, 0x38},
  {0x380c, 0x0d},
  {0x380d, 0xf0},
  {0x380e, 0x07},
  {0x380f, 0x4c},
  {0x3810, 0x00},
  {0x3811, 0x10},
  {0x3812, 0x00},
  {0x3813, 0x06},
  {0x3814, 0x11},
  {0x3815, 0x11},
  {0x3820, 0x80},
  {0x3821, 0x16},
  {0x3f00, 0x02},
  {0x4005, 0x18},
  {0x404f, 0x8f},
  {0x4600, 0x04},
  {0x4601, 0x01},
  {0x4602, 0x00},
  {0x4837, 0x1e},
  {0x5068, 0x53},
  {0x506a, 0x53},
};

static struct msm_camera_i2c_reg_array res2_reg_array[] = {
  /* 1632x1224_2Lane_30fps_Sysclk=133.3M_656MBps/lane */
  {0x3004, 0xd8},
  {0x3005, 0x00},
  {0x3006, 0x10},
  {0x3007, 0x3b},
  {0x3011, 0x01},
  {0x3012, 0x80},
  {0x3013, 0x39},
  {0x3020, 0x01},
  {0x3106, 0x15},

  {0x3500, 0x00},
  {0x3501, 0x4e},
  {0x3502, 0xa0},
  {0x350a, 0x00},
  {0x350b, 0x0b},

  {0x3600, 0x06},
  {0x3601, 0x34},
  {0x3602, 0x42},

  {0x3700, 0x20},
  {0x3702, 0x50},
  {0x3703, 0xcc},
  {0x3704, 0x19},
  {0x3705, 0x32},
  {0x3706, 0x4b},
  {0x3708, 0x84},
  {0x3709, 0x40},

  {0x370a, 0x33},
  {0x370e, 0x00},

  {0x3711, 0x0f},
  {0x3712, 0x9c},
  {0x3724, 0x01},
  {0x3725, 0x92},
  {0x3726, 0x01},
  {0x3727, 0xc7},
  {0x3800, 0x00},
  {0x3801, 0x00},

  {0x3802, 0x00},
  {0x3803, 0x00},
  {0x3804, 0x0c},
  {0x3805, 0xdf},
  {0x3806, 0x09},
  {0x3807, 0x9b},
  {0x3808, 0x06},
  {0x3809, 0x60},
  {0x380a, 0x04},
  {0x380b, 0xc8},
  {0x380c, 0x0d},
  {0x380d, 0xbc},
  {0x380e, 0x04},
  {0x380f, 0xf0},
  {0x3810, 0x00},
  {0x3811, 0x08},
  {0x3812, 0x00},
  {0x3813, 0x04},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3820, 0x81},
  {0x3821, 0x17},
  {0x3f00, 0x00},
  {0x4005, 0x18},
  {0x404f, 0x8f},
  {0x4600, 0x04},
  {0x4601, 0x00},
  {0x4602, 0x30},
  {0x4837, 0x1e},
  {0x5068, 0x00},
  {0x506a, 0x00},
};

static struct msm_camera_i2c_reg_array res3_reg_array[] = {
  /* 800*480@60fps, 2lane */
  {0x3003, 0xce}, /* PLL_CTRL0 */
  {0x3004, 0xbf}, /* PLL_CTRL1 */
  {0x3005, 0x10}, /* PLL_CTRL2 */
  {0x3006, 0x00}, /* PLL_CTRL3 */
  {0x3007, 0x3b}, /* PLL_CTRL4 */
  {0x3011, 0x01}, /* 2lane MIPI */
  {0x3012, 0x80}, /* SC_PLL CTRL_S0 */
  {0x3013, 0x39}, /* SC_PLL CTRL_S1 */
  {0x3020, 0x01},
  {0x3106, 0x15}, /* SRB_CTRL */

  {0x3500, 0x00},
  {0x3501, 0x4e},
  {0x3502, 0xe0},
  {0x350a, 0x00},
  {0x350b, 0x0b},

  {0x3600, 0x06}, /* ANACTRL0 */
  {0x3601, 0x34}, /* ANACTRL1 */
  {0x3602, 0x42},
  {0x3700, 0x20}, /* SENCTROL0 Sensor control */
  {0x3702, 0x50}, /* SENCTROL2 Sensor control */
  {0x3703, 0xcc}, /* SENCTROL3 Sensor control */
  {0x3704, 0x19}, /* SENCTROL4 Sensor control */
  {0x3705, 0x32}, /* SENCTROL5 Sensor control */
  {0x3706, 0x4b}, /* SENCTROL6 Sensor control */
  {0x3708, 0x84}, /* SENCTROL8 Sensor control */
  {0x3709, 0x40}, /* SENCTROL9 Sensor control */
  {0x370a, 0xb2}, /* SENCTROLA Sensor control */
  {0x370e, 0x08}, /* SENCTROLE Sensor control */
  {0x3711, 0x0f}, /* SENCTROL11 Sensor control */
  {0x3712, 0x9c}, /* SENCTROL12 Sensor control */
  {0x3724, 0x01}, /* Reserved */
  {0x3725, 0x92}, /* Reserved */
  {0x3726, 0x01}, /* Reserved */
  {0x3727, 0xc7}, /* Reserved */
  {0x3800, 0x00}, /* HS(HREF start High) */
  {0x3801, 0x08}, /* HS(HREF start Low) */
  {0x3802, 0x00}, /* VS(Vertical start High) */
  {0x3803, 0xf4}, /* VS(Vertical start Low) */
  {0x3804, 0x0c}, /* HW = 3295 */
  {0x3805, 0xd7}, /* HW */
  {0x3806, 0x08}, /* VH = 2459 */
  {0x3807, 0xa3}, /* VH */
  {0x3808, 0x03}, /* ISPHO = 1632 */
  {0x3809, 0x20}, /* ISPHO */
  {0x380a, 0x01}, /* ISPVO = 1224 */
  {0x380b, 0xe0}, /* ISPVO */
  {0x380c, 0x0a}, /* HTS = 2628 */
  {0x380d, 0x44}, /* HTS */
  {0x380e, 0x04}, /* VTS = 1268 */
  {0x380f, 0xf4}, /* VTS */
  {0x3810, 0x00}, /* HOFF = 8 */
  {0x3811, 0x0a}, /* HOFF */
  {0x3812, 0x00}, /* VOFF = 4 */
  {0x3813, 0x06}, /* VOFF */
  {0x3814, 0x71}, /* X INC */
  {0x3815, 0x35}, /* Y INC */
  {0x3820, 0x80}, /* Timing Reg20:Vflip */
  {0x3821, 0x17}, /* Timing Reg21:Hmirror */
  {0x3f00, 0x00}, /* PSRAM Ctrl0 */
  {0x4005, 0x18}, /* Gain trigger for BLC */
  {0x404f, 0x8F}, /* Auto BLC while more than value */
  {0x4600, 0x14}, /* VFIFO Ctrl0 */
  {0x4601, 0x14}, /* VFIFO Read ST High */
  {0x4602, 0x00}, /* VFIFO Read ST Low */
  {0x4837, 0x19}, /* MIPI PCLK PERIOD */
  {0x5068, 0x59}, /* HSCALE_CTRL */
  {0x506a, 0x5a}, /* VSCALE_CTRL */
};

static struct msm_camera_i2c_reg_array res4_reg_array[] = {
  /* 800*480@90fps, 2lane */
  {0x3003, 0xce}, /* PLL_CTRL0 */
  {0x3004, 0xbf}, /* PLL_CTRL1 */
  {0x3005, 0x10}, /* PLL_CTRL2 */
  {0x3006, 0x00}, /* PLL_CTRL3 */
  {0x3007, 0x3b}, /* PLL_CTRL4 */
  {0x3011, 0x01}, /* 2lane MIPI */
  {0x3012, 0x80}, /* SC_PLL CTRL_S0 */
  {0x3013, 0x39}, /* SC_PLL CTRL_S1 */
  {0x3020, 0x01},
  {0x3106, 0x15}, /* SRB_CTRL */

  {0x3500, 0x00},
  {0x3501, 0x34},
  {0x3502, 0x60},
  {0x350a, 0x00},
  {0x350b, 0x14},

  {0x3600, 0x06}, /* ANACTRL0 */
  {0x3601, 0x34}, /* ANACTRL1 */
  {0x3602, 0x42},
  {0x3700, 0x20}, /* SENCTROL0 Sensor control */
  {0x3702, 0x50}, /* SENCTROL2 Sensor control */
  {0x3703, 0xcc}, /* SENCTROL3 Sensor control */
  {0x3704, 0x19}, /* SENCTROL4 Sensor control */
  {0x3705, 0x32}, /* SENCTROL5 Sensor control */
  {0x3706, 0x4b}, /* SENCTROL6 Sensor control */
  {0x3708, 0x84}, /* SENCTROL8 Sensor control */
  {0x3709, 0x40}, /* SENCTROL9 Sensor control */
  {0x370a, 0xb2}, /* SENCTROLA Sensor control */
  {0x370e, 0x08}, /* SENCTROLE Sensor control */
  {0x3711, 0x0f}, /* SENCTROL11 Sensor control */
  {0x3712, 0x9c}, /* SENCTROL12 Sensor control */
  {0x3724, 0x01}, /* Reserved */
  {0x3725, 0x92}, /* Reserved */
  {0x3726, 0x01}, /* Reserved */
  {0x3727, 0xc7}, /* Reserved */
  {0x3800, 0x00}, /* HS(HREF start High) */
  {0x3801, 0x08}, /* HS(HREF start Low) */
  {0x3802, 0x00}, /* VS(Vertical start High) */
  {0x3803, 0xf4}, /* VS(Vertical start Low) */
  {0x3804, 0x0c}, /* HW = 3295 */
  {0x3805, 0xd7}, /* HW */
  {0x3806, 0x08}, /* VH = 2459 */
  {0x3807, 0xa3}, /* VH */
  {0x3808, 0x03}, /* ISPHO = 1632 */
  {0x3809, 0x20}, /* ISPHO */
  {0x380a, 0x01}, /* ISPVO = 1224 */
  {0x380b, 0xe0}, /* ISPVO */
  {0x380c, 0x0a}, /* HTS = 2628 */
  {0x380d, 0x44}, /* HTS */
  {0x380e, 0x03}, /* VTS = 845 */
  {0x380f, 0x4c}, /* VTS */
  {0x3810, 0x00}, /* HOFF = 8 */
  {0x3811, 0x0a}, /* HOFF */
  {0x3812, 0x00}, /* VOFF = 4 */
  {0x3813, 0x06}, /* VOFF */
  {0x3814, 0x71}, /* X INC */
  {0x3815, 0x35}, /* Y INC */
  {0x3820, 0x80}, /* Timing Reg20:Vflip */
  {0x3821, 0x17}, /* Timing Reg21:Hmirror */
  {0x3f00, 0x00}, /* PSRAM Ctrl0 */
  {0x4005, 0x18}, /* Gain trigger for BLC */
  {0x404f, 0x8F}, /* Auto BLC while more than value */
  {0x4600, 0x14}, /* VFIFO Ctrl0 */
  {0x4601, 0x14}, /* VFIFO Read ST High */
  {0x4602, 0x00}, /* VFIFO Read ST Low */
  {0x4837, 0x19}, /* MIPI PCLK PERIOD */
  {0x5068, 0x59}, /* HSCALE_CTRL */
  {0x506a, 0x5a}, /* VSCALE_CTRL */
};

static struct msm_camera_i2c_reg_array res5_reg_array[] = {
  /* 800*480@120fps, 2lane */
  {0x3003, 0xce}, /* PLL_CTRL0 */
  {0x3004, 0xbf}, /* PLL_CTRL1 */
  {0x3005, 0x10}, /* PLL_CTRL2 */
  {0x3006, 0x00}, /* PLL_CTRL3 */
  {0x3007, 0x3b}, /* PLL_CTRL4 */
  {0x3011, 0x01}, /* 2lane MIPI */
  {0x3012, 0x80}, /* SC_PLL CTRL_S0 */
  {0x3013, 0x39}, /* SC_PLL CTRL_S1 */
  {0x3020, 0x01},
  {0x3106, 0x15}, /* SRB_CTRL */

  {0x3500, 0x00},
  {0x3501, 0x27},
  {0x3502, 0x40},
  {0x350a, 0x00},
  {0x350b, 0x1b},

  {0x3600, 0x06}, /* ANACTRL0 */
  {0x3601, 0x34}, /* ANACTRL1 */
  {0x3602, 0x42},
  {0x3700, 0x20}, /* SENCTROL0 Sensor control */
  {0x3702, 0x50}, /* SENCTROL2 Sensor control */
  {0x3703, 0xcc}, /* SENCTROL3 Sensor control */
  {0x3704, 0x19}, /* SENCTROL4 Sensor control */
  {0x3705, 0x32}, /* SENCTROL5 Sensor control */
  {0x3706, 0x4b}, /* SENCTROL6 Sensor control */
  {0x3708, 0x84}, /* SENCTROL8 Sensor control */
  {0x3709, 0x40}, /* SENCTROL9 Sensor control */
  {0x370a, 0xb2}, /* SENCTROLA Sensor control */
  {0x370e, 0x08}, /* SENCTROLE Sensor control */
  {0x3711, 0x0f}, /* SENCTROL11 Sensor control */
  {0x3712, 0x9c}, /* SENCTROL12 Sensor control */
  {0x3724, 0x01}, /* Reserved */
  {0x3725, 0x92}, /* Reserved */
  {0x3726, 0x01}, /* Reserved */
  {0x3727, 0xc7}, /* Reserved */
  {0x3800, 0x00}, /* HS(HREF start High) */
  {0x3801, 0x08}, /* HS(HREF start Low) */
  {0x3802, 0x00}, /* VS(Vertical start High) */
  {0x3803, 0xf4}, /* VS(Vertical start Low) */
  {0x3804, 0x0c}, /* HW = 3295 */
  {0x3805, 0xd7}, /* HW */
  {0x3806, 0x08}, /* VH = 2459 */
  {0x3807, 0xa3}, /* VH */
  {0x3808, 0x03}, /* ISPHO = 1632 */
  {0x3809, 0x20}, /* ISPHO */
  {0x380a, 0x01}, /* ISPVO = 1224 */
  {0x380b, 0xe0}, /* ISPVO */
  {0x380c, 0x0a}, /* HTS = 2628 */
  {0x380d, 0x44}, /* HTS */
  {0x380e, 0x02}, /* VTS = 634 */
  {0x380f, 0x7a}, /* VTS */
  {0x3810, 0x00}, /* HOFF = 8 */
  {0x3811, 0x0a}, /* HOFF */
  {0x3812, 0x00}, /* VOFF = 4 */
  {0x3813, 0x06}, /* VOFF */
  {0x3814, 0x71}, /* X INC */
  {0x3815, 0x35}, /* Y INC */
  {0x3820, 0x80}, /* Timing Reg20:Vflip */
  {0x3821, 0x17}, /* Timing Reg21:Hmirror */
  {0x3f00, 0x00}, /* PSRAM Ctrl0 */
  {0x4005, 0x18}, /* Gain trigger for BLC */
  {0x404f, 0x8F}, /* Auto BLC while more than value */
  {0x4600, 0x14}, /* VFIFO Ctrl0 */
  {0x4601, 0x14}, /* VFIFO Read ST High */
  {0x4602, 0x00}, /* VFIFO Read ST Low */
  {0x4837, 0x19}, /* MIPI PCLK PERIOD */
  {0x5068, 0x59}, /* HSCALE_CTRL */
  {0x506a, 0x5a}, /* VSCALE_CTRL */
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
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
  {0, 0, 0, 0}, /* RES 1 */
  {0, 0, 0, 0}, /* RES 2 */

  {0, 0, 0, 0}, /* RES 3 */
  {0, 0, 0, 0}, /* RES 4 */
  {0, 0, 0, 0}, /* RES 5 */

};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
    .x_output = 3264,
    .y_output = 2448,
    .line_length_pclk = 3584,
    .frame_length_lines = 2480,
    .vt_pixel_clk = 133400000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 15,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {

    .x_output = 1632,
    .y_output = 1224,
    .line_length_pclk = 3516,
    .frame_length_lines = 1264,
    .vt_pixel_clk = 133400000,
    .op_pixel_clk = 320000000,
    .binning_factor = 2,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 0x320,
    .y_output = 0x1e0,
    .line_length_pclk = 0xa44,
    .frame_length_lines = 0x4f4,
    .vt_pixel_clk = 320000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 60.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    .x_output = 0x320,
    .y_output = 0x1e0,
    .line_length_pclk = 0xa44,
    .frame_length_lines = 0x34c,
    .vt_pixel_clk = 320000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 90.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },

  {
    .x_output = 0x320,
    .y_output = 0x1e0,
    .line_length_pclk = 0xa44,
    .frame_length_lines = 0x27a,
    .vt_pixel_clk = 320000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 120.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },


};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov8825_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov8825_res_table = {
  .res_cfg_type = ov8825_res_cfg,
  .size = ARRAY_SIZE(ov8825_res_cfg),
};

static struct sensor_lib_chromatix_t ov8825_chromatix[] = {
  {
    .common_chromatix = OV8825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV8825_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = OV8825_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = OV8825_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = OV8825_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
  {
    .common_chromatix = OV8825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV8825_LOAD_CHROMATIX(preview), /* RES2 */
    .camera_snapshot_chromatix = OV8825_LOAD_CHROMATIX(preview), /* RES2 */
    .camcorder_chromatix = OV8825_LOAD_CHROMATIX(default_video), /* RES2 */
    .liveshot_chromatix = OV8825_LOAD_CHROMATIX(liveshot), /* RES2 */
  },

  {
    .common_chromatix = OV8825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV8825_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .camera_snapshot_chromatix = OV8825_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .camcorder_chromatix = OV8825_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .liveshot_chromatix = OV8825_LOAD_CHROMATIX(liveshot), /* RES3 */
  },

  {
    .common_chromatix = OV8825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV8825_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .camera_snapshot_chromatix = OV8825_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .camcorder_chromatix = OV8825_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .liveshot_chromatix = OV8825_LOAD_CHROMATIX(liveshot), /* RES4 */
  },

  {
    .common_chromatix = OV8825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV8825_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
    .camera_snapshot_chromatix = OV8825_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
    .camcorder_chromatix = OV8825_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
    .liveshot_chromatix = OV8825_LOAD_CHROMATIX(liveshot), /* RES5 */
  },
};

static struct sensor_lib_chromatix_array ov8825_lib_chromatix_array = {
  .sensor_lib_chromatix = ov8825_chromatix,
  .size = ARRAY_SIZE(ov8825_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov8825_real_to_register_gain -
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
}


/*===========================================================================
 * FUNCTION    - ov8825_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov8825_register_to_real_gain(uint16_t reg_gain)
{
  float gain;
  float gain_mult;

  if (reg_gain > 0x1FF)
  reg_gain = 0x1FF;

  gain_mult = (reg_gain >> 4) + 1;
  gain = ((float)(reg_gain & 0xF)/16 + 1) * gain_mult;
  return gain;
}


/*===========================================================================
 * FUNCTION    - ov8825_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov8825_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov8825_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = ov8825_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov8825_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov8825_fill_exposure_array(uint16_t gain, uint32_t line,
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

static sensor_exposure_table_t ov8825_expsoure_tbl = {
  .sensor_calculate_exposure = ov8825_calculate_exposure,
  .sensor_fill_exposure_array = ov8825_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
  .actuator_name = "ov8825",
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
  .sensor_num_HDR_frame_skip = 1,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov8825_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov8825_cid_cfg),
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
  .sensor_res_cfg_table = &ov8825_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov8825_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov8825_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov8825_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - ov8825_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov8825_open_lib(void)
{
  return &sensor_lib_ptr;
}
