/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include <string.h>
#include "stats_proc.h"
#include "af.h"

/*===========================================================================
 * FUNCTION    - af_sp_calc_slopes -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_calc_slopes(stats_proc_t *sproc, af_t *af)
{
  af->sp.slp10 = ((float)af->sp.fv[1] - af->sp.fv[0]) /
    ((float)af->sp.lens_pos[1] - af->sp.lens_pos[0]);
  af->sp.slp21 = ((float)af->sp.fv[2] - af->sp.fv[1]) /
    ((float)af->sp.lens_pos[2] - af->sp.lens_pos[1]);
  af->sp.slp32 = ((float)af->sp.fv[3] - af->sp.fv[2]) /
    ((float)af->sp.lens_pos[3] - af->sp.lens_pos[2]);
  CDBG_AF("%s: 100x slp10,slp21,slp32:%d %d %d", __func__,
    100 * af->sp.slp10, 100 * af->sp.slp21, 100 * af->sp.slp32);
} /* af_sp_calc_slopes */

/*===========================================================================
 * FUNCTION    - af_sp_check_for_maxFV -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_check_for_maxFV(stats_proc_t *sproc, af_t *af)
{
  if (af->sp.fv[af->sp.cycle] > af->max_focus_val) {
    CDBG_AF("%s: Updating FVmax. Old: %d New: %d max_step: %d", __func__,
      af->max_focus_val, af->sp.fv[af->sp.cycle], af->sp.cycle);
    af->sp.max_step = af->sp.cycle;
    af->max_focus_val = af->sp.fv[af->sp.cycle];
    af->num_downhill = 0;
  } else if (af->sp.fv[af->sp.cycle]< af->sp.fv[af->sp.cycle - 1]) {
    af->num_downhill++;
    CDBG_AF("%s: downhill val %d", __func__, af->num_downhill);
  }
} /* af_sp_check_for_maxFV */

/*===========================================================================
 * FUNCTION    - af_sp_check_for_minFV -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_check_for_minFV(stats_proc_t *sproc, af_t *af)
{
  if (af->sp.fv[af->sp.cycle] < af->min_focus_val) {
    CDBG_AF("%s: FVmin updated. Old: %d New: %d", __func__,
      af->min_focus_val, af->sp.fv[af->sp.cycle]);
    af->min_focus_val = af->sp.fv[af->sp.cycle];
  }
} /* af_sp_check_for_minFV */

/*===========================================================================
 * FUNCTION    - af_sp_check_flat_FV -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_check_flat_FV(stats_proc_t *sproc, af_t *af)
{
  int i = 0;
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  if (af->min_focus_val >
    fptr->AF_slope_predictive.af_fv_curve_flat_threshold *
    af->max_focus_val) {
    CDBG_AF("%s: sp.cycle == %d :Very flat", __func__, af->sp.cycle);
    af->sp.flat_peak = 1;
  }
  else{
    CDBG_AF("%s: Resetting flat peak flag", __func__);
    af->sp.flat_peak = 0;
  }
} /* af_sp_check_flat_FV */


/*===========================================================================
 * FUNCTION    - af_sp_two_pass_cycle0_check -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_two_pass_cycle0_check(stats_proc_t *sproc, af_t *af)
{
  if (af->sp.cycle == 0) {
    af->max_focus_val = af->sp.fv[af->sp.cycle];
    af->sp.max_step = 0;
    af->num_downhill = 0;
  } else {
    if (af->sp.fv[af->sp.cycle] > af->max_focus_val) {
      af->sp.max_step = af->sp.cycle;
      af->max_focus_val = af->sp.fv[af->sp.cycle];
      af->num_downhill = 0;
    } else if (af->sp.fv[af->sp.cycle] < af->sp.fv[af->sp.cycle - 1])
      af->num_downhill++;
    CDBG_AF("%s: num_downhill %d", __func__, af->num_downhill);
  }
} /* af_sp_two_pass_cycle0_check */

/*===========================================================================
 * FUNCTION    - af_sp_two_pass_find_maxFV -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_two_pass_find_maxFV(stats_proc_t *sproc, af_t *af, int cnt)
{
  int i;
  /* sample enough numbers, need to find largest FVs */
  af->max_focus_val = 0;
  af->sp.max_step = 0;
  for (i = 0; i < cnt; i++) {
    if (af->sp.fv[i] > af->max_focus_val) {
      af->sp.max_step = i;
      af->max_focus_val = af->sp.fv[i];
      af->max_pos = af->sp.lens_pos[af->sp.max_step];
    }
  }

  CDBG_AF("%s: max_focus_val: %d max_pos: %d max_step: %d",
    __func__, af->max_focus_val, af->max_pos, af->sp.max_step);
} /* af_sp_two_pass_find_maxFV */

