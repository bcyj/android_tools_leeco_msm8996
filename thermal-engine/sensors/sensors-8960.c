/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "thermal.h"
#include "sensors-tsens.h"
#include "sensors-adc.h"
#include "sensors-bcl.h"
#include "sensors-gen.h"
#include "sensors-qmi.h"
#include "sensors-pm8821.h"
#include "sensors.h"
#include "sensors_manager_internal.h"


#define NUM_TSENS_SENSORS (11)

enum hybrid_state_t {
	HYBRID_STATE_MAIN_INTERRUPT,
	HYBRID_STATE_POLLING
};

enum sensor_type_8960_t {
	SENSOR_TYPE_NON_HYBRID,
	SENSOR_TYPE_HYBRID_MAIN,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_BCL,
};

/* Hybrid polling variables */
static pthread_cond_t hybrid_aux_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t hybrid_aux_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t hybrid_state_mtx = PTHREAD_MUTEX_INITIALIZER;
static enum hybrid_state_t hybrid_state;


/* Hybrid polling functions */
static int hybrid_sensor_get_temperature(struct sensor_info *sensor);
static void hybrid_main_interrupt_wait(struct sensor_info * sensor);
static void hybrid_main_update_thresholds(struct sensor_info * sensor, struct thresholds_req_t *thresh);
static void hybrid_aux_interrupt_wait(struct sensor_info * sensor);
static void hybrid_aux_update_thresholds(struct sensor_info * sensor, struct thresholds_req_t *thresh);

/* PA sensor setup function */
static int  pa_sensor_setup(struct sensor_info * sensor);


struct alias_map_t {
	char *sensor_name;
	char *alias;
};

static struct alias_map_t alias_map_8960[] = {
	{"tsens_tz_sensor0", "cpu0" },
	{"tsens_tz_sensor2", "cpu1" },
	{"tsens_tz_sensor3", "pop_mem" },
};

static struct alias_map_t alias_map_8930[] = {
	{"tsens_tz_sensor9", "cpu0" },
	{"tsens_tz_sensor6", "cpu1" },
	{"tsens_tz_sensor3", "pop_mem" },
};

static struct alias_map_t alias_map_8064[] = {
	{"tsens_tz_sensor7", "cpu0" },
	{"tsens_tz_sensor8", "cpu1" },
	{"tsens_tz_sensor9", "cpu2" },
	{"tsens_tz_sensor10", "cpu3" },
	{"tsens_tz_sensor6", "pop_mem" },
};

static struct sensor_info tsens_sensors[] = {
	{
		.name = "tsens_tz_sensor0",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_main_interrupt_wait,
		.update_thresholds = hybrid_main_update_thresholds,
		.tzn = 0,
		.data = NULL,
		.interrupt_enable = 1,
	},
	{
		.name = "tsens_tz_sensor1",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 1,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor2",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 2,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor3",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 3,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor4",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 4,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor5",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 5,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor6",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 6,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor7",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 7,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor8",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 8,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor9",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 9,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "tsens_tz_sensor10",
		.setup = tsens_sensors_setup,
		.shutdown = tsens_sensors_shutdown,
		.get_temperature = hybrid_sensor_get_temperature,
		.interrupt_wait = hybrid_aux_interrupt_wait,
		.update_thresholds = hybrid_aux_update_thresholds,
		.tzn = 10,
		.data = NULL,
		.interrupt_enable = 0,
	},
};

static struct sensor_info gen_sensors[] = {
	{
		.name = "pm8921_tz",
		.setup = gen_sensors_setup,
		.shutdown = gen_sensors_shutdown,
		.get_temperature = gen_sensor_get_temperature,
		.interrupt_wait = NULL,
		.update_thresholds = NULL,
		.tzn = 0,
		.data = NULL,
		.interrupt_enable = 0,
	}
};

static struct sensor_info gen_8064_sensors[] = {
	{
		.name = "pm8821_tz",
		.setup = pm8821_setup,
		.shutdown = pm8821_shutdown,
		.get_temperature = pm8821_get_temperature,
		.interrupt_wait = pm8821_interrupt_wait,
		.update_thresholds = NULL,
		.tzn = 12,
		.data = NULL,
		.interrupt_enable = 1,
	}
};

