/*===========================================================================

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "thermal.h"
#include "thermal_config.h"

#define CONFIG_FILE_SIZE_MAX  5120

enum field_type {
	DEBUG,
	SAMPLING,
	THRESHOLDS,
	THRESHOLDS_CLR,
	ACTIONS,
	ACTION_INFO,
	ALGO_TYPE,
	P_CONST,
	I_CONST,
	D_CONST,
	ERR_WEIGHT,
	I_SAMPLES,
	UNITS_PER_CALC,
	DISABLE,
	SET_POINT,
	SET_POINT_CLR,
	SENSOR_FIELD,
	DEVICE_FIELD,
	DESCENDING,
	TIME_CONSTANT,
	OVERRIDE,
	TEMP_RANGE,
	MAX_TEMP,
	OFFSET,
	SENSORS_FIELD,
	WEIGHTS,
	/* Add fields above this. */
	FIELD_IDX_MAX,
};

struct fields {
	char *name;
	enum field_type type;
};

struct algorithms {
	char *name;
	enum algo_type type;
};

struct fields fields[] = {
	{.name = "debug", .type = DEBUG},
	{.name = "sampling", .type = SAMPLING},
};

struct fields pid_fields[] = {
	{.name = "sampling", .type = SAMPLING},
	{.name = "p_const", .type = P_CONST},
	{.name = "i_const", .type = I_CONST},
	{.name = "d_const", .type = D_CONST},
	{.name = "err_weight", .type = ERR_WEIGHT},
	{.name = "i_samples", .type = I_SAMPLES},
	{.name = "dev_units_per_calc", .type = UNITS_PER_CALC},
	{.name = "disable", .type = DISABLE},
	{.name = "sensor", .type = SENSOR_FIELD},
	{.name = "device", .type = DEVICE_FIELD},
	{.name = "set_point", .type = SET_POINT},
	{.name = "set_point_clr", .type = SET_POINT_CLR},
	{.name = "override", .type = OVERRIDE},
};

struct fields ss_fields[] = {
	{.name = "sampling", .type = SAMPLING},
	{.name = "disable", .type = DISABLE},
	{.name = "sensor", .type = SENSOR_FIELD},
	{.name = "device", .type = DEVICE_FIELD},
	{.name = "set_point", .type = SET_POINT},
	{.name = "set_point_clr", .type = SET_POINT_CLR},
	{.name = "time_constant", .type = TIME_CONSTANT},
	{.name = "override", .type = OVERRIDE},
};

struct fields tm_fields[] = {
	{.name = "sampling", .type = SAMPLING},
	{.name = "thresholds", .type = THRESHOLDS},
	{.name = "thresholds_clr", .type = THRESHOLDS_CLR},
	{.name = "actions", .type = ACTIONS},
	{.name = "action_info", .type = ACTION_INFO},
	{.name = "disable", .type = DISABLE},
	{.name = "sensor", .type = SENSOR_FIELD},
	{.name = "descending", .type = DESCENDING},
	{.name = "override", .type = OVERRIDE},
};

struct fields eq_fields[] = {
	{.name = "sampling", .type = SAMPLING},
	{.name = "sensor", .type = SENSOR_FIELD},
	{.name = "sensors", .type = SENSORS_FIELD},
	{.name = "temp_range", .type = TEMP_RANGE},
	{.name = "max_temp", .type = MAX_TEMP},
	{.name = "offset", .type = OFFSET},
	{.name = "disable", .type = DISABLE},

};

struct fields v_fields[] = {
	{.name = "sampling", .type = SAMPLING},
	{.name = "trip_sensor", .type = SENSOR_FIELD},
	{.name = "sensors", .type = SENSORS_FIELD},
	{.name = "weights", .type = WEIGHTS},
	{.name = "set_point", .type = SET_POINT},
	{.name = "set_point_clr", .type = SET_POINT_CLR},
};

static struct algorithms algos[] = {
	{.name = "monitor", .type = MONITOR_ALGO_TYPE},
	{.name = "pid", .type = PID_ALGO_TYPE},
	{.name = "ss", .type = SS_ALGO_TYPE},
	{.name = "virtual", .type = VIRTUAL_SENSOR_TYPE},
};

static struct device_info *device_info_arr;
static uint32_t      device_info_arr_len;
static struct sensor_info_type *sensor_info_arr;
static uint32_t      sensor_info_arr_len;

void add_setting(struct thermal_setting_t *settings, struct setting_info *info)
{
	if ((settings == NULL) || (info == NULL)) {
		msg("%s: Invalid args.\n", __func__);
		return;
	}

	/* Insert into the config list head */
	info->next = settings->list;
	settings->list = info;
}

void init_settings(struct thermal_setting_t *settings)
{
	if (!settings) {
		return;
	}

	memset(settings, 0, sizeof(struct thermal_setting_t));
	settings->sample_period_ms = 0;
}

static void init_section_settings(struct setting_info *settings)
{
	if (!settings)
		return;

	memset(settings, 0, sizeof(struct setting_info));
	settings->algo_type = UNKNOWN_ALGO_TYPE;
}

void skip_space(char **ppos)
{
	char *pos = *ppos;

	while(*pos != '\0') {
		if ((*pos == ' ') || (*pos == '\t') || (*pos == '\n') ||
		    (*pos == '\r')) {
			pos++;
		}
		else
			break;
	}
	*ppos = pos;
}

void skip_line(char **ppos)
{
	char *pos = *ppos;
	char *ptmp;

	ptmp = strchr(pos, '\n');
	if (!ptmp) {
		*pos = '\0';
		return;
	}

	*ppos = ptmp + 1;
}

void skip_num(char **ppos)
{
	char *pos = *ppos;

	while(*pos != '\0') {
		if (*pos <= '9' && *pos >= '0') {
			pos++;
		}
		else
			break;
	}
	*ppos = pos;
	skip_space(ppos);
}

static int parse_algo(char **ppos)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	int  ret_val = UNKNOWN_ALGO_TYPE;


	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	if (pvalue != NULL) {
		int i;
		for (i = 0; i < ARRAY_SIZE(algos); i++) {
			if (strcasecmp(algos[i].name, pvalue) == 0) {
				ret_val = algos[i].type;
				dbgmsg("Algo Type '%s'\n", pvalue);
				break;
			}
		}

		if (ret_val == UNKNOWN_ALGO_TYPE) {
			msg("Unknown algo '%s'\n", pvalue);
		}
	}

	*ppos = ptmp + 1;
	return ret_val;
}

static int parse_float(char **ppos, float *result)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	float  ret_val = -(EFAULT);


	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	if (pvalue != NULL) {
		*result = strtof(pvalue, NULL);
		ret_val = 0;
	}

	*ppos = ptmp + 1;
	return (int)ret_val;
}