/*===========================================================================
 * FUNCTION    - af_sp_run_pass0 -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_run_pass0(stats_proc_t *sproc, af_t *af)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  camera_af_done_type cb;

  /* If we are still traversing through the range */
  CDBG_AF("%s: cycle: %d total_steps: %d", __func__,
    af->sp.cycle, af->sp.total_steps);
  if (af->sp.cycle < (af->sp.total_steps + 1) / 2 - 1) {
    /* check if FVs are flat in the last pass */
    if (af->sp.cycle == 0) {
      af_sp_check_flat_FV(sproc, af);
      af->max_focus_val = af->sp.fv[af->sp.cycle];
      af->sp.max_step = 0;
      af->num_downhill = 0;
    }

    if (af->num_downhill >= fptr->AF_slope_predictive.af_downhill_allowance) {
      if (af->sp.branch == 17) {
        af->sp.move_steps = af->cur_pos - af->max_pos;
      } else
        af->sp.move_steps = af->cur_pos - af->sp.lens_pos[af->sp.max_step] - 1;
      af->max_pos = af->sp.lens_pos[af->sp.max_step];
      if (af->sp.move_steps > 0) {
        if (af->sp.branch == 17)
          af_move_lens(sproc, af, MOVE_FAR, af->sp.move_steps);
        else
          af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
      } else if (af->sp.move_steps < 0) {
        if (af->sp.branch == 17)
          af_move_lens(sproc, af, MOVE_NEAR, -af->sp.move_steps);
        else
          af_move_lens(sproc, af, MOVE_FAR, -af->sp.move_steps);
      }
      af->sp.cycle = 0;
      if (af->sp.branch == 17) {
        af->sp.search_done = 1;
        af_sp_check_flat_FV(sproc, af);
        cb = af->sp.flat_peak ? CAMERA_EXIT_CB_FAILED : CAMERA_EXIT_CB_DONE;
        CDBG_HIGH("%s Final Pos move_steps %d, max_pos %d, cur_pos %d",
          __func__, af->sp.move_steps, af->max_pos, af->cur_pos);
        af_done(sproc , af, cb);
      } else {
        af->sp.total_steps = 3;
        af->sp.srch_pass_cnt = 1;
      }
    } else {
      af->sp.cycle++;
      if (af->sp.branch == 17)
        af_move_lens(sproc, af, MOVE_NEAR, 2);
      else
        af_move_lens(sproc, af, MOVE_FAR, 2);
    }
  } else { /* We are done through entire range for first pass*/
    af_sp_two_pass_find_maxFV(sproc, af, (int)((af->sp.total_steps + 1) / 2));

    if (af->sp.branch == 17) {
      af->sp.move_steps = af->max_pos - af->cur_pos -1;
      if (af->sp.move_steps > 0) {
        af_move_lens(sproc, af, MOVE_FAR, af->sp.move_steps);
        af->sp.total_steps = 3;
        af->sp.cycle = 0;
        af->sp.srch_pass_cnt = 1;
      } else {
        if (af->cur_pos > af->near_end) {
          af_move_lens(sproc, af, MOVE_NEAR, -af->sp.move_steps);
          af->sp.total_steps = 3;
          af->sp.cycle = 0;
          af->sp.srch_pass_cnt = 1;
        } else {
          af_move_lens(sproc, af, MOVE_FAR, 1);
          af->sp.total_steps = 2;
          af->sp.cycle = 0;
          af->sp.srch_pass_cnt = 1;
        }
      }
    } else {
      af->max_pos = af->sp.lens_pos[af->sp.max_step];
      af->sp.move_steps = af->cur_pos - af->max_pos -1;
      if (af->sp.move_steps > 0)
        af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
      else
        af_move_lens(sproc, af, MOVE_FAR, -af->sp.move_steps);
      af->sp.cycle = 0;
      af->sp.total_steps = 3;
      af->sp.srch_pass_cnt = 1;
    }
  }
} /* af_sp_run_pass0 */

