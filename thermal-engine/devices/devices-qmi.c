/*===========================================================================

  devices_qmi.c

  DESCRIPTION
  Mitigation action over QMI.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_communication_init() should be called before the modem_request().

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include "thermal.h"

#include "thermal_mitigation_device_service_v01.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_client_instance_defs.h"

#define QMI_MITIGATION_DEVICE_MODEM "pa"
#define MAX_MODEM_MITIGATION_LEVEL  (3)
#define QMI_DEVICE_VDD_RESTRICT "cpuv_restriction_cold"
#define MAX_VDD_RESTRICT_LEVEL  (1)
#define QMI_CX_VDD_LIMIT "cx_vdd_limit"
#define MAX_CX_VDD_LIMIT_LEVEL  (3)

struct qmi_dev_info {
	char   *dev_name;
	char   *debug_name;
	uint8_t supported;
	int     prev_req_lvl;
};

struct qmi_client_info {
	char                *name;
	qmi_client_type      handle;
	qmi_service_instance instance_id;
	pthread_mutex_t      mtx;
	pthread_t            thread;
	struct qmi_dev_info *dev_info;
	uint8_t              dev_info_cnt;
};

static struct qmi_dev_info modem_devs[] = {
	{
		.dev_name = QMI_MITIGATION_DEVICE_MODEM,
		.debug_name = "Modem",
	},
	{
		.dev_name = QMI_DEVICE_VDD_RESTRICT,
		.debug_name = "Vdd_restriction",
	},
	{
		.dev_name = QMI_CX_VDD_LIMIT,
		.debug_name = "modem_cx_limit",
	},
};

static struct qmi_client_info modem_clnt = {
	.name  = "MODEM",
	.instance_id  = 0,
	.mtx          = PTHREAD_MUTEX_INITIALIZER,
	.dev_info     = modem_devs,
	.dev_info_cnt = ARRAY_SIZE(modem_devs),
};

static struct qmi_dev_info fusion_devs[] = {
	{
		.dev_name = QMI_MITIGATION_DEVICE_MODEM,
		.debug_name = "Fusion",
	},
};

static struct qmi_client_info fusion_clnt = {
	.name  = "FUSION",
	.instance_id  = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0,
	.mtx          = PTHREAD_MUTEX_INITIALIZER,
	.dev_info     = fusion_devs,
	.dev_info_cnt = ARRAY_SIZE(fusion_devs),
};

static struct qmi_dev_info adsp_devs[] = {
	{
		.dev_name = QMI_DEVICE_VDD_RESTRICT,
		.debug_name = "Vdd_restriction",
	},
};

static struct qmi_client_info adsp_clnt = {
	.name         = "ADSP",
	.instance_id  = 1,
	.mtx          = PTHREAD_MUTEX_INITIALIZER,
	.dev_info     = adsp_devs,
	.dev_info_cnt = ARRAY_SIZE(adsp_devs),
};

static struct qmi_client_info *vdd_clnts[] = {
	&modem_clnt,
	&adsp_clnt,
};


static qmi_idl_service_object_type tmd_service_object;

static void qmi_clnt_error_cb(qmi_client_type clnt, qmi_client_error_type error,
			      void *error_cb_data);

static void set_req_lvl(const struct qmi_client_info *clnt_info, const char *dev,
			int req)
{
	uint8_t idx;

	for (idx = 0; idx < clnt_info->dev_info_cnt; idx++) {
		if (0 == strncasecmp(clnt_info->dev_info[idx].dev_name, dev,
				QMI_TMD_MITIGATION_DEV_ID_LENGTH_MAX_V01)) {
			clnt_info->dev_info[idx].prev_req_lvl = req;
			return;
		}
	}

	msg("%s: Invalid device %s", __func__, dev);
}

/*===========================================================================
LOCAL FUNCTION verify_tmd_device

Helper function to verify any thermal mitigation device of interest is on
remote QMI TMD service.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
static int verify_tmd_device(qmi_client_type clnt,
			     struct qmi_client_info *clnt_info)
{
	int rc = -1;
	int ret = -1;
	unsigned int i, j;
	tmd_get_mitigation_device_list_resp_msg_v01 data_resp;

	if (clnt == NULL) {
		return -1;
	}

	memset(&data_resp, 0, sizeof(data_resp));
	rc = qmi_client_send_msg_sync(clnt,
				      QMI_TMD_GET_MITIGATION_DEVICE_LIST_REQ_V01,
				      NULL, 0,
				      &data_resp, sizeof(data_resp), 0);
	if (rc == QMI_NO_ERR) {
		for (i = 0; i < data_resp.mitigation_device_list_len; i++) {
			for (j = 0; j < clnt_info->dev_info_cnt; j++) {
				if (0 == strncasecmp(clnt_info->dev_info[j].\
						     dev_name,
						     data_resp.\
						     mitigation_device_list[i].\
						     mitigation_dev_id.\
						     mitigation_dev_id,
					QMI_TMD_MITIGATION_DEV_ID_LENGTH_MAX_V01
						     )) {
					/* found matching device name */
					clnt_info->dev_info[j].supported = 1;
					ret = 0;
					break;
				}
			}
		}
	} else {
		msg("%s: QMI send_msg_sync to %s failed with error %d",
		     __func__, clnt_info->name, rc);
	}

	return (!rc && !ret) ? 0 : -1;
}

