/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util.h"
#include "sensor_util_bayer.h"
#include "mt9e013_u.h"

#define MT9E013_TUNED_R_OVER_G  0x2FC
#define MT9E013_TUNED_B_OVER_G  0x217

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_MT9E013 "mt9e013"
#define MT9E013_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_MT9E013"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *mt9e013_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
  MT9E013_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  MT9E013_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL, /* HD video recording */
  MT9E013_LOAD_CHROMATIX(video_hfr), /* HFR Video recording */
  MT9E013_LOAD_CHROMATIX(video_hfr), /* HFR Video recording */
  MT9E013_LOAD_CHROMATIX(video_hfr), /* HFR Video recording */
  NULL,
  MT9E013_LOAD_CHROMATIX(ar), /* AR */
  MT9E013_LOAD_CHROMATIX(preview), /* ZSL */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t mt9e013_inputformat[] = {
    CAMIF_BAYER_G_R,/*RES0*/
    CAMIF_BAYER_G_R,/*RES1*/
    CAMIF_BAYER_G_R,/*RES2*/
    CAMIF_BAYER_G_R,/*RES3*/
    CAMIF_BAYER_G_R,/*RES4*/
};

static sensor_crop_parms_t mt9e013_cropinfo[] = {
  {0, 0, 0, 0},/*RES0*/
  {0, 0, 0, 0},/*RES1*/
  {0, 0, 0, 0},/*RES2*/
  {0, 0, 0, 0},/*RES3*/
  {0, 0, 0, 0},/*RES4*/
};

