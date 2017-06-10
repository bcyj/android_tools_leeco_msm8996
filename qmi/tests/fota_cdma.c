/******************************************************************************
  @file    fota_cdma.c
  @brief   Sample simple RIL, CDMA WAP SMS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) WAP SMS subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>              //printf
#include <stdarg.h>             //va_list
#include <string.h>             //memset

#include "fota.h"
#include "wmstscdma.h"          //wms_ts_unpack_ascii
#include "qmi_client_utils.h"   //qmi_util_request_params


#define LOG_BUF_SIZE    1024
static char LOG_BUF[LOG_BUF_SIZE];
static size_t LOG_BUF_CUR =0;

#define LOG(...)            qmi_util_log(__VA_ARGS__)
#define LOG_BUF_CLEAR       LOG_BUF_CUR=0; LOG_BUF[0]=0
#define LOG_BUF_APPEND(...) fota_log_append(&LOG_BUF_CUR, LOG_BUF, LOG_BUF_SIZE, __VA_ARGS__)
#define LOG_BUF_END         LOG("%s", LOG_BUF)

extern boolean fota_verify_wap_push
(
    uint8 *data,            // start of wsp headers
    uint16 data_len,        // len in byof wsp header + remaining data
    uint8 **ppPushContent,  // to be returned to DME app
    uint16 *pPushLen        // len of pushContent
);

// see  wap-259-wdp-20010614-a section 6.5 for details on the WAP-WDP PDU format.
typedef struct wdp_cdma_data_gram_s //6.5.1
{
    uint16      src_port;
    uint16      dest_port;
    uint8       data_len;
    uint8       data[ WMS_CDMA_USER_DATA_MAX ]; //make data size match with cdma user data size for simplicity
}wdp_cdma_data_gram_s_type;

typedef struct wdp_cdma_user_data_s  //6.5.2
{
    uint8       message_type;
    uint8       total_segment;
    uint8       segment_number;
    uint8       datagram_len;
    uint8       datagram[ WMS_CDMA_USER_DATA_MAX ];
}wdp_cdma_user_data_s_type;


boolean is_wdp_cdma_user_data_concatenated(const wdp_cdma_user_data_s_type* user_data) {
    LOG ("user_data->total_segment %d", user_data->total_segment);
    return (user_data->total_segment > 1);
}

boolean is_wdp_cdma_message_type(const wdp_cdma_user_data_s_type* user_data) {
    return (0 == user_data->message_type);
}

void print_wdp_cdma_user_data(const wdp_cdma_user_data_s_type* user_data)
{
    LOG("33333333333333333333333333333333333333333333333333");
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("wdp_cdma_user_data {");
    LOG_BUF_APPEND(" message_type: %u", user_data->message_type);
    LOG_BUF_APPEND(", total_segment: %u", user_data->total_segment);
    LOG_BUF_APPEND(", segment_number: %u", user_data->segment_number);
    LOG_BUF_APPEND(", data_len: %u }", user_data->datagram_len);
    LOG_BUF_END;

    LOG(" datagram: ");
    print_wms_raw_uint8(user_data->datagram, user_data->datagram_len);

    LOG("33333333333333333333333333333333333333333333333333");
}

void print_wdp_cdma_data_gram_s_type(const wdp_cdma_data_gram_s_type* user_data)
{
    LOG("44444444444444444444444444444444444444444444444444");
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("wdp_cdma_data_gram {");
    LOG_BUF_APPEND(" src_port: %u", user_data->src_port);
    LOG_BUF_APPEND(", dest_port: %u", user_data->dest_port);
    LOG_BUF_APPEND(", data_len: %u }", user_data->data_len);
    LOG_BUF_END;

    LOG(" data: ");
    print_wms_raw_uint8(user_data->data, user_data->data_len);
    LOG("44444444444444444444444444444444444444444444444444");
}

void print_wms_cdma_user_data(const wms_cdma_user_data_s_type* user_data) {
    wms_client_bd_s_type           cl_bd_data ;
    memset( &cl_bd_data, 0, sizeof(wms_client_bd_s_type) );
    uint8 i;

    LOG_BUF_CLEAR;;
    LOG_BUF_APPEND("user_data {");;
    LOG_BUF_APPEND(" num_headers: %u }", user_data->num_headers);
    LOG_BUF_END;

    for (i = 0; i < user_data->num_headers; i++) {
        print_wms_udh(&user_data->headers[i]);
    }

    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("{ encoding (0=octet,1=is91ep, 2=ascii 4=unicode, 9=gsm7bit): %u", user_data->encoding);
    LOG_BUF_APPEND(", is91ep_type: %u", user_data->is91ep_type);
    LOG_BUF_APPEND(", padding_bits: %u", user_data->padding_bits);
    LOG_BUF_APPEND(", number_of_digits: %u", user_data->number_of_digits);
    LOG_BUF_APPEND(", data_len: %u }", user_data->data_len);
    LOG_BUF_END;

    // note: incase of WMS_ENCODING_GSM_7_BIT_DEFAULT
    // wms_ts_unpack_gw_7_bit_chars is already called
    // in wms_ts_decode_CDMA_bd
    if (WMS_ENCODING_GSM_7_BIT_DEFAULT == user_data->encoding) {
        LOG_BUF_CLEAR;
        LOG_BUF_APPEND("data: ");
        for (i = 0; i < user_data->data_len; i++) {
            LOG_BUF_APPEND("%c", user_data->data[i]);
        }
        LOG_BUF_END;
    }
    else if (WMS_ENCODING_ASCII == user_data->encoding) {
        cl_bd_data.user_data.data_len = wms_ts_unpack_ascii(
                user_data,
                user_data->number_of_digits + 1,
                cl_bd_data.user_data.data);

        LOG_BUF_CLEAR;
        LOG_BUF_APPEND(" unpacked ascii 7 bit data: {");
        LOG_BUF_APPEND(" data_len: %u, data: ", cl_bd_data.user_data.data_len);
        for (i = 0; i < cl_bd_data.user_data.data_len && i < WMS_CDMA_USER_DATA_MAX; i++) {
            LOG_BUF_APPEND("%c", cl_bd_data.user_data.data[i]);
            }
        LOG_BUF_APPEND("}");
        LOG_BUF_END;
    }

    print_wms_raw_uint8(user_data->data, user_data->data_len);
}

boolean fota_decode_wdp_cdma_datagram
(
  const wdp_cdma_user_data_s_type* user_data, /* IN */
  wdp_cdma_data_gram_s_type* datagram /* OUT */
)
{
    uint8 i=0;

    LOG("START fota_decode_wdp_cdma_datagram() DECODE STEP 4");
    if(NULL == user_data || NULL == datagram){
        LOG("Error: NULL arguments");
        return FALSE;
    }
    if (user_data->datagram_len < 4) {
        LOG("Error: len should be at least 4.  was %d",
                user_data->datagram_len);
        return FALSE;
    }
    memset(datagram, 0, sizeof(wdp_cdma_data_gram_s_type));
    datagram->src_port = user_data->datagram[i++] << 8;
    datagram->src_port |= user_data->datagram[i++];

    datagram->dest_port = user_data->datagram[i++] << 8;
    datagram->dest_port |= user_data->datagram[i++];

    datagram->data_len = user_data->datagram_len - i;

    if (4 < user_data->datagram_len) { //has data
        memcpy(datagram->data, user_data->datagram + i,
                user_data->datagram_len - i);
    }
    LOG("END fota_decode_wdp_cdma_datagram() DECODE STEP 4");
    return TRUE;
}