/*===========================================================================
LOCAL FUNCTION request_common_qmi

Common qmi request function.

ARGUMENTS
	clnt - client on which to request throttling
	mitigation_dev_id - name of mitigation_dev_id
	level - mitigation level to be requested

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
static int request_common_qmi(qmi_client_type clnt, char *mitigation_dev_id,
			      int level)
{
	int ret = -1;
	tmd_set_mitigation_level_req_msg_v01 data_req;
	tmd_set_mitigation_level_resp_msg_v01 data_resp;

	if (!clnt || !mitigation_dev_id)
		return ret;

	strlcpy(data_req.mitigation_dev_id.mitigation_dev_id, mitigation_dev_id,
		QMI_TMD_MITIGATION_DEV_ID_LENGTH_MAX_V01);
	data_req.mitigation_level = (uint8_t)level;
	ret = qmi_client_send_msg_sync((qmi_client_type) clnt,
				       QMI_TMD_SET_MITIGATION_LEVEL_REQ_V01,
				       &data_req, sizeof(data_req),
				       &data_resp, sizeof(data_resp), 0);
	return ret;
}

/*===========================================================================
FUNCTION qmi_register

Helper function to initialize QMI connection to service.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
static void *qmi_register(void *data)
{
	int rc = -1;
	int ret = 0;
	qmi_cci_os_signal_type os_params;
	qmi_service_info info;
	qmi_client_type notifier = NULL;
	void *clnt_local = NULL;
	struct qmi_client_info *clnt_info = data;
	uint8_t idx;

	if (clnt_info == NULL) {
		msg("%s: Invalid argument.", __func__);
		return NULL;
	}

	pthread_mutex_lock(&clnt_info->mtx);
	/* release any old handles for clnt */
	if (clnt_info->handle) {
		qmi_client_release(clnt_info->handle);
		clnt_info->handle = NULL;
	}
	pthread_mutex_unlock(&clnt_info->mtx);

	rc = qmi_client_notifier_init(tmd_service_object, &os_params,
				      &notifier);
	if (rc != QMI_NO_ERR) {
		msg("qmi: qmi_client_notifier_init failed.\n");
		ret = -1;
		goto handle_return;
	}

	while (1) {
		QMI_CCI_OS_SIGNAL_CLEAR(&os_params);
		rc = qmi_client_get_service_instance(tmd_service_object,
						     clnt_info->instance_id,
						     &info);
		if(rc == QMI_NO_ERR)
			break;
		/* wait for server to come up */
		QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
	};

	rc = qmi_client_init(&info, tmd_service_object, NULL, NULL, NULL,
			     (qmi_client_type *) (&clnt_local));
	if (rc != QMI_NO_ERR) {
		msg("%s thermal mitigation not available.", clnt_info->name);
		ret = -1;
		goto handle_return;
	}
	/* Verify mitigation device present on service */
	rc = verify_tmd_device(clnt_local, clnt_info);
	if (rc != 0) {
		qmi_client_release(clnt_local);
		clnt_local = NULL;
		ret = -1;
		goto handle_return;
	}

	/* best effort register for error */
	qmi_client_register_error_cb(clnt_local, qmi_clnt_error_cb, data);
	clnt_info->handle = clnt_local;
	info("%s thermal mitigation available.", clnt_info->name);

	pthread_mutex_lock(&clnt_info->mtx);
	for (idx = 0; idx < clnt_info->dev_info_cnt; idx++) {
		if (!clnt_info->dev_info[idx].supported) {
			dbgmsg("%s: %s not supported for clnt %s", __func__,
			       clnt_info->dev_info[idx].dev_name,
			       clnt_info->name);
			continue;
		}

		ret = request_common_qmi(clnt_info->handle,
					 clnt_info->dev_info[idx].dev_name,
					 clnt_info->dev_info[idx].prev_req_lvl);
		if (ret)
			msg("(%s) %s mitigation failed with %d for level %d",
			    clnt_info->name, clnt_info->dev_info[idx].dev_name,
			    ret, clnt_info->dev_info[idx].prev_req_lvl);
		else {
			info("ACTION: %s - Pending request: %s mitigation"
			     " succeeded for level %d.", clnt_info->name,
			     clnt_info->dev_info[idx].dev_name,
			     clnt_info->dev_info[idx].prev_req_lvl);
			if(!strncmp(clnt_info->dev_info[idx].dev_name, "pa", 2))
				thermalmsg(LOG_LVL_INFO, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
					   "%s:Modem:%d\n", MITIGATION,
					   clnt_info->dev_info[idx].prev_req_lvl);
			else
				thermalmsg(LOG_LVL_INFO, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
					   "%s:VDD[%s-%s]:%d\n",
				           MITIGATION, clnt_info->name,
					   clnt_info->dev_info[idx].dev_name,
					   clnt_info->dev_info[idx].prev_req_lvl);
		}
	}
	pthread_mutex_unlock(&clnt_info->mtx);

