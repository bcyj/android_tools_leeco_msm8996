/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string.h>
#include <dirent.h>
#include "thermal.h"
#if defined (ENABLE_MODEM_MITIGATION) || defined(ENABLE_MODEM_TS)
#  include "qmi_client_instance_defs.h"
#endif
#ifdef ANDROID
#  include "cutils/properties.h"
#endif

#define THERMAL_SYSFS "/sys/devices/virtual/thermal"

/* Sys proc ID */
#define SYSFS_PLATFORMID_DEPRECATED   "/sys/devices/system/soc/soc0/id"
#define SYSFS_HW_PLATFORM_DEPRECATED  "/sys/devices/system/soc/soc0/hw_platform"
#define SYSFS_MSM_VERSION_DEPRECATED  "/sys/devices/system/soc/soc0/version"
#define SYSFS_HW_SUBTYPE_DEPRECATED   "/sys/devices/system/soc/soc0/platform_subtype_id"
#define SYSFS_PMIC_MODEL_DEPRECATED   "/sys/devices/system/soc/soc0/pmic_model"
#define SYSFS_PLATFORMID   "/sys/devices/soc0/soc_id"
#define SYSFS_HW_PLATFORM  "/sys/devices/soc0/hw_platform"
#define SYSFS_MSM_VERSION  "/sys/devices/soc0/revision"
#define SYSFS_HW_SUBTYPE   "/sys/devices/soc0/platform_subtype_id"
#define SYSFS_PMIC_MODEL  "/sys/devices/soc0/pmic_model"

#define MAX_INT_ARRAY_STR_BUF  768
#define MAX_SOC_INFO_NAME_LEN (15)
#define MAX_CONFIG_PATH  (50)
#ifdef ANDROID
#define DEFAULT_CONFIG_PATH  "/system/etc/"
#else
#define DEFAULT_CONFIG_PATH  "/etc/"
#endif

struct therm_msm_soc_type {
	enum therm_msm_id msm_id;
	int  soc_id;
};

struct therm_hw_platform_type {
	enum therm_hw_platform platform;
	char *platform_name;
};

struct therm_msm_version_type {
	enum therm_msm_version version;
	int version_id;
};

struct target_thermal_config {
	enum therm_msm_id msm_id;
	char *config_file;
};

struct therm_pmic_type {
	enum therm_pmic_model pmic;
	int pmic_id;
};

