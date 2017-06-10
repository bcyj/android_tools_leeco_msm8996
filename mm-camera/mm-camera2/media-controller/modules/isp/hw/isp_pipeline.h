/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_PIPELINE_H__
#define __ISP_PIPELINE_H__

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "camera_dbg.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_hw.h"
#include "isp.h"

#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))
#define Q_TO_FLOAT(exp, i) \
  ((float)i/( 1 << (exp)))
/* Return v1 * ratio + v2 * ( 1.0 - ratio ) */
#define LINEAR_INTERPOLATION(v1, v2, ratio) \
  ((v2) + ((ratio) * ((v1) - (v2))))

#define LINEAR_INTERPOLATION_INT(v1, v2, ratio) \
  (roundf((v2) + ((ratio) * ((v1) - (v2)))))

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)
#define CEILING4(X)  (((X) + 0x0003) & 0xFFFC)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)

#define FLOOR32(X) ((X) & (~0x1F))
#define FLOOR16(X) ((X) & 0xFFF0)
#define FLOOR8(X)  ((X) & 0xFFF8)
#define FLOOR4(X)  ((X) & 0xFFFC)
#define FLOOR2(X)  ((X) & 0xFFFE)
#define ODD(X) ((X) & 0x0001) ? (X) : ((X) - 0x0001)
#define EVEN(X) ((X) & 0x0001) ? ((X) - 0x0001) : (X)

#define CameraExp(x) (exp(x))
#define CameraSquareRoot(x) (sqrt(x))

#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))

#define MIRED(x) (1000000.0f / (x))

#define F_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

#define GET_INTERPOLATION_RATIO(ct, s, e) (1.0 - ((ct) - (s))/((e) - (s)))

#define MATCH(v1, v2, th) ((abs(v2-v1) <= (th)))

#define CALC_CCT_TRIGGER_MIRED(out, in) ({ \
  out.mired_start = MIRED(in.CCT_start); \
  out.mired_end = MIRED(in.CCT_end); \
})

#define TBL_INTERPOLATE(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION(in1[i], in2[i], ratio); })

#define TBL_INTERPOLATE_INT(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION_INT(in1[i], in2[i], ratio); })

#define CEIL_LOG2(n) ({ \
  int32_t val = 0, n1 = n; \
  if (n <= 1) \
    val = 0; \
  else { \
    while (n1 > 1) { \
      val++; \
      n1 >>= 1; \
    }\
  } \
  val;})

#define MATRIX_INVERSE_3x3(MatIn, MatOut) ({\
  typeof (MatOut[0]) __det; \
  if (MatIn == NULL || MatOut == NULL) \
    return FALSE; \
  __det = MatIn[0]*(MatIn[4]*MatIn[8]-MatIn[5]*MatIn[7]) + \
          MatIn[1]*(MatIn[5]*MatIn[6]-MatIn[3]*MatIn[8]) + \
          MatIn[2]*(MatIn[3]*MatIn[7]-MatIn[4]*MatIn[6]); \
  if (__det == 0) \
    return FALSE; \
  MatOut[0] = (MatIn[4]*MatIn[8] - MatIn[5]*MatIn[7]) / __det; \
  MatOut[1] = (MatIn[2]*MatIn[7] - MatIn[1]*MatIn[8]) / __det; \
  MatOut[2] = (MatIn[1]*MatIn[5] - MatIn[2]*MatIn[4]) / __det; \
  MatOut[3] = (MatIn[5]*MatIn[6] - MatIn[3]*MatIn[8]) / __det; \
  MatOut[4] = (MatIn[0]*MatIn[8] - MatIn[2]*MatIn[6]) / __det; \
  MatOut[5] = (MatIn[2]*MatIn[3] - MatIn[0]*MatIn[5]) / __det; \
  MatOut[6] = (MatIn[3]*MatIn[7] - MatIn[4]*MatIn[6]) / __det; \
  MatOut[7] = (MatIn[1]*MatIn[6] - MatIn[0]*MatIn[7]) / __det; \
  MatOut[8] = (MatIn[0]*MatIn[4] - MatIn[1]*MatIn[3]) / __det; \
})


#define COPY_MATRIX(IN, OUT, M, N) ({ \
  int i, j; \
  for (i=0; i<M; i++) \
    for (j=0; j<N; j++) \
      OUT[i][j] = IN[i][j]; })

