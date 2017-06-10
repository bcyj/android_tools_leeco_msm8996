/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_GC0339 "skuab_shinetech_gc0339"
#define GC0339_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_GC0339"_"#n".so"


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
    .seq_val = CAM_VDIG,
    .config_val = 0,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_VREG,
    .config_val = 0,
    .seq_val = CAM_VANA,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_HIGH,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 24000000,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_LOW,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_HIGH,
    .delay = 1,
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
    .delay = 1,
  },
};

static struct msm_camera_sensor_slave_info sensor_slave_info = {
  /* Camera slot where this camera is mounted */
  .camera_id = CAMERA_1,
  /* sensor slave address */
  .slave_addr = 0x42,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x0,
    /* sensor id */
    .sensor_id = 0xc8,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 1,
  .position = 1,
  .sensor_mount_angle = 180,
};

static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_8_BIT_DIRECT,
};

static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0x01,
  .y_output = 0x02,
  .line_length_pclk = 0x0b,
  .frame_length_lines = 0x09,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x51,
  .global_gain_addr = 0x03,
  .vert_offset = 4,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 4.0,
  .max_linecount = 4095,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.93,
  .pix_size = 2.5,
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
  .csid_core = {1},
  .csi_phy_sel = 1,
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
{0xfc,0x10},
{0xfe,0x00},
{0xf6,0x07},
{0xf7,0x01},
{0xf7,0x03},
{0xfc,0x16},
{0x06,0x00},
{0x08,0x04},
{0x09,0x01},
{0x0a,0xe8},
{0x0b,0x02},
{0x0c,0x88},
{0x0f,0x02},
{0x14,0x23},
{0x1a,0x21},
{0x1b,0x80},
{0x1c,0x49},
{0x61,0x2a},
{0x62,0x8c},
{0x63,0x02},
{0x32,0x00},
{0x3a,0x20},
{0x3b,0x20},
{0x69,0x03},
{0x65,0x10},
{0x6c,0xaa},
{0x6d,0x00},
{0x67,0x10},
{0x4a,0x40},
{0x4b,0x40},
{0x4c,0x40},
{0xe8,0x04},
{0xe9,0xbb},
{0x42,0x20},
{0x47,0x10},
{0x50,0x40},
{0xd0,0x00},
{0xd3,0x50},
{0xf6,0x05},
{0x01,0x6a},
{0x02,0x0c},
{0x0f,0x00},
{0x6a,0x11},
{0x71,0x01},
{0x72,0x02},
{0x79,0x02},
{0x73,0x01},
{0x7a,0x01},
{0x2e,0x30},
{0x2b,0x00},
{0x2c,0x03},
{0xd2,0x00},
{0x20,0xb0},
};


static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 1,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
 {0x60,0x94},
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 10,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x60,0x84},
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 10,
};

static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg gc0339_cid_cfg[] = {
  {0, CSI_RAW8, CSI_DECODE_8BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params gc0339_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(gc0339_cid_cfg),
      .vc_cfg = {
         &gc0339_cid_cfg[0],
		 &gc0339_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x26,
  },
};

static struct sensor_pix_fmt_info_t gc0339_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t gc0339_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t gc0339_stream_info[] = {
  {1, &gc0339_cid_cfg[0], gc0339_pix_fmt0_fourcc},
  {1, &gc0339_cid_cfg[1], gc0339_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t gc0339_stream_info_array = {
  .sensor_stream_info = gc0339_stream_info,
  .size = ARRAY_SIZE(gc0339_stream_info),
};


static struct msm_camera_i2c_reg_array res0_reg_array[] = {
};


static struct msm_camera_i2c_reg_setting res_settings[] = {
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};


static struct msm_camera_csi2_params *csi_params[] = {
  &gc0339_csi_params, /* RES 0*/
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
    .x_output = 640,
    .y_output = 480,
    .line_length_pclk = 800,
    .frame_length_lines = 500,
    .vt_pixel_clk = 12000000,
    .op_pixel_clk = 480000000,
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

static sensor_res_cfg_type_t gc0339_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t gc0339_res_table = {
  .res_cfg_type = gc0339_res_cfg,
  .size = ARRAY_SIZE(gc0339_res_cfg),
};

static struct sensor_lib_chromatix_t gc0339_chromatix[] = {
  {
    .common_chromatix = GC0339_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = GC0339_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = GC0339_LOAD_CHROMATIX(preview), /* RES0 */
    .camcorder_chromatix = GC0339_LOAD_CHROMATIX(default_video), /* RES0 */
  },
};

static struct sensor_lib_chromatix_array gc0339_lib_chromatix_array = {
  .sensor_lib_chromatix = gc0339_chromatix,
  .size = ARRAY_SIZE(gc0339_chromatix),
};

/*===========================================================================
 * FUNCTION    - gc0339_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t gc0339_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if(gain < 1.0) {
      gain = 1.0;
  } else if (gain > 4.0) {
      gain = 4.0;
  }
  gain = (gain) * 64.0;
  reg_gain = (uint16_t) gain - 1;

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - gc0339_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float gc0339_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  if (reg_gain < 0x10) {
      reg_gain = 0x10;
  } else if (reg_gain > 0x100) {
      reg_gain = 0x100;
  }
  real_gain = (float) reg_gain / 64.0;

  return real_gain;
}

/*===========================================================================
 * FUNCTION    - gc0339_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t gc0339_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = gc0339_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = gc0339_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - gc0339_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t gc0339_fill_exposure_array(uint16_t gain, uint32_t line,
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
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = gain;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0x0F00) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = line & 0xFF;
  reg_count++;

  for (i = 0; i < sensor_lib_ptr.groupoff_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_BYTE_ADDR;
  reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;

  return rc;
}

static sensor_exposure_table_t gc0339_expsoure_tbl = {
  .sensor_calculate_exposure = gc0339_calculate_exposure,
  .sensor_fill_exposure_array = gc0339_fill_exposure_array,
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
  .sensor_num_HDR_frame_skip = 1,
  /* sensor pipeline immediate delay */
  .sensor_max_pipeline_frame_delay = 1,
  /* sensor exposure table size */
  .exposure_table_size = 5,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = gc0339_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(gc0339_cid_cfg),
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
  .sensor_res_cfg_table = &gc0339_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &gc0339_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &gc0339_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &gc0339_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - SKUAA_ST_gc0339_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *skuab_shinetech_gc0339_open_lib(void)
{
  return &sensor_lib_ptr;
}
