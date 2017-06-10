/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __WB40_REG_H__
#define __WB40_REG_H__

#define ISP_WB40_OFF 0x00000580
#define ISP_WB40_LEN 1

#define ISP_WB40_INIT_G_GAIN 1
#define ISP_WB40_INIT_B_GAIN 1.8
#define ISP_WB40_INIT_R_GAIN 1

typedef struct ISP_WhiteBalanceConfigCmdType {
  /* WB Config */
  uint32_t          ch0Gain             : 9;
  uint32_t          ch1Gain             : 9;
  uint32_t          ch2Gain             : 9;
  uint32_t         /* reserved */       : 5;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceConfigCmdType;

typedef struct ISP_WhiteBalanceRightConfigCmdType {
  /* WB Config */
  uint32_t          ch0GainRight        : 9;
  uint32_t          ch1GainRight        : 9;
  uint32_t          ch2GainRight        : 9;
  uint32_t         /* reserved */       : 5;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceRightConfigCmdType;

#endif //__WB40_REG_H__
