/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_EXPOSURE_REG_H__
#define __BAYER_EXPOSURE_REG_H__

#define BE_STATS_OFF 0x0000088C
#define BE_STATS_LEN 3

/*memory size by bytes*/
#define ISP_STATS_BE_BUF_NUM  4
#define ISP_STATS_BE_BUF_SIZE   ((32 * 24 * 6) * 4)

typedef struct ISP_StatsBe_CfgCmdType {
  /*  VFE_STATS_BE_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BE_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t        rgnHeight             :    9;
  uint32_t        rgnHNum               :    5;
  uint32_t        /* reserved */        :    1;
  uint32_t        rgnVNum               :    5;
  uint32_t        /* reserved */        :    3;
  /* VFE_STATS_BE_THRESHOLD_CFG */
  uint32_t        rMax                 :    8;
  uint32_t        grMax                :    8;
  uint32_t        bMax                 :    8;
  uint32_t        gbMax                :    8;
}__attribute__((packed, aligned(4))) ISP_StatsBe_CfgCmdType;

#endif /*__BAYER_EXPOSURE_REG_H__*/
