/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#ifndef __GPSONE_CONN_BRIDGE_H__
#define __GPSONE_CONN_BRIDGE_H__

#include "gpsone_ctrl_msg.h"
#include "gpsone_thread_helper.h"

#define MAX_BUFFER 1024
#define QMI_MAX_BUFFER 2048
struct gpsone_qmi_session_context {
    boolean ready_to_receive;
    boolean notify_data_ready_received;
    int retry_attempts;
    unsigned long sequence_number;
    unsigned long max_recv_payload_size;
    unsigned long bytes_leftover;
    char retry_buf[QMI_MAX_BUFFER];
};
struct gpsone_conn_bridge_obj {
    struct gpsone_thelper thelper;
    struct gpsone_qmi_session_context qmi_session_context;
    int fwd_rx_pipe, fwd_tx_pipe;
    int rx_pipe, tx_pipe;
    int socket_inet;
    char socket_inet_valid;
    int rx_bufsz;
    char rx_buf[MAX_BUFFER];
    unsigned long rx_cnt;
    int tx_bufsz;
    char tx_buf[MAX_BUFFER];
    unsigned long tx_cnt;
    char unblock_flag;
    int len;
    int surfix;
    char data[1];
};

void * gpsone_conn_bridge_create(struct ctrl_msgbuf *pmsg, int len);
int gpsone_conn_bridge_destroy(void * connection_bridge_handle);
unsigned int gpsone_conn_bridge_get_ip_addr(void * conn_bridge_handle);

#endif /* __GPSONE_CONN_BRIDGE_H__ */
