#ifndef GPSONE_BIT_API_H_trimmed
#define GPSONE_BIT_API_H_trimmed
/*=============================================================================
  @file  gpsone_bit_api.h

  ---------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

  $Id: //depot/asic/SVLTE2_FUSION/8K_MODEM_APIS/libs/remote_apis/gpsone_bit_api/inc/gpsone_bit_api.h#2 $

  Notes:
     ==== Auto-Generated File, do not edit manually ====
     Generated from build type: AAABQNSVH
     #defines containing AABQNSVH replaced with ________
=============================================================================*/
/******************************************************************************
  @file:  gpsone_bit_api.h
  @brief: BIT (Bearer-Independent-Transport) API header file
******************************************************************************/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/SVLTE2_FUSION/8K_MODEM_APIS/libs/remote_apis/gpsone_bit_api/inc/gpsone_bit_api.h#2 $DateTime: 2010/05/10 18:43:35 $$Author: tduong $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/02/10    rh     Cleaned up some comments for code review
04/28/10    rh     Initial Release

===========================================================================*/

#ifndef GPSONE_BIT_API_H
#define GPSONE_BIT_API_H

#ifndef COMDEF_H
#define COMDEF_H
/*===========================================================================

                   S T A N D A R D    D E C L A R A T I O N S

DESCRIPTION
  This header file contains general types and macros that are of use
  to all modules.  The values or definitions are dependent on the specified
  target.  T_WINNT specifies Windows NT based targets, otherwise the
  default is for ARM targets.

       T_WINNT  Software is hosted on an NT platforn, triggers macro and
                type definitions, unlike definition above which triggers
                actual OS calls

DEFINED CONSTANTS

       Name      Definition
       -------   --------------------------------------------------------
       ON        Asserted condition
       OFF       Deasserted condition

       NULL      Pointer to nothing

       PACK()    Used to declare a C type packed for either GCC or ARM
                 compilers.

       *** DEPRECATED - WON'T WORK FOR newer versions (3.x) of GCC ***
       PACKED    Used to indicate structures which should use packed
                 alignment
       *** DEPRECATED - WON'T WORK FOR newer versions (3.x) of GCC ***

       INLINE    Used to inline functions for compilers which support this

Copyright (c) 1990-2010 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/

#ifndef COM_DTYPES_H
#define COM_DTYPES_H
/*===========================================================================

                   S T A N D A R D    D E C L A R A T I O N S

DESCRIPTION
  This header file contains general data types that are of use to all modules.
  The values or definitions are dependent on the specified
  target.  T_WINNT specifies Windows NT based targets, otherwise the
  default is for ARM targets.

  T_WINNT  Software is hosted on an NT platforn, triggers macro and
           type definitions, unlike definition above which triggers
           actual OS calls

Copyright (c) 2009-2010 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/* ------------------------------------------------------------------------
** Constants
** ------------------------------------------------------------------------ */

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE   1   /* Boolean true value. */
#define FALSE  0   /* Boolean false value. */


#ifndef NULL
  #define NULL  0
#endif

#endif  /* COM_DTYPES_H */

#ifndef _MSC_VER
#ifndef TARGET_H
#define TARGET_H
/*===========================================================================

      T A R G E T   C O N F I G U R A T I O N   H E A D E R   F I L E

DESCRIPTION
  All the declarations and definitions necessary for general configuration
  of the DMSS software for a given target environment.

Copyright (c) 1998,1999,2000,2001,2002  by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/* All featurization starts from customer.h which includes the appropriate
**    cust*.h and targ*.h
*/
#ifdef CUST_H
#ifndef CUSTOMER_H
#define CUSTOMER_H
/*===========================================================================

                   C U S T O M E R    H E A D E R    F I L E

DESCRIPTION
  This header file provides customer specific information for the current
  build.  It expects the compile time switch /DCUST_H=CUSTxxxx.H.  CUST_H
  indicates which customer file is to be used during the current build.
  Note that cust_all.h contains a list of ALL the option currently available.
  The individual CUSTxxxx.H files define which options a particular customer
  has requested.


Copyright (c) 1996, 1997       by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 1998, 1999, 2000 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 2001, 2002       by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/* Make sure that CUST_H is defined and then include whatever file it
** specifies.
*/
#ifdef CUST_H
#ifndef CUSTA_________H
#define CUSTA_________H
/* ========================================================================
FILE: CUSTA________

Copyright (c) 2001-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
=========================================================================== */

