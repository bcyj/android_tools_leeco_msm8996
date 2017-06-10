/*==========================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

==========================================================*/
#ifndef __AF_ALGO_TUNING_H__
#define __AF_ALGO_TUNING_H__

#include <media/msm_cam_sensor.h>

typedef int boolean;


#ifdef AF_2X13_FILTER_SUPPORT
#ifndef MAX_HPF_COEFF
#define MAX_HPF_COEFF              26
#endif
#else
#ifndef MAX_HPF_COEFF
#define MAX_HPF_COEFF              10
#endif
#endif



/** af_algo_type: Type of algorithm currently supported
**/
typedef enum {
  AF_PROCESS_DEFAULT   = -2,
  AF_PROCESS_UNCHANGED = -1,
  AF_EXHAUSTIVE_SEARCH = 0,
  AF_EXHAUSTIVE_FAST,
  AF_HILL_CLIMBING_CONSERVATIVE,
  AF_HILL_CLIMBING_DEFAULT,
  AF_HILL_CLIMBING_AGGRESSIVE,
  AF_FULL_SWEEP,
  AF_SLOPE_PREDICTIVE_SEARCH,
  AF_CONTINUOUS_SEARCH,
  AF_PROCESS_MAX
} af_algo_type;

/** single_index_t: Enum for indexing mapping for distance to
 *  lens position
**/
typedef enum _single_index_t {
  SINGLE_NEAR_LIMIT_IDX    = 0,
  SINGLE_7CM_IDX           = 1,
  SINGLE_10CM_IDX          = 2,
  SINGLE_14CM_IDX          = 3,
  SINGLE_20CM_IDX          = 4,
  SINGLE_30CM_IDX          = 5,
  SINGLE_40CM_IDX          = 6,
  SINGLE_50CM_IDX          = 7,
  SINGLE_60CM_IDX          = 8,
  SINGLE_120CM_IDX         = 9,
  SINGLE_HYP_F_IDX         = 10,
  SINGLE_INF_LIMIT_IDX     = 11,
  SINGLE_MAX_IDX           = 12,
}single_index_t; //TODO : Chage to Enum Capital

/** _ACTUATOR_TYPE: Enum for Type of actuator, which impacts core algo behaviors
**/
typedef enum _ACTUATOR_TYPE {
  ACT_TYPE_CLOSELOOP        = 0,
  ACT_TYPE_OPENLOOP         = 1,
  ACT_TYPE_MAX              = 2,
}_ACTUATOR_TYPE;

/** _step_size_t: Scan Step size for Single AF

 *  @rgn_0: step size for far end to rgn 0 boundary

 *  @rgn_1: step size for rgn 0 to rgn 1 boundary

 *  @rgn_2: step size for rgn 1 to rgn 2 boundary

 *  @rgn_3: step size for rgn 2 to rgn 3 boundary

  * @rgn_4: step size for rgn 3 to near end boundary

 **/
typedef struct _step_size_t {
  unsigned short rgn_0; /* reserved */
  unsigned short rgn_1;
  unsigned short rgn_2;
  unsigned short rgn_3;
  unsigned short rgn_4; /* reserved */
}step_size_t;

/** _step_size_table_t:

 *  @Prescan_normal_light: Pre scan step size for normal light

 *  @Prescan_low_light: Pre scan step size for low light

 *  @Finescan_normal_light: Fine scan step size for normal light

 *  @Finescan_low_light: Fine scan step size for low light

 **/
typedef struct _step_size_table_t {
  step_size_t    Prescan_normal_light;
  step_size_t    Prescan_low_light;
  step_size_t    Finescan_normal_light;
  step_size_t    Finescan_low_light;
}step_size_table_t;


/** _BV_threshold_t:

 *    @thres: float threshold value for different BV Lux level

 **/
typedef struct _BV_threshold_t {
  float thres[8]; /* CUR_BV_INFO */ /* 0, 20, 50, 100, 400, 700, OUTDOOR_, Sky */
}BV_threshold_t;

