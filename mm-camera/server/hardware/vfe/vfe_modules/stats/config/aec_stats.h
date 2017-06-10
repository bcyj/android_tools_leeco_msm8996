/*============================================================================
   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AEC_STATS_H__
#define __AEC_STATS_H__

#include "vfe_util_common.h"

#ifndef VFE_31
typedef struct VFE_StatsAe_CfgCmdType {
  /*  VFE_STATS_AE_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    4;

  /*  VFE_STATS_AE_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    10;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnHeight             :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;
}__attribute__((packed, aligned(4))) VFE_StatsAe_CfgCmdType;
#else
typedef struct VFE_StatsAe_CfgCmdType {
  /*  VFE_STATS_AE_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   12;
  uint32_t      /* reserved */          :    4;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    4;

  /*  VFE_STATS_AE_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHeight             :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;
}__attribute__((packed, aligned(4))) VFE_StatsAe_CfgCmdType;
#endif

typedef struct {
  VFE_StatsAe_CfgCmdType aec_stats_cmd;
  int8_t enable;
  vfe_module_ops_t ops;
  int8_t use_hal_buf;
}aec_stats_t;

vfe_status_t vfe_aec_stats_init(int mod_id, void *stats_aec,
  void *vparams);
vfe_status_t vfe_aec_stats_config(int mod_id, void *stats_aec,
  void *vparams);
vfe_status_t vfe_aec_stats_enable(int mod_id, void *stats_aec,
  void *vparams, int8_t enable, int8_t hw_write);
vfe_status_t vfe_aec_stats_ops_init(void *mod);
vfe_status_t vfe_aec_stats_ops_deinit(void *mod);

#endif /*__AEC_STATS_H__*/
