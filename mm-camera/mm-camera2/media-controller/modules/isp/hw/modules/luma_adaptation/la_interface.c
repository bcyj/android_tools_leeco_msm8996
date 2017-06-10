/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "isp_pipeline.h"
#include "isp_hw_module_ops.h"

extern isp_ops_t *la40_open(uint32_t version);
extern isp_ops_t *la32_open(uint32_t version);

/** ISP_MOD_LA_open
 *
 * DESCRIPTION:
 *
 **/
isp_ops_t *ISP_MOD_LA_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return la40_open(version);
  case ISP_VERSION_32:
    return la32_open(version);
  default:
    return NULL;
  }

  return NULL;
}
