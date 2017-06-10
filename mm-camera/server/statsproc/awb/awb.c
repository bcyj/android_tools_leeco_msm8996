/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "camera.h"
#include "stats_proc.h"
#include "awb.h"

/*==========================================================================
 * FUNCTION    - awb_cap_gain -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void awb_cap_gain (float *cur_gain, float max_gain, float min_gain)
{
  if (*cur_gain > max_gain)
    *cur_gain = max_gain;
  else if (*cur_gain < min_gain)
    *cur_gain = min_gain;
} /* awb_cap_gain */

/*==========================================================================
 * FUNCTION    - awb_set_grid_index -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void awb_set_grid_index (awb_t *awb, int awb_min_ratio_x, int awb_max_ratio_x)
{
  int index, index2, rgx, bgx;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  /* find the location of the 8 lighting conditions on the grid matrix */
  for (index = 0; index < AWB_NUMBER_OF_REFERENCE_POINT; index++) {
    /* no normalization, need to set rg_adj and bg_adj to be 1 */
    rgx = agw_p->red_gain_table_x[index];
    bgx = agw_p->blue_gain_table_x[index];

    if (rgx < awb_min_ratio_x)
      agw_p->rg_grid[index] = 0;
    else if (rgx >= awb_max_ratio_x)
      agw_p->rg_grid[index] = AGW_NUMBER_GRID_POINT - 1;
    else {
      for (index2 = 0; index2 < AGW_NUMBER_GRID_POINT - 1; index2++) {
        if (((rgx - agw_p->rgbg_grid_x[index2]) *
          (rgx  - agw_p->rgbg_grid_x[index2 + 1])) <= 0) {
          agw_p->rg_grid[index] = index2;
          break;
        }
      }
    }
    /* convert bg ratio from float to grid */
    if (bgx < awb_min_ratio_x)
      agw_p->bg_grid[index] = 0;
    else if (bgx >= awb_max_ratio_x)
      agw_p->bg_grid[index] = AGW_NUMBER_GRID_POINT - 1;
    else {
      for (index2 = 0; index2 < AGW_NUMBER_GRID_POINT - 1; index2++) {
        if (((bgx - agw_p->rgbg_grid_x[index2]) *
          (bgx - agw_p->rgbg_grid_x[index2 + 1])) <= 0) {
          agw_p->bg_grid[index] = index2;
          break;
        }
      }
    }
  }
} /* awb_set_grid_index */

/*===========================================================================
 * FUNCTION    - awb_set_min_max_ratios -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_set_min_max_ratios (awb_t *awb, float *awb_min_ratio,
  float *awb_max_ratio)
{
  int i;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  for (i = 0; i < AWB_NUMBER_OF_REFERENCE_POINT; i++) {

    if (agw_p->red_gain_table[i] > *awb_max_ratio)
      *awb_max_ratio = agw_p->red_gain_table[i];
    if (agw_p->blue_gain_table[i] > *awb_max_ratio)
      *awb_max_ratio = agw_p->blue_gain_table[i];
    if (agw_p->red_gain_table[i] < *awb_min_ratio)
      *awb_min_ratio = agw_p->red_gain_table[i];
    if (agw_p->blue_gain_table[i] < *awb_min_ratio)
      *awb_min_ratio = agw_p->blue_gain_table[i];
    /* prepare fixed point red/blue gain tables */
    agw_p->red_gain_table_x[i] =
      (int) (agw_p->red_gain_table[i] * (1 << RGBG_GRID_Q_NUM));
    agw_p->blue_gain_table_x[i] =
      (int) (agw_p->blue_gain_table[i] * ( 1 << RGBG_GRID_Q_NUM));
  }
  agw_p->led_rg_ratio_x = (int)agw_p->led_rg_ratio * (1 <<RGBG_GRID_Q_NUM);
  agw_p->led_bg_ratio_x = (int)agw_p->led_bg_ratio * (1 <<RGBG_GRID_Q_NUM);

  if (*awb_min_ratio > agw_p->led_rg_ratio)
    *awb_min_ratio = agw_p->led_rg_ratio;
  if (*awb_min_ratio > agw_p->led_bg_ratio)
    *awb_min_ratio = agw_p->led_bg_ratio;
  if (*awb_max_ratio < agw_p->led_rg_ratio)
    *awb_max_ratio = agw_p->led_rg_ratio;
  if (*awb_max_ratio < agw_p->led_bg_ratio)
    *awb_max_ratio = agw_p->led_bg_ratio;

  *awb_max_ratio *= 1.2;
  *awb_min_ratio *= 0.8;
  /* the constraint on reciprocal symmetry is removed here
   * to increase grid resolution */
  if (*awb_max_ratio < (1.0 / *awb_min_ratio))
    *awb_max_ratio = 1.0 / *awb_min_ratio;
  else if (*awb_min_ratio > (1.0 / *awb_max_ratio))
    *awb_min_ratio = 1.0 / *awb_max_ratio;
} /* awb_set_min_max_ratios */

/*===========================================================================
 * FUNCTION    - awb_setup_lighting_condition -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_setup_lighting_condition(float r1, float r2, float b1, float b2,
  int *param1, int (*param2)[2])
{
  float m;
  int index;
  if (fabs((double)(r1 - r2)) >= fabs((double)(b1 - b2))) {
    if ((r1 - r2) == 0)  /* divide by 0 protection */
      m = 0;
    else
      m = (b1 - b2) / (r1 - r2);
    if (r2 > r1) { /* if (r2<r1), calibration is wrong. */
      *param1 = (int) (r2 - r1) + 1;
      for (index = (int) r1; index <= (int) r2; index++) {
        param2[index - (int)r1][0] = index;
        param2[index - (int)r1][1] = (int)(m* ((float)index - r1) + b1 + 0.5);
      }
    } else {
      *param1 = (int) (r1 - r2) + 1;
      for (index = (int) r2; index <= (int) r1; index++) {
        param2[(*param1 - 1) - (index - (int) r2)][0] = index;
        param2[(*param1 - 1) - (index - (int) r2)][1] =
          (int) (m * ((float) index - r2) + b2 + 0.5);
      }
    }
  } else {
    if ((b1 - b2) == 0)  /* divide by 0 protection */
      m = 0;
    else
      m = (r1 - r2) / (b1 - b2);
    if (b2 > b1) { /* if (b2>b1) calibration is wrong */
      *param1 = (int) (b2 - b1) + 1;
      for (index = (int) b1; index <=( int) b2; index++) {
        param2[index - (int) b1][0] =
          (int) (m * ((float) index - b1) + r1 + 0.5);
        param2[index - (int) b1][1] = index;
      }
    } else {
      *param1 = (int) (b1 - b2) + 1;
      for (index = (int) b2; index <= (int) b1; index++) {
        param2[(*param1 - 1) - (index - (int) b2)][0] =
          (int)(m *((float) index - b2) + r2 + 0.5);
        param2[(*param1 - 1) - (index - (int) b2)][1] = index;
      }
    }
  }
} /* awb_setup_lighting_condition */

