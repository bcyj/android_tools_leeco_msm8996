/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "chromatix.h"
#include "camera.h"
#include "awb.h"

/*===========================================================================
 * FUNCTION    - awb_util_history_find_last_pos -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_util_history_find_last_pos(awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  if (agw_p->awb_history_count == 0)
    return 0;
  else if (agw_p->awb_history_next_pos == 0)
    return(AWB_MAX_HISTORY - 1);
  else
    return(agw_p->awb_history_next_pos - 1);
} /* awb_util_history_find_last_pos */

/* AWB AEC HISTORY */
/*===========================================================================
 * FUNCTION    - awb_util_aec_history_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_util_aec_history_update(stats_proc_t *sproc, awb_t *awb)
{
  awb_advanced_grey_world_t *agw_p = &(awb->agw);

  /* Add new entry to the history */
  agw_p->aec_history[agw_p->aec_history_next_pos].exp_index =
    (int)(sproc->share.prev_exp_index);
  agw_p->aec_history[agw_p->aec_history_next_pos].frame_luma =
    (int)(sproc->share.aec_ext.cur_luma);

  /* Update buffer count and next position */
  if (agw_p->aec_history_count < AWB_AEC_MAX_HISTORY)
    agw_p->aec_history_count++;
  agw_p->aec_history_next_pos = (agw_p->aec_history_next_pos + 1)
  % AWB_AEC_MAX_HISTORY;
} /* awb_util_aec_history_update */

/*===========================================================================
 * FUNCTION    - awb_util_convert_to_grid -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_util_convert_to_grid(awb_t *awb, int rg_ratio_x,
  int bg_ratio_x, int *rg_grid, int *bg_grid)
{
  int index2;

  for (index2 = 0; index2 < AGW_NUMBER_GRID_POINT - 1; index2++) {
    /* find the R/G grid index */
    if (rg_ratio_x < awb->agw.rgbg_grid_x[0]) {
      *rg_grid = 0;
      break;
    }
    if (rg_ratio_x >= awb->agw.rgbg_grid_x[index2] &&
      rg_ratio_x <  awb->agw.rgbg_grid_x[index2 + 1]) {
      *rg_grid = index2;
      break;
    }
  }

  if (index2 == AGW_NUMBER_GRID_POINT)
    *rg_grid = AGW_NUMBER_GRID_POINT - 1;
  else if ((rg_ratio_x - awb->agw.rgbg_grid_x[index2]) >
    (awb->agw.rgbg_grid_x[index2 + 1] - rg_ratio_x))
    *rg_grid++;

  for (index2 = 0; index2 < AGW_NUMBER_GRID_POINT - 1; index2++) {
    /* find the B/G grid index */
    if (bg_ratio_x < awb->agw.rgbg_grid_x[0]) {
      *bg_grid = 0;
      break;
    }
    if (bg_ratio_x >= awb->agw.rgbg_grid_x[index2] &&
      bg_ratio_x <  awb->agw.rgbg_grid_x[index2 + 1]) {
      *bg_grid = index2;
      break;
    }
  }

  if (index2 == AGW_NUMBER_GRID_POINT)
    *bg_grid = AGW_NUMBER_GRID_POINT - 1;
  else if ((bg_ratio_x - awb->agw.rgbg_grid_x[index2]) >
    (awb->agw.rgbg_grid_x[index2 + 1] - bg_ratio_x))
    *bg_grid++;

}/* awb_util_convert_to_grid */

/*===========================================================================
 * FUNCTION    - awb_set_bestshot_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_set_bestshot_mode(stats_proc_t *sproc, awb_t *awb,
  camera_bestshot_mode_type new_mode)
{
  int rc = 0;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  if (new_mode  >= CAMERA_BESTSHOT_MAX)
    return -1;

  if (awb->bestshot_d.curr_mode == new_mode)
    return 0; /* Do Nothing */

  CDBG_AWB("%s: mode %d", __func__, new_mode);
  /* Store current AWB vals */
  if (awb->bestshot_d.curr_mode == CAMERA_BESTSHOT_OFF) {
    awb->bestshot_d.stored_wb = sproc->share.awb_ext.current_wb_type;
    awb->bestshot_d.stored_bst_blue_gain_adj = awb->bst_blue_gain_adj;
  }
  /* CONFIG AWB for BESTHOT mode */
  if (new_mode  != CAMERA_BESTSHOT_OFF) {
    switch (new_mode) {
      case CAMERA_BESTSHOT_SNOW:
        awb->bst_blue_gain_adj = cptr->snow_blue_gain_adj_ratio;
        CDBG_AWB("%s: snow %f", __func__, awb->bst_blue_gain_adj);
        break;
      case CAMERA_BESTSHOT_BEACH:
        awb->bst_blue_gain_adj = cptr->beach_blue_gain_adj_ratio;
        CDBG_AWB("%s: beach %f", __func__, awb->bst_blue_gain_adj);
        break;
      case CAMERA_BESTSHOT_OFF:
      case CAMERA_BESTSHOT_LANDSCAPE:
      case CAMERA_BESTSHOT_NIGHT:
      case CAMERA_BESTSHOT_PORTRAIT:
      case CAMERA_BESTSHOT_BACKLIGHT:
      case CAMERA_BESTSHOT_SPORTS:
      case CAMERA_BESTSHOT_ANTISHAKE:
      case CAMERA_BESTSHOT_FLOWERS:
      case CAMERA_BESTSHOT_PARTY:
      case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
      case CAMERA_BESTSHOT_THEATRE:
      case CAMERA_BESTSHOT_ACTION:
      case CAMERA_BESTSHOT_AR:
      case CAMERA_BESTSHOT_SUNSET:
      case CAMERA_BESTSHOT_FIREWORKS:
      case CAMERA_BESTSHOT_CANDLELIGHT:
      default:
        awb->bst_blue_gain_adj = 1.0;
    }
    switch (new_mode) {
      case CAMERA_BESTSHOT_FIREWORKS:
        rc = awb_set_current_wb(sproc, awb, CAMERA_WB_CLOUDY_DAYLIGHT);
        break;
      case CAMERA_BESTSHOT_CANDLELIGHT:
      case CAMERA_BESTSHOT_SUNSET:
        rc = awb_set_current_wb(sproc, awb, CAMERA_WB_INCANDESCENT);
        break;
      case CAMERA_BESTSHOT_OFF:
      case CAMERA_BESTSHOT_LANDSCAPE:
      case CAMERA_BESTSHOT_SNOW:
      case CAMERA_BESTSHOT_BEACH:
      case CAMERA_BESTSHOT_NIGHT:
      case CAMERA_BESTSHOT_PORTRAIT:
      case CAMERA_BESTSHOT_BACKLIGHT:
      case CAMERA_BESTSHOT_SPORTS:
      case CAMERA_BESTSHOT_ANTISHAKE:
      case CAMERA_BESTSHOT_FLOWERS:
      case CAMERA_BESTSHOT_PARTY:
      case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
      case CAMERA_BESTSHOT_THEATRE:
      case CAMERA_BESTSHOT_ACTION:
      case CAMERA_BESTSHOT_AR:
      default:
        rc = awb_set_current_wb(sproc, awb, CAMERA_WB_AUTO);
        break;
    }
  } else { /* Restore AWB vals */
    rc = awb_set_current_wb(sproc, awb, awb->bestshot_d.stored_wb);
    awb->bst_blue_gain_adj =  awb->bestshot_d.stored_bst_blue_gain_adj;
  }
  awb->bestshot_d.curr_mode = new_mode;
  return rc;
} /* awb_set_bestshot_mode */