void parse_action(char *pvalue, struct action_t *result)
{
	uint32_t i;

	for (i = 0; i < device_info_arr_len; i++) {
		if (strcasecmp(device_info_arr[i].name, pvalue) == 0) {
			strlcpy(result->device, device_info_arr[i].name,
				DEVICES_MAX_NAME_LEN);
			return;
		}
	}
	msg("Unknown action '%s'\n", pvalue);
}

int parse_action_values(char **ppos,
			struct action_t result[THRESHOLDS_MAX][ACTIONS_MAX],
			uint32_t *num_actions)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	uint32_t  nresult = 0;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		char *pstr = NULL;
		char *psave2 = NULL;
		uint32_t count = 0;

		pstr = strtok_r(pvalue , "+", &psave2);
		while (pstr != NULL) {
			dbgmsg("Found action '%s'\n", pstr);
			parse_action(pstr, &result[nresult][count]);
			count++;
			if (count >= ACTIONS_MAX)
				break;

			pstr = strtok_r(NULL, "+", &psave2);
		}

		num_actions[nresult++] = count;
		if (nresult >= THRESHOLDS_MAX)
			break;
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}

	dbgmsg("No. of items found : %d\n", nresult);

	*ppos = ptmp + 1;
	return (int)nresult;
}

static char *verify_sensor(char *pvalue, char **sensor_ptr, uint32_t cnt)
{
	uint32_t i;

	/* Make sure we don't have a duplicate sensor in list already */
	for (i = 0; i < cnt; i++) {
		if (strcasecmp(sensor_ptr[i], pvalue) == 0) {
			msg("%s: Duplicate '%s'\n", __func__, pvalue);
			return NULL;
		}
	}

	/* Make sure sensor name is valid */
	for (i = 0; i < sensor_info_arr_len; i++) {
		if (strcasecmp(sensor_info_arr[i].name, pvalue) == 0) {
			return sensor_info_arr[i].name;
		}
	}

	msg("%s: Unknown '%s'\n", __func__, pvalue);
	return NULL;
}

static char **parse_sensor_values(char **ppos, int *num_sensors)
{
	char *pos = *ppos;
	char *ptmp = NULL;
	char *pvalue, *psave = NULL;
	char *sensor = NULL;
	uint32_t nresult = 0;

	char **sensor_ptr = (char**)malloc(sizeof(char*) * sensor_info_arr_len);

	if (sensor_ptr == NULL)
		goto handle_return;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		dbgmsg("Found sensor '%s'\n", pvalue);
		sensor = verify_sensor(pvalue, sensor_ptr, nresult);
		if (sensor != NULL) {
			size_t len = strnlen(sensor, MAX_SENSOR_NAME_LEN) + 1;
			sensor_ptr[nresult] = (char*)malloc(sizeof(char) * len);
			if (sensor_ptr[nresult]) {
				strlcpy(sensor_ptr[nresult], sensor, len);
				nresult++;
			}
		}
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}

handle_return:
	dbgmsg("No. of items found : %d\n", nresult);
	if (ptmp != NULL)
		*ppos = ptmp + 1;

	if (nresult == 0) {
		if (sensor_ptr) {
			free(sensor_ptr);
		}
		sensor_ptr = NULL;
	}
	*num_sensors = (int)nresult;

	return sensor_ptr;
}

int parse_sensor_field(char **ppos, char **result)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	int  ret_val = -(EFAULT);
	uint32_t i;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	if (pvalue == NULL)
		return ret_val;

	for (i = 0; i < sensor_info_arr_len; i++) {
		if (strcasecmp(sensor_info_arr[i].name, pvalue) == 0) {
			*result = sensor_info_arr[i].name;
			ret_val = 0;
			break;
		}
	}

	if (ret_val)
		msg("%s: Unknown sensor %s\n", __func__, pvalue);

	if (ptmp)
		*ppos = ptmp + 1;
	else
		*ppos = pos + strlen(pvalue);
	return ret_val;
}

int parse_device_field(char **ppos, char **result)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	int  ret_val = -(EFAULT);
	uint32_t i;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	if (pvalue == NULL)
		return ret_val;

	for (i = 0; i < device_info_arr_len; i++) {
		if (strcasecmp(device_info_arr[i].name, pvalue) == 0) {
			*result = device_info_arr[i].name;
			ret_val = 0;
			break;
		}
	}

	if (ret_val)
		msg("%s: Unknown device %s\n", __func__, pvalue);

	if (ptmp)
		*ppos = ptmp + 1;
	else
		*ppos = pos + strlen(pvalue);
	return ret_val;
}

static int parse_action_info_values(char **ppos,
				     struct action_t result[THRESHOLDS_MAX][ACTIONS_MAX],
				     uint32_t *num_actions)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	uint32_t nresult = 0;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		char *pstr = NULL;
		char *psave2 = NULL;
		uint32_t count = 0;

		pstr = strtok_r(pvalue , "+", &psave2);
		while (pstr != NULL) {
			dbgmsg("Found action '%s'\n", pstr);

			result[nresult][count].info = (int)strtod(pstr, NULL);
			count++;
			if (count >= ACTIONS_MAX)
				break;

			pstr = strtok_r(NULL, "+", &psave2);
		}

		num_actions[nresult++] = count;
		if (nresult >= THRESHOLDS_MAX)
			break;
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}

	dbgmsg("No. of items found : %d\n", nresult);

	*ppos = ptmp + 1;
	return (int)nresult;
}

int parse_values(char **ppos, double *result)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	uint32_t nresult = 0;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		result[nresult] = strtod(pvalue, NULL);
		nresult++;
		if (nresult >= THRESHOLDS_MAX)
			break;
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}

	dbgmsg("No. of items found : %d\n", nresult);

	*ppos = ptmp + 1;
	return (int)nresult;
}

static int *parse_weight_values(char **ppos, int *num_weight)
{
	char *pos = *ppos;
	char *ptmp = NULL;
	char *pvalue, *psave = NULL;
	int nresult = 0;
	int sum = 0;

	int *weight_ptr = (int*)malloc(sizeof(int) * sensor_info_arr_len);

	if (weight_ptr == NULL)
		goto handle_return;

	ptmp = strchr(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		weight_ptr[nresult] = (int)strtod(pvalue, NULL);
		dbgmsg("Found weight '%d'\n", weight_ptr[nresult]);
		sum += weight_ptr[nresult];
		nresult++;
		if ((uint32_t)nresult >= sensor_info_arr_len)
			break;
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}
	if (sum != 100)
		dbgmsg("%s: Weights sum: %d don't add up 100\n",
				__func__, sum);

handle_return:
	dbgmsg("No. of items found : %d\n", nresult);
	if (ptmp != NULL)
		*ppos = ptmp + 1;

	if (nresult == 0) {
		if (weight_ptr)
			free(weight_ptr);
		weight_ptr = NULL;
	}
	*num_weight = nresult;

	return weight_ptr;
}

