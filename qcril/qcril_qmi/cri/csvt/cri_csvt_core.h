/***************************************************************************************************
    @file
    cri_csvt_core.h

    @brief
    Supports functions for handling QMI CSVT requests, responses and indications.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_CSVT_CORE
#define CRI_CSVT_CORE

#include "utils_common.h"
#include "cri_core.h"
#include "cri_csvt_utils.h"
#include "circuit_switched_video_telephony_v01.h"






/***************************************************************************************************
    @function
    cri_csvt_core_async_resp_handler

    @brief
    Handles QMI CSVT async responses.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the async response
        message_id
           message id of the async resp message
        resp_data
           pointer to the response message that was received
        resp_data_len
           length of the response message that was received
        cri_core_context
           context comprising of HLOS token id, subscription id and CRI token id

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_core_async_resp_handler(int qmi_service_client_id,
                                      unsigned long message_id,
                                      void *resp_data,
                                      int resp_data_len,
                                      cri_core_context_type cri_core_context);





/***************************************************************************************************
    @function
    cri_csvt_core_unsol_ind_handler

    @brief
    Handles QMI CSVT indications.

    @param[in]
        qmi_service_client_id
           CRI client ID of QMI CSVT client that received the indication
        message_id
           message id of the indication
        ind_data
           pointer to the indication data that was received
        ind_data_len
           length of the indication data that was received

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_core_unsol_ind_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *ind_data,
                                     int ind_data_len);






/***************************************************************************************************
    @function
    cri_csvt_core_dial_request_handler

    @brief
    Attempts to originate a call through QMI CSVT.

    @param[in]
        cri_core_context
           context comprising of HLOS token id and subscription id
        req_message
           pointer to the request message that needs to be sent
        hlos_cb_data
           user callback data that needs to be passed when calling the callback
        hlos_resp_cb
           user callback the needs to be called when a call has been originated through QMI CSVT

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if origination attempt was successful, appropriate error code otherwise
***************************************************************************************************/
cri_core_error_type cri_csvt_core_dial_request_handler(cri_core_context_type cri_core_context,
                                                       csvt_originate_call_req_msg_v01 *req_message,
                                                       void *hlos_cb_data,
                                                       hlos_resp_cb_type hlos_resp_cb);





/***************************************************************************************************
    @function
    cri_csvt_core_answer_request_handler

    @brief
    Attempts to answer a incoming call through QMI CSVT.

    @param[in]
        cri_core_context
           context comprising of HLOS token id and subscription id
        req_message
           pointer to the request message that needs to be sent
        hlos_cb_data
           user callback data that needs to be passed when calling the callback
        hlos_resp_cb
           user callback the needs to be called when a call has been answered through QMI CSVT

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if answer attempt was successful, appropriate error code otherwise
***************************************************************************************************/
cri_core_error_type cri_csvt_core_answer_request_handler(cri_core_context_type cri_core_context,
                                                         int hlos_call_id,
                                                         int is_answer,
                                                         int reject_value,
                                                         void *hlos_cb_data,
                                                         hlos_resp_cb_type hlos_resp_cb);





/***************************************************************************************************
    @function
    cri_csvt_core_end_request_handler

    @brief
    Attempts to end a ongoing call through QMI CSVT.

    @param[in]
        cri_core_context
           context comprising of HLOS token id and subscription id
        hlos_call_id
           HLOS call of of the call that needs to be ended
        hlos_cb_data
           user callback data that needs to be passed when calling the callback
        hlos_resp_cb
           user callback the needs to be called when a call has been ended through QMI CSVT

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if end attempt was successful, appropriate error code otherwise
***************************************************************************************************/
cri_core_error_type cri_csvt_core_end_request_handler(cri_core_context_type cri_core_context,
                                                      int hlos_call_id,
                                                      void *hlos_cb_data,
                                                      hlos_resp_cb_type hlos_resp_cb);





/***************************************************************************************************
    @function
    cri_csvt_core_confirm_request_handler

    @brief
    Attempts to confirm a incoming call through QMI CSVT.

    @param[in]
        cri_core_context
           context comprising of HLOS token id and subscription id
        req_message
           pointer to the request message that needs to be sent
        hlos_cb_data
           user callback data that needs to be passed when calling the callback
        hlos_resp_cb
           user callback the needs to be called when a call has been confirmed through QMI CSVT

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if confirm attempt was successful, appropriate error code otherwise
***************************************************************************************************/
cri_core_error_type cri_csvt_core_confirm_request_handler(
                                                        cri_core_context_type cri_core_context,
                                                        csvt_confirm_call_req_msg_v01 *req_message,
                                                        void *hlos_cb_data,
                                                        hlos_resp_cb_type hlos_resp_cb);


#endif
