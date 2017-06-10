/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
 QSEECom Safty Verification Suite
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <utils/Log.h>
#include "tzcommon_entry.h"
#include "QSEEComAPI.h"
#include "common_log.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


/* Size of PR license or challenge */

#define TZ_CM_MAX_NAME_LEN          256   /* Fixed. Don't increase the size of TZ_CM_MAX_NAME_LEN*/
#define TZ_CM_MAX_DATA_LEN          20000
#define TZ_CM_PROV_PKG_SIZE         10240  //TZ_CM_PROV_PKG_SIZE must be smaller than TZ_CM_MAX_DATA_LEN

#define TZCOMMON_CREATE_CMD(x)  (SVC_TZCOMMMON_ID | x)

#define CLIENT_CMD25_RUN_SAFE_TEST  (25)

#define MAX_APP_NAME 25

/**
  provision check result for QSEE APPs
 */
typedef enum
{
	PROVCHECK_PROVISIONED = 1,
	PROVCHECK_NOT_PROVISIONED,
	PROVCHECK_NOT_REQUIRED,
	PROVCHECK_INVAID //the provisioning status.  (note: 'invalid' means something wrong when the tz-app tries to get provisioning status.)
} qsapps_prov_check_t;

/* the data structure that the HLOS app sends to DRM TZ-APP to query its provisioning status. */
typedef struct _ta_prov_request {
	tz_common_cmd_type cmd_id;
} ta_prov_request_t;

/* the data structure that the DRM TZ-APP returns to HLOS app for its provisioning status. */
typedef struct _ta_prov_reply {
	/** First 4 bytes are always command id */
	tz_common_cmd_type       cmd_id;
	/** provisioning check result */
	qsapps_prov_check_t      status;
	/** additional data; reserved for future */
	uint8                     resultVector[64];   //reserved
	/** messages for provisioning check */
	uint8                    resultLogs[TZ_CM_MAX_DATA_LEN];
	// provision_t status;  // the provisioning status.  (note: 'invalid' means something wrong when the tz-app tries to get provisioning status.)
	// int  status;  // the provisioning status.  (note: 'invalid' means something wrong when the tz-app tries to get provisioning status.)
	// int  version;               // the software version number of this tz-app.
} ta_prov_reply_t;

typedef struct _safe_drm_request {
	tz_common_cmd_type cmd_id;
} safe_drm_request_t;

typedef struct _safe_drm_reply {
	tz_common_cmd_type  cmd_id;
	qsapps_prov_check_t status;
	int		    version;
} safe_drm_reply_t;

struct qsc_send_cmd {
	uint32_t cmd_id;
	uint32_t data;
	uint32_t data2;
	uint32_t len;
	uint32_t start_pkt;
	uint32_t end_pkt;
	uint32_t test_buf_size;
};

struct qsc_send_cmd_rsp {
	uint32_t data;
	int32_t status;
};

static char bridge_app_name[MAX_APP_NAME] = "sampleapp";

static long drm_prov_open_services(struct QSEECom_handle **p_qsee_cmd_handle, const char *drm_prog_name)
{

	int ret = 0;

	do {
		if ( *p_qsee_cmd_handle == NULL ) {
			ret = QSEECom_start_app(p_qsee_cmd_handle, "/system/etc/firmware", drm_prog_name, 100*1024);
			if ( ret != 0 )
				ret = QSEECom_start_app(p_qsee_cmd_handle, "/firmware/image", drm_prog_name, 100*1024);
			if ( ret ) {
				LOGE("\t Error: QSEECom_start_app device is failed! \n");
				LOGE("\t Error: The program exits !\n");
				printf("Error: QSEECom_start_app device is failed! \n");
				ret = -1;
				break;
			}
		} else
			LOGD("QSEECom_start_app already opened");

		ret = QSEECom_set_bandwidth(*p_qsee_cmd_handle, true);
		if ( ret != 0 ) {
			LOGE("Error: QSEECom_set_bandwidth(true) returned=%d", ret);
			printf("Error: QSEECom_set_bandwidth(true) returned=%d", ret);
			ret = -1;
			break;
		}
	} while (0);

	return ret;
}

