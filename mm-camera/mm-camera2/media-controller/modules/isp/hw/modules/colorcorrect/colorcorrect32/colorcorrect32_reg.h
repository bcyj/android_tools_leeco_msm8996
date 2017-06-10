/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __COLOR_CORRECT32_REG_H__
#define __COLOR_CORRECT32_REG_H__

#define ISP_COLOR_COR32_OFF 0x00000388
#define ISP_COLOR_COR32_LEN 13

/* Color Correction Config */
typedef struct ISP_ColorCorrectionCfgCmdType {
  /* Color Corr. Coefficient 0 Config */
  int32_t      C0                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 1 Config */
  int32_t      C1                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 2 Config */
  int32_t      C2                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 3 Config */
  int32_t      C3                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 4 Config */
  int32_t      C4                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 5 Config */
  int32_t      C5                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 6 Config */
  int32_t      C6                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 7 Config */
  int32_t      C7                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 8 Config */
  int32_t      C8                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Offset 0 Config */
  int32_t      K0                          :11;
  int32_t     /* reserved */               :21;
  /* Color Corr. Offset 1 Config */
  int32_t      K1                          :11;
  int32_t     /* reserved */               :21;
  /* Color Corr. Offset 2 Config */
  int32_t      K2                          :11;
  int32_t     /* reserved */               :21;
  /* Color Corr. Coefficient Q Config */
  uint32_t    coefQFactor                  : 2;
  uint32_t    /* reserved */               :30;
}__attribute__((packed, aligned(4))) ISP_ColorCorrectionCfgCmdType;

#endif //__COLOR_CORRECT32_REG_H__
