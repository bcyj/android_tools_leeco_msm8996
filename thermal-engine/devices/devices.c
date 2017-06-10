/*===========================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stringl.h>

#include "devices_manager_internal.h"
#include "devices_actions.h"
#include "devices.h"
#include "thermal.h"
#include "inc/thermal_ioctl_interface.h"

/* CPU Frequency Scaling Action */

#define FREQ_AVAIL_NODE "/cpufreq/scaling_available_frequencies"
#define POSSIBLE_CPUS   (CPU_SYSFS_DIR "/possible")

struct tmd_generic_dev_info_t {
	char *name;
	uint32_t num_of_lvls;
	device_action action;
	void *data;
	enum device_type dev_type;
};
static int wlan_action(struct devices_manager_dev *dev_mgr);
static int battery_action(struct devices_manager_dev *dev_mgr);
static int lcd_action(struct devices_manager_dev *dev_mgr);
static int hotplug_action(struct devices_manager_dev *dev_mgr);
static int ts_shutdown_action(struct devices_manager_dev *dev_mgr);
static int ts_report_action(struct devices_manager_dev *dev_mgr);
static int cpu_action(struct devices_manager_dev *dev_mgr);
static int cpu_all_action(struct devices_manager_dev *dev_mgr);
static int cluster_action(struct devices_manager_dev *dev_mgr);
static int gpu_action(struct devices_manager_dev *dev_mgr);
static int modem_action(struct devices_manager_dev *dev_mgr);
static int fusion_modem_action(struct devices_manager_dev *dev_mgr);
static int vdd_restriction_action(struct devices_manager_dev *dev_mgr);
static int kernel_mitigation_action(struct devices_manager_dev *dev_mgr);
static int camera_action(struct devices_manager_dev *dev_mgr);
static int camcorder_action(struct devices_manager_dev *dev_mgr);
static int vdd_dig_automode_diable_action(struct devices_manager_dev *dev_mgr);
static int optimum_current_request_action(struct devices_manager_dev *dev_mgr);
static int mdp_action(struct devices_manager_dev *dev_mgr);
static int venus_action(struct devices_manager_dev *dev_mgr);
static int modem_cx_limit_action(struct devices_manager_dev *dev_mgr);

static void generic_dev_release(struct devices_manager_dev *dev_mgr);
static int generic_dev_add(struct tmd_generic_dev_info_t *gen_info);

static device_clnt_handle cpu_clnt[MAX_CPUS];

static struct tmd_generic_dev_info_t gen_dev_list[] = {
	{
		.name = "wlan",
		.num_of_lvls = MAX_WLAN_MITIGATION_LEVEL + 1,
		.action = wlan_action,
	},
	{
		.name = "battery",
		.num_of_lvls = MAX_BATTERY_MITIGATION_LEVEL + 1,
		.action = battery_action,
	},
	{
		.name = "lcd",
		.num_of_lvls = 255,
		.action = lcd_action,
		.dev_type = DEVICE_OP_VALUE_TYPE,
	},
	{
		.name = "shutdown",
		.num_of_lvls = UINT32_MAX,
		.action = ts_shutdown_action,
		.dev_type = DEVICE_DIRECT_ACTION_TYPE,
	},
	{
		.name = "none",
		.dev_type = DEVICE_NONE_TYPE,
	},
	{
		.name = "report",
		.dev_type = DEVICE_DIRECT_ACTION_TYPE,
		.action = ts_report_action,
	},
	{
		.name = "modem",
		.num_of_lvls = 4,
		.action = modem_action,
	},
	{
		.name = "fusion",
		.num_of_lvls = 4,
		.action = fusion_modem_action,
	},
	{
		.name = "vdd_restriction",
		.num_of_lvls = 2,
		.action = vdd_restriction_action,
	},
	{
		.name = "kernel",
		.num_of_lvls = 2,
		.action = kernel_mitigation_action,
	},
	{
		.name = "camera",
		.num_of_lvls = MAX_CAMERA_MITIGATION_LEVEL + 1,
		.action = camera_action,
	},
	{
		.name = "camcorder",
		.num_of_lvls = MAX_CAMCORDER_MITIGATION_LEVEL + 1,
		.action = camcorder_action,
	},
	{
		.name = "vdd_dig_swmode",
		.num_of_lvls = 2,
		.action = vdd_dig_automode_diable_action,
	},
	{
		.name = "opt_curr_req",
		.num_of_lvls = 2,
		.action = optimum_current_request_action,
	},
	{
		.name = "mdp",
		.num_of_lvls = MAX_MDP_MITIGATION_LEVEL + 1,
		.action = mdp_action,
	},
	{
		.name = "venus",
		.num_of_lvls = MAX_VENUS_MITIGATION_LEVEL + 1,
		.action = venus_action,
	},
	{
		.name = "modem_cx",
		.num_of_lvls = 4,
		.action = modem_cx_limit_action,
	},
};

