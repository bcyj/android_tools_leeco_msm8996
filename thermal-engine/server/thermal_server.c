/*========================================================================================

 thermal_server.c

 GENERAL DESCRIPTION
 Thermal socket server related code to communicate with thermal DLL.

 Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

==========================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stringl.h>
#include "thermal.h"
#include "thermal_lib_common.h"
#include "thermal_server.h"
#ifdef ANDROID
#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

static int sockfd_server_send = -1;
static int sockfd_server_recv = -1;
static int sockfd_server_recv_passive = -1;
static int sockfd_server_log = -1;
static int config_query_client_fd = -1;
static pthread_t listen_client_fd_thread;
static int server_socket_exit = 0;
static pthread_mutex_t soc_client_mutex;
static int thermal_send_fds[NUM_LISTEN_QUEUE];
struct thermal_cdata *list_head = NULL;

enum {
	NON_MITIGATION_DEVICE,
	MITIGATION_DEVICE,
} client_device_type;

struct req_client {
	char *name;
	int  *fd;
};
struct notify_client {
	char *name;
	int cur_level;
	int device_type;
};
static struct req_client req_clients[] = {
	{
		.name = "spkr",
		.fd   = &sockfd_server_recv_passive,
	},
	{
		.name = "override",
		.fd   = &sockfd_server_recv,
	},
	{
		.name = CONFIG_QUERY_CLIENT,
		.fd   = &sockfd_server_recv,
	},
	{
		.name = CONFIG_SET_CLIENT,
		.fd   = &sockfd_server_recv,
	},
	{
		.name = UI_LOCALSOCKET_NAME,
		.fd   = &sockfd_server_log,
	},
};

static struct notify_client notify_clients[] =
{
	{
		.name      = "camera",
		.cur_level = 0,
		.device_type = MITIGATION_DEVICE,
	},
	{
		.name      = "camcorder",
		.cur_level = 0,
		.device_type = MITIGATION_DEVICE,
	},
	{
		.name      = "spkr",
		.cur_level = 0,
		.device_type = NON_MITIGATION_DEVICE,
	},
	{
		.name      = CONFIG_QUERY_CLIENT,
		.cur_level = 0,
		.device_type = NON_MITIGATION_DEVICE,
	},
};

/*========================================================================================
FUNCTION thermal_config_update_recv

Receive requested data from client through socket for config parameter set request.
First message(argument) is for number of configs requests at a time, then it will
recv valid configs one by one till EOM and invokes appropriate thermal callback
from list which already registered.

ARGUMENTS
	client_msg -  config data from socket
	client_fd  -  client fd

RETURN VALUE
	0 on success, negative error(-1) on failure.
========================================================================================*/
static int thermal_config_update_recv(struct thermal_msg_data client_msg,
				      int client_fd)
{
	ssize_t rc;
	int ret = 0;
	int count = 0;
	int msg_idx;
	int (*callback)(int, void *, void *);
	struct thermal_cdata *callback_node;
	struct thermal_msg_data thermal_msg[MAX_CONFIG_INSTANCES_SUPPORTED + 1];
	int num_configs = 0;

	if (client_fd < 0 ||
	    0 != strcmp(client_msg.client_name, CONFIG_SET_CLIENT))
		return -1;

	/* First packet will be number of valid configs going to recv */
	if (client_msg.msg_type != ALGO_IDX_MAX) {
		msg("Thermal-Server: %s: Unexpected data from client\n",
		    __func__);
		return -1;
	}

	if (client_msg.msg_type == ALGO_IDX_MAX) {
		if (client_msg.req_data > 0) {
			num_configs = client_msg.req_data;
			info("Thermal-Server: Num of config set request %d\n",
			      num_configs);
		} else if (client_msg.req_data == 0) {
			msg("Thermal-Server: Num of config set request %d\n",
			      num_configs);
			return -1;
		}
	}

	msg_idx = 0;
	while (1) {
		if (msg_idx > num_configs) {
			msg("Thermal-Server: %s: Invalid number of configs\n", __func__);
			return -1;
		}

		memset(&thermal_msg[msg_idx], 0, sizeof(struct thermal_msg_data));
		rc = recv(client_fd, (struct thermal_msg_data *)&thermal_msg[msg_idx],
			  sizeof(struct thermal_msg_data), 0);
		if (rc <= 0) {
			msg("Thermal-Server:%s: recv failed, received byte %zd\n",
			     __func__, rc);
			ret = -1;
			break;
		}
		if (rc != sizeof(struct thermal_msg_data)) {
			msg("Thermal-Server: %s: Invalid size of recv message\n",
			    __func__);
			ret = -1;
			break;
		}
		/* Check for EOM condition */
		if (thermal_msg[msg_idx].msg_type == ALGO_IDX_MAX &&
		    thermal_msg[msg_idx].req_data == 0) {
			info("Thermal-Server: Num of config set request %d\n",
			      num_configs);
			break;
		}

		if (strnlen(thermal_msg[msg_idx].client_name, CLIENT_NAME_MAX) >=
								CLIENT_NAME_MAX) {
			msg("Thermal-Server: %s: Invalid client name %s\n",
			    __func__, thermal_msg[msg_idx].client_name);
				ret = -1;
				break;
		}

		if (0 != strcmp(thermal_msg[msg_idx].client_name, CONFIG_SET_CLIENT)) {
			msg("Thermal-Server: %s: Invalid client\n", __func__);
			ret = -1;
			break;
		}
		msg_idx++;
	}

	if (ret || (msg_idx != num_configs))
		return -1;

	for (callback_node = list_head; callback_node != NULL;) {

		callback_node = get_callback_node_from_list(callback_node,
						client_msg.client_name);
		if (callback_node) {
			count++;
			callback = callback_node->callback;
			if (callback) {
				callback_node->data_reserved =
					 (struct thermal_msg_data *)thermal_msg;
				callback(num_configs, callback_node->user_data,
					 callback_node->data_reserved);
			}
			callback_node = callback_node->next;
		} else {
			if (count == 0)
				msg("Thermal-Server: No clients are "
				    "connected for %s", client_msg.client_name);
		}
	}

	return 0;
}

