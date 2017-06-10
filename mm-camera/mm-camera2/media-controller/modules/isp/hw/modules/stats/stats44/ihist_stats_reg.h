/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __IHIST_STATS_REG_H__
#define __IHIST_STATS_REG_H__

#define IHIST_TABLE_LENGTH 256

#define IHIST_STATS_OFF 0x000008F4
#define IHIST_STATS_LEN 2

/*memory size by bytes*/
#define ISP_STATS_IHIST_BUF_NUM  4
#define ISP_STATS_IHIST_BUF_SIZE 128 * 4

/* Stats ihist Config*/
typedef struct ISP_StatsIhist_CfgType {
  /*  VFE_STATS_IHIST_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :    13;
  uint32_t        channelSelect         :     3;
  uint32_t        rgnVOffset            :    12;
  uint32_t        shiftBits             :    3;
  uint32_t        siteSelect            :    1;

  /*  VFE_STATS_IHIST_RGN_SIZE_CFG  */
  uint32_t        rgnHNum               :   12;
  uint32_t        rgnVNum               :   11;
  uint32_t      /* reserved */          :    9;
}__attribute__((packed, aligned(4))) ISP_StatsIhist_CfgType;
#endif /*__IHIST_STATS_REG_H__*/