#define FEATURE_MULTIPROCESSOR

#endif /* CUSTA_________H */
#else
#endif

/* Now perform certain Sanity Checks on the various options and combinations
** of option.  Note that this list is probably NOT exhaustive, but just
** catches the obvious stuff.
*/


#endif /* CUSTOMER_H */
#endif

#endif /* TARGET_H */
#endif


/* ---------------------------------------------------------------------
** Compiler Keyword Macros
** --------------------------------------------------------------------- */


#if (! defined T_WINNT) && (! defined __GNUC__)

    /* Non WinNT Targets */

    #if defined(__ARMCC_VERSION)
      #define PACKED __packed
      #define PACKED_POST
    #else  /* __GNUC__ */
    #endif /* defined (__GNUC__) */


#else /* T_WINNT || TARGET_OS_SOLARIS || __GNUC__ */
#endif /* T_WINNT */

/* ----------------------------------------------------------------------
** Lint does not understand __packed, so we define it away here.  In the
** past we did this:
**   This helps us catch non-packed pointers accessing packed structures,
**   for example, (although lint thinks it is catching non-volatile pointers
**   accessing volatile structures).
**   This does assume that volatile is not being used with __packed anywhere
**   because that would make Lint see volatile volatile (grrr).
** but found it to be more trouble than it was worth as it would emit bogus
** errors
** ---------------------------------------------------------------------- */
#ifdef _lint
  #ifdef __packed
    #undef __packed
  #endif /* __packed */
  #define __packed
#endif

/* ----------------------------------------------------------------------
**                          STANDARD MACROS
** ---------------------------------------------------------------------- */


#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif


#endif  /* COMDEF_H */

/* Each time BIT daemon call gpsone_bit_register(), a unique/new "transport handle" will be allocated and returned */
typedef uint16 gpsone_bit_transport_handle_type;

/* BIT daemon will return a unique/new "session handle" for each successful gpsone_bit_connect() call */
typedef uint16 gpsone_bit_session_handle_type;

/* Constant value for invalid transport handle */
#define GPSONE_BIT_INVALID_TRANSPORT_HANDLE  0xFFFF

/* Constant value for invalid session handle */
#define GPSONE_BIT_INVALID_SESSION_HANDLE    0xFFFF

/* Upperbound of BIT send/recv buffer (limited by RPC): 16K */
#define GPSONE_BIT_MAX_BUF_SIZE              0x3FFF

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_E_TYPE

DESCRIPTION
  enum which defines the BIT transport's type
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_TYPE_NONE = 0,
  GPSONE_BIT_TYPE_IP,
  // GPSONE_BIT_TYPE_DBM    /*Data Burst Message,  msg sent/received on control plane */
}gpsone_bit_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_PROTOCOL_E_TYPE

DESCRIPTION
  enum which defines all the AGPS protocol which will be payload of BIT
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_PROTOCOL_TYPE_NONE = -1,
  GPSONE_BIT_PROTOCOL_TYPE_ANY = 0,
  GPSONE_BIT_PROTOCOL_TYPE_SUPL,
  GPSONE_BIT_PROTOCOL_TYPE_IS801,
  GPSONE_BIT_PROTOCOL_TYPE_V1,
  GPSONE_BIT_PROTOCOL_TYPE_V2,
  GPSONE_BIT_PROTOCOL_TYPE_JCDMA,
  GPSONE_BIT_PROTOCOL_TYPE_MAX,
} gpsone_bit_protocol_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_STATUS_E_TYPE

