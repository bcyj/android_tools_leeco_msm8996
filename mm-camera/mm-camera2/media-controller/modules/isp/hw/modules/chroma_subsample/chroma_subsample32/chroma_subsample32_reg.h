/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CHROMA_SUBSAMPLE32_REG_H__
#define __CHROMA_SUBSAMPLE32_REG_H__

#define ISP_CHROMA_SS32_OFF 0x000004F8
#define ISP_CHROMA_SS32_LEN 3

/*  Chroma Subsample Config Command */
typedef struct ISP_ChromaSubsampleConfigCmdType {
  /* Chroma Subsample Selection */
  uint32_t    hCositedPhase                             : 1;
  uint32_t    vCositedPhase                             : 1;
  uint32_t    hCosited                                  : 1;
  uint32_t    vCosited                                  : 1;
  uint32_t    hsubSampleEnable                          : 1;
  uint32_t    vsubSampleEnable                          : 1;
  uint32_t    cropEnable                                : 1;
  uint32_t    /* reserved */                            :25;

  uint32_t    cropWidthLastPixel                        :12;
  uint32_t    /* reserved */                            : 4;
  uint32_t    cropWidthFirstPixel                       :12;
  uint32_t    /* reserved */                            : 4;

  uint32_t    cropHeightLastLine                        :12;
  uint32_t    /* reserved */                            : 4;
  uint32_t    cropHeightFirstLine                       :12;
  uint32_t    /* reserved */                            : 4;
} __attribute__((packed, aligned(4))) ISP_ChromaSubsampleConfigCmdType;

#endif //__CHROMA_SUBSAMPLE32_REG_H__