static struct tmd_generic_dev_info_t hotplug_dev_list[] = {
	{
		.name = "hotplug_0",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)0,
	},
	{
		.name = "hotplug_1",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)1,
	},
	{
		.name = "hotplug_2",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)2,
	},
	{
		.name = "hotplug_3",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)3,
	},
	{
		.name = "hotplug_4",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)4,
	},
	{
		.name = "hotplug_5",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)5,
	},
	{
		.name = "hotplug_6",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)6,
	},
	{
		.name = "hotplug_7",
		.num_of_lvls = 2,
		.action = hotplug_action,
		.data   = (void *)7,
	},
};

static void tmd_release_cluster_dev(struct devices_manager_dev *dev_mgr)
{
	free(dev_mgr->lvl_info);
	free(dev_mgr);
}

static int tmd_add_clusterfreq_dev_data(int cluster_id)
{
	int ret_val = 0;
	uint32_t num_freqs = 0;
	struct devices_manager_dev *dev_mgr = NULL;
	unsigned int *arr = NULL;
	uint32_t arr_idx;

	dev_mgr = malloc(sizeof(struct devices_manager_dev));
	if (dev_mgr == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}
	memset(dev_mgr, 0, sizeof(struct devices_manager_dev));

	if (thermal_ioctl_get_cluster_freq_plan(
		cluster_id, arr, &num_freqs)) {
		msg("%s: error reading cluster[%d] freq plan len\n",
			__func__, cluster_id);
		ret_val = -(EFAULT);
		goto handle_error;
	}

	if (num_freqs > 0) {
		arr = malloc(sizeof(unsigned int) * num_freqs);
		if (arr == NULL) {
			msg("%s: malloc failed.\n", __func__);
			ret_val = -(ENOMEM);
			goto handle_error;
		}
		if (thermal_ioctl_get_cluster_freq_plan(
			cluster_id, arr, &num_freqs)) {
			msg("%s: error in get cluster[%d] freq\n",
				__func__, cluster_id);
			ret_val = -(EFAULT);
			goto handle_error;
		}
	} else {
		msg("%s: Invalid number for freqs:%d\n", __func__, num_freqs);
		ret_val = -(EFAULT);
		goto handle_error;
	}

	/* Sort in descending order */
	sort_int_arr((int *)arr, num_freqs, 0);

	dev_mgr->lvl_info =
		malloc(sizeof(struct device_lvl_info) * num_freqs);
	if (dev_mgr->lvl_info == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}

	snprintf(dev_mgr->dev_info.name, DEVICES_MAX_NAME_LEN, "cluster%d",
		 cluster_id);
	dev_mgr->data   = (void *)(intptr_t)cluster_id;
	dev_mgr->action = cluster_action;

	memset(dev_mgr->lvl_info, 0,
	       sizeof(struct device_lvl_info) * num_freqs);
	for (arr_idx = 0; arr_idx < num_freqs; arr_idx++) {
		dbgmsg("%s: %s lvl_info[%d]=%d", __func__,
		       dev_mgr->dev_info.name, arr_idx, arr[arr_idx]);
		dev_mgr->lvl_info[arr_idx].lvl.value = (int)arr[arr_idx];
	}

	dev_mgr->dev_info.num_of_levels = num_freqs;

	dev_mgr->dev_info.dev_type = DEVICE_OP_VALUE_TYPE;

	dev_mgr->dev_info.max_dev_op_value_valid = 1;
	dev_mgr->dev_info.max_dev_op_value = dev_mgr->lvl_info[0].lvl.value;
	dev_mgr->dev_info.min_dev_op_value_valid = 1;
	dev_mgr->dev_info.min_dev_op_value =
		dev_mgr->lvl_info[num_freqs - 1].lvl.value;
	dev_mgr->active_req.value = dev_mgr->dev_info.max_dev_op_value;
	dev_mgr->release = tmd_release_cluster_dev;

	if (devices_manager_add_dev(dev_mgr) != 0) {
		msg("%s: Can't add device\n", __func__);
		ret_val = -(EFAULT);
		goto handle_error;
	}

handle_error:
	if (ret_val < 0) {
		/* Error clean up */
		if (dev_mgr) {
			if (dev_mgr->lvl_info)
				free(dev_mgr->lvl_info);
			free(dev_mgr);
		}
		if (arr)
			free(arr);
	}
	return ret_val;
}