// see  wap-259-wdp-20010614-a section 6.5 for details on the WAP-WDP PDU format.
boolean fota_decode_wdp_cdma_user_data_from_chari
(
  const uint8* chari, /* IN */
  uint8 len,          /* IN */
  wdp_cdma_user_data_s_type* user_data /* OUT */
)
{
    uint8 i=0;

    LOG("START fota_decode_wdp_cdma_user_data_from_chari() DECODE STEP 3");
    if(NULL == user_data || NULL == chari){
        LOG("Error: NULL arguments");
        return FALSE;
    }
    if (len < 3){
        LOG("Error: len should be at least 3.  was %d", len);
        return FALSE;
    }
    memset(user_data, 0, sizeof(wdp_cdma_user_data_s_type));
    user_data->message_type = chari[i++];
    user_data->total_segment = chari[i++];
    user_data->segment_number = chari[i++];

    user_data->datagram_len = len - i;
    if (3 < len) // has data
    {
        memcpy(user_data->datagram, chari + i, len - i);
    }
    LOG("END fota_decode_wdp_cdma_user_data_from_chari() DECODE STEP 3");
    return TRUE;
}

// beardata userdata to cdma userdata
// then cdma userdata to wdp userdata
//wms_client_bd_s_type.user_data == wms_cdma_user_data_s_type
boolean fota_decode_wdp_cdma_user_data
(
  const wms_cdma_user_data_s_type* wms_cdma_user_data, /* IN */
  wdp_cdma_user_data_s_type* wdp_cdma_user_data /* OUT */
)
{
    return fota_decode_wdp_cdma_user_data_from_chari(wms_cdma_user_data->data,
            wms_cdma_user_data->data_len, wdp_cdma_user_data);
}

