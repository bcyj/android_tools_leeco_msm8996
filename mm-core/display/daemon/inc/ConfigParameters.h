/*
 * DESCRIPTION
 * This file contains methods for parsing the XML file using TinyXML
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#ifndef _CONFIGPARAMETERS_H
#define _CONFIGPARAMETERS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include "tinyxml.h"
#include "common_log.h"
#include "ConfigParamsDefine.h"
#include "abl_oem.h"
#include "aba_type.h"

#define MAX_NUMERICAL_STRING_LENGTH     15
#define MAX_ARRAY_STRING_LENGTH         1024
#define MAX_VARIABLE_NAME_LENGTH        100
#define UNITS_ATTRIBUTE                 "units"
#define PROP_CABL_XML_SET               "config.cabl.xml"
#define PROP_CABL_XML_PATH              "config.cabl.path"
#define PROP_CABL_XML_PRINT             "config.cabl.xml.print"
#define PROP_SVI_XML_SET                "config.svi.xml"
#define PROP_SVI_XML_PATH               "config.svi.path"
#define PROP_SVI_XML_PRINT              "config.svi.xml.print"
#define MAX_CONFIG_LUT_LENGTH           33
#define CAP_ARRAY_RANGE(len)            (((len) > MAX_CONFIG_LUT_LENGTH) ? \
                                        MAX_CONFIG_LUT_LENGTH : (len))

enum {
    UNITS_TYPE_INT = 10,
    UNITS_TYPE_UINT,
    UNITS_TYPE_LONG,
    UNITS_TYPE_DOUBLE,
    UNITS_TYPE_HEX,
    UNITS_TYPE_UNKNOWN,
};

enum {
    ERROR_FILE_NOT_PARSED = -1,
    ERROR_FILE_FORMAT = -2,
    ERROR_BAD_PARAMS = -3,
    ERROR_ELEMENT_NOT_FOUND = -4,
    ERROR_CONTENT_NOT_FOUND = -5,
    ERROR_CONTENT_FORMAT = -6,
    ERROR_TYPE_ATTRIBUTE_MISSING = -7,
    ERROR_NO_MEMORY = -8,
};

class CP {
private:
    int32_t AddValueOfType(char * val, void * arr, int32_t type);
    int32_t GetElement(const char* n, TiXmlElement **x);
    int32_t GetElementUnits(const char* n);
    int32_t GetRawValue(const char* n, char **v);
    template <typename T>
        int32_t ParseAsArray(const char* n, int32_t t, T **v, int32_t l);

protected:
    bool bFileParsed;
    TiXmlDocument xmlCP;

    template <typename T>
        int32_t CopyArray(T **dest, T *src, uint32_t len);
    int32_t GetDoubleValue(const char* n, double *v);
    int32_t GetIntegerValue(const char* n, int *v);
    int32_t GetGroupID(char *gpID);
    int32_t GetHexValue(const char* x, uint32_t *v);
    template <typename T>
        int32_t ParseArrayElement(const char* n, int32_t t, T **v, int32_t l);
    void XMLParseError(int32_t r);

    virtual void LoadDefaults() {
        LOGE("%s:, Configuring Parameters Not supported!", __func__);
    }
    virtual int32_t VerifyID() {
        LOGE("%s:, Configuring Parameters Not supported!", __func__);
        return 0;
    }

    void SetFileParsed(bool v);
    void PrintArray(void *v, int32_t len, int32_t type);

public:
    CP (): bFileParsed(false){}
    virtual ~CP () {}
    int32_t ParseXML(const char* xmlFile);
    virtual int32_t ParseParameters() {
        LOGE("%s:, Configuring Parameters Not supported!", __func__);
        return 0;
    }
};

class CABLHWParams : public CP {
    static const char* ID;

public:
    uint32_t  nQualityLevels;

    /*Following array pointers have length nQualityLevels*/
    uint32_t* pBLMinRatio;
    uint32_t* pBLMaxRatio;
    uint32_t* pPixelDistortion;
    uint32_t* pBLChangeSpeed;

    /*Following reserved params array pointers have length nQualityLevels*/
    double*   pRPSoftClippingSlope;
    int32_t*  pRPLUTType;
    uint32_t* pRPWindowSizeThreshold;
    uint32_t* pRPFilterCoefficientThreshold;
    uint32_t* pRPBackLightReductionFactor;
    uint32_t* pRPBackLightStepSizeHighCorrelation;
    uint32_t* pRPSceneCorrelationThreshold;
    uint32_t* pRPSceneChangeThreshold;

    int32_t   mDefaultQualityMode;
    int32_t   mUIQualityLevel;
    int32_t   mVideoQualityLevel;
    uint32_t  mBLMaxLevel;
    uint32_t  mBLMinLevel;
    uint32_t  mBLLevelThreshold;

    uint32_t  mLengthGammaLUT;
    /* Following array pointers have length mLengthGammaLUT */
    uint32_t* pGammaGrayScale;
    uint32_t* pGammaLuminance;

    uint32_t  mLengthBLLuminanceLUT;
    /* Following array pointers have length mLengthBLLuminanceLUT */
    uint32_t* pBLResponseBL;
    uint32_t* pBLResponseLuminance;

    CABLHWParams() : nQualityLevels(ABL_QUALITY_MAX),
        mUIQualityLevel(ABL_QUALITY_HIGH), mVideoQualityLevel(ABL_QUALITY_LOW),
        mBLMaxLevel(255), mBLMinLevel(30){};

    ~CABLHWParams() {};

    /* Called from CABL Context */
    int32_t ConfigureABLParameters(bl_oem_api *api_para,
        uint32_t initQualityLevel, uint32_t initDataLevel);

    /* Called from AbaContext */
    int32_t ConfigureCABLParameters(CablInitialConfigType *parameters);

    int32_t VerifyID();
    int32_t ParseNonArrayParameters();
    int32_t ParsePanelGammaParameters();
    int32_t ParseBackLightParameters();
    int32_t ParseReservedParameters();

    int32_t ParseParameters();
    void    FreeUnusedMemory();
    void    PrintParsedParameters();
    void    LoadDefaults();
};