/** _single_threshold_t:

 *  @flat_inc_thres: Threshold for detect flat trend in INCREASE

 *  @flat_dec_thres: Threshold for detect flat trend in DECREASE

 *  @macro_thres: Threshold for detect Macro Peak

    @drop_thres: Threshold for large drop in invalid trend

 *  @hist_dec_dec_thres: Threshold for Histogram value to as a
               interrupt for decision

 *  @hist_inc_dec_thres: Threshold for Histogram value to as a
               interrupt for decision

 *  @dec_dec_3frame: Threshold in 3 frame decrease decrease peak

 *  @inc_dec_3frame: Threshold in 3 frame increase decrease peak

 *  @dec_dec: Threshold in decrease decrease peak

    @dec_dec_noise: Threshold in decrease decrease peak with
                  noise present

 *  @inc_dec: Threshold in increase decrease peak

 *  @inc_dec_noise: Threshold in increase decrease peak with
                  noise present

 **/
typedef struct _single_threshold_t {
  float flat_inc_thres;
  float flat_dec_thres;
  float macro_thres;
  float drop_thres;
  uint32_t hist_dec_dec_thres;
  uint32_t hist_inc_dec_thres;
  BV_threshold_t dec_dec_3frame;
  BV_threshold_t inc_dec_3frame;
  BV_threshold_t dec_dec;
  BV_threshold_t dec_dec_noise;
  BV_threshold_t inc_dec;
  BV_threshold_t inc_dec_noise;
  BV_threshold_t flat_threshold;
}single_threshold_t;

/** _single_optic_t: Contain all optic boundary and setup
    PLEASE USE THIS TYPE IN HEADER: single_index_t

 *  @CAF_far_end: far end in search range for Single AF in CAF

 *  @CAF_near_end: near end in search range for Single AF in CAF

 *  @TAF_far_end: far end in search range for Single AF in TAF

 *  @TAF_near_end: near end in search range for Single AF in TAF

 *  @srch_rgn_1: Region Boundary for Step table near far end

 *  @srch_rgn_2: Region Boundary for Step table in linear range

 *  @srch_rgn_3: Region Boundary for Step table in macro range

 *  @fine_srch_rgn: Region Boundary to enable Finesearch

 *  @far_zone: Region Boundary to clip to for search start
             position. if curposition is beyond this zone,
             starting position will start at the boundary

 *  @near_zone: Region Boundary to clip to for search start
             position. if curposition is before this zone,
             starting position will start at the boundary

 *  @mid_zone: Region Boundary to decide start direction when
             current position is neither in far or near zone

 *  @init_pos: initial position for first search
 *  @far_start_pos: search start position when lens is at far
                  zone.
 *  @near_start_pos: search start position when
              lens is at near zone
 **/
typedef struct _single_optic_t {
  unsigned short CAF_far_end;
  unsigned short CAF_near_end;
  unsigned short TAF_far_end;
  unsigned short TAF_near_end;
  unsigned short srch_rgn_1;
  unsigned short srch_rgn_2;
  unsigned short srch_rgn_3;
  unsigned short fine_srch_rgn;
  unsigned short far_zone;
  unsigned short near_zone;
  unsigned short mid_zone;
  unsigned short far_start_pos;
  unsigned short near_start_pos;
  unsigned short init_pos;
}single_optic_t;

/** _af_tuning_single_t: AF tuning parameters specific to Single
 *  AF algorithm

 *  @index: lens position index mapping from physical distance.

 *  @actuator_type: Type of Actuator used

 *  @is_hys_comp_needed: flag for need for hysteresis

 *  @step_index_per_um: number of step index for 1 micro-meter

 *  @TAF_step_table: Scan step table for TAF usecase

 *  @CAF_step_table: Scan step table for CAF usecase

 *  @PAAF_enable: Flag for enabling PAAF in Cam/Video

 *  @sw: Set of threshold for Software Stats

 *  @hw: Set of threshold for hardware Stats

 *  @BV_gain: Gain Table to map to BV Lux Level(Should not
 *          change)

 *  @optics: Contain All optic boundary and setup region and
 *         boundary

 **/
typedef struct _af_tuning_single_t {
  unsigned short       index[SINGLE_MAX_IDX]; /* single_index_t */
  unsigned short       actuator_type;         /* ACTUATOR_TYPE */
  unsigned short       is_hys_comp_needed;
  unsigned short       step_index_per_um;
  step_size_table_t    CAF_step_table;
  step_size_table_t    TAF_step_table;
  unsigned short       PAAF_enable;
  single_threshold_t   sw;
  single_threshold_t   hw;
  float                BV_gain[8];            /* CUR_BV_INFO */
  single_optic_t       optics;
}af_tuning_single_t;

