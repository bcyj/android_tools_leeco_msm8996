/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

/* Face detection enable */
.enable = 1,
.min_face_adj_type = FD_FACE_ADJ_FLOATING,
.min_face_size = 90,
.min_face_size_ratio = 0.1,
.max_face_size = 1000,
.max_num_face_to_detect = 5,
.angle_front = FD_ANGLE_ALL,
.angle_front_bitmask = 0,
.angle_half_profile = FD_ANGLE_15_ALL,
.angle_half_profile_bitmask = 0,
.angle_full_profile = FD_ANGLE_NONE,
.angle_full_profile_bitmask = 0,
.detection_mode = FD_CHROMATIX_MODE_MOTION_PROGRESS,
.frame_skip = 1,
.enable_smile_detection = 1,
.enable_blink_detection = 1,
.enable_gaze_detection = 1,
.search_density_nontracking = 33,
.search_density_tracking = 33,
.direction = 0,
.refresh_count = 2,
.threshold = 520,
.face_retry_count = 3,
.head_retry_count = 3,
.hold_count = 2,
.lock_faces = 1,
.move_rate_threshold = 8,
.ct_detection_mode = FD_CONTOUR_MODE_EYE,
/* Stabilization parameters */
.stab_enable = 1,
.stab_history = 3,
/* Position stabilization tuning params */
.stab_pos = {
  .enable = 1,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 0,
  .threshold = 4,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Size stabilization tuning params */
.stab_size = {
  .enable = 1,
  .mode = FD_STAB_CONTINUES_SMALLER,
  .state_cnt = 3,
  .threshold = 250,
  .use_reference = 0,
  .filter_type = FD_STAB_TEMPORAL,
  .temp = {
    .num = 8,
    .denom = 6,
  },
},
/* Mouth stabilization tuning params */
.stab_mouth = {
  .enable = 1,
  .mode = FD_STAB_CONTINUES_CLOSER_TO_REFERENCE,
  .state_cnt = 1,
  .threshold = 4,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Smile stabilization tuning params */
.stab_smile = {
  .enable = 1,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 3,
  .threshold = 0,
  .use_reference = 0,
  .filter_type = FD_STAB_HYSTERESIS,
  .hyst = {
    .start_A = 25,
    .end_A = 35,
    .start_B = 55,
    .end_B = 65,
  },
},
