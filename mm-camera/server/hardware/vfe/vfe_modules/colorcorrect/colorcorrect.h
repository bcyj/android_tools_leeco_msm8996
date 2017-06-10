/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __COL_CORR_H__
#define __COL_CORR_H__

#define CC_COEFF(x, q) (FLOAT_TO_Q((q), (x)))

/* Color Correction Config */
#ifndef VFE_2X
typedef struct VFE_ColorCorrectionCfgCmdType {
  /* Color Corr. Coefficient 0 Config */
  int32_t      C0                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 1 Config */
  int32_t      C1                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 2 Config */
  int32_t      C2                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 3 Config */
  int32_t      C3                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 4 Config */
  int32_t      C4                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 5 Config */
  int32_t      C5                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 6 Config */
  int32_t      C6                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 7 Config */
  int32_t      C7                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Coefficient 8 Config */
  int32_t      C8                          :12;
  int32_t     /* reserved */               :20;
  /* Color Corr. Offset 0 Config */
  int32_t      K0                          :11;
  int32_t     /* reserved */               :21;
  /* Color Corr. Offset 1 Config */
  int32_t      K1                          :11;
  int32_t     /* reserved */               :21;
  /* Color Corr. Offset 2 Config */
  int32_t      K2                          :11;
  int32_t     /* reserved */               :21;
  /* Color Corr. Coefficient Q Config */
  uint32_t      coefQFactor                 : 2;
  uint32_t     /* reserved */               :30;
}__attribute__((packed, aligned(4))) VFE_ColorCorrectionCfgCmdType;
#else
typedef struct VFE_ColorCorrectionCfgCmdType {
  /* Color Corr. Coefficient 0 Config */
  int32_t      C0                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 1 Config */
  int32_t      C1                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 2 Config */
  int32_t      C2                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Offset 0 Config */
  int32_t      K0                          :11;
  int32_t     /* reserved */               :5;
  /* Color Corr. Coefficient 3 Config */
  int32_t      C3                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 4 Config */
  int32_t      C4                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 5 Config */
  int32_t      C5                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Offset 1 Config */
  int32_t      K1                          :11;
  int32_t     /* reserved */               :5;
  /* Color Corr. Coefficient 6 Config */
  int32_t      C6                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 7 Config */
  int32_t      C7                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Coefficient 8 Config */
  int32_t      C8                          :12;
  int32_t     /* reserved */               :4;
  /* Color Corr. Offset 2 Config */
  int32_t      K2                          :11;
  int32_t     /* reserved */               :5;
  /* Color Corr. Coefficient Q Config */
  uint32_t      coefQFactor                 : 2;
  uint32_t     /* reserved */               :30;
}__attribute__((packed, aligned(4))) VFE_ColorCorrectionCfgCmdType;
#endif

typedef enum {
  CC_MODE_NONE,
  CC_MODE_NORMAL,
  CC_MODE_LED,
  CC_LED_STROBE,
}cc_mode_t;

typedef struct
{
  int32_t  c0;
  int32_t  c1;
  int32_t  c2;
  int32_t  c3;
  int32_t  c4;
  int32_t  c5;
  int32_t  c6;
  int32_t  c7;
  int32_t  c8;
  int32_t  k0;
  int32_t  k1;
  int32_t  k2;

  uint8_t  q_factor;  // QFactor
} color_correct_type;

typedef struct {
  color_correct_type  chromatix_TL84_color_correction_snapshot;
  color_correct_type  chromatix_yhi_ylo_color_correction_snapshot;
  color_correct_type  chromatix_D65_color_correction_snapshot;
  color_correct_type  chromatix_A_color_correction_snapshot;
  color_correct_type  chromatix_LED_color_correction_snapshot;
  color_correct_type  chromatix_STROBE_color_correction;
  color_correct_type  chromatix_TL84_color_correction;
  color_correct_type  chromatix_yhi_ylo_color_correction;
  color_correct_type  chromatix_LED_color_correction_VF;
  color_correct_type  chromatix_D65_color_correction_VF;
  color_correct_type  chromatix_A_color_correction_VF;
}color_correct_tab_t;

typedef struct {
  VFE_ColorCorrectionCfgCmdType VFE_PrevColorCorrectionCmd;
  VFE_ColorCorrectionCfgCmdType VFE_PrevColorCorrectionCmd_Right;
  VFE_ColorCorrectionCfgCmdType VFE_SnapColorCorrectionCmd;
  VFE_ColorCorrectionCfgCmdType VFE_SnapColorCorrectionCmd_Right;
  float aec_ratio;
  color_correct_type cc[2]; /*0 preview 1 snap*/
  float dig_gain[2]; /*0 preview 1 snap*/
  float effects_matrix[3][3];
  int8_t update;
  vfe_op_mode_t cur_vfe_mode;
  float cur_ratio;
  cc_mode_t cur_cc_mode;
  color_correct_tab_t table;
  int8_t cc_enable;
  int trigger_enable;
  int reload_params;
  uint32_t color_temp;
  int hw_enable;
  vfe_module_ops_t ops;
} color_correct_mod_t;

vfe_status_t vfe_color_correct_ops_init(void *mod);
vfe_status_t vfe_color_correct_ops_deinit(void *mod);
vfe_status_t vfe_color_correct_init(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_color_correct_update(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_color_correct_trigger_update(int module_id,
  void *mod, void* vfe_parms);
vfe_status_t vfe_color_correct_config(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_color_correct_enable(int module_id, void *mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_color_correct_reload_params(int module_id, void *mod,
  void* vfe_parms);
vfe_status_t vfe_color_correct_trigger_enable(int module_id, void *mod,
  void* vfe_parms, int enable);
vfe_status_t vfe_color_correct_tv_validate(int module_id, void* input,
  void* output);
vfe_status_t vfe_color_correct_deinit(int mod_id, void *module,
  void *params);
vfe_status_t vfe_color_correct_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode);
vfe_status_t vfe_color_correct_set_effect(int mod_id,
  void *mod, void* parms, vfe_effects_type_t type);
vfe_status_t vfe_color_correct_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__COL_CORR_H__
