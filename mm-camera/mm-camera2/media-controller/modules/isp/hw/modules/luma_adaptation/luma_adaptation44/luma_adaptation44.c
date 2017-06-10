/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include "luma_adaptation44.h"
#include "isp_log.h"

#if 0
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define MIN_SAT_PIXELS_PERCENT .1

/*default backlight table for bestshot*/
static int32_t default_backlight_la_tbl[] = {
  0x2d30, 0x1b5d, 0x0e78, 0x0a86, 0x0a90, 0x0d9a, 0x0ca7, 0x05b3, 0xffb8,
  0xfbb7, 0xfab2, 0xfaac, 0xfaa6, 0xfaa0, 0xfb9a, 0xfa95, 0xfb8f, 0xfc8a,
  0xfb86, 0xfc81, 0xfc7d, 0xfc79, 0xfd75, 0xfd72, 0xfd6f, 0xfd6c, 0xfd69,
  0xfe66, 0xfe64, 0xfe62, 0xfe60, 0xfe5e, 0xfe5c, 0xfe5a, 0xff58, 0xfe57,
  0xff55, 0xff54, 0xfe53, 0xff51, 0xff50, 0xff4f, 0xff4e, 0xff4d, 0xff4c,
  0xff4b, 0xff4a, 0x0049, 0xff49, 0xff48, 0xff47, 0x0046, 0xff46, 0xff45,
  0x0044, 0xff44, 0x0043, 0xff43, 0xff42, 0x0041, 0xff41, 0x0040, 0x0040,
  0x0040
};

static const uint16_t curve[256] = {
  64, 64, 64, 64, 64, 64, 64, 64, 64, 63, 63,
  63, 63, 63, 63, 63, 62, 62, 62, 62, 61, 61,
  61, 61, 60, 60, 60, 59, 59, 59, 58, 58, 58,
  57, 57, 56, 56, 56, 55, 55, 54, 54, 53, 53,
  53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 48,
  47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42,
  41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 35,
  35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30,
  29, 29, 28, 27, 27, 26, 26, 25, 25, 25, 24,
  24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19,
  19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 15,
  14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
  11, 11, 10, 10, 10,  9,  9,  9,  9,  8,  8,
  8,  8,  7,  7,  7,  7,  7,  6,  6,  6,  6,
  6,  6,  5,  5,  5,  5,  5,  5,  4,  4,  4,
  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0};

static const uint8_t solarize_la[64] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 61, 57, 54, 51, 47, 44, 42, 39,
    36, 34, 32, 29, 27, 25, 23, 22, 20, 18,
    17, 15, 13, 12, 11, 9,  8,  7,  5,  4,
    3,  2,  1,  0};

static const uint8_t posterize_la[64] = {
    0,  0,  0,  0,  0,  0,  0,  0, 0,  0,
    64, 58, 53, 50, 46, 43, 41, 38, 36, 34,
    66, 63, 60, 57, 55, 53, 51, 49, 47, 46,
    67, 65, 63, 61, 59, 57, 56, 54, 53, 51,
    67, 65, 64, 62, 61, 60, 58, 57, 56, 55,
    67, 66, 65, 64, 62, 61, 60, 59, 58, 57,
    67, 66, 65, 64};


#define LA_SCALE(v, out_min, out_max, in_min, in_max) \
  (((float)((v) - in_min) * (float)(out_max - out_min)) / \
    ((float)(in_max) - (float)(in_min)) + (float)(out_min))

/** la_cfg_set_from_chromatix:
 *    @cfg: local luma adaptation configuration
 *    @chromatix: chromatix header configuration
 *
 * Convert chromatix header tuning for local luma adaptation usage.
 **/
static void la_cfg_set_from_chromatix(la_8k_type *cfg, LA_args_type *chromatix)
{
  cfg->offset =
    LA_SCALE(chromatix->LA_reduction_fine_tune, 0, 16, 0, 100);
  cfg->low_beam =
    LA_SCALE(chromatix->highlight_suppress_fine_tune, 0, 4, 0, 100);
  cfg->high_beam =
    LA_SCALE(chromatix->shadow_boost_fine_tune, 0, 4, 0, 100);

  /* CDF_50_thr maps in inverse to shadow_boost allowance. It should be 100 when
   * when allowance is min(0) and 70, when allowance is max(70) */
  cfg->CDF_50_thr =
    LA_SCALE(chromatix->shadow_boost_allowance, 100, 70, 0, 100);
  cfg->cap_high =
    LA_SCALE(chromatix->shadow_boost_allowance, 0.05, 0.25, 0, 100);
  cfg->histogram_cap =
    LA_SCALE(chromatix->shadow_boost_allowance, 3, 12, 0, 100);
  cfg->cap_low =
    LA_SCALE(chromatix->shadow_boost_allowance, 1.5, 1.5, 0, 100);

  cfg->cap_adjust = 256.0f / chromatix->shadow_range;
}

/** la_get_min_pdf_count:
 *    @num_hist_pixels: number of pixels
 *
 * Return: min of Hist pixels
 **/
static uint64_t la_get_min_pdf_count(uint32_t num_hist_pixels)
{
  return (uint64_t)(MIN_SAT_PIXELS_PERCENT * (float)num_hist_pixels);
}

