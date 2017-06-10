/*===========================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/


#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <stringl.h>
#include <signal.h>
#include "thermal.h"
#include "sensors_manager.h"
#include "devices_manager.h"
#include "pid_algorithm.h"
#include "server/thermal_lib_common.h"
#include "thermal_server.h"

/* Limit MAX SAMPLE PERIOD to 1000000ms */
#define PID_SAMPLE_PERIOD (1000000)
#define I_SAMPLES (3)

#define PID_TIMER_EV  0x1
#define PID_THRESH_EV 0x2
#define PID_OVERRIDE_EV 0x4
#define PID_CONFIG_SET_EV 0x8
#define MAX_PID_INSTANCES_SUPPORTED 32

#define ONE_CELSIUS   (1000)

#define TIMER_SAMPLING_SET(s_grp, sampling) { \
	(s_grp.timer_val.it_value.tv_sec = (sampling / 1000)); \
	(s_grp.timer_val.it_value.tv_nsec = ((sampling % 1000) * 1000000)); \
	}

static pthread_t        pid_thread;
static pthread_cond_t   wait_cond = PTHREAD_COND_INITIALIZER;
static uint32_t         pid_ev_mask;
static uint32_t         thresh_reached_mask;
static uint32_t         sampling_group_mask;
static uint8_t          override_mode;
static pthread_mutex_t  wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  config_set_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint8_t sampling_group_count;

struct err_history_t {
	int *err;
	int err_I_total;
	uint32_t curr_idx;
};

struct pid_algo_t {
	char                 *desc;
	struct pid_setting   *setting;
	sensor_clnt_handle    sensor_clnt;
	device_clnt_handle    dev_clnt;
	int                   min_op_lvl;
	int                   max_op_lvl;
	int                   dev_op_lvl;
	uint32_t              num_of_lvls;
	int                   applied_freq;
	struct device_lvl_info *lvl_arr;
	uint8_t               state;  /* 0 Inactive, 1 Mitigate */
	struct err_history_t  err_info; /* Error info */
	struct err_history_t  freq_err;
	int                   active_set_point;
	int                   active_set_point_clr;
	int                   prev_therm_adjustment;
	int                   prev_freq_adjustment;
	uint8_t               disabled;
	uint8_t               sampling_group_id;
};

struct sampling_group {
	uint32_t sampling;
	uint32_t pid_clnt_state;
	uint8_t enabled;  /* 0 Timer Disabled, 1 Timer Enabled */
	struct sigevent sig_evt;
	timer_t timer_id;
	struct timespec   timer_start;
	struct itimerspec timer_val;
};

static struct sampling_group sampling_groups[MAX_PID_INSTANCES_SUPPORTED];
static struct pid_algo_t pid_algo_clnt[MAX_PID_INSTANCES_SUPPORTED];
static uint32_t pid_clnt_cnt;
static struct thermal_msg_data client_config[MAX_PID_INSTANCES_SUPPORTED];
static uint32_t cfg_set_cnt;

static int get_err(struct err_history_t *err_info, int idx)
{
	idx = idx % I_SAMPLES;

	idx += (int)err_info->curr_idx;

	if (idx < 0) {
		idx += I_SAMPLES;
	} else if (idx >= I_SAMPLES) {
		idx -= I_SAMPLES;
	}

	return err_info->err[idx];
}

static void set_curr_err(struct err_history_t *err_info, int err, uint32_t sample_cnt)
{
	uint32_t idx;

	if (sample_cnt > I_SAMPLES)
		sample_cnt = I_SAMPLES;

	/* Subtract out error values to be replaced */
	for (idx = 0; idx < sample_cnt; idx++) {
		err_info->err_I_total -= get_err(err_info, (int)idx+1);
	}

	/* Add in the newest entries */
	for (idx = 0; idx < sample_cnt; idx++) {
		++(err_info->curr_idx);
		err_info->curr_idx %= I_SAMPLES;
		err_info->err[err_info->curr_idx] = err;
		err_info->err_I_total += err;
	}
}

static void pid_thresh_notify(sensor_clnt_handle clnt,
			      enum sensor_notify_event_type event,
			      int reading, void *data)
{
	if (NULL == clnt) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	dbgmsg("%s: PID Id %s, Update recieved %s %d", __func__,
	       pid_algo_clnt[(uintptr_t)data].desc,
	       pid_algo_clnt[(uintptr_t)data].setting->sensor,
	       reading);

	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	thresh_reached_mask |= (0x1U << (uintptr_t)data);
	pid_ev_mask |= PID_THRESH_EV;
	pthread_cond_broadcast(&wait_cond);
	pthread_mutex_unlock(&wait_mutex);
}

static int pid_settimer(struct sampling_group *sampling_group)
{
	int ret_val = 0;

	ret_val = clock_gettime(CLOCK_MONOTONIC, &sampling_group->timer_start);
	if (ret_val) {
		msg("%s: Error getting time.\n", __func__);
		return ret_val;
	}

	ret_val = timer_settime(sampling_group->timer_id, 0,
			        &sampling_group->timer_val, NULL);
	if (ret_val) {
		msg("%s: Error setting timer.\n", __func__);
		return ret_val;
	}

	dbgmsg("%s: Start timer %d.%03d(sec)", __func__,
	       (int)sampling_group->timer_val.it_value.tv_sec,
	       (int)sampling_group->timer_val.it_value.tv_nsec / 1000000);

	return ret_val;
}

static int override_notify(int mode, void *data, void *reserved)
{
	dbgmsg("%s: PID mode %d", __func__, mode);

	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	if (override_mode != mode) {
		override_mode = (uint8_t)mode;
		pid_ev_mask |= PID_OVERRIDE_EV;
		pthread_cond_broadcast(&wait_cond);
	}
	pthread_mutex_unlock(&wait_mutex);

	return 0;
}

static void pid_timer_expired(union sigval sval)
{
	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	pid_ev_mask |= PID_TIMER_EV;
	sampling_group_mask |= (0x1U << sval.sival_int);
	pthread_cond_broadcast(&wait_cond);
	pthread_mutex_unlock(&wait_mutex);
}

