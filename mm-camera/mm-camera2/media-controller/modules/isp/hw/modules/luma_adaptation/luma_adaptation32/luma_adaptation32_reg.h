/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LUMA_ADAPTATION32_REG_H__
#define __LUMA_ADAPTATION32_REG_H__

#define ISP_LA32_OFF 0x000003C0
#define ISP_LA32_LEN 1

typedef struct VFE_LABankSel {
  /* LA Config */
  uint32_t      lutBankSelect               : 1;
  uint32_t     /* reserved */               :31;
}__attribute__((packed, aligned(4))) ISP_LABankSel;

/* LA Enable/Disable on fly */
typedef struct ISP_LumaAdapEnCmdType {
  uint32_t /*reserved*/     :17;
  uint32_t luma_enable      :1;
  uint32_t /*reserved*/     :14;
} ISP_LumaAdapEnCmdType;


#endif
