/*===========================================================================

  sensor-bcl.c

  DESCRIPTION
  BCL sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  setup() function should be called before get_temperature().
  shutdown() function should be called to clean up resources.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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

#include "sensors-bcl.h"
#include "sensors_manager_internal.h"
#include "thermal.h"

#define LVL_BUF_MAX (12)

#ifndef BCL_DUMP_FILE_DEFAULT
#define BCL_DUMP_FILE_DEFAULT "/data/bcl_dump.log"
#endif

/* BCL sysfs defines */
#define BCL_MODE "mode"
#define BCL_TYPE "type"
#define BCL_IAVAIL "iavail"
#define BCL_VBAT_MIN "vbat_min"
#define BCL_POLL_INTERVAL "poll_interval"
#define BCL_IAVAIL_LOW_THRESHOLD_MODE "iavail_low_threshold_mode"
#define BCL_IAVAIL_HIGH_THRESHOLD_MODE "iavail_high_threshold_mode"
#define BCL_IAVAIL_LOW_THRESHOLD_VALUE "iavail_low_threshold_value"
#define BCL_IAVAIL_HIGH_THRESHOLD_VALUE "iavail_high_threshold_value"

#define BMS_SOC "/sys/class/power_supply/battery/capacity"
#define PLATFORM_DEVICE "/sys/devices/platform/battery_current_limit/"
#define OF_NODE "qcom,bcl"

#ifdef _PLATFORM_BASE
#define SYS_DEV_DIR  "/sys"_PLATFORM_BASE
#else
#define SYS_DEV_DIR "/sys/devices/"
#endif

struct bcl_data {
	pthread_t bcl_thread;
	pthread_mutex_t bcl_mutex;
	pthread_cond_t bcl_condition;
	int threshold_reached;
	int sensor_shutdown;
	struct sensor_info *sensor;

	FILE *dump_bcl_fd;
	int iavail_idx;
	int soc_idx;
	int iavail;

};

static int get_bcl_dir_name(char *name)
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	char cwd[MAX_PATH] = {0};
	int ret = -ENODEV;

	if (!getcwd(cwd, sizeof(cwd)))
		return ret;

	chdir(SYS_DEV_DIR);
	tdir = opendir(SYS_DEV_DIR);
	if (!tdir) {
		msg("Unable to open %s\n", SYS_DEV_DIR);
		return -EFAULT;
	}

	while ((tdirent = readdir(tdir))) {
		if (strstr(tdirent->d_name, OF_NODE)) {
			snprintf(name, MAX_PATH, "%s%s/",
					SYS_DEV_DIR, tdirent->d_name);
			dbgmsg("Found %s at %s\n", OF_NODE, name);
			ret = 0;
			break;
		}
	}

	closedir(tdir);
	chdir(cwd);

	return ret;
}

static int bcl_get_node_name(const char *node, char *path)
{
	static uint8_t init_done;
	static char qualifier[MAX_PATH] = {0};
	int ret = 0;

	if (!init_done) {
		char temp[MAX_PATH] = {0};

		ret = get_bcl_dir_name(temp);
		if (!ret) {
			strlcpy(qualifier, temp, sizeof(qualifier));
		} else {
			char buf[MAX_PATH] = {0};
			snprintf(temp, MAX_PATH, "%s%s", PLATFORM_DEVICE, node);
			ret = read_line_from_file(temp, buf, sizeof(buf));
			if (ret > 0)
				strlcpy(qualifier, PLATFORM_DEVICE,
						sizeof(qualifier));
		}
		init_done = 1;
	}

	if (!qualifier[0])
		return -ENODEV;

	snprintf(path, MAX_PATH, "%s%s", qualifier, node);

	return 0;
}

