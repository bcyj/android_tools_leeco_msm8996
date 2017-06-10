/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "chromatix.h"
#include "camera.h"
#include "awb.h"
#include "camera_dbg.h"

#define BUFF_SIZE_80 80

/*===========================================================================
 * FUNCTION    - awb_self_cal_adj_ref_points -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void awb_self_cal_adj_ref_points(stats_proc_t *sproc, awb_t *awb)
{
  float x1, x2, x3, x4, xp, y1, y2, y3, y4, yp;
  float m1, m2, b1, b2, x, y, d;
  float vx = 0, vx1 = 0, vx2 = 0, vx3 = 0;
  float vy = 0, vy1 = 0, vy2 = 0, vy3 = 0;
  float w1, w2, dx, dy, fx, fy;
  int   i, correction_type;
  float d50_d65_range, self_cal_range;
  float shifted_d65_rg, shifted_d65_bg, shifted_d50_rg, shifted_d50_bg;
  int   se_is_reset = 0, nw_is_reset = 0, vertical_shifted = 0;;
  float y_limit_ratio = 0.0f, y_shift_limit_up, y_shift_limit_down;
  float x_shift_limit_up, x_shift_limit_down, ave_threshold = 0.0f;

  chromatix_parms_type *cptr = sproc->input.chromatix;

  awb_advanced_grey_world_t *agw_p = &(awb->agw);
  awb_self_cal_t *self_cal = &(awb->self_cal);

  x_shift_limit_up = cptr->awb_self_cal_adj_ratio_high;
  x_shift_limit_down = cptr->awb_self_cal_adj_ratio_low;
  CDBG_AWB("Self-Cal: ave cnt=%d, ave rg=%f, ave bg=%f\n",
    self_cal->ave_cnt, self_cal->ave_rg, self_cal->ave_bg);
  CDBG_AWB("Self-Cal: SE ave cnt=%d, SE ave rg=%f, SE ave bg=%f\n",
    self_cal->se_ave_cnt, self_cal->se_ave_rg, self_cal->se_ave_bg);
  CDBG_AWB("Self-Cal: NW ave cnt=%d, NW ave rg=%f, NW ave bg=%f\n",
    self_cal->nw_ave_cnt, self_cal->nw_ave_rg, self_cal->nw_ave_bg);

  if (self_cal->ave_cnt<SELF_CAL_AVE_LOW_THRESHOLD)
    return; /* not enough data to perform reference point adjustments */

  /* point 1 = D50,   point 2 = D65,   point 3 = Noon */
  x1 = self_cal->prev_fx *
    agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];
  y1 = self_cal->prev_fy *
    agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];

  x2 = self_cal->prev_fx *
    agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT];
  y2 = self_cal->prev_fy *
    agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT];

  ave_threshold = 0.85 * y2 + 0.15 * y1;
  CDBG_AWB("Self-Cal: ave_threshold=%f for upshift\n", ave_threshold);

  x3 = agw_p->red_gain_table[AGW_AWB_OUTDOOR_NOON];
  y3 = agw_p->blue_gain_table[AGW_AWB_OUTDOOR_NOON];

  d50_d65_range = y2 - y1; /* the B/G difference between D65 and D50. */
  self_cal_range = self_cal->nw_ave_bg - self_cal->se_ave_bg;
  CDBG_AWB("Self-Cal: Ref points range=%f, self_cal_range=%f\n",
    d50_d65_range, self_cal_range);

  if (x2 == x1) /* invalid reference points. Do nothing */
    return;
  CDBG_AWB("D50 rg=%f, D50 bg=%f\nD65 rg=%f, D65 bg=%f", x1, y1, x2, y2);
  CDBG_AWB("Self-Cal Update: Noon rg (x3)=%f, Noon bg (y3)=%f\n", x3, y3);
  /* determine the quality of SE ave and NW ave. if they are too far away from
   * the line that passes through ave and paralle to line 3, they are reset.
   * This is done to prevent the collection of bad data set point 4 to be
   * self_cal AVE */
  x4 = self_cal->ave_rg;
  y4 = self_cal->ave_bg;

  m1 = (y2 - y1) / (x2 - x1); /* it has the slope of day line 2 */
  /* day light line 4 pass through self_cal ave & parallel to day line 2 & 3 */
  b1 = y4 - m1 * x4;

  y_limit_ratio = fabs(1.0 / m1);

  x = self_cal->se_ave_rg;
  y = self_cal->se_ave_bg;

  d = m1 * x - y + b1;
  d = (d * d) / (m1 * m1 + 1);
  CDBG_AWB("Self-Cal: d2 from SE ave to line 4 (dd=%f)\n", d);
  if (d > 0.0005 || self_cal->se_ave_rg <= self_cal->ave_rg) {
    CDBG_AWB("Self-Cal Update: SE ave deviates too far (dd=%f), reset\n", d);
    se_is_reset = 1;
  }
  x = self_cal->nw_ave_rg;
  y = self_cal->nw_ave_bg;

  d = m1 * x - y + b1;
  d = (d * d) / (m1 * m1 + 1);
  CDBG_AWB("Self-Cal: d2 from NW ave to line 4 (dd=%f)\n", d);
  if (d > 0.0005 || self_cal->nw_ave_rg >= self_cal->ave_rg) {
    CDBG_AWB("Self-Cal: NW ave deviates too far (dd=%f), reset\n", d);
    nw_is_reset = 1;
  }

  b1 = y3 - m1 * x3; /* but (x3,y3) is on the line */
  if (m1 == 0)
    return; /* check for reference point calibration error */

  m2 = -1.0 / m1;
  b2 = y4 - (x4 * m2);
  /* determine how much shift it is from dayline 3 to self_cal average */
  if ((m1 - m2) == 0)  /* dayline 3 (pass through noon (x3,y3) */
    return; /* check for reference point calibration error */

  xp = (b2 - b1) / (m1 - m2);
  yp = m1 * xp + b1;
  CDBG_AWB("Self-Cal: point on line3 nearest to ave (xp=%f, yp=%f)\n", xp, yp);
  CDBG_AWB("Self-Cal: ave is x4=%f, y4=%f\n", x4, y4);

  /* determine the line shift vector from (xp,yp) to (x4,y4) */
  vx1 = x4 - xp;
  vy1 = y4 - yp;
  CDBG_AWB("line shift needed from line3 to ave: vx1=%f, vy1=%f", vx1, vy1);
  if (self_cal_range < (0.95 * d50_d65_range) || self_cal_range > (1.05 *
    d50_d65_range) || self_cal->nw_ave_cnt < SELF_CAL_NW_SE_AVE_LOW_THRESHOLD
    || self_cal->se_ave_cnt < SELF_CAL_NW_SE_AVE_LOW_THRESHOLD ||
    nw_is_reset == 1 || se_is_reset == 1) { /* use only self cal ave */
    vx = vx1;
    vy = vy1;
    vy2 = 0.0;
    correction_type = 1;
    CDBG_AWB("Self-Cal: type 1, stat data not good for point matching, \
      line shift vx=%f, vy=%f", vx, vy);
    /* for the vertical direction, the line shift may be adjusted based on
     * the property of SE and NW ave */
    if ((self_cal->se_ave_bg > y1 && self_cal->ave_bg > ave_threshold)) {
      vy2 = 0.5 * (self_cal->se_ave_bg - y1 + self_cal->ave_bg-ave_threshold);
      vy = 0.5 * vy1 + 0.5 * vy2;
      CDBG_AWB("Self-Cal Update: needs upshift\n");
      vertical_shifted = 1;
    } else if (self_cal->ave_bg < y3 && self_cal->se_ave_bg < y1) {
      vy2 = 0.33 * (self_cal->se_ave_bg - y1 + self_cal->ave_bg - y3 +
        self_cal->nw_ave_bg - y2);
      vy = 0.4 * vy1 + 0.6 * vy2;
      CDBG_AWB("Self-Cal Update: needs downshift\n");
      vertical_shifted = 1;
    }
    CDBG_AWB("adjusted line shift in y: vy1=%f, vy2=%f, vy=%f", vy1, vy2, vy);
  } else { /* use all three self cal data points */
    w1 = 0.5; /* applies to v1 */
    w2 = 0.25; /* applies to v2&v3 */
    correction_type = 2;
    /* determine daylight line 2 */
    if (x2 == x1)
      return; /* check for reference point calibration error */

    m1 = (y2 - y1) / (x2 - x1);
    b1 = y1 - m1 * x1;
    if (m1 == 0)
      return; /* check for reference point calibration error */

    m2 = -1.0 / m1;
    b2 = y3 - (x3 * m2);
    if (m1 == m2)
      return; /* check for reference point calibration error */

    xp = (b2 - b1) / (m1 - m2);
    yp = m1 * xp + b1;
    dx = (x3 - xp);
    dy = (y3 - yp);
    shifted_d65_rg =x2 + dx;
    shifted_d65_bg =y2 + dy;
    shifted_d50_rg =x1 + dx;
    shifted_d50_bg =y1 + dy;
    CDBG_AWB("nearest point on line 2 to noon, (xp=%f, yp=%f)", xp, yp);
    CDBG_AWB("shift from line 2 to line 3, (dx=%f, dy=%f)\n", dx, dy);
    CDBG_AWB("shifted D65, (rg=%f, bg=%f)\n", shifted_d65_rg, shifted_d65_bg);
    CDBG_AWB("shifted D50, (rg=%f, bg=%f)\n", shifted_d50_rg, shifted_d50_bg);

    vx2 = (self_cal->nw_ave_rg-shifted_d65_rg);
    vy2 = (self_cal->nw_ave_bg-shifted_d65_bg);
    vx3 = (self_cal->se_ave_rg-shifted_d50_rg);
    vy3 = (self_cal->se_ave_bg-shifted_d50_bg);
    CDBG_AWB("shift from D65 to NW ave, (vx2=%f, vy2=%f)\n", vx2, vy2);
    CDBG_AWB("shift from D50 to SE ave, (vx3=%f, vy3=%f)\n",vx3, vy3);

    if ((fabs(vx3) > 2.5 * fabs(vx2) && fabs(fabs(vx3) - fabs(vx2)) > 0.01) ||
      (fabs(vy3) > 2.5 * fabs(vy2) && fabs(fabs(vy3) - fabs(vy2)) > 0.01)) {
      /* SE ave may have problematic data. reset it */
      se_is_reset = 1;
      CDBG_AWB("Self-Cal Update: SE AVE has problem\n");
      correction_type = 1;
    } else if ((fabs(vx2) > 2.5 * fabs(vx3) && fabs(fabs(vx2) - fabs(vx3)) >
      0.01) || (fabs(vy2) > 2.5 * fabs(vy3) && fabs(fabs(vy2) - fabs(vy3)) >
      0.01)) {
      /* NW ave may have problematic data. reset it */
      nw_is_reset = 1;
      CDBG_AWB("Self-Cal Update: NW AVE has problem\n");
      correction_type = 1;
    } else {
      CDBG_AWB("Self-Cal Update: line shift + 2 points are used\n");
      correction_type = 2;
    }
    /* total shift */
    if (correction_type == 1) {
      vx = vx1;
      vy = vy1;
      vy2 = 0.0;
      /* for the vertical direction, the line shift may be adjusted based
       * on the property of SE and NW ave */
      if ((self_cal->se_ave_bg > y1 && self_cal->ave_bg > ave_threshold)) {
        /* needs more upshift (SE bg > D50 bg)  (AVE bg > noon bg) */
        vy2 = 0.5 * (self_cal->se_ave_bg - y1 +self_cal->ave_bg-ave_threshold);
        vy = 0.5 * vy1 + 0.5 * vy2;
        CDBG_AWB("Self-Cal Update: needs upshift\n");
        vertical_shifted = 1;
      } else if (self_cal->ave_bg < y3 && self_cal->se_ave_bg < y1) {
        vy2 = 0.33 * (self_cal->se_ave_bg - y1 + self_cal->ave_bg - y3 +
          self_cal->nw_ave_bg - y2);
        vy = 0.4 * vy1 + 0.6 * vy2;
        CDBG_AWB("Self-Cal Update: needs downshift\n");
        vertical_shifted = 1;
      }
      CDBG_AWB("line shift in y: vy1=%f, vy2=%f, vy=%f\n", vy1, vy2,vy);
    } else {
      vx = w1 * vx1 + w2 * (vx2 + vx3);
      vy = w1 * vy1 + w2 * (vy2 + vy3);
    }
  }
  CDBG_AWB("composite shift: vx=%f, vy=%f\n", vx, vy);
  /* apply shift to all reference points */
  CDBG_AWB("previous  fx=%f, fy=%f\n", self_cal->prev_fx, self_cal->prev_fy);

  fx = 1.0 + vx / agw_p->red_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];
  fy = 1.0 + vy / agw_p->blue_gain_table[AGW_AWB_OUTDOOR_SUNLIGHT1];
  CDBG_AWB("Self-Cal Update: new mult-factor: fx=%f, fy=%f\n", fx, fy);

  fx *= self_cal->prev_fx;
  fy *= self_cal->prev_fy;
  CDBG_AWB("Self-Cal: combined ref point mult-factor:fx=%f,fy=%f", fx, fy);
  /* clamp the correction factor,
   * constraint the shifting limit in x and y direction */
  if (fx > x_shift_limit_up)
    fx = x_shift_limit_up;
  else if (fx < x_shift_limit_down)
    fx = x_shift_limit_down;

  if (correction_type == 1) { /* line match only by slope */
    if (vertical_shifted == 1) {
      y_shift_limit_up = x_shift_limit_up;
      y_shift_limit_down = x_shift_limit_down;
    } else {
      y_shift_limit_up = (x_shift_limit_up - 1.0) * y_limit_ratio + 1.0;
      y_shift_limit_down = (x_shift_limit_down - 1.0) * y_limit_ratio + 1.0;
    }

    if (fy > y_shift_limit_up)
      fy = y_shift_limit_up;
    else if (fy < y_shift_limit_down)
      fy = y_shift_limit_down;
  } else { /* line match by 2 pts & slope,allow greater shift in Y direction */
    y_shift_limit_up = x_shift_limit_up;
    y_shift_limit_down = x_shift_limit_down;
    if (fy > y_shift_limit_up)
      fy = y_shift_limit_up;
    else if (fy < y_shift_limit_down)
      fy = y_shift_limit_down;
  }
  CDBG_AWB("SelfCal :x_shift_limit_up=%f,y_shift_limit_up=%f, y_limit_ratio=%f",
    x_shift_limit_up, y_shift_limit_up, y_limit_ratio);
  CDBG_AWB("Self-Cal Update: x_shift_limit_down=%f,y_shift_limit_down=%f\n",
    x_shift_limit_down, y_shift_limit_down);

  CDBG_AWB("After clamping,combined ref pnt mult-factor: fx=%f, fy=%f", fx, fy);
  /* apply correction factors to all reference points */
  for (i = 0; i < AWB_NUMBER_OF_REFERENCE_POINT; i++) {
    agw_p->red_gain_table[i] *= fx;
    agw_p->blue_gain_table[i] *= fy;
  }
  agw_p->red_gain_table[AGW_AWB_OUTDOOR_NOON] *= fx;
  agw_p->blue_gain_table[AGW_AWB_OUTDOOR_NOON] *= fy;
  /* update the correction factor */
  self_cal->prev_fx = fx;
  self_cal->prev_fy = fy;
  /* print out adjusted reference points */
  for (i = 0; i < AWB_NUMBER_OF_REFERENCE_POINT; i++) {
    CDBG_AWB("Self-Cal: Adjusted reference point [%d]: rg=%f, bg=%f\n",
      i, agw_p->red_gain_table[i],
      agw_p->blue_gain_table[i]);
  }
  CDBG_AWB("Self-Cal Update: adjusted noon rg=%f, bg=%f\n",
    agw_p->red_gain_table[AGW_AWB_OUTDOOR_NOON],
    agw_p->blue_gain_table[AGW_AWB_OUTDOOR_NOON]);

  /* if reference points have been updated, reduce the count for statistics
   * to allow iterative approaching to desired destination */
  if (self_cal->ave_cnt > SELF_CAL_AVE_LOW_THRESHOLD && correction_type == 1) {
    self_cal->ave_cnt = SELF_CAL_AVE_LOW_THRESHOLD;
    CDBG_AWB("Self-Cal Update: ave_cnt reset\n");
  }
  if (correction_type == 2) { /* make it harder to shift away */
    self_cal->se_ave_cnt = SELF_CAL_LOCK_THRESHOLD;
    self_cal->nw_ave_cnt = SELF_CAL_LOCK_THRESHOLD;
    self_cal->ave_cnt = SELF_CAL_LOCK_THRESHOLD;
  } else { /* reduce the count so that it's easier to recalibrate */
    if (se_is_reset == 1) {
      self_cal->se_ave_rg = self_cal->ave_rg;
      self_cal->se_ave_bg = self_cal->ave_bg;
      self_cal->se_ave_cnt = 1;
      CDBG_AWB("Self-Cal Update: se_ave_cnt reset to 1\n");
    }
    if (nw_is_reset == 1) {
      self_cal->nw_ave_rg = self_cal->ave_rg;
      self_cal->nw_ave_bg = self_cal->ave_bg;
      self_cal->nw_ave_cnt = 1;
      CDBG_AWB("Self-Cal Update: nw_ave_cnt reset to 1\n");
    }
    if (self_cal->se_ave_cnt >
      SELF_CAL_NW_SE_AVE_LOW_THRESHOLD) {
      self_cal->se_ave_cnt = SELF_CAL_NW_SE_AVE_LOW_THRESHOLD;
      CDBG_AWB("Self-Cal Update: se_ave_cnt reduced\n");
    }
    if (self_cal->nw_ave_cnt >
      SELF_CAL_NW_SE_AVE_LOW_THRESHOLD) {
      self_cal->nw_ave_cnt = SELF_CAL_NW_SE_AVE_LOW_THRESHOLD;
      CDBG_AWB("Self-Cal Update: nw_ave_cnt reduced\n");
    }
  }
  CDBG_AWB("Self-Cal Update: done with analyzing self-cal data\n");
} /* awb_self_cal_adj_ref_points */

