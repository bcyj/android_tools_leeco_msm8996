/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "stats_proc.h"
#include "af.h"

#define CONTAF_LOW_LIGHT_WAIT 0
/*===========================================================================
 * FUNCTION    - af_exhstive_coarse_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int af_exhstive_coarse_srch(stats_proc_t *sproc, af_t *af)
{
  int gross_stat_pts;
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;


  if (sproc->input.sensor_info.preview_fps >> 8 <= 8) {
    gross_stat_pts = fptr->num_gross_steps_between_stat_points * 3 / 2;
  } else {
    gross_stat_pts = fptr->num_gross_steps_between_stat_points;
  }

  /*  Keep same state, take no action just decrment frame delay count.
   *  Thought is that the stats for this frame are not trustworthy
   *  so wait until they are. */
  if (af->frame_delay > 0) {
    if (af->index == 0) {
      CDBG_AF("%s:COARSE MODE, Frame Delay %d, %d, %d\n", __func__, af->
        frame_delay,
        (stats->af_op.Focus / stats->af_op.NFocus / MAX(sproc->share.cur_af_luma, 1)),
        af->cur_pos);
    }
    af->frame_delay--;
    return 0;
  }
  if (af->index < AF_COLLECTION_POINTS) {
    /*  Store 27 bit focus value divided by num valid rows to get an average */
    af->stats[af->index++] = (int) (stats->af_op.Focus / stats->af_op.NFocus /
      MAX(sproc->share.cur_af_luma, 1));

    CDBG_AF("%s:Index, Focusval %d, LensPosition %d, %d\n", __func__,
      af->index - 1,
      (stats->af_op.Focus / stats->af_op.NFocus / MAX(sproc->share.cur_af_luma, 1)),
      af->cur_pos);

    if (af->max_focus_val < af->stats[af->index - 1]) {
      af->max_focus_val = af->stats[af->index - 1];
      af->max_pos       = af->cur_pos;
      af->num_downhill  = 0;
      if (af->index == 1)
        af->min_focus_val = af->max_focus_val;
    } else { /* To include minFV */
      if (af->min_focus_val > af->stats[af->index - 1])
        af->min_focus_val = af->stats[af->index - 1];

      if (af->stats[af->index - 1] <= af->stats[af->index - 2])
        af->num_downhill++;
      else
        af->num_downhill = 0;
    }
  }
  /*  Is there room to move the sensor again? */
  if (((af->cur_pos - gross_stat_pts) >= af->near_end) && !((af->
    num_downhill >= af->downhill_allowance) && (af->stats[af->
    index - 1] < af->max_focus_val * af->fv_drop_allowance))) {
    af_move_lens(sproc, af, MOVE_NEAR, gross_stat_pts);
    af->exhstive.move_flag = 1;
  }
  /* Traversed the whole focal range & gathered focal stats at each pt*/
  /*  If so we are at the nearest focal position */
  if ((((af->cur_pos - gross_stat_pts) <= af->near_end) &&
    (af->exhstive.move_flag == 0)) || ((af->num_downhill >= af->
    downhill_allowance) && (af->stats[af->index - 1] < af->
    max_focus_val * af->fv_drop_allowance))) {

    int maxindex, localmax, i, moveBackSteps;
    /*  point to last true data value */
    af->index--;
    maxindex = localmax = 0;
    for (i = 0; i <= af->index; i++)
      if (af->stats[i] > localmax) {
        localmax = af->stats[i];
        maxindex = i;
      }
    moveBackSteps = (af->index - maxindex) * gross_stat_pts;
    /* REVISIT to check FV_min/FV_max */
    if ((af->max_focus_val <
      (int)((MODE_CHANGE_THRESH + 200) / MAX(sproc->share.cur_af_luma, 1))) ||
      (af->min_focus_val > ((float) af->max_focus_val *
      CONFIDENCE_LEVEL_EXH))) {
      /* FV_min & FV_max are too close, stay at maximum position & quit, no
       *need to do fine srch, the case is either noisy, or low contrast */
      CDBG_AF("%s:No need fine search focus val_max %d focus val_min %d \n",
        __func__, af->max_focus_val, af->min_focus_val);
      if (moveBackSteps > 0) {
        af->state = AF_UNABLE_TO_FOCUS;
        af_move_lens(sproc, af, MOVE_FAR, moveBackSteps);
        af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
      } else {
        if (af->collect_end_stat)
          af->state = AF_COLLECT_END_STAT;
        else
          af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
      }
      return -1;
    }
    /* Accommodate #Fine search here */
    if (moveBackSteps >= (af->exhstive.fine_focal_pos >> 1)) {
      /*  Position start of search in center between the max recorded point */
      if (af->frame_skip && (maxindex > 0))
        moveBackSteps -= (af->exhstive.fine_focal_pos >> 1) - gross_stat_pts;
      else
        moveBackSteps -= af->exhstive.fine_focal_pos  >> 1;
      /*  Select bin for finer search */
      if (af->frame_skip)
        af->lens_state_changed = TRUE;
      af_move_lens(sproc, af, MOVE_FAR, moveBackSteps);
    } else { /*  Select bin for finer search */
      if (af->frame_skip)
        af->lens_state_changed = TRUE;
      moveBackSteps = af->cur_pos - af->near_end;
      af_move_lens(sproc, af, MOVE_NEAR, moveBackSteps);
      /*  REVISIT: depending on the num steps, may need to wait longer
       *  between next stat process */
    }
    /* Change state  */
    CDBG_AF("FOR DEBUG PURPOSE HERE CHANGE to FINESEARCH %d, %d, %d",
      af->max_focus_val, af->min_focus_val, af->cur_pos);

    af->state                = AF_GATHER_STATS_FINE;
    sproc->share.af_ext.direction = MOVE_FAR;
    af->max_focus_val        = 0;
    af->index                = 0;
    af->num_downhill         = 0;
  }
  return 0;
} /* af_exhstive_coarse_srch */

