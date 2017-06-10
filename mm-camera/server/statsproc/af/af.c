/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include "stats_proc.h"
#include "af.h"
#include "camera_dbg.h"

/*===========================================================================
 * FUNCTION    - af_run_algorithm -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_run_algorithm(stats_proc_t *sproc, af_t *af)
{
  int rc =0;
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  sproc->share.af_ext.reset_lens       = FALSE;
  sproc->share.af_ext.done_flag        = FALSE;
  sproc->share.af_ext.move_lens_status = FALSE;
  sproc->share.af_ext.stop_af          = FALSE;

  /* If search mode is infinity, we'll just return */
  if (af->srch_mode == AF_MODE_INFINITY) {
    CDBG_AF("%s: Search mode INFY. Returning!", __func__);
    return 0;
  }

  /* We cannot focus forever, if we exceed some max count, then just fail
   * and return to inactive state */
  if (af->elapsed_frame_cnt >= MAX_FRAMES_TO_PERFORM_FOCUS) {
    CDBG_ERROR("%s: Fail: AF is taking too long - exiting", __func__);
    af_stop_focus(sproc, af);
    return 0;
  }
  if (stats->af_op.NFocus > STATS_BUFFER_MAX_ENTRIES)
    stats->af_op.NFocus = STATS_BUFFER_MAX_ENTRIES;

  CDBG("%s: Focus : %u, NFocus : %u\n", __func__, stats->af_op.Focus, stats->af_op.NFocus);
  /* if af stat is not valid, skip a frame */
  if ((!stats->af_op.Focus) || (!stats->af_op.NFocus)) {
    CDBG_ERROR("%s:Fail Invalid AF stats", __func__);
    af_done(sproc, af, CAMERA_EXIT_CB_FAILED);
    return -1;
  }
  af->cur_focus_val = (int) (stats->af_op.Focus / stats->af_op.NFocus /
    MAX(sproc->share.cur_af_luma, 1));

  if (af->algo_type == AF_EXHAUSTIVE_FAST &&
    !sproc->share.af_ext.cont_af_enabled)
    af->frame_skip = TRUE;
  else
    af->frame_skip = FALSE;

  switch (af->algo_type) {
    case AF_EXHAUSTIVE_SEARCH:
    case AF_EXHAUSTIVE_FAST:
	if (sproc->share.af_ext.cont_af_enabled)
          rc = af_exhaustive_search(sproc, af);
        else
          rc = af_slope_predictive_srch(sproc, af);
      break;

    case AF_HILL_CLIMBING_CONSERVATIVE:
    case AF_HILL_CLIMBING_DEFAULT:
    case AF_HILL_CLIMBING_AGGRESSIVE:
#ifdef HILL_CLIMB_ALGO
      /* HILL climb search is not enabled
       * we use exhaustive search for both CAF & Snapshot AF */
      af_init_hill_climbing_search(sproc, af);
      rc = af_hill_climbing_search(sproc, af);
#endif
      break;
    default:
      CDBG_ERROR("%s:Fail Undefined ALGO TYPE %d", __func__, af->algo_type);
      return -1;
  }
  return rc;
} /* af_run_algorithm */
