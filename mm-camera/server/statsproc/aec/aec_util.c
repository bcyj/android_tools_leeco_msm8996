/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#ifdef _ANDROID_
  #include <cutils/properties.h>
#endif
#include <math.h>
#include <inttypes.h>
#include <sys/time.h>
#include <media/msm_camera.h>
#include <string.h>
#include <stdlib.h>
#include "camera.h"
#include "stats_proc.h"
#include "aec.h"

#define HJR_DEFAULT_GAIN_FACTOR    2
#define HJR_LINE_COUNT_MIN_LIMIT  10
#define HJR_AF_LOCK_NUM            5
#define HJR_LUMA_THRESHOLD(x) ((x) * 0.8)

#define MAX_LUMA_TARGET          220
#define STATS_PROC_DEF_BRIGHTNESS       3
#define LED_NUM_SKIP_FRAMES        3
#define PREP_SNAP_TIMEOUT 1

/* Formula to calucalte the new EV target using adjustment step (EV_Comp) and
 * the default value.
 * EV_Comp Enew = 2        * EVdefault.
 * We are using the adjustment step of 1/6, the following table contains the
 * EV_comp 2       value at different EV_Comp step.
 * Enum camera_ev_compensation_type is used as an index to get the correct
 * multiplying factor from this table.*/
float aec_ev_comp_one_over_six_tbl[] = {
  0.2500,    /* 2^EV_Comp = 2^-12/6 */
  0.2806,    /* 2^EV_Comp = 2^-11/6 */
  0.3150,    /* 2^EV_Comp = 2^-10/6 */
  0.3536,    /* 2^EV_Comp = 2^-9/6  */
  0.3969,    /* 2^EV_Comp = 2^-8/6  */
  0.4454,    /* 2^EV_Comp = 2^-7/6  */
  0.5000,    /* 2^EV_Comp = 2^-6/6  */
  0.5612,    /* 2^EV_Comp = 2^-5/6  */
  0.6299,    /* 2^EV_Comp = 2^-4/6  */
  0.7071,    /* 2^EV_Comp = 2^-3/6  */
  0.7937,    /* 2^EV_Comp = 2^-2/6  */
  0.8909,    /* 2^EV_Comp = 2^-1/6  */
  1.0000,    /* 2^EV_Comp = 2^0     */
  1.1225,    /* 2^EV_Comp = 2^1/6   */
  1.2599,    /* 2^EV_Comp = 2^2/6   */
  1.4142,    /* 2^EV_Comp = 2^3/6   */
  1.5874,    /* 2^EV_Comp = 2^4/6   */
  1.7818,    /* 2^EV_Comp = 2^5/6   */
  2.0000,    /* 2^EV_Comp = 2^6/6   */
  2.2449,    /* 2^EV_Comp = 2^7/6   */
  2.5198,    /* 2^EV_Comp = 2^8/6   */
  2.8284,    /* 2^EV_Comp = 2^9/6   */
  3.1748,    /* 2^EV_Comp = 2^10/6  */
  3.5636,    /* 2^EV_Comp = 2^11/6  */
  4.0000     /* 2^EV_Comp = 2^12/6  */
};

static aec_exposure_compensation_t exp_comp_tbl = {
  -12, 6,      /* Minimun value  Numerator,Denominator  */
  12, 6,      /* Maximum value  Numerator,Denominator  */
  1, 6,       /* Step Value     Numerator,Denominator  */
  0, 6,       /* Default value  Numerator,Denominator  */
  12,         /* Value to convert fraction to index    */
  aec_ev_comp_one_over_six_tbl,
};

/*  UTILITY APIS*/
/*===========================================================================
 * FUNCTION    - aec_init_data -
 *
 * DESCRIPTION: set the aec settings to default values
 *==========================================================================*/
int aec_init_data(stats_proc_t *sproc, aec_t *aec)
{
  int i;

  memset(&(aec->slow_c), 0, sizeof(aec_slow_conv_t));
  memset(aec->luma.previous_sumArray, 0, sizeof(uint32_t) * 256);

  aec->luma.lux_index   = 50;
  aec->exp_increase     = TRUE;
  aec->sensor_update_ok = TRUE;
  aec->roi.enable       = FALSE;
  aec->aec_lock         = FALSE;
  aec->fps              = 0;
  sproc->share.aec_ext.afr_enable       = TRUE;
  if (!sproc->share.aec_ext.cur_real_gain)
    sproc->share.aec_ext.cur_real_gain = 1.0;
  sproc->share.hjr_af_enabled = FALSE;
  sproc->share.aec_ext.sof_update = FALSE;

  /* snapshot exif tag initialization */
  sproc->share.aec_ext.snap.exp_mode = 0;
  sproc->share.aec_ext.snap.exp_program = 0;

  sproc->share.aec_ext.prev_sensitivity = 0;
  aec_fast_conv_config(sproc, aec);
  aec->motion       = 0.0;
  aec->mtn.tmp_val  = 0.0;
  aec->mtn.val      = 1.0;
  aec->mtn.snap_val = 1.0;
  aec->mtn.frame    = 1.0;
  aec->mtn.apply    = FALSE;
  aec->sensor_update_ok = TRUE;
  aec->aec_mtr_area.num_area = 0;
  for (i=0; i< MOTION_ARRAY_SIZE; i++)
    aec->mtn.array[i] = 0.0;

  aec->mtn.status  = MOTION_ISO_OFF;
  sproc->share.aec_ext.aec_flash_settled = AEC_SETTLED;
  aec_set_exp_metering_mode(sproc, aec, CAMERA_AEC_FRAME_AVERAGE);

  /* Init for digital gain */
  sproc->share.aec_ext.stored_digital_gain = 1.0;

  /* Report starting frame rate */
  aec->flash.strb_int_state = STROBE_OFF;
  sproc->share.aec_ext.strobe_cfg_st = STROBE_NOT_NEEDED;
  sproc->share.aec_ext.aec_settled  = FALSE;

  aec->m_iso_test.mtn       = 1.0;
  aec->m_iso_test.motion_up = TRUE;
  aec->m_iso_test.frame_cnt = 0;
  aec->exp_comp_val = 0;
  aec->roi_test.cur_num = 0;
  aec->roi_test.fr_cnt  = 0;
  if(aec->exp_tbl_ptr ==  NULL){
    aec->exp_tbl_ptr = (exposure_entry_type *)malloc(MAX_EXPOSURE_TABLE_SIZE *
      sizeof(exposure_entry_type));
    if(!aec->exp_tbl_ptr) {
      CDBG_ERROR("%s: Error Exposure table not initialized as Malloc failed!!!",
        __func__);
      return -1;
    }
  }

  sproc->share.aec_ext.iso = CAMERA_ISO_AUTO;
  sproc->share.aec_ext.band_50hz_gap =
   (float) (((float) 1 / 100) *
   (((float) (sproc->input.sensor_info.pixel_clock)) /
   ((float) (sproc->input.sensor_info.pixel_clock_per_line))))+0.5;
  return 0;
} /* aec_init_data */

/*==========================================================================
 * FUNCTION    - aec_est_strobe_flash_for_snapshot -
 *
 * DESCRIPTION:
 *=========================================================================*/
