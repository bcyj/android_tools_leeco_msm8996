/******************************************************************************
  @file    qmi_simple_ril_misc.c
  @brief   Sample simple RIL, DMS&PB&UIM

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) DMS&PB&UIM subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2010, 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

#include "qmi_simple_ril_misc.h"
#include "device_management_service_v01.h"   
#include "user_identity_module_v01.h" 


#include "phonebook_manager_service_v01.h"

#define TEMP_ARRAY_SIZE 256
#define TRUE 1
#define FALSE 0
#define ONLINE_MODE 0x00
#define LPM_MODE 0x01
#define OFFLINE 0x03
#define SEND_APDU_DATA_OFFSET 3
#define UIM_IMSI_PATH_SIZE 4
#define UIM_IMSI_M_RAW_SIZE           10
#define UIM_IMSI_M_PARSED_SIZE        16
#define UIM_FILEID_EF_IMSI      0x6F07

static char temp_char_array[TEMP_ARRAY_SIZE];
static int session_type_gw = 1;

static void pb_print_indication_register_resp(pbm_indication_register_resp_msg_v01* pb_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void pb_print_record_update_ind(pbm_record_update_ind_msg_v01* pb_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message);
static void pb_print_pb_ready_ind(pbm_pb_ready_ind_msg_v01* pb_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message);


int qmi_simple_ril_misc_dms_modem_mode_command_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    char* arg;

    dms_set_operating_mode_req_msg_v01 set_mode_request_msg;
    void * req_msg_payload = NULL;
    int req_msg_payload_len;
    int msg_id;
    int err = QMI_SIMPLE_RIL_ERR_ARG;

    memset(&set_mode_request_msg, 0, sizeof(set_mode_request_msg));
    msg_id = -1;
    if (cmd_params->info_set->nof_entries == 1)
        {
        arg = cmd_params->info_set->entries[0];
        if (0 == strcmp("status", arg) )
            { // get mode
            msg_id = QMI_DMS_GET_OPERATING_MODE_REQ_V01;
            req_msg_payload = NULL;
            req_msg_payload_len = 0;
            }
        else if ((0 == strcmp("online", arg) ) || (0 == strcmp("on", arg) ))
            { // set online
            set_mode_request_msg.operating_mode = ONLINE_MODE;
            req_msg_payload = &set_mode_request_msg;
            req_msg_payload_len = sizeof(set_mode_request_msg);
            msg_id = QMI_DMS_SET_OPERATING_MODE_REQ_V01;
            }
        else if ((0 == strcmp("offline", arg) ) || (0 == strcmp("off", arg) ))
            { // set online
            set_mode_request_msg.operating_mode = OFFLINE;
            req_msg_payload = &set_mode_request_msg;
            req_msg_payload_len = sizeof(set_mode_request_msg);
            msg_id = QMI_DMS_SET_OPERATING_MODE_REQ_V01;
            }
	else if ((0 == strcmp("lpm", arg) ))
            { // set lpm
            set_mode_request_msg.operating_mode = LPM_MODE;
            req_msg_payload = &set_mode_request_msg;
            req_msg_payload_len = sizeof(set_mode_request_msg);
            msg_id = QMI_DMS_SET_OPERATING_MODE_REQ_V01;
            }
        else if ((0 == strcmp("get_time", arg) ))
            { // get time
            req_msg_payload = NULL;
            req_msg_payload_len = 0;
            msg_id = QMI_DMS_GET_TIME_REQ_V01;
            }
        else if ((0 == strcmp("reset", arg) ))
            { // reset
            set_mode_request_msg.operating_mode = DMS_OP_MODE_RESETTING_V01;
            req_msg_payload = &set_mode_request_msg;
            req_msg_payload_len = sizeof(set_mode_request_msg);
            msg_id = QMI_DMS_SET_OPERATING_MODE_REQ_V01;
            }
    
        if (msg_id != -1)
            {
            memset(&req_params, 0, sizeof(req_params));
            req_params.service_id = QMI_UTIL_SVC_DMS;
            req_params.message_id = msg_id;
            req_params.message_specific_payload = req_msg_payload;
            req_params.message_specific_payload_len = req_msg_payload_len;
    
            ack_info->request_id = qmi_util_post_request(&req_params);
    
            if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
                {
                err = QMI_SIMPLE_RIL_ERR_NONE;
                }
            }
        }

    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }
    return 0;
    }

int qmi_simple_ril_misc_dms_get_dev_id_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;

    memset(&req_params, 0, sizeof(req_params));
    req_params.service_id = QMI_UTIL_SVC_DMS;
    req_params.message_id = QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01;
    req_params.message_specific_payload = NULL;
    req_params.message_specific_payload_len = 0;

    ack_info->request_id = qmi_util_post_request(&req_params);

    return 0;
}

static char* qmi_simple_ril_dms_get_modem_mode_str(int state)
    {
    char* res;
    switch (state)
        {
        case 0x00:
            res = "ONLINE";
            break;

        case 0x01:
            res = "LPM";
            break;

        case 0x02:
            res = "FTM";
            break;

        case 0x03:
            res = "OFFLINE";
            break;

        case 0x04:
            res = "RESETTING";
            break;

        case 0x05:
            res = "SHUTTING DOWN";
            break;

        default:
            res = "UNKNOWN";
            break;
        }
    return res;
    }

int qmi_simple_ril_misc_dms_modem_mode_qmi_message_handler (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    dms_get_operating_mode_resp_msg_v01* get_mode_resp;
    dms_set_operating_mode_resp_msg_v01* set_mode_resp;
    dms_get_time_resp_msg_v01* get_time_resp;
    dms_reset_resp_msg_v01* reset_resp;
    char mode_buf[150];
    double elapsed_time=0;
    int iter_i=0;
        
    switch (qmi_msg_info->message_id)
        {
        case QMI_DMS_GET_OPERATING_MODE_RESP_V01:
            get_mode_resp = (dms_get_operating_mode_resp_msg_v01*)qmi_msg_info->message_specific_payload;
            sprintf(mode_buf,"modem is %s", qmi_simple_ril_dms_get_modem_mode_str(get_mode_resp->operating_mode) );
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, mode_buf);
            qmi_util_logln1("qmi_simple_ril_misc_dms_modem_mode_qmi_message_handler modem status ", get_mode_resp->operating_mode);
            break;

        case QMI_DMS_SET_OPERATING_MODE_RESP_V01:
            set_mode_resp = (dms_set_operating_mode_resp_msg_v01*)qmi_msg_info->message_specific_payload;
            break;

        case QMI_DMS_GET_TIME_RESP_V01:
            get_time_resp = (dms_get_time_resp_msg_v01*)qmi_msg_info->message_specific_payload;                        

            
            for(iter_i=5;iter_i>=0;iter_i--)
                {
                elapsed_time = elapsed_time * 256.0 + (double) get_time_resp->device_time.time_count[iter_i];
                }
            elapsed_time = (elapsed_time * 1.25)/1000.0;
            
            
            sprintf(mode_buf,"seconds elapsed from start of GPS time = %lf, time_source = %d", elapsed_time,get_time_resp->device_time.time_source);            
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, mode_buf);
            break;

        case QMI_DMS_RESET_RESP_V01:
            reset_resp = (dms_reset_resp_msg_v01*)qmi_msg_info->message_specific_payload;
            break;

        default: // ignore
            break;
        }
    return 0;
    }

int qmi_simple_ril_misc_dms_get_device_id_message_handler
(
    qmi_util_service_message_info* qmi_msg_info,
    qmi_simple_ril_cmd_completion_info* uplink_message
)
{
    dms_get_device_serial_numbers_resp_msg_v01* resp;
    char buf[512];

    resp = (dms_get_device_serial_numbers_resp_msg_v01*)qmi_msg_info->message_specific_payload;

    if (resp->esn_valid)
    {
        sprintf(buf,"esn is %s", resp->esn);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, buf);
        qmi_util_logln1s("qmi_simple_ril_misc_dms_get_device_id_message_handler esn: ", buf);
    }

    if (resp->imei_valid)
    {
        sprintf(buf,"imei is %s", resp->imei);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, buf);
        qmi_util_logln1s("qmi_simple_ril_misc_dms_get_device_id_message_handler imei: ", buf);
    }

    if (resp->imeisv_svn_valid)
    {
        sprintf(buf,"imeisv_svn is %s", resp->imeisv_svn);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, buf);
        qmi_util_logln1s("qmi_simple_ril_misc_dms_get_device_id_message_handler imeisv_svn: ", buf);
    }

    if (resp->meid_valid)
    {
        sprintf(buf,"meid is %s", resp->meid);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, buf);
        qmi_util_logln1s("qmi_simple_ril_misc_dms_get_device_id_message_handler meid: ", buf);
    }
    return 0;
}

// ------------- UIM -----------------------------------------------------------------------------------------------------------------------------
int qmi_simple_ril_misc_uim_get_imsi_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    int slot = 0;
    char* session_type;
    // This request would fail unless you have done a Verify PIN
    // and the card is in READY state.
    uim_read_transparent_req_msg_v01 read_params;
    void * req_msg_payload = NULL;
    int req_msg_payload_len = 0;
    int err = QMI_SIMPLE_RIL_ERR_NONE;
    uint8_t gw_path[] = {0x3F, 0x00, 0x7F, 0x20};
    uint8_t cdma_path[] = {0x3F, 0x00, 0x7F, 0x25};

    memset(&read_params, 0, sizeof(uim_read_transparent_req_msg_v01));
    if (cmd_params->info_set->nof_entries == 2)
    {
        slot = atoi(cmd_params->info_set->entries[0]);
        session_type = cmd_params->info_set->entries[1];
        qmi_util_logln1("qmi_simple_ril_misc_uim_get_imsi_handler Slot: ", slot);
        qmi_util_logln1s("qmi_simple_ril_misc_uim_get_imsi_handler Session type: ", session_type);
        if (0 == strcmp("gw", session_type) )
        {
            session_type_gw = 1;
            switch (slot)
            {
              case 0:
              read_params.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
              break;

              case 1:
              read_params.session_information.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
              break;

              case 2:
              read_params.session_information.session_type = UIM_SESSION_TYPE_TERTIARY_GW_V01;
              break;

              default:
              err = QMI_SIMPLE_RIL_ERR_ARG;
              break;
            }
            read_params.file_id.file_id = UIM_FILEID_EF_IMSI;
            read_params.file_id.path_len = UIM_IMSI_PATH_SIZE;
            read_params.session_information.aid_len = 0;
            qmi_simple_ril_misc_uim_flip_data(read_params.file_id.path,
                          (uint8_t*)gw_path,
                          read_params.file_id.path_len);
         }
         else if (0 == strcmp("1x", session_type) )
         {
            session_type_gw = 0;
            switch (slot)
            {
              case 0:
              read_params.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
              break;

              case 1:
              read_params.session_information.session_type = UIM_SESSION_TYPE_SECONDARY_1X_V01;
              break;

              case 2:
              read_params.session_information.session_type = UIM_SESSION_TYPE_TERTIARY_1X_V01;
              break;

              default:
              err = QMI_SIMPLE_RIL_ERR_ARG;
              break;
            }
            read_params.file_id.file_id = UIM_FILEID_EF_IMSI;
            read_params.file_id.path_len = UIM_IMSI_PATH_SIZE;
            read_params.session_information.aid_len = 0;
            qmi_simple_ril_misc_uim_flip_data(read_params.file_id.path,
                          (uint8_t*)cdma_path,
                          read_params.file_id.path_len);
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
       if ( err == QMI_SIMPLE_RIL_ERR_NONE )
       {
          req_msg_payload = &read_params;
          req_msg_payload_len = sizeof(read_params);

          if (req_msg_payload != NULL)
          {
            memset(&req_params, 0, sizeof(req_params));
            req_params.service_id = QMI_UTIL_SVC_UIM;
            req_params.message_id = QMI_UIM_READ_TRANSPARENT_REQ_V01;
            req_params.message_specific_payload = req_msg_payload;
            req_params.message_specific_payload_len = req_msg_payload_len;

            ack_info->request_id = qmi_util_post_request(&req_params);

            if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
            {
               qmi_util_logln0("qmi_simple_ril_misc_uim_get_imsi_handler QMI_UIM_READ_TRANSPARENT_REQ_V01 sent!");
               err = QMI_SIMPLE_RIL_ERR_NONE;
            }
          }
        }

      if (err != QMI_SIMPLE_RIL_ERR_NONE)
      {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
      }
      return 0;
}

int qmi_simple_ril_misc_uim_read_transp_message_handler
(
    qmi_util_service_message_info* qmi_msg_info,
    qmi_simple_ril_cmd_completion_info* uplink_message
)
{
    char                            * imsi_ptr    = NULL;
    int                               imsi_len = 0;
    char                            buf[50];
    uim_read_transparent_resp_msg_v01* resp;
    if ( ( qmi_msg_info->error_code == QMI_NO_ERR ) && ( qmi_msg_info->message_id == QMI_UIM_READ_TRANSPARENT_REQ_V01 ) )
    {
      qmi_util_logln1("qmi_simple_ril_misc_uim_read_transp_message_handler session_type_gw: ", session_type_gw);
      resp = (uim_read_transparent_resp_msg_v01*)qmi_msg_info->message_specific_payload;
      if ( session_type_gw == 1 )
      {
        imsi_ptr = qmi_simple_ril_misc_uim_parse_gw_imsi(
                     resp->read_result.content,
                     resp->read_result.content_len,
                     &imsi_len);
      }
      else
      {
        imsi_ptr = qmi_simple_ril_misc_uim_parse_1x_imsi(
                     resp->read_result.content,
                     resp->read_result.content_len,
                     &imsi_len);
      }
      if(imsi_ptr)
      {
          snprintf(buf, sizeof(buf), "IMSI is %s", imsi_ptr);
          qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, buf);
      }
    }
    return 0;
}

char qmi_simple_ril_misc_uim_bin_to_hexchar
(
  uint8_t ch
)
{
  if (ch < 0x0a)
  {
    return (ch + '0');
  }
  return (ch + 'a' - 10);
}

void qmi_simple_ril_misc_uim_flip_data
(
  uint8_t                      * des,
  const uint8_t                * src,
  uint32_t                       data_len
)
{
  unsigned int     i             =   0;
  unsigned short   temp_path     =   0;

  for(i = 0; i < (data_len + 1); i += 2)
  {
    temp_path = (*(src + i) << 8) |
                (*(src + i + 1));

    memcpy(des, (unsigned char*)&temp_path, sizeof(unsigned short));
    des += sizeof(unsigned short);
  }
} /* qcril_uim_qmi_flip_data */

