/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/


#ifndef __FD_CHROMATIX_H__
#define __FD_CHROMATIX_H__


/**
 *   face detection modes
 **/
#define FD_CHROMATIX_MODE_MOTION_DEFAULT 1
#define FD_CHROMATIX_MODE_MOTION_FULL 1
#define FD_CHROMATIX_MODE_MOTION_PARTITION 2
#define FD_CHROMATIX_MODE_MOTION_PROGRESS 3

/** fd_chromatix_angle_t
 *   FD_FACE_ADJ_FIXED: Use fixed minface size
 *   FD_FACE_ADJ_FLOATING: apply the ratio for the face size
 *
 *   face angles for detection
 **/
typedef enum {
  FD_FACE_ADJ_FIXED,
  FD_FACE_ADJ_FLOATING,
} fd_face_adj_type_t;

/** fd_chromatix_angle_t
 *   FD_ANGLE_NONE: disable angle detection
 *   FD_ANGLE_15_ALL: 15 degree detection in all angles
 *   FD_ANGLE_45_ALL: 45 degree detection in all angles
 *   FD_ANGLE_ALL: detection in all angles
 *
 *   face angles for detection
 **/
typedef enum {
  FD_ANGLE_NONE,
  FD_ANGLE_15_ALL,
  FD_ANGLE_45_ALL,
  FD_ANGLE_ALL,
  FD_ANGLE_MANUAL,
} fd_chromatix_angle_t;

/** fd_chromatix_ct_detection_mode_t
 *   FD_CONTOUR_MODE_DEFAULT: All counters are detected
 *   FD_CONTOUR_MODE_EYE: Only eyes are detected
 *
 *   Contour detection mode structure
 **/
typedef enum {
  FD_CONTOUR_MODE_DEFAULT = 0,
  FD_CONTOUR_MODE_EYE = 1,
} fd_chromatix_ct_detection_mode_t;

/** fd_face_stab_filter_t
 *   FD_STAB_NO_FILTER: Without stabilization filter.
 *   FD_STAB_TEMPORAL: Temporal filter.
 *   FD_STAB_HYSTERESIS: Hysteresis.
 *
 *   Face stabilization filter type.
 **/
typedef enum {
  FD_STAB_NO_FILTER,
  FD_STAB_TEMPORAL,
  FD_STAB_HYSTERESIS,
} fd_face_stab_filter_t;

/** fd_chromatix_angle_t
 *   FD_STAB_EQUAL: Values will be marked as stable when two consecutive
 *     values are equal.
 *   FD_STAB_BIGGER: Values will be marked as stable if new values are
 *     bigger then old ones.
 *   FD_STAB_SMALLER: Values will be marked as stable if new values are
 *     smaller then old ones:
 *   FD_STAB_CLOSER_TO_REFERENCE:  Values will be marked as stable when
 *     the distance to reference is smaller.
 *   FD_STAB_CONTINUES_BIGGER: The same as FD_STAB_BIGGER but it works in continues
 *     mode, if values are stable and new bigger values are received.
 *     they will be updated.
 *   FD_STAB_CONTINUES_SMALLER: The same as FD_STAB_SMALLER but it works in continues
 *     mode, if values are stable and new smaller values are received.
 *     they will be updated.
 *   FD_STAB_CONTINUES_CLOSER_TO_REFERENCE: The same as FD_STAB_BIGGER but it
 *     works in continues mode, if values are stable and new closer to reference
 *     values are received they will be updated.
 *
 *   Face stabilization mode
 **/
typedef enum {
  FD_STAB_EQUAL,
  FD_STAB_SMALLER,
  FD_STAB_BIGGER,
  FD_STAB_CLOSER_TO_REFERENCE,
  FD_STAB_CONTINUES_SMALLER,
  FD_STAB_CONTINUES_BIGGER,
  FD_STAB_CONTINUES_CLOSER_TO_REFERENCE,
} fd_face_stab_mode_t;


/** fd_face_stab_params_t
 *   @enable: Enable stabilization.
 *   @mode: Stabilization mode.
 *   @threshold: Stabilization threshold (Within threshold new values will not
 *     be accepted).
 *   @state_cnt: Number of consecutive frames to wait
 *     for entry to became stable.
 *   @use_reference: Stabilize entry by reference
 *       (Current reference are eyes).
 *   @filter_type: Filter type to be used for stabilization
 *     @temp: Temporal filter:
 *       @num: Strength numerator.
 *       @denom: Strength denominator.
 *     @hyst: Hysteresis - Two square zones available: Zone A and Zone B.
 *       Requirement - zone A < zone B. Fields:
 *       @start_A: A Start point.
 *       @end_A: A end point.
 *       @start_B: B Start point.
 *       @end_B: B end point.
 *
 *   Structure which holds face stabilization tuning parameters.
 **/
