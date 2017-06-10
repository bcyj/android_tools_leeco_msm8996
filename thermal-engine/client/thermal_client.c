/*========================================================================================

 thermal_client.c

 GENERAL DESCRIPTION
 Thermal client library for external userspace apps interested in thermal mitigation.

 Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

==========================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/un.h>

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif

/* Below macro is duplicated in
   server/thermal_lib_common.h */
#define SUPPORTED_FIELDS_ENABLED 1
#include "thermal_config.h"
#include "thermal_lib_common.h"
#include "thermal_client.h"

#ifdef ANDROID
#  include "cutils/properties.h"
#  ifdef USE_ANDROID_LOG
#    define LOG_TAG         "Thermal-Lib"
#    include "cutils/log.h"
#  endif
#else
#  include <syslog.h>
#endif

#ifdef USE_ANDROID_LOG
#include "common_log.h" /* define after cutils/log.h */
#define msg(format, ...)   LOGE(format, ## __VA_ARGS__)
#define info(format, ...)   LOGI(format, ## __VA_ARGS__)
#else
#define msg(format, ...)   syslog(LOG_ERR, format, ## __VA_ARGS__)
#define info(format, ...)   syslog(LOG_INFO, format, ## __VA_ARGS__)
#endif

/* Utility macros */
#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))

static pthread_t thermal_client_recv_thread;
static int thermal_client_shutdown = 0;
static int first_client = 1;
static struct thermal_msg_data client_msg;
struct thermal_cdata *list_head = NULL;
static pthread_mutex_t client_mutex;
static int pipe_fds[2] = {-1, -1};

/* Supported Thermal clients */
struct req_client {
	char *name;
	char *send_sock_path;
};

static struct req_client req_clients[] = {
	{
		.name = "spkr",
		.send_sock_path = THERMAL_RECV_PASSIVE_CLIENT_SOCKET,
	},
	{
		.name = "override",
		.send_sock_path = THERMAL_RECV_CLIENT_SOCKET,
	},
	{
		.name = CONFIG_SET_CLIENT,
		.send_sock_path = THERMAL_RECV_CLIENT_SOCKET,
	},
	{
		.name = CONFIG_QUERY_CLIENT,
		.send_sock_path = THERMAL_RECV_CLIENT_SOCKET,
	},
};


struct notify_client {
	char *name;
	int  min_req_data;
	int  max_req_data;
};

static struct notify_client notify_clients[] = {
	{
		.name = "camera",
		.min_req_data = 0,
		.max_req_data = MAX_CAMERA_MITIGATION_LEVEL,
	},
	{
		.name = "camcorder",
		.min_req_data = 0,
		.max_req_data = MAX_CAMCORDER_MITIGATION_LEVEL,
	},
	{
		.name = "spkr",
		.min_req_data = -30,
		.max_req_data = 150,
	},
	{
		.name = CONFIG_QUERY_CLIENT,
		.min_req_data = 0,
		.max_req_data = LEVEL_MAX,
	}
};

struct algorithms {
	char *name;
	enum algo_type type;
};

static struct algorithms algos[] = {
	{.name = "monitor", .type = MONITOR_ALGO_TYPE},
	{.name = "pid", .type = PID_ALGO_TYPE},
	{.name = "ss", .type = SS_ALGO_TYPE},
	{.name = "all", .type = ALGO_IDX_MAX},
};

static int add_int_to_field(void *value, void *data);
static int add_string_to_field(void *value, void *data);
static int add_thresholds_to_field(void *value, void *data);
static int add_thresholds_clr_to_field(void *value, void *data);
static int add_actions_to_field(void *value, void *data);
static int add_action_info_to_field(void *value, void *data);

struct field_table {
	char *field_name;
	enum field_data_type type;
	unsigned int field_mask;
	int offset;
	int sub_offset;
	int (*callback)(void *, void *);
};

static struct field_table monitor_fields[] = {
	{
	 .field_name = "disable",
	 .type = FIELD_INT,
	 .field_mask = DISABLE_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.disable),
	 .sub_offset = 0, .callback = &add_int_to_field
	},
	{
	 .field_name = "sampling",
	 .type = FIELD_INT,
	 .field_mask = SAMPLING_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.sampling),
	 .sub_offset = 0, .callback = &add_int_to_field
	},
	{
	 .field_name = "thresholds",
	 .type = FIELD_INT_ARR,
	 .field_mask = THRESHOLDS_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.m_setting),
	 .sub_offset = offsetof(struct override_threshold_t, threshold_trig),
	 .callback = &add_thresholds_to_field
	},
	{
	 .field_name = "thresholds_clr",
	 .type = FIELD_INT_ARR,
	 .field_mask = THRESHOLDS_CLR_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.m_setting),
	 .sub_offset = offsetof(struct override_threshold_t, threshold_clr),
	 .callback = &add_thresholds_clr_to_field
	},
	{
	 .field_name = "actions",
	 .type = FIELD_ARR_STR,
	 .field_mask = UNKNOWN_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.m_setting),
	 .sub_offset = offsetof(struct override_threshold_t, actions),
	 .callback = &add_actions_to_field
	},
	{
	 .field_name = "action_info",
	 .type = FIELD_ARR_INT_ARR,
	 .field_mask = ACTION_INFO_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.m_setting),
	 .sub_offset = offsetof(struct override_threshold_t, actions),
	 .callback = &add_action_info_to_field
	}
};

/* field_table for both SS and PID */
static struct field_table pid_ss_fields[] = {
	{
	 .field_name = "disable",
	 .type = FIELD_INT,
	 .field_mask = DISABLE_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.disable),
	 .sub_offset = 0,
	 .callback = &add_int_to_field
	},
	{
	 .field_name = "sampling",
	 .type = FIELD_INT,
	 .field_mask = SAMPLING_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.sampling),
	 .sub_offset = 0,
	 .callback = &add_int_to_field
	},
	{
	 .field_name = "set_point",
	 .type = FIELD_INT,
	 .field_mask = SET_POINT_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.d_setting.set_point),
	 .sub_offset = 0,
	 .callback = &add_int_to_field
	},
	{
	 .field_name = "set_point_clr",
	 .type = FIELD_INT,
	 .field_mask = SET_POINT_CLR_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.d_setting.set_point_clr),
	 .sub_offset = 0,
	 .callback = &add_int_to_field
	},
	{
	 .field_name = "device",
	 .type = FIELD_STR,
	 .field_mask = UNKNOWN_FIELD,
	 .offset = offsetof(struct thermal_msg_data, config.d_setting.device),
	 .sub_offset = 0,
	 .callback = &add_string_to_field
	}
};

