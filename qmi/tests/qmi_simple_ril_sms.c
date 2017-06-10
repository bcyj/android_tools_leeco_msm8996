/******************************************************************************
  @file    qmi_simple_ril_sms.c
  @brief   Sample simple RIL, SMS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) SMS subsystem

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

#include "qmi_simple_ril_sms.h"
#include "wireless_messaging_service_v01.h"
#include "fota.h"

#define TEMP_ARRAY_SIZE 256
#define TRUE 1
#define FALSE 0

static char temp_char_array[TEMP_ARRAY_SIZE];

static void wms_print_sms_ind(wms_event_report_ind_msg_v01* wms_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void wms_print_mo_sms_resp(wms_raw_send_resp_msg_v01* wms_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static int wms_encode_destination_number(int is_gw, char * sender_number, char * encoded_stream);

int qmi_simple_ril_mt_sms_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    wms_set_event_report_req_msg_v01 req_mt_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_mt_msg,0,sizeof(req_mt_msg));
        req_mt_msg.report_mt_message_valid = 1; /* request to report mt sms */
        req_mt_msg.report_mt_message = 1; /*  Report new MT messages
             0x00 - Disable
            0x01 - Enable*/


        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_SET_EVENT_REPORT_REQ_V01;
        req_params.message_specific_payload = &req_mt_msg;
        req_params.message_specific_payload_len = sizeof(req_mt_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_mt_sms_req_handler_fota(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    int ret = 0;
    qmi_util_log("START qmi_simple_ril_mt_sms_req_handler_fota");
    set_fota_cmd_enabled(TRUE);
    ret = qmi_simple_ril_mt_sms_req_handler(cmd_params, ack_info);
    qmi_util_log("END qmi_simple_ril_mt_sms_req_handler_fota");
    return ret;
}
int qmi_simple_ril_mt_sms_route_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    wms_set_routes_req_msg_v01 req_route;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_route,0,sizeof(req_route));
        req_route.route_list_tuple_len = 1;
        req_route.route_list_tuple[0].message_type = 0;
        req_route.route_list_tuple[0].message_class = 0x05; 
        /*  Message class:
           0x00 - Class 0
           0x01 - Class 1
           0x02 - Class 2
           0x03 - Class 3
           0x04 - Class None
           0x05 - Class CDMA
       */
        req_route.route_list_tuple[0].route_storage = 0xff; /*  If action is "store," where to store the incoming
           message
           0x00 - UIM
           0x01 - NV
           0xFF - None
       */
        req_route.route_list_tuple[0].receipt_action = 0x02; /* transfer only */
        /*  Action to be taken on receipt of a message
           matching specified type and class for this route
           0x00 - Discard - Incoming messages for this
            route are discarded by the WMS service
            without notifying QMI_WMS clients
           0x01 - Store and notify - Incoming messages
            for this route are stored to the specified device
            memory and new message notifications are
            sent to registered clients
           0x02 - Transfer Only - Incoming messages for
            this route are transferred to the client and the
            client is expected to send ACK to the network
           0x03 - Transfer and ACK - Incoming
            messages for this route are transferred to the
            client and ACK is sent to the network
       */

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_SET_ROUTES_REQ_V01;
        req_params.message_specific_payload = &req_route;
        req_params.message_specific_payload_len = sizeof(req_route);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_wms_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    qmi_util_log("START qmi_simple_ril_wms_unsolicited_qmi_message_handler");
    if (NULL != unsolicited_msg_info->message_specific_payload) {
        if (is_fota_cmd_enabled()) {
            wms_print_sms_ind_fota(
                    ((wms_event_report_ind_msg_v01*) unsolicited_msg_info->message_specific_payload),
                    uplink_message);
        } else {
            wms_print_sms_ind(
                    ((wms_event_report_ind_msg_v01*) unsolicited_msg_info->message_specific_payload),
                    uplink_message);
        }
    }
    else {
        sprintf(temp_char_array, "\nIndication has no message payload\n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set,
                temp_char_array);
    }
    qmi_util_log("END qmi_simple_ril_wms_unsolicited_qmi_message_handler");
    return 0;
}

