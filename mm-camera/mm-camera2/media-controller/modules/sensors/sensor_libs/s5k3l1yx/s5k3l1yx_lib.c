/* s5k3l1yx_lib.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_S5K3L1YX "s5k3l1yx"
#define S5K3L1YX_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_S5K3L1YX"_"#n".so"

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VDIG,
    .config_val = 0,
    .delay = 0,
  },
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
    .seq_val = CAM_VAF,
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
    .delay = 30,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_LOW,
    .delay = 1,
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
  .camera_id = CAMERA_0,
  /* sensor slave address */
  .slave_addr = 0x6e,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x00,
    /* sensor id */
    .sensor_id = 0x3121,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_SUPPORTED,
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
  .x_output = 0x34C,
  .y_output = 0x34E,
  .line_length_pclk = 0x342,
  .frame_length_lines = 0x340,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0204,
  .vert_offset = 8,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 16.0,
  .max_linecount = 57888,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 1.15,
  .pix_size = 1.4,
  .f_number = 2.6,
  .total_f_dist = 1.5,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xE4,
  .csi_lane_mask = 0xF,
  .csi_if = 1,
  .csid_core = {0},
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x1F,
  .csi_if = 1,
  .csid_core = {0},
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array[] = {
  {0x0100, 0x00},
  {0x0103, 0x01}, /* software_reset */
  {0x0104, 0x00}, /* grouped_parameter_hold */
  {0x0114, 0x03}, /* CSI_lane_mode, 4 lane setting */
  {0x0120, 0x00}, /* gain_mode, global analogue gain*/
  {0x0121, 0x00}, /* exposure_mode, global exposure */
  {0x0136, 0x18}, /* Extclk_frequency_mhz */
  {0x0137, 0x00}, /* Extclk_frequency_mhz */
  {0x0200, 0x08}, /* fine_integration_time */
  {0x0201, 0x88}, /* fine_integration_time */
  {0x0204, 0x00}, /* analogue_gain_code_global */
  {0x0205, 0xff}, /* analogue_gain_code_global */
  {0x020E, 0x01}, /* digital_gain_greenR */
  {0x020F, 0x00}, /* digital_gain_greenR */
  {0x0210, 0x01}, /* digital_gain_red */
  {0x0211, 0x00}, /* digital_gain_red */
  {0x0212, 0x01}, /* digital_gain_blue */
  {0x0213, 0x00}, /* digital_gain_blue */
  {0x0214, 0x01}, /* digital_gain_greenB */
  {0x0215, 0x00}, /* digital_gain_greenB */
  {0x0300, 0x00}, /* vt_pix_clk_div */
  {0x0301, 0x02}, /* vt_pix_clk_div */
  {0x0302, 0x00}, /* vt_sys_clk_div */
  {0x0303, 0x01}, /* vt_sys_clk_div */
  {0x0304, 0x00}, /* pre_pll_clk_div */
  {0x0305, 0x06}, /* pre_pll_clk_div */
  {0x0308, 0x00}, /* op_pix_clk_div */
  {0x0309, 0x02}, /* op_pix_clk_div */
  {0x030A, 0x00}, /* op_sys_clk_div */
  {0x030B, 0x01}, /* op_sys_clk_div */
  {0x0800, 0x00}, /* tclk_post for D-PHY control */
  {0x0801, 0x00}, /* ths_prepare for D-PHY control */
  {0x0802, 0x00}, /* ths_zero_min for D-PHY control */
  {0x0803, 0x00}, /* ths_trail for D-PHY control */
  {0x0804, 0x00}, /* tclk_trail_min for D-PHY control */
  {0x0805, 0x00}, /* tclk_prepare for D-PHY control */
  {0x0806, 0x00}, /* tclk_zero_zero for D-PHY control */
  {0x0807, 0x00}, /* tlpx for D-PHY control */
  {0x0820, 0x02}, /* requested_link_bit_rate_mbps */
  {0x0821, 0x94}, /* requested_link_bit_rate_mbps */
  {0x0822, 0x00}, /* requested_link_bit_rate_mbps */
  {0x0823, 0x00}, /* requested_link_bit_rate_mbps */
  {0x3000, 0x0A},
  {0x3001, 0xF7},
  {0x3002, 0x0A},
  {0x3003, 0xF7},
  {0x3004, 0x08},
  {0x3005, 0xF8},
  {0x3006, 0x5B},
  {0x3007, 0x73},
  {0x3008, 0x49},
  {0x3009, 0x0C},
  {0x300A, 0xF8},
  {0x300B, 0x4E},
  {0x300C, 0x64},
  {0x300D, 0x5C},
  {0x300E, 0x71},
  {0x300F, 0x0C},
  {0x3010, 0x6A},
  {0x3011, 0x14},
  {0x3012, 0x14},
  {0x3013, 0x0C},
  {0x3014, 0x24},
  {0x3015, 0x4F},
  {0x3016, 0x86},
  {0x3017, 0x0E},
  {0x3018, 0x2C},
  {0x3019, 0x30},
  {0x301A, 0x31},
  {0x301B, 0x32},
  {0x301C, 0xFF},
  {0x301D, 0x33},
  {0x301E, 0x5C},
  {0x301F, 0xFA},
  {0x3020, 0x36},
  {0x3021, 0x46},
  {0x3022, 0x92},
  {0x3023, 0xF5},
  {0x3024, 0x6E},
  {0x3025, 0x19},
  {0x3026, 0x32},
  {0x3027, 0x4B},
  {0x3028, 0x04},
  {0x3029, 0x50},
  {0x302A, 0x0C},
  {0x302B, 0x04},
  {0x302C, 0xEF},
  {0x302D, 0xC1},
  {0x302E, 0x74},
  {0x302F, 0x40},
  {0x3030, 0x00},
  {0x3031, 0x00},
  {0x3032, 0x00},
  {0x3033, 0x00},
  {0x3034, 0x0F},
  {0x3035, 0x01},
  {0x3036, 0x00},
  {0x3037, 0x00},
  {0x3038, 0x88},
  {0x3039, 0x98},
  {0x303A, 0x1F},
  {0x303B, 0x01},
  {0x303C, 0x00},
  {0x303D, 0x03},
  {0x303E, 0x2F},
  {0x303F, 0x09},
  {0x3040, 0xFF},
  {0x3041, 0x22},
  {0x3042, 0x03},
  {0x3043, 0x03},
  {0x3044, 0x20},
  {0x3045, 0x10},
  {0x3046, 0x10},
  {0x3047, 0x08},
  {0x3048, 0x10},
  {0x3049, 0x01},
  {0x304A, 0x00},
  {0x304B, 0x80},
  {0x304C, 0x80},
  {0x304D, 0x00},
  {0x304E, 0x00},
  {0x304F, 0x00},
  {0x3051, 0x09},
  {0x3052, 0xC4},
  {0x305A, 0xE0},
  {0x323D, 0x04},
  {0x323E, 0x38},
  {0x3305, 0xDD},
  {0x3050, 0x01},
  {0x3202, 0x01},
  {0x3203, 0x01},
  {0x3204, 0x01},
  {0x3205, 0x01},
  {0x3206, 0x01},
  {0x3207, 0x01},
  {0x320A, 0x05},
  {0x320B, 0x20},
  {0x3235, 0xB7},
  {0x324C, 0x04},
  {0x324A, 0x07},
  {0x3902, 0x01},
  {0x3915, 0x70},
  {0x3916, 0x80},
  {0x3A00, 0x01},
  {0x3A06, 0x03},
  {0x3B29, 0x01},
  {0x3C11, 0x08},
  {0x3C12, 0x7B},
  {0x3C13, 0xC0},
  {0x3C14, 0x70},
  {0x3C15, 0x80},
  {0x3C20, 0x04},
  {0x3C23, 0x03},
  {0x3C24, 0x00},
  {0x3C50, 0x72},
  {0x3C51, 0x85},
  {0x3C53, 0x40},
  {0x3C55, 0xA0},
  {0x3D00, 0x00},
  {0x3D01, 0x00},
  {0x3D11, 0x01},
  {0x3486, 0x05},
  {0x3B35, 0x06},
  {0x3A05, 0x01},
  {0x3A07, 0x2B},
  {0x3A09, 0x01},
  {0x3940, 0xFF},
  {0x3300, 0x00},
  {0x3900, 0xFF},
  {0x3914, 0x08},
  {0x3A01, 0x0F},
  {0x3A02, 0xA0},
  {0x3A03, 0x0B},
  {0x3A04, 0xC8},
  {0x3701, 0x00},
  {0x3702, 0x00},
  {0x3703, 0x00},
  {0x3704, 0x00},
  {0x0101, 0x00}, /* image_orientation, mirror & flip off*/
  {0x0105, 0x01}, /* mask_corrupted_frames */
  {0x0110, 0x00}, /* CSI-2_channel_identifier */
  {0x3942, 0x01}, /* [0] 1:mipi, 0:pvi */
  {0x0B00, 0x00},
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

static struct msm_camera_csid_vc_cfg s5k3l1yx_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
  {2, CSI_RAW8, CSI_DECODE_DPCM_10_8_10},
};