static struct sensor_info pa_sensors[] = {
	{
		.name = "pa_therm0",
		.setup = pa_sensor_setup,
		.shutdown = NULL,
		.get_temperature = NULL,
		.interrupt_wait = NULL,
		.update_thresholds = NULL,
		.tzn = 0,
		.data = NULL,
		.interrupt_enable = 0,
	},
	{
		.name = "pa_therm1",
		.setup = pa_sensor_setup,
		.shutdown = NULL,
		.get_temperature = NULL,
		.interrupt_wait = NULL,
		.update_thresholds = NULL,
		.tzn = 0,
		.data = NULL,
		.interrupt_enable = 0,
	}
};

static struct sensor_info bcl_sensors[] = {
	{
		.name = "bcl",
		.setup = bcl_setup,
		.shutdown = bcl_shutdown,
		.get_temperature = bcl_get_iavail,
		.interrupt_wait = bcl_interrupt_wait,
		.update_thresholds = bcl_update_thresholds,
		.tzn = 0,
		.data = NULL,
		.interrupt_enable = 1,
	}
};

/* NOTE: number of indexes in sensor_type should match g_sensors */
static enum sensor_type_8960_t tsens_sensor_type[] = {
	SENSOR_TYPE_HYBRID_MAIN,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
	SENSOR_TYPE_HYBRID_AUX,
};

/* status of main sensor's threshold interrupt, toggle 0 and 1 */
static int main_sensor_hi_threshold_enabled;
/* Sensors low threshold cleared mask.  Init to cleared. */
static unsigned int threshold_cleared = (0x1 << NUM_TSENS_SENSORS) - 1;

#define THRESH_ALL_LOW_CLEARED ((threshold_cleared) == ((0x1 << NUM_TSENS_SENSORS) - 1))

static char *find_alias(const char * sensor_name)
{
	char               *ret_val = NULL;
	static uint8_t      init;
	static struct alias_map_t *arr;
	static uint8_t      arr_len;

	if (init == 0) {
		switch (therm_get_msm_id()) {
			case THERM_MSM_8960:
				arr = alias_map_8960;
				arr_len = ARRAY_SIZE(alias_map_8960);
				break;
			case THERM_MSM_8930:
				arr = alias_map_8930;
				arr_len = ARRAY_SIZE(alias_map_8930);
				break;
			case THERM_MSM_8064AB:
			case THERM_MSM_8064:
				arr = alias_map_8064;
				arr_len = ARRAY_SIZE(alias_map_8064);
				break;
			default:
				msg("%s: Uknown A-family device", __func__);
				break;
		}
		init = 1;
	}

	if (arr != NULL) {
		uint8_t idx;
		for (idx = 0; idx < arr_len; idx++) {
			if (strncmp(sensor_name, arr[idx].sensor_name,
				    MAX_SENSOR_NAME_LEN) == 0) {
				ret_val = arr[idx].alias;
				break;
			}
		}
	}
	return ret_val;
}

static int hybrid_sensor_id(struct sensor_info *sensor)
{
	int sensor_id = 0;

	if (NULL == sensor ||
	    NULL == sensor->name) {
		msg("%s: Unexpected NULL", __func__);
		return -1;
	}

	if (!sscanf(sensor->name, "tsens_tz_sensor%d", &sensor_id))
		return -1;
	else
		return sensor_id;
}

/* Manually disable sensors for unconfigured sensors */
static void disable_sensor_manual(int tzn)
{
	char name[MAX_PATH] = {0};

	snprintf(name, MAX_PATH, TZ_MODE, tzn);
	write_to_file(name, "disabled", strlen("disabled"));
	dbgmsg("TSENS '%d' not configured - set disabled\n", tzn);
}

static int sensor_is_configured(struct sensor_info *sensor)
{
	struct sensors_mgr_sensor_info *sensor_mgr = sensor->sensor_mgr;
	return (((int)sensor_mgr->client_list)?(1):(0));
}

