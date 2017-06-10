/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_LA_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define MIN_SAT_PIXELS_PERCENT .1
//default backlight table for bestshot
int32_t default_backlight_la_tbl[] = {
  0x2d30, 0x1b5d, 0x0e78, 0x0a86, 0x0a90, 0x0d9a, 0x0ca7, 0x05b3, 0xffb8,
  0xfbb7, 0xfab2, 0xfaac, 0xfaa6, 0xfaa0, 0xfb9a, 0xfa95, 0xfb8f, 0xfc8a,
  0xfb86, 0xfc81, 0xfc7d, 0xfc79, 0xfd75, 0xfd72, 0xfd6f, 0xfd6c, 0xfd69,
  0xfe66, 0xfe64, 0xfe62, 0xfe60, 0xfe5e, 0xfe5c, 0xfe5a, 0xff58, 0xfe57,
  0xff55, 0xff54, 0xfe53, 0xff51, 0xff50, 0xff4f, 0xff4e, 0xff4d, 0xff4c,
  0xff4b, 0xff4a, 0x0049, 0xff49, 0xff48, 0xff47, 0x0046, 0xff46, 0xff45,
  0x0044, 0xff44, 0x0043, 0xff43, 0xff42, 0x0041, 0xff41, 0x0040, 0x0040,
  0x0040
};

const uint16_t curve[256] = {
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

const uint8_t solarize_la[64] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 61, 57, 54, 51, 47, 44, 42, 39,
    36, 34, 32, 29, 27, 25, 23, 22, 20, 18,
    17, 15, 13, 12, 11, 9,  8,  7,  5,  4,
    3,  2,  1,  0};

const uint8_t posterize_la[64] = {
    0,  0,  0,  0,  0,  0,  0,  0, 0,  0,
    64, 58, 53, 50, 46, 43, 41, 38, 36, 34,
    66, 63, 60, 57, 55, 53, 51, 49, 47, 46,
    67, 65, 63, 61, 59, 57, 56, 54, 53, 51,
    67, 65, 64, 62, 61, 60, 58, 57, 56, 55,
    67, 66, 65, 64, 62, 61, 60, 59, 58, 57,
    67, 66, 65, 64};

#ifdef FEATURE_VFE_TEST_VECTOR
const uint32_t test_hist[256] = {
   0, 0, 9, 63, 50, 60, 65, 60, 77, 84, 104, 97, 118, 161, 150, 143,
   155, 160, 208, 287, 312, 323, 355, 361, 357, 350, 357, 435, 482, 445, 396, 421,
   439, 492, 523, 613, 601, 666, 674, 706, 727, 850, 877, 1020, 1048, 1108, 1065, 1211,
   1327, 1375, 1367, 1386, 1346, 1320, 1464, 1366, 1188, 1029, 944, 906, 892, 838, 846, 788,
   823, 769, 782, 875, 809, 849, 779, 850, 885, 940, 933, 937, 941, 891, 965, 915,
   932, 940, 1068, 1091, 1101, 1064, 1177, 1142, 1162, 1256, 1255, 1324, 1328, 1432, 1571, 1580,
   1564, 1639, 1810, 1906, 1927, 1751, 1620, 1659, 1613, 1591, 1537, 1675, 1774, 1916, 1792, 1391,
   1208, 786, 488, 332, 276, 253, 232, 252, 215, 237, 241, 204, 247, 260, 235, 232,
   272, 279, 293, 326, 254, 220, 228, 228, 194, 218, 186, 192, 205, 210, 244, 241,
   275, 236, 282, 279, 276, 309, 283, 333, 373, 330, 372, 401, 495, 458, 434, 409,
   423, 413, 392, 405, 402, 399, 467, 492, 438, 393, 335, 352, 371, 412, 405, 384,
   389, 380, 404, 416, 410, 394, 377, 421, 479, 491, 506, 543, 499, 555, 596, 704,
   785, 877, 921, 967, 1038, 1091, 1135, 1139, 1001, 903, 862, 809, 837, 836, 728, 728,
   795, 1060, 1151, 1016, 858, 687, 598, 560, 558, 364, 331, 210, 147, 164, 156, 182,
   187, 218, 232, 234, 310, 288, 303, 289, 307, 222, 133, 25, 8, 4, 8, 2,
   5, 3, 6, 8, 9, 8, 9, 7, 10, 7, 6, 0, 0, 0, 0, 0};
