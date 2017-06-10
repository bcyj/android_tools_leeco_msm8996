#ifndef _SocketWrapperClient_h_
#define _SocketWrapperClient_h_

/*==============================================================================
  FILE:         SocketWrapperClient.h

  OVERVIEW:     Wrapper interface

  DEPENDENCIES: Logging

                Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
                Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ----------------------------------------------------------
  05-17-2010  dwc     First revision.
  06-27-2013  mts     NSRM 2.0 changes.
==============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/
#include <netinet/in.h> // sockaddr
#include <sys/socket.h> // socklen_t
#include <arpa/inet.h> //inet_ntop
#ifdef __cplusplus
#include <string>
#endif

/*------------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ---------------------------------------------------------------------------*/


#define CNE_WRAPPER_AFLOCAL_PATH "/dev/socket/nims"
#define DPM_WRAPPER_AFLOCAL_PATH "/dev/socket/dpmwrapper"

#define SWIM_NIMS_T_UPDATE_PERIOD_MS 2000
#define SWIM_NIMS_STALE_CONN_PERIOD 5
#define SWIM_NIMS_STALE_HISTORY_PERIOD 300

/*------------------------------------------------------------------------------
 * Type Declarations
 * ---------------------------------------------------------------------------*/

//
// Types of return codes
//
typedef enum SwimNimsRetCodeType_e
{
  SWIM_NIMS_RET_CODE_SUCCESS = 0,
  SWIM_NIMS_RET_CODE_FAILURE = -1,
  SWIM_NIMS_RET_CODE_NO_INTERFACES_AVAILABLE = -2,
  SWIM_NIMS_RET_CODE_RESEND = -3,
  SWIM_NIMS_RET_CODE_NOT_ALLOWED = -4,
  SWIM_NIMS_RET_CODE_SKIP_SELECTION = -5,
} SwimNimsRetCodeType_t;


