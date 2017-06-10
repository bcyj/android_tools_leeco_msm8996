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
#include "ss_algorithm.h"
#include "server/thermal_lib_common.h"
#include "thermal_server.h"

/* Limit MAX SAMPLE PERIOD to 1000000ms */
#define SAMPLE_PERIOD (1000000U)
#define ERR_HISTORY (5)

#define TIMER_EV  0x1
#define THRESH_EV 0x2
#define OVERRIDE_EV 0x4
#define CONFIG_SET_EV 0x8
#define MAX_INSTANCES_SUPPORTED 32

enum ss_algo_state {
	SS_STATE_UNKNOWN = 0,
	SS_STATE_STOP_ALGO,
	SS_STATE_START_SAMPLING,
	SS_STATE_STOP_SAMPLING
};

#define TIMER_SAMPLING_SET(s_grp, sampling) { \
	(s_grp.timer_val.it_value.tv_sec = (sampling / 1000)); \
	(s_grp.timer_val.it_value.tv_nsec = ((sampling % 1000) * 1000000)); \
	}

static pthread_t        thread;
static pthread_cond_t   wait_cond = PTHREAD_COND_INITIALIZER;
static uint32_t         ev_mask;
static uint32_t         thresh_reached_mask;
static uint32_t         sampling_group_mask;
static uint8_t          override_mode;
static pthread_mutex_t  wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  config_set_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint8_t sampling_group_count;

struct err_history_t {
	int err[ERR_HISTORY];
	int err_I_total;
	uint32_t curr_idx;
};

struct ss_algo_t {
	char                   *desc;
	struct ss_setting      *setting;
	sensor_clnt_handle      sensor_clnt;
	device_clnt_handle      dev_clnt;
	int                     min_op_lvl;
	int                     max_op_lvl;
	int                     dev_op_lvl;
	uint32_t                num_of_lvls;
	int                     curr_lvl;
	int                     dev_mtgn_max_limit;
	enum ss_algo_state      state;
	struct err_history_t    err_info; /* Error info */
	uint8_t                 disabled;
	struct device_lvl_info *lvl_arr;
	int                     tc_delay;
	int                     active_set_point;
	int                     active_set_point_clr;
	uint8_t                 sampling_group_id;
};

struct sampling_group {
	uint32_t sampling;
	uint32_t ss_clnt_state;
	uint8_t enabled;  /* 0 Timer Disabled, 1 Timer Enabled */
	struct sigevent sig_evt;
	timer_t timer_id;
	struct timespec   timer_start;
	struct itimerspec timer_val;
};

static void thresh_notify(sensor_clnt_handle clnt,
                          enum sensor_notify_event_type event,
                          int reading, void *data);
static struct sampling_group sampling_groups[MAX_INSTANCES_SUPPORTED];
static struct ss_algo_t algo_clnt[MAX_INSTANCES_SUPPORTED];
static uint32_t clnt_cnt;
static struct thermal_msg_data client_config[MAX_INSTANCES_SUPPORTED];
static uint32_t cfg_set_cnt;

static void handle_thresh_sig(void);

static int get_err(struct err_history_t *err_info, int idx)
{
	idx = idx % ERR_HISTORY;

	idx += (int)err_info->curr_idx;

	if (idx < 0)
		idx += ERR_HISTORY;
	else if (idx >= ERR_HISTORY)
		idx -= ERR_HISTORY;

	return err_info->err[idx];
}

static void set_curr_err(struct err_history_t *err_info, int err,
			 uint32_t sample_cnt)
{
	uint32_t idx;

	if (sample_cnt > ERR_HISTORY)
		sample_cnt = ERR_HISTORY;

	/* Subtract out error values to be replaced */
	for (idx = 0; idx < sample_cnt; idx++)
		err_info->err_I_total -= get_err(err_info, (int)idx+1);

	/* Add in the newest entries */
	for (idx = 0; idx < sample_cnt; idx++) {
		++(err_info->curr_idx);
		err_info->curr_idx %= ERR_HISTORY;
		err_info->err[err_info->curr_idx] = err;
		err_info->err_I_total += err;
	}
}

static int override_notify(int mode, void *data, void *reserved)
{
	dbgmsg("%s: SS mode %d", __func__, mode);

	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	if (override_mode != mode) {
		override_mode = (uint8_t)mode;
		ev_mask |= OVERRIDE_EV;
		pthread_cond_broadcast(&wait_cond);
	}
	pthread_mutex_unlock(&wait_mutex);

	return 0;
}

static int settimer(struct sampling_group *sampling_group)
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