static struct therm_msm_soc_type msm_soc_table[] = {
	{THERM_MSM_8960,   87},
	{THERM_MSM_8960,   122},
	{THERM_MSM_8960,   123},
	{THERM_MSM_8960,   124},
	{THERM_MSM_8960AB, 138},
	{THERM_MSM_8960AB, 139},
	{THERM_MSM_8960AB, 140},
	{THERM_MSM_8960AB, 141},
	{THERM_MSM_8930,   116},
	{THERM_MSM_8930,   117},
	{THERM_MSM_8930,   118},
	{THERM_MSM_8930,   119},
	{THERM_MSM_8064,   109},
	{THERM_MSM_8064,   130},
	{THERM_MSM_8064AB, 153},
	{THERM_MSM_8974,   126},
	{THERM_MSM_8974,   184},
	{THERM_MSM_8974,   185},
	{THERM_MSM_8974,   186},
	{THERM_MSM_8974PRO_AA, 208},
	{THERM_MSM_8974PRO_AA, 211},
	{THERM_MSM_8974PRO_AA, 214},
	{THERM_MSM_8974PRO_AA, 217},
	{THERM_MSM_8974PRO_AB, 209},
	{THERM_MSM_8974PRO_AB, 212},
	{THERM_MSM_8974PRO_AB, 215},
	{THERM_MSM_8974PRO_AB, 218},
	{THERM_MSM_8974PRO_AC, 194},
	{THERM_MSM_8974PRO_AC, 210},
	{THERM_MSM_8974PRO_AC, 213},
	{THERM_MSM_8974PRO_AC, 216},
	{THERM_MSM_8226,   145},
	{THERM_MSM_8226,   158},
	{THERM_MSM_8226,   159},
	{THERM_MSM_8226,   198},
	{THERM_MSM_8226,   199},
	{THERM_MSM_8926,   200},
	{THERM_MSM_8226,   205},
	{THERM_MSM_8226,   219},
	{THERM_MSM_8226,   220},
	{THERM_MSM_8226,   221},
	{THERM_MSM_8226,   222},
	{THERM_MSM_8226,   223},
	{THERM_MSM_8926,   224},
	{THERM_MSM_8610,   147},
	{THERM_MDM_9625,   134},
	{THERM_MDM_9625,   148},
	{THERM_MDM_9625,   149},
	{THERM_MDM_9625,   150},
	{THERM_MDM_9625,   151},
	{THERM_MDM_9625,   152},
	{THERM_MDM_9625,   173},
	{THERM_MDM_9625,   174},
	{THERM_MDM_9625,   175},
	{THERM_MSM_8610,   161},
	{THERM_MSM_8610,   162},
	{THERM_MSM_8610,   163},
	{THERM_MSM_8610,   164},
	{THERM_MSM_8610,   165},
	{THERM_MSM_8610,   166},
	{THERM_MSM_8084,   178},
	{THERM_MSM_8x62,   195},
	{THERM_MSM_8x62,   196},
	{THERM_MSM_8x62,   197},
	{THERM_MDM_9635,   187},
	{THERM_MPQ_8092,   146},
	{THERM_MSM_8916,   206},
	{THERM_MSM_8916,   247},
	{THERM_MSM_8916,   248},
	{THERM_MSM_8916,   249},
	{THERM_MSM_8916,   250},
	{THERM_FSM_9900,   188},
	{THERM_FSM_9900,   189},
	{THERM_FSM_9900,   190},
	{THERM_FSM_9900,   191},
	{THERM_FSM_9900,   192},
	{THERM_FSM_9900,   193},
	{THERM_MSM_8939,   239},
	{THERM_MSM_8939,   241},
	{THERM_MSM_8939,   263},
	{THERM_MSM_8929,   268},
	{THERM_MSM_8929,   269},
	{THERM_MSM_8929,   270},
	{THERM_MSM_8929,   271},
	{THERM_MSM_8994,   207},
	{THERM_MSM_8994,   253},
	{THERM_MSM_8936,   233},
	{THERM_MSM_8936,   240},
	{THERM_MSM_8936,   242},
	{THERM_MSM_8909,   245},
	{THERM_MSM_8909,   258},
	{THERM_MSM_8909,   259},
	{THERM_MSM_8909,   260},
	{THERM_MSM_8909,   261},
	{THERM_MSM_8909,   262},
	{THERM_MSM_8909,   265},
};

static struct therm_hw_platform_type platform_table[] = {
	{THERM_PLATFORM_UNKNOWN,    "Unknown"},
	{THERM_PLATFORM_SURF,       "Surf"},
	{THERM_PLATFORM_FFA,        "FFA"},
	{THERM_PLATFORM_FLUID,      "Fluid"},
	{THERM_PLATFORM_SVLTE_FFA,  "SVLTE_FFA"},
	{THERM_PLATFORM_SVLTE_SURF, "SLVTE_SURF"},
	{THERM_PLATFORM_MTP,        "MTP"},
	{THERM_PLATFORM_LIQUID,     "Liquid"},
	{THERM_PLATFORM_DRAGON,     "Dragon"},
	{THERM_PLATFORM_QRD,        "QRD"},
};

static struct therm_msm_version_type version_table[] = {
	{THERM_VERSION_V1,  1},
	{THERM_VERSION_V2,  2},
};

