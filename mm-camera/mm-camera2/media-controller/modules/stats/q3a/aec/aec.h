/* aec.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AEC_H__
#define __AEC_H__

#include "q3a_stats.h"
#include "mct_event_stats.h"

#define MAX_EXP_ENTRIES 5
#define MAX_EXP_CHAR 50

/******************** chromatix reserved part starts ***********************/
/******Shall be moved to chromatix and cleaned up whe new chroamtix relase*/
#define HIST_TARGET_ADJUST_ENABLE          (0) // Disabled by default.
#define OUTDOOR_MAX_TARGET_ADJUST_RATIO    (1.8)
#define OUTDOOR_MIN_TARGET_ADJUST_RATIO    (0.9)
#define INDOOR_MAX_TARGET_ADJUST_RATIO     (1.8)
#define INDOOR_MIN_TARGET_ADJUST_RATIO     (0.9)
#define LOWLIGHT_MAX_TARGET_ADJUST_RATIO   (1)
#define LOWLIGHT_MIN_TARGET_ADJUST_RATIO   (0.85)
#define TARGET_FILTER_FACTOR               (0.5)
#define HIST_SAT_PCT                       (0.02)
#define HIST_DARK_PCT                      (0.15)
#define HIST_SAT_LOW_REF                   (200)
#define HIST_SAT_HIGH_REF                  (240)
#define HIST_DARK_LOW_REF                  (3)
#define HIST_DARK_HIGH_REF                 (70)


#define AEC_MANUAL_EXPOSURE_TIME_MAX  2000000 /* in micro-second, 2s*/
#define AEC_MANUAL_EXPOSURE_TIME_MIN  200     /* in micro-second, 1/5000s */
#define AEC_MANUAL_EXPOSURE_TIME_AUTO 0

#define AEC_SUBSAMPLE     (10)
#define MIN_AEC_SUBSAMPLE (1)

#define PREVIEW_ISO_ENABLE              (0)
#define EXTREME_GREEN_COLOR_THLD_RADIUS (0.5)
#define START_EXP_INDEX                 (240)
#define USE_ROI_FOR_LED                 (1)

/* Slow conv parameters */
#define LUMA_TOLERANCE                  (2)
#define FRAME_SKIP                      (1)
#define HT_ENABLE                       (1)
#define HT_LUMA_TOLERANCE               (4)
#define HT_THRES                        (3.0f)
#define HT_MAX                          (1.0f)
#define HT_GYRO_ENABLE                  (1)
/* End - Tuning parameters */

/* Start - Advanced tuning parameters */
#define REF_FRAME_RATE                  (30.0f)
#define STEP_DARK                       (17.0f)
#define STEP_BRIGHT                     (12.0f)
#define STEP_REGULAR                    (4.0f)
#define LUMA_TOL_RATIO_DARK             (0.30f)
#define LUMA_TOL_RATIO_BRIGHT           (0.20f)
#define RAW_STEP_ADJUST_CAP             (2.0f)
#define ADJUST_SKIP_LUMA_TOLERANCE      (4)
#define DARK_REGION_NUM_THRES           (0.05f)
#define BRIGHT_REGION_NUM_THRES         (0.05f)
#define HT_LUMA_THRES_LOW               (0.5f)
#define HT_LUMA_THRES_HIGH              (0.7f)
#define HT_LUMA_VAL_LOW                 (1.0f)
#define HT_LUMA_VAL_HIGH                (0.6f)
#define HT_GYRO_THRES_LOW               (0.5f)
#define HT_GYRO_THRES_HIGH              (4.0f)
#define HT_GYRO_VAL_LOW                 (1.0f)
#define HT_GYRO_VAL_HIGH                (0.4f)


#define AEC_DARK_REGION_ENABLE          (0)
#define AEC_BRIGHTNESS_STEP_SIZE        (2)
/* prameters for low light luma target */
#define AEC_LOW_LIGHT_LUMA_TARGET_INIT      26
#define AEC_LOW_LIGHT_LUMA_START_IDX_INIT   350
#define AEC_LOW_LIGHT_LUMA_END_IDX_INIT     420

/********************* chromatix reserved part ends*************************/

typedef enum {
  AEC_STATS_YUV,
  AEC_STATS_BAYER
} aec_stats_type_t;

/* Data structure used for stats, q3a and aec: STARTS */
typedef enum {
  AEC_OPERATION_MODE_NONE,
  AEC_OPERATION_MODE_INIT,
  AEC_OPERATION_MODE_PREVIEW,
  AEC_OPERATION_MODE_SNAPSHOT,
  AEC_OPERATION_MODE_CAMCORDER,
  AEC_OPERATION_MODE_INVALID,
} aec_operation_mode_t;

typedef enum {
  AEC_FPS_MODE_AUTO,
  AEC_FPS_MODE_FIXED
} aec_fps_mode_t;

/** _aec_interested_region:
 *    @enable:                 TODO
 *    @rgn_index:              TODO
 *    @rgn_index_window_start: TODO
 *    @rgn_index_window_end:   TODO
 *    @r:                      TODO
 *    @num_regions:            TODO
 *
 * TODO description
 **/
