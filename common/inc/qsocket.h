#ifndef _QSOCKET_H_
#define _QSOCKET_H_
/******************************************************************************
  @file    qsocket.h
  @brief   Generic Socket-like interface

  DESCRIPTION

  Provides a wrapper interface over standard socket interface used for
  IPC Router.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

 *******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "stdlib.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
                            SOCKET TYPES
============================================================================*/

/* Datagram data socket */
#define QSOCK_DGRAM SOCK_DGRAM
/* Stream-like socket */
#define QSOCK_STREAM SOCK_STREAM

/*============================================================================
                          SOCKET SEND/RECV FLAGS
============================================================================*/

/* Refrain from reading the next available packet but return the next
 * packet size instead. */
#define QMSG_PEEK MSG_PEEK

/* Send will block on flow control, recv will block if there
 * are no packets to receive */
#define QMSG_DONTWAIT MSG_DONTWAIT

/* Truncate the message if the buffer it too long and return
 * the full length of the buffer */
#define QMSG_TRUNC MSG_TRUNC

/*============================================================================
                          SOCKET OPTION DEFINES
============================================================================*/
/* Generic non-protocol specific socket options */
#define QSOL_SOCKET SOL_SOCKET

/*============================================================================
                          SOCKET OPTION DEFINES
  Generic non-protocol specific options valid with LEVEL = QSOL_SOCKET
============================================================================*/

/* Set the read timeout. Option value is of type 'int' containing
 * the timeout for reads in milli-seconds.
 * 0 - Non-blocking writes by default.
 * -1 - Block for infinity
 * x>0 - block for x milli-seconds */
#define QSO_RCVTIMEO SO_RCVTIMEO

/* Set the write timeout. Option value is of type 'int' containing
 * the timeout for writes in milli-seconds.
 * 0 - Non-blocking writes by default.
 * -1 - Block for infinity
 * x>0 - block for x milli-seconds */
#define QSO_SNDTIMEO SO_SNDTIMEO

/*============================================================================
                            SHUTDOWN TYPES
============================================================================*/
/* Close RX path for the socket */
#define QSHUT_RD SHUT_RD

/* Close TX for the socket */
#define QSHUT_WR SHUT_WR

/* Close both RX and TX of the socket */
#define QSHUT_RDWR SHUT_RDWR

/*============================================================================
                            POLL EVENT TYPES
============================================================================*/

/* Input event */
#define QPOLLIN POLLIN

/* Output event -- Unused, provided for completeness */
#define QPOLLOUT POLLOUT

/* Remove side has hungup on this port */
#define QPOLLHUP POLLHUP

/* Invalid parameters/arguments/operations */
#define QPOLLNVAL POLLNVAL

/*============================================================================
                                ERROR CODES
============================================================================*/
/* Bad socket descriptor */
#define QEBADF EBADF
/* The connection is reset */
#define QECONNRESET ECONNRESET
/* This is not a connected socket and a destination is required */
#define QEDESTADDRREQ EDESTADDRREQ
/* Generic access fault */
#define QEFAULT EFAULT
/* Invalid parameters */
#define QEINVAL EINVAL
/* Is a connected socket but an addr was provided */
#define QEISCONN EISCONN
/* message size too large to be transmitted as one chunk */
#define QEMSGSIZE EMSGSIZE
/* No TX queue buffer remaining */
#define QENOBUFS ENOBUFS
/* Memory allocation failed */
#define QENOMEM ENOMEM
/* Socket is not connected and no target was provided */
#define QENOTCONN ENOTCONN
/* This operation or flags are not supported */
#define QENOTSUPP ENOTSUPP
/* Transmission would require the call to block */
#define QEAGAIN EAGAIN
/* Operation would block but the user expected it to not block */
#define QEWOULDBLOCK EWOULDBLOCK
/* The recv was refused as the buffer size was insufficient */
#define QEWOULDTRUNC EWOULDTRUNC
/* AF is not supported  */
#define QEAFNOTSUPP EAFNOTSUPP
/* Operation not supported */
#define QEOPNOTSUPP EOPNOTSUPP

/*============================================================================
                             TYPES
============================================================================*/

#define qsockaddr sockaddr
#define qpollfd pollfd
#define qsocklen_t socklen_t
typedef size_t qnfds_t;

/*============================================================================
                        FUNCTION PROTOTYPES
============================================================================*/

