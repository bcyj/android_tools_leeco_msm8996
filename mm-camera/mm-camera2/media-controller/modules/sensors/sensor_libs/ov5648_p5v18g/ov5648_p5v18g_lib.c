/* ov5648_p5v18g_lib.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_OV5648_P5V18G "ov5648_p5v18g"
#define OV5648_P5V18G_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV5648_P5V18G"_"#n".so"

#undef DEBUG_INFO
#define OV5648_P5V18G_DEBUG
#ifdef OV5648_P5V18G_DEBUG
#include <utils/Log.h>
#define SERR(fmt, args...) \
  ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)

#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting ov5648_p5v18g_power_setting[] = {
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VDIG,
    .config_val = GPIO_OUT_LOW,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VANA,
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
    .seq_val = SENSOR_GPIO_STANDBY,
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
    .sensor_id_reg_addr = 0x300a,
    /* sensor id */
    .sensor_id = 0x5648,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = ov5648_p5v18g_power_setting,
    .size = ARRAY_SIZE(ov5648_p5v18g_power_setting),
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
  .max_gain = 12.0,
  .max_linecount = 26880,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 3.16,
  .pix_size = 1.4,
  .f_number = 2.4,
  .total_f_dist = 1.2,
  .hor_view_angle = 56.0,
  .ver_view_angle = 42.0,
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
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array[] = {
  {0x0103, 0x01}, // software reset
  {0x0100, 0x00}, //

  {0x3001, 0x00}, // D[7:0] set to input
  {0x3002, 0x00}, // Vsync, PCLK, FREX, Strobe, CSD, CSK, GPIO input
  {0x3011, 0x02}, // Drive strength 2x
  {0x3018, 0x4c}, // MIPI 2 lane
  {0x3022, 0x00},
  {0x3034, 0x1a}, // 10-bit mode
  {0x3035, 0x21}, // PLL
  {0x3036, 0x69}, // PLL
  {0x3037, 0x03}, // PLL
  {0x3038, 0x00}, // PLL
  {0x3039, 0x00}, // PLL
  {0x303a, 0x00}, // PLLS
  {0x303b, 0x19}, // PLLS
  {0x303c, 0x11}, // PLLS
  {0x303d, 0x30}, //  PLLS
  {0x3105, 0x11},
  {0x3106, 0x05}, // PLL
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

  {0x3503, 0x03}, // gain has no delay, manual agc/aec

  {0x3601, 0x33}, // analog control
  {0x3602, 0x00}, // analog control
  {0x3611, 0x0e}, // analog control
  {0x3612, 0x2b}, // analog control
  {0x3614, 0x50}, // analog control
  {0x3620, 0x33}, // analog control
  {0x3622, 0x00}, // analog control
  {0x3630, 0xad}, // analog control
  {0x3631, 0x00}, // analog control
  {0x3632, 0x94}, // analog control
  {0x3633, 0x17}, // analog control
  {0x3634, 0x14}, // analog control

  {0x3704, 0xc0}, //
  {0x4050, 0x37}, // blc level trigger
  {0x4051, 0x8f}, // blc level trigger


  {0x3705, 0x2a}, // analog control
  {0x3708, 0x66}, // analog control
  {0x3709, 0x52}, // analog control
  {0x370b, 0x23}, // analog control
  {0x370c, 0xc3}, // analog control
  {0x370d, 0x00}, // analog control
  {0x370e, 0x00}, // analog control
  {0x371c, 0x07}, // analog control
  {0x3739, 0xd2}, // analog control
  {0x373c, 0x00},

  {0x3800, 0x00}, // xstart = 0
  {0x3801, 0x00}, // xstart
  {0x3802, 0x00}, // ystart = 0
  {0x3803, 0x00}, // ystart
  {0x3804, 0x0a}, // xend = 2623
  {0x3805, 0x3f}, // yend
  {0x3806, 0x07}, // yend = 1955
  {0x3807, 0xa3}, // yend
  {0x3808, 0x05}, // x output size = 1296
  {0x3809, 0x10}, // x output size
  {0x380a, 0x03}, // y output size = 972
  {0x380b, 0xcc}, // y output size
  {0x380c, 0x05}, // hts = 1408
  {0x380d, 0x80}, // hts
  {0x380e, 0x03}, // vts = 992
  {0x380f, 0xe0}, // vts
  {0x3810, 0x00}, // isp x win = 8
  {0x3811, 0x08}, // isp x win
  {0x3812, 0x00}, // isp y win = 4
  {0x3813, 0x04}, // isp y win
  {0x3814, 0x31}, // x inc
  {0x3815, 0x31}, // y inc
  {0x3817, 0x00}, // hsync start
  {0x3820, 0x08}, // flip off, v bin off
  {0x3821, 0x07}, // mirror on, h bin on
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

  {0x3b00, 0x00}, // strobe off
  {0x3b02, 0x08}, // shutter delay
  {0x3b03, 0x00}, // shutter delay
  {0x3b04, 0x04}, // frex_exp
  {0x3b05, 0x00}, // frex_exp
  {0x3b06, 0x04},
  {0x3b07, 0x08}, // frex inv
  {0x3b08, 0x00}, // frex exp req
  {0x3b09, 0x02}, // frex end option
  {0x3b0a, 0x04}, // frex rst length
  {0x3b0b, 0x00}, // frex strobe width
  {0x3b0c, 0x3d}, // frex strobe width

  {0x3f01, 0x0d},
  {0x3f0f, 0xf5},

  {0x4000, 0x89}, // blc enable
  {0x4001, 0x02}, // blc start line
  {0x4002, 0x45}, // blc auto, reset frame number = 5
  {0x4004, 0x02}, // black line number
  {0x4005, 0x18}, // blc normal freeze
  {0x4006, 0x08},
  {0x4007, 0x10},
  {0x4008, 0x00},

  {0x4300, 0xf8},
  {0x4303, 0xff},
  {0x4304, 0x00},
  {0x4307, 0xff},
  {0x4520, 0x00},
  {0x4521, 0x00},
  {0x4511, 0x22},

  {0x481f, 0x3c}, // MIPI clk prepare min
  {0x4826, 0x00}, // MIPI hs prepare min
  {0x4837, 0x18}, // MIPI global timing
  {0x4b00, 0x06},
  {0x4b01, 0x0a},
  {0x5000, 0xff}, // bpc on, wpc on
  {0x5001, 0x00}, // awb disable
  {0x5002, 0x41}, // win enable, awb gain enable
  {0x5003, 0x0a}, // buf en, bin auto en
  {0x5004, 0x00}, // size man off
  {0x5043, 0x00},
  {0x5013, 0x00},
  {0x501f, 0x03}, // ISP output data
  {0x503d, 0x00}, // test pattern off
  {0x5180, 0x08}, // manual gain enable
  {0x5a00, 0x08},
  {0x5b00, 0x01},
  {0x5b01, 0x40},
  {0x5b02, 0x00},
  {0x5b03, 0xf0},

  {0x4837, 0x17}, // MIPI global timing
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
  {0x100, 0x1},
  {0x4800, 0x4},
  {0x4202, 0x00},  /* streaming on */
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 10,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x100, 0x0},
  {0x4800, 0x25},
  {0x4202, 0x0f},  /* streaming off*/
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 10,
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

