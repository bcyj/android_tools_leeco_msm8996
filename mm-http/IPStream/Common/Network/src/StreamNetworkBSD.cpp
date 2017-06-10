/*
 * StreamNetworkBSD.cpp
 *
 * @brief internal implementation of the Data Services mobile BSD socket API
 * file.
 *
 * COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
 * All rights reserved. QUALCOMM Technologies proprietary and confidential.
 *
 */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/Network/BSD/main/latest/src/StreamNetworkBSD.cpp#14 $
$DateTime: 2012/09/04 07:38:31 $
$Change: 2762671 $

========================================================================== */

/*===========================================================================

                           INCLUDE FILES FOR MODULE

===========================================================================*/

/* BSD includes */
#include "StreamNetworkBSD.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

/* utils */
#include "SourceMemDebug.h"
#include "qtv_msg.h"
#include "MMTime.h"

#define UNUSED(x) ((void)x)

/*===========================================================================

                             FORWARD DECLARATIONS

===========================================================================*/

/*===========================================================================

                    DEFINITIONS AND VARIABLE DECLARATIONS

===========================================================================*/
/*
 * @brief Creates an instance of BSD socket services for an application.
 *
 * @param[out]  0 if successful. -1 if an error occurred.
 */
CStreamNetworkBSD::CStreamNetworkBSD
(
  int &result
) : m_aborted(false),
    m_hConnection(0),
    m_hAbortConnection(0),
    m_numOfSockets(0)
{
  InitializeErrorCodes();
  result = STREAMNET_SUCCESS;
}

/*
 * @brief: Closes BSD socket services to an application
 *
 */
CStreamNetworkBSD::~CStreamNetworkBSD
(
  void
)
{
  /*Dummy Destructor*/
}

/*
 * @brief Creates a socket
 *
 * Creates an endpoint for data communication.  socket() checks the socket
 * reference count to determine the number of sockets the application has
 * opened.
 *
 * @param[in] family. address family (only AF_INET is supported)
 * @param[in] type. type of service (SOCK_STREAM for TCP, SOCK_DGRAM for UDP)
 * @param[in] protocol. protocol number to use for the given family and type.
 *            The caller may specify 0 for the protocol number to request the
 *            default for the given family and type or explicitly specify
 *            IPPROTO_TCP and IPPROTO_UDP as appropriate.
 *
 * @return socket descriptor or -1 to indicate that an error has occurred.
 *
 * errno contains the code specifying the cause of the error.
 *
 */
int CStreamNetworkBSD::socket
(
  int family,
  int type,
  int protocol
)
{
  int result = 0;

  // bring up the data connection for the first socket
  m_numOfSockets++;
  if (m_numOfSockets == 1)
  {
    result = AttemptConnection();
    if (result != 0)
    {
      QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                    "AttemptConnection failed %d", result);
      m_numOfSockets = 0;
    }
  }

  QTV_MSG_PRIO3(QTVDIAG_GENERAL, QTVDIAG_PRIO_LOW,
                "creating socket... family:%d type:%d proto:%d", family, type, protocol);
  // create a socket
  if (result == 0)
  {
    result = ::socket(family, type, protocol);
  }

  QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
                "created socket status:%d", result);

  // check for user aborts
  if (IsAborted())
  {
    if(result > 0)
    {
      ::close(result);
    }

    //  socket created then close and return error
    if (result != INVALID_SOCKET)
    {
      close(result);
    }
    errno = EINTR;
    result = INVALID_SOCKET;
  }

  return result;
}

/*
 * @brief Attaches a local address and port value to a client socket
 *
 * If the call is not explicitly issued, the socket will implicitly bind in
 * calls to connect() or sendto().  Note that this function does not support
 * binding a local IP address, but rather ONLY a local port number.  The local
 * IP address is assigned automatically by the sockets library.
 *
 * param[in] sockfd. socket descriptor
 * param[in] localaddr. Address of the structure specifying the IP address and
 *           port number
 * param[in]  addrlen. size of the above address structure in bytes
 *
 * @return 0 if successful.-1 if an error occurred.
 *
 * An error code indicating the cause of the error is written to errno.
 */
