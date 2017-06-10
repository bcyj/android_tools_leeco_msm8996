/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __RS_STATS32_H__
#define __RS_STATS32_H__

#include "../isp_stats.h"
#include "rs_stats_reg.h"

typedef struct
{
  float   row_sum_hor_Loffset_ratio;  // default=0;
  float   row_sum_ver_Toffset_ratio;  //default=0;
  float   row_sum_hor_Roffset_ratio;  // default=0;
  float   row_sum_ver_Boffset_ratio;  //default=0;
  unsigned char   row_sum_V_subsample_ratio; //1 to 4, int
  unsigned char   row_sum_H_subsample_ratio;
  uint32_t rs_max_v_rgns;
  uint32_t rs_max_h_rgns;
  uint32_t shift_bits;
} rs_stat_config_type_t;

isp_ops_t *rs_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type);

#endif /*__RS_STATS32_H__*/
