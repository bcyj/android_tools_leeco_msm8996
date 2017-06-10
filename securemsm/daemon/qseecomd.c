/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        HLOS QSEECom Daemon

GENERAL DESCRIPTION
	QSEECom Daemon starts the listeners that are used for interacting
with QSEECom.

EXTERNALIZED FUNCTIONS
	None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
#include <cutils/properties.h>

#ifdef ANDROID_CHANGES
#include <android/log.h>
#include <private/android_filesystem_config.h>
#else
#define AID_SYSTEM 1000
#endif

/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "QSEECOMD: "
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
#define FWLOCK_SERVICE 0x6000

/* End of list */

#define RPMB_LISTENER_INDEX 0 /* value dependent on struct 'qseecom_listener_services' */
#define SSD_LISTENER_INDEX 1 /* value dependent on struct 'qseecom_listener_services' */
#define SEC_UI_LISTENER_INDEX 2 /* value dependent on struct 'qseecom_listener_services' */
#define FWLOCK_LISTENER_INDEX 5 /* value dependent on struct 'qseecom_listener_services' */

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
	 /* Change the value of SEC_UI_LISTENER_INDEX incase you need to change this order */
	{
		.service_name = "secure UI services",
		.id = SECURE_UI_SERVICE,
		.file_name = "libsecureui.so",
		.file_start = "secure_ui_start",
		.file_stop = "secure_ui_close",
		.qhandle = NULL,
		.file_init = "secure_ui_init",
	},

	{
		.service_name = "time services",
		.id = TIME_SERVICE,
		.file_name = "libdrmtime.so",
		.file_start = "atime_start",
		.file_stop = "atime_close",
		.qhandle = NULL,
		.file_init = "",
	},
	{
		.service_name = "file system services",
		.id = FILE_SERVICE,
		.file_name = "libdrmfs.so",
		.file_start = "fs_start",
		.file_stop = "fs_close",
		.qhandle = NULL,
		.file_init = "",
	},
	{
		.service_name = "firmware lock services",
		.id = FWLOCK_SERVICE,
		.file_name = "libfwlock.so",
		.file_start = "fwlock_lib_start",
		.file_stop = "fwlock_lib_close",
		.qhandle = NULL,
		.file_init = "fwlock_lib_init",
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
		LOGE("setgid error");
		return retvalue;
	}
	retvalue = setuid(AID_SYSTEM);
	if(retvalue != 0){
		LOGE("setuid error");
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

	LOGE("Listener: index = %d, hierarchy = %d",index,hierarchy);
	listeners[index].qhandle  = dlopen(listeners[index].file_name, RTLD_NOW);
	if (listeners[index].qhandle) {
		LOGD("Init dlopen(%s, RTLD_NOW) succeeds",
					listeners[index].file_name);
		/* call rpmb_init_service or ssd_init_service or secure_ui_service, only if the call is from parent*/
		if (((listeners[index].id == RPMB_SERVICE) ||
			(listeners[index].id == SSD_SERVICE) ||
			(listeners[index].id == FWLOCK_SERVICE) ||
			(listeners[index].id == SECURE_UI_SERVICE))&&
			(hierarchy == QSEECOMD_PARENT_PROCESS)) {
			*(void **)(&service_initialize)=
						dlsym(listeners[index].qhandle,
							listeners[index].file_init);
			if (service_initialize) {
				LOGD("Init::Init dlsym(g_FSHandle %s) succeeds",
							listeners[index].file_init);
				ret = (*service_initialize)(listeners[index].id);
				LOGD("Init %s ret = %d", listeners[index].file_init, ret);
				if (ret == 0){
					LOGD("%s: init (Listener ID = %d)",
						listeners[index].service_name, listeners[index].id );
				}
				else {
					LOGE("Init dlsym(g_FSHandle, %s) fail",
							listeners[index].file_init);
				}
			} else {
				LOGE("file_init handle failed");
				ret = -1;
			}
			return ret;
		}
		/* Normal file_start for all services */
		*(void **)(&service_initialize)=
					dlsym(listeners[index].qhandle,
						listeners[index].file_start);
		if (service_initialize) {
			LOGD("Init::Init dlsym(g_FSHandle %s) succeeds",
						listeners[index].file_start);
			ret = (*service_initialize)(listeners[index].id);
			LOGD("Init %s ret = %d", listeners[index].file_start, ret);
			if (ret == 0){
				LOGD("%s: Started (Listener ID = %d)",
					listeners[index].service_name, listeners[index].id );
			}
			else
				LOGE("Init dlsym(g_FSHandle, %s) fail",
						listeners[index].file_start);
		} else {
			LOGE("Init dlopen(%s, RLTD_NOW) is failed....",
						listeners[index].file_name);
			ret = -1;
		}
	} else {
		LOGE("Init dlopen(%s, RLTD_NOW) is failed....",
					listeners[index].file_name);
		ret = -1;
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
	LOGD("Total listener services to start = %d", num_listeners);
	for (listener_index = 0; listener_index < num_listeners; listener_index++)
		ret += start_listener(listener_index, QSEECOMD_CHILD_PROCESS);
}

int main() {
	int ret = 0;
	int status = 0;
	struct QSEECom_handle *keymaster_handle = NULL;
	LOGD("qseecom listener services process entry PPID = %d", getppid());

	ret = start_listener(RPMB_LISTENER_INDEX, QSEECOMD_PARENT_PROCESS);
	if(ret) {
		LOGE("ERROR: RPMB_INIT failed, shall not start listener services");
		return -1;
	}

	ret = start_listener(SSD_LISTENER_INDEX, QSEECOMD_PARENT_PROCESS);
	if(ret) {
		LOGE("ERROR: SSD_INIT failed, shall not start listener services");
		return -1;
	}

	ret = start_listener(SEC_UI_LISTENER_INDEX, QSEECOMD_PARENT_PROCESS);
	if(ret) {
		LOGD("SEC_UI_INIT failed, shall not start secure ui listener");
	}

	ret = start_listener(FWLOCK_LISTENER_INDEX, QSEECOMD_PARENT_PROCESS);
	if(ret) {
		LOGD("ERROR: FWLOCK_INIT failed, shall not start fwlock listener");
	}

	if(0 != qseecomd_update_cap()){
		LOGE("qseecomd permission reset failed, not starting services");
		return -1;
	}

	ret = fork();
	if (ret < 0) {
		LOGE("Failed to create qseecom daemon!! ");
		exit(1);
	}

	if (ret > 0) {
		LOGD("Parent qseecom daemon process paused!! ");
		ret = wait(&status);
		return -1;
	}

	/* listener services (qceecom daemon) continues */
	LOGD("qseecom listener service threads starting!!! ");
	start_listener_services();
	property_set("sys.listeners.registered", "true");

	ret = QSEECom_start_app(&keymaster_handle, "/firmware/image", "keymaste", 1024);
	if (ret)
		LOGE("Loading keymaster app failied");
	else
		property_set("sys.keymaster.loaded", "true");

	LOGD("QSEECOM DAEMON RUNNING ");
	pause();
	LOGD("qseecom listener services process exiting!!! ");
	ret = QSEECom_shutdown_app(&keymaster_handle);
	if (ret) {
		LOGE("Shutdown app failed with ret = %d", ret);
		printf("%s: Shutdown app failed with ret = %d",__func__,ret);
	}
	return -1;
}

#ifdef __cplusplus
}
#endif
