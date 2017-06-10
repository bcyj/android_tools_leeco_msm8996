/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_OV16825 "ov16825"
#define OV16825_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV16825"_"#n".so"

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
  .camera_id = CAMERA_0,
  /* sensor slave address */
  .slave_addr = 0x6c,
  /* sensor i2c frequency*/
  .i2c_freq_mode = I2C_FAST_MODE,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x300a,
    /* sensor id */
    .sensor_id = 0x0168,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
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
  .x_output = 0x3808,
  .y_output = 0x380a,
  .line_length_pclk = 0x380c,
  .frame_length_lines = 0x380e,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x3501,
  .global_gain_addr = 0x3508,
  .vert_offset = 4,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 16.0,
  .max_linecount = 65535,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.67,
  .pix_size = 1.4,
  .f_number = 2.0,
  .total_f_dist = 1.2,
  .hor_view_angle = 67.5,
  .ver_view_angle = 53.3,
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
  .csi_lane_mask = 0x1F,
  .csi_if = 1,
  .csid_core = { 0 },
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103, 0x01, 0x00},
};

static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x0300, 0x02, 0x00},
  {0x0302, 0x64, 0x00},
  {0x0305, 0x01, 0x00},
  {0x0306, 0x00, 0x00},
  {0x030b, 0x02, 0x00},
  {0x030c, 0x14, 0x00},
  {0x030e, 0x00, 0x00},
  {0x0313, 0x02, 0x00},
  {0x0314, 0x14, 0x00},
  {0x031f, 0x00, 0x00},
  {0x3022, 0x01, 0x00},
  {0x3032, 0x80, 0x00},
  {0x3601, 0xf8, 0x00},
  {0x3602, 0x00, 0x00},
  {0x3605, 0x50, 0x00},
  {0x3606, 0x00, 0x00},
  {0x3607, 0x2b, 0x00},
  {0x3608, 0x16, 0x00},
  {0x3609, 0x00, 0x00},
  {0x360e, 0x99, 0x00},
  {0x360f, 0x75, 0x00},
  {0x3610, 0x69, 0x00},
  {0x3611, 0x59, 0x00},
  {0x3612, 0x40, 0x00},
  {0x3613, 0x89, 0x00},
  {0x3615, 0x64, 0x00},
  {0x3617, 0x00, 0x00},
  {0x3618, 0x20, 0x00},
  {0x3619, 0x00, 0x00},
  {0x361a, 0x10, 0x00},
  {0x361c, 0x10, 0x00},
  {0x361d, 0x00, 0x00},
  {0x361e, 0x00, 0x00},
  {0x3640, 0x66, 0x00},
  {0x3641, 0x56, 0x00},
  {0x3642, 0x73, 0x00},
  {0x3643, 0x32, 0x00},
  {0x3644, 0x03, 0x00},
  {0x3645, 0x04, 0x00},
  {0x3646, 0x85, 0x00},
  {0x364a, 0x07, 0x00},
  {0x3707, 0x08, 0x00},
  {0x3718, 0x75, 0x00},
  {0x371a, 0x55, 0x00},
  {0x371c, 0x55, 0x00},
  {0x3733, 0x80, 0x00},
  {0x3760, 0x00, 0x00},
  {0x3761, 0x30, 0x00},
  {0x3762, 0x00, 0x00},
  {0x3763, 0xc0, 0x00},
  {0x3764, 0x03, 0x00},
  {0x3765, 0x00, 0x00},
  {0x3823, 0x08, 0x00},
  {0x3827, 0x02, 0x00},
  {0x3828, 0x00, 0x00},
  {0x3832, 0x00, 0x00},
  {0x3833, 0x00, 0x00},
  {0x3834, 0x00, 0x00},
  {0x3d85, 0x17, 0x00},
  {0x3d8c, 0x70, 0x00},
  {0x3d8d, 0xa0, 0x00},
  {0x3f00, 0x02, 0x00},
  {0x4001, 0x83, 0x00},
  {0x400e, 0x00, 0x00},
  {0x4011, 0x00, 0x00},
  {0x4012, 0x00, 0x00},
  {0x4200, 0x08, 0x00},
  {0x4302, 0x7f, 0x00},
  {0x4303, 0xff, 0x00},
  {0x4304, 0x00, 0x00},
  {0x4305, 0x00, 0x00},
  {0x4501, 0x30, 0x00},
  {0x4603, 0x60, 0x00},
  {0x4b00, 0x22, 0x00},
  {0x4903, 0x00, 0x00},
  {0x5000, 0x7f, 0x00},
  {0x5001, 0x01, 0x00},
  {0x5004, 0x00, 0x00},
  {0x5013, 0x20, 0x00},
  {0x5051, 0x00, 0x00},
  {0x5500, 0x01, 0x00},
  {0x5501, 0x00, 0x00},
  {0x5502, 0x07, 0x00},
  {0x5503, 0xff, 0x00},
  {0x5505, 0x6c, 0x00},
  {0x5509, 0x02, 0x00},
  {0x5780, 0xfc, 0x00},
  {0x5781, 0xff, 0x00},
  {0x5787, 0x40, 0x00},
  {0x5788, 0x08, 0x00},
  {0x578a, 0x02, 0x00},
  {0x578b, 0x01, 0x00},
  {0x578c, 0x01, 0x00},
  {0x578e, 0x02, 0x00},
  {0x578f, 0x01, 0x00},
  {0x5790, 0x01, 0x00},
  {0x5792, 0x00, 0x00},
  {0x5980, 0x00, 0x00},
  {0x5981, 0x21, 0x00},
  {0x5982, 0x00, 0x00},
  {0x5983, 0x00, 0x00},
  {0x5984, 0x00, 0x00},
  {0x5985, 0x00, 0x00},
  {0x5986, 0x00, 0x00},
  {0x5987, 0x00, 0x00},
  {0x5988, 0x00, 0x00},

  {0x3201, 0x15, 0x00},
  {0x3202, 0x2a, 0x00},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
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
  {0x0100, 0x01, 0x00},
  {0x301c, 0xf0, 0x00}, /* release clock */
  {0x301a, 0x70, 0x00}, /* MIPI stream on */
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x301a, 0x71, 0x00}, /* MIPI stream off */
  {0x301c, 0xf4, 0x00}, /* clock in LP11 mode */
  {0x0100, 0x00, 0x00},
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

