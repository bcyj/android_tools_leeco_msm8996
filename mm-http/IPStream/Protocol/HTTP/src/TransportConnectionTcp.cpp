/************************************************************************* */
/**
 * TransportConnectionTcp.cpp
 *
 * @brief Implements a TCP based connection for the HTTP protocol stack
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/TransportConnectionTcp.cpp#25 $
$DateTime: 2013/09/02 23:30:27 $
$Change: 4369104 $

========================================================================== */

#include "qtv_msg.h"
#include "SourceMemDebug.h"
#include "TransportConnectionTcp.h"
#include "StreamNetwork.h"

static bool CONFIG_USE_ISOCKPORT = false;

/* Calculating the tcp recv buf size for 1mbps clip bandwidth
 * recvbufsize = clipbandwidth(in bps) * delay
 * delay = 0.5 clipbandwith(in bps) = (1*1024 * 1024)/8
 * Therefore recvbufsize= (((1 * 1024 * 1024 )/8)*0.5) = 65536
*/

#define MAX_RECV_BUF_SIZE  65536

/* set liger timeout value to zero, earlier linger timeout value of 1
 * sec was creating an issue at DSS where TCP socket was going in to TCP_TIME_WAIT state
 * when FIN ACK from other end is receivied before the linger timer expires ,forcing TCP socket
 * to be in TCP_TIME_WAIT state for default 30 secs before cleanup happens,
 * in back to back start and stop case,this is causing socket memory pool exhaustion
 * as TCP sockets are not being cleanedup
 */
static const int32 DEFAULT_LINGER_TIMEOUT = 0;

TransportConnectionTcp::TransportConnectionTcp(
  ResultCode& result, CStreamNetwork *pCStreamNetwork,
  const char *serverIP, size_t serverIPLen, uint16 serverPort,
  int socketRcvSizeMax, int socketRcvSize, int socketSndSize) :
    TransportConnection(result, pCStreamNetwork, serverIP, serverIPLen, serverPort,
                        STD_MAX(socketRcvSizeMax, MAX_RECV_BUF_SIZE), socketRcvSize, socketSndSize),
    m_TcpEstdState(TCP_ESTD_INITIAL),
    m_LingerTimeoutMs(DEFAULT_LINGER_TIMEOUT)
{

}

/**
 * d'tor
 */
TransportConnectionTcp::~TransportConnectionTcp()
{

}

