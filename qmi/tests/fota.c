/******************************************************************************
  @file    fota.c
  @brief   Sample simple RIL, WAP SMS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) WAP SMS subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>              //printf
#include <stdarg.h>             //va_list
#include <stdlib.h>             //free/malloc
#include <string.h>             //memset, strcmp, strlen
#include <time.h>               //time

#include "fota.h"
#include "qmi_client_utils.h"   //qmi_util_log, qmi_util_request_params

#define LOG_BUF_SIZE    1024
static char LOG_BUF[LOG_BUF_SIZE];
static size_t LOG_BUF_CUR =0;

#define LOG(...)            qmi_util_log(__VA_ARGS__)
#define LOG_BUF_CLEAR       LOG_BUF_CUR=0; LOG_BUF[0]=0
#define LOG_BUF_APPEND(...) fota_log_append(&LOG_BUF_CUR, LOG_BUF, LOG_BUF_SIZE, __VA_ARGS__)
#define LOG_BUF_END         LOG("%s", LOG_BUF)

#define PORT_WAP_PUSH          2948
#define PORT_WAP_PUSH_SECURE   2949

static int s_is_fota_cmd_enabled = FALSE;

extern FILE* qmi_simple_ril_suite_output_handle();

void fota_log_append(size_t* pcur, char* str, size_t size, const char * format, ... ){
    size_t cur = *pcur;
    size_t written=0;

    va_list args;
    va_start (args, format);
    if(cur < size)
    {
        written+= vsnprintf(&str[cur], size-cur, format, args);
        cur += (written > 0) ? written : 0;
        *pcur = cur;
    }
    va_end (args);
}

void set_fota_cmd_enabled(int enable)
{
    s_is_fota_cmd_enabled = enable;
}

int is_fota_cmd_enabled()
{
    return s_is_fota_cmd_enabled;
}

boolean fota_verify_wap_push
(
    uint8 *data,            // start of wsp headers
    uint16 data_len,        // len in byof wsp header + remaining data
    uint8 **ppPushContent,  // to be returned to DME application
    uint16 *pPushLen        // len of pushContent
);


void print_wms_address(const wms_address_s_type* address, boolean cdma){
    uint8 i;

    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("address {");
    LOG_BUF_APPEND(" digit_mode: %d", address->digit_mode);

    if (cdma) {
        LOG_BUF_APPEND(", number_mode: %d", address->number_mode);
        LOG_BUF_APPEND(", number_type: %d", address->number_type);
        LOG_BUF_APPEND(", number_plan: %d", address->number_plan);
    }
    LOG_BUF_APPEND(", number_of_digits: %u", address->number_of_digits);
    LOG_BUF_APPEND(", address.digits: ");
    for (i = 0; i < address->number_of_digits; i++) {
        LOG_BUF_APPEND("%u", address->digits[i]);
    }
    LOG_BUF_APPEND(" }");
    LOG_BUF_END;
}

void print_wms_subaddress(const wms_subaddress_s_type* subaddress){
    uint8 i;
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("subaddress {");
    LOG_BUF_APPEND(" type: %u", subaddress->type);
    LOG_BUF_APPEND(", odd: %u", subaddress->odd);

    LOG_BUF_APPEND(", number_of_digits: %u", subaddress->number_of_digits);
    LOG_BUF_APPEND(", subaddress.digits: ");
    for (i = 0; i < subaddress->number_of_digits; i++) {
        LOG("%u", subaddress->digits[i]);
    }
    LOG_BUF_APPEND(" }");
    LOG_BUF_END;
}

void print_wms_gw_dcs(const wms_gw_dcs_s_type* dcs) {
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("dcs {");
    LOG_BUF_APPEND(" msg_class: %d", dcs->msg_class);
    LOG_BUF_APPEND(", is_compressed: %d", dcs->is_compressed);

    //0 == WMS_GW_ALPHABET_7_BIT_DEFAULT,
    //1 == WMS_GW_ALPHABET_8_BIT,
    //2 == WMS_GW_ALPHABET_UCS2
    LOG_BUF_APPEND(", alphabet (0=7bit, 1=8bit, 2=usc2): %d", dcs->alphabet);
    LOG_BUF_APPEND(", msg_waiting: %d", dcs->msg_waiting);
    LOG_BUF_APPEND(", msg_waiting_active: %d", dcs->msg_waiting_active);
    LOG_BUF_APPEND(", msg_waiting_kind: %d", dcs->msg_waiting_kind);
    LOG_BUF_APPEND(", raw_dcs_data: %u", dcs->raw_dcs_data);
    LOG_BUF_APPEND(" }");
    LOG_BUF_END;
}

void print_wms_timestamp(const wms_timestamp_s_type* timestamp) {
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("timestamp (y:m:d:h:m:s:tz):  ");
    LOG_BUF_APPEND("%u:%u:%u:%u:%u:%u:%d",
            timestamp->year, timestamp->month, timestamp->day,
            timestamp->hour, timestamp->minute, timestamp->second,
            timestamp->timezone);
    LOG_BUF_END;
}

// udh is array of headers
void print_wms_udh(const wms_udh_s_type* udh) {
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("udh {");
    LOG_BUF_APPEND(" header_id: %u", udh->header_id);

    switch (udh->header_id) {
    case WMS_UDH_PORT_8:
        LOG_BUF_APPEND(", wap_8.orig_port: %u", udh->u.wap_8.orig_port);
        LOG_BUF_APPEND(", wap_8.dest_port: %u", udh->u.wap_8.dest_port);
        break;
    case WMS_UDH_PORT_16:
        LOG_BUF_APPEND(", wap_16.orig_port: %u", udh->u.wap_16.orig_port);
        LOG_BUF_APPEND(", wap_16.dest_port: %u", udh->u.wap_16.dest_port);
        break;
    case WMS_UDH_CONCAT_8:
        LOG_BUF_APPEND(", wms_udh_concat_8: {");
        LOG_BUF_APPEND(", msg_ref: %u",  udh->u.concat_8.msg_ref);
        LOG_BUF_APPEND(", total_sm: %u", udh->u.concat_8.total_sm);
        LOG_BUF_APPEND(", seq_num: %u }",  udh->u.concat_8.seq_num);
        break;
    case WMS_UDH_CONCAT_16:
        LOG_BUF_APPEND(", wms_udh_concat_16: {");
        LOG_BUF_APPEND(", msg_ref: %u",  udh->u.concat_16.msg_ref);
        LOG_BUF_APPEND(", total_sm: %u", udh->u.concat_16.total_sm);
        LOG_BUF_APPEND(", seq_num: %u }",  udh->u.concat_16.seq_num);
        break;
    default:
        LOG_BUF_APPEND(", unhandled header id");
        break;
    }
    LOG_BUF_APPEND(" }");
    LOG_BUF_END;
}

void print_wms_raw_uint8(const uint8* data, uint32 data_len) {
    uint32 i;
    uint32 j;
    uint8 k;

    LOG("RAW uint8 in hex with space (data_len=%d):", data_len);
    for (i = 0, k=1; i < data_len; i += 8, k++) {
        LOG_BUF_CLEAR;
        LOG_BUF_APPEND("%d:  ", k);
        for (j = i; j < i + 8 && j < data_len; j++) {
            LOG_BUF_APPEND("%.2x ", data[j]);
        }
        LOG_BUF_END;
    }
}

void fota_send_raw_data(char const * const data, int nof_bytes) {
    int idx;

    if (NULL == distributor_data_handle) {
        LOG("data pipe is NULL");
        return;
    }
    LOG("Sending %d bytes on data pipe", nof_bytes);

    fprintf(distributor_data_handle, "%06d", nof_bytes);
    for (idx = 0; idx < nof_bytes; idx++)
    {
        fprintf(distributor_data_handle, "%c", data[idx]);
    }
    fflush(distributor_data_handle);

    LOG("Done sending %d bytes on data pipe", nof_bytes);
}

void print_wms_user_data(const wms_gw_user_data_s_type* user_data, const wms_gw_dcs_s_type *dcs) {
    uint16 i;
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("user_data {");
    LOG_BUF_APPEND(" num_headers: %u }", user_data->num_headers);
    LOG_BUF_END;

    for (i = 0; i < user_data->num_headers; i++) {
        print_wms_udh(&user_data->headers[i]);
    }
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("{ sm_len: %u", user_data->sm_len);
    LOG_BUF_APPEND(", sm_data: ");
    if (WMS_GW_ALPHABET_7_BIT_DEFAULT == dcs->alphabet) {
        for (i = 0; i < user_data->sm_len; i++) {
            LOG_BUF_APPEND("%c", user_data->sm_data[i]);
        }
    }
    LOG_BUF_APPEND(" }");
    LOG_BUF_END;

    print_wms_raw_uint8(user_data->sm_data, user_data->sm_len);
}

void print_wms_gw_deliver(const wms_gw_deliver_s_type* deliver) {
    LOG("==================================================");
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("deliver {");
    LOG_BUF_APPEND(" more: %u", deliver->more);
    LOG_BUF_APPEND(", reply_path_present: %u", deliver->reply_path_present);
    LOG_BUF_APPEND(", user_data_header_present: %u", deliver->user_data_header_present);
    LOG_BUF_APPEND(", status_report_enabled: %u", deliver->status_report_enabled);
    LOG_BUF_APPEND(", pid: %u", deliver->pid);
    LOG_BUF_APPEND(" }");
    LOG_BUF_END;

    print_wms_address(&deliver->address, FALSE);
    print_wms_gw_dcs(&deliver->dcs);
    print_wms_timestamp(&deliver->timestamp);
    print_wms_user_data(&deliver->user_data, &deliver->dcs);
    LOG("==================================================");
}

boolean is_wms_gw_user_data_concatenated(const wms_gw_user_data_s_type* user_data) {
    uint8 i;

    for (i = 0; i < user_data->num_headers; i++) {
        switch (user_data->headers[i].header_id) {
            case WMS_UDH_CONCAT_8:
                return (user_data->headers[i].u.concat_8.total_sm > 1);
            case WMS_UDH_CONCAT_16:
                return (user_data->headers[i].u.concat_16.total_sm > 1);
            default:
                continue;
            }
    }
    return FALSE;
}

void wms_process_mt_gw_sms
(
  wms_event_report_ind_msg_v01 * event_report_ind
)
{
    wms_status_e_type decode_status = WMS_STATUS_MAX;
    wms_gw_deliver_s_type *deliver;
    wms_transfer_route_mt_message_type_v01 *trmtMsg = &event_report_ind->transfer_route_mt_message;

    LOG("START wms_process_mt_gw_sms ==================================================");

    /* Allocate buffer to decode new GW SMS */
    deliver = malloc( sizeof( wms_gw_deliver_s_type ));
    if (NULL == deliver) {
        LOG("ERROR: Failed to allocate buffer to decode new GW SMS.");
    }
    else if(event_report_ind->transfer_route_mt_message_valid != 1){
        LOG("ERROR: TRANSFER_ROUTE_MT_MESSAGE INVALID");
    }
    else if (WMS_MESSAGE_FORMAT_GW_PP_V01 != trmtMsg->format)
    {
        LOG("ERROR: Invalid format");
    }
    else
    {
        print_wms_raw_uint8(trmtMsg->data, trmtMsg->data_len); // up to 255
        if (WMS_OK_S == (decode_status = wms_ts_decode_deliver_from_uint8(
                trmtMsg->data,
                deliver,
                TRUE))) //TRUE to check for MTI RESERVED
        {
            print_wms_gw_deliver(deliver);

            if(fota_check_gw_wap_push_message(deliver)){
                LOG("******* FOUND GW WAP ***********************");
                uint8 *push_content_ptr;
                uint16 push_content_len = 0;
                // assuming wap is not segmented.
                if (is_wms_gw_user_data_concatenated(&deliver->user_data)) {
                    LOG("GW WAP was concatenated, bail out");
                }
                // Sanity check for data len.  Use max from:
                // wms_gw_user_data_s_type.sm_data[WMS_MAX_LEN]
                // Note, max will change if concatenated wap is ever supported.
                else if (deliver->user_data.sm_len > WMS_MAX_LEN) {
                    LOG("ERROR: data_len %u > max %d", deliver->user_data.sm_len, WMS_MAX_LEN);
                }
                else if (fota_verify_wap_push(deliver->user_data.sm_data, deliver->user_data.sm_len,
                        &push_content_ptr, &push_content_len)) {
                    LOG("nofity daemon here, sending %u bytes:", push_content_len);
                    print_wms_raw_uint8(push_content_ptr, push_content_len);
                    fota_send_raw_data((char*)push_content_ptr, push_content_len);
                }
            }
            else {
                LOG("******* NOT GW WAP stop processing *********");
            }
        } else {
            // note if MT was not MTI deliver ==0x00 or reserved 0x11, then
            // err is WMS_INVALID_TPDU_TYPE_S (200)
            LOG("ERROR: decode deliver FAILURE! err = %d", decode_status);
        }
    }
    free( deliver );
    LOG("END wms_process_mt_gw_sms ==================================================");
}

