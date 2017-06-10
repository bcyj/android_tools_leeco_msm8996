/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __RS_STATS_H__
#define __RS_STATS_H__

/* Stats RS Config */
#ifndef VFE_31
typedef struct VFE_StatsRs_CfgType {
  /*  VFE_STATS_RS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset          :   13;
  uint32_t      /* reserved */        :    3;
  uint32_t        rgnVOffset          :   12;
  uint32_t        shiftBits           :    3;
  uint32_t      /* reserved */        :    1;

  /*  VFE_STATS_RS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth            :   13;
  uint32_t        rgnHeight           :    2;
  uint32_t      /* reserved */        :    1;
  uint32_t        rgnVNum             :   10;
  uint32_t      /* reserved */        :    6;
}__attribute__((packed, aligned(4))) VFE_StatsRs_CfgType;
#else
typedef struct VFE_StatsRs_CfgType {
  /*  VFE_STATS_RS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset          :   12;
  uint32_t      /* reserved */        :    4;
  uint32_t        rgnVOffset          :   12;
  uint32_t        shiftBits           :    3;
  uint32_t      /* reserved */        :    1;

  /*  VFE_STATS_RS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth            :   12;
  uint32_t        rgnHeight           :    2;
  uint32_t      /* reserved */        :    2;
  uint32_t        rgnVNum             :   10;
  uint32_t      /* reserved */        :    6;
}__attribute__((packed, aligned(4))) VFE_StatsRs_CfgType;
#endif

typedef struct {
  VFE_StatsRs_CfgType rs_stats_cmd;
  int enable;
  vfe_module_ops_t ops;
  int8_t use_hal_buf;
}rs_stats_t;

#ifndef VFE_31
/* Stats CS Config */
typedef struct VFE_StatsCs_CfgType {
  /*  VFE_STATS_CS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /*  VFE_STATS_CS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth              :    2;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnHeight             :   12;
  uint32_t        rgnHNum               :   11;
  uint32_t      /* reserved */          :    5;
}__attribute__((packed, aligned(4))) VFE_StatsCs_CfgType;
#else
typedef struct VFE_StatsCs_CfgType {
  /*  VFE_STATS_CS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   12;
  uint32_t      /* reserved */          :    4;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /*  VFE_STATS_CS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth              :    2;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnHeight             :   12;
  uint32_t        rgnHNum               :   10;
  uint32_t      /* reserved */          :    6;
}__attribute__((packed, aligned(4))) VFE_StatsCs_CfgType;
#endif

typedef struct {
  VFE_StatsCs_CfgType cs_stats_cmd;
  int enable;
  vfe_module_ops_t ops;
  int8_t use_hal_buf;
}cs_stats_t;

vfe_status_t vfe_rs_stats_config(int mod_id, void *stats_rs, void *vparams);
vfe_status_t vfe_rs_stats_config(int mod_id, void *stats_rs, void *vparams);
vfe_status_t vfe_rs_stats_enable(int mod_id, void *stats_rs, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_cs_stats_config(int mod_id, void *stats_cs, void *vparams);
vfe_status_t vfe_cs_stats_enable(int mod_id, void *stats_cs, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_rs_stats_ops_init(void *mod);
vfe_status_t vfe_rs_stats_ops_deinit(void *mod);
vfe_status_t vfe_cs_stats_ops_init(void *mod);
vfe_status_t vfe_cs_stats_ops_deinit(void *mod);

#endif /*__RS_STATS_H__*/
