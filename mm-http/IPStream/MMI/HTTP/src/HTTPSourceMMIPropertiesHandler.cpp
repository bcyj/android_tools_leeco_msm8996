/************************************************************************* */
/**
 * HTTPSourceMMIPropertiesHandler.cpp
 * @brief Implementation of HTTPSourceMMIPropertiesHandler.
 *  HTTPSourceMMIPropertiesHandler is a helper class for HTTPSourceMAP that
 *  sets and gets HTTP specific properties to and from HTTP streamer library.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIPropertiesHandler.cpp#23 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIPropertiesHandler.cpp
** ======================================================================= */
#include <stddef.h>
#include "HTTPSourceMMIPropertiesHandler.h"
#include "HTTPController.h"
#include "httpInternalDefs.h"

#include <MMFile.h>

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
const HTTPSourceMMIPropertiesHandler::PropKeyNameIDPair
HTTPSourceMMIPropertiesHandler::m_PropKeyName2IDMap[] =
{
  {"HTTP_PROPERTY_DATA_STORAGE", iHTTPBase::HTTP_PROPERTY_DATA_STORAGE},
  {"HTTP_PROPERTY_INIT_PREROLL_MSEC", iHTTPAPI::HTTP_PROPERTY_INIT_PREROLL_MSEC},
  {"HTTP_PROPERTY_REBUFF_PREROLL_MSEC", iHTTPAPI::HTTP_PROPERTY_REBUFF_PREROLL_MSEC},
  {"HTTP_PROPERTY_DISABLE_MEDIA_SAMPLE_LOGGING", INTERNAL_HTTP_PROPERTY_DISABLE_MEDIA_SAMPLE_LOGGING},
  {"HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT", iHTTPBase::HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT},
  {"HTTP_PROPERTY_NUM_REQUESTS", iHTTPAPI::HTTP_PROPERTY_NUM_REQUESTS},
  {"HTTP_PROPERTY_DISABLE_DATA_UNIT_CANCELLATION", iHTTPBase::HTTP_PROPERTY_DISABLE_DATA_UNIT_CANCELLATION},
  {"HTTP_PROPERTY_MAX_SUPPORTED_REPRESENTATION_BANDWIDTH", iHTTPBase::HTTP_PROPERTY_MAX_SUPPORTED_REPRESENTATION_BANDWIDTH},
  {"HTTP_PROPERTY_MAX_SUPPORTED_ASC_VALUE", iHTTPBase::HTTP_PROPERTY_MAX_SUPPORTED_ASC_VALUE},
  {"HTTP_PROPERTY_USE_TSB_FOR_STARTUP_LATENCY_IMPROVEMENT", iHTTPBase::HTTP_PROPERTY_USE_TSB_FOR_STARTUP_LATENCY_IMPROVEMENT},
  {"HTTP_PROPERTY_QUALCOMM_TRANSPORT_ACCELERATOR", iHTTPBase::HTTP_PROPERTY_QUALCOMM_TRANSPORT_ACCELERATOR},
  {"HTTP_PROPERTY_ENABLE_AUDIO_SWITCHING", iHTTPBase::HTTP_PROPERTY_ENABLE_AUDIO_SWITCHING}
};
/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief Set the value of property.
  *
  * @param[in] pKey - Reference to property key
  * @param[in] pProp - Reference to property value structure
  * @return
  * rc_OK - Property value set successfully
  * rc_notOK - Otherwise
  */