handle_return:
	if (notifier != NULL) {
		qmi_client_release(notifier);
	}

	return NULL;
}

/*===========================================================================
LOCAL FUNCTION qmi_clnt_error_cb

Callback function called by the QCCI infrastructure when it receives a
REMOVE SERVER message from the qmi service.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
static void qmi_clnt_error_cb(qmi_client_type clnt,
			      qmi_client_error_type error,
			      void *error_cb_data)
{
	struct qmi_client_info *clnt_info = error_cb_data;
	uint8_t idx;

	if (clnt_info == NULL) {
		msg("%s: Invalid argument.", __func__);
		return;
	}

	msg("%s: with error %d called for clnt %s\n", __func__, error,
	    clnt_info->name);

	if (clnt == NULL)
		return;

	pthread_join(clnt_info->thread, NULL);
	/* Clear supported device flag */
	for (idx = 0; idx < clnt_info->dev_info_cnt; idx++)
		clnt_info->dev_info[idx].supported = 0;
	pthread_create(&(clnt_info->thread), NULL, qmi_register,
		       (void *)clnt_info);
}

/*===========================================================================
FUNCTION vdd_restrict_qmi_request

Action function to throttle modem functionality.

ARGUMENTS
	level - 0-1 throttling level for vdd restrict mitigation

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int vdd_restrict_qmi_request(int level)
{
	int ret = -1;
	struct qmi_client_info *clnt_info = NULL;
	int idx;

	/* cpuv_restriction_cold level:
	 * 0 - No action
	 * 1 - Restrict to nominal voltage
	 */

	if (level < 0)
		level = 0;

	if (level > MAX_VDD_RESTRICT_LEVEL)
		level = MAX_VDD_RESTRICT_LEVEL;

	for (idx = 0; idx < ARRAY_SIZE(vdd_clnts); idx++) {
		clnt_info = vdd_clnts[idx];
		pthread_mutex_lock(&clnt_info->mtx);
		if (!clnt_info->handle) {
			info("%s: %s req level(%d) is recorded and waiting for "
			     "completing QMI registration", __func__,
			     clnt_info->name, level);
			ret = 0;
			goto handle_clnt_done;
		}

		ret = request_common_qmi(clnt_info->handle,
					 QMI_DEVICE_VDD_RESTRICT,
					 level);
		if (ret)
			msg("(%s) Vdd_restriction mitigation failed with %d "
			    "for level %d", clnt_info->name, ret, level);
		else {
			info("ACTION: %s - Vdd_restriction mitigation "
			     "succeeded for level %d.", clnt_info->name, level);
			thermalmsg(LOG_LVL_INFO, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
				   "%s:VDD[%s]:%d\n", MITIGATION,
				   clnt_info->name, level);
		}

handle_clnt_done:
		/* Save previous request to handle subsytem restart, and
		   requests prior to QMI service being available. */
		set_req_lvl(clnt_info, QMI_DEVICE_VDD_RESTRICT, level);
		pthread_mutex_unlock(&clnt_info->mtx);
	}

	return ret;
}

