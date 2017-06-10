#ifndef _QMMI_IP_NETLINK_H_
#define _QMMI_IP_NETLINK_H_

/******************************************************************************

                        QMI_IP_NETLINK.H

******************************************************************************/

/******************************************************************************

  @file    qmi_ip_netlink.h
  @brief   Qualcomm mapping interface over IP netlink handler

  DESCRIPTION


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
08/30/13   tw         Initial version

******************************************************************************/
#include <openssl/ssl.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "netmgr.h"

#define USB_TETHERED_SMD_CH    "/dev/smdcntl8"

typedef enum
{
  QMI_IP_LINK_UP_EVENT =1,
  QMI_IP_LINK_DOWN_EVENT
} qmi_ip_event_e;

/*=============================================================================
                        EXTERNAL FUNCTION DECLARATION
==============================================================================*/

extern int qmi_ip_process_link_event
(
  qmi_ip_event_e    event,
  int               mode
);

#endif
