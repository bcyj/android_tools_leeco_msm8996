/******************************************************************************

                        D S _ T R A C E . C

******************************************************************************/

/******************************************************************************

  @file    ds_trace.c
  @brief   dsutils Ftrace Functions Implementation File

  DESCRIPTION
  Implementation file for dsutils Ftrace functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
05/20/14   sr         Initial version

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ds_util.h"

#ifdef FEATURE_DS_TRACE_ON

#define MAX_BUFFER_SIZE 256
#define TRACE_MARKER_LOCATION "/tracing/trace_marker"

static int trace_marker_fd = -1;

static const char *ds_get_debugfs_location(void)
{
  static char debugfs_location[MAX_BUFFER_SIZE];
  char fs_type[MAX_BUFFER_SIZE];
  FILE *file_dsc;
  static int is_found;

  if (!is_found) {
    file_dsc = fopen("/proc/mounts","r");
    if (file_dsc == NULL)
      return NULL;

    while (fscanf(file_dsc, "%*s %255s %255s %*s %*d %*d\n",
                  debugfs_location, fs_type) == 2) {
      if (strcmp(fs_type, "debugfs") == 0) {
        is_found = 1;
        break;
      }
    }
    fclose(file_dsc);
  }

  return is_found ? debugfs_location : NULL;
}

void ds_open_trace_marker_file_desc()
{
  char path[MAX_BUFFER_SIZE];
  const char *debugfs_location;

  if (trace_marker_fd > 0)
    return;

  debugfs_location = ds_get_debugfs_location();
  if (debugfs_location) {
    strlcpy(path, debugfs_location, strlen(debugfs_location)+1);
    strlcat(path, TRACE_MARKER_LOCATION, MAX_BUFFER_SIZE);
    trace_marker_fd = open(path, O_WRONLY);
    if (trace_marker_fd <= 0)
      ds_log_err("Failed to open trace_marker");
  }
  else
    ds_log_err("Failed to find debugfs");
}

void ds_trace_marker_write(const char *format, ...)
{
  va_list args;
  char buffer[MAX_BUFFER_SIZE];
  int size = 0;

  if (trace_marker_fd > 0) {
    va_start(args, format);
    size = vsnprintf(buffer, MAX_BUFFER_SIZE, format, args);
    va_end(args);

    write(trace_marker_fd, buffer, size);
  }
}

#else  /* FEATURE_DS_TRACE_ON */

void ds_trace_marker_write(const char *format, ...)
{
}

#endif  /* FEATURE_DS_TRACE_ON */
