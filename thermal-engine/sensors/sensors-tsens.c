/*===========================================================================

  sensor-tsens.c

  DESCRIPTION
  TSENS sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <dirent.h>
#include <fnmatch.h>
#include "sensors-tsens.h"
#include "sensors_manager_internal.h"
#include "thermal.h"

#define TSENS_TZ_TRIP_TYPE "/sys/devices/virtual/thermal/thermal_zone%d/trip_point_%d_type"
#define TSENS_TZ_TRIP_TEMP "/sys/devices/virtual/thermal/thermal_zone%d/trip_point_%d_temp"
#define TSENS_TZ_DIR "/sys/devices/virtual/thermal/thermal_zone%d/"
#define TSENS_TZ_TRIP_TYPE_FORMAT_STR "trip_point_%d_type"
#define TSENS_TZ_TRIP_TYPE_SEARCH_PATTERN "trip_point_*_type"
#define TSENS_TZ_TRIP_VAL_MAX 20
#define TSENS_TZ_TRIP_CONFIG_HI "configurable_hi"
#define TSENS_TZ_TRIP_CONFIG_LOW "configurable_low"
#define LVL_BUF_MAX (12)

struct tsens_data {
	pthread_t tsens_thread;
	pthread_mutex_t tsens_mutex;
	pthread_cond_t tsens_condition;
	int threshold_reached;
	int sensor_shutdown;
	int trip_min;
	int trip_max;
	struct sensor_info *sensor;
};

static void *tsens_uevent(void *data)
{
	int err = 0;
	struct sensor_info *sensor = (struct sensor_info *)data;
	struct pollfd fds;
	int fd;
	char uevent[MAX_PATH] = {0};
	char buf[MAX_PATH] = {0};
	struct tsens_data *tsens = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return NULL;
	}
	tsens = (struct tsens_data *) sensor->data;

	/* Looking for tsens uevent */
	snprintf(uevent, MAX_PATH, TZ_TYPE, sensor->tzn);

	fd = open(uevent, O_RDONLY);
	if (fd < 0) {
		msg("Unable to open %s to receive notifications.\n", uevent);
		return NULL;
	};

	while (!tsens->sensor_shutdown) {
		fds.fd = fd;
		fds.events = POLLERR|POLLPRI;
		fds.revents = 0;
		err = poll(&fds, 1, -1);
		if (err == -1) {
			msg("Error in uevent poll.\n");
			break;
		}

		read(fd, buf, sizeof(buf));
		lseek(fd, 0, SEEK_SET);

		dbgmsg("%s: %s", __func__, buf);

		/* notify the waiting threads */
		pthread_mutex_lock(&(tsens->tsens_mutex));
		tsens->threshold_reached = 1;
		pthread_cond_broadcast(&(tsens->tsens_condition));
		pthread_mutex_unlock(&(tsens->tsens_mutex));
	}
	close(fd);

	return NULL;
}