static int config_parameter_set_notify(int num_cfg, void *data, void *reserved)
{
	uint8_t idx;

	if (reserved == NULL ||
	    num_cfg <= 0 ||
	    (unsigned int)num_cfg > MAX_CONFIG_INSTANCES_SUPPORTED)
		return -1;

	pthread_mutex_lock(&config_set_mutex);
	struct thermal_msg_data *current = (struct thermal_msg_data *)reserved;

	cfg_set_cnt = 0;
	for (idx = 0; idx < num_cfg; idx++) {
		if (current[idx].msg_type != PID_ALGO_TYPE)
			continue;

		if (cfg_set_cnt >= MAX_PID_INSTANCES_SUPPORTED)
			break;

		/* copy data to algo local structure */
		memset(&client_config[cfg_set_cnt], 0,
		       sizeof(struct thermal_msg_data));
		client_config[cfg_set_cnt].msg_type = current[idx].msg_type;
		strlcpy(client_config[cfg_set_cnt].client_name,
		        current[idx].client_name, CLIENT_NAME_MAX);
		strlcpy(client_config[cfg_set_cnt].config.config_desc,
		         current[idx].config.config_desc, MAX_ALGO_DESC);
		client_config[cfg_set_cnt].config.fields_mask =
			current[idx].config.fields_mask;
		client_config[cfg_set_cnt].config.disable =
			current[idx].config.disable;
		client_config[cfg_set_cnt].config.sampling =
			current[idx].config.sampling;
		client_config[cfg_set_cnt].config.d_setting.set_point =
			current[idx].config.d_setting.set_point;
		client_config[cfg_set_cnt].config.d_setting.set_point_clr =
			current[idx].config.d_setting.set_point_clr;
		strlcpy(client_config[cfg_set_cnt].config.d_setting.device,
		        current[idx].config.d_setting.device,
		        DEVICES_MAX_NAME_LEN);
		cfg_set_cnt++;
	}

	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	if (cfg_set_cnt > 0) {
		pid_ev_mask |= PID_CONFIG_SET_EV;
		pthread_cond_broadcast(&wait_cond);
	}
	pthread_mutex_unlock(&wait_mutex);

	pthread_mutex_unlock(&config_set_mutex);

	return 0;
}

