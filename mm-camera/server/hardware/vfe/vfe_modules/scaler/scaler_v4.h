/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __SCALER_V4_H__
#define __SCALER_v4_H__

/* Y Scaler Config Cmd*/
typedef struct VFE_Y_ScaleCfgCmdType {
  /* Y Scale Config */
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  /* Y Scale H Image Size Config */
  uint32_t     hIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     hOut                           :13;
  uint32_t     /* reserved */                 : 3;
  /* Y Scale H Phase Config */
  uint32_t     horizPhaseMult                 :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 :10;
  /* Y Scale H Stripe Config */
  uint32_t     horizMNInit                    :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     horizPhaseInit                 :16;

  /* Y Scale V Image Size Config */
  uint32_t     vIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     vOut                           :13;
  uint32_t     /* reserved */                 : 3;
  /* Y Scale V Phase Config */
  uint32_t     vertPhaseMult                  :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 :10;
  /* Y Scale V Stripe Config */
  uint32_t     vertMNInit                     :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     vertPhaseInit                  :16;

}__attribute__((packed, aligned(4))) VFE_Y_ScaleCfgCmdType;

/* CbCr Scaler Config Cmd*/
typedef struct VFE_CbCr_ScaleCfgCmdType {
  /* CbCr config*/
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  /* CbCr H Image Size config */
  uint32_t     hIn                            :15;
  uint32_t     /* reserved */                 : 1;
  uint32_t     hOut                           :14;
  uint32_t     /* reserved */                 : 2;
  /* CbCr H Phase config */
  uint32_t     horizPhaseMult                 :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 :10;
  /* CbCr H Stripe config 0*/
  uint32_t     horizMNInit                    :15;
  uint32_t     /* reserved */                 :17;
  /* CbCr H Stripe config 1*/
  uint32_t     horizPhaseInit                 :18;
  uint32_t     /* reserved */                 :14;
  /* CbCr H Pad config*/
  uint32_t     ScaleCbCrInWidth               :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     HSkipCount                     :13;
  uint32_t     /* reserved */                 : 2;
  uint32_t     RightPadEnable                 : 1;

  /* CbCr V Image Size config */
  uint32_t     vIn                            :15;
  uint32_t     /* reserved */                 : 1;
  uint32_t     vOut                           :14;
  uint32_t     /* reserved */                 : 2;
  /* CbCr V Phase config */
  uint32_t     vertPhaseMult                  :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 :10;
  /* CbCr V Stripe config 0*/
  uint32_t     vertMNInit                     :15;
  uint32_t     /* reserved */                 :17;
  /* CbCr V Stripe config 1*/
  uint32_t     vertPhaseInit                  :18;
  uint32_t     /* reserved */                 :14;
  /* CbCr V Pad config*/
  uint32_t     ScaleCbCrInHeight              :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     VSkipCount                     :13;
  uint32_t     /* reserved */                 : 2;
  uint32_t     BottomPadEnable                : 1;
}__attribute__((packed, aligned(4))) VFE_CbCr_ScaleCfgCmdType;

/* Scaler Config Cmd Type*/
typedef struct VFE_ScaleCfgCmdType {
  VFE_Y_ScaleCfgCmdType     Y_ScaleCfg;
  VFE_CbCr_ScaleCfgCmdType  CbCr_ScaleCfg;
}__attribute__((packed, aligned(4))) VFE_ScaleCfgCmdType;

typedef struct {
  VFE_ScaleCfgCmdType Enc_scaler_cmd;
  VFE_ScaleCfgCmdType View_scaler_cmd;
  uint8_t scaler_update;
  uint8_t scaler_enc_enable;
  uint8_t scaler_view_enable;
  float out_1_scale_ratio;
  float out_2_scale_ratio;
  uint32_t vfe_op_mode;
  vfe_module_ops_t ops;
}scaler_mod_t;

vfe_status_t vfe_scaler_ops_init(void *mod);
vfe_status_t vfe_scaler_ops_deinit(void *mod);
vfe_status_t vfe_scaler_init(int mod_id, void *module, void *params);
vfe_status_t vfe_scaler_config(int mod_id, void* mod_sc, void* vparams);
vfe_status_t vfe_scaler_update(int mod_id, void* mod_sc, void* vparams);
vfe_status_t vfe_scaler_enable(int mod_id, void* mod_sc, void* vparams, int8_t enable, int8_t hw_write);
vfe_status_t vfe_set_scaler(void *params);
vfe_status_t vfe_scaler_deinit(int mod_id, void *module, void *params);
#endif //__SCALER_H__