char * qmi_simple_ril_misc_uim_parse_gw_imsi
(
  const unsigned char * raw_imsi_ptr,
  unsigned short        raw_imsi_len,
  int                 * parsed_imsi_len_ptr
)
{
  int             src             = 0;
  int             dst             = 0;
  char          * parsed_imsi_ptr = NULL;

  /* Sanity check on input parameters */
  if ((raw_imsi_ptr == NULL) || (parsed_imsi_len_ptr == NULL))
  {
    return NULL;
  }

  /* Check for the length of IMSI bytes in the first byte */
  *parsed_imsi_len_ptr = *raw_imsi_ptr;
  if (*parsed_imsi_len_ptr >= raw_imsi_len)
  {
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  /* Allocate required amount of memory for IMSI in ASCII string format,
     note that it is freed by the caller */
  parsed_imsi_ptr = malloc((2 * (*parsed_imsi_len_ptr)));
  if (parsed_imsi_ptr == NULL)
  {
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  /* Compose IMSI */
  memset(parsed_imsi_ptr, 0, (2 * (*parsed_imsi_len_ptr)));
  for (src = 1, dst = 0;
      (src <= (*parsed_imsi_len_ptr)) && (dst < ((*parsed_imsi_len_ptr) * 2));
       src++)
  {
    /* Only process lower part of byte for second and subsequent bytes */
    if (src > 1)
    {
      parsed_imsi_ptr[dst] = qmi_simple_ril_misc_uim_bin_to_hexchar(raw_imsi_ptr[src] & 0x0F);
      dst++;
    }
    /* Process upper part of byte for all bytes */
    parsed_imsi_ptr[dst] = qmi_simple_ril_misc_uim_bin_to_hexchar(raw_imsi_ptr[src] >> 4);
    dst++;
  }

  /* Update IMSI length in bytes - parsed IMSI in ASCII is raw times 2 */
  *parsed_imsi_len_ptr *= 2;

  return parsed_imsi_ptr;
}

char * qmi_simple_ril_misc_uim_parse_1x_imsi
(
  const unsigned char * raw_imsi_ptr,
  unsigned short        raw_imsi_len,
  int                 * parsed_imsi_len_ptr
)
{
  uint8_t     i             = 0;
  uint16_t    mcc           = 0;
  uint8_t     mnc           = 0;
  uint32_t    min1          = 0;
  uint16_t    min2          = 0;
  uint16_t    second_three  = 0;
  uint8_t     thousands     = 0xFF;
  uint16_t    last_three    = 0;
  uint8_t     min_to_num[]  = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
  uint8_t     bcd_to_num[]  = { 0xFF, '1', '2', '3', '4', '5', '6', '7', '8',
                              '9', '0', 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  char          * parsed_imsi_ptr = NULL;

  /* Sanity check on input parameters */
  if ((raw_imsi_ptr == NULL) || (parsed_imsi_len_ptr == NULL))
  {
    return NULL;
  }

  /* Check for the minumim length of IMSI_M expected */
  if (raw_imsi_len < UIM_IMSI_M_RAW_SIZE)
  {
    return NULL;
  }

  /* Sanity check for IMSI_M_PROGRAMMED indicator */
  if ((raw_imsi_ptr[7] & 0x80) == 0)
  {
    return NULL;
  }

  /* Update parsed length - null terminated ASCII string length */
  *parsed_imsi_len_ptr = UIM_IMSI_M_PARSED_SIZE;

  /* Allocate the number of bytes */
  parsed_imsi_ptr = malloc(*parsed_imsi_len_ptr);
  if (parsed_imsi_ptr == NULL)
  {
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  memset(parsed_imsi_ptr, 0, *parsed_imsi_len_ptr);

  /* Parse certain parameters */
  mcc           = (((raw_imsi_ptr[9] & 0x03) << 8) + raw_imsi_ptr[8]);
  mnc           = raw_imsi_ptr[6] & 0x7F;
  min1          = ((raw_imsi_ptr[5] <<16) +(raw_imsi_ptr[4] <<8) + raw_imsi_ptr[3]);
  min2          = ((raw_imsi_ptr[2] <<8)  + raw_imsi_ptr[1]);
  /* left 10 bits */
  second_three  = (min1 & 0x00FFC000) >> 14;
  /* middle 4 bits */
  thousands     = (min1 & 0x00003C00) >> 10;
  /* right 10 bits */
  last_three    = (min1 & 0x000003FF);
  thousands     = bcd_to_num[thousands];

  if ((mcc > 999) || (mnc > 99) || (min2 > 999) || (thousands == 0xFF) ||
      (second_three > 999) || (last_three > 999))
  {
    *parsed_imsi_len_ptr = 0;
    return NULL;
  }

  /* Construct ASCII IMSI_M, format:
     <3_digit_MCC><2_digit_11_12_digits><LS_10_digits_IMSI> */
  /* Update MCC - 3 digits */
  parsed_imsi_ptr[i++] = min_to_num[mcc/100];
  mcc %= 100;
  parsed_imsi_ptr[i++] = min_to_num[mcc/10];
  parsed_imsi_ptr[i++] = min_to_num[mcc%10];
  /* Update MNC - 2 digits */
  parsed_imsi_ptr[i++] = min_to_num[mnc/10];
  parsed_imsi_ptr[i++] = min_to_num[mnc%10];
  /* Update the first 3 digits of IMSI */
  parsed_imsi_ptr[i++] = min_to_num[min2/100];
  min2 %= 100;
  parsed_imsi_ptr[i++] = min_to_num[min2/10];
  parsed_imsi_ptr[i++] = min_to_num[min2%10];
  /* Update the last 7 digits of IMSI */
  parsed_imsi_ptr[i++] = min_to_num[second_three/100];
  second_three %= 100;
  parsed_imsi_ptr[i++] = min_to_num[second_three/10];
  parsed_imsi_ptr[i++] = min_to_num[second_three%10];
  parsed_imsi_ptr[i++] = thousands;
  parsed_imsi_ptr[i++] = min_to_num[last_three/100];
  last_three %= 100;
  parsed_imsi_ptr[i++] = min_to_num[last_three/10];
  parsed_imsi_ptr[i++] = min_to_num[last_three%10];

  return parsed_imsi_ptr;
} /* qcril_uim_parse_1x_imsi */

int qmi_simple_ril_misc_card_command_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    char* arg;

    void * req_msg_payload = NULL;
    int req_msg_payload_len;
    int msg_id=-1;
    int err = QMI_SIMPLE_RIL_ERR_ARG;

    if (cmd_params->info_set->nof_entries == 1)
        {
        arg = cmd_params->info_set->entries[0];
        if (0 == strcmp("status", arg) )
            { // get mode
            msg_id = QMI_UIM_GET_CARD_STATUS_REQ_V01;
            req_msg_payload = NULL;
            req_msg_payload_len = 0;
            }
   
        if (msg_id >= 0)
            {
            memset(&req_params, 0, sizeof(req_params));
            req_params.service_id = QMI_UTIL_SVC_UIM;
            req_params.message_id = msg_id;
            req_params.message_specific_payload = req_msg_payload;
            req_params.message_specific_payload_len = req_msg_payload_len;
    
            ack_info->request_id = qmi_util_post_request(&req_params);
    
            if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
                {
                err = QMI_SIMPLE_RIL_ERR_NONE;
                }
            }
        }

    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }
    return 0;
    }

int qmi_simple_ril_misc_set_srv_status_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    char* arg;
    // This request would fail unless you have done a Verify PIN2
    // The command to do so in Simple RIL is "verify_pin"    
    uim_set_service_status_req_msg_v01 set_srv_status_req_msg;
    void * req_msg_payload = NULL;
    int req_msg_payload_len = 0;
    int err = QMI_SIMPLE_RIL_ERR_ARG;

    memset(&set_srv_status_req_msg, 0, sizeof(set_srv_status_req_msg));
    if (cmd_params->info_set->nof_entries == 1)
        {
        arg = cmd_params->info_set->entries[0];
        if (0 == strcmp("gw", arg) )
            { // SESSION_TYPE_PRIMARY_GW
            set_srv_status_req_msg.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
            set_srv_status_req_msg.fdn_status_valid = TRUE;
            set_srv_status_req_msg.fdn_status = 0;
            req_msg_payload = &set_srv_status_req_msg;
            req_msg_payload_len = sizeof(set_srv_status_req_msg);
            }
        else if (0 == strcmp("1x", arg) )
            { // SESSION_TYPE_PRIMARY_1X
            set_srv_status_req_msg.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
            set_srv_status_req_msg.fdn_status_valid = TRUE;
            set_srv_status_req_msg.fdn_status = 0;
            req_msg_payload = &set_srv_status_req_msg;
            req_msg_payload_len = sizeof(set_srv_status_req_msg);
            }

   
        if (req_msg_payload != NULL)
            {
            memset(&req_params, 0, sizeof(req_params));
            req_params.service_id = QMI_UTIL_SVC_UIM;
            req_params.message_id = QMI_UIM_SET_SERVICE_STATUS_REQ_V01;
            req_params.message_specific_payload = req_msg_payload;
            req_params.message_specific_payload_len = req_msg_payload_len;
    
            ack_info->request_id = qmi_util_post_request(&req_params);
    
            if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
                {
                err = QMI_SIMPLE_RIL_ERR_NONE;
                }
            }
        }

    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }
    return 0;
    }

