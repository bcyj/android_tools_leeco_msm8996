/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "chromatix.h"
#include "camera.h"
#include "awb.h"

/*===========================================================================
 * FUNCTION    - awb_agw_set_rg_bg_ratios -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_set_rg_bg_ratios(stats_proc_t *sproc, awb_t *awb)
{
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  if (awb->current_awb_stat_config == AWB_STAT_REGULAR) {
    awb->regular_ave_rg_ratio = agw_d->ave_rg_ratio;
    awb->regular_ave_bg_ratio = agw_d->ave_bg_ratio;
    CDBG_AWB("%s: regular stat: ave_rg_ratio %f, ave_bg_ratio %f", __func__,
      agw_d->ave_rg_ratio, agw_d->ave_bg_ratio);
  } else if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
    awb->white_ave_rg_ratio = agw_d->ave_rg_ratio;
    awb->white_ave_bg_ratio = agw_d->ave_bg_ratio;
    CDBG_AWB("%s: white stat: ave_rg_ratio %f, ave_bg_ratio %f", __func__,
      agw_d->ave_rg_ratio, agw_d->ave_bg_ratio);
  }
} /* awb_agw_set_rg_bg_ratios */

/*===========================================================================
 * FUNCTION    - awb_agw_reg_to_white_dec_change -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_reg_to_white_dec_change(stats_proc_t *sproc, awb_t *awb)
{
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  CDBG_AWB("reglar dec %d to white dec %d, white rg %f, bg %f",
    agw_d->sample_decision, awb->white_decision,
    awb->white_ave_rg_ratio, awb->white_ave_bg_ratio);
  agw_d->rg_ratio         = awb->white_ave_rg_ratio;
  agw_d->bg_ratio         = awb->white_ave_bg_ratio;
  agw_d->sample_decision  = awb->white_decision;
  awb->regular_decision   = awb->white_decision;
  agw_d->decision_changed = TRUE;
} /* awb_agw_reg_to_white_dec_change */

/*===========================================================================
 * FUNCTION    - awb_agw_calc_boundry_points -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_calc_boundry_points(awb_t *awb, int x_rg_grid)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  /* if a sample falls into this zone, it is likely to be a green sample in
   * outdoor lighting but it's not 100% certainty min of (CW, D50) */
  awb->agw_d.p1 = STATS_PROC_MIN(agw_p->rg_grid[AGW_AWB_INDOOR_COLD_FLO],
    agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]);
  /* (D50+D65)/2 bg_ratio */
  awb->agw_d.p2 = agw_p->green_offset_bg +
    (agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] +
    agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]) / 2;

  awb->agw_d.p3 = agw_p->bg_grid[AGW_AWB_HORIZON] + agw_p->green_offset_bg;
  awb->agw_d.p4 = ((agw_p->green_line_mx * x_rg_grid) /
    (1 << GREEN_Q_NUM) + agw_p->green_line_bx);
} /* awb_agw_calc_boundry_points */

/*===========================================================================
 * FUNCTION    - awb_agw_is_decision_outdoor -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_is_decision_outdoor(chromatix_awb_light_type decision)
{
  if (decision  == AGW_AWB_OUTDOOR_SUNLIGHT ||
    decision == AGW_AWB_OUTDOOR_SUNLIGHT1 || decision == AGW_AWB_HYBRID ||
    decision == AGW_AWB_OUTDOOR_CLOUDY || decision == AGW_AWB_OUTDOOR_NOON)
    return 0;
  else
    return -1;
} /* awb_agw_is_decision_outdoor */

/*===========================================================================
 * FUNCTION    - awb_agw_is_decision_FL -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_is_decision_FL(chromatix_awb_light_type decision)
{
  if (decision  == AGW_AWB_INDOOR_WARM_FLO || decision ==
    AGW_AWB_INDOOR_COLD_FLO || decision == AGW_AWB_INDOOR_CUSTOM_FLO)
    return 0;
  else
    return -1;
} /* awb_agw_is_decision_FL */

/* AWB HISTORY */
/*===========================================================================
 * FUNCTION    - awb_agw_history_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_history_update(stats_proc_t *sproc, awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  /* Add new entry to the history */
  agw_p->awb_history[agw_p->awb_history_next_pos].rg_ratio =
    (int)(awb->agw_d.rg_ratio * 1000.0 + 0.5);
  agw_p->awb_history[agw_p->awb_history_next_pos].bg_ratio =
    (int)(awb->agw_d.bg_ratio * 1000.0 + 0.5);
  agw_p->awb_history[agw_p->awb_history_next_pos].decision =
    awb->agw_d.sample_decision;
  agw_p->awb_history[agw_p->awb_history_next_pos].exp_index =
    (int)(sproc->share.prev_exp_index);
  agw_p->awb_history[agw_p->awb_history_next_pos].replaced = 0;
  agw_p->awb_history[agw_p->awb_history_next_pos].is_white =
    awb->current_awb_stat_config;

  /* Update buffer count and next position */
  if (agw_p->awb_history_count < AWB_MAX_HISTORY)
    agw_p->awb_history_count++;
  agw_p->awb_history_next_pos = (agw_p->awb_history_next_pos + 1)
  % AWB_MAX_HISTORY;
} /* awb_agw_history_update */

/*===========================================================================
 * FUNCTION    - awb_agw_single_color_tracking_all_outliers -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_single_color_tracking_all_outliers(stats_proc_t *sproc,
  awb_t *awb, int *outlier_rg_grid, int *outlier_bg_grid,
  int outlier_ave_rg, int outlier_ave_bg)
{
  int dist_threshod = 60; /* 60% of the max_dist */
  int i, sum_rg = 0, sum_bg = 0, cnt = 0, d1, d2, dist;
  int threshold;
  uint8_t selected[256];
  int max_dist2;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  chromatix_parms_type *cptr = sproc->input.chromatix;
  /* Currently this single color tracking only applies to not so bright */
  if (sproc->share.prev_exp_index < agw_p->indoor_index ||
    awb->agw.awb_history_count < 5)
    return 0;
  /* bright condition, do not apply this test
   * limited to indoor condition only at this moment
   * if there is not enough history entires, do not continue either. */

  threshold = (dist_threshod * awb->agw_d.max_dist + 50) / 100;

  CDBG_AWB ("max_dist %d, threshold %d", awb->agw_d.max_dist, threshold);

  memset(&selected, 0, sizeof(uint8_t) * 256);

  for (i = 0; i < awb->agw_d.outlier_cnt && i < 256; i++) {

    d1 = outlier_rg_grid[i] - outlier_ave_rg;
    d2 = outlier_bg_grid[i] - outlier_ave_bg;
    dist = d1 * d1 + d2 * d2;

    if (dist < threshold) {
      sum_rg += outlier_rg_grid[i];
      sum_bg += outlier_bg_grid[i];
      cnt++;
      selected[i] = 1;
    }
  }

  if (cnt == 0)
    return 0; /* single color rule does not apply, return and continue the */

  /* the regular green heuristics
   * recalculate the mean of the remaining outlier samples
   * and also compute the max dist */
  sum_rg = (sum_rg + (cnt >> 1)) / cnt;
  sum_bg = (sum_bg + (cnt >> 1)) / cnt;

  CDBG_AWB ("After dist thresholding, outlier ave(rg=%d, bg=%d)\n",
    sum_rg,sum_bg);
  CDBG_AWB ("After dist thresholding, outlier cnt=%d, (before=%d)\n",
    cnt, awb->agw_d.outlier_cnt);

  max_dist2 = 0;
  for (i = 0; i < awb->agw_d.outlier_cnt && i < 256; i++) {
    if (selected[i] == 1) {
      d1 = outlier_rg_grid[i] - sum_rg;
      d2 = outlier_bg_grid[i] - sum_bg;
      dist = d1 * d1 + d2 * d2;
      if (dist > max_dist2)
        max_dist2 = dist;
    }
  }
  CDBG_AWB ("outlier max_dist (before=%d, after=%d\n)",
    awb->agw_d.max_dist, max_dist2);

  if (max_dist2 < (2 * cptr->compact_cluster_r2 * 4))
    return -1; /* very likely to be single color, reject this frame */
  else
    return 0; /* continue for further analysis */
} /* awb_agw_single_color_tracking_all_outliers */

/*===========================================================================
 * FUNCTION    - awb_agw_search_nearest_reference_point -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_search_nearest_reference_point(stats_proc_t *sproc,
  awb_t *awb, int rg_grid, int bg_grid)
{
  int   index;
  int   d1, d2, dist;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  awb->agw_d.min_dist = 0x3fffffff;

  /* Search all lighting conditions */
  for (index = 0; index < AWB_NUMBER_OF_REFERENCE_POINT; index++) {
    /* fixed reference points */
    d1 = (rg_grid - agw_p->rg_grid[index]);
    d2 = (bg_grid - agw_p->bg_grid[index]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      /* set lighting condition */
      awb->agw_d.sample_decision = (chromatix_awb_light_type)index;
      awb->agw_d.min_dist = dist; /* update the new minimum distance */

    } else if (dist == awb->agw_d.min_dist)
      /* give fluorescent more preference over A & H */
      if ((awb_agw_is_decision_FL(index) == 0) &&
        (awb->agw_d.sample_decision == AGW_AWB_INDOOR_INCANDESCENT ||
        awb->agw_d.sample_decision == AGW_AWB_HORIZON))
        awb->agw_d.sample_decision = (chromatix_awb_light_type)index;
  } /* end for */
  /* LED ON, consider grey zone */
  if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF) {
    d1 = (rg_grid - agw_p->led_rg_grid);
    d2 = (bg_grid - agw_p->led_bg_grid);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT; /* set light cond */
      awb->agw_d.min_dist = dist; /* update the new minimum distance */
    }
  }
  for (index = 1; index < agw_p->n_day1 - 1 &&
    index < AGW_NUMBER_GRID_POINT; index++) {
    /* examine the daylight line 1 */
    d1 = (rg_grid - agw_p->day_line_1[index][0]);
    d2 = (bg_grid - agw_p->day_line_1[index][1]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist; /* update the new minimum distance */
      awb->agw_d.sample_decision = AGW_AWB_HYBRID;
      /* no normalization, need to set rg_adj and bg_adj to be 1 */
      awb->agw_d.day_idx = 1;
    }
  }
  for (index = 1; index < agw_p->n_day2 - 1 &&
    index < AGW_NUMBER_GRID_POINT; index++) {
    /* examine the daylight line 2 */
    d1 = (rg_grid - agw_p->day_line_2[index][0]);
    d2 = (bg_grid - agw_p->day_line_2[index][1]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist; /* update the new minimum distance */
      awb->agw_d.sample_decision = AGW_AWB_HYBRID;
      awb->agw_d.day_idx = 2;
    } /* end if */
  } /* end for */
  for (index = 0; index < agw_p->n_day3 &&
    index < AGW_NUMBER_GRID_POINT; index++) {
    /* examine the daylight line 3 */
    d1 = (rg_grid - agw_p->day3_rg[index]);
    d2 = (bg_grid - agw_p->day3_bg[index]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist;
      awb->agw_d.sample_decision = AGW_AWB_HYBRID;
      awb->agw_d.day_idx = 2; /* dont assign to line 3 as it's not accnted */
    }
  }
  for (index = 1;index < agw_p->n_fline - 1; index++) {
    /* examine the F line */
    d1 = (rg_grid - agw_p->Fline[index][0]);
    d2 = (bg_grid - agw_p->Fline[index][1]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist;
      awb->agw_d.sample_decision = AGW_AWB_INDOOR_WARM_FLO;
    } else if (dist == awb->agw_d.min_dist) {
      /* give fluorescent more preference over A & H */
      if (awb->agw_d.sample_decision == AGW_AWB_INDOOR_INCANDESCENT ||
        awb->agw_d.sample_decision == AGW_AWB_HORIZON) {

        awb->agw_d.sample_decision = AGW_AWB_INDOOR_WARM_FLO;
      }
    }
  }
  for (index = 1; index < agw_p->n_day_f_line - 1; index++) {
    /* examine the Day_F line */
    d1 = (rg_grid - agw_p->Day_F_line[index][0]);
    d2 = (bg_grid - agw_p->Day_F_line[index][1]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist;
      awb->agw_d.sample_decision = AGW_AWB_INDOOR_WARM_FLO;
    } else if (dist == awb->agw_d.min_dist) {
      /* give fluorescent more preference over D50 */
      if (awb->agw_d.sample_decision == AGW_AWB_OUTDOOR_SUNLIGHT1) {
        awb->agw_d.sample_decision = AGW_AWB_INDOOR_WARM_FLO;
      }
    }
  }
  for (index = 0; index < agw_p->n_aline1 - 1; index++) {
    /* examine the A line 1 */
    d1 = (rg_grid - agw_p->Aline1[index][0]);
    d2 = (bg_grid - agw_p->Aline1[index][1]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist;
      awb->agw_d.sample_decision = AGW_AWB_INDOOR_INCANDESCENT;
    }
  }
  for (index = 1; index < agw_p->n_aline2 - 1; index++) {
    /* examine the A line 2 */
    d1 = (rg_grid - agw_p->Aline2[index][0]);
    d2 = (bg_grid - agw_p->Aline2[index][1]);
    dist = d1 * d1 + d2 * d2;

    if (dist < awb->agw_d.min_dist) {
      awb->agw_d.min_dist = dist;
      awb->agw_d.sample_decision = AGW_AWB_INDOOR_INCANDESCENT;
    }
  }
} /* awb_agw_search_nearest_reference_point */

/*===========================================================================
 * FUNCTION    - awb_agw_history_daylight_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_history_daylight_srch(awb_t *awb)
{
  int i;
  for (i = 0; i < AWB_MAX_HISTORY; i++)
    if (awb_agw_is_decision_outdoor(awb->agw.awb_history[i].decision) == 0) {
      CDBG_AWB ("found daylight in history");
      return 0;
    }
  return -1;
} /* awb_agw_history_daylight_srch */

/*===========================================================================
 * FUNCTION    - awb_agw_aec_history_check -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_aec_history_check(awb_t *awb,
  int *max_exp, int *min_exp, int *max_luma, int *min_luma,
  int *max_exp_delta, int *max_luma_delta)
{
  int prev_exp, prev_luma, tmp;
  unsigned int i;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  i = (agw_p->aec_history_next_pos + 1) % AWB_AEC_MAX_HISTORY;
  prev_exp = agw_p->aec_history[agw_p->aec_history_next_pos].exp_index;
  prev_luma = agw_p->aec_history[agw_p->aec_history_next_pos].frame_luma;

  while (i != agw_p->aec_history_next_pos) {
    if (agw_p->aec_history[i].exp_index > *max_exp)
      *max_exp = agw_p->aec_history[i].exp_index;

    if (agw_p->aec_history[i].exp_index < *min_exp)
      *min_exp = agw_p->aec_history[i].exp_index;

    if (agw_p->aec_history[i].frame_luma > *max_luma)
      *max_luma = agw_p->aec_history[i].frame_luma;

    if (agw_p->aec_history[i].frame_luma < *min_luma)
      *min_luma = agw_p->aec_history[i].frame_luma;

    tmp = abs(agw_p->aec_history[i].exp_index - prev_exp);
    if (tmp > *max_exp_delta)
      *max_exp_delta = tmp;

    tmp = abs(agw_p->aec_history[i].frame_luma - prev_luma);
    if (tmp > *max_luma_delta)
      *max_luma_delta = tmp;

    prev_luma = agw_p->aec_history[i].frame_luma;
    prev_exp = agw_p->aec_history[i].exp_index;
    i = (i + 1) % AWB_AEC_MAX_HISTORY;
  }
} /* awb_agw_aec_history_check */

/*===========================================================================
 * FUNCTION    - awb_agw_aec_history_find_last_pos -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_aec_history_find_last_pos(awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  if (agw_p->aec_history_count == 0)
    return 0;
  else if (agw_p->aec_history_next_pos == 0)
    return(agw_p->aec_history_count - 1);
  else
    return(agw_p->aec_history_next_pos - 1);
} /* awb_agw_aec_history_find_last_pos */

/*===========================================================================
 * FUNCTION    - awb_agw_history_delete_last_entry -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_history_delete_last_entry(awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  int index;

  index = awb_util_history_find_last_pos(awb);
  agw_p->awb_history[index].rg_ratio  = 0.0;
  agw_p->awb_history[index].bg_ratio  = 0.0;
  agw_p->awb_history[index].decision  = AGW_AWB_INVALID_LIGHT;
  agw_p->awb_history[index].exp_index = -1;
  agw_p->awb_history[index].replaced  = 0;
  agw_p->awb_history[index].is_white  = -1;
  agw_p->awb_history_next_pos         = index;
  if (agw_p->awb_history_count > 0)
    agw_p->awb_history_count--;
} /* awb_agw_history_delete_last_entry */

