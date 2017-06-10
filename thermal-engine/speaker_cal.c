/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "thermal.h"
#include "sensors_manager.h"
#include "speaker_cal.h"
#include "thermal_server.h"
#include <poll.h>
#include <signal.h>

#define MILLISEC (1000)
#define MICROSEC (1000 * 1000)
#define NANOSEC  (1000 * 1000 * 1000)

enum alarm_type {
	ALARM_NONE = 0,
	ALARM_MSM_THERMAL_NODE,
	ALARM_POSIX_TIMER,
	ALARM_POSIX_TIMER_SIGNAL
};

static pthread_t        thread;
static uint8_t          cal_mode;
static pthread_mutex_t  cal_mode_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_t        alarm_thread;
static pthread_cond_t   wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static int              alarm_value;
static enum alarm_type  msm_thermal_alarm;
static uint8_t          speaker_cal_cancel;
static pthread_mutex_t  spkr_notify_mtx = PTHREAD_MUTEX_INITIALIZER;
static timer_t          timerid;

struct speaker_sensor_info {
	sensor_clnt_handle  sensor_clnt;
	int            curr_reading;
	int            max_reading;
	int            min_reading;
};

struct speaker_cal_t {
	char                        *desc;
	struct equilibrium_setting  *setting;
	struct speaker_sensor_info  *sensors_arr;
	uint8_t                      sensors_cnt;
	struct speaker_sensor_info   sensor;
	uint8_t                      enabled;
	uint8_t                      state;
};

#define ALARM_NODE "/sys/module/msm_thermal/thermal_timer/wakeup_ms"

static struct speaker_cal_t speaker_cal_data;
static int server_handle;

static void *alarm_handler(void *data)
{
	int err = 0;
	struct pollfd fds;
	int fd;
	char buf[UINT_BUF_MAX] = {0};

	fd = open(ALARM_NODE, O_RDONLY);
	if (fd < 0) {
		msg("%s: Unable to open Alarm node.", __func__);
		return NULL;
	}

	while (1) {
		fds.fd = fd;
		fds.events = POLLERR|POLLPRI;
		fds.revents = 0;
		err = poll(&fds, 1, -1);
		if (err == -1) {
			msg("Error in Alarm node poll.\n");
			break;
		}

		read(fd, buf, sizeof(buf));
		lseek(fd, 0, SEEK_SET);

		/* notify the waiting threads */
		pthread_mutex_lock(&wait_mutex);
		alarm_value = atoi(buf);
		dbgmsg("%s: %d", __func__, alarm_value);
		pthread_cond_broadcast(&wait_cond);
		pthread_mutex_unlock(&wait_mutex);
	}
	close(fd);
	return NULL;
}

static void alarm_sig_handler(int sig, siginfo_t *sa, void *data)
{
	/* notify the waiting threads */
	pthread_mutex_lock(&wait_mutex);
	alarm_value = sig;
	dbgmsg("%s: %d", __func__, alarm_value);
	pthread_cond_broadcast(&wait_cond);
	pthread_mutex_unlock(&wait_mutex);
}

