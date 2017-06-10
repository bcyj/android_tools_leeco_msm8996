/*
 * DESCRIPTION
 * This file contains constants pertaining to XML file for PP Algorithms
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#ifndef _CONFIGPARAMETERSDEFINE_H
#define _CONFIGPARAMETERSDEFINE_H

/*CABL Configuration XML Element names*/
#define CABL_GROUP_ID                   "CABLConfiguration"

#define CABL_BL_MAX_LEVEL               "CABLBackLightMaxValue"
#define CABL_BL_MIN_LEVEL               "CABLBackLightMinValue"
#define CABL_BL_THRESHOLD               "CABLBackLightThreshold"

#define CABL_LEN_GAMMA_LUT              "GammaResponseTableLength"
#define CABL_LEN_BL_LUM_LUT             "BackLightResponseTableLength"

#define CABL_GAMMA_GRAYSCALE            "GammaResponseTableGrayScale"
#define CABL_GAMMA_LUMINANCE            "GammaResponseTableLuminance"

#define CABL_BL_MIN_RATIOS              "CABLBackLightMinRatio"
#define CABL_BL_MAX_RATIOS              "CABLBackLightMaxRatio"
#define CABL_PIXEL_DISTORTION           "CABLPixelDistortion"
#define CABL_BL_CHANGE_SPEED            "CABLBackLightStepSize"

#define CABL_BL_RESP_TABLE              "BackLightResponseValueTable"
#define CABL_BL_RESP_LUMINANCE          "BackLightResponseLumaValues"

#define CABL_RP_SOFT_SLOPE              "CABLSoftClippingSlope"
#define CABL_RP_LUT_TYPE                "CABLLutType"
#define CABL_RP_WST                     "CABLWindowSizeThreshold"
#define CABL_RP_FCT                     "CABLFilterCoefficientThreshold"
#define CABL_RP_BL_RF                   "CABLBackLightReductionFactor"
#define CABL_RP_BL_SSHC                 "CABLBackLightStepSizeHighCorrelation"
#define CABL_RP_SC_CORR_THRES           "CABLSceneCorrelationThreshold"
#define CABL_RP_SC_CHANGE_THRES         "CABLSceneChangeThreshold"

/*SVI Configuration XML Element names*/
#define SVI_GROUP_ID                    "SVI Config"

#define SVI_BRIGHT_STRENGTH             "BrightnessStrength"

#define SVI_CONT_STRENGTH               "ContrastStrength"

#define SVI_FILTER_SIZE_UI              "FilterStepSizeUIMode"
#define SVI_FILTER_SIZE_VIDEO           "FilterStepSizeVideoMode"

#define SVI_LEN_SENSOR_RESP             "SensorResponseTableLength"
#define SVI_LEN_BL_RESP                 "BackLightResponseTableLength"

#define SVI_SENSOR_RESP_REF             "SensorResponseTableReference"
#define SVI_SENSOR_RESP_TABLE           "SensorResponseTableSensor"

#define SVI_BL_RESP_IN                  "BacklightResponseTableInput"
#define SVI_BL_RESP_OUT                 "BacklightResponseTableOutput"

#define SVI_PANEL_REFLECT_RATIO         "PanelReflectanceRatio"
#define SVI_PANEL_PEAK_BRIGHT           "PanelPeakBrightness"

#define SVI_BL_MIN_RED_RATIO            "MinBacklightReductionRatio"
#define SVI_BL_REDUCTION_MODE           "BacklightReductionMode"
#define SVI_EN_BL_REDUCTION             "EnableBacklightReduction"

#define SVI_INDOOR_MIN_LUX              "IndoorMinLuxLevel"
#define SVI_INDOOR_MIN_BRIGHT           "IndoorMinBrightnessLevel"

#define SVI_INDOOR_MAX_LUX              "IndoorMaxLuxLevel"
#define SVI_INDOOR_MAX_BRIGHT           "IndoorMaxBrightnessLevel"

#define SVI_OUTDOOR_MAX_LUX             "OutdoorMaxLuxLevel"
#define SVI_OUTDOOR_MAX_BRIGHT          "OutdoorMaxBrightnessLevel"

#endif
