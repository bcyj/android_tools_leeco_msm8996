/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __AF_STATS_H__
#define __AF_STATS_H__

#include "../isp_stats.h"
#include "af_stats_reg.h"

isp_ops_t *af_stats_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__AF_STATS_H__*/