bool HTTPSourceMMIPropertiesHandler::SetProperty
(
 const int8* pKey,
 const tProperty* pProp
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIPropertiesHandler::SetProperty" );
  bool result = false;

  if (m_pHTTPController == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: m_pHTTPController is NULL - property cannot be set" );
  }
  else if (pKey && pProp && pProp->data)
  {
    result = true;
    const char* pValue = reinterpret_cast<const char *> (pProp->data);
    uint32 id = HTTPCommon::ConvertString2UnsignedInteger(reinterpret_cast<const char *> (pKey));
    switch (id)
    {
    case iHTTPBase::HTTP_PROPERTY_DATA_STORAGE:
      {
        //Set data storage option
        DataStorageType dataStorage =
          static_cast<DataStorageType> (HTTPCommon::ConvertString2UnsignedInteger(pValue));
        m_pHTTPController->SetDataStorageOption(dataStorage);
      }
      break;

    case iHTTPAPI::HTTP_PROPERTY_INIT_PREROLL_MSEC:
      {
        //Set initial preroll
        uint32 initPreroll = HTTPCommon::ConvertString2UnsignedInteger(pValue);
        m_pHTTPController->SetInitialPreroll(initPreroll);
      }
      break;

    case iHTTPAPI::HTTP_PROPERTY_REBUFF_PREROLL_MSEC:
      {
        //Set rebuffering preroll
        uint32 rebuffPreroll = HTTPCommon::ConvertString2UnsignedInteger(pValue);
        m_pHTTPController->SetRebufferPreroll(rebuffPreroll);
      }
      break;

    case INTERNAL_HTTP_PROPERTY_DISABLE_MEDIA_SAMPLE_LOGGING:
      {
        //Set media sample logging flag
        bool bMediaSampleLoggingAllowed = (*pValue == 0);
        SetMediaSampleLoggingAllowed(bMediaSampleLoggingAllowed);
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT:
      {
        //Set data heap storage limit
        uint32 heapStorageLimit = HTTPCommon::ConvertString2UnsignedInteger(pValue);
        heapStorageLimit *= 1024 * 1024;
        m_pHTTPController->SetDataHeapStorageLimit(heapStorageLimit);
      }
      break;
    case iHTTPAPI::HTTP_PROPERTY_NUM_REQUESTS:
      {
      //Set number of http requests limit
         uint32 httpReqsLimit = HTTPCommon::ConvertString2UnsignedInteger(pValue);
         m_pHTTPController->SetHTTPRequestsLimit(httpReqsLimit);
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_DISABLE_DATA_UNIT_CANCELLATION:
      {
        bool bIsSet = ((pValue != NULL) ? true : false);
        m_pHTTPController->SetDataUnitCancellationDisabled(bIsSet);
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_MAX_SUPPORTED_REPRESENTATION_BANDWIDTH:
      {
        uint32 maxBw = HTTPCommon::ConvertString2UnsignedInteger(pValue);
        m_pHTTPController->SetMaxSupportedRepBandwidth(maxBw);
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_MAX_SUPPORTED_ASC_VALUE:
      {
        uint32 maxASCVal = HTTPCommon::ConvertString2UnsignedInteger(pValue);
        m_pHTTPController->SetMaxSupportedASCValue(maxASCVal);
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_USE_TSB_FOR_STARTUP_LATENCY_IMPROVEMENT:
      {
        bool bIsSet = ((pValue != NULL) ? true : false);
        if (bIsSet)
        {
          m_pHTTPController->UseTsbForStartupLatencyImprovement();
        }
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_ENABLE_AUDIO_SWITCHING:
      {
        bool bIsSet = (*pValue);
        m_pHTTPController->SetAudioSwitchingEnabled(bIsSet);
      }
      break;

    default:
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: Unsupported property for Set - id %u", id );
      result = false;
      break;
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either invalid input key or value" );
  }

  return result;
}

/** @brief Get the value of property given the key.
  *
  * @param[in] pKey - Reference to property key
  * @param[out] pProp - Reference to property value structure
  * @return
  * rc_OK - Queried property successfully read
  * rc_notOK - Otherwise
  */
bool HTTPSourceMMIPropertiesHandler::GetProperty
(
 const int8* pKey,
 tProperty* pProp
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIPropertiesHandler::GetProperty" );
  bool result = false;
  DataStorageType dataStorage = iHTTPAPI::DEFAULT_STORAGE;

  if (m_pHTTPController == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: m_pHTTPController is NULL - property cannot be obtained" );
  }
  else if (pKey && pProp)
  {
    result = true;
    PropValue value;
    uint32 id = HTTPCommon::ConvertString2UnsignedInteger(reinterpret_cast<const char *> (pKey));
    switch (id)
    {
    case iHTTPBase::HTTP_PROPERTY_DATA_STORAGE:
      {
        //Get data storage option
        (void)m_pHTTPController->GetDataStorageOption(dataStorage);
        value.type = VT_UINT32;
        value.v_u = static_cast<uint32> (dataStorage);
      }
      break;

    case iHTTPAPI::HTTP_PROPERTY_INIT_PREROLL_MSEC:
      {
        //Get initial preroll
        value.type = VT_UINT32;
        value.v_u = m_pHTTPController->GetInitialPreroll();
      }
      break;

    case iHTTPAPI::HTTP_PROPERTY_REBUFF_PREROLL_MSEC:
      {
        //Get rebuffering preroll
        value.type = VT_UINT32;
        value.v_u = m_pHTTPController->GetRebufferPreroll();
      }
      break;

    case INTERNAL_HTTP_PROPERTY_DISABLE_MEDIA_SAMPLE_LOGGING:
      {
        //Get media sample logging flag
        value.type = VT_BOOL;
        value.v_b = !IsMediaSampleLoggingAllowed();
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT:
      {
        //Get data heap storage limit
        value.type = VT_UINT32;
        value.v_u = (m_pHTTPController->GetDataHeapStorageLimit() / 1024) / 1024;
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_NUM_REQUESTS:
      {
      //Get number of http requests
        value.type = VT_UINT32;
        value.v_u = (m_pHTTPController->GetHTTPRequestsLimit());
      }
      break;

    case iHTTPBase::HTTP_PROPERTY_DISABLE_DATA_UNIT_CANCELLATION:
      {
        value.type = VT_BOOL;
        value.v_u = m_pHTTPController->IsDataUnitCancellationDisabled();
      }
      break;

    default:
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: Unsupported property for Get - id %u", id );
      break;
    }

    if (result)
    {
      result = SetPropValue(value, pProp);
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Invalid input param" );
  }

  return result;
}

/** @brief Set the property value based on its type.
  *
  * @param[in] propValue - Property value
  * @param[out] pProp - Reference to property value structure
  * @return
  * rc_OK - Property value set
  * rc_notOK - Otherwise
  */
bool HTTPSourceMMIPropertiesHandler::SetPropValue
(
 const PropValue& propValue,
 tProperty* pProp
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIPropertiesHandler::SetPropValue" );
  bool result = false;

  if (pProp)
  {
    size_t reqdSize = 0;
    switch (propValue.type)
    {
    case VT_BOOL:
      reqdSize = sizeof(bool);
      break;
    case VT_UINT32:
      reqdSize = sizeof(uint32);
      break;
    case VT_STRING:
      if (propValue.v_s)
      {
        reqdSize = std_strlen(propValue.v_s) + 1;
      }
      break;
    default:
      break;
    }

    if (pProp->data == NULL)
    {
      pProp->size = reqdSize;
      result = true;
    }
    else if (pProp->size >= reqdSize)
    {
      switch (propValue.type)
      {
      case VT_BOOL:
        *(pProp->data) = propValue.v_b;
        result = true;
        break;
      case VT_UINT32:
        HTTPCommon::ConvertUnsignedInteger2String(propValue.v_u,
                                                  reinterpret_cast<char *> (pProp->data));
        result = true;
        break;
      case VT_STRING:
        if (propValue.v_s)
        {
          std_strlcpy(reinterpret_cast<char *> (pProp->data), propValue.v_s, reqdSize);
          result = true;
        }
        break;
      default:
        break;
      }
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: Insufficient prop value size - "
                     "reqdSize %u, pProp->size %u",
                     reqdSize, pProp->size );
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pProp is NULL" );
  }

  return result;
}

/** @brief Set the properties from HTTP property config file (if exists).
  *
  * @param[in] pConfigFilePath - Reference to config file path
  * @return
  * TRUE - File doesn't exist or properties set successfully if file exists
  * FALSE - Otherwise
  */
bool HTTPSourceMMIPropertiesHandler::SetPropertiesFromConfigFile
(
 const char* pConfigFilePath
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIPropertiesHandler::SetPropertiesFromConfigFile" );
  bool bOk = false;

  if (pConfigFilePath)
  {
    MM_HANDLE pConfigFile = NULL;

    //Open HTTP property config file in the RO mode. Don't treat the error as fatal
    bOk = (MM_File_Create(pConfigFilePath, MM_FILE_CREATE_R, &pConfigFile) == 0 && pConfigFile);
    if (!bOk)
    {
      QTV_MSG_SPRINTF_PRIO_1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                              "Error: HTTP property config file open failed %s, could be missing",
                            pConfigFilePath );
      bOk = true;
    }
    else
    {
      bOk = false;
      char* pConfigFileInfo = NULL;
      size_t configFileSize = 0;
      if (MM_File_GetSize(pConfigFile, (unsigned long *) &configFileSize) == 0 && configFileSize > 0)
      {
        pConfigFileInfo = (char*)QTV_Malloc(configFileSize + 1);
        if (pConfigFileInfo == NULL)
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "Error: Memory allocation failed for config file info size : %u", configFileSize );
        }
        else
        {
          size_t numRead = 0;
          if (MM_File_Read(pConfigFile, pConfigFileInfo, (ssize_t)configFileSize, (ssize_t *)&numRead) == 0 &&
              numRead > 0)
          {
            if (static_cast<size_t> (configFileSize) != numRead)
            {
              QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Error: Cannot read config file, only read %d of %u bytes" ,
                             numRead, configFileSize );
            }
            else
            {
              //Config file info successfully read into pConfigFileInfo
              pConfigFileInfo[configFileSize] = '\0';
              bOk = true;
            }
          }
        }
      }// if (MM_File_GetSize() == 0 && configFileSize > 0)

      //Close the config file (release file handle)
      (void)MM_File_Release(pConfigFile);
      pConfigFile = NULL;

      //Proceed with config file parsing if success
      if (bOk)
      {
        bOk = ParseConfigFile(pConfigFileInfo);
      }

      if (pConfigFileInfo)
      {
        QTV_Free(pConfigFileInfo);
        pConfigFileInfo = NULL;
      }
    }

    //Close the config file (release file handle)
    if(pConfigFile)
    {
      (void)MM_File_Release(pConfigFile);
      pConfigFile = NULL;
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: SetPropertiesFromConfigFile failed - bad input" );
  }

  return bOk;
}

/** @brief Parse config file and set the HTTP properties.
  *
  * @param[in] pConfigFileInfo - Reference to config file info
  * @return
  * TRUE - Config file parsed successfully
  * FALSE - Otherwise
  */
bool HTTPSourceMMIPropertiesHandler::ParseConfigFile
(
 char* pConfigFileInfo
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIPropertiesHandler::ParseConfigFile" );
  bool bOk = false;

  if (pConfigFileInfo)
  {
    bOk = true;
    size_t configFileLen = (size_t)std_strlen(pConfigFileInfo);
    ptrdiff_t numBytesToBeParsed = configFileLen;
    int numLines = 1;
    char* pLineStart = pConfigFileInfo;
    size_t nNewLineCharLen = 0;

    //Loop through the entire file looking for property lines
    while (numBytesToBeParsed > 0)
    {
      //Find the end of line (in case of multiple lines)
      char* pLineEnd = std_strstr(pLineStart, "\r\n");
      if (pLineEnd == NULL)
      {
        pLineEnd = std_strstr(pLineStart, "\r");
        pLineEnd = (pLineEnd == NULL)? std_strstr(pLineStart, "\n"):pLineEnd;
        nNewLineCharLen = (pLineEnd != NULL)? std_strlen("\r"):0;
      }
      else
      {
        nNewLineCharLen = std_strlen("\r\n");
      }

      if (pLineEnd == NULL)
      {
        pLineEnd = pConfigFileInfo + configFileLen;
      }

      //Look for property key<->value separator ("="). If not found
      //move on to the next line
      char* pSeparator = std_strstr(pLineStart, "=");
      if (pSeparator)
      {
        //Strip trailing spaces from property key and store the key
        char* pPropKeyEnd = pSeparator - 1;
        while ((pPropKeyEnd >= pLineStart) &&
               ((*pPropKeyEnd == ' ') || (*pPropKeyEnd == '\t')))
        {
          pPropKeyEnd--;
        }
        pPropKeyEnd++;
        char keyStr[HTTP_MAX_CONFIG_STRING_LENGTH] = {0},
             valStr[HTTP_MAX_CONFIG_STRING_LENGTH] = {0};
        size_t dstBufLen = STD_MIN(sizeof(keyStr), (size_t)(pPropKeyEnd - pLineStart + 1));
        (void)std_strlcpy(keyStr, pLineStart, dstBufLen);

        //Strip leading spaces from property value and store the value
        char* pPropValStart = pSeparator + 1;
        while ((pPropValStart < pLineEnd) &&
               ((*pPropValStart == ' ') || (*pPropValStart == '\t')))
        {
          pPropValStart++;
        }
        dstBufLen = STD_MIN(sizeof(valStr),(size_t)(pLineEnd - pPropValStart + 1));
        (void)std_strlcpy(valStr, pPropValStart, dstBufLen);

        if (std_strlen(keyStr) > 0 && std_strlen(valStr) > 0)
        {
          //Check for key validity
          for (int i = 0; i < STD_ARRAY_SIZE(m_PropKeyName2IDMap); i++)
          {
            if (std_strcmp(m_PropKeyName2IDMap[i].name, keyStr) == 0)
            {
              uint32 keyInt = m_PropKeyName2IDMap[i].id;
              char propKey[sizeof(uint32) + 1] = {0};
              HTTPCommon::ConvertUnsignedInteger2String(keyInt, propKey);

              //Treat HTTP_PROPERTY_DATA_STORAGE (valid values - HEAP, FILE_SYSTEM, BUFFER_PORT)
              //HTTP_PROPERTY_CONTENT_SAVE_LOCATION, HTTP_PROPERTY_SAVED_CONTENT_LOCATION as
              //strings, while the rest as integers
              tProperty propVal;
              switch (keyInt)
              {
              case iHTTPBase::HTTP_PROPERTY_DATA_STORAGE:
                {
                  iHTTPBase::DataStorageType dataStorageOption = iHTTPBase::DEFAULT_STORAGE;
                  if (std_strcmp(valStr, "HEAP") == 0)
                  {
                    dataStorageOption = iHTTPBase::HEAP;
                  }
                  else if (std_strcmp(valStr, "FILE_SYSTEM") == 0)
                  {
                    dataStorageOption = iHTTPBase::FILE_SYSTEM;
                  }
                  else if (std_strcmp(valStr, "HEAP_FILE_SYSTEM") == 0)
                  {
                    dataStorageOption = iHTTPBase::HEAP_FILE_SYSTEM;
                  }
                  else if (std_strcmp(valStr, "BUFFER_PORT") == 0)
                  {
                    dataStorageOption = iHTTPBase::BUFFER_PORT;
                  }
                  std_memset(valStr, 0x0, sizeof(valStr));
                  HTTPCommon::ConvertUnsignedInteger2String(dataStorageOption, valStr);
                  propVal.size = sizeof(uint32);
                }
                break;

              case iHTTPAPI::HTTP_PROPERTY_INIT_PREROLL_MSEC:
              case iHTTPAPI::HTTP_PROPERTY_REBUFF_PREROLL_MSEC:
              case iHTTPBase::HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT:
              case iHTTPBase::HTTP_PROPERTY_NUM_REQUESTS:
              case iHTTPBase::HTTP_PROPERTY_MAX_SUPPORTED_REPRESENTATION_BANDWIDTH:
              case iHTTPBase::HTTP_PROPERTY_MAX_SUPPORTED_ASC_VALUE:
                {
                  const char * end_ptr = NULL;
                  int errorno;
                  uint32 valInt = std_scanul(valStr, 0, &end_ptr, &errorno);
                  std_memset(valStr, 0x0, sizeof(valStr));
                  HTTPCommon::ConvertUnsignedInteger2String(valInt, valStr);
                  propVal.size = sizeof(uint32);
                }
                break;

              case INTERNAL_HTTP_PROPERTY_DISABLE_MEDIA_SAMPLE_LOGGING:
              case iHTTPBase::HTTP_PROPERTY_DISABLE_DATA_UNIT_CANCELLATION:
              case iHTTPBase::HTTP_PROPERTY_USE_TSB_FOR_STARTUP_LATENCY_IMPROVEMENT:
              case iHTTPBase::HTTP_PROPERTY_QUALCOMM_TRANSPORT_ACCELERATOR:
              case iHTTPBase::HTTP_PROPERTY_ENABLE_AUDIO_SWITCHING:
                {
                  const char * end_ptr = NULL;
                  int errorno;
                  uint32 valInt = std_scanul(valStr, 0, &end_ptr, &errorno);
                  std_memset(valStr, 0x0, sizeof(valStr));
                  valStr[0] = (valInt != 0) ? 1 : 0;
                  propVal.size = sizeof(bool);
                }
                break;

              default:
                break;
              }

              //Set HTTP property (using iPropertyBag iface) into HTTP streamer library
              propVal.data = reinterpret_cast<int8 *> (valStr);
              if (SetProperty(reinterpret_cast<const int8 *> (propKey), &propVal))
              {
                QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                               "ParseConfigFile - Property in line %d successfully set",
                               numLines );
              }

              break;
            }
          }// for (int i = 0; i < STD_ARRAY_SIZE(m_PropKeyName2IDMap); i++)
        }// if (std_strlen(keyStr) > 0 && std_strlen(valStr) > 0)
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Error: ParseConfigFile - Either empty property value or key in line %d",
                         numLines );
        }
      }
      numBytesToBeParsed -= pLineEnd - pLineStart;
      numLines++;

      //Find the start of next line (if more data left to be parsed)
      if (numBytesToBeParsed > 0)
      {
        pLineStart = pLineEnd + nNewLineCharLen;
        numBytesToBeParsed -= nNewLineCharLen;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: ParseConfigFile failed - bad input" );
  }

  return bOk;
}

}/* namespace video */
