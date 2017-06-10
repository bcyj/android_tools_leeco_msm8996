/******************************************************************************
  @file    qmi_simple_ril_core.c
  @brief   Sample simple RIL voice

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) voice subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

#include "voice_service_v02.h"
#include "qmi_simple_ril_voice.h"


#ifndef FEATURE_QMI_ANDROID
#define strlcpy g_strlcpy
#endif /* FEATURE_QMI_ANDROID */

typedef struct qmi_simple_ril_voice_svc_info
{
    int ringing_call_id;
    int existing_call_id;

    qmi_util_request_id init_call_state_req_id;
} qmi_simple_ril_voice_svc_info;


static qmi_simple_ril_voice_svc_info simple_ril_voice_svc_info;

static char* qmi_simple_ril_voice_get_call_state_str(int state);


void qmi_simple_ril_voice_init()
{
    qmi_util_request_params req_params;

    memset(&simple_ril_voice_svc_info, 0, sizeof(simple_ril_voice_svc_info));

    // let's ask for cur call states
    memset( &req_params, 0, sizeof(req_params) );
    req_params.service_id = QMI_UTIL_SVC_VOICE;
    req_params.message_id = QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02;
    req_params.message_specific_payload = NULL; // no payload
    req_params.message_specific_payload_len = 0;
    simple_ril_voice_svc_info.init_call_state_req_id = qmi_util_post_request(&req_params);
}

int qmi_simple_ril_voice_call_state_resp_handler (qmi_util_service_message_info* unsolicited_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    qmi_util_logln1("qmi_simple_ril_voice_call_state_resp_handler", unsolicited_msg_info->message_specific_payload_len);
    voice_get_all_call_info_resp_msg_v02* call_info_msg;

    char call_state_buf[32 + 20]; //extra 20 characters for formatting the output.
    char * call_state_str;
    uint32_t idx;

    if ( uplink_message->request_id == simple_ril_voice_svc_info.init_call_state_req_id )
    {
        uplink_message->must_be_silent = TRUE;
    }

    if (unsolicited_msg_info->message_specific_payload_len > 0 && NULL != unsolicited_msg_info->message_specific_payload)
        {
        call_info_msg = (voice_get_all_call_info_resp_msg_v02*)unsolicited_msg_info->message_specific_payload;
        qmi_util_logln1("qmi_simple_ril_voice_call_state_resp_handler", call_info_msg->call_info_len);
    
        simple_ril_voice_svc_info.ringing_call_id = 0;
    
        switch (call_info_msg->call_info_len)
            {
            case 0: // no calls, no output
                qmi_simple_ril_core_set_cond_var("call_state", "NONE");
                simple_ril_voice_svc_info.ringing_call_id = 0;
                simple_ril_voice_svc_info.existing_call_id = 0;
                break;
    
            case 1: // single call, simple output
                call_state_str = qmi_simple_ril_voice_get_call_state_str(call_info_msg->call_info[0].call_state);
                qmi_simple_ril_core_set_cond_var("call_state", call_state_str);
                sprintf(call_state_buf, "call state %s",call_state_str );
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, call_state_buf);
                if (0x02 == call_info_msg->call_info[0].call_state)
                    {
                    simple_ril_voice_svc_info.ringing_call_id = call_info_msg->call_info[0].call_id;
                    }
                simple_ril_voice_svc_info.existing_call_id = call_info_msg->call_info[0].call_id;
                break; 
    
            default: // multiple calls
                sprintf(call_state_buf, "voice call state update, total calls %d", call_info_msg->call_info_len);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, call_state_buf);
    
                for (idx = 0; idx < call_info_msg->call_info_len; idx++)
                    {
                    sprintf(call_state_buf, "call %d in state %s", idx + 1, qmi_simple_ril_voice_get_call_state_str(call_info_msg->call_info[0].call_state));
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, call_state_buf);
                    if (0x02 == call_info_msg->call_info[idx].call_state)
                        {
                        simple_ril_voice_svc_info.ringing_call_id = call_info_msg->call_info[idx].call_id;
                        }
                    simple_ril_voice_svc_info.existing_call_id = call_info_msg->call_info[idx].call_id;
                    }
                break;
            }
        }
    return 0;
    }


