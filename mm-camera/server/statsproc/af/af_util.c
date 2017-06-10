/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "stats_proc.h"
#include "af.h"

#define CAF_DISABLE_GYRO 0

/* gyro thresholds - should be obtained from chromatix (not available now)*/
#define CAF_GYRO_MIN_MOVEMENT_THRESHOLD_VID        0.02
#define CAF_GYRO_MIN_MOVEMENT_THRESHOLD_CAM        0.33
#define CAF_GYRO_STABLE_DETECTED_THRESHOLD_VID     0.014
#define CAF_GYRO_STABLE_DETECTED_THRESHOLD_CAM     0.015
#define CAF_GYRO_FAST_PAN_THRESHOLD                0.10
#define CAF_GYRO_SLOW_PAN_THRESHOLD                0.04
#define CAF_GYRO_FAST_PAN_COUNT_THRESHOLD          8
#define CAF_GYRO_SUM_RETURN_TO_ORIGINAL_THRESHOLD  15
#define CAF_GYRO_STABLE_COUNT_DELAY                30

/* SAD (Sum of Absolute Difference) thresholds - should be in chromatix */
#define CAF_SAD_THRESHOLD_MIN                      5
#define CAF_SAD_THRESHOLD_MAX                      10
#define CAF_SAD_GAIN_MIN                           2.0
#define CAF_SAD_GAIN_MAX                           4.0

#define CAF_SAD_REF_THRESHOLD_MIN                  5
#define CAF_SAD_REF_THRESHOLD_MAX                  10
#define CAF_SAD_REF_GAIN_MIN                       2.0
#define CAF_SAD_REF_GAIN_MAX                       4.0

#define CAF_SAD_FRAMES_TO_WAIT                     5

/* Define auto focus windows for average selection */
uint8_t af_average_windows[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS] =
{ 10, 13, 16, 37, 40, 43, 64, 67, 70};

/* Define auto focus windows for spot selection */
uint8_t af_spot_windows[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS] =
{ 30, 31, 32, 39, 40, 41, 48, 49, 50};

/* Define auto focus windows for center weighted selection */
uint8_t af_center_weighted_windows[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS] =
{ 13, 21, 23, 38, 40, 42, 57, 59, 67};

/* Define auto focus windows for face detection/touch af */
uint8_t af_multi_roi_windows[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0};

/************************
     UTIL APIS
*************************/

/*===========================================================================
* FUNCTION    - af_cont_get_sad_threshold -
*
* DESCRIPTION: calculate SAD threshold
*==========================================================================*/
static int af_cont_get_sad_threshold(
  float cur_real_gain,
  int th_min, int th_max,
  float gain_min, float gain_max)
{
  int threshold;

  if (cur_real_gain <= gain_min) {
    threshold = th_min;
  }
  else if (cur_real_gain >= gain_max) {
    threshold = th_max;
  }
  else
  {
    threshold = (int)((th_max - th_min) *
      (cur_real_gain - gain_min)/ (gain_max - gain_min) + th_min);
  }

  CDBG_AF("%s: SAD threshold to check: %d", __func__, threshold);
  return threshold;
}

