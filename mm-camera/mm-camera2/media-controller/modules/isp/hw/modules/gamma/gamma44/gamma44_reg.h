/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __GAMMA40_REG_H__
#define __GAMMA40_REG_H__

#define ISP_RGB_LUT40_OFF 0x00000638
#define ISP_RGB_LUT40_LEN 1

typedef struct ISP_GammaLutSelect {
  /* LUT Bank Select Config */
  uint32_t      ch0BankSelect               : 1;
  uint32_t      ch1BankSelect               : 1;
  uint32_t      ch2BankSelect               : 1;
  uint32_t     /* reserved */               :29;
}__attribute__((packed, aligned(4))) ISP_GammaLutSelect;


#endif //__GAMMA40_REG_H__
