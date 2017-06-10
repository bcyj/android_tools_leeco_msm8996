/******************************************************************************

                        N E T M G R _ S T U B S . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_stubs.h
  @brief   Network Manager test stubs Header File

  DESCRIPTION
  Header file for NetMgr utility functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/10/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_STUBS_H__
#define __NETMGR_STUBS_H__

#include <linux/sockios.h>
#include <netinet/in.h>
#include <stdint.h>

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define MTU_IPV6 1280
#define MTU_IPV4 1500
#define MTU      MTU_IPV4

/* Present in Android builds */
#define SIOCKILLADDR    0x8939

/* Resolve header usage conflict */
struct in6_ifreq {
  struct in6_addr ifr6_addr;
  uint32_t        ifr6_prefixlen;
  int             ifr6_ifindex;
};

int netmgr_stub_ioctl(int fd, int flag, struct ifreq *ifr);


#endif /* __NETMGR_STUBS_H__ */
