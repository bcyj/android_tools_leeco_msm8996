/******************************************************************************
  @file    qmi_simple_ril_ss.c
  @brief   Sample simple RIL, SS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) SS/USSD

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

#include "qmi_simple_ril_ss.h"
#include "voice_service_v02.h"

#define TEMP_ARRAY_SIZE 256
#define NUMBER_OF_INFORMATION_CLASSES 9

static char temp_char_array[TEMP_ARRAY_SIZE];

static void ss_print_get_call_waiting_info_resp(voice_get_call_waiting_resp_msg_v02* ss_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void ss_print_get_call_barring_info_resp(voice_get_call_barring_resp_msg_v02* ss_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void ss_print_get_call_forwarding_info_resp(voice_get_call_forwarding_resp_msg_v02* ss_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);

int qmi_simple_ril_show_call_waiting_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    voice_get_call_waiting_req_msg_v02 ss_call_waiting_req_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&ss_call_waiting_req_msg,0,sizeof(ss_call_waiting_req_msg));

        req_params.service_id = QMI_UTIL_SVC_VOICE;
        req_params.message_id = QMI_VOICE_GET_CALL_WAITING_REQ_V02;
        req_params.message_specific_payload = &ss_call_waiting_req_msg;
        req_params.message_specific_payload_len = sizeof(ss_call_waiting_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_show_call_waiting_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            ss_print_get_call_waiting_info_resp(((voice_get_call_waiting_resp_msg_v02*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                
        }
    else
        {
        sprintf(temp_char_array, "[SS_TEST] ERROR in sending the Get_Call_Waiting Request, ERROR CODE:%d\n",uplink_message->error_code);
        printf("SIZEEE %d\n",sizeof(voice_get_call_waiting_resp_msg_v02));
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void ss_print_get_call_waiting_info_resp(voice_get_call_waiting_resp_msg_v02* ss_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0;
    int class_info_helper=1;
    if(QMI_RESULT_SUCCESS_V01 == ss_arg_resp_msg->resp.result)
    {
        sprintf(temp_char_array, "[SS_TEST] Call Waiting Info : \n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == ss_arg_resp_msg->service_class_valid)
        {
            if(0 != ss_arg_resp_msg->service_class)
            {
                for(iter1 = 0; iter1 < NUMBER_OF_INFORMATION_CLASSES; iter1++)
                {
                    switch((ss_arg_resp_msg->service_class & class_info_helper))
                    {
                        case 0x01:
                            sprintf(temp_char_array, "[SS_TEST] VOICE Information Class has Call Waiting active\n");
                            break;
                        case 0x02:
                            sprintf(temp_char_array, "[SS_TEST] Data Information Class has Call Waiting active\n");
                            break;
                        case 0x04:
                            sprintf(temp_char_array, "[SS_TEST] FAX Information Class has Call Waiting active\n");
                            break;
                        case 0x08:
                            sprintf(temp_char_array, "[SS_TEST] SMS Information Class has Call Waiting active\n");
                            break;
                        case 0x10:
                            sprintf(temp_char_array, "[SS_TEST] DATACIRCUITSYNC Information Class has Call Waiting active\n");
                            break;
                        case 0x20:
                            sprintf(temp_char_array, "[SS_TEST] DATACIRCUITASYNC Information Class has Call Waiting active\n");
                            break;
                        case 0x40:
                            sprintf(temp_char_array, "[SS_TEST] PACKETACCESS Information Class has Call Waiting active\n");
                            break;
                        case 0x80:
                            sprintf(temp_char_array, "[SS_TEST] PADACCESS Information Class has Call Waiting active\n");
                            break;
                        default:                            
                            sprintf(temp_char_array, "");
                            break;
                    }            
                    class_info_helper<<=1;
                    if(temp_char_array[0])
                    {
                        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }
                    
                }
            }
            else
            {
                sprintf(temp_char_array, "[SS_TEST] None of the Information Classes have Call Waiting active\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }      
        }
        else
        {
           sprintf(temp_char_array, "[SS_TEST] no information provided\n");
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
    else
    {
        sprintf(temp_char_array, "[SS_TEST] ERROR in Get_Call_Waiting Response, ERROR CODE:%d\n\n",ss_arg_resp_msg->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == ss_arg_resp_msg->failure_cause_valid)
        {
           sprintf(temp_char_array, "[SS_TEST] Failure Cause : %d\n",ss_arg_resp_msg->failure_cause); 
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else
        {
            sprintf(temp_char_array, "[SS_TEST] QMI ERROR : Unknown Error\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
}

int qmi_simple_ril_show_call_barring_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    voice_get_call_barring_req_msg_v02 ss_call_barring_req_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&ss_call_barring_req_msg,0,sizeof(ss_call_barring_req_msg));
        ss_call_barring_req_msg.reason=0x0A; //BARR_ALLINCOMING

        req_params.service_id = QMI_UTIL_SVC_VOICE;
        req_params.message_id = QMI_VOICE_GET_CALL_BARRING_REQ_V02;
        req_params.message_specific_payload = &ss_call_barring_req_msg;
        req_params.message_specific_payload_len = sizeof(ss_call_barring_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_show_call_barring_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            ss_print_get_call_barring_info_resp(((voice_get_call_barring_resp_msg_v02*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                        
        }
    else
        {
        sprintf(temp_char_array, "[SS_TEST] ERROR in sending the Get_Call_Barring Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void ss_print_get_call_barring_info_resp(voice_get_call_barring_resp_msg_v02* ss_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0,class_info_helper=1;
    if(QMI_RESULT_SUCCESS_V01 == ss_arg_resp_msg->resp.result)
    {
        sprintf(temp_char_array, "[SS_TEST] Call Barring Info : \n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == ss_arg_resp_msg->service_class_valid)
        {
            if(0 != ss_arg_resp_msg->service_class)
            {
                for(iter1 = 0; iter1 < NUMBER_OF_INFORMATION_CLASSES; iter1++)
                {
                    switch((ss_arg_resp_msg->service_class & class_info_helper))
                    {
                        case 0x01:
                            sprintf(temp_char_array, "[SS_TEST] VOICE Information Class has Call Barring active\n");
                            break;
                        case 0x02:
                            sprintf(temp_char_array, "[SS_TEST] Data Information Class has Call Barring active\n");
                            break;
                        case 0x04:
                            sprintf(temp_char_array, "[SS_TEST] FAX Information Class has Call Barring active\n");
                            break;
                        case 0x08:
                            sprintf(temp_char_array, "[SS_TEST] SMS Information Class has Call Barring active\n");
                            break;
                        case 0x10:
                            sprintf(temp_char_array, "[SS_TEST] DATACIRCUITSYNC Information Class has Call Barring active\n");
                            break;
                        case 0x20:
                            sprintf(temp_char_array, "[SS_TEST] DATACIRCUITASYNC Information Class has Call Barring active\n");
                            break;
                        case 0x40:
                            sprintf(temp_char_array, "[SS_TEST] PACKETACCESS Information Class has Call Barring active\n");
                            break;
                        case 0x80:
                            sprintf(temp_char_array, "[SS_TEST] PADACCESS Information Class has Call Barring active\n");
                            break;
                        default:
                            sprintf(temp_char_array, "");
                            break;
                    }            
                    class_info_helper<<=1;
                    if(temp_char_array[0])
                    {
                        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }
                }
            }
            else
            {
                sprintf(temp_char_array, "[SS_TEST] None of the Information Classes have Call Barring active\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }      
        }
        else
        {
           sprintf(temp_char_array, "[SS_TEST] no service class info provided \n");
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
    else
    {
        sprintf(temp_char_array, "[SS_TEST] ERROR in Get_Call_Barring Response, ERROR CODE:%d\n",ss_arg_resp_msg->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == ss_arg_resp_msg->failure_cause_valid)
        {
           sprintf(temp_char_array, "[SS_TEST] Failure Cause : %d\n",ss_arg_resp_msg->failure_cause); 
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else
        {
            sprintf(temp_char_array, "[SS_TEST] no failure cause provided\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
}

int qmi_simple_ril_show_call_forwarding_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    voice_get_call_forwarding_req_msg_v02 ss_call_forwarding_req_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&ss_call_forwarding_req_msg,0,sizeof(ss_call_forwarding_req_msg));
        ss_call_forwarding_req_msg.reason=0x04; //FWDREASON_UNREACHABLE

        req_params.service_id = QMI_UTIL_SVC_VOICE;
        req_params.message_id = QMI_VOICE_GET_CALL_FORWARDING_REQ_V02;
        req_params.message_specific_payload = &ss_call_forwarding_req_msg;
        req_params.message_specific_payload_len = sizeof(ss_call_forwarding_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_show_call_forwarding_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            ss_print_get_call_forwarding_info_resp(((voice_get_call_forwarding_resp_msg_v02*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                
        }
    else
        {
        sprintf(temp_char_array, "[SS_TEST] ERROR in sending the Get_Call_Forwarding Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void ss_print_get_call_forwarding_info_resp(voice_get_call_forwarding_resp_msg_v02* ss_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0,iter2=0,class_info_helper=1;
    if(QMI_RESULT_SUCCESS_V01 == ss_arg_resp_msg->resp.result)
    {
        sprintf(temp_char_array, "[SS_TEST] Call Forwarding Info : \n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == ss_arg_resp_msg->get_call_forwarding_info_valid)
        {
            if(GET_CALL_FORWARDING_INFO_MAX_V02 >= ss_arg_resp_msg->get_call_forwarding_info_len)
            {
                for(iter1 = 0; iter1 < ss_arg_resp_msg->get_call_forwarding_info_len; iter1++)
                {
                    class_info_helper=1;
                    sprintf(temp_char_array, "[SS_TEST] Call Forwarding Info for number-%s : \n",ss_arg_resp_msg->get_call_forwarding_info[iter1].number);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    if(TRUE == ss_arg_resp_msg->get_call_forwarding_info[iter1].service_status)
                    {
                        for(iter2 = 0; iter2 < NUMBER_OF_INFORMATION_CLASSES; iter2++)
                        {
                            switch((ss_arg_resp_msg->get_call_forwarding_info[iter1].service_class & class_info_helper))
                            {
                                case 0x01:
                                    sprintf(temp_char_array, "[SS_TEST] VOICE Information Class has Call Forwarding active\n");
                                    break;
                                case 0x02:
                                    sprintf(temp_char_array, "[SS_TEST] Data Information Class has Call Forwarding active\n");
                                    break;
                                case 0x04:
                                    sprintf(temp_char_array, "[SS_TEST] FAX Information Class has Call Forwarding active\n");
                                    break;
                                case 0x08:
                                    sprintf(temp_char_array, "[SS_TEST] SMS Information Class has Call Forwarding active\n");
                                    break;
                                case 0x10:
                                    sprintf(temp_char_array, "[SS_TEST] DATACIRCUITSYNC Information Class has Call Forwarding active\n");
                                    break;
                                case 0x20:
                                    sprintf(temp_char_array, "[SS_TEST] DATACIRCUITASYNC Information Class has Call Forwarding active\n");
                                    break;
                                case 0x40:
                                    sprintf(temp_char_array, "[SS_TEST] PACKETACCESS Information Class has Call Forwarding active\n");
                                    break;
                                case 0x80:
                                    sprintf(temp_char_array, "[SS_TEST] PADACCESS Information Class has Call Forwarding active\n");
                                    break;
                                default:
                                    sprintf(temp_char_array, "");
                                    break;
                            }            
                            class_info_helper<<=1;
                            if(temp_char_array[0])
                            {
                                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                            }
                        }
                        
                    }
                    else
                    {
                        sprintf(temp_char_array, "[SS_TEST] None of the Information Classes have Call Forwarding active for number-%s\n",ss_arg_resp_msg->get_call_forwarding_info[iter1].number);
                        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }
                }
            }
            else
            {
                sprintf(temp_char_array, "[SS_TEST] QMI ERROR : Invalid Length\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
        }
        else
        {
           sprintf(temp_char_array, "[SS_TEST] no information provided for SS status\n");
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
    else
    {
        sprintf(temp_char_array, "[SS_TEST] ERROR in Get_Call_Forwarding Response, ERROR CODE:%d\n",ss_arg_resp_msg->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == ss_arg_resp_msg->failure_cause_valid)
        {
           sprintf(temp_char_array, "[SS_TEST] Failure Cause : %d\n",ss_arg_resp_msg->failure_cause); 
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else
        {
            sprintf(temp_char_array, "[SS_TEST] no faulire cause provided \n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
}

int qmi_simple_ril_ss_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    sprintf(temp_char_array, "******************Received SS Indication****************\n");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    return 0;
}




