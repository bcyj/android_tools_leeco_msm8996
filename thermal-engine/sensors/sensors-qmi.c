/*===========================================================================

  sensor-qmi.c

  DESCRIPTION
  QMI TS sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_ts_setup() function should be called before qmi_ts_get_temperature().
  qmi_ts_shutdown() function should be called to clean up resources.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "sensors_manager_internal.h"
#include "thermal.h"
#include "sensors-qmi.h"

#include "thermal_sensor_service_v01.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_client_instance_defs.h"

/* Specific qmi_ts sensor data */
struct qmi_ts_data {
	pthread_mutex_t mutex;
	pthread_cond_t  condition;
	int threshold_reached;
	struct sensor_info *sensor;
	struct thresholds_req_t thresh;
};

/* Help track individual sensors for setup, shutdown, and updates. */
struct qmi_ts_thermald_data {
	const char  *thermald_name;
	const char  *ts_qmi_name;
	int          last_read;
	struct qmi_ts_data *data;
};

/* Used to protect access of thermald_info */
static pthread_mutex_t qmi_ts_info_mtx = PTHREAD_MUTEX_INITIALIZER;

/* Add newly supported sensors here. */
static struct qmi_ts_thermald_data thermald_info[] = {
	{"pa_therm0", "pa", 0, NULL},
	{"pa_therm1", "pa_1", 0, NULL}
};

#define QMI_TS_MAX_STRING 16

#define QMI_SENSOR_MODEM "pa"
static void *fusion_modem_clnt;

static pthread_t fusion_qmi_register_thread;
static int fusion_modem_qmi_ts_ready;

static ts_temp_report_ind_msg_v01  ind_struct;

static qmi_idl_service_object_type ts_service_object;

/* Used to protect sync client request from multiple threads. */
static pthread_mutex_t ts_req_mtx = PTHREAD_MUTEX_INITIALIZER;

static void modem_clnt_error_cb(qmi_client_type clnt,
				 qmi_client_error_type error,
				 void *error_cb_data);

static int modem_ts_reg_notify(void *clnt, const char *sensor_id,
				int send_current_temp_report,
				struct thresholds_req_t *thresh,
				uint8_t seq_num_valid, uint32_t seq_num);
static void qmi_ts_update_temperature(ts_temp_report_ind_msg_v01 *ind,
				      uint8_t notify);

/*===========================================================================
LOCAL FUNCTION qmi_ts_ind_cb

Handle QMI TS indication callbacks.

ARGUMENTS
	user_handle - QMI client user handle.
	msg_id - Indication message id.
	ind_buf - Indication encoded data.
	ind_buf_len - Indication encoded data length.
	ind_cb_data - TS provided callback data.

RETURN VALUE
	None.
===========================================================================*/
static void qmi_ts_ind_cb(qmi_client_type user_handle,
			   unsigned int     msg_id,
			   void            *ind_buf,
			   unsigned int     ind_buf_len,
			   void            *ind_cb_data)
{
	qmi_client_error_type rc = QMI_NO_ERR;

	if (msg_id == QMI_TS_TEMP_REPORT_IND_V01) {
		memset((void *)&ind_struct, 0,
		       sizeof(ts_temp_report_ind_msg_v01));
		rc = qmi_client_message_decode(user_handle, QMI_IDL_INDICATION,
					       msg_id, ind_buf, ind_buf_len,
					       &ind_struct,
					       sizeof(ts_temp_report_ind_msg_v01));

		if (rc != QMI_NO_ERR) {
			msg("Error invalid indication message received.\n");
		} else {
			uint8_t notify = 0;

			/* Determine whether indication warrants sensor
			   notification. Only threshold indications should
			   trigger notify update. The exception being special
			   case were seq_num_valid field is used to in
			   combination with current temp request to trigger
			   update to initialize the state of temperature data
			   after successful registration. */
			if ((ind_struct.report_type !=
			     QMI_TS_TEMP_REPORT_CURRENT_TEMP_V01) ||
			    ind_struct.seq_num_valid)
				notify = 1;

			if (ind_struct.temp_valid) {
				dbgmsg("%s: %s %d\n", __func__,
				       ind_struct.sensor_id.sensor_id,
				       (int)ind_struct.temp);
				qmi_ts_update_temperature(&ind_struct, notify);
			} else {
				msg("Error invalid temperature field.");
			}
		}
	} else {
		printf("\nWarning invalid indication message received.\n");
	}
}