/*===========================================================================
 * FUNCTION    - awb_init_agw_algorithm -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_init_agw_algorithm (stats_proc_t *sproc,awb_t *awb)
{
  int   index, i, n;
  float delta;
  int   rgx, bgx;
  float r1, r2, b1, b2;
  float awb_min_ratio = 1.0, awb_max_ratio = 1.0;
  int   awb_min_ratio_x = 1, awb_max_ratio_x = 1;
  float x1, y1, x2, y2, x3, y3, m1, m2, xp, yp, vx, vy;
  int   kx = 0, ky = 0, dx = 0, dy = 0;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  chromatix_parms_type *cptr = sproc->input.chromatix;

  /*Extreme stats configuration*/
  float d65ref_bg =
    awb->agw.blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT];
  float d50ref_bg =
    awb->agw.blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];
  float greenoffsetbg = (float)awb->agw.green_offset_bg;
  float href_bg = awb->agw.blue_gain_table[AGW_AWB_HORIZON];
  float cwref_rg =
    awb->agw.red_gain_table[AGW_AWB_INDOOR_COLD_FLO];
  float d50ref_rg =
    awb->agw.red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];
  float greenoffsetrg = (float)awb->agw.green_offset_rg;
  float d65ref_rg =
    awb->agw.red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT];
  float aref_bg =
    awb->agw.blue_gain_table[AGW_AWB_INDOOR_INCANDESCENT];
  float AWBExtremeBGRatioThresh = cptr->awb_extreme_BG_ratio_threshold;
  float AWBExtremeRGRatioThresh = cptr->awb_extreme_RG_ratio_threshold;


  agw_p->indoor_F_WB_locked = 0;

  /* rest the indoor flurescent lock clearing out past history */
  for (i = 0; i < AWB_MAX_HISTORY; i++) {
    agw_p->awb_history[i].rg_ratio = 0.0;
    agw_p->awb_history[i].bg_ratio = 0.0;
    agw_p->awb_history[i].decision = AGW_AWB_INVALID_LIGHT;
    agw_p->awb_history[i].exp_index = -1;
    agw_p->awb_history[i].replaced = 0;
    agw_p->awb_history[i].is_white = -1;
  }
  agw_p->awb_history_count = 0;
  agw_p->awb_history_next_pos = 0;

  /* Initialize AEC History */
  agw_p->aec_history_count = 0;
  agw_p->aec_history_next_pos = 0;
  for (i = 0; i < AWB_AEC_MAX_HISTORY; i++) {
    agw_p->aec_history[i].exp_index = -1;
    agw_p->aec_history[i].frame_luma = 0;
  }

  /* 3A 1.5 AWB Self-Calibration */
  CDBG_AWB("awb_self_cal_enable %d\n", cptr->awb_self_cal_enable);
  if (cptr->awb_self_cal_enable)
    awb_self_cal_data_init(sproc, awb);

  n = (AGW_NUMBER_GRID_POINT - 1) / 2;
  awb_set_min_max_ratios(awb,&awb_min_ratio, &awb_max_ratio);

  /* prepare fixed point min/max ratio */
  awb_min_ratio_x = (int) (awb_min_ratio * (1 << RGBG_GRID_Q_NUM));
  awb_max_ratio_x = (int) (awb_max_ratio * (1 << RGBG_GRID_Q_NUM));

  /* for region < 1 */
  delta = (1.0 - awb_min_ratio) / ((float) n + 0.5);
  agw_p->rgbg_grid[0] = awb_min_ratio;
  agw_p->rgbg_grid_x[0] = (awb_min_ratio * (1 << RGBG_GRID_Q_NUM));

  CDBG_AWB("awb_algo_init_agw_algorithm: d65ref_bg= %f\n", d65ref_bg);
  CDBG_AWB("awb_algo_init_agw_algorithm: d50ref_bg= %f\n", d50ref_bg);
  CDBG_AWB("awb_algo_init_agw_algorithm: greenoffsetbg= %f\n", greenoffsetbg);
  CDBG_AWB("awb_algo_init_agw_algorithm: href_bg= %f\n", href_bg);
  CDBG_AWB("awb_algo_init_agw_algorithm: cwref_rg= %f\n", cwref_rg);
  CDBG_AWB("awb_algo_init_agw_algorithm: d50ref_rg= %f\n", d50ref_rg);
  CDBG_AWB("awb_algo_init_agw_algorithm: greenoffsetrg= %f\n", greenoffsetrg);
  CDBG_AWB("awb_algo_init_agw_algorithm: d65ref_rg= %f\n", d65ref_rg);
  CDBG_AWB("awb_algo_init_agw_algorithm: aref_bg= %f\n", aref_bg);
  CDBG_AWB("awb_algo_init_agw_algorithm: AWBExtremeBGRatioThresh= %f\n",
    AWBExtremeBGRatioThresh);
  CDBG_AWB("awb_algo_init_agw_algorithm: AWBExtremeRGRatioThresh= %f\n",
    AWBExtremeRGRatioThresh);
  CDBG_AWB("awb_algo_init_agw_algorithm: delta= %f\n", delta);

  sproc->share.awb_ext.estats.t1 =
    (uint32_t)(((0.5 * (d65ref_bg + d50ref_bg)) + greenoffsetbg * delta) * 64);
  sproc->share.awb_ext.estats.t2 =
    (uint32_t)((href_bg + greenoffsetbg * delta) * 64);
  sproc->share.awb_ext.estats.t3 =
    (uint32_t)((STATS_PROC_MIN(cwref_rg, d50ref_rg) + greenoffsetrg * delta) * 64);
  sproc->share.awb_ext.estats.mg =
    (uint32_t)(fabs((d65ref_bg - d50ref_bg) / (d65ref_rg - d50ref_rg) * 64));
  sproc->share.awb_ext.estats.t4 = (uint32_t)(((aref_bg + greenoffsetbg *
    delta) + (float)(sproc->share.awb_ext.estats.mg) / 64 *
    (STATS_PROC_MIN(cwref_rg, d50ref_rg) + greenoffsetrg * delta)) * 64);
  sproc->share.awb_ext.estats.t5 = (uint32_t)(AWBExtremeBGRatioThresh * 64.0);
  sproc->share.awb_ext.estats.t6 = (uint32_t)(AWBExtremeRGRatioThresh * 64.0);

  CDBG_AWB("%s:ESTATS t1 %d, t2 %d, t3 %d, t4 %d, t5 %d, t6 %d, mg %d",
    __func__, sproc->share.awb_ext.estats.t1,
    sproc->share.awb_ext.estats.t2, sproc->share.awb_ext.estats.t3,
    sproc->share.awb_ext.estats.t4, sproc->share.awb_ext.estats.t5,
    sproc->share.awb_ext.estats.t6, sproc->share.awb_ext.estats.mg);

  for (index = 1; index <= n; index++) {
    agw_p->rgbg_grid[index] = agw_p->rgbg_grid[index - 1] + delta;
    agw_p->rgbg_grid_x[index] =
      (int) (agw_p->rgbg_grid[index] * (1 << RGBG_GRID_Q_NUM));
  }
  /* for region > 1 */
  delta = (awb_max_ratio - 1.0) / ((float) n + 0.5);

  for (index = n + 1; index < AGW_NUMBER_GRID_POINT; index++) {
    agw_p->rgbg_grid[index] = agw_p->rgbg_grid[index - 1] + delta;
    agw_p->rgbg_grid_x[index] =
      (int) (agw_p->rgbg_grid[index] * (1 << RGBG_GRID_Q_NUM));
  }

  agw_p->rgbg_grid[AGW_NUMBER_GRID_POINT] = awb_max_ratio;
  agw_p->rgbg_grid_x[AGW_NUMBER_GRID_POINT] =
    (int) (awb_max_ratio * (1 << RGBG_GRID_Q_NUM));

  awb_set_grid_index(awb, awb_min_ratio_x, awb_max_ratio_x);

  awb_util_convert_to_grid(awb, agw_p->led_rg_ratio_x,
    agw_p->led_bg_ratio_x, &agw_p->led_rg_grid, & agw_p->led_bg_grid);

  /* generate day light line 1 (between shade & D65) */
  r1 = (float) agw_p->rg_grid[AGW_AWB_OUTDOOR_CLOUDY];
  b1 = (float) agw_p->bg_grid[AGW_AWB_OUTDOOR_CLOUDY];
  r2 = (float) agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
  b2 = (float) agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];

  /* make sure the line goes from D75 to D65 */
  awb_setup_lighting_condition(r1, r2, b1, b2, &(agw_p->n_day1),
    agw_p->day_line_1);

  if ((agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] ==
    agw_p->rg_grid[AGW_AWB_OUTDOOR_CLOUDY]) &&
    (agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] ==
    agw_p->bg_grid[AGW_AWB_OUTDOOR_CLOUDY])) {
    agw_p->n_day1 = 1;
    agw_p->day_line_1[0][0] = agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
    agw_p->day_line_1[0][1] = agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
  }
  /* generate day light line 2 (between D65 & D50) */
  r1 = agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
  b1 = agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
  r2 = agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1];
  b2 = agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1];

  /* from D65 to D50 */
  awb_setup_lighting_condition(r1, r2, b1, b2, &(agw_p->n_day2),
    agw_p->day_line_2);

  if ((agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] ==
    agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]) &&
    (agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] ==
    agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1])) {

    agw_p->n_day2 = 1;
    agw_p->day_line_2[0][0] = agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
    agw_p->day_line_2[0][1] = agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
  }
  /* add a fluorescent line from CW to TL */
  r1 = (float) agw_p->rg_grid[AGW_AWB_INDOOR_COLD_FLO];
  b1 = (float) agw_p->bg_grid[AGW_AWB_INDOOR_COLD_FLO];
  r2 = (float) agw_p->rg_grid[AGW_AWB_INDOOR_WARM_FLO];
  b2=  (float) agw_p->bg_grid[AGW_AWB_INDOOR_WARM_FLO];

  /* make sure the line goes from CW to TL84 */
  awb_setup_lighting_condition(r1, r2, b1, b2, &(agw_p->n_fline),
    agw_p->Fline);

  /* add a fluorescent line from D50 to (CW+TL84)/2 , 3A 1.4 change */
  r1 = (float) 0.5 * (agw_p->rg_grid[AGW_AWB_INDOOR_WARM_FLO] +
    agw_p->rg_grid[AGW_AWB_INDOOR_COLD_FLO]);
  b1 = (float) 0.5 * (agw_p->bg_grid[AGW_AWB_INDOOR_WARM_FLO] +
    agw_p->bg_grid[AGW_AWB_INDOOR_COLD_FLO]);
  r2 = (float) agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1];
  b2 = (float) agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1];
  /* make sure the line goes from D50 to fluorescent */
  awb_setup_lighting_condition(r1, r2, b1, b2, &(agw_p->n_day_f_line),
    agw_p->Day_F_line);

  /* A line 1: from 3200K to A this point (3200K) is estimated by TL and A */
  i = ((agw_p->rg_grid[AGW_AWB_INDOOR_WARM_FLO] + 2 *
    agw_p->rg_grid[AGW_AWB_INDOOR_INCANDESCENT]) / 3);
  r1 = (float) i;
  i = (float) ((agw_p->bg_grid[AGW_AWB_INDOOR_WARM_FLO] +
    2 * agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT]) / 3);
  b1 = (float) i;
  r2 = (float) agw_p->rg_grid[AGW_AWB_INDOOR_INCANDESCENT];
  b2 = (float) agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT];

  /* make A line 1 from 3200 to A.*/
  awb_setup_lighting_condition(r1, r2, b1, b2, &(agw_p->n_aline1),
    agw_p->Aline1);

  /* A line 2:  from A to H */
  r1 = (float)agw_p->rg_grid[AGW_AWB_INDOOR_INCANDESCENT];
  b1 = (float)agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT];
  r2 = (float)agw_p->rg_grid[AGW_AWB_HORIZON];
  b2 = (float)agw_p->bg_grid[AGW_AWB_HORIZON];
  /* make it go from A to H */
  awb_setup_lighting_condition(r1, r2, b1, b2, &(agw_p->n_aline2),
    agw_p->Aline2);

  /* estimate the green line (fixed point) */
  agw_p->green_line_mx = (agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] -
    agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]) * (1 << GREEN_Q_NUM);

  n = (agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT] -
    agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1]);

  if (n == 0)
    n = 1;  /* make sure no divide-by-zero */

  agw_p->green_line_mx = (agw_p->green_line_mx) / n;
  /* fixed point slope with Q number = GREEN_Q_NUM */

  n = agw_p->green_offset_rg + STATS_PROC_MIN(
    agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT1],
    agw_p->rg_grid[AGW_AWB_INDOOR_COLD_FLO]);

  agw_p->green_line_bx = agw_p->bg_grid[AGW_AWB_INDOOR_INCANDESCENT] +
    agw_p->green_offset_bg - (agw_p->green_line_mx * n) / (1 << GREEN_Q_NUM);

  /* point1 (x1,y1) is D65 */
  x1 = agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT];
  y1 = agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT];

  /* point2 (x2,y2) is D50 */
  x2 = agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];
  y2 = agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];

  /* point3 (x3,y3) is the high noon RG and BG ratio */
  x3 = agw_p->red_gain_table[AGW_AWB_OUTDOOR_NOON];
  y3 = agw_p->blue_gain_table[AGW_AWB_OUTDOOR_NOON];

  m1 = (y2 - y1) / (x2 - x1);
  b1 = y2 - m1 * x2;
  m2 = -1.0 / m1;
  b2 = y3 - m2 * x3;
  /* (xp,yp) is the intersection point of line 1 and line 2 */
  xp = (b2 - b1) / (m1 - m2);
  yp = m1 * xp + b1;

  vx = (x3 - xp); /* (vx,vy) is vector pointing from (xp,yp) to high noon) */
  vy = (y3 - yp);

  agw_p->shifted_d50_rg = x2 + vx;
  agw_p->shifted_d50_bg = y2 + vy;

  CDBG_AWB("d65_rg=%f, d65_bg =%f\n", x1, y1);
  CDBG_AWB("d50_rg=%f, d50_bg =%f\n", x2,y2);
  CDBG_AWB("noon_rg=%f, noon_bg =%f\n", x3, y3);
  CDBG_AWB("xp=%f, yp =%f\n", xp, yp);
  CDBG_AWB("vx=%f, vy =%f\n", vx, vy);
  CDBG_AWB("shifted d50_rg=%f, shifted d50_bg =%f\n", agw_p->
    shifted_d50_rg, agw_p->shifted_d50_bg);

  /* D65 shifted = (x1,y1) in floating point */
  x1 = agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT] + vx;
  y1 = agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT] + vy;
  CDBG_AWB("shifted D65 (float): rg=%f, bg =%f\n", x1, y1);
  /* shifted D65 in fixed point */
  rgx = (int) (x1 * (1 << RGBG_GRID_Q_NUM));
  bgx = (int) (y1 * (1 << RGBG_GRID_Q_NUM));

  for (index = 0; index < AGW_NUMBER_GRID_POINT; index++) {
    if (((rgx - agw_p->rgbg_grid_x[index]) *
      (rgx  - agw_p->rgbg_grid_x[index + 1])) <= 0) {
      kx = index;
      break;
    }
  }
  for (index = 0; index < AGW_NUMBER_GRID_POINT; index++) {
    if (((bgx - agw_p->rgbg_grid_x[index]) *
      (bgx - agw_p->rgbg_grid_x[index + 1])) <= 0) {
      ky = index;
      break;
    }
  }
  CDBG_AWB("shifted D65 (grid): rg=%d, bg =%d\n", kx,ky);
  dx = kx - agw_p->rg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];
  dy = ky - agw_p->bg_grid[AGW_AWB_OUTDOOR_SUNLIGHT];

  CDBG_AWB("Delta (grid) for D65 is: dx=%d, dy =%d\n", dx,dy);

  agw_p->n_day3=agw_p->n_day2;
  for (i = 0; i < agw_p->n_day3 && i < AGW_NUMBER_GRID_POINT; i++) {
    agw_p->day3_rg[i] = dx + agw_p->day_line_2[i][0];
    agw_p->day3_bg[i] = dy + agw_p->day_line_2[i][1];
  }

  agw_p->led_off_last_rg =
    agw_p->red_gain_table[AGW_AWB_INDOOR_WARM_FLO];
  agw_p->led_off_last_bg =
    agw_p->blue_gain_table[AGW_AWB_INDOOR_WARM_FLO];
  agw_p->led_on_last_rg = agw_p->led_rg_ratio;
  agw_p->led_on_last_bg = agw_p->led_bg_ratio;
  agw_p->led_fired_for_this_frame = FALSE;

  for (i = 0; i < agw_p->n_day3 && i < AGW_NUMBER_GRID_POINT; i++)
    CDBG_AWB("Day line 3=(%d, %d)\n", agw_p->day3_rg[i],agw_p->day3_bg[i]);

  /* print the line parameters for debug purpose */
  CDBG_AWB("------------ AWB init debug -----------\n");
  for (i = 0; i < agw_p->n_day1 && i < AGW_NUMBER_GRID_POINT; i++)
    CDBG_AWB("Day line 1=(%d, %d)\n", agw_p->day_line_1[i][0],
      agw_p->day_line_1[i][1]);

  for (i = 0; i < agw_p->n_day2 && i < AGW_NUMBER_GRID_POINT; i++)
    CDBG_AWB("Day line 2=(%d, %d)\n", agw_p->day_line_2[i][0],
      agw_p->day_line_2[i][1]);

  for (i = 0; i < agw_p->n_day3 && i < AGW_NUMBER_GRID_POINT; i++)
    CDBG_AWB("Day line 3=(%d, %d)\n", agw_p->day3_rg[i], agw_p->day3_bg[i]);

  for (i = 0; i < agw_p->n_fline && i < AGW_F_A_LINE_NUM_POINTS; i++)
    CDBG_AWB("F line =(%d, %d)\n", agw_p->Fline[i][0], agw_p->Fline[i][1]);

  for (i = 0; i < agw_p->n_day_f_line && i < AGW_F_A_LINE_NUM_POINTS; i++)
    CDBG_AWB("Day_F line =(%d, %d)\n", agw_p->Day_F_line[i][0],
      agw_p->Day_F_line[i][1]);

  for (i = 0; i < agw_p->n_aline1 && i < AGW_F_A_LINE_NUM_POINTS; i++)
    CDBG_AWB("A line 1=(%d, %d)\n", agw_p->Aline1[i][0], agw_p->Aline1[i][1]);

  for (i = 0; i < agw_p->n_aline2 && i < AGW_F_A_LINE_NUM_POINTS; i++)
    CDBG_AWB("A line 2=(%d, %d)\n", agw_p->Aline2[i][0], agw_p->Aline2[i][1]);

  CDBG_AWB("------------ END AWB init debug -----------\n");
} /* awb_init_agw_algorithm */

