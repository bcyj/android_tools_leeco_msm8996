/******************************************************************************
  @file    qcril_uim_remote_server_msg_meta.c
  @brief   qcril - uim remote server message meta info

  DESCRIPTION
    Handles uim remote server message meta info related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_uim_remote_server_msg_meta.h"

typedef struct
{
    MsgId msg_id;
    MsgType msg_type;
    const pb_field_t* msg_pb_fields;
    size_t msg_req_size;
    qcril_evt_e_type msg_event;
    const char* msg_log_str;
} qcril_uim_remote_server_msg_meta_type;

const qcril_uim_remote_server_msg_meta_type qcril_uim_remote_server_msg_meta_data[] =
{
    // Requests, Response and Indications
    {MsgId_RIL_SIM_SAP_CONNECT, MsgType_REQUEST,  RIL_SIM_SAP_CONNECT_REQ_fields, sizeof(RIL_SIM_SAP_CONNECT_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_CONNECT_REQ"},
    {MsgId_RIL_SIM_SAP_CONNECT, MsgType_RESPONSE,  RIL_SIM_SAP_CONNECT_RSP_fields, sizeof(RIL_SIM_SAP_CONNECT_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_CONNECT_RSP"},
    {MsgId_RIL_SIM_SAP_DISCONNECT, MsgType_REQUEST, RIL_SIM_SAP_DISCONNECT_REQ_fields, sizeof(RIL_SIM_SAP_DISCONNECT_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_DISCONNECT_REQ"},
    {MsgId_RIL_SIM_SAP_DISCONNECT, MsgType_RESPONSE, RIL_SIM_SAP_DISCONNECT_RSP_fields, sizeof(RIL_SIM_SAP_DISCONNECT_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_DISCONNECT_RSP"},
    {MsgId_RIL_SIM_SAP_DISCONNECT, MsgType_UNSOL_RESPONSE, RIL_SIM_SAP_DISCONNECT_IND_fields, sizeof(RIL_SIM_SAP_DISCONNECT_IND), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_DISCONNECT_IND"},
    {MsgId_RIL_SIM_SAP_APDU, MsgType_REQUEST, RIL_SIM_SAP_APDU_REQ_fields, sizeof(RIL_SIM_SAP_APDU_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_APDU_REQ"},
    {MsgId_RIL_SIM_SAP_APDU, MsgType_RESPONSE, RIL_SIM_SAP_APDU_RSP_fields, sizeof(RIL_SIM_SAP_APDU_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_APDU_RSP"},
    {MsgId_RIL_SIM_SAP_TRANSFER_ATR, MsgType_REQUEST, RIL_SIM_SAP_TRANSFER_ATR_REQ_fields, sizeof(RIL_SIM_SAP_TRANSFER_ATR_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_TRANSFER_ATR_REQ"},
    {MsgId_RIL_SIM_SAP_TRANSFER_ATR, MsgType_RESPONSE, RIL_SIM_SAP_TRANSFER_ATR_RSP_fields, sizeof(RIL_SIM_SAP_TRANSFER_ATR_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_TRANSFER_ATR_RSP"},
    {MsgId_RIL_SIM_SAP_POWER, MsgType_REQUEST, RIL_SIM_SAP_POWER_REQ_fields, sizeof(RIL_SIM_SAP_POWER_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_POWER_REQ"},
    {MsgId_RIL_SIM_SAP_POWER, MsgType_RESPONSE, RIL_SIM_SAP_POWER_RSP_fields, sizeof(RIL_SIM_SAP_POWER_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_POWER_RSP"},
    {MsgId_RIL_SIM_SAP_RESET_SIM, MsgType_REQUEST, RIL_SIM_SAP_RESET_SIM_REQ_fields, sizeof(RIL_SIM_SAP_RESET_SIM_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_RESET_SIM_REQ"},
    {MsgId_RIL_SIM_SAP_RESET_SIM, MsgType_RESPONSE, RIL_SIM_SAP_RESET_SIM_RSP_fields, sizeof(RIL_SIM_SAP_RESET_SIM_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_RESET_SIM_RSP"},
    {MsgId_RIL_SIM_SAP_STATUS, MsgType_UNSOL_RESPONSE, RIL_SIM_SAP_STATUS_IND_fields, sizeof(RIL_SIM_SAP_STATUS_IND), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_STATUS_IND"},
    {MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS, MsgType_REQUEST, RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_fields, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_TRANSFER_CARD_READER_STATUS_REQ"},
    {MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS, MsgType_RESPONSE, RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_TRANSFER_CARD_READER_STATUS_RSP"},
    {MsgId_RIL_SIM_SAP_ERROR_RESP, MsgType_UNSOL_RESPONSE, RIL_SIM_SAP_ERROR_RSP_fields, sizeof(RIL_SIM_SAP_ERROR_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_ERROR_RSP"},
    {MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL, MsgType_REQUEST, RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_fields, sizeof(RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ), QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, "UIM_REMOTE_SERVER_TRANSFER_PROTOCOL_REQ"},
    {MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL, MsgType_RESPONSE, RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields, sizeof(RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP), QCRIL_EVT_NONE, "UIM_REMOTE_SERVER_TRANSFER_PROTOCOL_RSP"}
};

size_t qcril_uim_remote_server_get_msg_size(MsgId msg_id, MsgType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_server_msg_meta_data)/sizeof(qcril_uim_remote_server_msg_meta_data[0]); i++)
    {
        if ( qcril_uim_remote_server_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_server_msg_meta_data[i].msg_type == msg_type )
        {
            return qcril_uim_remote_server_msg_meta_data[i].msg_req_size;
        }
    }
    return 0;
}

const pb_field_t* qcril_uim_remote_server_get_msg_pb_fields(MsgId msg_id, MsgType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_server_msg_meta_data)/sizeof(qcril_uim_remote_server_msg_meta_data[0]); i++)
    {
        if ( qcril_uim_remote_server_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_server_msg_meta_data[i].msg_type == msg_type )
        {
            return qcril_uim_remote_server_msg_meta_data[i].msg_pb_fields;
        }
    }
    return NULL;
}

const char* qcril_uim_remote_server_get_msg_log_str(MsgId msg_id, MsgType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_server_msg_meta_data)/sizeof(qcril_uim_remote_server_msg_meta_data[0]); i++)
    {
        if ( qcril_uim_remote_server_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_server_msg_meta_data[i].msg_type == msg_type )
        {
            if (qcril_uim_remote_server_msg_meta_data[i].msg_log_str)
            {
                return qcril_uim_remote_server_msg_meta_data[i].msg_log_str;
            }
            else
            {
                break;
            }
        }
    }
    return "Unknown msg";
}

qcril_evt_e_type qcril_uim_remote_server_get_msg_event(MsgId msg_id, MsgType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_server_msg_meta_data)/sizeof(qcril_uim_remote_server_msg_meta_data[0]); i++)
    {
        if (qcril_uim_remote_server_msg_meta_data[i].msg_id == msg_id)
        {
            return qcril_uim_remote_server_msg_meta_data[i].msg_event;
        }
    }
    QCRIL_LOG_ERROR("did not find the corresponding event for %s(msg id %d, type %d)", qcril_uim_remote_server_get_msg_log_str(msg_id, msg_type), msg_id, msg_type);
    return QCRIL_EVT_NONE;
}
boolean qcril_uim_remote_server_is_msg_valid(MsgId msg_id, MsgType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_server_msg_meta_data)/sizeof(qcril_uim_remote_server_msg_meta_data[0]); i++)
    {
        if (qcril_uim_remote_server_msg_meta_data[i].msg_id == msg_id  &&
             qcril_uim_remote_server_msg_meta_data[i].msg_type == msg_type)
        {
            return TRUE;
        }
    }
    QCRIL_LOG_ERROR("not a valid msg event for %s(msg id %d, type %d)", qcril_uim_remote_server_get_msg_log_str(msg_id, msg_type), msg_id, msg_type);
    return FALSE;
}
