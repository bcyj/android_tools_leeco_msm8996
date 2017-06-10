/*===========================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __THERMAL_CONFIG_H__
#define __THERMAL_CONFIG_H__

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "devices_manager.h"
#include "sensors_manager.h"

#define SAMPLING_MS_DEFAULT  (5000U)
#ifdef SENSORS_8960
#define SAMPLING_MS_MINIMUM  (100U)
#else
#define SAMPLING_MS_MINIMUM  (2U)
#endif

/* Below few macros are duplicated in
   server/thermal_lib_common.h */
#define THRESHOLDS_MAX       (8U)
#define ACTIONS_MAX          (32U)
#define INTEGERS_MAX         (4U)
#define MAX_ALGO_DESC        (32U)
#define LOAD_VIRTUAL_FLAG    (1U)
#define LOAD_ALL_FLAG        (0U)
#define PRINT_TO_INFO        (0U)
#define PRINT_TO_STDOUT      (1U)

enum algo_type {
	UNKNOWN_ALGO_TYPE = -1,
	MONITOR_ALGO_TYPE = 0,
	PID_ALGO_TYPE,
	SS_ALGO_TYPE,
	EQUILIBRIUM_TYPE,
	VIRTUAL_SENSOR_TYPE,
	ALGO_IDX_MAX,
};

struct action_t {
	char device[DEVICES_MAX_NAME_LEN];
	int  info;
	int  device_idx;
};

struct threshold_t {
	int lvl_trig;
	int lvl_clr;
	struct action_t actions[ACTIONS_MAX];
	uint32_t num_actions;
};

struct pid_setting {
	char sensor[MAX_SENSOR_NAME_LEN];
	char device[DEVICES_MAX_NAME_LEN];
	int  override;
	uint32_t sampling_period_ms;
	int  i_samples;
	int  set_point;
	int  set_point_clr;
	int  units_per_C;
	float freq_scale;
	float p_const;
	float i_const;
	float d_const;
	float err_weight;
};

struct tm_setting {
	char    sensor[MAX_SENSOR_NAME_LEN];
	uint32_t sampling_period_ms;
	uint32_t  num_thresholds;
	int     override;
	uint8_t descending_thresh;
	struct  threshold_t t[THRESHOLDS_MAX];

	/* internal counters used during config parsing */
	uint32_t _n_thresholds;
	uint32_t _n_to_clear;
	uint32_t _n_actions;
	uint32_t _n_action_info;
};

struct ss_setting {
	char sensor[MAX_SENSOR_NAME_LEN];
	char device[DEVICES_MAX_NAME_LEN];
	int  override;
	uint32_t  sampling_period_ms;
	int  set_point;
	int  set_point_clr;
	int  time_constant;
	int  device_mtgn_max_limit;
};

struct equilibrium_setting {
	char   **sensor_list;
	uint8_t  list_cnt;
	uint8_t  list_size;
	#ifdef ENABLE_OLD_PARSER
	char    *sensor;
	#else
	char     sensor[MAX_SENSOR_NAME_LEN];
	#endif
	uint32_t sensor_delta;
	uint32_t sampling_period_ms;
	uint32_t temp_range;
	int      max_temp;
	int      offset;
	uint32_t      long_sampling_period_ms;
	uint32_t      long_sampling_cnt;
	uint32_t      long_sample_range;
	uint32_t      pre_cal_delay_ms;
	uint32_t      sampling_list[INTEGERS_MAX];
	uint32_t      temp_range_list[INTEGERS_MAX];
};

struct virtual_setting {
	#ifdef ENABLE_OLD_PARSER
	char *trip_sensor;
	#else
	char trip_sensor[MAX_SENSOR_NAME_LEN];
	#endif
	char **sensor_list;
	/* Keeps track of number of sensors in sensor_list */
	uint8_t list_cnt;
	/* Keeps track of number of allocated sensors in sensor_list */
	uint8_t list_size;
	int set_point;
	int set_point_clr;
	uint32_t sampling_period_ms;
	int *weight_list;
	uint32_t weight_cnt;
	uint32_t weight_size;
};

union algo_data {
	struct pid_setting pid;
	struct tm_setting tm;
	struct ss_setting ss;
	struct equilibrium_setting eq;
	struct virtual_setting v;
};

struct setting_info {
	char desc[MAX_ALGO_DESC];
	int algo_type;
	uint8_t disable;
	uint8_t err_disable;
	uint8_t dynamic;  /* dynamic != 0 if allocated using malloc. */
	struct setting_info *next;
	union algo_data data;
};

struct thermal_setting_t {
	uint32_t sample_period_ms;
	struct setting_info *list;
};

void init_settings(struct thermal_setting_t *settings);
#ifdef ENABLE_OLD_PARSER
int  load_config(struct thermal_setting_t *settings, const char *pFName);
#else
int  load_config(struct thermal_setting_t *settings, const char *pFName, int flag);
#endif
void add_setting(struct thermal_setting_t *settings, struct setting_info *info);
void print_setting(struct setting_info *setting);
void print_settings(struct thermal_setting_t *settings);

#endif  /* __THERMAL_CONFIG_H__ */