/*==============================================================================
FUNCTION check_for_algo_type

Internal function to get either algo name or algo type enum.
if first argument is negative, check for matching algo name and return
respective enum type otherwise copy name to algo_name

ARGUMENTS
	algo      - algo_type enum
	algo_name - algo_name

RETURN VALUE
	return algo type enum on success UNKNOWN_ALGO_TYPE on error
==============================================================================*/
static enum algo_type check_for_algo_type(enum algo_type algo, char *algo_name)
{
	uint8_t i;

	if (algo < 0 && algo_name[0] == '\0')
		return UNKNOWN_ALGO_TYPE;

	for (i = 0; i < ARRAY_SIZE(algos); i++) {
		if (algo > UNKNOWN_ALGO_TYPE) {
			if (algos[i].type == algo) {
				strlcpy(algo_name, algos[i].name, MAX_ALGO_NAME);
				break;
			}
		} else {
			if (0 == strcmp(algos[i].name, algo_name))
				break;
		}
	}

	if (i >= ARRAY_SIZE(algos))
		return UNKNOWN_ALGO_TYPE;

	return algos[i].type;
}

/*==============================================================================
FUNCTION do_request_to_thermal

Thermal client request sending thread.
This function will run in a separate thread. This thread is intented to
unblock parent client if connect() fails.

ARGUMENTS
	data - data pointer which points to message to be sent

RETURN VALUE
	void * - not used.
===============================================================================*/
static void *do_request_to_thermal(void *data)
{
	ssize_t rc = 0;
	struct sockaddr_un client_addr_send;
	char *send_sock_path = (char *)data;
	int sockfd_client_send = -1;

	while(1) {
		sockfd_client_send = socket(AF_LOCAL, SOCK_STREAM, 0);
		if (sockfd_client_send < 0) {
			msg("Thermal-Lib-Client: %s: failed setup "
			    "client send sockfd", __func__);
			sleep(5);
			continue;
		}

		memset(&client_addr_send, 0, sizeof(struct sockaddr_un));
		strlcpy(client_addr_send.sun_path, send_sock_path, UNIX_PATH_MAX);
		client_addr_send.sun_family = AF_LOCAL;
		rc = connect(sockfd_client_send, (struct sockaddr *)&client_addr_send,
				     (socklen_t)(sizeof(sa_family_t) + strlen(send_sock_path)));
		if (rc != 0) {
			close(sockfd_client_send);
			sleep(5);
			continue;
		}

		rc = send(sockfd_client_send, &client_msg, sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
		if (rc <= 0) {
			msg("Thermal-Lib-Client: "
			    "Unable to send request data to fd %d", sockfd_client_send);
			break;
		}
		info("Thermal-Lib-Client: Client request sent");
		break;
	}
	close(sockfd_client_send);
	return NULL;
}

/*================================================================================================
FUNCTION thermal_client_request

Thermal client request to thermal function.
The client which will send/notify request to thermal.

ARGUMENTS
	client_name - client name
	req_data    - requested data to be sent

RETURN VALUE
	0 on success, negative on failure.
=================================================================================================*/
int thermal_client_request(char *client_name, int req_data)
{
	int rc = 0;
	int ret = -EINVAL;
	int i;
	pthread_t thermal_client_request_thread;

	if (NULL == client_name) {
		msg("Thermal-Lib-Client:%s: unexpected NULL", __func__);
		return ret;
	}
	/* Check for client is supported  or not*/
	for (i = 0; i < ARRAY_SIZE(req_clients); i++) {
		if (0 == strncmp(req_clients[i].name, client_name, CLIENT_NAME_MAX))
			break;
	}

	if (i >= ARRAY_SIZE(req_clients)) {
		msg("Thermal-Lib-Client:%s is not in supported "
		    "thermal client list", client_name);
		return ret;
	}

	memset(&client_msg, 0, sizeof(struct thermal_msg_data));
	strlcpy(client_msg.client_name, client_name, CLIENT_NAME_MAX);
	client_msg.req_data = req_data;

	rc = pthread_create(&thermal_client_request_thread, NULL, do_request_to_thermal,
			    (void*) req_clients[i].send_sock_path);
	if (rc != 0) {
		msg("Thermal-Lib-Client: Unable to create pthread to "
		    "send client request from %s", client_name);
		return ret;
	}

	ret = 0;
	return ret;
}

/*==============================================================================
FUNCTION add_int_to_field

Internal helper function for adding integer data to field.

ARGUMENTS
	value - value
	data  - field in which value will be stored

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_int_to_field(void *value, void *data)
{
	int *int_data = NULL;
	struct field_data *field = (struct field_data *)data;

	if (value == NULL || data == NULL)
		return -1;

	int_data = (int *)malloc(sizeof(int));
	if (int_data == NULL) {
		msg("%s: malloc failed", __func__);
		return -1;
	}

	*int_data = *(int *)value;
	field->data = int_data;
	field->num_data = 1;

	return 0;
}

/*==============================================================================
FUNCTION add_string_to_field

Internal helper function for adding string data to field.

ARGUMENTS
	value - value
	data  - field in which value will be stored

RETURN VALUE
	0 on success, negative error(-1) on failure.
===============================================================================*/
static int add_string_to_field(void *value, void *data)
{
	char *string = (char *)value;
	char *char_data = NULL;
	struct field_data *field = (struct field_data *)data;

	if (value == NULL || data == NULL)
		return -1;

	char_data = (char *)malloc(sizeof(char) * DEVICES_MAX_NAME_LEN);
	if (char_data == NULL) {
		msg("%s: malloc failed", __func__);
		return -1;
	}
	strlcpy(char_data, string, DEVICES_MAX_NAME_LEN);
	field->data = char_data;
	field->num_data = 1;

	return 0;
}

/*==============================================================================
FUNCTION add_thresholds_to_field

Helper function for adding integer array data to field.

ARGUMENTS
	value - value
	data  - field in which value will be stored

RETURN VALUE
	0 on success, negative error(-1) on failure.
===============================================================================*/
static int add_thresholds_to_field(void *value, void *data)
{
	uint32_t i = 0;
	int *int_data = NULL;
	struct field_data *field = (struct field_data *)data;

	if (value == NULL || data == NULL)
		return -1;

	struct monitor_settings *ptr_m = (struct monitor_settings *)value;
	field->num_data = ptr_m->num_thresholds;

	int_data = (int *)malloc(sizeof(int) * field->num_data);
	if (int_data == NULL) {
		msg("%s: malloc failed", __func__);
		return -1;
	}

	for (i = 0; i < field->num_data; i++) {
		int_data[i] = ptr_m->t[i].threshold_trig;
	}
	field->data = int_data;

	return 0;
}

/*==============================================================================
FUNCTION add_thresholds_clr_to_field

Internal helper function for adding integer array data to field.

ARGUMENTS
	value - value
	data  - field in which value will be stored

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_thresholds_clr_to_field(void *value, void *data)
{
	uint32_t i = 0;
	int *int_data = NULL;
	struct field_data *field = (struct field_data *)data;

	if (value == NULL || data == NULL)
		return -1;

	struct monitor_settings *ptr_m = (struct monitor_settings *)value;
	field->num_data = ptr_m->num_thresholds;

	int_data = (int *)malloc(sizeof(int) * field->num_data);
	if (int_data == NULL) {
		msg("%s: malloc failed", __func__);
		return -1;
	}

	for (i = 0; i < field->num_data; i++) {
		int_data[i] = ptr_m->t[i].threshold_clr;
	}

	field->data = int_data;

	return 0;
}

/*==============================================================================
FUNCTION add_actions_to_field

Internal helper function for adding array of string data to field.

ARGUMENTS
	value - value
	data  - field in which value will be stored

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_actions_to_field(void *value, void *data)
{
	uint32_t i = 0;
	uint32_t j = 0;
	unsigned int string_len = 0;
	char **char_data = NULL;
	struct field_data *field = (struct field_data *)data;

	if (value == NULL || data == NULL)
		return -1;

	struct monitor_settings *ptr_m = (struct monitor_settings *)value;
	field->num_data = ptr_m->num_thresholds;

	char_data = (char **)malloc(sizeof(char *) * field->num_data);
	if (char_data == NULL) {
		msg("%s: malloc failed", __func__);
		return -1;
	}

	for (i = 0; i < field->num_data; i++) {
		for (j = 0; j < ptr_m->t[i].num_actions; j++) {
			string_len += DEVICES_MAX_NAME_LEN+1;
		}
		char_data[i] = (char *)malloc(sizeof(char) * (string_len));
		if (char_data[i] == NULL) {
			msg("%s: malloc failed", __func__);
			for(j = 0; j < i; j++)
				free(char_data[j]);
			free(char_data);
			return -1;
		}
		memset(char_data[i], 0, sizeof(char_data[i]));
		for (j = 0; j < ptr_m->t[i].num_actions; j++) {
			if (j > 0) {
				if (strlcat(char_data[i], "+", string_len) >=
						string_len)
					break;
			}
			if (strlcat(char_data[i], ptr_m->t[i].actions[j].device,
				    string_len) >= string_len)
				break;
		}
	}
	field->data = char_data;

	return 0;
}

/*==============================================================================
FUNCTION add_action_info_to_field

Internal function for adding action info data to field.

ARGUMENTS
	value - value
	data  - field in which value will be stored

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_action_info_to_field(void *value, void *data)
{
	uint32_t i = 0;
	uint32_t j = 0;
	struct action_info_data *action_info = NULL;
	struct field_data *field = (struct field_data *)data;

	if (value == NULL || data == NULL)
		return -1;

	struct monitor_settings *ptr_m = (struct monitor_settings *)value;
	field->num_data = ptr_m->num_thresholds;

	action_info =
	(struct action_info_data *)malloc(sizeof(struct action_info_data) * \
					  field->num_data);
	if (action_info == NULL) {
		msg("%s: malloc failed", __func__);
		return -1;
	}

	for (i = 0; i < field->num_data; i++) {
		action_info[i].num_actions = ptr_m->t[i].num_actions;
		for (j = 0; j <  ptr_m->t[i].num_actions; j++)
			action_info[i].info[j] = ptr_m->t[i].actions[j].info;
	}

	field->data = action_info;

	return 0;
}

/*==============================================================================
FUNCTION add_settings_to_client_config

Internal function for adding fields to client config.

ARGUMENTS
	config - client config
	thermal_data -  thermal shared config
	fields - supported field table
	num_fields - number of fields supported

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_settings_to_client_config(struct config_instance *config,
					 struct thermal_msg_data *thermal_data,
					 struct field_table *field_table,
					 uint32_t num_fields)
{
	uint32_t i = 0;
	struct field_data *fields = NULL;

	if (config == NULL || thermal_data == NULL || field_table == NULL)
		return -1;

	config->num_fields = num_fields;

	fields =
	(struct field_data *)malloc(sizeof(struct field_data) * num_fields);
	if (fields == NULL) {
		msg("%s: malloc failed for fields", __func__);
		return -1;
	}

	for (i = 0; i < num_fields; i++) {
		uint32_t j = 0;
		fields[i].field_name =
			 (char *)malloc(sizeof(char) * MAX_FIELD_NAME);
		if (fields[i].field_name == NULL) {
			msg("%s: malloc failed for field_name", __func__);
			for (j = 0; j < i; j++) {
				free(fields[j].field_name);
				free(fields[j].data);
			}
			free(fields);
			return -1;
		}
		strlcpy(fields[i].field_name, field_table[i].field_name,
		        MAX_FIELD_NAME);

		fields[i].data_type = field_table[i].type;
		if (field_table[i].callback((void *)((char *)thermal_data +
						     (field_table[i].offset)),
						    (void *)&fields[i])) {
			free(fields[i].field_name);
			for (j = 0; j < i; j++) {
				free(fields[j].field_name);
				free(fields[j].data);
			}
			free(fields);
			return -1;
		}
	}
	config->fields = fields;

	return 0;
}

/*==============================================================================
FUNCTION connect_to_socket

Internal function to get sockfd and to connect with thermal server.

ARGUMENTS
	client_name    -  client_name
	client_addr    -  socketadd_un

RETURN VALUE
        return sockfd on success, negative error(-1) on failure.
==============================================================================*/
static int connect_to_socket(char *client_name, struct sockaddr_un *client_addr)
{
	uint8_t i;
	int rc;
	int sockfd_client = -1;

	if (client_name == NULL)
		return -1;

	/* Get socket file to connect using client_name */
	for (i = 0; i < ARRAY_SIZE(req_clients); i++) {
		if (0 == strcmp(req_clients[i].name, client_name))
			break;
	}

	if (i >= ARRAY_SIZE(req_clients)) {
		msg("Thermal-Lib-Client:%s is not in supported "
		    "thermal client list", client_name);
		return -1;
	}

	sockfd_client = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd_client < 0)
		return -1;

	memset(client_addr, 0, sizeof(struct sockaddr_un));
	strlcpy(client_addr->sun_path, req_clients[i].send_sock_path,
	        UNIX_PATH_MAX);
	client_addr->sun_family = AF_LOCAL;
	rc = connect(sockfd_client, (struct sockaddr *)client_addr,
		     (socklen_t)(sizeof(sa_family_t) + strlen(req_clients[i].send_sock_path)));
	if (rc != 0) {
		close(sockfd_client);
		return -1;
	}

	return sockfd_client;
}

