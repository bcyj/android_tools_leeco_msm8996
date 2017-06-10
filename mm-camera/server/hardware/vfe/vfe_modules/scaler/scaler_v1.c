/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_SCALER_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#ifndef VFE_CMD_SCALE_OUTPUT1_CONFIG
#define VFE_CMD_SCALE_OUTPUT1_CONFIG 131
#define VFE_CMD_SCALE_OUTPUT2_CONFIG 135
#endif

#define VFE_DOWNSCALER_MN_FACTOR_OFFSET 13

int16_t PolyphaseFilterCoeffs[NUM_POLY_SCALER_COEFFS]
[NUM_VERT_POLY_SCALER_BANKS * 4] =
{
  {
    0, 111, 306, 95,
    1, 127, 303, 81,
    2, 144, 298, 68,
    4, 161, 292, 55,
    6, 179, 283, 44,
    8, 196, 272, 36,
    12, 213, 259, 28,
    16, 229, 245, 22,
    21, 245, 229, 17,
    28, 259, 213, 12,
    36, 272, 196, 8,
    45, 283, 179, 5,
    56, 292, 161, 3,
    68, 298, 144, 2,
    81, 303, 127, 1,
    95, 306, 111, 0,
  },
  {
    -1, 107, 314, 92,
    0, 124, 312, 76,
    1, 142, 307, 62,
    2, 160, 299, 51,
    3, 178, 289, 42,
    6, 197, 278, 31,
    9, 215, 264, 24,
    13, 232, 249, 18,
    18, 249, 232, 13,
    24, 264, 215, 9,
    32, 278, 197, 5,
    41, 289, 178, 4,
    51, 299, 160, 2,
    63, 307, 142, 0,
    77, 312, 124, -1,
    91, 314, 107, 0,
  },
  {
    -2, 103, 324, 87,
    -2, 121, 321, 72,
    -1, 139, 315, 59,
    0, 158, 307, 47,
    1, 178, 297, 36,
    3, 197, 284, 28,
    6, 217, 269, 20,
    9, 235, 253, 15,
    14, 253, 235, 10,
    20, 269, 217, 6,
    28, 284, 197, 3,
    36, 297, 178, 1,
    47, 307, 158, 0,
    59, 315, 139, -1,
    72, 321, 121, -2,
    87, 324, 103, -2,
  },
  {
    -4, 99, 334, 83,
    -3, 117, 331, 67,
    -3, 136, 325, 54,
    -2, 156, 316, 42,
    -1, 177, 304, 32,
    1, 198, 291, 22,
    3, 218, 275, 16,
    6, 238, 257, 11,
    10, 257, 238, 7,
    16, 275, 218, 3,
    23, 291, 198, 0,
    32, 304, 177, -1,
    42, 316, 156, -2,
    54, 325, 136, -3,
    67, 331, 117, -3,
    82, 334, 99, -3,
  },
  {
    -5, 95, 344, 78,
    -4, 113, 341, 62,
    -4, 133, 335, 48,
    -3, 154, 325, 36,
    -3, 176, 313, 26,
    -2, 198, 298, 18,
    0, 220, 280, 12,
    3, 241, 261, 7,
    7, 261, 241, 3,
    12, 280, 220, 0,
    18, 298, 198, -2,
    27, 313, 176, -4,
    36, 325, 154, -3,
    48, 335, 133, -4,
    62, 341, 113, -4,
    77, 344, 95, -4,
  },
  {
    -5, 90, 356, 71,
    -5, 109, 352, 56,
    -5, 130, 345, 42,
    -5, 152, 334, 31,
    -5, 174, 321, 22,
    -4, 198, 305, 13,
    -2, 221, 286, 7,
    0, 244, 266, 2,
    3, 266, 244, -1,
    7, 286, 221, -2,
    14, 305, 198, -5,
    21, 321, 174, -4,
    31, 334, 152, -5,
    43, 345, 130, -6,
    56, 352, 109, -5,
    73, 356, 90, -7,
  },
  {
    -6, 84, 367, 67,
    -6, 104, 363, 51,
    -6, 126, 356, 36,
    -6, 149, 344, 25,
    -6, 173, 330, 15,
    -6, 197, 312, 9,
    -5, 222, 292, 3,
    -3, 247, 270, -2,
    -1, 270, 247, -4,
    3, 292, 222, -5,
    9, 312, 197, -6,
    16, 330, 173, -7,
    25, 344, 149, -6,
    37, 356, 126, -7,
    50, 363, 104, -5,
    66, 367, 84, -5,
  },
  {
    -6, 79, 380, 59,
    -7, 99, 375, 45,
    -7, 121, 367, 31,
    -7, 145, 355, 19,
    -8, 170, 339, 11,
    -8, 197, 320, 3,
    -7, 223, 298, -2,
    -6, 249, 274, -5,
    -4, 274, 249, -7,
    -1, 298, 223, -8,
    4, 320, 197, -9,
    11, 339, 170, -8,
    20, 355, 145, -8,
    31, 367, 121, -7,
    44, 375, 99, -6,
    61, 380, 79, -8,
  },
  {
    -6, 73, 393, 52,
    -7, 93, 388, 38,
    -8, 116, 379, 25,
    -8, 141, 365, 14,
    -9, 168, 348, 5,
    -10, 195, 327, 0,
    -10, 224, 304, -6,
    -10, 251, 278, -7,
    -9, 278, 251, -8,
    -5, 304, 224, -11,
    -2, 327, 195, -8,
    5, 348, 168, -9,
    14, 365, 141, -8,
    24, 379, 116, -7,
    38, 388, 93, -7,
    54, 393, 73, -8,
  },
  {
    -6, 66, 406, 46,
    -7, 87, 400, 32,
    -8, 111, 391, 18,
    -9, 137, 376, 8,
    -10, 165, 358, -1,
    -11, 194, 335, -6,
    -12, 224, 310, -10,
    -13, 253, 282, -10,
    -12, 282, 253, -11,
    -9, 310, 224, -13,
    -6, 335, 194, -11,
    0, 358, 165, -11,
    8, 376, 137, -9,
    18, 391, 111, -8,
    31, 400, 87, -6,
    48, 406, 66, -8,
  },
  {
    -5, 59, 420, 38,
    -7, 80, 414, 25,
    -8, 105, 403, 12,
    -9, 132, 387, 2,
    -11, 161, 367, -5,
    -12, 192, 343, -11,
    -14, 223, 315, -12,
    -15, 254, 286, -13,
    -15, 286, 254, -13,
    -14, 315, 223, -12,
    -10, 343, 192, -13,
    -5, 367, 161, -11,
    2, 387, 132, -9,
    11, 403, 105, -7,
    24, 414, 80, -6,
    40, 420, 59, -7,
  },
  {
    -5, 52, 434, 31,
    -6, 74, 428, 16,
    -8, 98, 416, 6,
    -9, 126, 399, -4,
    -11, 157, 377, -11,
    -13, 189, 351, -15,
    -16, 222, 321, -15,
    -17, 256, 289, -16,
    -18, 289, 256, -15,
    -17, 321, 222, -14,
    -14, 351, 189, -14,
    -11, 377, 157, -11,
    -4, 399, 126, -9,
    5, 416, 98, -7,
    18, 428, 74, -8,
    33, 434, 52, -7,
  },
  {
    -4, 44, 449, 23,
    -5, 66, 442, 9,
    -7, 91, 429, -1,
    -9, 120, 410, -9,
    -11, 152, 387, -16,
    -13, 186, 359, -20,
    -16, 221, 326, -19,
    -19, 257, 292, -18,
    -20, 292, 257, -17,
    -20, 326, 221, -15,
    -19, 359, 186, -14,
    -16, 387, 152, -11,
    -10, 410, 120, -8,
    -2, 429, 91, -6,
    10, 442, 66, -6,
    25, 449, 44, -6,
  },
  {
    -3, 36, 464, 15,
    -4, 58, 457, 1,
    -6, 85, 443, -10,
    -9, 114, 422, -15,
    -11, 147, 397, -21,
    -14, 182, 366, -22,
    -17, 219, 332, -22,
    -20, 257, 295, -20,
    -22, 295, 257, -18,
    -23, 332, 219, -16,
    -23, 366, 182, -13,
    -20, 397, 147, -12,
    -16, 422, 114, -8,
    -8, 443, 85, -8,
    3, 457, 58, -6,
    17, 464, 36, -5,
  },
  {
    -2, 27, 479, 8,
    -3, 50, 471, -6,
    -6, 76, 456, -14,
    -8, 106, 434, -20,
    -10, 141, 407, -26,
    -13, 178, 374, -27,
    -17, 217, 337, -25,
    -21, 257, 298, -22,
    -23, 298, 257, -20,
    -26, 337, 217, -16,
    -26, 374, 178, -14,
    -25, 407, 141, -11,
    -21, 434, 106, -7,
    -15, 456, 76, -5,
    -4, 471, 50, -5,
    9, 479, 27, -3,
  },
  {
    -1, 19, 494, 0,
    -2, 41, 486, -13,
    -4, 68, 470, -22,
    -7, 99, 446, -26,
    -10, 134, 416, -28,
    -13, 174, 381, -30,
    -17, 214, 341, -26,
    -21, 257, 300, -24,
    -24, 300, 257, -21,
    -28, 341, 214, -15,
    -29, 381, 174, -14,
    -29, 416, 134, -9,
    -27, 446, 99, -6,
    -21, 470, 68, -5,
    -12, 486, 41, -3,
    1, 494, 19, -2,
  },
  {
    0, 10, 510, -8,
    -1, 32, 501, -20,
    -3, 59, 484, -28,
    -6, 91, 458, -31,
    -8, 127, 426, -33,
    -12, 168, 388, -32,
    -17, 211, 346, -28,
    -21, 256, 301, -24,
    -25, 301, 256, -20,
    -29, 346, 211, -16,
    -31, 388, 168, -13,
    -32, 426, 127, -9,
    -31, 458, 91, -6,
    -26, 484, 59, -5,
    -19, 501, 32, -2,
    -7, 510, 10, -1,
  }
};

