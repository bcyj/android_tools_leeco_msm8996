/*===========================================================================

Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stringl.h>
#include "thermal.h"
#include "thermal_config.h"
#include "sensors_manager.h"
#include "devices_manager.h"
#include "server/thermal_lib_common.h"
#include "thermal_server.h"

#define MAX_TM_INSTANCES_SUPPORTED 64U
#define MAX_ACTIONS_PER_TM_INSTANCE 16U

static pthread_t       tm_thread;
static pthread_cond_t  wait_cond = PTHREAD_COND_INITIALIZER;
static uint32_t        thresh_reached_mask[2];
static uint8_t         override_mode;
static pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;

#define THRESH_MASK_SET(idx) ((idx > 31)?\
		(thresh_reached_mask[1] |= (((uint32_t)0x1) << (idx - 32))):\
		(thresh_reached_mask[0] |= (((uint32_t)0x1) << idx)))
#define THRESH_MASK_CLR(idx) ((idx > 31)?\
		(thresh_reached_mask[1] &= (~(((uint32_t)0x1) << (idx - 32)))):\
		(thresh_reached_mask[0] &= (~(((uint32_t)0x1) << idx))))
#define THRESH_MASK_ANY_SET (thresh_reached_mask[0] | thresh_reached_mask[1])
#define THRESH_MASK_IS_SET(idx) ((idx > 31)?\
		(thresh_reached_mask[1] & (((uint32_t)0x1) << (idx - 32))):\
		(thresh_reached_mask[0] & (((uint32_t)0x1) << idx)))

struct tm_action {
	struct device_info *dev_info;
	int    dev_clnt_idx;
};

struct tm_threshold {
	struct tm_action action[ACTIONS_MAX];
};

struct tm_instance_info {
	struct setting_info *setting;
	int                  last_lvl;
	unsigned int         action_mask;
	int                  lvl_alarm[THRESHOLDS_MAX];
	device_clnt_handle   dev_clnt_list[MAX_ACTIONS_PER_TM_INSTANCE];
	struct device_info  *dev_info_list[MAX_ACTIONS_PER_TM_INSTANCE];
	sensor_clnt_handle   ts_clnt;
	uint8_t              disable;
};

static device_clnt_handle kernel_dev;

static struct tm_instance_info tm_states[MAX_TM_INSTANCES_SUPPORTED];
static uint32_t tm_cnt;

static struct device_info *device_info_arr;
static uint32_t  device_info_arr_len;

static int report_msg(device_clnt_handle clnt, const char *sensor,
		      int temperature, int level, int is_trigger)
{
	char tempBuf[REPORT_MSG_MAX];
	union device_request req;

	info("ACTION: REPORT - "
	    "Sensor '%s' - temperature %d, threshold level %d, is_trigger %s",
	    sensor, temperature, level + 1,
	    is_trigger ? "true" : "false");

	snprintf(tempBuf, REPORT_MSG_MAX, "%s\n%d\n%d\n%s",
		 sensor, temperature, level + 1,
		 is_trigger ? "true" : "false");

	req.data = (void*)tempBuf;
	device_clnt_request(clnt, &req);
	return 0;
}

struct tm_devices_list {
	char *device[MAX_ACTIONS_PER_TM_INSTANCE];
	int   cnt;
};

static struct device_info *get_device_info(const char *device)
{
	uint32_t i;

	struct device_info *info = NULL;
	for (i = 0; i < device_info_arr_len; i++) {
		if (strncasecmp(device_info_arr[i].name, device,
				DEVICES_MAX_NAME_LEN) == 0)
			info = &device_info_arr[i];
	}
	return info;
}

/* Add device to list if needed, and return position it was added or already
   exists. List will be filled in order. Return -1 if can not be added. */
static int add_device_to_list(struct tm_instance_info *tm_instance_info,
			      struct tm_devices_list *list, const char *device)
{
	uint32_t i;

	/* Search for match or first available slot. */
	for (i = 0; i < MAX_ACTIONS_PER_TM_INSTANCE; i++) {
		/* Add to first empty entry, if no previous match. */
		if (list->device[i] == NULL)
			break;
		if (strncasecmp(list->device[i], device,
				DEVICES_MAX_NAME_LEN) == 0)
			break;
	}

	if (i >= MAX_ACTIONS_PER_TM_INSTANCE) {
		msg("%s: No room for device %s", __func__, device);
		return -1;
	}

	/* Check if we need to create the device client */
	if (list->device[i] == NULL) {
		tm_instance_info->dev_info_list[i] = get_device_info(device);
		if (tm_instance_info->dev_info_list[i] == NULL)
			return -1;
		tm_instance_info->dev_clnt_list[i] =
			devices_manager_reg_clnt(device);
		if (tm_instance_info->dev_clnt_list[i] == NULL)
			return -1;
		list->device[i] = (char*)device;
		list->cnt++;
	}

	return (int)i;
}
static int create_device_clnts(struct tm_instance_info *tm_instance_info)
{
	int ret_val = 0;
	uint32_t t_idx, a_idx;
	struct tm_devices_list list;
	struct tm_setting *tm_setting_info = &tm_instance_info->setting->data.tm;

	memset(&list, 0x0, sizeof(struct tm_devices_list));
	/* Create list of unique actions */
	for (t_idx = 0; t_idx < tm_setting_info->num_thresholds; t_idx++) {
		for (a_idx = 0; a_idx < tm_setting_info->t[t_idx].num_actions;
		     a_idx++) {
			/* Index used by tm to make requests on correct device
			   client */
			tm_setting_info->t[t_idx].actions[a_idx].device_idx  =
				add_device_to_list(tm_instance_info, &list,
						   tm_setting_info->t[t_idx].actions[a_idx].device);
			if (tm_setting_info->t[t_idx].actions[a_idx].device_idx < 0) {
				msg("%s: Error adding device %s\n", __func__,
				    tm_setting_info->t[t_idx].actions[a_idx].device);
				ret_val = -(EFAULT);
				goto error_handler;
			}
		}
	}

error_handler:
	return ret_val;
}

