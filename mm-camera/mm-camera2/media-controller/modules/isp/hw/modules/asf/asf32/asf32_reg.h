/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ASF32_REG_H__
#define __ASF32_REG_H__

#define ISP_ASF_OFF 0x000004A0
#define ISP_ASF_LEN 12

typedef struct ISP_AdaptiveFilterConfigCmdType {
  /* ASF Config Command */
  uint32_t                     smoothFilterEnabled     : 1;
  uint32_t                     sharpMode               : 2;
  uint32_t                     lpfMode                 : 1;
  uint32_t                     smoothCoefSurr          : 4;
  uint32_t                     smoothCoefCenter        : 8;
  uint32_t                     pipeFlushCount          : 13;
  uint32_t                     pipeFlushOvd            : 1;
  uint32_t                     flushHaltOvd            : 1;
  uint32_t                     cropEnable              : 1;
  /* Sharpening Config 0 */
  uint32_t                     sharpThreshE1           : 7;
  uint32_t                     /* reserved */          : 1;
  int32_t                      sharpK1                 : 6;
  uint32_t                     /* reserved */          : 2;
  int32_t                      sharpK2                 : 6;
  uint32_t                     /* reserved */          : 2;
  uint32_t                     normalizeFactor         : 7;
  uint32_t                     cutYSmooth              : 1;
  /* Sharpening Config 1 */
  int32_t                      sharpThreshE2           : 8;
  int32_t                      sharpThreshE3           : 8;
  int32_t                      sharpThreshE4           : 8;
  int32_t                      sharpThreshE5           : 8;
  /* Sharpening Coefficients 0 */
  int32_t                      F1Coeff0                : 6;
  int32_t                      F1Coeff1                : 6;
  int32_t                      F1Coeff2                : 6;
  int32_t                      F1Coeff3                : 6;
  int32_t                      F1Coeff4                : 6;
  int32_t                    /* reserved */            : 2;
  /* Sharpening Coefficients 1 */
  int32_t                      F1Coeff5                : 6;
  int32_t                      F1Coeff6                : 6;
  int32_t                      F1Coeff7                : 6;
  int32_t                      F1Coeff8                : 7;
  int32_t                     /* reserved */           : 7;
  /* Sharpening Coefficients 2 */
  int32_t                      F2Coeff0                : 6;
  int32_t                      F2Coeff1                : 6;
  int32_t                      F2Coeff2                : 6;
  int32_t                      F2Coeff3                : 6;
  int32_t                      F2Coeff4                : 6;
  int32_t                     /* reserved */           : 2;
  /* Sharpening Coefficients 3 */
  int32_t                      F2Coeff5                : 6;
  int32_t                      F2Coeff6                : 6;
  int32_t                      F2Coeff7                : 6;
  int32_t                      F2Coeff8                : 7;
  int32_t                     /* reserved */           : 7;
  /* Sharpening Coefficients 4 */
  int32_t                      F3Coeff0                : 6;
  int32_t                      F3Coeff1                : 6;
  int32_t                      F3Coeff2                : 6;
  int32_t                      F3Coeff3                : 6;
  int32_t                      F3Coeff4                : 6;
  int32_t                     /* reserved */           : 2;
  /* Sharpening Coefficients 5 */
  int32_t                      F3Coeff5                : 6;
  int32_t                      F3Coeff6                : 6;
  int32_t                      F3Coeff7                : 6;
  int32_t                      F3Coeff8                : 7;
  int32_t                     /* reserved */           : 7;
  /* TODO: Below Params are unused so far. */
  /* asf max edge */
  uint32_t                     maxEdge                 :13;
  uint32_t                    /* reserved */           : 3;
  uint32_t                     HBICount                :13;
  uint32_t                    /* reserved */           : 3;
  /* ASF Crop Width Config */
  uint32_t                     lastPixel               : 13;
  uint32_t                    /* reserved */           : 3;
  uint32_t                     firstPixel              : 13;
  uint32_t                    /* reserved */           : 3;
  /* ASP Crop Height Config */
  uint32_t                     lastLine                : 12;
  uint32_t                    /* reserved */           : 4;
  uint32_t                     firstLine               : 12;
  uint32_t                    /* reserved */           : 4;
  /* ASF Special Effects config */
  uint32_t                     nzFlag0                 : 2;
  uint32_t                     nzFlag1                 : 2;
  uint32_t                     nzFlag2                 : 2;
  uint32_t                     nzFlag3                 : 2;
  uint32_t                     nzFlag4                 : 2;
  uint32_t                     nzFlag5                 : 2;
  uint32_t                     nzFlag6                 : 2;
  uint32_t                     nzFlag7                 : 2;
  uint32_t                    /* reserved */           : 16;
} __attribute__((packed, aligned(4))) ISP_AdaptiveFilterConfigCmdType;

#endif //__ASF32_H__