/**
 * @brief
 *  Establish tcp connection
 *    1. DNS Lookup
 *    2. Create non-blocking socket
 *    3. Tcp Handshake
 *    4. Poll Isockport if FEATURE_RTPNET_UNICAST_WS is defined
 *    for unicast connection.
 *    5. Connection estd
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::Open()
{
  ResultCode result = EFAILED;

  if (TCP_ESTD_INITIAL == m_TcpEstdState)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "TransportConnectionTcp::Open. Transition from state "
                 "TCP_ESTD_INITIAL to TCP_ESTD_DNS_LOOKUP");

    m_TcpEstdState = TCP_ESTD_DNS_LOOKUP;
  }

  if (TCP_ESTD_DNS_LOOKUP == m_TcpEstdState)
  {
    result = DNSLookup();

    if (SUCCESS == result)
    {
      // Create socket and initiate Tcp connection
      if (SUCCESS != CreateNonBlockingSocket())
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
          "TransportConnectionTcp::Open() Failed to create non-blocking "
          "socket. Transition from state TCP_ESTD_DNS_LOOKUP to TCP_ESTD_ERROR");

        m_TcpEstdState = TCP_ESTD_ERROR;
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                     "TransportConnectionTcp::Open() Transition from state "
                     "TCP_ESTD_DNS_LOOKUP to TCP_ESTD_NET_WAIT");
        m_TcpEstdState = TCP_ESTD_NET_WAIT;
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "TransportConnection::Open() DNSLookup failed");
    }

  } // end TCP_ESTD_DNS_LOOKUP == m_TcpEstdState

  if (TCP_ESTD_NET_WAIT == m_TcpEstdState)
  {
    result = TcpConnect();

    if (SUCCESS == result)
    {
      // this should not be reached as non-blocking socket.
      m_TcpEstdState = TCP_ESTD_COMPLETE;
    }
    else
    {
      if (EWAITING == result)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                     "TransportConnectionTcp::Open() Transition from state "
                     "TCP_ESTD_NET_WAIT to TCP_ESTD_POLL_SOCKET");
        m_TcpEstdState = TCP_ESTD_POLL_SOCKET;
      }
      else if (ENETWAITING == result)
      {
        //no-op
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                     "TransportConnectionTcp::Open() waiting for ppp");
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "TransportConnectionTcp::Open() Transition from state "
                     "TCP_ESTD_NET_WAIT to TCP_ESTD_ERROR");
        m_TcpEstdState = TCP_ESTD_ERROR;
      }
    }
  }

  if (TCP_ESTD_POLL_SOCKET == m_TcpEstdState)
  {
    result = IsControlSocketWritable();

    if (SUCCESS == result)
    {
      // For non unicast sessions the windows socket API's are used
      // instead ISockPort.
      // Windows documents that due to ambiguities in version 1.1 of the
      // Windows Sockets specification, error codes returned from connect
      // while a connection is already pending may vary among implementations.
      // As a result, it is not recommended that applications use multiple
      // calls to connect to detect connection completion.
      if (true == CONFIG_USE_ISOCKPORT)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                     "TransportConnection::Open() Transition from state "
                     "TCP_ESTD_POLL_SOCKET to TCP_ESTD_POLL_ISOCKPORT");

        // Do a connect again for multicast socket
        m_TcpEstdState = TCP_ESTD_POLL_ISOCKPORT;
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                     "TransportConnection::Open() Transition from state "
                     "TCP_ESTD_POLL_SOCKET to TCP_ESTD_COMPLETE");

        // Connect went through for unicast sockets
        m_TcpEstdState = TCP_ESTD_COMPLETE;
      }
    }
  } // TCP_ESTD_POLL_SOCKET == m_TcpEstdState

  if (TCP_ESTD_POLL_ISOCKPORT == m_TcpEstdState)
  {
    result = TcpConnect();

    if (SUCCESS == result)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                   "TransportConnection::Open() Transition from state "
                   "TCP_ESTD_POLL_ISOCKPORT to TCP_ESTD_COMPLETE");

      m_TcpEstdState = TCP_ESTD_COMPLETE;
    }
    else
    {
      if (EWAITING == result)
      {
        // Nothing to do
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "TransportConnection::Open() Transition from state "
                     "TCP_ESTD_POLL_ISOCKPORT to TCP_ESTD_ERROR");

        m_TcpEstdState = TCP_ESTD_ERROR;
      }
    }
  }

  if (TCP_ESTD_COMPLETE == m_TcpEstdState)
  {
    result = SUCCESS;

    if (m_SocketRcvSize > 0)
    {
      int r = SetSockOpt(video::HTTP_SOCKET_RECV_BUF_SIZE, m_SocketRcvSize);
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "TransportConnectionTcp: Set rcvBufSize to %d after connect. retval %d",
        m_SocketRcvSize, r);

    }
  }
  else
  {
    if (TCP_ESTD_ERROR == m_TcpEstdState)
    {
      result = EFAILED;
    }
  }


  return result;
}

TransportConnection::ResultCode
TransportConnectionTcp::SetTcpCloseTimeout (/*in*/ ::int32 lingerTimeoutMs)
{
  m_LingerTimeoutMs = lingerTimeoutMs;
  return SUCCESS;
}