static int validate_device_client(struct tm_instance_info *tm_instance_info)
{
	uint32_t t_idx, a_idx;
	struct tm_setting *tm_setting_info = &tm_instance_info->setting->data.tm;

	for (t_idx = 0; t_idx < tm_setting_info->num_thresholds; t_idx++) {
		for (a_idx = 0; a_idx < tm_setting_info->t[t_idx].num_actions;
		     a_idx++) {
			int device_idx =
				tm_setting_info->t[t_idx].actions[a_idx].device_idx;
			if (!(device_idx >= 0 &&
			      tm_instance_info->dev_info_list[device_idx] != NULL &&
			      tm_instance_info->dev_clnt_list[device_idx] != NULL))
				return -1;
		}
	}
	return 0;
}

static int sensors_setup(void)
{
	uint32_t i = 0;
	int sensor_count = 0;

	if (!tm_cnt)
		return 0;

	/* Set up tm instances */
	dbgmsg("%s: tm_cnt %d", __func__, tm_cnt);
	for (i = 0; i < tm_cnt; i++) {
		struct tm_instance_info *tm_instance_info;
		struct setting_info *setting;
		struct tm_setting *tm_setting_info;

		tm_instance_info = &tm_states[i];
		setting = tm_instance_info->setting;
		tm_setting_info = &(setting->data.tm);

		dbgmsg("%s: TM Id %s Sensor %s num_thresholds %d", __func__,
		    setting->desc, tm_setting_info->sensor,
		    tm_setting_info->num_thresholds);
		if (tm_setting_info->num_thresholds > 0) {
			/* Create sensor client */
			tm_instance_info->ts_clnt =
				sensors_manager_reg_clnt(tm_setting_info->sensor);
			if (tm_instance_info->ts_clnt == NULL) {
				msg("%s: Can't create client for %s.\n",
				    __func__, tm_setting_info->sensor);
				tm_instance_info->disable = 1;
				continue;
			}

			/* Create necessary device clients */
			if (create_device_clnts(tm_instance_info) == 0)
				sensor_count++;
			else
				tm_instance_info->disable = 1;
		}
	}
	return sensor_count;
}

#if 0
static void sensors_shutdown(void)
{
	int i = 0;
	int a_idx = 0;
	/* Set up sensors */
	for (i = 0; i < tm_cnt; i++) {
		/* Destroy ts client */
		if (tm_states[i].ts_clnt)
			sensors_manager_dereg_clnt(tm_states[i].ts_clnt);

		/* Destroy tmd clients */
		for (a_idx = 0; a_idx < MAX_ACTIONS_PER_TM_INSTANCE; a_idx++) {
			if (tm_states[i].dev_clnt_list[a_idx])
				devices_manager_dereg_clnt(tm_states[i].dev_clnt_list[a_idx]);
		}
	}
}
#endif
static int sensor_threshold_trigger(int value, struct tm_instance_info *sensor, int level)
{
	struct tm_setting *tm_info = &(sensor->setting->data.tm);

	if (tm_info->descending_thresh) {
		if (value <= tm_info->t[level].lvl_trig)
			return 1;
		else
			return 0;
	} else {
		int active_trig = tm_info->t[level].lvl_trig;

		if (override_mode)
			active_trig += tm_info->override;;

		if (value >= active_trig)
			return 1;
		else
			return 0;
	}
}

static int sensor_threshold_clear(int value, struct tm_instance_info *sensor, int level)
{
	struct tm_setting *tm_info = &(sensor->setting->data.tm);

	if (tm_info->descending_thresh) {
		if (value >= tm_info->t[level].lvl_clr)
			return 1;
		else
			return 0;
	} else {
		int active_clr = tm_info->t[level].lvl_clr;

		if (override_mode)
			active_clr += tm_info->override;

		if (value <= active_clr)
			return 1;
		else
			return 0;
	}
}


static int sensor_get_temperature(struct tm_instance_info *setting)
{
	int temp = 0;

	if (setting == NULL ||
	    setting->ts_clnt == NULL) {
		return -EFAULT;
	}

	temp = sensors_manager_read(setting->ts_clnt);
	dbgmsg("TM Id %s Sensor %s Reading %d\n",
	       setting->setting->desc,
	       setting->setting->data.tm.sensor, temp);

	return temp;
}

static int override_notify(int mode, void *data, void *reserved)
{
	uint32_t idx;
	struct tm_instance_info *sensor;
	struct setting_info *info;
	struct tm_setting *tm_info;

	dbgmsg("%s: TM mode %d", __func__, mode);
	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	if (override_mode != mode) {
		override_mode = (uint8_t)mode;
		for (idx = 0; idx < tm_cnt; idx++) {
			sensor = &tm_states[idx];
			info = sensor->setting;
			tm_info = &(info->data.tm);

			if ((tm_info->num_thresholds < 1) ||
			    (sensor->disable) ||
			    (tm_info->override == 0))
				continue;
			THRESH_MASK_SET(idx);
		}
		/*Trigger thresholds for overridable tm instances.
		  This will cause mitigation levels to be re-evaluated */
		pthread_cond_broadcast(&wait_cond);
	}
	pthread_mutex_unlock(&wait_mutex);

	return 0;
}