/* IN1 MxN  IN2 NxL*/
#define MATRIX_MULT(IN1, IN2, OUT, M, N, L) ({ \
  int i, j, k; \
  for (i=0; i<M; i++) \
    for (j=0; j<L; j++) { \
      OUT[i][j] = 0; \
      for (k=0; k<N; k++) \
        OUT[i][j] += (IN1[i][k] * IN2[k][j]); \
    } })

#define IS_UNITY_MATRIX(IN, N) ({\
  int i, j, ret = TRUE; \
  for (i=0; i<N; i++) \
     for (j=0; j<N; j++) \
       if (((i == j) && (IN[i][j] != 1)) \
         || ((i != j) && (IN[i][j] != 0))) { \
         ret = FALSE; \
         break; \
       } \
   ret;})

#define SET_UNITY_MATRIX(IN, N) ({\
  int i, j;\
  for (i=0; i<N; i++) \
     for (j=0; j<N; j++) { \
       if (i == j) \
         IN[i][j] = 1; \
       else \
         IN[i][j] = 0; \
     }})

#define IS_MANUAL_WB(parms) (parms->cfg.wb_mode != CAM_WB_MODE_AUTO) //TODO: see below
#define DEGREE_TO_RADIAN(d) \
  (0.0174532925 * d)

/* #define IS_MANUAL_WB(parms) (parms->wb_mode != CAMERA_WB_TYPE_AUTO) */

#define CUBIC_F(fs, fc0, fc1, fc2, fc3) ({ \
  double fs3, fs2;\
  fs2 = fs * fs; \
  fs3 = fs * fs2; \
  fc0 = 0.5 * (-fs3 + (2.0 * fs2) - fs); \
  fc1 = 0.5 * ((3.0 * fs3) - (5.0 * fs2) + 2.0); \
  fc2 = 0.5 * ((-3.0 * fs3) + (4.0 * fs2) + fs); \
  fc3 = 0.5 * (fs3 - fs2); \
})

typedef enum {
  ISP_PIX_STATE_INVALID,   /* initial state */
  ISP_PIX_STATE_INIT,      /* initialized */
  ISP_PIX_STATE_CFG,       /* configgured */
  ISP_PIX_STATE_ACTIVE,    /* PIX running - active */
  ISP_PIX_STATE_MAX        /* max state num */
} isp_pix_state_t;

typedef enum awb_cct_type {
  AWB_CCT_TYPE_D65,
  AWB_CCT_TYPE_D65_TL84,
  AWB_CCT_TYPE_TL84,
  AWB_CCT_TYPE_TL84_A,
  AWB_CCT_TYPE_A,
  AWB_CCT_TYPE_MAX,
}awb_cct_type;

typedef struct{
  float mired_start;
  float mired_end;
}cct_trigger_type;

typedef struct {
  cct_trigger_type trigger_A;
  cct_trigger_type trigger_d65;
  float mired_color_temp;
}cct_trigger_info;

typedef enum {
  TRIGGER_LOWLIGHT,
  TRIGGER_NORMAL,
  TRIGGER_OUTDOOR,
} trigger_lighting_t;

typedef struct {
  float ratio;
  trigger_lighting_t lighting;
} trigger_ratio_t;

typedef enum color_xform_type {
  XFORM_601_601,      /*None. ITU601(0-255)*/
  XFORM_601_601_SDTV, /*ITU601 (16-235)*/
  XFORM_601_709,      /*ITU709 */
  XFORM_MAX,          /*No color transform*/
}color_xform_type_t;

/*===========================================================================
 *  rolloff Interface info
 *==========================================================================*/
typedef enum
{
  ISP_ROLLOFF_TL84_LIGHT,   /* Flourescent  */
  ISP_ROLLOFF_A_LIGHT,      /* Incandescent */
  ISP_ROLLOFF_D65_LIGHT,    /* Day Light    */
  ISP_ROLLOFF_TL84_LOW_LIGHT,    /* Flourescent - Low Light */
  ISP_ROLLOFF_A_LOW_LIGHT,    /* Incandescent - Low Light */
  ISP_ROLLOFF_D65_LOW_LIGHT,    /* Day Light - Low Light */
  ISP_ROLLOFF_LED_FLASH,    /* Led Flash    */
  ISP_ROLLOFF_STROBE_FLASH, /* Stribe Flash */
  ISP_ROLLOFF_MAX_LIGHT,
  ISP_ROLLOFF_INVALID_LIGHT = ISP_ROLLOFF_MAX_LIGHT
}rolloff_light_type;