/*===========================================================================
 * FUNCTION    - awb_self_cal_data_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_self_cal_data_init(stats_proc_t *sproc, awb_t *awb)
{
  stats_proc_awb_data_t *awb_ext_ip = &(sproc->share.awb_ext);
  awb_self_cal_t *self_cal = &(awb->self_cal);
  if (awb->self_cal_flag) {
    CDBG_AWB("auto self cal: ave(rg=%f, bg=%f) cnt=%d\n",
      self_cal->ave_rg, self_cal->ave_bg, self_cal->ave_cnt);
    CDBG_AWB("auto self cal: nw(rg=%f, bg=%f) cnt=%d\n",
      self_cal->nw_ave_rg, self_cal->nw_ave_bg, self_cal->nw_ave_cnt);
    CDBG_AWB("auto self cal: se(rg=%f, bg=%f) cnt=%d\n",
      self_cal->se_ave_rg, self_cal->se_ave_bg, self_cal->se_ave_cnt);
    CDBG_AWB("auto self cal: prev(fx=%f, fy=%f)\n",
      self_cal->prev_fx, self_cal->prev_fy);

    if (self_cal->ave_cnt == 0) {
      self_cal->ave_rg = 0.0;
      self_cal->ave_bg = 0.0;
      self_cal->nw_ave_rg = 0.0;
      self_cal->nw_ave_bg = 0.0;
      self_cal->nw_ave_cnt = 0;
      self_cal->se_ave_rg = 0.0;
      self_cal->se_ave_bg = 0.0;
      self_cal->se_ave_cnt = 0;
      self_cal->prev_fx = 1.0;
      self_cal->prev_fy = 1.0;
      CDBG_AWB("Self-Cal data is empty. fill in default initial values\n");
    }
    if (self_cal->enable)
      awb_self_cal_adj_ref_points(sproc, awb);
  } else {
    CDBG_AWB("File open error, awbautocal.bin, File does not exist\n");
    memset(self_cal, 0, sizeof(awb_self_cal_t));
    self_cal->prev_fx = 1.0;
    self_cal->prev_fy = 1.0;
    self_cal->enable = 1;
    CDBG_AWB("Self-Cal Update: fill in default initial values\n");
  }
} /* awb_self_cal_data_init */