DESCRIPTION
  enum which defines all the possible success/error codes for BIT APIs
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_STATUS_SUCCESS = 0,             /* success */
  GPSONE_BIT_STATUS_WAIT,                    /* result is pending, will be reported back later */
  GPSONE_BIT_STATUS_FAIL,                    /* generic failure error code */

  GPSONE_BIT_STATUS_INVALID_PARAM,           /* invalid parameters */
  GPSONE_BIT_STATUS_NOT_REGISTERED,          /* no registered BIT */
  GPSONE_BIT_STATUS_IPC_FAILURE,             /* IPC failure */
  GPSONE_BIT_STATUS_FATAL_ERROR,             /* fatal error */
  GPSONE_BIT_STATUS_NOT_ALLOWED,             /* request is not allowed (for example, send/recv before connect) */
  GPSONE_BIT_STATUS_OPEN_FAILED,             /* failed to open BIT daemon */
  GPSONE_BIT_STATUS_NO_MORE_DATA,            /* running out of data for recv */
  GPSONE_BIT_STATUS_NO_RESOURCES,            /* out of resources for new request */
  GPSONE_BIT_STATUS_OUT_OF_MEMORY,           /* out of memory */
  GPSONE_BIT_STATUS_MEMORY_ACCESS_ERROR,     /* error while accessing send data or recv buffer */
  GPSONE_BIT_STATUS_NETWORK_NOT_AVAILABLE,   /* data network is not available (for example, Airplane mode) */
  GPSONE_BIT_STATUS_NOT_INTIALIZED,          /* BIT daemon has not been intialized */
  GPSONE_BIT_STATUS_NOT_IMPLEMENTED,         /* functionality has not been implemented */
} gpsone_bit_status_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_EVENT_TYPE

DESCRIPTION
  enum which defines all the possible events gpsone_bit_notify() can send
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_EVENT_NONE = 0,
  GPSONE_BIT_EVENT_OPEN_RESULT,        /* event to report result for open */
  GPSONE_BIT_EVENT_CLOSE_RESULT,       /* event to report result for close */
  GPSONE_BIT_EVENT_CONNECT_RESULT,     /* event to report result for connect */
  GPSONE_BIT_EVENT_DISCONNECT_RESULT,  /* event to report result for disconnect */
  GPSONE_BIT_EVENT_SEND_RESULT,        /* event to report result for send */
  GPSONE_BIT_EVENT_IOCTL_RESULT,       /* event to report result for ioctl */
  GPSONE_BIT_EVENT_DATA_READY,         /* event to notify PDSM of new data ready */
  GPSONE_BIT_EVENT_NETWORK_STATUS,     /* event to notify PDSM of network status change, reserved for future use */
}gpsone_bit_event_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_ADDR_FAMILY_ENUM_TYPE

DESCRIPTION
  enum which defines the IP addresses family type
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_IP_V4  = 1,                    /* Protocol Family - Internet v4 */
  GPSONE_BIT_IP_V6  = 2                     /* Protocol Family - Internet v6 */
} gpsone_bit_addr_family_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_IP_ADDR_TYPE

DESCRIPTION
  structure which defines the IP addresses that BIT API supports
---------------------------------------------------------------------------*/
typedef struct
{
  gpsone_bit_addr_family_e_type  type;

  union
  {
    uint32  v4_addr;
    /*~ IF (_DISC_ == GPSONE_BIT_IP_V4) gpsone_bit_ip_addr_type.addr.v4_addr*/
    uint8   v6_addr[16];
    /*~ IF (_DISC_ == GPSONE_BIT_IP_V6) gpsone_bit_ip_addr_type.addr.v6_addr */
    /*~ DEFAULT gpsone_bit_ip_addr_type.addr.void */
  } addr;
  /*~ FIELD gpsone_bit_ip_addr_type.addr DISC gpsone_bit_ip_addr_type.type*/
} gpsone_bit_ip_addr_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_EVENT_PAYLOAD_TYPE

DESCRIPTION
  structure which defines the event payload
