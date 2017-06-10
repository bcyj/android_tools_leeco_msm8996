/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#ifndef __AWB_H__
#define __AWB_H__

#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include <media/msm_camera.h>
#include "camera_dbg.h"
#include "stats_proc.h"

#if(AWB_DEBUG_HIGH)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AWB"
#endif
#if(AWB_DEBUG_LOW)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AWB"
  #define CDBG_AWB(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_AWB(fmt, args...) do{}while(0)
#endif

#define AWB_LOW_AGGRESSIVENESS            0.05
#define AWB_MED_AGGRESSIVENESS            0.10
#define AWB_HIGH_AGGRESSIVENESS           0.30
#define RGBG_GRID_Q_NUM                   10
#define YMIN_HIGH_LIMIT                   98    /* percent */
#define YMIN_LOW_LIMIT                    60    /* percent */
#define AGW_NUMBER_GRID_POINT             241
#define AGW_F_A_LINE_NUM_POINTS           128
#define GREEN_Q_NUM                       8
#define AWB_MAX_HISTORY                   15
#define AWB_AEC_MAX_HISTORY               25

#define SELF_CAL_AVE_LOW_THRESHOLD        50
#define SELF_CAL_NW_SE_AVE_LOW_THRESHOLD  15
#define SELF_CAL_LOCK_THRESHOLD           500

#define YCBCR_TO_RGB_Q_NUM 5
#define YCBCR_STATS_OFFSET 128
#define Q5_ROUNDOFF 16

#define AWB_NUMBER_OF_REFERENCE_POINT  AGW_AWB_HYBRID
#define STATS_PROC_AWB_NUMBER_OF_LIGHTING_CONDITION  \
  (AGW_AWB_HYBRID + 1)

/* AWB ENUMS */
typedef enum {
  AWB_STAT_REGULAR = 0,
  AWB_STAT_WHITE
} awb_stat_config_type;

typedef enum {
  AWB_AGW_INDOOR_ONLY=0,
  AWB_AGW_OUTDOOR_ONLY,
  AWB_AGW_INOUTDOOR_BOTH
} agw_srch_mode_t;

/* AWB STRUCTS */
typedef struct {
  int     bin_cnt;
  int region_cnt;
  int rx[64];
  int gx[64];
  int bx[64];
} awb_input_stats_type;

typedef struct {
  int8_t   is_confident;
  chromatix_awb_light_type sample_decision;
  float   wb_gain_r;
  float   wb_gain_g;
  float   wb_gain_b;
  float   gain_adj_r;
  float   gain_adj_b;
} awb_gain_t;

typedef enum {
  STATS_PROC_LOW_AGGRESSIVENESS,
  STATS_PROC_MED_AGGRESSIVENESS,
  STATS_PROC_HIGH_AGGRESSIVENESS
} stats_proc_aggressiveness_t;

/* AWB History Struct */
typedef struct {
  int rg_ratio;
  int bg_ratio;
  int exp_index;
  int is_white;
  int replaced;
  chromatix_awb_light_type decision;
} awb_history_t;

/* AWB AEC History Struct */
typedef struct {
  int exp_index;
  int frame_luma;
} awb_aec_history_t;

