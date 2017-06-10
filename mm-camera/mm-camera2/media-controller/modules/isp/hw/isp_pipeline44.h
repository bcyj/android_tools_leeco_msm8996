/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_PIPELINE44_H__
#define __ISP_PIPELINE44_H__
#include "chromatix.h"

#define ISP_PIXEL_IF_MIPI 3

#define MAX_ISP_WIDTH 5276
#define MAX_ISP_HEIGHT 4032

#define MAX_DMI_TBL_SIZE 1500 /*MAX DMI READ SIZE x uint32_t*/

#define MAX_Y_SCALING_FACTOR 16
#define MAX_CHROMA_SCALING_FACTOR 32
#define DEMOSAIC_WIDTH_DELTA 0
#define DEMOSAIC_HEIGHT_DELTA 0

#define ISP44_MODULE_CFG 0x00000018
#define ISP44_MODULE_CFG_LEN 4
#define ISP44_CORE_CFG 0x0000001C
#define ISP44_CORE_CFG_LEN 4
#define ISP44_REALIGN_BUF_CFG 0x00000884
#define ISP44_REALIGN_BUF_CFG_LEN 4
#define ISP44_CHROMA_UPSAMPLE_CFG 0x0000057C
#define ISP44_CHROMA_UPSAMPLE_CFG_LEN 4
#define ISP44_STATS_CFG 0x00000888
#define ISP44_STATS_CFG_LEN 4
#define ISP44_CAMIF_CFG 0x000002F8
#define ISP44_CAMIF_CFG_LEN 36

#define ISP44_DMI_CFG_DEFAULT 0x00000100
#define ISP44_DMI_CFG_OFF 0x00000910
#define ISP44_DMI_ADDR 0x00000914
#define ISP44_DMI_DATA_HI 0x00000918
#define ISP44_DMI_DATA_LO 0x0000091c

#define ISP44_DMI_NO_MEM_SELECTED 0
#define BLACK_LUT_RAM_BANK0       0x1
#define BLACK_LUT_RAM_BANK1       0x2
#define ROLLOFF_RAM0_BANK0        0x3
#define ROLLOFF_RAM0_BANK1        0x4
#define STATS_BHIST_RAM0          0x7
#define STATS_BHIST_RAM1          0x8
#define	RGBLUT_RAM_CH0_BANK0      0x9
#define	RGBLUT_RAM_CH0_BANK1      0xa
#define	RGBLUT_RAM_CH1_BANK0      0xb
#define	RGBLUT_RAM_CH1_BANK1      0xc
#define	RGBLUT_RAM_CH2_BANK0      0xd
#define	RGBLUT_RAM_CH2_BANK1      0xe
#define	RGBLUT_CHX_BANK0          0xf
#define	RGBLUT_CHX_BANK1          0x10
#define STATS_IHIST_RAM           0x11
#define LA_LUT_RAM_BANK0          0x12
#define LA_LUT_RAM_BANK1          0x13

#define ISP_RGB_LUT44_BANK_SEL_MASK           0x00000007

typedef struct ISP_CoreCfgPacked {
  uint32_t  inputPixelPattern    :  3;  /* bayor or yuv pixel pattern  */
  uint32_t  /* reserved */       :  13;
  uint32_t  inputMuxSelect       :  2;  /* input source for vfe pipeline */
  uint32_t  /* reserved */       :  2;
  uint32_t  ispClkOffEnable      :  1;  /* should = 0  */
  uint32_t  /* reserved */       :  11;
}__attribute__((packed, aligned(4))) ISP_CoreCfgPacked;

/****packed data structure *************************/
typedef struct ISP_ModuleCfgPacked {
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
}__attribute__((packed, aligned(4))) ISP_ModuleCfgPacked;

/* changed in 8974 */
typedef struct ISP_StatsCfg {
  uint32_t colorConvEnable      : 1;
  uint32_t bayerHistSelect      : 1;
  uint32_t /* reserved */       : 30;
} __attribute__((packed, aligned(4))) ISP_StatsCfg;

typedef struct ISP_ChromaUpsampleCfgPacked {
  uint32_t    yuvInputCositingMode  :  1;   /* bayor or yuv pixel pattern  */
  uint32_t    /* reserved */        :  31;
}__attribute__((packed, aligned(4))) ISP_ChromaUpsampleCfgPacked;

typedef struct ISP_RealignConfigCmdType {
  uint32_t            cbOddLine           : 1;
  uint32_t            crOddLine           : 1;
  uint32_t            cbOddPixel          : 1;
  uint32_t            crOddPixel          : 1;
  uint32_t            hsubEnable          : 1;
  uint32_t            vsubEnable          : 1;
  uint32_t            realignInputSel     : 1;
  uint32_t            /* reserved */      : 25;
} __attribute__((packed, aligned(4))) ISP_RealignConfigCmdType;

