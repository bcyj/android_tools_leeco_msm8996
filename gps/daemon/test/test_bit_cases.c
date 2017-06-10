/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "gpsone_conn_client.h"
#include "test_script_parser.h"
#include "gpsone_bit_forward.h"

#include "gpsone_bit_api.h"
#include "gpsone_bit_local_api.h"

#include "gpsone_daemon_dbg.h"


#include "gpsone_udp_modem.h"


#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "test_dbg.h"


static char is_notified = 0;
pthread_cond_t  notify_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t notify_mutex = PTHREAD_MUTEX_INITIALIZER;

static char is_data_ready_notified = 0;
pthread_cond_t  is_data_ready_notify_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t is_data_ready_notify_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint32 transaction_count = 0;

#define MAX_SESSION 10
struct session_info {
    gpsone_bit_session_handle_type     session_handle;
    uint32                             transaction_id;
};

static struct session_info session_tbl[MAX_SESSION];

void init_session_handle()
{
    int i;
    for (i = 0; i < MAX_SESSION; i++) {
        session_tbl[i].session_handle = -1;
        session_tbl[i].transaction_id = -1;
    }
}

void print_table()
{
    int i;
    for (i = 0; i < MAX_SESSION; i++) {
        TEST_DBG("session_tbl[%d].{session_handle=0x%x, transaction_id=0x%x}\n",i,session_tbl[i].session_handle, session_tbl[i].transaction_id);
    }
}

gpsone_bit_session_handle_type get_session_handle(char *p_args)
{
        int i;
        char * conn_index = (char *) "0";
        gpsone_bit_session_handle_type session_handle = GPSONE_BIT_INVALID_SESSION_HANDLE;

        if (p_args != NULL) {
                conn_index = strtok(p_args, " ");
        }

        i = atoi(conn_index);
        if (i > MAX_SESSION - 1) {
            TEST_DBG("exceed max connections\n");
        } else {
            session_handle = session_tbl[i].session_handle;
        }

        return session_handle;
}

void set_transaction_id(char *p_args, uint32 transaction_id)
{
        int i;
        char * conn_index = (char *) "0";

        if (p_args != NULL) {
                conn_index = p_args;
        }

        i = atoi(conn_index);
        if (i > MAX_SESSION - 1) {
            TEST_DBG("exceed max connections\n");
        } else {
            session_tbl[i].transaction_id = transaction_id;
        }
}

int set_session_handle(uint32 transaction_id, gpsone_bit_session_handle_type session_handle)
{
    int i;
    for (i = 0; i < MAX_SESSION; i++) {
        if (session_tbl[i].transaction_id == transaction_id) {
            session_tbl[i].session_handle = session_handle;
            TEST_DBG("%s:%d] i = %d, transaction_id = %d, session_handle = %d\n", __func__, __LINE__, i, transaction_id, (int)session_handle);
            return i;
        }
    }
    return -1;
}

int del_session_handle(gpsone_bit_session_handle_type session_handle)
{
    int i;
    for (i = 0; i < MAX_SESSION; i++) {
        if (session_tbl[i].session_handle == session_handle) {
            session_tbl[i].session_handle = -1;
            TEST_DBG("%s:%d] i = %d, transaction_id = %d, session_handle = %d\n", __func__, __LINE__, i, session_tbl[i].transaction_id, (int)session_handle);
            return i;
        }
    }
    return -1;
}