typedef enum {
  ISP_ROLLOFF_LENS_POSITION_INF = 0,
  ISP_ROLLOFF_LENS_POSITION_MACRO,
  ISP_ROLLOFF_LENS_POSITION_MAX
}rolloff_lens_position;

typedef enum {
  ISP_ROLLOFF_CH_R,
  ISP_ROLLOFF_CH_GR,
  ISP_ROLLOFF_CH_GB,
  ISP_ROLLOFF_CH_B,
  ISP_ROLLOFF_CH_MAX
}rolloff_color_channels;

typedef struct {
  isp_zoom_info_t y;
  isp_zoom_info_t cbcr;
} isp_crop_info_t;

typedef struct {
  int x;                  /* top left coordinate */
  int y;                  /* top left coordinate */
  int width;              /* window width */
  int height;             /* window height */
  uint32_t first_pixel;   /* first pixel */
  uint32_t last_pixel;    /* last pixel */
  uint32_t first_line;    /* first line */
  uint32_t last_line;     /* last line */
} isp_win_t;

typedef struct {
  uint8_t enable;
  uint32_t sub_enb_mask;
  uint8_t fast_aec_mode;
} isp_mod_set_enable_t;

typedef struct {
  uint8_t enable;
  uint32_t sub_enb_mask;
} isp_mod_get_enable_t;

typedef struct {
  int mod_id;
  uint8_t enable;
  uint32_t sub_enb_mask;
} isp_mod_enable_motify_t;

typedef struct {
  float applied_dig_gain;
} isp_hw_mode_get_applied_dig_gain_t;

typedef struct {
  uint8_t demosaic_update_flag;
} isp_hw_mode_get_demosaic_update_flag_t;

typedef struct {
  int fd;                  /* ISP subdef fd */
  uint32_t max_stats_mask; /* supported stats mask */
  uint32_t isp_version;
  void *buf_mgr;
  int dev_idx;
  uint32_t max_scaler_out_width;
  uint32_t max_scaler_out_height;
} isp_hw_mod_init_params_t;

typedef struct isp_hw_pix_dep isp_hw_pix_dep_t;
struct isp_hw_pix_dep {
  uint32_t max_mod_mask_continuous_bayer;
  uint32_t max_mod_mask_burst_bayer;
  uint32_t max_supported_stats;
  uint32_t max_mod_mask_continuous_yuv;
  uint32_t max_mod_mask_burst_yuv;
  uint32_t num_mod_cfg_order_bayer;
  uint16_t *mod_cfg_order_bayer;
  uint32_t num_mod_cfg_order_yuv;
  uint16_t *mod_cfg_order_yuv;
  uint32_t num_mod_trigger_update_order_bayer;
  uint16_t *mod_trigger_update_order_bayer;
  void *op_ptr;
  void (*destroy)(isp_hw_pix_dep_t *dep);
  int (*operation_config) (void *pix_ptr, int is_bayer_input);
  int (*module_start)(void *pix_ptr);
  int (*module_reconf_module)(void *pix_ptr);
  int (*module_enable_notify)(void *pix_ptr,
    isp_mod_enable_motify_t *notify_data);
  int (*do_zoom)(void *pix_ptr, isp_hw_set_crop_factor_t *crop_factor);
  int (*read_dmi_tbl)(void *pix, isp_hw_read_info *dmi_read_info,
    void *dump_entry);
  int (*util_set_uv_subsample)(void *pix_ptr, boolean enable);
  int(*get_roi_map)(void *pix_ptr, isp_hw_zoom_param_entry_t* zoom_entry);
  float (*util_get_aec_ratio_lowlight)(unsigned char tunning,
    void *trigger, aec_update_t* aec_out,
    int8_t is_snap_mode);
  int (*util_get_aec_ratio_bright_low)(unsigned char tuning,
   void *outdoor_trigger,
   void *lowlight_trigger,
   aec_update_t* aec_out,
   int8_t is_snap_mode,
   trigger_ratio_t *rt);
  float (*util_get_aec_ratio_bright)(unsigned char tunning,
    void *trigger, aec_update_t* aec_out,
    int8_t is_snap_mode);
  awb_cct_type (*util_get_awb_cct_type)(cct_trigger_info* trigger,
    void *chromatix_ptr);
  void (*util_get_param)(void *in_params, uint32_t in_params_size,
    uint32_t params_id, void* out_param, uint32_t out_params_size, void *ctrl);
};

