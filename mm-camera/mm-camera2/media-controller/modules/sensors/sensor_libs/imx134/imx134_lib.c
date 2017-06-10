/*============================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "imx134_lib.h"

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
  .slave_addr = 0x34,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x0016,
    /* sensor id */
    .sensor_id = 0x0134,
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
  .x_output = 0x034C,
  .y_output = 0x034E,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0205,
  .vert_offset = 4,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 8.0,
  .max_linecount = 106840,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 4.6,
  .pix_size = 1.4,
  .f_number = 2.65,
  .total_f_dist = 1.97,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xE4,
  .csi_lane_mask = 0x3,
  .csi_if = 1,
  .csid_core = { 0 },
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x1F,
  .csi_if = 1,
  .csid_core = { 0 },
  .csi_phy_sel = 0,
};
#endif

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
  { 0x0100, 0x01 },
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  { 0x0100, 0x00 },
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
  { 0x0104, 0x01 },
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  { 0x0104, 0x00 },
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg imx134_cid_cfg[] = {
  { 0, CSI_RAW10, CSI_DECODE_10BIT },
  { 1, 0x35, CSI_DECODE_8BIT },
  { 2, CSI_EMBED_DATA, CSI_DECODE_8BIT },
};

static struct msm_camera_csi2_params imx134_csi_params = {
  .csid_params = {
    .lane_cnt = 2,
    .lut_params = {
      .num_cid = ARRAY_SIZE(imx134_cid_cfg),
      .vc_cfg = {
        &imx134_cid_cfg[0],
        &imx134_cid_cfg[1],
        &imx134_cid_cfg[2],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x14,
  },
};

static struct sensor_pix_fmt_info_t imx134_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SRGGB10},
  { MSM_V4L2_PIX_FMT_META },
};

static struct sensor_pix_fmt_info_t imx134_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t imx134_stream_info[] = {
  { 2, &imx134_cid_cfg[0], imx134_pix_fmt0_fourcc },
  { 1, &imx134_cid_cfg[1], imx134_pix_fmt1_fourcc },
};

