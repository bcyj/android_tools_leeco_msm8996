/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_OV9724 "ov9724"
#define OV9724_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV9724"_"#n".so"

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
    .seq_val = CAM_VIO,
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
    .delay = 5,
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
    .delay = 5,
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
    .delay = 5,
  },
  {
    .seq_type = SENSOR_I2C_MUX,
    .seq_val = 0,
    .config_val = 0,
    .delay = 30,
  },
};

static struct msm_camera_sensor_slave_info sensor_slave_info = {
  /* Camera slot where this camera is mounted */
  .camera_id = CAMERA_1,
  /* sensor slave address */
  .slave_addr = 0x20,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x0,
    /* sensor id */
    .sensor_id = 0x9724,
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
  .x_output = 0x034c,
  .y_output = 0x034e,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0205,
  .vert_offset = 6,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 15.5,
  .max_linecount = 18240, /* updated in PLD gating */
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
  {0x0307, 0x3C},
};

static struct msm_camera_i2c_reg_array init_reg_array2[] = {

  {0x3210, 0x43},
  {0x3606, 0x75}, /* v06 */
  {0x3705, 0x67},
  {0x0340, 0x02},
  {0x0341, 0xf8},
  {0x0342, 0x06},
  {0x0343, 0x28},
  {0x0202, 0x02},
  {0x0203, 0xf0},
  {0x4801, 0x0f},
  {0x4801, 0x8f},
  {0x4814, 0x2b},
  {0x4307, 0x3a},
  {0x5000, 0x06},
  {0x5001, 0x73},
  {0x0205, 0x3f},
  {0x0205, 0x38},
  {0x0101, 0x01}, /* mirror and flip off */
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 20,
  },
  {
    .reg_setting = init_reg_array1,
    .size = ARRAY_SIZE(init_reg_array1),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 20,
  },
  {
    .reg_setting = init_reg_array2,
    .size = ARRAY_SIZE(init_reg_array2),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 20,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 3,
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
  {0x0104, 0x0},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg ov9724_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov9724_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov9724_cid_cfg),
      .vc_cfg = {
         &ov9724_cid_cfg[0],
         &ov9724_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x1B,
  },
};

static struct sensor_pix_fmt_info_t ov9724_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SGBRG10 },
};

static struct sensor_pix_fmt_info_t ov9724_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov9724_stream_info[] = {
  {1, &ov9724_cid_cfg[0], ov9724_pix_fmt0_fourcc},
  {1, &ov9724_cid_cfg[1], ov9724_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov9724_stream_info_array = {
  .sensor_stream_info = ov9724_stream_info,
  .size = ARRAY_SIZE(ov9724_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  {0x0202, 0x02},
  {0x0203, 0xf0},
};


static struct msm_camera_i2c_reg_setting res_settings[] = {
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
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
  &ov9724_csi_params, /* RES 0*/
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
    .x_output = 0x500, /* 1280 */
    .y_output = 0x2d0, /* 720 */
    .line_length_pclk = 0x628, /* 1576 */
    .frame_length_lines = 0x2f8, /* 760 */
    .vt_pixel_clk = 36000000,
    .op_pixel_clk = 36000000,
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

static sensor_res_cfg_type_t ov9724_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov9724_res_table = {
  .res_cfg_type = ov9724_res_cfg,
  .size = ARRAY_SIZE(ov9724_res_cfg),
};

static struct sensor_lib_chromatix_t ov9724_chromatix[] = {
  {
    .common_chromatix = OV9724_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV9724_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = OV9724_LOAD_CHROMATIX(preview), /* RES0 */
    .camcorder_chromatix = OV9724_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = OV9724_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
};

static struct sensor_lib_chromatix_array ov9724_lib_chromatix_array = {
  .sensor_lib_chromatix = ov9724_chromatix,
  .size = ARRAY_SIZE(ov9724_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov9724_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov9724_real_to_register_gain(float gain)
{

  uint16_t reg_gain;
  uint16_t reg_gain_bits_6_4;
  uint16_t reg_gain_bits_3_0;

  if (gain < 1) {
    /* Clamped to Min analog gain x1.0 */
    /* gain = 1.0 */
    reg_gain = 0x00;
  } else if (gain >= 15.5) {
    /* Clamped to Max analog gain x15.5 */
    /* gain = 15.5 */
    reg_gain = 0x7F;
  } else {
    /* Calculate analog gain register */
    reg_gain_bits_6_4 = 0;
    /* Max gain of 0x0205[3:0] is 1.9375 -> 1+(15/16) */
    while (gain >= 2.0) {
      reg_gain_bits_6_4 = (reg_gain_bits_6_4 << 1) | 1;
      gain /= 2.0;
    }
    reg_gain_bits_3_0 = (uint16_t)((gain - 1.0) * 16.0);
    reg_gain = (reg_gain_bits_6_4 << 4) | reg_gain_bits_3_0;
  }
  return (reg_gain & 0x7F);
}

/*===========================================================================
 * FUNCTION    - ov9724_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov9724_register_to_real_gain(uint16_t reg_gain)
{

  float real_gain;
  uint16_t reg_gain_bit_4, reg_gain_bit_5, reg_gain_bit_6;
  uint16_t reg_gain_bits_3_0;

  reg_gain_bit_6 = (reg_gain & 0x40) >> 6;
  reg_gain_bit_5 = (reg_gain & 0x20) >> 5;
  reg_gain_bit_4 = (reg_gain & 0x10) >> 4;
  reg_gain_bits_3_0 = reg_gain & 0x0F;

  real_gain =
    (float)(reg_gain_bit_6 + 1.0) *
    (float)(reg_gain_bit_5 + 1.0) *
    (float)(reg_gain_bit_4 + 1.0) *
    (float)(1.0 + ((float)reg_gain_bits_3_0 / 16.0)) ;

  return real_gain;
}

/*===========================================================================
 * FUNCTION    - ov9724_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov9724_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov9724_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = ov9724_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov9724_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov9724_fill_exposure_array(uint16_t gain, uint32_t line,
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
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line& 0xFF00) >> 8 ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF);
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
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

static sensor_exposure_table_t ov9724_expsoure_tbl = {
  .sensor_calculate_exposure = ov9724_calculate_exposure,
  .sensor_fill_exposure_array = ov9724_fill_exposure_array,
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
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 2,
  /* sensor exposure table size */
  .exposure_table_size = 7,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov9724_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov9724_cid_cfg),
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
  .sensor_res_cfg_table = &ov9724_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov9724_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov9724_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov9724_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - ov9724_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov9724_open_lib(void)
{
  return &sensor_lib_ptr;
}