---------------------------------------------------------------------------*/
typedef struct
{
  gpsone_bit_event_e_type       event;             /* event code */
  gpsone_bit_status_e_type      result;            /* result status code for the event */

  union
  {
    uint32                      bytes_available;   /* available_bytes for DATA_READY */
    /*~ CASE  GPSONE_BIT_EVENT_DATA_READY     gpsone_bit_event_payload_type.arg.bytes_available */
    uint32                      bytes_sent;        /* sent_bytes for SEND_RESULT */
    /*~ CASE  GPSONE_BIT_EVENT_SEND_RESULT    gpsone_bit_event_payload_type.arg.bytes_sent */
    gpsone_bit_ip_addr_type     ipaddr;            /* IP address for IOCTL_RESULT (only applicable for IP-related IOCTLs */
    /*~ CASE  GPSONE_BIT_EVENT_IOCTL_RESULT   gpsone_bit_event_payload_type.arg.ipaddr */
    /*~ DEFAULT gpsone_bit_event_payload_type.arg.void */
  } arg;
  /*~ FIELD gpsone_bit_event_payload_type.arg DISC gpsone_bit_event_payload_type.event */
} gpsone_bit_event_payload_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_OPEN_PARAM_TYPE

DESCRIPTION
  structure which is used by the gpsone_bit_open_f_type function
---------------------------------------------------------------------------*/
typedef struct
{
  boolean      force_connection_up;   /* force data connection to be up */
  /* more parameters can be added as needed for open */
} gpsone_bit_open_params_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_CLOSE_PARAM_TYPE

DESCRIPTION
  structure which used by the gpsone_bit_close_f_type functions
---------------------------------------------------------------------------*/
typedef struct
{
  boolean     modem_restarting;    /* TRUE if Modem is restarting */
  boolean     force_dormancy;      /* TRUE to force data connection into formancy */
  /* more parameters can be added as needed for close */
} gpsone_bit_close_params_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_IFACE_NAME_E_TYPE

DESCRIPTION
  enum which defines data connection interface name type
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_IFACE_NONE = -1,
  GPSONE_BIT_IFACE_ANY = 0,        /* any data connection interface */
  GPSONE_BIT_IFACE_WWAN,           /* Wireless WAN data connection interface */
  GPSONE_BIT_IFACE_WIFI,           /* 802.11 WiFi data connection interface */
  GPSONE_BIT_IFACE_WIRED,          /* Wired network data connection interface */
  GPSONE_BIT_IFACE_CDMA_SN,
  GPSONE_BIT_IFACE_CDMA_AN,
  GPSONE_BIT_IFACE_UMTS,
  GPSONE_BIT_IFACE_MAX
} gpsone_bit_iface_name_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_IFACE_PARAM_TYPE

DESCRIPTION
  structure which defines parameters for the data interface
---------------------------------------------------------------------------*/
#define  GPSONE_BIT_MAX_APN_STRING_LEN    100   /* Max length of APN string    */

typedef struct
{
  gpsone_bit_iface_name_e_type    iface_name;    /* Interface name */
  gpsone_bit_addr_family_e_type   iface_family;  /* Interface family */
  //gpsone_bit_ip_addr_type       iface_ip_addr; /* Interface IP address, IPv4 or IPv6 */
  //char                          apn_name[GPSONE_BIT_MAX_APN_STRING_LEN+1];  /* APN Name */
  // /*~ FIELD gpsone_bit_open_params_type.apn_name STRING 101 */
} gpsone_bit_iface_params_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_LINK_TYPE

DESCRIPTION
  enums for the connect link type (need to match PDCOMM's definition)
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_LINK_HTTP = 0,
  GPSONE_BIT_LINK_TCP,
  GPSONE_BIT_LINK_SMS,
  GPSONE_BIT_LINK_UDP,
  GPSONE_BIT_LINK_TYPE_MAX
} gpsone_bit_link_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_HOST_TYPE

DESCRIPTION
  enum which describes the fields in the structure
  gpsone_bit_connect_params_type.
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_IP_ADDR,
  GPSONE_BIT_HOST_NAME,
  GPSONE_BIT_HOST_NAME_AND_IP_ADDR,
} gpsone_bit_host_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_CONNECT_PARAM_TYPE

DESCRIPTION
  structure which is used by the gpsone_bit_connect_f_type functions
