/**********************************************************************
  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include <sys/time.h>
#include <media/msm_camera.h>
#include "camera.h"
#include "stats_proc.h"
#include "aec.h"

#define THRESHOLD_HOLD_LOW          6
#define THRESHOLD_HOLD_HIGH       215
#define MIN_LIMIT                   5
#define MAX_LIMIT                   2
#define MIN_LUMA_SLOW               1
#define MAX_LUMA_SLOW             240
#define SKIP_RANGE_LOW              1
#define SKIP_RANGE_HIGH           235
#define MAX_PREVIEW_EXP_TIME        0.033

#define LOG10_1P03 0.012837224705f
#define AEC_Q7     0x00000080

/*==========================================================================
 * FUNCTION    - aec_test_MOTION_ISO -
 *
 * DESCRIPTION: fucntion for testing motion iso
 *=========================================================================*/
static float aec_test_MOTION_ISO(aec_t *aec, float mtn_thld)
{
  if (aec->m_iso_test.frame_cnt % 30 == 0) {
    if (aec->m_iso_test.motion_up) {
      aec->m_iso_test.mtn = aec->m_iso_test.mtn * 1.2;
      if (aec->m_iso_test.mtn >= 16.0) {
        aec->m_iso_test.mtn = 16.0;
        aec->m_iso_test.motion_up = FALSE;
      }
    } else {
      aec->m_iso_test.mtn = aec->m_iso_test.mtn / 1.2;
      if (aec->m_iso_test.mtn <= 0.7) {
        aec->m_iso_test.mtn = 0.7;
        aec->m_iso_test.motion_up = TRUE;
      }
    }
  }
  aec->m_iso_test.frame_cnt++;
  return aec->m_iso_test.mtn * mtn_thld;
} /* aec_test_MOTION_ISO */

/*==========================================================================
 * FUNCTION    - aec_test_ROI -
 *
 * DESCRIPTION: fucntion for testing ROI
 *=========================================================================*/
static void aec_test_ROI(stats_proc_t *sproc, aec_t *aec)
{
  stats_proc_interested_region_t interested_region;

  /*ausme 30fps, update every 45seconds*/
  if ((aec->roi_test.fr_cnt + 140) % 150 == 0) {
    interested_region.enable = TRUE;
    interested_region.rgn_index = aec->roi_test.cur_num;
    CDBG_AEC("%s: aec_test_roi ROI Index %d", __func__,
      interested_region.rgn_index);
    //aec_set_ROI(sproc, aec, interested_region);
    aec->roi_test.cur_num++;
    if (aec->roi_test.cur_num >= 256)
      aec->roi_test.cur_num =0;
  }
  aec->roi_test.fr_cnt++;
} /* aec_test_ROI */

/*==========================================================================
 * FUNCTION    - aec_calc_lux_index -
 *
 * DESCRIPTION:  Returns calculated lux index.
 *=========================================================================*/
static float aec_calc_lux_index(stats_proc_t *sproc, aec_t *aec)
{
  float lux_index, dbg_lux_idx, new_luma, fps_factor, fps;

  fps        = (float) aec->afr_frame_rate / 256.0f;
  fps_factor =  sproc->input.chromatix->max_video_fps / fps;

  if (sproc->input.chromatix->linear_afr_support ||
    sproc->input.chromatix->max_video_fps == 0)
    fps_factor = 1.0;

  new_luma = (aec->luma.lux_index - sproc->input.isp_info.blk_inc_comp) / 16;
  CDBG_AEC("%s: exp_index %d, new_luma %f, luma_target %d, "
    "linear_afr_support %d, fps %f, max_fps %f, blk_increase %f",
    __func__, sproc->share.aec_ext.exp_index, new_luma, aec->luma.
    comp_target, sproc->input.chromatix->linear_afr_support, fps,
    sproc->input.chromatix->max_video_fps,
    (sproc->input.isp_info.blk_inc_comp / 16));

  if (new_luma <= 1)
    new_luma = 1;

  dbg_lux_idx = log10((((float)aec->luma.comp_target) /
    new_luma) * fps_factor) / LOG10_1P03;
  lux_index = sproc->share.aec_ext.exp_index + dbg_lux_idx;
  if(lux_index < 0)
    lux_index = 0;
  CDBG_AEC("%s: lux_index %f", __func__, lux_index);
  return lux_index;
} /* aec_calc_lux_index */

/*==========================================================================
 * FUNCTION    - aec_dark_region_reduction_config -
 *
 * DESCRIPTION:  Configure reduction rate based on gamma table
 *=========================================================================*/
static void aec_dark_region_reduction_config(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t index, threshold0_slope, threshold1_slope;
  uint32_t threshold2_slope, threshold3_slope, last_slope;
  float    weight;

  /* calculating slope of gamma at each threshold */
  threshold0_slope = sproc->input.chromatix->
    chromatix_rgb_default_gamma_table.gamma[((1 + 1) * 4 - 1)];

  index = sproc->input.chromatix->default_luma_target *
    sproc->input.chromatix->dark_region.threshold_LO + 0.5;
  threshold1_slope = sproc->input.chromatix->chromatix_rgb_default_gamma_table.
    gamma[((index + 1) * 4 - 1)] - sproc->input.chromatix->
    chromatix_rgb_default_gamma_table.gamma[((index) * 4 - 1)];

  index = sproc->input.chromatix->default_luma_target * (1.0 / 3.0 * (aec->
    dark_rgn_discard_thld - sproc->input.chromatix->dark_region.
    threshold_LO) + sproc->input.chromatix->dark_region.threshold_LO) + 0.5;

  threshold2_slope = sproc->input.chromatix->chromatix_rgb_default_gamma_table.
    gamma[((index + 1) * 4 - 1)] - sproc->input.chromatix->
    chromatix_rgb_default_gamma_table.gamma[((index) * 4 - 1)];

  index = sproc->input.chromatix->default_luma_target * (2.0 / 3.0 *
    (aec->dark_rgn_discard_thld - sproc->input.chromatix->dark_region.
    threshold_LO) + sproc->input.chromatix->dark_region.threshold_LO) + 0.5;

  threshold3_slope = sproc->input.chromatix->chromatix_rgb_default_gamma_table.
    gamma[((index + 1) * 4 - 1)] - sproc->input.chromatix->
    chromatix_rgb_default_gamma_table.gamma[((index) * 4 - 1)];

  index = sproc->input.chromatix->default_luma_target *
    sproc->input.chromatix->dark_region.threshold_HI + 0.5;
  last_slope = sproc->input.chromatix->chromatix_rgb_default_gamma_table.
    gamma[((index + 1) * 4 - 1)] - sproc->input.chromatix->
    chromatix_rgb_default_gamma_table.gamma[((index) * 4 - 1)];  /* 2 */

  if (last_slope >= threshold0_slope) { /* Dark region reduction not applied
                         * on gammas where dark areas are already reduced */
    aec->dark_reduction0 = 1;
    aec->dark_reduction1 = 1;
    aec->dark_reduction2 = 1;
    aec->dark_reduction3 = 1;
  } else { /* 3.33 */
    weight = 1 / (1 / (float) last_slope - 1 / (float) threshold0_slope);
    aec->dark_reduction0 = 0;
    aec->dark_reduction1 = weight *
      (1 / (float) threshold1_slope - 1 / (float) threshold0_slope);
    aec->dark_reduction2 = weight *
      (1 / (float) threshold2_slope - 1 / (float) threshold0_slope);
    aec->dark_reduction3 = weight *
      (1 / (float) threshold3_slope - 1 / (float) threshold0_slope);
  }
} /* aec_dark_region_reduction_config */

/*==========================================================================
 * FUNCTION    - aec_calc_bright_region_threshold -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_calc_bright_region_threshold(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t threshold, range;
  float ratio;
  threshold = (uint32_t)(sproc->input.chromatix->bright_region.threshold_LO);
  /* range is now determined by lux_idx_lo and hi for each segment. */
  if (sproc->share.aec_ext.lux_idx < sproc->input.chromatix->
    bright_region.lux_index_HI) { /* very bright scene */
    threshold = (uint32_t)(sproc->input.chromatix->bright_region.threshold_HI);
  } else if (sproc->share.aec_ext.lux_idx < sproc->input.chromatix->
    bright_region.lux_index_LO) {
    /* interpolate threshold HI and LO between lux_index_LO and HI. */
    range = sproc->input.chromatix->bright_region.lux_index_LO -
      sproc->input.chromatix->bright_region.lux_index_HI;
    ratio = (float)(sproc->input.chromatix->bright_region.lux_index_LO -
      sproc->share.aec_ext.lux_idx ) / range;
    threshold = (uint32_t)(threshold * (1 - ratio) +
      (sproc->input.chromatix->bright_region.threshold_HI) * ratio);
  } else if (sproc->share.aec_ext.lux_idx  < (2 * sproc->input.chromatix->
    aec_outdoor_index - sproc->input.chromatix->bright_region.lux_index_LO)) {
    /* interpolate threshold LO and 256 between lux index LO and
     * out_idx+(out_idx-lux_idx_LO) */
    range = 2 * (sproc->input.chromatix->aec_outdoor_index -
      sproc->input.chromatix->bright_region.lux_index_LO);
    ratio = (float)((2 * sproc->input.chromatix->aec_outdoor_index -
      sproc->input.chromatix->bright_region.lux_index_LO) -
      sproc->share.aec_ext.lux_idx) / range;
    threshold = (uint32_t)(256 * (1 - ratio) + threshold * ratio);
  } else
    threshold = 256;

  if (sproc->input.chromatix->bright_region.is_supported == TRUE &&
    !(aec->roi.enable) && (aec->exp_comp_val == 0))
    aec->outdoor_bright_rgn_discard_thld = threshold;
  else
    aec->outdoor_bright_rgn_discard_thld = 256;
} /* aec_calc_bright_region_threshold */

