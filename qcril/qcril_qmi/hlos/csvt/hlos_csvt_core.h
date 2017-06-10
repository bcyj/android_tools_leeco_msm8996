/***************************************************************************************************
    @file
    hlos_csvt_core.h

    @brief
    Supports functions for handling HLOS CSVT requests.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef HLOS_CSVT_CORE
#define HLOS_CSVT_CORE

#include "utils_common.h"







/***************************************************************************************************
    @function
    hlos_csvt_unsol_ind_handler

    @brief
    Handles CRI CSVT indications.

    @param[in]
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
void hlos_csvt_unsol_ind_handler(unsigned long message_id,
                                 void *ind_data,
                                 int ind_data_len);





/***************************************************************************************************
    @function
    hlos_csvt_request_handler

    @brief
    Checks If a HLOS request is related to CSVT and If so, queues it up so that it can be processed
    in the context of core thread afterwards.

    @param[in]
        request_id
           request id of the HLOS request
        token_id
           token id of the HLOS request
        data
           data related to the HLOS request
        data_len
           length of HLOS request data

    @param[out]
        none

    @retval
    TRUE if HLOS request is related to CSVT, FALSE otherwise
***************************************************************************************************/
int hlos_csvt_request_handler(Ims__MsgId request_id,
                              uint32_t token_id,
                              void *data,
                              size_t data_len);






/***************************************************************************************************
    @function
    hlos_csvt_dial_request_handler

    @brief
    Handles IMS__MSG_ID__REQUEST_DIAL for CSVT call in context of the core thread
    (as opposed to the IMS thread).

    @param[in]
        event_data
           pointer to data that comprises of the HLOS request related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void hlos_csvt_dial_request_handler(void *event_data);





/***************************************************************************************************
    @function
    hlos_csvt_answer_request_handler

    @brief
    Handles IMS__MSG_ID__REQUEST_ANSWER for CSVT call in context of the core thread
    (as opposed to the IMS thread).

    @param[in]
        event_data
           pointer to data that comprises of the HLOS request related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void hlos_csvt_answer_request_handler(void *event_data);





/***************************************************************************************************
    @function
    hlos_csvt_hangup_request_handler

    @brief
    Handles IMS__MSG_ID__REQUEST_HANGUP for CSVT call in context of the core thread
    (as opposed to the IMS thread).

    @param[in]
        event_data
           pointer to data that comprises of the HLOS request related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void hlos_csvt_hangup_request_handler(void *event_data);






/***************************************************************************************************
    @function
    hlos_csvt_get_current_calls_request_handler

    @brief
    Handles IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS for CSVT call in context of the core thread
    (as opposed to the IMS thread).

    @param[in]
        event_data
           pointer to data that comprises of the HLOS request related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void hlos_csvt_get_current_calls_request_handler(void *event_data);







/***************************************************************************************************
    @function
    hlos_csvt_last_call_failure_cause_request_handler

    @brief
    Handles IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE for CSVT call in context of the core thread
    (as opposed to the IMS thread).

    @param[in]
        event_data
           pointer to data that comprises of the HLOS request related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void hlos_csvt_last_call_failure_cause_request_handler(void *event_data);





#endif

