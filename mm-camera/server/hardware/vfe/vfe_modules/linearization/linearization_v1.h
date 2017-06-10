/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __LINEARIZATION_H__
#define __LINEARIZATION_H__

#define VFE32_LINEARIZATON_TABLE_LENGTH 36

#define V32_LINEAR_OFF1 0x00000264
#define V32_LINEAR_OFF2 0x0000067C

#define CALC_SLOPE(x1,x2,y1,y2) \
  ((float)(y2 - y1) /(float)(x2 -x1))

typedef struct VFE_PointSlopeData {
  /* INTERP_0 */
  uint32_t kneePoint_P1         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P0         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_1 */
  uint32_t kneePoint_P3         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P2         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_2 */
  uint32_t kneePoint_P5         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P4         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_3 */
  uint32_t kneePoint_P7         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P6         :12;
  uint32_t /* reserved */       : 4;
}__attribute__((packed, aligned(4))) VFE_PointSlopeData;

typedef struct VFE_LinearizationCfgParams {
  /* BLACK_CONFIG */
  uint32_t lutBankSel           : 1;
  uint32_t /* reserved */       :31;

  /* Knee points for R channel */
  VFE_PointSlopeData pointSlopeR;

  /* Knee points for Gr channel */
  VFE_PointSlopeData pointSlopeGb;

  /* Knee points for B channel */
  VFE_PointSlopeData pointSlopeB;

  /* Knee points for Gb channel */
  VFE_PointSlopeData pointSlopeGr;

}__attribute__((packed, aligned(4))) VFE_LinearizationCfgParams;

typedef struct VFE_LinearizationRightCfgParams {
  /* Knee points for R channel Right */
  VFE_PointSlopeData pointSlopeR;

  /* Knee points for Gr channel Right */
  VFE_PointSlopeData pointSlopeGb;

  /* Knee points for B channel Right */
  VFE_PointSlopeData pointSlopeB;

  /* Knee points for Gb channel Right */
  VFE_PointSlopeData pointSlopeGr;
}__attribute__((packed, aligned(4))) VFE_LinearizationRightCfgParams;

typedef struct VFE_LinearizationCfgTable {
  uint32_t Lut[VFE32_LINEARIZATON_TABLE_LENGTH];
}VFE_LinearizationCfgTable;

typedef struct VFE_LinearizationCmdType {
  VFE_LinearizationCfgParams CfgParams;
  VFE_LinearizationCfgTable CfgTbl;
}VFE_LinearizationCmdType;

typedef struct VFE_LinearizationRightCmdType {
  VFE_LinearizationRightCfgParams CfgParams;
  VFE_LinearizationCfgTable CfgTbl;
}VFE_LinearizationRightCmdType;

typedef struct
{
  unsigned short r_lut_p_l[8]; // 12uQ0
  unsigned short gr_lut_p[8]; // 12uQ0
  unsigned short gb_lut_p[8]; // 12uQ0
  unsigned short b_lut_p[8]; // 12uQ0

  unsigned short r_lut_base[9]; // 12uQ0
  unsigned short gr_lut_base[9]; // 12uQ0
  unsigned short gb_lut_base[9]; // 12uQ0
  unsigned short b_lut_base[9]; // 12uQ0

  unsigned int r_lut_delta[9]; // 18uQ9
  unsigned int gr_lut_delta[9]; // 18uQ9
  unsigned int gb_lut_delta[9]; // 18uQ9
  unsigned int b_lut_delta[9]; // 18uQ9
}VFE_LinearizationLut;

typedef enum {
  LINEAR_AEC_BRIGHT = 0,
  LINEAR_AEC_BRIGHT_NORMAL,
  LINEAR_AEC_NORMAL,
  LINEAR_AEC_NORMAL_LOW,
  LINEAR_AEC_LOW,
  LINEAR_AEC_LUX_MAX,
} vfe_linear_lux_t;

typedef struct {
  VFE_LinearizationCmdType  video_linear_cmd;
  VFE_LinearizationCmdType  snapshot_linear_cmd;
  cct_trigger_info trigger_info;
  VFE_LinearizationLut linear_lut;
  int hw_enable;
  uint8_t linear_trigger;
  uint8_t linear_update;
  uint8_t linear_enable;
  awb_cct_type prev_cct_type;
  vfe_op_mode_t prev_mode;
  vfe_linear_lux_t prev_lux;
  float blk_inc_comp;
  vfe_module_ops_t ops;
}linear_mod_t;

vfe_status_t vfe_linearization_init(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_linearization_update(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_linearization_trigger_update(int module_id,
  void *mod, void* vfe_parms);
vfe_status_t vfe_linearization_config(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_linearization_enable(int module_id, void *mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_linearization_reload_params(int module_id, void *mod,
  void* vfe_parms);
vfe_status_t vfe_linearization_trigger_enable(int module_id, void *mod,
  void* vfe_parms, int enable);
vfe_status_t vfe_linearization_tv_validate(int module_id, void* input,
  void* output);
vfe_status_t vfe_linearization_deinit(int mod_id, void *module,
  void *params);
vfe_status_t vfe_linearization_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode);

vfe_status_t vfe_linearization_ops_init(void *mod);
vfe_status_t vfe_linearization_ops_deinit(void *mod);
vfe_status_t vfe_linearization_plugin_update(int module_id, void *mod,
  void *vparams);

#endif //__LINEARIZATION_H__
