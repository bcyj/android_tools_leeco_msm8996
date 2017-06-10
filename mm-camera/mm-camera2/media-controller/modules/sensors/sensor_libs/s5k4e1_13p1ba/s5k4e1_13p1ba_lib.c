/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_s5k4e1 "S5K4E1_13P1BA"
#define s5k4e1_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_s5k4e1"_"#n".so"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
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
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VANA,
    .config_val = GPIO_OUT_LOW,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VANA,
    .config_val = GPIO_OUT_HIGH,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VAF,
    .config_val = 0,
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
    .seq_val = SENSOR_GPIO_AF_PWDM,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
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
    .sensor_id_reg_addr = 0x00,
    /* sensor id */
    .sensor_id = 0x4e10,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 0,
  .position = 0,
  .sensor_mount_angle = 90,
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
    .vert_offset = 8,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 16.0,
  .max_linecount = 23808,/*preview frame_length_lines * 24 */
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 3.19,
  .pix_size = 1.4,
  .f_number = 2.4,
  .total_f_dist = 1.97,
  .hor_view_angle = 73.6,
  .ver_view_angle = 70.5,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xe4,
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
  .csi_phy_sel = 0,
};
#endif


static struct msm_camera_i2c_reg_array init_reg_array[] = {
  /* Reset setting */
  {0x0103, 0x01},
  /* MIPI settings */
  {0x302E, 0x0B},
  {0x30BD, 0x00},/* SEL_CCP[0] */
  {0x3084, 0x15},/* SYNC Mode */
  {0x30BE, 0x1A},/* M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0] */
  {0x30C1, 0x01},/* pack video enable [0] */
  {0x30EE, 0x02},/* DPHY enable [ 1] */
  {0x3111, 0x86},/* Embedded data off [5] */

  /* REC Settings */
  /*CDS timing setting ... */
  {0x0101,0x03},
  {0x3000,0x05},
  {0x3001,0x03},
  {0x3002,0x08},
  {0x3003,0x09},
  {0x3004,0x2E},
  {0x3005,0x06},
  {0x3006,0x34},
  {0x3007,0x00},
  {0x3008,0x3C},
  {0x3009,0x3C},
  {0x300A,0x28},
  {0x300B,0x04},
  {0x300C,0x0A},
  {0x300D,0x02},
  {0x300E,0xE8},
  {0x300F,0x82},

  /* CDS option setting ... */
  {0x3010, 0x00},
  {0x3011, 0x4C},
  {0x3029, 0xC6},
  {0x3012, 0x30},
  {0x3013, 0xC0},
  {0x3014, 0x00},
  {0x3015, 0x00},
  {0x3016, 0x2C},
  {0x3017, 0x94},
  {0x3018, 0x78},
  {0x301C, 0x04},
  {0x301D, 0xD4},
  {0x3021, 0x02},
  {0x3022, 0x24},
  {0x3024, 0x40},
  {0x3027, 0x08},

  /* Pixel option setting ...   */
  {0x301C, 0x04},
  {0x30D8, 0x3F},
  {0x302B, 0x01},

  {0x3070, 0x5F},
  {0x3071, 0x00},
  {0x3080, 0x04},
  {0x3081, 0x38},