void aec_est_strobe_flash_for_snapshot(stats_proc_t *sproc, aec_t *aec)
{
  int exp_idx_adj = 0, flash_influence_only = FALSE;
  float gain = 1.0, strobe_gain=1.0, estimated_luma_hi_gain;
  float extrapolated_luma_low, luma_target;
  float si_off, si_low, si_hi;

  CDBG_AEC("%s: luma_off = %u, luma_on = %u", __func__,
    aec->flash.strb_est.luma_off, aec->flash.strb_est.luma_on);
  /*boost both on & off by 1*/
  if (!aec->flash.strb_est.luma_on || !aec->flash.strb_est.luma_off) {
    if (aec->flash.strb_est.luma_on < 255)
      aec->flash.strb_est.luma_on++;
    if (aec->flash.strb_est.luma_off < 255)
      aec->flash.strb_est.luma_off++;
  }
  si_off = (float) aec->exp_tbl_ptr[aec->flash.strb_est.index_off].gain / 256.0;
  si_off *= aec->exp_tbl_ptr[aec->flash.strb_est.index_off].line_count;

  si_low = ((float) aec->flash.strb_est.luma_off /
    aec->flash.strb_est.luma_on) * si_off;

  luma_target = aec->luma.comp_target;
  /* This is frame luma would be if exposure index would not change
     when turning on 100mA led */
  extrapolated_luma_low = (int)aec->flash.strb_est.luma_on;

  /* Sensitivity required to maintain frame luma as LED off when LED turns on.
    Should be <=1x. */
  if ((aec->flash.strb_est.luma_off == 0) ||
    (extrapolated_luma_low - aec->flash.strb_est.luma_off <= 0))
    /* completely drk even with LED on, just max out the gain. */
    estimated_luma_hi_gain = si_off;
  else
    estimated_luma_hi_gain = si_off * aec->flash.strb_est.luma_off /
    ((float) aec->flash.strb_est.luma_off + gain * (extrapolated_luma_low -
      aec->flash.strb_est.luma_off));

  si_hi = estimated_luma_hi_gain;
  /* Increase gain to reach luma target: */
  if (extrapolated_luma_low == 0)
    extrapolated_luma_low = 1;

  estimated_luma_hi_gain *= luma_target / aec->flash.strb_est.luma_off;

  exp_idx_adj = (int)(log10(estimated_luma_hi_gain / si_off) *
    sproc->input.chromatix->exposure_index_adj_step);

  if (aec->flash.strb_est.luma_on <= aec->flash.strb_est.luma_off) {
    strobe_gain = sproc->input.chromatix->AEC_strobe_flash.max_strobe_gain;
    CDBG_AEC("%s: preflash no diff, strobe_gain=%f", __func__, strobe_gain);
  } else if (exp_idx_adj > 0) {
    strobe_gain = ((float)(luma_target - aec->flash.strb_est.
      luma_off)) / (float) (aec->flash.strb_est.luma_on -
      aec->flash.strb_est.luma_off);
    CDBG_AEC("%s: preflash made diff, strobe_gain=%f", __func__, strobe_gain);
    if (strobe_gain > sproc->input.chromatix->AEC_strobe_flash.max_strobe_gain)
      strobe_gain = sproc->input.chromatix->AEC_strobe_flash.max_strobe_gain;
  } else
    strobe_gain = 1.0;

  aec->flash.strb_est.strb_gain = strobe_gain;
  aec->flash.strb_est.exp_index_adjustment = exp_idx_adj;
  /* for AWB compensation. */
  si_hi = si_off * aec->flash.strb_est.luma_off / ((float) aec->
    flash.strb_est.luma_off + strobe_gain * (extrapolated_luma_low -
    aec->flash.strb_est.luma_off));
  if (flash_influence_only) {
    sproc->share.aec_ext.flash_si.off = 1000;
    sproc->share.aec_ext.flash_si.low = 100;
    sproc->share.aec_ext.flash_si.high = 1;
  } else {
    sproc->share.aec_ext.flash_si.off = si_off;
    sproc->share.aec_ext.flash_si.low = si_low;
    sproc->share.aec_ext.flash_si.high = MAX(1,si_hi);
  }
  CDBG_AEC("%s: exp_idx %d, cur_line_cnt %d, exp_index_adj %d, si_off %f,  \
    si_low %f, si_hi %f", __func__, sproc->share.aec_ext.exp_index,
    sproc->share.aec_ext.cur_line_cnt,
    aec->flash.strb_est.exp_index_adjustment, sproc->share.aec_ext.flash_si.off,
    sproc->share.aec_ext.flash_si.low, sproc->share.aec_ext.flash_si.high);
} /* aec_est_strobe_flash_for_snapshot */

/*==========================================================================
 * FUNCTION    - aec_strobe_flash_store_est -
 *
 * DESCRIPTION:
 *=========================================================================*/
void aec_strobe_flash_store_est(stats_proc_t *sproc, aec_t *aec,
  strobe_internal_state_t strb_state)
{
  if (strb_state == STROBE_OFF) {
    aec->flash.strb_est.luma_off = sproc->share.aec_ext.cur_luma;
    aec->flash.strb_est.index_off  = sproc->share.aec_ext.exp_index;

    aec->flash.strb_est.lux_idx_off    = sproc->share.aec_ext.lux_idx;
    aec->flash.strb_est.linecount_off  = sproc->share.aec_ext.cur_line_cnt;
    aec->flash.strb_est.real_gain_off  = sproc->share.aec_ext.cur_real_gain;
    sproc->share.aec_ext.use_strobe = 0;
    /*todo lot of strobe flash state ENUMS STROBE_FLASH_PRE_ON*/
  } else if (strb_state == STROBE_PRE_ON) {
    aec->flash.strb_est.luma_on          = sproc->share.aec_ext.cur_luma;
    sproc->share.aec_ext.use_strobe  = 1;
    aec_est_strobe_flash_for_snapshot(sproc, aec);
  } else
    sproc->share.aec_ext.use_strobe  = 0;

  CDBG_AEC("%s: state %d, luma %u, index %u, lux_idx %f\n", __func__,
    aec->flash.strobe_mode, sproc->share.aec_ext.cur_luma,
    sproc->share.aec_ext.exp_index, sproc->share.aec_ext.lux_idx);
} /* aec_strobe_flash_store_est */
/*==========================================================================
 * FUNCTION    - aec_load_exp_tbl -
 *
 * DESCRIPTION:  Load exp table based to manipulate number of entries
 *=========================================================================*/
static void aec_load_exp_tbl(stats_proc_t *sproc, aec_t *aec)
{
  int i;
  int exp_comp_val_entries = 0;
  aec->valid_entries = sproc->input.chromatix->chromatix_exposure_table.
    valid_entries;
  if(aec->exp_tbl_ptr == NULL){
    CDBG_ERROR("%s Error aec exp tbl is NULL ",__func__);
    return ;
  }
  exp_comp_val_entries = aec->exp_comp_val * log10(2)/ log10(1.03);
  aec->valid_entries = STATS_PROC_MIN((sproc->input.chromatix->
    chromatix_exposure_table.valid_entries +
    exp_comp_val_entries), MAX_EXPOSURE_TABLE_SIZE);
  memcpy(aec->exp_tbl_ptr, sproc->input.chromatix->chromatix_exposure_table.
    exposure_entries,(sproc->input.chromatix->chromatix_exposure_table.
    valid_entries )* sizeof(exposure_entry_type));
  for (i=sproc->input.chromatix->chromatix_exposure_table.valid_entries;
    i< aec->valid_entries; i++) {
    aec->exp_tbl_ptr[i].gain = aec->exp_tbl_ptr[i-1].gain * 1.03;
    aec->exp_tbl_ptr[i].line_count = aec->exp_tbl_ptr[i-1].line_count;
  }
  sproc->share.aec_ext.exp_tbl_val = aec->valid_entries;
  if(aec->fps != 0)
    aec_set_parm_fps(sproc,aec, aec->fps);
}
/*==========================================================================
 * FUNCTION    - aec_set_brightness -
 *
 * DESCRIPTION:  Set brightness
 *=========================================================================*/
static void aec_set_brightness(stats_proc_t *sproc,  aec_t *aec, int luma)
{
  uint32_t luma_value;
  int luma_offset;

  if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_INIT)
    return;

  luma_value = (0.5 + (float) luma /
    sproc->input.chromatix->color_correction_global_gain);
  if (luma_value > MAX_LUMA_TARGET)
    luma_value = MAX_LUMA_TARGET;

  if (sproc->input.mctl_info.opt_state !=
    STATS_PROC_STATE_CAMCORDER) {
    if (luma_value < aec->luma.tolerance)
      luma_value = aec->luma.tolerance;

    aec->default_luma_target_compensated = luma_value;
    luma_offset = (int) luma_value - (int) (0.5 + (float)sproc->input.chromatix->
      default_luma_target / sproc->input.chromatix->color_correction_global_gain);

    aec->outdoor_luma_target_compensated = luma_offset + (int) (0.5 +
      (float)sproc->input.chromatix->outdoor_luma_target /
      sproc->input.chromatix->color_correction_global_gain);

  } else if (sproc->input.mctl_info.opt_state ==
    STATS_PROC_STATE_CAMCORDER) { /* Make sure results of luma and luma_tolerance
                                * addition and subtraction are within range. */
    if ((luma_value + 8 * aec->luma.tolerance) > 255)
      luma_value = 255 - 8 * aec->luma.tolerance;
    else if (luma_value < 8 * aec->luma.tolerance)
      luma_value = 8 * aec->luma.tolerance;

    /* Also cap new luma target, high end only.
     * Low end has already been caped */
    if (luma_value > MAX_LUMA_TARGET)
      luma_value = MAX_LUMA_TARGET;

    aec->luma.tolerance = (luma_value * ((float) pow(10.0, (float) (1.0 /
      sproc->input.chromatix->exposure_index_adj_step)) - 1)) + 1;

    aec->default_luma_target_compensated = luma_value;

    luma_offset = (int)luma_value - (int) (0.5 + (float) sproc->input.chromatix->
      default_luma_target / sproc->input.chromatix->color_correction_global_gain);

    aec->outdoor_luma_target_compensated = luma_offset + (int)(0.5 +
      (float)sproc->input.chromatix->outdoor_luma_target /
      sproc->input.chromatix->color_correction_global_gain);
  }
} /* aec_set_brightness */

/*===========================================================================
 * FUNCTION    - aec_load_chromatix -
 *
 * DESCRIPTION: load the aec settings  from chromatix
 *==========================================================================*/