---------------------------------------------------------------------------*/
#define GPSONE_BIT_MAX_URL_STRING_LEN 256
/* this length must match pd_comms_tcp_connect_params_type's p_host_name[] length */

typedef struct
{
  gpsone_bit_protocol_e_type      protocol_type;  /* AGPS protocol */

  gpsone_bit_iface_params_type    iface_param;    /* interface parameters */

  gpsone_bit_link_e_type          link_type;

  gpsone_bit_host_e_type          adr_type;
  uint16                          ip_port;
  gpsone_bit_ip_addr_type         ip_addr;
  char                            host_name[GPSONE_BIT_MAX_URL_STRING_LEN];
  /*~ FIELD gpsone_bit_connect_params_type.host_name STRING 256 */

} gpsone_bit_connect_params_type;


/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_DISCONNECT_PARAM_TYPE

DESCRIPTION
  structure which used by the gpsone_bit_disconnect_f_type functions
---------------------------------------------------------------------------*/
typedef struct
{
  boolean       force_disconnect;     /* force disconnect immediately */
} gpsone_bit_disconnect_params_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_IOCTL_E_TYPE

DESCRIPTION
  enum which defines all the possible ioctl codes for BIT APIs
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_IOCTL_MIN = 0,                   /* not used */
  GPSONE_BIT_IOCTL_FORCE_DORMANCY,            /* force dormancy */
  GPSONE_BIT_IOCTL_UNFORCE_DORMANCY,          /* unforce dormancy */
  GPSONE_BIT_IOCTL_GET_LOCAL_IP_ADDR,         /* get local IP address */
}gpsone_bit_ioctl_e_type;

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_IOCTL_PARAM_TYPE

DESCRIPTION
  structure which used by the gpsone_bit_ioctl_f_type functions
---------------------------------------------------------------------------*/
typedef struct
{
  uint32 arg;       /* optional argument (no need to use variable length ioctl argument for BIT now) */
  uint32 reserved;  /* reserved for future use, should be set to 0*/
} gpsone_bit_ioctl_params_type;

