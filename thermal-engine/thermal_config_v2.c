/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <limits.h>
#include <stringl.h>
#include "thermal.h"
#include "thermal_config.h"

#define MAX_TOKEN_TEXT      MAX(MAX(MAX_ALGO_DESC, DEVICES_MAX_NAME_LEN), \
				MAX_SENSOR_NAME_LEN)

#define ERR_SAMPLING_VAL    0
#define ERR_INT_VAL         INT_MIN
#define ERR_UINT_VAL        0
#define ERR_ISAMPLES_VAL    INT_MIN
#define ERR_SETPOINT_VAL    INT_MIN
#define ERR_SETPOINTCLR_VAL INT_MIN
#define ERR_UNITSPERC_VAL   INT_MIN
#define ERR_PCONST_VAL      -1.0f
#define ERR_ICONST_VAL      -1.0f
#define ERR_DCONST_VAL      -1.0f
#define ERR_FREQ_SCALE      -1.0f
#define ERR_ERRWEIGHT_VAL   -1.0f
#define ERR_TRIG_VAL        INT_MIN
#define ERR_CLR_VAL         INT_MIN
#define ERR_DEVICE_VAL      '\0'
#define ERR_INFO_VAL        INT_MIN
#define ERR_SENSOR_VAL      '\0'
#define ERR_WEIGHT_VAL      INT_MIN

static struct device_info *device_info_arr;
static uint32_t           device_info_arr_len;
static struct sensor_info_type *sensor_info_arr;
static uint32_t           sensor_info_arr_len;

static enum token_type lookahead;
static FILE *configFile;
static char tokenText[MAX_TOKEN_TEXT];
static uint32_t tokenTextLength;
static char **fields;

/* For parse functions:
   If passing in static array,
   cast value to equivelent type.
   (e.g. char sensor[MAX_SENSOR_NAME_LEN] cast value to (char *)).
   If passing in dynamic array or variable,
   cast value to address of type and dereference.
   (e.g. char **sensor_list cast value to *(char ***),
   int sampling_period_ms cast value to *(int *)). */
static int parse_debug(void *value);
static int parse_float(void *value);
static int parse_int(void *value);
static int parse_int_opt(void *value);
static int parse_sensor(void *value);
static int parse_sensors(void *value);
static int parse_device(void *value);
static int parse_thresh(void *value);
static int parse_thresh_clr(void *value);
static int parse_actions(void *value);
static int parse_action_info(void *value);
static int parse_weights(void *value);
static int parse_ints(void *value);

struct table {
	char *field_name;
	int offset;
	int (*callback_func)(void *);
};

struct algorithms {
	char *name;
	enum algo_type type;
};

enum token_type{
	OBRACKET,
	EBRACKET,
	FLOAT,
	INTEGER,
	STRING_DATA,
	STRING_FIELD,
	PLUS,
	END,
	FIELD_IDX_MAX,
};

static struct table global_table[] = {
	{.field_name = "debug", .offset = 0,
		.callback_func = &parse_debug},
	{.field_name = "sampling", .offset = offsetof(struct thermal_setting_t,
		sample_period_ms),
		.callback_func = &parse_int}
};

static struct table pid_table[] = {
	{.field_name = "sampling", .offset = offsetof(struct setting_info,
		data.pid.sampling_period_ms),
		.callback_func = &parse_int},
	{.field_name = "p_const", .offset = offsetof(struct setting_info,
		data.pid.p_const),
		.callback_func = &parse_float},
	{.field_name = "i_const", .offset = offsetof(struct setting_info,
		data.pid.i_const),
		.callback_func = &parse_float},
	{.field_name = "d_const", .offset = offsetof(struct setting_info,
		data.pid.d_const),
		.callback_func = &parse_float},
	{.field_name = "err_weight", .offset = offsetof(struct setting_info,
		data.pid.err_weight),
		.callback_func = &parse_float},
	{.field_name = "i_samples", .offset = offsetof(struct setting_info,
		data.pid.i_samples),
		.callback_func = &parse_int},
	{.field_name = "dev_units_per_calc", .offset = offsetof(struct setting_info,
		data.pid.units_per_C),
		.callback_func = &parse_int},
	{.field_name = "freq_scale", .offset = offsetof(struct setting_info,
		data.pid.freq_scale),
		.callback_func = &parse_float},
	{.field_name = "disable", .offset = offsetof(struct setting_info,
		disable),
		.callback_func = &parse_int_opt},
	{.field_name = "sensor", .offset = offsetof(struct setting_info,
		data.pid.sensor),
		.callback_func = &parse_sensor},
	{.field_name = "device", .offset = offsetof(struct setting_info,
		data.pid.device),
		.callback_func = &parse_device},
	{.field_name = "set_point", .offset = offsetof(struct setting_info,
		data.pid.set_point),
		.callback_func = &parse_int},
	{.field_name = "set_point_clr", .offset = offsetof(struct setting_info,
		data.pid.set_point_clr),
		.callback_func = &parse_int},
	{.field_name = "override", .offset = offsetof(struct setting_info,
		data.pid.override),
		.callback_func = &parse_int_opt}
};

static struct table tm_table[] = {
	{.field_name = "thresholds", .offset = offsetof(struct setting_info,
		data.tm.t),
		.callback_func = &parse_thresh},
	{.field_name = "thresholds_clr", .offset = offsetof(struct setting_info,
		data.tm.t),
		.callback_func = &parse_thresh_clr},
	{.field_name = "actions", .offset = offsetof(struct setting_info,
		data.tm.t),
		.callback_func = &parse_actions},
	{.field_name = "action_info", .offset = offsetof(struct setting_info,
		data.tm.t),
		.callback_func = &parse_action_info},
	{.field_name = "descending", .offset = offsetof(struct setting_info,
		data.tm.descending_thresh),
		.callback_func = &parse_int_opt},
	{.field_name = "sampling", .offset = offsetof(struct setting_info,
		data.tm.sampling_period_ms),
		.callback_func = &parse_int},
	{.field_name = "disable", .offset = offsetof(struct setting_info,
		disable),
		.callback_func = &parse_int_opt},
	{.field_name = "sensor", .offset = offsetof(struct setting_info,
		data.tm.sensor),
		.callback_func = &parse_sensor},
	{.field_name = "override", .offset = offsetof(struct setting_info,
		data.tm.override),
		.callback_func = &parse_int_opt}
};