static int handle_sampling_adjust(struct pid_algo_t *pid_info, uint32_t algo_idx, uint32_t sampling)
{
	uint8_t s_idx = 0;
	int ret_val = 0;

	if (pid_info == NULL ||
	    algo_idx >= pid_clnt_cnt ||
	    sampling < SAMPLING_MS_MINIMUM)
		return -1;

	for (s_idx = 0; s_idx < sampling_group_count; s_idx++) {
		if (sampling == sampling_groups[s_idx].sampling)
			break;
	}
	if(s_idx >= sampling_group_count) {

		if (sampling_group_count == MAX_PID_INSTANCES_SUPPORTED)
			return -1;

		s_idx = sampling_group_count;
		sampling_group_count++;
		sampling_groups[s_idx].sampling = sampling;
		memset(&sampling_groups[s_idx].sig_evt, 0,
		       sizeof(struct sigevent));
		sampling_groups[s_idx].sig_evt.sigev_notify =
				SIGEV_THREAD;
		sampling_groups[s_idx].sig_evt.sigev_notify_function =
				pid_timer_expired;
		sampling_groups[s_idx].sig_evt.sigev_value.sival_ptr =
				&sampling_groups[s_idx].timer_id;
		sampling_groups[s_idx].sig_evt.sigev_value.sival_int = s_idx;

		ret_val = timer_create(CLOCK_MONOTONIC,
				       &sampling_groups[s_idx].sig_evt,
				       &sampling_groups[s_idx].timer_id);
		if (ret_val) {
			msg("%s: Error cannot create timer.\n", __func__);
			return -1;
		}

		memset(&sampling_groups[s_idx].timer_val, 0,
		       sizeof(struct itimerspec));
		TIMER_SAMPLING_SET(sampling_groups[s_idx],
				   sampling_groups[s_idx].sampling);
	}

	if (!pid_info->disabled &&
	    (sampling_groups[pid_info->sampling_group_id].pid_clnt_state &
							 (0x1U << algo_idx))) {
		sampling_groups[s_idx].pid_clnt_state |= (0x1U << algo_idx);
		if (sampling_groups[s_idx].pid_clnt_state &&
					!sampling_groups[s_idx].enabled) {
			TIMER_SAMPLING_SET(sampling_groups[s_idx],
					   sampling_groups[s_idx].sampling);
			ret_val = pid_settimer(&sampling_groups[s_idx]);
			if (ret_val) {
				msg("%s: Error cannot settime.\n", __func__);
				sampling_groups[s_idx].pid_clnt_state &=
							(~(0x1U << algo_idx));
				return -1;
			} else {
				sampling_groups[s_idx].enabled = 1;
			}
		}
		sampling_groups[pid_info->sampling_group_id].pid_clnt_state &=
								(~(0x1U << algo_idx));
		if (!sampling_groups[pid_info->sampling_group_id].pid_clnt_state &&
		    sampling_groups[pid_info->sampling_group_id].enabled) {
			/* Disarm the timer */
			TIMER_SAMPLING_SET(sampling_groups[pid_info->sampling_group_id], 0);
			ret_val = pid_settimer(&sampling_groups[pid_info->sampling_group_id]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[pid_info->sampling_group_id].enabled = 0;
		}
	}

	pid_info->sampling_group_id = s_idx;

	return 0;
}

static int handle_set_point_adjust(struct pid_algo_t *pid_info, int set_point, int set_point_clr)
{
	if (pid_info == NULL)
		return -1;

	/* Set requested active set_point and set_point_clr */
	if (set_point != INT_MIN)
		pid_info->active_set_point = set_point;
	if (set_point_clr != INT_MIN)
		pid_info->active_set_point_clr = set_point_clr;

	return 0;
}

static int validate_disabled_config(struct pid_algo_t *pid_info)
{
	if (pid_info == NULL ||
	    pid_info->sensor_clnt == NULL ||
	    pid_info->dev_clnt == NULL ||
	    pid_info->setting == NULL ||
	    pid_info->active_set_point <=
			pid_info->active_set_point_clr ||
	    sampling_groups[pid_info->sampling_group_id].sampling <
					SAMPLING_MS_MINIMUM)
		return -1;

	return 0;
}

static int handle_disable_config_adjust(struct pid_algo_t *pid_info, uint32_t idx, uint8_t disable)
{
	int ret_val = 0;

	if (pid_info == NULL)
		return -1;

	if(!disable) {
		if (validate_disabled_config(pid_info))
			return -1;

		pid_info->disabled = disable;
	} else {
		if (pid_info->sensor_clnt)
			sensors_manager_set_thresh_lvl(pid_info->sensor_clnt,
						       NULL);

		if (pid_info->dev_clnt)
			device_clnt_cancel_request(pid_info->dev_clnt);

		pid_info->dev_op_lvl = pid_info->max_op_lvl;
		pid_info->state = 0;
		pid_info->prev_freq_adjustment = 0;
		pid_info->prev_therm_adjustment = 0;

		/* disarm timer if no other client */
		sampling_groups[pid_info->sampling_group_id].pid_clnt_state &=
									(~(0x1U << idx));
		if (!sampling_groups[pid_info->sampling_group_id].pid_clnt_state &&
		    sampling_groups[pid_info->sampling_group_id].enabled) {
			/* Disarm the timer */
			TIMER_SAMPLING_SET(sampling_groups[pid_info->sampling_group_id], 0);
			ret_val = pid_settimer(&sampling_groups[pid_info->sampling_group_id]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[pid_info->sampling_group_id].enabled = 0;
		}
		pid_info->disabled = disable;
	}
	return 0;
}

/* return idx of algo  if success, negtaive on error */
static int validate_client_config(struct thermal_msg_data *client_config)
{
	uint32_t idx;

	if ((client_config->msg_type != PID_ALGO_TYPE))
		return -1;

	for (idx = 0; idx < pid_clnt_cnt; idx++) {
		if (0 == strcmp(client_config->config.config_desc,
				 pid_algo_clnt[idx].desc))
			break;
	}
	if (idx >= pid_clnt_cnt) {
		msg("No matching config instance is configured %s\n",
		     client_config->config.config_desc);
		return -1;
	}

	if (client_config->config.fields_mask == UNKNOWN_FIELD) {
		msg("Unexpected fields flag for config request %s\n",
		     client_config->config.config_desc);
		return -1;
	}

	if (client_config->config.fields_mask & DISABLE_FIELD) {
		if (client_config->config.disable > 1) {
			msg("Invalid disable request from client %d\n",
			     client_config->config.disable);
			return -1;
		}
	}

	if (client_config->config.fields_mask & SAMPLING_FIELD) {
		if (client_config->config.sampling < SAMPLING_MS_MINIMUM) {
			msg("Invalid sampling request from client %d\n",
			     client_config->config.sampling);
			return -1;
		}
	}

	if (client_config->config.fields_mask & SET_POINT_FIELD) {
		int set_point = client_config->config.d_setting.set_point;
		int set_point_clr = client_config->config.d_setting.set_point_clr;
		if (set_point == INT_MIN ||
		    (set_point_clr != INT_MIN && set_point <= set_point_clr) ||
		    (set_point_clr == INT_MIN && set_point <=
				pid_algo_clnt[idx].active_set_point_clr)) {
			msg("Invalid set_point request %d\n",
			     set_point);
			return -1;
		}
	}

	if (client_config->config.fields_mask & SET_POINT_CLR_FIELD) {
		int set_point = client_config->config.d_setting.set_point;
		int set_point_clr = client_config->config.d_setting.set_point_clr;
		if (set_point_clr == INT_MIN ||
		    (set_point != INT_MIN && set_point <= set_point_clr) ||
		    (set_point == INT_MIN && set_point_clr >=
				pid_algo_clnt[idx].active_set_point)) {
			msg("Invalid set_point_clr request %d\n",
			     set_point_clr);
			return -1;
		}
	}

	return (int)idx;
}

static void handle_config_set_sig(void)
{
	uint32_t i;
	int ret;
	uint32_t a_idx[cfg_set_cnt];

	/* Block next request until current request process */
	pthread_mutex_lock(&config_set_mutex);

	/* Check all client configs are valid */
	for (i = 0; i < cfg_set_cnt; i++) {
		ret = validate_client_config(&client_config[i]);
		if (ret < 0) {
			pthread_mutex_unlock(&config_set_mutex);
			return;
		}
		a_idx[i] = (uint32_t)ret;
	}

	for (i = 0; i < cfg_set_cnt; i++) {
		uint8_t disable_adjust_done = 0;
		uint8_t sampling_adjust_done = 0;
		uint8_t set_point_adjust_done = 0;
		uint8_t set_point_clr_adjust_done = 0;
		uint8_t curr_disabled = 0;
		uint32_t curr_sampling = 0;
		int curr_set_point = 0;
		int curr_set_point_clr = 0;
		struct pid_algo_t *pid_info = &pid_algo_clnt[a_idx[i]];

		/* Enable/disable client instance adjustment */
		curr_disabled = pid_info->disabled;
		if ((client_config[i].config.fields_mask & DISABLE_FIELD) &&
		    client_config[i].config.disable != curr_disabled) {
			if (handle_disable_config_adjust(pid_info, a_idx[i],
					client_config[i].config.disable)) {
				msg("%s: Config %s request failed for %s\n",
				     __func__, client_config[i].config.disable ? \
				    "disable":"enable", pid_info->desc);
				goto error_handler;
			} else {
				disable_adjust_done = 1;
			}
		}

		/* set sampling adjustment */
		curr_sampling =
		     sampling_groups[pid_info->sampling_group_id].sampling;
		if ((client_config[i].config.fields_mask & SAMPLING_FIELD) &&
		     client_config[i].config.sampling != curr_sampling) {
			if (handle_sampling_adjust(pid_info, a_idx[i],
					client_config[i].config.sampling)) {
				msg("%s: Sampling adjust failed for %d\n",
				     __func__, client_config[i].config.sampling);
				goto error_handler;
			} else {
				sampling_adjust_done = 1;
			}
		}

		/* set set_point adjustment */
		curr_set_point = pid_info->active_set_point;
		if (client_config[i].config.fields_mask & SET_POINT_FIELD &&
		    client_config[i].config.d_setting.set_point !=
							curr_set_point) {

			if (handle_set_point_adjust(pid_info,
				client_config[i].config.d_setting.set_point,
				INT_MIN)) {
				msg("%s: Set_point adjust failed for %d\n",
				     __func__,
				    client_config[i].config.d_setting.set_point);
				goto error_handler;
			} else {
				set_point_adjust_done = 1;
			}
		}

		/* set set_point_clr adjustment */
		curr_set_point_clr = pid_info->active_set_point_clr;
		if (client_config[i].config.fields_mask & SET_POINT_CLR_FIELD &&
		    client_config[i].config.d_setting.set_point_clr !=
							curr_set_point_clr) {

			if (handle_set_point_adjust(pid_info,
				INT_MIN,
				client_config[i].config.d_setting.set_point_clr)) {
				msg("%s: Set_point_clr adjust failed for %d\n",
				     __func__,
				    client_config[i].config.d_setting.set_point_clr);
				goto error_handler;
			} else {
				set_point_clr_adjust_done = 1;
			}
		}

algo_notify:
		if (disable_adjust_done ||
		    set_point_adjust_done ||
		    set_point_clr_adjust_done ||
		    sampling_adjust_done) {
			/* Re-adjust thresholds */
			pid_thresh_notify(pid_info->sensor_clnt, 0, 0,
			      (void *)(uintptr_t)a_idx[i]);

			/* Flush error history */
			set_curr_err(&pid_info->err_info, 0, I_SAMPLES);

			dbgmsg("%s: Adjusted config: %s disable %d, sampling %d, "
			        "active set point %d, active set point_clr %d\n"
			       , __func__, pid_info->desc,
			       pid_info->disabled,
			       sampling_groups[pid_info->sampling_group_id].sampling,
			       pid_info->active_set_point,
			       pid_info->active_set_point_clr);
		}
		continue;

error_handler:
		if (disable_adjust_done)
			handle_disable_config_adjust(pid_info, a_idx[i], curr_disabled);

		if (sampling_adjust_done)
			handle_sampling_adjust(pid_info, a_idx[i], curr_sampling);

		if (set_point_adjust_done)
			handle_set_point_adjust(pid_info, curr_set_point,
						INT_MIN);

		if (set_point_clr_adjust_done)
			handle_set_point_adjust(pid_info, INT_MIN,
					        curr_set_point_clr);

		goto algo_notify;
	}
	pthread_mutex_unlock(&config_set_mutex);
	return;
}

static int config_query_notify(int algo_type, void *data, void *reserved)
{
	uint32_t idx;
	struct thermal_msg_data query_data;

	if ((algo_type != PID_ALGO_TYPE) && (algo_type != ALGO_IDX_MAX))
		return 0;

	/* Request for num of configs if reserved is 1 */
	if (reserved != NULL) {
		int *num_cfg_req = (int *)reserved;
		if (*num_cfg_req == 1)
			return (int)pid_clnt_cnt;
	}

	for (idx = 0; idx < pid_clnt_cnt; idx++) {
		struct pid_algo_t *pid_info = &pid_algo_clnt[idx];
		memset(&query_data, 0, sizeof(struct thermal_msg_data));
		query_data.msg_type = PID_ALGO_TYPE;
		strlcpy(query_data.client_name, CONFIG_QUERY_CLIENT,
		        CLIENT_NAME_MAX);
		strlcpy(query_data.config.config_desc, pid_info->desc,
		        MAX_ALGO_DESC);
		query_data.config.disable = pid_info->disabled;
		query_data.config.sampling =
			sampling_groups[pid_info->sampling_group_id].sampling;
		query_data.config.d_setting.set_point =
			pid_info->active_set_point;
		query_data.config.d_setting.set_point_clr =
			pid_info->active_set_point_clr;
		strlcpy(query_data.config.d_setting.device,
		        pid_info->setting->device,
		        DEVICES_MAX_NAME_LEN);
		thermal_server_config_info_to_client(&query_data);
	}

	return 0;
}

static void handle_override_sig(void)
{
	uint8_t idx = 0;

	for (idx = 0; idx < pid_clnt_cnt; ++idx) {
		if (pid_algo_clnt[idx].disabled ||
		    (pid_algo_clnt[idx].setting->override == 0))
			continue;

		if (override_mode) {
			/* Set to active set point to overide */
			pid_algo_clnt[idx].active_set_point =
				pid_algo_clnt[idx].setting->set_point +
				pid_algo_clnt[idx].setting->override;
			pid_algo_clnt[idx].active_set_point_clr =
				pid_algo_clnt[idx].setting->set_point_clr +
				pid_algo_clnt[idx].setting->override;

		} else {
			/* Set to active set point to overide */
			pid_algo_clnt[idx].active_set_point =
				pid_algo_clnt[idx].setting->set_point;
			pid_algo_clnt[idx].active_set_point_clr =
				pid_algo_clnt[idx].setting->set_point_clr;
		}

		/* Reset Adjustment */
		pid_algo_clnt[idx].prev_therm_adjustment = 0;
		pid_algo_clnt[idx].prev_freq_adjustment = 0;

		/* Re-adjust thresholds */
		pid_thresh_notify(pid_algo_clnt[idx].sensor_clnt, 0, 0,
			      (void *)(uintptr_t)idx);

		/* Flush error history */
		set_curr_err(&pid_algo_clnt[idx].err_info, 0, I_SAMPLES);
		set_curr_err(&pid_algo_clnt[idx].freq_err, 0, I_SAMPLES);

		dbgmsg("%s: %s active set point %d, original set point %d "
		       "mode %d", __func__, pid_algo_clnt[idx].desc,
		       pid_algo_clnt[idx].active_set_point,
		       pid_algo_clnt[idx].setting->set_point, override_mode);
	}
}

static void handle_timer_sig(void)
{
	uint8_t idx = 0;
	uint8_t sample_id = 0;
	int temp;
	int ret_val;
	int error;
	struct timespec curr_time = {0, 0};
	uint32_t sample_cnt = 0;
	int adjustment = 0;
	union device_request req;

	/* Make local copy and clear only the ones that were actually
	   handled and use local copy to restart handled timers */
	uint32_t local_sampling_group_mask = sampling_group_mask;

	ret_val = clock_gettime(CLOCK_MONOTONIC, &curr_time);
	if (ret_val) {
		msg("%s: Error getting time.\n", __func__);
		return;
	}

	/* Get temp and handle */
	for (sample_id = 0; sample_id < sampling_group_count; sample_id++) {
		if ((local_sampling_group_mask & (0x1U << sample_id)) == 0)
			continue;

		sample_cnt = (uint32_t)(((curr_time.tv_sec -
			       sampling_groups[sample_id].timer_start.tv_sec) *
			       1000) + ((curr_time.tv_nsec -
			       sampling_groups[sample_id].timer_start.tv_nsec) /
			       1000000)) / sampling_groups[sample_id].sampling;

		if (sample_cnt > I_SAMPLES)
			sample_cnt = I_SAMPLES;

		for (idx = 0; idx < pid_clnt_cnt; idx++) {
			int applied_lvl;
			if (pid_algo_clnt[idx].disabled)
				continue;

			if ((sampling_groups[sample_id].pid_clnt_state &
			    (0x1U << idx)) == 0)
				continue;

			if (pid_algo_clnt[idx].state == 0)
				continue;

			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s",
				   SENSOR_READ_PRE_REQ, pid_algo_clnt[idx].desc,
				   SENSORS, pid_algo_clnt[idx].setting->sensor);
			temp = sensors_manager_read(pid_algo_clnt[idx].sensor_clnt);
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d mC",
				   SENSOR_READ_POST_REQ, pid_algo_clnt[idx].desc, SENSORS,
				   pid_algo_clnt[idx].setting->sensor, TEMPERATURE, temp);

			/* Converge to thresh clearing temperature when
			   mitigating */
			error = pid_algo_clnt[idx].active_set_point - temp;

			set_curr_err(&pid_algo_clnt[idx].err_info, error,
				     sample_cnt);

			dbgmsg("%s: PID Id %s Read %s %dmC, Err %dmC, "
			       "ErrSum %dmC, SampleCnt %d",
			       __func__, pid_algo_clnt[idx].desc,
			       pid_algo_clnt[idx].setting->sensor, temp, error,
			       pid_algo_clnt[idx].err_info.err_I_total,
			       sample_cnt);

			/* Convert error from mC to C */
			float E0 = ((float)get_err(&pid_algo_clnt[idx].err_info,
						   0))/1000;
			float E1 = ((float)get_err(&pid_algo_clnt[idx].err_info,
						   -1))/1000;
			float E2 = ((float)get_err(&pid_algo_clnt[idx].err_info,
						   -2))/1000;
			/* Convert from mSec to Sec */
			float Ts = ((float)(sampling_groups[sample_id].sampling)) / 1000;
			float Units = (float)pid_algo_clnt[idx].setting->units_per_C;
			float freq_scale = pid_algo_clnt[idx].setting->freq_scale;

			if ((E0 == 0) || ((E0 > 0) && (E1 < 0)) ||
				((E0 < 0) && (E1 > 0))) {
				dbgmsg("%s: PID Id %s Reset Prev Therm Adj.",
					__func__, pid_algo_clnt[idx].desc);
				pid_algo_clnt[idx].prev_therm_adjustment = 0;
			}

			error = pid_algo_clnt[idx].max_op_lvl - pid_algo_clnt[idx].applied_freq;

			set_curr_err(&pid_algo_clnt[idx].freq_err, error,
				     sample_cnt);

			dbgmsg("%s: PID Id %s Read %s %dkHz, Err %dkHz, "
			       "ErrSum %dkHz, SampleCnt %d",
			       __func__, pid_algo_clnt[idx].desc,
			       pid_algo_clnt[idx].setting->sensor,
			       pid_algo_clnt[idx].dev_op_lvl, error,
			       pid_algo_clnt[idx].freq_err.err_I_total,
			       sample_cnt);

			float F0 = ((float)get_err(&pid_algo_clnt[idx].freq_err,
						   0));
			float F1 = ((float)get_err(&pid_algo_clnt[idx].freq_err,
						   -1));
			float F2 = ((float)get_err(&pid_algo_clnt[idx].freq_err,
						   -2));
			float proportional;
			float integral;
			float differential;

			if (temp >= pid_algo_clnt[idx].active_set_point) {
					dbgmsg("%s: PID Id %s, Prev Adj. %dkHz, E0 %.1fC, "
					       "E1 %.1fC, E2 %.1fC",__func__,
					pid_algo_clnt[idx].desc,
					pid_algo_clnt[idx].prev_therm_adjustment, E0, E1, E2);
					if ((E0 == 0) && (E1 == 0) && (E2 == 0)) {
						dbgmsg("%s: PID Id %s Error = 0 SKIP Adj.",
						       __func__, pid_algo_clnt[idx].desc);
						continue;
					}
					proportional = E0 - E1;
					integral = E0 * Ts;
					differential = (E0 - (2 * E1) + E2) / Ts;
					adjustment = pid_algo_clnt[idx].prev_therm_adjustment +
						     (int)(((pid_algo_clnt[idx].setting->p_const *
						     proportional) +
						     (pid_algo_clnt[idx].setting->i_const *
						     integral) +
						     (pid_algo_clnt[idx].setting->d_const *
						     differential)) * Units);
					pid_algo_clnt[idx].prev_therm_adjustment = adjustment;
					pid_algo_clnt[idx].prev_freq_adjustment = 0;
			} else if (temp < pid_algo_clnt[idx].active_set_point) {
					if (F0 == 0) {
						dbgmsg("%s: PID Id %s Error = 0 SKIP Freq Adj.",
							__func__, pid_algo_clnt[idx].desc);
						continue;
					}
					dbgmsg("%s: PID Id %s, Prev Adj. %dkHz, F0 %.1fkHz, "
						"F1 %.1fkHz, F2 %.1fkHz",__func__,
						pid_algo_clnt[idx].desc,
						pid_algo_clnt[idx].prev_freq_adjustment, F0, F1, F2);
					proportional = (F0 - F1);
					integral = (F0 * Ts);
					differential = ((F0 - (2 * F1) + F2) / Ts);
					adjustment = pid_algo_clnt[idx].prev_freq_adjustment +
							(int)(((pid_algo_clnt[idx].setting->p_const *
							proportional) +
							(pid_algo_clnt[idx].setting->i_const *
							 integral) +
							(pid_algo_clnt[idx].setting->d_const *
							 differential)) / freq_scale);
					pid_algo_clnt[idx].prev_freq_adjustment = adjustment;
					pid_algo_clnt[idx].prev_therm_adjustment = 0;
			}
			pid_algo_clnt[idx].dev_op_lvl += adjustment;

			/* Cap adjustment to min frequency */
			if (pid_algo_clnt[idx].dev_op_lvl <
			    pid_algo_clnt[idx].min_op_lvl) {
				dbgmsg("%s: PID Id %s, Clip Calc freq %dkHz",
				       __func__, pid_algo_clnt[idx].desc,
				       pid_algo_clnt[idx].dev_op_lvl);
				pid_algo_clnt[idx].dev_op_lvl  =
					pid_algo_clnt[idx].min_op_lvl;
				/* Reset Adjustment */
				pid_algo_clnt[idx].prev_therm_adjustment = 0;
			}

			/* Cap adjustment to max frequency */
			if (pid_algo_clnt[idx].dev_op_lvl >
			    pid_algo_clnt[idx].max_op_lvl) {
				dbgmsg("%s: PID Id %s, Clip Calc freq %dkHz",
				       __func__, pid_algo_clnt[idx].desc,
				       pid_algo_clnt[idx].dev_op_lvl);
				pid_algo_clnt[idx].dev_op_lvl  =
					pid_algo_clnt[idx].max_op_lvl;
				/* Reset Adjustment */
				pid_algo_clnt[idx].prev_therm_adjustment = 0;
			}

			dbgmsg("%s: PID Id %s, Sensor %s, Device %s, Adj. %dkHz, Calc freq %dkHz",
			       __func__, pid_algo_clnt[idx].desc,
			       pid_algo_clnt[idx].setting->sensor,
			       pid_algo_clnt[idx].setting->device, adjustment,
			       pid_algo_clnt[idx].dev_op_lvl);

			applied_lvl = (pid_algo_clnt[idx].dev_op_lvl > 0)?
					(pid_algo_clnt[idx].dev_op_lvl):(0);
			if (temp < pid_algo_clnt[idx].active_set_point) {
				uint32_t i;
				for (i = 1; i < pid_algo_clnt[idx].num_of_lvls; i++) {
					if (pid_algo_clnt[idx].dev_op_lvl > pid_algo_clnt[idx].lvl_arr[i].lvl.value) {
						applied_lvl = pid_algo_clnt[idx].lvl_arr[i-1].lvl.value;
						break;
					}
				}
			}
			req.value = applied_lvl;
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d",
				   CPU_FREQ_PRE_REQ, pid_algo_clnt[idx].desc, DEVICE,
				   pid_algo_clnt[idx].setting->device, FREQUENCY, req.value);
			applied_lvl =
				device_clnt_request(pid_algo_clnt[idx].dev_clnt,
						    &req);
			if (applied_lvl > 0) {
				dbgmsg("%s: PID Id %s, Sensor %s, Device %s, Applied freq %dkHz",
				       __func__, pid_algo_clnt[idx].desc,
				       pid_algo_clnt[idx].setting->sensor,
				       pid_algo_clnt[idx].setting->device,
				       applied_lvl);
				thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d",
					   CPU_FREQ_POST_REQ, pid_algo_clnt[idx].desc, DEVICE,
					   pid_algo_clnt[idx].setting->device, FREQUENCY, applied_lvl);
				pid_algo_clnt[idx].applied_freq = applied_lvl;
			} else {
				msg("%s: PID Id %s, Error setting dev op value\n.",
				    __func__, pid_algo_clnt[idx].desc);
			}
		}
		pthread_mutex_lock(&wait_mutex);
		/* Mark off handled item */
		sampling_group_mask &= (~(0x1U << sample_id));
		pthread_mutex_unlock(&wait_mutex);
	}

	/* Restart handled timers */
	for (sample_id = 0; sample_id < sampling_group_count; sample_id++) {
		if ((local_sampling_group_mask & (0x1U << sample_id)) == 0)
			continue;

		ret_val = pid_settimer(&sampling_groups[sample_id]);
		if (ret_val)
			msg("%s: Error cannot settime.\n", __func__);
	}
}

