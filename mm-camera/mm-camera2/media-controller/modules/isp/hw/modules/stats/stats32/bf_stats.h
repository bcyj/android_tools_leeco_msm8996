/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_FOCUS_V32_H__
#define __BAYER_FOCUS_V32_H__

#include "../isp_stats.h"
#include "bf_stats_reg.h"

isp_ops_t *bf_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);
isp_ops_t *bf_stats33_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__BAYER_FOCUS_V32_H__*/
