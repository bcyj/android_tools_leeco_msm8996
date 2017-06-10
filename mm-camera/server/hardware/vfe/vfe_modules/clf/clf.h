/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CLF_H__
#define __CLF_H__

typedef enum {
  VFE_CLF_LUMA_CHROMA_DISABLE,
  VFE_CLF_LUMA_ENABLE,
  VFE_CLF_CHROMA_ENABLE,
  VFE_CLF_LUMA_CHROMA_ENABLE,
}vfe_clf_enable_type_t;

#define CLF_CF_COEFF(x) MIN(128, FLOAT_TO_Q(6, (x)))

typedef struct VFE_CLF_Cfg {
  uint32_t       colorconv_enable       :   1;
  uint32_t    /* reserved */            :  15;
  uint32_t       pipe_flush_cnt         :  13;
  uint32_t       pipe_flush_ovd         :   1;
  uint32_t       flush_halt_ovd         :   1;
  uint32_t    /* reserved */            :   1;

}__attribute__((packed, aligned(4))) VFE_CLF_Cfg;

typedef struct VFE_CLF_Luma_Cfg {
  /* CLF Luma cfg 0 */
  uint32_t       cutoff_1               :  12;
  uint32_t    /* reserved */            :   4;
  uint32_t       cutoff_2               :  12;
  uint32_t    /* reserved */            :   4;

  /* CLF Luma cfg 1 */
  uint32_t       cutoff_3               :  12;
  uint32_t    /* reserved */            :  20;

  /* CLF Luma cfg 2 */
  uint32_t       mult_neg               :  12;
  uint32_t    /* reserved */            :   4;
  uint32_t       mult_pos               :  12;
  uint32_t    /* reserved */            :   4;
}__attribute__((packed, aligned(4))) VFE_CLF_Luma_Cfg;

typedef struct VFE_CLF_Luma_Pos_Lut {
  /* CLF Luma POS LUT */
  int32_t       pos_lut0             : 12;
  int32_t     /* reserved */         :  4;
  int32_t       pos_lut1             : 12;
  int32_t     /* reserved */         :  4;
}__attribute__((packed, aligned(4))) VFE_CLF_Luma_Pos_Lut;

typedef struct VFE_CLF_Luma_Neg_Lut {
  /* CLF Luma Neg LUT */
  int32_t       neg_lut0             : 12;
  int32_t     /* reserved */         :  4;
  int32_t       neg_lut1             : 12;
  int32_t     /* reserved */         :  4;
}__attribute__((packed, aligned(4))) VFE_CLF_Luma_Neg_Lut;

typedef struct VFE_CLF_Chroma_Coeff {
  /* CLF Chroma coef 0 */
  uint32_t       v_coeff0               :   7;
  uint32_t    /* reserved */            :   1;
  uint32_t       v_coeff1               :   6;
  uint32_t    /* reserved */            :  18;

  /* CLF Chroma coef 1 */
  uint32_t       h_coeff0               :   7;
  uint32_t    /* reserved */            :   1;
  uint32_t       h_coeff1               :   6;
  uint32_t    /* reserved */            :   2;
  uint32_t       h_coeff2               :   6;
  uint32_t    /* reserved */            :   2;
  uint32_t       h_coeff3               :   6;
  uint32_t    /* reserved */            :   2;
} __attribute__((packed, aligned(4))) VFE_CLF_Chroma_Coeff;

/* Luma Update Command  */
typedef struct VFE_CLF_Luma_Update_CmdType {
  VFE_CLF_Luma_Cfg      luma_Cfg;
  VFE_CLF_Luma_Pos_Lut  luma_pos_LUT[8];
  VFE_CLF_Luma_Neg_Lut  luma_neg_LUT[4];
}__attribute__((packed, aligned(4))) VFE_CLF_Luma_Update_CmdType;

/* Luma Update Command  */
typedef struct VFE_CLF_Chroma_Update_CmdType {
  VFE_CLF_Chroma_Coeff  chroma_coeff;
}__attribute__((packed, aligned(4))) VFE_CLF_Chroma_Update_CmdType;

/* CLF config Command  */
typedef struct VFE_CLF_CmdType {
  VFE_CLF_Cfg                    clf_cfg;
  VFE_CLF_Luma_Update_CmdType    lumaUpdateCmd;
  VFE_CLF_Chroma_Update_CmdType  chromaUpdateCmd;
}__attribute__((packed, aligned(4))) VFE_CLF_CmdType;

typedef struct {
  Chroma_filter_type cf_param;
  chromatix_adaptive_bayer_filter_data_type2 lf_param;
}clf_params_t;

typedef struct {
  int8_t cf_enable;
  int8_t lf_enable;
  int8_t cf_update;
  int8_t lf_update;
  int8_t cf_enable_trig;
  int8_t lf_enable_trig;
  VFE_CLF_CmdType VFE_PrevCLF_Cmd;
  VFE_CLF_CmdType VFE_SnapshotCLF_Cmd;
  float cur_cf_ratio;
  clf_params_t clf_params;
  vfe_op_mode_t cur_mode_luma;
  trigger_ratio_t cur_lf_trig_ratio;
  vfe_op_mode_t cur_mode_chroma;
  int trigger_enable;
  int reload_params;
  int hw_enable;
  vfe_module_ops_t ops;
}clf_mod_t;

vfe_status_t vfe_clf_ops_init(void *mod);
vfe_status_t vfe_clf_ops_deinit(void *mod);
vfe_status_t vfe_clf_init(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_clf_update(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_clf_trigger_update(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_clf_config(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_clf_enable(int module_id, void *mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_clf_reload_params(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_clf_trigger_enable(int module_id, void *mod, void* vfe_parms,
  int enable);
vfe_status_t vfe_clf_tv_validate(int module_id, void* input, void* output);
vfe_status_t vfe_clf_deinit(int mod_id, void *module, void *params);
vfe_status_t vfe_clf_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode);
vfe_status_t vfe_clf_plugin_update(int module_id, void *mod,
  void *vparams);

#endif //__CLF_H__