/** _af_tuning_sp: AF tuning parameters specific to slope
*   predictive algorithm.

*    @fv_curve_flat_threshold: threshold to determine if FV
*                               curve is flat (def: 0.9)
*
*    @slope_threshold1: sp thresholds 1 (def: 0.9)

*    @slope_threshold2: sp thresholds 2 (def: 1.1)
*
*    @slope_threshold3: sp thresholds 3 (def: 0.5)
*
*    @slope_threshold4: sp thresholds 4 (def: 3)
*
*    @lens_pos_0: Lens poisiton when the object is at 3m
*
*    @lens_pos_1: Lens poisiton when the object is at 70cm
*
*    @lens_pos_2: Lens poisiton when the object is at 30cm
*
*    @lens_pos_3: Lens poisiton when the object is at 20cm
*
*    @lens_pos_4: Lens poisiton when the object is at 10cm
*
*    @lens_pos_5: Lens poisiton Macro
*
*    @base_frame_delay: sp frame delay (def: 1)
*
*    @downhill_allowance: max number of consecutive
*                             downhill in the first 4 or 6
*                             samples
*
*    @downhill_allowance_1: max number of consecutive
*                            downhill in 2 or 3 round

**/
typedef struct _af_tuning_sp {
  float fv_curve_flat_threshold;
  float slope_threshold1;
  float slope_threshold2;
  float slope_threshold3;
  float slope_threshold4;
  unsigned int lens_pos_0;
  unsigned int lens_pos_1;
  unsigned int lens_pos_2;
  unsigned int lens_pos_3;
  unsigned int lens_pos_4;
  unsigned int lens_pos_5;
  unsigned int base_frame_delay;
  int downhill_allowance;
  int downhill_allowance_1;
} af_tuning_sp_t;

/** _af_tuning_gyro: AF tuning parameters specific to gyro
 *  support.
 *
 *  @enable: enable/disable gyro assisted caf
 *
 *  @min_movement_threshold:
 *    above this device is supposed to be moving.
 *
 *  @stable_detected_threshold:
 *    less than this, device is stable.
 *
 *  @unstable_count_th:
 *    Number of frames gyro metric should be ablve minimum
 *    movement threshold to indicate motion.
 *
 *  @stable_count_th:
 *    Number of frames we need to be stable after panning.
 *
 *  @fast_pan_threshold:
 *    Threshold to be consider as fast panning, comparing to
 *    gyro_data->sqr
 *
 *  @slow_pan_threshold:
 *    Threshold to be consider as flowpanning, comparing to
 *    gyro_data->sqr
 *
 *  @fast_pan_count_threshold:
 *    Threshold of fast panning cnt to trigger refocusing
 *
 *  @sum_return_to_orig_pos_threshold:
 *    apart from fast panning count, if the gyro data sum is
 *    pass the threshold, it will trigger refocus
 *
 *  @stable_count_delay:
 *    Number of stable frame from gyro to be consider stable
 *    long enough
 *
 **/
typedef struct _af_tuning_gyro {
  boolean               enable;
  float                 min_movement_threshold;
  float                 stable_detected_threshold;
  unsigned short        unstable_count_th;
  unsigned short        stable_count_th;
  float                 fast_pan_threshold;
  float                 slow_pan_threshold;
  unsigned short        fast_pan_count_threshold;
  unsigned short        sum_return_to_orig_pos_threshold;
  unsigned short        stable_count_delay;
} af_tuning_gyro_t;

/** _af_tuning_sad: AF tuning parameters specific to Sum of
 *  Absolute Difference (SAD) scene detection.
 *
 *  @enable: enable/disable sad scene detection
 *
 *  @gain_min: minimum gain
 *
 *  @gain_max: maximum gain
 *
 *  @ref_gain_min: minimum referece gain
 *
 *  @ref_gain_max: maximum reference gain
 *
 *  @threshold_min: threshold when current gain is less than
 *                minimum gain
 *
 *  @threshold_max: threshold when current gain is more than
 *                maximum gain.
 *
 *  @ref_threshold_min: threshold when current gain is less than
 *                minimum reference gain
 *
 *  @ref_threshold_max: threshold when current gain is more than
 *                maximum reference gain   */
