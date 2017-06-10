/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ASF_H__
#define __ASF_H__

#define VFE_ASF_OFF1  0x000004A0

#ifndef VFE_2X
typedef struct VFE_AdaptiveFilterConfigCmdType {
  /* ASF Config Command */
  uint32_t                     smoothFilterEnabled     : 1;
  uint32_t                     sharpMode               : 2;
  uint32_t                     lpfMode                 : 1;
  uint32_t                     smoothCoefSurr          : 4;
  uint32_t                     smoothCoefCenter        : 8;
#ifdef VFE_31
  uint32_t                     pipeFlushCount          : 12;
#else
  uint32_t                     pipeFlushCount          : 13;
#endif
  uint32_t                     pipeFlushOvd            : 1;
  uint32_t                     flushHaltOvd            : 1;
  uint32_t                     cropEnable              : 1;
#ifdef VFE_31
  uint32_t                     /* reserved */          : 1;
#endif
  /* Sharpening Config 0 */
  uint32_t                     sharpThreshE1           : 7;
  uint32_t                    /* reserved */           : 1;
  int32_t                      sharpK1                 : 6;
  uint32_t                     /* reserved */          : 2;
  int32_t                      sharpK2                 : 6;
  uint32_t                     /* reserved */          : 2;
  uint32_t                     normalizeFactor         : 7;
#ifdef VFE_31
  uint32_t                    /* reserved */           : 1;
#else
  uint32_t                     cutYSmooth              : 1;
#endif
  /* Sharpening Config 1 */
  int32_t                      sharpThreshE2           : 8;
  int32_t                      sharpThreshE3           : 8;
  int32_t                      sharpThreshE4           : 8;
  int32_t                      sharpThreshE5           : 8;
  /* Sharpening Coefficients 0 */
  int32_t                      F1Coeff0                : 6;
  int32_t                      F1Coeff1                : 6;
  int32_t                      F1Coeff2                : 6;
  int32_t                      F1Coeff3                : 6;
  int32_t                      F1Coeff4                : 6;
  int32_t                    /* reserved */            : 2;
  /* Sharpening Coefficients 1 */
  int32_t                      F1Coeff5                : 6;
  int32_t                      F1Coeff6                : 6;
  int32_t                      F1Coeff7                : 6;
  int32_t                      F1Coeff8                : 7;
  int32_t                     /* reserved */           : 7;
  /* Sharpening Coefficients 2 */
  int32_t                      F2Coeff0                : 6;
  int32_t                      F2Coeff1                : 6;
  int32_t                      F2Coeff2                : 6;
  int32_t                      F2Coeff3                : 6;
  int32_t                      F2Coeff4                : 6;
  int32_t                     /* reserved */           : 2;
  /* Sharpening Coefficients 3 */
  int32_t                      F2Coeff5                : 6;
  int32_t                      F2Coeff6                : 6;
  int32_t                      F2Coeff7                : 6;
  int32_t                      F2Coeff8                : 7;
  int32_t                     /* reserved */           : 7;
  /* Sharpening Coefficients 4 */
  int32_t                      F3Coeff0                : 6;
  int32_t                      F3Coeff1                : 6;
  int32_t                      F3Coeff2                : 6;
  int32_t                      F3Coeff3                : 6;
  int32_t                      F3Coeff4                : 6;
  int32_t                     /* reserved */           : 2;
  /* Sharpening Coefficients 5 */
  int32_t                      F3Coeff5                : 6;
  int32_t                      F3Coeff6                : 6;
  int32_t                      F3Coeff7                : 6;
  int32_t                      F3Coeff8                : 7;
  int32_t                     /* reserved */           : 7;
  /* TODO: Below Params are unused so far. */
  /* asf max edge */
  uint32_t                     maxEdge                 :13;
  uint32_t                    /* reserved */           : 3;
#ifdef VFE_31
  uint32_t                     HBICount                :12;
  uint32_t                    /* reserved */           : 4;
#else
  uint32_t                     HBICount                :13;
  uint32_t                    /* reserved */           : 3;
#endif
  /* ASF Crop Width Config */
#ifdef VFE_31
  uint32_t                     lastPixel               : 12;
  uint32_t                    /* reserved */           : 4;
  uint32_t                     firstPixel              : 12;
  uint32_t                    /* reserved */           : 4;
#else
  uint32_t                     lastPixel               : 13;
  uint32_t                    /* reserved */           : 3;
  uint32_t                     firstPixel              : 13;
  uint32_t                    /* reserved */           : 3;
#endif
  /* ASP Crop Height Config */
  uint32_t                     lastLine                : 12;
  uint32_t                    /* reserved */           : 4;
  uint32_t                     firstLine               : 12;
  uint32_t                    /* reserved */           : 4;
#ifdef VFE_32
  /* ASF Special Effects config */
  uint32_t                     nzFlag0                 : 2;
  uint32_t                     nzFlag1                 : 2;
  uint32_t                     nzFlag2                 : 2;
  uint32_t                     nzFlag3                 : 2;
  uint32_t                     nzFlag4                 : 2;
  uint32_t                     nzFlag5                 : 2;
  uint32_t                     nzFlag6                 : 2;
  uint32_t                     nzFlag7                 : 2;
  uint32_t                    /* reserved */           : 16;
#endif
} __attribute__((packed, aligned(4))) VFE_AdaptiveFilterConfigCmdType;
#else /*7x targets*/
typedef struct VFE_AdaptiveFilterConfigCmdType {
  /* ASF Config Command */
  uint32_t                     smoothFilterEnabled     : 1;
  uint32_t                     sharpMode               : 2;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     smoothCoefSurr          : 4;
  uint32_t                     smoothCoefCenter        : 8;
  uint32_t                    /* reserved */           :16;

  /* Sharpening Config 0 */
  uint32_t                     sharpThreshE1           : 7;
  uint32_t                    /* reserved */           : 1;
  int32_t                      sharpK1                 : 5;
  uint32_t                     /* reserved */          : 3;
  int32_t                      sharpK2                 : 5;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     normalizeFactor         : 7;
  uint32_t                     /* reserved */          : 1;
  /* Sharpening Config 1 */
  int32_t                      sharpThreshE2           : 8;
  int32_t                      sharpThreshE3           : 8;
  int32_t                      sharpThreshE4           : 8;
  int32_t                      sharpThreshE5           : 8;
  /* Sharpening Coefficients 0 */
  int32_t                      F1Coeff0                : 6;
  int32_t                      F1Coeff1                : 6;
  int32_t                      F1Coeff2                : 6;
  int32_t                      F1Coeff3                : 6;
  int32_t                      F1Coeff4                : 6;
  int32_t                    /* reserved */            : 2;
  /* Sharpening Coefficients 1 */
  int32_t                      F1Coeff5                : 6;
  int32_t                      F1Coeff6                : 6;
  int32_t                      F1Coeff7                : 6;
  int32_t                      F1Coeff8                : 6;
  int32_t                     /* reserved */           : 8;
  /* Sharpening Coefficients 2 */
  int32_t                      F2Coeff0                : 6;
  int32_t                      F2Coeff1                : 6;
  int32_t                      F2Coeff2                : 6;
  int32_t                      F2Coeff3                : 6;
  int32_t                      F2Coeff4                : 6;
  int32_t                     /* reserved */           : 2;
  /* Sharpening Coefficients 3 */
  int32_t                      F2Coeff5                : 6;
  int32_t                      F2Coeff6                : 6;
  int32_t                      F2Coeff7                : 6;
  int32_t                      F2Coeff8                : 6;
  int32_t                     /* reserved */           : 8;
} __attribute__((packed, aligned(4))) VFE_AdaptiveFilterConfigCmdType;
#endif

