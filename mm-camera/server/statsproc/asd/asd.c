/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "chromatix.h"
#include "stats_proc.h"
#include "stats_proc_interface.h"
#include "asd.h"

/*==========================================================================
 * FUNCTION    - asd_init_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
void asd_init_data(stats_proc_t *sproc, asd_t *asd)
{
  chromatix_parms_type *cptr = sproc->input.chromatix;
  memset(asd, 0, sizeof(asd_t));
  memset(&(asd->gw_mode), -1, sizeof(asd_mode_t));
  memset(&(asd->ww_mode), -1, sizeof(asd_mode_t));
  memset(&(sproc->share.asd_ext), 0, sizeof(stats_proc_asd_data_t));
  sproc->share.asd_ext.soft_focus_dgr = 1.0;

  asd->bl_thld.low = cptr->backlit_scene_detect.low_luma_threshold;
  asd->bl_thld.high = cptr->backlit_scene_detect.high_luma_threshold;
  asd->bl_thld.low_pct
    = cptr->backlit_scene_detect.low_luma_count_percent_threshold / 100;
  asd->bl_thld.high_pct
    = cptr->backlit_scene_detect.high_luma_count_percent_threshold /100;

} /* asd_init_data */

/*==========================================================================
 * FUNCTION    - asd_histogram_backlight_detect -
 *
 * DESCRIPTION:
 *=========================================================================*/
void asd_histogram_backlight_detect(stats_proc_t *sproc, asd_t *asd)
{
  uint32_t i, lowsum = 0, highsum = 0, allsum = 0;

  /*Histogram detection for backlight detection */
  for (i = 0; i < asd->bl_thld.low; i++)
    lowsum += sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.ihist_op.histogram[i];

  for (i = asd->bl_thld.high; i < 256; i++)
    highsum += sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.ihist_op.histogram[i];

  allsum = lowsum + highsum;
  for (i = asd->bl_thld.low; i < asd->bl_thld.high; i++)
    allsum += sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.ihist_op.histogram[i];

  CDBG_LOW("%s: sum: low %d, high %d, all %d", __func__, lowsum, highsum,
    allsum);
  CDBG_LOW("%s: threshold: low_pct %f, high_pct %f low %d high %d\n",
    __func__, asd->bl_thld.low_pct, asd->bl_thld.high_pct,
    asd->bl_thld.low, asd->bl_thld.high);

  /* severity is determined by the amount of samples below and above
   * the high/low threshold pct */
  if (lowsum > ((float) allsum * asd->bl_thld.low_pct) ||
    highsum  > ((float) allsum * asd->bl_thld.high_pct)) {
    /*percent high contrast samples = (sumlow + sumhigh)/sumall */
    float hi_lo_pct = asd->bl_thld.low_pct + asd->bl_thld.high_pct;
    int hi_lo_sum = lowsum + highsum;

    asd->histo_backlight_scene_severity = (uint32_t)(((float)hi_lo_sum /
      allsum - hi_lo_pct) / (1 - hi_lo_pct) * 255);

    asd->histo_backlight_detected = TRUE;

    CDBG_LOW("%s: hi_lo_sum=%d, sumall=%d, hi_lo_pct=%d\n", __func__,
      hi_lo_sum, allsum, (int)(hi_lo_pct * 100));
  } else {
    asd->histo_backlight_scene_severity = 0;
    asd->histo_backlight_detected = FALSE;
  }
  CDBG_HIGH("%s: backlight histogram severity = %d", __func__,
    asd->histo_backlight_scene_severity);
  return;
} /* asd_histogram_backlight_detect */

/*==========================================================================
 * FUNCTION    - asd_calc_mode -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void asd_calc_mode(int cnt, uint32_t array[], int *mode, int *mode2)
{
  int i;
  if (cnt >= 1) {
    for (i = 0; i < 10;) {
      if (array[i] > array[*mode]) {
        mode2 = mode;
        *mode = i;
      }
      i++;
    }
  } else {
    *mode = -1;
    *mode2 = -1;
  }
} /* asd_calc_mode */

