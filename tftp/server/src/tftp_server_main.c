/***********************************************************************
 * tftp_server_main.c
 *
 * The main entry fucntion for the TFTP server.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The main entry fucntion for the TFTP server.
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
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-14   rp    Create

===========================================================================*/

#include "tftp_server_config.h"
#include "tftp_server.h"
#include "tftp_os.h"

#ifndef TFTP_SIMULATOR_BUILD
int MAIN_TYPE
main (int argc, const char *argv[])
{
  return tftp_server (argc, argv);
}
#endif
