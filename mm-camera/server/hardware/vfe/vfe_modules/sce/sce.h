/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __SCE_H__
#define __SCE_H__

#define V32_SCE_OFF 0x00000418

typedef struct VFE_Skin_Enhan_Coordinates {
  /* VFE_SKIN_ENHAN_Cx_COORD_0  */
  int32_t      vertex00                 : 8;
  int32_t      vertex01                 : 8;
  int32_t      vertex02                 : 8;
  int32_t     /* reserved */            : 8;
  /* VFE_SKIN_ENHAN_Cx_COORD_1  */
  int32_t      vertex10                 : 8;
  int32_t      vertex11                 : 8;
  int32_t      vertex12                 : 8;
  int32_t     /* reserved */            : 8;
  /* VFE_SKIN_ENHAN_Cx_COORD_2  */
  int32_t      vertex20                 : 8;
  int32_t      vertex21                 : 8;
  int32_t      vertex22                 : 8;
  int32_t     /* reserved */            : 8;
  /* VFE_SKIN_ENHAN_Cx_COORD_3  */
  int32_t      vertex30                 : 8;
  int32_t      vertex31                 : 8;
  int32_t      vertex32                 : 8;
  int32_t     /* reserved */            : 8;
  /* VFE_SKIN_ENHAN_Cx_COORD_4  */
  int32_t      vertex40                 : 8;
  int32_t      vertex41                 : 8;
  int32_t      vertex42                 : 8;
  int32_t     /* reserved */            : 8;
}__attribute__((packed, aligned(4))) VFE_Skin_Enhan_Coordinates;

typedef struct VFE_Skin_Enhan_Coeff {
  /* coef_reg_0 */
  int32_t      coef00                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef01                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_1 */
  int32_t      coef10                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef11                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_2 */
  int32_t      coef20                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef21                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_3 */
  int32_t      coef30                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef31                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_4 */
  int32_t      coef40                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef41                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_5 */
  int32_t      coef50                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef51                    :  12;
  int32_t     /* reserved */             :   4;
}__attribute__((packed, aligned(4))) VFE_Skin_Enhan_Coeff;

typedef struct VFE_Skin_Enhan_Offset {
  /*  offset 0 */
  int32_t      offset0                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift0                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 1 */
  int32_t      offset1                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift1                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 2 */
  int32_t      offset2                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift2                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 3 */
  int32_t      offset3                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift3                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 4 */
  int32_t      offset4                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift4                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 5 */
  int32_t      offset5                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift5                     :   4;
  int32_t     /* reserved */               :   8;
}__attribute__((packed, aligned(4))) VFE_Skin_Enhan_Offset;

typedef struct VFE_Skin_enhan_ConfigCmdType {
  VFE_Skin_Enhan_Coordinates crcoord;
  VFE_Skin_Enhan_Coordinates cbcoord;
  VFE_Skin_Enhan_Coeff       crcoeff;
  VFE_Skin_Enhan_Coeff       cbcoeff;
  VFE_Skin_Enhan_Offset      croffset;
  VFE_Skin_Enhan_Offset      cboffset;
}__attribute__((packed, aligned(4))) VFE_Skin_enhan_ConfigCmdType;

typedef struct {
  VFE_Skin_enhan_ConfigCmdType sce_cmd;
  sce_cr_cb_triangle_set origin_triangles_A;
  sce_cr_cb_triangle_set destination_triangles_A;
  sce_cr_cb_triangle_set origin_triangles_D65;
  sce_cr_cb_triangle_set destination_triangles_D65;
  sce_cr_cb_triangle_set origin_triangles_TL84;
  sce_cr_cb_triangle_set destination_triangles_TL84;
  sce_cr_cb_triangle_set origin_triangles_ACC_GREEN;
  sce_cr_cb_triangle_set destination_triangles_ACC_GREEN;
  sce_cr_cb_triangle_set origin_triangles_ACC_BLUE;
  sce_cr_cb_triangle_set destination_triangles_ACC_BLUE;
  sce_cr_cb_triangle_set origin_triangles_ACC_ORANGE;
  sce_cr_cb_triangle_set destination_triangles_ACC_ORANGE;
  sce_cr_cb_triangle_set *orig;
  sce_cr_cb_triangle_set *dest;
  double sce_adjust_factor;
  cct_trigger_info trigger_info;
  int hw_enable;
  uint8_t sce_enable;
  uint8_t sce_trigger;
  uint8_t sce_update;
  awb_cct_type prev_cct_type;
  vfe_op_mode_t prev_mode;
  double prev_sce_adj;
  float prev_aec_ratio;
  vfe_module_ops_t ops;
  vfe_spl_effects_type active_spl_effect;
}sce_mod_t;

vfe_status_t vfe_sce_ops_init(void *mod);
vfe_status_t vfe_sce_ops_deinit(void *mod);
vfe_status_t vfe_sce_enable(int mod_id, void* module, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_sce_init(int mod_id, void *module, void *vparams);
vfe_status_t vfe_sce_deinit(int mod_id, void *module, void *vparams);
vfe_status_t vfe_sce_config(int mod_id, void *module, void *vparams);
vfe_status_t vfe_sce_update(int mod_id, void *module, void *vparams);
vfe_status_t vfe_sce_trigger_update(int mod_id, void *module,
  void *vparams);
vfe_status_t vfe_sce_trigger_enable(int mod_id, void *module, void *vparams,
  int enable);
vfe_status_t vfe_sce_reload_params(int mod_id, void *module,
  void *vparams);
vfe_status_t vfe_sce_tv_validate(int mod_id, void *test_input,
  void *test_output);
vfe_status_t vfe_sce_setup(sce_mod_t *sce_mod,vfe_params_t *vfe_params,
  int32_t val);
vfe_status_t vfe_sce_set_bestshot(int mod_id, void *module, void* vparams,
  camera_bestshot_mode_type mode);
vfe_status_t vfe_sce_plugin_update(int module_id, void *mod,
  void *vparams);
vfe_status_t vfe_sce_set_spl_effect(int mod_id, void* mod_sec, void *vparams,
  vfe_spl_effects_type type);
#endif //__SCE_H__