int qmi_simple_ril_misc_get_fdn_status_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    char* arg;
    
    uim_get_service_status_req_msg_v01 get_srv_status_req_msg;
    void * req_msg_payload = NULL;
    int req_msg_payload_len = 0;
    int err = QMI_SIMPLE_RIL_ERR_ARG;

    memset(&get_srv_status_req_msg, 0, sizeof(get_srv_status_req_msg));
    if (cmd_params->info_set->nof_entries == 1)
        {
        arg = cmd_params->info_set->entries[0];
        if (0 == strcmp("gw", arg) )
            { // SESSION_TYPE_PRIMARY_GW
            get_srv_status_req_msg.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
            get_srv_status_req_msg.mask = 0x01;
            req_msg_payload = &get_srv_status_req_msg;
            req_msg_payload_len = sizeof(get_srv_status_req_msg);
            }
        else if (0 == strcmp("1x", arg) )
            { // SESSION_TYPE_PRIMARY_1X
            get_srv_status_req_msg.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
            get_srv_status_req_msg.mask = 0x01;
            req_msg_payload = &get_srv_status_req_msg;
            req_msg_payload_len = sizeof(get_srv_status_req_msg);
            }

   
        if (req_msg_payload != NULL)
            {
            memset(&req_params, 0, sizeof(req_params));
            req_params.service_id = QMI_UTIL_SVC_UIM;
            req_params.message_id = QMI_UIM_GET_SERVICE_STATUS_REQ_V01;
            req_params.message_specific_payload = req_msg_payload;
            req_params.message_specific_payload_len = req_msg_payload_len;
    
            ack_info->request_id = qmi_util_post_request(&req_params);
    
            if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
                {
                err = QMI_SIMPLE_RIL_ERR_NONE;
                }
            }
        }

    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }
    return 0;
    }

    int qmi_simple_ril_misc_verify_pin_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    char* arg;
    
    uim_verify_pin_req_msg_v01 verify_pin_req_msg;
    void * req_msg_payload = NULL;
    int req_msg_payload_len = 0;
    int err = QMI_SIMPLE_RIL_ERR_ARG;

    memset(&verify_pin_req_msg, 0, sizeof(verify_pin_req_msg));
    do
        {
        if (cmd_params->info_set->nof_entries == 3)
            {
            // First argument is the session type (gw/1x)
            arg = cmd_params->info_set->entries[0];
            if (0 == strcmp("gw", arg) )
                { // SESSION_TYPE_PRIMARY_GW
                verify_pin_req_msg.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
                }
            else if (0 == strcmp("1x", arg) )
                { // SESSION_TYPE_PRIMARY_1X
                verify_pin_req_msg.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
                }
            else
                {
                break;
                }
    
            // Second argument is the Pin Id (e.g. 2 for PIN2)
            arg = cmd_params->info_set->entries[1];
            if (strlen(arg) != 1)
                {
                break;
                }
            verify_pin_req_msg.verify_pin.pin_id = arg[0] - '0';

            // Third argument is the PIN Value (e.g. "1234")
            arg = cmd_params->info_set->entries[2];
            if (strlen(arg) >= QMI_UIM_PIN_MAX_V01)
                {
                break;
                }
            strcpy(verify_pin_req_msg.verify_pin.pin_value, arg);

            req_msg_payload = &verify_pin_req_msg;
            req_msg_payload_len = sizeof(verify_pin_req_msg);
            memset(&req_params, 0, sizeof(req_params));
            req_params.service_id = QMI_UTIL_SVC_UIM;
            req_params.message_id = QMI_UIM_VERIFY_PIN_REQ_V01;
            req_params.message_specific_payload = req_msg_payload;
            req_params.message_specific_payload_len = req_msg_payload_len;
    
            ack_info->request_id = qmi_util_post_request(&req_params);
    
            if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
                {
                err = QMI_SIMPLE_RIL_ERR_NONE;
                }
            }
        }while(0);

    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }
    return 0;
    }