static struct table ss_table[] = {
	{.field_name = "time_constant", .offset = offsetof(struct setting_info,
		data.ss.time_constant),
		.callback_func = &parse_int},
	{.field_name = "sampling", .offset = offsetof(struct setting_info,
		data.ss.sampling_period_ms),
		.callback_func = &parse_int},
	{.field_name = "disable", .offset = offsetof(struct setting_info,
		disable),
		.callback_func = &parse_int_opt},
	{.field_name = "sensor", .offset = offsetof(struct setting_info,
		data.ss.sensor),
		.callback_func = &parse_sensor},
	{.field_name = "device", .offset = offsetof(struct setting_info,
		data.ss.device),
		.callback_func = &parse_device},
	{.field_name = "set_point", .offset = offsetof(struct setting_info,
		data.ss.set_point),
		.callback_func = &parse_int},
	{.field_name = "set_point_clr", .offset = offsetof(struct setting_info,
		data.ss.set_point_clr),
		.callback_func = &parse_int},
	{.field_name = "override", .offset = offsetof(struct setting_info,
		data.ss.override),
		.callback_func = &parse_int_opt},
	{.field_name = "device_max_limit", .offset = offsetof(struct setting_info,
		data.ss.device_mtgn_max_limit),
		.callback_func = &parse_int_opt}
};

static struct table v_table[] = {
	{.field_name = "weights", .offset = offsetof(struct setting_info,
		data.v.weight_list),
		.callback_func = &parse_weights},
	{.field_name = "trip_sensor", .offset = offsetof(struct setting_info,
		data.v.trip_sensor),
		.callback_func = &parse_sensor},
	{.field_name = "sampling", .offset = offsetof(struct setting_info,
		data.v.sampling_period_ms),
		.callback_func = &parse_int},
	{.field_name = "sensors", .offset = offsetof(struct setting_info,
		data.v.sensor_list),
		.callback_func = &parse_sensors},
	{.field_name = "set_point", .offset = offsetof(struct setting_info,
		data.v.set_point),
		.callback_func = &parse_int},
	{.field_name = "set_point_clr", .offset = offsetof(struct setting_info,
		data.v.set_point_clr),
		.callback_func = &parse_int}
};

static struct table eq_table[] = {
	{.field_name = "temp_range", .offset = offsetof(struct setting_info,
		data.eq.temp_range_list),
		.callback_func = &parse_ints},
	{.field_name = "max_temp", .offset = offsetof(struct setting_info,
		data.eq.max_temp),
		.callback_func = &parse_int},
	{.field_name = "offset", .offset = offsetof(struct setting_info,
		data.eq.offset),
		.callback_func = &parse_int},
	{.field_name = "sampling", .offset = offsetof(struct setting_info,
		data.eq.sampling_list),
		.callback_func = &parse_ints},
	{.field_name = "sensors", .offset = offsetof(struct setting_info,
		data.eq.sensor_list),
		.callback_func = &parse_sensors},
	{.field_name = "disable", .offset = offsetof(struct setting_info,
		disable),
		.callback_func = &parse_int_opt},
	{.field_name = "sensor", .offset = offsetof(struct setting_info,
		data.eq.sensor),
		.callback_func = &parse_sensor}
};

static struct algorithms algos[] = {
	{.name = "monitor", .type = MONITOR_ALGO_TYPE},
	{.name = "pid", .type = PID_ALGO_TYPE},
	{.name = "ss", .type = SS_ALGO_TYPE},
	{.name = "virtual", .type = VIRTUAL_SENSOR_TYPE}
};

static int verify_string(int c)
{
	/* If c is a graphic character and is not a PLUS, OBRACKET, or EBRACKET
	return success */
	if (c >= 33 && c <= 126 && c != '+' && c != ']' && c != '[')
		return 0;
	return 1;
}

static int verify_number(int c)
{
	if (c >= '0' && c <= '9')
		return 0;
	return 1;
}

static int verify_whitespace(int c)
{
	/* Check for comment
	If comment then skip to new line */
	if (c == '#') {
		while (c != '\n' && !feof(configFile)) {
			c = getc(configFile);
		}
	}
	/* Check if c is a graphic character
	If c is not a graphic character then return success */
	if (!(c >= 33 && c <= 126))
		return 0;
	return 1;
}

static void append_token_text(int c)
{
	if (tokenTextLength < MAX_TOKEN_TEXT) {
		tokenText[tokenTextLength] = (char)c;
		if (c != '\0')
			tokenTextLength++;
	}
	if (tokenTextLength == MAX_TOKEN_TEXT) {
		tokenTextLength--;
		tokenText[tokenTextLength] = '\0';
	}
}

static void collect_string(void)
{
	int c;

	while ((c = getc(configFile))) {
		if (verify_string(c) == 0)
			append_token_text(c);
		else {
			ungetc(c, configFile);
			return;
		}
	}
}

static void collect_num(void)
{
	int c;

	while ((c = getc(configFile))) {
		if (verify_number(c) == 0)
			append_token_text(c);
		else if (c == '.')
			append_token_text(c);
		else {
			ungetc(c, configFile);
			return;
		}
	}
}

static int is_string_field(void)
{
	int i;

	for (i = 0; fields[i] != 0; i++) {
		if (strncmp(tokenText, fields[i], tokenTextLength + 1) == 0)
			return 0;
	}
	return 1;
}

static enum token_type next_token(void)
{
	uint32_t i;
	int c = getc(configFile);
	int count;

