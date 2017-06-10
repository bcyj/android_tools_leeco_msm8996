/***************************************************************************************************
    @file
    cri_csvt_core.c

    @brief
    Implements functions supported in cri_csvt_core.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "cri_csvt_core.h"
#include "cri_csvt_rules.h"
#include "cri_rule_handler.h"
#include "qcril_cm_util.h"






/***************************************************************************************************
    @function
    cri_csvt_originate_resp_handler

    @brief
    Handles QMI CSVT origination response.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the async response
        resp_data
           pointer to the response message that was received
        cri_core_context
           context comprising of HLOS token id, subscription id and CRI token id

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void cri_csvt_originate_resp_handler(int qmi_service_client_id,
                                            csvt_originate_call_resp_msg_v01
                                            *csvt_originate_call_resp_msg,
                                            cri_core_context_type cri_core_context);





/***************************************************************************************************
    @function
    cri_csvt_answer_resp_handler

    @brief
    Handles QMI CSVT answer response.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the async response
        resp_data
           pointer to the response message that was received
        cri_core_context
           context comprising of HLOS token id, subscription id and CRI token id

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void cri_csvt_answer_resp_handler(int qmi_service_client_id,
                                         csvt_answer_call_resp_msg_v01 *csvt_answer_call_resp_msg,
                                         cri_core_context_type cri_core_context);





/***************************************************************************************************
    @function
    cri_csvt_end_resp_handler

    @brief
    Handles QMI CSVT end response.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the async response
        resp_data
           pointer to the response message that was received
        cri_core_context
           context comprising of HLOS token id, subscription id and CRI token id

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void cri_csvt_end_resp_handler(int qmi_service_client_id,
                                      csvt_end_call_resp_msg_v01 *csvt_end_call_resp_msg,
                                      cri_core_context_type cri_core_context);




/***************************************************************************************************
    @function
    cri_csvt_confirm_resp_handler

    @brief
    Handles QMI CSVT confirm response.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the async response
        resp_data
           pointer to the response message that was received
        cri_core_context
           context comprising of HLOS token id, subscription id and CRI token id

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void cri_csvt_confirm_resp_handler(int qmi_service_client_id,
                                          csvt_confirm_call_resp_msg_v01
                                          *csvt_confirm_call_resp_msg,
                                          cri_core_context_type cri_core_context);





/***************************************************************************************************
    @function
    cri_csvt_event_report_ind_handler

    @brief
    Handles QMI CSVT event report indication.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the indication
        ind_data
           pointer to the indication data that was received

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void cri_csvt_event_report_ind_handler(int qmi_service_client_id,
                                              csvt_event_report_ind_msg_v01
                                              *csvt_event_report_ind_msg);






/***************************************************************************************************
    @function
    cri_csvt_core_async_resp_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_core_async_resp_handler(int qmi_service_client_id,
                                      unsigned long message_id,
                                      void *resp_data,
                                      int resp_data_len,
                                      cri_core_context_type cri_core_context)
{
    UTIL_LOG_MSG("entry");

    if(resp_data && resp_data_len)
    {
        switch(message_id)
        {
            case QMI_CSVT_ORIGINATE_CALL_RESP_V01:
                cri_csvt_originate_resp_handler(qmi_service_client_id,
                                                resp_data,
                                                cri_core_context);
                break;

            case QMI_CSVT_ANSWER_CALL_RESP_V01:
                cri_csvt_answer_resp_handler(qmi_service_client_id,
                                             resp_data,
                                             cri_core_context);
                break;

            case QMI_CSVT_END_CALL_RESP_V01:
                cri_csvt_end_resp_handler(qmi_service_client_id,
                                          resp_data,
                                          cri_core_context);
                break;

            case QMI_CSVT_CONFIRM_CALL_RESP_V01:
                cri_csvt_confirm_resp_handler(qmi_service_client_id,
                                              resp_data,
                                              cri_core_context);
                break;

            default:
                //no action
                break;

        }
    }

    UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    cri_csvt_originate_resp_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_originate_resp_handler(int qmi_service_client_id,
                                     csvt_originate_call_resp_msg_v01 *csvt_originate_call_resp_msg,
                                     cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;

    cri_core_error = QMI_ERR_NONE_V01;

    if(csvt_originate_call_resp_msg)
    {
        cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR,
                                                    &csvt_originate_call_resp_msg->resp);
        cri_rule_handler_rule_check(cri_core_context,
                                    cri_core_error,
                                    NULL);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_answer_resp_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_answer_resp_handler(int qmi_service_client_id,
                                  csvt_answer_call_resp_msg_v01 *csvt_answer_call_resp_msg,
                                  cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;

    cri_core_error = QMI_ERR_NONE_V01;

    if(csvt_answer_call_resp_msg)
    {
        cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR,
                                                    &csvt_answer_call_resp_msg->resp);
        cri_rule_handler_rule_check(cri_core_context,
                                    cri_core_error,
                                    NULL);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_end_resp_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_end_resp_handler(int qmi_service_client_id,
                               csvt_end_call_resp_msg_v01 *csvt_end_call_resp_msg,
                               cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;

    cri_core_error = QMI_ERR_NONE_V01;

    if(csvt_end_call_resp_msg)
    {
        cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR,
                                                    &csvt_end_call_resp_msg->resp);
        cri_rule_handler_rule_check(cri_core_context,
                                    cri_core_error,
                                    NULL);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_confirm_resp_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_confirm_resp_handler(int qmi_service_client_id,
                                   csvt_confirm_call_resp_msg_v01 *csvt_confirm_call_resp_msg,
                                   cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;

    cri_core_error = QMI_ERR_NONE_V01;

    if(csvt_confirm_call_resp_msg)
    {
        cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR,
                                                    &csvt_confirm_call_resp_msg->resp);
        cri_rule_handler_rule_check(cri_core_context,
                                    cri_core_error,
                                    NULL);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_core_unsol_ind_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_core_unsol_ind_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *ind_data,
                                     int ind_data_len)
{
    UTIL_LOG_MSG("entry");

    switch(message_id)
    {
        case QMI_CSVT_EVENT_REPORT_IND_V01:
            cri_csvt_event_report_ind_handler(qmi_service_client_id,
                                              ind_data);
            break;

        default:
            //no action
            break;
    }

    UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    cri_csvt_event_report_ind_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_event_report_ind_handler(int qmi_service_client_id,
                                       csvt_event_report_ind_msg_v01 *csvt_event_report_ind_msg)
{
    int csvt_call_object_id;
    hlos_ind_cb_type hlos_ind_cb;
    int is_send_ind_to_hlos;
    char bcd_incoming_number[CSVT_MAX_DIAL_STRING_LEN_V01 + 1];
    char incoming_number[CSVT_MAX_DIAL_STRING_LEN_V01 + 1];

    UTIL_LOG_MSG("entry");

    csvt_call_object_id = NIL;
    hlos_ind_cb = NULL;
    is_send_ind_to_hlos = FALSE;
    memset(bcd_incoming_number,
           NIL,
           sizeof(bcd_incoming_number));
    memset(incoming_number,
           NIL,
           sizeof(incoming_number));

    if(csvt_event_report_ind_msg)
    {
        cri_csvt_utils_log_csvt_call_objects();
        UTIL_LOG_MSG("processing indication");
        switch(csvt_event_report_ind_msg->event_type)
        {
            case CSVT_EVENT_TYPE_CONFIRM_V01:
                csvt_call_object_id = cri_csvt_utils_find_csvt_call_object_id_for_confirm_event(
                                              csvt_event_report_ind_msg->instance_id
                                          );
                if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
                {
                    cri_csvt_utils_update_csvt_call_object_with_csvt_info(
                        csvt_call_object_id,
                        csvt_event_report_ind_msg
                    );
                    cri_rule_handler_rule_check(NIL,
                                                QMI_ERR_NONE_V01,
                                                NULL);
                    is_send_ind_to_hlos = TRUE;
                }
                break;

            case CSVT_EVENT_TYPE_PROGRESS_V01:
            case CSVT_EVENT_TYPE_CONNECT_V01:
                csvt_call_object_id = cri_csvt_utils_find_csvt_call_object_id_based_on_qmi_id(
                                          csvt_event_report_ind_msg->instance_id
                                      );
                if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
                {
                    cri_csvt_utils_update_csvt_call_object_with_csvt_info(
                        csvt_call_object_id,
                        csvt_event_report_ind_msg
                    );
                    cri_rule_handler_rule_check(NIL,
                                                QMI_ERR_NONE_V01,
                                                NULL);
                    is_send_ind_to_hlos = TRUE;
                }
                break;

            case CSVT_EVENT_TYPE_SETUP_V01:
                cri_csvt_utils_confirm_call_based_on_qmi_id(csvt_event_report_ind_msg->instance_id);
                break;

            case CSVT_EVENT_TYPE_INCOMING_V01:
                if(TRUE == csvt_event_report_ind_msg->incoming_number_valid)
                {
                    UTIL_LOG_MSG("converting bcd incoming number to ascii");
                    if(TRUE == csvt_event_report_ind_msg->incoming_number_length_valid)
                    {
                        bcd_incoming_number[0] = csvt_event_report_ind_msg->incoming_number_length;
                    }
                    else
                    {
                        bcd_incoming_number[0] = strlen(csvt_event_report_ind_msg->incoming_number);
                    }
                    strlcpy(&bcd_incoming_number[1],
                            csvt_event_report_ind_msg->incoming_number,
                            sizeof(bcd_incoming_number)-1);
                    qcril_cm_util_bcd_to_ascii((byte*) bcd_incoming_number,
                                               (byte*) incoming_number);
                }
                csvt_call_object_id = cri_csvt_utils_allocate_csvt_call_object(incoming_number,
                                                                               TRUE);
                if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
                {
                    cri_csvt_utils_update_csvt_call_object_with_csvt_info(
                        csvt_call_object_id,
                        csvt_event_report_ind_msg
                    );
                    is_send_ind_to_hlos = TRUE;
                }
                break;

            case CSVT_EVENT_TYPE_END_V01:
                csvt_call_object_id = cri_csvt_utils_find_csvt_call_object_id_based_on_qmi_id(
                                          csvt_event_report_ind_msg->instance_id
                                      );
                if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
                {
                    cri_csvt_utils_update_csvt_call_object_with_csvt_info(
                        csvt_call_object_id,
                        csvt_event_report_ind_msg
                    );
                    cri_rule_handler_rule_check(NIL,
                                                QMI_ERR_NONE_V01,
                                                NULL);
                    cri_csvt_utils_setup_timer_to_invalidate_csvt_call_object(csvt_call_object_id);
                    is_send_ind_to_hlos = TRUE;
                }
                break;

            default: //no action
                break;
        }
        cri_csvt_utils_log_csvt_call_objects();

        if(TRUE == is_send_ind_to_hlos)
        {
            UTIL_LOG_MSG("HLOS inform episode");
            hlos_ind_cb = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
            if(hlos_ind_cb)
            {
                ((*hlos_ind_cb) (QMI_CSVT_EVENT_REPORT_IND_V01,
                                 NULL,
                                 NIL));
            }
            else
            {
                UTIL_LOG_MSG("No registered HLOS ind handler");
            }
        }
    }

    UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    cri_csvt_core_dial_request_handler

    @implementation detail
    None.
***************************************************************************************************/
cri_core_error_type cri_csvt_core_dial_request_handler(cri_core_context_type cri_core_context,
                                                       csvt_originate_call_req_msg_v01 *req_message,
                                                       void *hlos_cb_data,
                                                       hlos_resp_cb_type hlos_resp_cb)
{
    qmi_error_type_v01 ret_val;
    int csvt_call_object_id;
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;
    cri_rule_handler_user_rule_info_type user_rule_info;

    ret_val = QMI_ERR_INTERNAL_V01;
    csvt_call_object_id = NIL;
    cri_csvt_rules_generic_rule_data = NULL;
    memset(&user_rule_info,
           NIL,
           sizeof(user_rule_info));

    if(req_message)
    {
        csvt_call_object_id = cri_csvt_utils_allocate_csvt_call_object(req_message->dial_string,
                                                                       FALSE);
        if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
        {
            req_message->instance_id = cri_csvt_utils_find_hlos_id_based_on_csvt_call_object_id(
                                           csvt_call_object_id
                                       );
            req_message->call_mode = CSVT_CALL_MODE_DATA_V01;

            cri_csvt_utils_update_csvt_call_object_with_qmi_id(csvt_call_object_id,
                                                               req_message->instance_id);

            cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*)
                                    util_memory_alloc(sizeof(*cri_csvt_rules_generic_rule_data));
            if(cri_csvt_rules_generic_rule_data)
            {
                cri_csvt_rules_generic_rule_data->hlos_call_id =
                    cri_csvt_utils_find_hlos_id_based_on_csvt_call_object_id(csvt_call_object_id);

                user_rule_info.rule_data = cri_csvt_rules_generic_rule_data;
                user_rule_info.rule_check_handler = cri_csvt_rules_originating_rule_check_handler;
                user_rule_info.rule_data_free_handler =
                                            cri_csvt_rules_generic_rule_data_free_handler;
                ret_val =  cri_core_qmi_send_msg_async(
                               cri_core_context,
                               csvt_client_id,
                               QMI_CSVT_ORIGINATE_CALL_REQ_V01,
                               req_message,
                               sizeof(*req_message),
                               sizeof(csvt_originate_call_resp_msg_v01),
                               hlos_cb_data,
                               hlos_resp_cb,
                               CRI_CORE_MAX_TIMEOUT,
                               &user_rule_info);
            }

            if(QMI_ERR_NONE_V01 != ret_val)
            {
                cri_csvt_utils_invalidate_csvt_call_object(csvt_call_object_id);
                if(cri_csvt_rules_generic_rule_data)
                {
                    cri_csvt_rules_generic_rule_data_free_handler(cri_csvt_rules_generic_rule_data);
                }
            }
        }
    }

    return ret_val;
}

