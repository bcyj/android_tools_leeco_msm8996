/*============================================================================

  Copyright (c)2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_IMX214 "imx214"
#define IMX214_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_IMX214"_"#n".so"

#define SNAPSHOT_PARAMS  1
#define PREVIEW_PARAMS   1

#define ABS_GAIN_R_WORD_ADDR        0x0b90
#define ABS_GAIN_B_WORD_ADDR        0x0b92
#define EXPO_RATIO_ADDR             0x0222

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
    .seq_val = SENSOR_GPIO_AF_PWDM,
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
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_AF_PWDM,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
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
  .slave_addr = 0x20,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x0016,
    /* sensor id */
    .sensor_id = 0x0214,
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
  .sensor_mount_angle = 270,
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
  .global_gain_addr = 0x0204,
  .vert_offset = 10,
};

static sensor_manual_exposure_info_t manual_exp_info = {
  .min_exposure_time = 10322,/*in nano sec = 1line*/
  .max_exposure_time = 86000000000,/*in nano sec = FFFF lines*/
  .min_iso = 100,
  .max_iso = 800,
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
  .near_end_distance = 10,/*in cm*/
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xE4,
  .csi_lane_mask = 0xF,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x1F,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0101, 0x00},
  {0x0105, 0x01},
  {0x0106, 0x01},
  {0x0136, 0x18},
  {0x0137, 0x00},
  {0x30B3, 0x01},
  {0x4601, 0x04},
  {0x4642, 0x01},
  {0x6276, 0x00},
  {0x900E, 0x06},
  {0xA802, 0x90},
  {0xA803, 0x11},
  {0xA804, 0x62},
  {0xA805, 0x77},
  {0xA806, 0xAE},
  {0xA807, 0x34},
  {0xA808, 0xAE},
  {0xA809, 0x35},
  {0xA80A, 0x62},
  {0xA80B, 0x83},
  {0xAE33, 0x00},
  {0x4174, 0x00},
  {0x4175, 0x11},
  {0x4612, 0x29},
  {0x461B, 0x1C},
  {0x461F, 0x06},
  {0x4635, 0x07},
  {0x4637, 0x30},
  {0x463F, 0x18},
  {0x4641, 0x0D},
  {0x465B, 0x2C},
  {0x465F, 0x2B},
  {0x4663, 0x2B},
  {0x4667, 0x24},
  {0x466F, 0x24},
  {0x470E, 0x09},
  {0x4909, 0xAB},
  {0x490B, 0x95},
  {0x4915, 0x5D},
  {0x4A5F, 0xFF},
  {0x4A61, 0xFF},
  {0x4A73, 0x62},
  {0x4A85, 0x00},
  {0x4A87, 0xFF},
  {0x583C, 0x04},
  {0x620E, 0x04},
  {0x6603, 0x01},
  {0x6EB2, 0x01},
  {0x6EB3, 0x00},
  {0x9300, 0x02},
  // video hdr image quality
  {0x3001, 0x07},
  {0x6d12, 0x03},
  {0x6d13, 0xff},
  {0x9344, 0x03},
  {0x9708, 0x03},
  {0x9e04, 0x01},
  {0x9e05, 0x00},
  {0x9e0c, 0x01},
  {0x9e0d, 0x02},
  {0x69d8, 0x01},
  {0x6957, 0x01},
  {0x6987, 0x17},
  {0x698a, 0x03},
  {0x698b, 0x03},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
   {
   .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 20,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size =ARRAY_SIZE(init_reg_setting),
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
  {0x0104, 0x01},
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x0104, 0x00},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg imx214_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, 0x35, CSI_DECODE_8BIT },
  {2, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx214_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = ARRAY_SIZE(imx214_cid_cfg),
      .vc_cfg = {
         &imx214_cid_cfg[0],
         &imx214_cid_cfg[1],
         &imx214_cid_cfg[2],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 0x1b,
  },
};