int aec_load_chromatix(stats_proc_t *sproc, aec_t *aec)
{
  int i;
  if(aec->exp_tbl_ptr == NULL) {
    aec->exp_tbl_ptr = (exposure_entry_type *)malloc(MAX_EXPOSURE_TABLE_SIZE *
      sizeof(exposure_entry_type));
    if(!aec->exp_tbl_ptr) {
      CDBG_ERROR("%s: Error Exposure table not initialized as Malloc failed!!!",
        __func__);
      return -1;
    }
  }
  sproc->share.aec_ext.cur_luma = sproc->input.chromatix->default_luma_target;
  sproc->share.aec_ext.indoor_index = sproc->input.chromatix->aec_indoor_index;
  sproc->share.aec_ext.outdoor_index = sproc->input.chromatix->aec_outdoor_index;
  sproc->share.aec_ext.exp_tbl_val =
    sproc->input.chromatix->chromatix_exposure_table.valid_entries;
  sproc->share.aec_ext.exp_index = sproc->input.chromatix->aec_indoor_index;
  if (sproc->input.chromatix->aec_led_preview_flux == 0 ||
    (sproc->input.chromatix->aec_led_snapshot_flux_hi == 0))
    /* disable LED AEC compensation for snapshot since tuning
     * parameters not valid */
    aec->flash.led_compensation_disabled = TRUE;
  else {
    aec->flash.led_compensation_disabled = FALSE;
    memset(&(sproc->share.aec_ext.flash_si), 1,
      sizeof(stats_proc_flash_sensitivity_t));
  }
  aec_load_exp_tbl(sproc,aec);

  aec->num_exp_tbl_for_min_fps = aec->valid_entries;
  sproc->share.aec_ext.exp_tbl_val = aec->valid_entries;
  if(aec->fps != 0)
    aec_set_parm_fps(sproc,aec, aec->fps);
  aec->luma.target     =sproc->input.chromatix->default_luma_target;
  aec->luma.comp_target = aec->luma.target;
  aec->luma.tolerance  = sproc->input.chromatix->luma_tolerance;


  aec_set_brightness(sproc, aec, aec->luma.target);
  aec_set_strobe_mode(sproc, aec,STROBE_OFF);
  aec_set_exp_metering_mode(sproc, aec,
    sproc->share.aec_ext.metering_type);

  /* Report starting frame rate */
  aec->afr_frame_rate       = sproc->input.sensor_info.preview_fps;
  aec_update_exp_idx(sproc, aec);
  sproc->share.prev_exp_index    = sproc->share.aec_ext.exp_index;
  sproc->share.aec_ext.cur_line_cnt = aec->exp_tbl_ptr[sproc->
    share.aec_ext.exp_index].line_count;
  sproc->share.aec_ext.cur_real_gain =(float)(aec->exp_tbl_ptr[sproc->
    share.aec_ext.exp_index].gain) / AEC_Q8;
  aec->luma.target = sproc->input.chromatix->default_luma_target;
  aec->current_luma_target = sproc->input.chromatix->default_luma_target;
  sproc->share.aec_ext.max_line_cnt = aec->exp_tbl_ptr[
    aec->valid_entries- 1].line_count;
  return 0;
} /* aec_load_chromatix */
/*==========================================================================
 * FUNCTION    - aec_util_calculate_led_low_bias_table -
 *
 * DESCRIPTION: When Preflash is enabled, led low bias tables needs to applied.
 * ASSUMPTION:  Preflash is enabled.
 *=========================================================================*/
void aec_util_calculate_led_low_bias_table(stats_proc_t *sproc, aec_t *aec)
{
  int i;
  uint32_t flash_on_stat[256], max_delta = 0;
  float ratio[256], max_ratio = 0;
  uint32_t *sumLumaArray =
    (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;
  if(sproc->share.aec_ext.aec_flash_settled != AEC_FLASH_SETTLE_WAIT)
    return;
  memset(aec->led_low_bias_table, 0, sizeof(int) * 256);
  for(i = 0; i < (int) sproc->input.mctl_info.numRegions; i++) {
    flash_on_stat[i] = sumLumaArray[i] * pow(1.03,
      aec->flash.led_est.index_off - sproc->share.aec_ext.exp_index);
    max_delta =
      max_delta > (flash_on_stat[i] - aec->led_off_SY[i]) ? max_delta :
      (flash_on_stat[i] - aec->led_off_SY[i]);
    /* Calculate Diff between led on and led off*/
    flash_on_stat[i] -= aec->led_off_SY[i];
  }
  for(i = 0; i < (int) sproc->input.mctl_info.numRegions; i++) {
    if(flash_on_stat[i] > max_delta * 0.25) {
      ratio[i] = sumLumaArray[i]/aec->led_off_SY[i];
      if(max_ratio < ratio[i])
        max_ratio = ratio[i];
    }
  }
  for(i = 0; i < (int)sproc->input.mctl_info.numRegions; i++) {
    if(ratio[i] > max_ratio *0.5)
      aec->led_low_bias_table[i] = 1.0;
  }
  aec->bias_table = &aec->led_low_bias_table[0];
  sproc->share.aec_ext.metering_type = CAMERA_AEC_USER_METERING;
}
/*==========================================================================
 * FUNCTION    - aec_fast_conv_config -
 *
 * DESCRIPTION:  Update fast convergence related variables.
 *=========================================================================*/
void aec_fast_conv_config(stats_proc_t *sproc, aec_t *aec)
{
  if (!aec->reach_target_before) /* during camera launch */
    aec->fast_conv_speed = (uint32_t) (0.95 * AEC_Q8);
  else
    aec->fast_conv_speed = sproc->input.chromatix->
    aggressiveness_values * AEC_Q8;

  aec->frame_skip = sproc->input.chromatix->aec_fast_convergence_skip;
} /* aec_fast_conv_config */

/*==========================================================================
 * FUNCTION    - aec_update_exp_idx_for_led -
 *
 * DESCRIPTION: Updates the exposure idx, gain and line count based on
                led influence.
 *=========================================================================*/
void aec_update_exp_idx_for_led(stats_proc_t *sproc, aec_t *aec)
{
  float estimated_luma_600_gain, exp_index;
  int exp_idx_adj;
  if (aec->flash.led_compensation_disabled)
          return;

  estimated_luma_600_gain = sproc->share.aec_ext.flash_si.high *
    ((double)aec->luma.comp_target /  aec->flash.led_est.luma_off);

  exp_idx_adj = (int)(log10(estimated_luma_600_gain /
    sproc->share.aec_ext.flash_si.off) *
    sproc->input.chromatix->exposure_index_adj_step);

  exp_index = aec->flash.led_est.index_off + exp_idx_adj;
  if (exp_index < 0)
    exp_index = 0;

  if ((exp_index > sproc->share.aec_ext.exp_tbl_val - 1)
    || aec->flash.led_compensation_disabled)
    exp_index = sproc->share.aec_ext.exp_tbl_val - 1;
  sproc->share.aec_ext.exp_index = exp_index;
  sproc->share.aec_ext.cur_real_gain = (float)aec->
    exp_tbl_ptr[sproc->share.aec_ext.exp_index].gain / 256.0;

  sproc->share.aec_ext.cur_line_cnt =
    aec->exp_tbl_ptr[sproc->share.aec_ext.exp_index].line_count;
  CDBG_AEC("%s : Final applied led gain %f line_cnt %d", __func__,
    sproc->share.aec_ext.cur_real_gain,
    sproc->share.aec_ext.cur_line_cnt);

}
/*==========================================================================
 * FUNCTION    - aec_adjust_exp_settings_for_led -
 *
 * DESCRIPTION: This fn estimates the led influence and calculate led 
                sensitivity.
 *=========================================================================*/
void aec_adjust_exp_settings_for_led(stats_proc_t *sproc, aec_t *aec)
{
  float gain = 1, estimated_luma_600_gain;
  int exp_idx_adj = 0, led_influence_only = FALSE;
  float exp_index, extrapolated_luma_on_100;
  float si_off, si_low, si_hi;

  if (aec->flash.led_compensation_disabled)
    return;

  CDBG_AEC("%s: adj_exp_settings_for_led, luma_off %u, luma_on_100 %u",
    __func__, aec->flash.led_est.luma_off,
    aec->flash.led_est.luma_on_100);
  if (aec->flash.led_est.luma_off == 0) {
    aec->flash.led_est.luma_off = 1;
    /*boost on and off both by 1*/
    if (aec->flash.led_est.luma_on_100 < 255)
      aec->flash.led_est.luma_on_100++;
    led_influence_only = TRUE;
  }
  if (aec->flash.led_est.luma_on_100 == 0) {
    aec->flash.led_est.luma_on_100 = 1;
    /*boost on and off both by 1*/
    if (aec->flash.led_est.luma_off < 255)
      aec->flash.led_est.luma_off++;
    led_influence_only = TRUE;
  }

  gain = sproc->input.chromatix->aec_led_snapshot_flux_hi /
    sproc->input.chromatix->aec_led_preview_flux;

  si_off = (float) aec->exp_tbl_ptr[aec->flash.led_est.index_off].gain / 256.0;
  si_low = ((float) aec->flash.led_est.luma_off /
    aec->flash.led_est.luma_on_100) * (float) aec->
    exp_tbl_ptr[aec->flash.led_est.index_on_100].gain / 256.0;

  si_off *= aec->exp_tbl_ptr[aec->flash.led_est.index_off].line_count;
  si_low *= aec->exp_tbl_ptr[aec->flash.led_est.index_on_100].line_count;

  /* This is frame luma would be if exposure index would not change
     when turning on 100mA led */
  extrapolated_luma_on_100 = (int32_t)(aec->flash.led_est.
    luma_on_100 * pow(1.03, (aec->flash.led_est.index_off -
    aec->flash.led_est.index_on_100)));

  /* Sensitivity required to maintain frame luma as LED off when LED turns on.
    Should be <=1x. */
  if ((aec->flash.led_est.luma_off == 0) ||
    (extrapolated_luma_on_100 - aec->flash.led_est.luma_off <= 0))
    /* completely drk even with LED on, just max out the gain. */
    estimated_luma_600_gain = si_off;
  else
    estimated_luma_600_gain = si_off * aec->flash.led_est.
    luma_off / ((float) aec->flash.led_est.luma_off + gain *
      (extrapolated_luma_on_100 - aec->flash.led_est.luma_off));

  si_hi = estimated_luma_600_gain;
  /* Increase gain to reach luma target: */
  if (extrapolated_luma_on_100 == 0)
    extrapolated_luma_on_100 = 1;
  if (led_influence_only) {
    sproc->share.aec_ext.flash_si.off = 1000;
    sproc->share.aec_ext.flash_si.low = 100;
    sproc->share.aec_ext.flash_si.high = 1;
  } else {
    sproc->share.aec_ext.flash_si.off = si_off;
    sproc->share.aec_ext.flash_si.low = si_low;
    sproc->share.aec_ext.flash_si.high = MAX(1,si_hi);
  }
  CDBG_AEC("%s: exp_index_adj %d, sensitivity_led_off %f,"
    "si_low %f, si_hi %f", __func__,
    exp_idx_adj, si_off, si_low, si_hi);
} /* aec_adjust_exp_settings_for_led */
/*==========================================================================
 * FUNCTION    - aec_calc_exposure_time-
 *
 * DESCRIPTION:
 *=========================================================================*/
float aec_calc_sensitivity(stats_proc_t *sproc, float gain, uint32_t line_count)
{
  uint32_t preview_linesPerFrame, preview_fps;
  float preview_exposuretime;
  preview_linesPerFrame = sproc->input.sensor_info.preview_linesPerFrame;
  /*Max sensor preview fps*/
  preview_fps = sproc->input.sensor_info.preview_fps;
  preview_exposuretime = line_count /
    ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);
  CDBG_AEC("%s: gain %f exptime %f",__func__, gain, preview_exposuretime);
  CDBG_AEC("fps %d clk %d,",sproc->input.sensor_info.preview_fps,
    sproc->input.sensor_info.preview_linesPerFrame );
  return(gain* preview_exposuretime);
}
/*==========================================================================
 * FUNCTION    - aec_update_exp_idx-
 *
 * DESCRIPTION:
 *=========================================================================*/
