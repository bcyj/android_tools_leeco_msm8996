/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __VFE_MODULE_OPS_H__
#define __VFE_MODULE_OPS_H__

#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <media/msm_isp.h>
#include <media/msm_camera.h>
#include "vfe_interface.h"
#include "vfe_tgtcommon.h"
#include "chromatix.h"
#include "camera_dbg.h"

#define VFE_SUB_MODULE_NOT_USED        0

typedef struct {
  vfe_status_t (*init) (int module_id, void *module_ctrl, void *vfe_parms);
  vfe_status_t (*update) (int module_id, void *module_ctrl, void *vfe_parms);
  vfe_status_t (*trigger_update) (int module_id, void *module_ctrl,
    void *vfe_parms);
  vfe_status_t (*config) (int module_id, void *module_ctrl, void *vfe_parms);
  vfe_status_t (*enable) (int module_id, void *module_ctrl, void *params,
    int8_t enable, int8_t hw_write);
  vfe_status_t (*reload_params) (int module_id, void *module_ctrl,
    void *vfe_parms);
  vfe_status_t (*trigger_enable) (int module_id, void *module_ctrl,
    void *vfe_parms, int enable);
  vfe_status_t (*test_vector_validate) (int module_id, void* input,
    void* output);
  vfe_status_t (*set_effect)(int module_id, void *module_ctrl, void *parms,
    vfe_effects_type_t type);
  vfe_status_t (*set_spl_effect)(int module_id, void *module_ctrl, void *parms,
    vfe_spl_effects_type effects);
  vfe_status_t (*set_manual_wb)(int module_id, void *module_ctrl, void *parms);
  vfe_status_t (*set_bestshot) (int module_id, void *module_ctrl, void *vfe_parms,
    camera_bestshot_mode_type mode);
  vfe_status_t (*set_contrast)(int module_id, void *module_ctrl, void *vparms,
    int32_t contrast);
  vfe_status_t (*deinit)(int module_id, void *module_ctrl, void *params);
} vfe_module_ops_t;

#endif //__VFE_MODULE_OPS_H__