static void *bcl_uevent(void *data)
{
	int err = 0;
	struct sensor_info *sensor = (struct sensor_info *)data;
	struct pollfd fds;
	int fd;
	char uevent[MAX_PATH] = {0};
	char buf[MAX_PATH] = {0};
	struct bcl_data *bcl = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return NULL;
	}
	bcl = (struct bcl_data *) sensor->data;

	/* Looking for bcl uevent */
	err = bcl_get_node_name(BCL_TYPE, uevent);
	if (err) {
		msg("%s: Unexpected node error", __func__);
		return NULL;
	}

	fd = open(uevent, O_RDONLY);
	if (fd < 0) {
		msg("Unable to open %s to receive notifications.\n", uevent);
		return NULL;
	};

	while (!bcl->sensor_shutdown) {
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

		dbgmsg("BCL uevent :%s", buf);

		/* notify the waiting threads */
		pthread_mutex_lock(&(bcl->bcl_mutex));
		bcl->threshold_reached = 1;
		pthread_cond_broadcast(&(bcl->bcl_condition));
		pthread_mutex_unlock(&(bcl->bcl_mutex));
	}

	if (fd >= 0)
		close(fd);

	return NULL;
}

static void enable_bcl(struct bcl_data *bcl, int enabled)
{
	int ret = 0;
	char name[MAX_PATH] = {0};

	if (NULL == bcl ||
	    NULL == bcl->sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	ret = bcl_get_node_name(BCL_MODE, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	if (enabled)
		ret = write_to_file(name, "enable", strlen("enable"));
	else
		ret = write_to_file(name, "disable", strlen("disable"));

	if (ret <= 0) {
		msg("BCL failed to set mode %d\n", enabled);
	} else {
		dbgmsg("BCL mode set to %d\n", enabled);
	}

}

static void enable_bcl_threshold_high(struct bcl_data *bcl, int enabled)
{
	int ret = 0;
	char name[MAX_PATH] = {0};

	if (NULL == bcl ||
	    NULL == bcl->sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	ret = bcl_get_node_name(BCL_IAVAIL_HIGH_THRESHOLD_MODE, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	if (enabled)
		ret = write_to_file(name, "enable", strlen("enable"));
	else
		ret = write_to_file(name, "disable", strlen("disable"));

	if (ret <= 0) {
		msg("BCL high threshold failed to %d\n", enabled);
	} else {
		dbgmsg("BCL high threshold enabled: %d\n", enabled);
	}
}

static void enable_bcl_threshold_low(struct bcl_data *bcl, int enabled)
{
	int ret = 0;
	char name[MAX_PATH] = {0};

	if (NULL == bcl ||
	    NULL == bcl->sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	ret = bcl_get_node_name(BCL_IAVAIL_LOW_THRESHOLD_MODE, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	if (enabled)
		ret = write_to_file(name, "enable", strlen("enable"));
	else
		ret = write_to_file(name, "disable", strlen("disable"));

	if (ret <= 0) {
		msg("BCL low threshold failed to %d\n", enabled);
	} else {
		dbgmsg("BCL low threshold enabled: %d\n", enabled);
	}
}

static void set_bcl_thresholds(struct bcl_data *bcl, struct thresholds_req_t *thresh)
{
	char highname[MAX_PATH]= {0};
	char lowname[MAX_PATH]= {0};
	char buf[LVL_BUF_MAX] = {0};
	int ret = 0;

	if (NULL == bcl) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	ret = bcl_get_node_name(BCL_IAVAIL_HIGH_THRESHOLD_VALUE, highname);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}
	ret = bcl_get_node_name(BCL_IAVAIL_LOW_THRESHOLD_VALUE, lowname);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	/* Disable all thresholds for update */
	enable_bcl_threshold_high(bcl, 0);
	enable_bcl_threshold_low(bcl, 0);

	if (thresh->high_valid) {
		dbgmsg("Setting up BCL thresholds high trigger: %d\n", thresh->high);
		snprintf(buf, LVL_BUF_MAX, "%d", thresh->high);
		ret = write_to_file(highname, buf, strlen(buf));
		if (ret <= 0)
			msg("BCL threshold high trigger failed to set %d\n", thresh->high);
		enable_bcl_threshold_high(bcl, thresh->high_valid);
	}

	if (thresh->low_valid) {
		dbgmsg("Setting up BCL thresholds low trigger: %d\n", thresh->low);
		snprintf(buf, LVL_BUF_MAX, "%d", thresh->low);
		ret = write_to_file(lowname, buf, strlen(buf));
		if (ret <= 0)
			msg("BCL threshold low trigger failed to set %d\n", thresh->low);
		enable_bcl_threshold_low(bcl, thresh->low_valid);
	}
}

void bcl_enable_thresholds(struct sensor_info *sensor, int enabled)
{
	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("bcl: unexpected NULL");
		return;
	}

	enable_bcl_threshold_high(sensor->data, enabled);
	enable_bcl_threshold_low(sensor->data, enabled);
}

void bcl_enable(struct sensor_info *sensor, int enabled)
{
	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("bcl: unexpected NULL");
		return;
	}

	enable_bcl(sensor->data, enabled);
}

/* NOTE: bcl_setup() function does not enable the sensor
 * or set thresholds. This should be done in the target-specific setup */
int bcl_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	char name[MAX_PATH] = {0};
	struct bcl_data *bcl = NULL;
	char buf[LVL_BUF_MAX] = {0};
	int ret = 0;
	const char *dump_file = (dump_bcl_file) ? dump_bcl_file : BCL_DUMP_FILE_DEFAULT;
	int polling_interval = SENSOR_DEFAULT_POLLING_INTERVAL;
#if 0
	int vbat_min = 3400;
#endif
	/* Allocate BCL data */
	bcl = (struct bcl_data *) malloc(sizeof(struct bcl_data));
	if (NULL == bcl) {
		msg("%s: malloc failed", __func__);
		return sensor_count;
	}
	memset(bcl, 0, sizeof(struct bcl_data));
	sensor->data = (void *) bcl;

	if (dump_bcl) {
		bcl->dump_bcl_fd = fopen(dump_file, "w");
		if (bcl->dump_bcl_fd == NULL) {
			msg("Failed to open BCL dump file %s\n", dump_file);
			free(bcl);
			return sensor_count;
		}
		fprintf(bcl->dump_bcl_fd, "iavail(mA) soc\n");
		fflush(bcl->dump_bcl_fd);
	}

	sensor->tzn = 0;

	ret = bcl_get_node_name(BCL_IAVAIL, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		if (bcl->dump_bcl_fd != NULL)
			fclose(bcl->dump_bcl_fd);
		return sensor_count;
	}

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		if (bcl->dump_bcl_fd != NULL)
			fclose(bcl->dump_bcl_fd);
		return sensor_count;
	}
	bcl->iavail_idx = fd;

	strlcpy(name, BMS_SOC, MAX_PATH);
	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		close(bcl->iavail_idx);
		if (bcl->dump_bcl_fd != NULL)
			fclose(bcl->dump_bcl_fd);
		return sensor_count;
	}
	bcl->soc_idx = fd;

	sensor_count++;
	pthread_mutex_init(&(bcl->bcl_mutex), NULL);
	pthread_cond_init(&(bcl->bcl_condition), NULL);
	bcl->sensor_shutdown = 0;
	bcl->threshold_reached = 0;
	bcl->sensor = sensor;

	if (sensor->interrupt_enable) {
		pthread_create(&(bcl->bcl_thread), NULL,
			       bcl_uevent, sensor);
	}

	ret = bcl_get_node_name(BCL_POLL_INTERVAL, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		close(bcl->iavail_idx);
		close( bcl->soc_idx);
		if (bcl->dump_bcl_fd != NULL)
			fclose(bcl->dump_bcl_fd);
		return sensor_count;
	}

	snprintf(buf, LVL_BUF_MAX, "%d", polling_interval);
	ret = write_to_file(name, buf, strlen(buf));
	if (ret <= 0)
		msg("BCL poll interval failed to set %d\n", polling_interval);

#if 0
	ret = bcl_get_node_name(BCL_VBAT_MIN, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return sensor_count;
	}

	snprintf(buf, LVL_BUF_MAX, "%d", vbat_min);
	ret = write_to_file(name, buf, strlen(buf));
	if (ret <= 0)
		msg("BCL VBAT MIN failed to set %d\n", vbat_min);
#endif

	bcl_enable(sensor, 1);

	info("BCL sensors setup done.\n");

	return sensor_count;
}

