/***********************************************************************
 * tftp_server_folders.h
 *
 * TFTP server folder management module.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * TFTP server folder management module.
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
2013-12-17   nr    Create

===========================================================================*/

#ifndef __TFTP_SERVER_FOLDERS_H__
#define __TFTP_SERVER_FOLDERS_H__

#include "tftp_server_config.h"
#include "tftp_comdef.h"

int tftp_server_folders_init (void);

const char* tftp_server_folders_lookup_path_prefix (uint32 instance_id);

int tftp_server_folders_check_if_shared_file (const char *path);

#endif /* not __TFTP_SERVER_FOLDERS_H__ */