/*===========================================================================
 * FUNCTION    - af_exhstive_fine_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int af_exhstive_fine_srch(stats_proc_t *sproc, af_t *af)
{
  int ok_to_move, fine_stat_pts, stop_condition;
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  CDBG_AF("AF_GATHER_STATS_FINE: frame_delay %d", af->frame_delay);
  if (af->frame_delay > 0) {
    if (af->index == 0) {
      CDBG_AF("Frame Delays, FINE MODE,  and Nlines %d, %d, %d\n",
        af->frame_delay,
        (stats->af_op.Focus / stats->af_op.NFocus  / MAX(sproc->share.cur_af_luma, 1)),
        af->cur_pos);
    }
    af->frame_delay--;
    return 0;
  }
  CDBG_AF("Index, Focusvalue, and LensPosition %d, %d, %d\n", af->
    index, stats->af_op.Focus / stats->af_op.NFocus / MAX(sproc->share.cur_af_luma, 1),
    af->cur_pos);
  if (sproc->input.sensor_info.preview_fps >> 8 <= 8)
    fine_stat_pts  = fptr->num_fine_steps_between_stat_points * 2;
  else
    fine_stat_pts  = fptr->num_fine_steps_between_stat_points;

  if (af->index < AF_COLLECTION_POINTS) {
    /*  Store 27 bit focus value divided by num valid rows to get an average */
    af->stats[af->index++] = (int) (stats->af_op.Focus / stats->af_op.NFocus /
      MAX(sproc->share.cur_af_luma, 1));

    /*  increse index */
    if (af->max_focus_val < af->stats[af->index - 1]) {
      af->max_focus_val = af->stats[af->index - 1];
      af->max_pos       = af->cur_pos;
      af->num_downhill  = 0;
      af->num_uphill++;  /*  update uphill indicator */

      if (af->index == 1)
        af->min_focus_val = af->max_focus_val;
    } else { /* To include minFV */
      if (af->min_focus_val >= af->stats[af->index - 1])
        af->min_focus_val = af->stats[af->index - 1];

      if (af->stats[af->index - 1] <=
        af->stats[af->index - 2]) {
        af->num_downhill++;
        af->num_uphill = 0;  /*  clear uphill indicator */
      } else {
        af->num_downhill = 0;
        af->num_uphill++;  /*  update uphill indicator */
      }
    }
  }
  if (fptr->undershoot_protect) {
    /* undershoot condition is met if
     * 1) at far end or near end in macro mode
     * 2) at max fv
     * 3) has strong increasing tendency */
    if (((af->cur_pos == af->near_end && af->srch_mode ==
      AF_MODE_MACRO) || af->cur_pos == af->far_end) && af->
      cur_focus_val == af->max_focus_val && af->num_uphill >
      af->uphill_threshold) {
      if (af->cur_pos == af->near_end) {
        af->cur_pos = af->cur_pos + fptr->undershoot_adjust;
        CDBG_AF("%s:undershoot detected change pos to %d\n",
          __func__, af->cur_pos);
        /*  update position_max as well */
        af->max_pos = af->max_pos + fptr->undershoot_adjust;
      } else if (af->cur_pos == af->far_end) {
        af->cur_pos = af->cur_pos - fptr->undershoot_adjust;
        /*  update position_max as well */
        af->max_pos = af->max_pos - fptr->undershoot_adjust;
      }
      af->num_uphill = 0;  /*  clear uphill indicator */
    }
  }
  /*  Is there need to move the sensor again, or have we moved enough? */
  ok_to_move = ((sproc->share.af_ext.direction == MOVE_FAR && (af->
    cur_pos + fine_stat_pts <= af->far_end)) || (sproc->share.af_ext.
    direction == MOVE_NEAR && (af->cur_pos - fine_stat_pts >= af->
    near_end))) && (af->num_downhill < af->downhill_allowance);

  if (af->srch_mode != AF_MODE_NORMAL)
    ok_to_move = ok_to_move && (af->index < af->exhstive.fine_focal_pos);

  if (ok_to_move) {
    af_move_lens(sproc, af, sproc->share.af_ext.direction, fine_stat_pts);
    af->exhstive.move_flag = 1;
  }

  stop_condition = (((af->cur_pos >= af->far_end) || (af->
    cur_pos <= af->near_end)) && (af->exhstive.move_flag == 0));

  if (af->srch_mode == AF_MODE_NORMAL) {
    stop_condition =  stop_condition || (af->
      num_downhill >= af->downhill_allowance);
  } else {
    stop_condition = stop_condition || ((af->index >= af->exhstive.
      fine_focal_pos) && (!fptr->undershoot_protect)) || ((af->index >=
      af->exhstive.fine_focal_pos + fptr->undershoot_adjust) && (fptr->
      undershoot_protect)) || (af->num_downhill >= af->downhill_allowance);
  }
  CDBG_AF("%s:stop_condition %d", __func__, stop_condition);
  if (stop_condition) {
    int moveBackSteps, stats_not_reliable = FALSE;
    /*  Select bin for finer search */
    /*  To accommodate two-modes */
    /*  first to see if the min and max FV are meaningful
     *  in the NORMAL mode only  */
    if ((af->srch_mode == AF_MODE_NORMAL) && ((af->max_focus_val <
      MODE_CHANGE_THRESH + 200) || (af->min_focus_val > ((float) af->
      max_focus_val * CONFIDENCE_LEVEL_EXH)))) {
      /* FV_min and FV_max are too close stay at the max position, & quit */
      /* Need to report to UI that AF is low confident stats_not_reliable = TRUE; */
      CDBG_AF("%s:Warning FVs not reliable, Move to Max loc & quit %d, %d,"
        "%d", __func__, af->max_focus_val, af->min_focus_val,
        af->cur_pos);
    }
    moveBackSteps = af->cur_pos - af->max_pos;
    if (af->frame_skip)
      if ((moveBackSteps != 0) && ((af->index - 1) *
        fine_stat_pts > abs(moveBackSteps)))
        moveBackSteps += (moveBackSteps > 0) ? fine_stat_pts : -fine_stat_pts;

    if (moveBackSteps > 0) {
      af->state = AF_FOCUSING;
      af_move_lens(sproc, af, MOVE_NEAR, moveBackSteps);
    } else if (moveBackSteps < 0) {
      af->state = AF_FOCUSING;
      af_move_lens(sproc, af, MOVE_FAR, -moveBackSteps);
    } else { /*  we are done */
      if (af->collect_end_stat) /* state = AF_COLLECT_END_STAT; */
        af->state = AF_COLLECT_END_STAT;
      else {
        if (stats_not_reliable) { /*  AF Completed unsuccessfully */
          af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
          return -1;
        } else /*  AF Completed successfully */
          af_done(sproc, af, CAMERA_EXIT_CB_DONE);
      }
    }
  }
  return 0;
} /* af_exhstive_fine_srch */

/*===========================================================================
 * FUNCTION    - af_exhstive_start_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int af_exhstive_start_srch(stats_proc_t *sproc, af_t *af)
{
  int steps_to_move;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  af->frame_delay = 0;
  af->lens_moved = 0;
  af->caf.max_luma = sproc->share.cur_af_luma;
  CDBG_AF("%s:AF_START type %d, mode %d, NE %d, FE %d", __func__,  af->
    algo_type, af->srch_mode, af->near_end, af->far_end);

  /*  Get the sensor to a known position */
  if (af->cur_pos <= af->far_end) {
    if ((fptr->undershoot_protect) && (af->far_end == (int)
      fptr->position_far_end)) {
      CDBG_AF("%s:orig position is %d\n", __func__, af->cur_pos);
      steps_to_move = (af->far_end - af->cur_pos) * 8 / 5;
      /*  at least 2 steps */
      steps_to_move = (steps_to_move >= 2) ? steps_to_move : 2;
      if (steps_to_move > af->far_end) /*  don't want negative pos */
        steps_to_move = af->far_end;

      af->cur_pos = af->far_end - steps_to_move;
    } else {
      steps_to_move = af->far_end - af->cur_pos;
      CDBG_AF("%s: changed position to %d\n", __func__, af->cur_pos);
    }
    /* only need to set this flag if move lens */
    if (steps_to_move > 0) {
      af->lens_state_changed = TRUE;
      af_move_lens(sproc, af, MOVE_FAR, steps_to_move);
      af->exhstive.move_flag = 1;
    }
  } else if (af->cur_pos > af->far_end) {
    CDBG_AF("%s:Lens Pos %d > than default_focus %d\n", __func__,
      af->cur_pos, af->default_focus);
    /*  Consider the current position not trustworthy */
    af->cur_pos = 0;
    /* only need to set this flag if move lens */
    af->lens_state_changed = TRUE;
    af_move_lens(sproc, af, MOVE_FAR, af->far_end - af->cur_pos);
    af->exhstive.move_flag = 1;
  }
  CDBG_AF("%s:search_mode %d\n", __func__, af->srch_mode);
  if (af->srch_mode == AF_MODE_NORMAL)
    af->state = AF_GATHER_STATS_FINE;
  else
    af->state = AF_GATHER_STATS_COARSE;

  sproc->share.af_ext.direction = MOVE_NEAR;
  af->index = 0;
  af->max_focus_val = 0;
  af->min_focus_val = 0;
  af->num_downhill = 0;
  af->num_uphill = 0;
  af->start_lens_pos = af->infy_pos;

  if (af->exhstive.move_flag == 1) {
    CDBG_AF("%s: Frame Delay after init %d, %d, %d\n", __func__, af->
      frame_delay,
      (stats->af_op.Focus / stats->af_op.NFocus  / MAX(sproc->share.cur_af_luma, 1)),
      af->cur_pos);
    return -1;
  }
  return 0;
} /* af_exhstive_start_srch */

