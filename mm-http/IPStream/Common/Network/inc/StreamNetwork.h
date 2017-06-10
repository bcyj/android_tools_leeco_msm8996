#ifndef STREAMNETWORK_H
#define STREAMNETWORK_H
/*
 * StreamNetwork.h
 *
 * @brief The Stream Services mobile BSD socket API file. Contains basic API
 * functions for socket manipulation.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/Network/BSD/main/latest/inc/StreamNetwork.h#10 $
$DateTime: 2012/03/20 08:56:06 $
$Change: 2284745 $

========================================================================== */

/*===========================================================================

                           INCLUDE FILES FOR MODULE

===========================================================================*/
#include "dsbsd.h"
#include "StreamQueue.h"
#ifdef WIN32
typedef int socklen_t;
#endif

/*ioctl event status types*/
typedef enum
{
  BSD_STATUS_ERROR                      = -3,

  BSD_QOS_STATUS_MIN                    = -2,
  BSD_QOS_STATUS_WAITING                = BSD_QOS_STATUS_MIN,
  BSD_QOS_STATUS_ERROR                  = -1,
  BSD_QOS_STATUS_AVAILABLE              = 0,
  BSD_QOS_STATUS_AVAILABLE_MODIFIED     = 1,
  BSD_QOS_STATUS_UNAVAILABLE            = 2,
  BSD_QOS_STATUS_DEACTIVATED            = 3,
  BSD_QOS_STATUS_MODIFY_ACCEPTED        = 4,
  BSD_QOS_STATUS_MODIFY_REJECTED        = 5,
  BSD_QOS_STATUS_MAX                    = BSD_QOS_STATUS_MODIFY_REJECTED,

  BSD_MCAST_STATUS_MIN                  = 6,
  BSD_MCAST_STATUS_WAITING              = BSD_MCAST_STATUS_MIN,
  BSD_MCAST_STATUS_ERROR                = 7,
  BSD_MCAST_STATUS_SUCCESS              = 8,
  BSD_MCAST_STATUS_DEREGISTERED         = 9,
  BSD_MCAST_STATUS_FAILED               = 10,
  BSD_MCAST_STATUS_MAX                  = BSD_MCAST_STATUS_FAILED,

  BSD_STATUS_MIN                        = 11,
  BSD_STATUS_NETWORK_BANDWIDTH_MODIFIED = BSD_STATUS_MIN,
  BSD_STATUS_MAX                        = BSD_STATUS_NETWORK_BANDWIDTH_MODIFIED,

  BSD_QOS_SYSTEM_MIN                    = 12,
  BSD_QOS_AWARE_SYSTEM                  = BSD_QOS_SYSTEM_MIN,
  BSD_QOS_UNAWARE_SYSTEM                = 13,
  BSD_QOS_SYSTEM_MAX                    = BSD_QOS_UNAWARE_SYSTEM
} bsd_status_enum_type;

typedef enum
{
  BSD_IFACE_IOCTL_MIN                   = 0,
  BSD_IFACE_IOCTL_GET_BEARER_TECHNOLOGY,
  BSD_IFACE_IOCTL_GET_DATA_BEARER_RATE,
  BSD_IFACE_IOCTL_GET_ST_DATA_BEARER_RATE,
  BSD_IFACE_IOCTL_ON_QOS_AWARE_SYSTEM
} bsd_iface_ioctl_type;

typedef struct
{
  uint8 type;
  union
  {
    struct sockaddr_in  v4;
    struct sockaddr_in6 v6;
  }sockaddr;
} SockAddrType;

typedef void (*bsdcb_fcn_type)(bsd_status_enum_type,
                               void *cb_data, void* cb_info);

typedef bsd_status_enum_type bsd_qos_status_enum_type;

typedef enum
{
  /* Nn addtional information, do the default */
  BSD_QOS_INFO_CODE_NONE,
  /* Request a reactivation of the QOS */
  BSD_QOS_INFO_CODE_REACTIVATE,
  /* Not further QOS requests are allowed */
  BSD_QOS_INFO_CODE_REQUEST_DISALLOWED
} bsd_qos_info_code_enum_type;

typedef struct
{
  //Must be the first element of this structure for qget/qput to work!
  StreamQ_link_type link;
  void* user_data;
  int16 app_id;
  bool in_use;
  bsd_iface_ioctl_event_enum_type event;
  bsd_iface_ioctl_event_info_union_type event_info;
  bsd_iface_id_type iface_id;
}bsd_iface_ioctl_cb_info_type;

typedef struct
{
  StreamQ_link_type link;
  uint32 event_mask;
  int32 sockfd;
  int8 in_use;
  int16 dss_nethandle;
}bsd_sock_cb_info_type;

typedef struct
{
  bsd_iface_ioctl_qos_get_flow_spec_type* granted_qos_spec;
  /* Carries the additional information indicating
   * the action that can be taken */
  bsd_qos_info_code_enum_type qos_info_code;
}bsd_qos_event_type;

