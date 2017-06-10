/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#ifndef __AF_H__
#define __AF_H__

#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "camera.h"
#include "camera_dbg.h"

#if(AF_DEBUG_HIGH)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AF"
#endif
#if(AF_DEBUG_LOW)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AF"
  #define CDBG_AF(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_AF(fmt, args...) do{}while(0)
#endif

// #define HILL_CLIMB_ALGO
#define CONTAF_FREQUENCY            4
#define THRESHOLD_IN_NOISE         (0.05)
#define FOCUS_ATTEMPTS            100
#define STATS_BUFFER_MAX_ENTRIES 1056
#define LENS_DONE_MOVE_THRESH     400 /* wait time for lens done msg */

#define MAX_FRAMES_TO_PERFORM_FOCUS (300 + FOCUS_ATTEMPTS + \
  AEC_SETTLE_MAX_FRAME_CNT)

#define CONFIDENCE_LEVEL_EXH    (0.9) /*  DLI added */
#define CONFIDENCE_LEVEL        (0.7) /*  DLI added */
#define REFOCUSING_CHECK_INDEX     4  /*  DLI changed for speed up */
#define MODE_CHANGE_THRESH       300 /* DLI added for switching 2 mode after
                                      * frist two FVs */
typedef enum {
  AF_INACTIVE = 0,              /* Default focused state, at the edge of infinity */
  AF_START,                     /* start state */
  AF_DECISION,                  /* decision making state */
  AF_EXHAUSTIVE,
  AF_GATHER_STATS_COARSE,       /* gather stats with big steps */
  AF_GATHER_STATS_FINE,         /* gather stats with small steps */
  AF_FOCUSING,                  /* found lens focus position */
  AF_UNABLE_TO_FOCUS,           /* unable to focus due to noise */
  AF_ERROR,                     /* error state */
  AF_COLLECT_END_STAT,          /* collect end stat */
  AF_WAIT_FOR_AEC_TO_SETTLE,
  AF_WAIT_FOR_AEC_WITH_LED_TO_SETTLE,
  /*used for CONTINUOUS AF SEARCH*/
  AF_GATHER_STATS_CONT_MONITOR, /* 12 */
  AF_MAKE_DECISION,             /* 13 */
  AF_GATHER_STATS_CONT_SEARCH,  /* 14 */
  AF_MOVING_LENS,               /* 15 */
  /*used for slope predictive algorithm states*/
  AF_SLOPE_PREDICTIVE,
  /*end adding new states here */
  AF_NUM_STATES
} af_state_enum_type;

typedef enum {
  AF_PROCESS_DEFAULT   = -2,
  AF_PROCESS_UNCHANGED = -1,
  AF_EXHAUSTIVE_SEARCH = 0,
  AF_EXHAUSTIVE_FAST,
  AF_HILL_CLIMBING_CONSERVATIVE,
  AF_HILL_CLIMBING_DEFAULT,
  AF_HILL_CLIMBING_AGGRESSIVE,
  AF_FULL_SWEEP,
  AF_PROCESS_MAX
} af_algo_type_t;

typedef enum {
  AF_MOVE_LENS_NEAR,
  AF_MOVE_LENS_FAR
} af_lens_move_t;

typedef struct {
  /* if set TRUE, CAF is initiated irrespective of other conditions */
  int refocus_for_gyro;
  /* device is stable with respect to gyro and can be considered for
     refocus if other conditions are satisfied */
  int panning_stable_for_video;
  /* number of frames more than fast pan threshold */
  int fast_pan_cnt;
  /* number of frames received during slow panning - not used currently */
  int slow_pan_cnt;
  /* numbe of frames device has been stable */
  int no_pan_cnt;
  /* Flag to indicate gyro had been in motion (used in Camera mode) */
  int gyro_moved_flag;
  /* Gyro metric used - sum of square of angular velocity in 3 dimensions */
  float sqr;
  /* sum of angular velocity in x-axis since last focus */
  float x_sum;
  /* sum of angular velocity in x-axis since last focus */
  float y_sum;
  /* sum of angular velocity in x-axis since last focus */
  float z_sum;
} af_cont_gyro_t;

typedef struct {
  int stable_count;
  int frame_count;
  int initialized;
  int set_ref_luma_sum;
  unsigned int ref_luma_sum[256];
  unsigned int prev_luma_sum[256];
} af_cont_sad_t;

typedef struct {
  int frame_cnt;
  int trig_refocus;
  int exp_index_before_change;
  int trial_in_noise1;
  int back_step_cnt;
  int panning_index;
  int panning_track[10]; /* used for panning detection */
  int clean_panning_track;
  int fov_allowed_steps;
  int tryingstep;
  int smaller_tryingstep; /* smaller step in macro range */
  int bigger_tryingstep;  /* bigger step in infinity range */
  int no_of_indecision;
  int baseDelay;
  int cur_luma;
  int max_luma;
  int start_luma;
  int panning_unstable_cnt;
  int status; /* focused/not focused/focusing */
  int locked; /* if locked CAF won't refocus unless unlocked */
  int send_event_later; /* if status FOCUSING send CAF-done later once done*/
  int ignore_full_sweep;
  af_cont_gyro_t gyro_data;
  af_cont_sad_t sad_data;
} af_continuous_t;