/** stats_calc_hist_curve:
 *
 * This function process histogram stats from the isp and
 * and calculates the LA curve
 *
 **/
static int stats_calc_hist_curve(isp_ihist_params_t *ihist_stats,
  isp_la_mod_t *la_mod, uint32_t Offset, uint8_t *la_curve)
{
  uint16_t shift, CDF_50_threshold;
  uint64_t size = 0, capped_count, cap_inc, tmp64;
  uint32_t *hist, *threshold, avg_inc;
  /* full_bin: hist==thld, high_bin: hist>thld */
  uint32_t high_bin, full_bin, iter_cnt;
  uint64_t *H;
  uint64_t num_hist_pixels;
  float tmp0, tmp1, tmp, cap_adjust, cap_ratio, cap_orig, cap_max, cap;
  float backlight = la_mod->la_config.low_beam;
  float frontlight = la_mod->la_config.high_beam;
  register int i;

  /* LA adjustable range [0, EqRange); y=x for luma values [EqRange, 255] */
  const int EqRange = 255;  /* Original: 255 */

  /* Adjusts width of Gaussian cap curve, default: 3.6 */
  cap_adjust = la_mod->la_config.cap_adjust;
  /* Normal: 0.05; Backlight: 0.25 */
  cap_ratio  = la_mod->la_config.cap_high;
  /* Height of Gaussian above 1.0, default: 1.5 */
  cap_orig   = la_mod->la_config.cap_high;
  /* Normal: 3; Backlight: 12 */
  cap_max    = la_mod->la_config.histogram_cap;
  /* Normal: 100; Backlight: 70 */
  CDF_50_threshold = la_mod->la_config.CDF_50_thr;

  hist = (uint32_t*)malloc(256*sizeof(uint32_t));
  if (!hist) {
    CDBG_ERROR("isp_stats_calc_hist_curve malloc failed \n");
    return -1;
  }
  H = (uint64_t*)malloc(256*sizeof(uint64_t));
  if (!H) {
    free(hist);
    hist = NULL;
    CDBG_ERROR("isp_stats_calc_hist_curve malloc failed \n");
    return -1;
  }
  threshold = (uint32_t*)malloc(256*sizeof(uint32_t));
  if (!threshold) {
    free(hist);
    free(H);
    CDBG_ERROR("isp_stats_calc_hist_curve malloc failed \n");
    return -1;
  }

  /* Total histogram counts */
  for (i=0; i<256; i++) {
     /* Original histogram bins */
     hist[i] = ihist_stats->isp_ihist_data[i];
     /* New total count */
     size += hist[i];
  }

  /*compute original CDF, then calculate cap*/
  H[0] = hist[0];
  num_hist_pixels = H[0];

  /*Avoid the saturated pixels*/
  for (i=1; i<=255; i++) {
     if (i <= 250) {
        H[i] = H[i-1] + hist[i];
     }
     num_hist_pixels += hist[i];
  }
  /*if overall histogram is bright and saturated, then return from LA*/
  if (H[250] < la_get_min_pdf_count(num_hist_pixels)) {
    ISP_DBG(ISP_MOD_LA, "%s: pdf count %llu", __func__, H[250]);
    free(hist);
    free(H);
    free(threshold);
    return -1;
  }

  if (H[250] == 0) {
    CDBG_ERROR("%s: ALL Ihist stats = 0!!\n", __func__);
    return -1;
  }
  /*Avoid the saturated pixels*/
  for (i=1; i<=250; i++) {
     H[i] = H[i] * 255 / H[250];
  }
  /* check in scale of bin 50's */
  cap = cap_orig;
  if (H[50] > CDF_50_threshold) {
        cap = cap_orig + (H[50] - CDF_50_threshold) * cap_ratio;
        cap = (cap < cap_max)? cap : cap_max;
  }
  /* new curve */
  for (i=0; i<256; i++) {
     /* Nonlinear cap curve */
     tmp = 0;
     /* interpolation */
     if ((int)(i * cap_adjust) < 255) {
       tmp0 = (float)curve[(int16_t)(i * cap_adjust)];
       tmp1 = (float)curve[(int16_t)(i * cap_adjust) + 1];
       tmp  = tmp0 + (tmp1 - tmp0) * (i * cap_adjust -
         (int16_t)(i * cap_adjust));
       threshold[i] = (uint32_t)((((uint32_t)(tmp * cap) + 64) * (size >> 8)) >> 6);
     } else {
       threshold[i] = (uint32_t)(size >> 8);  /* (64*(size>>8))>>6 */
     }
  }
  /* apply cap to histogram curve */
  avg_inc = 0;
  iter_cnt = 0;
  do {
    for (i=0; i<256; i++) {
       /* Add back average capped histogram counts to uncapped bins */
       if (hist[i] < threshold[i])
         hist[i] += avg_inc;  /* KSH: Changed from < to <= for uncapped bins */
       }
       for (i=0, capped_count = high_bin = full_bin = 0; i<256; i++) {
          /* Cap the histogram if bin count > threshold */
          if (hist[i] > threshold[i]) {
            high_bin++;
            capped_count += (hist[i] - threshold[i]);
            hist[i] = threshold[i];
          } else if (hist[i] == threshold[i]) {
                    full_bin++;
          }
       }
       ISP_DBG(ISP_MOD_LA, "iterartion %d,", iter_cnt);
       if ((full_bin+high_bin) < 256)  /*alway true*/
         /* Distribute capped histogram counts to uncapped bins */
         avg_inc = capped_count / (256-full_bin-high_bin);
         iter_cnt++;
        ISP_DBG(ISP_MOD_LA, "full_bin, %d, high_bin, %d, avg_inc, %d\n",
          full_bin, high_bin, avg_inc);
  } while (high_bin > 0 && iter_cnt < 10);
  /* Adjust histogram: Offset, Low light boost, High light boost */
  /* adjusted histogram */
  size = 0;
  for (i=0; i<256; i++) {
     hist[i] += Offset;
     /*assert((32 + backlight * 4) <= (256 - 32- (frontlight * 4)));*/
     if (i < (32+(int)(backlight*4))) {
       /* Low light boost */
       hist[i] = (uint32_t)((hist[i]) * (1.0f + (float)(32 + backlight * 4 - i) *
         backlight * 0.4f/36.0f));
     }
     if (i > (256-32-(int)(frontlight * 4))) {
       /* High light boost */
       hist[i] = (uint32_t)((hist[i])*(1.0f + (float)(i - (256 - 32 -
         (frontlight * 4))) * frontlight * 0.6f/32.0f));
     }
     /* New total count */
     size += hist[i];
  }
  /* Compute LA curve / Compute target CDF */
  H[0] = hist[0];
  for (i=1; i <= EqRange; i++) {
     H[i] = H[i-1] + hist[i];
  }
  /* Scale target CDF with enacted equalization range (default full 255) */
  /*scaled CDF */
  for (i=0; i <= EqRange; i++) {
     if (H[EqRange] == 0) {
        CDBG_ERROR("%s: H[EqRange] = 0\n", __func__);
        return -1;
     }
     H[i] = EqRange * H[i]/H[EqRange];
  }
  for ( ; i<256; i++) {
     /* Straight line y=x of slope 1 after EqRange */
     H[i] = i;
  }
  /* Smooth target mapping function */
  for (iter_cnt=0; iter_cnt<1; iter_cnt++) {
     H[0]=0;
     H[1]=(0+H[1]+H[2])/3;
     for (i=2;i<254;i++) {
        H[i] = (H[i-2]+H[i-1]+H[i]+H[i+1]+H[i+2])/5;
     }
     H[254]=(H[253]+H[254]+255)/3;
     H[255]=255;
  }
  /* smoothed CDF (final output) */
  for (i=0; i<256; i++) {
     la_curve[i] = H[i];
  }
  if (threshold) free(threshold);
  if (hist) free(hist);
  if (H) free(H);
  threshold = hist = NULL;
  H = NULL;
  return 0;
}

