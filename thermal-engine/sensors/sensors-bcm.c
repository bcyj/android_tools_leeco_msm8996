/*===========================================================================

  sensors-bcm.c

  DESCRIPTION
  Buck current monitor sensor access functions.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  bcm_setup() function should be called before bcm_get_current().
  bcm_shutdown() function should be called to clean up resources.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <dirent.h>

#include "sensors-bcm.h"
#include "thermal.h"

#define LVL_BUF_MAX (12)
#define MAX_CFG 4

/* bcm sysfs defines */
#define BCM_ENABLE        "enable"
#define BCM_CURR_I_CRIT   "curr1_crit"
#define BCM_CURR_I_WARN   "curr1_warn"
#define BCM_I_CRIT_ALARM  "curr1_crit_alarm"
#define BCM_I_WARN_ALARM  "curr1_warn_alarm"
#define PLATFORM_DEVICE "/sys/class/hwmon/hwmon0/device/"
#define OF_NODE "buck-current-monitor"
#ifdef _PLATFORM_BASE
#define SYS_DEV_DIR  "/sys"_PLATFORM_BASE
#else
#define SYS_DEV_DIR "/sys/devices/"
#endif

#define ROUND_DOWN(x)  (x - (x % 10))

/* Supported thresholds for each interrupt in % of rated current */
static const int icrit_threshold_map[MAX_CFG] = {60, 70, 80, 90};
static const int iwarn_threshold_map[MAX_CFG] = {40, 50, 60, 70};

enum interrupts {
	IWARN = 0,
	ICRIT,
	MAX_INTERRUPT,
};

enum interrupt_mode {
	MODE_DISABLED = 0,
	MODE_ENABLED,
};

enum interrupt_type {
	INTERRUPT_CLR = 0,
	INTERRUPT_TRIG,
	INTERRUPT_TYPE_MAX,
};

struct bcm_data {
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	enum interrupts notified_type;
	enum interrupt_mode iwarn[INTERRUPT_TYPE_MAX];
	enum interrupt_mode icrit[INTERRUPT_TYPE_MAX];
        int current_low_threshold[MAX_INTERRUPT];
	int threshold_reached;
	int sensor_shutdown;
	struct sensor_info *sensor;
	int alarm_fds[MAX_INTERRUPT];
	int set_fds[MAX_INTERRUPT];
};

static int get_bcm_dir_name(char *name)
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