/*==============================================================================
FUNCTION recv_and_extract_packet

Internal function to recv thermal message over socket, validate and extract
received data.

ARGUMENTS
	sockfd    -  client sockfd
	configs   -  client configs pointer
	thermal_msg - socket msg data
	num_configs  - To save number of valid configs

RETURN VALUE
        return greater than 0 for valid config, 0 to check for next config and
	negative error(-1) for end of packet message or unexpected error.
==============================================================================*/
static int recv_and_extract_packet(int sockfd, struct config_instance **configs,
				   struct thermal_msg_data *thermal_msg,
				   uint32_t *num_configs)
{
	ssize_t rc;
	uint32_t i;

	if (thermal_msg == NULL || sockfd < 0)
		return -1;

	memset(thermal_msg, 0, sizeof(struct thermal_msg_data));
	rc = recv(sockfd, (struct thermal_msg_data *)thermal_msg,
		  sizeof(struct thermal_msg_data), 0);
	if (rc <= 0) {
		msg("Thermal-Lib-Client:%s: recv failed, received byte %zd\n",
		     __func__, rc);
		return -1;
	}

	/* Validate size of message, client name, EOM condtion and
	   first packet condition using msg_type and req_data and config
	   description for all valid config packet */
	if (rc != sizeof(struct thermal_msg_data)) {
		msg("Thermal-Lib-Client:%s: size of received message not matching\n",
		     __func__);
		return -1;
	}

	if (strnlen(thermal_msg->client_name, CLIENT_NAME_MAX) >= CLIENT_NAME_MAX) {
		msg("Thermal-Lib-Client:%s: invalid client name %s\n",
		     __func__, thermal_msg->client_name);
		return -1;
	}

	info("Thermal-Lib-Client:%s: Client received msg %s %d\n",
		__func__, thermal_msg->client_name, thermal_msg->msg_type);

	/* Check for client is supported or not*/
	for (i = 0; i < ARRAY_SIZE(notify_clients); i++) {
		if (0 == strcmp(notify_clients[i].name, thermal_msg->client_name))
			break;
	}

	if (i >= ARRAY_SIZE(notify_clients)) {
		msg("Thermal-Lib-Client:%s is not in supported "
		    "thermal client list", thermal_msg->client_name);
		return -1;
	}

	/* End of config recv */
	if (thermal_msg->msg_type == ALGO_IDX_MAX && thermal_msg->req_data == 0)
		return -1;

	/* First packet: Number of valid configs going to recv */
	if (thermal_msg->msg_type == ALGO_IDX_MAX && thermal_msg->req_data > 0) {
		if ((unsigned int)thermal_msg->req_data < MAX_CONFIG_INSTANCES_SUPPORTED) {
			*configs =
			(struct config_instance *)malloc(sizeof(struct config_instance) * \
						         (size_t)thermal_msg->req_data);
			if (*configs == NULL) {
				msg("%s: malloc failed for fields", __func__);
				return -1;
			}
			*num_configs = (uint32_t)thermal_msg->req_data;
			return 0;
		}
		msg("Thermal-Lib-Client:%s Invalid number configs %d\n",
		     __func__, thermal_msg->req_data);
		return -1;
	}

	if (strnlen(thermal_msg->config.config_desc, MAX_ALGO_DESC) >= MAX_ALGO_DESC) {
		msg("Thermal-Lib-Client:%s: invalid config description %s\n",
		     __func__, thermal_msg->config.config_desc);
		return -1;
	}

	return 1;
}

