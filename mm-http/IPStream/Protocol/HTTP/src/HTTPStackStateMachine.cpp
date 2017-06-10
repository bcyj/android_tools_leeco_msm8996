/************************************************************************* */
/**
 * HTTPStackStateMachine.cpp
 * @brief implementation of the HTTPStackStateMachine.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackStateMachine.cpp#27 $
$DateTime: 2013/07/27 07:46:50 $
$Change: 4174225 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackHelper.h"
#include "HTTPResponseStatusHandler.h"
#include "HTTPStackStateObjects.h"

#include "SourceMemDebug.h"
#include "qtv_msg.h"

namespace video
{

HTTPStateBase::HTTPStateBase()
{
}

HTTPStateBase::~HTTPStateBase()
{
}

HTTPReturnCode
HTTPStateBase::IsResponseReceived(uint32 /*requestId*/,
                                  HTTPStackHelper& /* HTTPStackHelper */,
                                  void * /* arg */)
{
  return HTTP_FAILURE;
}

HTTPReturnCode
HTTPStateBase::GetData(uint32 /*requestId*/,
                       HTTPStackHelper& /* HTTPStackHelper */,
                       void * /* arg */)
{
  return HTTP_FAILURE;
}

// Protected methods

// HTTPStackHelper methods.

HTTPStateIdle::HTTPStateIdle()
{

}

HTTPStateIdle::~HTTPStateIdle()
{

}

const char *
HTTPStateIdle::GetStateName()
{
  return "HTTP_STATE_IDLE";
}

// Definitions of HTTPStateConnecting

HTTPStateConnecting::HTTPStateConnecting()
{

}

HTTPStateConnecting::~HTTPStateConnecting()
{
}

HTTPReturnCode
HTTPStateConnecting::IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void * /* arg */)
{
  HTTPStateInfo& stateInfo = httpStackHelper.m_HTTPStateInfo;
  HTTPReturnCode result = HTTP_FAILURE;

  if (false == stateInfo.IsConnected())
  {
    result = stateInfo.OpenConnection();
  }

  switch (result)
  {
  case HTTP_SUCCESS:
    // Poll to execute next state
    httpStackHelper.SetState(&HTTPStackStateObjects::HTTPStateConnectedObj);
    result = httpStackHelper.IsResponseReceived(requestId);
    break;

  case HTTP_WAIT:
    // nothing to do
    break;

  default:
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                  "HTTPStateConnecting: Failed to connect to peer. Socket error '%d'",
                  stateInfo.GetLastError());

    httpStackHelper.SetState(&HTTPStackStateObjects::HTTPStateErrorObj);

    if (stateInfo.ShouldRetry(requestId))
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Error in IsResponseReceived while connecting");
      result = HTTP_WAIT;
    }
    break;
  }

  return result;
}

HTTPReturnCode
HTTPStateConnecting::GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg)
{
  GetDataArgument *getDataArg = (GetDataArgument *)arg;
  *(getDataArg->m_NumRead) = 0;

  HTTPReturnCode result = httpStackHelper.IsResponseReceived(requestId);
  result = (result == HTTP_SUCCESS ? HTTP_WAIT : result);

  if (HTTP_WAIT != result)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Error in GetData while connecting");
      httpStackHelper.SetState(&HTTPStackStateObjects::HTTPStateErrorObj);
  }

  return result;
}

const char *
HTTPStateConnecting::GetStateName()
{
  return "HTTP_STATE_CONNECTING";
}

HTTPStateConnected::HTTPStateConnected()
{

}

HTTPStateConnected::~HTTPStateConnected()
{

}

HTTPReturnCode
HTTPStateConnected::IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void * /* arg */)
{
  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStateInfo& stateInfo = httpStackHelper.m_HTTPStateInfo;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                "HTTPStateConnected::IsResponseReceived(%u)",requestId);

  // http request not fully sent yet
  bool bCreateNewConnection = false;
  result = stateInfo.SendPendingRequests(bCreateNewConnection);

  // if the head request that have been queued recently needs stack to connect to new server,
  //  close the current connection and bring up new connection
  if (result != HTTP_SUCCESS && bCreateNewConnection)
  {
    result = httpStackHelper.SendRequestInternal(requestId);
    if(result == HTTP_SUCCESS)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                   "HTTPStateConnected::IsResponseReceived "
                   "Internal Request Sent for New Connection");
      result = HTTP_WAIT;
    }
  }
  else
  {
    //Check if the request for given requestId is sent out completely
    if (true == stateInfo.IsRequestSent(requestId))
    {
      result = stateInfo.ReceiveResponse(requestId);
    }

    if (HTTP_SUCCESS == result)
    {
      // we got here means,
      // 1xx response: fully received
      // 2xx response: at least headers have been received
      // 3xx, 4xx, 5xx: entire response - headers and entity body have been
      //                received.
      result = stateInfo.HandleResponseStatus(requestId,httpStackHelper );
    }
    else
    {
      if (HTTP_WAIT != result)
      {
        // Move to state error if cause is networking related, eg socket error.
        // If retry mechanism is activated, then will recover from ERROR state
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "HTTPStateConnected::IsResponseReceived() Error Occured, "
                     "Moving to state ERROR");
        httpStackHelper.SetState(&HTTPStackStateObjects::HTTPStateErrorObj);
        result = HTTP_WAIT;
      }
    }
  }
  return result;
}

