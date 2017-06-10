/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CHROMA_ENHAN_H__
#define __CHROMA_ENHAN_H__

typedef enum {
  BESTSHOT_COLOR_CONVERSION_NORMAL = 0,
  BESTSHOT_COLOR_CONVERSION_SUNSET,
  BESTSHOT_COLOR_CONVERSION_SKIN_TONE,
  BESTSHOT_COLOR_CONVERSION_SATURATED,
  BESTSHOT_COLOR_CONVERSION_MAX
} bestshot_cv_t;

#ifndef VFE_2X
typedef struct VFE_Chroma_Enhance_CfgCmdType {
  /* Color Conversion (RGB to Y) Config */
  int32_t      RGBtoYConversionV0      :12;
  int32_t     /* reserved */           :20;
  /* Conversion Coefficient 1 */
  int32_t      RGBtoYConversionV1      :12;
  int32_t     /* reserved */           :20;
  /* Conversion Coefficient 2 */
  int32_t      RGBtoYConversionV2      :12;
  int32_t     /* reserved */           :20;
  /* Conversion Offset */
  uint32_t      RGBtoYConversionOffset  : 8;
  uint32_t     /* reserved */           :24;
  /* Chroma Enhance A Config */
  int32_t      ap                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      am                      :12;
  int32_t     /* reserved */           : 4;
  /* Chroma Enhance B Config */
  int32_t      bp                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      bm                      :12;
  int32_t     /* reserved */           : 4;
  /* Chroma Enhance C Config */
  int32_t      cp                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      cm                      :12;
  int32_t     /* reserved */           : 4;
  /* Chroma Enhance D Config */
  int32_t      dp                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      dm                      :12;
  int32_t     /* reserved */           : 4;
  /* Chroma Enhance K Config */
  int32_t      kcb                     :11;
  int32_t     /* reserved */           : 5;
  int32_t      kcr                     :11;
  int32_t     /* reserved */           : 5;
}__attribute__((packed, aligned(4))) VFE_Chroma_Enhance_CfgCmdType;
#else
typedef struct VFE_Chroma_Enhance_CfgCmdType {
  /* Chroma Enhance A Config */
  int32_t      ap                      :11;
  int32_t     /* reserved */           : 5;
  int32_t      am                      :11;
  int32_t     /* reserved */           : 5;
  /* Chroma Enhance B Config */
  int32_t      bp                      :11;
  int32_t     /* reserved */           : 5;
  int32_t      bm                      :11;
  int32_t     /* reserved */           : 5;
  /* Chroma Enhance C Config */
  int32_t      cp                      :11;
  int32_t     /* reserved */           : 5;
  int32_t      cm                      :11;
  int32_t     /* reserved */           : 5;
  /* Chroma Enhance D Config */
  int32_t      dp                      :11;
  int32_t     /* reserved */           : 5;
  int32_t      dm                      :11;
  int32_t     /* reserved */           : 5;
  /* Chroma Enhance K Config */
  int32_t      kcb                     :11;
  int32_t     /* reserved */           : 5;
  int32_t      kcr                     :11;
  int32_t     /* reserved */           : 5;
  /* Color Conversion (RGB to Y) Config */
  int32_t      RGBtoYConversionV0      :12;
  int32_t     /* reserved */           : 4;
  /* Conversion Coefficient 1 */
  int32_t      RGBtoYConversionV1      :12;
  int32_t     /* reserved */           : 4;
  /* Conversion Coefficient 2 */
  int32_t      RGBtoYConversionV2      :12;
  int32_t     /* reserved */           : 4;
  /* Conversion Offset */
  uint32_t      RGBtoYConversionOffset  : 12;
  uint32_t     /* reserved */           : 4;
}__attribute__((packed, aligned(4))) VFE_Chroma_Enhance_CfgCmdType;
#endif

typedef struct {
  int32_t a_m;  /* 11-bit, Q8, signed*/
  int32_t a_p;  /* 11-bit, Q8, signed*/
  int32_t b_m;  /* 11-bit, Q8, signed*/
  int32_t b_p;  /* 11-bit, Q8, signed*/
  int32_t c_m;  /* 11-bit, Q8, signed*/
  int32_t c_p;  /* 11-bit, Q8, signed*/
  int32_t d_m;  /* 11-bit, Q8, signed*/
  int32_t d_p;  /* 11-bit, Q8, signed*/
  int16_t k_cb; /* 11-bit, Q8, signed*/
  int16_t k_cr; /* 11-bit, Q8, signed*/
}chroma_enhancement_t;

typedef struct {
  int32_t v0; /* 12-bit, Q8, signed*/
  int32_t v1; /* 12-bit, Q8, signed*/
  int32_t v2; /* 12-bit, Q8, signed*/
  uint16_t k; /* 12-bit, Q8, signed*/
}luma_calculation_t;

typedef struct {
  chroma_enhancement_t chroma;
  luma_calculation_t luma;
} color_conversion_t;

typedef struct {
  VFE_Chroma_Enhance_CfgCmdType chroma_enhan_cmd;
  chromatix_color_conversion_type cv_data;
  chromatix_color_conversion_type *p_cv;
  vfe_module_ops_t ops;
  float effects_matrix[2][2];
  int8_t cv_update;
  int8_t cv_enable;
  trigger_ratio_t curr_ratio;
  vfe_op_mode_t cur_mode;
  uint32_t color_temp;
  int trigger_enable;
  int reload_params;
  int hw_enable;
}chroma_enhan_mod_t;

vfe_status_t vfe_chroma_enhan_ops_init(void *mod);
vfe_status_t vfe_chroma_enhan_ops_deinit(void *mod);
vfe_status_t vfe_chroma_enhan_init(int mod_id, void *mod, void *parms);
vfe_status_t vfe_chroma_enhan_deinit(int mod_id, void *mod, void *parms);
vfe_status_t vfe_color_conversion_enable(int mod_id, void *mod_cv, void *vparms,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_color_conversion_config(int mod_id, void *mod_cv,
  void *vparms);
vfe_status_t vfe_color_conversion_update(int mod_id, void *mod, void *parms);
vfe_status_t vfe_color_conversion_trigger_update(int mod_id, void *mod,
  void *parms);
vfe_status_t vfe_color_conversion_set_hue(chroma_enhan_mod_t *mod,
  vfe_params_t *parms, float hue);
vfe_status_t vfe_color_conversion_set_effect(int mod_id, void *mod,
  void *parms, vfe_effects_type_t type);
vfe_status_t vfe_color_conversion_set_spl_effect(int mod_id, void *mod,
  void *parms, vfe_spl_effects_type effects);
vfe_status_t vfe_color_conversion_set_manual_wb(int mod_id, void *mod,
  void *parms);
vfe_status_t vfe_color_conversion_set_bestshot(int mod_id, void *mod,
  void *parms, camera_bestshot_mode_type mode);
vfe_status_t vfe_color_conversion_reload_params(int mod_id, void *mod,
  void *parms);
vfe_status_t vfe_color_conversion_trigger_enable(int mod_id, void *mod,
  void *parms, int enable);
vfe_status_t vfe_color_conversion_tv_validate(int mod_id, void *in, void *op);
vfe_status_t vfe_color_conv_plugin_update(int module_id, void *mod,
  void *vparams);
#endif// __CHROMA_ENHAN_H__
