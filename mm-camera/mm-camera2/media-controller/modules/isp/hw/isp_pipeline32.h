/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_PIPELINE32_H__
#define __ISP_PIPELINE32_H__
#include "chromatix.h"

#define MAX_ISP_WIDTH 5276
#define MAX_ISP_HEIGHT 4032

#define ISP32_MODULE_CFG 0x00000010
#define ISP32_MODULE_CFG_LEN 4
#define ISP32_CORE_CFG 0x00000014
#define ISP32_CORE_CFG_LEN 4
#define ISP32_REALIGN_BUF_CFG 0x0000052C
#define ISP32_REALIGN_BUF_CFG_LEN 4
#define ISP32_CHROMA_UPSAMPLE_CFG 0x0000035C
#define ISP32_CHROMA_UPSAMPLE_CFG_LEN 4
#define ISP32_STATS_CFG 0x00000530
#define ISP32_STATS_CFG_LEN 4

/* DMI offset and config */
#define ISP32_DMI_CFG_DEFAULT 0x00000100
#define ISP32_DMI_CFG_OFF 0x00000598
#define ISP32_DMI_ADDR 0x0000059C
#define ISP32_DMI_DATA_HI 0x000005A0
#define ISP32_DMI_DATA_LO 0x000005A4

#define ISP32_DMI_NO_MEM_SELECTED 0

#define BLACK_LUT_RAM_BANK0       0x1
#define BLACK_LUT_RAM_BANK1       0x2

#define STATS_BHIST_RAM0          0x6
#define STATS_BHIST_RAM1          0x7

#define ROLLOFF_RAM0_BANK0        0x3
#define ROLLOFF_RAM1_BANK0        0x13
#define ROLLOFF_RAM0_BANK1        0x14
#define ROLLOFF_RAM1_BANK1        0x15

#define RGBLUT_RAM_CH0_BANK0      0x8
#define RGBLUT_RAM_CH0_BANK1      0x9
#define RGBLUT_RAM_CH1_BANK0      0xA
#define RGBLUT_RAM_CH1_BANK1      0xB
#define RGBLUT_RAM_CH2_BANK0      0xC
#define RGBLUT_RAM_CH2_BANK1      0xD
#define RGBLUT_CHX_BANK0          0xE
#define RGBLUT_CHX_BANK1          0xF
#define STATS_IHIST_RAM           0x10
#define LA_LUT_RAM_BANK0          0x11
#define LA_LUT_RAM_BANK1          0x12


typedef struct ISP_CoreCfgPacked {
  uint32_t  inputPixelPattern    :  3;  /* bayor or yuv pixel pattern  */
  uint32_t  /* reserved */       :  13;
  uint32_t  inputMuxSelect       :  2;  /* input source for vfe pipeline */
  uint32_t  /* reserved */       :  2;
  uint32_t  ispClkOffEnable      :  1;  /* should = 0 */
  uint32_t  ispClkIdleEnable     :  1;  /* should = 0 */
  uint32_t  axiClkOffEnable      :  1;  /* should = 0 */
  uint32_t  axiClkIdleEnable     :  1;  /* should = 0 */
  uint32_t  /* reserved */       :  8;
}__attribute__((packed, aligned(4))) ISP_CoreCfgPacked;

typedef struct ISP_ModuleCfgPacked {
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
  uint32_t  clfEnable                   :   1;    /* bit 13 */
  uint32_t  colorCorrectionEnable       :   1;    /* bit 14 */
  uint32_t  rgbLUTEnable                :   1;    /* bit 15 */
  uint32_t  statsIhistEnable            :   1;    /* bit 16 */
  uint32_t  lumaAdaptationEnable        :   1;    /* bit 17 */
  uint32_t  chromaEnhanEnable           :   1;    /* bit 18 */
  uint32_t  statsSkinBhistEnable        :   1;    /* bit 19 */
  uint32_t  chromaSuppressionMceEnable  :   1;    /* bit 20 */
  uint32_t  skinEnhancementEnable       :   1;    /* bit 21 */
  uint32_t  asfEnable                   :   1;    /* bit 22 */
  uint32_t  chromaSubsampleEnable       :   1;    /* bit 23 */
  uint32_t  scaler2YEnable              :   1;    /* bit 24 */
  uint32_t  scaler2CbcrEnable           :   1;    /* bit 25 */
  uint32_t  realignmentBufEnable        :   1;    /* bit 26 */
  uint32_t  /* reserved */              :   5;
}__attribute__((packed, aligned(4))) ISP_ModuleCfgPacked;

typedef struct ISP_StatsCfg {
  uint32_t colorConvEnable      : 1;
  uint32_t bayerGridSelect      : 1;
  uint32_t bayerFocusSelect     : 1;
  uint32_t bayerHistSelect      : 1;
  uint32_t /* reserved */       : 28;
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
  ISP_CoreCfgPacked           ispCfg;            /* ISP32_CORE_CFG */
  ISP_ModuleCfgPacked         moduleCfg;         /* ISP32_MODULE_CFG */
  ISP_RealignConfigCmdType    realignBufCfg;     /* ISP32_REALIGN_BUF_CFG */
  ISP_ChromaUpsampleCfgPacked chromaUpsampleCfg; /* ISP32_CHROMA_UPSAMPLE_CFG*/
  ISP_StatsCfg                ispStatsCfg;       /* ISP32_STATS_CFG */
} ISP_OperationConfigCmdType;

typedef struct {
  ISP_OperationConfigCmdType op_cmd;
}isp_operation_cfg_t;

typedef struct Hist_DMI_CfgCmdType {
  uint32_t set_channel;
  uint32_t set_start_addr;
  uint64_t table[256];
  uint32_t reset_channel;
  uint32_t reset_start_addr;
}Hist_DMI_CfgCmdType;
/* End: Data structures to hold HW specific table formats and params */

#endif /* __ISP_PIPELINE32_H__ */