static int parse_tm_section(struct setting_info *section, char *pos,
			    char *end_of_section)
{
	int  i;
	char *maxpos = end_of_section;
	int error_found = 0;
	char *idx;
	int in_field = FIELD_IDX_MAX;

	dbgmsg("%s: Parsing section %s %s\n", __func__, section->desc, pos);
	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			skip_space(&pos);
			break;

		default:
			if (in_field != FIELD_IDX_MAX) {
				uint32_t j;
				int    num;
				double values[THRESHOLDS_MAX];
				uint32_t    num_actions[THRESHOLDS_MAX];
				struct action_t  actions[THRESHOLDS_MAX][ACTIONS_MAX];
				char*  result_str;

				memset(&actions, 0x0, sizeof(actions));

				switch (in_field) {
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num != 1)
					{
						msg("Invalid sampling value\n");
						error_found = 1;
						break;
					}
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					section->data.tm.sampling_period_ms = (uint32_t)values[0];
					break;
				case THRESHOLDS:
					num = parse_values(&pos, values);
					if (num < 1) {
						msg("Invalid thresholds\n");
						error_found = 1;
						break;
					}
					section->data.tm._n_thresholds = (uint32_t)num;
					for (i = 0; i < num; i++) {
						section->data.tm.t[i].lvl_trig = (int)values[i];
					}
					break;
				case THRESHOLDS_CLR:
					num = parse_values(&pos, values);
					if (num < 1) {
						msg("Invalid thresholds_clr\n");
						error_found = 1;
						break;
					}
					section->data.tm._n_to_clear = (uint32_t)num;
					for (i = 0; i < num; i++) {
						section->data.tm.t[i].lvl_clr = (int)values[i];
					}
					break;
				case ACTIONS:
					num = parse_action_values(&pos, actions, num_actions);
					if (num < 1) {
						msg("Invalid actions\n");
						error_found = 1;
						break;
					}
					section->data.tm._n_actions = (uint32_t)num;
					for (i = 0; i < num; i++) {
						section->data.tm.t[i].num_actions = num_actions[i];
						for (j = 0; j < num_actions[i]; j++) {
							strlcpy(section->data.tm.t[i].actions[j].device,
								actions[i][j].device, DEVICES_MAX_NAME_LEN);
							dbgmsg("Threshold %d Action[%d] : %s\n", i, j,
							    section->data.tm.t[i].actions[j].device);
						}
					}
					break;
				case ACTION_INFO:
					num = parse_action_info_values(&pos, actions, num_actions);
					if (num < 1) {
						msg("Invalid action info\n");
						error_found = 1;
						break;
					}
					section->data.tm._n_action_info  = (uint32_t)num;
					for (i = 0; i < num; i++) {
						for (j = 0; j < num_actions[i]; j++) {
							section->data.tm.t[i].actions[j].info = actions[i][j].info;
							dbgmsg("Threshold %d Action Info[%d] : %d\n", i, j,
							    actions[i][j].info);
						}
					}
					break;
				case SENSOR_FIELD:
					result_str = NULL;
					if (parse_sensor_field(&pos, &result_str) != 0) {
						msg("Invalid sensor\n");
						error_found = 1;
						break;
					}
					strlcpy(section->data.tm.sensor, result_str,
						MAX_SENSOR_NAME_LEN);
					break;
				case DESCENDING:
					dbgmsg("Descending threshold triggers\n");
					section->data.tm.descending_thresh = 1;
					break;
				case DISABLE:
					dbgmsg("Disable section %s\n", section->desc);
					section->disable = 1;
					break;
				case OVERRIDE:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid override value\n");
						error_found = 1;
						break;
					}
					section->data.tm.override = (int)values[0];
					break;
				}
				in_field = FIELD_IDX_MAX;
				break;
			}

			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < ARRAY_SIZE(tm_fields); i++) {
				if (strcasecmp(pos, tm_fields[i].name) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", tm_fields[i].name);
				in_field = tm_fields[i].type;
				break;
			}
			if (i == ARRAY_SIZE(tm_fields)) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			break;
		}

		if (error_found) {
			return -(EFAULT);
		}
	}
	return 0;
}

static int parse_pid_section(struct setting_info *section, char *pos,
			     char *end_of_section)
{
	int i = 0;
	char *maxpos = end_of_section;
	int error_found = 0;
	char *idx;
	int in_field = FIELD_IDX_MAX;

	dbgmsg("%s: Parsing section %s %s\n", __func__, section->desc, pos);

	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			skip_space(&pos);
			break;

		default:
			if (in_field != FIELD_IDX_MAX) {
				int    num;
				double values[THRESHOLDS_MAX];
				float  fvalue;
				char   *result_str;

				switch (in_field) {
				case P_CONST:
					if (parse_float(&pos, &fvalue) == 0) {
						section->data.pid.p_const = fvalue;
						dbgmsg("%s: %f\n", pid_fields[i].name, fvalue);
					}
					break;
				case I_CONST:
					if (parse_float(&pos, &fvalue) == 0) {
						section->data.pid.i_const = fvalue;
						dbgmsg("%s: %f\n", pid_fields[i].name, fvalue);
					}
					break;
				case D_CONST:
					if (parse_float(&pos, &fvalue) == 0) {
						section->data.pid.d_const = fvalue;
						dbgmsg("%s: %f\n", pid_fields[i].name, fvalue);
					}
					break;
				case ERR_WEIGHT:
					if (parse_float(&pos, &fvalue) == 0) {
						section->data.pid.err_weight = fvalue;
						dbgmsg("%s: %f\n", pid_fields[i].name, fvalue);
					}
					break;
				case I_SAMPLES:
					num = parse_values(&pos, values);
					if ((num != 1) || (values[0] <= 0)) {
						msg("Sampling window invalid, using %d",
						    I_SAMPLES);
						error_found = 1;
						break;
					}
					section->data.pid.i_samples = (int)values[0];
					break;
				case UNITS_PER_CALC:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid units per C value\n");
						error_found = 1;
						break;
					}
					section->data.pid.units_per_C = (int)values[0];
					break;
				case SET_POINT:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid set_point value\n");
						error_found = 1;
						break;
					}
					section->data.pid.set_point = (int)values[0];
					break;
				case SET_POINT_CLR:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid set_point value\n");
						error_found = 1;
						break;
					}
					section->data.pid.set_point_clr = (int)values[0];
					break;
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num != 1)
					{
						msg("Invalid sampling value\n");
						error_found = 1;
						break;
					}
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					section->data.pid.sampling_period_ms = (uint32_t)values[0];
					break;
				case SENSOR_FIELD:
					result_str = NULL;
					if (parse_sensor_field(&pos, &result_str) != 0) {
						msg("Invalid sensor\n");
						error_found = 1;
						break;
					}
					strlcpy(section->data.pid.sensor, result_str,
						MAX_SENSOR_NAME_LEN);
					break;
				case DEVICE_FIELD:
					result_str = NULL;
					if (parse_device_field(&pos, &result_str) != 0) {
						msg("Invalid device\n");
						error_found = 1;
						break;
					}
					strlcpy(section->data.pid.device, result_str,
						DEVICES_MAX_NAME_LEN);
					break;
				case DISABLE:
					dbgmsg("Disable section %s\n", section->desc);
					section->disable = 1;
					break;
				case OVERRIDE:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid override value\n");
						error_found = 1;
						break;
					}
					section->data.pid.override = (int)values[0];
					break;
				}
				in_field = FIELD_IDX_MAX;
				break;
			}

			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < ARRAY_SIZE(pid_fields); i++) {
				if (strcasecmp(pos, pid_fields[i].name) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", pid_fields[i].name);
				in_field = pid_fields[i].type;
				break;
			}
			if (i == ARRAY_SIZE(pid_fields)) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			break;
		}

		if (error_found) {
			return -(EFAULT);
		}
	}
	return 0;
}