static void hybrid_enable_auxiliary_sensors(int enabled)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(tsens_sensor_type); i++) {
		if (tsens_sensor_type[i] != SENSOR_TYPE_HYBRID_AUX)
			continue;

		if (sensor_is_configured(&tsens_sensors[i]))
			tsens_sensor_enable_sensor(&tsens_sensors[i], enabled);
		else if (enabled == 0 &&
			   !sensor_is_configured(&tsens_sensors[i])) {
			/* disable sensor manually if unconfigured */
			disable_sensor_manual(tsens_sensors[i].tzn);
		}
	}
}

static int hybrid_sensor_get_temperature(struct sensor_info *sensor)
{
	int ret_val;

	pthread_mutex_lock(&hybrid_state_mtx);
	if (hybrid_state == HYBRID_STATE_MAIN_INTERRUPT)
		ret_val = tsens_sensor_get_temperature(&tsens_sensors[0]);
	else
		ret_val = tsens_sensor_get_temperature(sensor);
	pthread_mutex_unlock(&hybrid_state_mtx);
	return ret_val;
}

static void hybrid_main_interrupt_wait(struct sensor_info *sensor)
{
	int performed_wait = 0;

	if (sensor == NULL ||
	    sensor->name == NULL) {
		msg("%s: Unexpected NULL", __func__);
		return;
	}

	/* If main sensor in interrupt mode */
	if (hybrid_state == HYBRID_STATE_MAIN_INTERRUPT) {
		if (main_sensor_hi_threshold_enabled == 0) {
			/* disable auxiliary sensors first, then
			   reenable main sensor hi threshold */
			hybrid_enable_auxiliary_sensors(0);

			tsens_sensor_enable_thresholds(sensor, 1, 0);
			main_sensor_hi_threshold_enabled = 1;
		}
		tsens_sensor_interrupt_wait(sensor);
		performed_wait = 1;
	}

	/* Don't sleep on first poll */
	if (!performed_wait) {
		struct sensors_mgr_sensor_info *sensor_mgr = sensor->sensor_mgr;
		int polling_interval =
			(sensor_mgr->active_thresh.polling_interval_valid)?
			(sensor_mgr->active_thresh.polling_interval):
			(SENSOR_DEFAULT_POLLING_INTERVAL);
		dbgmsg("%s: Interval %dms\n", __func__, polling_interval);
		usleep(polling_interval*1000);
	}
}

static void hybrid_aux_interrupt_wait(struct sensor_info *sensor)
{
	int performed_wait = 0;

	if (sensor == NULL) {
		msg("%s: Unexpected NULL", __func__);
		return;
	}

	/* Auxiliary sensors should block on main sensor interrupt */
	if (hybrid_state != HYBRID_STATE_POLLING) {
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&hybrid_aux_mtx);
		while (HYBRID_STATE_POLLING != hybrid_state) {
			pthread_cond_wait(&hybrid_aux_cond,
					  &hybrid_aux_mtx);
		}
		pthread_mutex_unlock(&hybrid_aux_mtx);

		performed_wait = 1;
	}

	/* Don't sleep on first poll */
	if (!performed_wait) {
		struct sensors_mgr_sensor_info *sensor_mgr = sensor->sensor_mgr;
		int polling_interval =
			(sensor_mgr->active_thresh.polling_interval_valid)?
			(sensor_mgr->active_thresh.polling_interval):
			(SENSOR_DEFAULT_POLLING_INTERVAL);
		dbgmsg("%s: Interval %dms\n", __func__, polling_interval);
		usleep(polling_interval*1000);
	}
}

static void hybrid_main_update_thresholds(struct sensor_info *sensor,
					   struct thresholds_req_t *thresh)
{
	if (sensor == NULL) {
		msg("%s: Unexpected NULL", __func__);
		return;
	}

	/* TSENS thresholds don't need updating for 8960
	   sensor_setup sets threshold 0 */
	/* Update state for hybrid polling */
	if (thresh->low_valid)
		threshold_cleared &= ~0x1;
	else
		threshold_cleared |= 0x1;