	tokenTextLength = 0;
	while (verify_whitespace(c) == 0) {
		if (feof(configFile))
			return END;
		c = getc(configFile);
	}
	append_token_text(c);

	if (c == '+') {
		append_token_text('\0');
		return PLUS;
	} else if (c == '[') {
		append_token_text('\0');
		return OBRACKET;
	} else if (c == ']') {
		append_token_text('\0');
		return EBRACKET;
	}

	if (c == '-') {
		c = getc(configFile);
		append_token_text(c);
	}
	if (c == '.') {
		c = getc(configFile);
		append_token_text(c);
	}

	if (verify_number(c) == 0) {
		collect_num();
		append_token_text('\0');
		count = 0;
		for (i = 0; i < tokenTextLength; i++)
			if (tokenText[i] == '.')
				count++;
		if (count == 0)
			return INTEGER;
		else if (count == 1)
			return FLOAT;
		else
			return STRING_DATA;
	} else if (verify_string(c) == 0) {
		collect_string();
		append_token_text('\0');
		if (is_string_field() == 0)
			return STRING_FIELD;
		return STRING_DATA;
	}

	ungetc(c, configFile);
	tokenTextLength--;
	tokenText[tokenTextLength] = '\0';
	return STRING_DATA;
}

static int verify_sensor(char *value)
{
	uint32_t i;

	for (i = 0; i < sensor_info_arr_len; i++)
		if (strncasecmp(sensor_info_arr[i].name, value,
		    MAX_SENSOR_NAME_LEN) == 0)
			return 0;

	return 1;
}

static int verify_device(char *value)
{
	uint32_t i;

	for (i = 0; i < device_info_arr_len; i++)
		if (strncasecmp(device_info_arr[i].name, value,
		    DEVICES_MAX_NAME_LEN) == 0)
			return 0;

	return 1;
}

static int parse_debug(void *value)
{
	if (lookahead == INTEGER) {
		debug_output = atoi(tokenText);
		lookahead = next_token();
	} else
		debug_output = LOG_LVL_DBG;

	if (debug_output)
		info("Debug output enabled from config");
	else
		info("Debug output disabled from config");

	return 0;
}

static int parse_float(void *value)
{
	if (lookahead == FLOAT) {
		*(float *)value = (float)atof(tokenText);
		lookahead = next_token();
	} else
		return 1;

	return 0;
}

static int parse_int(void *value)
{
	if (lookahead == INTEGER) {
		*(int *)value = atoi(tokenText);
		lookahead = next_token();
	} else
		return 1;

	return 0;
}

static int parse_int_opt(void *value)
{
	if (lookahead == INTEGER) {
		*(int *)value = atoi(tokenText);
		lookahead = next_token();
	} else
		*(int *)value = 1;

	return 0;
}

static int parse_sensor(void *value)
{
	char *sensor = (char *)value;

	if (lookahead == STRING_DATA && verify_sensor(tokenText) == 0) {
		strlcpy(sensor, tokenText, MAX_SENSOR_NAME_LEN);
		lookahead = next_token();
	} else
		return 1;

	return 0;
}

static int parse_sensors(void *value)
{
	char **sensor_list = *(char ***)value;
	uint32_t i;

	for (i = 0; lookahead == STRING_DATA && i < sensor_info_arr_len
		    && verify_sensor(tokenText) == 0; i++) {
		strlcpy(sensor_list[i], tokenText, MAX_SENSOR_NAME_LEN);
		lookahead = next_token();
	}
	if (i == 0)
		return 1;
	while (i < sensor_info_arr_len) {
		sensor_list[i][0] = ERR_SENSOR_VAL;
		i++;
	}

	return 0;
}

static int parse_device(void *value)
{
	char *device = (char *)value;

	if (lookahead == STRING_DATA && verify_device(tokenText) == 0) {
		strlcpy(device, tokenText, DEVICES_MAX_NAME_LEN);
		lookahead = next_token();
	} else
		return 1;

	return 0;
}

static int parse_thresh(void *value)
{
	uint32_t i;
	struct threshold_t *t_ptr = (struct threshold_t *)value;

	for (i = 0; lookahead == INTEGER && i < THRESHOLDS_MAX; i++) {
		t_ptr[i].lvl_trig = atoi(tokenText);
		lookahead = next_token();
	}
	if (i == 0)
		return 1;
	while(i < THRESHOLDS_MAX) {
		t_ptr[i].lvl_trig = ERR_TRIG_VAL;
		i++;
	}

	return 0;
}

static int parse_thresh_clr(void *value)
{
	uint32_t i;
	struct threshold_t *t_ptr = (struct threshold_t *)value;

	for (i = 0; lookahead == INTEGER && i < THRESHOLDS_MAX; i++) {
		t_ptr[i].lvl_clr = atoi(tokenText);
		lookahead = next_token();
	}
	if (i == 0)
		return 1;
	while(i < THRESHOLDS_MAX) {
		t_ptr[i].lvl_clr = ERR_CLR_VAL;
		i++;
	}

	return 0;
}

static int parse_actions(void *value)
{
	uint32_t i, j;
	struct threshold_t *t_ptr = (struct threshold_t *)value;

	for (i = 0; lookahead == STRING_DATA && i < THRESHOLDS_MAX
		    && verify_device(tokenText) == 0; i++) {
		strlcpy(t_ptr[i].actions[0].device, tokenText,
			DEVICES_MAX_NAME_LEN);
		lookahead = next_token();
		for (j = 1; lookahead == PLUS && j < ACTIONS_MAX; j++) {
			lookahead = next_token();
			if (lookahead == STRING_DATA && verify_device(tokenText) == 0) {
				strlcpy(t_ptr[i].actions[j].device, tokenText,
					DEVICES_MAX_NAME_LEN);
				lookahead = next_token();
			}
		}
		while(j < ACTIONS_MAX) {
			t_ptr[i].actions[j].device[0] = ERR_DEVICE_VAL;
			j++;
		}
	}
	if (i == 0)
		return 1;
	while(i < THRESHOLDS_MAX) {
		t_ptr[i].actions[0].device[0] = ERR_DEVICE_VAL;
		i++;
	}

	return 0;
}