mesh_rolloff_array_type lsc_gmdata =
{
 221,
 {
  103, 187, 214, 235, 257, 276, 297, 312, 324, 329, 329, 318, 302, 278,
  250, 221, 129, 183, 229, 256, 283, 309, 338, 365, 390, 408, 415, 409,
  392, 366, 334, 300, 265, 207, 211, 255, 286, 316, 349, 384, 418, 449,
  470, 478, 470, 447, 412, 372, 332, 291, 229, 231, 281, 315, 352, 391,
  433, 475, 512, 537, 545, 532, 502, 459, 410, 361, 316, 246, 250, 304,
  344, 386, 432, 481, 530, 573, 601, 609, 592, 554, 502, 445, 388, 336,
  262, 263, 323, 367, 414, 465, 521, 576, 622, 650, 658, 638, 594, 536,
  472, 408, 351, 273, 270, 334, 380, 431, 485, 544, 601, 647, 674, 682,
  662, 615, 553, 485, 419, 359, 277, 272, 336, 383, 433, 487, 545, 601,
  646, 672, 678, 657, 610, 549, 483, 418, 358, 276, 264, 328, 373, 421,
  472, 526, 577, 619, 643, 645, 624, 582, 526, 465, 404, 348, 268, 252,
  312, 354, 397, 442, 489, 533, 568, 589, 590, 571, 535, 487, 434, 381,
  330, 256, 231, 289, 327, 365, 403, 442, 477, 505, 521, 522, 507, 479,
  440, 396, 351, 306, 238, 207, 261, 296, 329, 360, 390, 417, 439, 451,
  452, 441, 419, 390, 355, 317, 278, 220, 167, 219, 254, 281, 304, 326,
  343, 357, 365, 364, 357, 345, 326, 303, 273, 242, 184},
  {170, 304, 345, 376, 401, 417, 426, 425, 417, 401, 381, 359, 334, 312,
  281, 250, 152, 294, 366, 409, 448, 481, 509, 526, 531, 523, 501, 467,
  431, 396, 362, 329, 294, 238, 334, 404, 451, 498, 541, 579, 603, 613,
  604, 576, 535, 489, 443, 401, 362, 324, 264, 362, 438, 492, 548, 604,
  653, 687, 702, 694, 662, 611, 553, 497, 444, 396, 353, 287, 385, 467,
  529, 596, 663, 722, 766, 787, 780, 745, 686, 618, 549, 486, 429, 379,
  307, 401, 489, 556, 632, 707, 776, 827, 853, 846, 811, 748, 671, 594,
  523, 457, 400, 322, 409, 501, 572, 653, 732, 805, 861, 886, 881, 848,
  786, 706, 624, 546, 476, 415, 333, 409, 503, 575, 654, 733, 805, 861,
  888, 885, 854, 793, 715, 633, 555, 484, 421, 335, 402, 494, 562, 637,
  711, 779, 829, 857, 857, 828, 771, 699, 622, 548, 479, 418, 332, 385,
  475, 537, 604, 671, 730, 773, 797, 798, 773, 724, 661, 594, 526, 463,
  405, 321, 356, 446, 504, 562, 618, 667, 702, 722, 722, 702, 661, 610,
  552, 494, 438, 384, 304, 323, 409, 465, 515, 560, 600, 629, 645, 645,
  628, 595, 554, 507, 458, 408, 358, 284, 267, 351, 407, 451, 486, 512,
  531, 541, 541, 532, 511, 482, 447, 408, 364, 318, 246},
  {158, 292, 336, 373, 404, 431, 454, 468, 478, 479, 470, 456, 433, 403,
  364, 320, 188, 270, 342, 386, 429, 470, 511, 546, 574, 590, 591, 576,
  548, 513, 473, 428, 378, 298, 304, 372, 419, 467, 518, 570, 616, 653,
  673, 673, 653, 617, 572, 520, 468, 414, 328, 326, 396, 447, 503, 564,
  627, 686, 733, 759, 759, 733, 688, 631, 568, 505, 445, 353, 341, 413,
  468, 531, 601, 675, 745, 802, 834, 835, 804, 750, 682, 609, 535, 468,
  372, 349, 423, 480, 547, 622, 704, 783, 846, 882, 886, 854, 794, 719,
  637, 556, 484, 384, 351, 423, 481, 548, 624, 709, 792, 857, 895, 903,
  873, 812, 733, 648, 565, 490, 388, 343, 415, 470, 534, 607, 690, 772,
  839, 879, 888, 860, 802, 725, 642, 560, 486, 386, 332, 398, 449, 508,
  575, 651, 727, 792, 834, 844, 818, 766, 696, 620, 543, 473, 377, 313,
  375, 421, 472, 531, 598, 664, 722, 761, 772, 752, 709, 650, 583, 515,
  452, 358, 286, 348, 389, 434, 483, 538, 594, 643, 677, 688, 674, 641,
  593, 537, 479, 422, 336, 255, 317, 357, 396, 436, 481, 525, 566, 594,
  605, 596, 571, 534, 489, 439, 387, 311, 213, 273, 317, 352, 382, 415,
  445, 472, 492, 501, 498, 485, 462, 429, 387, 341, 265},
  {107, 193, 220, 243, 259, 269, 273, 273, 271, 264, 256, 246, 231, 212,
  191, 168, 100, 187, 235, 264, 289, 309, 323, 331, 333, 329, 320, 306,
  288, 269, 248, 225, 200, 156, 215, 261, 293, 321, 345, 363, 373, 376,
  371, 359, 342, 321, 297, 272, 246, 218, 174, 233, 283, 318, 351, 380,
  402, 415, 419, 413, 399, 378, 352, 324, 295, 265, 234, 186, 247, 301,
  340, 376, 409, 435, 450, 456, 450, 433, 409, 378, 346, 313, 279, 246,
  196, 255, 313, 354, 394, 429, 458, 476, 481, 473, 455, 428, 395, 359,
  323, 287, 253, 203, 259, 317, 360, 400, 438, 467, 485, 488, 479, 461,
  432, 397, 360, 323, 287, 254, 204, 257, 313, 356, 396, 432, 460, 477,
  479, 469, 450, 420, 385, 349, 313, 279, 247, 201, 248, 302, 343, 380,
  413, 439, 453, 455, 445, 424, 395, 362, 327, 295, 265, 235, 191, 235,
  285, 322, 355, 384, 406, 417, 418, 408, 388, 361, 330, 300, 272, 246,
  218, 176, 214, 264, 296, 325, 349, 367, 376, 376, 366, 348, 324, 297,
  271, 247, 224, 199, 161, 192, 238, 269, 293, 313, 327, 334, 333, 325,
  309, 288, 267, 245, 224, 203, 180, 148, 152, 200, 229, 251, 265, 277,
  280, 281, 273, 261, 248, 231, 215, 198, 177, 156, 124},
};


