/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCALER32_REG_H__
#define __SCALER32_REG_H__

#define ISP_SCALER32_MAIN_OFF 0x00000368
#define ISP_SCALER32_MAIN_LEN 7
#define ISP_SCALER32_Y_OFF    0x000004D0
#define ISP_SCALER32_Y_LEN    5
#define ISP_SCALER32_CBCR_OFF 0x000004E4
#define ISP_SCALER32_CBCR_LEN 5

typedef struct ISP_Main_Scaler_ConfigCmdType {
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
} __attribute__((packed, aligned(4))) ISP_Main_Scaler_ConfigCmdType;

typedef struct ISP_Output_YScaleCfgCmdType {
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
}__attribute__((packed, aligned(4))) ISP_Output_YScaleCfgCmdType;

typedef struct ISP_Output_CbCrScaleCfgCmdType {

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
}__attribute__((packed, aligned(4))) ISP_Output_CbCrScaleCfgCmdType;

typedef struct {
  ISP_Main_Scaler_ConfigCmdType main_scaler_cmd;
  ISP_Output_YScaleCfgCmdType y_scaler_cmd;
  ISP_Output_CbCrScaleCfgCmdType cbcr_scaler_cmd;
}ISP_SCaler_ConfigCmdType;

#endif //__SCALER32_REG_H__
