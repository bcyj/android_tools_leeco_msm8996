/***********************************************************************
 * tftp_connection_addr.h
 *
 * Short description
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose description.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-06-04   rp    Create

===========================================================================*/

#ifndef __TFTP_CONNECTION_ADDR_H__
#define __TFTP_CONNECTION_ADDR_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

enum tftp_connection_ipcr_addr_type
{
  TFTP_CONNECTION_IPCR_ADDR_TYPE_NAME,
  TFTP_CONNECTION_IPCR_ADDR_TYPE_PORT
};

struct tftp_connection_ipcr_name_addr
{
  uint32 service_id;
  uint32 instance_id;
};

struct tftp_connection_ipcr_port_addr
{
  uint32 proc_id;
  uint32 port_id;
};

struct tftp_connection_ipcr_addr
{
  enum tftp_connection_ipcr_addr_type addr_type;
  union
  {
    struct tftp_connection_ipcr_name_addr name_addr;
    struct tftp_connection_ipcr_port_addr port_addr;
  }u;
};

#endif /* not __TFTP_CONNECTION_ADDR_H__ */