typedef struct
{
  bsd_qos_event_type qos_event;
  bsd_mcast_handle_type mcast_handle;
}event_info_type;

typedef struct
{
  bool is_valid;
  bsd_status_enum_type qos_status;
  bsd_qos_handle_type qos_handle;
}bsd_qos_info;

typedef struct
{
  bool isQoSEnabled;
  int32 primaryPDPProfileNo;
  int addrFamily;
  int32 ifaceName;
}NetPolicyInfo;

/* supported network interfaces */
typedef enum
{
  STREAM_NETWORK_IFACE_ANY,
  STREAM_NETWORK_IFACE_CDMA_SN,
  STREAM_NETWORK_IFACE_CDMA_AN,
  STREAM_NETWORK_IFACE_UMTS,
  STREAM_NETWORK_IFACE_SIO,
  STREAM_NETWORK_IFACE_CDMA_BCAST,
  STREAM_NETWORK_IFACE_WLAN,
  STREAM_NETWORK_IFACE_DUN,
  STREAM_NETWORK_IFACE_FLO,
  STREAM_NETWORK_IFACE_DVBH,
  STREAM_NETWORK_IFACE_STA,
  STREAM_NETWORK_IFACE_IPSEC,
  STREAM_NETWORK_IFACE_LO,
  STREAM_NETWORK_IFACE_MBMS,
  STREAM_NETWORK_IFACE_IWLAN_3GPP,
  STREAM_NETWORK_IFACE_IWLAN_3GPP2,
  STREAM_NETWORK_IFACE_MIP6,
  STREAM_NETWORK_IFACE_UW_FMC,
  STREAM_NETWORK_IFACE_CMMB,
  STREAM_NETWORK_IFACE_MAX          = STREAM_NETWORK_IFACE_CMMB + 1
}StreamNetworkIfaceEnum;

typedef void * DEVICE_HANDLE;

/*supported mbms interfaces*/
typedef enum
{
  BSD_MBMS_SERVICE_METHOD_UNAVAILABLE,
  BSD_MBMS_SERVICE_METHOD_BROADCAST,
  BSD_MBMS_SERVICE_METHOD_MULTICAST
}bsd_mbms_service_method;

// MBMS join information
typedef struct bsd_mbms_join_info
{
  uint64 tmgi;
  bsd_mbms_service_method service_method;
} bsd_mbms_join_info;

/*---------------------------------------------------------------------------
  Structure used for multicast add/drop membership through ioctl
---------------------------------------------------------------------------*/
struct ip_mcast_info
{
  IpAddrType im_multiaddr;             /* multicast group to join          */
  uint16 im_port ;
  bsd_mbms_join_info mbms_info;
};


/*
 * CStreamNetwork interface from which specializations like the unicast (using
 * winsocket) and multicast (using ISockPort) will be defined
 *
 */
class CStreamNetwork
{
public:
  /*
   * @brief Creates an instance of BSD socket services for an application.
   *
   * @param[in] pEnv. CS Environment pointer
   * @param[out]  0 if successful. -1 if an error occurred.
   */
  CStreamNetwork
  (
    void
  )
  {
  }

  /*
   * @brief: Closes BSD socket services to an application
   *
   */
  virtual ~CStreamNetwork
  (
    void
  )
  {
  }

  /*
   * @brief: Creates an instance of the Stream Services Nekwork Object
   *
   * @param[in] pEnv. Component services environment pointer.
   * @param[in] isMcastSession. Indicates if the netwrok object will be used
   *            for MultiCast session, false indicates Unicast Session.
   *
   * @return. Returns an instance of the Stream Network Object
   *
   */
  static CStreamNetwork *CreateInstance
  (
    bool bPersistentConnection=false
  );

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
  virtual int socket
  (
    int family,
    int type,
    int protocol
  ) = 0;

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
  virtual int bind
  (
    int sockfd,
    struct sockaddr *localaddr,
    int addrlen
  ) = 0;

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
  virtual int connect
  (
    int sockfd,
    struct sockaddr *addr,
    socklen_t addrlen
  ) = 0;

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
  virtual int close
  (
    int sockfd
  ) = 0;

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
  virtual int read
  (
    int   sockfd,
    void *buf,
    size_t len
  ) = 0;

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
   */
  virtual int recvfrom
  (
    int   sockfd,
    void *buf,
    int   len,
    struct sockaddr *fromaddr,
    int  *fromlen
  ) = 0;

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
  virtual int write
  (
    int   sockfd,
    const void *buf,
    size_t len
  ) = 0;

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
  virtual int sendto
  (
    int   sockfd,
    const void *buf,
    int   len,
    struct sockaddr *toaddr,
    int   tolen
  ) = 0;

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
  virtual int select
  (
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    struct timeval *timeout
  ) = 0;

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
  virtual struct hostent *gethostbyname
  (
    const char *name
  ) = 0;

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
  virtual int getsockname
  (
    int sockfd,
    struct sockaddr *addr,
    int *addrlen
  ) = 0;

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
  virtual int getpeername
  (
    int sockfd,
    struct sockaddr *addr,
    int *addrlen
  ) = 0;

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
  virtual int ioctl
  (
    int handle,
    int request,
    void *arg_ptr
  ) = 0;