static struct msm_camera_csid_vc_cfg ov16825_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov16825_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov16825_cid_cfg),
      .vc_cfg = {
         &ov16825_cid_cfg[0],
         &ov16825_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 0x1B,
  },
};

static struct msm_camera_csi2_params *csi_params[] = {
  &ov16825_csi_params, /* RES 0*/
  &ov16825_csi_params, /* RES 1*/
  &ov16825_csi_params, /* RES 2*/
  &ov16825_csi_params, /* RES 3*/
  &ov16825_csi_params, /* RES 4*/
  &ov16825_csi_params, /* RES 5*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t ov16825_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov16825_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov16825_stream_info[] = {
  {1, &ov16825_cid_cfg[0], ov16825_pix_fmt0_fourcc},
  {1, &ov16825_cid_cfg[1], ov16825_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov16825_stream_info_array = {
  .sensor_stream_info = ov16825_stream_info,
  .size = ARRAY_SIZE(ov16825_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  /* MIPI 4-Lane 4608x3456 10-bit 15fps 1096Mbps/lane */
  {0x0302, 0x89, 0x00},/*96 26.4 fps*/
  {0x0305, 0x01, 0x00},
  {0x030e, 0x00, 0x00},
  {0x3018, 0x7a, 0x00},
  {0x3031, 0x0a, 0x00},
  {0x3603, 0x00, 0x00},
  {0x3604, 0x00, 0x00},
  {0x360a, 0x00, 0x00},
  {0x360b, 0x82, 0x00},
  {0x360c, 0x1a, 0x00},
  {0x360d, 0x00, 0x00},
  {0x3614, 0x77, 0x00},
  {0x3616, 0x30, 0x00},
  {0x3631, 0x60, 0x00},
  {0x3700, 0x60, 0x00},
  {0x3701, 0x10, 0x00},
  {0x3702, 0x22, 0x00},
  {0x3703, 0x40, 0x00},
  {0x3704, 0x10, 0x00},
  {0x3705, 0x01, 0x00},
  {0x3706, 0x34, 0x00},
  {0x3708, 0x40, 0x00},
  {0x3709, 0x78, 0x00},
  {0x370a, 0x02, 0x00},
  {0x370b, 0xde, 0x00},
  {0x370c, 0x06, 0x00},
  {0x370e, 0x40, 0x00},
  {0x370f, 0x0a, 0x00},
  {0x3710, 0x30, 0x00},
  {0x3711, 0x40, 0x00},
  {0x3714, 0x31, 0x00},
  {0x3719, 0x25, 0x00},
  {0x371b, 0x05, 0x00},
  {0x371d, 0x05, 0x00},
  {0x371e, 0x11, 0x00},
  {0x371f, 0x2d, 0x00},
  {0x3720, 0x15, 0x00},
  {0x3721, 0x30, 0x00},
  {0x3722, 0x15, 0x00},
  {0x3723, 0x30, 0x00},
  {0x3724, 0x08, 0x00},
  {0x3725, 0x08, 0x00},
  {0x3726, 0x04, 0x00},
  {0x3727, 0x04, 0x00},
  {0x3728, 0x04, 0x00},
  {0x3729, 0x04, 0x00},
  {0x372a, 0x29, 0x00},
  {0x372b, 0xc9, 0x00},
  {0x372c, 0xa9, 0x00},
  {0x372d, 0xb9, 0x00},
  {0x372e, 0x95, 0x00},
  {0x372f, 0x55, 0x00},
  {0x3730, 0x55, 0x00},
  {0x3731, 0x55, 0x00},
  {0x3732, 0x05, 0x00},
  {0x3734, 0x90, 0x00},
  {0x3739, 0x05, 0x00},
  {0x373a, 0x40, 0x00},
  {0x373b, 0x18, 0x00},
  {0x373c, 0x38, 0x00},
  {0x373e, 0x15, 0x00},
  {0x373f, 0x80, 0x00},
  {0x3800, 0x00, 0x00},
  {0x3801, 0x20, 0x00},
  {0x3802, 0x00, 0x00},
  {0x3803, 0x0e, 0x00},
  {0x3804, 0x12, 0x00},
  {0x3805, 0x3f, 0x00},
  {0x3806, 0x0d, 0x00},
  {0x3807, 0x93, 0x00},
  {0x3808, 0x12, 0x00},
  {0x3809, 0x00, 0x00},
  {0x380a, 0x0d, 0x00},
  {0x380b, 0x80, 0x00},
  {0x380c, 0x07, 0x00},
  {0x380d, 0x76, 0x00},
  {0x380e, 0x0d, 0x00},
  {0x380f, 0xa2, 0x00},
  {0x3811, 0x0f, 0x00},
  {0x3813, 0x02, 0x00},
  {0x3814, 0x01, 0x00},
  {0x3815, 0x01, 0x00},
  {0x3820, 0x00, 0x00},
  {0x3821, 0x06, 0x00},
  {0x3829, 0x00, 0x00},
  {0x382a, 0x01, 0x00},
  {0x382b, 0x01, 0x00},
  {0x3830, 0x08, 0x00},
  {0x3f00, 0x02, 0x00},
  {0x3f02, 0x00, 0x00},
  {0x3f04, 0x00, 0x00},
  {0x3f05, 0x00, 0x00},
  {0x3f08, 0x40, 0x00},
  {0x4002, 0x04, 0x00},
  {0x4003, 0x08, 0x00},
  {0x4306, 0x00, 0x00},
  {0x4837, 0x0e, 0x00},
  {0x3501, 0xd9, 0x00},
  {0x3502, 0xe0, 0x00},
  {0x3508, 0x08, 0x00},
  {0x3509, 0xff, 0x00},
  {0x3638, 0x00, 0x00},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  /*2304x1728@30fps*/
  {0x0302, 0x64, 0x00},
  {0x0305, 0x01, 0x00},
  {0x030c, 0x15, 0x00},/*sysclk = 168*/
  {0x030e, 0x01, 0x00},
  {0x3018, 0x7a, 0x00},
  {0x3031, 0x0a, 0x00},
  {0x3603, 0x00, 0x00},
  {0x3604, 0x00, 0x00},
  {0x360a, 0x00, 0x00},
  {0x360b, 0x82, 0x00},
  {0x360c, 0x1a, 0x00},
  {0x360d, 0x00, 0x00},
  {0x3614, 0x75, 0x00},
  {0x3616, 0x30, 0x00},
  {0x3631, 0x60, 0x00},
  {0x3700, 0x30, 0x00},
  {0x3701, 0x08, 0x00},
  {0x3702, 0x11, 0x00},
  {0x3703, 0x20, 0x00},
  {0x3704, 0x08, 0x00},
  {0x3705, 0x00, 0x00},
  {0x3706, 0x9a, 0x00},
  {0x3708, 0x20, 0x00},
  {0x3709, 0x3c, 0x00},
  {0x370a, 0x01, 0x00},
  {0x370b, 0x6f, 0x00},
  {0x370c, 0x03, 0x00},
  {0x370e, 0x20, 0x00},
  {0x370f, 0x05, 0x00},
  {0x3710, 0x20, 0x00},
  {0x3711, 0x20, 0x00},
  {0x3714, 0x31, 0x00},
  {0x3719, 0x13, 0x00},
  {0x371b, 0x03, 0x00},
  {0x371d, 0x03, 0x00},
  {0x371e, 0x09, 0x00},
  {0x371f, 0x17, 0x00},
  {0x3720, 0x0b, 0x00},
  {0x3721, 0x18, 0x00},
  {0x3722, 0x0b, 0x00},
  {0x3723, 0x18, 0x00},
  {0x3724, 0x04, 0x00},
  {0x3725, 0x04, 0x00},
  {0x3726, 0x02, 0x00},
  {0x3727, 0x02, 0x00},
  {0x3728, 0x02, 0x00},
  {0x3729, 0x02, 0x00},
  {0x372a, 0x25, 0x00},
  {0x372b, 0x65, 0x00},
  {0x372c, 0x55, 0x00},
  {0x372d, 0x65, 0x00},
  {0x372e, 0x53, 0x00},
  {0x372f, 0x33, 0x00},
  {0x3730, 0x33, 0x00},
  {0x3731, 0x33, 0x00},
  {0x3732, 0x03, 0x00},
  {0x3734, 0x90, 0x00},
  {0x3739, 0x03, 0x00},
  {0x373a, 0x20, 0x00},
  {0x373b, 0x0c, 0x00},
  {0x373c, 0x1c, 0x00},
  {0x373e, 0x0b, 0x00},
  {0x373f, 0x80, 0x00},
  {0x3800, 0x00, 0x00},
  {0x3801, 0x00, 0x00},
  {0x3802, 0x00, 0x00},
  {0x3803, 0x0c, 0x00},
  {0x3804, 0x12, 0x00},
  {0x3805, 0x3f, 0x00},
  {0x3806, 0x0d, 0x00},
  {0x3807, 0x97, 0x00},
  {0x3808, 0x09, 0x00},
  {0x3809, 0x00, 0x00},
  {0x380a, 0x06, 0x00},
  {0x380b, 0xc0, 0x00},
  {0x380c, 0x05, 0x00},
  {0x380d, 0xf0, 0x00},
  {0x380e, 0x07, 0x00},
  {0x380f, 0x32, 0x00},
  {0x3811, 0x17, 0x00},
  {0x3813, 0x02, 0x00},
  {0x3814, 0x03, 0x00},
  {0x3815, 0x01, 0x00},
  {0x3820, 0x01, 0x00},
  {0x3821, 0x07, 0x00},
  {0x3829, 0x02, 0x00},
  {0x382a, 0x03, 0x00},
  {0x382b, 0x01, 0x00},
  {0x3830, 0x08, 0x00},
  {0x3f00, 0x06, 0x00},
  {0x3f02, 0x00, 0x00},
  {0x3f04, 0x03, 0x00},
  {0x3f05, 0x58, 0x00},
  {0x3f08, 0x20, 0x00},
  {0x4002, 0x02, 0x00},
  {0x4003, 0x04, 0x00},
  {0x4306, 0x00, 0x00},
  {0x4837, 0x14, 0x00},
  {0x3501, 0x6d, 0x00},
  {0x3502, 0x60, 0x00},
  {0x3508, 0x08, 0x00},
  {0x3509, 0xff, 0x00},
  {0x3638, 0x00, 0x00},//activate,0x36xx
};