/*=============================================================================
FUNCTION thermal_client_config_query

Thermal client config query request to thermal function.

ARGUMENTS
	algo_type    -  algo type of instances which client is interested.
	configs      -  config instances which return back to client.

RETURN VALUE
        return number of conifgs on success, 0 on failure.
==============================================================================*/
int thermal_client_config_query(char *algo_type,
				struct config_instance **configs)
{
	uint32_t idx = 0;
	ssize_t rc = 0;
	struct thermal_msg_data query_msg;  /* sending data */
	struct thermal_msg_data thermal_msg; /* recieving data */
	struct sockaddr_un client_addr;
	struct config_instance *config_local = NULL;
	int sockfd_client = -1;
	char algo_name[MAX_ALGO_NAME];
	enum algo_type algo = UNKNOWN_ALGO_TYPE;
	uint32_t num_configs = 0;

	if (NULL == algo_type) {
		msg("Thermal-Lib-Client:%s: unexpected NULL", __func__);
		return 0;
	}

	/* Get algo type enum if it is supported */
	algo = check_for_algo_type(UNKNOWN_ALGO_TYPE, algo_type);
	if (algo == UNKNOWN_ALGO_TYPE)
		return 0;

	memset(&query_msg, 0, sizeof(struct thermal_msg_data));
	query_msg.msg_type = algo;
	query_msg.req_data = 0;
	strlcpy(query_msg.client_name, CONFIG_QUERY_CLIENT, CLIENT_NAME_MAX);

	sockfd_client = connect_to_socket(query_msg.client_name, &client_addr);
	if (sockfd_client < 0) {
		msg("Thermal-Lib-Client:%s: could not connect to socket\n",
		    __func__);
		return 0;
	}

	/* Send client info to thermal server */
	rc = send(sockfd_client, &query_msg, sizeof(struct thermal_msg_data),
		  MSG_NOSIGNAL);
	if (rc <= 0) {
		msg("Thermal-Lib-Client: "
		    "Unable to send request data to fd %d", sockfd_client);
		close(sockfd_client);
		return 0;
	}
	info("Thermal-Lib-Client: Client config query request sent %s  %d\n",
	      query_msg.client_name, query_msg.msg_type);

