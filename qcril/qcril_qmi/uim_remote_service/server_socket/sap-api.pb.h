/******************************************************************************
  @file    sap-api.pb.h
  @brief   SAP api proto for uim remote server

  DESCRIPTION
    Handles uim_remote_server messages

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef _PB_SAP_API_PB_H_
#define _PB_SAP_API_PB_H_
#include <pb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _MsgType {
    MsgType_UNKNOWN = 0,
    MsgType_REQUEST = 1,
    MsgType_RESPONSE = 2,
    MsgType_UNSOL_RESPONSE = 3
} MsgType;

typedef enum _MsgId {
    MsgId_UNKNOWN_REQ = 0,
    MsgId_RIL_SIM_SAP_CONNECT = 1,
    MsgId_RIL_SIM_SAP_DISCONNECT = 2,
    MsgId_RIL_SIM_SAP_APDU = 3,
    MsgId_RIL_SIM_SAP_TRANSFER_ATR = 4,
    MsgId_RIL_SIM_SAP_POWER = 5,
    MsgId_RIL_SIM_SAP_RESET_SIM = 6,
    MsgId_RIL_SIM_SAP_STATUS = 7,
    MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS = 8,
    MsgId_RIL_SIM_SAP_ERROR_RESP = 9,
    MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL = 10
} MsgId;

typedef enum _Error {
    Error_RIL_E_SUCCESS = 0,
    Error_RIL_E_RADIO_NOT_AVAILABLE = 1,
    Error_RIL_E_GENERIC_FAILURE = 2,
    Error_RIL_E_REQUEST_NOT_SUPPORTED = 3,
    Error_RIL_E_CANCELLED = 4,
    Error_RIL_E_INVALID_PARAMETER = 5,
    Error_RIL_E_UNUSED = 6
} Error;

typedef enum _RIL_SIM_SAP_CONNECT_RSP_Response {
    RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE = 1,
    RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_MSG_SIZE_TOO_LARGE = 2,
    RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_MSG_SIZE_TOO_SMALL = 3,
    RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_OK_CALL_ONGOING = 4
} RIL_SIM_SAP_CONNECT_RSP_Response;

typedef enum _RIL_SIM_SAP_DISCONNECT_IND_DisconnectType {
    RIL_SIM_SAP_DISCONNECT_IND_DisconnectType_RIL_S_DISCONNECT_TYPE_GRACEFUL = 0,
    RIL_SIM_SAP_DISCONNECT_IND_DisconnectType_RIL_S_DISCONNECT_TYPE_IMMEDIATE = 1
} RIL_SIM_SAP_DISCONNECT_IND_DisconnectType;

typedef enum _RIL_SIM_SAP_APDU_REQ_Type {
    RIL_SIM_SAP_APDU_REQ_Type_RIL_TYPE_APDU = 0,
    RIL_SIM_SAP_APDU_REQ_Type_RIL_TYPE_APDU7816 = 1
} RIL_SIM_SAP_APDU_REQ_Type;

typedef enum _RIL_SIM_SAP_APDU_RSP_Type {
    RIL_SIM_SAP_APDU_RSP_Type_RIL_TYPE_APDU = 0,
    RIL_SIM_SAP_APDU_RSP_Type_RIL_TYPE_APDU7816 = 1
} RIL_SIM_SAP_APDU_RSP_Type;

typedef enum _RIL_SIM_SAP_APDU_RSP_Response {
    RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE = 1,
    RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_NOT_READY = 2,
    RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF = 3,
    RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ABSENT = 4
} RIL_SIM_SAP_APDU_RSP_Response;

typedef enum _RIL_SIM_SAP_TRANSFER_ATR_RSP_Response {
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_GENERIC_FAILURE = 1,
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF = 3,
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ALREADY_POWERED_ON = 18,
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ABSENT = 4,
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_DATA_NOT_AVAILABLE = 6
} RIL_SIM_SAP_TRANSFER_ATR_RSP_Response;

typedef enum _RIL_SIM_SAP_POWER_RSP_Response {
    RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_POWER_RSP_Response_RIL_E_GENERIC_FAILURE = 2,
    RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ABSENT = 11,
    RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF = 17,
    RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ALREADY_POWERED_ON = 18
} RIL_SIM_SAP_POWER_RSP_Response;

typedef enum _RIL_SIM_SAP_RESET_SIM_RSP_Response {
    RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_GENERIC_FAILURE = 2,
    RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ABSENT = 11,
    RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_NOT_READY = 16,
    RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF = 17
} RIL_SIM_SAP_RESET_SIM_RSP_Response;

typedef enum _RIL_SIM_SAP_STATUS_IND_Status {
    RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_UNKNOWN_ERROR = 0,
    RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_RESET = 1,
    RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_NOT_ACCESSIBLE = 2,
    RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_REMOVED = 3,
    RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_INSERTED = 4,
    RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_RECOVERED = 5
} RIL_SIM_SAP_STATUS_IND_Status;

typedef enum _RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response {
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_GENERIC_FAILURE = 2,
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SIM_DATA_NOT_AVAILABLE = 6
} RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response;

typedef enum _RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_Protocol {
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_Protocol_t0 = 0,
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_Protocol_t1 = 1
} RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_Protocol;

typedef enum _RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response {
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response_RIL_E_SUCCESS = 0,
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response_RIL_E_GENERIC_FAILURE = 2,
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response_RIL_E_SIM_ABSENT = 11,
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response_RIL_E_SIM_NOT_READY = 16,
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF = 17
} RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response;

/* Struct definitions */
typedef struct _RIL_SIM_SAP_DISCONNECT_REQ {
    uint8_t dummy_field;
} RIL_SIM_SAP_DISCONNECT_REQ;