/*==============================================================================
FUNCTION thermal_config_query_recv

Receive requested data from client through socket for config_query request.
Server sends first packet to inform number of valid configs and each algo
callback from list which already registered sends its own configs and
finally server sends EOM packet.

ARGUMENTS
	client_msg -  config data from socket
	client_fd  -  client fd

RETURN VALUE
	0 on success, negative error(-1) on failure.
==============================================================================*/
static int thermal_config_query_recv(struct thermal_msg_data client_msg,
				      int client_fd)
{
	int count = 0;
	int (*callback)(int, void *, void *);
	struct thermal_cdata *callback_node;
	struct thermal_msg_data thermal_msg;
	int num_cfg_req_per_algo = 1;
	int num_configs = 0;

	if (client_fd < 0 ||
	    0 != strcmp(client_msg.client_name, CONFIG_QUERY_CLIENT))
		return -1;

	config_query_client_fd = client_fd;

	/* Get num of requested config file and send it to client as a first
	   packet(msg_type = ALGO_IDX_MAX and req_data = num_configs) */
	for (callback_node = list_head; callback_node != NULL;) {
		callback_node = get_callback_node_from_list(callback_node,
						client_msg.client_name);
		if (callback_node) {
			callback = callback_node->callback;
			if (callback)
				num_configs = num_configs + \
					callback(client_msg.msg_type,
						 callback_node->user_data,
						 (int *)&num_cfg_req_per_algo);
			callback_node = callback_node->next;
		}
	}

	info("Thermal-Server: total num of config queried %d\n", num_configs);
	memset(&thermal_msg, 0, sizeof(struct thermal_msg_data));
	thermal_msg.msg_type = ALGO_IDX_MAX;
	thermal_msg.req_data = num_configs;
	strlcpy(thermal_msg.client_name, CONFIG_QUERY_CLIENT, CLIENT_NAME_MAX);
	thermal_server_config_info_to_client(&thermal_msg);

	for (callback_node = list_head; callback_node != NULL;) {
		callback_node = get_callback_node_from_list(callback_node,
							client_msg.client_name);
		if (callback_node) {
			count++;
			callback = callback_node->callback;
			if (callback)
				callback(client_msg.msg_type,
				         callback_node->user_data, NULL);
			callback_node = callback_node->next;
		} else {
			if (count == 0)
				msg("Thermal-Server: No clients are "
				    "connected for %s", client_msg.client_name);
		}
	}

	/* End of packet algo_type ALGO_IDX_MAX and req_data = 0 */
	memset(&thermal_msg, 0, sizeof(struct thermal_msg_data));
	thermal_msg.msg_type = ALGO_IDX_MAX;
	thermal_msg.req_data = 0;
	strlcpy(thermal_msg.client_name, CONFIG_QUERY_CLIENT, CLIENT_NAME_MAX);
	thermal_server_config_info_to_client(&thermal_msg);

	config_query_client_fd = -1;
	return 0;
}