static int qmi_simple_ril_misc_hexstr_to_bin(char* hex_string)
    {
    int len = 0;
    int offset = 0;
    int i = 0;
    int temp_bin = 0;
    int bin_out = 0;

    len = strlen(hex_string);

    if(len > 0 && len <= 2)
        {
        offset = 0;
        }
    else if(len > 2 && len <= 4)
        {
        // If the hex string starts with "0x"
        offset = 2; 
        }
    else
        {
        return -1;
        }
    
    for(i = offset; i < len; i++ )
        {
        if (hex_string[i] >= '0' && hex_string[i] <= '9')
            {
            temp_bin = hex_string[i] - '0';
            }
        else if (hex_string[i] >= 'A' && hex_string[i] <= 'F') 
            {
            temp_bin = hex_string[i] - 'A' + 10;
            }
        else if (hex_string[i] >= 'a' && hex_string[i] <= 'f')  
            {
            temp_bin = hex_string[i] - 'a' + 10;
            }
        else
            {
              return -1;
            }

        if(i == offset && (len - offset) > 1)
            {
            bin_out = (temp_bin << 4) & 0xF0;
            }
        else 
            {
            bin_out = bin_out | (temp_bin & 0x0F);
            }
        }
    
    return bin_out;    
    }

int qmi_simple_ril_misc_send_apdu_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    
    uim_send_apdu_req_msg_v01 send_apdu_req_msg;
    void * req_msg_payload = NULL;
    int req_msg_payload_len = 0;
    int err = QMI_SIMPLE_RIL_ERR_ARG;
    int arg = 0;
    int apdu_len_low = 0;
    int apdu_len_high = 0;
    int i = 0;

    memset(&send_apdu_req_msg, 0, sizeof(send_apdu_req_msg));

    // Check for minimum number of arguments for SEND APDU command
    if (cmd_params->info_set->nof_entries < 7)
        {
        goto send_error;
        }

    // First argument is the slot
    arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[0]);
    if(arg == -1)
        {
        goto send_error;
        }
    send_apdu_req_msg.slot = (uint8_t)arg;

    // Second argument is the length of APDU
    apdu_len_low = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[1]);
    apdu_len_high = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[2]);
    if(apdu_len_low == -1 || apdu_len_high == -1)
        {
        goto send_error;
        }

    send_apdu_req_msg.apdu_len = (uint32_t)(((apdu_len_high << 8) & 0xFF00) | apdu_len_low);
    if (cmd_params->info_set->nof_entries < (int)send_apdu_req_msg.apdu_len + SEND_APDU_DATA_OFFSET)
        {
        goto send_error;
        }

    // Third argument onward is the APDU data
    for (i = 0; i < (int)send_apdu_req_msg.apdu_len && i < QMI_UIM_APDU_DATA_MAX_V01; i++)
        {
        arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[i + SEND_APDU_DATA_OFFSET]);
        if(arg == -1)
            {
            goto send_error;
            }
        send_apdu_req_msg.apdu[i] = (uint8_t)arg;
        }
    
    // Final (optional) argument is channel id
    if (cmd_params->info_set->nof_entries > (int)send_apdu_req_msg.apdu_len + SEND_APDU_DATA_OFFSET)
        {
        arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[i+SEND_APDU_DATA_OFFSET]);
        if(arg == -1)
            {
            goto send_error;
            }
        send_apdu_req_msg.channel_id = (uint8_t)arg;
        send_apdu_req_msg.channel_id_valid = TRUE;
        }
    else
        {
        send_apdu_req_msg.channel_id_valid = FALSE;
        }

    req_msg_payload = &send_apdu_req_msg;
    req_msg_payload_len = sizeof(send_apdu_req_msg);
    memset(&req_params, 0, sizeof(req_params));
    req_params.service_id = QMI_UTIL_SVC_UIM;
    req_params.message_id = QMI_UIM_SEND_APDU_REQ_V01;
    req_params.message_specific_payload = req_msg_payload;
    req_params.message_specific_payload_len = req_msg_payload_len;

    ack_info->request_id = qmi_util_post_request(&req_params);

    if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
        {
        err = QMI_SIMPLE_RIL_ERR_NONE;
        }

