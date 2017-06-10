/*===========================================================================

Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include "thermal.h"
#include "thermal_config.h"

#define SHUTDOWN_MONITOR_THRESH 120000
#define SHUTDOWN_MONITOR_THRESH_CLR 115000
#define VDD_RESTRICTION_THRESH 5000
#define VDD_RESTRICTION_THRESH_CLR 10000

static struct setting_info tm_cfgs_8916[] =
{
	{
		.desc = "CPU0-1_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0-1",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 97000,
				.lvl_clr =  92000,
				.num_actions = 1,
				.actions[0] = {
					.device = "shutdown",
					.info = 0,
				},
			},
		},
	},
	{
		.desc = "CPU2-3_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu2-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 97000,
				.lvl_clr = 92000,
				.num_actions = 1,
				.actions[0] = {
					.device = "shutdown",
					.info = 0,
				},
			},
		},
	},
};

static struct setting_info tm_cfgs_8939[] =
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
		.desc = "CPU3_HOTPLUG_MONITOR",
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
				.lvl_trig = 85000,
				.lvl_clr = 55000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_3",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CAMERA_CAMCORDER_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 85000,
				.lvl_clr = 80000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 1,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 1,
				},
			},
			.t[1] = {
				.lvl_trig = 90000,
				.lvl_clr = 85000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 2,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 2,
				},
			}
		},
	},
};

static struct setting_info tm_cfgs_8909[] =
{
	{
		.desc = "CPU3_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 85000,
				.lvl_clr = 80000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_3",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CPU2_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0-2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 88000,
				.lvl_clr = 85000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_2",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CPU1_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_1",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CAMERA_CAMCORDER_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 250,
			.num_thresholds = 3,
			._n_thresholds = 3,
			._n_to_clear = 3,
			._n_actions = 3,
			._n_action_info = 3,
			.t[0] = {
				.lvl_trig = 80000,
				.lvl_clr = 75000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 1,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 1,
				},
			},
			.t[1] = {
				.lvl_trig = 85000,
				.lvl_clr = 80000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 2,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 2,
				},
			},
			.t[2] = {
				.lvl_trig = 88000,
				.lvl_clr = 85000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 10,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
	{
		.desc = "CX_MITIGATION_MONITOR_TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 250,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 2,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 1,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 1,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 1,
				},
				.actions[5] = {
					.device = "camera",
					.info = 0,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 0,
				},
			},
			.t[1] = {
				.lvl_trig = 100000,
				.lvl_clr = 92000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 3,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 3,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 3,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 4,
				},
				.actions[5] = {
					.device = "camera",
					.info = 10,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
	{
		.desc = "CX_MITIGATION_MONITOR_TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 250,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 2,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 1,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 1,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 1,
				},
				.actions[5] = {
					.device = "camera",
					.info = 0,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 0,
				},
			},
			.t[1] = {
				.lvl_trig = 100000,
				.lvl_clr = 92000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 3,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 3,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 3,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 4,
				},
				.actions[5] = {
					.device = "camera",
					.info = 10,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
	{
		.desc = "CX_MITIGATION_MONITOR_TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 250,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 2,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 1,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 1,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 1,
				},
				.actions[5] = {
					.device = "camera",
					.info = 0,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 0,
				},
			},
			.t[1] = {
				.lvl_trig = 100000,
				.lvl_clr = 92000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 3,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 3,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 3,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 4,
				},
				.actions[5] = {
					.device = "camera",
					.info = 10,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
};

static struct setting_info tm_cfgs_8909_qrd_hq[] =
{
	{
		.desc = "BATTERY-CHARGE-MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 1000,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 50000,
				.lvl_clr = 45000,
				.num_actions = 1,
				.actions[0] = {
					.device = "battery",
					.info = 1,
				},
			},
			.t[1] = {
				.lvl_trig = 55000,
				.lvl_clr = 50000,
				.num_actions = 1,
				.actions[0] = {
					.device = "battery",
					.info = 2,
				},
			}
		},
	},
	{
		.desc = "CPU3_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 85000,
				.lvl_clr = 80000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_3",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CPU2_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0-2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 88000,
				.lvl_clr = 85000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_2",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CPU1_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_1",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CAMERA_CAMCORDER_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor1",
			.sampling_period_ms = 250,
			.num_thresholds = 3,
			._n_thresholds = 3,
			._n_to_clear = 3,
			._n_actions = 3,
			._n_action_info = 3,
			.t[0] = {
				.lvl_trig = 80000,
				.lvl_clr = 75000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 1,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 1,
				},
			},
			.t[1] = {
				.lvl_trig = 85000,
				.lvl_clr = 80000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 2,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 2,
				},
			},
			.t[2] = {
				.lvl_trig = 88000,
				.lvl_clr = 85000,
				.num_actions = 2,
				.actions[0] = {
					.device = "camera",
					.info = 10,
				},
				.actions[1] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
	{
		.desc = "CX_MITIGATION_MONITOR_TSENS2",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor2",
			.sampling_period_ms = 250,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 2,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 1,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 1,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 1,
				},
				.actions[5] = {
					.device = "camera",
					.info = 0,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 0,
				},
			},
			.t[1] = {
				.lvl_trig = 100000,
				.lvl_clr = 92000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 3,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 3,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 3,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 4,
				},
				.actions[5] = {
					.device = "camera",
					.info = 10,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
	{
		.desc = "CX_MITIGATION_MONITOR_TSENS3",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor3",
			.sampling_period_ms = 250,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 2,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 1,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 1,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 1,
				},
				.actions[5] = {
					.device = "camera",
					.info = 0,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 0,
				},
			},
			.t[1] = {
				.lvl_trig = 100000,
				.lvl_clr = 92000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 3,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 3,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 3,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 4,
				},
				.actions[5] = {
					.device = "camera",
					.info = 10,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
	{
		.desc = "CX_MITIGATION_MONITOR_TSENS4",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "tsens_tz_sensor4",
			.sampling_period_ms = 250,
			.num_thresholds = 2,
			._n_thresholds = 2,
			._n_to_clear = 2,
			._n_actions = 2,
			._n_action_info = 2,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 2,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 1,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 1,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 1,
				},
				.actions[5] = {
					.device = "camera",
					.info = 0,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 0,
				},
			},
			.t[1] = {
				.lvl_trig = 100000,
				.lvl_clr = 92000,
				.num_actions = 7,
				.actions[0] = {
					.device = "modem_cx",
					.info = 3,
				},
				.actions[1] = {
					.device = "gpu",
					.info = 200000000,
				},
				.actions[2] = {
					.device = "venus",
					.info = 3,
				},
				.actions[3] = {
					.device = "mdp",
					.info = 3,
				},
				.actions[4] = {
					.device = "wlan",
					.info = 4,
				},
				.actions[5] = {
					.device = "camera",
					.info = 10,
				},
				.actions[6] = {
					.device = "camcorder",
					.info = 10,
				},
			}
		},
	},
};

static struct setting_info tm_cfgs_8909_pm8916[] =
{
	{
		.desc = "CPU3_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 85000,
				.lvl_clr = 80000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_3",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CPU2_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu0-2",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 88000,
				.lvl_clr = 85000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_2",
					.info = 1,
				},
			}
		},
	},
	{
		.desc = "CPU1_HOTPLUG_MONITOR",
		.algo_type = MONITOR_ALGO_TYPE,
		.data.tm = {
			.sensor = "cpu1-3",
			.sampling_period_ms = 1000,
			.num_thresholds = 1,
			._n_thresholds = 1,
			._n_to_clear = 1,
			._n_actions = 1,
			._n_action_info = 1,
			.t[0] = {
				.lvl_trig = 92000,
				.lvl_clr = 85000,
				.num_actions = 1,
				.actions[0] = {
					.device = "hotplug_1",
					.info = 1,
				},
			}
		},
	},
};

void thermal_monitor_init_data(struct thermal_setting_t *setting)
{
	unsigned int i;
	struct setting_info *cfg = NULL;
	unsigned int arr_size = 0;

	switch (therm_get_msm_id()) {
		case THERM_MSM_8916:
			cfg = tm_cfgs_8916;
			arr_size = ARRAY_SIZE(tm_cfgs_8916);
			break;
		case THERM_MSM_8939:
		case THERM_MSM_8936:
		case THERM_MSM_8929:
			cfg = tm_cfgs_8939;
			arr_size = ARRAY_SIZE(tm_cfgs_8939);
			break;
		case THERM_MSM_8909:
			switch (therm_get_pmic_model()) {
				case THERM_PMIC_PM8916:
					cfg = tm_cfgs_8909_pm8916;
					arr_size =
					ARRAY_SIZE(tm_cfgs_8909_pm8916);
					break;
				case THERM_PMIC_PM8909:
				default:
					if (therm_get_hw_platform_subtype() ==
						THERM_PLATFORM_SUB_QRD_HUAQIN) {
						cfg = tm_cfgs_8909_qrd_hq;
						arr_size =
						ARRAY_SIZE(tm_cfgs_8909_qrd_hq);
					} else {
						cfg = tm_cfgs_8909;
						arr_size =
						ARRAY_SIZE(tm_cfgs_8909);
					}
					break;
			}
			break;
		default:
			msg("%s: Uknown Bear family device", __func__);
			break;
	}

	for (i = 0; i < arr_size; i++) {
		add_setting(setting, &cfg[i]);
	}
}
