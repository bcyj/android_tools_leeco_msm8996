/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FOVCROP32_REG_H__
#define __FOVCROP32_REG_H__

#define ISP_FOV32_OFF 0x00000360
#define ISP_FOV32_LEN 2

/* Field Of View (FOV) Crop Config Command */
typedef struct ISP_FOV_CropConfigCmdType {
  uint32_t  lastPixel        : 13;
  uint32_t  /* reserved */   :  3;
  uint32_t  firstPixel       : 13;
  uint32_t  /* reserved */   :  3;
  /* FOV Corp, Part 2 */
  uint32_t  lastLine         : 12;
  uint32_t  /* reserved */   :  4;
  uint32_t  firstLine        : 12;
  uint32_t  /* reserved */   :  4;
}__attribute__((packed, aligned(4))) ISP_FOV_CropConfigCmdType;

#endif //__FOVCROP32_REG_H__
