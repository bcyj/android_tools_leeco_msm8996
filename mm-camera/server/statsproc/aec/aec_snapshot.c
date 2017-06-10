/**********************************************************************
  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <math.h>
#include "stats_proc.h"
#include "aec.h"

/*==========================================================================
 * FUNCTION    - aec_config_snapshot_luma_target -
 *
 * DESCRIPTION: adjust luma target depending on count of high luma regions
 *=========================================================================*/
static void aec_config_snapshot_luma_target(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t gain_on_indoor_index;
  float real_gain_on_indoor_index;
  uint32_t *luma_target = &(sproc->share.aec_ext.snap.luma_target);

  /* adjust snapshot luma target */
  if (sproc->share.aec_ext.high_luma_region_count < 2)
    *luma_target = aec->luma.comp_target; /* no compensation */
  else if (sproc->share.aec_ext.high_luma_region_count >= 8)
    *luma_target = aec->luma.comp_target - 8;
  else if (sproc->share.aec_ext.high_luma_region_count >= 4)
    *luma_target = aec->luma.comp_target - 5;
  else if (sproc->share.aec_ext.high_luma_region_count >= 2)
    *luma_target = aec->luma.comp_target - 3;

  *luma_target = aec->luma.comp_target;
} /* aec_config_snapshot_luma_target */

/*==========================================================================
 * FUNCTION    - aec_adjust_exp_settings_for_strobe -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_adjust_exp_settings_for_strobe(stats_proc_t *sproc, aec_t *aec)
{
  int max_strobe_length = 0, min_strb_len, new_line_count;
  float strobe_time_extend = 1.0, gain_reduction = 1.0, intersect_time_gain;
  float new_gain, min_gain_allowed;
  float snapshot_full_frame_exposure_time, preview_exposuretime;

  CDBG_AEC("%s: STRB ENBL %d, STRB used %d", __func__, sproc->input.chromatix->
    AEC_strobe_flash.strobe_enable, sproc->share.aec_ext.use_strobe);
  if (sproc->input.chromatix->AEC_strobe_flash.strobe_enable) {
    min_strb_len = sproc->input.chromatix->AEC_strobe_flash.strobe_min_time;
    intersect_time_gain = sproc->input.chromatix->
      AEC_strobe_flash.intersect_time_gain;
    max_strobe_length = min_strb_len * intersect_time_gain + (sproc->input.
      chromatix->AEC_strobe_flash.max_strobe_gain - intersect_time_gain) *
      sproc->input.chromatix->AEC_strobe_flash.
      post_intersect_strobe_flux_rate_increase;

    sproc->share.aec_ext.stobe_len = min_strb_len;
    CDBG_AEC("%s: strb len %d", __func__, sproc->share.aec_ext.stobe_len);
    if (aec->flash.strb_est.exp_index_adjustment > 0) {
      if (aec->flash.strb_est.strb_gain <= intersect_time_gain) {
        strobe_time_extend = aec->flash.strb_est.strb_gain;
        /* assuming linear */
        if ((min_strb_len * strobe_time_extend) > max_strobe_length)
          sproc->share.aec_ext.stobe_len = max_strobe_length;
        else
          sproc->share.aec_ext.stobe_len = min_strb_len * strobe_time_extend;
      } else if (aec->flash.strb_est.strb_gain >
        intersect_time_gain) {
        float extra_gain_offset;
        strobe_time_extend = intersect_time_gain;
        extra_gain_offset = (aec->flash.strb_est.strb_gain -
          intersect_time_gain) * sproc->input.chromatix->AEC_strobe_flash.
          post_intersect_strobe_flux_rate_increase;
        sproc->share.aec_ext.stobe_len = min_strb_len * strobe_time_extend +
          extra_gain_offset;
      }
      CDBG_AEC("%s: strobe len %d", __func__, sproc->share.aec_ext.stobe_len);
    } else /* can't reduce exp time, need full frame.*/
      gain_reduction = pow(1.03, aec->flash.strb_est.
        exp_index_adjustment);

    /* for digital gain only, may need to add for register gain */
    sproc->share.aec_ext.cur_real_gain = sproc->share.aec_ext.cur_real_gain * gain_reduction;
    if (sproc->share.aec_ext.cur_real_gain < 1.0) {
      /* most likely will overexpose since cannot make strobe shorter
         nor reduce gain further. */
      sproc->share.aec_ext.cur_real_gain = 1.0;
      CDBG_AEC("STRB FLSH likely to overexpose, gain can't be reduced");
    }
    /* Before completion make sure we force expo time at >= 1/snap_fps */
    preview_exposuretime = (float)sproc->input.sensor_info.preview_fps / 256.0;
    preview_exposuretime = (float)(sproc->share.aec_ext.cur_line_cnt) /
      (preview_exposuretime * sproc->input.sensor_info.preview_linesPerFrame);

    snapshot_full_frame_exposure_time = 256.0 /
      ((float)sproc->input.sensor_info.snapshot_fps);
    CDBG_AEC("%s: preview_exp_time %f, snap_full_frm_exp_time %f", __func__,
      preview_exposuretime, snapshot_full_frame_exposure_time);
    if (preview_exposuretime < snapshot_full_frame_exposure_time) {
      /* increase line_count to achieve snapshot_full_frame_exposure_time
       * and decrease gain but not less than one. */
      min_gain_allowed = (float) aec->exp_tbl_ptr[0].gain / 256.0;
      new_line_count = (uint32_t) (snapshot_full_frame_exposure_time * ((float)
        sproc->input.sensor_info.preview_fps / AEC_Q8) *
        sproc->input.sensor_info.preview_linesPerFrame);
      new_gain = sproc->share.aec_ext.cur_real_gain *
        sproc->share.aec_ext.cur_line_cnt / new_line_count;
      if (new_gain < min_gain_allowed)
        new_gain = min_gain_allowed;

      CDBG_AEC("%s:cur_line_cnt %d, new_line_cnt %d,  cur_real_gain %f, "
        "new_gain %f", __func__, sproc->share.aec_ext.cur_line_cnt,
        new_line_count, sproc->share.aec_ext.cur_real_gain, new_gain);
      sproc->share.aec_ext.cur_line_cnt = new_line_count;
      sproc->share.aec_ext.cur_real_gain = new_gain;
    }
  }
  sproc->share.aec_ext.use_strobe = FALSE;
} /* aec_adjust_exp_settings_for_strobe */

