/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LUMA_ADAPTATION40_REG_H__
#define __LUMA_ADAPTATION40_REG_H__

#define ISP_LA40_OFF 0x0000063C
#define ISP_LA40_LEN 1

typedef struct ISP_LABankSel {
  /* LA Config */
  uint32_t      lutBankSelect               : 1;
  uint32_t     /* reserved */               :31;
}__attribute__((packed, aligned(4))) ISP_LABankSel;

/* LA Enable/Disable on fly */
typedef struct ISP_LumaAdapEnCmdType {
  uint32_t /*reserved*/     :16;
  uint32_t luma_enable      :1;
  uint32_t /*reserved*/     :15;
} ISP_LumaAdapEnCmdType;


#endif
