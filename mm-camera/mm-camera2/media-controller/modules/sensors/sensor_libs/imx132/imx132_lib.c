/* imx132_lib.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_IMX132 "imx132"
#define IMX132_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_IMX132"_"#n".so"


static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
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
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VIO,
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
    .delay = 5,
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
  .camera_id = CAMERA_1,
  /* sensor slave address */
  .slave_addr = 0x6C,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x00,
    /* sensor id */
    .sensor_id = 0x0132,
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
  .x_output = 0x034C,
  .y_output = 0x034E,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0204,
  .vert_offset = 5,
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
  .sensing_method = SENSOR_SMETHOD_NOT_DEFINED,
  .crop_factor = 1.33, //(4:3) its sensor's physical dimension dependent factor
};

static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x7,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 1,
};

static struct msm_camera_i2c_reg_array init_reg_array[] = {
  {0x3087, 0x53},
  {0x308B, 0x5A},
  {0x3094, 0x11},
  {0x309D, 0xA4},
  {0x30AA, 0x01},
  {0x30C6, 0x00},
  {0x30C7, 0x00},
  {0x3118, 0x2F},
  {0x312A, 0x00},
  {0x312B, 0x0B},
  {0x312C, 0x0B},
  {0x312D, 0x13},
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
  {0x104, 0x01},
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x104, 0x00},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg imx132_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
};

static struct msm_camera_csi2_params imx132_csi_params = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = ARRAY_SIZE(imx132_cid_cfg),
      .vc_cfg = {
         &imx132_cid_cfg[0],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x1B,
  },
};

static struct sensor_pix_fmt_info_t imx132_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SRGGB10 },
};

static sensor_stream_info_t imx132_stream_info[] = {
  {1, &imx132_cid_cfg[0], imx132_pix_fmt0_fourcc},
};