/*===========================================================================
 * FUNCTION    - scalar_coeffs_val -
 *
 * DESCRIPTION:
 *==========================================================================*/
inline static int16_t *scalar_coeffs_val(int c, int r)
{
	return &(PolyphaseFilterCoeffs[c][r]);
}/*scalar_coeffs_val*/

/*===========================================================================
 * FUNCTION    - vfe_main_scaler_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static vfe_status_t vfe_scaler1_config(vfe_cmd_scaleoutput1_t
  *VFE_ScaleOutput1CfgCmd, vfe_params_t *vfe_params)
{
  vfe_status_t rc = VFE_SUCCESS;
  float polyphase_ratio = 0.0;
  int i, j;
  int16_t *pcoeffs;
  uint32_t input_width, input_height,output_width, output_height;
  unsigned int scale_factor_horiz, scale_factor_vert;
  //TODO: need to check the code again for zoom
  // for now getting the dimesions from active crop
  input_width = vfe_params->crop_info.crop_last_pixel -
                  vfe_params->crop_info.crop_first_pixel + 1;;
  input_height = vfe_params->crop_info.crop_last_line -
                   vfe_params->crop_info.crop_first_line + 1;

  output_width = vfe_params->output1w;
  output_height = vfe_params->output1h;

  scale_factor_horiz = input_width / output_width;
  scale_factor_vert = input_height / output_height;

  /* Save this value for sharpness control */
  vfe_params->sharpness_info.downscale_factor =
    1.0 * ((float)input_width / (float)output_width);

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    CDBG("Output2 is configure larger than camsensor FOV.\n");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
    vfe_params->sharpness_info.downscale_factor = 1.0;
  }

  VFE_ScaleOutput1CfgCmd->scalerOutput1YInputSelection =
                    VFE_STANDARD_INPUT_TO_OUTPUT_1_SCALER;
  VFE_ScaleOutput1CfgCmd->horizMN_Y_ScalerEnable =
                    VFE_ENABLE_HORIZ_MN_Y_SCALER;
  VFE_ScaleOutput1CfgCmd->vertMN_Y_ScalerEnable =
                    VFE_ENABLE_VERT_MN_Y_SCALER;
  VFE_ScaleOutput1CfgCmd->scalerOutput1CbCrInputSelection =
                    VFE_STANDARD_INPUT_TO_OUTPUT_1_SCALER;
  VFE_ScaleOutput1CfgCmd->horizMN_CbCr_ScalerEnable =
                    VFE_ENABLE_HORIZ_MN_CBCR_SCALER;
  VFE_ScaleOutput1CfgCmd->vertMN_CbCr_ScalerEnable =
                    VFE_ENABLE_VERT_MN_CBCR_SCALER;
  VFE_ScaleOutput1CfgCmd->widthOfInputToY_MN_Scaler = input_width;
  VFE_ScaleOutput1CfgCmd->heightOfInputToY_MN_Scaler = input_height;
  VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_MN_Scaler =
      VFE_ScaleOutput1CfgCmd->widthOfInputToY_MN_Scaler /
                    scale_factor_horiz;
  VFE_ScaleOutput1CfgCmd->heightOfOutputOfY_MN_Scaler =
      VFE_ScaleOutput1CfgCmd->heightOfInputToY_MN_Scaler /
                    scale_factor_vert;
  VFE_ScaleOutput1CfgCmd->widthOfInputToY_Polyphase_Scaler =
      VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_MN_Scaler;
  VFE_ScaleOutput1CfgCmd->heightOfInputToY_Polyphase_Scaler =
      VFE_ScaleOutput1CfgCmd->heightOfOutputOfY_MN_Scaler;
  VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_Polyphase_Scaler = output_width;
  VFE_ScaleOutput1CfgCmd->heightOfOutputOfY_Polyphase_Scaler = output_height;
  VFE_ScaleOutput1CfgCmd->widthOfInputToCbCr_MN_Scaler = input_width;
  VFE_ScaleOutput1CfgCmd->heightOfInputToCbCr_MN_Scaler = input_height;
  VFE_ScaleOutput1CfgCmd->widthOfOutputOfCbCr_MN_Scaler = output_width;
  VFE_ScaleOutput1CfgCmd->heightOfOutputOfCbCr_MN_Scaler = output_height;

  if (VFE_ScaleOutput1CfgCmd->widthOfInputToY_Polyphase_Scaler ==
    VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_Polyphase_Scaler)
    VFE_ScaleOutput1CfgCmd->horizPolyphaseY_ScalerEnable =
      VFE_DISABLE_HORIZ_POLYPHASE_Y_SCALER;
  else
    VFE_ScaleOutput1CfgCmd->horizPolyphaseY_ScalerEnable =
      VFE_ENABLE_HORIZ_POLYPHASE_Y_SCALER;

  if (VFE_ScaleOutput1CfgCmd->heightOfInputToY_Polyphase_Scaler ==
    VFE_ScaleOutput1CfgCmd->heightOfOutputOfY_Polyphase_Scaler)
    VFE_ScaleOutput1CfgCmd->vertPolyphaseY_ScalerEnable =
      VFE_DISABLE_VERT_POLYPHASE_Y_SCALER;
  else
    VFE_ScaleOutput1CfgCmd->vertPolyphaseY_ScalerEnable =
      VFE_ENABLE_VERT_POLYPHASE_Y_SCALER;

  polyphase_ratio =
    (float)(VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_Polyphase_Scaler) /
    (float)(VFE_ScaleOutput1CfgCmd->widthOfInputToY_Polyphase_Scaler);

  j = (int)((polyphase_ratio - 0.5) * 32 + 0.5);

                /* Polyphase Y Scaler Horizontal Coefficients */
  for (i = 0; i < NUM_HORIZ_POLY_SCALER_BANKS; i++) {

   pcoeffs = scalar_coeffs_val(j, i * 4);
   VFE_ScaleOutput1CfgCmd->horizPolyphaseScalerCoef[i].
     polyphaseBankNScalerCoefBits1_0 = (*pcoeffs & 0x3);
   VFE_ScaleOutput1CfgCmd->horizPolyphaseScalerCoef[i].
     polyphaseBankNScalerCoefBits9_2 =
     ((*pcoeffs >> 2) & 0xFF);
   VFE_ScaleOutput1CfgCmd->horizPolyphaseScalerCoef[i].
     polyphaseBankN16ScalerCoef = (*(++pcoeffs) & 0x3FF);
   VFE_ScaleOutput1CfgCmd->horizPolyphaseScalerCoef[i].
     polyphaseBankN32ScalerCoef = (*(++pcoeffs) & 0x3FF);
   VFE_ScaleOutput1CfgCmd->horizPolyphaseScalerCoef[i].
     polyphaseBankN48ScalerCoef = (*(++pcoeffs) & 0x3FF);
  }

  polyphase_ratio = (float)(VFE_ScaleOutput1CfgCmd->
    heightOfOutputOfY_Polyphase_Scaler) /
    (float)(VFE_ScaleOutput1CfgCmd->heightOfInputToY_Polyphase_Scaler);

  j = (int)((polyphase_ratio - 0.5) * 32 + 0.5);

  /* Polyphase Y Scaler Vertical Coefficients */
  for (i = 0; i < NUM_VERT_POLY_SCALER_BANKS; i++) {
    pcoeffs = scalar_coeffs_val(j, i * 4);
    VFE_ScaleOutput1CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankNScalerCoefBits1_0 = (*pcoeffs & 0x3);
    VFE_ScaleOutput1CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankNScalerCoefBits9_2 =
      ((*pcoeffs >> 2) & 0xFF);
    VFE_ScaleOutput1CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankN16ScalerCoef = (*(++pcoeffs) & 0x3FF);
    VFE_ScaleOutput1CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankN32ScalerCoef = (*(++pcoeffs) & 0x3FF);
    VFE_ScaleOutput1CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankN48ScalerCoef = (*(++pcoeffs) & 0x3FF);
  }

  VFE_ScaleOutput1CfgCmd->loadY_Polyphase_ScalerCoefs =
    VFE_LOAD_NEW_POLYPHASE_COEFS_TO_HW;

  CDBG("horizMN_Y_ScalerEnable             = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->horizMN_Y_ScalerEnable);
  CDBG("horizPolyphaseY_ScalerEnable       = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->horizPolyphaseY_ScalerEnable);
  CDBG("vertMN_Y_ScalerEnable              = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->vertMN_Y_ScalerEnable);
  CDBG("vertPolyphaseY_ScalerEnable        = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->vertPolyphaseY_ScalerEnable);
  CDBG("horizMN_CbCr_ScalerEnable          = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->horizMN_CbCr_ScalerEnable);
  CDBG("vertMN_CbCr_ScalerEnable           = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->vertMN_CbCr_ScalerEnable);
  CDBG("widthOfInputToY_MN_Scaler          = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->widthOfInputToY_MN_Scaler);
  CDBG("heightOfInputToY_MN_Scaler         = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->heightOfInputToY_MN_Scaler);
  CDBG("widthOfOutputOfY_MN_Scaler         = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_MN_Scaler);
  CDBG("heightOfOutputOfY_MN_Scaler        = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->heightOfOutputOfY_MN_Scaler);
  CDBG("widthOfInputToCbCr_MN_Scaler       = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->widthOfInputToCbCr_MN_Scaler);
  CDBG("heightOfInputToCbCr_MN_Scaler      = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->heightOfInputToCbCr_MN_Scaler);
  CDBG("widthOfOutputOfCbCr_MN_Scaler      = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->widthOfOutputOfCbCr_MN_Scaler);
  CDBG("heightOfOutputOfCbCr_MN_Scaler     = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->heightOfOutputOfCbCr_MN_Scaler);
  CDBG("initialPhaseOfHorizMN_Scaler       = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->initialPhaseOfHorizMN_Scaler);
  CDBG("initialPhaseOfVertMN_Scaler        = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->initialPhaseOfVertMN_Scaler);
  CDBG("widthOfInputToY_Polyphase_Scaler   = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->widthOfInputToY_Polyphase_Scaler);
  CDBG("heightOfInputToY_Polyphase_Scaler  = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->heightOfInputToY_Polyphase_Scaler);
  CDBG("loadY_Polyphase_ScalerCoefs        = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->loadY_Polyphase_ScalerCoefs);
  CDBG("widthOfOutputOfY_Polyphase_Scaler  = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->widthOfOutputOfY_Polyphase_Scaler);
  CDBG("heightOfOutputOfY_Polyphase_Scaler = 0x%x\n",
    VFE_ScaleOutput1CfgCmd->heightOfOutputOfY_Polyphase_Scaler);
  CDBG("vertPolyphaseScalerCoef            = %p\n",
    VFE_ScaleOutput1CfgCmd->vertPolyphaseScalerCoef);
  CDBG("horizPolyphaseScalerCoef           = %p\n",
    VFE_ScaleOutput1CfgCmd->horizPolyphaseScalerCoef);

  vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)VFE_ScaleOutput1CfgCmd,
    sizeof(*VFE_ScaleOutput1CfgCmd), VFE_CMD_SCALE_OUTPUT1_CONFIG);

  return rc;
} /* vfe_main_scaler_config */

/*===========================================================================
 * FUNCTION    - VFE_Output1_YScaleCfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler2_config(vfe_cmd_scaleoutput2_t *VFE_ScaleOutput2CfgCmd,
  vfe_params_t *vfe_params)
{
  vfe_status_t rc = VFE_SUCCESS;
  uint32_t input_width, input_height,output_width, output_height;
  unsigned int scale_factor_horiz, scale_factor_vert;
  float polyphase_ratio = 0.0;
  int i, j;
  int16_t *pcoeffs;

  input_width = vfe_params->crop_info.crop_last_pixel -
                  vfe_params->crop_info.crop_first_pixel + 1;
  input_height = vfe_params->crop_info.crop_last_line -
                   vfe_params->crop_info.crop_first_line + 1;

  output_width = vfe_params->output2w;
  output_height = vfe_params->output2h;
  CDBG("input_width : %d %d\n", input_width, output_width);
  CDBG("input_height : %d %d\n", input_height, output_height);

  scale_factor_horiz = input_width / output_width;
  scale_factor_vert = input_height / output_height;

  /* Save this value for sharpness control */
  vfe_params->sharpness_info.downscale_factor =
    1.0 * ((float)input_width / (float)output_width);

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    CDBG("Output2 is configure larger than camsensor FOV.\n");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
    vfe_params->sharpness_info.downscale_factor = 1.0;
  }

	if (scale_factor_horiz == 1)
		VFE_ScaleOutput2CfgCmd->horizMN_Y_ScalerEnable =
		    VFE_DISABLE_HORIZ_MN_Y_SCALER;
	else
		VFE_ScaleOutput2CfgCmd->horizMN_Y_ScalerEnable =
		    VFE_ENABLE_HORIZ_MN_Y_SCALER;

	if (scale_factor_vert == 1)
		VFE_ScaleOutput2CfgCmd->vertMN_Y_ScalerEnable =
		    VFE_DISABLE_VERT_MN_Y_SCALER;
	else
		VFE_ScaleOutput2CfgCmd->vertMN_Y_ScalerEnable =
		    VFE_ENABLE_VERT_MN_Y_SCALER;

  VFE_ScaleOutput2CfgCmd->horizMN_CbCr_ScalerEnable =
    VFE_ENABLE_HORIZ_MN_CBCR_SCALER;
  VFE_ScaleOutput2CfgCmd->vertMN_CbCr_ScalerEnable =
    VFE_ENABLE_VERT_MN_CBCR_SCALER;
  VFE_ScaleOutput2CfgCmd->widthOfInputToY_MN_Scaler = input_width;
  VFE_ScaleOutput2CfgCmd->heightOfInputToY_MN_Scaler = input_height;
  VFE_ScaleOutput2CfgCmd->widthOfOutputOfY_MN_Scaler =
    VFE_ScaleOutput2CfgCmd->widthOfInputToY_MN_Scaler /
    scale_factor_horiz;
  VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_MN_Scaler =
    VFE_ScaleOutput2CfgCmd->heightOfInputToY_MN_Scaler /
    scale_factor_vert;
  VFE_ScaleOutput2CfgCmd->widthOfInputToY_Polyphase_Scaler =
    VFE_ScaleOutput2CfgCmd->widthOfOutputOfY_MN_Scaler;
  VFE_ScaleOutput2CfgCmd->heightOfInputToY_Polyphase_Scaler =
    VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_MN_Scaler;
  VFE_ScaleOutput2CfgCmd->widthOfOutputOfY_Polyphase_Scaler = output_width;
  VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_Polyphase_Scaler = output_height;
  VFE_ScaleOutput2CfgCmd->widthOfInputToCbCr_MN_Scaler = input_width;
  VFE_ScaleOutput2CfgCmd->heightOfInputToCbCr_MN_Scaler = input_height;
  VFE_ScaleOutput2CfgCmd->widthOfOutputOfCbCr_MN_Scaler = output_width/2;
  VFE_ScaleOutput2CfgCmd->heightOfOutputOfCbCr_MN_Scaler = output_height/2;

  if (VFE_ScaleOutput2CfgCmd->widthOfInputToY_Polyphase_Scaler ==
    VFE_ScaleOutput2CfgCmd->widthOfOutputOfY_Polyphase_Scaler)
     VFE_ScaleOutput2CfgCmd->horizPolyphaseY_ScalerEnable =
     VFE_DISABLE_HORIZ_POLYPHASE_Y_SCALER;
  else
    VFE_ScaleOutput2CfgCmd->horizPolyphaseY_ScalerEnable =
      VFE_ENABLE_HORIZ_POLYPHASE_Y_SCALER;

   if (VFE_ScaleOutput2CfgCmd->heightOfInputToY_Polyphase_Scaler ==
       VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_Polyphase_Scaler)
     VFE_ScaleOutput2CfgCmd->vertPolyphaseY_ScalerEnable =
       VFE_DISABLE_VERT_POLYPHASE_Y_SCALER;
    else
      VFE_ScaleOutput2CfgCmd->vertPolyphaseY_ScalerEnable =
        VFE_ENABLE_VERT_POLYPHASE_Y_SCALER;

    polyphase_ratio = (float)(VFE_ScaleOutput2CfgCmd->
      widthOfOutputOfY_Polyphase_Scaler) /
      (float)(VFE_ScaleOutput2CfgCmd->widthOfInputToY_Polyphase_Scaler);

    j = (int)((polyphase_ratio - 0.5) * 32 + 0.5);

  /* Polyphase Y Scaler Horizontal Coefficients */
  for (i = 0; i < NUM_HORIZ_POLY_SCALER_BANKS; i++) {
    pcoeffs = scalar_coeffs_val(j, i * 4);

    VFE_ScaleOutput2CfgCmd->horizPolyphaseScalerCoef[i].
      polyphaseBankNScalerCoefBits1_0 = (*pcoeffs & 0x3);
    VFE_ScaleOutput2CfgCmd->horizPolyphaseScalerCoef[i].
      polyphaseBankNScalerCoefBits9_2 = ((*pcoeffs >> 2) & 0xFF);
    VFE_ScaleOutput2CfgCmd->horizPolyphaseScalerCoef[i].
      polyphaseBankN16ScalerCoef = (*(++pcoeffs) & 0x3FF);
    VFE_ScaleOutput2CfgCmd->horizPolyphaseScalerCoef[i].
      polyphaseBankN32ScalerCoef = (*(++pcoeffs) & 0x3FF);
    VFE_ScaleOutput2CfgCmd->horizPolyphaseScalerCoef[i].
      polyphaseBankN48ScalerCoef = (*(++pcoeffs) & 0x3FF);
  }

  polyphase_ratio =
    (float)(VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_Polyphase_Scaler) /
    (float)(VFE_ScaleOutput2CfgCmd->heightOfInputToY_Polyphase_Scaler);

  j = (int)((polyphase_ratio - 0.5) * 32 + 0.5);

  /* Polyphase Y Scaler Vertical Coefficients */
  for (i = 0; i < NUM_VERT_POLY_SCALER_BANKS; i++) {
    pcoeffs = scalar_coeffs_val(j, i * 4);
    VFE_ScaleOutput2CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankNScalerCoefBits1_0 = (*pcoeffs & 0x3);
    VFE_ScaleOutput2CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankNScalerCoefBits9_2 = ((*pcoeffs >> 2) & 0xFF);
    VFE_ScaleOutput2CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankN16ScalerCoef = (*(++pcoeffs) & 0x3FF);
    VFE_ScaleOutput2CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankN32ScalerCoef = (*(++pcoeffs) & 0x3FF);
    VFE_ScaleOutput2CfgCmd->vertPolyphaseScalerCoef[i].
      polyphaseBankN48ScalerCoef = (*(++pcoeffs) & 0x3FF);
  }

  VFE_ScaleOutput2CfgCmd->loadY_Polyphase_ScalerCoefs =
    VFE_LOAD_NEW_POLYPHASE_COEFS_TO_HW;

  CDBG("horizMN_Y_ScalerEnable             = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->horizMN_Y_ScalerEnable);
  CDBG("horizPolyphaseY_ScalerEnable       = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->horizPolyphaseY_ScalerEnable);
  CDBG("vertMN_Y_ScalerEnable              = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->vertMN_Y_ScalerEnable);
  CDBG("vertPolyphaseY_ScalerEnable        = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->vertPolyphaseY_ScalerEnable);
  CDBG("dummy2                             = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->dummy2);
  CDBG("horizMN_CbCr_ScalerEnable          = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->horizMN_CbCr_ScalerEnable);
  CDBG("vertMN_CbCr_ScalerEnable           = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->vertMN_CbCr_ScalerEnable);
  CDBG("widthOfInputToY_MN_Scaler          = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->widthOfInputToY_MN_Scaler);
  CDBG("heightOfInputToY_MN_Scaler         = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->heightOfInputToY_MN_Scaler);
  CDBG("widthOfOutputOfY_MN_Scaler         = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->widthOfOutputOfY_MN_Scaler);
  CDBG("heightOfOutputOfY_MN_Scaler        = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_MN_Scaler);
  CDBG("widthOfInputToCbCr_MN_Scaler       = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->widthOfInputToCbCr_MN_Scaler);
  CDBG("heightOfInputToCbCr_MN_Scaler      = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->heightOfInputToCbCr_MN_Scaler);
  CDBG("widthOfOutputOfCbCr_MN_Scaler      = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->widthOfOutputOfCbCr_MN_Scaler);
  CDBG("heightOfOutputOfCbCr_MN_Scaler     = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->heightOfOutputOfCbCr_MN_Scaler);
  CDBG("initialPhaseOfHorizMN_Scaler       = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->initialPhaseOfHorizMN_Scaler);
  CDBG("initialPhaseOfVertMN_Scaler        = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->initialPhaseOfVertMN_Scaler);
  CDBG("widthOfInputToY_Polyphase_Scaler   = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->widthOfInputToY_Polyphase_Scaler);
  CDBG("heightOfInputToY_Polyphase_Scaler  = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->heightOfInputToY_Polyphase_Scaler);
  CDBG("loadY_Polyphase_ScalerCoefs        = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->loadY_Polyphase_ScalerCoefs);
  CDBG("widthOfOutputOfY_Polyphase_Scaler  = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->widthOfOutputOfY_Polyphase_Scaler);
  CDBG("heightOfOutputOfY_Polyphase_Scaler = 0x%x\n",
    VFE_ScaleOutput2CfgCmd->heightOfOutputOfY_Polyphase_Scaler);
  CDBG("vertPolyphaseScalerCoef            = %p\n",
    VFE_ScaleOutput2CfgCmd->vertPolyphaseScalerCoef);
  CDBG("horizPolyphaseScalerCoef           = %p\n",
    VFE_ScaleOutput2CfgCmd->horizPolyphaseScalerCoef);

  vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)VFE_ScaleOutput2CfgCmd,
    sizeof(*VFE_ScaleOutput2CfgCmd), VFE_CMD_SCALE_OUTPUT2_CONFIG);

  return rc;
} /* vfe_main_scaler_config */

/*===========================================================================
 * FUNCTION    - vfe_config_scaler -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_config(scaler_mod_t *scaler_mod,
  vfe_params_t *vfe_params, current_output_info_t *vfe_out)
{
  vfe_status_t rc = VFE_SUCCESS;

  /* main scaler is always on */
  rc = vfe_scaler2_config(&(scaler_mod->VFE_ScaleOutput2CfgCmd),vfe_params);
  if (rc != VFE_SUCCESS) {
    CDBG("%s:failed",__func__);
    return rc;
  }
  if (vfe_out->vfe_operation_mode == VFE_OUTPUTS_MAIN_AND_THUMB ||
      VFE_OUTPUTS_MAIN_AND_PREVIEW == vfe_out->vfe_operation_mode) {
    rc = vfe_scaler1_config(&(scaler_mod->VFE_ScaleOutput1CfgCmd),vfe_params);
    if (rc != VFE_SUCCESS) {
      CDBG("%s:failed",__func__);
    return rc;
    }
  }

  return rc;
} /* vfe_config_scaler */

/*===========================================================================
 * FUNCTION    - vfe_scaler_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_scaler_update(scaler_mod_t *scaler_mod, vfe_params_t *vfe_params,
  current_output_info_t *vfe_out)
{
  vfe_status_t status;

  if(scaler_mod->scaler_update) {
    status = vfe_scaler_config(scaler_mod,vfe_params, vfe_out);
    CDBG("%s: doing scaler_config", __func__);
    if (status != VFE_SUCCESS)
      CDBG("%s: Failed\n",__func__);
    else
      vfe_params->update |= VFE_MOD_SCALER;
  }
  scaler_mod->scaler_update = FALSE;
  return VFE_SUCCESS;
}

/*===========================================================================
 * Function:           vfe_scaler_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_scaler_enable(scaler_mod_t* scaler_mod,
  vfe_params_t* vfe_params, current_output_info_t *vfe_out,
  int8_t enable, int8_t hw_write)
{
  return VFE_SUCCESS;
} /* vfe_scaler_enable */