static int bcm_get_node_name(const char *node, char *path)
{
	static uint8_t init_done;
	static char qualifier[MAX_PATH] = {0};
	int ret = 0;

	if (!init_done) {
		char temp[MAX_PATH] = {0};

		ret = get_bcm_dir_name(temp);
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

static void update_next_interrupt(struct bcm_data *bcm, enum interrupts last_intr,
			      enum interrupt_type last_type)
{
	if (bcm == NULL ||
	    last_intr >= MAX_INTERRUPT ||
	    last_type >= INTERRUPT_TYPE_MAX) {
		msg("%s: Unexpected error\n", __func__);
		return;
	}

	if (last_intr == ICRIT) {
		if (last_type == INTERRUPT_TRIG) {
			bcm->icrit[INTERRUPT_TRIG] = MODE_DISABLED;
			bcm->icrit[INTERRUPT_CLR] = MODE_ENABLED;
			bcm->iwarn[INTERRUPT_CLR] = MODE_DISABLED;
		} else {
			bcm->icrit[INTERRUPT_TRIG] = MODE_ENABLED;
			bcm->icrit[INTERRUPT_CLR] = MODE_DISABLED;
			bcm->iwarn[INTERRUPT_CLR] = MODE_ENABLED;
		}
	} else {
		if (last_type == INTERRUPT_TRIG) {
			bcm->icrit[INTERRUPT_TRIG] = MODE_ENABLED;
			bcm->iwarn[INTERRUPT_TRIG] = MODE_DISABLED;
			bcm->iwarn[INTERRUPT_CLR] = MODE_ENABLED;
		} else {
			bcm->iwarn[INTERRUPT_TRIG] = MODE_ENABLED;
			bcm->icrit[INTERRUPT_TRIG] = MODE_DISABLED;
			bcm->iwarn[INTERRUPT_CLR] = MODE_DISABLED;
		}
	}
}

static void *bcm_uevent(void *data)
{
	int err = 0;
	struct sensor_info *sensor = (struct sensor_info *)data;
	struct pollfd fds[2];
	char buf[MAX_PATH] = {0};
	struct bcm_data *bcm = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return NULL;
	}
	bcm = (struct bcm_data *) sensor->data;
	/* Looking for bcm uevent */
	while (!bcm->sensor_shutdown) {
		fds[0].fd = bcm->alarm_fds[ICRIT];
		fds[0].events = POLLPRI;
		fds[0].revents = 0;
		fds[1].fd = bcm->alarm_fds[IWARN];
		fds[1].events = POLLPRI;
		fds[1].revents = 0;

		err = poll(fds, 2, -1);
		if (err == -1) {
			msg("Error in uevent poll.\n");
			break;
		}

		if (fds[0].revents & POLLPRI) {
			bcm->notified_type = ICRIT;
		} else if (fds[1].revents & POLLPRI) {
			bcm->notified_type = IWARN;
		} else {
			msg("%s::Unexpected error\n", __func__);
			continue;
		}

		read(bcm->alarm_fds[bcm->notified_type], buf, sizeof(buf));
		lseek(bcm->alarm_fds[bcm->notified_type], 0, SEEK_SET);

		dbgmsg("BCM uevent:%s: %s",
		        bcm->notified_type ? "icrit" : "iwarn", buf);
		/* notify the waiting threads */
		pthread_mutex_lock(&(bcm->mutex));
		bcm->threshold_reached = 1;
		pthread_cond_broadcast(&(bcm->condition));
		pthread_mutex_unlock(&(bcm->mutex));
	}

	return NULL;
}

/* Check for initial status of interrupt and configured
   thresholds for each interrupt return -1 for error, 0 */
static int update_initial_interrupt_status(struct bcm_data *bcm)
{
	char buf[LVL_BUF_MAX] = {0};
	enum interrupt_type iwarn_status;
	int ret = 0;

	if (read(bcm->alarm_fds[IWARN], buf, sizeof(buf) - 1) < 0) {
		msg("%s: Unexpected read error\n", __func__);
		ret = -EINVAL;
		goto ERR_HANDLE;
	}
	iwarn_status = atoi(buf) ? INTERRUPT_TRIG: INTERRUPT_CLR;
	lseek(bcm->alarm_fds[IWARN], 0, SEEK_SET);

	if (iwarn_status) {
		enum interrupt_type icrit_status;
		buf[0] = '\0';
		if (read(bcm->alarm_fds[ICRIT], buf, sizeof(buf)) < 0) {
			msg("%s: Unexpected read error\n", __func__);
			ret = -EINVAL;
			goto ERR_HANDLE;
		}
		icrit_status = atoi(buf) ? INTERRUPT_TRIG: INTERRUPT_CLR;
		lseek(bcm->alarm_fds[ICRIT], 0, SEEK_SET);

		if (icrit_status)
			update_next_interrupt(bcm, ICRIT, INTERRUPT_TRIG);
		else
			update_next_interrupt(bcm, IWARN, INTERRUPT_TRIG);

	} else {
		/* No trigger happened yet */
		update_next_interrupt(bcm, IWARN, INTERRUPT_CLR);
	}
ERR_HANDLE:
	return ret;
}

static void enable_bcm(struct bcm_data *bcm, int enabled)
{
	int ret = 0;
	char buf[LVL_BUF_MAX] = {0};
	char node[LVL_BUF_MAX] = {0};
	char name[MAX_PATH] = {0};
	int  threshold = 0;

	if (NULL == bcm ||
	    NULL == bcm->sensor) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	if (enabled) {
		/* Check and update initial status of interrupt */
		ret = update_initial_interrupt_status(bcm);
		if (ret < 0) {
			msg("%s: Unexpected error: enable bcm failed\n", __func__);
			return;
		}
	} else {
		goto ENABLE_BCM;
	}

	/* Before enabling BCM, set initial thresholds(max value) for each
	   interrupt if there is no threshold set already */
	strlcpy(node, BCM_CURR_I_WARN, LVL_BUF_MAX);
	ret = bcm_get_node_name(node, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	if (read_line_from_file(name, buf, sizeof(buf)) <= 0) {
		msg("%s: Unexpected read error\n", __func__);
		return;
	}
	threshold = atoi(buf);
	if (!threshold) {
		threshold = iwarn_threshold_map[MAX_CFG - 1];
		snprintf(buf, LVL_BUF_MAX, "%d", threshold);
		ret = write_to_file(name, buf, strlen(buf));
		if (ret <= 0) {
			msg("bcm initial Iwarn thresholds failed to set\n");
			return;
		}
	}

	bcm->current_low_threshold[ICRIT] = CONV(threshold);
	bcm->current_low_threshold[IWARN] = 0;

	strlcpy(node, BCM_CURR_I_CRIT, LVL_BUF_MAX);
	ret = bcm_get_node_name(node, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	if (read_line_from_file(name, buf, sizeof(buf)) <= 0) {
		msg("%s: Unexpected read error\n", __func__);
		return;
	}
	threshold = atoi(buf);
	if (!threshold) {
		threshold = icrit_threshold_map[MAX_CFG - 1];
		snprintf(buf, LVL_BUF_MAX, "%d", threshold);
		ret = write_to_file(name, buf, strlen(buf));
		if (ret <= 0) {
			msg("bcm initial Icrit thresholds failed to set\n");
			return;
		}
	}

ENABLE_BCM:
	ret = bcm_get_node_name(BCM_ENABLE, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return;
	}

	if (enabled)
		ret = write_to_file(name, "1", strlen("1"));
	else
		ret = write_to_file(name, "0", strlen("0"));

	if (ret <= 0) {
		msg("bcm failed to set mode %d\n", enabled);
	} else {
		dbgmsg("bcm mode set to %d\n", enabled);
	}
}

static void set_bcm_thresholds(struct bcm_data *bcm, struct thresholds_req_t *thresh)
{
	char highname[MAX_PATH]= {0};
	char buf[LVL_BUF_MAX] = {0};
	char node[LVL_BUF_MAX] = {0};
	int ret = 0;

	if (NULL == bcm) {
		msg("%s: Unexpected NULL", __func__);
		return;
	}

	if (thresh->high_valid) {
		int active_threshold = 0;

		active_threshold = ROUND_DOWN(RCONV(thresh->high));

		if (bcm->icrit[INTERRUPT_TRIG] == MODE_ENABLED) {
			if (active_threshold < icrit_threshold_map[0] ||
			    active_threshold > icrit_threshold_map[MAX_CFG - 1]) {
				msg("%s: Invalid threshold for level 2\n", __func__);
				return;
			}
			strlcpy(node, BCM_CURR_I_CRIT, LVL_BUF_MAX);
		} else if (bcm->iwarn[INTERRUPT_TRIG] == MODE_ENABLED) {
			if (active_threshold < iwarn_threshold_map[0] ||
			    active_threshold > iwarn_threshold_map[MAX_CFG - 1]) {
				msg("%s: Invalid threshold for level 1\n", __func__);
				return;
			}
			strlcpy(node, BCM_CURR_I_WARN, LVL_BUF_MAX);
		} else {
			msg("Only 2 level threshold settings are supported\n");
			return;
		}

		ret = bcm_get_node_name(node, highname);
		if (ret) {
			msg("%s: Unexpected node error", __func__);
			return;
		}
		dbgmsg("Setting up bcm thresholds high trigger with rounddown value: %d\n",
		       active_threshold);
		snprintf(buf, LVL_BUF_MAX, "%d", active_threshold);
		ret = write_to_file(highname, buf, strlen(buf));
		if (ret <= 0) {
			msg("bcm threshold trigger failed to set %d\n", thresh->high);
			return;
		}
	}

	if (thresh->low_valid) {
		dbgmsg("Setting up bcm thresholds low trigger: %d\n", thresh->low);
		if (bcm->icrit[INTERRUPT_CLR] == MODE_ENABLED)
			bcm->current_low_threshold[ICRIT] = thresh->low;
		else if (bcm->iwarn[INTERRUPT_CLR] == MODE_ENABLED)
			bcm->current_low_threshold[IWARN] = thresh->low;
	}
}

void bcm_enable(struct sensor_info *sensor, int enabled)
{
	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("bcm: unexpected NULL");
		return;
	}

	enable_bcm(sensor->data, enabled);
}

/* NOTE: bcm_setup() function does not enable the sensor
 * or set thresholds. This should be done in the target-specific setup */
int bcm_setup(struct sensor_info *sensor)
{
	int fd = -1;
	int sensor_count = 0;
	char name[MAX_PATH] = {0};
	struct bcm_data *bcm = NULL;
	int ret = 0;

	/* Allocate bcml data */
	bcm = (struct bcm_data *) malloc(sizeof(struct bcm_data));
	if (NULL == bcm) {
		msg("%s: malloc failed", __func__);
		return sensor_count;
	}
	memset(bcm, 0, sizeof(struct bcm_data));
	sensor->data = (void *)bcm;

	sensor->tzn = 0;

	ret = bcm_get_node_name(BCM_I_WARN_ALARM, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		return sensor_count;
	}

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		return sensor_count;
	}
	bcm->alarm_fds[IWARN] = fd;

	ret = bcm_get_node_name(BCM_I_CRIT_ALARM, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		close(bcm->alarm_fds[IWARN]);
		return sensor_count;
	}

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		close(bcm->alarm_fds[IWARN]);
		return sensor_count;
	}
	bcm->alarm_fds[ICRIT] = fd;

	ret = bcm_get_node_name(BCM_CURR_I_WARN, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		close(bcm->alarm_fds[IWARN]);
		close(bcm->alarm_fds[ICRIT]);
		return sensor_count;
	}

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		close(bcm->alarm_fds[IWARN]);
		close(bcm->alarm_fds[ICRIT]);
		return sensor_count;
	}
	bcm->set_fds[IWARN] = fd;

	ret = bcm_get_node_name(BCM_CURR_I_CRIT, name);
	if (ret) {
		msg("%s: Unexpected node error", __func__);
		close(bcm->alarm_fds[IWARN]);
		close(bcm->alarm_fds[ICRIT]);
		close(bcm->set_fds[IWARN]);
		return sensor_count;
	}

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		msg("%s: Error opening %s\n", __func__, name);
		close(bcm->alarm_fds[IWARN]);
		close(bcm->alarm_fds[ICRIT]);
		close(bcm->set_fds[IWARN]);
		return sensor_count;
	}
	bcm->set_fds[ICRIT] = fd;

	sensor_count++;
	pthread_mutex_init(&(bcm->mutex), NULL);
	pthread_cond_init(&(bcm->condition), NULL);
	bcm->sensor_shutdown = 0;
	bcm->threshold_reached = 0;
	bcm->sensor = sensor;

	if (sensor->interrupt_enable) {
		pthread_create(&(bcm->thread), NULL,
			       bcm_uevent, sensor);
	}

	bcm_enable(sensor, 1);

	info("bcm sensors setup done.\n");

	return sensor_count;
}