/*===========================================================================
 * FUNCTION    - af_exhaustive_search -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_exhaustive_search(stats_proc_t *sproc, af_t *af)
{
  int rc = 0;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  af->exhstive.move_flag = 0;

  if (af->lens_move_done == FALSE) {
    af->frame_delay--;
    if (af->frame_delay < -LENS_DONE_MOVE_THRESH)
      af->lens_move_done = TRUE;
    return 0;
  }
  if (sproc->input.sensor_info.current_fps >> 8 <= 10) {
    af->exhstive.fine_focal_pos = fptr->num_fine_search_points / 2;
    af->in_low_light = TRUE;
  } else {
    af->exhstive.fine_focal_pos = fptr->num_fine_search_points;
    af->in_low_light = FALSE;
  }
  if (stats == NULL || af->step_fail_cnt > FOCUS_ATTEMPTS) {
    if (af->step_fail_cnt > FOCUS_ATTEMPTS)
      af->state = AF_ERROR;
    else
      af->state = AF_INACTIVE;
    sproc->share.af_ext.active = FALSE;
    af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
    return -1;
  }
  CDBG_AF("%s:state %d curpos %d", __func__, af->state, af->cur_pos);

  /* If CAF and touch-AF are enabled and we just finished focusing
   using touch-AF, we'll directly go to Monitor starte rather than
   doing full sweep again */
  if (af->state == AF_START && sproc->share.af_ext.cont_af_enabled &&
    (sproc->share.af_ext.touch_af_enabled ||
    af->caf.ignore_full_sweep)) {
    sproc->share.af_ext.touch_af_enabled = FALSE;
    af->caf.clean_panning_track = TRUE;
    af->caf.panning_unstable_cnt    = 0;
    af->caf.ignore_full_sweep = TRUE;
    af->caf.exp_index_before_change = sproc->share.aec_ext.exp_index;
    af->state                   = AF_GATHER_STATS_CONT_MONITOR;
    af->frame_delay             = fptr->af_CAF.af_cont_base_frame_delay;

    /* Reset stale gyro information */
    memset(&(af->caf.gyro_data), 0, sizeof(af->caf.gyro_data));
    CDBG_AF("%s: Entering CAF monitoring state....\n", __func__);
  }

  if (af->state == AF_START)
    if (af_exhstive_start_srch(sproc, af) < 0)
      return 0;

  if (af->state == AF_GATHER_STATS_COARSE)
    rc = af_exhstive_coarse_srch(sproc, af);
  else if (af->state == AF_GATHER_STATS_FINE)
    rc = af_exhstive_fine_srch(sproc, af);
  else if (af->state == AF_COLLECT_END_STAT) {
    if (af->frame_delay > -2) {
      CDBG_AF("%s:Frame Delays, Focusvalue, & Nlines %d, %d, %d", __func__,
        af->frame_delay,
        stats->af_op.Focus / stats->af_op.NFocus / MAX(sproc->share.cur_af_luma, 1),
        af->cur_pos);
      af->frame_delay--;
      return 0;
    }
    /*  Store 27 bit focus value divided by num valid rows to get an average */
    af->locn[af->index] = af->cur_pos;
    af->stats[af->index++] = (stats->af_op.Focus / stats->af_op.NFocus  /
      MAX(sproc->share.cur_af_luma, 1));
    CDBG_AF("%s:Index, Focusvalue, & LensPosition %d, %d, %d", __func__,
      af->index - 1,
      stats->af_op.Focus / stats->af_op.NFocus / MAX(sproc->share.cur_af_luma, 1),
      af->cur_pos);
    CDBG_AF("%s:Took AF %d moves to focus", __func__, af->lens_moved);
    /* AF Completed successfully */
    af_done(sproc, af, CAMERA_EXIT_CB_DONE);
  } else if (sproc->share.af_ext.cont_af_enabled) /* Conti AF srch */
    rc = af_continuous_search(sproc, af);

  return rc;
} /* af_exhaustive_search */

/*===========================================================================
* FUNCTION    - af_CAF_monitor -
*
* DESCRIPTION:
*==========================================================================*/
static void af_move_lens_for_trying_step(stats_proc_t *sproc, af_t *af)
{
  if (sproc->share.af_ext.direction == MOVE_NEAR)
    af_move_lens(sproc, af, MOVE_NEAR, MIN(af->caf.tryingstep,
      af->cur_pos - af->near_end));
  else
    af_move_lens(sproc, af, MOVE_FAR, MIN(af->caf.tryingstep,
      af->far_end - af->cur_pos));
} /* af_move_lens_for_trying_step */


/*===========================================================================
* FUNCTION    - af_cont_initiate_caf -
*
* DESCRIPTION:
*==========================================================================*/
static void af_cont_initiate_caf(stats_proc_t *sproc, af_t *af)
{
  af_continuous_t *caf = &(af->caf);
  af_cont_gyro_t *gyro_data = &(caf->gyro_data);
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  CDBG_AF("%s:Alert,focus val changed: prev max, newFV luma cur %d, %d, %d",
    __func__, af->max_focus_val, stats->af_op.Focus / stats->af_op.NFocus / MAX(caf->
    cur_luma, 1), caf->cur_luma);

  /* Initialize a few variables */
  caf->exp_index_before_change = sproc->share.aec_ext.exp_index;
  af->caf.panning_unstable_cnt = 0;
  af->index                    = 0;
  af->num_downhill             = 0;
  caf->trial_in_noise1         = 0;
  caf->frame_cnt               = 0;
  caf->max_luma                = 0;
  caf->no_of_indecision        = 0;
  caf->sad_data.set_ref_luma_sum = 1;
  af->start_lens_pos           = af->cur_pos;

  if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER)
  {
    if (af->in_low_light) {/* Small stp in macro rng, Bigg stp infy rng */
      caf->smaller_tryingstep = fptr->af_CAF.af_cont_search_step_size;
      caf->bigger_tryingstep = fptr->af_CAF.af_cont_search_step_size * 2;
      caf->fov_allowed_steps = 2;
    } else {
      caf->smaller_tryingstep = fptr->af_CAF.af_cont_search_step_size;
      caf->bigger_tryingstep = fptr->af_CAF.af_cont_search_step_size;
      caf->fov_allowed_steps = 1;
    }
  }
  else
  {
    if (af->in_low_light) {/* Small stp in macro rng, Bigg stp infy rng */
      caf->smaller_tryingstep = fptr->af_CAF.af_cont_search_step_size * 2;
      caf->bigger_tryingstep = fptr->af_CAF.af_cont_search_step_size * 2;
      caf->fov_allowed_steps = 2;
    } else {
      caf->smaller_tryingstep = fptr->af_CAF.af_cont_search_step_size;
      caf->bigger_tryingstep = fptr->af_CAF.af_cont_search_step_size;
      caf->fov_allowed_steps = 1;
    }
  }

  af->locn[af->index] = af->cur_pos;
  af->stats[af->index++] = af->cur_focus_val;
  CDBG_HIGH("%s: IDX %d, prev_FV %d, FV %d, Cur_Pos %d, NEAR End %d, FAR End"
    " %d", __func__, af->index - 1, af->max_focus_val, af->cur_focus_val,
    af->cur_pos, af->far_end, af->near_end);

  af->max_focus_val = af->stats[af->index - 1];
  af->min_focus_val = af->stats[af->index - 1];
  af->max_pos       = af->cur_pos;

  caf->trig_refocus = FALSE;
  if (af->cur_pos < af->far_end &&
    af->cur_pos > af->near_end) {
    caf->tryingstep = (af->far_end - af->cur_pos) < (af->
      far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
    af_move_lens_for_trying_step(sproc, af);
    caf->status = CAF_FOCUSING;
    af->state = AF_MAKE_DECISION;
  } else {
    if (af->cur_pos == af->far_end)
      sproc->share.af_ext.direction = MOVE_NEAR;
    else
      sproc->share.af_ext.direction = MOVE_FAR;
    af_move_lens(sproc, af, sproc->share.af_ext.direction,
      caf->bigger_tryingstep);
    caf->status = CAF_FOCUSING;
    af->state = AF_GATHER_STATS_CONT_SEARCH;
  }
  af->frame_delay = caf->baseDelay;
  /* We should take this opportunity to reset gyro parameters */
  memset((af_cont_gyro_t *)gyro_data, 0, sizeof(af_cont_gyro_t));
} /* af_cont_initiate_caf */