/*===========================================================================
LOCAL FUNCTION modem_verify_ts_device

Helper function to verify QMI_SENSOR_MODEM thermal sensor exits on remote QMI
TS service.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -(ERRNO) Code on failure.
===========================================================================*/
static int modem_verify_ts_device(void *clnt)
{
	int rc;
	int ret = -(EFAULT);
	unsigned int i;

	ts_get_sensor_list_resp_msg_v01 *list_resp = NULL;
	ts_register_notification_temp_req_msg_v01 *reg_notify_req = NULL;
	ts_register_notification_temp_resp_msg_v01 *reg_notify_resp = NULL;
	/* Large structs let's not put it on the stack. */

	if (clnt == NULL) {
		ret = -(EINVAL);
		goto error_handler;
	}

	list_resp = malloc(sizeof(ts_get_sensor_list_resp_msg_v01));
	if (list_resp == NULL) {
		msg("%s: Malloc list_resp failure", __func__);
		ret = -(ENOMEM);
		goto error_handler;
	}

	reg_notify_req = malloc(sizeof(ts_register_notification_temp_req_msg_v01));
	if (reg_notify_req == NULL) {
		msg("%s: Malloc reg_notify_req failure", __func__);
		ret = -(ENOMEM);
		goto error_handler;
	}

	reg_notify_resp = malloc(sizeof(ts_register_notification_temp_resp_msg_v01));
	if (reg_notify_resp == NULL) {
		msg("%s: Malloc reg_notify_resp failure", __func__);
		ret = -(ENOMEM);
		goto error_handler;
	}

	memset(list_resp, 0, sizeof(ts_get_sensor_list_resp_msg_v01));
	pthread_mutex_lock(&ts_req_mtx);
	rc = qmi_client_send_msg_sync(clnt,
				      QMI_TS_GET_SENSOR_LIST_REQ_V01,
				      NULL, 0,
				      list_resp, sizeof(ts_get_sensor_list_resp_msg_v01), 0);
	pthread_mutex_unlock(&ts_req_mtx);
	if (rc == QMI_NO_ERR) {
		for (i = 0; i < list_resp->sensor_list_len; i++) {
			if (0 == strncasecmp(QMI_SENSOR_MODEM,
					     list_resp->sensor_list[i].sensor_id,
					     QMI_TS_SENSOR_ID_LENGTH_MAX_V01)) {
				/* found matching device name */
				fusion_modem_clnt = clnt;
				ret = 0;
			}

			/* Send out current temp notify request to get
			   clients to configure threshold triggers
			   properly. */
			modem_ts_reg_notify(clnt, list_resp->sensor_list[i].sensor_id,
					    1, NULL, 1, 0);
		}
	} else {
		msg("%s: QMI send_msg_sync failed with error %d", __func__, rc);
		ret = -(EFAULT);
	}

error_handler:
	if (list_resp != NULL)
		free(list_resp);

	if (reg_notify_req != NULL)
		free(reg_notify_req);

	if (reg_notify_resp != NULL)
		free(reg_notify_resp);

	return ret;
}

