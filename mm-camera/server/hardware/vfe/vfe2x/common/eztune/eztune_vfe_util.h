/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __EZTUNE_VFE_UTIL_H__
#define __EZTUNE_VFE_UTIL_H__
#include "eztune_vfe_diagnostics.h"
#include "gamma.h"
#include "rolloff.h"
#include "black_level_v1.h"
#include "vfe_tgtcommon.h"
#include "asf.h"

vfe_status_t ez_vfe_set(void *ctrl_obj, void *param1, void *param2);
void ez_vfe_diagnostics(void *ctrl_obj);
void ez_vfe_diagnostics_update(void *ctrl_obj);
vfe_status_t ez_vfe_gamma_enable(gamma_mod_t* mod,
  vfe_params_t *params, int8_t g_enable, int8_t hw_write);
vfe_status_t ez_vfe_rolloff_enable(rolloff_mod_t* rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable, int8_t hw_write);
vfe_status_t ez_vfe_color_conversion_enable(chroma_enhan_mod_t *mod,
  vfe_params_t *params, int8_t enable, int8_t hw_write);
vfe_status_t ez_vfe_black_level_enable(black_level_mod_t* mod,
  vfe_params_t* params, int8_t enable, int8_t hw_write);
vfe_status_t ez_vfe_asf_enable(asf_mod_t* asf_ctrl, vfe_params_t* vfe_params,
  int8_t enable, int8_t hw_write);
#endif /* __EZTUNE_VFE_UTIL_H__ */
