/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC32_REG_H__
#define __DEMOSAIC32_REG_H__

#define ISP_DEMOSAIC32_CLASSIFIER_CNT 18

#define ISP_DEMOSAIC32_OFF 0x00000298
#define ISP_DEMOSAIC32_LEN 1

#define ISP_DEMOSAIC_MIX_CFG_OFF 0x00000298
#define ISP_DEMOSAIC32_ENABLE_LEN 1

#define ISP_DEMOSAIC32_WB_GAIN_OFF 0x0000061C
#define ISP_DEMOSAIC32_WB_GAIN_LEN 2

#define ISP_DEMOSAIC_CLASSIFIER_OFF 0x00000624
#define ISP_DEMOSAIC_CLASSIFIER_LEN 18

#define ISP_DEMOSAIC_INTERP_GAIN_OFF 0x0000066C
#define ISP_DEMOSAIC_INTERP_GAIN_LEN 2

typedef struct ISP_Demosaic32InterpClassifierType {
  uint32_t w_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t t_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t l_n                             : 5;
  uint32_t   /* reserved */                : 2;
  uint32_t b_n                             : 1;
}__attribute__((packed, aligned(4))) ISP_Demosaic32InterpClassifierType;

typedef union  ISP_Demosaic32MixConfigCmdType {
  struct {
    /* Demosaic Config */
     uint32_t  /*reserve*/                 : 4;

     /* cosited rgb enable */
    uint32_t cositedRgbEnable              : 1;
    uint32_t   /* reserved */              : 3;

    /* abcc lut bank sel */
    uint32_t abccLutBankSel                : 1;
    uint32_t   /* reserved */              : 7;

    /* pipe flush count */
    uint32_t pipeFlushCount                : 13;
    uint32_t   /* reserved */              : 1;
    uint32_t pipeFlushOvd                  : 1;
    uint32_t flushHaltOvd                  : 1;
  }__attribute__((packed, aligned(4)));
  uint32_t cfg;
}__attribute__((packed, aligned(4))) ISP_Demosaic32MixConfigCmdType;


typedef struct ISP_Demosaic32ConfigCmdType {
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
  ISP_Demosaic32InterpClassifierType
    interpClassifier[ISP_DEMOSAIC32_CLASSIFIER_CNT];

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 16;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 3;
  uint32_t a                               : 6;
  uint32_t   /* reserved */                : 14;
}__attribute__((packed, aligned(4))) ISP_Demosaic32ConfigCmdType;

#if 0
typedef struct ISP_Demosaic32UpdateCmdType {
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
}__attribute__((packed, aligned(4))) ISP_Demosaic32UpdateCmdType;
#endif

#endif /* __DEMOSAIC32_REG_H__ */
