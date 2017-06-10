/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"

extern isp_ops_t *chroma_suppress40_open(uint32_t version);
extern isp_ops_t *chroma_suppress32_open(uint32_t version);

/** ISP_MOD_CHROMA_SUPPRESS_open:
 *
 *    @version:
 *
 **/
isp_ops_t *ISP_MOD_CHROMA_SUPPRESS_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return chroma_suppress40_open(version);

  case ISP_VERSION_32:
    return chroma_suppress32_open(version);

  default:
    return NULL;
  }

  return NULL;
}
