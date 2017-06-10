/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __SCALER_H__
#define __SCALER_H__

typedef enum {
  VFE_STANDARD_INPUT_TO_OUTPUT_1_SCALER,
  VFE_CASCADE_OUTPUT2_INPUT_TO_OUTPUT_1_SCALER,
  VFE_LAST_INPUT_SCALER_ENUM =
      VFE_CASCADE_OUTPUT2_INPUT_TO_OUTPUT_1_SCALER,
} VFE_ScaleInputSelectionType;
typedef enum {
  VFE_DISABLE_HORIZ_MN_Y_SCALER,
  VFE_ENABLE_HORIZ_MN_Y_SCALER,
  VFE_LAST_HORIZ_MN_Y_SCALER_ENABLE_ENUM = VFE_ENABLE_HORIZ_MN_Y_SCALER,  /* Used for count purposes only */
} VFE_HorizMN_Y_ScalerEnableType;

typedef enum {
  VFE_DISABLE_VERT_MN_Y_SCALER,
  VFE_ENABLE_VERT_MN_Y_SCALER,
  VFE_LAST_VERT_MN_Y_SCALER_ENABLE_ENUM = VFE_ENABLE_VERT_MN_Y_SCALER,    /* Used for count purposes only */
} VFE_VertMN_Y_ScalerEnableType;

typedef enum {
  VFE_DISABLE_HORIZ_POLYPHASE_Y_SCALER,
  VFE_ENABLE_HORIZ_POLYPHASE_Y_SCALER,
  VFE_LAST_HORIZ_POLYPHASE_Y_SCALER_ENUM = VFE_ENABLE_HORIZ_POLYPHASE_Y_SCALER,   /* Used for count purposes only */
} VFE_HorizPolyphaseY_ScalerEnableType;

typedef enum {
  VFE_DISABLE_VERT_POLYPHASE_Y_SCALER,
  VFE_ENABLE_VERT_POLYPHASE_Y_SCALER,
  VFE_LAST_VERT_POLYPHASE_Y_SCALER_ENUM = VFE_ENABLE_VERT_POLYPHASE_Y_SCALER,     /* Used for count purposes only */
} VFE_VertPolyphaseY_ScalerEnableType;

typedef enum {
  VFE_DISABLE_HORIZ_MN_CBCR_SCALER,
  VFE_ENABLE_HORIZ_MN_CBCR_SCALER,
  VFE_LAST_HORIZ_MN_CBCR_SCALER_ENABLE_ENUM = VFE_ENABLE_HORIZ_MN_CBCR_SCALER,    /* Used for count purposes only */
} VFE_HorizMN_CbCr_ScalerEnableType;

typedef enum {
  VFE_DISABLE_VERT_MN_CBCR_SCALER,
  VFE_ENABLE_VERT_MN_CBCR_SCALER,
  VFE_LAST_VERT_MN_CBCR_SCALER_ENABLE_ENUM = VFE_ENABLE_VERT_MN_CBCR_SCALER,      /* Used for count purposes only */
} VFE_VertMN_CbCr_ScalerEnableType;


typedef enum {
  VFE_DO_NOT_LOAD_NEW_POLYPHASE_COEFS_TO_HW,
  VFE_LOAD_NEW_POLYPHASE_COEFS_TO_HW,
  VFE_LAST_POLYPHASE_SCALSER_Y_LOAD_COEF_ENUM
      = VFE_LOAD_NEW_POLYPHASE_COEFS_TO_HW,
} VFE_PolyphaseScalerYLoadCoefType;

typedef struct {
  signed int polyphaseBankNScalerCoefBits9_2:8;
  unsigned int /* reserved */ :24;
  signed int polyphaseBankN48ScalerCoef:10;
  signed int polyphaseBankN32ScalerCoef:10;
  signed int polyphaseBankN16ScalerCoef:10;
  signed int polyphaseBankNScalerCoefBits1_0:2;
} __attribute__ ((packed, aligned(4))) PolyphaseScalerCoefType;