/*==========================================================================
 * FUNCTION    - aec_calc_dark_region_threshold -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_calc_dark_region_threshold(stats_proc_t *sproc, aec_t *aec,
  float *d0, float *d1, float *d2)
{
  float index_ratio, dark_threshold_HI_scale;
  int pixelsPerRegion = sproc->input.mctl_info.pixelsPerRegion;

  aec->dark_rgn_TH_HI_outdoor_index_ratio = 0.5;
  aec->dark_rgn_TH_HI_for_outdoor         = 0.5;

  /* to reduce the effect in outdoor high dynamic scenario */
  if (sproc->share.aec_ext.exp_index < sproc->share.aec_ext.outdoor_index) {
    index_ratio = (float)sproc->share.aec_ext.exp_index /
      sproc->share.aec_ext.outdoor_index;

    if (index_ratio < aec->dark_rgn_TH_HI_outdoor_index_ratio)
      /* TH_HI_for_outdoor specifies the reduction of scale */
      dark_threshold_HI_scale = aec->dark_rgn_TH_HI_for_outdoor;
    else
      dark_threshold_HI_scale = ((1 - aec->dark_rgn_TH_HI_for_outdoor) /
        (1 - aec->dark_rgn_TH_HI_outdoor_index_ratio)) * (index_ratio - aec->
        dark_rgn_TH_HI_outdoor_index_ratio) + aec->dark_rgn_TH_HI_for_outdoor;

    aec->dark_rgn_discard_thld = dark_threshold_HI_scale *
      sproc->input.chromatix->dark_region.threshold_HI;
  } else
    aec->dark_rgn_discard_thld =
    sproc->input.chromatix->dark_region.threshold_HI;

  *d0 = pixelsPerRegion * sproc->share.aec_ext.cur_luma *
    sproc->input.chromatix->dark_region.threshold_LO;
  *d1 = pixelsPerRegion * sproc->share.aec_ext.cur_luma *
    (1.0 / 3.0 * (aec->dark_rgn_discard_thld -
    sproc->input.chromatix->dark_region.threshold_LO) +
    sproc->input.chromatix->dark_region.threshold_LO);
  *d2 = pixelsPerRegion * sproc->share.aec_ext.cur_luma *
    (2.0 / 3.0 * (aec->dark_rgn_discard_thld -
    sproc->input.chromatix->dark_region.threshold_LO) +
    sproc->input.chromatix->dark_region.threshold_LO);
} /* aec_calc_dark_region_threshold */

/*==========================================================================
 * FUNCTION    - aec_calculate_current_metered_luma -
 *
 * DESCRIPTION: Calculates the luma for the preview frame from VFE statistics
 * & based on an input weight table.  This function also explicitly
 * knows about frame_average mode for AEC where no bias weight is applied
 * (i.e. every AEC region is weighed into the current luma calc
 *  evenly).
 * Otherwise, the passed in table of Q8 format numbers is applied
 * to the VFE array of brightness sums.  The table may be center
 * weighted, spot metering, or virtually any other combination.
 *
 *  Applies for 8x8 arrays and 16x16 arrays.
 *==========================================================================*/
