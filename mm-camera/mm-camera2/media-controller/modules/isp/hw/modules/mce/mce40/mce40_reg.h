/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MCE40_REG_H__
#define __MCE40_REG_H__

#define ISP_MCE40_OFF 0x00000670
#define ISP_MCE40_LEN 9

#define ISP_MCE40_CHORMA_SUPP_MIX_OFF1 0x00000668
#define ISP_MCE40_CHORMA_SUPP_MIX_LEN1 1
#define ISP_MCE40_CHORMA_SUPP_MIX_OFF2 0x0000066C
#define ISP_MCE40_CHORMA_SUPP_MIX_LEN2 1

typedef struct ISP_mce_per_component_ConfigCmdType {
  /* config 0 */
  uint32_t         y1                      :   8;
  uint32_t         y2                      :   8;
  uint32_t         y3                      :   8;
  uint32_t         y4                      :   8;

  /*  config 1 */
  uint32_t         yM1                     :   7;
  uint32_t        /* reserved  */          :   1;
  uint32_t         yM3                     :   7;
  uint32_t         yS1                     :   4;
  uint32_t         yS3                     :   4;
  uint32_t         transWidth              :   5;
  uint32_t         transTrunc              :   4;

  /*  config 2 */
  int32_t          CRZone                  :   8;
  int32_t          CBZone                  :   8;
  int32_t          transSlope              :   5;
  int32_t          K                       :   9;
  int32_t         /* reserved  */          :   2;
}__attribute__((packed, aligned(4))) ISP_mce_per_component_ConfigCmdType;

typedef struct ISP_MCE_ConfigCmdType {
  ISP_mce_per_component_ConfigCmdType     redCfg;
  ISP_mce_per_component_ConfigCmdType     greenCfg;
  ISP_mce_per_component_ConfigCmdType     blueCfg;

}__attribute__((packed, aligned(4))) ISP_MCE_ConfigCmdType;

typedef union ISP_MCE_MIX_ConfigCmdType_1 {
  struct {
    /* hw_write_mask: control which bit we are writing to HW*/
    uint32_t     /* reserved  */        : 28;
    uint32_t     hw_wr_mask             : 1;
    uint32_t     /* reserved  */        : 3;

    /* Chroma Suppress 1 Config */
    /* reserved for chroma suppression.*/
    uint32_t     /* reserved  */        : 28;
    uint32_t     enable                 : 1;
    uint32_t     /* reserved  */        : 3;
  }__attribute__((packed, aligned(4)));

  struct {
    uint32_t mask;
    uint32_t cfg;
  }__attribute__((packed, aligned(4)));

}__attribute__((packed, aligned(4))) ISP_MCE_MIX_ConfigCmdType_1;

typedef union ISP_MCE_MIX_ConfigCmdType_2 {
  struct {
    /* hw_write_mask: control which bit we are writing to HW*/
    uint32_t     /* reserved  */        : 28;
    uint32_t     hw_wr_mask             : 4;

    /* Chroma Suppress 2 Config */
    /* reserved for chroma suppression.*/
    uint32_t     /* reserved  */        : 28;
    uint32_t     qk                     : 4;
  }__attribute__((packed, aligned(4)));

  struct {
    uint32_t mask;
    uint32_t cfg;
  }__attribute__((packed, aligned(4)));

}__attribute__((packed, aligned(4))) ISP_MCE_MIX_ConfigCmdType_2;

#endif //__MCE40_REG_H__
