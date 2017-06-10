/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __RS_STATS_V32_H__
#define __RS_STATS_V32_H__

#include "../isp_stats.h"
#include "cs_stats_reg.h"

typedef struct
{
  float   col_sum_hor_Loffset_ratio;  //default=0
  float   col_sum_ver_Toffset_ratio;  // default=0;
  float   col_sum_hor_Roffset_ratio;  //default=0
  float   col_sum_ver_Boffset_ratio;  // default=0;
  unsigned char   col_sum_H_subsample_ratio; //2 to 4, int
  unsigned char   col_sum_V_subsample_ratio; //2 to 4, int
  uint32_t cs_max_v_rgns;
  uint32_t cs_max_h_rgns;
  uint32_t shift_bits;
} cs_stat_config_type_t;

isp_ops_t *cs_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__RS_STATS_V32_H__*/