typedef struct
{
  float offset;  // default=3.3, range =0.0 to 8.0
  float low_beam;  // default=0.9, range =0.0 to 1.0
  float high_beam;  // default=0.1 , range =0.0 to 1.0
  float histogram_cap;  // default=5.0, range 2.0 to 8.0
  float cap_high;   // default=2.0, range=1.0 to 4.0
  float cap_low;   // default=0.75, range=0.0 to 2.0
} la_8k_type_t;

typedef struct {
   uint32_t isp_ihist_data[256]; /* histogram data from VFE */
   la_8k_type_t   la_config;
}isp_ihist_params_t;

typedef struct {
  isp_ihist_params_t ihist_params;
  aec_update_t aec_update;
  awb_update_t awb_update;
  af_update_t  af_update;
  asd_update_t asd_update;
}isp_stats_udpate_t;

typedef struct {
  int16_t gamma_r[ISP_METADATA_GAMMATBL_SIZE];
  int16_t gamma_g[ISP_METADATA_GAMMATBL_SIZE];
  int16_t gamma_b[ISP_METADATA_GAMMATBL_SIZE];
} isp_gamma_t;

typedef struct {
  modulesChromatix_t chromatix_ptrs; /* chromatix headers */
  cam_streaming_mode_t streaming_mode; /* burst or continuous */
  isp_hwif_output_cfg_t outputs[ISP_PIX_PATH_MAX]; /* width == height == 0 means not used */
  isp_pixel_window_info_t scaler_output[ISP_PIX_PATH_MAX];
  isp_hwif_output_cfg_t raw_output;
  isp_pix_camif_cfg_t camif_cfg;
  isp_pixel_line_info_t demosaic_output;
  isp_crop_info_t crop_info;
  uint32_t crop_factor;      /* used for zoom. get from HAL */
  isp_effects_params_t effects; /* effects saved here */
  cam_scene_mode_type bestshot_mode; /* such as CAMERA_BESTSHOT_BEACH */
  sensor_rolloff_config_t sensor_rolloff_config; /* to calc rollof by sensor info*/
  isp_flash_params_t flash_params;
  int32_t contrast;
  int32_t sce_factor;
  int32_t sharpness;
  uint8_t strobe_enabled;
  cam_wb_mode_type wb_mode;
  //config3a_wb_type_t wb_mode; /* such as CAMERA_WB_AUTO, CAMERA_WB_SHADE, etc */
  uint8_t max_luma_scaling_ratio; /* max downscaling ratio, usually 15 for Y */
  isp_stats_config_t stats_cfg;
  int ion_fd;
  uint32_t stats_sub_mask;
  af_rolloff_info_t af_rolloff_info;
  isp_gamma_t gamma_rgb;
  int32_t recording_hint;
  uint32_t vhdr_enable;
  isp_tintless_data_t *tintless_data;
  boolean do_fullsize_cfg;
  isp_zoom_roi_params_t saved_zoom_roi;
  mct_bracket_ctrl_t bracketing_data;
} isp_hw_pix_setting_params_t;

typedef struct {
  isp_stats_udpate_t stats_update;
  float digital_gain;        /* received from sensor,
                              * used for DEMUX trigger update, default 1.0 */
  cam_flash_mode_t flash_mode;
  uint16_t lens_position_current_step;
  uint8_t demosaic_update_flag; /* pipeline will set this flag after wb trigger update */
  boolean is_init_setting;
} isp_hw_pix_trigger_update_params_t;

typedef struct {
  isp_hw_pix_setting_params_t cfg;
  isp_hw_pix_trigger_update_params_t trigger_input;
} isp_pix_trigger_update_input_t;

#define IS_BAYER_FORMAT(fmt) (fmt == CAMIF_BAYER_G_B || fmt == CAMIF_BAYER_B_G || fmt == CAMIF_BAYER_G_R || fmt == CAMIF_BAYER_R_G)
#define IS_BURST_STREAMING(params) \
  (((isp_hw_pix_setting_params_t *)params)->streaming_mode == CAM_STREAMING_MODE_BURST)

