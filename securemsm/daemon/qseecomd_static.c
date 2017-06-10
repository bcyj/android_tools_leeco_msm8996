/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        HLOS Static QSEECom Daemon

GENERAL DESCRIPTION
	Static QSEECom Daemon starts the listeners that are used for interacting
with QSEECom in recovery mode

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
#include <sys/capability.h>
#include <utils/Log.h>
#include <QSEEComAPI.h>
#include <common_log.h>
#include <sys/prctl.h>
#include <qseecomd_static.h>

#ifdef ANDROID_CHANGES
#include <android/log.h>
#include <private/android_filesystem_config.h>
#else
#define AID_SYSTEM 1000
#endif

/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "QSEECOMD_STATIC: "
#endif

#define LOG_NDDEBUG 0 //Define to enable LOGD

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#define LOG_NDEBUG  0 //Define to enable LOGV
#endif

/* List of services id's */
#define FILE_SERVICE 10
#define TIME_SERVICE 11
#define RPMB_SERVICE 0x2000
#define SSD_SERVICE  0x3000
#define SECURE_UI_SERVICE  0x4000
#define ML_SERVICE 0x5000 //Reserving the number here to avoid future conflicts. The listener will be started from the ML daemon.

/* End of list */

#define RPMB_LISTENER_INDEX 0 /* value dependent on struct 'qseecom_listener_services' */
#define SSD_LISTENER_INDEX 1 /* value dependent on struct 'qseecom_listener_services' */
#define SEC_UI_LISTENER_INDEX 2 /* value dependent on struct 'qseecom_listener_services' */

enum qseecomd_hierarchy {
	QSEECOMD_PARENT_PROCESS,
	QSEECOMD_CHILD_PROCESS
};

struct qseecom_listener_services {
	char *service_name;
	int  id;
	char *file_name;
	char *file_start;
	char *file_stop;
	void *qhandle;
	char *file_init;
};

static struct qseecom_listener_services listeners[] = {

	 /* Change the value of RPMB_LISTENER_INDEX incase you need to change this order */
        {
		.service_name = "RPMB system services",
		.id = RPMB_SERVICE,
		.file_name = "librpmb.so",
		.file_start = "rpmb_start",
		.file_stop = "rpmb_close",
		.qhandle = NULL,
		.file_init = "rpmb_init_service",
	},

	 /* Change the value of SSD_LISTENER_INDEX incase you need to change this order */
	{
		.service_name = "SSD system services",
		.id = SSD_SERVICE,
		.file_name = "libssd.so",
		.file_start = "ssd_start",
		.file_stop = "ssd_close",
		.qhandle = NULL,
		.file_init = "ssd_init_service",
	},

};

static int qseecomd_update_cap(void)
{
	int retvalue = 0;
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap;

	memset(&header, 0, sizeof(header));
	memset(&cap, 0, sizeof(cap));
	prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
	retvalue =  setgid(AID_SYSTEM);
	if(retvalue != 0){
		fprintf(stderr, "setgid error \n");
		return retvalue;
	}
	retvalue = setuid(AID_SYSTEM);
	if(retvalue != 0){
		fprintf(stderr, "setuid error \n");
		return retvalue;
	}
	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = 0;
	cap.effective = (1 << CAP_SYS_RAWIO) | (1 << CAP_SYS_ADMIN) |
				(1 << CAP_NET_RAW);
	cap.permitted = cap.effective;
	cap.inheritable = 0;
	capset(&header, &cap);
	return 0;
}

/* @brief This function calls all the starting functions of the listeners.
*/

int start_listener(int index, enum qseecomd_hierarchy hierarchy)
{
	int ret = 0;
	long (*service_initialize)(int) = NULL;

	fprintf(stderr,"Listener: index = %d, hierarchy = %d \n",index,hierarchy);

	if (hierarchy == QSEECOMD_PARENT_PROCESS) {
		switch (index) {
			case RPMB_LISTENER_INDEX:
				ret = rpmb_init_service();
				break;
			case SSD_LISTENER_INDEX:
				ret = ssd_init_service();
				break;
			default:
				fprintf(stderr, "invalid listener id %d \n", index);
				return -1;
		}
		if (ret == 0){
				fprintf(stderr, "Init %s OK \n",
					listeners[index].file_init);
		} else {
				fprintf(stderr, "Init %s fail \n",
					listeners[index].file_init);
				return -1;
		}
	} else {

		switch (index) {
			case RPMB_LISTENER_INDEX:
				ret = rpmb_start(RPMB_SERVICE);
				break;
			case SSD_LISTENER_INDEX:
				ret = ssd_start(SSD_SERVICE);
				break;
			default:
				LOGD("invalid listener id %d \n", index);
				return -1;
		}
		if (ret == 0){
				fprintf(stderr, "Init %s OK \n",
					listeners[index].file_init);
		} else {
				fprintf(stderr, "Init %s fail \n",
					listeners[index].file_init);
				return -1;
		}
	}

	return ret;

}

void start_listener_services(void)
{
	int ret = 0;
	int num_listeners = 0;
	int listener_index = 0;

	/* Start the listener services */
	num_listeners = sizeof(listeners)/sizeof(struct qseecom_listener_services);
	fprintf(stderr,"Total listener services to start = %d \n", num_listeners);
	for (listener_index = 0; listener_index < num_listeners; listener_index++)
		ret += start_listener(listener_index, QSEECOMD_CHILD_PROCESS);
}

int main() {
	int ret = 0;
	int status = 0;

	fprintf(stderr,"qseecom listener services process entry PPID = %d \n", getppid());

	ret = start_listener(RPMB_LISTENER_INDEX, QSEECOMD_PARENT_PROCESS);
	if(ret) {
		fprintf(stderr,"ERROR: RPMB_INIT failed, shall not start listener services \n");
		return -1;
	}
	fprintf(stderr,"qseecom listener services RPMB_INIT ok \n");

	ret = start_listener(SSD_LISTENER_INDEX, QSEECOMD_PARENT_PROCESS);
	if(ret) {
		fprintf(stderr,"ERROR: SSD_INIT failed, shall not start listener services \n");
		return -1;
	}
	fprintf(stderr,"qseecom listener services SSD_INIT ok \n");

	if(0 != qseecomd_update_cap()){
		fprintf(stderr,"qseecomd permission reset failed, not starting services \n");
		return -1;
	}
	fprintf(stderr,"qseecomd permission reset ok, to start services \n");

	ret = fork();
	if (ret < 0) {
		fprintf(stderr,"Failed to create qseecom daemon!! \n");
		exit(1);
	}
	fprintf(stderr,"Create qseecom daemon!! \n");

	if (ret > 0) {
		fprintf(stderr,"Parent qseecom daemon process paused!! \n");
		ret = wait(&status);
		return -1;
	}
	fprintf(stderr,"Parent qseecom daemon process!! \n");

	/* listener services (qceecom daemon) continues */
	fprintf(stderr,"qseecom listener service threads starting!!! \n");
	start_listener_services();
	fprintf(stderr,"QSEECOM DAEMON RUNNING \n");
	pause();
	fprintf(stderr,"qseecom listener services process exiting!!! \n");
	return -1;
}

#ifdef __cplusplus
}
#endif
