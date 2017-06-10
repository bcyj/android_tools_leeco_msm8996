/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_OV2680 "ov2680"
#define OV2680_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV2680"_"#n".so"

#define SNAPSHOT_1600X1200_PARMS  1
#define PREVIEW_800X600_PARMS     1

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
    .sensor_id = 0x2680,
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
  .focal_length = 2.93,
  .pix_size = 1.4,
  .f_number = 2.8,
  .total_f_dist = 1.2,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x7,
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
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103, 0x01, 0x00},
};

static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x3002, 0x00, 0x00}, //gpio0 input, vsync input, fsin input
  {0x3016, 0x1c, 0x00}, //drive strength = 0x01, bypass latch of hs_enable
  {0x3018, 0x44, 0x00}, //MIPI 10-bit mode
  {0x3020, 0x00, 0x00}, //output raw
  {0x3080, 0x02, 0x00}, //PLL
  {0x3082, 0x37, 0x00}, //PLL
  {0x3084, 0x09, 0x00}, //PLL
  {0x3085, 0x04, 0x00}, //PLL
  {0x3086, 0x01, 0x00}, //PLL
  {0x3501, 0x26, 0x00}, //exposure M
  {0x3502, 0x40, 0x00}, //exposure L
  {0x3503, 0x03, 0x00}, //vts auto, gain manual, exposure manual
  {0x350b, 0x36, 0x00}, //gain L
  {0x3600, 0xb4, 0x00}, //analog control
  {0x3603, 0x39, 0x00}, //
  {0x3604, 0x24, 0x00}, //
  {0x3605, 0x00, 0x00}, //
  {0x3620, 0x26, 0x00}, //
  {0x3621, 0x37, 0x00}, //
  {0x3622, 0x04, 0x00}, //
  {0x3628, 0x00, 0x00}, //analog control
  {0x3705, 0x3c, 0x00}, //sennsor control
  {0x370c, 0x50, 0x00}, //
  {0x370d, 0xc0, 0x00}, //
  {0x3718, 0x88, 0x00}, //
  {0x3720, 0x00, 0x00}, //
  {0x3721, 0x00, 0x00}, //
  {0x3722, 0x00, 0x00}, //
  {0x3723, 0x00, 0x00}, //
  {0x3738, 0x00, 0x00}, //
  {0x370a, 0x23, 0x00}, //
  {0x3717, 0x58, 0x00}, //sensor control
  {0x3781, 0x80, 0x00}, //PSRAM
  {0x3789, 0x60, 0x00}, //PSRAM
  {0x3800, 0x00, 0x00}, //x start H
  {0x3801, 0x00, 0x00}, //x start L
  {0x3802, 0x00, 0x00}, //y start H
  {0x3803, 0x00, 0x00}, //y start L
  {0x3804, 0x06, 0x00}, //x end H
  {0x3805, 0x4f, 0x00}, //x end L
  {0x3806, 0x04, 0x00}, //y end H
  {0x3807, 0xbf, 0x00}, //y end L
  {0x3808, 0x03, 0x00}, //x output size H
  {0x3809, 0x20, 0x00}, //x output size L
  {0x380a, 0x02, 0x00}, //y output size H
  {0x380b, 0x58, 0x00}, //y output size L
  {0x380c, 0x06, 0x00}, //HTS H
  {0x380d, 0xac, 0x00}, //HTS L
  {0x380e, 0x02, 0x00}, //VTS H
  {0x380f, 0x84, 0x00}, //VTS L
  {0x3810, 0x00, 0x00}, //ISP x win H
  {0x3811, 0x04, 0x00}, //ISP x win L
  {0x3812, 0x00, 0x00}, //ISP y win H
  {0x3813, 0x04, 0x00}, //ISP y win L
  {0x3814, 0x31, 0x00}, //x inc
  {0x3815, 0x31, 0x00}, //y inc
  {0x3819, 0x04, 0x00}, //vsync end row
  {0x3820, 0xc2, 0x00}, //vsun48_blc, vflip_blc, vbinf
  {0x3821, 0x01, 0x00}, //hbin
  {0x4000, 0x81, 0x00}, //avg_weight = 0x08, mf_en
  {0x4001, 0x40, 0x00}, //format_trig_beh
  {0x4008, 0x00, 0x00}, //blc_start
  {0x4009, 0x03, 0x00}, //blc_end
  {0x4602, 0x02, 0x00}, //frame reset enable
  {0x481f, 0x36, 0x00}, //CLK PREPARE MIN
  {0x4825, 0x36, 0x00}, //LPX P MIN
  {0x4837, 0x30, 0x00}, //MIPI global timing
  {0x5002, 0x30, 0x00}, //
  {0x5080, 0x00, 0x00}, //test pattern off
  {0x5081, 0x41, 0x00}, //window cut enable, random seed = 0x01
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
    .delay = 10,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 2,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
  {0x0100, 0x01, 0x00},
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
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
  {0x3208, 0x0, 0x00},
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x3208, 0x10, 0x00},
  {0x3208, 0xA0, 0x00},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg ov2680_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov2680_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov2680_cid_cfg),
      .vc_cfg = {
         &ov2680_cid_cfg[0],
         &ov2680_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x15,
    .combo_mode = 1,
  },
};