void bcl_shutdown(struct sensor_info *sensor)
{
	struct bcl_data *bcl;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	bcl = (struct bcl_data *) sensor->data;
	bcl->sensor_shutdown = 1;

	if (bcl->iavail_idx > 0)
		close(bcl->iavail_idx);

	if (bcl->soc_idx > 0)
		close(bcl->soc_idx);

	if (bcl->dump_bcl_fd)
		fclose(bcl->dump_bcl_fd);

	bcl_enable(sensor, 0);

	if (sensor->interrupt_enable)
		pthread_join(bcl->bcl_thread, NULL);
	free(bcl);
}

int bcl_get_iavail(struct sensor_info *sensor)
{
	char buf[16];
	int temp = 0;
	struct bcl_data *bcl;

	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}

	memset(buf, 0, sizeof(buf));

	bcl = (struct bcl_data *) sensor->data;
	if (read(bcl->iavail_idx, buf, sizeof(buf) - 1) != -1)
		temp = atoi(buf);
	lseek(bcl->iavail_idx, 0, SEEK_SET);

	return (temp);
}

int bcl_get_soc(struct sensor_info *sensor)
{
	char buf[64];
	int temp = 0;
	struct bcl_data *bcl;

	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}

	memset(buf, 0, sizeof(buf));

	bcl = (struct bcl_data *) sensor->data;
	if (read(bcl->soc_idx, buf, sizeof(buf) - 1) != -1)
		temp = atoi(buf);
	lseek(bcl->soc_idx, 0, SEEK_SET);

	return (temp);
}

