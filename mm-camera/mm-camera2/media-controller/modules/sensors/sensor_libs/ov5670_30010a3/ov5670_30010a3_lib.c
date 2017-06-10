/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"
#include <utils/Log.h>

#define SENSOR_MODEL_NO_OV5670_30010A3 "ov5670_30010a3"
#define OV5670_30010A3_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV5670_30010A3"_"#n".so"

static sensor_lib_t sensor_lib_ptr;

#define SNAPSHOT_PARAM    1
#define PREVIEW_PARAM     1
#define VIDEO_1080P_PARAM 0

#define LOG_TAG "OV5670"

static struct msm_sensor_power_setting ov5670_30010a3_power_setting[] = {
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
  .slave_addr = 0x6D,
  /* sensor i2c frequency*/
  .i2c_freq_mode = I2C_FAST_MODE,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x300b,
    /* sensor id */
    .sensor_id = 0x5670,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = ov5670_30010a3_power_setting,
    .size = ARRAY_SIZE(ov5670_30010a3_power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = CAMERA_MODE_2D_B,
  .position = BACK_CAMERA_B,
  .sensor_mount_angle = SENSOR_MOUNTANGLE_90,
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
  .global_gain_addr = 0x3508,
  .vert_offset = 4,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 15.5,
  .max_linecount = 26880,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.26,
  .pix_size = 1.4,
  .f_number = 2.2,
  .total_f_dist = 1.2,
  .hor_view_angle = 64.8,
  .ver_view_angle = 50.7,
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
  .csi_lane_mask = 0x7,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103, 0x01, 0x00},
};

