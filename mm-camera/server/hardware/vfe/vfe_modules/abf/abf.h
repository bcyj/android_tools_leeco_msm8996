/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_ABF_H__
#define __DEMOSAIC_ABF_H__

#define ABF2_CUTOFF1(c1) MAX(17, c1)
#define ABF2_CUTOFF2(c1, c2) MAX((c1-1), c2)
#define ABF2_CUTOFF3(c2, c3) MAX((c2+9), c3)
#define ABF2_MULT_NEG(c2, c3) FLOAT_TO_Q(12, 8.0/(c3 - c2))
#define ABF2_MULT_POS(c1) FLOAT_TO_Q(12, 16.0/c1)
#define ABF2_LUT(v) MIN(2047, MAX(-2047, (FLOAT_TO_Q(11, v))))
#define ABF2_SP_KERNEL(v) MIN(64, MAX(1, FLOAT_TO_Q(6, v)))

typedef struct VFE_DemosaicABF_Cfg {
  /* Demosaic Config */
#ifdef VFE_31
  uint32_t     /* reserved */                   :  1;
  uint32_t       enable                         :  1;
  uint32_t       forceOn                        :  1;
  uint32_t     /* reserved */                   : 29;
#else
  uint32_t     /* reserved */                   :  3;
  uint32_t       enable                         :  1;
  uint32_t     /* reserved */                   : 28;
#endif
}__attribute__((packed, aligned(4))) VFE_DemosaicABF_Cfg;

#ifndef VFE_2X
typedef struct VFE_DemosaicABF_gCfg {
  /* Demosaic ABF Green Config 0 */
  uint32_t       Cutoff1                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       Cutoff2                : 12;
  uint32_t     /* reserved */           :  4;
  /* Demosaic ABF Green Config 1 */
  uint32_t       Cutoff3                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       SpatialKernelA0        :  7;
  uint32_t     /* reserved */           :  1;
  uint32_t       SpatialKernelA1        :  7;
  uint32_t     /* reserved */           :  1;
  /* Demosaic ABF Green Config 2 */
  uint32_t       MultNegative           : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       MultPositive           : 12;
  uint32_t     /* reserved */           :  4;
}__attribute__((packed, aligned(4))) VFE_DemosaicABF_gCfg;
#else
typedef struct VFE_DemosaicABF_gCfg {
  /* Demosaic ABF Green Config 0 */
  uint32_t       Cutoff1                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       Cutoff2                : 12;
  uint32_t     /* reserved */           :  3;
  uint32_t       enable                 :  1;
  /* Demosaic ABF Green Config 1 */
  uint32_t       Cutoff3                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       SpatialKernelA0        :  7;
  uint32_t     /* reserved */           :  1;
  uint32_t       SpatialKernelA1        :  7;
  uint32_t     /* reserved */           :  1;
  /* Demosaic ABF Green Config 2 */
  uint32_t       MultNegative           : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       MultPositive           : 12;
  uint32_t     /* reserved */           :  4;
}__attribute__((packed, aligned(4))) VFE_DemosaicABF_gCfg;
#endif

typedef struct VFE_DemosaicABF_Pos_Lut {
  /* Demosaic ABF POS LUT */
  int32_t       PostiveLUT0             : 12;
  int32_t     /* reserved */            :  4;
  int32_t       PostiveLUT1             : 12;
  int32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4)))  VFE_DemosaicABF_Pos_Lut;

typedef struct VFE_DemosaicABF_Neg_Lut {
  /* Demosaic ABF Green NEG LUT */
  int32_t       NegativeLUT0            : 12;
  int32_t     /* reserved */            :  4;
  int32_t       NegativeLUT1            : 12;
  int32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4))) VFE_DemosaicABF_Neg_Lut;

typedef struct VFE_DemosaicABF_RBCfg {
/* blue */
  /* Demosaic ABF Blue Config 0 */
  uint32_t       Cutoff1                 : 12;
  uint32_t     /* reserved */            :  4;
  uint32_t       Cutoff2                 : 12;
  uint32_t     /* reserved */            :  4;
  /* Demosaic ABF Blue Config 1 */
  uint32_t       Cutoff3                 : 12;
  uint32_t     /* reserved */            : 20;
  /* Demosaic ABF Blue Config 2 */
  uint32_t       MultNegative            : 12;
  uint32_t     /* reserved */            :  4;
  uint32_t       MultPositive            : 12;
  uint32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4)))  VFE_DemosaicABF_RBCfg;

/* Demosaic ABF Update Command  */
typedef struct VFE_DemosaicABF_CmdType {
#ifndef VFE_2X
  /* ABF config */
  VFE_DemosaicABF_Cfg    abf_Cfg;
#endif
  /* Green config */
  VFE_DemosaicABF_gCfg    gCfg;
  VFE_DemosaicABF_Pos_Lut gPosLut[8] ;
  VFE_DemosaicABF_Neg_Lut gNegLut[4] ;
  /* Blue config */
  VFE_DemosaicABF_RBCfg   bCfg;
  VFE_DemosaicABF_Pos_Lut bPosLut[8] ;
  VFE_DemosaicABF_Neg_Lut bNegLut[4] ;
  /* Red config */
  VFE_DemosaicABF_RBCfg   rCfg;
  VFE_DemosaicABF_Pos_Lut rPosLut[8] ;
  VFE_DemosaicABF_Neg_Lut rNegLut[4] ;
} __attribute__((packed, aligned(4))) VFE_DemosaicABF_CmdType;

typedef struct {
  float table_pos[16];
  float table_neg[8];
} abf2_table_t;

typedef struct {
  abf2_table_t r_table;
  abf2_table_t g_table;
  abf2_table_t b_table;
  chromatix_adaptive_bayer_filter_data_type2 data;
  int8_t table_updated;
}abf2_parms_t;

typedef struct {
  VFE_DemosaicABF_CmdType  VFE_DemosaicABFCfgCmd;
  VFE_DemosaicABF_CmdType  VFE_SnapshotDemosaicABFCfgCmd;
  vfe_module_ops_t ops;
  abf2_parms_t abf2_parms;
  int8_t abf2_update;
  vfe_op_mode_t cur_mode;
  trigger_ratio_t abf2_ratio;
  int8_t enable;
  int trigger_enable;
  int reload_params;
} abf_mod_t;

vfe_status_t vfe_abf_ops_init(void *mod);
vfe_status_t vfe_abf_ops_deinit(void *mod);
vfe_status_t vfe_abf_init(int mod_id, void *ctrl, void *parms);
vfe_status_t vfe_abf_update(int mod_id, void *ctrl, void* parms);
vfe_status_t vfe_abf_trigger_update(int mod_id, void *mod, void *parms);
vfe_status_t vfe_abf_config(int mod_id, void *mod, void* params);
vfe_status_t vfe_abf_enable(int mod_id, void *abf_ctrl,  void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_abf_reload_params(int mod_id, void *mod, void *parms);
vfe_status_t vfe_abf_trigger_enable(int mod_id, void *mod, void *params,
  int enable);
vfe_status_t vfe_abf_tv_validate(int mod_id, void *input, void *output);
vfe_status_t vfe_abf_deinit(int mod_id, void *mod, void *params);
vfe_status_t vfe_abf_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__DEMOSAIC_ABF_H__