/*===========================================================================

FUNCTION: gpsone_bit_open_f_type

DESCRIPTION:
  This function is used to inform BIT daemon that PDSM will start using its
  service. If for some reason, BIT daemon fails to initialize, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that BIT daemon is in the process of being
  brought up and open result will be sent asynchronously via gpsone_bit_notify
  with OPEN_RESULT event.

  If BIT daemon can finish the open operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
typedef    gpsone_bit_status_e_type (gpsone_bit_open_f_type)
(
  gpsone_bit_transport_handle_type    transport_handle,
  const gpsone_bit_open_params_type   *open_param
  /*~ PARAM open_param POINTER */
);
/*~ CALLBACK gpsone_bit_open_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION: gpsone_bit_close_f_type

DESCRIPTION:
  This function is used to inform BIT daemon that PDSM will stop using its
  service. If for some reason, BIT daemon fails to close, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that BIT daemon is in the process of
  being shut down and close result will be sent asynchronously via
  gpsone_bit_notify with CLOSE_RESULT event.

  If BIT daemon can finish the close operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
typedef    gpsone_bit_status_e_type (gpsone_bit_close_f_type)
(
  gpsone_bit_transport_handle_type     transport_handle,
  const gpsone_bit_close_params_type   *close_param
  /*~ PARAM close_param POINTER */
);
/*~ CALLBACK gpsone_bit_close_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION: gpsone_bit_connect_f_type

DESCRIPTION:
  This function is used to establish end-to-end connection to a network server.
  If for some reason, end-to-end connection can not be established, it should
  return GPSONE_BIT_STATUS_FAIL or other error code. therwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that end-to-end connection is in the
  process of being established. Connect result should be sent asynchronously
  via gpsone_bit_notify with CONNECT_RESULT event.

  If BIT daemon can finish the connect operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
typedef    gpsone_bit_status_e_type (gpsone_bit_connect_f_type)
(
  gpsone_bit_transport_handle_type       transport_handle,
  uint32                                 transaction_id,
  const gpsone_bit_connect_params_type   *connect_param
  /*~ PARAM connect_param POINTER */
);
/*~ CALLBACK gpsone_bit_connect_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION: gpsone_bit_disconnect_f_type

DESCRIPTION:
  This function is used to tear down end-to-end connection to a network server.
  If for some reason, end-to-end connection can not be torn down, it should
  return GPSONE_BIT_STATUS_FAIL. Otherwise, it should return GPSONE_BIT_STATUS_WAIT
  to inform PDSM that end-to-end connection is in the process of being torn down.
  Disconnect result should be sent asynchronously via gpsone_bit_notify with
  CONNECT_RESULT event.

  If BIT daemon can finish the disconnect operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
typedef    gpsone_bit_status_e_type (gpsone_bit_disconnect_f_type)
(
  gpsone_bit_transport_handle_type         transport_handle,
  gpsone_bit_session_handle_type           session_handle,
  uint32                                   transaction_id,
  const gpsone_bit_disconnect_params_type  *disconnect_param
  /*~ PARAM disconnect_param POINTER */
);
/*~ CALLBACK gpsone_bit_disconnect_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION: gpsone_bit_send_f_type

DESCRIPTION:
  This function is used by PDSM to send data to the network. It passes a pointer
  to a data buffer (*send_buf) and also specifies the size (in bytes) of the data buffer.
  If for some reason, BIT daemon is not able to process the send request,
  it should return GPSONE_BIT_STATUS_FAIL or other error code to inform PDSM that the
  send request has failed. Otherwise, it should return GPSONE_BIT_STATUS_WAIT and return
  result asynchronously via gpsone_bit_notify() with SEND_RESULT event.

  If BIT daemon can finish the send operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
typedef    gpsone_bit_status_e_type (gpsone_bit_send_f_type)
(
  gpsone_bit_transport_handle_type  transport_handle,
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  uint8                             *send_buf,
 /*~ PARAM send_buf VARRAY GPSONE_BIT_MAX_BUF_SIZE LENGTH length */
  uint32                            length
);
/*~ CALLBACK gpsone_bit_send_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION: gpsone_bit_recv_f_type

DESCRIPTION:
  This function is used by PDSM to receive data from BIT daemon, when it gets
  DATA_READY event from BIT daemon. Upon getting recv request, BIT daemon should
  copy incoming data to the data buffer pointed by *recv_buf. If incoming data
  size is larger than max_buff_size, it should only copy max_buf_size to the
  data buffer pointed by *recv_buf.

  BIT daemon should return the number of bytes copied to the data buffer and the
  bytes left in its data buffer. BIT daemon should return GPSONE_BIT_STATUS_SUCCESS
  when some bytes are succesfully returned, otherwise an error code should be returned.
  If no data available now, BIT daemon should return GPSONE_BIT_STATUS_NO_MORE_DATA.

  Please note that this is a synchronous call, there is no async event for recv(),
  BIT daemon should handle the recv operation in a non-blocking way.
===========================================================================*/
typedef gpsone_bit_status_e_type (gpsone_bit_recv_f_type)
(
  gpsone_bit_transport_handle_type   transport_handle,
  gpsone_bit_session_handle_type     session_handle,
  uint8                              *recv_buf,
  /*~ PARAM OUT recv_buf VARRAY GPSONE_BIT_MAX_BUF_SIZE LENGTH max_buf_size */
  uint32                             max_buf_size,
  uint32                             *bytes_returned,
  /*~ PARAM OUT bytes_returned POINTER */
  uint32                             *bytes_leftover
  /*~ PARAM OUT bytes_leftover POINTER */
);
/*~ CALLBACK gpsone_bit_recv_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION: pgpsone_bit_ioctl_f_type

DESCRIPTION:
  This function is used by PDSM to request ioctl operation on BIT daemon.
  If for some reason, BIT daemon can't do this ioctl operation, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that the ioctl operation is in progress.
  Result will be sent back asynchronously via gpsone_bit_notify() later with
  IOCTL_RESULT event.
===========================================================================*/
typedef gpsone_bit_status_e_type (gpsone_bit_ioctl_f_type)
(
 gpsone_bit_transport_handle_type     transport_handle,
 gpsone_bit_session_handle_type       session_handle,
 uint32                               transaction_id,
 gpsone_bit_ioctl_e_type              ioctl_request,
 const gpsone_bit_ioctl_params_type   *ioctl_param
  /*~ PARAM ioctl_param POINTER */
);
/*~ CALLBACK gpsone_bit_ioctl_f_type
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_REGISTER_PARAMS_TYPE

DESCRIPTION
  Parameters passing in to gpsone_bit_register (except callback functions)
---------------------------------------------------------------------------*/
#define  GPSONE_BIT_MAX_NAME_STRING_LEN    16