static int set_and_wait_alarm(uint32_t time_ms)
{
	char buf[UINT_BUF_MAX] = {0};
	int ret_val;
	struct itimerspec wakeup;
	struct sigevent sigev;
	struct sigaction sa;

	if (speaker_cal_cancel) {
		info("%s: Cancel wait", __func__);
		return 0;
	}

	dbgmsg("%s: Set Alarm", __func__);
	if (msm_thermal_alarm == ALARM_MSM_THERMAL_NODE) {
		snprintf(buf, UINT_BUF_MAX, "%d", time_ms);
		if (write_to_file(ALARM_NODE, buf, strlen(buf)) != (int) strlen(buf)) {
			msg("%s: Write failure", __func__);
			return -1;
		}
	} else {
		if (msm_thermal_alarm == ALARM_POSIX_TIMER) {
			sa.sa_flags = SA_SIGINFO;
			sa.sa_sigaction = alarm_sig_handler;
			sigemptyset(&sa.sa_mask);
			if (sigaction(SIGALRM, &sa, NULL) == -1) {
				msg("%s: error creating signal handler", __func__);
				return -(EFAULT);
			}
			memset(&sigev, 0, sizeof(struct sigevent));
			sigev.sigev_notify = SIGEV_SIGNAL;
			sigev.sigev_signo = SIGALRM;

			/* create timer */
			if (timer_create(CLOCK_REALTIME_ALARM, &sigev,
					&timerid) != 0) {
				msg("%s: error: failed to create timer", __func__);
				return -1;
			}
			msm_thermal_alarm = ALARM_POSIX_TIMER_SIGNAL;
		}

		/* get current time */
		if(clock_gettime(CLOCK_REALTIME_ALARM, &wakeup.it_value)) {
			msg("%s: error: failed to get time ", __func__);
			return -1;
		}
		/* start timer */
		wakeup.it_value.tv_sec += time_ms/MILLISEC;
		wakeup.it_value.tv_nsec += (time_ms % MILLISEC) * MICROSEC;
		wakeup.it_interval.tv_sec = 0;
		wakeup.it_interval.tv_nsec = 0;
		if (wakeup.it_value.tv_nsec >= NANOSEC) {
			wakeup.it_value.tv_sec++;
			wakeup.it_value.tv_nsec -= NANOSEC;
		}
		if(timer_settime(timerid, TIMER_ABSTIME, &wakeup, NULL)) {
			msg("%s: error: failed to set wakeup time timerid %ld", __func__, (long)timerid);
			return 0;
		}
	}
	pthread_mutex_lock(&wait_mutex);
	if (!alarm_value)
		pthread_cond_wait(&wait_cond, &wait_mutex);
	/* Make local copy and clear. */
	ret_val = alarm_value;
	alarm_value = 0;
	pthread_mutex_unlock(&wait_mutex);

	dbgmsg("%s: Alarm Recieved %d", __func__, ret_val);
	return ret_val;
}

static void cancel_alarm(void)
{
	char buf[UINT_BUF_MAX] = {0};

	dbgmsg("%s: Cancel Alarm", __func__);
	if (msm_thermal_alarm == ALARM_MSM_THERMAL_NODE)
	{
		snprintf(buf, UINT_BUF_MAX, "%d", 0);
		if (write_to_file(ALARM_NODE, buf, strlen(buf)) != (int) strlen(buf))
			msg("%s: Write failure", __func__);
	} else {
		if (msm_thermal_alarm == ALARM_POSIX_TIMER_SIGNAL) {
			if (timer_delete(timerid) == -1)
				msg("%s: Failed to delete timer", __func__);
			msm_thermal_alarm = ALARM_POSIX_TIMER;
		}
	}
}