static int validate_config_thresholds(struct tm_instance_info *tm_instance_info)
{
	uint8_t i = 0;
	struct tm_setting *tm_info = &tm_instance_info->setting->data.tm;

	if (tm_info->num_thresholds == 0 ||
	    tm_info->num_thresholds > THRESHOLDS_MAX)
		return -1;

	for (i = 0; i < tm_info->num_thresholds; i++) {
		if (tm_info->t[i].lvl_trig == INT_MIN ||
		    tm_info->t[i].lvl_clr == INT_MIN)
			return -1;

		if (tm_info->descending_thresh) {
			if (tm_info->t[i].lvl_trig >= tm_info->t[i].lvl_clr) {
				msg("%s: Invalid thresholds and thrsholds_clr rquest\n",
				    __func__);
				return -1;
			}
		} else {
			if (tm_info->t[i].lvl_trig <= tm_info->t[i].lvl_clr) {
				msg("%s: Invalid thresholds and thrsholds_clr rquest\n",
				    __func__);
				return -1;
			}
		}
	}
	return 0;
}

static int validate_disabled_config(struct tm_instance_info *tm_instance_info)
{
	if (tm_instance_info == NULL)
		return -1;

	struct setting_info *setting = tm_instance_info->setting;

	if (setting == NULL ||
	    tm_instance_info->ts_clnt == NULL ||
	    validate_device_client(tm_instance_info) < 0 ||
	    validate_config_thresholds(tm_instance_info) < 0)
		return -1;

	return 0;
}

static int handle_disable_config_adjust(struct tm_instance_info *tm_instance_info,
				        uint8_t disable)
{
	if (tm_instance_info == NULL)
		return -1;

	if (!disable) {
		if (validate_disabled_config(tm_instance_info))
			return -1;

		tm_instance_info->disable = disable;
	} else {
		uint8_t i;
		/* release sensor client req */
		if (tm_instance_info->ts_clnt)
			sensors_manager_set_thresh_lvl(tm_instance_info->ts_clnt,
						       NULL);

		/* Cancel all device request */
		for (i = 0; i < MAX_ACTIONS_PER_TM_INSTANCE; i++) {
			if (tm_instance_info->dev_clnt_list[i] == NULL)
				continue;

			device_clnt_cancel_request(tm_instance_info->dev_clnt_list[i]);
		}
		tm_instance_info->disable = disable;
	}

	return 0;
}

static void copy_current_monitor_setting(struct tm_setting *tm_info,
					 struct monitor_settings* curr,
					 enum supported_fields field)
{
	uint8_t i = 0;

	if (tm_info == NULL ||
	    curr == NULL ||
	    field == UNKNOWN_FIELD)
		return;

	curr->num_thresholds = tm_info->num_thresholds;

	switch(field) {
		case THRESHOLDS_FIELD:
			for (i = 0; i < tm_info->num_thresholds; i++)
				curr->t[i].threshold_trig =
					tm_info->t[i].lvl_trig;
			break;
		case THRESHOLDS_CLR_FIELD:
			for (i = 0; i < tm_info->num_thresholds; i++)
				curr->t[i].threshold_clr =
					tm_info->t[i].lvl_clr;
			break;
		case ACTION_INFO_FIELD:
			for (i = 0; i < tm_info->num_thresholds; i++) {
				uint8_t j = 0;
				curr->t[i].num_actions =
				tm_info->t[i].num_actions;
				for (j = 0; j < curr->t[i].num_actions; j++) {
					curr->t[i].actions[j].info =
						tm_info->t[i].actions[j].info;
				}
			}
			break;
		default:
			break;
	}
	return;
}

static int validate_client_actions_info(struct tm_instance_info *tm_instance_info,
				        struct override_threshold_t *clnt_t,
				        uint32_t num_thresholds)
{
	uint8_t i = 0;

	if (tm_instance_info == NULL ||
	    num_thresholds == 0 ||
	    num_thresholds >= THRESHOLDS_MAX)
		return -1;

	struct tm_setting *tm_info = &tm_instance_info->setting->data.tm;

	if (num_thresholds != tm_info->num_thresholds)
		return -1;

	for (i = 0; i < num_thresholds; i++) {
		uint8_t j = 0;
		uint32_t num_actions = clnt_t[i].num_actions;
		struct override_action_t *actions = clnt_t[i].actions;
		for (j = 0; j < num_actions; j++) {
			if (actions[j].info == INT_MIN)
				return -1;
		}
	}

	return 0;
}

