#ifndef __HTTPSOURCEMMIPROPERTIESHANDLER_H__
#define __HTTPSOURCEMMIPROPERTIESHANDLER_H__
/************************************************************************* */
/**
 * HTTPSourceMMIPropertiesHandler.h
 * @brief Header file for HTTPSourceMMIPropertiesHandler.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIPropertiesHandler.h#8 $
$DateTime: 2012/08/22 19:17:04 $
$Change: 2724240 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIPropertiesHandler.h
** ======================================================================= */
#include <AEEStdDef.h>
#include <HTTPBase.h>
#include <stdlib.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#if defined (WIN32)
#define HTTP_DEFAULT_CONFIG_FILE_PATH     ".\\HTTPPropertiesConfig.cfg"
#else
#define HTTP_DEFAULT_CONFIG_FILE_PATH     "/data/misc/media/HTTPPropertiesConfig.cfg"
#endif /* WIN32 */

#define HTTP_MAX_CONFIG_STRING_LENGTH    100
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPController;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class tProperty
{
public:
  size_t size;
  int8*  data;

  tProperty() : size(0), data((int8 *)0) { }
  ~tProperty() { }
private:
  tProperty(const tProperty&);
  tProperty& operator=(const tProperty&);
};

class HTTPSourceMMIPropertiesHandler
{
//Constructors/destructor
public:
  explicit HTTPSourceMMIPropertiesHandler(HTTPController* pHTTPController)
           : m_pHTTPController(pHTTPController),
             m_bMediaSampleLoggingAllowed(true){ };
  virtual ~HTTPSourceMMIPropertiesHandler(){ };

//Generic interface for setting and retrieving keyed property data
public:
  virtual bool GetProperty(const int8* pKey, tProperty* pProp);
  virtual bool SetProperty(const int8* pKey, const tProperty* pProp);

//HTTPSourceMMIPropertiesHandler method(s)
public:
  //Maximum meta data buffer size. For HEAP storage option when content length
  //exceeds storage limit, data is downloaded into an over-writeable circular
  //buffer and meta data is stored aside in a separate buffer, whose size is
  //determined by this. Default is 512KB.
  static const HTTPSourceProperty INTERNAL_HTTP_PROPERTY_CIRCBUFF_MAX_METADATA_BUF_SIZE = 0x010737FA;

  static const HTTPSourceProperty INTERNAL_HTTP_PROPERTY_DISABLE_MEDIA_SAMPLE_LOGGING = 0x010737FF;

  bool SetPropertiesFromConfigFile(const char* pConfigFilePath = HTTP_DEFAULT_CONFIG_FILE_PATH);
  bool IsMediaSampleLoggingAllowed() const
  {
    return m_bMediaSampleLoggingAllowed;
  };

private:
  enum PropValueType
  {
    VT_UNKNOWN,
    VT_BOOL,
    VT_UINT32,
    VT_STRING
  };
  struct PropValue
  {
    PropValue() : type(VT_UNKNOWN){ };
    PropValueType type;
    union
    {
      bool v_b;
      uint32 v_u;
      char* v_s;
    };
  };
  struct PropKeyNameIDPair
  {
    char name[HTTP_MAX_CONFIG_STRING_LENGTH];
    uint32 id;
  };
  static const PropKeyNameIDPair m_PropKeyName2IDMap[];
  void SetMediaSampleLoggingAllowed(const bool bMediaSampleLoggingAllowed)
  {
    m_bMediaSampleLoggingAllowed = bMediaSampleLoggingAllowed;
  };
  bool SetPropValue(const PropValue& propValue,
                     tProperty* pProp);
  bool ParseConfigFile(char* pConfigFileInfo);

  HTTPController* m_pHTTPController;
  bool m_bMediaSampleLoggingAllowed;
};

}/* namespace video */

#endif /* __HTTPSOURCEMMIPROPERTIESHANDLER_H__ */
