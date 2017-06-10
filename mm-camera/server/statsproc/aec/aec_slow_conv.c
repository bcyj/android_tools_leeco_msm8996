/**********************************************************************
 * Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **********************************************************************/
#include "stats_proc.h"
#include "aec.h"

int exposure_step_size_lut[4][12] = {
    {-8, -6, -4, -2, -1, 0, 1, 1, 2, 2, 2},        /* V2_STILL */
    {-10, -9, -7, -5, -2, -1, 0, 1, 1, 2, 4, 8},   /* V2_SLOW */
    {-10, -9, -8, -6, -3, -1, 0, 1, 2, 4, 8, 8},   /* V2_MEDIUM */
    {-12, -10, -8, -6, -3, -1, 0, 1, 2, 4, 8, 10}  /* V2_FAST */
};

/*==========================================================================
 * FUNCTION    - aec_gyro_history_update -
 *
 * DESCRIPTION:  Add an entry to gyro history
 *=========================================================================*/
static void aec_gyro_history_update(aec_t *aec, float gyro_sqr)
{
  aec_gyro_history_t *aec_gyro_history = &(aec->slow_c.aec_gyro_history);

  /* Add new entry to the history */
  aec_gyro_history->v2[aec_gyro_history->index] = gyro_sqr;

  /* Update index */
  aec_gyro_history->index = (aec_gyro_history->index + 1)
      % AEC_GYRO_HISTORY_NUM;
}

/*==========================================================================
 * FUNCTION    - aec_gyro_history_find_max -
 *
 * DESCRIPTION:  Find max value in gyro history
 *=========================================================================*/
static float aec_gyro_history_find_max(aec_t *aec)
{
  float v2_max = 0.0;
  int i;
  aec_gyro_history_t *aec_gyro_history = &(aec->slow_c.aec_gyro_history);

  for (i = 0; i < AEC_GYRO_HISTORY_NUM; i++) {
    if (aec_gyro_history->v2[i] > v2_max) {
      v2_max = aec_gyro_history->v2[i];
    }
  }

  return v2_max;
}

/*==========================================================================
 * FUNCTION    - aec_gyro_classify_motion -
 *
 * DESCRIPTION:  Classify motion based on gyro data
 *=========================================================================*/
static aec_gyro_motion_t aec_gyro_classify_motion(stats_proc_gyro_info_t *gyro,
    aec_t *aec)
{
  float gyro_sqr = 0.0;
  float v2_max = 0.0;

  if (!gyro->float_ready) {
    return V2_MEDIUM;
  }

#if (AEC_GYRO_DISABLE)
  return V2_MEDIUM;
#endif

  gyro_sqr = (gyro->flt[0] * gyro->flt[0])
      + (gyro->flt[1] * gyro->flt[1])
      + (gyro->flt[2] * gyro->flt[2]);

  aec_gyro_history_update(aec, gyro_sqr);
  v2_max = aec_gyro_history_find_max(aec);

  if (v2_max < AEC_GYRO_SLOW_THRESH) {
    return V2_STILL;
  } else if (v2_max < AEC_GYRO_MEDIUM_THRESH) {
   return V2_SLOW;
  } else if (v2_max < AEC_GYRO_FAST_THRESH) {
    return V2_MEDIUM;
  } else {
    return V2_FAST;
  }
}

/*==========================================================================
 * FUNCTION    - aec_calc_holding_time -
 *
 * DESCRIPTION:  Calculate holding time
 *=========================================================================*/
static void aec_calc_holding_time(aec_slow_conv_t *slow_conv) {
  switch (slow_conv->motion_type) {
  case V2_STILL:
    slow_conv->holding_time = slow_conv->current_frame_rate;
    break;
  case V2_SLOW:
    slow_conv->holding_time = (slow_conv->current_frame_rate * 3/5);
    break;
  case V2_MEDIUM:
    slow_conv->holding_time = (slow_conv->current_frame_rate * 2/5);
    break;
  case V2_FAST:
    slow_conv->holding_time = (slow_conv->current_frame_rate * 1/5);
    break;
  default:
    break;
  }
}