typedef struct
{
  char                 bit_name[GPSONE_BIT_MAX_NAME_STRING_LEN+1];
  /*~ FIELD gpsone_bit_register_params_type.bit_name STRING 17 */
  gpsone_bit_e_type    bit_type;
} gpsone_bit_register_params_type;

/*===========================================================================

FUNCTION gpsone_bit_register

DESCRIPTION
  This function is called by BIT daemon to register with PDCOMM TCP Task
  (callback functions need to be top-level parameters due to ONCRPC)

DEPENDENCIES

RETURN VALUE
  GPSONE_BIT_STATUS_SUCCESS if registration is successful, otherwise Error Code

SIDE EFFECTS

===========================================================================*/
extern gpsone_bit_status_e_type gpsone_bit_register
(
  const gpsone_bit_register_params_type     *reg_param,
    /*~ PARAM reg_param POINTER */
  gpsone_bit_transport_handle_type          *transport_handle,
    /*~ PARAM OUT transport_handle POINTER */
  gpsone_bit_open_f_type                    *open_fp,
   /*~ PARAM open_fp POINTER */
  gpsone_bit_close_f_type                   *close_fp,
   /*~ PARAM close_fp POINTER */
  gpsone_bit_connect_f_type                 *connect_fp,
   /*~ PARAM connect_fp POINTER */
  gpsone_bit_disconnect_f_type              *disconnect_fp,
   /*~ PARAM disconnect_fp POINTER */
  gpsone_bit_send_f_type                    *send_fp,
   /*~ PARAM send_fp POINTER */
  gpsone_bit_recv_f_type                    *recv_fp,
   /*~ PARAM recv_fp POINTER */
  gpsone_bit_ioctl_f_type                   *ioctl_fp
   /*~ PARAM ioctl_fp POINTER */
);
/*~ FUNCTION gpsone_bit_register
    RELEASE_FUNC gpsone_bit_deregister( *transport_handle)
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION gpsone_bit_deregister

DESCRIPTION
  This function is called by BIT daemon to deregister with PDCOMMS TCP task

DEPENDENCIES

RETURN VALUE
  GPSONE_BIT_STATUS_SUCCESS if deregistration is successful, otherwise Error Code

SIDE EFFECTS

===========================================================================*/
extern  gpsone_bit_status_e_type gpsone_bit_deregister
(
  gpsone_bit_transport_handle_type     transport_handle
);
/*~ FUNCTION gpsone_bit_deregister
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

/*===========================================================================

FUNCTION gpsone_bit_notify

DESCRIPTION
  This function is called by BIT daemon to notify PDCOMM TCP task about
  asynchronous events and command results

DEPENDENCIES

RETURN VALUE
  GPSONE_BIT_STATUS_SUCCESS if PDSM handles the notification succesfully,
  otherwise Error Code

SIDE EFFECTS

===========================================================================*/
extern gpsone_bit_status_e_type  gpsone_bit_notify
(
  gpsone_bit_transport_handle_type       transport_handle,
  gpsone_bit_session_handle_type         session_handle,    /* CONNECT_RESULT will pass in new handle here */
  uint32                                 transaction_id,
  const gpsone_bit_event_payload_type    *event_payload
  /*~ PARAM event_payload POINTER */
);
/*~ FUNCTION gpsone_bit_notify
    ONERROR return GPSONE_BIT_STATUS_FAIL
*/

#endif /* GPSONE_BIT_API_H */

#endif /* ! GPSONE_BIT_API_H_trimmed */
