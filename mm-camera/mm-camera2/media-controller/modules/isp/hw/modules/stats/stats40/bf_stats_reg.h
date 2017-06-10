/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_FOCUS40_REG_H__
#define __BAYER_FOCUS40_REG_H__

#define BF_STATS_OFF 0x000008A4
#define BF_STATS_LEN 6

#define ISP_STATS_BF_BUF_NUM  4
#define ISP_STATS_BF_BUF_SIZE  ((18 * 14 * 14) * 4)


typedef struct ISP_StatsBf_CfgCmdType {
  /*  VFE_STATS_BF_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHeight             :    9;
  uint32_t        rgnHNum               :    5;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnVNum               :    4;
  /* VFE_STATS_BF_FILTER_CFG_0 */
  uint32_t        r_fv_min              :    14;
  uint32_t      /* reserved 14:15 */    :    2;
  uint32_t        gr_fv_min             :    14;
  uint32_t      /* reserved 30:31 */    :    2;
  /* VFE_STATS_BF_FILTER_CFG_1 */
  uint32_t        b_fv_min              :    14;
  uint32_t      /* reserved 14:15 */    :    2;
  uint32_t        gb_fv_min             :    14;
  uint32_t      /* reserved 30:31 */    :    2;
  /* VFE_STATS_BF_FILTER_COEFF_0 */
  int32_t        a00                   :    5;
  int32_t        a01                   :    5;
  int32_t        a02                   :    5;
  int32_t        a03                   :    5;
  int32_t        a04                   :    5;
  uint32_t      /* reserved 25:31 */   :    7;
  /* VFE_STATS_BF_FILTER_COEFF_1 */
  int32_t        a10                   :    5;
  int32_t        a11                   :    5;
  int32_t        a12                   :    5;
  uint32_t      /* reserved */         :    1;
  int32_t        a13                   :    5;
  uint32_t      /* reserved */         :    3;
  int32_t        a14                   :    5;
  uint32_t      /* reserved 25:31 */   :    3;
}__attribute__((packed, aligned(4))) ISP_StatsBf_CfgCmdType;

#endif /*__BAYER_FOCUS40_REG_H__*/