/*==========================================================================
 * FUNCTION    - aec_luma_history_update -
 *
 * DESCRIPTION:  Add a new entry to luma history
 *=========================================================================*/
static void aec_luma_history_update(aec_slow_conv_t *slow_conv)
{
  aec_luma_history_t *luma_history = &(slow_conv->aec_luma_history);

  if (luma_history->count == 0) {
    luma_history->luma_array[0] = slow_conv->current_vfe_luma;
    luma_history->count++;
  } else if (luma_history->count == 1) {
    luma_history->luma_array[1] = slow_conv->current_vfe_luma;
    luma_history->count++;
  } else if (luma_history->count == 2) {
    luma_history->luma_array[0] = luma_history->luma_array[1];
    luma_history->luma_array[1] = slow_conv->current_vfe_luma;
  }
}

/*==========================================================================
 * FUNCTION    - aec_luma_history_weighted_luma -
 *
 * DESCRIPTION:  Calculate weighted luma from history
 *=========================================================================*/
static uint32_t aec_luma_history_weighted_luma(aec_t *aec)
{
  aec_luma_history_t *luma_history = &(aec->slow_c.aec_luma_history);
  uint32_t weighted_luma;

  if (luma_history->count == 0) {
    weighted_luma = AEC_WEIGHTED_LUMA_DEFAULT;
  } else if (luma_history->count == 1) {
    weighted_luma = luma_history->luma_array[0];
  } else {
    weighted_luma = ((luma_history->luma_array[0] * AEC_LUMA_WEIGHT_0)
        + (luma_history->luma_array[1] * AEC_LUMA_WEIGHT_1))
        / (AEC_LUMA_WEIGHT_0 + AEC_LUMA_WEIGHT_1);
    if (weighted_luma > 255)
      weighted_luma = 255;
  }

  return weighted_luma;
}

/*==========================================================================
 * FUNCTION    - aec_holding_time_control -
 *
 * DESCRIPTION:  Adaptive holding time control using gyro sensor data
 *=========================================================================*/
static int aec_holding_time_control(aec_slow_conv_t *slow_conv)
{
  float ht;
  int frame_skip_threshold;

  frame_skip_threshold = (int)slow_conv->holding_time;

  if ((slow_conv->current_vfe_luma >= (slow_conv->comp_luma_target - 4))
      && (slow_conv->current_vfe_luma <= (slow_conv->comp_luma_target + 4))) {
    slow_conv->min_luma_hold = slow_conv->comp_luma_target - 4;
    slow_conv->max_luma_hold = slow_conv->comp_luma_target + 4;
    slow_conv->luma_target_reached = 1;
    slow_conv->ht_complete = 0;
    slow_conv->ht_t_cnt = 0;
    slow_conv->frame_skip_count_hold = 0;
    slow_conv->ad_frame_skip_hold = 0;
  } else if ((slow_conv->luma_target_reached) &&
      (slow_conv->frame_skip_count_hold > slow_conv->ad_frame_skip_hold)) {
    /* Hold time and luma target reached. Reinit */
    slow_conv->min_luma_hold = slow_conv->comp_luma_target - 4;
    slow_conv->max_luma_hold = slow_conv->comp_luma_target + 4;
    slow_conv->luma_target_reached = 0;
    slow_conv->frame_skip_count_hold = 0;
    slow_conv->ht_complete = 1;
    //return 0;
  }

  if ((!slow_conv->ht_complete) && (slow_conv->luma_target_reached)) {
    if (slow_conv->current_vfe_luma < slow_conv->min_luma_hold) {
      slow_conv->min_luma_hold = slow_conv->current_vfe_luma;
    } else if (slow_conv->current_vfe_luma > slow_conv->max_luma_hold) {
      slow_conv->max_luma_hold = slow_conv->current_vfe_luma;
    }

    if ((!slow_conv->ht_delta_luma)
        || (slow_conv->ht_t_cnt > frame_skip_threshold)) {
      ht = 0;
      slow_conv->ad_frame_skip_hold = (int)ht;
      slow_conv->ht_complete = 1;
    } else {
      ht = slow_conv->ht_delta_luma - slow_conv->ht_t_cnt;
      if (ht <= 0) {
        ht = 0;
        slow_conv->ad_frame_skip_hold = (int)ht;
        slow_conv->ht_complete = 1;
      } else {
        slow_conv->ad_frame_skip_hold = (int)ht;
        slow_conv->ht_t_cnt++;
        return 1;
      }
    }

    if (((slow_conv->min_luma_hold >= AEC_THRESH_HOLD_LOW)
        && (slow_conv->min_luma_hold < (slow_conv->comp_luma_target - 4)))
        || ((slow_conv->max_luma_hold > (slow_conv->comp_luma_target + 4))
        && (slow_conv->max_luma_hold <= AEC_THRESH_HOLD_HIGH))) {
      slow_conv->frame_skip_count_hold++;

      if (slow_conv->frame_skip_count_hold > slow_conv->ad_frame_skip_hold) {
        slow_conv->ht_complete = 1;
      }

      slow_conv->frame_num_slow_conv++;
      return 1;
    }
  }

  return 0;
}

