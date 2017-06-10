/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_BPC32_REG_H__
#define __DEMOSAIC_BPC32_REG_H__

#define ISP_DBPC32_DEMOSAIC_MIX_CFG_OFF  0x00000298
#define ISP_DBPC32_DEMOSAIC_MIX_CFG_LEN  1

#define ISP_DBPC32_CFG_OFF  0x0000029C
#define ISP_DBPC32_LEN 2

#define ISP_DBPC32_CFG_OFF_1 0x00000604
#define ISP_DBPC32_LEN_1 2

/* Demosaic DBPC Update Command */
typedef union  ISP_DemosaicDBPCCfg_CmdType {
  struct  {
    /* Demosaic Config */
    uint32_t     enable                         : 1;
    uint32_t     /* reserved */                 : 31;
  }__attribute__((packed, aligned(4)));
  uint32_t cfg_reg;
}__attribute__((packed, aligned(4))) ISP_DemosaicDBPCCfg_CmdType;

/* Demosaic DBPC Update Command */
typedef struct ISP_DemosaicDBPC_CmdType {
  /* DBPC_CFG */
  uint32_t     fminThreshold                    : 8;
  uint32_t     fmaxThreshold                    : 8;
  uint32_t     /* reserved */                   : 16;

  /* DBPC_OFFSET_CFG_0 */
  uint32_t     rOffsetLo                        : 10;
  uint32_t     rOffsetHi                        : 10;
  uint32_t     grOffsetLo                       : 10;
  uint32_t     /* reserved */                   :  2;

  /* DBPC_OFFSET_CFG_1 */
  uint32_t     gbOffsetLo                       : 10;
  uint32_t     gbOffsetHi                       : 10;
  uint32_t     grOffsetHi                       : 10;
  uint32_t     /* reserved */                   :  2;

  /* DBPC_OFFSET_CFG_2 */
  uint32_t     bOffsetLo                        : 10;
  uint32_t     bOffsetHi                        : 10;
  uint32_t     /* reserved */                   : 12;
}__attribute__((packed, aligned(4))) ISP_DemosaicDBPC_CmdType;

#endif //__DEMOSAIC_BPC32_REG_H__
