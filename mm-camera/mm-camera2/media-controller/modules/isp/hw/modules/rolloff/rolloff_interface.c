/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_pipeline.h"
#include "isp_hw_module_ops.h"

extern isp_ops_t *mesh_rolloff40_open(uint32_t version);
extern isp_ops_t *pca_rolloff32_open(uint32_t version);

isp_ops_t *ISP_MOD_ROLLOFF_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return mesh_rolloff40_open(version);
  case ISP_VERSION_32:
    return pca_rolloff32_open(version);
  default:
    return NULL;
  }

  return NULL;
}