static int validate_client_thresholds(struct tm_instance_info *tm_instance_info,
				      struct override_threshold_t *clnt_t,
				      uint32_t num_thresholds,
				      unsigned int field_mask)
{
	uint8_t i = 0;
	uint8_t thresh_cnt = 0;
	uint8_t thresh_clr_cnt = 0;

	if (tm_instance_info == NULL ||
	    clnt_t == NULL ||
	    num_thresholds == 0 ||
	    num_thresholds > THRESHOLDS_MAX ||
	    field_mask == UNKNOWN_FIELD)
		return -1;

	struct tm_setting *tm_info = &tm_instance_info->setting->data.tm;

	if (num_thresholds != tm_info->num_thresholds)
		return -1;

	for (i = 0; i < num_thresholds; i++) {
		if (clnt_t[i].threshold_trig != INT_MIN)
			thresh_cnt++;

		if (clnt_t[i].threshold_clr != INT_MIN)
			thresh_clr_cnt++;
	}

	if (field_mask & THRESHOLDS_FIELD) {
		if (thresh_cnt != num_thresholds)
			return -1;
	}

	if (field_mask & THRESHOLDS_CLR_FIELD) {
		if (thresh_clr_cnt != num_thresholds)
			return -1;
	}

	for (i = 0; i < num_thresholds; i++) {

		if ((field_mask & THRESHOLDS_FIELD) &&
		    (field_mask & THRESHOLDS_CLR_FIELD)) {
			if (tm_info->descending_thresh) {
				if (clnt_t[i].threshold_trig >=
						clnt_t[i].threshold_clr) {
					msg("%s: Invalid thresholds and " \
					    "thresholds_clr request\n",
					    __func__);
					return -1;
				}
			} else {
				if (clnt_t[i].threshold_trig <=
						clnt_t[i].threshold_clr) {
					msg("%s: Invalid thresholds and " \
					    "thresholds_clr request\n",
					    __func__);
					return -1;
				}
			}
		}

		/* Validate if client requested only either thresholds or
		   thresholds_clr with current thresholds or threshold_clr */

		/* thresholds request only */
		if ((field_mask & THRESHOLDS_FIELD) &&
		    (!(field_mask & THRESHOLDS_CLR_FIELD))) {
			if (tm_info->descending_thresh) {
				if (tm_info->t[i].lvl_clr <=
						clnt_t[i].threshold_trig) {
					msg("%s: Invalid thresholds request\n",
					    __func__);
					return -1;
				}
			} else {
				if (tm_info->t[i].lvl_clr >=
						clnt_t[i].threshold_trig) {
					msg("%s: Invalid thresholds request\n",
					    __func__);
					return -1;
				}
			}
		}

		/* thresholds_clr request only */
		if ((field_mask & THRESHOLDS_CLR_FIELD) &&
		    (!(field_mask & THRESHOLDS_FIELD))) {
			if (tm_info->descending_thresh) {
				if (tm_info->t[i].lvl_trig >=
						clnt_t[i].threshold_clr) {
					msg("%s: Invalid thresholds_clr request\n",
					    __func__);
					return -1;
				}
			} else {
				if (tm_info->t[i].lvl_trig <= clnt_t[i].threshold_clr) {
					msg("%s: Invalid thresholds_clr request\n",
					    __func__);
					return -1;
				}
			}
		}
	}
	return 0;
}

static int handle_action_info_adjust (struct tm_instance_info *tm_instance_info,
				      struct monitor_settings *m_setting)
{
	uint8_t i = 0;

	if (tm_instance_info == NULL ||
	    m_setting == NULL)
		return -1;

	struct setting_info *setting = tm_instance_info->setting;
	struct tm_setting *tm_info = &(setting->data.tm);

	for (i = 0; i < m_setting->num_thresholds; i++) {
		uint8_t j = 0;
		for (j = 0; j < tm_info->t[i].num_actions; j++)
			tm_info->t[i].actions[j].info =
				m_setting->t[i].actions[j].info;
	}
	return 0;
}

static int handle_thresholds_adjust(struct tm_instance_info *tm_instance_info,
				    struct monitor_settings *m_setting,
				    enum supported_fields field)
{
	uint8_t i = 0;

	if (tm_instance_info == NULL ||
	    m_setting == NULL ||
	    field == UNKNOWN_FIELD)
		return -1;

	struct setting_info *setting = tm_instance_info->setting;
	struct tm_setting *tm_info = &(setting->data.tm);

	for (i = 0; i < m_setting->num_thresholds; i++) {
		if (field == THRESHOLDS_FIELD)
			tm_info->t[i].lvl_trig = m_setting->t[i].threshold_trig;
		if (field == THRESHOLDS_CLR_FIELD)
			tm_info->t[i].lvl_clr = m_setting->t[i].threshold_clr;
	}

	return 0;
}

/* return idx of algo  if success, negtaive on error */
static int validate_client_config(struct thermal_msg_data *client_config)
{
	uint32_t idx;

	for (idx = 0; idx < tm_cnt; idx++) {
		if (0 == strcmp(client_config->config.config_desc,
			         tm_states[idx].setting->desc))
			break;
	}
	if (idx >= tm_cnt) {
		msg("No matching config instance is configured %s\n",
		     client_config->config.config_desc);
		return -1;
	}

	if (client_config->config.fields_mask == UNKNOWN_FIELD) {
		msg("Unexpected fields flag for config request %s\n",
		     client_config->config.config_desc);
		return -1;
	}

	if (client_config->config.fields_mask & DISABLE_FIELD) {
		if (client_config->config.disable > 1) {
			msg("Invalid disable request from client %d\n",
			     client_config->config.disable);
			return -1;
		}
	}

	if (client_config->config.fields_mask & SAMPLING_FIELD) {
		if (client_config->config.sampling < SAMPLING_MS_MINIMUM) {
			msg("Invalid sampling request from client %d\n",
			     client_config->config.sampling);
			return -1;
		}
	}

	if (client_config->config.fields_mask & THRESHOLDS_FIELD) {
		struct monitor_settings *m_setting =
				&client_config->config.m_setting;
		if (validate_client_thresholds(&tm_states[idx], m_setting->t,
					       m_setting->num_thresholds,
					       client_config->config.fields_mask)) {
			msg("Invalid thresholds request from client\n");
			return -1;
		}
	}

	if (client_config->config.fields_mask & THRESHOLDS_CLR_FIELD) {
		struct monitor_settings *m_setting =
				&client_config->config.m_setting;
		if (validate_client_thresholds(&tm_states[idx], m_setting->t,
					       m_setting->num_thresholds,
					       client_config->config.fields_mask)) {
			msg("Invalid thresholds_clr request from client\n");
			return -1;
		}
	}

	if (client_config->config.fields_mask & ACTION_INFO_FIELD) {
		struct monitor_settings *m_setting =
				&client_config->config.m_setting;
		if (validate_client_actions_info(&tm_states[idx], m_setting->t,
						 m_setting->num_thresholds)) {
			msg("Invalid actions data request from client\n");
			return -1;
		}
	}

	return (int)idx;
}

