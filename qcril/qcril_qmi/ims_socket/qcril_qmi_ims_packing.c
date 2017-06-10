/******************************************************************************
  @file    qcril_qmi_ims_packing.c
  @brief   qcril qmi - ims packing

  DESCRIPTION
    Handles ims message packing/unpacking related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_qmi_ims_packing.h"
#include "qcril_log.h"

//===========================================================================
// qcril_qmi_ims_pack_msg_tag
//===========================================================================
size_t qcril_qmi_ims_pack_msg_tag(uint32_t token, Ims__MsgType type, Ims__MsgId message_id, Ims__Error error, uint8_t *buf, size_t buf_size)
{
    QCRIL_LOG_FUNC_ENTRY();
    if (NULL == buf)
    {
        QCRIL_LOG_ERROR("buf is NULL");
        return 0;
    }

    Ims__MsgTag msg_tag = IMS__MSG_TAG__INIT;
    size_t tag_size = 0;

    msg_tag.token = token;
    msg_tag.type = type;
    msg_tag.id = message_id;
    msg_tag.error = error;

    tag_size = qcril_qmi_ims__msg_tag__get_packed_size(&msg_tag);

    if (buf_size < tag_size+1)
    {
        QCRIL_LOG_ERROR("buf_size < tag_size+1");
        return 0;
    }
    else
    {
        buf[0] = tag_size;
        if (qcril_qmi_ims__msg_tag__pack(&msg_tag, &(buf[1])) != tag_size)
        {
            QCRIL_LOG_ERROR("tag_size is different from ims__msg_tag__pack size");
        }
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET((int) (tag_size+1));
    return tag_size+1;
} // qcril_qmi_ims_pack_msg_tag

//===========================================================================
// qcril_qmi_ims_unpack_msg_tag
//===========================================================================
Ims__MsgTag* qcril_qmi_ims_unpack_msg_tag(const uint8_t *buf)
{
    QCRIL_LOG_FUNC_ENTRY();
    Ims__MsgTag* msg_tag_ptr = NULL;
    size_t msg_tag_size = 0;

    if (NULL == buf)
    {
        QCRIL_LOG_ERROR("buf is NULL");
    }
    else
    {
        msg_tag_size = buf[0];
        msg_tag_ptr = qcril_qmi_ims__msg_tag__unpack (NULL, msg_tag_size, &buf[1]);
    }

    QCRIL_LOG_FUNC_RETURN();
    return msg_tag_ptr;
} // qcril_qmi_ims_unpack_msg_tag

//===========================================================================
// qcril_qmi_ims_pack_msg
//===========================================================================
size_t qcril_qmi_ims_pack_msg(const void *msg, Ims__MsgType type, Ims__MsgId message_id, uint8_t *buf, size_t max_buf_size)
{
    size_t ret = 0;
    QCRIL_LOG_FUNC_ENTRY();

    if (NULL == buf)
    {
        QCRIL_LOG_ERROR("buf is NULL");
    }
    else
    {
        QCRIL_LOG_INFO("message id: %d, type: %d", message_id, type);
        if (NULL == msg)
        {
            QCRIL_LOG_INFO("msg is NULL");
        }
        else
        {
            if (IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE == message_id && IMS__MSG_TYPE__RESPONSE == type ||
                IMS__MSG_ID__UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__registration__get_packed_size((Ims__Registration*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__registration__pack((Ims__Registration*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_DIAL == message_id && IMS__MSG_TYPE__REQUEST == type)
            {
                ret = qcril_qmi_ims__dial__get_packed_size((Ims__Dial*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__dial__pack((Ims__Dial*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_ANSWER == message_id && IMS__MSG_TYPE__REQUEST == type)
            {
                ret = qcril_qmi_ims__answer__get_packed_size((Ims__Answer*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__answer__pack((Ims__Answer*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_HANGUP == message_id && IMS__MSG_TYPE__REQUEST == type)
            {
                ret = qcril_qmi_ims__hangup__get_packed_size((Ims__Hangup*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__hangup__pack((Ims__Hangup*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE == message_id && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__call_fail_cause_response__get_packed_size((Ims__CallFailCauseResponse*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__call_fail_cause_response__pack((Ims__CallFailCauseResponse*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if ( (IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS == message_id && IMS__MSG_TYPE__RESPONSE == type) ||
                      (IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type) )
            {
                ret = qcril_qmi_ims__call_list__get_packed_size((Ims__CallList*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__call_list__pack((Ims__CallList*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING == message_id && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__call_waiting_info__get_packed_size((Ims__CallWaitingInfo*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__call_waiting_info__pack((Ims__CallWaitingInfo*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS == message_id && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__call_forward_info_list__get_packed_size((Ims__CallForwardInfoList*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__call_forward_info_list__pack((Ims__CallForwardInfoList*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_QUERY_SERVICE_STATUS == message_id && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__srv_status_list__get_packed_size((Ims__SrvStatusList*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__srv_status_list__pack((Ims__SrvStatusList*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION == message_id && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__supp_svc_status__get_packed_size((Ims__SuppSvcStatus*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__supp_svc_status__pack((Ims__SuppSvcStatus*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_GET_CLIR == message_id && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__clir__get_packed_size((Ims__Clir*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__clir__pack((Ims__Clir*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (((IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS == message_id) ||
                       (IMS__MSG_ID__REQUEST_SET_CALL_FORWARD_STATUS == message_id) ||
                       (IMS__MSG_ID__REQUEST_SET_CALL_WAITING == message_id))&&
                     IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__supp_svc_response__get_packed_size(
                        (Ims__SuppSvcResponse*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");

                    ret = 0;
                }
                else
                {
                    if (qcril_qmi_ims__supp_svc_response__pack(
                          (Ims__SuppSvcResponse*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_QUERY_CLIP == message_id &&
                     IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__clip_provision_status__get_packed_size(\
                  (Ims__ClipProvisionStatus*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    ret = 0;
                }
                else
                {
                    if (qcril_qmi_ims__clip_provision_status__pack(\
                      (Ims__ClipProvisionStatus*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_GET_COLR == message_id &&
                     IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__colr__get_packed_size((Ims__Colr*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    ret = 0;
                }
                else
                {
                    if (qcril_qmi_ims__colr__pack((Ims__Colr*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY == message_id &&
                     IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__video_call_quality__get_packed_size(
                        (Ims__VideoCallQuality*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__video_call_quality__pack(
                          (Ims__VideoCallQuality*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if ( (IMS__MSG_ID__REQUEST_GET_RTP_STATISTICS == message_id ||
                       IMS__MSG_ID__REQUEST_GET_RTP_ERROR_STATISTICS == message_id)
                      && IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__rtp_statistics_data__get_packed_size(
                        (Ims__RtpStatisticsData*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__rtp_statistics_data__pack(
                          (Ims__RtpStatisticsData*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if ( IMS__MSG_ID__REQUEST_GET_WIFI_CALLING_STATUS == message_id &&
                      IMS__MSG_TYPE__RESPONSE == type)
            {
                ret = qcril_qmi_ims__wifi_calling_info__get_packed_size(
                        (Ims__WifiCallingInfo*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__wifi_calling_info__pack(
                          (Ims__WifiCallingInfo*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_RINGBACK_TONE == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__ring_back_tone__get_packed_size((Ims__RingBackTone*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__ring_back_tone__pack((Ims__RingBackTone*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_MODIFY_CALL == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__call_modify__get_packed_size((Ims__CallModify*)msg);
                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__call_modify__pack((Ims__CallModify*)msg, buf) != ret)

                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_RESPONSE_HANDOVER == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__handover__get_packed_size((Ims__Handover*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__handover__pack((Ims__Handover*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_REFRESH_CONF_INFO == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__conf_info__get_packed_size((Ims__ConfInfo*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__conf_info__pack((Ims__ConfInfo*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_SRV_STATUS_UPDATE == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__srv_status_list__get_packed_size((Ims__SrvStatusList*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__srv_status_list__pack((Ims__SrvStatusList*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_SUPP_SVC_NOTIFICATION == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__supp_svc_notification__get_packed_size((Ims__SuppSvcNotification*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__supp_svc_notification__pack((Ims__SuppSvcNotification*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_RADIO_STATE_CHANGED == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__radio_state_changed__get_packed_size((Ims__RadioStateChanged*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__radio_state_changed__pack((Ims__RadioStateChanged*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_TTY_NOTIFICATION == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__tty_notify__get_packed_size((Ims__TtyNotify*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__tty_notify__pack((Ims__TtyNotify*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else if (IMS__MSG_ID__UNSOL_MWI == message_id && IMS__MSG_TYPE__UNSOL_RESPONSE == type)
            {
                ret = qcril_qmi_ims__mwi__get_packed_size((Ims__Mwi*)msg);

                if (max_buf_size < ret)
                {
                    QCRIL_LOG_ERROR("buf_size < size");
                    return 0;
                }
                else
                {
                    if (qcril_qmi_ims__mwi__pack((Ims__Mwi*)msg, buf) != ret)
                    {
                        QCRIL_LOG_ERROR("get_packed_size is different from pack size");
                    }
                }
            }
            else
            {
                QCRIL_LOG_INFO("cannot pack this mesage");
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} // qcril_qmi_ims_pack_msg

//===========================================================================
// qcril_qmi_ims_parse_packed_msg
//===========================================================================
void qcril_qmi_ims_parse_packed_msg(Ims__MsgType type, Ims__MsgId message_id, const uint8_t *packed_msg, size_t packed_msg_size,
                                     void** unpacked_msg, size_t*unpacked_msg_size_ptr, qcril_evt_e_type* event_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();

    *unpacked_msg = NULL;
    *unpacked_msg_size_ptr = 0;
    *event_ptr = QCRIL_EVT_NONE;

    if (NULL == packed_msg)
    {
        QCRIL_LOG_ERROR("packed_msg is NULL");
    }
    else
    {
        QCRIL_LOG_INFO("message id: %d, type: %d", message_id, type);

        if (IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_IMS_REGISTRATION_STATE;
        }
        else if (IMS__MSG_ID__REQUEST_DIAL == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Dial *msg_dial_ptr = NULL;
            msg_dial_ptr = qcril_qmi_ims__dial__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_dial_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Dial);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_DIAL;
        }
        else if (IMS__MSG_ID__REQUEST_ANSWER == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Answer *msg_answer_ptr = NULL;
            msg_answer_ptr = qcril_qmi_ims__answer__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_answer_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Answer);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_ANSWER;
        }
        else if (IMS__MSG_ID__REQUEST_HANGUP == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Hangup *msg_hangup_ptr = NULL;
            msg_hangup_ptr = qcril_qmi_ims__hangup__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_hangup_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Hangup);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_HANGUP;
        }
        else if (IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE;
        }
        else if (IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS;
        }
        else if (IMS__MSG_ID__REQUEST_HANGUP_WAITING_OR_BACKGROUND == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND;
        }
        else if (IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND;
        }
        else if (IMS__MSG_ID__REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__SwitchWaitingOrHoldingAndActive *msg_switch_waiting_or_holding_and_active_ptr = NULL;
            msg_switch_waiting_or_holding_and_active_ptr = qcril_qmi_ims__switch_waiting_or_holding_and_active__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_switch_waiting_or_holding_and_active_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__SwitchWaitingOrHoldingAndActive);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
        }
        else if (IMS__MSG_ID__REQUEST_CONFERENCE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE;
        }
        else if (IMS__MSG_ID__REQUEST_EXIT_EMERGENCY_CALLBACK_MODE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_EXIT_ECBM;
        }
        else if (IMS__MSG_ID__REQUEST_DTMF == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Dtmf *msg_dtmf_ptr = NULL;
            msg_dtmf_ptr = qcril_qmi_ims__dtmf__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_dtmf_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Dtmf);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_DTMF;
        }
        else if (IMS__MSG_ID__REQUEST_DTMF_START == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Dtmf *msg_dtmf_ptr = NULL;
            msg_dtmf_ptr = qcril_qmi_ims__dtmf__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_dtmf_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Dtmf);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START;
        }
        else if (IMS__MSG_ID__REQUEST_DTMF_STOP == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP;
        }
        else if (IMS__MSG_ID__REQUEST_MODIFY_CALL_INITIATE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__CallModify *msg_callmodify_ptr = NULL;
            msg_callmodify_ptr = qcril_qmi_ims__call_modify__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_callmodify_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__CallModify);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE;
        }
        else if (IMS__MSG_ID__REQUEST_MODIFY_CALL_CONFIRM == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__CallModify *msg_callmodify_ptr = NULL;
            msg_callmodify_ptr = qcril_qmi_ims__call_modify__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_callmodify_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__CallModify);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM;
        }
        else if (IMS__MSG_ID__REQUEST_QUERY_CLIP == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP;
        }
        else if (IMS__MSG_ID__REQUEST_GET_CLIR == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR;
        }
        else if (IMS__MSG_ID__REQUEST_SET_CLIR == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Clir *msg_clir_ptr = NULL;
            msg_clir_ptr = qcril_qmi_ims__clir__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_clir_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Clir);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR;
        }
        else if (IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__CallForwardInfoList *msg_callforwardinfolist_ptr = NULL;
            msg_callforwardinfolist_ptr = qcril_qmi_ims__call_forward_info_list__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_callforwardinfolist_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__CallForwardInfoList);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS;
        }
        else if (IMS__MSG_ID__REQUEST_SET_CALL_FORWARD_STATUS == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__CallForwardInfoList *msg_callforwardinfolist_ptr = NULL;
            msg_callforwardinfolist_ptr = qcril_qmi_ims__call_forward_info_list__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_callforwardinfolist_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__CallForwardInfoList);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS;
        }
        else if (IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__ServiceClass *msg_serviceclass_ptr = NULL;
            msg_serviceclass_ptr = qcril_qmi_ims__service_class__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_serviceclass_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__ServiceClass);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING;
        }
        else if (IMS__MSG_ID__REQUEST_SET_CALL_WAITING == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__CallWaitingInfo *msg_callwaitinginfo_ptr = NULL;
            msg_callwaitinginfo_ptr = qcril_qmi_ims__call_waiting_info__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_callwaitinginfo_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__CallWaitingInfo);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING;
        }
        else if (IMS__MSG_ID__REQUEST_IMS_REG_STATE_CHANGE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Registration *msg_registration_ptr = NULL;
            msg_registration_ptr = qcril_qmi_ims__registration__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_registration_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Registration);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_IMS_REG_STATE_CHANGE;
        }
        else if (IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__SuppSvcStatus *msg_suppsvcstatus_ptr = NULL;
            msg_suppsvcstatus_ptr = qcril_qmi_ims__supp_svc_status__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_suppsvcstatus_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__SuppSvcStatus);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION;
        }
        else if (IMS__MSG_ID__REQUEST_ADD_PARTICIPANT == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Dial *msg_dial_ptr = NULL;
            msg_dial_ptr = qcril_qmi_ims__dial__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_dial_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Dial);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT;
        }
        else if (IMS__MSG_ID__REQUEST_QUERY_SERVICE_STATUS == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_SERVICE_STATUS;
        }
        else if (IMS__MSG_ID__REQUEST_SET_SERVICE_STATUS == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Info *msg_info_ptr = NULL;
            msg_info_ptr = qcril_qmi_ims__info__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_info_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Info);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_SERVICE_STATUS;
        }
        else if (IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS == message_id &&
                 IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__SuppSvcRequest *msg_supp_svc_request_ptr = NULL;
            msg_supp_svc_request_ptr = qcril_qmi_ims__supp_svc_request__unpack (NULL,
                                                                      packed_msg_size,
                                                                      packed_msg);
            *unpacked_msg = (void*)msg_supp_svc_request_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__SuppSvcRequest);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS;
        }
        else if (IMS__MSG_ID__REQUEST_DEFLECT_CALL == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__DeflectCall *msg_call_deflect_ptr = NULL;
            msg_call_deflect_ptr = qcril_qmi_ims__deflect_call__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_call_deflect_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__DeflectCall);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION;
        }
        else if (IMS__MSG_ID__REQUEST_GET_COLR == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR;
        }
        else if (IMS__MSG_ID__REQUEST_SET_COLR == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Colr *msg_colr_ptr = NULL;
            msg_colr_ptr = qcril_qmi_ims__colr__unpack(NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_colr_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Colr);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR;
        }
        else if (IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY == message_id &&
                 IMS__MSG_TYPE__REQUEST == type)
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY;
        }
        else if (IMS__MSG_ID__REQUEST_SET_VT_CALL_QUALITY == message_id &&
                 IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__VideoCallQuality *msg_vcq_ptr = NULL;
            msg_vcq_ptr = qcril_qmi_ims__video_call_quality__unpack(NULL,
                    packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_vcq_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__VideoCallQuality);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_VT_CALL_QUALITY;
        }
        else if (IMS__MSG_ID__REQUEST_HOLD == message_id
                 && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Hold *msg_hold_ptr = NULL;
            msg_hold_ptr = qcril_qmi_ims__hold__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_hold_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Hold);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_HOLD;
            QCRIL_LOG_ERROR("In packing: has_callid:%d",msg_hold_ptr->has_callid);
        }
        else if (IMS__MSG_ID__REQUEST_RESUME == message_id
                 && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__Resume *msg_resume_ptr = NULL;
            msg_resume_ptr = qcril_qmi_ims__resume__unpack (NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_resume_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__Resume);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_RESUME;
        }
        else if (IMS__MSG_ID__REQUEST_SEND_UI_TTY_MODE == message_id && IMS__MSG_TYPE__REQUEST == type)
        {
            Ims__TtyNotify *msg_tty_notify_ptr = NULL;
            msg_tty_notify_ptr = qcril_qmi_ims__tty_notify__unpack(NULL, packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_tty_notify_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__TtyNotify);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_TTY_MODE;
        }
        else if (IMS__MSG_ID__REQUEST_GET_RTP_STATISTICS == message_id
                 && IMS__MSG_TYPE__REQUEST == type )
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_STATISTICS;
        }
        else if (IMS__MSG_ID__REQUEST_GET_RTP_ERROR_STATISTICS == message_id
                 && IMS__MSG_TYPE__REQUEST == type )
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_ERROR_STATISTICS;
        }
        else if (IMS__MSG_ID__REQUEST_GET_WIFI_CALLING_STATUS == message_id
                 && IMS__MSG_TYPE__REQUEST == type )
        {
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS;
        }
        else if (IMS__MSG_ID__REQUEST_SET_WIFI_CALLING_STATUS == message_id
                 && IMS__MSG_TYPE__REQUEST == type )
        {
            Ims__WifiCallingInfo *msg_wci_ptr = NULL;
            msg_wci_ptr = qcril_qmi_ims__wifi_calling_info__unpack(NULL,
                    packed_msg_size, packed_msg);
            *unpacked_msg = (void*)msg_wci_ptr;
            *unpacked_msg_size_ptr = sizeof(Ims__WifiCallingInfo);
            *event_ptr = QCRIL_EVT_IMS_SOCKET_REQ_SET_WIFI_CALLING_STATUS;
        }
        else
        {
            QCRIL_LOG_DEBUG("NULL data");
        }
    }
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_ims_parse_packed_msg