static int parse_ss_section(struct setting_info *section, char *pos,
			    char *end_of_section)
{
	int i;
	char *maxpos = end_of_section;
	int error_found = 0;
	char *idx;
	int in_field = FIELD_IDX_MAX;

	dbgmsg("%s: Parsing section %s %s\n", __func__, section->desc, pos);

	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			skip_space(&pos);
			break;

		default:
			if (in_field != FIELD_IDX_MAX) {
				int    num;
				double values[THRESHOLDS_MAX];
				char   *result_str;

				switch (in_field) {
				case SET_POINT:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid set_point value\n");
						error_found = 1;
						break;
					}
					section->data.ss.set_point = (int)values[0];
					break;
				case SET_POINT_CLR:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid set_point value\n");
						error_found = 1;
						break;
					}
					section->data.ss.set_point_clr = (int)values[0];
					break;
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num != 1)
					{
						msg("Invalid sampling value\n");
						error_found = 1;
						break;
					}
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					section->data.ss.sampling_period_ms = (uint32_t)values[0];
					break;
				case SENSOR_FIELD:
					result_str = NULL;
					if (parse_sensor_field(&pos, &result_str) != 0) {
						msg("Invalid sensor\n");
						error_found = 1;
						break;
					}
					strlcpy(section->data.ss.sensor, result_str,
						MAX_SENSOR_NAME_LEN);
					break;
				case DEVICE_FIELD:
					result_str = NULL;
					if (parse_device_field(&pos, &result_str) != 0) {
						msg("Invalid device\n");
						error_found = 1;
						break;
					}
					strlcpy(section->data.ss.device, result_str,
						DEVICES_MAX_NAME_LEN);
					break;
				case TIME_CONSTANT:
					num = parse_values(&pos, values);
					if (num != 1)
					{
						msg("Invalid Time Constant value\n");
						error_found = 1;
						break;
					}
					if (values[0] < 0) {
						msg("Invalid TC specified %d ms",
						    (int)values[0]);
						values[0] = 0;
					}
					section->data.ss.time_constant = (int)values[0];
					break;
				case DISABLE:
					dbgmsg("Disable section %s\n", section->desc);
					section->disable = 1;
					break;
				case OVERRIDE:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid override value\n");
						error_found = 1;
						break;
					}
					section->data.ss.override = (int)values[0];
					break;
				}
				in_field = FIELD_IDX_MAX;
				break;
			}

			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < ARRAY_SIZE(ss_fields); i++) {
				if (strcasecmp(pos, ss_fields[i].name) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", ss_fields[i].name);
				in_field = ss_fields[i].type;
				break;
			}
			if (i == ARRAY_SIZE(ss_fields)) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			break;
		}

		if (error_found) {
			return -(EFAULT);
		}
	}
	return 0;
}

static int parse_eq_section(struct setting_info *section, char *pos,
			    char *end_of_section)
{
	int i;
	char *maxpos = end_of_section;
	int error_found = 0;
	char *idx;
	int in_field = FIELD_IDX_MAX;

	dbgmsg("%s: Parsing section %s %s\n", __func__, section->desc, pos);

	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			skip_space(&pos);
			break;

		default:
			if (in_field != FIELD_IDX_MAX) {
				int    num;
				double values[THRESHOLDS_MAX];
				char   *result_str;
				char **sensor_list;

				switch (in_field) {
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num != 4)
					{
						msg("Invalid sampling value\n");
						error_found = 1;
						break;
					}
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					section->data.eq.sampling_period_ms =
						(uint32_t)values[0];

					if (values[1] < SAMPLING_MS_MINIMUM) {
						values[1] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					section->data.eq.long_sampling_period_ms =
						(uint32_t)values[1];

					section->data.eq.long_sampling_cnt =
						(uint32_t)values[2];
					if (section->data.eq.long_sampling_cnt == 0)
						section->data.eq.long_sampling_cnt = 10;

					section->data.eq.pre_cal_delay_ms =
						(uint32_t)values[3];

					break;
				case TEMP_RANGE:
					num = parse_values(&pos, values);
					if ((num != 3) || (values[0] < 0) ||
					    (values[1] < 0) || (values[2] < 0)) {
						msg("Invalid temp_range values\n");
						error_found = 1;
						break;
					}
					section->data.eq.temp_range =
						(uint32_t)values[0];
					section->data.eq.sensor_delta =
						(uint32_t)values[1];
					section->data.eq.long_sample_range =
						(uint32_t)values[2];
					break;
				case MAX_TEMP:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid max_temp value\n");
						error_found = 1;
						break;
					}
					section->data.eq.max_temp =
						(int)values[0];
					break;
				case SENSOR_FIELD:
					result_str = NULL;
					if (parse_sensor_field(&pos, &result_str) != 0) {
						msg("No valid sensor\n");
						error_found = 1;
						break;
					} else {
						char *tmp = (char*)malloc(sizeof(char) *
								      (strnlen(result_str,
									       MAX_SENSOR_NAME_LEN) + 1));
						if (tmp) {
							section->data.eq.sensor = tmp;
							strlcpy(section->data.eq.sensor, result_str,
								MAX_SENSOR_NAME_LEN);
						} else {
							msg("%s: Allocate failed\n", __func__);
							error_found = 1;
							break;
						}
					}
					break;
				case SENSORS_FIELD:
					sensor_list = parse_sensor_values(&pos, &num);
					if ((num < 1) || (sensor_list == NULL)) {
						msg("No valid sensors list\n");
						error_found = 1;
						break;
					}
					section->data.eq.sensor_list = sensor_list;
					section->data.eq.list_cnt = (uint8_t)num;
					break;
				case OFFSET:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid offset value\n");
						error_found = 1;
						break;
					}
					section->data.eq.offset =
						(int)values[0];
					break;
				case DISABLE:
					dbgmsg("Disable section %s\n", section->desc);
					section->disable = 1;
					break;
				}
				in_field = FIELD_IDX_MAX;
				break;
			}

			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < ARRAY_SIZE(eq_fields); i++) {
				if (strcasecmp(pos, eq_fields[i].name) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", eq_fields[i].name);
				in_field = eq_fields[i].type;
				break;
			}
			if (i == ARRAY_SIZE(eq_fields)) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			break;
		}

		if (error_found) {
			return -(EFAULT);
		}
	}
	return 0;
}

