/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "stats_proc.h"
#include "aec.h"

static aec_t *aecCtrl[MAX_INSTANCES];

/*==========================================================================
 * FUNCTION    - aec_process_eztune_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_process_eztune_data(stats_proc_t *sproc, aec_t *aec)
{
  if (!aec->eztune.lock_output) { /* Save output for EZ Tune*/
    aec->eztune.stored_line_count = sproc->share.aec_ext.cur_line_cnt;
    aec->eztune.stored_gain = sproc->share.aec_ext.cur_real_gain;
  } else {
    sproc->share.aec_ext.cur_line_cnt = aec->eztune.stored_line_count;
    sproc->share.aec_ext.cur_real_gain = aec->eztune.stored_gain;
    if (aec->eztune.force_exp) {
      sproc->share.aec_ext.cur_line_cnt = aec->eztune.force_linecount;
      sproc->share.aec_ext.cur_real_gain = aec->eztune.force_gain;
    }
    if (aec->eztune.force_snapshot_exp) {
      sproc->share.aec_ext.snap.line_count = aec->eztune.force_snap_linecount;
      sproc->share.aec_ext.snap.real_gain = aec->eztune.force_snap_gain;
    }
  }
} /* aec_process_eztune_data */

/*==========================================================================
 * FUNCTION    - aec_process_mobicat_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_process_mobicat_data(stats_proc_t *sproc, aec_t *aec)
{
  sproc->share.aec_ext.mobicat_aec.expIndex =
    sproc->share.aec_ext.exp_index;
  sproc->share.aec_ext.mobicat_aec.lightCond =
    aec_get_flash_for_snapshot(sproc, aec);
  sproc->share.aec_ext.mobicat_aec.expMode =
    sproc->share.aec_ext.metering_type;

  if (sproc->input.mctl_info.opt_mode == STATS_PROC_MODE_2D_ZSL) {
    sproc->share.aec_ext.mobicat_aec.analogGain =
      sproc->share.aec_ext.cur_real_gain;
    sproc->share.aec_ext.mobicat_aec.expTime =
      aec_get_preview_exp_time(sproc, aec);
  }
  else {
    sproc->share.aec_ext.mobicat_aec.analogGain =
      sproc->share.aec_ext.snap.real_gain;
    sproc->share.aec_ext.mobicat_aec.expTime =
      sproc->share.aec_ext.snap.exp_time;
  }

  sproc->share.aec_ext.mobicat_aec.expBias =
    aec->exp_comp_val;
  CDBG_AEC("%s: expIndex: %d lightCond: %d expMode: %d analogGain: %f"
    "expTime: %f expBias: %d", __func__,
    sproc->share.aec_ext.mobicat_aec.expIndex,
    sproc->share.aec_ext.mobicat_aec.lightCond,
    sproc->share.aec_ext.mobicat_aec.expMode,
    sproc->share.aec_ext.mobicat_aec.analogGain,
    sproc->share.aec_ext.mobicat_aec.expTime,
    sproc->share.aec_ext.mobicat_aec.expBias);
} /* aec_process_mobicat_data */

/*==========================================================================
 * FUNCTION    - aec_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  aecCtrl[index] = malloc(sizeof(aec_t));
  if (!aecCtrl[index])
    return -1;
  memset(aecCtrl[index], 0, sizeof(aec_t));

  aec_load_chromatix(sproc, aecCtrl[index]);
  aec_init_data(sproc, aecCtrl[index]);
  return 0;
} /* aec_init */