/*===========================================================================
 * FUNCTION    - awb_compute_color_temp -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float awb_compute_color_temp(float ratio, float gain_table[],
  chromatix_awb_light_type illum1, chromatix_awb_light_type illum2)
{
  int col_temps [7] = {6500, 7500, 2850, 4180, 4100, 2300, 5000};
  float w1, w2;
  w1 = fabs(ratio - gain_table[illum1]);
  w2 = fabs(gain_table[illum2] - ratio);
  CDBG_AWB("### w1=%f, w2=%f\n", w1, w2);
  return((w1 * col_temps[illum2] + w2 * col_temps[illum1]) / (w1 + w2));
} /* awb_compute_color_temp */

/*===========================================================================
 * FUNCTION    - awb_interpolate_gain_adj -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_interpolate_gain_adj(awb_t *awb, uint32_t cct,
  float red_gain_adj[], float blue_gain_adj[], chromatix_awb_light_type illum1,
  chromatix_awb_light_type illum2)
{
  float mired;
  float mired1;
  float mired2;
  float w1;
  float w2;
  uint32_t cct1;
  uint32_t cct2;
  uint32_t col_temps[7] = {6500, 7500, 2850, 4180, 4100, 2300, 5000};

  cct1 = (uint32_t)col_temps[illum1];
  cct2 = (uint32_t)col_temps[illum2];

  mired1 = 1000000 / cct1;
  mired = 1000000 / cct;
  mired2 = 1000000 / cct2;
  w1 = mired1 - mired;
  w2 = mired - mired2;

  awb->red_gain_adjust = ((w1 * red_gain_adj[illum2]) +
    (w2 * red_gain_adj[illum1])) / (w1 + w2);
  awb->blue_gain_adjust = ((w1 * blue_gain_adj[illum2]) +
    (w2 * blue_gain_adj[illum1])) / (w1 + w2);
}

/*===========================================================================
 * FUNCTION    - awb_compute_gain_adj -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_compute_gain_adj(awb_t *awb, float red_gain_adj[],
  float blue_gain_adj[], uint32_t cct)
{
  /* Linear interpolation for gain adjustment */
  if (cct >= 7500) {
    /* Use D75 gain adjust */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_OUTDOOR_CLOUDY];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_OUTDOOR_CLOUDY];
  } else if (cct > 6500) {
    /* Interpolate between D65 and D75 gain adjust */
    awb_interpolate_gain_adj(awb, cct, red_gain_adj, blue_gain_adj,
      AGW_AWB_OUTDOOR_SUNLIGHT, AGW_AWB_OUTDOOR_CLOUDY);
  } else if (cct == 6500) {
    /* Use D65 Gain Adj */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT];
  } else if (cct > 5000) {
    /* Interpolate between D50 and D65 gain adjust */
    awb_interpolate_gain_adj(awb, cct, red_gain_adj, blue_gain_adj,
      AGW_AWB_OUTDOOR_SUNLIGHT1, AGW_AWB_OUTDOOR_SUNLIGHT);
  } else if (cct == 5000) {
    /* Use D50 Gain Adj */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT1];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_OUTDOOR_SUNLIGHT1];
  } else if (cct >= 4300) {
    /* Interpolate between D50 and CW gain adjust */
    awb_interpolate_gain_adj(awb, cct, red_gain_adj, blue_gain_adj,
      AGW_AWB_INDOOR_COLD_FLO, AGW_AWB_OUTDOOR_SUNLIGHT1);
  } else if (cct >= 4180) {
    /* Use CW gain adjust */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_INDOOR_COLD_FLO];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_INDOOR_COLD_FLO];
  } else if (cct >= 4100) {
    /* Interpolate between CW and TL84 gain adjust */
    awb_interpolate_gain_adj(awb, cct, red_gain_adj, blue_gain_adj,
      AGW_AWB_INDOOR_WARM_FLO, AGW_AWB_INDOOR_COLD_FLO);
  } else if (cct >= 3800) {
    /* Use TL84 gain adjust */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_INDOOR_WARM_FLO];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_INDOOR_WARM_FLO];
  } else if (cct > 2850) {
    /* Interpolate between A and TL84 gain adjust */
    awb_interpolate_gain_adj(awb, cct, red_gain_adj, blue_gain_adj,
      AGW_AWB_INDOOR_INCANDESCENT, AGW_AWB_INDOOR_WARM_FLO);
  } else if (cct == 2850) {
    /* Use A gain adjust */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_INDOOR_INCANDESCENT];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_INDOOR_INCANDESCENT];
  } else if (cct > 2300) {
    /* Interpolate between A and H gain adjust */
    awb_interpolate_gain_adj(awb, cct, red_gain_adj, blue_gain_adj,
      AGW_AWB_HORIZON, AGW_AWB_INDOOR_WARM_FLO);
  } else {
    /* Use H gain adjust */
    awb->red_gain_adjust = red_gain_adj[AGW_AWB_HORIZON];
    awb->blue_gain_adjust = blue_gain_adj[AGW_AWB_HORIZON];
  }
} /* awb_compute_gain_adj */

