/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CHROMA_SUBSAMPLE32_H__
#define __CHROMA_SUBSAMPLE32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "chroma_subsample32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "chromatix.h"

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_ChromaSubsampleConfigCmdType reg_cmd;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t enable;
} isp_chroma_ss_mod_t;

#endif //__CHROMA_SUBSAMPLE32_H__
