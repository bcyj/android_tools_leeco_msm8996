/*===========================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <stringl.h>

#include "thermal.h"
#include "sensors_manager_internal.h"

#define THERM_MUTEX_LOCK(x) pthread_mutex_lock((x)); {
#define THERM_MUTEX_UNLOCK(x) } pthread_mutex_unlock(x);

static struct sensors_mgr_sensor_info *sensor_list;
static uint32_t sensor_cnt;
static uint32_t alias_cnt;

/* Protect client request collisions  */
#ifdef ANDROID
static pthread_mutex_t ts_clnt_mtx = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
static pthread_mutex_t ts_clnt_mtx = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

static struct sensors_mgr_sensor_info *find_sensor(const char *sensor_name)
{
	struct sensors_mgr_sensor_info *curr = sensor_list;

	/* Find dev info */
	while (curr != NULL) {
		if (strncmp(sensor_name, curr->name,
			    MAX_SENSOR_NAME_LEN) == 0) {
			break;
		}

		if ((curr->alias != NULL) &&
		    (strncmp(sensor_name, curr->alias,
			     MAX_SENSOR_NAME_LEN) == 0))
			break;
		curr = curr->next_sensor;
	}

	return curr;
}

static int validate_thresh_info(struct sensor_thresh_req *thresh_info)
{
	struct thresholds_req_t *thresh = &(thresh_info->thresh);

	if (thresh_info->notify_cb_func == NULL)
		return -(EINVAL);

	if (thresh->high_valid && thresh->low_valid &&
	    (thresh->low >= thresh->high))
		return -(EINVAL);

	if (thresh_info->polling_interval_valid &&
	    (thresh_info->polling_interval == 0))
		return -(EINVAL);

	return 0;
}

static int update_active_thresh(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_client_type  *client = NULL;
	struct sensor_thresh_req *active = NULL;
	uint8_t                active_req = 0;


	if (sensor_mgr == NULL)
		return -(EINVAL);

	active = &sensor_mgr->active_thresh;

	memset(active, 0, sizeof(struct sensor_thresh_req));
	active->thresh.low = INT32_MIN;
	active->thresh.high  = INT32_MAX;
	active->polling_interval = UINT32_MAX;

	client = sensor_mgr->client_list;
	THERM_MUTEX_LOCK(&ts_clnt_mtx);
	while (client != NULL) {
		if (!client->request_active) {
			client = client->next_clnt;
			continue;
		}

		struct sensor_thresh_req *thresh = &client->request;
		/* Find active high */
		if (thresh->thresh.high_valid) {
			active->thresh.high_valid = 1;
			active->thresh.high = MIN(active->thresh.high,
							thresh->thresh.high);
		}

		/* Find active low */
		if (thresh->thresh.low_valid) {
			active->thresh.low_valid = 1;
			active->thresh.low = MAX(active->thresh.low,
						       thresh->thresh.low);
		}

		/* Find min polling interval */
		if (thresh->polling_interval_valid) {
			active->polling_interval_valid = 1;
			active->polling_interval = MIN(active->polling_interval,
							     thresh->polling_interval);
		}

		active_req = 1;
		client = client->next_clnt;
	}

	if ((active->thresh.high > active->thresh.low) &&
	    (active->thresh.high_valid || active->thresh.low_valid)) {
		/* We can take advantage of interrupt */
		sensor_mgr->active_thresh_valid = 1;
	} else {
		sensor_mgr->active_thresh_valid = 0;
	}

	dbgmsg("%s: %s Active(%d), Hi(%d) %d, Lo(%d) %d, Interval(%d) %d\n",
	       __func__, sensor_mgr->name,
	       sensor_mgr->active_thresh_valid,
	       active->thresh.high_valid, active->thresh.high,
	       active->thresh.low_valid, active->thresh.low,
	       active->polling_interval_valid,
	       active->polling_interval);

	/* Room for optimization if thresholds didn't change. */
	if (sensor_mgr->active_thresh_valid &&
	    sensor_mgr->update_thresholds) {
		sensor_mgr->update_thresholds(sensor_mgr);
	}

	if (!sensor_mgr->req_active && active_req) {
		/* Advertise there is now an active request available */
		pthread_mutex_lock(&(sensor_mgr->req_wait_mutex));
		sensor_mgr->req_active = 1;
		pthread_cond_broadcast(&(sensor_mgr->req_wait_cond));
		pthread_mutex_unlock(&(sensor_mgr->req_wait_mutex));
	} else {
		sensor_mgr->req_active = active_req;
	}
	THERM_MUTEX_UNLOCK(&ts_clnt_mtx);
	return 0;
}