  // PLL setting ...
  // input clock 24MHz
  // (3) MIPI 2-lane Serial(TST = 0000b or TST = 0010b), 15 fps
  {0x0305,0x04},//06////PLL P = 6
  {0x0306,0x00},//PLL M[8] = 0
  {0x0307,0x44},//36//65//PLL M = 101
  {0x30B5,0x01},//01//PLL S = 1
  {0x30E2,0x02},//02//num lanes[1:0] = 2
  {0x30F1,0x70},//70//DPHY BANDCTRL 408MHz=40.8MHz

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

static struct msm_camera_csid_vc_cfg s5k4e1_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params s5k4e1_csi_params = {
  .csid_params = {
     .lane_cnt = 2,
     .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &s5k4e1_cid_cfg[0],
         &s5k4e1_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x14,
  },
  .csi_clk_scale_enable = 1,
};

static struct msm_camera_csi2_params *csi_params[] = {
  &s5k4e1_csi_params, /* RES 0*/
  &s5k4e1_csi_params, /* RES 1*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t s5k4e1_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SGBRG10 },
};

static struct sensor_pix_fmt_info_t s5k4e1_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t s5k4e1_stream_info[] = {
  {1, &s5k4e1_cid_cfg[0], s5k4e1_pix_fmt0_fourcc},
  {1, &s5k4e1_cid_cfg[1], s5k4e1_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t s5k4e1_stream_info_array = {
  .sensor_stream_info = s5k4e1_stream_info,
  .size = ARRAY_SIZE(s5k4e1_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  /*Output Size (2592x1944)*/
  {0x0101, 0x03},
  {0x301B, 0x77},
  {0x30A9, 0x03},/* Horizontal Binning Off */
  {0x300E, 0xE8},/* Vertical Binning Off */
  {0x0387, 0x01},/* y_odd_inc */
  {0x0344, 0x00},/* x_addr_start 8 */
  {0x0345, 0x08},
  {0x0348, 0x0A},/* x_addr_end 2599 */
  {0x0349, 0x27},
  {0x0346, 0x00},/* y_addr_start 8 */
  {0x0347, 0x08},
  {0x034A, 0x07},/* y_addr_end 1951 */
  {0x034B, 0x9F},
  {0x034C, 0x0A},/* x_output size 2592*/
  {0x034D, 0x20},
  {0x034E, 0x07},/* y_output size 1944*/
  {0x034F, 0x98},
  {0x30BF, 0xAB},/* outif_enable[7], data_type[5:0](2Bh = bayer 10bit} */
  {0x30C0, 0x00},/* video_offset[7:4] 3240%12 */
  {0x30C8, 0x0C},/* video_data_length 3240 = 2592 * 1.25 */
  {0x30C9, 0xA8},
  /*Timing configuration*/
  {0x0202, 0x06},
  {0x0203, 0x28},
  {0x0204, 0x00},
  {0x0205, 0x80},
  {0x0340, 0x07},/* Frame Length */
  {0x0341, 0xB4},
  {0x0342, 0x0A},/* 2738 Line Length */
  {0x0343, 0xB2},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  /* Output Size (1296 x 972) */
  {0x0101, 0x03},
  {0x301B, 0x83},
  {0x30A9, 0x02},/* Horizontal Binning On */
  {0x300E, 0xEB},/* Vertical Binning On */
  {0x0344, 0x00},/* x_addr_start 8 */
  {0x0345, 0x08},
  {0x0348, 0x0A},/* x_addr_end 2599 */
  {0x0349, 0x27},
  {0x0346, 0x00},/* y_addr_start 8 */
  {0x0347, 0x08},
  {0x034A, 0x07},/* y_addr_end 1951 */
  {0x034B, 0x9F},
  {0x0380, 0x00},/* x_even_inc 1 */
  {0x0381, 0x01},
  {0x0382, 0x00},/* x_odd_inc 1 */
  {0x0383, 0x01},
  {0x0384, 0x00},/* y_even_inc 1 */
  {0x0385, 0x01},
  {0x0386, 0x00},/* y_odd_inc 3 */
  {0x0387, 0x03},
  {0x034C, 0x05},/* x_output_size 1296 */
  {0x034D, 0x10},
  {0x034E, 0x03},/* y_output_size 972 */
  {0x034F, 0xCC},
  {0x30BF, 0xAB},/* outif_enable[7], data_type[5:0](2Bh = bayer 10bit} */
  {0x30C0, 0x00},/* video_offset[7:4] 1620%12 */
  {0x30C8, 0x06},/* video_data_length 1620 = 1296 * 1.25 */
  {0x30C9, 0x54},
  /* Timing Configuration */
  {0x0202, 0x03},
  {0x0203, 0x14},
  {0x0204, 0x00},
  {0x0205, 0x80},
  {0x0340, 0x03},/* Frame Length */
  {0x0341, 0xD9},
  {0x0342, 0x0A},/* 2738  Line Length */
  {0x0343, 0xB2},
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
    .line_length_pclk = 0xAB2,
    .frame_length_lines = 0x7B4,
    .vt_pixel_clk = 81600000,
    .op_pixel_clk = 81600000,
    .binning_factor = 0,
    .max_fps = 15,
    .min_fps = 5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 1296,
    .y_output = 972,
    .line_length_pclk = 0xAB2,
    .frame_length_lines = 0x3E0,
    .vt_pixel_clk = 80400000,
    .op_pixel_clk = 80400000,
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

static sensor_res_cfg_type_t s5k4e1_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t s5k4e1_res_table = {
  .res_cfg_type = s5k4e1_res_cfg,
  .size = ARRAY_SIZE(s5k4e1_res_cfg),
};

static struct sensor_lib_chromatix_t s5k4e1_chromatix[] = {
  {
    .common_chromatix = s5k4e1_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = s5k4e1_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = s5k4e1_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = s5k4e1_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = s5k4e1_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
  {
    .common_chromatix = s5k4e1_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = s5k4e1_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = s5k4e1_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix = s5k4e1_LOAD_CHROMATIX(preview), /* RES1 */
    .liveshot_chromatix = s5k4e1_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
};

static struct sensor_lib_chromatix_array s5k4e1_lib_chromatix_array = {
  .sensor_lib_chromatix = s5k4e1_chromatix,
  .size = ARRAY_SIZE(s5k4e1_chromatix),
};

/*===========================================================================
 * FUNCTION    - s5k4e1_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t s5k4e1_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if(gain < 1.0) {
      gain = 1.0;
  } else if (gain > 16.0) {
      gain = 16.0;
  }
  gain = (gain) * 32.0;
  reg_gain = (uint16_t) gain;

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - s5k4e1_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float s5k4e1_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  if (reg_gain > 0x200) {
      reg_gain = 0x200;
  }
  real_gain = (float) reg_gain / 32.0;

  return real_gain;
}

/*===========================================================================
 * FUNCTION    - s5k4e1_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t s5k4e1_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = s5k4e1_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = s5k4e1_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - s5k4e1_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t s5k4e1_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
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

  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;

  return rc;
}

static sensor_exposure_table_t s5k4e1_expsoure_tbl = {
  .sensor_calculate_exposure = s5k4e1_calculate_exposure,
  .sensor_fill_exposure_array = s5k4e1_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
   .actuator_name = "dw9714_13p1ba",
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
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 1,
  /* sensor exposure table size */
  .exposure_table_size = 8,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = s5k4e1_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(s5k4e1_cid_cfg),
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
  .sensor_res_cfg_table = &s5k4e1_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &s5k4e1_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &s5k4e1_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &s5k4e1_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - S5K4E1_13P1BA_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *S5K4E1_13P1BA_open_lib(void)
{
  return &sensor_lib_ptr;
}