static void current_monitor_setting_init(struct monitor_settings *curr)
{
	uint8_t i = 0;

	if (curr == NULL)
		return;

	curr->num_thresholds = 0;
	for (i = 0; i < THRESHOLDS_MAX; i++) {
		uint8_t j = 0;
		curr->t[i].threshold_trig = INT_MIN;
		curr->t[i].threshold_clr = INT_MIN;
		curr->t[i].num_actions = 0;
		for (j = 0; j < ACTIONS_MAX; j++) {
			curr->t[i].actions[j].info = INT_MIN;
			curr->t[i].actions[j].device[0] = '\0';
		}
	}
	return;
}

static int config_set_notify(int num_cfg_cnt, void *data, void *reserved)
{
	int a_idx[MAX_CONFIG_INSTANCES_SUPPORTED] = {0};
	int c_idx;
	uint8_t notification = 0;

	if (reserved == NULL ||
	    num_cfg_cnt <= 0 ||
	    (unsigned int)num_cfg_cnt > MAX_CONFIG_INSTANCES_SUPPORTED)
		return -1;

	pthread_mutex_lock(&wait_mutex);
	struct thermal_msg_data *current = (struct thermal_msg_data *)reserved;

	/* Check all monitor client configs are valid */
	for (c_idx = 0; c_idx < num_cfg_cnt; c_idx++) {

		if (current[c_idx].msg_type != MONITOR_ALGO_TYPE)
			continue;

		a_idx[c_idx] = validate_client_config(&current[c_idx]);
		if (a_idx[c_idx] < 0) {
			pthread_mutex_unlock(&wait_mutex);
			return -1;
		}
	}

	for (c_idx = 0; c_idx < num_cfg_cnt; c_idx++) {
		uint8_t disable_adjust_done = 0;
		uint8_t sampling_adjust_done = 0;
		uint8_t thresholds_adjust_done = 0;
		uint8_t thresholds_clr_adjust_done = 0;
		uint8_t action_info_adjust_done = 0;
		uint8_t curr_disabled = 0;
		uint32_t curr_sampling = 0;
		struct monitor_settings *m_setting;
		struct monitor_settings curr_setting;

		if (current[c_idx].msg_type != MONITOR_ALGO_TYPE)
			continue;

		current_monitor_setting_init(&curr_setting);
		struct tm_instance_info *tm_instance_info = &tm_states[a_idx[c_idx]];
		struct setting_info *setting = tm_instance_info->setting;
		struct tm_setting *tm_info = &(setting->data.tm);

		/* Enable/disable config instance adjustment */
		curr_disabled = tm_instance_info->disable;
		if ((current[c_idx].config.fields_mask & DISABLE_FIELD) &&
		    current[c_idx].config.disable != curr_disabled) {
			if (handle_disable_config_adjust(tm_instance_info,
						current[c_idx].config.disable)) {
				msg("%s: Config %s request failed for %s\n",
				    __func__,
				    current[c_idx].config.disable ?
					"disable":"enable",
				    setting->desc);
				goto error_handler;
			} else {
				disable_adjust_done = 1;
			}
		}

		/* set sampling adjustment if valid */
		curr_sampling = tm_info->sampling_period_ms;
		if ((current[c_idx].config.fields_mask & SAMPLING_FIELD) &&
		    current[c_idx].config.sampling != curr_sampling) {
			tm_info->sampling_period_ms = current[c_idx].config.sampling;
			sampling_adjust_done = 1;
		}

		/* thresholds adjustment */
		while (current[c_idx].config.fields_mask & THRESHOLDS_FIELD) {
			uint32_t i = 0;
			uint32_t count = 0;
			m_setting = &current[c_idx].config.m_setting;

			for (i = 0; i < m_setting->num_thresholds; i++) {
				if (m_setting->t[i].threshold_trig ==
						tm_info->t[i].lvl_trig)
					count++;
			}

			if (count == m_setting->num_thresholds)
				break;

			copy_current_monitor_setting(tm_info, &curr_setting,
						     THRESHOLDS_FIELD);
			if (handle_thresholds_adjust(tm_instance_info,
					&current[c_idx].config.m_setting,
					THRESHOLDS_FIELD)) {
				msg("%s: thresholds adjust failed\n", __func__);
				goto error_handler;
			} else {
				thresholds_adjust_done = 1;
				break;
			}
		}

		/* thresholds_clr adjustment */
		while (current[c_idx].config.fields_mask & THRESHOLDS_CLR_FIELD) {
			uint32_t i = 0;
			uint32_t count = 0;
			m_setting = &current[c_idx].config.m_setting;
			for (i = 0; i < m_setting->num_thresholds; i++) {
				if (m_setting->t[i].threshold_clr ==
						tm_info->t[i].lvl_clr)
					count++;
			}

			if (count == m_setting->num_thresholds)
				break;

			copy_current_monitor_setting(tm_info, &curr_setting,
						     THRESHOLDS_CLR_FIELD);
			if (handle_thresholds_adjust(tm_instance_info,
					&current[c_idx].config.m_setting,
					THRESHOLDS_CLR_FIELD)) {
				msg("%s: thresholds_clr adjust failed\n", __func__);
				goto error_handler;
			} else {
				thresholds_clr_adjust_done = 1;
				break;
			}
		}

		/* set action_info adjustment if valid */
		while (current[c_idx].config.fields_mask & ACTION_INFO_FIELD) {
			uint32_t i = 0;
			uint32_t j = 0;
			uint32_t cnt_action = 0;
			uint32_t cnt_threshold = 0;
			m_setting = &current[c_idx].config.m_setting;
			for (i = 0; i < m_setting->num_thresholds; i++) {
				for (j = 0; j < m_setting->t[i].num_actions; j++) {
					if ((m_setting->t[i].actions[j].info ==
							tm_info->t[i].lvl_clr))
						cnt_action++;
				}
				if (cnt_action == m_setting->t[i].num_actions)
					cnt_threshold++;
			}

			if (cnt_threshold == m_setting->num_thresholds)
				break;

			copy_current_monitor_setting(tm_info, &curr_setting,
						     ACTION_INFO_FIELD);
			if (handle_action_info_adjust(tm_instance_info,
					&current[c_idx].config.m_setting)) {
				msg("%s: action_info adjust failed\n", __func__);
				goto error_handler;
			} else {
				action_info_adjust_done = 1;
				break;
			}
		}

algo_notify:
		if (disable_adjust_done ||
		    sampling_adjust_done ||
		    thresholds_adjust_done ||
		    thresholds_clr_adjust_done ||
		    action_info_adjust_done) {
			THRESH_MASK_SET(a_idx[c_idx]);
			notification = 1;
		}
		continue;

error_handler:
		if (disable_adjust_done)
			handle_disable_config_adjust(tm_instance_info, curr_disabled);

		if (sampling_adjust_done)
			tm_info->sampling_period_ms = curr_sampling;

		if (thresholds_adjust_done)
			handle_thresholds_adjust(tm_instance_info, &curr_setting,
						 THRESHOLDS_FIELD);
		if (thresholds_clr_adjust_done)
			handle_thresholds_adjust(tm_instance_info, &curr_setting,
						 THRESHOLDS_CLR_FIELD);
		if (action_info_adjust_done)
			handle_action_info_adjust(tm_instance_info,
					&current[c_idx].config.m_setting);
		goto algo_notify;
	}
	if (notification)
		 pthread_cond_broadcast(&wait_cond);
	pthread_mutex_unlock(&wait_mutex);

	return 0;
}

