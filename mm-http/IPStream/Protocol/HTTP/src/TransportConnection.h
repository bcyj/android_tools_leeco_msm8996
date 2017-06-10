#ifndef _TRANSPORT_CONNECTION_H_
#define _TRANSPORT_CONNECTION_H_

/************************************************************************* */
/**
 * TransportConnection.h
 *
 * @brief Defines a connection interface for the HTTP Protocl Stack
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/TransportConnection.h#18 $
$DateTime: 2013/09/02 23:30:27 $
$Change: 4369104 $

========================================================================== */

#include "HTTPStackInterface.h"
#include "StreamNetwork.h"

const uint16 UNINITIALIZED_PORT_VAL = 0;
const int32 SOCKET_UNINITIALIZED = -1;
const int SOCKET_WATCH_DURATION = 5 * 1000; //millisecs

class TransportConnection
{
public:
  enum ResultCode
  {
    SUCCESS,
    EFAILED,
    EWAITING,
    ENETWAITING,
    ENOMEMORY,
    ECLOSEDBYPEER,
    ESOCKERROR      // errno will provide appropraite error code.
  };

  TransportConnection(ResultCode& result, CStreamNetwork *pCStreamNetwork,
                      const char *serverIP, size_t serverIPLen, uint16 serverPort,
                      int sockRcvSizeMax, int sockRcvSize, int sockSndSize);

  virtual ~TransportConnection();

  /**
   * @brief
   *  Open connection with peer.
   *
   * @return ResultCode
   *  SUCCESS   Open succeeded.
   *  EWAITING  Waiting to complete open
   *  EFAILED   Failed.
   */
  virtual ResultCode Open() = 0;

  /**
   * @brief
   *    Set the linger timeout in Millisec on a TCP close. This
   *    should be valid only for TCP connection.
   *
   * @param lingerTimeoutMs Use value of zero to send RST on TCP
   *                        close.
   *
   * @return ResultCode
   */
  virtual ResultCode SetTcpCloseTimeout (/*in*/ ::int32 lingerTimeoutMs) = 0;

  /**
   * @brief
   *  Close TCP connection
   *
   * @return ResultCode
   */
  virtual ResultCode Close() = 0;

  /**
   * @brief
   *  Write data to socket.
   *
   * @param buffer
   * @param bufsize
   * @param numWritten
   *
   * @return ResultCode
   *  SUCCESS   Wrote bufsize bytes successfully to socket
   *  EWAITING  Wrote zero bytes to socket.
   *  EFAILED   Socket Error. Need to look at errno for more info.
   */
  virtual ResultCode Send(const char *buffer, size_t bufsize, size_t& numWritten) = 0;

  /**
   * @brief
   *  Receive data from socket
   *
   * @param buffer
   * @param bufsize
   * @param numBytesRead
   *
   * @return ResultCode
   *  SUCCESS   Successfully read bufsize bytes from socket
   *  EWAITING  Waiting for data on socket.
   *  EFAILED   Socket error. Use get_next_error() for more info.
   */
  virtual ResultCode Recv(char *buffer, size_t bufsize, size_t &numBytesRead) = 0;

  /**
   * @brief
   *  Check if socket is connected to peer.
   *
   * @return bool
   */
  virtual bool IsConnected() const = 0;

  virtual const char*   GetHostName() const;
  virtual unsigned int  GetPort()     const;
  virtual char*         GetIPAddr(int& size) const;

  virtual int GetLastError();

  /**
   * @brief
   *  Sets the netpolicy on streamNetwork.
   *
   * @param net_policy
   *
   * @return bool
   *  true on success, false on failure.
   */
  bool SetNetPolicy(NetPolicyInfo* net_policy);

  /**
   * @brief
   *  Gets the netpolicy from streamNetwork.
   *
   * @param net_policy
   *
   * @return bool
   *  true on success, false on failure.
   */
  bool GetNetPolicy(NetPolicyInfo* net_policy);

  /**
   * @brief
   *  Get number of bytes available to read
   *
   * @param nBytesAvailableToRead
   *
   * @return ResultCode
   *  SUCCESS   Successfully computed number of bytes available to read
   *  EWAITING  Waiting for data on socket
   *  EFAILED   Otherwise (socket error etc.)
   */
  virtual ResultCode GetBytesAvailableToRead(int& nBytesAvailableToRead) = 0;

  virtual ResultCode GetSockOpt(video::HTTPStackOption optionType, int& value) = 0;
  virtual ResultCode SetSockOpt(video::HTTPStackOption optionType, int value) = 0;

protected:
  ResultCode IsControlSocketWritable();

  typedef struct DNSLookupStruct
  {
    enum DNSLookupState
    {
      DNS_LOOKUP_INITIAL,     // DNS request not yet sent
      DNS_LOOKUP_IN_PROGRESS, // DNS request sent. Waiting for completion
      DNS_LOOKUP_COMPLETE,    // DNS reply received
      DNS_LOOKUP_ERROR        // Error
    };

    DNSLookupState  m_DNSLookupState;
    uint16          m_ControlPort;
    char*           m_ServerIP;

  } DNSLookupStructType;

  DNSLookupStructType m_DNSLookupStruct;

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
  ResultCode DNSLookup();

  struct sockaddr_in m_Server;
  int m_Sockfd;

  // The rcvbuf value that will be set on the OS before calling connect.
  int m_SocketRcvSizeMax;

  int m_SocketRcvSize;
  int m_SocketSndSize;

  static const int MAX_IPADRESS_SIZE  = 20;
  char m_pIPAddress[MAX_IPADRESS_SIZE];
  CStreamNetwork *m_pCStreamNetwork;

private:
  TransportConnection();
  TransportConnection(const TransportConnection &);
  TransportConnection& operator=(const TransportConnection &);

  ResultCode Init();
};

#endif
