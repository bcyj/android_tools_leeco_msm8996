/*===========================================================================

  sensors-virtual.c

  DESCRIPTION
  Virtual sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stringl.h>
#include "thermal.h"
#include "sensors_manager.h"
#include "sensors_manager_internal.h"
#include "sensors-virtual.h"
#include <poll.h>

static struct timespec time_val;

struct reg_sensor_info {
	sensor_clnt_handle  sensor_clnt;
	int                 curr_reading;
};

struct v_sensor_calc_t {
	char			*desc;
	/* regular sensors */
	struct reg_sensor_info	*sensors_arr;
	struct reg_sensor_info	trip_sensor;
	int			state;
	int			out_value;
};

struct v_sensor_info {
        struct sensors_mgr_sensor_info vs;
        struct virtual_setting *setting;
        struct v_sensor_calc_t rs;
	struct v_sensor_info *next;
	/* setup function needs to be called before get_temperature */
	int (*setup)(struct sensors_mgr_sensor_info *sensor);
	pthread_mutex_t wait_mutex;
	pthread_cond_t wait_cond;
	int threshold_reached;
};

struct v_sensor_list{
       struct v_sensor_info    *list;
       int                     vs_cnt;
};

static struct v_sensor_list virtual_sensor_list;
static int v_sensor_init;

static void trip_sens_thresh_notify(sensor_clnt_handle clnt,
			      enum sensor_notify_event_type event,
			      int reading, void *data)
{
	struct v_sensor_info *vs_info = (struct v_sensor_info *)data;

	if (data == NULL) {
		msg("%s: Invalid data", __func__);
		return;
	}

	dbgmsg("%s: %s, Update recieved trip sensor %s %d", __func__,
	       vs_info->vs.name,
	       vs_info->setting->trip_sensor,
	       reading);

	/* Notify the waiting thread */
	pthread_mutex_lock(&vs_info->wait_mutex);
	vs_info->threshold_reached = 1;
	pthread_cond_broadcast(&vs_info->wait_cond);
	pthread_mutex_unlock(&vs_info->wait_mutex);
}

static void handle_trip_sens_thresh_sig(struct v_sensor_info *vs_info)
{
	int     temp;
	uint8_t transition = 0;
	struct sensor_thresh_req thresh;

	/* Get temp and handle */
	temp = sensors_manager_read(vs_info->rs.trip_sensor.sensor_clnt);
	dbgmsg("%s: %s read trip sensor %s %dmC", __func__, vs_info->vs.name,
			vs_info->setting->trip_sensor, temp);
	if ((vs_info->rs.state == 0) &&
	    (temp >= vs_info->setting->set_point)) {
		transition = 1;
	} else if ((vs_info->rs.state == 1) &&
		   (temp <= vs_info->setting->set_point_clr)) {
		transition = 1;
	}

	if (transition) {
		vs_info->rs.state = (vs_info->rs.state)?(0):(1);
		dbgmsg("%s: %s Transition State %d", __func__,
		       vs_info->setting->trip_sensor,
		       vs_info->rs.state);
	}

	memset(&thresh, 0x0, sizeof(thresh));
	thresh.notify_cb_func = trip_sens_thresh_notify;
	thresh.notify_cb_data = (void*)vs_info;
	if (vs_info->rs.state == 0) {
		thresh.thresh.high_valid = 1;
		thresh.thresh.high = vs_info->setting->set_point;
	} else {
		thresh.thresh.low_valid = 1;
		thresh.thresh.low = vs_info->setting->set_point_clr;
	}

	pthread_mutex_lock(&vs_info->wait_mutex);
	/* Mark off handled item */
	vs_info->threshold_reached = 0;
	pthread_mutex_unlock(&vs_info->wait_mutex);

	/* Set Thresh */
	sensors_manager_set_thresh_lvl(vs_info->rs.trip_sensor.sensor_clnt, &thresh);
}