int CStreamNetworkBSD::bind
(
  int sockfd,
  struct sockaddr *localaddr,
  int addrlen
)
{
  int result = ::bind(sockfd, localaddr, addrlen);

  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }

  return result;
}

/*
 * @brief  Initiates a TCP handshake with a remote endpoint address.
 *
 * The underlying implementation does not support connected UDP sockets.
 *
 * @param[in] sockfd. socket descriptor
 * @param[in] addr.  pointer to the structure that specifies the IP address and
 *            port number of the server
 * @param[in] addrlen. size of the server address structure in bytes
 *
 * @return 0 if TCP is established. -1 if an error occurred.
 *
 * errno contains a code specifying the cause of the error.
 */
int CStreamNetworkBSD::connect
(
  int sockfd,
  struct sockaddr *addr,
  socklen_t addrlen
)
{

  struct sockaddr_in *sockAddr = (struct sockaddr_in *)addr;
  int result = ::connect(sockfd, addr, addrlen);
  QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_LOW,
               "connect status %d errno=%d", result, errno);
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Frees the socket descriptor for reuse.
 *
 * For a socket that uses TCP, a handshake to terminate the TCP session is
 * initiated.
 *
 * @param[in] sockfd. Socket descriptor
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * errno describes the error encountered.
 *
 */
int CStreamNetworkBSD::close
(
  int sockfd
)
{
  //close the socket
  int result = ::close(sockfd);//Assuming we dont need to send or receive messages on the socket

  // for the last socket closed, go ahead and release the data connection
  if (m_numOfSockets > 0)
  {
    m_numOfSockets--;
    if (m_numOfSockets == 0)
    {
      ReleaseConnection();
    }
  }

  // check for user aborts
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief  Reads data from a socket that uses TCP.
 *
 * param[in] sockfd. socket descriptor
 * param[in] buf. address of the buffer to put the data
 * param[in] len. size of the buffer in bytes
 *
 * @result The number of bytes read or -1 on failure.
 *
 * When read() fails, errno is set to one of the following values.
 *
 * Typically, errno values are meaningful only when an error occurs
 * (function returns -1).  However, there is an exception.  When read()
 * returns 0 as the number of bytes read and the caller specified a
 * nonzero length, it means that the socket has closed.  In this case,
 * errno is set to EEOF.
 */
int CStreamNetworkBSD::read
(
  int   sockfd,
  void *buf,
  size_t len
)
{
  ssize_t result = recv(sockfd, (char *)buf, len, 0);
/* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = (ssize_t)(-1);
  }
  return (int)result;
}

/*
 * @brief Attempts to read a message from a UDP socket.
 *
 * @param[in] sockfd. socket descriptor
 * @param[in] buf. address of the buffer to put the message
 * @param[in] len. size of the buffer in bytes
 * @param[in] flags. 0 (not supported)
 * @param[out] fromaddr. Pointer to the sender's address
 * @param[out] fromlen. Length of the sender's address in bytes
 *
 * @return. The number of bytes read or -1 on failure.
 *
 * When recvfrom() fails, errno is set.
 *
 * Errno Values
 * ------------
 * EBADF            invalid socket descriptor is specfied
 * EAFNOSUPPORT     address family not supported
 * ENETDOWN         network subsystem went down
 * ENONET           attempt to bring up PPP failed - network subsystem down
 * EFAULT           DS Sock internal error, invalid BSD control block
 * EOPNOTSUPP       option not supported
 * EINTR            BSD application's callback function encountered an error
 */
int CStreamNetworkBSD::recvfrom
(
  int   sockfd,
  void *buf,
  int   len,
  struct sockaddr *fromaddr,
  int  *fromlen
)
{
  ssize_t result = ::recvfrom(sockfd, (char *)buf, len, 0, fromaddr,(socklen_t *) fromlen);/*Edited for QNX*/
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = (ssize_t)(-1);
  }
  return (int)result;
}

/*
 * @brief Writes data to a socket descriptor for transfer over TCP.
 *
 * @param[in]  sockfd. socket descriptor
 * @param[in] buf. address of the buffer containing the data
 * @param[in] len. number of bytes in the buffer

 * @result. The number of bytes written or -1 on failure.
 *
 * When write() fails, errno indicates the cause of error.  Note that an
 * error may happen after a partial write.  For instance, the underlying
 * DS Sock layer may initially accept only part of the data because the system
 * is low on buffers.  write() blocks until the DS Sock layer accepts remaining
 * data.  During the wait, an application's callback function gets invoked and
 * returns an error.  In such cases, the number of bytes written thus far is
 * returned not -1.
 *
 */
int CStreamNetworkBSD::write
(
  int   sockfd,
  const void *buf,
  size_t len
)
{
  ssize_t  result = ::send(sockfd, (const char *)buf, len, MSG_NOSIGNAL);
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = (ssize_t)(-1);
  }
  return (int)result;
}