void wms_send_ack(wms_event_report_ind_msg_v01* event_report_ind, wms_message_protocol_enum_v01 protocol)
{
    qmi_util_request_id request_id;

    if(event_report_ind->transfer_route_mt_message_valid &&
            WMS_ACK_INDICATOR_SEND_ACK_V01 == event_report_ind->transfer_route_mt_message.ack_indicator)
    {
        wms_send_ack_req_msg_v01 req_ack_msg;
        qmi_util_request_params req_params;

        memset(&req_ack_msg, 0, sizeof(req_ack_msg));
        req_ack_msg.ack_information.transaction_id = event_report_ind->transfer_route_mt_message.transaction_id;
        req_ack_msg.ack_information.message_protocol = protocol;
        req_ack_msg.ack_information.success = 1; /* success */

        // check if MT SMS was over IMS
        if (event_report_ind->sms_on_ims_valid && event_report_ind->sms_on_ims)
        {
            req_ack_msg.sms_on_ims_valid = TRUE;
            req_ack_msg.sms_on_ims = TRUE;
        }

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_WMS;
        req_params.message_id = QMI_WMS_SEND_ACK_REQ_V01;
        req_params.message_specific_payload = &req_ack_msg;
        req_params.message_specific_payload_len = sizeof(req_ack_msg);

        request_id = qmi_util_post_request(&req_params);

        LOG("SENT ack for protocol(0=cdma, 1=wcdma): %d, transaction_id: %d, ims: %d, request_id:%d",
                req_ack_msg.ack_information.message_protocol,
                req_ack_msg.ack_information.transaction_id,
                req_ack_msg.sms_on_ims_valid,
                request_id);
    }
}

