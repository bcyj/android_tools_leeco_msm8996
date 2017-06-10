/*===========================================================================

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include "thermal.h"
#include "thermal_config.h"
#include "speaker_cal.h"

static char *sensor_list[] = {
	"tsens_tz_sensor1",
	"tsens_tz_sensor2",
	"tsens_tz_sensor3",
	"tsens_tz_sensor4",
	"tsens_tz_sensor7",
	"tsens_tz_sensor8",
	"tsens_tz_sensor9",
	"tsens_tz_sensor10",
};

static struct setting_info cfgs_8974[] =
{
	{
		.desc = "SPEAKER-CAL",
		.algo_type = EQUILIBRIUM_TYPE,
		.data.eq = {
			.sensor_list = sensor_list,
			.list_cnt = ARRAY_SIZE(sensor_list),
			.list_size = 0,
			.sensor = "pm8941_tz",
			.sensor_delta = 10000,
			.sampling_period_ms = 30000,
			.temp_range = 6000,
			.max_temp = 45000,
			.offset = -4000,
			.long_sampling_period_ms = 30000,
			.long_sampling_cnt = 10,
			.long_sample_range = 2000,
			.pre_cal_delay_ms = 1800000,
			},
	},
};

static struct setting_info cfgs_8084[] =
{
        {
                .desc = "SPEAKER-CAL",
                .algo_type = EQUILIBRIUM_TYPE,
                .data.eq = {
                        .sensor_list = sensor_list,
                        .list_cnt = ARRAY_SIZE(sensor_list),
                        .sensor = "pma8084_tz",
                        .sensor_delta = 10000,
                        .sampling_period_ms = 30000,
                        .temp_range = 6000,
                        .max_temp = 45000,
                        .offset = -4000,
                        .long_sampling_period_ms = 30000,
                        .long_sampling_cnt = 10,
                        .long_sample_range = 2000,
                        .pre_cal_delay_ms = 1800000,
                        },
        },
};

static struct setting_info cfgs_8994[] =
{
        {
                .desc = "SPEAKER-CAL",
                .algo_type = EQUILIBRIUM_TYPE,
                .data.eq = {
                        .sensor_list = sensor_list,
                        .list_cnt = ARRAY_SIZE(sensor_list),
                        .sensor = "pm8994_tz",
                        .sensor_delta = 10000,
                        .sampling_period_ms = 30000,
                        .temp_range = 6000,
                        .max_temp = 45000,
                        .offset = -4000,
                        .long_sampling_period_ms = 30000,
                        .long_sampling_cnt = 10,
                        .long_sample_range = 2000,
                        .pre_cal_delay_ms = 1800000,
                        },
        },
};

void speaker_cal_init_data(struct thermal_setting_t *setting)
{
	int i;
	int arr_size = 0;

	struct setting_info *cfg = NULL;

	switch (therm_get_msm_id()) {
		case THERM_MSM_8974:
		case THERM_MSM_8974PRO_AA:
		case THERM_MSM_8974PRO_AB:
		case THERM_MSM_8974PRO_AC:
			cfg = cfgs_8974;
			arr_size = ARRAY_SIZE(cfgs_8974);
			break;
		case THERM_MSM_8084:
		case THERM_MSM_8x62:
			cfg = cfgs_8084;
			arr_size = ARRAY_SIZE(cfgs_8084);
			break;
		case THERM_MSM_8994:
			cfg = cfgs_8994;
			arr_size = ARRAY_SIZE(cfgs_8994);
			break;
		default:
			msg("%s: ERROR Uknown device\n", __func__);
			break;
	}

	for (i = 0; i < arr_size; i++) {
		add_setting(setting, &cfg[i]);
	}
}