/*===========================================================================
* FUNCTION    - af_cont_did_sad_change -
*
* DESCRIPTION: check if SAD of luma sum has changed.
*==========================================================================*/
int af_cont_did_sad_change(stats_proc_t *sproc, af_t *af)
{
  int i, j;
  unsigned int sad = 0, sadr = 0;
  int af_num_divisions_at_each_side, af_num_focus_regions;
  unsigned int sad_stable_th, sadr_change_th;
  af_continuous_t *caf = &(af->caf);
  unsigned int *sumLumaArray =
    (unsigned int *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;

  // If we haven't initialized yet
  if (!caf->sad_data.initialized) {
    CDBG_AF("%s: Initialize SAD Luma Sum arrarys...", __func__);
    memcpy(caf->sad_data.prev_luma_sum, sumLumaArray,
      sproc->input.mctl_info.numRegions * sizeof(unsigned int));
    memcpy(caf->sad_data.ref_luma_sum, sumLumaArray,
      sproc->input.mctl_info.numRegions * sizeof(unsigned int));
    caf->sad_data.initialized = TRUE;
  }

  // Update reference sad data if necessary
  if (caf->sad_data.set_ref_luma_sum) {
    // We will need to wait for few frames for AEC stats to
    // settle down.
    if (caf->sad_data.frame_count++ >= 4) {
      memcpy(caf->sad_data.ref_luma_sum, sumLumaArray,
      sproc->input.mctl_info.numRegions * sizeof(unsigned int));
      caf->sad_data.set_ref_luma_sum = 0;

      CDBG_AF("%s: SAD Reference set.", __func__);
      caf->sad_data.frame_count = 0;
    }
  }

  // directly copied from aec.c
  // TBD: If CAF is enabled with face detection? Need to investigate.
  // vfe configured to use 16x16
  if (sproc->input.mctl_info.numRegions == 256) {
    af_num_divisions_at_each_side = 16;
    af_num_focus_regions = 64;
    // use center region
    for (i = 4; i < 12; i++) {
      for (j = 4; j < 12; j++) {
        int region = (i * af_num_divisions_at_each_side + j);
        sad += abs(sumLumaArray[region] - caf->sad_data.prev_luma_sum[region]);
        sadr += abs(sumLumaArray[region] - caf->sad_data.ref_luma_sum[region]);
      }
    }
  }
  else {
    // vfe configured for 8x8 region
    af_num_divisions_at_each_side = 8;
    af_num_focus_regions = 16;
    // use center region
    for (i = 2; i < 6; i++) {
      for (j = 2; j < 6; j++) {
        int region = (i * af_num_divisions_at_each_side + j);
        sad += abs(sumLumaArray[region] - caf->sad_data.prev_luma_sum[region]);
        sadr += abs(sumLumaArray[region] - caf->sad_data.ref_luma_sum[region]);
      }
    }
  }

  // Normalize SAD values
  sad = (int) ((float)sad / af_num_focus_regions /
    sproc->input.mctl_info.pixelsPerRegion);
  sadr = (int) ((float)sadr / af_num_focus_regions /
    sproc->input.mctl_info.pixelsPerRegion);

  memcpy(caf->sad_data.prev_luma_sum, sumLumaArray,
    sproc->input.mctl_info.numRegions * sizeof(unsigned int));

  sad_stable_th = af_cont_get_sad_threshold(sproc->share.aec_ext.cur_real_gain,
    CAF_SAD_THRESHOLD_MIN, CAF_SAD_THRESHOLD_MAX,
    CAF_SAD_GAIN_MIN, CAF_SAD_GAIN_MAX);
  sadr_change_th = af_cont_get_sad_threshold(sproc->share.aec_ext.cur_real_gain,
    CAF_SAD_REF_THRESHOLD_MIN, CAF_SAD_REF_THRESHOLD_MAX,
    CAF_SAD_REF_GAIN_MIN, CAF_SAD_REF_GAIN_MAX);

  // Count number of stable frames based on SAD metric
  if (sad < sad_stable_th)
    caf->sad_data.stable_count++;
  else
    caf->sad_data.stable_count = 0;

  CDBG_AF("%s: sad=%d sad_stable_th=%d, sadr=%d sadr_change_th=%d AEC_gain=%f, "
    "stable_count: %d",
    __func__, sad, sad_stable_th, sadr, sadr_change_th,
    sproc->share.aec_ext.cur_real_gain, caf->sad_data.stable_count);

  // Compare SADR with its threshold only when scene is stable,
  // and new SADR is stored.
  if (sadr > sadr_change_th &&
    caf->sad_data.stable_count > CAF_SAD_FRAMES_TO_WAIT &&
    caf->sad_data.set_ref_luma_sum == 0){
    CDBG_AF("%s: Return True", __func__);
    return 1;
  }

  return 0;
}

/*===========================================================================
* FUNCTION    - af_cont_update_gyro_metrics -
*
* DESCRIPTION: update gyro metrics
*==========================================================================*/
static int af_cont_update_gyro_metrics(af_cont_gyro_t *gyro_data,
  stats_proc_gyro_info_t *gyro_info)
{
  int ret = 0;

#if (CAF_DISABLE_GYRO)
  return -1;
#endif

  if (gyro_info->float_ready) {
    CDBG_AF("%s: Gyro data ready! gyro_x: %f gyro_y: %f gyro_z: %f",
      __func__, gyro_info->flt[0], gyro_info->flt[1], gyro_info->flt[2]);
    gyro_data->sqr = gyro_info->flt[0] * gyro_info->flt[0] +
      gyro_info->flt[1] * gyro_info->flt[1] +
      gyro_info->flt[2] * gyro_info->flt[2];
    CDBG_AF("%s: Gyro metrics - gyro_sqr: %f", __func__, gyro_data->sqr);
  } else {
    CDBG_AF("%s: Gyro data not ready yet. Will try later!!", __func__);
    ret = -1;
  }

  return ret;

} /* af_cont_update_gyro_metrics */

/*===========================================================================
* FUNCTION    - af_cont_gyro_monitor_mode_camera -
*
* DESCRIPTION: Camera Mode Gyro assisted CAF Logic. Goal is to  initiate
* focus as soon as the target becomes stable after motion.
*==========================================================================*/
int af_cont_gyro_monitor_mode_camera(af_cont_gyro_t *gyro_data,
  stats_proc_gyro_info_t *gyro_info)
{
  /* update gyro metrics */
  if (af_cont_update_gyro_metrics(gyro_data, gyro_info)) {
    CDBG_AF("%s: Warning - failure updating gyro metrics !!!", __func__);
    gyro_data->panning_stable_for_video = TRUE;
    return 0;
  }

  /* In Camera, we want to enable auto-focus even while device is in motion */
  gyro_data->panning_stable_for_video = TRUE;

  if (gyro_data->sqr > CAF_GYRO_MIN_MOVEMENT_THRESHOLD_CAM) {
    /* As soon as we move, we should be ready to refocus */
    gyro_data->gyro_moved_flag = TRUE;
    CDBG_AF("%s: Gyro in motion. Set the flag(%d)!!!",
      __func__, gyro_data->gyro_moved_flag);
  } else {
    if (gyro_data->sqr < CAF_GYRO_STABLE_DETECTED_THRESHOLD_CAM) {
      /* Device is stable after motion - we should just do auto-focus */
      gyro_data->refocus_for_gyro = gyro_data->gyro_moved_flag ? TRUE : FALSE;
      CDBG_AF("%s: Gyro stable. Refocus forced?"
        "(Refocus_Flag: %d Moved_Flag: %d)!", __func__,
        gyro_data->refocus_for_gyro, gyro_data->gyro_moved_flag);

      /* Set gyro_moved_flag to False */
      CDBG_AF("%s: Resetting Gyro Moved Flag to FALSE!!!", __func__);
      gyro_data->gyro_moved_flag = FALSE;
    }
  }

  return 0;
} /* af_cont_gyro_monitor_mode_camera */

/*===========================================================================
* FUNCTION    - af_cont_gyro_monitor_mode_video -
*
* DESCRIPTION: Video Mode Gyro assisted CAF Logic in Monitor State.
* Goal is to disable CAF during motion and initiate focus as soon as target
* becomes stable for some specified amount of time after fast panning.
* RETURNS: 0 - Good to proceed further to another state.
*          1 - Not ready yet. Stay in monitor state.
*==========================================================================*/
int af_cont_gyro_monitor_mode_video(af_cont_gyro_t *gyro_data,
  stats_proc_gyro_info_t *gyro_info)
{
  /* update gyro metrics */
  /* update gyro metrics */
  if (af_cont_update_gyro_metrics(gyro_data, gyro_info)) {
    CDBG_AF("%s: Warning - failure updating gyro metrics !!!", __func__);
    gyro_data->panning_stable_for_video = TRUE;
    return 0;
  }

  /* Are we in motion? */
  if (gyro_data->sqr > CAF_GYRO_MIN_MOVEMENT_THRESHOLD_VID) {
    CDBG_AF("%s: Gyro metric(%f) signifes motion greater than threshold(%f)",
      __func__, gyro_data->sqr, CAF_GYRO_MIN_MOVEMENT_THRESHOLD_VID);
    gyro_data->x_sum += gyro_info->flt[0];
    gyro_data->y_sum += gyro_info->flt[1];
    gyro_data->z_sum += gyro_info->flt[2];
    gyro_data->no_pan_cnt = 0;
    gyro_data->panning_stable_for_video = FALSE;

    if (gyro_data->sqr > CAF_GYRO_FAST_PAN_THRESHOLD) {
      /* We are doing fast panning */
      gyro_data->fast_pan_cnt += 1;
      gyro_data->slow_pan_cnt = 0;  /* not used currently */
      CDBG_AF("%s: Gyro indicates fast-panning(count=%d)."
        "Resetting slow-pan count",
        __func__, gyro_data->fast_pan_cnt);
    } else if (gyro_data->sqr < CAF_GYRO_SLOW_PAN_THRESHOLD) {
      /* We are panning slowly. */
      /* Slow panning is ignored now - we just make sure we don't do
       * CAF during slow panning though */
      gyro_data->slow_pan_cnt += 1;
      gyro_data->fast_pan_cnt = 0;
      CDBG_AF("%s: Gyro indicates slow-panning(count=%d)."
        "Resetting fast-pan count to 0.",
        __func__, gyro_data->slow_pan_cnt);
    }
  } else {
    CDBG_AF("%s: Gyro metric less than minimum threshold. Check stability",
      __func__);
    /* Check if the target is stable */
    if (gyro_data->sqr < CAF_GYRO_STABLE_DETECTED_THRESHOLD_VID) {
      /* Increment 'device-is-stable' count */
      gyro_data->no_pan_cnt += 1;
      float gyro_sum = gyro_data->x_sum +
        gyro_data->y_sum +
        gyro_data->z_sum;

      CDBG_AF("%s: Gyro stable. Stable count = %d. Gyro Sum: %f",
        __func__, gyro_data->no_pan_cnt, gyro_sum);

      /* we need absolute value - if negative, make it positive */
      if (gyro_sum < 0) gyro_sum *= (-1);

      /* if device was doing fast-panning for long time (more than
       * predefined threshold) and we are not back to the same position
       * where we started from, we set the refocus_for_gyro flag to TRUE
       * which will result in fast refocus irrespective of other
       * conditions. */
      if ((gyro_data->fast_pan_cnt > CAF_GYRO_FAST_PAN_COUNT_THRESHOLD) &&
        (gyro_sum > CAF_GYRO_SUM_RETURN_TO_ORIGINAL_THRESHOLD)) {
        gyro_data->refocus_for_gyro = TRUE;
        gyro_data->no_pan_cnt = CAF_GYRO_STABLE_COUNT_DELAY + 1;
        CDBG_AF("%s: Fast panning more than threshold. Refocus forced.",
          __func__);
      }
    }
  }

  /* We always end up in this check */
  /* Check if we have been stable for predefined number of frames */
  if (gyro_data->no_pan_cnt > CAF_GYRO_STABLE_COUNT_DELAY) {
    CDBG_AF("%s: Gyro stable for long time now."
      "Can be refocused if other conditions meet!", __func__);
    gyro_data->no_pan_cnt = CAF_GYRO_STABLE_COUNT_DELAY + 1;
    gyro_data->x_sum = 0;
    gyro_data->y_sum = 0;
    gyro_data->z_sum = 0;

    /* Our device is stable - we are free to refocus if other conditions
     * change. */
    gyro_data->panning_stable_for_video = TRUE;
  } else {
    /* Device not stable yet. not ready to refocus. */
    CDBG_AF("%s: Device not stable. Returning!", __func__);
    return 1;
  }

  return 0;
} /* af_cont_gyro_monitor_mode_video */

/*===========================================================================
* FUNCTION    - af_safe_move_lens -
*
* DESCRIPTION:
*==========================================================================*/
void af_safe_move_lens(stats_proc_t *sproc, af_t *af, int dir, int *steps)
{
  CDBG_AF("%s:direction %d, infy pos %d, cur pos %d\n", __func__, dir,
    af->infy_pos, af->cur_pos);

  if (dir == MOVE_NEAR && (af->cur_pos - *steps) < 0)
    *steps = af->cur_pos;

  if (dir == MOVE_FAR && (af->cur_pos + *steps) > af->infy_pos)
    *steps = af->infy_pos - af->cur_pos;
} /* af_safe_move_lens */

/*===========================================================================
* FUNCTION    - af_move_lens -
*
* DESCRIPTION:
*==========================================================================*/
void af_move_lens(stats_proc_t *sproc, af_t *af, int dir, int steps)
{
  CDBG_AF("%s:direction %d steps %d\n", __func__, dir, steps);
  af_safe_move_lens(sproc, af, dir, &steps);
  sproc->share.af_ext.direction = dir;
  sproc->share.af_ext.num_of_steps = steps;
  sproc->share.af_ext.move_lens_status = TRUE;
} /* af_move_lens */

/*===========================================================================
 * FUNCTION    - af_stop_focus -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_stop_focus(stats_proc_t *sproc, af_t *af)
{
  int rc = -1;

  if (sproc->share.af_ext.active &&
    sproc->share.af_ext.cont_af_enabled == FALSE) {
    CDBG_AF("%s:Set the Auto focus stop, No CONTI AF\n", __func__);
    sproc->share.af_ext.stop_af = TRUE;
    sproc->share.af_ext.num_of_steps = 0;
    rc = 0;
  }
  af->state = AF_INACTIVE;
  sproc->share.af_ext.active = FALSE;

  if (sproc->input.flash_info.led_state != LED_MODE_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_TORCH)
    sproc->share.af_ext.af_led_on_skip_frm = 3;
  else
    sproc->share.af_ext.af_led_on_skip_frm = 0;
  return rc;
} /* af_stop_focus */

/*==========================================================================
 * FUNCTION    - af_load_chromatix -
 *
 * DESCRIPTION:
 *=========================================================================*/
void af_load_chromatix(stats_proc_t *sproc, af_t *af)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  /* We should not reset cur_pos if we are going to
     ignore full-sweep. That means we want to preserve
     previous state */
  if (NULL == fptr)
    return;

  if (!af->caf.ignore_full_sweep) {
    af->cur_pos = fptr->position_far_end;
  }
  af->algo_type = fptr->af_process_type;
  af_set_focus_mode(sproc, af);

} /* af_load_chromatix */