static int parse_action_info(void *value)
{
	uint32_t i, j;
	struct threshold_t *t_ptr = (struct threshold_t *)value;

	for (i = 0; lookahead == INTEGER && i < THRESHOLDS_MAX; i++) {
		t_ptr[i].actions[0].info = atoi(tokenText);
		lookahead = next_token();
		for (j = 1; lookahead == PLUS && j < ACTIONS_MAX; j++) {
			lookahead = next_token();
			if (lookahead == INTEGER) {
				t_ptr[i].actions[j].info = atoi(tokenText);
				lookahead = next_token();
			}
		}
		while(j < ACTIONS_MAX) {
			t_ptr[i].actions[j].info = ERR_INFO_VAL;
			j++;
		}
	}
	if (i == 0)
		return 1;
	while(i < THRESHOLDS_MAX) {
		t_ptr[i].actions[0].info = ERR_INFO_VAL;
		i++;
	}

	return 0;
}

static int parse_weights(void *value)
{
	uint32_t i;
	int *weights = *(int **)value;
	int weight = 0;

	for (i = 0; lookahead == INTEGER && i < sensor_info_arr_len; i++) {
		weights[i] = atoi(tokenText);
		weight += weights[i];
		lookahead = next_token();
	}
	if (i == 0 || weight != 100)
		return 1;
	while(i < sensor_info_arr_len) {
		weights[i] = ERR_WEIGHT_VAL;
		i++;
	}

	return 0;
}

static int parse_ints(void *value)
{
	uint32_t i;
	int *int_list = (int *)value;

	for (i = 0; lookahead == INTEGER && i < INTEGERS_MAX; i++) {
		int_list[i] = atoi(tokenText);
		lookahead = next_token();
	}
	if (i == 0)
		return 1;
	while (i < INTEGERS_MAX) {
		int_list[i] = ERR_INT_VAL;
		i++;
	}

	return 0;
}

void add_setting(struct thermal_setting_t *settings, struct setting_info *info)
{
	if ((settings == NULL) || (info == NULL)) {
		msg("Invalid args.\n");
		return;
	}

	/* Insert into the config list head */
	info->next = settings->list;
	settings->list = info;
}

void init_settings(struct thermal_setting_t *settings)
{
	if (!settings)
		return;

	memset(settings, 0, sizeof(struct thermal_setting_t));
	settings->sample_period_ms = ERR_SAMPLING_VAL;
}

