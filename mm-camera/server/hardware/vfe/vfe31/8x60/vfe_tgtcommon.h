/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __VFE_TGTCOMMON_H__
#define __VFE_TGTCOMMON_H__

#define VFE_PIXEL_IF_MIPI 3
#define VFE_PIXEL_IF_MIPI_1 4

#define MSM8960V1 0x3030b
#define MSM8960V2 0x30408
#define MSM8660 0x30217
#define MSM8930   0x3040f
/* TODO: need to define MSM8974 VFE version*/
#define MSM8974   0x10000

#define MAX_VFE_WIDTH 4096
#define MAX_VFE_HEIGHT 3072

#define DEMOSAIC_WIDTH_DELTA 12
#define DEMOSAIC_HEIGHT_DELTA 6

/* This defines total number registers in VFE. Each register is 4 bytes long
 * so to get the range, multiply this number with 4. */
#define VFE31_REGISTER_TOTAL 0x0000017E

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
  uint32_t  /* reserved */       :  1;
  uint32_t  pixelIfInputSelect   :  3;  /* disabled, camif, MDDI, or MIPI */
  uint32_t  /* reserved */       :  1;
  /* serialInputDataSize is not needed for 8660 but it is kept for backward
   * compatibility with 7x30. So for 8x60 they are treated as reserved. */
  uint32_t  serialInputDataSize  :  2;
  uint32_t  /* reserved */       :  6;
  uint32_t  inputMuxSelect       :  2;  /* input source for vfe pipeline */
  uint32_t  /* reserved */       :  2;
  uint32_t  vfeClkOffEnable      :  1;  /* should = 0  */
  uint32_t  vfeClkIdleEnable     :  1;  /* should = 0  */
  uint32_t  axiClkOffEnable      :  1;  /* should = 0  */
  uint32_t  axiClkIdleEnable     :  1;  /* should = 0  */
  uint32_t  /* reserved */       :  8;
}__attribute__((packed, aligned(4))) VFE_CFGPacked;

/****packed data structure *************************/
typedef struct VFE_ModuleCfgPacked {
  uint32_t  blackLevelCorrectionEnable  :   1;    /* bit 0  */
  uint32_t  lensRollOffEnable           :   1;    /* bit 1  */
  uint32_t  demuxEnable                 :   1;    /* bit 2  */
  uint32_t  chromaUpsampleEnable        :   1;    /* bit 3  */
  uint32_t  demosaicEnable              :   1;    /* bit 4  */
  uint32_t  statsAeBgEnable             :   1;    /* bit 5  */
  uint32_t  statsAfBfEnable             :   1;    /* bit 6  */
  uint32_t  statsAwbEnable              :   1;    /* bit 7  */
  uint32_t  statsRsEnable               :   1;    /* bit 8  */
  uint32_t  statsCsEnable               :   1;    /* bit 9  */
  uint32_t  cropEnable                  :   1;    /* bit 10 */
  uint32_t  mainScalerEnable            :   1;    /* bit 11 */
  uint32_t  whiteBalanceEnable          :   1;    /* bit 12 */
  uint32_t  colorCorrectionEnable       :   1;    /* bit 13 */
  uint32_t  rgbLUTEnable                :   1;    /* bit 14 */
  uint32_t  statsIhistEnable            :   1;    /* bit 15 */
  uint32_t  lumaAdaptationEnable        :   1;    /* bit 16 */
  uint32_t  chromaEnhanEnable           :   1;    /* bit 17 */
  uint32_t  statsSkinEnable             :   1;    /* bit 18 */
  uint32_t  chromaSuppressionMceEnable  :   1;    /* bit 19 */
  uint32_t  skinEnhancementEnable       :   1;    /* bit 20 */
  uint32_t  asfEnable                   :   1;    /* bit 21 */
  uint32_t  chromaSubsampleEnable       :   1;    /* bit 22 */
  uint32_t  scaler2YEnable              :   1;    /* bit 23 */
  uint32_t  scaler2CbcrEnable           :   1;    /* bit 24 */
  uint32_t  realignmentBufEnable        :   1;    /* bit 25 */
  uint32_t  /* reserved */              :   6;
}__attribute__((packed, aligned(4))) VFE_ModuleCfgPacked;


typedef struct VFE_StatsCfg {
  uint32_t colorConvEnable      : 1;
  uint32_t /* reserved */       : 31;
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
