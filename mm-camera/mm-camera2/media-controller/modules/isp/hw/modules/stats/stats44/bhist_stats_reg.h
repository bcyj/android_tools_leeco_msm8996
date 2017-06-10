/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_HIST_REG_H__
#define __BAYER_HIST_REG_H__

#define BHIST_STATS_OFF 0x000008BC
#define BHIST_STATS_LEN 2

#define ISP_STATS_BHIST_BUF_NUM  4
#define ISP_STATS_BHIST_BUF_SIZE   (256 * 4) * 4

typedef struct ISP_StatsBhist_CfgCmdType {
  /*  VFE_STATS_BHIST_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BHIST_RGN_SIZE_CFG */
  uint32_t        rgnHNum               :    12;
  uint32_t        rgnVNum               :    11;
  uint32_t      /* reserved 23:31 */    :     9;
}__attribute__((packed, aligned(4))) ISP_StatsBhist_CfgCmdType;

#endif /*__BAYER_HIST_REG_H__*/
