/*===========================================================================

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include "thermal.h"
#include "thermal_config.h"

#define HOTPLUG_HIGH_THRESHOLD (100000)
#define HOTPLUG_LOW_THRESHOLD  (HOTPLUG_HIGH_THRESHOLD - 20000)
#define HOTPLUG_HIGH_THRESHOLD_8064 (115000)
#define HOTPLUG_LOW_THRESHOLD_8064  (HOTPLUG_HIGH_THRESHOLD_8064 - 20000)
#define HOTPLUG_HIGH_THRESHOLD_8930 (110000)
#define HOTPLUG_LOW_THRESHOLD_8930  (HOTPLUG_HIGH_THRESHOLD_8930 - 20000)

#define HYBRID_POLLING_THRESH 65000
#define HYBRID_POLLING_THRESH_CLR 62000

#define HYBRID_POLLING_THRESH_8064 70000
#define HYBRID_POLLING_THRESH_CLR_8064 67000

#define VDD_RESTRICTION_THRESH 5000
#define VDD_RESTRICTION_THRESH_CLR 10000

#define OPT_CURRENT_REQ_THRESH 85000
#define OPT_CURRENT_REQ_THRESH_CLR 75000

#define VDD_DIG_AUTOMODE_DISABLE_THRESH 85000
#define VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR 75000

/* PMIC 8821 alarms */
/* State 1 Alarm thresholds */
#define PMIC_ALARM_STAGE_1_TEMP_TRIG  107000
#define PMIC_ALARM_STAGE_1_TEMP_CLR  103000

/* Stage 2 Alarm thresholds */
#define PMIC_ALARM_STAGE_2_TEMP_TRIG  127000
#define PMIC_ALARM_STAGE_2_TEMP_CLR  123000


