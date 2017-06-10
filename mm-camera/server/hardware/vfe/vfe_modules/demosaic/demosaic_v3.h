/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_V3_H__
#define __DEMOSAIC_V3_H__

#define VFE_DEMOSAICV3_CLASSIFIER_CNT 18

#define VFE_DEMOSAICV3_0_OFF      0x00000298
#define VFE_DEMOSAICV3_1_OFF      0x0000061C
#define VFE_DEMOSAICV3_2_OFF      0x0000066C

typedef struct VFE_DemosaicV3InterpClassifierType {
  uint32_t w_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t t_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t l_n                             : 5;
  uint32_t   /* reserved */                : 2;
  uint32_t b_n                             : 1;
}__attribute__((packed, aligned(4))) VFE_DemosaicV3InterpClassifierType;

#ifndef VFE_2X
typedef struct VFE_DemosaicV3ConfigCmdType {
  /* Demosaic Config */
  /* dbpc enable */
  uint32_t dbpcEnable                      : 1;
  /* dbcc enable */
  uint32_t dbccEnable                      : 1;
  /* abcc enable */
  uint32_t abccEnable                      : 1;
  /* abf enable */
  uint32_t abfEnable                       : 1;
  /* cosited rgb enable */
  uint32_t cositedRgbEnable                : 1;
  uint32_t   /* reserved */                : 3;
  /* abcc lut bank sel */
  uint32_t abccLutBankSel                  : 1;
  uint32_t   /* reserved */                : 7;
  /* pipe flush count */
  uint32_t pipeFlushCount                  : 13;
  uint32_t   /* reserved */                : 1;
  uint32_t pipeFlushOvd                    : 1;
  uint32_t flushHaltOvd                    : 1;

  /* Interp WB Gain 0 */
  uint32_t rgWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t bgWbGain                        : 9;
  uint32_t   /* reserved */                : 8;

  /* Interp WB Gain 1 */
  uint32_t grWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t gbWbGain                        : 9;
  uint32_t   /* reserved */                : 8;

  /* Interp Classifier */
  VFE_DemosaicV3InterpClassifierType
    interpClassifier[VFE_DEMOSAICV3_CLASSIFIER_CNT];

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 16;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 3;
  uint32_t a                               : 6;
  uint32_t   /* reserved */                : 14;
}__attribute__((packed, aligned(4))) VFE_DemosaicV3ConfigCmdType;
#else
typedef struct VFE_DemosaicV3ConfigCmdType {
  /* Demosaic Config */
  /* Interp WB Gain 0 */
  uint32_t rgWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t bgWbGain                        : 9;
  uint32_t   /* reserved */                : 7;
  /* cosited rgb enable */
  uint32_t cositedRgbEnable                : 1;

  /* Interp WB Gain 1 */
  uint32_t grWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t gbWbGain                        : 9;
  uint32_t   /* reserved */                : 8;

  /* Interp Classifier */
  VFE_DemosaicV3InterpClassifierType
    interpClassifier[VFE_DEMOSAICV3_CLASSIFIER_CNT];

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 16;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 3;
  uint32_t a                               : 6;
  uint32_t   /* reserved */                : 14;
}__attribute__((packed, aligned(4))) VFE_DemosaicV3ConfigCmdType;
#endif

#ifndef VFE_2X
typedef struct VFE_DemosaicV3UpdateCmdType {
  /* Demosaic Config */
  /* dbpc enable */
  uint32_t dbpcEnable                      : 1;
  /* dbcc enable */
  uint32_t dbccEnable                      : 1;
  /* abcc enable */
  uint32_t abccEnable                      : 1;
  /* abf enable */
  uint32_t abfEnable                       : 1;
  /* cosited rgb enable */
  uint32_t cositedRgbEnable                : 1;
  uint32_t   /* reserved */                : 3;
  /* abcc lut bank sel */
  uint32_t abccLutBankSel                  : 1;
  uint32_t   /* reserved */                : 7;
  /* pipe flush count */
  uint32_t pipeFlushCount                  : 13;
  uint32_t   /* reserved */                : 1;
  uint32_t pipeFlushOvd                    : 1;
  uint32_t flushHaltOvd                    : 1;

  /* Interp WB Gain 0 */
  uint32_t rgWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t bgWbGain                        : 9;
  uint32_t   /* reserved */                : 8;

  /* Interp WB Gain 1 */
  uint32_t grWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t gbWbGain                        : 9;
  uint32_t   /* reserved */                : 8;

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 16;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 3;
  uint32_t a                               : 6;
  uint32_t   /* reserved */                : 14;
}__attribute__((packed, aligned(4))) VFE_DemosaicV3UpdateCmdType;
#else
typedef VFE_DemosaicV3ConfigCmdType VFE_DemosaicV3UpdateCmdType;
#endif

typedef struct {
  VFE_DemosaicV3ConfigCmdType demosaic_cmd;
  VFE_DemosaicV3ConfigCmdType demosaic_snapshot_cmd;
  VFE_DemosaicV3UpdateCmdType demosaic_vf_up_cmd;
  VFE_DemosaicV3UpdateCmdType demosaic_snap_up_cmd;
  vfe_op_mode_t cur_mode;
  int8_t update;
  int8_t enable;
  float ratio;
  int trigger_enable;
  int reload_params;
  int hw_enable;
  vfe_module_ops_t ops;
}demosaic_mod_t;

vfe_status_t vfe_demosaic_ops_init(void *mod);
vfe_status_t vfe_demosaic_ops_deinit(void *mod);
vfe_status_t vfe_demosaic_init(int mod_id, void *module, void *vparams);
vfe_status_t vfe_demosaic_enable(int mod_id, void *module, void *vparams,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_demosaic_config(int mod_id, void *module, void *vparams);
vfe_status_t vfe_demosaic_update(int mod_id, void *module, void *vparams);
vfe_status_t vfe_demosaic_trigger_update(int mod_id, void *module,
  void *vparams);
vfe_status_t vfe_demosaic_reload_params(int mod_id, void* module,
  void* params);
vfe_status_t vfe_demosaic_trigger_enable(int mod_id, void* module,
  void* params, int enable);
vfe_status_t vfe_demosaic_tv_validate(int mod_id, void *test_input,
  void *test_output);
vfe_status_t vfe_demosaic_deinit(int mod_id, void *module,
  void *params);
vfe_status_t vfe_demosaic_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode);
vfe_status_t vfe_demosaic_plugin_update(int module_id, void *mod,
  void *vparams);
#endif /* __DEMOSAIC_V3_H__ */