/*===========================================================================
 * FUNCTION    - awb_estimate_color_temperature -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint32_t awb_estimate_color_temperature(awb_t *awb, float redGain,
    float greenGain, float blueGain)
{
  float rg_ratio;
  float bg_ratio;
  float cct = 0;
  float cct1 = 0;
  float cct2 = 10000;

  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  if (redGain < 0.01 || blueGain < 0.01) {
    CDBG_AWB("%s: Error! Red or blue gain too small. Return.\n", __func__);
    return 0;
  }

  /* Use R/G ratio to decide CCT1 */
  rg_ratio = greenGain / redGain;

  if (rg_ratio <= agw_p->red_gain_table[AGW_AWB_OUTDOOR_CLOUDY]) {
    cct1 = 7500;
    CDBG_AWB("%s: cct1 = %f, RG <= D75\n", __func__, cct1);
  } else if (rg_ratio <= agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT]) {
    cct1 = awb_compute_color_temp(rg_ratio, agw_p->red_gain_table,
        AGW_AWB_OUTDOOR_CLOUDY, AGW_AWB_OUTDOOR_SUNLIGHT);
    CDBG_AWB("%s: cct1 = %f, RG between D75 and D65\n", __func__, cct1);
  } else if (rg_ratio <= agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1]) {
    cct1 = awb_compute_color_temp(rg_ratio, agw_p->red_gain_table,
        AGW_AWB_OUTDOOR_SUNLIGHT, AGW_AWB_OUTDOOR_SUNLIGHT1);
    CDBG_AWB("%s: cct1 = %f, RG between D65 and D50\n", __func__, cct1);
  } else if (rg_ratio <= agw_p->red_gain_table[AGW_AWB_INDOOR_WARM_FLO]) {
    cct1 = awb_compute_color_temp(rg_ratio, agw_p->red_gain_table,
        AGW_AWB_OUTDOOR_SUNLIGHT1, AGW_AWB_INDOOR_WARM_FLO);
    CDBG_AWB("%s: cct1 = %f, RG between D50 and TL84\n", __func__, cct1);
  } else if (rg_ratio <= agw_p->red_gain_table[AGW_AWB_INDOOR_INCANDESCENT]) {
    cct1 = awb_compute_color_temp(rg_ratio, agw_p->red_gain_table,
        AGW_AWB_INDOOR_WARM_FLO, AGW_AWB_INDOOR_INCANDESCENT);
    CDBG_AWB("%s: cct1 = %f, RG between TL84 and A\n", __func__, cct1);
  } else if (rg_ratio <= agw_p->red_gain_table[AGW_AWB_HORIZON]) {
    cct1 = awb_compute_color_temp(rg_ratio, agw_p->red_gain_table,
        AGW_AWB_INDOOR_INCANDESCENT, AGW_AWB_HORIZON);
    CDBG_AWB("%s: cct1 = %f, RG between A and Horizon\n", __func__, cct1);
  } else {
    cct1 = 2300;
    CDBG_AWB("%s: cct1 = %f, RG > Horizon\n", __func__, cct1);
  }

  /* Use B/G to estimate CCT2 */
  bg_ratio = greenGain / blueGain;

  if (bg_ratio >= agw_p->blue_gain_table[AGW_AWB_OUTDOOR_CLOUDY]) {
    cct2 = 7500;
    CDBG_AWB("%s: cct2 = %f, BG >= D75\n", __func__, cct2);
  } else if (bg_ratio >= agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT]) {
    cct2 = awb_compute_color_temp(bg_ratio, agw_p->blue_gain_table,
        AGW_AWB_OUTDOOR_CLOUDY, AGW_AWB_OUTDOOR_SUNLIGHT);
    CDBG_AWB("%s: cct2 = %f, BG between D75 and D65\n", __func__, cct2);
  } else if (bg_ratio >= agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1]) {
    cct2 = awb_compute_color_temp(bg_ratio, agw_p->blue_gain_table,
        AGW_AWB_OUTDOOR_SUNLIGHT, AGW_AWB_OUTDOOR_SUNLIGHT1);
    CDBG_AWB("%s: cct2 = %f, BG between D65 and D50\n", __func__, cct2);
  } else if (bg_ratio >= agw_p->blue_gain_table[AGW_AWB_INDOOR_COLD_FLO]) {
    cct2 = awb_compute_color_temp(bg_ratio, agw_p->blue_gain_table,
        AGW_AWB_OUTDOOR_SUNLIGHT1, AGW_AWB_INDOOR_COLD_FLO);
    CDBG_AWB("%s: cct2 = %f, BG between D50 and TL84\n", __func__, cct2);
  } else if (bg_ratio >= agw_p->blue_gain_table[AGW_AWB_INDOOR_INCANDESCENT]) {
    cct2 = awb_compute_color_temp(bg_ratio, agw_p->blue_gain_table,
        AGW_AWB_INDOOR_COLD_FLO, AGW_AWB_INDOOR_INCANDESCENT);
    CDBG_AWB("%s: cct2 = %f, BG between TL84 and A\n", __func__, cct2);
  } else if (bg_ratio >= agw_p->blue_gain_table[AGW_AWB_HORIZON]) {
    cct2 = awb_compute_color_temp(bg_ratio, agw_p->blue_gain_table,
        AGW_AWB_INDOOR_INCANDESCENT, AGW_AWB_HORIZON);
    CDBG_AWB("%s: cct2 = %f, BG between A and Horizon\n", __func__, cct2);
  } else {
    cct2 = 2300;
    CDBG_AWB("%s: cct2 = %f, BG < Horizon\n", __func__, cct2);
  }

  /* Estimate CCT Output */
  cct = 0.5 * (cct1 + cct2);

  /* Special cases for some regions */
  if ((rg_ratio < agw_p->red_gain_table[AGW_AWB_INDOOR_WARM_FLO])
      && (bg_ratio <= agw_p->blue_gain_table[AGW_AWB_INDOOR_COLD_FLO])) {
    /* Too green. Fix CCT to 4000 */
    cct = 4000;
    CDBG_AWB("%s: RG < min(D50,CW), BG < CW (too green) use cct = %f\n",
        __func__, cct);
  } else if (rg_ratio <
      STATS_PROC_MIN(agw_p->red_gain_table[AGW_AWB_INDOOR_COLD_FLO],
      agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1])) {
    /* RG < min(D50,CW, use CCT2 only to avoid sudden CCT jump */
    cct = cct2;
    CDBG_AWB("%s: RG < min(D50,CW), use cct2, cct = %f\n", __func__, cct);
  } else if ((rg_ratio >= agw_p->red_gain_table[AGW_AWB_INDOOR_WARM_FLO])
    && (rg_ratio < (0.5 * (agw_p->red_gain_table[AGW_AWB_INDOOR_INCANDESCENT]
    + agw_p->red_gain_table[AGW_AWB_INDOOR_WARM_FLO])))
    && (cct2 < 3800)) {
    /* RG between TL and half (TL,A), BG < 3800, limit cct2 to 3800 */
    cct = 0.5 * (cct1 + 3800);
    CDBG_AWB("%s: RG between TL and half (TL,A), BG < 3800, set cct2 to 3800,"
        " cct = %f\n", __func__, cct);
  } else if ((rg_ratio >= (0.5 *
      (agw_p->red_gain_table[AGW_AWB_INDOOR_INCANDESCENT]
      + agw_p->red_gain_table[AGW_AWB_INDOOR_WARM_FLO])))
      && (rg_ratio < agw_p->red_gain_table[AGW_AWB_INDOOR_INCANDESCENT])) {
    /* RG between half(TL, A), Limit cct2 to 2850 and 4000 */
    if (cct2 < 2850) {
      cct = 0.5 * (cct1 + 2850);
      CDBG_AWB("%s: RG between half(TL,A) and A, BG < 2850, set cct2 to 2850,"
          " cct = %f\n", __func__, cct);
    } else if (cct2 > 4000) {
      cct = 0.5 * (cct1 + 4000);
      CDBG_AWB("%s: RG between half(TL,A) and A, cct2 > 4000, set cct2 to 4000,"
          " cct = %f\n", __func__, cct);
    }
  } else if (rg_ratio >= agw_p->red_gain_table[AGW_AWB_INDOOR_INCANDESCENT]) {
    if (cct2 < 2300) {
      cct = 0.5 * (cct1 + 2300);
      CDBG_AWB("%s: RG between A and H, cct2 < 2300, set cct2 to 2300,"
          " cct = %f\n", __func__, cct);
    } else if (cct2 > 3200) {
      cct = 0.5 * (cct1 + 3200);
      CDBG_AWB("%s: RG between A and H, cct2 > 3200, set cct2 to 3200,"
          " cct = %f\n", __func__, cct);
    }
  }

  return (uint32_t)cct;
} /* awb_estimate_color_temperature */

/*===========================================================================
 * FUNCTION    - awb_scale_gains -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_scale_gains(float *r_gain, float *g_gain, float *b_gain)
{
  float min_gain;
  float scale_factor;

  /* Find minimum gain between R and B*/
  min_gain = (*r_gain < *b_gain) ? (*r_gain) : (*b_gain);

  if (min_gain < 1.0) {
    /* Upscale all gains so minimum gain is 1.0 */
    scale_factor = 1 / min_gain;
    *r_gain *= scale_factor;
    *g_gain *= scale_factor;
    *b_gain *= scale_factor;
  }
}

