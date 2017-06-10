/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "stats_proc.h"
#include "af.h"

#ifdef HILL_CLIMB_ALGO
/*===========================================================================
 * FUNCTION    - af_init_hill_climbing_search -
 *
 * DESCRIPTION:
 *==========================================================================*/
void af_init_hill_climbing_search(stats_proc_t *sproc, af_t *af)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  /*  depending on sensor start */
  af->hc.steps_btwn_stat_pts_macro_coarse =
    fptr->num_gross_steps_between_stat_points;
  af->hc.steps_btwn_stat_pts_macro_fine =
    fptr->num_fine_steps_between_stat_points;
  af->hc.steps_btwn_stat_pts_normal_fine =
    fptr->num_fine_steps_between_stat_points;
  af->hc.trial_of_macro_fine = fptr->num_fine_search_points;

  /*  for noise handling */
  af->hc.fv0_in_noise = 0;
  af->hc.fv1_in_noise = 0;
  af->hc.fv2_in_noise = 0;
  af->hc.trial_in_noise = 0;

  af->state = AF_INACTIVE;
  sproc->share.af_ext.active = FALSE;
  if (af->algo_type == AF_HILL_CLIMBING_CONSERVATIVE) {
    af->downhill_allowance = 10;
    af->fv_drop_allowance  = 0;
  } else if (af->algo_type == AF_HILL_CLIMBING_DEFAULT) {
    af->downhill_allowance = 3;
    af->fv_drop_allowance  = 0.5;
  } else if (af->algo_type == AF_EXHAUSTIVE_SEARCH) {
    af->downhill_allowance = 2;
    af->fv_drop_allowance  = 0.5;
  } else {
    af->downhill_allowance = 2;
    af->fv_drop_allowance  = 0.75;
  }
  return;
} /* af_init_hill_climbing_search */

/*===========================================================================
* FUNCTION    - af_approach_to_peak -
*
* DESCRIPTION:
*==========================================================================*/
static int af_approach_to_peak(stats_proc_t *sproc, af_t *af)
{
  int move_back;

  if (af->hc.testactive == 1 && (af->min_focus_val >
    (af->max_focus_val * CONFIDENCE_LEVEL))) {
    /* only COARSE and AF_EXHAUSTIVE call this function? */
    af_reset_lens(sproc, af);
    af->state = AF_EXHAUSTIVE;
    af->hc.testactive = 0;
    return 0;
  }
  move_back = af->cur_pos - af->max_pos;

  if (move_back > (af->hc.trial_of_macro_fine >> 1))
    af_move_lens(sproc, af, MOVE_NEAR, (move_back - (af->
      hc.trial_of_macro_fine >> 1)));
  else if (move_back < -(af->hc.trial_of_macro_fine >> 1))
    af_move_lens(sproc, af, MOVE_FAR, ((-move_back) - (af->
      hc.trial_of_macro_fine >> 1)));
  else {
    if (af->cur_pos >= af->max_pos)
      sproc->share.af_ext.direction = MOVE_NEAR;
    else
      sproc->share.af_ext.direction = MOVE_FAR;
  }
  return 0;
} /* af_approach_to_peak */

/*===========================================================================
* FUNCTION    - af_finish_search -
*
* DESCRIPTION:
*==========================================================================*/
static int af_finish_search(stats_proc_t *sproc, af_t *af)
{
  int move_back;

  if (af->hc.testactive && af->min_focus_val >
    (af->max_focus_val * (CONFIDENCE_LEVEL + 0.1))) {
    af_reset_lens(sproc, af);
    af->state = AF_EXHAUSTIVE;
    af->hc.testactive = 0;
    return 0;
  }
  move_back = af->cur_pos - af->max_pos;

  if (move_back > 0) {
    af->state = AF_FOCUSING;
    af_move_lens(sproc, af, MOVE_NEAR, move_back);
  } else if (move_back < 0) {
    af->state = AF_FOCUSING;
    af_move_lens(sproc, af, MOVE_FAR, -move_back);
  } else {
    if (af->collect_end_stat)
      af->state = AF_COLLECT_END_STAT;
    else /*  AF Completed successfully */
      af_done(sproc, af, CAMERA_EXIT_CB_DONE);
  }
  return 0;
} /* af_finish_search */