/***************************************************************************************************
    @function
    cri_csvt_core_answer_request_handler

    @implementation detail
    None.
***************************************************************************************************/
cri_core_error_type cri_csvt_core_answer_request_handler(cri_core_context_type cri_core_context,
                                                         int hlos_call_id,
                                                         int is_answer,
                                                         int reject_value,
                                                         void *hlos_cb_data,
                                                         hlos_resp_cb_type hlos_resp_cb)
{
    csvt_answer_call_req_msg_v01 csvt_answer_call_req_msg;
    qmi_error_type_v01 ret_val;
    int csvt_call_object_id;
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;
    cri_rule_handler_user_rule_info_type user_rule_info;

    memset(&csvt_answer_call_req_msg, NIL, sizeof(csvt_answer_call_req_msg));
    ret_val = QMI_ERR_INTERNAL_V01;
    csvt_call_object_id = NIL;
    cri_csvt_rules_generic_rule_data = NULL;
    memset(&user_rule_info,
           NIL,
           sizeof(user_rule_info));

    csvt_call_object_id =
                        cri_csvt_utils_find_csvt_call_object_id_based_on_hlos_call_id(hlos_call_id);
    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        csvt_answer_call_req_msg.instance_id =
                    cri_csvt_utils_find_qmi_id_based_on_csvt_call_object_id(csvt_call_object_id);
        csvt_answer_call_req_msg.answer_call = is_answer;
        if(NIL != reject_value)
        {
            csvt_answer_call_req_msg.reject_value_valid = TRUE;
            csvt_answer_call_req_msg.reject_value = reject_value;
        }

        cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*)
                                    util_memory_alloc(sizeof(*cri_csvt_rules_generic_rule_data));
        if(cri_csvt_rules_generic_rule_data)
        {
            cri_csvt_rules_generic_rule_data->hlos_call_id = hlos_call_id;

            user_rule_info.rule_data = cri_csvt_rules_generic_rule_data;
            user_rule_info.rule_check_handler = cri_csvt_rules_answering_rule_check_handler;
            user_rule_info.rule_data_free_handler =
                                        cri_csvt_rules_generic_rule_data_free_handler;

            ret_val =  cri_core_qmi_send_msg_async(cri_core_context,
                                                   csvt_client_id,
                                                   QMI_CSVT_ANSWER_CALL_REQ_V01,
                                                   &csvt_answer_call_req_msg,
                                                   sizeof(csvt_answer_call_req_msg),
                                                   sizeof(csvt_answer_call_resp_msg_v01),
                                                   hlos_cb_data,
                                                   hlos_resp_cb,
                                                   CRI_CORE_MAX_TIMEOUT,
                                                   &user_rule_info);
        }

        if(QMI_ERR_NONE_V01 != ret_val)
        {
            if(cri_csvt_rules_generic_rule_data)
            {
                cri_csvt_rules_generic_rule_data_free_handler(cri_csvt_rules_generic_rule_data);
            }
        }
    }

    return ret_val;
}