void wms_print_sms_ind_fota(wms_event_report_ind_msg_v01* event_report_ind, qmi_simple_ril_cmd_completion_info* uplink_message)
{
    wms_transfer_route_mt_message_type_v01 *trmtMsg;
    int format;

    LOG("START wms_print_sms_ind_fota");

    do {
        if (NULL == event_report_ind)
        {
            LOG("ERROR: event_report_ind is NULL");
            break;
        }
        /* SMS on SIM */
        if ((event_report_ind->mt_message_valid) &&
                   (event_report_ind->message_mode_valid))
        {
            LOG("It is SMS on SIM with message_mode (0=cdma, 1=gw): %d, Ignoring...",
                    event_report_ind->message_mode);
            break;
        }
        /* ETWS message */
        if (event_report_ind->etws_message_valid)
        {
            LOG("It is ETWS message, Ignoring...");
            break;
        }
        if (event_report_ind->transfer_route_mt_message_valid)
        {
            trmtMsg = &event_report_ind->transfer_route_mt_message;
            format = trmtMsg->format;

            LOG_BUF_CLEAR;
            LOG_BUF_APPEND("format(0=cdma, 6=gw_pp, 7=gw_bc, 8=mwi): %d, ", format);
            LOG_BUF_APPEND("ack_indicator (0=ack, 1=don't ack): %d, ", trmtMsg->ack_indicator);
            LOG_BUF_APPEND("transaction id: %d, ", trmtMsg->transaction_id);
            LOG_BUF_APPEND("sms_on_ims_valid: %d, ", event_report_ind->sms_on_ims_valid);
            LOG_BUF_APPEND("sms_on_ims: %d, ", event_report_ind->sms_on_ims);
            LOG_BUF_APPEND("data_len: %u, ", trmtMsg->data_len);
            LOG_BUF_APPEND("mt_message_smsc_address_valid: %u, ", event_report_ind->mt_message_smsc_address_valid);
            LOG_BUF_APPEND("mt_message_smsc_address.data_len: %u ", event_report_ind->mt_message_smsc_address.data_len);
            LOG_BUF_END;

            // qmi checks for those:
            // qcril_sms_process_event_report_ind
            if ((WMS_MESSAGE_FORMAT_CDMA_V01 == format)
                    || (WMS_MESSAGE_FORMAT_MWI_V01 == format))
            {
                wms_process_mt_cdma_sms(event_report_ind);
                wms_send_ack(event_report_ind, WMS_MESSAGE_PROTOCOL_CDMA_V01);
            }
            else if (WMS_MESSAGE_FORMAT_GW_PP_V01 == format)
            {
                wms_process_mt_gw_sms(event_report_ind);
                wms_send_ack(event_report_ind, WMS_MESSAGE_PROTOCOL_WCDMA_V01);
            }
            else if (WMS_MESSAGE_FORMAT_GW_BC_V01 == format)
            {
                LOG("It is a broadcast SMS, Ignoring...");
            }
            else
            {
                LOG("ERROR: Unknown format %d", format);
            }
        } /* transfer_route_mt_message_valid */
    } while (FALSE); // run just once
    LOG("END wms_print_sms_ind_fota");
}