typedef struct {
  isp_mode_t isp_mode; /* 2D/3D */
  isp_zoom_info_t crop_info;
  uint32_t max_module_mask;  /* maximum allowed module mask */
  uint32_t cur_module_mask;  /* currently in use mask */
  uint32_t user_module_mask; /* user configured mask */
  uint32_t cur_stats_mask; /*max supported stats*/
  void *op_ptr; /* ISP_OperationConfigCmdType */
  isp_pix_trigger_update_input_t cfg_and_3a_params;
} isp_pix_params_t;

typedef struct {
  int fd;
  uint32_t isp_version;
  isp_notify_ops_t mod_notify_ops;
  isp_ops_t *mod_ops[ISP_MOD_MAX_NUM];
  isp_pix_state_t pix_state;
  isp_ops_t pix_ops;
  void *parent;
  isp_pix_params_t pix_params;
  isp_pix_params_t cur_pix_params; /* holds the pix_params for current frame */
  isp_hw_pix_dep_t dep;
  int num_active_streams;
  void *buf_mgr;
  uint8_t fast_aec_mode;
  uint32_t hfr_update_mod_mask;
  uint32_t hfr_update_batch1;
  uint32_t hfr_update_batch2;
  uint32_t stats_burst_len;
} isp_pipeline_t;

typedef enum {
  ISP_PIX_NOTIFY_INVALID,
  ISP_PIX_NOTIFY_STATS,
  ISP_PIX_NOTIFY_MAX
} isp_pix_notify_param_t;

typedef enum {
  ISP_PIX_SET_PARAM_INVALID,
  ISP_PIX_SET_ION_FD,             /* modulesChromatix_t */
  ISP_PIX_SET_CHROMATIX,          /* modulesChromatix_t */
  ISP_PIX_SET_AF_ROLLOFF_PARAMS, /* isp_rolloff_tables_inf_t */
  ISP_PIX_SET_CONTRAST,           /* unt32_t */
  ISP_PIX_SET_BESTSHOT,           /* int maps to camera_bestshot_mode_type */
  ISP_PIX_SET_EFFECT,             /* isp_effects_params_t */
  ISP_PIX_SET_SATURATION,          /* int32_t */
  ISP_PIX_SET_STATS_CFG_UPDATE,             /* isp_set_af_params_t */
  ISP_PIX_SET_STATS_ENABLE,       /* isp_stats_enable_t */
  ISP_PIX_SET_CAMIF_CFG,          /* isp_pix_camif_cfg_t */
  ISP_PIX_SET_STREAM_CFG,         /* isp_hwif_output_cfg_t */
  ISP_PIX_SET_STREAM_UNCFG,       /* isp_hw_stream_uncfg_t */
  ISP_PIX_SET_SENSOR_LENS_POSITION_TRIGGER_UPDATE, /* lens_position_update_isp_t */
  ISP_PIX_SET_AWB_TRIGGER_UPDATE,    /* stats_udpate_t */
  ISP_PIX_SET_AEC_TRIGGER_UPDATE,    /*stats_udpate_t*/
  ISP_PIX_SET_IHIST_LA_TRIGGER_UPDATE, /*q3a_ihist_stats_t*/
  ISP_PIX_SET_WB_MODE,            /* int maps to config3a_wb_type_t */
  ISP_PIX_SET_SHARPNESS,          /* isp_sharpness_info_t */
  ISP_PIX_SET_FLASH_MODE,         /* camera_flash_type */
  ISP_PIX_CAMIF_SYNC_TIMER_CFG,   /* camif_strobe_info_t */
  ISP_PIX_SET_STATS_CFG,
  ISP_PIX_SET_SAVED_PARAMS,       /* set saved params to pix */
  ISP_PIX_SET_UV_SUBSAMPLE,       /* uint32_t */
  ISP_PIX_SAVE_AEC_PARAMS,        /* stats_udpate_t */
  ISP_PIX_SAVE_ASD_PARAMS,        /* stats_udpate_t */
  ISP_PIX_SET_PARAM_CROP_FACTOR,  /* isp_hw_set_crop_factor_t */
  ISP_PIX_SET_SCE,                /* int32_t */
  ISP_PIX_SET_RECORDING_HINT,     /* int32_t */
  ISP_PIX_SET_VHDR,               /* unt32_t */
  ISP_PIX_SET_RELOAD_CHROMATIX,   /* modulesChromatix_t */
  ISP_PIX_SET_TINTLESS,           /* uint8_t */
  ISP_PIX_SET_MOD_TRIGGER,        /* int32_t */
  ISP_PIX_SET_MOD_ENABLE,         /* int32_t */
  ISP_PIX_SET_FLASH_PARAMS,       /* isp_flash_params_t */
  ISP_PIX_SET_BRACKETING_DATA,    /* mct_bracket_ctrl_t */
  ISP_PIX_SET_MAX_NUM,
} isp_pix_set_param_id_t;

