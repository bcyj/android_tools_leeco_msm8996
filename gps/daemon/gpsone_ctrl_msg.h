/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#ifndef __GPSONE_CTRL_MSG_H__
#define __GPSONE_CTRL_MSG_H__

#include <arpa/inet.h>

#if defined (FEATURE_QMI) || defined (DEBUG_X86)
//Use the following for QMI Target & general off-target compilation
#include "comdef.h"
#else
#include "oncrpc.h"
#endif

#include <gpsone_bit_api.h>

#define V6_ADDR_SIZE 16

enum {
    GPSONE_INVALID,
    GPSONE_BIT_OPEN,
    GPSONE_BIT_CLOSE,
    GPSONE_BIT_CONNECT,
    GPSONE_BIT_DISCONNECT,
    GPSONE_BIT_SEND,
    GPSONE_BIT_RECEIVE,
    GPSONE_BIT_FORCE_DORMANCY,
    GPSONE_BIT_UNFORCE_DORMANCY,
    GPSONE_BIT_GET_LOCAL_IP_ADDR,

    /* Daemon internal control messages
       has to be same as defined in loc_eng_data_server_handler.h */
    GPSONE_LOC_API_IF_REQUEST   = 0xF0,
    GPSONE_LOC_API_IF_RELEASE,
    GPSONE_LOC_API_RESPONSE,
    GPSONE_UNBLOCK,
    GPSONE_FORCE_DORMANCY_STATUS,
};

struct ctrl_msg_open {
    unsigned char reserved;
};

struct ctrl_msg_close {
    unsigned char reserved;
};

struct ctrl_msg_connect {
    unsigned is_supl; /* 1: use Android SUPL connection; 0: use Android default internet connection */
    unsigned is_udp;
    uint16 ip_port;
    gpsone_bit_ip_addr_type ip_addr;
};

struct ctrl_msg_disconnect {
    int reserved;
};

struct ctrl_msg_ioctl {
    int reserved;
};

/* Here below are daemon internal control messages
   has to be same as defined in loc_eng_data_server_handler.h */

enum {
    GPSONE_LOC_API_IF_REQUEST_SUCCESS = 0xF0,
    GPSONE_LOC_API_IF_RELEASE_SUCCESS,
    GPSONE_LOC_API_IF_FAILURE,
};

struct ctrl_msg_response {
    int result;
};

typedef enum {
  IF_REQUEST_TYPE_SUPL = 0,
  IF_REQUEST_TYPE_WIFI,
  IF_REQUEST_TYPE_ANY
} ctrl_if_req_type_e_type;

typedef enum {
  IF_REQUEST_SENDER_ID_QUIPC = 0,
  IF_REQUEST_SENDER_ID_MSAPM,
  IF_REQUEST_SENDER_ID_MSAPU,
  IF_REQUEST_SENDER_ID_GPSONE_DAEMON,
  IF_REQUEST_SENDER_ID_MODEM
} ctrl_if_req_sender_id_e_type;

#define SSID_BUF_SIZE (32+1)
struct ctrl_msg_if_request {
    ctrl_if_req_type_e_type type;
    ctrl_if_req_sender_id_e_type sender_id;
    unsigned long ipv4_addr;
    unsigned char ipv6_addr[16];
    char ssid[SSID_BUF_SIZE];
    char password[SSID_BUF_SIZE];
};

struct ctrl_msg_unblock {
    int reserved;
};

struct ctrl_msgbuf {
    size_t msgsz;
    gpsone_bit_session_handle_type session_handle;
    uint32_t transaction_id;
    uint8_t ctrl_type;
    union {
        struct ctrl_msg_open       cmsg_open;
        struct ctrl_msg_close      cmsg_close;
        struct ctrl_msg_connect    cmsg_connect;
        struct ctrl_msg_disconnect cmsg_disconnect;
        struct ctrl_msg_ioctl      cmsg_ioctl;
        struct ctrl_msg_response   cmsg_response;
        struct ctrl_msg_unblock    cmsg_unblock;
        struct ctrl_msg_if_request cmsg_if_request;
    } cmsg;
};

#endif /* __GPSONE_CTRL_MSG_H__ */
