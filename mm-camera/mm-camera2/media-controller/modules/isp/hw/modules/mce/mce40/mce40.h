/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MCE40_H__
#define __MCE40_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "mce40_reg.h"
#include "chromatix.h"

typedef struct{
  ISP_MCE_MIX_ConfigCmdType_1 mce_mix_cmd_1;
  ISP_MCE_MIX_ConfigCmdType_2 mce_mix_cmd_2;
  ISP_MCE_ConfigCmdType mce_cmd;
  ISP_MCE_ConfigCmdType applied_mce_cmd;
  int cnt;
  int fd;
  uint8_t mce_trigger;
  uint8_t mce_trigger_enable;
  uint8_t mce_update;
  uint8_t mce_enable;
  float prev_lux_idx;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_mce_mod_t;

#endif //__MCE40_H_
