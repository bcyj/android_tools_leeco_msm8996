/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_ABF40_REG_H__
#define __DEMOSAIC_ABF40_REG_H__

#define ISP_ABF_DEMOSAIC_MIX_CFG_OFF 0x00000440

#define ISP_ABF40_OFF 0x00000464
#define ISP_ABF40_LEN 45

typedef union ISP_DemosaicABF_Cfg {
  struct {
    /* Demosaic Config */
    uint32_t     /* reserved */                   :  3;
    uint32_t       enable                         :  1;
    uint32_t     /* reserved */                   : 28;
  } __attribute__((packed, aligned(4)));
  uint32_t cfg;
}__attribute__((packed, aligned(4))) ISP_DemosaicABF_Cfg;

typedef struct ISP_DemosaicABF_gCfg {
  /* Demosaic ABF Green Config 0 */
  uint32_t       Cutoff1                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       Cutoff2                : 12;
  uint32_t     /* reserved */           :  4;
  /* Demosaic ABF Green Config 1 */
  uint32_t       Cutoff3                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       SpatialKernelA0        :  7;
  uint32_t     /* reserved */           :  1;
  uint32_t       SpatialKernelA1        :  7;
  uint32_t     /* reserved */           :  1;
  /* Demosaic ABF Green Config 2 */
  uint32_t       MultNegative           : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       MultPositive           : 12;
  uint32_t     /* reserved */           :  4;
}__attribute__((packed, aligned(4))) ISP_DemosaicABF_gCfg;

typedef struct ISP_DemosaicABF_Lut {
  /* Demosaic ABF LUT */
  int32_t       LUT0                    : 12;
  int32_t     /* reserved */            :  4;
  int32_t       LUT1                    : 12;
  int32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4)))  ISP_DemosaicABF_Lut;

typedef struct ISP_DemosaicABF_RBCfg {
/* blue */
  /* Demosaic ABF Blue Config 0 */
  uint32_t       Cutoff1                 : 12;
  uint32_t     /* reserved */            :  4;
  uint32_t       Cutoff2                 : 12;
  uint32_t     /* reserved */            :  4;
  /* Demosaic ABF Blue Config 1 */
  uint32_t       Cutoff3                 : 12;
  uint32_t     /* reserved */            : 20;
  /* Demosaic ABF Blue Config 2 */
  uint32_t       MultNegative            : 12;
  uint32_t     /* reserved */            :  4;
  uint32_t       MultPositive            : 12;
  uint32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4)))  ISP_DemosaicABF_RBCfg;

/* Demosaic ABF Update Command  */
typedef struct ISP_DemosaicABF_CmdType {
  /* Green config */
  ISP_DemosaicABF_gCfg    gCfg;
  ISP_DemosaicABF_Lut     gPosLut[8] ;
  ISP_DemosaicABF_Lut     gNegLut[4] ;
  /* Blue config */
  ISP_DemosaicABF_RBCfg   bCfg;
  ISP_DemosaicABF_Lut     bPosLut[8] ;
  ISP_DemosaicABF_Lut     bNegLut[4] ;
  /* Red config */
  ISP_DemosaicABF_RBCfg   rCfg;
  ISP_DemosaicABF_Lut     rPosLut[8] ;
  ISP_DemosaicABF_Lut     rNegLut[4] ;
} __attribute__((packed, aligned(4))) ISP_DemosaicABF_CmdType;

#endif //__DEMOSAIC_ABF40_REG_H__
