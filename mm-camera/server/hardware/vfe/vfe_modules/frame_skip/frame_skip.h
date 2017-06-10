/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __FRAME_SKIP_H__
#define __FRAME_SKIP_H__
#include "vfe_util_common.h"

/*  Frame Skip Config Command */
typedef struct VFE_FrameSkipConfigCmdType {
  /* new */
  /* Frame Drop Enc (output2) */
  uint32_t output2YPeriod               : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output2CbCrPeriod            : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output2YPattern              : 32;
  uint32_t output2CbCrPattern           : 32;
  /* Frame Drop View (output1) */
  uint32_t output1YPeriod               : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output1CbCrPeriod            : 5;
  uint32_t /* reserved */               : 27;
  uint32_t output1YPattern              : 32;
  uint32_t output1CbCrPattern           : 32;
}__attribute__((packed, aligned(4))) VFE_FrameSkipConfigCmdType;

typedef struct {
 VFE_FrameSkipConfigCmdType frame_skip_cmd;
 VFE_FrameSkipConfigCmdType ext_frame_skip_cmd;
 vfe_module_ops_t ops;
 uint8_t fs_change;
 int8_t fs_enable;
}frame_skip_mod_t;

vfe_status_t vfe_frame_skip_ops_init(void *mod);
vfe_status_t vfe_frame_skip_ops_deinit(void *mod);
vfe_status_t vfe_frame_skip_init(int mod_id, void *mod, void *parm);
vfe_status_t vfe_frame_skip_config(int mod_id, void *mod_fs, void *parm);
vfe_status_t vfe_frame_skip_enable(int mod_id, void *mod, void *parm,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_frame_skip_deinit(int mod_id, void *mod, void *parm);
vfe_status_t vfe_frame_skip_config_pattern(int mod_id, void *mod_fs,
  void *parm);
vfe_status_t vfe_frame_skip_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__FRAME_SKIP_H__
