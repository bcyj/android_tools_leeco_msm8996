/* awb.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AWB_H__
#define __AWB_H__
/* Data sturcture for stats: AWB */
#include "chromatix_common.h"
#include "q3a_stats.h"
#include "mct_event_stats.h"

#ifdef AWB_DEBG_HIGH
#define CDBG_AWB  CDBG_HIGH
#else
#undef CDBG_AWB
#define CDBG_AWB  CDBG
#endif

/* Tuning param values controlled from outside of algorithm */
#define INITIAL_CCT                       (4100) /* TL84 */
#define K1_FLASH_SENSITIVITY_NORMAL_LIGHT (7.5)
#define K1_FLASH_SENSITIVITY_WARM_LIGHT   (3.0)

#define AWB_MAX_HISTORY                   (30)// Max possible value = 30
#define AWB_AEC_MAX_HISTORY               (30)// Max possible value = 30
#define STAT_SAT_TH                       (75)
#define ALL_OUTLIER_HEURISTIC_FLAG        (0) /*1:enable all outlier heuristic; 0: disable all outlier heuristic*/
#define DAY_LOCK_ENABLE                   (1)
#define F_LOCK_ENABLE                     (0)
#define A_LOCK_ENABLE                     (0)
#define DAY_STABILITY_ENABLE              (1)
#define F_STABILITY_ENABLE                (1)
#define A_STABILITY_ENABLE                (1)
#define H_STABILITY_ENABLE                (1)
#define STABLE_RANGE_THRESHOLD            (15)
#define HISTORY_SAVE_AVERAGE              (0)
#define GREEN_ZONE_TOP_RG_OFFSET          (-30)
#define GREY_WEIGHT_DAY                   (0.8)
#define GREY_WEIGHT_F                     (1)
#define GREY_WEIGHT_A                     (1)
#define GREY_WEIGHT_H                     (1)
#define WHITE_WEIGHT_DAY                  (0.2)
#define WHITE_WEIGHT_F                    (1)
#define WHITE_WEIGHT_A                    (1)
#define WHITE_WEIGHT_H                    (1)
#define WHITE_STAT_CNT_TH                 (3)
#define WHITE_YAMX_YMID_DIST_TH           (5)
#define WHITE_HISTORY_WEIGHT              (1)
#define WHITE_HISTORY_EXP_TH              (5)
#define OUTLIER_DIST2_A_H_LEFT            (6)
#define D50_D65_WEIGHTED_SAMPLE_BOUNDARY  (1.0/3.0)
#define AWB_DAY_ZONE_LEFT_OUTLIER_DIST    (12)
#define AWB_DAY_ZONE_TOP_OUTLIER_DIST     (8)


#define AWB_SUBSAMPLE               (4)
#define MIN_AWB_SUBSAMPLE           (1)
#define AWB_STATS_PROC_FREQ_PREVIEW (2)
#define AWB_STATS_PROC_FREQ_VIDEO   (2)

/* Dual Led Params */
#define DUAL_LED_HIGH_LOW_LED_RATIO (7.5f)
#define DUAL_LED_INTERSECT_SLOPE    (2.0f)
#define DUAL_LED_RED_RG_ADJ         (1.0f)
#define DUAL_LED_RED_BG_ADJ         (1.0f)
#define DUAL_LED_BLUE_RG_ADJ        (1.0f)
#define DUAL_LED_BLUE_BG_ADJ        (1.0f)
#define USE_ENHANCED_K_INTERPOLATION (0)

/* For Dual LED Calibration
 * Manual LED control */
#define DUAL_LED_MANUAL_SETTING_NUM 11
static awb_dual_led_settings_t manual_settings[] = {
  {200, 0, 0, 0},
  {180, 20, 0, 0},
  {160, 40, 0, 0},
  {140, 60, 0, 0},
  {120, 80, 0, 0},
  {100, 100, 0, 0},
  {80, 120, 0, 0},
  {60, 140, 0, 0},
  {40, 160, 0, 0},
  {20, 180, 0, 0},
  {0, 200, 0, 0}
};