static int config_query_notify(int algo_type, void *data, void *reserved)
{
	uint32_t i;
	struct thermal_msg_data query_data;

	if ((algo_type != MONITOR_ALGO_TYPE) && (algo_type != ALGO_IDX_MAX))
		return 0;

	/* Request for num of configs if reserved is 1 */
	if (reserved != NULL) {
		int *num_cfg_req = (int *)reserved;
		if (*num_cfg_req == 1)
			return (int)tm_cnt;
	}

	for (i = 0; i < tm_cnt; i++) {
		uint32_t j;
		memset(&query_data, 0, sizeof(struct thermal_msg_data));
		query_data.msg_type = MONITOR_ALGO_TYPE;
		strlcpy(query_data.client_name, CONFIG_QUERY_CLIENT, CLIENT_NAME_MAX);
		strlcpy(query_data.config.config_desc,tm_states[i].setting->desc,
		        MAX_ALGO_DESC);
		query_data.config.disable = tm_states[i].disable;
		query_data.config.sampling =
				tm_states[i].setting->data.tm.sampling_period_ms;
		query_data.config.m_setting.num_thresholds =
				 tm_states[i].setting->data.tm.num_thresholds;
		for (j = 0; j < query_data.config.m_setting.num_thresholds; j++) {
			uint32_t k;
			query_data.config.m_setting.t[j].threshold_trig =
					tm_states[i].setting->data.tm.t[j].lvl_trig;
			query_data.config.m_setting.t[j].threshold_clr =
					tm_states[i].setting->data.tm.t[j].lvl_clr;
			query_data.config.m_setting.t[j].num_actions =
					tm_states[i].setting->data.tm.t[j].num_actions;
			for (k= 0; k < query_data.config.m_setting.t[j].num_actions; k++) {
				query_data.config.m_setting.t[j].actions[k].info =
				     tm_states[i].setting->data.tm.t[j].actions[k].info;
				strlcpy(query_data.config.m_setting.t[j].actions[k].device,
				        tm_states[i].setting->data.tm.t[j].actions[k].device,
				        DEVICES_MAX_NAME_LEN);
			}
		}
		thermal_server_config_info_to_client(&query_data);
	}

	return 0;
}

static void sensor_thresh_notify(sensor_clnt_handle  clnt,
			   enum sensor_notify_event_type   event,
			   int                    reading,
			   void                  *data)
{
	if (NULL == clnt) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	if (((uintptr_t)data) >= tm_cnt) {
		msg("%s: unexpected idx %zd", __func__, (uintptr_t)data);
		return;
	}

	dbgmsg("%s: Update recieved %s %d", __func__,
	       tm_states[(uintptr_t)data].setting->desc,
	       reading);

	/* Notify the waiting thread */
	pthread_mutex_lock(&wait_mutex);
	THRESH_MASK_SET((uintptr_t)data);
	pthread_cond_broadcast(&wait_cond);
	pthread_mutex_unlock(&wait_mutex);
}

