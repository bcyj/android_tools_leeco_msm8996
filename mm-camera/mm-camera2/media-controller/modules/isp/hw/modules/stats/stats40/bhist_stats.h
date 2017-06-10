/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_HIST_H__
#define __BAYER_HIST_H__

#include "../isp_stats.h"
#include "bhist_stats_reg.h"

isp_ops_t *bhist_stats_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__BAYER_HIST_H__*/
