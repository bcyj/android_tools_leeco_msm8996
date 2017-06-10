#include "TransportConnection.h"
#include "SourceMemDebug.h"
#include "AEEstd.h"

/************************************************************************* */
/**
 * TransportConnection.cpp
 *
 * @brief Implements a transport connection interface for the HTTP protocol stack
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/TransportConnection.cpp#27 $
$DateTime: 2013/09/02 23:30:27 $
$Change: 4369104 $

========================================================================== */

#include "qtv_msg.h"

TransportConnection::TransportConnection(
  ResultCode& result, CStreamNetwork *pCStreamNetwork,
  const char *serverIP, size_t serverIPLen, uint16 serverPort,
  int socketRcvSizeMax, int socketRcvSize, int socketSndSize)
{
  m_SocketRcvSizeMax = STD_MAX(socketRcvSizeMax, socketRcvSize);
  m_SocketRcvSize = socketRcvSize;
  m_SocketSndSize = socketSndSize;

  result = Init();

  // Allocate m_ServerIP
  if (SUCCESS == result)
  {
    m_pCStreamNetwork = pCStreamNetwork;
    size_t serverIPBufSize = serverIPLen + 1;
    m_DNSLookupStruct.m_ServerIP = (char *)QTV_Malloc(
      serverIPBufSize * sizeof(char));

    if (NULL == m_DNSLookupStruct.m_ServerIP)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                    "Failed to alloc '%d' bytes for m_DNSLookupStruct.m_ServerIP",
                    serverIPBufSize);

      result = ENOMEMORY;
    }
    else
    {
      std_strlcpy(m_DNSLookupStruct.m_ServerIP, serverIP, serverIPBufSize);
      m_DNSLookupStruct.m_ControlPort = serverPort;

      m_Server.sin_family = AF_INET;
      m_Server.sin_port = htons(m_DNSLookupStruct.m_ControlPort);
      memset(m_Server.sin_zero, 0, sizeof(m_Server.sin_zero));
    }

    memset(m_pIPAddress, 0, sizeof(m_pIPAddress));
  }

  if (NULL == m_pCStreamNetwork)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "TransportConnection m_pCStreamNetwork is NULL");

    result = EFAILED;
  }
}

/**
 * d'tor
 */
TransportConnection::~TransportConnection()
{
  if (NULL != m_DNSLookupStruct.m_ServerIP)
  {
    QTV_Free(m_DNSLookupStruct.m_ServerIP);
    m_DNSLookupStruct.m_ServerIP = NULL;
  }
}

TransportConnection::ResultCode
TransportConnection::Init()
{
  ResultCode result = SUCCESS;

  m_Sockfd = SOCKET_UNINITIALIZED;
  m_pCStreamNetwork = NULL;

  m_DNSLookupStruct.m_ControlPort = UNINITIALIZED_PORT_VAL;
  m_DNSLookupStruct.m_DNSLookupState = DNSLookupStruct::DNS_LOOKUP_INITIAL;
  memset(&m_Server, 0, sizeof(struct sockaddr_in));

  return result;
}

/**
 * @brief
 *  Sets the netpolicy on streamNetwork.
 *
 * @param net_policy
 *
 * @return bool
 *  true on success, false on failure.
 */
bool TransportConnection::SetNetPolicy(NetPolicyInfo* net_policy)
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, false);
  return (0 == m_pCStreamNetwork->set_net_policy(net_policy)
          ? true : false);
}

/**
 * @brief
 *  Gets the netpolicy from streamNetwork.
 *
 * @param net_policy
 *
 * @return bool
 *  true on success, false on failure.
 */
bool TransportConnection::GetNetPolicy(NetPolicyInfo* net_policy)
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, false);
  return (0 == m_pCStreamNetwork->get_net_policy(net_policy)
          ? true : false);
}

const char*
TransportConnection::GetHostName() const
{
  return m_DNSLookupStruct.m_ServerIP;
}

unsigned int
TransportConnection::GetPort() const
{
  return m_DNSLookupStruct.m_ControlPort;
}

/**
 * @brief
 *  Create IPAddress and populate it. Return same to caller
 *
 * @param size
 *
 * @return  Pointer to created ipaddress.
 */
char*
TransportConnection::GetIPAddr(int& size) const
{
  // Signature needs  to change return type to const char *. That can be
  // done when HTTPStackInterface.h for this API is changed.
  std_strlprintf((char *)m_pIPAddress,
                 sizeof(m_pIPAddress),
                 "%d.%d.%d.%d",
                 m_Server.sin_addr.s_addr&0xFF,(m_Server.sin_addr.s_addr&0xFF00)>>8,(m_Server.sin_addr.s_addr&0xFF0000)>>16,(m_Server.sin_addr.s_addr&0xFF000000)>>24);
  size = (int)std_strlen(m_pIPAddress);
  return (char *)m_pIPAddress;
}

int
TransportConnection::GetLastError()
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, -1);
  return m_pCStreamNetwork->get_last_error();
}
/**
 * @brief
 *  Function maintains 3 states:
 *  DNS_LOOKUP_INITIAL:     Extracts hostname and port.
 *  DNS_LOOKUP_IN_PROGRESS: Calls DNS Resolving API
 *  DNS_LOOKUP_COMPLETE:    DNS Ressolinv complete.
 *  DNS_LOOKUP_ERROR:       Non recoverable error
 *
 * @return ResultCode
 *  SUCCESS   DNS Lookup successfully complete
 *  EWAITING  Waiting to complete DNS lookup
 *  EFAILED   Failed
 */
