/*   Copyright (c) 2012 Qualcomm Atheros, Inc.
     All Rights Reserved.
     Qualcomm Atheros Confidential and Proprietary
 */
#ifndef __GPSONE_QMI_MSG_H__
#define __GPSONE_QMI_MSG_H__

#include <arpa/inet.h>
#if defined (FEATURE_QMI) || defined (DEBUG_X86)
//Use the following for QMI Target & general off-target compilation
#include "comdef.h"
#else
#include "oncrpc.h"
#endif

#include <gpsone_bit_api.h>
#include "bearer_independent_transport_v01.h"

#define V6_ADDR_SIZE 16

enum {
    QMI_INVALID,
    QMI_BIT_OPEN,
    QMI_BIT_CLOSE,
    QMI_BIT_CONNECT,
    QMI_BIT_DISCONNECT,
    QMI_BIT_SEND,
    QMI_BIT_READY_TO_RECEIVE,
    QMI_BIT_DATA_RECEIVED_STATUS,
    QMI_BIT_SET_DORMANCY,
    QMI_BIT_GET_LOCAL_HOST_INFO,
    QMI_BIT_NOTIFY,
};

struct bit_msg_notify {
 gpsone_bit_session_handle_type session_handle;
 uint32_t transaction_id;
 gpsone_bit_event_payload_type event_payload;

};

struct qmi_msgbuf {
    size_t msgsz;
    uint8_t qmi_msg_type;
    union {
        bit_open_req_msg_v01                      qbitfwd_msg_open;
        bit_close_req_msg_v01                     qbitfwd_msg_close;
        bit_connect_req_msg_v01                   qbitfwd_msg_connect;
        bit_send_req_msg_v01                      qbitfwd_msg_send;
        bit_ready_to_receive_req_msg_v01          qbitfwd_msg_ready_to_receive;
        bit_data_received_status_req_msg_v01      qbitfwd_msg_data_received_status;
        bit_get_local_host_info_req_msg_v01       qbitfwd_msg_get_local_host_info;
        bit_disconnect_req_msg_v01                qbitfwd_msg_disconnect;
        bit_set_dormancy_req_msg_v01              qbitfwd_msg_set_dormancy;
        struct bit_msg_notify                     qbitfwd_msg_notify;
    } qmsg;
};

#endif /* __GPSONE_QMI_MSG_H__ */