/* Dual LED tuning data, rg/bg ratios
 * Filled with golden led module data
 * Need to match the number of entries in Chromatix */
static float dual_led_ratios[] = {
  1.0998f, 0.2901f,   // rg_ratio, bg_ratio (Entry 1)
  0.9953f, 0.3404f,   // rg_ratio, bg_ratio (Entry 2)
  0.9058f, 0.3859f,   // rg_ratio, bg_ratio (Entry 3)
  0.8659f, 0.4075f,   // rg_ratio, bg_ratio (Entry 4)
  0.7927f, 0.4468f,   // rg_ratio, bg_ratio (Entry 5)
  0.7416f, 0.4751f,   // rg_ratio, bg_ratio (Entry 6)
  0.6929f, 0.5022f,   // rg_ratio, bg_ratio (Entry 7)
  0.6327f, 0.5371f,   // rg_ratio, bg_ratio (Entry 8)
  0.6035f, 0.5542f,   // rg_ratio, bg_ratio (Entry 9)
  0.5466f, 0.5893f,   // rg_ratio, bg_ratio (Entry 10)
  0.4884f, 0.6219f    // rg_ratio, bg_ratio (Entry 11)
};


typedef enum {
  AWB_STATS_YUV,
  AWB_STATS_BAYER
} awb_stats_type_t;

typedef enum {
  CAMERA_WB_MIN_MINUS_1 = -1,
  CAMERA_WB_AUTO = 0,  /* This list must match aeecamera.h */
  CAMERA_WB_CUSTOM,
  CAMERA_WB_INCANDESCENT,
  CAMERA_WB_FLUORESCENT,
  CAMERA_WB_WARM_FLUORESCENT,
  CAMERA_WB_DAYLIGHT,
  CAMERA_WB_CLOUDY_DAYLIGHT,
  CAMERA_WB_TWILIGHT,
  CAMERA_WB_SHADE,
  CAMERA_WB_MANUAL,
  CAMERA_WB_OFF,
  CAMERA_WB_MAX_PLUS_1
} awb_config3a_wb_t;

typedef enum {
  AWB_UPDATE,
  AWB_SEND_EVENT,
} awb_output_type_t;
#if 0 /*remove later: Marvin*/
typedef enum {
  AWB_BESTSHOT_OFF = 0,
  AWB_BESTSHOT_AUTO = 1,
  AWB_BESTSHOT_LANDSCAPE = 2,
  AWB_BESTSHOT_SNOW,
  AWB_BESTSHOT_BEACH,
  AWB_BESTSHOT_SUNSET,
  AWB_BESTSHOT_NIGHT,
  AWB_BESTSHOT_PORTRAIT,
  AWB_BESTSHOT_BACKLIGHT,
  AWB_BESTSHOT_SPORTS,
  AWB_BESTSHOT_ANTISHAKE,
  AWB_BESTSHOT_FLOWERS,
  AWB_BESTSHOT_CANDLELIGHT,
  AWB_BESTSHOT_FIREWORKS,
  AWB_BESTSHOT_PARTY,
  AWB_BESTSHOT_NIGHT_PORTRAIT,
  AWB_BESTSHOT_THEATRE,
  AWB_BESTSHOT_ACTION,
  AWB_BESTSHOT_AR,
  AWB_BESTSHOT_MAX
} awb_bestshot_mode_type_t;
#endif