TransportConnection::ResultCode
TransportConnection::DNSLookup()
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);
  ResultCode result = EFAILED;

  if (DNSLookupStruct::DNS_LOOKUP_INITIAL ==
      m_DNSLookupStruct.m_DNSLookupState)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "DNSLookup: Starting DNS lookup...");

    if (NULL != m_DNSLookupStruct.m_ServerIP)
    {
      m_DNSLookupStruct.m_DNSLookupState =
        DNSLookupStruct::DNS_LOOKUP_IN_PROGRESS;
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "DNSLookup: in progress...");
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "DNSLookup:  Failed. NULL server IP");

      m_DNSLookupStruct.m_DNSLookupState =
        DNSLookupStruct::DNS_LOOKUP_ERROR;
    }

  } // DNS_LOOKUP_INITIAL == m_DNSLookupState

  if (DNSLookupStruct::DNS_LOOKUP_IN_PROGRESS ==
      m_DNSLookupStruct.m_DNSLookupState)
  {
    if (NULL == m_DNSLookupStruct.m_ServerIP)
    {
      // Should never happen
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "DNSLookup: Failed. NULL server IP");
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW,
        "DNSLookup: before inet_addr: %s", m_DNSLookupStruct.m_ServerIP);
      if ((uint32)inet_addr(m_DNSLookupStruct.m_ServerIP) == INADDR_NONE)
      {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW,
                   "DNSLookup: before gethostbyname");
        hostent *hp = m_pCStreamNetwork->gethostbyname(m_DNSLookupStruct.m_ServerIP);
        if (hp == NULL)
        {
          int lasterr = GetLastError();
          if (lasterr == ETRYAGAIN)
          {
            result = EWAITING;
          }
          else
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                          "DNSLookup: Gethostbyname failed. "
                          "Socket Error '%d'", lasterr);

            m_DNSLookupStruct.m_DNSLookupState = DNSLookupStruct::DNS_LOOKUP_ERROR;
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                       "DNSLookup: Complete. Name resolved");

          memcpy(&m_Server.sin_addr.s_addr,
                 hp->h_addr,
                 sizeof(m_Server.sin_addr.s_addr));

          m_DNSLookupStruct.m_DNSLookupState =
            DNSLookupStruct::DNS_LOOKUP_COMPLETE;
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                     "DNSLookup: Complete. Did not need name resolving");

        m_Server.sin_addr.s_addr = inet_addr(m_DNSLookupStruct.m_ServerIP);
        m_DNSLookupStruct.m_DNSLookupState = DNSLookupStruct::DNS_LOOKUP_COMPLETE;
      }
    }
  } // DNS_LOOKUP_IN_PROGRESS == m_DNSLookupState

  if (DNSLookupStruct::DNS_LOOKUP_COMPLETE ==
      m_DNSLookupStruct.m_DNSLookupState)
  {
    result = SUCCESS;
  }
  else
  {
    if (DNSLookupStruct::DNS_LOOKUP_ERROR ==
        m_DNSLookupStruct.m_DNSLookupState)
    {
      result = EFAILED;
    }
  }

  return result;
}

/**
 * @brief
 *  Poll to check if socket is writable
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnection::IsControlSocketWritable()
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);

  ResultCode result = EFAILED;

  fd_set socket_set;
  struct timeval timeout;

  // test connection status
  FD_ZERO(&socket_set);
  if (m_Sockfd == SOCKET_UNINITIALIZED)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR, "m_Sockfd is SOCKET_UNINITIALIZED");
  }
  else
  {
    FD_SET(m_Sockfd, &socket_set);
  }
  timeout.tv_sec = 0;
  timeout.tv_usec = SOCKET_WATCH_DURATION;

  int errorCode = m_pCStreamNetwork->select(
    NULL, &socket_set, NULL, &timeout);

  if (errorCode == 1)
  {
    if (m_Sockfd != SOCKET_UNINITIALIZED && FD_ISSET(m_Sockfd, &socket_set))
    {
      result = SUCCESS;

#ifdef DSS_VERSION
      errorCode = m_pCStreamNetwork->connect( m_Sockfd,
                                            (struct sockaddr *) &m_Server,
                                            sizeof(m_Server) );

      int lastError = m_pCStreamNetwork->get_last_error();

      if ((errorCode == 0) || (lastError == EISCONN))
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH, "tcp established");
        result = SUCCESS;
      }
      else
      {
        if (lastError == EWOULDBLOCK ||
            lastError == EINPROGRESS ||
            lastError == ENETWOULDBLOCK)
        {
          result = EWAITING;
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR, "tcp establish failed");
          result = EFAILED;
        }
      }
#endif
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR, "m_Sockfd is SOCKET_UNINITIALIZED");
    }
  }
  else
  {
    if (0 == errorCode)
    {
      result = EWAITING;
    }
    else
    {
      result = EFAILED;
    }
  }

  return result;
}