typedef struct _aec_interested_region {
  uint8_t     enable;
  stats_roi_t r[MAX_STATS_ROI_NUM];
  uint8_t     num_regions;
} aec_interested_region_t;

/** _aec_hist:
 *    @roi_pixels: TODO
 *    @bin:        TODO
 *
 * TODO description
 **/
typedef struct _aec_hist {
  uint32_t roi_pixels;
  uint32_t *bin;
} aec_hist_t;

typedef enum {
  ROI_TYPE_GENERAL,
  ROI_TYPE_FACE,
} aec_roi_type_t;

/** _aec_proc_roi_info:
 *    @roi:        TODO
 *    @hist:       TODO
 *    @type:       TODO
 *    @num_roi:    TODO
 *    @frm_width:  TODO
 *    @frm_height: TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_roi_info {
  stats_roi_t    roi[MAX_STATS_ROI_NUM];
  aec_hist_t     hist[MAX_STATS_ROI_NUM];
  aec_roi_type_t type;
  uint32_t       num_roi;
  uint32_t       frm_width;
  uint32_t       frm_height;
} aec_proc_roi_info_t;

/** Gyro information
 *
 **/
typedef struct {
  /* Gyro data in float */
  int float_ready;
  float flt[3];
  /* Gyro data in Q16 */
  int q16_ready;
  long q16[3];
} aec_algo_gyro_info_t;

/** _aec_proc_mtr_area:
 *    @num_area: TODO
 *    @weight:   TODO
 *    @mtr_area: TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_mtr_area {
  int         num_area;
  int         weight[MAX_STATS_ROI_NUM];
  stats_roi_t mtr_area[MAX_STATS_ROI_NUM];
} aec_proc_mtr_area_t;

typedef enum {
  STROBE_FLASH_MODE_OFF,
  STROBE_FLASH_MODE_AUTO,
  STROBE_FLASH_MODE_ON,
  STROBE_FLASH_MODE_MAX,
} aec_strobe_flash_mode_t;

/** _aec_set_asd_param:
 *    @backlight_detected:                TODO
 *    @backlight_scene_severity:          TODO
 *    @backlight_luma_target_offset:      TODO
 *    @snow_or_cloudy_scene_detected:     TODO
 *    @snow_or_cloudy_luma_target_offset: TODO
 *    @landscape_severity:                TODO
 *    @portrait_severity:                 TODO
 *    @soft_focus_dgr:                    TODO
 *
 * TODO description
 **/
typedef struct _aec_set_asd_param {
  uint32_t backlight_detected;
  uint32_t backlight_scene_severity;
  uint32_t backlight_luma_target_offset;
  uint32_t snow_or_cloudy_scene_detected;
  uint32_t snow_or_cloudy_luma_target_offset;
  uint32_t landscape_severity;
  uint32_t portrait_severity;
  float    soft_focus_dgr;
  boolean  enable;
} aec_set_asd_param_t;

typedef enum {
  STATS_PROC_ANTIBANDING_OFF,
  STATS_PROC_ANTIBANDING_60HZ,
  STATS_PROC_ANTIBANDING_50HZ,
  STATS_PROC_ANTIBANDING_AUTO,
  STATS_PROC_MAX_ANTIBANDING,
} aec_antibanding_type_t;

typedef enum {
  AFD_OFF = 0,
  AFD_REGULAR_EXPOSURE_TABLE,
  AFD_60HZ_EXPOSURE_TABLE,
  AFD_50HZ_EXPOSURE_TABLE,
  AFD_50HZ_AUTO_EXPOSURE_TABLE,
  AFD_60HZ_AUTO_EXPOSURE_TABLE,
} aec_afd_status_t;

/** _aec_set_afd_parm:
 *    @afd_enable:    TODO
 *    @afd_monitor:   TODO
 *    @afd_exec_once: TODO
 *    @afd_status:    TODO
 *    @afd_atb:       TODO
 *
 * TODO description
 **/
typedef struct _aec_set_afd_parm {
  boolean                afd_enable;
  int                    afd_monitor;
  int                    afd_exec_once;
  aec_afd_status_t       afd_status;
  aec_antibanding_type_t afd_atb;
} aec_set_afd_parm_t;

/** _aec_set_awb_parm:
 *    @r_gain:    TODO
 *    @g_gain:    TODO
 *    @b_gain:    TODO
 *    @colortemp: TODO
 *
 * TODO description
 **/
typedef struct _aec_set_awb_parm {
  float r_gain;
  float g_gain;
  float b_gain;
  int   colortemp;
} aec_set_awb_parm_t;

typedef enum {
  AEC_ISO_AUTO = 0,
  AEC_ISO_DEBLUR,
  AEC_ISO_100,
  AEC_ISO_200,
  AEC_ISO_400,
  AEC_ISO_800,
  AEC_ISO_1600,
  AEC_ISO_3200,
  AEC_ISO_MAX
} aec_iso_mode_t;