/*===========================================================================
 * FUNCTION    - awb_agw_adjust_rg_bg_by_history -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_adjust_rg_bg_by_history(stats_proc_t *sproc, awb_t *awb)
{
  int a_cluster = 0, f_cluster = 0, day_cluster = 0, index;
  int sum = 0, exp_ave, tmp;
  int a_rg = 0, a_bg = 0, f_rg = 0, f_bg = 0, day_rg = 0, day_bg = 0;
  int a_rg_ave, a_bg_ave, f_rg_ave, f_bg_ave, day_rg_ave, day_bg_ave;
  int max_exp = 0, min_exp = 999, range;
  int stable_range_threshold = 25; /* for 3% AEC table step size */
  int max_exp_delta = 0, small_range, last_index;
  int max_luma = 0, min_luma = 255, max_luma_delta = 0, luma_range;
  int luma_range_threshold = 40, luma_delta_threshold = 20;

  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  stats_proc_aec_data_t *aec_d               = &(sproc->share.aec_ext);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);
  chromatix_parms_type *cptr   = sproc->input.chromatix;

  /* VIDEO MODE */
  if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER) {
    luma_range_threshold   = 45;
    luma_delta_threshold   = 30;
    stable_range_threshold = 35;
  } else { /* STREAMING MODE */
    luma_range_threshold   = 40;
    luma_delta_threshold   = 20;
    stable_range_threshold = 25;
  }
  CDBG_AWB("luma range thld %d, luma delta thld %d",
    luma_range_threshold, luma_delta_threshold);
  /* if the number of history entries is small, return */
  if (agw_p->awb_history_count <= 3 ||
    agw_p->aec_history_count <= 5)
    return;

  CDBG_AWB("AWB history cnt %d, AEC history cnt %d",
    agw_p->awb_history_count, agw_p->aec_history_count);
  /* first, we look at the history to see how
   * this part of the history has the data for exp, decision, rg & bg */
  for (index = 0; index < AWB_MAX_HISTORY; index++) {
    sum += agw_p->awb_history[index].exp_index;
    if (agw_p->awb_history[index].exp_index > max_exp)
      max_exp = agw_p->awb_history[index].exp_index;
    if (agw_p->awb_history[index].exp_index < min_exp)
      min_exp = agw_p->awb_history[index].exp_index;

    if (agw_p->awb_history[index].decision == AGW_AWB_HORIZON
      || agw_p->awb_history[index].decision ==
      AGW_AWB_INDOOR_INCANDESCENT) {
      a_cluster++;
      a_rg += agw_p->awb_history[index].rg_ratio;
      a_bg += agw_p->awb_history[index].bg_ratio;
    } else if (awb_agw_is_decision_FL(agw_p->
      awb_history[index].decision) == 0) {
      f_cluster++;
      f_rg += agw_p->awb_history[index].rg_ratio;
      f_bg += agw_p->awb_history[index].bg_ratio;
    } else if (awb_agw_is_decision_outdoor(agw_p->
      awb_history[index].decision) == 0) {
      day_cluster++;
      day_rg += agw_p->awb_history[index].rg_ratio;
      day_bg += agw_p->awb_history[index].bg_ratio;
    }
  }
  exp_ave = (sum + (agw_p->awb_history_count >> 1)) /
    agw_p->awb_history_count;
  small_range = max_exp - min_exp; /* exp range of the AWB/AEC history */

  /* now check AEC history (which has more items than
   * the AWB history due to frame skip) */
  max_exp = 0;
  min_exp = 999;

  awb_agw_aec_history_check(awb, &max_exp, &min_exp, &max_luma, &min_luma,
    &max_exp_delta, &max_luma_delta);

  range = max_exp - min_exp; /* exp range of the AEC history */
  luma_range = max_luma - min_luma; /* frame luma range of AEC history */

  CDBG_AWB(" AWB history: A cnt %d, F cnt %d, Day cnt %d",
    a_cluster, f_cluster, day_cluster);
  CDBG_AWB(" AWB history: complete exp range %d (max %d, min %d)",
    range, max_exp, min_exp);
  CDBG_AWB(" AWB history: max exp delta %d, exp ave %d, small range %d",
    max_exp_delta, exp_ave, small_range);
  CDBG_AWB(" AWB history: max luma delta %d, min_luma %d, max luma %d",
    max_luma_delta, min_luma, max_luma);
  CDBG_AWB(" AWB history: luma range=%d", luma_range);
  /* verify the last entry of the history and compare to the average
   * the purpose is to reduce the effect of history outliers */
  if (f_cluster > 0 && a_cluster == 0 && day_cluster == 0 &&
    agw_p->awb_history_count == AWB_MAX_HISTORY && small_range <
    (stable_range_threshold >> 2) && max_exp_delta <
    (stable_range_threshold >> 1)) {
    /* all the past history is fluorescent type and the history is full */
    int tmp_rg, tmp_bg;
    int dev_rg, dev_bg;
    index = awb_util_history_find_last_pos(awb);
    /* point to last entry fixed point number (1000 * floating point)
     * removing last entry from the summation */
    tmp_rg = f_rg - agw_p->awb_history[index].rg_ratio;
    tmp_bg = f_bg - agw_p->awb_history[index].bg_ratio;
    /* average of the history (excluding the last entry) */
    tmp_rg = (tmp_rg + ((f_cluster - 1) >> 1)) / (f_cluster - 1);
    /* average of the fluorescent cluster R/G ratio */
    tmp_bg = (tmp_bg + ((f_cluster - 1) >> 1)) / (f_cluster - 1);
    /* average of the fluorescent cluster B/G ratio */
    CDBG_AWB("AWB history: all flo, previous entres: RG ave=%d, BG ave=%d\n",
      tmp_rg, tmp_bg);
    /* compare the last entry with the average of the previous entries */
    dev_rg = (1000 * agw_p->awb_history[index].rg_ratio) / tmp_rg;
    dev_bg = (1000 * agw_p->awb_history[index].bg_ratio) / tmp_bg;
    if (dev_rg > 1060 || dev_rg < 940 || dev_bg > 1060 || dev_bg < 940) {
      CDBG_AWB("AWB history: all flo, last entry deviates too far: RG \
        dev=%d,  BG Dev=%d, replaced by prior samples\n", dev_rg, dev_bg);
      // give a 3 vs 1 weighting to adjust last entry
      agw_p->awb_history[index].rg_ratio = (3 * tmp_rg +
        agw_p->awb_history[index].rg_ratio + 2) >> 2;
      agw_p->awb_history[index].bg_ratio = (3 * tmp_bg +
        agw_p->awb_history[index].bg_ratio + 2) >> 2;
      agw_d->rg_ratio = 0.001 * (float) agw_p->awb_history[index].rg_ratio;
      agw_d->bg_ratio = 0.001 * (float) agw_p->awb_history[index].bg_ratio;
      return;
    }
  }
  /* the analysis of the history is done. Now start to do lock and unlock LOCK/
   * UNLOCK HEURISTIC 1: if the past history is mostly daylight and it's
   * bright, reject current indoor decisions */
  /* point to the last entry in history */
  if (cptr->awb_enable_lock_heuristics_1) {
    index = awb_util_history_find_last_pos(awb);
    if (max_exp <= (int)(agw_p->indoor_index - 30) && max_exp_delta <=
        stable_range_threshold && exp_ave <= (int)(agw_p->indoor_index -
            46) && day_cluster > f_cluster && day_cluster > a_cluster &&
            max_luma_delta < luma_delta_threshold &&
            luma_range < luma_range_threshold) {

      if (day_cluster > f_cluster && day_cluster > a_cluster &&
          agw_p->awb_history[index].decision != AGW_AWB_OUTDOOR_SUNLIGHT
          && agw_p->awb_history[index].decision != AGW_AWB_OUTDOOR_SUNLIGHT1
          && agw_p->awb_history[index].decision != AGW_AWB_OUTDOOR_CLOUDY
          && agw_p->awb_history[index].decision != AGW_AWB_HYBRID) {
        if (day_cluster == 0)
          day_cluster = 1;
        day_rg_ave = (day_rg + (day_cluster >> 1)) / day_cluster;
        day_bg_ave = (day_bg + (day_cluster >> 1)) / day_cluster;
        agw_d->rg_ratio = 0.001 * (float) day_rg_ave;
        agw_d->bg_ratio = 0.001 * (float) day_bg_ave;
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1;
        /* remove the last history entry */
        awb_agw_history_delete_last_entry(awb);
        CDBG_AWB("AWB history:(LH0) indoor decision is changed to D50 (day ave)"
            " by history (exp_idx=%d)\n", agw_p->awb_history[index].exp_index);

        return;
      }
    }
  }

  index = awb_util_history_find_last_pos(awb);

  /* for outdoor stability (actual outdoor condition) */
  if (day_cluster>a_cluster && day_cluster>f_cluster && exp_ave <=
    (int)(agw_p->indoor_index - 30) && max_exp_delta <
    (stable_range_threshold >> 1) && max_luma_delta < 10) {
    /* 3A 1.5 change (outdoor stability) */
    if (day_cluster == 0)
      day_cluster = 1;
    day_rg_ave = (day_rg + (day_cluster >> 1)) / day_cluster;
    day_bg_ave = (day_bg + (day_cluster >> 1)) / day_cluster;
    agw_d->rg_ratio = 0.001 * (float) day_rg_ave;
    agw_d->bg_ratio = 0.001 * (float) day_bg_ave;
    CDBG_AWB("------ AWB history: Day cluster, rg = %d, bg= %d ",
      day_rg_ave, day_bg_ave);

    if (agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_SUNLIGHT
      || agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_SUNLIGHT1
      || agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_CLOUDY
      || agw_p->awb_history[index].decision == AGW_AWB_HYBRID) {
      agw_p->awb_history[index].rg_ratio = day_rg_ave;
      agw_p->awb_history[index].bg_ratio = day_bg_ave;
      agw_p->awb_history[index].replaced = 1;
      CDBG_AWB("------ AWB history:(PVS)outdoor last entry replaced \
        rg =%d, bg= %d ", day_rg_ave,day_bg_ave);
      return;
    }
  }

  /* LOCK/UNLOCK HEURISTIC 2A: if the history is mostly fluorescent, the
   * exposure range is 0 (no change in exposure level) lock the WB gains */
  if (cptr->awb_enable_lock_heuristics_2) {
    index = awb_util_history_find_last_pos(awb);
    if (range <= 1 && small_range <= 1 && f_cluster > day_cluster && f_cluster >
  a_cluster && exp_ave < (int)aec_d->exp_tbl_val - 1 &&
  max_luma_delta < luma_delta_threshold && luma_range<luma_range_threshold) {
      /* 3A 1.4 changes (indoor fluorescent gains lock) */
      if (awb_agw_is_decision_FL(agw_p->awb_history[index].decision) == 0) {
        if (agw_p->indoor_F_WB_locked == 0) { /* not locked yet */
          agw_p->indoor_F_WB_locked = 1; /* lock it */
          f_rg_ave = (f_rg + (f_cluster >> 1)) / f_cluster;
          f_bg_ave = (f_bg + (f_cluster >> 1)) / f_cluster;
          agw_d->rg_ratio = 0.001 * (float) f_rg_ave;
          agw_d->bg_ratio = 0.001 * (float) f_bg_ave;
          agw_p->awb_history[index].replaced = 1;
          /* replace last entry of history */
          agw_p->awb_history[index].rg_ratio = f_rg_ave;
          agw_p->awb_history[index].bg_ratio = f_bg_ave;
          CDBG_AWB("history:(LH2A) F locked (exp_idx=%d, range=%d, exp_ave=%d)",
              agw_p->awb_history[index].exp_index, range, exp_ave);

          return;
        } else { /* it's already locked, use previous last entry */
          last_index = awb_util_history_find_last_pos(awb);
          agw_d->rg_ratio = 0.001 *
              (float) agw_p->awb_history[last_index].rg_ratio;
          agw_d->bg_ratio = 0.001 *
              (float) agw_p->awb_history[last_index].bg_ratio;
          /* remove current history entry */
          awb_agw_history_delete_last_entry(awb);
          CDBG_AWB("history:(LH2A) F locked (exp_idx=%d, range=%d),gain locked",
              agw_p->awb_history[index].exp_index, range);
          return;
        }
      } else {
        /* this current frame is not fluorescent, but the exposure range is 0,
         * leave it later to use preview stabilization to take care of it. */
        CDBG_AWB("------ AWB history: (LH2A) bypass current frame\n");
      }

    } else { /* break the lock */
      agw_p->indoor_F_WB_locked = 0;
      CDBG_AWB("AWB history: (LH2A)indoor_F_WB_locked reset to 0\n");
      CDBG_AWB(" range=%d, small_range=%d, exp_ave=%d",
          range, small_range, exp_ave);
    }
    /* LOCK/UNLOCK HEURISTIC 2B: in indoor situation, minor change of exposure
     * should not move from F to outdoor the blue wall on the hallway
     * can cause this to happen */
    index = awb_util_history_find_last_pos(awb);
    if (small_range < (stable_range_threshold >> 2) && max_exp_delta <
        (stable_range_threshold >> 2) && f_cluster > a_cluster && f_cluster >
    day_cluster && agw_p->awb_history[index].exp_index >=
    (int)agw_p->indoor_index && luma_range<(luma_range_threshold>>1)) {
      if (agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_SUNLIGHT
          || agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_SUNLIGHT1
          || agw_p->awb_history[index].decision == AGW_AWB_HYBRID
          || agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_NOON) {
        if (f_cluster == 0)
          f_cluster = 1;
        f_rg_ave = (f_rg + (f_cluster >> 1)) / f_cluster;
        f_bg_ave = (f_bg+ (f_cluster >> 1)) / f_cluster;
        agw_d->rg_ratio = 0.001 * (float) f_rg_ave;
        agw_d->bg_ratio = 0.001 * (float) f_bg_ave;
        agw_d->sample_decision = AGW_AWB_INDOOR_WARM_FLO; /*TL84; */
        awb_agw_history_delete_last_entry(awb);

        /* change the history, but exp_index is not modified. */
        agw_p->awb_history[index].replaced = 1;
        agw_p->awb_history[index].rg_ratio = f_rg_ave;
        agw_p->awb_history[index].bg_ratio = f_bg_ave;
        agw_p->awb_history[index].decision =
            AGW_AWB_INDOOR_WARM_FLO;

        CDBG_AWB("AWB history: (LH2)outdoor decision is changed to TL84 by \
        history (exp_idx=%d, range=%d)\n",
        agw_p->awb_history[index].exp_index, range);
        return;
      }
    }
  }
  /* LOCK/UNLOCK HEURISTIC 3: if the past history is mostly flurorescent, and
   * the exposure is quite stable, reject the decision of A and HG */
  if (cptr->awb_enable_lock_heuristics_3) {
    index = awb_agw_aec_history_find_last_pos(awb);
    if (range < stable_range_threshold && max_exp_delta <
        (stable_range_threshold >> 1) && f_cluster > day_cluster && f_cluster >
    a_cluster && agw_p->aec_history[index].exp_index >=
    awb->inoutdoor_midpoint && max_luma_delta <
    luma_delta_threshold && luma_range < luma_range_threshold) {
      /* F cluster is dominant exposure variation range is less than a threshold
       * max exposure delta is less than a threshold */
      index = awb_util_history_find_last_pos(awb);
      if (agw_p->awb_history[index].decision == AGW_AWB_INDOOR_INCANDESCENT
          || agw_p->awb_history[index].decision == AGW_AWB_HORIZON) {
        if (f_cluster > 0) {
          f_rg_ave = (f_rg + (f_cluster >> 1)) / f_cluster;
          f_bg_ave = (f_bg + (f_cluster >> 1)) / f_cluster;
          agw_d->rg_ratio = 0.001 * (float) f_rg_ave;
          agw_d->bg_ratio = 0.001 * (float) f_bg_ave;
          agw_d->sample_decision = AGW_AWB_INDOOR_WARM_FLO;
          awb_agw_history_delete_last_entry(awb);
          CDBG_AWB("AWB history: (LH3) A or H decision is changed to TL84 by \
          history (exp_idx=%d, range=%d)\n",
          agw_p->awb_history[index].exp_index, range);
          return;
        }
      }
    }
  }
  /* the following logic is to remove sudden decision jitter to increase
   * preview stability. The following code is for preview stability for
   * the decision in the same category */
  /* point to the last entry in the history */
  index = awb_util_history_find_last_pos(awb);
  if (small_range <= 15 && luma_range <= 15 && max_luma_delta <= 3) {
    /* it's considered a stable exposure history, should lock */
    if (a_cluster > f_cluster && a_cluster > day_cluster) {
      /* A cluster is dominant */
      if (a_cluster == 0)
        a_cluster = 1;
      a_rg_ave = (a_rg + (a_cluster >> 1)) / a_cluster;
      a_bg_ave=(a_bg + (a_cluster >> 1)) / a_cluster;
      agw_d->rg_ratio=0.001 * (float) a_rg_ave;
      agw_d->bg_ratio=0.001 * (float) a_bg_ave;
      CDBG_AWB("------ AWB history: A cluster is dominant, rg = %d, bg= %d\n",
        a_rg_ave, a_bg_ave);

    } else if (f_cluster > a_cluster && f_cluster > day_cluster) {
      /* F cluster is dominant */
      if (f_cluster == 0)
        f_cluster = 1;
      f_rg_ave = (f_rg + (f_cluster >> 1)) / f_cluster;
      f_bg_ave = (f_bg + (f_cluster >> 1)) / f_cluster;
      agw_d->rg_ratio = 0.001 * (float) f_rg_ave;
      agw_d->bg_ratio = 0.001 * (float) f_bg_ave;
      CDBG_AWB("------ AWB history: F cluster is dominant, rg = %d, bg= %d\n",
        f_rg_ave, f_bg_ave);

      if (awb_agw_is_decision_FL(agw_p->awb_history[index].decision) == 0) {
        /* current decision is F */
        agw_p->awb_history[index].rg_ratio = f_rg_ave;
        agw_p->awb_history[index].bg_ratio = f_bg_ave;
        agw_p->awb_history[index].replaced = 1;
        CDBG_AWB("AWB history: (PVS)last entry replaced>> rg = %d, bg= %d\n",
          f_rg_ave, f_bg_ave);
      } else {
        /* current decision is not flureoscent */
        tmp = agw_p->indoor_index;
        if (max_exp_delta <= 2 && exp_ave >= tmp &&
          agw_p->awb_history[index].exp_index >= tmp) {
          agw_p->awb_history[index].rg_ratio = f_rg_ave;
          agw_p->awb_history[index].bg_ratio = f_bg_ave;
          agw_p->awb_history[index].decision = AGW_AWB_INDOOR_WARM_FLO;
          agw_d->sample_decision = AGW_AWB_INDOOR_WARM_FLO;
          agw_p->awb_history[index].replaced = 1;
          CDBG_AWB("(PVS)last entry changed, decision=%d --> TL84\n",
            agw_d->sample_decision);
        }
      }
    } else if (day_cluster > a_cluster && day_cluster > f_cluster) {
      /* day cluster is dominant (light box condition) */
      if (day_cluster == 0)
        day_cluster = 1;
      day_rg_ave = (day_rg + (day_cluster >> 1)) / day_cluster;
      day_bg_ave = (day_bg + (day_cluster >> 1)) / day_cluster;
      agw_d->rg_ratio = 0.001 * (float) day_rg_ave;
      agw_d->bg_ratio = 0.001 * (float) day_bg_ave;
      CDBG_AWB("------ AWB history: Day cluster, rg = %d, bg= %d\n",
        day_rg_ave, day_bg_ave);

      if (agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_SUNLIGHT
        || agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_SUNLIGHT1
        || agw_p->awb_history[index].decision == AGW_AWB_OUTDOOR_CLOUDY
        || agw_p->awb_history[index].decision == AGW_AWB_HYBRID) {
        agw_p->awb_history[index].rg_ratio = day_rg_ave;
        agw_p->awb_history[index].bg_ratio = day_bg_ave;
        agw_p->awb_history[index].replaced = 1;
        CDBG_AWB("AWB history: (PVS)last entry replaced>> rg = %d, bg= %d\n",
          day_rg_ave, day_bg_ave);
      }
    }
  } else if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER &&
    luma_range < (luma_range_threshold >> 2) && small_range <
    (stable_range_threshold >> 2) && range < (stable_range_threshold >> 2)) {
    /* avg history entries & return the avg back to calling function */
    sum = a_cluster + day_cluster + f_cluster;
    agw_d->rg_ratio = 0.001 * (float) ((a_rg + f_rg + day_rg +
      (sum >> 1)) / sum);
    agw_d->bg_ratio = 0.001 * (float) ((a_bg + f_bg + day_bg +
      (sum >> 1)) / sum);
  }
  CDBG_AWB("%s: after history adjustment: rg %f, bg %f", __func__,
    agw_d->rg_ratio, agw_d->bg_ratio);
} /* awb_agw_adjust_rg_bg_by_history */

/*===========================================================================
 * FUNCTION    - awb_agw_history_get_daylight -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_history_get_daylight(awb_t *awb)
{
  int i, sum_rg = 0, sum_bg = 0, cnt = 0;

  for (i = 0; i < AWB_MAX_HISTORY; i++) {
    if (awb_agw_is_decision_outdoor(awb->agw.awb_history[i].decision) == 0) {
      sum_rg += awb->agw.awb_history[i].rg_ratio;
      sum_bg += awb->agw.awb_history[i].bg_ratio;
      cnt++;
      break;
    }
  }
  if (cnt == 0)
    return 0;
  else {
    sum_rg = (sum_rg + (cnt >> 1)) / cnt;
    sum_bg = (sum_bg + (cnt >> 1)) / cnt;
    awb->agw_d.rg_ratio = 0.001 * (float) sum_rg;
    awb->agw_d.bg_ratio = 0.001 * (float) sum_bg;
    CDBG_AWB ("daylight history: rg %d, bg %d, cnt %d",sum_rg, sum_bg, cnt);
    return cnt;
  }
} /* awb_agw_history_get_daylight */

/*===========================================================================
 * FUNCTION    - awb_agw_set_decision -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_set_decision(awb_t *awb)
{
  if (awb->current_awb_stat_config == AWB_STAT_WHITE)
    awb->white_decision = awb->agw_d.sample_decision;
  if (awb->current_awb_stat_config == AWB_STAT_REGULAR)
    awb->regular_decision = awb->agw_d.sample_decision;
} /* awb_agw_set_decision */

