/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_BCC32_REG_H__
#define __DEMOSAIC_BCC32_REG_H__

#define ISP_DBCC32_DEMOSAIC_MIX_CFG_OFF  0x00000298
#define ISP_DBCC32_DEMOSAIC_MIX_CFG_LEN  1

#define ISP_DBCC32_OFF 0x0000060C
#define ISP_DBCC32_LEN 4

/* Demosaic DBCC Update Command */
typedef union  ISP_DemosaicDBCCCfg_CmdType {
  struct {
    /* Demosaic Config */
    uint32_t     /* reserved */                 : 1;
    uint32_t     enable                         : 1;
    uint32_t     /* reserved */                 : 30;
  }__attribute__((packed, aligned(4)));
  uint32_t cfg;
}__attribute__((packed, aligned(4))) ISP_DemosaicDBCCCfg_CmdType;

/* Demosaic DBCC config command */
typedef struct ISP_DemosaicDBCC_CmdType {
  /* DBCC_CFG */
  uint32_t     fminThreshold                    : 8;
  uint32_t     fmaxThreshold                    : 8;
  uint32_t     /* reserved */                   : 16;

  /* DBCC_OFFSET_CFG_0 */
  uint32_t     rOffsetLo                        : 10;
  uint32_t     rOffsetHi                        : 10;
  uint32_t     grOffsetLo                       : 10;
  uint32_t     /* reserved */                   :  2;

  /* DBCCC_OFFSET_CFG_1 */
  uint32_t     gbOffsetLo                       : 10;
  uint32_t     gbOffsetHi                       : 10;
  uint32_t     grOffsetHi                       : 10;
  uint32_t     /* reserved */                   :  2;

  /* DBCC_OFFSET_CFG_2 */
  uint32_t     bOffsetLo                        : 10;
  uint32_t     bOffsetHi                        : 10;
  uint32_t     /* reserved */                   : 12;
}__attribute__((packed, aligned(4))) ISP_DemosaicDBCC_CmdType;

#endif //__DEMOSAIC_BCC32_REG_H__
