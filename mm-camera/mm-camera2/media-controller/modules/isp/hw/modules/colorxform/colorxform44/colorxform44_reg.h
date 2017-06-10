/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __COLOR_XFORM40_REG_H__
#define __COLOR_XFORM40_REG_H__

#define ISP_COLOR_XFORM40_OFF 0x0000071C
#define ISP_COLOR_XFORM40_LEN 16

typedef struct ISP_colorXformCfgCmdType {
  /*  VFE_COLOR_XFORM_ENC_Y_MATRIX_0   */
  uint32_t        m00                   :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        m01                   :   13;
  uint32_t       /*reserved */          :    3;
  /*  VFE_COLOR_XFORM_ENC_Y_MATRIX_1 */
  uint32_t        m02                   :   13;
  uint32_t        /* reserveed */       :    1;
  uint32_t        o0                    :    9;
  uint32_t        S0                    :    9;
   /*  VFE_COLOR_XFORM_ENC_CB_MATRIX_0   */
  uint32_t        m10                   :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        m11                   :   13;
  uint32_t       /*reserved */          :    3;
   /*  VFE_COLOR_XFORM_ENC_CB_MATRIX_1   */
  uint32_t        m12                   :   13;
  uint32_t      /* reserved */          :    1;
  uint32_t        o1                    :    9;
  uint32_t        s1                    :    9;
   /*  VFE_COLOR_XFORM_ENC_CR_MATRIX_0   */
  uint32_t        m20                   :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        m21                   :   13;
  uint32_t       /*reserved */          :    3;
   /*  VFE_COLOR_XFORM_ENC_CR_MATRIX_1   */
  uint32_t        m22                   :   13;
  uint32_t      /* reserved */          :    1;
  uint32_t        o2                    :    9;
  uint32_t        s2                    :    9;
     /* VFE_COLOR_XFORM_ENC_CLAMP_HI */
    uint32_t        c01                   :    8;
    uint32_t        c11                   :    8;
    uint32_t        c21                   :    8;
    uint32_t      /* reserved */          :    8;
     /* VFE_COLOR_XFORM_ENC_CLAMP_LO */
    uint32_t        c00                   :    8;
    uint32_t        c10                   :    8;
    uint32_t        c20                   :    8;
    uint32_t      /* reserved */          :    8;

}__attribute__((packed, aligned(4))) ISP_colorXformCfgCmdType;


#endif //__COLOR_XFORM40_REG_H__
