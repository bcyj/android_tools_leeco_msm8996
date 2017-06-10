/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FRAME_SKIP32_REG_H__
#define __FRAME_SKIP32_REG_H__


/*  Frame Skip Config Command */
typedef struct ISP_FrameSkipConfigCmdType {
  /* new */
  /* Frame Drop Enc (output2) */
  uint32_t output2YPeriod               : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output2CbCrPeriod            : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output2YPattern              : 32;
  uint32_t output2CbCrPattern           : 32;
  /* Frame Drop View (output1) */
  uint32_t output1YPeriod               : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output1CbCrPeriod            : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output1YPattern              : 32;
  uint32_t output1CbCrPattern           : 32;
}__attribute__((packed, aligned(4))) ISP_FrameSkipConfigCmdType;

#endif //__FRAME_SKIP32_REG_H__