static struct target_thermal_config thermal_config_table[] = {
	{THERM_MSM_8960,   "thermal-engine-8960.conf"},
	{THERM_MSM_8960AB, "thermal-engine-8960.conf"},
	{THERM_MSM_8930,   "thermal-engine-8930.conf"},
	{THERM_MSM_8064,   "thermal-engine-8064.conf"},
	{THERM_MSM_8064AB, "thermal-engine-8064ab.conf"},
	{THERM_MSM_8974,   "thermal-engine-8974.conf"},
	{THERM_MSM_8974PRO_AA, "thermal-engine-8974.conf"},
	{THERM_MSM_8974PRO_AB, "thermal-engine-8974.conf"},
	{THERM_MSM_8974PRO_AC, "thermal-engine-8974.conf"},
	{THERM_MSM_8226,   "thermal-engine-8226.conf"},
	{THERM_MSM_8926,   "thermal-engine-8226.conf"},
	{THERM_MSM_8610,   "thermal-engine-8610.conf"},
	{THERM_MDM_9625,   NULL},
	{THERM_MSM_8084,   NULL},
	{THERM_MPQ_8092,   NULL},
	{THERM_MSM_8x62,   "thermal-engine-8974.conf"},
	{THERM_MDM_9635,   NULL},
	{THERM_MSM_8916,   NULL},
	{THERM_MSM_8939,   NULL},
	{THERM_MSM_8994,   NULL},
	{THERM_MSM_8936,   NULL},
	{THERM_MSM_8909,   NULL},
	{THERM_MSM_8929,   NULL},
};

static struct therm_pmic_type pmic_type_table[] = {
	{THERM_PMIC_PM8909, 65549},
	{THERM_PMIC_PM8916, 65547},
};

static int get_soc_info(char *buf, char *soc_node, char *soc_node_dep)
{
	int ret = 0;

	ret = read_line_from_file(soc_node, buf, MAX_SOC_INFO_NAME_LEN);
	if (ret < 0) {
		ret = read_line_from_file(soc_node_dep, buf,
					  MAX_SOC_INFO_NAME_LEN);
		if (ret < 0) {
			msg("Error getting platform_id %d", ret);
			return ret;
		}
	}
	if (ret && buf[ret - 1] == '\n')
		buf[ret - 1] = '\0';

	return ret;
}

enum therm_msm_id therm_get_msm_id(void)
{
	static enum therm_msm_id msm_id;
	static uint8_t msm_id_init;

	if (!msm_id_init) {
		int idx;
		char buf[MAX_SOC_INFO_NAME_LEN];

		if (soc_id < 0) {
			get_soc_info(buf, SYSFS_PLATFORMID, SYSFS_PLATFORMID_DEPRECATED);
			soc_id = atoi(buf);
		}

		for (idx = 0; idx < ARRAY_SIZE(msm_soc_table); idx++) {
			if (soc_id == msm_soc_table[idx].soc_id) {
				msm_id = msm_soc_table[idx].msm_id;
				break;
			}
		}
		if (!msm_id)
			msg("Unknown target identified with soc id %d\n", soc_id);

		msm_id_init = 1;
	}
	return msm_id;
}

enum therm_hw_platform therm_get_hw_platform(void)
{
	static enum therm_hw_platform platform;
	static uint8_t hw_platform_init;

	if (!hw_platform_init) {
		int idx;
		char buf[MAX_SOC_INFO_NAME_LEN];

		get_soc_info(buf, SYSFS_HW_PLATFORM,
				   SYSFS_HW_PLATFORM_DEPRECATED);

		for (idx = 0; idx < ARRAY_SIZE(platform_table); idx++) {
			if (strncmp(platform_table[idx].platform_name, buf,
			    MAX_SOC_INFO_NAME_LEN) == 0) {
				platform = platform_table[idx].platform;
				break;
			}
		}
		hw_platform_init = 1;
	}
	return platform;
}

enum therm_msm_version therm_get_msm_version(void)
{
	static enum therm_msm_version version;
	static uint8_t msm_version_init;