static struct msm_camera_csi2_params *csi_params[] = {
  &imx214_csi_params, /* RES 0*/
  &imx214_csi_params, /* RES 1*/
  &imx214_csi_params, /* RES 2*/
  &imx214_csi_params, /* RES 3*/
  &imx214_csi_params, /* RES 4*/
  &imx214_csi_params, /* RES 5*/
  &imx214_csi_params, /* RES 6*/
  &imx214_csi_params, /* RES 7*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t imx214_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SRGGB10 },
  { MSM_V4L2_PIX_FMT_META },
};

static struct sensor_pix_fmt_info_t imx214_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t imx214_stream_info[] = {
  {2, &imx214_cid_cfg[0], imx214_pix_fmt0_fourcc},
  {1, &imx214_cid_cfg[1], imx214_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t imx214_stream_info_array = {
  .sensor_stream_info = imx214_stream_info,
  .size = ARRAY_SIZE(imx214_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x0C},
  {0x0341, 0x7A},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0C},
  {0x034B, 0x2F},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x00},
  {0x0901, 0x00},
  {0x0902, 0x00},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x10},
  {0x034D, 0x70},
  {0x034E, 0x0C},
  {0x034F, 0x30},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x10},
  {0x040D, 0x70},
  {0x040E, 0x0C},
  {0x040F, 0x30},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0xC8},
  {0x0309, 0x0A},
  {0x030B, 0x01},
  {0x0310, 0x00},
  {0x0820, 0x12},
  {0x0821, 0xc0},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x09},
  {0x3A04, 0x10},
  {0x3A05, 0x02},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0xFF},
  {0x9D00, 0x01},
  {0x9D01, 0x01},
  {0x9D02, 0x01},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x60},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x00},
  {0x0202, 0x0C},
  {0x0203, 0x70},
  {0x0224, 0x01},
  {0x0225, 0xF4},
  {0x4170, 0x00},
  {0x4171, 0x10},
  {0x4176, 0x00},
  {0x4177, 0x3C},
  {0xAE20, 0x04},
  {0xAE21, 0x5C},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x06},
  {0x0341, 0x4C},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0C},
  {0x034B, 0x2F},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x01},
  {0x0901, 0x22},
  {0x0902, 0x02},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x08},
  {0x034D, 0x38},
  {0x034E, 0x06},
  {0x034F, 0x18},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x08},
  {0x040D, 0x38},
  {0x040E, 0x06},
  {0x040F, 0x18},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0x65},
  {0x0309, 0x0A},
  {0x030B, 0x01},
  {0x0310, 0x00},
  {0x0820, 0x09},
  {0x0821, 0x78},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x06},
  {0x3A04, 0x88},
  {0x3A05, 0x01},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0xFF},
  {0x9D00, 0x01},
  {0x9D01, 0x01},
  {0x9D02, 0x01},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x60},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x00},
  {0x0202, 0x06},
  {0x0203, 0x42},
  {0x0224, 0x01},
  {0x0225, 0xF4},
  {0x4170, 0x00},
  {0x4171, 0x10},
  {0x4176, 0x00},
  {0x4177, 0x3C},
  {0xAE20, 0x04},
  {0xAE21, 0x5C},
};

static struct msm_camera_i2c_reg_array res2_reg_array[] = {
  /* 2104x1184 at 30 fps video*/
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x04},
  {0x0341, 0xFC},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x01},
  {0x0347, 0x78},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0A},
  {0x034B, 0xB7},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x01},
  {0x0901, 0x22},
  {0x0902, 0x02},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  /* Address value */
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x08},
  {0x034D, 0x38},
  {0x034E, 0x04},
  {0x034F, 0xA0},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x08},
  {0x040D, 0x38},
  {0x040E, 0x04},
  {0x040F, 0xA0},
  /* Address value */
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x03},
  {0x0306, 0x00},
  {0x0307, 0x3C},
  {0x0309, 0x0A},
  {0x030B, 0x01},
  {0x0310, 0x00},
  /* Address value */
  {0x0820, 0x07},
  {0x0821, 0x80},
  {0x0822, 0x00},
  {0x0823, 0x00},
  /* Address value */
  {0x3A03, 0x06},
  {0x3A04, 0x68},
  {0x3A05, 0x01},
  /* Address value */
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  /* Address value */
  {0x30B4, 0x00},
  /* Address value */
  {0x3A02, 0xFF},
  /* Address value */
  {0x3011, 0x00},
  {0x3013, 0x01},
  /* Address value */
  {0x0202, 0x04},
  {0x0203, 0xF2},
  {0x0224, 0x01},
  {0x0225, 0xF4},
  /* Address value */
  {0x0204, 0x00},
  {0x0205, 0x00},
  {0x020E, 0x01},
  {0x020F, 0x00},
  {0x0210, 0x01},
  {0x0211, 0x00},
  {0x0212, 0x01},
  {0x0213, 0x00},
  {0x0214, 0x01},
  {0x0215, 0x00},
  {0x0216, 0x00},
  {0x0217, 0x00},
  /* Address value */
  {0x4170, 0x00},
  {0x4171, 0x10},
  {0x4176, 0x00},
  {0x4177, 0x3C},
  {0xAE20, 0x04},
  {0xAE21, 0x5C},
};

