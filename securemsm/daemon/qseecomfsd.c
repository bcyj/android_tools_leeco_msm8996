/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        HLOS QSEECom File System Daemon

GENERAL DESCRIPTION
	QSEECom Daemon starts the file system listener(which is built in as a static library).
File system service is used for interacting with QSEECom.

EXTERNALIZED FUNCTIONS
	None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <QSEEComAPI.h>
#include <common_log.h>
#include <linux/capability.h>
#include <sys/prctl.h>
#include <android/log.h>

/* List of services id's */
#define FILE_SERVICE 10

int fs_start(int);

int main() {
	int ret = 0;
	int status = 0;
	LOGD("qseecom_fs listener services process entry PPID = %d", getppid());

	ret = fork();
	if (ret < 0) {
		LOGE("Failed to create qseecom file service daemon!! ");
		exit(1);
	}

	if (ret > 0) {
		LOGD("Parent qseecom file service daemon process paused!! ");
		ret = wait(&status);
		return -1;
	}

	/* listener services (qceecom daemon) continues */
	LOGE("qseecom fs listener service threads starting!!! ");
	ret = fs_start(FILE_SERVICE);
	if (ret == 0) {
		LOGD("Started file service listener (static)");
	}else {
		LOGE("Could not start file service");
	}

	LOGD("QSEECOM FS DAEMON RUNNING ");
	pause();
	LOGD("qseecom fs listener services process exiting!!! ");
	return -1;
}

#ifdef __cplusplus
}
#endif