  /*
   * @brief Gets the socket option
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
  virtual int getsockopt
  (
    int sockfd,
    int level,
    int optname,
    void* optval,
    size_t *optlen
  ) = 0;

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
  virtual int setsockopt
  (
    int sockfd,
    int level,
    int optname,
    void* optval,
    size_t optlen
  ) = 0;

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
  virtual int accept
  (
    int sockfd,
    struct sockaddr *localaddr,
    socklen_t *addrlen
  ) = 0;

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
  virtual int listen
  (
    int sockfd,
    int backlog
  ) = 0;

  /*
   * @brief Aborts the current opertation and returns immediatly
   *
   */
  virtual void abort
  (
    void
  ) = 0;

  /*
   * @brief Return the error code of the last socket operation
   *
   * @return the errno
   *
   */
  virtual int get_last_error
  (
     void
  ) = 0;

  /*
   * @brief Sets the errno value
   *
   * @param[in] dsbsd_errno. the errno value to be set
   *
   */
  virtual void set_last_error
  (
    int dsbsd_errno
  ) = 0;

  /*
   * @brief Registers with network (RIL) for network events
   * (Qos, Mcast etc).
   *
   * @param[in] cb_fcn - Pointer to the callback function
   * @param[in] cb_data - Pointer to the callback data
   *
   * @return TRUE for success and FALSE otherwise
   */
  virtual bool register_for_network_events
  (
    bsdcb_fcn_type cb_fcn,
    void* cb_data
  ) = 0;

  /*
   * @brief De-registers with network (RIL) for network events
   * (Qos, Mcast etc).
   *
   * @return TRUE for success and FALSE otherwise
   */
  virtual bool deregister_for_network_events
  (
    void
  ) = 0;

  /*
   * @brief Get network bandwidth.
   *
   * @param[out] nwBandwidth - Approx network bandwidth in bps (actually Kbps * 1000).
   *                           0 indicates an invalid bandwidth. Look at nwBandwidth if
   *                           the return value is TRUE
   * @param[in] qos_info - qos information
   *
   * @return TRUE for success and FALSE otherwise
   */
  virtual bool get_network_bandwidth
  (
    uint32& nwBandwidth
  ) = 0;

  /*
   * @brief Selects and opens the broadcast interface
   *
   * @param[in] bcast_iface_name bcast ifface that is to be used
   * @param[in] addr type true if IPv6
   *
   * @return 0 in case of success -1 otherwise
   *
   */
  virtual int open_bcast_iface
  (
    StreamNetworkIfaceEnum bcast_iface_name,
    bool mcastAddrIPv6Type
  ) = 0;

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
  virtual int enable_mcast
  (
    bsd_mcast_handle_type* mcast_handle,
    struct ip_mcast_info* mcast_info,
    bool mcast_cb_flag,
    void* mcast_cb_data
  ) = 0;

  /*
   * @brief Leave the mcastsession
   *
   * @param[in] mcast_handle. handle to multicast session returned in join
   *
   *
   * @return The status of the operation success or error.
   */
  virtual int disable_mcast
  (
    bsd_mcast_handle_type mcast_handle
  ) = 0;

  /*
   * @brief. Closing the broadcast interface. Currently nothing to be done.
   *
   * @return 0 if successful. -1 if an error occurred.
   */
  virtual int close_bcast_iface
  (
    void
  ) = 0;

  /*
   * @brief Sets the network policy for the BSD application
   * as specified by the parameter
   *
   * @param[in] net_policy. net poliy hadnle
   *
   *
   * @return 0 success, <0 for failure
   */
  virtual int set_net_policy
  (
    NetPolicyInfo* net_policy
  )=0;

  /*
   * @brief Gets the network policy for the BSD application
   * as specified by the parameter
   *
   * @param[out] net_policy. net policy hadnle
   *
   *
   * @return 0 success, <0 for failure
   */
  virtual int get_net_policy
  (
    NetPolicyInfo* net_policy
  )=0;

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
  virtual bsd_qos_status_enum_type request_qos
  (
    bsd_qos_handle_type* qos_handle,
    bsd_qos_spec_type* req_qos_spec,
    bool qos_cb_flag,
    void* qos_cb_data
  )=0;

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
  virtual bsd_qos_status_enum_type modify_qos
  (
    bsd_qos_handle_type qos_handle,
    bsd_qos_spec_type *req_qos_spec
  )=0;

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
  virtual int release_qos
  (
    bsd_qos_handle_type qos_handle
  )=0;

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
  virtual int reactivate_qos
  (
    bsd_qos_handle_type qos_handle
  )=0;

};

#endif /* STREAMNETWORK_H */