/*===========================================================================
  FUNCTION  qsocket
===========================================================================*/
/*!
@brief

  This function returns a socket descriptor

@param[in]   domain    The address family of the socket (Currently only
                       AF_IPC_ROUTER is supported)
@param[in]   type      Type of socket. See SOCKET TYPES for more info
@param[in]   protocol  Family specific protocol (Currently unused)

@return
  Positive socket descriptor upon success, negative error code on failure.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qsocket(int domain, int type, int protocol)
{
  int ret;
  ret = socket(domain, type, protocol);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qconnect
===========================================================================*/
/*!
@brief

  Connect the socket to the specified destination endpoint. Considering
  IPC Router is a connection-less protocol, this does not result in
  an actual connection, but just associates the socket to the remote
  endpoint. Thus, if the remote endpoint terminates, the sender will
  detect a failure.

@param[in]   fd        Socket descriptor
@param[in]   addr      Address of the destination endpoint
@param[in]   addrlen   Length of the address

@return
  0 on success, negative error code on failure.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qconnect(int fd, struct qsockaddr *addr, qsocklen_t addrlen)
{
  int ret;
  ret = connect(fd, (const struct sockaddr *)addr, addrlen);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qbind
===========================================================================*/
/*!
@brief

  Binds a socket to a specific name.
  Since all ports in IPC Router are ephemeral ports, it is not allowed
  to bind to a specified physical port ID (Like UDP/IP).

@param[in]   fd        Socket descriptor
@param[in]   addr      Name to bind with the socket
@param[in]   addrlen   Length of the address

@return
  0 on success, negative error code on failure.

@note

  - Dependencies
    - Note that the 'type' field of the NAME _must_ be allocated and reserved
      by the QTI Interface Control board else it is highly likely that
      the users might be in conflict with names allocated to other users.

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qbind(int fd, struct qsockaddr *addr, qsocklen_t addrlen)
{
  int ret;
  ret = bind(fd, (const struct sockaddr *)addr, addrlen);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qsendto
===========================================================================*/
/*!
@brief

  Sends a message to a specific remote address

@param[in]   fd        Socket descriptor
@param[in]   buf       Pointer to the buffer
@param[in]   len       Length of the buffer
@param[in]   flags     Flags - See SOCKET SEND/RECV FLAGS for more info
@param[in]   addr      Address of the destination
@param[in]   addrlen   Length of the address

@return
  Transmitted length on success, negative error code on failure.
  If the destination is flow controlled, and QMSG_DONTWAIT is
  set in flags, then the function can return QEAGAIN

  If the address is of type IPCR_ADDR_NAME, the message is multicast
  to all ports which has bound to the specified name.

@note
  - Dependencies
    - None

  - Retry logic (After receiving an error of QEAGAIN)
    - If QMSG_DONTWAIT is set in the flags, the user must retry
      after blocking and receiving the:
      * QPOLLOUT event (In the case of a connected socket)'
      * QPOLLIN event in the case of a unconnected socket and
        qrecvfrom() returns a zero length message with the unblocked
        destination in the output address.

  - Side Effects
    - Function can block.
*/
/*=========================================================================*/
static inline int qsendto(int fd, void *buf, size_t len, int flags,
                          struct qsockaddr *addr, qsocklen_t addrlen)
{
  int ret;
  ret = sendto(fd, buf, len, flags, (const struct sockaddr *)addr, addrlen);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qsend
===========================================================================*/
/*!
@brief

  Sends a message to the connected address

@param[in]   fd        Socket descriptor
@param[in]   buf       Pointer to the buffer
@param[in]   len       Length of the buffer
@param[in]   flags     Flags - See SOCKET SEND/RECV FLAGS for more info

@return
  Transmitted length on success, negative error code on failure.
  If the destination is flow controlled, and QMSG_DONTWAIT is
  set in flags, then the function can return QEAGAIN

  If the address is of type IPCR_ADDR_NAME, the message is multicast
  to all ports which has bound to the specified name.

@note
  - Dependencies
    - None

  - Retry logic (After receiving an error of QEAGAIN)
    - If QMSG_DONTWAIT is set in the flags, the user must retry
      after blocking and receiving the:
      * QPOLLOUT event (In the case of a connected socket)'
      * QPOLLIN event in the case of a unconnected socket and
        qrecvfrom() returns a zero length message with the unblocked
        destination in the output address.

  - Dependencies
    - qconnect() must have been called to associate this socket
      with a destination address

  - When the connected endpoint terminates (normally or abnormally), the
    function would unblock and return QENOTCONN/QEDESTADDRREQ.

  - Side Effects
    - Function can block.
*/
/*=========================================================================*/
static inline int qsend(int fd, void *buf, size_t len, int flags)
{
  int ret;
  ret = send(fd, buf, len, flags);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qrecvfrom
===========================================================================*/
/*!
@brief

  Receives a message from a remote address

@param[in]   fd        Socket descriptor
@param[in]   buf       Pointer to the buffer
@param[in]   len       Length of the buffer
@param[in]   flags     Flags - See SOCKET SEND/RECV FLAGS for more info
@param[out]  addr      Address of the sender
@param[inout]addrlen   Input: Size of the passed in buffer for address
                       Output: Size of the address filled by the framework.
                       (Can be NULL if addr is also NULL).

@return
  Received packet size in bytes, negative error code in failure.

@note

  - Dependencies
    - None

  - Retry logic
    - If QMSG_DONTWAIT is set in the flags, the user must retry
      after blocking and receiving the QPOLLIN event using qpoll()

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qrecvfrom(int fd, void *buf, size_t len, int flags,
                            struct qsockaddr *addr, qsocklen_t *addrlen)
{
  int ret;
  ret = recvfrom(fd, buf, len, flags,  (const struct sockaddr *)addr,
                 (socklen_t *)addrlen);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qrecv
===========================================================================*/
/*!
@brief

  Receives a message from the connected address address

@param[in]   fd        Socket descriptor
@param[in]   buf       Pointer to the buffer
@param[in]   len       Length of the buffer
@param[in]   flags     Flags - See SOCKET SEND/RECV FLAGS for more info

@return
  Received packet size in bytes, negative error code in failure.

@note

  - Dependencies
    - qconnect() must have been called to associate this socket
      with a destination address

  - Retry logic
    - If QMSG_DONTWAIT is set in the flags, the user must retry
      after blocking and receiving the QPOLLIN event using qpoll()

  - When the connected endpoint terminates (normally or abnormally), the
    function would unblock and return QENOTCONN/QEDESTADDRREQ.

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qrecv(int fd, void *buf, size_t len, int flags)
{
  int ret;
  ret = recv(fd, buf, len, flags);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qgetsockopt
===========================================================================*/
/*!
@brief

  Gets an option value

@param[in]   fd        Socket descriptor
@param[in]   level     Level of the option. Currently the only supported
                       level is QSOL_IPC_ROUTER
@param[in]   optname   Option name to get
@param[in]   optval    Buffer to place the option
@param[inout]optlen    In: Length of the buffer passed into qgetsockopt
                       Out: Length of the filled in options

@return
  0 on success, negative error code on failure.

@note
  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qgetsockopt(int fd, int level, int optname, void *optval,
                              qsocklen_t *optlen)
{
  int ret;
  ret = getsockopt(fd, level, optname, optval, (socklen_t *)optlen);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qsetsockopt
===========================================================================*/
/*!
@brief

  Sets an option on a socket

@param[in]   fd        Socket descriptor
@param[in]   level     Level of the option. Currently the only supported
                       level is QSOL_IPC_ROUTER
@param[in]   optname   Option name to get
@param[in]   optval    Buffer to place the option
@param[in]   optlen    Length of the buffer passed into qsetsockopt

@return
  0 on success, negative error code on failure.

@note
  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qsetsockopt(int fd, int level, int optname, void *optval,
                              qsocklen_t optlen)
{
  int ret;
  ret = setsockopt(fd, level, optname, optval, optlen);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qpoll
===========================================================================*/
/*!
@brief

  Blocks on requested events on the provided socket descriptors

@param[in]   pfd       Array of poll info (See qpollfd for more info)
@param[in]   num       Number of sockets to wait on (Len of pfd array)
@param[in]   timeout_ms Timeout in milli-seconds:
                        -1 = Infinite
                        0  = poll, return immediately if there are no events
                        val > 0, timeout

@return
  Total number of socket descriptors which have events on them on success
  0 if there were no events, and the function unblocked after the timeout.
  Negative error code on failure.

@note

  - Dependencies
    - None

  - Side Effects
    - Blocks waiting for events
*/
/*=========================================================================*/
static inline int qpoll(struct qpollfd *pfd, qnfds_t num, int timeout_ms)
{
  int ret;
  ret = poll((struct pollfd *)pfd, num, timeout_ms);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qclose
===========================================================================*/
/*!
@brief

  Closes the socket

@param[in]   fd        Socket descriptor

@return
  0 on success, negative error code on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static inline int qclose(int fd)
{
  int ret;
  ret = close(fd);
  if (ret < 0)
    return -errno;

  return ret;
}

/*===========================================================================
  FUNCTION  qshutdown
===========================================================================*/
/*!
@brief

  Shuts down a socket

@param[in]   fd        Socket descriptor
@param[in]   how       QSHUTRD (or 0) - Stop receiving packets
                       QSHUTWR (or 1) - Stop transmitting packets
                       QSHUTRDWR (or 2) - Stop both receiving and transmitting

@return
  0 on success  negative error code on failure.

@note

  - Dependencies
    - None

  - Side Effects
    - shutting down both RD and WR will have the same effect as qclose()
*/
/*=========================================================================*/
static inline int qshutdown(int fd, int how)
{
  int ret;
  ret = shutdown(fd, how);
  if (ret < 0)
    return -errno;

  return ret;
}


#ifdef __cplusplus
}
#endif

#endif /*_QSOCKET_H_*/