/*===========================================================================
* FUNCTION    - af_CAF_monitor -
*
* DESCRIPTION:
*==========================================================================*/
int af_CAF_monitor(stats_proc_t *sproc, af_t *af)
{
  af_continuous_t *caf = &(af->caf);
  af_cont_gyro_t *gyro_data = &(caf->gyro_data);
  stats_proc_gyro_info_t *gyro_info = &(sproc->input.gyro_info);
  /* If maximum exposure index is used (low light condition) */
  int alpha, exp_indx_chng_thld, targetchange, panning_stable, i;
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  int ret = 0;
  int fv_var = 0;

  /* If CAF is locked we'd just return without doing anything */
  if (caf->locked) {
    CDBG_AF("%s: CAF is locked. Ignore and return!",__func__);
    return ret;
  }

  CDBG_AF("%s:FVmax, FVcur, and Lens Position %d, %d, %d\n", __func__, af->
    max_focus_val, af->cur_focus_val, af->cur_pos);
  CDBG_AF("%s:Overall Luma, Curr selected Luma, Luma Max %d,%d,%d\n",
    __func__, sproc->share.aec_ext.comp_luma,
    caf->cur_luma, caf->max_luma);
  if (af->index >= AF_COLLECTION_POINTS) {
    af->index = 1;
    af->stats[0] = af->stats[AF_COLLECTION_POINTS - 1];
  }
  af->stats[af->index++] = (stats->af_op.Focus / stats->af_op.
    NFocus / MAX(caf->cur_luma, 1));

  /* If it's video mode */
  if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER) {
    ret = af_cont_gyro_monitor_mode_video(gyro_data, gyro_info);
  } else { /*it's a preview mode*/
    ret = af_cont_gyro_monitor_mode_camera(gyro_data, gyro_info);
  }

  /* If we are not supposed to change our state, return from here */
  if (ret != 0)
    return 0;

  /* If we must refocus we'll ignore other condition checks*/
  if (gyro_data->refocus_for_gyro) {
    CDBG_HIGH("%s: Gyro requests Refocus. Initiate CAF!!!", __func__);
    if(fptr->af_CAF.af_search_mode &&
      (sproc->input.mctl_info.opt_state != STATS_PROC_STATE_CAMCORDER)) {
      CDBG_AF("%s: Doing full-sweep!!!", __func__);
      af->state = AF_START;
      af->caf.ignore_full_sweep = FALSE;
      af_reset_lens(sproc, af);
    }
    else
      af_cont_initiate_caf(sproc, af);
  } else {
    CDBG_AF("%s: Gyro condition not met yet. Check others!!", __func__);
    CDBG_AF("%s: frame_delay %d\n", __func__, af->frame_delay);
    if (af->frame_delay > 0) {
      if (sproc->share.luma_settled_cnt <= 2 &&
        (sproc->share.aec_ext.comp_luma <= 25 ||
        sproc->share.aec_ext.comp_luma >= 100 ) &&
        ((uint32_t)sproc->share.aec_ext.exp_index <
        sproc->share.aec_ext.exp_tbl_val - 1)) {
        CDBG_AF("%s:WARNING: AEC IS CHANGING, before, current %d, %d, %d",
          __func__, caf->exp_index_before_change,
          sproc->share.aec_ext.exp_index, sproc->share.luma_settled_cnt);
      }
      af->frame_delay--;
      caf->frame_cnt ++;
      CDBG_AF("%s: rc = 0", __func__);
      return 0;
    }
    if (caf->panning_index == 10)
      caf->panning_index = 0;

    caf->panning_track[caf->panning_index++] = af->cur_focus_val;

    if (caf->clean_panning_track) {
      int tmp_fv = af->cur_focus_val;
      CDBG_AF("%s: tmp_fv %d", __func__, tmp_fv);
      caf->clean_panning_track = FALSE;
      for (i = 0; i < 10; i++)
        caf->panning_track[i] = tmp_fv;
    }
    /* wait for 3 frames if maximum exposure index is used and */
    /* then perform the FV normalization */
    if (af->luma_tabilize_wait  && sproc->share.aec_ext.comp_luma <= 25 &&
      sproc->share.aec_ext.exp_index <
      (int)sproc->share.aec_ext.exp_tbl_val - 1) {
      af->frame_delay = CONTAF_LOW_LIGHT_WAIT;
      caf->frame_cnt ++;
      af->luma_tabilize_wait = FALSE;
      CDBG_AF("%s: rc = 0", __func__);
      return 0;
    }
    af->luma_tabilize_wait = TRUE;
    caf->frame_cnt ++;
    if (af->index == 0)
      caf->exp_index_before_change = sproc->share.aec_ext.exp_index;

    CDBG_AF("%s:FVmax, FV, and Lens Position %d, %d, %d\n", __func__, af->
      max_focus_val, af->cur_focus_val, af->cur_pos);
    CDBG_AF("%s:Overall Luma, Curr selected Luma, Luma Max %d,%d,%d\n",
      __func__, sproc->share.aec_ext.comp_luma,
      caf->cur_luma, caf->max_luma);

    /* REVIST for THRESHOLD CHECK, REVIST for this CbCr info
     * Histogram collection should be OFF when AF is active
     * Digital Zoom is true ? */
    targetchange = 0;
    /* Reorganize the scene change detection criteria and add the adaptive
     * scene change detection criteria based on lighting condition. */
    if (af->in_low_light) {
      alpha = (int)(fptr->af_CAF.af_scene_change_detection_ratio * 1.2 + 0.5);
      exp_indx_chng_thld = fptr->af_CAF.af_cont_lux_index_change_threshold / 2;
    } else {
      alpha = fptr->af_CAF.af_scene_change_detection_ratio;;
      exp_indx_chng_thld = fptr->af_CAF.af_cont_lux_index_change_threshold;
    }
    fv_var = abs(af->cur_focus_val- af->max_focus_val);
    CDBG_AF("%s: abs %d, alpha %d, luma_target %d, MAX %d", __func__,
      fv_var, alpha,
      sproc->share.aec_ext.target_luma, MAX(caf->cur_luma, 1));

    CDBG_AF("%s:focusval_max %d, vfe_af_WinHorWidth %d, MAX %d", __func__,
      af->max_focus_val, sproc->input.mctl_info.vfe_af_WinHorWidth,
      MAX(caf->max_luma, 1));

    CDBG_AF("%s: abs %d, exp_index_change_thres %d", __func__, abs(caf->
      exp_index_before_change - sproc->share.aec_ext.exp_index),
      exp_indx_chng_thld);

    if (((fv_var * alpha) > af->max_focus_val) ||
      (abs(caf->exp_index_before_change - sproc->share.
      aec_ext.exp_index) > exp_indx_chng_thld)) {
      targetchange = 1;
      /* FV variation. Target may not be in focus. Setting CAF_UNKNOWN flag */
      caf->status = CAF_UNKNOWN;
      CDBG_AF("%s: Warning: Big FV variation, target might be changed FVmax %d"
        "FVcur %d, luma_max %d, luma_cur %d\n", __func__, af->
        max_focus_val,
        af->cur_focus_val,
        caf->max_luma, caf->cur_luma);
    }
    if (caf->trig_refocus == TRUE) {
      CDBG_AF("%s: CAF Warning: trigger_refocus due to AEC", __func__);
      targetchange = 1;
      caf->status = CAF_UNKNOWN;
    }
    if (af_panning_stable_check(sproc, af) < 0) {
      panning_stable = FALSE;
      af->caf.panning_unstable_cnt++;
    } else {
      panning_stable = TRUE;
      af->caf.panning_unstable_cnt = 0;
    }
    if (af->caf.panning_unstable_cnt >
      fptr->af_CAF.af_panning_unstable_trigger_cnt) {
      targetchange = 1;
      caf->status = CAF_UNKNOWN;
      CDBG_AF("targetchange is 1 due to panning_unstable_cnt");
    }

    // check SAD condition - only for camera as it's too sensitive
    // for camcorder.
    if (sproc->input.mctl_info.opt_state != STATS_PROC_STATE_CAMCORDER) {
      if(af_cont_did_sad_change(sproc, af) == TRUE){
        targetchange =1;
        caf->status = CAF_UNKNOWN;
        CDBG_AF("targetchange is 1 due to  due to SAD change");
      }
    }

    if (caf->panning_index == 10)
      caf->panning_index = 0;

    caf->panning_track[caf->panning_index++] =
      af->stats[af->index - 1];

    CDBG_AF("%s: tgtchng %d, luma_settled_cnt %d, exp_ind %d, exp_val %d",
      __func__, targetchange, sproc->share.luma_settled_cnt,
      sproc->share.aec_ext.exp_index, sproc->share.aec_ext.exp_tbl_val);

    caf->start_luma = caf->cur_luma;

    /* Let's check whether we meet the condition for initiating autofocus */
    if (targetchange > 0 && panning_stable &&
      gyro_data->panning_stable_for_video &&
      (sproc->share.luma_settled_cnt >= 2 ||
      (sproc->share.aec_ext.exp_index ==
      (int)sproc->share.aec_ext.exp_tbl_val - 1))) {
      CDBG_HIGH("%s: Refocus conditions met. Initiating CAF!!!", __func__);
      if(fptr->af_CAF.af_search_mode &&
        (sproc->input.mctl_info.opt_state != STATS_PROC_STATE_CAMCORDER)) {
        CDBG_AF("%s: Doing full-sweep!!!", __func__);
        af->state = AF_START;
        af->caf.ignore_full_sweep = FALSE;
        af_reset_lens(sproc, af);
      }
      else
        af_cont_initiate_caf(sproc, af);
    }
  }
  return 0;
} /* af_CAF_monitor */