static int aec_calculate_current_metered_luma(stats_proc_t *sproc, aec_t *aec)
{
  /* current luma represented as Q4. This is necessary to reduce
   * quantization errors when calculating lux index */
  int i, j, lumaSum = 0, spotLuma = 0, weightSum = 0, temp_bias_weight;
  int non_zero_hregions = 0, non_zero_vregions = 0, shift = 0;
  uint32_t high_luma_rg_thld, high_luma_rg_thld_per_outdoor, low_luma_rgn_thld;
  int discarded_bright_regions = 0;
  int discarded_dark_regions = 0, range, temp_dark_region_weight =0;
  float si_delta; /* Sensitivity Delta */

  /* discarded counts for debugging purpose */
  int drk_rgn0_disc_cnt = 0, drk_rgn1_disc_cnt = 0;
  int drk_rgn2_disc_cnt = 0, drk_rgn3_disc_cnt = 0;
  float ratio, dark_reduction;
  float dark_interpolated_thld0 = 0.0;
  float dark_interpolated_thld1 = 0.0;
  float dark_interpolated_thld2 = 0.0;

  int numRegions    = sproc->input.mctl_info.numRegions;
  int pixPerRgn     = sproc->input.mctl_info.pixelsPerRegion;;
  uint32_t *sumLumaArray = (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;

  CDBG_AEC("%s: numRegions %d, pixelsPerRegion %d", __func__,
    numRegions, pixPerRgn);

  if (sumLumaArray == NULL)
    return 0;

  if (pixPerRgn <= 0)
    return 0;

  range = (sproc->share.aec_ext.indoor_index -
    sproc->share.aec_ext.outdoor_index) / 4;

  sproc->share.aec_ext.lux_idx = aec_calc_lux_index(sproc, aec);
  aec_calc_bright_region_threshold(sproc, aec);

  if (sproc->input.chromatix->dark_region.is_supported && !aec->roi.enable
     && (aec->exp_comp_val == 0)) {
    aec_calc_dark_region_threshold(sproc, aec, &dark_interpolated_thld0,
      &dark_interpolated_thld1, &dark_interpolated_thld2);
    aec_dark_region_reduction_config(sproc, aec);
  } else
    aec->dark_rgn_discard_thld = 0;

  if (numRegions == 64) {
    non_zero_hregions = 8 - sproc->input.mctl_info.zero_hregions * 2;
    non_zero_vregions = 8 - sproc->input.mctl_info.zero_vregions * 2;
    shift = 3;
  } else if (numRegions == 256) {
    non_zero_hregions = 16 - sproc->input.mctl_info.zero_hregions * 2;
    non_zero_vregions = 16 - sproc->input.mctl_info.zero_vregions * 2;
    shift = 4;
  }

  if (sproc->input.mctl_info.zero_hregions != 0 ||
    sproc->input.mctl_info.zero_vregions != 0)
    numRegions = non_zero_hregions * non_zero_vregions;

  /* The bayer data that is considered to be too bright */
  high_luma_rg_thld = pixPerRgn * sproc->input.chromatix->high_luma_region_threshold;
  sproc->share.aec_ext.high_luma_region_count = 0;

  high_luma_rg_thld_per_outdoor = pixPerRgn *
    aec->outdoor_bright_rgn_discard_thld;
  low_luma_rgn_thld = pixPerRgn * aec->dark_rgn_discard_thld *
    sproc->share.aec_ext.cur_luma;

  /* This check for frame average is specifically to avoid
   * having a frame average bias table that contains all ones
   * in it - this just seems like a waste of memory! */
  if (sproc->share.aec_ext.metering_type == CAMERA_AEC_FRAME_AVERAGE) {
    for (i = sproc->input.mctl_info.zero_hregions; i <
      (sproc->input.mctl_info.zero_hregions + non_zero_hregions); i++) {
      for (j = sproc->input.mctl_info.zero_vregions; j <
        (sproc->input.mctl_info.zero_vregions + non_zero_vregions); j++) {
        int idx;
        idx = (i << shift) + j;
        /* Just add 'em up */
        lumaSum += sumLumaArray[idx];
        if (sumLumaArray[i] > high_luma_rg_thld)
          sproc->share.aec_ext.high_luma_region_count++;

        /* Total discarded regions not to exceed 90% of the total regions */
        if ((discarded_bright_regions + discarded_dark_regions) <
          (numRegions * 0.90)) {
          if ((sumLumaArray[idx] > high_luma_rg_thld_per_outdoor) &&
            (discarded_bright_regions < (numRegions * sproc->input.chromatix->
            bright_region.discard_ratio))) {

            discarded_bright_regions++;
            lumaSum -= (uint32_t) (sumLumaArray[idx] *
              (1 - sproc->input.chromatix->bright_region.reduction));
          } else if ((sumLumaArray[idx] < low_luma_rgn_thld) &&
            (discarded_dark_regions < (numRegions *
            sproc->input.chromatix->dark_region.discard_ratio))) {

            discarded_dark_regions++;
            /* 0 = discard completely, 0.5 = discard 50%. etc. */
            if (sumLumaArray[idx] < dark_interpolated_thld0) {
              drk_rgn0_disc_cnt++;
              dark_reduction = aec->dark_reduction0;
            } else if (sumLumaArray[idx] < dark_interpolated_thld1) {
              drk_rgn1_disc_cnt++;
              dark_reduction = aec->dark_reduction1;
            } else if (sumLumaArray[idx] < dark_interpolated_thld2) {
              drk_rgn2_disc_cnt++;
              dark_reduction = aec->dark_reduction2;
            } else {
              drk_rgn3_disc_cnt++;
              dark_reduction = aec->dark_reduction3;
            }

            lumaSum -= (uint32_t) (sumLumaArray[idx] * (1 - dark_reduction));
            temp_dark_region_weight += (uint32_t) (1 - dark_reduction);
          } /* end else if */
        } /* end if */
      } /* end for j loop */
    } /* end for i loop */
    weightSum = ((uint32_t) (numRegions - discarded_bright_regions *
      (1 - sproc->input.chromatix->bright_region.reduction))) << 8;
  } else {
    if (!aec->bias_table)
      return 0;

    for (i = sproc->input.mctl_info.zero_hregions; i <
      (sproc->input.mctl_info.zero_hregions + non_zero_hregions); i++) {
      for (j = sproc->input.mctl_info.zero_vregions; j <
        (sproc->input.mctl_info.zero_vregions + non_zero_vregions); j++) {
        int idx;
        idx = (i << shift) + j;
        /* Multiply each entry by Q8 format bias table and sum  results
         * On the bias table:
         * Table that interprets how the  AEC stats are summed into a
         * current luma  */
        if (sumLumaArray[idx] > high_luma_rg_thld)
          sproc->share.aec_ext.high_luma_region_count++;

        /* Total discarded regions not to exceed 90% of the total regions */
        if (((discarded_bright_regions + discarded_dark_regions) <
            (numRegions * 0.90)) && (sproc->share.aec_ext.
            metering_type != CAMERA_AEC_SPOT_METERING)) {

          if ((sumLumaArray[idx] > high_luma_rg_thld_per_outdoor) &&
              (discarded_bright_regions < (numRegions * sproc->input.
              chromatix->bright_region.reduction)) && (sproc->
              share.aec_ext.metering_type != CAMERA_AEC_SPOT_METERING)) {
            discarded_bright_regions++;
            temp_bias_weight = (aec->bias_table[idx] * 256) *
              sproc->input.chromatix->bright_region.reduction;

          } else if ((sumLumaArray[idx] < low_luma_rgn_thld) &&
              (discarded_dark_regions < (numRegions *
              sproc->input.chromatix->dark_region.discard_ratio))) {
            discarded_dark_regions++;
            /* 0 = discard completely, 0.5 = discard 50%. etc. */
            if (sumLumaArray[idx] < dark_interpolated_thld0) {
              drk_rgn0_disc_cnt++;
              dark_reduction = aec->dark_reduction0;
            } else if (sumLumaArray[idx] < dark_interpolated_thld1) {
              drk_rgn1_disc_cnt++;
              dark_reduction = aec->dark_reduction1;
            } else if (sumLumaArray[idx] < dark_interpolated_thld2) {
              drk_rgn2_disc_cnt++;
              dark_reduction = aec->dark_reduction2;
            } else {
              drk_rgn3_disc_cnt++;
              dark_reduction = aec->dark_reduction3;
            }
            temp_bias_weight = (aec->bias_table[idx] * 256) * dark_reduction;

          } else {
            temp_bias_weight = (aec->bias_table[idx] * 256);
          }
        } else { /* include in stats */
          temp_bias_weight = (aec->bias_table[idx] * 256);
        }

        spotLuma  = (temp_bias_weight * (sumLumaArray[idx] >> 2)); /* uint32_t overflow may happen, bitshift by 2. */
        weightSum = weightSum + temp_bias_weight;

        /* Perform the shift down by 8 before summing, this effectively
         * converts the bias back to a Q0 number.  We lose some
         * precision, but not much after all the shifting to follow */
        lumaSum += (spotLuma >> 6); /* update to bitshif by 6 instead of 8 due to overflow. */
      } /* end for j loop */
    } /* end for i loop */
  }
  weightSum = weightSum >> 8;
  if (weightSum == 0)
    weightSum = 1;

  lumaSum = lumaSum / weightSum;
  /* Finally, divided the weighed average of all
   * regions by the number of pixels per region */
  aec->luma.lux_index = (lumaSum * 16) / (uint32_t)pixPerRgn;
  if (aec->luma.lux_index > (255 * 16))
    aec->luma.lux_index = 255 * 16;

  lumaSum = aec->luma.lux_index >> 4;
  lumaSum *= MAX(1.0,sproc->input.mctl_info.vfe_dig_gain);
  CDBG_HIGH("%s: lumaSum: %d, pixelsPerRegion: %d, weightSum: %d", __func__,
    lumaSum, pixPerRgn, weightSum);

  CDBG_AEC("%s:discrd cnt: drk_rgn0 %d, drk_rgn1 %d, drk_rgn2 %d, drk_rgn3 %d",
    __func__, drk_rgn0_disc_cnt, drk_rgn1_disc_cnt,
    drk_rgn2_disc_cnt, drk_rgn3_disc_cnt);

  CDBG_AEC("%s: discarded_bright_regions %d, discarded_dark_regions %d",
    __func__, discarded_bright_regions, discarded_dark_regions);

  if (lumaSum > 255) {
    CDBG_AEC("%s: Luma sum > expected %d. %d", __func__, lumaSum, numRegions);
    lumaSum = 255;
  }
  return lumaSum;
} /* aec_calculate_current_metered_luma */

/*==========================================================================
 * FUNCTION    - aec_calculate_current_ROI_luma -
 *
 * DESCRIPTION:
 *=========================================================================*/
static int aec_calculate_current_ROI_luma(stats_proc_t *sproc, aec_t *aec)
{
  int lumaSum = 0, j;
  uint32_t i;
  uint32_t *sumLumaArray = (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;

  if (sumLumaArray == NULL)
    return 0;
  else {
    for (i = 0; i < aec->sub_roi.number; i++) {
      j = aec->sub_roi.index[i];
      lumaSum += sumLumaArray[j];
    }
    lumaSum = lumaSum / aec->sub_roi.number;
    return(lumaSum / sproc->input.mctl_info.pixelsPerRegion);
  }
} /* aec_calculate_current_ROI_luma */

/*==========================================================================
 * FUNCTION    - aec_calc_touch_RIO_luma -
 *
 * DESCRIPTION:
 *=========================================================================*/
static uint32_t aec_calc_touch_RIO_luma(stats_proc_t *sproc, aec_t *aec,
  uint32_t roi_luma_array[])
{
  uint32_t i, global_roi_luma = 0, roi_index, white_area_percentile;
  uint32_t roi_size[MAX_ROI];
  float    face_skin_ratio[MAX_ROI];
  stats_proc_roi_info_t *roiInfo  = &(sproc->share.fd_roi);

  /* Beyond this point, ROI's are faces; their histograms are also present */
  for (roi_index = 0; roi_index < roiInfo->num_roi; roi_index++) {
    uint32_t region_cnt;
    uint32_t samples, bright_samples, non_white_bins, white_samples;
    uint32_t sum, white_sum;

    /* calc roi luma post gamma from histogram */
    sum = 0;
    samples = 0;
    white_sum = 0;
    white_samples = 0;

    /* 255 and 254 bins most likely correspond to saturated portions.
     * include more "white" samples. */
    non_white_bins = (uint32_t)(0.94 * (roiInfo->hist[roi_index].roi_pixels -
      roiInfo->hist[roi_index].bin[255] - roiInfo->hist[roi_index].bin[254]));

    CDBG_AEC("%s: non_white_bins = %d bin[255] = %d bin[254] = %d\n",
      __func__, non_white_bins, roiInfo->hist[roi_index].bin[255],
      roiInfo->hist[roi_index].bin[254]);

    white_area_percentile = 0;

    for (i = 0; i < 256; i++) {
      samples += roiInfo->hist[roi_index].bin[i];
      sum     += roiInfo->hist[roi_index].bin[i] * i;
      if ((samples >= non_white_bins) && (white_area_percentile == 0))
        white_area_percentile = i;
    }
    CDBG_AEC("%s: sum = %d samples = %d white_area_percentile = %d\n",
      __func__, sum, samples, white_area_percentile);

    sum = sum / samples;
    /* Luma average above x percentile to determine skin lightness */
    for (i = white_area_percentile; i < 254; i++) {
      white_sum     = white_sum     + roiInfo->hist[roi_index].bin[i] * i;
      white_samples = white_samples + roiInfo->hist[roi_index].bin[i];
    }
    if (white_samples > 1)
      white_sum = white_sum / white_samples;
    else
      white_sum = sum;

    face_skin_ratio[roi_index] = (float) sum / white_sum;
    CDBG_AEC("%s:sum=%d, w_sum=%d, smpls=%d, w_smpls=%d, ratio=%f", __func__,
      sum, white_sum, samples, white_samples, face_skin_ratio[roi_index]);

    roi_size[roi_index] = (uint32_t)(roiInfo->roi[roi_index].dx * roiInfo->roi[roi_index].dy);
  }
  /* calculate final luma value from all roi lumas, largest rois
   * have more weight; sort by roi sizes */
  for (roi_index = 0; roi_index < roiInfo->num_roi; roi_index++) {
    float    dark_skin_ratio, light_skin_ratio;
    float    face_ratio_interpolation, face_luma_target;
    uint32_t dark_skin_luma_target, light_skin_luma_target;

    /* To start just average all the faces: Modify each face's luma according
     * to skin tone lightness/darkness as determined by the white/skin ratio
     * White is about 0.70, Dark  is about 0.45, Dark skin tone luma target
     * about 30 corresponding to face skin ratio of 0.45 White luma skin tone
     * target about 80 corresponfing to face skin ratio of 0.70 */
    dark_skin_ratio        = 0.45;
    light_skin_ratio       = 0.70;
    dark_skin_luma_target  = 36; /* 30 looked too dark */
    light_skin_luma_target = 80; /* 100 looked too bright */

    /* boundary check */
    if (face_skin_ratio[roi_index] < dark_skin_ratio)
      face_skin_ratio[roi_index] = dark_skin_ratio;
    else if (face_skin_ratio[roi_index] > light_skin_ratio)
      face_skin_ratio[roi_index] = light_skin_ratio;

    face_ratio_interpolation = (face_skin_ratio[roi_index] -
      dark_skin_ratio) / (light_skin_ratio - dark_skin_ratio);

    face_luma_target = (float)dark_skin_luma_target  *
      (1 - face_ratio_interpolation) + (float)light_skin_luma_target *
      face_ratio_interpolation;
    face_luma_target = aec->luma.target / face_luma_target;

    global_roi_luma = (uint32_t)(roi_luma_array[roi_index] *
      face_luma_target) + global_roi_luma;
  }
  return global_roi_luma;
} /* aec_calc_touch_RIO_luma */

/*==========================================================================
 * FUNCTION    - aec_cal_luma_adj_with_mtr_area -
 *
 * DESCRIPTION:
 *=========================================================================*/
static int aec_cal_luma_adj_with_mtr_area(stats_proc_t *sproc, aec_t *aec)
{
  int      region_dx = 0, region_dy = 0;
  uint32_t WeightSum=0, global_mtr_luma = 0, i, j;;
  int  mtr_index;
  uint32_t *sumLumaArray = (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;
  stats_proc_mtr_area_t *mtrArea  = &(aec->aec_mtr_area);
  int LumaSum[mtrArea->num_area];

  if (mtrArea->num_area <= 0) {
    CDBG_AEC("%s: No AEC metering Area adjustment required\n", __func__);
    return 0;
  }
  for (mtr_index = 0; mtr_index < mtrArea->num_area; mtr_index++) {
    if (mtrArea->weight[mtr_index]< 0 &&mtrArea->weight[mtr_index] > 1000) {
      CDBG_AEC("Metering Area weight is out of range ie <0 or >1000");
      return 0;
    }
  }
  /* 16x16 grids */
  switch (sproc->input.mctl_info.numRegions) {
    case 4:
      region_dx = (sproc->input.mctl_info.preview_width) >> 1;
      region_dy = (sproc->input.mctl_info.preview_height) >> 1;
      break;
    case 16:
      region_dx = (sproc->input.mctl_info.preview_width) >> 2;
      region_dy = (sproc->input.mctl_info.preview_height) >> 2;
      break;
    case 64:
      region_dx = (sproc->input.mctl_info.preview_width) >> 3;
      region_dy = (sproc->input.mctl_info.preview_height) >> 3;
      break;
    case 256:
    default:
      region_dx = (sproc->input.mctl_info.preview_width) >> 4;
      region_dy = (sproc->input.mctl_info.preview_height) >> 4;
      break;
  }
  for (mtr_index = 0; mtr_index < mtrArea->num_area; mtr_index++) {
    uint32_t y_region_start, y_region_dy, x_region_start, x_region_dx;
    uint32_t region_count = 0, lumaSum = 0;
    /* get region locations for the roi
     * Rotation and zoom not considered */
    y_region_start =
      (uint32_t)((float)mtrArea->mtr_area[mtr_index].y  / region_dy + 0.5);
    y_region_dy    =
      (uint32_t)((float)mtrArea->mtr_area[mtr_index].dy / region_dy + 0.5);
    x_region_start =
      (uint32_t)((float)mtrArea->mtr_area[mtr_index].x  / region_dx + 0.5);
    x_region_dx    =
      (uint32_t)((float)mtrArea->mtr_area[mtr_index].dx / region_dx + 0.5);

    for (i = y_region_start; i < (y_region_start + y_region_dy); i++) {  /* rows */
      for (j = x_region_start; j < (x_region_start + x_region_dx); j++) {  /*cols*/
        lumaSum += sumLumaArray[i * 16 + j];
        region_count++;
      }
    }
    lumaSum = lumaSum / region_count / sproc->input.mctl_info.pixelsPerRegion;
    LumaSum[mtr_index] = lumaSum;
  }
  for (mtr_index = 0; mtr_index < mtrArea->num_area; mtr_index++) {
    global_mtr_luma += (LumaSum[mtr_index] * mtrArea->weight[mtr_index]);
    WeightSum += mtrArea->weight[mtr_index];
  }
  global_mtr_luma = global_mtr_luma / WeightSum;
  CDBG_HIGH("%s: Luma Adj metering area %d",__func__,  global_mtr_luma);
  return global_mtr_luma;
}  /* aec_cal_luma_adj_with_mtr_area */
/*==========================================================================
 * FUNCTION    - aec_cal_luma_adj_with_FD_ROI -
 *
 * DESCRIPTION:
 *=========================================================================*/
static int aec_cal_luma_adj_with_FD_ROI(stats_proc_t *sproc, aec_t *aec)
{
  int      region_dx = 0, region_dy = 0;
  uint32_t roi_luma_array[MAX_ROI], global_roi_luma = 0, roi_index, i, j;
  uint32_t *sumLumaArray = (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;

  stats_proc_roi_info_t *roiInfo  = &(sproc->share.fd_roi);

  if (roiInfo->num_roi <= 0) {
    aec->fd_adjusted_luma = 0;
    CDBG_AEC("%s: No fd_luma_agjust\n", __func__);
    return -1;
  } else if (roiInfo->roi_updated == FALSE) {
    CDBG_AEC("%s: Fd_luma_agjust is the same\n", __func__);
    return 0;
  }
  /* 16x16 grids */
  switch (sproc->input.mctl_info.numRegions) {
    case 4:
      region_dx = (roiInfo->frm_width) >> 1;
      region_dy = (roiInfo->frm_height) >> 1;
      break;
    case 16:
      region_dx = (roiInfo->frm_width) >> 2;
      region_dy = (roiInfo->frm_height) >> 2;
      break;
    case 64:
      region_dx = (roiInfo->frm_width) >> 3;
      region_dy = (roiInfo->frm_height) >> 3;
      break;
    case 256:
    default:
      region_dx = (roiInfo->frm_width) >> 4;
      region_dy = (roiInfo->frm_height) >> 4;
      break;
  }

  for (roi_index = 0; roi_index < roiInfo->num_roi; roi_index++) {
    uint32_t y_region_start, y_region_dy, x_region_start, x_region_dx;
    uint32_t region_count = 0, lumaSum = 0;
    /* get region locations for the roi
     * Rotation and zoom not considered */
    y_region_start = (uint32_t)((float)roiInfo->roi[roi_index].y  / region_dy + 0.5);
    y_region_dy    = (uint32_t)((float)roiInfo->roi[roi_index].dy / region_dy + 0.5);
    x_region_start = (uint32_t)((float)roiInfo->roi[roi_index].x  / region_dx + 0.5);
    x_region_dx    = (uint32_t)((float)roiInfo->roi[roi_index].dx / region_dx + 0.5);

    for (i = y_region_start; i < (y_region_start + y_region_dy); i++) { /* rows */
      for (j = x_region_start; j < (x_region_start + x_region_dx); j++) {/*cols*/
        lumaSum += sumLumaArray[i * 16 + j];
        region_count++;
      }
    }
    lumaSum = lumaSum / region_count / sproc->input.mctl_info.pixelsPerRegion;
    roi_luma_array[roi_index] = lumaSum;
    global_roi_luma += lumaSum;
  }

  /* If ROI's are not faces, then no need for skin-tone adjustment */
  if (roiInfo->type != ROI_TYPE_GENERAL)
    global_roi_luma = aec_calc_touch_RIO_luma(sproc, aec, roi_luma_array);

  aec->fd_adjusted_luma = global_roi_luma / roiInfo->num_roi;
  CDBG_AEC("%s: fd_luma_agjust %d\n", __func__, aec->fd_adjusted_luma);
  return 0;
} /* aec_cal_luma_adj_with_FD_ROI */

/*==========================================================================
 * FUNCTION    - aec_calculate_current_luma -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_calculate_current_luma(stats_proc_t *sproc, aec_t *aec)
{
  int spotLuma, metered_luma;
  metered_luma = aec_calculate_current_metered_luma(sproc, aec);

  if (aec->roi.enable) {
    spotLuma = aec_calculate_current_ROI_luma(sproc, aec);
    return(int)(((float)(spotLuma)) * sproc->input.chromatix->AEC_touch.
      touch_roi_weight + ((float)(metered_luma))*(1.0 - sproc->input.chromatix->
      AEC_touch.touch_roi_weight));
  } else
    return metered_luma;
} /* aec_calculate_current_luma */

/*===========================================================================
 * FUNCTION    - aec_cal_current_luma_for_af -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int aec_calculate_current_luma_for_af(stats_proc_t *sproc, aec_t *aec)
{
  int  luma_cur, i, j, af_lumaSum = 0;
  int  af_num_divisions_at_each_side, af_num_focus_regions;

  uint32_t *sumLumaArray = (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;

  if (sproc->input.mctl_info.numRegions == 256) {
    af_num_divisions_at_each_side = 16;
    af_num_focus_regions          = 64;
    for (i = 4; i < 12; i++)
      for (j = 4; j < 12; j++)
        af_lumaSum += sumLumaArray[(i * af_num_divisions_at_each_side + j)];
  } else {
    af_num_divisions_at_each_side = 8;
    af_num_focus_regions          = 16;
    for (i = 2; i < 6; i++)
      for (j = 2; j < 6; j++)
        af_lumaSum += sumLumaArray[(i * af_num_divisions_at_each_side + j)];
  }
  luma_cur = af_lumaSum / sproc->input.mctl_info.pixelsPerRegion /
    af_num_focus_regions;

  CDBG_HIGH("%s: luma_cur %d, pixels_per_rgn %d, numRegions %d, af lumaSum %d"
    " af_num_focus_regions %d", __func__, luma_cur,
    sproc->input.mctl_info.pixelsPerRegion,
    sproc->input.mctl_info.numRegions, af_lumaSum, af_num_focus_regions);
  return luma_cur;
} /* aec_calculate_current_luma_for_af */

/*==========================================================================
 * FUNCTION    - aec_apply_multi_luma_target -
 *
 * DESCRIPTION:  Apply multi luma target to correct current_vfe_luma
 *=========================================================================*/
static int aec_apply_multi_luma_target(stats_proc_t *sproc, aec_t *aec)
{
  int   outdoor_target, indoor_target, offset = 0;
  float ratio = 1.0;
  stats_proc_aec_data_t *aec_ext = &(sproc->share.aec_ext);

  indoor_target  = aec->default_luma_target_compensated;
  outdoor_target = aec->outdoor_luma_target_compensated;
  /* apply outdoor luma target */
  if (aec_ext->exp_index < aec_ext->outdoor_index)
    offset = outdoor_target - indoor_target;
  else if (aec_ext->exp_index < aec_ext->indoor_index) {
    ratio = (float)((float)(aec_ext->exp_index - aec_ext->outdoor_index) /
      (float)(aec_ext->indoor_index - aec_ext->outdoor_index));

    offset = (int32_t)(((1.0 - ratio) * (float)outdoor_target) +
      (ratio * (float) indoor_target));
    offset = offset - indoor_target;
  } else /* apply default luma target */
    offset = 0;

  aec->luma.target = offset + indoor_target;
  CDBG_AEC("%s: offset = %d, indoor_target %d, outdoor_target %d, target=%d\n",
    __func__, offset, indoor_target, outdoor_target, aec->luma.target);
  return offset;
} /* aec_apply_multi_luma_target */

/*==========================================================================
 * FUNCTION    - aec_compensate_motion -
 *
 * DESCRIPTION:  Determines trade-off between gain and exposure for a given
 *               motion between adjacent frames.
 *=========================================================================*/
static void aec_compensate_motion(stats_proc_t *sproc, aec_t *aec)
{
  uint32_t line_countmt, preview_linesPerFrame, preview_fps;
  float motion_weight = 0.10, preview_exposuretime, cur_real_gain, mtt;
  float gainmt = 1.0, snap_gainmt = 1.0, snap_mtt, max_aec_table_gain;

  float mtn_thld =
    sproc->input.chromatix->aec_motion_iso_preview.motion_iso_threshold;
  float snap_mtn_thld =
    sproc->input.chromatix->aec_motion_iso_snapshot.motion_iso_threshold;

  if (aec->mtn.apply == FALSE) {
    /* Raise threshold when there is no motion and motion is suddenly
     * detected, this is too remove borderline triggering of this feature*/
    mtn_thld      = mtn_thld      * 1.2;
    snap_mtn_thld = snap_mtn_thld * 1.2;
  }
  if (sproc->share.aec_ext.exp_index == (int)
    (sproc->share.aec_ext.exp_tbl_val - 1)) {
    /* very low light, high gains and noise, increases threshold to prevent
     * false triggering may have not reached luma target */
    mtn_thld      =
      sproc->input.chromatix->aec_motion_iso_preview.motion_iso_threshold * 1.5;
    snap_mtn_thld =
      sproc->input.chromatix->aec_motion_iso_snapshot.motion_iso_threshold * 1.5;
  }
  /* slow down reaction motion compensation metric */
  aec->mtn.tmp_val = (1 - motion_weight) *
    aec->mtn.tmp_val + motion_weight * aec->motion;

  if (aec->mtn.tmp_val < snap_mtn_thld) /* no adjustment made for snapshot */
    aec->mtn.apply = FALSE;
  else /* adjustment will be made for snapshot */
    aec->mtn.apply = TRUE;

  /* for digital gain support only. */
  max_aec_table_gain = (float)aec->exp_tbl_ptr[
    sproc->share.aec_ext.exp_tbl_val - 1].gain / 256.0;
  /* reduce effect due to noisy environment. Prevent false triggering. */
  if (sproc->share.aec_ext.cur_real_gain == max_aec_table_gain) {
    mtn_thld      = mtn_thld * 2;
    snap_mtn_thld = snap_mtn_thld * 2;
  }
  /* trade-off min 500 max 5000, Need to add LUT, characterize motion metric */
  mtt      = (aec->mtn.tmp_val / mtn_thld) *
    sproc->input.chromatix->aec_motion_iso_preview.motion_iso_aggressiveness;
  snap_mtt = (aec->mtn.tmp_val / snap_mtn_thld) *
    sproc->input.chromatix->aec_motion_iso_snapshot.motion_iso_aggressiveness;

  if (mtt > 4.0) /* max cap 4x tradeoff. */
    mtt = 4.0;
  else if (mtt <1.0)
    mtt = 1.0; /* 1 means not applied. */

  if (snap_mtt > 4.0) /* max cap 4x tradeoff. */
    snap_mtt = 4.0;
  else if (snap_mtt <1.0)
    snap_mtt = 1.0; /* 1 means not applied. */

  /* slowdown for preview to reduce oscillations. */
  aec->mtn.val = (0.8 * aec->mtn.val) + (0.2 * mtt);
  /* speed up response for snapshot. */
  aec->mtn.snap_val = (0.1 * aec->mtn.snap_val) + (0.9 * snap_mtt);

  preview_linesPerFrame = sproc->input.sensor_info.preview_linesPerFrame;
  preview_fps           = sproc->input.sensor_info.preview_fps;

  preview_exposuretime = (float)preview_fps / AEC_Q8;
  preview_exposuretime = (float)sproc->share.aec_ext.cur_line_cnt /
    (preview_exposuretime * preview_linesPerFrame);

  if (preview_exposuretime < MAX_PREVIEW_EXP_TIME)
    CDBG_AEC("%s: preview_exposuretime is less than 0.033 \n", __func__);

  cur_real_gain = sproc->share.aec_ext.cur_real_gain;
  gainmt        = cur_real_gain * aec->mtn.val;
  snap_gainmt   = cur_real_gain * aec->mtn.snap_val;

  if (gainmt > sproc->input.chromatix->aec_motion_iso_preview.motion_iso_max_gain)
    gainmt = sproc->input.chromatix->aec_motion_iso_preview.motion_iso_max_gain;

  if (snap_gainmt > sproc->input.chromatix->
    aec_motion_iso_snapshot.motion_iso_max_gain)
    snap_gainmt = sproc->input.chromatix->aec_motion_iso_snapshot.motion_iso_max_gain;

  /* Potential brightness changes with fast changes in gain may be cause
   * by async application of digital gain, sensor gain and line count
   * cap gain changes during preview when aec is stable */
  if (sproc->share.aec_ext.exp_index == sproc->share.prev_exp_index)
    CDBG_AEC("%s: check gain change", __func__);

  if (gainmt > 0.0) {
    line_countmt = (sproc->share.aec_ext.cur_line_cnt *
      (cur_real_gain / gainmt));
    gainmt       = gainmt *  (sproc->share.aec_ext.cur_line_cnt *
      (cur_real_gain / gainmt)) / line_countmt;
  } else
    line_countmt = (uint32_t)sproc->share.aec_ext.cur_line_cnt;

  if (snap_gainmt > 0.0) {
    sproc->share.aec_ext.snap.line_count = (uint32_t)(sproc->share.aec_ext.
      cur_line_cnt * (cur_real_gain / snap_gainmt));

    snap_gainmt = snap_gainmt * (sproc->share.aec_ext.cur_line_cnt *
      (cur_real_gain / snap_gainmt)) / sproc->share.aec_ext.snap.line_count;
  }

  /* fine gain adjustment needed due to line_count having discrete values */
  sproc->share.aec_ext.cur_line_cnt = line_countmt;

  /* update global current real gain used */
  sproc->share.aec_ext.cur_real_gain = gainmt;
  aec->mtn.iso_snap_real_gain = snap_gainmt;

  CDBG_AEC("%s: output snap_line_cnt = %d mt = %f, snap_mt = %f"
    , __func__,  sproc->share.aec_ext.snap.line_count, aec->mtn.val,
    aec->mtn.snap_val);
  CDBG_AEC("%s: temporal_motion = %f, motion = %f", __func__,
    aec->mtn.tmp_val, aec->motion);
} /* aec_compensate_motion */

/*==========================================================================
 * FUNCTION    - aec_enhance_banding -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_enhance_banding(stats_proc_t *sproc, aec_t *aec)
{
  float preview_exposuretime;
  int preview_linesPerFrame = sproc->input.sensor_info.preview_linesPerFrame;
  int preview_fps = sproc->input.sensor_info.preview_fps;
  int gain;

  preview_exposuretime = sproc->share.aec_ext.cur_line_cnt/
    ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);

  CDBG_AEC("%s: afd_status %d, preview_exposure_time=%f", __func__,
    sproc->share.afd_status, preview_exposuretime);

  switch (sproc->share.afd_status) {
    case AFD_REGULAR_EXPOSURE_TABLE:
      /* force 50 and 60hz bands, use smallest exposure time possible but
         larger than 1 band gaps. */
      if(sproc->share.afd_exec_once){
        CDBG_AEC("%s: Dont enhance bands as afd is in monitor state", __func__);
        return;
      }
      if (preview_exposuretime > (1 / 90.0)) {
        float new_rem, rem60, rem50, temp_preview_exposuretime, preview_linesPerS;

        CDBG_AEC("%s: current real gain: %f, line_count: %d", __func__,
          sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt);

        rem60 = (float)((uint32_t)(preview_exposuretime * 120));
        temp_preview_exposuretime = (float)(rem60 / 120.0) - 0.00208;

        /* Make exp_time NOT fit to band gap, 25% off from gap */
        rem50 = temp_preview_exposuretime * 100;
        new_rem = (float)((uint32_t)(rem50));
        if (rem50 < 1)  /* Cannot Cancel bands */
          return;

        if (rem50 - new_rem < 0.2 || rem50 - new_rem > 0.8)
          temp_preview_exposuretime = ((float)(new_rem / 100)) - (0.002);

        sproc->share.aec_ext.cur_real_gain = sproc->share.aec_ext.cur_real_gain *
          preview_exposuretime / temp_preview_exposuretime;

        preview_linesPerS = preview_linesPerFrame *  preview_fps;
        sproc->share.aec_ext.cur_line_cnt = (uint32_t) (((temp_preview_exposuretime *
          preview_linesPerS) / AEC_Q8) + 0.5);
        preview_exposuretime = sproc->share.aec_ext.cur_line_cnt/
          ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);
        CDBG_AEC("%s: Regular new real gain: %f, line_count: %d new preview_exp_time %f",
          __func__,sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt,
          preview_exposuretime);
      }
      break;

    case AFD_60HZ_EXPOSURE_TABLE:
      /* force 50 and cancel 60hz bands, use smallest exposure time possible
         but larger than 1 band gaps only need to change for 60Hz gaps of
         5, 10, 15, etc.. to gap-1 */
      if (preview_exposuretime > (1 / 120.0)) {
        float rem60, temp_preview_exposuretime, preview_linesPerS;
        CDBG_AEC("%s: current real gain: %f, line_count: %d", __func__,
          sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt);
        rem60 = (uint32_t) (preview_exposuretime * 120);

        /* 1/120 and 1/100 overlap when n=multiple of 5. */
        if ((uint32_t)rem60 % 5 == 0)
          rem60 = rem60 + 1;

        /* Make exp_time fit to band gap */
        temp_preview_exposuretime = rem60 * (1 / 120.0);
        sproc->share.aec_ext.cur_real_gain = sproc->share.aec_ext.cur_real_gain *
          preview_exposuretime / temp_preview_exposuretime;
        preview_exposuretime = temp_preview_exposuretime;
        preview_linesPerS = preview_linesPerFrame *  preview_fps;
        sproc->share.aec_ext.cur_line_cnt = (uint32_t) ((temp_preview_exposuretime *
          preview_linesPerS / AEC_Q8) + 0.5);
        preview_exposuretime = sproc->share.aec_ext.cur_line_cnt/
          ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);
        CDBG_AEC("%s: 60 new real gain: %f, line_count: %d new preview_exp_time %f",
          __func__, sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt,
          preview_exposuretime);
      }
      break;

    case AFD_50HZ_EXPOSURE_TABLE:
      /* Cancel 50 bands, use smallest exposure time possible but larger than 1
         band gaps.  force 50 and cancel 60hz bands, use smallest exposure time
         possible but larger than 1 band gaps only need to change for 60Hz gaps
         of 5, 10, 15, etc.. to gap-1 */
      if (preview_exposuretime > (1 / 100.0)) {
        float rem50, temp_preview_exposuretime, preview_linesPerS;
        CDBG_AEC("%s: current real gain: %f, line_count: %d", __func__,
          sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt);
        rem50 = (uint32_t) (preview_exposuretime * 100);

        /* Make exp_time fit to band gap */
        temp_preview_exposuretime = rem50 * (1 / 100.0);
        sproc->share.aec_ext.cur_real_gain = sproc->share.aec_ext.cur_real_gain *
          preview_exposuretime / temp_preview_exposuretime;
        preview_exposuretime = temp_preview_exposuretime;
        preview_linesPerS = preview_linesPerFrame *  preview_fps;
        sproc->share.aec_ext.cur_line_cnt = (uint32_t)(temp_preview_exposuretime *
          preview_linesPerS / AEC_Q8 + 0.5);
        preview_exposuretime = sproc->share.aec_ext.cur_line_cnt/
                      ((float)preview_fps / AEC_Q8 * preview_linesPerFrame);
        CDBG_ERROR("%s: 50 new real gain: %f, line_count: %d, new preview_exp_time %f", __func__,
          sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt, preview_exposuretime);
      }
      break;
    default:
      break;
  }
  return;
} /* aec_enhance_banding */

