/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AWB_STATS_H__
#define __AWB_STATS_H__

typedef struct VFE_StatsAwb_CfgCmdType {
  /*  VFE_STATS_AWB_RGN_OFFSET_CFG  */
#ifndef VFE_31
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    1;
  uint32_t      /* reserved */          :    3;
#else
  uint32_t      /* reserved */          :   28;
  uint32_t        shiftBits             :    4;
#endif
  /*  VFE_STATS_AWB_RGN_SIZE_CFG  */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHeight             :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;

  /*  VFE_STATS_AWB_SGW_CFG */
  uint32_t        yMax                  :    8;
  uint32_t        yMin                  :    8;
  uint32_t      /* reserved */          :   16;

  /*  VFE_STATS_AWB_AGW_CFG_0 */
  int32_t        c1                    :   12;
  int32_t      /* reserved */          :    4;
  int32_t        c2                    :   12;
  int32_t      /* reserved */          :    4;

  /*  VFE_STATS_AWB_AGW_CFG_1 */
  int32_t        c3                    :   12;
  int32_t      /* reserved */          :    4;
  int32_t        c4                    :   12;
  int32_t      /* reserved */          :    4;

  /*  VFE_STATS_AWB_AGW_CFG_2 */
  int32_t        m1                    :   8;
  int32_t        m2                    :   8;
  int32_t        m3                    :   8;
  int32_t        m4                    :   8;

  /*  VFE_STATS_AWB_GX_CFG_0 */
  uint32_t        t1                    :   8;
  uint32_t        t2                    :   8;
  uint32_t        t3                    :   8;
  uint32_t        t6                    :   8;

  /*  VFE_STATS_AWB_GX_CFG_1 */
  uint32_t        t4                    :  10;
  uint32_t      /* reserved */          :   2;
  uint32_t        mg                    :   9;
  uint32_t      /* reserved */          :   3;
  uint32_t        t5                    :   8;
}__attribute__((packed, aligned(4))) VFE_StatsAwb_CfgCmdType;

typedef struct {
  VFE_StatsAwb_CfgCmdType VFE_StatsAwb_ConfigCmd;
  int enable;
  int update;
  vfe_module_ops_t ops;
  int8_t use_hal_buf;
}awb_stats_t;

vfe_status_t vfe_awb_stats_init(int mod_id, void *stats_wb, void *vparams);
vfe_status_t vfe_awb_stats_enable(int mod_id, void *stats_wb, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_awb_stats_config(int mod_id, void *stats_wb, void *vparams);
vfe_status_t vfe_awb_stats_update(int mod_id, void *stats_wb, void *vparams);
vfe_status_t vfe_awb_stats_trigger_update(int mod_id, void *stats_wb,
  void *vparams);
vfe_status_t vfe_awb_stats_get_shiftbits(awb_stats_t *mod,
  vfe_params_t *vfe_params, uint32_t *p_shiftbits);
vfe_status_t vfe_awb_stats_ops_init(void *mod);
vfe_status_t vfe_awb_stats_ops_deinit(void *mod);
vfe_status_t vfe_awb_stats_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__AWB_STATS_H__