typedef struct {
  uint32_t enable;
  fd_face_stab_mode_t mode;
  uint32_t threshold;
  uint32_t state_cnt;
  uint32_t use_reference;
  fd_face_stab_filter_t filter_type;
  union {
    struct {
      uint32_t num;
      uint32_t denom;
    } temp;
    struct {
      uint32_t start_A;
      uint32_t end_A;
      uint32_t start_B;
      uint32_t end_B;
      } hyst;
  };
} fd_face_stab_params_t;

/** fd_chromatix_t
 *   @enable: flag to enable face detection
 *   @min_face_adj_type: minimum face size adjustment type
 *   @min_face_size: minimum face size
 *   @min_face_size_ratio: ratio of minimum face size w.r.t
 *                       image size
 *   @max_face_size: maximum face size
 *   &max_num_face_to_detect: maximum number of faces to detect
 *   @angle_front: angles to detect from front pose
 *   @angle_front_bitmask: bitmask for manual angles
 *   @angle_half_profile: angles to detect from front pose
 *   @angle_half_profile_bitmask: bitmask for manual angles
 *   @angle_full_profile: angles to detect from front pose
 *   @angle_full_profile_bitmask: bitmask for manual angles
 *   @detection_mode: detection mode
 *   @frame_skip: number of frames to skip
 *   @enable_smile_detection: flag to enable smile detection
 *   @enable_blink_detection: flag to enable blink detection
 *   @enable_gaze_detection: flag to enable gaze detection
 *   @search_density_nontracking: search density for face proc
 *                  algorithm for non-tracking mode 20-40.
 *   @search_density_tracking: search density for face proc
 *                  algorithm for tracking mode 20-40.
 *   @direction: enable/disable face tracking direction
 *   @refresh_count: interval count after which fresh face
 *                 search is done
 *   @threshold: detection threshold
 *   @face_retry_count: Face retry count (when face is lost during motion)
 *   @head_retry_count: Head retry count (when head is lost during motion)
 *   @hold_count: Hold count (output previous face when face is lost)
 *   @lock_faces: lock Faces Enable
 *               (Faces are locked not allow other faces to replace them)
 *   @move_rate_threshold: Position Modify rate threshold
 *                (in percents) how smoothly face changes coordinates
 *   @stab_enable: enable face stabilization
 *   @stab_history: Stabilization history depth (min is 2)
 *   @stab_pos: Face position stabilization parameters.
 *   @stab_size: Face size stabilization parameters.
 *   @stab_mouth: Mouth position stabilization parameters.
 *   @stab_smile: Smile degree stabilization parameters.
 *
 *
 *   faceproc chromatix header
 **/
typedef struct {
  int8_t enable;
  fd_face_adj_type_t min_face_adj_type;
  uint32_t min_face_size;
  float min_face_size_ratio;
  uint32_t max_face_size;
  uint32_t max_num_face_to_detect;
  fd_chromatix_angle_t angle_front;
  uint16_t angle_front_bitmask;
  fd_chromatix_angle_t angle_half_profile;
  uint16_t angle_half_profile_bitmask;
  fd_chromatix_angle_t angle_full_profile;
  uint16_t angle_full_profile_bitmask;
  uint32_t detection_mode;
  uint32_t frame_skip;
  uint32_t enable_smile_detection;
  uint32_t enable_blink_detection;
  uint32_t enable_gaze_detection;
  uint32_t search_density_nontracking;
  uint32_t search_density_tracking;
  uint32_t direction;
  uint32_t refresh_count;
  uint32_t threshold;
  uint32_t face_retry_count;
  uint32_t head_retry_count;
  uint32_t hold_count;
  uint32_t lock_faces;
  uint32_t move_rate_threshold;
  fd_chromatix_ct_detection_mode_t ct_detection_mode;
  int8_t stab_enable;
  uint32_t stab_history;
  fd_face_stab_params_t stab_pos;
  fd_face_stab_params_t stab_size;
  fd_face_stab_params_t stab_mouth;
  fd_face_stab_params_t stab_smile;
} fd_chromatix_t;


#endif //__FD_CHROMATIX_H__