/*===========================================================================
LOCAL FUNCTION fusion_qmi_register

Helper function to initialize QMI connection to modem service.

ARGUMENTS
	None.

RETURN VALUE
	NULL on exit
===========================================================================*/
static void *fusion_qmi_register(void *data)
{
	int rc;
	qmi_cci_os_signal_type os_params;
	qmi_service_info info;
	qmi_client_type notifier = NULL;
	void *modem_clnt_local = NULL;
	qmi_service_instance instance_id = get_fusion_qmi_client_type();

	/* release any old handles for fusion_modem_clnt */
	if (fusion_modem_clnt) {
		qmi_client_release(fusion_modem_clnt);
		fusion_modem_clnt = NULL;
	}

	rc = qmi_client_notifier_init(ts_service_object, &os_params, &notifier);
	if (rc != QMI_NO_ERR) {
		msg("qmi: qmi_client_notifier_init failed.\n");
		goto error_handler;
	}

	info("qmi: Instance id %d for fusion TS", instance_id);
	while (1) {
		QMI_CCI_OS_SIGNAL_CLEAR(&os_params);
		rc = qmi_client_get_service_instance(ts_service_object,
						     instance_id,
						     &info);
		if (rc == QMI_NO_ERR)
			break;
		/* wait for server to come up */
		QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
	};

	rc = qmi_client_init(&info, ts_service_object, qmi_ts_ind_cb, NULL, NULL,
			     (qmi_client_type *) (&modem_clnt_local));
	if (rc != QMI_NO_ERR) {
		msg("Modem thermal sensor service not available.\n");
		goto error_handler;
	}
	/* Verify modem sensor service present on modem */
	rc = modem_verify_ts_device(modem_clnt_local);
	if (rc != 0) {
		qmi_client_release(modem_clnt_local);
		modem_clnt_local = NULL;
		goto error_handler;
	}

	/* best effort register for error */
	qmi_client_register_error_cb(modem_clnt_local, modem_clnt_error_cb, NULL);

	fusion_modem_qmi_ts_ready = 1;
	info("Modem thermal sensor service available.\n");

error_handler:
	if (notifier != NULL)
		qmi_client_release(notifier);

	return NULL;
}

/*===========================================================================
LOCAL FUNCTION modem_clnt_error_cb

Callback function called by the QCCI infrastructure when it receives a
REMOVE SERVER message from the modem.

ARGUMENTS
	None.

RETURN VALUE
	None.
===========================================================================*/
static void modem_clnt_error_cb(qmi_client_type clnt,
				 qmi_client_error_type error,
				 void *error_cb_data)
{
	msg("%s: with %d called for clnt %p\n", __func__, error, (void *)clnt);
	if (clnt == NULL)
		return;

	if (clnt == fusion_modem_clnt) {
		fusion_modem_qmi_ts_ready = 0;
		pthread_join(fusion_qmi_register_thread, NULL);
		pthread_create(&fusion_qmi_register_thread, NULL,
			       fusion_qmi_register, NULL);
	}
}

/*===========================================================================
LOCAL FUNCTION modem_ts_reg_notify

Common TS qmi modem register notify function.

ARGUMENTS
	clnt - client on which to request throttling
	sensor_id - name of sensor_id
	send_current_temp_report - 1 for trigger and immediate sensor reading,
				   0 for set threshold.
	thresh - high_thresh and low_thresh
	seq_num_valid - sets seq_num valid field in request
			0 - disable
			!0 - enable
	seq_num - only valid if seq_num_valid is !0.
		  Used to associate a reponse with request

RETURN VALUE
	0 on success, -(ERRNO) on failure.
===========================================================================*/
static int modem_ts_reg_notify(void *clnt, const char *sensor_id,
				int send_current_temp_report,
				struct thresholds_req_t *thresh,
				uint8_t seq_num_valid, uint32_t seq_num)
{
	int ret = -(EFAULT);
	qmi_client_error_type qmi_error = QMI_NO_ERR;

	ts_register_notification_temp_req_msg_v01  data_req;
	ts_register_notification_temp_resp_msg_v01 data_resp;

	if (!clnt || !sensor_id)
		return -(EINVAL);

	memset(&data_req, 0x0, sizeof(data_req));
	strlcpy(data_req.sensor_id.sensor_id, sensor_id,
		QMI_TS_SENSOR_ID_LENGTH_MAX_V01);

	data_req.seq_num_valid = seq_num_valid;
	data_req.seq_num = seq_num;

	if (send_current_temp_report) {
		data_req.send_current_temp_report = 1;
	} else {
		data_req.temp_threshold_high_valid = thresh->high_valid;
		data_req.temp_threshold_high = (float)thresh->high;
		data_req.temp_threshold_low_valid = thresh->low_valid;
		data_req.temp_threshold_low = (float)thresh->low;
	}

	pthread_mutex_lock(&ts_req_mtx);
	qmi_error = qmi_client_send_msg_sync((qmi_client_type) clnt,
				       QMI_TS_REGISTER_NOTIFICATION_TEMP_REQ_V01,
				       &data_req, sizeof(data_req),
				       &data_resp, sizeof(data_resp), 0);
	pthread_mutex_unlock(&ts_req_mtx);
	if (qmi_error == QMI_NO_ERR) {
		ret = 0;
	} else {
		msg("qmi: qmi_client_send_msg_sync failed. Error %d\n", qmi_error);
	}

	return ret;
}