static uint32_t mt9e013_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_RES_QTR,/*SENSOR_MODE_VIDEO_HD*/
  MSM_SENSOR_RES_2,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_RES_3,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_RES_4,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_ZSL*/
};

static struct msm_camera_csi_params mt9e013_csi_params = {
  .data_format = CSI_10BIT,
  .lane_cnt    = 2,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 0x18,
};

static struct msm_camera_csi_params *mt9e013_csi_params_array[] = {
  &mt9e013_csi_params,/*FULL*/
  &mt9e013_csi_params,/*QTR*/
  &mt9e013_csi_params,/*RES2*/
  &mt9e013_csi_params,/*RES3*/
  &mt9e013_csi_params,/*RES4*/
};

/*===========================================================================
 * FUNCTION    - mt9e013_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t mt9e013_real_to_register_gain(float gain)
{
  uint16_t reg_gain;
  uint16_t cg, asc1;

  if (gain > 15.875)
    gain = 15.875;

  if (gain < 1.984375) {
    gain = gain;
    cg = 0x0;
    asc1 = 0x0;
  } else if (gain < 3.96875) {
    gain = gain/2.0;
    cg = 0x400;
    asc1 = 0x0;
  } else if (gain < 7.9375) {
    gain = gain/4.0;
    cg = 0x600;
    asc1 = 0x0;
  } else {
    gain = gain/8.0;
    cg = 0x600;
    asc1 = 0x80;
  }

  reg_gain = (uint16_t)(gain * 64.0);
  reg_gain |= cg;
  reg_gain |= asc1;
  return reg_gain;
}

/*===========================================================================
 * FUNCTION    - mt9e013_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float mt9e013_register_to_real_gain(uint16_t reg_gain)
{
  float gain;
  float multiplier1 = 0.0;
  float multiplier2 = 0.0;
  if (reg_gain > 0x16FF)
    reg_gain = 0x16FF;

  multiplier1 = (float)((reg_gain & 0x700)>> 8);
    multiplier2 = (float)((reg_gain & 0x80)>> 7);

  if (multiplier1 == 0.0f) multiplier1 = 1.0f;
  else if (multiplier1 == 4.0f) multiplier1 = 2.0f;
  else if (multiplier1 == 6.0f) multiplier1 = 4.0f;

  if (multiplier2 == 0.0f) multiplier2 = 1.0f;
  else if (multiplier2 == 1.0f) multiplier2 = 2.0f;

    gain = (float)(reg_gain&0x7F)/64.0f;

    gain = multiplier1*multiplier2*gain;

  return gain;
}

static float mt9e013_wbgain_calibration (float gain, float calib_factor)
{
  /* gain = 1 / [(1/gain) * calibration_r_over_g_factor] */
  return (gain / calib_factor);
}