	if (hybrid_state == HYBRID_STATE_MAIN_INTERRUPT &&
	    (thresh->high_valid && !thresh->low_valid)) {
		/* Rearm threshold if we didn't actually cross hi threshold */
		dbgmsg("%s: Arm TSENS threshold\n", __func__);
		tsens_sensor_update_thresholds(sensor, thresh);
	} else if (hybrid_state == HYBRID_STATE_MAIN_INTERRUPT &&
		   (!THRESH_ALL_LOW_CLEARED)) {
		/* Enable auxiliary hybrid polling on threshold crossing */
		dbgmsg("Hybrid state -> polling enabled\n");
		/* transition to polling expected,
		   disable interrupts, enable aux sensors */
		tsens_sensor_enable_thresholds(sensor, 0, 0);
		main_sensor_hi_threshold_enabled = 0;

		pthread_mutex_lock(&hybrid_state_mtx);
		hybrid_enable_auxiliary_sensors(1);
		hybrid_state = HYBRID_STATE_POLLING;
		pthread_mutex_lock(&hybrid_aux_mtx);
		pthread_cond_broadcast(&hybrid_aux_cond);
		pthread_mutex_unlock(&hybrid_aux_mtx);
		pthread_mutex_unlock(&hybrid_state_mtx);
	} else if (hybrid_state == HYBRID_STATE_POLLING &&
		   THRESH_ALL_LOW_CLEARED) {
		/* disable polling on threshold clearing */
		pthread_mutex_lock(&hybrid_state_mtx);
		dbgmsg("Hybrid state -> interrupt\n");
		hybrid_state = HYBRID_STATE_MAIN_INTERRUPT;
		dbgmsg("%s: Arm TSENS threshold\n", __func__);
		tsens_sensor_update_thresholds(sensor, thresh);
		pthread_mutex_unlock(&hybrid_state_mtx);
	}
}

static void hybrid_aux_update_thresholds(struct sensor_info *sensor,
					  struct thresholds_req_t *thresh)
{
	int hybrid_id = 0;

	if (sensor == NULL) {
		msg("%s: Unexpected NULL", __func__);
		return;
	}

	hybrid_id = hybrid_sensor_id(sensor);
	if (hybrid_id <= 0 || hybrid_id >= NUM_TSENS_SENSORS) {
		msg("%s: Bad hybrid sensor_id\n", __func__);
		return;
	}

	/* Update state for hybrid polling */
	if (thresh->low_valid)
		threshold_cleared &= (~(0x1 << hybrid_id));
	else
		threshold_cleared |= (0x1 << hybrid_id);

	/* disable polling on threshold clearing */
	if (hybrid_state == HYBRID_STATE_POLLING &&
	    THRESH_ALL_LOW_CLEARED) {
		pthread_mutex_lock(&hybrid_state_mtx);
		dbgmsg("Hybrid state -> interrupt\n");
		hybrid_state = HYBRID_STATE_MAIN_INTERRUPT;
		pthread_mutex_unlock(&hybrid_state_mtx);
	}
}

static void generic_wait(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	sensor->interrupt_wait(sensor);
}

static int generic_read(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	return sensor->get_temperature(sensor);
}

static void generic_update_thresholds(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	sensor->update_thresholds(sensor,
				 &sensor_mgr->active_thresh.thresh);
}

static void generic_shutdown(struct sensors_mgr_sensor_info *sensor_mgr)
{
	struct sensor_info *sensor = (struct sensor_info *)sensor_mgr->data;
	sensor->shutdown(sensor);

	free(sensor_mgr);
}

static int  pa_sensor_setup(struct sensor_info *sensor)
{
	int sensor_count = 0;

	sensor_count = adc_sensors_setup(sensor);
	if (sensor_count) {
		info("ADC sensor found for %s\n", sensor->name);
		sensor->shutdown        = adc_sensors_shutdown;
		sensor->get_temperature = adc_sensor_get_temperature;
	} else {
		sensor_count = qmi_ts_setup(sensor);
		if (sensor_count) {
			info("QMI TS sensor found for %s\n", sensor->name);
			sensor->update_thresholds = qmi_ts_update_thresholds;
			sensor->get_temperature   = qmi_ts_get_temperature;
			sensor->interrupt_wait    = qmi_ts_interrupt_wait;
			sensor->shutdown          = qmi_ts_shutdown;
			sensor->interrupt_enable  = 1;
		}
	}
	return sensor_count;
}

