/******************************************************************************
  @file    uim_remote_client.pb.h
  @brief

  DESCRIPTION
    Handles uim_remote_client message nanopb encode/decode related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef _PB_UIM_REMOTE_CLIENT_PB_H_
#define _PB_UIM_REMOTE_CLIENT_PB_H_
#include <pb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _com_qualcomm_uimremoteclient_MessageType {
    com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_UNKNOWN = 0,
    com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_REQUEST = 1,
    com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE = 2,
    com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION = 3
} com_qualcomm_uimremoteclient_MessageType;

typedef enum _com_qualcomm_uimremoteclient_MessageId {
    com_qualcomm_uimremoteclient_MessageId_UNKNOWN_REQ = 0,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_EVENT = 1,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_APDU = 2,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_CONNECT = 3,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_DISCONNECT = 4,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_POWER_UP = 5,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_POWER_DOWN = 6,
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_RESET = 7
} com_qualcomm_uimremoteclient_MessageId;

typedef enum _com_qualcomm_uimremoteclient_Error {
    com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_SUCCESS = 0,
    com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_GENERIC_FAILURE = 1,
    com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_NOT_SUPPORTED = 2,
    com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_INVALID_PARAMETER = 3
} com_qualcomm_uimremoteclient_Error;

typedef enum _com_qualcomm_uimremoteclient_UimRemoteEventReq_Event {
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CONN_UNAVAILABLE = 0,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CONN_AVAILABLE = 1,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_INSERTED = 2,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_REMOVED = 3,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_ERROR = 4,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_RESET = 5,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_WAKEUP = 6
} com_qualcomm_uimremoteclient_UimRemoteEventReq_Event;

typedef enum _com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause {
    com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_UNKNOWN = 0,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_NO_LINK_EST = 1,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_CMD_TIMEOUT = 2,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_POWER_DOWN = 3,
    com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_POWER_DOWN_TELECOM = 4
} com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause;

typedef enum _com_qualcomm_uimremoteclient_UimRemoteEventResp_Status {
    com_qualcomm_uimremoteclient_UimRemoteEventResp_Status_UIM_REMOTE_SUCCESS = 0,
    com_qualcomm_uimremoteclient_UimRemoteEventResp_Status_UIM_REMOTE_FAILURE = 1
} com_qualcomm_uimremoteclient_UimRemoteEventResp_Status;

typedef enum _com_qualcomm_uimremoteclient_UimRemoteApduReq_ApduStatus {
    com_qualcomm_uimremoteclient_UimRemoteApduReq_ApduStatus_UIM_REMOTE_SUCCESS = 0,
    com_qualcomm_uimremoteclient_UimRemoteApduReq_ApduStatus_UIM_REMOTE_FAILURE = 1
} com_qualcomm_uimremoteclient_UimRemoteApduReq_ApduStatus;

typedef enum _com_qualcomm_uimremoteclient_UimRemoteApduResp_Status {
    com_qualcomm_uimremoteclient_UimRemoteApduResp_Status_UIM_REMOTE_SUCCESS = 0,
    com_qualcomm_uimremoteclient_UimRemoteApduResp_Status_UIM_REMOTE_FAILURE = 1
} com_qualcomm_uimremoteclient_UimRemoteApduResp_Status;

typedef enum _com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass {
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_C_LOW = 0,
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_C = 1,
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_C_HIGH = 2,
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_B_LOW = 3,
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_B = 4,
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_B_HIGH = 5
} com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass;

typedef enum _com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode {
    com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode_UIM_REMOTE_PDOWN_TELECOM_INTERFACE = 0,
    com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode_UIM_REMOTE_PDOWN_CARD = 1
} com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode;

/* Struct definitions */
typedef struct _com_qualcomm_uimremoteclient_MessageTag {
    com_qualcomm_uimremoteclient_MessageType type;
    com_qualcomm_uimremoteclient_MessageId id;
    bool has_error;
    com_qualcomm_uimremoteclient_Error error;
    bool has_token;
    uint32_t token;
    pb_callback_t payload;
} com_qualcomm_uimremoteclient_MessageTag;

typedef struct _com_qualcomm_uimremoteclient_UimRemoteApduInd {
    pb_callback_t apduCommand;
} com_qualcomm_uimremoteclient_UimRemoteApduInd;