static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x0300, 0x04, 0x00}, // PLL
  {0x0301, 0x00, 0x00},
  {0x0302, 0x69, 0x00},
  {0x0303, 0x00, 0x00},
  {0x0304, 0x03, 0x00},
  {0x0305, 0x01, 0x00},
  {0x0306, 0x01, 0x00},
  {0x030a, 0x00, 0x00},
  {0x030b, 0x00, 0x00},
  {0x030c, 0x00, 0x00},
  {0x030d, 0x1e, 0x00},
  {0x030e, 0x00, 0x00},
  {0x030f, 0x06, 0x00},
  {0x0312, 0x01, 0x00}, // PLL
  {0x3000, 0x00, 0x00}, // Fsin/Vsync input
  {0x3002, 0x21, 0x00}, // ULPM output
  {0x3005, 0xf0, 0x00}, // sclk_psram on, sclk_syncfifo on
  {0x3007, 0x00, 0x00}, //
  {0x3015, 0x0f, 0x00}, // npump clock div = 1, disable Ppumu_clk
  {0x3018, 0x32, 0x00}, // MIPI 2 lane
  {0x301a, 0xf0, 0x00}, // sclk_stb on, sclk_ac on, slck_tc on
  {0x301b, 0xf0, 0x00}, // sclk_blc on, sclk_isp on, sclk_testmode on, sclk_vfifo on
  {0x301c, 0xf0, 0x00}, // sclk_mipi on, sclk_dpcm on, sclk_otp on
  {0x301d, 0xf0, 0x00}, // sclk_asram_tst on, sclk_grp on, sclk_bist on,
  {0x301e, 0xf0, 0x00}, // sclk_ilpwm on, sclk_lvds on, sclk-vfifo on, sclk_mipi on,
  {0x3030, 0x00, 0x00}, // sclk normal, pclk normal
  {0x3031, 0x0a, 0x00}, // 10-bit mode
  {0x303c, 0xff, 0x00}, // reserved
  {0x303e, 0xff, 0x00}, // reserved
  {0x3040, 0xf0, 0x00}, // sclk_isp_fc_en, sclk_fc-en, sclk_tpm_en, sclk_fmt_en
  {0x3041, 0x00, 0x00}, // reserved
  {0x3042, 0xf0, 0x00}, // reserved
  {0x3106, 0x11, 0x00}, // sclk_div = 1, sclk_pre_div = 1
  {0x3500, 0x00, 0x00}, // exposure H
  {0x3501, 0x3d, 0x00}, // exposure M
  {0x3502, 0x00, 0x00}, // exposure L
  {0x3503, 0x04, 0x00}, // gain no delay, use sensor gain
  {0x3504, 0x03, 0x00}, // exposure manual, gain manual
  {0x3505, 0x83, 0x00}, // sensor gain fixed bit
  {0x3508, 0x07, 0x00}, // gain H
  {0x3509, 0x80, 0x00}, // gain L
  {0x350e, 0x04, 0x00}, // short digital gain H
  {0x350f, 0x00, 0x00}, // short digital gain L
  {0x3510, 0x00, 0x00}, // short exposure H
  {0x3511, 0x02, 0x00}, // short exposure M
  {0x3512, 0x00, 0x00}, // short exposure L
  {0x3601, 0xc8, 0x00}, // analog control
  {0x3610, 0x88, 0x00},
  {0x3612, 0x48, 0x00},
  {0x3614, 0x5b, 0x00},
  {0x3615, 0x96, 0x00},
  {0x3621, 0xd0, 0x00},
  {0x3622, 0x00, 0x00},
  {0x3623, 0x00, 0x00},
  {0x3633, 0x13, 0x00},
  {0x3634, 0x13, 0x00},
  {0x3635, 0x13, 0x00},
  {0x3636, 0x13, 0x00},
  {0x3645, 0x13, 0x00},
  {0x3646, 0x82, 0x00},
  {0x3650, 0x00, 0x00},
  {0x3652, 0xff, 0x00},
  {0x3655, 0x20, 0x00},
  {0x3656, 0xff, 0x00},
  {0x365a, 0xff, 0x00},
  {0x365e, 0xff, 0x00},
  {0x3668, 0x00, 0x00},
  {0x366a, 0x07, 0x00},
  {0x366e, 0x08, 0x00},
  {0x366d, 0x00, 0x00},
  {0x366f, 0x80, 0x00}, // analog control
  {0x3700, 0x28, 0x00}, // sensor control
  {0x3701, 0x10, 0x00},
  {0x3702, 0x3a, 0x00},
  {0x3703, 0x19, 0x00},
  {0x3704, 0x10, 0x00},
  {0x3705, 0x00, 0x00},
  {0x3706, 0x66, 0x00},
  {0x3707, 0x08, 0x00},
  {0x3708, 0x34, 0x00},
  {0x3709, 0x40, 0x00},
  {0x370a, 0x01, 0x00},
  {0x370b, 0x1b, 0x00},
  {0x3714, 0x24, 0x00},
  {0x371a, 0x3e, 0x00},
  {0x3733, 0x00, 0x00},
  {0x3734, 0x00, 0x00},
  {0x373a, 0x05, 0x00},
  {0x373b, 0x06, 0x00},
  {0x373c, 0x0a, 0x00},
  {0x373f, 0xa0, 0x00},
  {0x3755, 0x00, 0x00},
  {0x3758, 0x00, 0x00},
  {0x375b, 0x0e, 0x00},
  {0x3766, 0x5f, 0x00},
  {0x3768, 0x00, 0x00},
  {0x3769, 0x22, 0x00},
  {0x3773, 0x08, 0x00},
  {0x3774, 0x1f, 0x00},
  {0x3776, 0x06, 0x00},
  {0x37a0, 0x88, 0x00},
  {0x37a1, 0x5c, 0x00},
  {0x37a7, 0x88, 0x00},
  {0x37a8, 0x70, 0x00},
  {0x37aa, 0x88, 0x00},
  {0x37ab, 0x48, 0x00},
  {0x37b3, 0x66, 0x00},
  {0x37c2, 0x04, 0x00},
  {0x37c5, 0x00, 0x00},
  {0x37c8, 0x00, 0x00}, // sensor control
  {0x3800, 0x00, 0x00}, // x addr start H
  {0x3801, 0x0c, 0x00}, // x addr start L
  {0x3802, 0x00, 0x00}, // y addr start H
  {0x3803, 0x04, 0x00}, // y addr start L
  {0x3804, 0x0a, 0x00}, // x addr end H
  {0x3805, 0x33, 0x00}, // x addr end L
  {0x3806, 0x07, 0x00}, // y addr end H
  {0x3807, 0xa3, 0x00}, // y addr end L
  {0x3808, 0x05, 0x00}, // x output size H
  {0x3809, 0x10, 0x00}, // x outout size L
  {0x380a, 0x03, 0x00}, // y output size H
  {0x380b, 0xc0, 0x00}, // y output size L
  {0x380c, 0x06, 0x00}, // HTS H
  {0x380d, 0x8c, 0x00}, // HTS L
  {0x380e, 0x07, 0x00}, // VTS H
  {0x380f, 0xfd, 0x00}, // VTS L
  {0x3811, 0x04, 0x00}, // ISP x win L
  {0x3813, 0x02, 0x00}, // ISP y win L
  {0x3814, 0x03, 0x00}, // x inc odd
  {0x3815, 0x01, 0x00}, // x inc even
  {0x3816, 0x00, 0x00}, // vsync start H
  {0x3817, 0x00, 0x00}, // vsync star L
  {0x3818, 0x00, 0x00}, // vsync end H
  {0x3819, 0x00, 0x00}, // vsync end L
  {0x3820, 0x90, 0x00}, // vsyn48_blc on, vflip off
  {0x3821, 0x47, 0x00}, // hsync_en_o, mirror on, dig_bin on
  {0x3822, 0x48, 0x00}, // addr0_num[3:1]=0x02, ablc_num[5:1]=0x08
  {0x3826, 0x00, 0x00}, // r_rst_fsin H
  {0x3827, 0x08, 0x00}, // r_rst_fsin L
  {0x382a, 0x03, 0x00}, // y inc odd
  {0x382b, 0x01, 0x00}, // y inc even
  {0x3830, 0x08, 0x00},
  {0x3836, 0x02, 0x00},
  {0x3837, 0x00, 0x00},
  {0x3838, 0x10, 0x00},
  {0x3841, 0xff, 0x00},
  {0x3846, 0x48, 0x00},
  {0x3861, 0x00, 0x00},
  {0x3862, 0x04, 0x00},
  {0x3863, 0x06, 0x00},
  {0x3a11, 0x01, 0x00},
  {0x3a12, 0x78, 0x00},
  {0x3b00, 0x00, 0x00}, // strobe
  {0x3b02, 0x00, 0x00},
  {0x3b03, 0x00, 0x00},
  {0x3b04, 0x00, 0x00},
  {0x3b05, 0x00, 0x00}, // strobe
  {0x3c00, 0x89, 0x00},
  {0x3c01, 0xab, 0x00},
  {0x3c02, 0x01, 0x00},
  {0x3c03, 0x00, 0x00},
  {0x3c04, 0x00, 0x00},
  {0x3c05, 0x03, 0x00},
  {0x3c06, 0x00, 0x00},
  {0x3c07, 0x05, 0x00},
  {0x3c0c, 0x00, 0x00},
  {0x3c0d, 0x00, 0x00},
  {0x3c0e, 0x00, 0x00},
  {0x3c0f, 0x00, 0x00},
  {0x3c40, 0x00, 0x00},
  {0x3c41, 0xa3, 0x00},
  {0x3c43, 0x7d, 0x00},
  {0x3c45, 0xd7, 0x00},
  {0x3c47, 0xfc, 0x00},
  {0x3c50, 0x05, 0x00},
  {0x3c52, 0xaa, 0x00},
  {0x3c54, 0x71, 0x00},
  {0x3c56, 0x80, 0x00},
  {0x3d85, 0x17, 0x00},
  {0x3f03, 0x00, 0x00}, // PSRAM
  {0x3f0a, 0x00, 0x00},
  {0x3f0b, 0x00, 0x00}, // PSRAM
  {0x4001, 0x60, 0x00}, // BLC, K enable
  {0x4009, 0x05, 0x00}, // BLC, black line end line
  {0x4020, 0x00, 0x00}, // BLC, offset compensation th000
  {0x4021, 0x00, 0x00}, // BLC, offset compensation K000
  {0x4022, 0x00, 0x00},
  {0x4023, 0x00, 0x00},
  {0x4024, 0x00, 0x00},
  {0x4025, 0x00, 0x00},
  {0x4026, 0x00, 0x00},
  {0x4027, 0x00, 0x00},
  {0x4028, 0x00, 0x00},
  {0x4029, 0x00, 0x00},
  {0x402a, 0x00, 0x00},
  {0x402b, 0x00, 0x00},
  {0x402c, 0x00, 0x00},
  {0x402d, 0x00, 0x00},
  {0x402e, 0x00, 0x00},
  {0x402f, 0x00, 0x00},
  {0x4040, 0x00, 0x00},
  {0x4041, 0x03, 0x00},
  {0x4042, 0x00, 0x00},
  {0x4043, 0x7a, 0x00},
  {0x4044, 0x00, 0x00},
  {0x4045, 0x7a, 0x00},
  {0x4046, 0x00, 0x00},
  {0x4047, 0x7a, 0x00},
  {0x4048, 0x00, 0x00}, // BLC, kcoef_r_man H
  {0x4049, 0x80, 0x00}, // BLC, kcoef_r_man L
  {0x4303, 0x00, 0x00},
  {0x4307, 0x30, 0x00},
  {0x4500, 0x58, 0x00},
  {0x4501, 0x04, 0x00},
  {0x4502, 0x40, 0x00},
  {0x4503, 0x10, 0x00},
  {0x4508, 0x55, 0x00},
  {0x4509, 0x55, 0x00},
  {0x450a, 0x00, 0x00},
  {0x450b, 0x00, 0x00},
  {0x4600, 0x00, 0x00},
  {0x4601, 0x81, 0x00},
  {0x4700, 0xa4, 0x00},
  {0x4800, 0x4c, 0x00}, // MIPI conrol
  {0x4816, 0x53, 0x00}, // emb_dt
  {0x481f, 0x40, 0x00}, // clock_prepare_min
  {0x4837, 0x13, 0x00}, // clock period of pclk2x
  {0x5000, 0x56, 0x00}, // awb_gain_en, bc_en, wc_en
  {0x5001, 0x01, 0x00}, // blc_en
  {0x5002, 0x28, 0x00}, // otp_dpc_en
  {0x5004, 0x0c, 0x00}, // ISP size auto control enable
  {0x5006, 0x0c, 0x00},
  {0x5007, 0xe0, 0x00},
  {0x5008, 0x01, 0x00},
  {0x5009, 0xb0, 0x00},
  {0x5901, 0x00, 0x00}, // VAP
  {0x5a01, 0x00, 0x00}, // WINC x start offset H
  {0x5a03, 0x00, 0x00}, // WINC x start offset L
  {0x5a04, 0x0c, 0x00}, // WINC y start offset H
  {0x5a05, 0xe0, 0x00}, // WINC y start offset L
  {0x5a06, 0x09, 0x00}, // WINC window width H
  {0x5a07, 0xb0, 0x00}, // WINC window width L
  {0x5a08, 0x06, 0x00}, // WINC window height H
  {0x5e00, 0x00, 0x00}, // WINC window height L
  {0x3618, 0x2a, 0x00},
  //Ally031414
  {0x3734, 0x40, 0x00}, // Improve HFPN
  {0x5b00, 0x01, 0x00}, // [2:0] otp start addr[10:8]
  {0x5b01, 0x10, 0x00}, // [7:0] otp start addr[7:0]
  {0x5b02, 0x01, 0x00}, // [2:0] otp end addr[10:8]
  {0x5b03, 0xDB, 0x00}, // [7:0] otp end addr[7:0]
  {0x3d8c, 0x71, 0x00}, //Header address high byte
  {0x3d8d, 0xEA, 0x00}, //Header address low byte
  {0x4017, 0x10, 0x00}, //threshold = 4LSB for Binning sum format.
  //Strong DPC1.53
  {0x5780, 0x3e, 0x00},
  {0x5781, 0x0f, 0x00},
  {0x5782, 0x44, 0x00},
  {0x5783, 0x02, 0x00},
  {0x5784, 0x01, 0x00},
  {0x5785, 0x01, 0x00},
  {0x5786, 0x00, 0x00},
  {0x5787, 0x04, 0x00},
  {0x5788, 0x02, 0x00},
  {0x5789, 0x0f, 0x00},
  {0x578a, 0xfd, 0x00},
  {0x578b, 0xf5, 0x00},
  {0x578c, 0xf5, 0x00},
  {0x578d, 0x03, 0x00},
  {0x578e, 0x08, 0x00},
  {0x578f, 0x0c, 0x00},
  {0x5790, 0x08, 0x00},
  {0x5791, 0x04, 0x00},
  {0x5792, 0x00, 0x00},
  {0x5793, 0x52, 0x00},
  {0x5794, 0xa3, 0x00},
  //Ping
  {0x3503, 0x30, 0x00}, // exposure gain/exposure delay not used
  //added
  {0x3d85, 0x17, 0x00}, // OTP power up load data enable,
  {0x3655, 0x20, 0x00},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 50,
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
  {0x3208, 0x00, 0x00},
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