typedef struct ISP_OperationConfigCmdType {
  ISP_CoreCfgPacked           ispCfg;            /* ISP44_CORE_CFG */
  ISP_ModuleCfgPacked         moduleCfg;         /* ISP44_MODULE_CFG */
  ISP_RealignConfigCmdType    realignBufCfg;     /* ISP44_REALIGN_BUF_CFG */
  ISP_ChromaUpsampleCfgPacked chromaUpsampleCfg; /* ISP44_CHROMA_UPSAMPLE_CFG*/
  ISP_StatsCfg                ispStatsCfg;       /* ISP44_STATS_CFG */
} ISP_OperationConfigCmdType;

/* ===  CAMIF Config Command === */
typedef enum ISP_CAMIF_SYNC_EDGE {
  /* 0x0 = Active high. */
  ISP_CAMIF_SYNC_EDGE_ActiveHigh,
  /* 0x1 = Active low.  */
  ISP_CAMIF_SYNC_EDGE_ActiveLow
} ISP_CAMIF_SYNC_EDGE;

typedef enum ISP_CAMIF_SYNC_MODE {
  /* 0x00 = APS (active physical synchronization) */
  ISP_CAMIF_SYNC_MODE_APS,
  /* 0x01 = EFS (embedded frame synchronization) */
  ISP_CAMIF_SYNC_MODE_EFS,
  /* 0x10 = ELS (embedded line synchronization)  */
  ISP_CAMIF_SYNC_MODE_ELS,
  /* 0x11 = Illegal                              */
  ISP_CAMIF_SYNC_MODE_ILLEGAL
} ISP_CAMIF_SYNC_MODE;

typedef enum CAMIF_SUBSAMPLE_FRAME_SKIP {
  CAMIF_SUBSAMPLE_FRAME_SKIP_0,
  CAMIF_SUBSAMPLE_FRAME_SKIP_AllFrames,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_2Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_3Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_4Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_5Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_6Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_7Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_8Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_9Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_10Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_11Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_12Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_13Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_14Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_15Frame
} CAMIF_SUBSAMPLE_FRAME_SKIP;

typedef struct Hist_DMI_CfgCmdType {
  uint32_t set_channel;
  uint32_t set_start_addr;
  uint64_t table[256];
  uint32_t reset_channel;
  uint32_t reset_start_addr;
}Hist_DMI_CfgCmdType;
/* End: Data structures to hold HW specific table formats and params */

typedef struct {
  uint32_t set_channel;
  uint32_t set_start_addr;
  uint32_t dmi_tbl[MAX_DMI_TBL_SIZE];
  uint32_t reset_channel;
  uint32_t reset_start_addr;
} isp_hw_dmi_dump_t;

typedef enum ISP_CAMIF_To_ISP_SubsampleEnableType {
  ISP_DISABLE_CAMIF_TO_ISP_SUBSAMPLE,
  ISP_ENABLE_CAMIF_TO_ISP_SUBSAMPLE,
  ISP_LAST_CAMIF_TO_ISP_SUBSAMPLE_ENABLE_ENUM =
    ISP_ENABLE_CAMIF_TO_ISP_SUBSAMPLE, /* Used for count purposes only */
} ISP_CAMIF_To_VFE_SubsampleEnableType;

typedef enum ISP_BusSubsampleEnableType {
  ISP_DISABLE_BUS_SUBSAMPLE,
  ISP_ENABLE_BUS_SUBSAMPLE,
  ISP_LAST_BUS_SUBSAMPLE_ENABLE_ENUM =
    ISP_ENABLE_BUS_SUBSAMPLE,  /* Used for count purposes only */
} ISP_BusSubsampleEnableType;

typedef enum ISP_IRQ_SubsampleEnableType {
  ISP_DISABLE_IRQ_SUBSAMPLE,
  ISP_ENABLE_IRQ_SUBSAMPLE,
  ISP_LAST_IRQ_SUBSAMPLE_ENABLE_ENUM =
    ISP_ENABLE_IRQ_SUBSAMPLE,  /* Used for count purposes only */
} ISP_IRQ_SubsampleEnableType;

typedef enum ISP_CAMIFPixelSkipWrapType {
  ISP_USE_ALL_16_BITS_OF_PIXEL_SKIP_PATTERN,
  ISP_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN,
  ISP_LAST_PIXEL_SKIP_WRAP_ENUM = ISP_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN
} ISP_CAMIFPixelSkipWrapType;

typedef struct {
  ISP_OperationConfigCmdType op_cmd;
}isp_operation_cfg_t;

#endif /* __ISP_PIPELINE44_H__ */