static void wms_print_sms_ind(wms_event_report_ind_msg_v01* wms_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int i=0;

    if (wms_arg_ind_msg->transfer_route_mt_message_valid == 1) 
    {
        if(wms_arg_ind_msg->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_CDMA_V01)/* CDMA */
        {
            sprintf(temp_char_array, "\nMESSAGE MODE CDMA\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, "\nmt_message_valid:%d\n", wms_arg_ind_msg->mt_message_valid);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, "\nstorage_type:%d, storage_index:%d\n", wms_arg_ind_msg->mt_message.storage_type, wms_arg_ind_msg->mt_message.storage_index);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, "\ntransfer_route_mt_message_valid:%d\n", wms_arg_ind_msg->transfer_route_mt_message_valid);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, "\nmt_sms.transfer_route_mt_message.ack_indicator:%d\n", wms_arg_ind_msg->transfer_route_mt_message.ack_indicator);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, "\ntransaction_id: %d\n", wms_arg_ind_msg->transfer_route_mt_message.transaction_id);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            sprintf(temp_char_array, "\nformat:%d\n", wms_arg_ind_msg->transfer_route_mt_message.format);
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

            sprintf(temp_char_array, "\n\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            if(wms_arg_ind_msg->mt_message_valid == 1)
            {
                sprintf(temp_char_array, "\nMT_MESSAGE IS VALID\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                sprintf(temp_char_array, "storage_type:%d, storage_index:%d\n", wms_arg_ind_msg->mt_message.storage_type, wms_arg_ind_msg->mt_message.storage_index);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            else
            {
                sprintf(temp_char_array, "MT_MESSAGE IS INVALID\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            if(wms_arg_ind_msg->transfer_route_mt_message_valid == 1)
            {
                sprintf(temp_char_array, "TRANSFER_ROUTE_MT_MESSAGE IS VALID\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                sprintf(temp_char_array, "ack_indicator: %d\n", wms_arg_ind_msg->transfer_route_mt_message.ack_indicator);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                sprintf(temp_char_array, "transaction_id: %d\n", wms_arg_ind_msg->transfer_route_mt_message.transaction_id);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                sprintf(temp_char_array, "format:%d\n", wms_arg_ind_msg->transfer_route_mt_message.format);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                sprintf(temp_char_array, "message_data :\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                for (i = 0; i < wms_arg_ind_msg->transfer_route_mt_message.data_len; i++)
                {
                    sprintf(temp_char_array, "%02x\t", wms_arg_ind_msg->transfer_route_mt_message.data[i]);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }

                if(wms_arg_ind_msg->transfer_route_mt_message.ack_indicator == WMS_ACK_INDICATOR_SEND_ACK_V01)
                {
                    wms_send_ack_req_msg_v01 req_ack_msg;
                    qmi_util_request_params req_params;
                    sprintf(temp_char_array, "\nAcking..\n");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    // NEED TO ACK MT SMS
                    memset(&req_ack_msg, 0, sizeof(req_ack_msg));
                    req_ack_msg.ack_information.transaction_id = wms_arg_ind_msg->transfer_route_mt_message.transaction_id;
                    req_ack_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_CDMA_V01;
                    req_ack_msg.ack_information.success = 1; /* success */

                    memset(&req_params, 0, sizeof(req_params));
                    req_params.service_id = QMI_UTIL_SVC_WMS;
                    req_params.message_id = QMI_WMS_SEND_ACK_REQ_V01;
                    req_params.message_specific_payload = &req_ack_msg;
                    req_params.message_specific_payload_len = sizeof(req_ack_msg);
                    qmi_util_post_request(&req_params);
                }
                sprintf(temp_char_array, "\n\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            else
            {
                sprintf(temp_char_array, "TRANSFER_ROUTE_MT_MESSAGE IS INVALID\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            if(wms_arg_ind_msg->message_mode_valid == 1)
            {
                sprintf(temp_char_array, "MESSAGE MODE VALID: %d\n", wms_arg_ind_msg->message_mode);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
            else
            {
                sprintf(temp_char_array, "MESSAGE MODE INVALID\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
            }
        }
        else if (wms_arg_ind_msg->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_PP_V01 || 
                 wms_arg_ind_msg->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_BC_V01) /* GW */
        {
            sprintf(temp_char_array, "MESSAGE MODE GW, NO PROCESS\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else if (wms_arg_ind_msg->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_MWI_V01)
        {
            sprintf(temp_char_array, "MESSAGE MODE MWI, NO PROCESS\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
        else
        {
            sprintf(temp_char_array, "UNKNOWN MESSAGE MODE\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
}

int qmi_simple_ril_cdma_mo_sms_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    int i=0,j=0;
    qmi_util_request_params req_params;
    wms_raw_send_req_msg_v01 req_mo_msg;

    uint8_t arr[WMS_MESSAGE_LENGTH_MAX_V01];

    if (cmd_params->info_set->nof_entries != 1)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_mo_msg,0,sizeof(req_mo_msg));
        memset(arr,0,sizeof(arr));
        req_mo_msg.raw_message_data.format = 0x00; // CDMA

        arr[0] = 0x00;
        arr[1] = 0x00;
        arr[2] = 0x02;
        arr[3] = 0x10;
        arr[4] = 0x02;
        arr[5] = 0x04;
        arr[6] = wms_encode_destination_number(FALSE,cmd_params->info_set->entries[0],&arr[7]);

        j = 7 + arr[6];

        arr[j++] = 0x08;
        arr[j++] = 0x10;
        arr[j++] = 0x00;
        arr[j++] = 0x03;
        arr[j++] = 0x20;
        arr[j++] = 0x56;
        arr[j++] = 0x10;
        arr[j++] = 0x01;
        arr[j++] = 0x03;
        arr[j++] = 0x10;
        arr[j++] = 0x0C;
        arr[j++] = 0x10;
        arr[j++] = 0x08;
        arr[j++] = 0x01;
        arr[j++] = 0x00;
        arr[j++] = 0x0C;
        arr[j++] = 0x01;
        arr[j++] = 0x00;

        req_mo_msg.raw_message_data.raw_message_len = j;
        for (i = 0; i< j; i++)
        {
            req_mo_msg.raw_message_data.raw_message[i] = arr[i];
        }

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_RAW_SEND_REQ_V01;
        req_params.message_specific_payload = &req_mo_msg;
        req_params.message_specific_payload_len = sizeof(req_mo_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_mo_sms_req_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            wms_print_mo_sms_resp(((wms_raw_send_resp_msg_v01*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }        
        }
    else
        {
        sprintf(temp_char_array, "ERROR in sending the MO_SMS Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void wms_print_mo_sms_resp(wms_raw_send_resp_msg_v01* wms_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    sprintf(temp_char_array, "MESSAGE ID %d\n", wms_arg_resp_msg->message_id);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "RESULT CODE %d\n", wms_arg_resp_msg->resp.result);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "CAUSE CODE VALID %d\n", wms_arg_resp_msg->cause_code_valid);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "CAUSE_CODE %d\n", wms_arg_resp_msg->cause_code);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "ERROR_CLASS_VALID %d\n", wms_arg_resp_msg->error_class_valid);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "ERROR_CLASS %d\n", wms_arg_resp_msg->error_class);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
}

int wms_encode_destination_number(int is_gw, char * sender_number, char * encoded_stream)
{
    int encoded_stream_len = 0;
    int temp_i = 0;
    int bitpos=2;

    if(is_gw)
    {
        while(*sender_number)
        {
            if(!temp_i)
            {
                encoded_stream[encoded_stream_len] = (*sender_number - 48);
                temp_i=1;
            }
            else
            {
                encoded_stream[encoded_stream_len] = ((*sender_number - 48) << 4) | encoded_stream[encoded_stream_len];
                encoded_stream_len++;
                temp_i=0;
            }
            sender_number++;
        }

        if(temp_i)
        {
            encoded_stream[encoded_stream_len] = 0xF0 | encoded_stream[encoded_stream_len];
            encoded_stream_len++;
        }
    }
    else
    {
        temp_i = strlen(sender_number);
        do
        {
            if(2 == bitpos)
            {
                encoded_stream[encoded_stream_len] |= (temp_i >> 2);
                encoded_stream[encoded_stream_len + 1] |= (temp_i << 6);
                encoded_stream_len++;
                bitpos = 4;
            }
            else
            {
                encoded_stream[encoded_stream_len] |= (temp_i << 2);
                bitpos = 2;
            }
            temp_i = (*sender_number - 48);
        }while(*sender_number++);

        encoded_stream_len++;
    }

    return encoded_stream_len;
}

int qmi_simple_ril_gsm_mo_sms_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    int i=0;
    int j=0;
    int destination_number_len=0;

    qmi_util_request_params req_params;
    wms_raw_send_req_msg_v01 req_mo_msg;
    uint8_t arr[WMS_MESSAGE_LENGTH_MAX_V01];

    if (cmd_params->info_set->nof_entries != 1)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_mo_msg,0,sizeof(req_mo_msg));
        memset(arr,0,sizeof(arr));

        destination_number_len = strlen(cmd_params->info_set->entries[0]);
        req_mo_msg.raw_message_data.format = 0x06; //GW_PP
        
        arr[0] = 0x00;
        arr[1] = 0x11;
        arr[2] = 0x00;
        arr[3] = destination_number_len;
        arr[4] = 0x81;



        j = 5 + wms_encode_destination_number(TRUE,cmd_params->info_set->entries[0],&arr[5]);

        arr[j++] = 0x00;
        arr[j++] = 0x00;
        arr[j++] = 0xAA;
        arr[j++] = 0x04;
        arr[j++] = 0xF4;
        arr[j++] = 0xF2;
        arr[j++] = 0x9C;
        arr[j++] = 0x0E;

        req_mo_msg.raw_message_data.raw_message_len = j;
        for (i = 0; i< j; i++)
        {
            req_mo_msg.raw_message_data.raw_message[i] = arr[i];
        }        

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_RAW_SEND_REQ_V01;
        req_params.message_specific_payload = &req_mo_msg;
        req_params.message_specific_payload_len = sizeof(req_mo_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_cdma_activate_broadcast_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    wms_set_broadcast_activation_req_msg_v01 req_broadcast_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_broadcast_msg,0,sizeof(req_broadcast_msg));
        req_broadcast_msg.broadcast_activation_info.bc_activate=1;
        req_broadcast_msg.broadcast_activation_info.message_mode=0; //0x00 - CDMA

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01;
        req_params.message_specific_payload = &req_broadcast_msg;
        req_params.message_specific_payload_len = sizeof(req_broadcast_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_gw_activate_broadcast_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    wms_set_broadcast_activation_req_msg_v01 req_broadcast_msg;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_broadcast_msg,0,sizeof(req_broadcast_msg));
        req_broadcast_msg.broadcast_activation_info.bc_activate=1;
        req_broadcast_msg.broadcast_activation_info.message_mode=1; //0x01 - GW

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01;
        req_params.message_specific_payload = &req_broadcast_msg;
        req_params.message_specific_payload_len = sizeof(req_broadcast_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}


