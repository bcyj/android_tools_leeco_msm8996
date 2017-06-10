/**********************************************************************
 * Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **********************************************************************/
#ifndef _AEC_SLOW_CONV_H_
#define _AEC_SLOW_CONV_H_

#define AEC_GYRO_DISABLE                 0
#define AEC_GYRO_HISTORY_NUM             10
#define AEC_GYRO_SLOW_THRESH             0.02
#define AEC_GYRO_MEDIUM_THRESH           0.1
#define AEC_GYRO_FAST_THRESH             2
#define AEC_LUMA_THRESH                  20
#define AEC_LUMA_HISTORY_NUM             2
#define AEC_WEIGHTED_LUMA_DEFAULT        128
#define AEC_LUMA_WEIGHT_0                80
#define AEC_LUMA_WEIGHT_1                20
#define AEC_DELTA_LUMA_PCT_THRESH        0.5
#define AEC_THRESH_HOLD_LOW              6
#define AEC_THRESH_HOLD_HIGH             215
#define AEC_SKIP_RANGE_LOW               1
#define AEC_SKIP_RANGE_HIGH              235
#define AEC_MIN_LUMA_SLOW                1
#define AEC_MAX_LUMA_SLOW                240
#define AEC_MIN_LIMIT                    5
#define AEC_MAX_LIMIT                    2
#define AEC_FPS_THRESH                   20

typedef enum {
  V2_STILL,
  V2_SLOW,
  V2_MEDIUM,
  V2_FAST,
} aec_gyro_motion_t;

typedef struct {
  float v2[AEC_GYRO_HISTORY_NUM];
  unsigned int index;
} aec_gyro_history_t;

typedef struct {
  uint32_t luma_array[AEC_LUMA_HISTORY_NUM];
  int count;
} aec_luma_history_t;

typedef enum {
  FPS_LOW,
  FPS_HIGH,
  FPS_MAX,
} aec_fps_t;

typedef struct {
  float ht_delta_luma;
  uint32_t comp_luma_target;
  uint32_t current_vfe_luma;
  float holding_time;
  int luma_target_reached;
  int ht_complete;
  uint32_t min_luma_hold;
  uint32_t max_luma_hold;
  float current_frame_rate;
  aec_fps_t fps_type;
  int frame_skip_count_hold;
  int ad_frame_skip_hold;
  int ht_t_cnt;
  int frame_num_slow_conv;
  int max_exp_step_flag;
  int frame_skip_flag;
  int max_account;
  int min_account;
  uint32_t weighted_luma;

  aec_gyro_motion_t motion_type;
  aec_gyro_history_t aec_gyro_history;
  aec_luma_history_t aec_luma_history;
} aec_slow_conv_t;

#endif /* _AEC_SLOW_CONV_H_ */
