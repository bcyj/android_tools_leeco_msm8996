/*
 * DESCRIPTION
 * This file contains methods for parsing the XML file using TinyXML
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "ConfigParameters.h"

/*
 * FUNCTION: GetElement
 *
 * Gets element with provided name
 * Parameter :
 * Name of the element to match -- const char*
 *
 * Return :
 * 0 = Success
 * Else error
 */
int32_t CP::GetElement(const char * name, TiXmlElement **xmlEl) {
    TiXmlNode* xmlCurr;
    TiXmlElement* xmlTemp;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if (!name)
        return ERROR_BAD_PARAMS;

    xmlCurr = (TiXmlNode *)&xmlCP;
    if (xmlCurr->NoChildren())
        return ERROR_FILE_FORMAT;

    /* Group variable */
    xmlCurr = xmlCurr->FirstChild("Group");
    if (xmlCurr->NoChildren())
        return ERROR_FILE_FORMAT;

    xmlCurr = xmlCurr->FirstChild();
    while (xmlCurr != NULL) {
        if (strncmp(xmlCurr->Value(), name, MAX_VARIABLE_NAME_LENGTH)== 0)
            break;
        xmlCurr = xmlCurr->NextSibling();
    }

    if (!xmlCurr)
        return ERROR_ELEMENT_NOT_FOUND;

    xmlTemp = xmlCurr->ToElement();

    if(!xmlTemp)
        return ERROR_ELEMENT_NOT_FOUND;

    *xmlEl = xmlTemp;

    return 0;
}

/*
 * FUNCTION: GetRawValue
 *
 * Gets the integer/long/double/hex value present in element as char*
 * Parameter :
 * Name of the element to match -- const char*
 * Pointer to buffer to return string
 *
 * Return :
 * 0 = Success
 * Else error
 */
int32_t CP::GetRawValue(const char* name, char **val) {
    int32_t ret = 0, n = 0;
    char *text = NULL;
    TiXmlElement* xmlEl;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if ((!val) || (!name))
        return ERROR_BAD_PARAMS;

    ret = GetElement(name, &xmlEl);
    if (ret)
        goto exit_err;

    text = (char *)malloc(MAX_VARIABLE_NAME_LENGTH * sizeof(char));
    if (!text) {
        ret = ERROR_NO_MEMORY;
        goto exit_err;
    }

    if (xmlEl->GetText() == NULL) {
        ret = ERROR_ELEMENT_NOT_FOUND;
    } else {
        n = snprintf(text, MAX_VARIABLE_NAME_LENGTH, "%s", xmlEl->GetText());
        if (n <= 0)
            ret = ERROR_CONTENT_NOT_FOUND;
    }

exit_err:
    *val = text;
    return ret;
}


/*
 * FUNCTION: GetDoubleValue
 *
 * Gets double value contained in the element
 * Parameter :
 * Name of the element to match -- const char*
 * Pointer to double variable val
 *
 * Return :
 * 0 = Success
 * Else error
 */
int32_t CP::GetDoubleValue(const char * name, double * val) {
    int32_t ret = 0;
    char *text;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if ((!val) || (!name))
        return ERROR_BAD_PARAMS;

    ret = GetRawValue(name, &text);
    if (ret)
        goto exit_err;

    if (sscanf(text, "%lf", val) <= 0)
        ret = ERROR_CONTENT_FORMAT;

exit_err:
    free(text);
    return ret;
}

/*
 * FUNCTION: GetElementUnits
 *
 * Gets value in units attribute for the Element name
 * Parameter :
 * Name of the element to match -- const char*
 *
 * Return :
 * Success = UNITS_TYPE_*
 * Else error
 */
int32_t CP::GetElementUnits(const char * name) {
    int32_t ret = ERROR_ELEMENT_NOT_FOUND;
    TiXmlElement* xmlEl;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if (!name)
        return ERROR_BAD_PARAMS;

    ret = GetElement(name, &xmlEl);
    if (ret) {
        LOGE("%s: Element name - %s not found!", __func__, name);
        return ret;
    }

    if (xmlEl->Attribute(UNITS_ATTRIBUTE)) {
        if (strncmp(xmlEl->Attribute(UNITS_ATTRIBUTE), "int", 3) == 0)
            return UNITS_TYPE_INT;
        if (strncmp(xmlEl->Attribute(UNITS_ATTRIBUTE), "uint", 4) == 0)
            return UNITS_TYPE_UINT;
        if (strncmp(xmlEl->Attribute(UNITS_ATTRIBUTE), "long", 4) == 0)
            return UNITS_TYPE_LONG;
        if (strncmp(xmlEl->Attribute(UNITS_ATTRIBUTE), "double", 6) == 0)
            return UNITS_TYPE_DOUBLE;
        return UNITS_TYPE_UNKNOWN;
    }
    return ret;
}


/*
 * FUNCTION: GetIntegerValue
 *
 * Gets integer value contained in the element
 * Parameter :
 * Name of the element to match -- const char*
 * Pointer to integer variable val
 *
 * Return :
 * 0 = Success
 * Else error
 */
int32_t CP::GetIntegerValue(const char* name, int * val) {
    int32_t ret = 0;
    char *text;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if ((!val) || (!name))
        return ERROR_BAD_PARAMS;

    ret = GetRawValue(name, &text);
    if (ret)
        goto exit_err;

    if (sscanf(text, "%d", val) <= 0)
        ret = ERROR_CONTENT_FORMAT;

exit_err:
    free(text);
    return ret;
}

/*
 * FUNCTION: GetGroupID
 *
 * Gets attribute value set in group element which is root element
 * Parameter :
 *
 * Return :
 * 0 = Success
 * Else error
 *
 */
int32_t CP::GetGroupID(char *gpID){
    TiXmlElement *xmlRoot;
    int32_t n=0;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if (!gpID)
        return ERROR_BAD_PARAMS;

    xmlRoot = xmlCP.RootElement();

    n = snprintf(gpID, MAX_VARIABLE_NAME_LENGTH, "%s",
            xmlRoot->Attribute("id"));

    if (n <= 0)
        return ERROR_CONTENT_FORMAT;

    return 0;
}

/*
 * FUNCTION: GetHexValue
 *
 * Gets hex value contained in the element
 * Parameter :
 * Name of the element to match -- const char*
 * Pointer to integer variable val
 *
 * Return :
 * 0 = Success
 * Else error
 */