gpsone_bit_status_e_type gpsone_bit_post_event
(
  gpsone_bit_session_handle_type         bit_session_handle,
  uint32                                 transaction_id,
  const gpsone_bit_event_payload_type    *p_event_payload
)
{
    switch(p_event_payload->event)
    {
    case GPSONE_BIT_EVENT_OPEN_RESULT:
        TEST_DBG("%s:%d] POST OPEN\n", __func__, __LINE__);
        gpsone_bit_process_event((gpsone_bit_event_payload_type    *)p_event_payload);

        pthread_mutex_lock(&notify_mutex);
        is_notified = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
        break;

    case GPSONE_BIT_EVENT_CLOSE_RESULT:
        TEST_DBG("%s:%d] POST CLOSE\n", __func__, __LINE__);
        gpsone_bit_process_event((gpsone_bit_event_payload_type    *)p_event_payload);

        pthread_mutex_lock(&notify_mutex);
        is_notified = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
        break;

    case GPSONE_BIT_EVENT_CONNECT_RESULT:    /* event to report result for connect */
        set_session_handle(transaction_id, bit_session_handle);
        TEST_DBG("%s:%d] POST CONNECT\n", __func__, __LINE__);

        TEST_DBG("%s in connect result transaction id is %d\n", __func__, transaction_id);

        pthread_mutex_lock(&notify_mutex);
        is_notified = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
        break;

    case GPSONE_BIT_EVENT_DISCONNECT_RESULT: /* event to report result for disconnect */
        del_session_handle(bit_session_handle);
        TEST_DBG("%s:%d] POST DISCONNECT\n", __func__, __LINE__);

        TEST_DBG("%s in connect result transaction id is %d\n", __func__, transaction_id);

        pthread_mutex_lock(&notify_mutex);
        is_notified = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
        break;

    case GPSONE_BIT_EVENT_SEND_RESULT:       /* event to report result for send */
        TEST_DBG("%s:%d] POST SEND\n", __func__, __LINE__);

        break;


    case GPSONE_BIT_EVENT_DATA_READY:        /* event to notify PDSM of new data ready */
        TEST_DBG("%s:%d] POST DATA\n", __func__, __LINE__);

        pthread_mutex_lock(&is_data_ready_notify_mutex);
        is_data_ready_notified = 1;
        pthread_cond_signal(&is_data_ready_notify_cond);
        pthread_mutex_unlock(&is_data_ready_notify_mutex);
        break;

    case GPSONE_BIT_EVENT_NETWORK_STATUS:    /* event to notify PDSM of network status change */
        TEST_DBG("%s:%d] POST NETWORK\n", __func__, __LINE__);

        pthread_mutex_lock(&notify_mutex);
        is_notified = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
        break;

    case GPSONE_BIT_EVENT_IOCTL_RESULT:      /* event to report result for ioctl */
    {
        struct in_addr ip_addr;

        ip_addr.s_addr = p_event_payload->arg.ipaddr.addr.v4_addr;

        TEST_DBG("%s:%d] IP Address %s\n", __func__, __LINE__, inet_ntoa(ip_addr));

        pthread_mutex_lock(&notify_mutex);
        is_notified = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
        break;
    }

    default:
        TEST_DBG("%s:%d] POST default\n", __func__, __LINE__);
        break;
    }

    /* @todo save session_handle */
    TEST_DBG("%s:%d] POST DONE %d\n", __func__, __LINE__, is_notified);

    return GPSONE_BIT_STATUS_SUCCESS;
}


gpsone_bit_status_e_type test_gpsone_open(char *p_args, char *p_result)
{
    gpsone_bit_status_e_type status;
    gpsone_bit_open_params_type open_param;

    open_param.force_connection_up = FALSE;

    init_session_handle();
    TEST_DBG("%s:%d] open\n", __func__, __LINE__);
    status = gpsone_bit_open(&open_param);
    TEST_DBG("%s:%d] open ongoing...\n", __func__, __LINE__);
    return status;
}

gpsone_bit_status_e_type test_gpsone_close(char *p_args, char *p_result)
{
    gpsone_bit_status_e_type status;
    gpsone_bit_close_params_type close_param;

    close_param.modem_restarting = FALSE;
    close_param.force_dormancy = FALSE;

    TEST_DBG("%s:%d] close\n", __func__, __LINE__);
    status = gpsone_bit_close(&close_param);
    TEST_DBG("%s:%d] close ongoing...\n", __func__, __LINE__);
    return status;
}