	if (!msm_version_init) {
		int idx;
		int version_id;
		char buf[MAX_SOC_INFO_NAME_LEN];

		get_soc_info(buf, SYSFS_MSM_VERSION,
				   SYSFS_MSM_VERSION_DEPRECATED);

		version_id = atoi(buf);

		for (idx = 0; idx < ARRAY_SIZE(version_table); idx++) {
			if (version_id == version_table[idx].version_id) {
				version = version_table[idx].version;
				break;
			}
		}
		msm_version_init = 1;
	}
	return version;
}

enum therm_hw_platform_subtype therm_get_hw_platform_subtype(void)
{
	static enum therm_hw_platform_subtype subtype =
					THERM_PLATFORM_SUB_UNKNOWN;
	static uint8_t subtype_id_init;

	if (!subtype_id_init) {
		int subtype_id = 0;
		char buf[MAX_SOC_INFO_NAME_LEN];

		get_soc_info(buf, SYSFS_HW_SUBTYPE, SYSFS_HW_SUBTYPE_DEPRECATED);
		subtype_id = atoi(buf);
		if (subtype_id > THERM_PLATFORM_SUB_UNKNOWN)
			subtype = subtype_id;

		subtype_id_init = 1;
	}
	return subtype;
}

char *get_target_default_thermal_config_file(void)
{
	static char config_file_path[MAX_CONFIG_PATH] = {0};
	static uint8_t config_init;

	if (!config_init) {
		int idx;
		char *config = NULL;
		enum therm_msm_id msm_id = therm_get_msm_id();

		for (idx = 0; idx < ARRAY_SIZE(thermal_config_table); idx++) {
			if (msm_id == thermal_config_table[idx].msm_id) {
				config = thermal_config_table[idx].config_file;
				break;
			}
		}
		if (config) {
			snprintf(config_file_path, MAX_CONFIG_PATH, "%s%s",
			         DEFAULT_CONFIG_PATH, config);
		}
		config_init = 1;
	}
	if (config_file_path[0] == 0)
		return NULL;
	else
		return config_file_path;
}

enum therm_pmic_model therm_get_pmic_model(void)
{
	static enum therm_pmic_model pmic = THERM_PMIC_UNKNOWN;
	static uint8_t pmic_id_init;

	if (!pmic_id_init) {
		int idx;
		int pmic_id = 0;
		char buf[MAX_SOC_INFO_NAME_LEN] = {0};

		get_soc_info(buf, SYSFS_PMIC_MODEL,
				SYSFS_PMIC_MODEL_DEPRECATED);
		pmic_id = atoi(buf);

		for (idx = 0; idx < ARRAY_SIZE(pmic_type_table); idx++) {
			if (pmic_id == pmic_type_table[idx].pmic_id) {
				pmic = pmic_type_table[idx].pmic;
				break;
			}
		}
		if (!pmic)
			msg("Unknown pmic identified with pmic id %d\n",
			    pmic_id);

		pmic_id_init = 1;
	}

	return pmic;
}

unsigned int read_int_list_from_file(const char *path, int *arr, uint32_t arr_sz)
{
	char buf[MAX_INT_ARRAY_STR_BUF];
	int bytes_read = 0;
	uint32_t ints_read = 0;
	char *int_str = NULL;
	char *saveptr;

	if (path == NULL || arr == NULL || arr_sz == 0) {
		msg("%s: Bad arg.", __func__);
		return 0;
	}

	/* Read integer list from file. */
	bytes_read = read_line_from_file(path, buf, sizeof(buf));

	if (bytes_read <= 0) {
		msg("%s: Read error %d.", __func__, bytes_read);
		return 0;
	}

	/* Parse out individual values into array */
	int_str = strtok_r(buf, " \n\r\t", &saveptr);
	while ((int_str != NULL) && ((uint32_t)ints_read < arr_sz)) {
		arr[ints_read] = atoi(int_str);
		ints_read++;
		int_str = strtok_r(NULL, " \n\r\t", &saveptr);
	}

	return ints_read;
}

static int compare_ascending_int(const void *x, const void *y)
{
	int i = *((int*)x);
	int j = *((int*)y);

	if (i < j)
		return -1;
	else if (i > j)
		return 1;
	return 0;
}

