/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FRAME_SKIP32_H__
#define __FRAME_SKIP32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "frame_skip32_reg.h"
#include "chromatix.h"

typedef struct {
  ISP_FrameSkipConfigCmdType frame_skip_cmd;
  ISP_FrameSkipConfigCmdType ext_frame_skip_cmd;
  int fd;
  uint8_t fs_change;
  int8_t fs_enable;
  int8_t fs_trigger_enable;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_frame_skip_mod_t;

#endif //__FRAME_SKIP32_H__