static struct msm_camera_csi2_params s5k3l1yx_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &s5k3l1yx_cid_cfg[0],
         &s5k3l1yx_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 0x1B,
  },
};

static struct msm_camera_csi2_params s5k3l1yx_csi_dpcm_params = {
  .csid_params = {
    .lane_assign = 0xe4,
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &s5k3l1yx_cid_cfg[2],
         &s5k3l1yx_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 0x1B,
  },
};

static struct sensor_pix_fmt_info_t s5k3l1yx_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SGRBG10 },
};

static struct sensor_pix_fmt_info_t s5k3l1yx_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static struct sensor_pix_fmt_info_t s5k3l1yx_pix_fmt2_fourcc[] = {
  { V4L2_PIX_FMT_SGRBG10DPCM8 },
};

static sensor_stream_info_t s5k3l1yx_stream_info[] = {
  {1, &s5k3l1yx_cid_cfg[0], s5k3l1yx_pix_fmt0_fourcc},
  {1, &s5k3l1yx_cid_cfg[1], s5k3l1yx_pix_fmt1_fourcc},
  {1, &s5k3l1yx_cid_cfg[2], s5k3l1yx_pix_fmt2_fourcc},
};

static sensor_stream_info_array_t s5k3l1yx_stream_info_array = {
  .sensor_stream_info = s5k3l1yx_stream_info,
  .size = ARRAY_SIZE(s5k3l1yx_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  /* full size */
  {0x0501, 0x00}, /* compression_algorithim_L(1d) */
  {0x0112, 0x0A}, /* CCP_data_format_H */
  {0x0113, 0x0A}, /* CCP_data_format_L raw8=0808 ,DCPM10 -->8= 0A08 */
  {0x0306, 0x00}, /* pll_multiplier */
  {0x0307, 0xA5}, /* pll_multiplier */
  {0x0202, 0x09}, /* coarse_integration_time */
  {0x0203, 0x32}, /* coarse_integration_time */
  {0x0340, 0x0B}, /* frame_length_lines */
  {0x0341, 0xEC}, /* frame_length_lines */
  {0x0342, 0x14}, /* line_length_pck */
  {0x0343, 0xD8}, /* line_length_pck */
  {0x0344, 0x00}, /* x_addr_start */
  {0x0345, 0x08}, /* x_addr_start */
  {0x0346, 0x00}, /* y_addr_start */
  {0x0347, 0x00}, /* y_addr_start */
  {0x0348, 0x0F}, /* x_addr_end */
  {0x0349, 0xA7}, /* x_addr_end */
  {0x034A, 0x0B}, /* y_addr_end */
  {0x034B, 0xC7}, /* y_addr_end */
  {0x034C, 0x0F}, /* x_output_size */
  {0x034D, 0xA0}, /* x_output_size */
  {0x034E, 0x0B}, /* y_output_size */
  {0x034F, 0xC8}, /* y_output_size */
  {0x0380, 0x00}, /* x_even_inc */
  {0x0381, 0x01}, /* x_even_inc */
  {0x0382, 0x00}, /* x_odd_inc */
  {0x0383, 0x01}, /* x_odd_inc */
  {0x0384, 0x00}, /* y_even_inc */
  {0x0385, 0x01}, /* y_even_inc */
  {0x0386, 0x00}, /* y_odd_inc */
  {0x0387, 0x01}, /* y_odd_inc */
  {0x0900, 0x00}, /* binning_mode */
  {0x0901, 0x22}, /* binning_type */
  {0x0902, 0x01}, /* binning_weighting */
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  /* 30fps 1/2 * 1/2 */
  {0x0501, 0x00}, /* compression_algorithim_L(1d) */
  {0x0112, 0x0A}, /* CCP_data_format_H */
  {0x0113, 0x0A}, /* CCP_data_format_L raw8=0808 ,DCPM10 -->8= 0A08 */
  {0x0306, 0x00}, /* pll_multiplier */
  {0x0307, 0xA5}, /* pll_multiplier */
  {0x0202, 0x06}, /* coarse_integration_time */
  {0x0203, 0x00}, /* coarse_integration_time */
  {0x0340, 0x09}, /* frame_length_lines */
  {0x0341, 0x6C}, /* frame_length_lines */
  {0x0342, 0x11}, /* line_length_pck */
  {0x0343, 0x80}, /* line_length_pck */
  {0x0344, 0x00}, /* x_addr_start */
  {0x0345, 0x18}, /* x_addr_start */
  {0x0346, 0x00}, /* y_addr_start */
  {0x0347, 0x00}, /* y_addr_start */
  {0x0348, 0x0F}, /* x_addr_end */
  {0x0349, 0x97}, /* x_addr_end */
  {0x034A, 0x0B}, /* y_addr_end */
  {0x034B, 0xC7}, /* y_addr_end */
  {0x034C, 0x07}, /* x_output_size */
  {0x034D, 0xC0}, /* x_output_size */
  {0x034E, 0x05}, /* y_output_size */
  {0x034F, 0xE4}, /* y_output_size */
  {0x0380, 0x00}, /* x_even_inc */
  {0x0381, 0x01}, /* x_even_inc */
  {0x0382, 0x00}, /* x_odd_inc */
  {0x0383, 0x03}, /* x_odd_inc */
  {0x0384, 0x00}, /* y_even_inc */
  {0x0385, 0x01}, /* y_even_inc */
  {0x0386, 0x00}, /* y_odd_inc */
  {0x0387, 0x03}, /* y_odd_inc */
  {0x0900, 0x01}, /* binning_mode */
  {0x0901, 0x22}, /* binning_type */
  {0x0902, 0x01}, /* binning_weighting */
};

static struct msm_camera_i2c_reg_array res2_reg_array[] = {
  {0x0501, 0x00}, /* compression_algorithim_L(1d) */
  {0x0112, 0x0A}, /* CCP_data_format_H */
  {0x0113, 0x0A}, /* CCP_data_format_L raw8=0808 ,DCPM10 -->8= 0A08 */
  {0x0306, 0x00}, /* pll_multiplier */
  {0x0307, 0xA5}, /* pll_multiplier */
  {0x0202, 0x03}, /* coarse_integration_time */
  {0x0203, 0xD8}, /* coarse_integration_time */
  {0x0340, 0x03}, /* frame_length_lines */
  {0x0341, 0xE0}, /* frame_length_lines */
  {0x0342, 0x14}, /* line_length_pck */
  {0x0343, 0xD8}, /* line_length_pck */
  {0x0344, 0x01}, /* x_addr_start */
  {0x0345, 0x20}, /* x_addr_start */
  {0x0346, 0x02}, /* y_addr_start */
  {0x0347, 0x24}, /* y_addr_start */
  {0x0348, 0x0E}, /* x_addr_end */
  {0x0349, 0xA0}, /* x_addr_end */
  {0x034A, 0x09}, /* y_addr_end */
  {0x034B, 0xA4}, /* y_addr_end */
  {0x034C, 0x03}, /* x_output_size */
  {0x034D, 0x60}, /* x_output_size */
  {0x034E, 0x01}, /* y_output_size */
  {0x034F, 0xE0}, /* y_output_size */
  {0x0380, 0x00}, /* x_even_inc */
  {0x0381, 0x01}, /* x_even_inc */
  {0x0382, 0x00}, /* x_odd_inc */
  {0x0383, 0x07}, /* x_odd_inc */
  {0x0384, 0x00}, /* y_even_inc */
  {0x0385, 0x01}, /* y_even_inc */
  {0x0386, 0x00}, /* y_odd_inc */
  {0x0387, 0x07}, /* y_odd_inc */
  {0x0900, 0x01}, /* binning_mode */
  {0x0901, 0x44}, /* binning_type */
  {0x0902, 0x01}, /* binning_weighting */
};

static struct msm_camera_i2c_reg_array res3_reg_array[] = {
  {0x0501, 0x00}, /* compression_algorithim_L(1d) */
  {0x0112, 0x0A}, /* CCP_data_format_H */
  {0x0113, 0x0A}, /* CCP_data_format_L raw8=0808 ,DCPM10 -->8= 0A08 */
  {0x0306, 0x00}, /* pll_multiplier */
  {0x0307, 0xA5}, /* pll_multiplier */
  {0x0202, 0x02}, /* coarse_integration_time */
  {0x0203, 0x90}, /* coarse_integration_time */
  {0x0340, 0x02}, /* frame_length_lines */
  {0x0341, 0x98}, /* frame_length_lines */
  {0x0342, 0x14}, /* line_length_pck */
  {0x0343, 0xD8}, /* line_length_pck */
  {0x0344, 0x01}, /* x_addr_start */
  {0x0345, 0x20}, /* x_addr_start */
  {0x0346, 0x02}, /* y_addr_start */
  {0x0347, 0x24}, /* y_addr_start */
  {0x0348, 0x0E}, /* x_addr_end */
  {0x0349, 0xA0}, /* x_addr_end */
  {0x034A, 0x09}, /* y_addr_end */
  {0x034B, 0xA4}, /* y_addr_end */
  {0x034C, 0x03}, /* x_output_size */
  {0x034D, 0x60}, /* x_output_size */
  {0x034E, 0x01}, /* y_output_size */
  {0x034F, 0xE0}, /* y_output_size */
  {0x0380, 0x00}, /* x_even_inc */
  {0x0381, 0x01}, /* x_even_inc */
  {0x0382, 0x00}, /* x_odd_inc */
  {0x0383, 0x07}, /* x_odd_inc */
  {0x0384, 0x00}, /* y_even_inc */
  {0x0385, 0x01}, /* y_even_inc */
  {0x0386, 0x00}, /* y_odd_inc */
  {0x0387, 0x07}, /* y_odd_inc */
  {0x0900, 0x01}, /* binning_mode */
  {0x0901, 0x44}, /* binning_type */
  {0x0902, 0x01}, /* binning_weighting */
};

static struct msm_camera_i2c_reg_array res4_reg_array[] = {
  {0x0501, 0x00}, /* compression_algorithim_L(1d) */
  {0x0112, 0x0A}, /* CCP_data_format_H */
  {0x0113, 0x0A}, /* CCP_data_format_L raw8=0808 ,DCPM10 -->8= 0A08 */
  {0x0306, 0x00}, /* pll_multiplier */
  {0x0307, 0xA5}, /* pll_multiplier */
  {0x0202, 0x01}, /* coarse_integration_time */
  {0x0203, 0xFA}, /* coarse_integration_time */
  {0x0340, 0x02}, /* frame_length_lines */
  {0x0341, 0x02}, /* frame_length_lines */
  {0x0342, 0x14}, /* line_length_pck */
  {0x0343, 0xD8}, /* line_length_pck */
  {0x0344, 0x01}, /* x_addr_start */
  {0x0345, 0x20}, /* x_addr_start */
  {0x0346, 0x02}, /* y_addr_start */
  {0x0347, 0x24}, /* y_addr_start */
  {0x0348, 0x0E}, /* x_addr_end */
  {0x0349, 0xA0}, /* x_addr_end */
  {0x034A, 0x09}, /* y_addr_end */
  {0x034B, 0xA4}, /* y_addr_end */
  {0x034C, 0x03}, /* x_output_size */
  {0x034D, 0x60}, /* x_output_size */
  {0x034E, 0x01}, /* y_output_size */
  {0x034F, 0xE0}, /* y_output_size */
  {0x0380, 0x00}, /* x_even_inc */
  {0x0381, 0x01}, /* x_even_inc */
  {0x0382, 0x00}, /* x_odd_inc */
  {0x0383, 0x07}, /* x_odd_inc */
  {0x0384, 0x00}, /* y_even_inc */
  {0x0385, 0x01}, /* y_even_inc */
  {0x0386, 0x00}, /* y_odd_inc */
  {0x0387, 0x07}, /* y_odd_inc */
  {0x0900, 0x01}, /* binning_mode */
  {0x0901, 0x44}, /* binning_type */
  {0x0902, 0x01}, /* binning_weighting */
};

static struct msm_camera_i2c_reg_array res5_reg_array[] = {
  {0x0501, 0x01}, /* compression_algorithim_L(1d) */
  {0x0112, 0x0A}, /* CCP_data_format_H */
  {0x0113, 0x08}, /* CCP_data_format_L raw8=0808 ,DCPM10 -->8= 0A08 */
  {0x0306, 0x00}, /* pll_multiplier */
  {0x0307, 0xA0}, /* pll_multiplier */
  {0x0202, 0x09}, /* coarse_integration_time */
  {0x0203, 0x32}, /* coarse_integration_time */
  {0x0340, 0x0B}, /* frame_length_lines */
  {0x0341, 0xEC}, /* frame_length_lines */
  {0x0342, 0x11}, /* line_length_pck */
  {0x0343, 0x80}, /* line_length_pck */
  {0x0344, 0x00}, /* x_addr_start */
  {0x0345, 0x08}, /* x_addr_start */
  {0x0346, 0x00}, /* y_addr_start */
  {0x0347, 0x00}, /* y_addr_start */
  {0x0348, 0x0F}, /* x_addr_end */
  {0x0349, 0xA7}, /* x_addr_end */
  {0x034A, 0x0B}, /* y_addr_end */
  {0x034B, 0xC7}, /* y_addr_end */
  {0x034C, 0x0F}, /* x_output_size */
  {0x034D, 0xA0}, /* x_output_size */
  {0x034E, 0x0B}, /* y_output_size */
  {0x034F, 0xC8}, /* y_output_size */
  {0x0380, 0x00}, /* x_even_inc */
  {0x0381, 0x01}, /* x_even_inc */
  {0x0382, 0x00}, /* x_odd_inc */
  {0x0383, 0x01}, /* x_odd_inc */
  {0x0384, 0x00}, /* y_even_inc */
  {0x0385, 0x01}, /* y_even_inc */
  {0x0386, 0x00}, /* y_odd_inc */
  {0x0387, 0x01}, /* y_odd_inc */
  {0x0900, 0x00}, /* binning_mode */
  {0x0901, 0x22}, /* binning_type */
  {0x0902, 0x01}, /* binning_weighting */
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
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct msm_camera_csi2_params *csi_params[] = {
  &s5k3l1yx_csi_params, /* RES 0*/
  &s5k3l1yx_csi_params, /* RES 1*/
  &s5k3l1yx_csi_params, /* RES 2*/
  &s5k3l1yx_csi_params, /* RES 3*/
  &s5k3l1yx_csi_params, /* RES 4*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
  {0, 0, 0, 0}, /* RES 1 */
  {0, 0, 0, 0}, /* RES 2 */
  {0, 0, 0, 0}, /* RES 3 */
  {0, 0, 0, 0}, /* RES 4 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
    /* full size */
    .x_output = 4000,
    .y_output = 3016,
    .line_length_pclk = 5336,
    .frame_length_lines = 3052,
#ifndef VFE_40
    .vt_pixel_clk = 330000000,
#else
    .vt_pixel_clk = 329600000,
#endif
    .op_pixel_clk = 264000000,
    .binning_factor = 1,
    .max_fps = 20.2,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    /* 30 fps 1/2 * 1/2 */
    .x_output = 1984,
    .y_output = 1508,
    .line_length_pclk = 6980,
    .frame_length_lines = 1548,
#ifndef VFE_40
    .vt_pixel_clk = 330000000,
#else
    .vt_pixel_clk = 329600000,
#endif
    .op_pixel_clk = 264000000,
    .binning_factor = 1,
    .max_fps = 30.5,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
  {
    .x_output = 864,
    .y_output = 480,
    .line_length_pclk = 5336,
    .frame_length_lines = 992,
    .vt_pixel_clk = 330000000,
    .op_pixel_clk = 264000000,
    .binning_factor = 1,
    .max_fps = 62.34,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    .x_output = 864,
    .y_output = 480,
    .line_length_pclk = 5336,
    .frame_length_lines = 664,
    .vt_pixel_clk = 330000000,
    .op_pixel_clk = 264000000,
    .binning_factor = 1,
    .max_fps = 93.13,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    .x_output = 864,
    .y_output = 480,
    .line_length_pclk = 5336,
    .frame_length_lines = 514,
    .vt_pixel_clk = 330000000,
    .op_pixel_clk = 264000000,
    .binning_factor = 1,
    .max_fps = 120.31,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t s5k3l1yx_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t s5k3l1yx_res_table = {
  .res_cfg_type = s5k3l1yx_res_cfg,
  .size = ARRAY_SIZE(s5k3l1yx_res_cfg),
};

static struct sensor_lib_chromatix_t s5k3l1yx_chromatix[] = {
  {
    .common_chromatix = S5K3L1YX_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K3L1YX_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camera_snapshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = S5K3L1YX_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
  {
    .common_chromatix = S5K3L1YX_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K3L1YX_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix = S5K3L1YX_LOAD_CHROMATIX(preview), /* RES1 */
    .liveshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
  {
    .common_chromatix = S5K3L1YX_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_60fps), /* RES2 */
    .camera_snapshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_60fps), /* RES2 */
    .camcorder_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_60fps), /* RES2 */
    .liveshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(liveshot), /* RES2 */
  },
  {
    .common_chromatix = S5K3L1YX_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_90fps), /* RES3 */
    .camera_snapshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_90fps), /* RES3 */
    .camcorder_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_90fps), /* RES3 */
    .liveshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(liveshot), /* RES3 */
  },
  {
    .common_chromatix = S5K3L1YX_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_120fps), /* RES4 */
    .camera_snapshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_120fps), /* RES4 */
    .camcorder_chromatix = S5K3L1YX_LOAD_CHROMATIX(hfr_120fps), /* RES4 */
    .liveshot_chromatix = S5K3L1YX_LOAD_CHROMATIX(liveshot), /* RES4 */
  },
};