/*===========================================================================
 * FUNCTION    - aec_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int aec_get_params(stats_proc_t *sproc, stats_proc_get_aec_data_t *data)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  aec_t *aec = aecCtrl[index];

  switch (data->type) {
    case  AEC_OVER_EXP_STATE:
      data->d.aec_over_exposure = aec_get_LED_over_exp_check(sproc, aec);
      break;
    case  AEC_LED_SETTLE_CNT:
      data->d.aec_led_settle_cnt = aec_get_settled_cnt(sproc, aec);
      break;
    case  AEC_LED_STROBE:
      data->d.use_strobe = sproc->share.aec_ext.use_strobe;
      break;
    case  AEC_QUERY_FLASH_FOR_SNAPSHOT:
      data->d.query_flash_for_snap = aec_get_flash_for_snapshot(sproc, aec);
      break;
    case  AEC_AFR_PREVIEW_FPS:
      data->d.afr_preview_fps = aec_get_preview_fps(sproc, aec);
      break;
    case  AEC_PREVIEW_EXP_TIME:
      data->d.aec_preview_expotime = aec_get_preview_exp_time(sproc, aec);
      break;
    case AEC_FPS_MODE_G:
      data->d.fps_mode = sproc->share.aec_ext.afr_enable ? AEC_FPS_MODE_AUTO :
        AEC_FPS_MODE_FIXED;
      break;
    case AEC_PARMS:
      data->d.aec_parms.cur_luma = sproc->share.aec_ext.cur_luma;
      data->d.aec_parms.cur_real_gain = sproc->share.aec_ext.cur_real_gain;
      data->d.aec_parms.exp_index = sproc->share.aec_ext.exp_index;
      data->d.aec_parms.exp_tbl_val = sproc->share.aec_ext.exp_tbl_val;
      data->d.aec_parms.lux_idx = sproc->share.aec_ext.lux_idx;
      data->d.aec_parms.snapshot_real_gain = sproc->share.aec_ext.snap.real_gain;
      data->d.aec_parms.target_luma = sproc->share.aec_ext.target_luma;
      break;
    case AEC_EXPOSURE_PARAMS:
      CDBG_AEC("%s:%d] AEC_EXPOSURE_PARAMS snapshot %d", __func__, __LINE__,
        data->d.exp_params.is_snapshot);
      if (data->d.exp_params.is_snapshot) {
        data->d.exp_params.gain = sproc->share.aec_ext.snap.real_gain;
        data->d.exp_params.linecount = sproc->share.aec_ext.snap.line_count;
      } else {
        data->d.exp_params.gain = sproc->share.aec_ext.cur_real_gain;
        data->d.exp_params.linecount = sproc->share.aec_ext.cur_line_cnt;
      }
      data->d.exp_params.current_luma = sproc->share.aec_ext.cur_luma;
      data->d.exp_params.luma_target = sproc->share.aec_ext.target_luma;
      break;
    case AEC_FLASH_DATA: {
      if (sproc->share.aec_ext.strobe_cfg_st == STROBE_PRE_FIRED)
        data->d.flash_parms.flash_mode = CAMERA_FLASH_STROBE;
      else if (sproc->share.aec_ext.use_led_estimation)
        data->d.flash_parms.flash_mode = CAMERA_FLASH_LED;
      else
        data->d.flash_parms.flash_mode = CAMERA_FLASH_NONE;

      data->d.flash_parms.sensitivity_led_hi  = sproc->share.aec_ext.flash_si.high;
      data->d.flash_parms.sensitivity_led_low = sproc->share.aec_ext.flash_si.low;
      data->d.flash_parms.sensitivity_led_off = sproc->share.aec_ext.flash_si.off;
      data->d.flash_parms.strobe_duration     = sproc->share.aec_ext.stobe_len;
      break;
    }
    default:
      rc = -1;
      CDBG_ERROR("Invalid AEC Get Params Type");
      break;
  }
  return rc;
} /* aec_get_params */

