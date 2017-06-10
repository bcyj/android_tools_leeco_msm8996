/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCALER40_REG_H__
#define __SCALER40_REG_H__

#define ISP_SCALER40_ENC_OFF 0x0000075C
#define ISP_SCALER40_ENC_LEN 18

#define ISP_SCALER40_VIEW_OFF 0x000007A4
#define ISP_SCALER40_VIEW_LEN 18

#define ISP_SCALER40_MAX_VIEW_H_OUT 2816
#define ISP_SCALER40_MAX_SCALER_FACTOR 16
#define ISP_SCALER40_LIMIT_SCALER_FACTOR 8
/* system work around for non double buffered Y stripe config*/
#define ISP_SCALER40_SYSTEM_WORKAROUND_OFF_1 0x0000092C
#define ISP_SCALER40_SYSTEM_WORKAROUND_LEN_1 1

#define ISP_SCALER40_SYSTEM_WORKAROUND_OFF_2 0x0000090C
#define ISP_SCALER40_SYSTEM_WORKAROUND_LEN_2 1

/* Y Scaler Config Cmd*/
typedef struct ISP_Y_ScaleCfgCmdType {
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
}__attribute__((packed, aligned(4))) ISP_Y_ScaleCfgCmdType;

/* CbCr Scaler Config Cmd*/
typedef struct ISP_CbCr_ScaleCfgCmdType {
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
}__attribute__((packed, aligned(4))) ISP_CbCr_ScaleCfgCmdType;

/* skin stats register to fake vfe1 scaler stripe reg:
     VFE1 only, hw limitation, this register is NOT double buffered
     so we can't update it on the fly*/
typedef struct ISP_Y_Stripe_WorkAround_CfgCmdType {
   /*VFE_1_SCALE_Y_H_STRIPE_CFG*/
  uint32_t     vfe1_horizMNInit                    :13;
  uint32_t     /* reserved */                      : 3;
  uint32_t     vfe1_horizPhaseInit                 :16;
}__attribute__((packed, aligned(4))) ISP_Y_Stripe_WorkAround_CfgCmdType;

/* Scaler Config Cmd Type*/
typedef struct ISP_ScaleCfgCmdType {
  ISP_Y_ScaleCfgCmdType     Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType  CbCr_ScaleCfg;
  ISP_Y_Stripe_WorkAround_CfgCmdType VFE1_Y_stripe_workaround_cfg;
}__attribute__((packed, aligned(4))) ISP_ScaleCfgCmdType;



#endif /* __SCALER40_REG_H__ */