/*==========================================================================
 * FUNCTION    - aec_fast_convergence -
 *
 * DESCRIPTION: Adjust exposure on the camera sensor.  Implement a
 *             generic algorithm that can work with any sensor
 *=========================================================================*/
static void aec_fast_convergence(stats_proc_t *sproc, aec_t *aec,
  uint32_t current_vfe_luma)
{
  uint32_t weighted_luma = 128;
  int      luma_delta = 0, tol_lo, tol_hi, exp_index_adjust = 0;
  float    index_adjust;
  int32_t adj_luma_tolerance;

  if (!aec->sensor_update_ok)
    aec->next_update_frame_cnt++;

  if (aec->frame_in_current_fps < aec->next_update_frame_cnt) {
    aec->update_hw = FALSE;
    return;
  }
  weighted_luma = current_vfe_luma;
  aec->next_update_frame_cnt = aec->frame_in_current_fps +
    aec->frame_skip +1;
  CDBG_AEC("%s: curt_vfe_luma %d, luma target %d", __func__,
    current_vfe_luma, aec->luma.comp_target);

  luma_delta = (int) weighted_luma - (int) aec->luma.comp_target;

  if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_TORCH) {
    sproc->share.luma_settled_cnt = LUMA_SETTLED_BEFORE_AF_CNT;
    adj_luma_tolerance = 4 * aec->luma.tolerance;
    tol_lo = (-1)* aec->luma.comp_target;
    tol_hi = aec->luma.comp_target;
  } else {
    adj_luma_tolerance = aec->luma.tolerance;
    tol_lo = (-1) * adj_luma_tolerance;
    tol_hi = adj_luma_tolerance;
  }

  /*Luma Tolerance verification, if true, we adjust exposure */
  if ((luma_delta > tol_hi) || (luma_delta < tol_lo)) {
    sproc->share.luma_settled_cnt = 0;

    if (weighted_luma == 0)
      weighted_luma = 1;

    index_adjust = (float) ((log10((double) aec->luma.comp_target /
      weighted_luma)) * sproc->input.chromatix->exposure_index_adj_step);

    if (index_adjust < 0) {
      exp_index_adjust = (int) ((aec->fast_conv_speed * index_adjust) / AEC_Q8);

      if (exp_index_adjust == 0)
        exp_index_adjust = exp_index_adjust - 1;
    } else {
      exp_index_adjust = (int) ((aec->fast_conv_speed * index_adjust) / AEC_Q8);
      /* halve speed to go to darker scene.. */
      if (exp_index_adjust == 0)
        exp_index_adjust = exp_index_adjust + 1;
    }
    sproc->share.aec_ext.exp_index = sproc->share.aec_ext.exp_index +
      exp_index_adjust;

    if (sproc->share.aec_ext.exp_index < 0)
      sproc->share.aec_ext.exp_index = 0;

  } else {
    index_adjust = 0;
    sproc->share.luma_settled_cnt++;
  }

  /* Subract 1 from num exposure values to get max exposure index */
  if (sproc->share.aec_ext.exp_index > (int)
    (sproc->share.aec_ext.exp_tbl_val - 1)) {
    sproc->share.aec_ext.exp_index =
      (int) (sproc->share.aec_ext.exp_tbl_val - 1);
    CDBG_AEC("%s: greater than exposure table  %d", __func__,
      sproc->share.aec_ext.exp_tbl_val - 1);
  }
} /* aec_fast_convergence */