typedef enum {
  AEC_BESTSHOT_OFF = 0,
  AEC_BESTSHOT_AUTO = 1,
  AEC_BESTSHOT_LANDSCAPE = 2,
  AEC_BESTSHOT_SNOW,
  AEC_BESTSHOT_BEACH,
  AEC_BESTSHOT_SUNSET,
  AEC_BESTSHOT_NIGHT,
  AEC_BESTSHOT_PORTRAIT,
  AEC_BESTSHOT_BACKLIGHT,
  AEC_BESTSHOT_SPORTS,
  AEC_BESTSHOT_ANTISHAKE, /* 10 */
  AEC_BESTSHOT_FLOWERS,
  AEC_BESTSHOT_CANDLELIGHT,
  AEC_BESTSHOT_FIREWORKS,
  AEC_BESTSHOT_PARTY,
  AEC_BESTSHOT_NIGHT_PORTRAIT,
  AEC_BESTSHOT_THEATRE,
  AEC_BESTSHOT_ACTION,
  AEC_BESTSHOT_AR,
  AEC_BESTSHOT_MAX
} aec_bestshot_mode_type_t;

typedef enum {
  AEC_FLASH_MODE_NONE,
  AEC_FLASH_MODE_LED,
  AEC_FLASH_MODE_STROBE
} aec_flash_mode_t;

typedef enum {
  AEC_METERING_FRAME_AVERAGE,
  AEC_METERING_CENTER_WEIGHTED,
  AEC_METERING_SPOT_METERING,
  AEC_METERING_SMART_METERING,
  AEC_METERING_USER_METERING,
  AEC_METERING_SPOT_METERING_ADV,
  AEC_METERING_CENTER_WEIGHTED_ADV,
  AEC_METERING_MAX_MODES
} aec_auto_exposure_mode_t;

/** _aec_parms:
 *    @target_luma:        TODO
 *    @cur_luma:           TODO
 *    @exp_index:          TODO
 *    @exp_tbl_val:        TODO
 *    @lux_idx:            TODO
 *    @cur_real_gain:      TODO
 *    @snapshot_real_gain: TODO
 *
 * TODO description
 **/
typedef struct _aec_parms {
  uint32_t target_luma;
  uint32_t cur_luma;
  int32_t  exp_index;
  int32_t  exp_tbl_val;
  float    lux_idx;
  float    cur_real_gain;
  float    snapshot_real_gain;
} aec_parms_t;

/** _aec_exp_parms:
 *    @luma_target:        TODO
 *    @current_luma:       TODO
 *    @gain:               TODO
 *    @linecount:          TODO
 *    @led_off_gain:       TODO
 *    @led_off_linecount:  TODO
 *    @valid_exp_entries:  TODO
 *    @use_led_estimation: TODO
 *    @lux_idx:            TODO
 *    @exp_time:           TODO
 *
 * TODO description
 **/
typedef struct _aec_exp_parms {
  int      luma_target;
  int      current_luma;
  float    gain[MAX_EXP_ENTRIES];
  uint32_t linecount[MAX_EXP_ENTRIES];
  float    led_off_gain;
  uint32_t led_off_linecount;
  int      valid_exp_entries;
  int      use_led_estimation;
  int      exposure_index;
  float    lux_idx;
  float    exp_time;
  uint32_t iso;
  aec_auto_exposure_mode_t metering_type;
} aec_exp_parms_t;

/** _aec_flash_parms:
 *    @flash_mode:          TODO
 *    @sensitivity_led_off: TODO
 *    @sensitivity_led_low: TODO
 *    @sensitivity_led_hi:  TODO
 *    @strobe_duration:     TODO
 *
 * TODO description
 **/
typedef struct _aec_flash_parms {
  aec_flash_mode_t flash_mode;
  uint32_t sensitivity_led_off;
  uint32_t sensitivity_led_low;
  uint32_t sensitivity_led_hi;
  uint32_t strobe_duration;
} aec_flash_parms_t;

typedef enum {
  AEC_GET_PARAM_LED_SETTLE_CNT,
  AEC_GET_PARAM_OVER_EXP_STATE,
  AEC_GET_PARAM_LED_STROBE,
  AEC_GET_PARAM_FLASH_FOR_SNAPSHOT,
  AEC_GET_PARAM_FPS_MODE_G,
  AEC_GET_PARAM_AFR_PREVIEW_FPS,
  AEC_GET_PARAM_PREVIEW_EXP_TIME,
  AEC_GET_PARAM_LOCK_STATE,
  AEC_GET_PARAM_FORCED_EXPOSURE,
  AEC_GET_PARAM_PARMS,
  AEC_GET_PARAM_EXPOSURE_PARAMS,
  AEC_GET_PARAM_FLASH_DATA
} aec_get_parameter_type;

/** _aec_get_parameter:
 *    @type:                 TODO
 *    @aec_over_exposure:    TODO
 *    @aec_led_settle_cnt:   TODO
 *    @use_strobe:           TODO
 *    @afr_preview_fps:      TODO
 *    @aec_preview_expotime: TODO
 *    @query_flash_for_snap: TODO
 *    @fps_mode:             TODO
 *    @aec_param:            TODO
 *    @exp_params:           TODO
 *    @flash_param:          TODO
 *
 * TODO description
 **/
typedef struct _aec_get_parameter {
  aec_get_parameter_type type;

  union {
    boolean           aec_over_exposure;
    int               aec_led_settle_cnt;
    boolean           use_strobe;
    boolean           flash_needed;
    float             afr_preview_fps;
    float             aec_preview_expotime;
    aec_flash_mode_t  query_flash_for_snap;
    aec_fps_mode_t    fps_mode;
    aec_parms_t       aec_param;
    aec_exp_parms_t   exp_params;
    aec_flash_parms_t flash_param;
  } u;
} aec_get_parameter_t;

