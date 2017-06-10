/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "thermal.h"
#include "sensors-tsens.h"
#include "sensors-bcl.h"
#include "sensors-thermal.h"
#include "sensors.h"
#include "sensors_manager_internal.h"

#define SENSOR_INFO_SYSFS     "/sys/module/msm_thermal/sensor_info"
#define THERMAL_SYSFS         "/sys/devices/virtual/thermal"
#define SENSOR_TZ_TYPE        "/sys/devices/virtual/thermal/%s/type"
#define MAX_SENSOR_INFO_LEN   4096
#define TSENS_TYPE            "tsens"
#define ALARM_TYPE            "alarm"
#define ADC_TYPE              "adc"

static struct sensor_info bcl_sensors[] = {
	{
		.name = "bcl",
		.setup = bcl_setup,
		.shutdown = bcl_shutdown,
		.get_temperature = bcl_get_iavail,
		.interrupt_wait = bcl_interrupt_wait,
		.update_thresholds = bcl_update_thresholds,
		.tzn = 0,
		.data = NULL,
		.interrupt_enable = 1,
	}
};

#define MAX_NUM_SENSORS 50
struct msm_sensor_info {
	char type[MAX_SENSOR_NAME_LEN];
	char name[MAX_SENSOR_NAME_LEN];
	char alias[MAX_SENSOR_NAME_LEN];
	int  scaling_factor;
};

static struct msm_sensor_info sensors[MAX_NUM_SENSORS];
static int sensor_cnt;

static void init_sensor_alias()
{
	char buf[MAX_SENSOR_INFO_LEN];
	char *psensor_info = NULL, *psave1 = NULL;

	if (read_line_from_file(SENSOR_INFO_SYSFS, buf,
			MAX_SENSOR_INFO_LEN) <= 0) {
		msg("%s: Error reading sensor info node %s\n",
			__func__, SENSOR_INFO_SYSFS);
		return;
	}
	psensor_info = strtok_r(buf, " ", &psave1);
	while (psensor_info != NULL && sensor_cnt < MAX_NUM_SENSORS) {
		char *psensor = NULL;
		char *psave2 = NULL;

		psensor = strtok_r(psensor_info, ":", &psave2);
		if ((psensor == NULL) || strlcpy(sensors[sensor_cnt].type,
			psensor, MAX_SENSOR_NAME_LEN) >= MAX_SENSOR_NAME_LEN)
			goto next_sensor;

		psensor = strtok_r(NULL, ":", &psave2);
		if ((psensor == NULL) || strlcpy(sensors[sensor_cnt].name,
			psensor, MAX_SENSOR_NAME_LEN) >= MAX_SENSOR_NAME_LEN)
			goto next_sensor;

		if (psensor[strlen(psensor) + 1] != ':') {
			psensor = strtok_r(NULL, ":", &psave2);
			if ((psensor == NULL) ||
				strlcpy(sensors[sensor_cnt].alias, psensor,
				MAX_SENSOR_NAME_LEN) >= MAX_SENSOR_NAME_LEN)
					goto next_sensor;
		}

		psensor = strtok_r(NULL, ":", &psave2);
		if (psensor == NULL)
			goto next_sensor;
		sensors[sensor_cnt].scaling_factor =
			(int)strtol( psensor, NULL, 10);
		sensor_cnt++;
next_sensor:
		psensor_info = strtok_r(NULL, " ", &psave1);
	}
	dbgmsg("%s: sensor_cnt:%d\n", __func__, sensor_cnt);
}

static char *find_alias(const char *sensor_name)
{
	char *ret_val = NULL;
	int i;

	if (sensor_name == NULL) {
		msg("%s: sensor name is NULL\n", __func__);
		return ret_val;
	}
	for (i = 0; i < sensor_cnt; i++) {
		if (strncmp(sensors[i].name, sensor_name,
				MAX_SENSOR_NAME_LEN) == 0) {
			if (strlen(sensors[i].alias))
				ret_val = sensors[i].alias;
			break;
		}
	}
	dbgmsg("%s: sensor:%s alias:%s\n",
		__func__, sensor_name, ret_val ? : "NULL");

	return ret_val;
}