/*========================================================================================
FUNCTION thermal_recv_data_from_client

Recieve requested data from client through socket and based on data
invoke appropriate thermal callback from list which already registered.

ARGUMENTS
	client_fd -  client fd for thermal recv socket
	socket_fd -  socket fd

RETURN VALUE
	0 on success, negative on failure.
========================================================================================*/
static int thermal_recv_data_from_client(int client_fd, int socket_fd)
{
	ssize_t rc;
	uint32_t i;
	int ret = -EINVAL;
	int count = 0;
	int (*callback)(int, void *, void *);
	struct thermal_cdata *callback_node;
	struct thermal_msg_data client_msg;

	if ((client_fd == -1) || (socket_fd == -1))
		return ret;

	memset(&client_msg, 0, sizeof(struct thermal_msg_data));
	rc = recv(client_fd, &client_msg, sizeof(struct thermal_msg_data), 0);
	if (rc <= 0) {
		msg("Thermal-Server: %s: recv failed", __func__);
		return ret;
	}

	if (rc != sizeof(struct thermal_msg_data))
		return ret;

	for (i = 0; i < CLIENT_NAME_MAX; i++) {
		if (client_msg.client_name[i] == '\0')
			break;
	}
	if (i >= CLIENT_NAME_MAX)
		client_msg.client_name[CLIENT_NAME_MAX - 1] = '\0';
	info("Thermal-Server: Thermal received "
	     "msg from  %s\n", client_msg.client_name);

	/* Check for client is supported or not*/
	for (i = 0; i < ARRAY_SIZE(req_clients); i++) {
		if (0 == strncmp(req_clients[i].name, client_msg.client_name,
				 CLIENT_NAME_MAX) &&
		    *(req_clients[i].fd) == socket_fd)
			break;
	}

	if (i >= ARRAY_SIZE(req_clients)) {
		msg("Thermal-Server:%s is not in supported "
		    "thermal client list", client_msg.client_name);
		return ret;
	}

	if (0 == strncmp(client_msg.client_name, CONFIG_QUERY_CLIENT, CLIENT_NAME_MAX)) {
		thermal_config_query_recv(client_msg, client_fd);
		return 0;
	} else if (0 == strncmp(client_msg.client_name, CONFIG_SET_CLIENT, CLIENT_NAME_MAX)) {
		thermal_config_update_recv(client_msg, client_fd);
		return 0;
	}

	for (callback_node = list_head; callback_node != NULL; callback_node = callback_node->next) {

		callback_node = get_callback_node_from_list(callback_node, client_msg.client_name);
		if (callback_node) {
			count++;
			callback = callback_node->callback;
			if (callback)
				callback(client_msg.req_data, callback_node->user_data, callback_node->data_reserved);
		} else {
			if (count == 0)
				msg("Thermal-Server: No clients are "
				    "connected for %s", client_msg.client_name);
			break;
		}
	}
	ret = 0;
	return ret;
}