static void enable_sensor(struct tsens_data *tsens, int enabled)
{
	int ret = 0;
	char name[MAX_PATH] = {0};

	if (NULL == tsens ||
	    NULL == tsens->sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	snprintf(name, MAX_PATH, TZ_MODE, tsens->sensor->tzn);
	if (enabled)
		ret = write_to_file(name, "enabled", strlen("enabled"));
	else
		ret = write_to_file(name, "disabled", strlen("disabled"));

	if (ret <= 0) {
		msg("TSENS tzn %d failed to set mode %d\n", tsens->sensor->tzn, enabled);
	} else {
		dbgmsg("TSENS tzn %d mode set to %d\n", tsens->sensor->tzn, enabled);
	}
}

static void enable_threshold(struct tsens_data *tsens, int trip, int enabled)
{
	int ret = 0;
	char name[MAX_PATH] = {0};

	if (NULL == tsens ||
	    NULL == tsens->sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	snprintf(name, MAX_PATH, TSENS_TZ_TRIP_TYPE, tsens->sensor->tzn, trip);
	dbgmsg("%s: %s (%s)\n", __func__, tsens->sensor->name, name);

	if (enabled)
		ret = write_to_file(name, "enabled", strlen("enabled"));
	else
		ret = write_to_file(name, "disabled", strlen("disabled"));

	if (ret <= 0) {
		msg("TSENS threshold at %d failed to %d\n", trip, enabled);
	} else {
		dbgmsg("TSENS threshold at %d enabled: %d\n", trip, enabled);
	}
}

static void set_thresholds(struct tsens_data *tsens, struct thresholds_req_t *thresh)
{
	char minname[MAX_PATH]= {0};
	char maxname[MAX_PATH]= {0};
	char buf[LVL_BUF_MAX] = {0};
	int ret = 0;
	int mintemp = 0;

	if (NULL == tsens) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	snprintf(minname, MAX_PATH, TSENS_TZ_TRIP_TEMP, tsens->sensor->tzn, tsens->trip_min);
	snprintf(maxname, MAX_PATH, TSENS_TZ_TRIP_TEMP, tsens->sensor->tzn, tsens->trip_max);

	/* Set thresholds in legal order */
	if (read_line_from_file(minname, buf, sizeof(buf)) > 0) {
		mintemp = atoi(buf);
	}

	if (thresh->high >= mintemp) {
		/* set high threshold first */
		if (thresh->high_valid) {
			dbgmsg("Setting up TSENS thresholds high: %d\n", thresh->high);
			snprintf(buf, LVL_BUF_MAX, "%d", thresh->high);
			ret = write_to_file(maxname, buf, strlen(buf));
			if (ret <= 0)
				msg("TSENS threshold high failed to set %d\n", thresh->high);
		}
		enable_threshold(tsens, tsens->trip_max, thresh->high_valid);

		if (thresh->low_valid) {
			dbgmsg("Setting up TSENS thresholds low: %d\n", thresh->low);
			snprintf(buf, LVL_BUF_MAX, "%d", thresh->low);
			ret = write_to_file(minname, buf, strlen(buf));
			if (ret <= 0)
				msg("TSENS threshold low failed to set %d\n", thresh->low);
		}
		enable_threshold(tsens, tsens->trip_min, thresh->low_valid);

	} else {
		if (thresh->low_valid) {
			dbgmsg("Setting up TSENS thresholds low: %d\n", thresh->low);
			snprintf(buf, LVL_BUF_MAX, "%d", thresh->low);
			ret = write_to_file(minname, buf, strlen(buf));
			if (ret <= 0)
				msg("TSENS threshold low failed to set %d\n", thresh->low);
		}
		enable_threshold(tsens, tsens->trip_min, thresh->low_valid);

		if (thresh->high_valid) {
			dbgmsg("Setting up TSENS thresholds high: %d\n", thresh->high);
			snprintf(buf, LVL_BUF_MAX, "%d", thresh->high);
			ret = write_to_file(maxname, buf, strlen(buf));
			if (ret <= 0)
				msg("TSENS threshold high failed to set %d\n", thresh->high);
		}
		enable_threshold(tsens, tsens->trip_max, thresh->high_valid);
	}
}

void tsens_sensor_enable_thresholds(struct sensor_info *sensor, int hi_enabled,
				    int lo_enabled)
{
#ifdef ENABLE_TSENS_INTERRUPT
	struct tsens_data *tsens;
	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	tsens = (struct tsens_data *) sensor->data;

	enable_threshold(tsens, tsens->trip_max, hi_enabled);
	enable_threshold(tsens, tsens->trip_min, lo_enabled);
#endif
}

void tsens_sensor_enable_sensor(struct sensor_info *sensor, int enabled)
{
	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	enable_sensor(sensor->data, enabled);
}

int tsens_trip_filter(const struct dirent * file)
{
	if (fnmatch(TSENS_TZ_TRIP_TYPE_SEARCH_PATTERN, file->d_name,
		FNM_PATHNAME) == 0) {
		return 1;
	} else {
		return 0;
	}
}

void init_tsens_trip_type(struct tsens_data * tsens)
{
	char trip_path[MAX_PATH] = "";
	char trip_val[TSENS_TZ_TRIP_VAL_MAX] = "";
	int tsens_num = 0, index = 0, trip_type = 0, match = 0;
	struct dirent **file_list;
	unsigned short max_trip_found = 0, min_trip_found = 0;

	if (tsens == NULL) {
		msg("%s: Invalid input\n", __func__);
		return;
	}

	tsens_num = tsens->sensor->tzn;
	snprintf(trip_path, MAX_PATH, TSENS_TZ_DIR, tsens_num);
	match = scandir(trip_path, &file_list, &tsens_trip_filter, 0);

	if (match <= 0) {
		if (match == 0) {
			msg("%s: no match found by scandir in %s\n",
				__func__, trip_path);
		} else {
			msg("%s: Error in scanning directory %s\n",
				__func__, trip_path);
		}
		goto TRIP_INIT_EXIT;
	}

	for(; (index < match) && (!max_trip_found || !min_trip_found);
		index++) {
		trip_type = -1;
		sscanf(file_list[index]->d_name,
			TSENS_TZ_TRIP_TYPE_FORMAT_STR, &trip_type);
		if (trip_type < 0) {
			msg("%s: Error in reading trip_type from file %s\n",
				__func__, file_list[index]->d_name);
			continue;
		}
		memset(trip_path, '\0', MAX_PATH);
		memset(trip_val, '\0', TSENS_TZ_TRIP_VAL_MAX);
		snprintf(trip_path, MAX_PATH, TSENS_TZ_TRIP_TYPE,
				tsens_num, trip_type);

		if (read_line_from_file(trip_path, trip_val,
			TSENS_TZ_TRIP_VAL_MAX) <= 0)
			continue;

		if (!max_trip_found && (strncmp(trip_val,
			TSENS_TZ_TRIP_CONFIG_HI,
			strlen(TSENS_TZ_TRIP_CONFIG_HI)) == 0)) {
			max_trip_found = 1;
			tsens->trip_max = trip_type;
		} else if (!min_trip_found && (strncmp(trip_val,
			TSENS_TZ_TRIP_CONFIG_LOW,
			strlen(TSENS_TZ_TRIP_CONFIG_LOW)) == 0)) {
			min_trip_found = 1;
			tsens->trip_min = trip_type;
		}
	}

	while (match--)
		free(file_list[match]);
	free(file_list);
TRIP_INIT_EXIT:
	if (!max_trip_found || !min_trip_found) {
		msg("%s: Error in determining trip type for tsens:%d\n",
			__func__, tsens_num);
		tsens->trip_min = 1;
		tsens->trip_max = 0;
	}
	dbgmsg("%s: tsens_num: %d trip_min: %d, trip_max: %d\n", __func__,
		tsens_num, tsens->trip_min, tsens->trip_max);
}


/* NOTE: tsens_sensors_setup() function does not enable the sensor
 * or set thresholds. This should be done in the target-specific setup */
int tsens_sensors_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	char name[MAX_PATH] = {0};
	int tzn = 0;
	struct tsens_data *tsens = NULL;

	tzn = get_tzn(sensor->name);
	if (tzn < 0) {
		msg("No thermal zone device found in the kernel for sensor %s\n", sensor->name);
		return sensor_count;
	}
	sensor->tzn = tzn;

	snprintf(name, MAX_PATH, TZ_TEMP, sensor->tzn);
	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, TZ_TEMP);
		return sensor_count;
	}

	/* Allocate TSENS data */
	tsens = (struct tsens_data *) malloc(sizeof(struct tsens_data));
	if (NULL == tsens) {
		msg("%s: malloc failed", __func__);
		close(fd);
		return sensor_count;
	}
	memset(tsens, 0, sizeof(struct tsens_data));
	sensor->data = (void *) tsens;

	sensor_count++;
	pthread_mutex_init(&(tsens->tsens_mutex), NULL);
	pthread_cond_init(&(tsens->tsens_condition), NULL);
	tsens->sensor_shutdown = 0;
	tsens->threshold_reached = 0;
	tsens->sensor = sensor;
	sensor->fd = fd;
	init_tsens_trip_type(tsens);

	if (sensor->interrupt_enable) {
		pthread_create(&(tsens->tsens_thread), NULL,
			       tsens_uevent, sensor);
	}

	return sensor_count;
}