/*==========================================================================
 * FUNCTION    - aec_restore_exp_level_prior_to_led_fire -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_restore_exp_level_prior_to_led_fire(stats_proc_t *sproc,
  aec_t *aec)
{
  if (sproc->share.aec_ext.use_led_estimation) {
    CDBG_HIGH("%s: real_gain %f, linecount %d", __func__, sproc->share.
      aec_ext.snap.real_gain, sproc->share.aec_ext.snap.line_count);
    CDBG_HIGH("%s: Restore exp index from %d to %d", __func__,
      sproc->share.aec_ext.exp_index, aec->flash.led_est.index_off);

    sproc->share.aec_ext.exp_index = aec->flash.led_est.index_off;
    sproc->share.aec_ext.cur_real_gain = (float)aec->exp_tbl_ptr[sproc->
      share.aec_ext.exp_index].gain / 256.0;
    sproc->share.aec_ext.cur_line_cnt =
      aec->exp_tbl_ptr[sproc->share.aec_ext.exp_index].line_count;

    sproc->share.aec_ext.cur_luma = aec->flash.led_est.luma_off;
    sproc->share.prev_exp_index = aec->flash.led_est.index_off;
    sproc->share.aec_ext.lux_idx = aec->flash.led_est.lux_idx_off;
  }
} /* aec_restore_exp_level_prior_to_led_fire */

/*==========================================================================
 * FUNCTION    - aec_prepare_snapshot -
 *
 * DESCRIPTION:  prepare for snapshot
 *=========================================================================*/
