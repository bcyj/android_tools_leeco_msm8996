/***********************************************************************
 * tftp_server_utils.h
 *
 * Utils functions for the server.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Utils functions for the server.
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

#ifndef __TFTP_SERVER_UTILS_H__
#define __TFTP_SERVER_UTILS_H__

#include "tftp_server_config.h"
#include "tftp_comdef.h"


int32 tftp_server_utils_normalize_path (char *raw_filepath, uint32 instance_id,
                                         char *normalized_filepath_buf,
                                         uint32 normalized_filepath_buf_len);

#endif /* not __TFTP_SERVER_UTILS_H__ */