/*===========================================================================
 * FUNCTION    - aec_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int aec_set_params(stats_proc_t *sproc, stats_proc_set_aec_data_t *data)
{
  int rc = 0;
  int roi_updated = 0;
  uint32_t index = sproc->handle & 0xFF;
  aec_t *aec = aecCtrl[index];
  CDBG_AEC("aec_set_params type =%d", data->type);

  switch (data->type) {
    case AEC_METERING_MODE:
      rc = aec_set_exp_metering_mode(sproc, aec, data->d.aec_metering);
      break;
    case  AEC_ISO_MODE:
      rc = aec_set_iso_mode(sproc, aec, data->d.aec_iso_mode);
      break;
    case  AEC_ANTIBANDING:
      rc = aec_set_antibanding(sproc, aec, data->d.aec_atb);
      break;
    case  AEC_ANTIBANDING_STATUS:
      rc = aec_set_antibanding_status(sproc, aec, data->d.aec_atb);
      break;
    case  AEC_BRIGHTNESS_LVL:
      rc = aec_set_brightness_level(sproc, aec, data->d.aec_brightness);
      break;
    case  AEC_EXP_COMPENSATION:
      rc = aec_set_exposure_compensation(sproc, aec, data->d.aec_exp_comp);
      break;
    case  AEC_FPS_MODE:
      rc = aec_set_fps_mode(sproc, aec, data->d.aec_fps_mode);
      break;
    case AEC_PARM_FPS:
      rc = aec_set_parm_fps(sproc, aec, data->d.aec_fps);
      break;
    case  AEC_HJR:
      rc = aec_set_hjr(sproc, aec);
      break;
    case  AEC_HJR_AF:
      /*todo ask ruben about rename */
      rc = aec_set_for_hjr_af(sproc, aec, data->d.aec_af_hjr);
      break;
    case  AEC_SET_ROI:
      rc =  aec_set_ROI(sproc, aec, data->d.aec_roi);
      break;
    case  AEC_SET_FD_ROI:
      if (sproc->share.fd_roi.frm_id != data->d.fd_roi.frm_id)
        roi_updated = 1;
      else
        roi_updated = 0;
      memcpy(&sproc->share.fd_roi, &data->d.fd_roi,
        sizeof(stats_proc_roi_info_t));
      sproc->share.fd_roi.roi_updated = roi_updated;
      break;
    case  AEC_SET_MTR_AREA:
      memcpy(&aec->aec_mtr_area, &data->d.mtr_area,
        sizeof(stats_proc_mtr_area_t));
      break;
    case  AEC_LED_RESET:
      rc = aec_reset_LED(sproc, aec);
      break;
    case  AEC_STROBE_MODE:
      rc = aec_set_strobe_mode(sproc, aec, data->d.aec_strobe_mode);
      break;
    case  AEC_PREPARE_FOR_SNAPSHOT:
      rc = aec_prepare_snapshot(sproc, aec);
      break;
    case  AEC_REDEYE_REDUCTION_MODE:
      aec->redeye_reduction = data->d.aec_redeye_reduction_mode;
      break;
    case AEC_SOF:
      aec->sensor_update_ok = data->d.aec_sensor_update_ok;
      break;
    case AEC_BESTSHOT:
      rc = aec_set_bestshot_mode(sproc, aec, data->d.bestshot_mode);
      break;
    case AEC_STROBE_CFG_ST:
      sproc->share.aec_ext.strobe_cfg_st = data->d.aec_strobe_cfg_st;
      break;
    case AEC_EZ_DISABLE:
      aec->aec_lock = data->d.ez_disable;
      break;
    case AEC_EZ_LOCK_OUTPUT:
      aec->eztune.lock_output = data->d.ez_lock_output;
      break;
    case AEC_EZ_FORCE_EXP:
      aec->eztune.force_exp = data->d.ez_force_exp;
      break;
    case AEC_EZ_FORCE_LINECOUNT:
      aec->eztune.force_linecount = data->d.ez_force_linecount;
      break;
    case AEC_EZ_FORCE_GAIN:
      aec->eztune.force_gain = data->d.ez_force_gain;
      break;
    case AEC_EZ_TEST_ENABLE:
      aec->eztune.test_enable = data->d.ez_test_enable;
      break;
    case AEC_EZ_TEST_ROI:
      aec->eztune.test_roi = data->d.ez_test_roi;
      break;
    case AEC_EZ_TEST_MOTION:
      aec->eztune.test_motion = data->d.ez_test_motion;
      break;
    case AEC_EZ_FORCE_SNAPSHOT:
      aec->eztune.force_snapshot_exp = data->d.ez_force_snapshot;
      break;
    case AEC_EZ_FORCE_SNAP_LINECOUNT:
      aec->eztune.force_snap_linecount = data->d.ez_force_snap_linecount;
      break;
    case AEC_EZ_FORCE_SNAP_GAIN:
      aec->eztune.force_snap_gain = data->d.ez_force_snap_gain;
      break;
    case AEC_SET_LOCK:
      aec->aec_lock = data->d.force_aec_lock;
      break;
    case AEC_SET_SENSITIVITY_RATIO:
      if(data->d.sensitivity_ratio > 0)
        sproc->share.aec_ext.prev_sensitivity *=  data->d.sensitivity_ratio;
      break;
    default:
      rc = -1;
      CDBG_ERROR("Invalid AEC Set Params Type");
  }
  if (rc < 0)
    CDBG_AEC("%s:ERROR INVAL parm for set type %d", __func__, data->type);

  return rc;
} /* aec_set_params */