/** la_prepare_hw_entry
 *    @la_curve: the hist cap curve to adjust hist pixels
 *
 *  this function prepare the hw DMI table entries
 **/
static int la_prepare_hw_entry(isp_la_mod_t *la_mod, uint8_t *la_curve,
  isp_pix_trigger_update_input_t *trigger_params)
{
  int rc = 0;
  int i,j;
  int32_t val;
  int Q_yratio, unit_Q;

  /* 8974 V1 is is using 6 bit accuracy(Q6) for y ratio curve
     8974 and above is using 10 bit(Q10) */
  if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2)
    Q_yratio = 6;
  else
    Q_yratio = 10;

  unit_Q = 1 << Q_yratio;

  /* Cipher 256-entry Y_ratio curve to 64-entry LUT */
  for (i=0; i<64; i++) {
    val = 0;
    for (j=0; j<4; j++) {
      if (i || j)
        val += ((int)la_curve[(i<<2)+j] << Q_yratio) / ((i<<2)+j);
      else // both i & j are 0
        val += unit_Q;
    }
    la_mod->LUT_Yratio[i] = (val >> 2); //devided by 4, unsign (Qyratio+2) bits

    if (la_mod->LUT_Yratio[i] > (int32_t)(3.99 * unit_Q))
      la_mod->LUT_Yratio[i] = (int32_t)(3.99 * unit_Q);

    if (la_mod->LUT_Yratio[i] < (int32_t)(0.75 * unit_Q))
      la_mod->LUT_Yratio[i] = (int32_t)(0.75 * unit_Q);
  }

  for (i=0; i<63; i++) {
    val = la_mod->LUT_Yratio[i+1] - la_mod->LUT_Yratio[i];

    if (val > ((1 << (Q_yratio + 1)) - 1))
      val = (1 << (Q_yratio + 1)) - 1;
    if (val < -(1 << (Q_yratio + 1)))
      val = -(1 << (Q_yratio + 1));

    if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
      la_mod->LUT_Yratio[i] = (int32_t)(val << 8) | la_mod->LUT_Yratio[i];  /* 16s */
    } else {
      la_mod->LUT_Yratio[i] = (int32_t) (
        ((val & 0x00F) << 20) |
        ((la_mod->LUT_Yratio[i] & 0x00F) << 16) |
        ((val & 0xFF0) << 4) |
        ((la_mod->LUT_Yratio[i] & 0xFF0) >> 4));
    }
  }

  /* Fill in the last entry */
  val = la_mod->LUT_Yratio[ISP_LA_TABLE_LENGTH - 1];

  if (val > ((1 << (Q_yratio + 1)) - 1))
    val = (1 << (Q_yratio + 1)) - 1;
  if (val < -(1 << (Q_yratio + 1)))
    val = -(1 << (Q_yratio + 1));

  if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
      la_mod->LUT_Yratio[ISP_LA_TABLE_LENGTH - 1] =
         (int32_t)(val << 8) | la_mod->LUT_Yratio[ISP_LA_TABLE_LENGTH - 1];  /* 16s */
  } else {
    la_mod->LUT_Yratio[ISP_LA_TABLE_LENGTH - 1] = (int32_t) (
      ((val & 0x00F) << 20) |
      ((la_mod->LUT_Yratio[ISP_LA_TABLE_LENGTH - 1] & 0x00F) << 16) |
      ((val & 0xFF0) << 4)|
      ((la_mod->LUT_Yratio[ISP_LA_TABLE_LENGTH - 1] & 0xFF0) >> 4));
   }

  for (i = 0; i<ISP_LA_TABLE_LENGTH ; i++){
    la_mod->la_cmd.TblEntry.table[i] = (int16_t)la_mod->LUT_Yratio[i];
  }

  return rc;
}