/*===========================================================================
 * FUNCTION    - awb_update_wb_gain_values -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_update_wb_gain_values(stats_proc_t *sproc, awb_t *awb,
  float redGain, float greenGain, float blueGain)
{
  float weightedRedGain, weightedBlueGain, weightedGreenGain;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  CDBG_AWB("temporal filter receives: r_gain=%f, g_gain=%f, b_gain=%f\n",
    redGain,greenGain,blueGain);
  /* Enforce user-configurable min and max WB gains */
  /* These gains are floats in Q0 format            */
  awb_cap_gain(&redGain,   cptr->awb_max_gains.r_gain,
      cptr->awb_min_gains.r_gain);
  awb_cap_gain(&greenGain, cptr->awb_max_gains.g_gain,
      cptr->awb_min_gains.g_gain);
  awb_cap_gain(&blueGain,  cptr->awb_max_gains.b_gain,
      cptr->awb_min_gains.b_gain);

  if ((redGain != awb->last_wb.r_gain) || (greenGain != awb->
    last_wb.g_gain) || (blueGain != awb->last_wb.b_gain)) {
    float aggressiveness = 0;

    if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
      /* Use 1/8 of the new WB gain to change the old */
      if (cptr->awb_aggressiveness == STATS_PROC_LOW_AGGRESSIVENESS ||
        sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER) {
        /* VIDEO MODE (always use low aggressiveness) */
        aggressiveness = AWB_LOW_AGGRESSIVENESS;
        if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER)
          aggressiveness = (aggressiveness / 2.0);
      } else if (cptr->awb_aggressiveness == STATS_PROC_MED_AGGRESSIVENESS)
        aggressiveness = AWB_MED_AGGRESSIVENESS;
      else /* STATS_PROC_HIGH_AGGRESSIVENESS */
        aggressiveness = AWB_HIGH_AGGRESSIVENESS;

      weightedRedGain  = (redGain * aggressiveness)   +
        ((1.0 - aggressiveness) * awb->last_wb.r_gain);
      weightedGreenGain= (greenGain * aggressiveness) +
        ((1.0 - aggressiveness) * awb->last_wb.g_gain);
      weightedBlueGain = (blueGain * aggressiveness)  +
        ((1.0 - aggressiveness) * awb->last_wb.b_gain);

      CDBG_AWB("temporal filter is run WITH smoothing: ");
    } else { /* LED is ON */
      weightedRedGain   = redGain;
      weightedGreenGain = greenGain;
      weightedBlueGain  = blueGain;
      CDBG_AWB("temporal filter is run WITHOUT smoothing: ");
    }
    CDBG_AWB("r_gain=%f,g_gain=%f,b_gain=%f\n",
      weightedRedGain, weightedGreenGain, weightedBlueGain);
    /* Set current to previous for next time.
       Do this BEFORE applying the color correction bias!! */
    awb->last_wb.r_gain = weightedRedGain;
    awb->last_wb.g_gain = weightedGreenGain;
    awb->last_wb.b_gain = weightedBlueGain;
    /* compute the color temperature */
    sproc->share.awb_ext.color_temp = awb_estimate_color_temperature(awb,
      weightedRedGain, weightedGreenGain, weightedBlueGain);
    CDBG_AWB("%s: estimated color temperature %d\n", __func__,
      sproc->share.awb_ext.color_temp);

    /* Compute Gain Adjust for preview */
    awb_compute_gain_adj(awb, agw_p->awb_r_adj_VF, agw_p->awb_b_adj_VF,
        sproc->share.awb_ext.color_temp);
    /* Apply gain adjusts for preview */
    weightedRedGain *= awb->red_gain_adjust;
    weightedBlueGain *= awb->blue_gain_adjust;

    /* Now apply bias to all gains to make up for
     * intentionally darker exposure ctrl setting */
    weightedBlueGain  *= awb->bst_blue_gain_adj;

    /* Upscale gains to ensure minimum gain is 1.0 */
    awb_scale_gains(&weightedRedGain, &weightedGreenGain, &weightedBlueGain);

    /* APPLY again to try and correct cropping errors */
    /* Enforce user-configurable min and max WB gains */
    /* These gains are floats in Q0 format            */
    awb_cap_gain(&weightedRedGain,   cptr->awb_max_gains.r_gain,
        cptr->awb_min_gains.r_gain);
    awb_cap_gain(&weightedGreenGain, cptr->awb_max_gains.g_gain,
        cptr->awb_min_gains.g_gain);
    awb_cap_gain(&weightedBlueGain,  cptr->awb_max_gains.b_gain,
        cptr->awb_min_gains.b_gain);

    /* Update CMD_VFE_WHITE_BALANCE_CONFIG */
    sproc->share.awb_ext.curr_gains.r_gain   = weightedRedGain;
    sproc->share.awb_ext.curr_gains.g_gain   = weightedGreenGain;
    sproc->share.awb_ext.curr_gains.b_gain   = weightedBlueGain;
  } else
    CDBG_AWB("temporal filter is not run: r_gain=%f, g_gain=%f, b_gain=%f\n",
      awb->last_wb.r_gain, awb->last_wb.g_gain,
      awb->last_wb.b_gain);
} /* awb_update_wb_gain_values */

/*===========================================================================
 * FUNCTION    - awb_set_current_wb -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_set_current_wb(stats_proc_t *sproc, awb_t *awb, uint32_t new_wb)
{
  int rc = 0;
  int update_hw = TRUE;
  float weightedRedGain = 1.0, weightedGreenGain = 1.0, weightedBlueGain = 1.0;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  sproc->share.awb_ext.rolloff_tbl = ROLLOFF_TL84_LIGHT;

  if (sproc->share.awb_ext.current_wb_type == new_wb && new_wb == CAMERA_WB_AUTO)
    update_hw = FALSE;
  else
    sproc->share.awb_ext.current_wb_type = new_wb;

  switch (sproc->share.awb_ext.current_wb_type) {
    case CAMERA_WB_AUTO:
      sproc->share.awb_ext.awb_update = TRUE;
      sproc->share.awb_ext.rolloff_tbl = ROLLOFF_TL84_LIGHT;

      weightedRedGain   = awb->last_wb.r_gain;
      weightedGreenGain = awb->last_wb.g_gain;
      weightedBlueGain  = awb->last_wb.b_gain;
      break;

    case CAMERA_WB_OFF: /* Maintain previous wb gains, Turn off auto WB */
      sproc->share.awb_ext.awb_update = FALSE;
      sproc->share.awb_ext.rolloff_tbl = ROLLOFF_TL84_LIGHT;

      weightedRedGain   = awb->last_wb.r_gain;
      weightedGreenGain = awb->last_wb.g_gain;
      weightedBlueGain  = awb->last_wb.b_gain;
      break;

    case CAMERA_WB_CLOUDY_DAYLIGHT:
      /* Note the use of "day light" color conversion for cloudy here */
      /* Turn off auto WB */
      sproc->share.awb_ext.awb_update= FALSE;
      sproc->share.awb_ext.rolloff_tbl = ROLLOFF_D65_LIGHT;

      weightedRedGain = cptr->chromatix_d65_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[0].red_gain_adj;
      weightedGreenGain = cptr->chromatix_d65_white_balance.g_gain;
      weightedBlueGain = cptr->chromatix_d65_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[0].blue_gain_adj;
      sproc->share.awb_ext.decision = 6;
      break;

    case CAMERA_WB_INCANDESCENT: /* a.k.a "A",  Turn off auto WB */
      sproc->share.awb_ext.awb_update = FALSE;
      sproc->share.awb_ext.rolloff_tbl = ROLLOFF_A_LIGHT;

      weightedRedGain = cptr->chromatix_incandescent_white_balance.
        r_gain * cptr->awb_reference_hw_rolloff[2].red_gain_adj;
      weightedGreenGain = cptr->chromatix_incandescent_white_balance.g_gain;
      weightedBlueGain = cptr->chromatix_incandescent_white_balance.
        b_gain * cptr->awb_reference_hw_rolloff[2].blue_gain_adj;

      sproc->share.awb_ext.decision = 2;
      break;

    case CAMERA_WB_FLUORESCENT: /* a.k.a "TL84",  Turn off auto WB */
      sproc->share.awb_ext.awb_update = FALSE;
      sproc->share.awb_ext.rolloff_tbl = ROLLOFF_TL84_LIGHT;

      weightedRedGain = cptr->chromatix_tl84_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[3].red_gain_adj;
      weightedGreenGain = cptr->chromatix_tl84_white_balance.g_gain;
      weightedBlueGain = cptr->chromatix_tl84_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[3].blue_gain_adj;
      sproc->share.awb_ext.decision = 3;
      break;
    case CAMERA_WB_DAYLIGHT: /* a.k.a "D65", Turn off auto WB */
      sproc->share.awb_ext.awb_update = FALSE;
      sproc->share.awb_ext.rolloff_tbl = ROLLOFF_D65_LIGHT;

      weightedRedGain = cptr->chromatix_d50_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[6].red_gain_adj;
      weightedGreenGain = cptr->chromatix_d50_white_balance.g_gain;
      weightedBlueGain = cptr->chromatix_d50_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[6].blue_gain_adj;
      sproc->share.awb_ext.decision = 0;
      break;

    case CAMERA_WB_CUSTOM:
    case CAMERA_WB_TWILIGHT:
    case CAMERA_WB_SHADE:
    default:
      rc = -1;
      break;
  }
  if (rc == 0 && update_hw) {
    awb->last_wb.r_gain = weightedRedGain;
    awb->last_wb.g_gain = weightedGreenGain;
    awb->last_wb.b_gain = weightedBlueGain;

    sproc->share.awb_ext.curr_gains.r_gain = weightedRedGain;
    sproc->share.awb_ext.curr_gains.g_gain = weightedGreenGain;
    sproc->share.awb_ext.curr_gains.b_gain = weightedBlueGain *
      awb->bst_blue_gain_adj;

    if (new_wb >= CAMERA_WB_MAX_PLUS_1 || new_wb <= CAMERA_WB_MIN_MINUS_1)
      rc = -1;
    else {
      sproc->share.awb_ext.current_wb_type = new_wb;
      rc = 0;
    }
  }
  return rc;
} /* awb_set_current_wb */

/*===========================================================================
FUNCTION      awb_update_roll_off_for_wb

DESCRIPTION  Updates the Roll-Off table parts of the VFE color conversion
             module based on the illuminant choice or sample decision.
===========================================================================*/
static void awb_update_roll_off_for_wb(stats_proc_t *sproc, awb_t *awb)
{
  int decision = sproc->share.awb_ext.decision;

  /* Compare Illuminant choice with previous choice */
  if (decision != awb->old_illuminant_choice) {
    awb->old_illuminant_choice_count = 1;
    awb->old_illuminant_choice = decision;
    return;
  } else
    awb->old_illuminant_choice_count++;

  if (awb->old_illuminant_choice_count != 6)
    return;

  /* Use daylight color conversion array */
  if ((decision == AGW_AWB_OUTDOOR_SUNLIGHT) || (decision == AGW_AWB_HYBRID)||
    (decision == AGW_AWB_OUTDOOR_SUNLIGHT1) || (decision == AGW_AWB_OUTDOOR_CLOUDY)) {
    sproc->share.awb_ext.rolloff_tbl = ROLLOFF_D65_LIGHT;
  } else if ((decision == AGW_AWB_INDOOR_WARM_FLO) ||/* flourescent color conv */
    (decision == AGW_AWB_INDOOR_CUSTOM_FLO) || (decision == AGW_AWB_INDOOR_COLD_FLO)) {
    sproc->share.awb_ext.rolloff_tbl = ROLLOFF_TL84_LIGHT;
  } else if ((decision == AGW_AWB_INDOOR_INCANDESCENT) ||
    (decision == AGW_AWB_HORIZON)) {  /* Use incandescent color conv */
    sproc->share.awb_ext.rolloff_tbl = ROLLOFF_A_LIGHT;
  }
} /* awb_update_roll_off_for_wb */