/*==========================================================================
 * FUNCTION    - af_init_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
void af_init_data(stats_proc_t *sproc, af_t *af)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  af_algo_type_t af_process = AF_PROCESS_DEFAULT;

  if (NULL == fptr)
    return;

  sproc->share.af_ext.cur_af_mode = AF_MODE_MACRO;
  af->default_focus = fptr->position_default_in_normal;
  af->start_lens_pos = af->infy_pos;
  af->lens_move_done = TRUE;
  /* Init AF state struct */
  af->state = AF_INACTIVE;
  sproc->share.af_ext.active = FALSE;
  af->lens_moved = 0;
  af->in_low_light = FALSE;

  CDBG_AF("%s: position=%d  default process_type %d\n", __func__, af->cur_pos,
    fptr->af_process_type);
  /*  see which search algorithm we are to use */
  af->soft_focus_degree = 1.0;

  switch (af->algo_type) {
  case AF_EXHAUSTIVE_SEARCH:
  case AF_EXHAUSTIVE_FAST:
    /* INIT EXHAUSTIVE Saerch Params */
    af->state = AF_INACTIVE;
    sproc->share.af_ext.active = FALSE;
    af->downhill_allowance = 2;
    af->uphill_threshold   = 3;
    af->fv_drop_allowance  = 0.5;
    break;
  case AF_HILL_CLIMBING_CONSERVATIVE:
  case AF_HILL_CLIMBING_DEFAULT:
  case AF_HILL_CLIMBING_AGGRESSIVE:
#ifdef HILL_CLIMB_ALGO
    af_init_hill_climbing_search(sproc, af);
#endif
    break;
  default:
    CDBG_ERROR("%s: %d:Failed: Undefined af_process_type %d\n", __func__, __LINE__,
      af->algo_type);
 }
  af->collect_end_stat = FALSE;
  sproc->share.af_ext.direction = MOVE_NEAR;
  af->luma_tabilize_wait = TRUE;
  return;
} /* af_init_data */