static void mt9e013_awb_calibration (void *sctrl)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_2d_cali_data *calib_info = &(ctrl->cali_data_2d);
  float r_gain, b_gain, g_gain, min_gain, gr_over_gb;
  chromatix_parms_type *chromatix = &(ctrl->chromatixData);
  uint32_t indx, light_indx = AGW_AWB_MAX_LIGHT-1;
  float r_over_g_calib_factor = (float)calib_info->rave_over_grave;
  CDBG("mt9e013 calib_info->rave_over_grave = 0x%X \n", calib_info->rave_over_grave);
  CDBG("mt9e013 calib_info->bave_over_gbave = 0x%X \n", calib_info->bave_over_gbave);
  CDBG("mt9e013 calib_info->grave_over_gbave = 0x%X \n", calib_info->grave_over_gbave);

  r_over_g_calib_factor /= MT9E013_TUNED_R_OVER_G;
  float b_over_g_calib_factor = (float)calib_info->bave_over_gbave;
    b_over_g_calib_factor /= MT9E013_TUNED_B_OVER_G;

  /* Calibrate the AWB in chromatix based on color measurement read */
  for (indx = 0; indx < light_indx; indx++) {
    chromatix->awb_reference_hw_rolloff[indx].red_gain *=
      r_over_g_calib_factor;
    chromatix->awb_reference_hw_rolloff[indx].blue_gain *=
      b_over_g_calib_factor;
  }

  /* Calibrate the MWB in chromatix based on color measurement read */
  /* MWB TL84 */
  r_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_tl84_white_balance.r_gain,
    r_over_g_calib_factor);
  b_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_tl84_white_balance.b_gain,
    b_over_g_calib_factor);
  g_gain = chromatix->chromatix_tl84_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_tl84_white_balance.r_gain = r_gain / min_gain;
  chromatix->chromatix_tl84_white_balance.g_gain = g_gain / min_gain;
  chromatix->chromatix_tl84_white_balance.b_gain = b_gain / min_gain;

  /* MWB D50 */
  r_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_d50_white_balance.r_gain,
    r_over_g_calib_factor);
  b_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_d50_white_balance.b_gain,
    b_over_g_calib_factor);
  g_gain = chromatix->chromatix_d50_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_d50_white_balance.r_gain = r_gain / min_gain;
  chromatix->chromatix_d50_white_balance.g_gain = g_gain / min_gain;
  chromatix->chromatix_d50_white_balance.b_gain = b_gain / min_gain;

  /* MWB Incandescent */
  r_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_incandescent_white_balance.r_gain,
    r_over_g_calib_factor);
  b_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_incandescent_white_balance.b_gain,
    b_over_g_calib_factor);
  g_gain = chromatix->chromatix_incandescent_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_incandescent_white_balance.r_gain =
    r_gain / min_gain;
  chromatix->chromatix_incandescent_white_balance.g_gain =
    g_gain / min_gain;
  chromatix->chromatix_incandescent_white_balance.b_gain =
    b_gain / min_gain;

  /* MWB D65 */
  r_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_d65_white_balance.r_gain, r_over_g_calib_factor);
  b_gain = mt9e013_wbgain_calibration (
    chromatix->chromatix_d65_white_balance.b_gain, b_over_g_calib_factor);
  g_gain = chromatix->chromatix_d65_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_d65_white_balance.r_gain = r_gain / min_gain;
  chromatix->chromatix_d65_white_balance.g_gain = g_gain / min_gain;
  chromatix->chromatix_d65_white_balance.b_gain = b_gain / min_gain;

  /* MWB Strobe */
  r_gain = mt9e013_wbgain_calibration (
    chromatix->strobe_flash_white_balance.r_gain, r_over_g_calib_factor);
  b_gain = mt9e013_wbgain_calibration (
    chromatix->strobe_flash_white_balance.b_gain, b_over_g_calib_factor);
  g_gain = chromatix->strobe_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->strobe_flash_white_balance.r_gain = r_gain / min_gain;
  chromatix->strobe_flash_white_balance.g_gain = g_gain / min_gain;
  chromatix->strobe_flash_white_balance.b_gain = b_gain / min_gain;

  /* MWB LED flash */
  r_gain = mt9e013_wbgain_calibration (
    chromatix->led_flash_white_balance.r_gain, r_over_g_calib_factor);
  b_gain = mt9e013_wbgain_calibration (
    chromatix->led_flash_white_balance.b_gain, b_over_g_calib_factor);
  g_gain = chromatix->led_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->led_flash_white_balance.r_gain = r_gain / min_gain;
  chromatix->led_flash_white_balance.g_gain = g_gain / min_gain;
  chromatix->led_flash_white_balance.b_gain = b_gain / min_gain;

  /* Gr/Gb channel balance */
  gr_over_gb = (float)calib_info->grave_over_gbave / 1000.0;
  /* To compensate invert the measured gains */
  b_gain = gr_over_gb;
  r_gain = 1;
  CDBG("%s: gb_gain: %f, gr_gain: %f\n", __func__, b_gain, r_gain);
  min_gain = MIN(r_gain, b_gain);
  CDBG("%s: min_gain: %f\n", __func__, min_gain);
  chromatix->chromatix_channel_balance_gains.green_even = r_gain/min_gain;
  chromatix->chromatix_channel_balance_gains.green_odd = b_gain/min_gain;
  return;
}

const int ROLLOFF_MESH_NUM_ROWS = 13;
const int ROLLOFF_MESH_NUM_COLS = 17;