static int parse_v_section(struct setting_info *section, char *pos,
			    char *end_of_section)
{
	int i;
	char *maxpos = end_of_section;
	int error_found = 0;
	char *idx;
	int in_field = FIELD_IDX_MAX;

	dbgmsg("%s: Parsing section %s %s\n", __func__, section->desc, pos);

	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			skip_space(&pos);
			break;

		default:
			if (in_field != FIELD_IDX_MAX) {
				int    num;
				double values[THRESHOLDS_MAX];
				char   *result_str;
				char **sensor_list;
				int *weight_list;

				switch (in_field) {
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num != 1)
					{
						msg("Invalid sampling value\n");
						error_found = 1;
						break;
					}
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					section->data.v.sampling_period_ms = (uint32_t)values[0];
					break;
				case SET_POINT:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid set_point value\n");
						error_found = 1;
						break;
					}
					section->data.v.set_point = (int)values[0];
					break;
				case SET_POINT_CLR:
					num = parse_values(&pos, values);
					if (num != 1) {
						msg("Invalid set_point value\n");
						error_found = 1;
						break;
					}
					section->data.v.set_point_clr = (int)values[0];
					break;
				case SENSOR_FIELD:
					result_str = NULL;
					if (parse_sensor_field(&pos, &result_str) != 0) {
						msg("No valid sensor\n");
						error_found = 1;
						break;
					} else {
						char *tmp = (char*)malloc(sizeof(char) *
								      (strnlen(result_str,
									       MAX_SENSOR_NAME_LEN) + 1));
						if (tmp) {
							section->data.v.trip_sensor = tmp;
							strlcpy(section->data.v.trip_sensor, result_str,
								MAX_SENSOR_NAME_LEN);
						} else {
							msg("%s: Allocate failed\n", __func__);
							error_found = 1;
							break;
						}
					}
					break;
				case SENSORS_FIELD:
					sensor_list = parse_sensor_values(&pos, &num);
					if ((num < 1) || (sensor_list == NULL)) {
						msg("No valid sensors list\n");
						error_found = 1;
						break;
					}
					section->data.v.sensor_list = sensor_list;
					section->data.v.list_cnt = (uint8_t)num;
					break;
				case WEIGHTS:
					weight_list = parse_weight_values(&pos, &num);
					if ((num < 1) || (weight_list == NULL)) {
						msg("Invalid weights\n");
						error_found = 1;
						break;
					}
					section->data.v.weight_list = weight_list;
					section->data.v.weight_cnt = (uint32_t)num;
					break;
				}
				in_field = FIELD_IDX_MAX;
				break;
			}
			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < ARRAY_SIZE(v_fields); i++) {
				if (strcasecmp(pos, v_fields[i].name) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", v_fields[i].name);
				in_field = v_fields[i].type;
				break;
			}
			if (i == ARRAY_SIZE(v_fields)) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			break;
		}

		if (error_found) {
			return -(EFAULT);
		}
	}
	return 0;
}
static int parse_section(struct setting_info *section, char *pos,
			 char *end_of_section)
{
	char *idx;
	char *maxpos = end_of_section;

	dbgmsg("Parsing section %s\n", section->desc);

	/* Back up end of section so as not to destroy the start of another
	   section */
	if (*maxpos == '[')
		--maxpos;

	*maxpos = '\0';

	/* Skip to first actual character. */
	skip_space(&pos);

	if (section->dynamic) {
		idx = strpbrk(pos, "\t\r\n ");
		if (!idx) {
			msg("%s: Error in section\n", __func__);
			return -(EFAULT);
		}
		*idx = '\0';

		/* First field should be algo_type */
		if (strcasecmp(pos, "algo_type") != 0) {
			msg("%s: algo_type field not first field in section\n", __func__);
			return -(EFAULT);
		}

		pos = idx + 1;
		skip_space(&pos);
		section->algo_type = parse_algo(&pos);
		skip_space(&pos);
	}

	section->disable = 0;
	section->err_disable = 0;
	switch (section->algo_type) {
	case MONITOR_ALGO_TYPE:
		if (parse_tm_section(section, pos, maxpos) != 0)
			section->err_disable = 1;
		break;
	case PID_ALGO_TYPE:
		if (parse_pid_section(section, pos, maxpos) != 0)
			section->err_disable = 1;
		break;
	case SS_ALGO_TYPE:
		if (parse_ss_section(section, pos, maxpos) != 0)
			section->err_disable = 1;
		break;
	case EQUILIBRIUM_TYPE:
		if (parse_eq_section(section, pos, maxpos) != 0)
			section->err_disable = 1;
		break;
	case VIRTUAL_SENSOR_TYPE:
		if (parse_v_section(section, pos, maxpos) != 0)
			section->err_disable = 1;
		break;
	default:
		dbgmsg("%s: Unknown algo type for section %s", __func__,
		       section->desc);
		section->err_disable = 1;
		break;
	}
	return 0;
}

struct setting_info *find_section_id(struct thermal_setting_t *settings,
				     const char *desc)
{
	struct setting_info *curr = NULL;

	curr = settings->list;
	while (curr) {
		if (strncasecmp(curr->desc, desc, MAX_ALGO_DESC) == 0)
			return curr;
		curr = curr->next;
	}
	return NULL;
}

void parse_comments(char *buf) {
	char *pos;

	pos = strchr(buf, '#');
	while (pos != NULL) {
		while (*pos != '\n') {
			*pos = ' ';
			pos++;
		}
		pos = strchr(pos + 1, '#');
	}
}

char *strip_unknown(char *start)
{
	char *pos = start;

	while(*start != '\0') {
		*pos = *start;
		/* Characters between 32 and 126 are printable characters */
		if ((*pos >= 32 && *pos <= 126) || *pos == '\n')
			pos++;
		start++;
	}
	*pos = '\0';
	return pos;
}