typedef struct _af_tuning_sad {
  boolean enable;
  float gain_min;
  float gain_max;
  float ref_gain_min;
  float ref_gain_max;
  unsigned short threshold_min;
  unsigned short threshold_max;
  unsigned short ref_threshold_min;
  unsigned short ref_threshold_max;
  unsigned short frames_to_wait;
} af_tuning_sad_t;


/** _af_tuning_continuous: AF tuning parameters specific to
 *  Continuous AF
 *
 *  @enable: CAF is enabled/disabled (currently only used by
 *         ez-tune)
 *
 *  @scene_change_detection_ratio: fv change to trigger a
 *                                  target change
 *
 *  @panning_stable_fv_change_trigger: fv change vs past
 *                                      frame fv to trigger to
 *                                      determine if scene is
 *                                      stable
 *
 *  @panning_stable_fvavg_to_fv_change_trigger: fv change vs
 *                                               average of 10
 *                                               past frame's FV
 *
 *  @panning_unstable_trigger_cnt: how many panning unstabl
 *                                  detections before triggering
 *                                  a scene change.
 *
 *  @downhill_allowance: number of extra steps to search once
 *                     peak FV is found
 *
 *  @uphill_allowance: number of steps to move if FV keeps
 *                   increasing.
 *
 *  @base_frame_delay: number of frames to skip after lens
 *                   movement.
 *
 *  @scene_change_luma_threshold: threshold above which a change
 *                             in AF roi lux will trigger new
 *                             search.
 *
 *  @luma_settled_threshold: AF calculates AEC settled condition as follows
 *                      if abs(Prev AF Luma - Current AF Luma) < luma_settled_threshold
 *                      then CAF can begin focus search without waiting for AEC to
 *                      completely settle
 *
 *
 *  @threshold_in_noise: determine if the fv variaion is due to
 *                     noise.
 *
 *  @search_step_size: size of each steps
 *
 *  @init_search_type: search algorithm to use before entering
 *                   monitor mode.
 *
 *  @search_type: search algorithm to use after monitor mode
 *              decides new search is required.
 *
 *  @ low_light_wait: how many frames to skip when in low light
 *
 *  @max_indecision_cnt: maximum number of times allowed to stay
 *                     in make_decision if it's not clear which
 *                     way to go.
 *
 *  @af_sp: af parameters for slope-predictive algorithm
 *
 *  @af_gyro: gyro parameters to assist AF
 **/
typedef struct _af_tuning_continuous {
  boolean enable;
  unsigned char   scene_change_detection_ratio;
  float           panning_stable_fv_change_trigger;
  float           panning_stable_fvavg_to_fv_change_trigger;
  unsigned short  panning_unstable_trigger_cnt;
  unsigned short  panning_stable_trigger_cnt;
  unsigned long   downhill_allowance;
  unsigned short  uphill_allowance;
  unsigned short  base_frame_delay;
  unsigned short  scene_change_luma_threshold;
  unsigned short  luma_settled_threshold;
  float           noise_level_th;
  unsigned short  search_step_size;
  unsigned short  init_search_type;
  unsigned short  search_type;
  unsigned short  low_light_wait;
  unsigned short  max_indecision_cnt;
  float           flat_fv_confidence_level;
  af_tuning_sad_t af_sad;
  af_tuning_gyro_t af_gyro;
}af_tuning_continuous_t;


/** _af_tuning_exhaustive: AF tuning parameters specific to
 *  exhaustive search AF
 *
 *  @num_gross_steps_between_stat_points:
 *  Used to control how rough initial AF search is.
 *
 *  @num_fine_steps_between_stat_points:
 *  control how precise fine search is
 *
 *  @num_fine_search_points: how many search points to gather in
 *                         fine search.
 *
 *  @downhill_allowance: number of extra steps to search once
 *                     peak FV is found
 *
 *  @uphill_allowance: number of steps to move if FV keeps
 *                   increasing.
 *
 *  @base_frame_delay: how many frames to delay after each lens movement
 *
 *  @coarse_frame_delay: how many frames to delay after lens movement in
 *                       coarse search
 *
 *  @fine_frame_delay: how many frames to delay after lens movement in
 *                     fine search
 *
 *  @coarse_to_fine_frame_delay: how many frames to delay after lens reaching
 *                               the end position for the coarse search and
 *                               before starting the fine search
 *
 *  @enable_multiwindow:
 *    Enable Flag for using Multi window or Single window AF
 *    stats.
 *  @mw_enable_gain_thresh:
 *    AEC gain threshold to trigger multiwindow AF
 *
 **/
