/*==========================================================================
Description
  This file has the constants to write BT logs into the QXDM

# Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef _BT_LOG_H
#define _BT_LOG_H

#include <sys/types.h>
#include <cutils/log.h>
#include <stdbool.h>
#include "log.h"

#define LOG_BT_ENABLE

/* Direction of logs */
#define LOG_BT_HOST_TO_SOC 0
#define LOG_BT_SOC_TO_HOST 1

/* Packet types */
#define LOG_BT_CMD_PACKET_TYPE     0x01
#define LOG_BT_ACL_PACKET_TYPE     0x02
#define LOG_BT_SCO_PACKET_TYPE     0x03
#define LOG_BT_EVT_PACKET_TYPE     0x04
#define LOG_BT_EVT_VENDOR_SPECIFIC 0xFF

/* Message type of the log from controller */
#define LOG_BT_CONTROLLER_LOG        0x01
#define LOG_BT_MESSAGE_TYPE_VSTR     0x02
#define LOG_BT_MESSAGE_TYPE_PACKET   0x05
#define LOG_BT_MESSAGE_TYPE_MEM_DUMP 0x08

/* Sub log ID for the message type PACKET */
#define LOG_BT_HCI_CMD   0
#define LOG_BT_HCI_EVENT 1

#define LOG_BT_RX_LMP_PDU      18
#define LOG_BT_TX_LMP_PDU      19
#define LOG_BT_RX_LE_CTRL_PDU  20
#define LOG_BT_TX_LE_CTRL_PDU  21
#define LOG_BT_TX_LE_CONN_MNGR 22

#define LOG_BT_LINK_MANAGER_STATE    0x80
#define LOG_BT_CONN_MANAGER_STATE    0x81
#define LOG_BT_SECURITY_STATE        0x82
#define LOG_BT_LE_CONN_MANAGER_STATE 0x83
#define LOG_BT_LE_CHANNEL_MAP_STATE  0x84
#define LOG_BT_LE_ENCRYPTION_STATE   0x85

/* Sub log ID for the message type VSTR */
#define LOG_BT_VSTR_ERROR 0
#define LOG_BT_VSTR_HIGH  1
#define LOG_BT_VSTR_LOW   2

/* QXDM ID for LMP packers */
#define LOG_BT_DIAG_LMP_LOG_ID 0x1041
#define LOG_BT_DIAG_LMP_RX_ID  0x1048
#define LOG_BT_DIAG_LMP_TX_ID  0x1049

/* To format LMP logs */
#define LOG_BT_QXDM_PKT_LENGTH_POS    0
#define LOG_BT_QXDM_PKT_LENGTH2_POS   1
#define LOG_BT_QXDM_DEVICE_IDX_POS    2
#define LOG_BT_QXDM_PKT_POS           3

#define LOG_BT_DBG_DEVICE_IDX_POS 0
#define LOG_BT_DBG_PKT_LENGTH_POS 1
#define LOG_BT_DBG_PKT_POS 2

/* Headed size of the log */
#define LOG_BT_HEADER_SIZE (sizeof(bt_log_pkt) - 1)
#define CRASH_SOURCE_FILE_PATH_LEN 50
typedef enum {
    BT_CRASH_REASON_UNKNOWN        =  0x81,
    BT_CRASH_REASON_SW_REQUESTED   =  0x82,
    BT_CRASH_REASON_STACK_OVERFLOW =  0x83,
    BT_CRASH_REASON_EXCEPTION      =  0x84,
    BT_CRASH_REASON_ASSERT         =  0x85,
    BT_CRASH_REASON_TRAP           =  0x86,
    BT_CRASH_REASON_OS_FATAL       =  0x87,
    BT_CRASH_REASON_HCI_RESET      =  0x88,
    BT_CRASH_REASON_PATCH_RESET    =  0x89,
    BT_CRASH_REASON_POWERON        =  0x90,
    BT_CRASH_REASON_WATCHDOG       =  0x91
}Reset_reason_e;

typedef struct {
    Reset_reason_e reason;
    char reasonStr[50];
}Reason_map_st;

typedef PACKED struct
{
  log_hdr_type hdr;
  byte data[1];
} bt_log_pkt;

bool diag_init();
void diag_deinit();
void send_btlog_pkt(uint8 *pBtPaktBuf, int packet_len, int direction);
int is_snoop_log_enabled();
boolean is_crashdump_enabled(void);
#endif