/** _aec_proc_flash_sensitivity:
 *    @off:  TODO
 *    @low:  TODO
 *    @high: TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_flash_sensitivity {
  float off;
  float low;
  float high;
} aec_proc_flash_sensitivity_t;

/** _aec_update_af:
 *    @luma_settled_cnt: TODO
 *    @cur_af_luma:      TODO
 *
 * TODO description
 **/
typedef struct _aec_update_af {
  unsigned int luma_settled_cnt;
  unsigned int cur_af_luma;
} aec_update_af_t;

/** _aec_update_awb:
 *    @prev_exp_index: TODO
 *
 * TODO description
 **/
typedef struct _aec_update_awb {
  int prev_exp_index;
} aec_update_awb_t;

typedef enum {
  AEC_UPDATE,
  AEC_SEND_EVENT,
} aec_output_type_t;

/** _aec_update_asd:
 *    @roi_info: TODO
 *    @SY:       TODO
 *
 * TODO description
 **/
typedef struct _aec_update_asd {
  aec_proc_roi_info_t roi_info;
  uint32_t            *SY;
} aec_update_asd_t;

/** _aec_proc_snapshot:
 *    @luma_target:             TODO
 *    @line_count:              TODO
 *    @redeye_led_on_skip_frm:  TODO
 *    @redeye_led_off_skip_frm: TODO
 *    @auto_mode:               TODO
 *    @exp_mode:                TODO
 *    @exp_program:             TODO
 *    @real_gain:               TODO
 *    @exp_time:                TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_snapshot {
  uint32_t luma_target;
  uint32_t line_count;
  uint32_t redeye_led_on_skip_frm;
  uint32_t redeye_led_off_skip_frm;
  boolean  auto_mode;
  uint32_t exp_mode;
  uint32_t exp_program;
  float    real_gain;
  float    exp_time;
} aec_proc_snapshot_t;

typedef enum {
  AEC_SET_PARAM_INIT_CHROMATIX_SENSOR   = 1,
  AEC_SET_PARAM_EXP_COMPENSATION        ,
  AEC_SET_PARAM_BRIGHTNESS_LVL          ,
  AEC_SET_PARAM_HJR_AF                  ,
  AEC_SET_PARAM_HJR                     ,
  AEC_SET_PARAM_REDEYE_REDUCTION_MODE   ,
  AEC_SET_PARAM_METERING_MODE           ,
  AEC_SET_PARAM_ISO_MODE                ,
  AEC_SET_PARAM_MOTION_ISO              ,
  AEC_SET_PARAM_ANTIBANDING             , /* 10 */
  AEC_SET_PARAM_ANTIBANDING_STATUS      ,
  AEC_SET_PARAM_FPS_MODE                ,
  AEC_SET_PARAM_LED_RESET               ,
  AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT    ,
  AEC_SET_PARAM_STROBE_MODE             ,
  AEC_SET_PARAM_FPS                     ,
  AEC_SET_PARAM_ROI                     ,
  AEC_SET_PARAM_MTR_AREA                ,
  AEC_SET_PARAM_SOF                     ,
  AEC_SET_PARAM_BESTSHOT                , /* 20 */
  AEC_SET_PARAM_FD_ROI                  ,
  AEC_SET_PARAM_STROBE_CFG_ST           ,
  AEC_SET_PARAM_EZ_DISABLE              ,
  AEC_SET_PARAM_EZ_LOCK_OUTPUT          ,
  AEC_SET_PARAM_EZ_FORCE_EXP            ,
  AEC_SET_PARAM_EZ_FORCE_LINECOUNT      ,
  AEC_SET_PARAM_EZ_FORCE_GAIN           ,
  AEC_SET_PARAM_EZ_TEST_ENABLE          ,
  AEC_SET_PARAM_EZ_TEST_ROI             ,
  AEC_SET_PARAM_EZ_TEST_MOTION          , /* 30 */
  AEC_SET_PARAM_EZ_FORCE_SNAP_EXP       ,
  AEC_SET_PARAM_EZ_FORCE_SNAP_LINECOUNT ,
  AEC_SET_PARAM_EZ_FORCE_SNAP_GAIN      ,
  AEC_SET_PARAM_EZ_TUNE_RUNNING         ,
  AEC_SET_PARAM_LOCK                    ,
  AEC_SET_PARAM_SENSITIVITY_RATIO       ,
  AEC_SET_PARAM_LED_EST                 ,
  AEC_SET_PARAM_ASD_PARM                ,
  AEC_SET_PARAM_AFD_PARM                ,
  AEC_SET_PARAM_INIT_SENSOR_INFO        , /* 40 */
  AEC_SET_PARAM_ENABLE                  ,
  AEC_SET_PARAM_AWB_PARM                ,
  AEC_SET_PARAM_GYRO_INFO               ,
  AEC_SET_PARAM_LED_MODE                ,
  AEC_SET_PARAM_BRACKET                 ,
  AEC_SET_PARAM_FLASH_BRACKET           ,
  AEC_SET_PARAM_UI_FRAME_DIM            ,
  AEC_SET_PARAM_CROP_INFO               ,
  AEC_SET_PARAM_ZSL_OP                  ,
  AEC_SET_PARAM_VIDEO_HDR               ,
  AEC_SET_PARAM_RESET_STREAM_INFO       ,
  AEC_SET_PARAM_STATS_DEBUG_MASK        , /* 50 */
  AEC_SET_PARAM_ON_OFF                  ,
  AEC_SET_PARAM_CTRL_MODE               ,
  AEC_SET_PARAM_MANUAL_EXP_TIME         ,
  AEC_SET_PARAM_MANUAL_GAIN             ,
  AEC_SET_PACK_OUTPUT                   ,
  AEC_SET_PARAM_SUPER_EVT               ,
  AEC_SET_PARAM_CAPTURE_MODE            ,
  AEC_SET_PARAM_SENSOR_ROI              , /* the roi is based on sensor output coordinate */
  AEC_SET_PARAM_DO_LED_EST_FOR_AF       ,
  AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY, /* 60 */
  AEC_SET_PARAM_PREP_FOR_SNAPSHOT_LEGACY,
  AEC_SET_PARAM_RESET_LED_EST,
  AEC_SET_PARAM_EXP_TIME                ,
  AEC_SET_PARAM_LONGSHOT_MODE           ,
  AEC_SET_PARAM_SUBSAMPLING_FACTOR      ,
  AEC_SET_PARAM_MAX
} aec_set_parameter_type;

