/******************************************************************************

  @file    sap_constants.h
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef __SAP_CONSTANTS_H__
#define __SAP_CONSTANTS_H__

#define SAP_HEADER_SIZE         (4)

#define SAP_MAX_IPC_MSG_LEN     (SAP_IPC_HEADER_SIZE + SAP_MAX_MSG_LEN)
#define SAP_IPC_HEADER_SIZE     (3)
#define SAP_IPC_CTRL_MSG_SIZE   (1)

#define SAP_PARAM_HEADER_SIZE   (4)

#define SAP_MAX_MSG_LEN         (32764)

#define FOUR_BYTE_PADDING(x)    (((x)%4) ? (4-((x)%4)) : (0))
#define BTADDR_SIZE             (18)

#define LOG_TAG "BT_SAP"

/*30 secs as the disconnect time out value*/
#define  DISCONNECT_TIMEOUT     (30)
/*Server side events*/
#define CARD_STATUS_ERROR      (0xFD)
#define CARD_STATUS_REMOVED    (0xFE)
#define  DISCONNECT_GRACEFULLY  (0xFF)
#define SAP_SERVER "qcom.sap.server"

#ifdef __cplusplus
extern "C" {
#endif

int disconnect_sap(int);
int send_card_status(int);

#ifdef __cplusplus
}
#endif
/*SAP states*/
typedef enum {
        DISCONNECTED = 0x00,
        DISCONNECTING,
        CONNECTING,
        CONNECTED_READY,
        PROCESSING_APDU,
        PROCESSING_ATR,
        PROCESSING_SIM_OFF,
        PROCESSING_SIM_ON,
        PROCESSING_SIM_RESET,
        PROCESSING_TX_CARD_RDR_STATUS,
} sap_state_type;

/* SAP internal errors*/
typedef enum
{
        SAP_ERR_NONE        = -1000,
        SAP_ERR_ILLEGAL_ARG = -1001,
        SAP_ERR_SND_FAIL    = -1002,
        SAP_ERR_RCV_FAIL    = -1003,
        SAP_ERR_QMI         = -1004,
        SAP_ERR_MEM_LOW     = -1005,
        SAP_ERR_UNDEFINED   = -1006,
}sap_internal_error;

/*SAP message ids*/
typedef enum
{
        CONNECT_REQ = 0x00,
        CONNECT_RESP,
        DISCONNECT_REQ,
        DISCONNECT_RESP,
        DISCONNECT_IND,
        TRANSFER_APDU_REQ,
        TRANSFER_APDU_RESP,
        TRANSFER_ATR_REQ,
        TRANSFER_ATR_RESP,
        POWER_SIM_OFF_REQ,
        POWER_SIM_OFF_RESP,
        POWER_SIM_ON_REQ,
        POWER_SIM_ON_RESP,
        RESET_SIM_REQ,
        RESET_SIM_RESP,
        TRANSFER_CARD_READER_STATUS_REQ,
        TRANSFER_CARD_READER_STATUS_RESP,
        STATUS_IND,
        ERROR_RESP,
} sap_msg_id;

/*SAP param ids*/
typedef enum
{
        MAX_MSG_SIZE = 0x00,
        CONNECTION_STATUS,
        RESULT_CODE,
        DISCONNECTION_TYPE,
        COMMAND_APDU,
        COMMAND_APDU_7816 = 0x10,
        RESPONSE_APDU = 0x05,
        ATR = 0x06,
        CARD_READER_STATUS = 0x07,
        STATUS_CHANGE = 0x08,
        TRANSPORT_PROTOCOL = 0x09
} sap_param_id;

/*
 *      Sap Parameter values
 */
typedef enum
{
        CONN_OK = 0x00,
        CONN_ERR,
        CONN_MAX_SIZE_NOT_SUPPORTED,
        CONN_MAX_SIZE_TOO_SMALL,
        CONN_ONGOING_CALL
}sap_connection_status;

typedef enum
{
        GRACEFUL_DISCONN = 0x00,
        IMMEDIATE_DISCONN
}sap_disconnect_type;

typedef enum
{
        ERR_NONE = 0x00,
        ERR_UNDEFINED,
        ERR_CARD_NOT_ACCESSIBLE,
        ERR_CARD_POWERED_OFF,
        ERR_CARD_REMOVED,
        ERR_CARD_ALREADY_ON,
        ERR_DATA_UNAVAILABLE,
        ERR_NOT_SUPPORTED,
} sap_result_code;


typedef enum
{
        UNKNOWN_ERROR = 0x00,
        CARD_RESET,
        CARD_NOT_ACCESSIBLE,
        CARD_REMOVED,
        CARD_INSERTED,
        CARD_RECOVERED,
        /*This status indicated the SIM is busy
	 with on on-going call, this is used for internal purpose*/
        CARD_BUSY,
} sap_status_change;

typedef enum
{
        SIM_CARD_NOT_ACCESSIBLE,
        SIM_CARD_REMOVED,
        SIM_CARD_POWERED_ON,
        SIM_CARD_POWERED_OFF,
} sap_sim_card_state;

typedef enum
{
        SAP_IPC_MSG_SAP_REQUEST = 0x00,
        SAP_IPC_MSG_SAP_RESPONSE,
        SAP_IPC_MSG_CTRL_REQUEST,
        SAP_IPC_MSG_CTRL_RESPONSE
} sap_ipc_msg_type;

typedef enum
{
        SAP_CRTL_MSG_DISCONNECT_REQ = 0x00,
        SAP_CRTL_MSG_DISCONNECT_REQ_IMM,
        SAP_CRTL_MSG_CONNECTED_RESP,
        SAP_CRTL_MSG_DISCONNECTED_RESP,
} sap_ipc_ctrl_msg_type;

extern int conn_sk;
extern const int sap_param_count[];
extern const int sap_param_length[];
extern sap_state_type sap_state;
extern sap_sim_card_state sim_card_state;

#endif /*__SAP_CONSTATNS_H__*/