typedef enum _awb_set_parameter_type {
  AWB_SET_PARAM_INIT_CHROMATIX_SENSOR   = 1,
  AWB_SET_PARAM_WHITE_BALANCE,
  AWB_SET_PARAM_RESTORE_LED_GAINS,
  AWB_SET_PARAM_LOCK,
  AWB_SET_PARAM_BESTSHOT,
  AWB_SET_PARAM_EZ_DISABLE,
  AWB_SET_PARAM_EZ_LOCK_OUTPUT,
  AWB_SET_PARAM_LINEAR_GAIN_ADJ,
  AWB_SET_PARAM_AEC_PARM,
  AWB_SET_PARAM_OP_MODE,
  AWB_SET_PARAM_VIDEO_HDR,
  AWB_SET_PARAM_STATS_DEBUG_MASK,
  AWB_SET_PARAM_ENABLE,
  AWB_SET_PARAM_EZ_TUNE_RUNNING,
  AWB_SET_PARAM_MANUAL_WB,
  AWB_SET_PARAM_SUBSAMPLING_FACTOR,
} awb_set_parameter_type;

typedef q3a_operation_mode_t awb_operation_mode_t;


typedef struct _awb_tunable_params {
  uint8_t             awb_stats_proc_freq;
  uint32_t            awb_initial_cct;
  float               awb_k1_flash_sensitivity_normal_light;
  float               awb_k1_flash_sensitivity_warm_light;
  uint16_t            awb_max_history;
  uint16_t            awb_aec_max_history;
  uint8_t             awb_stat_sat_th;
  uint8_t             awb_all_outlier_heuristic_flag;
  uint8_t             awb_day_lock_enable;
  uint8_t             awb_f_lock_enable;
  uint8_t             awb_a_lock_enable;
  uint8_t             awb_day_stability_enable;
  uint8_t             awb_f_stability_enable;
  uint8_t             awb_a_stability_enable;
  uint8_t             awb_h_stability_enable;
  uint8_t             awb_stable_range_threshold;
  uint8_t             awb_history_save_average;
  int8_t              awb_green_zone_top_rg_offset;
  float               awb_grey_weight_day;
  float               awb_grey_weight_f;
  float               awb_grey_weight_a;
  float               awb_grey_weight_h;
  float               awb_white_weight_day;
  float               awb_white_weight_f;
  float               awb_white_weight_a;
  float               awb_white_weight_h;
  uint16_t            awb_white_stat_cnt_th;
  uint8_t             awb_white_yamx_ymid_dist_th;
  uint8_t             awb_white_history_weight;
  uint8_t             awb_white_history_exp_th;
  uint8_t             awb_outlier_dist2_a_h_left;
  float               awb_d50_d65_weighted_sample_boundary;
  int32_t             awb_day_zone_left_outlier_dist;
  int32_t             awb_day_zone_top_outlier_dist;

  /* Dual led params */
  float               awb_dual_led_high_low_led_ratio;
  float               awb_dual_led_intersect_slope;
  float               awb_dual_led_red_rg_adj;
  float               awb_dual_led_red_bg_adj;
  float               awb_dual_led_blue_rg_adj;
  float               awb_dual_led_blue_bg_adj;
  uint8_t             awb_use_enhanced_k_interpolation;
} awb_tuning_params_t;

typedef struct _awb_set_parameter_init {
  awb_stats_type_t     stats_type;

  void                 *chromatix;
  void                 *comm_chromatix;
  void                 *dual_led_ratios;

  /* op_mode can be derived from stream info */
  awb_operation_mode_t op_mode;
  awb_tuning_params_t  awb_tuning_params;

} awb_set_parameter_init_t;

typedef struct {
   int   exp_index;
   int   indoor_index;
   int   outdoor_index;
   float lux_idx;
   int   aec_settled;

   /* Luma */
   uint32_t target_luma;
   uint32_t cur_luma;
   uint32_t average_luma;
   /* exposure */
   uint32_t cur_line_cnt;
   float cur_real_gain;
   float stored_digital_gain;
   uint32_t cur_preview_fps;

   q3q_flash_sensitivity_t  flash_sensitivity;

   /*Led state*/
   int led_state;
   int use_led_estimation;
   int aec_flash_settled;
   uint32_t exp_tbl_val;
   aec_led_est_state_t est_state;
} awb_set_aec_parms;