/** _aec_sensor_info:
 *    @current_fps:           TODO
 *    @preview_fps:           TODO
 *    @snapshot_fps:          TODO
 *    @video_fps:             hardcode it to 30
 *    @max_preview_fps:       TODO
 *    @preview_linesPerFrame: TODO
 *    @snap_linesPerFrame:    TODO
 *    @snap_max_line_cnt:     TODO
 *    @max_gain:              TODO
 *    @pixel_clock:           TODO
 *    @pixel_clock_per_line:  TODO
 *    @af_is_supported:       TODO
 *    @sensor_res_width:      TODO
 *    @sensor_res_height:     TODO
 *    @pixel_sum_factor:      the binning factor
 *
 * TODO description
 **/
typedef struct _aec_sensor_info {
  uint32_t current_fps;
  uint32_t preview_fps;
  uint32_t snapshot_fps;
  uint32_t video_fps;
  uint32_t max_preview_fps;
  uint32_t preview_linesPerFrame;
  uint32_t snap_linesPerFrame;
  uint32_t snap_max_line_cnt;
  float    max_gain;
  uint32_t pixel_clock;
  uint32_t pixel_clock_per_line;
  uint32_t af_is_supported;
  uint32_t sensor_res_width;
  uint32_t sensor_res_height;
  uint16_t pixel_sum_factor;
} aec_sensor_info_t;

/** _aec_ui_frame_dim:
 *    @width:  TODO
 *    @height: TODO
 *
 * dimension of the ui (i.e preview) frame
 **/
typedef struct _aec_ui_frame_dim {
  uint32_t width;
  uint32_t height;
} aec_ui_frame_dim_t;

/** Gyro information
 *
 **/
typedef struct {
  /* Gyro data in float */
  int float_ready;
  float flt[3];
  /* Gyro data in Q16 */
  int q16_ready;
  long q16[3];
} aec_gyro_info_t;

typedef struct {

  /* Start - Tuning parameters */
  int        aec_luma_tolerance;
  int        aec_frame_skip;
  int        aec_ht_enable;
  int        aec_ht_luma_tolerance;
  float      aec_ht_thres;
  float      aec_ht_max;
  int        aec_ht_gyro_enable;
  /* End - Tuning parameters */

  /* Start - Advanced tuning parameters */
  float      aec_ref_frame_rate;
  float      aec_step_dark;
  float      aec_step_bright;
  float      aec_step_regular;
  float      aec_luma_tol_ratio_dark;
  float      aec_luma_tol_ratio_bright;
  float      aec_raw_step_adjust_cap;
  int        aec_adjust_skip_luma_tolerance;
  float      aec_dark_region_num_thres;
  float      bright_region_num_thres;

  float      aec_ht_luma_thres_low;
  float      ht_luma_thres_high;
  float      aec_ht_luma_val_low;
  float      ht_luma_val_high;
  float      aec_ht_gyro_thres_low;
  float      ht_gyro_thres_high;
  float      aec_ht_gyro_val_low;
  float      ht_gyro_val_high;
} aec_slow_smooth_conv_t;


/** aec_reserved_parameter_t
    since current chromatix does not have such parameters,
    so add them here to be passed to algorithm

*/
typedef struct {
  int hist_target_adjust_enable;
  float outdoor_max_target_adjust_ratio;
  float outdoor_min_target_adjust_ratio;
  float indoor_max_target_adjust_ratio;
  float indoor_min_target_adjust_ratio;
  float lowlight_max_target_adjust_ratio;
  float lowlight_min_target_adjust_ratio;
  float target_filter_factor;
  float hist_sat_pct;
  float hist_dark_pct;
  float hist_sat_low_ref;
  float hist_sat_high_ref;
  float hist_dark_low_ref;
  float hist_dark_high_ref;
} aec_histogram_parameter_t;