typedef struct _af_tuning_exhaustive {
  unsigned short num_gross_steps_between_stat_points;
  unsigned short num_fine_steps_between_stat_points;
  unsigned short num_fine_search_points;
  unsigned short downhill_allowance;
  unsigned short uphill_allowance;
  unsigned short base_frame_delay;
  unsigned short coarse_frame_delay;
  unsigned short fine_frame_delay;
  unsigned short coarse_to_fine_frame_delay;
  float          noise_level_th;
  float          flat_fv_confidence_level;
  float          climb_ratio_th;
  int            low_light_luma_th;
  int            enable_multiwindow;
  float          gain_thresh;
}af_tuning_exhaustive_t;

/** _af_tuning_fullsweep_t: AF tuning parameters specific to
 *  full sweep search AF
 *
 *  @num_steps_between_stat_points:
 *  Used to control how many steps to move the lens at a time during search.
 *
 *  @frame_delay_inf: how many frames to delay after resetting the lens in
 *                    initial position.
 *
 *  @frame_delay_norm: how many frames to delay after each lens movement
 *
 *  @frame_delay_final: how many frames to delay after reaching max position
 **/
typedef struct _af_tuning_fullsweep_t {
  unsigned short num_steps_between_stat_points;
  unsigned short frame_delay_inf;
  unsigned short frame_delay_norm;
  unsigned short frame_delay_final;
}af_tuning_fullsweep_t;

/** _af_shake_resistant: AF tuning parameters specific to af
 *  shake resistant.
 *
 * @enable: true if enabled
 *
 * @max_gain:
 *
 * @min_frame_luma:
 *
 * @tradeoff_ratio:
 *
 * toggle_frame_skip:
 **/
typedef struct _af_shake_resistant {
  boolean enable;
  float max_gain;
  unsigned char min_frame_luma;
  float tradeoff_ratio;
  unsigned char toggle_frame_skip;
}af_shake_resistant_t;


/** _af_motion_sensor: AF tuning parameters specific to af
 *  motion sensor. Controls how each component affect af scene
 *  change detection. Bigger the value less sensitive AF
 *  response is.
 *
 *  @af_gyro_trigger: control how sensitive AF is to gyro
 *                  trigger.
 *
 *  @af_accelerometer_trigger: controls how sensitive AF is to
 *                           accelerometer.
 *
 *  @af_magnetometer_trigger: controls how sensistive AF is to
 *                          magnetometer.
 *
 *  @af_dis_motion_vector_trigger: controls how sensitive AF is
 *                               to DIS motion vector.
 **/
typedef struct _af_motion_sensor {
  float af_gyro_trigger;
  float af_accelerometer_trigger;
  float af_magnetometer_trigger;
  float af_dis_motion_vector_trigger;
}af_motion_sensor_t;

/** _af_fd_priority_caf: AF tuning parameters specific to
 *  face-detection priority af.
 *
 *  @pos_change_th: controls when to reconfigure ROI when
 *  position has changed with respect to last stable ROI.
 *
 *  @pos_stable_th_hi: percentage differnce between last and
 *  current position above this indicate not stable.
 *
 *  @pos_stable_th_low: position is deemed stable only after
 *  difference is less than this threshold.
 *
 *  @size_change_th: threshold to check if size change has
 *  decreased enough to deem size stable enough to focus.
 *
 *  @old_new_size_diff_th: percentage difference between
 *  last biggest face and current biggest face.
 *
 *  @stable_count_size: number of frames face size should remain
 *  stable to trigger new search.
 *
 *  @stable_count_pos: number of frames face position should
 *                   remain stable.
 *
 *  @no_face_wait_th: number of frames to wait to reset ROI to
 *  default once face disappears.
 *
 *  @fps_adjustment_th: if current fps falls below this
 *                    threshold we'll adjust stability counts.
 *  */
typedef struct _af_fd_priority_caf {
  float pos_change_th;
  float pos_stable_th_hi;
  float pos_stable_th_low;
  float size_change_th;
  float old_new_size_diff_th;
  int stable_count_size;
  int stable_count_pos;
  int no_face_wait_th;
  int fps_adjustment_th;
} af_fd_priority_caf_t;


