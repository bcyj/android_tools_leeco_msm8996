/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __BAYER_GRID_V4_H__
#define __BAYER_GRID_V4_H__

typedef struct VFE_StatsBg_CfgCmdType {
  /*  VFE_STATS_BG_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BG_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHeight             :    8;
  uint32_t        rgnHNum               :    7;
  uint32_t        rgnVNum               :    6;
  uint32_t      /* reserved */          :    1;
  /* VFE_STATS_BG_THRESHOLD_CFG */
  uint32_t        rMax                  :    8;
  uint32_t        grMax                 :    8;
  uint32_t        bMax                  :    8;
  uint32_t        gbMax                 :    8;
}__attribute__((packed, aligned(4))) VFE_StatsBg_CfgCmdType;

typedef struct {
  VFE_StatsBg_CfgCmdType bg_stats_cmd;
  vfe_module_ops_t ops;
  int8_t enable;
} bg_stats_t;

vfe_status_t vfe_bg_stats_init(int mod_id, void *stats_bg, void *vparams);
vfe_status_t vfe_bg_stats_enable(int mod_id, void *stats, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_bg_stats_config(int mod_id, void *stats, void *vparams);
vfe_status_t vfe_bg_stats_ops_init(void *mod);
vfe_status_t vfe_bg_stats_ops_deinit(void *mod);

#endif /*__BAYER_GRID_H__*/