static sensor_stream_info_array_t imx134_stream_info_array = {
  .sensor_stream_info = imx134_stream_info,
  .size = ARRAY_SIZE(imx134_stream_info),
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
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct msm_camera_csi2_params* csi_params[] = {
  &imx134_csi_params, /* RES 0 */
  &imx134_csi_params, /* RES 1 */
  &imx134_csi_params, /* RES 2 */
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_crop_parms_t crop_params[] = {
  { 0, 0, 0, 0 }, /* RES 0 */
  { 0, 0, 0, 0 }, /* RES 1 */
  { 0, 0, 0, 0 }, /* RES 2 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
    /* full size @ 30.0 fps*/
    .x_output = 3280,
    .y_output = 2464,
    .line_length_pclk = 3600,
    .frame_length_lines = 2762,
    .vt_pixel_clk = 300000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 15.0,
    .min_fps = 7.5,
  },
  {
    /* preview */
    .x_output = 1640,
    .y_output = 1232,
    .line_length_pclk = 3600,
    .frame_length_lines = 2486,
    .vt_pixel_clk = 360000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 30.00,
    .min_fps = 7.5,
  },
  {
    /* VHDR */
    .x_output = 1632,
    .y_output = 1232,
    .line_length_pclk = 3600,
    .frame_length_lines = 2488,
    .vt_pixel_clk = 360000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 30.00,
    .min_fps = 7.5,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static struct sensor_meta_data_out_info_t sensor_meta_data_out_info[] = {
  {
    /* meta data info */
    .width  = 1632,
    .height = 2,
  },
};

static struct sensor_lib_meta_data_info_array meta_data_out_info_array = {
  .meta_data_out_info = sensor_meta_data_out_info,
  .size = ARRAY_SIZE(sensor_meta_data_out_info),
};

static sensor_res_cfg_type_t imx134_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX,  /* set chromatix prt */
  SENSOR_SEND_EVENT,      /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t imx134_res_table = {
  .res_cfg_type = imx134_res_cfg,
  .size = ARRAY_SIZE(imx134_res_cfg),
};

static struct sensor_lib_chromatix_t imx134_chromatix[] = {
  {
    .common_chromatix = IMX134_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX134_LOAD_CHROMATIX(snapshot),   /* RES0 */
    .camera_snapshot_chromatix = IMX134_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = IMX134_LOAD_CHROMATIX(default_video),  /* RES0 */
  },
  {
    .common_chromatix = IMX134_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX134_LOAD_CHROMATIX(preview),  /* RES1 */
    .camera_snapshot_chromatix = IMX134_LOAD_CHROMATIX(preview), /* RES1 */
    .camcorder_chromatix = IMX134_LOAD_CHROMATIX(default_video),       /* RES1 */
  },
  {
    .common_chromatix = IMX134_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = IMX134_LOAD_CHROMATIX(preview),  /* RES2 */
    .camera_snapshot_chromatix = IMX134_LOAD_CHROMATIX(preview), /* RES2 */
    .camcorder_chromatix = IMX134_LOAD_CHROMATIX(preview),       /* RES2 */
  },
};

static struct sensor_lib_chromatix_array imx134_lib_chromatix_array = {
  .sensor_lib_chromatix = imx134_chromatix,
  .size = ARRAY_SIZE(imx134_chromatix),
};

/*===========================================================================
 * FUNCTION    - imx134_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t imx134_real_to_register_gain(float gain) {
  uint16_t reg_gain;
  if (gain < 1.0)
    gain = 1.0;
  if (gain > 8.0)
    gain = 8.0;
  reg_gain = (uint16_t)(256.0 - 256.0 / gain);
  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - imx134_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float imx134_register_to_real_gain(uint16_t reg_gain) {
  float gain;
  if (reg_gain > 224)
    reg_gain = 224;
  gain = 256.0 / (256.0 - reg_gain);
  return gain;
}

/*===========================================================================
 * FUNCTION    - imx134_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx134_calculate_exposure(float real_gain,
                                         uint16_t line_count, sensor_exposure_info_t* exp_info) {
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = imx134_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain = imx134_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - imx134_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t imx134_fill_exposure_array(uint16_t gain, uint32_t line,
                                          uint32_t fl_lines,  int32_t luma_avg, uint32_t fgain,struct msm_camera_i2c_reg_setting* reg_setting) {
  uint16_t reg_count = 0;
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
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_count = reg_count + 1;

  /* For video HDR mode */
  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->delay = 0;
  return 0;
}

static int32_t imx134_fill_awb_hdr_array(uint16_t awb_gain_r, uint16_t awb_gain_b,
  struct msm_camera_i2c_seq_reg_setting* reg_setting) {
  return 0;
}

static sensor_exposure_table_t imx134_expsoure_tbl = {
  .sensor_calculate_exposure = imx134_calculate_exposure,
  .sensor_fill_exposure_array = imx134_fill_exposure_array,
};

static sensor_video_hdr_table_t imx134_video_hdr_tbl = {
  .sensor_fill_awb_array = imx134_fill_awb_hdr_array,
  .awb_table_size = 2,
  .video_hdr_capability = 0x100, /* (1<<8) */
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
  .sensor_num_frame_skip = 0,
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
  .csi_cid_params = imx134_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(imx134_cid_cfg),
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
  .sensor_res_cfg_table = &imx134_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &imx134_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &imx134_expsoure_tbl,
  /* video hdr func table */
  .video_hdr_awb_lsc_func_table = &imx134_video_hdr_tbl,
  /* chromatix array */
  .chromatix_array = &imx134_lib_chromatix_array,
  /* meta data info */
  .meta_data_out_info_array = &meta_data_out_info_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - imx134_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void* imx134_open_lib(void) {
  return &sensor_lib_ptr;
}