void vs_calc_temperature(struct sensors_mgr_sensor_info *sensor)
{

	int value = 0;
	int weight_sum = 0;
	uint8_t i = 0;
	struct v_sensor_info *vs_info = (struct v_sensor_info *)sensor->data;
	struct v_sensor_calc_t *vs_data = &vs_info->rs;

	vs_data->out_value = 0;

	for (i = 0; i < vs_info->setting->list_cnt; i++) {
		if (vs_data->sensors_arr[i].sensor_clnt == NULL)
			  continue;
		vs_data->sensors_arr[i].curr_reading =
			sensors_manager_read(vs_data->sensors_arr[i].\
					sensor_clnt);
		value = vs_data->sensors_arr[i].curr_reading *
			vs_info->setting->weight_list[i];
		info("%s: read[%d] %s %d mC, weight[%d] %d", __func__, i,
			vs_data->sensors_arr[i].sensor_clnt->sensor_mgr->name,
			vs_data->sensors_arr[i].curr_reading, i,
			vs_info->setting->weight_list[i]);
		weight_sum +=vs_info->setting->weight_list[i];
		vs_data->out_value += value;
	}
	vs_data->out_value = vs_data->out_value/(weight_sum);
	dbgmsg("%s: Virtual sensor %s calculate results: %d mC", __func__,
				vs_info->vs.name, vs_data->out_value);
	return;
}

void vs_interrupt_wait(struct sensors_mgr_sensor_info *sensor)
{
	int wait_ret = 0;
	struct v_sensor_info  *vs_info = (struct v_sensor_info *)sensor->data;

	pthread_mutex_lock(&vs_info->wait_mutex);
	if (vs_info->rs.state) {
		/*
		 * If state == 1 (above set point state), wait for the condition
		 * that the set point clear threshold is reached or polling time is
		 * expired.
		 */
		clock_gettime(CLOCK_REALTIME, &time_val);
		time_val.tv_sec += vs_info->vs.default_polling_interval / 1000;
		time_val.tv_nsec += (vs_info->vs.\
				default_polling_interval % 1000) * 1000000;
		while(!vs_info->threshold_reached) {
			wait_ret = pthread_cond_timedwait(&vs_info->wait_cond,
				&vs_info->wait_mutex, &time_val);
			if (wait_ret == ETIMEDOUT) {
				msg("%s: Time out after %d.%03d(sec)", __func__,
					vs_info->vs.default_polling_interval / 1000,
					(vs_info->vs.\
					default_polling_interval % 1000));
				break;
			}
		}
		vs_calc_temperature(sensor);
	} else {
		/*
		 * If state == 0 (below set point clear state or never crossed
		 * set point to begin with), wait for the condition that set point
		 * threshold is reached.
		 */
		while(!vs_info->threshold_reached)
			pthread_cond_wait(&vs_info->wait_cond, &vs_info->wait_mutex);
		vs_calc_temperature(sensor);
	}
	pthread_mutex_unlock(&vs_info->wait_mutex);
	handle_trip_sens_thresh_sig(vs_info);
}

/*
 * NOTE: vs_setup() function does not enable the sensor
 * or set thresholds. This should be done in the target-specific setup
 */
