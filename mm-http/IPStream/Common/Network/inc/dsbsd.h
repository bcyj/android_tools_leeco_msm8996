#ifndef DSBSD_H
#define DSBSD_H
/*
 * dsbsd.h
 *
 * @brief Contains dtatatypes used by the netwrok interfaces
 *
 * EXTERNALIZED FUNCTIONS
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/Network/BSD/main/latest/inc/dsbsd.h#11 $
$DateTime: 2012/03/28 02:59:52 $
$Change: 2302973 $

========================================================================== */

/*===========================================================================

                           INCLUDE FILES FOR MODULE

===========================================================================*/
/*QNX Socket APIs*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>/*For hostent*/
#include <sys/select.h>/*for select and fd_set related calls*/
#include <sys/ioctl.h> /*for ioctl*/
#include <arpa/inet.h>/*Inet related*/
#include <netinet/tcp.h>
#include <errno.h>
#include "AEEStdDef.h"
#include "string.h"


/*===========================================================================
                          MACROS AND TYPE DEFINITIONS
===========================================================================*/


/*---------------------------------------------------------------------------
                      Transport Error Condition Values
---------------------------------------------------------------------------*/
extern int     EEOF;           /* End of file                   */
extern int     EMSGTRUNC;      /* TODO : message truncated     */
extern int     ENETWOULDBLOCK;
extern int     ENETINPROGRESS;

/*---------------------------------------------------------------------------
                          DNS Error Condition Values
---------------------------------------------------------------------------*/
extern int     ETRYAGAIN;     /* Temporary error*/
extern int     ENORECOVERY;     /* Irrecoverable error occurred   */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                        DSBSD return codes
---------------------------------------------------------------------------*/
#define STREAMNET_SUCCESS      0
#define STREAMNET_FAILED      -1

/* Max sockets per applications */
#define STREAMNET_MAX_SOCKS 15

/*---------------------------------------------------------------------------
  Macros used for accessing bits in a variable of type fd_set.
---------------------------------------------------------------------------*/

#define FD_RESET FD_CLR



/*---------------------------------------------------------------------------
                         Type/structure definitions
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                           DNS-relate structure(s)
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                             Socket types
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                          Address Family
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
               Internet-family specific host internet address
---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------
TYPEDEF IP_ADDR_ENUM_TYPE

DESCRIPTION
  An enum that defines all of the address types supported - used to
  discriminate the union below.

  NOTE: The values are chosen to easy debugging.
---------------------------------------------------------------------------*/
typedef enum
{
  IP_ANY_ADDR     = 0,
  IPV4_ADDR       = 4,
  IPV6_ADDR       = 6,
  IP_ADDR_INVALID = 255,
} ip_addr_enum_type;

typedef struct
{
  ip_addr_enum_type type;
  union
  {
    struct in_addr  v4;
    struct in6_addr v6;
  }addr;
} IpAddrType;


/*---------------------------------------------------------------------------
                  ioctl options on dsbsd sockets
---------------------------------------------------------------------------*/
//#define FIONBIO         100           /* request to change I/O behavior    */

/*---------------------------------------------------------------------------
                             Socket ids
---------------------------------------------------------------------------*/
typedef int    SOCKET;

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

/*---------------------------------------------------------------------------
                Supported Socket Option Names
---------------------------------------------------------------------------*/
/*Should we add include tcp.h header file in dsbsd.h */
//#define TCP_SACK           TCPOPT_SACK           /* Set SACK option                  *//*check in tcp.h file*/
//#define TCP_NODELAY        6           /* Disable Nagle's algorithm        *//*check in tcp.h file*/

/*---------------------------------------------------------------------------
              Supported Socket Option Levels
---------------------------------------------------------------------------*/
/* These are the standard values taken from virtually all BSD flavors.
 * They are based on RFC 790. Note that ps_in.h defines IPPROTO_IP as 4
 * which conflicts with the following definition. The problem is that 4 is
 * supposed to be mapped to CMCC Gateway Monitoring Message according to
 * the RFC, and the definition of IPROTO_IP is pretty much standardized to
 * 0. If a file includes this source file, we expect that it wants the
 * BSD definitions instead of the QC definitions. By using the standard
 * values, we allow the user to pass in well-known values which are not
 * based on the defintions below.
 */

#define SOL_SOCK           SOL_SOCKET           /* Socket                           */

/*---------------------------------------------------------------------------
          Socket Option Value for SO_LINGER
---------------------------------------------------------------------------*/
typedef linger so_linger_type;