static void handle_thresh_sig(void)
{
	uint8_t idx = 0;
	int     temp;
	uint8_t transition = 0;
	struct sensor_thresh_req thresh;
	int ret_val;

	/* Get temp and handle */
	for (idx = 0; idx < pid_clnt_cnt; idx++) {
		uint8_t s_id = 0;
		if ((thresh_reached_mask & (0x1U << idx)) == 0)
			continue;

		if (pid_algo_clnt[idx].disabled) {
			pthread_mutex_lock(&wait_mutex);
			/* Mark off handled item for disabled pid instance */
			thresh_reached_mask &= (~(0x1U << idx));
			pthread_mutex_unlock(&wait_mutex);
			continue;
		}

		s_id = pid_algo_clnt[idx].sampling_group_id;

		temp = sensors_manager_read(pid_algo_clnt[idx].sensor_clnt);
		dbgmsg("%s: PID Id %s, Read %s %dmC", __func__,
		       pid_algo_clnt[idx].desc,
		       pid_algo_clnt[idx].setting->sensor, temp);
		transition = 0;
		if ((pid_algo_clnt[idx].state == 0) &&
		    (temp >= pid_algo_clnt[idx].active_set_point_clr)) {
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d mC",
				   SENSOR_THRESHOLD_HIT, pid_algo_clnt[idx].desc, SENSORS,
				   pid_algo_clnt[idx].setting->sensor, TEMPERATURE, temp);
			transition = 1;
		} else if ((pid_algo_clnt[idx].state == 1) &&
			   (temp <= pid_algo_clnt[idx].active_set_point_clr - ONE_CELSIUS)) {
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d mC",
				   SENSOR_THRESHOLD_CLR, pid_algo_clnt[idx].desc, SENSORS,
				   pid_algo_clnt[idx].setting->sensor, TEMPERATURE, temp);
			transition = 1;
		}

		if (transition) {
			pid_algo_clnt[idx].state = (pid_algo_clnt[idx].state)?(0):(1);
			dbgmsg("%s: PID Id %s Transition State %d", __func__,
			       pid_algo_clnt[idx].desc,
			       pid_algo_clnt[idx].state);

			/* Set initial Dev OP value point */
			if (pid_algo_clnt[idx].state)
				pid_algo_clnt[idx].dev_op_lvl =
					pid_algo_clnt[idx].max_op_lvl;
		}

		memset(&thresh, 0x0, sizeof(thresh));
		thresh.notify_cb_func = pid_thresh_notify;
		thresh.notify_cb_data = (void*)(uintptr_t)idx;
		if (pid_algo_clnt[idx].state == 0) {
			/* Allow CPU to run at max frequency */
			device_clnt_cancel_request(pid_algo_clnt[idx].dev_clnt);
			pid_algo_clnt[idx].prev_therm_adjustment = 0;
			pid_algo_clnt[idx].prev_freq_adjustment = 0;
			thresh.thresh.high_valid = 1;
			thresh.thresh.high = pid_algo_clnt[idx].active_set_point_clr;
			sampling_groups[s_id].pid_clnt_state &= (~(0x1U << idx));
			/* Flush error history */
			set_curr_err(&pid_algo_clnt[idx].err_info, 0, I_SAMPLES);
			set_curr_err(&pid_algo_clnt[idx].freq_err, 0, I_SAMPLES);
		} else {
			thresh.thresh.low_valid = 1;
			thresh.thresh.low = pid_algo_clnt[idx].active_set_point_clr - ONE_CELSIUS;
			sampling_groups[s_id].pid_clnt_state |= (0x1U << idx);
		}

		pthread_mutex_lock(&wait_mutex);
		/* Mark off handled item */
		thresh_reached_mask &= (~(0x1U << idx));
		pthread_mutex_unlock(&wait_mutex);

		/* Set Thresh */
		sensors_manager_set_thresh_lvl(pid_algo_clnt[idx].sensor_clnt, &thresh);
	}

	/* Adjust the sampling interval period. */
	for(idx = 0; idx < sampling_group_count; idx++) {
		if (sampling_groups[idx].pid_clnt_state) {
			if (sampling_groups[idx].enabled)
				continue;

			TIMER_SAMPLING_SET(sampling_groups[idx],
					   sampling_groups[idx].sampling);
			ret_val = pid_settimer(&sampling_groups[idx]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[idx].enabled = 1;
		} else {
			if (sampling_groups[idx].enabled == 0)
				continue;

			/* Disarm the timer */
			TIMER_SAMPLING_SET(sampling_groups[idx], 0);
			ret_val = pid_settimer(&sampling_groups[idx]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[idx].enabled = 0;
		}
	}
}

static void *pid_algo_monitor(void *data)
{
	uint32_t        local_ev_mask = 0;
	int ret_val = 0;
	uint8_t i = 0;

	/* Set mask to all CPU's Trigger setting all thresholds */
	if (pid_clnt_cnt < MAX_PID_INSTANCES_SUPPORTED)
		thresh_reached_mask = (1U << pid_clnt_cnt) - 1;
	else
		thresh_reached_mask = UINT32_MAX;

	/* Create timer id for each sampling group */
	for(i = 0; i < sampling_group_count; i++) {
		memset(&sampling_groups[i].sig_evt, 0, sizeof(struct sigevent));
		sampling_groups[i].sig_evt.sigev_notify = SIGEV_THREAD;
		sampling_groups[i].sig_evt.sigev_notify_function = pid_timer_expired;
		sampling_groups[i].sig_evt.sigev_value.sival_ptr = &sampling_groups[i].timer_id;
		sampling_groups[i].sig_evt.sigev_value.sival_int = i;

		ret_val = timer_create(CLOCK_MONOTONIC, &sampling_groups[i].sig_evt,
				       &sampling_groups[i].timer_id);
		if (ret_val) {
			msg("%s: Error cannot create timer.\n", __func__);
			return NULL;
		}
		memset(&sampling_groups[i].timer_val, 0,
		       sizeof(struct itimerspec));
		TIMER_SAMPLING_SET(sampling_groups[i],
				   sampling_groups[i].sampling);
	}
	/* Set inital active thresholds */
	handle_override_sig();
	/* Set initial thresholds */
	handle_thresh_sig();

	thermal_server_register_client_req_handler("override", override_notify, NULL);
	thermal_server_register_client_req_handler(CONFIG_QUERY_CLIENT, config_query_notify, NULL);
	thermal_server_register_client_req_handler(CONFIG_SET_CLIENT, config_parameter_set_notify, NULL);

	while (1) {
		dbgmsg("%s: Wait for EV", __func__);
		pthread_mutex_lock(&wait_mutex);
		if (!pid_ev_mask) {
			pthread_cond_wait(&wait_cond, &wait_mutex);
		}
		/* Make local copy and clear. */
		local_ev_mask = pid_ev_mask;
		pid_ev_mask = 0;
		pthread_mutex_unlock(&wait_mutex);

		if (local_ev_mask & PID_OVERRIDE_EV) {
			dbgmsg("%s: Override EVT", __func__);
			handle_override_sig();
		}

		if (local_ev_mask & PID_TIMER_EV) {
			dbgmsg("%s: Timer EVT", __func__);
			handle_timer_sig();
		}

		if (local_ev_mask & PID_THRESH_EV) {
			dbgmsg("%s: Thresh EVT", __func__);
			handle_thresh_sig();
		}

		if (local_ev_mask & PID_CONFIG_SET_EV) {
			dbgmsg("%s: Config set EVT", __func__);
			handle_config_set_sig();
		}

	}
	return NULL;
}

int pid_algo_init(struct thermal_setting_t *setting)
{
	int ret_val = 0;
	uint32_t idx;
	struct device_info dev_info;
	struct setting_info *pid_cfg;
	int err = 0;
	uint8_t i = 0;

	if (setting == NULL) {
		msg("%s: Invalid Argument\n", __func__);
		return -(EINVAL);
	}

	pid_cfg = setting->list;
	/* Import settings */
	while (pid_cfg) {
		err = 0;
		if (pid_clnt_cnt >= MAX_PID_INSTANCES_SUPPORTED) {
			msg("%s: Max PID instances reached.", __func__);
			break;
		}

		if (pid_cfg->algo_type != PID_ALGO_TYPE) {
			pid_cfg = pid_cfg->next;
			continue;
		}

		if (pid_cfg->err_disable) {
			info("%s: Entry Disabled %s.", __func__,
			    pid_cfg->desc);
			pid_cfg = pid_cfg->next;
			continue;
		}

		if (pid_cfg->data.pid.sampling_period_ms == 0) {
			msg("%s: Sampling is not configured, "
			    "disabling entry %s", __func__,  pid_cfg->desc);
			continue;
		}

		pid_algo_clnt[pid_clnt_cnt].sensor_clnt =
			sensors_manager_reg_clnt(pid_cfg->data.pid.sensor);
		if (pid_algo_clnt[pid_clnt_cnt].sensor_clnt == NULL) {
			msg("%s: Sensor clnt create fail %s\n", __func__,
			    pid_cfg->data.pid.sensor);
			err = 1;
			goto error_handler;
		}

		pid_algo_clnt[pid_clnt_cnt].dev_clnt =
			devices_manager_reg_clnt(pid_cfg->data.pid.device);
		if (pid_algo_clnt[pid_clnt_cnt].dev_clnt == NULL) {
			msg("%s: Device clnt create fail %s\n", __func__,
			    pid_cfg->data.pid.device);
			err = 1;
			goto error_handler;
		}

		err = devices_manager_get_info(pid_cfg->data.pid.device,
					       &dev_info);
		if (err) {
			msg("%s: Can not grab dev info %s\n", __func__,
			    pid_cfg->data.pid.device);
			goto error_handler;
		}

		pid_algo_clnt[pid_clnt_cnt].lvl_arr = (struct device_lvl_info *)
			malloc(sizeof(struct device_lvl_info) *
			       dev_info.num_of_levels);

		err = devices_manager_get_lvl_list(pid_cfg->data.pid.device,
						   pid_algo_clnt[pid_clnt_cnt].lvl_arr,
						   &dev_info.num_of_levels);
		if (err) {
			msg("%s: Can not grab dev lvl list %s\n", __func__,
			    pid_cfg->data.pid.device);
			goto error_handler;
		}
error_handler:
		if (err) {
			/* Clean up and look for next PI cfg */
			if (pid_algo_clnt[pid_clnt_cnt].sensor_clnt)
				sensors_manager_dereg_clnt(
				   pid_algo_clnt[pid_clnt_cnt].sensor_clnt);
			if (pid_algo_clnt[pid_clnt_cnt].dev_clnt)
				devices_manager_dereg_clnt(
				   pid_algo_clnt[pid_clnt_cnt].dev_clnt);
			if (pid_algo_clnt[pid_clnt_cnt].lvl_arr)
				free(pid_algo_clnt[pid_clnt_cnt].lvl_arr);
			memset(&pid_algo_clnt[pid_clnt_cnt], 0x0,
			       sizeof(struct pid_algo_t));
		} else {
			pid_algo_clnt[pid_clnt_cnt].disabled =
				pid_cfg->disable;
			pid_algo_clnt[pid_clnt_cnt].min_op_lvl =
				dev_info.min_dev_op_value;
			pid_algo_clnt[pid_clnt_cnt].max_op_lvl =
				dev_info.max_dev_op_value;
			pid_algo_clnt[pid_clnt_cnt].dev_op_lvl =
				dev_info.max_dev_op_value;
			pid_algo_clnt[pid_clnt_cnt].applied_freq =
				dev_info.max_dev_op_value;
			pid_algo_clnt[pid_clnt_cnt].setting =
				&(pid_cfg->data.pid);
			pid_algo_clnt[pid_clnt_cnt].desc = pid_cfg->desc;
			pid_algo_clnt[pid_clnt_cnt].num_of_lvls =
				dev_info.num_of_levels;
			pid_cfg->data.pid.sampling_period_ms =
			      MIN(PID_SAMPLE_PERIOD,
				  pid_cfg->data.pid.sampling_period_ms);
			pid_algo_clnt[pid_clnt_cnt].active_set_point =
				pid_cfg->data.pid.set_point;
			pid_algo_clnt[pid_clnt_cnt].active_set_point_clr =
				pid_cfg->data.pid.set_point_clr;
			dbgmsg("%s: Desc %s\n", __func__, pid_cfg->desc);
			dbgmsg("%s: Sensor %s, K_P %.1f, K_I %.1f, K_D %.1f\n",
			       __func__, pid_cfg->data.pid.sensor,
			       pid_cfg->data.pid.p_const,
			       pid_cfg->data.pid.i_const,
			       pid_cfg->data.pid.d_const);
			dbgmsg("%s: Device %s, Err weight %.1f, Int samples %d, Units Per C %dkHz\n",
			       __func__, pid_cfg->data.pid.device,
			       pid_cfg->data.pid.err_weight,
			       pid_cfg->data.pid.i_samples,
			       pid_cfg->data.pid.units_per_C);
			dbgmsg("%s: Set Point %dmC, Set Point Clr %dmC, Override %dmC\n",
			       __func__, pid_cfg->data.pid.set_point,
			       pid_cfg->data.pid.set_point_clr,
			       pid_cfg->data.pid.override);
			dbgmsg("%s: MAXFREQ %dkHz", __func__,
			       dev_info.max_dev_op_value);
			if (!pid_cfg->disable)
				print_setting(pid_cfg);

			/* Create sampling groups for similarly configured
			   intervals and save sampling group id of each client
			   on its own algo info */
			for (i = 0; i < MAX_PID_INSTANCES_SUPPORTED &&
			     sampling_groups[i].sampling != 0; i++) {
				if (pid_cfg->data.pid.sampling_period_ms ==
				    sampling_groups[i].sampling) {
					pid_algo_clnt[pid_clnt_cnt].sampling_group_id = i;
					break;
				}
			}
			if (i < MAX_PID_INSTANCES_SUPPORTED &&
			    sampling_groups[i].sampling == 0) {
				sampling_groups[i].sampling =
					pid_cfg->data.pid.sampling_period_ms;
				pid_algo_clnt[pid_clnt_cnt].sampling_group_id = i;
				sampling_group_count++;
			}
			pid_clnt_cnt++;
		}
		pid_cfg = pid_cfg->next;
	}

	if (pid_clnt_cnt == 0) {
		info("%s: No PID's to be configured.\n", __func__);
		return -(EFAULT);
	}

	for (idx = 0; idx < pid_clnt_cnt; ++idx) {
		/* Allocate sample data array */
		pid_algo_clnt[idx].err_info.err =
			(int*)malloc(sizeof(int) * I_SAMPLES);
		pid_algo_clnt[idx].freq_err.err =
			(int*)malloc(sizeof(int) * I_SAMPLES);
		if (pid_algo_clnt[idx].err_info.err == NULL ||
		    pid_algo_clnt[idx].freq_err.err == NULL) {
			pid_algo_clnt[idx].disabled = 1;
			msg("%s: Failed to alloc sample error data\n",
			    __func__);
			ret_val = -(ENOMEM);
			break;
		}
		memset(pid_algo_clnt[idx].err_info.err, 0x0,
		       sizeof(int) * I_SAMPLES);
		memset(pid_algo_clnt[idx].freq_err.err, 0x0,
		       sizeof(int) * I_SAMPLES);
	}

	if (ret_val != 0) {
		return ret_val;
	}

	ret_val = pthread_create(&pid_thread, NULL, (void *)&pid_algo_monitor,
				 NULL);
	if (ret_val != 0) {
		msg("Error initializing pid algo monitor\n");
	}
	return ret_val;
}