static long drm_prov_close_services(struct QSEECom_handle** QSEEComHandle)
{

	int32_t ret = 0;

	LOGD("Closing services starts! ");

	do {
		if (*QSEEComHandle != NULL) {
			ret = QSEECom_set_bandwidth(*QSEEComHandle, false);
			if (ret != 0) {
				LOGE("Error: QSEECom_set_bandwidth(false) returned=%u", ret);
				ret = -1;
				break;
			}

			ret = QSEECom_shutdown_app(QSEEComHandle);
			if (ret)
				LOGE("pkwpr::playready_close_services tzcom ioctl ABORT_REQ failed: %u", ret);
		}
	} while(0);

	*QSEEComHandle = NULL;

	LOGD("Closing services ends! ");

	return ret;
}

int32_t safe_issue_cmd(struct QSEECom_handle   *l_QSEEComHandle,
	                     safe_drm_request_t      *send_cmd,
	                     safe_drm_reply_t        *cmd_rsp)
{
	int32_t ret = 0;
	int32_t req_len = 0;
	int32_t rsp_len = 0;
	int     i;
	ta_prov_request_t *msgreq;      /* request data sent to QSEE */
	ta_prov_reply_t   *msgrsp;  /* response data sent from QSEE */
	tz_qsappsver_get_ver_req_t *ms_ver_req;
	tz_qsappsver_get_ver_rsp_t *ms_ver_rsp;

	LOGD("send prov cmd: start");
	/* populate the data in shared buffer */
	msgreq = (ta_prov_request_t *)l_QSEEComHandle->ion_sbuffer;
	msgreq->cmd_id = send_cmd->cmd_id;
	req_len = sizeof(ta_prov_request_t);
	rsp_len = sizeof(ta_prov_reply_t);

	if (req_len & QSEECOM_ALIGN_MASK)
		req_len = QSEECOM_ALIGN(req_len);

	if (rsp_len & QSEECOM_ALIGN_MASK)
		rsp_len = QSEECOM_ALIGN(rsp_len);

	msgrsp=(ta_prov_reply_t *)l_QSEEComHandle->ion_sbuffer + req_len;

	LOGD("req len = %d bytes",req_len);
	LOGD("rsp len = %d bytes",rsp_len);
	/* send request from HLOS to QSEApp */
	ret = QSEECom_send_cmd(l_QSEEComHandle,
				msgreq,
				req_len,
				msgrsp,
				rsp_len);
	if (ret) {
		LOGE("send command failed with ret = %d", ret);
		printf("%s: Send command failed with ret = %d\n",__func__,ret);
		return ret;
	}
	LOGD("return status = %x", msgrsp->status);
	cmd_rsp->status  = msgrsp->status;

	/* populate the data in shared buffer */
	ms_ver_req = (tz_qsappsver_get_ver_req_t *)l_QSEEComHandle->ion_sbuffer;
	ms_ver_req->cmd_id = TZ_CM_CMD_VERSION_GET_VER;
	req_len = sizeof(tz_qsappsver_get_ver_req_t);
	rsp_len = sizeof(tz_qsappsver_get_ver_rsp_t);

	if (req_len & QSEECOM_ALIGN_MASK)
		req_len = QSEECOM_ALIGN(req_len);

	if (rsp_len & QSEECOM_ALIGN_MASK)
		rsp_len = QSEECOM_ALIGN(rsp_len);

	ms_ver_rsp = (tz_qsappsver_get_ver_rsp_t *)l_QSEEComHandle->ion_sbuffer + req_len;

	LOGD("req len = %d bytes",req_len);
	LOGD("rsp len = %d bytes",rsp_len);
	/* send request from HLOS to QSEApp */
	ret = QSEECom_send_cmd(l_QSEEComHandle,
				ms_ver_req,
				req_len,
				ms_ver_rsp,
				rsp_len);
	if (ret) {
		LOGE("send command failed with ret = %d", ret);
		printf("%s: Send command failed with ret = %d\n",__func__,ret);
		return ret;
	}
	LOGD("return version = %d", ms_ver_rsp->version);
	cmd_rsp->version  = ms_ver_rsp->version;

	return ret;
}

