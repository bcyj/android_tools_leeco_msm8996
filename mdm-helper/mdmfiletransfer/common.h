/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  common.h : Interface to commonly used functions.
 *
 */
#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef WINDOWSPC
#include <Windows.h>
#else
#include <termios.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
//#include "kickstart_log.h"
//#include "kickstart_utils.h"

#define MAX_FILE_NAME_LENGTH 256

typedef unsigned char byte;

#ifndef WINDOWSPC
typedef enum {
    FALSE,
    TRUE
} boolean;
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/*Enum containing the two supported protocol types */
enum protocol_type {
    DLOAD_PROTOCOL,
    SAHARA_PROTOCOL,
    STREAMING_PROTOCOL
};

extern const char* path_to_save_files;

FILE* open_file (const char *filename, boolean for_reading);
boolean close_file (FILE *fp);

#if defined(LINUXPC) || defined(WINDOWSPC)
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
#endif

#endif