static void sensor_update_thresholds(struct tm_instance_info *setting,
				     int threshold_type, int level,
				     uint32_t idx)
{
	struct sensor_thresh_req ts_thresh_req;
	struct thresholds_req_t *thresh = &ts_thresh_req.thresh;
	struct tm_setting *tm_info = NULL;

	if (setting == NULL  ||
	    setting->ts_clnt == NULL) {
		msg("%s: Unexpected NULL", __func__);
		return;
	}

	tm_info = &(setting->setting->data.tm);
	memset(&ts_thresh_req, 0, sizeof(struct sensor_thresh_req));

	ts_thresh_req.notify_cb_func = sensor_thresh_notify;
	ts_thresh_req.notify_cb_data = (void *)(uintptr_t)idx;
	ts_thresh_req.polling_interval_valid = 1;
	ts_thresh_req.polling_interval = tm_info->sampling_period_ms;
	thresh->high_valid = 1;
	thresh->low_valid = 1;

	dbgmsg("%s: TM Id %s Sensor %s threshold_type %d, level %d",
	       __func__, setting->setting->desc, tm_info->sensor,
	       threshold_type, level);
	if (tm_info->descending_thresh) {
		/* Falling Trigger */
		if ((uint32_t)level >= tm_info->num_thresholds) {
			/* handle corner low case */
			thresh->low = tm_info->t[tm_info->num_thresholds - 1].lvl_trig;
			thresh->low_valid = 0;
		} else
			thresh->low = tm_info->t[level].lvl_trig;

		if (level <= 0) {
			/* handle corner high case */
			thresh->high = tm_info->t[0].lvl_clr;
			thresh->high_valid = 0;
		} else
			thresh->high = tm_info->t[level - 1].lvl_clr;
	} else {
		/* Rising trigger */
		if ((uint32_t)level >= tm_info->num_thresholds) {
			/* handle corner high case */
			thresh->high = tm_info->t[tm_info->num_thresholds - 1].lvl_trig;
			thresh->high_valid = 0;
		} else
			thresh->high = tm_info->t[level].lvl_trig;

		if (level <= 0) {
			/* handle corner low case */
			thresh->low = tm_info->t[0].lvl_clr;
			thresh->low_valid = 0;
		} else
			thresh->low = tm_info->t[level - 1].lvl_clr;

		if (override_mode) {
			thresh->high += tm_info->override;
			thresh->low += tm_info->override;
		}
	}
	sensors_manager_set_thresh_lvl(setting->ts_clnt, &ts_thresh_req);
}

static void clear_all_alarms(struct tm_instance_info *sensor)
{
	uint32_t i;

	for (i = 0; i < MAX_ACTIONS_PER_TM_INSTANCE; i++) {
		if (sensor->dev_clnt_list[i] == NULL)
			continue;

		/* check if action may have been set */
		if ((sensor->action_mask & (1U << i)) == 0)
			continue;

		switch(sensor->dev_info_list[i]->dev_type) {
		case DEVICE_GENERIC_TYPE:
		case DEVICE_OP_VALUE_TYPE:
			device_clnt_cancel_request(sensor->dev_clnt_list[i]);
			break;
		default:
			dbgmsg("%s: No clearing of action %s\n", __func__,
			       sensor->dev_info_list[i]->name);
		}
	}

	sensor->action_mask = 0;
}

static void handle_thresh_sig(void)
{
	uint32_t j;
	int i;
	uint32_t idx;
	int max_thr;
	int alarm_raised = 0, alarm_cleared = 0;
	int sensor_temp = 0;
	int lvl_max, lvl_min;
	int threshold_type = THRESHOLD_NOCHANGE;
	int threshold_level = 0;
	int action_info, action_idx;
	struct device_info *dev_info;
	struct tm_instance_info *sensor;
	struct setting_info *info;
	struct tm_setting *tm_info;
	union device_request req;

	/* Get temp and handle */
	for (idx = 0; idx < tm_cnt; idx++) {
		if (THRESH_MASK_IS_SET(idx) == 0) {
			continue;
		}

		sensor = &tm_states[idx];
		info = sensor->setting;
		tm_info = &(info->data.tm);

		if ((tm_info->num_thresholds < 1) ||
		    (sensor->disable)) {
			/* Unmask TM instance as handled */
			THRESH_MASK_CLR(idx);
			continue;
		}

		max_thr = (int)tm_info->num_thresholds;
		sensor_temp = sensor_get_temperature(sensor);
		dbgmsg("%s: TM Id %s Sensor %s Temp %d\n", __func__,
		       info->desc, tm_info->sensor, sensor_temp);

		lvl_max = -1;
		lvl_min = INT_MAX;
		for (i = max_thr - 1; i >= 0; i--) {
			/* Scan for new alarm conditions */
			if (sensor_threshold_trigger(sensor_temp, sensor, i)) {
				if (sensor->lvl_alarm[i] == 0) {
					info("TM Id '%s' Sensor '%s' - alarm raised %d at %d.%d degC\n",
					    info->desc, tm_info->sensor, i + 1,
					    RCONV(sensor_temp),
					    (sensor_temp%1000)/100);
					sensor->lvl_alarm[i] = 1;
					alarm_raised = 1;
				}
				if (i > lvl_max)
					lvl_max = i;
			}
			/* Scan for alarm clearing conditions */
			if (sensor_threshold_clear(sensor_temp, sensor, i)) {
				if (sensor->lvl_alarm[i] == 1) {
					info("TM Id '%s' Sensor '%s' - alarm cleared %d at %d.%d degC\n",
					    info->desc, tm_info->sensor, i + 1,
					    RCONV(sensor_temp),
					    (sensor_temp%1000)/100);
					sensor->lvl_alarm[i] = 0;
					alarm_cleared = 1;
				}
				if (i < lvl_min)
					lvl_min = i;
			}
		}

		/* Update temperature thresholds */
		if (alarm_raised) {
			threshold_type = THRESHOLD_CROSS;
			threshold_level = lvl_max + 1;
		} else if (alarm_cleared) {
			threshold_type = THRESHOLD_CLEAR;
			threshold_level = lvl_min;
		} else {
			threshold_type = THRESHOLD_NOCHANGE;
			threshold_level = sensor->last_lvl;
		}
		sensor->last_lvl = threshold_level;

		pthread_mutex_lock(&wait_mutex);
		/* Unmask TM instance as handled */
		THRESH_MASK_CLR(idx);
		pthread_mutex_unlock(&wait_mutex);

		sensor_update_thresholds(sensor, threshold_type,
					 threshold_level, idx);

		if (!alarm_raised && !alarm_cleared) {
			continue;
		}

		/* Perform actions on highest level alarm */
		for (i = max_thr - 1; i >= 0; i--) {
			if (sensor->lvl_alarm[i] == 0)
				continue;

			for (j = 0; j < tm_info->t[i].num_actions; j++) {
				action_idx = tm_info->t[i].actions[j].device_idx;
				dev_info = sensor->dev_info_list[action_idx];
				if (dev_info == NULL)
					continue;

				action_info = tm_info->t[i].actions[j].info;
				dbgmsg ("j=%d i=%d TM Id %s Sensor %s: Action %s value %d\n",
					j, i, info->desc, tm_info->sensor,
					dev_info->name, action_info);
				sensor->action_mask |= (1U << action_idx);
				req.value = action_info;
				switch(dev_info->dev_type) {
				case DEVICE_DIRECT_ACTION_TYPE:
					if (alarm_raised &&
					    (strncasecmp(dev_info->name,
							"report",
							DEVICES_MAX_NAME_LEN) == 0))
						report_msg(sensor->dev_clnt_list[action_idx],
							   tm_info->sensor, sensor_temp, i, 1);
					else
						device_clnt_request(sensor->dev_clnt_list[action_idx],
								    &req);
					break;
				case DEVICE_OP_VALUE_TYPE:
				case DEVICE_GENERIC_TYPE:
					device_clnt_request(sensor->dev_clnt_list[action_idx], &req);
					break;
				case DEVICE_NONE_TYPE:
					break;
				default:
					msg("Unknown action %s\n", dev_info->name);
				}
			}

			break;
		}

		if (alarm_cleared) {
			/* Handle alarm clearing cases */
			if (lvl_min == 0) {
				dbgmsg("Clearing all alarms\n");
				clear_all_alarms(sensor);
			}
			/* report threshold clearing */
			for (j = 0; j < tm_info->t[lvl_min].num_actions; j++) {
				action_idx = tm_info->t[lvl_min].actions[j].device_idx;
				dev_info = sensor->dev_info_list[action_idx];
				if (strncasecmp(dev_info->name, "report",
						DEVICES_MAX_NAME_LEN) == 0) {
					report_msg(sensor->dev_clnt_list[action_idx],
						   tm_info->sensor, sensor_temp,
						   lvl_min, 0);
					break;
				}
			}
		}
		alarm_raised = 0;
		alarm_cleared = 0;
	}
}

