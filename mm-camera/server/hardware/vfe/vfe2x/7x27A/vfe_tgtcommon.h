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
/* TODO: need to define MSM8974 VFE version*/
#define MSM8974   0x10000

#define MAX_VFE_WIDTH 4096
#define MAX_VFE_HEIGHT 3072

#define DEMOSAIC_WIDTH_DELTA 0
#define DEMOSAIC_HEIGHT_DELTA 0

typedef enum {
  VFE_INPUT_SOURCE_CAMIF,
  VFE_INPUT_SOURCE_AXI,
  VFE_LAST_INPUT_SOURCE_ENUM = VFE_INPUT_SOURCE_AXI,  /* For count only */
} VFE_InputSourceType;

typedef enum {
  VFE_DISABLE_BLACK_LEVEL_CORRECTION,
  VFE_ENABLE_BLACK_LEVEL_CORRECTION,
  VFE_LAST_ENABLE_BLACK_LEVEL_CORRECTION_ENUM = VFE_ENABLE_BLACK_LEVEL_CORRECTION,  /* Used for count purposes only */
} VFE_BlackLevelCorrectionEnableType;

typedef enum {
  VFE_DISABLE_LENS_ROLL_OFF_CORRECTION,
  VFE_ENABLE_LENS_ROLL_OFF_CORRECTION,
  VFE_LAST_LENS_ROLL_OFF_CORRECTION_ENABLE_ENUM = VFE_ENABLE_LENS_ROLL_OFF_CORRECTION,  /* Used for count purposes only */
} VFE_LensRollOffCorrectionEnableType;

typedef enum {
  VFE_DISABLE_WHITE_BALANCE,
  VFE_ENABLE_WHITE_BALANCE,
  VFE_LAST_WHITE_BALANCE_ENABLE_ENUM = VFE_ENABLE_WHITE_BALANCE,  /* Used for count purposes only */
} VFE_WhiteBalanceEnableType;

typedef enum {
  VFE_DISABLE_RGB_GAMMA,
  VFE_ENABLE_RGB_GAMMA,
  VFE_LAST_RGB_GAMMA_ENABLE_ENUM = VFE_ENABLE_RGB_GAMMA,  /* Used for count purposes only */
} VFE_RGB_GammaEnableType;

typedef enum {
  VFE_DISABLE_LUMA_NOISE_REDUCTION_PATH,
  VFE_ENABLE_LUMA_NOISE_REDUCTION_PATH,
  VFE_LAST_LUMA_NOISE_REDUCTION_PATH_ENABLE_ENUM = VFE_ENABLE_LUMA_NOISE_REDUCTION_PATH,  /* Used for count purposes only */
} VFE_LumaNoiseReductionPathEnableType;

typedef enum {
  VFE_DISABLE_ADAPTIVE_SPATIAL_FILTER,
  VFE_ENABLE_ADAPTIVE_SPATIAL_FILTER,
  VFE_LAST_ADAPTIVE_SPATIAL_FILTER_ENABLE_ENUM = VFE_ENABLE_ADAPTIVE_SPATIAL_FILTER,  /* Used for count purposes only */
} VFE_AdaptiveSpatialFilterEnableType;

typedef enum {
  VFE_DISABLE_CHROMA_SUBSAMPLE,
  VFE_ENABLE_CHROMA_SUBSAMPLE,
  VFE_LAST_CHROMA_SUBSAMPLE_ENABLE_ENUM = VFE_ENABLE_CHROMA_SUBSAMPLE,  /* Used for count purposes only */
} VFE_ChromaSubsampleEnableType;

typedef struct VFE_ModuleCfgPacked {
  uint32_t  blackLevelCorrectionEnable  :   1;    /* bit 0  */
  uint32_t  lensRollOffEnable           :   1;    /* bit 1  */
  uint32_t  demuxEnable                 :   1;    /* bit 2  */
  //uint32_t  chromaUpsampleEnable        :   1;    /* bit 3  */
  uint32_t  demosaicEnable              :   1;    /* bit 4  */
  //uint32_t  statsAeBgEnable             :   1;    /* bit 5  */
  //uint32_t  statsAfBfEnable             :   1;    /* bit 6  */
  //uint32_t  statsAwbEnable              :   1;    /* bit 7  */
  //uint32_t  statsRsEnable               :   1;    /* bit 8  */
  //uint32_t  statsCsEnable               :   1;    /* bit 9  */
  uint32_t  cropEnable                  :   1;    /* bit 10 */
  uint32_t  mainScalerEnable            :   1;    /* bit 11 */
  uint32_t  whiteBalanceEnable          :   1;    /* bit 12 */
  //uint32_t  clfEnable                   :   1;    /* bit 13 */
  uint32_t  colorCorrectionEnable       :   1;    /* bit 14 */
  uint32_t  rgbLUTEnable                :   1;    /* bit 15 */
  uint32_t  statsIhistEnable            :   1;    /* bit 16 */
  //uint32_t  lumaAdaptationEnable        :   1;    /* bit 17 */
  uint32_t  chromaEnhanEnable           :   1;    /* bit 18 */
  //uint32_t  statsSkinBhistEnable        :   1;    /* bit 19 */
  //uint32_t  chromaSuppressionMceEnable  :   1;    /* bit 20 */
  //uint32_t  skinEnhancementEnable       :   1;    /* bit 21 */
  uint32_t  asfEnable                   :   1;    /* bit 22 */
  uint32_t  chromaSubsampleEnable       :   1;    /* bit 23 */
  //uint32_t  scaler2YEnable              :   1;    /* bit 24 */
  //Euint32_t  scaler2CbcrEnable           :   1;    /* bit 25 */
  //uint32_t  realignmentBufEnable        :   1;    /* bit 26 */
  uint32_t  /* reserved */              :   5;
}__attribute__((packed, aligned(4))) VFE_ModuleCfgPacked;

typedef struct {
  VFE_InputSourceType inputSource:1;
  uint32_t modeOfOperation:1;
  uint32_t snapshotNumber:4;
  uint32_t /* reserved */ :26;
  /* Image Pipeline Modules */
  VFE_BlackLevelCorrectionEnableType blackLevelCorrectionEnable:1;
  VFE_LensRollOffCorrectionEnableType lensRollOffCorrectionEnable:1;
  VFE_WhiteBalanceEnableType whiteBalanceEnable:1;
  VFE_RGB_GammaEnableType RGB_GammaEnable:1;
  VFE_LumaNoiseReductionPathEnableType lumaNoiseReductionPathEnable:1;
  VFE_AdaptiveSpatialFilterEnableType adaptiveSpatialFilterEnable:1;
  VFE_ChromaSubsampleEnableType chromaSubsampleEnable:1;
  uint32_t /* reserved */ :25;

  /* The dimension fed to the statistics module */
  uint32_t lastPixel:12;
  uint32_t /* reserved */ :4;
  uint32_t lastLine:12;
  uint32_t /* reserved */ :4;
} __attribute__ ((packed, aligned(4))) VFE_StartConfigCmdType;

typedef struct {
  uint32_t firstPixel;
  uint32_t lastPixel;
  uint32_t firstLine;
  uint32_t lastLine;
}vfe_active_region_t;

#endif /* __VFE_TGTCOMMON_H__ */