int32_t CP::GetHexValue(const char* name, uint32_t *val){
    int32_t ret = 0;
    char *text;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if ((!val) || (!name))
        return ERROR_BAD_PARAMS;

    ret = GetRawValue(name, &text);
    if (ret)
        goto exit_err;

    if (sscanf(text, "%X", val) <= 0)
        ret = ERROR_CONTENT_FORMAT;

exit_err:
    free(text);
    return ret;
}

/*
 * FUNCTION: ParseXML
 *
 * Parses the file name provided as const char*
 * Parameter :
 * Filename -- const char*
 *
 * Return :
 * 0 = Success
 * Else error code of tinyXML
 */
int32_t CP::ParseXML(const char* xmlFile) {
    int ret = 0;

    if (!xmlCP.LoadFile(xmlFile)) {
        LOGE("%s, Parsing XML failed with errno = %d", __func__, ret);
        ret = ERROR_FILE_NOT_PARSED;
        SetFileParsed(false);
    }
    else
        SetFileParsed(true);

    return ret;
}

/*
 * FUNCTION: AddValueOfType
 *
 * Parses the char array to get number of type
 * Parameter :
 * Char array to convert -- const char*
 * Void pointer containing pointer to which value is to be stored
 * type to which we convert array content
 *
 * Return :
 * 0 = Success
 * Else error
 */
int32_t CP::AddValueOfType(char *val, void *arr, int32_t type) {
    int32_t ret = ERROR_CONTENT_FORMAT;

    switch (type) {
        case UNITS_TYPE_INT:
            if (sscanf(val, "%d", (int32_t *)arr) <= 0)
                return ret;
            break;
        case UNITS_TYPE_UINT:
            if (sscanf(val, "%u", (uint32_t *)arr) <= 0)
                return ret;
            break;
        case UNITS_TYPE_LONG:
            if (sscanf(val, "%ld", (long int *)arr) <= 0)
                return ret;
            break;
        case UNITS_TYPE_DOUBLE:
            if (sscanf(val, "%lf", (double *)arr) <= 0)
                return ret;
            break;
        case UNITS_TYPE_HEX:
            if (sscanf(val, "%X", (uint32_t *)arr) <= 0)
                return ret;
            break;
        default:
            return ret;
    }

    return 0;
}

/*
 * FUNCTION: ParseAsArray
 *
 * Gets content of element with given name as array
 * Parameter :
 * Name of the element to match -- const char*
 * Integer value to denote type of units of array
 * Void pointer to array which would contain the parsed array on return
 * Length of array
 *
 * Return :
 * 0 = Success
 * Else error
 */
template <typename T>
int32_t CP::ParseAsArray(const char* name, int32_t type, T **v, int32_t len){
    int32_t ret = 0, n = 0, i = 0;
    char *text, *pCh, *saveptr;
    char val[MAX_NUMERICAL_STRING_LENGTH];
    TiXmlElement* xmlEl;
    T *arr;

    if (!bFileParsed)
        return ERROR_FILE_NOT_PARSED;
    else if ((!name) || (!len))
        return ERROR_BAD_PARAMS;

    ret = GetElement(name, &xmlEl);
    if (ret)
        return ret;

    text = (char *)malloc(MAX_ARRAY_STRING_LENGTH * sizeof(char));
    if (!text)
        return ERROR_NO_MEMORY;

    if (xmlEl->GetText() == NULL) {
        return ERROR_ELEMENT_NOT_FOUND;
    } else {
        n = snprintf(text, MAX_ARRAY_STRING_LENGTH, "%s", xmlEl->GetText());
        if (n <= 0)
            return ERROR_CONTENT_NOT_FOUND;
    }

    arr = (T *)malloc(len * sizeof(T));
    if (arr == NULL) {
        ret = ERROR_NO_MEMORY;
        goto exit_err;
    }

    /* Loop over each element and convert to given type */
    pCh = strtok_r(text, " ", &saveptr);
    for (i = 0; i < len; i++) {

        if (!pCh)
            break;

        memset(val, 0, sizeof(val));
        n = snprintf(val, MAX_NUMERICAL_STRING_LENGTH, "%s", pCh);
        if (n <= 0) {
            LOGE("%s, Error at Element = %s, Index = %d", __func__, name, i);
            ret = ERROR_CONTENT_FORMAT;
            goto exit_mem;
        }

        ret = AddValueOfType(val, &arr[i], type);
        if (ret)
            goto exit_mem;
        pCh = strtok_r(NULL, " ", &saveptr);
    }

    if (i!=len) {
        ret = ERROR_CONTENT_FORMAT;
        goto exit_mem;
    }

    free(text);
    *v = arr;
    return ret;

exit_mem:
    free(arr);
exit_err:
    free(text);
    *v = NULL;
    return ret;
}

/*
 * FUNCTION: ParseArrayElement
 *
 * Wrapper around ParseAsArray and GetElementUnits
 * Parameter :
 * Name of the element to match -- const char*
 * Integer value to denote type of units of array
 * Void pointer to array which would contain the parsed error on return
 * Length of array
 *
 * Return :
 * 0 = Success
 * Else error
 */
template<typename T>
int32_t CP::ParseArrayElement(const char* name, int32_t t, T **v, int32_t len) {
    int32_t ret = 0;
    T *temp = NULL;

    ret = GetElementUnits(name);
    if (ret != t) {
        if (ret > 0)
            ret = ERROR_CONTENT_FORMAT;
        return ret;
    }

    ret = ParseAsArray<T>(name, t, &temp, len);
    *v = temp;

    return ret;
}

/*
 * FUNCTION: XMLParseError
 *
 * Returns a string for each error
 *
 */
void CP::XMLParseError(int32_t r) {
    switch (r) {
        case ERROR_FILE_NOT_PARSED:
            LOGE("ERROR: File was not parsed");
        return;
        case ERROR_FILE_FORMAT:
            LOGE("ERROR: File not of right format");
        return;
        case ERROR_BAD_PARAMS:
            LOGE("ERROR: Encountered Bad Parameters to function");
        return;
        case ERROR_ELEMENT_NOT_FOUND:
            LOGE("ERROR: Corresponding element was not found");
        return;
        case ERROR_CONTENT_NOT_FOUND:
            LOGE("ERROR: Text corresponding to object not found");
        return;
        case ERROR_CONTENT_FORMAT:
            LOGE("ERROR: Content in object not in compliant format");
        return;
        case ERROR_TYPE_ATTRIBUTE_MISSING:
            LOGE("ERROR: Array Element has no type Attribute");
        return;
        case ERROR_NO_MEMORY:
            LOGE("ERROR: Ran out of memory");
        return;
        default:
            LOGE("ERROR: Unknown error id encountered");
        return;
    }
    return;
}

