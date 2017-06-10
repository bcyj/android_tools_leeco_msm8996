/*===========================================================================

  pm8821-sensor.c

  DESCRIPTION
  pm8821 temperature alarm access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>

#include "sensors-pm8821.h"
#include "sensors_manager_internal.h"
#include "thermal.h"

struct pmic_data {
	pthread_t pmic_thread;
	pthread_mutex_t pmic_mutex;
	pthread_cond_t pmic_condition;
	int threshold_reached;
	int temp_idx;
	int sensor_shutdown;
	struct sensor_info *sensor;
} pmic_data;

static void *pmic_uevent(void *data)
{
	int err = 0;
	struct sensor_info *sensor = (struct sensor_info *)data;
	struct pollfd fds;
	int fd;
	char uevent[MAX_PATH] = {0};
	char buf[MAX_PATH] = {0};
	struct pmic_data *pmic = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return NULL;
	}
	pmic = (struct pmic_data *) sensor->data;

	/* Looking for pmic uevent */
	snprintf(uevent, MAX_PATH, TZ_TYPE, pmic->sensor->tzn);
	fd = open(uevent, O_RDONLY);
	if (fd < 0) {
		msg("Unable to open %s to receive notifications.\n", uevent);
		return NULL;
	};

	while (!pmic->sensor_shutdown) {
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

		dbgmsg("pmic uevent :%s", buf);

		/* notify the waiting threads */
		pthread_mutex_lock(&(pmic->pmic_mutex));
		pmic->threshold_reached = 1;
		pthread_cond_broadcast(&(pmic->pmic_condition));
		pthread_mutex_unlock(&(pmic->pmic_mutex));
	}
	close(fd);

	return NULL;
}

void pm8821_interrupt_wait(struct sensor_info *sensor)
{
	struct pmic_data *pmic;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	pmic = (struct pmic_data *) sensor->data;

	if (sensor->interrupt_enable) {
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(pmic->pmic_mutex));
		while (!pmic->threshold_reached) {
			pthread_cond_wait(&(pmic->pmic_condition),
					&(pmic->pmic_mutex));
		}
		pmic->threshold_reached = 0;
		pthread_mutex_unlock(&(pmic->pmic_mutex));
	}
}

/* pm8821_tz sensor specific set up */
int pm8821_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	int tzn = 0;
	char name[MAX_PATH] = {0};
	struct pmic_data *pmic = NULL;

	if (sensor == NULL) {
		msg("pm8821_tz:unexpected NULL");
		return 0;
	}

	tzn = get_tzn(sensor->name);
	if (tzn < 0) {
		msg("No thermal zone device found in the kernel for sensor %s\n", sensor->name);
		return sensor_count;
	}
	sensor->tzn = tzn;

	/* Allocate pmic data */
	pmic = (struct pmic_data *) malloc(sizeof(struct pmic_data));
	if (NULL == pmic) {
		msg("%s: malloc failed", __func__);
		return sensor_count;
	}
	memset(pmic, 0, sizeof(pmic_data));
	sensor->data = (void *) pmic;
	pmic->sensor = sensor;

	snprintf(name, MAX_PATH, TZ_TEMP, sensor->tzn);
	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		free(pmic);
		return sensor_count;
	}
	pmic->temp_idx = fd;
	sensor_count++;
	pthread_mutex_init(&(pmic->pmic_mutex), NULL);
	pthread_cond_init(&(pmic->pmic_condition), NULL);
	pmic->sensor_shutdown = 0;
	pmic->threshold_reached = 0;

	if (sensor->interrupt_enable) {
		pthread_create(&(pmic->pmic_thread), NULL,
				pmic_uevent, sensor);
	}

	return sensor_count;
}

int pm8821_get_temperature(struct sensor_info *sensor)
{
	struct pmic_data *pmic;
	int temp = CONV(-273);
	char buf[MAX_PATH] = {0};

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return temp;
	}

	pmic = (struct pmic_data *) sensor->data;

	if (read(pmic->temp_idx, buf, sizeof(buf) - 1) != -1)
		temp = atoi(buf);
	lseek(pmic->temp_idx, 0, SEEK_SET);

	return temp;
}

void pm8821_shutdown(struct sensor_info *sensor)
{
	struct pmic_data *pmic;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	pmic = (struct pmic_data *) sensor->data;
	pmic->sensor_shutdown = 1;
	if (sensor->interrupt_enable)
		pthread_join(pmic->pmic_thread, NULL);
	free(pmic);
}
