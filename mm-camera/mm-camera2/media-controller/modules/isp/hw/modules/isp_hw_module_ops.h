/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_HW_MODULE_OPS_H__
#define __ISP_HW_MODULE_OPS_H__

#include <unistd.h>
#include <math.h>
#include <stdint.h>

#include "isp_ops.h"

typedef enum {
  ISP_MOD_LINEARIZATION   = 0,
  ISP_MOD_ROLLOFF         = 1,
  ISP_MOD_DEMUX           = 2,
  ISP_MOD_DEMOSAIC        = 3,
  ISP_MOD_BPC             = 4,
  ISP_MOD_ABF             = 5,
  ISP_MOD_ASF             = 6,
  ISP_MOD_COLOR_CONV      = 7,
  ISP_MOD_COLOR_CORRECT   = 8,
  ISP_MOD_CHROMA_SS       = 9,
  ISP_MOD_CHROMA_SUPPRESS = 10,
  ISP_MOD_LA              = 11,
  ISP_MOD_MCE             = 12,
  ISP_MOD_SCE             = 13,
  ISP_MOD_CLF             = 14,
  ISP_MOD_WB              = 15,
  ISP_MOD_GAMMA           = 16,
  ISP_MOD_FOV             = 17,
  ISP_MOD_SCALER          = 18,
  ISP_MOD_BCC             = 19,
  ISP_MOD_CLAMP           = 20,
  ISP_MOD_FRAME_SKIP      = 21,
  ISP_MOD_STATS           = 22,
  ISP_MOD_COLOR_XFORM     = 23,
  ISP_MOD_MAX_NUM         = 24
} isp_hw_module_id_t;

typedef enum {
  ISP_HW_MOD_NOTIFY_INVALID,             /* not used */
  ISP_HW_MOD_NOTIFY_FETCH_SCALER_OUTPUT, /* isp_pixel_window_info_t */
  ISP_HW_MOD_NOTIFY_FETCH_FOVCROP_OUTPUT, /*isp_pixel_line_info_t*/
  ISP_HW_MOD_NOTIFY_FETCH_SCALER_CROP_REQUEST, /*uint32_t*/
  ISP_HW_MOD_NOTIFY_MOD_ENABLE_FLAG,     /* isp_mod_enable_motify_t */
  ISP_HW_MOD_NOTIFY_BE_CONFIG,           /* notify be config data*/
  ISP_HW_MOD_NOTIFY_ROLL_CONFIG,         /* notify rolloff config data*/
  ISP_HW_MOD_NOTIFY_GET_ROLLOFF_TABLE,   /* notify get rolloff table*/
  ISP_HW_MOD_NOTIFY_BG_PCA_STATS_CONFIG,         /* notify rolloff config data*/
  ISP_HW_MOD_NOTIFY_MAX_NUM              /* max num param types */
} isp_hw_mod_notify_param_t;

typedef enum {
  ISP_HW_MOD_SET_INVALID,            /* not used enum */
  ISP_HW_MOD_SET_MOD_ENABLE,         /* enable module, isp_mod_set_enable_t */
  ISP_HW_MOD_SET_MOD_CONFIG,         /* config module, isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_CONFIG_UPDATE,      /* config module, isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_TRIGGER_ENABLE,     /* enable trigger update, isp_mod_set_enable_t */
  ISP_HW_MOD_SET_TRIGGER_UPDATE,     /* isp_pix_trigger_update_input_t */
  ISP_HW_MOD_SET_LA_HIST_UPDATE,     /* isp_pix_trigger_update_input_t */
  ISP_HW_MOD_SET_EFFECT,             /* isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_MANUAL_WB,          /* isp_pix_trigger_update_input_t */
  ISP_HW_MOD_SET_BESTSHOT,           /* isp_mod_set_enable_t, isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_SCE_FACTOR,         /* int32_t */
  ISP_HW_MOD_SET_CONTRAST,           /* isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_ZOOM_RATIO,         /* isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_CHROMATIX_RELOAD,   /* chromatix changed, isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_SHARPNESS_FACTOR,   /* int32_t */
  ISP_HW_MOD_SET_STATS_FULLSIZE_CFG, /* isp_hw_pix_setting_params_t */
  ISP_HW_MOD_SET_BRACKETING_DATA,    /* mct_bracket_ctrl_t */
  ISP_HW_MOD_SET_MAX_NUM             /* max set param num */
} isp_hw_mod_set_param_id_t;

typedef enum {
  ISP_HW_MOD_GET_INVALID,          /* not used enum */
  ISP_HW_MOD_GET_MOD_ENABLE,       /* enable module */
  ISP_HW_MOD_GET_APPLIED_DIG_GAIN, /* isp_hw_mode_get_applied_dig_gain_t */
  ISP_HW_MOD_GET_DEMOSAIC_UPDATE_FLAG, /* isp_hw_mode_get_demosaic_update_flag_t */
  ISP_PIX_GET_SCALER_OUTPUT,
  ISP_PIX_GET_UV_SUBSAMPLE_SUPPORTED, /* isp_hw_pix_uv_subsample_supported_t */
  ISP_PIX_GET_SCALER_CROP_REQUEST,   /*uint32_t*/
  ISP_HW_MOD_GET_CS_RS_CONFIG,     /* isp_cs_rs_config_t */
  ISP_HW_MOD_GET_FOV,              /* isp_hw_zoom_param_t */
  ISP_HW_MOD_GET_TBLS,             /* mct_isp_get_tbls*/
  ISP_PIX_GET_FOV_OUTPUT,
  ISP_HW_MOD_GET_ROLLOFF_GRID_INFO,
  ISP_HW_MOD_GET_ROLLOFF_TABLE,
  ISP_HW_MOD_GET_TABLE_SIZE,
  ISP_HW_MOD_GET_DMI_DUMP_USER,
  ISP_PIPELINE_GET_CDS_TRIGGER_VAL, /*isp_uv_subsample_t*/
  ISP_HW_MOD_GET_VFE_DIAG_INFO_USER, /*vfe_diagnostics_t*/
  ISP_HW_MOD_GET_TINTLESS_RO, /*Tintless Rolloff */
  ISP_HW_MOD_GET_MAX_NUM           /* max get param num */
} isp_hw_mod_get_param_id_t;

typedef enum {
  ISP_HW_MOD_ACTION_INVALID,   /* invalid action code */
  ISP_HW_MOD_ACTION_HW_UPDATE, /* do hw reg update */
  ISP_HW_MOD_ACTION_BUF_CFG,     /* stats mask */
  ISP_HW_MOD_ACTION_BUF_UNCFG,  /* stats mask */
  ISP_HW_MOD_ACTION_STREAMON,    /* stats mask */
  ISP_HW_MOD_ACTION_STREAMOFF,   /* stats mask */
  ISP_HW_MOD_ACTION_STATS_PARSE, /* stats mask */
  ISP_HW_MOD_ACTION_RESET,      /* null */
  ISP_HW_MOD_ACTION_MAX_NUM,    /* max num action codes */
}isp_hw_mod_action_code_t;

/* create isp module open function table */
isp_ops_t *isp_hw_module_open(uint32_t version, isp_hw_module_id_t module_id);

#endif /* __ISP_HW_MODULE_OPS_H__ */