/*===========================================================================
LOCAL FUNCTION qmi_ts_get_thermald_info_idx

Helper function for finding thermald_info index of information based off of
thermald sensor name..

ARGUMENTS
	sensor - Thermald version of sensor name.

RETURN VALUE
	-1 on Failure, thermald_info array idx on Success.
===========================================================================*/
static int qmi_ts_get_thermald_info_idx(const char *sensor)
{
	int idx = -1;

	/* Find corresponding struct qmi_ts_thermald_data */
	for (idx = 0; idx < ARRAY_SIZE(thermald_info); idx++) {
		if (strncmp(thermald_info[idx].thermald_name, sensor,
			    QMI_TS_MAX_STRING) == 0) {
			break;
		}
	}

	if (idx == ARRAY_SIZE(thermald_info))
		idx = -1;

	return idx;
}

/*===========================================================================
FUNCTION qmi_ts_update_temperature

Updates the temperature for the thermald_info array.

ARGUMENTS
	ind - QMI TS indicator msg.
	notify - 0 - Don't notify interrupt wait thread on update.
		 !0 -  Notify interrupt wait thread on update.
RETURN VALUE
	None
===========================================================================*/
static void qmi_ts_update_temperature(ts_temp_report_ind_msg_v01 *ind,
				      uint8_t notify)
{
	int idx = 0;

	if (NULL == ind) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	/* Find corresponding struct qmi_ts_data */
	for (idx = 0; idx < ARRAY_SIZE(thermald_info); idx++) {
		if (strncmp(thermald_info[idx].ts_qmi_name,
			    ind->sensor_id.sensor_id,
			    QMI_TS_MAX_STRING) == 0) {
			break;
		}
	}

	if (idx >= ARRAY_SIZE(thermald_info)) {
		msg("%s: unknown sensor %s\n", __func__,
		    ind->sensor_id.sensor_id);
		return;
	}

	thermald_info[idx].last_read = CONV((int)ind->temp);

	if (!notify)
		return;

	pthread_mutex_lock(&qmi_ts_info_mtx);
	if (thermald_info[idx].data != NULL) {
		struct qmi_ts_data *qmi_ts = thermald_info[idx].data;
		/* notify the waiting thread */
		dbgmsg("%s: notify update %s %dC\n", __func__,
		       ind->sensor_id.sensor_id,
		       (int)ind->temp);
		pthread_mutex_lock(&(qmi_ts->mutex));
		qmi_ts->threshold_reached = 1;
		pthread_cond_broadcast(&(qmi_ts->condition));
		pthread_mutex_unlock(&(qmi_ts->mutex));
	}
	pthread_mutex_unlock(&qmi_ts_info_mtx);
}