/*
 * FUNCTION: SetFileParsed
 *
 * Use this to set the file parsed variable value
 *
 */
void CP::SetFileParsed(bool v) {
    bFileParsed = v;
}

/*
 * FUNCTION: CopyArray
 *
 * Use this to copy array of type T
 *
 */
template <typename T>
int32_t CP::CopyArray(T **dest, T *src, uint32_t len){
    T *ptr;
    uint32_t i;

    ptr = (T *)malloc(len * sizeof(T));
    if (!ptr)
        return ERROR_NO_MEMORY;

    for (i = 0; i < len; i++) {
        ptr[i] = src[i];
    }

    *dest = ptr;
    return 0;
}

/*
 * FUNCTION: PrintArray
 *
 * Use this to print array
 * Each set of 50 elements in one row
 *
 */
void CP::PrintArray(void *arr, int32_t len, int32_t type) {
    int32_t i = 0, j = 0;
    size_t l = 0;
    char str[MAX_ARRAY_STRING_LENGTH] = "";

    if (len <= 0) {
        LOGE("%s: Error: length of array is %d", __func__, len);
        return;
    }

    len = CAP_ARRAY_RANGE(len);

    for (i = 0; i < len ; i++) {
        l = strlen(str);

        switch (type) {
            case UNITS_TYPE_INT:
                if (snprintf(str + l, (MAX_ARRAY_STRING_LENGTH - l) *
                    sizeof(char), "%d ", *((int32_t *)arr+ i)) < 0)
                    goto exit_err;
                break;
            case UNITS_TYPE_UINT:
                if (snprintf(str + l, (MAX_ARRAY_STRING_LENGTH - l) *
                    sizeof(char), "%u ", *((uint32_t *)arr+ i)) < 0)
                    goto exit_err;
                break;
            case UNITS_TYPE_LONG:
                if (snprintf(str + l, (MAX_ARRAY_STRING_LENGTH - l) *
                    sizeof(char), "%ld ", *((long int *)arr+ i)) < 0)
                    goto exit_err;
                break;
            case UNITS_TYPE_DOUBLE:
                if (snprintf(str + l, (MAX_ARRAY_STRING_LENGTH - l) *
                    sizeof(char), "%f ", *((double *)arr+ i)) < 0)
                    goto exit_err;
                break;
            case UNITS_TYPE_HEX:
                if (snprintf(str + l, (MAX_ARRAY_STRING_LENGTH - l) *
                    sizeof(char), "%u ", *((uint32_t *)arr+ i)) < 0)
                    goto exit_err;
                break;
            default:
                LOGE("%s: Unknown type to print array, returning", __func__);
                goto exit_err;
        }

        j++;
        if (j == 50) {
            LOGE("%s", str);
            memset(str, 0, sizeof(str));
            j = 0;
        }
    }

    if ( j != 0)
        LOGE("%s", str);
    return;

exit_err:
    LOGE("%s: Error encounterd!", __func__);
    return;
}

/* Start CABLHWParams Functions */
const char *CABLHWParams::ID = CABL_GROUP_ID;

