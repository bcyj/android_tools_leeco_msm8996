/***********************************************************************
 * tftp_os.c
 *
 * The TFTP OS Abstraction layer.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP OS Abstraction layer.
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
2014-01-20   dks   Fix compilation on windows.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-21   nr    Abstract the OS layer across the OS's.
2013-11-14   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_os.h"
#include "tftp_string.h"
#include "tftp_malloc.h"

#include <stdio.h>

int32
tftp_os_auto_mkdir (const char *path, uint16 mode, int32 gid)
{
  int result = 0;
  int index = 0;
  int path_len = 0;
  char *temp_path_buf;
  uint32 temp_path_buf_size;

  if (path == NULL)
  {
    return -EINVAL;
  }

  path_len = strlen (path);

  if (path_len == 0)
  {
    return -EINVAL;
  }

  temp_path_buf_size = path_len + 1;
  temp_path_buf = tftp_malloc (temp_path_buf_size);

  if (temp_path_buf == NULL)
  {
    return -ENOMEM;
  }

  tftp_strlcpy (temp_path_buf, path, temp_path_buf_size);
  /* Set index to path_len - 1 as there is at least one char in the path.
   * and then go backwards in the path and try to create the directories.
   * We exit when the path runs out or we create one directory successfully or
   * if an error is encountered.
   */

  for (index = path_len - 1; index > 0; index--)
  {
    if (path[index] == '/')
    {
      temp_path_buf[index] = 0x00;
      /* Attempt to make subdirectories */
      result = tftp_os_mkdir (temp_path_buf, mode);

      /* Account for broken symbolic links. */
      if (result == -EEXIST)
      {
        result = 0;
      }

      /* Exit if we are successful in creating a directory. */
      if (result == 0)
      {
        tftp_os_chgrp(temp_path_buf, gid);
        break;
      }

      /* Exit if we encounter an error other than ENOENT. ENOENT implies the
       * parent directory of the directory we tried to create doesn't exist so
       * we try to create that directory. Any other error means we must exit.
       */
      else if (result != -ENOENT)
      {
        break;
      }
    }
  }

  if (result == 0)
  {
    /* If the current index was a '/' put it back. */
    if (path[index] == '/')
    {
      temp_path_buf[index++] = '/';
    }

    for (/* Current index value */; index < path_len; index++)
    {
      /* Go forward up the path until we reach the end */
      if (path[index] == '/')
      {
        /* Attempt to make subdirectories */
        result = tftp_os_mkdir (temp_path_buf, mode);

        /* If we fail to create a directory and it doesn't exist we exit .*/
        if (result != 0)
        {
          if (result == -EEXIST)
          {
            result = 0; /* The directory exists we can proceed */
          }
          else
          {
            break;
          }
        }
        else
        {
          tftp_os_chgrp(temp_path_buf, gid);
        }
        temp_path_buf[index] = '/';
      }
    }
  }

  tftp_free (temp_path_buf);
  return result;
}