/*==========================================================================
 * FUNCTION    - asd_check_mixed_light -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void asd_check_mixed_light(asd_t *asd, int drk_m, int mid_m,
  int mid_m2, int brit_m, int brit_m2,
  uint32_t mid_rgn_cnt[], uint32_t brit_rgn_cnt[])
{
  if (drk_m != -1 && brit_m!= -1 && drk_m != brit_m) {
    CDBG_LOW("%s: Case 1", __func__);
    asd->mixed_light = TRUE;
  } else if (drk_m != -1 && mid_m!= -1 && drk_m != mid_m) {
    CDBG_LOW("%s: Case 2", __func__);
    asd->mixed_light = TRUE;
  } else if (mid_m != -1 && brit_m!= -1 && mid_m != brit_m) {
    CDBG_LOW("%s: Case 3", __func__);
    asd->mixed_light = TRUE;
  } else if (asd->gw_mode.dark != -1 && asd->ww_mode.dark!= -1 &&
    asd->gw_mode.dark != asd->ww_mode.dark) {
    /* Detect also different WB decisions between white and grey worlds */
    CDBG_LOW("%s: Case 4", __func__);
    asd->mixed_light = TRUE;
  } else if (asd->gw_mode.dark != -1 && asd->ww_mode.mid!= -1 &&
    asd->gw_mode.dark != asd->ww_mode.mid) {
    CDBG_LOW("%s: Case 5", __func__);
    asd->mixed_light = TRUE;
  } else if (asd->gw_mode.dark != -1 && asd->ww_mode.bright!= -1 &&
    asd->gw_mode.dark != asd->ww_mode.bright) {
    CDBG_LOW("%s: Case 6", __func__);
    asd->mixed_light = TRUE;
  } else if (asd->gw_mode.mid != -1 && asd->ww_mode.mid != -1 &&
    asd->gw_mode.mid != asd->ww_mode.mid) {
    CDBG_LOW("%s: Case 7", __func__);
    asd->mixed_light = TRUE;
  } else if (asd->gw_mode.mid != -1 && asd->ww_mode.bright != -1 &&
    asd->gw_mode.mid != asd->ww_mode.bright) {
    CDBG_LOW("%s: Case 8", __func__);
    asd->mixed_light = TRUE;
  } else if (asd->gw_mode.bright != -1 && asd->ww_mode.bright != -1
    && asd->gw_mode.bright != asd->ww_mode.bright) {
    CDBG_LOW("%s: Case 9", __func__);
    asd->mixed_light = TRUE;
  } else if (mid_m != -1 && mid_m2 != -1 && mid_m2 != mid_m &&
    ((float)mid_rgn_cnt[mid_m] / mid_rgn_cnt[mid_m2]) > 0.5) {
    /*mixed light check within same catalogged brightness, dark, mid, bright*/
    CDBG_LOW("%s: Case 10", __func__);
    asd->mixed_light = TRUE;
  } else if (brit_m != -1 && brit_m2 != -1 && brit_m != brit_m2 &&
    ((float)brit_rgn_cnt[brit_m] / brit_rgn_cnt[brit_m2]) > 0.5) {
    CDBG_LOW("%s: Case 11", __func__);
    asd->mixed_light = TRUE;
  } else {
    CDBG_LOW("%s: Failed all test cases", __func__);
    asd->mixed_light = FALSE;
  }
} /* asd_check_mixed_light */

/*==========================================================================
 * FUNCTION    - asd_backlight_and_snowscene_detect -
 *
 * DESCRIPTION:
 *=========================================================================*/
