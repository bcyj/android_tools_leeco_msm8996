/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_wms_core.h"
#include "cri_utils.h"
#include "cri_rule_handler.h"

int wms_client_id;

static qmi_error_type_v01 cri_wms_init_client_state();
static void cri_wms_cleanup_client_state();
static qmi_error_type_v01 cri_wms_core_perform_initial_configuration();
int cri_wms_core_encode_gw_sms(char *destination_number,
                               char *message_content,
                               uint8_t *encoded_bytes,
                               cri_wms_mo_pp_sms_type concatenated,
                               int seg_number,
                               int total_segments);

int cri_wms_core_encode_cdma_sms(char *destination_number,
                               char *message_content,
                               uint8_t *encoded_bytes);

static int cri_wms_core_encode_destination_number(int is_gw,
                                                  char *destination_number,
                                                  uint8_t *encoded_bytes);
static int cri_wms_core_decode_destination_number(int is_gw,
                                                  char *destination_number,
                                                  uint8_t *encoded_bytes);
static int cri_wms_core_encode_message_content(int is_gw,
                                               cri_wms_mo_pp_sms_type concatenated,
                                               char *message_content,
                                               uint8_t *encoded_bytes);
static int cri_wms_core_decode_message_content(int is_gw,
                                               char *message_content,
                                               uint8_t *encoded_bytes);
static int cri_wms_core_decode_cb_message(int is_gw,
                                          char *message_content,
                                          uint8_t *encoded_bytes);

static void cri_wms_core_event_report_ind_handler(int qmi_service_client_id,
                                                  wms_event_report_ind_msg_v01 *ind_msg);


static qmi_error_type_v01 cri_wms_core_receive_gw_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                                      cri_wms_mt_pp_sms_type *mt_pp_sms);

static qmi_error_type_v01 cri_wms_core_receive_cdma_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                                      cri_wms_mt_pp_sms_type *mt_pp_sms);

static qmi_error_type_v01 cri_wms_core_receive_gw_cb_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                                      cri_wms_mt_cb_sms_type *mt_cb_sms);

static qmi_error_type_v01 cri_wms_core_receive_cdma_cb_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                                      cri_wms_mt_cb_sms_type *mt_cb_sms);

static qmi_error_type_v01 cri_wms_gw_cb_config();

static qmi_error_type_v01 cri_wms_gw_cb_activation();

static qmi_error_type_v01 cri_wms_cdma_cb_config();

static qmi_error_type_v01 cri_wms_cdma_cb_activation();

int cri_wms_core_retrieve_client_id()
{
    return wms_client_id;
}

qmi_error_type_v01 cri_wms_core_init_client(hlos_ind_cb_type hlos_ind_cb)
{
    qmi_error_type_v01 client_init_error;

    client_init_error = QMI_ERR_INTERNAL_V01;

    wms_client_id = cri_core_create_qmi_service_client(QMI_WMS_SERVICE,
                                                        hlos_ind_cb);
    if(QMI_INTERNAL_ERR != wms_client_id)
    {
        client_init_error = cri_wms_init_client_state();
    }

    return client_init_error;
}

void cri_wms_core_release_client(int qmi_service_client_id)
{
    cri_wms_cleanup_client_state();
    cri_core_release_qmi_service_client(qmi_service_client_id);
    wms_client_id = NIL;
}

qmi_error_type_v01 cri_wms_init_client_state()
{
    cri_wms_core_perform_initial_configuration();
    cri_wms_gw_cb_config();
    cri_wms_gw_cb_activation();

    cri_wms_cdma_cb_config();
    cri_wms_cdma_cb_activation();
    return QMI_ERR_NONE_V01;
}

void cri_wms_cleanup_client_state()
{

}

