/******************************************************************************
  @file    qcril_uim_remote_client_msg_meta.c
  @brief   qcril - uim remote card client message meta info

  DESCRIPTION
    Handles uim remote card client message meta info related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_uim_remote_client_msg_meta.h"

typedef struct
{
    com_qualcomm_uimremoteclient_MessageId msg_id;
    com_qualcomm_uimremoteclient_MessageType msg_type;
    const pb_field_t* msg_pb_fields;
    size_t msg_req_size;
    qcril_evt_e_type msg_event;
    const char* msg_log_str;
} qcril_uim_remote_client_msg_meta_type;

const qcril_uim_remote_client_msg_meta_type qcril_uim_remote_client_msg_meta_data[] =
{
    // Requests, Response and Indications
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_EVENT, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_REQUEST,  com_qualcomm_uimremoteclient_UimRemoteEventReq_fields, sizeof(com_qualcomm_uimremoteclient_UimRemoteEventReq), QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_EVENT, "UIM_REMOTE_CLIENT_EVENT_REQ"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_EVENT, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE,  com_qualcomm_uimremoteclient_UimRemoteEventResp_fields, sizeof(com_qualcomm_uimremoteclient_UimRemoteEventResp), QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_EVENT_RSP"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_APDU, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_REQUEST, com_qualcomm_uimremoteclient_UimRemoteApduReq_fields, sizeof(com_qualcomm_uimremoteclient_UimRemoteApduReq), QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_APDU, "UIM_REMOTE_CLIENT_APDU_REQ"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_APDU, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE, com_qualcomm_uimremoteclient_UimRemoteApduResp_fields, sizeof(com_qualcomm_uimremoteclient_UimRemoteApduResp), QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_APDU_RSP"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_APDU, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION, com_qualcomm_uimremoteclient_UimRemoteApduInd_fields, sizeof(com_qualcomm_uimremoteclient_UimRemoteApduInd), QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_APDU_IND"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_CONNECT, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION, NULL, 0, QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_CONNECT_IND"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_DISCONNECT, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION, NULL, 0, QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_DISCONNECT_IND"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_POWER_UP, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION, com_qualcomm_uimremoteclient_UimRemotePowerUpInd_fields, sizeof(com_qualcomm_uimremoteclient_UimRemotePowerUpInd), QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_POWER_UP_IND"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_POWER_DOWN, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION, com_qualcomm_uimremoteclient_UimRemotePowerDownInd_fields, sizeof(com_qualcomm_uimremoteclient_UimRemotePowerDownInd), QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_POWER_DOWN_IND"},
    {com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_RESET, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION, NULL, 0, QCRIL_EVT_NONE, "UIM_REMOTE_CLIENT_RESET_IND"}
};

size_t qcril_uim_remote_client_get_msg_size(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_client_msg_meta_data)/sizeof(qcril_uim_remote_client_msg_meta_data[0]); i++)
    {
        if ( qcril_uim_remote_client_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_client_msg_meta_data[i].msg_type == msg_type )
        {
            return qcril_uim_remote_client_msg_meta_data[i].msg_req_size;
        }
    }
    return -1;
}

const pb_field_t* qcril_uim_remote_client_get_msg_pb_fields(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_client_msg_meta_data)/sizeof(qcril_uim_remote_client_msg_meta_data[0]); i++)
    {
        if ( qcril_uim_remote_client_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_client_msg_meta_data[i].msg_type == msg_type )
        {
            return qcril_uim_remote_client_msg_meta_data[i].msg_pb_fields;
        }
    }
    return NULL;
}

const char* qcril_uim_remote_client_get_msg_log_str(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_client_msg_meta_data)/sizeof(qcril_uim_remote_client_msg_meta_data[0]); i++)
    {
        if ( qcril_uim_remote_client_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_client_msg_meta_data[i].msg_type == msg_type )
        {
            if (qcril_uim_remote_client_msg_meta_data[i].msg_log_str)
            {
                return qcril_uim_remote_client_msg_meta_data[i].msg_log_str;
            }
            else
            {
                break;
            }
        }
    }
    return "Unknown msg";
}

qcril_evt_e_type qcril_uim_remote_client_get_msg_event(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_client_msg_meta_data)/sizeof(qcril_uim_remote_client_msg_meta_data[0]); i++)
    {
        if (qcril_uim_remote_client_msg_meta_data[i].msg_id == msg_id)
        {
            return qcril_uim_remote_client_msg_meta_data[i].msg_event;
        }
    }
    QCRIL_LOG_ERROR("did not find the corresponding event for %s(msg id %d, type %d)", qcril_uim_remote_client_get_msg_log_str(msg_id, msg_type), msg_id, msg_type);
    return QCRIL_EVT_NONE;
}

boolean qcril_uim_remote_client_is_msg_valid(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type)
{
    size_t i;
    for (i=0; i<sizeof(qcril_uim_remote_client_msg_meta_data)/sizeof(qcril_uim_remote_client_msg_meta_data[0]); i++)
    {
        if (qcril_uim_remote_client_msg_meta_data[i].msg_id == msg_id &&
             qcril_uim_remote_client_msg_meta_data[i].msg_type == msg_type)
        {
            return TRUE;
        }
    }
    QCRIL_LOG_ERROR("did not find a valid entry in the meta table for %s(msg id %d, type %d)", qcril_uim_remote_client_get_msg_log_str(msg_id, msg_type), msg_id, msg_type);
    return FALSE;
}