/** average_la_curve
 *
 * This function calculate average values between two la_curves
 * We take three parameters, First one is old value of curve,
 * second one is new value of curve and we put average values in
 * the third parameter.
 *
 **/
static void average_la_curve(uint8_t *la_curve_old,
  uint8_t *la_curve_new, uint8_t *la_curve_avg)
{
  uint32_t i;

  for (i=0; i<256; i++){
    la_curve_avg[i] = (la_curve_old[i] + la_curve_new[i])/2;
  }
}

/** stats_process_hist
 *
 * This function process histogram stats from the isp and
 * updates the Luma Adaptation LUT, this function is called by
 * media controller when stats obtained from kernel
 *
 **/
static int la_hist_trigger_update(isp_la_mod_t *la_mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  uint8_t la_curve[256];
  uint8_t la_curve_avg[256];
  uint32_t i,j;
  int32_t N_pixels, offset;
  float equalize;
  isp_ihist_params_t *ihist_stats =
    &(in_params->trigger_input.stats_update.ihist_params);
  chromatix_parms_type *pchromatix =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  equalize = la_mod->la_config.offset;

  if (la_mod->bracketing_data.state != MCT_BRACKET_CTRL_OFF) {
    ISP_DBG(ISP_MOD_LA, "%s: luma adaptation need to be disabled for bracketing mode\n",
      __func__);
    return 0;
  }

  if (!isp_util_aec_check_settled(&(in_params->trigger_input.stats_update.aec_update))) {
      ISP_DBG(ISP_MOD_LA, "%s: AEC is not setteled. Skip the trigger\n", __func__);
      return 0;
  }

  /* Reset N_pixel to match histogram for histogram offset */
  N_pixels = 0;

  for (i=0; i<256; i++){
    N_pixels += ihist_stats->isp_ihist_data[i];
  }
  offset = (N_pixels * equalize) / 256;

  /* system algo: Compute LA Y->Ynew curve */
  rc = stats_calc_hist_curve(ihist_stats, la_mod, offset, la_curve);
  if (rc != 0) {
    /* since we cannot calculate the new curve, use the old table */
    ISP_DBG(ISP_MOD_LA, "%s: calculate new la curve fail, use previous table\n", __func__);
    return 0;
  }

  /* Calculate average only after we rise la_mod->la_curve_is_valid flag */
  if (la_mod->la_curve_is_valid)
    average_la_curve(la_mod->la_curve, la_curve, la_curve_avg);
  else
    memcpy(la_curve_avg, la_curve, sizeof(la_curve_avg));

  /* system algo: to pack la_curve to hw dmi entry*/
  rc = la_prepare_hw_entry(la_mod, la_curve_avg, in_params);
  if (rc != 0) {
    ISP_DBG(ISP_MOD_LA, "%s: pack la curve to hw entry fail, use previous table\n", __func__);
    return 0;
  }

  /* save la_curve in isp la mod structure */
  memcpy(la_mod->la_curve, la_curve, sizeof(la_mod->la_curve));
  la_mod->la_curve_is_valid = TRUE;

  la_mod->hw_update_pending = TRUE;
  return rc;
}

/** la_enable
 *
 * description: enable la
 *
 **/
static int la_enable(isp_la_mod_t *la,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  la->la_enable = enable->enable;

  return 0;
}

/** la_init_lut_y_ratio
 *    @lut_y_ratio: Y ratio LUT
 *
 *  Fills Y ratio LUT with default values.
 *
 **/
static void la_init_lut_y_ratio(int32_t *lut_y_ratio)
{
  uint32_t i;
  /*initial Luma Adaptation table*/
  for (i = 0; i < ISP_LA_TABLE_LENGTH ;i++) {
    lut_y_ratio[i] = 64;
  }
}

/** la_set_bracketing_data
 *
 * updates the ISP with the bracketing data
 *
 **/
static int la_set_bracketing_data(isp_la_mod_t *la_mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  la_mod->bracketing_data = in_params->bracketing_data;

  return 0;
}

/** la_set_bestshot
 *
 * updates the ISP with the backlight/default LA LUT
 *
 **/