static int compare_descending_int(const void *x, const void *y)
{
	int i = *((int*)x);
	int j = *((int*)y);

	if (j < i)
		return -1;
	else if (j > i)
		return 1;
	return 0;
}

void sort_int_arr(int *arr, uint32_t arr_sz, uint8_t ascending)
{
	if (arr == NULL || arr_sz == 0) {
		msg("%s: Bad arg.", __func__);
		return;
	}

	if (ascending)
		qsort(arr, arr_sz, sizeof(int), compare_ascending_int);
	else
		qsort(arr, arr_sz, sizeof(int), compare_descending_int);
}


/*===========================================================================
FUNCTION open_file

Utility function to open file.

ARGUMENTS
	path - pathname for file
	flags - file open flags

RETURN VALUE
	file descriptor on success,
	-1 on failure.
===========================================================================*/
static int open_file(const char *path, int flags)
{
	int rv;

	if (!path) return -EINVAL;

	rv = open(path, flags);
	if (rv < 0)
		rv = -errno;

	return rv;
}

/*===========================================================================
FUNCTION write_to_fd

Utility function to write to provided file descriptor.

ARGUMENTS
	fd - file descriptor
	buf - destination buffer to write to
	count - number of bytes to write to fd

RETURN VALUE
	number of bytes written on success, -errno on failure.
===========================================================================*/
static int write_to_fd(int fd, const char *buf, size_t count)
{
	ssize_t pos = 0;
	ssize_t rv = 0;
	/* When thermal-engine is in dump_conf mode we do not want to
	interfere with any other instance of thermal-engine that is running */
	if (output_conf)
		return 0;

	do {
		rv = write(fd, buf + (size_t)pos, count - (size_t)pos);
		if (rv < 0)
			return -errno;
		pos += rv;
	} while ((ssize_t)count > pos);

	return (int)count;
}

/*===========================================================================
FUNCTION read_line_from_file

Utility function to read characters from file and stores them in a string
until (count-1) characters are read or either a newline or EOF is reached.

ARGUMENTS
	path - file path
	buf - destination buffer to read from
	count - max number of bytes to read from file

RETURN VALUE
	number of bytes read on success, -errno on failure.
===========================================================================*/
int read_line_from_file(const char *path, char *buf, size_t count)
{
	char * fgets_ret;
	FILE * fd;
	int rv;

	fd = fopen(path, "r");
	if (fd == NULL)
		return -1;

	fgets_ret = fgets(buf, (int)count, fd);
	if (NULL != fgets_ret) {
		rv = (int)strlen(buf);
	} else {
		rv = ferror(fd);
	}

	fclose(fd);

	return rv;
}

/*===========================================================================
FUNCTION write_to_file

Utility function to write to provided file path.

ARGUMENTS
	path - file path
	buf - destination buffer to write to
	count - number of bytes to write to file

RETURN VALUE
	number of bytes written on success, -errno on failure.
===========================================================================*/
int write_to_file(const char *path, const char *buf, size_t count)
{
	int fd, rv;
	/* When thermal-engine is in dump_conf mode we do not want to
	interfere with any other instance of thermal-engine that is running */
	if (output_conf)
		return 0;

	fd = open_file(path, O_RDWR);
	if (fd < 0)
		return fd;

	rv = write_to_fd(fd, buf, count);
	close(fd);

	return rv;
}

/*===========================================================================
FUNCTION connect_local_file_socket

Utility function to open and connect to local UNIX filesystem socket.

ARGUMENTS
	socket_name - name of local UNIX filesystem socket to be connected

RETURN VALUE
	Connected socket_fd on success,
	-1 on failure.
===========================================================================*/
static int connect_local_file_socket(const char *socket_name)
{
	int socket_fd = 0;
	struct sockaddr_un serv_addr;

	socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		dbgmsg("socket error - %s\n", strerror(errno));
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	snprintf(serv_addr.sun_path, UNIX_PATH_MAX, "%s", socket_name);
	serv_addr.sun_family = AF_LOCAL;

	if (connect(socket_fd, (struct sockaddr *) &serv_addr,
		    (socklen_t)(sizeof(sa_family_t) + strlen(socket_name)) ) != 0) {
		dbgmsg("connect error on %s - %s\n",
		       socket_name, strerror(errno));
		close(socket_fd);
		return -1;
	}

	return socket_fd;
}