int parse_config(struct thermal_setting_t *settings, int fd)
{
	char buf[CONFIG_FILE_SIZE_MAX + 2];
	ssize_t sz;
	int i;
	char *pos = buf;
	char *maxpos;
	char *end_of_section = NULL;
	int error_found = 0;
	char *idx;
	struct setting_info *in_section = NULL;
	int in_field = FIELD_IDX_MAX;

	if (fd == -1)
		return -(EIO);

	dbgmsg("Loading configuration file...\n");

	sz = read(fd, buf, CONFIG_FILE_SIZE_MAX);
	if (sz < 0) {
		msg("Failed to read config file\n");
		return -(EIO);
	}
	maxpos = pos + sz;
	dbgmsg("Config file read (%zd bytes)\n", sz);
	*(maxpos) = '\n';
	maxpos++;
	*(maxpos) = '\0';
	buf[CONFIG_FILE_SIZE_MAX + 1] = '\0';

	parse_comments(buf);
	maxpos = strip_unknown(pos);

	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '[':
			idx = strchr(++pos, ']');
			if (!idx) {
				error_found = 1;
				break;
			}
			in_field = FIELD_IDX_MAX;
			*idx = '\0';
			/* Look for existing section */
			in_section = find_section_id(settings, pos);

			/*If it doesn't exist allocate one */
			if (in_section == NULL) {
				in_section = malloc(sizeof(struct setting_info));
				if (in_section == NULL) {
					msg("Failed to alloc tm struct setting_info\n");
					error_found = 1;
					break;
				}
				init_section_settings(in_section);

				/* Insert into the config list head */
				add_setting(settings, in_section);

				strlcpy(in_section->desc, pos, MAX_ALGO_DESC);
				in_section->dynamic = 1;
				dbgmsg("Create section '%s'\n", in_section->desc);
			} else
				dbgmsg("Found section '%s'\n", in_section->desc);
			pos = idx + 1;

			/* Find end of section */
			end_of_section = strpbrk(pos, "[");
			if (!end_of_section)
				end_of_section = maxpos;

			parse_section(in_section, pos, end_of_section);
			pos = end_of_section;
			break;

		case '\t':
		case '\r':
		case '\n':
		case ' ':
			skip_space(&pos);
			break;

		default:

			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < ARRAY_SIZE(fields); i++) {
				if (strcasecmp(pos, fields[i].name) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", fields[i].name);
				in_field = i;
				break;
			}
			if (i == ARRAY_SIZE(fields)) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			if (in_field != FIELD_IDX_MAX) {
				int    num;
				double values[THRESHOLDS_MAX];

				switch (in_field) {
				case DEBUG:
					debug_output = LOG_LVL_DBG;
					dbgmsg("Debug output enabled from config");
					skip_num(&pos);
					break;
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num < 1)
						break;
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						msg("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					dbgmsg("Setting sampling to %d\n", (int)values[0]);
					settings->sample_period_ms = (uint32_t)values[0];
					break;
				}
				in_field = FIELD_IDX_MAX;
			}
			break;
		}

		if (error_found) {
			return -(EFAULT);
		}
	}
	return 0;
}

int threshold_array_compare(const void *x, const void *y)
{
	struct threshold_t *i = (struct threshold_t *)x;
	struct threshold_t *j = (struct threshold_t *)y;

	if (i->lvl_trig < j->lvl_trig)
		return -1;
	else if (i->lvl_trig > j->lvl_trig)
		return 1;
	return 0;
}

int descending_threshold_array_compare(const void *x, const void *y)
{
	struct threshold_t *i = (struct threshold_t *)x;
	struct threshold_t *j = (struct threshold_t *)y;

	if (i->lvl_trig < j->lvl_trig)
		return 1;
	else if (i->lvl_trig > j->lvl_trig)
		return -1;
	return 0;
}

static void fallback_sampling_values(struct thermal_setting_t *settings,
				     struct setting_info * section)
{
	/* Fallback to default sampling times if sampling period
	   was not configured */
	if (settings == NULL ||
	    section == NULL) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	if (settings->sample_period_ms > 0 &&
	    section->data.tm.sampling_period_ms == 0) {
		section->data.tm.sampling_period_ms =
			settings->sample_period_ms;
		dbgmsg("Using configured default sampling period "
		       "%dms for section[%s] sensor[%s]\n",
		       settings->sample_period_ms,
		       section->desc,
		       section->data.tm.sensor);
	} else if (settings->sample_period_ms == 0 &&
		   section->data.tm.sampling_period_ms == 0) {
		section->data.tm.sampling_period_ms =
			SAMPLING_MS_DEFAULT;
		dbgmsg("Using default sampling period %dms "
		       "for section[%s] sensor[%s]\n",
		       SAMPLING_MS_DEFAULT,
		       section->desc,
		       section->data.tm.sensor);
	}
}

void validate_config(struct thermal_setting_t *settings)
{
	struct setting_info *s, *next_s = NULL;
	uint32_t j, k;

	s = settings->list;
	while (s != NULL) {
		next_s = s->next;

		/* Only validate monitor type configs */
		if (s->algo_type != MONITOR_ALGO_TYPE) {
			s = next_s;
			continue;
		}

		/* Disable no threshold entries */
		if (s->data.tm._n_thresholds == 0) {
			s->err_disable = 1;
			s = next_s;
			continue;
		}

		/* Fill out the number of thresholds based on min entry */
		s->data.tm.num_thresholds = s->data.tm._n_thresholds;
		if (s->data.tm.num_thresholds > s->data.tm._n_to_clear)
			s->data.tm.num_thresholds = s->data.tm._n_to_clear;
		if (s->data.tm.num_thresholds > s->data.tm._n_actions)
			s->data.tm.num_thresholds = s->data.tm._n_actions;
		if (s->data.tm.num_thresholds > s->data.tm._n_action_info)
			s->data.tm.num_thresholds = s->data.tm._n_action_info;

		/* Ensure thresolds are in ascending or descending order
		   depending on algorithm instance configuration */
		if (s->data.tm.num_thresholds > 1) {
			if (s->data.tm.descending_thresh)
				qsort(s->data.tm.t, s->data.tm.num_thresholds,
				      sizeof(struct threshold_t),
				      descending_threshold_array_compare);
			else
				qsort(s->data.tm.t, s->data.tm.num_thresholds,
				      sizeof(struct threshold_t),
				      threshold_array_compare);
		}

		/* Sort so as to move shutdown to last action */
		for (j = 0; j < s->data.tm.num_thresholds; j++) {
			for (k = 0; k < s->data.tm.t[j].num_actions; k++) {
				if ((k < s->data.tm.t[j].num_actions - 1) &&
				    (s->data.tm.t[j].actions[k].device != NULL) &&
				    (strncasecmp(s->data.tm.t[j].actions[k].device,
						 "shutdown", DEVICES_MAX_NAME_LEN) == 0)) {
					struct action_t temp_action;

					/* Swap entries */
					memcpy(&temp_action,
					       &(s->data.tm.t[j].actions[k]),
					       sizeof(struct action_t));
					memcpy(&(s->data.tm.t[j].actions[k]),
					       &(s->data.tm.t[j].actions[k+1]),
					       sizeof(struct action_t));
					memcpy(&(s->data.tm.t[j].actions[k+1]),
					       &temp_action,
					       sizeof(struct action_t));
				}
			}
		}

		/* Fallback on default sampling times, if not configured */
		fallback_sampling_values(settings, s);

		dbgmsg("Desc '%s' Sensor '%s' config:  sampling %d\n",
		       s->desc, s->data.tm.sensor, s->data.tm.sampling_period_ms);
		for (j = 0; j < s->data.tm.num_thresholds; j++) {
			for (k = 0; k < s->data.tm.t[j].num_actions; k++)
				dbgmsg(" %d %d %s %d\n",
				       s->data.tm.t[j].lvl_trig,
				       s->data.tm.t[j].lvl_clr,
				       (s->data.tm.t[j].actions[k].device != NULL) ?
				       (s->data.tm.t[j].actions[k].device):("unknown"),
				       s->data.tm.t[j].actions[k].info);
		}
		s = next_s;
	}
}