/*========================================================================================
FUNCTION notify_client_on_register

Notify thermal current level to client if that client register with
thermal on target thermal condition.

ARGUMENTS
	client_fd - client fd for thermal send socket

RETURN VALUE
	Void
========================================================================================*/
static void notify_client_on_register(int client_fd)
{
	ssize_t rc;
	int i;
	struct thermal_msg_data thermal_msg;

	pthread_mutex_lock(&soc_client_mutex);

	for (i = 0; i < ARRAY_SIZE(notify_clients); i++) {
		if (notify_clients[i].device_type == MITIGATION_DEVICE) {

			memset(&thermal_msg, 0, sizeof(struct thermal_msg_data));
			strlcpy(thermal_msg.client_name, notify_clients[i].name, CLIENT_NAME_MAX);
			thermal_msg.req_data = notify_clients[i].cur_level;

			rc = send(client_fd, &thermal_msg,
				      sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
			if (rc <= 0)
				msg("Thermal-Server: Unable to send "
				    "event to fd %d", client_fd);
		}
	}

	pthread_mutex_unlock(&soc_client_mutex);
}

/*========================================================================================
FUNCTION do_listen_client_fd

Seperate thread function for monitoring thermal usersapce clients and recieving
client data from thermal recv socket once it is notified.

ARGUMENTS
	data - data, not used

RETURN VALUE
	void
========================================================================================*/
static void *do_listen_client_fd(void *data)
{
	int fd;
	uint8_t i;
	int nread;
	int result;
	socklen_t client_len;
	int client_fd = -1;
	struct sockaddr_un client_addr;
	fd_set t_readfds,testfds;

	FD_ZERO(&t_readfds);
	FD_SET(sockfd_server_send, &t_readfds);
	FD_SET(sockfd_server_recv, &t_readfds);
	FD_SET(sockfd_server_recv_passive, &t_readfds);
	FD_SET(sockfd_server_log, &t_readfds);

	for (i = 0; i < NUM_LISTEN_QUEUE; i++) {
		thermal_send_fds[i] = -1;
	}

	while(server_socket_exit != 1) {
		testfds = t_readfds;
		result = select(FD_SETSIZE, &testfds, (fd_set *)0,
			       (fd_set *)0, (struct timeval *) 0);
		if (result < 1) {
			msg("Thermal-Server: %s select error", __func__);
			break;
		}

		for (fd = 0; fd < FD_SETSIZE; fd++) {
			if (FD_ISSET(fd,&testfds)) {
				if (fd == sockfd_server_send) {
					client_len = sizeof(struct sockaddr_un);
					client_fd = accept(sockfd_server_send,
						          (struct sockaddr *)&client_addr,
						           &client_len);
					if (client_fd < 0)
						continue;
					FD_SET(client_fd, &t_readfds);
					info("Thermal-Server: Adding thermal event listener on fd %d\n", client_fd);
					for (i = 0; i < NUM_LISTEN_QUEUE && thermal_send_fds[i] != -1; i++)
							continue;
						if (i < NUM_LISTEN_QUEUE)
							thermal_send_fds[i] = client_fd;
					notify_client_on_register(client_fd);

				} else if (fd == sockfd_server_recv ||
					   fd == sockfd_server_recv_passive) {
					client_len = sizeof(struct sockaddr_un);
					client_fd = accept(fd,
						          (struct sockaddr *)&client_addr,
						           &client_len);
					if (client_fd < 0)
						continue;
					thermal_recv_data_from_client(client_fd, fd);
					close(client_fd);
				} else if (fd == sockfd_server_log) {
					client_len = sizeof(struct sockaddr_un);
					client_fd = accept(fd,
						           (struct sockaddr *)&client_addr,
							    &client_len);
					if (client_fd < 0)
						continue;
					add_local_socket_fd(client_fd);
				} else {
					ioctl(fd, FIONREAD, &nread);
					if (nread == 0) {
						close(fd);
						FD_CLR(fd, &t_readfds);
						info("Thermal-Server: removing client on fd %d\n", fd);
						for (i = 0; i < NUM_LISTEN_QUEUE && thermal_send_fds[i] != fd; i++)
							continue;
						if (i < NUM_LISTEN_QUEUE) {
							thermal_send_fds[i] = -1;
						}
					}
				}
			}
		}
	}
	for (i = 0; i < NUM_LISTEN_QUEUE; i++) {
		if (thermal_send_fds[i] > -1)
			close(thermal_send_fds[i]);
	}
	return NULL;
}

/*========================================================================================
FUNCTION thermal_server_init

Thermal socket initialization function for different clients
This function setup a server for both thermal sending socket and
thermal recieving socket.

ARGUMENTS
	none

RETURN VALUE
	0 on success, negative on failure.
=========================================================================================*/
int thermal_server_init(void)
{
	int rc;
	int ret = -EINVAL;
	static struct sockaddr_un server_addr_send;
	static struct sockaddr_un server_addr_recv;
	static struct sockaddr_un server_addr_recv_passive;
	static struct sockaddr_un server_log;

	if (sockfd_server_send != -1 || sockfd_server_recv != -1 ||
	    sockfd_server_recv_passive != -1 || sockfd_server_log != -1) {
		sockfd_server_send = -1;
		sockfd_server_recv = -1;
		sockfd_server_recv_passive = -1;
		sockfd_server_log = -1;
	}
#ifdef ANDROID
	sockfd_server_send = android_get_control_socket(
				THERMAL_SEND_SOCKET_NAME);
	/* If socket in not created from init, create it from thermal-engine */
	if (sockfd_server_send < 0) {
		msg("Error in SOCKET CREATE. CREATING IT LOCALLY\n");
		goto create_file_socket;
	}

	sockfd_server_recv = android_get_control_socket(
				THERMAL_RECV_SOCKET_NAME);
	if (sockfd_server_recv < 0)
		goto error;

	sockfd_server_recv_passive = android_get_control_socket(
				THERMAL_RECV_PASSIVE_SOCKET_NAME);
	if (sockfd_server_recv_passive < 0)
		goto error;

	goto end_file_socket_create;
#endif
create_file_socket:
	sockfd_server_send = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd_server_send < 0) {
		goto error;
	}

	sockfd_server_recv = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd_server_recv < 0) {
		goto error;
	}

	sockfd_server_recv_passive = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd_server_recv_passive < 0) {
		goto error;
	}

	memset(&server_addr_send, 0, sizeof(struct sockaddr_un));
	snprintf(server_addr_send.sun_path, UNIX_PATH_MAX, THERMAL_SEND_CLIENT_SOCKET);
	server_addr_send.sun_family = AF_LOCAL;

	memset(&server_addr_recv, 0, sizeof(struct sockaddr_un));
	snprintf(server_addr_recv.sun_path, UNIX_PATH_MAX, THERMAL_RECV_CLIENT_SOCKET);
	server_addr_recv.sun_family = AF_LOCAL;

	memset(&server_addr_recv_passive, 0, sizeof(struct sockaddr_un));
	snprintf(server_addr_recv_passive.sun_path, UNIX_PATH_MAX, THERMAL_RECV_PASSIVE_CLIENT_SOCKET);
	server_addr_recv_passive.sun_family = AF_LOCAL;

	/* Delete existing socket file if necessary */
	unlink(server_addr_send.sun_path);
	unlink(server_addr_recv.sun_path);
	unlink(server_addr_recv_passive.sun_path);

	rc = bind(sockfd_server_send,(struct sockaddr  const *)&server_addr_send, sizeof(struct sockaddr_un));
	if (rc != 0) {
		msg("Thermal-Server: Send socket: bind error - %s", (char *)strerror(errno));
		goto error;
	}
	rc = bind(sockfd_server_recv,(struct sockaddr  const *)&server_addr_recv, sizeof(struct sockaddr_un));
	if (rc != 0) {
		msg("Thermal-Server: Recv socket: bind error - %s", strerror(errno));
		goto error;
	}

	rc = bind(sockfd_server_recv_passive,(struct sockaddr  const *)&server_addr_recv_passive, sizeof(struct sockaddr_un));
	if (rc != 0) {
		msg("Thermal-Server: Recv passive socket: bind error - %s", strerror(errno));
		goto error;
	}

	/* Allow any client to connect to send socket.
	   Allow any client to connect to passive receive socket.
	   Only root group clients can connect recieve socket. */
	chmod(server_addr_send.sun_path, 0666);
	chmod(server_addr_recv.sun_path, 0660);
	chmod(server_addr_recv_passive.sun_path, 0666);

