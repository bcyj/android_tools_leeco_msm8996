/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __AWB_STATS32_REG_H__
#define __AWB_STATS32_REG_H__

#define AWB_STATS_OFF 0x0000054C
#define AWB_STATS_LEN 8

#define ISP_STATS_AWB_BUF_NUM 4
#define ISP_STATS_AWB_BUF_SIZE (1040 * 4) //(memory size by bytes)

typedef struct ISP_StatsAwb_CfgCmdType {
  /*  VFE_STATS_AWB_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    1;
  uint32_t      /* reserved */          :    3;
  /*  VFE_STATS_AWB_RGN_SIZE_CFG  */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHeight             :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;

  /*  VFE_STATS_AWB_SGW_CFG */
  uint32_t        yMax                  :    8;
  uint32_t        yMin                  :    8;
  uint32_t      /* reserved */          :   16;

  /*  VFE_STATS_AWB_AGW_CFG_0 */
  int32_t        c1                    :   12;
  int32_t      /* reserved */          :    4;
  int32_t        c2                    :   12;
  int32_t      /* reserved */          :    4;

  /*  VFE_STATS_AWB_AGW_CFG_1 */
  int32_t        c3                    :   12;
  int32_t      /* reserved */          :    4;
  int32_t        c4                    :   12;
  int32_t      /* reserved */          :    4;

  /*  VFE_STATS_AWB_AGW_CFG_2 */
  int32_t        m1                    :   8;
  int32_t        m2                    :   8;
  int32_t        m3                    :   8;
  int32_t        m4                    :   8;

  /*  VFE_STATS_AWB_GX_CFG_0 */
  uint32_t        t1                    :   8;
  uint32_t        t2                    :   8;
  uint32_t        t3                    :   8;
  uint32_t        t6                    :   8;

  /*  VFE_STATS_AWB_GX_CFG_1 */
  uint32_t        t4                    :  10;
  uint32_t      /* reserved */          :   2;
  uint32_t        mg                    :   9;
  uint32_t      /* reserved */          :   3;
  uint32_t        t5                    :   8;
}__attribute__((packed, aligned(4))) ISP_StatsAwb_CfgCmdType;
#endif /*__AWB_STATS32_REG_H__*/