typedef struct _RIL_SIM_SAP_DISCONNECT_RSP {
    uint8_t dummy_field;
} RIL_SIM_SAP_DISCONNECT_RSP;

typedef struct _RIL_SIM_SAP_ERROR_RSP {
    uint8_t dummy_field;
} RIL_SIM_SAP_ERROR_RSP;

typedef struct _RIL_SIM_SAP_RESET_SIM_REQ {
    uint8_t dummy_field;
} RIL_SIM_SAP_RESET_SIM_REQ;

typedef struct _RIL_SIM_SAP_TRANSFER_ATR_REQ {
    uint8_t dummy_field;
} RIL_SIM_SAP_TRANSFER_ATR_REQ;

typedef struct _RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ {
    uint8_t dummy_field;
} RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ;

typedef struct _MsgHeader {
    uint32_t token;
    MsgType type;
    MsgId id;
    Error error;
    pb_callback_t payload;
} MsgHeader;

typedef struct _RIL_SIM_SAP_APDU_REQ {
    RIL_SIM_SAP_APDU_REQ_Type type;
    pb_bytes_array_t *command;
} RIL_SIM_SAP_APDU_REQ;

typedef struct _RIL_SIM_SAP_APDU_RSP {
    RIL_SIM_SAP_APDU_RSP_Type type;
    RIL_SIM_SAP_APDU_RSP_Response response;
    pb_bytes_array_t *apduResponse;
} RIL_SIM_SAP_APDU_RSP;

typedef struct _RIL_SIM_SAP_CONNECT_REQ {
    int32_t max_message_size;
} RIL_SIM_SAP_CONNECT_REQ;

typedef struct _RIL_SIM_SAP_CONNECT_RSP {
    RIL_SIM_SAP_CONNECT_RSP_Response response;
    bool has_max_message_size;
    int32_t max_message_size;
} RIL_SIM_SAP_CONNECT_RSP;

typedef struct _RIL_SIM_SAP_DISCONNECT_IND {
    RIL_SIM_SAP_DISCONNECT_IND_DisconnectType disconnectType;
} RIL_SIM_SAP_DISCONNECT_IND;

typedef struct _RIL_SIM_SAP_POWER_REQ {
    bool state;
} RIL_SIM_SAP_POWER_REQ;

typedef struct _RIL_SIM_SAP_POWER_RSP {
    RIL_SIM_SAP_POWER_RSP_Response response;
} RIL_SIM_SAP_POWER_RSP;

typedef struct _RIL_SIM_SAP_RESET_SIM_RSP {
    RIL_SIM_SAP_RESET_SIM_RSP_Response response;
} RIL_SIM_SAP_RESET_SIM_RSP;

typedef struct _RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ {
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_Protocol protocol;
} RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ;

typedef struct _RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP {
    RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_Response response;
} RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP;

typedef struct _RIL_SIM_SAP_STATUS_IND {
    RIL_SIM_SAP_STATUS_IND_Status statusChange;
} RIL_SIM_SAP_STATUS_IND;

typedef struct _RIL_SIM_SAP_TRANSFER_ATR_RSP {
    RIL_SIM_SAP_TRANSFER_ATR_RSP_Response response;
    pb_bytes_array_t *atr;
} RIL_SIM_SAP_TRANSFER_ATR_RSP;