class SVIHWParams : public CP {
    static const char* ID;

public:
    /*Gain Tuning Parameters*/
    uint32_t  mContrastStrength;
    uint32_t  mBrightnessStrength;

    /*Temporal Filter Parameters*/
    uint32_t  mFilterStepSizeUI;
    uint32_t  mFilterStepSizeVideo;

    /*Panel Parameters*/
    uint32_t  mPanelReflectanceRatio;
    uint32_t  mPanelPeakBrightness;

    uint32_t  mMinBLReductionRatio;
    uint32_t  mBacklightReductionMode;
    uint32_t  mEnableBacklightReduction;
    uint32_t  mIndoorMinLux;
    uint32_t  mIndoorMinBright;
    uint32_t  mIndoorMaxLux;
    uint32_t  mIndoorMaxBright;
    uint32_t  mOutdoorMaxLux;
    uint32_t  mOutdoorMaxBright;

    uint32_t  mLengthSensorResponseTable;
    /* The following arrays are of length mentioned above*/
    uint32_t *pSensorRespRef;
    uint32_t *pSensorRespSensor;

    uint32_t  mLengthBLResponseTable;
    /* The following arrays are of length mentioned above*/
    uint32_t *pBLRespInput;
    uint32_t *pBLRespOutput;

    SVIHWParams() {};

    ~SVIHWParams() {};

    /* Called from AbaContext */
    int32_t ConfigureSVIParameters(SVIConfigParametersType *parameters);

    int32_t VerifyID();
    int32_t ParseNonArrayParameters();
    int32_t ParseArrayParameters();

    int32_t ParseParameters();
    void    PrintParsedParameters();
    void    LoadDefaults();
};


#endif /* _CONFIGPARAMETERS_H */