/*===========================================================================
FUNCTION qmi_ts_setup

QMI TS setup sensor.

ARGUMENTS
	sensor - thermald sensor data

RETURN VALUE
	0 on Failure, 1 on Success
===========================================================================*/
int qmi_ts_setup(struct sensor_info *sensor)
{
	struct qmi_ts_data *qmi_ts = NULL;
	int idx;

	idx = qmi_ts_get_thermald_info_idx(sensor->name);
	if (idx < 0) {
		msg("%s: invalid sensor name %s", __func__, sensor->name);
		return 0;
	}

	/* Allocate QMI TS data */
	qmi_ts = (struct qmi_ts_data *) malloc(sizeof(struct qmi_ts_data));
	if (NULL == qmi_ts) {
		msg("%s: malloc failed", __func__);
		return 0;
	}
	memset(qmi_ts, 0, sizeof(struct qmi_ts_data));
	sensor->data = (void *) qmi_ts;

	pthread_mutex_init(&(qmi_ts->mutex), NULL);
	pthread_cond_init(&(qmi_ts->condition), NULL);
	qmi_ts->threshold_reached = 0;
	qmi_ts->sensor = sensor;
	thermald_info[idx].data = qmi_ts;

	return 1;
}

/*===========================================================================
FUNCTION qmi_ts_shutdown

QMI TS shutdown sensor.

ARGUMENTS
	sensor - thermald sensor

RETURN VALUE
	None
===========================================================================*/
void qmi_ts_shutdown(struct sensor_info *sensor)
{
	struct qmi_ts_data *qmi_ts;
	int idx;

	if (NULL == sensor ||
	    NULL == sensor->name ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	idx = qmi_ts_get_thermald_info_idx(sensor->name);
	if (idx < 0) {
		msg("%s: invalid sensor name %s", __func__,
		    sensor->name);
		return;
	}

	pthread_mutex_lock(&qmi_ts_info_mtx);
	/* Make sure an indication cannot be issued to a sensor being shutdown */
	qmi_ts = thermald_info[idx].data;
	thermald_info[idx].data = NULL;
	sensor->data = NULL;
	pthread_mutex_unlock(&qmi_ts_info_mtx);

	pthread_mutex_destroy(&qmi_ts->mutex);
	pthread_cond_destroy(&qmi_ts->condition);

	free(qmi_ts);
}

/*===========================================================================
FUNCTION qmi_ts_get_temperature

QMI TS get sensor temperature.

ARGUMENTS
	sensor - thermald sensor

RETURN VALUE
	Current temperature, CONV(-273) on failure
===========================================================================*/
int qmi_ts_get_temperature(struct sensor_info *sensor)
{
	int temp = CONV(-273);
	int idx;

	if (NULL == sensor ||
	    NULL == sensor->name ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return temp;
	}

	idx = qmi_ts_get_thermald_info_idx(sensor->name);
	if (idx < 0) {
		msg("%s: invalid sensor name %s", __func__,
		    sensor->name);
		return temp;
	}

	temp = thermald_info[idx].last_read;

	/* Trigger new read update in case a polling algorithm requires an
	   updated value. */
	modem_ts_temp_request(thermald_info[idx].ts_qmi_name, 1, NULL);
	return temp;
}

/*===========================================================================
FUNCTION qmi_ts_interrupt_wait

QMI TS sensor wait for interrupt.

ARGUMENTS
	sensor - thermald sensor

RETURN VALUE
	None
===========================================================================*/
void qmi_ts_interrupt_wait(struct sensor_info *sensor)
{
	struct qmi_ts_data *qmi_ts;

	if (NULL == sensor ||
	    NULL == sensor->name ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	if (sensor->interrupt_enable) {
		qmi_ts = (struct qmi_ts_data *) sensor->data;

		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(qmi_ts->mutex));
		while (!qmi_ts->threshold_reached) {
			pthread_cond_wait(&(qmi_ts->condition),
					  &(qmi_ts->mutex));
		}
		qmi_ts->threshold_reached = 0;
		pthread_mutex_unlock(&(qmi_ts->mutex));
	}
}

/*===========================================================================
FUNCTION qmi_ts_update_thresholds

QMI TS sensor update interrupt thresholds.

ARGUMENTS
	sensor - thermald sensor
	thresh - type of threshold change that triggered update

RETURN VALUE
	None
===========================================================================*/
void qmi_ts_update_thresholds(struct sensor_info *sensor,
			       struct thresholds_req_t *thresh)
{
	int idx;

	if (NULL == thresh ||
	    NULL == sensor ||
	    NULL == sensor->data ||
	    NULL == sensor->name) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	idx = qmi_ts_get_thermald_info_idx(sensor->name);
	if (idx < 0) {
		msg("%s: invalid sensor name %s", __func__, sensor->name);
		return;
	}

	/* Convert thresholds to Celsius for QMI TS*/
	thresh->high = RCONV(thresh->high);
	thresh->low = RCONV(thresh->low);

	modem_ts_temp_request(thermald_info[idx].ts_qmi_name, 0, thresh);
}

/*===========================================================================
FUNCTION modem_ts_temp_request

Function to request sensor read or notify threshold functionality.

ARGUMENTS
	sensor_id - name of sensor_id
	send_current_temp_report - 1 for trigger and immediate sensor reading, 0 for
				   set threshold.
	thresh

RETURN VALUE
	0 on success, -(ERRNO) on failure.
===========================================================================*/
int modem_ts_temp_request(const char *sensor_id,
			   int send_current_temp_report,
			   struct thresholds_req_t *thresh)
{
	int ret = -(EFAULT);

	if ((send_current_temp_report == 0) && ((thresh->high_valid == 1) &&
						(thresh->low_valid == 1)) &&
						(thresh->high <= thresh->low)) {
		msg("Invalid thresh level. High %d, Low %d",
		    thresh->high, thresh->low);
		return -(EINVAL);
	}

	if (!fusion_modem_clnt) {
		msg("Modem TS service failed - QMI registration incomplete");
		return ret;
	}

	ret = modem_ts_reg_notify(fusion_modem_clnt, sensor_id, send_current_temp_report,
				  thresh, 0, 0);
	if (send_current_temp_report) {
		dbgmsg("%s %s, sensor %s, Get Immediate\n",
		       __func__, (ret) ? ("Failed") : ("Success"), sensor_id);
	} else {
		dbgmsg("%s %s, sensor %s, High valid: %s, High %d "
		       "Low valid: %s, Low %d\n",
		       __func__, (ret) ? ("Failed") : ("Success"), sensor_id,
		       (thresh->high_valid) ? ("YES") : ("NO"), thresh->high,
		       (thresh->low_valid) ? ("YES") : ("NO"), thresh->low);
	}

	return ret;
}

/*===========================================================================
FUNCTION modem_ts_qmi_init

Helper function to initialize TS qmi communication to modem.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -(ERRNO) on failure.
===========================================================================*/
int modem_ts_qmi_init(void)
{
	/* Get the service object for the ts API */
	ts_service_object = ts_get_service_object_v01();
	if (!ts_service_object) {
		msg("qmi: ts_get_service_object failed.\n");
		return -(EFAULT);
	}

	/* start thread to register with QMI services */
	pthread_create(&fusion_qmi_register_thread, NULL, fusion_qmi_register, NULL);

	return 0;
}

/*===========================================================================
FUNCTION modem_qmi_ts_comm_release

Release function for modem communication to clean up resources.
Called after use of client handles is complete.

ARGUMENTS
	None.

RETURN VALUE
	0 on success, -(ERRNO) on failure.
===========================================================================*/
int modem_qmi_ts_comm_release(void)
{
	int rc;
	int ret = 0;

	pthread_join(fusion_qmi_register_thread, NULL);

	if (fusion_modem_clnt) {
		rc = qmi_client_release(fusion_modem_clnt);
		if (rc) {
			msg("qmi: qmi_client_release modem clnt failed.\n");
			ret = -(EFAULT);
		}
		fusion_modem_clnt = 0;
	}

	return ret;
}