static struct msm_camera_i2c_reg_array res3_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x04},
  {0x0341, 0xFC},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x01},
  {0x0347, 0x78},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0A},
  {0x034B, 0xB7},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x01},
  {0x0901, 0x22},
  {0x0902, 0x02},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x08},
  {0x034D, 0x38},
  {0x034E, 0x04},
  {0x034F, 0xA0},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x08},
  {0x040D, 0x38},
  {0x040E, 0x04},
  {0x040F, 0xA0},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0xA0},
  {0x0309, 0x0A},
  {0x030B, 0x01},
  {0x0310, 0x00},
  {0x0820, 0x0F},
  {0x0821, 0x00},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x06},
  {0x3A04, 0x88},
  {0x3A05, 0x01},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0xFF},
  {0x9D00, 0x01},
  {0x9D01, 0x01},
  {0x9D02, 0x01},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x60},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x00},
  {0x0202, 0x04},
  {0x0203, 0xF2},
  {0x0224, 0x01},
  {0x0225, 0xF4},
  {0x4170, 0x00},
  {0x4171, 0x10},
  {0x4176, 0x00},
  {0x4177, 0x3C},
  {0xAE20, 0x04},
  {0xAE21, 0x5C},
};

static struct msm_camera_i2c_reg_array res4_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x03},
  {0x0341, 0x54},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0C},
  {0x034B, 0x2F},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x01},
  {0x0901, 0x44},
  {0x0902, 0x02},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x04},
  {0x034D, 0x1C},
  {0x034E, 0x03},
  {0x034F, 0x0C},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x04},
  {0x040D, 0x1C},
  {0x040E, 0x03},
  {0x040F, 0x0C},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0xA0},
  {0x0309, 0x0A},
  {0x030B, 0x02},
  {0x0310, 0x00},
  {0x0820, 0x07},
  {0x0821, 0x80},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x03},
  {0x3A04, 0x50},
  {0x3A05, 0x02},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0xFF},
  {0x9D00, 0x01},
  {0x9D01, 0x01},
  {0x9D02, 0x01},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x60},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x00},
  {0x0202, 0x03},
  {0x0203, 0x4A},
  {0x0224, 0x01},
  {0x0225, 0xF4},
  {0x4170, 0x00},
  {0x4171, 0x07},
  {0x4176, 0x00},
  {0x4177, 0x3D},
  {0xAE20, 0x04},
  {0xAE21, 0x38},
};

static struct msm_camera_i2c_reg_array res5_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x03},
  {0x0341, 0x20},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x28},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0C},
  {0x034B, 0x07},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x01},
  {0x0901, 0x44},
  {0x0902, 0x02},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x04},
  {0x034D, 0x1C},
  {0x034E, 0x02},
  {0x034F, 0xF8},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x04},
  {0x040D, 0x1C},
  {0x040E, 0x02},
  {0x040F, 0xF8},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0xC8},
  {0x0309, 0x0A},
  {0x030B, 0x02},
  {0x0310, 0x00},
  {0x0820, 0x09},
  {0x0821, 0x60},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x03},
  {0x3A04, 0x60},
  {0x3A05, 0x02},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0xFF},
  {0x9D00, 0x01},
  {0x9D01, 0x01},
  {0x9D02, 0x01},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x60},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x00},
  {0x0202, 0x03},
  {0x0203, 0x16},
  {0x0224, 0x01},
  {0x0225, 0xF4},
  {0x4170, 0x00},
  {0x4171, 0x07},
  {0x4176, 0x00},
  {0x4177, 0x3D},
  {0xAE20, 0x04},
  {0xAE21, 0x38},
};

