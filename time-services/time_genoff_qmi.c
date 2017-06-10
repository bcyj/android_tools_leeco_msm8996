/*
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2011, 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "localdefs.h"
#include "time_genoff_i.h"

/*
 * The library works as below.
 * 1. Application calls time_genoff_operation() with the required arguments.
 * 2. Library opens socket with the time_daemon.
 * 3. Sends the get/set data to the daemon who does the processing.
 * 4. The library receives data from the daemon.
 * 5. Closes socket and replies back to the application.
 */

int time_genoff_operation(time_genoff_info_type *pargs)
{
	int sock_id, length;
	struct sockaddr_un time_socket;
	struct send_recv_struct to_send;
	struct tm *time_julian, intermediate_result;
	struct timeval timeout;

	if (!pargs) {
		TIME_LOGE("Lib:%s: Invalid input to library\n", __func__);
		return -EINVAL;
	}

	if (pargs->operation == T_GET) {
		TIME_LOGV("Lib:%s: pargs->base = %d\n", __func__, pargs->base);
		TIME_LOGV("Lib:%s: pargs->operation = %d\n", __func__,pargs->operation);
	} else {
		TIME_LOGD("Lib:%s: pargs->base = %d\n", __func__, pargs->base);
		TIME_LOGD("Lib:%s: pargs->operation = %d\n", __func__,pargs->operation);
	}

	/* Validity check */
	if ( (pargs->base >= ATS_MAX) ||
		!(pargs->unit > TIME_STAMP && pargs->unit < TIME_20MS_FRAME) ||
		(pargs->operation >= T_MAX) ||
		(pargs->ts_val == NULL)) {
		TIME_LOGE("Lib:%s: Invalid input arguments\n", __func__);
		return -EINVAL;
	}
	if (pargs->operation == T_GET) {
		TIME_LOGV("Lib:%s: pargs->ts_val = %llu\n", __func__,
						*(uint64_t *)(pargs->ts_val));
	} else {
		TIME_LOGD("Lib:%s: pargs->ts_val = %llu\n", __func__,
						*(uint64_t *)(pargs->ts_val));
	}
	sock_id = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_id < 0) {
		TIME_LOGE("Lib:%s: Unable to create socket\n", __func__);
		return -EINVAL;
	}

	/* Set socket recv/send timeout to 10 sec */
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt(sock_id, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
							sizeof(timeout));
	setsockopt(sock_id, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
							sizeof(timeout));
	time_socket.sun_family = AF_UNIX;
	strlcpy(time_socket.sun_path, GENOFF_SOCKET_NAME, UNIX_PATH_MAX);
	/* Create abstract domain socket */
	time_socket.sun_path[0] = 0;
	length = strlen(GENOFF_SOCKET_NAME) + sizeof(time_socket.sun_family);
	if (connect(sock_id, (struct sockaddr *)&time_socket, length) == -1) {
		TIME_LOGE("Lib:%s: Connection failed !!\n", __func__);
		goto error_close_socket;
	}

	/*
	 * Format to send : base, unit, operation, value (for set)
	 * Time sent/received in msecs. Later converted to required format.
	 */
	if (pargs->operation == T_GET) {
		to_send.base = pargs->base;
		to_send.unit = TIME_MSEC;
		to_send.operation = T_GET;
		to_send.value = 0;
		to_send.result = -1;

		if (send(sock_id, &to_send, sizeof(to_send), 0) < 0) {
			TIME_LOGE("Lib:%s: Send to server failed !!\n", __func__);
			goto error_close_socket;
		} else {
			TIME_LOGV("Lib:%s: Send to server  passed pid %d\n",
					__func__, getpid());
			if (recv(sock_id, (void *)&to_send, sizeof(to_send), 0) < 0) {
				TIME_LOGE("Lib:%s: Unable to recv data\n",__func__);
				goto error_close_socket;
			}
			TIME_LOGV("Lib:Receive Passed == base = %d, unit = %d,"
				"operation = %d, value = %llu, result = %d\n", to_send.base,
				to_send.unit, to_send.operation, to_send.value, to_send.result);
			if (to_send.result != 0) {
				TIME_LOGE("Lib: Time Get for Base = %d failed\n",
								to_send.base);
				goto error_close_socket;
			}
		}
		close(sock_id);

		switch(pargs->unit) {
		case TIME_MSEC:
			*(uint64_t *)pargs->ts_val = to_send.value;
			break;
		case TIME_SECS:
			/* convert the time to seconds */
			to_send.value = MSEC_TO_SEC(to_send.value);
			*(uint64_t *)pargs->ts_val = to_send.value;
			break;
		case TIME_JULIAN:
			/* convert the time to julian format */
			to_send.value = MSEC_TO_SEC(to_send.value);
			time_julian = gmtime_r((time_t *)&to_send.value,
					&intermediate_result);
			if (!time_julian) {
				TIME_LOGE("Lib:%s: Invalid time %llu",
						__func__, to_send.value);
				goto error_close_socket;
			}

			memcpy(pargs->ts_val, time_julian, sizeof(struct tm));
			break;
		default:
		        TIME_LOGE("Lib:%s:Invalid time unit %d", __func__, pargs->unit);
			goto error_close_socket;
		}
	} else if (pargs->operation == T_SET) {
		switch(pargs->unit) {
		case TIME_MSEC:
			to_send.value = *(uint64_t *)pargs->ts_val;
			break;
		case TIME_SECS:
			/* Convert the time to msecs */
			to_send.value = *(uint64_t *)pargs->ts_val;
			to_send.value = SEC_TO_MSEC(to_send.value);
			break;
		case TIME_JULIAN:
			/* Convert the time to UTC and then msecs */
			to_send.value= mktime((struct tm *)pargs->ts_val);
			to_send.value = SEC_TO_MSEC(to_send.value);
		        break;
		default:
		        TIME_LOGE("Lib:%s:Invalid time unit %d", __func__, pargs->unit);
			goto error_close_socket;
		}
		to_send.base = pargs->base;
		to_send.unit = TIME_MSEC;
		to_send.operation = T_SET;
		to_send.result = -1;

		if (send(sock_id, &to_send, sizeof(to_send), 0) < 0) {
			TIME_LOGE("Lib:%s: Send to server failed !!\n", __func__);
			goto error_close_socket;
		} else {
			TIME_LOGD("Lib:%s: Send to server  passed!!\n", __func__);
			if (recv(sock_id, (void *)&to_send, sizeof(to_send), 0) < 0) {
				TIME_LOGE("Lib:%s: Unable to recv data\n",__func__);
				goto error_close_socket;
			}
			TIME_LOGE("Receive Passed == base = %d, unit = %d, operation = %d, result = %d\n",
			to_send.base, to_send.unit, to_send.operation, to_send.result);
			if (to_send.result != 0) {
				TIME_LOGE("Lib: Time Set for Base = %d failed\n",
								to_send.base);
				goto error_close_socket;
			}
		}
		close(sock_id);
	} else if (pargs->operation == T_IS_SET) {
		to_send.base = pargs->base;
		to_send.unit = TIME_MSEC;
		to_send.operation = T_IS_SET;
		to_send.result = -1;
		to_send.value = 0;

		if (send(sock_id, &to_send, sizeof(to_send), 0) < 0) {
			TIME_LOGE("Lib:%s: Send to server failed !!\n", __func__);
			goto error_close_socket;
		} else {
			TIME_LOGD("Lib:%s: Send to server  passed!!\n", __func__);
			if (recv(sock_id, (void *)&to_send, sizeof(to_send), 0) < 0) {
				TIME_LOGE("Lib:%s: Unable to recv data\n",__func__);
				goto error_close_socket;
			}
			TIME_LOGE("Receive Passed == base = %d, unit = %d, operation = %d, result = %d\n",
			to_send.base, to_send.unit, to_send.operation, to_send.result);
			*(uint64_t *)pargs->ts_val = to_send.value;
		}
		close(sock_id);
	} else {
		TIME_LOGE("Lib:%s: Invalid operation specified\n", __func__);
		goto error_close_socket;
	}

	return 0;