boolean fota_check_gw_wap_push_message(const wms_gw_deliver_s_type *gw_deliver) {
    boolean ret_value = FALSE;

    if (NULL == gw_deliver)
    {
        return FALSE;
    }
    if (gw_deliver->user_data_header_present) {
        uint8 i = 0;
        for (i = 0; i < gw_deliver->user_data.num_headers; i++) {
            if ((WMS_UDH_PORT_16 == gw_deliver->user_data.headers[i].header_id)
                && ((PORT_WAP_PUSH == gw_deliver->user_data.headers[i].u.wap_16.dest_port)
                   || ( PORT_WAP_PUSH_SECURE == gw_deliver->user_data.headers[i].u.wap_16.dest_port)))
            {
                /* Is a WAP Push Message */
                ret_value = TRUE;
                break;
            }
        }
    }
    return ret_value;
} /* fota_check_gw_wap_push_message() */

/* To test CDMA:
 * 00=message_type, 1=total_segment, 0=segment_num // WDP header
 * 23f0(9200)=src_port, 0b84(2948)=dest_port // wdp datagram header
 * 01=tid, 06=pdutype push, 07=headerlen, c4=contentType in short int (0x44), af87=appid field name&val //WSP header
 *
 * 00 01 00
 * 23 f0 0b 84
 * 01 06 07 c4 b4 84 8d 9e af 87
 * a0 f2 a3 55 d1 53 ec 43 ff 4d 56 bb 8f 3f 00 84 02 d8 00 00 00 0a  //final payload
 * OR just
 * 01 02 03 for final payload
*/