/*
 * FUNCTION: ParseReservedParameters
 *
 * Parses parameters which are not arrays like lengths and uint
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t CABLHWParams::ParseReservedParameters(){
    int32_t ret = 0;

    ret = ParseArrayElement<double>(CABL_RP_SOFT_SLOPE, UNITS_TYPE_DOUBLE,
                &pRPSoftClippingSlope, nQualityLevels);
    if (ret)
        return ret;

    ret = ParseArrayElement<int32_t>(CABL_RP_LUT_TYPE, UNITS_TYPE_INT,
                &pRPLUTType, nQualityLevels);
    if (ret)
        goto exit_mem1;

    ret = ParseArrayElement<uint32_t>(CABL_RP_WST, UNITS_TYPE_UINT,
                &pRPWindowSizeThreshold, nQualityLevels);
    if (ret)
        goto exit_mem2;

    ret = ParseArrayElement<uint32_t>(CABL_RP_FCT, UNITS_TYPE_UINT,
                &pRPFilterCoefficientThreshold, nQualityLevels);
    if (ret)
        goto exit_mem3;

    ret = ParseArrayElement<uint32_t>(CABL_RP_BL_RF, UNITS_TYPE_UINT,
                &pRPBackLightReductionFactor, nQualityLevels);
    if (ret)
        goto exit_mem4;

    ret = ParseArrayElement<uint32_t>(CABL_RP_BL_SSHC, UNITS_TYPE_UINT,
                &pRPBackLightStepSizeHighCorrelation, nQualityLevels);
    if (ret)
        goto exit_mem5;

    ret = ParseArrayElement<uint32_t>(CABL_RP_SC_CORR_THRES, UNITS_TYPE_UINT,
                &pRPSceneCorrelationThreshold, nQualityLevels);
    if (ret)
        goto exit_mem6;

    ret = ParseArrayElement<uint32_t>(CABL_RP_SC_CHANGE_THRES, UNITS_TYPE_UINT,
                &pRPSceneChangeThreshold, nQualityLevels);
    if (ret)
        goto exit_mem7;

    return ret;

exit_mem7:
    free(pRPSceneCorrelationThreshold);
exit_mem6:
    free(pRPBackLightStepSizeHighCorrelation);
exit_mem5:
    free(pRPBackLightReductionFactor);
exit_mem4:
    free(pRPFilterCoefficientThreshold);
exit_mem3:
    free(pRPWindowSizeThreshold);
exit_mem2:
    free(pRPLUTType);
exit_mem1:
    free(pRPSoftClippingSlope);
    return ret;
}

/*
 * FUNCTION: ParseBackLightParameters
 *
 * Parses OEM Backlight arrays
 *
 * ASSUMPTION: Parameter Arrays are of Type UINT
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t CABLHWParams::ParseBackLightParameters(){
    int32_t ret = 0;

    ret = ParseArrayElement<uint32_t>(CABL_BL_MIN_RATIOS, UNITS_TYPE_UINT,
                &pBLMinRatio, nQualityLevels);
    if (ret)
        return ret;

    ret = ParseArrayElement<uint32_t>(CABL_BL_MAX_RATIOS, UNITS_TYPE_UINT,
                &pBLMaxRatio, nQualityLevels);
    if (ret)
        goto exit_mem1;

    ret = ParseArrayElement<uint32_t>(CABL_PIXEL_DISTORTION, UNITS_TYPE_UINT,
                &pPixelDistortion, nQualityLevels);
    if (ret)
        goto exit_mem2;

    ret = ParseArrayElement<uint32_t>(CABL_BL_CHANGE_SPEED, UNITS_TYPE_UINT,
                &pBLChangeSpeed, nQualityLevels);
    if (ret)
        goto exit_mem3;

    ret = ParseArrayElement<uint32_t>(CABL_BL_RESP_TABLE, UNITS_TYPE_UINT,
                &pBLResponseBL, mLengthBLLuminanceLUT);
    if (ret)
        goto exit_mem4;

    ret = ParseArrayElement<uint32_t>(CABL_BL_RESP_LUMINANCE, UNITS_TYPE_UINT,
                &pBLResponseLuminance, mLengthBLLuminanceLUT);
    if (ret)
        goto exit_mem5;

    return ret;

exit_mem5:
    free(pBLResponseBL);
exit_mem4:
    free(pBLChangeSpeed);
exit_mem3:
    free(pPixelDistortion);
exit_mem2:
    free(pBLMaxRatio);
exit_mem1:
    free(pBLMinRatio);
    return ret;
}

/*
 * FUNCTION: ParsePanelGammaParameters
 *
 * Parses OEM Gamma grayscale and luminance arrays
 *
 * ASSUMPTION: Parameter Arrays are of Type UINT
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t CABLHWParams::ParsePanelGammaParameters() {
    int32_t ret = 0;

    ret = ParseArrayElement<uint32_t>(CABL_GAMMA_GRAYSCALE, UNITS_TYPE_UINT,
                &pGammaGrayScale, mLengthGammaLUT);
    if (ret)
        return ret;

    ret = ParseArrayElement<uint32_t>(CABL_GAMMA_LUMINANCE, UNITS_TYPE_UINT,
                &pGammaLuminance, mLengthGammaLUT);
    if (ret)
        goto exit_mem;

    return ret;

exit_mem:
    free(pGammaGrayScale);
    return ret;
}

/*
 * FUNCTION: ParseNonArrayParameters
 *
 * Parses parameters which are not arrays like lengths and uint
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t CABLHWParams::ParseNonArrayParameters() {
    int32_t ret = 0;
    int32_t tmp = 0;

    ret = GetIntegerValue(CABL_BL_MAX_LEVEL, &tmp);
    if (ret)
        return ret;
    mBLMaxLevel = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(CABL_BL_MIN_LEVEL, &tmp);
    if (ret) {
        /*
         * If BLMinLevel is not present then set value to 0,
         * Later during call check if this is set as 0 and then
         * use interpolate function to get the required value
        */
        mBLMinLevel= 0;
    } else {
        mBLMinLevel = tmp > 0 ? tmp : 0;
    }

    ret = GetIntegerValue(CABL_BL_THRESHOLD, &tmp);
    if (ret)
        return ret;
    mBLLevelThreshold = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(CABL_LEN_GAMMA_LUT, &tmp);
    if (ret)
        return ret;
    mLengthGammaLUT = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(CABL_LEN_BL_LUM_LUT, &tmp);
    if (ret)
        return ret;
    mLengthBLLuminanceLUT = tmp > 0 ? tmp : 0;

    return ret;
}

/*
 * FUNCTION: VerifyID
 *
 * Checks if ID in group element of XML matches the one of class
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t CABLHWParams::VerifyID() {
    int32_t ret = 0;
    char gpID[MAX_VARIABLE_NAME_LENGTH];

    ret = GetGroupID(gpID);
    if (ret)
        return ret;

    if (strncmp(ID, gpID, sizeof(ID)-1) != 0)
        ret = ERROR_CONTENT_FORMAT;

    return ret;
}

/*
 * FUNCTION: ParseParameters
 *
 * Calls Parsing of parameters one after the other
 * Works on handling error cases and setting of values of CP class
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t CABLHWParams::ParseParameters() {
    int32_t ret = 0;
    char property[PROPERTY_VALUE_MAX];

    ret = VerifyID();
    if (ret)
        goto exit_params;

    ret = ParseNonArrayParameters();
    if (ret)
        goto exit_params;

    ret = ParsePanelGammaParameters();
    if (ret)
        goto exit_params;

    ret = ParseBackLightParameters();
    if (ret)
        goto exit_params;

    ret = ParseReservedParameters();
    if (ret)
        goto exit_params;

    if (property_get(PROP_CABL_XML_PRINT, property, 0) > 0)
        PrintParsedParameters();

return ret;

exit_params:
    XMLParseError(ret);
    LOGE("%s: Parsing Error occured, Loading Defaults ret = %d", __func__, ret);
    LoadDefaults();
    return ret;

}

/*
 * FUNCTION: PrintParsedParameters
 *
 * This is to print parsed parameters to logcat if prop PROP_CABL_XML_PRINT is set
 *
 */
