/*=========================================================================

 thermal_ioctl_interface.c

 GENERAL DESCRIPTION
 Thermal ioctl wrapper library for userspace applications to interact with
 kernel thermal monitor's ioctl interface.

 Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

=========================================================================*/

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <linux/msm_thermal_ioctl.h>
#include "thermal_ioctl_interface.h"

#ifdef ANDROID
#  include "cutils/properties.h"
#  ifdef USE_ANDROID_LOG
#    define LOG_TAG         "Thermal-IOCTL"
#    include "cutils/log.h"
#  endif
#else
#  include <syslog.h>
#endif

#ifdef USE_ANDROID_LOG
#include "common_log.h" /* define after cutils/log.h */
#define msg(format, ...)   LOGE(format, ## __VA_ARGS__)
#define info(format, ...)   LOGI(format, ## __VA_ARGS__)
#define dbg(format, ...)   LOGD(format, ##__VA_ARGS__)
#else
#define msg(format, ...)   syslog(LOG_ERR, format, ## __VA_ARGS__)
#define info(format, ...)   syslog(LOG_INFO, format, ## __VA_ARGS__)
#define dbg(format, ...)   syslog(LOG_DEBUG, format, ## __VA_ARGS__)
#endif

#define MSM_THERMAL_IOCTL_PATH "/dev/msm_thermal_query"

static int ioctl_fd = -1;
static uint32_t max_num_cpu;

static void init_msm_thermal_ioctl(struct msm_thermal_ioctl* init_var)
{
	memset(init_var, 0, sizeof(struct msm_thermal_ioctl));
	init_var->size = sizeof(struct msm_thermal_ioctl);
	return;
}

int thermal_ioctl_init(void)
{
	char ioctl_file[] = MSM_THERMAL_IOCTL_PATH;
	int ret = 0;

	if (ioctl_fd >= 0)
		goto INIT_EXIT;

	ioctl_fd = open(ioctl_file, O_RDWR);
	if (ioctl_fd < 0) {
		msg("Error in opening IOCTL file [%s]\n", ioctl_file);
		ret = -1;
		goto INIT_EXIT;
	}
	max_num_cpu = (uint32_t)sysconf(_SC_NPROCESSORS_CONF);
	info("KTM IOCTL interface \"%s\" opened\n", ioctl_file);
INIT_EXIT:
	return ret;
}

/*thermal_ioctl_defined(): determines whether the thermal IOCTL is
* defined/initialized
*
* return: 0 when the thermal IOCTL is initialized
*	  -1 when there is an error in thermal IOCTL initialization
*/
int thermal_ioctl_defined(void)
{
	return thermal_ioctl_init();
}

int thermal_ioctl_set_cluster_frequency(uint32_t cluster_num,
	uint32_t freq_request, uint8_t max_freq)
{
	int ret = 0;
	struct msm_thermal_ioctl query_req;

	if (ioctl_fd < 0) {
		msg("Thermal IOCTL not initialized\n");
		ret = -1;
		goto SET_FREQ_EXIT;
	}

	init_msm_thermal_ioctl(&query_req);
	query_req.cpu_freq.cpu_num = cluster_num;
	query_req.cpu_freq.freq_req = freq_request;
	ret = ioctl(ioctl_fd, (max_freq) ? MSM_THERMAL_SET_CLUSTER_MAX_FREQUENCY :
			MSM_THERMAL_SET_CLUSTER_MIN_FREQUENCY, &query_req);
	if (ret) {
		if (max_freq)
			msg(
			"Error in setting max frequency [%u] for cluster [%u].err %d\n",
			freq_request, cluster_num, ret);
		else
			msg(
			"Error in setting min frequency [%u] for cluster [%u].err %d\n",
			freq_request, cluster_num, ret);
		goto SET_FREQ_EXIT;
	}
SET_FREQ_EXIT:
	return ret;
}

int thermal_ioctl_set_frequency(uint32_t cpu_num, uint32_t freq_request, uint8_t max_freq)
{
	int ret = 0;
	struct msm_thermal_ioctl query_req;

	if (ioctl_fd < 0) {
		msg("Thermal IOCTL not initialized\n");
		ret = -1;
		goto SET_FREQ_EXIT;
	}
	if (cpu_num >= max_num_cpu) {
		msg("Invalid cpu value: [%u]\n", cpu_num);
		ret = -1;
		goto SET_FREQ_EXIT;
	}

	init_msm_thermal_ioctl(&query_req);
	query_req.cpu_freq.cpu_num = cpu_num;
	query_req.cpu_freq.freq_req = freq_request;
	ret = ioctl(ioctl_fd, (max_freq) ? MSM_THERMAL_SET_CPU_MAX_FREQUENCY :
		MSM_THERMAL_SET_CPU_MIN_FREQUENCY, &query_req);
	if (ret) {
		if (max_freq)
			msg("Error in setting max frequency [%u] for cpu [%u]\n",
			freq_request, cpu_num);
		else
			msg("Error in setting min frequency [%u] for cpu [%u]\n",
			freq_request, cpu_num);
		goto SET_FREQ_EXIT;
	}
SET_FREQ_EXIT:
	return ret;
}

int thermal_ioctl_get_cluster_freq_plan(int cluster_num,
				uint32_t *arr, uint32_t *len)
{
	int ret = 0;
	uint32_t set_idx = 0, len_index = 0;
	struct msm_thermal_ioctl query_req;

	if (ioctl_fd < 0) {
		msg("Thermal IOCTL not initialized\n");
		ret = -1;
		goto FREQ_PLAN_EXIT;
	}
	init_msm_thermal_ioctl(&query_req);
	query_req.clock_freq.cluster_num = (uint32_t)cluster_num;
	ret = ioctl(ioctl_fd, (int)MSM_THERMAL_GET_CLUSTER_FREQUENCY_PLAN, &query_req);
	if (ret) {
		msg("Error in frequency plan len query for cluster[%d].err %d\n",
			cluster_num, ret);
		goto FREQ_PLAN_EXIT;
	}

	if (!*len) {
		*len = query_req.clock_freq.freq_table_len;
		goto FREQ_PLAN_EXIT;
	}
	if (arr == NULL) {
		ret = -1;
		msg("Null pointer for frequency array for cluster[%d].ret %d\n",
			cluster_num, ret);
		goto FREQ_PLAN_EXIT;
	}

	set_idx = query_req.clock_freq.freq_table_len / MSM_IOCTL_FREQ_SIZE;
	if (query_req.clock_freq.freq_table_len % MSM_IOCTL_FREQ_SIZE)
		set_idx++;

	while (set_idx--) {
		query_req.clock_freq.set_idx = set_idx;
		ret = ioctl(ioctl_fd, (int)MSM_THERMAL_GET_CLUSTER_FREQUENCY_PLAN, &query_req);
		if (ret) {
			msg("Error in frequency plan query for cluster[%d].err %d\n",
				cluster_num, ret);
			goto FREQ_PLAN_EXIT;
		}
		memcpy(&arr[len_index], query_req.clock_freq.freq_table,
			query_req.clock_freq.freq_table_len * sizeof(unsigned int));
		len_index += query_req.clock_freq.freq_table_len;
	}

FREQ_PLAN_EXIT:
	return ret;
}

void thermal_ioctl_release(void)
{
	if (ioctl_fd >= 0) {
		close(ioctl_fd);
		ioctl_fd = -1;
	}
}