/*===========================================================================
FUNCTION write_to_local_file_socket

Utility function to write to local filesystem UNIX socket.

ARGUMENTS
	socket_name - socket name to be written
	msg - message to be written on socket
	count - size of msg buffer to be written

RETURN VALUE
	Number of bytes written on success,
	-1 on failure.
===========================================================================*/
int write_to_local_file_socket(const char *socket_name, const char *msg, size_t count)
{
	int socket_fd = 0;
	int rv;

	if (minimum_mode) {
		return -1;
	}

	socket_fd = connect_local_file_socket(socket_name);
        if (socket_fd < 0) {
		return -1;
	}

	rv = write_to_fd(socket_fd, msg, count);
	close(socket_fd);

	return rv;
}

#define MAX_SOCKET_FD 3
static int socket_fd[MAX_SOCKET_FD] = { [0 ... MAX_SOCKET_FD-1] = -1};
static pthread_mutex_t local_socket_mtx = PTHREAD_MUTEX_INITIALIZER;

void add_local_socket_fd(int sfd)
{
	int i;
	char buf;

	pthread_mutex_lock(&local_socket_mtx);
	for (i = 0; i < MAX_SOCKET_FD; i++) {
		/* Add socket_id if the socket index is empty (-1) or if the socket_fd is invalid */
		if ((socket_fd[i] == -1) ||
		    (recv(socket_fd[i], &buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0)) {
			if (socket_fd[i] != -1)
				close(socket_fd[i]);
			socket_fd[i] = sfd;
			dbgmsg("Socket fd %d added to index %d\n", sfd, i);
			break;
		}
	}

	if (i == MAX_SOCKET_FD) {
		msg("Exceeded max local sockets request\n");
		close(sfd);
	}
	pthread_mutex_unlock(&local_socket_mtx);
}

/*===========================================================================
FUNCTION write_to_local_socket

Utility function to write to abstract local UNIX socket.

ARGUMENTS
        msg - message to be written on socket
        count - size of msg buffer to be written

RETURN VALUE
        Number of bytes written on success,
        -1 on failure.
===========================================================================*/
int write_to_local_socket(const char *msg, size_t count)
{
	int i, rv = 0;

	pthread_mutex_lock(&local_socket_mtx);
	for (i = 0; i < MAX_SOCKET_FD; i++) {
		if (socket_fd[i] >= 0) {
			if ((rv = write_to_fd(socket_fd[i], msg, count)) < 0) {
				close(socket_fd[i]);
				socket_fd[i] = -1;
			}
		}
	}
	pthread_mutex_unlock(&local_socket_mtx);
	return rv;
}

/*===========================================================================
FUNCTION get_tzn

Utility function to match a sensor name with thermal zone id.

ARGUMENTS
	sensor_name - name of sensor to match

RETURN VALUE
	Thermal zone id on success,
	-1 on failure.
===========================================================================*/
int get_tzn(const char *sensor_name)
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	int found = -1;
	int tzn = 0;
	char name[MAX_PATH] = {0};
	char cwd[MAX_PATH] = {0};

	if (!getcwd(cwd, sizeof(cwd)))
		return found;

	chdir(THERMAL_SYSFS); /* Change dir to read the entries. Doesnt work
				 otherwise */
	tdir = opendir(THERMAL_SYSFS);
	if (!tdir) {
		msg("Unable to open %s\n", THERMAL_SYSFS);
		return found;
	}

	while ((tdirent = readdir(tdir))) {
		char buf[50];
		struct dirent *tzdirent;
		DIR *tzdir = NULL;

		tzdir = opendir(tdirent->d_name);
		if (!tzdir)
			continue;
		while ((tzdirent = readdir(tzdir))) {
			if (strcmp(tzdirent->d_name, "type"))
				continue;
			snprintf(name, MAX_PATH, TZ_TYPE, tzn);
			dbgmsg("Opening %s\n", name);
			read_line_from_file(name, buf, sizeof(buf));
			buf[strlen(sensor_name)] = '\0';
			if (!strcmp(buf, sensor_name)) {
				found = 1;
				break;
			}
			tzn++;
		}
		closedir(tzdir);
		if (found == 1)
			break;
	}

	closedir(tdir);
	chdir(cwd); /* Restore current working dir */

	if (found == 1) {
		found = tzn;
		dbgmsg("Sensor %s found at tz: %d\n", sensor_name, tzn);
	}

	return found;
}

