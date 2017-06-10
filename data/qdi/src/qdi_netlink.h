/*!
  @file
  qdi_netlink.h

  @brief
  Provides ability to obtain the IP address(es) for an interface by interfacing
  with the kernel via netlink sockets

*/

/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/01/11   sg      inital version

===========================================================================*/

#ifndef _QDI_NETLINK_H_
#define _QDI_NETLINK_H_

#include "dsi_netctrl.h"

/*===========================================================================
  FUNCTION  qdi_nl_get_addr_info
===========================================================================*/
/*!
@brief
  This function returns the IP address of the given IP family for the given
  interface

@param
  iface     - Interface on which IP address is being requested
  ip_family - IP family of the address to return
  addr      - Return the corresponding address
  mask      - Return the corresponding mask

@return
  QDI_SUCCESS - If successful
  QDI_FAILURE - Otherwise

*/
/*=========================================================================*/

int qdi_nl_get_ip_addr_info
(
  const char              *iface,
  int                     ip_family,
  dsi_addr_info_t         *addr_info,
  qmi_wds_iface_name_type tech_name
);

#endif /* _QDI_NETLINK_H_ */