static struct setting_info *find_section_id(struct thermal_setting_t *settings,
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

static void init_section_settings(struct setting_info *settings, char *desc, enum algo_type algo)
{
	if (!settings)
		return;

	memset(settings, 0, sizeof(struct setting_info));
	settings->algo_type = algo;
	strlcpy(settings->desc, desc, MAX_ALGO_DESC);
	settings->dynamic = 1;
	settings->disable = 0;
	settings->err_disable = 0;
}

static char **init_sensor_list(char **prev, uint32_t cnt, uint32_t size)
{
	uint32_t i, j;
	char **ret;
	/* If size is equal to sensor_info_arr_len then prev
	has enough memory for parsing */
	if (size == sensor_info_arr_len)
		return prev;
	if ((cnt > 0 || size > 0) && prev == NULL) {
		msg("%s: Invalid args\n", __func__);
		return NULL;
	}
	/* ret is going to be our new sensor_list so we must allocate
	enough memory for parsing */
	ret = (char **)malloc(sizeof(char *)*sensor_info_arr_len);
	if (ret == NULL)
		return NULL;
	for (i = 0; i < sensor_info_arr_len; i++) {
		ret[i] = (char *)malloc(sizeof(char)*MAX_SENSOR_NAME_LEN);
		if (ret[i] == NULL) {
			for (j = 0; j < i; j++)
				free(ret[j]);
			free(ret);
			return NULL;
		}
		ret[i][0] = ERR_SENSOR_VAL;
		/* cnt is the amount of sensors in prev. If i is less than cnt
		we need to copy prev[i] into ret[i] so no data is lost */
		if (i < cnt)
			strlcpy(ret[i], prev[i], MAX_SENSOR_NAME_LEN);
	}
	/* Since all of prev's data is in ret we no longer need prev.
	Free all of prev's data. If size is equal to 0 then nothing is freed */
	for (i = 0; i < size; i++)
		free(prev[i]);
	if (size != 0)
		free(prev);
	/* ret should be returned to a temp value so we can check for NULL
	before setting it equal to sensor_list */
	return ret;
}

static int *init_weight_list(int *prev, uint32_t cnt, uint32_t size)
{
	uint32_t i;
	int *ret;
	/* If size is equal to sensor_info_arr_len then prev
	has enough memory for parsing */
	if (size == sensor_info_arr_len)
		return prev;
	if (cnt > 0 && prev == NULL) {
		msg("%s: Invalid args\n", __func__);
		return NULL;
	}
	/* ret is going to be our new weight_list so we must allocate
	enough memory for parsing */
	ret = (int *)malloc(sizeof(int)*sensor_info_arr_len);
	if (ret == NULL)
		return NULL;

	for (i = 0; i < sensor_info_arr_len; i++) {
		ret[i] = ERR_WEIGHT_VAL;
		/* cnt is the amount of weights in prev. If i is less than cnt
		we need to copy prev[i] into ret[i] so no data is lost */
		if (i < cnt)
			ret[i] = prev[i];
	}
	/* Since all of prev's data is in ret we no longer need prev.
	Free all of prev's data. If size is equal to 0 then nothing is freed */
	if (size != 0)
		free(prev);
	/* ret should be returned to a temp value so we can check for NULL
	before setting it equal to weight_list */
	return ret;
}

static void init_pid_settings(struct setting_info *section)
{
	if (section->dynamic) {
		section->data.pid.sampling_period_ms = ERR_SAMPLING_VAL;
		section->data.pid.i_samples = ERR_ISAMPLES_VAL;
		section->data.pid.set_point = ERR_SETPOINT_VAL;
		section->data.pid.set_point_clr = ERR_SETPOINTCLR_VAL;
		section->data.pid.units_per_C = ERR_UNITSPERC_VAL;
		section->data.pid.p_const = ERR_PCONST_VAL;
		section->data.pid.i_const = ERR_ICONST_VAL;
		section->data.pid.d_const = ERR_DCONST_VAL;
		section->data.pid.err_weight = ERR_ERRWEIGHT_VAL;
		section->data.pid.freq_scale = ERR_FREQ_SCALE;
	}
}

static void init_tm_settings(struct setting_info *section)
{
	uint32_t i, j;

	if (section->dynamic)
		section->data.tm.sampling_period_ms = ERR_SAMPLING_VAL;
	for (i = section->data.tm.num_thresholds; i < THRESHOLDS_MAX; i++) {
		section->data.tm.t[i].lvl_trig = ERR_TRIG_VAL;
		section->data.tm.t[i].lvl_clr = ERR_CLR_VAL;
		for (j = section->data.tm.t[i].num_actions; j < ACTIONS_MAX; j++) {
			section->data.tm.t[i].actions[j].device[0] = ERR_DEVICE_VAL;
			section->data.tm.t[i].actions[j].info = ERR_INFO_VAL;
		}
	}
}

static void init_ss_settings(struct setting_info *section)
{
	if (section->dynamic) {
		section->data.ss.sampling_period_ms = ERR_SAMPLING_VAL;
		section->data.ss.set_point = ERR_SETPOINT_VAL;
		section->data.ss.set_point_clr = ERR_SETPOINTCLR_VAL;
	}
}

static int init_v_settings(struct setting_info *section)
{
	char **temp;
	int *itemp;

	if (section->dynamic) {
		section->data.v.set_point = ERR_SETPOINT_VAL;
		section->data.v.set_point_clr = ERR_SETPOINTCLR_VAL;
		section->data.v.sampling_period_ms = ERR_SAMPLING_VAL;
	}

	temp = init_sensor_list(section->data.v.sensor_list,
		 section->data.v.list_cnt, section->data.v.list_size);
	if (temp == NULL) {
		msg("Failed to alloc v sensor_list\n");
		return 1;
	}
	section->data.v.sensor_list = temp;
	section->data.v.list_size = (uint8_t)sensor_info_arr_len;

	itemp = init_weight_list(section->data.v.weight_list,
		 section->data.v.weight_cnt, section->data.v.weight_size);
	if (itemp == NULL) {
		msg("Failed to alloc v weight_list\n");
		return 1;
	}
	section->data.v.weight_list = itemp;
	section->data.v.weight_size = sensor_info_arr_len;
	return 0;
}

static int init_eq_settings(struct setting_info *section)
{
	uint32_t i;
	char **temp;

	for (i = 0; i < INTEGERS_MAX; i++) {
		section->data.eq.sampling_list[i] = ERR_SAMPLING_VAL;
		section->data.eq.temp_range_list[i] = ERR_UINT_VAL;
	}
	temp = init_sensor_list(section->data.eq.sensor_list,
		 section->data.eq.list_cnt, section->data.eq.list_size);
	if (temp == NULL) {
		msg("Failed to alloc eq sensor_list\n");
		return 1;
	}
	section->data.eq.sensor_list = temp;
	section->data.eq.list_size =(uint8_t)sensor_info_arr_len;
	return 0;
}

static void finalize_tm_settings(struct setting_info *section)
{
	uint32_t i, j;

	section->data.tm.num_thresholds = 0;
	for (i = 0; i < THRESHOLDS_MAX ; i++) {
		if (section->data.tm.t[i].lvl_trig == ERR_TRIG_VAL
		   || section->data.tm.t[i].lvl_clr == ERR_CLR_VAL)
			break;
		section->data.tm.t[i].num_actions = 0;
		for (j = 0; j < ACTIONS_MAX; j++) {
			if (section->data.tm.t[i].actions[j].device[0]
			    == ERR_DEVICE_VAL
			   || section->data.tm.t[i].actions[j].info
			    == ERR_INFO_VAL)
				break;
			section->data.tm.t[i].num_actions++;
		}
		section->data.tm.num_thresholds++;
	}
}

static void finalize_v_settings(struct setting_info *section)
{
	uint32_t i;

	section->data.v.list_cnt = 0;
	section->data.v.weight_cnt = 0;
	for (i = 0 ; i < sensor_info_arr_len; i++) {
		if (section->data.v.sensor_list[i][0] == ERR_SENSOR_VAL
		   || section->data.v.weight_list[i] == ERR_WEIGHT_VAL)
			break;
		section->data.v.list_cnt++;
		section->data.v.weight_cnt++;
	}
}

static void finalize_eq_settings(struct setting_info *section)
{
	uint32_t i;
	uint32_t *sampling_list = section->data.eq.sampling_list;
	uint32_t *temp_range_list = section->data.eq.temp_range_list;

	if (sampling_list[0] != ERR_SAMPLING_VAL)
		section->data.eq.sampling_period_ms = sampling_list[0];
	if (sampling_list[1] != ERR_SAMPLING_VAL)
		section->data.eq.long_sampling_period_ms = sampling_list[1];
	if (sampling_list[2] != ERR_SAMPLING_VAL)
		section->data.eq.long_sampling_cnt = sampling_list[2];
	if (sampling_list[3] != ERR_SAMPLING_VAL)
		section->data.eq.pre_cal_delay_ms = sampling_list[3];
	if (temp_range_list[0] != ERR_UINT_VAL)
		section->data.eq.temp_range = temp_range_list[0];
	if (temp_range_list[1] != ERR_UINT_VAL)
		section->data.eq.sensor_delta = temp_range_list[1];
	if (temp_range_list[2] != ERR_UINT_VAL)
		section->data.eq.long_sample_range = temp_range_list[2];

	section->data.eq.list_cnt = 0;
	for (i = 0; i < sensor_info_arr_len && section->data.eq.sensor_list[i][0]
	    != ERR_SENSOR_VAL; i++)
		section->data.eq.list_cnt++;
}

static int parse_settings(void *section, char *desc,
				struct table *gen_table, uint32_t table_size)
{
	uint32_t i;

	info("Parsing section %s\n", desc);
	while (lookahead != OBRACKET && lookahead != END) {
		while (lookahead != STRING_FIELD) {
			lookahead = next_token();
			if (lookahead == OBRACKET || lookahead == END)
				return 0;
		}
		info("Found field '%s'\n", tokenText);

		for (i = 0; i < table_size; i++) {
			if (strncasecmp(tokenText, gen_table[i].field_name,
			    tokenTextLength + 1) == 0) {
				lookahead = next_token();
				if (gen_table[i].callback_func((void *)
				   ((char *)section + gen_table[i].offset))) {
					msg("Error with field '%s'\n",
					    gen_table[i].field_name);
					return 1;
				}
				break;
			}
		}
		if (i == table_size) {
			msg("Ignoring unknown field '%s'\n", tokenText);
			lookahead = next_token();
		}
	}
	return 0;
}

static void parse_pid_settings(struct setting_info *section)
{
	info("Algo Type 'pid'\n");
	init_pid_settings(section);
	if (parse_settings((void *)section, section->desc, pid_table,
			   ARRAY_SIZE(pid_table)))
		section->err_disable = 1;
}

static void parse_tm_settings(struct setting_info *section)
{
	info("Algo Type 'monitor'\n");
	init_tm_settings(section);
	if (parse_settings((void *)section, section->desc, tm_table,
			   ARRAY_SIZE(tm_table)))
		section->err_disable = 1;
	finalize_tm_settings(section);
}

static void parse_ss_settings(struct setting_info *section)
{
	info("Algo Type 'ss'\n");
	init_ss_settings(section);
	if (parse_settings((void *)section, section->desc, ss_table,
			   ARRAY_SIZE(ss_table)))
		section->err_disable = 1;
}

static void parse_v_settings(struct setting_info *section)
{
	info("Algo Type 'virtual'\n");
	if (init_v_settings(section)) {
		section->err_disable = 1;
		return;
	}
	if (parse_settings((void *)section, section->desc, v_table,
			   ARRAY_SIZE(v_table)))
		section->err_disable = 1;
	finalize_v_settings(section);
}

static void parse_eq_settings(struct setting_info *section)
{
	info("Algo Type 'eq'\n");
	if (init_eq_settings(section)) {
		section->err_disable = 1;
		return;
	}
	if (parse_settings((void *)section, section->desc, eq_table,
			   ARRAY_SIZE(eq_table)))
		section->err_disable = 1;
	finalize_eq_settings(section);
}

static void parse_global(struct thermal_setting_t *settings)
{
	while (parse_settings((void *)settings, "global", global_table,
			      ARRAY_SIZE(global_table)));
}

static enum algo_type parse_algo_type(void)
{
	int i;

	if (strncasecmp("algo_type", tokenText, tokenTextLength + 1) != 0)
		return UNKNOWN_ALGO_TYPE;

	lookahead = next_token();
	for (i = 0; i < ARRAY_SIZE(algos); i++) {
		if (strncasecmp(tokenText, algos[i].name,
		     tokenTextLength + 1) == 0) {
			lookahead = next_token();
			return algos[i].type;
		}
	}

	msg("Unknown algo type %s\n", tokenText);
	return UNKNOWN_ALGO_TYPE;
}

static int parse_algo_desc(char *destination)
{
	if (lookahead != OBRACKET)
		return 1;
	lookahead = next_token();

	if (lookahead != STRING_DATA)
		return 1;
	strlcpy(destination, tokenText, MAX_ALGO_DESC);
	lookahead = next_token();

	if (lookahead != EBRACKET)
		return 1;
	lookahead = next_token();
	return 0;
}

static void parse_section(struct thermal_setting_t *settings, int flag)
{
	struct setting_info *in_section;
	char section_desc[MAX_ALGO_DESC];

	if (parse_algo_desc(section_desc))
		return;
	in_section = find_section_id(settings, (const char *)section_desc);
	if (in_section == NULL) {
		enum algo_type temp_type = parse_algo_type();
		if (temp_type == UNKNOWN_ALGO_TYPE)
			return;
		if (flag == LOAD_VIRTUAL_FLAG && temp_type != VIRTUAL_SENSOR_TYPE)
			return;
		if (flag != LOAD_VIRTUAL_FLAG && temp_type == VIRTUAL_SENSOR_TYPE)
			return;

		in_section = (struct setting_info *)malloc(
			sizeof(struct setting_info));
		if (in_section == NULL) {
			msg("Failed to alloc struct setting_info\n");
			return;
		}
		init_section_settings(in_section, section_desc, temp_type);
		add_setting(settings, in_section);
		info("Created section '%s'\n", in_section->desc);
	} else {
		if (flag == LOAD_VIRTUAL_FLAG
		   && in_section->algo_type != VIRTUAL_SENSOR_TYPE)
			return;
		if (flag != LOAD_VIRTUAL_FLAG
		   && in_section->algo_type == VIRTUAL_SENSOR_TYPE)
			return;
		info("Found section '%s'\n", in_section->desc);
		in_section->disable = 0;
		in_section->err_disable = 0;
	}

	switch(in_section->algo_type) {
	case MONITOR_ALGO_TYPE:
		parse_tm_settings(in_section);
		break;
	case PID_ALGO_TYPE:
		parse_pid_settings(in_section);
		break;
	case SS_ALGO_TYPE:
		parse_ss_settings(in_section);
		break;
	case EQUILIBRIUM_TYPE:
		parse_eq_settings(in_section);
		break;
	case VIRTUAL_SENSOR_TYPE:
		parse_v_settings(in_section);
		break;
	default:
		in_section->err_disable = 1;
		msg("Unknown algo type for section %s\n", in_section->desc);
		break;
	}
}

static void parse_file(struct thermal_setting_t *settings, int flag)
{
	lookahead = next_token();
	parse_global(settings);
	while (lookahead == OBRACKET) {
		parse_section(settings, flag);
		while (lookahead != OBRACKET && lookahead != END)
			lookahead = next_token();
	}
}

static int threshold_array_compare(const void *x, const void *y)
{
	struct threshold_t *i = (struct threshold_t *)x;
	struct threshold_t *j = (struct threshold_t *)y;

	if (i->lvl_trig < j->lvl_trig)
		return -1;
	else if (i->lvl_trig > j->lvl_trig)
		return 1;
	return 0;
}

static int descending_threshold_array_compare(const void *x, const void *y)
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
				     struct setting_info *section)
{
	/* Fallback to default sampling times if sampling period
	   was not configured */
	if (settings == NULL || section == NULL) {
		msg("unexpected NULL");
		return;
	}