void CABLHWParams::PrintParsedParameters() {

    LOGD("*******************************************************************");
    LOGD("%s: Printing out the parsed parameters for CABL", __func__);
    LOGD("*******************************************************************");
    LOGD("QualityLevels__________________= %u", nQualityLevels);
    LOGD("DefaultQualityMode_____________= %u", mDefaultQualityMode);
    LOGD("UIQualityLevel_________________= %u", mUIQualityLevel);
    LOGD("VideoQualityLevel______________= %u", mVideoQualityLevel);
    LOGD("BLMaxLevel_____________________= %u", mBLMaxLevel);
    LOGD("BLMinLevel_____________________= %u", mBLMinLevel);
    LOGD("BLLevelThreshold_______________= %u", mBLLevelThreshold);
    LOGD("LengthGammaLUT_________________= %u", mLengthGammaLUT);
    LOGD("LengthBLLuminanceLUT___________= %u", mLengthBLLuminanceLUT);

    LOGD("\nBacklight Parameters:");
    LOGD("%s =>",CABL_BL_MIN_RATIOS);
    PrintArray(pBLMinRatio, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_BL_MAX_RATIOS);
    PrintArray(pBLMaxRatio, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_PIXEL_DISTORTION);
    PrintArray(pPixelDistortion, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_BL_CHANGE_SPEED);
    PrintArray(pBLChangeSpeed, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_BL_RESP_TABLE);
    PrintArray(pBLResponseBL, mLengthBLLuminanceLUT, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_BL_RESP_LUMINANCE);
    PrintArray(pBLResponseLuminance, mLengthBLLuminanceLUT, UNITS_TYPE_UINT);

    LOGD("\nPanel Gamma Parameters:");
    LOGD("%s =>",CABL_GAMMA_GRAYSCALE);
    PrintArray(pGammaGrayScale, mLengthGammaLUT, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_GAMMA_LUMINANCE);
    PrintArray(pGammaLuminance, mLengthGammaLUT, UNITS_TYPE_UINT);

    LOGD("\nReserved Parameters:");
    LOGD("%s =>",CABL_RP_SOFT_SLOPE);
    PrintArray(pRPSoftClippingSlope, nQualityLevels, UNITS_TYPE_DOUBLE);
    LOGD("%s =>",CABL_RP_LUT_TYPE);
    PrintArray(pRPLUTType, nQualityLevels, UNITS_TYPE_INT);
    LOGD("%s =>",CABL_RP_WST);
    PrintArray(pRPWindowSizeThreshold, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_RP_FCT);
    PrintArray(pRPFilterCoefficientThreshold, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_RP_BL_RF);
    PrintArray(pRPBackLightReductionFactor, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_RP_BL_SSHC);
    PrintArray(pRPBackLightStepSizeHighCorrelation, nQualityLevels,
        UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_RP_SC_CORR_THRES);
    PrintArray(pRPSceneCorrelationThreshold, nQualityLevels, UNITS_TYPE_UINT);
    LOGD("%s =>",CABL_RP_SC_CHANGE_THRES);
    PrintArray(pRPSceneChangeThreshold, nQualityLevels, UNITS_TYPE_UINT);

    LOGD("*******************************************************************");

}

/*
 * FUNCTION: LoadDefaults
 *
 * This is the fallback in case parsing fails
 *
 */
void CABLHWParams::LoadDefaults() {

    uint32_t BLMinRatio[]          = {  461,  614,  768};
    uint32_t BLMaxRatio[]          = { 1024, 1024, 1024};
    uint32_t PixelDistortion[]     = {  150,  100,  100};
    uint32_t BLChangeSpeed[]       = {    8,   12,   16};

    double   RPSoftClippingSlope[]                = { 0.32, 0.28, 0.25};
    int32_t  RPLUTType[]                          = {    3,    3,    3};
    uint32_t RPWindowSizeThreshold[]              = {    8,    6,    4};
    uint32_t RPFilterCoefficientThreshold[]       = {  820,  409,  409};
    uint32_t RPBackLightReductionFactor[]         = {    4,    2,    4};
    uint32_t RPBackLightStepSizeHighCorrelation[] = {    8,    8,    8};
    uint32_t RPSceneCorrelationThreshold[]        = { 1020, 1024, 1024};
    uint32_t RPSceneChangeThreshold[]             = {  800,  800,  800};

    uint32_t BLResponseBL[]        = { 0, 100, 200, 300, 400, 500, 600, 700,
                                800, 900, 1024};
    uint32_t BLResponseLuminance[] = { 0, 100, 200, 300, 400, 500, 600, 700,
                                800, 900, 1024};

    uint32_t GammaGrayScale[]  = {
               0,   32,   64,   96,  129,  161,  193,  225,
             257,  289,  321,  353,  386,  418,  450,  482,
             514,  546,  578,  610,  643,  675,  707,  739,
             771,  803,  835,  867,  900,  932,  964,  996,
            1024};
    uint32_t GammaLuminance[]  = {
               0,    1,    2,    6,   11,   17,   26,   36,
              49,   63,   79,   98,  118,  141,  166,  193,
             223,  255,  289,  325,  364,  405,  449,  495,
             544,  595,  649,  705,  763,  825,  888,  955,
            1024};

    mLengthGammaLUT                        =  33;
    mLengthBLLuminanceLUT                  =  11;

    mBLLevelThreshold                      = 124;

    if (CopyArray(&pBLMinRatio, BLMinRatio, nQualityLevels) ||
        CopyArray(&pBLMaxRatio, BLMaxRatio, nQualityLevels) ||
        CopyArray(&pPixelDistortion, PixelDistortion, nQualityLevels) ||
        CopyArray(&pBLChangeSpeed, BLChangeSpeed, nQualityLevels) ||
        CopyArray(&pRPSoftClippingSlope, RPSoftClippingSlope, nQualityLevels) ||
        CopyArray(&pRPLUTType, RPLUTType, nQualityLevels) ||
        CopyArray(&pRPLUTType, RPLUTType, nQualityLevels) ||
        CopyArray(&pRPWindowSizeThreshold, RPWindowSizeThreshold,
            nQualityLevels) ||
        CopyArray(&pRPFilterCoefficientThreshold, RPFilterCoefficientThreshold,
            nQualityLevels) ||
        CopyArray(&pRPBackLightReductionFactor, RPBackLightReductionFactor,
            nQualityLevels) ||
        CopyArray(&pRPBackLightStepSizeHighCorrelation,
            RPBackLightStepSizeHighCorrelation, nQualityLevels) ||
        CopyArray(&pRPSceneCorrelationThreshold, RPSceneCorrelationThreshold,
            nQualityLevels) ||
        CopyArray(&pRPSceneChangeThreshold, RPSceneChangeThreshold,
            nQualityLevels) ||
        CopyArray(&pBLResponseBL, BLResponseBL, mLengthBLLuminanceLUT) ||
        CopyArray(&pBLResponseLuminance, BLResponseLuminance,
            mLengthBLLuminanceLUT) ||
        CopyArray(&pGammaGrayScale, GammaGrayScale, mLengthGammaLUT) ||
        CopyArray(&pGammaLuminance, GammaLuminance, mLengthGammaLUT))
        XMLParseError(ERROR_NO_MEMORY);

}

/*
 * FUNCTION: FreeUnusedMemory
 *
 * This is to free all the memory allocated dynamically which is not used after values
 * have been copied to the algorithm OEM parameters
 *
 */
void CABLHWParams::FreeUnusedMemory() {
    free(pBLMinRatio);
    free(pBLMaxRatio);
    free(pPixelDistortion);
    free(pBLChangeSpeed);
    free(pRPSoftClippingSlope);
    free(pRPLUTType);
    free(pRPWindowSizeThreshold);
    free(pRPFilterCoefficientThreshold);
    free(pRPBackLightReductionFactor);
    free(pRPBackLightStepSizeHighCorrelation);
    free(pRPSceneCorrelationThreshold);
    free(pRPSceneChangeThreshold);
}