#endif

/*===========================================================================
 * FUNCTION    - la_get_min_pdf_count -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint64_t la_get_min_pdf_count(uint32_t num_pixels)
{
  return (uint64_t)(MIN_SAT_PIXELS_PERCENT * (float)num_pixels);
}/*la_get_min_pdf_count*/

/*===========================================================================
 * FUNCTION    - vfe_stats_calc_hist_curve -
 *
 * DESCRIPTION:
 * This function process histogram stats from the vfe and
 * and calculates the LA curve
 *==========================================================================*/
static vfe_status_t vfe_stats_calc_hist_curve(ihist_stats_t *ihist_stats,
  la_mod_t *la_mod, uint32_t Offset, uint8_t *la_curve)
{
  uint16_t shift, CDF_50_threshold;
  uint64_t size = 0, capped_count, cap_inc, tmp64;
  uint32_t *hist, *threshold, avg_inc;
  /* full_bin: hist==thld, high_bin: hist>thld */
  uint32_t high_bin, full_bin, iter_cnt;
  uint64_t *H;
  float tmp0, tmp1, tmp, cap_adjust, cap_ratio, cap_orig, cap_max, cap;

  float backlight = la_mod->la_config.low_beam;
  float frontlight = la_mod->la_config.high_beam;
  register int i;

#ifndef FEATURE_VFE_TEST_VEC
  /* LA adjustable range [0, EqRange); y=x for luma values [EqRange, 255] */
  const int EqRange = 255;  /* Original: 255 */
  cap_adjust = 3.6;  /* Adjusts width of Gaussian cap curve, default: 3.6 */
  /* Normal: 0.05; Backlight: 0.25 */
  cap_ratio  = 0.02; //la_mod->la_config.cap_high;
  cap_orig   = 0.75; //1.5;  /* Height of Gaussian above 1.0, default: 1.5 */
  /* Normal: 3; Backlight: 12 */
  cap_max    = 1.5; //la_mod->la_config.histogram_cap;
  /* Normal: 100; Backlight: 70 */
  CDF_50_threshold = (uint16_t)(la_mod->la_config.cap_low);
#else
  /* LA adjustable range [0, EqRange); y=x for luma values [EqRange, 255] */
  const int EqRange = 255;  /* Original: 255 */
  cap_adjust = 3.6;  /* Adjusts width of Gaussian cap curve, default: 3.6 */
  /* Normal: 0.05; Backlight: 0.25 */
  cap_ratio  = 0.05; //la_mod->la_config.cap_high;
  cap_orig   = 1.5; //1.5;  /* Height of Gaussian above 1.0, default: 1.5 */
  /* Normal: 3; Backlight: 12 */
  cap_max    = 3; //la_mod->la_config.histogram_cap;
  /* Normal: 100; Backlight: 70 */
  CDF_50_threshold = (uint16_t)100;
#endif

  hist = (uint32_t*)malloc(256*sizeof(uint32_t));
  if (!hist) {
    CDBG_ERROR("vfe_stats_calc_hist_curve malloc failed \n");
    return VFE_ERROR_GENERAL;
  }
  H = (uint64_t*)malloc(256*sizeof(uint64_t));
  if (!H) {
    free(hist);
    hist = NULL;
    CDBG_ERROR("vfe_stats_calc_hist_curve malloc failed \n");
    return VFE_ERROR_GENERAL;
  }
  threshold = (uint32_t*)malloc(256*sizeof(uint32_t));
  if (!threshold) {
    free(hist);
    free(H);
    CDBG_ERROR("vfe_stats_calc_hist_curve malloc failed \n");
    return VFE_ERROR_GENERAL;
  }

  /* Total histogram counts */
  for (i=0; i<256; i++) {
     // Original histogram bins
     hist[i] = ihist_stats->vfe_Ihist_data[i];
     // New total count
     size += hist[i];
  }
  /* compute original CDF, then calculate cap */
  H[0] = hist[0];
  for (i=1; i<=250; i++) {  // Avoid the saturated pixels
     H[i] = H[i-1] + hist[i];
  }

  if (H[250] < la_get_min_pdf_count(la_mod->num_pixels)) {
    CDBG("%s: pdf count %llu", __func__, H[250]);
    free(hist);
    free(H);
    free(threshold);
    return VFE_ERROR_GENERAL;
  }

  for (i=1; i<=250; i++) {  // Avoid the saturated pixels
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
     if ((int)(i * cap_adjust) < 255) {
       tmp0 = (float)curve[(int16_t)(i * cap_adjust)];
       tmp1 = (float)curve[(int16_t)(i * cap_adjust) + 1];
       tmp  = tmp0 + (tmp1 - tmp0) * (i * cap_adjust -
         (int16_t)(i * cap_adjust));  // interpolation
       threshold[i] = (uint32_t)((((uint32_t)(tmp * cap) + 64) * (size >> 8)) >> 6);
     } else {
       threshold[i] = (uint32_t)(size >> 8);  // (64*(size>>8))>>6
     }
  }
  // apply cap to histogram curve
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
       CDBG("iterartion %d,", iter_cnt);
       if ((full_bin+high_bin) < 256)  // alway true
         /* Distribute capped histogram counts to uncapped bins */
         avg_inc = capped_count / (256-full_bin-high_bin);
         iter_cnt++;
        CDBG("full_bin, %d, high_bin, %d, avg_inc, %d\n",
          full_bin, high_bin, avg_inc);
  } while (high_bin > 0 && iter_cnt < 10);

  /* Adjust histogram: Offset, Low light boost, High light boost */
  /* adjusted histogram */
  size = 0;
  for (i=0; i<256; i++) {
     hist[i] += Offset;
     //assert((32 + backlight * 4) <= (256 - 32- (frontlight * 4)));
     if (i < (32+(int)(backlight*4))) {
       // Low light boost
       hist[i] = (uint32_t)((hist[i]) * (1.0f + (float)(32 + backlight * 4 - i) *
         backlight * 0.4f/36.0f));
     }
     if (i > (256-32-(int)(frontlight * 4))) {
       // High light boost
       hist[i] = (uint32_t)((hist[i])*(1.0f + (float)(i - (256 - 32 -
         (frontlight * 4))) * frontlight * 0.6f/32.0f));
     }
     // New total count
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
  return VFE_SUCCESS;
} /* vfe_stats_calc_hist_curve */

