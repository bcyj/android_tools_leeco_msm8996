/*============================================================================
   Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __WB_H__
#define __WB_H__

#define WB_GAIN(x) FLOAT_TO_Q(7, (x))

#define WB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1.r_gain, g2.r_gain) && \
  F_EQUAL(g1.g_gain, g2.g_gain) && \
  F_EQUAL(g1.b_gain, g2.b_gain))

#define WB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1.r_gain, 0.0) || \
  F_EQUAL(g1.g_gain, 0.0) || \
  F_EQUAL(g1.b_gain, 0.0))

typedef struct VFE_WhiteBalanceConfigCmdType {
  /* WB Config */
  uint32_t          ch0Gain             : 9;
  uint32_t          ch1Gain             : 9;
  uint32_t          ch2Gain             : 9;
  uint32_t         /* reserved */       : 5;
}__attribute__((packed, aligned(4))) VFE_WhiteBalanceConfigCmdType;

typedef struct VFE_WhiteBalanceRightConfigCmdType {
  /* WB Config */
  uint32_t          ch0GainRight        : 9;
  uint32_t          ch1GainRight        : 9;
  uint32_t          ch2GainRight        : 9;
  uint32_t         /* reserved */       : 5;
}__attribute__((packed, aligned(4))) VFE_WhiteBalanceRightConfigCmdType;

typedef struct  {
  VFE_WhiteBalanceConfigCmdType    VFE_WhiteBalanceCfgCmd;
  VFE_WhiteBalanceRightConfigCmdType VFE_WhiteBalanceRightCfgCmd;
  int8_t enable;
  int8_t update;
  chromatix_manual_white_balance_type awb_gain[2];
  vfe_module_ops_t ops;
  float dig_gain[2];
  int trigger_enable;
  int hw_enable;
} wb_mod_t;

vfe_status_t vfe_wb_ops_init(void *mod);
vfe_status_t vfe_wb_ops_deinit(void *mod);
vfe_status_t vfe_wb_init(int mod_id, void *mod, void *params);
vfe_status_t vfe_wb_update(int mod_id, void *mod, void *params);
vfe_status_t vfe_wb_config(int mod_id, void *mod, void *params);
vfe_status_t vfe_wb_enable(int mod_id, void *mod, void *params,
  int8_t wb_enable, int8_t hw_write);
vfe_status_t vfe_wb_trigger_update(int mod_id, void *mod, void *params);
vfe_status_t vfe_wb_init(int mod_id, void *mod, void *params);
vfe_status_t vfe_wb_set_manual_wb(int mod_id, void *mod, void *params);
vfe_status_t vfe_wb_set_bestshot(int mod_id, void *mod, void *params,
  camera_bestshot_mode_type mode);
vfe_status_t vfe_wb_trigger_enable(int mod_id, void *mod, void *params,
  int enable);
vfe_status_t vfe_wb_tv_validate(int mod_id, void *in, void *op);
vfe_status_t vfe_wb_deinit(int mod_id, void *module, void *params);
vfe_status_t vfe_wb_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__WB_H__