/*==========================================================================
 * FUNCTION    - af_reset_lens -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_reset_lens(stats_proc_t *sproc, af_t *af)
{
  /* After snapshot when CAF is retriggered af_reset_lens is
     called which will - reset lens to default positin and
     update cur_pos to infy. If we are ignoring full sweep
     we do not want to reset lens. */
  if (!sproc->share.af_ext.cont_af_enabled ||
     !af->caf.ignore_full_sweep){
    af->cur_pos = af->infy_pos;
    sproc->share.af_ext.reset_lens = TRUE;
  }
  af->sp.frame_delay = 1;
  sproc->share.af_ext.num_of_steps  = 0;
  return 0;
} /* af_reset_lens */

/*==========================================================================
 * FUNCTION    - af_set_focus_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_set_focus_mode(stats_proc_t *sproc, af_t *af)
{
  int rc = 0;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  CDBG_AF("%s:AF mode is %d\n", __func__,
    sproc->share.af_ext.cur_af_mode);

  /* If bestshot mode is enabled, AF mode is determined while configuring
   bestshot mode here in 3A (updated in af->srch_mode).*/
  if (af->bestshot_d.curr_mode != CAMERA_BESTSHOT_OFF) {
    sproc->share.af_ext.cur_af_mode = af->srch_mode;
  }
  else /* Otherwise update af->srch_mode */
    af->srch_mode = sproc->share.af_ext.cur_af_mode;

  switch (af->srch_mode) {
  case AF_MODE_NORMAL:
    af->near_end  = fptr->position_boundary;
    af->far_end   = fptr->position_far_end;
    af->infy_pos  = fptr->position_far_end;
    af->default_focus = fptr->position_default_in_normal;
    break;
  case AF_MODE_AUTO:
  case AF_MODE_MACRO:
    af->near_end  = fptr->position_near_end;
    af->far_end   = fptr->position_far_end;
    af->infy_pos  = fptr->position_far_end;
    af->default_focus = fptr->position_default_in_macro;
    break;
  case AF_MODE_INFINITY:
    CDBG_AF("%s: Infinity mode -"
      "Reset lens only if we are not at infy position!!!", __func__);
    if (af->cur_pos != af->infy_pos) {
      af_reset_lens(sproc, af);
    }
    break;
  case AF_MODE_CAF:
    af->near_end  = fptr->position_near_end;
    af->far_end   = fptr->position_far_end;
    af->infy_pos  = fptr->position_far_end;
    af->default_focus = fptr->position_default_in_normal;
    break;
  case AF_MODE_UNCHANGED:
    break;
  default:
    CDBG_ERROR("%s: Failed: Unknown AF mode\n", __func__);
    rc = -1;
    break;
  }

  return rc;
}
/*==========================================================================
 * FUNCTION    - af_set_start_parameters -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_set_start_parameters(stats_proc_t *sproc, af_t *af)
{
  stats_proc_actuator_info_t *actr_info = &(sproc->input.actr_info);

  af->state             = AF_START;
  sproc->share.af_ext.active = TRUE;
  af->elapsed_frame_cnt = 0;

  sproc->share.af_ext.stop_af    = FALSE;
  sproc->share.af_ext.done_status   = FALSE;
  sproc->share.af_ext.done_flag    = FALSE;
  sproc->share.af_ext.eztune.tracing_index = 0;
  af_set_focus_mode(sproc, af);
  return 0;
} /* af_set_start_parameters */

/*==========================================================================
 * FUNCTION    - af_move_lens_to -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_move_lens_to(stats_proc_t *sproc, af_t *af, int steps)
{
  if (sproc->share.af_ext.active) {
    CDBG_AF("%s:Calling af_stop_focus\n", __func__);
    af_stop_focus(sproc, af);
  }
  CDBG_AF("%s:Step size=%d\n", __func__, steps);

  sproc->share.af_ext.move_lens_status = TRUE;
  af->state                       = AF_INACTIVE;
  sproc->share.af_ext.active = FALSE;
  if (steps > 0) {
    sproc->share.af_ext.direction    = AF_MOVE_LENS_FAR;
    sproc->share.af_ext.num_of_steps = steps;
    af->cur_pos += sproc->share.af_ext.num_of_steps;
  } else if (steps < 0) {
    sproc->share.af_ext.direction    = AF_MOVE_LENS_NEAR;
    sproc->share.af_ext.num_of_steps = -steps;
    af->cur_pos -= sproc->share.af_ext.num_of_steps;
  }
  return 0;
} /* af_move_lens_to */

