/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CLAMP40_REG_H__
#define __CLAMP40_REG_H__

#define ISP_CLAMP40_ENC_OFF         0x00000874
#define ISP_CLAMP40_ENC_LEN         2

#define ISP_CLAMP40_VIEW_OFF        0x0000087C
#define ISP_CLAMP40_VIEW_LEN        2

/* Output Clamp Config Command */
typedef struct ISP_OutputClampConfigCmdType {
  /* Output Clamp Maximums */
  uint32_t  yChanMax         :  8;
  uint32_t  cbChanMax        :  8;
  uint32_t  crChanMax        :  8;
  uint32_t  /* reserved */   :  8;
  /* Output Clamp Minimums */
  uint32_t  yChanMin         :  8;
  uint32_t  cbChanMin        :  8;
  uint32_t  crChanMin        :  8;
  uint32_t  /* reserved */   :  8;
}__attribute__((packed, aligned(4))) ISP_OutputClampConfigCmdType;

#endif // __CLAMP40_REG_H__
