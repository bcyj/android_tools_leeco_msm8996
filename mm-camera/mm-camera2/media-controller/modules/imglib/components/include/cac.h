/******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.         *
* Qualcomm Technologies Proprietary and Confidential.                         *
*******************************************************************************/

#ifndef __CAC_H__
#define __CAC_H__

#include "img_common.h"

/** cac_3a_info_t
 *   @awb_gr_gain: Whitebalance g gain
 *   @awb_gb_gain:
 *
 *   3a info for denoise
 **/
typedef struct {
  float awb_gr_gain;
  float awb_gb_gain;
} cac_3a_info_t;

/** cac_chromatix_info_t
 *   @edgeTH: edge detection threshold
 *   @saturatedTH: Y component saturation threshold
 *   @chrom0LowTH: R/G hue low threshold
 *   @chrom0HighTH: R/G hue high threshold
 *   @chrom1LowTH: B/G hue low threshold
 *   @chrom1HighTH: B/G hue low threshold
 *   @chrom0LowDiffTH: R/G hue difference low threshold
 *   @chorm0HighDiffTH: R/G hue difference high threshold
 *   @chrom1LowDiffTH: B/G hue difference low threshold
 *   @chorm1HighDiffTH: B/G hue difference high threshold
 *
 *  CAC Chromatix info
 **/
typedef struct {
  int16_t edgeTH;
  uint8_t saturatedTH;
  int32_t chrom0LowTH;
  int32_t chrom0HighTH;
  int32_t chrom1LowTH;
  int32_t chrom1HighTH;
  int32_t chrom0LowDiffTH;
  int32_t chorm0HighDiffTH;
  int32_t chrom1LowDiffTH;
  int32_t chorm1HighDiffTH;
} cac_chromatix_info_t;

/** cac_v2_chromatix_info_t
 *   @bright_spot_highTH: high threshold
 *   @bright_spot_lowTH: low threshold
 *   @saturation_TH: saturation threshold
 *   @color_Cb_TH: color threshold for cb
 *   @color_Cr_TH: color threshold for cr
 *   @corrRatio_TH: ration threshold
 *
 *  CAC V2 Chromatix info
 **/
typedef struct {
  int32_t detection_th1;
  int32_t detection_th2;
  int32_t detection_th3;
  int32_t verification_th1;
  int32_t correction_strength;
} cac_v2_chromatix_info_t;

/** rnr_chromatix_info_t
 *   @sampling_factor: hRNR downsample/upsample factor
 *   @sigma_lut: Pointer to RNR sigma (threshold) lookup table,
 *               162 length
 *   @lut_size: Size of the sigma_lut
 *   @scale_factor: Size of the sigma_lut
 *   @center_noise_sigma: center ratio
 *   @center_noise_weight: default 1.0
 *   @weight_order: 2.0f if sampling factor=2, 1.5f if sampling
 *                factor=4, 1.0f if sampling factor=8
 *
 *  CAC V2 Chromatix info
 **/
typedef struct {
  uint8_t sampling_factor;
  float sigma_lut[RNR_LUT_SIZE];
  int lut_size;
  float scale_factor;
  float center_noise_sigma;
  float center_noise_weight;
  float weight_order;
} rnr_chromatix_info_t;


/** cac_v2_chromatix_info_t
 *   @CAC_CHROMA_ORDER_CBCR: Order is CBCR
 *   @CAC_CHROMA_ORDER_CRCB: order is CrCb
 *
 * CAC Chroma order
 **/
typedef enum {
  CAC_CHROMA_ORDER_CBCR = 0,
  CAC_CHROMA_ORDER_CRCB
} cac_chroma_order;

/** CAC parameters
 *
 **/
#define QCAC_RGAMMA_TABLE     (QIMG_CAC_PARAM_OFF +  1)
#define QCAC_GGAMMA_TABLE     (QIMG_CAC_PARAM_OFF +  2)
#define QCAC_BGAMMA_TABLE     (QIMG_CAC_PARAM_OFF +  3)
#define QCAC_CHROMATIX_INFO   (QIMG_CAC_PARAM_OFF +  4)
#define QCAC_3A_INFO          (QIMG_CAC_PARAM_OFF +  5)
#define QCAC_CHROMA_ORDER     (QIMG_CAC_PARAM_OFF +  6)
#define QRNR_CHROMATIX_INFO   (QIMG_CAC_PARAM_OFF +  7)
#define QCAC_ENABLED          (QIMG_CAC_PARAM_OFF +  8)
#define QRNR_ENABLED          (QIMG_CAC_PARAM_OFF +  9)


#endif