typedef struct _aec_tunable_params {
  /* this is to apply ISO values for Non-zsl preview frame*/
  uint8_t                 aec_preview_iso_enable;
  float                   aec_extreme_green_color_thld_radius;
  uint16_t                aec_start_exp_index;
  uint8_t                 aec_use_roi_for_led;
  aec_slow_smooth_conv_t  aec_slow_conv;
  aec_histogram_parameter_t aec_histogram_params;

  /*brightness step size will be aec_brightness_step_size * aec->tolerance */
  uint32_t                aec_brightness_step_size;
  uint8_t                 aec_dark_region_enable;
} aec_tuning_params_t;

/** aec_low_light_init_t:
 * luma_target: low light luma target
 * start_idx: start exposure index
 * end_idx: end exposure index
 *
 **/

typedef struct {
  uint32_t luma_target;
  uint32_t start_idx;
  uint32_t end_idx;
} aec_low_light_init_t;

/** _aec_set_parameter_init:
 *    @stats_type:     TODO
 *    @chromatix:      TODO
 *    @comm_chromatix: TODO
 *    @sensor_info:    TODO
 *    @numRegions:     information from VFE/ISP
 *    @op_mode:        op_mode can be derived from stream info
 *    @frame_dim:      TODO
 *
 * TODO description
 **/
typedef struct _aec_set_parameter_init {
  aec_stats_type_t     stats_type;
  void                 *chromatix;
  void                 *comm_chromatix;
  aec_sensor_info_t    sensor_info;
  unsigned int         numRegions;
  aec_operation_mode_t op_mode;
  aec_ui_frame_dim_t   frame_dim;
  aec_tuning_params_t  aec_tuning_params;
  aec_low_light_init_t low_light_init;
} aec_set_parameter_init_t;

/** aec_precapture_trigger_t:
 *    @trigger:    The HAL3 trigger
 *    @trigger_id: The ID of the trigger
 *
 * TODO description
 **/
typedef struct {
  uint8_t trigger;
  int32_t trigger_id;
} aec_precapture_trigger_t;

/** _aec_fps_range:
 *    @min_fps: min fpa for aec
 *    @max_fps: max fps for aec
 *
 * TODO description
 **/
typedef struct _aec_fps_range {
  int min_fps;
  int max_fps;
} aec_fps_range_t;

/** aec_precapture_mode_t:
 *
 * The HAL3 AEC precapture trigger type
 **/
typedef enum {
  AEC_PRECAPTURE_TRIGGER_IDLE,
  AEC_PRECAPTURE_TRIGGER_START
} aec_precapture_mode_t;

/** _aec_set_parameter:
 *    @type:                    TODO
 *    @init_param:              TODO
 *    @aec_metering:            TODO
 *    @iso:                     TODO
 *    @antibanding:             TODO
 *    @antibanding_status:      TODO
 *    @brightness:              TODO
 *    @exp_comp:                TODO
 *    @fps_mode:                TODO
 *    @fps:                     TODO
 *    @aec_af_hjr:              TODO
 *    @aec_roi:                 TODO
 *    @fd_roi:                  TODO
 *    @mtr_area:                TODO
 *    @strobe_mode:             TODO
 *    @redeye_mode:             TODO
 *    @sensor_update:           TODO
 *    @bestshot_mode:           TODO
 *    @aec_bracket:             TODO
 *    @strobe_cfg_st:           TODO
 *    @ez_disable:              TODO
 *    @ez_lock_output:          TODO
 *    @ez_force_exp:            TODO
 *    @ez_force_linecount:      TODO
 *    @ez_force_gain:           TODO
 *    @ez_test_enable:          TODO
 *    @ez_test_roi;             TODO
 *    @ez_test_motion:          TODO
 *    @ez_force_snapshot_exp:   TODO
 *    @ez_force_snap_linecount: TODO
 *    @ez_force_snap_gain:      TODO
 *    @aec_lock:                TODO
 *    @sensitivity_ratio:       TODO
 *    @led_est_state:           TODO
 *    @asd_param:               TODO
 *    @afd_param:               TODO
 *    @awb_param:               TODO
 *    @led_mode:                TODO
 *    @stream_crop:             TODO
 *    @zsl_op:                  TODO
 *    @video_hdr:               TODO
 *
 * TODO description
 **/
typedef struct {
    int      forced;
    uint32_t force_linecount_value;
} aec_ez_force_linecount_t;

typedef struct {
    int   forced;
    float force_gain_value;
} aec_ez_force_gain_t;

typedef struct {
    int forced;
    float force_exp_value;
} aec_ez_force_exp_t;

typedef struct {
    int      forced;
    uint32_t force_snap_linecount_value;
} aec_ez_force_snap_linecount_t;

typedef struct {
    int forced;
    float force_snap_gain_value;
} aec_ez_force_snap_gain_t;

typedef struct {
    int forced;
    float force_snap_exp_value;
} aec_ez_force_snap_exp_t;