static struct msm_camera_csid_vc_cfg ov5670_30010a3_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov5670_30010a3_csi_params = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = ARRAY_SIZE(ov5670_30010a3_cid_cfg),
      .vc_cfg = {
         &ov5670_30010a3_cid_cfg[0],
         &ov5670_30010a3_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x1b,
  },
};

static struct sensor_pix_fmt_info_t ov5670_30010a3_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov5670_30010a3_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov5670_30010a3_stream_info[] = {
  {1, &ov5670_30010a3_cid_cfg[0], ov5670_30010a3_pix_fmt0_fourcc},
  {1, &ov5670_30010a3_cid_cfg[1], ov5670_30010a3_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov5670_30010a3_stream_info_array = {
  .sensor_stream_info = ov5670_30010a3_stream_info,
  .size = ARRAY_SIZE(ov5670_30010a3_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
//Capture 2592x1944 30fps 24M MCLK 2lane 960Mbps/lane
  {0x3501, 0x7b, 0x00}, // exposore M
  {0x3623, 0x00, 0x00}, // analog control
  {0x366e, 0x10, 0x00}, // analog control
  {0x370b, 0x1b, 0x00}, // sensor control
  {0x3808, 0x0a, 0x00}, // x output size H
  {0x3809, 0x20, 0x00}, // x output size L
  {0x380a, 0x07, 0x00}, // y outout size H
  {0x380b, 0x98, 0x00}, // y output size L
  {0x380c, 0x06, 0x00}, // HTS H
  {0x380d, 0x8c, 0x00}, // HTS L
  {0x380e, 0x07, 0x00}, // VTS H
  {0x380f, 0xfd, 0x00}, // VTS L
  {0x3814, 0x01, 0x00}, // x inc odd
  {0x3820, 0x80, 0x00}, // vflip off
  {0x3821, 0x46, 0x00}, // hsync_en_o, mirror on, dig_bin off
  {0x382a, 0x01, 0x00}, // y inc odd
  {0x4009, 0x0d, 0x00}, // BLC, black line end line
  {0x400a, 0x02, 0x00}, // BLC, offset trigger threshold H
  {0x400b, 0x00, 0x00}, // BLC, offset trigger threshold L
  {0x4502, 0x40, 0x00}, //
  {0x4508, 0xaa, 0x00}, //
  {0x4509, 0xaa, 0x00}, //
  {0x450a, 0x00, 0x00}, //
  {0x4600, 0x01, 0x00}, //
  {0x4601, 0x03, 0x00}, //
  {0x4017, 0x08, 0x00}, // BLC, offset trigger threshold
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
//Preview 1296x972 30fps 24M MCLK 2lane 960Mbps/lane
  {0x3501, 0x3d, 0x00}, //exposure M
  {0x3623, 0x00, 0x00}, //analog control
  {0x366e, 0x08, 0x00}, //analog control
  {0x370b, 0x1b, 0x00}, //sensor control
  {0x3808, 0x05, 0x00}, //x output size H
  {0x3809, 0x10, 0x00}, //x output size L
  {0x380a, 0x03, 0x00}, //y outout size H
  {0x380b, 0xcc, 0x00}, //y output size L
  {0x380c, 0x06, 0x00}, //HTS H
  {0x380d, 0x8c, 0x00}, //HTS L
  {0x380e, 0x07, 0x00}, //VTS H
  {0x380f, 0xfd, 0x00}, //VTS L
  {0x3814, 0x03, 0x00}, //x inc odd
  {0x3820, 0x90, 0x00}, //vsyn48_blc on, vflip off
  {0x3821, 0x47, 0x00}, //hsync_en_o, mirror on, dig_bin on
  {0x382a, 0x03, 0x00}, //y inc odd
  {0x4009, 0x05, 0x00}, //BLC, black line end line
  {0x400a, 0x02, 0x00}, //BLC, offset trigger threshold H
  {0x400b, 0x00, 0x00}, //BLC, offset trigger threshold L
  {0x4502, 0x44, 0x00},
  {0x4508, 0x55, 0x00},
  {0x4509, 0x55, 0x00},
  {0x450a, 0x00, 0x00},
  {0x4600, 0x00, 0x00},
  {0x4601, 0x81, 0x00},
  {0x4017, 0x10, 0x00}, //BLC, offset trigger threshold
};

static struct msm_camera_i2c_reg_array res2_reg_array[] = {
//Video 1080p crop 30fps 24M MCLK 2lane 960Mbps/lane
  {0x3501, 0x45, 0x00}, // exposure M
  {0x3623, 0x00, 0x00}, // analog control
  {0x366e, 0x10, 0x00}, // analog control
  {0x370b, 0x05, 0x00}, // sensor control
  {0x3808, 0x07, 0x00}, // x output size H
  {0x3809, 0x80, 0x00}, // x output size L
  {0x380a, 0x04, 0x00}, // y outout size H
  {0x380b, 0x38, 0x00}, // y output size L
  {0x380c, 0x05, 0x00}, // HTS H
  {0x380d, 0x74, 0x00}, // HTS L
  {0x380e, 0x09, 0x00}, // VTS H
  {0x380f, 0x98, 0x00}, // VTS L
  {0x3814, 0x01, 0x00}, // x inc odd
  {0x3820, 0x80, 0x00}, // vflip off
  {0x3821, 0x46, 0x00}, // hsync_en_o, mirror on, dig_bin off
  {0x382a, 0x01, 0x00}, // y inc odd
  {0x4009, 0x0d, 0x00}, // BLC, black line end line
  {0x400a, 0x01, 0x00}, // BLC, offset trigger threshold H
  {0x400b, 0x62, 0x00}, // BLC, offset trigger threshold L
  {0x4502, 0x44, 0x00},
  {0x4508, 0xaa, 0x00},
  {0x4509, 0xaa, 0x00},
  {0x450a, 0x00, 0x00},
  {0x4600, 0x00, 0x00},
  {0x4601, 0xc0, 0x00},
  {0x4017, 0x08, 0x00}, // BLC, offset trigger threshold
};
static struct msm_camera_i2c_reg_array res3_reg_array[] = {
//Preview 1296x960 60fps 24M MCLK 2lane 960Mbps/lane
  {0x3501, 0x3d, 0x00}, //exposure M
  {0x3623, 0x00, 0x00}, //analog control
  {0x366e, 0x08, 0x00}, //analog control
  {0x370b, 0x1b, 0x00}, //sensor control
  {0x3808, 0x05, 0x00}, //x output size H
  {0x3809, 0x10, 0x00}, //x output size L
  {0x380a, 0x03, 0x00}, //y outout size H
  {0x380b, 0xc0, 0x00}, //y output size L
  {0x380c, 0x06, 0x00}, //HTS H
  {0x380d, 0xc0, 0x00}, //HTS L
  {0x380e, 0x03, 0x00}, //VTS H
  {0x380f, 0xe0, 0x00}, //VTS L
  {0x3814, 0x03, 0x00}, //x inc odd
  {0x3820, 0x90, 0x00}, //vsyn48_blc on, vflip off
  {0x3821, 0x47, 0x00}, //hsync_en_o, mirror on, dig_bin on
  {0x382a, 0x03, 0x00}, //y inc odd
  {0x4009, 0x05, 0x00}, //BLC, black line end line
  {0x400a, 0x02, 0x00}, //BLC, offset trigger threshold H
  {0x400b, 0x00, 0x00}, //BLC, offset trigger threshold L
  {0x4502, 0x44, 0x00},
  {0x4508, 0x55, 0x00},
  {0x4509, 0x55, 0x00},
  {0x450a, 0x00, 0x00},
  {0x4600, 0x00, 0x00},
  {0x4601, 0x81, 0x00},
  {0x4017, 0x10, 0x00}, //BLC, offset trigger threshold
};
static struct msm_camera_i2c_reg_array res4_reg_array[] = {
//Video 640x480 90fps 24M MCLK 2lane 960Mbps/lane
  {0x3501, 0x1f, 0x00}, //exposure M
  {0x3623, 0x04, 0x00}, //analog control
  {0x366e, 0x08, 0x00}, //analog control
  {0x370b, 0x1b, 0x00}, //sensor control
  {0x3808, 0x02, 0x00}, //x output size H
  {0x3809, 0x80, 0x00}, //x output size L
  {0x380a, 0x01, 0x00}, //y outout size H
  {0x380b, 0xe0, 0x00}, //y output size L
  {0x380c, 0x06, 0x00}, //HTS H
  {0x380d, 0x8c, 0x00}, //HTS L
  {0x380e, 0x02, 0x00}, //VTS H
  {0x380f, 0xa0, 0x00}, //VTS L
  {0x3814, 0x07, 0x00}, //x inc odd
  {0x3820, 0x90, 0x00}, //vsyn48_blc on, vflip off
  {0x3821, 0xc6, 0x00}, //dig_hbin4 on, hsync_en_o, mirror on, dig_bin off
  {0x382a, 0x07, 0x00}, //y inc odd
  {0x4009, 0x05, 0x00}, //BLC, black line end line
  {0x400a, 0x02, 0x00}, //BLC, offset trigger threshold H
  {0x400b, 0x00, 0x00}, //BLC, offset trigger threshold L
  {0x4502, 0x44, 0x00},
  {0x4508, 0x55, 0x00},
  {0x4509, 0x55, 0x00},
  {0x450a, 0x03, 0x00},
  {0x4600, 0x00, 0x00},
  {0x4601, 0x40, 0x00},
  {0x4017, 0x10, 0x00}, //BLC, offset trigger threshold
};
static struct msm_camera_i2c_reg_setting res_settings[] = {
#if SNAPSHOT_PARAM
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
#if PREVIEW_PARAM
  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
#if VIDEO_1080P_PARAM
  {
    .reg_setting = res2_reg_array,
    .size = ARRAY_SIZE(res2_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
  {
    .reg_setting = res3_reg_array,
    .size = ARRAY_SIZE(res3_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res4_reg_array,
    .size = ARRAY_SIZE(res3_reg_array),
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
  &ov5670_30010a3_csi_params, /* RES 0*/
  &ov5670_30010a3_csi_params, /* RES 1*/
  &ov5670_30010a3_csi_params, /* RES 2*/
  &ov5670_30010a3_csi_params, /* RES 3*/
  &ov5670_30010a3_csi_params, /* RES 4*/
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
#if SNAPSHOT_PARAM
  {
    .x_output = 2592,
    .y_output = 1944,
    .line_length_pclk = 1676,
    .frame_length_lines = 2045,
    .vt_pixel_clk = 102822600,
    .op_pixel_clk = 168000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if PREVIEW_PARAM
  {
    .x_output = 1296,
    .y_output = 972,
    .line_length_pclk = 1676,
    .frame_length_lines = 2045,
    .vt_pixel_clk = 102822600,
    .op_pixel_clk = 168000000,
    .binning_factor = 2,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if VIDEO_1080P_PARAM
  {
    .x_output = 1920,
    .y_output = 1080,
    .line_length_pclk = 1396,
    .frame_length_lines = 2456,
    .vt_pixel_clk = 102860000,
    .op_pixel_clk = 168000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
  {
    .x_output = 1296,
    .y_output = 960,
    .line_length_pclk = 1728,
    .frame_length_lines = 992,
    .vt_pixel_clk = 102850000,
    .op_pixel_clk = 168000000,
    .binning_factor = 1,
    .max_fps = 60.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
  {
    .x_output = 640,
    .y_output = 480,
    .line_length_pclk = 1676,
    .frame_length_lines = 672,
    .vt_pixel_clk = 101400000,
    .op_pixel_clk = 168000000,
    .binning_factor = 1,
    .max_fps = 90.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov5670_30010a3_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov5670_30010a3_res_table = {
  .res_cfg_type = ov5670_30010a3_res_cfg,
  .size = ARRAY_SIZE(ov5670_30010a3_res_cfg),
};

static struct sensor_lib_chromatix_t ov5670_30010a3_chromatix[] = {
#if SNAPSHOT_PARAM
  {
    .common_chromatix = OV5670_30010A3_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5670_30010A3_LOAD_CHROMATIX(zsl), /* RES0 */
    .camera_snapshot_chromatix = OV5670_30010A3_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = OV5670_30010A3_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix =  OV5670_30010A3_LOAD_CHROMATIX(liveshot), /* RES0 */
  },
#endif
#if PREVIEW_PARAM
  {
    .common_chromatix = OV5670_30010A3_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5670_30010A3_LOAD_CHROMATIX(preview), /* RES1 */
    .camera_snapshot_chromatix = OV5670_30010A3_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix = OV5670_30010A3_LOAD_CHROMATIX(default_video), /* RES1 */
    .liveshot_chromatix =  OV5670_30010A3_LOAD_CHROMATIX(liveshot), /* RES1 */
  },
#endif
#if VIDEO_1080P_PARAM
  {
    .common_chromatix = OV5670_30010A3_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5670_30010A3_LOAD_CHROMATIX(video_hd), /* RES2 */
    .camera_snapshot_chromatix = OV5670_30010A3_LOAD_CHROMATIX(video_hd), /* RES2 */
    .camcorder_chromatix = OV5670_30010A3_LOAD_CHROMATIX(video_hd), /* RES2 */
    .liveshot_chromatix =  OV5670_30010A3_LOAD_CHROMATIX(liveshot), /* RES2 */
  },
#endif
  {
    .common_chromatix = OV5670_30010A3_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5670_30010A3_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .camera_snapshot_chromatix = OV5670_30010A3_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .camcorder_chromatix = OV5670_30010A3_LOAD_CHROMATIX(hfr_60fps), /* RES3 */
    .liveshot_chromatix =  OV5670_30010A3_LOAD_CHROMATIX(liveshot), /* RES3 */
  },
  {
    .common_chromatix = OV5670_30010A3_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5670_30010A3_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .camera_snapshot_chromatix = OV5670_30010A3_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .camcorder_chromatix = OV5670_30010A3_LOAD_CHROMATIX(hfr_90fps), /* RES4 */
    .liveshot_chromatix =  OV5670_30010A3_LOAD_CHROMATIX(liveshot), /* RES4 */
  },
};

static struct sensor_lib_chromatix_array ov5670_30010a3_lib_chromatix_array = {
  .sensor_lib_chromatix = ov5670_30010a3_chromatix,
  .size = ARRAY_SIZE(ov5670_30010a3_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov5670_30010a3_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov5670_30010a3_real_to_register_gain(float gain)
{
  uint16_t reg_gain;

  if (gain < 1.0) {
      gain = 1.0;
  } else if (gain > 15.5) {
      gain = 15.5;
  }
  gain = (gain) * 128.0;
  reg_gain = (uint16_t) gain;

  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - ov5670_30010a3_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov5670_30010a3_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  if (reg_gain < 0x80) {
      reg_gain = 0x80;
  } else if (reg_gain > 0x7C0) {
      reg_gain = 0x7C0;
  }
  real_gain = (float) reg_gain / 128.0;

  return real_gain;
}

/*===========================================================================
 * FUNCTION    - ov5670_30010a3_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov5670_30010a3_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov5670_30010a3_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = ov5670_30010a3_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov5670_30010a3_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov5670_30010a3_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
  struct msm_camera_i2c_reg_setting* reg_setting)
{
  int32_t rc = 0;
  uint16_t reg_count = 0;
  uint16_t i = 0;
  uint8_t gain_factor = 0;

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
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xffff) >> 12;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0x0fff) >> 4;
  reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 2;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0x0f) << 4;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0x1FFF) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_count++;

  if(gain >= 1024) {
    // gain >= 8x
    gain_factor = 0x07;
  } else if(gain >= 512){
    // 4x =< gain < 8x
    gain_factor = 0x03;
  } else if(gain >= 256){
    // 2x =< gain < 4x
    gain_factor = 0x01;
  } else{
    // 1x =< gain < 2x
    gain_factor = 0x00;
  }
  reg_setting->reg_setting[reg_count].reg_addr = 0x366a;
  reg_setting->reg_setting[reg_count].reg_data = gain_factor;
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

static sensor_exposure_table_t ov5670_30010a3_expsoure_tbl = {
  .sensor_calculate_exposure = ov5670_30010a3_calculate_exposure,
  .sensor_fill_exposure_array = ov5670_30010a3_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor actuator name */
  .actuator_name = "dw9714_pc0fe",
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* sensor eeprom name */
  .eeprom_name = "sunrise_pc0fe",
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
  .exposure_table_size = 11,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov5670_30010a3_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov5670_30010a3_cid_cfg),
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
  .sensor_res_cfg_table = &ov5670_30010a3_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov5670_30010a3_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov5670_30010a3_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov5670_30010a3_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
  .sync_exp_gain = 1,
};

/*===========================================================================
 * FUNCTION    - ov5670_30010a3_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov5670_30010a3_open_lib(void)
{
  return &sensor_lib_ptr;
}
