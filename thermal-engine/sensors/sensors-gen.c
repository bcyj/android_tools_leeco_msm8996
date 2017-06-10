/*===========================================================================

  sensor-gen.c

  DESCRIPTION
  Generic thermal zone sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "sensors-gen.h"
#include "sensors_manager_internal.h"
#include "thermal.h"

int gen_sensors_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	char name[MAX_PATH] = {0};
	int tzn = 0;

	tzn = get_tzn(sensor->name);
	if (tzn < 0) {
		msg("No thermal zone device found in the kernel for sensor %s\n", sensor->name);
		return sensor_count;
	}

	sensor->tzn = tzn;
	snprintf(name, MAX_PATH, TZ_TEMP, sensor->tzn);

	fd = open(name, O_RDONLY);
	if (fd > -1) {
		sensor->fd = fd;
		sensor_count++;
	} else {
		msg("%s: Error opening %s\n", __func__, name);
	}

	return sensor_count;
}

void gen_sensors_shutdown(struct sensor_info *sensor)
{
	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	if (sensor->fd > -1)
		close(sensor->fd);
}

int gen_sensor_get_temperature(struct sensor_info *sensor)
{
	char buf[10] = {0};
	int temp = 0;

	if (read(sensor->fd, buf, sizeof(buf) - 1) != -1)
		temp = atoi(buf);
	lseek(sensor->fd, 0, SEEK_SET);

	return temp;
}