end_file_socket_create:
	sockfd_server_log = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd_server_log < 0)
		goto error;

	memset(&server_log, 0, sizeof(struct sockaddr_un));
	snprintf(server_log.sun_path, UNIX_PATH_MAX, " %s",
			UI_LOCALSOCKET_NAME);
	server_log.sun_family = AF_LOCAL;

	/* abstract namespace socket starts with NULL char */
	server_log.sun_path[0] = '\0';
	rc = bind(sockfd_server_log,
			(struct sockaddr  const *)&server_log,
			(int)(sizeof(sa_family_t) + 1
			+ strlen(UI_LOCALSOCKET_NAME)));

	if (rc != 0) {
		msg("Thermal-Server: Server log socket: bind error - %s",
			strerror(errno));
		goto error;
	}
	rc = listen(sockfd_server_send, NUM_LISTEN_QUEUE);
	if (rc != 0) {
		goto error;
	}

	rc = listen(sockfd_server_recv, NUM_LISTEN_QUEUE);
	if (rc != 0) {
		goto error;
	}

	rc = listen(sockfd_server_recv_passive, NUM_LISTEN_QUEUE);
	if (rc != 0) {
		goto error;
	}

	rc = listen(sockfd_server_log, NUM_LISTEN_QUEUE);
	if (rc != 0) {
		goto error;
	}

	rc = pthread_create(&listen_client_fd_thread, NULL, do_listen_client_fd, NULL);
	if (rc != 0) {
		msg("Thermal-Server: Unable to create pthread to "
		    "listen thermal clients fd");
		goto error;
	}

	ret = 0;
	return ret;