static void tmd_release_cpufreq_dev(struct devices_manager_dev *dev_mgr)
{
	free(dev_mgr->lvl_info);
	free(dev_mgr);
}

static int tmd_add_cpufreq_dev_data(int cpu_idx)
{
	int ret_val = 0;
	uint32_t num_freqs = 0;
	struct devices_manager_dev *dev_mgr = NULL;
	char path[DEVICES_MAX_PATH] = {0};
	int arr[50];
	uint32_t arr_idx;

	dev_mgr = malloc(sizeof(struct devices_manager_dev));
	if (dev_mgr == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}
	memset(dev_mgr, 0, sizeof(struct devices_manager_dev));

	memset(arr, 0, sizeof(arr));

	/* Use CPU0 frequencies because they are equivalent on all CPU's
	   and sysfs nodes required for generating information are present
	   only in CPU's that are online. It is possible that CPU1+ maybe
	   offline when this info set is being generated. */
	snprintf(path, sizeof(path), CPU_SYSFS(FREQ_AVAIL_NODE), 0);
	num_freqs = read_int_list_from_file(path, arr, ARRAY_SIZE(arr));
	if (num_freqs < 1) {
		msg("%s: Invalid number for freqs\n", __func__);
		ret_val = -(EFAULT);
		goto handle_error;
	}

	/* Sort in descending order */
	sort_int_arr(arr, num_freqs, 0);

	dev_mgr->lvl_info =
		malloc(sizeof(struct device_lvl_info) * num_freqs);
	if (dev_mgr->lvl_info == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}

	if (cpu_idx == INT32_MAX) {
		snprintf(dev_mgr->dev_info.name, DEVICES_MAX_NAME_LEN, "cpu");
		dev_mgr->action = cpu_all_action;
	} else {
		snprintf(dev_mgr->dev_info.name, DEVICES_MAX_NAME_LEN, "cpu%d",
			 cpu_idx);
		dev_mgr->data   = (void *)(intptr_t)cpu_idx;
		dev_mgr->action = cpu_action;
	}

	memset(dev_mgr->lvl_info, 0,
	       sizeof(struct device_lvl_info) * num_freqs);
	for (arr_idx = 0; arr_idx < num_freqs; arr_idx++) {
		dbgmsg("%s: %s lvl_info[%d]=%d", __func__,
		       dev_mgr->dev_info.name, arr_idx, arr[arr_idx]);
		dev_mgr->lvl_info[arr_idx].lvl.value = arr[arr_idx];
	}

	dev_mgr->dev_info.num_of_levels = num_freqs;

	dev_mgr->dev_info.dev_type = DEVICE_OP_VALUE_TYPE;

	dev_mgr->dev_info.max_dev_op_value_valid = 1;
	dev_mgr->dev_info.max_dev_op_value = dev_mgr->lvl_info[0].lvl.value;
	dev_mgr->dev_info.min_dev_op_value_valid = 1;
	dev_mgr->dev_info.min_dev_op_value =
		dev_mgr->lvl_info[num_freqs - 1].lvl.value;
	dev_mgr->active_req.value = dev_mgr->dev_info.max_dev_op_value;
	dev_mgr->release = tmd_release_cpufreq_dev;

	if (devices_manager_add_dev(dev_mgr) != 0) {
		msg("%s: Can't add device\n", __func__);
		ret_val = -(EFAULT);
	}

handle_error:
	if (ret_val < 0) {
		/* Error clean up */
		if (dev_mgr) {
			if (dev_mgr->lvl_info)
				free(dev_mgr->lvl_info);
			free(dev_mgr);
		}
	}
	return ret_val;
}