	if (settings->sample_period_ms > 0 &&
	    section->data.tm.sampling_period_ms == ERR_SAMPLING_VAL) {
		section->data.tm.sampling_period_ms  =
			settings->sample_period_ms;
		info("Using configured default sampling period "
		       "%d ms for section[%s] sensor[%s]\n",
		       settings->sample_period_ms,
		       section->desc,
		       section->data.tm.sensor);
	} else if (settings->sample_period_ms == ERR_SAMPLING_VAL &&
		   section->data.tm.sampling_period_ms == ERR_SAMPLING_VAL) {
		section->data.tm.sampling_period_ms  =
			SAMPLING_MS_DEFAULT;
		info("Using default sampling period %d ms "
		       "for section[%s] sensor[%s]\n",
		       SAMPLING_MS_DEFAULT,
		       section->desc,
		       section->data.tm.sensor);
	}
}

static int validate_v(struct setting_info *s)
{
	if (s->data.v.trip_sensor[0] == ERR_SENSOR_VAL
	   || s->data.v.set_point == ERR_SETPOINT_VAL
	   || s->data.v.set_point_clr == ERR_SETPOINTCLR_VAL
	   || s->data.v.sampling_period_ms == ERR_SAMPLING_VAL) {
		return 1;
	}
	if (s->data.v.sampling_period_ms < SAMPLING_MS_MINIMUM)
		s->data.v.sampling_period_ms = SAMPLING_MS_MINIMUM;
	return 0;
}