int bcm_get_current(struct sensor_info *sensor)
{
	char buf[16];
	enum interrupt_type iwarn_status = 0;
	enum interrupt_type icrit_status = 0;
	int curr = 0;
	struct bcm_data *bcm;

	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}

	bcm = (struct bcm_data *) sensor->data;

	memset(buf, 0, sizeof(buf));
	if (read(bcm->alarm_fds[IWARN], buf,
			sizeof(buf) - 1) != -1)
		iwarn_status = atoi(buf);
	lseek(bcm->alarm_fds[IWARN], 0, SEEK_SET);

	memset(buf, 0, sizeof(buf));
	if (read(bcm->alarm_fds[ICRIT], buf,
			sizeof(buf) - 1) != -1)
		icrit_status = atoi(buf);
	lseek(bcm->alarm_fds[ICRIT], 0, SEEK_SET);

	if (iwarn_status) {
		if (icrit_status) {
			memset(buf, 0, sizeof(buf));
			if (read(bcm->set_fds[ICRIT], buf,
					sizeof(buf) - 1) != -1)
				curr = CONV(atoi(buf));
			lseek(bcm->set_fds[ICRIT], 0, SEEK_SET);
		} else {
			if (bcm->notified_type == ICRIT) {
				curr = bcm->current_low_threshold[ICRIT];
			} else {
				memset(buf, 0, sizeof(buf));
				if (read(bcm->set_fds[IWARN], buf,
						sizeof(buf) - 1) != -1)
					curr = CONV(atoi(buf));
				lseek(bcm->set_fds[IWARN], 0, SEEK_SET);
			}
		}
		update_next_interrupt(bcm, ICRIT, icrit_status);
	} else {
		curr = bcm->current_low_threshold[IWARN];
		update_next_interrupt(bcm, IWARN, iwarn_status);
	}

	return curr;
}