/*==========================================================================
 * FUNCTION    - aec_conv_time_control_init -
 *
 * DESCRIPTION:  Initialize variables for convergence control
 *=========================================================================*/
static void aec_conv_time_control_init(aec_slow_conv_t *slow_conv)
{
  slow_conv->frame_skip_flag = 0;
  slow_conv->max_exp_step_flag = 0;
  slow_conv->max_account = 0;
  slow_conv->min_account = 0;
}

/*==========================================================================
 * FUNCTION    - aec_convergence_time_control -
 *
 * DESCRIPTION:  Adjust Convergence Time
 *=========================================================================*/
static int aec_convergence_time_control(aec_slow_conv_t *slow_conv)
{
  int frame_skip_slow_conv;

  if (slow_conv->fps_type == FPS_LOW) {
    /* LOW FRAME RATE */
    frame_skip_slow_conv = 1;
    slow_conv->max_exp_step_flag = 1;

    if (((slow_conv->frame_num_slow_conv % (frame_skip_slow_conv + 1)) != 0)
        && (slow_conv->current_vfe_luma >= AEC_SKIP_RANGE_LOW)
        && (slow_conv->current_vfe_luma <= AEC_SKIP_RANGE_HIGH)) {
      /* Skip this frame */
      slow_conv->frame_num_slow_conv++;
      return 1;
    }
    /* Frame not skipped */
    slow_conv->frame_num_slow_conv++;
  } else {
    /* HIGH FRAME RATE */
    if (slow_conv->current_vfe_luma >= AEC_MAX_LUMA_SLOW) {
      slow_conv->max_account++;
      slow_conv->max_exp_step_flag = 1;
      if (slow_conv->max_account >= AEC_MAX_LIMIT)
        slow_conv->frame_skip_flag = 1;
    } else if (slow_conv->current_vfe_luma >= AEC_MIN_LUMA_SLOW) {
      slow_conv->min_account++;
      slow_conv->max_exp_step_flag = 1;
      if (slow_conv->min_account >= AEC_MIN_LIMIT)
        slow_conv->frame_skip_flag = 1;
    }

    if (slow_conv->frame_skip_flag) {
      frame_skip_slow_conv = 1;
      if (((slow_conv->frame_num_slow_conv % (frame_skip_slow_conv + 1)) != 0)
          && (slow_conv->current_vfe_luma >= AEC_SKIP_RANGE_LOW)
          && (slow_conv->current_vfe_luma <= AEC_SKIP_RANGE_HIGH)) {
        slow_conv->frame_num_slow_conv++;
        return 1;
      }
    }

    if (((slow_conv->frame_skip_flag) || (slow_conv->max_exp_step_flag))
        && (slow_conv->current_vfe_luma >= (slow_conv->comp_luma_target - 4))
        && (slow_conv->current_vfe_luma <= (slow_conv->comp_luma_target + 4))) {
      aec_conv_time_control_init(slow_conv);
    }
  }

  return 0;
}