static struct setting_info tm_cfgs_8064[] =
{
	{
		.desc = "HOTPLUG-CPU1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HOTPLUG_HIGH_THRESHOLD_8064,
				.lvl_clr = HOTPLUG_LOW_THRESHOLD_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_1",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "HOTPLUG-CPU2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HOTPLUG_HIGH_THRESHOLD_8064,
				.lvl_clr = HOTPLUG_LOW_THRESHOLD_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_2",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "HOTPLUG-CPU3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HOTPLUG_HIGH_THRESHOLD_8064,
				.lvl_clr = HOTPLUG_LOW_THRESHOLD_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_3",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "HYBRID-THRESH",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH_8064,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU0-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH_8064,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU1-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH_8064,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU2-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH_8064,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU3-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH_8064,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-POPMEM-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pop_mem",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH_8064,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR_8064,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "PM8821-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pm8821_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 4,
				.actions = {
					[0] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[2] = {
						.device = "cpu2",
						.info = 918000,
					},
					[3] = {
						.device = "cpu3",
						.info = 918000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 2,
				.actions = {
					[0] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8960[] =
{
	{
		.desc = "HOTPLUG-CPU1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HOTPLUG_HIGH_THRESHOLD,
				.lvl_clr = HOTPLUG_LOW_THRESHOLD,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_1",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "HYBRID-THRESH",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU0-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU1-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-POPMEM-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pop_mem",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8930[] =
{
	{
		.desc = "HOTPLUG-CPU1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HOTPLUG_HIGH_THRESHOLD_8930,
				.lvl_clr = HOTPLUG_LOW_THRESHOLD_8930,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_1",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "HYBRID-THRESH",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU0-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-CPU1-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "HYBRID-POPMEM-POLLING",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pop_mem",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = HYBRID_POLLING_THRESH,
				.lvl_clr = HYBRID_POLLING_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "none",
					.info = 0,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8974[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pm8841_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[3] = {
						.device = "cpu0",
						.info = 960000,
					},
					[4] = {
						.device = "cpu1",
						.info = 960000,
					},
					[5] = {
						.device = "cpu2",
						.info = 960000,
					},
					[6] = {
						.device = "cpu3",
						.info = 960000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 3,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8974_pro_ab[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pm8841_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[3] = {
						.device = "cpu0",
						.info = 960000,
					},
					[4] = {
						.device = "cpu1",
						.info = 960000,
					},
					[5] = {
						.device = "cpu2",
						.info = 960000,
					},
					[6] = {
						.device = "cpu3",
						.info = 960000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 3,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8084[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pma8084_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[3] = {
						.device = "cpu0",
						.info = 960000,
					},
					[4] = {
						.device = "cpu1",
						.info = 960000,
					},
					[5] = {
						.device = "cpu2",
						.info = 960000,
					},
					[6] = {
						.device = "cpu3",
						.info = 960000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 3,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8092[] =
{
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pma8084_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[3] = {
						.device = "cpu0",
						.info = 960000,
					},
					[4] = {
						.device = "cpu1",
						.info = 960000,
					},
					[5] = {
						.device = "cpu2",
						.info = 960000,
					},
					[6] = {
						.device = "cpu3",
						.info = 960000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 3,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8974_pro_ac[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pma8084_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[3] = {
						.device = "cpu0",
						.info = 960000,
					},
					[4] = {
						.device = "cpu1",
						.info = 960000,
					},
					[5] = {
						.device = "cpu2",
						.info = 960000,
					},
					[6] = {
						.device = "cpu3",
						.info = 960000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 3,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "OPT_CURR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = OPT_CURRENT_REQ_THRESH,
				.lvl_clr = OPT_CURRENT_REQ_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "opt_curr_req",
					.info = 1,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8x62[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_DIG_SWMODE_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_DIG_AUTOMODE_DISABLE_THRESH,
				.lvl_clr = VDD_DIG_AUTOMODE_DISABLE_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_dig_swmode",
					.info = 1,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8226_v1[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	/* Disable TSENS4/TSENS5 for vdd min monitor for bring up because
	 * it always returns 0 */
	/*
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	*/
};

static struct setting_info tm_cfgs_8226_v2[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8610[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_9625[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_9900[] =
{
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pma8084_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[3] = {
						.device = "cpu0",
						.info = 960000,
					},
					[4] = {
						.device = "cpu1",
						.info = 960000,
					},
					[5] = {
						.device = "cpu2",
						.info = 960000,
					},
					[6] = {
						.device = "cpu3",
						.info = 960000,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 3,
				.actions = {
					[0] = {
						.device = "hotplug_1",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_3",
						.info = 1,
					},
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8994[] =
{
	{
		.desc = "VDD_RSTR_MONITOR-TSENS0",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor0",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS1",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS5",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS6",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS7",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS8",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor8",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS9",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor9",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS10",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor10",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS11",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor11",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS12",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor12",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS13",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor13",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS14",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor14",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "VDD_RSTR_MONITOR-TSENS15",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor15",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			.descending_thresh = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = VDD_RESTRICTION_THRESH,
				.lvl_clr = VDD_RESTRICTION_THRESH_CLR,
				.num_actions = 1,
				.actions[0] = {
					.device = "vdd_restriction",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "PMIC-ALARM-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "pm8994_tz",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = PMIC_ALARM_STAGE_1_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_1_TEMP_CLR,
				.num_actions = 9,
				.actions = {
					[0] = {
						.device = "hotplug_7",
						.info = 0,
					},
					[1] = {
						.device = "hotplug_6",
						.info = 0,
					},
					[2] = {
						.device = "hotplug_5",
						.info = 0,
					},
					[3] = {
						.device = "hotplug_4",
						.info = 0,
					},
					[4] = {
						.device = "hotplug_3",
						.info = 0,
					},
					[5] = {
						.device = "hotplug_2",
						.info = 0,
					},
					[6] = {
						.device = "hotplug_1",
						.info = 0,
					},
					[7] = {
						.device = "cluster1",
						.info = 302400,
					},
					[8] = {
						.device = "cluster0",
						.info = 302400,
					},
				},
			},
			.t[1] = {
				.lvl_trig = PMIC_ALARM_STAGE_2_TEMP_TRIG,
				.lvl_clr = PMIC_ALARM_STAGE_2_TEMP_CLR,
				.num_actions = 7,
				.actions = {
					[0] = {
						.device = "hotplug_7",
						.info = 1,
					},
					[1] = {
						.device = "hotplug_6",
						.info = 1,
					},
					[2] = {
						.device = "hotplug_5",
						.info = 1,
					},
					[3] = {
						.device = "hotplug_4",
						.info = 1,
					},
					[4] = {
						.device = "hotplug_3",
						.info = 1,
					},
					[5] = {
						.device = "hotplug_2",
						.info = 1,
					},
					[6] = {
						.device = "hotplug_1",
						.info = 1,
					},
				},
			},
		},
	},
	{
		.desc = "CPU5_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu5",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 75000 ,
				.lvl_clr = 45000,
				.num_actions = 3,
				.actions[0] = {
					.device = "hotplug_5",
					.info = 1,
				},
				.actions[1] = {
					.device = "hotplug_6",
					.info = 1,
				},
				.actions[2] = {
					.device = "hotplug_7",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "CPU6_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu6",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 75000 ,
				.lvl_clr = 45000,
				.num_actions = 3,
				.actions[0] = {
					.device = "hotplug_5",
					.info = 1,
				},
				.actions[1] = {
					.device = "hotplug_6",
					.info = 1,
				},
				.actions[2] = {
					.device = "hotplug_7",
					.info = 1,
				},
			},
		},
	},
	{
		.desc = "CPU7_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu7",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 75000 ,
				.lvl_clr = 45000,
				.num_actions = 3,
				.actions[0] = {
					.device = "hotplug_5",
					.info = 1,
				},
				.actions[1] = {
					.device = "hotplug_6",
					.info = 1,
				},
				.actions[2] = {
					.device = "hotplug_7",
					.info = 1,
				},
			},
		},
	},
};

void thermal_monitor_init_data(struct thermal_setting_t *setting)
{
	int i;
	int arr_size = 0;

	struct setting_info *cfg = NULL;

	switch (therm_get_msm_id()) {
		case THERM_MSM_8960:
			cfg = tm_cfgs_8960;
			arr_size = ARRAY_SIZE(tm_cfgs_8960);
			break;
		case THERM_MSM_8930:
			cfg = tm_cfgs_8930;
			arr_size = ARRAY_SIZE(tm_cfgs_8930);
			break;
		case THERM_MSM_8064AB:
		case THERM_MSM_8064:
			cfg = tm_cfgs_8064;
			arr_size = ARRAY_SIZE(tm_cfgs_8064);
			break;
		case THERM_MSM_8974:
			cfg = tm_cfgs_8974;
			arr_size = ARRAY_SIZE(tm_cfgs_8974);
			break;
		case THERM_MSM_8x62:
			cfg = tm_cfgs_8x62;
			arr_size = ARRAY_SIZE(tm_cfgs_8x62);
			break;
		case THERM_MSM_8974PRO_AA:
		case THERM_MSM_8974PRO_AB:
			cfg = tm_cfgs_8974_pro_ab;
			arr_size = ARRAY_SIZE(tm_cfgs_8974_pro_ab);
			break;
		case THERM_MSM_8974PRO_AC:
			cfg = tm_cfgs_8974_pro_ac;
			arr_size = ARRAY_SIZE(tm_cfgs_8974_pro_ac);
			break;
		case THERM_MSM_8084:
			cfg = tm_cfgs_8084;
			arr_size = ARRAY_SIZE(tm_cfgs_8084);
			break;
		case THERM_MPQ_8092:
			cfg = tm_cfgs_8092;
			arr_size = ARRAY_SIZE(tm_cfgs_8092);
			break;
		case THERM_MSM_8226:
			switch (therm_get_msm_version()) {
				case THERM_VERSION_V1:
					cfg = tm_cfgs_8226_v1;
					arr_size = ARRAY_SIZE(tm_cfgs_8226_v1);
					break;
				default:
					cfg = tm_cfgs_8226_v2;
					arr_size = ARRAY_SIZE(tm_cfgs_8226_v2);
					break;
			}
			break;
		case THERM_MSM_8926:
			cfg = tm_cfgs_8226_v2;
			arr_size = ARRAY_SIZE(tm_cfgs_8226_v2);
			break;
		case THERM_MSM_8610:
			cfg = tm_cfgs_8610;
			arr_size = ARRAY_SIZE(tm_cfgs_8610);
			break;
		case THERM_MDM_9625:
		case THERM_MDM_9635:
			cfg = tm_cfgs_9625;
			arr_size = ARRAY_SIZE(tm_cfgs_9625);
			break;
		case THERM_FSM_9900:
			cfg = tm_cfgs_9900;
			arr_size = ARRAY_SIZE(tm_cfgs_9900);
			break;
		case THERM_MSM_8994:
			cfg = tm_cfgs_8994;
			arr_size = ARRAY_SIZE(tm_cfgs_8994);
			break;
		default:
			msg("%s: ERROR Uknown device\n", __func__);
			cfg = NULL;
			arr_size = 0;
			break;
	}

	if (cfg != NULL) {
		for (i = 0; i < arr_size; i++) {
			add_setting(setting, &cfg[i]);
		}
	}
}
