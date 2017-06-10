/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ROLLOFF_H__
#define __ROLLOFF_H__

#include "mesh_rolloff.h"
#include "pca_rolloff.h"
#include "mesh_rolloff_v4.h"
#include "vfe_util_common.h"

/*===========================================================================
 * Common Roll-off data structures
 *==========================================================================*/
typedef struct {
  int rolloff_enable;
  uint32_t rolloff_update;
  vfe_rolloff_info_t *rolloff_tbls;
  mesh_rolloff_mod_t mesh_ctrl;
  pca_rolloff_mod_t pca_ctrl;
  mesh_rolloff_V4_mod_t mesh_v4_ctrl;
  int hw_enable_cmd;
#ifdef VFE_2X
  int8_t vfe_reconfig;
#endif
  vfe_module_ops_t ops;
}rolloff_mod_t;

/*===========================================================================
 *  RollOff Interface APIs
 *==========================================================================*/
vfe_status_t vfe_rolloff_ops_init(void *mod);
vfe_status_t vfe_rolloff_ops_deinit(void *mod);
vfe_status_t vfe_rolloff_enable(int mod_id, void *module, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_rolloff_init(int mod_id, void *module, void *vparams);
vfe_status_t vfe_rolloff_deinit(int mod_id, void *module, void *vparams);
vfe_status_t vfe_rolloff_config(int mod_id, void *module, void *vparams);
vfe_status_t vfe_rolloff_update(int mod_id, void *module, void *vparams);
vfe_status_t vfe_rolloff_trigger_update(int mod_id, void *module,
  void *vparams);
vfe_status_t vfe_rolloff_tv_validate(int mod_id, void *ip, void *op);
/*===========================================================================
 *  RollOff Interface APIs for EzTune
 *==========================================================================*/
vfe_status_t vfe_rolloff_trigger_enable(int mod_id, void *module, void *vparams,
  int enable);
vfe_status_t vfe_rolloff_reload_params(int mod_id, void *module, void *vparams);
vfe_status_t vfe_rolloff_plugin_update(int module_id, void *mod,
  void *vparams);
#endif /* __ROLLOFF_H__ */