/*===========================================================================
 * FUNCTION    - vfe_stats_process_hist -
 *
 * DESCRIPTION:
 * This function process histogram stats from the vfe and
 * updates the Luma Adaptation LUT, this function is called by
 * media controller when stats obtained from kernel
 *==========================================================================*/
vfe_status_t vfe_stats_process_hist(void * ctrl)
{

  uint8_t la_curve[256];
  uint32_t i,j;
  int32_t val, N_pixels,offset;
  float equalize;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  ihist_stats_t *ihist_stats = &(p_obj->vfe_module.stats.ihist_stats);
  la_mod_t *la_mod = &(p_obj->vfe_module.la_mod);

  equalize = ihist_stats->la_config.offset;

#ifdef FEATURE_VFE_TEST_VECTOR
  memcpy(ihist_stats->vfe_Ihist_data, test_hist,
    IHIST_TABLE_LENGTH * sizeof(uint32_t));
#endif
  /*If la flag is disable in chromatix dont update the la table*/
  if(!p_obj->vfe_params.chroma3a->la_8k_enable)
    return VFE_SUCCESS;
  /* Reset N_pixel to match histogram for histogram offset */
  N_pixels = 0;
  for (i=0; i<256; i++)
    N_pixels += ihist_stats->vfe_Ihist_data[i];

  offset = (N_pixels * equalize)/256;

  /* Compute LA Y->Ynew curve */
  if (VFE_SUCCESS != vfe_stats_calc_hist_curve(ihist_stats, la_mod, offset,
    la_curve)) {
    /* since we cannot calculate the new curve, use the old table */
    return VFE_SUCCESS;
  }

  /* Cipher 256-entry Y_ratio curve to 64-entry LUT */
  for (i=0; i<64; i++) {
    val = 0;
    for (j=0; j<4; j++) {
      if (i || j)
        val += ((int)la_curve[(i<<2)+j] << 6) / ((i<<2)+j);  /* Q6 */
    }
    la_mod->LUT_Yratio[i] = (val >> 2);

    if (la_mod->LUT_Yratio[i] > 196)
      la_mod->LUT_Yratio[i] = 196;

    if (la_mod->LUT_Yratio[i] < 48)
      la_mod->LUT_Yratio[i] = 48;
  }

  for (i=0; i<63; i++) {
    val = la_mod->LUT_Yratio[i+1] - la_mod->LUT_Yratio[i];

    if (val > 127) val = 127;
    if (val < -128) val = -128;
    val = val & 0xFF;  /* 8s */
    la_mod->LUT_Yratio[i] = (val<<8) | la_mod->LUT_Yratio[i];  /* 16s */
  }
    /* Fill in the last entry */
    val = la_mod->LUT_Yratio[VFE_LA_TABLE_LENGTH - 1];
    if (val > 127) val = 127;
    if (val < -128) val = -128;
    val = val & 0xFF;  /* 8s */
    la_mod->LUT_Yratio[VFE_LA_TABLE_LENGTH - 1] =
	   (val<<8) | la_mod->LUT_Yratio[VFE_LA_TABLE_LENGTH  - 1];  /* 16s */

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_la_set_bestshot -
 *
 * DESCRIPTION:
 * This function updates the VFE with the backlight/default LA LUT
 *==========================================================================*/
vfe_status_t vfe_la_set_bestshot(int mod_id, void* module, void* vparams,
  camera_bestshot_mode_type mode)
{
  la_mod_t *la_mod = (la_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;
  la_mod->la_trigger = FALSE;
  CDBG("%s\n",__func__);

  uint8_t mod_enable = TRUE;

  switch (mode) {
    case CAMERA_BESTSHOT_BACKLIGHT:
      la_mod->pLUT_Yratio = default_backlight_la_tbl;
      mod_enable = TRUE;
      break;
    case CAMERA_BESTSHOT_SUNSET:
    case CAMERA_BESTSHOT_FLOWERS:
    case CAMERA_BESTSHOT_CANDLELIGHT:
      //Disable Module for these Best Shot Mode
      mod_enable = FALSE;
      break;
    default:
      la_mod->la_trigger = TRUE;
      mod_enable = TRUE;
      la_mod->pLUT_Yratio = la_mod->LUT_Yratio;
      break;
  }
  la_mod->la_update = TRUE;

  if(status != vfe_la_enable(mod_id, la_mod, params, mod_enable, TRUE)) {
    CDBG("%s: LA Enable/Diable Failed", __func__);
  }

  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_la_config -
 *
 * DESCRIPTION:
 * This function configures the VFE with the new LA LUT
 *==========================================================================*/
vfe_status_t vfe_la_config(int mod_id, void *module, void *vparams)
{
  uint32_t i;
  la_mod_t *la_mod = (la_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  int8_t is_snap = IS_SNAP_MODE(vfe_params);

  VFE_LA_ConfigCmdType *la_cmd = (!is_snap) ?
    &la_mod->la_vf_cmd : &la_mod->la_snap_cmd;

  if (!la_mod->la_enable) {
    CDBG("%s: LA not enabled", __func__);
    return VFE_SUCCESS;
  }

  la_mod->num_pixels =  (FLOOR16((vfe_params->output2w)/2)-1)
    * (FLOOR16((vfe_params->output2h)/2)-1);
  CDBG("%s %d\n",__func__, la_mod->num_pixels);

  for (i = 0; i<VFE_LA_TABLE_LENGTH ; i++)
    la_cmd->TblEntry.table[i] = (int16_t)la_mod->pLUT_Yratio[i];

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    la_cmd, sizeof(VFE_LA_ConfigCmdType), VFE_CMD_LA_CFG)) {
    CDBG_HIGH("%s: Module config failed\n", __func__);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_la_update -
 *
 * DESCRIPTION:
 * This function updates the VFE with the new LA LUT
 *==========================================================================*/
vfe_status_t vfe_la_update(int mod_id, void *module, void *vparams)
{
  uint32_t i;
  vfe_status_t status;
  la_mod_t *la_mod = (la_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  int8_t is_snap = IS_SNAP_MODE(vfe_params);

  VFE_LA_ConfigCmdType *la_cmd = (!is_snap) ?
    &la_mod->la_vf_cmd : &la_mod->la_snap_cmd;

  if(la_mod->la_update == FALSE){
    CDBG("%s: Update Disabled, skip update", __func__);
    return VFE_SUCCESS;
  }

  if (la_mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(vfe_params->camfd,
      CMD_GENERAL, vfe_params->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    vfe_params->update |= VFE_MOD_LA;
    la_mod->hw_enable = FALSE;
  }

  if (!la_mod->la_enable) {
    CDBG("%s: LA not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s\n",__func__);

  for (i = 0; i<VFE_LA_TABLE_LENGTH ; i++)
    la_cmd->TblEntry.table[i] = (int16_t)la_mod->pLUT_Yratio[i];

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL,
    la_cmd, sizeof(VFE_LA_ConfigCmdType), VFE_CMD_LA_UPDATE)) {
    CDBG_HIGH("%s: Module update failed\n", __func__);
    return VFE_ERROR_GENERAL;
  }
  la_mod->la_update = FALSE;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_la_prep_spl_effect_lut -
 *
 * DESCRIPTION: Prepare special effects table in Init state.
 *==========================================================================*/
void vfe_la_prep_spl_effect_lut(int *spl_eff_lut, const uint8_t *tbl)
{
  int32_t val, i;
  memset(spl_eff_lut, 0, VFE_LA_TABLE_LENGTH);
  for (i=0; i < 63; i++) {
     val = tbl[i+1] - tbl[i];
     if (val > 127) val = 127;
     if (val < -128) val = -128;
     val = val & 0xFF;  /* 8s */
     spl_eff_lut[i] = (val<<8) | tbl[i];  /* 16s */
  }
     /* Fill in the last entry */
     val = tbl[VFE_LA_TABLE_LENGTH - 1];
     if (val > 127) val = 127;
     if (val < -128) val = -128;
     val = val & 0xFF;  /* 8s */
     spl_eff_lut[VFE_LA_TABLE_LENGTH - 1] =
	   (val<<8) | tbl[VFE_LA_TABLE_LENGTH  - 1];  /* 16s */

}
/*===========================================================================
 * FUNCTION    - vfe_la_init -
 *
 * DESCRIPTION: This function copies the gamma LUT values to the LA look up
 * table, for the init
 *==========================================================================*/
vfe_status_t vfe_la_init(int mod_id, void *module, void *vparams)
{
  uint32_t i;
  la_mod_t *la_mod = (la_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  CDBG("%s\n",__func__);

  for (i = 0; i < VFE_LA_TABLE_LENGTH ;i++) {
    la_mod->LUT_Yratio[i] = 64;
  }
  vfe_la_prep_spl_effect_lut(la_mod->solarize_la_tbl,
    solarize_la);
  vfe_la_prep_spl_effect_lut(la_mod->posterize_la_tbl,
    posterize_la);
  la_mod->pLUT_Yratio = la_mod->LUT_Yratio;
  la_mod->la_trigger = TRUE;
  la_mod->hw_enable = FALSE;
  la_mod->la_update = FALSE;
  return VFE_SUCCESS;
} /* vfe_la_init */

/*===========================================================================
FUNCTION      vfe_la_trigger_update

DESCRIPTION   interpolating the la_config based on aec result.
===========================================================================*/
vfe_status_t vfe_la_trigger_update(int mod_id, void *module, void *vparams)
{
  float ratio = 0.0;
  la_mod_t *la_mod = (la_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  chromatix_parms_type *chromatix = vfe_params->chroma3a;
  int8_t is_snapmode = IS_SNAP_MODE(vfe_params);
  tuning_control_type *tc;
  trigger_point_type  *tp;

  la_8k_type   la_8k_config_indoor, la_8k_config_outdoor;
  la_8k_type la_config_compensated, la_config_backlight;
  vfe_asd_params_t *sd_out = &(vfe_params->asd_params);
  uint32_t backlight_scene_severity = 0;

  if (!la_mod->la_enable) {
    CDBG("%s: LA not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!la_mod->la_trigger) {
    CDBG("%s: LA trigger not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (vfe_params->bs_mode == CAMERA_BESTSHOT_BACKLIGHT)
    backlight_scene_severity = 255; // max severity
  else
    backlight_scene_severity =
      MIN(255, sd_out->backlight_scene_severity);

  CDBG("%s: operation mode = %d", __func__, vfe_params->vfe_op_mode);
  switch (vfe_params->vfe_op_mode) {
    case VFE_OP_MODE_PREVIEW:
    case VFE_OP_MODE_VIDEO:
    case VFE_OP_MODE_ZSL:
      la_8k_config_indoor = chromatix->la_8k_config_indoor_VF;
      la_8k_config_outdoor = chromatix->la_8k_config_outdoor_VF;
      tc = &(chromatix->control_la_VF);
      tp = &(chromatix->la_brightlight_trigger_VF);
      break;
    case VFE_OP_MODE_SNAPSHOT:
    case VFE_OP_MODE_JPEG_SNAPSHOT:
      la_8k_config_indoor= chromatix->la_8k_config;
      la_8k_config_outdoor = chromatix->la_8k_config_outdoor;
      tc = &(chromatix->control_la);
      tp = &(chromatix->la_brightlight_trigger);
      break;
    default:
      CDBG_ERROR("%s, invalid mode = %d",__func__, vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
  }
  ratio = vfe_util_get_aec_ratio(*tc, tp, vfe_params);

  if (ratio > 1.0)
    ratio = 1.0;
  else if (ratio < 0.0)
    ratio = 0.0;

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

  if (backlight_scene_severity != 0) {
    la_config_compensated = la_mod->la_config;
    la_config_backlight =
      chromatix->backlit_scene_detect.backlight_la_8k_config;

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
  }
  la_mod->la_update = TRUE;

  CDBG("%s:\n",__func__);
  CDBG("ratio: %f,la_config.offset: %f\n",ratio, la_mod->la_config.offset);
  CDBG("low beam: %f\n",la_mod->la_config.low_beam);
  CDBG("high beam: %f\n",la_mod->la_config.high_beam);
  CDBG("histogram cap: %f\n",la_mod->la_config.histogram_cap);
  CDBG("cap high: %f\n",la_mod->la_config.cap_high);
  CDBG("cap low: %f\n",la_mod->la_config.cap_low);
  CDBG("backlight_scene_severity :%d \n",sd_out->backlight_scene_severity);

  return VFE_SUCCESS;
} /* vfe_la_trigger_update */

/*===========================================================================
 * Function:           vfe_la_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_la_enable(int mod_id, void *module, void* vparams,
  int8_t enable, int8_t hw_write)
{
  la_mod_t *la_mod = (la_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;
  vfe_params->moduleCfg->lumaAdaptationEnable = enable;

  if (hw_write && (la_mod->la_enable == enable))
    return VFE_SUCCESS;

  la_mod->la_enable = enable;
  la_mod->hw_enable = hw_write;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_LA)
      : (vfe_params->current_config & ~VFE_MOD_LA);
  }
  return VFE_SUCCESS;
} /* vfe_la_enable */

/*===========================================================================
 * Function:           vfe_la_get_table
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_la_get_table(la_mod_t* la_mod, vfe_params_t* vfe_params,
  vfe_pp_params_t *pp_info)
{
  CDBG("%s: enter\n",__func__);
  if (!la_mod->la_enable) {
    CDBG("%s: LA not enabled", __func__);
    pp_info->la_enable = 0;
    pp_info->luma_table = NULL;
    pp_info->luma_num_entries = 0;
    return VFE_SUCCESS;
  }
  if (la_mod->pLUT_Yratio != NULL) {
    pp_info->la_enable = 0;
    pp_info->luma_table = (int16_t *)la_mod->pLUT_Yratio;
    pp_info->luma_num_entries = VFE_LA_TABLE_LENGTH;
  } else {
      CDBG_HIGH("%s: LA table is NULL",__func__);
      return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_la_trigger_enable -
 *
 * DESCRIPTION: This function updates the mce trigger enable flag
 *==========================================================================*/
vfe_status_t vfe_la_trigger_enable(int mod_id, void *module, void *vparams,
  int enable)
{
  la_mod_t *mod = (la_mod_t *)module;
  CDBG("%s:enable :%d\n",__func__, enable);
  mod->la_trigger = enable;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_la_set_spl_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_la_set_spl_effect(int mod_id, void* mod_la, void *vparams,
  vfe_spl_effects_type type)
{
  la_mod_t *mod = (la_mod_t *)mod_la;
  vfe_params_t *parms = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;
  chromatix_parms_type *chromatix_ptr = parms->chroma3a;

  if (parms->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: Best shot enabled, skip seteffect", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s: type %d", __func__, type);
  switch (type) {
    case CAMERA_EFFECT_POSTERIZE:
      mod->pLUT_Yratio = mod->posterize_la_tbl;
      break;
    case CAMERA_EFFECT_SOLARIZE:
      mod->pLUT_Yratio = mod->solarize_la_tbl;
      break;
    default:
      mod->pLUT_Yratio = mod->LUT_Yratio;
      break;
  }
  return status;
} /* vfe_la_set_effect */

/*===========================================================================
 * FUNCTION    - vfe_la_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
vfe_status_t vfe_la_tv_validate(int mod_id,
  void *test_input, void *test_output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;

  uint32_t i, ip, op;
  CDBG("%s, size of LA = %d \n",__func__, mod_in->la.size);
  for(i = 0; i < mod_in->la.size; i++) {
    ip = mod_in->la.table[i] & 0x0000ffff;
    op = mod_in->la.table[i] & 0x0000ffff;
    VALIDATE_TST_LUT(ip, op, 3, "LA", i);
    ip = mod_in->la.table[i] & 0xffff0000;
    op = mod_in->la.table[i] & 0xffff0000;
    VALIDATE_TST_LUT(ip, op, 3, "LA", i);
  }
  return VFE_SUCCESS;
} /* vfe_la_test_vector_validate*/

/*===========================================================================
 * FUNCTION    - vfe_la_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_la_deinit(int mod_id, void *module, void *params)
{
  la_mod_t *la_mod = (la_mod_t *)module;
  memset(la_mod, 0 , sizeof(la_mod_t));
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_la_reload_params -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_la_reload_params(int module_id, void *mod,
  void* vfe_parms)
{
  CDBG_ERROR("%s: Not implemented\n", __func__);
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_la_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_la_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  la_module_t *cmd = (la_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->la_cmd),
     sizeof(VFE_LA_ConfigCmdType),
     VFE_CMD_LA_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_la_plugin_update */
#endif