/* Function for initializing ABA CABL Config Parameters */
int32_t CABLHWParams::ConfigureCABLParameters(CablInitialConfigType *cp) {

    int32_t i, ret = 0;
    CablQualityParametersType *qp;
    char property[PROPERTY_VALUE_MAX];
    memset(cp, 0, sizeof(CablInitialConfigType));

    if (property_get(PROP_CABL_XML_SET, property, 0) <= 0)
        goto load_default;

    if (property_get(PROP_CABL_XML_PATH, property, 0) > 0) {
        ret = ParseXML(property);
        if (ret) {
            XMLParseError(ret);
            goto load_default;
        }
    }

    ret = ParseParameters();
    if (!ret)
        goto config_start;

load_default:
    LoadDefaults();

config_start:
    qp = cp->aCablQualityParameters;

    for (i = ABL_QUALITY_LOW; i < ABL_QUALITY_MAX; i++) {
       qp[i].uBacklightScaleRatioLowerLimit    = pBLMinRatio[i];
       qp[i].uBacklightScaleRatioUpperLimit    = pBLMaxRatio[i];
       qp[i].uPixelDistortionRate              = pPixelDistortion[i];
       qp[i].uFilterStepSize                   = pBLChangeSpeed[i];
       qp[i].SoftClippingSlope                 = pRPSoftClippingSlope[i];
       qp[i].uLutType                          = pRPLUTType[i];
       qp[i].uWindowSizeThreshold              = pRPWindowSizeThreshold[i];
       qp[i].uFilterCoefficientThreshold       =
                                               pRPFilterCoefficientThreshold[i];
       qp[i].uBacklightReductionFactor         = pRPBackLightReductionFactor[i];
       qp[i].uBacklightStepSizeHighCorrelation =
                                         pRPBackLightStepSizeHighCorrelation[i];
       qp[i].uSceneCorrelationThreshold        =
                                                pRPSceneCorrelationThreshold[i];
       qp[i].uSceneChangeThreshold             = pRPSceneChangeThreshold[i];
    }

    cp->uGammaResponseTableLength              = mLengthGammaLUT;
    cp->uBacklightResponseTableLength          = mLengthBLLuminanceLUT;
    cp->pGammaResponseX                        = pGammaGrayScale;
    cp->pGammaResponseY                        = pGammaLuminance;
    cp->pBacklightResponseX                    = pBLResponseBL;
    cp->pBacklightResponseY                    = pBLResponseLuminance;
    cp->uBacklightThresholdLevel               = mBLLevelThreshold;

    FreeUnusedMemory();
    return ABA_STATUS_SUCCESS;
}

/* Function for initializing ABL CABL Config Parameters */
int32_t CABLHWParams::ConfigureABLParameters(bl_oem_api *api_para,
            uint32_t initQualityLevel, uint32_t initDataLevel) {
    uint32_t i;
    int maxBLFd = -1, ret = 0;
    ssize_t bytes;
    char buffer[MAX_BACKLIGHT_LEN];
    cabl_per_ql_params *q;

    char property[PROPERTY_VALUE_MAX];
    (void)memset(api_para, 0, sizeof(bl_oem_api));

    if (property_get(PROP_CABL_XML_SET, property, 0) <= 0)
        goto load_default;

    if (property_get(PROP_CABL_XML_PATH, property, 0) > 0) {
        ret = ParseXML(property);
        if (ret) {
            XMLParseError(ret);
            goto load_default;
        }
    }

    ret = ParseParameters();
    if (!ret)
        goto config_start;

load_default:
    LoadDefaults();

config_start:
    if (property_get("debug.cabl.logs", property, 0) > 0) {
        api_para->bl_debug = atoi(property);
        api_para->bl_debug = ((api_para->bl_debug < 0) ? 0
                : ((api_para->bl_debug > 3) ? 3 : api_para->bl_debug));
    }

    q = api_para->cabl_quality_params;

    for (i = 0; i < nQualityLevels; i++) {
        q[i].bl_min_ratio         = pBLMinRatio[i];
        q[i].bl_max_ratio         = pBLMaxRatio[i];
        q[i].pixel_distortion     = pPixelDistortion[i];
        q[i].bl_filter_stepsize   = pBLChangeSpeed[i];

        q[i].reserved_param_SS    = pRPSoftClippingSlope[i];
        q[i].reserved_param_LT    = pRPLUTType[i];
        q[i].reserved_param_WST   = pRPWindowSizeThreshold[i];
        q[i].reserved_param_FCT   = pRPFilterCoefficientThreshold[i];
        q[i].reserved_param_BDF   = pRPBackLightReductionFactor[i];
        q[i].reserved_param_BSTHC = pRPBackLightStepSizeHighCorrelation[i];
        q[i].reserved_param_SCT   = pRPSceneCorrelationThreshold[i];
        q[i].reserved_param_SCD   = pRPSceneChangeThreshold[i];
    }

    api_para->default_ql_mode     = mDefaultQualityMode;
    if (property_get("hw.cabl.level", property, 0) > 0) {
        if (strncmp(property, CABL_LVL_AUTO, sizeof(CABL_LVL_AUTO)-1) != 0)
            api_para->default_ql_mode = USER_QL_MODE;
    }

    api_para->bl_max_level = mBLMaxLevel;
    maxBLFd = open(SYS_MAX_BRIGHTNESS, O_RDONLY);
    if (maxBLFd < 0) {
        LOGE("%s: Cannot open backlight node!", __func__);
    } else {
        memset(buffer, 0, MAX_BACKLIGHT_LEN);
        bytes = pread(maxBLFd, buffer, sizeof(char) * MAX_BACKLIGHT_LEN, 0);
        if (bytes > 0) {
            api_para->bl_max_level = atoi(&buffer[0]);
        }
        close(maxBLFd);
    }

    api_para->ui_quality_lvl     = mUIQualityLevel;
    api_para->video_quality_lvl  = mVideoQualityLevel;
    api_para->SetLevel           = initQualityLevel;
    api_para->bl_level_len[0]    = mLengthGammaLUT;
    api_para->bl_level_len[1]    = mLengthBLLuminanceLUT;
    api_para->pY_shade           = pGammaGrayScale;
    api_para->pY_gamma           = pGammaLuminance;
    api_para->pY_lvl             = pBLResponseBL;
    api_para->pbl_lvl            = pBLResponseLuminance;
    api_para->bl_level_threshold = mBLLevelThreshold;

    api_para->bl_min_level       = mBLMinLevel;
    api_para->orig_level         = (initDataLevel > mBLMinLevel) ?
                                initDataLevel: mBLMinLevel;

    FreeUnusedMemory();
    return 0;
}