static struct msm_camera_i2c_reg_array res6_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x01},
  {0x0221, 0x22},
  {0x0222, 0x08},
  {0x0340, 0x06},
  {0x0341, 0x4C},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0C},
  {0x034B, 0x2F},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x00},
  {0x0901, 0x00},
  {0x0902, 0x00},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x08},
  {0x034D, 0x38},
  {0x034E, 0x06},
  {0x034F, 0x18},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x08},
  {0x040D, 0x38},
  {0x040E, 0x06},
  {0x040F, 0x18},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0x65},
  {0x0309, 0x0A},
  {0x030B, 0x01},
  {0x0310, 0x00},
  {0x0820, 0x09},
  {0x0821, 0x78},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x06},
  {0x3A04, 0x48},
  {0x3A05, 0x04},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0x06},
  {0x9D00, 0x17},
  {0x9D01, 0x17},
  {0x9D02, 0x17},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x05},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x01},
  {0x0202, 0x06},
  {0x0203, 0x42},
  {0x0224, 0x00},
  {0x0225, 0xC8},
  {0x4170, 0x00},
  {0x4171, 0x10},
  {0x4176, 0x00},
  {0x4177, 0x3C},
  {0xAE20, 0x04},
  {0xAE21, 0x5C},
};

static struct msm_camera_i2c_reg_array res7_reg_array[] = {
  {0x0114, 0x03},
  {0x0220, 0x00},
  {0x0221, 0x11},
  {0x0222, 0x01},
  {0x0340, 0x0C},
  {0x0341, 0x7A},
  {0x0342, 0x13},
  {0x0343, 0x90},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x10},
  {0x0349, 0x6F},
  {0x034A, 0x0C},
  {0x034B, 0x2F},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x0900, 0x00},
  {0x0901, 0x00},
  {0x0902, 0x00},
  {0x3000, 0x35},
  {0x3054, 0x01},
  {0x305C, 0x11},
  {0x0112, 0x0A},
  {0x0113, 0x0A},
  {0x034C, 0x10},
  {0x034D, 0x70},
  {0x034E, 0x0C},
  {0x034F, 0x30},
  {0x0401, 0x00},
  {0x0404, 0x00},
  {0x0405, 0x10},
  {0x0408, 0x00},
  {0x0409, 0x00},
  {0x040A, 0x00},
  {0x040B, 0x00},
  {0x040C, 0x10},
  {0x040D, 0x70},
  {0x040E, 0x0C},
  {0x040F, 0x30},
  {0x0301, 0x05},
  {0x0303, 0x02},
  {0x0305, 0x04},
  {0x0306, 0x00},
  {0x0307, 0xC8},
  {0x0309, 0x0A},
  {0x030B, 0x01},
  {0x0310, 0x00},
  {0x0820, 0x12},
  {0x0821, 0xc0},
  {0x0822, 0x00},
  {0x0823, 0x00},
  {0x3A03, 0x08},
  {0x3A04, 0xb0},
  {0x3A05, 0x01},
  {0x0B06, 0x01},
  {0x30A2, 0x00},
  {0x30B4, 0x00},
  {0x3A02, 0x06},
  {0x9D00, 0x17},
  {0x9D01, 0x17},
  {0x9D02, 0x17},
  {0x9D03, 0x0F},
  {0x9D04, 0x0F},
  {0x9D05, 0x0F},
  {0x9D06, 0x05},
  {0x9D07, 0x60},
  {0x9D08, 0x60},
  {0x3013, 0x01},
  {0x0202, 0x0C},
  {0x0203, 0x70},
  {0x0224, 0x01},
  {0x0225, 0x8E},
  {0x4170, 0x00},
  {0x4171, 0x10},
  {0x4176, 0x00},
  {0x4177, 0x3C},
  {0xAE20, 0x04},
  {0xAE21, 0x5C},
};

