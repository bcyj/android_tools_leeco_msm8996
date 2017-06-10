/*===========================================================================

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include "thermal.h"
#include "thermal_config.h"
#include "ss_algorithm.h"

static struct setting_info ss_cfgs_8974[] =
{
	{
		.desc = "SS-CPU0",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu3",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 80000,
			.set_point_clr = 55000,
			.time_constant = 16,
		},
	},
};

static struct setting_info ss_cfgs_8974_pro_ac[] =
{
	{
		.desc = "SS-CPU0",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu3",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 80000,
			.set_point_clr = 55000,
			.time_constant = 16,
		},
	},
	{
		.desc = "SS-GPU",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "tsens_tz_sensor10",
			.device = "gpu",
			.sampling_period_ms = 250,
			.set_point = 100000,
			.set_point_clr = 65000,
		},
	},
};

static struct setting_info ss_cfgs_8226_v1[] =
{
	{
		.desc = "SS-CPU2-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2-3",
			.device = "cpu",
			.sampling_period_ms = 1000,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 1000,
			.set_point = 50000,
			.set_point_clr = 45000,
			.time_constant = 5,
		},
	},
};

static struct setting_info ss_cfgs_8226[] =
{
	{
		.desc = "SS-CPU0-1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0-1",
			.device = "cpu",
			.sampling_period_ms = 250,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2-3",
			.device = "cpu",
			.sampling_period_ms = 250,
			.set_point = 90000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 1000,
			.set_point = 60000,
			.set_point_clr = 45000,
			.time_constant = 2,
		},
	},
};

static struct setting_info ss_cfgs_8610[] =
{
	{
		.desc = "SS-CPU0-1-2-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0-1-2-3",
			.device = "cpu",
			.sampling_period_ms = 250,
			.set_point = 80000,
			.set_point_clr = 65000,
		},
	},
};

static struct setting_info ss_cfgs_8916[] =
{
	{
		.desc = "SS-CPU0-1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0-1",
			.device = "cpu",
			.sampling_period_ms = 250,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2-3",
			.device = "cpu",
			.sampling_period_ms = 250,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 1000,
			.set_point = 70000,
			.set_point_clr = 45000,
			.time_constant = 2,
		},
	},
};

static struct setting_info ss_cfgs_9900[] =
{
	{
		.desc = "SS-CPU0",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu3",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
};

static struct setting_info ss_cfgs_8939[] =
{
	{
		.desc = "SS-CPU0",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu3",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU4-5-6-7",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu4-5-6-7",
			.device = "cluster0",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cluster1",
			.sampling_period_ms = 250,
			.set_point = 70000,
			.set_point_clr = 55000,
			.time_constant = 2,
		},
	},
};

static struct setting_info ss_cfgs_8936[] =
{
	{
		.desc = "SS-CPU0",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0",
			.device = "cluster0",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1",
			.device = "cluster0",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2",
			.device = "cluster0",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu3",
			.device = "cluster0",
			.sampling_period_ms = 50,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cluster0",
			.sampling_period_ms = 250,
			.set_point = 70000,
			.set_point_clr = 55000,
			.time_constant = 2,
		},
	},
};

static struct setting_info ss_cfgs_8994[] =
{
	{
		.desc = "SS-CPU0",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.set_point = 90000,
			.set_point_clr = 70000,
		},
	},
	{
		.desc = "SS-CPU1",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.set_point = 90000,
			.set_point_clr = 70000,
		},
	},
	{
		.desc = "SS-CPU2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu2",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.set_point = 90000,
			.set_point_clr = 70000,
		},
	},
	{
		.desc = "SS-CPU3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu3",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.set_point = 90000,
			.set_point_clr = 70000,
		},
	},
	{
		.desc = "SS-CPU4",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu4",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.set_point = 65000,
			.set_point_clr = 50000,
		},
	},
	{
		.desc = "SS-CPU5",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu5",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.set_point = 65000,
			.set_point_clr = 50000,
		},
	},
	{
		.desc = "SS-CPU6",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu6",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.set_point = 65000,
			.set_point_clr = 50000,
		},
	},
	{
		.desc = "SS-CPU7",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu7",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.set_point = 65000,
			.set_point_clr = 50000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.set_point = 80000,
			.set_point_clr = 55000,
			.time_constant = 16,
		},
	},
	{
		.desc = "SS-GPU",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "gpu",
			.device = "gpu",
			.sampling_period_ms = 250,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
};

static struct setting_info ss_cfgs_8909_qrd_lc[] =
{
	{
		.desc = "SS-CPU0-2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0-2",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1-3",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-EXT_THERM_XO",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "xo_therm",
			.device = "cpu",
			.sampling_period_ms = 1000,
			.set_point = 53000,
			.set_point_clr = 48000,
			.time_constant = 3,
			.device_mtgn_max_limit = 533000,
		},
	},
	{
		.desc = "SS-GPU",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "tsens_tz_sensor2",
			.device = "gpu",
			.sampling_period_ms = 250,
			.set_point = 80000,
			.set_point_clr = 65000,
		},
	},
};

static struct setting_info ss_cfgs_8909_qrd_hq[] =
{
	{
		.desc = "SS-CPU0-2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0-2",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1-3",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-EXT_THERM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "case_therm",
			.device = "cpu",
			.sampling_period_ms = 1000,
			.set_point = 45000,
			.set_point_clr = 40000,
			.time_constant = 3,
			.device_mtgn_max_limit = 533000,
		},
	},
	{
		.desc = "SS-GPU",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "tsens_tz_sensor2",
			.device = "gpu",
			.sampling_period_ms = 250,
			.set_point = 80000,
			.set_point_clr = 65000,
		},
	},
};

static struct setting_info ss_cfgs_8909[] =
{
	{
		.desc = "SS-CPU0-2",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu0-2",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-CPU1-3",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "cpu1-3",
			.device = "cpu",
			.sampling_period_ms = 65,
			.set_point = 85000,
			.set_point_clr = 55000,
		},
	},
	{
		.desc = "SS-POPMEM",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 250,
			.set_point = 75000,
			.set_point_clr = 55000,
			.time_constant = 2,
		},
	},
	{
		.desc = "SS-GPU",
		.algo_type = SS_ALGO_TYPE,
		.data.ss =
		{
			.sensor = "tsens_tz_sensor2",
			.device = "gpu",
			.sampling_period_ms = 250,
			.set_point = 80000,
			.set_point_clr = 65000,
		},
	},
};

void ss_init_data(struct thermal_setting_t *setting)
{
	int i, arr_size;
	struct setting_info *cfg;

	switch (therm_get_msm_id()) {
		case THERM_MSM_8974:
		case THERM_MSM_8x62:
		case THERM_MSM_8974PRO_AA:
		case THERM_MSM_8974PRO_AB:
			cfg = ss_cfgs_8974;
			arr_size = ARRAY_SIZE(ss_cfgs_8974);
			break;
		case THERM_MSM_8974PRO_AC:
		case THERM_MSM_8084:
			cfg = ss_cfgs_8974_pro_ac;
			arr_size = ARRAY_SIZE(ss_cfgs_8974_pro_ac);
			break;
		case THERM_MSM_8226:
			switch (therm_get_msm_version()) {
				case THERM_VERSION_V1:
					cfg = ss_cfgs_8226_v1;
					arr_size = ARRAY_SIZE(ss_cfgs_8226_v1);
					break;
				default:
					cfg = ss_cfgs_8226;
					arr_size = ARRAY_SIZE(ss_cfgs_8226);
					break;
			}
			break;
		case THERM_MSM_8926:
			cfg = ss_cfgs_8226;
			arr_size = ARRAY_SIZE(ss_cfgs_8226);
			break;
		case THERM_MSM_8916:
			cfg = ss_cfgs_8916;
			arr_size = ARRAY_SIZE(ss_cfgs_8916);
			break;
		case THERM_MSM_8610:
			cfg = ss_cfgs_8610;
			arr_size = ARRAY_SIZE(ss_cfgs_8610);
			break;
		case THERM_FSM_9900:
			cfg = ss_cfgs_9900;
			arr_size = ARRAY_SIZE(ss_cfgs_9900);
			break;
		case THERM_MSM_8939:
		case THERM_MSM_8929:
			cfg = ss_cfgs_8939;
			arr_size = ARRAY_SIZE(ss_cfgs_8939);
			break;
		case THERM_MSM_8936:
			cfg = ss_cfgs_8936;
			arr_size = ARRAY_SIZE(ss_cfgs_8936);
			break;
		case THERM_MSM_8994:
			cfg = ss_cfgs_8994;
			arr_size = ARRAY_SIZE(ss_cfgs_8994);
			break;
		case THERM_MSM_8909:
			switch (therm_get_hw_platform()) {
				case THERM_PLATFORM_QRD:
					if (therm_get_hw_platform_subtype() ==
						THERM_PLATFORM_SUB_QRD_HUAQIN) {
						cfg = ss_cfgs_8909_qrd_hq;
						arr_size =
						ARRAY_SIZE(ss_cfgs_8909_qrd_hq);
					} else {
						cfg = ss_cfgs_8909_qrd_lc;
						arr_size =
						ARRAY_SIZE(ss_cfgs_8909_qrd_lc);
					}
					break;
				default:
					cfg = ss_cfgs_8909;
					arr_size = ARRAY_SIZE(ss_cfgs_8909);
					break;
			}
			break;
		default:
			msg("%s: ERROR Uknown device\n", __func__);
			/* Better to have something in place even if it's wrong. */
			cfg = ss_cfgs_8974;
			arr_size = ARRAY_SIZE(ss_cfgs_8974);
			break;
	}

	for (i = 0; i < arr_size; i++)
		add_setting(setting, &cfg[i]);
}