static int notify_clnts(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_client_type *client = NULL;
	enum sensor_notify_event_type thresh_event;

	if (sensor_mgr == NULL)
		return -(EINVAL);

	client = sensor_mgr->client_list;

	THERM_MUTEX_LOCK(&ts_clnt_mtx);
	while (client != NULL) {
		if (client->request_active) {
			struct thresholds_req_t *thresh = &client->request.thresh;

			/* Notify clients of thresh crossings */
			thresh_event = SENSOR_NOTIFY_NORMAL_THRESH_EVENT;
			if (thresh->high_valid &&
			    (sensor_mgr->last_reading >= thresh->high)) {
				thresh_event = SENSOR_NOTIFY_HIGH_THRESH_EVENT;
			} else if (thresh->low_valid &&
				   (sensor_mgr->last_reading <= thresh->low)) {
				thresh_event = SENSOR_NOTIFY_LOW_THRESH_EVENT;
			}

			if (thresh_event != SENSOR_NOTIFY_NORMAL_THRESH_EVENT) {
				client->request_active = 0;
				client->request.notify_cb_func(client,
							       thresh_event,
							       sensor_mgr->last_reading,
							       client->request.notify_cb_data);
			}
		}
		client = client->next_clnt;
	}
	THERM_MUTEX_UNLOCK(&ts_clnt_mtx);
	update_active_thresh(sensor_mgr);
	return 0;
}

static int validate_clnt(struct sensor_client_type *clnt)
{
	struct sensors_mgr_sensor_info *sensor_mgr = sensor_list;
	struct sensor_client_type   *sensor_clnt = NULL;
	uint8_t sensor_valid = 0;
	uint8_t clnt_valid = 0;

	if ((clnt == NULL) || (clnt->sensor_mgr == NULL)) {
		msg("%s: Invalid args.\n", __func__);
		return -(EINVAL);
	}

	/* Validate the device pointer */
	while (sensor_mgr != NULL) {
		if (sensor_mgr == clnt->sensor_mgr) {
			sensor_valid = 1;
			break;
		}
		sensor_mgr = sensor_mgr->next_sensor;
	}

	if (sensor_valid == 0) {
		msg("%s: Invalid sensor.\n", __func__);
		return -(EFAULT);
	}

	THERM_MUTEX_LOCK(&ts_clnt_mtx);
	/* Validate the client pointer exists for device */
	sensor_clnt = sensor_mgr->client_list;
	while (sensor_clnt != NULL) {
		if (sensor_clnt == clnt) {
			clnt_valid = 1;
			break;
		}
		sensor_clnt = sensor_clnt->next_clnt;
	}
	THERM_MUTEX_UNLOCK(&ts_clnt_mtx);

	if (clnt_valid == 0) {
		msg("%s: Invalid clnt.\n", __func__);
		return -(EFAULT);
	}
	return 0;
}

