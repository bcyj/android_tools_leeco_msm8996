/***********************************************************************
 * tftp_file.h
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

#ifndef __TFTP_FILE_H__
#define __TFTP_FILE_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

typedef void* tftp_file_handle;

int32 tftp_file_open (tftp_file_handle *handle, const char *path,
                     const char *mode);

int32 tftp_file_open_stream (tftp_file_handle *handle, int32 fd,
                             const char *mode);

int32 tftp_file_read(tftp_file_handle handle, void *buffer, int32 buffer_size);

int32 tftp_file_write (tftp_file_handle handle, const void *buffer,
                     int32 buffer_size);

int32 tftp_file_seek (tftp_file_handle handle, int32 offset, int whence);

int32 tftp_file_close (tftp_file_handle handle);

int32 tftp_file_get_file_size (const char *path);


#endif /* not __TFTP_FILE_H__ */