/*===========================================================================
 * FUNCTION    - awb_parse_stats -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_parse_stats(stats_proc_t *sproc, awb_t *awb)
{
  int y_ave_x, cb_ave_x, cr_ave_x, index, bin_idx = 0;
  unsigned long SY1, SCb, SCr, NSCb;

  awb_stats_convert_coeff_info_t *stats_coef = &(awb->awb_stats_conv_coef);
  awb_input_stats_type *pstats = &(awb->stats_ptr);
  isp_stats_t *statStruct = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  memset(&(sproc->share.stat_index_mapping), -1, (sizeof(int16_t)* 64));

  /* among the 64 or 256 bins of SCb, SCr, SY1, some are zeros.
   Skip the empty bins and work on the non-empty bins */
  for (index = 0; index < (int)sproc->input.mctl_info.numRegions;) {
    if (sproc->input.mctl_info.numRegions == 256) {
      /* Combine 16x16 regions spatially to generate 8x8 stats */
      SY1  = (statStruct->awb_op.SY1[index] + statStruct->awb_op.SY1[index + 1] +
        statStruct->awb_op.SY1[index + 16] + statStruct->awb_op.SY1[index + 16 + 1]);
      SCb  = (statStruct->awb_op.SCb[index] + statStruct->awb_op.SCb[index + 1] +
        statStruct->awb_op.SCb[index + 16] + statStruct->awb_op.SCb[index + 16 + 1]);
      SCr  = (statStruct->awb_op.SCr[index] + statStruct->awb_op.SCr[index + 1] +
        statStruct->awb_op.SCr[index + 16] + statStruct->awb_op.SCr[index + 16 + 1]);
      NSCb = (statStruct->awb_op.NSCb[index] + statStruct->awb_op.NSCb[index + 1] +
        statStruct->awb_op.NSCb[index + 16] + statStruct->awb_op.NSCb[index + 16 + 1]);
      index += 2; /* jump to the next even horizontal regions */
      if ((index & 0x0F) == 0x0) {
        index += 16; /* jump to the next even vertical regions */
      }
    } else {
      SY1  = statStruct->awb_op.SY1[index];
      SCb  = statStruct->awb_op.SCb[index];
      SCr  = statStruct->awb_op.SCr[index];
      NSCb = statStruct->awb_op.NSCb[index];
      index++;
    }

    if (NSCb != 0 && pstats->bin_cnt < 64) {
      /* convert sum of Y, Cb and Cr into ave Y, Cb, Cr, and also remove
       * the offset of 128 */
      y_ave_x  = ((SY1 << YCBCR_TO_RGB_Q_NUM) + (NSCb >> 1)) / NSCb;
      cb_ave_x = ((SCb << YCBCR_TO_RGB_Q_NUM) + (NSCb >> 1)) / NSCb -
        (YCBCR_STATS_OFFSET << YCBCR_TO_RGB_Q_NUM);
      cr_ave_x = ((SCr << YCBCR_TO_RGB_Q_NUM) + (NSCb >> 1)) / NSCb -
        (YCBCR_STATS_OFFSET << YCBCR_TO_RGB_Q_NUM);

      /* fixed point computation of R, G, B  */
      pstats->rx[awb->stats_ptr.bin_cnt] = (stats_coef->A11 *
        y_ave_x) - (stats_coef->A12 * cb_ave_x) +(stats_coef->A13 * cr_ave_x);

      pstats->rx[pstats->bin_cnt] = (pstats->rx[pstats->bin_cnt] +
        Q5_ROUNDOFF) >> YCBCR_TO_RGB_Q_NUM;
      if (pstats->rx[pstats->bin_cnt] < 0)
        CDBG_ERROR("ERROR!!! Invalid STATS RX\n");

      pstats->gx[pstats->bin_cnt] = (stats_coef->A21 * y_ave_x)  -
        (stats_coef->A22 * cb_ave_x) - (stats_coef->A23 * cr_ave_x);

      pstats->gx[pstats->bin_cnt] = (pstats->gx[pstats->bin_cnt] +
        Q5_ROUNDOFF) >> YCBCR_TO_RGB_Q_NUM;
      if (pstats->gx[pstats->bin_cnt] < 0)
        CDBG_ERROR("ERROR!!! Invalid STATS GX\n");

      pstats->bx[pstats->bin_cnt] = (stats_coef->A31 * y_ave_x) +
        (stats_coef->A32 * cb_ave_x) - (stats_coef->A33 * cr_ave_x);

      pstats->bx[pstats->bin_cnt]= (pstats->bx[pstats->bin_cnt] +
        Q5_ROUNDOFF) >> YCBCR_TO_RGB_Q_NUM;
      if (pstats->bx[pstats->bin_cnt] < 0)
        CDBG_ERROR("ERROR!!! Invalid STATS BX\n");

      sproc->share.stat_index_mapping[bin_idx] = pstats->bin_cnt;
      pstats->bin_cnt++;
    }
    bin_idx++;
  }
  if (sproc->input.mctl_info.numRegions == 256)
    pstats->region_cnt = (sproc->input.mctl_info.numRegions >> 2);
  else
    pstats->region_cnt = sproc->input.mctl_info.numRegions;
} /* awb_parse_stats */

/*===========================================================================
 * FUNCTION    - awb_interpolate_grey_world_ymin -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_interpolate_grey_world_ymin(stats_proc_t *sproc, awb_t *awb)
{
  int exp_index;
  int exp_index_low;
  int exp_index_high;
  float grey_y_min;
  chromatix_parms_type *cptr = sproc->input.chromatix;

  exp_index = sproc->share.prev_exp_index;
  exp_index_low = cptr->AWB_lowlight_LUT.lut_entry[4].lux_index;
  exp_index_high = cptr->AWB_lowlight_LUT.lut_entry[5].lux_index;

  if (exp_index_low >= exp_index_high)
    return;

  if (exp_index > exp_index_high) {
    grey_y_min = 0;
  } else if (exp_index >= exp_index_low) {
    grey_y_min = (float)(exp_index_high - exp_index)
        / (float)(exp_index_high - exp_index_low);
    grey_y_min *= cptr->wb_exp_stats_hw_rolloff[AWB_STATS_LOW_LIGHT].y_min;
  } else {
    return;
  }

  sproc->share.awb_ext.bounding_box.y_min = (unsigned char) grey_y_min;
}


/*===========================================================================
 * FUNCTION    - awb_select_bounding_box -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_select_bounding_box(stats_proc_t *sproc, awb_t *awb)
{
  int white_y_min;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);

  if (awb->current_awb_stat_config == AWB_STAT_REGULAR) {
    /* Do not toggle bounding box if LED is on */
    if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
      /* LED is OFF */
      white_y_min = (cptr->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].
        y_max * awb->white_y_min_percent) / 100;
      CDBG_AWB("### fixed point: white_y_min= %d\n", white_y_min);

      /* check bounding box condition for next frame */
      if (sproc->share.prev_exp_index == (int)aec_d->exp_tbl_val - 1) {
        sproc->share.awb_ext.bounding_box =   /* fully relax the bounding box */
          cptr->wb_exp_stats_hw_rolloff[AWB_STATS_LOW_LIGHT];
        CDBG_AWB("### fixed point: set bounding box lowlight WHITE ");
      } else { /* restore to the original bounding box (normal light) */
        sproc->share.awb_ext.bounding_box =
          cptr->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT];
        CDBG_AWB("### fixed point: set bounding box normal WHITE ");
      }
      sproc->share.awb_ext.bounding_box.y_min = white_y_min;
    }
  } else { /* AWB_STAT_WHITE */
    /* check bounding box condition for next frame */
    if (sproc->share.prev_exp_index == (int)aec_d->exp_tbl_val - 1) {
      sproc->share.awb_ext.bounding_box =   /* fully relax the bounding box */
        cptr->wb_exp_stats_hw_rolloff[AWB_STATS_LOW_LIGHT];
      if (cptr->AWB_lowlight_LUT.enable) {
        awb_interpolate_grey_world_ymin(sproc, awb);
      }
      CDBG_AWB("### fixed point: set bounding box lowlight GREY: ");
    } else { /* restore to the original bounding box (normal light) */
      sproc->share.awb_ext.bounding_box =
        cptr->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT];
      CDBG_AWB("### fixed point: set bounding box normal GREY: ");
    }
  }
  CDBG_AWB("%s: y_min= %d", __func__, sproc->share.awb_ext.bounding_box.y_min);
} /* awb_select_bounding_box */

/*===========================================================================
 * FUNCTION    - awb_set_led_gains -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_set_led_gains(stats_proc_t *sproc, awb_t *awb,
  float r_gain, float g_gain, float b_gain){
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
    agw_p->led_off_last_rg = g_gain / r_gain;
    agw_p->led_off_last_bg = g_gain / b_gain;
    CDBG_AWB("### LED off, led last rg=%f, last bg=%f\n",
      agw_p->led_off_last_rg, agw_p->led_off_last_bg);
  } else { /* LED on */
    agw_p->led_on_last_rg = g_gain / r_gain;
    agw_p->led_on_last_bg = g_gain / b_gain;
    CDBG_AWB("### LED on, led last rg=%f, last bg=%f\n",
      agw_p->led_on_last_rg, agw_p->led_on_last_bg);
  }
} /* awb_set_led_gains */

/*===========================================================================
 * FUNCTION    - awb_check_stat_type -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_check_stat_type(stats_proc_t *sproc, awb_t *awb)
{
  if (awb->current_awb_stat_config == AWB_STAT_REGULAR) {
    awb->current_awb_stat_config = AWB_STAT_WHITE;
  } else { /* AWB_STAT_WHITE */
    awb->current_awb_stat_config = AWB_STAT_REGULAR;
  }
} /* awb_check_stat_type */

/*===========================================================================
FUNCTION    - awb_ajdust_rgb_gains -

DESCRIPTION Adjusts rgb gains as per min gain
===========================================================================*/
static void awb_ajdust_rgb_gains(float *r_gain, float *g_gain,
  float *b_gain){
  float gain_min;
  gain_min = *g_gain;
  if (*r_gain <= gain_min)
    gain_min = *r_gain;
  if (*b_gain <= gain_min)
    gain_min = *b_gain;

  *r_gain = *r_gain * (1.0 / gain_min);
  *g_gain = *g_gain * (1.0 / gain_min);
  *b_gain = *b_gain * (1.0 / gain_min);
} /* awb_ajdust_rgb_gains */

