/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __IHIST_STATS_H__
#define __IHIST_STATS_H__

#define IHIST_TABLE_LENGTH 256

#ifndef VFE_31
/* Stats ihist Config*/
typedef struct VFE_StatsIhist_CfgType {
  /*  VFE_STATS_IHIST_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :    13;
  uint32_t        channelSelect         :     3;
  uint32_t        rgnVOffset            :    12;
  uint32_t        shiftBits             :    3;
  uint32_t        siteSelect            :    1;

  /*  VFE_STATS_IHIST_RGN_SIZE_CFG  */
  uint32_t        rgnHNum               :   12;
  uint32_t        rgnVNum               :   11;
  uint32_t      /* reserved */          :    9;
}__attribute__((packed, aligned(4))) VFE_StatsIhist_CfgType;
#else
typedef struct VFE_StatsIhist_CfgType {
  /*  VFE_STATS_IHIST_RGN_OFFSET_CFG  */
  uint32_t        siteSelect            :    1;
  uint32_t      /* reserved */          :    3;
  uint32_t        channelSelect         :    3;
  uint32_t      /* reserved */          :   21;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /*  VFE_STATS_IHIST_RGN_SIZE_CFG  */
  uint32_t        rgnHNum               :   11;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnVNum               :   11;
  uint32_t      /* reserved */          :    9;
}__attribute__((packed, aligned(4))) VFE_StatsIhist_CfgType;
#endif

typedef struct {
  VFE_StatsIhist_CfgType ihist_stats_cmd;
  uint32_t vfe_Ihist_data[256]; /* histogram data from VFE */
  la_8k_type   la_config;
  int enable;
  vfe_module_ops_t ops;
  int8_t use_hal_buf;
}ihist_stats_t;

vfe_status_t vfe_stats_process_hist(void * ctrl);
vfe_status_t vfe_ihist_stats_config(int mod_id, void *stats_ih, void *vparams);
vfe_status_t vfe_ihist_stats_enable(int mod_id, void *stats_ih, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_ihist_stats_get_shiftbits(ihist_stats_t *ihist_stats,
  vfe_params_t *vfe_params, uint32_t *p_shiftbits);
vfe_status_t vfe_ihist_stats_ops_init(void *mod);
vfe_status_t vfe_ihist_stats_ops_deinit(void *mod);

#endif //__IHIST_STATS_H__