/*
 * @brief Sends a message to a remote machine via a socket that uses UDP.
 *
 * @param[in] sockfd. socket descriptor
 * @param[in] buf. address of the buffer containing the data
 * @param[in] len. number of bytes in the buffer
 * @param[in] toaddr. pointer to the destination address structure
 * @param[in] tolen. length of the destination address in bytes
 *
 * @result The number of bytes sent or -1 on failure.
 *
 * When sendto() fails, errno is set.
 *
 * Note that the underlying layer may accept only some of the bytes before an
 * error occurs.  In such a case, the number of bytes accepted thus far is
 * returned not -1.
 */
int CStreamNetworkBSD::sendto
(
  int   sockfd,
  const void *buf,
  int   len,
  struct sockaddr *toaddr,
  int   tolen
)
{
  ssize_t  result = ::sendto(sockfd, (char *)buf, len, 0, toaddr, tolen);;
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = (ssize_t)(-1);
  }
  return (int)result;
}
/*
 *
 * @brief Waits on a list of socket descriptors
 *
 * Unblocks when all of them become ready or the maximum timeout value can
 * also be specified.
 *
 * param[in] n. any value (not supported)
 * param[in] readfds. set of socket descriptors to be monitored for input
 * param[in] writefds. set of socket descriptors to be monitored for output
 * param[in] exceptfds. NULL (not supported)
 * param[in] timeout. address of a structure specifying maximum time to wait. If
 *            NULL, the wait is infinite.
 *
 * return Number of ready socket descriptors or -1 if an error occurred.
 *
 * When select() encounters an error, errno is set.
 *
 */
int CStreamNetworkBSD::select
(
  fd_set *readfds,
  fd_set *writefds,
  fd_set *exceptfds,
  struct timeval *timeout
)
{
  int result = ::select(FD_SETSIZE, readfds, writefds, exceptfds, timeout);
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Aborts the current opertation and returns immediatly
 *
 */
void CStreamNetworkBSD::abort
(
  void
)
{
  m_aborted = true;
}


/*
 * @brief Gets information about a host given the host's name.
 *
 * @param[in] name. Character string that contains the host name.
 *
 * @return Pointer to a hostent structure if successful.
 *
 * NULL on error with errno containing the error code.
 *
 */
struct hostent *CStreamNetworkBSD::gethostbyname
(
  const char *name
)
{
  //Bring up data call if not already up. Also release
  //connection as m_numOfSockets is not changed here
  struct hostent *result = NULL;
  if (m_numOfSockets == 0)
  {
    if (AttemptConnection() == 0)
    {
      QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_LOW,
                    "calling system call gethostbyname... %s", name);
      result = ::gethostbyname(name);
    }
    ReleaseConnection();
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_LOW,
                  "calling system call gethostbyname: %s", name);
    result = ::gethostbyname(name);
  }

  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = NULL;
  }
  return result;
}

/*
 * @brief Gets information (local address) on a given socket.
 *
 * @param[in] sockfd. socket descriptor
 * @param[out] addr. address of the socket
 * @param[in] addrlen. address length
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * errno describes the error encountered.
 *
 */
