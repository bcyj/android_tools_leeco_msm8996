/***********************************************************************
 * tftp_config_i.h
 *
 * Short description
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
2014-01-05   rp    In logs do not print full-paths just file-name portion.
2014-12-30   dks   Fixes to config and log modules.
2014-07-28   rp    Move log-buffer from global to stack.
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-20   dks   Configure message printing.
2013-12-05   nr    Separate the socket type from the build flavour.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_CONFIG_I_H__
#define __TFTP_CONFIG_I_H__

#include "tftp_config.h"

#if defined (FEATURE_TFTP_SERVER_BUILD)
  #if defined (TFTP_LA_BUILD)
    #define TFTP_USE_IPCR_LA_SOCKETS
    #define TFTP_USE_POSIX_THREADS
    #define TFTP_SERVER_NO_FOLDER_ABSTRACTION
  #elif defined (TFTP_WINDOWS_BUILD)
    #define TFTP_USE_IPCR_WINDOWS_SOCKETS
    #define TFTP_USE_WINDOWS_THREADS
    #define TFTP_SERVER_NO_FOLDER_ABSTRACTION
  #elif defined (TFTP_NHLOS_BUILD)
    #define TFTP_USE_IPCR_MODEM_SOCKETS
    #define TFTP_USE_REX_THREADS
  #else
    #error "Server-build : unknown config"
  #endif
#elif defined (FEATURE_TFTP_CLIENT_BUILD)
  #if defined (TFTP_NHLOS_BUILD)
    #define TFTP_USE_IPCR_MODEM_SOCKETS
    #define TFTP_USE_REX_THREADS
  #else
    #error "Client-build : unknown config"
  #endif
#else
  #error "Configure build for either Client or Server"
#endif

#if defined (__FILENAME__)
  #define TFTP_SOURCE_FILE_NAME __FILENAME__
#else
  #define TFTP_SOURCE_FILE_NAME __FILE__
#endif

#endif /* not __TFTP_CONFIG_I_H__ */