/*===========================================================================
 * FUNCTION    - af_sp_run_pass1 -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_run_pass1(stats_proc_t *sproc, af_t *af)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  camera_af_done_type cb;

  if (af->sp.cycle < af->sp.total_steps - 1) {
    af_sp_two_pass_cycle0_check(sproc, af);
    if (af->num_downhill >=
        fptr->AF_slope_predictive.af_downhill_allowance_1) { //max 0
      af->max_pos = af->sp.lens_pos[af->sp.max_step];
      af->sp.move_steps = af->max_pos - af->cur_pos;
      if (af->sp.move_steps > 0) {
        if (af->sp.branch == 17)
          af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
        else
          af_move_lens(sproc, af, MOVE_FAR, af->sp.move_steps);
      } else if (af->sp.move_steps < 0) {
        if (af->sp.branch == 17)
          af_move_lens(sproc, af, MOVE_FAR, -af->sp.move_steps);
        else
          af_move_lens(sproc, af, MOVE_NEAR, -af->sp.move_steps);
      }
      af->sp.search_done = 1;
      CDBG_HIGH("%s Final Pos move_steps %d, max_pos %d, cur_pos %d max_fv: %d",
        __func__, af->sp.move_steps, af->max_pos, af->cur_pos, af->max_focus_val);
      /* Check for flat FV. If not we are good */
      af_sp_check_flat_FV(sproc, af);
      cb = af->sp.flat_peak ? CAMERA_EXIT_CB_FAILED : CAMERA_EXIT_CB_DONE;
      af_done(sproc , af, cb);
    } else {
      if (af->sp.branch == 17)
        af_move_lens(sproc, af, MOVE_FAR, 1);
      else
        af_move_lens(sproc, af, MOVE_NEAR, 1);
      af->sp.cycle++;
    }
  } else {
    af_sp_two_pass_find_maxFV(sproc, af, af->sp.total_steps);
    if (af->sp.branch == 17)
      af->sp.move_steps = af->cur_pos - af->max_pos;
    else
      af->sp.move_steps = af->max_pos - af->cur_pos;

    if (af->sp.move_steps > 0) {
      if (af->sp.branch == 17)
        af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
      else
        af_move_lens(sproc, af, MOVE_FAR, -af->sp.move_steps);
    } else if (af->sp.move_steps < 0) {
      if (af->sp.branch == 17)
        af_move_lens(sproc, af, MOVE_FAR, -af->sp.move_steps);
      else
        af_move_lens(sproc, af, MOVE_NEAR, -af->sp.move_steps);
    }
    af->sp.search_done = 1;
    CDBG_HIGH("%s Final Pos move_steps %d, max_pos %d, cur_pos %d max_fv: %d",
      __func__, af->sp.move_steps, af->max_pos, af->cur_pos, af->max_focus_val);

    /* Check if FV is flat */
    af_sp_check_flat_FV(sproc, af);
    cb = af->sp.flat_peak ? CAMERA_EXIT_CB_FAILED : CAMERA_EXIT_CB_DONE;
    af_done(sproc , af, cb);
    CDBG_AF("maxfv,maxloc = %d: %d", af->max_focus_val, af->max_pos);
  }
} /* af_sp_run_pass1 */

/*===========================================================================
 * FUNCTION    - af_sp_two_pass_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_two_pass_srch(stats_proc_t *sproc, af_t *af)
{
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  af->sp.fv[af->sp.cycle] =
    (int)((stats->af_op.Focus / stats->af_op.NFocus) / MAX(sproc->share.cur_af_luma, 1));
  af->sp.lens_pos[af->sp.cycle] = af->cur_pos;
  af_sp_check_for_minFV(sproc, af);
  af_sp_check_for_maxFV(sproc, af);
  CDBG_AF("%s: Srch Pass %d, Cycle %d, FV %d, FVmax %d FVmin %d lens_pos %d",
    __func__,
    af->sp.srch_pass_cnt, af->sp.cycle, af->sp.fv[af->sp.cycle],
    af->max_focus_val, af->min_focus_val, af->sp.lens_pos[af->sp.cycle]);

  if (af->sp.srch_pass_cnt == 0)
    af_sp_run_pass0(sproc, af);
  else
    af_sp_run_pass1(sproc, af);

} /* af_sp_two_pass_srch */

