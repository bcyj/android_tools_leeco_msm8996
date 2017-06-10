/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "stats_proc.h"
#include "af.h"

static af_t *afCtrl[MAX_INSTANCES];

/*==========================================================================
 * FUNCTION    - af_export_eztune_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void af_export_eztune_data(stats_proc_t *sproc, af_t *af)
{
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);

  CDBG_AF("%s: Cur_Pos: %d FV: %d af_state: %d", __func__,
    af->cur_pos, (int)(stats->af_op.Focus / stats->af_op.NFocus),
    af->state);
  sproc->share.af_ext.eztune.peakpos_index = af->cur_pos;
  if (af->state == AF_INACTIVE)
    sproc->share.af_ext.eztune.tracing_index = 0;
  else if ((af->state == AF_GATHER_STATS_FINE) ||
    (af->state == AF_GATHER_STATS_COARSE)||
    (af->state == AF_FOCUSING) ||
    (af->state == AF_COLLECT_END_STAT) ||
    (af->state == AF_MAKE_DECISION) ||
    (af->state == AF_GATHER_STATS_CONT_SEARCH)) {
      CDBG_AF("%s: Reported Cur_Pos: %d FV: %d", __func__, af->cur_pos,
    (int)(stats->af_op.Focus / stats->af_op.NFocus));
    sproc->share.af_ext.eztune.tracing_stats[sproc->share.af_ext.
      eztune.tracing_index] = (int) (stats->af_op.Focus / stats->af_op.NFocus);
    sproc->share.af_ext.eztune.tracing_pos[sproc->share.af_ext.
      eztune.tracing_index] = af->cur_pos;
    sproc->share.af_ext.eztune.tracing_index++;
    if (sproc->share.af_ext.eztune.tracing_index > AF_COLLECTION_POINTS)
      sproc->share.af_ext.eztune.tracing_index = 0;
  }
} /* af_export_eztune_data */

/*==========================================================================
 * FUNCTION    - af_export_mobicat_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void af_export_mobicat_data(stats_proc_t *sproc, af_t *af)
{
  isp_stats_t *stats =
    &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  int index = sproc->share.af_ext.mobicat_af.index;
  int i = 0;

  sproc->share.af_ext.mobicat_af.focusMode = af->srch_mode;
  CDBG_AF("%s: focusMode: %d", __func__,
    sproc->share.af_ext.mobicat_af.focusMode);
  sproc->share.af_ext.mobicat_af.cafStatus = af->caf.status;
  CDBG_AF("%s: cafStatus: %d", __func__,
    sproc->share.af_ext.mobicat_af.cafStatus);

  /* Update Focus Area */
  sproc->share.af_ext.mobicat_af.numOfFocusArea =
    sproc->share.af_ext.roiInfo.num_roi;
  CDBG_AF("%s: numOfFocusArea: %d", __func__,
    sproc->share.af_ext.mobicat_af.numOfFocusArea);

  memset((roi_t *)sproc->share.af_ext.mobicat_af.focusArea,
    0, sizeof(roi_t) * MAX_ROI);
  for (i = 0; i < MAX_ROI; i++) {
    sproc->share.af_ext.mobicat_af.focusArea[i].x =
      sproc->share.af_ext.roiInfo.roi[i].x;
    sproc->share.af_ext.mobicat_af.focusArea[i].dx =
      sproc->share.af_ext.roiInfo.roi[i].dx;
    sproc->share.af_ext.mobicat_af.focusArea[i].y =
      sproc->share.af_ext.roiInfo.roi[i].y;
    sproc->share.af_ext.mobicat_af.focusArea[i].dy =
      sproc->share.af_ext.roiInfo.roi[i].dy;
    CDBG_AF("%s: Area: %d x: %d y: %d dx: %d dy: %d",
      __func__, i, sproc->share.af_ext.mobicat_af.focusArea[i].x,
      sproc->share.af_ext.mobicat_af.focusArea[i].y,
      sproc->share.af_ext.mobicat_af.focusArea[i].dx,
      sproc->share.af_ext.mobicat_af.focusArea[i].dy);
  }

  /* start/final lens position - This is used to determine
   * start and end lens position when running AF algorithm*/
  sproc->share.af_ext.mobicat_af.startLensPos =
    af->start_lens_pos;
  CDBG_AF("%s: startLensPos: %d", __func__,
    sproc->share.af_ext.mobicat_af.startLensPos);

  sproc->share.af_ext.mobicat_af.finalLensPos =
    af->final_lens_pos;
  CDBG_AF("%s: finalLensPos: %d", __func__,
    sproc->share.af_ext.mobicat_af.finalLensPos);

  if (af->state == AF_INACTIVE) {
    CDBG_AF("%s: Resetting trace index to 0", __func__);
    sproc->share.af_ext.mobicat_af.index = 0;
  }
  else {
    sproc->share.af_ext.mobicat_af.focusValue[index] =
      af->cur_focus_val;
    sproc->share.af_ext.mobicat_af.focusSteps[index] =
      af->cur_pos;
    sproc->share.af_ext.mobicat_af.index++;

    CDBG_AF("%s: index: %d focusValue: %d focusStep: %d", __func__,
      sproc->share.af_ext.mobicat_af.index - 1,
      sproc->share.af_ext.mobicat_af.focusValue[index],
      sproc->share.af_ext.mobicat_af.focusSteps[index]);

    if (sproc->share.af_ext.mobicat_af.index >=
      AF_COLLECTION_POINTS) {
      sproc->share.af_ext.mobicat_af.index = 0;
    }
  }
} /* af_export_mobicat_data */