/*===========================================================================
 * FUNCTION    - af_lens_move_done -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_lens_move_done(stats_proc_t *sproc, af_t *af, int status)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  int af_frame_delay = 0;

  if (status) {
    if (sproc->share.af_ext.cont_af_enabled)
      af_frame_delay = fptr->af_CAF.af_cont_base_frame_delay;
    else
      af_frame_delay = fptr->basedelay_snapshot_AF;

    if (af->frame_skip) {
      af->frame_delay = 0;
      if (af->lens_state_changed) {
        af->frame_delay = 1;
        af->lens_state_changed = FALSE;
      }
    } else if (af->in_low_light)
      af->frame_delay = af_frame_delay - 1;
    else
      af->frame_delay = af_frame_delay;
    af->lens_move_done = TRUE;

    af->lens_moved++;

    CDBG_AF("%s: CP %d, STATS[-2] %d, STATS[-1] %d",
      __func__, af->cur_pos, af->cur_stat,af->prev_stat);
    if (sproc->share.af_ext.direction == MOVE_NEAR) {
      if (fptr->undershoot_protect && (af->state == AF_MAKE_DECISION ||
        af->state == AF_GATHER_STATS_CONT_SEARCH)) {
        if ((af->cur_pos - sproc->share.af_ext.num_of_steps < 2) &&
          (af->cur_stat > af->prev_stat)) {
          af->cur_pos = af->near_end + 2;
          CDBG_AF("%s: Reached macro, update CUR POS to %d",
            __func__, af->cur_pos);
        } else {
          af->cur_pos -= sproc->share.af_ext.num_of_steps;
        }
      } else {
        af->cur_pos -= sproc->share.af_ext.num_of_steps;
      }
    } else {
      if (fptr->undershoot_protect && (af->state == AF_MAKE_DECISION ||
        af->state == AF_GATHER_STATS_CONT_SEARCH)) {
        if (((af->cur_pos + sproc->share.af_ext.num_of_steps) >
          af->far_end - 2) && (af->cur_stat > af->prev_stat)) {
          af->cur_pos = af->far_end - 2;
          CDBG_AF("%s: Reached infinity, don't update CUR POS to %d",
            __func__, af->cur_pos);
        } else {
          af->cur_pos += sproc->share.af_ext.num_of_steps;
        }
      } else {
        af->cur_pos += sproc->share.af_ext.num_of_steps;
      }
    }

    /* In CAF when autofocus call is made we check the CAF status. If
     * CAF search is going on and not in monitor mode, we send the autofocus
     * event later once focusing is complete. once we find maximum FV*/
    if (sproc->share.af_ext.cont_af_enabled) {
      if ((af->state != AF_MOVING_LENS) &&
        af->caf.send_event_later && (af->caf.status == CAF_FOCUSED)) {
        sproc->share.af_ext.done_status = 1;
        sproc->share.af_ext.done_flag = TRUE;
        af->caf.send_event_later = FALSE;
      }
    }
  } else {
    CDBG_ERROR("%s: Failed: Lens move failed.\n", __func__);
    af->step_fail_cnt++;

    /* bail out if failed to move lens */
    af_reset_lens(sproc, af);
    af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
    return 0;
  }

  if (af->state == AF_FOCUSING || af->state == AF_UNABLE_TO_FOCUS) {
    if (af->collect_end_stat)
      af->state = AF_COLLECT_END_STAT;
    else {
      if (af->state == AF_UNABLE_TO_FOCUS) {
        CDBG_AF("%s: AF ENDs here - unable to focus\n", __func__);
        af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
      } else if (af->near_end == af->cur_pos) {
        CDBG_AF("%s: AF ENDs here - peak postion is near end\n", __func__);
        af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
      } else {
        CDBG_AF("%s: AF ENDs here - took %d moves to focus lens\n", __func__,
          af->lens_moved);
        af_done(sproc, af, CAMERA_EXIT_CB_DONE);
      }
    }
  }
  return 0;
} /* af_lens_move_done */

/*===========================================================================
 * FUNCTION    - af_done -
 *
 * DESCRIPTION:
 *==========================================================================*/
void af_done(stats_proc_t *sproc, af_t *af, camera_af_done_type cb)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  CDBG_AF("%s:focus_result=%d\n", __func__, cb);
  if (!af_stop_focus(sproc, af)) {
    CDBG_AF("%s:AF STATS stops sucessfully\n",__func__);
    sproc->share.af_ext.done_status = cb;
    sproc->share.af_ext.done_flag = TRUE;
  }

  /* Update final lens position - needed for mobicat*/
  af->final_lens_pos = af->cur_pos;

  /* Activate continuous AF after exhaustive AF */
  if (af->state == AF_INACTIVE && sproc->share.af_ext.cont_af_enabled) {
    if (af->caf.send_event_later) {
      sproc->share.af_ext.done_flag = TRUE;
      sproc->share.af_ext.done_status = cb;
      af->caf.send_event_later = FALSE;
    }
    af->caf.clean_panning_track = FALSE;
    af->caf.panning_unstable_cnt    = 0;
    af->caf.ignore_full_sweep = TRUE;
    af->caf.exp_index_before_change = sproc->share.aec_ext.exp_index;
    af->caf.status = CAF_FOCUSED;
    af->state                   = AF_GATHER_STATS_CONT_MONITOR;
    af->frame_delay             = fptr->af_CAF.af_cont_base_frame_delay;
    memset(&(af->caf.gyro_data), 0, sizeof(af->caf.gyro_data));
    memset(&(af->caf.sad_data), 0, sizeof(af->caf.sad_data));
    CDBG_AF("%s: Entering CAF monitoring state....\n", __func__);
  }
} /* af_done */

/************************
     GET APIS
*************************/

/*===========================================================================
 * FUNCTION    - af_get_focus_distance -
 *
 * DESCRIPTION
 *===========================================================================*/
