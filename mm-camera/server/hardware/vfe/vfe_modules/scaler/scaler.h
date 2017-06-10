/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __SCALER_H__
#define __SCALER_H__

typedef struct VFE_Main_Scaler_ConfigCmdType {
  /* Scaler Enable Config */
  uint32_t                          hEnable                     : 1;
  uint32_t                          vEnable                     : 1;
  uint32_t                         /* reserved */               : 30;
  /* Scale H Image Size Config */
  uint32_t                          inWidth                     : 13;
  uint32_t                         /* reserved */               : 3;
  uint32_t                          outWidth                    : 13;
  uint32_t                         /* reserved */               : 3;
  /* Scale H Phase Config */
  uint32_t                          horizPhaseMult              : 19;
  uint32_t                         /* reserved */               : 1;
  uint32_t                          horizInterResolution        : 2;
  uint32_t                         /* reserved */               : 10;
  /* Scale H Stripe Config */
  uint32_t                          horizMNInit                 : 13;
  uint32_t                         /* reserved */               : 3;
  uint32_t                          horizPhaseInit              : 16;
  /* Scale V Image Size Config */
  uint32_t                          inHeight                    : 13;
  uint32_t                         /* reserved */               : 3;
  uint32_t                          outHeight                   : 13;
  uint32_t                         /* reserved */               : 3;
  /* Scale V Phase Config */
  uint32_t                          vertPhaseMult               : 19;
  uint32_t                         /* reserved */               : 1;
  uint32_t                          vertInterResolution         : 2;
  uint32_t                         /* reserved */               : 10;
  /* Scale V Stripe Config */
  uint32_t                          vertMNInit                  : 13;
  uint32_t                         /* reserved */               : 3;
  uint32_t                          vertPhaseInit               : 16;

} __attribute__((packed, aligned(4))) VFE_Main_Scaler_ConfigCmdType;

typedef struct VFE_Output_CbCrScaleCfgCmdType {

  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  uint32_t     hIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     hOut                           :13;
  uint32_t     /* reserved */                 : 3;

  uint32_t     horizPhaseMult                 :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 :10;

  uint32_t     vIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     vOut                           :13;
  uint32_t     /* reserved */                 : 3;

  uint32_t     vertPhaseMult                  :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 :10;
}__attribute__((packed, aligned(4))) VFE_Output_CbCrScaleCfgCmdType;

typedef struct VFE_Output_YScaleCfgCmdType {
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  uint32_t     hIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     hOut                           :13;
  uint32_t     /* reserved */                 : 3;

  uint32_t     horizPhaseMult                 :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 :10;

  uint32_t     vIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     vOut                           :13;
  uint32_t     /* reserved */                 : 3;

  uint32_t     vertPhaseMult                  :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 :10;
}__attribute__((packed, aligned(4))) VFE_Output_YScaleCfgCmdType;

typedef struct {
  VFE_Main_Scaler_ConfigCmdType main_scaler_cmd;
  VFE_Output_YScaleCfgCmdType y_scaler_cmd;
  VFE_Output_CbCrScaleCfgCmdType cbcr_scaler_cmd;
  uint8_t scaler_update;
  uint8_t scaler_enable;
  uint32_t vfe_op_mode;
  vfe_module_ops_t ops;
}scaler_mod_t;

typedef struct {
  VFE_Main_Scaler_ConfigCmdType main_scaler_cmd;
  VFE_Output_YScaleCfgCmdType y_scaler_cmd;
  VFE_Output_CbCrScaleCfgCmdType cbcr_scaler_cmd;
}VFE_SCaler_ConfigCmdType;

vfe_status_t vfe_scaler_ops_init(void *mod);
vfe_status_t vfe_scaler_ops_deinit(void *mod);
vfe_status_t vfe_scaler_config(int mod_id, void* mod_sc,
  void* vparams);
vfe_status_t vfe_scaler_update(int mod_id, void* mod_sc,
  void* vparams);
vfe_status_t vfe_scaler_enable(int mod_id, void* mod_sc,
  void* vparams, int8_t enable, int8_t hw_write);
vfe_status_t vfe_scaler_deinit(int mod_id, void *module, void *params);
vfe_status_t vfe_scaler_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__SCALER_H__