/*==========================================================================
 * FUNCTION    - af_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
int af_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  afCtrl[index] = malloc(sizeof(af_t));
  if (!afCtrl[index])
    return -1;
  memset(afCtrl[index], 0, sizeof(af_t));
  af_load_chromatix(sproc, afCtrl[index]);
  af_init_data(sproc, afCtrl[index]);
  return 0;
} /* af_init */

/*===========================================================================
 * FUNCTION    - af_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_get_params(stats_proc_t *sproc, stats_proc_get_af_data_t *data)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  af_t *af = afCtrl[index];
  switch (data->type) {
    case AF_SHARPNESS:
      data->d.af_sharpness = af->max_focus_val;
      break;
    case AF_FOCUS_DISTANCES:
      data->d.af_focus_distance = af_get_focus_distance(sproc, af);
      break;
    case AF_CUR_LENS_POSITION:
      data->d.af_cur_lens_pos = af->cur_pos;
      break;
    case AF_STATUS:
      data->d.af_status = af->caf.status;
      break;
    case AF_FOCUS_MODE:
      data->d.af_mode = af->srch_mode;
      break;
    case AF_STATS_CONFIG_MODE:
      af_get_stats_config_mode(sproc);
      data->d.af_stats_config.mode =
        sproc->share.af_ext.af_stats_config_mode;
      data->d.af_stats_config.af_multi_roi_window =
        sproc->share.af_ext.af_multi_roi_window;
      data->d.af_stats_config.af_multi_nfocus =
        sproc->share.af_ext.af_multi_nfocus;
      memcpy( data->d.af_stats_config.region,
        sproc->share.af_ext.af_roi, sizeof(roi_t) * MAX_ROI);
      break;
    default:
      rc = -1;
      CDBG_ERROR("Invalid AF Get Params Type");
      break;
  }
  return rc;
} /* af_get_params */