void bcm_interrupt_wait(struct sensor_info *sensor)
{
	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	struct bcm_data *bcm = (struct bcm_data *) sensor->data;

	if (sensor->interrupt_enable) {
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(bcm->mutex));
		while (!bcm->threshold_reached) {
			pthread_cond_wait(&(bcm->condition),
					&(bcm->mutex));
		}
		bcm->threshold_reached = 0;
		pthread_mutex_unlock(&(bcm->mutex));
	}
}

void bcm_update_thresholds(struct sensor_info *sensor, struct thresholds_req_t *thresh)
{
	if (NULL == sensor ||
	    NULL == sensor->data ||
	    thresh == NULL) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	struct bcm_data *bcm = (struct bcm_data *) sensor->data;
	set_bcm_thresholds(bcm, thresh);
}

void bcm_shutdown(struct sensor_info *sensor)
{
	uint8_t i = 0;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	struct bcm_data *bcm = (struct bcm_data *) sensor->data;
	bcm->sensor_shutdown = 1;

	bcm_enable(sensor, 0);

	for (i = 0; i < MAX_INTERRUPT; i++) {
		if (bcm->alarm_fds[i] > 0)
			close(bcm->alarm_fds[i]);

		if (bcm->set_fds[i] > 0)
			close(bcm->set_fds[i]);
	}

	if (sensor->interrupt_enable)
		pthread_join(bcm->thread, NULL);
	free(bcm);
}
