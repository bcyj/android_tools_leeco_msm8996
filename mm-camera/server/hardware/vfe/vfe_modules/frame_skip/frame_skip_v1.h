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
  uint32_t output1Pattern              : 30;
  uint32_t /* reserved */              :  2;
  uint32_t output2Pattern              : 30;
  uint32_t /* reserved */              :  2;
}__attribute__((packed, aligned(4))) VFE_FrameSkipConfigCmdType;

typedef struct {
 VFE_FrameSkipConfigCmdType frame_skip_cmd;
 int8_t fs_enable;
 VFE_FrameSkipConfigCmdType ext_frame_skip_cmd;
 uint8_t fs_change;
}frame_skip_mod_t;

vfe_status_t vfe_frame_skip_init(frame_skip_mod_t *mod, vfe_params_t *parm);
vfe_status_t vfe_frame_skip_config(frame_skip_mod_t *mod, vfe_params_t *p_obj);
vfe_status_t vfe_frame_skip_enable(frame_skip_mod_t *mod, vfe_params_t *p_obj,
  int8_t enable, int8_t hw_write);
#endif //__FRAME_SKIP_H__