int vs_setup(struct sensors_mgr_sensor_info *sensor)
{
	int idx;
	int sensor_count = 0;
	struct v_sensor_info *vs_info = (struct v_sensor_info *)sensor->data;
	struct virtual_setting *cfg = vs_info->setting;
	struct v_sensor_calc_t *vs_data = &vs_info->rs;

        if (!v_sensor_init) {
		msg("%s: Virtual sensor not initialized.\n", __func__);
		return sensor_count;
	}

	if ((cfg->list_cnt == 0) || (cfg->sensor_list == NULL)) {
		msg("No valid sensors listed");
		return sensor_count;
	}

	if ((cfg->trip_sensor == NULL)) {
		msg("No valid trip sensor listed");
		return sensor_count;
	}

	if (cfg->sampling_period_ms == 0)
		cfg->sampling_period_ms = SAMPLING_MS_DEFAULT;

	vs_data->trip_sensor.sensor_clnt =
			sensors_manager_reg_clnt(cfg->trip_sensor);
	if (vs_data->trip_sensor.sensor_clnt == NULL) {
		msg("%s: Sensor clnt create fail %s\n", __func__,
				cfg->trip_sensor);
			return sensor_count;
	}

	/* Allocate handle array and set to NULL */
	vs_data->sensors_arr = (struct reg_sensor_info *)
			malloc(sizeof(struct reg_sensor_info) *
			       cfg->list_cnt);
	if (vs_data->sensors_arr == NULL) {
		msg("%s: Allocate failed", __func__);
			return sensor_count;
	}
	memset(vs_data->sensors_arr, 0x0, sizeof(struct reg_sensor_info) *
			cfg->list_cnt);

	for (idx = 0; idx < cfg->list_cnt; idx++) {
		vs_data->sensors_arr[idx].sensor_clnt =
			sensors_manager_reg_clnt(cfg->sensor_list[idx]);

		/*
		 * Don't completely fail on a single sensor client create
		 * fail.
		 */
		if (vs_data->sensors_arr[idx].sensor_clnt == NULL)
			msg("%s: Sensor clnt create fail %s\n", __func__,
					cfg->sensor_list[idx]);
		else {
			sensor_count++;
			dbgmsg("%s: sensors: %s", __func__,
					cfg->sensor_list[idx]);
		}

		if (sensor_count == 0) {
			msg("No valid sensors listed");
			free(vs_data->sensors_arr);
			return sensor_count;
		}
	}

	/* Set mask to all CPU's Trigger setting all thresholds */
	vs_info->threshold_reached = 1;

	/* Set initial thresholds */
	handle_trip_sens_thresh_sig(vs_info);
	return sensor_count;
}

void vs_shutdown(struct sensors_mgr_sensor_info *sensor)
{
	int i = 0;
	struct v_sensor_info *vs_info = (struct v_sensor_info *)sensor->data;
	struct v_sensor_calc_t *vs_data = &vs_info->rs;

	if (vs_data->trip_sensor.sensor_clnt)
		sensors_manager_dereg_clnt(vs_data->trip_sensor.sensor_clnt);

	for (i = 0; i < vs_info->setting->list_cnt; i++) {
	       if (vs_data->sensors_arr[i].sensor_clnt)
		       sensors_manager_dereg_clnt(vs_data->sensors_arr[i].\
				       sensor_clnt);
	}
}

int vs_get_temperature(struct sensors_mgr_sensor_info *sensor)
{
	struct v_sensor_info *vs_info = (struct v_sensor_info *)sensor->data;
	struct v_sensor_calc_t *vs_data = &vs_info->rs;
	int temp;

	pthread_mutex_lock(&vs_info->wait_mutex);
	if (!vs_info->rs.state)
		vs_calc_temperature(sensor);
	dbgmsg("%s: Virtual sensor %s read: %d mC", __func__,
			vs_info->vs.name, vs_data->out_value);
	temp = vs_data->out_value;
	pthread_mutex_unlock(&vs_info->wait_mutex);

	return temp;
}

static int add_virtual_sensor(struct v_sensor_info *sensor)
{
	int ret_val = 0;

	if (sensor->setup(&sensor->vs) == 0) {
		ret_val = -(EFAULT);
		goto error_handler;
	}

	sensors_manager_add_sensor(&sensor->vs);

error_handler:
	return ret_val;
}

static int add_virtual_sensors_set(struct v_sensor_info *sensor_list)
{
	int ret_val = 0;

	while(sensor_list) {
		if (add_virtual_sensor(sensor_list) != 0) {
			msg("%s: Error adding %s\n", __func__,
			    sensor_list->vs.name);
			ret_val = -(EFAULT);
			break;
		}
		sensor_list=sensor_list->next;
	}

	return ret_val;
}