/*===========================================================================
FUNCTION modem_request

Action function to throttle modem functionality.

ARGUMENTS
	level - 0-3 throttling level for modem mitigation

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int modem_request(int level)
{
	int ret = -1;

	/* Modem level: 0 - No action
	 * 		1 - Mitigation level 1
	 * 		2 - Mitigation level 2
	 * 		3 - Emergency
	 */

	if (level < 0)
		level = 0;

	if (level > MAX_MODEM_MITIGATION_LEVEL)
		level = MAX_MODEM_MITIGATION_LEVEL;

	pthread_mutex_lock(&modem_clnt.mtx);
	if (!modem_clnt.handle) {
		info("%s: Requested level(%d) is recorded and waiting for "
		     "completing QMI registration", __func__, level);
		ret = 0;
		goto handle_return;
	}

	ret = request_common_qmi(modem_clnt.handle,
				 QMI_MITIGATION_DEVICE_MODEM,
				 level);
	if (ret)
		msg("Modem mitigation failed with %d for level %d",
		    ret, level);
	else {
		info("ACTION: MODEM - "
		     "Modem mitigation succeeded for level %d.", level);
		thermalmsg(LOG_LVL_INFO, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
			   "%s:Modem:%d\n", MITIGATION, level);
	}

handle_return:
	/* Save previous request to handle subsytem restart, and
	   requests prior to QMI service being available. */
	set_req_lvl(&modem_clnt, QMI_MITIGATION_DEVICE_MODEM, level);
	pthread_mutex_unlock(&modem_clnt.mtx);

	return ret;
}

/*===========================================================================
FUNCTION fusion_modem_request

Action function to throttle fusion modem functionality.

ARGUMENTS
	level - 0-3 throttling level for modem mitigation

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int fusion_modem_request(int level)
{
	int ret = -1;

	/* Modem level: 0 - No action
	 * 		1 - Mitigation level 1
	 * 		2 - Mitigation level 2
	 * 		3 - Emergency
	 */

	if (level < 0)
		level = 0;

	if (level > MAX_MODEM_MITIGATION_LEVEL)
		level = MAX_MODEM_MITIGATION_LEVEL;

	pthread_mutex_lock(&fusion_clnt.mtx);

	if (!fusion_clnt.handle) {
		info("%s: Requested level(%d) is recorded and waiting for "
		     "completing QMI registration", __func__, level);
		ret = 0;
		goto handle_return;
	}

	ret = request_common_qmi(fusion_clnt.handle,
				 QMI_MITIGATION_DEVICE_MODEM,
				 level);
	if (ret)
		msg("Fusion modem mitigation failed with %d for level %d", ret,
		    level);
	else {
		info("ACTION: FUSION - "
		     "Fusion modem mitigation succeeded for level %d.", level);
		thermalmsg(LOG_LVL_INFO, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
			   "%s:FusionModem:%d\n", MITIGATION, level);
	}