/**
 * @brief
 *  Close connection.
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::Close()
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);

  ResultCode result = SUCCESS;

  if (SOCKET_UNINITIALIZED == m_Sockfd)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "TransportConnectionTcp::Close() Connection not open");
  }
  else
  {
    so_linger_type so_linger = {0, 0};
    so_linger.l_onoff = 1;
    so_linger.l_linger = DEFAULT_LINGER_TIMEOUT;

    if ((m_LingerTimeoutMs >= 0) && (m_LingerTimeoutMs <= 0xffff))
    {
      so_linger.l_linger = (uint16)m_LingerTimeoutMs;
    }

    if (0 != m_pCStreamNetwork->setsockopt(m_Sockfd,
                                           SOL_SOCK,
                                           SO_LINGER,
                                           (char *) &so_linger,
                                           sizeof(so_linger)))
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "TransportConnectionTcp::Close() Failed to set socket option SO_LINGER");

      result = EFAILED;
    }

    if(0 != m_pCStreamNetwork->close(m_Sockfd))
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "TransportConnectionTcp::Close: Socket Error '%d'",
                     m_pCStreamNetwork->get_last_error());

      result = EFAILED;
    }

    if (SUCCESS == result)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                   "TransportConnectionTcp::Close() Connection closed");
    }
  }

  m_TcpEstdState = TCP_ESTD_CLOSED;

  return result;
}

/**
 * @brief
 *  Write data to socket
 *
 * @param buffer
 * @param bufsize
 * @param numWritten
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::Send(const char *buffer, size_t bufsize, size_t& numWritten)
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);

  ResultCode result = EFAILED;

  if (false == IsConnected())
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "TransportConnectionTcp::Send() failed. Reason: Not connected");
  }
  else
  {
    numWritten = 0;

    int numSent = m_pCStreamNetwork->write(m_Sockfd, buffer, bufsize);

    if (numSent > 0)
    {
      numWritten = numSent;
      result = SUCCESS;
    }
    else if ((0 == numSent) ||
             (-1 == numSent && EWOULDBLOCK == m_pCStreamNetwork->get_last_error()) ||
             (EINTR == m_pCStreamNetwork->get_last_error()))
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

/**
 * @brief
 *  Receive data from socket
 *
 * @param buffer
 * @param bufsize
 * @param numBytesRead
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::Recv(char *buffer, size_t bufsize, size_t& numBytesRead)
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);
  ResultCode result = EFAILED;

  if (false == IsConnected())
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "TransportConnectionTcp::Recv Failed. Reason: Not connected");
  }
  else
  {
    numBytesRead = 0;

    int numRead = m_pCStreamNetwork->read(m_Sockfd, buffer, bufsize);

    if (numRead > 0)
    {
      numBytesRead = numRead;
      result = SUCCESS;
    }
    else
    {
      if (0 == numRead)
      {
        result = ECLOSEDBYPEER;
      }
      else
      {
        if ((EWOULDBLOCK == m_pCStreamNetwork->get_last_error()) ||
            (EINTR == m_pCStreamNetwork->get_last_error()))
        {
          result = EWAITING;
        }
        else
        {
          result = EFAILED;
        }
      }
    }
  }

  return result;
}

bool
TransportConnectionTcp::IsConnected() const
{
  return (TCP_ESTD_COMPLETE == m_TcpEstdState ? true : false);
}

/**
 * @brief
 *  Create non-blocking tcp socket
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::CreateNonBlockingSocket()
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);

  ResultCode result = EFAILED;

  m_Sockfd = m_pCStreamNetwork->socket( AF_INET /* family */,
                                        SOCK_STREAM /* type */,
                                        0 /* protocol */);

  if (m_Sockfd < 0)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "TransportConnectionTcp::CreateNonBlockingSocket() failed to create TCP socket");

    m_Sockfd = SOCKET_UNINITIALIZED;
  }
  else
  {
    unsigned long ioctl_arg = 1;
    if (m_pCStreamNetwork->ioctl(m_Sockfd,
                                 FIONBIO,
                                 (void *)&ioctl_arg))
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "CreateNonBlockingSocket() Failed");

      m_pCStreamNetwork->close(m_Sockfd);
      m_Sockfd = SOCKET_UNINITIALIZED;
    }
    else
    {
      //Set socket recv buffer to atleast MAX_RECV_BUF_SIZE bytes (this is
      //needed as otherwise on Win32 default seems to be 8K much smaller than
      //actual TCP window size - setting SO_RCVBUF seems to be on the kernel
      //socket driver level and does not directly alter the TCP window size!)
      size_t recvBufSize = 0;
      size_t len = sizeof(recvBufSize);
      if ( m_pCStreamNetwork->getsockopt( m_Sockfd,
                                          SOL_SOCK,
                                          SO_RCVBUF,
                                          (void *)&recvBufSize,
                                          &len ) )
      {
        recvBufSize = 0;
        QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                      "getsockopt() SO_RCVBUF Failed" );
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "TransportConnectionTcp: Default sockRecvSize %d", recvBufSize);
      }


      if (m_SocketRcvSizeMax > 0 && (int)recvBufSize < m_SocketRcvSizeMax)
      {
        recvBufSize = m_SocketRcvSizeMax;
        if ( m_pCStreamNetwork->setsockopt( m_Sockfd,
                                            SOL_SOCK,
                                            SO_RCVBUF,
                                            (char *)&m_SocketRcvSizeMax,
                                            sizeof(m_SocketRcvSizeMax) ) != 0 )
        {
          QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Set Recv Buf Size failed size %d error %d",
                         recvBufSize, m_pCStreamNetwork->get_last_error() );
          recvBufSize = 0;
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "TransportConnectionTcp: Set sockRcvSize to %d before connect",
                        m_SocketRcvSizeMax);
        }
      }

      QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_LOW,
                     "Socket %d created with RCVBUF %d",
                     m_Sockfd, recvBufSize );


      if (m_SocketSndSize >= 0)
      {
        SetSockOpt(video::HTTP_SOCKET_SEND_BUF_SIZE, m_SocketSndSize);

        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "CreateNonBlockingSocket: set SO_SNDBUF with size %d, on socket %d, rslt %d",
                      m_SocketSndSize, m_Sockfd, result);
      }

      result = SUCCESS;
    }
  }

  return result;
}

