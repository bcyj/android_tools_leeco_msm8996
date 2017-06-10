/******************************************************************************
  @file:  xtra_linux.h
  @brief: xtra linux layer include

  DESCRIPTION

  XTRA Daemon

  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#ifndef _XTRA_LINUX_H_
#define _XTRA_LINUX_H_
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>


#ifdef USE_GLIB
#define strlcpy g_strlcpy
#endif

#ifndef XTRA_DEFAULT_CFG_PATH
#define XTRA_DEFAULT_CFG_PATH "/etc/xtraconf.bin"
#endif

#define XTRA_PID_FILE "/var/run/xtrad.pid"

#endif