typedef enum {
  ISP_PIX_GET_CAPABILITY,  /* isp_hw_cap_t */
  ISP_PIX_GET_CS_RS_CONFIG,  /* isp_cs_rs_config_t */
  ISP_PIX_GET_ROLLOFF_GRID_INFO,
  ISP_PIX_GET_FOV,         /* isp_hw_zoom_param_t */
  ISP_PIX_GET_ROI_MAP,
  ISP_PIX_GET_LA_GAMMA_TBLS, /*mct_isp_table_t*/
  ISP_PIX_GET_ROLLOFF_TABLE, /*mct_isp_table_t*/
  ISP_PIX_GET_ROLLOFF_TABLE_SIZE, /*isp_hw_read_info*/
  ISP_PIX_GET_GAMMA_TABLE_SIZE,   /*isp_hw_read_info*/
  ISP_PIX_GET_LINEARIZATION_TABLE_SIZE, /*isp_hw_read_info*/
  ISP_PIX_GET_LA_TABLE_SIZE, /*isp_hw_read_info*/
  ISP_PIX_GET_DMI_DUMP, /*void *, dump pointer*/
  ISP_PIX_GET_CDS_TRIGGER_VAL, /*mct_isp_table_t*/
  ISP_PIX_GET_VFE_DIAG_INFO, /*vfe_diagnostics_t*/
  ISP_PIX_GET_TINTLESS_RO, /* Get CURR Rolloff for tintless*/
  ISP_PIX_GET_MAX_NUM,
} isp_pix_get_param_id_t;

typedef enum {
  ISP_PIX_ACTION_CODE_INVALID,
  ISP_PIX_ACTION_CODE_STREAM_START,      /* isp_action_stream_start_stop_t */
  ISP_PIX_ACTION_CODE_STREAM_STOP,       /* isp_action_stream_start_stop_t */
  ISP_PIX_ACTION_CODE_SYNC_TIMER_START,  /* start sync timer */
  ISP_PIX_ACTION_CODE_AF_START,          /* start af */
  ISP_PIX_ACTION_CODE_AF_STOP,           /* stop af */
  ISP_PIX_ACTION_CODE_STATS_PARSE,       /* isp_pipeline_stats_parse_t */
  ISP_PIX_ACTION_CODE_HW_UPDATE,         /* do hw update*/
  ISP_PIX_ACTION_CODE_MAX_NUM,
} isp_pix_action_code_t;

typedef struct {
  uint32_t session_id;
  struct msm_isp_event_data *raw_stats_event;
  mct_event_stats_isp_t *parsed_stats_event;
} isp_pipeline_stats_parse_t;

typedef struct {
  uint32_t session_id;
  tintless_mesh_rolloff_array_t *rolloff;
}isp_pipeline_curr_rolloff_t;

int isp_pipeline_init(
   void *ctrl, /* pix ptr */
   void *in_params, /* int num of pix outputs */
   void *parent);
int isp_pipeline_destroy(void *ctrl);
int isp_pipeline_set_params(
  void *ctrl,
  uint32_t params_id,
  void *in_params, uint32_t in_params_size);
int isp_pipeline_get_params(
  void *ctrl,
  uint32_t params_id,
  void *in_params, uint32_t in_params_size,
  void *out_params, uint32_t out_params_size);
int isp_pipeline_action(
  void *ctrl, uint32_t action_code,
  void *action_data, uint32_t action_data_size);

int isp_pipeline_set_stats_fullsize(void *ctrl, boolean enable);
#endif /* __ISP_PIPELINE_H__ */
