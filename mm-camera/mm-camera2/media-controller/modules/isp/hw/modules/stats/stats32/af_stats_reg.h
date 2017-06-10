/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __AF_STATS_REG_H__
#define __AF_STATS_REG_H__

typedef struct ISP_StatsAf_CfgCmdType {
  /*  VFE_STATS_AF_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /* VFE_STATS_AF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :   12;
  uint32_t        rgnHeight             :   11;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;
  /*  VFE_STATS_AF_HPF_COEF0 */
  int32_t    a00                       :    5; /* from chromatix */
  int32_t    a02                       :    5; /* from chromatix */
  int32_t    a04                       :    5; /* from chromatix */
  int32_t      /* reserved */          :    1;
  uint32_t    fvMin                     :   11; /* metrixmax */
  uint32_t      /* reserved */          :    4;
  uint32_t    fvMode                    :    1; /* metricSelection */

  /*  VFE_STATS_AF_HPF_COEF1 */
  int32_t    a20                       :   5; /* from chromatix */
  int32_t    a21                       :   5; /* from chromatix */
  int32_t    a22                       :   5; /* from chromatix */
  int32_t    a23                       :   5; /* from chromatix */
  int32_t    a24                       :   5; /* from chromatix */
  uint32_t      /* reserved */          :   7;

}__attribute__((packed, aligned(4))) ISP_StatsAf_CfgCmdType;
#endif /*__AF_STATS_REG_H__*/