void asd_backlight_and_snowscene_detect(stats_proc_t *sproc, asd_t *asd)
{
  uint32_t cur_backlight_scene_severity, AY64[64];
  uint32_t cnt = 0, dark_cnt = 0, brit_cnt  =0, mid_cnt = 0, index, i = 0;
  uint32_t dark_rgn_wb_dec_cnt[10], bright_rgn_wb_dec_cnt[10];
  uint32_t mid_rgn_wb_dec_cnt[10];
  uint32_t cloudy_snow_cnt = 0;
  uint32_t very_bright_no_awb_cldy_snow_cnt = 0;
  uint32_t temp_luma_offset = 0, temp_invalid_awb_cnt = 0;

  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  chromatix_parms_type *cptr = sproc->input.chromatix;

  sproc->share.backlight_luma_target_offset = 0;

  const uint32_t y_cloudy_snow_threshold       =
    cptr->snow_scene_detect.y_cloudy_snow_threshold;
  const uint32_t awb_y_max_in_grey             =
    cptr->snow_scene_detect.awb_y_max_in_grey;
  const uint32_t min_snow_cloudy_sample_th     =
    cptr->snow_scene_detect.min_snow_cloudy_sample_th;
  const uint32_t extreme_snow_cloudy_sample_th =
    cptr->snow_scene_detect.extreme_snow_cloudy_sample_th;
  const uint32_t extreme_luma_target_offset    =  (uint32_t)
  (cptr->snow_scene_detect.extreme_luma_target_offset);
  const uint32_t max_backlight_luma_offset     = (uint32_t) (cptr->
    backlit_scene_detect.backlight_max_la_luma_target_adj *
    cptr->default_luma_target);

  /*Backlight detection:
   * 1. check for backlight hist flag detector to be TRUE,
   *    if FALSE return FALSE.
   * 2. Verify there are at least two different illum AWB detections
   *    in stat_sample_decision.
   * 3. Remap item 2 to AEC stats to correlate with brighter and
   *    darker regions
   * 4. Report detection result sd_output.backlight_detected; */
  memset (&dark_rgn_wb_dec_cnt, 0, sizeof(uint32_t) * 10);
  memset (&bright_rgn_wb_dec_cnt, 0, sizeof(uint32_t) * 10);
  memset (&mid_rgn_wb_dec_cnt, 0, sizeof(uint32_t) * 10);
  asd->mixed_light = FALSE;

  /*converting AE stats from 16x16 to 8x8*/
  if (sproc->input.mctl_info.numRegions == 256) {
    uint32_t region=0;
    for (index = 0; index < sproc->input.mctl_info.numRegions;) {
      /* Combine 16x16 regions spatially to generate 8x8 stats */
      AY64[region]  = (stats->aec_op.SY[index] + stats->aec_op.SY[index + 1] +
        stats->aec_op.SY[index + 16] + stats->aec_op.SY[index + 16 + 1]) >> 2;
      AY64[region] = AY64[region] / sproc->input.mctl_info.pixelsPerRegion;
      region++;

      index += 2; /* jump to the next even horizontal regions */
      if ((index & 0x0F) == 0x0)
        index += 16; /* jump to the next even vertical regions */
    }
  } else if (sproc->input.mctl_info.numRegions == 64) {
    for (index = 0; index < sproc->input.mctl_info.numRegions; index++)
      AY64[index] = stats->aec_op.SY[index] / sproc->input.mctl_info.pixelsPerRegion;

  } else
    return;
  /*continue backlight check else already failed 1st detector*/
  /*check valid awb decision*/
  for (i = 0; i < 64;) {
    if (sproc->share.stat_index_mapping[i] != -1) {
      int tmp_awb_dec;
      uint32_t y_avg;
      tmp_awb_dec = sproc->share.stat_sample_decision[cnt];
      if (tmp_awb_dec > AGW_AWB_MAX_LIGHT || tmp_awb_dec < 0) {
        temp_invalid_awb_cnt++;
        tmp_awb_dec = AGW_AWB_MAX_LIGHT;
      }
      cnt++;
      /*assuming AWB stat regions map directly to AEC stats regions*/
      y_avg = AY64[i];
      if (asd->histo_backlight_detected && asd->asd_enable) {
        if (y_avg < asd->bl_thld.low &&
          tmp_awb_dec < AGW_AWB_MAX_LIGHT) {
          dark_cnt++;
          dark_rgn_wb_dec_cnt[tmp_awb_dec]++;
        } else if (y_avg > (sproc->share.aec_ext.target_luma * 2.5) &&
          tmp_awb_dec<AGW_AWB_MAX_LIGHT) {
          brit_cnt++;
          bright_rgn_wb_dec_cnt[tmp_awb_dec]++;
        } else if (tmp_awb_dec < AGW_AWB_MAX_LIGHT) {
          mid_cnt++;
          mid_rgn_wb_dec_cnt[tmp_awb_dec]++;
        }
      }
      /* Probable Snow/cloudy samples counted */
      if (y_avg > y_cloudy_snow_threshold && (tmp_awb_dec ==
        AGW_AWB_OUTDOOR_SUNLIGHT || tmp_awb_dec == AGW_AWB_OUTDOOR_CLOUDY
        || tmp_awb_dec == AGW_AWB_OUTDOOR_SUNLIGHT1 || tmp_awb_dec ==
        AGW_AWB_OUTDOOR_NOON || tmp_awb_dec == AGW_AWB_HYBRID)) {
        cloudy_snow_cnt++;
        CDBG_LOW("%s: count_cloudy_snow_sample increased = %d, i=%d\n",
          __func__, cloudy_snow_cnt, i);
      } else
        CDBG_LOW("%s: No snow scene tmp_awb_dec =%d, i =%d, y_avg=%d\n",
          __func__, tmp_awb_dec, i, y_avg);
    } else {
      /* No WB decision for this rgn, chk if too bright in grey==TRUE,
      then could be snow or cloudy sample, may further check next time
      same rgn when grey==FALSE to see if decision is made on same region. */
      if (AY64[i] > awb_y_max_in_grey) {
        very_bright_no_awb_cldy_snow_cnt++;
        CDBG_HIGH("%s: cnt_very_bright_no_awb_snow_samples=%d, i=%d\n",
          __func__, very_bright_no_awb_cldy_snow_cnt, i);
      } else
        CDBG_LOW("%s: No De: y_avg=%d, y_cloudy_snow_thrd=%d, region=%d\n",
          __func__, AY64[i], y_cloudy_snow_threshold, i);
    }
    i++;
  } /*for (i=0; i<64;)*/
  CDBG_HIGH("%s: temp_invalid_awb_cnt %d", __func__, temp_invalid_awb_cnt);
  /*Snow cloudy detection only for grey world WB stats*/
  if (sproc->share.grey_world_stats)
    CDBG_LOW("%s:cldy_snow_smple %d_very_bright_no_awb_cldy_snow_smple %d",
      __func__, cloudy_snow_cnt, very_bright_no_awb_cldy_snow_cnt);

  CDBG_LOW("%s: dark_cnt %d, brit_cnt %d, mid_cnt %d ", __func__,
    dark_cnt, brit_cnt,mid_cnt);
  /*test for decision of bright vs. dark and bright vs. mid and mid
   *vs. dark.use "mode" of each array of decisions to see most common
   *one to compare against;
   * any of these can ptentially flag a backlight situation
   * take into account white vs. grey world awb decision. */
  if ((dark_cnt > 0 || brit_cnt > 0 || mid_cnt > 0) &&
    asd->histo_backlight_detected) {
    int dark_mode = 0, bright_mode = 0, mid_mode = 0;
    int dark_mode2 = 0, bright_mode2 = 0, mid_mode2 = 0;
    /*find most common AWB decision for darker regions, the mode*/
    asd_calc_mode(dark_cnt, dark_rgn_wb_dec_cnt, &dark_mode, &dark_mode2);
    /*find most common AWB decision for bright regions */
    asd_calc_mode(brit_cnt, bright_rgn_wb_dec_cnt, &bright_mode,&bright_mode2);
    /*find most common AWB decision for medium regions */
    asd_calc_mode(mid_cnt, mid_rgn_wb_dec_cnt, &mid_mode, &mid_mode2);

    CDBG_LOW("%s: dark_mode=%d, bright_mode=%d, mid_mode=%d", __func__,
      dark_mode, bright_mode, mid_mode);
    CDBG_LOW("%s: Rgns with valid WB data bins cnt %d", __func__, cnt);
    if (sproc->share.grey_world_stats) {
      asd->gw_mode.dark   = dark_mode;
      asd->gw_mode.mid    = mid_mode;
      asd->gw_mode.bright = bright_mode;
    } else {
      asd->ww_mode.dark   = dark_mode;
      asd->ww_mode.mid    = mid_mode;
      asd->ww_mode.bright = bright_mode;
    }
    /* Detect if WB decision modes differ between dark and mid or dark & bright
     * or mid and bright. if they do, this indicates it could be backlight
     * situation.After this, add some history tracking for stability. */
    asd_check_mixed_light(asd, dark_mode, mid_mode, mid_mode2,
      bright_mode, bright_mode2, mid_rgn_wb_dec_cnt, bright_rgn_wb_dec_cnt);
  }
  /*Verify to allow luma-target increase for backlight case, only check
   *over-exposure, (no under-exposure), and very dark condition */
  if (sproc->share.aec_ext.comp_luma - sproc->input.chromatix->
    luma_tolerance > (int)sproc->share.aec_ext.target_luma || sproc->
    share.aec_ext.exp_index >=(int)sproc->share.aec_ext.exp_tbl_val - 1) {
    CDBG_HIGH("AEC maxed out or not reaching luma target due to not settled");
  }
  if (!asd->mixed_light)
    asd->no_backlight_cnt++;
  else
    asd->no_backlight_cnt = 0;

  if (asd->no_backlight_cnt > 2) {
    sproc->share.asd_ext.backlight_detected = FALSE;
    cur_backlight_scene_severity = 0;
  } else {
    sproc->share.asd_ext.backlight_detected = TRUE;
    cur_backlight_scene_severity = asd->histo_backlight_scene_severity;
  }
  sproc->share.asd_ext.backlight_scene_severity =
    (cur_backlight_scene_severity +
    sproc->share.asd_ext.backlight_scene_severity * 4) / 5;
  sproc->share.backlight_luma_target_offset = (uint32_t)
  max_backlight_luma_offset * sproc->share.asd_ext.
    backlight_scene_severity / 255;

  /*Clear stat_index_mapping so detector only runs after AWB runs.*/
  CDBG_HIGH("%s:cldy_snow_cnt %d,brit_no_awb_cldy_snow_smpls %d",
    __func__, cloudy_snow_cnt, very_bright_no_awb_cldy_snow_cnt);
  CDBG_HIGH("%s:backlight_detected %d backlight_luma_target_off %d", __func__,
    sproc->share.asd_ext.backlight_detected,
    sproc->share.backlight_luma_target_offset);

  memset (sproc->share.stat_index_mapping, -1, sizeof(int32_t) * 64);
  /*snow or cloudy scene detection, don't update for white world*/
  if (sproc->share.grey_world_stats) {
    if (cloudy_snow_cnt + very_bright_no_awb_cldy_snow_cnt >
      min_snow_cloudy_sample_th) {
      float interpolate, cnt, interpolate_exp_index;
      int offset_exp_index = 0;
      CDBG_LOW("%s:offset_exp_index=%d, exp_index=%d, outdoor_index=%ld ",
        __func__, offset_exp_index, sproc->share.aec_ext.exp_index,
        sproc->input.chromatix->aec_outdoor_index);
      if (sproc->share.aec_ext.exp_index <
        sproc->input.chromatix->aec_indoor_index) {
        sproc->share.asd_ext.snow_or_cloudy_scene_detected = TRUE;
        /*Interpolate offset compensation by how bright the scene is,
          based on exp_idx, snow scene detection only works outdoors,
          this reduces gradually total offset that can be made on luma target*/
        if (sproc->share.aec_ext.exp_index >
          sproc->input.chromatix->aec_outdoor_index) {
          interpolate_exp_index = sproc->share.aec_ext.exp_index -
            sproc->input.chromatix->aec_outdoor_index;
          interpolate_exp_index = interpolate_exp_index / (sproc->input.
            chromatix->aec_indoor_index - sproc->input.
            chromatix->aec_outdoor_index);
          offset_exp_index = (int) (interpolate_exp_index *
            extreme_luma_target_offset + 0.5);
          CDBG_LOW("%s: interpolate_exp_index x 100 %d", __func__,
            (int)(interpolate_exp_index * 100));
        }
        cnt = cloudy_snow_cnt + very_bright_no_awb_cldy_snow_cnt;
        if (cnt > extreme_snow_cloudy_sample_th)
          cnt = extreme_snow_cloudy_sample_th;

        /*interpolate offset based on amount of snow samples retrieved
          from the scene*/
        interpolate = (cnt - min_snow_cloudy_sample_th) /
          (extreme_snow_cloudy_sample_th - min_snow_cloudy_sample_th);
        CDBG_LOW("snow interpolate x100 =%d",(int)(interpolate * 100));
        temp_luma_offset = (uint8_t)(interpolate * (
          extreme_luma_target_offset - offset_exp_index) + 0.5);
      } else {
        sproc->share.asd_ext.snow_or_cloudy_scene_detected = FALSE;
        CDBG_LOW("Darker than lux_idx = %d",
          (int)((sproc->input.chromatix->aec_indoor_index +
          sproc->input.chromatix->aec_outdoor_index) / 2));
      }
    } else
      sproc->share.asd_ext.snow_or_cloudy_scene_detected = FALSE;

    /*use 20% of the new offset*/
    sproc->share.snow_or_cloudy_luma_target_offset = (temp_luma_offset
      + sproc->share.snow_or_cloudy_luma_target_offset * 4) / 5;
  }
  if (!asd->asd_enable) {
    sproc->share.asd_ext.snow_or_cloudy_scene_detected = FALSE;
    sproc->share.snow_or_cloudy_luma_target_offset = 0;
    CDBG_HIGH("%s: Snow Scene is disabled\n", __func__);
  }
  CDBG_LOW("%s: snow_or_cldy_scene detect %d luma tgt offset %d, no_tf %d",
    __func__, sproc->share.asd_ext.snow_or_cloudy_scene_detected,
    sproc->share.snow_or_cloudy_luma_target_offset, temp_luma_offset);
  CDBG_HIGH("%s:backlight detected = %d, severity = %d, cur_severity =%d\n",
    __func__, sproc->share.asd_ext.backlight_detected, sproc->share.asd_ext.
    backlight_scene_severity, cur_backlight_scene_severity);
} /* asd_backlight_and_snowscene_detect */

