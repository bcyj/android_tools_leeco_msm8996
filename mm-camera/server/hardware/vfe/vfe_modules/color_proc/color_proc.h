
/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __COL_PROC_H__
#define __COL_PROC_H__

#include "vfe_util_common.h"
#include "camera.h"
#include "vfe_test_vector.h"
#include "colorcorrect.h"
#include "chroma_enhan.h"

/* Color Proc Config */
typedef struct VFE_ColorProcCfgCmdType {
  VFE_ColorCorrectionCfgCmdType colorCorrect;
  VFE_Chroma_Enhance_CfgCmdType chromaEnhan;
}__attribute__((packed, aligned(4))) VFE_ColorProcCfgCmdType;

typedef struct {
  VFE_ColorProcCfgCmdType VFE_PrevColorProcCmd;
  VFE_ColorProcCfgCmdType VFE_SnapColorProcCmd;
  color_correct_mod_t cc_mod;
  chroma_enhan_mod_t cv_mod;
} color_proc_mod_t;

vfe_status_t vfe_color_proc_init(color_proc_mod_t* mod,
  vfe_params_t* parms);
vfe_status_t vfe_color_proc_trigger_update(color_proc_mod_t* mod,
  vfe_params_t* parms);
vfe_status_t vfe_color_proc_enable(color_proc_mod_t* mod,
  vfe_params_t *params, int8_t enable, int8_t hw_write);
vfe_status_t vfe_color_proc_config(color_proc_mod_t* mod,
  vfe_params_t* parms);
vfe_status_t vfe_color_proc_update(color_proc_mod_t* mod,
  vfe_params_t* parms);
vfe_status_t vfe_color_proc_set_effect(color_proc_mod_t* mod,
  vfe_params_t* parms, vfe_effects_type_t type);
vfe_status_t vfe_color_proc_set_bestshot(color_proc_mod_t* mod,
  vfe_params_t* params, camera_bestshot_mode_type mode);
vfe_status_t vfe_color_proc_reload_params(color_proc_mod_t* mod,
  vfe_params_t* params);
vfe_status_t vfe_color_proc_trigger_enable(color_proc_mod_t* mod,
  vfe_params_t* params, int enable);
#endif //__COL_PROC_H__