typedef struct _aec_set_parameter {
  aec_set_parameter_type type;

  union {
    aec_set_parameter_init_t      init_param;
    aec_auto_exposure_mode_t      aec_metering;
    uint32_t                      iso;
    uint64_t                       manual_exposure_time; /* in nano-second */
    aec_antibanding_type_t        antibanding;
    aec_afd_status_t              antibanding_status;
    int                           brightness;
    int32_t                       exp_comp;
    aec_fps_mode_t                fps_mode;
    aec_fps_range_t               fps;
    uint32_t                      aec_af_hjr;
    aec_interested_region_t       aec_roi;
    aec_proc_roi_info_t           fd_roi;
    aec_proc_mtr_area_t           mtr_area;
    aec_strobe_flash_mode_t       strobe_mode;
    boolean                       redeye_mode;
    boolean                       sensor_update;
    aec_bestshot_mode_type_t      bestshot_mode;
    char                          aec_bracket[MAX_EXP_CHAR];
    q3a_strobe_confg_st_type      strobe_cfg_st;
    boolean                       ez_disable;
    boolean                       ez_lock_output;
    aec_ez_force_exp_t            ez_force_exp;
    aec_ez_force_linecount_t      ez_force_linecount;
    aec_ez_force_gain_t           ez_force_gain;
    boolean                       ez_test_enable;
    boolean                       ez_test_roi;
    boolean                       ez_test_motion;
    aec_ez_force_snap_exp_t       ez_force_snap_exp;
    aec_ez_force_snap_linecount_t ez_force_snap_linecount;
    aec_ez_force_snap_gain_t      ez_force_snap_gain;
    boolean                       ez_running;
    boolean                       aec_lock;
    boolean                       aec_enable;
    float                         sensitivity_ratio;
    q3a_led_flash_state_t         led_est_state;
    aec_set_asd_param_t           asd_param;
    aec_set_afd_parm_t            afd_param;
    aec_set_awb_parm_t            awb_param;
    q3a_led_flash_mode_t          led_mode;
    q3a_stream_crop_t             stream_crop;
    int32_t                       zsl_op;
    int32_t                       video_hdr;
    uint32_t                      stats_debug_mask;
    uint32_t                      subsampling_factor;

    aec_precapture_trigger_t      aec_trigger;
    uint8_t                       aec_ctrl_mode;
    uint32_t                      current_frame_id;
    uint32_t                      capture_type;
    boolean                       est_for_af;

    aec_algo_gyro_info_t          gyro_info;
    int32_t                       chromaflash_enable;
    boolean                       longshot_mode;
  } u;
} aec_set_parameter_t;

typedef enum _aec_operation_type {
  AEC_OPERATION_TYPE_2D_NORM,
  AEC_OPERATION_TYPE_2D_ZSL,
  AEC_OPERATION_TYPE_3D_NORM,
  AEC_OPERATION_TYPE_3D_ZSL,
} aec_operation_type_t;

/** _aec_ez_tune:
 *    @test_enable:          TODO
 *    @test_roi:             TODO
 *    @test_motion:          TODO
 *    @force_snapshot_exp:   TODO
 *    @disable:              TODO
 *    @lock_output:          TODO
 *    @force_exp:            TODO
 *    @force_linecount:      TODO
 *    @force_gain:           TODO
 *    @force_snap_linecount: TODO
 *    @force_snap_gain:      TODO
 *    @stored_gain:          TODO
 *    @stored_line_count:    TODO
 *    @touch_roi_luma:       TODO
 *    @preview_exp_time:     TODO
 *
 * TODO description
 **/
typedef struct __attribute__((__packed__)) {
  boolean                        test_enable;
  boolean                        test_roi;
  boolean                        test_motion;
  int                            disable;
  boolean                        lock_output; /* ??? */
  aec_ez_force_exp_t             force_exp;
  aec_ez_force_linecount_t       force_linecount;
  aec_ez_force_gain_t            force_gain;
  aec_ez_force_snap_linecount_t  force_snap_linecount;
  aec_ez_force_snap_gain_t       force_snap_gain;
  aec_ez_force_snap_exp_t        force_snapshot_exp;
  float                          stored_gain; /* ??? */
  uint32_t                       stored_line_count; /* ???*/
  uint32_t                       touch_roi_luma; /* ??? */
  boolean                        running;
  boolean                        enable;
  int32_t                        luma;
  int32_t                        lock;
  int32_t                        exposure_index;
  int32_t                        lux_index;
  int32_t                        preview_linecount;
  float                          preview_realgain;
  float                          preview_exp_time;
  int32_t                        snap_linecount;
  float                          snap_realgain;
  float                          snap_exp_time;
  int32_t                        antibanding_enable;
} aec_ez_tune_t;