/* Start SVIHWParams Functions */
const char *SVIHWParams::ID = SVI_GROUP_ID;

/*
 * FUNCTION: ParseArrayParameters
 *
 * Parses Lux and BL Arrays for SVI
 *
 * ASSUMPTION: Parameter Arrays are of Type UINT
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t SVIHWParams::ParseArrayParameters(){
    int32_t ret = 0;

    ret = ParseArrayElement<uint32_t>(SVI_BL_RESP_IN, UNITS_TYPE_UINT,
                &pBLRespInput, mLengthBLResponseTable);
    if (ret)
        return ret;

    ret = ParseArrayElement<uint32_t>(SVI_BL_RESP_OUT, UNITS_TYPE_UINT,
                &pBLRespOutput, mLengthBLResponseTable);
    if (ret)
        goto exit_mem1;

    ret = ParseArrayElement<uint32_t>(SVI_SENSOR_RESP_REF, UNITS_TYPE_UINT,
                &pSensorRespRef, mLengthSensorResponseTable);
    if (ret)
        goto exit_mem2;

    ret = ParseArrayElement<uint32_t>(SVI_SENSOR_RESP_TABLE, UNITS_TYPE_UINT,
                &pSensorRespSensor, mLengthSensorResponseTable);
    if (ret)
        goto exit_mem3;

    return ret;

exit_mem3:
    free(pSensorRespRef);
exit_mem2:
    free(pBLRespOutput);
exit_mem1:
    free(pBLRespInput);
    return ret;
}

/*
 * FUNCTION: ParseNonArrayParameters
 *
 * Parses parameters which are not arrays like lengths and uint
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t SVIHWParams::ParseNonArrayParameters() {
    int32_t ret = 0;
    int32_t tmp = 0;

    ret = GetIntegerValue(SVI_LEN_BL_RESP, &tmp);
    if (ret)
        return ret;
    mLengthBLResponseTable = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_LEN_SENSOR_RESP, &tmp);
    if (ret)
        return ret;
    mLengthSensorResponseTable = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_FILTER_SIZE_UI, &tmp);
    if (ret)
        return ret;
    mFilterStepSizeUI = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_FILTER_SIZE_VIDEO, &tmp);
    if (ret)
        return ret;
    mFilterStepSizeVideo = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_CONT_STRENGTH, &tmp);
    if (ret)
        return ret;
    mContrastStrength = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_BRIGHT_STRENGTH, &tmp);
    if (ret)
        return ret;
    mBrightnessStrength = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_PANEL_REFLECT_RATIO, &tmp);
    if (ret)
        return ret;
    mPanelReflectanceRatio = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_PANEL_PEAK_BRIGHT, &tmp);
    if (ret)
        return ret;
    mPanelPeakBrightness = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_BL_MIN_RED_RATIO, &tmp);
    if (ret)
        return ret;
    mMinBLReductionRatio = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_BL_REDUCTION_MODE, &tmp);
    if (ret)
        return ret;
    mBacklightReductionMode = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_EN_BL_REDUCTION, &tmp);
    if (ret)
        return ret;
    mEnableBacklightReduction = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_INDOOR_MIN_LUX, &tmp);
    if (ret)
        return ret;
    mIndoorMinLux = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_INDOOR_MIN_BRIGHT, &tmp);
    if (ret)
        return ret;
    mIndoorMinBright = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_INDOOR_MAX_LUX, &tmp);
    if (ret)
        return ret;
    mIndoorMaxLux = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_INDOOR_MAX_BRIGHT, &tmp);
    if (ret)
        return ret;
    mIndoorMaxBright = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_OUTDOOR_MAX_LUX, &tmp);
    if (ret)
        return ret;
    mOutdoorMaxLux = tmp > 0 ? tmp : 0;

    ret = GetIntegerValue(SVI_OUTDOOR_MAX_BRIGHT, &tmp);
    if (ret)
        return ret;
    mOutdoorMaxBright = tmp > 0 ? tmp : 0;

    return ret;
}

/*
 * FUNCTION: VerifyID
 *
 * Checks if ID in group element of XML matches the one of class
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t SVIHWParams::VerifyID() {
    int32_t ret = 0;
    char gpID[MAX_VARIABLE_NAME_LENGTH];

    ret = GetGroupID(gpID);
    if (ret)
        return ret;

    if (strncmp(ID, gpID, sizeof(ID)-1) != 0)
        ret = ERROR_CONTENT_FORMAT;

    return ret;
}

/*
 * FUNCTION: ParseParameters
 *
 * Calls Parsing of parameters one after the other
 * Works on handling error cases and setting of values of CP class
 *
 * Return :
 * 0 = Success
 * Else error code
 */
int32_t SVIHWParams::ParseParameters() {
    int32_t ret = 0;
    char property[PROPERTY_VALUE_MAX];

    ret = VerifyID();
    if (ret)
        goto exit_params;

    ret = ParseNonArrayParameters();
    if (ret)
        goto exit_params;

    ret = ParseArrayParameters();
    if (ret)
        goto exit_params;

    if (property_get(PROP_SVI_XML_PRINT, property, 0) > 0)
        PrintParsedParameters();

return ret;

exit_params:
    XMLParseError(ret);
    LOGE("%s: Parsing Error occured, Loading Defaults ret = %d", __func__, ret);
    LoadDefaults();
    return ret;

}

/*
 * FUNCTION: PrintParsedParameters
 *
 * This is to print parsed parameters to logcat if prop PROP_SVI_XML_PRINT is set
 *
 */
