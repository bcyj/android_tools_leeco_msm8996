/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __VFE_INTERFACE_H__
#define __VFE_INTERFACE_H__

#include "tgtcommon.h"
#include "intf_comm_data.h"
#include "chromatix.h"

/*===========================================================================
 *  VFE Interface info
 *==========================================================================*/
typedef enum {
  VFE_SUCCESS = 0,
  VFE_ERROR_GENERAL,
  VFE_ERROR_INVALID_OPERATION,
  VFE_ERROR_NO_MEMORY,
  VFE_ERROR_NOT_SUPPORTED,
}vfe_status_t;

typedef enum {
  VFE_OP_MODE_INVALID,
  VFE_OP_MODE_PREVIEW,
  VFE_OP_MODE_SNAPSHOT,
  VFE_OP_MODE_RAW_SNAPSHOT,
  VFE_OP_MODE_VIDEO,
  VFE_OP_MODE_ZSL,
  VFE_OP_MODE_JPEG_SNAPSHOT,
  VFE_OP_MODE_MAX,
}vfe_op_mode_t;

typedef enum {
  VFE_CONFIG_MODE,
  VFE_TRIGGER_UPDATE_FOR_ASD,
  VFE_SOF_NOTIFY,
  VFE_CONFIG_AF,
  VFE_STOP_AF,
  VFE_TRIGGER_UPDATE_FOR_3A,
  VFE_MODULE_INIT,
  VFE_CMD_OPS,
  VFE_RELEASE_STATS,
  VFE_PLUGIN_MOD_UPDATE,
  VFE_PLUGIN_REG_UPDATE,
}vfe_process_t;

typedef enum {
  VFE_GET_FOV_CROP_PARM,
  VFE_GET_CAMIF_PARM,
  VFE_GET_AEC_SHIFT_BITS,
  VFE_GET_AWB_SHIFT_BITS,
  VFE_GET_RS_CS_PARM,
  VFE_GET_IHIST_SHIFT_BITS,
  VFE_GET_DIAGNOSTICS_PTR,
  VFE_GET_PIXEL_CROP_INFO,
  VFE_GET_PIXEL_SKIP_INFO,
  VFE_GET_PP_INFO,
  VFE_GET_PRE_FOV_PARM,
  VFE_GET_OUTPUT_DIMENSION,
  VFE_GET_BLK_INC_COMP,
  VFE_GET_GAMMA_RECONFIG_VFE,
#ifdef VFE_2X
  VFE_GET_ROLLOFF_RECONFIG_VFE,
  VFE_GET_BLACK_LEVEL_RECONFIG_VFE,
  VFE_GET_5X5ASF_RECONFIG_VFE,
#endif
  VFE_GET_ZOOM_CROP_INFO,
  VFE_GET_STATS_CONF_ENABLE,
  VFE_GET_RESIDUE_DIG_GAIN,
  VFE_GET_STATS_BUF_PTR,
  VFE_GET_VERSION,
  VFE_GET_STATS_BUF_SIZE,
  VFE_GET_MOBICAT_ISP_INFO,
}vfe_get_parm_t;

typedef enum {
  VFE_SET_SENSOR_PARM,
  VFE_SET_CHROMATIX_PARM,
  VFE_SET_EFFECTS,
  VFE_SET_CAMERA_MODE,
  VFE_SET_BESTSHOT,
  VFE_SET_AEC_PARAMS,
  VFE_SET_AWB_PARMS,
  VFE_SET_ASD_PARMS,
  VFE_SET_SENSOR_DIG_GAIN,
  VFE_SET_FLASH_PARMS,
  VFE_SET_EZTUNE,
  VFE_SET_SHARPNESS_PARMS,
  VFE_SET_WB, /*parm1 - config3a_wb_t(is mwb enabled) parm2 - gain for mwb*/
  VFE_SET_MCE,
  VFE_SET_SCE,
  VFE_SET_CAMIF_DIM,
  VFE_SET_OUTPUT_INFO,
  VFE_SET_FOV_CROP_FACTOR,
  VFE_SET_FRAME_SKIP,
  VFE_SET_HFR_MODE,
  VFE_PARM_HW_VERSION,
  VFE_PARM_ADD_OBJ_ID,
  VFE_SET_STATS_VERSION,
  VFE_SETS_STATS_STREAMS,
  VFE_SET_MOBICAT,
}vfe_set_parm_t;

typedef struct {
  sensor_mode_t sensor_mode;
} vfe_set_data_t;

typedef struct {
  vfe_set_parm_t type;
  vfe_set_data_t data;
} vfe_set_t;

typedef struct {
  vfe_stats_aec_params aec_params;
  vfe_stats_rs_cs_params rs_cs_params;
  uint32_t ihist_shift_bits;
  uint32_t awb_shift_bits;
  float blk_inc_comp;
}vfe_output_t;

typedef struct {
  int preview_hfr;
  uint32_t hfr_mode;
}vfe_hfr_info_t;

typedef struct {
  enum msm_stats_enum_type type;
  int8_t use_hal_buff;
}vfe_set_hal_stream;


/*===========================================================================
 *  sensor Interface info
 *==========================================================================*/
/*for camif*/
typedef enum {
  CAMIF_HIGH = 0,
  CAMIF_LOW
}sensor_camif_active_t;

typedef enum {
  CAMIF_APS = 0,
  CAMIF_EFS = 1,
  CAMIF_ELS = 2
}sensor_camif_syncmode_t;

typedef enum {
  VFE_FLASH_NONE,
  VFE_FLASH_LED,
  VFE_FLASH_STROBE,
}vfe_flash_type;

