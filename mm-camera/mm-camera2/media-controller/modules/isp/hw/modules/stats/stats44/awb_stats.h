/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __AWB_STATS_H__
#define __AWB_STATS_H__

#include "../isp_stats.h"
#include "awb_stats_reg.h"

isp_ops_t *awb_stats44_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif //__AWB_STATS_H__
