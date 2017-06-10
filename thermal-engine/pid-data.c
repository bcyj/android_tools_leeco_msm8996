/*===========================================================================

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include "thermal.h"
#include "thermal_config.h"
#include "pid_algorithm.h"

#define KHZ_PER_C (5000U)
#define SET_POINT (90000U)
#define SET_POINT_CLR (55000U)
#define POP_MEM_SET_POINT (80000U)
#define POP_MEM_SET_POINT_CLR (55000U)
#define K_P (0.7f)
#define K_I (15.0f)
#define K_D (0.3f)
#define PID_SAMPLE_PERIOD (65U)
#define I_SAMPLES (10U)
#define ERR_WEIGHT (1.0f)
#define FREQ_SCALE (500.0f)

#define PID_HYBRID_PERIOD (1000U)

static struct setting_info pid_cfgs_8974[] =
{
	{
		.desc = "PID-CPU0",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu0",
			.device = "cpu0",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU1",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu1",
			.device = "cpu1",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU2",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu2",
			.device = "cpu2",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU3",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu3",
			.device = "cpu3",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-POPMEM",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = POP_MEM_SET_POINT,
			.set_point_clr = POP_MEM_SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
};

static struct setting_info pid_cfgs_8226[] =
{
	{
		.desc = "PID-POPMEM",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = POP_MEM_SET_POINT,
			.set_point_clr = POP_MEM_SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.err_weight = ERR_WEIGHT,
		},
	},
};

static struct setting_info pid_cfgs_8610[] =
{
	{
		.desc = "PID-POPMEM",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = POP_MEM_SET_POINT,
			.set_point_clr = POP_MEM_SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.err_weight = ERR_WEIGHT,
		},
	},
};

static struct setting_info pid_cfgs_9900[] =
{
	{
		.desc = "PID-CPU0",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu0",
			.device = "cpu0",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = 85000U,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU1",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu1",
			.device = "cpu1",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = 85000U,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU2",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu2",
			.device = "cpu2",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = 85000U,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU3",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu3",
			.device = "cpu3",
			.sampling_period_ms = PID_SAMPLE_PERIOD,
			.i_samples = I_SAMPLES,
			.set_point = 85000U,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
};

static struct setting_info pid_cfgs_8939[] =
{
	{
		.desc = "PID-CPU0",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu0",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU1",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu1",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU2",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu2",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU3",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu3",
			.device = "cluster1",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU4-5-6-7",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu4-5-6-7",
			.device = "cluster0",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-POPMEM",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "pop_mem",
			.device = "cluster1",
			.sampling_period_ms = 250,
			.i_samples = I_SAMPLES,
			.set_point = POP_MEM_SET_POINT,
			.set_point_clr = POP_MEM_SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
};

static struct setting_info pid_cfgs_8936[] =
{
	{
		.desc = "PID-CPU0",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu0",
			.device = "cpu0",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU1",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu1",
			.device = "cpu1",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU2",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu2",
			.device = "cpu2",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU3",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu3",
			.device = "cpu3",
			.sampling_period_ms = 50,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-POPMEM",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "pop_mem",
			.device = "cpu",
			.sampling_period_ms = 250,
			.i_samples = I_SAMPLES,
			.set_point = POP_MEM_SET_POINT,
			.set_point_clr = POP_MEM_SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
};

static struct setting_info pid_cfgs_8994[] =
{
	{
		.desc = "PID-CPU0",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu0",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU1",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu1",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU2",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu2",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU3",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu3",
			.device = "cluster0",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU4",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu4",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU5",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu5",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU6",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu6",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-CPU7",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "cpu7",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = SET_POINT,
			.set_point_clr = SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
	{
		.desc = "PID-POPMEM",
		.algo_type = PID_ALGO_TYPE,
		.disable = 1,
		.data.pid =
		{
			.sensor = "pop_mem",
			.device = "cluster1",
			.sampling_period_ms = 10,
			.i_samples = I_SAMPLES,
			.set_point = POP_MEM_SET_POINT,
			.set_point_clr = POP_MEM_SET_POINT_CLR,
			.units_per_C = KHZ_PER_C,
			.p_const = K_P,
			.i_const = K_I,
			.d_const = K_D,
			.freq_scale = FREQ_SCALE,
			.err_weight = ERR_WEIGHT,
		},
	},
};

void pid_init_data(struct thermal_setting_t *setting)
{
	int i, arr_size;
	struct setting_info *cfg;

	switch (therm_get_msm_id()) {
		case THERM_MSM_8974:
		case THERM_MSM_8084:
		case THERM_MSM_8x62:
		case THERM_MSM_8974PRO_AA:
		case THERM_MSM_8974PRO_AB:
		case THERM_MSM_8974PRO_AC:
			cfg = pid_cfgs_8974;
			arr_size = ARRAY_SIZE(pid_cfgs_8974);
			break;
		case THERM_MSM_8226:
		case THERM_MSM_8926:
		case THERM_MSM_8916:
		case THERM_MSM_8909:
			cfg = pid_cfgs_8226;
			arr_size = ARRAY_SIZE(pid_cfgs_8226);
			break;
		case THERM_MSM_8610:
			cfg = pid_cfgs_8610;
			arr_size = ARRAY_SIZE(pid_cfgs_8610);
			break;
		case THERM_FSM_9900:
			cfg = pid_cfgs_9900;
			arr_size = ARRAY_SIZE(pid_cfgs_9900);
			break;
		case THERM_MSM_8939:
		case THERM_MSM_8929:
			cfg = pid_cfgs_8939;
			arr_size = ARRAY_SIZE(pid_cfgs_8939);
			break;
		case THERM_MSM_8936:
			cfg = pid_cfgs_8936;
			arr_size = ARRAY_SIZE(pid_cfgs_8936);
			break;
		case THERM_MSM_8994:
			cfg = pid_cfgs_8994;
			arr_size = ARRAY_SIZE(pid_cfgs_8994);
			break;
		default:
			msg("%s: ERROR Uknown device\n", __func__);
			/* Better to have something in place even if it's wrong. */
			cfg = pid_cfgs_8974;
			arr_size = ARRAY_SIZE(pid_cfgs_8974);
			break;
	}

	for (i = 0; i < arr_size; i++)
		add_setting(setting, &cfg[i]);
}
