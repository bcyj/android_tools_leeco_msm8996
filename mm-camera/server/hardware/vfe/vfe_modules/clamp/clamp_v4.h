/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CLAMP_V4_H__
#define __CLAMP_V4_H__
#include "vfe_util_common.h"

/* Output Clamp Config Command */
typedef struct VFE_ClampConfigCmdType {
  /* Output Clamp Maximums */
  uint32_t  yChanMax         :  8;
  uint32_t  cbChanMax        :  8;
  uint32_t  crChanMax        :  8;
  uint32_t  /* reserved */   :  8;
  /* Output Clamp Minimums */
  uint32_t  yChanMin         :  8;
  uint32_t  cbChanMin        :  8;
  uint32_t  crChanMin        :  8;
  uint32_t  /* reserved */   :  8;
}__attribute__((packed, aligned(4))) VFE_ClampConfigCmdType;

/* Output Clamp Config Command */
typedef struct VFE_OutputClampConfigCmdType {
VFE_ClampConfigCmdType enc_clamp_config;
VFE_ClampConfigCmdType view_clamp_config;
}__attribute__((packed, aligned(4))) VFE_OutputClampConfigCmdType;

typedef struct {
  VFE_OutputClampConfigCmdType clamp_cfg_cmd;
  vfe_module_ops_t ops;
  int8_t clamp_enable;
}clamp_mod_t;

vfe_status_t vfe_clamp_ops_init(void *mod);
vfe_status_t vfe_clamp_ops_deinit(void *mod);
vfe_status_t vfe_clamp_init(int mod_id, void *mod, void* parms);
vfe_status_t vfe_clamp_config(int mod_id, void *mod_clamp, void* parm);
vfe_status_t vfe_clamp_enable(int mod_id, void *mod, void* parms,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_clamp_deinit(int mod_id, void *mod, void* parms);

#endif// __CLAMP_H__