/*===========================================================================
 * FUNCTION    - af_sp_get_branch_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int af_sp_get_branch_info(stats_proc_t *sproc, af_t *af)
{
  CDBG_AF("%s: Branch %d", __func__, af->sp.branch);
  switch (af->sp.branch) {
    case 1:
      CDBG_AF("Case Peak is between [1,Infinity]");
      af->sp.move_steps = af->sp.lens_pos[1] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos0_step_cnt + af->sp.pos1_step_cnt - 1;
      break;
    case 3:
      CDBG_AF("Case flat Peak is between [2,0]");
      af->sp.move_steps = af->sp.lens_pos[2] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos1_step_cnt + af->sp.pos2_step_cnt - 1;
      break;
    case 4:
      CDBG_AF("Case 4, Peak is between [2,1]");
      af->sp.move_steps = af->sp.lens_pos[2] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos2_step_cnt - 1;
      break;
    case 5:
      CDBG_AF("Case 5, peak very close to 2, [(3+2)/2,(2+1)/2]");
      af->sp.move_steps = (int)((af->sp.lens_pos[3] + af->sp.lens_pos[2]) / 2) - af->cur_pos + 1;
      af->sp.total_steps = (af->sp.pos2_step_cnt + af->sp.pos3_step_cnt) / 2 - 1;
      break;
    case 6:
      CDBG_AF("Case 6, Peak is between [3,1]");
      af->sp.move_steps = af->sp.lens_pos[3] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos2_step_cnt + af->sp.pos3_step_cnt - 1;
      break;
    case 7:
      CDBG_AF("Case 7, Peak is between [3,2]");
      af->sp.move_steps = af->sp.lens_pos[3] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos3_step_cnt - 1;
      break;
    case 8:
      CDBG_AF("Case 8, Peak is between [near,2], very close");
      af->sp.move_steps = af->sp.pos4_step_cnt;
      af->sp.total_steps = af->sp.move_steps;
      break;
    case 9:
      CDBG_AF("Case 9 peak at [3,2]");
      af->sp.move_steps = af->sp.lens_pos[1] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos3_step_cnt - 1;
      break;
    case 10:
      CDBG_AF("Case 10, Peak is between [4,3]");
      af->sp.move_steps = af->sp.lens_pos[2] - af->cur_pos + 1 ;
      af->sp.total_steps = af->sp.pos4_step_cnt - 1;
      break;
    case 11:
      CDBG_AF("Case 11, Peak is between (mid(4,3), mid(3,2))");
      af->sp.move_steps = (int)((af->sp.lens_pos[2] + af->sp.lens_pos[1]) / 2) - af->cur_pos + 1;
      af->sp.total_steps = (af->sp.pos4_step_cnt + af->sp.pos3_step_cnt) / 2 - 1;
      break;
    case 12:
      CDBG_AF("Case 12, Peak is between [4,2]");
      af->sp.move_steps = af->sp.lens_pos[2] - af->cur_pos + 1 ;
      af->sp.total_steps = af->sp.pos3_step_cnt + af->sp.pos4_step_cnt - 1;
      break;
    case 13:
      CDBG_AF("Case 13, Peak is between [4,3]");
      af->sp.move_steps = af->sp.lens_pos[2] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos4_step_cnt - 1;
      break;
    case 14:
      CDBG_AF("Case 14, Peak is between (mid(5,4),mid(4,3))");
      af->sp.move_steps = (int)((af->sp.lens_pos[3] + af->sp.lens_pos[2]) / 2) - af->cur_pos + 1;
      af->sp.total_steps = (af->sp.pos4_step_cnt + af->sp.pos5_step_cnt) / 2 - 1;
      break;
    case 15:
      CDBG_AF("Case 15, Peak is between [5,3]");
      af->sp.move_steps = af->sp.lens_pos[3] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos4_step_cnt + af->sp.pos5_step_cnt - 1;
      break;
    case 16:
      CDBG_AF("Case 16, Peak is between [5,4]");
      af->sp.move_steps = af->sp.lens_pos[3] - af->cur_pos + 1;
      af->sp.total_steps = af->sp.pos5_step_cnt - 1 ;
      break;
    case 17:
      CDBG_AF("Case 17, Peak is between [near,mid(5,4)], very close");
      af->sp.move_steps = (int)((af->sp.lens_pos[3] + af->sp.lens_pos[2]) / 2) - af->cur_pos;
      af->sp.total_steps =(int)((af->sp.lens_pos[3] + af->sp.lens_pos[2]) / 2) - af->near_end + 1;
      break;
    default:
      return -1;
  }
  return 0;
} /* af_sp_get_branch_info */