/*===========================================================================
 * FUNCTION    - awb_agw_is_custom_flo_close_to_daylight -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_is_custom_flo_close_to_daylight(awb_t *awb)
{
  int d1, d2;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  /* if the bg_ratio of custom flo is > D50_bg, it's close to daylight */
  if (agw_p->bg_grid[AGW_AWB_INDOOR_CUSTOM_FLO] >= agw_p->bg_grid[
    AGW_AWB_OUTDOOR_SUNLIGHT1] || agw_p->rg_grid[
    AGW_AWB_INDOOR_CUSTOM_FLO] <
    agw_p->rg_grid[AGW_AWB_INDOOR_COLD_FLO]) {
    CDBG_AWB ("custom flo close to daylight\n");
    return 0;
  }

  /* if custom flo is close to D50 and its bg ration are > than CW & TL84 */
  d1 = (agw_p->rg_grid[AGW_AWB_INDOOR_CUSTOM_FLO] -
    agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]);
  d2 = (agw_p->bg_grid[AGW_AWB_INDOOR_CUSTOM_FLO] -
    agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]);

  if ((d1 * d1 + d2 * d2) <= 10 && agw_p->bg_grid[
    AGW_AWB_INDOOR_CUSTOM_FLO] > agw_p->bg_grid[
    AGW_AWB_INDOOR_COLD_FLO] && agw_p->bg_grid[
    AGW_AWB_INDOOR_CUSTOM_FLO] >
    agw_p->bg_grid[AGW_AWB_INDOOR_WARM_FLO]) {

    CDBG_AWB ("custom flo close to daylight\n");
    return 0;
  } else
    return -1;

} /* awb_agw_is_custom_flo_close_to_daylight */

/*===========================================================================
 * FUNCTION    - awb_agw_history_is_daylight_dominant -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_history_is_daylight_dominant(awb_advanced_grey_world_t 
  *agw_p)
{
  int index, a_cluster = 0, day_cluster = 0, f_cluster = 0;

  for (index = 0; index < AWB_MAX_HISTORY; index++) {
    if (agw_p->awb_history[index].decision == AGW_AWB_HORIZON ||
      agw_p->awb_history[index].decision == AGW_AWB_INDOOR_INCANDESCENT)
      a_cluster++;
    else if (agw_p->awb_history[index].decision == AGW_AWB_INDOOR_WARM_FLO ||
      agw_p->awb_history[index].decision == AGW_AWB_INDOOR_COLD_FLO ||
      agw_p->awb_history[index].decision == AGW_AWB_INDOOR_CUSTOM_FLO)
      f_cluster++;
    else
      day_cluster++;
  }
  if (day_cluster > f_cluster && day_cluster > a_cluster)
    return 1;
  else
    return 0;
} /* awb_agw_history_is_daylight_dominant */

/*===========================================================================
 * FUNCTION    - awb_agw_aec_history_find_min_exp -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_aec_history_find_min_exp(awb_t *awb)
{
  int i;
  int min_index = 9999;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  if (agw_p->aec_history_count == 0)
    return min_index;

  for (i = 0; i < agw_p->aec_history_count &&
    i < AWB_AEC_MAX_HISTORY; i++) {
    if (agw_p->aec_history[i].exp_index < min_index)
      min_index = agw_p->aec_history[i].exp_index;
  }
  return min_index;
} /* awb_agw_aec_history_find_min_exp */

/*===========================================================================
 * FUNCTION    - awb_agw_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_init(stats_proc_t *sproc, awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  memset(agw_p->outlier_rg_grid, 0, sizeof(int) * 256);
  memset(agw_p->outlier_bg_grid, 0, sizeof(int) * 256);
  memset(sproc->share.stat_sample_decision, -1, sizeof(int)*64);
  memset(agw_d, 0, sizeof(awb_advanced_grey_world_algo_data_t));
  agw_d->gx = 1;
  agw_d->sample_decision = AGW_AWB_INVALID_LIGHT;
  agw_d->r_gain = 1.0;
  agw_d->g_gain = 1.0;
  agw_d->b_gain = 1.0;
  agw_d->d55_rg = 1.0;
  agw_d->d55_bg = 1.0;
  agw_d->f_idx  = -1;
  agw_d->a_idx  = -1;
  agw_d->a_line = -1;
  agw_d->rg_target = 0.0;
  agw_d->bg_target = 0.0;
  agw_d->rg_ratio_x = 1;
  agw_d->bg_ratio_x = 1;
  agw_d->rg_ratio = 1.0;
  agw_d->bg_ratio = 1.0;

  agw_d->output.is_confident = FALSE; /* not confident as a default */
  agw_d->output.gain_adj_b = 1.0;
  agw_d->output.gain_adj_r = 1.0;
  agw_d->output.wb_gain_r  = 1.0;
  agw_d->output.wb_gain_g  = 1.0;
  agw_d->output.wb_gain_b  = 1.0;
  agw_d->output.sample_decision = AGW_AWB_INVALID_LIGHT;
} /* awb_agw_init */

/*===========================================================================
 * FUNCTION    - awb_agw_aec_settle_check -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_aec_settle_check(stats_proc_t *sproc, awb_t *awb)
{
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  if (sproc->share.prev_exp_index == 0) {
    CDBG_HIGH("%s: ERROR: exposure index is 0 !!!", __func__);
    return -1;
  }
  agw_d->aec_luma_delta = abs((int)aec_d->target_luma -
    (int) sproc->share.aec_ext.cur_luma);

  if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
    agw_d->aec_luma_delta > 15 &&
    sproc->share.prev_exp_index < (int)aec_d->exp_tbl_val - 1) {
    /* AEC not stable yet. Discard white world frame */
    CDBG_AWB("AEC not stablized yet. Reject white world frame\n");
    return -1;
  }
  return 0;
} /* awb_agw_aec_settle_check */

/*===========================================================================
 * FUNCTION    - awb_agw_classify_samples -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_classify_samples(stats_proc_t *sproc, awb_t *awb)
{
  int index;
  awb_input_stats_type *stats  = &(awb->stats_ptr);
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  for (index = 0; index < stats->bin_cnt && index < 64; index++) {
    if (stats->rx[index] < 0 || stats->gx[index] < 0 || stats->bx[index] < 0) {
      /* Detect erroneuous stat and reject the frame, , 3A 2.0 */
      CDBG_HIGH("%s: Invalid stat: r %d, g %d, b %d, reject this frame",
        __func__, stats->rx[index], stats->gx[index], stats->bx[index]);
      return -1;
    }
    agw_d->g_ave += agw_d->gx;
    if (agw_d->gx > agw_d->g_max)
      agw_d->g_max = agw_d->gx;

    /* R, G, B are Q10 numbers */
    if (stats->gx[index] != 0) /* gx already initialized to 1 */
      agw_d->gx = stats->gx[index];

    agw_d->rg_ratio_x = ((stats->rx[index] << RGBG_GRID_Q_NUM) +
      (agw_d->gx >> 1)) / agw_d->gx;
    agw_d->bg_ratio_x = ((stats->bx[index] << RGBG_GRID_Q_NUM) +
      (agw_d->gx >> 1)) / agw_d->gx;

    agw_d->simple_rg_ratio_x += agw_d->rg_ratio_x;
    agw_d->simple_bg_ratio_x += agw_d->bg_ratio_x;

    if (agw_d->rg_ratio_x > 4000 || agw_d->bg_ratio_x > 4000) {
      /* Detect erroneuous stat and reject the frame, 3A 2.0 */
      CDBG_HIGH("%s: Invalid stat: rg_ratio_x %d, bg_ratio_x %d, reject frame",
        __func__, agw_d->rg_ratio_x, agw_d->bg_ratio_x);
      return -1;
    }

    /* compute the grid coordinate on the 2D plot */
    awb_util_convert_to_grid (awb, agw_d->rg_ratio_x,
      agw_d->bg_ratio_x, &agw_d->x_rg_grid, &agw_d->x_bg_grid);

    awb_agw_calc_boundry_points(awb, agw_d->x_rg_grid);

    if (agw_d->x_bg_grid <= agw_d->p4 && agw_d->x_rg_grid >  0 &&
      agw_d->x_rg_grid <= (agw_d->p1 + agw_p->green_offset_rg * 2) &&
      agw_d->x_bg_grid < agw_d->p2 && agw_d->x_bg_grid > agw_d->p3) {
      agw_d->green_cnt++;
      agw_d->green_bgx += agw_d->x_bg_grid;
    }
    /* count the samples that may be indoor green pixels */
    if (agw_d->x_rg_grid < agw_d->p1 &&  agw_d->x_bg_grid <= agw_d->p3)
      agw_d->indoor_green_cnt++;

    if (agw_d->x_bg_grid <  agw_p->bg_grid[AGW_AWB_HORIZON] &&
      agw_d->x_rg_grid >= agw_d->p1 &&\
      agw_d->x_rg_grid < agw_p->rg_grid[AGW_AWB_INDOOR_INCANDESCENT])
      agw_d->indoor_green_cnt++;

    /* the outdoor green and indoor green have some overlap.
     * they are not 100% clear cut then, test whether this point is an outlier
     * first, comparing it with the all reference points */
    awb_agw_search_nearest_reference_point (sproc, awb, agw_d->x_rg_grid,
      agw_d->x_bg_grid);

    /* added for backlight detection */
    sproc->share.stat_sample_decision[index] = agw_d->sample_decision;

    if (agw_d->min_dist > (agw_p->outlier_distance * 2 *
      agw_p->outlier_distance * 2)) {
      /* for backlight detection */
      sproc->share.stat_sample_decision[index] = -1;  /* outliers */
    }

    /* if the minimum distance from sample to the reference points are
     * greater than the threshold, it's an outlier */
    if ((agw_d->min_dist > (agw_p->outlier_distance *
      agw_p->outlier_distance)) ||
      agw_d->x_bg_grid > agw_p->bg_grid[AGW_AWB_OUTDOOR_CLOUDY] ||
      agw_d->x_rg_grid > (agw_p->rg_grid[AGW_AWB_HORIZON] + 3) ||
      agw_d->x_bg_grid < (agw_p->bg_grid[AGW_AWB_HORIZON] - 3)) {
      /* collect outliers */
      agw_p->outlier_rg_grid[agw_d->outlier_cnt] = agw_d->x_rg_grid;
      agw_p->outlier_bg_grid[agw_d->outlier_cnt] = agw_d->x_bg_grid;
      agw_d->outlier_cnt++;
    } else {
      /* collect valid samples */
      agw_p->sample_rg_grid[agw_d->smpl_cnt] = agw_d->x_rg_grid;
      agw_p->sample_bg_grid[agw_d->smpl_cnt] = agw_d->x_bg_grid;
      agw_d->smpl_cnt++;
      /* continuous weight vector */
      if (sproc->share.prev_exp_index >= agw_p->indoor_index) {
        /* dark situation, use indoor weighting */
        if ((int)agw_d->sample_decision >= 0)
          agw_d->wt_per_smpl =  agw_p->awb_weight_vector[agw_d->sample_decision][0];
        else
          agw_d->wt_per_smpl = 0;

      } else if (sproc->share.prev_exp_index >= awb->inoutdoor_midpoint) {
        /* indoor < idx < inout mid */
        agw_d->p1 = sproc->share.prev_exp_index - awb->inoutdoor_midpoint;
        agw_d->p2 = agw_p->indoor_index - sproc->share.prev_exp_index;
        if ((int)agw_d->sample_decision >= 0)
          agw_d->wt_per_smpl = (agw_d->p1 *
            agw_p->awb_weight_vector[agw_d->sample_decision][0]
            + agw_d->p2 * agw_p->awb_weight_vector[agw_d->sample_decision][2]) /
            (agw_d->p1 + agw_d->p2);
        else
          agw_d->wt_per_smpl  = 0;

      } else if (sproc->share.prev_exp_index >= agw_p->outdoor_index) {
        /* inout mid < idx < outdoor */
        agw_d->p1 = sproc->share.prev_exp_index - agw_p->outdoor_index;
        agw_d->p2 = awb->inoutdoor_midpoint - sproc->share.prev_exp_index;
        if ((int)agw_d->sample_decision >= 0)
          agw_d->wt_per_smpl = (agw_d->p1 *
            agw_p->awb_weight_vector[agw_d->sample_decision][2] +
            agw_d->p2 * agw_p->awb_weight_vector[agw_d->sample_decision][1]) /
            (agw_d->p1 + agw_d->p2);
        else
          agw_d->wt_per_smpl  = 0;

      } else if (sproc->share.prev_exp_index >= awb->outdoor_midpoint) {
        /* outdoor < idx < out mid */
        agw_d->p1 = sproc->share.prev_exp_index - awb->outdoor_midpoint;
        agw_d->p2 = agw_p->outdoor_index - sproc->share.prev_exp_index;
        if ((int)agw_d->sample_decision >= 0)
          agw_d->wt_per_smpl = (agw_d->p1 *
            agw_p->awb_weight_vector[agw_d->sample_decision][2] +
            agw_d->p2 * agw_p->awb_weight_vector[agw_d->sample_decision][1]) /
            (agw_d->p1 + agw_d->p2);
        else
          agw_d->wt_per_smpl  = 0;

      } else { /* idx < outdoor_midpoint */
        agw_d->p1 = sproc->share.prev_exp_index;
        agw_d->p2 = awb->outdoor_midpoint - sproc->share.prev_exp_index;
        if (agw_d->sample_decision == AGW_AWB_OUTDOOR_CLOUDY) /* shade D75 */
          agw_d->wt_per_smpl = (agw_d->p1 *
            agw_p->awb_weight_vector[agw_d->sample_decision][1] +
            agw_d->p2 * 1) / (agw_d->p1 + agw_d->p2);
        else if (agw_d->sample_decision == AGW_AWB_OUTDOOR_SUNLIGHT)
          agw_d->wt_per_smpl = (agw_d->p1 *
            agw_p->awb_weight_vector[agw_d->sample_decision][1] +
            agw_d->p2 * 2) / (agw_d->p1 + agw_d->p2);
        else if (agw_d->sample_decision == AGW_AWB_HYBRID)
          /* Daylight lines */
          if (agw_d->day_idx == 1)
            agw_d->wt_per_smpl = (agw_d->p1 *
              agw_p->awb_weight_vector[agw_d->sample_decision][1] +
              agw_d->p2 * 2) / (agw_d->p1 + agw_d->p2);
          else
            agw_d->wt_per_smpl = (agw_d->p1 * agw_p->awb_weight_vector[agw_d->
              sample_decision][1] + agw_d->p2 * agw_p->
              awb_weight_vector[agw_d->sample_decision][1] / 3) /
              (agw_d->p1 + agw_d->p2);
        else
          agw_d->wt_per_smpl  = agw_p->awb_weight_vector[agw_d->sample_decision][1];
      }
      /* compute weighted R/G and B/G for valid samples */
      agw_d->x_cnt          += (agw_d->wt_per_smpl);
      agw_d->ave_rg_ratio_x += (agw_d->rg_ratio_x * agw_d->wt_per_smpl);
      agw_d->ave_bg_ratio_x += (agw_d->bg_ratio_x * agw_d->wt_per_smpl);

      /* classify the valid samples into 3 different cluster */
      if (agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT ||
        agw_d->sample_decision == AGW_AWB_HORIZON) {
        /* A light cluster */
        agw_d->a_cluster++;
        agw_d->a_cluster_rg_x += agw_d->rg_ratio_x;
        agw_d->a_cluster_bg_x += agw_d->bg_ratio_x;
      } else if (awb_agw_is_decision_FL(agw_d->sample_decision) == 0) {
        /* Fluorescent light cluster */
        agw_d->f_cluster++;
        agw_d->f_cluster_rg_x += agw_d->rg_ratio_x;
        agw_d->f_cluster_bg_x += agw_d->bg_ratio_x;
      } else { /* outdoor daylight cluster */
        agw_d->day_cluster++;
        agw_d->day_cluster_rg_x += agw_d->rg_ratio_x;
        agw_d->day_cluster_bg_x += agw_d->bg_ratio_x;
      }
    }
  } /* end of loop index */
  return 0;
} /* awb_agw_classify_samples */

/*===========================================================================
 * FUNCTION    - awb_agw_compute_rg_bg_for_sgw -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_compute_rg_bg_for_sgw(stats_proc_t *sproc, awb_t *awb)
{
  awb_input_stats_type *stats  = &(awb->stats_ptr);

  awb->agw_d.simple_rg_ratio_x = (awb->agw_d.simple_rg_ratio_x +
    (stats->bin_cnt >> 1)) / stats->bin_cnt;
  awb->agw_d.simple_bg_ratio_x = (awb->agw_d.simple_bg_ratio_x +
    (stats->bin_cnt >> 1)) / stats->bin_cnt;
  awb->agw_d.simple_rg_ratio = (float) awb->agw_d.simple_rg_ratio_x *
    (1.0 / (float) (1 << RGBG_GRID_Q_NUM));
  awb->agw_d.simple_bg_ratio = (float) awb->agw_d.simple_bg_ratio_x *
    (1.0 / (float) (1 << RGBG_GRID_Q_NUM));
} /* awb_agw_compute_rg_bg_for_sgw */

/*===========================================================================
 * FUNCTION    - awb_agw_process_low_light -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_process_low_light(stats_proc_t *sproc, awb_t *awb)
{
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  CDBG_AWB("current frame luma %d", sproc->share.aec_ext.cur_luma);
  /* It's very dark. max_exp_idx is used, but frame luma is still very low */
  if (sproc->share.aec_ext.cur_luma == 0 &&
    sproc->share.prev_exp_index == (int)aec_d->exp_tbl_val - 1) {
    if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
      CDBG_HIGH("%s: scene is too dark, reject", __func__);
      return -1;
    } else if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF &&
      sproc->input.flash_info.led_mode != LED_MODE_OFF &&
      sproc->input.flash_info.led_mode != LED_MODE_TORCH) {
      /* have to make decision, use LED's RGB gains for WB */
      agw_d->rg_ratio = agw_p->led_rg_ratio;
      agw_d->bg_ratio = agw_p->led_bg_ratio;
      agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
      /* LED is normally daylight, */
      agw_d->g_gain = 1.0;
      agw_d->r_gain = 1.0 / agw_d->rg_ratio;
      agw_d->b_gain = 1.0 / agw_d->bg_ratio;
      agw_d->low_light = TRUE;
      CDBG_AWB("lowlight & LED ON: use LED ref point for WB (r,g,b)gain "
        "(%f,%f,%f)", agw_d->r_gain, agw_d->g_gain, agw_d->b_gain);
    }
  }

  /* if LED is turned on for this frame, do not use green sample as well. */
  if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_TORCH) {
    agw_d->green_cnt = 0;
    CDBG_AWB("green sample cnt set to 0\n");
  }
  if ((sproc->share.prev_exp_index == (int)aec_d->exp_tbl_val - 1) &&
    agw_d->day_cluster == 0 && agw_d->f_cluster == 0 && agw_d->a_cluster == 0) {
    /* low light & all of them are outliers */
    awb_util_convert_to_grid(awb, agw_d->simple_rg_ratio_x,
      agw_d->simple_bg_ratio_x, &agw_d->x_rg_grid, &agw_d->x_bg_grid);

    awb_agw_search_nearest_reference_point(sproc, awb, agw_d->x_rg_grid,
      agw_d->x_bg_grid);

    if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
      if (agw_d->min_dist > (agw_p->outlier_distance *
        agw_p->outlier_distance * 2)) {
        /* the collected sample average is too far from any reference point.
         * Too dangerous to perform SGW. reject this frame */
        CDBG_HIGH("%s: all outliers, SGW ave to nearest ref point %d, "
          "low confidence, reject\n", __func__, agw_d->min_dist);
        return -1;
      }
      agw_d->rg_ratio = agw_d->simple_rg_ratio;
      agw_d->bg_ratio = agw_d->simple_bg_ratio;
      CDBG_AWB("lowlight: use simple grey world. set decision=%d\n",
        agw_d->sample_decision);
    } else { /* LED is ON */
      if (sproc->input.flash_info.led_mode != LED_MODE_OFF &&
        sproc->input.flash_info.led_mode != LED_MODE_TORCH) {
        agw_d->rg_ratio = agw_p->led_rg_ratio;
        agw_d->bg_ratio = agw_p->led_bg_ratio;
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
        CDBG_AWB("lowlight: use LED. set decision=%d\n", agw_d->sample_decision);
      } else { /* LED ON, but not auto mode (it's UI selected ON mode) */
        CDBG_HIGH("%s: all outliers, LED is ON, reject", __func__);
        return -1;
      }
    }
    agw_d->g_gain    = 1.0;
    agw_d->r_gain    = 1.0 / agw_d->rg_ratio;
    agw_d->b_gain    = 1.0 / agw_d->bg_ratio;
    agw_d->low_light = TRUE;
    CDBG_AWB("lowlight: use SGW. set decision %d", agw_d->sample_decision);
  }
  return 0;
} /* awb_agw_process_low_light */