typedef struct _com_qualcomm_uimremoteclient_UimRemoteApduReq {
    com_qualcomm_uimremoteclient_UimRemoteApduReq_ApduStatus status;
    pb_callback_t apduResponse;
} com_qualcomm_uimremoteclient_UimRemoteApduReq;

typedef struct _com_qualcomm_uimremoteclient_UimRemoteApduResp {
    com_qualcomm_uimremoteclient_UimRemoteApduResp_Status status;
} com_qualcomm_uimremoteclient_UimRemoteApduResp;

typedef struct _com_qualcomm_uimremoteclient_UimRemoteEventReq {
    com_qualcomm_uimremoteclient_UimRemoteEventReq_Event event;
    pb_callback_t atr;
    bool has_wakeup_support;
    bool wakeup_support;
    bool has_error_code;
    com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause error_code;
} com_qualcomm_uimremoteclient_UimRemoteEventReq;

typedef struct _com_qualcomm_uimremoteclient_UimRemoteEventResp {
    com_qualcomm_uimremoteclient_UimRemoteEventResp_Status response;
} com_qualcomm_uimremoteclient_UimRemoteEventResp;

typedef struct _com_qualcomm_uimremoteclient_UimRemotePowerDownInd {
    bool has_mode;
    com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode mode;
} com_qualcomm_uimremoteclient_UimRemotePowerDownInd;

typedef struct _com_qualcomm_uimremoteclient_UimRemotePowerUpInd {
    bool has_timeout;
    int32_t timeout;
    bool has_voltageclass;
    com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass voltageclass;
} com_qualcomm_uimremoteclient_UimRemotePowerUpInd;

/* Default values for struct fields */

/* Field tags (for use in manual encoding/decoding) */
#define com_qualcomm_uimremoteclient_MessageTag_type_tag 1
#define com_qualcomm_uimremoteclient_MessageTag_id_tag 2
#define com_qualcomm_uimremoteclient_MessageTag_error_tag 3
#define com_qualcomm_uimremoteclient_MessageTag_token_tag 4
#define com_qualcomm_uimremoteclient_MessageTag_payload_tag 5
#define com_qualcomm_uimremoteclient_UimRemoteApduInd_apduCommand_tag 1
#define com_qualcomm_uimremoteclient_UimRemoteApduReq_status_tag 1
#define com_qualcomm_uimremoteclient_UimRemoteApduReq_apduResponse_tag 2
#define com_qualcomm_uimremoteclient_UimRemoteApduResp_status_tag 1
#define com_qualcomm_uimremoteclient_UimRemoteEventReq_event_tag 1
#define com_qualcomm_uimremoteclient_UimRemoteEventReq_atr_tag 2
#define com_qualcomm_uimremoteclient_UimRemoteEventReq_wakeup_support_tag 3
#define com_qualcomm_uimremoteclient_UimRemoteEventReq_error_code_tag 4
#define com_qualcomm_uimremoteclient_UimRemoteEventResp_response_tag 1
#define com_qualcomm_uimremoteclient_UimRemotePowerDownInd_mode_tag 1
#define com_qualcomm_uimremoteclient_UimRemotePowerUpInd_timeout_tag 1
#define com_qualcomm_uimremoteclient_UimRemotePowerUpInd_voltageclass_tag 2

/* Struct field encoding specification for nanopb */
extern const pb_field_t com_qualcomm_uimremoteclient_MessageTag_fields[6];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemoteEventReq_fields[5];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemoteEventResp_fields[2];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemoteApduReq_fields[3];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemoteApduResp_fields[2];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemoteApduInd_fields[2];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemotePowerUpInd_fields[3];
extern const pb_field_t com_qualcomm_uimremoteclient_UimRemotePowerDownInd_fields[2];

/* Maximum encoded size of messages (where known) */
#define com_qualcomm_uimremoteclient_UimRemoteEventResp_size 6
#define com_qualcomm_uimremoteclient_UimRemoteApduResp_size 6
#define com_qualcomm_uimremoteclient_UimRemotePowerUpInd_size 17
#define com_qualcomm_uimremoteclient_UimRemotePowerDownInd_size 6

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
