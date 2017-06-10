/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __BAYER_EXPOSURE_H__
#define __BAYER_EXPOSURE_H__

#include "../isp_stats.h"
#include "be_stats_reg.h"

#define BE_GRID_H        (32)
#define BE_GRID_V        (24)
#define PIX_VALUE_OFFSET (1)

isp_ops_t *be_stats_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__BAYER_EXPOSURE_H__*/