int qmi_simple_ril_dial_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    voice_dial_call_req_msg_v02	 dial_call_req_msg;
    qmi_util_request_params req_params;

    if (cmd_params->info_set->nof_entries != 1)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&dial_call_req_msg, 0, sizeof(dial_call_req_msg));	
        // first and only  entry should be phone number

        strlcpy(dial_call_req_msg.calling_number,
                 cmd_params->info_set->entries[0],
                 sizeof(dial_call_req_msg.calling_number));

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_VOICE;
        req_params.message_id = QMI_VOICE_DIAL_CALL_REQ_V02;
        req_params.message_specific_payload = &dial_call_req_msg;
        req_params.message_specific_payload_len = sizeof(dial_call_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
    }

static int qmi_simple_ril_do_voice_answer_reject(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info, int is_answer)
    {
    int error;
    qmi_util_request_params req_params;
    voice_answer_call_req_msg_v02 answer_params;
    voice_end_call_req_msg_v02 end_params;
    if (simple_ril_voice_svc_info.ringing_call_id)
        {
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_VOICE;

        if (is_answer)
            {
            memset(&answer_params, 0, sizeof(answer_params));
            answer_params.call_id = simple_ril_voice_svc_info.ringing_call_id;
            req_params.message_specific_payload = &answer_params;
            req_params.message_specific_payload_len = sizeof(answer_params);
            req_params.message_id = QMI_VOICE_ANSWER_CALL_REQ_V02;
            }
        else
            { // reject
            memset(&end_params, 0, sizeof(end_params));
            end_params.call_id = simple_ril_voice_svc_info.ringing_call_id;
            req_params.message_specific_payload = &end_params;
            req_params.message_specific_payload_len = sizeof(end_params);
            req_params.message_id = QMI_VOICE_END_CALL_REQ_V02;
            }

        ack_info->request_id = qmi_util_post_request(&req_params);

        error = QMI_SIMPLE_RIL_ERR_NONE;
        }
    else
        {
        error = QMI_SIMPLE_RIL_ERR_INVALID_CONTEXT;
        }
    if (QMI_SIMPLE_RIL_ERR_NONE != error)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, error);
        }
    return 0;
    }

int qmi_simple_ril_voice_answer_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    return qmi_simple_ril_do_voice_answer_reject(cmd_params, ack_info, 1);
    }

int qmi_simple_ril_voice_reject_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    return qmi_simple_ril_do_voice_answer_reject(cmd_params, ack_info, 0);
    }

int qmi_simple_ril_voice_end_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    int error;
    qmi_util_request_params req_params;
    voice_end_call_req_msg_v02 end_params;
    if (simple_ril_voice_svc_info.existing_call_id)
        {
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_VOICE;

        memset(&end_params, 0, sizeof(end_params));
        end_params.call_id = simple_ril_voice_svc_info.existing_call_id;
        req_params.message_specific_payload = &end_params;
        req_params.message_specific_payload_len = sizeof(end_params);
        req_params.message_id = QMI_VOICE_END_CALL_REQ_V02;

        ack_info->request_id = qmi_util_post_request(&req_params);

        error = QMI_SIMPLE_RIL_ERR_NONE;
        }
    else
        {
        error = QMI_SIMPLE_RIL_ERR_INVALID_CONTEXT;
        }
    if (QMI_SIMPLE_RIL_ERR_NONE != error)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, error);
        }
    return 0;
    }