/*===========================================================================
 * FUNCTION    - af_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_set_params(stats_proc_t *sproc, stats_proc_set_af_data_t *data)
{
  int rc = 0;;
  uint32_t index = sproc->handle & 0xFF;
  af_t *af = afCtrl[index];

  switch (data->type) {
    case AF_RESET_LENS:
      rc = af_reset_lens(sproc, af);
      break;
    case AF_METERING_MODE:
      af->metering_mode = data->d.af_metering_mode;
      break;
    case AF_START_PARAMS:
      rc = af_set_start_parameters(sproc, af);
      break;
    case AF_MOVE_LENS:
      rc = af_move_lens_to(sproc, af, data->d.af_step);
      break;
    case AF_RESET_MOVE_LENS_STATUS:
      sproc->share.af_ext.move_lens_status = FALSE;
      break;
    case AF_CONTINUOS_FOCUS:
      sproc->share.af_ext.cont_af_enabled = data->d.af_conti_focus;
      /* if we are disabling CAF we might be changing mode.
       * So next time when CAF starts we'll do full sweep.*/
      if (data->d.af_conti_focus == FALSE) {
        af->caf.ignore_full_sweep = FALSE;
      }
      break;
    case AF_LENS_MOVE_DONE:
      rc = af_lens_move_done(sproc, af, data->d.af_lens_move_status);
      break;
    case AF_CANCEL:
      af_done(sproc, af, CAMERA_EXIT_CB_ABORT);
      break;
    case AF_STOP:
      sproc->share.af_ext.stop_af = data->d.af_stop;
      break;
    case AF_FOCUSRECT:
      sproc->share.af_ext.cur_focusrect_value = data->d.cur_focusrect_value;
      break;
    case AF_MODE:
      sproc->share.af_ext.cur_af_mode = data->d.cur_af_mode;
      rc = af_set_focus_mode(sproc, af);
      break;
    case AF_STATE:
      sproc->share.af_ext.active = data->d.af_state;
      break;
    case AF_ROI:
      memcpy(&sproc->share.af_ext.roiInfo, &data->d.roiInfo, sizeof(stats_proc_roi_info_t));
      /* Enable a FLAG to indicate touch-AF is enabled. It's used in
        CAF mode where we won't do full sweep again when we change
        state from AF on user specified point(touch-AF) to CAF */
      /* Note: Upper layer in setParameter function calls this function
        with num_roi = 0 to reset focus area. In that case we don't need
        to set touch focus to true */
      sproc->share.af_ext.touch_af_enabled =
        (data->d.roiInfo.roi_updated && sproc->share.af_ext.roiInfo.num_roi) ?
        TRUE : FALSE;
      break;
    case AF_FACE_DETECTION_ROI:
      memcpy(&sproc->share.af_ext.fdInfo, &data->d.roiInfo, sizeof(stats_proc_roi_info_t));
      break;
    case AF_BESTSHOT:
      rc = af_set_bestshot_mode(sproc, af, data->d.bestshot_mode);
      break;
    case AF_EZ_DISABLE:
      af->eztune.disable = data->d.ez_disable;
      break;
    case AF_LOCK_CAF:
      af->caf.locked = data->d.af_lock_caf;
      break;
    case AF_SEND_CAF_DONE_EVT_LTR:
      af->caf.send_event_later = data->d.af_send_evt_ltr;
      break;
    default:
      CDBG_ERROR("Invalid AF Set Params Type");
      rc = -1;
  }
  if (rc < 0)
    CDBG_ERROR("%s:ERROR INVAL parm for set type %d", __func__, data->type);
  return rc;
} /* af_set_params */

/*===========================================================================
 * FUNCTION    - af_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
int af_process(stats_proc_t *sproc)
{
  int rc = -1;
  uint32_t index = sproc->handle & 0xFF;
  af_t *af = afCtrl[index];

  if (af->eztune.disable && sproc->share.eztune_enable)
    return 0;

  switch (sproc->input.mctl_info.opt_state) {
    case STATS_PROC_STATE_PREVIEW:
    case STATS_PROC_STATE_CAMCORDER:
      rc = af_run_algorithm(sproc, af);
      break;
    default:
      CDBG_ERROR("%s: %d: Failed: af_algo_preview\n", __func__, __LINE__);
  }
  if (sproc->share.eztune_enable){
    CDBG_AF("%s: frame_delay: %d", __func__, af->frame_delay);
    /* if frame_delay is equal or more than 0, we won't report
     to eztune.*/
    if (af->frame_delay >= 0) {
      af_export_eztune_data(sproc, af);
    }
  }
  if (sproc->share.mobicat_enable) {
    af_export_mobicat_data(sproc, af);
  }
  return rc;
} /* af_process */

/*===========================================================================
 * FUNCTION    - af_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void af_deinit(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index < MAX_INSTANCES) {
    if (afCtrl[index]){
      free(afCtrl[index]);
      afCtrl[index] = NULL;
    }
  }
} /* af_deinit */

/*===========================================================================
 * FUNCTION    - af_chromatix_reload -
 *
 * DESCRIPTION:
 *==========================================================================*/
void af_chromatix_reload(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  af_load_chromatix(sproc, afCtrl[index]);
} /* af_chromatix_reload */