int load_config(struct thermal_setting_t *settings, const char *pFName)
{
	int config_fd;
	int ret_val = 0;
	const char *cf = (pFName) ? pFName : CONFIG_FILE_DEFAULT;

	config_fd = open(cf, O_RDONLY);
	if (config_fd < 0) {
		msg("Unable to open config file '%s'\n", cf);
		ret_val = -(EIO);
		goto error_handler;
	}

	/* Get sensors list count*/
	if (sensors_manager_get_list(NULL, &sensor_info_arr_len)) {
		msg("Failed to get sensor list length\n");
		ret_val = -(EFAULT);
		goto error_handler;
	}

	sensor_info_arr = (struct sensor_info_type *)
		malloc(sizeof(struct sensor_info_type)*sensor_info_arr_len);
	if (sensor_info_arr == NULL) {
		msg("Failed to alloc ts_info_arr\n");
		ret_val = -(EFAULT);
		goto error_handler;
	}

	if (sensors_manager_get_list(sensor_info_arr,
				     &sensor_info_arr_len)) {
		msg("Failed to get sensor list\n");
		ret_val = -(EFAULT);
		goto error_handler;
	}

	if (devices_manager_get_list(NULL, &device_info_arr_len)) {
		msg("Failed to get tmd list length\n");
		ret_val = -(EFAULT);
		goto error_handler;
	}

	device_info_arr = (struct device_info *)
		malloc(sizeof(struct device_info)*device_info_arr_len);

	if (device_info_arr == NULL) {
		msg("Failed to alloc tmd_info_arr\n");
		ret_val = -(EFAULT);
		goto error_handler;
	}

	if (devices_manager_get_list(device_info_arr,
			     &device_info_arr_len)) {
		msg("Failed to get tmd list\n");
		ret_val = -(EFAULT);
		goto error_handler;
	}

	if (parse_config(settings, config_fd)) {
		msg("Failed to parse config file '%s'\n", cf);
		close(config_fd);
		ret_val = -(EFAULT);
		goto error_handler;
	}

	validate_config(settings);

error_handler:
	close(config_fd);
	if (ret_val) {
		if (sensor_info_arr)
			free(sensor_info_arr);

		if (device_info_arr)
			free(device_info_arr);
	}
	return ret_val;
}

static void print_alias_comments(void)
{
	uint32_t i;

	if (sensors_manager_get_list(NULL, &sensor_info_arr_len))
		return;

	sensor_info_arr = (struct sensor_info_type *)
		malloc(sizeof(struct sensor_info_type)*sensor_info_arr_len);
	if (sensor_info_arr == NULL)
		return;

	if (sensors_manager_get_list(sensor_info_arr, &sensor_info_arr_len))
		goto alias_error;
	printf("# SENSOR : ALIAS\n");
	for (i = 0; i < sensor_info_arr_len; i++) {
		char alias_buf[MAX_SENSOR_NAME_LEN];
		if (sensors_get_alias(sensor_info_arr[i].name, alias_buf) == 0)
			printf("# %s : %s\n", sensor_info_arr[i].name,
				alias_buf);
	}
alias_error:
	free(sensor_info_arr);
}

static void print_gbl_setting(struct thermal_setting_t *settings, int flag)
{
	char *config_file;

	if (settings->sample_period_ms != 0)
		PRINT_SETTING(flag, "sampling %d", settings->sample_period_ms);
	if (debug_output)
		PRINT_SETTING(flag, "debug");
	config_file = get_target_default_thermal_config_file();
	PRINT_SETTING(flag, "#Conf file: %s", (config_file) ? config_file : CONFIG_FILE_DEFAULT);
}

static void print_tm_setting(struct setting_info *setting, int flag)
{
	char info_buf[256];
	char int_buf[UINT_BUF_MAX];
	uint8_t idx, action_idx;
	struct tm_setting *tm = &setting->data.tm;

	PRINT_SETTING(flag, "[%s]", setting->desc);
	if (setting->dynamic)
		PRINT_SETTING(flag, "algo_type monitor");
	else
		PRINT_SETTING(flag, "#algo_type monitor");
	PRINT_SETTING(flag, "sampling %d\nsensor %s",
	     tm->sampling_period_ms,
	     tm->sensor);

	snprintf(info_buf, sizeof(info_buf), "thresholds");
	for (idx = 0; idx < tm->num_thresholds; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
		    sizeof(info_buf))
			break;
		snprintf(int_buf, sizeof(int_buf), "%d", tm->t[idx].lvl_trig);
		if (strlcat(info_buf, int_buf, sizeof(info_buf)) >=
			    sizeof(info_buf))
			break;
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);

	snprintf(info_buf, sizeof(info_buf), "thresholds_clr");
	for (idx = 0; idx < tm->num_thresholds; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
		    sizeof(info_buf))
			break;

		snprintf(int_buf, sizeof(int_buf), "%d", tm->t[idx].lvl_clr);
		if (strlcat(info_buf, int_buf,
			    sizeof(info_buf)) >= sizeof(info_buf))
			break;
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);

	snprintf(info_buf, sizeof(info_buf), "actions");
	for (idx = 0; idx < tm->num_thresholds; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
		    sizeof(info_buf))
			break;

		for (action_idx = 0; action_idx <
		      tm->t[idx].num_actions; action_idx++) {
			if (action_idx > 0) {
				if (strlcat(info_buf, "+",
					    sizeof(info_buf)) >=
				    sizeof(info_buf))
					break;
			}

			if (strlcat(info_buf, tm->t[idx].actions[action_idx].\
				    device, sizeof(info_buf)) >=
			    sizeof(info_buf))
				break;
		}
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);

	snprintf(info_buf, sizeof(info_buf), "action_info");
	for (idx = 0; idx < tm->num_thresholds; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
		    sizeof(info_buf))
			break;
		for (action_idx = 0; action_idx <
		      tm->t[idx].num_actions; action_idx++) {
			if (action_idx > 0) {
				if (strlcat(info_buf, "+",
					    sizeof(info_buf)) >=
				    sizeof(info_buf))
					break;
			}
			snprintf(int_buf, sizeof(int_buf), "%d",
				 tm->t[idx].actions[action_idx].info);
			if (strlcat(info_buf, int_buf, sizeof(info_buf))
			     >= sizeof(info_buf))
				break;
		}
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);

	if (tm->descending_thresh)
		PRINT_SETTING(flag, "descending");
	if (tm->override)
		PRINT_SETTING(flag, "override %d", tm->override);
}