/*===========================================================================
 * FUNCTION    - awb_no_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int awb_no_data(stats_proc_t *sproc, awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  float r_gainf, g_gainf, b_gainf;

  if (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
    if (awb->current_awb_stat_config == AWB_STAT_WHITE) {

      if (awb->white_y_min_percent >= YMIN_LOW_LIMIT)
        awb->white_y_min_percent--;

      awb->white_decision = AGW_AWB_INVALID_LIGHT;
      awb->white_has_single_peak = FALSE;
      CDBG_AWB("### white stat : zero bins. return, white decision reset \
            to -1, percentage=%d\n", awb->white_y_min_percent);
    } else
      CDBG_AWB("### reg stat : zero bins. return\n");
  } else { /* LED is ON */
    if (awb->current_awb_stat_config == AWB_STAT_WHITE) {
      return -1;
    }

    r_gainf = 1.0 / agw_p->led_rg_ratio;
    b_gainf = 1.0 / agw_p->led_bg_ratio;
    g_gainf = 1.0;

    agw_p->led_on_last_rg = agw_p->led_rg_ratio;
    agw_p->led_on_last_bg = agw_p->led_bg_ratio;
    CDBG_AWB("### LED on, zero bin, led last rg=%f, last bg=%f\n",
      agw_p->led_on_last_rg, agw_p->led_on_last_bg);

    awb_ajdust_rgb_gains(&r_gainf, &g_gainf, &b_gainf);

    awb_update_wb_gain_values(sproc, awb, r_gainf, g_gainf, b_gainf);
    sproc->share.awb_ext.decision = AGW_AWB_OUTDOOR_SUNLIGHT;

    awb->stored_prev_r_gain = r_gainf;
    awb->stored_prev_g_gain = g_gainf;
    awb->stored_prev_b_gain = b_gainf;
  }
  awb_update_wb_gain_values(sproc, awb, awb->stored_prev_r_gain,
    awb->stored_prev_g_gain, awb->stored_prev_b_gain);

  awb_check_stat_type(sproc, awb);

  return 0;
}

/*===========================================================================
 * FUNCTION    - awb_advanced_grey_world_algo_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_advanced_grey_world_algo_execute(stats_proc_t *sproc, awb_t *awb){
  int index;
  int y_ave_x, cb_ave_x, cr_ave_x;
  awb_gain_t awb_output;

  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  isp_stats_t *statStruct = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  chromatix_parms_type *cptr = sproc->input.chromatix;

  if (aec_d->use_led_estimation)
    agw_p->led_fired_for_this_frame = TRUE;
  else
    agw_p->led_fired_for_this_frame = FALSE;

  if ((awb->bestshot_d.curr_mode == CAMERA_BESTSHOT_LANDSCAPE)
      ||(awb->bestshot_d.curr_mode == CAMERA_BESTSHOT_BEACH)
      ||(awb->bestshot_d.curr_mode == CAMERA_BESTSHOT_SNOW))
    awb->search_mode = AWB_AGW_OUTDOOR_ONLY;
  else
    awb->search_mode = AWB_AGW_INOUTDOOR_BOTH;

  awb->outdoor_midpoint   = (agw_p->outdoor_index + 1) >> 1;
  awb->inoutdoor_midpoint =
    (agw_p->indoor_index + agw_p->outdoor_index + 1) >> 1;

  awb_util_aec_history_update(sproc, awb);

  if (awb->toggle_frame_skip) {
    awb_update_wb_gain_values(sproc, awb, awb->stored_prev_r_gain,
      awb->stored_prev_g_gain, awb->stored_prev_b_gain);
    awb->toggle_frame_skip = FALSE;
    return 0;
  }
  /* Proceed for AWB stats update algorithm. */
  /* setup awb input stats data start */
  memset(&(awb->stats_ptr), 0, sizeof(awb_input_stats_type));

  awb_parse_stats(sproc, awb);
  CDBG_AWB("### Current Y_min percentage=%d\n",
    awb->white_y_min_percent);

  awb->search_mode = AWB_AGW_INOUTDOOR_BOTH;

  awb->toggle_frame_skip = TRUE;

  if (!cptr->awb_enable_white_world)
    awb->current_awb_stat_config = AWB_STAT_REGULAR;

  awb_select_bounding_box(sproc, awb);

  CDBG_AWB("### bin count=%d\n", awb->stats_ptr.bin_cnt);

  if (!awb->stats_ptr.bin_cnt) { /* No data, nothing to do */
    if (awb_no_data(sproc, awb))
      return -1;
    goto done;
  }

  CDBG_AWB("### === prev white decision = %d , reg decision= %d, exp index=%d",
    awb->white_decision, awb->regular_decision,
    sproc->share.prev_exp_index);
  CDBG_AWB("### prev r gain = %f , g gain= %f, b gain=%f", awb->last_wb.
    r_gain, awb->last_wb.g_gain, awb->last_wb.b_gain);
  CDBG_AWB("Calling awb_agw_algo");

  awb_output = awb_agw_algo(sproc, awb);

  awb_check_stat_type(sproc, awb);

  if (!awb_output.is_confident) {

    awb_update_wb_gain_values(sproc, awb, awb->stored_prev_r_gain,
      awb->stored_prev_g_gain, awb->stored_prev_b_gain);

    CDBG_AWB("### AWB not confident");
    awb_set_led_gains(sproc, awb, awb->stored_prev_r_gain,
      awb->stored_prev_g_gain, awb->stored_prev_b_gain);

    goto done;
  }

  if (awb_output.sample_decision != AGW_AWB_INVALID_LIGHT &&
    awb_output.is_confident) { /* store gain adjustment for snapshot */
    awb->prev_r_gain_adj = FLOAT_TO_Q(7, awb_output.gain_adj_r);
    awb->prev_b_gain_adj = FLOAT_TO_Q(7, awb_output.gain_adj_b);
  }

  awb_update_wb_gain_values(sproc, awb, awb_output.wb_gain_r,
    awb_output.wb_gain_g, awb_output.wb_gain_b);

  if (awb->linear_gain_adj) {
    awb_compute_gain_adj(awb, agw_p->red_gain_adj, agw_p->blue_gain_adj,
      sproc->share.awb_ext.color_temp);
    awb->prev_r_gain_adj = FLOAT_TO_Q(7, awb->red_gain_adjust);
    awb->prev_b_gain_adj = FLOAT_TO_Q(7, awb->blue_gain_adjust);
  }

  CDBG_AWB("### AWB is confident");
  awb_set_led_gains(sproc, awb, awb_output.wb_gain_r,
    awb_output.wb_gain_g, awb_output.wb_gain_b);

  sproc->share.awb_ext.decision = awb_output.sample_decision;

  awb->stored_prev_r_gain = awb_output.wb_gain_r;
  awb->stored_prev_g_gain = awb_output.wb_gain_g;
  awb->stored_prev_b_gain = awb_output.wb_gain_b;

  done:
  awb_update_roll_off_for_wb(sproc, awb);
  return 0;
} /* awb_advanced_grey_world_algo_execute */

/*===========================================================================
 * FUNCTION    - awb_algo_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_algo_snapshot(stats_proc_t *sproc, awb_t *awb)
{
  float r_gain, g_gain, b_gain, k1;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  stats_proc_aec_data_t *aec_d = &(sproc->share.aec_ext);
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  switch (sproc->share.awb_ext.current_wb_type) {
    case CAMERA_WB_CLOUDY_DAYLIGHT:
      /* Note the use of "day light" color conversion for cloudy here */
      CDBG_AWB("Adjusting to CAMERA_WB_CLOUDY_DAYLIGHT\n");
      r_gain = cptr->chromatix_d65_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[0].red_gain_adj;
      b_gain = cptr->chromatix_d65_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[0].blue_gain_adj;
      g_gain = cptr->chromatix_d65_white_balance.g_gain;
      break;
    case CAMERA_WB_INCANDESCENT:  /* a.k.a "A" */
      CDBG_AWB("Adjusting to CAMERA_WB_INCANDESCENT\n");
      r_gain = cptr->chromatix_incandescent_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[2].red_gain_adj;
      b_gain = cptr->chromatix_incandescent_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[2].blue_gain_adj;
      g_gain = cptr->chromatix_incandescent_white_balance.g_gain;
      break;

    case CAMERA_WB_FLUORESCENT:  /* a.k.a "TL84" */
      CDBG_AWB("Adjusting to CAMERA_WB_FLUORESCENT\n");
      r_gain = cptr->chromatix_tl84_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[3].red_gain_adj;
      b_gain = cptr->chromatix_tl84_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[3].blue_gain_adj;
      g_gain = cptr->chromatix_tl84_white_balance.g_gain;
      break;

    case CAMERA_WB_DAYLIGHT:  /* a.k.a "D65" */
      CDBG_AWB("Adjusting to CAMERA_WB_DAYLIGHT\n");
      r_gain = cptr->chromatix_d50_white_balance.r_gain *
        cptr->awb_reference_hw_rolloff[6].red_gain_adj;
      b_gain = cptr->chromatix_d50_white_balance.b_gain *
        cptr->awb_reference_hw_rolloff[6].blue_gain_adj;
      g_gain = cptr->chromatix_d50_white_balance.g_gain;
      break;
    case CAMERA_WB_AUTO:
    case CAMERA_WB_CUSTOM:
    case CAMERA_WB_TWILIGHT:
    case CAMERA_WB_SHADE:
    default:
      CDBG_AWB("Adjusting to CAMERA_WB_AUTO\n");
      /* get the latest AEC state for LED mode operation */
      awb->outdoor_midpoint   = (agw_p->outdoor_index + 1) >> 1;
      awb->inoutdoor_midpoint =
        (agw_p->indoor_index + agw_p->outdoor_index + 1) >> 1;

      /* TODO */
      if (0/*camera_la_enable && camera_la_detect*/) {
        r_gain = awb->last_wb.r_gain;
        b_gain = awb->last_wb.b_gain;
        g_gain = awb->last_wb.g_gain;
      } else {
        if (aec_d->use_led_estimation)
          agw_p->led_fired_for_this_frame = TRUE;
        else
          agw_p->led_fired_for_this_frame = FALSE;

        if (agw_p->led_fired_for_this_frame) {
          /* if LED is in AUTO detect mode and the LED has been fired
           * do AWB prediction based on AEC data */

          /* this step is taken to guard against the
           * situation where AEC passes wrong values to AWB */
          if (aec_d->flash_si.off == 0 || aec_d->flash_si.low == 0 ||
            aec_d->flash_si.high == 0) {
            /* AEC sensitivity data is wrong. LED WB gains */
            r_gain = 1.0 / agw_p->led_off_last_rg;
            b_gain = 1.0 / agw_p->led_off_last_bg;
            g_gain = 1.0;
            awb_ajdust_rgb_gains(&r_gain, &g_gain, &b_gain);
            CDBG_AWB("prepare for snapshot: AEC sesnsitivity data invalid, use \
              LED r,g,b gain=(%f,%f,%f)", r_gain, g_gain, b_gain);
          } else {
            CDBG_AWB("prepare for snapshot: LED off last (rg,bg)=(%f,%f)",
              agw_p->led_off_last_rg, agw_p->led_off_last_bg);
            CDBG_AWB("prepare for snapshot: LED on last (rg,bg)=(%f,%f)",
              agw_p->led_on_last_rg, agw_p->led_on_last_bg);

            k1 = aec_d->flash_si.off / aec_d->flash_si.high;
            if (k1 < 1.0)
              k1 = 1.0;

            if (k1 >= 7.5) {
              /* the LED influence is strong enough to use LED RGB
               * gains for white balance */
              r_gain = 1.0 / agw_p->led_rg_ratio;
              b_gain = 1.0 / agw_p->led_bg_ratio;
            } else {/* use mixed influence between LED & ambient lights */
              float rg, bg;
              rg = ((7.5 - k1) * agw_p->led_off_last_rg +
                (k1 - 1.0) * agw_p->led_rg_ratio);
              bg = ((7.5 - k1) * agw_p->led_off_last_bg +
                (k1 - 1.0) * agw_p->led_bg_ratio);
              r_gain = 6.5 / rg;
              b_gain = 6.5 / bg;
            }
            g_gain = 1.0;

            awb_ajdust_rgb_gains(&r_gain, &g_gain, &b_gain);

            CDBG_AWB("prepare for snapshot with estimation: \
              r,g,b gain=(%f,%f,%f)", r_gain, g_gain, b_gain);
          }
        } else {
          r_gain = (awb->last_wb.r_gain * awb->prev_r_gain_adj) / 128;
          b_gain = (awb->last_wb.b_gain * awb->prev_b_gain_adj) / 128;
          g_gain = awb->last_wb.g_gain;
        }
      }
      break;
  }
  switch (sproc->share.awb_ext.current_wb_type) {
    case CAMERA_WB_AUTO:
    case CAMERA_WB_CUSTOM:
    case CAMERA_WB_TWILIGHT:
    case CAMERA_WB_SHADE:
      b_gain *= awb->bst_blue_gain_adj;
    default:
      break;
  }

  /* estimate color temperature, LED & Xenon flash can alter the color
   * temperature of the last preview estimate */
  sproc->share.awb_ext.color_temp = awb_estimate_color_temperature(awb,
    r_gain, g_gain, b_gain);
  CDBG_AWB("%s: estimated color temperature %d\n", __func__,
    sproc->share.awb_ext.color_temp);

  /* Scale gains to ensure minimum gain is 1.0 */
  awb_scale_gains(&r_gain, &g_gain, &b_gain);

  /* Cap gains to user defined values */
  awb_cap_gain(&r_gain, cptr->awb_max_gains.r_gain,
      cptr->awb_min_gains.r_gain);
  awb_cap_gain(&g_gain, cptr->awb_max_gains.g_gain,
      cptr->awb_min_gains.g_gain);
  awb_cap_gain(&b_gain, cptr->awb_max_gains.b_gain,
      cptr->awb_min_gains.b_gain);

  sproc->share.awb_ext.snapshot_wb.r_gain = r_gain;
  sproc->share.awb_ext.snapshot_wb.g_gain = g_gain;
  sproc->share.awb_ext.snapshot_wb.b_gain = b_gain;
  CDBG_AWB("awb_prepare_wb_for_snapshot r_gain %f, g_gain %f, b_gain %f\n",
    sproc->share.awb_ext.snapshot_wb.r_gain, sproc->share.awb_ext.
    snapshot_wb.g_gain, sproc->share.awb_ext.snapshot_wb.b_gain);
  memcpy(&(sproc->share.awb_ext.curr_gains), &(sproc->share.awb_ext.snapshot_wb),
    sizeof(chromatix_manual_white_balance_type));
} /* awb_algo_snapshot */