/*===========================================================================
 * FUNCTION    - awb_agw_find_dominant_cluster -
 *
 * DESCRIPTION:  Find dominant cluster & calculates RG BG Ratios based on
 *               dominat cluster or weight vector
 *==========================================================================*/
static void awb_agw_find_dominant_cluster(stats_proc_t *sproc, awb_t *awb)
{
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  /* normal samples there are some valid samples this heuristics is for
   * stable performance for MCC in lightbox make decision based on
   * cluster statistics */
  if (agw_d->a_cluster > ((cptr->a_cluster_threshold * agw_d->smpl_cnt) / 100)
    && sproc->share.prev_exp_index >= awb->inoutdoor_midpoint) {
    /* A cluster is dominant, not too bright use only A cluster for WB  */
    if (agw_d->a_cluster == 0)
      agw_d->a_cluster = 1;
    agw_d->ave_rg_ratio_x = (agw_d->a_cluster_rg_x +
      (agw_d->a_cluster >> 1)) / agw_d->a_cluster;
    agw_d->ave_bg_ratio_x = (agw_d->a_cluster_bg_x +
      (agw_d->a_cluster >> 1)) / agw_d->a_cluster;
    CDBG_HIGH("%s: A cluster is dominant", __func__);
  } else if (agw_d->f_cluster > ((cptr->f_cluster_threshold *
    agw_d->smpl_cnt) / 100)
    && sproc->share.prev_exp_index >= agw_p->indoor_index) {
    /* F cluster is dominant, not too bright use only F cluster for WB */
    if (agw_d->f_cluster == 0)
      agw_d->f_cluster = 1;
    agw_d->ave_rg_ratio_x = (agw_d->f_cluster_rg_x +
      (agw_d->f_cluster >> 1)) / agw_d->f_cluster;
    agw_d->ave_bg_ratio_x = (agw_d->f_cluster_bg_x +
      (agw_d->f_cluster >> 1)) / agw_d->f_cluster;
    CDBG_HIGH("%s: F cluster is dominant", __func__);
  } else if (agw_d->day_cluster > ((cptr->day_cluster_threshold *
    agw_d->smpl_cnt) / 100)) {
    /* day cluster is the dominant, use only Day cluster for WB */
    if (agw_d->day_cluster == 0)
      agw_d->day_cluster = 1;
    agw_d->ave_rg_ratio_x =(agw_d->day_cluster_rg_x +
      (agw_d->day_cluster >> 1)) / agw_d->day_cluster;
    agw_d->ave_bg_ratio_x =(agw_d->day_cluster_bg_x +
      (agw_d->day_cluster >> 1)) / agw_d->day_cluster;
    CDBG_HIGH("%s: Day cluster is dominant", __func__);
  } else { /* use weight vector */
    if (agw_d->x_cnt == 0)
      agw_d->x_cnt = 1;
    agw_d->ave_rg_ratio_x = (agw_d->ave_rg_ratio_x +
      (agw_d->x_cnt >> 1)) / agw_d->x_cnt;
    agw_d->ave_bg_ratio_x = (agw_d->ave_bg_ratio_x +
      (agw_d->x_cnt >> 1)) / agw_d->x_cnt;
    CDBG_HIGH("%s: No dominant cluster, use Weight Vector", __func__);
  }
  agw_d->ave_rg_ratio = (float) agw_d->ave_rg_ratio_x * (1.0 /
    (float) (1 << RGBG_GRID_Q_NUM));
  agw_d->ave_bg_ratio = (float) agw_d->ave_bg_ratio_x * (1.0 /
    (float) (1 << RGBG_GRID_Q_NUM));

  awb_agw_set_rg_bg_ratios(sproc, awb);
} /* awb_agw_find_dominant_cluster */

/*===========================================================================
 * FUNCTION    - awb_agw_outlier_hueristics -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_outlier_hueristics(stats_proc_t *sproc, awb_t *awb)
{
  int index, d1, d2, dist;
  float gain_min;

  awb_input_stats_type *stats  = &(awb->stats_ptr);
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER) {
    /* VIDEO MODE & all outliers. return. don't try to make decision */
    CDBG_HIGH("%s: VIDEO MODE and all outliers, return", __func__);
    return -1;
  }
  if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF  &&
    sproc->input.flash_info.led_mode != LED_MODE_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_TORCH) {
    /* LED is on, all outliers, then use LED referende point
     * modify the global variables */
    agw_d->g_gain = 1.0;
    agw_d->r_gain = 1.0 / agw_p->led_rg_ratio;
    agw_d->b_gain = 1.0 / agw_p->led_bg_ratio;
    gain_min = STATS_PROC_MIN(agw_d->g_gain,
      STATS_PROC_MIN(agw_d->r_gain, agw_d->b_gain));
    agw_d->r_gain = agw_d->r_gain * (1.0 / gain_min);
    agw_d->g_gain = agw_d->g_gain * (1.0 / gain_min);
    agw_d->b_gain = agw_d->b_gain * (1.0 / gain_min);
    CDBG_HIGH("%s: LED ON, all outliers, D65: r_gain %f, g_gain %f, b_gain"
      " %f", __func__, agw_d->r_gain, agw_d->g_gain, agw_d->b_gain);

    agw_d->output.is_confident = TRUE;
    /* confident to use the gains and decision */
    agw_d->output.sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT; /* D65 */
    agw_d->output.wb_gain_r = agw_d->r_gain;
    agw_d->output.wb_gain_g = agw_d->g_gain;
    agw_d->output.wb_gain_b = agw_d->b_gain;
    return -1;
  }
  if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
    awb->white_decision        = AGW_AWB_INVALID_LIGHT;
    awb->white_has_single_peak = FALSE;
    if (awb->white_y_min_percent >= YMIN_LOW_LIMIT)
      awb->white_y_min_percent--;
    CDBG_HIGH("%s: white stat : all outliers. Ymin pct %d return",
      __func__, awb->white_y_min_percent);
    return -1;
  }
  /* when max exposure index is used and it's all outliers situation,
   * the current frame is not trustworthy. Reject */
  if (sproc->share.prev_exp_index == (int)aec_d->exp_tbl_val - 1) {
    CDBG_HIGH("all outliers, too dark, reject");
    return -1;
  }
  agw_d->ave_rg_ratio = agw_d->simple_rg_ratio;
  agw_d->ave_bg_ratio = agw_d->simple_bg_ratio;

  awb_util_convert_to_grid(awb, agw_d->simple_rg_ratio_x, agw_d->simple_bg_ratio_x,
    &agw_d->ave_rg_grid, &agw_d->ave_bg_grid);
  /* compute the max distance between an outlier
   * and the center of the outliers */
  for (index = 0; index < agw_d->outlier_cnt && index < 256; index++) {

    d1   = agw_p->outlier_rg_grid[index] - agw_d->ave_rg_grid;
    d2   = agw_p->outlier_bg_grid[index] - agw_d->ave_bg_grid;
    dist = d1 * d1 + d2 * d2;
    if (dist > agw_d->max_dist)
      agw_d->max_dist = dist;
  }
  if (agw_d->max_dist <= cptr->compact_cluster_r2 * 4
    && awb->current_awb_stat_config == AWB_STAT_REGULAR)
    /* is a compact cluster */
    agw_d->compact_cluster = 1;
  else /* is NOT a compact cluster */
    agw_d->compact_cluster = 0;

  awb_agw_search_nearest_reference_point (sproc, awb, agw_d->ave_rg_grid,
    agw_d->ave_bg_grid);

  /* too blue, too red,--> low confidence. reject this frame */
  if (agw_d->ave_rg_grid > (agw_p->rg_grid[AGW_AWB_HORIZON] + 3)
    || agw_d->ave_bg_grid > agw_p->bg_grid[AGW_AWB_OUTDOOR_CLOUDY]) {
    CDBG_HIGH("too red or too blue. return");
    return -1;
  }
  CDBG_AWB("### all outliers: compact_cluster %d, max_dist %d",
    agw_d->compact_cluster, agw_d->max_dist);
  CDBG_AWB("### all outliers : outliers RG grid=%d, BG grid=%d\n",
    agw_d->ave_rg_grid, agw_d->ave_bg_grid);

  if (awb_agw_single_color_tracking_all_outliers(sproc, awb,
    agw_p->outlier_rg_grid, agw_p->outlier_bg_grid, agw_d->ave_rg_grid,
    agw_d->ave_bg_grid) < 0) {
    /* 3A 1.4 change (single color tracking) */
    CDBG_HIGH("all outliers : frame rejected for being single color");
    return -1;
  }
  /* using actual daylight line for estimating D55 */
  agw_d->d55_rg = 0.7 * agw_p->rgbg_grid[agw_p->day3_rg[0]]
    + 0.3 * agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]];

  agw_d->d55_bg = 0.7 * agw_p->rgbg_grid[agw_p->day3_bg[0]]
    + 0.3 * agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]];
  CDBG_AWB("D55_rg %f, D55_bg %f", agw_d->d55_rg, agw_d->d55_bg);

  if (agw_d->compact_cluster == 1 &&
    agw_d->min_dist < cptr->compact_cluster_to_ref_point_r2 * 4) {
    /* all outliers, and it's a compact cluster although all ouliers, but
     * it's a compact cluster not too far from reference point this one is
     * design for the situation that the test chart is grey, but lighting
     * is not reference point. scene can not be too red or too blue */
    if ((agw_d->green_cnt > ((cptr->outdoor_green_threshold *
      stats->region_cnt) / 100)  &&  agw_d->indoor_green_cnt ==
      0) || agw_d->green_cnt > cptr->outdoor_green_upper_threshold) {

      if (sproc->share.prev_exp_index < agw_p->outdoor_index) {
        /* outdoor situation */
        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1; /* D50 */
          agw_d->rg_ratio = agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]];

          agw_d->bg_ratio = agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]];
          CDBG_AWB("### all outliers, compact cluster, near ref,\
                D50 (0, outdoor)\n");
        }
      } else if (sproc->share.prev_exp_index >= agw_p->outdoor_index
        && sproc->share.prev_exp_index < awb->inoutdoor_midpoint) {

        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else { /* Daylight line2 */
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->p1 = sproc->share.prev_exp_index - agw_p->outdoor_index;
          agw_d->p2 = awb->inoutdoor_midpoint - sproc->share.prev_exp_index;
          agw_d->rg_ratio = (agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]] *
            agw_d->p2 + agw_d->p1 * agw_d->d55_rg) / (agw_d->p1 + agw_d->p2); ;

          agw_d->bg_ratio = (agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]] *
            agw_d->p2 + agw_d->p1 * agw_d->d55_bg) / (agw_d->p1 + agw_d->p2);

          CDBG_AWB("### all outliers, compact cluster, near ref, daylight \
                line 2, (outdoor, inout mid)");
        }
      } else if (sproc->share.prev_exp_index >= awb->
        inoutdoor_midpoint && sproc->share.prev_exp_index < agw_p->
        indoor_index && agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_HORIZON]) {
        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT; /* D65 */
          agw_d->rg_ratio        = agw_d->d55_rg;
          agw_d->bg_ratio        = agw_d->d55_bg;
          CDBG_AWB("### all outliers, compact cluster, near ref, \
                D55 (inout mid, indoor)\n");
        }
      } else if (sproc->share.prev_exp_index >= agw_p->indoor_index
        && agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_HORIZON]) {
        /* indoor situation */
        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          /* when it's dark, and all outliers are compact cluster, average
           * is > D50. it's suspecious that it's not true outdoor grey */
          if (agw_d->ave_rg_grid >=
            agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]) {
            CDBG_HIGH("all outliers, compact cluster, near ref,  dark,"
              " sample location suspecious, return");
            return -1;
          } else {
            agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT; /* D65 */
            agw_d->rg_ratio = agw_d->d55_rg;
            agw_d->bg_ratio = agw_d->d55_bg;
            CDBG_AWB("### all outliers, compact cluster, near ref, \
                  D565 (indoor, max)\n");
          }
        }
      }
    } else { /* use simple grey world, not enough green information */
      agw_d->rg_ratio = agw_d->simple_rg_ratio;
      agw_d->bg_ratio = agw_d->simple_bg_ratio;
      CDBG_AWB("### all outliers, compact cluster, use simple grey world\n");
    }
  } /* end of all outliers & compact cluster & close to reference point */

  if (agw_d->compact_cluster == 1 &&
    agw_d->min_dist >= cptr->compact_cluster_to_ref_point_r2 * 4) {
    /* all outliers, and it's a compact cluster this case, it could be user
     * points the camera to single color object which has high chromacity */
    if ((agw_d->green_cnt > ((cptr->outdoor_green_threshold *
      stats->region_cnt) / 100) && agw_d->indoor_green_cnt ==
      0) || agw_d->green_cnt > cptr->outdoor_green_upper_threshold) {

      if (sproc->share.prev_exp_index < agw_p->outdoor_index) {
        /* outdoor situation */
        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1; /* D50 */
          agw_d->rg_ratio = agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]];

          agw_d->bg_ratio = agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]];
          CDBG_AWB("### all outliers, compact cluster, far ref, D50, \
                (0, outdoor)\n");
        }
      } else if (sproc->share.prev_exp_index >= agw_p->outdoor_index
        && sproc->share.prev_exp_index < awb->inoutdoor_midpoint) {

        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else { /* Daylight line2 */
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line         = 2;
          agw_d->p1 = sproc->share.prev_exp_index - agw_p->outdoor_index;
          agw_d->p2 = awb->inoutdoor_midpoint - sproc->share.prev_exp_index;

          agw_d->rg_ratio = (agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]] *
            agw_d->p2 + agw_d->p1 * agw_d->d55_rg) / (agw_d->p1 + agw_d->p2); ;
          agw_d->bg_ratio = (agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]] *
            agw_d->p2 + agw_d->p1 * agw_d->d55_bg) / (agw_d->p1 + agw_d->p2);
          CDBG_AWB("### all outliers, compact cluster, far ref, Daylight \
                line 2, (outdoor, inout mid)\n");
        }
      } else if (sproc->share.prev_exp_index >= awb->inoutdoor_midpoint &&
        sproc->share.prev_exp_index < agw_p->indoor_index &&
        agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_HORIZON]) {
        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
          agw_d->rg_ratio        = agw_d->d55_rg;
          agw_d->bg_ratio        = agw_d->d55_bg;

          CDBG_AWB("### all outliers, compact cluster, near ref, \
                (inout_mid, indoor),  D55\n");
        }
      } else if (sproc->share.prev_exp_index >= agw_p->indoor_index
        && agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_HORIZON]) {/* indoor */
        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
          agw_d->rg_ratio        = agw_d->d55_rg;
          agw_d->bg_ratio        = agw_d->d55_bg;
          CDBG_AWB("###  all outliers,compact cluster, near ref, \
                (indoor, max),  D55\n");
        }
      }
    } else {
      /* sometimes the green samples do not fall exactly into green zone.
       * need this for dark condition */
      if (sproc->share.prev_exp_index >= ((awb->inoutdoor_midpoint +
        agw_p->indoor_index) >> 1) && agw_d->green_cnt > 0 &&
        agw_d->indoor_green_cnt == 0 && agw_d->green_bgx > agw_p->bg_grid[
        AGW_AWB_HORIZON] && agw_d->ave_rg_grid < agw_p->
        rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]) {

        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
        agw_d->rg_ratio        = agw_d->d55_rg;
        agw_d->bg_ratio        = agw_d->d55_bg;
        CDBG_AWB("### all outliers, compact cluster, far ref,  \
              D55 (inout mid, max)\n");
      } else { /* do not make decision */
        CDBG_HIGH("all outliers, campact+high chroma. return\n");
        return -1;
      }
    }
  } /* end of all outliers & compact cluster & far away from ref point */

  if (agw_d->compact_cluster == 0) {
    /* all outliers, and also not a compact cluster */
    if (sproc->share.prev_exp_index < agw_p->outdoor_index) {
      /* outdoor situation if it's very bright. guess D50 anyway */
      if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
        return -1;
      } else {
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1;
        agw_d->rg_ratio = agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]];
        agw_d->bg_ratio = agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]];
        CDBG_AWB("### all outliers, not campact. D50 (0, outdoor)\n");
      }
    } else if (sproc->share.prev_exp_index >= agw_p->outdoor_index &&
      sproc->share.prev_exp_index < awb->inoutdoor_midpoint) {

      if ((agw_d->green_cnt > ((cptr->outdoor_green_threshold * stats->
        region_cnt) / 100) && agw_d->indoor_green_cnt == 0) ||
        agw_d->green_cnt > ((cptr->outdoor_green_upper_threshold *
        stats->region_cnt) / 100)) {

        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else { /* Daylight line2 */
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->p1 = sproc->share.prev_exp_index - awb->outdoor_midpoint;
          agw_d->p2 = awb->inoutdoor_midpoint - sproc->share.prev_exp_index;
          agw_d->rg_ratio = (agw_p->rgbg_grid[agw_p->day3_rg[agw_p->n_day3 - 1]] *
            agw_d->p2 + agw_d->p1 * agw_d->d55_rg) / (agw_d->p1 + agw_d->p2);
          agw_d->bg_ratio = (agw_p->rgbg_grid[agw_p->day3_bg[agw_p->n_day3 - 1]] *
            agw_d->p2 + agw_d->p1 * agw_d->d55_bg) / (agw_d->p1 + agw_d->p2);
          CDBG_AWB("### all outliers, not campact. Daylight line 2, \
                (outdoor, inout mid)\n");
        }
      } else {
        CDBG_HIGH("all outliers, not campact. not enough green. return\n");
        return -1;
      }
    } else if (sproc->share.prev_exp_index >= awb->inoutdoor_midpoint &&
      sproc->share.prev_exp_index < agw_p->indoor_index && agw_d->green_bgx >
      agw_p->bg_grid[AGW_AWB_HORIZON]) {

      if ((agw_d->green_cnt > ((cptr->outdoor_green_threshold *
        stats->region_cnt) / 100) && agw_d->indoor_green_cnt ==
        0) || agw_d->green_cnt > ((cptr->outdoor_green_upper_threshold *
        stats->region_cnt) / 100)) {

        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
          agw_d->rg_ratio        = agw_d->d55_rg;
          agw_d->bg_ratio        = agw_d->d55_bg;
          CDBG_AWB("### all outliers, compact cluster, near ref, \
                (inout mid, indoor), D55\n");
        }
      } else {
        CDBG_HIGH("all outliers, not campact. not enough green. return\n");
        return -1;
      }
    } else if (sproc->share.prev_exp_index >= agw_p->indoor_index &&
      agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_HORIZON]) {
      if ((agw_d->green_cnt > ((cptr->outdoor_green_threshold *
        stats->region_cnt)/100) && agw_d->indoor_green_cnt ==
        0) || agw_d->green_cnt > ((cptr->outdoor_green_upper_threshold
        * stats->region_cnt) / 100)) {

        if (awb->search_mode == AWB_AGW_INDOOR_ONLY) {
          return -1;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
          agw_d->rg_ratio        = agw_d->d55_rg;
          agw_d->bg_ratio        = agw_d->d55_bg;
          CDBG_AWB("### all outliers, compact cluster, near ref,"
            " (indoor, max),  D55");
        }
      } else {
        CDBG_HIGH("all outliers, not campact. not enough green. return\n");
        return -1;
      }
    } else {
      CDBG_HIGH("all outliers, not campact. should not reach here. return");
      return -1;
    }
  } /* end of the case of all outliers and not compact cluster */

  CDBG_AWB("### AWB all outliers, estimated rg_ratio %f, bg_ratio %f",
    agw_d->rg_ratio, agw_d->bg_ratio);

  /* check the history to see if there is any better
   * (more reliable decision) with valid near grey samples */
  if (awb_agw_is_decision_outdoor(agw_d->sample_decision) == 0) {
    /* all outlier with history tracking */
    CDBG_AWB("all outliers: change rg %f bg %f by history daylight cnt %d",
      agw_d->rg_ratio, agw_d->bg_ratio, agw_d->p1);
    CDBG_AWB("all outliers: update hist decision %d", agw_d->sample_decision);
    awb_agw_history_update(sproc, awb);
    awb_agw_adjust_rg_bg_by_history(sproc, awb);
  }
  if (agw_d->rg_ratio == 0.0)
    agw_d->rg_ratio   = 1.0;
  if (agw_d->bg_ratio == 0.0)
    agw_d->bg_ratio   = 1.0;

  agw_d->g_gain = 1.0;
  agw_d->r_gain = 1.0 / agw_d->rg_ratio;
  agw_d->b_gain = 1.0 / agw_d->bg_ratio;

  CDBG_AWB("%s: AWB all_outliers 1 r_gain %f, g_gain %f, b_gain %f",
    __func__, agw_d->r_gain, agw_d->g_gain, agw_d->b_gain);

  agw_d->all_outliers       = 1;  /* to skip the following if case */
  awb->regular_decision     = agw_d->sample_decision;
  awb->regular_ave_rg_ratio = agw_d->rg_ratio;
  awb->regular_ave_bg_ratio = agw_d->bg_ratio;
  return 0;
} /* awb_agw_outlier_hueristics */