send_error:
    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }

    return 0;
    }

int qmi_simple_ril_misc_logical_channel_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
    qmi_util_request_params req_params;
    
    uim_logical_channel_req_msg_v01 logical_chan_req_msg;
    void * req_msg_payload = NULL;
    int req_msg_payload_len = 0;
    int err = QMI_SIMPLE_RIL_ERR_ARG;
    int arg = 0;
    int i = 0;

    memset(&logical_chan_req_msg, 0, sizeof(logical_chan_req_msg));

    qmi_util_logln1("Number of entries ", cmd_params->info_set->nof_entries);

    // Check for minimum number of arguments for LOGICAL CHANNEL command
    if (cmd_params->info_set->nof_entries < 2)
        {
        goto send_error;
        }

    // First argument is the slot id
    arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[0]);
    if(arg == -1)
        {
        goto send_error;
        }
    logical_chan_req_msg.slot = (uint8_t)arg;

    // If more than 3 arguments then command is for opening a channel
    if(cmd_params->info_set->nof_entries > 3)
        {
        logical_chan_req_msg.aid_valid = TRUE;

        arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[1]);
        if(arg == -1)
            {
            goto send_error;
            }
        logical_chan_req_msg.aid_len = (uint32_t)arg;

        if (cmd_params->info_set->nof_entries < (int)logical_chan_req_msg.aid_len + 2)
            {
            goto send_error;
            }

        // Third argument onward is the AID
        for (i = 0; i < (int)logical_chan_req_msg.aid_len && i < QMI_UIM_AID_MAX_V01; i++)
            {
            arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[i + 2]);
            if(arg == -1)
                {
                goto send_error;
                }
            logical_chan_req_msg.aid[i] = (uint8_t)arg;
            }

        // Parse the file_control_information argument if present
        if (cmd_params->info_set->nof_entries > (int)logical_chan_req_msg.aid_len + 2)
            {
            arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[i + 2]);
            if(arg == -1)
                {
                goto send_error;
                }
            logical_chan_req_msg.file_control_information_valid = TRUE;
            logical_chan_req_msg.file_control_information = (uim_file_control_information_enum_v01)arg;
            }
        else
            {
            logical_chan_req_msg.file_control_information_valid = FALSE;
            }

        // Mark channel id as not valid
        logical_chan_req_msg.channel_id_valid = FALSE;
        }
    else
        {
        // The command is for closing a channel
        logical_chan_req_msg.channel_id_valid = TRUE;
        arg = qmi_simple_ril_misc_hexstr_to_bin(cmd_params->info_set->entries[1]);
        if(arg == -1)
            {
            goto send_error;
            }
        logical_chan_req_msg.channel_id = (uint8_t)arg;

        // Mark aid and FCI as not valid
        logical_chan_req_msg.aid_valid = FALSE;
        logical_chan_req_msg.file_control_information_valid = FALSE;
        }

    req_msg_payload = &logical_chan_req_msg;
    req_msg_payload_len = sizeof(logical_chan_req_msg);
    memset(&req_params, 0, sizeof(req_params));
    req_params.service_id = QMI_UTIL_SVC_UIM;
    req_params.message_id = QMI_UIM_LOGICAL_CHANNEL_REQ_V01;
    req_params.message_specific_payload = req_msg_payload;
    req_params.message_specific_payload_len = req_msg_payload_len;

    ack_info->request_id = qmi_util_post_request(&req_params);

    if (QMI_UTIL_INVALID_REQUEST_ID != ack_info->request_id)
        {
        err = QMI_SIMPLE_RIL_ERR_NONE;
        }

