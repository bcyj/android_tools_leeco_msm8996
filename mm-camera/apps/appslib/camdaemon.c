/* Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#ifdef _ANDROID_
#include <cutils/log.h>
#endif
#include <dlfcn.h>

#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"

/*===========================================================================
 * FUNCTION    - main -
 *
 * DESCRIPTION:
 *==========================================================================*/
int main(int argc, char **argv)
{
  if(qcamsvr_start() < 0){
	  CDBG("%s ERROR\n", __func__);
	  return -1;
  }
  return 0;
}