static struct sensor_lib_chromatix_array s5k31lyx_lib_chromatix_array = {
  .sensor_lib_chromatix = s5k3l1yx_chromatix,
  .size = ARRAY_SIZE(s5k3l1yx_chromatix),
};

/*===========================================================================
 * FUNCTION    - s5k3l1yx_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t s5k3l1yx_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if (gain < 1.0)
    gain = 1.0;

  if (gain > 16.0)
    gain = 16.0;

  reg_gain = (uint16_t)(gain * 32.0);

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - s5k3l1yx_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float s5k3l1yx_register_to_real_gain(uint16_t reg_gain)
{
  float gain;

  if (reg_gain > 0x0200)
    reg_gain = 0x0200;

  gain = (float) reg_gain / 32.0;

  return gain;
}

/*===========================================================================
 * FUNCTION    - s5k3l1yx_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t s5k3l1yx_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = s5k3l1yx_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain =
    s5k3l1yx_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - s5k3l1yx_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t s5k3l1yx_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting* reg_setting)
{
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
  return 0;
}

static sensor_exposure_table_t s5k3l1yx_expsoure_tbl = {
  .sensor_calculate_exposure = s5k3l1yx_calculate_exposure,
  .sensor_fill_exposure_array = s5k3l1yx_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
  .actuator_name = "dw9716",
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
  .exposure_table_size = 8,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = s5k3l1yx_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(s5k3l1yx_cid_cfg),
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
  .sensor_res_cfg_table = &s5k3l1yx_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &s5k3l1yx_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &s5k3l1yx_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &s5k31lyx_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - s5k3l1yx_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *s5k3l1yx_open_lib(void)
{
  return &sensor_lib_ptr;
}