/*==========================================================================
 * FUNCTION    - aec_exposure_step_size_control -
 *
 * DESCRIPTION:  Calculate AEC Exposure Step Size
 *=========================================================================*/
static int aec_exposure_step_size_control(aec_slow_conv_t *slow_conv)
{
  int row = 0;
  int column = 0;
  int exp_step_size;

  /* Select Row */
  if (slow_conv->motion_type == V2_STILL)
    row = 0;
  else if (slow_conv->motion_type == V2_SLOW)
    row = 1;
  else if (slow_conv->motion_type == V2_MEDIUM)
    row = 2;
  else if (slow_conv->motion_type == V2_FAST)
    row = 3;

  /* Select Column */
  if ((slow_conv->weighted_luma >
      (slow_conv->comp_luma_target + 128 +
      (255 - 128 - slow_conv->comp_luma_target) / 2))
      && (slow_conv->weighted_luma <= 255)) {
    column = 0;
  } else if ((slow_conv->weighted_luma > (slow_conv->comp_luma_target + 128))
      && (slow_conv->weighted_luma <=
      (slow_conv->comp_luma_target + 128 +
      (255 - 128 - slow_conv->comp_luma_target) / 2))) {
    column = 1;
  } else if ((slow_conv->weighted_luma > (slow_conv->comp_luma_target + 64))
      && (slow_conv->weighted_luma <= (slow_conv->comp_luma_target + 128))) {
    column = 2;
  } else if ((slow_conv->weighted_luma > (slow_conv->comp_luma_target + 32))
      && (slow_conv->weighted_luma <= (slow_conv->comp_luma_target + 64))) {
    column = 3;
  } else if ((slow_conv->weighted_luma > (slow_conv->comp_luma_target + 8))
      && (slow_conv->weighted_luma <= (slow_conv->comp_luma_target + 32))) {
    column = 4;
  } else if ((slow_conv->weighted_luma > (slow_conv->comp_luma_target + 4))
      && (slow_conv->weighted_luma <= (slow_conv->comp_luma_target + 8))) {
    column = 5;
  } else if ((slow_conv->weighted_luma >= (slow_conv->comp_luma_target - 4))
      && (slow_conv->weighted_luma <= (slow_conv->comp_luma_target + 4))) {
    column = 6;
  } else if ((slow_conv->weighted_luma >= (slow_conv->comp_luma_target - 8))
      && (slow_conv->weighted_luma < (slow_conv->comp_luma_target - 4))) {
    column = 7;
  } else if ((slow_conv->weighted_luma >= (slow_conv->comp_luma_target - 16))
      && (slow_conv->weighted_luma < (slow_conv->comp_luma_target - 8))) {
    column = 8;
  } else if ((slow_conv->weighted_luma >= (slow_conv->comp_luma_target - 32))
      && (slow_conv->weighted_luma < (slow_conv->comp_luma_target - 16))) {
    column = 9;
  } else if ((slow_conv->weighted_luma >=
      (slow_conv->comp_luma_target - 32 -
      ((slow_conv->comp_luma_target - 32) / 2)))
      && (slow_conv->weighted_luma < (slow_conv->comp_luma_target - 32))) {
    column = 10;
  } else if ((slow_conv->weighted_luma <
      (slow_conv->comp_luma_target - 32 -
      ((slow_conv->comp_luma_target - 32) / 2)))) {
    column = 11;
  }

  /* Get exposure step size from LUT */
  exp_step_size = exposure_step_size_lut[row][column];

  /* Check for exceptions */
  if (row == 0) {
    if (slow_conv->fps_type == FPS_MAX) {
      if (column == 4)
        exp_step_size = -2;
      if (column == 8)
        exp_step_size = 2;
    }
    if ((slow_conv->fps_type == FPS_LOW) || (slow_conv->max_exp_step_flag)) {
      if (column == 1)
        exp_step_size = -7;
      if (column == 2)
        exp_step_size = -6;
      if (column == 3)
        exp_step_size = -4;
      if ((column == 9) || (column == 10) || (column == 11))
        exp_step_size = 4;
    }
  }

  return exp_step_size;
}