	/* Recv configs one by one from thermal */
	while (1) {
		rc = recv_and_extract_packet(sockfd_client, &config_local,
					     &thermal_msg, &num_configs);
		if (rc < 0) {
			if (idx == num_configs)
				break;
			else
				goto error_handler;
		} else if (rc == 0) {
			continue;
		}

		/* check idx is within limit */
		if (idx >= num_configs)
			goto error_handler;

		/* validate config data with client request */
		if ((query_msg.msg_type != ALGO_IDX_MAX) &&
		    (thermal_msg.msg_type != query_msg.msg_type))
			goto error_handler;

		/* Get algo_name if it is supported */
		algo = check_for_algo_type(thermal_msg.msg_type, algo_name);
		if (algo == UNKNOWN_ALGO_TYPE)
			goto error_handler;

		if (config_local == NULL)
			goto error_handler;

		/* allocate memory and save config fields */
		config_local[idx].cfg_desc =
		(char *)malloc(sizeof(char) * MAX_ALGO_DESC);
		if (config_local[idx].cfg_desc == NULL) {
			msg("%s: malloc failed for config_desc", __func__);
			goto error_handler;
		}
		strlcpy(config_local[idx].cfg_desc, thermal_msg.config.config_desc,
			MAX_ALGO_DESC);

		config_local[idx].algo_type =
		(char *)malloc(sizeof(char) * MAX_ALGO_NAME);
		if (config_local[idx].algo_type == NULL) {
			msg("%s: malloc failed for algo_type", __func__);
			goto error_handler;
		}
		strlcpy(config_local[idx].algo_type, algo_name,
		        MAX_ALGO_NAME);

		/* fields_mask is valid during only set request */
		config_local[idx].fields_mask = UNKNOWN_FIELD;

		switch(thermal_msg.msg_type) {
			case MONITOR_ALGO_TYPE:
				if (add_settings_to_client_config(&config_local[idx],
						&thermal_msg, monitor_fields,
						ARRAY_SIZE(monitor_fields))) {
					msg("Thermal-Lib-Client:%s: config data is invalid\n",
					     __func__);
					goto error_handler;
				}
				idx++;
				break;
			case SS_ALGO_TYPE:
			case PID_ALGO_TYPE:
				if (add_settings_to_client_config(&config_local[idx],
						&thermal_msg, pid_ss_fields,
						ARRAY_SIZE(pid_ss_fields))) {
					msg("Thermal-Lib-Client:%s: config data is invalid\n",
					     __func__);
					goto error_handler;
				}
				idx++;
				break;
			default:
				msg("Thermal-Lib-Client:Unknown algo type\n");
				goto error_handler;
		}
	}

	*configs = config_local;
	close(sockfd_client);
	return (int)idx;

error_handler:
	if (config_local != NULL) {
		if (&config_local[idx]) {
			if (config_local[idx].cfg_desc)
				free(config_local[idx].cfg_desc);
			if (config_local[idx].algo_type)
				free(config_local[idx].algo_type);
		}
		thermal_client_config_cleanup(config_local, idx);
		config_local = NULL;
	}
	*configs = NULL;
	close(sockfd_client);
	return 0;
}

/*==============================================================================
FUNCTION validate_config_fields

Internal function for validating dynamic type client config fields.

ARGUMENTS
	fields - fields pointer
	num_fields - number of fields
	s_field_table - supported field table
	s_num_fields - supported field size

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int validate_config_fields(struct field_data *fields,
				  uint32_t num_fields,
				  struct field_table *s_field_table,
				  int s_num_fields)
{
	uint32_t i;

	if (fields == NULL)
		return -1;

	for (i = 0; i < num_fields; i++) {
		int k = 0;
		if (fields[i].field_name == NULL) {
			return -1;
		} else {
			for (k = 0; k < s_num_fields; k++) {
				if (0 == strcmp(fields[i].field_name,
				    s_field_table[k].field_name))
					break;
			}

			if (k >= s_num_fields)
				return -1;
		}

		if (fields[i].num_data <= 0)
			return -1;

		if (fields[i].data_type >= FIELD_MAX)
			return -1;

		if (fields[i].data == NULL)
			return -1;
	}
	return 0;
}

/*==============================================================================
FUNCTION validate_config

Internal function for validating client_config.

ARGUMENTS
	config - client requested config

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int validate_config(struct config_instance *config)
{
	enum algo_type algo = UNKNOWN_ALGO_TYPE;
	int ret = -1;

	if (config == NULL ||
	    config->cfg_desc == NULL ||
	    config->algo_type == NULL ||
	    config->num_fields <= 0 ||
	    config->fields == NULL)
		return ret;

	if (config->fields_mask == UNKNOWN_FIELD) {
		msg("Thermal-Lib:%s No adjustment request\n", __func__);
		return ret;
	}

	/* Get algo type enum if it is supported */
	algo = check_for_algo_type(UNKNOWN_ALGO_TYPE, config->algo_type);
	if (algo == UNKNOWN_ALGO_TYPE)
		return ret;

	switch(algo) {
		case SS_ALGO_TYPE:
		case PID_ALGO_TYPE:
			if(!validate_config_fields(config->fields,
						   config->num_fields,
						   pid_ss_fields,
						   ARRAY_SIZE(pid_ss_fields)))
				ret = 0;
			break;
		case MONITOR_ALGO_TYPE:
			if(!validate_config_fields(config->fields,
						   config->num_fields,
						   monitor_fields,
						   ARRAY_SIZE(monitor_fields)))
				ret = 0;
		default:
			break;
	}

	return ret;
}