int sensors_manager_get_list(struct sensor_info_type *info_arr, uint32_t *info_arr_len)
{
	uint32_t dev_idx;
	struct sensors_mgr_sensor_info *curr = sensor_list;

	if ((info_arr == NULL) && (info_arr_len == NULL)) {
		msg("%s: Invalid args.\n", __func__);
		return -(EINVAL);
	}

	if (info_arr == NULL) {
		/* Interpret as request for number of sensors present. */
		*info_arr_len = sensor_cnt + alias_cnt;
		return 0;
	}

	*info_arr_len = MIN(*info_arr_len, sensor_cnt + alias_cnt);

	curr = sensor_list;
	for (dev_idx = 0; (dev_idx < *info_arr_len) && (curr != NULL); dev_idx++) {
		strlcpy(info_arr[dev_idx].name, curr->name, MAX_SENSOR_NAME_LEN);
		if ((curr->alias != NULL) &&
		    (++dev_idx < *info_arr_len)) {
			/* Copy alias name over if possible. */
			strlcpy(info_arr[dev_idx].name, curr->alias, MAX_SENSOR_NAME_LEN);
		}
		curr = curr->next_sensor;
	}

	return 0;
}

int sensors_get_alias(char *sensor_name, char *alias_dest)
{
	struct sensors_mgr_sensor_info *curr = sensor_list;
	while (curr != NULL) {
		if (curr->alias != NULL && strncmp(sensor_name, curr->name,
						   MAX_SENSOR_NAME_LEN) == 0) {
			strlcpy(alias_dest, curr->alias, MAX_SENSOR_NAME_LEN);
			return 0;
		}
		curr = curr->next_sensor;
	}
	return 1;
}

sensor_clnt_handle sensors_manager_reg_clnt(const char *sensor_name)
{
	struct sensor_client_type *client = NULL;
	struct sensors_mgr_sensor_info *sensor_mgr = NULL;

	if (sensor_name == NULL) {
		msg("%s: Invalid args.\n", __func__);
		return client;
	}

	sensor_mgr = find_sensor(sensor_name);

	if (sensor_mgr == NULL) {
		msg("%s: Invalid sensor %s.\n", __func__, sensor_name);
		return client;
	}

	client = malloc(sizeof(struct sensor_client_type));

	if (client == NULL) {
		msg("%s: Alloc. failed for %s.\n", __func__, sensor_name);
		return client;
	}
	memset(client, 0x0, sizeof(struct sensor_client_type));

	THERM_MUTEX_LOCK(&ts_clnt_mtx);
	/* Insert the client */
	client->sensor_mgr = sensor_mgr;
	client->next_clnt = sensor_mgr->client_list;
	sensor_mgr->client_list = client;
	THERM_MUTEX_UNLOCK(&ts_clnt_mtx);

	return client;
}

int sensors_manager_dereg_clnt(sensor_clnt_handle clnt)
{
	struct sensor_client_type   *prev = NULL;
	struct sensor_client_type   *curr = NULL;
	struct sensors_mgr_sensor_info *sensor_mgr = NULL;
	struct sensor_client_type   *client = clnt;
	int ret_val = 0;

	if (client == NULL) {
		msg("%s: Invalid args.\n", __func__);
		return -(EINVAL);
	}

	ret_val = validate_clnt(client);
	if (ret_val != 0)
		return ret_val;

	sensor_mgr = client->sensor_mgr;

	/* Search for client in dev client list */
	curr = sensor_mgr->client_list;

	THERM_MUTEX_LOCK(&ts_clnt_mtx);
	while (curr != NULL) {
		if ((curr == client) && (prev != NULL)) {
			/* In middle of list */
			prev->next_clnt = curr->next_clnt;
			break;
		} else if ((curr == client)) {
			/* At begining of list */
			sensor_mgr->client_list = curr->next_clnt;
			break;
		}
		prev = curr;
		curr = curr->next_clnt;
	}
	THERM_MUTEX_UNLOCK(&ts_clnt_mtx);

	if (client->request_active) {
		/* Update thresholds without client request. */
		update_active_thresh(sensor_mgr);
	}
	free(client);

	return 0;
}