/*==========================================================================
 * FUNCTION    - aec_minor_dist_level_eval -
 *
 * DESCRIPTION:  Evaluate minor disturbance level
 *=========================================================================*/
static void aec_minor_dist_level_eval(aec_slow_conv_t *slow_conv)
{
  float delta_luma_pct;
  int frame_skip_hold;

  frame_skip_hold = (int)slow_conv->holding_time;

  if (slow_conv->current_vfe_luma >= slow_conv->comp_luma_target) {
    delta_luma_pct = slow_conv->current_vfe_luma
        - slow_conv->comp_luma_target;
  } else {
    delta_luma_pct = slow_conv->comp_luma_target;
  }

  if (delta_luma_pct >
      (AEC_DELTA_LUMA_PCT_THRESH * slow_conv->comp_luma_target * 4/4)) {
    slow_conv->ht_delta_luma = 0;
  } else if (delta_luma_pct >=
      (AEC_DELTA_LUMA_PCT_THRESH * slow_conv->comp_luma_target * 3/4)) {
    slow_conv->ht_delta_luma = 0.2 * frame_skip_hold;
  } else if (delta_luma_pct >=
      (AEC_DELTA_LUMA_PCT_THRESH * slow_conv->comp_luma_target * 2/4)) {
    slow_conv->ht_delta_luma = 0.4 * frame_skip_hold;
  } else if (delta_luma_pct >=
      (AEC_DELTA_LUMA_PCT_THRESH * slow_conv->comp_luma_target * 1/4)) {
    slow_conv->ht_delta_luma = 0.8 * frame_skip_hold;
  } else {
    slow_conv->ht_delta_luma = 1 * frame_skip_hold;
  }
}

/*==========================================================================
 * FUNCTION    - aec_slow_convergence_gaht -
 *
 * DESCRIPTION:  Adjust exposure on the camera sensor.  Implement a
 *               generic algorithm that can work with any sensor
 *=========================================================================*/
void aec_slow_convergence(stats_proc_t *sproc, aec_t *aec,
    uint32_t current_vfe_luma)
{
  int exp_step_size;
  int max_exp_index;
  aec_slow_conv_t *slow_conv;

  slow_conv = &(aec->slow_c);

  slow_conv->motion_type = aec_gyro_classify_motion(&(sproc->input.gyro_info),
      aec);

  slow_conv->current_frame_rate = (sproc->input.sensor_info.current_fps >> 8);
  if (slow_conv->current_frame_rate <= AEC_FPS_THRESH)
    slow_conv->fps_type = FPS_LOW;
  else if (sproc->input.sensor_info.preview_fps ==
      sproc->input.sensor_info.max_preview_fps)
    slow_conv->fps_type = FPS_MAX;
  else
    slow_conv->fps_type = FPS_HIGH;

  slow_conv->current_vfe_luma = current_vfe_luma;
  slow_conv->comp_luma_target = aec->luma.comp_target;

  aec_luma_history_update(slow_conv);

  aec_calc_holding_time(slow_conv);

  aec_minor_dist_level_eval(slow_conv);

  if (aec_holding_time_control(slow_conv))
    return;

  if (aec_convergence_time_control(slow_conv))
    return;

  slow_conv->weighted_luma = aec_luma_history_weighted_luma(aec);

  exp_step_size = aec_exposure_step_size_control(slow_conv);

  if (exp_step_size == 0)
    sproc->share.luma_settled_cnt++;

  sproc->share.aec_ext.exp_index += exp_step_size;

  /* Subract 1 from num exposure values to get max exposure index */
  max_exp_index = (int)(sproc->share.aec_ext.exp_tbl_val - 1);
  if (sproc->share.aec_ext.exp_index > max_exp_index)
    sproc->share.aec_ext.exp_index = max_exp_index;
  if (sproc->share.aec_ext.exp_index < 0)
    sproc->share.aec_ext.exp_index = 0;
  return;
}