focus_distances_info af_get_focus_distance(stats_proc_t *sproc, af_t *af)
{
  float opt_focus_dist, near_focus_dist, far_focus_dist;
  float coc, back_focus, hyperD;
  focus_distances_info focus_dist_info;
  stats_proc_actuator_info_t *actr_info = &(sproc->input.actr_info);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  float f_len = actr_info->focal_length;
  float f_num = actr_info->af_f_num;
  float f_pix = actr_info->af_f_pix;
  int num_steps;
  float f_dist = actr_info->af_total_f_dist * 100; /* recvd m, change to cm */

  if (NULL == fptr)
    num_steps = 0.0;
  else
    num_steps = (float)fptr->position_far_end;

  CDBG_AF("%s:focal_lngth %f f_num=%f f_pix=%f pos_far_end %d total_f_dist %f",
    __func__, actr_info->focal_length, actr_info->af_f_num, actr_info->af_f_pix,
    num_steps, actr_info->af_total_f_dist);

  if (!actr_info->focal_length || !actr_info->af_f_num || !actr_info->af_f_pix ||
    !num_steps || !actr_info->af_total_f_dist) {
    /* There is no support to check focus distance is supported for this
     * sensor or not. so returning fixed focus distances to pass the test */
    focus_dist_info.focus_distance[FOCUS_DISTANCE_NEAR_INDEX] = 0.10;
    focus_dist_info.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] = 0.15;
    focus_dist_info.focus_distance[FOCUS_DISTANCE_FAR_INDEX] = 0.17;
    CDBG_AF("%s: Return fixed focus distance for this sensor\n", __func__);
    return focus_dist_info;
  }

  coc = 2.0 * f_pix;
  hyperD = f_len * f_len / (f_num * coc) * 1000.0;;
  back_focus = 5 + ((f_dist / num_steps) * (num_steps - af->cur_pos + 1));
  back_focus = (back_focus +  f_len * 1000.0) / 1000.0;
  opt_focus_dist = 1.0 / ((1.0 / f_len) - (1.0 / back_focus));

  near_focus_dist = (hyperD * opt_focus_dist) /
    (hyperD + (opt_focus_dist - f_len));

  far_focus_dist = (hyperD * opt_focus_dist) /
    (hyperD-(opt_focus_dist - f_len));

  focus_dist_info.focus_distance[FOCUS_DISTANCE_NEAR_INDEX] =
    near_focus_dist / 1000;  /* in meters */

  focus_dist_info.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] =
    opt_focus_dist / 1000;  /* in meters */

  focus_dist_info.focus_distance[FOCUS_DISTANCE_FAR_INDEX] =
    far_focus_dist / 1000;  /* in meters */

  if(focus_dist_info.focus_distance[FOCUS_DISTANCE_FAR_INDEX] < 0
     || focus_dist_info.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] < 0
     || focus_dist_info.focus_distance[FOCUS_DISTANCE_NEAR_INDEX] < 0 )
  {
    focus_dist_info.focus_distance[FOCUS_DISTANCE_NEAR_INDEX] = 0.10;
    focus_dist_info.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] = 0.15;
    focus_dist_info.focus_distance[FOCUS_DISTANCE_FAR_INDEX] = 0.17;
  }

  CDBG_AF("%s:LP %d, FD %f, NF %f, FF %f", __func__, af->cur_pos,
    opt_focus_dist / 1000, near_focus_dist / 1000, far_focus_dist / 1000);
  return focus_dist_info;
} /* af_get_focus_distance */

/*===========================================================================
 * FUNCTION    - af_panning_stable_check -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_panning_stable_check(stats_proc_t *sproc, af_t *af)
{
  int tmp_fv = 0, ave_fv = 0, max_fv = 0, min_fv = 0, cur_fv = 0, pre_fv = 0;
  int i, num_Samples = 5, rc = 0;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  for (i = 1; i <= num_Samples; i++) {
    if (af->caf.panning_index - i < 1)
      tmp_fv = af->caf.panning_track[af->caf.panning_index - i + 9];
    else
      tmp_fv = af->caf.panning_track[af->caf.panning_index - i];

    if (i == 1)
      cur_fv = tmp_fv;
    else if (i == 2)
      pre_fv = tmp_fv;
    else if (abs(cur_fv-pre_fv) *
      fptr->af_CAF.af_panning_stable_fv_change_trigger > pre_fv) {
      CDBG_AF("%s: Return -1", __func__);
      af->stable_cnt = 0;
      return -1;
    }
    if (tmp_fv > max_fv)
      max_fv = tmp_fv;
    else if (tmp_fv < min_fv)
      min_fv = tmp_fv;

    ave_fv += tmp_fv;
  }
  ave_fv = (int32_t)(ave_fv/num_Samples);
  if (abs(af->stats[af->index - 1] - ave_fv) *
    fptr->af_CAF.af_panning_stable_fvavg_to_fv_change_trigger > ave_fv) {
    af->stable_cnt = 0;
    rc = -1;
  } else {
    if (af->stable_cnt > fptr->af_CAF.af_scene_change_trigger_cnt)
      rc = 0;
    else {
      af->stable_cnt++;
      rc = -1;
    }
  }
  CDBG_AF("%s: Return val  %d\n", __func__, rc);
  return rc;
} /* af_panning_stable_check */

/*===========================================================================
 * FUNCTION    - af_slow_move_lens -
 *
 * DESCRIPTION:
 *==========================================================================*/
void af_slow_move_lens(stats_proc_t *sproc, af_t *af, int dir, int steps)
{
  int allowed_steps = (int)af->caf.fov_allowed_steps;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  int baseDelay = 0;

  if (sproc->share.af_ext.cont_af_enabled)
    baseDelay = fptr->af_CAF.af_cont_base_frame_delay;
  else
    baseDelay = fptr->basedelay_snapshot_AF;

  if (af->state != AF_MOVING_LENS)
    af->previous_state = af->state;

  if (steps > allowed_steps) {
    af_safe_move_lens(sproc, af, dir, &allowed_steps);
    af->state = AF_MOVING_LENS;
    sproc->share.af_ext.num_of_steps = allowed_steps;
  } else {
    af_safe_move_lens(sproc, af, dir, &steps);
    af->state = af->previous_state;
    sproc->share.af_ext.num_of_steps = steps;
    af->frame_delay = baseDelay;
  }
  sproc->share.af_ext.move_lens_status = TRUE;
  sproc->share.af_ext.direction = dir;
} /* af_slow_move_lens */

