#ifndef HTTPSTACKSTATEMACHINE_H
#define HTTPSTACKSTATEMACHINE_H
/************************************************************************* */
/**
 * HTTPStackStateMachine.h
 * @brief Implements the HTTPStackStateMachine
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackStateMachine.h#10 $
$DateTime: 2013/07/27 08:03:59 $
$Change: 4174247 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

struct CRTPNetworkBase;

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackCommon.h"

namespace video {

class HTTPStackHelper;

class HTTPStateBase
{
public:
  HTTPStateBase();
  virtual ~HTTPStateBase() = 0;

  virtual HTTPReturnCode IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);
  virtual HTTPReturnCode GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);

  virtual const char *GetStateName() = 0;

protected:

  friend class HTTPStackHelper;
};

class HTTPStateIdle : public HTTPStateBase
{
public:
  HTTPStateIdle();
  virtual ~HTTPStateIdle();

  virtual const char *GetStateName();
};

class HTTPStateConnecting : public HTTPStateBase
{
public:
  HTTPStateConnecting();
  virtual ~HTTPStateConnecting();
  virtual HTTPReturnCode IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);
  virtual HTTPReturnCode GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);

  virtual const char *GetStateName();
};

class HTTPStateConnected : public HTTPStateBase
{
public:
  HTTPStateConnected();
  virtual ~HTTPStateConnected();
  virtual HTTPReturnCode IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);
  virtual HTTPReturnCode GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);

  virtual const char *GetStateName();
};

class HTTPStateClosed : public HTTPStateBase
{
public:
  HTTPStateClosed();
  virtual ~HTTPStateClosed();
  virtual HTTPReturnCode IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);

  virtual const char *GetStateName();
};

class HTTPStateError : public HTTPStateBase
{
public:
  HTTPStateError();
  virtual ~HTTPStateError();

  virtual HTTPReturnCode IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);
  virtual HTTPReturnCode GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg);

  virtual const char *GetStateName();
};

// Arguments for state transition

struct GetDataArgument
{
  char *m_ReadBuf;
  size_t m_ReadBufSize;
  size_t *m_NumRead;

  GetDataArgument(char *readBuf, size_t readBufSize, size_t *numRead) :
    m_ReadBuf(readBuf), m_ReadBufSize(readBufSize), m_NumRead(numRead)
  {
  }
};

struct GetContentTypeArgument
{
  char *m_ContentType;
  size_t m_ContentTypeLen;
  size_t *m_ContentTypeLenReq;

  GetContentTypeArgument( char *contentType,
                          size_t contentTypeLen,
                          size_t *contentTypeLenReq):
    m_ContentType(contentType),
    m_ContentTypeLen(contentTypeLen),
    m_ContentTypeLenReq(contentTypeLenReq)
  {
  }
};

} // end namespace video

#endif /* HTTPSTACKSTATEMACHINE_H */
