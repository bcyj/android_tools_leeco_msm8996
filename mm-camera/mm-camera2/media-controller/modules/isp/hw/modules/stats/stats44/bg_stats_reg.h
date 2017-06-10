/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_GRID40_REG_H__
#define __BAYER_GRID40_REG_H__

#define BG_STATS_OFF 0x00000898
#define BG_STATS_LEN 3

#define ISP_STATS_BG_BUF_NUM  4
#define ISP_STATS_BG_BUF_SIZE   ((64 * 48 * 6) * 4)

typedef struct ISP_StatsBg_CfgCmdType {
  /*  VFE_STATS_BG_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BG_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHeight             :    8;
  uint32_t        rgnHNum               :    7;
  uint32_t        rgnVNum               :    6;
  uint32_t      /* reserved */          :    1;
  /* VFE_STATS_BG_THRESHOLD_CFG */
  uint32_t        rMax                  :    8;
  uint32_t        grMax                 :    8;
  uint32_t        bMax                  :    8;
  uint32_t        gbMax                 :    8;
}__attribute__((packed, aligned(4))) ISP_StatsBg_CfgCmdType;
#endif /*__BAYER_GRID40_REG_H__*/