send_error:
    if (err != QMI_SIMPLE_RIL_ERR_NONE)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, err);
        }

    return 0;
    }

int qmi_simple_ril_misc_uim_send_apdu_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    uim_send_apdu_resp_msg_v01* send_apdu_resp;
    int i = 0;

    if (qmi_msg_info->error_code == QMI_NO_ERR)
        {
        if(NULL != qmi_msg_info->message_specific_payload)
            {
            send_apdu_resp = (uim_send_apdu_resp_msg_v01*)qmi_msg_info->message_specific_payload;;
            if (send_apdu_resp->resp.result == QMI_RESULT_SUCCESS_V01)
                {
                sprintf(temp_char_array, "send_apdu request returned success\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                if(send_apdu_resp->apdu_valid)
                    {
                    sprintf(temp_char_array, "length of APDU response: 0x%x\n", send_apdu_resp->apdu_len);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    sprintf(temp_char_array, "APDU response data: \n");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    for (i = 0; i < (int)send_apdu_resp->apdu_len; i++)
                        {
                        sprintf(temp_char_array, "0x%x ", send_apdu_resp->apdu[i]);
                        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                        }
                    }
                }
            else
                {
                sprintf(temp_char_array, "send_apdu request returned error code: 0x%x\n", send_apdu_resp->resp.error);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
        else
            {
            sprintf(temp_char_array, "send_apdu response has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "ERROR in sending APDU, error code :0x%x\n",qmi_msg_info->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }

    return 0;
    }

int qmi_simple_ril_misc_uim_logical_channel_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                      qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    uim_logical_channel_resp_msg_v01* logical_chan_resp;
    int i = 0;

    if (qmi_msg_info->error_code == QMI_NO_ERR)
        {
        if(NULL != qmi_msg_info->message_specific_payload)
            {
            logical_chan_resp = (uim_logical_channel_resp_msg_v01*)qmi_msg_info->message_specific_payload;;
            if (logical_chan_resp->resp.result == QMI_RESULT_SUCCESS_V01)
                {
                sprintf(temp_char_array, "logical_channel request returned success\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                if(logical_chan_resp->channel_id_valid)
                    {
                    sprintf(temp_char_array, "channel_id: 0x%x\n", logical_chan_resp->channel_id);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }

                if(logical_chan_resp->card_result_valid)
                    {
                    sprintf(temp_char_array, "SW1: 0x%x\n", logical_chan_resp->card_result.sw1);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    sprintf(temp_char_array, "SW2: 0x%x\n", logical_chan_resp->card_result.sw2);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }

                if(logical_chan_resp->select_response_valid)
                    {
                    sprintf(temp_char_array, "SELECT response: \n");
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);

                    for (i = 0; i < (int)logical_chan_resp->select_response_len && i < QMI_UIM_SELECT_RESPONSE_MAX_V01; i++)
                        {
                        sprintf(temp_char_array, "0x%x ", logical_chan_resp->select_response[i]);
                        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                        }
                    }
                }
            else
                {
                sprintf(temp_char_array, "logical_channel req returned error code: 0x%x\n", logical_chan_resp->resp.error);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
        else
            {
            sprintf(temp_char_array, "logical_channel response has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "ERROR in sending APDU, error code: 0x%x\n",qmi_msg_info->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }

    return 0;
    }

int qmi_simple_ril_misc_uim_card_status_qmi_message_handler (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    uim_get_card_status_resp_msg_v01* card_status_msg = (uim_get_card_status_resp_msg_v01*)qmi_msg_info->message_specific_payload;
    char info_buf[128];
    char *ext;
    int i;
        
    if (card_status_msg->card_status_valid && (card_status_msg->card_status.card_info_len > 0))
    {
      for ( i = 0; i < card_status_msg->card_status.card_info_len; i++ )
      {
        snprintf(info_buf, sizeof(info_buf), "CARD %d", i );
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
        // card state        
        snprintf(info_buf, sizeof(info_buf), "card state %s", (card_status_msg->card_status.card_info[i].card_state == 0) ? "Absent" : ((card_status_msg->card_status.card_info[i].card_state == 1) ? "Present" : "Error") );
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
    
        // upin state
        switch (card_status_msg->card_status.card_info[i].upin.pin_state)
            {
            case 0:  // - Unknown
                ext = "Unknown";
                break;
    
            case 1: // - Enabled and not verified
                ext = "Enabled and not verified";
                break;
    
            case 2: // - Enabled and verified
                ext = "Enabled and verified";
                break;
    
            case 3: //  - Disabled
                ext = "Disabled";
                break;
    
            case 4: // - Blocked
                ext = "Blocked";
                break;
    
            case 5: // - Permanently blocked
                ext = "Permanently blocked";
                break;
    
            default:
                ext = "???";
                break;
            }
        snprintf(info_buf, sizeof(info_buf), "upin state %s", ext );
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
    
        // upin entries
        snprintf(info_buf, sizeof(info_buf), "upin retries %d", card_status_msg->card_status.card_info[i].upin.pin_retries );
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
    
        // upuk entries
        snprintf(info_buf, sizeof(info_buf), "upuk retries %d", card_status_msg->card_status.card_info[i].upin.puk_retries );
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
    
        if(card_status_msg->card_status.card_info[i].app_info_len > 0)
            {
            // perso state
            switch (card_status_msg->card_status.card_info[i].app_info[0].perso_state)
                {
                case 0:  // - Unknown
                    ext = "Unknown";
                    break;
        
                case 1: // - Personalization operation is in progress
                    ext = "Personalization operation is in progress";
                    break;
        
                case 2: // - Ready
                    ext = "Ready";
                    break;
        
                case 3: //  - Personalization code is required
                    ext = "Personalization code is required";
                    break;
        
                case 4: // - Blocked
                    ext = "PUK for personalization code is required";
                    break;
        
                case 5: // - Permanently blocked
                    ext = "Permanently blocked";
                    break;
        
                default:
                    ext = "???";
                    break;
                }
            snprintf(info_buf, sizeof(info_buf), "perso state %s", ext );
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
        
        
            // pin1 state
            switch (card_status_msg->card_status.card_info[i].app_info[0].pin1.pin_state)
                {
                case 0:  // - Unknown
                    ext = "Unknown";
                    break;
        
                case 1: // - Enabled and not verified
                    ext = "Enabled and not verified";
                    break;
        
                case 2: // - Enabled and verified
                    ext = "Enabled and verified";
                    break;
        
                case 3: //  - Disabled
                    ext = "Disabled";
                    break;
        
                case 4: // - Blocked
                    ext = "Blocked";
                    break;
        
                case 5: // - Permanently blocked
                    ext = "Permanently blocked";
                    break;
        
                default:
                    ext = "???";
                    break;
                }
            snprintf(info_buf, sizeof(info_buf), "pin1 state %s", ext );
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
            
            // pin 1 retries
            snprintf(info_buf, sizeof(info_buf), "pin1 retries %d", card_status_msg->card_status.card_info[i].app_info[0].pin1.pin_retries );
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
        
            // pin2 state
            switch (card_status_msg->card_status.card_info[i].app_info[0].pin2.pin_state)
                {
                case 0:  // - Unknown
                    ext = "Unknown";
                    break;
        
                case 1: // - Enabled and not verified
                    ext = "Enabled and not verified";
                    break;
        
                case 2: // - Enabled and verified
                    ext = "Enabled and verified";
                    break;
        
                case 3: //  - Disabled
                    ext = "Disabled";
                    break;
        
                case 4: // - Blocked
                    ext = "Blocked";
                    break;
        
                case 5: // - Permanently blocked
                    ext = "Permanently blocked";
                    break;
        
                default:
                    ext = "???";
                    break;
                }
            snprintf(info_buf, sizeof(info_buf), "pin2 state %s", ext );
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
        
            // pin 2 retries
            snprintf(info_buf, sizeof(info_buf), "pin2 retries %d", card_status_msg->card_status.card_info[i].app_info[0].pin2.pin_retries );
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, info_buf);
            }
        }
    }
    else
    {
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, "no card information available");
    }

    return 0;
    }
int qmi_simple_ril_misc_uim_get_fdn_status_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    uim_get_service_status_resp_msg_v01* uim_resp;
    if (qmi_msg_info->error_code == QMI_NO_ERR)
        {
        if(NULL != qmi_msg_info->message_specific_payload)
            {
            uim_resp = (uim_get_service_status_resp_msg_v01*) qmi_msg_info->message_specific_payload;
            if (uim_resp->resp.result == QMI_RESULT_SUCCESS_V01)
                {
                sprintf(temp_char_array, "[UIM_Test] Get Service Status returned success\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                if (TRUE == uim_resp->fdn_status_valid)
                    {
                        if (uim_resp->fdn_status == 0)
                            {
                            sprintf(temp_char_array, "[UIM_Test] FDN not available\n");
                            }
                        else if (uim_resp->fdn_status == 1)
                            {
                            sprintf(temp_char_array, "[UIM_Test] FDN available but disabled\n");
                            }
                        else if (uim_resp->fdn_status == 2)
                            {
                            sprintf(temp_char_array, "[UIM_Test] FDN available and enabled\n");
                            }
                        else
                            {
                            sprintf(temp_char_array, "[UIM_Test] FDN status unknown\n");
                            }
                        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }
                }
            else
                {
                sprintf(temp_char_array, "[UIM_Test] Get Service Status returned error code: %d\n", uim_resp->resp.error);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
        else
            {
            sprintf(temp_char_array, "[UIM_Test] Get Service status callback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "[UIM_TEST] ERROR in sending Get Service Status request, error code :%d\n",qmi_msg_info->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

int qmi_simple_ril_misc_uim_verify_pin_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
    {
    uim_verify_pin_resp_msg_v01* uim_resp;
    if (qmi_msg_info->error_code == QMI_NO_ERR)
        {
        if(NULL != qmi_msg_info->message_specific_payload)
            {
            uim_resp = (uim_verify_pin_resp_msg_v01*) qmi_msg_info->message_specific_payload;
            if (uim_resp->resp.result == QMI_RESULT_SUCCESS_V01)
                {
                sprintf(temp_char_array, "[UIM_Test] Verify PIN returned success\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                if (TRUE == uim_resp->retries_left_valid)
                    {
                    sprintf(temp_char_array, "[UIM_Test] Unblocking attempts left: %d\n", uim_resp->retries_left.unblock_left);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    sprintf(temp_char_array, "[UIM_Test] Verifying attempts left: %d\n", uim_resp->retries_left.verify_left);
                    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                    }
                }
            else
                {
                sprintf(temp_char_array, "[UIM_Test] Verify PIN returned error code: %d\n", uim_resp->resp.error);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
        else
            {
            sprintf(temp_char_array, "[UIM_Test] Verify PIN callback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "[UIM_TEST] ERROR in sending Verify PIN request, error code :%d\n",qmi_msg_info->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

int qmi_simple_ril_misc_uim_set_srv_status_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message)
{
    uim_set_service_status_resp_msg_v01* set_srv_status_resp;
    if (qmi_msg_info->error_code == QMI_NO_ERR)
        {
        if(NULL != qmi_msg_info->message_specific_payload)
            {
            set_srv_status_resp = (uim_set_service_status_resp_msg_v01*) qmi_msg_info->message_specific_payload;
            if (set_srv_status_resp->resp.result == QMI_RESULT_SUCCESS_V01)
                {
                sprintf(temp_char_array, "[UIM_Test] Set Service Status returned success\n");
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            else
                {
                sprintf(temp_char_array, "[UIM_Test] Set Service Status returned error code: %d\n", set_srv_status_resp->resp.error);
                qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
                }
            }
        else
            {
            sprintf(temp_char_array, "[UIM_Test] Set Service status callback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                                                
        }
    else
        {
        sprintf(temp_char_array, "[UIM_TEST] ERROR in sending Set Service Status request, error code :%d\n",qmi_msg_info->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

int qmi_simple_ril_indication_register_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;
    pbm_indication_register_req_msg_v01 pbm_indication_register_req_msg;
    

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&pbm_indication_register_req_msg,0,sizeof(pbm_indication_register_req_msg));
        pbm_indication_register_req_msg.reg_mask=3;

        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_PB;
        req_params.message_id = QMI_PBM_INDICATION_REGISTER_REQ_V01;
        req_params.message_specific_payload = &pbm_indication_register_req_msg;
        req_params.message_specific_payload_len = sizeof(pbm_indication_register_req_msg);

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_indication_register_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(NULL != callback_msg_info->message_specific_payload)
            {
            pb_print_indication_register_resp(((pbm_indication_register_resp_msg_v01*)(callback_msg_info->message_specific_payload)),uplink_message);
            }
        else
            {
            sprintf(temp_char_array, "\nCallback has no message payload\n");
            qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
            }                        
        }
    else
        {
        sprintf(temp_char_array, "[PB_TEST] ERROR in sending the Indication_Register Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

static void pb_print_indication_register_resp(pbm_indication_register_resp_msg_v01* pb_arg_resp_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0;
    if(QMI_RESULT_SUCCESS_V01 == pb_arg_resp_msg->resp.result)
    {
        sprintf(temp_char_array, "\n[PB_TEST] Registration Info : \n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        if(TRUE == pb_arg_resp_msg->reg_mask_valid)
        {
        sprintf(temp_char_array, "[PB_TEST] Registration MASK was set to : %d\n",pb_arg_resp_msg->reg_mask);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);        
        }
        else
        {
        sprintf(temp_char_array, "\n[PB_TEST] Registration MASK was not set : \n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    }
    else
    {
        sprintf(temp_char_array, "[PB_TEST] ERROR in Indication_Register Response, ERROR CODE:%d",pb_arg_resp_msg->resp.error);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    }
}

int qmi_simple_ril_pb_record_update_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if(NULL != unsolicited_msg_info->message_specific_payload)
        {
        pb_print_record_update_ind(((pbm_record_update_ind_msg_v01*) unsolicited_msg_info->message_specific_payload),uplink_message);
        }
    else
        {
        sprintf(temp_char_array, "\nIndication has no message payload\n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
        }                            
    return 0;
}

static void pb_print_record_update_ind(pbm_record_update_ind_msg_v01* pb_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0;
    sprintf(temp_char_array, "\n[PB_TEST] Record Update Info : \n");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[PB_TEST] Session Type : %d\n",pb_arg_ind_msg->record_update_info.session_type);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[PB_TEST] PB Type : %d\n",pb_arg_ind_msg->record_update_info.pb_type);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[PB_TEST] Operation : %d\n",pb_arg_ind_msg->record_update_info.operation);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[PB_TEST] Record ID : %d\n",pb_arg_ind_msg->record_update_info.record_id);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
}

int qmi_simple_ril_pb_ready_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if(NULL != unsolicited_msg_info->message_specific_payload)
        {
        pb_print_pb_ready_ind(((pbm_pb_ready_ind_msg_v01*) unsolicited_msg_info->message_specific_payload),uplink_message);
        }
    else
        {
        sprintf(temp_char_array, "\nIndication has no message payload\n");
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);            
        }                               
    return 0;
}

static void pb_print_pb_ready_ind(pbm_pb_ready_ind_msg_v01* pb_arg_ind_msg,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    unsigned int iter1=0;
    sprintf(temp_char_array, "\n[PB_TEST] Ready Info : \n");
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[PB_TEST] Session Type : %d\n",pb_arg_ind_msg->phonebook_ready_info.session_type);
    qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
    sprintf(temp_char_array, "[PB_TEST] PB Type : %d\n",pb_arg_ind_msg->phonebook_ready_info.pb_type);
}


int qmi_simple_ril_misc_common_qmi_svc_versions_command_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
    {
     static char *svc_names[] = 
     {
         "Voice",
         "NAS", 
         "WMS",
         "WDS",
         "DMS",
         "UIM",
         "PBM",
         "RF_SAR"
     };

    int svc;
    char ver_buf[128];

    qmi_service_version_info ver_info;
    qmi_simple_ril_cmd_completion_info* uplink = qmi_simple_ril_completion_info_allocate_ex(cmd_params);

    if (uplink)
        {
        for (svc = QMI_UTIL_SVC_FIRST; svc < QMI_UTIL_SVC_LAST; svc++)
            {
            qmi_util_get_service_version(svc, &ver_info);
            sprintf(ver_buf,"service %s version %d . %d", svc_names[svc], ver_info.major_ver, ver_info.minor_ver);
    
            qmi_simple_ril_util_add_entry_to_info_set(uplink->info_set, ver_buf);            
            }    
        qmi_simple_ril_complete_request(uplink);
        }
    else
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_NO_RESOURCES);
        }

    return 0;
    }

int qmi_simple_ril_misc_ping_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info)
{
    qmi_util_request_params req_params;

    if (cmd_params->info_set->nof_entries != 0)
        {
        qmi_simple_ril_complete_request_from_cmd_and_err(cmd_params, QMI_SIMPLE_RIL_ERR_ARG);
        }
    else
        {
        memset(&req_params, 0, sizeof(req_params));
        req_params.service_id = QMI_UTIL_SVC_DMS;
        req_params.message_id = QMI_DMS_GET_DEVICE_MFR_REQ_V01;
        req_params.message_specific_payload = NULL;
        req_params.message_specific_payload_len = 0;

        ack_info->request_id = qmi_util_post_request(&req_params);
        }
    return 0;
}

int qmi_simple_ril_ping_misc_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message)
{
    if (uplink_message->error_code == QMI_NO_ERR || uplink_message->error_code == QMI_IDL_LIB_MISSING_TLV)
        {
        if(callback_msg_info)
            {
            sprintf(temp_char_array, "\nTime taken for QMI transport = %lld micro seconds\n",
                    qmi_util_calculate_time_difference(&callback_msg_info->request_sent_time, &callback_msg_info->response_received_time));
            qmi_util_logln0(temp_char_array);
            }
        }
    else
        {
        sprintf(temp_char_array, "[PB_TEST] ERROR in sending the Indication_Register Request, ERROR CODE:%d\n",uplink_message->error_code);
        qmi_simple_ril_util_add_entry_to_info_set(uplink_message->info_set, temp_char_array);
        }
    return 0;
}