static int la_set_bestshot(isp_la_mod_t *la_mod, isp_hw_pix_setting_params_t *in_params,
  uint32_t in_param_size)
{
  int rc = 0;

  isp_mod_set_enable_t mod_enable;
  mod_enable.enable = TRUE;

  switch (in_params->bestshot_mode) {
  case CAM_SCENE_MODE_BACKLIGHT: {
    memcpy(la_mod->LUT_Yratio, default_backlight_la_tbl,
      sizeof(la_mod->LUT_Yratio));
    mod_enable.enable = TRUE;
  }
    break;

  case CAM_SCENE_MODE_SUNSET:
  case CAM_SCENE_MODE_FLOWERS:
  case CAM_SCENE_MODE_CANDLELIGHT: {
    /*Disable Module for these Best Shot Mode*/
    mod_enable.enable = FALSE;
  }
    break;

  default:
    la_mod->la_trigger_enable = TRUE;
    mod_enable.enable = TRUE;
    la_init_lut_y_ratio(la_mod->LUT_Yratio);
    break;
  }

  if(0 != la_enable(la_mod, &mod_enable, sizeof(isp_mod_set_enable_t))) {
    ISP_DBG(ISP_MOD_LA, "%s: LA Enable/Diable Failed", __func__);
  }

  return 0;
}

/** la_cfg_debug
 *
 * print debug message for LA prameters (for la_curve algorithm)
 *
 **/
static void la_param_debug(isp_la_mod_t *la_mod){
  ISP_DBG(ISP_MOD_LA, "%s:\n",__func__);

  /*LA algo params*/
  ISP_DBG(ISP_MOD_LA, "%s: la_config.offset: %f\n", __func__, la_mod->la_config.offset);
  ISP_DBG(ISP_MOD_LA, "%s: low beam: %f\n", __func__, la_mod->la_config.low_beam);
  ISP_DBG(ISP_MOD_LA, "%s: high beam: %f\n", __func__, la_mod->la_config.high_beam);
  ISP_DBG(ISP_MOD_LA, "%s: histogram cap: %f\n", __func__, la_mod->la_config.histogram_cap);
  ISP_DBG(ISP_MOD_LA, "%s: cap high: %f\n", __func__, la_mod->la_config.cap_high);
  ISP_DBG(ISP_MOD_LA, "%s: cap low: %f\n", __func__, la_mod->la_config.cap_low);

}

/** la_cfg_debug
 *
 * print la config debug message
 *
 **/
static void la_cfg_debug(isp_la_mod_t *la_mod){
  int i;

  /*LA DMI table SEL*/
  ISP_DBG(ISP_MOD_LA, "%s: lutBankSelect: %d\n", __func__, la_mod->la_cmd.CfgCmd.lutBankSelect);

  for (i = 0; i < ISP_LA_TABLE_LENGTH ; i++) {
    ISP_DBG(ISP_MOD_LA, "%s: TblEntry.table[%d] = %d\n", __func__, i, la_mod->la_cmd.TblEntry.table[i]);

  }
}

/** la_prep_spl_effect_lut
 *
 * DESCRIPTION: Prepare special effects table in Init state.
 *
 **/
static void la_prep_spl_effect_lut(int *spl_eff_lut, const uint8_t *tbl)
{
  int32_t val, i;
  memset(spl_eff_lut, 0, ISP_LA_TABLE_LENGTH);
  for (i=0; i < 63; i++) {
    val = tbl[i+1] - tbl[i];
    if (val > 127) val = 127;
    if (val < -128) val = -128;
    val = val & 0xFF;  /* 8s */
    spl_eff_lut[i] = (val<<8) | tbl[i];  /* 16s */
  }

  /* Fill in the last entry */
  val = tbl[ISP_LA_TABLE_LENGTH - 1];
  if (val > 127) val = 127;
  if (val < -128) val = -128;
  val = val & 0xFF;  /* 8s */
  spl_eff_lut[ISP_LA_TABLE_LENGTH - 1] =
  (val<<8) | tbl[ISP_LA_TABLE_LENGTH  - 1];  /* 16s */
}

/** la_trigger_update
 *
 * interpolating the la_config based on aec result.
 *
 **/