static struct msm_camera_csid_vc_cfg ov5648_p5v18g_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov5648_p5v18g_csi_params = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov5648_p5v18g_cid_cfg),
      .vc_cfg = {
         &ov5648_p5v18g_cid_cfg[0],
         &ov5648_p5v18g_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x1B,
  },
};

static struct sensor_pix_fmt_info_t ov5648_p5v18g_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov5648_p5v18g_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov5648_p5v18g_stream_info[] = {
  {1, &ov5648_p5v18g_cid_cfg[0], ov5648_p5v18g_pix_fmt0_fourcc},
  {1, &ov5648_p5v18g_cid_cfg[1], ov5648_p5v18g_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov5648_p5v18g_stream_info_array = {
  .sensor_stream_info = ov5648_p5v18g_stream_info,
  .size = ARRAY_SIZE(ov5648_p5v18g_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  //2592x1944 15fps 2 lane MIPI 420Mbps/lane
  {0x3708, 0x63}, //
  {0x3709, 0x12}, //
  {0x370c, 0xc0}, //

  {0x3500, 0x00}, // exposure [19:16]
  {0x3501, 0x10}, // exposure [15:8]
  {0x3502, 0x80}, // exposure [7:0], exposure
  {0x3503, 0x03}, // gain has no delay, manual agc/aec
  {0x350a, 0x00}, // gain[9:8]
  {0x350b, 0x40}, // gain[7:0],

  {0x3800, 0x00}, //xstart = 0
  {0x3801, 0x00}, //xstart
  {0x3802, 0x00}, //ystart = 0
  {0x3803, 0x00}, //ystart
  {0x3804, 0x0a}, //xend = 2623
  {0x3805, 0x3f}, //xend
  {0x3806, 0x07}, //yend = 1955
  {0x3807, 0xa3}, //yend
  {0x3808, 0x0a}, //x output size = 2592
  {0x3809, 0x20}, //x output size
  {0x380a, 0x07}, //y output size = 1944
  {0x380b, 0x98}, //y output size
  {0x380c, 0x0b}, //hts = 2816
  {0x380d, 0x00}, //hts
  {0x380e, 0x07}, //vts = 1984
  {0x380f, 0xc0}, //vts
  {0x3810, 0x00}, //isp x win = 16
  {0x3811, 0x10}, //isp x win
  {0x3812, 0x00}, //isp y win = 6
  {0x3813, 0x06}, //isp y win
  {0x3814, 0x11}, //x inc
  {0x3815, 0x11}, //y inc
  {0x3817, 0x00}, //hsync start
  {0x3820, 0x40}, //flip off, v bin off
  {0x3821, 0x06}, //mirror on, v bin off

  {0x4004, 0x04}, //black line number
  {0x4005, 0x1a}, //BLC always update
  {0x4837, 0x17}, //MIPI global timing

};
static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  // 1296x972 30fps 2 lane MIPI 420Mbps/lane
  {0x3708, 0x66},
  {0x3709, 0x52},
  {0x370c, 0xc3},

  {0x3500, 0x00}, // exposure [19:16]
  {0x3501, 0x3d}, // exposure [15:8]
  {0x3502, 0xc0}, // exposure [7:0], exposure
  {0x3503, 0x07}, // gain has no delay, manual agc/aec
  {0x350a, 0x00}, // gain[9:8]
  {0x350b, 0x23}, // gain[7:0],

  {0x3800, 0x00},//xstart = 0
  {0x3801, 0x00},//x start
  {0x3802, 0x00},//y start = 0
  {0x3803, 0x00},//y start
  {0x3804, 0x0a},//xend = 2623
  {0x3805, 0x3f},//xend
  {0x3806, 0x07},//yend = 1955
  {0x3807, 0xa3},//yend
  {0x3808, 0x05},//x output size = 1296
  {0x3809, 0x10},//x output size
  {0x380a, 0x03},//y output size = 972
  {0x380b, 0xcc},//y output size
  {0x380c, 0x0a},//hts = 2808
  {0x380d, 0xf8},//hts
  {0x380e, 0x03},//vts = 992
  {0x380f, 0xe0},//vts
  {0x3810, 0x00},//isp x win = 8
  {0x3811, 0x08},//isp x win
  {0x3812, 0x00},//isp y win = 4
  {0x3813, 0x04},//isp y win
  {0x3814, 0x31},//x inc
  {0x3815, 0x31},//y inc
  {0x3817, 0x00},//hsync start
  {0x3820, 0x08},//flip off, v bin off
  {0x3821, 0x07},//mirror on, h bin on

  {0x4004, 0x02},//black line number
  {0x4005, 0x18},//BLC normal freeze
  {0x4837, 0x17},//MIPI global timing

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
  &ov5648_p5v18g_csi_params, /* RES 0*/
  &ov5648_p5v18g_csi_params, /* RES 1*/
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
  {// full size
    .x_output = 0xa20, /* 2592 */
    .y_output = 0x798, /* 1944 */
    .line_length_pclk = 0xb00, /* 2816 */
    .frame_length_lines = 0x7c0, /* 1984 */
    .vt_pixel_clk = 83800000,
    .op_pixel_clk = 84000000,
    .binning_factor = 1,
    .max_fps = 15.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {//
    .x_output = 0x510, /* 1296 */
    .y_output = 0x3cc, /* 972 */
    .line_length_pclk = 0xaf8, /* 2808 */
    .frame_length_lines = 0x3e0, /* 1120 */
    .vt_pixel_clk = 83600000,
    .op_pixel_clk = 84000000,
    .binning_factor = 1,
    .max_fps = 30.00,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },


};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov5648_p5v18g_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov5648_p5v18g_res_table = {
  .res_cfg_type = ov5648_p5v18g_res_cfg,
  .size = ARRAY_SIZE(ov5648_p5v18g_res_cfg),
};

static struct sensor_lib_chromatix_t ov5648_p5v18g_chromatix[] = {
  {
    .common_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(default_video), /* RES0 */
  },
  {
    .common_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(preview), /* RES2 */
    .camera_snapshot_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(preview), /* RES2 */
    .camcorder_chromatix = OV5648_P5V18G_LOAD_CHROMATIX(default_video), /* RES2 */
  },
};

static struct sensor_lib_chromatix_array ov5648_p5v18g_lib_chromatix_array = {
  .sensor_lib_chromatix = ov5648_p5v18g_chromatix,
  .size = ARRAY_SIZE(ov5648_p5v18g_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov5648_p5v18g_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov5648_p5v18g_real_to_register_gain(float gain)
{
  uint16_t reg_gain, reg_temp;
  reg_gain = (uint16_t)gain;
  reg_temp = reg_gain<<4;
  reg_gain = reg_temp | (((uint16_t)((gain - (float)reg_gain)*16.0))&0x0f);
  return reg_gain;

}

/*===========================================================================
 * FUNCTION    - ov5648_p5v18g_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov5648_p5v18g_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;
  real_gain = (float) ((float)(reg_gain>>4)+(((float)(reg_gain&0x0f))/16.0));
  return real_gain;

}

/*===========================================================================
 * FUNCTION    - ov5648_p5v18g_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov5648_p5v18g_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov5648_p5v18g_real_to_register_gain(real_gain);
  float sensor_real_gain = ov5648_p5v18g_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / sensor_real_gain;
  exp_info->line_count = line_count;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov5648_p5v18g_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov5648_p5v18g_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting* reg_setting)
{
  uint16_t reg_count = 0;
  uint16_t i = 0;

  if (!reg_setting) {
    return -1;
  }

  DEBUG_INFO("%s:gain=%d,line=%d,fl_lines=%d",__func__,
    gain,line,fl_lines) ;
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
  reg_setting->reg_setting[reg_count].reg_data = line  >> 12;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = line >> 4;
  reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 2;
  reg_setting->reg_setting[reg_count].reg_data = line << 4;
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

static sensor_exposure_table_t ov5648_p5v18g_expsoure_tbl = {
  .sensor_calculate_exposure = ov5648_p5v18g_calculate_exposure,
  .sensor_fill_exposure_array = ov5648_p5v18g_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
   .actuator_name = "dw9714",
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
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov5648_p5v18g_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov5648_p5v18g_cid_cfg),
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
  .sensor_res_cfg_table = &ov5648_p5v18g_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov5648_p5v18g_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov5648_p5v18g_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov5648_p5v18g_lib_chromatix_array,
};

/*===========================================================================
 * FUNCTION    - ov5648_p5v18g_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov5648_p5v18g_open_lib(void)
{
  return &sensor_lib_ptr;
}