void aec_update_exp_idx(stats_proc_t *sproc, aec_t *aec)
{
  float new_sensitivity, ratio, val;
  float gain =
    (float)(aec->exp_tbl_ptr[sproc->share.aec_ext.exp_index].gain) / AEC_Q8;
  CDBG_AEC("%s default exp index %d", __func__,
    sproc->share.aec_ext.exp_index);
  uint32_t line_count =   sproc->share.aec_ext.cur_line_cnt = aec->
    exp_tbl_ptr[sproc->share.aec_ext.exp_index].line_count;
  if(sproc->share.aec_ext.prev_sensitivity <= 0) {
    CDBG_AEC("%s: prev_sensitivity %f. Ignore First time launch", __func__,
    sproc->share.aec_ext.prev_sensitivity);
    return;
  }
  new_sensitivity = aec_calc_sensitivity(sproc,gain, line_count);
  CDBG_AEC("%s  new sens  %f old sens %f", __func__,new_sensitivity,
    sproc->share.aec_ext.prev_sensitivity);
  if (new_sensitivity == sproc->share.aec_ext.prev_sensitivity)
    return;
  val = (log10( sproc->share.aec_ext.prev_sensitivity / new_sensitivity)/
    log10(1.03));
  sproc->share.aec_ext.exp_index += round(val);
  if(sproc->share.aec_ext.exp_index < 0)
    sproc->share.aec_ext.exp_index = 0;
  if (sproc->share.aec_ext.exp_index >
    (int) sproc->share.aec_ext.exp_tbl_val - 1)
    sproc->share.aec_ext.exp_index = (int) sproc->share.aec_ext.exp_tbl_val - 1;
  CDBG_AEC("%s nex exp index %d",__func__,sproc->share.aec_ext.exp_index);
}

/*==========================================================================
 * FUNCTION    - aec_preview_antibanding -
 *
 * DESCRIPTION:
 *=========================================================================*/
void aec_preview_antibanding(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t preview_linesPerFrame, preview_fps;
  float preview_exposuretime, min_gain_allowed;

  min_gain_allowed = (float) aec->exp_tbl_ptr[0].gain / 256.0;
  CDBG_AEC("%s: antibanding %d", __func__, aec->antibanding);
  if (aec->antibanding == STATS_PROC_ANTIBANDING_60HZ) {
    preview_linesPerFrame = sproc->input.sensor_info.preview_linesPerFrame;

    /*Max sensor preview fps*/
    preview_fps = sproc->input.sensor_info.preview_fps;
    preview_exposuretime = sproc->share.aec_ext.cur_line_cnt /
      ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);
    if (preview_exposuretime > (1/120.0)) {
      float rem, temp_preview_exposuretime;
      /*round exposure time to me multiple of 1/120s*/
      if (sproc->share.aec_ext.cur_real_gain >= 2 * min_gain_allowed)
        rem = (float)((uint32_t) ((preview_exposuretime * 120) + 0.5));
      else
        rem = (float)((uint32_t) (preview_exposuretime * 120));

      /* Make exp_time fit to band gap */
      temp_preview_exposuretime = rem * (1/120.0);
      sproc->share.aec_ext.cur_real_gain = sproc->share.aec_ext.cur_real_gain *
        preview_exposuretime / temp_preview_exposuretime;
      CDBG_AEC("%s: 60Hz, new exp_time (ms) %f, no antiband exp_time %f",
        __func__, temp_preview_exposuretime, preview_exposuretime);
      preview_exposuretime = temp_preview_exposuretime;
      sproc->share.aec_ext.cur_line_cnt = (uint32_t)(((preview_exposuretime *
        preview_linesPerFrame * preview_fps) / AEC_Q8) + 0.5);
    } else
      CDBG_AEC("%s: 60Hz on, but ET less than 8ms exp_time (ms) %f",
        __func__, preview_exposuretime);

  } else if (aec->antibanding == STATS_PROC_ANTIBANDING_50HZ) {
    preview_linesPerFrame = sproc->input.sensor_info.preview_linesPerFrame;
    /*Max sensor preview fps*/
    preview_fps = sproc->input.sensor_info.preview_fps;

    preview_exposuretime = sproc->share.aec_ext.cur_line_cnt/
      ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);
    if (preview_exposuretime > (1/100.0)) {
      float rem, temp_preview_exposuretime;
      /*round exposure time to me multiple of 1/120s*/
      if (sproc->share.aec_ext.cur_real_gain >= 2 * min_gain_allowed)
        rem = (float)((uint32_t) ((preview_exposuretime * 100) + 0.5));
      else
        rem = (float)((uint32_t) (preview_exposuretime * 100));

      /* Make exp_time fit to band gap */
      temp_preview_exposuretime = rem * (1/100.0);
      /* Update global real gain */
      sproc->share.aec_ext.cur_real_gain = sproc->share.aec_ext.cur_real_gain *
        preview_exposuretime / temp_preview_exposuretime;
      CDBG_AEC("%s: 50Hz, new exp_time (ms) %f, no antiband exp_time %f",
        __func__, temp_preview_exposuretime, preview_exposuretime);
      preview_exposuretime = temp_preview_exposuretime;
      sproc->share.aec_ext.cur_line_cnt = (((preview_exposuretime *
        preview_linesPerFrame * preview_fps) / AEC_Q8) + 0.5);
    } else
      CDBG_AEC("%s: 50Hz on, but ET less than 10ms exp_time (ms) %f",
        __func__, preview_exposuretime);
  }
}  /* aec_preview_antibanding */

/*==========================================================================
 * FUNCTION    - aec_strobe_enable_check -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_strobe_enable_check(void)
{
  int32_t enabled = 0;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
  property_get("persist.camera.strobe", value, "0");
  enabled = atoi(value);

  CDBG_AEC("strobe flash enabled =%d\n", enabled);
#endif
  return enabled;
} /* aec_strobe_enable_check */

/*==========================================================================
 * FUNCTION    - aec_use_strobe -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_use_strobe(stats_proc_t *sproc, aec_t *aec)
{
  float snapshot_exposure_time, preview_exposure_time, min_gain_allowed;

  min_gain_allowed      = (float)aec->exp_tbl_ptr[0].gain / 256.0;
  preview_exposure_time = ((float)(sproc->share.aec_ext.cur_line_cnt * 256)) /
    ((float)(sproc->input.sensor_info.preview_fps *
    sproc->input.sensor_info.preview_linesPerFrame));

  snapshot_exposure_time = 256.0 / ((float)(sproc->input.sensor_info.snapshot_fps));

  /* check that line_count is full frame and gain at least 4x min gain.
   * snapshot must have line_count >= vsync active for flash to work
   * without mechanical shutter.
   * check preview exp_time >= 1/snapshot_fps to be able to get snapshot
   * full frame exposure.then check current real gain  > 4x min gain */
  if (sproc->share.aec_ext.cur_real_gain * preview_exposure_time >=
    snapshot_exposure_time * min_gain_allowed)
    return TRUE;
  else
    return FALSE;
} /* aec_use_strobe */

