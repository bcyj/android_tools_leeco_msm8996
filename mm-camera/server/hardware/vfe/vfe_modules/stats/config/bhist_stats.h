/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __BAYER_HIST_H__
#define __BAYER_HIST_H__

typedef struct VFE_StatsBhist_CfgCmdType {
  /*  VFE_STATS_BHIST_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BHIST_RGN_SIZE_CFG */
  uint32_t        rgnHNum               :    12;
  uint32_t        rgnVNum               :    11;
  uint32_t      /* reserved 23:31 */    :     9;
}__attribute__((packed, aligned(4))) VFE_StatsBhist_CfgCmdType;

typedef struct {
  VFE_StatsBhist_CfgCmdType bhist_stats_cmd;
  vfe_module_ops_t ops;
  int8_t enable;
}bhist_stats_t;

typedef struct {
  bg_stats_t bg_stats;
  bf_stats_t bf_stats;
  bhist_stats_t bhist_stats;
} bayer_stats_t;

vfe_status_t vfe_bhist_stats_init(int mod_id, void *stats_bhist, void *vparams);
vfe_status_t vfe_bhist_stats_enable(int mod_id, void *stats_bhist, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_bhist_stats_config(int mod_id, void *stats_bhist, void *vparams);
vfe_status_t vfe_bhist_stats_ops_init(void *mod);
vfe_status_t vfe_bhist_stats_ops_deinit(void *mod);
#endif /*__BAYER_HIST_H__*/