int tmd_init_cluster_devs(void)
{
	int num_cluster = 0;
	int ret_val = 0;
	int idx = 0;
	char name[DEVICES_MAX_NAME_LEN];

	num_cluster = get_num_clusters();
	if (num_cluster <= 0) {
		msg("%s: Can't read number of clusters\n", __func__);
		return -(EIO);
	}

	/* Add  cluster TMD's */
	for (idx = 0; idx < num_cluster; idx++) {
		if (tmd_add_clusterfreq_dev_data(idx) != 0) {
			msg("%s: Error adding cluster%d Device\n", __func__,
			    idx);
			ret_val = -(EFAULT);
			break;
		}

		snprintf(name, DEVICES_MAX_NAME_LEN, "cluster%d", idx);
	}

	return ret_val;
}

int tmd_init_cpu_devs(void)
{
	int num_cpus = 0;
	int ret_val = 0;
	int idx = 0;
	char name[DEVICES_MAX_NAME_LEN];
	int cluster_id = -1;

	num_cpus = get_num_cpus();
	if (num_cpus <= 0) {
		msg("%s: Can't read number of CPUs\n", __func__);
		return -(EIO);
	}
	/* CPU device is supported only on targets with just
	   one async cluster */
	if (get_num_clusters() <= 0 ||
		(get_num_clusters() == 1 &&
			is_cluster_sync(get_cluster_id(0)) == 0)) {
		/* Add overall CPU TMD node */
		if (tmd_add_cpufreq_dev_data(INT32_MAX) != 0) {
			msg("%s: Error adding overall CPU Device\n", __func__);
			return -(EFAULT);
		}
	} else {
                dbgmsg("%s: Target doesn't support overall CPU device\n",
			__func__);
	}

	/* Add individual CPU TMD's */
	for (idx = 0; idx < num_cpus; idx++) {
		cluster_id = get_cluster_id(idx);
		if (cluster_id != -1) {
			if (is_cluster_sync(cluster_id)) {
				dbgmsg(
				"%s: Target doesn't support CPU%d device\n",
				__func__,idx);
				continue;
			}
		}
		if (tmd_add_cpufreq_dev_data(idx) != 0) {
			msg("%s: Error adding CPU%d Device\n", __func__,
			    idx);
			ret_val = -(EFAULT);
			break;
		}

		snprintf(name, DEVICES_MAX_NAME_LEN, "cpu%d", idx);
		cpu_clnt[idx] = devices_manager_reg_clnt(name);
		if (cpu_clnt[idx] == NULL) {
			msg("%s: Error adding adding client for %s\n"
			    , __func__, name);
			ret_val = -(EFAULT);
			break;
		}
	}

	/* Add hotplug devices */
	for (idx = 0; idx < ARRAY_SIZE(hotplug_dev_list); idx++) {
		if ((idx >=  ARRAY_SIZE(hotplug_dev_list)) ||
		    (generic_dev_add(&hotplug_dev_list[idx]) != 0)) {
			msg("%s: Error adding %s\n", __func__,
			    hotplug_dev_list[idx].name);
			ret_val = -(EFAULT);
			break;
		}
	}

	return ret_val;
}
#if 0
static void tmd_release_gpufreq_dev(struct devices_manager_dev *dev_mgr)
{
	free(dev_mgr->lvl_info);
	free(dev_mgr);
}
#endif
int tmd_init_gpu_devs(void)
{
	int ret_val = 0;
	uint32_t num_freqs = 0;
	struct devices_manager_dev *dev = NULL;
	char path[DEVICES_MAX_PATH] = {0};
	int arr[50];
	uint32_t arr_idx;

	dev = malloc(sizeof(struct devices_manager_dev));
	if (dev == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}
	memset(dev, 0, sizeof(struct devices_manager_dev));

	memset(arr, 0, sizeof(arr));
	snprintf(path, sizeof(path), GPU_SYSFS(GPU_AVAIL_FREQ_NODE), 0);
	num_freqs = read_int_list_from_file(path, arr, ARRAY_SIZE(arr));
	if (num_freqs < 1) {
		msg("%s: Invalid number for freqs\n", __func__);
		ret_val = -(EFAULT);
		goto handle_error;
	}
	/* Sort in descending order */
	sort_int_arr(arr, num_freqs, 0);

	dev->lvl_info =
		malloc(sizeof(struct device_lvl_info) * num_freqs);
	if (dev->lvl_info == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}
	memset(dev->lvl_info, 0, sizeof(struct device_lvl_info) * num_freqs);
	snprintf(dev->dev_info.name, DEVICES_MAX_NAME_LEN, "gpu");

	for (arr_idx = 0; arr_idx < num_freqs; arr_idx++) {
		dbgmsg("%s: %s lvl_info[%d]=%d", __func__, dev->dev_info.name,
		       arr_idx, arr[arr_idx]);
		dev->lvl_info[arr_idx].lvl.value = arr[arr_idx];
	}

	dev->dev_info.num_of_levels = num_freqs;
	dev->dev_info.dev_type = DEVICE_OP_VALUE_TYPE;

	dev->dev_info.max_dev_op_value_valid = 1;
	dev->dev_info.max_dev_op_value = dev->lvl_info[0].lvl.value;
	dev->active_req.value = dev->dev_info.max_dev_op_value;
	dev->dev_info.min_dev_op_value_valid = 1;
	dev->dev_info.min_dev_op_value =
		dev->lvl_info[num_freqs - 1].lvl.value;
	dev->action = gpu_action;
	dev->release = generic_dev_release;

	if (devices_manager_add_dev(dev) != 0) {
		msg("%s: Can not add device\n", __func__);
		ret_val = -(EFAULT);
	}

handle_error:
	if (ret_val < 0) {
		/* Error clean up */
		if (dev) {
			if (dev->lvl_info)
				free(dev->lvl_info);
			free(dev);
		}
	}
	return ret_val;
}