/*===========================================================================
* FUNCTION    - af_mind_boundary_and_edge -
*
* DESCRIPTION:
*==========================================================================*/
static void af_mind_boundary_and_edge(stats_proc_t *sproc, af_t *af, int stat_pts)
{
  if (sproc->share.af_ext.direction == MOVE_FAR) {
    if (af->cur_pos + stat_pts <= af->far_end)
      af_move_lens(sproc, af, MOVE_FAR, stat_pts);
    else { /*  too close to the far_end/boundry , switch the direction */
      af_move_lens(sproc, af, MOVE_NEAR, stat_pts);
      sproc->share.af_ext.direction = MOVE_NEAR;
    }
  } else { /*  direction == MOVE_NEAR */
    if (af->cur_pos - stat_pts > af->near_end)
      af_move_lens(sproc, af, MOVE_NEAR, stat_pts);
    else { /*  too close to the boundary/near_end, switch the direction */
      af_move_lens(sproc, af, MOVE_FAR, stat_pts);
      sproc->share.af_ext.direction = MOVE_FAR;
    }
  }
} /* af_mind_boundary_and_edge */

/*===========================================================================
* FUNCTION    - af_prepare_decision_making -
*
* DESCRIPTION:
*==========================================================================*/
static void af_prepare_decision_making(stats_proc_t *sproc, af_t *af)
{
  int stat_pts;
  if (af->srch_mode == AF_MODE_NORMAL)
    stat_pts= af->hc.steps_btwn_stat_pts_normal_fine;
  else
    stat_pts = af->hc.steps_btwn_stat_pts_macro_coarse;

  if (af->cur_pos > af->far_end - 3 * stat_pts)
    sproc->share.af_ext.direction = MOVE_NEAR;

  if (af->cur_pos < af->near_end + 3 * stat_pts)
    sproc->share.af_ext.direction = MOVE_FAR;

  af_mind_boundary_and_edge(sproc, af, stat_pts);
  af->state = AF_DECISION;
} /* af_prepare_decision_making */

/*===========================================================================
* FUNCTION    - af_coarse_search -
*
* DESCRIPTION:
*==========================================================================*/
static int af_coarse_search(stats_proc_t *sproc, af_t *af)
{
  int stat_pts = af->hc.steps_btwn_stat_pts_macro_coarse;

  if (sproc->share.af_ext.direction == MOVE_FAR) {
    if (af->cur_pos + stat_pts <= af->far_end)
      af_move_lens(sproc, af, MOVE_FAR, stat_pts);
    else { /*  switch to fine search */
      if (af_approach_to_peak(sproc, af) < 0)
        return -1;
      af->state         = AF_GATHER_STATS_FINE;
      af->max_focus_val = 0;
    }
  } else {
    if (af->cur_pos - stat_pts > af->near_end)
      /*  still in Macro range? */
      af_move_lens(sproc, af, MOVE_NEAR, stat_pts);
    else { /*  switch to fine search */
      if (af_approach_to_peak(sproc, af) < 0)
        return -1;
      af->state         = AF_GATHER_STATS_FINE;
      af->max_focus_val = 0;
    }
  }
  return 0;
} /* af_coarse_search */

/*===========================================================================
* FUNCTION    - af_fine_search -
*
* DESCRIPTION:
*==========================================================================*/
static int af_fine_search(stats_proc_t *sproc, af_t *af)
{
  int stat_pts;
  if (af->srch_mode == AF_MODE_NORMAL) {
    stat_pts = af->hc.steps_btwn_stat_pts_normal_fine;
    if (sproc->share.af_ext.direction == MOVE_FAR) {
      if (af->cur_pos + stat_pts <= af->far_end)
        af_move_lens(sproc, af, MOVE_FAR, stat_pts);
      else if (af_finish_search(sproc, af) < 0) /*  finalize searching */
        return -1;
    } else if ((af->cur_pos - stat_pts) >= af->near_end)/*NORM&NEAR*/
      af_move_lens(sproc, af, MOVE_NEAR, stat_pts);
    else if (af_finish_search(sproc, af) < 0) /*  finalize searching */
      return -1;
  } else {
    stat_pts = af->hc.steps_btwn_stat_pts_macro_fine;
    if (af->hc.fine_index < stat_pts) {
      if (sproc->share.af_ext.direction == MOVE_FAR)
        af_move_lens(sproc, af, MOVE_FAR, stat_pts);
      else if (af->cur_pos - stat_pts >= af->near_end) /*MACRO&NEAR*/
        af_move_lens(sproc, af, MOVE_NEAR, stat_pts);
      else
        af->hc.fine_index = af->hc.trial_of_macro_fine;
      af->hc.fine_index++;
    } else if (af_finish_search(sproc, af) < 0) /*  finalize searching */
      return -1;
  }
  return 0;
} /* af_fine_search */

