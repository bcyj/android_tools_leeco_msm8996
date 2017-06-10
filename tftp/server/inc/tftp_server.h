/***********************************************************************
 * tftp_server.h
 *
 * Short description.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose Description
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-06-04   rp    Switch to IPCRouter sockets.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_SERVER_H__
#define __TFTP_SERVER_H__

#include "tftp_server_config.h"

int tftp_server (int argc, const char *argv[]);



#endif /* __TFTP_SERVER_H__ */
