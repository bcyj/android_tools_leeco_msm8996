/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __COLOR_XFORM_H__
#define __COLOR_XFORM_H__

#include "vfe_util_common.h"


typedef struct VFE_colorXformCfgCmdType {
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
}__attribute__((packed, aligned(4))) VFE_colorXformCfgCmdType;

typedef struct {
  int8_t enable;
  int8_t trigger_update;
  int8_t update;
  VFE_colorXformCfgCmdType VFE_ColorXformCfgCmd;
  int trigger_enable;
  int8_t hw_write;
  vfe_module_ops_t ops;
} color_xform_mod_t;

vfe_status_t vfe_color_xform_ops_init(void* mod_cx);
vfe_status_t vfe_color_xform_ops_deinit(void* mod_cx);
vfe_status_t vfe_color_xform_init(int mod_id, void* mod_cx);
vfe_status_t vfe_color_xform_config(int mod_id, void* mod_cx, void *vparams);
vfe_status_t vfe_color_xform_trigger_update(int mod_id, void* mod_cx,
  void* *vparams);
vfe_status_t vfe_color_xform_update(int* mod_id, void* mod_cx, void* vparams);
vfe_status_t vfe_color_xform_enable(int mod_id, void* mod_cx, void* vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_color_xform_trigger_enable(int mod_id, void* mod_cx, void* vparams,
  int enable);
#endif //__COLOR_XFORM_