/* AEC SET APIS */
/*==========================================================================
 * FUNCTION    - aec_set_exp_metering_mode -
 *
 * DESCRIPTION:  Set mode of the Auto Exposure Control
 *=========================================================================*/
int aec_set_exp_metering_mode(stats_proc_t *sproc, aec_t *aec,
  camera_auto_exposure_mode_type aec_metering)
{
 stats_proc_aspect_ratio_t aspect_ratio = STATS_PROC_UNDEF;
  if((sproc->input.mctl_info.preview_width *3) ==
    (sproc->input.mctl_info.preview_height *4))
    aspect_ratio = STATS_PROC_4_TO_3;
  if((sproc->input.mctl_info.preview_width *9) ==
    (sproc->input.mctl_info.preview_height *16))
    aspect_ratio = STATS_PROC_16_TO_9;
  sproc->input.mctl_info.numRegions = 256;

  if (aec_metering >= CAMERA_AEC_MAX_MODES)
    return -1;
  else
    sproc->share.aec_ext.metering_type = aec_metering;
  /* Choose an exposure table based on algorith selection above */
  switch (sproc->input.mctl_info.numRegions) {
    case 256:
      if (aec_metering == CAMERA_AEC_CENTER_WEIGHTED)
        aec->bias_table =
          &(sproc->input.chromatix->AEC_weight_center_weighted[0][0]);
      else if (aec_metering == CAMERA_AEC_CENTER_WEIGHTED_ADV) {
        /* Advanced Center weighted metering mode */
        if(aspect_ratio != STATS_PROC_UNDEF)
          aec->bias_table = aec_center_weighted_adv[aspect_ratio];
        else
          aec->bias_table =
            &(sproc->input.chromatix->AEC_weight_center_weighted[0][0]);
      } else if (aec_metering == CAMERA_AEC_SPOT_METERING)
        aec->bias_table = &(sproc->input.chromatix->AEC_weight_spot_meter[0][0]);
      else if (aec_metering == CAMERA_AEC_SPOT_METERING_ADV) {
        /* Advanced Spot metering metering mode */
        if(aspect_ratio != STATS_PROC_UNDEF)
          aec->bias_table = aec_spot_metering_adv[aspect_ratio];
        else
          aec->bias_table =
            &(sproc->input.chromatix->AEC_weight_spot_meter[0][0]);;
      }
      else
        aec->bias_table = NULL;
      CDBG_AEC("%s: set_bias_table case 256",  __func__);
      break;
    case 64:
      if (aec_metering == CAMERA_AEC_CENTER_WEIGHTED)
        aec->bias_table = aec_center_weighted_8x8;
      else if (aec_metering == CAMERA_AEC_SPOT_METERING)
        aec->bias_table = aec_spot_metering_8x8;
      else
        aec->bias_table = NULL;
      CDBG_AEC("%s: set_bias_table case 64",  __func__);
      break;
    case 16:
      if (aec_metering == CAMERA_AEC_CENTER_WEIGHTED)
        aec->bias_table = aec_center_weighted_4x4;
      else if (aec_metering == CAMERA_AEC_SPOT_METERING)
        aec->bias_table = aec_spot_metering_4x4;
      else
        aec->bias_table = NULL;
      CDBG_AEC("%s: set_bias_table case 16",  __func__);
      break;
    default:
      aec->bias_table = NULL;
      CDBG_AEC("%s:ERROR set_bias_table fail",  __func__);
      return -1;
  }
  return 0;
} /* aec_set_exp_metering_mode */

/*==========================================================================
 * FUNCTION    - aec_set_iso_mode -
 *
 * DESCRIPTION:  Set iso mode
 *=========================================================================*/
int aec_set_iso_mode(stats_proc_t *sproc,  aec_t *aec, int iso)
{
  if (iso >= CAMERA_ISO_MAX)
    return -1;
  else
    sproc->share.aec_ext.iso = iso;
  return 0;
} /* aec_set_iso_mode */

/*==========================================================================
 * FUNCTION    - aec_set_antibanding -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_set_antibanding(stats_proc_t *sproc,  aec_t *aec,
  const stats_proc_antibanding_type antibanding)
{
  CDBG("aec_set_antibanding: %d\n", antibanding);
  if (antibanding >= STATS_PROC_MAX_ANTIBANDING) /* Check for valid param */
    return -1;
  /* Manual banding can only be set when AFD is disabled*/
//  if (!sproc->share.afd_enable)
    aec->antibanding = antibanding;
  return 0;
} /* aec_set_antibanding */

/*==========================================================================
 * FUNCTION    - aec_set_antibanding_status -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_set_antibanding_status(stats_proc_t *sproc,  aec_t *aec,
  const int status)
{
    CDBG("aec_set_antibanding_status: %d\n", status);
    sproc->share.afd_status = status;
  return 0;
} /* aec_set_antibanding */

/*==========================================================================
 * FUNCTION    - aec_set_brightness_level -
 *
 * DESCRIPTION: Set brightness level of the Auto Exposure Control
 *=========================================================================*/
int aec_set_brightness_level(stats_proc_t *sproc,  aec_t *aec, int brightness)
{
  int new_luma_target = brightness - STATS_PROC_DEF_BRIGHTNESS;
  int luma_target;

  /* step 2* AEC tolerance here */
  int brightness_step = aec->luma.tolerance * 2;
  /* must change by step size */
  new_luma_target *= brightness_step;
  /* Add current luma */
  new_luma_target += sproc->input.chromatix->default_luma_target;
  /* Force the luma target to be in [0, 255] range */
  if (new_luma_target < 0)
    luma_target = 0;
  else if (new_luma_target > 255)
    luma_target = 255;
  else
    luma_target = new_luma_target;

  aec->current_luma_target = luma_target;
  /* Set brightness of aec algorithm */
  aec_set_brightness(sproc, aec, luma_target);
  return 0;
} /* aec_set_brightness_level */

/*==========================================================================
 * FUNCTION    - aec_set_exposure_compensation -
 *
 * DESCRIPTION:  Set exposure compensation of the Auto Exposure Control
 *=========================================================================*/
int aec_set_exposure_compensation(stats_proc_t *sproc,  aec_t *aec,
  uint32_t aec_exp_comp)
{
  int rc = 0;
  if (aec_exp_comp == 0){
    CDBG("exposure compensation is zero. returning from %s", __func__);
    return rc;
  }
  int parmCurNum = (int16_t)(aec_exp_comp >> 16);
  int parmCurDen = (uint16_t)(aec_exp_comp & 0x0000FFFF);
  if (0 == parmCurDen) {
    CDBG_ERROR("%s:%d] Invalid exp value parmCurNum %d parmCurDen %d",
      __func__, __LINE__, parmCurNum, parmCurDen);
    return 0;
  }
  aec->exp_comp_val = aec_exp_comp;
  int max_parmVal = (exp_comp_tbl.max_numerator_val * exp_comp_tbl.
    step_denominator_val) / (exp_comp_tbl. max_denominator_val *
    exp_comp_tbl.step_numerator_val);

  int min_parmVal = (exp_comp_tbl.min_numerator_val * exp_comp_tbl.
    step_denominator_val) / (exp_comp_tbl.min_denominator_val *
    exp_comp_tbl.step_numerator_val);

  int parmVal = (parmCurNum * exp_comp_tbl.step_denominator_val)/
    (parmCurDen * exp_comp_tbl.step_numerator_val);

  int default_luma_target = aec->current_luma_target;
  int new_luma_target;

  if (parmVal > max_parmVal)
    rc = -1;
  else if (parmVal < min_parmVal)
    rc = -1;
  else { /* parm contains Numerator,Denominator. So first extract the
          * numerator and covert into index. */
  if(aec->exp_comp_val != (parmCurNum/exp_comp_tbl.step_denominator_val)){
     aec->exp_comp_val = (parmCurNum/ exp_comp_tbl.step_denominator_val);
     aec_load_exp_tbl(sproc,aec);
  }

    new_luma_target = (int) parmVal + exp_comp_tbl.val_to_get_index;
    new_luma_target = (int) ((default_luma_target *
      exp_comp_tbl.ev_ptr[new_luma_target]) + 0.5);
    aec_set_brightness(sproc, aec, new_luma_target);
  }
  return rc;
} /* aec_set_exposure_compensation */

/*==========================================================================
 * FUNCTION    - aec_set_fps_mode -
 *
 * DESCRIPTION:  Set mode of the Auto Exposure Control
 *=========================================================================*/