static void *speaker_cal_algo(void *data)
{
	int ret_val = 0;
	struct equilibrium_setting *setting = speaker_cal_data.setting;
	uint8_t idx;
	int min = 0, max;
	uint32_t delta;
	uint32_t sleep_period;
	uint32_t long_sample_cnt = 0;

	alarm_value = 0;

	if (setting->pre_cal_delay_ms)
		set_and_wait_alarm(setting->pre_cal_delay_ms);

	while (!speaker_cal_cancel) {
		/* Set a default extreme min and max temp. */
		min = 300000; /* 300C */
		max = -273000; /* -273C */

		speaker_cal_data.sensor.curr_reading =
			sensors_manager_read(speaker_cal_data.sensor.\
					     sensor_clnt);
		dbgmsg("%s: reading %s %d mC", __func__,
		       speaker_cal_data.setting->sensor,
		       speaker_cal_data.sensor.curr_reading);

		for (idx = 0; idx < speaker_cal_data.sensors_cnt; idx++) {
			speaker_cal_data.sensors_arr[idx].curr_reading =
				sensors_manager_read(speaker_cal_data.\
						     sensors_arr[idx].\
						     sensor_clnt);
			dbgmsg("%s: reading[%d] %d mC", __func__, idx,
			       speaker_cal_data.sensors_arr[idx].curr_reading);
			min = MIN(min, speaker_cal_data.sensors_arr[idx].\
				  curr_reading);
			max = MAX(max, speaker_cal_data.sensors_arr[idx].\
				  curr_reading);
		}
		dbgmsg("%s: min %d mC, max %d mC", __func__, min, max);

		/* Check if single sensor reading is within sensor delta range
		   of sensors array */
		delta = (uint32_t)abs(speaker_cal_data.sensor.curr_reading -
				      max);
		if (delta > setting->sensor_delta) {
			dbgmsg("%s: %d > %d (single sensor delta out of range)",
			       __func__, delta, setting->sensor_delta);
			speaker_cal_data.state = 0;
			goto sleep_handler;
		}

		/* Check if all sensors in array are withing temp_range of each
		   other and less than max_temp*/
		if ((max > setting->max_temp) ||
		    ((max - min) > (int)setting->temp_range)) {
			msg("%s: %d > %d (max_temp) || %d > %d(temp_range)",
			    __func__, max, setting->max_temp, (max - min),
			    setting->temp_range);
			speaker_cal_data.state = 0;
			goto sleep_handler;
		}

		if (long_sample_cnt != 0) {
			/* Record min and max for long sample history */
			speaker_cal_data.sensor.min_reading =
				MIN(speaker_cal_data.sensor.min_reading,
				    speaker_cal_data.sensor.curr_reading);
			speaker_cal_data.sensor.max_reading =
				MAX(speaker_cal_data.sensor.max_reading,
				    speaker_cal_data.sensor.curr_reading);

			for (idx = 0; idx < speaker_cal_data.sensors_cnt;
			     idx++) {
				speaker_cal_data.sensors_arr[idx].min_reading =
					MIN(speaker_cal_data.sensors_arr[idx].\
					    min_reading,
					    speaker_cal_data.sensors_arr[idx].\
					    curr_reading);
				speaker_cal_data.sensors_arr[idx].max_reading =
					MAX(speaker_cal_data.sensors_arr[idx].\
					    max_reading,
					    speaker_cal_data.sensors_arr[idx].\
					    curr_reading);
			}
		} else {
			/* Init min and max long sample history */
			speaker_cal_data.sensor.max_reading =
				speaker_cal_data.sensor.curr_reading;
			speaker_cal_data.sensor.min_reading =
				speaker_cal_data.sensor.curr_reading;
			for (idx = 0; idx < speaker_cal_data.sensors_cnt;
			     idx++) {
				speaker_cal_data.sensors_arr[idx].max_reading =
					speaker_cal_data.sensors_arr[idx].\
						curr_reading;
				speaker_cal_data.sensors_arr[idx].min_reading =
					speaker_cal_data.sensors_arr[idx].\
						curr_reading;
			}
		}

		delta = (uint32_t)(speaker_cal_data.sensor.max_reading -
			speaker_cal_data.sensor.min_reading);
		if (delta > setting->long_sample_range) {
			dbgmsg("%s: %d > %d (long sample range)",
			       __func__, delta, setting->long_sample_range);
			speaker_cal_data.state = 0;
			goto sleep_handler;
		}

		for (idx = 0; idx < speaker_cal_data.sensors_cnt;
		     idx++) {
			delta = (uint32_t)(speaker_cal_data.sensors_arr[idx].max_reading -
				speaker_cal_data.sensors_arr[idx].min_reading);
			if (delta > setting->long_sample_range) {
				dbgmsg("%s:[%d] %d > %d (long sample range)",
				       __func__, idx, delta,
				       setting->long_sample_range);
				speaker_cal_data.state = 0;
				goto sleep_handler;
			}
		}

		long_sample_cnt++;

		/* Check if equilibrium achieved.*/
		if (long_sample_cnt >= setting->long_sampling_cnt) {
			break;
		}

		speaker_cal_data.state = 1;

		dbgmsg("%s: long sample cnt %d", __func__, long_sample_cnt);

sleep_handler:
		/* Wait for next polling interval */
		if (speaker_cal_data.state == 0) {
			sleep_period = setting->sampling_period_ms;
			long_sample_cnt = 0;
		} else {
			sleep_period =
				setting->long_sampling_period_ms;
		}

		set_and_wait_alarm(sleep_period);
	}

	if (!speaker_cal_cancel) {
		min += speaker_cal_data.setting->offset;
		info("%s: Speaker Cal temp %dC", __func__, RCONV(min));
		ret_val = thermal_server_notify_clients("spkr", RCONV(min));
		if (ret_val < 0)
			msg("%s: Error notifying spkr clients", __func__);
		else
			dbgmsg("%s: Success notifying spkr clients", __func__);
	}

	if (msm_thermal_alarm == ALARM_POSIX_TIMER_SIGNAL) {
		if (timer_delete(timerid) == -1)
			msg("%s: Failed to delete timer", __func__);
		msm_thermal_alarm = ALARM_POSIX_TIMER;
	}

	pthread_mutex_lock(&cal_mode_mtx);
	cal_mode = 0;
	pthread_mutex_unlock(&cal_mode_mtx);
	return NULL;
}

