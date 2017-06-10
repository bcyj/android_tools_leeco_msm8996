/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __IHIST_STATS_H__
#define __IHIST_STATS_H__

#include "../isp_stats.h"
#include "ihist_stats_reg.h"

isp_ops_t *ihist_stats44_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__IHIST_STATS_H__*/