static void *sensor_monitor(void *data)
{
	uint32_t idx;

	for (idx = 0; idx < tm_cnt; idx++)
		THRESH_MASK_SET(idx);

	/* Set initial thresholds */
	handle_thresh_sig();

	/* Vote okay to disable kernel mitigation */
	device_clnt_cancel_request(kernel_dev);

	thermal_server_register_client_req_handler("override", override_notify, NULL);
	thermal_server_register_client_req_handler(CONFIG_QUERY_CLIENT, config_query_notify, NULL);
	thermal_server_register_client_req_handler(CONFIG_SET_CLIENT, config_set_notify, NULL);

	while (exit_daemon != 1) {
		dbgmsg("%s: Wait for EV", __func__);
		pthread_mutex_lock(&wait_mutex);
		if (!THRESH_MASK_ANY_SET) {
			pthread_cond_wait(&wait_cond, &wait_mutex);
		}
		pthread_mutex_unlock(&wait_mutex);

		dbgmsg("%s: Thresh EVT", __func__);
		handle_thresh_sig();
	}

	free(device_info_arr);
	return NULL;
}

void thermal_monitor(struct thermal_setting_t *settings)
{
	struct setting_info *cfg_setting;
	union device_request req;

	/* Build Device Info List */
	if (devices_manager_get_list(NULL, &device_info_arr_len)) {
		msg("Failed to get device list length\n");
		return;
	}

	device_info_arr = (struct device_info *)
		malloc(sizeof(struct device_info)*device_info_arr_len);

	if (device_info_arr == NULL) {
		msg("Failed to alloc device_info_arr\n");
		return;
	}

	if (devices_manager_get_list(device_info_arr,
				     &device_info_arr_len)) {
		msg("Failed to get device list\n");
		free(device_info_arr);
		return;
	}

	cfg_setting = settings->list;

	while (cfg_setting && (tm_cnt < MAX_TM_INSTANCES_SUPPORTED)) {
		if ((cfg_setting->algo_type != MONITOR_ALGO_TYPE) ||
		    (cfg_setting->err_disable != 0)) {
			cfg_setting = cfg_setting->next;
			continue;
		}

		dbgmsg("%s: Import %s", __func__, cfg_setting->desc);
		tm_states[tm_cnt].setting = cfg_setting;
		tm_states[tm_cnt].disable = cfg_setting->disable;
		tm_cnt++;

		if (!cfg_setting->disable)
			print_setting(cfg_setting);

		/* KEEP at end of while block */
		cfg_setting = cfg_setting->next;
	}

	if (!sensors_setup()) {
		msg("Failed to setup at least one sensor for monitoring\n");
		return;
	}

	/* Vote to keep kernel mitigation enabled until thermal monitor has
	   processed initial thresholds. */
	kernel_dev = devices_manager_reg_clnt("kernel");
	if (kernel_dev == NULL) {
		msg("%s Failed to create kernel device handle\n", __func__);
		return;
	}
	req.value = 1;
	device_clnt_request(kernel_dev, &req);

	if (pthread_create(&tm_thread, NULL, (void *)&sensor_monitor,
			   (void *)NULL) != 0) {
		msg("Error initializing thermal monitor\n");
		device_clnt_cancel_request(kernel_dev);
	}
}