/** _af_tuning_algo: AF tuning parameters specific to AF
 *  algorithm.
 *
 *  @af_process_type: af algorithm type used
 *
 *  @position_near_end: nearest position lens can move to
 *
 *  @position_default_in_macro: default lens rest position when
 *                            focus mode is Macro.
 *
 *  @position_boundary: determines near end of search range for
 *                    Normal focus mode.
 *
 *  @position_default_in_normal: default lens reset position
 *                             when focus mode is other than
 *                             macro.
 *
 *  @position_far_end: farthest point of the search range
 *
 *  @position_normal_hyperfocal: normal position of the lens when
 *                               focus fails
 *
 *  @undershoot_protect: when enabled, lens will be moved more
 *                     in either forward or backward direction.
 *  @undershoot_adjust: when undershoot_protect is enabled, lens
 *                    movement is adjusted by this amount.
 *
 *  @fv_drop_allowance: amount by which fv is allowed to drop
 *                    below max
 *
 *  @lef_af_assist_enable: enable/disable led assisted AF
 *
 *  @led_af_assist_trigger_idx: Lux Index\A0at which LED assist
 *                            for autofocus is enabled.
 *
 *  @af_tuning_continuous_t: af parameters for continuous focus
 *
 *  @af_exh: af parameters for exhaustive af
 *
 *  @af_sad: sad related tuning parameters
 *
 *  @af_shake_resistant: tuning parameters for af shake
 *                     resistant.
 *
 *  @af_motion_sensor: trigger parameters for af motion sensor.
 **/
typedef struct _af_tuning_algo {
  unsigned short af_process_type;
  unsigned short position_near_end;
  unsigned short position_default_in_macro;
  unsigned short position_boundary;
  unsigned short position_default_in_normal;
  unsigned short position_far_end;
  unsigned short position_normal_hyperfocal;
  unsigned short position_macro_rgn;
  unsigned short undershoot_protect;
  unsigned short undershoot_adjust;
  float min_max_ratio_th;
  int lef_af_assist_enable;
  long led_af_assist_trigger_idx;
  int lens_reset_frame_skip_cnt;
  float low_light_gain_th;
  float base_delay_adj_th;
  af_tuning_continuous_t af_cont;
  af_tuning_exhaustive_t af_exh;
  af_tuning_fullsweep_t af_full;
  af_tuning_sp_t af_sp;
  af_tuning_single_t af_single;
  af_shake_resistant_t af_shake_resistant;
  af_motion_sensor_t af_motion_sensor;
  af_fd_priority_caf_t fd_prio;
}af_tuning_algo_t;

/** _af_vfe_config: vfe configuration info
**/
typedef struct _af_vfe_config {
  unsigned short fv_min;
  unsigned short max_h_num;
  unsigned short max_v_num;
  unsigned short max_block_width;
  unsigned short max_block_height;
  unsigned short min_block_width;
  unsigned short min_block_height;
  float h_offset_ratio_normal_light;
  float v_offset_ratio_normal_light;
  float h_clip_ratio_normal_light;
  float v_clip_ratio_normal_light;
  float h_offset_ratio_low_light;
  float v_offset_ratio_low_light;
  float h_clip_ratio_low_light;
  float v_clip_ratio_low_light;
  float touch_scaling_factor_normal_light;
  float touch_scaling_factor_low_light;
} af_vfe_config_t;

/** _af_vfe_legacy_hpf: high pass filter coefficients for
 *  legacy YUV stats
 **/
typedef struct _af_vfe_legacy_hpf {
  char      a00;
  char      a02;
  char      a04;
  char      a20;
  char      a21;
  char      a22;
  char      a23;
  char      a24;
} af_vfe_legacy_hpf_t;

/** _af_vfe_bayer_hpf: high pass filter coefficients for
 *  bayer stats
 **/
typedef struct _af_vfe_bayer_hpf {
  int      a00;
  int      a01;
  int      a02;
  int      a03;
  int      a04;
  int      a10;
  int      a11;
  int      a12;
  int      a13;
  int      a14;
} af_vfe_bayer_hpf_t;

/** _af_vfe_hpf: high pass filter coefficients.
 *  @af_hpf: hpf for yuv
 *  @bf_hpf: hpf for bayer
 **/