/**
 * @brief:
 *  First call to GetData after HTTP Response headers are completely received
 */
HTTPReturnCode
HTTPStateConnected::GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void *arg)
{
  GetDataArgument *getDataArg = (GetDataArgument *)arg;
  *(getDataArg->m_NumRead) = 0;

  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStateInfo& stateInfo = httpStackHelper.m_HTTPStateInfo;

  // send out any pending requests out that can be pipelined,if there is a need to create new connection
  // as the request needs different hostname, then isResponse received will take care
  // of closing and creating new connection, don't close the connection the middle of reading
  bool bCreateNewConnection = false;
  (void)stateInfo.SendPendingRequests(bCreateNewConnection);

  if (false == stateInfo.IsResponseHeaderFullyReceived(requestId))
  {
    result = httpStackHelper.IsResponseReceived(requestId);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                  "HTTPStateConnected::GetData() Too early to be reading data, response status %d", result);
  }
  else
  {
    result = HTTP_SUCCESS;
  }

  if (result == HTTP_SUCCESS)
  {
    result = stateInfo.GetData(requestId,
                               getDataArg->m_ReadBuf,
                               getDataArg->m_ReadBufSize,
                               *(getDataArg->m_NumRead));

    if ((HTTP_SUCCESS != result) && (HTTP_WAIT != result) && (HTTP_INSUFFBUFFER != result))
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPStateConnected::GetData() Error occured before download complete");

      //Reconnect
      httpStackHelper.SetState(&HTTPStackStateObjects::HTTPStateErrorObj);
      result = HTTP_WAIT;
    }

    QTV_MSG_PRIO3(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                  "HTTPStateConnected::GetData(%u) Client Buf Size '%d', "
                  "NumBytes read to client buffer '%d'",
                  requestId, getDataArg->m_ReadBufSize, *(getDataArg->m_NumRead));
  }

  return result;
}

const char *
HTTPStateConnected::GetStateName()
{
  return "HTTP_STATE_CONNECTED";
}

HTTPStateClosed::HTTPStateClosed()
{

}

HTTPStateClosed::~HTTPStateClosed()
{

}

/**
 * @brief
 *  @param HTTPStackHelper
 *    Connection with peer closed. If rsp headers are recd, that
 *    means the entire response has been downloaded. If some
 *    non-recoverable error would have occured then would have
 *    moved to state error.
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode
HTTPStateClosed::IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void * /* arg */)
{
  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStateInfo& stateInfo = httpStackHelper.m_HTTPStateInfo;

  if (true == stateInfo.IsRequestRspHeaderReceived(requestId))
  {
    result = HTTP_SUCCESS;
  }
  else
  {
    // check if there are pending requests to be sent out
    if(stateInfo.GetNumPendingRequestTobeSent() > 0 )
    {
      result = httpStackHelper.CreateOrReuseConnectionObject();

      if(result != HTTP_SUCCESS)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "Failed in bringing up the connection for pending HTTPRequests");
      }
      else
      {
        result = httpStackHelper.IsResponseReceived(requestId);
      }
    }
  }

  return result;
}

const char *
HTTPStateClosed::GetStateName()
{
  return "HTTP_STATE_CLOSED";
}

HTTPStateError::HTTPStateError()
{

}

HTTPStateError::~HTTPStateError()
{

}

/**
 * @brief
 *  State machine goes into state closed when a socket error
 *  occurs or when connection is terminated by server
 *  prematurely. Retry in such cases.
 *
 * @param httpStackHelper
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode
HTTPStateError::IsResponseReceived(uint32 requestId, HTTPStackHelper& httpStackHelper, void * /*arg */)
{
  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStateInfo& stateInfo = httpStackHelper.m_HTTPStateInfo;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                   "HTTPStateError::IsResponseReceived(%u)",requestId);

  result = stateInfo.HandleReconnect(requestId);

  if(result == HTTP_SUCCESS)
  {
    // This will reset all the Queued Requests to initial state,
    // resets response and response status handler
    stateInfo.Reset();

    // This will move it to state 'CONNECTING'.
    result = httpStackHelper.SendRequestInternal(requestId);

    if (HTTP_SUCCESS != result)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                    "HTTPStateError::IsResponseReceived(%u) "
                    "SendRequestInternal failed",requestId);
    }
    else
    {
      // Poll in CONNECTING state
      result = HTTP_WAIT;
    }
  }

  return result;
}

HTTPReturnCode
HTTPStateError::GetData(uint32 requestId, HTTPStackHelper& httpStackHelper, void * /*arg */)
{
  return httpStackHelper.IsResponseReceived(requestId);
}

const char *
HTTPStateError::GetStateName()
{
  return "HTTP_STATE_ERROR";
}

} // end namespace video