static struct msm_camera_i2c_reg_array res2_reg_array[] = {
  {0x0302, 0x64, 0x00},
  {0x0305, 0x01, 0x00},
  {0x030c, 0x14, 0x00},
  {0x030e, 0x00, 0x00},
  {0x3018, 0x7a, 0x00},
  {0x3031, 0x0a, 0x00},
  {0x3603, 0x00, 0x00},
  {0x3604, 0x00, 0x00},
  {0x360a, 0x02, 0x00},
  {0x360b, 0x82, 0x00},
  {0x360c, 0x1a, 0x00},
  {0x360d, 0x00, 0x00},
  {0x3614, 0x75, 0x00},
  {0x3616, 0x31, 0x00},
  {0x3631, 0x60, 0x00},
  {0x3700, 0x60, 0x00},
  {0x3701, 0x10, 0x00},
  {0x3702, 0x22, 0x00},
  {0x3703, 0x40, 0x00},
  {0x3704, 0x10, 0x00},
  {0x3705, 0x01, 0x00},
  {0x3706, 0x34, 0x00},
  {0x3708, 0x40, 0x00},
  {0x3709, 0x78, 0x00},
  {0x370a, 0x02, 0x00},
  {0x370b, 0xde, 0x00},
  {0x370c, 0x06, 0x00},
  {0x370e, 0x40, 0x00},
  {0x370f, 0x0a, 0x00},
  {0x3710, 0x30, 0x00},
  {0x3711, 0x40, 0x00},
  {0x3714, 0x31, 0x00},
  {0x3719, 0x25, 0x00},
  {0x371b, 0x05, 0x00},
  {0x371d, 0x05, 0x00},
  {0x371e, 0x11, 0x00},
  {0x371f, 0x2d, 0x00},
  {0x3720, 0x15, 0x00},
  {0x3721, 0x30, 0x00},
  {0x3722, 0x15, 0x00},
  {0x3723, 0x30, 0x00},
  {0x3724, 0x08, 0x00},
  {0x3725, 0x08, 0x00},
  {0x3726, 0x04, 0x00},
  {0x3727, 0x04, 0x00},
  {0x3728, 0x04, 0x00},
  {0x3729, 0x04, 0x00},
  {0x372a, 0x29, 0x00},
  {0x372b, 0xc9, 0x00},
  {0x372c, 0xa9, 0x00},
  {0x372d, 0xb9, 0x00},
  {0x372e, 0x95, 0x00},
  {0x372f, 0x55, 0x00},
  {0x3730, 0x55, 0x00},
  {0x3731, 0x55, 0x00},
  {0x3732, 0x05, 0x00},
  {0x3734, 0x90, 0x00},
  {0x3739, 0x05, 0x00},
  {0x373a, 0x40, 0x00},
  {0x373b, 0x18, 0x00},
  {0x373c, 0x38, 0x00},
  {0x373e, 0x15, 0x00},
  {0x373f, 0x80, 0x00},
  {0x3800, 0x01, 0x00},
  {0x3801, 0x80, 0x00},
  {0x3802, 0x02, 0x00},
  {0x3803, 0x94, 0x00},
  {0x3804, 0x10, 0x00},
  {0x3805, 0xbf, 0x00},
  {0x3806, 0x0b, 0x00},
  {0x3807, 0x0f, 0x00},
  {0x3808, 0x07, 0x00},
  {0x3809, 0x80, 0x00},
  {0x380a, 0x04, 0x00},
  {0x380b, 0x38, 0x00},
  {0x380c, 0x04, 0x00},
  {0x380d, 0xb6, 0x00},
  {0x380e, 0x10, 0x00},
  {0x380f, 0xe0, 0x00},
  {0x3811, 0x17, 0x00},
  {0x3813, 0x02, 0x00},
  {0x3814, 0x03, 0x00},
  {0x3815, 0x01, 0x00},
  {0x3820, 0x00, 0x00},
  {0x3821, 0x07, 0x00},
  {0x3829, 0x00, 0x00},
  {0x382a, 0x03, 0x00},
  {0x382b, 0x01, 0x00},
  {0x3830, 0x08, 0x00},
  {0x3f00, 0x02, 0x00},
  {0x3f02, 0x00, 0x00},
  {0x3f04, 0x00, 0x00},
  {0x3f05, 0x00, 0x00},
  {0x3f08, 0x40, 0x00},
  {0x4002, 0x02, 0x00},
  {0x4003, 0x04, 0x00},
  {0x4306, 0x00, 0x00},
  {0x4837, 0x14, 0x00},
  {0x3501, 0x44, 0x00},
  {0x3502, 0xe0, 0x00},
  {0x3508, 0x08, 0x00},
  {0x3509, 0xff, 0x00},
  {0x3638, 0x00, 0x00},
};