typedef struct {
  /* current sensor ouput resolution */
  uint16_t sensor_width;
  uint16_t sensor_height;

  /* Full size input to the VFE. */
  uint16_t full_size_width;
  uint16_t full_size_height;

  /* Qtr Size input to the VFE */
  uint16_t qtr_size_width;
  uint16_t qtr_size_height;

  uint32_t lastLine;
  uint32_t firstLine;

  uint32_t lastPixel;
  uint32_t firstPixel;

  sensor_output_format_t vfe_snsr_fmt;
  sensor_camif_inputformat_t vfe_camif_in_fmt;
  sensor_raw_output_t  vfe_raw_depth;
  sensor_connection_mode_t vfe_snsr_conn_mode;
}vfe_sensor_params_t;

/*===========================================================================
 *  asd Interface info
 *==========================================================================*/
typedef struct {
  uint32_t landscape_severity;
  uint32_t backlight_detected;
  uint32_t backlight_scene_severity;
  uint32_t portrait_severity;
  float asd_soft_focus_dgr;
}vfe_asd_params_t;

/*===========================================================================
 *  aec Interface info
 *==========================================================================*/
typedef struct {
  uint32_t target_luma;
  uint32_t cur_luma;
  int32_t exp_index;
  int32_t exp_tbl_val;
  float lux_idx;
  float cur_real_gain;
  float snapshot_real_gain;
}vfe_aec_parms_t;

/*===========================================================================
 *  af Interface info
 *==========================================================================*/
typedef struct {
  uint32_t rgn_hoffset;
  uint32_t rgn_voffset;
  uint32_t rgn_width;
  uint32_t rgn_height;
  uint32_t shift_bits;
  uint32_t rgn_hnum;
  uint32_t rgn_vnum;
  uint32_t win_mode;
  uint8_t *multi_roi_win;
}vfe_af_params_t;

/*===========================================================================
 *  flash Interface info
 *==========================================================================*/
typedef struct {
  camera_flash_type flash_mode;
  uint32_t sensitivity_led_off;
  uint32_t sensitivity_led_low;
  uint32_t sensitivity_led_hi;
  uint32_t strobe_duration;
}vfe_flash_parms_t;

/*===========================================================================
 *  awb Interface info
 *==========================================================================*/

typedef struct {
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t6;
  uint32_t t4;
  uint32_t mg;
  uint32_t t5;
}vfe_awb_exterme_col_param_t;

typedef struct {
  uint32_t regionW;
  uint32_t regionH;
  uint32_t regionHNum;
  uint32_t regionVNum;
  uint32_t regionHOffset;
  uint32_t regionVOffset;
  uint32_t shiftBits;
}vfe_awb_region_info_t;

typedef struct {
  chromatix_manual_white_balance_type gain;
  uint32_t color_temp;
  chromatix_wb_exp_stats_type bounding_box;
  vfe_awb_region_info_t region_info;
  vfe_awb_exterme_col_param_t exterme_col_param;
}vfe_awb_params_t;

/*===========================================================================
 *  Effects info
 *==========================================================================*/
typedef int vfe_spl_effects_type;

typedef enum {
  VFE_CONTRAST,
  VFE_HUE,
  VFE_SATURATION,
  VFE_SPL_EFFECT,
}vfe_effects_type_t;

typedef struct {
  /* % value */
  float contrast;
  float hue;
  float saturation;
  vfe_spl_effects_type spl_effect;
}vfe_effects_params_t;

/*===========================================================================
 *  fov crop info
 *==========================================================================*/
typedef struct {
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
}vfe_fov_crop_params_t;

typedef struct {
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
}vfe_active_crop_params_t;

/*===========================================================================
 *  zoom info
 *==========================================================================*/
typedef struct {
  uint32_t crop_first_pixel;
  uint32_t crop_last_pixel;
  uint32_t crop_first_line;
  uint32_t crop_last_line;
  uint32_t crop_out_x;
  uint32_t crop_out_y;
  uint32_t crop_factor;
}vfe_zoom_info_t;

/*===========================================================================
 *  eztune info
 *==========================================================================*/
typedef struct {
  void *diag_ptr;
}vfe_eztune_info_t;

/*===========================================================================
 *  sharpness info
 *==========================================================================*/
typedef struct {
  float asd_soft_focus_dgr;
  float bst_soft_focus_dgr;
  float ui_sharp_ctrl_factor;
  float downscale_factor;
  float portrait_severity;
}vfe_sharpness_info_t;

/*===========================================================================
 *  postproc params info
 *==========================================================================*/
typedef struct {
  int16_t *gamma_table;
  uint32_t gamma_num_entries;
  int8_t la_enable;
  int16_t *luma_table;
  uint32_t luma_num_entries;
}vfe_pp_params_t;

/*===========================================================================
 *  stats info
 *==========================================================================*/
#define VFE_3A_MASK 0xFF

typedef enum {
  VFE_TRIGGER_UPDATE_AEC   = (1<<0),
  VFE_TRIGGER_UPDATE_AWB   = (1<<1),
}vfe_3A_mask;

typedef struct {
  int aec_enabled;
  int awb_enabled;
  int af_enabled;
  int rs_cs_enabled;
  int hist_enabled;
}vfe_stats_conf_enable_t;

typedef struct {
  uint32_t mask;
  uint32_t *stats_buf;
}vfe_3a_parms_udpate_t;
/*===========================================================================
 *  VFE Interface APIs
 *==========================================================================*/

int VFE_comp_create();
uint32_t VFE_client_open(module_ops_t *ops, int sdev_number);
int VFE_comp_destroy();

#endif /* __VFE_INTERFACE_H__ */