/*===========================================================================
FUNCTION get_num_cpus

Get number of CPU's

ARGUMENTS
	None

RETURN VALUE
	Number of CPUs
===========================================================================*/
int get_num_cpus(void)
{
	static int ncpus;
	static uint8_t ncpus_init;

	if (!ncpus_init) {
		ncpus = sysconf(_SC_NPROCESSORS_CONF);
		if (ncpus < 1)
			msg("%s: Error retrieving number of cores", __func__);
		else
			ncpus_init = 1;
	}
	dbgmsg("Number of CPU cores %d\n", ncpus);
	return ncpus;
}

#ifdef _PLATFORM_BASE
#define SYS_DEV_DIR  "/sys"_PLATFORM_BASE
#else
#define SYS_DEV_DIR "/sys/devices/"
#endif
/*===========================================================================
FUNCTION get_sysdev_dt_dir_name

Find path for driver name in "/sys/devices" directory.

ARGUMENTS
	dt_dir - char buffer of size MAX_PATH
	driver_name - name to search for in "/sys/devices"

RETURN VALUE
	Number of CPUs
===========================================================================*/
int get_sysdev_dt_dir_name(char *dt_dir, const char *driver_name)
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
		if (strstr(tdirent->d_name, driver_name)) {
			snprintf(dt_dir, MAX_PATH, "%s%s/", SYS_DEV_DIR,
				 tdirent->d_name);
			dbgmsg("%s: Found %s at %s\n", __func__, driver_name,
			       dt_dir);
			ret = 0;
			break;
		}
	}

	closedir(tdir);
	chdir(cwd);

	return ret;
}

/*===========================================================================
FUNCTION check_node

Check the node is present.

ARGUMENTS
	node_name - name of the node

RETURN VALUE
	0 - if the node is present
       -1 - if the node is not present/failure
===========================================================================*/
int check_node(char *node_name)
{
	int fd, ret = 0;

	if (node_name) {
		fd = open(node_name, O_RDONLY);
		if (fd < 0)
			ret = -1;
		else
			close(fd);
	} else {
		ret = -1;
	}

	return ret;
}

ssize_t write_to_trace(const char *msg, size_t count)
{
	ssize_t ret = 0;

	if (trace_fd > -1)
		ret = write(trace_fd, msg, count);
	return ret;
}

#define PROP_BASEBAND_VALUE_FUSION_4_5 "mdm2"
#define PROPERTY_BASEBAND    "ro.baseband"
#if defined (ENABLE_MODEM_MITIGATION) || defined(ENABLE_MODEM_TS)

qmi_client_qmux_instance_type get_fusion_qmi_client_type(void)
{
	qmi_client_qmux_instance_type ret_val = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
#ifdef ANDROID
	char prop_value[PROPERTY_VALUE_MAX];
	prop_value[0] = '\0';

	property_get(PROPERTY_BASEBAND, prop_value, "");

	if (strcmp(prop_value, PROP_BASEBAND_VALUE_FUSION_4_5) == 0)
		ret_val = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
#endif
	return ret_val;
}

#endif