/**
 * @brief
 *  Initiate 3 way tcp-handshake
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::TcpConnect()
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);

  ResultCode result = EFAILED;

  union
  {
    sockaddr_in *in;
    struct sockaddr *out;
  }u;

  u.in = &m_Server;
  struct sockaddr *pServer = u.out;
  int errorCode = m_pCStreamNetwork->connect( m_Sockfd,
                                              pServer,
                                              (socklen_t)sizeof(m_Server) );

  int lastError = m_pCStreamNetwork->get_last_error();

  if ((errorCode == 0) || (lastError == EISCONN))
  {
    result = SUCCESS;
  }
  else if (lastError == ENETINPROGRESS)
  {
    result = ENETWAITING;
  }
  else
  {
    if (lastError == EWOULDBLOCK ||
        lastError == EINPROGRESS ||
        lastError == EBUSY       ||
        lastError == ENETWOULDBLOCK)
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

/**
 * @brief
 *  Get number of bytes available to read
 *
 * @param nBytesAvailableToRead
 *
 * @return ResultCode
 */
TransportConnection::ResultCode
TransportConnectionTcp::GetBytesAvailableToRead(int& nBytesAvailableToRead)
{
  QTV_NULL_PTR_CHECK(m_pCStreamNetwork, EFAILED);
  ResultCode result = EWAITING;
  nBytesAvailableToRead = 0;

  if (IsConnected())
  {
    result = SUCCESS;
    if (m_pCStreamNetwork->ioctl(m_Sockfd,
                                 FIONREAD,
                                 (void *)&nBytesAvailableToRead))
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "ioctl() FIONREAD Failed %d",
                     m_pCStreamNetwork->get_last_error() );
      result = EFAILED;
    }
  }

  return result;
}

/**
 * @brief
 *  GetSock option value for socket.
 *
 * @return TransportConnection::ResultCode
 *  SUCCESS - value returned from OS is success.
 *  EWAITING - socket not yet created
 *  FAILURE - Invalid option or OS returned failure.
 */
TransportConnection::ResultCode TransportConnectionTcp::GetSockOpt(
   video::HTTPStackOption optionType, int& value)
{
  TransportConnection::ResultCode rslt = EFAILED;

  int optname = -1;

  switch (optionType)
  {
  case video::HTTP_SOCKET_RECV_BUF_SIZE:
    optname = SO_RCVBUF;
    rslt = SUCCESS;
    break;

  case video::HTTP_SOCKET_SEND_BUF_SIZE:
    optname = SO_SNDBUF;
    rslt = SUCCESS;
    break;

  default:
    break;
  }

  if (SUCCESS == rslt)
  {
    if (SOCKET_UNINITIALIZED != m_Sockfd)
    {
      size_t s = sizeof(value);
      int r = m_pCStreamNetwork->getsockopt(m_Sockfd, SOL_SOCK, optname, &value, &s);
      rslt = (0 == r ? SUCCESS : EFAILED);
    }
    else
    {
      rslt = EWAITING;
    }
  }

  return rslt;
}

/**
 * @brief
 *  SetSock option value for socket.
 *
 * @return TransportConnection::ResultCode
 *  SUCCESS - value returned from OS is success.
 *  EWAITING - socket not yet created. Option will be set
 *  later FAILURE - Invalid option or OS returned failure.
 */
TransportConnection::ResultCode TransportConnectionTcp::SetSockOpt(video::HTTPStackOption optionType, int value)
{
  TransportConnection::ResultCode rslt = EFAILED;

  int optname = -1;

  switch (optionType)
  {
  case video::HTTP_SOCKET_RECV_BUF_SIZE:
    optname = SO_RCVBUF;
    m_SocketRcvSize = value;
    rslt = SUCCESS;

    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "SetSockOpt: SO_RCVBUF with val %d, on socket %d",
                  m_SocketRcvSize, m_Sockfd);
    break;
  case video::HTTP_SOCKET_SEND_BUF_SIZE:
    optname = SO_SNDBUF;
    m_SocketSndSize = value;
    rslt = SUCCESS;

    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "SetSockOpt: SO_SNDBUF with val %d, on socket %d",
                  m_SocketSndSize, m_Sockfd);
    break;
  default:
    break;
  }

  if (SUCCESS == rslt)
  {
    if (SOCKET_UNINITIALIZED != m_Sockfd)
    {
      int r = m_pCStreamNetwork->setsockopt(m_Sockfd, SOL_SOCK, optname, &value, sizeof(value));
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "setsockopt on platform return code %d", r);
      rslt = (0 == r ? SUCCESS : EFAILED);
    }
    else
    {
      rslt = EWAITING;
    }
  }

  return rslt;
}