handle_return:
	/* Save previous request to handle subsytem restart, and
	   requests prior to QMI service being available. */
	set_req_lvl(&fusion_clnt, QMI_MITIGATION_DEVICE_MODEM, level);
	pthread_mutex_unlock(&fusion_clnt.mtx);

	return ret;
}

/*===========================================================================
FUNCTION modem_cx_limit_request

Action function to throttle modem CX limit.

ARGUMENTS
	level - 0-3 throttling level for modem CX limit

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int modem_cx_limit_request(int level)
{
	int ret = -1;

	/* Modem CX limit level: 0 - No action
	 *		1 - Mitigation level 1
	 *		2 - Mitigation level 2
	 *		3 - Emergency
	 */

	if (level < 0)
		level = 0;

	if (level > MAX_CX_VDD_LIMIT_LEVEL)
		level = MAX_CX_VDD_LIMIT_LEVEL;

	pthread_mutex_lock(&modem_clnt.mtx);
	if (!modem_clnt.handle) {
		info("%s: Requested level(%d) is recorded and waiting for "
		     "completing QMI registration", __func__, level);
		ret = 0;
		goto handle_return;
	}

	ret = request_common_qmi(modem_clnt.handle,
				 QMI_CX_VDD_LIMIT,
				 level);
	if (ret)
		msg("Modem CX limit mitigation failed with %d for level %d",
		    ret, level);
	else {
		dbgmsg("ACTION: MODEM_CX_LIMIT - "
		     "Modem CX limit succeeded for level %d.", level);
		thermalmsg(LOG_LVL_INFO, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
			   "%s:Modem_CX_limit:%d\n", MITIGATION, level);
	}

handle_return:
	/* Save previous request to handle subsytem restart, and
	   requests prior to QMI service being available. */
	set_req_lvl(&modem_clnt, QMI_CX_VDD_LIMIT, level);
	pthread_mutex_unlock(&modem_clnt.mtx);

	return ret;
}

/*===========================================================================
FUNCTION qmi_communication_init

Helper function to initialize QMI TMD communication.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int qmi_communication_init(void)
{
	/* Get the service object for the tmd API */
	tmd_service_object = tmd_get_service_object_v01();
	if (!tmd_service_object) {
		msg("qmi: tmd_get_service_object failed.\n");
		return -1;
	}

	fusion_clnt.instance_id = get_fusion_qmi_client_type();

	/* start thread to register with onchip Modem QMI services */
	pthread_create(&(modem_clnt.thread), NULL, qmi_register,
		       (void*)&modem_clnt);

	/* start thread to register with fusion Modem QMI services */
	pthread_create(&(fusion_clnt.thread), NULL, qmi_register,
		       (void*)&fusion_clnt);

	/* start thread to register with ADSP QMI services */
	pthread_create(&(adsp_clnt.thread), NULL, qmi_register,
		       (void*)&adsp_clnt);

	return 0;
}

/*===========================================================================
FUNCTION qmi_communication_release

Release function for QMI TMD communication to clean up resources.
Called after use of client handles is complete.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int qmi_communication_release(void)
{
	int rc = 0;

	pthread_join(modem_clnt.thread, NULL);
	pthread_join(fusion_clnt.thread, NULL);
	pthread_join(adsp_clnt.thread, NULL);

	if (modem_clnt.handle) {
		rc = qmi_client_release(modem_clnt.handle);
		if (rc)
			msg("qmi: qmi_client_release modem clnt failed.\n");
		modem_clnt.handle = NULL;
	}
	if (fusion_clnt.handle) {
		rc = qmi_client_release(fusion_clnt.handle);
		if (rc)
			msg("qmi: qmi_client_release fusion clnt failed.\n");
		fusion_clnt.handle = NULL;
	}
	if (adsp_clnt.handle) {
		rc = qmi_client_release(adsp_clnt.handle);
		if (rc)
			msg("qmi: qmi_client_release adsp clnt failed.\n");
		adsp_clnt.handle = NULL;
	}

	return rc;
}