typedef struct {
  uint32_t srch_mode;
  float soft_focus_degree;
  camera_bestshot_mode_type   curr_mode;
} af_bestshot_data_t;

typedef struct {
  int move_flag;
  int fine_focal_pos;
} af_exhaustive_algo_t;

typedef struct {
  int   testactive;
  int   fv0_in_noise;
  int   fv1_in_noise;
  int   fv2_in_noise;
  int   steps_btwn_stat_pts_macro_coarse;
  int   steps_btwn_stat_pts_macro_fine;
  int   steps_btwn_stat_pts_normal_fine;
  int   trial_of_macro_fine;
  int   trial_in_noise;
  int   fine_index;
  int   drop_at_first_shift;
} af_hill_climb_algo_t;

typedef struct {
  int branch;
  int stage;
  int total_steps;
  int flat_peak;
  int pos0_step_cnt;
  int pos1_step_cnt;
  int pos2_step_cnt;
  int pos3_step_cnt;
  int pos4_step_cnt;
  int pos5_step_cnt;
  int pos_cnt;
  int slp10;
  int slp21;
  int slp32;
  int fv[10];
  int lens_pos[10];
  int srch_pass_cnt;
  int cycle;
  int move_steps;
  int search_done;
  int max_step;
  int frame_delay;
} af_slope_predictive_algo_t;

typedef struct {
  int disable;
} af_ez_tune_t;

typedef struct {
  int   stats[AF_COLLECTION_POINTS];
  int cur_stat;
  int prev_stat;
  int   index; /*stats array index */
  int metering_mode;
  uint32_t srch_mode;
  int cur_pos;
  int near_end;
  int far_end;
  int infy_pos;
  int default_focus;
  int start_lens_pos;
  int final_lens_pos;
  int elapsed_frame_cnt;
  int frame_skip;
  int frame_delay;
  int lens_state_changed;
  int lens_move_done;
  int in_low_light;
  int lens_moved;
  int step_fail_cnt;
  int collect_end_stat;
  int max_focus_val;
  int min_focus_val;
  int prev_max_focus_val;
  int cur_focus_val;
  int   downhill_allowance;
  int   uphill_threshold;
  int   num_downhill;
  int   num_uphill;
  int max_pos;
  int locn[AF_COLLECTION_POINTS];
  /*static variables in C files */
  int stable_cnt;
  int luma_tabilize_wait;
  float soft_focus_degree;
  float fv_drop_allowance;

  af_state_enum_type previous_state;
  af_continuous_t caf;
  af_algo_type_t algo_type;
  af_state_enum_type state;
  af_bestshot_data_t        bestshot_d;
  af_exhaustive_algo_t exhstive;
  af_hill_climb_algo_t hc;
  af_slope_predictive_algo_t sp;
  af_ez_tune_t eztune;
} af_t;

int af_run_algorithm(stats_proc_t *sproc, af_t *af);
int af_hill_climbing_search(stats_proc_t *sproc, af_t *af);
int af_exhaustive_search(stats_proc_t *sproc, af_t *af);
int af_slope_predictive_srch(stats_proc_t *sproc, af_t *af);

/* UTILITY APIS*/
void af_init_data(stats_proc_t *sproc, af_t *af);
void af_load_chromatix(stats_proc_t *sproc, af_t *af);
int af_stop_focus(stats_proc_t *sproc, af_t *af);
void af_safe_move_lens(stats_proc_t *sproc, af_t *af, int dir, int *steps);
void af_move_lens(stats_proc_t *sproc, af_t *af, int dir, int steps);
int af_continuous_search (stats_proc_t *sproc, af_t *af);
void af_init_hill_climbing_search(stats_proc_t *sproc, af_t *af);
void af_slow_move_lens(stats_proc_t *sproc, af_t *af, int dir, int steps);
int af_panning_stable_check(stats_proc_t *sproc, af_t *af);
int af_cont_gyro_monitor_mode_video(af_cont_gyro_t *gyro_data,
                                    stats_proc_gyro_info_t *gyro_info);
int af_cont_gyro_monitor_mode_camera(af_cont_gyro_t *gyro_data,
                                     stats_proc_gyro_info_t *gyro_info);
int af_get_stats_config_mode(stats_proc_t *sproc);
int af_cont_did_sad_change(stats_proc_t *sproc, af_t *af);

/* SET APIS */
int af_reset_lens(stats_proc_t *sproc, af_t *af);
int af_set_start_parameters(stats_proc_t *sproc, af_t *af);
int af_move_lens_to(stats_proc_t *sproc, af_t *af, int steps);
int af_lens_move_done(stats_proc_t *sproc, af_t *af, int status);
void af_done(stats_proc_t *sproc, af_t *af, camera_af_done_type cb);
int af_set_bestshot_mode(stats_proc_t *sproc, af_t *af,
  camera_bestshot_mode_type new_mode);
int af_set_focus_mode(stats_proc_t *sproc, af_t *af);

/* GET APIS */
focus_distances_info af_get_focus_distance(stats_proc_t *sproc, af_t *af);
#endif /* __AF_H__ */