/*==========================================================================
 * FUNCTION    - af_set_bestshot_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_set_bestshot_mode(stats_proc_t *sproc, af_t *af,
  camera_bestshot_mode_type new_mode)
{
  int rc = 0;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  if (new_mode  >= CAMERA_BESTSHOT_MAX)
    return -1;

  if (af->bestshot_d.curr_mode == new_mode)
    return 0; /* Do Nothing */

  /* Store current AEC vals */
  if (af->bestshot_d.curr_mode == CAMERA_BESTSHOT_OFF) {
    af->bestshot_d.srch_mode = af->srch_mode;
    af->bestshot_d.soft_focus_degree = af->soft_focus_degree;
  }
  /* CONFIG AEC for BESTHOT mode */
  if (new_mode != CAMERA_BESTSHOT_OFF) {
    switch (new_mode) {
    case CAMERA_BESTSHOT_SNOW:
    case CAMERA_BESTSHOT_BEACH:
    case CAMERA_BESTSHOT_NIGHT:
    case CAMERA_BESTSHOT_PORTRAIT:
    case CAMERA_BESTSHOT_BACKLIGHT:
    case CAMERA_BESTSHOT_CANDLELIGHT:
    case CAMERA_BESTSHOT_PARTY:
      af->srch_mode = AF_MODE_NORMAL;
      sproc->share.af_ext.reset_lens = FALSE;
      break;
    case CAMERA_BESTSHOT_THEATRE:
    case CAMERA_BESTSHOT_SPORTS:
    case CAMERA_BESTSHOT_ANTISHAKE:
    case CAMERA_BESTSHOT_FIREWORKS:
    case CAMERA_BESTSHOT_ACTION:
      af->srch_mode = AF_MODE_AUTO;
      sproc->share.af_ext.reset_lens = FALSE;
      break;
    case CAMERA_BESTSHOT_SUNSET:
    case CAMERA_BESTSHOT_LANDSCAPE:
      af->srch_mode = AF_MODE_INFINITY;
      sproc->share.af_ext.reset_lens = TRUE;
      break;
    case CAMERA_BESTSHOT_FLOWERS:
      af->srch_mode = AF_MODE_MACRO;
      sproc->share.af_ext.reset_lens = FALSE;
      break;
    case CAMERA_BESTSHOT_OFF:
    case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
    case CAMERA_BESTSHOT_AR:
    default:
      af->srch_mode = AF_MODE_AUTO;
      sproc->share.af_ext.reset_lens = FALSE;
      break;
    }

    switch (new_mode) {
    case CAMERA_BESTSHOT_PORTRAIT:
      af->soft_focus_degree = cptr->soft_focus_degree;
      break;
    case CAMERA_BESTSHOT_LANDSCAPE:
    case CAMERA_BESTSHOT_SNOW:
    case CAMERA_BESTSHOT_BEACH:
    case CAMERA_BESTSHOT_SUNSET:
    case CAMERA_BESTSHOT_OFF:
    case CAMERA_BESTSHOT_NIGHT:
    case CAMERA_BESTSHOT_BACKLIGHT:
    case CAMERA_BESTSHOT_SPORTS:
    case CAMERA_BESTSHOT_ANTISHAKE:
    case CAMERA_BESTSHOT_FLOWERS:
    case CAMERA_BESTSHOT_CANDLELIGHT:
    case CAMERA_BESTSHOT_FIREWORKS:
    case CAMERA_BESTSHOT_PARTY:
    case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
    case CAMERA_BESTSHOT_THEATRE:
    case CAMERA_BESTSHOT_ACTION:
    case CAMERA_BESTSHOT_AR:
    default:
      af->soft_focus_degree = 1.0;
    }
  } else { /* Restore AF vals */
    af->srch_mode = af->bestshot_d.srch_mode;
    af->soft_focus_degree = af->bestshot_d.soft_focus_degree;
  }
  af->bestshot_d.curr_mode = new_mode;
  return rc;
} /* af_set_bestshot_mode */