/*===========================================================================
* FUNCTION    - af_make_decision -
*
* DESCRIPTION:
*==========================================================================*/
static void af_make_decision(stats_proc_t *sproc, af_t *af)
{
  float noise;
  int   tmp_index, stat_pts;

  af->hc.testactive = 1;
  /* For confidence level check */
  if (!af->hc.trial_in_noise) { /*  check switching Macro/Normal mode */
    if (af->stats[af->index - 2] < af->stats[af->index - 1])
      tmp_index = af->stats[af->index - 1];
    else
      tmp_index = af->stats[af->index - 2];

    if (tmp_index < 0) {
      /* Redefine the threshold after more test (300 here) */
      af_reset_lens(sproc, af);
      af->state = AF_EXHAUSTIVE;
      af->hc.testactive = 0;
      return;
    }
    af->hc.fv0_in_noise = af->stats[af->index - 2];
    af->hc.fv1_in_noise = af->stats[af->index - 1];
    noise = (float) (abs(af->hc.fv0_in_noise - af->hc.fv1_in_noise)) /
      (float) (STATS_PROC_MAX(af->hc.fv0_in_noise, af->hc.fv1_in_noise));

    if (noise > THRESHOLD_IN_NOISE) {
      if (af->stats[af->index - 1] <
        af->stats[af->index - 2]) {
        if (sproc->share.af_ext.direction == MOVE_NEAR)
          sproc->share.af_ext.direction = MOVE_FAR;
        else
          sproc->share.af_ext.direction = MOVE_NEAR;

        af->hc.drop_at_first_shift = 1;
      }
      if (af->srch_mode == AF_MODE_NORMAL) {
        af->state = AF_GATHER_STATS_FINE;
        stat_pts = af->hc.steps_btwn_stat_pts_normal_fine;
      } else {
        af->state = AF_GATHER_STATS_COARSE;
        stat_pts = af->hc.steps_btwn_stat_pts_macro_coarse;
      }
      af_move_lens(sproc, af, sproc->share.af_ext.direction, stat_pts);
    } else {
      if (af->srch_mode == AF_MODE_NORMAL)
        stat_pts = af->hc.steps_btwn_stat_pts_normal_fine;
      else
        stat_pts = af->hc.steps_btwn_stat_pts_macro_coarse;

      af_move_lens(sproc, af, sproc->share.af_ext.direction, stat_pts);
      af->hc.trial_in_noise = 1;
    }
  } else {
    af->hc.fv2_in_noise = af->stats[af->index - 1];

    if (af->hc.fv0_in_noise < af->hc.fv1_in_noise &&
      af->hc.fv1_in_noise < af->hc.fv2_in_noise) { /*  increasing */

      af->hc.trial_in_noise = 0;
      if (af->srch_mode == AF_MODE_NORMAL) {
        af->state = AF_GATHER_STATS_FINE;
        stat_pts = af->hc.steps_btwn_stat_pts_normal_fine;
      } else {
        af->state = AF_GATHER_STATS_COARSE;
        stat_pts = af->hc.steps_btwn_stat_pts_macro_coarse;
      }
      af_move_lens(sproc, af, sproc->share.af_ext.direction, stat_pts);

    } else if (af->hc.fv0_in_noise > af->hc.fv1_in_noise &&
      af->hc.fv1_in_noise > af->hc.fv2_in_noise) {
      af->hc.drop_at_first_shift = af->hc.trial_in_noise + 1;
      af->hc.trial_in_noise = 0;

      if (sproc->share.af_ext.direction == MOVE_NEAR)
        sproc->share.af_ext.direction = MOVE_FAR;
      else
        sproc->share.af_ext.direction = MOVE_NEAR;

      stat_pts = af->hc.drop_at_first_shift;
      if (af->srch_mode == AF_MODE_NORMAL) {
        af->state = AF_GATHER_STATS_FINE;
        stat_pts = stat_pts * af->hc.steps_btwn_stat_pts_normal_fine;
      } else {
        af->state = AF_GATHER_STATS_COARSE;
        stat_pts = stat_pts * af->hc.steps_btwn_stat_pts_macro_coarse;
      }
      af_move_lens(sproc, af, sproc->share.af_ext.direction, stat_pts);
    } else {  /*  in noise */
      if ((af->hc.trial_in_noise == 1)
        && (af->srch_mode == AF_MODE_NORMAL)) {
        af_move_lens(sproc, af, sproc->share.af_ext.direction,
          af->hc.steps_btwn_stat_pts_normal_fine);
        af->hc.trial_in_noise = 2;
        af->hc.fv0_in_noise = af->hc.fv1_in_noise;
        af->hc.fv1_in_noise = af->hc.fv2_in_noise;
      } else { /*  failure */
        af->hc.trial_in_noise = 0;
        af_reset_lens(sproc, af);
        af->state = AF_EXHAUSTIVE;
        af->hc.testactive = 0;
        return;
      }
    }
  }
  return;
} /* af_make_decision */

