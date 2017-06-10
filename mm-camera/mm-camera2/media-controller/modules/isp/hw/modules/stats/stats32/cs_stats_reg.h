/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CS_STATS32_REG_H__
#define __CS_STATS32_REG_H__

#define CS_MAX_V_REGIONS 1
#define CS_MAX_H_REGIONS 1344

#define CS_STATS_OFF 0x00000574
#define CS_STATS_LEN 2

#define ISP_STATS_CS_BUF_NUM  4
#define ISP_STATS_CS_BUF_SIZE 1344 * 4 //(membory size by bytes)

/* Stats CS Config */
typedef struct ISP_StatsCs_CfgType {
  /*  VFE_STATS_CS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /*  VFE_STATS_CS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth              :    2;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnHeight             :   12;
  uint32_t        rgnHNum               :   11;
  uint32_t       /* reserved */         :    5;
}__attribute__((packed, aligned(4))) ISP_StatsCs_CfgType;

#endif /*__CS_STATS32_REG_H__*/