static void generic_wait(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	sensor->interrupt_wait(sensor);
}

static int generic_read(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	return sensor->get_temperature(sensor);
}

static void generic_update_thresholds(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	sensor->update_thresholds(sensor,
				 &sensor_mgr->active_thresh.thresh);
}

static void generic_shutdown(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	sensor->shutdown(sensor);

	free(sensor_mgr);
}

static int add_tgt_sensor(struct sensor_info *sensor)
{
	int ret_val = 0;
	struct sensors_mgr_sensor_info *sensor_mgr = NULL;

	sensor_mgr = malloc(sizeof(struct sensors_mgr_sensor_info));
	if (sensor_mgr == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto error_handler;
	}
	memset(sensor_mgr, 0, sizeof(struct sensors_mgr_sensor_info));

	sensor_mgr->name = sensor->name;
	sensor_mgr->alias = find_alias(sensor->name);

	if (sensor->setup(sensor) == 0) {
		ret_val = -(EFAULT);
		goto error_handler;
	}

	sensor_mgr->data = (void*) sensor;
	sensor->sensor_mgr = sensor_mgr;

	sensor_mgr->get_temperature = generic_read;
	sensor_mgr->shutdown = generic_shutdown;

	if (sensor->interrupt_wait)
		sensor_mgr->wait = generic_wait;
	if (sensor->update_thresholds)
		sensor_mgr->update_thresholds = generic_update_thresholds;

	sensors_manager_add_sensor(sensor_mgr);

error_handler:
	if (ret_val) {
		if (sensor_mgr)
			free(sensor_mgr);
	}
	return ret_val;
}

int add_tgt_sensors_set(struct sensor_info *sensor_arr, int arr_size)
{
	int idx = 0;
	int ret_val = 0;

	for (idx = 0; idx < arr_size; idx++) {
		if (add_tgt_sensor(&sensor_arr[idx]) != 0) {
			msg("%s: Error adding %s\n", __func__,
			    sensor_arr[idx].name);
			ret_val = -(EFAULT);
			break;
		}
	}

	return ret_val;
}

static int add_tgt_tsens_sensors(char *sensor_name)
{
	int ret_val = 0;
	struct sensor_info *sensor;

	sensor = malloc(sizeof(struct sensor_info));
	if (sensor == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto error_handler;
	}
	memset(sensor, 0, sizeof(struct sensor_info));

	sensor->name = malloc(MAX_SENSOR_NAME_LEN);
	if (sensor->name == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto error_handler;
	}
	memset(sensor->name, 0, MAX_SENSOR_NAME_LEN);

	strlcpy(sensor->name, sensor_name, MAX_SENSOR_NAME_LEN);
	sensor->setup = tsens_sensors_setup;
	sensor->shutdown = tsens_sensors_shutdown;
	sensor->get_temperature = tsens_sensor_get_temperature;
	sensor->interrupt_wait = tsens_sensor_interrupt_wait;
	sensor->update_thresholds = tsens_sensor_update_thresholds;
	sensor->tzn = 0;
	sensor->data = NULL;
	sensor->interrupt_enable = 1;
	add_tgt_sensor(sensor);

error_handler:
	if (ret_val) {
		if (sensor) {
			if (sensor->name)
				free(sensor->name);
			free(sensor);
		}
	}
	return ret_val;
}