/*===========================================================================
 * FUNCTION    - awb_agw_FL_type_heuristics -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_FL_type_heuristics(stats_proc_t *sproc, awb_t *awb)
{
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  awb_input_stats_type *stats  = &(awb->stats_ptr);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  /* this case occurs for green plants with wood chips
 * or red bush, green leaves mixed with pink/red flowers, etc. */
  if (sproc->share.prev_exp_index < agw_p->outdoor_index) {
    if (agw_d->compact_cluster == 0 && agw_d->decision_changed == 0) {
      /* make sure it's not MCC in strong light */
      if (awb->search_mode==AWB_AGW_OUTDOOR_ONLY ||
        awb->search_mode==AWB_AGW_INOUTDOOR_BOTH) {

        CDBG_AWB("valid, not campact. decision=%d ->D50 (0,outdoor)\n",
          awb->regular_decision);
        agw_d->decision_changed = TRUE;
        agw_d->sample_decision  = AGW_AWB_OUTDOOR_SUNLIGHT1; /* D50 */

        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];

        awb_agw_set_decision(awb);
      } else {
        CDBG_AWB("exp<outdoor, is a compact cluster %d, not changed",
          agw_d->max_dist);
      }
    } else {
      CDBG_HIGH("exp < outdoor, no compact cluster, low conf, reject");
      return -1;
    }
  } else if (sproc->share.prev_exp_index < awb->inoutdoor_midpoint) {
    /* exp_index between (outdoor, inoutdoor_midpoint) do
     * interpolation to avoid sudden AWB jump across the boundary */
    if (!agw_d->compact_cluster && agw_d->max_dist < cptr->r2_threshold * 4
      && agw_d->green_cnt > 0 && agw_d->green_bgx >
      agw_p->bg_grid[AGW_AWB_HORIZON]) {
      /* make sure it's not MCC in strong light */
      if (awb->search_mode == AWB_AGW_OUTDOOR_ONLY ||
        awb->search_mode == AWB_AGW_INOUTDOOR_BOTH) {

        agw_d->decision_changed = TRUE;
        if (awb_agw_history_daylight_srch(awb) == 0) {
          /* use past daylight history average, 3A 1.4 change */
          awb_agw_history_get_daylight(awb);
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->day_idx  = 0;
        } else { /* Daylight line 2 */
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->p1 = sproc->share.prev_exp_index - agw_p->outdoor_index;
          agw_d->p2 = awb->inoutdoor_midpoint - sproc->share.prev_exp_index;

          agw_d->rg_ratio = (agw_p->red_gain_table[
            AGW_AWB_OUTDOOR_SUNLIGHT] * agw_d->p2 + agw_d->p1 * agw_p->
            red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1]) /
            (agw_d->p1 + agw_d->p2);
          agw_d->bg_ratio = (agw_p->blue_gain_table[
            AGW_AWB_OUTDOOR_SUNLIGHT] * agw_d->p2 + agw_d->p1 * agw_p->
            blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1]) /
            (agw_d->p1 + agw_d->p2);
          CDBG_AWB("### valid, not campact, decision=%d ->daylight \
                    line 2 (outdoor, inout mid), p1 %d, p2 %d",
            awb->regular_decision, agw_d->p1, agw_d->p2);
        }
        awb_agw_set_decision(awb);
      }
    } else {
      CDBG_AWB("### outdoor<exp<inout_mid, max_dist %d, not changed",
        agw_d->max_dist);
    }
  } else if (sproc->share.prev_exp_index <= agw_p->indoor_index) {
    /* some outdoor scene has only distribution in F cluster only.
     * it could be shady or directly illuminated by sun */
    if (agw_d->green_cnt > ((cptr->outdoor_green_threshold_bright_F *
      stats->region_cnt) / 100) && !agw_d->compact_cluster &&
      agw_d->max_dist < cptr->r2_threshold * 4 && !agw_d->indoor_green_cnt &&
      agw_d->green_bgx > ((agw_p->bg_grid[AGW_AWB_HORIZON] +
      agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT]) >> 1)) {

      CDBG_AWB("### valid, not campact. decision=%d ->outdoor \
              (iout mid,indoor)", awb->regular_decision);
      agw_d->decision_changed = TRUE;
      if (awb_agw_history_daylight_srch(awb) == 0) {
        awb_agw_history_get_daylight(awb);
        agw_d->sample_decision = AGW_AWB_HYBRID;
        agw_d->day_line = 2;
        agw_d->day_idx = 0;
      } else {
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;  /* D65 */
        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
      }
      awb_agw_set_decision(awb);
    }
    if (awb->current_awb_stat_config == AWB_STAT_REGULAR
      && !agw_d->decision_changed) {
      if (awb_agw_is_decision_FL(awb->white_decision) == 0) {
        awb_agw_reg_to_white_dec_change(sproc, awb);
        CDBG_AWB("(inout mid, indoor)");
      }
    }
    if (agw_d->green_cnt > 0 && !agw_d->compact_cluster &&
      !agw_d->decision_changed && (awb_agw_history_daylight_srch(awb)
      == 0) && agw_d->green_bgx > ((agw_p->bg_grid[AGW_AWB_HORIZON] +
      agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT]) >> 1)) {
      /* 3A 1.4 change (daylight-type fluroscent handling */
      if (awb_agw_is_decision_outdoor(awb->white_decision) == 0) {
        CDBG_AWB("### valid, not campact. decision=%d ->outdoor \
                (inoutmid, indoor)\n", awb->regular_decision);
        agw_d->decision_changed = TRUE;
        agw_d->sample_decision  = awb->white_decision;
        awb_agw_history_get_daylight(awb);
        awb_agw_set_decision(awb);
      }
    }
  } else {
    /* exp_index in (indoor, max)
     * less bright, needs more green samples to reject this frame */
    if (agw_d->green_cnt > ((cptr->outdoor_green_threshold_dark_F *
        stats->region_cnt) / 100) && !agw_d->compact_cluster &&
        agw_d->max_dist < cptr->r2_threshold * 4 && !agw_d->indoor_green_cnt &&
        agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT]) {
      /* (daylight-type F handling), in some country, the custom flo
       * is very close to daylight, so the indoor green objects
       * are mistaken as outdoor green. Try to prevent this. */
      if (awb_agw_is_custom_flo_close_to_daylight(awb) == 0 &&
          (awb_agw_aec_history_find_min_exp(awb) >=
          (int)agw_p->indoor_index ||
          awb_agw_history_daylight_srch(awb) == 0)) {
        /* maintain indoor decision, no change,
         * daylight-type F handling */
        CDBG_AWB("custom flo close to daylight (indoor,max), do not \
        change decision\n");
      } else {
        CDBG_AWB("### valid, not campact. decision=%d ->D65 (indoor,max)\
            green bgx=%d, horizon bg=%d\n", awb->regular_decision,
            agw_d->green_bgx, agw_p->bg_grid[AGW_AWB_HORIZON]);
        agw_d->decision_changed = TRUE;
        if (awb_agw_history_daylight_srch(awb) == 0) {
          /* use past daylight history avg,daylight-type F hanlding */
          awb_agw_history_get_daylight(awb);
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->day_idx = 0;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
          agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
          agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
        }
        awb_agw_set_decision(awb);
      }
    }
    if (agw_d->green_cnt > ((cptr->outdoor_green_upper_threshold *
        stats->region_cnt) / 100) && !agw_d->decision_changed &&
        !agw_d->indoor_green_cnt && agw_d->green_bgx >
    agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT]) {

      if (awb_agw_is_custom_flo_close_to_daylight(awb) == 0 &&
          (awb_agw_aec_history_find_min_exp(awb) >=
          (int)agw_p->indoor_index ||
          awb_agw_history_daylight_srch(awb) == 0)) {
        /* 3A 1.4 (daylight-type F handling) */
        CDBG_AWB("custom flo close to daylight (indoor,max), do \
        not change decision\n");
      } else {
        CDBG_AWB("### green cnt > threshold,  decision=%d ->D65 \
            (indoor,max)\n", awb->regular_decision);
        agw_d->decision_changed = TRUE;
        if (awb_agw_history_daylight_srch(awb) == 0) {
          /* use past daylight history average */
          awb_agw_history_get_daylight(awb);
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->day_idx = 0;
        } else {
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT; /* D65 */
          agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
          agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
        }
        awb_agw_set_decision(awb);
      }
    }
    if (awb->current_awb_stat_config == AWB_STAT_REGULAR
        && !agw_d->decision_changed) {
      if (awb->white_decision ==
          AGW_AWB_INDOOR_INCANDESCENT && awb->
          white_has_single_peak == TRUE && awb->white_ave_rg_ratio
          <= agw_p->red_gain_table[AGW_AWB_INDOOR_INCANDESCENT] &&
          awb->white_ave_bg_ratio >=
          agw_p->blue_gain_table[AGW_AWB_INDOOR_INCANDESCENT]
          && agw_d->ave_rg_ratio * 1.3 > awb->white_ave_rg_ratio
          && agw_d->ave_bg_ratio < 1.3 * awb->white_ave_bg_ratio) {
        /* (daylight-type F handling) */
        awb_agw_reg_to_white_dec_change(sproc, awb);
        CDBG_AWB("(intdoor, max)");
      }
      if (awb_agw_is_decision_FL(awb->white_decision) == 0) {
        awb_agw_reg_to_white_dec_change(sproc, awb);
        CDBG_AWB("(intdoor, max)");
      }
    }
    if (agw_d->green_cnt > 0 && !agw_d->compact_cluster &&
        !agw_d->decision_changed && agw_d->green_bgx >
        agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT] &&
        awb_agw_history_daylight_srch(awb) == 0) {
      /* 3A 1.4 change (daylight-type F handling) */
      if ((awb->white_decision == AGW_AWB_HYBRID ||
          awb->white_decision == AGW_AWB_OUTDOOR_SUNLIGHT1 ||
          awb->white_decision == AGW_AWB_OUTDOOR_NOON) &&
          awb->current_awb_stat_config == AWB_STAT_REGULAR && awb->
          regular_decision == AGW_AWB_INDOOR_CUSTOM_FLO) {

        if ((agw_d->day_cluster < ((15 * agw_d->smpl_cnt) / 100) ||
            agw_d->f_cluster > ((80 * agw_d->smpl_cnt) / 100)) &&
            agw_d->green_cnt < ((30 * stats->region_cnt) / 100))
          CDBG_AWB("Keep F decision for LGE indoor office condition\n");
        else {
          CDBG_AWB("valid, not campact. dec %d change to %d (indoor,max)",
              awb->regular_decision,
              awb->white_decision);
          agw_d->decision_changed = TRUE;
          agw_d->sample_decision = awb->white_decision;
          awb_agw_history_get_daylight(awb);

          if (awb->current_awb_stat_config == AWB_STAT_REGULAR)
            awb->regular_decision = agw_d->sample_decision;
        }
      } else {
        if (awb->white_decision != AGW_AWB_INVALID_LIGHT) {
          CDBG_AWB("### valid, not campact. decision=%d ->%d \
              (indoor,max)\n", awb->regular_decision,
              awb->white_decision);
          agw_d->decision_changed = TRUE;
          agw_d->sample_decision = awb->white_decision;
          awb_agw_history_get_daylight(awb);
        }
        awb_agw_set_decision(awb);
      }
    }
  }
  /* for the case that current decision is A or H */

  return 0;
} /* awb_agw_FL_type_heuristics */

/*===========================================================================
 * FUNCTION    - awb_agw_INC_type_heuristics -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_INC_type_heuristics(stats_proc_t *sproc, awb_t *awb)
{
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  awb_input_stats_type *stats  = &(awb->stats_ptr);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  if (awb->current_awb_stat_config == AWB_STAT_WHITE)
    if ((awb->regular_decision ==
      AGW_AWB_OUTDOOR_SUNLIGHT1 || awb->
      regular_decision == AGW_AWB_OUTDOOR_SUNLIGHT ||
      awb->regular_decision == AGW_AWB_OUTDOOR_CLOUDY ||
      awb->regular_decision == AGW_AWB_HYBRID ) &&
      awb_agw_history_daylight_srch(awb) == 0) {
      /* current decision is white decision and it's A or H, but the
       * grey world decision is daylight. It's very likely to be mixed
       * lighting condition reject A/H to avoid bluish appearance */
      CDBG_HIGH("%s: white decision %d, grey decision %d, reject",
        __func__, awb->white_decision, awb->regular_decision);
      awb->white_decision = AGW_AWB_INVALID_LIGHT;
      return -1;
    }
    /* this case occurs for green plants with wood chips
     * or red bush, green leaves mixed with pink/red flowers, etc. */
  if (sproc->share.prev_exp_index < agw_p->outdoor_index) {

    if (awb->search_mode == AWB_AGW_OUTDOOR_ONLY ||
      awb->search_mode == AWB_AGW_INOUTDOOR_BOTH) {

      if (agw_d->compact_cluster == 0 && agw_d->decision_changed == 0) {
        /* make sure it's not MCC in strong light */
        agw_d->decision_changed = TRUE;
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1; /* D50 */
        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
        CDBG_AWB("### valid, not campact, decision=%d -> D50, (0, \
                  outdooor)\n", awb->regular_decision);
        awb_agw_set_decision(awb);
      } else {
        if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
          CDBG_HIGH("%s: not campact, decision=%d (0, outdooor), low "
            "confidence, reject", __func__, awb->regular_decision);
          return -1;
        }
      }
    }
  } else if (sproc->share.prev_exp_index < awb->inoutdoor_midpoint) {

    if (agw_d->compact_cluster == 0 && agw_d->decision_changed == 0 &&
      agw_d->max_dist < cptr->r2_threshold * 4 && agw_d->green_cnt > 0) {
      /* make sure it's not MCC in strong light */
      if (awb->search_mode == AWB_AGW_OUTDOOR_ONLY ||
        awb->search_mode == AWB_AGW_INOUTDOOR_BOTH) {

        if (awb_agw_history_daylight_srch(awb) == 0) {
          /* use past daylight history avg (daylighttype F handling) */
          CDBG_AWB("### valid, not campact, decision=%d -> hybrid \
                    with history(out, inout mid)\n", agw_d->sample_decision);
          awb_agw_history_get_daylight(awb);
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->day_idx  = 0;
        } else { /*Daylight line 2 */
          agw_d->sample_decision = AGW_AWB_HYBRID;
          agw_d->day_line = 2;
          agw_d->p1 = sproc->share.prev_exp_index - agw_p->outdoor_index;
          agw_d->p2 = awb->inoutdoor_midpoint - sproc->share.prev_exp_index;

          agw_d->rg_ratio = (agw_p->red_gain_table[
            AGW_AWB_OUTDOOR_SUNLIGHT] * agw_d->p2 + agw_d->p1 *
            agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1]) /
            (agw_d->p1 + agw_d->p2);
          agw_d->bg_ratio = (agw_p->blue_gain_table[
            AGW_AWB_OUTDOOR_SUNLIGHT] * agw_d->p2 + agw_d->p1 *
            agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1]) /
            (agw_d->p1 + agw_d->p2);
          CDBG_AWB("### valid, not campact, decision=%d -> daylight \
                    line 2 (out, inout mid), p1 %d, p2 %d",
            awb->regular_decision, agw_d->p1, agw_d->p2);
        }

        agw_d->decision_changed = TRUE;
        awb_agw_set_decision(awb);
      }
    } else {
      if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
        CDBG_HIGH("%s: valid, not campact, decision %d, low confidence,"
          " reject (out, inout mid)", __func__, agw_d->sample_decision);
        return -1;
      }
    }
  } else if (sproc->share.prev_exp_index < agw_p->indoor_index) {
    /* some outdoor scene has only distribution in A cluster only.
     * it could be shady or directly illuminated by sun */
    if (agw_d->green_cnt > ((cptr->outdoor_green_threshold_bright_A *
      stats->region_cnt) / 100) && agw_d->compact_cluster == 0 &&
      agw_d->max_dist < cptr->r2_threshold * 4 && agw_d->decision_changed == 0
      && agw_d->indoor_green_cnt == 0 && agw_d->green_bgx >
      agw_p->bg_grid[AGW_AWB_HORIZON]) {

      if (awb_agw_history_daylight_srch(awb) == 0) {
        /* use past daylight history avg (daylight-type F handling) */
        CDBG_AWB("### valid, not campact, decision=%d -> hybrid with \
                  history(inout mid, in)\n", agw_d->sample_decision);
        awb_agw_history_get_daylight(awb);
        agw_d->sample_decision = AGW_AWB_HYBRID;
        agw_d->day_line = 2;
        agw_d->day_idx = 0;
      } else {

        CDBG_AWB("### valid, not campact. decision=%d ->D65 \
                  (inout_mid,indoor)\n", awb->regular_decision);
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
      }
      agw_d->decision_changed = TRUE;
      awb_agw_set_decision(awb);
    }
    if (awb->current_awb_stat_config == AWB_STAT_REGULAR &&
      !agw_d->decision_changed) {
      if ((awb->white_decision == AGW_AWB_INDOOR_INCANDESCENT ||
        awb->white_decision == AGW_AWB_HORIZON) &&
        awb->white_has_single_peak == TRUE) {
        awb_agw_reg_to_white_dec_change(sproc, awb);
        CDBG_AWB("(inout mid, indoor)");
      }

      if (awb_agw_is_decision_FL(awb->white_decision) == 0) {
        awb_agw_reg_to_white_dec_change(sproc, awb);
        CDBG_AWB("(inout mid, indoor)");
      }
      if (agw_d->green_cnt > 0 && agw_d->compact_cluster == 0 &&
        agw_d->decision_changed == 0 && agw_d->green_bgx > agw_p->
        bg_grid[AGW_AWB_HORIZON]) {

        if (awb_agw_is_decision_outdoor(awb->white_decision) == 0) {
          /* (daylight-type F handling) */
          if (awb_agw_history_daylight_srch(awb) == 0) {
            /* use past daylight history average */
            CDBG_AWB("### valid, not campact, decision=%d -> hybrid \
                      with history(intout mid, in)\n", agw_d->sample_decision);
            awb_agw_history_get_daylight(awb);
            agw_d->sample_decision = AGW_AWB_HYBRID;
            agw_d->day_line = 2;
            agw_d->day_idx = 0;
          } else {
            CDBG_AWB("valid, not campact. decision=%d ->D65 (indout \
                      mid,indoor)\n", awb->regular_decision);
            agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
            agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
            agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
          }
          agw_d->decision_changed = TRUE;
          awb_agw_set_decision(awb);
        }
      }
    } /* end of regular_stat_on == TRUE && !agw_d->decision_changed) */
  } else {
    /* less bright, needs more green samples to reject this frame */
    if (agw_d->green_cnt > ((cptr->outdoor_green_threshold_dark_A *
      stats->region_cnt) / 100) && agw_d->compact_cluster == 0 &&
      agw_d->max_dist < cptr->r2_threshold * 4 && agw_d->decision_changed == 0 &&
      agw_d->indoor_green_cnt == 0 && agw_d->green_bgx >
      agw_p->bg_grid[AGW_AWB_HORIZON]) {
      /* 3A 1.4 change (daylight-type F handling) */
      if (awb_agw_history_daylight_srch(awb) == 0) {
        /* use past daylight history average */
        CDBG_AWB("### valid, not campact, decision=%d -> hybrid with \
                  history(in, max)", agw_d->sample_decision);
        awb_agw_history_get_daylight(awb);
        agw_d->sample_decision = AGW_AWB_HYBRID;
        agw_d->day_line = 2;
        agw_d->day_idx = 0;
      } else {
        CDBG_AWB("valid, not campact. decision=%d ->D65 (indoor,max)\n",
          awb->regular_decision);

        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
      }
      agw_d->decision_changed = TRUE;
      awb_agw_set_decision(awb);
    }

    if (agw_d->green_cnt > ((cptr->outdoor_green_upper_threshold *
      stats->region_cnt) / 100) && !agw_d->decision_changed &&
      agw_d->indoor_green_cnt == 0 &&
      agw_d->green_bgx > agw_p->bg_grid[AGW_AWB_HORIZON]) {

      if (awb_agw_history_daylight_srch(awb) == 0) {
        /* use past daylight history avg (daylight-type F handling) */
        CDBG_AWB("valid, not campact, dec %d -> hybrid with history(in, max)",
          agw_d->sample_decision);
        awb_agw_history_get_daylight(awb);
        agw_d->sample_decision = AGW_AWB_HYBRID;
        agw_d->day_line = 2;
        agw_d->day_idx = 0;
      } else {
        CDBG_AWB("### valid, not campact. decision=%d ->D65 \
                (indoor,max)\n", awb->regular_decision);
        agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
      }
      agw_d->decision_changed = TRUE;
      awb_agw_set_decision(awb);
    }

    if (awb->current_awb_stat_config == AWB_STAT_REGULAR
      && !agw_d->decision_changed) {
      if ((awb->white_decision == AGW_AWB_INDOOR_INCANDESCENT ||
        awb->white_decision == AGW_AWB_HORIZON) &&
        awb->white_has_single_peak == TRUE) {
        awb_agw_reg_to_white_dec_change(sproc, awb);
        CDBG_AWB("(indoor, max)");
      }
    }
    if (awb_agw_is_decision_FL(awb->white_decision) == 0) {
      awb_agw_reg_to_white_dec_change(sproc, awb);
      CDBG_AWB("(indoor, max)");
    }
    if (agw_d->green_cnt > 0 && agw_d->compact_cluster == 0 &&
      !agw_d->decision_changed && agw_d->green_bgx > agw_p->
      bg_grid[AGW_AWB_HORIZON] &&
      awb_agw_history_daylight_srch(awb) == 0) {
      if (awb_agw_is_decision_outdoor(awb->white_decision) == 0) {
        CDBG_AWB("valid, not campact, dec %d -> hybrid with history(in, max)",
          agw_d->sample_decision);
        awb_agw_history_get_daylight(awb);
        agw_d->sample_decision  = AGW_AWB_HYBRID;
        agw_d->day_line         = 2;
        agw_d->day_idx          = 0;
        agw_d->decision_changed = TRUE;
        awb_agw_set_decision(awb);
      }
    }
  }  /* end of if stat_on is TRUE && !agw_d->decision_changed) */
  return 0;
} /* awb_agw_INC_type_heuristics */