typedef struct {
  int     awb_weight_vector[STATS_PROC_AWB_NUMBER_OF_LIGHTING_CONDITION][3];
  int     n_day1;
  int     n_day2;
  int     day_line_1[AGW_NUMBER_GRID_POINT][2];
  int     day_line_2[AGW_NUMBER_GRID_POINT][2];
  int     rg_grid[AWB_NUMBER_OF_REFERENCE_POINT];
  int     bg_grid[AWB_NUMBER_OF_REFERENCE_POINT];
  int     sample_rg_grid[256];
  int     sample_bg_grid[256];

  int     n_fline, Fline[AGW_F_A_LINE_NUM_POINTS][2];
  int     n_aline1, Aline1[AGW_F_A_LINE_NUM_POINTS][2];
  int     n_aline2, Aline2[AGW_F_A_LINE_NUM_POINTS][2];
  int     n_day_f_line, Day_F_line[AGW_F_A_LINE_NUM_POINTS][2];

  int     green_offset_rg;
  int     green_offset_bg;
  int     green_line_mx;
  int     green_line_bx;

  /* AWB History */
  awb_history_t awb_history[AWB_MAX_HISTORY];
  uint32_t awb_history_next_pos;
  int awb_history_count;

  /* AEC History */
  awb_aec_history_t aec_history[AWB_AEC_MAX_HISTORY];
  uint32_t aec_history_next_pos;
  int aec_history_count;

  int     n_day3;
  int     day3_rg[AGW_NUMBER_GRID_POINT];
  int     day3_bg[AGW_NUMBER_GRID_POINT];

  int     led_rg_ratio_x;  /* R/G ratio for LED light in fixed point */
  int     led_bg_ratio_x;  /* B/G ratio for LED light in fixed point */
  int     led_rg_grid;    /* grid coordinate for LED R/G */
  int     led_bg_grid;    /* grid coordinate for LED B/G */
  int     indoor_F_WB_locked;

  int     led_fired_for_this_frame; /* for LED AWB */

  int     outlier_rg_grid[256];
  int     outlier_bg_grid[256];

  int     red_gain_table_x[AWB_NUMBER_OF_REFERENCE_POINT];
  int     blue_gain_table_x[AWB_NUMBER_OF_REFERENCE_POINT];
  int     rgbg_grid_x[AGW_NUMBER_GRID_POINT + 1];

  float   red_gain_table[AWB_NUMBER_OF_REFERENCE_POINT];
  float   blue_gain_table[AWB_NUMBER_OF_REFERENCE_POINT];
  float   red_gain_adj[AWB_NUMBER_OF_REFERENCE_POINT];
  float   blue_gain_adj[AWB_NUMBER_OF_REFERENCE_POINT];
  float   rgbg_grid[AGW_NUMBER_GRID_POINT + 1];
  float   green_line_m, green_line_b;

  float   noon_rg;
  float   noon_bg;
  float   shifted_d50_rg;
  float   shifted_d50_bg;
  float   led_rg_ratio;  /* R/G ratio for LED light */
  float   led_bg_ratio;  /* B/G ratio for LED light */

  float   led_off_last_rg;  /* for LED AWB */
  float   led_off_last_bg;  /* for LED AWB */
  float   led_on_last_rg;   /* for LED AWB */
  float   led_on_last_bg;   /* for LED AWB */

  int outlier_distance;
  long int indoor_index;
  long int outdoor_index;
  float  awb_r_adj_VF[AGW_AWB_MAX_LIGHT - 1];
  float  awb_b_adj_VF[AGW_AWB_MAX_LIGHT - 1];
} awb_advanced_grey_world_t;

typedef struct {
  int     gx;
  int     g_ave;
  int     g_max;
  int     min_dist;
  int     max_dist;
  int     x_cnt;
  int     ave_rg_ratio_x;
  int     ave_bg_ratio_x;
  int     x_rg_grid;
  int     x_bg_grid;
  int     ave_rg_grid;
  int     ave_bg_grid;
  int     outlier_cnt;
  int     simple_rg_ratio_x;
  int     simple_bg_ratio_x;
  int     rg_ratio_x;
  int     bg_ratio_x;
  int     all_outliers;
  int     smpl_cnt;
  int     wt_per_smpl;
  int     compact_cluster;
  int     a_line;
  int     a_idx;
  int     a_cluster;
  int     a_cluster_rg_x;
  int     a_cluster_bg_x;
  int     f_idx;
  int     f_cluster;
  int     f_cluster_rg_x;
  int     f_cluster_bg_x;
  int     day_line;
  int     day_cluster;
  int     day_cluster_rg_x;
  int     day_cluster_bg_x;
  int     day_idx;
  int     p1; /* boundry points */
  int     p2;
  int     p3;
  int     p4;
  int     green_bgx;
  int     decision_changed;
  int     low_light;
  int     green_cnt;
  int     indoor_green_cnt;
  int     aec_luma_delta;

  float   r_gain;
  float   g_gain;
  float   b_gain;
  float   simple_rg_ratio;
  float   simple_bg_ratio;
  float   ave_rg_ratio;
  float   ave_bg_ratio;
  float   rg_ratio;
  float   bg_ratio;
  float   d55_rg;
  float   d55_bg;
  float   rg_target;
  float   bg_target;
  chromatix_awb_light_type  sample_decision;
  awb_gain_t output;
} awb_advanced_grey_world_algo_data_t;