static sensor_stream_info_array_t imx132_stream_info_array = {
  .sensor_stream_info = imx132_stream_info,
  .size = ARRAY_SIZE(imx132_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = { //full settings 1976x1200@30fps
  /*PLL Setting*/
  {0x0305, 0x02},
  {0x0307, 0x3C},
  {0x30A4, 0x02},
  {0x303C, 0x4B},
  /*Mode Setting*/
  {0x0340, 0x08},
  {0x0341, 0x56},
  {0x0342, 0x08},
  {0x0343, 0xC8},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x07},
  {0x0349, 0xB7},
  {0x034A, 0x04},
  {0x034B, 0xAF},
  {0x034C, 0x07},
  {0x034D, 0xB8},
  {0x034E, 0x04},
  {0x034F, 0xB0},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x303D, 0x10},
  {0x303E, 0x4A},
  {0x3040, 0x08},
  {0x3041, 0x97},
  {0x3048, 0x00},
  {0x304C, 0x2F},
  {0x304D, 0x02},
  {0x3064, 0x92},
  {0x306A, 0x10},
  {0x309B, 0x00},
  {0x309E, 0x41},
  {0x30A0, 0x10},
  {0x30A1, 0x0B},
  {0x30B2, 0x00},
  {0x30D5, 0x00},
  {0x30D6, 0x00},
  {0x30D7, 0x00},
  {0x30D8, 0x00},
  {0x30D9, 0x00},
  {0x30DA, 0x00},
  {0x30DB, 0x00},
  {0x30DC, 0x00},
  {0x30DD, 0x00},
  {0x30DE, 0x00},
  {0x3102, 0x0C},
  {0x3103, 0x33},
  {0x3104, 0x30},
  {0x3105, 0x00},
  {0x3106, 0xCA},
  {0x3107, 0x00},
  {0x3108, 0x06},
  {0x3109, 0x04},
  {0x310A, 0x04},
  {0x315C, 0x3D},
  {0x315D, 0x3C},
  {0x316E, 0x3E},
  {0x316F, 0x3D},
  {0x3301, 0x00},
  {0x3304, 0x07},
  {0x3305, 0x06},
  {0x3306, 0x19},
  {0x3307, 0x03},
  {0x3308, 0x0F},
  {0x3309, 0x07},
  {0x330A, 0x0C},
  {0x330B, 0x06},
  {0x330C, 0x0B},
  {0x330D, 0x07},
  {0x330E, 0x03},
  {0x3318, 0x62},
  {0x3322, 0x09},
  {0x3342, 0x00},
  {0x3348, 0xE0},
  /*Shutter Gain Setting*/
  {0x0202, 0x08},
  {0x0203, 0xAB},
  {0x020E, 0x01},
  {0x020F, 0x00},
  {0x0210, 0x01},
  {0x0211, 0x00},
  {0x0212, 0x01},
  {0x0213, 0x00},
  {0x0214, 0x01},
  {0x0215, 0x00},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = { //preview settings
  /*PLL Setting*/
  {0x0305, 0x04},
  {0x0307, 0x7D},
  {0x30A4, 0x02},
  {0x303C, 0x4B},
  /*Mode Setting*/
  {0x0340, 0x08},
  {0x0341, 0xB0},
  {0x0342, 0x08},
  {0x0343, 0xC8},
  {0x0344, 0x00},
  {0x0345, 0x1C},
  {0x0346, 0x00},
  {0x0347, 0x3C},
  {0x0348, 0x07},
  {0x0349, 0x9B},
  {0x034A, 0x04},
  {0x034B, 0x73},
  {0x034C, 0x07},
  {0x034D, 0x80},
  {0x034E, 0x04},
  {0x034F, 0x38},
  {0x0381, 0x01},
  {0x0383, 0x01},
  {0x0385, 0x01},
  {0x0387, 0x01},
  {0x303D, 0x10},
  {0x303E, 0x4A},
  {0x3040, 0x08},
  {0x3041, 0x97},
  {0x3048, 0x00},
  {0x304C, 0x2F},
  {0x304D, 0x02},
  {0x3064, 0x92},
  {0x306A, 0x10},
  {0x309B, 0x00},
  {0x309E, 0x41},
  {0x30A0, 0x10},
  {0x30A1, 0x0B},
  {0x30B2, 0x00},
  {0x30D5, 0x00},
  {0x30D6, 0x00},
  {0x30D7, 0x00},
  {0x30D8, 0x00},
  {0x30D9, 0x00},
  {0x30DA, 0x00},
  {0x30DB, 0x00},
  {0x30DC, 0x00},
  {0x30DD, 0x00},
  {0x30DE, 0x00},
  {0x3102, 0x0C},
  {0x3103, 0x33},
  {0x3104, 0x30},
  {0x3105, 0x00},
  {0x3106, 0xCA},
  {0x3107, 0x00},
  {0x3108, 0x06},
  {0x3109, 0x04},
  {0x310A, 0x04},
  {0x315C, 0x3D},
  {0x315D, 0x3C},
  {0x316E, 0x3E},
  {0x316F, 0x3D},
  {0x3301, 0x00},
  {0x3304, 0x07},
  {0x3305, 0x06},
  {0x3306, 0x19},
  {0x3307, 0x03},
  {0x3308, 0x0F},
  {0x3309, 0x07},
  {0x330A, 0x0C},
  {0x330B, 0x06},
  {0x330C, 0x0B},
  {0x330D, 0x07},
  {0x330E, 0x03},
  {0x3318, 0x62},
  {0x3322, 0x09},
  {0x3342, 0x00},
  {0x3348, 0xE0},
  /*Shutter Gain Setting*/
  {0x0202, 0x08},
  {0x0203, 0xAB},
  {0x020E, 0x01},
  {0x020F, 0x00},
  {0x0210, 0x01},
  {0x0211, 0x00},
  {0x0212, 0x01},
  {0x0213, 0x00},
  {0x0214, 0x01},
  {0x0215, 0x00},
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

static struct msm_camera_csi2_params *csi_params[] = {
  &imx132_csi_params, /* RES 0*/
  &imx132_csi_params, /* RES 1*/
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
    .x_output = 0x7b8, /* 1976 */
    .y_output = 0x4b0, /* 1200 */
    .line_length_pclk = 0x8c8, /* 2248 */
    .frame_length_lines = 0x856, /* 2134 */
    .vt_pixel_clk = 144000000,
    .op_pixel_clk = 144000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
  },
  {
    .x_output = 0x780, /* 1920 */
    .y_output = 0x438, /* 1080 */
    .line_length_pclk = 0x8c8, /* 2248 */
    .frame_length_lines = 0x8b0, /* 2224 */
    .vt_pixel_clk = 150000000,
    .op_pixel_clk = 150000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t imx132_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t imx132_res_table = {
  .res_cfg_type = imx132_res_cfg,
  .size = ARRAY_SIZE(imx132_res_cfg),
};

static struct sensor_lib_chromatix_t imx132_chromatix[] = {
  {
    .common_chromatix = IMX132_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX132_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = IMX132_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = IMX132_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = IMX132_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
  {
    .common_chromatix = IMX132_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX132_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = IMX132_LOAD_CHROMATIX(snapshot), /* RES1 */
    .camcorder_chromatix = IMX132_LOAD_CHROMATIX(default_video), /* RES1 */
    .liveshot_chromatix = IMX132_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
};

static struct sensor_lib_chromatix_array imx132_lib_chromatix_array = {
  .sensor_lib_chromatix = imx132_chromatix,
  .size = ARRAY_SIZE(imx132_chromatix),
};

/*===========================================================================
 * FUNCTION    - imx132_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t imx132_real_to_register_gain(float gain)
{
  uint16_t reg_gain;
  if (gain < 1.0)
    gain = 1.0;
  if (gain > 8.0)
    gain = 8.0;
  reg_gain = (uint16_t)(256.0 - 256.0 / gain);

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - imx132_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float imx132_register_to_real_gain(uint16_t reg_gain)
{
  float gain;
  if (reg_gain > 240)
    reg_gain = 240;
  gain = 256.0 / (256.0 - reg_gain);
  return gain;
}

/*===========================================================================
 * FUNCTION    - imx132_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx132_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = imx132_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = imx132_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - imx132_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx132_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting* reg_setting) {

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
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF00) >> 8;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_count = reg_count + 1;
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

static sensor_exposure_table_t imx132_expsoure_tbl = {
  .sensor_calculate_exposure = imx132_calculate_exposure,
  .sensor_fill_exposure_array = imx132_fill_exposure_array,
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
  /* sensor exposure table size */
  .exposure_table_size = 8,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = imx132_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(imx132_cid_cfg),
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
  .sensor_res_cfg_table = &imx132_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &imx132_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &imx132_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &imx132_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - imx132_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *imx132_open_lib(void)
{
  return &sensor_lib_ptr;
}
