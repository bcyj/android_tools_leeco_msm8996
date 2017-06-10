/*
 * imx091_params.h
 *
 *  Created on: Aug 8, 2012
 *      Author: jackwang
 */

#ifndef IMX091_PARAMS_H_
#define IMX091_PARAMS_H_

static struct msm_camera_i2c_reg_array sensor_start_settings[] = {
  {0x0100, 0x01},
};

static struct msm_camera_i2c_reg_array sensor_prev_settings[] = {
  /* 30fps 1/2 * 1/2 */
  /* PLL setting */
  {0x0305, 0x02}, /* pre_pll_clk_div[7:0] */
  {0x0307, 0x2F},/* pll_multiplier[7:0] */
  {0x30A4, 0x02},
  {0x303C, 0x4B},
  /* mode setting */
  {0x0340, 0x06}, /* frame_length_lines[15:8] */
  {0x0341, 0x5A}, /* frame_length_lines[7:0] */
  {0x0342, 0x12},/* line_length_pck[15:8] */
  {0x0343, 0x0C}, /* line_length_pck[7:0] */
  {0x0344, 0x00}, /* x_addr_start[15:8] */
  {0x0345, 0x08}, /* x_addr_start[7:0] */
  {0x0346, 0x00}, /* y_addr_start[15:8] */
  {0x0347, 0x30}, /* y_addr_start[7:0] */
  {0x0348, 0x10}, /* x_addr_end[15:8] */
  {0x0349, 0x77}, /* x_addr_end[7:0] */
  {0x034A, 0x0C}, /* y_addr_end[15:8] */
  {0x034B, 0x5F}, /* y_addr_end[7:0] */
  {0x034C, 0x08},/* x_output_size[15:8] */
  {0x034D, 0x38}, /* x_output_size[7:0] */
  {0x034E, 0x06}, /* y_output_size[15:8] */
  {0x034F, 0x18}, /* y_output_size[7:0] */
  {0x0381, 0x01}, /* x_even_inc[3:0] */
  {0x0383, 0x03}, /* x_odd_inc[3:0] */
  {0x0385, 0x01}, /* y_even_inc[7:0] */
  {0x0387, 0x03}, /* y_odd_inc[7:0] */
  {0x3040, 0x08},
  {0x3041, 0x97},
  {0x3048, 0x01},
  {0x3064, 0x12},
  {0x309B, 0x28},
  {0x309E, 0x00},
  {0x30D5, 0x09},
  {0x30D6, 0x01},
  {0x30D7, 0x01},
  {0x30D8, 0x64},
  {0x30D9, 0x89},
  {0x30DE, 0x02},
  {0x3102, 0x10},
  {0x3103, 0x44},
  {0x3104, 0x40},
  {0x3105, 0x00},
  {0x3106, 0x0D},
  {0x3107, 0x01},
  {0x310A, 0x0A},
  {0x315C, 0x99},
  {0x315D, 0x98},
  {0x316E, 0x9A},
  {0x316F, 0x99},
  {0x3318, 0x73},
};

static struct msm_camera_i2c_reg_array sensor_snap_settings[] = {
  /* full size */
  /* PLL setting */
  {0x0305, 0x02}, /* pre_pll_clk_div[7:0] */
  {0x0307, 0x2B}, /* pll_multiplier[7:0] */
  {0x30A4, 0x02},
  {0x303C, 0x4B},
  /* mode setting */
  {0x0340, 0x0C}, /* frame_length_lines[15:8] */
  {0x0341, 0x8C}, /* frame_length_lines[7:0] */
  {0x0342, 0x12}, /* line_length_pck[15:8] */
  {0x0343, 0x0C}, /* line_length_pck[7:0] */
  {0x0344, 0x00}, /* x_addr_start[15:8] */
  {0x0345, 0x08}, /* x_addr_start[7:0] */
  {0x0346, 0x00}, /* y_addr_start[15:8] */
  {0x0347, 0x30}, /* y_addr_start[7:0] */
  {0x0348, 0x10}, /* x_addr_end[15:8] */
  {0x0349, 0x77}, /* x_addr_end[7:0] */
  {0x034A, 0x0C}, /* y_addr_end[15:8] */
  {0x034B, 0x5F}, /* y_addr_end[7:0] */
  {0x034C, 0x10}, /* x_output_size[15:8] */
  {0x034D, 0x70}, /* x_output_size[7:0] */
  {0x034E, 0x0C}, /* y_output_size[15:8] */
  {0x034F, 0x30}, /* y_output_size[7:0] */
  {0x0381, 0x01}, /* x_even_inc[3:0] */
  {0x0383, 0x01}, /* x_odd_inc[3:0] */
  {0x0385, 0x01}, /* y_even_inc[7:0] */
  {0x0387, 0x01}, /* y_odd_inc[7:0] */
  {0x3040, 0x08},
  {0x3041, 0x97},
  {0x3048, 0x00},
  {0x3064, 0x12},
  {0x309B, 0x20},
  {0x309E, 0x00},
  {0x30D5, 0x00},
  {0x30D6, 0x85},
  {0x30D7, 0x2A},
  {0x30D8, 0x64},
  {0x30D9, 0x89},
  {0x30DE, 0x00},
  {0x3102, 0x10},
  {0x3103, 0x44},
  {0x3104, 0x40},
  {0x3105, 0x00},
  {0x3106, 0x0D},
  {0x3107, 0x01},
  {0x310A, 0x0A},
  {0x315C, 0x99},
  {0x315D, 0x98},
  {0x316E, 0x9A},
  {0x316F, 0x99},
  {0x3318, 0x64},
};