/*===========================================================================
 * FUNCTION    - aec_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
int aec_process(stats_proc_t *sproc)
{
  int rc = -1;
  uint32_t index = sproc->handle & 0xFF;
  aec_t *aec = aecCtrl[index];

  switch (sproc->input.mctl_info.opt_state) {
    case STATS_PROC_STATE_PREVIEW: /* this state covers ZSL as well */
      if ((rc = aec_process_preview_and_video(sproc, aec)) == -1)
        CDBG_AEC("%s: aec_algo_preview: FAILED....", __func__);
      break;
    case STATS_PROC_STATE_CAMCORDER:
      if ((rc = aec_process_preview_and_video(sproc, aec)) == -1)
        CDBG_AEC("%s: aec_algo_video: FAILED....", __func__);
      break;
    case  STATS_PROC_STATE_SNAPSHOT:
      if (!(aec->eztune.force_snapshot_exp && sproc->share.eztune_enable)) {
        if ((rc = aec_process_snapshot(sproc, aec)) == -1)
          CDBG_AEC("%s: aec_process_snapshot: FAILED....", __func__);
      } else {
        rc = 0;
      }
      break;
    default:
      CDBG_ERROR("Invalid AEC Process Type");
      return -1;
      break;
  }
  if (sproc->share.eztune_enable)
    aec_process_eztune_data(sproc, aec);

  if (sproc->share.mobicat_enable)
    aec_process_mobicat_data(sproc, aec);

  if (rc == 0) {
    if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_PREVIEW ||
      sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER ||
      sproc->input.mctl_info.opt_mode == STATS_PROC_MODE_2D_ZSL)
      aec_get_settled_cnt(sproc, aec);

    sproc->share.aec_ext.target_luma = aec->luma.comp_target;

    if ((sproc->share.aec_ext.cur_luma < (aec->luma.comp_target + 2 *
      aec->luma.tolerance)) && (sproc->share.aec_ext.cur_luma >
      (aec->luma.comp_target - 2 * aec->luma.tolerance)))
      sproc->share.aec_ext.aec_settled = TRUE;
    else
      sproc->share.aec_ext.aec_settled = FALSE;

    /* aec cannot converge if its too dark. We still need to report
       its settled because aec won't increase sensitivity anymore */
    if ((sproc->share.aec_ext.cur_luma <
      (aec->luma.comp_target - aec->luma.tolerance)) &&
      (sproc->share.aec_ext.exp_index >= (int)sproc->share.aec_ext.exp_tbl_val -1)){
      sproc->share.aec_ext.aec_settled = TRUE;
    }
  }
  return rc;
} /* aec_process */

/*===========================================================================
 * FUNCTION    - aec_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void aec_deinit(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index < MAX_INSTANCES) {
    if (aecCtrl[index]){
      if(aecCtrl[index]->exp_tbl_ptr) {
        free(aecCtrl[index]->exp_tbl_ptr);
        aecCtrl[index]->exp_tbl_ptr = NULL;
      }
      free(aecCtrl[index]);
      aecCtrl[index] = NULL;
    }
  }
} /* aec_deinit */

/*===========================================================================
 * FUNCTION    - aec_chromatix_reload -
 *
 * DESCRIPTION:
 *==========================================================================*/
void aec_chromatix_reload(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  aec_load_chromatix(sproc, aecCtrl[index]);
} /* aec_chromatix_reload */

