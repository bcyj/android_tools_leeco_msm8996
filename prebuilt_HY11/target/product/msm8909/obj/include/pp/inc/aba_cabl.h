#ifndef ABA_CABL_H
#define ABA_CABL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*=================================================================================================

  File: aba_cabl.h

  DESCRIPTION
  This file contains the declaration for types that are specific to CABL.

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=================================================================================================*/

/*=================================================================================================
  Defines
=================================================================================================*/

#ifndef TRUE
#define TRUE   1   /* Boolean true value. */
#endif

#ifndef FALSE
#define FALSE  0   /* Boolean false value. */
#endif

#ifndef _BOOL32_DEFINED
typedef  unsigned long int  bool32;        /* Boolean value type. */
#define _BOOL32_DEFINED
#endif

#define ABA_CABL_VERSION   6
/*=================================================================================================
  Types
=================================================================================================*/
#include <stdint.h>



/* CABLQualityLevelType
   Enumeration value describing the CABL quality level.
*/
typedef enum
{
    ABL_QUALITY_LOW,                                      // Low quality, high power savings
    ABL_QUALITY_NORMAL,                                   // Mediam qualtiy, moderate power savings
    ABL_QUALITY_HIGH,                                     // High quality, lowe power savings
    ABL_QUALITY_MAX,
    ABL_QUALITY_AUTO
} CABLQualityLevelType;

typedef enum
{
    USER_QL_MODE,
    AUTO_QL_MODE
} DEFAULT_QL_MODE;

/* CABLDebugInfoType
 * Use for debugging purposes. Contains CABL numbers for the latest frame.
*/
typedef struct
{
   uint32_t        uCutoffPoint;                         // Cutoff calculated from the histogram
   uint32_t        uCablBacklightPowerSaveRatio;         // Power saving ratio calculated by CABL
   uint32_t        uFilteredBacklightPowerSaveRatio;     // Filtered backlight ratio for gradual changes
   uint32_t        uLimitedBacklightPowerSaveRatio;      // Filtered backlight ration limited by uBacklightScaleRatioUpperLimit
   uint32_t        uBacklightPowerSaveRatioMin;          // Minimum ration to apply to the input backlight without going below the threshold
   int32_t         iLastBacklightRatioDelta;             // Ratio delta between two frames
   uint32_t        eState;                               // Current internal state of CABL
} CABLDebugInfoType;


/* CablQualityParametersType
  Defines quality level dependant parameters for CABL.
*/
typedef struct
{
   uint32_t        uBacklightScaleRatioLowerLimit;       // minimum ratio to be applied by CABL
   uint32_t        uBacklightScaleRatioUpperLimit;       // maximum ratio to be applied by CABL.
   uint32_t        uFilterStepSize;                      // refer to the deltas of temporal filtering for BL and LUT
   uint32_t        uPixelDistortionRate;                 // allowed percentage of saturated pixel

   // Reserved parameters for CABL process
   double          SoftClippingSlope;                    // Soft clipping slope for the LUT
   uint32_t        uLutType;                             // LUT type defines the shape of the c
   uint32_t        uWindowSizeThreshold;                 // Maximum value for the window size
   uint32_t        uFilterCoefficientThreshold;          // Minimum value for the filter coefficient
   uint32_t        uBacklightReductionFactor;            // Factor used to reduce the speed of the backlight change
   uint32_t        uBacklightStepSizeHighCorrelation;    // Backlight change delta allowed when b2b frames are similar
   uint32_t        uSceneCorrelationThreshold;           // Correlation value beyond which frames are considered similar
   uint32_t        uSceneChangeThreshold;                // Correlation value below which the scene is considered to have changed
} CablQualityParametersType;

/* CablInitialConfigType
  Defines generic CABL parameters. Use when get/set-ing default/OEM parameters.
*/
typedef struct
{
   // ABL debug message level, 0 to disable debug messages
   uint32_t                                     uDebugLevel;

   // Minimum backlight level. Range: 0-1024
   uint32_t                                     uBacklightThresholdLevel;

   // Quality level specific params
   CablQualityParametersType                    aCablQualityParameters[ABL_QUALITY_MAX];

   // Next parameters used for Gamma response LUT

   // Luminance
   uint32_t                                    *pGammaResponseY;

   // Grayscale
   uint32_t                                    *pGammaResponseX;

   uint32_t                                     uGammaResponseTableLength;

   // Next parameters used for Backlight response LUT

   // Backlight
   uint32_t                                    *pBacklightResponseX;

   // Luminance
   uint32_t                                    *pBacklightResponseY;

   uint32_t                                     uBacklightResponseTableLength;

   // Ambient light level at which to stop CABL
   uint32_t                                     uLuxEndPoint;

   // minimum backlight level converted from bl_level_threshold
   // range: 0-255
   uint32_t                                     bl_min_level;

   //CABL quality level mode
   uint32_t                                     default_ql_mode;
   //default UI and video quality levels
   CABLQualityLevelType                         ui_quality_lvl;
   CABLQualityLevelType                         video_quality_lvl;
} CablInitialConfigType;

#ifdef __cplusplus
}
#endif

#endif /* ABA_CABL_H */