static void timer_expired(union sigval sval)
{
	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	ev_mask |= TIMER_EV;
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

	/* Notify the waiting thread */
	cfg_set_cnt = 0;
	for (idx = 0; idx < num_cfg; idx++) {
		if (current[idx].msg_type != SS_ALGO_TYPE)
			continue;

		if (cfg_set_cnt >= MAX_INSTANCES_SUPPORTED)
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

	pthread_mutex_lock(&wait_mutex);
	if (cfg_set_cnt > 0) {
		ev_mask |= CONFIG_SET_EV;
		pthread_cond_broadcast(&wait_cond);

	}
	pthread_mutex_unlock(&wait_mutex);

	pthread_mutex_unlock(&config_set_mutex);

	return 0;
}

static int handle_sampling_adjust(struct ss_algo_t *ss_info, uint32_t algo_idx, uint32_t sampling)
{
	uint8_t s_idx = 0;
	int ret_val = 0;

	if (ss_info == NULL ||
	    algo_idx >= clnt_cnt ||
	    sampling < SAMPLING_MS_MINIMUM)
		return -1;

	for (s_idx = 0; s_idx < sampling_group_count; s_idx++) {
		if (sampling == sampling_groups[s_idx].sampling)
			break;
	}
	if(s_idx >= sampling_group_count) {

		if (sampling_group_count == MAX_INSTANCES_SUPPORTED)
			return -1;

		s_idx = sampling_group_count;
		sampling_group_count++;
		sampling_groups[s_idx].sampling = sampling;
		memset(&sampling_groups[s_idx].sig_evt, 0, sizeof(struct sigevent));
		sampling_groups[s_idx].sig_evt.sigev_notify = SIGEV_THREAD;
		sampling_groups[s_idx].sig_evt.sigev_notify_function =
						timer_expired;
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

	if (!ss_info->disabled &&
	    (sampling_groups[ss_info->sampling_group_id].ss_clnt_state &
							 (0x1U << algo_idx))) {
		sampling_groups[s_idx].ss_clnt_state |= (0x1U << algo_idx);
		if (sampling_groups[s_idx].ss_clnt_state &&
					!sampling_groups[s_idx].enabled) {
			TIMER_SAMPLING_SET(sampling_groups[s_idx],
					   sampling_groups[s_idx].sampling);
			ret_val = settimer(&sampling_groups[s_idx]);
			if (ret_val) {
				msg("%s: Error cannot settime.\n", __func__);
				sampling_groups[s_idx].ss_clnt_state &=
							(~(0x1U << algo_idx));
				return -1;
			} else {
				sampling_groups[s_idx].enabled = 1;
			}
		}
		sampling_groups[ss_info->sampling_group_id].ss_clnt_state &=
							(~(0x1U << algo_idx));
		if (!sampling_groups[ss_info->sampling_group_id].ss_clnt_state &&
		    sampling_groups[ss_info->sampling_group_id].enabled) {
			/* Disarm the timer */
			TIMER_SAMPLING_SET(sampling_groups[ss_info->sampling_group_id], 0);
			ret_val = settimer(&sampling_groups[ss_info->sampling_group_id]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[ss_info->sampling_group_id].enabled = 0;
		}
	}

	ss_info->sampling_group_id = s_idx;

	return 0;
}

static int handle_set_point_adjust(struct ss_algo_t *ss_info, int set_point, int set_point_clr)
{
	if (ss_info == NULL)
		return -1;

	/* Set requested active set_point and set_point_clr */
	if (set_point != INT_MIN)
		ss_info->active_set_point = set_point;
	if (set_point_clr != INT_MIN)
		ss_info->active_set_point_clr = set_point_clr;

	return 0;
}

static int validate_disabled_config(struct ss_algo_t *ss_info)
{
	if (ss_info == NULL ||
	    ss_info->sensor_clnt == NULL ||
	    ss_info->dev_clnt == NULL ||
	    ss_info->setting == NULL ||
	    ss_info->lvl_arr == NULL ||
	    (ss_info->active_set_point <= ss_info->active_set_point_clr) ||
	    sampling_groups[ss_info->sampling_group_id].sampling <
			SAMPLING_MS_MINIMUM ||
	    ss_info->num_of_lvls < 1)
		return -1;

	return 0;
}

static int handle_disable_config_adjust(struct ss_algo_t *ss_info, uint32_t idx, uint8_t disable)
{
	int ret_val = 0;

	if (ss_info == NULL)
		return -1;

	if(!disable) {
		if (validate_disabled_config(ss_info))
			return -1;

		ss_info->disabled = disable;
	} else {
		if (ss_info->sensor_clnt)
			sensors_manager_set_thresh_lvl(ss_info->sensor_clnt, NULL);

		if (ss_info->dev_clnt)
			device_clnt_cancel_request(ss_info->dev_clnt);

		ss_info->dev_op_lvl = ss_info->max_op_lvl;
		ss_info->state = SS_STATE_STOP_ALGO;
		ss_info->tc_delay = 0;

		/* disarm timer if no other client */
		sampling_groups[ss_info->sampling_group_id].ss_clnt_state &=
								(~(0x1U << idx));
		if (!sampling_groups[ss_info->sampling_group_id].ss_clnt_state &&
		    sampling_groups[ss_info->sampling_group_id].enabled) {
			/* Disarm the timer */
			TIMER_SAMPLING_SET(sampling_groups[ss_info->sampling_group_id], 0);
			ret_val = settimer(&sampling_groups[ss_info->sampling_group_id]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[ss_info->sampling_group_id].enabled = 0;
		}
		ss_info->disabled = disable;
	}
	return 0;
}

/* return idx of algo  if success, negtaive on error */
static int validate_client_config(struct thermal_msg_data *client_config)
{
	uint32_t idx;

	if ((client_config->msg_type != SS_ALGO_TYPE))
		return -1;

	for (idx = 0; idx < clnt_cnt; idx++) {
		if (0 == strcmp(client_config->config.config_desc,
			         algo_clnt[idx].desc))
			break;
	}
	if (idx >= clnt_cnt) {
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
				algo_clnt[idx].active_set_point_clr)) {
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
				algo_clnt[idx].active_set_point)) {
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
		struct ss_algo_t *ss_info = &algo_clnt[a_idx[i]];

		/* Enable/disable client instance adjustment */
		curr_disabled = ss_info->disabled;
		if ((client_config[i].config.fields_mask & DISABLE_FIELD) &&
		    client_config[i].config.disable != curr_disabled) {
			if (handle_disable_config_adjust(ss_info, a_idx[i],
					client_config[i].config.disable)) {
				msg("%s: Config %s request failed for %s\n",
				     __func__, client_config[i].config.disable ? \
				    "disable":"enable", ss_info->desc);
				goto error_handler;
			} else {
				disable_adjust_done = 1;
			}
		}

		/* set sampling adjustment */
		curr_sampling =
		     sampling_groups[ss_info->sampling_group_id].sampling;
		if ((client_config[i].config.fields_mask & SAMPLING_FIELD) &&
		     client_config[i].config.sampling != curr_sampling) {
			if (handle_sampling_adjust(ss_info, a_idx[i],
					client_config[i].config.sampling)) {
				msg("%s: Sampling adjust failed for %d\n",
				     __func__, client_config[i].config.sampling);
				goto error_handler;
			} else {
				sampling_adjust_done = 1;
			}
		}

		/* set set_point adjustment */
		curr_set_point = ss_info->active_set_point;
		if (client_config[i].config.fields_mask & SET_POINT_FIELD &&
		    client_config[i].config.d_setting.set_point !=
							curr_set_point) {

			if (handle_set_point_adjust(ss_info,
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
		curr_set_point_clr = ss_info->active_set_point_clr;
		if (client_config[i].config.fields_mask & SET_POINT_CLR_FIELD &&
		    client_config[i].config.d_setting.set_point_clr !=
							curr_set_point_clr) {

			if (handle_set_point_adjust(ss_info,
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
			thresh_notify(ss_info->sensor_clnt, 0, 0,
			      (void *)(uintptr_t)a_idx[i]);

			/* Flush error history */
			set_curr_err(&ss_info->err_info, 0, ERR_HISTORY);

			dbgmsg("%s: Adjusted config: %s disable %d, sampling %d, "
			        "active set point %d, active set point_clr %d\n"
			       , __func__, ss_info->desc,
			       ss_info->disabled,
			       sampling_groups[ss_info->sampling_group_id].sampling,
			       ss_info->active_set_point,
			       ss_info->active_set_point_clr);
		}
		continue;
error_handler:
		if (disable_adjust_done)
			handle_disable_config_adjust(ss_info, a_idx[i],
						     curr_disabled);

		if (sampling_adjust_done)
			handle_sampling_adjust(ss_info, a_idx[i],
					       curr_sampling);

		if (set_point_adjust_done)
			handle_set_point_adjust(ss_info, curr_set_point,
						INT_MIN);
		if (set_point_clr_adjust_done)
			handle_set_point_adjust(ss_info, INT_MIN,
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

	if ((algo_type != SS_ALGO_TYPE) && (algo_type != ALGO_IDX_MAX))
		return 0;

	/* Request for num of configs if reserved is 1 */
	if (reserved != NULL) {
		int *num_cfg_req = (int *)reserved;
		if (*num_cfg_req == 1)
			return (int)clnt_cnt;
	}
	for (idx = 0; idx < clnt_cnt; idx++) {
		struct ss_algo_t *ss_info = &algo_clnt[idx];
		memset(&query_data, 0, sizeof(struct thermal_msg_data));
		query_data.msg_type = SS_ALGO_TYPE;
		strlcpy(query_data.client_name, CONFIG_QUERY_CLIENT, CLIENT_NAME_MAX);
		strlcpy(query_data.config.config_desc, ss_info->desc,
		        MAX_ALGO_DESC);
		query_data.config.disable = ss_info->disabled;
		query_data.config.sampling =
			sampling_groups[ss_info->sampling_group_id].sampling;
		query_data.config.d_setting.set_point =
			ss_info->active_set_point;
		query_data.config.d_setting.set_point_clr =
			ss_info->active_set_point_clr;
		strlcpy(query_data.config.d_setting.device,
		        ss_info->setting->device, DEVICES_MAX_NAME_LEN);
		thermal_server_config_info_to_client(&query_data);
	}

	return 0;
}

static void thresh_notify(sensor_clnt_handle clnt,
			  enum sensor_notify_event_type event,
			  int reading, void *data)
{
	if (NULL == clnt) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	dbgmsg("%s: SS Id %s, Update recieved %s %d", __func__,
	       algo_clnt[(uintptr_t)data].desc,
	       algo_clnt[(uintptr_t)data].setting->sensor,
	       reading);

	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	thresh_reached_mask |= (0x1U << (uintptr_t)data);
	ev_mask |= (THRESH_EV | TIMER_EV);
	sampling_group_mask |= algo_clnt[(uintptr_t)data].sampling_group_id;
	pthread_cond_broadcast(&wait_cond);
	pthread_mutex_unlock(&wait_mutex);
}

static void handle_override_sig(void)
{
	uint8_t idx = 0;

	for (idx = 0; idx < clnt_cnt; ++idx) {
		if (algo_clnt[idx].disabled ||
		    (algo_clnt[idx].setting->override == 0))
			continue;

		if (override_mode) {
			/* Set to active set point to overide */
			algo_clnt[idx].active_set_point =
				algo_clnt[idx].setting->set_point +
				algo_clnt[idx].setting->override;
			algo_clnt[idx].active_set_point_clr =
				algo_clnt[idx].setting->set_point_clr +
				algo_clnt[idx].setting->override;

		} else {
			/* Set to active set point to overide */
			algo_clnt[idx].active_set_point =
				algo_clnt[idx].setting->set_point;
			algo_clnt[idx].active_set_point_clr =
				algo_clnt[idx].setting->set_point_clr;
		}

		/* Re-adjust thresholds */
		thresh_notify(algo_clnt[idx].sensor_clnt, 0, 0,
			      (void *)(uintptr_t)idx);

		/* Flush error history */
		set_curr_err(&algo_clnt[idx].err_info, 0, ERR_HISTORY);

		dbgmsg("%s: %s active set point %d, original set point %d "
		       "mode %d", __func__, algo_clnt[idx].desc,
		       algo_clnt[idx].active_set_point,
		       algo_clnt[idx].setting->set_point, override_mode);
	}
}

static void increase_mitigation_lvl(struct ss_algo_t *instance)
{
	int applied_lvl;
	union device_request req;

	if (instance->curr_lvl >= (int)(instance->num_of_lvls - 1)) {
		/* Nothing to be done at max level. */
		return;
	}

	if (instance->dev_mtgn_max_limit &&
	    instance->lvl_arr[instance->curr_lvl + 1].lvl.value <
			instance->dev_mtgn_max_limit) {
		dbgmsg("Already at device mitigation max limit\n");
		return;
	}

	instance->curr_lvl++;

	instance->dev_op_lvl = instance->lvl_arr[instance->curr_lvl].lvl.value;

	req.value = instance->dev_op_lvl;
	thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d",
		   CPU_FREQ_PRE_REQ, instance->desc, DEVICE,
		   instance->setting->device, FREQUENCY, req.value);
	applied_lvl = device_clnt_request(instance->dev_clnt, &req);
	if (applied_lvl > 0) {
		dbgmsg("%s: SS Id %s, Device %s, Req. Freq %dKhz, Applied freq %dkHz, TC delay %d",
		       __func__, instance->desc, instance->setting->device,
		       req.value, applied_lvl, instance->tc_delay);
		thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d",
			   CPU_FREQ_POST_REQ, instance->desc, DEVICE,
			   instance->setting->device, FREQUENCY, applied_lvl);
	} else {
		msg("%s: SS Id %s, Error setting dev op value %d\n.",
		    __func__, instance->desc, req.value);
	}
}

static void decrease_mitigation_lvl(struct ss_algo_t *instance)
{
	int applied_lvl;
	union device_request req;

	if (instance->curr_lvl <= 0) {
		/* Nothing to be done at min lvl */
		return;
	}

	instance->curr_lvl--;

	instance->dev_op_lvl = instance->lvl_arr[instance->curr_lvl].lvl.value;

	req.value = instance->dev_op_lvl;
	thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d",
		   CPU_FREQ_PRE_REQ, instance->desc, DEVICE,
		   instance->setting->device, FREQUENCY, req.value);
	applied_lvl = device_clnt_request(instance->dev_clnt, &req);
	if (applied_lvl > 0) {
		dbgmsg("%s: SS Id %s, Device %s, Req. Freq %dKhz, Applied freq %dkHz, TC delay %d",
		       __func__, instance->desc, instance->setting->device,
		       req.value, applied_lvl, instance->tc_delay);
		thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d",
			   CPU_FREQ_POST_REQ, instance->desc, DEVICE,
			   instance->setting->device, FREQUENCY, applied_lvl);
	} else {
		msg("%s: SS Id %s, Error setting dev op value %d\n.",
		    __func__, instance->desc, req.value);
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

		if (sample_cnt > ERR_HISTORY)
			sample_cnt = ERR_HISTORY;

		for (idx = 0; idx < clnt_cnt; idx++) {
			if (algo_clnt[idx].disabled)
				continue;

			if ((sampling_groups[sample_id].ss_clnt_state &
			    (0x1U << idx)) == 0)
				continue;

			if ((algo_clnt[idx].state == SS_STATE_STOP_ALGO) ||
			    (algo_clnt[idx].state == SS_STATE_STOP_SAMPLING))
				continue;
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s",
				   SENSOR_READ_PRE_REQ, algo_clnt[idx].desc, SENSORS,
				   algo_clnt[idx].setting->sensor);

			temp = sensors_manager_read(algo_clnt[idx].sensor_clnt);
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d mC",
				   SENSOR_READ_POST_REQ, algo_clnt[idx].desc, SENSORS,
				   algo_clnt[idx].setting->sensor, TEMPERATURE, temp);

			/* Converge to set_point temperature when mitigating */
			error = algo_clnt[idx].active_set_point - temp;

			set_curr_err(&algo_clnt[idx].err_info, error,
				     sample_cnt);

			dbgmsg("%s: SS Id %s Read %s %dmC, Err %dmC, "
			       "SampleCnt %d", __func__, algo_clnt[idx].desc,
			       algo_clnt[idx].setting->sensor, temp, error,
			       sample_cnt);

			int E0 = get_err(&algo_clnt[idx].err_info, 0);
			int E1 = get_err(&algo_clnt[idx].err_info, -1);

			dbgmsg("%s: SS Id %s, E0 %dmC, E1 %dmC", __func__,
			       algo_clnt[idx].desc, E0, E1);

			if ((algo_clnt[idx].setting->time_constant > 1) &&
			    (E0 == E1)) {
				algo_clnt[idx].tc_delay++;

				if (algo_clnt[idx].tc_delay <
				    algo_clnt[idx].setting->time_constant)
					continue;
			}

			if (E0 < 0) {
				/* Negative error means Hot */
				increase_mitigation_lvl(&algo_clnt[idx]);
			} else {
				decrease_mitigation_lvl(&algo_clnt[idx]);
				/* Stop sampling if the freq is at max */
				if (algo_clnt[idx].curr_lvl <= 0) {
					pthread_mutex_lock(&wait_mutex);
					thresh_reached_mask |= (0x1U << idx);
					pthread_mutex_unlock(&wait_mutex);
					algo_clnt[idx].state = SS_STATE_STOP_SAMPLING;
					handle_thresh_sig();
				}
			}

			algo_clnt[idx].tc_delay = 0;
		}
		pthread_mutex_lock(&wait_mutex);
		/* Mark off handled item */
		sampling_group_mask &= (~(0x1U << sample_id));
		pthread_mutex_unlock(&wait_mutex);
	}

	/* Restart handled timers */
	for (sample_id = 0; sample_id < sampling_group_count; sample_id++) {
		if (((local_sampling_group_mask & (0x1U << sample_id)) == 0) ||
		    (sampling_groups[sample_id].ss_clnt_state == 0))
			continue;

		ret_val = settimer(&sampling_groups[sample_id]);
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
	for (idx = 0; idx < clnt_cnt; idx++) {
		uint8_t s_id = 0;
		if ((thresh_reached_mask & (0x1U << idx)) == 0)
			continue;

		if (algo_clnt[idx].disabled) {
			pthread_mutex_lock(&wait_mutex);
			/* Mark off handled item for disabled ss instance */
			thresh_reached_mask &= (~(0x1U << idx));
			pthread_mutex_unlock(&wait_mutex);
			continue;
		}

		s_id = algo_clnt[idx].sampling_group_id;

		temp = sensors_manager_read(algo_clnt[idx].sensor_clnt);
		dbgmsg("%s: SS Id %s, Read %s %dmC", __func__,
		       algo_clnt[idx].desc,
		       algo_clnt[idx].setting->sensor, temp);
		transition = 0;
		if (((algo_clnt[idx].state == SS_STATE_STOP_ALGO) ||
		     (algo_clnt[idx].state == SS_STATE_STOP_SAMPLING)) &&
		     (temp >= algo_clnt[idx].active_set_point)) {
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d mC",
				   SENSOR_THRESHOLD_HIT, algo_clnt[idx].desc, SENSORS,
				   algo_clnt[idx].setting->sensor, TEMPERATURE, temp);
			transition = 1;
			algo_clnt[idx].state = SS_STATE_START_SAMPLING;
		} else if (((algo_clnt[idx].state == SS_STATE_START_SAMPLING) ||
			    (algo_clnt[idx].state == SS_STATE_STOP_SAMPLING)) &&
			    (temp <= algo_clnt[idx].active_set_point_clr)) {
			thermalmsg(LOG_LVL_INFO, LOG_TRACE, "%s: %s: %s=%s %s=%d mC",
				   SENSOR_THRESHOLD_CLR, algo_clnt[idx].desc, SENSORS,
				   algo_clnt[idx].setting->sensor, TEMPERATURE, temp);
			algo_clnt[idx].state = SS_STATE_STOP_ALGO;
			transition = 1;
		} else if ((algo_clnt[idx].state == SS_STATE_STOP_SAMPLING) &&
			   (temp < algo_clnt[idx].active_set_point)) {
			transition = 1;
		}

		if (transition) {
			dbgmsg("%s: SS Id %s Transition State %d", __func__,
			       algo_clnt[idx].desc,
			       algo_clnt[idx].state);

			/* Set initial Dev OP value point */
			if (algo_clnt[idx].state == SS_STATE_START_SAMPLING)
				algo_clnt[idx].dev_op_lvl =
					algo_clnt[idx].max_op_lvl;
		}

		memset(&thresh, 0x0, sizeof(thresh));
		thresh.notify_cb_func = thresh_notify;
		thresh.notify_cb_data = (void *)(uintptr_t)idx;
		if ((algo_clnt[idx].state == SS_STATE_STOP_ALGO) ||
		    (algo_clnt[idx].state == SS_STATE_STOP_SAMPLING)) {
			/* Allow CPU to run at max frequency */
			device_clnt_cancel_request(algo_clnt[idx].dev_clnt);
			algo_clnt[idx].tc_delay = 0;
			thresh.thresh.high_valid = 1;
			thresh.thresh.high = algo_clnt[idx].active_set_point;
			sampling_groups[s_id].ss_clnt_state &= (~(0x1U << idx));
			/* Flush error history */
			set_curr_err(&algo_clnt[idx].err_info, 0, ERR_HISTORY);
		} else {
			thresh.thresh.low_valid = 1;
			thresh.thresh.low = algo_clnt[idx].active_set_point_clr;
			sampling_groups[s_id].ss_clnt_state |= (0x1U << idx);
		}

		pthread_mutex_lock(&wait_mutex);
		/* Mark off handled item */
		thresh_reached_mask &= (~(0x1U << idx));
		pthread_mutex_unlock(&wait_mutex);

		/* Set Thresh */
		sensors_manager_set_thresh_lvl(algo_clnt[idx].sensor_clnt, &thresh);
	}

	/* Adjust the sampling interval period. */
	for(idx = 0; idx < sampling_group_count; idx++) {
		if (sampling_groups[idx].ss_clnt_state) {
			if (sampling_groups[idx].enabled)
				continue;

			TIMER_SAMPLING_SET(sampling_groups[idx],
					   sampling_groups[idx].sampling);
			ret_val = settimer(&sampling_groups[idx]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[idx].enabled = 1;
		} else {
			if (sampling_groups[idx].enabled == 0)
				continue;

			/* Disarm the timer */
			TIMER_SAMPLING_SET(sampling_groups[idx], 0);
			ret_val = settimer(&sampling_groups[idx]);
			if (ret_val)
				msg("%s: Error cannot settime.\n", __func__);
			else
				sampling_groups[idx].enabled = 0;
		}
	}
}

static void *algo_monitor(void *data)
{
	uint32_t        local_ev_mask = 0;
	int ret_val = 0;
	uint8_t i = 0;

	/* Set mask to all trigger setting all thresholds */
	if (clnt_cnt < MAX_INSTANCES_SUPPORTED)
		thresh_reached_mask = (1U << clnt_cnt) - 1;
	else
		thresh_reached_mask = UINT32_MAX;

	/* Create timer id for each sampling group */
	for(i = 0; i < sampling_group_count; i++) {
		memset(&sampling_groups[i].sig_evt, 0, sizeof(struct sigevent));
		sampling_groups[i].sig_evt.sigev_notify = SIGEV_THREAD;
		sampling_groups[i].sig_evt.sigev_notify_function = timer_expired;
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
		if (!ev_mask)
			pthread_cond_wait(&wait_cond, &wait_mutex);

		/* Make local copy and clear. */
		local_ev_mask = ev_mask;
		ev_mask = 0;
		pthread_mutex_unlock(&wait_mutex);

		if (local_ev_mask & OVERRIDE_EV) {
			dbgmsg("%s: Override EVT", __func__);
			handle_override_sig();
		}

		if (local_ev_mask & THRESH_EV) {
			dbgmsg("%s: Thresh EVT", __func__);
			handle_thresh_sig();
		}

		if (local_ev_mask & TIMER_EV) {
			dbgmsg("%s: Timer EVT", __func__);
			handle_timer_sig();
		}

		if (local_ev_mask & CONFIG_SET_EV) {
			dbgmsg("%s: Config set EVT", __func__);
			handle_config_set_sig();
		}
	}
	return NULL;
}

int ss_algo_init(struct thermal_setting_t *setting)
{
	int ret_val = 0;
	struct device_info dev_info;
	struct setting_info *cfg;
	int err = 0;
	uint8_t i = 0;

	if (setting == NULL) {
		msg("%s: Invalid Argument\n", __func__);
		return -(EINVAL);
	}

	cfg = setting->list;
	/* Import settings */
	while (cfg) {
		err = 0;
		if (clnt_cnt >= MAX_INSTANCES_SUPPORTED) {
			msg("%s: Max SS instances reached.", __func__);
			break;
		}

		if (cfg->algo_type != SS_ALGO_TYPE) {
			cfg = cfg->next;
			continue;
		}

		if (cfg->err_disable) {
			info("%s: Entry Disabled %s.", __func__,
			    cfg->desc);
			cfg = cfg->next;
			continue;
		}

		if (cfg->data.ss.sampling_period_ms == 0) {
			msg("%s: Sampling is not configured, "
			    "disabling entry %s", __func__,  cfg->desc);
			continue;
		}

		algo_clnt[clnt_cnt].sensor_clnt =
			sensors_manager_reg_clnt(cfg->data.ss.sensor);
		if (algo_clnt[clnt_cnt].sensor_clnt == NULL) {
			msg("%s: Sensor clnt create fail %s\n", __func__,
			    cfg->data.ss.sensor);
			err = EFAULT;
			goto error_handler;
		}

		algo_clnt[clnt_cnt].dev_clnt =
			devices_manager_reg_clnt(cfg->data.ss.device);
		if (algo_clnt[clnt_cnt].dev_clnt == NULL) {
			msg("%s: Device clnt create fail %s\n", __func__,
			    cfg->data.ss.device);
			err = EFAULT;
			goto error_handler;
		}

		err = devices_manager_get_info(cfg->data.ss.device,
					       &dev_info);
		if (err) {
			msg("%s: Can not grab dev info %s\n", __func__,
			    cfg->data.ss.device);
			goto error_handler;
		}

		if (dev_info.dev_type != DEVICE_OP_VALUE_TYPE) {
			msg("%s: Improper dev_type %d\n", __func__,
			    cfg->algo_type);
			err = EFAULT;
			goto error_handler;
		}

		if (dev_info.num_of_levels == 0) {
			msg("%s: No levels supported for this device.\n",
			    __func__);
			err = EFAULT;
			goto error_handler;
		}

		algo_clnt[clnt_cnt].lvl_arr = (struct device_lvl_info *)
			malloc(sizeof(struct device_lvl_info) *
			       dev_info.num_of_levels);

		err = devices_manager_get_lvl_list(cfg->data.ss.device,
						   algo_clnt[clnt_cnt].lvl_arr,
						   &dev_info.num_of_levels);
		if (err) {
			msg("%s: Can not grab dev lvl list %s\n", __func__,
			    cfg->data.ss.device);
			goto error_handler;
		}
error_handler:
		if (err) {
			/* Clean up and look for next SS cfg */
			if (algo_clnt[clnt_cnt].sensor_clnt)
				sensors_manager_dereg_clnt(
				   algo_clnt[clnt_cnt].sensor_clnt);
			if (algo_clnt[clnt_cnt].dev_clnt)
				devices_manager_dereg_clnt(
				   algo_clnt[clnt_cnt].dev_clnt);
			if (algo_clnt[clnt_cnt].lvl_arr)
				free(algo_clnt[clnt_cnt].lvl_arr);
			memset(&algo_clnt[clnt_cnt], 0x0,
			       sizeof(struct ss_algo_t));
		} else {
			algo_clnt[clnt_cnt].disabled =
				cfg->disable;
			algo_clnt[clnt_cnt].state =
				SS_STATE_STOP_ALGO;
			algo_clnt[clnt_cnt].min_op_lvl =
				dev_info.min_dev_op_value;
			algo_clnt[clnt_cnt].max_op_lvl =
				dev_info.max_dev_op_value;
			algo_clnt[clnt_cnt].dev_op_lvl =
				dev_info.max_dev_op_value;
			algo_clnt[clnt_cnt].setting = &(cfg->data.ss);
			algo_clnt[clnt_cnt].desc = cfg->desc;
			algo_clnt[clnt_cnt].num_of_lvls =
				dev_info.num_of_levels;
			algo_clnt[clnt_cnt].active_set_point =
				cfg->data.ss.set_point;
			algo_clnt[clnt_cnt].active_set_point_clr =
				cfg->data.ss.set_point_clr;
			if (cfg->data.ss.device_mtgn_max_limit == 1)
				algo_clnt[clnt_cnt].dev_mtgn_max_limit = 0;
			else
				algo_clnt[clnt_cnt].dev_mtgn_max_limit =
					cfg->data.ss.device_mtgn_max_limit;
			cfg->data.ss.sampling_period_ms =
				MIN(SAMPLE_PERIOD,
				    cfg->data.ss.sampling_period_ms);
			dbgmsg("%s: SS ID %s, Sensor %s\n", __func__, cfg->desc,
			       cfg->data.ss.sensor);
			dbgmsg("%s: Device %s, Number of Levels %d\n",
			       __func__, cfg->data.ss.device,
			       algo_clnt[clnt_cnt].num_of_lvls);
			dbgmsg("%s: Set Point %dmC, Set Point Clr %dmC, "
			       "Override %dmC\n", __func__,
			       cfg->data.ss.set_point,
			       cfg->data.ss.set_point_clr,
			       cfg->data.ss.override);
			dbgmsg("%s: MAXFREQ %dkHz, MINFREQ %dkHz, TC %d", __func__,
			       dev_info.max_dev_op_value,
			       dev_info.min_dev_op_value,
			       cfg->data.ss.time_constant);
			if (!cfg->disable)
				print_setting(cfg);

			/* Create sampling groups for similarly configured
			   intervals and save sampling group id of each client
			   on its own algo info */
			for (i = 0; i < MAX_INSTANCES_SUPPORTED &&
			     sampling_groups[i].sampling != 0; i++) {
				if (cfg->data.ss.sampling_period_ms ==
				    sampling_groups[i].sampling) {
					algo_clnt[clnt_cnt].sampling_group_id = i;
					break;
				}
			}

			if (i < MAX_INSTANCES_SUPPORTED &&
			    sampling_groups[i].sampling == 0) {
				sampling_groups[i].sampling =
					cfg->data.ss.sampling_period_ms;
				algo_clnt[clnt_cnt].sampling_group_id = i;
				sampling_group_count++;
			}
			clnt_cnt++;
		}
		cfg = cfg->next;
	}

	if (clnt_cnt == 0) {
		info("%s: No SS's configured.\n", __func__);
		return -(EFAULT);
	}

	ret_val = pthread_create(&thread, NULL, (void *)&algo_monitor,
				 NULL);
	if (ret_val != 0)
		msg("Error initializing SS algo monitor\n");
	return ret_val;
}