error:
	msg("Thermal-Server: Unable to create socket server for thermal clients");
	if (sockfd_server_send != -1) {
		close(sockfd_server_send);
		sockfd_server_send = -1;
	}
	if (sockfd_server_recv != -1) {
		close(sockfd_server_recv);
		sockfd_server_recv = -1;
	}
	if (sockfd_server_recv_passive != -1) {
		close(sockfd_server_recv_passive);
		sockfd_server_recv_passive = -1;
	}
	if (sockfd_server_log != -1) {
		close(sockfd_server_log);
		sockfd_server_log = -1;
	}
	return ret;
}

/*=======================================================================================
FUNCTION thermal_server_config_query_to_clients

Function to send thermal config data through socket to thermal clients.

ARGUMENTS
	data  - thermal query data send back to client

RETURN VALUE
	return 0 on success and -EINVAL on failure.
========================================================================================*/
int thermal_server_config_info_to_client(void *data)
{
	ssize_t rc = 0;
	int i;

	if (data == NULL || config_query_client_fd < 0)
		return -EINVAL;

	struct thermal_msg_data *query_data = (struct thermal_msg_data *)data;

	/* Check for client is supported or not*/
	for (i = 0; i < ARRAY_SIZE(notify_clients); i++) {
		if (0 == strncmp(notify_clients[i].name, query_data->client_name, CLIENT_NAME_MAX))
			break;
	}

	if (i >= ARRAY_SIZE(notify_clients)) {
		msg("Thermal-Server: %s is not in supported "
		    "thermal client list", query_data->client_name);
		return -EINVAL;
	}

	rc = send(config_query_client_fd, query_data, sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
	if (rc <= 0) {
		msg("Thermal-Server: Unable to send "
		    "event to fd %d", config_query_client_fd);
	}

	return 0;
}
/*=======================================================================================
FUNCTION thermal_server_notify_clients

Function to send thermal event through socket to thermal clients.

ARGUMENTS
	client_name - client_name
	level       - mitigation level

RETURN VALUE
	return 0 on success and -EINVAL on failure.
========================================================================================*/
int thermal_server_notify_clients(char *client_name, int level)
{
	ssize_t rc;
	uint8_t i = 0;
	int ret = -EINVAL;
	int send_count = 0;
	struct thermal_msg_data thermal_msg;

	/* Check for client is supported  or not*/
	for (i = 0; i < ARRAY_SIZE(notify_clients); i++) {
		if (0 == strncmp(notify_clients[i].name, client_name, CLIENT_NAME_MAX))
			break;
	}

	if (i >= ARRAY_SIZE(notify_clients)) {
		msg("Thermal-Server: %s is not in supported "
		    "thermal client list", client_name);
		return ret;
	}

	memset(&thermal_msg, 0, sizeof(struct thermal_msg_data));
	strlcpy(thermal_msg.client_name, client_name, CLIENT_NAME_MAX);
	thermal_msg.req_data = level;

	pthread_mutex_lock(&soc_client_mutex);

	notify_clients[i].cur_level = level;

	if (sockfd_server_send == -1) {
		msg("Thermal-Server: socket is not created, Trying to start server..");
		thermal_server_release();
		if ( thermal_server_init() || sockfd_server_send == -1) {
			goto handle_error;
		}
	}

	/* Send event to all connected clients */
	for (i = 0; i < NUM_LISTEN_QUEUE; i++) {
		if(thermal_send_fds[i] != -1) {
			send_count++;
			rc = send(thermal_send_fds[i], &thermal_msg,
				      sizeof(struct thermal_msg_data), MSG_NOSIGNAL);
			if (rc <= 0) {
				msg("Thermal-Server: Unable to send "
				    "event to fd %d", thermal_send_fds[i]);
			}
		}
	}

	if (send_count == 0)
		info("Thermal-Server: No client connected to socket");

	ret = 0;

handle_error:
	pthread_mutex_unlock(&soc_client_mutex);
	return ret;
}

/*================================================================================================
FUNCTION thermal_server_register_client_req_handler

Thermal register function to register callback
Whenever thermal wants to register a new callback for clients,
invoke this API.

ARGUMENTS
	client_name - client name
	callback    - callback function pointer with req_data, user_data pointer and
	              reserved data as arguments
	data        - data

RETURN VALUE
	valid non zero client_cb_handle on success, zero on failure.
=================================================================================================*/
int thermal_server_register_client_req_handler(char *client_name, int (*callback)(int, void *, void *), void *data)
{
	int i;
	uint32_t client_cb_handle;

	if (NULL == client_name ||
	    NULL == callback) {
		msg("Thermal-Server: %s: unexpected NULL client registraion failed ", __func__);
		return 0;
	}

	/* Check for client is supported  or not*/
	for (i = 0; i < ARRAY_SIZE(req_clients); i++) {
		if (0 == strncmp(req_clients[i].name, client_name, CLIENT_NAME_MAX))
			break;
	}

	if (i >= ARRAY_SIZE(req_clients)) {
		msg("Thermal-Server: %s is not in supported thermal client list", client_name);
		return 0;
	}

	client_cb_handle = add_to_list(client_name, callback, data);
	if (client_cb_handle == 0) {
		msg("Thermal-Server: %s: Client Registration failed", __func__);
		return 0;
	}

	return (int)client_cb_handle;
}

/*========================================================================================
FUNCTION  thermal_server_unregister_client_req_handler

Function to unregister client req_handler.

ARGUMENTS
	client_cb_handle - client handle which retured on succesful registeration

RETURN VALUE
	void
========================================================================================*/
void thermal_server_unregister_client_req_handler(int client_cb_handle)
{
	if (remove_from_list((unsigned int)client_cb_handle) < 0)
		msg("Thermal-Server: thermal client unregister callback error");
}
/*========================================================================================
FUNCTION  thermal_server_release

Function to release socket thread and fd

ARGUMENTS
	none

RETURN VALUE
	void
========================================================================================*/
void thermal_server_release(void)
{
	uint8_t i;

	server_socket_exit = 1;
	pthread_join(listen_client_fd_thread, NULL);
	close(sockfd_server_send);
	close(sockfd_server_recv);
	close(sockfd_server_recv_passive);
	sockfd_server_send = -1;
	sockfd_server_recv = -1;
	sockfd_server_recv_passive = -1;

	for (i = 1; i < CLIENT_HANDLE_MAX && list_head != NULL; i++) {
		if (remove_from_list(i) < 0)
			msg("Thermal-Server: thermal client unregister callback error");
	}
}