static struct setting_info *get_virtual_sensors_setting(\
		struct setting_info *list)
{
	struct setting_info *v_tmp = NULL;
	struct setting_info *v_list_head = NULL;

	if (list == NULL) {
		msg("%s: Invalid args.\n", __func__);
		return NULL;
	}

	while (list) {
		if (list->algo_type == VIRTUAL_SENSOR_TYPE && !list->err_disable) {
			v_tmp = (struct setting_info *)malloc(\
					sizeof(struct setting_info));
			if (!v_tmp) {
				msg("%s: Failed to allocate memory for v_tmp\n",
						__func__);
				return NULL;
			}
			memcpy(v_tmp, list, sizeof(struct setting_info));

			v_tmp->next = v_list_head;
			v_list_head = v_tmp;
			list = list->next;
		} else
			list = list->next;
	}

	return v_list_head;
}

int virtual_sensors_init(struct thermal_setting_t *thermal_setting,
		const char *pFName)
{
	int ret_val = 0;
	int i = 0;
	struct setting_info *list = NULL;
	struct setting_info *v_list = NULL;
	struct v_sensor_info *sensor = NULL;
	struct v_sensor_info *sensor_list_head = NULL;

	info("Loading config file for virtual sensor\n");
	#ifdef ENABLE_OLD_PARSER
	load_config(thermal_setting, pFName);
	#else
	load_config(thermal_setting, pFName, LOAD_VIRTUAL_FLAG);
	#endif
	/* list of setting info from entire config files */
	list = thermal_setting->list;
	if (list == NULL) {
		info("%s: No setting lists.\n", __func__);
		return ret_val;
	}

	/* create a list that contains virtual sensors setting only */
	v_list = get_virtual_sensors_setting(list);
	if (!v_list) {
		msg("%s: Error getting virtual sensors set\n", __func__);
		ret_val = -(EFAULT);
		goto error_handler;
	}

	while (v_list) {
		sensor = (struct v_sensor_info *)malloc(\
				sizeof(struct v_sensor_info));
		if (!sensor) {
			msg("%s: Fail to allocate memory for sensor\n",
					__func__);
			ret_val = -ENOMEM;
			goto error_handler;
		}
		memset(sensor, 0, sizeof(struct v_sensor_info));

		sensor->vs.name = (char *)malloc(sizeof(char) *
				(strlen(v_list->desc)+1));
		if (!sensor->vs.name) {
			msg("%s: Fail to allocate memory for sensor->vs.name\n"
					, __func__);
			free(sensor);
			ret_val = -ENOMEM;
			goto error_handler;
		}
		strlcpy(sensor->vs.name, v_list->desc, strlen(v_list->desc)+1);
		sensor->vs.alias = NULL;
		sensor->vs.shutdown = vs_shutdown;
		sensor->vs.get_temperature = vs_get_temperature;
		sensor->vs.wait = vs_interrupt_wait;;
		sensor->vs.update_thresholds = NULL;
		sensor->vs.data = sensor;
		sensor->vs.interrupt_enable = 1,
		sensor->setup = vs_setup;
		sensor->setting = &(v_list->data.v);
		sensor->vs.default_polling_interval = v_list->data.v.sampling_period_ms;
		print_setting(v_list);
		v_list = v_list->next;
		sensor->next = sensor_list_head;
		sensor_list_head = sensor;
		i++;
	}

	virtual_sensor_list.list = sensor_list_head;
	virtual_sensor_list.vs_cnt = i;
	v_sensor_init = 1;

	if (add_virtual_sensors_set(sensor_list_head)!= 0) {
		msg("%s: Error adding virtual sensors set\n", __func__);
		ret_val = -(EFAULT);
		goto error_handler;
	}

error_handler:
	return ret_val;
}