void print_wms_message_id(const wms_message_id_s_type* message_id) {
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("message_id {");
    LOG_BUF_APPEND(" type (1=DELIVER, 2=SUBMIT): %u", (unsigned int)message_id->type);
    LOG_BUF_APPEND(", id_number: %u", (unsigned int) message_id->id_number);
    LOG_BUF_APPEND(", udh_present: %u }", message_id->udh_present);
    LOG_BUF_END;
}

void print_wms_reply_option(const wms_reply_option_s_type* reply_option) {
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("reply_option {");
    LOG_BUF_APPEND(" user_ack_requested: %u", reply_option->user_ack_requested);
    LOG_BUF_APPEND(", delivery_ack_requested: %u", reply_option->delivery_ack_requested);
    LOG_BUF_APPEND(", read_ack_requested: %u }", reply_option->read_ack_requested);
    LOG_BUF_END;
}

// mask used for wap is:
//enum{ WMS_MASK_BD_MSG_ID           =   0x00000001 };
//enum{ WMS_MASK_BD_USER_DATA        =   0x00000002 };
void print_wms_client_bd(const wms_client_bd_s_type* bd) {
    uint32 mask = bd->mask;

    LOG("22222222222222222222222222222222222222222222222222");
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("bd {");
    LOG_BUF_APPEND(" mask: %u  %#x }", mask, mask);
    LOG_BUF_END;

    if (mask & WMS_MASK_BD_MSG_ID) {
        print_wms_message_id(&bd->message_id);
    }
    // user_data // see at the bottom
    // user_response
    if (mask & WMS_MASK_BD_MC_TIME) {
        print_wms_timestamp(&bd->mc_time);
    }
    // validity_absolute
    // validity_relative
    // deferred_absolute
    // deferred_relative
    if (mask & WMS_MASK_BD_PRIORITY) {
        LOG(" priority: %u", bd->priority);
    }
    if (mask & WMS_MASK_BD_PRIVACY) {
        LOG(" privacy: %u", bd->privacy);
    }
    if (mask & WMS_MASK_BD_REPLY_OPTION) {
        print_wms_reply_option(&bd->reply_option); //Indicates whether SMS acknowledgment is requested or not requested.
    }
    // num_messages (for vm)
    if (mask & WMS_MASK_BD_ALERT) {
        LOG(" alert_mode: %u", bd->alert_mode);
    }
    if (mask & WMS_MASK_BD_LANGUAGE) {
        LOG(" language (1=en): %u", bd->language);
    }
    if (mask & WMS_MASK_BD_CALLBACK) {
        LOG("callback address:");
        print_wms_address(&bd->callback, TRUE); //all-back number to be dialed in reply to a received SMS message.
    }
    if (mask & WMS_MASK_BD_DISPLAY_MODE) {
        LOG_BUF_CLEAR;
        LOG_BUF_APPEND(" display_mode: %u", bd->display_mode);

        if (WMS_DISPLAY_MODE_RESERVED == bd->display_mode) {
            LOG_BUF_APPEND(", download_mode: %u", bd->download_mode);
        }
        LOG_BUF_END;
    }
    // delivery_status (MO)
    // deposit_index

    // scpt_data_ptr
    // scpt_result_ptr
    // ip_address
    // rsn_no_notify
    // other
    if (mask & WMS_MASK_BD_USER_DATA) {
        print_wms_cdma_user_data(&bd->user_data);
    }
    LOG("22222222222222222222222222222222222222222222222222");
}

void print_wms_cdma_message(const wms_cdma_message_s_type* cdma_message) {
    LOG("11111111111111111111111111111111111111111111111111");
    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("cdma_message {");
    LOG_BUF_APPEND(" is_mo: %u", cdma_message->is_mo);
    LOG_BUF_APPEND(", teleservice: %u }", cdma_message->teleservice);
    LOG_BUF_END;

    print_wms_address(&cdma_message->address, TRUE);
    print_wms_subaddress(&cdma_message->subaddress);

    LOG_BUF_CLEAR;
    LOG_BUF_APPEND("{ is_tl_ack_requested: %u", cdma_message->is_tl_ack_requested);
    LOG_BUF_APPEND(", is_service_present: %u", cdma_message->is_service_present);
    LOG_BUF_APPEND(", service: %u }", cdma_message->service);
    LOG_BUF_END;

    //cdma_message->raw_ts;
    if(0) {
        print_wms_raw_uint8((uint8*)cdma_message, sizeof(wms_cdma_message_s_type));
    }
    LOG("11111111111111111111111111111111111111111111111111");
}