int aec_set_fps_mode(stats_proc_t *sproc,  aec_t *aec, stats_proc_fps_mode_type fps_mode)
{
  if (fps_mode == AEC_FPS_MODE_AUTO) {
    sproc->share.aec_ext.afr_enable = TRUE;
    sproc->share.aec_ext.exp_tbl_val =
      aec->valid_entries;
  } else { /* FPS_MODE_FIXED */
    sproc->share.aec_ext.afr_enable = FALSE;
    sproc->share.aec_ext.exp_tbl_val =
      sproc->input.chromatix->fix_fps_aec_table_index;
  }
  aec->num_exp_tbl_for_min_fps = sproc->share.aec_ext.exp_tbl_val;

  aec->min_fps = 0;

  CDBG_AEC("%s: afr_enable = %d, num_exp_table_vals = %d\n", __func__,
    sproc->share.aec_ext.afr_enable, sproc->share.aec_ext.exp_tbl_val);
  return 0;
} /* aec_set_fps_mode */

/*==========================================================================
 * FUNCTION    - aec_set_parm_fps -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_set_parm_fps(stats_proc_t *sproc,  aec_t *aec, int fps)
{
  uint32_t max_fps_point, fixed_index, min_fps_exp_idx;
  uint32_t max_fps = (fps & 0x0000FFFF) * AEC_Q8;
  uint32_t min_fps = ((fps & 0xFFFF0000) >> 16) * AEC_Q8;
  if((aec->valid_entries <= 0) || (fps <= 0))
    return -1;
  if (max_fps > sproc->input.sensor_info.max_preview_fps)
    max_fps = sproc->input.sensor_info.max_preview_fps;
  if (min_fps > max_fps)
    min_fps = 0;
  fixed_index     = sproc->input.chromatix->fix_fps_aec_table_index;
  if(((int)aec->fps == fps) &&
   (aec->num_exp_tbl_for_min_fps ==  sproc->share.aec_ext.exp_tbl_val)){
   CDBG_AEC("No change in fps and exp table count");
   return 0;
  }
  sproc->share.aec_ext.preview_fps = max_fps;
  aec->afr_frame_rate              = max_fps;
  aec->fps = fps;
  min_fps_exp_idx =
    aec->valid_entries - 1;

  if (min_fps != 0 && sproc->share.aec_ext.afr_enable) {
    max_fps_point = aec->exp_tbl_ptr[fixed_index].line_count;
    max_fps_point *= max_fps;

    for (; min_fps_exp_idx > fixed_index; min_fps_exp_idx--)
      if (max_fps_point >= min_fps *
        aec->exp_tbl_ptr[min_fps_exp_idx].line_count)
        break;
  } else if (!sproc->share.aec_ext.afr_enable)
    min_fps_exp_idx = fixed_index;

  aec->min_fps                     = min_fps;
  aec->num_exp_tbl_for_min_fps     = min_fps_exp_idx + 1;
  sproc->share.aec_ext.exp_tbl_val = min_fps_exp_idx + 1;
  aec->valid_entries               = min_fps_exp_idx + 1;
  if(sproc->share.aec_ext.exp_index  > (int)min_fps_exp_idx) {
    sproc->share.aec_ext.exp_index  = min_fps_exp_idx;
    sproc->share.aec_ext.cur_line_cnt = aec->exp_tbl_ptr[sproc->
      share.aec_ext.exp_index].line_count;
    sproc->share.aec_ext.cur_real_gain =(float)(aec->exp_tbl_ptr[sproc->
      share.aec_ext.exp_index].gain) / AEC_Q8;
  }
  return 0;
} /* aec_set_parm_fps */

/*==========================================================================
 * FUNCTION    - aec_hjr_adjust_for_max_gain -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_hjr_adjust_for_max_gain(stats_proc_t *sproc, aec_t *aec, float gain)
{
  /* 2 is max because any more creates too much quantization error */
  while (gain > 2) {
    gain /= 2;
    aec->hjr_data.new_line_count *= 2;
  }
  sproc->share.aec_ext.hjr_dig_gain = FLOAT_TO_Q(7, gain);
} /* aec_hjr_adjust_for_max_gain */

/*==========================================================================
 * FUNCTION    - aec_set_hjr -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_set_hjr(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t max_exp_index, fix_fps_exp_idx;
  uint32_t new_frame_rate, gain_factor, line_count_fix_fps_line_cnt;
  float gain_temp, max_sensor_gain;
  uint32_t max_register_gain;
  gain_factor =  HJR_DEFAULT_GAIN_FACTOR;
  max_exp_index  = sproc->share.aec_ext.exp_tbl_val;
  aec->hjr_data.new_line_count = sproc->share.aec_ext.cur_line_cnt;
  aec->hjr_data.new_sensor_gain = sproc->share.aec_ext.cur_real_gain;
  if((sproc->share.aec_ext.cur_line_cnt <
    (uint32_t)HJR_LINE_COUNT_MIN_LIMIT * gain_factor) ||
     !sproc->input.chromatix->linear_afr_support) {
    return 0;
  }
  aec->hjr_data.new_sensor_gain *= 2;
  aec->hjr_data.new_line_count /=2;
  CDBG_AEC("%s X sensorgain %f", __func__, aec->hjr_data.new_sensor_gain);
  return 0;
} /* aec_set_hjr */

/*===========================================================================
 * FUNCTION    - aec_set_for_hjr_af -
 *
 * DESCRIPTION:
 *==========================================================================*/
int aec_set_for_hjr_af(stats_proc_t *sproc,  aec_t *aec, int af_hjr)
{
  float real_gain, luma_ratio, gain_ratio;
  float max_preview_gain_allowed, preview_exposuretime;

  max_preview_gain_allowed = aec->exp_tbl_ptr[
    sproc->share.aec_ext.exp_tbl_val - 1].gain / 256.0;

  if (af_hjr) { /* Store current gain & line_count */
    if (sproc->share.afd_enable){
      /*Ensure HJR AF is not executed if exposure time after trade-off
      calculation is less than the minimum banding requiremen */
      preview_exposuretime =  aec_get_preview_exp_time(sproc, aec);
      if ((preview_exposuretime/2) <= (1/90.0))
        return 0;
    }
    aec->af_hjr_frame_skip_count = 3;
    aec->hjr_data.hjr_af_line_count = sproc->share.aec_ext.cur_line_cnt;
    aec->hjr_data.hjr_af_gain       = sproc->share.aec_ext.cur_real_gain;
    sproc->share.hjr_af_enabled = TRUE;

    luma_ratio = sproc->share.aec_ext.cur_luma /
      (float)aec->luma.comp_target;
    /* todo: see if we can do 1/4 exposure time for 4x fps increase...
     * brighter than x% of LT * means we have room to decrease exposure by
     * 2x with fps accordingly. */
    if (luma_ratio > 0.0)
      if (sproc->input.chromatix->linear_afr_support)
        if (sproc->share.aec_ext.exp_index >
          sproc->share.aec_ext.outdoor_index) {
          real_gain = sproc->share.aec_ext.cur_real_gain  * 2.0;
          if (real_gain > 2.0 * max_preview_gain_allowed)/*cap gain at 2*max */
            real_gain = 2.0 * max_preview_gain_allowed;

          gain_ratio = real_gain / sproc->share.aec_ext.cur_real_gain;
          sproc->share.aec_ext.cur_real_gain  = real_gain;
          sproc->share.aec_ext.cur_line_cnt = (uint32_t)((float)sproc->
            share.aec_ext.cur_line_cnt / gain_ratio);
          aec_preview_antibanding(sproc, aec);
        }
  } else { /* return all fps and exposure to normal */
    /* Restore gain & line_count to original values */
    if(sproc->share.hjr_af_enabled){
      sproc->share.aec_ext.cur_line_cnt  = aec->hjr_data.hjr_af_line_count;
      sproc->share.aec_ext.cur_real_gain = aec->hjr_data.hjr_af_gain;
      aec->hjr_af_lock_cnt          = HJR_AF_LOCK_NUM;
      sproc->share.hjr_af_enabled = FALSE;
     }
  }
  return 0;
} /* aec_set_for_hjr_af */

/*==========================================================================
 * FUNCTION    - aec_set_ROI -
 *
 * DESCRIPTION: Sets AEC Region Of Interest (ROI) param
 *=========================================================================*/
