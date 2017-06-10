/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LINEARIZATION40_REG_H__
#define __LINEARIZATION40_REG_H__

#define ISP_LINEARIZATION40_OFF 0x0000037C
#define ISP_LINEARIZATION40_LEN 17

typedef struct ISP_PointSlopeData {
  /* INTERP_0 */
  uint32_t kneePoint_P1         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P0         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_1 */
  uint32_t kneePoint_P3         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P2         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_2 */
  uint32_t kneePoint_P5         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P4         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_3 */
  uint32_t kneePoint_P7         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P6         :12;
  uint32_t /* reserved */       : 4;
}__attribute__((packed, aligned(4))) ISP_PointSlopeData;

typedef struct ISP_LinearizationCfgParams {
  /* BLACK_CONFIG */
  uint32_t lutBankSel           : 1;
  uint32_t /* reserved */       :31;

  /* Knee points for R channel */
  ISP_PointSlopeData pointSlopeR;

  /* Knee points for Gr channel */
  ISP_PointSlopeData pointSlopeGb;

  /* Knee points for B channel */
  ISP_PointSlopeData pointSlopeB;

  /* Knee points for Gb channel */
  ISP_PointSlopeData pointSlopeGr;

}__attribute__((packed, aligned(4))) ISP_LinearizationCfgParams;

#endif //__LINEARIZATION40_REG_H__
