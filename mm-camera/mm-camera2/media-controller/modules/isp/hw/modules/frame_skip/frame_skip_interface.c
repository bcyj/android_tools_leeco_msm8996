/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "isp_pipeline.h"
#include "isp_hw_module_ops.h"

extern isp_ops_t *frame_skip32_open(uint32_t version);

isp_ops_t *ISP_MOD_FRAME_SKIP_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return NULL;
  case ISP_VERSION_32:
    /* todo: return frame_skip32_open(version); */
    return NULL;
  default:
    return NULL;
  }

  return NULL;
}