static int add_tgt_gen_sensors(char *sensor_name, struct msm_sensor_info *sens_info)
{
	int ret_val = 0;
	struct sensor_info *sensor;

	sensor = malloc(sizeof(struct sensor_info));
	if (sensor == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto error_handler;
	}
	memset(sensor, 0, sizeof(struct sensor_info));

	sensor->name = malloc(MAX_SENSOR_NAME_LEN);
	if (sensor->name == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto error_handler;
	}
	memset(sensor->name, 0, MAX_SENSOR_NAME_LEN);

	strlcpy(sensor->name, sensor_name, MAX_SENSOR_NAME_LEN);
	sensor->setup = thermal_sensor_setup;
	sensor->shutdown = thermal_sensor_shutdown;
	sensor->get_temperature = thermal_sensor_get_temperature;
	if (!strncmp(sens_info->type, ALARM_TYPE, strlen(ALARM_TYPE))) {
		sensor->interrupt_enable = 1;
		sensor->interrupt_wait = thermal_sensor_interrupt_wait;
	} else {
		sensor->interrupt_enable = 0;
		sensor->interrupt_wait = NULL;
	}
	sensor->scaling_factor = sens_info->scaling_factor;
	sensor->update_thresholds = NULL;
	sensor->tzn = 0;
	sensor->data = NULL;
	add_tgt_sensor(sensor);

error_handler:
	if (ret_val) {
		if (sensor) {
			if (sensor->name)
				free(sensor->name);
			free(sensor);
		}
	}
	return ret_val;
}

static void parse_thermal_zones()
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	char name[MAX_PATH] = {0};
	char cwd[MAX_PATH] = {0};
	int cnt = 0, ret = 0;

	if (!getcwd(cwd, sizeof(cwd)))
		return;

	chdir(THERMAL_SYSFS); /* Change dir to read the entries. Doesnt work
                                 otherwise */
	tdir = opendir(THERMAL_SYSFS);
	if (!tdir) {
		msg("%s: Unable to open %s\n", __func__, THERMAL_SYSFS);
		return;
	}
	while ((tdirent = readdir(tdir))) {
		char buf[MAX_PATH];
		DIR *tzdir = NULL;
		tzdir = opendir(tdirent->d_name);
		if (!tzdir)
			continue;
		snprintf(name, MAX_PATH, SENSOR_TZ_TYPE, tdirent->d_name);
		dbgmsg("%s: Opening %s\n", __func__, name);
		ret = read_line_from_file(name, buf, sizeof(buf));
		if (ret <= 0)
			continue;
		buf[strlen(buf) -1] = '\0';
		for (cnt = 0; cnt < sensor_cnt; cnt++) {
			if (strncmp(buf, sensors[cnt].name,
					MAX_SENSOR_NAME_LEN))
				continue;
			if (!strncmp(sensors[cnt].type, TSENS_TYPE,
					strlen(TSENS_TYPE))) {
				add_tgt_tsens_sensors(sensors[cnt].name);
				dbgmsg(
				"%s: Added sensor: %s to %s sensor list\n",
				__func__, buf, sensors[cnt].type);
			} else if (!strncmp(sensors[cnt].type, ALARM_TYPE,
					strlen(ALARM_TYPE)) ||
				(!strncmp(sensors[cnt].type, ADC_TYPE,
					strlen(ADC_TYPE)))) {
				add_tgt_gen_sensors(sensors[cnt].name,
							&sensors[cnt]);
				dbgmsg(
				"%s: Added sensor: %s to %s sensor list\n",
				__func__, buf, sensors[cnt].type);
			}
		}
		closedir(tzdir);
	}
	closedir(tdir);
	chdir(cwd); /* Restore current working dir */
}


static int min_mode;
int sensors_init(int minimum_mode)
{
	int ret_val = 0;
	min_mode = minimum_mode;

	if (!min_mode)
		modem_ts_qmi_init();

	init_sensor_alias();
	parse_thermal_zones();

	/* BCL */
	if (add_tgt_sensors_set(bcl_sensors,
			   ARRAY_SIZE(bcl_sensors)) != 0) {
		msg("%s: Error adding BCL TS\n", __func__);
		ret_val = -(EFAULT);
	}

	return ret_val;
}

void sensors_release(void)
{
	if (!min_mode)
		modem_qmi_ts_comm_release();
}
