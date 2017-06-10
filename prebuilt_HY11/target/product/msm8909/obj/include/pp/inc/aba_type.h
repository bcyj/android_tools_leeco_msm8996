#ifndef ABA_TYPE_H
#define ABA_TYPE_H
/*=================================================================================================

File: aba_type.h
This file contains the declaration for ABA types.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=================================================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>


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

  //Returns the minimum value
#define M_MIN(a, b)  (((int32_t)(a) < (int32_t)(b)) ? (int32_t)(a) : (int32_t)(b))

  //Returns the maximum value
#define M_MAX(a, b)  (((int32_t)(a) > (int32_t)(b)) ? (int32_t)(a) : (int32_t)(b))

  //Returns the minimum value
#define M_MIN64(a, b)  (((int64_t)(a) < (int64_t)(b)) ? (int64_t)(a) : (int64_t)(b))

  //Returns the maximum value
#define M_MAX64(a, b)  (((int64_t)(a) > (int64_t)(b)) ? (int64_t)(a) : (int64_t)(b))

  //Returns absolute value
#define M_ABS(x)  (uint32_t)((((int32_t)x) >= 0) ? (uint32_t)(x) : (uint32_t)(-x))

  //Clip corresponding value
#define CLIP(x, up, lw) (((x)>(up)) ? up: ((x)<=(lw))? (lw):(x))

  //Scale to corresponding Q8 value
#define SCALEQ8(x, uf) (((x)/((1 << (8 - uf)) - 1)) * 255)

  // Returns the sign of an integer
#define M_SIGN(x)    (int32_t)((((int32_t)x) > 0)   ?  1  : (((int32_t)(x) < 0) ? (-1):0))

  // right shift
#define RS(a, b) (int32_t)((int32_t)((int32_t)M_ABS(a)>>b)*(int32_t)M_SIGN(a))

  // left shift
#define LS(a, b) (int32_t)((int32_t)((int32_t)M_ABS(a)<<b)*(int32_t)M_SIGN(a))

  // right shift
#define RS64(a, b) (int64_t)((int64_t)((int64_t)M_ABS(a)>>b)*(int64_t)M_SIGN(a))

  // left shift
#define LS64(a, b) (int64_t)((int64_t)((int64_t)M_ABS(a)<<b)*(int64_t)M_SIGN(a))

  //Maximum value for shade in the gamma response table
#define ABA_SHADE_MAX_VALUE              1024

  //Maximum value for gamma in the gamma response table
#define ABA_GAMMA_MAX_VALUE              1024

  //Maximum value for the client backlight level.
#define ABA_MAX_CLIENT_BACKLIGHT_LEVEL   255

  //Maximum value for the backlight ratio.
#define ABA_MAX_BACKLIGHT_RATIO          1024


  //List of ABA features

#define ABA_FEATURE_CABL                0x00000001
#define ABA_FEATURE_FOSS                0x00000002
#define ABA_FEATURE_SVI                 0x00000004




  /*=================================================================================================
  Types
  =================================================================================================*/

  /*
  AbaPanelType
  Enumeration value used to discribe the display panel.
  LCD_PANEL is associated with CABL.
  AMOLED_PANEL is associated with FOSS.
  */
  typedef enum
  {
    LCD_PANEL,
    AMOLED_PANEL
  } AbaPanelType;


  /* AbaConfigInfoType.
  Describes the configuration to be initialized by ABA.
  */
  typedef struct
  {
    uint32_t        eFeature;                        //Core algorithm CABL, SVI...
    AbaPanelType  ePanelType;                      //Panel type, LCD, OLED...
  } AbaConfigInfoType;



  /* AbaHardwareInfoType.
  Describes the configuration of the histogram/LUT for the target.
  */
  typedef struct
  {
    uint32_t uHistogramBins;            //Hardware histogram size: 128 bins or 256 bins
    uint32_t uHistogramComponents;      //3 for RGB, 1 for HSV
    uint32_t uLUTSize;                  //Hardware look up table size: e.g 256, 128, etc
    uint32_t uFactor;                   //reserved
    uint32_t uBlock;                    //reserved
  } AbaHardwareInfoType;


  /*
  AbaQualityLevelType
  Enumeration value describing the quality level for ABA.
  */
  typedef enum
  {
    ABA_QUALITY_LOW,                     // Indicates a low quality level for the core algoritm
    //
    ABA_QUALITY_MEDIUM,                  // Indicates a medium quality level for the core algoritm
    //
    ABA_QUALITY_HIGH                     // Indicates a high quality level for the core algoritm
  } AbaQualityLevelType;

  /*
  AbaFilterLevelType
  Enumeration value describing the speed level of the temporal filter.
  */
  typedef enum
  {
    ABA_FILTER_SPEED_LOW,                // Indicates a lower speed for the temporal filter
    //
    ABA_FILTER_SPEED_MEDIUM,             // Indicates a medium speed for the temporal filter
    //
    ABA_FILTER_SPEED_HIGH                // Indicates a higher speed for the temporal filter
  } AbaFilterLevelType;


  /*
  AbaStatusType
  Enumeration listing return values from ABA function calls.

  */
  typedef enum
  {
    ABA_STATUS_SUCCESS,                // Operation successful
    ABA_STATUS_FAIL,                   // Operation Failed
    ABA_STATUS_BAD_PARAM,              // Bad Parameter
    ABA_STATUS_NOT_SUPPORTED,          // Operation not supported
  } AbaStatusType;

  /*
  AbaStateType

  When ABA is in the inactive state it does not change the backlight nor the LUT.
  When ABA is activate when it can modify the backlight and/or LUT based on content
  When ABA is disabled upon an explicit request from the client.
  */
  typedef enum
  {
    ABA_STATE_INACTIVE,                // ABA is inactive
    ABA_STATE_ACTIVE,                  // ABA is active
    ABA_STATE_DISABLING,               // ABA is transitioning to disabled
    ABA_STATE_DISABLED                 // ABA is disabled
  } AbaStateType;


  typedef struct
  {
    uint32_t        uMonth;              //Aba Major Version
    uint32_t        uDay;
    uint32_t        uYear;
  }AbaDateType;

  typedef struct
  {
    uint32_t        uABAMajorVersion;              //Aba Major Version
    uint32_t        uABAMinorVersion;
    uint32_t        uSVIVersion;
    uint32_t        uCABLVersion;
    AbaDateType     sDate;
  }AbaVersionType;

#ifdef __cplusplus
}
#endif

#endif  /* _ABA_TYPE_H_ */
