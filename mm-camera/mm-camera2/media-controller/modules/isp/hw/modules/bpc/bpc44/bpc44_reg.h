/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_BPC40_REG_H__
#define __DEMOSAIC_BPC40_REG_H__

#define ISP_DBPC40_CFG_OFF  0x00000444
#define ISP_DBPC40_LEN 4

#define ISP_DBPC_DEMOSAIC_MIX_CFG_OFF  0x00000440
#define ISP_DBPC_DEMOSAIC_MIX_CFG_LEN  1

/* Demosaic DBPC Update Command */
typedef union  ISP_DemosaicDBPCCfg_CmdType {
  struct  {
    /* Demosaic Config */
    /* Reserved for dbpc enable  */
    uint32_t     enable                           : 1;
    /* Reserved for dbcc enable  */
    uint32_t     /* reserved */                   : 1;
    /* Reserved for abcc enable */
    uint32_t     /* reserved */                   : 1;
    /* Reserved for abf enable  */
    uint32_t     /* reserved */                   : 1;
    /* Reserved for cosited rgb enable */
    uint32_t     /* reserved */                   : 1;
    uint32_t     /* reserved */                   : 3;
    /* reserved for abcc lut bank selection   */
    uint32_t      /* reserved */                  : 1;
    uint32_t     /* reserved */                   : 7;
    /* reserved for  pipe flush count   */
    uint32_t     /* reserved */                   : 13;
    uint32_t     /* reserved */                   : 1;
    /* reserved for pipe flush ovd */
    uint32_t     /* reserved */                   : 1;
    /* reserved for flush halt ovd */
    uint32_t     /* reserved */                   : 1;
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
  uint32_t     gbOffsetLo                       :10;
  uint32_t     gbOffsetHi                       :10;
  uint32_t     grOffsetHi                       :10;
  uint32_t     /* reserved */                   : 2;

  /* DBPC_OFFSET_CFG_2 */
  uint32_t     bOffsetLo                        :10;
  uint32_t     bOffsetHi                        :10;
  uint32_t     /* reserved */                   :12;
}__attribute__((packed, aligned(4))) ISP_DemosaicDBPC_CmdType;

#endif //__DEMOSAIC_BPC40_REG_H__