typedef enum {
  MANUAL_WB_MODE_CCT,
  MANUAL_WB_MODE_GAIN,
  MANUAL_WB_MODE_MAX
} manual_wb_mode_type;

typedef struct {
  manual_wb_mode_type type;
  union {
    int32_t cct;
    awb_gain_t gains;
  } u;
} manual_wb_parm_t;

typedef struct _awb_set_parameter {
  awb_set_parameter_type type;

  union {
    awb_set_parameter_init_t          init_param;
    int32_t                           awb_current_wb;
    manual_wb_parm_t                  manual_wb_params;
    int32_t                           awb_best_shot;
    int                               ez_disable;
    int                               ez_lock_output;
    int                               linear_gain_adj;
    awb_set_aec_parms                 aec_parms;
    boolean                           awb_lock;
    boolean                           awb_enable;
    int32_t                           video_hdr;
    uint32_t                          stats_debug_mask;
    boolean                           ez_running;
    uint32_t                          subsampling_factor;
  } u;
} awb_set_parameter_t;

typedef struct {
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t6;
  uint32_t t4;
  uint32_t mg;
  uint32_t t5;
}awb_exterme_col_param_t;

typedef struct {
  uint32_t regionW;
  uint32_t regionH;
  uint32_t regionHNum;
  uint32_t regionVNum;
  uint32_t regionHOffset;
  uint32_t regionVOffset;
}awb_stats_region_info_t;


typedef struct {
  chromatix_manual_white_balance_type gain;
  uint32_t color_temp;
  chromatix_wb_exp_stats_type bounding_box;
  //awb_stats_region_info_t region_info;
  awb_exterme_col_param_t exterme_col_param;
}stats_proc_awb_params_t;

typedef struct {
  chromatix_manual_white_balance_type curr_gains;
  uint32_t color_temp;
}stats_proc_awb_gains_t;

/* AWB GET DATA */
typedef enum {
  AWB_PARMS,
  AWB_GAINS,
} awb_get_t;

typedef struct {
  awb_get_t type;
  union {
    stats_proc_awb_params_t awb_params;
    stats_proc_awb_gains_t  awb_gains;
  } d;
} stats_proc_get_awb_data_t;


typedef struct _awb_get_parameter {
  awb_get_t type;
    union {
      stats_proc_awb_params_t awb_params;
      stats_proc_awb_gains_t  awb_gains;
    } u;
} awb_get_parameter_t;