int aec_prepare_snapshot(stats_proc_t *sproc, aec_t *aec)
{
  if (aec_strobe_enable_check() && (aec->flash.strobe_mode ==
    STROBE_FLASH_MODE_ON || (aec->flash.strobe_mode ==
    STROBE_FLASH_MODE_AUTO && aec_use_strobe(sproc, aec)))) {
    if (sproc->share.aec_ext.strobe_cfg_st != STROBE_PRE_FIRED) {
      aec_strobe_flash_store_est(sproc, aec, STROBE_OFF);
      aec->snap_est_lock = TRUE;

      CDBG_AEC("%s: strobe_chrg_ready =%d\n", __func__,
        sproc->input.flash_info.strobe_chrg_ready);
      if (sproc->input.flash_info.strobe_chrg_ready) {
        aec->flash.strb_int_state = STROBE_PRE_FLASH;
        /* adjust frame rate to full frame for pre-flash */
        aec_set_full_frame_exp(sproc, aec);
      } else {
        aec->flash.strb_frame_cnt_wait = 0;
        aec->flash.strb_int_state = STROBE_CHECK_READY;
      }
      aec->prev_bias_table = aec->bias_table;
      aec->prev_metering_type =
        sproc->share.aec_ext.metering_type;
      sproc->share.aec_ext.aec_flash_settled = AEC_FLASH_SETTLE_WAIT;
    } else
      sproc->share.aec_ext.aec_flash_settled = AEC_SETTLED;
  } else if (sproc->input.flash_info.led_mode == LED_MODE_ON || (sproc->input.flash_info.
    led_mode == LED_MODE_AUTO && aec_get_strobe(sproc, aec))) {
    /* AEC needs to save current stats before LED is enabled */
    aec_store_LED_est_stats(sproc, aec, MSM_CAMERA_LED_OFF);
    sproc->share.aec_ext.led_state = MSM_CAMERA_LED_LOW;
    if(sproc->input.mctl_info.numRegions <= 256)
      memcpy(&aec->led_off_SY[0],
        (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY,
         sizeof(uint32_t) * sproc->input.mctl_info.numRegions);
    aec->prev_bias_table = aec->bias_table;
    aec->prev_metering_type =
      sproc->share.aec_ext.metering_type;
    aec->next_update_frame_cnt           = 0;
    /* Update AEC state as LED is now on */
    aec->frame_in_current_fps            = 0;
    sproc->share.aec_ext.led_frame_skip_cnt  = 0;

    sproc->share.aec_ext.aec_flash_settled = AEC_FLASH_SETTLE_WAIT;
  } else if (sproc->share.luma_settled_cnt <
    LUMA_SETTLED_BEFORE_AF_CNT && sproc->share.aec_ext.exp_index <
    (int) sproc->share.aec_ext.exp_tbl_val - 1) {
    sproc->share.aec_ext.aec_flash_settled = AEC_SETTLE_WAIT;
  } else /* If exposure needs more time ... */
    sproc->share.aec_ext.aec_flash_settled = AEC_SETTLED;

  if (sproc->share.aec_ext.aec_flash_settled == AEC_SETTLED)
    return 0;
  else
    return -1;
} /* aec_prepare_snapshot */

/*==========================================================================
 * FUNCTION    - aec_process_snapshot -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_process_snapshot(stats_proc_t *sproc, aec_t *aec)
{
  aec_hjr_data_t *hjr_data = &(aec->hjr_data);
  stats_proc_aec_snapshot_t *snap = &(sproc->share.aec_ext.snap);
  stats_proc_sensor_info_t *sinfo = &(sproc->input.sensor_info);
  snapshot_exposure_type *snap_lut =&(sproc->input.chromatix->aec_exp_stretch);
  camera_iso_mode_type cur_iso;

  /* todo hard code redeye_led_duration will get from 208 chromatix*/
  int redeye_led_duration = 200, redeye_led_wait = 200; /*in ms*/

  float preview_exp_time, snap_exp_time, snap_real_gain;
  float gain_trade_off = 0.0, max_gain_allowed, min_gain_allowed;
  float snapshot_max_exp_time, iso_100_real_gain;
  uint32_t preview_linesPerFrame, iso_multiplier = 1, cur_snap_fps_Q8;
  int snap_fps_divider = 1, i, current_line_count;

  memset(snap, 0, sizeof(stats_proc_aec_snapshot_t));
  max_gain_allowed = (float) aec->exp_tbl_ptr[
    sproc->share.aec_ext.exp_tbl_val - 1].gain / 256.0;
  min_gain_allowed = (float) aec->exp_tbl_ptr[0].gain / 256.0;

  CDBG_HIGH("%s: iso_mode %d", __func__, sproc->share.aec_ext.iso);

  aec_config_snapshot_luma_target(sproc, aec);

  if (sproc->share.aec_ext.use_strobe)
    aec_adjust_exp_settings_for_strobe(sproc, aec);
  else if (sproc->share.aec_ext.use_led_estimation)
    aec_adjust_exp_settings_for_led(sproc, aec);
  current_line_count = sproc->share.aec_ext.cur_line_cnt;
  if (sproc->share.aec_ext.iso == CAMERA_ISO_AUTO) {
    snap_real_gain = sproc->share.aec_ext.cur_real_gain;
    snap->auto_mode = TRUE;
    CDBG_AEC("%s: apply_motion_iso %d, curr_real_gain %f, snap_real_gain %f",
      __func__, aec->mtn.apply, sproc->share.aec_ext.cur_real_gain,
      aec->mtn.iso_snap_real_gain);
    if (aec->mtn.apply && sproc->input.chromatix->aec_motion_iso_snapshot.
      motion_iso_enable && (sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF ||
      sproc->input.flash_info.led_mode == LED_MODE_TORCH)) {
      snap_real_gain = aec->mtn.iso_snap_real_gain;
      snap->auto_mode = FALSE;
    } else
      CDBG_AEC("%s: No motion iso, real_gain %f", __func__, snap_real_gain);
  } else if (sproc->share.aec_ext.iso == CAMERA_ISO_DEBLUR) {
    snap->auto_mode = FALSE;
    aec_set_hjr(sproc, aec);
    snap_real_gain = hjr_data->new_sensor_gain;
    current_line_count = hjr_data->new_line_count;
  } else {
    iso_100_real_gain = sproc->input.chromatix->ISO100_gain;
    cur_iso = sproc->share.aec_ext.iso;
    if (cur_iso < CAMERA_ISO_100)
      cur_iso = CAMERA_ISO_100;
    else if (cur_iso >= CAMERA_ISO_MAX)
      cur_iso = CAMERA_ISO_MAX -1;

    iso_multiplier = cur_iso - CAMERA_ISO_100;
    iso_multiplier = (1 << iso_multiplier);
    snap->auto_mode = FALSE;
    /* To make sure we are using the correct/available sensor gain due to
     * only discrete gains available from sensor. */
    CDBG_AEC("%s:ISO %d, muliplier %d", __func__, cur_iso, iso_multiplier);
    snap_real_gain = iso_100_real_gain * iso_multiplier;
    /* Need to use VFE's digital gain option */
    snap->real_gain = snap_real_gain;   /* real gains */
  }
  CDBG_AEC("%s: snap_real_gain %f, output.snapshot_gain %f\n",
    __func__, snap_real_gain, snap->real_gain);

  /* Step 1 Calculate Preview ExposureTime */
  preview_linesPerFrame = sinfo->preview_linesPerFrame;
  preview_exp_time = (float) sinfo->preview_fps / AEC_Q8;
  preview_exp_time =  (sproc->share.aec_ext.cur_line_cnt) /
    (preview_exp_time * preview_linesPerFrame);

  /* Step 2 Calculate Snapshot Exposure Time */
  snap_exp_time = (sproc->share.aec_ext.cur_real_gain *
    preview_exp_time) / snap_real_gain;

  snapshot_max_exp_time = snap_exp_time;
  if ((sproc->share.aec_ext.iso == CAMERA_ISO_AUTO) && (!aec->mtn.apply)) {
    /******************* LUT ********************/
    if (snap_lut->enable) {
      int last_entry = snap_lut->valid_entries -1;
      /* Calcluate Gain Trade Off */
      if (sproc->share.aec_ext.lux_idx <= snap_lut->
        snapshot_ae_table[0].lux_index) {
        gain_trade_off = snap_lut->snapshot_ae_table[0].gain_trade_off;
        snapshot_max_exp_time = snap_lut->snapshot_ae_table[0].max_exp_time;
      } else if ((last_entry >= 0) && (sproc->share.aec_ext.lux_idx >
        snap_lut->snapshot_ae_table[last_entry].lux_index)) {
        gain_trade_off = snap_lut->snapshot_ae_table[last_entry].gain_trade_off;
        snapshot_max_exp_time =
          snap_lut->snapshot_ae_table[last_entry].max_exp_time;
      } else {
        for (i= 1; i <= last_entry; i++) {
          if (sproc->share.aec_ext.lux_idx <=
            snap_lut->snapshot_ae_table[i].lux_index) {
            float slope = (snap_lut->snapshot_ae_table[i].gain_trade_off -
              snap_lut->snapshot_ae_table[i-1].gain_trade_off) /
              (float)(snap_lut->snapshot_ae_table[i].lux_index -
              snap_lut->snapshot_ae_table[i-1].lux_index);
            gain_trade_off = slope * (float)(sproc->share.aec_ext.lux_idx -
              snap_lut->snapshot_ae_table[i-1].lux_index) +
              snap_lut->snapshot_ae_table[i-1].gain_trade_off;
            /* Interpolate the maximum snapshot exp time from the snapshot LUT */
            slope = (snap_lut->snapshot_ae_table[i].max_exp_time -
              snap_lut->snapshot_ae_table[i-1].max_exp_time) /
              (float)(snap_lut->snapshot_ae_table[i].lux_index -
              snap_lut->snapshot_ae_table[i-1].lux_index);

            snapshot_max_exp_time = slope * (float)(sproc->share.aec_ext.
              lux_idx - snap_lut->snapshot_ae_table[i-1].lux_index) +
              snap_lut->snapshot_ae_table[i-1].max_exp_time;
            break;
          }
        }
      }
      if (snap_exp_time >= snapshot_max_exp_time)
        snap_exp_time = snapshot_max_exp_time ;

      if (gain_trade_off != 0.0f) {
        /* If luma_target not reached use exposure stretch */
        if (snap_lut[0].exposure_stretch_enable) {
          if (sproc->share.aec_ext.cur_luma < snap->luma_target - 4)
            snap_exp_time = snap_exp_time * snap->luma_target /
              sproc->share.aec_ext.cur_luma;
        }
        if (snap_real_gain > 2 *min_gain_allowed) {
          snap_real_gain = snap_real_gain * gain_trade_off;
          snap_exp_time = (snap_exp_time / gain_trade_off);
        }
        if (snap_real_gain < min_gain_allowed) {
          gain_trade_off = min_gain_allowed / snap_real_gain;
          snap_real_gain = min_gain_allowed;
          snap_exp_time = (snap_exp_time * gain_trade_off);
        }
        CDBG_AEC("%s: LUT: real_gain %f", __func__, snap_real_gain);
      }
    } else {
      /* Check if gain is above max allowed gain */
      if (snap_real_gain > max_gain_allowed) {
        /* If so limit the gain and increase the exposure time. */
        snap_exp_time = snap_exp_time * snap_real_gain /
          max_gain_allowed;
        snap_real_gain = max_gain_allowed;

        /* Check if new exposure exceed max allowed exposure time */
        if (snap_exp_time >=
          sproc->input.chromatix->max_snapshot_exposure_time_allowed)
          snap_exp_time =
            sproc->input.chromatix->max_snapshot_exposure_time_allowed;
      }
      /* Check if exposure exceed max allowed exposure time */
      else if (snap_exp_time >=
        sproc->input.chromatix->max_snapshot_exposure_time_allowed) {
        /* If so limit the exposure time and increase the gain. */
        snap_real_gain = snap_real_gain * snap_exp_time /
          sproc->input.chromatix->max_snapshot_exposure_time_allowed;
        snap_exp_time =
          sproc->input.chromatix->max_snapshot_exposure_time_allowed;

        /* Check if new gain exceeds max allowed gain */
        if (snap_real_gain > max_gain_allowed)
          snap_real_gain = max_gain_allowed;
      }
      /* Check if gain is are below min allowed gain */
      else if (snap_real_gain < min_gain_allowed) {
        /* If so limit the gain and increase the exposure time. */
        snap_exp_time = snap_exp_time * snap_real_gain /
          min_gain_allowed;
        snap_real_gain = min_gain_allowed;
      }
    }
  }
  /* prevents driver from modifying the exposure settings. */
  if (sproc->input.flash_info.led_state != LED_MODE_OFF &&
    sproc->input.flash_info.led_state != LED_MODE_TORCH &&
    sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF)
    snap->auto_mode = FALSE;

  CDBG_AEC("%s:cur_real_gain %f /snap_real_gain %f = %f\n", __func__,
    sproc->share.aec_ext.cur_real_gain, snap_real_gain,
    sproc->share.aec_ext.cur_real_gain / snap_real_gain);
  CDBG_AEC("%s:snap_exp_time %f /preview_exp_time %f = %f", __func__,
    snap_exp_time, preview_exp_time, snap_exp_time / preview_exp_time);

  /* support for anti-banding with trade-off of changing true
   * ISO gain in order to remove bands */
  if (aec->antibanding == STATS_PROC_ANTIBANDING_60HZ ||
    aec->antibanding == STATS_PROC_ANTIBANDING_50HZ) {
    int limit;
    if (aec->antibanding == STATS_PROC_ANTIBANDING_60HZ)
      limit = 120;
    else
      limit = 100;

    if (snap_exp_time > 1.0/limit) {
      float rem, temp_snap_exp_time;
      rem =  (uint32_t) (snap_exp_time * limit);
      temp_snap_exp_time = rem * (1.0/limit); /*Make exp_time fit to bandgap*/
      snap_real_gain = snap_real_gain * snap_exp_time / temp_snap_exp_time;
      snap_exp_time = temp_snap_exp_time;
    }
  }
  snap->real_gain = snap_real_gain;

  /* Step 3 Calculate Snapshot Exposure Settings */
  CDBG_AEC("%s: preview_fps %d, preview_linesPerFrame %d", __func__,
    sinfo->preview_fps, preview_linesPerFrame);
  CDBG_AEC("%s: snap_exp_time %f, snap_real_gain %f, snap_linesPerFrame %d,"
    "snap_fps %d", __func__, snap_exp_time, snap_real_gain,
    sinfo->snap_linesPerFrame, sinfo->snapshot_fps);
  snap->line_count = (uint32_t) (((snap_exp_time * sinfo->
    snap_linesPerFrame * sinfo->snapshot_fps) / AEC_Q8) + 0.5);

  if (snap->line_count == 0)
    snap->line_count = 1;

  CDBG_AEC("%s: snap_linecount %d, max_snap_linecount %d", __func__,
    snap->line_count, sinfo->snap_max_line_cnt);
  while (snap->line_count > sinfo->snap_max_line_cnt) {
    snap_fps_divider = (int) (snap_fps_divider * 2);
    snap->line_count = (uint32_t) (((snap_exp_time *  sinfo->
      snap_linesPerFrame * sinfo->snapshot_fps) / (AEC_Q8 *
      snap_fps_divider)) + 0.5);
    if (snap->line_count == 0)
      snap->line_count = 1;
  }
  snap->exp_time = snap_exp_time;
  if (snap->line_count == 0) /* linecnt cant be 0, if so, it should be 1 */
    snap->line_count = 1;

  snap->real_gain = snap_real_gain;
  if (snap->line_count <  sinfo->snap_linesPerFrame)
    cur_snap_fps_Q8  =  sinfo->snapshot_fps;
  else
    cur_snap_fps_Q8 = (sinfo->snapshot_fps * sinfo->
      snap_linesPerFrame) / snap->line_count;


  CDBG_AEC("%s: snap_linecount %d, real_gain %f", __func__,
    snap->line_count, snap->real_gain);
  if (aec->redeye_reduction && sproc->share.aec_ext.use_led_estimation) {
    snap->redeye_led_on_skip_frm = (uint32_t)((redeye_led_duration *
      cur_snap_fps_Q8 + 255000) / ( 1000 * 256));
    snap->redeye_led_off_skip_frm = (uint32_t)((redeye_led_wait *
      cur_snap_fps_Q8 + 255000) / ( 1000 * 256));
  } else {
    snap->redeye_led_on_skip_frm = 0;
    snap->redeye_led_off_skip_frm = 0;
  }
  CDBG_AEC("%s: redeye_on_skip %d, redeye_off_skip %d", __func__,
    snap->redeye_led_on_skip_frm, snap->redeye_led_off_skip_frm);
  /*restore the exposure before LED was fired*/
  aec_restore_exp_level_prior_to_led_fire(sproc, aec);

  CDBG_HIGH("%s: real_gain %f, linecount %d", __func__,
    snap->real_gain, snap->line_count);
  return 0;
} /* aec_process_snapshot */