static int wlan_action(struct devices_manager_dev *dev_mgr)
{
	return wlan_request(dev_mgr->active_req.value);
}

static int battery_action(struct devices_manager_dev *dev_mgr)
{
	return battery_request(dev_mgr->active_req.value);
}

static int lcd_action(struct devices_manager_dev *dev_mgr)
{
	return lcd_brightness_request(dev_mgr->active_req.value);
}

static int hotplug_action(struct devices_manager_dev *dev_mgr)
{
	return hotplug_request((int)(intptr_t)dev_mgr->data,
			       (dev_mgr->active_req.value) ? (1) : (0));
}

static int ts_shutdown_action(struct devices_manager_dev *dev_mgr)
{
	/* Level greater than zero is delay time */
	return shutdown_action((int)dev_mgr->active_req.value);
}

static int ts_report_action(struct devices_manager_dev *dev_mgr)
{
	/* Level greater than zero is delay time */
	return report_action((char *)dev_mgr->active_req.data);
}

static int cluster_action(struct devices_manager_dev *dev_mgr)
{
	return clusterfreq_request((int)(intptr_t)dev_mgr->data,
			       dev_mgr->active_req.value);
}

static int cpu_action(struct devices_manager_dev *dev_mgr)
{
	return cpufreq_request((int)(intptr_t)dev_mgr->data,
			       dev_mgr->active_req.value);
}

static int cpu_all_action(struct devices_manager_dev *dev_mgr)
{
	int idx;

	for (idx = 0; idx < num_cpus; idx++)
		device_clnt_request(cpu_clnt[idx], &(dev_mgr->active_req));
	return 0;
}

static int gpu_action(struct devices_manager_dev *dev_mgr)
{
	return gpufreq_request(0, dev_mgr->active_req.value);
}

static int modem_action(struct devices_manager_dev *dev_mgr)
{
	return modem_request(dev_mgr->active_req.value);
}

static int fusion_modem_action(struct devices_manager_dev *dev_mgr)
{
	return fusion_modem_request(dev_mgr->active_req.value);
}

static int vdd_restriction_action(struct devices_manager_dev *dev_mgr)
{
	return vdd_restriction_request(dev_mgr->active_req.value);
}