typedef struct _awb_output_eztune_data {
  boolean  awb_enable;
  boolean  ez_running;
  int      prev_exp_index;
  int      valid_sample_cnt;
  int      n_outlier;
  float    day_rg_ratio;
  float    day_bg_ratio;
  int      day_cluster;
  int      day_cluster_weight_distance;
  int      day_cluster_weight_illuminant;
  int      day_cluster_weight_dis_ill;
  float    f_rg_ratio;
  float    f_bg_ratio;
  int      f_cluster;
  int      f_cluster_weight_distance;
  int      f_cluster_weight_illuminant;
  int      f_cluster_weight_dis_ill;
  float    a_rg_ratio;
  float    a_bg_ratio;
  int      a_cluster;
  int      a_cluster_weight_distance;
  int      a_cluster_weight_illuminant;
  int      a_cluster_weight_dis_ill;
  float    h_rg_ratio;
  float    h_bg_ratio;
  int      h_cluster;
  int      h_cluster_weight_distance;
  int      h_cluster_weight_illuminant;
  int      h_cluster_weight_dis_ill;
  int      sgw_cnt;
  float    sgw_rg_ratio;
  float    sgw_bg_ratio;
  int      green_line_mx;
  int      green_line_bx;
  int      green_zone_top;
  int      green_zone_bottom;
  int      green_zone_left;
  int      green_zone_right;
  float    outdoor_green_rg_ratio;
  float    outdoor_green_bg_ratio;
  float    outdoor_green_grey_rg_ratio;
  float    outdoor_green_grey_bg_ratio;
  int      outdoor_green_cnt;
  int      green_percent;
  float    slope_factor_m;
  int      extreme_b_mag;
  int      nonextreme_b_mag;
  int      ave_rg_ratio_x;
  int      ave_bg_ratio_x;
  int      weighted_sample_rg_grid;
  int      weighted_sample_bg_grid;
  float    weighted_sample_day_rg_ratio;
  float    weighted_sample_day_bg_ratio;
  float    weighted_sample_day_shade_rg_ratio;
  float    weighted_sample_day_shade_bg_ratio;
  float    weighted_sample_day_d50_rg_ratio;
  float    weighted_sample_day_d50_bg_ratio;
  float    weighted_sample_fah_rg_ratio;
  float    weighted_sample_fah_bg_ratio;
  float    white_rg_ratio;
  float    white_bg_ratio;
  int      white_stat_y_threshold_low;
  int      unsat_y_min_threshold;
  int      unsat_y_max;
  int      unsat_y_mid;
  int      unsat_y_day_max;
  int      unsat_y_f_max;
  int      unsat_y_a_max;
  int      unsat_y_h_max;
  float    sat_day_rg_ratio;
  float    sat_day_bg_ratio;
  int      sat_day_cluster;
  float    sat_f_rg_ratio;
  float    sat_f_bg_ratio;
  int      sat_f_cluster;
  float    sat_a_rg_ratio;
  float    sat_a_bg_ratio;
  int      sat_a_cluster;
  float    sat_h_rg_ratio;
  float    sat_h_bg_ratio;
  int      sat_h_cluster;
  float    max_compact_cluster;
  int      count_extreme_b_mcc;
  int      green_zone_right2;
  int      green_line_bx2;
  int      green_zone_bottom2;
  int      output_is_confident;
  int      output_sample_decision;
  float    output_wb_gain_r;
  float    output_wb_gain_g;
  float    output_wb_gain_b;
  float    regular_ave_rg_ratio;
  float    regular_ave_bg_ratio;
  float    cct_awb_bayer;
  int      count_extreme_b;
  float    r_gain;
  float    g_gain;
  float    b_gain;
  int      color_temp;
  int      decision;
  int      samp_decision[64];
  boolean  lock;
} awb_output_eztune_data_t;

/** _awb_output_data
 *
 **/
typedef struct _awb_output_data {
  stats_update_t  stats_update;
  float  r_gain;
  float  g_gain;
  float  b_gain;
  int    color_temp;
  int    awb_update;
  int    decision;
  int    samp_decision[64];
  int    wb_mode;
  int    best_mode;
  uint32_t frame_id;
  awb_output_type_t type;
  awb_output_eztune_data_t eztune_data;
  char awb_debug_data_array[AWB_DEBUG_DATA_SIZE];
  uint32_t  awb_debug_data_size;
  uint8_t   awb_stats_proc_freq;
  int gains_restored;
  awb_dual_led_settings_t  dual_led_settings;
} awb_output_data_t;
/*Data structure for awb ends */

typedef boolean (* awb_set_parameters_func)(awb_set_parameter_t *param,
  void *awb_obj);

typedef boolean (* awb_get_parameters_func)(awb_get_parameter_t *param,
  void *awb_obj);

typedef void    (* awb_process_func)(stats_t *stats,
  void *awb_obj, awb_output_data_t *output);

typedef struct {
  awb_set_parameters_func set_parameters;
  awb_get_parameters_func get_parameters;
  awb_process_func        process;
}awb_ops_t;

void *awb_init( awb_ops_t *awb_ops);
void awb_destroy(void *awb_obj);
#endif /* __AWB_H__ */