gpsone_bit_status_e_type test_gpsone_connect(char *p_args, char *p_result)
{
    uint32 transaction_id = transaction_count ++;
    int is_udp;
    int is_host_name;
    char * conn_index = (char *) "0";
    char * server_ip_host_name = (char *) "127.0.0.1";
    char * server_port = (char *) "9009";


    gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_FAIL;
    gpsone_bit_connect_params_type connect_param;

    TEST_DBG("%s:%d]-%s-: %s\n", __func__, __LINE__, p_args, p_result);
    if (p_args != NULL) {
        is_udp = atoi(strtok(p_args, " "));
        is_host_name = atoi(strtok(NULL, " "));
        server_ip_host_name = strtok(NULL, " ");
        server_port = strtok(NULL, " ");
    TEST_DBG("%s %d %s %s\n", conn_index, is_host_name, server_ip_host_name, server_port);
    }

    set_transaction_id(conn_index, transaction_id);

    connect_param.protocol_type = GPSONE_BIT_PROTOCOL_TYPE_SUPL;
    connect_param.ip_port = atoi(server_port);
    if (is_host_name) {
      strncpy(connect_param.host_name,
             server_ip_host_name,
             GPSONE_BIT_MAX_URL_STRING_LEN);
      connect_param.adr_type = GPSONE_BIT_HOST_NAME;
      TEST_DBG("%s:%d] connect to host_name: %s port: %s\n", __func__, __LINE__, server_ip_host_name, server_port);
    } else {
      connect_param.ip_addr.addr.v4_addr = inet_addr(server_ip_host_name);
      connect_param.ip_addr.type = GPSONE_BIT_IP_V4;
      connect_param.adr_type = GPSONE_BIT_IP_ADDR;
      TEST_DBG("%s connecting to server ip %s:%s...\n", __func__,
          server_ip_host_name, server_port);
    }

    if(is_udp) {
      connect_param.link_type = GPSONE_BIT_LINK_UDP;
    } else {
      connect_param.link_type = GPSONE_BIT_LINK_TCP;
    }


    status = gpsone_bit_connect(transaction_id, &connect_param);
    if (status != GPSONE_BIT_STATUS_SUCCESS) {
        TEST_DBG("connection waiting\n");
    }
    TEST_DBG("%s:%d] connecting ongoing...\n", __func__, __LINE__);

    return status;
}

gpsone_bit_status_e_type test_gpsone_disconnect(char *p_args, char *p_result)
{
    uint32 transaction_id = transaction_count ++;
    gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_FAIL;
    gpsone_bit_disconnect_params_type disconnect_param;
    gpsone_bit_session_handle_type session_handle = get_session_handle(p_args);

    TEST_DBG("%s:%d] begin test_gpsone_disconnect\n", __func__, __LINE__);
    if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE) {
      TEST_DBG("%s:%d] invalid session handle 0x%x\n", __func__, __LINE__, session_handle);
      //print_table();
        return status;
    }

    disconnect_param.force_disconnect = FALSE;
    status = gpsone_bit_disconnect(session_handle, transaction_id, &disconnect_param);
    TEST_DBG("%s:%d] disconnecting ongoing...\n", __func__, __LINE__);

    return status;
}

gpsone_bit_status_e_type test_gpsone_send(char *p_args, char *p_result)
{
    uint32 transaction_id = transaction_count ++;
    gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_FAIL;
    int size;
    char * conn_index;
    char * p_cmd;

    conn_index = strtok_r(p_args, " ", &p_cmd);

    gpsone_bit_session_handle_type session_handle = get_session_handle(p_args);

    if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE)
    {
        return status;
    }

    size = strlen(p_cmd) + 1;
    TEST_DBG("%s:%d] sending: {%s} - %d.\n", __func__, __LINE__, p_cmd, size);
    status = gpsone_bit_send(session_handle, transaction_id, (uint8_t *) p_cmd, size);
    TEST_DBG("%s:%d] sending ongoing...\n", __func__, __LINE__);
    return status;
}

gpsone_bit_status_e_type test_gpsone_receive(char *p_args, char *p_result)
{
    TEST_DBG("%s:%d] \n", __func__, __LINE__);
    gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_FAIL;
    uint32                             bytes_returned;
    uint32                             bytes_leftover;
    char recv_buf[2048] = {'\0'};
    gpsone_bit_session_handle_type session_handle = get_session_handle(p_args);

    if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE) {
        return status;
    }

    TEST_DBG("%s:%d] receiving\n", __func__, __LINE__);

    do {
    status = gpsone_bit_recv(session_handle, (uint8_t *) recv_buf, 2048, &bytes_returned, &bytes_leftover);
        TEST_DBG("%s:%d] bytes_returned = %d, bytes_leftover = %d, %s\n", __func__, __LINE__,
            (int) bytes_returned, (int) bytes_leftover, recv_buf);
    } while (bytes_returned == 0);

    return status;
}

gpsone_bit_status_e_type test_gpsone_ioctl(char *p_args, char *p_result)
{
    uint32 transaction_id = transaction_count ++;
    gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_FAIL;
    gpsone_bit_ioctl_e_type            ioctl_request;
    gpsone_bit_ioctl_params_type       ioctl_param;
    gpsone_bit_session_handle_type session_handle = get_session_handle(p_args);

    if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE) {
        return status;
    }

    ioctl_request = (gpsone_bit_ioctl_e_type) atoi(p_args);

    TEST_DBG("%s:%d] ioctl\n", __func__, __LINE__);
    status = gpsone_bit_ioctl(session_handle, transaction_id, ioctl_request, &ioctl_param);
    TEST_DBG("%s:%d] ioctl ongoing...\n", __func__, __LINE__);

    return status;
}

