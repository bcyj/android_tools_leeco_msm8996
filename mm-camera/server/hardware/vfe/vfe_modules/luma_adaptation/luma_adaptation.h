/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __LUMA_ADAPTATION_H__
#define __LUMA_ADAPTATION_H__

#include "vfe_test_vector.h"

#define VFE_LA_TABLE_LENGTH           64

typedef struct VFE_LABankSel {
  /* LA Config */
  uint32_t      lutBankSelect               : 1;
  uint32_t     /* reserved */               :31;
}__attribute__((packed, aligned(4))) VFE_LABankSel;


typedef struct VFE_LA_TblEntry {
  /* Luma adaptation table entries. */
  int16_t table[VFE_LA_TABLE_LENGTH];
} VFE_LA_TblEntry;

typedef struct VFE_LA_ConfigCmdType {
  /* LA Config */
  VFE_LABankSel CfgCmd;
  VFE_LA_TblEntry  TblEntry;
} VFE_LA_ConfigCmdType;

/* LA Enable/Disable on fly */
typedef struct VFE_LumaAdapEnCmdType {
  uint32_t /*reserved*/     :16;
  uint32_t luma_enable      :1;
  uint32_t /*reserved*/     :15;
} VFE_LumaAdapEnCmdType;

typedef struct {
  VFE_LA_ConfigCmdType la_vf_cmd;
  VFE_LA_ConfigCmdType la_snap_cmd;
  int32_t LUT_Yratio[VFE_LA_TABLE_LENGTH]; /* LUT for LA*/
  int32_t solarize_la_tbl[VFE_LA_TABLE_LENGTH]; /* LUT for LA*/
  int32_t posterize_la_tbl[VFE_LA_TABLE_LENGTH]; /* LUT for LA*/
  int32_t *pLUT_Yratio;
  // used for histogram calculation
  la_8k_type   la_config;
  int hw_enable;
  uint8_t la_trigger;
  uint8_t la_update;
  uint8_t la_enable;
  uint32_t num_pixels;
  vfe_module_ops_t ops;
}la_mod_t;


vfe_status_t vfe_la_ops_init(void *mod);
vfe_status_t vfe_la_ops_deinit(void *mod);
vfe_status_t vfe_la_enable(int mod_id, void *module, void* vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_la_init(int mod_id, void *la_mod, void *vparams);
vfe_status_t vfe_la_update(int mod_id, void *module, void *vparams);
vfe_status_t vfe_la_config(int mod_id, void *module, void *vparams);
vfe_status_t vfe_la_trigger_update(int mod_id, void *la_mod, void *vparams);
vfe_status_t vfe_la_trigger_enable(int mod_id, void *module, void *vparams,
  int enable);
vfe_status_t vfe_la_tv_validate(int mod_id, void *test_input,
  void *test_output);
vfe_status_t vfe_la_reload_params(int module_id, void *mod,
  void* vfe_parms);
vfe_status_t vfe_la_deinit(int mod_id, void *module, void *params);
vfe_status_t vfe_la_set_bestshot(int mod_id, void* la_mod,
  void* vparams, camera_bestshot_mode_type mode);
vfe_status_t vfe_la_get_table(la_mod_t* la_mod, vfe_params_t* vfe_params,
  vfe_pp_params_t *pp_info);
vfe_status_t vfe_la_set_spl_effect(int mod_id, void* mod_la, void *vparams,
  vfe_spl_effects_type type);
vfe_status_t vfe_la_plugin_update(int module_id, void *mod,
  void *vparams);
#endif
