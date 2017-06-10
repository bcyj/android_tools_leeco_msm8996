/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __AEC_STATS_REG_H__
#define __AEC_STATS_REG_H__

typedef struct ISP_StatsAe_CfgCmdType {
  /*  VFE_STATS_AE_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    4;

  /*  VFE_STATS_AE_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    10;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnHeight             :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;
}__attribute__((packed, aligned(4))) ISP_StatsAe_CfgCmdType;

#endif /*__AEC_STATS_REG_H__*/