//
// Unit for describing a transport endpoint
//
typedef union __attribute__(( packed )) SwimNimsSockAddrUnion
{

  struct sockaddr sa;
  struct sockaddr_in in;
  struct sockaddr_in6 in6;

#ifdef __cplusplus

  /*
   *--------------------------------------------------------------------------------------
   *  FUNCTION:     SwimNimsSockAddr_t :: toString
   *
   *  DESCRIPTION:   returns the string representation of IPaddress and port
   *  like "IPAddress:Port"
   *
   *  DEPENDENCIES:  none
   *
   *  PARAMETERS:    none
   *
   *  RETURN VALUE:  std::string
   *
   *  SIDE EFFECTS:  None
   *
   *--------------------------------------------------------------------------------------
   */
  const std::string toString() const
  {
    char addrPortStr[ INET6_ADDRSTRLEN + 7 ]; //5 bytes for port and 1 byte for separator
                                              //and 1 for null terminator
    bzero( addrPortStr, sizeof(addrPortStr));

    switch ( sa.sa_family )
    {
      case AF_INET:
        {
          inet_ntop( sa.sa_family, &in.sin_addr, addrPortStr, (socklen_t)sizeof( addrPortStr ) );
          snprintf( addrPortStr + strlen( addrPortStr ),
                    sizeof( addrPortStr ) - strlen( addrPortStr ), ":%d", ntohs(in.sin_port) );
        }
        break;

      case AF_INET6:
        {
          inet_ntop( sa.sa_family, &in6.sin6_addr, addrPortStr, (socklen_t)sizeof( addrPortStr ) );
          snprintf( addrPortStr + strlen( addrPortStr ),
                    sizeof( addrPortStr ) - strlen( addrPortStr ), ":%d", ntohs(in6.sin6_port) );
        }
        break;

      default:
        break;
    }

    return std::string(addrPortStr);
  }

  /*----------------------------------------------------------------------------
   * FUNCTION      equality
   *
   * DESCRIPTION   Compare whether addresses are the same
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  bool operator==( const union SwimNimsSockAddrUnion &rhs ) const
  {
    if ( sa.sa_family != rhs.sa.sa_family )
      return false;

    switch ( sa.sa_family )
    {

      case AF_INET:
        if ( in.sin_addr.s_addr != rhs.in.sin_addr.s_addr )
          return false;
        if ( in.sin_port != rhs.in.sin_port )
          return false;
        break;

      case AF_INET6:
        if ( in6.sin6_port != rhs.in6.sin6_port )
          return false;
        if (memcmp(&(in6.sin6_addr), &(rhs.in6.sin6_addr), sizeof(struct in6_addr)) != 0) {
           return false;
        }

        break;

      default:
        break;

    }

    return true;
  };

  /*----------------------------------------------------------------------------
   * FUNCTION      equality
   *
   * DESCRIPTION   Similar to ==, but compare only if addresses are the same
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  bool addr_cmp( const union SwimNimsSockAddrUnion &rhs ) const
  {
    if ( sa.sa_family != rhs.sa.sa_family )
      return false;

    switch ( sa.sa_family )
    {

      case AF_INET:
        if ( in.sin_addr.s_addr != rhs.in.sin_addr.s_addr )
          return false;
        break;

      case AF_INET6:
        if ( in6.sin6_addr.s6_addr32[0] !=
             rhs.in6.sin6_addr.s6_addr32[0] )
          return false;
        if ( in6.sin6_addr.s6_addr32[1] !=
             rhs.in6.sin6_addr.s6_addr32[1] )
          return false;
        if ( in6.sin6_addr.s6_addr32[2] !=
             rhs.in6.sin6_addr.s6_addr32[2] )
          return false;
        if ( in6.sin6_addr.s6_addr32[3] !=
             rhs.in6.sin6_addr.s6_addr32[3] )
          return false;
        break;

      default:
        break;

    }

    return true;
  };
#endif

} SwimNimsSockAddr_t;

//
// Outer header for making a request of NIMS
//
typedef struct SwimNimsMsg_s
{

  unsigned short type;
  unsigned short length;
  char value[0];

} __attribute__(( packed )) SwimNimsMsg_t;

//
// Types of requests
//
typedef enum CneIpcReqMsgType_e
{
  CNE_IPC_REQ_MASK_INVAL = 0x0000,
  CNE_IPC_REQ_MASK_IFSEL = 0x0001,
  CNE_IPC_REQ_MASK_SYNC = 0x0002,
  CNE_IPC_REQ_MASK_FILTER_REPORT = 0x0004,
  CNE_IPC_REQ_MASK_DNS_REPORT = 0x0008,
  CNE_IPC_REQ_MASK_SOCK_TRACK = 0x0010,
  CNE_IPC_REQ_MASK_ALL = 0xFFFF
} __attribute__(( packed )) CneIpcReqMsgType_t;

//
// Types of responses
//
typedef enum CneIpcRspMsgType_e
{
  CNE_IPC_RSP_MASK_INVAL = 0x0000,
  CNE_IPC_RSP_MASK_IFSEL = 0x0001,
  CNE_IPC_RSP_MASK_SYNC = 0x0002,
  CNE_IPC_RSP_MASK_FILTER_REPORT = 0x0004,
  CNE_IPC_RSP_MASK_DNS_REPORT = 0x0008,
  CNE_IPC_RSP_MASK_SOCK_TRACK = 0x0010,
  CNE_IPC_RSP_MASK_ALL = 0xFFFF
} __attribute__(( packed )) CneIpcRspMsgType_t;

//
// Types of atp filter actions
//
typedef enum AtpFitlerActions_e
{
  ATP_FILTER_ACTION_INVALID = 0,
  ATP_FILTER_ACTION_ADD = 1,
  ATP_FILTER_ACTION_REMOVE = 2,
} CneAtpFitlerActions_t;

//
// ATP filter to save in wrapper.
//
typedef struct CneAtpFilter_s
{
  uint8_t  action;
  uint16_t family;
  uint16_t src_port;
  uint16_t dst_port;
  union{
    struct in_addr  v4;
    struct in6_addr v6;
  }src_addr;
  union{
    struct in_addr  v4;
    struct in6_addr v6;
  }dst_addr;
  int protocol;
  uid_t uid;  //unsigned int
  uid_t parent_uid;
  pid_t pid; //int
} __attribute__(( packed )) CneAtpFilter_t;


//
// IPC request message structure..
//
typedef struct CneIpcReqMsg_s
{
  SwimNimsMsg_t msgh;
  SwimNimsSockAddr_t src;
  socklen_t srclen;
  SwimNimsSockAddr_t dst;
  socklen_t dstlen;

  unsigned char sync;
  unsigned char type;

  unsigned int time;

  uint8_t  action;
  int protocol;
  uid_t uid;  //unsigned int
  uid_t parent_uid;
  pid_t pid; //int
  int fd_val; //application's socket fd value
}  __attribute__(( packed )) CneIpcReqMsg_t;

//
// IPC response message structure..
//
typedef struct CneIpcRspMsg_s
{
  SwimNimsMsg_t msgh;
  int genRetCode;
  int ifselRetCode;
  int syncRetCode;
  int fltrReportRetCode;
  int dnsReportRetcode;
  unsigned int dnsThresh;
  unsigned short gateState;
}  __attribute__(( packed )) CneIpcRspMsg_t;

/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/


#endif /* _SocketWrapperClient_h_ */