/* Scale Output 1 Config Command */
typedef struct {
   VFE_ScaleInputSelectionType scalerOutput1YInputSelection:1;
   VFE_HorizMN_Y_ScalerEnableType horizMN_Y_ScalerEnable:1;
   VFE_HorizPolyphaseY_ScalerEnableType horizPolyphaseY_ScalerEnable:1;
   VFE_VertMN_Y_ScalerEnableType vertMN_Y_ScalerEnable:1;
   VFE_VertPolyphaseY_ScalerEnableType vertPolyphaseY_ScalerEnable:1;
   VFE_ScaleInputSelectionType scalerOutput1CbCrInputSelection:1;
   VFE_HorizMN_CbCr_ScalerEnableType horizMN_CbCr_ScalerEnable:1;
   VFE_VertMN_CbCr_ScalerEnableType vertMN_CbCr_ScalerEnable:1;
   unsigned int /* reserved */ :24;
   /* MN Y Scaler Configuration, Part 1 */
   unsigned int widthOfInputToY_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   unsigned int heightOfInputToY_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   /* MN Y Scaler Configuration, Part 2 */
   unsigned int widthOfOutputOfY_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   unsigned int heightOfOutputOfY_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   /* MN CbCr Scaler Configuration, Part 1 */
   unsigned int widthOfInputToCbCr_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   unsigned int heightOfInputToCbCr_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   /* MN CbCr Scaler Configuration, Part 2 */
   unsigned int widthOfOutputOfCbCr_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   unsigned int heightOfOutputOfCbCr_MN_Scaler:12;
   unsigned int /* reserved */ :4;
   /* MN CbCr Scaler Configuration, Part 3 */
   unsigned int initialPhaseOfHorizMN_Scaler:15;
   unsigned int /* reserved */ :1;
   unsigned int initialPhaseOfVertMN_Scaler:15;
   unsigned int /* reserved */ :1;
   /* Polyphase Y Scaler Configuration, Part 1 */
   unsigned int widthOfInputToY_Polyphase_Scaler:12;
   unsigned int /* reserved */ :4;
   unsigned int heightOfInputToY_Polyphase_Scaler:12;
   unsigned int /* reserved */ :3;
   VFE_PolyphaseScalerYLoadCoefType loadY_Polyphase_ScalerCoefs:1;
   /* Polyphase Y Scaler Configuration, Part 2 */
   unsigned int widthOfOutputOfY_Polyphase_Scaler:12;
   unsigned int /* reserved */ :4;
   unsigned int heightOfOutputOfY_Polyphase_Scaler:12;
   unsigned int /* reserved */ :4;
   /* Polyphase Y Scaler Vertical Coefficients */
   PolyphaseScalerCoefType
     vertPolyphaseScalerCoef[NUM_VERT_POLY_SCALER_BANKS];
   /* Polyphase Y Scaler Horizontal Coefficients */
   PolyphaseScalerCoefType
     horizPolyphaseScalerCoef[NUM_HORIZ_POLY_SCALER_BANKS];
} __attribute__ ((packed, aligned(4))) vfe_cmd_scaleoutput1_t;

/* Scale Output 2 Config Command */
typedef struct {
  /* Scale Output 2 Selection */
  unsigned int dummy1:1;
  VFE_HorizMN_Y_ScalerEnableType horizMN_Y_ScalerEnable:1;
  VFE_HorizPolyphaseY_ScalerEnableType horizPolyphaseY_ScalerEnable:1;
  VFE_VertMN_Y_ScalerEnableType vertMN_Y_ScalerEnable:1;
  VFE_VertPolyphaseY_ScalerEnableType vertPolyphaseY_ScalerEnable:1;
  unsigned int dummy2:1;
  VFE_HorizMN_CbCr_ScalerEnableType horizMN_CbCr_ScalerEnable:1;
  VFE_VertMN_CbCr_ScalerEnableType vertMN_CbCr_ScalerEnable:1;
  unsigned int /* reserved */ :24;
  /* MN Y Scaler Configuration, Part 1 */
  unsigned int widthOfInputToY_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  unsigned int heightOfInputToY_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  /* MN Y Scaler Configuration, Part 2 */
  unsigned int widthOfOutputOfY_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  unsigned int heightOfOutputOfY_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  /* MN CbCr Scaler Configuration, Part 1 */
  unsigned int widthOfInputToCbCr_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  unsigned int heightOfInputToCbCr_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  /* MN CbCr Scaler Configuration, Part 2 */
  unsigned int widthOfOutputOfCbCr_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  unsigned int heightOfOutputOfCbCr_MN_Scaler:12;
  unsigned int /* reserved */ :4;
  /* MN CbCr Scaler Configuration, Part 3 */
  unsigned int initialPhaseOfHorizMN_Scaler:15;
  unsigned int /* reserved */ :1;
  unsigned int initialPhaseOfVertMN_Scaler:15;
  unsigned int /* reserved */ :1;
  /* Polyphase Y Scaler Configuration, Part 1 */
  unsigned int widthOfInputToY_Polyphase_Scaler:12;
  unsigned int /* reserved */ :4;
  unsigned int heightOfInputToY_Polyphase_Scaler:12;
  unsigned int /* reserved */ :3;
  VFE_PolyphaseScalerYLoadCoefType loadY_Polyphase_ScalerCoefs:1;
  /* Polyphase Y Scaler Configuration, Part 2 */
  unsigned int widthOfOutputOfY_Polyphase_Scaler:12;
  unsigned int /* reserved */ :4;
  unsigned int heightOfOutputOfY_Polyphase_Scaler:12;
  unsigned int /* reserved */ :4;
  /* Polyphase Y Scaler Vertical Coefficients */
   PolyphaseScalerCoefType
      vertPolyphaseScalerCoef[NUM_VERT_POLY_SCALER_BANKS];
  /* Polyphase Y Scaler Horizontal Coefficients */
   PolyphaseScalerCoefType
      horizPolyphaseScalerCoef[NUM_HORIZ_POLY_SCALER_BANKS];
} __attribute__ ((packed, aligned(4))) vfe_cmd_scaleoutput2_t;

typedef struct {
  vfe_cmd_scaleoutput1_t VFE_ScaleOutput1CfgCmd;
  vfe_cmd_scaleoutput2_t VFE_ScaleOutput2CfgCmd;
  uint8_t scaler_update;
  uint8_t scaler_enable;
}scaler_mod_t;

vfe_status_t vfe_scaler_config(scaler_mod_t *scaler_mod,
  vfe_params_t *vfe_params, current_output_info_t *vfe_out);
vfe_status_t vfe_scaler_update(scaler_mod_t *scaler_mod,
  vfe_params_t *vfe_params, current_output_info_t *vfe_out);
vfe_status_t vfe_scaler_enable(scaler_mod_t* scaler_mod,
  vfe_params_t* vfe_params, current_output_info_t *vfe_out,
  int8_t enable, int8_t hw_write);
#endif //__SCALER_H__
