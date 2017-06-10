/***********************************************************************
 * tftp_server_config.h
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

#ifndef __TFTP_SERVER_CONFIG_H__
#define __TFTP_SERVER_CONFIG_H__

#include "tftp_config_i.h"

#ifndef TFTP_SERVER_MAX_CLIENTS
  #define TFTP_SERVER_MAX_CLIENTS    100
#endif

#ifndef TFTP_SERVER_LAST_ACK_ADDITIONAL_TIMEOUT_MS
  #define TFTP_SERVER_LAST_ACK_ADDITIONAL_TIMEOUT_MS    10
#endif

#ifndef TFTP_SERVER_ROOT_DIRECTORY
  #define TFTP_SERVER_ROOT_DIRECTORY    "."
#endif

#ifndef TFTP_SERVER_UMASK
  #define TFTP_SERVER_UMASK   0007 /*file perms rw-rw---- dir rwxrwx---*/
#endif

#ifndef TFTP_MAJOR_VERSION
  #define TFTP_MAJOR_VERSION  1
#endif

#ifndef TFTP_MINOR_VERSION
  #define TFTP_MINOR_VERSION  0
#endif

#endif /* __TFTP_SERVER_CONFIG_H__ */
