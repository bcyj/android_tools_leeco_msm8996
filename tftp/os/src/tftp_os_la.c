/***********************************************************************
 * tftp_os_la.c
 *
 * Short description.
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_comdef.h"
#include "tftp_file.h"
#include "tftp_server_config.h"
#include "tftp_os.h"


#if !defined (TFTP_LA_BUILD)
  #error "This file is to be included only in LA builds."
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void
tftp_os_init (void)
{
  umask (TFTP_SERVER_UMASK);
}

int32
tftp_os_open_file (const char *path, int32 oflags, uint16 mode, int32 group)
{
  int32 result, fd;

  /* This mode when used is modified by umask */
  result = open (path, oflags, mode);

  if (result < 0)
  {
    result = -(errno);
  }
  else
  {
    if (group != -1)
    {
      fd = result;
      fchown(fd, -1, group);
    }
  }

  return result;
}

int32
tftp_os_mkdir (const char *path, uint16 mode)
{
  int32 result;

  result = mkdir (path, mode); /* This mode when used is modified by umask */

  if (result != 0)
  {
    result = -(errno);
  }

  return result;
}

int32
tftp_os_chgrp (const char *path, int32 group)
{
  int32 result = 0;

  if (group != -1)
  {
    result = chown(path, -1, group);
  }

  if (result != 0)
  {
    result = -(errno);
  }

  return result;
}

int32
tftp_os_symlink (const char *symlink_path, const char *target_path,
                 int is_dir)
{
  int32 result;

  (void) is_dir;
  result = symlink (target_path, symlink_path);

  if (result != 0)
  {
    result = -(errno);
  }
  return result;
}

int32 tftp_os_unlink (const char *path)
{
  int32 result;

  result = unlink (path);

  if (result != 0)
  {
    result = -(errno);
  }
  return result;
}

int32
tftp_os_datasync_for_fd (int32 fd)
{
  int32 result = 0;

  result = fdatasync(fd);

  if (result != 0)
  {
    result = -(errno);
  }
  return result;
}

int32
tftp_os_datasync (const char *path)
{
  int32 result = 0;
  int fd;

  fd = open(path, O_RDONLY);

  if (fd == -1)
  {
    return -(errno);
  }

  result = fdatasync(fd);

  if (result != 0)
  {
    close(fd);
    return -(errno);
  }

  close(fd);
  return result;
}

int32
tftp_os_rename (const char *oldpath, const char *newpath)
{
  int32 result = 0;

  result = tftp_os_datasync (oldpath);
  if (result != 0)
  {
    return result;
  }

  result = rename(oldpath, newpath);

  if (result != 0)
  {
    result = -(errno);
  }
  return result;
}