void wms_process_mt_cdma_sms
(
  wms_event_report_ind_msg_v01 * event_report_ind
)
 {
    wms_status_e_type decode_status = WMS_STATUS_MAX;
    wms_transfer_route_mt_message_type_v01 *trmtMsg = &event_report_ind->transfer_route_mt_message;
    wms_client_message_s_type cl_msg;
    wms_client_bd_s_type cl_bd;

    LOG("START wms_process_mt_cdma_sms ==================================================");

    do {
        if(event_report_ind->transfer_route_mt_message_valid != 1){
            LOG("ERROR: TRANSFER_ROUTE_MT_MESSAGE INVALID");
            break;
        }
        if ( WMS_MESSAGE_FORMAT_CDMA_V01 == trmtMsg->format) {
            // DECODE STEP 1:  ota data ==> wms_cdma_message_s_type
            LOG("CALLING wms_ts_cdma_OTA2cl()  DECODE STEP 1");
            if ((decode_status = wms_ts_cdma_OTA2cl(
                    trmtMsg->data, trmtMsg->data_len, &cl_msg)) // up to 255
                    != WMS_OK_S) {
                LOG("ERROR: wms_ts_cdma_OTA2cl() failed! err=%d", decode_status);
                break;
            }
            print_wms_cdma_message(&cl_msg.u.cdma_message);
            LOG("DONE wms_ts_cdma_OTA2cl DECODE STEP 1");

            if(! fota_check_cdma_wap_push_message(&cl_msg.u.cdma_message)) {
                LOG("******* NOT CDMA WAP stop processing**********");
                break;  // coment this break out for testing decode beardata
            } else {
                LOG("******* FOUND CDMA WAP ***********************");
            }

            // DECODE STEP 2: Raw bearer data ==> wms_client_bd_s_type
            LOG("CALLING wms_ts_decode_bearer_data() DECODE STEP 2");
            if ((decode_status = wms_ts_decode_bearer_data(
                    &cl_msg.u.cdma_message.raw_ts,
                    &cl_bd)
                    ) != WMS_OK_S)
            {
                LOG("ERROR: wms_ts_decode_bearer_data() failed! err=%d", decode_status);
                break;
            }
            print_wms_client_bd(&cl_bd);
            LOG("DONE wms_ts_decode_bearer_data() DECODE STEP 2");

            // DECODE STEP 3: wms_cdma_user_data_s_type ==> wdp_cdma_user_data_s_type
            wdp_cdma_user_data_s_type wdp_cdma_user_data;
            wdp_cdma_data_gram_s_type datagram;
            if(FALSE == fota_decode_wdp_cdma_user_data(&cl_bd.user_data, &wdp_cdma_user_data))
            {
                LOG("ERROR: fota_decode_wdp_cdma_user_data() failed.");
                break;
            }
            print_wdp_cdma_user_data(&wdp_cdma_user_data);

            if(FALSE == is_wdp_cdma_message_type(&wdp_cdma_user_data) ){
                LOG("CDMA WAP is not WDP message type, bail out");
                break;
            }
            if(is_wdp_cdma_user_data_concatenated(&wdp_cdma_user_data)) {
                LOG("CDMA WAP was concatenated, bail out");
                break;
            }
            // DECODE STEP 4: wdp_cdma_user_data_s_type ==> wdp_cdma_data_gram_s_type
            if(fota_decode_wdp_cdma_datagram(&wdp_cdma_user_data, &datagram))
            {
                uint8 *push_content_ptr;
                uint16 push_content_len = 0;
                print_wdp_cdma_data_gram_s_type(&datagram);

                // Sanity check for data len.  Use max from:
                // wdp_cdma_data_gram_s_type.data[ WMS_CDMA_USER_DATA_MAX ]
                // Note, max will change if concatenated wap is ever supported.
                if (datagram.data_len > WMS_CDMA_USER_DATA_MAX) {
                    LOG("ERROR: data_len %u > max %d", datagram.data_len, WMS_CDMA_USER_DATA_MAX);
                    break;
                }
                // DECODE STEP 5: verify wap push content intended for DME app for fota
                if(fota_verify_wap_push(datagram.data, datagram.data_len,
                        &push_content_ptr, &push_content_len)){
                    LOG("nofity daemon here, sending %u bytes:", push_content_len);
                    print_wms_raw_uint8(push_content_ptr, push_content_len);
                    fota_send_raw_data((char*)push_content_ptr, push_content_len);
                }
            }
        } else if (WMS_MESSAGE_FORMAT_MWI_V01 == trmtMsg->format) {
            LOG("was WMS_MESSAGE_FORMAT_MWI_V01...IGNORE..");
        } else {
            LOG("ERROR Invalid format %d", trmtMsg->format);
        }
        break;
    } while (FALSE); // run just once
    LOG("END wms_process_mt_cdma_sms ====================================================");
}

boolean fota_check_cdma_wap_push_message(const wms_cdma_message_s_type *cdma_message) {
    if (NULL == cdma_message)
    {
        return FALSE;
    }

    /* Check for WAP teleservice */
    return (WMS_TELESERVICE_WAP == cdma_message->teleservice);
} /* fota_check_cdma_wap_push_message */
