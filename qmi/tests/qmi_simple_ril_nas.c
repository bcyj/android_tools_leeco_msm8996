/******************************************************************************
  @file    qmi_simple_ril_nas.c
  @brief   Sample simple RIL, NAS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) NW reg

  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

#include "qmi_simple_ril_nas.h"


#include "network_access_service_v01.h"
#include "device_management_service_v01.h"
#include "specific_absorption_rate_v01.h"

#define TEMP_ARRAY_SIZE 256 + 50 //extra 50 characters for output formatting
#define NUM_MODE_PREF 22
#define NUM_ACQ_ORDER 3
#define SUMMARY_TEXT_LEN 160

static int nas_wcdma_networks_found[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];
static unsigned int nas_wcdma_networks_found_number=0;
static char temp_char_array[TEMP_ARRAY_SIZE];

static qmi_util_request_id init_fetch_sys_info_req_id;

static void nas_cdma_print_serving_info_resp(nas_get_serving_system_resp_msg_v01* nas_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void nas_wcdma_print_network_scan_resp(nas_perform_network_scan_resp_msg_v01* nas_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void nas_print_serving_system_ind(nas_serving_system_ind_msg_v01* nas_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message);

static char* nas_tech_names[] = 
{
    "unknown",
    "GSM",
    "WCDMA",
    "LTE",
    "CDMA",
    "HDR",
    "TDSCDMA"
};

int qmi_simple_ril_show_cdma_ntw_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01;
        req_params.message_specific_payload = NULL;
        req_params.message_specific_payload_len = 0;

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_show_cdma_ntw_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            nas_cdma_print_serving_info_resp(((nas_get_serving_system_resp_msg_v01*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                        
        }
    else
        {
        sprintf(temp_char_array, "[NAS_TEST] ERROR in sending the Get_Serving_Info Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void nas_cdma_print_serving_info_resp(nas_get_serving_system_resp_msg_v01* nas_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0;
    if(QMI_RESULT_SUCCESS_V01 == nas_arg_resp_msg->resp.result)
    {
        sprintf(temp_char_array, "\n[NAS_TEST] Serving Info : \n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, "[NAS_TEST] Registration State : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        switch(nas_arg_resp_msg->serving_system.registration_state)
        {
            case 0: //QMI_NAS_NOT_REGISTERED
                sprintf(temp_char_array, "Not registered-mobile is not currently searching for a new network to provide service\n");
                break;
            case 1: //QMI_NAS_REGISTERED
                sprintf(temp_char_array, "Registered with a network\n");
                break;
            case 2: //QMI_NAS_NOT_REGISTERED_SEARCHING
                sprintf(temp_char_array, "Registration denied by the visible network \n");
                break;
            case 3: //QMI_NAS_REGISTRATION_DENIED
                sprintf(temp_char_array, "Registered with a network\n");
                break;
            case 4: //QMI_NAS_REGISTRATION_UNKNOWN
                sprintf(temp_char_array, "Registration state is unknown\n");
                break;
            default:
                sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
        }
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

        sprintf(temp_char_array, "[NAS_TEST] Circuit switch domain attach state : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        switch(nas_arg_resp_msg->serving_system.cs_attach_state)
        {
            case 0:
                sprintf(temp_char_array, "Unknown\n");
                break;
            case 1:
                sprintf(temp_char_array, "Attached\n");
                break;
            case 2:
                sprintf(temp_char_array, "Detached\n");
                break;
            default:
                sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
        }
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

        sprintf(temp_char_array, "[NAS_TEST] Packet switch domain attach state : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        switch(nas_arg_resp_msg->serving_system.ps_attach_state)
        {
            case 0:
                sprintf(temp_char_array, "Unknown\n");
                break;
            case 1:
                sprintf(temp_char_array, "Attached\n");
                break;
            case 2:
                sprintf(temp_char_array, "Detached\n");
                break;
            default:
                sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
        }
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            
        sprintf(temp_char_array, "[NAS_TEST] Registered Network : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        switch(nas_arg_resp_msg->serving_system.selected_network)
        {
            case 0:
                sprintf(temp_char_array, "Unknown\n");
                break;
            case 1:
                sprintf(temp_char_array, "3GPP2 network\n");
                break;
            case 2:
                sprintf(temp_char_array, "3GPP network\n");
                break;
            default:
                sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
        }
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

        sprintf(temp_char_array, "[NAS_TEST] Radio Interface(s) currently in use : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(nas_arg_resp_msg->serving_system.radio_if_len <= NAS_RADIO_IF_LIST_MAX_V01)
        {
            for(iter1 = 0; iter1 < nas_arg_resp_msg->serving_system.radio_if_len; iter1++)
            {
                switch(nas_arg_resp_msg->serving_system.radio_if[iter1])
                {
                    case 0:
                        sprintf(temp_char_array, "No Service, ");
                        break;
                    case 1:
                        sprintf(temp_char_array, "cdma2000 1X, ");
                        break;
                    case 2:
                        sprintf(temp_char_array, "cdma2000 HRPD (1xEV-DO), ");
                        break;
                    case 3:
                        sprintf(temp_char_array, "AMPS, ");
                        break;
                    case 4:
                        sprintf(temp_char_array, "GSM, ");
                        break;
                    case 5:
                        sprintf(temp_char_array, "UMTS, ");
                        break;
                    default:
                        sprintf(temp_char_array, "QMI ERROR : Invalid Value");
                }            
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            sprintf(temp_char_array, "\n\n");               
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else
        {
           sprintf(temp_char_array, "QMI ERROR : Invalid Length\n");
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }

        if(TRUE == nas_arg_resp_msg->detailed_service_info_valid)
        {
            sprintf(temp_char_array, " [NAS_TEST] Detailed Service Info : ");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] Service status : %d\n", nas_arg_resp_msg->detailed_service_info.srv_status);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] Service capability : %d\n", nas_arg_resp_msg->detailed_service_info.srv_capability);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] HDR Service status : %d\n", nas_arg_resp_msg->detailed_service_info.hdr_srv_status);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] HDR hybrid : %d\n", nas_arg_resp_msg->detailed_service_info.hdr_hybrid);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] Is system forbidden : %d\n", nas_arg_resp_msg->detailed_service_info.is_sys_forbidden);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }

        if(TRUE == nas_arg_resp_msg->cdma_system_id_ext_valid) 
        {
            sprintf(temp_char_array, " [NAS_TEST] CDMA System Info : ");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] MCC : %d\n", nas_arg_resp_msg->cdma_system_id_ext.mcc);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] IMSI 11 12 : %d\n", nas_arg_resp_msg->cdma_system_id_ext.imsi_11_12);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        if(TRUE == nas_arg_resp_msg->nas_3gpp_time_zone_valid)
        {
            sprintf(temp_char_array, " [NAS_TEST] 3GPP Time Zone Info: ");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] Leap seconds since start of CDMA time : %d\n", nas_arg_resp_msg->nas_3gpp_time_zone.lp_sec);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] Offset (2's complement) in 30 mins increment : %d\n", nas_arg_resp_msg->nas_3gpp_time_zone.ltm_offset);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, " [NAS_TEST] Daylight savings (1 = ON, 0 = OFF) : %d\n", nas_arg_resp_msg->nas_3gpp_time_zone.daylt_savings);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
    else
    {
        sprintf(temp_char_array, "[NAS_TEST] ERROR in Get_Serving_Info Response, ERROR CODE:%d",nas_arg_resp_msg->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
}

int qmi_simple_ril_perform_network_scan_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01;
        req_params.message_specific_payload = NULL;
        req_params.message_specific_payload_len = 0;

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_perform_network_scan_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if(0 != nas_wcdma_networks_found_number)
        {
        memset(nas_wcdma_networks_found,0,sizeof(nas_wcdma_networks_found));
        nas_wcdma_networks_found_number=0;
        }
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            nas_wcdma_print_network_scan_resp(((nas_perform_network_scan_resp_msg_v01*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "[NAS_TEST] ERROR in sending the Get_Serving_Info Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void nas_wcdma_print_network_scan_resp(nas_perform_network_scan_resp_msg_v01* nas_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0,network_status=0,user_choice=0;
    if(QMI_RESULT_SUCCESS_V01 == nas_arg_resp_msg->resp.result)
    {
        if(TRUE == nas_arg_resp_msg->nas_3gpp_network_info_valid)
        {
            sprintf(temp_char_array, "\n[NAS_TEST] Network Scan Info - 3GPP Network(s) present: \n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            if(nas_arg_resp_msg->nas_3gpp_network_info_len <= NAS_3GPP_NETWORK_INFO_LIST_MAX_V01)
            {

                nas_wcdma_networks_found_number = nas_arg_resp_msg->nas_3gpp_network_info_len;                
                for(iter1 = 0; iter1 < nas_wcdma_networks_found_number; iter1++)
                {
                    nas_wcdma_networks_found[iter1]=nas_arg_resp_msg->nas_3gpp_network_info[iter1].mobile_country_code;
                    nas_wcdma_networks_found[iter1]|=(nas_arg_resp_msg->nas_3gpp_network_info[iter1].mobile_network_code<<16);
                    sprintf(temp_char_array, "[NAS_TEST] Network ID - %d :\n",iter1+1);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    sprintf(temp_char_array, "[NAS_TEST] Description : %s\n",nas_arg_resp_msg->nas_3gpp_network_info[iter1].network_description);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    sprintf(temp_char_array, "[NAS_TEST] Country Code : %d\n",nas_arg_resp_msg->nas_3gpp_network_info[iter1].mobile_country_code);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    sprintf(temp_char_array, "[NAS_TEST] Network Code : %d\n",nas_arg_resp_msg->nas_3gpp_network_info[iter1].mobile_network_code);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    
                    network_status = (nas_arg_resp_msg->nas_3gpp_network_info[iter1].network_status) & 3;                    
                    sprintf(temp_char_array, "[NAS_TEST] In-use status : ");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    switch(network_status)
                    {
                        case 0:
                            sprintf(temp_char_array, "Unknown\n");
                            break;
                        case 1:
                            sprintf(temp_char_array, "Current Serving\n");
                            break;
                        case 2:
                            sprintf(temp_char_array, "Available\n");
                            break;
                        default:
                            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
                    }            
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    network_status = (nas_arg_resp_msg->nas_3gpp_network_info[iter1].network_status >> 2) & 3;                    
                    sprintf(temp_char_array, "[NAS_TEST] Roaming status : ");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    switch(network_status)
                    {
                        case 0:
                            sprintf(temp_char_array, "Unknown\n");
                            break;
                        case 1:
                            sprintf(temp_char_array, "Home\n");
                            break;
                        case 2:
                            sprintf(temp_char_array, "Roam\n");
                            break;
                        default:
                            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
                    }            
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    network_status = (nas_arg_resp_msg->nas_3gpp_network_info[iter1].network_status >> 4) & 3;                    
                    sprintf(temp_char_array, "[NAS_TEST] Forbidden status : ");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    switch(network_status)
                    {
                        case 0:
                            sprintf(temp_char_array, "Unknown\n");
                            break;
                        case 1:
                            sprintf(temp_char_array, "Forbidden\n");
                            break;
                        case 2:
                            sprintf(temp_char_array, "Not Forbidden\n");
                            break;
                        default:
                            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
                    }            
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    network_status = (nas_arg_resp_msg->nas_3gpp_network_info[iter1].network_status >> 6) & 3;                    
                    sprintf(temp_char_array, "[NAS_TEST] Preferred status : ");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    switch(network_status)
                    {
                        case 0:
                            sprintf(temp_char_array, "Unknown\n");
                            break;
                        case 1:
                            sprintf(temp_char_array, "Preferred\n");
                            break;
                        case 2:
                            sprintf(temp_char_array, "Not Preferred\n");
                            break;
                        default:
                            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
                    }            
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    sprintf(temp_char_array, "----------------------\n");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
            else
            {
               sprintf(temp_char_array, "QMI ERROR : Invalid Length\n");
               qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
        }
        else
        {
            sprintf(temp_char_array, "[NAS_TEST] No Info\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
    else
    {
        sprintf(temp_char_array, "[NAS_TEST] ERROR in Perform_Network_Scan Response, ERROR CODE:%d",nas_arg_resp_msg->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
    sprintf(temp_char_array, "\n");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
}

int qmi_simple_ril_initiate_network_auto_register_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    nas_initiate_network_register_req_msg_v01 nas_network_register_req_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&nas_network_register_req_msg,0,sizeof(nas_network_register_req_msg));
        nas_network_register_req_msg.register_action=1;

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_INITIATE_NETWORK_REGISTER_REQ_MSG_V01;
        req_params.message_specific_payload = &nas_network_register_req_msg;
        req_params.message_specific_payload_len = sizeof(nas_network_register_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_initiate_network_manual_register_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    nas_initiate_network_register_req_msg_v01 nas_network_register_req_msg;

    unsigned int nas_wcdma_user_network = atoi(cmd_params->info_set->entries[0])-1;
    unsigned int nas_wcdma_user_radio_tech = atoi(cmd_params->info_set->entries[1])+3;


    if (cmd_params->info_set->nof_entries != 2 || (nas_wcdma_user_network >= nas_wcdma_networks_found_number))
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&nas_network_register_req_msg,0,sizeof(nas_network_register_req_msg));
        nas_network_register_req_msg.register_action=2;
        nas_network_register_req_msg.manual_network_register_info_valid=TRUE;
        nas_network_register_req_msg.manual_network_register_info.mobile_country_code=nas_wcdma_networks_found[nas_wcdma_user_network]&((1<<16)-1);
        nas_network_register_req_msg.manual_network_register_info.mobile_network_code=(nas_wcdma_networks_found[nas_wcdma_user_network]>>16)&((1<<16)-1);
        nas_network_register_req_msg.manual_network_register_info.radio_access_technology=nas_wcdma_user_radio_tech;
        
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_INITIATE_NETWORK_REGISTER_REQ_MSG_V01;
        req_params.message_specific_payload = &nas_network_register_req_msg;
        req_params.message_specific_payload_len = sizeof(nas_network_register_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}
int qmi_simple_ril_set_3gpp2_subscription_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    nas_set_3gpp2_subscription_info_req_msg_v01 req_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_msg,0,sizeof(req_msg));
        memset(&req_params, 0, sizeof(req_params));
        req_msg.nam_id = 0;
        req_msg.cdma_sys_id_valid = FALSE;
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01;
        req_params.message_specific_payload = &req_msg;
        req_params.message_specific_payload_len = sizeof(req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}
int qmi_simple_ril_mode_pref_handler (qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
  qmi_util_request_params req_params;
  nas_set_system_selection_preference_req_msg_v01 req_msg;
  char* arg;
  int mode_pref;
  int err = QMI_SIMPLE_RIL_ERR_NONE;

  if (cmd_params->info_set->nof_entries >= 1)
  {
    arg = cmd_params->info_set->entries[0];
    if (0 == strcmp("help", arg) )
    {
      qmi_simple_ril_show_mode_pref();
    }
    else if (0 == strcmp("set", arg) )
    {
      if (cmd_params->info_set->nof_entries >= 2)
      {
        memset(&req_msg, 0, sizeof(req_msg));
        req_msg.mode_pref_valid =  TRUE;
        mode_pref = atoi(cmd_params->info_set->entries[1]);
        req_msg.mode_pref = mode_pref;
        qmi_util_logln1("req_msg.mode_pref: ", req_msg.mode_pref);
        if (cmd_params->info_set->nof_entries == 3)
        {
          arg = cmd_params->info_set->entries[2];
          req_msg.gw_acq_order_pref_valid = TRUE;
          req_msg.gw_acq_order_pref = atoi(arg);
        }
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01;
        req_params.message_specific_payload = &req_msg;
        req_params.message_specific_payload_len = sizeof(req_msg);
        ack_info->request_id = qmi_util_post_request(&req_params);
      }
      else
      {
        err = QMI_SIMPLE_RIL_ERR_ARG;
      }
    }
    else if (0 == strcmp("get", arg) )
    {
        req_params.service_id = QMI_UTIL_SVC_NAS;
        req_params.message_id = QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01;
        req_params.message_specific_payload = NULL;
        req_params.message_specific_payload_len = 0;
        ack_info->request_id = qmi_util_post_request(&req_params);
    }
    else
    {
      err = QMI_SIMPLE_RIL_ERR_ARG;
    }
  }
  else
  {
    err = QMI_SIMPLE_RIL_ERR_ARG;
  }
  if (err == QMI_SIMPLE_RIL_ERR_ARG)
  {
    qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
  }
  return 0;
}
void qmi_simple_ril_show_mode_pref()
{
  char *mode_pref[NUM_MODE_PREF] = {"CDMA -----------------------------> 1",
                                  "HRPD -----------------------------> 2",
                                  "CDMA,HRPD ------------------------> 3",
                                  "GSM ------------------------------> 4",
                                  "UMTS -----------------------------> 8",
                                  "LTE ------------------------------> 10",
                                  "GSM,UMTS -------------------------> 12",
                                  "CDMA,HRPD,LTE --------------------> 13",
                                  "GSM,UMTS,CDMA,HRPD ---------------> 15",
                                  "UMTS,LTE -------------------------> 18",
                                  "TDSCDMA --------------------------> 20",
                                  "GSM,UMTS,LTE ---------------------> 22",
                                  "GSM,TDSCDMA ----------------------> 24",
                                  "GSM,UMTS,CDMA,HRPD,LTE -----------> 25",
                                  "UMTS,TDSCDMA ---------------------> 28",
                                  "TDSCDMA,LTE ----------------------> 30",
                                  "GSM,UMTS,TDSCDMA -----------------> 32",
                                  "GSM,TDSCDMA,LTE ------------------> 34",
                                  "GSM,TDSCDMA,CDMA,HRPD,UMTS -------> 35",
                                  "UMTS,TDSCDMA,LTE -----------------> 38",
                                  "GSM,UMTS,TDSCDMA,LTE -------------> 42",
                                  "GSM,UMTS,TDSCDMA,CDMA,HRPD,LTE ---> 45"};

  char *acq_order[NUM_ACQ_ORDER] = {"GW Acq Order AUTOMATIC --------------> 0",
                                    "GW Acq Order GSM,WCDMA --------------> 1",
                                    "GW Acq Order WCDMA,GSM --------------> 2"};

  int i;
  for (i = 0; i < NUM_MODE_PREF; i++)
  {
    printf ("%s\n", mode_pref[i]);
  }
  printf("==================================\n");
  for (i = 0; i < NUM_ACQ_ORDER; i++)
  {
    printf ("%s\n", acq_order[i]);
  }
}

int qmi_simple_ril_set_mode_pref_callback (qmi_util_service_message_info* qmi_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
  nas_set_system_selection_preference_resp_msg_v01 * set_sys_sel_pref_resp;
  if (qmi_msg_info->error_code == QMI_NO_ERR)
  {
    if(NULL != qmi_msg_info->message_specific_payload)
    {
      set_sys_sel_pref_resp = (nas_set_system_selection_preference_resp_msg_v01*) qmi_msg_info->message_specific_payload;
      if (set_sys_sel_pref_resp->resp.result == QMI_RESULT_SUCCESS_V01)
      {
        snprintf(temp_char_array, sizeof(temp_char_array), "%s", "Set mode pref returned success\n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
      }
      else
      {
        snprintf(temp_char_array, sizeof(temp_char_array), "Set mode pref returned error code: %d\n", set_sys_sel_pref_resp->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
      }
    }
  }
  else
  {
    snprintf(temp_char_array, sizeof(temp_char_array), "ERROR in sending Set mode pref request, error code :%d\n",qmi_msg_info->error_code);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
  }
  return 0;
}

int qmi_simple_ril_get_mode_pref_resp_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
  nas_get_system_selection_preference_resp_msg_v01* get_mode_pref_resp_msg;

  if(NULL != unsolicited_msg_info->message_specific_payload)
  {
    get_mode_pref_resp_msg = (nas_get_system_selection_preference_resp_msg_v01*)unsolicited_msg_info->message_specific_payload;

    if ( get_mode_pref_resp_msg->mode_pref_valid )
    {
      snprintf(temp_char_array, sizeof(temp_char_array), "Mode pref :%d\n",(int)get_mode_pref_resp_msg->mode_pref);
      qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
  }
  else
  {
    snprintf(temp_char_array, sizeof(temp_char_array), "%s", "Unable to fetch mode pref");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
  }
  return 0;
}
int qmi_simple_ril_nas_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if(NULL != unsolicited_msg_info->message_specific_payload)
        {
        nas_print_serving_system_ind(((nas_serving_system_ind_msg_v01*) unsolicited_msg_info->message_specific_payload),uplink_message);
        }
    else
        {
        sprintf(temp_char_array, "\nIndication has no message payload\n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
        }                                                    
    return 0;
}

int qmi_simple_ril_set_3gpp2_subs_callback (qmi_util_service_message_info* qmi_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    nas_set_3gpp2_subscription_info_resp_msg_v01* set_3gpp2_subs_resp;
    if (qmi_msg_info->error_code == QMI_NO_ERR)
        {
        if(NULL != qmi_msg_info->message_specific_payload)
            {
            set_3gpp2_subs_resp = (nas_set_3gpp2_subscription_info_resp_msg_v01*) qmi_msg_info->message_specific_payload;
            if (set_3gpp2_subs_resp->resp.result == QMI_RESULT_SUCCESS_V01)
                {
                sprintf(temp_char_array, "[NAS_Test] Set Service Status returned success\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            else
                {
                sprintf(temp_char_array, "[NAS_Test] Set Service Status returned error code: %d\n", set_3gpp2_subs_resp->resp.error);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
        else
            {
            sprintf(temp_char_array, "[NAS_Test] Set Service status callback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "[NAS_TEST] ERROR in sending Set Service Status request, error code :%d\n",qmi_msg_info->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void nas_print_serving_system_ind(nas_serving_system_ind_msg_v01* nas_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0;
    sprintf(temp_char_array, "\n[NAS_TEST] Serving Info : \n");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[NAS_TEST] Registration State : ");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    switch(nas_arg_ind_msg->serving_system.registration_state)
    {
        case 0: //QMI_NAS_NOT_REGISTERED
            sprintf(temp_char_array, "Not registered-mobile is not currently searching for a new network to provide service\n");
            break;
        case 1: //QMI_NAS_REGISTERED
            sprintf(temp_char_array, "Registered with a network\n");
            break;
        case 2: //QMI_NAS_NOT_REGISTERED_SEARCHING
            sprintf(temp_char_array, "Registration denied by the visible network \n");
            break;
        case 3: //QMI_NAS_REGISTRATION_DENIED
            sprintf(temp_char_array, "Registered with a network\n");
            break;
        case 4: //QMI_NAS_REGISTRATION_UNKNOWN
            sprintf(temp_char_array, "Registration state is unknown\n");
            break;
        default:
            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
    }
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

    sprintf(temp_char_array, "[NAS_TEST] Circuit switch domain attach state : ");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    switch(nas_arg_ind_msg->serving_system.cs_attach_state)
    {
        case 0:
            sprintf(temp_char_array, "Unknown\n");
            break;
        case 1:
            sprintf(temp_char_array, "Attached\n");
            break;
        case 2:
            sprintf(temp_char_array, "Detached\n");
            break;
        default:
            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
    }
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

    sprintf(temp_char_array, "[NAS_TEST] Packet switch domain attach state : ");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    switch(nas_arg_ind_msg->serving_system.ps_attach_state)
    {
        case 0:
            sprintf(temp_char_array, "Unknown\n");
            break;
        case 1:
            sprintf(temp_char_array, "Attached\n");
            break;
        case 2:
            sprintf(temp_char_array, "Detached\n");
            break;
        default:
            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
    }
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        
    sprintf(temp_char_array, "[NAS_TEST] Registered Network : ");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    switch(nas_arg_ind_msg->serving_system.selected_network)
    {
        case 0:
            sprintf(temp_char_array, "Unknown\n");
            break;
        case 1:
            sprintf(temp_char_array, "3GPP2 network\n");
            break;
        case 2:
            sprintf(temp_char_array, "3GPP network\n");
            break;
        default:
            sprintf(temp_char_array, "QMI ERROR : Invalid Value\n");
    }
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    
    sprintf(temp_char_array, "[NAS_TEST] Radio Interface(s) currently in use : ");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    if(nas_arg_ind_msg->serving_system.radio_if_len <= NAS_RADIO_IF_LIST_MAX_V01)
    {
        for(iter1 = 0; iter1 < nas_arg_ind_msg->serving_system.radio_if_len; iter1++)
        {
            switch(nas_arg_ind_msg->serving_system.radio_if[iter1])
            {
                case 0:
                    sprintf(temp_char_array, "No Service, ");
                    break;
                case 1:
                    sprintf(temp_char_array, "cdma2000 1X, ");
                    break;
                case 2:
                    sprintf(temp_char_array, "cdma2000 HRPD (1xEV-DO), ");
                    break;
                case 3:
                    sprintf(temp_char_array, "AMPS, ");
                    break;
                case 4:
                    sprintf(temp_char_array, "GSM, ");
                    break;
                case 5:
                    sprintf(temp_char_array, "UMTS, ");
                    break;
                default:
                    sprintf(temp_char_array, "QMI ERROR : Invalid Value");
            }            
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        sprintf(temp_char_array, "\n");               
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
    else
    {
       sprintf(temp_char_array, "QMI ERROR : Invalid Length\n");
       qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }

    if(TRUE == nas_arg_ind_msg->data_capabilities_valid)
    {
        sprintf(temp_char_array, "[NAS_TEST] Data Capabilities supported : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(nas_arg_ind_msg->data_capabilities_len <= NAS_DATA_CAPABILITIES_LIST_MAX_V01)
        {
            for(iter1 = 0; iter1 < nas_arg_ind_msg->data_capabilities_len; iter1++)
            {
                switch(nas_arg_ind_msg->data_capabilities[iter1])
                {
                    case 1:
                        sprintf(temp_char_array, "GPRS, ");
                        break;
                    case 2:
                        sprintf(temp_char_array, "EDGE, ");
                        break;
                    case 3:
                        sprintf(temp_char_array, "HSDPA, ");
                        break;
                    case 4:
                        sprintf(temp_char_array, "HSUPA, ");
                        break;
                    case 5:
                        sprintf(temp_char_array, "WCDMA, ");
                        break;
                    case 6:
                        sprintf(temp_char_array, "CDMA, ");
                        break;
                    case 7:
                        sprintf(temp_char_array, "EV-DO REV 0, ");
                        break;
                    case 8:
                        sprintf(temp_char_array, "EV-DO REV A, ");
                        break;
                    case 9:
                        sprintf(temp_char_array, "GSM, ");
                        break;

                    default:
                        sprintf(temp_char_array, "QMI ERROR : Invalid Value");
                }            
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            sprintf(temp_char_array, "\n");               
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else
        {
           sprintf(temp_char_array, "QMI ERROR : Invalid Length\n");
           qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }

    if(TRUE == nas_arg_ind_msg->current_plmn_valid)
    {
        sprintf(temp_char_array, "[NAS_TEST] Current PLMN : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, "[NAS_TEST] Description : %s\n",nas_arg_ind_msg->current_plmn.network_description);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, "[NAS_TEST] Country Code : %d\n",nas_arg_ind_msg->current_plmn.mobile_country_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, "[NAS_TEST] Network Code : %d\n",nas_arg_ind_msg->current_plmn.mobile_network_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
    if(TRUE == nas_arg_ind_msg->roaming_indicator_valid)
    {
        sprintf(temp_char_array, "[NAS_TEST] Roaming Indicator Information : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        switch(nas_arg_ind_msg->roaming_indicator)
        {
            case 0:
                sprintf(temp_char_array, "Roaming");
                break;
            case 1:
                sprintf(temp_char_array, "Home");
                break;
            case 2:
                sprintf(temp_char_array, "Flashing");
                break;
            default:
                sprintf(temp_char_array, "Operator Defined value");
        }
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, "\n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
    if(TRUE == nas_arg_ind_msg->detailed_service_info_valid)
    {
        sprintf(temp_char_array, " [NAS_TEST] Detailed Service Info : ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] Service status : %d\n", nas_arg_ind_msg->detailed_service_info.srv_status);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] Service capability : %d\n", nas_arg_ind_msg->detailed_service_info.srv_capability);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] HDR Service status : %d\n", nas_arg_ind_msg->detailed_service_info.hdr_srv_status);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] HDR hybrid : %d\n", nas_arg_ind_msg->detailed_service_info.hdr_hybrid);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] HDR hybrid : %d\n", nas_arg_ind_msg->detailed_service_info.is_sys_forbidden);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
    if(TRUE == nas_arg_ind_msg->nas_3gpp_time_zone_valid)
    {
        sprintf(temp_char_array, " [NAS_TEST] 3GPP Time Zone Info: ");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] Leap seconds since start of CDMA time : %d\n", nas_arg_ind_msg->nas_3gpp_time_zone.lp_sec);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] Offset (2's complement) in 30 mins increment : %d\n", nas_arg_ind_msg->nas_3gpp_time_zone.ltm_offset);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        sprintf(temp_char_array, " [NAS_TEST] Daylight savings (1 = ON, 0 = OFF) : %d\n", nas_arg_ind_msg->nas_3gpp_time_zone.daylt_savings);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
}


int qmi_simple_ril_qmi_sar_power_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    sar_rf_set_state_req_msg_v01 sar_state_req_msg;

    char str_param[256];

    qmi_util_logln1("qmi_simple_ril_qmi_sar_power_handler entry HEY", cmd_params->info_set->nof_entries );

    if (cmd_params->info_set->nof_entries != 1)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset( &sar_state_req_msg, 0, sizeof( sar_state_req_msg ) );
        
        *str_param = 0;
        strncpy(str_param, cmd_params->info_set->entries[0], sizeof(str_param) - 1);
        sar_state_req_msg.sar_rf_state = atoi( str_param );

        qmi_util_logln1s("qmi_simple_ril_qmi_sar_power_handler power param ", str_param );

        qmi_util_logln1("qmi_simple_ril_qmi_sar_power_handler power param to QMI", (int)sar_state_req_msg.sar_rf_state );

        memset( &req_params , 0, sizeof( req_params ) );

        req_params.service_id = QMI_UTIL_SVC_RF_SAR;
        req_params.message_id = QMI_SAR_RF_SET_STATE_REQ_MSG_V01;
        req_params.message_specific_payload = &sar_state_req_msg;
        req_params.message_specific_payload_len = sizeof(sar_state_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}


int qmi_simple_ril_nas_unsolicited_sys_info_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    nas_sys_info_ind_msg_v01* sys_info_ind;
    int idx;
    char full_srv_tech[128 + 1];
    char ltd_srv_tech[128 + 1];
    char summary[SUMMARY_TEXT_LEN];

    nas_service_status_enum_type_v01 reg_info[RTE_MAX];

    if(NULL != unsolicited_msg_info->message_specific_payload)
    {
        sys_info_ind = (nas_sys_info_ind_msg_v01*)unsolicited_msg_info->message_specific_payload;

        memset( reg_info, 0, sizeof(reg_info) );

        if ( sys_info_ind->cdma_srv_status_info_valid ) 
        {
            reg_info[RTE_1X] = sys_info_ind->cdma_srv_status_info.srv_status;
        }
        if ( sys_info_ind->hdr_srv_status_info_valid ) 
        {
            reg_info[RTE_HDR] = sys_info_ind->hdr_srv_status_info.srv_status;
        }
        if ( sys_info_ind->gsm_srv_status_info_valid ) 
        {
            reg_info[RTE_GSM] = sys_info_ind->gsm_srv_status_info.srv_status;
        }
        if ( sys_info_ind->wcdma_srv_status_info_valid ) 
        {
            reg_info[RTE_WCDMA] = sys_info_ind->wcdma_srv_status_info.srv_status;
        }
        if ( sys_info_ind->lte_srv_status_info_valid ) 
        {
            reg_info[RTE_LTE] = sys_info_ind->lte_srv_status_info.srv_status;
        }
        if ( sys_info_ind->tdscdma_srv_status_info_valid )
        {
            reg_info[RTE_TDSCDMA] = sys_info_ind->tdscdma_srv_status_info.srv_status;
        }


        qmi_util_logln1("ind srv: cdma", reg_info[RTE_1X] );
        qmi_util_logln1("ind srv: hdr", reg_info[RTE_HDR] );
        qmi_util_logln1("ind srv: gsm", reg_info[RTE_GSM] );
        qmi_util_logln1("ind srv: wcdma", reg_info[RTE_WCDMA] );
        qmi_util_logln1("ind srv: lte", reg_info[RTE_LTE] );
        qmi_util_logln1("ind srv: tdscdma", reg_info[RTE_TDSCDMA] );

        *full_srv_tech = 0;
        *ltd_srv_tech = 0;
        for ( idx = RTE_FIRST; idx < RTE_MAX; idx++ ) 
        {
            switch ( reg_info[idx] ) 
            {
                case NAS_SYS_SRV_STATUS_SRV_V01:
                    if (*full_srv_tech) 
                    {
                        strcat( full_srv_tech, ",");  
                    }
                    strcat( full_srv_tech, nas_tech_names[ idx ] );  
                    break;

                case NAS_SYS_SRV_STATUS_LIMITED_V01:
                case NAS_SYS_SRV_STATUS_LIMITED_REGIONAL_V01:
                    if (*ltd_srv_tech) 
                    {
                        strcat( ltd_srv_tech, ",");  
                    }
                    strcat( ltd_srv_tech, nas_tech_names[ idx ] );  
                    break;

                case NAS_SYS_SRV_STATUS_PWR_SAVE_V01:
                    break;

                default:
                    break;
            }
        }
        if  (*full_srv_tech) 
        { // full service
            snprintf(summary,SUMMARY_TEXT_LEN, "NW_REG (IN SERVICE) (%s)", full_srv_tech );
            qmi_simple_ril_core_set_cond_var("nw_reg", "FULL_SERVICE");
            qmi_simple_ril_core_set_cond_var("nw_rte", full_srv_tech );
        }
        else if(*ltd_srv_tech) 
        { // limited service
            snprintf(summary, SUMMARY_TEXT_LEN, "NW_REG (LIMITED SERVICE) (%s)", ltd_srv_tech );
            qmi_simple_ril_core_set_cond_var("nw_reg", "LIMITED_SERVICE");
            qmi_simple_ril_core_set_cond_var("nw_rte", ltd_srv_tech );
        }
        else
        { // no service
            snprintf(summary, SUMMARY_TEXT_LEN, "NW_REG (NO SERVICE) " );
            qmi_simple_ril_core_set_cond_var("nw_reg", "NO_SERVICE");
            qmi_simple_ril_core_set_cond_var("nw_rte", "" );
        }

        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, summary);

    }
    return 0;
}

int qmi_simple_ril_dms_unsolicited_event_report_ind_handler
(
    qmi_util_service_message_info* unsolicited_msg_info,
    qmi_simple_ril_cmd_completion_info* uplink_message
)
{
    dms_event_report_ind_msg_v01* dms_event_report_ind_msg;

    if(NULL != unsolicited_msg_info->message_specific_payload)
    {
        dms_event_report_ind_msg = (dms_event_report_ind_msg_v01*)unsolicited_msg_info->message_specific_payload;
        if(dms_event_report_ind_msg->operating_mode_valid)
        {
            qmi_util_logln1("dms ind : modem operating mdoe %d", dms_event_report_ind_msg->operating_mode );
            if (dms_event_report_ind_msg->operating_mode == DMS_OP_MODE_ONLINE_V01)
            {
                qmi_simple_ril_core_set_cond_var("modem_state", "ONLINE");
            }
            else if (dms_event_report_ind_msg->operating_mode == DMS_OP_MODE_LOW_POWER_V01 )
            {
                qmi_simple_ril_core_set_cond_var("modem_state", "LPM");
            }
        }
    }
    return 0;
}

int qmi_simple_ril_nas_fetch_sys_info_resp_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    nas_get_sys_info_resp_msg_v01* sys_info_resp;
    int idx;
    char full_srv_tech[128 + 1];
    char ltd_srv_tech[128 + 1];
    char summary[SUMMARY_TEXT_LEN];

    nas_service_status_enum_type_v01 reg_info[RTE_MAX];

    if(NULL != unsolicited_msg_info->message_specific_payload)
    {
        sys_info_resp = (nas_get_sys_info_resp_msg_v01*)unsolicited_msg_info->message_specific_payload;

        memset( reg_info, 0, sizeof(reg_info) );

        if ( sys_info_resp->cdma_srv_status_info_valid ) 
        {
            reg_info[RTE_1X] = sys_info_resp->cdma_srv_status_info.srv_status;
        }
        if ( sys_info_resp->hdr_srv_status_info_valid ) 
        {
            reg_info[RTE_HDR] = sys_info_resp->hdr_srv_status_info.srv_status;
        }
        if ( sys_info_resp->gsm_srv_status_info_valid ) 
        {
            reg_info[RTE_GSM] = sys_info_resp->gsm_srv_status_info.srv_status;
        }
        if ( sys_info_resp->wcdma_srv_status_info_valid ) 
        {
            reg_info[RTE_WCDMA] = sys_info_resp->wcdma_srv_status_info.srv_status;
        }
        if ( sys_info_resp->lte_srv_status_info_valid ) 
        {
            reg_info[RTE_LTE] = sys_info_resp->lte_srv_status_info.srv_status;
        }
        if ( sys_info_resp->tdscdma_srv_status_info_valid )
        {
            reg_info[RTE_TDSCDMA] = sys_info_resp->tdscdma_srv_status_info.srv_status;
        }


        qmi_util_logln1("ind srv: cdma", reg_info[RTE_1X] );
        qmi_util_logln1("ind srv: hdr", reg_info[RTE_HDR] );
        qmi_util_logln1("ind srv: gsm", reg_info[RTE_GSM] );
        qmi_util_logln1("ind srv: wcdma", reg_info[RTE_WCDMA] );
        qmi_util_logln1("ind srv: lte", reg_info[RTE_LTE] );

        *full_srv_tech = 0;
        *ltd_srv_tech = 0;
        for ( idx = RTE_FIRST; idx < RTE_MAX; idx++ ) 
        {
            switch ( reg_info[idx] ) 
            {
                case NAS_SYS_SRV_STATUS_SRV_V01:
                    if (*full_srv_tech) 
                    {
                        strcat( full_srv_tech, ",");  
                    }
                    strcat( full_srv_tech, nas_tech_names[ idx ] );  
                    break;

                case NAS_SYS_SRV_STATUS_LIMITED_V01:
                case NAS_SYS_SRV_STATUS_LIMITED_REGIONAL_V01:
                    if (*ltd_srv_tech) 
                    {
                        strcat( ltd_srv_tech, ",");  
                    }
                    strcat( ltd_srv_tech, nas_tech_names[ idx ] );  
                    break;

                case NAS_SYS_SRV_STATUS_PWR_SAVE_V01:
                    break;

                default:
                    break;
            }
        }
        if  (*full_srv_tech) 
        { // full service
            sprintf(summary, SUMMARY_TEXT_LEN,"NW_REG (IN SERVICE) (%s)", full_srv_tech );
            qmi_simple_ril_core_set_cond_var("nw_reg", "FULL_SERVICE");
            qmi_simple_ril_core_set_cond_var("nw_rte", full_srv_tech );
        }
        else if(*ltd_srv_tech) 
        { // limited service
            snprintf(summary, SUMMARY_TEXT_LEN,"NW_REG (LIMITED SERVICE) (%s)", ltd_srv_tech );
            qmi_simple_ril_core_set_cond_var("nw_reg", "LIMITED_SERVICE");
            qmi_simple_ril_core_set_cond_var("nw_rte", ltd_srv_tech );
        }
        else
        { // no service
            snprintf(summary, SUMMARY_TEXT_LEN,"NW_REG (NO SERVICE) " );
            qmi_simple_ril_core_set_cond_var("nw_reg", "NO_SERVICE");
            qmi_simple_ril_core_set_cond_var("nw_rte", "" );
        }

        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, summary);

        if (init_fetch_sys_info_req_id == uplink_message->request_id) 
        {
            qmi_util_logln1("qmi_simple_ril_nas_fetch_sys_info_resp_handler bypassing report as init driven", init_fetch_sys_info_req_id );
            uplink_message->must_be_silent = TRUE;
        }

    }
    return 0;
}

void qmi_simple_ril_dms_init()
{
    dms_set_event_report_req_msg_v01    req;
    qmi_util_request_params             req_params;

    memset( &req, 0, sizeof( req ) );

    req.report_oprt_mode_state_valid = TRUE;
    req.report_oprt_mode_state       = TRUE;

    // ind subscription
    memset( &req_params , 0, sizeof( req_params ) );
    req_params.service_id = QMI_UTIL_SVC_DMS;
    req_params.message_id = QMI_DMS_SET_EVENT_REPORT_REQ_V01;
    req_params.message_specific_payload = &req;
    req_params.message_specific_payload_len = sizeof(req);
    qmi_util_post_request(&req_params);
}

void qmi_simple_ril_nas_init(qmi_util_request_id* indication_reg_req_id, qmi_util_request_id* sys_info_fetch_req_id)
{
    nas_indication_register_req_msg_v01 req;
    qmi_util_request_params             req_params;

    memset( &req, 0, sizeof( req ) );
    req.req_serving_system_valid    = TRUE;
    req.req_serving_system          = FALSE;

    req.sys_info_valid = TRUE;
    req.sys_info       = TRUE;

    // ind subscription
    memset( &req_params , 0, sizeof( req_params ) );
    req_params.service_id = QMI_UTIL_SVC_NAS;
    req_params.message_id = QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01;
    req_params.message_specific_payload = &req;
    req_params.message_specific_payload_len = sizeof(req);
    *indication_reg_req_id = qmi_util_post_request(&req_params);

    // fetch sys info    
    memset( &req_params , 0, sizeof( req_params ) );
    req_params.service_id = QMI_UTIL_SVC_NAS;
    req_params.message_id = QMI_NAS_GET_SYS_INFO_REQ_MSG_V01;
    req_params.message_specific_payload = NULL;
    req_params.message_specific_payload_len = 0; // empty payload
    *sys_info_fetch_req_id = qmi_util_post_request(&req_params);
}

int qmi_simple_ril_nas_init_resp_handler (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
{
    nas_indication_register_resp_msg_v01 * resp = (nas_indication_register_resp_msg_v01 *)qmi_msg_info->message_specific_payload;

    uplink_message->command = qmi_util_str_clone("init nas cmd");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, "simple ril NW reg init complete");

    return 0;
}

int qmi_simple_ril_nas_initialize_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_id init_req;

    qmi_simple_ril_nas_init( &init_req, &init_fetch_sys_info_req_id );

    ack_info->request_id = init_req;

    return 0;
}