/*==========================================================================
 * FUNCTION    - asd_landscape_detect -
 *
 * DESCRIPTION:
 *=========================================================================*/
void asd_landscape_detect(stats_proc_t *sproc, asd_t *asd)
{
  float egN_glbN_ratio, ebN_glbN_ratio;
  uint32_t cur_landscape_sev = 0;
  isp_stats_t *stats = &(sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct);
  chromatix_parms_type *cptr = sproc->input.chromatix;

  if (sproc->share.grey_world_stats) {
    if (cptr->landscape_scene_detect.landscape_detection_enable &&
      asd->asd_enable) {
      egN_glbN_ratio = (float)stats->awb_op.Green_N   / stats->awb_op.GLB_N;
      ebN_glbN_ratio = (float)stats->awb_op.ExtBlue_N / stats->awb_op.GLB_N;

      /*Verify extreme stats*/
      CDBG_LOW("%s: GLB_Y %d, GLB_Cb %d, GLB_Cr %d, GLB_N %d", __func__,
        stats->awb_op.GLB_Y, stats->awb_op.GLB_Cb, stats->awb_op.GLB_Cr, stats->awb_op.GLB_N);
      CDBG_LOW("%s: Green_R %d, Green_G %d,Green_B %d, Green_N %d", __func__,
        stats->awb_op.Green_R, stats->awb_op.Green_G, stats->awb_op.Green_B, stats->awb_op.Green_N);
      CDBG_LOW("%s: ExtBlue_R %d, ExtBlue_G %d,ExtBlue_B %d, ExtBlue_N %d",
        __func__, stats->awb_op.ExtBlue_R, stats->awb_op.ExtBlue_G, stats->awb_op.ExtBlue_B,
        stats->awb_op.ExtBlue_N);
      CDBG_LOW("%s: ExtRed_R %d, ExtRed_G %d,ExtRed_B %d, ExtRed_N %d", 
        __func__, stats->awb_op.ExtRed_R, stats->awb_op.ExtRed_G, stats->awb_op.ExtRed_B,
        stats->awb_op.ExtRed_N);
      CDBG_LOW("%s: green_GN_Ratio_MCE %f, extblue_GN_Ratio_MCE %f", __func__,
        egN_glbN_ratio,ebN_glbN_ratio );
      /*Green samples are much less due to bounding box config*/
      if ((egN_glbN_ratio + ebN_glbN_ratio) > cptr->landscape_scene_detect.
        min_blue_green_content_detection_threshold) {
        cur_landscape_sev = ((egN_glbN_ratio + ebN_glbN_ratio - cptr->
          landscape_scene_detect.min_blue_green_content_detection_threshold) /
          (cptr->landscape_scene_detect.
          max_blue_green_content_detecton_threshold - cptr->
          landscape_scene_detect.
          min_blue_green_content_detection_threshold)) * 255;

        cur_landscape_sev = STATS_PROC_MIN(cur_landscape_sev, 255);
      } else
        cur_landscape_sev = 0;

      if (sproc->share.aec_ext.lux_idx > cptr->
        landscape_scene_detect.lux_idx_indoor)
        cur_landscape_sev = 0;
      else if (sproc->share.aec_ext.lux_idx >
        cptr->landscape_scene_detect.lux_idx_outdoor)
        cur_landscape_sev =  (int)(cur_landscape_sev *
          (float)((cptr->landscape_scene_detect.lux_idx_indoor) -
          sproc->share.aec_ext.lux_idx) / (cptr->landscape_scene_detect.
          lux_idx_indoor - cptr->landscape_scene_detect.lux_idx_outdoor));
    } else
      cur_landscape_sev = 0;
  }
  sproc->share.asd_ext.landscape_severity = (uint32_t)((float)
    cur_landscape_sev * cptr->landscape_scene_detect.aggressiveness +
    (float)sproc->share.asd_ext.landscape_severity * (1.0 - cptr->
    landscape_scene_detect.aggressiveness));
  if (sproc->share.asd_ext.landscape_severity > 255)
    sproc->share.asd_ext.landscape_severity = 255;
  CDBG_HIGH("%s:LANDSPACE: severity %d curr severity %d lux_id %f", __func__,
    sproc->share.asd_ext.landscape_severity, cur_landscape_sev,
    sproc->share.aec_ext.lux_idx);
} /* asd_landscape_detect */

