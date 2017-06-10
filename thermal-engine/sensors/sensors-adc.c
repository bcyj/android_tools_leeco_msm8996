/*===========================================================================

  sensor-adc.c

  DESCRIPTION
  ADC sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <dirent.h>
#include <stringl.h>

#include "sensors-adc.h"
#include "sensors_manager_internal.h"
#include "thermal.h"

#define MSM_ADC_NODE  "/sys/devices/platform/msm_ssbi.0/pm8921-core/pm8xxx-adc/"
#define LVL_BUF_MAX (12)
#define OF_NODE "qpnp-vadc"
#define SYS_ADC_DIR "/sys/bus/spmi/devices/"

static int get_adc_dir_name(char *name)
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	char cwd[MAX_PATH] = {0};
	int ret = -ENODEV;

	if (!getcwd(cwd, sizeof(cwd)))
		return ret;

	chdir(SYS_ADC_DIR);
	tdir = opendir(SYS_ADC_DIR);
	if (!tdir) {
		msg("Unable to open %s\n", SYS_ADC_DIR);
		return -EFAULT;
	}

	while ((tdirent = readdir(tdir))) {
		if (strstr(tdirent->d_name, OF_NODE)) {
			snprintf(name, MAX_PATH, "%s%s/",
					SYS_ADC_DIR, tdirent->d_name);
			dbgmsg("Found %s at %s\n", OF_NODE, name);
			ret = 0;
			break;
		}
	}

	closedir(tdir);
	chdir(cwd);

	return ret;
}

static int adc_get_node_name(const char *node, char *path)
{
	static uint8_t init_done;
	static char qualifier[MAX_PATH] = {0};
	int ret = 0;

	if (!init_done) {
		char temp[MAX_PATH] = {0};

		ret = get_adc_dir_name(temp);
		if (!ret) {
			strlcpy(qualifier, temp, sizeof(qualifier));
		} else {
			char buf[MAX_PATH] = {0};
			snprintf(temp, MAX_PATH, "%s%s", MSM_ADC_NODE, node);
			ret = read_line_from_file(temp, buf, sizeof(buf));
			if (ret > 0)
				strlcpy(qualifier, MSM_ADC_NODE,
						sizeof(qualifier));
		}
		init_done = 1;
	}

	if (!qualifier[0])
		return -ENODEV;

	snprintf(path, MAX_PATH, "%s%s", qualifier, node);

	return 0;
}


int adc_sensors_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	char name[MAX_PATH] = {0};
	int ret = 0;

	ret = adc_get_node_name(sensor->name, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return sensor_count;
	}
	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		return sensor_count;
	}

	sensor_count++;
	sensor->fd = fd;

	return sensor_count;
}

void adc_sensors_shutdown(struct sensor_info *sensor)
{
	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	if (sensor->fd > 0)
		close(sensor->fd);
}

int adc_sensor_get_temperature(struct sensor_info *sensor)
{
	char buf[3*LVL_BUF_MAX] = {0};
	int temp = 0;

	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}

	if (read(sensor->fd, buf, sizeof(buf) - 1) != -1) {
		sscanf(buf, "Result:%d Raw:%*d\n", &temp);
	}

	lseek(sensor->fd, 0, SEEK_SET);
	return CONV(temp);
}