/*===========================================================================
 * FUNCTION    - af_slope_predictive_start_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_slope_predictive_start_srch(stats_proc_t *sproc, af_t *af)
{
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;
  unsigned int pos0, pos1;

  memset(&(af->sp), 0, sizeof(af_slope_predictive_algo_t));
  af->state =  AF_SLOPE_PREDICTIVE;
  af->max_focus_val = 0;
  af->min_focus_val = 0;
  af->num_downhill  = 0;

  /* Steps to reach lens position with focued image when object is at 3m*/
  pos0 = fptr->AF_slope_predictive.af_lens_pos_0;
  af->sp.pos0_step_cnt = af->infy_pos - pos0;

  /* Steps to reach lens position with focued image when object is at 70cm*/
  pos1 = fptr->AF_slope_predictive.af_lens_pos_1;
  af->sp.pos1_step_cnt = pos0 - pos1;

  /* Steps to reach lens position with focued image when object is at 30cm*/
  pos0 = fptr->AF_slope_predictive.af_lens_pos_2;
  af->sp.pos2_step_cnt = pos1 - pos0;

  /* Steps to reach lens position with focued image when object is at 20cm*/
  pos1 = fptr->AF_slope_predictive.af_lens_pos_3;
  af->sp.pos3_step_cnt = pos0 - pos1;

  /* Steps to reach lens position with focued image when object is at 10cm*/
  pos0 = fptr->AF_slope_predictive.af_lens_pos_4;
  af->sp.pos4_step_cnt = pos1 - pos0;

  /* Step to reach lens position when object is closer than 10 cm.*/
  pos1 = fptr->AF_slope_predictive.af_lens_pos_5;
  af->sp.pos5_step_cnt = pos0 - pos1;

  CDBG_AF("%s: Calculated steps to reach pos0=%d pos1=%d pos2=%d pos3=%d"
    "pos4=%d pos5=%d", __func__, af->sp.pos0_step_cnt,
    af->sp.pos1_step_cnt, af->sp.pos2_step_cnt, af->sp.pos3_step_cnt,
    af->sp.pos4_step_cnt, af->sp.pos5_step_cnt);

  af_move_lens(sproc, af, MOVE_NEAR, af->sp.pos0_step_cnt);
} /* af_slope_predictive_start_srch */