static void mt9e013_lsc_calibration (void *sctrl)
{
  int i, j, index;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_2d_cali_data *calib_info = &(ctrl->cali_data_2d);
  chromatix_parms_type *chromatix = &(ctrl->chromatixData);
  chromatix_4_channel_black_level *chroma_blcklvl =
    &(chromatix->normal_light_4_channel);
  mesh_rolloff_array_type lsc_claibdata;
  uint16_t temp_gain;

  for (i = 0; i < 221; i++) {
  if (i<150)
    temp_gain = ((calib_info->lsc_data[(i*5)+4] & 0xAA) >> 1) |
     ((calib_info->lsc_data[(i*5)+4] & 0x55) << 1);
   else
    temp_gain = calib_info->lsc_data[(i*5)+4];
  lsc_claibdata.r_gain[i] =
    ((uint16_t)calib_info->lsc_data[i*5] | (temp_gain &0xC0) << 2);
  lsc_claibdata.gr_gain[i] =
    ((uint16_t)calib_info->lsc_data[(i*5)+1] | (temp_gain &0x30) << 4);
  lsc_claibdata.gb_gain[i] =
    ((uint16_t)calib_info->lsc_data[(i*5)+2] | (temp_gain &0x0C) << 6);
  lsc_claibdata.b_gain[i] =
    ((uint16_t)calib_info->lsc_data[(i*5)+3] | (temp_gain &0x03) << 8);
  }
  CDBG("%s R Gain   Gr Gain   Gb Gain B Gain \n", __func__);
  for (i = 0; i < ROLLOFF_MESH_NUM_ROWS * ROLLOFF_MESH_NUM_COLS; i++)
    CDBG("calibdata[%d],    %d    %d    %d    %d\n", i,
    (int32_t)lsc_claibdata.r_gain[i] ,
    (int32_t)lsc_claibdata.gr_gain[i],
    (int32_t)lsc_claibdata.gb_gain[i],
    (int32_t)lsc_claibdata.b_gain[i]);

    j = (ROLLOFF_MESH_NUM_COLS * ROLLOFF_MESH_NUM_ROWS) - (ROLLOFF_MESH_NUM_COLS);
    for (i = 0; i < ROLLOFF_MESH_NUM_COLS; i++) {
      index = i;
      if (i==0)
        index = i + 1;
      if (i==16)
        index = i - 1;
      /*first row*/
      lsc_claibdata.r_gain[i] = 2 * lsc_claibdata.r_gain[i] -
        (lsc_claibdata.r_gain[index + ROLLOFF_MESH_NUM_COLS]);
      /*last  row*/
      lsc_claibdata.r_gain[j + i] = 2 * lsc_claibdata.r_gain[j + i] -
        (lsc_claibdata.r_gain[j + index - ROLLOFF_MESH_NUM_COLS]);
      /*first row*/
      lsc_claibdata.b_gain[i] = 2 * lsc_claibdata.b_gain[i] -
        (lsc_claibdata.b_gain[index + ROLLOFF_MESH_NUM_COLS]);
      /*last  row*/
      lsc_claibdata.b_gain[j + i] = 2 * lsc_claibdata.b_gain[j + i] -
        (lsc_claibdata.b_gain[j + index - ROLLOFF_MESH_NUM_COLS]);

      /*first row*/
      lsc_claibdata.gb_gain[i] = 2 * lsc_claibdata.gb_gain[i] -
        (lsc_claibdata.gb_gain[index + ROLLOFF_MESH_NUM_COLS]);
      /*last  row*/
      lsc_claibdata.gb_gain[j + i] = 2 * lsc_claibdata.gb_gain[j + i] -
        (lsc_claibdata.gb_gain[j + index - ROLLOFF_MESH_NUM_COLS]);

      /*first row*/
      lsc_claibdata.gr_gain[i] = 2 * lsc_claibdata.gr_gain[i] -
        (lsc_claibdata.gr_gain[index + ROLLOFF_MESH_NUM_COLS]);
      /*last  row*/
      lsc_claibdata.gr_gain[j + i] = 2 * lsc_claibdata.gr_gain[j + i] -
        (lsc_claibdata.gr_gain[j + index - ROLLOFF_MESH_NUM_COLS]);

  }


    for (i = 1; i < ROLLOFF_MESH_NUM_ROWS-1; i++) {
      lsc_claibdata.r_gain[i * ROLLOFF_MESH_NUM_COLS] =
        2 * lsc_claibdata.r_gain[i * ROLLOFF_MESH_NUM_COLS] -
          (lsc_claibdata.r_gain[(i * ROLLOFF_MESH_NUM_COLS) + 1]);

      lsc_claibdata.r_gain[i * ROLLOFF_MESH_NUM_COLS +16] =
        2 * lsc_claibdata.r_gain[i * ROLLOFF_MESH_NUM_COLS + 16] -
          (lsc_claibdata.r_gain[(i * ROLLOFF_MESH_NUM_COLS) + 15]);

      lsc_claibdata.gr_gain[i * ROLLOFF_MESH_NUM_COLS] =
        2 * lsc_claibdata.gr_gain[i * ROLLOFF_MESH_NUM_COLS] -
          (lsc_claibdata.gr_gain[(i * ROLLOFF_MESH_NUM_COLS) + 1]);

      lsc_claibdata.gr_gain[i * ROLLOFF_MESH_NUM_COLS +16] =
        2 * lsc_claibdata.gr_gain[i * ROLLOFF_MESH_NUM_COLS + 16] -
          (lsc_claibdata.gr_gain[(i * ROLLOFF_MESH_NUM_COLS) + 15]);

      lsc_claibdata.gb_gain[i * ROLLOFF_MESH_NUM_COLS] =
        2 * lsc_claibdata.gb_gain[i * ROLLOFF_MESH_NUM_COLS] -
          (lsc_claibdata.gb_gain[(i * ROLLOFF_MESH_NUM_COLS) + 1]);

      lsc_claibdata.gb_gain[i * ROLLOFF_MESH_NUM_COLS +16] =
        2 * lsc_claibdata.gb_gain[i * ROLLOFF_MESH_NUM_COLS + 16] -
          (lsc_claibdata.gb_gain[(i * ROLLOFF_MESH_NUM_COLS) + 15]);

      lsc_claibdata.b_gain[i * ROLLOFF_MESH_NUM_COLS] =
        2 * lsc_claibdata.b_gain[i * ROLLOFF_MESH_NUM_COLS] -
          (lsc_claibdata.b_gain[(i * ROLLOFF_MESH_NUM_COLS) + 1]);

      lsc_claibdata.b_gain[i * ROLLOFF_MESH_NUM_COLS +16] =
        2 * lsc_claibdata.b_gain[i * ROLLOFF_MESH_NUM_COLS + 16] -
          (lsc_claibdata.b_gain[(i * ROLLOFF_MESH_NUM_COLS) + 15]);

    }

  CDBG("%s R Gain   Gr Gain   Gb Gain B Gain \n", __func__);
  for (i = 0; i < ROLLOFF_MESH_NUM_ROWS * ROLLOFF_MESH_NUM_COLS; i++)
    CDBG("calibdata[%d],    %d    %d    %d    %d\n", i,
    (int32_t)lsc_claibdata.r_gain[i],
    (int32_t)lsc_claibdata.gr_gain[i],
    (int32_t)lsc_claibdata.gb_gain[i],
    (int32_t)lsc_claibdata.b_gain[i]);

  CDBG("%s Chromatix R Gain   Gr Gain   Gb Gain B Gain \n", __func__);
  for (i = 0; i < ROLLOFF_MESH_NUM_ROWS * ROLLOFF_MESH_NUM_COLS; i++)
    CDBG("calibdata[%d],    %f    %f    %f    %f\n", i,
    chromatix->chromatix_mesh_rolloff_table[0].r_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].gr_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].gb_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].b_gain[i]);

  for (j = 0; j < ROLLOFF_MAX_LIGHT; j++) {
    for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
     chromatix->chromatix_mesh_rolloff_table[j].r_gain[i] =
        ((float)(lsc_claibdata.r_gain[i] - (chroma_blcklvl->black_even_row_odd_col>>2))/
        (float)(lsc_gmdata.r_gain[i]-(chroma_blcklvl->black_even_row_odd_col>>2)))
         * chromatix->chromatix_mesh_rolloff_table[j].r_gain[i];

     chromatix->chromatix_mesh_rolloff_table[j].gr_gain[i] =
        ((float)(lsc_claibdata.gr_gain[i] - (chroma_blcklvl->black_even_row_even_col>>2))/
        (float)(lsc_gmdata.gr_gain[i]-(chroma_blcklvl->black_even_row_even_col>>2)))
         * chromatix->chromatix_mesh_rolloff_table[j].gr_gain[i];

     chromatix->chromatix_mesh_rolloff_table[j].gb_gain[i] =
        ((float)(lsc_claibdata.gb_gain[i] - (chroma_blcklvl->black_odd_row_even_col>>2))/
        (float)(lsc_gmdata.gb_gain[i]-(chroma_blcklvl->black_odd_row_even_col>>2)))
         * chromatix->chromatix_mesh_rolloff_table[j].gb_gain[i];

     chromatix->chromatix_mesh_rolloff_table[j].b_gain[i] =
        ((float)(lsc_claibdata.b_gain[i] - (chroma_blcklvl->black_odd_row_odd_col>>2))/
        (float)(lsc_gmdata.b_gain[i]-(chroma_blcklvl->black_odd_row_odd_col>>2)))
         * chromatix->chromatix_mesh_rolloff_table[j].b_gain[i];
    }
  }
  CDBG("%s Chromatix After R Gain   Gr Gain   Gb Gain B Gain \n", __func__);
  for (i = 0; i < ROLLOFF_MESH_NUM_ROWS * ROLLOFF_MESH_NUM_COLS; i++)
    CDBG("calibdata[%d],    %f    %f    %f    %f\n", i,
    chromatix->chromatix_mesh_rolloff_table[0].r_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].gr_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].gb_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].b_gain[i]);
}
static void mt9e013_sensor_calibration (void *sctrl)
{
  mt9e013_awb_calibration(sctrl);
  mt9e013_lsc_calibration(sctrl);
}