int sensors_manager_read(sensor_clnt_handle clnt)
{
	struct sensors_mgr_sensor_info *sensor_mgr = NULL;
	struct sensor_client_type   *client = clnt;
	int ret_val = INT32_MIN;

	if (client == NULL) {
		msg("%s: Invalid args.\n", __func__);
		return ret_val;
	}

	if (validate_clnt(client) != 0)
		return ret_val;

	sensor_mgr = client->sensor_mgr;

	ret_val = sensor_mgr->get_temperature(sensor_mgr);
	if(!strncmp(sensor_mgr->name, "bcl", 3)) {
		thermalmsg(LOG_LVL_DBG, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
			   "%s:%s:%d mA\n", SENSORS,
			   sensor_mgr->name, ret_val);
	} else if (!strncmp(sensor_mgr->name, "bcm", 3)) {
		thermalmsg(LOG_LVL_DBG, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
			   "%s:%s:%d mPercent\n", SENSORS,
			   sensor_mgr->name, sensor_mgr->last_reading);
	} else {
		thermalmsg(LOG_LVL_DBG, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
			   "%s:%s:%d mC\n", SENSORS,
			   sensor_mgr->name, ret_val);
	}
	return ret_val;
}

int sensors_manager_set_thresh_lvl(sensor_clnt_handle clnt,
		       struct sensor_thresh_req *thresh_info)
{
	struct sensors_mgr_sensor_info *sensor_mgr = NULL;
	struct sensor_client_type   *client = clnt;
	int ret_val = 0;

	if (client == NULL) {
		msg("%s: Invalid args.\n", __func__);
		return -(EINVAL);
	}

	ret_val = validate_clnt(client);
	if (ret_val != 0)
		return ret_val;

	sensor_mgr = client->sensor_mgr;

	THERM_MUTEX_LOCK(&ts_clnt_mtx);
	if ( thresh_info == NULL) {
		/* Clear client request */
		client->request_active = 0;
		dbgmsg("%s: %s clear request.\n", __func__, sensor_mgr->name);
	} else if (validate_thresh_info(thresh_info) == 0) {
		memcpy(&client->request, thresh_info,
		       sizeof(struct sensor_thresh_req));
		client->request_active = 1;
		dbgmsg("%s: %s Hi(%d) %d, Lo(%d) %d, Interval(%d) %d\n",
		       __func__, sensor_mgr->name,
		       thresh_info->thresh.high_valid, thresh_info->thresh.high,
		       thresh_info->thresh.low_valid, thresh_info->thresh.low,
		       thresh_info->polling_interval_valid,
		       thresh_info->polling_interval);
	}
	THERM_MUTEX_UNLOCK(&ts_clnt_mtx);

	/* Update thresholds. */
	update_active_thresh(sensor_mgr);
	return 0;
}

/* Sensor internal manager functions */
static void sensor_wait(struct sensors_mgr_sensor_info *sensor_mgr)
{
	if (sensor_mgr->wait)
		sensor_mgr->wait(sensor_mgr);
	else {
		uint32_t polling_interval =
			(sensor_mgr->active_thresh.polling_interval_valid)?
			(sensor_mgr->active_thresh.polling_interval):
			(sensor_mgr->default_polling_interval);
		dbgmsg("%s: %s Wait start. %dms\n", __func__, sensor_mgr->name, polling_interval);
		usleep(polling_interval*1000);
		dbgmsg("%s: %s Wait done.\n", __func__, sensor_mgr->name);
	}
}

