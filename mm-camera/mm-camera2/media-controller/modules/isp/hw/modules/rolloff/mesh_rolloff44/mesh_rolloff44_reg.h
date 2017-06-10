/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MESH_ROLLOFF40_REG_H__
#define __MESH_ROLLOFF40_REG_H__

#define ISP_MESH_ROLLOFF40_CFG_OFF             0x00000400
#define ISP_MESH_ROLLOFF40_CFG_LEN             9

#define ISP_MESH_ROLLOFF40_TABLE_SIZE          130

/* Start: Data structures to hold HW specific table formats and params */
typedef struct MESH_RollOff_v4_ConfigParams {
  /* ISP_ROLLOFF_CONFIG */
  uint32_t                     pixelOffset             : 9;
  uint32_t                     /* reserved */          : 7;
  uint32_t                     pcaLutBankSel           : 1;
  uint32_t                     /* reserved */          : 15;
   /* ISP_ROLLOFF_GRID_CFG_0 */
  uint32_t                     blockWidth              : 9;
  uint32_t                     blockHeight             : 9;
  uint32_t                     /* reserved */          : 14;
  /* ISP_ROLLOFF_GRID_CFG_1 */
  uint32_t                     subGridXDelta           : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridYDelta           : 10;
  uint32_t                     interpFactor            : 2;
    /* ISP_ROLLOFF_GRID_CFG_2 */
  uint32_t                     subGridWidth            : 9;
  uint32_t                     subGridHeight           : 9;
  uint32_t                     /* reserved */          : 14;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_0 */
  uint32_t                     blockWidthRight         : 9;
  uint32_t                     blockHeightRight        : 9;
  uint32_t                     /* reserved */          : 14;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_1 */
  uint32_t                     subGridXDeltaRight      : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridYDeltaRight      : 10;
  uint32_t                     interpFactorRight       : 2;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_2 */
  uint32_t                      subGridWidthRight      : 9;
  uint32_t                      subGridHeightRight     : 9;
  uint32_t                      /* reserved */         : 14;
  /* ISP_ROLLOFF_STRIPE_CFG_0 */
  uint32_t                     blockXIndex             : 4;
  uint32_t                     blockYIndex             : 4;
  uint32_t                     PixelXIndex             : 9;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     PixelYIndex             : 9;
  uint32_t                     /* reserved */          : 3;
  /* ISP_ROLLOFF_STRIPE_CFG_1 */
  uint32_t                     yDeltaAccum             : 13;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridXIndex           : 3;
  uint32_t                     /* reserved */          : 5;
  uint32_t                     subGridYIndex           : 3;
  uint32_t                     /* reserved */          : 5;
}__attribute__((packed, aligned(4))) MESH_RollOff_v4_ConfigParams;

#endif /* __MESH_ROLLOFF40_REG_H__ */