/*==========================================================================
 * FUNCTION    - aec_convergence_test -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_convergence_test(stats_proc_t *sproc, aec_t *aec)
{
  if ((aec->frame_in_current_fps % 3) != 0)
    sproc->share.aec_ext.exp_index = sproc->share.prev_exp_index;
  else if (aec->exp_increase)
    sproc->share.aec_ext.exp_index += 2;
  else
    sproc->share.aec_ext.exp_index -= 2;

  if ((sproc->share.aec_ext.exp_index - 2) < 0) {
    aec->exp_increase              = TRUE;
    sproc->share.aec_ext.exp_index = 1;
  }

  if (sproc->share.aec_ext.exp_index >
    (int)(sproc->share.aec_ext.exp_tbl_val - 1)) {
    aec->exp_increase              = FALSE;
    sproc->share.aec_ext.exp_index = sproc->share.aec_ext.exp_tbl_val - 1;
  }
} /* aec_convergence_test */

/*==========================================================================
 * FUNCTION    - aec_adjust_exposure -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void aec_adjust_exposure(stats_proc_t *sproc, aec_t *aec)
{
  int  luma_offset;
  uint32_t comp_ratio, temp_vfe_luma, digital_gained_luma, asd_luma_offset;
  uint32_t current_vfe_luma = sproc->share.aec_ext.cur_luma;
  chromatix_parms_type *cptr = sproc->input.chromatix;

  /* digital_gain_adj default value should be 1.0 */
  digital_gained_luma = current_vfe_luma * sproc->input.isp_info.digital_gain_adj;
  if (digital_gained_luma > 255) digital_gained_luma = 255;

  current_vfe_luma = sproc->share.aec_ext.cur_luma = (uint8_t)digital_gained_luma;

  aec_apply_multi_luma_target(sproc, aec);
  aec->asd_luma_offset = 0;

  if (sproc->share.asd_ext.backlight_detected)
    aec->asd_luma_offset += sproc->share.backlight_luma_target_offset;
  if (sproc->share.asd_ext.snow_or_cloudy_scene_detected)
    aec->asd_luma_offset += sproc->share.snow_or_cloudy_luma_target_offset;

  if (sproc->share.asd_ext.portrait_severity < 20)
    asd_luma_offset = aec->asd_luma_offset;
  else if (sproc->share.asd_ext.portrait_severity > 84)
    asd_luma_offset = 0;
  else
    asd_luma_offset = aec->asd_luma_offset *
    (84 - sproc->share.asd_ext.portrait_severity) / 64;

  /* CAP asd luma offset */
  if (sproc->share.asd_ext.backlight_detected &&
    sproc->share.asd_ext.snow_or_cloudy_scene_detected && asd_luma_offset >
    MAX(cptr->snow_scene_detect.extreme_luma_target_offset,
    (aec->luma.comp_target * cptr->backlit_scene_detect.
    backlight_max_la_luma_target_adj - aec->luma.comp_target)))
    asd_luma_offset = MAX(cptr->snow_scene_detect.extreme_luma_target_offset,
      (aec->luma.comp_target * cptr->backlit_scene_detect.
      backlight_max_la_luma_target_adj - aec->luma.comp_target));
  else if (sproc->share.asd_ext.backlight_detected && asd_luma_offset >
    aec->luma.comp_target * cptr->backlit_scene_detect.
    backlight_max_la_luma_target_adj - aec->luma.comp_target)
    asd_luma_offset = aec->luma.comp_target * cptr->backlit_scene_detect.
      backlight_max_la_luma_target_adj - aec->luma.comp_target;
  else if (sproc->share.asd_ext.snow_or_cloudy_scene_detected &&
    asd_luma_offset > cptr->snow_scene_detect.extreme_luma_target_offset)
    asd_luma_offset = cptr->snow_scene_detect.extreme_luma_target_offset;

  aec->luma.comp_target = aec->luma.target + asd_luma_offset;
  if (aec->luma.comp_target < aec->luma.target)
    aec->luma.comp_target = 255;

  sproc->share.aec_ext.comp_luma = current_vfe_luma;
  CDBG_AEC("%s:comp_target %d, sd_offset %d, compensated_luma %d\n",
    __func__,  aec->luma.comp_target, aec->asd_luma_offset,
    sproc->share.aec_ext.comp_luma);

  if (!aec->eztune.test_enable) {
    if (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER)
      aec_slow_convergence(sproc, aec, current_vfe_luma);
    else
      aec_fast_convergence(sproc, aec, current_vfe_luma);
  } else
    aec_convergence_test(sproc, aec);

  if (aec->update_hw) { /* since we always support digital gain, no need
                           * to check it, Q8 real gain is in exposure table */
    sproc->share.aec_ext.sof_update = aec->update_hw;
    sproc->share.aec_ext.cur_real_gain = (float) aec->
      exp_tbl_ptr[sproc->share.aec_ext.exp_index].gain / 256.0;

    sproc->share.aec_ext.cur_line_cnt =
      aec->exp_tbl_ptr[sproc->share.aec_ext.exp_index].line_count;
    CDBG_AEC("%s: exp index %d, line cnt %u, motion_iso %d, preview_iso %d, "
      "interested_region %d", __func__, sproc->share.aec_ext.exp_index, sproc->
      share.aec_ext.cur_line_cnt, aec->mtn.status, sproc->input.chromatix->
      aec_motion_iso_preview.motion_iso_enable, aec->sub_roi.enable);

    /* Comp motion if motion iso is on, or if touch aec is not enabled */
    if (aec->mtn.status == MOTION_ISO_ON && sproc->input.chromatix->
      aec_motion_iso_preview.motion_iso_enable && !(aec->roi.enable))
      aec_compensate_motion(sproc, aec);
    else if (sproc->input.mctl_info.opt_state != STATS_PROC_STATE_CAMCORDER &&
      (sproc->input.mctl_info.opt_mode == STATS_PROC_MODE_2D_ZSL ||
      sproc->input.mctl_info.opt_mode == STATS_PROC_MODE_3D_ZSL) &&
      sproc->share.aec_ext.iso >= CAMERA_ISO_100 &&
      sproc->share.aec_ext.iso < CAMERA_ISO_MAX) {
      uint32_t  iso_multiplier = 1;
      camera_iso_mode_type cur_iso;
      float iso_real_gain = sproc->input.chromatix->ISO100_gain;

      CDBG_AEC("%s: iso_real_gain %f",__func__, iso_real_gain);
      if (iso_real_gain != 0.0) {
        cur_iso = sproc->share.aec_ext.iso;
        iso_multiplier = cur_iso - CAMERA_ISO_100;
        iso_multiplier = (1 << iso_multiplier);
        iso_real_gain = iso_real_gain * iso_multiplier;
        sproc->share.aec_ext.cur_line_cnt = (float)sproc->share.aec_ext.
          cur_line_cnt * sproc->share.aec_ext.cur_real_gain / iso_real_gain;
        sproc->share.aec_ext.cur_real_gain = iso_real_gain; /* real gains */
      }
    }
    /* anti-banding support for preview, snapshot anti-banding support
     * is taken care in ISO function */
    if(!sproc->share.afd_monitor)
      aec_preview_antibanding(sproc, aec);
    else
      aec_enhance_banding(sproc, aec);
    sproc->share.aec_ext.prev_sensitivity = aec_calc_sensitivity(sproc,
      sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt);
    CDBG_AEC("%s: cur_real_gain %f, preview line_count %d", __func__,
      sproc->share.aec_ext.cur_real_gain, sproc->share.aec_ext.cur_line_cnt);
  }
} /* aec_adjust_exposure */