static int qmi_simple_ril_do_voice_manage_calls(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info, uint8_t action)
    {
    int error;
    qmi_util_request_params req_params;
    voice_manage_calls_req_msg_v02 manage_params;
    qmi_util_logln2("qmi_simple_ril_do_voice_manage_calls ", simple_ril_voice_svc_info.existing_call_id, (int)action);
    if (simple_ril_voice_svc_info.existing_call_id)
        {
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_VOICE;
        req_params.message_specific_payload = &manage_params;
        req_params.message_specific_payload_len = sizeof(manage_params);
        req_params.message_id = QMI_VOICE_MANAGE_CALLS_REQ_V02;

        memset(&manage_params, 0, sizeof(manage_params));
        manage_params.call_id = simple_ril_voice_svc_info.existing_call_id;
        manage_params.call_id_valid = 1;
        manage_params.sups_type = action;

        ack_info->request_id = qmi_util_post_request(&req_params);

        error = QMI_SIMPLE_RIL_ERR_NONE;
        }
    else
        {
        error = QMI_SIMPLE_RIL_ERR_INVALID_CONTEXT;
        }
    if (QMI_SIMPLE_RIL_ERR_NONE != error)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, error);
        }
    return 0;
    }


int qmi_simple_ril_voice_hold_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    return qmi_simple_ril_do_voice_manage_calls(cmd_params, ack_info, 0x03); // HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD
    }

int qmi_simple_ril_voice_resume_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    return qmi_simple_ril_do_voice_manage_calls(cmd_params, ack_info, 0x04); // HOLD_ALL_EXCEPT_SPECIFIED_CALL
    }


char* qmi_simple_ril_voice_get_call_state_str(int state)
    {
    char* res;
    switch (state)
        {
        case CALL_STATE_ORIGINATING_V02: 
            res = "ORIGINATION";
            break;

        case CALL_STATE_INCOMING_V02: 
            res = "INCOMING";
            break;

        case CALL_STATE_CONVERSATION_V02: 
            res = "CONVERSATION";
            break;

        case CALL_STATE_CC_IN_PROGRESS_V02: 
            res = "CC_IN_PROGRESS";
            break;

        case CALL_STATE_ALERTING_V02: 
            res = "ALERTING";
            break;

        case CALL_STATE_HOLD_V02: 
            res = "HOLD";
            break;

        case CALL_STATE_WAITING_V02: 
            res = "WAITING";
            break;

        case CALL_STATE_DISCONNECTING_V02: 
            res = "DISCONNECTING";
            break;

        case CALL_STATE_END_V02: 
            res = "END";
            break;

        case CALL_STATE_SETUP_V02: 
            res = "SETUP";
            break;

        default:
            res = "UNKNOWN";
            break;
        }
    return res;
    }