static int validate_eq(struct setting_info *s)
{
	if (s->data.eq.list_cnt == 0) {
		return 1;
	}
	if (s->data.eq.sampling_period_ms < SAMPLING_MS_MINIMUM)
		s->data.eq.sampling_period_ms = SAMPLING_MS_MINIMUM;

	if (s->data.eq.long_sampling_period_ms < SAMPLING_MS_MINIMUM)
		s->data.eq.long_sampling_period_ms = SAMPLING_MS_MINIMUM;

	if (s->data.eq.long_sampling_cnt == 0)
		s->data.eq.long_sampling_cnt = 10;

	return 0;
}

static int validate_ss(struct setting_info *s)
{
	if (s->data.ss.sensor[0] == ERR_SENSOR_VAL
	   || s->data.ss.device[0] == ERR_DEVICE_VAL
	   || s->data.ss.sampling_period_ms == ERR_SAMPLING_VAL
	   || s->data.ss.set_point == ERR_SETPOINT_VAL
	   || s->data.ss.set_point_clr == ERR_SETPOINTCLR_VAL) {
		return 1;
	}
	if (s->data.ss.sampling_period_ms < SAMPLING_MS_MINIMUM)
		s->data.ss.sampling_period_ms = SAMPLING_MS_MINIMUM;
	return 0;
}

static int validate_pid(struct setting_info *s)
{
	if (s->data.pid.sensor[0] == ERR_SENSOR_VAL
	   || s->data.pid.device[0] == ERR_DEVICE_VAL
	   || s->data.pid.sampling_period_ms == ERR_SAMPLING_VAL
	   || s->data.pid.i_samples == ERR_ISAMPLES_VAL
	   || s->data.pid.set_point == ERR_SETPOINT_VAL
	   || s->data.pid.set_point_clr == ERR_SETPOINTCLR_VAL
	   || s->data.pid.units_per_C == ERR_UNITSPERC_VAL
	   || s->data.pid.p_const == ERR_PCONST_VAL
	   || s->data.pid.i_const == ERR_ICONST_VAL
	   || s->data.pid.d_const == ERR_DCONST_VAL
	   || s->data.pid.err_weight == ERR_ERRWEIGHT_VAL) {
		return 1;
	}
	if (s->data.pid.sampling_period_ms < SAMPLING_MS_MINIMUM)
		s->data.pid.sampling_period_ms = SAMPLING_MS_MINIMUM;
	return 0;
}

static int validate_tm(struct setting_info *s)
{
	uint32_t i;
	uint32_t j;
	uint32_t k;

	/* Disable no threshold entries */
	if (s->data.tm.num_thresholds == 0)
		return 1;
	if (s->data.tm.sensor[0] == ERR_SENSOR_VAL)
		return 1;
	for (i = 0;i<s->data.tm.num_thresholds;i++) {
		if (s->data.tm.t[i].num_actions == 0)
			return 1;
	}
	if (s->data.tm.sampling_period_ms < SAMPLING_MS_MINIMUM
	    && s->data.tm.sampling_period_ms != ERR_SAMPLING_VAL)
		s->data.pid.sampling_period_ms = SAMPLING_MS_MINIMUM;

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
			if ((k < s->data.tm.t[j].num_actions-1)
			    && (s->data.tm.t[j].actions[k].device != NULL)
			    && (strncasecmp(s->data.tm.t[j].actions[k].device,
			    "shutdown", DEVICES_MAX_NAME_LEN) == 0)) {
				struct action_t temp_action;
				/* Swap entries */
				memcpy(&temp_action,
					&(s->data.tm.t[j].actions[k]),
					sizeof(struct action_t));
				memcpy(&(s->data.tm.t[j].actions[k]),
					&(s->data.tm.t[j].actions[k + 1]),
					sizeof(struct action_t));
				memcpy(&(s->data.tm.t[j].actions[k + 1]),
					&temp_action,
					sizeof(struct action_t));
			}
		}
	}
	return 0;
}

