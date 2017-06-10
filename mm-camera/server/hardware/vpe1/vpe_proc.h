/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef vpe_proc_h
#define vpe_proc_h

#include "vpe_api.h"

#define MM_VPE_SCALE_COEFF_MASK     0x3ff
typedef enum {
  MM_VPE_SCALE_TABLE_0 = 0,
  MM_VPE_SCALE_TABLE_1,
  MM_VPE_SCALE_TABLE_2,
  MM_VPE_SCALE_TABLE_3,
}MM_VPE_SCALE_TABLE_ENUM;

typedef enum {
  MM_VPE_SCALE_0P2_TO_0P4_INDEX = 0,
  MM_VPE_SCALE_0P4_TO_0P6_INDEX = 32,
  MM_VPE_SCALE_0P6_TO_0P8_INDEX = 64,
  MM_VPE_SCALE_0P8_TO_8P0_INDEX = 96,
}MM_VPE_SCALE_INDEX_ENUM;

/* -----------------------------------------------------------------------
** PPP Rotation & Mirror (flip) select options
** ----------------------------------------------------------------------- */
#define MM_VPE_ROT_TYPE             4
#define MM_VPE_MIRROR_TYPE          4

/*
 * VPE_colorFormatType
 *
 * Define the whether the color format is RGB or YCbCr.
 */
typedef enum {
  MM_VPE_BASE_COLOR_FORMAT_NONE,
  MM_VPE_BASE_COLOR_FORMAT_YUV
} mm_vpe_base_color_format_enum;

/*
 * vpe_color_packing_enum
 *
 *
 */
typedef enum {
  MM_VPE_COLOR_PACKING_INTERLEAVED,
  MM_VPE_COLOR_PACKING_PSUEDO_PLANAR
} mm_vpe_color_packing_enum;

/*
 * vpe_chroma_sampling_enum
 */
typedef enum {
  MM_VPE_CHROMA_SAMPLING_YCbCr,
  MM_VPE_CHROMA_SAMPLING_H2V1,
  MM_VPE_CHROMA_SAMPLING_H1V2,
  MM_VPE_CHROMA_SAMPLING_420,
} mm_vpe_chroma_sampling_enum;

/*
 * VPE_ColorInfoType
 *
 * Hardware specific color information.
 */
typedef struct {
  mm_vpe_base_color_format_enum    base_color_format;
  mm_vpe_color_packing_enum        color_packing;
  mm_vpe_chroma_sampling_enum      chroma_tile;
  uint8_t                       ae_pack_pattern[4];
  struct {
    int8_t                      iY_Green;
    int8_t                      iCb_Blue;
    int8_t                      iCr_Red;
    int8_t                      iAlpha;
  } sBitsPerColor;
  uint32_t                      iInterleavedBytesPerPixel;
  uint32_t                      iInterlacedColorElements;
  uint8_t                       bEnableAlpha;
  uint8_t                       bIsTightPacked;
  uint8_t                       bIsMsbAligned;
  uint8_t                       bIsWmvColor;
  uint8_t                       bIsChromaOffsite;
} MM_VPE_ColorInfoType;

/* void vpe_get_colorInfo(COLOR_FORMAT_ENUM color_format,
  VPE_ColorInfoType *pcolor_info); */

#endif /* vpe_rroc_h */
