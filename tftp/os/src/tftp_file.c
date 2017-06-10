/***********************************************************************
 * tftp_file.c
 *
 * HLOS File IO Abstraction.
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2015-01-06   dks   Compile TFTP server on TN Apps.
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Create

===========================================================================*/


#include "tftp_config_i.h"
#include "tftp_file.h"
#include "tftp_assert.h"
#include "tftp_malloc.h"
#include "tftp_string.h"
#include "tftp_os.h"
#include <stdio.h>


int32
tftp_file_open (tftp_file_handle *handle, const char *path, const char *mode)
{
  int32 result;

  FILE *file_handle;

  file_handle = fopen (path, mode);

  if(file_handle == NULL)
  {
    *handle = NULL;
    result = -(errno);
  }
  else
  {
    *handle = (void *) file_handle;
    result = 0;
  }

  return result;
}

int32
tftp_file_open_stream (tftp_file_handle *handle, int32 fd, const char *mode)
{
   int32 result;

  FILE *file_handle;

  file_handle = fdopen (fd, mode);

  if(file_handle == NULL)
  {
    *handle = NULL;
    result = -(errno);
  }
  else
  {
    *handle = (void *) file_handle;
    result = 0;
  }

  return result;
}

int32
tftp_file_read (tftp_file_handle handle, void *buffer, int32 buffer_size)
{
  int32 return_val;
  size_t remaining_bytes, read_bytes, result;
  FILE *file_handle = (FILE *) handle;
  uint8 *data_buffer = (uint8 *) buffer;
  int is_eof, is_read_error;

  TFTP_ASSERT (file_handle != NULL);
  if (file_handle == NULL)
  {
    return -EINVAL;
  }

  if (buffer_size > INT32_MAX)
  {
    return_val = -(EFBIG);
    return return_val;
  }

  if(feof (file_handle))
  {
    return 0;
  }

  read_bytes = 0;
  remaining_bytes = buffer_size;
  while (remaining_bytes > 0)
  {
    result = fread (data_buffer, 1, buffer_size, file_handle);
    if (result == 0)
    {
      is_eof = feof (file_handle);
      if (is_eof)
      {
        break;
      }

      is_read_error = ferror (file_handle);
      if (is_read_error)
      {
        return (-errno);
      }
    }
    data_buffer += result;
    read_bytes += result;
    remaining_bytes -= result;
  }

  return_val = (int32) read_bytes;
  return return_val;
}

int32
tftp_file_write(tftp_file_handle handle, const void *buffer, int32 buffer_size)
{
  int32 return_val;
  size_t remaining_bytes, written_bytes, result;
  FILE *file_handle = (FILE *) handle;
  uint8 *data_buffer = (uint8 *) buffer;
  uint8 *buffer_end;
  int is_write_failed;

  TFTP_ASSERT (file_handle != NULL);
  if (file_handle == NULL)
  {
    return -EINVAL;
  }

  TFTP_ASSERT (buffer_size >= 0);
  if (buffer_size == 0)
  {
    return 0;
  }

  TFTP_ASSERT (buffer != NULL);
  if ((buffer == NULL) ||
      (buffer_size < 0))
  {
    return -EINVAL;
  }

  if (buffer_size > INT32_MAX)
  {
    return -EFBIG;
  }

  if(feof (file_handle))
  {
    return 0;
  }

  buffer_end = (uint8 *) buffer + buffer_size;
  written_bytes = 0;
  remaining_bytes = buffer_size;
  while (remaining_bytes > 0)
  {
    TFTP_ASSERT (data_buffer + remaining_bytes <= buffer_end);
    result = fwrite (data_buffer, 1, remaining_bytes, file_handle);
    if (result == 0)
    {
      is_write_failed = ferror (file_handle);
      TFTP_DEBUG_ASSERT (is_write_failed == 0);
      if (is_write_failed)
      {
        return -(errno);
      }
      break;
    }
    data_buffer += result;
    written_bytes += result;
    remaining_bytes -= result;
  }

  return_val = (int32) written_bytes;
  return return_val;
}

int32
tftp_file_seek (tftp_file_handle handle, int32 offset, int whence)
{
  int32 result;
  FILE *file_handle = (FILE *) handle;

  if (file_handle == NULL)
  {
    return -EINVAL;
  }

  result = (int32) fseek (file_handle, (long)offset, whence);

  if (result != 0)
  {
    result = -(errno);
  }
  return result;
}

int32
tftp_file_close (tftp_file_handle handle)
{
  int32 result, fd;
  FILE *file_handle = (FILE *) handle;

  if (file_handle == NULL)
  {
    return -EINVAL;
  }

  result = (int32) fflush (file_handle);

  if (result != 0)
  {
    result = -(errno);
    fclose (file_handle);
    return result;
  }

  fd = fileno(file_handle);

  tftp_os_datasync_for_fd (fd);

  result = (int32) fclose (file_handle);

  if (result != 0)
  {
    result = -(errno);
  }

  return result;
}

int32
tftp_file_get_file_size (const char *path)
{
  FILE *file_handle;
  long file_size;
  int32 result;

  file_handle = fopen (path, "rb"); /* Open in read mode */
  if(file_handle == NULL)
  {
    result = -(errno);
    return result;
  }

  result = fseek (file_handle, 0, SEEK_END); // Seek to end of file
  if (result != 0)
  {
    result = -(errno);
    goto cleanup;
  }

  file_size = ftell (file_handle);
  if (file_size == -1)
  {
    result = -(errno);
    goto cleanup;
  }

  if (file_size > INT32_MAX)
  {
    result = -(EFBIG);
    goto cleanup;
  }
  result  = (int32) file_size;

cleanup:
    fclose (file_handle);
    return result;
}