/*==============================================================================
FUNCTION add_actions_info_to_sock_data

Internal function for adding action_info data to socket data.

ARGUMENTS
	data - value to be saved on the sub offset of data
	field - client field, data of which to be copied
	sub_offset - offset of data where value to be saved

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_actions_info_to_sock_data(void *data, void *field, int sub_offset)
{
	uint32_t i = 0;
	uint32_t j = 0;
	struct action_info_data *action_info = NULL;
	struct field_data *ptr_field = (struct field_data *)field;
	struct monitor_settings *ptr_m = (struct monitor_settings *)data;

	if (ptr_m == NULL ||
	    ptr_field == NULL ||
	    ptr_field->num_data <= 0)
		return -1;

	action_info = (struct action_info_data *)ptr_field->data;
	ptr_m->num_thresholds = ptr_field->num_data;

	for (i = 0; i < ptr_m->num_thresholds; i++) {
		struct override_action_t *ptr_action = NULL;
		ptr_m->t[i].num_actions = action_info[i].num_actions;
		ptr_action = (struct override_action_t *)((char *)&ptr_m->t[i] + \
								sub_offset);
		for (j = 0; j < action_info[i].num_actions; j++)
			ptr_action[j].info = action_info[i].info[j];
	}

	return 0;
}

/*==============================================================================
FUNCTION add_thresholds_to_sock_data

Internal function for adding thresholds/thresholds_clr data to
socket data.

ARGUMENTS
	data - value to be saved on the sub offset of data
	field - client field, data of which to be copied
	sub_offset - offset of data where value to be saved

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_thresholds_to_sock_data(void *data, void *field, int sub_offset)
{
	uint32_t i = 0;
	int *value = NULL;
	struct field_data *ptr_field = (struct field_data *)field;
	struct monitor_settings *ptr_m = (struct monitor_settings *)data;

	if (ptr_m == NULL ||
	    ptr_field == NULL ||
	    ptr_field->num_data <= 0)
		return -1;

	value = (int *)ptr_field->data;
	ptr_m->num_thresholds = ptr_field->num_data;

	for (i = 0; i < ptr_m->num_thresholds; i++) {
		int *int_data = NULL;
		int_data = (int *)((char *)&ptr_m->t[i] + sub_offset);
		*int_data = value[i];
	}

	return 0;
}

/*==============================================================================
FUNCTION add_algo_specific_data_to_sock_data

Internal function for add algo specific client data to socket data.

ARGUMENTS
	config - client requested config
	soc_data - socket_data for thermal
	fields - supported field table
	num_fields - number of fields supported

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_algo_specific_data_to_sock_data(struct config_instance *config,
					       struct thermal_msg_data *soc_data,
					       struct field_table *fields,
					       uint32_t num_fields)
{
	char *string;
	int *int_data;
	char *data_base;
	uint32_t i = 0;
	uint32_t j = 0;
	uint8_t count = 0;
	struct field_data *fields_local = NULL;

	if (config == NULL || soc_data == NULL ||
	    fields == NULL || num_fields < 1 ||
	    config->fields_mask == UNKNOWN_FIELD)
		return -1;

	fields_local = config->fields;
	for (i = 0; i < config->num_fields; i++) {
		for (j = 0; j < num_fields; j++) {
			if ((0 == strcmp(fields_local[i].field_name,
					  fields[j].field_name)) &&
			    fields_local[i].data_type == fields[j].type) {
				break;
			}
		}

		if (j >= num_fields)
			return -1;

		if (fields[j].field_mask == UNKNOWN_FIELD ||
		    (!(config->fields_mask & fields[j].field_mask))) {
			count++;
			continue;
		}

		switch(fields_local[i].data_type) {
			case FIELD_INT:
				data_base = (char *)soc_data;
				int_data = (int *)(data_base + fields[j].offset);
				*int_data = *(int *)fields_local[i].data;
				break;
			case FIELD_STR:
				data_base = (char *)soc_data;
				string = (char *)(data_base + fields[j].offset);
				strlcpy(string, (char *)fields_local[i].data,
				        DEVICES_MAX_NAME_LEN);
				break;
			case FIELD_INT_ARR:
				add_thresholds_to_sock_data((void *)((char *)soc_data + \
							    fields[j].offset),
							    &fields_local[i],
							    fields[j].sub_offset);
				break;
			case FIELD_ARR_INT_ARR:
				add_actions_info_to_sock_data((void *)((char *)soc_data + \
							      fields[j].offset),
							      &fields_local[i],
							      fields[j].sub_offset);
				break;
			default:
				break;
		}
	}
	/* if none of supported fields bit are set, invalid request */
	if (count == i)
		return -1;

	return 0;
}

/*==============================================================================
FUNCTION add_config_data_to_sock_data

Internal function for adding client config data to socket data.

ARGUMENTS
	config -  new client config
	soc_data  - socket data to thermal

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int add_config_data_to_sock_data(struct config_instance *config,
					struct thermal_msg_data *soc_data)
{
	if (config == NULL ||
	    soc_data == NULL ||
	    config->fields_mask == UNKNOWN_FIELD)
		return -1;

	strlcpy(soc_data->config.config_desc, config->cfg_desc, MAX_ALGO_DESC);
	soc_data->config.fields_mask = config->fields_mask;
	switch(soc_data->msg_type) {
		case MONITOR_ALGO_TYPE:
			if (add_algo_specific_data_to_sock_data(config, soc_data,
				    monitor_fields, ARRAY_SIZE(monitor_fields)))
				return -1;
			break;
		case SS_ALGO_TYPE:
		case PID_ALGO_TYPE:
			if (add_algo_specific_data_to_sock_data(config, soc_data,
				  pid_ss_fields, ARRAY_SIZE(pid_ss_fields)))
				return -1;
			break;
		default:
			return -1;
	}
	return 0;
}

/*==============================================================================
FUNCTION init_setting_for_sock_data

Internal function for initializing field of socket data.

ARGUMENTS
	socket_data   -    config data which sends over socket to thermal
	config        -    client requested config

RETURN VALUE
	Nothing
==============================================================================*/
static void init_setting_for_sock_data(struct thermal_msg_data *socket_data,
				       struct config_instance *config)
{
	uint32_t i = 0;
	enum algo_type algo = UNKNOWN_ALGO_TYPE;
	struct monitor_settings *m_setting = NULL;
	struct dynamic_settings *d_setting = NULL;

	memset(socket_data, 0, sizeof(struct thermal_msg_data));
	socket_data->msg_type = UNKNOWN_ALGO_TYPE;
	socket_data->client_name[0] = '\0';
	socket_data->config.fields_mask = UNKNOWN_FIELD;
	socket_data->config.disable = 0;
	socket_data->config.sampling = 0;
	socket_data->config.config_desc[0] = '\0';

	/* Get algo type enum it is supported */
	algo = check_for_algo_type(UNKNOWN_ALGO_TYPE, config->algo_type);
	if (algo == UNKNOWN_ALGO_TYPE)
		return;