static int speaker_cal_notify(int mode, void *data, void *reserved)
{
	int ret_val = 0;

	/* Just in case CB is called from multiple threads lock it up. */
	pthread_mutex_lock(&spkr_notify_mtx);
	dbgmsg("%s: request %d", __func__, mode);
	if (mode) {
		pthread_mutex_lock(&cal_mode_mtx);
		if ((cal_mode == 0) && (speaker_cal_data.enabled)) {
			/* Spawn a calibration thread */
			cal_mode = 1;
			speaker_cal_cancel = 0;
			ret_val = pthread_create(&thread, NULL,
						 (void *)&speaker_cal_algo, NULL);
			if (ret_val)
				msg("%s: error creating thread %d", __func__, ret_val);
		} else
			dbgmsg("%s: Cal already running", __func__);
		pthread_mutex_unlock(&cal_mode_mtx);
	} else {
		if (cal_mode) {
			speaker_cal_cancel = 1;
			cancel_alarm();
			/* notify the waiting threads */
			pthread_mutex_lock(&wait_mutex);
			alarm_value = 0;
			pthread_cond_broadcast(&wait_cond);
			pthread_mutex_unlock(&wait_mutex);
			pthread_join(thread, NULL);
			cal_mode = 0;
		}
	}
	pthread_mutex_unlock(&spkr_notify_mtx);

	return 0;
}