void SVIHWParams::PrintParsedParameters() {

    LOGD("*******************************************************************");
    LOGD("%s: Printing out the parsed parameters for SVI", __func__);
    LOGD("*******************************************************************");
    LOGD("BL Response Table Length_______= %u", mLengthBLResponseTable);
    LOGD("Sensor Response Table Length___= %u", mLengthSensorResponseTable);
    LOGD("Filter Stepsize UI Mode________= %u", mFilterStepSizeUI);
    LOGD("Filter Stepsize Video Mode_____= %u", mFilterStepSizeVideo);
    LOGD("Contrast Strength______________= %u", mContrastStrength);
    LOGD("Brightness Strength____________= %u", mBrightnessStrength);
    LOGD("Panel Reflectance Ratio________= %u", mPanelReflectanceRatio);
    LOGD("Panel Peak Brigthness__________= %u", mPanelPeakBrightness);
    LOGD("BL Min Reduction Ratio_________= %u", mMinBLReductionRatio);
    LOGD("BL Reduction Mode______________= %u", mBacklightReductionMode);
    LOGD("Enable BL Reduction____________= %u", mEnableBacklightReduction);
    LOGD("Indoor Min LUX value___________= %u", mIndoorMinLux);
    LOGD("Indoor Min Brightness__________= %u", mIndoorMinBright);
    LOGD("Indoor Max LUX value___________= %u", mIndoorMaxLux);
    LOGD("Indoor Max Brightness__________= %u", mIndoorMaxBright);
    LOGD("Outdoor Max LUX value___________= %u", mOutdoorMaxLux);
    LOGD("Outdoor Max Brightness__________= %u", mOutdoorMaxBright);

    LOGD("\nBacklight Response Parameters:");
    LOGD("%s =>",SVI_BL_RESP_IN);
    PrintArray(pBLRespInput, mLengthBLResponseTable, UNITS_TYPE_UINT);
    LOGD("%s =>",SVI_BL_RESP_OUT);
    PrintArray(pBLRespOutput, mLengthBLResponseTable, UNITS_TYPE_UINT);

    LOGD("\nSensor Response Parameters:");
    LOGD("%s =>",SVI_SENSOR_RESP_REF);
    PrintArray(pSensorRespRef, mLengthSensorResponseTable, UNITS_TYPE_UINT);
    LOGD("%s =>",SVI_SENSOR_RESP_TABLE);
    PrintArray(pSensorRespSensor, mLengthSensorResponseTable, UNITS_TYPE_UINT);

    LOGD("*******************************************************************");

}

/*
 * FUNCTION: LoadDefaults
 *
 * This is the fallback in case parsing fails
 *
 */
void SVIHWParams::LoadDefaults() {

    uint32_t BLRespInput[]       = {    0,  100,  255};
    uint32_t BLRespOutput[]      = {    0,  100,  255};

    uint32_t SensorRespRef[]     = {  297, 1495, 2900, 4226, 4750, 5551, 10500};
    uint32_t SensorRespSensor[]  = {  297, 1495, 2900, 4226, 4750, 5551, 10500};

    mLengthBLResponseTable                  =    3;
    mLengthSensorResponseTable              =    7;

    mFilterStepSizeUI                       =   16;
    mFilterStepSizeVideo                    =    8;

#ifdef MDP3_TARGET
    mContrastStrength                       =    1;
    mBrightnessStrength                     =   64;
#else
    mContrastStrength                       =  128;
    mBrightnessStrength                     =  128;
#endif

    mPanelReflectanceRatio                  =    8;
    mPanelPeakBrightness                    =  250;

    mMinBLReductionRatio                    =  820;
    mBacklightReductionMode                 =    0;
    mEnableBacklightReduction               =    0;

    mIndoorMinLux                           =    3;
    mIndoorMinBright                        =   17;

    mIndoorMaxLux                           = 3000;
    mIndoorMaxBright                        =  135;

    mOutdoorMaxLux                          =15000;
    mOutdoorMaxBright                       =  255;

    if (CopyArray(&pBLRespInput, BLRespInput, mLengthBLResponseTable) ||
        CopyArray(&pBLRespOutput, BLRespOutput, mLengthBLResponseTable) ||
        CopyArray(&pSensorRespRef, SensorRespRef, mLengthSensorResponseTable) ||
        CopyArray(&pSensorRespSensor, SensorRespSensor,
            mLengthSensorResponseTable))
        XMLParseError(ERROR_NO_MEMORY);

}


/* Function for initializing ABA SVI Config Parameters */
int32_t SVIHWParams::ConfigureSVIParameters(SVIConfigParametersType *cp) {
    int32_t i, ret = 0;

    char property[PROPERTY_VALUE_MAX];
    memset(cp, 0, sizeof(SVIConfigParametersType));

    if (property_get(PROP_SVI_XML_SET, property, 0) <= 0)
        goto load_default;

    if (property_get(PROP_SVI_XML_PATH, property, 0) > 0) {
        ret = ParseXML(property);
        if (ret) {
            XMLParseError(ret);
            goto load_default;
        }
    }

    ret = ParseParameters();
    if (!ret)
        goto config_start;

load_default:
    LoadDefaults();

config_start:
    cp->uContrastStrengthFactor             = mContrastStrength;
    cp->uFilterStepSizeUIMode               = mFilterStepSizeUI;
    cp->uFilterStepSizeVideoMode            = mFilterStepSizeVideo;
    cp->uBrightnessStrengthFactor           = mBrightnessStrength;
    cp->pSensorMappingTableReference        = pSensorRespRef;
    cp->pSensorMappingTableSensor           = pSensorRespSensor;
    cp->uSensorMappingTableLength           = mLengthSensorResponseTable;
    cp->uPanelReflectanceRatio              = mPanelReflectanceRatio;
    cp->uPanelPeakBrightness                = mPanelPeakBrightness;
    cp->pBacklightResponseTableInput        = pBLRespInput;
    cp->pBacklightResponseTableOutput       = pBLRespOutput;
    cp->uBacklightResponseTableLength       = mLengthBLResponseTable;
    cp->uBacklightReductionRatio            = mMinBLReductionRatio;
    cp->uBacklightReductionMode             = mBacklightReductionMode;
    cp->bEnableBacklightReduction           = mEnableBacklightReduction;
    cp->uIndoorMinLuxLevel                  = mIndoorMinLux;
    cp->uIndoorMinBrightnessLevel           = mIndoorMinBright;
    cp->uIndoorMaxLuxLevel                  = mIndoorMaxLux;
    cp->uIndoorMaxBrightnessLevel           = mIndoorMaxBright;
    cp->uOutdoorLuxlevel                    = mOutdoorMaxLux;
    cp->uOutdoorMaxBrightnessLevel          = mOutdoorMaxBright;

    return ABA_STATUS_SUCCESS;
}