static void *sensor_monitor(void *vsensor_mgr)
{
	struct sensors_mgr_sensor_info *sensor_mgr = vsensor_mgr;

	while (sensor_mgr->thread_shutdown != 1) {
		/* Wait here until there is actually a request to process */
		if (!sensor_mgr->req_active) {
			dbgmsg("%s: %s Wait for client request.\n", __func__, sensor_mgr->name);
			pthread_mutex_lock(&(sensor_mgr->req_wait_mutex));
			while (!sensor_mgr->req_active) {
				pthread_cond_wait(&(sensor_mgr->req_wait_cond),
						&(sensor_mgr->req_wait_mutex));
			}
			pthread_mutex_unlock(&(sensor_mgr->req_wait_mutex));
		}
		dbgmsg("%s: %s Sensor wait.\n", __func__, sensor_mgr->name);
		sensor_wait(sensor_mgr);

		sensor_mgr->last_reading = sensor_mgr->get_temperature(sensor_mgr);
		dbgmsg("%s: %s Reading %d .\n", __func__, sensor_mgr->name,
		       sensor_mgr->last_reading);
		if (!strncmp(sensor_mgr->name, "bcl", 3)) {
			thermalmsg(LOG_LVL_DBG, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
				   "%s:%s:%d mA\n", SENSORS,
				   sensor_mgr->name, sensor_mgr->last_reading);
		} else if (!strncmp(sensor_mgr->name, "bcm", 3)) {
			thermalmsg(LOG_LVL_DBG, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
				   "%s:%s:%d mPercent\n", SENSORS,
				   sensor_mgr->name, sensor_mgr->last_reading);
		} else {
			thermalmsg(LOG_LVL_DBG, (LOG_LOGCAT | LOG_LOCAL_SOCKET),
				   "%s:%s:%d mC\n", SENSORS,
				   sensor_mgr->name, sensor_mgr->last_reading);
		}
		notify_clnts(sensor_mgr);
	}

	return NULL;
}

int sensors_manager_add_sensor(struct sensors_mgr_sensor_info *sensor_mgr)
{
	if (sensor_mgr == NULL) {
		msg("%s: Invalid argument\n", __func__);
		return -(EINVAL);
	}

	if (!sensor_mgr->default_polling_interval)
		sensor_mgr->default_polling_interval = SENSOR_DEFAULT_POLLING_INTERVAL;
	/* Just add new device to begining of list */
	sensor_mgr->next_sensor = sensor_list;
	sensor_list = sensor_mgr;
	sensor_cnt++;
	if (sensor_mgr->alias)
		alias_cnt++;

	pthread_mutex_init(&(sensor_mgr->req_wait_mutex), NULL);
	pthread_cond_init(&(sensor_mgr->req_wait_cond), NULL);
	pthread_create(&(sensor_mgr->monitor_thread), NULL,
			       sensor_monitor, sensor_mgr);
	return 0;
}

int sensors_manager_rm_sensor(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensors_mgr_sensor_info *prev = NULL;
	struct sensors_mgr_sensor_info *curr = sensor_list;
	int ret_val = 0;

	if (sensor_mgr == NULL) {
		msg("%s: Invalid argument\n", __func__);
		return -(EINVAL);
	}
	/* Find the previous sensor in list */
	while (curr != NULL) {
		if ((curr == sensor_mgr) && (prev != NULL)) {
			/* In middle of list */
			prev->next_sensor = curr->next_sensor;
			break;
		} else if ((curr == sensor_mgr)) {
			/* At begining of list */
			sensor_list = curr->next_sensor;
			break;
		}
		prev = curr;
		curr = curr->next_sensor;
	}
	/* TODO clean up clients */
	sensor_mgr->thread_shutdown = 1;
	pthread_join(sensor_mgr->monitor_thread, NULL);
	pthread_cond_destroy(&(sensor_mgr->req_wait_cond));
	pthread_mutex_destroy(&(sensor_mgr->req_wait_mutex));
	if (sensor_mgr->alias)
		alias_cnt--;

	sensor_mgr->shutdown(sensor_mgr->data);
	sensor_cnt--;

	return ret_val;
}

int sensors_manager_init(void)
{
	info("%s: Init\n", __func__);
	return 0;
}

void sensors_manager_release(void)
{
	info("%s: Release\n", __func__);
}