static int add_tgt_sensor(struct sensor_info *sensor)
{
	int ret_val = 0;
	struct sensors_mgr_sensor_info *sensor_mgr = NULL;

	sensor_mgr = malloc(sizeof(struct sensors_mgr_sensor_info));
	if (sensor_mgr == NULL) {
		msg("%s: malloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto error_handler;
	}
	memset(sensor_mgr, 0, sizeof(struct sensors_mgr_sensor_info));

	sensor_mgr->name = sensor->name;
	sensor_mgr->alias = find_alias(sensor->name);

	if (sensor->setup(sensor) == 0) {
		ret_val = -(EFAULT);
		goto error_handler;
	}

	sensor_mgr->data = (void*) sensor;
	sensor->sensor_mgr = sensor_mgr;


	sensor_mgr->get_temperature = generic_read;
	sensor_mgr->shutdown = generic_shutdown;

	if (sensor->interrupt_wait)
		sensor_mgr->wait = generic_wait;

	if (sensor->update_thresholds)
		sensor_mgr->update_thresholds = generic_update_thresholds;

	sensors_manager_add_sensor(sensor_mgr);

error_handler:
	if (ret_val) {
		if (sensor_mgr)
			free(sensor_mgr);
	}
	return ret_val;
}

static int add_tgt_sensors_set(struct sensor_info *sensor_arr, int arr_size)
{
	int idx = 0;
	int ret_val = 0;

	for (idx = 0; idx < arr_size; idx++) {
		if (add_tgt_sensor(&sensor_arr[idx]) != 0) {
			msg("%s: Error adding %s\n", __func__,
			    sensor_arr[idx].name);
			ret_val = -(EFAULT);
			break;
		}
	}

	return ret_val;
}
static int min_mode;
int sensors_init(int minimum_mode)
{
	int ret_val = 0;
	min_mode = minimum_mode;
	enum therm_msm_id msm_id = therm_get_msm_id();

	if (!min_mode)
		modem_ts_qmi_init();

	if (add_tgt_sensors_set(tsens_sensors,
			   ARRAY_SIZE(tsens_sensors)) != 0) {
		msg("%s: Error adding TSENS TS\n", __func__);
		ret_val = -(EFAULT);
	}

	/* Disable Aux Sensors */
	hybrid_state = HYBRID_STATE_MAIN_INTERRUPT;
	hybrid_enable_auxiliary_sensors(0);
	/* Enable main TSENS */
	tsens_sensor_enable_sensor(&tsens_sensors[0], 1);

	/* PA */
	if (add_tgt_sensors_set(pa_sensors,
			   ARRAY_SIZE(pa_sensors)) != 0) {
		msg("%s: Error adding PA TS\n", __func__);
		ret_val = -(EFAULT);
	}

	/* GEN */
	if (add_tgt_sensors_set(gen_sensors,
			   ARRAY_SIZE(gen_sensors)) != 0) {
		msg("%s: Error adding GEN TS\n", __func__);
		ret_val = -(EFAULT);
	}

	/* GEN 8064 */
	if ((msm_id == THERM_MSM_8064) ||
	    (msm_id == THERM_MSM_8064AB)) {
		if (add_tgt_sensors_set(gen_8064_sensors,
					ARRAY_SIZE(gen_8064_sensors)) != 0) {
			msg("%s: Error adding GEN 8064 TS\n", __func__);
			ret_val = -(EFAULT);
		}
	}

	/* BCL */
	if (add_tgt_sensors_set(bcl_sensors,
			   ARRAY_SIZE(bcl_sensors)) != 0) {
		msg("%s: Error adding BCL TS\n", __func__);
		ret_val = -(EFAULT);
	}

	return ret_val;
}

void sensors_release(void)
{
	if (!min_mode)
		modem_qmi_ts_comm_release();
}