static void validate_config(struct thermal_setting_t *settings)
{
	struct setting_info *s;
	int error;

	s = settings->list;
	while (s != NULL) {
		error = 0;
		switch(s->algo_type) {
		case MONITOR_ALGO_TYPE:
			error = validate_tm(s);
			if (error == 0)
				fallback_sampling_values(settings, s);
			break;
		case PID_ALGO_TYPE:
			error = validate_pid(s);
			break;
		case SS_ALGO_TYPE:
			error = validate_ss(s);
			break;
		case EQUILIBRIUM_TYPE:
			error = validate_eq(s);
			break;
		case VIRTUAL_SENSOR_TYPE:
			error = validate_v(s);
			break;
		default:
			break;
		}
		if (error) {
			msg("Invalid input for section %s\n", s->desc);
			s->err_disable = 1;
		}
		s = s->next;
	}
}

static int load_sensors(void)
{
	if (sensor_info_arr) {
		free(sensor_info_arr);
		sensor_info_arr = NULL;
	}
	if (sensors_manager_get_list(NULL, &sensor_info_arr_len)) {
		msg("Failed to get sensor list length\n");
		return 1;
	}

	sensor_info_arr = (struct sensor_info_type *)
		malloc(sizeof(struct sensor_info_type)*sensor_info_arr_len);
	if (sensor_info_arr == NULL) {
		msg("Failed to alloc ts_info_arr\n");
		return 1;
	}

	if (sensors_manager_get_list(sensor_info_arr,
				     &sensor_info_arr_len)) {
		msg("Failed to get sensor list\n");
		return 1;
	}
	return 0;
}

static int load_devices(void)
{
	if (device_info_arr) {
		free(device_info_arr);
		device_info_arr = NULL;
	}
	if (devices_manager_get_list(NULL, &device_info_arr_len)) {
		msg("Failed to get tmd list length\n");
		return 1;
	}

	device_info_arr = (struct device_info *)
		malloc(sizeof(struct device_info)*device_info_arr_len);
	if (device_info_arr == NULL) {
		msg("Failed to alloc tmd_info_arr\n");
		return 1;
	}

	if (devices_manager_get_list(device_info_arr,
			     &device_info_arr_len)) {
		msg("Failed to get tmd list\n");
		return 1;
	}
	return 0;
}

static void get_unique_fields(struct table *gen_table, int table_size,
			      int *unique_fields)
{
	int i;
	int j;
	for (i = 0; i < table_size; i++) {
		for (j = *unique_fields - 1; j >= 0; j--) {
			if(strncmp(fields[j], gen_table[i].field_name,
				   MAX_TOKEN_TEXT) == 0)
				break;
		}
		if (j < 0) {
			fields[*unique_fields] = gen_table[i].field_name;
			*unique_fields = *unique_fields + 1;
		}
	}
}

static int load_fields(void)
{
	int unique_fields = 0;
	uint32_t max_fields = ARRAY_SIZE(global_table) + ARRAY_SIZE(pid_table) +
			 ARRAY_SIZE(eq_table) + ARRAY_SIZE(tm_table) +
			 ARRAY_SIZE(ss_table) + ARRAY_SIZE(v_table);

	fields = (char **)malloc(sizeof(char*)*(max_fields + 1));
	if (fields == NULL) {
		msg("Failed to alloc fields\n");
		return 1;
	}
	memset(fields, 0, sizeof(char*)*(max_fields + 1));
	get_unique_fields(pid_table, ARRAY_SIZE(pid_table), &unique_fields);
	get_unique_fields(tm_table, ARRAY_SIZE(tm_table), &unique_fields);
	get_unique_fields(eq_table, ARRAY_SIZE(eq_table), &unique_fields);
	get_unique_fields(ss_table, ARRAY_SIZE(ss_table), &unique_fields);
	get_unique_fields(v_table, ARRAY_SIZE(v_table), &unique_fields);
	get_unique_fields(global_table, ARRAY_SIZE(global_table), &unique_fields);
	return 0;
}

int load_config(struct thermal_setting_t *settings, const char *pFName, int flag)
{
	int ret_val = 0;
	const char *cf = (pFName) ? pFName : CONFIG_FILE_DEFAULT;

	info("Loading configuration file %s\n", cf);
	configFile = fopen(cf,"r");
	if (configFile == NULL) {
		msg("Unable to open config file '%s'\n",cf);
		ret_val = -(EIO);
		goto error_handler;
	}

	if (load_sensors() || load_devices() || load_fields()) {
		ret_val = -(EFAULT);
		goto error_handler;
	}

	parse_file(settings, flag);

	validate_config(settings);

error_handler:
	if (sensor_info_arr)
		free(sensor_info_arr);
	if (device_info_arr)
		free(device_info_arr);
	if (configFile)
		fclose(configFile);
	if (fields)
		free(fields);

	fields = NULL;
	sensor_info_arr = NULL;
	device_info_arr = NULL;
	configFile = NULL;

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

	if (settings->sample_period_ms != ERR_SAMPLING_VAL)
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
	PRINT_SETTING(flag, "freq_scale %f\n", pid->freq_scale);
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
	if (ss->device_mtgn_max_limit)
		PRINT_SETTING(flag, "device_max_limit %d", ss->device_mtgn_max_limit);
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
