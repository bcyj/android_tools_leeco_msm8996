/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC40_REG_H__
#define __DEMOSAIC40_REG_H__

#define ISP_DEMOSAIC40_CLASSIFIER_CNT 18

#define ISP_DEMOSAIC40_OFF 0x00000440
#define ISP_DEMOSAIC40_LEN 1

#define ISP_DEMOSAIC_MIX_CFG_OFF 0x00000440
#define ISP_DEMOSAIC40_ENABLE_LEN 1

#define ISP_DEMOSAIC40_WB_GAIN_OFF 0x00000518
#define ISP_DEMOSAIC40_WB_GAIN_LEN 2

#define ISP_DEMOSAIC_CLASSIFIER_OFF 0x00000520
#define ISP_DEMOSAIC_CLASSIFIER_LEN 18

#define ISP_DEMOSAIC_INTERP_GAIN_OFF 0x00000568
#define ISP_DEMOSAIC_INTERP_GAIN_LEN 2

typedef struct ISP_Demosaic40InterpClassifierType {
  uint32_t w_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t t_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t l_n                             : 5;
  uint32_t   /* reserved */                : 2;
  uint32_t b_n                             : 1;
}__attribute__((packed, aligned(4))) ISP_Demosaic40InterpClassifierType;

typedef struct ISP_Demosaic40MixConfigCmdType{
  /* Demosaic Config */
   uint32_t  /*reserve*/                   : 4;

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
}__attribute__((packed, aligned(4))) ISP_Demosaic40MixConfigCmdType;

typedef struct ISP_Demosaic40ConfigCmdType {
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
  ISP_Demosaic40InterpClassifierType
    interpClassifier[ISP_DEMOSAIC40_CLASSIFIER_CNT];

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 16;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 3;
  uint32_t a                               : 6;
  uint32_t   /* reserved */                : 14;
}__attribute__((packed, aligned(4))) ISP_Demosaic40ConfigCmdType;

#if 0
typedef struct ISP_Demosaic40UpdateCmdType {
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
}__attribute__((packed, aligned(4))) ISP_Demosaic40UpdateCmdType;
#endif

#endif /* __DEMOSAIC40_REG_H__ */