typedef struct {
  int A11;
  int A12;
  int A13;
  int A21;
  int A22;
  int A23;
  int A31;
  int A32;
  int A33;
} awb_stats_convert_coeff_info_t;

typedef struct {
  float   ave_rg;
  float   ave_bg;
  int     ave_cnt;
  float   nw_ave_rg;
  float   nw_ave_bg;
  int     nw_ave_cnt;
  float   se_ave_rg;
  float   se_ave_bg;
  int     se_ave_cnt;
  float   prev_fx;
  float   prev_fy;
  int     enable;
} awb_self_cal_t;

typedef struct {
  float                       stored_bst_blue_gain_adj;
  config3a_wb_t               stored_wb;
  camera_bestshot_mode_type   curr_mode;
} awb_bestshot_data_t;

typedef struct {
  int disable;
  int lock_output;
  chromatix_manual_white_balance_type stored_gains;
} awb_ez_tune_t;

typedef struct {
  int      self_cal_flag;
  int      old_illuminant_choice;
  int      old_illuminant_choice_count;
  int      toggle_frame_skip;

  int      reg_green_cnt;
  int      reg_blue_cnt;
  int      white_y_min_percent;
  int      white_has_single_peak;

  int      inoutdoor_midpoint;
  int      outdoor_midpoint;

  uint32_t prev_r_gain_adj; /* red gain adjustment in Q7 */
  uint32_t prev_b_gain_adj; /* blue gain adjustment in Q7 */

  float    bst_blue_gain_adj; /* Internal during init */

  float    stored_prev_r_gain;
  float    stored_prev_g_gain;
  float    stored_prev_b_gain;

  float    regular_ave_rg_ratio;
  float    regular_ave_bg_ratio;
  float    white_ave_rg_ratio;
  float    white_ave_bg_ratio;

  agw_srch_mode_t                     search_mode;
  awb_stat_config_type                current_awb_stat_config;
  chromatix_awb_light_type            white_decision;
  chromatix_awb_light_type            regular_decision;
  chromatix_manual_white_balance_type last_wb;
  awb_stats_convert_coeff_info_t      awb_stats_conv_coef;

  awb_advanced_grey_world_t  agw;
  awb_advanced_grey_world_algo_data_t agw_d;
  awb_input_stats_type       stats_ptr;
  awb_self_cal_t             self_cal;
  awb_bestshot_data_t        bestshot_d;
  awb_ez_tune_t              eztune;  /* EZtune Set Parameters */
  int linear_gain_adj;
  float red_gain_adjust;
  float blue_gain_adjust;
} awb_t;

/* UTIL APIS */
void awb_util_aec_history_update(stats_proc_t *sproc, awb_t *ctrl);
int awb_util_history_find_last_pos(awb_t *awb);
/* SELF CALIBRATION APIS */
void awb_self_cal_init(stats_proc_t *sproc, awb_t *awb);
void awb_self_cal_data_init(stats_proc_t *sproc, awb_t *awb);
void awb_self_cal_update(stats_proc_t *sproc, awb_t *awb,
  float rg_ratio, float bg_ratio);
/* ADVANCED GREY WORLD APIS */
awb_gain_t awb_agw_algo(stats_proc_t *sproc, awb_t *awb);

void awb_util_convert_to_grid(awb_t *ctrl, int rg_ratio_x,
  int bg_ratio_x, int *rg_grid, int *bg_grid);
void awb_settings_init(stats_proc_t *sproc, awb_t *awb);
void awb_load_chromatix(stats_proc_t *sproc, awb_t *awb);
int awb_set_current_wb(stats_proc_t *sproc, awb_t *awb, uint32_t new_wb);
void awb_restore_pre_led_settings(stats_proc_t *sproc, awb_t *awb);
int awb_advanced_grey_world_algo_execute(stats_proc_t *sproc, awb_t *awb);
void awb_algo_snapshot(stats_proc_t *sproc, awb_t *awb);
int awb_set_bestshot_mode(stats_proc_t *sproc, awb_t *awb,
  camera_bestshot_mode_type new_mode);
#endif /* __AWB_H__ */