static int kernel_mitigation_action(struct devices_manager_dev *dev_mgr)
{
	return kernel_mitigation_request(dev_mgr->active_req.value);
}
static int camera_action(struct devices_manager_dev *dev_mgr)
{
	return camera_request(dev_mgr->active_req.value);
}

static int camcorder_action(struct devices_manager_dev *dev_mgr)
{
	return camcorder_request(dev_mgr->active_req.value);
}

static int vdd_dig_automode_diable_action(struct devices_manager_dev *dev_mgr)
{
	return vdd_dig_sw_mode_request((dev_mgr->active_req.value) ? (2) : (0));
}

static int optimum_current_request_action(struct devices_manager_dev *dev_mgr)
{
	return optimum_current_request(dev_mgr->active_req.value);
}

static int mdp_action(struct devices_manager_dev *dev_mgr)
{
	return mdp_request(dev_mgr->active_req.value);
}

static int venus_action(struct devices_manager_dev *dev_mgr)
{
	return venus_request(dev_mgr->active_req.value);
}

static int modem_cx_limit_action(struct devices_manager_dev *dev_mgr)
{
	return modem_cx_limit_request(dev_mgr->active_req.value);
}

static void generic_dev_release(struct devices_manager_dev *dev_mgr)
{
	free(dev_mgr);
}

static int generic_dev_add(struct tmd_generic_dev_info_t *gen_info)
{
	int ret_val = 0;
	struct devices_manager_dev *dev = NULL;

	dev = malloc(sizeof(struct devices_manager_dev));
	if (dev == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}
	memset(dev, 0, sizeof(struct devices_manager_dev));

	dev->dev_info.num_of_levels = gen_info->num_of_lvls;

	strlcpy(dev->dev_info.name, gen_info->name, DEVICES_MAX_NAME_LEN);
	dev->dev_info.dev_type = gen_info->dev_type;

	if (dev->dev_info.dev_type == DEVICE_OP_VALUE_TYPE) {
		dev->dev_info.max_dev_op_value_valid = 1;
		dev->dev_info.max_dev_op_value = (int)gen_info->num_of_lvls;
		dev->active_req.value = dev->dev_info.max_dev_op_value;
	}

	if (gen_info->action)
		dev->action = gen_info->action;

	if (gen_info->data)
		dev->data = gen_info->data;

	dev->release = generic_dev_release;

	if (devices_manager_add_dev(dev) != 0) {
		msg("%s: Can not add device\n", __func__);
		ret_val = -(EFAULT);
	}

handle_error:
	if (ret_val < 0) {
		/* Error clean up */
		if (dev)
			free(dev);
	}
	return ret_val;
}

static int init_generic_devs(void)
{
	int ret_val = 0;
	int idx;

	for (idx = 0; idx < ARRAY_SIZE(gen_dev_list); idx++) {
		ret_val = generic_dev_add(&gen_dev_list[idx]);
		if (ret_val) {
			msg("%s: failed to add %s device.\n", __func__,
			    gen_dev_list[idx].name);
			break;
		}
	}
	return ret_val;
}

static int min_mode;

/* TMD init */
int devices_init(int minimum_mode)
{
	int ret_val = 0;

	min_mode = minimum_mode;
	gpufreq_init();
	cpufreq_init();
	clusterfreq_init();
	thermal_ioctl_init();

	if (!min_mode)
		qmi_communication_init();

	ret_val = tmd_init_cpu_devs();
	if (ret_val)
		msg("%s: Init CPU TMD failed %d\n", __func__, ret_val);

	ret_val = tmd_init_cluster_devs();
	if (ret_val)
		msg("%s: Init cluster TMD failed %d\n", __func__, ret_val);

	ret_val = tmd_init_gpu_devs();
	if (ret_val)
		msg("%s: Init GPU TMD failed %d\n", __func__, ret_val);

	ret_val = init_generic_devs();
	if (ret_val)
		msg("%s: Init generic TMDs failed %d\n", __func__, ret_val);

	/* Functions to execute post devices added to device manager */
	hotplug_init();
	vdd_dig_sw_mode_init();
	vdd_rstr_init();
	opt_curr_req_init();

	return ret_val;
}

void devices_release(void)
{
	if (!min_mode)
		qmi_communication_release();
	thermal_ioctl_release();
}
