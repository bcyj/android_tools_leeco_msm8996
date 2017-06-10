/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#ifndef __DENOISE_H__
#define __DENOISE_H__

#include "img_common.h"

/**
 *  MACROS and CONSTANTS
 **/

/** wd_mode_t
 *   WD_MODE_YCBCR_PLANE: denoise for luma and chroma planes
 *   WD_MODE_CBCR_ONLY: denoise for chroma only
 *   WD_MODE_STREAMLINE_YCBCR: denoise for streamlined luma and chroma
 *   WD_MODE_STREAMLINED_CBCR: denoise for streamlined chroma only
 *
 *   wave denoise modes
 **/
typedef enum {
  WD_MODE_YCBCR_PLANE,
  WD_MODE_CBCR_ONLY,
  WD_MODE_STREAMLINE_YCBCR,
  WD_MODE_STREAMLINED_CBCR,
  WD_MODE_MAX
} wd_mode_t;

/** wd_3a_info_t
 *   @wb_g_gain: Whitebalance g gain
 *   @lux_idx: lux index
 *   @aec_real_gain: AEC real gain
 *
 *   3a info for denoise
 **/
typedef struct {
  float wb_g_gain;
  float lux_idx;
  float aec_real_gain;
} wd_3a_info_t;

/** wd_buffers_realloc_info_t
 *   @width: frame width
 *   @height: frame height
 *   @mode: wave denoise mode
 *   @uv_subsampling: uv subsampling factor
 *
 *   reallocation info for denoise
 **/
typedef struct {
  uint32_t width;
  uint32_t height;
  wd_mode_t mode;
  img_subsampling_t uv_subsampling;
} wd_buffers_realloc_info_t;

/** Wavelet denoise parameters
 *   QWD_GAMMA_TABLE: get/set gamma table
 *   QWD_LA_TABLE: get/set LA table
 *   QWD_3A_INFO: get/set 3A information
 *   QWD_LOW_GAMMA_TABLE: get/set lowlight gamma table
 *   QWD_CHROMATIX: set chromatix paramters
 *   QWD_MODE: set wd_mode_t
 *   QWD_BUFFERS_REALLOC: set wd_buffers_realloc_info_t
 *   QWD_SW_WNR_SPEC_CHROMATIX: Sw wnr specific chromatix,
 *     contain tuning specific to sw wnr implementation.
 *
 **/
#define QWD_GAMMA_TABLE           (QIMG_DENOISE_PARAM_OFF +  1)
#define QWD_LA_TABLE              (QIMG_DENOISE_PARAM_OFF +  2)
#define QWD_3A_INFO               (QIMG_DENOISE_PARAM_OFF +  3)
#define QWD_LOW_GAMMA_TABLE       (QIMG_DENOISE_PARAM_OFF +  4)
#define QWD_CHROMATIX             (QIMG_DENOISE_PARAM_OFF +  5)
#define QWD_MODE                  (QIMG_DENOISE_PARAM_OFF +  6)
#define QWD_EARLY_CB              (QIMG_DENOISE_PARAM_OFF +  7)
#define QWD_BUFFERS_REALLOC       (QIMG_DENOISE_PARAM_OFF +  8)
#define QWD_SW_WNR_SPEC_CHROMATIX (QIMG_DENOISE_PARAM_OFF +  9)

#endif //__DENOISE_H__