static int la_trigger_update(isp_la_mod_t *la_mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int i;
  float ratio = 0.0;
  chromatix_parms_type *chromatix =
    (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_LA_type *chromatix_LA =
    &chromatix->chromatix_VFE.chromatix_LA;
  ASD_struct_type *ASD_algo_data = &chromatix->ASD_algo_data;

  int8_t is_burst = IS_BURST_STREAMING((&trigger_params->cfg));
  tuning_control_type *tc;
  trigger_point_type  *tp;

  la_8k_type la_8k_config_indoor, la_8k_config_outdoor;
  la_8k_type la_config_compensated, la_config_backlight;
  asd_update_t *sd_out = &(trigger_params->trigger_input.stats_update.asd_update);
  uint32_t backlight_scene_severity = 0;

  if (!la_mod->la_enable || !la_mod->la_trigger_enable) {
    ISP_DBG(ISP_MOD_LA, "%s: no trigger update fo LA:LA enable = %d, trigger_enable = %d",
         __func__, la_mod->la_enable, la_mod->la_trigger_enable);
    return 0;
  }

  if (trigger_params->cfg.bestshot_mode == CAM_SCENE_MODE_BACKLIGHT)
    backlight_scene_severity = 255; /* max severity*/
  else
    backlight_scene_severity =
      MIN(255, sd_out->backlight_scene_severity);

  ISP_DBG(ISP_MOD_LA, "%s: current streaming mode = %d", __func__, trigger_params->cfg.streaming_mode);

  la_cfg_set_from_chromatix(&la_8k_config_indoor, &chromatix_LA->LA_config);
  la_cfg_set_from_chromatix(&la_8k_config_outdoor,
    &chromatix_LA->LA_config_outdoor);
  tc = &(chromatix_LA->control_la);
  tp = &(chromatix_LA->la_brightlight_trigger);

  ratio = isp_util_get_aec_ratio(la_mod->notify_ops->parent, *tc, tp,
    &trigger_params->trigger_input.stats_update.aec_update, is_burst);

  if (ratio > 1.0)
    ratio = 1.0;
  else if (ratio < 0.0)
    ratio = 0.0;

  ISP_DBG(ISP_MOD_LA, "%s: aec ratio = %f\n", __func__, ratio);
  la_mod->la_config.offset = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.offset, la_8k_config_outdoor.offset, ratio));
  la_mod->la_config.low_beam = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.low_beam, la_8k_config_outdoor.low_beam,
    ratio));
  la_mod->la_config.high_beam = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.high_beam, la_8k_config_outdoor.high_beam,
    ratio));
  la_mod->la_config.histogram_cap = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.histogram_cap, la_8k_config_outdoor.histogram_cap,
    ratio));
  la_mod->la_config.cap_high = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_high, la_8k_config_outdoor.cap_high, ratio));
  la_mod->la_config.cap_low = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_low, la_8k_config_outdoor.cap_low, ratio));

  la_mod->la_config.cap_adjust = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_adjust, la_8k_config_outdoor.cap_adjust, ratio));

  la_mod->la_config.CDF_50_thr = (float)(LINEAR_INTERPOLATION(
    la_8k_config_indoor.CDF_50_thr, la_8k_config_outdoor.CDF_50_thr, ratio));

  if (backlight_scene_severity != 0) {
    la_config_compensated = la_mod->la_config;
    la_cfg_set_from_chromatix(&la_config_backlight,
      &ASD_algo_data->backlit_scene_detect.backlight_la_8k_config);

    la_mod->la_config.offset = (la_config_compensated.offset * (255 -
      sd_out->backlight_scene_severity) + la_config_backlight.offset *
      sd_out->backlight_scene_severity)/255.0;

    la_mod->la_config.low_beam = (la_config_compensated.low_beam * (255 -
      sd_out->backlight_scene_severity) + la_config_backlight.low_beam *
      sd_out->backlight_scene_severity)/255.0;

    la_mod->la_config.high_beam = (la_config_compensated.high_beam * (255
      - sd_out->backlight_scene_severity) + la_config_backlight.high_beam *
      sd_out->backlight_scene_severity)/255.0;

    la_mod->la_config.histogram_cap =(la_config_compensated.histogram_cap
      * (255 - sd_out->backlight_scene_severity) + la_config_backlight.
      histogram_cap * sd_out->backlight_scene_severity)/255.0;

    la_mod->la_config.cap_high = (la_config_compensated.cap_high * (255 -
      sd_out->backlight_scene_severity) + la_config_backlight.cap_high *
      sd_out->backlight_scene_severity)/255;

    la_mod->la_config.cap_low = (la_config_compensated.cap_low * (255 -
      sd_out->backlight_scene_severity) + la_config_backlight.cap_low *
      sd_out->backlight_scene_severity)/255.0;

    la_mod->la_config.cap_adjust = (la_config_compensated.cap_adjust * (255 -
      sd_out->backlight_scene_severity) + la_config_backlight.cap_adjust *
      sd_out->backlight_scene_severity)/255.0;

    la_mod->la_config.CDF_50_thr = (la_config_compensated.CDF_50_thr *
      (255 - sd_out->backlight_scene_severity) +
      la_config_backlight.CDF_50_thr * sd_out->backlight_scene_severity)
        / 255.0;
  }

  ISP_DBG(ISP_MOD_LA, "%s: backlight_scene_severity :%d \n", __func__, sd_out->backlight_scene_severity);

  /*update LA HW write command*/
  for (i = 0; i<ISP_LA_TABLE_LENGTH ; i++)
    la_mod->la_cmd.TblEntry.table[i] = (int16_t)la_mod->LUT_Yratio[i];

  la_mod->hw_update_pending = TRUE;
  return 0;
}

/** la_set_spl_effect
 *
 * set special setting/tables for different effect
 *
 **/