int qmi_simple_ril_voice_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    qmi_util_logln1("qmi_simple_ril_voice_unsolicited_qmi_message_handler", unsolicited_msg_info->message_specific_payload_len);
    voice_all_call_status_ind_msg_v02* call_info_msg;
    char call_state_buf[32 + 20]; //extra 20 characters for formatting the output
    char * call_state_str;
    uint32_t idx;

    if (unsolicited_msg_info->message_specific_payload_len > 0 && NULL != unsolicited_msg_info->message_specific_payload)
        {
        call_info_msg = (voice_all_call_status_ind_msg_v02*)unsolicited_msg_info->message_specific_payload;
        qmi_util_logln1("qmi_simple_ril_voice_unsolicited_qmi_message_handler", call_info_msg->call_info_len);
    
        simple_ril_voice_svc_info.ringing_call_id = 0;
    
        switch (call_info_msg->call_info_len)
            {
            case 0: // no calls, no output
                simple_ril_voice_svc_info.ringing_call_id = 0;
                simple_ril_voice_svc_info.existing_call_id = 0;
                qmi_simple_ril_core_set_cond_var("call_state", "NONE");
                break;
    
            case 1: // single call, simple output
                call_state_str = qmi_simple_ril_voice_get_call_state_str(call_info_msg->call_info[0].call_state);
                qmi_simple_ril_core_set_cond_var("call_state", call_state_str);
                sprintf(call_state_buf, "call state %s",call_state_str );
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, call_state_buf);
                if (0x02 == call_info_msg->call_info[0].call_state)
                    {
                    simple_ril_voice_svc_info.ringing_call_id = call_info_msg->call_info[0].call_id;
                    }
                simple_ril_voice_svc_info.existing_call_id = call_info_msg->call_info[0].call_id;
                break; 
    
            default: // multiple calls
                sprintf(call_state_buf, "voice call state update, total calls %d", call_info_msg->call_info_len);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, call_state_buf);
    
                for (idx = 0; idx < call_info_msg->call_info_len; idx++)
                    {
                    sprintf(call_state_buf, "call %d in state %s", idx + 1, qmi_simple_ril_voice_get_call_state_str(call_info_msg->call_info[0].call_state));
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, call_state_buf);
                    if (0x02 == call_info_msg->call_info[idx].call_state)
                        {
                        simple_ril_voice_svc_info.ringing_call_id = call_info_msg->call_info[idx].call_id;
                        }
                    simple_ril_voice_svc_info.existing_call_id = call_info_msg->call_info[idx].call_id;
                    }
                break;
            }
        }
    return 0;
    }


int qmi_simple_ril_send_cdma_burst_dtmf_handler (qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    voice_burst_dtmf_req_msg_v02  cdma_burst_dtmf_req_msg;

    if (cmd_params->info_set->nof_entries != 3)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&cdma_burst_dtmf_req_msg, 0, sizeof(cdma_burst_dtmf_req_msg));
        cdma_burst_dtmf_req_msg.burst_dtmf_info.call_id = 0xFF;

        strlcpy(cdma_burst_dtmf_req_msg.burst_dtmf_info.digit_buffer,
                 cmd_params->info_set->entries[0],
                sizeof(cdma_burst_dtmf_req_msg.burst_dtmf_info.digit_buffer));
        cdma_burst_dtmf_req_msg.dtmf_lengths_valid = TRUE;

        switch(atoi(cmd_params->info_set->entries[1]))
            {
            case 95:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_95MS_V02;
                break;
            case 150:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_150MS_V02;
                break;
            case 200:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_200MS_V02;
                break;
            case 250:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_250MS_V02;
                break;
            case 300:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_300MS_V02;
                break;
            case 350:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_350MS_V02;
                break;
            default:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_SMS_V02;
                break;            
            }
 
        switch(atoi(cmd_params->info_set->entries[2]))
            {
            case 60:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_60MS_V02;
                break;
            case 100:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_100MS_V02;
                break;
            case 150:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_150MS_V02;
                break;
            case 200:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_200MS_V02;
                break;
            default:
                cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_150MS_V02;
                break;            
            }
                

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_VOICE;
        req_params.message_id = QMI_VOICE_BURST_DTMF_REQ_V02;
        req_params.message_specific_payload = &cdma_burst_dtmf_req_msg;
        req_params.message_specific_payload_len = sizeof(cdma_burst_dtmf_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_send_cdma_burst_dtmf_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    char temp_char_array[100];
    voice_burst_dtmf_resp_msg_v02 *temp_dtmf_resp_msg;

    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            temp_dtmf_resp_msg = ((voice_burst_dtmf_resp_msg_v02*)(callback_msg_info->message_specific_payload));
            sprintf(temp_char_array, "\nError code for CDMA BURST DTMF - %d\n",temp_dtmf_resp_msg->resp.error);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);                        
            if(temp_dtmf_resp_msg->call_id_valid)
                {
                sprintf(temp_char_array, "\nCDMA BURST DTMF sent for call id - %d\n",temp_dtmf_resp_msg->call_id);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);                        
                }
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "[VOICE_TEST] ERROR in sending the SEND_CDMA_BURST_DTMF Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