void bcl_interrupt_wait(struct sensor_info *sensor)
{
	struct bcl_data *bcl;
	struct sensors_mgr_sensor_info *sensor_mgr;
	uint32_t polling_interval;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	sensor_mgr = sensor->sensor_mgr;
	polling_interval =
		(sensor_mgr->active_thresh.polling_interval_valid)?
		(sensor_mgr->active_thresh.polling_interval):
		(SENSOR_DEFAULT_POLLING_INTERVAL);
	bcl = (struct bcl_data *) sensor->data;

	if (dump_bcl == 1) {
		usleep(polling_interval*1000);
		/* Read all BCL related params for dumping to a file */
		bcl->iavail = bcl_get_iavail(sensor);

		if (bcl->dump_bcl_fd) {
			fprintf(bcl->dump_bcl_fd, "%d %d\n", bcl->iavail,
				bcl_get_soc(sensor));
			fflush(bcl->dump_bcl_fd);
		}
		return;
	}

	if (sensor->interrupt_enable) {
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(bcl->bcl_mutex));
		while (!bcl->threshold_reached) {
			pthread_cond_wait(&(bcl->bcl_condition),
					&(bcl->bcl_mutex));
		}
		bcl->threshold_reached = 0;
		pthread_mutex_unlock(&(bcl->bcl_mutex));
		/* Read all BCL related params after event is fired */
		bcl->iavail = bcl_get_iavail(sensor);
	}
}

void bcl_update_thresholds(struct sensor_info *sensor, struct thresholds_req_t *thresh)
{
	struct bcl_data *bcl;
	uint32_t polling_interval;
	char buf[LVL_BUF_MAX] = {0};
	int ret = 0;
	struct sensors_mgr_sensor_info *sensor_mgr;
	char name[MAX_PATH] = {0};

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	bcl = (struct bcl_data *) sensor->data;
	sensor_mgr = sensor->sensor_mgr;
	polling_interval =
		(sensor_mgr->active_thresh.polling_interval_valid)?
		(sensor_mgr->active_thresh.polling_interval):
		(SENSOR_DEFAULT_POLLING_INTERVAL);

	ret = bcl_get_node_name(BCL_POLL_INTERVAL, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	snprintf(buf, LVL_BUF_MAX, "%d", polling_interval);
	ret = write_to_file(name, buf, strlen(buf));
	if (ret <= 0)
		msg("BCL poll interval failed to set %d\n", polling_interval);

	set_bcl_thresholds(bcl, thresh);
}