/*==========================================================================
 * FUNCTION    - af_set_bestshot_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
static uint32_t af_stats_get_multi_roi_window(stats_proc_t *sproc)
{
  int rc = TRUE;

  stats_proc_roi_info_t *roi_info = &sproc->share.af_ext.roiInfo;
  int num_roi = roi_info->num_roi;
  int32_t i, j, k, cnt;
  int32_t ii, jj;
  int32_t grid_cnt_per_roi[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS];
  roi_t grids[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS];
  int32_t total_grid_cnt_old;
  int32_t total_grid_cnt = 0;

  /* If number of regions is more than the AF window maximum,clamp it */
  if (num_roi > NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS)
    num_roi = NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS;

  /* Count how many grids each ROI contains */
  for (i = 0; i < num_roi; i++) {
    grids[i].x  = roi_info->roi[i].x *
      NUM_AUTOFOCUS_HORIZONTAL_GRID / roi_info->frm_width;
    grids[i].y  = roi_info->roi[i].y   *
      NUM_AUTOFOCUS_VERTICAL_GRID / roi_info->frm_height;
    grids[i].dx = (roi_info->roi[i].dx *
      NUM_AUTOFOCUS_HORIZONTAL_GRID / roi_info->frm_width) + 1;
    grids[i].dy = (roi_info->roi[i].dy *
      NUM_AUTOFOCUS_VERTICAL_GRID / roi_info->frm_height) + 1;

    if (grids[i].x + grids[i].dx >= NUM_AUTOFOCUS_HORIZONTAL_GRID)
      grids[i].dx = NUM_AUTOFOCUS_HORIZONTAL_GRID - grids[i].x - 1;
    if (grids[i].y + grids[i].dy >= NUM_AUTOFOCUS_VERTICAL_GRID)
      grids[i].dy = NUM_AUTOFOCUS_VERTICAL_GRID - grids[i].y - 1;

    grid_cnt_per_roi[i] = grids[i].dx * grids[i].dy;
    total_grid_cnt += grid_cnt_per_roi[i];
    CDBG("%s: grid_cnt_per_roi[%d] has %d grids", __func__,
      i, grid_cnt_per_roi[i]);
  }
  total_grid_cnt_old = total_grid_cnt;

  /* Now determine the number of representatives from each ROI by
   * normalization
   */
  if (total_grid_cnt_old > NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS) {
    total_grid_cnt = 0;
    for (i = 0; i < num_roi; i++) {
      grid_cnt_per_roi[i] = grid_cnt_per_roi[i] *
      NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS / total_grid_cnt_old;
      if (grid_cnt_per_roi[i] == 0)
        grid_cnt_per_roi[i] = 1;
      total_grid_cnt += grid_cnt_per_roi[i];
    }
    i = 0;
    /* Adjust the representative after normalization */
    while (total_grid_cnt < NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS) {
      grid_cnt_per_roi[i]++;
      total_grid_cnt++;
      i = (i + 1) % num_roi;
    }
    while (total_grid_cnt > NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS) {
      if (grid_cnt_per_roi[i] > 1) {
        grid_cnt_per_roi[i]--;
        total_grid_cnt--;
      }
      i = (i + 1) % num_roi;
    }
  }

  /* At this point, the total number of representative grids should be
   * less than NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS */
  cnt = 0;
  for (i = 0; i < num_roi; i++) {
    float f_step = (float)(grids[i].dx * grids[i].dy) /
      (float)(grid_cnt_per_roi[i] + 1);
    for (j = 0; j < grid_cnt_per_roi[i]; j++) {
      int32_t tmp = (int32_t)(f_step * (float)(j+1));
      ii = grids[i].y + (tmp / grids[i].dx);
      jj = grids[i].x + (tmp % grids[i].dx);
      af_multi_roi_windows[cnt] =
        ii * NUM_AUTOFOCUS_HORIZONTAL_GRID + jj;
      CDBG("%s:af multi roi windows[%d]=%d ii,jj = %d,%d",
        __func__, cnt, af_multi_roi_windows[cnt], ii, jj);
      cnt++;
    }
  }

  sproc->share.af_ext.af_multi_nfocus = cnt;
  return TRUE;
}


/*==========================================================================
 * FUNCTION    - af_get_stats_config_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_get_stats_config_mode(stats_proc_t *sproc)
{
  stats_proc_roi_info_t *roi_info = NULL;
  sproc->share.af_ext.af_multi_roi_window = NULL;

  /* If Touch-AF is enabled along with face-detection,
     touch-af will have higher priority */
  if (sproc->share.af_ext.roiInfo.num_roi) {
    roi_info = &sproc->share.af_ext.roiInfo;
  }
  else if(sproc->share.af_ext.fdInfo.num_roi) {
    roi_info = &sproc->share.af_ext.fdInfo;
  }

  /* Update common variable to store AF ROIs */
  if (roi_info != NULL) {
    memcpy(sproc->share.af_ext.af_roi,
      roi_info->roi, sizeof(roi_t) * MAX_ROI);
  }

  switch (sproc->share.af_ext.cur_focusrect_value) {
    case AUTO:
      if (roi_info != NULL && roi_info->num_roi == 1) {
        CDBG_AF("%s: Single ROI !!!", __func__);

        /* Check if region sizes are same as frame sizes. If yes we'll use
          default mode. Also if both are zero, we'll use default mode. */
        if ((((roi_info->roi[0].x + roi_info->roi[0].dx) >=
          roi_info->frm_width) &&
          ((roi_info->roi[0].y + roi_info->roi[0].dy) >=
          roi_info->frm_width)) ||
          (!(roi_info->roi[0].x + roi_info->roi[0].dx) &&
          !(roi_info->roi[0].y + roi_info->roi[0].dy))) {
          sproc->share.af_ext.af_stats_config_mode =
            AF_STATS_CONFIG_MODE_DEFAULT;
        }
        else {
          sproc->share.af_ext.af_stats_config_mode =
            AF_STATS_CONFIG_MODE_SINGLE;
        }
      }
      else if (roi_info != NULL && roi_info->num_roi >= 2) {
        CDBG_AF("%s: Multiple ROIs !!!", __func__);
        sproc->share.af_ext.af_multi_nfocus =
          af_stats_get_multi_roi_window(sproc);
        sproc->share.af_ext.af_multi_roi_window = af_multi_roi_windows;
        sproc->share.af_ext.af_stats_config_mode =
          AF_STATS_CONFIG_MODE_MULTIPLE;
      }
      else {
        CDBG_AF("%s: Default AF Stats Mode!!!", __func__);
        sproc->share.af_ext.af_stats_config_mode = AF_STATS_CONFIG_MODE_DEFAULT;
      }
      break;

    case SPOT:
      CDBG_AF("%s: SPOT Metering mode!!!", __func__);
      sproc->share.af_ext.af_multi_roi_window = af_spot_windows;
      sproc->share.af_ext.af_stats_config_mode =
        AF_STATS_CONFIG_MODE_MULTIPLE;
      sproc->share.af_ext.af_multi_nfocus = NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS;
      break;

    case AVERAGE:
      CDBG_AF("%s: Average Metering mode!!!", __func__);
      sproc->share.af_ext.af_multi_roi_window = af_average_windows;
      sproc->share.af_ext.af_stats_config_mode = AF_STATS_CONFIG_MODE_MULTIPLE;
      sproc->share.af_ext.af_multi_nfocus = NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS;
      break;

    case CENTER_WEIGHTED:
      CDBG_AF("%s: Center Weighted Metering mode!!!", __func__);
      sproc->share.af_ext.af_multi_roi_window = af_center_weighted_windows;
      sproc->share.af_ext.af_stats_config_mode =
        AF_STATS_CONFIG_MODE_MULTIPLE;
      sproc->share.af_ext.af_multi_nfocus = NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS;
      break;

    default:
      CDBG_ERROR("%s: Invalid focus rectangle value(%d)."
        "Resetting to default mode", __func__,
        sproc->share.af_ext.cur_focusrect_value);
      sproc->share.af_ext.af_stats_config_mode =
        AF_STATS_CONFIG_MODE_DEFAULT;
      break;
  }

  return TRUE;
}
