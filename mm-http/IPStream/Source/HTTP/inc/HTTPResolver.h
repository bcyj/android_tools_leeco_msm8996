#ifndef __HTTPRESOLVER_H__
#define __HTTPRESOLVER_H__
/************************************************************************* */
/**
 * HTTPResolver.h
 * @brief Header file for HTTPResolver.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPResolver.h#10 $
$DateTime: 2013/08/16 11:51:00 $
$Change: 4287091 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPResolver.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPSessionInfo.h"

#include <HTTPStackInterface.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPResolver
{
public:
  HTTPResolver(HTTPSessionInfo& sessionInfo,
               HTTPStackInterface& HTTPStack,
               const int32 nMaxRequestSize);
  virtual ~HTTPResolver();

  HTTPDownloadStatus ResolveHTTPFlavor();

  const char* GetContentType() const
  {
    return (const char*)m_contentType;
  };

  uint32 GetResolverRequestID() const;

private:
  enum State
  {
    IDLE,
    WAIT_FOR_DATA,
    STATE_MAX
  };

  class BaseStateHandler
  {
  protected:
    State m_state;    //HTTP Resolver state

  public:
    BaseStateHandler(){ };
    virtual ~BaseStateHandler(){ };

    virtual HTTPDownloadStatus Execute(HTTPResolver* pHTTPResolver)
    {
      (void)pHTTPResolver;
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };
  };

  class IdleStateHandler : public BaseStateHandler
  {
  public:
    IdleStateHandler(){ m_state = IDLE; };
    virtual ~IdleStateHandler(){ };

    virtual HTTPDownloadStatus Execute(HTTPResolver* pResolver);
  };

  class WaitForDataStateHandler : public BaseStateHandler
  {
  public:
    WaitForDataStateHandler(){ m_state = WAIT_FOR_DATA; };
    virtual ~WaitForDataStateHandler(){ };

    virtual HTTPDownloadStatus Execute(HTTPResolver* pResolver);
  };

  virtual void SetStateHandler(BaseStateHandler* pStatehandler)
  {
    m_pStateHandler = pStatehandler;
  };
  virtual BaseStateHandler* GetStateHandler()
  {
    return m_pStateHandler;
  };

  virtual HTTPDownloadStatus CurrentStateHandler(HTTPResolver* pHTTPResolver);

  void SetContentType(const char* pContentType)
  {
    if (pContentType)
    {
      std_strlcpy(m_contentType, pContentType, sizeof(m_contentType));
    }
  };

  IdleStateHandler m_IdleStateHandler;
  WaitForDataStateHandler m_WaitForDataStateHandler;
  BaseStateHandler* m_pStateHandler;

  HTTPStackInterface& m_pHTTPStack;
  HTTPSessionInfo& m_sessionInfo;
  char* m_pLaunchURL;
  int32 m_nMaxRequestSize;
  uint32 m_nRequestID;
  char m_contentType[HTTP_MAX_CONTENTTYPE_LEN];
};

}/* namespace video */
#endif /* __HTTPRESOLVER_H__ */