int32_t query_drm_status(struct QSEECom_handle   *l_QSEEComHandle, const char *drm_prog_name, int *status)
{
	int32_t ret = 0;
	safe_drm_request_t     send_cmd;
	safe_drm_reply_t       cmd_rsp;

	send_cmd.cmd_id = TZ_CM_CMD_PROV_CHECK;

	ret = safe_issue_cmd(l_QSEEComHandle, &send_cmd, &cmd_rsp);

	switch (cmd_rsp.status) {
		case PROVCHECK_PROVISIONED:
			printf("Service %s is provisioned, version %d\n", drm_prog_name, cmd_rsp.version);
			break;
		case PROVCHECK_NOT_PROVISIONED:
			printf("Service %s is NOT provisioned, version %d\n", drm_prog_name, cmd_rsp.version);
			(*status) ++;
			break;
		case PROVCHECK_NOT_REQUIRED:
			printf("Service %s provisioning is NOT REQUIRED, version %d\n", drm_prog_name, cmd_rsp.version);
			break;
		case PROVCHECK_INVAID:
			printf("Get error during provisioning query to service %s\n", drm_prog_name);
			break;
		default:
			printf("Get error from safe_issue_cmd. status = %d\n", cmd_rsp.status);
			break;
	}

	return ret;
}

int probe_drm_provisioning(const char *drm_prog_name, int *status)
{
	int ret = 0;
	struct QSEECom_handle *qsee_cmd_handle = NULL;

	ret = drm_prov_open_services(&qsee_cmd_handle, drm_prog_name);
	if (0 == ret)
		ret = query_drm_status(qsee_cmd_handle, drm_prog_name, status);
	else
		printf("service %s open failed\n", drm_prog_name);
	drm_prov_close_services(&qsee_cmd_handle);

	return ret;
}

/**@brief:  Implement simple application start
 *
 * @param[in/out]       handle.
 * @param[in]           appname.
 * @param[in]           buffer size.
 * @return      zero on success or error count on failure.
 */
int32_t qsc_start_app(struct QSEECom_handle **l_QSEEComHandle,
			const char *appname,
			int32_t buf_size)
{
	int32_t ret = 0;

	/* start the application */
	ret = QSEECom_start_app(l_QSEEComHandle,
				"/system/etc/firmware",
				appname,
				buf_size);
	if (ret) {
		LOGE("Loading app -%s failed",appname);
		printf("%s: Loading app -%s failed\n",__func__,appname);
	} else {
		LOGD("Loading app -%s succeded",appname);
		printf("Loading app -%s succeded",appname);
	}
	return ret;
}

/**@brief:  Implement simple shutdown app
 * @param[in]   handle.
 * @return      zero on success or error count on failure.
 */
int32_t qsc_shutdown_app(struct QSEECom_handle **l_QSEEComHandle)
{
	int32_t ret = 0;

	LOGD("qsc_shutdown_app: start");
	/* shutdown the application */
	if (*l_QSEEComHandle != NULL) {
		ret = QSEECom_shutdown_app(l_QSEEComHandle);
		if (ret) {
			LOGE("Shutdown app failed with ret = %d", ret);
			printf("%s: Shutdown app failed with ret = %d\n",__func__,ret);
		} else
			LOGD("shutdown app: pass");
	} else {
		LOGE("cannot shutdown as the handle is NULL");
		printf("%s:cannot shutdown as the handle is NULL\n",__func__);
	}
	return ret;
}

/**@brief:  Exercise send command
 * @param[in]   handle.
 * @param[in]   data to be send to secure app.
 * @return      zero on success or error count on failure.
 */
int32_t qsc_issue_send_cmd_req(struct QSEECom_handle   *l_QSEEComHandle,
	                             struct qsc_send_cmd     *send_cmd,
	                             struct qsc_send_cmd_rsp *send_cmd_rsp)
{
	int32_t ret = 0;
	int32_t req_len = 0;
	int32_t rsp_len = 0;
	struct qsc_send_cmd *msgreq;    /* request data sent to QSEE */
	struct qsc_send_cmd_rsp *msgrsp;        /* response data sent from QSEE */

	LOGD("send modified cmd: start");
	/* populate the data in shared buffer */
	msgreq=(struct qsc_send_cmd *)l_QSEEComHandle->ion_sbuffer;
	msgreq->cmd_id = send_cmd->cmd_id;
	msgreq->data = 100;
	req_len = sizeof(struct qsc_send_cmd);
	rsp_len = sizeof(struct qsc_send_cmd_rsp);

	if (req_len & QSEECOM_ALIGN_MASK)
		req_len = QSEECOM_ALIGN(req_len);

	if (rsp_len & QSEECOM_ALIGN_MASK)
		rsp_len = QSEECOM_ALIGN(rsp_len);