int test_bit_cases(int command_id, char *p_args, char *p_result)
{
    gpsone_bit_status_e_type result = GPSONE_BIT_STATUS_FAIL;
    TEST_DBG("%s:%d] command_id = %d\n", __func__, __LINE__, command_id);
    TEST_DBG("%s:%d] STEPHENL: p_args: %s\n", __func__, __LINE__, p_args);
    switch(command_id)
    {
        case TS_BIT_REGISTER:
             /* @todo wait */
            break;

        case TS_BIT_DEREGISTER:
            TEST_DBG("         unimplemented command: %d\n", command_id);
            result = GPSONE_BIT_STATUS_SUCCESS;
            break;

        case TS_BIT_WAIT_NOTIFY:
            TEST_DBG("<<<<<<<<<<<<<< Waiting for Notify %s\n", p_args);
            pthread_mutex_lock(& notify_mutex);
            while (is_notified == 0) {
                pthread_cond_wait( &notify_cond, & notify_mutex);
            }
            pthread_mutex_unlock(& notify_mutex);
            TEST_DBG(">>>>>>>>>>>>>> Received Notify %s\n", p_args);
            result = GPSONE_BIT_STATUS_SUCCESS;
            break;

        case TS_BIT_DATA_READY_WAIT_NOTIFY:
            TEST_DBG("<<<<<<<<<<<<<< Waiting for Data Ready Notify %s\n", p_args);
            pthread_mutex_lock(& is_data_ready_notify_mutex);
            while (is_data_ready_notified == 0) {
                pthread_cond_wait( &is_data_ready_notify_cond, & is_data_ready_notify_mutex);
            }
            is_data_ready_notified = 0;
            pthread_mutex_unlock(& is_data_ready_notify_mutex);
            TEST_DBG(">>>>>>>>>>>>>> Received Data Ready Notify %s\n", p_args);
            result = GPSONE_BIT_STATUS_SUCCESS;
            break;

        case TS_BIT_OPEN:
            pthread_mutex_lock(&notify_mutex);
            is_notified = 0;
            pthread_mutex_unlock(&notify_mutex);
            result = test_gpsone_open(p_args, p_result);
            break;

        case TS_BIT_CLOSE:
            pthread_mutex_lock(&notify_mutex);
            is_notified = 0;
            pthread_mutex_unlock(&notify_mutex);
            result = test_gpsone_close(p_args, p_result);
            break;

        case TS_BIT_CONNECT:
            pthread_mutex_lock(&notify_mutex);
            is_notified = 0;
            pthread_mutex_unlock(&notify_mutex);
            result = test_gpsone_connect(p_args, p_result);
            break;

        case TS_BIT_DISCONNECT:
            TEST_DBG("before disconnect mutex lock%s\n", p_args);
            pthread_mutex_lock(&notify_mutex);
            TEST_DBG("after disconnect mutex lock %s\n", p_args);
            is_notified = 0;
            pthread_mutex_unlock(&notify_mutex);
            TEST_DBG("after disconnect mutex unlock%s\n", p_args);
            result = test_gpsone_disconnect(p_args, p_result);
            break;

        case TS_BIT_SEND:
            pthread_mutex_lock(&notify_mutex);
            is_notified = 0;
            pthread_mutex_unlock(&notify_mutex);
            result = test_gpsone_send(p_args, p_result);
            break;

        case TS_BIT_RECEIVE:
            TEST_DBG("TS_BIT_RECEIVE case\n");
            result = test_gpsone_receive(p_args, p_result);
            break;

        case TS_BIT_IOCTL:
            pthread_mutex_lock(&notify_mutex);
            is_notified = 0;
            pthread_mutex_unlock(&notify_mutex);
            result = test_gpsone_ioctl(p_args, p_result);
            break;

        case TS_CLIENT_EXIT:
            result = GPSONE_BIT_STATUS_SUCCESS;
            break;

        default:
            TEST_DBG("         unsupported command: %d\n", command_id);
            break;
    }

    TEST_DBG("         result: %d, notify = %d\n", result, is_notified);
    return result;
}