	socket_data->msg_type = algo;
	switch(socket_data->msg_type) {
		case MONITOR_ALGO_TYPE:
			m_setting = &socket_data->config.m_setting;
			m_setting->num_thresholds = 0;
			for (i = 0; i < THRESHOLDS_MAX; i++) {
				uint32_t j = 0;
				m_setting->t[i].threshold_trig = INT_MIN;
				m_setting->t[i].threshold_clr = INT_MIN;
				m_setting->t[i].num_actions = 0;
				for (j = 0; j < ACTIONS_MAX; j++) {
					m_setting->t[i].actions[j].device[0] = '\0';
					m_setting->t[i].actions[j].info = INT_MIN;
				}
			}
			break;
		case SS_ALGO_TYPE:
		case PID_ALGO_TYPE:
			d_setting = &socket_data->config.d_setting;
			d_setting->set_point = INT_MIN;
			d_setting->set_point_clr = INT_MIN;
			d_setting->device[0] = '\0';
			break;
		default:
			break;
	}
	return;
}

/*==============================================================================
FUNCTION thermal_client_config_set

Thermal client config field set request to thermal function.

ARGUMENTS
	configs  - struct config_instance whose fields need to modify
	num_configs    - number of config instances

RETURN VALUE
	num of config sent on success, negative error(-1) on failure.
==============================================================================*/
int  thermal_client_config_set(struct  config_instance *configs,
			       unsigned int config_size)
{
	ssize_t rc = 0;
	uint8_t i;
	int sockfd_client = -1;
	int num_config_send = 0;
	struct sockaddr_un client_addr;
	struct thermal_msg_data config_set_data;

	if (NULL == configs ||
	    config_size == 0) {
		msg("Thermal-Lib-Client:%s: unexpected NULL", __func__);
		return -1;
	}

	for (i = 0; i < config_size; i++) {
		if (validate_config(&configs[i])) {
			msg("Thermal-Lib-Client:%s: request config is not valid",
			   __func__);
			return -1;
		}
	}

	sockfd_client = connect_to_socket(CONFIG_SET_CLIENT, &client_addr);
	if (sockfd_client < 0) {
		msg("Thermal-Lib-Client:%s: Could not connect to socket\n",
		     __func__);
		return -1;
	}

	/* Send first packet as number of configs for set request config
	   (msg_type = ALGO_IDX_MAX and req_data = config_size) */
	memset(&config_set_data, 0, sizeof(struct thermal_msg_data));
	config_set_data.msg_type = ALGO_IDX_MAX;
	config_set_data.req_data = (int)config_size;
	strlcpy(config_set_data.client_name, CONFIG_SET_CLIENT, CLIENT_NAME_MAX);

	rc = send(sockfd_client, &config_set_data,
		  sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
	if (rc <= 0) {
		msg("Thermal-Lib-Client: "
		    "Unable to send request data to fd %d",
		     sockfd_client);
		close(sockfd_client);
		return -1;
	}

	for (i = 0; i < config_size; i++) {
		init_setting_for_sock_data(&config_set_data, &configs[i]);
		strlcpy(config_set_data.client_name, CONFIG_SET_CLIENT,
		        CLIENT_NAME_MAX);

		if (add_config_data_to_sock_data(&configs[i], &config_set_data)) {
			msg("Thermal-Lib-Client: config %d data is not valid\n",
									i+1);
			continue;
		}
		rc = send(sockfd_client, &config_set_data,
			  sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
		if (rc <= 0) {
			msg("Thermal-Lib-Client: "
			    "Unable to send request data to fd %d",
			     sockfd_client);
			continue;
		}
		num_config_send++;
		info("Thermal-Lib-Client: Client config query request sent %s  %d\n",
			config_set_data.client_name, config_set_data.msg_type);
	}
	/* Send last packet as EOM.
	   (msg_type = ALGO_IDX_MAX and req_data = 0) */
	memset(&config_set_data, 0, sizeof(struct thermal_msg_data));
	config_set_data.msg_type = ALGO_IDX_MAX;
	config_set_data.req_data = 0;
	strlcpy(config_set_data.client_name, CONFIG_SET_CLIENT, CLIENT_NAME_MAX);

	rc = send(sockfd_client, &config_set_data,
		  sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
	if (rc <= 0) {
		msg("Thermal-Lib-Client: "
		    "Unable to send request data to fd %d",
		     sockfd_client);
		close(sockfd_client);
		return -1;
	}

	close(sockfd_client);
	return num_config_send;
}

/*==============================================================================
FUNCTION thermal_client_config_cleanup

Thermal client configs clean up function.

ARGUMENTS
	configs  - config list head to clean up
	config_size - size of configs

RETURN VALUE
	Nothing
==============================================================================*/
void thermal_client_config_cleanup(struct  config_instance *configs,
                                   unsigned int config_size)
{
	uint32_t idx = 0;

	if (configs == NULL ||
	    config_size < 1)
		return;

	for (idx = 0; idx < config_size; idx++) {
		uint8_t i = 0;

		if (configs[idx].cfg_desc != NULL)
			free(configs[idx].cfg_desc);

		if (configs[idx].algo_type != NULL)
			free(configs[idx].algo_type);

		if (configs[idx].fields == NULL)
			continue;

		for (i = 0; i < configs[idx].num_fields; i++) {
			if (configs[idx].fields[i].field_name != NULL)
				free(configs[idx].fields[i].field_name);

			if (configs[idx].fields[i].data != NULL)
				free(configs[idx].fields[i].data);
		}
		free(configs[idx].fields);
	}
	free(configs);
	return;
}

/*============================================================================================
FUNCTION do_listen

Function to listen thermal socket.
This function will run in a separate thread.

ARGUMENTS
	data - data pointer.

RETURN VALUE
	void * - not used.
=============================================================================================*/
static void *do_listen(void *data)
{
	ssize_t rc;
	uint32_t i;
	int count;
	int (*callback)(int, void *, void *);
	struct thermal_cdata *callback_node;
	static struct sockaddr_un client_addr;
	struct thermal_msg_data thermal_msg;
	int sockfd_client_recv = -1;
	fd_set readfds, testfds;

	if (pipe(pipe_fds))
		msg("Thermal-Client-Lib: Pipe error");

	while (thermal_client_shutdown != 1) {

		sockfd_client_recv = socket(AF_LOCAL, SOCK_STREAM, 0);
		if (sockfd_client_recv < 0) {
			sleep(5);
			continue;
		}

		memset(&client_addr, 0, sizeof(struct sockaddr_un));
		snprintf(client_addr.sun_path, UNIX_PATH_MAX, THERMAL_SEND_CLIENT_SOCKET);
		client_addr.sun_family = AF_LOCAL;

		rc = connect(sockfd_client_recv, (struct sockaddr *)&client_addr,
				     (socklen_t)(sizeof(sa_family_t) + strlen(THERMAL_SEND_CLIENT_SOCKET)));
		if (rc != 0) {
			close(sockfd_client_recv);
			sleep(5);
			continue;
		}

		FD_ZERO(&readfds);
		FD_SET(sockfd_client_recv, &readfds);
		FD_SET(pipe_fds[0], &readfds);
		while (thermal_client_shutdown != 1) {

			testfds = readfds;
			rc = select(FD_SETSIZE, &testfds, (fd_set *)0,
				           (fd_set *)0, (struct timeval *)0);
			if (rc < 1)
				continue;

                        if (FD_ISSET(pipe_fds[0], &testfds))
				break;

			memset(&thermal_msg, 0, sizeof(struct thermal_msg_data));
			rc = recv(sockfd_client_recv, &thermal_msg, sizeof(struct thermal_msg_data), 0);
			if (rc <= 0) {
				msg("Thermal-Lib-Client:%s: recv failed", __func__);
				break;
			}

			if (rc != sizeof(struct thermal_msg_data))
				continue;

			for (i = 0; i < CLIENT_NAME_MAX; i++) {
				if (thermal_msg.client_name[i] == '\0')
					break;
			}
			if (i >= CLIENT_NAME_MAX)
				thermal_msg.client_name[CLIENT_NAME_MAX - 1] = '\0';

			info("Thermal-Lib-Client: Client received msg %s %d",
					thermal_msg.client_name, thermal_msg.req_data);

			/* Check for client is supported  or not*/
			for (i = 0; i < ARRAY_SIZE(notify_clients); i++) {
				if (0 == strncmp(notify_clients[i].name, thermal_msg.client_name, CLIENT_NAME_MAX))
					break;
			}

			if (i >= ARRAY_SIZE(notify_clients)) {
				msg("Thermal-Lib-Client:%s is not in supported "
				    "thermal client list", thermal_msg.client_name);
				continue;
			} else if (thermal_msg.req_data < notify_clients[i].min_req_data ||
				   thermal_msg.req_data > notify_clients[i].max_req_data) {
				msg("Thermal-Lib-Client:%s: invalid level %d "
				    "unexpected", __func__, thermal_msg.req_data);
				continue;
			}

			callback_node = list_head;
			count = 0;
			for (; callback_node != NULL; callback_node = callback_node->next) {

				callback_node = get_callback_node_from_list(callback_node, thermal_msg.client_name);
				if (callback_node) {
					count++;
					callback = callback_node->callback;
					if (callback)
						callback(thermal_msg.req_data, callback_node->user_data,
									    callback_node->data_reserved);
				} else {
					if (count == 0)
						msg("Thermal-Lib-Client: No Callback "
						    "registered for %s", thermal_msg.client_name);
					break;
				}
			}
		}
		close(sockfd_client_recv);
		FD_CLR(sockfd_client_recv, &readfds);
		FD_CLR(pipe_fds[0], &readfds);
	}
	close(pipe_fds[0]);
	close(pipe_fds[1]);

	return NULL;
}

/*================================================================================================
FUNCTION thermal_client_register_callback

Thermal client registration function.
The client is registered with name and a callback funcion and keep on connecting
thermal local socket.Based on client name in the message from socket,corresponding
callback will be called.

ARGUMENTS
	client_name - client name
	callback    - callback function pointer with level, user_data pointer and
	              reserved data as arguments
	data        - user data

RETURN VALUE
	valid non zero client_cb_handle on success, zero on failure.
=================================================================================================*/
int thermal_client_register_callback(char *client_name, int (*callback)(int, void *, void *), void *data)
{
	int rc = 0;
	int ret = 0;
	uint8_t i;
	uint32_t client_cb_handle;

	if (NULL == client_name ||
	    NULL == callback) {
		msg("Thermal-Lib-Client:%s: unexpected NULL client registraion "
		    "failed ", __func__);
		return ret;
	}

	/* Check for client is supported  or not*/
	for (i = 0; i < ARRAY_SIZE(notify_clients); i++) {
		if (0 == strncmp(notify_clients[i].name, client_name, CLIENT_NAME_MAX))
			break;
	}

	if (i >= ARRAY_SIZE(notify_clients)) {
		msg("Thermal-Lib-Client:%s is not in supported thermal client list", client_name);
		return ret;
	}

	pthread_mutex_lock(&client_mutex);

	if (thermal_client_shutdown != 0)
		thermal_client_shutdown = 0;

	client_cb_handle = add_to_list(client_name, callback, data);
	if (client_cb_handle == 0) {
		msg("Thermal-Lib-Client: %s: Client Registration failed", __func__);
		goto error_handler;
	}

	if (first_client == 1) {
		rc = pthread_create(&thermal_client_recv_thread, NULL, do_listen, NULL);
		if (rc != 0) {
			msg("Thermal-Lib-Client: Unable to create pthread to "
			    "listen thermal events for %s", client_name);
			remove_from_list(client_cb_handle);
			goto error_handler;
		}
		first_client = 0;
	}

	info("Thermal-Lib-Client: Registraion successful "
	     "for %s with handle:%d", client_name, client_cb_handle);
	ret = (int)client_cb_handle;
error_handler:
	pthread_mutex_unlock(&client_mutex);
	return ret;
}

/*===========================================================================
FUNCTION thermal_client_unregister_callback

Function to unregister client req_handler.

ARGUMENTS
	client_cb_handle - client handle which retured on
	                   succesful registeration

RETURN VALUE
	void  - return nothing.
===========================================================================*/
void thermal_client_unregister_callback(int client_cb_handle)
{
	pthread_mutex_lock(&client_mutex);

	if (remove_from_list((unsigned int)client_cb_handle) < 0) {
		msg("Thermal-Lib-Client: thermal client unregister callback error");
	} else {
		if (list_head == NULL) {
			thermal_client_shutdown = 1;
			write(pipe_fds[1], "exit", 5);
			pthread_join(thermal_client_recv_thread, NULL);
			first_client = 1;
		}
		info("Thermal-Lib-Client: Unregisteration is successfull for "
		     "handle:%d", client_cb_handle);
	}
	pthread_mutex_unlock(&client_mutex);
}
