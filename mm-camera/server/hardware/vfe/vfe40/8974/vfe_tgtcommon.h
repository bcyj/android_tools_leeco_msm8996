/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __VFE_TGTCOMMON_H__
#define __VFE_TGTCOMMON_H__

#define VFE_PIXEL_IF_MIPI 3

#define MSM8960V1 0x3030b
#define MSM8960V2 0x30408
#define MSM8930   0x3040f
#define MSM8974   0x10000018

#define MAX_VFE_WIDTH 5276
#define MAX_VFE_HEIGHT 4032

#define DEMOSAIC_WIDTH_DELTA 0
#define DEMOSAIC_HEIGHT_DELTA 0

/* This defines total number registers in VFE. Each register is 4 bytes long
 * so to get the range, multiply this number with 4. */
#define VFE32_REGISTER_TOTAL 0x000001CD
#define VFE33_REGISTER_TOTAL 0x000001EE

typedef enum VFE_START_PIXEL_PATTERN {
  VFE_BAYER_RGRGRG,
  VFE_BAYER_GRGRGR,
  VFE_BAYER_BGBGBG,
  VFE_BAYER_GBGBGB,
  VFE_YUV_YCbYCr,
  VFE_YUV_YCrYCb,
  VFE_YUV_CbYCrY,
  VFE_YUV_CrYCbY
} VFE_START_PIXEL_PATTERN;

typedef struct VFE_CFGPacked {
  uint32_t  inputPixelPattern    :  3;  /* bayor or yuv pixel pattern  */
  uint32_t  /* reserved */       :  13;
  uint32_t  inputMuxSelect       :  2;  /* input source for vfe pipeline */
  uint32_t  /* reserved */       :  2;
  uint32_t  vfeClkOffEnable      :  1;  /* should = 0  */
  uint32_t  /* reserved */       :  11;
}__attribute__((packed, aligned(4))) VFE_CFGPacked;

/****packed data structure *************************/
typedef struct VFE_ModuleCfgPacked {
  uint32_t  blackLevelCorrectionEnable  :   1;    /* bit 0  */
  uint32_t  lensRollOffEnable           :   1;    /* bit 1  */
  uint32_t  demuxEnable                 :   1;    /* bit 2  */
  uint32_t  chromaUpsampleEnable        :   1;    /* bit 3  */
  uint32_t  demosaicEnable              :   1;    /* bit 4  */
  uint32_t  statsBeEnable               :   1;    /* bit 5  */
  uint32_t  statsAeBgEnable             :   1;    /* bit 6  */
  uint32_t  statsAfBfEnable             :   1;    /* bit 7  */
  uint32_t  statsAwbEnable              :   1;    /* bit 8  */
  uint32_t  statsRsEnable               :   1;    /* bit 9  */
  uint32_t  statsCsEnable               :   1;    /* bit 10 */
  uint32_t  whiteBalanceEnable          :   1;    /* bit 11 */
  uint32_t  clfEnable                   :   1;    /* bit 12 */
  uint32_t  colorCorrectionEnable       :   1;    /* bit 13 */
  uint32_t  rgbLUTEnable                :   1;    /* bit 14 */
  uint32_t  statsIhistEnable            :   1;    /* bit 15 */
  uint32_t  lumaAdaptationEnable        :   1;    /* bit 16 */
  uint32_t  chromaEnhanEnable           :   1;    /* bit 17 */
  uint32_t  statsSkinBhistEnable        :   1;    /* bit 18 */
  uint32_t  chromaSuppressionMceEnable  :   1;    /* bit 19 */
  uint32_t  skinEnhancementEnable       :   1;    /* bit 20 */
  uint32_t  colorXformEncEnable         :   1;    /* bit 21 */
  uint32_t  colorXformViewEnable        :   1;    /* bit 22 */
  uint32_t  scalerEncEnable             :   1;    /* bit 23 */
  uint32_t  scalerViewEnable            :   1;    /* bit 24 */
  uint32_t  asfEncEnable                :   1;    /* bit 25 */
  uint32_t  asfViewEnable               :   1;    /* bit 26 */
  uint32_t  cropEncEnable               :   1;    /* bit 27 */
  uint32_t  cropViewEnable              :   1;    /* bit 28 */
  uint32_t  realignmentBufEnable        :   1;    /* bit 29 */
  uint32_t  /* reserved */              :   2;
}__attribute__((packed, aligned(4))) VFE_ModuleCfgPacked;

/* changed in 8974 */
typedef struct VFE_StatsCfg {
  uint32_t colorConvEnable      : 1;
  uint32_t bayerHistSelect      : 1;
  uint32_t /* reserved */       : 30;
} __attribute__((packed, aligned(4))) VFE_StatsCfg;

typedef struct VFE_ChromaUpsampleCfgPacked {
  uint32_t    yuvInputCositingMode  :  1;   /* bayor or yuv pixel pattern  */
  uint32_t    /* reserved */        :  31;
}__attribute__((packed, aligned(4))) VFE_ChromaUpsampleCfgPacked;

typedef struct VFE_RealignConfigCmdType {
  uint32_t            cbOddLine           : 1;
  uint32_t            crOddLine           : 1;
  uint32_t            cbOddPixel          : 1;
  uint32_t            crOddPixel          : 1;
  uint32_t            hsubEnable          : 1;
  uint32_t            vsubEnable          : 1;
  uint32_t            realignInputSel     : 1;
  uint32_t            /* reserved */      : 25;
} __attribute__((packed, aligned(4))) VFE_RealignConfigCmdType;

#endif /* __VFE_TGTCOMMON_H__ */