static void print_pid_setting(struct setting_info *setting, int flag)
{
	struct pid_setting *pid = &setting->data.pid;

	PRINT_SETTING(flag, "[%s]", setting->desc);
	if (setting->dynamic)
		PRINT_SETTING(flag, "algo_type pid");
	else
		PRINT_SETTING(flag, "#algo_type pid");
	PRINT_SETTING(flag, "sampling %d\nsensor %s\ndevice %s",
	     pid->sampling_period_ms, pid->sensor, pid->device);
	PRINT_SETTING(flag, "set_point %d\nset_point_clr %d", pid->set_point,
	     pid->set_point_clr);
	PRINT_SETTING(flag, "p_const %f\ni_const %f\nd_const %f", pid->p_const, pid->i_const,
	     pid->d_const);
	PRINT_SETTING(flag, "i_samples %d\ndev_units_per_calc %d", pid->i_samples,
	     pid->units_per_C);
	if (pid->override)
		PRINT_SETTING(flag, "override %d", pid->override);
}

static void print_ss_setting(struct setting_info *setting, int flag)
{
	struct ss_setting *ss = &setting->data.ss;

	PRINT_SETTING(flag, "[%s]", setting->desc);
	if (setting->dynamic)
		PRINT_SETTING(flag, "algo_type ss");
	else
		PRINT_SETTING(flag, "#algo_type ss");
	PRINT_SETTING(flag, "sampling %d\nsensor %s\ndevice %s",
	     ss->sampling_period_ms, ss->sensor, ss->device);
	PRINT_SETTING(flag, "set_point %d\nset_point_clr %d\ntime_constant %d",
	     ss->set_point,
	     ss->set_point_clr,
	     ss->time_constant);
	if (ss->override)
		PRINT_SETTING(flag, "override %d", ss->override);
}

static void print_eq_setting(struct setting_info *setting, int flag)
{
	char info_buf[256];
	int idx;
	struct equilibrium_setting *eq = &setting->data.eq;

	PRINT_SETTING(flag, "[%s]\nsampling %d %d %d %d\nsensor %s", setting->desc,
	     eq->sampling_period_ms, eq->long_sampling_period_ms,
	     eq->long_sampling_cnt, eq->pre_cal_delay_ms,
	     eq->sensor);

	snprintf(info_buf, sizeof(info_buf), "sensors");
	for (idx = 0; idx < eq->list_cnt; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
			    sizeof(info_buf))
			break;
		if (strlcat(info_buf, eq->sensor_list[idx],
			    sizeof(info_buf)) >= sizeof(info_buf))
			break;
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);
	PRINT_SETTING(flag, "temp_range %d %d %d\nmax_temp %d\noffset %d",
	     eq->temp_range, eq->sensor_delta, eq->long_sample_range,
	     eq->max_temp, eq->offset);
}

static void print_vs_setting(struct setting_info *setting, int flag)
{
	uint8_t idx;
	char info_buf[256];
	char int_buf[UINT_BUF_MAX];
	struct virtual_setting *vs = &setting->data.v;

	PRINT_SETTING(flag, "[%s]", setting->desc);
	if (setting->dynamic)
		PRINT_SETTING(flag, "algo_type virtual");
	else
		PRINT_SETTING(flag, "#algo_type virtual");
	PRINT_SETTING(flag, "trip_sensor %s\nset_point %d\n"
	     "set_point_clr %d", vs->trip_sensor,
	     vs->set_point, vs->set_point_clr);
	snprintf(info_buf, sizeof(info_buf), "sensors");
	for (idx = 0; idx < vs->list_cnt; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
		    sizeof(info_buf))
			break;
		if (strlcat(info_buf, vs->sensor_list[idx],
			    sizeof(info_buf)) >= sizeof(info_buf))
			break;
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);

	snprintf(info_buf, sizeof(info_buf), "weights");
	for (idx = 0; idx < vs->weight_cnt; idx++) {
		if (strlcat(info_buf, " ", sizeof(info_buf)) >=
		    sizeof(info_buf))
			break;
		snprintf(int_buf, sizeof(int_buf), "%d", vs->weight_list[idx]);
		if (strlcat(info_buf, int_buf,
			    sizeof(info_buf)) >= sizeof(info_buf))
			break;
	}
	info_buf[sizeof(info_buf) - 1] = '\0';
	PRINT_SETTING(flag, "%s", info_buf);
	PRINT_SETTING(flag, "sampling %d", vs->sampling_period_ms);
}

void print_setting(struct setting_info *setting)
{
	switch (setting->algo_type) {
	case MONITOR_ALGO_TYPE:
		print_tm_setting(setting, PRINT_TO_INFO);
		break;
	case PID_ALGO_TYPE:
		print_pid_setting(setting, PRINT_TO_INFO);
		break;
	case SS_ALGO_TYPE:
		print_ss_setting(setting, PRINT_TO_INFO);
		break;
	case EQUILIBRIUM_TYPE:
		print_eq_setting(setting, PRINT_TO_INFO);
		break;
	case VIRTUAL_SENSOR_TYPE:
		print_vs_setting(setting, PRINT_TO_INFO);
		break;
	default:
		msg("%s: Uknown algo type", __func__);
		break;
	}
}

void print_settings(struct thermal_setting_t *settings)
{
	struct setting_info *setting = settings->list;

	print_alias_comments();
	printf("\n");
	print_gbl_setting(settings, PRINT_TO_STDOUT);
	printf("\n");
	while (setting != NULL && (setting->err_disable || setting->disable))
		setting = setting->next;
	while (setting != NULL) {
		switch (setting->algo_type) {
		case MONITOR_ALGO_TYPE:
			print_tm_setting(setting, PRINT_TO_STDOUT);
			break;
		case PID_ALGO_TYPE:
			print_pid_setting(setting, PRINT_TO_STDOUT);
			break;
		case SS_ALGO_TYPE:
			print_ss_setting(setting, PRINT_TO_STDOUT);
			break;
		case EQUILIBRIUM_TYPE:
			print_eq_setting(setting, PRINT_TO_STDOUT);
			break;
		case VIRTUAL_SENSOR_TYPE:
			print_vs_setting(setting, PRINT_TO_STDOUT);
			break;
		default:
			msg("%s: Uknown algo type", __func__);
			break;
		}
		printf("\n");
		setting = setting->next;
		while (setting != NULL &&
		       (setting->err_disable || setting->disable))
			setting = setting->next;
	}
}