/** _aec_output_data
 *    @stats_update:            TODO
 *    @result:                  TODO
 *    @type:                    TODO
 *    @aec_af:                  AF related update
 *    @aec_awb:                 AWB related update
 *    @aec_asd:                 ASD related update
 *    @exp_index:               TODO
 *    @indoor_index:            TODO
 *    @outdoor_index:           TODO
 *    @lux_idx:                 TODO
 *    @pixelsPerRegion:         TODO
 *    @numRegions:              TODO
 *    @SY:                      TODO
 *    @preview_fps:             TODO
 *    @afr_enable:              TODO
 *    @metering_type:           TODO
 *    @iso:                     TODO
 *    @preview_exp_time:        TODO
 *    @aec_settled:             TODO
 *    @stored_digital_gain:     TODO
 *    @target_luma:             TODO
 *    @cur_luma:                TODO
 *    @high_luma_region_count:  TODO
 *    @cur_line_cnt:            TODO
 *    @cur_real_gain:           TODO
 *    @prev_sensitivity:        TODO
 *    @exp_tbl_val:             TODO
 *    @max_line_cnt:            TODO
 *    @sof_update:              TODO
 *    @comp_luma:               TODO
 *    @led_state:               TODO
 *    @use_led_estimation:      TODO
 *    @led_frame_skip_cnt:      TODO
 *    @max_led_frame_skip:      TODO
 *    @aec_flash_settled:       TODO
 *    @use_strobe:              TODO
 *    @strobe_len:              TODO
 *    @flash_si:                TODO
 *    @strobe_cfg_st:           TODO
 *    @prep_snap_no_led:        TODO
 *    @band_50hz_gap:           TODO
 *    @cur_atb:                 TODO
 *    @hjr_snap_frame_cnt:      TODO
 *    @asd_extreme_green_cnt:   TODO
 *    @asd_extreme_blue_cnt:    TODO
 *    @asd_extreme_tot_regions: total bayer stat regions 64x48
 *                              used to get ratio of extreme stats.
 *    @hjr_dig_gain:            TODO
 *    @snap:                    TODO
 *    @eztune:                  TODO
 *    @iso_Exif:                TODO
 *    @config:                  TODO
 *
 * TODO description
 **/
typedef struct _aec_output_data {
  stats_update_t           stats_update;
  boolean                  result;
  aec_output_type_t        type;
  aec_update_af_t          aec_af;
  aec_update_awb_t         aec_awb;
  aec_update_asd_t         aec_asd;
  int                      exp_index;
  int                      indoor_index;
  int                      outdoor_index;
  float                    lux_idx;
  int                      pixelsPerRegion;
  unsigned int             numRegions;
  unsigned int             SY[256];
  int                      preview_fps;
  boolean                  afr_enable;
  aec_auto_exposure_mode_t metering_type;
  uint32_t                 iso;
  float                    preview_exp_time;
  int                      aec_settled;
  float                    stored_digital_gain;
  uint32_t                 target_luma;
  uint32_t                 cur_luma;
  int                      high_luma_region_count;
  uint32_t                 cur_line_cnt;
  float                    cur_real_gain;
  float                    prev_sensitivity;
  uint32_t                 exp_tbl_val;
  int                      max_line_cnt;
  int                      comp_luma;
  boolean                  use_strobe;
  uint32_t                 strobe_len;
  q3q_flash_sensitivity_t  flash_si;
  q3a_strobe_confg_st_type strobe_cfg_st;
  float                    band_50hz_gap;
  aec_antibanding_type_t   cur_atb;
  int                      hjr_snap_frame_cnt;
  int                      asd_extreme_green_cnt;
  int                      asd_extreme_blue_cnt;
  int                      asd_extreme_tot_regions;
  uint32_t                 hjr_dig_gain;
  aec_proc_snapshot_t      snap;
  aec_ez_tune_t            eztune;
  short                    iso_Exif;
  float                    Bv_Exif;
  float                    Av_Exif;
  float                    Sv_Exif;
  float                    Tv_Exif;
  aec_config_t             config;
  int                      need_config;
  char                     aec_debug_data_array[AEC_DEBUG_DATA_SIZE];
  uint32_t                 aec_debug_data_size;
} aec_output_data_t;

typedef enum {
  AR_UNDEF   = -1,
  AR_4_TO_3  =  0, /* used as index into the weight tables */
  AR_16_TO_9 =  1,
} aec_aspect_ratio_t;

typedef boolean (* aec_set_parameters_func)(aec_set_parameter_t *param,
  void *aec_obj);
typedef boolean (* aec_get_parameters_func)(aec_get_parameter_t *param,
  void *aec_obj);
typedef boolean (* aec_process_func)(stats_t *stats, void *aec_obj,
  aec_output_data_t *output);
typedef void (* aec_callback_func)(aec_output_data_t *output, void *port);
typedef void *(* aec_init_func)(void);
typedef void (* aec_deinit_func)(void *aec);

/** _aec_object:
 *    @obj_lock:       TODO
 *    @aec:            typecase to aec_internal_control_t
 *    @stats:          TODO
 *    @set_parameters: TODO
 *    @get_parameters: TODO
 *    @process:        TODO
 *    @init:           TODO
 *    @deinit:         TODO
 *    @cb:             TODO
 *    @output:         TODO
 *    @port:           TODO
 *    @thread_data:    typecase to q3a_thread_data_t
 *
 * TODO description
 **/
typedef struct _aec_object {
  pthread_mutex_t         obj_lock;
  void                    *aec;
  stats_t                 stats;
  aec_set_parameters_func set_parameters;
  aec_get_parameters_func get_parameters;
  aec_process_func        process;
  aec_init_func           init;
  aec_deinit_func         deinit;
  aec_callback_func       cb;
  aec_output_data_t       output;
  void                    *port;
  void                    *thread_data;
} aec_object_t;

void aec_load_function(aec_object_t *aec_object, unsigned int flag);

#endif /* __AEC_H__ */