/***************************************************************************************************
    @function
    cri_csvt_core_end_request_handler

    @implementation detail
    None.
***************************************************************************************************/
cri_core_error_type cri_csvt_core_end_request_handler(cri_core_context_type cri_core_context,
                                                      int hlos_call_id,
                                                      void *hlos_cb_data,
                                                      hlos_resp_cb_type hlos_resp_cb)
{
    csvt_end_call_req_msg_v01 csvt_end_call_req_msg;
    qmi_error_type_v01 ret_val;
    int csvt_call_object_id;
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;
    cri_rule_handler_user_rule_info_type user_rule_info;

    memset(&csvt_end_call_req_msg, NIL, sizeof(csvt_end_call_req_msg));
    ret_val = QMI_ERR_INTERNAL_V01;
    csvt_call_object_id = NIL;
    cri_csvt_rules_generic_rule_data = NULL;
    memset(&user_rule_info,
           NIL,
           sizeof(user_rule_info));

    csvt_call_object_id = cri_csvt_utils_find_csvt_call_object_id_based_on_hlos_call_id(
                              hlos_call_id
                          );
    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        csvt_end_call_req_msg.instance_id = cri_csvt_utils_find_qmi_id_based_on_csvt_call_object_id(
                                                 csvt_call_object_id
                                            );

        cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*)
                                    util_memory_alloc(sizeof(*cri_csvt_rules_generic_rule_data));
        if(cri_csvt_rules_generic_rule_data)
        {
            cri_csvt_rules_generic_rule_data->hlos_call_id = hlos_call_id;

            user_rule_info.rule_data = cri_csvt_rules_generic_rule_data;
            user_rule_info.rule_check_handler = cri_csvt_rules_ending_rule_check_handler;
            user_rule_info.rule_data_free_handler =
                                        cri_csvt_rules_generic_rule_data_free_handler;

            ret_val =  cri_core_qmi_send_msg_async(cri_core_context,
                                                   csvt_client_id,
                                                   QMI_CSVT_END_CALL_REQ_V01,
                                                   &csvt_end_call_req_msg,
                                                   sizeof(csvt_end_call_req_msg),
                                                   sizeof(csvt_end_call_resp_msg_v01),
                                                   hlos_cb_data,
                                                   hlos_resp_cb,
                                                   CRI_CORE_MAX_TIMEOUT,
                                                   &user_rule_info);
        }

        if(QMI_ERR_NONE_V01 != ret_val)
        {
            if(cri_csvt_rules_generic_rule_data)
            {
                cri_csvt_rules_generic_rule_data_free_handler(cri_csvt_rules_generic_rule_data);
            }
        }
    }

    return ret_val;
}

/***************************************************************************************************
    @function
    cri_csvt_core_confirm_request_handler

    @implementation detail
    None.
***************************************************************************************************/
cri_core_error_type cri_csvt_core_confirm_request_handler(
                                                        cri_core_context_type cri_core_context,
                                                        csvt_confirm_call_req_msg_v01 *req_message,
                                                        void *hlos_cb_data,
                                                        hlos_resp_cb_type hlos_resp_cb)
{
    qmi_error_type_v01 ret_val;

    ret_val = QMI_ERR_INTERNAL_V01;

    if(req_message)
    {
        ret_val =  cri_core_qmi_send_msg_async(cri_core_context,
                                               csvt_client_id,
                                               QMI_CSVT_CONFIRM_CALL_REQ_V01,
                                               req_message,
                                               sizeof(*req_message),
                                               sizeof(csvt_confirm_call_resp_msg_v01),
                                               hlos_cb_data,
                                               hlos_resp_cb,
                                               CRI_CORE_MAX_TIMEOUT,
                                               NULL);

    }

    return ret_val;
}

