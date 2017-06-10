/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CHROMA_SUPPRESS32_REG_H__
#define __CHROMA_SUPPRESS32_REG_H__

#define ISP_CHROMA32_SUP_OFF 0x000003E8
#define ISP_CHROMA32_SUP_LEN 1

#define ISP_CHROMA32_SUP_MIX_OFF_1 0x000003EC
#define ISP_CHROMA32_SUP_MIX_LEN_1 1

#define ISP_CHROMA32_SUP_MIX_OFF_2 0x000003F0
#define ISP_CHROMA32_SUP_MIX_LEN_2 1

/* Chroma Suppression Config */
typedef struct ISP_ChromaSuppress_ConfigCmdType {
  /* Chroma Suppress 0 Config */
  uint32_t                      ySup1                  : 8;
  uint32_t                      ySup2                  : 8;
  uint32_t                      ySup3                  : 8;
  uint32_t                      ySup4                  : 8;
}__attribute__((packed, aligned(4))) ISP_ChromaSuppress_ConfigCmdType;

/* Chroma Suppression Config MIX regster 1 */
typedef union ISP_ChromaSuppress_Mix1_ConfigCmdType {
  struct {
    /* Chroma Suppress 1 Config */
    uint32_t                      ySupM1                 : 7;
    uint32_t                     /* reserved  */         : 1;
    uint32_t                      ySupM3                 : 7;
    uint32_t                     /* reserved  */         : 1;
    uint32_t                      ySupS1                 : 3;
    uint32_t                     /* reserved  */         : 1;
    uint32_t                      ySupS3                 : 3;
    uint32_t                     /* reserved  */         : 1;
    uint32_t                      chromaSuppressEn       : 1;
    uint32_t                     /* reserved  */         : 3;
    /* reserved for MCE enable */
    uint32_t                      /* reserved  */        : 1;
    uint32_t                      /* reserved  */        : 3;
  } __attribute__((packed, aligned(4)));
  uint32_t cfg;
}__attribute__((packed, aligned(4))) ISP_ChromaSuppress_Mix1_ConfigCmdType;

/* Chroma Suppression Config Mix Register 2 */
typedef union ISP_ChromaSuppress_Mix2_ConfigCmdType {
  struct {
    /* Chroma Suppress 2 Config */
    uint32_t                      cSup1                  : 8;
    uint32_t                      cSup2                  : 8;
    uint32_t                      cSupM1                 : 7;
    uint32_t                     /* reserved  */         : 1;
    uint32_t                      cSupS1                 : 3;
    uint32_t                     /* reserved  */         : 1;
    /* reserved for QK */
    uint32_t                      /* reserved  */        : 4;
  } __attribute__((packed, aligned(4)));
  uint32_t cfg;
}__attribute__((packed, aligned(4))) ISP_ChromaSuppress_Mix2_ConfigCmdType;

#endif //__CHROMA_SUPPRESS32_REG_H__
