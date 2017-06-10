/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"

extern isp_ops_t *linearization40_open(uint32_t version);
extern isp_ops_t *linearization32_open(uint32_t version);

/** ISP_MOD_LINEARIZATION_open
 *
 * DESCRIPTION:
 *
 **/
isp_ops_t *ISP_MOD_LINEARIZATION_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return linearization40_open(version);
  case ISP_VERSION_32:
    return linearization32_open(version);
  default:
    return NULL;
  }

  return NULL;
}