/*==========================================================================
 * FUNCTION    - aec_calculate_motion -
 *
 * DESCRIPTION:
 *=========================================================================*/
static float aec_calculate_motion(stats_proc_t *sproc, aec_t *aec)
{
  int      j, temp;
  uint32_t i, region_diff_sum = 0, *sumLumaArray;
  float    motion = 0.0, med, mtn_thld, motion_array_sort[MOTION_ARRAY_SIZE];

  sumLumaArray = (uint32_t *)sproc->input.mctl_info.vfe_stats_out->vfe_stats_struct.aec_op.SY;
  mtn_thld = sproc->input.chromatix->aec_motion_iso_preview.motion_iso_threshold;

  if (sproc->input.mctl_info.numRegions > 15) {
    for (i = 0; i < sproc->input.mctl_info.numRegions; i++) {
      if (sumLumaArray[i] > aec->luma.previous_sumArray[i])
        region_diff_sum = region_diff_sum +
          (sumLumaArray[i] - aec->luma.previous_sumArray[i]);
      else
        region_diff_sum = region_diff_sum +
        (aec->luma.previous_sumArray[i] - sumLumaArray[i]);
    }
    /* compare with max number of regions */
    if (sproc->input.mctl_info.numRegions <= (int)(sizeof(aec->luma.
      previous_sumArray) / sizeof(aec->luma.previous_sumArray[0])))
      memcpy(aec->luma.previous_sumArray, sumLumaArray,
        sproc->input.mctl_info.numRegions * sizeof(uint32_t));

    motion = region_diff_sum / ((float) sproc->input.mctl_info.numRegions);
    motion = motion / sproc->input.mctl_info.pixelsPerRegion;

    /* if skipped stats motion frame will be > 1 */
    motion = motion / aec->mtn.frame;
    aec->mtn.frame = 1;
  } else { /* no stats available */
    /* Increase stats frame skip counter */
    aec->mtn.frame++;
    /* reuse last motion estimate value */
    motion = aec->mtn.array[0] / 100;
  }

  CDBG_HIGH("%s: motion_frame %d motion %f", __func__, aec->mtn.frame, motion);
  for (i = MOTION_ARRAY_SIZE - 1; i > 0; i--)
    aec->mtn.array[i] = aec->mtn.array[i - 1];

  aec->mtn.array[0] = motion * 100;
  if (aec->mtn.array[0] > (16 * mtn_thld))
    aec->mtn.array[0] = 16 * mtn_thld;

  memcpy(motion_array_sort, aec->mtn.array, MOTION_ARRAY_SIZE);
  /* sort for med filter */
  for (i = 1; i < MOTION_ARRAY_SIZE; i++) {
    temp = motion_array_sort[i];
    j = i;
    while ((j > 0) && (motion_array_sort[j-1] > temp)) {
      motion_array_sort[j] = motion_array_sort[j-1];
      j -= 1;
    }
    motion_array_sort[j] = temp;
  }

  if (MOTION_ARRAY_SIZE & 1) /* odd num */
    med = motion_array_sort[MOTION_ARRAY_SIZE / 2];
  else /*even number*/
    med = (motion_array_sort[MOTION_ARRAY_SIZE / 2] +
      motion_array_sort[MOTION_ARRAY_SIZE / 2 + 1]) / 2;

  CDBG_HIGH("%s: motion %f, med %f", __func__, motion, med);
  return med;
} /* aec_calculate_motion */