static struct msm_camera_i2c_reg_setting res_settings[] = {
#if SNAPSHOT_PARAMS
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
#endif
#if PREVIEW_PARAMS
  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
#endif
  {
    .reg_setting = res2_reg_array,
    .size = ARRAY_SIZE(res2_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
  {
    .reg_setting = res3_reg_array,
    .size = ARRAY_SIZE(res3_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
  {
    .reg_setting = res4_reg_array,
    .size = ARRAY_SIZE(res4_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
  {
    .reg_setting = res5_reg_array,
    .size = ARRAY_SIZE(res5_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
  {
    .reg_setting = res6_reg_array,
    .size = ARRAY_SIZE(res6_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
  },
  {
    .reg_setting = res7_reg_array,
    .size = ARRAY_SIZE(res7_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 10,
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
  {0, 0, 0, 0}, /* RES 6 */
  {0, 0, 0, 0}, /* RES 7 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
#if SNAPSHOT_PARAMS
  {/* 25 fps full size settings */
    .x_output = 4208,
    .y_output = 3120,
    .line_length_pclk = 5008,
    .frame_length_lines = 3194,
    .vt_pixel_clk = 480000000,
    .op_pixel_clk = 480000000,
    .binning_factor = 1,
    .max_fps = 30.33,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if PREVIEW_PARAMS
  {/* 30 fps qtr size settings */
    .x_output = 2104,
    .y_output = 1560,
    .line_length_pclk = 5008,
    .frame_length_lines = 1612,
    .vt_pixel_clk = 242400000,
    .op_pixel_clk = 242400000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
  {
  /* 1080P @ 30 fps*/
   .x_output = 2104,
   .y_output = 1184,
   .line_length_pclk = 5008,
   .frame_length_lines = 1276,
   .vt_pixel_clk = 192000000,
   .op_pixel_clk = 192000000,
   .binning_factor = 1,
   .max_fps = 30.00,
   .min_fps = 7.5,
   .mode = SENSOR_DEFAULT_MODE,
  },
  {/* 60 fps settings */
    .x_output = 2104,
    .y_output = 1184,
    .line_length_pclk = 5008,
    .frame_length_lines = 1276,
    .vt_pixel_clk = 384000000,
    .op_pixel_clk = 384000000,
    .binning_factor = 1,
    .max_fps = 60.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {/* 90 fps settings */
    .x_output = 1052,
    .y_output = 780,
    .line_length_pclk = 5008,
    .frame_length_lines = 852,
    .vt_pixel_clk = 384000000,
    .op_pixel_clk = 192000000,
    .binning_factor = 1,
    .max_fps = 90.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {/* 120 fps settings */
    .x_output = 1052,
    .y_output = 760,
    .line_length_pclk = 5008,
    .frame_length_lines = 800,
    .vt_pixel_clk = 480000000,
    .op_pixel_clk = 240000000,
    .binning_factor = 1,
    .max_fps = 120.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {/* 30 fps settings */
    .x_output = 2104,
    .y_output = 1560,
    .line_length_pclk = 5008,
    .frame_length_lines = 1612,
    .vt_pixel_clk = 242400000,
    .op_pixel_clk = 242400000,
    .binning_factor = 1,
    .max_fps = 30.02,
    .min_fps = 7.5,
    .mode = SENSOR_HDR_MODE,
  },
  {/* 30 fps full size settings */
    .x_output = 4208,
    .y_output = 3120,
    .line_length_pclk = 5008,
    .frame_length_lines = 3194,
    .vt_pixel_clk = 480000000,
    .op_pixel_clk = 480000000,
    .binning_factor = 1,
    .max_fps = 30.33,
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
    .width  = 4208,
    .height = 2,
    .stats_type = YYYY,
  },
};

static struct sensor_lib_meta_data_info_array meta_data_out_info_array = {
  .meta_data_out_info = sensor_meta_data_out_info,
  .size = ARRAY_SIZE(sensor_meta_data_out_info),
};

static sensor_res_cfg_type_t imx214_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t imx214_res_table = {
  .res_cfg_type = imx214_res_cfg,
  .size = ARRAY_SIZE(imx214_res_cfg),
};

static struct sensor_lib_chromatix_t imx214_chromatix[] = {
#if SNAPSHOT_PARAMS
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
#endif
#if PREVIEW_PARAMS
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(default_video), /* RES1 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
#endif
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(preview), /* RES2 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(preview), /* RES2 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(video_1080p), /* RES2 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(liveshot), /* RES2 */
  },
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
  },
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
  },
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(hfr_120fps), /* RES5 */
  },
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(video_hdr), /* RES6 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(video_hdr), /* RES6 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(video_hdr), /* RES6 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(video_hdr), /* RES6 */
  },
  {
    .common_chromatix = IMX214_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX214_LOAD_CHROMATIX(snapshot_hdr), /* RES7 */
    .camera_snapshot_chromatix = IMX214_LOAD_CHROMATIX(snapshot_hdr), /* RES7 */
    .camcorder_chromatix = IMX214_LOAD_CHROMATIX(snapshot_hdr), /* RES7 */
    .liveshot_chromatix = IMX214_LOAD_CHROMATIX(snapshot_hdr), /* RES7 */
  },
};

static struct sensor_lib_chromatix_array imx214_lib_chromatix_array = {
  .sensor_lib_chromatix = imx214_chromatix,
  .size = ARRAY_SIZE(imx214_chromatix),
};

/*===========================================================================
 * FUNCTION    - imx214_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t imx214_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if (gain < 1.0)
    gain = 1.0;
  if (gain > 8.0)
    gain = 8.0;
  reg_gain = (uint16_t)(512.0 - 512.0 / gain);

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - imx214_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float imx214_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  if (reg_gain > 448)
    reg_gain = 448;

  real_gain = 512.0 /(512.0 - reg_gain);

  return real_gain;
}

/*===========================================================================
 * FUNCTION    - imx214_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx214_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = imx214_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = imx214_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - imx214_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx214_fill_exposure_array(uint16_t gain,
  uint32_t line, uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting *reg_setting)
{
  int32_t rc = 0;
  uint16_t reg_count = 0;
  uint16_t i = 0;
  uint16_t longexposure_times = 0;

  if (!reg_setting) {
    return -1;
  }

  while(line > aec_info.max_linecount){
    line = line/2;
    longexposure_times++;
  }

  if (fl_lines > aec_info.max_linecount)
    fl_lines = aec_info.max_linecount + exp_gain_info.vert_offset;
/* HDR control */
  if(fgain != 0) {

    uint32_t hdr_indoor_detected = (fgain >> 16) & 0x1;
    uint32_t ratio = 8;

    if(hdr_indoor_detected)
      ratio = 2;
    else
      ratio = 8;

    reg_setting->reg_setting[reg_count].reg_addr = EXPO_RATIO_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = ratio;
    reg_count = reg_count + 1;
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
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF00) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF);
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

  if(longexposure_times != 0){
    reg_setting->reg_setting[reg_count].reg_addr = 0x350;
    reg_setting->reg_setting[reg_count].reg_data = 0x01;
    reg_count = reg_count + 1;
    reg_setting->reg_setting[reg_count].reg_addr = 0x3028;
    reg_setting->reg_setting[reg_count].reg_data = (longexposure_times & 0x07);
    reg_count = reg_count + 1;
  }else {
    reg_setting->reg_setting[reg_count].reg_addr = 0x350;
    reg_setting->reg_setting[reg_count].reg_data = 0x0;
    reg_count = reg_count + 1;
    reg_setting->reg_setting[reg_count].reg_addr = 0x3028;
    reg_setting->reg_setting[reg_count].reg_data = 0x0;
    reg_count = reg_count + 1;
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;

  return rc;
}

static sensor_exposure_table_t imx214_expsoure_tbl = {
  .sensor_calculate_exposure = imx214_calculate_exposure,
  .sensor_fill_exposure_array = imx214_fill_exposure_array,
};


static int32_t imx214_fill_awb_hdr_array(uint16_t awb_gain_r,
  uint16_t awb_gain_b, struct msm_camera_i2c_seq_reg_setting* reg_setting) {

  uint16_t reg_count = 0;
  uint16_t i = 0;

  reg_setting->reg_setting[reg_count].reg_addr = ABS_GAIN_R_WORD_ADDR;
  reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_r & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_r & 0xFF);
  reg_setting->reg_setting[reg_count].reg_data_size = 2;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr = ABS_GAIN_B_WORD_ADDR;
  reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_b & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_b & 0xFF);
  reg_setting->reg_setting[reg_count].reg_data_size = 2;
  reg_count = reg_count + 1;

  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->delay = 0;

  return 0;
}

static sensor_video_hdr_table_t imx214_video_hdr_tbl = {
  .sensor_fill_awb_array = imx214_fill_awb_hdr_array,
  .awb_table_size = 2,
  .video_hdr_capability = (1<<8) | (1<<20),
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
  .actuator_name = "dw9714_q13n04a",
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
  .sensor_num_frame_skip = 1,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 2,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 1,
  /* sensor exposure table size */
  .exposure_table_size = 20,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = imx214_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(imx214_cid_cfg),
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
  .sensor_res_cfg_table = &imx214_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &imx214_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &imx214_expsoure_tbl,
  /* video hdr func table */
  .video_hdr_awb_lsc_func_table = &imx214_video_hdr_tbl,
  /* chromatix array */
  .chromatix_array = &imx214_lib_chromatix_array,
  /* meta data info */
  .meta_data_out_info_array = &meta_data_out_info_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - imx214_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *imx214_open_lib(void)
{
  return &sensor_lib_ptr;
}