	msgrsp=(struct qsc_send_cmd_rsp *)(l_QSEEComHandle->ion_sbuffer + req_len);

	LOGD("req data = %d",msgreq->data);
	LOGD("req len = %d bytes",req_len);
	LOGD("rsp len = %d bytes",rsp_len);
	/* send request from HLOS to QSEApp */
	ret = QSEECom_send_cmd(l_QSEEComHandle,
				msgreq,
				req_len,
				msgrsp,
				rsp_len);
	if (ret) {
		LOGE("send command failed with ret = %d\n", ret);
		printf("%s: Send command failed with ret = %d\n",__func__,ret);
	}
	LOGD("rsp data = %d", msgrsp->data);
	send_cmd_rsp->data = msgrsp->data;
	send_cmd_rsp->status = msgrsp->status;

	return ret;

}

int probe_secure_boot_setup(int *status)
{
	int ret = 0;
	struct qsc_send_cmd     send_cmd;
	struct qsc_send_cmd_rsp send_cmd_rsp;
	struct QSEECom_handle *l_QSEEComHandle = NULL;

	ret = qsc_start_app(&l_QSEEComHandle,bridge_app_name, 1024);
	if (ret) {
		LOGE("cannot load sample-app, so cannot detect secure boot information.\n");
		printf("cannot load sample-app, so cannot detect secure boot information.\n");
		return ret;
	}

	if (l_QSEEComHandle == NULL) {
		LOGE("Failed to get QSEECOM handle\n");
		printf("Failed to get QSEECOM handle\n");
		return -1;
	}

	send_cmd.cmd_id = CLIENT_CMD25_RUN_SAFE_TEST;
	ret = QSEECom_set_bandwidth(l_QSEEComHandle, true);

	ret = qsc_issue_send_cmd_req(l_QSEEComHandle, &send_cmd, &send_cmd_rsp);
	if (ret) {
		LOGE("send cmd with safe test:fail");
		printf("send cmd with safe test:fail\n");
	} else {
		if ((send_cmd_rsp.status & 0x1) == 0) {
			if ((send_cmd_rsp.status & 0x2) == 0) {
				LOGD("Secure boot is setup on this device\n");
				printf("Secure boot is setup on this device\n");
			} else {
				LOGD("Secure boot is NOT setup yet on this device\n");
				printf("Secure boot is NOT setup yet on this device\n");
				(*status) ++;
			}
			if ((send_cmd_rsp.status & 0x4) == 0) {
				LOGD("RPMB key is provisioned on this device\n");
				printf("RPMB key is provisioned on this device\n");
			} else {
				LOGD("RPMB key is NOT provisioned on this device\n");
				printf("RPMB key is NOT provisioned on this device\n");
				(*status) ++;
			}
		} else {
			LOGD("Secure boot detection failure.\n");
			printf("Secure boot detection failure.\n");
			(*status) ++;
		}
	}

	ret = QSEECom_set_bandwidth(l_QSEEComHandle, false);
	/* Shutdown the application */
	ret = qsc_shutdown_app(&l_QSEEComHandle);
	if (ret) {
		LOGE("   Failed to shutdown app: %d",ret);
		printf("   Failed to shutdown app: %d",ret);
	}
	printf("   successful to shutdown app: %d",ret);

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int status = 0;

	LOGE("open drm tz-app\n");

	printf("\n\n");
	printf("       *****************************************************\n");
	printf("       This is QHCK test program from Qualcomm.\n");
	printf("       This program is going to test the DRM key provisioning status, \n");
	printf("       and the secure boot configuration on the device.\n");
	printf("       *****************************************************\n\n\n");

	ret = probe_drm_provisioning("playread", &status);

	if (0 != ret)
		printf("get error during probing playready provisioning status\n");

	ret = probe_drm_provisioning("widevine", &status);

	if (0 != ret)
		printf("get error during probing widevine provisioning status\n");

	ret = probe_secure_boot_setup(&status);

	if (0 != ret)
		printf("get error during probing secure boot status\n");

	printf("\n\n");

	printf("      ============ SUMMARY ============\n");

	if (0 == status)
		printf("      OK. The Security Setup on this device is done.\n");
	else
		printf("      WARNING!!! This device is NOT securely setup.\n");
	printf("      ============ SUMMARY ============\n");

	return ret;
}