static struct msm_camera_i2c_reg_array sensor_recommend_settings[] = {
  /* global setting */
  {0x3087, 0x53},
  {0x309D, 0x94},
  {0x30A1, 0x08},
  {0x30C7, 0x00},
  {0x3115, 0x0E},
  {0x3118, 0x42},
  {0x311D, 0x34},
  {0x3121, 0x0D},
  {0x3212, 0xF2},
  {0x3213, 0x0F},
  {0x3215, 0x0F},
  {0x3217, 0x0B},
  {0x3219, 0x0B},
  {0x321B, 0x0D},
  {0x321D, 0x0D},
  /* black level setting */
  {0x3032, 0x40},
};

static struct msm_camera_i2c_reg_array sensor_groupon_settings[] = {
  {0x0104, 0x01},
};

static struct msm_camera_i2c_reg_array sensor_groupoff_settings[] = {
  {0x0104, 0x00},
};


static struct msm_camera_i2c_reg_setting sensor_init_conf[] = {
  {
    &sensor_recommend_settings[0],
    ARRAY_SIZE(sensor_recommend_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  }
};

static struct msm_camera_i2c_reg_setting sensor_start_settings_array[] = {
  {
    &sensor_start_settings[0],
    ARRAY_SIZE(sensor_start_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  }
};

static struct msm_camera_i2c_reg_setting sensor_groupon_settings_array[] = {
  {
    &sensor_groupon_settings[0],
    ARRAY_SIZE(sensor_groupon_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  }
};

static struct msm_camera_i2c_reg_setting sensor_groupoff_settings_array[] = {
  {
    &sensor_groupoff_settings[0],
    ARRAY_SIZE(sensor_groupoff_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  }
};


static struct msm_camera_i2c_reg_array sensor_stop_settings[] = {
  {0x0100, 0x00},
};

static struct msm_camera_i2c_reg_setting sensor_stop_settings_array[] = {
  {
    &sensor_stop_settings[0],
    ARRAY_SIZE(sensor_stop_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  }
};


static struct msm_camera_i2c_reg_setting sensor_confs[] = {
  {
    &sensor_snap_settings[0],
    ARRAY_SIZE(sensor_snap_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  },
  {
    &sensor_prev_settings[0],
    ARRAY_SIZE(sensor_prev_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_BYTE_DATA,
    0
  },
};


static struct msm_sensor_exp_gain_info_t sensor_exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0204,
  .vert_offset = 5,
};

//ctrl->driver_params->mode_settings[

static struct msm_sensor_output_info_t sensor_dimensions[] = {
  {
    /* full size */
    .x_output = 0x1070, /* 4208 */
    .y_output = 0x0C30, /* 3120 */
    .line_length_pclk = 0x120C, /* 4620 */
    .frame_length_lines = 0x0C8C, /* 3212 */
    .vt_pixel_clk = 206400000,
    .op_pixel_clk = 206400000,
    .binning_factor = 1,
  },
  {
    /* 30 fps 1/2 * 1/2 */
    .x_output = 0x0838, /* 2104 */
    .y_output = 0x0618, /* 1560 */
    .line_length_pclk = 0x120C, /* 4620 */
    .frame_length_lines = 0x065A, /* 1626 */
    .vt_pixel_clk = 225600000,
    .op_pixel_clk = 225600000,
    .binning_factor = 1,
  },
};

static struct msm_sensor_output_reg_addr_t sensor_reg_addr = {
  .x_output = 0x034C,
  .y_output = 0x034E,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};


#endif /* IMX091_PARAMS_H_ */