/*---------------------------------------------------------------------------
  Destination is a multicast address if in the range of
  224.0.0.0 to 239.255.255.255

  i.e. if address starts with 0xE
---------------------------------------------------------------------------*/
#define IN_IS_ADDR_MULTICAST(v4)                                            \
  ((((unsigned long )(v4)) & htonl(0xF0000000)) == htonl(0xE0000000))

#define IN6_IS_ADDR_MULTICAST_NW(v6) (((uint8 *) (v6))[0] == 0xff)

#define IN6_IS_ADDR_V4MAPPED_NW(v6)                                             \
        ((((const unsigned int *) (v6))[0] == 0)                             \
        && (((const unsigned int *) (v6))[1] == 0)                          \
         && (((const unsigned int *) (v6))[2] == htonl (0xffff)))
#define IN6_IS_V4_MAPPED_V6_ADDR_MULTICAST(v6)                              \
            ( IN6_IS_ADDR_V4MAPPED_NW(v6) &&                                   \
              IN_IS_ADDR_MULTICAST(((int *)(v6))[3]) )

/* Mcast handle for each ioctl */
typedef int bsd_mcast_handle_type;


/*
 * @brief Macro that converts host-to-network long integer.
 *
 * @param[in] x unsigned long integer value to be converted.
 *
 * @return  The host byte-ordered value.
 */
#define htonll(x) \
        ((uint64)((((uint64)(x) & 0x00000000000000ffULL) << 56)   | \
                  (((uint64)(x) & 0x000000000000ff00ULL) << 40)   | \
                  (((uint64)(x) & 0x0000000000ff0000ULL) << 24)   | \
                  (((uint64)(x) & 0x00000000ff000000ULL) << 8)    | \
                  (((uint64)(x) & 0x000000ff00000000ULL) >> 8)    | \
                  (((uint64)(x) & 0x0000ff0000000000ULL) >> 24)   | \
                  (((uint64)(x) & 0x00ff000000000000ULL) >> 40)   | \
                  (((uint64)(x) & 0xff00000000000000ULL) >> 56)))

/*
 * @brief Macro that network-to-host long integer.
 *
 * @param[in] x unsigned long integer value to be converte.
 *
 * @return  The host byte-ordered value.
 */
#define ntohll(x) \
        ((uint64)((((uint64)(x) & 0x00000000000000ffULL) << 56)   | \
                  (((uint64)(x) & 0x000000000000ff00ULL) << 40)   | \
                  (((uint64)(x) & 0x0000000000ff0000ULL) << 24)   | \
                  (((uint64)(x) & 0x00000000ff000000ULL) << 8)    | \
                  (((uint64)(x) & 0x000000ff00000000ULL) >> 8)    | \
                  (((uint64)(x) & 0x0000ff0000000000ULL) >> 24)   | \
                  (((uint64)(x) & 0x00ff000000000000ULL) >> 40)   | \
                  (((uint64)(x) & 0xff00000000000000ULL) >> 56)))

typedef int bsd_net_policy_info_type;

typedef int bsd_qos_spec_type;
typedef int bsd_qos_handle_type;
typedef int bsd_iface_ioctl_qos_get_flow_spec_type;
typedef int bsd_ip_filter_type;

typedef int bsd_data_bearer_rate;
typedef int bsd_bearer_tech_type;
typedef int bsd_mcast_handle_type;

typedef int bsd_iface_id_type;
typedef int bsd_iface_ioctl_event_info_union_type;
typedef int bsd_iface_ioctl_event_enum_type;

/*===========================================================================

                         PUBLIC FUNCTION DECLARATIONS

===========================================================================*/


/*
 * This function is called with a presentation (printable or ASCII) format
 * address to be converted to its network address (binary) format.  The af
 * argument can be either AF_INET if the address to be converted is an IPv4
 * address or AF_INET6 if the address is an IPv6 address.  In case of error
 * the error code is returned in the dss_errno argument.
 *
 * The dst argument should have sufficient memory for the network address
 * of the appropriate family.  For IPv4 it should be at least
 * sizeof(struct in_addr) while for IPv6 it should be at least
 * sizeof(struct in6_addr).
 */
int32 inet_pton
(
  int32       af,        /* Address family of address in src argument      */
  const char *src,       /* String containing presentation form IP address */
  void *dst     /* Memory for returning address in network format */
);


#endif /* DSBSD_H */