/*===========================================================================
 * FUNCTION    - awb_self_cal_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_self_cal_init(stats_proc_t *sproc, awb_t *awb)
{
  /* AWB SELF CALIBRATION */
  FILE    *awb_autocal_fp = NULL;
  char     file_name [BUFF_SIZE_80] = {0};

  /* Read auto calibration */
  snprintf(file_name, BUFF_SIZE_80, "/data/awbautocal.bin");
  awb_autocal_fp = fopen(file_name, "rb");

  if (awb_autocal_fp) {
    CDBG_AWB("reading awb auto cal from efs\n");
    fread(&(awb->self_cal),
      sizeof (awb_self_cal_t), 1, awb_autocal_fp);
    fclose(awb_autocal_fp);
    awb->self_cal_flag = TRUE;
  } else
    awb->self_cal_flag = FALSE;
} /* awb_self_cal_init */

/*===========================================================================
 * FUNCTION    - awb_self_cal_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_self_cal_update(stats_proc_t *sproc, awb_t *awb,
  float rg_ratio, float bg_ratio)
{
  stats_proc_awb_data_t *awb_ext_out = &(sproc->share.awb_ext);
  awb_self_cal_t *self_cal = &(awb->self_cal);
  /* update average */
  CDBG_AWB("awb_util_agw_update_self_cal bg_ratio %f\n", bg_ratio);
  self_cal->ave_rg = self_cal->ave_rg *
    self_cal->ave_cnt + rg_ratio;
  self_cal->ave_bg = self_cal->ave_bg *
    self_cal->ave_cnt + bg_ratio;
  CDBG_AWB("ave_bg %f\n", self_cal->ave_bg);

  self_cal->ave_cnt++;

  self_cal->ave_rg =
    self_cal->ave_rg / (float) self_cal->ave_cnt;
  self_cal->ave_bg =
    self_cal->ave_bg / (float) self_cal->ave_cnt;

  CDBG_AWB("ave_bg %f\n", self_cal->ave_bg);
  if (self_cal->nw_ave_cnt == 0) { /* initialize the NW cluster */
    self_cal->nw_ave_cnt = 1;
    self_cal->nw_ave_rg = self_cal->ave_rg;
    self_cal->nw_ave_bg = self_cal->ave_bg;
  } else if (bg_ratio >= self_cal->nw_ave_bg) {
    /* gradually push the NW cluster to the NW */
    self_cal->nw_ave_rg = self_cal->nw_ave_rg *
      self_cal->nw_ave_cnt + rg_ratio;
    self_cal->nw_ave_bg = self_cal->nw_ave_bg *
      self_cal->nw_ave_cnt + bg_ratio;

    self_cal->nw_ave_cnt++;

    self_cal->nw_ave_rg = self_cal->nw_ave_rg /
      (float) self_cal->nw_ave_cnt;
    self_cal->nw_ave_bg = self_cal->nw_ave_bg /
      (float) self_cal->nw_ave_cnt;
  }
  if (self_cal->se_ave_cnt == 0) { /* initialize the SE cluster */
    self_cal->se_ave_cnt = 1;
    self_cal->se_ave_rg = self_cal->ave_rg;
    self_cal->se_ave_bg = self_cal->ave_bg;
  } else if (bg_ratio <= self_cal->se_ave_bg) {
    /* gradually push the SE cluster to the SE */
    self_cal->se_ave_rg = self_cal->se_ave_rg *
      self_cal->se_ave_cnt + rg_ratio;
    self_cal->se_ave_bg = self_cal->se_ave_bg *
      self_cal->se_ave_cnt + bg_ratio;

    self_cal->se_ave_cnt++;

    self_cal->se_ave_rg = self_cal->se_ave_rg /
      (float) self_cal->se_ave_cnt;
    self_cal->se_ave_bg = self_cal->se_ave_bg /
      (float) self_cal->se_ave_cnt;
  }

  CDBG_AWB("Self-Cal Update: ave rg=%f, ave bg=%f\n",
    self_cal->ave_rg, self_cal->ave_bg);
  CDBG_AWB("Self-Cal Update: ave nw cnt=%d, nw ave rg=%f, nw ave bg=%f\n",
    self_cal->nw_ave_cnt, self_cal->nw_ave_rg,
    self_cal->nw_ave_bg);
  CDBG_AWB("Self-Cal Update: ave se cnt=%d, se ave rg=%f, se ave bg=%f\n",
    self_cal->se_ave_cnt, self_cal->se_ave_rg,
    self_cal->se_ave_bg);
} /* awb_self_cal_update */