/*===========================================================================
* FUNCTION    - af_arrange_cur_frame_info -
*
* DESCRIPTION:
*==========================================================================*/
static int af_arrange_cur_frame_info(stats_proc_t *sproc, af_t *af)
{
  int i;
  isp_stats_t *stats = sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct;
  int delay = -1;

  if (af->state == AF_START) {
    af->index = 0;
    af->hc.fine_index = 0;

    if ((float) af->max_focus_val >
      (float) (af->prev_max_focus_val * 0.5)) {
      af->prev_max_focus_val = af->max_focus_val;
    }
    af->num_downhill = 0;
    af->hc.drop_at_first_shift = 0;
    af->hc.testactive = 0;
    memset(&(af->stats), 0, sizeof(int) * AF_COLLECTION_POINTS);

    if (stats->Focus != 0) {
      af->stats[af->index] = (int) (stats->Focus / stats->NFocus);
      af->index++;        /*  increse index */
      /*  set maximums */
      af->max_focus_val = af->stats[af->index - 1];
      af->max_pos = af->cur_pos;
    }
  } else if (af->frame_delay > 0) {
    af->frame_delay--;
    return 0;
  } else {
    if (stats->Focus != 0) {
      af->stats[af->index] = (int) (stats->Focus / stats->NFocus);
      af->index++;

      if (af->max_focus_val <= af->stats[af->index - 1]) {
        af->max_focus_val = af->stats[af->index - 1];
        af->max_pos = af->cur_pos;
        af->num_downhill = 0;

        if (af->index == 1)
          af->min_focus_val = af->max_focus_val; /* change FV_min */
      } else { /*  include minFV */
        if (af->min_focus_val >= af->stats[af->index - 1])
          af->min_focus_val = af->stats[af->index - 1];

        if (af->stats[af->index - 1] <=
          af->stats[af->index - 2])
          af->num_downhill++;
        else
          af->num_downhill = 0;
      }
      /*  check re-focusing case */
      if (af->algo_type == AF_HILL_CLIMBING_DEFAULT ||
        af->algo_type == AF_HILL_CLIMBING_AGGRESSIVE) {
        if (af->hc.drop_at_first_shift && (af->index ==
          REFOCUSING_CHECK_INDEX + (af->hc.drop_at_first_shift - 1))) {
          af->hc.drop_at_first_shift = 0;
          if ((af->stats[af->index - 2] * (1 - THRESHOLD_IN_NOISE) >
            af->stats[af->index - 1])) { /* added confidence check */

            if (af->state == AF_GATHER_STATS_COARSE) {
              if (af_approach_to_peak (sproc, af) == 0) {
                af->state = AF_GATHER_STATS_FINE;
                delay = 0;
              } else {
                sproc->share.af_ext.active = FALSE;
                af->state = AF_ERROR;
              }

            } else if (af->state == AF_GATHER_STATS_FINE)
              if (af_finish_search(sproc, af) < 0) {
                sproc->share.af_ext.active = FALSE;
                af->state = AF_ERROR;
              }
            return delay;
          }
        }
      }
      /*  check downhill case, To release for FINE search */
      if (af->num_downhill >= af->downhill_allowance) {
        int8_t ret_val;
        if ((af->state == AF_GATHER_STATS_COARSE) &&
          ((af->stats[af->index - 1]) <
          (af->max_focus_val) * af->fv_drop_allowance)) {
          if (af_approach_to_peak(sproc, af) == 0) {
            /*  prevent going further and get next frame after delay */
            af->state = AF_GATHER_STATS_FINE;
            delay = 0;
            af->num_downhill = 0;
          } else {
            sproc->share.af_ext.active = FALSE;
            af->state = AF_ERROR;
          }
        } else if (af->state == AF_GATHER_STATS_FINE) {
          if (af_finish_search(sproc, af) < 0) {
            sproc->share.af_ext.active = FALSE;
            af->state = AF_ERROR;
          }

          af->num_downhill = 0;
        }
        return delay;
      }
    }
  }
  return delay;
} /* af_arrange_cur_frame_info */