/*==========================================================================
 * FUNCTION    - aec_restore_normal_frame_exp -
 *
 * DESCRIPTION: change aec back to normal frame exposure after pre-fire
 *              strobe flash
 * ASSUMPTION:  the current aec result already saved to strobe_est
 *=========================================================================*/
static void aec_restore_normal_frame_exp(stats_proc_t *sproc, aec_t *aec)
{
  if (sproc->share.aec_ext.cur_line_cnt != aec->flash.strb_est.linecount_off) {
    sproc->share.aec_ext.cur_line_cnt = aec->flash.strb_est.linecount_off;
    sproc->share.aec_ext.cur_real_gain  = aec->flash.strb_est.real_gain_off;
  }
} /* aec_restore_normal_frame_exp */

/*==========================================================================
 * FUNCTION    - aec_process_preview_and_video -
 *
 * DESCRIPTION:
 *=========================================================================*/
int aec_process_preview_and_video(stats_proc_t *sproc, aec_t *aec)
{
  int  previous_luma, new_edgeThreshold, new_noiseThreshold;
  int lowlightWeight, current_edgeThreshold, current_noiseThreshold;
  float preview_exp_time, mtr_area_luma_adj;

  if (aec->af_hjr_frame_skip_count) {
    aec->af_hjr_frame_skip_count --;
    if (aec->af_hjr_frame_skip_count < 0)
      aec->af_hjr_frame_skip_count = 0;
    return 0;
  }

  if (sproc->input.flash_info.led_state != MSM_CAMERA_LED_OFF &&
    sproc->input.flash_info.led_mode != LED_MODE_TORCH) {
    CDBG_AEC("%s: led_frame_skip_cnt %d", __func__,
      sproc->share.aec_ext.led_frame_skip_cnt);

    if (sproc->share.aec_ext.led_frame_skip_cnt++ < LED_FRAME_SKIP_CNT) {
      aec->update_hw     = FALSE;
      return 0;
    }
  } else
    sproc->share.aec_ext.led_frame_skip_cnt = 0;
  if (sproc->share.afd_enable)
    aec->antibanding = sproc->share.afd_atb;

  if (aec->eztune.test_roi)
    aec_test_ROI(sproc, aec);

  aec->update_hw    = TRUE;

  preview_exp_time = (float) (sproc->input.sensor_info.preview_fps / AEC_Q8);
  preview_exp_time =  (sproc->share.aec_ext.cur_line_cnt) /
    (preview_exp_time * sproc->input.sensor_info.preview_linesPerFrame);
  sproc->share.aec_ext.eztune.preview_exp_time = preview_exp_time;

  /* calculate motion */
  if (sproc->input.chromatix->aec_motion_iso_preview.motion_iso_enable &&
    !(aec->roi.enable)) {
    if (aec->eztune.test_motion)
      aec_test_MOTION_ISO(aec, sproc->input.chromatix->aec_motion_iso_preview.
        motion_iso_threshold);
    else
      aec->motion = aec_calculate_motion(sproc, aec);
  }
  previous_luma = sproc->share.aec_ext.cur_luma;
#if (USE_AEC_LED_ROI)
  if (!aec->roi.enable &&
    (sproc->share.aec_ext.aec_flash_settled == AEC_FLASH_SETTLE_WAIT) ) {
    aec_util_calculate_led_low_bias_table(sproc, aec);
  }
#endif
  sproc->share.aec_ext.cur_luma = aec_calculate_current_luma(sproc, aec);
  mtr_area_luma_adj = aec_cal_luma_adj_with_mtr_area(sproc,aec);
  if (mtr_area_luma_adj  > 0)
    sproc->share.aec_ext.cur_luma = 0.3 * previous_luma + 0.7 * mtr_area_luma_adj;
  if (aec_cal_luma_adj_with_FD_ROI(sproc, aec) == 0)
    /* 0.7 is weight adjustment */
    sproc->share.aec_ext.cur_luma = 0.7 * previous_luma + 0.3 * (aec->
      fd_adjusted_luma * 0.7 + sproc->share.aec_ext.cur_luma * (1 - 0.7));

  sproc->share.cur_af_luma = aec_calculate_current_luma_for_af(sproc, aec);

  /* Count frames in Auto Frame Rate */
  aec->frame_in_current_fps++;
  CDBG_AEC("%s:SNAP esti lock %d", __func__, aec->snap_est_lock);

  if (aec->aec_lock &&
      sproc->input.flash_info.led_state == MSM_CAMERA_LED_OFF) {
    CDBG_AEC("%s: AEC is LOCKED", __func__);
  } else if (aec->snap_est_lock) {
    CDBG_AEC("%s: strb_init_state %d, strb_flsh_st %d", __func__,
      aec->flash.strb_int_state, sproc->share.aec_ext.strobe_cfg_st);

    if (sproc->share.aec_ext.strobe_cfg_st == STROBE_PRE_FIRED) {
      /* todo ask ruben if this if STROBE_ON*/
      aec_strobe_flash_store_est(sproc, aec, STROBE_PRE_ON);
      aec->snap_est_lock = FALSE;
      aec_restore_normal_frame_exp(sproc, aec);
      sproc->share.aec_ext.aec_flash_settled = AEC_SETTLED;
    } else if (aec->flash.strb_int_state == STROBE_CHECK_READY) {
      /* check if strobe flash charge ready */
      aec->flash.strb_frame_cnt_wait++;
      if (sproc->input.flash_info.strobe_chrg_ready ||
        aec->flash.strb_frame_cnt_wait > 30) {
        aec->flash.strb_int_state = STROBE_PRE_FLASH;
        /* adjust frame rate to full frame for pre-flash*/
        aec_set_full_frame_exp(sproc, aec);
      }
    } else if (sproc->share.aec_ext.strobe_cfg_st == STROBE_NOT_NEEDED) {
      aec->snap_est_lock = FALSE;
      sproc->share.aec_ext.aec_flash_settled = AEC_SETTLED;
    }
  } else if (aec->hjr_af_lock_cnt > 0) {
    aec->hjr_af_lock_cnt--;
  } else {
    sproc->share.prev_exp_index = sproc->share.aec_ext.exp_index;
    aec_adjust_exposure(sproc, aec);

    if (((sproc->share.aec_ext.cur_luma >= aec->luma.comp_target) &&
      (sproc->share.aec_ext.cur_luma - aec->luma.comp_target) <= aec->luma.
      tolerance) || ((sproc->share.aec_ext.cur_luma <=
      aec->luma.comp_target && aec->luma.comp_target -
      sproc->share.aec_ext.cur_luma <= aec->luma.tolerance))) {
      aec->reach_target_before = 1;
      aec_fast_conv_config(sproc, aec);
    }
  }
  return 0;
} /* aec_process_preview_and_video */