static int la_set_spl_effect(isp_la_mod_t *mod, isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  int rc =0;
  int i;
  int32_t *pLUT_Yratio;

  chromatix_parms_type *chromatix_ptr =
     (chromatix_parms_type *)pix_settings->chromatix_ptrs.chromatixPtr;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
  return -1;
  }

  if (pix_settings->bestshot_mode != CAM_SCENE_MODE_OFF) {
    ISP_DBG(ISP_MOD_LA, "%s: Best shot enabled, skip seteffect", __func__);
    return 0;
  }

  /* no need to trigger update for special effect */
  mod->la_trigger_enable = FALSE;

  ISP_DBG(ISP_MOD_LA, "%s: effect %d", __func__, pix_settings->effects.spl_effect);
  switch (pix_settings->effects.spl_effect) {
  case CAM_EFFECT_MODE_POSTERIZE: {
    pLUT_Yratio = mod->posterize_la_tbl;
  }
    break;

  case CAM_EFFECT_MODE_SOLARIZE: {
    pLUT_Yratio = mod->solarize_la_tbl;
  }
    break;

  default:
    mod->la_trigger_enable = TRUE;
    pLUT_Yratio = mod->LUT_Yratio;
    break;
  }

  for (i = 0; i<ISP_LA_TABLE_LENGTH ; i++)
    mod->la_cmd.TblEntry.table[i] = (int16_t)pLUT_Yratio[i];

  return rc;
}

/** la_reset
 *
 *  reset the module param when all stream off
 *
 **/
static void la_reset(isp_la_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->la_cmd, 0, sizeof(mod->la_cmd));
  mod->hw_update_pending = 0;
  mod->la_trigger_enable = 0;
  mod->la_enable = 0;
  memset(&mod->LUT_Yratio, 0, sizeof(mod->LUT_Yratio));
  memset(&mod->solarize_la_tbl, 0, sizeof(mod->solarize_la_tbl));
  memset(&mod->posterize_la_tbl, 0, sizeof(mod->posterize_la_tbl));
  memset(&mod->la_config, 0, sizeof(mod->la_config));
  mod->la_curve_is_valid = FALSE;
}

/** la_init
 *
 * init when open isp hw
 *
 **/
static int la_init (void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_la_mod_t *la_mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  la_mod->isp_version = init_params->isp_version;
  la_mod->fd = init_params->fd;
  la_mod->notify_ops = notify_ops;
  la_mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  la_mod->hw_update_pending = FALSE;
  la_mod->la_curve_is_valid = FALSE;
  la_reset(la_mod);
  return 0;
}

/** la_config
 *
 * initial configuration when first streamon
 *
 **/
static int la_config(isp_la_mod_t *la_mod, isp_hw_pix_setting_params_t *in_params,
                     uint32_t in_param_size)
{
  int  rc = 0;
  uint32_t i;

  ISP_DBG(ISP_MOD_LA, "%s: E\n",__func__);

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
  return -1;
  }

  la_init_lut_y_ratio(la_mod->LUT_Yratio);
  la_prep_spl_effect_lut(la_mod->solarize_la_tbl,
    solarize_la);
  la_prep_spl_effect_lut(la_mod->posterize_la_tbl,
    posterize_la);

  /*update LA HW write command*/
  for (i = 0; i<ISP_LA_TABLE_LENGTH ; i++)
    la_mod->la_cmd.TblEntry.table[i] = (int16_t)la_mod->LUT_Yratio[i];

  la_mod->hw_update_pending = TRUE;

  return rc;
}

/** la_trigger_enable
 *
 * description: enable trigger update feature
 *
 **/
static int la_trigger_enable(isp_la_mod_t *la,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  la->la_trigger_enable = enable->enable;

  return 0;
}

/** la_destroy
 *
 * description: close la
 *
 **/
static int la_destroy (void *mod_ctrl)
{
  isp_la_mod_t *la = mod_ctrl;

  memset(la,  0,  sizeof(isp_la_mod_t));
  free(la);
  return 0;
}

/** la_set_params
 *
 * description: set parameters
 *
 **/
