#ifndef ABA_SVI_H
#define ABA_SVI_H

#ifdef __cplusplus
extern "C"
{
#endif

  /*=================================================================================================

  File: aba_avi.h

  DESCRIPTION
  This file contains the declaration for types that are specific to SVI.
  These types are SVI configuration parameters.

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
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

#define ABA_SVI_LUX_ADJUSTMENT_TABLE_LENGTH 32
#define ABA_SVI_VERSION   4

  /*=================================================================================================
  Types
  =================================================================================================*/
#include <stdint.h>

  /* SVIConfigParametersType
  Defines generic SVI parameters. Use when get/set-ing default/OEM parameters.
  */
  typedef struct
  {
    uint32_t   uContrastStrengthFactor;              // 0 - 255, histogram segmentation (content based contrast gain)
    uint32_t   uFilterStepSizeUIMode;                // Speed of the temporal filter in UI mode.
    uint32_t   uFilterStepSizeVideoMode;             // Speed of the temporal filter in video mode.
    uint32_t   uBrightnessStrengthFactor;            // 0 - 255, use to control maximal low centroid boost ratio
    uint32_t  *pSensorMappingTableReference;         // Lux Mapping table for sensor calibration - reference axis
    uint32_t  *pSensorMappingTableSensor;            // Lux Mapping table for sensor calibration - sensor axis
    uint32_t   uSensorMappingTableLength;            // Lux Mapping table length
    uint32_t   uPanelReflectanceRatio;               // Reflectance ratio for the panel
    uint32_t   uPanelPeakBrightness;                 // Peak brightness for the panel
    uint32_t  *pBacklightResponseTableInput;         // Backlight mapping table - input(user) axis
    uint32_t  *pBacklightResponseTableOutput;        // Backlight mapping table - output axis
    uint32_t   uBacklightResponseTableLength;        // Backlight Mapping table length
    uint32_t   uBacklightReductionRatio;             // Value 0-1024 indicating the reduction ratio relative to the original backlight
    uint32_t   uBacklightReductionMode;              // 1:  adaptive backlight, 0: constant backlight
    bool32     bEnableBacklightReduction;            // TRUE or FALSE, use to enable backlight/brightness reduction
    uint32_t   uIndoorMinLuxLevel;                   // Lower limit for indoor lux level
    uint32_t   uIndoorMinBrightnessLevel;            // Brighntess level desired for uIndoorMinLuxLevel
    uint32_t   uIndoorMaxLuxLevel;                   // Upper limit for indoor lux level
    uint32_t   uIndoorMaxBrightnessLevel;            // Brightness level desired for uIndoorMaxLuxLevel
    uint32_t   uOutdoorLuxlevel;                     // Upper limit for outdoor lux level
    uint32_t   uOutdoorMaxBrightnessLevel;           // Brightness level desired for uOutdoorLuxlevel
    uint32_t   uLuxStartPoint;                       // Ambient light level at which to start SVI
  } SVIConfigParametersType;


  /* SVIDynamicConfigType
  Use for dynamic configuration of SVI parameters. Most parameters are extracted from the static config structure.
  */
  typedef struct
  {
    uint32_t   uContrastStrengthFactor;              // 0 - 255, content based contrast gain
    uint32_t   uBrightnessStrengthFactor;            // 0 - 255, content based brightness gain
    uint32_t   uFilterStepSizeUIMode;                // Speed of the temporal filter in UI mode.
    uint32_t   uFilterStepSizeVideoMode;             // Speed of the temporal filter in video mode.
    uint32_t   uIndoorMaxBrightnessLevel;            // Brightness level desired for uIndoorMaxLuxLevel
    uint32_t   uOutdoorLuxlevel;                     // Upper limit for outdoor lux level
    uint32_t   uOutdoorMaxBrightnessLevel;           // Brightness level desired for uOutdoorLuxlevel
  } SVIDynamicConfigType;
#ifdef __cplusplus
}
#endif

#endif /* ABA_SVI_H */