int CStreamNetworkBSD::getsockname
(
  int sockfd,
  struct sockaddr *addr,
  int *addrlen
)
{
  int result = ::getsockname(sockfd, addr,( socklen_t*) addrlen);/*Edited for QNX*/
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Gets information (address) on the remote connected socket.
 *
 * @param[in] sockfd. socket descriptor
 * @param[out] addr. address of the remote socket
 * @param[in] addrlen. address length
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * errno describes the error encountered.
 *
 */
int CStreamNetworkBSD::getpeername
(
  int sockfd,
  struct sockaddr *addr,
  int *addrlen
)
{
  int result = ::getpeername(sockfd, addr,( socklen_t*) addrlen);/*Edited for QNX*/
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Sets a socket's I/O behavior.
 *
 * When the request is FIONBIO, ioctl() sets or clears the nonblocking I/O
 * flag depending on the contents of arg_ptr. If *arg_ptr is nonzero, the
 * socket is set for nonblocking I/O.  If *arg_ptr is zero, the socket returns
 * to its default I/O behavior which is blocking.
 * @param[in] sockfd. socket descriptor
 * @param[in] request. request for ioctl() to perform
 * @param[in] arg_ptr. address of the request's argument
 *
 * @result 0 if successful. -1 if an error occurred.
 *
 * errno describes the error encountered.
 *
 */
int CStreamNetworkBSD::ioctl
(
  int handle,
  int request,
  void *arg_ptr
)
{


  int result = ::ioctl(handle,
                       request,
                       (unsigned long *) arg_ptr);
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Sets the socket option
 *
 * @param[in] sockfd. socket descriptor
 * @param[in] level. IP protocol numbers. IPPROTO_XXX
 * @param[in] optname. Option name.
 * @param[in] optval. Option value.
 * @param[in/out] optlen. Option length.
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * An error code indicating the cause of the error is written to errno.
 *
 */
int CStreamNetworkBSD::getsockopt
(
  int sockfd,
  int level,
  int optname,
  void* optval,
  size_t  *optlen
)
{
  int result = ::getsockopt(sockfd, level, optname, (char *)optval,(socklen_t *) optlen);/*Edited for QNX*/
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;

}

/*
 * @brief Sets the socket option
 *
 * @param[in] sockfd. socket descriptor
 * @param[in] level. IP protocol numbers. IPPROTO_XXX
 * @param[in] optname. Option name.
 * @param[in] optval. Option value.
 * @param[in] optlen. Option length.
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * An error code indicating the cause of the error is written to errno.
 *
 */
int CStreamNetworkBSD::setsockopt
(
  int sockfd,
  int level,
  int optname,
  void* optval,
  size_t optlen
)
{
  if ( level == SOL_SOCK && optname == SO_LINGER &&
       optlen == sizeof(so_linger_type) &&
       ((so_linger_type *)optval)->l_linger > 0 )
  {
    // the API is defining this option as milliseconds, windows takes
    // in seconds
    ((so_linger_type *)optval)->l_linger /= 1000;
  }
  int result = ::setsockopt(sockfd, level, optname, (char *)optval, (socklen_t)optlen);/*Edited for QNX*/
  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Permits an incoming connection attempt on a socket.
 *
 * @param[in] sockfd. socket descriptor.
 * @param[out] localaddr. Address of the structure specifying the IP address and
 *           port number
 * @param[out] addrlen. A pointer to the size of the above address structure in bytes.
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * An error code indicating the cause of the error is written to errno.
 *
 */
int CStreamNetworkBSD::accept
(
  int sockfd,
  struct sockaddr *localaddr,
  socklen_t *addrlen
)
{
  int result = ::accept(sockfd, localaddr, addrlen);

  /* check for user aborts */
  if (IsAborted())
  {
    if(result > 0)
    {
      ::close(result);
    }

    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}

/*
 * @brief Socket listens for an incoming connection
 *
 * @param[in] sockfd. socket descriptor.
 * @param[in] backlog. Maximum number of pending connections
 *
 * @return 0 if successful. -1 if an error occurred.
 *
 * An error code indicating the cause of the error is written to errno.
 *
*/
int CStreamNetworkBSD::listen
(
  int sockfd,
  int backlog
)
{
  int result = ::listen(sockfd, backlog);

  /* check for user aborts */
  if (IsAborted())
  {
    /* return failure */
    errno = EINTR;
    result = -1;
  }
  return result;
}


/*
 * @brief Return the error code of the last socket operation
 *
 * @return the errno
 *
 */
int CStreamNetworkBSD::get_last_error
(
   void
)
{
  return errno;
}

/*
 * @brief Sets the errno value
 *
 * @param[in] dsbsd_errno. the errno value to be set
 *
 */
void CStreamNetworkBSD::set_last_error
(
  int dsbsd_errno
)
{
  errno = dsbsd_errno;
}

/*
 * @brief Registers with network (RIL) for network events
 * (Qos, Mcast etc).
 *
 * @param[in] cb_fcn - Pointer to the callback function
 * @param[in] cb_data - Pointer to the callback data
 *
 * @return TRUE for success and FALSE otherwise
 */
bool CStreamNetworkBSD::register_for_network_events
(
  bsdcb_fcn_type cb_fcn,
  void* cb_data
)
{
  UNUSED(cb_fcn);
  UNUSED(cb_data);
  return false;
}

/*
 * @brief De-registers with network (RIL).
 *
 * @return TRUE for success and FALSE otherwise
 */
bool CStreamNetworkBSD::deregister_for_network_events
(
  void
)
{
  return false;
}

/*
 * @brief Get network bandwidth.
 *
 * @param[out] nwBandwidth - Approx network bandwidth in bps (actually Kbps * 1000).
 *                           0 indicates an invalid bandwidth. Look at nwBandwidth if
 *                           the return value is TRUE
 *
 * @return TRUE for success and FALSE otherwise
 */
bool CStreamNetworkBSD::get_network_bandwidth
(
  uint32& nwBandwidth
)
{
  UNUSED(nwBandwidth);
  return false;
}

/*
 * @brief Selects and opens the broadcast interface
 *
 * @param[in] bcast_iface_name bcast ifface that is to be used
 * @param[in] addr type true if IPv6
 *
 * @return 0 in case of success -1 otherwise
 *
 */
int CStreamNetworkBSD::open_bcast_iface
(
  StreamNetworkIfaceEnum /* bcast_iface_name */,
  bool /* mcastAddrIPv6Type */
)
{
  return -1;
}

/*
 * @brief joins a mulitcast session
 *
 * @param[out] handle to multicast session
 * @param[in] the ip/port details of the mcast session to join
 * @param[in] to join set to true, to leave set to false
 * @param[in] callback data to passed on for the notification handler
 *
 * @return The status of the operation waiting, success or error.
 */
int CStreamNetworkBSD::enable_mcast
(
  bsd_mcast_handle_type* /* mcast_handle*/,
  struct ip_mcast_info* /* mcast_info */,
  bool /* mcast_cb_flag */,
  void* /* mcast_cb_data */
)
{
  return -1;
}

/*
 * @brief Leave the mcastsession
 *
 * @param[in] mcast_handle. handle to multicast session returned in join
 *
 * @return The status of the operation success or error.
 */
int CStreamNetworkBSD::disable_mcast
(
  bsd_mcast_handle_type /* mcast_handle */
)
{
  return -1;
}

/*
 * @brief. Closing the broadcast interface. Currently nothing to be done.
 *
 * @return 0 if successful. -1 if an error occurred.
 */
int CStreamNetworkBSD::close_bcast_iface
(
  void
)
{
  return -1;
}

/*
 * @brief Gets the network policy for the BSD application
 * as specified by the parameter
 *
 * @param[in] net_policy. net policy hadnle
 *
 *
 * @return 0 success, <0 for failure
 */
int CStreamNetworkBSD::get_net_policy
(
  NetPolicyInfo* /*bsd_net_policy*/
)
{
  return 0;
}

/*
 * @brief Sets the network policy for the BSD application
 * as specified by the parameter
 *
 * @param[in] net_policy. net poliy hadnle
 *
 *
 * @return 0 success, <0 for failure
 * SIDE EFFECTS
 * Sets errno to EISCONN if the network is already up,
 * or EINVAL if the argument is invalid. Otherwise errno
 * is something else.
 */
int CStreamNetworkBSD::set_net_policy
(
  NetPolicyInfo* /*bsd_net_policy*/
)
{
  return 0;
}

/**@breif Requests the specified QoS from the data services. This is an
 * asynchronous function, the result of which is reported in the argument
 * callback function.
 * This function installs a signal handler to handle asynchronous indications
 * from data services.
 *
 * @param[out]
 * qos_handle: Returns the QoS handle to be used for future QoS related calls.
 *
 * @param[in]
 * req_qos_spec: Specification of requested QoS
 * qos_cb_flag: true if app installed a CB
 * qos_cb_data: cb data
 *
 * @return
 * BSD_QOS_STATUS_WAITING: The QoS request is in process.
 * BSD_QOS_STATUS_ERROR: An error was encountered while requesting/obtaining QoS.
 */
bsd_qos_status_enum_type CStreamNetworkBSD::request_qos
(
  bsd_qos_handle_type* /*qos_handle*/,
  bsd_qos_spec_type* /*req_qos_spec*/,
  bool /*qos_cb_flag*/,
  void* /*qos_cb_data*/
)
{
  return BSD_QOS_STATUS_UNAVAILABLE;
}

/**
 * @breif Modifies the specified QoS from the data services. This is an
 * asynchrounous operation, the result of which is reported to the callback
 * function specified to bsd_request_qos().
 *
 * @param[in]
 * qos_handle: The QoS handle of the existing QoS.
 * req_qos_spec: Specification of requested QoS
 *
 * @return
 * BSD_QOS_STATUS_WAITING: The QoS modification is in process.
 * BSD_QOS_STATUS_ERROR: An error was encountered while modifying QoS.
*/
bsd_qos_status_enum_type CStreamNetworkBSD::modify_qos
(
  bsd_qos_handle_type /*qos_handle*/,
  bsd_qos_spec_type * /*req_qos_spec*/
)
{
  return BSD_QOS_STATUS_UNAVAILABLE;
}

/**
 * @breif Releases the specified QoS instance. This function does not wait for an
 * indication from the data services to indicate completion of the release.
 * It returns after issuing a release IOCTL and DS takes care of releasing
 * the instance in the background, if necessary.
 *
 * @param[in]
 * qos_handle: The handle, obtained from bsd_request_qos() call, corr. to
 * the QoS instance.
 *
 * @return
 * >= 0: in case of success
 * < 0: otherwise
 */
int CStreamNetworkBSD::release_qos
(
  bsd_qos_handle_type /*qos_handle*/
)
{
  return -1;
}

/**
 * @breif Reactivates the specified QoS instance. It simply amounts to issuing a
 * GO_ACTIVE IOCTL to the iface. If the iface is already active, GO_ACTIVE
 * is a NOOP.
 *
 * @param[in]
 * qos_handle: The handle, obtained from bsd_request_qos() call, corr. to
 * the QoS instance.
 *
 * @param[in]
 * >= 0: in case of success
 * < 0: otherwise
 */
int CStreamNetworkBSD::reactivate_qos
(
  bsd_qos_handle_type /*qos_handle*/
)
{
  return -1;
}

/*
 * @brief Inits the error codes to the WS32 specific values
 *
 */
void CStreamNetworkBSD::InitializeErrorCodes
(
  void
)
{
  EEOF           = EIO;
  EMSGTRUNC      = EFBIG;
  ENETWOULDBLOCK = EWOULDBLOCK;
  ETRYAGAIN      = EINPROGRESS;
}

/*
 * Brings up a data connection using the Connection Manager
 *
 * @return 0 on success or -1 to indicate that an error has occurred.
 */
int CStreamNetworkBSD::AttemptConnection
(
  void
)
{
/*Edited for QNX*//*what do we do here?*/
/*Execute shell command mount to load a network module,

Example to load a PPPmgr network module

    mount -d drivername -p pppmgr
*/

  return 0;
}

/*
 * Releases active data connection using the Connection Manager
 *
 */
void CStreamNetworkBSD::ReleaseConnection
(
  void
)
{
/*Edited for QNX*//*what do we do here?*/
/*Execute shell command unmount to unload a network module

unmount /dev/io-net/
*/
}