/*===========================================================================
* FUNCTION    - af_CAF_make_decision -
*
* DESCRIPTION:
*==========================================================================*/
int af_CAF_make_decision(stats_proc_t *sproc, af_t *af)
{
  int rc = 0, fv0_in_noise, fv1_in_noise, fv2_in_noise;
  af_continuous_t *caf = &(af->caf);
  const int CAF_MAX_INDECISION_CNT = 1;
  /* To indicate if the target is panning */
  float noise;
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  CDBG_HIGH("%s: frame_delay %d, IDX %d, FV %d, Lens Pos, %d, Cur_Luma %d",
    __func__, af->frame_delay, af->index - 1, stats->af_op.Focus / stats->af_op.
    NFocus, af->cur_pos, caf->cur_luma);

  if (af->frame_delay > 0) {
    af->frame_delay --;
    caf->frame_cnt ++;
    return rc;
  }
  if (caf->cur_luma > 0) {
    if (0.8 > (float)caf->start_luma / caf->cur_luma ||
      (float)caf->start_luma / caf->cur_luma > 1.2) {
      CDBG_AF("%s:trigger_refocus: AEC is changing\n", __func__);
      caf->trig_refocus = TRUE;
    }
  }

  if (af->index >= AF_COLLECTION_POINTS) {
    af->index = 1;
    af->stats[0] = af->stats[AF_COLLECTION_POINTS - 1];
  }

  caf->frame_cnt ++;
  af->locn[af->index] = af->cur_pos;
  af->stats[af->index++] =
    (stats->af_op.Focus / stats->af_op.NFocus / MAX(caf->cur_luma, 1));
  CDBG_AF("%s:LensPosition, FV, and current luma %d, %d, %d", __func__,
    af->cur_pos, stats->af_op.Focus / stats->af_op.NFocus / MAX(
    caf->cur_luma, 1), caf->cur_luma);
  if (caf->trial_in_noise1 == 0) { /* first two search points only */
    fv0_in_noise = af->stats[af->index - 2];
    fv1_in_noise = af->stats[af->index - 1];

    if (fv1_in_noise > af->max_focus_val) {
      af->max_focus_val = fv1_in_noise;
      af->max_pos = af->cur_pos;
    } else if (fv1_in_noise < af->min_focus_val)
      af->min_focus_val = fv1_in_noise;

    if (af->cur_pos == af->far_end ||
      af->cur_pos == af->near_end) {
      if (af->cur_pos == af->far_end) {
        sproc->share.af_ext.direction = MOVE_NEAR;
        caf->back_step_cnt =
          af->cur_pos - af->locn[0] + caf->bigger_tryingstep;
      } else {
        sproc->share.af_ext.direction = MOVE_FAR;
        caf->back_step_cnt =
          af->locn[0] - af->cur_pos + caf->smaller_tryingstep;
      }
      af->state = AF_GATHER_STATS_CONT_SEARCH;
      af_slow_move_lens(sproc, af, sproc->share.af_ext.direction,
        caf->back_step_cnt);
      caf->back_step_cnt -= caf->fov_allowed_steps;
      CDBG_AF("%s: rc %d\n", __func__, rc);
      return rc;
    }

    noise = (float)(abs(fv1_in_noise - fv0_in_noise)) / (float)(fv0_in_noise);
    if (noise < THRESHOLD_IN_NOISE) {
      caf->tryingstep = (af->far_end - af->cur_pos) < (af->
        far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
      af_move_lens_for_trying_step(sproc, af);
      caf->trial_in_noise1++;
      af->frame_delay = caf->baseDelay;
      CDBG_AF("%s:%d: rc=%d\n", __func__, __LINE__, rc);
      return rc;  /* make another move directly and quit */
    } else if (fv0_in_noise > fv1_in_noise) {
      /* noise >= threshold, first search drop and big, CASE1 */
      caf->tryingstep = (af->far_end - af->cur_pos) < (af->
        far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
      if (sproc->share.af_ext.direction == MOVE_NEAR) {
        sproc->share.af_ext.direction = MOVE_FAR;
        caf->back_step_cnt = MIN((af->locn[0] - af->cur_pos +
          caf->tryingstep), af->far_end - af->cur_pos);
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
        caf->back_step_cnt -= caf->fov_allowed_steps;
      } else {
        sproc->share.af_ext.direction = MOVE_NEAR;
        caf->back_step_cnt = MIN((af->cur_pos - af->locn[0] +
          caf->tryingstep), af->cur_pos - af->near_end);
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
        caf->back_step_cnt -= caf->fov_allowed_steps;
      }
      CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
      return rc;
    } else if (fv0_in_noise < fv1_in_noise) {
      caf->tryingstep = ( af->far_end - af->cur_pos) <
        (af->far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
      af_move_lens_for_trying_step(sproc, af);
      /* do the search because the right direction is found */
      af->state = AF_GATHER_STATS_CONT_SEARCH;
      af->frame_delay = caf->baseDelay;
      CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
      return rc;
    }
  } else {
    /* first three search points,last two FVs didn't show much variation,*/
    /* FV0, FV1 and FV2 are along the same search direction */
    fv0_in_noise = af->stats[af->index - 3];
    fv1_in_noise = af->stats[af->index - 2];
    fv2_in_noise = af->stats[af->index - 1];

    if ((fv0_in_noise <= fv1_in_noise) &&(fv1_in_noise <= fv2_in_noise)) {
      /* Going UP, direction is right, CASE2 */
      if (fv2_in_noise > af->max_focus_val) {
        af->max_focus_val = fv2_in_noise;
        af->max_pos = af->cur_pos;
      }
      if (fv0_in_noise < af->min_focus_val) {
        af->min_focus_val = fv0_in_noise;
      }
      if (af->cur_pos == af->far_end) {
        af->num_downhill++;
        sproc->share.af_ext.direction = MOVE_NEAR;
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        caf->back_step_cnt =
          af->cur_pos - af->locn[0] + caf->bigger_tryingstep;
        af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
        caf->back_step_cnt -= caf->fov_allowed_steps;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      }
      if (af->cur_pos == af->near_end) {
        caf->tryingstep = ( af->far_end - af->cur_pos) < (af->
          far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
        af->num_downhill++;
        sproc->share.af_ext.direction = MOVE_FAR;
        caf->back_step_cnt = af->locn[0] -
          af->cur_pos + caf->tryingstep;
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
        caf->back_step_cnt -= caf->fov_allowed_steps;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      }
      caf->tryingstep = ( af->far_end - af->cur_pos) < (af->
        far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
      af_move_lens_for_trying_step(sproc, af);
      af->state = AF_GATHER_STATS_CONT_SEARCH;
      af->frame_delay = caf->baseDelay;
      CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
      return rc;
    } else if ((fv0_in_noise >= fv1_in_noise)
      && (fv1_in_noise >=fv2_in_noise)) {
      /* Going Down, direction is not right, CASE1 */
      if (fv0_in_noise > af->max_focus_val) {
        /* first time position_max already updated before when */
        /* fv0 is recorded */
        af->max_focus_val = fv0_in_noise;
        af->max_pos = af->locn[af->index - 3];
      }
      if (fv2_in_noise < af->min_focus_val)
        af->min_focus_val = fv2_in_noise;

      if (af->cur_pos == af->far_end) {
        sproc->share.af_ext.direction = MOVE_NEAR;
        caf->back_step_cnt = af->cur_pos -
          af->locn[0] + caf->bigger_tryingstep;
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
        caf->back_step_cnt -= caf->fov_allowed_steps;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      }
      if (af->cur_pos == af->near_end) {
        sproc->share.af_ext.direction = MOVE_FAR;
        caf->back_step_cnt = af->locn[0] -
          af->cur_pos + caf->smaller_tryingstep;
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
        caf->back_step_cnt -= caf->fov_allowed_steps;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      }
      caf->tryingstep = ( af->far_end - af->cur_pos) < (af->
        far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
      if (sproc->share.af_ext.direction == MOVE_NEAR) {
        sproc->share.af_ext.direction = MOVE_FAR;
        caf->back_step_cnt = MIN((af->locn[0] -
          af->cur_pos + caf->tryingstep),
          af->far_end - af->cur_pos);
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
      } else {
        sproc->share.af_ext.direction = MOVE_NEAR;
        caf->back_step_cnt = MIN((af->cur_pos -
          af->locn[0] + caf->tryingstep),
          af->cur_pos - af->near_end);
        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
      }
      caf->back_step_cnt -= caf->fov_allowed_steps;
      CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
      return rc;
    } else if ((fv0_in_noise >= fv1_in_noise)
      && (fv1_in_noise <=fv2_in_noise)) {
      /* the middle is the smallest, but not sure. Exhaustive search? */
      if (fv1_in_noise < af->min_focus_val) {
        af->min_focus_val = fv1_in_noise;
      }
      noise = (float)(fv2_in_noise - fv1_in_noise) /(float)(fv1_in_noise);
      if (noise > THRESHOLD_IN_NOISE) {
        /* treated as UP case, CASE2 */
        if (fv2_in_noise > af->max_focus_val) {
          af->max_focus_val = fv2_in_noise;
          af->max_pos = af->cur_pos;
        }
        if (af->cur_pos == af->far_end) {
          af->num_downhill++;
          sproc->share.af_ext.direction = MOVE_NEAR;
          caf->back_step_cnt = af->cur_pos - af->locn[0] +
            caf->bigger_tryingstep;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        if (af->cur_pos == af->near_end) {
          af->num_downhill++;
          sproc->share.af_ext.direction = MOVE_FAR;
          caf->back_step_cnt = af->locn[0] - af->cur_pos +
            caf->bigger_tryingstep;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        caf->tryingstep = ( af->far_end  - af->cur_pos ) < (af->
          far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
        af_move_lens_for_trying_step(sproc, af);

        af->state = AF_GATHER_STATS_CONT_SEARCH;
        af->frame_delay = caf->baseDelay;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      } else {
        if (fv0_in_noise < fv2_in_noise) {
          if (fv2_in_noise > af->max_focus_val) {
            af->max_focus_val = fv2_in_noise;
            af->max_pos = af->cur_pos;
          }
        } else {
          if (fv0_in_noise > af->max_focus_val) {
            af->max_focus_val = fv0_in_noise;
            af->max_pos = af->locn[af->index - 3];
          }
        }
        if (af->cur_pos == af->far_end) {
          sproc->share.af_ext.direction = MOVE_NEAR;
          caf->back_step_cnt = af->cur_pos - af->locn[0]
            + caf->bigger_tryingstep;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        if (af->cur_pos == af->near_end) {
          sproc->share.af_ext.direction = MOVE_FAR;
          caf->back_step_cnt = af->locn[0] -
            af->cur_pos + caf->smaller_tryingstep;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        caf->no_of_indecision++;
        if (caf->no_of_indecision > CAF_MAX_INDECISION_CNT) {
          caf->tryingstep = ( af->far_end  - af->cur_pos) < (af->
            far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
          if (sproc->share.af_ext.direction == MOVE_NEAR) {
            sproc->share.af_ext.direction = MOVE_FAR;
            af->state = AF_GATHER_STATS_CONT_SEARCH;
            caf->back_step_cnt = MIN((af->locn[0] -
              af->cur_pos + caf->tryingstep),
              (af->far_end  - af->cur_pos));
            af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
            caf->back_step_cnt -= caf->fov_allowed_steps;
          } else {
            sproc->share.af_ext.direction = MOVE_NEAR;
            af->state = AF_GATHER_STATS_CONT_SEARCH;
            caf->back_step_cnt = MIN(af->cur_pos -
              af->locn[0] + caf->tryingstep, af->cur_pos);
            af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
            caf->back_step_cnt -= caf->fov_allowed_steps;
          }
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        } else {
          caf->tryingstep = (af->far_end - af->cur_pos) < (af->
            far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
          af_move_lens_for_trying_step(sproc, af);
          caf->trial_in_noise1 = 0;
          af->state = AF_MAKE_DECISION;
        }
        af->frame_delay = caf->baseDelay;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      }
    } else if (fv0_in_noise <= fv1_in_noise && fv1_in_noise >= fv2_in_noise) {
      /* the middle is the biggest, but not sure. Exhaustive search? */
      if (fv1_in_noise > af->max_focus_val) {
        af->max_focus_val = fv1_in_noise;
        af->max_pos = af->locn[af->index - 2];
      }
      noise = (float)(fv1_in_noise - fv2_in_noise) /(float)(fv1_in_noise);
      /* Treated as DOWN case, CASE2 */
      if (noise > THRESHOLD_IN_NOISE) {
        if (fv2_in_noise < af->min_focus_val)
          af->min_focus_val = fv2_in_noise;

        if (af->cur_pos == af->far_end) {
          sproc->share.af_ext.direction = MOVE_NEAR;
          caf->back_step_cnt = af->cur_pos -
            af->locn[0] + caf->bigger_tryingstep;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        if (af->cur_pos == af->near_end) {
          sproc->share.af_ext.direction = MOVE_FAR;
          caf->back_step_cnt = af->locn[0] -
            af->cur_pos + caf->smaller_tryingstep;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        caf->tryingstep = (af->far_end - af->cur_pos) < (af->
          far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
        if (sproc->share.af_ext.direction == MOVE_NEAR) {
          sproc->share.af_ext.direction = MOVE_FAR;
          caf->back_step_cnt = MIN(af->locn[0] - af->
            cur_pos + caf->tryingstep, af->far_end - af->cur_pos);
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
        } else {
          sproc->share.af_ext.direction = MOVE_NEAR;
          caf->back_step_cnt = MIN(af->cur_pos - af->
            locn[0] + caf->tryingstep, af->cur_pos - af->near_end);
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
        }
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      } else {
        if (af->cur_pos == af->far_end) {
          sproc->share.af_ext.direction = MOVE_NEAR;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          caf->back_step_cnt = af->cur_pos -
            af->locn[0] + caf->bigger_tryingstep;
          af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        if (af->cur_pos == af->near_end) {
          sproc->share.af_ext.direction = MOVE_FAR;
          af->state = AF_GATHER_STATS_CONT_SEARCH;
          caf->back_step_cnt = af->locn[0] -
            af->cur_pos + caf->smaller_tryingstep;
          af_slow_move_lens(sproc, af ,MOVE_FAR, caf->back_step_cnt );
          caf->back_step_cnt -= caf->fov_allowed_steps;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        }
        caf->no_of_indecision++;
        if (caf->no_of_indecision > CAF_MAX_INDECISION_CNT) {
          caf->tryingstep = (af->far_end  - af->cur_pos) < (af->
            far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
          if (sproc->share.af_ext.direction == MOVE_NEAR) {
            sproc->share.af_ext.direction = MOVE_FAR;
            af->state = AF_GATHER_STATS_CONT_SEARCH;
            caf->back_step_cnt = MIN(af->locn[0] - af->
              cur_pos + caf->tryingstep, af->far_end -
              af->cur_pos);
            CDBG_AF("%s:%d: Calling af_do_move_lens", __func__, __LINE__);
            af_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
            caf->back_step_cnt -= caf->fov_allowed_steps;
          } else {
            sproc->share.af_ext.direction = MOVE_NEAR;
            af->state = AF_GATHER_STATS_CONT_SEARCH;
            caf->back_step_cnt = MIN(af->cur_pos - af->
              locn[0] + caf->tryingstep, af->cur_pos);
            CDBG_AF("%s: %d: Calling af_do_move_lens", __func__, __LINE__);
            af_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
            caf->back_step_cnt -= caf->fov_allowed_steps;
          }
          af->frame_delay = caf->baseDelay;
          CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
          return rc;
        } else {
          caf->tryingstep = ( af->far_end  - af->cur_pos ) < (af->
            far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;
          af_move_lens_for_trying_step(sproc, af);
          caf->trial_in_noise1 = 0;
          af->state = AF_MAKE_DECISION;
        }
        af->frame_delay = caf->baseDelay;
        CDBG_AF("%s: %d: rc %d", __func__, __LINE__, rc);
        return rc;
      }
    }
  }
  return rc;
} /* af_CAF_make_decision */

/*===========================================================================
* FUNCTION    - af_CAF_gather_stats -
*
* DESCRIPTION:
*==========================================================================*/
int af_CAF_gather_stats(stats_proc_t *sproc, af_t *af)
{
  int downhill_allowance, just_moved = 0;
  af_continuous_t *caf = &(af->caf);
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  downhill_allowance = fptr->af_CAF.af_downhill_allowance;
  int flat_fv = FALSE;

  CDBG_AF("%s:AF_GATHER_STATS_CONT_SEARCH\n", __func__);
  CDBG_AF("%s:%d:luma_cur=%d, CAF_start_luma=%d\n",
    __func__, __LINE__, caf->cur_luma, caf->start_luma);

  if (caf->cur_luma > 0) {
    if (0.8 > (float)caf->start_luma / caf->cur_luma ||
      (float)caf->start_luma / caf->cur_luma > 1.2) {
      CDBG_AF("%s:trigger_refocus: AEC is changing\n", __func__);
      caf->trig_refocus = TRUE;
    }
  }
  if (af->frame_delay > 0) {
    if (af->index == 0)
      CDBG_AF("%s:Frame Delays, FVcur, LensP %d, %d, %d", __func__,
        af->frame_delay, stats->af_op.Focus/ stats->af_op.NFocus / MAX(
        caf->cur_luma, 1), af->cur_pos);
    af->frame_delay--;
    CDBG_AF("%s: rc = 0", __func__);
    return 0;
  }
  if (af->index >= AF_COLLECTION_POINTS) {
    af->index = 1;
    af->stats[0] = af->stats[AF_COLLECTION_POINTS - 1];
  }
  /* Store 27 bit focus value divided by num valid rows to get an average */
  af->stats[af->index++] = (int)(stats->af_op.Focus / stats->af_op.
    NFocus / MAX(caf->cur_luma, 1));
  CDBG_AF("%s:LensP, FVcur, and LUMAcur %d, %d, %d\n", __func__, af->
    cur_pos, stats->af_op.Focus / stats->af_op.NFocus / MAX(caf->cur_luma, 1),
    caf->cur_luma);
  if (af->max_focus_val < af->stats[af->index - 1]) {
    af->max_focus_val = af->stats[af->index - 1];
    af->max_pos = af->cur_pos;
    af->num_downhill = 0;
  } else {
    if (af->min_focus_val >= af->stats[af->index - 1])
      af->min_focus_val = af->stats[af->index - 1];

    /* To increase the robustness to FV variation */
    if (af->stats[af->index -1] < af->stats[af->index - 2])
      af->num_downhill++;
    else
      af->num_downhill  = 0;
  } /* end of moving out */
  if (((sproc->share.af_ext.direction == MOVE_FAR && af->cur_pos < af->far_end)
    || (sproc->share.af_ext.direction == MOVE_NEAR && af->
    cur_pos > af->near_end)) && af->num_downhill < downhill_allowance) {

    caf->tryingstep = ( af->far_end  - af->cur_pos) <
      (af->far_end / 3) ? caf->bigger_tryingstep : caf->smaller_tryingstep;

    af_move_lens_for_trying_step(sproc, af);
    just_moved = 1;
  }
  if (((af->cur_pos >= af->far_end || af->cur_pos <= af->
    near_end) && !just_moved) || af->num_downhill >= downhill_allowance) {
    /* Select bin for finer search */
    /* first to see if the min and max FV are meaningful */
    /* in the NORMAL mode only */
    /* Need to modify here, should use luma_max instead of luma_cur. */
    /* But do we need to record every luma? */
    if (af->min_focus_val >
      (float)af->max_focus_val * CONFIDENCE_LEVEL_EXH) {
      /* FV is either too small or too flat, need research CASE4 */
      /* CDBG_AF("%s:Warning: FVs not reliable, REFOCUSINng USING \ */
      /* Exhaustive search %d, %d, %d\n",__func__, \*/
      /* af->max_focus_val,ctrl->stats_proc_af_state.\ */
      /* focusvalue_min,af->cur_pos); */
      CDBG_AF("%s:Warning: FVs not reliable, return to monitoring state \
        %d, %d, %d\n",__func__, af->max_focus_val, \
        af->min_focus_val, af->cur_pos);
      flat_fv = TRUE;
    }
   /* If we were in FOCUSING mode before when autoFocus call was
     * made, we just returned setting 'send_event_later' flag. Now
     * the focus is complete we'll need to send event back.
     * If we conclude FV isn't flat, we've found the max. If
     * current position has the MAX FV, we'll set the flag to send
     * event now. Otherwise, we'll send the event later once we
     * complete moving the lens to max position. */
    caf->status = (flat_fv) ? CAF_UNKNOWN : CAF_FOCUSED;
    caf->back_step_cnt = af->cur_pos - af->max_pos;
    if (caf->back_step_cnt > 0) {
      caf->back_step_cnt = MIN(caf->back_step_cnt,
        af->cur_pos - af->near_end);
      sproc->share.af_ext.direction = MOVE_NEAR;
      af->state = AF_GATHER_STATS_CONT_MONITOR;
      af_slow_move_lens(sproc, af, MOVE_NEAR, caf->back_step_cnt);
      caf->back_step_cnt -= caf->fov_allowed_steps;
    } else if (caf->back_step_cnt < 0) {
      caf->back_step_cnt = MIN(-caf->back_step_cnt,
        af->far_end  - af->cur_pos);
      sproc->share.af_ext.direction = MOVE_FAR;
      af->state = AF_GATHER_STATS_CONT_MONITOR;
      af_slow_move_lens(sproc, af, MOVE_FAR, caf->back_step_cnt);
      caf->back_step_cnt -= caf->fov_allowed_steps;
    } else {
      af->max_focus_val = af->cur_focus_val;
      caf->max_luma = caf->cur_luma;
      af->state = AF_GATHER_STATS_CONT_MONITOR;

      if (caf->send_event_later) {
        sproc->share.af_ext.done_status = (caf->status == CAF_FOCUSED) ? 1 : 0;
        sproc->share.af_ext.done_flag = TRUE;
        caf->send_event_later = FALSE;
      }
    }
    af->index = 0;
    caf->clean_panning_track = TRUE;
    caf->panning_index = 0;
    af->frame_delay = caf->baseDelay;
    af->final_lens_pos = af->max_pos;
    CDBG_AF("%s: set panning_index to %d f6rame_delay %d", __func__,
      caf->panning_index, af->frame_delay);
  }
  return 0;
} /* af_CAF_gather_stats */

/*===========================================================================
* FUNCTION    - af_do_process_continuous_search -
*
* DESCRIPTION:
*==========================================================================*/
int af_continuous_search (stats_proc_t *sproc, af_t *af)
{
  int  rc = 0;
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  af->caf.baseDelay = fptr->af_CAF.af_cont_base_frame_delay;
  /* adjust baseDelay based on current FPS so that CAF does
     not become verly slow when fps is low. */
  CDBG_AF("%s: baseDelay: %d current_fps: %d max_fps: %d", __func__,
    af->caf.baseDelay, sproc->input.sensor_info.preview_fps,
    sproc->input.sensor_info.max_preview_fps);
  af->caf.baseDelay = (int)((float)(af->caf.baseDelay *
    (sproc->input.sensor_info.preview_fps /
    sproc->input.sensor_info.max_preview_fps) + 0.5));
  if (af->caf.baseDelay < 1) {
    af->caf.baseDelay = 0;
  }
  CDBG_AF("%s: baseDelay after admustment: %d", __func__, af->caf.baseDelay);

  af->prev_stat = af->cur_stat;
  af->cur_stat = af->cur_focus_val;

  /*todo ask ruben if we need this variable */
  af->caf.cur_luma = sproc->share.cur_af_luma;
  CDBG_AF("%s: luma_cur %d state=%d\n", __func__,
    af->caf.cur_luma, af->state);

  if (stats == NULL || (af->step_fail_cnt > FOCUS_ATTEMPTS)) {
    if (af->step_fail_cnt > FOCUS_ATTEMPTS) {
      CDBG_AF("%s:Focus, failed after %d attempt to move sensor position",
        __func__, FOCUS_ATTEMPTS);
      af->state = AF_ERROR;
    } else
      af->state = AF_INACTIVE;
    sproc->share.af_ext.active = FALSE;
  } else if (af->state == AF_GATHER_STATS_CONT_MONITOR)
    rc = af_CAF_monitor(sproc, af);
  else if (af->state == AF_MAKE_DECISION)
    rc = af_CAF_make_decision(sproc, af);
  else if (af->state == AF_GATHER_STATS_CONT_SEARCH) {
    rc = af_CAF_gather_stats(sproc, af);
    return rc;
  } else if (af->state == AF_MOVING_LENS) {
    af_slow_move_lens(sproc, af, sproc->share.af_ext.direction,
      af->caf.back_step_cnt);
    af->caf.back_step_cnt -= af->caf.fov_allowed_steps;
    CDBG_AF("%s: slow move lens position %d, direction %d\n", __func__,
      af->cur_pos,  sproc->share.af_ext.direction);
  }
  CDBG_AF("%s:CAF Pos %d rc %d", __func__, af->cur_pos, rc);
  return rc;
} /* af_continuous_search */