typedef struct {
  chromatix_asf_5_5_type data;
  asf_setting_type settings;
}asf_params_t;

typedef struct {
  int asf_enable;
  /* Driven only by EzTune */
  uint32_t asf_trigger_enable;
  uint32_t vfe_reconfig;
  /* Driven only by AEC trigger */
  uint32_t asf_trigger_update;
  /* Driven by UI, BST/ASD soft_focus_degree and VFE downscale factor */
  uint32_t asf_sharpness_update;
  /* Drivern only by EzTune */
  uint32_t asf_reload_params;
  uint32_t asf_sp_effect_HW_enable;
  vfe_sharpness_info_t asf_sharpness_data;
  trigger_ratio_t asf_trigger_ratio;
  VFE_AdaptiveFilterConfigCmdType asf_prev_cmd;
  VFE_AdaptiveFilterConfigCmdType asf_snap_cmd;
  asf_params_t asf_prev_param;
  asf_params_t asf_snap_param;
  vfe_module_ops_t ops;
  int hw_enable_cmd;
}asf_mod_t;

/*===========================================================================
 *  ASF Interface APIs
 *==========================================================================*/
vfe_status_t vfe_asf_ops_init(void *mod);
vfe_status_t vfe_asf_ops_deinit(void *mod);
vfe_status_t vfe_asf_enable(int mod_id, void *mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_asf_init(int mod_id, void *mod, void *params);
vfe_status_t vfe_asf_deinit(int mod_id, void *mod, void *params);
vfe_status_t vfe_asf_config(int mod_id, void *mod, void *params);
vfe_status_t vfe_asf_update(int mod_id, void *mod, void *params);
vfe_status_t vfe_asf_trigger_update(int mod_id, void *mod,
  void *params);
vfe_status_t vfe_asf_set_bestshot(int mod_id, void *asf_ctrl, void *params,
  camera_bestshot_mode_type mode);
vfe_status_t vfe_asf_set_special_effect(int mod_id, void *mod,
  void *params, vfe_spl_effects_type effects);
vfe_status_t vfe_asf_SP_effect_snapshot_adjust(int mod_id, void *mod,
  void *params, vfe_spl_effects_type effects);
vfe_status_t vfe_asf_test_vector_validation( int mod_id,
  void *mod_in, void *mod_op);

/*===========================================================================
 *  ASF Interface APIs for EzTune
 *==========================================================================*/
vfe_status_t vfe_asf_trigger_enable(int mod_id, void *asf_mod,
  void *params, int enable);
vfe_status_t vfe_asf_reload_params(int mod_id, void *mod,
  void *params);
vfe_status_t vfe_asf_plugin_update(int module_id, void *mod,
  void *vparams);
#endif