static struct msm_camera_i2c_reg_array res3_reg_array[] = {
};
static struct msm_camera_i2c_reg_array res4_reg_array[] = {
};

static struct msm_camera_i2c_reg_array res5_reg_array[] = {
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
    .x_output = 4608, /* 4608 */
    .y_output = 3456, /* 3456 */
    .line_length_pclk = 1910, //1910
    .frame_length_lines = 3490, //3490
    .vt_pixel_clk = 160000000,
    .op_pixel_clk = 480000000,
    .binning_factor = 0,
    .max_fps = 24,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 2304, /* 2304 */
    .y_output = 1728, /* 1728 */
    .line_length_pclk = 1520, //0x5d8,
    .frame_length_lines = 1842,
    .vt_pixel_clk = 80000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 30.5,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 1920, /*1920*/
    .y_output = 1080, /*1080*/
    .line_length_pclk = 1206,//0x4b6,
    .frame_length_lines = 4320,//0x452,
    .vt_pixel_clk = 80000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 30,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 0x780, /*1920*/
    .y_output = 0x438, /*1080*/
    .line_length_pclk = 0x43c, //433
    .frame_length_lines = 0x4ce,//0x452,
    .vt_pixel_clk = 160000000,//1600000000,
    .op_pixel_clk = 320000000,//398400000,//320000000,
    .binning_factor = 1,
    .max_fps = 120,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov16825_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov16825_res_table = {
  .res_cfg_type = ov16825_res_cfg,
  .size = ARRAY_SIZE(ov16825_res_cfg),
};

static struct sensor_lib_chromatix_t ov16825_chromatix[] = {
  {
    .common_chromatix = OV16825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV16825_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = OV16825_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = OV16825_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = OV16825_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
  {
    .common_chromatix = OV16825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV16825_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = OV16825_LOAD_CHROMATIX(snapshot), /* RES1 */
    .camcorder_chromatix = OV16825_LOAD_CHROMATIX(default_video), /* RES1 */
    .liveshot_chromatix = OV16825_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
  {
    .common_chromatix = OV16825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV16825_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = OV16825_LOAD_CHROMATIX(snapshot), /* RES1 */
    .camcorder_chromatix = OV16825_LOAD_CHROMATIX(default_video), /* RES1 */
    .liveshot_chromatix = OV16825_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
  {
    .common_chromatix = OV16825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV16825_LOAD_CHROMATIX(hfr_60fps), /* RES2 */
    .camera_snapshot_chromatix = OV16825_LOAD_CHROMATIX(hfr_60fps), /* RES2 */
    .camcorder_chromatix = OV16825_LOAD_CHROMATIX(hfr_60fps), /* RES2 */
    .liveshot_chromatix = OV16825_LOAD_CHROMATIX(liveshot), /* RES2 */
  },
  {
    .common_chromatix = OV16825_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV16825_LOAD_CHROMATIX(hfr_120fps), /* RES3 */
    .camera_snapshot_chromatix = OV16825_LOAD_CHROMATIX(hfr_120fps), /* RES3 */
    .camcorder_chromatix = OV16825_LOAD_CHROMATIX(hfr_120fps), /* RES3 */
    .liveshot_chromatix = OV16825_LOAD_CHROMATIX(liveshot), /* RES3 */
  },
};

static struct sensor_lib_chromatix_array ov16825_lib_chromatix_array = {
  .sensor_lib_chromatix = ov16825_chromatix,
  .size = ARRAY_SIZE(ov16825_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov16825_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov16825_real_to_register_gain(float gain)
{
  int i;
  uint16_t reg_gain;
  uint8_t gain_regh = 0;

  if (gain < 1.0)
    gain = 1.0;
  if (gain > 16.0)
    gain = 16.0;

  for (i = 1; i <= 3; i++){
    if (gain >= 2){
      gain_regh = gain_regh + 1;
      gain  /= 2;
    }
  }

  gain_regh = gain_regh << 2;

  if (gain >= 2.0) {
      reg_gain = 0xff;
  } else {
      reg_gain = (uint16_t)(gain * 0x80);
  }
  reg_gain = reg_gain | (gain_regh << 8);
  return reg_gain;
}


/*===========================================================================
 * FUNCTION    - ov16825_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov16825_register_to_real_gain(uint16_t reg_gain)
{
  float gain;

  if (reg_gain > 255 )
    reg_gain = 255;

  gain = reg_gain & 0xff;
  gain = gain / 0x80;
  reg_gain = reg_gain >> 8;

  if ((reg_gain & 0x01) == 0x01) {
    gain = gain * 2;
  }
  if ((reg_gain & 0x0c) == 0x0c) {
    gain = gain * 8;
  }
  if ((reg_gain & 0x0c) == 0x08) {
    gain = gain * 4;
  }
  if ((reg_gain & 0x0c) == 0x04) {
    gain = gain * 2;
  }
  return gain;
}


/*===========================================================================
 * FUNCTION    - ov16825_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov16825_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov16825_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = ov16825_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov16825_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov16825_fill_exposure_array(uint16_t gain, uint32_t line,
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

static sensor_exposure_table_t ov16825_expsoure_tbl = {
  .sensor_calculate_exposure = ov16825_calculate_exposure,
  .sensor_fill_exposure_array = ov16825_fill_exposure_array,
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
  .sensor_num_HDR_frame_skip = 1,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 2,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov16825_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov16825_cid_cfg),
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
  .sensor_res_cfg_table = &ov16825_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov16825_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov16825_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov16825_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - ov16825_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov16825_open_lib(void)
{
  return &sensor_lib_ptr;
}
