/*===========================================================================

  sensor-thermal.c

  DESCRIPTION
  Access functions for the thermal sensors registered via thermal sys.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include "sensors-thermal.h"
#include "sensors_manager_internal.h"
#include "thermal.h"

struct thermal_sensor_data {
	pthread_t thermal_sensor_thread;
	pthread_mutex_t thermal_sensor_mutex;
	pthread_cond_t thermal_sensor_condition;
	int threshold_reached;
	int sensor_shutdown;
	struct sensor_info *sensor;
};

static void *thermal_sensor_uevent(void *data)
{
	int err = 0;
	struct sensor_info *sensor = (struct sensor_info *)data;
	struct pollfd fds;
	int fd;
	char uevent[MAX_PATH] = {0};
	char buf[MAX_PATH] = {0};
	struct thermal_sensor_data *sensor_data = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return NULL;
	}
	sensor_data = (struct thermal_sensor_data *) sensor->data;

	/* Looking for thermal sensor uevent */
	snprintf(uevent, MAX_PATH, TZ_TYPE, sensor_data->sensor->tzn);
	fd = open(uevent, O_RDONLY);
	if (fd < 0) {
		msg("Unable to open %s to receive notifications.\n", uevent);
		return NULL;
	};

	while (!sensor_data->sensor_shutdown) {
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

		dbgmsg("thermal sensor uevent :%s", buf);

		/* notify the waiting threads */
		pthread_mutex_lock(&(sensor_data->thermal_sensor_mutex));
		sensor_data->threshold_reached = 1;
		pthread_cond_broadcast(&(sensor_data->thermal_sensor_condition));
		pthread_mutex_unlock(&(sensor_data->thermal_sensor_mutex));
	}
	close(fd);

	return NULL;
}

void thermal_sensor_interrupt_wait(struct sensor_info *sensor)
{
	struct thermal_sensor_data *sensor_data;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	sensor_data = (struct thermal_sensor_data *) sensor->data;

	if (sensor->interrupt_enable) {
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(sensor_data->thermal_sensor_mutex));
		while (!sensor_data->threshold_reached) {
			pthread_cond_wait(&(sensor_data->thermal_sensor_condition),
					&(sensor_data->thermal_sensor_mutex));
		}
		sensor_data->threshold_reached = 0;
		pthread_mutex_unlock(&(sensor_data->thermal_sensor_mutex));
	}
}

int thermal_sensor_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	int tzn = 0;
	char name[MAX_PATH] = {0};
	struct thermal_sensor_data *sensor_data = NULL;

	if (sensor == NULL) {
		msg("thermal_sensor_tz:unexpected NULL");
		return 0;
	}

	tzn = get_tzn(sensor->name);
	if (tzn < 0) {
		msg("No thermal zone device found in the kernel for sensor %s\n", sensor->name);
		return sensor_count;
	}
	sensor->tzn = tzn;

	snprintf(name, MAX_PATH, TZ_TEMP, sensor->tzn);
	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		return sensor_count;
	}

	/* Allocate for thermal sensor data */
	sensor_data = (struct thermal_sensor_data *) malloc(sizeof(struct thermal_sensor_data));
	if (NULL == sensor_data) {
		msg("%s: malloc failed", __func__);
		close(fd);
		return sensor_count;
	}
	memset(sensor_data, 0, sizeof(struct thermal_sensor_data));
	sensor->data = (void *) sensor_data;
	sensor->fd = fd;

	sensor_count++;
	pthread_mutex_init(&(sensor_data->thermal_sensor_mutex), NULL);
	pthread_cond_init(&(sensor_data->thermal_sensor_condition), NULL);
	sensor_data->sensor_shutdown = 0;
	sensor_data->threshold_reached = 0;
	sensor_data->sensor = sensor;

	if (sensor->interrupt_enable) {
		pthread_create(&(sensor_data->thermal_sensor_thread), NULL,
				thermal_sensor_uevent, sensor);
	}

	return sensor_count;
}

int thermal_sensor_get_temperature(struct sensor_info *sensor)
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
	temp = CONV(temp)/sensor->scaling_factor;

	return temp;
}

void thermal_sensor_shutdown(struct sensor_info *sensor)
{
	struct thermal_sensor_data *sensor_data;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	sensor_data = (struct thermal_sensor_data *) sensor->data;
	sensor_data->sensor_shutdown = 1;

	if (sensor->fd > -1)
		close(sensor->fd);

	if (sensor->interrupt_enable)
		pthread_join(sensor_data->thermal_sensor_thread, NULL);
	free(sensor_data);
}