/*==========================================================================
 * FUNCTION    - asd_portrait_detect -
 *
 * DESCRIPTION:
 *=========================================================================*/
void asd_portrait_detect(stats_proc_t *sproc, asd_t *asd)
{
  uint32_t roi_size[MAX_ROI], roi_index, frame_size;
  uint32_t face_area = 0, max_face_area =0, min_face_area =0;
  float face_to_image_size_ratio = 0, portrt_sev = 0;
  int portrait_detected = FALSE, cur_portrt_sev = 0;

  chromatix_parms_type *cptr = sproc->input.chromatix;

  if (cptr->portrait_scene_detect.portrait_detection_enable &&
    asd->asd_enable) {
    if (sproc->share.fd_roi.num_roi <= 0) {
      asd->portrait_face_delay_cnt++;
      if (asd->portrait_face_delay_cnt > 30)
        sproc->share.asd_ext.portrait_severity =
          (uint32_t)(sproc->share.asd_ext.portrait_severity *
          (1 - cptr->portrait_scene_detect.aggressiveness));

      CDBG_HIGH("%s: portrait: no_face = %d", __func__,
        sproc->share.asd_ext.portrait_severity);
      return;
    }
    asd->portrait_face_delay_cnt = 0;

    if (sproc->share.fd_roi.type != ROI_TYPE_GENERAL &&
      sproc->share.fd_roi.roi_updated) {
      frame_size = sproc->share.fd_roi.frm_width * sproc->share.fd_roi.frm_height;
      min_face_area = frame_size;
      if (frame_size > 0) {
        for (roi_index = 0; roi_index < sproc->share.fd_roi.num_roi;
          roi_index++) {
          roi_size[roi_index] = (uint32_t)(sproc->share.fd_roi.
            roi[roi_index].dx * sproc->share.fd_roi.roi[roi_index].dy);
          face_area = face_area + roi_size[roi_index];
          if (roi_size[roi_index]> max_face_area)
            max_face_area = roi_size[roi_index];
          if (roi_size[roi_index] < min_face_area)
            min_face_area = roi_size[roi_index];

          if ((float)roi_size[roi_index] / (float)frame_size >
            cptr->portrait_scene_detect.min_face_content_threshold) {
            portrait_detected = TRUE;
            CDBG_HIGH("%s:portrait detected by face size", __func__);
          }
        }
        if (portrait_detected)
          portrt_sev = (float)face_area / (float)frame_size;
        else if (sproc->share.fd_roi.num_roi > 1) {
          /* at least two faces:  verify if face size are ratios are all
           * over the place, if so, faces are located at different
           * distances, not a portait. if max_face_size/min_face_size > 4,
           * then don't consider a portrait if no face area is big enough. */
          if (max_face_area/min_face_area > 4.0)
            portrt_sev = 0;
          else
            portrt_sev = (float)face_area / (float)frame_size;
        }
        CDBG_HIGH("%s:PORTRT:face_area %d, sev %f, frame_size %d, faces cnt %d",
          __func__, face_area, portrt_sev, frame_size, sproc->share.fd_roi.num_roi);
        if (portrt_sev > cptr->portrait_scene_detect.
          min_face_content_threshold) {
          if (portrt_sev < cptr->portrait_scene_detect.
            max_face_content_threshold) {
            /* interpolate */
            portrt_sev = 255.0 * (portrt_sev - cptr->portrait_scene_detect.
              min_face_content_threshold) / (cptr->portrait_scene_detect.
              max_face_content_threshold - cptr->portrait_scene_detect.
              min_face_content_threshold);
            CDBG_LOW("%s:portrtsev range %f", __func__, portrt_sev);
          } else {
            portrt_sev = 255.0;
            CDBG_LOW("%s:portrait_sev_above_max %f", __func__, portrt_sev);
          }
        } else {
          portrt_sev = 0.0;
          CDBG_LOW("%s: portrait_sev_below_min %f", __func__, portrt_sev);
        }
        cur_portrt_sev = STATS_PROC_CAP_UINT8(portrt_sev);
      }
    }
  }
  sproc->share.asd_ext.portrait_severity = (uint32_t)((float)sproc->share.
    asd_ext.portrait_severity *
    (1.0 - cptr->portrait_scene_detect.aggressiveness) +
    (float)cur_portrt_sev * cptr->portrait_scene_detect.aggressiveness);

  sproc->share.asd_ext.portrait_severity =
    STATS_PROC_MIN(sproc->share.asd_ext.portrait_severity, 255);

  sproc->share.asd_ext.soft_focus_dgr = 1.0 - (float)sproc->share.asd_ext.
    portrait_severity * (1.0 - cptr->portrait_scene_detect.soft_focus_degree) /
    255.0;

  sproc->share.asd_ext.soft_focus_dgr = STATS_PROC_CAP(sproc->share.asd_ext.
    soft_focus_dgr, 0, 1.0);

  CDBG_HIGH("%s: portrait_severity %d, soft_focus_dgr %f", __func__, sproc->
    share.asd_ext.portrait_severity, sproc->share.asd_ext.soft_focus_dgr);
} /* asd_portrait_detect */