/*===========================================================================
FUNCTION      awb_restore_pre_led_settings

DESCRIPTION   Restores default rgb gains
===========================================================================*/
void awb_restore_pre_led_settings(stats_proc_t *sproc, awb_t *awb)
{
  float r_gain, g_gain, b_gain, rg_ratio, bg_ratio;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  int index;

  /* reset LED useage flag */
  agw_p->led_fired_for_this_frame = FALSE;
  /* restore pre led white balance gains */
  if (sproc->input.flash_info.led_mode != LED_MODE_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_TORCH &&
    agw_p->awb_history_count > 0) {
    index = awb_util_history_find_last_pos(awb);
    rg_ratio = 0.001*(float) agw_p->awb_history[index].rg_ratio;
    bg_ratio = 0.001*(float) agw_p->awb_history[index].bg_ratio;
    CDBG_AWB("### restore WB gains before preview starts. \
      rg_ratio = %f, bg_ratio = %f\n", rg_ratio, bg_ratio);

    g_gain = 1.0;
    r_gain = 1.0 / rg_ratio;
    b_gain = 1.0 / bg_ratio;
    awb_ajdust_rgb_gains(&r_gain, &g_gain, &b_gain);

    awb->last_wb.r_gain = r_gain;
    awb->last_wb.g_gain = g_gain;
    awb->last_wb.b_gain = b_gain;
    /* Update white balance config */
    sproc->share.awb_ext.curr_gains.r_gain = r_gain;
    sproc->share.awb_ext.curr_gains.g_gain = g_gain;
    sproc->share.awb_ext.curr_gains.b_gain = b_gain * awb->bst_blue_gain_adj;
    CDBG_AWB("### restore WB gains before preview starts. (r,g,b)"
      "gains=(%f,%f,%f)", r_gain, g_gain, b_gain);
  }
} /* awb_restore_pre_led_settings */

/*===========================================================================
 * FUNCTION    - awb_load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_load_chromatix(stats_proc_t *sproc, awb_t *awb)
{
  int i;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  awb->white_y_min_percent = cptr->awb_white_world_y_min_ratio;

  agw_p->led_rg_ratio = cptr->led_flash_white_balance.g_gain /
      cptr->led_flash_white_balance.r_gain;
  agw_p->led_bg_ratio = cptr->led_flash_white_balance.g_gain /
      cptr->led_flash_white_balance.b_gain;
  CDBG_AWB("led_rg_ratio %f, led_bg_ratio %f",
      agw_p->led_rg_ratio, agw_p->led_bg_ratio);
  /* Set Adv. Grey World 1/gain tables for each lighting condition    */
  /* Just copy the pointer over */
  for (i = 0; i < AWB_NUMBER_OF_REFERENCE_POINT; i++) {
    agw_p->red_gain_table[i] = cptr->awb_reference_hw_rolloff[i].red_gain;
    agw_p->blue_gain_table[i] = cptr->awb_reference_hw_rolloff[i].blue_gain;
    agw_p->red_gain_adj[i] = cptr->awb_reference_hw_rolloff[i].red_gain_adj;
    agw_p->blue_gain_adj[i] = cptr->awb_reference_hw_rolloff[i].blue_gain_adj;
  }
  agw_p->green_offset_rg = cptr->green_offset_rg * 2;
  agw_p->green_offset_bg = cptr->green_offset_bg * 2;
  agw_p->outlier_distance = cptr->outlier_distance * 2;
  agw_p->indoor_index = cptr->awb_indoor_index;
  agw_p->outdoor_index = cptr->awb_outdoor_index;

  for (i = 0;i < STATS_PROC_AWB_NUMBER_OF_LIGHTING_CONDITION; i++) {
    agw_p->awb_weight_vector[i][0] = cptr->awb_weight_vector[i].indoor_weight;
    agw_p->awb_weight_vector[i][1] = cptr->awb_weight_vector[i].outdoor_weight;
    agw_p->awb_weight_vector[i][2] = cptr->awb_weight_vector[i].inoutdoor_weight;
  }
  for (i = 0; i < (AGW_AWB_MAX_LIGHT - 1); i++) {
    agw_p->awb_r_adj_VF[i] = cptr->AWB_R_adj_VF[i];
    agw_p->awb_b_adj_VF[i] = cptr->AWB_B_adj_VF[i];
  }
}

/*===========================================================================
 * FUNCTION    - awb_settings_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_settings_init(stats_proc_t *sproc, awb_t *awb)
{
  int i;
  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  chromatix_parms_type *cptr = sproc->input.chromatix;

  sproc->share.awb_ext.awb_update = TRUE;
  /*   --------------- AUTO WHITE BALANCE INIT -------- */
  awb->current_awb_stat_config = AWB_STAT_REGULAR;
  awb->white_decision          = AGW_AWB_INVALID_LIGHT;
  awb->regular_decision        = AGW_AWB_INVALID_LIGHT;
  awb->reg_green_cnt           = 0;
  awb->reg_blue_cnt            = 0;
  awb->regular_ave_rg_ratio    = 0.0;
  awb->regular_ave_bg_ratio    = 0.0;
  awb->white_ave_rg_ratio      = 0.0;
  awb->white_ave_bg_ratio      = 0.0;
  awb->white_has_single_peak   = FALSE;
  sproc->share.awb_ext.current_wb_type   = CAMERA_WB_AUTO;

  awb_load_chromatix(sproc, awb);

  /* Setup Initial Decision to TL84 */
  awb->last_wb = cptr->chromatix_tl84_white_balance;
  awb->stored_prev_r_gain = cptr->chromatix_tl84_white_balance.r_gain;
  awb->stored_prev_g_gain = cptr->chromatix_tl84_white_balance.g_gain;
  awb->stored_prev_b_gain = cptr->chromatix_tl84_white_balance.b_gain;

  awb->awb_stats_conv_coef.A11 = 1 << RGBG_GRID_Q_NUM;
  awb->awb_stats_conv_coef.A12 =
    (int) (0.228 * (float) (1 << RGBG_GRID_Q_NUM) + 0.5);
  awb->awb_stats_conv_coef.A13 =
    (int) (1.402 * (float) (1 << RGBG_GRID_Q_NUM) + 0.5);
  awb->awb_stats_conv_coef.A21 = awb->awb_stats_conv_coef.A11;
  awb->awb_stats_conv_coef.A22 = awb->awb_stats_conv_coef.A12;
  awb->awb_stats_conv_coef.A23 =
    (int) (0.598 * (float) (1 << RGBG_GRID_Q_NUM) + 0.5);
  awb->awb_stats_conv_coef.A31 = awb->awb_stats_conv_coef.A11;
  awb->awb_stats_conv_coef.A32 =
    (int) (1.772 * (float) (1 << RGBG_GRID_Q_NUM) + 0.5);
  awb->awb_stats_conv_coef.A33 = awb->awb_stats_conv_coef.A23;
  awb_init_agw_algorithm(sproc, awb);

  awb->prev_r_gain_adj = 128; /* Q7 */
  awb->prev_b_gain_adj = 128; /* Q7 */

  awb->old_illuminant_choice       = 0;
  awb->old_illuminant_choice_count = 0;
  awb->toggle_frame_skip           = FALSE;
  awb->bestshot_d.curr_mode         = CAMERA_BESTSHOT_OFF;
  awb->bst_blue_gain_adj = 1.0;
} /* awb_settings_init */
