/*============================================================================
   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AF_STATS_H__
#define __AF_STATS_H__

typedef struct VFE_StatsAf_CfgCmdType {
#ifndef VFE_31
  /*  VFE_STATS_AF_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /* VFE_STATS_AF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :   12;
  uint32_t        rgnHeight             :   11;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;
#else
  /*  VFE_STATS_AF_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   12;
  uint32_t      /* reserved */          :    4;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t      /* reserved */          :    1;

  /* VFE_STATS_AF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :   11;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHeight             :   11;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHNum               :    4;
  uint32_t        rgnVNum               :    4;
#endif
  /*  VFE_STATS_AF_HPF_COEF0 */
  int32_t    a00                       :    5; /* from chromatix */
  int32_t    a02                       :    5; /* from chromatix */
  int32_t    a04                       :    5; /* from chromatix */
  int32_t      /* reserved */          :    1;
  uint32_t    fvMin                     :   11; /* metrixmax */
  uint32_t      /* reserved */          :    4;
  uint32_t    fvMode                    :    1; /* metricSelection */

  /*  VFE_STATS_AF_HPF_COEF1 */
  int32_t    a20                       :   5; /* from chromatix */
  int32_t    a21                       :   5; /* from chromatix */
  int32_t    a22                       :   5; /* from chromatix */
  int32_t    a23                       :   5; /* from chromatix */
  int32_t    a24                       :   5; /* from chromatix */
  uint32_t      /* reserved */          :   7;

}__attribute__((packed, aligned(4))) VFE_StatsAf_CfgCmdType;

typedef struct {
  VFE_StatsAf_CfgCmdType af_stats;
  int8_t enable;
  int8_t af_update;
  vfe_module_ops_t ops;
  int8_t use_hal_buf;
}af_stats_t;

vfe_status_t vfe_af_stats_init(int mod_id, void *af_stats, void *vparams);
vfe_status_t vfe_af_stats_config(int mod_id, void *stats_af, void *vparams);
vfe_status_t vfe_af_stats_enable(int mod_id, void *stats_af, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_af_stats_update(int mod_id, void *stats_af, void *vparams);
vfe_status_t vfe_af_stats_stop(af_stats_t *stats, vfe_params_t *params);
vfe_status_t vfe_af_stats_ops_init(void *mod);
vfe_status_t vfe_af_stats_ops_deinit(void *mod);

#endif /*__AF_STATS_H__*/