static sensor_function_table_t mt9e013_func_tbl = {
    .sensor_set_op_mode = sensor_util_set_op_mode,
    .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
    .sensor_get_dim_info = sensor_util_get_dim_info,

    .sensor_set_frame_rate = sensor_util_set_frame_rate,
    .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

    .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
    .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
    .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
    .sensor_register_to_real_gain = mt9e013_register_to_real_gain,
    .sensor_real_to_register_gain = mt9e013_real_to_register_gain,
    .sensor_get_max_supported_hfr_mode = sensor_util_get_max_supported_hfr_mode,
    .sensor_get_cur_fps = sensor_util_get_cur_fps,
    .sensor_get_lens_info = sensor_get_lens_info,
    .sensor_set_start_stream = sensor_util_set_start_stream,
    .sensor_set_stop_stream = sensor_util_set_stop_stream,
    .sensor_get_csi_params = sensor_util_get_csi_params,
    .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
    .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
    .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
    .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

int8_t mt9e013_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &mt9e013_func_tbl;
  sctrl->sensor.inputformat = mt9e013_inputformat;
  sctrl->sensor.crop_info = mt9e013_cropinfo;
  sctrl->sensor.mode_res = mt9e013_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &mt9e013_csi_params_array[0];

  sensor_util_get_output_info(sctrl);
  //sensor_util_get_eeprom_data(sctrl);
  //mt9e013_sensor_calibration(sctrl);

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_BAYER;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_10_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.max_gain = 8.0;
  sctrl->sensor.out_data.aec_info.max_linecount =
    sctrl->sensor.output_info[sctrl->sensor.
    mode_res[SENSOR_MODE_PREVIEW]].frame_length_lines * 24;
  sctrl->sensor.snapshot_exp_wait_frames = 1;

  sctrl->sensor.out_data.lens_info.focal_length = 4.6;
  sctrl->sensor.out_data.lens_info.pix_size = 1.4;
  sctrl->sensor.out_data.lens_info.f_number = 2.65;
  sctrl->sensor.out_data.lens_info.total_f_dist = 1.97;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 54.8;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 42.5;

  sensor_util_config(sctrl);
  return TRUE;
}
