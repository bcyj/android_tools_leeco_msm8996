/******************************************************************************
 *
 * 	MP-CTL-D.C
 *
 * GENERAL DESCRIPTION
 * 	PerfLock Control Daemon

 Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern int  mpctl_server_init(void);
extern void mpctl_server_exit(void);

#define LOG_TAG           "ANDR-PERF-MPCTL-D"
#include <cutils/log.h>
#if defined(ANDROID_JELLYBEAN)
#include "common_log.h"
#endif

#define QLOGE(...)    ALOGE(__VA_ARGS__)
#define QLOGW(...)    ALOGW(__VA_ARGS__)
#define QLOGI(...)    ALOGI(__VA_ARGS__)
#define QLOGV(...)    ALOGV(__VA_ARGS__)

/* Main entry to mp-ctl-d
   * argc - number of command-line arguments
   * argv - command-line argument array
   Return value - 0 if successful, negative otherwise */
int main(int argc, char *argv[])
{
	mpctl_server_init();

	mpctl_server_exit();
	return 0;
}