// sample wsp from test server in hex:
// 01 06 07 c4 b4 84 8d 9e af 87 a0 f2 a3 55 d1 53 ec 43 ff 4d 56 bb 8f 3f 00 84 02 d8 00 00 00 0a
/*
 ******************************************************************************
 * fota_verify_wap_push
 *
 * Function description:
 *
 * This function examines a received WAP Push message to determine if it contains
 * a trigger intended for DME application. If so it extracts the data payload.
 *
 * Parameters:
 *
 * data - Pointer to received WAP Push message
 * data_len - Length in bytes of received WAP Push message
 * ppPushContent - Pointer to a variable in which to store a pointer to the received WAP Push
 *   payload.
 * pPushLen - Pointer to a variable in which to store the length of the WAP Push Payload
 *
 * Return value:
 *
 * TRUE if a Push intended for DME app was detected and payload was successfully extracted
 * FALSE otherwise
 *
 ******************************************************************************
*/
boolean fota_verify_wap_push
(
    uint8 *data,            // start of wsp headers
    uint16 data_len,        // len in byof wsp header + remaining data
    uint8 **ppPushContent,  // to be returned to app
    uint16 *pPushLen        // len of pushContent
)
{
    uint8 *pUserData = data;
    uint8 TID;
    uint8 PDUType;
    uint32 PushHeaderLen;
    uint8 octet;

    // Determine if this is a message destined for the DME app
    LOG("START fota_verify_wap_push: data_len=%u", data_len);
    if (NULL == data || NULL == pPushLen) {
        LOG("ERROR: received NULL arguments");
        return FALSE;
    }
    if (data_len < 3) {
        // at least 3 bytes to account for mandatory headers: tid, pdu_type, header_len
        LOG("ERROR:: data_len %u < 3", data_len);
        return FALSE;
    }

    TID = *pUserData++; // TID
    PDUType = *pUserData++; // PDU Type
    LOG("tid = %u, pdu_type = %u", TID, PDUType);

    // PDU Type 0x6 == Push, 0x7==Confirmed Push
    if (0x6 != PDUType && 0x07 != PDUType) {
        LOG("ERROR: wrong pdu type");
        return FALSE;
    }

    /**
     * Parse HeaderLen (length of ContentType and Headers fields combined.)
     * From wap-230-wsp-20010705-a section 8.2.4.1
     * HeaderLen is uintvar. Section 8.1.2 states
     * The maximum size of a uintvar is 32 bits.
     * So it will be encoded in no more than 5 octets.
     *
     * However per OMA-TS-DM_Notification-V1_2_1-20080617-A section 7.1
     * the total length of the WDP and WSP headers never exceeds 48 bytes.
     * Hence, assume 1 octet.
     */
    if ((*pUserData & 0x80) != 0) { // MSB set to 1 means, more than 1 octet
        LOG("ERROR: header_len is not 1 octet");
        return FALSE;
    }
    PushHeaderLen = 0x7f & (*pUserData++);
    LOG("header_len=%d", PushHeaderLen);

    // header should be at least 3 bytes to account for contentType in short int(1) and AppId field name and val(2)
    if (PushHeaderLen < 3) {
        LOG("ERROR: header_len %d < 3", PushHeaderLen);
        return FALSE;
    }
    // Sanity check to ensure push header is not longer than the message itself
    //  Minus 3 to account for TID, PDU type, headerLen
    if (PushHeaderLen > (data_len - 3)) {
        LOG("ERROR:  header_len(%d) > (data_len(%d) - 3) = %d",
                PushHeaderLen, data_len, data_len - 3);
        return FALSE;
    }

    /**
     * Check content type field
     * OMA-TS-DM_Notification-V1_2_1-20080617-A: Section 7.1
     * The Content-Type header [PUSHMSG] MUST include the MIME media type for
     *  Packet #0 as defined in [IANA]. The Content-Type code 0x44 MUST be used
     *  instead of the textual representation "application/vnd.syncml.notification"
     *  of the MIME code.
     */
    if (0 == (*pUserData & 0x80)) { // MSB not set to 1, was not a short integer
        LOG("ERROR: content type was not short integer");
        return FALSE;
    }
    octet = 0x7f & (*pUserData);
    LOG("found content type %#x", octet);
    if (0x44 == octet) { /* this is the only legal WAP-encoded assigned number for content type */
        pUserData++;
        PushHeaderLen--;
    }
    else {
        LOG("ERROR: content type mismatch");
        return FALSE;
    }
    /** check for X-WAP-Application-ID header: field name followed by field value
     *  header may contain more than 1 pair of field, so loop thru header
     *  until we find the application id pair.
     *
     *  See wap-230-wsp-20010705-a: Table 3
     *  Looking for: 0xAF87 which is 0x2f07 with MSB set to 1 in short integer.
     *
     *  Section 8.4.1.1:  Field name is integer or text.
     *  Section 8.4.2.1:  Short-integer (0-127) = OCTET, MSB set to 1
     *
     *  OMA-TS-DM_Notification-V1_2_1-20080617-A: Section 7.1
     *  The X-WAP-Application-ID header [PUSHMSG] MUST include the application-id
     *  associated with the SyncML Device Management User Agent.
     *  The application-id code 0x07 MUST be used instead of the textual
     *  representation of the Application-id
     */
    do {
        // get field name
        octet = *pUserData++;
        PushHeaderLen--;

        // 0x2f & 0x80 == 0xAF
        // looking for 0xAF imediately followed by 0x87
        if (0xAF == octet) {
            // get field value
            octet = *pUserData++;
            PushHeaderLen--;
            // 0x07 & 0x80 == 0x87
            if (0x87 == octet){
                // found it!!! strip off MSB to get real app id value
                octet &= 0x7f;
                break;
            }
        }
    } while (PushHeaderLen >= 2);

    if (0x07 != octet) {
        LOG("ERROR: did not find application-id code 0x07 for fota. octet=%#.2x, header_len=%d",
                octet, PushHeaderLen);
        return FALSE;
    }
    /* Skip remaining header bytes, including Push Header, to get to the data payload. This layer is not equipped
     with a WAP stack that can properly inspect the remaining part, which could contain any number of optional
     elements */
    pUserData += PushHeaderLen;

    /* Set data payload pointer and length */
    *ppPushContent = pUserData;
    *pPushLen = data_len - (uint8)(pUserData - data);

    LOG("END fota_verify_wap_push");
    return TRUE;
}