/*===========================================================================
 * FUNCTION    - awb_agw_daylight_heuristics -
 *
 * DESCRIPTION: Likely indoor brightness. Heuristics is added to solve the
 *              problem of indoor with large blue object. Reg Dec is outdoor
 *              but blue is less likely to be picked up by white Dec.
 *              White Dec is used to convert it back to indoor light.
 *==========================================================================*/
static void awb_agw_daylight_heuristics(stats_proc_t *sproc, awb_t *awb)
{
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  if (sproc->share.prev_exp_index >= ((awb->inoutdoor_midpoint +
    agw_p->indoor_index) >> 1) &&
    awb->current_awb_stat_config == AWB_STAT_REGULAR &&
    !agw_d->green_cnt &&
    !agw_d->compact_cluster &&
    !agw_d->indoor_green_cnt &&
    !agw_d->decision_changed &&
    awb_agw_is_decision_FL(awb->white_decision) == 0 &&
    !awb_agw_history_is_daylight_dominant(agw_p)) {

    awb_agw_reg_to_white_dec_change(sproc, awb);
  }
  CDBG_AWB(" %s: ((inout_mid_point + indoor_index)/2, max)", __func__);
} /* awb_agw_daylight_heuristics */

/*===========================================================================
 * FUNCTION    - awb_agw_valid_sample_hueristics -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_agw_valid_sample_hueristics(stats_proc_t *sproc, awb_t *awb)
{
  int   index, d1, d2, dist;
  float w1, w2;

  awb_input_stats_type *stats  = &(awb->stats_ptr);
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  /* compute the grid coordinate of the average R/G and B/G ratio
   * find the R/G grid index */
  awb_util_convert_to_grid(awb, agw_d->ave_rg_ratio_x, agw_d->ave_bg_ratio_x,
    &agw_d->ave_rg_grid, &agw_d->ave_bg_grid);

  CDBG_AWB("### Before adjustment. ave_rg_ratio %f,  ave_bg_ratio %f",
    agw_d->ave_rg_ratio, agw_d->ave_bg_ratio);
  /* compute the max distance between sample center and samples */
  agw_d->max_dist = 0;
  for (index = 0; index < agw_d->smpl_cnt && index < 256; index++) {
    d1 = agw_p->sample_rg_grid[index] - agw_d->ave_rg_grid;
    d2 = agw_p->sample_bg_grid[index] - agw_d->ave_bg_grid;

    dist = d1 * d1 + d2 * d2;
    if (dist > agw_d->max_dist)
      agw_d->max_dist = dist;
  }

  for (index = 0; index < agw_d->outlier_cnt && index < 256; index++) {
    d1   = agw_p->outlier_rg_grid[index] - agw_d->ave_rg_grid;
    d2   = agw_p->outlier_bg_grid[index] - agw_d->ave_bg_grid;
    dist = d1*d1+d2*d2;

    if (dist > agw_d->max_dist)
      agw_d->max_dist = dist;
  }
  if (agw_d->max_dist <= cptr->compact_cluster_r2 * 4 &&
    awb->current_awb_stat_config == AWB_STAT_REGULAR)
    agw_d->compact_cluster = 1;
  else
    agw_d->compact_cluster = 0;

  CDBG_AWB("sample distribution r2 %d", agw_d->max_dist);

  /* figure out which reference point is the average R/G, B/G closest to */
  agw_d->min_dist = 0x3fffffff;

  /* Search all lighting conditions */
  for (index = 0; index < AWB_NUMBER_OF_REFERENCE_POINT;
    index++) { /* fixed reference points */
    d1   = (agw_d->ave_rg_grid - agw_p->rg_grid[index]);
    d2   = (agw_d->ave_bg_grid - agw_p->bg_grid[index]);

    dist = d1 * d1 + d2 * d2;
    if (dist < agw_d->min_dist) {
      agw_d->sample_decision = (chromatix_awb_light_type)index;
      agw_d->min_dist        = dist;

      if (agw_d->sample_decision == AGW_AWB_INDOOR_WARM_FLO ||
        agw_d->sample_decision == AGW_AWB_INDOOR_COLD_FLO ||
        (agw_d->sample_decision == AGW_AWB_INDOOR_CUSTOM_FLO &&
        (awb->search_mode == AWB_AGW_INDOOR_ONLY ||
        awb->search_mode == AWB_AGW_INOUTDOOR_BOTH))) {

        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else {
          agw_d->rg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
            agw_p->red_gain_table[agw_d->sample_decision] +
            (cptr->sample_influence.indoor_influence) * agw_d->ave_rg_ratio);
          agw_d->bg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
            agw_p->blue_gain_table[agw_d->sample_decision] +
            (cptr->sample_influence.indoor_influence) * agw_d->ave_bg_ratio);
        }
        CDBG_AWB("F light. rg_ratio %f  bg_ratio %f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);

        if (agw_d->sample_decision == AGW_AWB_INDOOR_COLD_FLO)
          agw_d->f_idx = 0;

        if (agw_d->sample_decision == AGW_AWB_INDOOR_WARM_FLO)
          agw_d->f_idx = agw_p->n_fline - 1;


      } else if (agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT &&
        (awb->search_mode == AWB_AGW_INDOOR_ONLY ||
        awb->search_mode == AWB_AGW_INOUTDOOR_BOTH)) {

        /* pull to NW corner to have warm appearance */
        if (agw_d->compact_cluster && agw_d->min_dist <= cptr->
          compact_cluster_to_ref_point_r2 * 4) { /* compact cluster */
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else { /* not a compact cluster, make it appear warm */
          agw_d->rg_ratio = agw_d->ave_rg_ratio;
          agw_d->bg_ratio = agw_d->ave_bg_ratio;
        }
        CDBG_AWB("A light. rg_ratio=%f  bg_ratio=%f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);
        agw_d->a_idx = 0;
        agw_d->a_line = 2;


      } else if (agw_d->sample_decision == AGW_AWB_HORIZON &&
        (awb->search_mode == AWB_AGW_INDOOR_ONLY ||
        awb->search_mode == AWB_AGW_INOUTDOOR_BOTH)) {

        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          /* likely to be grey chart test. must show grey */
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else { /* likely not grey chart, pull to incandescent to
                  * show warm appearance */
          agw_d->rg_ratio = agw_d->ave_rg_ratio;
          agw_d->bg_ratio = agw_d->ave_bg_ratio;
        }
        CDBG_AWB("Horizon. rg_ratio=%f  bg_ratio=%f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);
        agw_d->a_idx = agw_p->n_aline2 - 1;
        agw_d->a_line = 2;


      } else { /* outdoor */
        if (awb->search_mode == AWB_AGW_OUTDOOR_ONLY ||
          awb->search_mode == AWB_AGW_INOUTDOOR_BOTH) {

          if (agw_d->compact_cluster && agw_d->min_dist <= cptr->
            compact_cluster_to_ref_point_r2 * 4) {
            /* likely to be grey chart test. must show grey */
            agw_d->rg_ratio = agw_d->simple_rg_ratio;
            agw_d->bg_ratio = agw_d->simple_bg_ratio;
          } else {
            /* try to move the wb point to the south east corner of the
             * sample average and nearest reference point. Hopefully the
             * result is less red and more blue for outdoor condition */
            agw_d->rg_ratio = STATS_PROC_MAX(agw_d->ave_rg_ratio, agw_p->
              red_gain_table[agw_d->sample_decision]);
            agw_d->bg_ratio = STATS_PROC_MIN(agw_d->ave_bg_ratio, agw_p->
              blue_gain_table[agw_d->sample_decision]);
          }
          CDBG_AWB("day light. rg_ratio=%f  bg_ratio=%f\n",
            agw_d->rg_ratio, agw_d->bg_ratio);
        }
      }
    } else if (dist == agw_d->min_dist) {
      if (awb_agw_is_decision_FL(index) == 0) {
        if (index == (int)AGW_AWB_INDOOR_CUSTOM_FLO) {
          if (agw_d->sample_decision == AGW_AWB_INDOOR_WARM_FLO ||
            agw_d->sample_decision == AGW_AWB_INDOOR_COLD_FLO) {
            /* custom fluorescent is preferred over CW and TL */
            agw_d->sample_decision = (chromatix_awb_light_type)index;
            if (agw_d->compact_cluster && agw_d->min_dist <=
              cptr->compact_cluster_to_ref_point_r2 * 4) {
              /* likely to be grey chart test. must show grey */
              agw_d->rg_ratio = agw_d->simple_rg_ratio;
              agw_d->bg_ratio = agw_d->simple_bg_ratio;
            } else {
              agw_d->rg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
                agw_p->red_gain_table[agw_d->sample_decision] +
                (cptr->sample_influence.indoor_influence) * agw_d->ave_rg_ratio);
              agw_d->bg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
                agw_p->blue_gain_table[agw_d->sample_decision] +
                (cptr->sample_influence.indoor_influence) * agw_d->ave_bg_ratio);
            }
            CDBG_AWB("### Custom F light. rg_ratio=%f  bg_ratio=%f\n",
              agw_d->rg_ratio, agw_d->bg_ratio);
          }
        }
        if (agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT ||
          agw_d->sample_decision == AGW_AWB_HORIZON) {
          /* fluorescent is preferred over A and H */
          agw_d->sample_decision = (chromatix_awb_light_type)index;
          if (agw_d->compact_cluster && agw_d->min_dist <=
            cptr->compact_cluster_to_ref_point_r2 * 4) {
            /* likely to be grey chart test. must show grey */
            agw_d->rg_ratio = agw_d->simple_rg_ratio;
            agw_d->bg_ratio = agw_d->simple_bg_ratio;
          } else {
            agw_d->rg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
              agw_p->red_gain_table[agw_d->sample_decision] +
              (cptr->sample_influence.indoor_influence) * agw_d->ave_rg_ratio);

            agw_d->bg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
              agw_p->blue_gain_table[agw_d->sample_decision] +
              (cptr->sample_influence.indoor_influence) * agw_d->ave_bg_ratio);
          }
          CDBG_AWB("F light.rg_ratio=%f bg_ratio=%f\n", agw_d->rg_ratio, agw_d->bg_ratio);
        }
      }
      if (index == (int)AGW_AWB_OUTDOOR_SUNLIGHT1 &&
        sproc->share.prev_exp_index >= awb->inoutdoor_midpoint) {
        if (awb_agw_is_decision_FL(agw_d->sample_decision) == 0) {

          agw_d->sample_decision = (chromatix_awb_light_type)index;
          if (agw_d->compact_cluster && agw_d->min_dist <=
            cptr->compact_cluster_to_ref_point_r2 * 4) {
            /* likely to be grey chart test. must show grey */
            agw_d->rg_ratio = agw_d->simple_rg_ratio;
            agw_d->bg_ratio = agw_d->simple_bg_ratio;
          } else {
            /* try to move the wb point to the south east corner of the
             * sample average and nearest reference point. Hopefully the
             * result is less red and more blue */
            agw_d->rg_ratio = STATS_PROC_MAX(agw_d->ave_rg_ratio,
              agw_p->red_gain_table[agw_d->sample_decision]);
            agw_d->bg_ratio = STATS_PROC_MIN(agw_d->ave_bg_ratio,
              agw_p->blue_gain_table[agw_d->sample_decision]);
          }
          CDBG_AWB("D50. rg_ratio=%f  bg_ratio=%f\n", agw_d->rg_ratio, agw_d->bg_ratio);
        }
      }
    }
  } /* end for */

  if (awb->search_mode == AWB_AGW_OUTDOOR_ONLY ||
    awb->search_mode == AWB_AGW_INOUTDOOR_BOTH) {

    for (index = 1; index < agw_p->n_day1 - 1 &&
      index < AGW_NUMBER_GRID_POINT; index++) {
      /* examine the daylight line 1 */
      d1 = (agw_d->ave_rg_grid - agw_p->day_line_1[index][0]);
      d2 = (agw_d->ave_bg_grid- agw_p->day_line_1[index][1]);
      dist = d1 * d1 + d2 * d2;

      if (dist < agw_d->min_dist) {
        agw_d->min_dist = dist;
        agw_d->sample_decision = AGW_AWB_HYBRID;
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          /* likely to be grey chart test. must show grey */
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else { /* try to move the wb point to the south east corner of
                  * the sample average and nearest reference point.
                  * Hopefully the result is less red and more blue */
          agw_d->rg_ratio = STATS_PROC_MAX(agw_d->ave_rg_ratio,
            agw_p->rgbg_grid[agw_p->day_line_1[index][0]]);
          agw_d->bg_ratio = STATS_PROC_MIN(agw_d->ave_bg_ratio,
            agw_p->rgbg_grid[agw_p->day_line_1[index][1]]);
        }
        agw_d->day_line = 1;
        agw_d->day_idx  = index;
        CDBG_AWB("day line 1. rg_ratio %f  bg_ratio %f",
          agw_d->rg_ratio, agw_d->bg_ratio);
      }
    } /* for */

    for (index = 1; index < agw_p->n_day2 - 1 &&
      index < AGW_NUMBER_GRID_POINT; index++) {
      /* examine the daylight line 2 */
      d1 = (agw_d->ave_rg_grid - agw_p->day_line_2[index][0]);
      d2 = (agw_d->ave_bg_grid - agw_p->day_line_2[index][1]);
      dist = d1 * d1 + d2 * d2;

      if (dist < agw_d->min_dist) {
        agw_d->min_dist = dist;
        agw_d->sample_decision = AGW_AWB_HYBRID;
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else { /* try to move the wb point to the south east corner of
                  * the sample average and nearest reference point.
                  * Hopefully the result is less red and more blue */
          agw_d->rg_ratio = STATS_PROC_MAX(agw_d->ave_rg_ratio,
            agw_p->rgbg_grid[agw_p->day_line_2[index][0]]);
          agw_d->bg_ratio = STATS_PROC_MIN(agw_d->ave_bg_ratio,
            agw_p->rgbg_grid[agw_p->day_line_2[index][1]]);
        }
        agw_d->day_line = 2;
        agw_d->day_idx  = index;
        CDBG_AWB("### day line 2. rg_ratio=%f  bg_ratio=%f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);
      } /*end if */
    } /* end for */

    for (index = 0; index < agw_p->n_day3 &&
      index < AGW_NUMBER_GRID_POINT; index++) {
      /* examine the daylight line 2 */
      d1 = (agw_d->ave_rg_grid - agw_p->day3_rg[index]);
      d2 = (agw_d->ave_bg_grid - agw_p->day3_bg[index]);
      dist = d1 * d1 + d2 * d2;
      if (dist < agw_d->min_dist) {
        agw_d->min_dist = dist;
        agw_d->sample_decision = AGW_AWB_HYBRID;
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else { /* try to move the wb point to the south east corner of
                  * the sample average and nearest reference point.
                  * Hopefully the result is less red and more blue */
          agw_d->rg_ratio = STATS_PROC_MAX(agw_d->ave_rg_ratio,
            agw_p->rgbg_grid[agw_p->day3_rg[index]]);
          agw_d->bg_ratio = STATS_PROC_MIN(agw_d->ave_bg_ratio,
            agw_p->rgbg_grid[agw_p->day3_bg[index]]);
        }
        agw_d->day_line = 2;
        agw_d->day_idx  = index;
        CDBG_AWB("### day line 3. rg_ratio=%f  bg_ratio=%f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);
      } /* end if */
    } /* end for */
  } /* end if */
  if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF) { /* LED ON-mode */
    d1 = (agw_d->ave_rg_grid - agw_p->led_rg_grid);
    d2 = (agw_d->ave_bg_grid - agw_p->led_bg_grid);
    dist = d1 * d1 + d2 * d2;

    if (dist < agw_d->min_dist) {
      agw_d->min_dist = dist;
      agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;
    }
    if (agw_d->compact_cluster && agw_d->min_dist <=
      cptr->compact_cluster_to_ref_point_r2 * 4) {
      agw_d->rg_ratio = agw_d->simple_rg_ratio;
      agw_d->bg_ratio = agw_d->simple_bg_ratio;
    } else {
      agw_d->rg_ratio = agw_p->rgbg_grid[agw_d->ave_rg_grid];
      agw_d->bg_ratio = agw_p->rgbg_grid[agw_d->ave_bg_grid];
    }
  }

  if (sproc->input.chromatix->awb_self_cal_enable) {
    if (awb_agw_is_decision_outdoor(agw_d->sample_decision) == 0) {
      /* collect self-calibration statistics, if it's outdoor & bright
       * enough there has been some day light decision in the history
       * & AEC is reaonsably stable */
      agw_d->aec_luma_delta = abs((int)aec_d->target_luma -
        (int) sproc->share.aec_ext.cur_luma);
      CDBG_AWB("Self_cal update:luma target %d, frame luma %d, delta %d",
        aec_d->target_luma, sproc->share.aec_ext.cur_luma, agw_d->aec_luma_delta);

      if (sproc->share.prev_exp_index <= (agw_p->outdoor_index - 46)
        && awb_agw_history_daylight_srch(awb) == 0 &&
        agw_d->aec_luma_delta < 8) {
        /* use unaltered (r/g, b/g) ratios for calibration */
        awb_self_cal_update(sproc, awb, agw_d->ave_rg_ratio, agw_d->ave_bg_ratio);
      }
    }
  }

  if (cptr->white_balance_allow_fline &&
    (awb->search_mode == AWB_AGW_INDOOR_ONLY ||
    awb->search_mode == AWB_AGW_INOUTDOOR_BOTH)) {

    if (agw_p->Fline[0][0] > agw_p->Fline[agw_p->n_fline - 1][0])
      agw_d->p1 = 1; /* from TL to CW */
    else
      agw_d->p1 =-1; /* from CW to TL */

    agw_d->p2 = agw_p->n_fline >> 1; /* p2 is the mid point index */

    for (index = 1; index < agw_p->n_fline - 1 &&
      index < AGW_F_A_LINE_NUM_POINTS; index++) {
      /* examine the flourescent line */
      d1 = (agw_d->ave_rg_grid - agw_p->Fline[index][0]);
      d2 = (agw_d->ave_bg_grid - agw_p->Fline[index][1]);
      dist = d1 * d1 + d2 * d2;

      if (dist < agw_d->min_dist) {
        agw_d->min_dist = dist;
        if (index < agw_d->p2) { /* first half of the line */
          if (agw_d->p1 > 0) { /* TL to CW */
            agw_d->sample_decision = AGW_AWB_INDOOR_WARM_FLO;
          } else { /* CW to TL */
            agw_d->sample_decision = AGW_AWB_INDOOR_COLD_FLO;
          }
        } else { /* second half of the line */
          if (agw_d->p1 > 0) { /* TL to CW */
            agw_d->sample_decision = AGW_AWB_INDOOR_COLD_FLO;
          } else { /* CW to TL */
            agw_d->sample_decision = AGW_AWB_INDOOR_WARM_FLO;
          }
        }
        agw_d->f_idx = index;
        /* changed to iltered avg to compensate for sensor variation */
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          /* likely to be grey chart test. must show grey */
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else {
          agw_d->rg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
            agw_p->rgbg_grid[agw_p->Fline[index][0]] +
            (cptr->sample_influence.indoor_influence) * agw_d->ave_rg_ratio);

          agw_d->bg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
            agw_p->rgbg_grid[agw_p->Fline[index][1]] +
            (cptr->sample_influence.indoor_influence) * agw_d->ave_bg_ratio);
        }
        CDBG_AWB("F line. rg_ratio %f, bg_ratio %f",
          agw_d->rg_ratio, agw_d->bg_ratio);
      }
    }
    /* 3A 1.4 change (added day_f_line) */
    if (agw_p->Day_F_line[0][1] > agw_p->
      Day_F_line[agw_p->n_day_f_line - 1][1])
      agw_d->p1 = 1;  /* from D50 to flo */
    else
      agw_d->p1 = -1; /* from flo to D50 */

    agw_d->p2 = agw_p->n_day_f_line >> 1; /* p2 is mid point index */

    for (index = 1; index < agw_p->n_day_f_line - 1 &&
      index < AGW_F_A_LINE_NUM_POINTS; index++) {
      /* examine the flourescent line */
      d1 = (agw_d->ave_rg_grid - agw_p->Day_F_line[index][0]);
      d2 = (agw_d->ave_bg_grid - agw_p->Day_F_line[index][1]);
      dist = d1 * d1 + d2 * d2;
      if (dist < agw_d->min_dist) {
        agw_d->min_dist = dist;

        if (index < agw_d->p2) { /* first half of the line */
          if (agw_d->p1 > 0) { /* from D50 to flo */
            if (sproc->share.prev_exp_index <= ((agw_p->indoor_index)))
              agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1;
            else
              agw_d->sample_decision = AGW_AWB_INDOOR_CUSTOM_FLO;
          } else { /* from flo to D50  */
            agw_d->sample_decision = AGW_AWB_INDOOR_CUSTOM_FLO;
          }
        } else { /* second half of the line */
          if (agw_d->p1 > 0) { /* from D50 to flo */
            agw_d->sample_decision = AGW_AWB_INDOOR_CUSTOM_FLO;
          } else {
            /* from flo to D50 */
            if (sproc->share.prev_exp_index <= ((agw_p->indoor_index)))
              agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT1;
            else
              agw_d->sample_decision = AGW_AWB_INDOOR_CUSTOM_FLO;
          }
        }
        /* changed to filtered avg to compensate for sensor variation */
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {

          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else {
          agw_d->rg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
            agw_p->rgbg_grid[agw_p->Day_F_line[index][0]] +
            (cptr->sample_influence.indoor_influence) * agw_d->ave_rg_ratio);
          agw_d->bg_ratio = (float)((1.0 - cptr->sample_influence.indoor_influence) *
            agw_p->rgbg_grid[agw_p->Day_F_line[index][1]]+
            (cptr->sample_influence.indoor_influence) * agw_d->ave_bg_ratio);
        }
        CDBG_AWB("### Day_F line. rg_ratio=%f  bg_ratio=%f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);
      }
    }
  }
  /* A line 1 */
  if (awb->search_mode == AWB_AGW_INDOOR_ONLY ||
    awb->search_mode == AWB_AGW_INOUTDOOR_BOTH) {

    for (index = 0; index < agw_p->n_aline1 - 1 &&
      index < AGW_F_A_LINE_NUM_POINTS; index++) {
      /* examine the flourescent line */
      d1 = (agw_d->ave_rg_grid - agw_p->Aline1[index][0]);
      d2 = (agw_d->ave_bg_grid - agw_p->Aline1[index][1]);
      dist = d1 * d1 + d2 * d2;

      if (dist < agw_d->min_dist) {
        agw_d->min_dist = dist;
        agw_d->sample_decision = AGW_AWB_INDOOR_INCANDESCENT;
        /* changed to filtered avg to compensate for sensor variation */
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          /* compact cluster, likely to be grey chart */
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else {
          /* not a compact cluster, make it appear warm */
          agw_d->rg_ratio = agw_d->ave_rg_ratio;
          agw_d->bg_ratio = agw_d->ave_bg_ratio;
        }
        CDBG_AWB("### A line1. rg_ratio=%f  bg_ratio=%f\n",
          agw_d->rg_ratio, agw_d->bg_ratio);
      }
      agw_d->a_line = 2;
      agw_d->a_idx = 0; /* set it to A */
    }
    /* Aline 2 */
    /* compare the rg ratio of starting & ending point of A line 2 */
    if (agw_p->Aline2[0][0] > agw_p->Aline2[agw_p->n_aline2 - 1][0])
      agw_d->p1 = 1; /* from H to A */
    else
      agw_d->p1 = -1; /* from A to H */
    /* p2 is the mid point index */
    agw_d->p2 = agw_p->n_aline2 >> 1;

    for (index = 1; index < agw_p->n_aline2 - 1 &&
      index < AGW_F_A_LINE_NUM_POINTS; index++) {
      /* examine the flourescent line */
      d1 = (agw_d->ave_rg_grid - agw_p->Aline2[index][0]);
      d2 = (agw_d->ave_bg_grid - agw_p->Aline2[index][1]);
      dist = d1 * d1 + d2 * d2;

      if (dist < agw_d->min_dist) {
        agw_d->a_line = 2;
        agw_d->a_idx = index; /* set it to A */
        agw_d->min_dist = dist; /* update the new minimum distance */

        if (index < agw_d->p2) { /* first half */
          if (agw_d->p1 > 0) { /* H to A */
            agw_d->sample_decision = AGW_AWB_HORIZON;
          } else { /* A to H */
            agw_d->sample_decision = AGW_AWB_INDOOR_INCANDESCENT;
          }
        } else { /* second half of the line */
          if (agw_d->p1 > 0) { /* H to A */
            agw_d->sample_decision = AGW_AWB_INDOOR_INCANDESCENT;
          } else { /* A to H */
            agw_d->sample_decision = AGW_AWB_HORIZON;
          }
        }
        /* changed to use filtered average to compensate for sensor
         * variation */
        if (agw_d->compact_cluster && agw_d->min_dist <=
          cptr->compact_cluster_to_ref_point_r2 * 4) {
          /* compact cluster, likely to be grey chart */
          agw_d->rg_ratio = agw_d->simple_rg_ratio;
          agw_d->bg_ratio = agw_d->simple_bg_ratio;
        } else { /* not a compact cluster, make it appear warm */
          agw_d->rg_ratio = agw_d->ave_rg_ratio;
          agw_d->bg_ratio = agw_d->ave_bg_ratio;
        }
        CDBG_AWB("A line2. rg_ratio %f, bg_ratio %f",
          agw_d->rg_ratio, agw_d->bg_ratio);
      }
    }
  }
  /* modify outdoor average (rg,bg) ratio based on brightness
   * see if this can prevent the purple sky */
  if (agw_d->sample_decision == AGW_AWB_HYBRID ||
    agw_d->sample_decision == AGW_AWB_OUTDOOR_SUNLIGHT ||
    agw_d->sample_decision ==AGW_AWB_OUTDOOR_SUNLIGHT1) {

    if (agw_d->bg_ratio > agw_p->shifted_d50_bg) {
      agw_d->p1 = agw_p->outdoor_index;
      agw_d->p2 = agw_p->outdoor_index / 2;

      agw_d->rg_target = 0.5 * (agw_d->rg_ratio + agw_p->shifted_d50_rg);
      agw_d->bg_target = 0.5 * (agw_d->bg_ratio + agw_p->shifted_d50_bg);

      if ((int)sproc->share.prev_exp_index < agw_d->p1 &&
        (int)sproc->share.prev_exp_index >= agw_d->p2) {
        w1 = (float) (sproc->share.prev_exp_index - agw_d->p2) /
          (float) (agw_d->p1 - agw_d->p2);
        w2 = 1.0 - w1;

        agw_d->rg_ratio = STATS_PROC_MAX(agw_d->rg_ratio, w1 *
          agw_d->rg_ratio + w2 * agw_d->rg_target);
        agw_d->bg_ratio = STATS_PROC_MIN(agw_d->bg_ratio, w1 *
          agw_d->bg_ratio + w2 * agw_d->bg_target);

        CDBG_AWB("Purple sky prevention: w1=%f, w2=%f, outdoor_mid<exp_idx \
              <outdoor_idx\n", w1, w2);
        CDBG_AWB("rg_target %f, bg_target %f",
          agw_d->rg_target, agw_d->bg_target);
        CDBG_AWB("after adjustment: rg_ratio %f, bg_ratio %f",
          agw_d->rg_ratio, agw_d->bg_ratio);
      } else if ((int)sproc->share.prev_exp_index < agw_d->p2) {

        agw_d->rg_ratio = STATS_PROC_MAX(agw_d->rg_ratio, agw_d->rg_target);
        agw_d->bg_ratio = STATS_PROC_MIN(agw_d->bg_ratio, agw_d->bg_target);
        CDBG_AWB("Purple sky prevention: after adjustment: rg_ratio=%f, \
              bg_ratio=%f", agw_d->rg_ratio, agw_d->bg_ratio);
      }
    }
  }
  if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
    agw_d->sample_decision == AGW_AWB_HORIZON) {
    awb->white_decision = AGW_AWB_INVALID_LIGHT;
    CDBG_HIGH("white stat, decision=Horizon, low confidence. return");
    return -1;
  }

  if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
    (agw_d->sample_decision == AGW_AWB_HORIZON ||
    agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT)) {
    /* if the current stat is white and its decision is in the A cluster */
    if (awb->regular_decision!= AGW_AWB_HORIZON &&
      awb->regular_decision!= AGW_AWB_INDOOR_INCANDESCENT) {
      /* if the previous grey decision is not in A cluster
       * (that is, day, F or even invalid) reject this white stat */
      awb->white_decision = AGW_AWB_INVALID_LIGHT;
      CDBG_HIGH("%s: white stat, decision=A or Horizon, grey is"
        "not, low confidence. return", __func__);
      return -1;
    }
  }

  if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
    agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT &&
    agw_d->a_line == 2 && agw_d->a_idx > (agw_p->n_aline2 / 5)) {

    awb->white_decision = AGW_AWB_INVALID_LIGHT;
    CDBG_HIGH("%s: white stat, decision=A, beyond 1/5 of Aline2, low "
      "confidence. return", __func__);
    return -1;
  }

  awb_agw_set_rg_bg_ratios(sproc, awb);
  if (awb->current_awb_stat_config == AWB_STAT_WHITE)
    awb->white_decision = agw_d->sample_decision;
  else if (awb->current_awb_stat_config == AWB_STAT_REGULAR)
    awb->regular_decision = agw_d->sample_decision;

  if (agw_d->sample_decision == AGW_AWB_HYBRID) {
    CDBG_AWB("day light line %d, day idx %d",
      agw_d->day_line, agw_d->day_idx);
  }
  if ((int)awb->regular_decision < 0 &&
    (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF)) {
    CDBG_HIGH("%s: REG dec %d. return", __func__, awb->regular_decision);
    return -1;
  }
  /* low confidence: not bright enough, only a tiny fraction is close to
   * outdoor, a lot of outliers are in other lighting conditions this is
   * the case of indoor situation with small amount of outdoor objects.
   * since the major part of the scene is indoor (due to colored wall &
   * objects, it may not have valid indoor samples), it should not apply
   * outdoor white balance gain */
  if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {

    if (awb->current_awb_stat_config == AWB_STAT_REGULAR &&
      sproc->share.prev_exp_index > awb->inoutdoor_midpoint &&
      awb_agw_is_decision_outdoor(agw_d->sample_decision) == 0 &&
      agw_d->simple_bg_ratio < agw_p->blue_gain_table[AGW_AWB_INDOOR_COLD_FLO]
      && !agw_d->green_cnt && agw_d->day_cluster < ((stats->region_cnt *
      cptr->outdoor_valid_sample_cnt_threshold) / 100)) {
      CDBG_HIGH("%s: regular decision %d, day_cluster %d, may be indoor,"
        " return", __func__, awb->regular_decision, agw_d->day_cluster);
      return -1;
    }
    /* reject the case of compact cluster that is too red or too blue */
    if (agw_d->compact_cluster == 1 && (agw_d->ave_rg_grid > (agw_p->
      rg_grid[AGW_AWB_HORIZON] + 3) || agw_d->ave_bg_grid <
      (agw_p->bg_grid[AGW_AWB_HORIZON] - 3) || agw_d->ave_bg_grid >
      agw_p->bg_grid[AGW_AWB_OUTDOOR_CLOUDY])) {
      CDBG_HIGH("%s: REG dec %d, compact cluster, too red or blue too weak"
        " strong. return", __func__, awb->regular_decision);

      return -1; /* lock on to the previous decision. */
    }
    /* in some dark situation, a small number of near grey samples distort
     * the white balance although it should be near D65, but these near
     * grey samples draw it close to D50. */
    if (sproc->share.prev_exp_index > ((awb->inoutdoor_midpoint +
      agw_p->indoor_index) >> 1) && agw_d->smpl_cnt > 0) {
      /* there are some near grey samles, dark. */
      if (agw_d->sample_decision == AGW_AWB_OUTDOOR_SUNLIGHT1 ||
        agw_d->sample_decision == AGW_AWB_OUTDOOR_NOON ||
        (agw_d->sample_decision == AGW_AWB_HYBRID && agw_d->day_line == 2)) {
        /* if it's daylight line 2 or D50, it's low confidence */
        if (agw_d->green_cnt > ((cptr->outdoor_green_upper_threshold *
          stats->region_cnt) / 100) && !agw_d->compact_cluster &&
          agw_d->day_cluster < ((cptr->outdoor_valid_sample_cnt_threshold *
          stats->region_cnt) / 100)) {
          /* if enough green is available, not compact cluster, valid
           * samples in day cluster is small then move it up to D65. */
          CDBG_AWB("### dark, enough green, not campact. few samples in day \
                cluster, decision=%d ->D65\n", agw_d->sample_decision);
          agw_d->sample_decision = AGW_AWB_OUTDOOR_SUNLIGHT;

          if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
            awb->white_decision = agw_d->sample_decision;
            agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
            agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
            awb->white_ave_rg_ratio = agw_d->rg_ratio;
            awb->white_ave_bg_ratio = agw_d->bg_ratio;
          } else if (awb->current_awb_stat_config == AWB_STAT_REGULAR) {
            awb->regular_decision = agw_d->sample_decision;
            agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
            agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
            awb->regular_ave_rg_ratio = agw_d->rg_ratio;
            awb->regular_ave_bg_ratio = agw_d->bg_ratio;
          }
          agw_d->decision_changed = TRUE;
        }
      }
    }
    /* for outdoor situation, if it's very bright and a compact cluster,
     * turn them into D50 */
    if (agw_d->sample_decision == AGW_AWB_OUTDOOR_SUNLIGHT ||
      agw_d->sample_decision == AGW_AWB_OUTDOOR_CLOUDY   ||
      agw_d->sample_decision == AGW_AWB_HYBRID ||
      agw_d->sample_decision == AGW_AWB_OUTDOOR_NOON) {

      if (agw_d->compact_cluster &&
        awb->current_awb_stat_config == AWB_STAT_REGULAR &&
        sproc->share.prev_exp_index <= agw_p->outdoor_index) {

        CDBG_AWB("decision changed from %d to D50, bright and compact\n",
          agw_d->sample_decision);
        agw_d->sample_decision  = AGW_AWB_OUTDOOR_SUNLIGHT1;
        agw_d->decision_changed = TRUE;
        awb->regular_decision = agw_d->sample_decision;
        agw_d->rg_ratio = agw_p->red_gain_table[agw_d->sample_decision];
        agw_d->bg_ratio = agw_p->blue_gain_table[agw_d->sample_decision];
      }
    }

    /* white statistics supposed to get a narrow distribution
     * if the distribution is large, do not trust it. */
    if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
      agw_d->p1 = 0;
      if (agw_d->day_cluster > 0)
        agw_d->p1++;
      if (agw_d->f_cluster > 0)
        agw_d->p1++;
      if (agw_d->a_cluster > 0)
        agw_d->p1++;

      if (agw_d->p1 == 1)
        awb->white_has_single_peak = TRUE;
      else
        awb->white_has_single_peak = FALSE;

      if (agw_d->p1 >= 2 || agw_d->outlier_cnt > 0) {
        if (awb->white_y_min_percent <= YMIN_HIGH_LIMIT)
          awb->white_y_min_percent++;
        awb->white_decision = AGW_AWB_INVALID_LIGHT;
        CDBG_HIGH("%s: valid, white stat has multiple clusters, (D,F,A)="
          "(%d,%d,%d) Ymin pct %d", __func__, agw_d->day_cluster,
          agw_d->f_cluster, agw_d->a_cluster, awb->white_y_min_percent);
        return -1;
      }
    }
    if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
      (agw_d->sample_decision == AGW_AWB_HORIZON ||
      agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT)) {
      /* if current stat is white, white is A/H, but reg is
       * not A/H, reject white decision */
      if (awb->regular_decision != AGW_AWB_HORIZON &&
        awb->regular_decision != AGW_AWB_INDOOR_INCANDESCENT) {
        awb->white_decision = AGW_AWB_INVALID_LIGHT;
        CDBG_HIGH("%s:  white is A/H, grey is not A/H. reject", __func__);
        return -1;
      } else { /* grey stat is also A or H */
        if (agw_d->rg_ratio > awb->regular_ave_rg_ratio) {
          /* wrhite world gets redder stat, discard white world */
          agw_d->rg_ratio = awb->regular_ave_rg_ratio;
          agw_d->bg_ratio = awb->regular_ave_bg_ratio;
          agw_d->decision_changed = TRUE;
          CDBG_AWB("white & grey are both A/H,white is redder."
            " replaced by grey stat.\n");
        }
      }
    }
    if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
      (agw_d->sample_decision == AGW_AWB_INDOOR_CUSTOM_FLO ||
      agw_d->sample_decision == AGW_AWB_INDOOR_COLD_FLO ||
      agw_d->sample_decision == AGW_AWB_INDOOR_WARM_FLO)) {
      /* current stat is white, decision is F type */
      if (awb->regular_decision == AGW_AWB_INDOOR_CUSTOM_FLO ||
        awb->regular_decision == AGW_AWB_INDOOR_COLD_FLO ||
        awb->regular_decision == AGW_AWB_INDOOR_WARM_FLO) {
        /* prior grey decision is also F type */
        if (awb->regular_decision != agw_d->sample_decision) {
          /* they are different, which means they might differ in
           * (R/G,B/G) too much and causes instability. */
          awb->white_decision = AGW_AWB_INVALID_LIGHT;
          CDBG_HIGH("%s: white and grey are F, but different"
            " type. reject white", __func__);
          return -1;
        }
      }
    }

    /* for dark situation, if white decision is outdoor while
     * regular decision is indoor, reject */
    if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
      if (sproc->share.prev_exp_index >= agw_p->indoor_index) {
        if (awb_agw_is_decision_outdoor(awb->white_decision) == 0) {
          if (awb->regular_decision == AGW_AWB_HORIZON ||
            awb->regular_decision ==
            AGW_AWB_INDOOR_INCANDESCENT ||
            awb_agw_is_decision_FL(awb->regular_decision) == 0) {
            CDBG_HIGH("%s: valid, white dec %d, reg dec %d. low confi RET",
              __func__, awb->white_decision, awb->regular_decision);
            awb->white_decision = AGW_AWB_INVALID_LIGHT;
            return -1;
          }
        }
      }
      // white is A/H, grey is FL
      if (agw_d->sample_decision == AGW_AWB_HORIZON ||
        agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT) {
        if (awb->regular_decision == AGW_AWB_INDOOR_COLD_FLO ||
          awb->regular_decision == AGW_AWB_INDOOR_WARM_FLO ||
          awb->regular_decision == AGW_AWB_INDOOR_CUSTOM_FLO) {
          CDBG_HIGH("%s: white decision %d, reg decision %d. low confi"
            " RET", __func__, awb->white_decision, awb->regular_decision);
          awb->white_decision = AGW_AWB_INVALID_LIGHT;
          return -1;
        }
      }
    }

    if (awb_agw_is_decision_FL(awb->regular_decision) == 0) {
      if (awb_agw_FL_type_heuristics(sproc, awb) < 0)
        return -1;
    } else if (agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT ||
      agw_d->sample_decision == AGW_AWB_HORIZON) {
      if (awb_agw_INC_type_heuristics(sproc, awb) < 0)
        return -1;
    }
  } else
    awb_agw_daylight_heuristics(sproc, awb);

  if (agw_d->compact_cluster == 0)
    /* reject case:  non-compact distribution that is beyond horizon */
    if (agw_d->ave_rg_grid > (agw_p->rg_grid[AGW_AWB_HORIZON] +
      (agw_p->outlier_distance >> 1)) || agw_d->ave_bg_grid <
      (agw_p->bg_grid[AGW_AWB_HORIZON] - agw_p->outlier_distance)) {
      if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
        CDBG_HIGH("%s: REG decision %d, not compact, too red or weak"
          " blue, return", __func__, awb->regular_decision);
        return -1;  /* do not make decision */
      }
    }
  if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
    /* Run history tracking (including lock/unlock) */
    awb_agw_history_update(sproc, awb);
    awb_agw_adjust_rg_bg_by_history (sproc, awb);
    awb_agw_set_rg_bg_ratios(sproc, awb);
    if (awb->current_awb_stat_config == AWB_STAT_REGULAR)
      awb->regular_decision = agw_d->sample_decision;
    else
      awb->white_decision = agw_d->sample_decision;
  }
  agw_d->g_gain = 1.0;
  agw_d->r_gain = 1.0 / agw_d->rg_ratio;
  agw_d->b_gain = 1.0 / agw_d->bg_ratio;
  return 0;
} /* awb_agw_valid_sample_hueristics */