error_close_socket:
	close(sock_id);
	return -EINVAL;
}

int time_control_operations(time_genoff_info_type *pargs)
{
	int sock_id, length;
	struct sockaddr_un time_socket;
	struct timeval timeout;
	struct send_recv_struct to_send;

	sock_id = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_id < 0) {
		TIME_LOGE("Lib:%s: Unable to create socket\n", __func__);
		return -EINVAL;
	}

	/* Set socket recv/send timeout to 10 sec */
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt(sock_id, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
							sizeof(timeout));
	setsockopt(sock_id, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
							sizeof(timeout));
	time_socket.sun_family = AF_UNIX;
	strlcpy(time_socket.sun_path, GENOFF_SOCKET_NAME, UNIX_PATH_MAX);
	/* Create abstract domain socket */
	time_socket.sun_path[0] = 0;
	length = strlen(GENOFF_SOCKET_NAME) + sizeof(time_socket.sun_family);
	if (connect(sock_id, (struct sockaddr *)&time_socket, length) == -1) {
		TIME_LOGE("Lib:%s: Connection failed !!\n", __func__);
		goto error_close_socket;
	}

	if(pargs->operation == T_DISABLE) {
		to_send.base = pargs->base;
		to_send.unit = TIME_MSEC;
		to_send.operation = T_DISABLE;
		to_send.result = -1;

		if(send(sock_id, &to_send, sizeof(to_send), 0) < 0) {
			TIME_LOGE("Lib:%s: send to server failed !!\n", __func__);
			goto error_close_socket;
		} else {
			TIME_LOGD("Lib:%s: Send to server  passed!!\n", __func__);
			if (recv(sock_id, (void *)&to_send, sizeof(to_send), 0) < 0) {
				TIME_LOGE("Lib:%s: Unable to recv data\n",__func__);
				goto error_close_socket;
			}
			TIME_LOGE("Disable Passed == base = %d, unit = %d, operation = %d, result = %d\n",
				to_send.base, to_send.unit, to_send.operation, to_send.result);
			*(uint64_t *)pargs->ts_val = to_send.value;
		}
		close(sock_id);
	} else if (pargs->operation == T_ENABLE) {
		to_send.base = pargs->base;
		to_send.unit = TIME_MSEC;
		to_send.operation = T_ENABLE;
		to_send.result = -1;

		if(send(sock_id, &to_send, sizeof(to_send), 0) < 0) {
			TIME_LOGE("Lib:%s: send to server failed !!\n", __func__);
			goto error_close_socket;
		} else {
			TIME_LOGD("Lib:%s: Send to server  passed!!\n", __func__);
			if (recv(sock_id, (void *)&to_send, sizeof(to_send), 0) < 0) {
				TIME_LOGE("Lib:%s: Unable to recv data\n",__func__);
				goto error_close_socket;
			}
			TIME_LOGE("Enable Passed == base = %d, unit = %d, operation = %d, result = %d\n",
				to_send.base, to_send.unit, to_send.operation, to_send.result);
			*(uint64_t *)pargs->ts_val = to_send.value;
		}
		close(sock_id);
	} else {
		TIME_LOGE("Lib:%s: Invalid operation specified\n", __func__);
		goto error_close_socket;
	}

	return 0;

error_close_socket:
	close(sock_id);
	return -EINVAL;
}
