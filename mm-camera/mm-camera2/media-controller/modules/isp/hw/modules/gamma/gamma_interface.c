/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "isp_pipeline.h"
#include "isp_hw_module_ops.h"

extern isp_ops_t *gamma40_open(uint32_t version);
extern isp_ops_t *gamma32_open(uint32_t version);

isp_ops_t *ISP_MOD_GAMMA_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return gamma40_open(version);
  case ISP_VERSION_32:
    return gamma32_open(version);
  default:
    return NULL;
  }

  return NULL;
}