/*===========================================================================
 * FUNCTION    - af_sp_collect_sample_FV -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_collect_sample_FV(stats_proc_t *sproc, af_t *af)
{
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  if (af->sp.cycle <= 3) {
    af->sp.fv[af->sp.cycle] =
      (int)((stats->af_op.Focus / stats->af_op.NFocus) / MAX(sproc->share.cur_af_luma, 1));
    af->sp.lens_pos[af->sp.cycle] = af->cur_pos;
    af_sp_check_for_minFV(sproc, af);
    CDBG_AF("%s: Cycle %d, FV %d, FVmin %d Lens Pos %d", __func__,
      af->sp.cycle, af->sp.fv[af->sp.cycle], af->min_focus_val,
      af->sp.lens_pos[af->sp.cycle]);
  }
  if (af->sp.cycle == 0) {
    /* recieved 1st sample - sample 0. Just move to 2nd one */
    af->sp.max_step = af->sp.cycle;
    af->max_focus_val = af->sp.fv[af->sp.cycle];
    af->min_focus_val = af->sp.fv[af->sp.cycle];
    af->num_downhill = 0;
    af->sp.cycle++;
    af->sp.move_steps = af->sp.pos1_step_cnt;
    af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
  } else if (af->sp.cycle == 1) {
    /* Got second sample - sample 1. */
    af_sp_check_for_maxFV(sproc, af);
    /* Check if we exceed number of donwhill allowance.
       If yes, our maximum will probably be between Sample 1
       and infinity */
    if (af->num_downhill >=
        fptr->AF_slope_predictive.af_downhill_allowance) {
      af->sp.max_step = af->sp.cycle;
      af->sp.branch = 1;
      if (af_sp_get_branch_info(sproc, af) == 0)
        af_move_lens(sproc, af, MOVE_FAR, af->sp.move_steps);
      af->sp.stage = 1;
      af->sp.cycle = 0;
    } else {
      /* Move to next sample point */
      af->sp.cycle++;
      af->sp.move_steps = af->sp.pos2_step_cnt;
      af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
    }
  } else if (af->sp.cycle == 2) {
    /* Received third sample - sample 2*/
    af_sp_check_for_maxFV(sproc, af);

    /* Check if we have been going down */
    if (af->num_downhill >=
        fptr->AF_slope_predictive.af_downhill_allowance) {
      af->max_pos = af->sp.lens_pos[af->sp.max_step];
      /* If max value was at Sample 0, we'll enter branch 1*/
      if (af->sp.max_step == 0)
        af->sp.branch = 1;
      /* Max was at Sample 1. Peak can be between sample 2
         and sample 0. */
      else if (af->sp.max_step == 1)
        af->sp.branch = 3;
      if (af_sp_get_branch_info(sproc, af) == 0)
        af_move_lens(sproc, af, MOVE_FAR, af->sp.move_steps);
      af->sp.cycle = 0;
      af->sp.stage = 1;
    } else {
      /* Still within downhill allowance limit. Get one more sample*/
      af->sp.cycle++;
      af->sp.move_steps = af->sp.pos3_step_cnt;
      af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
    }
  } else if (af->sp.cycle == 3) {
    /* Received Sample 3 */
    int direction = MOVE_FAR;
    /* calculate slope */
    af_sp_calc_slopes(sproc, af);
    af_sp_check_for_maxFV(sproc, af);
    af->max_pos = af->sp.lens_pos[af->sp.max_step];
    CDBG_AF("%s: max fv position, max fv, min fv: %d: %d:%d", __func__,
      af->max_pos, af->max_focus_val, af->min_focus_val);

    af_sp_check_flat_FV(sproc, af);

    af->sp.cycle = 0; /* set cycle to 0 as it applies to all cases except 8 */
    /* Max is at Sample 1. So peak might lie between Sample 2 and Sample 0.*/
    if (af->sp.max_step == 1)
      af->sp.branch = 3;
    /* Max is at Sample 2.*/
    else if (af->sp.max_step == 2) {
      if (abs(af->sp.slp10) >
          fptr->AF_slope_predictive.af_slope_threshold4 * abs(af->sp.slp21) &&
          -af->sp.slp32 >
          fptr->AF_slope_predictive.af_slope_threshold4 * abs(af->sp.slp21))
        /* Peak between sample 2 and sample 1 */
        af->sp.branch = 4;
      else if (abs(af->sp.slp21)>
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp10) &&
               abs(af->sp.slp32) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp10)) {
        /* slope(1,3) is less. peak can be between midpoint of (1,2) and (2,3)*/
        af->sp.branch = 5;
        if (af->sp.fv[3] < af->sp.fv[1])
          CDBG_AF("Peak is between [3,2] but wierd");
      } else
        /* Can be between Sample 2 and 3.*/
        af->sp.branch = 6;
    } else {  //af->sp.max_step == 3
      /* Max is at Sample 3.*/
      if (abs(af->sp.slp32) <
          fptr->AF_slope_predictive.af_slope_threshold3 * abs(af->sp.slp21) &&
          !af->sp.flat_peak)
        af->sp.branch = 7;
      else {
        af->sp.branch = 8;
        /* Branch 8. Take two more samples */
        /* shift LP & FV vals to accomodate ref points 4 & 5*/
        af->sp.fv[0]       = af->sp.fv[2];
        af->sp.lens_pos[0] = af->sp.lens_pos[2];
        af->sp.fv[1]       = af->sp.fv[3];
        af->sp.lens_pos[1] = af->sp.lens_pos[3];
        af->sp.cycle = 2;
        af->sp.max_step = af->sp.max_step - 2;
        /* In this case we'll continue moving in same direction and take 2
           more samples */
        direction = MOVE_NEAR;
      }
    }
    if (af_sp_get_branch_info(sproc, af) == 0)
      af_move_lens(sproc, af, direction, af->sp.move_steps);
    af->sp.stage = 1;
  }
  af->sp.frame_delay = 1;
} /* af_sp_collect_sample_FV */