int aec_set_ROI(stats_proc_t *sproc,  aec_t *aec, stats_proc_interested_region_t aec_roi)
{
  uint32_t i, index;
  if (aec_roi.enable) { /*assuming index starting from 0*/
    if (aec_roi.rgn_index <= sproc->input.mctl_info.numRegions) {
      index = aec_roi.rgn_index;
      aec->roi = aec_roi;
      if (sproc->input.mctl_info.numRegions == 256) {
        /*in order to reduce the affect of crossing grid border, use 3x3 grids
         * for touch ROI in the center, 2x3 or 3x2 at border, 2x2 at corners */
        if (index < 16) { /*first line*/
          if (index == 0) { /*top-left corner*/
            aec->sub_roi.number = 4;
          } else if (index == 15) { /*top-right corner*/
            aec->sub_roi.number = 4;
            index --;
          } else {
            aec->sub_roi.number = 6;
          }
          aec->sub_roi.index[0] = index;
          aec->sub_roi.index[1] = index + 1;  /*next regn*/
          aec->sub_roi.index[2] = index + 16; /*regn below*/
          aec->sub_roi.index[3] = index + 17; /*below next*/
          if (aec->sub_roi.number == 6) {
            aec->sub_roi.index[4] = index - 1;
            aec->sub_roi.index[5] = index + 15;
          }
        } else if (index >= 240) { /*last line*/
          index -= 16;
          if ((index % 16) == 0) { /*bottom-left corner*/
            aec->sub_roi.number = 4;
          } else if ((index + 1) % 16 == 0) { /*bottom-right corner*/
            aec->sub_roi.number = 4;
            index --;
          } else {
            aec->sub_roi.number = 6;
          }
          aec->sub_roi.index[0] = index;
          aec->sub_roi.index[1] = index + 1;  /*next regn*/
          aec->sub_roi.index[2] = index + 16; /*regn below*/
          aec->sub_roi.index[3] = index + 17; /*below next*/
          if (aec->sub_roi.number == 6) {
            aec->sub_roi.index[4] = index - 1;
            aec->sub_roi.index[5] = index + 15; /*next regn*/
          }
        } else if (index % 16 ==0) { /*first colum*/
          aec->sub_roi.number = 6;
          aec->sub_roi.index[0] = index;
          aec->sub_roi.index[1] = index + 1; /*next regn*/
          aec->sub_roi.index[2] = index + 16;/*regn below*/
          aec->sub_roi.index[3] = index + 17;/*below next*/
          aec->sub_roi.index[4] = index - 16;
          aec->sub_roi.index[5] = index - 15; /*next regn*/
        } else if ((index + 1 ) % 16 == 0) { /*last colum*/
          index--;
          aec->sub_roi.number = 6;
          aec->sub_roi.index[0] = index;
          aec->sub_roi.index[1] = index + 1; /*next regn*/
          aec->sub_roi.index[2] = index + 16;/*regn below*/
          aec->sub_roi.index[3] = index + 17;/*below next*/
          aec->sub_roi.index[4] = index - 16;
          aec->sub_roi.index[5] = index - 15; /*next regn*/
        } else {  /*ccenter cases*/
          aec->sub_roi.number = 9;
          aec->sub_roi.index[0] = index;
          aec->sub_roi.index[1] = index + 1; /*regn nest*/
          aec->sub_roi.index[2] = index + 16;/*regn below*/
          aec->sub_roi.index[3] = index + 17;/*below next*/
          aec->sub_roi.index[4] = index - 16; /*regn above*/
          aec->sub_roi.index[5] = index - 15; /*above next*/
          aec->sub_roi.index[6] = index - 1;  /*regn bfr*/
          aec->sub_roi.index[7] = index - 17; /*above bfr*/
          aec->sub_roi.index[8] = index + 15; /*below bfr*/
        }
        CDBG_AEC("%s: rgns =%d\n", __func__, aec->sub_roi.number);
        for (i = 0; i < aec->sub_roi.number; i++)
          CDBG_AEC("reg[%d] idx =%dn", i, aec->sub_roi.index[i]);
      } else {
        aec->sub_roi.number = 1;
        aec->sub_roi.index[0] = index;
      }
    }
  } else
    aec->roi.enable = FALSE;

  CDBG_AEC("%s: ROI enable=%d, index=%d\n", __func__, aec->roi.enable,
    aec->roi.rgn_index);
  return 0;
} /* aec_set_ROI */

/*==========================================================================
 * FUNCTION    - aec_reset_LED -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_reset_LED(stats_proc_t *sproc,  aec_t *aec)
{
  if (sproc->share.aec_ext.led_frame_skip_cnt > 0) {
    sproc->share.aec_ext.exp_index = aec->flash.led_est.index_off;
    sproc->share.prev_exp_index = aec->flash.led_est.index_off;
  }
  CDBG_AEC("%s: led frame skip cnt  0", __func__);
  sproc->share.aec_ext.led_frame_skip_cnt = 0;
  sproc->share.aec_ext.led_state = MSM_CAMERA_LED_OFF;
  sproc->share.aec_ext.use_led_estimation = 0;
  return 0;
} /* aec_reset_LED */

/*==========================================================================
 * FUNCTION    - aec_store_LED_est_stats -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_store_LED_est_stats(stats_proc_t *sproc,  aec_t *aec,
  uint32_t led_state)
{
  uint32_t luma  = sproc->share.aec_ext.cur_luma;
  uint32_t index = sproc->share.aec_ext.exp_index;

  if (led_state == MSM_CAMERA_LED_OFF) {
    aec->flash.led_est.luma_off    = luma;
    aec->flash.led_est.index_off   = index;
    aec->flash.led_est.lux_idx_off = sproc->share.aec_ext.lux_idx;
    sproc->share.aec_ext.use_led_estimation = 0;
  } else if (led_state == MSM_CAMERA_LED_LOW) {
    aec->flash.led_est.luma_on_100      = luma;
    aec->flash.led_est.index_on_100     = index;
    sproc->share.aec_ext.use_led_estimation   = 1;
    aec_adjust_exp_settings_for_led(sproc, aec);
    aec_update_exp_idx_for_led(sproc, aec);

  #if !(USE_AEC_LED_ROI)
    /* Store bias table and exp type */
    aec->prev_bias_table = aec->bias_table;
    aec->prev_metering_type =
      sproc->share.aec_ext.metering_type;
    /* if pre-flash is enabled then calcualte
      led low bias table */
    if (!aec->roi.enable) {
      aec_util_calculate_led_low_bias_table(sproc, aec);
      luma = aec_calculate_current_luma(sproc, aec);
      aec->flash.led_est.luma_on_100      = luma;
      aec->flash.led_est.index_on_100     = index;
      aec_adjust_exp_settings_for_led(sproc, aec);
    }
    /* Restore bias table and mtr type */
    aec->bias_table = aec->prev_bias_table;
    sproc->share.aec_ext.metering_type = aec->prev_metering_type;
  #endif
  } else
    sproc->share.aec_ext.use_led_estimation = 0;

  return 0;
} /* aec_store_LED_est_stats */

/*==========================================================================
 * FUNCTION    - aec_set_strobe_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_set_strobe_mode(stats_proc_t *sproc,  aec_t *aec, int strobe_mode)
{
  /* Check the aec method is supported */
  if (sproc->input.chromatix->AEC_strobe_flash.strobe_enable != TRUE ||
    strobe_mode >= STROBE_FLASH_MODE_MAX)
    return -1;

  aec->flash.strobe_mode = strobe_mode;
  CDBG_AEC("%s: Strobe mode to %d\n", __func__, strobe_mode);
  return 0;
} /* aec_set_strobe_mode */

/* AEC GET APIS */
/*==========================================================================
 * FUNCTION    - aec_get_LED_over_exp_check -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_get_LED_over_exp_check(stats_proc_t *sproc, aec_t *aec)
{
  if (aec->flash.led_compensation_disabled)
    return FALSE;
  else if (sproc->share.aec_ext.cur_luma > 2 * aec->luma.comp_target)
    return TRUE;
  else
    return FALSE;
} /* aec_get_LED_over_exp_check */

/*===========================================================================
 * FUNCTION    - aec_get_settled_cnt -
 *
 * DESCRIPTION:
 *==========================================================================*/
int aec_get_settled_cnt(stats_proc_t *sproc, aec_t *aec)
{
  int rc = -1;
  if (sproc->share.aec_ext.aec_flash_settled == AEC_SETTLE_WAIT ||
    sproc->input.flash_info.led_mode == LED_MODE_TORCH) {

    if (sproc->share.luma_settled_cnt >= LUMA_SETTLED_BEFORE_AF_CNT || sproc->
      share.aec_ext.exp_index == (int)sproc->share.aec_ext.exp_tbl_val - 1
      || aec->settle_frame_cnt++ >= AEC_SETTLE_MAX_FRAME_CNT)
      rc = 0;
  } else if (sproc->share.aec_ext.aec_flash_settled == AEC_FLASH_SETTLE_WAIT) {
    CDBG_ERROR("%s: led_frame_skip %d  > %d, or > %f",__func__,
     sproc->share.aec_ext.led_frame_skip_cnt, LED_NUM_SKIP_FRAMES,
     (aec_get_preview_fps(sproc, aec)* 0.75 * PREP_SNAP_TIMEOUT));
    if (sproc->share.aec_ext.led_frame_skip_cnt >
      (aec_get_preview_fps(sproc, aec)* 0.75 * PREP_SNAP_TIMEOUT)){
      aec_store_LED_est_stats(sproc, aec, MSM_CAMERA_LED_LOW);
      rc = 0;
    }else if (sproc->share.aec_ext.led_frame_skip_cnt > LED_NUM_SKIP_FRAMES) {
      if (!aec_get_LED_over_exp_check(sproc, aec)) {
        aec_store_LED_est_stats(sproc, aec, MSM_CAMERA_LED_LOW);
        rc = 0;
      }
    }
  }
  if (rc == 0)
    sproc->share.aec_ext.aec_flash_settled = AEC_SETTLED;
  return rc;
} /* aec_get_settled_cnt */