/*===========================================================================
 * FUNCTION    - awb_agw_adjust_gains_for_rolloff -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_adjust_gains_for_rolloff(stats_proc_t *sproc, awb_t *awb)
{
  float   w1, w2;

  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  /* red and gain adjustment for Lens rolloff Correction */
  if ((int)agw_d->sample_decision < AWB_NUMBER_OF_REFERENCE_POINT) {
    if (agw_d->decision_changed == TRUE) {
      /* decision has been changed, just use the reference point gain adj
       * without any interpolation */
      agw_d->output.gain_adj_r = agw_p->red_gain_adj[agw_d->sample_decision];
      agw_d->output.gain_adj_b = agw_p->blue_gain_adj[agw_d->sample_decision];

      CDBG_AWB("gain adj: r_gain_adj %f, b_gain_adj %f",
        agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
    } else { /* decision has not been changed */
      if (agw_d->sample_decision == AGW_AWB_HORIZON  ||
        agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT) {
        if (agw_d->a_idx < 0) {
          agw_d->output.gain_adj_r = agw_p->red_gain_adj[agw_d->sample_decision];
          agw_d->output.gain_adj_b = agw_p->blue_gain_adj[agw_d->sample_decision];
          CDBG_AWB("gain adj: r_gain_adj %f, b_gain_adj %f",
            agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
        } else {
          w1 = ((float) agw_p->n_aline2 - agw_d->a_idx) / (float) agw_p->n_aline2;
          w2 = 1.0 - w1;

          agw_d->output.gain_adj_r = (w2*agw_p->red_gain_adj[AGW_AWB_HORIZON]
            + w1 * agw_p->red_gain_adj[AGW_AWB_INDOOR_INCANDESCENT]);
          agw_d->output.gain_adj_b = (w2*agw_p->blue_gain_adj[AGW_AWB_HORIZON]
            + w1 * agw_p->blue_gain_adj[AGW_AWB_INDOOR_INCANDESCENT]);

          CDBG_AWB("A line2 adj: r_gain_adj %f ,b_gain_adj %f",
            agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
        }
      } else if (agw_d->sample_decision == AGW_AWB_INDOOR_COLD_FLO ||
        agw_d->sample_decision == AGW_AWB_INDOOR_WARM_FLO) {
        if (agw_d->f_idx < 0) {
          agw_d->output.gain_adj_r =agw_p->red_gain_adj[agw_d->sample_decision];
          agw_d->output.gain_adj_b = agw_p->blue_gain_adj[agw_d->sample_decision];
          CDBG_AWB("gain adj: r_gain_adj %f, b_gain_adj %f",
            agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
        } else {
          w1 = ((float) agw_p->n_fline - agw_d->f_idx) / (float) agw_p->n_fline;
          w2 = 1.0 - w1;

          agw_d->output.gain_adj_r = (w2 * agw_p->
            red_gain_adj[AGW_AWB_INDOOR_WARM_FLO] + w1 *
            agw_p->red_gain_adj[AGW_AWB_INDOOR_COLD_FLO]);
          agw_d->output.gain_adj_b = (w2 * agw_p->
            blue_gain_adj[AGW_AWB_INDOOR_WARM_FLO] + w1 *
            agw_p->blue_gain_adj[AGW_AWB_INDOOR_COLD_FLO]);
          CDBG_AWB("F line adj:r_gain_adj %f, b_gain_adj %f",
            agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
        }
      } else {
        agw_d->output.gain_adj_r = agw_p->red_gain_adj[agw_d->sample_decision];
        agw_d->output.gain_adj_b = agw_p->blue_gain_adj[agw_d->sample_decision];
        CDBG_AWB("gain adj: r_gain_adj %f, b_gain_adj %f",
          agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
      }
    }
  } else { /* daylight line */
    if (agw_d->day_line == 1) {
      w1 = ((float) agw_p->n_day1 - (float) agw_d->day_idx) / (float) agw_p->n_day1;
      w2 = 1.0 - w1;

      agw_d->output.gain_adj_r = (w2 * agw_p->
        red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT] + w1 *
        agw_p->red_gain_adj[AGW_AWB_OUTDOOR_CLOUDY]);
      agw_d->output.gain_adj_b = (w2 * agw_p->
        blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT] + w1 *
        agw_p->blue_gain_adj[AGW_AWB_OUTDOOR_CLOUDY]);
      CDBG_AWB("day1 adj: r_gain_adj %.0f , b_gain_adj %.0f",
        agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
    } else if (agw_d->day_line == 2) {
      w1 = ((float) agw_p->n_day2 - (float) agw_d->day_idx) / (float) agw_p->n_day2;
      w2 = 1.0 - w1;
      agw_d->output.gain_adj_r = (w2 * agw_p->
        red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT1] + w1 *
        agw_p->red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT]);
      agw_d->output.gain_adj_b = (w2 * agw_p->
        blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT1] + w1 *
        agw_p->blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT]);
      CDBG_AWB("day2 adj: r_gain_adj %.0f , b_gain_adj %.0f",
        agw_d->output.gain_adj_r, agw_d->output.gain_adj_b);
    } else {
      /* this is a erroneous situation, should not get here */
      agw_d->output.gain_adj_r =agw_p->red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT];
      agw_d->output.gain_adj_b=agw_p->blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT];
      CDBG_HIGH("ERROR: r_gain_adj %.0f , b_gain_adj %.0f",
        agw_p->red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT],
        agw_p->blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT]);
    }
  }
} /* awb_agw_adjust_gains_for_rolloff */