/*===========================================================================
 * FUNCTION    - af_sp_do_check_for_stage2 -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void af_sp_do_check_for_stage2(stats_proc_t *sproc, af_t *af)
{
  int i;
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  af_tune_parms_t *fptr = sproc->input.af_tune_ptr;

  /* macro mode, move another 2 steps */
  CDBG_AF("peak at [near,2]");
  af->sp.fv[af->sp.cycle] =
    (int)((stats->af_op.Focus / stats->af_op.NFocus) / MAX(sproc->share.cur_af_luma, 1));
  af->sp.lens_pos[af->sp.cycle] = af->cur_pos;

  af_sp_check_for_minFV(sproc, af);
  af_sp_check_for_maxFV(sproc, af);
  CDBG_AF("%s: Cycle %d, FV %d, FVmin %d FVmax %d lens_pos %d", __func__,
    af->sp.cycle, af->sp.fv[af->sp.cycle], af->min_focus_val,
    af->max_focus_val, af->sp.lens_pos[af->sp.cycle]);

  if (af->sp.cycle == 2) {
    /* We are at Sample 4. Get Sample 5 */
    CDBG_AF("stage == 2, af->sp.cycle == 2");
    af->sp.cycle++;
    af->sp.move_steps = af->sp.pos5_step_cnt;
    af_move_lens(sproc, af, MOVE_NEAR, af->sp.move_steps);
  } else {
    /* Sample 5 received */
    CDBG_AF("stage == 3, af->sp.cycle == 3");
    /* need to determine next step */

    CDBG_AF("max fv position, max fv, min fv: %d: %d:%d",
      af->max_pos, af->max_focus_val, af->min_focus_val);
    af_sp_check_flat_FV(sproc, af);

    /* calculate slope */
    af_sp_calc_slopes(sproc, af);
    if (af->sp.max_step == 1) { // peak between [4,2]
      if (abs(af->sp.slp21) <
          fptr->AF_slope_predictive.af_slope_threshold2 *
          abs(af->sp.slp32) &&
          abs(af->sp.slp21) > fptr->AF_slope_predictive.af_slope_threshold1 *
          abs(af->sp.slp32) &&
          (af->sp.fv[1]) > af->sp.fv[3])
        af->sp.branch = 9;
      else if (abs(af->sp.slp10) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp21) &&
               abs(af->sp.slp32) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp21))
        af->sp.branch = 10;
      else if (abs(af->sp.slp10) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp32) &&
               abs(af->sp.slp21) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp32))
        af->sp.branch = 11;
      else
        af->sp.branch = 12;
    } else if (af->sp.max_step == 2) {
      if (abs(af->sp.slp32) > fptr->AF_slope_predictive.af_slope_threshold4 *
          abs(af->sp.slp21) &&
          abs(af->sp.slp10) > fptr->AF_slope_predictive.af_slope_threshold4 *
          abs(af->sp.slp21))
        af->sp.branch = 13;
      else if (abs(af->sp.slp32) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp10) &&
               abs(af->sp.slp21) >
               fptr->AF_slope_predictive.af_slope_threshold4 *
               abs(af->sp.slp10))
        af->sp.branch = 14;
      else
        af->sp.branch = 15;
    } else { //peak between [near,4]
      if (!af->sp.flat_peak && abs(af->sp.slp32) <
          fptr->AF_slope_predictive.af_slope_threshold3 * abs(af->sp.slp21))
        af->sp.branch = 16;
      else
        af->sp.branch = 17;
    } /* end of max_pos */
    if (af_sp_get_branch_info(sproc, af) == 0)
      af_move_lens(sproc, af, MOVE_FAR, af->sp.move_steps);
    af->sp.cycle = 0;
    af->sp.stage = 2;
  } /* end of cycle */
  af->sp.frame_delay = 1;
} /* af_sp_do_check_for_stage2 */

/*===========================================================================
 * FUNCTION    - af_slope_predictive_srch -
 *
 * DESCRIPTION:
 *==========================================================================*/
int  af_slope_predictive_srch(stats_proc_t *sproc, af_t *af)
{
  int rc = 0;

  if (af->sp.frame_delay > 0) {
    af->sp.frame_delay --;
    CDBG_AF("%s: Skipped this frame %d", __func__, af->sp.frame_delay);
    return rc;
  }

  if (af->state == AF_START)
    af_slope_predictive_start_srch(sproc, af);
  else if (af->sp.stage == 0)
    af_sp_collect_sample_FV(sproc, af);
  else if (af->sp.branch == 8)
    af_sp_do_check_for_stage2(sproc, af);
  else
    af_sp_two_pass_srch(sproc, af);

  return rc;
} /* af_slope_predictive_srch */