/*==========================================================================
 * FUNCTION    - aec_get_strobe -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_get_strobe(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t compensation = 1; /* added for discrete AFR support. */

  /* Use strobe if exposure_index is max */
  compensation = (uint32_t)((int32_t)sproc->input.chromatix->aec_indoor_index -
    (int)sproc->share.aec_ext.indoor_index);

  if (sproc->share.aec_ext.lux_idx >= (int)
    (sproc->input.chromatix->wled_trigger_idx - compensation))
    return TRUE;
  else
    return FALSE;
} /* aec_get_strobe */

/*===========================================================================
 * FUNCTION    - aec_get_flash_for_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
int aec_get_flash_for_snapshot(stats_proc_t *sproc,  aec_t *aec)
{
  CDBG_AEC("%s: Strb mode =%d, LED mode =%d", __func__,
    aec->flash.strobe_mode, sproc->input.flash_info.led_mode);

  if (aec_strobe_enable_check() && (aec->flash.strobe_mode ==
    STROBE_FLASH_MODE_ON || (aec->flash.strobe_mode ==
    STROBE_FLASH_MODE_AUTO && aec_use_strobe(sproc, aec))))
    return 1;
  else if (sproc->input.flash_info.led_mode == LED_MODE_ON || (sproc->input.flash_info.
    led_mode ==  LED_MODE_AUTO && aec_get_strobe(sproc, aec)))
    return 2;
  return 0;
} /* aec_get_flash_for_snapshot */

/*==========================================================================
 * FUNCTION    - aec_get_preview_fps -
 *
 * DESCRIPTION:  return the curent fps
 *=========================================================================*/
float aec_get_preview_fps(stats_proc_t *sproc, aec_t *aec)
{
  if (sproc->share.aec_ext.cur_line_cnt >
    sproc->input.sensor_info.preview_linesPerFrame)
    return((float) sproc->input.sensor_info.preview_linesPerFrame /
      (float) sproc->share.aec_ext.cur_line_cnt) *
      ((float) sproc->input.sensor_info.preview_fps / AEC_Q8);
  else
    return(float)sproc->input.sensor_info.preview_fps / AEC_Q8;
} /* aec_get_preview_fps */

/*==========================================================================
* FUNCTION    - aec_get_preview_exp_time -
*
* DESCRIPTION:  return the curent exposture time
*=========================================================================*/
float aec_get_preview_exp_time(stats_proc_t *sproc,  aec_t *aec)
{
  float preview_exposuretime;

  preview_exposuretime = (float) sproc->input.sensor_info.preview_fps / AEC_Q8;
  preview_exposuretime =  sproc->share.aec_ext.cur_line_cnt /
    (preview_exposuretime * sproc->input.sensor_info.preview_linesPerFrame);

  return preview_exposuretime;
} /* aec_get_preview_exp_time */

/*==========================================================================
 * FUNCTION    - aec_set_bestshot_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_set_bestshot_mode(stats_proc_t *sproc, aec_t *aec,
  camera_bestshot_mode_type new_mode)
{
  int rc = 0;
  uint32_t  exp_comp_val = 0;
  int16_t  numerator16, denominator16 = exp_comp_tbl.step_denominator_val;
  chromatix_parms_type *cptr = sproc->input.chromatix;
  if (new_mode  >= CAMERA_BESTSHOT_MAX)
    return -1;

  if (aec->bestshot_d.curr_mode == new_mode)
    return 0; /* Do Nothing */

  CDBG_AEC("%s: mode %d", __func__, new_mode);
  /* Store current AEC vals */
  if (aec->bestshot_d.curr_mode == CAMERA_BESTSHOT_OFF) {
    aec->bestshot_d.iso = sproc->share.aec_ext.iso;
    aec->bestshot_d.metering_type = sproc->share.aec_ext.metering_type;
    aec->bestshot_d.exp_comp_val = aec->exp_comp_val;
  }
  /* CONFIG AEC for BESTHOT mode */
  if (new_mode  != CAMERA_BESTSHOT_OFF) {
    switch (new_mode) {
      case CAMERA_BESTSHOT_SPORTS:
      case CAMERA_BESTSHOT_ANTISHAKE:
      case CAMERA_BESTSHOT_ACTION:
        rc = aec_set_iso_mode(sproc, aec, CAMERA_ISO_400);
        break;
      case CAMERA_BESTSHOT_OFF:
      case CAMERA_BESTSHOT_LANDSCAPE:
      case CAMERA_BESTSHOT_SNOW:
      case CAMERA_BESTSHOT_BEACH:
      case CAMERA_BESTSHOT_SUNSET:
      case CAMERA_BESTSHOT_NIGHT:
      case CAMERA_BESTSHOT_PORTRAIT:
      case CAMERA_BESTSHOT_BACKLIGHT:
      case CAMERA_BESTSHOT_FLOWERS:
      case CAMERA_BESTSHOT_CANDLELIGHT:
      case CAMERA_BESTSHOT_FIREWORKS:
      case CAMERA_BESTSHOT_PARTY:
      case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
      case CAMERA_BESTSHOT_THEATRE:
      case CAMERA_BESTSHOT_AR:
      default:
        rc = aec_set_iso_mode(sproc, aec, CAMERA_ISO_AUTO);
        break;
    }

    switch (new_mode) {
      case CAMERA_BESTSHOT_LANDSCAPE:
      case CAMERA_BESTSHOT_SNOW:
      case CAMERA_BESTSHOT_BEACH:
      case CAMERA_BESTSHOT_SUNSET:
        rc = aec_set_exp_metering_mode(sproc, aec, CAMERA_AEC_FRAME_AVERAGE);
        break;
      case CAMERA_BESTSHOT_OFF:
      case CAMERA_BESTSHOT_NIGHT:
      case CAMERA_BESTSHOT_PORTRAIT:
      case CAMERA_BESTSHOT_BACKLIGHT:
      case CAMERA_BESTSHOT_SPORTS:
      case CAMERA_BESTSHOT_ANTISHAKE:
      case CAMERA_BESTSHOT_FLOWERS:
      case CAMERA_BESTSHOT_CANDLELIGHT:
      case CAMERA_BESTSHOT_FIREWORKS:
      case CAMERA_BESTSHOT_PARTY:
      case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
      case CAMERA_BESTSHOT_THEATRE:
      case CAMERA_BESTSHOT_ACTION:
      case CAMERA_BESTSHOT_AR:
      default:
        rc = aec_set_exp_metering_mode(sproc, aec, CAMERA_AEC_CENTER_WEIGHTED);
    }
    switch (new_mode) {
      case CAMERA_BESTSHOT_SNOW:
      case CAMERA_BESTSHOT_BEACH:
        /* set EV of 6 */
        numerator16 = (int16_t)(6 & 0x0000ffff);
        exp_comp_val = numerator16 << 16 | denominator16;
        rc = aec_set_exposure_compensation(sproc, aec, exp_comp_val);
        break;
      case CAMERA_BESTSHOT_SUNSET:
      case CAMERA_BESTSHOT_CANDLELIGHT:
        /* set EV of -6 */
        numerator16 = (int16_t)(-6 & 0x0000ffff);
        exp_comp_val = numerator16 << 16 | denominator16;
        rc = aec_set_exposure_compensation(sproc, aec, exp_comp_val);
        break;
      default:
        CDBG_AEC("%s Maintain current exp val", __func__);
        numerator16 = 0;
        exp_comp_val = numerator16 << 16 | denominator16;
        rc = aec_set_exposure_compensation(sproc, aec, exp_comp_val);
    }
  } else { /* Restore AEC vals */
    CDBG_AEC("%s:%d] iso %d meter %d ev %d", __func__, __LINE__,
      aec->bestshot_d.iso, aec->bestshot_d.metering_type,
      aec->bestshot_d.exp_comp_val);
    rc = aec_set_iso_mode(sproc, aec, aec->bestshot_d.iso);
    rc = aec_set_exp_metering_mode(sproc, aec, aec->bestshot_d.metering_type);
    rc = aec_set_exposure_compensation(sproc, aec, aec->bestshot_d.exp_comp_val);
  }
  aec->bestshot_d.curr_mode = new_mode;
  return rc;
} /* aec_set_bestshot_mode */

/*==========================================================================
 * FUNCTION    - aec_set_full_frame_exp -
 *
 * DESCRIPTION: change aec to full frame exposure in order to pre-fire
 *              strobe flash
 * ASSUMPTION:  the current aec result already saved to strobe_est
 *=========================================================================*/
void aec_set_full_frame_exp(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t full_frame_line_cnt = aec->
    exp_tbl_ptr[sproc->input.chromatix->fix_fps_aec_table_index].line_count;

  if (sproc->share.aec_ext.cur_line_cnt < full_frame_line_cnt) {

    sproc->share.aec_ext.cur_line_cnt = full_frame_line_cnt;
    sproc->share.aec_ext.cur_real_gain  = sproc->share.aec_ext.
      cur_real_gain * (float)aec->flash.strb_est.linecount_off /
      (float)sproc->share.aec_ext.cur_line_cnt;
  }
  sproc->share.aec_ext.strobe_cfg_st = STROBE_TO_BE_CONFIGURED;
} /* aec_set_full_frame_exp */