typedef struct _af_vfe_hpf {
  af_vfe_legacy_hpf_t af_hpf;
  int bf_hpf_2x5[MAX_HPF_COEFF];
#ifdef AF_2X13_FILTER_SUPPORT
  int bf_hpf_2x13[MAX_HPF_COEFF];
#endif
} af_vfe_hpf_t;

#define FILTER_SW_LENGTH_FIR 11
#define FILTER_SW_LENGTH_IIR 6

/** af_vfe_sw_fir_hpf_t: FIR high pass filter coefficients for
 *  software stats
 **/
typedef struct _af_vfe_sw_fir_hpf_t {
  int                    a[FILTER_SW_LENGTH_FIR];
  double                 fv_min_hi;
  double                 fv_min_lo;
  uint32_t               coeff_length;
} af_vfe_sw_fir_hpf_t;

/** af_vfe_sw_iir_hpf_t: IIR high pass filter coefficients for
 *  software stats
 **/
typedef struct _af_vfe_sw_iir_hpf_t {
  double                 a[FILTER_SW_LENGTH_IIR];
  double                 b[FILTER_SW_LENGTH_IIR];
  double                 fv_min_hi;
  double                 fv_min_lo;
  uint32_t               coeff_length;
} af_vfe_sw_iir_hpf_t;

/** af_vfe_sw_hpf_t: high pass sw filter coefficients.
 *  @filter_type: filter type af_sw_filter_type (1/2)
 *  @fir: fir hpf coeff for sw stats
 *  @iir: iir hpf coeff for sw stats
 *  @fir_low_end: fir hpf coeff for sw stats low end target
 *  @iir_low_end: iir hpf coeff for sw stats low end target
 *  @filter_type : Specify type of filter used
 **/
typedef struct af_vfe_sw_hpf_t {
  unsigned short         filter_type;    /* af_sw_filter_type */
  af_vfe_sw_fir_hpf_t    fir;
  af_vfe_sw_iir_hpf_t    iir;
  af_vfe_sw_fir_hpf_t    fir_low_end;
  af_vfe_sw_iir_hpf_t    iir_low_end;
} af_vfe_sw_hpf_t;

/** _af_tuning_vfe: tuning parameters for AF stats.
 *  @af_config: vfe configuration info
 *
 *  @af_fv_metric: fv metric (0: sum of FV  1: max of FV)
 *
 *  @af_hpf: high pass filter coefficients
 *
 *  @af_vfe_sw_hpf_t: Filter coefficeint for SW stats
 *
 *  @sw_fv_min_lux_trig_hi: Lux trigger to use normal FV_min
 *
 *  @sw_fv_min_lux_trig_lo: Lux trigger to use lowlight FV_min
 *
 **/
typedef struct _af_tuning_vfe {
  unsigned short   fv_metric;
  af_vfe_config_t  config;
  af_vfe_hpf_t     hpf_default;
  af_vfe_hpf_t     hpf_face;
  af_vfe_hpf_t     hpf_low_light;
  af_vfe_sw_hpf_t  sw_hpf_default;
  af_vfe_sw_hpf_t  sw_hpf_face;
  af_vfe_sw_hpf_t  sw_hpf_lowlight;
  float            sw_fv_min_lux_trig_hi;
  float            sw_fv_min_lux_trig_lo;
} af_tuning_vfe_t;

/** _af_header_info:
 **/
typedef struct _af_header_info {
  uint16_t header_version;
  char module_name[MAX_ACT_MOD_NAME_SIZE];
  char actuator_name[MAX_ACT_NAME_SIZE];
  enum af_camera_name cam_name;
}af_header_info_t;

/** _af_tune_parms: Main tuning parameters exposed to outer
 *  world.
 * @af_header_info: version information
 *
 * @af_header_info: version information
 *
 * @af_algo: AF parameters specific to AF algorithm
 *
 * @af_vfe: AF parameters specific to VFE stats configuration
 *
 * @actuator_params: parameters specific to actuator
 *
 * @actuator_tuned_params:
**/
typedef struct _af_algo_tune_parms {
  af_header_info_t af_header_info;
  af_tuning_algo_t af_algo;
  af_tuning_vfe_t af_vfe;

} af_algo_tune_parms_t;

/**
 * af_algo_ctrl:
 * @af_algo_params: af algorithm params
 **/
typedef struct af_algo_ctrl {
  af_algo_tune_parms_t af_algo_params;
} af_algo_ctrl_t;

#endif /* __AF_ALGO_TUNING_H__ */