typedef struct _RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP {
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response response;
    bool has_CardReaderStatus;
    int32_t CardReaderStatus;
} RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP;

/* Default values for struct fields */

/* Field tags (for use in manual encoding/decoding) */
#define MsgHeader_token_tag                      1
#define MsgHeader_type_tag                       2
#define MsgHeader_id_tag                         3
#define MsgHeader_error_tag                      4
#define MsgHeader_payload_tag                    5
#define RIL_SIM_SAP_APDU_REQ_type_tag            1
#define RIL_SIM_SAP_APDU_REQ_command_tag         2
#define RIL_SIM_SAP_APDU_RSP_type_tag            1
#define RIL_SIM_SAP_APDU_RSP_response_tag        2
#define RIL_SIM_SAP_APDU_RSP_apduResponse_tag    3
#define RIL_SIM_SAP_CONNECT_REQ_max_message_size_tag 1
#define RIL_SIM_SAP_CONNECT_RSP_response_tag     1
#define RIL_SIM_SAP_CONNECT_RSP_max_message_size_tag 2
#define RIL_SIM_SAP_DISCONNECT_IND_disconnectType_tag 1
#define RIL_SIM_SAP_POWER_REQ_state_tag          1
#define RIL_SIM_SAP_POWER_RSP_response_tag       1
#define RIL_SIM_SAP_RESET_SIM_RSP_response_tag   1
#define RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_protocol_tag 1
#define RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_response_tag 1
#define RIL_SIM_SAP_STATUS_IND_statusChange_tag  1
#define RIL_SIM_SAP_TRANSFER_ATR_RSP_response_tag 1
#define RIL_SIM_SAP_TRANSFER_ATR_RSP_atr_tag     2
#define RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_response_tag 1
#define RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_CardReaderStatus_tag 2

/* Struct field encoding specification for nanopb */
extern const pb_field_t MsgHeader_fields[6];
extern const pb_field_t RIL_SIM_SAP_CONNECT_REQ_fields[2];
extern const pb_field_t RIL_SIM_SAP_CONNECT_RSP_fields[3];
extern const pb_field_t RIL_SIM_SAP_DISCONNECT_REQ_fields[1];
extern const pb_field_t RIL_SIM_SAP_DISCONNECT_RSP_fields[1];
extern const pb_field_t RIL_SIM_SAP_DISCONNECT_IND_fields[2];
extern const pb_field_t RIL_SIM_SAP_APDU_REQ_fields[3];
extern const pb_field_t RIL_SIM_SAP_APDU_RSP_fields[4];
extern const pb_field_t RIL_SIM_SAP_TRANSFER_ATR_REQ_fields[1];
extern const pb_field_t RIL_SIM_SAP_TRANSFER_ATR_RSP_fields[3];
extern const pb_field_t RIL_SIM_SAP_POWER_REQ_fields[2];
extern const pb_field_t RIL_SIM_SAP_POWER_RSP_fields[2];
extern const pb_field_t RIL_SIM_SAP_RESET_SIM_REQ_fields[1];
extern const pb_field_t RIL_SIM_SAP_RESET_SIM_RSP_fields[2];
extern const pb_field_t RIL_SIM_SAP_STATUS_IND_fields[2];
extern const pb_field_t RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_fields[1];
extern const pb_field_t RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields[3];
extern const pb_field_t RIL_SIM_SAP_ERROR_RSP_fields[1];
extern const pb_field_t RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_fields[2];
extern const pb_field_t RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields[2];

/* Maximum encoded size of messages (where known) */
#define RIL_SIM_SAP_CONNECT_REQ_size             11
#define RIL_SIM_SAP_CONNECT_RSP_size             17
#define RIL_SIM_SAP_DISCONNECT_REQ_size          0
#define RIL_SIM_SAP_DISCONNECT_RSP_size          0
#define RIL_SIM_SAP_DISCONNECT_IND_size          6
#define RIL_SIM_SAP_TRANSFER_ATR_REQ_size        0
#define RIL_SIM_SAP_POWER_REQ_size               2
#define RIL_SIM_SAP_POWER_RSP_size               6
#define RIL_SIM_SAP_RESET_SIM_REQ_size           0
#define RIL_SIM_SAP_RESET_SIM_RSP_size           6
#define RIL_SIM_SAP_STATUS_IND_size              6
#define RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_size 0
#define RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_size 17
#define RIL_SIM_SAP_ERROR_RSP_size               0
#define RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_size 6
#define RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_size 6

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