void tsens_sensors_shutdown(struct sensor_info *sensor)
{
	struct tsens_data *tsens;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	tsens = (struct tsens_data *) sensor->data;
	tsens->sensor_shutdown = 1;

	if (sensor->fd > 0)
		close(sensor->fd);

	if (sensor->interrupt_enable)
		pthread_join(tsens->tsens_thread, NULL);
	free(tsens);
}

int tsens_sensor_get_temperature(struct sensor_info *sensor)
{
	char buf[10] = {0};
	int temp = 0;

	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}

	if (read(sensor->fd, buf, sizeof(buf)) != -1)
		temp = atoi(buf);
	lseek(sensor->fd, 0, SEEK_SET);

	return CONV(temp);
}

void tsens_sensor_interrupt_wait(struct sensor_info *sensor)
{
	struct tsens_data *tsens;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	if (sensor->interrupt_enable) {
		tsens = (struct tsens_data *) sensor->data;
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(tsens->tsens_mutex));
		while (!tsens->threshold_reached) {
			pthread_cond_wait(&(tsens->tsens_condition),
					&(tsens->tsens_mutex));
		}
		tsens->threshold_reached = 0;
		pthread_mutex_unlock(&(tsens->tsens_mutex));
	}
}

void tsens_sensor_update_thresholds(struct sensor_info *sensor,
				    struct thresholds_req_t *thresh)
{
	struct tsens_data *tsens;

	if (NULL == thresh ||
	    NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	tsens = (struct tsens_data *) sensor->data;

	/* Convert thresholds to Celsius for TSENS*/
	thresh->high = RCONV(thresh->high);
	thresh->low = RCONV(thresh->low);

	set_thresholds(tsens, thresh);
}