qmi_error_type_v01 cri_wms_core_perform_initial_configuration()
{
    unsigned int iter_i;
    qmi_error_type_v01 err_code;
    wms_set_primary_client_req_msg_v01 set_primary_client_req_msg;
    wms_set_primary_client_resp_msg_v01 set_primary_client_resp_msg;
    wms_set_routes_req_msg_v01 set_routes_req_msg;
    wms_set_routes_resp_msg_v01 set_routes_resp_msg;
    wms_set_event_report_req_msg_v01 set_event_report_req_msg;
    wms_set_event_report_resp_msg_v01 set_event_report_resp_msg;

    iter_i = 0;
    err_code = QMI_ERR_INTERNAL_V01;
    memset(&set_primary_client_req_msg,
           NIL,
           sizeof(set_primary_client_req_msg));
    memset(&set_primary_client_resp_msg,
           NIL,
           sizeof(set_primary_client_resp_msg));
    memset(&set_routes_req_msg,
           NIL,
           sizeof(set_routes_req_msg));
    memset(&set_routes_resp_msg,
           NIL,
           sizeof(set_routes_resp_msg));
    memset(&set_event_report_req_msg,
           NIL,
           sizeof(set_event_report_req_msg));
    memset(&set_event_report_resp_msg,
           NIL,
           sizeof(set_event_report_resp_msg));

    do
    {
        set_primary_client_req_msg.primary_client = TRUE;
        err_code = cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                              QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01,
                                              &set_primary_client_req_msg,
                                              sizeof(set_primary_client_req_msg),
                                              &set_primary_client_resp_msg,
                                              sizeof(set_primary_client_resp_msg),
                                              CRI_CORE_MINIMAL_TIMEOUT);

        if(CRI_ERR_NONE_V01 != cri_core_retrieve_err_code(err_code,
                                                          &set_primary_client_resp_msg.resp))
        {
            err_code = CRI_ERR_INTERNAL_V01;
            break;
        }




        set_routes_req_msg.route_list_tuple_len = 6;
        for (iter_i = 0; iter_i < set_routes_req_msg.route_list_tuple_len; iter_i++)
        {
          set_routes_req_msg.route_list_tuple[iter_i].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
          set_routes_req_msg.route_list_tuple[iter_i].message_class = iter_i;
          set_routes_req_msg.route_list_tuple[iter_i].route_storage = WMS_STORAGE_TYPE_NONE_V01;
          set_routes_req_msg.route_list_tuple[iter_i].receipt_action = WMS_TRANSFER_ONLY_V01;
        }
        set_routes_req_msg.route_list_tuple[2].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
        set_routes_req_msg.route_list_tuple[2].message_class = WMS_MESSAGE_CLASS_2_V01;
        set_routes_req_msg.route_list_tuple[2].route_storage = WMS_STORAGE_TYPE_UIM_V01;
        set_routes_req_msg.route_list_tuple[2].receipt_action = WMS_STORE_AND_NOTIFY_V01;
        set_routes_req_msg.transfer_ind_valid = TRUE;
        set_routes_req_msg.transfer_ind = WMS_TRANSFER_IND_CLIENT_V01;
        err_code = cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                              QMI_WMS_SET_ROUTES_REQ_V01,
                                              &set_routes_req_msg,
                                              sizeof(set_routes_req_msg),
                                              &set_routes_resp_msg,
                                              sizeof(set_routes_resp_msg),
                                              CRI_CORE_MINIMAL_TIMEOUT);

        if(CRI_ERR_NONE_V01 != cri_core_retrieve_err_code(err_code,
                                                          &set_routes_resp_msg.resp))
        {
            err_code = CRI_ERR_INTERNAL_V01;
            break;
        }


        set_event_report_req_msg.report_mt_message_valid = TRUE;
        set_event_report_req_msg.report_mt_message = 0x01;
        err_code = cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                              QMI_WMS_SET_EVENT_REPORT_REQ_V01,
                                              &set_event_report_req_msg,
                                              sizeof(set_event_report_req_msg),
                                              &set_event_report_resp_msg,
                                              sizeof(set_event_report_resp_msg),
                                              CRI_CORE_MINIMAL_TIMEOUT);

        if(CRI_ERR_NONE_V01 != cri_core_retrieve_err_code(err_code,
                                                          &set_event_report_resp_msg.resp))
        {
            err_code = CRI_ERR_INTERNAL_V01;
            break;
        }

    }while(0);

    return err_code;
}
qmi_error_type_v01 cri_wms_gw_cb_config()
{
    qmi_error_type_v01 err_code = QMI_ERR_INTERNAL_V01;

    wms_set_broadcast_config_req_msg_v01   request_msg;
    wms_set_broadcast_config_resp_msg_v01  response_msg;

    UTIL_LOG_MSG("cri_wms_cb_config enter %d\n", err_code);

    memset(&request_msg, 0, sizeof(wms_set_broadcast_config_req_msg_v01));
    memset(&response_msg, 0, sizeof(wms_set_broadcast_config_resp_msg_v01));

    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;
    request_msg.wms_3gpp_broadcast_config_info_valid = TRUE;
    request_msg.wms_3gpp_broadcast_config_info_len = 3;
    /* 0-999 */
    request_msg.wms_3gpp_broadcast_config_info[0].from_service_id = 0;
    request_msg.wms_3gpp_broadcast_config_info[0].to_service_id   = 999;
    request_msg.wms_3gpp_broadcast_config_info[0].selected        = 1;

    /* 4352-4359 ETWS */
    request_msg.wms_3gpp_broadcast_config_info[1].from_service_id = 4352;
    request_msg.wms_3gpp_broadcast_config_info[1].to_service_id   = 4359;
    request_msg.wms_3gpp_broadcast_config_info[1].selected        = 1;

    /* 4370-4399 CMAS */
    request_msg.wms_3gpp_broadcast_config_info[2].from_service_id = 4370;
    request_msg.wms_3gpp_broadcast_config_info[2].to_service_id   = 4399;
    request_msg.wms_3gpp_broadcast_config_info[2].selected        = 1;
    err_code = cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                          QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                                          &request_msg,
                                          sizeof(wms_set_broadcast_config_req_msg_v01),
                                          &response_msg,
                                          sizeof(wms_set_broadcast_config_resp_msg_v01),
                                          CRI_CORE_MINIMAL_TIMEOUT);

    UTIL_LOG_MSG("cri_wms_cb_config exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_cdma_cb_config()
{
    qmi_error_type_v01 err_code = QMI_ERR_INTERNAL_V01;

    wms_set_broadcast_config_req_msg_v01   request_msg;
    wms_set_broadcast_config_resp_msg_v01  response_msg;
    int i;

    UTIL_LOG_MSG("cri_wms_cb_config enter %d\n", err_code);

    memset(&request_msg, 0, sizeof(wms_set_broadcast_config_req_msg_v01));
    memset(&response_msg, 0, sizeof(wms_set_broadcast_config_resp_msg_v01));

    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    request_msg.wms_3gpp2_broadcast_config_info_valid = TRUE;
    request_msg.wms_3gpp2_broadcast_config_info_len   = 36;

    for (i = 0; i < 32; i ++)
    {
        request_msg.wms_3gpp2_broadcast_config_info[i].service_category = i;
        request_msg.wms_3gpp2_broadcast_config_info[i].language         = WMS_LANGUAGE_ENGLISH_V01;
        request_msg.wms_3gpp2_broadcast_config_info[i].selected         = 1;
    }

    for (i = 0; i < 5; i ++ )
    {
        request_msg.wms_3gpp2_broadcast_config_info[i].service_category = 0x1000 + i;
        request_msg.wms_3gpp2_broadcast_config_info[i].language         = WMS_LANGUAGE_ENGLISH_V01;
        request_msg.wms_3gpp2_broadcast_config_info[i].selected         = 1;
    }

    err_code = cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                          QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                                          &request_msg,
                                          sizeof(wms_set_broadcast_config_req_msg_v01),
                                          &response_msg,
                                          sizeof(wms_set_broadcast_config_resp_msg_v01),
                                          CRI_CORE_MINIMAL_TIMEOUT);

    UTIL_LOG_MSG("cri_wms_cb_config exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_gw_cb_activation()
{

    qmi_error_type_v01 err_code = QMI_ERR_INTERNAL_V01;

    wms_set_broadcast_activation_req_msg_v01     request_msg;
    wms_set_broadcast_activation_resp_msg_v01    response_msg;

    memset(&request_msg, 0, sizeof(wms_set_broadcast_activation_req_msg_v01));
    memset(&response_msg, 0, sizeof(wms_set_broadcast_activation_resp_msg_v01));

    UTIL_LOG_MSG("cri_wms_core_cb_activation entry\n");
    request_msg.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_GW_V01;
    request_msg.broadcast_activation_info.bc_activate  = 1;
    request_msg.activate_all_valid = TRUE;
    request_msg.activate_all       = 1;

    err_code =  cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                           QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                                           &request_msg,
                                           sizeof(wms_set_broadcast_activation_req_msg_v01),
                                           &response_msg,
                                           sizeof(wms_set_broadcast_activation_resp_msg_v01),
                                           CRI_CORE_MINIMAL_TIMEOUT);
    UTIL_LOG_MSG("cri_wms_core_cb_activation exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_cdma_cb_activation()
{

    qmi_error_type_v01 err_code = QMI_ERR_INTERNAL_V01;

    wms_set_broadcast_activation_req_msg_v01     request_msg;
    wms_set_broadcast_activation_resp_msg_v01    response_msg;

    memset(&request_msg, 0, sizeof(wms_set_broadcast_activation_req_msg_v01));
    memset(&response_msg, 0, sizeof(wms_set_broadcast_activation_resp_msg_v01));

    UTIL_LOG_MSG("cri_wms_cdma_cb_activation entry\n");
    request_msg.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    request_msg.broadcast_activation_info.bc_activate  = 1;
    request_msg.activate_all_valid = TRUE;
    request_msg.activate_all       = 1;

    err_code =  cri_core_qmi_send_msg_sync(cri_wms_core_retrieve_client_id(),
                                           QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                                           &request_msg,
                                           sizeof(wms_set_broadcast_activation_req_msg_v01),
                                           &response_msg,
                                           sizeof(wms_set_broadcast_activation_resp_msg_v01),
                                           CRI_CORE_MINIMAL_TIMEOUT);
    UTIL_LOG_MSG("cri_wms_cdma_cb_activation exit %d\n", err_code);

    return err_code;
}

void cri_wms_send_mo_msg_response_handler(int qmi_service_client_id,
                                     wms_raw_send_resp_msg_v01 *resp_msg,
                                    cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;

    cri_core_error = QMI_ERR_NONE_V01;

    if(resp_msg)
    {
    cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR,
                                                &resp_msg->resp);
    cri_rule_handler_rule_check(cri_core_context,
                                cri_core_error,
                                resp_msg);
    }

}



void cri_wms_core_async_resp_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *resp_data,
                                     int resp_data_len,
                                     cri_core_context_type cri_core_context)
{

    if(resp_data && resp_data_len)
    {
        switch(message_id)
        {
        case QMI_WMS_RAW_SEND_RESP_V01:
            cri_wms_send_mo_msg_response_handler(qmi_service_client_id,
                                                resp_data,
                                                cri_core_context);
        break;

        default:
        //no action
        break;

        }
    }

}

void cri_wms_core_unsol_ind_handler(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len)
{
    UTIL_LOG_MSG("entry core ind hdlr sms %d", message_id);

    switch(message_id)
    {
    case QMI_WMS_EVENT_REPORT_IND_V01:
        cri_wms_core_event_report_ind_handler(qmi_service_client_id,
                                              (wms_event_report_ind_msg_v01*) ind_data);
        break;

        default:
            //no action
            break;
    }

    UTIL_LOG_MSG("exit");
}

void cri_wms_core_event_report_ind_handler(int qmi_service_client_id,
                                           wms_event_report_ind_msg_v01 *ind_msg)
{
    hlos_ind_cb_type hlos_ind_cb;
    cri_wms_mt_pp_sms_type *mt_pp_sms;
    cri_wms_mt_cb_sms_type *mt_cb_sms;
    qmi_error_type_v01 err_code;

    mt_pp_sms = NULL;
    mt_cb_sms = NULL;
    hlos_ind_cb = NULL;
    err_code = QMI_ERR_INTERNAL_V01;

    int data_len;

    if(ind_msg)
    {
        if(TRUE == ind_msg->transfer_route_mt_message_valid)
        {
		switch(ind_msg->transfer_route_mt_message.format)
                {

		    case WMS_MESSAGE_FORMAT_GW_PP_V01:
                            mt_pp_sms = util_memory_alloc(sizeof(*mt_pp_sms));
                            err_code = cri_wms_core_receive_gw_sms(ind_msg,
                                                                 mt_pp_sms);
			break;
		    case WMS_MESSAGE_FORMAT_CDMA_V01:
                            if(ind_msg)
                            {
                                if(ind_msg->transfer_route_mt_message.data[0] == 0x00)
                                {
                                    //point to point MT sms
                                    mt_pp_sms = util_memory_alloc(sizeof(*mt_pp_sms));
			err_code = cri_wms_core_receive_cdma_sms(ind_msg,
								  mt_pp_sms);
                                }
                                else if(ind_msg->transfer_route_mt_message.data[0] == 0x01)
                                {
                                     mt_cb_sms = util_memory_alloc(sizeof(*mt_cb_sms));
                                     err_code = cri_wms_core_receive_cdma_cb_sms(ind_msg,
                                                                                 mt_cb_sms);
                                }
                            }

                            break;
                        case WMS_MESSAGE_FORMAT_GW_BC_V01:
                             mt_cb_sms = util_memory_alloc(sizeof(*mt_cb_sms));
                             err_code = cri_wms_core_receive_gw_cb_sms(ind_msg,
                                                                        mt_cb_sms);
			break;
		    default:
			UTIL_LOG_MSG("Invalid message format - incoming message");
			break;
                }

                if(QMI_ERR_NONE_V01 == err_code)
                {
                    hlos_ind_cb = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
                    if(hlos_ind_cb)
                    {

                            if(mt_pp_sms)
                            {
                                UTIL_LOG_MSG("Calling hlos in PP");
                        ((*hlos_ind_cb) (CRI_WMS_MT_PP_SMS_IND,
                                         mt_pp_sms,
                                         sizeof(*mt_pp_sms)));

                            }
                            else if(mt_cb_sms)
                            {
                                UTIL_LOG_MSG("Calling hlos in CB");
                                ((*hlos_ind_cb) (CRI_WMS_MT_CB_SMS_IND,
                                                 mt_cb_sms,
                                                 sizeof(*mt_cb_sms)));

                            }
                            else
                                UTIL_LOG_MSG("Message format not recognized");
                    }
                    else
                    {
                        UTIL_LOG_MSG("No registered HLOS ind handler for incoming message");
                    }
                }
                util_memory_free((void**) &mt_pp_sms);
                  util_memory_free((void**) &mt_cb_sms);

        }

        else if(TRUE == ind_msg->message_mode_valid &&
                TRUE == ind_msg->mt_message_valid)
        {
            if (ind_msg->mt_message.storage_type == WMS_STORAGE_TYPE_UIM_V01)
            {
               if (ind_msg->message_mode == WMS_MESSAGE_MODE_GW_V01)
               {
                  UTIL_LOG_MSG("\n UIM_GW index:%d\n", ind_msg->mt_message.storage_index);
               }
               else if (ind_msg->message_mode == WMS_MESSAGE_MODE_CDMA_V01)
               {
                  UTIL_LOG_MSG("\n UIM_CDMA index:%d\n", ind_msg->mt_message.storage_index);
               }
               else
               {
                  UTIL_LOG_MSG("\n invalide message_mode:%d\n", ind_msg->message_mode);
               }
            }
            else if ((ind_msg->mt_message.storage_type == WMS_STORAGE_TYPE_NV_V01) &&
                     (ind_msg->message_mode == WMS_MESSAGE_MODE_CDMA_V01))
            {
               UTIL_LOG_MSG("\n NV_CDMA index:%d\n", ind_msg->mt_message.storage_index);
            }
        }
        else if (TRUE == ind_msg->etws_message_valid)
        {
           UTIL_LOG_MSG("\n ETWS data_len:%d\n", ind_msg->etws_message.data_len);
           UTIL_LOG_MSG("\n ETWS:%s\n", ind_msg->etws_message.data);
        }
    }

}


qmi_error_type_v01 cri_wms_core_send_gw_sms(cri_core_context_type cri_core_context,
                                            char *destination_number,
                                            char *message_content,
                                            void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb,
                                            cri_wms_mo_pp_sms_type concatenated,
                                            int seg_number,
                                            int total_segments)
{
    qmi_error_type_v01 err_code;
    wms_raw_send_req_msg_v01 wms_raw_send_req_msg;

    err_code = QMI_ERR_INTERNAL_V01;
    memset(&wms_raw_send_req_msg,
           NIL,
           sizeof(wms_raw_send_req_msg));

    UTIL_LOG_MSG("cri_wms_core_send_gw_sms entry\n");
    if(destination_number && message_content)
    {
        UTIL_LOG_MSG("Trying to send %s to %s\n",message_content, destination_number);

        wms_raw_send_req_msg.raw_message_data.format = WMS_MESSAGE_FORMAT_GW_PP_V01;
        wms_raw_send_req_msg.raw_message_data.raw_message_len =
                    cri_wms_core_encode_gw_sms(destination_number,
                                               message_content,
                                               wms_raw_send_req_msg.raw_message_data.raw_message,
                                               concatenated,
                                               seg_number,
                                               total_segments);

        err_code =  cri_core_qmi_send_msg_async(cri_core_context,
                                               wms_client_id,
                                               QMI_WMS_RAW_SEND_REQ_V01,
                                               &wms_raw_send_req_msg,
                                               sizeof(wms_raw_send_req_msg),
                                               sizeof(wms_raw_send_resp_msg_v01),
                                               hlos_cb_data,
                                               hlos_resp_cb,
                                               CRI_CORE_MAX_TIMEOUT,
                                               NULL);
    }
    UTIL_LOG_MSG("cri_wms_core_send_gw_sms exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_core_send_cdma_sms(cri_core_context_type cri_core_context,
                                            char *destination_number,
                                            char *message_content,
                                            void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb)
{
    qmi_error_type_v01 err_code;
    wms_raw_send_req_msg_v01 wms_raw_send_req_msg;

    err_code = QMI_ERR_INTERNAL_V01;
    memset(&wms_raw_send_req_msg,
           NIL,
           sizeof(wms_raw_send_req_msg));

    UTIL_LOG_MSG("cri_wms_core_send_cdma_sms entry\n");
    if(destination_number && message_content)
    {
        UTIL_LOG_MSG("Trying to send %s to %s\n",message_content, destination_number);

        wms_raw_send_req_msg.raw_message_data.format = WMS_MESSAGE_FORMAT_CDMA_V01;
        wms_raw_send_req_msg.raw_message_data.raw_message_len =
                    cri_wms_core_encode_cdma_sms(destination_number,
                                               message_content,
                                               wms_raw_send_req_msg.raw_message_data.raw_message);
        wms_raw_send_req_msg.sms_on_ims_valid = TRUE;
        wms_raw_send_req_msg.sms_on_ims = 0x00;

        err_code =  cri_core_qmi_send_msg_async(cri_core_context,
                                               wms_client_id,
                                               QMI_WMS_RAW_SEND_REQ_V01,
                                               &wms_raw_send_req_msg,
                                               sizeof(wms_raw_send_req_msg),
                                               sizeof(wms_raw_send_resp_msg_v01),
                                               hlos_cb_data,
                                               hlos_resp_cb,
                                               CRI_CORE_MAX_TIMEOUT,
                                               NULL);
    }
    UTIL_LOG_MSG("cri_wms_core_send_cdma_sms exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_core_receive_gw_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                               cri_wms_mt_pp_sms_type *mt_pp_sms)
{
    uint32_t iter_i;
    qmi_error_type_v01 err_code;

    iter_i = 1;
    err_code = QMI_ERR_INTERNAL_V01;

    UTIL_LOG_MSG("cri_wms_core_receive_gw_sms entry\n");
    if(ind_msg && mt_pp_sms)
    {
        memset(mt_pp_sms,
               NIL,
               sizeof(*mt_pp_sms));
        mt_pp_sms->format = WMS_MESSAGE_FORMAT_GW_PP_V01;
        mt_pp_sms->transaction_id = ind_msg->transfer_route_mt_message.transaction_id;
        iter_i += cri_wms_core_decode_destination_number(TRUE,
                                                         mt_pp_sms->source_number,
                                                         &ind_msg->transfer_route_mt_message.data[iter_i]);

    //getting bearer data length
	uint8_t bearer_data_len = ind_msg->transfer_route_mt_message.data[(iter_i+4)];
	int bdl_index = iter_i+4;
    //skip the next 9 bytes for the msg content
        iter_i+=9;

        if(iter_i < ind_msg->transfer_route_mt_message.data_len)
        {
            iter_i += cri_wms_core_decode_message_content(TRUE,
                                                          mt_pp_sms->message_content,
                                                          &ind_msg->transfer_route_mt_message.data[iter_i]);
        }
	iter_i= iter_i + (bearer_data_len - (iter_i - bdl_index));
        err_code = QMI_ERR_NONE_V01;
    }
    UTIL_LOG_MSG("cri_wms_core_receive_gw_sms exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_core_receive_cdma_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                               cri_wms_mt_pp_sms_type *mt_pp_sms)
{
    uint32_t iter_i;
    qmi_error_type_v01 err_code;

    iter_i = 6;
    err_code = QMI_ERR_INTERNAL_V01;

    UTIL_LOG_MSG("cri_wms_core_receive_cdma_pp_sms entry\n");
    if(ind_msg && mt_pp_sms)
    {
        memset(mt_pp_sms,
               NIL,
               sizeof(*mt_pp_sms));
        mt_pp_sms->format = WMS_MESSAGE_FORMAT_CDMA_V01;
        mt_pp_sms->transaction_id = ind_msg->transfer_route_mt_message.transaction_id;
        iter_i += cri_wms_core_decode_destination_number(FALSE,
                                                         mt_pp_sms->source_number,
                                                         &ind_msg->transfer_route_mt_message.data[iter_i]);

        iter_i+= 12;	//skip extra bits between number and message

        if(iter_i < ind_msg->transfer_route_mt_message.data_len &&
           ind_msg->transfer_route_mt_message.data_len < WMS_MESSAGE_LENGTH_MAX_V01 )
        {
            iter_i += cri_wms_core_decode_message_content(FALSE,
                                                          mt_pp_sms->message_content,
                                                          &ind_msg->transfer_route_mt_message.data[iter_i]);
        }

        err_code = QMI_ERR_NONE_V01;
    }
    UTIL_LOG_MSG("cri_wms_core_receive_cdma_pp_sms exit %d\n", err_code);

    return err_code;
}

qmi_error_type_v01 cri_wms_core_receive_gw_cb_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                                  cri_wms_mt_cb_sms_type *mt_cb_sms)
{
    int iter_i=0;
    qmi_error_type_v01 err_code;


    err_code = QMI_ERR_INTERNAL_V01;

    UTIL_LOG_MSG("cri_wms_core_receive_gw_cb_sms entry\n");
    if(ind_msg && mt_cb_sms)
    {
        memset(mt_cb_sms,
               NIL,
               sizeof(*mt_cb_sms));
        mt_cb_sms->format = WMS_MESSAGE_FORMAT_GW_BC_V01;

        iter_i += cri_wms_core_decode_cb_message(TRUE,
                                                 mt_cb_sms->message_content,
                                                 &ind_msg->transfer_route_mt_message.data[iter_i]);
        err_code = QMI_ERR_NONE_V01;
    }
    UTIL_LOG_MSG("cri_wms_core_receive_gw_cb_sms exit %d\n", err_code);

    return err_code;
}


qmi_error_type_v01 cri_wms_core_receive_cdma_cb_sms(wms_event_report_ind_msg_v01 *ind_msg,
                                                  cri_wms_mt_cb_sms_type *mt_cb_sms)
{
    int iter_i;
    qmi_error_type_v01 err_code;

    iter_i = 14;
    err_code = QMI_ERR_INTERNAL_V01;

    UTIL_LOG_MSG("cri_wms_core_receive_cdma_cb_sms entry\n");
    if(ind_msg && mt_cb_sms)
    {
        memset(mt_cb_sms,
               NIL,
               sizeof(*mt_cb_sms));
        mt_cb_sms->format = WMS_MESSAGE_FORMAT_CDMA_V01;

        iter_i += cri_wms_core_decode_cb_message(FALSE,
                                                 mt_cb_sms->message_content,
                                                 &ind_msg->transfer_route_mt_message.data[iter_i]);
        err_code = QMI_ERR_NONE_V01;
    }
    UTIL_LOG_MSG("cri_wms_core_receive_cdma_cb_sms exit %d\n", err_code);

    return err_code;
}



int cri_wms_core_encode_gw_sms(char *destination_number,
                               char *message_content,
                               uint8_t *encoded_bytes,
                               cri_wms_mo_pp_sms_type concatenated,
                               int seg_number,
                               int total_segments)
{
    int iter_i;

    iter_i = 0;

    if(destination_number &&
       message_content &&
       encoded_bytes)
    {
        encoded_bytes[iter_i++] = 0x00;
        if(concatenated == CRI_WMS_MO_PP_SMS_TYPE_CONCATENATED)
            encoded_bytes[iter_i++] = 0x41; // 0"1"00 need to be set to send concatenated SMS
        else
            encoded_bytes[iter_i++] = 0x01; // 0"0"00 is not set for normal text SMS
        encoded_bytes[iter_i++] = 0x00;

        if('+' == destination_number[0])
        {
            destination_number++;
            encoded_bytes[iter_i++] = strlen(destination_number);
            encoded_bytes[iter_i++] = 0x91; // international format
        }
        else
        {
            encoded_bytes[iter_i++] = strlen(destination_number);
            encoded_bytes[iter_i++] = 0x81;
        }

        iter_i += cri_wms_core_encode_destination_number(TRUE,
                                                         destination_number,
                                                         &encoded_bytes[iter_i]);

        encoded_bytes[iter_i++] = 0x00; //protocol identifier
        encoded_bytes[iter_i++] = 0x00; //data coding scheme
        if(concatenated == CRI_WMS_MO_PP_SMS_TYPE_CONCATENATED)
        {
                encoded_bytes[iter_i++] = strlen(message_content)+7; //6 bytes of header added to the length

                //PDU User data Header
                encoded_bytes[iter_i++] = 0x05;
        encoded_bytes[iter_i++] = 0x00;
                encoded_bytes[iter_i++] = 0x03;
                encoded_bytes[iter_i++] = 0xAC;
                encoded_bytes[iter_i++] = total_segments; //total number of segments
                encoded_bytes[iter_i++] = seg_number; //segment number of the PDU

        }
        else
            encoded_bytes[iter_i++] = strlen(message_content); //normal text msg length

        iter_i += cri_wms_core_encode_message_content(TRUE,
                                                      concatenated,
                                                      message_content,
                                                      &encoded_bytes[iter_i]);
    }

    return iter_i;
}



int cri_wms_core_encode_cdma_sms(char *destination_number,
                               char *message_content,
                               uint8_t *encoded_bytes)
{
    int iter_i;

    iter_i = 0;

    if(destination_number && message_content && encoded_bytes)
    {

        //fixed bytes common to all msgs
        encoded_bytes[iter_i++] =  0x00;
        encoded_bytes[iter_i++] =  0x00;
        encoded_bytes[iter_i++] =  0x02;
        encoded_bytes[iter_i++] =  0x10;
        encoded_bytes[iter_i++] =  0x02;
        encoded_bytes[iter_i++] =  0x04;

	//skip the +1 and +91 country codes iff given in input
        if('+' == destination_number[0])
        {
            destination_number++;
		if(destination_number[0] == '9') destination_number++;
            destination_number++;
        }

        iter_i += cri_wms_core_encode_destination_number(FALSE,
                                                         destination_number,
                                                         &encoded_bytes[iter_i]);

	//fixed bytes
        encoded_bytes[iter_i++] = 0x06;
        encoded_bytes[iter_i++] = 0x01;
        encoded_bytes[iter_i++] = 0xFC;
        encoded_bytes[iter_i++] = 0x08;

        int bdl_index = iter_i;  //store the index of beader data length

        encoded_bytes[iter_i++] = 0x00; //bearer data length given dummy value
        encoded_bytes[iter_i++] = 0x00;
        encoded_bytes[iter_i++] = 0x03;
        encoded_bytes[iter_i++] = 0x20;
        encoded_bytes[iter_i++] = 0x00;

        encoded_bytes[iter_i++] = 0x20;
        encoded_bytes[iter_i++] = 0x01;


        iter_i += cri_wms_core_encode_message_content(FALSE,
                                                      0, //not concatenated
                                                      message_content,
                                                      &encoded_bytes[iter_i]);

        encoded_bytes[bdl_index] = iter_i - (bdl_index+1); //assign the bearer data length
							   //from the bdl_index till the encoded index
    }

    return iter_i;
}

//to get dtmf value from the number
int cri_wms_retrieve_dtmf_value(char c)
{
   int value;

   //for 0 the dtmf value is 10 (1010 in binary)
   if (c == '0') {
      value = 10;
   }
   //for other digits it is the digit itself (c - 48 gives the actual digit)
   else
   {
      value = (c - 48);
   }

   return value;
}


int cri_wms_core_encode_destination_number(int is_gw,
                                           char *destination_number,
                                           uint8_t *encoded_bytes)
{
    int encoded_stream_len;
    int temp;

    encoded_stream_len = 0;
    temp = 0;

    if(is_gw)
    {
        UTIL_LOG_MSG("Send SMS destination number encode GW\n");
        char tempC;
        //every 2 digits in the number in decimal is swapped to form the encoded byte
        //eg: 8586582450 forms 58 68  85  42 05 in encoded bytes
        while(*destination_number)
        {
            if(!temp)
            {
                //get the first digit
                encoded_bytes[encoded_stream_len] = (*destination_number - 48);
                temp=1;
            }
            else
            {
                //"or" the first digit with the next digit, 4 bits pushed to left
                encoded_bytes[encoded_stream_len] = ((*destination_number - 48) << 4) | encoded_bytes[encoded_stream_len];
                encoded_stream_len++;
                temp=0;
            }
            destination_number++;
        }

        if(temp)
        {
            //add an F if a digit is left out i.e if the length of the phone number is odd
            encoded_bytes[encoded_stream_len] = 0xF0 | encoded_bytes[encoded_stream_len];
            encoded_stream_len++;
        }
    }
    else
    {

        UTIL_LOG_MSG("Send SMS destination number encode CDMA\n");
         int len_encoded_bytes = 0;
         encoded_bytes[encoded_stream_len++] = 0x00; //assign dummy for the length of encoded bytes

        //first 6-bits of length form the last 6 bits of the byte followed after length of encoded bytes
	encoded_bytes[encoded_stream_len] =(strlen(destination_number) >> 2);

        encoded_stream_len++;
        len_encoded_bytes++;
	//the remaining last 2 bits of the length are first 2 bits of next byte
        encoded_bytes[encoded_stream_len] =(strlen(destination_number) << 6);
	//followed by a 4-bit dtmf representation of each number in the destination number
	//until it forms a byte
        while(*destination_number)
        {
	    //digit appended to 3rd, 4th, 5th, 6th bit positions
            encoded_bytes[encoded_stream_len] = encoded_bytes[encoded_stream_len] | (cri_wms_retrieve_dtmf_value(*destination_number) << 2);
            destination_number++;


            if(*destination_number)
            {
		//1st 2 bits of digit in last two bits of the byte
                encoded_bytes[encoded_stream_len] = encoded_bytes[encoded_stream_len] | (cri_wms_retrieve_dtmf_value(*destination_number) >> 2);
                encoded_stream_len++;
                len_encoded_bytes++;
		//last two bits of the same digit to first 2 bits of a new byte
                encoded_bytes[encoded_stream_len] =  (cri_wms_retrieve_dtmf_value(*destination_number) << 6);
                destination_number++;
	   }



         }
         encoded_stream_len++;
         len_encoded_bytes++;
         encoded_bytes[0] = len_encoded_bytes; //assign the length
    }

    return encoded_stream_len;
}

int cri_wms_value_from_dtmf(int dtmf)
{
   int c;
   if(dtmf == 10)
	c = '0';
   else
        c = dtmf+48;

   return c;
}
int cri_wms_core_decode_destination_number(int is_gw,
                                           char *destination_number,
                                           uint8_t *encoded_bytes)
{
    int encoded_bytes_len;
    int iter_i;
    int iter_j;
    int temp;

    encoded_bytes_len = 0;
    iter_i = 0;
    iter_j = 0;
    temp = 0;

    if(is_gw)
    {
	UTIL_LOG_MSG("Incoming SMS destination number decode GW\n");
    //The length of the encoded bytes of phone number
        encoded_bytes_len = (((encoded_bytes[iter_i++] - 1)/2) + 1) + 2;
        if(0x91 == encoded_bytes[iter_i++])
        {
            destination_number[iter_j++] = '+';
        }
        //read the phone number in swapped form in to normal form
        // the bytes 58 68  85  42 05 would form 85-86-58-24-50 number
        while(iter_i < encoded_bytes_len)
        {
            destination_number[iter_j++] = (encoded_bytes[iter_i] & 0x0F) + 48;
            temp = ((encoded_bytes[iter_i++] >> 4) & 0x0F);
            if(0x0F != temp)
            {
                destination_number[iter_j++] =  temp + 48;
            }
        }
        destination_number[iter_j] = '\0';
    }
    else
    {
        UTIL_LOG_MSG("Incoming SMS destination number decode CDMA\n");
	//1st byte is encoded bytes length
	uint8_t eb_len = encoded_bytes[iter_i++];
	encoded_bytes_len = eb_len;

	//last 6 bits of next byte and first two bits of followed byte
	//form number of digits in phone number
	uint8_t phone_num_len = 0x00 | (encoded_bytes[iter_i++] << 2);
        phone_num_len = phone_num_len | (encoded_bytes[iter_i] >> 6);

	//From then follow taking 4 bits continuously to get the digit
	//until the phone number length is reached/the encoded bytes end
	while(iter_i<=encoded_bytes_len && iter_j<phone_num_len)
	{
	   uint8_t temp = 0x00 | (encoded_bytes[iter_i] << 2);
           temp = temp >> 4;
	   destination_number[iter_j++] = cri_wms_value_from_dtmf(temp);
	   temp  = 0x00 | (encoded_bytes[iter_i++] << 6);
	   temp = temp >> 4;
	   temp = temp | (encoded_bytes[iter_i] >> 6);
	   destination_number[iter_j++] = cri_wms_value_from_dtmf(temp);
	}
        destination_number[iter_j] = '\0';
    }

    return (encoded_bytes_len);
}

int cri_wms_core_encode_message_content(int is_gw,
                                        cri_wms_mo_pp_sms_type concatenated,
                                        char *message_content,
                                        uint8_t *encoded_bytes)
{
    unsigned char str_index;
    int encoded_stream_len;
    unsigned char shift;
    int message_content_len;

    str_index = 0;
    shift = 0;
    encoded_stream_len = 0;
    message_content_len = 0;
    if(encoded_bytes != NULL && message_content != NULL)
    {
        if(is_gw)
        {


        message_content_len = strlen(message_content);
            if(concatenated == CRI_WMS_MO_PP_SMS_TYPE_CONCATENATED)message_content_len++;
            while(str_index < (message_content_len))
        {
                //if the index is more than 7 this will start reading from 0
                //like num%7
            shift = str_index  & 0x07;
                //bring the last "shift" number of digits from the 7 bit ASCII value of next character to
                //the begining of the current characters 7 bit ASCII value.(the current one doesnot have the bits
                //that are shifted to its previous one).
                if(concatenated == CRI_WMS_MO_PP_SMS_TYPE_CONCATENATED)
                encoded_bytes[encoded_stream_len++] = ((message_content[str_index] << 1) >> shift) |
                                                    ((message_content[str_index+1] << 1) << (7-shift));
                else
                    encoded_bytes[encoded_stream_len++] = ((message_content[str_index]) >> shift) |
                                                    ((message_content[str_index+1]) << (7-shift));
            if(shift==6)
            {
                str_index++;
            }
            str_index++;
        }

        if(str_index < message_content_len)
        {
                //if one more character left out just add the bits left in the 7 bit ascii
            shift = str_index & 0x07;
                if(concatenated == CRI_WMS_MO_PP_SMS_TYPE_CONCATENATED)
                     encoded_bytes[encoded_stream_len++] = ((message_content[str_index] << 1) >> shift);
                else
            encoded_bytes[encoded_stream_len++] = (message_content[str_index] >> shift);
        }

        if((message_content_len & 0x07) == 0 &&
           message_content[message_content_len - 1] == 0x0D)
        {
            encoded_bytes[encoded_stream_len++] = 0x0D;
        }
    }
    else
    {
	//7-bit ASCII scheme
          UTIL_LOG_MSG("Send SMS message encode CDMA");
        int len_encoded_bytes = 0;

	//Only 7 bits of the ASCII characters are taken into consideration
	//The 7 bits are formed in to a byte by taking the extra needed bits from
	//the next 7bit ASCII character and 0's are appended to the last byte if needed
	//calculating %8 gives the number of zeros that need to be appended
        int extra_zero_bits = strlen(message_content)%8;
        encoded_bytes[encoded_stream_len++] = 0x00; //assign dummy for the length of encoded bytes
        encoded_bytes[encoded_stream_len] = 0x10; //00010 is the encoding scheme

	//the rest bits are appended the first 3 bits of the length
        encoded_bytes[encoded_stream_len] = encoded_bytes[encoded_stream_len] | (strlen(message_content) >> 5);

        encoded_stream_len++;
        len_encoded_bytes++;

	//the rest 5 bits of the length form the first 5 bits of the next byte
        encoded_bytes[encoded_stream_len] = 0x00 | (strlen(message_content) << 3);
        int k = 3;
	//check value to check if there are extra 0 bits to be appended
        int check_zero_bits = 0;

	//The 7-bit ASCII characters are then appended continuously from the above point
	//into bytes till all characters are appended. And extra zeros are added to the last
	//byte if needed
        while(*message_content)
        {


           encoded_bytes[encoded_stream_len] = encoded_bytes[encoded_stream_len] | ((*message_content << 1) >> (8-k));
            encoded_stream_len++;
            len_encoded_bytes++;
            if(k==7)
	    {
		*message_content++;
		if(*message_content)
		{
		   encoded_bytes[encoded_stream_len] = 0x00 | (*message_content << 1);
		   k=1;
		   *message_content++;
                   if (!*message_content)
                    {
                      encoded_stream_len++;
                      len_encoded_bytes++;
                      if ((extra_zero_bits-1)>0) //One zero is already appended. If there are any extra 0's
                         check_zero_bits = 1;

                   }
		}
                else if (extra_zero_bits>0) //any extra zero bits to be appended
                   check_zero_bits = 1;

	    }
	    else
	    {
            encoded_bytes[encoded_stream_len] = 0x00 | ((*message_content << 1) << k);
            k++;
            message_content++;
               if(!*message_content)
            {
                encoded_stream_len++;
                len_encoded_bytes++;
                if ((extra_zero_bits-k)>0) //extra 0 bits after appending k 0's
                   check_zero_bits = 1;

            }
        }
        }
	//if there are any extra 0 bits add 00 byte in the end
            if (check_zero_bits == 1)
            {
           encoded_bytes[encoded_stream_len] = 0x00;
           encoded_stream_len++;
           len_encoded_bytes++;
        }

        encoded_bytes[0] = len_encoded_bytes;
    }
    }



    return encoded_stream_len;
}

int cri_wms_core_decode_message_content(int is_gw,
                                        char *message_content,
                                        uint8_t *encoded_bytes)
{
    int encoded_bytes_len;
    int iter_i;
    int iter_j;
    unsigned char shift;
    unsigned char previous_value;
    unsigned char current_value;

    encoded_bytes_len = 0;
    iter_i = 0;
    iter_j = 0;
    shift = 0;
    previous_value = 0;
    current_value = 0;

    if(is_gw)
    {
      if(encoded_bytes != NULL && message_content != NULL)
      {
          //Both concatenated and text messages have the same decoding structure.
          //concatenated msgs are sent in 2 or 3 PDUs are received as inducidual msgs

	 UTIL_LOG_MSG("Receive SMS message decode GW\n");
          //the length of the message content
         encoded_bytes_len = (((encoded_bytes[iter_i++] * 7 - 1)/8) + 1) + 1;

          //the message content in GSM structure is decoded
          //the appended 'x' last bits is taken into previous value and "or"ed with current
          //bits
         while(iter_i < encoded_bytes_len)
         {
           shift = iter_j & 0x07;
           current_value = encoded_bytes[iter_i++];
           message_content[iter_j++] = ( (current_value << shift) | (previous_value >> (8-shift)) ) & 0x7F;
           if(shift == 6)
           {
             if((current_value >> 1) == 0x0D && iter_i == encoded_bytes_len)
             {
               break;
             }
             message_content[iter_j++] = current_value >> 1;
           }
           previous_value = current_value;
         }
         message_content[iter_j] = '\0';
       }
    }
    else
    {
        UTIL_LOG_MSG("Incoming SMS message content decode CDMA\n");
	uint8_t encoded_bytes_len = encoded_bytes[iter_i++];
	uint8_t encoding_scheme = 0x00 | (encoded_bytes[iter_i] >> 3);

	uint8_t msg_len = 0x00 | (encoded_bytes[iter_i++] << 5);
	msg_len = msg_len | (encoded_bytes[iter_i] >> 3);

	switch(encoding_scheme)
	{
	  case 2:
		UTIL_LOG_MSG("\n7 bit ASCII encoding\n");
		int k=3;
		//Read 7bits continuously into a character until msg len
		while(iter_i<=encoded_bytes_len && iter_j<msg_len)
		{
		    uint8_t temp = 0x00 |(encoded_bytes[iter_i++] << (8-k));
		    k++;
		    message_content[iter_j++] = temp>>1 | (encoded_bytes[iter_i] >> k);
		    if(k>=7) k=0;
		}
		break;
	  case 8:
		UTIL_LOG_MSG("\n Scheme:8 \n");
		//Read 8bits continuously into a character until msg_len
		while(iter_i<=encoded_bytes_len && iter_j<msg_len)
		{
			uint8_t temp = 0x00 | (encoded_bytes[iter_i++] << 5);
			temp = temp | (encoded_bytes[iter_i] >> 3);
			message_content[iter_j++] = temp;
		}
		break;
	  default:
		UTIL_LOG_MSG("\nEncoding scheme not supported\n");
		break;
	}
	message_content[iter_j] = '\0';
      }
    return encoded_bytes_len;
}


int cri_wms_core_decode_cb_message(int is_gw,
                                   char *message_content,
                                   uint8_t *encoded_bytes)
{
    UTIL_LOG_MSG("\ncri_wms_core_decode_cb_message entry\n");
    uint8_t data_coding_scheme = encoded_bytes[5];
    int encoded_bytes_len;
    int iter_i;
    int iter_j;
    unsigned char shift;
    unsigned char previous_value;
    unsigned char current_value;

    encoded_bytes_len = 0;
    iter_i = 0;
    iter_j = 0;
    shift = 0;
    previous_value = 0;
    current_value = 0;

    if(is_gw)
    {
        iter_i = 6;

        //the message content in GSM structure is decoded
        //the appended 'x' last bits is taken into previous value and "or"ed with current
        //bits
             while(encoded_bytes[iter_j])
             {
               shift = iter_j & 0x07;
               current_value = encoded_bytes[iter_i++];
               message_content[iter_j++] = ( (current_value << shift) | (previous_value >> (8-shift)) ) & 0x7F;
               if(shift == 6)
               {
                 if((current_value >> 1) == 0x0D && iter_i == encoded_bytes_len)
                 {
                   break;
                 }
                 message_content[iter_j++] = current_value >> 1;
               }
               previous_value = current_value;
             }
             message_content[iter_j] = '\0';

    }
    else
    {
        uint8_t encoding_scheme = 0x00 | (encoded_bytes[iter_i] >> 3);

        uint8_t msg_len = 0x00 | (encoded_bytes[iter_i++] << 5);
	    msg_len = msg_len | (encoded_bytes[iter_i] >> 3);

	    switch(encoding_scheme)
	    {
	    case 2:
            UTIL_LOG_MSG("\n7 bit ASCII encoding\n");
		    int k=3;
		    //Read 7bits continuously into a character until msg len
		    while(iter_j<msg_len && encoded_bytes[iter_i])
		    {
		        uint8_t temp = 0x00 |(encoded_bytes[iter_i++] << (8-k));
                k++;
		        message_content[iter_j++] = temp>>1 | (encoded_bytes[iter_i] >> k);
		        if(k>=7) k=0;
		    }
		    break;
	    case 8:
          UTIL_LOG_MSG("\n Scheme:8 \n");
		    //Read 8bits continuously into a character until msg_len
		    while(iter_j<msg_len && encoded_bytes[iter_i])
            {
			    uint8_t temp = 0x00 | (encoded_bytes[iter_i++] << 5);
			    temp = temp | (encoded_bytes[iter_i] >> 3);
			    message_content[iter_j++] = temp;
		    }
		    break;
        default:
          UTIL_LOG_MSG("\nEncoding scheme not supported\n");
		    break;
	    }
	    message_content[iter_j] = '\0';
    }
    UTIL_LOG_MSG("\ncri_wms_core_decode_cb_message exit\n");
    return iter_j;
}

