/*============================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_stats.h"

extern isp_ops_t *stats40_open(uint32_t version);
extern isp_ops_t *stats32_open(uint32_t version);

isp_ops_t *ISP_MOD_STATS_open(uint32_t isp_version)
{
  uint32_t version = GET_ISP_MAIN_VERSION(isp_version);
  switch(version) {
#ifdef VFE_40
  case ISP_VERSION_40:
  return stats40_open(version);
#endif /* VFE_40 */
#ifdef VFE_32
  case ISP_VERSION_32:
  return stats32_open(isp_version);
#endif /* VFE_32 */
  default:
  return NULL;
  }
  return NULL;
}