static int la_set_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_la_mod_t *la = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = la_enable(la, (isp_mod_set_enable_t *)in_params,
      in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = la_config(la, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = la_trigger_enable(la, (isp_mod_set_enable_t *)in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = la_trigger_update(la, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_BRACKETING_DATA: {
    rc = la_set_bracketing_data(la, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_BESTSHOT: {
    rc = la_set_bestshot(la,(isp_hw_pix_setting_params_t *)in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_LA_HIST_UPDATE: {
    /*enable LA_8k_enable, enable dynamic hist curve calculation*/
    rc = la_hist_trigger_update(la,
      (isp_pix_trigger_update_input_t *)in_params, in_param_size);
  }
    break;
  case ISP_HW_MOD_SET_EFFECT: {
     rc = la_set_spl_effect(la,(isp_hw_pix_setting_params_t *)in_params,
       in_param_size);
  }
     break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }

  return rc;
}

/** la_get_params
 *
 * description: get parameters
 *
 **/
static int la_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_la_mod_t *la = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = la->la_enable;
    break;
  }
  case ISP_HW_MOD_GET_TBLS: {
    mct_isp_table_t *isp_tbls = (mct_isp_table_t *)out_params;
    if (sizeof(mct_isp_table_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      return -1;
    }

    isp_tbls->la_enable = la->la_enable;
    if (la->la_enable && isp_tbls->luma_table) {
       memcpy(isp_tbls->luma_table, &la->la_cmd.TblEntry, sizeof(ISP_LA_TblEntry));
       isp_tbls->luma_num_entries = ISP_LA_TABLE_LENGTH;
    }

    break;
  }
  case ISP_HW_MOD_GET_TABLE_SIZE: {
    isp_hw_read_info *read_info = out_params;

    read_info->read_type = VFE_WRITE_DMI_16BIT;
    read_info->read_bank = LA_LUT_RAM_BANK0;
    read_info->bank_idx = 0;

    /* Linearization tbl len*/
    read_info->read_lengh = sizeof(int16_t) * ISP_LA_TABLE_LENGTH;
  }
    break;
  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/** la_single_HW_write
 *
 *  only write one register
 *
 **/
static int la_single_HW_write(uint32_t fd, void* cmd_offset, uint32_t cmd_len,
  uint32_t hw_reg_offset, uint32_t reg_num, uint32_t cmd_type)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  cfg_cmd.cfg_data = cmd_offset;
  cfg_cmd.cmd_len = cmd_len;
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = cmd_type;
  reg_cfg_cmd[0].u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd[0].u.rw_info.len = reg_num * sizeof(uint32_t);

  reg_cfg_cmd[0].u.dmi_info.hi_tbl_offset = 0;
  reg_cfg_cmd[0].u.dmi_info.lo_tbl_offset = 0;

  reg_cfg_cmd[0].u.dmi_info.hi_tbl_offset = 0;
  reg_cfg_cmd[0].u.dmi_info.lo_tbl_offset = 0;
  reg_cfg_cmd[0].u.dmi_info.len = cmd_len;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0){
    CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
    return rc;
  }

  return rc;

}

/** la_reset_dmi_cfg
 *
 * description: la_reset_dmi_cfg
 *
 **/
static int la_reset_dmi_cfg(isp_la_mod_t *la_mod, uint32_t la_channel)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[2];
  uint32_t dmi_cfg[2];

  /* reset dmi cfg: config dmi channel and set auto increment*/
  dmi_cfg[0] = ISP40_DMI_CFG_DEFAULT;
  dmi_cfg[0] += la_channel;

  /* reset dmi_addr_cfg: dmi address always start form 0 */
  dmi_cfg[1] = 0;

  /* PACK the 2 cfg cmd for 1 ioctl*/
  cfg_cmd.cfg_data = &dmi_cfg;
  cfg_cmd.cmd_len = sizeof(dmi_cfg);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 2;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP40_DMI_CFG_OFF;
  reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP40_DMI_ADDR;
  reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

  rc = ioctl(la_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0){
    CDBG_ERROR("%s: la DMI update error, rc = %d", __func__, rc);
    return rc;
  }

  return rc;
}

/** la_dmi_hw_update
 *
 * description: la_dmi_hw_update
 *
 **/
static int la_dmi_hw_update(isp_la_mod_t *la_mod, uint32_t bank_sel)
{
  int rc = 0;
  uint32_t dmi_cfg;
  uint32_t dmi_addr_reset = 0;

  int16_t *tbl = &la_mod->la_cmd.TblEntry.table[0];
  uint32_t tbl_len = sizeof(int16_t) * ISP_LA_TABLE_LENGTH;

  uint32_t la_channel =
    (bank_sel == 0)? LA_LUT_RAM_BANK0 : LA_LUT_RAM_BANK1;

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg
     4. flip the banksel bit*/

  /* write gamma channel 0 */
  la_reset_dmi_cfg(la_mod, la_channel);

  la_single_HW_write(la_mod->fd, (void *)tbl, tbl_len,
    ISP40_DMI_DATA_LO, 1, VFE_WRITE_DMI_16BIT);

  la_reset_dmi_cfg(la_mod, ISP40_DMI_NO_MEM_SELECTED);

  return rc;
}

/** function name: la_do_hw_update
 *
 * description: la_do_hw_update
 *
 **/
static int la_do_hw_update(isp_la_mod_t *la_mod)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (la_mod->hw_update_pending) {
    la_param_debug(la_mod);
    la_cfg_debug(la_mod);

    /*write DMI table first*/
    rc = la_dmi_hw_update(la_mod, la_mod->la_cmd.CfgCmd.lutBankSelect);
    if (rc < 0){
      CDBG_ERROR("%s: DMI update error, rc = %d", __func__, rc);
      return rc;
    }

    /*config the banksel after done writing into DMI*/
    cfg_cmd.cfg_data = (void *) &la_mod->la_cmd ;
    cfg_cmd.cmd_len = sizeof(la_mod->la_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_LA40_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_LA40_LEN * sizeof(uint32_t);

    rc = ioctl(la_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    /*flip the banksel bit*/
    la_mod->la_cmd.CfgCmd.lutBankSelect ^= 1;
    la_mod->hw_update_pending = 0;
  }

  return rc;
}

/** la_action
 *
 * description: processing the action
 *
 **/
static int la_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_la_mod_t *la = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = la_do_hw_update(la);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET: {
    la_reset(la);
  }
    break;

  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}

/** la44_open
 *
 * description: open la
 *
 **/
isp_ops_t *la44_open(uint32_t version)
{
  isp_la_mod_t *la = malloc(sizeof(isp_la_mod_t));

  if (!la) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(la,  0,  sizeof(isp_la_mod_t));

  la->isp_version = version;
  la->ops.ctrl = (void *)la;
  la->ops.init = la_init;
  /* destroy the module object */
  la->ops.destroy = la_destroy;
  /* set parameter */
  la->ops.set_params = la_set_params;
  /* get parameter */
  la->ops.get_params = la_get_params;
  la->ops.action = la_action;
  return &la->ops;
}