/*===========================================================================
 * FUNCTION    - af_hill_climbing_search -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_hill_climbing_search(stats_proc_t *sproc, af_t *af)
{
  int rc = 0;
  isp_stats_t *stats = sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct;

  if (af->lens_move_done == FALSE) {
    af->frame_delay--;
    if (af->frame_delay < -LENS_DONE_MOVE_THRESH)
      /*  INVAL STATE, we never got ack that lens is done moving */
      af->lens_move_done = TRUE;
    return 0;
  }
  if (stats == NULL || af->step_fail_cnt > FOCUS_ATTEMPTS) {
    af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
    return -1;  /*  Exit failure */
  }
  /*  no valid info from vfe, then return success to retry */
  if (!stats->NFocus)
    return 0;

  if (af_arrange_cur_frame_info(sproc, af) == 0)
    return 0; /* return if there is delay */

  /*  actual search start */
  if (af->state == AF_START) {
    af->lens_moved = 0;
    af_prepare_decision_making(sproc, af);
  } else if (af->state == AF_DECISION)
    af_make_decision(sproc, af);
  else if (af->state == AF_EXHAUSTIVE)
    rc = af_exhaustive_search(sproc, af);
  else if (af->state == AF_GATHER_STATS_COARSE)
    rc = af_coarse_search(sproc, af);
  else if (af->state == AF_GATHER_STATS_FINE)
    rc = af_fine_search(sproc, af);
  else if (af->state == AF_FOCUSING)
    rc = 0; /* success to focus */
  else if (af->state == AF_UNABLE_TO_FOCUS)
    rc = 0; /*  not focused but logic is ok, so retry */
  else if (af->state == AF_COLLECT_END_STAT) {
    if (af->frame_delay > -2) {
      af->frame_delay--;
      rc = 0;
    } else {
      af->locn[af->index] =
        af->cur_pos;
      af->stats[af->index++] =
        (stats->Focus / stats->NFocus);
      /*  Change state, we are done */
      af->state = AF_INACTIVE;
      sproc->share.af_ext.active = FALSE;
      rc = 0;
    }
  } else
    rc = -1; /* due to invalid state */
  /*  failure due to sensor's wrong movement,
   * so, add sensor failure count, reset and retry */
  if (rc < 0) {
    af->step_fail_cnt++;
    af_reset_lens(sproc, af); /*  Exit failure */
    af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
    return 0;
  }
  /* Unable to focus due to the noise, but functionality is ok, so,keep
   * prev_max_focus_val, reset and retry */
  if (af->state == AF_UNABLE_TO_FOCUS) {
    af->max_focus_val = af->prev_max_focus_val;
    af_reset_lens(sproc, af); /*  Exit failure */
    af->state = AF_INACTIVE;
    sproc->share.af_ext.active = FALSE;
    af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
  }
  if (af->state == AF_FOCUSING) /*  AF Completed successfully */
    af_done(sproc, af, CAMERA_EXIT_CB_DONE);

  return 0;
} /* af_hill_climbing_search */
#endif
