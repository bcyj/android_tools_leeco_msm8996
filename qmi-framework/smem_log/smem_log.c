/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 Copyright (c) 2011, 2013-2014 Qualcomm Technologies, Inc.
 All Rights Reserved.  Qualcomm Technologies Proprietary and Confidential.
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#include <fcntl.h>
#include <stdio.h>
#include "smem_log.h"

static int smem_log_fd;

/*===========================================================================
FUNCTION: smem_log_exit

DESCRIPTION:
   Opens up the smem_log_fd.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   -1 Fail, 0 Pass

SIDE EFFECTS: On success, sets smem_log_fd to the open fd
===========================================================================*/
int smem_log_init(void)
{
  int fd;
  fd = open("/dev/smem_log", O_RDWR);
  if (fd < 0)
  {
    perror("open");
    return -1;
  }
  ioctl(fd, SMIOC_SETMODE, SMIOC_BINARY);

  smem_log_fd = fd;
  return 0;
}

/*===========================================================================
FUNCTION: smem_log_exit

DESCRIPTION:
   Cleans up the smem_log_fd.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   None

SIDE EFFECTS: closes smem_log_fd and sets it to 0
===========================================================================*/
void smem_log_exit(void)
{
  if(smem_log_fd > 0) {
    close(smem_log_fd);
    smem_log_fd = 0;
  }
}

/*===========================================================================
FUNCTION: smem_log_get_fd

DESCRIPTION:
   An acessor for smem_log_fd so the #defined macros don't reference this
   directly.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   smem_log_fd

SIDE EFFECTS:
===========================================================================*/
int smem_log_get_fd(void)
{
  return smem_log_fd;
}

/*===========================================================================
 * End of module
 *=========================================================================*/