/*===========================================================================
 * FUNCTION    - awb_agw_pullup_channel_gains -
 *
 * DESCRIPTION: find minimal gain and pull every color channel gain >= 1.0
 *==========================================================================*/
static void awb_agw_pullup_channel_gains(stats_proc_t *sproc, awb_t *awb)
{
  float gain_min = 1.0;
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  if (agw_d->r_gain <= gain_min)
    gain_min = agw_d->r_gain;
  if (agw_d->b_gain <= gain_min)
    gain_min = agw_d->b_gain;

  /* modify the global variables */
  agw_d->r_gain = agw_d->r_gain * (1.0 / gain_min);
  agw_d->g_gain = agw_d->g_gain * (1.0 / gain_min);
  agw_d->b_gain = agw_d->b_gain * (1.0 / gain_min);

  CDBG_HIGH("%s: gain_min %f, r_gain %f, g_gain %f, b_gain %f", __func__,
    gain_min, agw_d->r_gain, agw_d->g_gain, agw_d->b_gain);
} /* awb_agw_pullup_channel_gains */

/*===========================================================================
 * FUNCTION    - awb_agw_lowlight_adjust -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_agw_lowlight_adjust(stats_proc_t *sproc, awb_t *awb)
{
  low_light_adj_type *lut_entry;
  int exp_index;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  chromatix_parms_type *cptr = sproc->input.chromatix;

  exp_index = sproc->share.prev_exp_index;
  if (exp_index <= cptr->AWB_lowlight_LUT.lut_entry[0].lux_index) {
    lut_entry = &(cptr->AWB_lowlight_LUT.lut_entry[0]);
  } else if (exp_index <= cptr->AWB_lowlight_LUT.lut_entry[1].lux_index) {
    lut_entry = &(cptr->AWB_lowlight_LUT.lut_entry[1]);
  } else if (exp_index <= cptr->AWB_lowlight_LUT.lut_entry[2].lux_index) {
    lut_entry = &(cptr->AWB_lowlight_LUT.lut_entry[2]);
  } else if (exp_index <= cptr->AWB_lowlight_LUT.lut_entry[3].lux_index) {
    lut_entry = &(cptr->AWB_lowlight_LUT.lut_entry[3]);
  } else if (exp_index <= cptr->AWB_lowlight_LUT.lut_entry[4].lux_index) {
    lut_entry = &(cptr->AWB_lowlight_LUT.lut_entry[4]);
  } else {
    lut_entry = &(cptr->AWB_lowlight_LUT.lut_entry[5]);
  }

  agw_p->green_offset_bg = (cptr->green_offset_bg
      + (int) lut_entry->green_bg_offset_adj) * 2;
  agw_p->green_offset_rg = (cptr->green_offset_rg
      + (int) lut_entry->green_rg_offset_adj) * 2;
  agw_p->outlier_distance = (cptr->outlier_distance
      + (int) lut_entry->outlier_dist_adj) * 2;
}

/*===========================================================================
 * FUNCTION    - awb_agw_algo -
 *
 * DESCRIPTION:
 *==========================================================================*/
awb_gain_t awb_agw_algo(stats_proc_t *sproc, awb_t *awb)
{
  awb_input_stats_type *stats  = &(awb->stats_ptr);
  chromatix_parms_type *cptr   = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p           = &(awb->agw);
  awb_advanced_grey_world_algo_data_t *agw_d = &(awb->agw_d);

  awb_agw_init(sproc, awb);

  if (stats == NULL)
    CDBG_AWB("ERROR stats_ptr NULL !!!\n");

  if (awb_agw_aec_settle_check(sproc, awb) < 0)
    return agw_d->output;

  if (awb->current_awb_stat_config == AWB_STAT_REGULAR)
    sproc->share.grey_world_stats = TRUE;
  else
    sproc->share.grey_world_stats = FALSE;

  /* set initial condition for rg_ratio & bg_ratio in case they fall through */
  if (awb->current_awb_stat_config == AWB_STAT_REGULAR &&
    awb->regular_decision != AGW_AWB_INVALID_LIGHT) {
    agw_d->rg_ratio = awb->regular_ave_rg_ratio;
    agw_d->bg_ratio = awb->regular_ave_bg_ratio;
  }
  if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
    awb->white_decision != AGW_AWB_INVALID_LIGHT) {
    agw_d->rg_ratio = awb->white_ave_rg_ratio;
    agw_d->bg_ratio = awb->white_ave_bg_ratio;
  }

  CDBG_AWB("indoor_index %ld, outdoor_index %ld, max_index %d",
    agw_p->indoor_index, agw_p->outdoor_index, aec_d->exp_tbl_val - 1);

  CDBG_HIGH("%s: LED STATE %d, LED MODE %d", __func__, sproc->input.
    flash_info.led_state, sproc->input.flash_info.led_mode);

  if (cptr->AWB_lowlight_LUT.enable) {
    awb_agw_lowlight_adjust(sproc, awb);
  }

  sproc->share.awb_asd_sync_flag = TRUE;
  if (awb_agw_classify_samples(sproc, awb) < 0)
    return agw_d->output;

  agw_d->g_ave = (agw_d->g_ave / stats->bin_cnt) >> RGBG_GRID_Q_NUM;
  CDBG_AWB("%s: agw_d->g_ave %d", __func__, agw_d->g_ave);
  if (agw_d->g_ave < 10 && agw_d->g_max < 20 &&
    sproc->share.prev_exp_index >= (int)aec_d->exp_tbl_val - 1) {
    CDBG_HIGH("%s: low light, low signal, reject", __func__);
    return agw_d->output;
  }
  if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
    int cnt = 0;
    if (agw_d->day_cluster > 0) cnt++;
    if (agw_d->f_cluster   > 0) cnt++;
    if (agw_d->a_cluster   > 0) cnt++;
    if (cnt > 1) {
      if (awb->white_y_min_percent < YMIN_HIGH_LIMIT)
        awb->white_y_min_percent++;
      CDBG_HIGH("%s: white world, multi-cluster, reject. Ymin pct %d",
        __func__, awb->white_y_min_percent);
      awb->white_decision = AGW_AWB_INVALID_LIGHT;
      return agw_d->output;
    }
  }
  awb_agw_compute_rg_bg_for_sgw(sproc, awb);
  CDBG_HIGH("%s: SGW: rg_ratio %f, bg_ratio %f, valid smpl cnt %d", __func__,
    agw_d->simple_rg_ratio, agw_d->simple_bg_ratio, agw_d->smpl_cnt);

  if (agw_d->green_cnt > 0) /* compute the average of the green samples */
    agw_d->green_bgx /= agw_d->green_cnt;

  if (awb->current_awb_stat_config == AWB_STAT_WHITE)
    awb->reg_green_cnt = agw_d->green_cnt;

  CDBG_HIGH("%s: outdoor_green %d, indoor green %d, x_cnt %d", __func__,
    agw_d->green_cnt, agw_d->indoor_green_cnt, agw_d->x_cnt);
  CDBG_HIGH("%s: day cluster %d, f cluster %d, a_cluster %d", __func__,
    agw_d->day_cluster, agw_d->f_cluster, agw_d->a_cluster);

  if (awb->current_awb_stat_config == AWB_STAT_WHITE &&
    !agw_d->day_cluster && !agw_d->a_cluster && !agw_d->f_cluster) {
    if (sproc->share.prev_exp_index == (int)aec_d->exp_tbl_val - 1) {
      if (awb->white_y_min_percent >= (YMIN_LOW_LIMIT - 20))
        awb->white_y_min_percent--;
    } else if (awb->white_y_min_percent >= YMIN_LOW_LIMIT)
      awb->white_y_min_percent--;

    CDBG_HIGH("%s: white stat, all outliers, low confidence, Ymin pct %d return",
      __func__, awb->white_y_min_percent);
    awb->white_decision = AGW_AWB_INVALID_LIGHT;
    return agw_d->output;
  }

  if (awb_agw_process_low_light(sproc, awb) < 0)
    return agw_d->output;

  if (agw_d->low_light == FALSE) { /* Normal Light */
    /* Find dominant cluster. A dominant cluster means the number of samples
     * fall into this cluster exceeds a user-defined threshold */
    if (agw_d->smpl_cnt > 0)
      awb_agw_find_dominant_cluster(sproc, awb);
    else if (awb_agw_outlier_hueristics(sproc, awb) < 0)
      return agw_d->output;  /* all the samples are ouliers */

    if (agw_d->all_outliers != 1) /* valid samples */
      if (awb_agw_valid_sample_hueristics(sproc, awb) < 0)
        return agw_d->output;

    if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
      /* reject the decision that is contrary to the search mode */
      if (awb->search_mode == AWB_AGW_INDOOR_ONLY)
        /* suupposed to be indoor only */
        if (awb_agw_is_decision_outdoor(agw_d->sample_decision) == 0)
          return agw_d->output;

      if (awb->search_mode == AWB_AGW_OUTDOOR_ONLY)
        /* suupposed to be OUTdoor only */
        if (agw_d->sample_decision == AGW_AWB_HORIZON ||
          agw_d->sample_decision == AGW_AWB_INDOOR_INCANDESCENT ||
          awb_agw_is_decision_FL(agw_d->sample_decision) == 0)
          return agw_d->output;
    }
    awb_agw_adjust_gains_for_rolloff(sproc, awb);
  } else { /* under low-light only red/blue gain_adj need to be decided */
    agw_d->output.gain_adj_r = agw_p->red_gain_adj[agw_d->sample_decision];
    agw_d->output.gain_adj_b = agw_p->blue_gain_adj[agw_d->sample_decision];
  }
  /* find the minimal gain and pull every color channel gain >= 1.0 */
  awb_agw_pullup_channel_gains(sproc, awb);

  /* confident to use the gains and decision */
  agw_d->output.is_confident    = TRUE;
  agw_d->output.sample_decision = agw_d->sample_decision;
  agw_d->output.wb_gain_r       = agw_d->r_gain;
  agw_d->output.wb_gain_g       = agw_d->g_gain;
  agw_d->output.wb_gain_b       = agw_d->b_gain;
  return agw_d->output;
} /* awb_agw_algo */

