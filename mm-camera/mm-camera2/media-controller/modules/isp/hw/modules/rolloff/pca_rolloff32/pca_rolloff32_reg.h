/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PCA_ROLLOFF32_REG_H__
#define __PCA_ROLLOFF32_REG_H__


#define ISP_PCA_ROLLOFF32_CFG_OFF_0   0x00000274
#define ISP_PCA_ROLLOFF32_CFG_OFF_1   0x000007A8

#define ISP_PCA_ROLLOFF32_CFG_LEN_0  4
#define ISP_PCA_ROLLOFF32_CFG_LEN_1  3

/* Start: Data structures to hold HW specific table formats and params */
typedef struct PCA_RollOffConfigParams {
  /* VFE_ROLLOFF_CONFIG */
  uint32_t                     pixelOffset             : 9;
  uint32_t                     /* reserved */          : 7;
  uint32_t                     pcaLutBankSel           : 1;
  uint32_t                     /* reserved */          : 15;
  /* VFE_ROLLOFF_GRID_CFG_0 */
  uint32_t                     xDelta                  : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     yDelta                  : 10;
  uint32_t                     /* reserved */          : 2;
  /* VFE_ROLLOFF_GRID_CFG_1 */
  uint32_t                     gridWidth               : 9;
  uint32_t                     gridHeight              : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_0 */
  uint32_t                     xDeltaRight             : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     yDeltaRight             : 10;
  uint32_t                     /* reserved */          : 2;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_1 */
  uint32_t                     gridWidthRight          : 9;
  uint32_t                     gridHeightRight         : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_STRIPE_CFG_0 */
  uint32_t                     gridXIndex              : 4;
  uint32_t                     gridYIndex              : 4;
  uint32_t                     gridPixelXIndex         : 9;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     gridPixelYIndex         : 9;
  uint32_t                     /* reserved */          : 3;
  /* VFE_ROLLOFF_STRIPE_CFG_1 */
  uint32_t                     yDeltaAccum             : 13;
  uint32_t                     /* reserved */          : 19;
}__attribute__((packed, aligned(4))) PCA_RollOffConfigParams;

#endif /* __PCA_ROLLOFF32_REG_H__ */
