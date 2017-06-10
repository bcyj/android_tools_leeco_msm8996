/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __RS_STATS40_REG_H__
#define __RS_STATS40_REG_H__

#define RS_STATS_OFF 0x000008E4
#define RS_STATS_LEN 2

#define ISP_STATS_RS_BUF_NUM  4
#define ISP_STATS_RS_BUF_SIZE (1024 * 4) * 2 // 1024x4 total number of 16-bit outputs


/* Stats RS Config */
typedef struct ISP_StatsRs_CfgType {
  /*  VFE_STATS_RS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset          :   13;
  uint32_t      /* reserved */        :    3;
  uint32_t        rgnVOffset          :   12;
  uint32_t        shiftBits           :    3;
  uint32_t      /* reserved */        :    1;

  /*  VFE_STATS_RS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth            :   13;
  uint32_t        rgnHeight           :    2;
  uint32_t      /* reserved */        :    1;
  uint32_t        rgnHNum             :    2;
  uint32_t       /* reserved */       :    2;
  uint32_t        rgnVNum             :   10;
  uint32_t      /* reserved */        :    2;
}__attribute__((packed, aligned(4))) ISP_StatsRs_CfgType;
#endif /*__RS_STATS40_REG_H__*/
