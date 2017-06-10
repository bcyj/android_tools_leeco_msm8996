/*
 * dsbsd.cpp
 *
 * @brief The Stream Services mobile BSD socket API file. Contains basic API
 * functions for socket manipulation.
 *
 * EXTERNALIZED FUNCTIONS
 *
 *   inet_addr()
 *     Takes a string containing a dotted-decimal IP address and returns the
 *     equivalent IP address in binary form.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/Network/BSD/main/latest/src/dsbsd.cpp#9 $
$DateTime: 2012/03/20 08:56:06 $
$Change: 2284745 $

========================================================================== */

/*===========================================================================

                           INCLUDE FILES FOR MODULE

===========================================================================*/

/* DS BSD headers */
#include "dsbsd.h"
#include "SourceMemDebug.h"
/* other utils */
#include "qtv_msg.h"
/*===========================================================================

                             FORWARD DECLARATIONS

===========================================================================*/

/*===========================================================================

                    DEFINITIONS AND VARIABLE DECLARATIONS

===========================================================================*/
//const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;                /* :: */
//const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;     /* ::1 */

/*---------------------------------------------------------------------------
                      Transport Error Condition Values
---------------------------------------------------------------------------*/
int     EEOF           =0;
int     EMSGTRUNC      =0;
int     ENETWOULDBLOCK =0;
int     ENETINPROGRESS =0;
/*---------------------------------------------------------------------------
                           Network Subsystem Errors
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                          DNS Error Condition Values
---------------------------------------------------------------------------*/
int     ETRYAGAIN      =0;

/*===========================================================================

                         EXTERNAL FUNCTION DEFINITIONS

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
)
{
  int32 retval = STREAMNET_FAILED;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  if (src == NULL || dst == NULL)
  {
    QTV_MSG_PRIO2( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                   "inet_pton(): Called with src %p, dst %p", src, dst );
    return STREAMNET_FAILED;
  }
  if(af)
  {
    int32 retval_tmp = ::inet_pton(af, src, dst);
    if(retval_tmp == 1)
    {
      retval = STREAMNET_SUCCESS;
    }
  }

  return retval;
} /* inet_pton() */