int speaker_cal_init(struct thermal_setting_t *setting)
{
	int idx;
	struct setting_info *cfg = NULL;
	int err = 0;

	if (setting == NULL) {
		msg("%s: Invalid Argument\n", __func__);
		return -(EINVAL);
	}

	/* Check to find out if msm thermal alarm is available */
	if (!check_node(ALARM_NODE))
		msm_thermal_alarm = ALARM_MSM_THERMAL_NODE;
	else
		msm_thermal_alarm = ALARM_POSIX_TIMER;

	cfg = setting->list;
	/* Import speaker cal settings */
	while (cfg) {
		err = 0;

		if (cfg->algo_type != EQUILIBRIUM_TYPE) {
			cfg = cfg->next;
			continue;
		}

		if (cfg->err_disable || cfg->disable) {
			info("%s: Entry Disabled %s.", __func__, cfg->desc);
			cfg = cfg->next;
			continue;
		}

		if (strncmp(cfg->desc, "SPEAKER-CAL", MAX_ALGO_DESC)) {
			msg("%s: unsupported algo instance %s.", __func__,
			    cfg->desc);
			cfg = cfg->next;
			continue;
		}

		/* Found what we're looking for */
		break;
	}

	if (cfg == NULL) {
		msg("No speaker_cal config");
		return -(ENOENT);
	}

	if ((cfg->data.eq.list_cnt == 0) ||
	    (cfg->data.eq.sensor_list == NULL)) {
		msg("No valid sensors listed");
		return -(ENOENT);
	}

	if ((cfg->data.eq.sensor == NULL)) {
		msg("No valid sensor listed");
		return -(ENOENT);
	}

	speaker_cal_data.desc = cfg->desc;

	if (cfg->data.eq.sampling_period_ms == 0)
		cfg->data.eq.sampling_period_ms = SAMPLING_MS_DEFAULT;

	if (cfg->data.eq.long_sampling_period_ms == 0)
		cfg->data.eq.long_sampling_period_ms = 30000;

	speaker_cal_data.sensor.sensor_clnt =
		sensors_manager_reg_clnt(cfg->data.eq.sensor);
	if (speaker_cal_data.sensor.sensor_clnt == NULL) {
		msg("%s: Sensor clnt create fail %s\n", __func__,
		    cfg->data.eq.sensor);
		return -(ENOENT);
	} else
		dbgmsg("%s: sensor %s", __func__,
		       cfg->data.eq.sensor);

	/* Allocate handle array and set to NULL */
	speaker_cal_data.sensors_arr = (struct speaker_sensor_info *)
			malloc(sizeof(struct speaker_sensor_info) *
			       cfg->data.eq.list_cnt);

	if (speaker_cal_data.sensors_arr == NULL) {
		msg("%s: Allocate failed", __func__);
		return -(ENOMEM);
	}

	memset(speaker_cal_data.sensors_arr, 0x0,
	       sizeof(struct speaker_sensor_info) * cfg->data.eq.list_cnt);

	for (idx = 0; idx < cfg->data.eq.list_cnt; idx++) {
		speaker_cal_data.sensors_arr[speaker_cal_data.sensors_cnt].sensor_clnt =
			 sensors_manager_reg_clnt(cfg->data.eq.sensor_list[idx]);
		/* Don't completely fail on a single sensor client create fail. */
		if (speaker_cal_data.sensors_arr[speaker_cal_data.sensors_cnt].sensor_clnt == NULL)
			msg("%s: Sensor clnt create fail %s\n", __func__,
			    cfg->data.eq.sensor_list[idx]);
		else {
			speaker_cal_data.sensors_cnt++;
			dbgmsg("%s: sensor %s", __func__,
			       cfg->data.eq.sensor_list[idx]);
		}
	}

	if (speaker_cal_data.sensors_cnt == 0) {
		msg("No valid sensors listed");
		free(speaker_cal_data.sensors_arr);
		return -(ENOENT);
	}

	speaker_cal_data.setting = &cfg->data.eq;
	speaker_cal_data.enabled = 1;

	dbgmsg("%s: Desc %s, max_temp %d mC, temp_range %d mC, sampling %d ms, "
	       "offset %d mC", __func__, speaker_cal_data.desc,
	       speaker_cal_data.setting->max_temp,
	       speaker_cal_data.setting->temp_range,
	       speaker_cal_data.setting->sampling_period_ms,
	       speaker_cal_data.setting->offset);
	dbgmsg("%s: sensor_delta %d mC, long_sample %d ms, long_sample_cnt %d, "
	       "long_sample_range %d mC, pre_cal_delay %d ms", __func__,
	       speaker_cal_data.setting->sensor_delta,
	       speaker_cal_data.setting->long_sampling_period_ms,
	       speaker_cal_data.setting->long_sampling_cnt,
	       speaker_cal_data.setting->long_sample_range,
	       speaker_cal_data.setting->pre_cal_delay_ms);
	print_setting(cfg);

	/* Create alarm handler thread */
	if (msm_thermal_alarm == ALARM_MSM_THERMAL_NODE) {
		err = pthread_create(&alarm_thread, NULL, (void *)&alarm_handler, NULL);
		if (err) {
			msg("%s: error creating thread %d", __func__, err);
			return -(EFAULT);
		}
	}

	server_handle = thermal_server_register_client_req_handler("spkr", speaker_cal_notify, NULL);
	if (server_handle == 0) {
		msg("Error initializing speaker_cal server handler");
	}
	return 0;
}