static struct sensor_pix_fmt_info_t ov2680_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov2680_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov2680_stream_info[] = {
  {1, &ov2680_cid_cfg[0], ov2680_pix_fmt0_fourcc},
  {1, &ov2680_cid_cfg[1], ov2680_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov2680_stream_info_array = {
  .sensor_stream_info = ov2680_stream_info,
  .size = ARRAY_SIZE(ov2680_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  //Capture 1600x1200 30fps
 //sysclk = 66Mhz, MIPI data rate 330Mbps
  {0x3086, 0x00, 0x00}, //PLL
  {0x3501, 0x4e, 0x00}, //exposure M
  {0x3502, 0xe0, 0x00}, //exposure L
  {0x3620, 0x26, 0x00}, //analog control
  {0x3621, 0x37, 0x00}, //
  {0x3622, 0x04, 0x00}, //analog control
  {0x370a, 0x21, 0x00}, //sennsor control
  {0x370d, 0xc0, 0x00}, //
  {0x3718, 0x88, 0x00}, //
  {0x3721, 0x00, 0x00}, //
  {0x3722, 0x00, 0x00}, //
  {0x3723, 0x00, 0x00}, //
  {0x3738, 0x00, 0x00}, //sennsor control
  {0x3803, 0x00, 0x00}, //y start L
  {0x3807, 0xbf, 0x00}, //y end L
  {0x3808, 0x06, 0x00}, //x output size H
  {0x3809, 0x40, 0x00}, //x output size L
  {0x380a, 0x04, 0x00}, //y output size H
  {0x380b, 0xb0, 0x00}, //y output size L
  {0x380c, 0x06, 0x00}, //HTS H
  {0x380d, 0xa4, 0x00}, //HTS L
  {0x380e, 0x05, 0x00}, //VTS H
  {0x380f, 0x0e, 0x00}, //VTS L
  {0x3811, 0x08, 0x00}, //ISP x win L
  {0x3813, 0x08, 0x00}, //ISP y win L
  {0x3814, 0x11, 0x00}, //x inc
  {0x3815, 0x11, 0x00}, //y inc
  {0x3820, 0xc0, 0x00}, //vsun48_blc, vflip_blc, vbin off
  {0x3821, 0x00, 0x00}, //hbin off
  {0x4008, 0x02, 0x00}, //blc_start
  {0x4009, 0x09, 0x00}, //blc_end
  {0x4837, 0x18, 0x00}, //MIPI global timing
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  //Preview 800x600 30fps
  //sysclk = 33Mhz, MIPI data rate 330Mbps
  {0x3086, 0x01, 0x00}, //PLL
  {0x3501, 0x26, 0x00}, //exposure M
  {0x3502, 0x40, 0x00}, //exposure L
  {0x3620, 0x26, 0x00}, //analog control
  {0x3621, 0x37, 0x00}, //
  {0x3622, 0x04, 0x00}, //analog control
  {0x370a, 0x23, 0x00}, //sennsor control
  {0x370d, 0xc0, 0x00}, //
  {0x3718, 0x88, 0x00}, //
  {0x3721, 0x00, 0x00}, //
  {0x3722, 0x00, 0x00}, //
  {0x3723, 0x00, 0x00}, //
  {0x3738, 0x00, 0x00}, //sennsor control
  {0x3803, 0x00, 0x00}, //y start L
  {0x3807, 0xbf, 0x00}, //y end L
  {0x3808, 0x03, 0x00}, //x output size H
  {0x3809, 0x20, 0x00}, //x output size L
  {0x380a, 0x02, 0x00}, //y output size H
  {0x380b, 0x58, 0x00}, //y output size L
  {0x380c, 0x06, 0x00}, //HTS H
  {0x380d, 0xac, 0x00}, //HTS L
  {0x380e, 0x02, 0x00}, //VTS H
  {0x380f, 0x84, 0x00}, //VTS L
  {0x3811, 0x04, 0x00}, //ISP x win L
  {0x3813, 0x04, 0x00}, //ISP y win L
  {0x3814, 0x31, 0x00}, //x inc
  {0x3815, 0x31, 0x00}, //y inc
  {0x3820, 0xc2, 0x00}, //vsun48_blc, vflip_blc, vbinf
  {0x3821, 0x01, 0x00}, //hbin
  {0x4008, 0x00, 0x00}, //blc_start
  {0x4009, 0x03, 0x00}, //blc_end
  {0x4837, 0x30, 0x00}, //MIPI global timing
};

static struct msm_camera_i2c_reg_setting res_settings[] = {
#if SNAPSHOT_1600X1200_PARMS
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
#if PREVIEW_800X600_PARMS
  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct msm_camera_csi2_params *csi_params[] = {
#if SNAPSHOT_1600X1200_PARMS
  &ov2680_csi_params, /* RES 0*/
#endif
#if PREVIEW_800X600_PARMS
  &ov2680_csi_params, /* RES 1*/
#endif
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
#if SNAPSHOT_1600X1200_PARMS
  {0, 0, 0, 0}, /* RES 0 */
#endif
#if PREVIEW_800X600_PARMS
  {0, 0, 0, 0}, /* RES 1 */
#endif
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
#if SNAPSHOT_1600X1200_PARMS
  {
    .x_output = 1600,
    .y_output = 1200,
    .line_length_pclk = 1700,
    .frame_length_lines = 1294,
    .vt_pixel_clk = 65400000,
    .op_pixel_clk = 66000000,
    .binning_factor = 1,
    .max_fps = 29.71,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if PREVIEW_800X600_PARMS
  {
    .x_output = 800,
    .y_output = 600,
    .line_length_pclk = 1708,
    .frame_length_lines = 644,
    .vt_pixel_clk = 32900000,
    .op_pixel_clk = 33000000,
    .binning_factor = 1,
    .max_fps = 29.85,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov2680_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov2680_res_table = {
  .res_cfg_type = ov2680_res_cfg,
  .size = ARRAY_SIZE(ov2680_res_cfg),
};

static struct sensor_lib_chromatix_t ov2680_chromatix[] = {
#if SNAPSHOT_1600X1200_PARMS
  {
    .common_chromatix = OV2680_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV2680_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = OV2680_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = OV2680_LOAD_CHROMATIX(default_video), /* RES0 */
  },
#endif
#if PREVIEW_800X600_PARMS
  {
    .common_chromatix = OV2680_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV2680_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = OV2680_LOAD_CHROMATIX(preview), /* RES0 */
    .camcorder_chromatix = OV2680_LOAD_CHROMATIX(default_video), /* RES0 */
  },
#endif
};

static struct sensor_lib_chromatix_array ov2680_lib_chromatix_array = {
  .sensor_lib_chromatix = ov2680_chromatix,
  .size = ARRAY_SIZE(ov2680_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov2680_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov2680_real_to_register_gain(float gain)
{
  uint16_t reg_gain, reg_temp;
  reg_gain = (uint16_t)gain;
  reg_temp = reg_gain<<4;
  reg_gain = reg_temp | (((uint16_t)((gain - (float)reg_gain)*16.0))&0x0f);
  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - ov2680_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov2680_register_to_real_gain(uint16_t reg_gain)
{

  float real_gain;
  real_gain = (float) ((float)(reg_gain>>4)+(((float)(reg_gain&0x0f))/16.0));
  return real_gain;
}

/*===========================================================================
 * FUNCTION    - ov2680_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov2680_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov2680_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = ov2680_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov2680_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov2680_fill_exposure_array(uint16_t gain, uint32_t line,
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
  reg_setting->reg_setting[reg_count].reg_data = line >> 12;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = line >> 4;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 2;
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

static sensor_exposure_table_t ov2680_expsoure_tbl = {
  .sensor_calculate_exposure = ov2680_calculate_exposure,
  .sensor_fill_exposure_array = ov2680_fill_exposure_array,
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
  .sensor_max_pipeline_frame_delay = 2,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov2680_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov2680_cid_cfg),
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
  .sensor_res_cfg_table = &ov2680_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov2680_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov2680_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov2680_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - ov2680_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov2680_07p2_open_lib(void)
{
  return &sensor_lib_ptr;
}
