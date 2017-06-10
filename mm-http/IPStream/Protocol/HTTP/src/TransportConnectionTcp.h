#ifndef _TRANSPORT_CONNECTION_TCP_
#define _TRANSPORT_CONNECTION_TCP_

/************************************************************************* */
/**
 * TransportConnectionTcp.h
 *
 * @brief Defines a TCP based connection for the HTTP protocol stack
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/TransportConnectionTcp.h#13 $
$DateTime: 2013/09/02 23:30:27 $
$Change: 4369104 $

========================================================================== */

#include "TransportConnection.h"

class TransportConnectionTcp : public TransportConnection
{
public:
  TransportConnectionTcp(ResultCode& result,
                         CStreamNetwork *pCStreamNetwork,
                         const char *serverIP,
                         size_t serverIPLen,
                         uint16 serverPort,
                         int socketRcvSizeMax, int socketRcvSize, int socketSndSize);

  virtual ~TransportConnectionTcp();

  virtual ResultCode Open();
  virtual ResultCode SetTcpCloseTimeout (/*in*/ ::int32 lingerTimeoutMs);
  virtual ResultCode Close();
  virtual ResultCode Send(const char *buffer, size_t bufsize, size_t& numWritten);
  virtual ResultCode Recv(char *buffer, size_t bufsize, size_t& numBytesRead);
  virtual bool IsConnected() const;
  virtual ResultCode GetBytesAvailableToRead(int& nBytesAvailableToRead);
  virtual ResultCode GetSockOpt(video::HTTPStackOption optionType, int& value);
  virtual ResultCode SetSockOpt(video::HTTPStackOption optionType, int value);

private:
  TransportConnectionTcp();
  TransportConnectionTcp(const TransportConnectionTcp &);
  TransportConnection& operator=(const TransportConnectionTcp &);

  ResultCode CreateNonBlockingSocket();
  ResultCode TcpConnect();

  enum TcpEstdState
  {
    TCP_ESTD_INITIAL,
    TCP_ESTD_DNS_LOOKUP,
    TCP_ESTD_NET_WAIT,
    TCP_ESTD_POLL_SOCKET,
    TCP_ESTD_POLL_ISOCKPORT,
    TCP_ESTD_COMPLETE,
    TCP_ESTD_SERVER_CLOSED,
    TCP_ESTD_ERROR,
    TCP_ESTD_CLOSED
  };

  TcpEstdState m_TcpEstdState;

  int32 m_LingerTimeoutMs;
};

#endif
