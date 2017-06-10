/***************************************************************************************************
    @file
    cri_csvt_utils.c

    @brief
    Implements functions supported in cri_csvt_utils.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "cri_csvt_utils.h"
#include "cri_csvt_core.h"
#include "cri_core.h"
#include "circuit_switched_video_telephony_v01.h"

int csvt_client_id;
cri_csvt_call_object_type csvt_calls[CRI_CSVT_MAX_CALLS];





/***************************************************************************************************
    @function
    cri_csvt_init_client_state

    @brief
    Intializes the QMI CSVT client once it is created.

    @param[in]
        subscription_id
            subscription on which the client needs to be bound upon

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void cri_csvt_init_client_state(cri_core_subscription_id_type subscription_id);





/***************************************************************************************************
    @function
    cri_csvt_utils_init_client

    @implementation detail
    None.
***************************************************************************************************/
qmi_error_type_v01 cri_csvt_utils_init_client(cri_core_subscription_id_type subscription_id,
                                              hlos_ind_cb_type hlos_ind_cb)
{
    qmi_error_type_v01 client_init_error;

    client_init_error = QMI_ERR_INTERNAL_V01;

    csvt_client_id = cri_core_create_qmi_service_client(QMI_CSVT_SERVICE,
                                                        hlos_ind_cb);
    if(QMI_INTERNAL_ERR != csvt_client_id)
    {
        client_init_error = QMI_ERR_NONE_V01;
        cri_csvt_init_client_state(subscription_id);
    }

    return client_init_error;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_reset_client

    @implementation detail
    None.
***************************************************************************************************/
qmi_error_type_v01 cri_csvt_utils_reset_client()
{
    qmi_error_type_v01      client_init_error = QMI_ERR_INTERNAL_V01;
    csvt_reset_req_msg_v01  csvt_reset_req;
    csvt_reset_resp_msg_v01 csvt_reset_resp;

    UTIL_LOG_MSG("entry");

    if(QMI_INTERNAL_ERR != csvt_client_id)
    {
        memset(&csvt_reset_req, NIL, sizeof(csvt_reset_req));

        client_init_error = cri_core_qmi_send_msg_sync(csvt_client_id,
                               QMI_CSVT_RESET_REQ_V01,
                               (void *)&csvt_reset_req,
                               sizeof(csvt_reset_req),
                               &csvt_reset_resp,
                               sizeof(csvt_reset_resp),
                               CRI_CORE_MINIMAL_TIMEOUT);
    }

    UTIL_LOG_MSG("exit");

    return client_init_error;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_reinit_client

    @implementation detail
    None.
***************************************************************************************************/
qmi_error_type_v01 cri_csvt_utils_reinit_client(cri_core_subscription_id_type subscription_id)
{
    qmi_error_type_v01 client_init_error = QMI_ERR_INTERNAL_V01;

    UTIL_LOG_MSG("entry");

    if(QMI_INTERNAL_ERR != csvt_client_id)
    {
        client_init_error = QMI_ERR_NONE_V01;
        cri_csvt_init_client_state(subscription_id);
    }

    UTIL_LOG_MSG("exit");

    return client_init_error;
}
/***************************************************************************************************
    @function
    cri_csvt_init_client_state

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_init_client_state(cri_core_subscription_id_type subscription_id)
{
    csvt_set_subscription_binding_req_msg_v01 csvt_set_subscription_binding_req_msg;
    csvt_set_subscription_binding_resp_msg_v01 csvt_set_subscription_binding_resp_msg;
    csvt_set_event_report_req_msg_v01 csvt_set_event_report_req_msg;
    csvt_set_event_report_resp_msg_v01 csvt_set_event_report_resp_msg;

    memset(&csvt_set_subscription_binding_req_msg,
           NIL,
           sizeof(csvt_set_subscription_binding_req_msg));
    memset(&csvt_set_event_report_req_msg,
           NIL,
           sizeof(csvt_set_event_report_req_msg));

    switch(subscription_id)
    {
        case CRI_CORE_PRIMARY_CRI_SUBSCRIPTION_ID:
            csvt_set_subscription_binding_req_msg.bind_subs = CSVT_PRIMARY_SUBS_V01;
            break;

        case CRI_CORE_SECONDARY_CRI_SUBSCRIPTION_ID:
            csvt_set_subscription_binding_req_msg.bind_subs = CSVT_SECONDARY_SUBS_V01;
            break;

        case CRI_CORE_TERTIARY_CRI_SUBSCRIPTION_ID:
            csvt_set_subscription_binding_req_msg.bind_subs = CSVT_TERTIARY_SUBS_V01;
            break;

        default:
            csvt_set_subscription_binding_req_msg.bind_subs = CSVT_PRIMARY_SUBS_V01;
            break;
    }
    cri_core_qmi_send_msg_sync(csvt_client_id,
                               QMI_CSVT_SET_SUBSCRIPTION_BINDING_REQ_V01,
                               (void *)&csvt_set_subscription_binding_req_msg,
                               sizeof(csvt_set_subscription_binding_req_msg),
                               &csvt_set_subscription_binding_resp_msg,
                               sizeof(csvt_set_subscription_binding_resp_msg),
                               CRI_CORE_MINIMAL_TIMEOUT);


    csvt_set_event_report_req_msg.report_call_events_valid = TRUE;
    csvt_set_event_report_req_msg.report_call_events = TRUE;
    csvt_set_event_report_req_msg.call_types_valid = TRUE;
    csvt_set_event_report_req_msg.call_types = CSVT_MASK_ASYNC_CSD_CALL_V01 |
                                               CSVT_MASK_SYNC_CSD_CALL_V01 |
                                               CSVT_MASK_VIDEO_TELEPHONY_CALL_V01;

    cri_core_qmi_send_msg_sync(csvt_client_id,
                               QMI_CSVT_SET_EVENT_REPORT_REQ_V01,
                               (void *)&csvt_set_event_report_req_msg,
                               sizeof(csvt_set_event_report_req_msg),
                               &csvt_set_event_report_resp_msg,
                               sizeof(csvt_set_event_report_resp_msg),
                               CRI_CORE_MINIMAL_TIMEOUT);
}

/***************************************************************************************************
    @function
    cri_csvt_utils_release_client

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_release_client(int qmi_service_client_id)
{
    cri_csvt_utils_cleanup_calls();
    cri_core_release_qmi_service_client(qmi_service_client_id);
    csvt_client_id = NIL;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_cleanup_calls

    @implementation detail
    Set call state as END for all the ongoing call, inform HLOS and then clean up the calls
***************************************************************************************************/
void cri_csvt_utils_cleanup_calls()
{
    hlos_ind_cb_type hlos_ind_cb         = NULL;
    boolean          is_send_ind_to_hlos = FALSE;
    int              iter_call_object    = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(csvt_calls[iter_call_object].is_valid)
        {
            csvt_calls[iter_call_object].csvt_info.event_type = CSVT_EVENT_TYPE_END_V01;
            is_send_ind_to_hlos = TRUE;
        }
    }

    if (is_send_ind_to_hlos)
    {
        UTIL_LOG_MSG("HLOS inform episode");
        hlos_ind_cb = cri_core_retrieve_hlos_ind_cb(csvt_client_id);
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

    memset(csvt_calls,
           NIL,
           sizeof(csvt_calls));
}

/***************************************************************************************************
    @function
    cri_csvt_utils_log_csvt_call_objects

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_log_csvt_call_objects()
{
    int iter_call_object;

    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        UTIL_LOG_MSG("csvt call object id %d, is valid %d",
                     iter_call_object,
                     csvt_calls[iter_call_object].is_valid);
        if(TRUE == csvt_calls[iter_call_object].is_valid)
        {
            UTIL_LOG_MSG("      call state %d, hlos call id id %d, qmi call id %08x",
                         csvt_calls[iter_call_object].csvt_info.event_type,
                         csvt_calls[iter_call_object].hlos_call_id,
                         csvt_calls[iter_call_object].csvt_info.instance_id);

            if(FALSE == csvt_calls[iter_call_object].is_mt)
            {
                UTIL_LOG_MSG("      mo call, remote party number %s",
                             csvt_calls[iter_call_object].remote_party_number);
            }
            else
            {
                UTIL_LOG_MSG("      mt call, remote party number %s",
                             csvt_calls[iter_call_object].remote_party_number);
            }

            if(TRUE == csvt_calls[iter_call_object].csvt_info.call_end_cause_valid)
            {
                UTIL_LOG_MSG("      call end cause %d",
                             csvt_calls[iter_call_object].csvt_info.call_end_cause);
            }
        }
    }
}

/***************************************************************************************************
    @function
    cri_csvt_utils_retrieve_number_of_ongoing_csvt_calls

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_retrieve_number_of_ongoing_csvt_calls(boolean ignore_call_end)
{
    int iter_call_object;
    int number_of_ongoing_csvt_calls;

    iter_call_object = NIL;
    number_of_ongoing_csvt_calls = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           CSVT_EVENT_TYPE_SETUP_V01 != csvt_calls[iter_call_object].csvt_info.event_type)
        {
            if (CSVT_EVENT_TYPE_END_V01 == csvt_calls[iter_call_object].csvt_info.event_type &&
                    ignore_call_end)
            {
                continue;
            }
            number_of_ongoing_csvt_calls++;
        }
    }

    UTIL_LOG_MSG("number of ongoing csvt calls %d",
                 number_of_ongoing_csvt_calls);

    return number_of_ongoing_csvt_calls;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_retrieve_hlos_ongoing_call_objects

    @implementation detail
    Allocates memory which has to be freed up by the caller.
***************************************************************************************************/
cri_csvt_utils_hlos_call_object_type* cri_csvt_utils_retrieve_hlos_ongoing_call_objects(
        boolean ignore_call_end
)
{
    int iter_call_object;
    int iter_ongoing_csvt_call;
    int number_of_ongoing_csvt_calls;
    cri_csvt_utils_hlos_call_object_type* hlos_ongoing_call_objects;

    UTIL_LOG_MSG("entry");

    iter_call_object = NIL;
    iter_ongoing_csvt_call = NIL;
    number_of_ongoing_csvt_calls = NIL;
    hlos_ongoing_call_objects = NULL;

    number_of_ongoing_csvt_calls = cri_csvt_utils_retrieve_number_of_ongoing_csvt_calls(
                                           ignore_call_end);
    if(NIL != number_of_ongoing_csvt_calls)
    {
        hlos_ongoing_call_objects = util_memory_alloc(number_of_ongoing_csvt_calls *
                                                      sizeof(*hlos_ongoing_call_objects));
        if(NULL != hlos_ongoing_call_objects)
        {
            for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
            {
                if(TRUE == csvt_calls[iter_call_object].is_valid &&
                   CSVT_EVENT_TYPE_SETUP_V01 != csvt_calls[iter_call_object].csvt_info.event_type)
                {
                    if ((CSVT_EVENT_TYPE_END_V01 ==
                                csvt_calls[iter_call_object].csvt_info.event_type) &&
                            ignore_call_end)
                    {
                        continue;
                    }

                    hlos_ongoing_call_objects[iter_ongoing_csvt_call].hlos_call_id =
                                                        csvt_calls[iter_call_object].hlos_call_id;
                    hlos_ongoing_call_objects[iter_ongoing_csvt_call].csvt_call_state =
                                                csvt_calls[iter_call_object].csvt_info.event_type;
                    strlcpy(hlos_ongoing_call_objects[iter_ongoing_csvt_call].remote_party_number,
                            csvt_calls[iter_call_object].remote_party_number,
                            sizeof(hlos_ongoing_call_objects[iter_ongoing_csvt_call].
                                   remote_party_number));
                    hlos_ongoing_call_objects[iter_ongoing_csvt_call].is_mt =
                                                                csvt_calls[iter_call_object].is_mt;
                    if (csvt_calls[iter_call_object].csvt_info.call_end_cause_valid == TRUE)
                    {
                        hlos_ongoing_call_objects[iter_ongoing_csvt_call].call_fail_cause =
                            csvt_calls[iter_call_object].csvt_info.call_end_cause;
                    }
                    iter_ongoing_csvt_call++;
                }
            }
        }
    }

    UTIL_LOG_MSG("exit");

    return hlos_ongoing_call_objects;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_retrieve_last_call_failure_cause

    @implementation detail
    None.
***************************************************************************************************/
uint32_t cri_csvt_utils_retrieve_last_call_failure_cause()
{
    int iter_call_object;
    uint32_t last_call_failure_cause;

    iter_call_object = NIL;
    last_call_failure_cause = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           CSVT_EVENT_TYPE_END_V01 == csvt_calls[iter_call_object].csvt_info.event_type)
        {
            if(TRUE == csvt_calls[iter_call_object].csvt_info.call_end_cause_valid)
            {
                last_call_failure_cause = csvt_calls[iter_call_object].csvt_info.call_end_cause;
            }
            cri_csvt_utils_invalidate_csvt_call_object(iter_call_object);
            break;
        }
    }

    UTIL_LOG_MSG("last call failure cause %d",
                 last_call_failure_cause);

    return last_call_failure_cause;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_is_hlos_call_id_belongs_to_csvt_call

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_is_hlos_call_id_belongs_to_csvt_call(int hlos_call_id)
{
    int is_hlos_call_id_belongs_to_csvt_call;
    int iter_call_object;

    is_hlos_call_id_belongs_to_csvt_call = FALSE;
    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid && hlos_call_id ==
           csvt_calls[iter_call_object].hlos_call_id)
        {
            is_hlos_call_id_belongs_to_csvt_call = TRUE;
            break;
        }
    }

    UTIL_LOG_MSG("hlos call id %d is csvt call %d",
                 hlos_call_id,
                 is_hlos_call_id_belongs_to_csvt_call);

    return is_hlos_call_id_belongs_to_csvt_call;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_is_csvt_calls_present

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_is_csvt_calls_present()
{
    int is_csvt_calls_present;
    int iter_call_object;

    is_csvt_calls_present = FALSE;
    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid)
        {
            is_csvt_calls_present = TRUE;
            break;
        }
    }

    UTIL_LOG_MSG("csvt calls present %d",
                 is_csvt_calls_present);

    return is_csvt_calls_present;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_find_csvt_call_object_id_for_confirm_event

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_find_csvt_call_object_id_for_confirm_event(uint32_t qmi_id)
{
    int iter_call_object;

    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           FALSE == csvt_calls[iter_call_object].is_mt &&
           NIL == csvt_calls[iter_call_object].csvt_info.event_type &&
           qmi_id == csvt_calls[iter_call_object].csvt_info.instance_id)
        {
            break;
        }
    }

    UTIL_LOG_MSG("csvt call object id %d for confirm event",
                 iter_call_object);

    return iter_call_object;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_find_qmi_id_based_on_csvt_call_object_id

    @implementation detail
    None.
***************************************************************************************************/
uint32_t cri_csvt_utils_find_qmi_id_based_on_csvt_call_object_id(int csvt_call_object_id)
{
    uint32_t qmi_id;

    qmi_id = NIL;

    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        qmi_id = csvt_calls[csvt_call_object_id].csvt_info.instance_id;
    }

    UTIL_LOG_MSG("csvt call object id %d, qmi call id %08x",
                 csvt_call_object_id, qmi_id);

    return qmi_id;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_find_csvt_call_object_id_based_on_qmi_id

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_find_csvt_call_object_id_based_on_qmi_id(uint32_t qmi_id)
{
    int iter_call_object;

    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           qmi_id == csvt_calls[iter_call_object].csvt_info.instance_id)
        {
            break;
        }
    }

    UTIL_LOG_MSG("qmi call id %08x, csvt call object id %d",
                 qmi_id,
                 iter_call_object);

    return iter_call_object;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_allocate_hlos_call_id

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_allocate_hlos_call_id()
{
    int iter_call_object;
    int hlos_call_id;

    iter_call_object = NIL;
    hlos_call_id = NIL;

    for(hlos_call_id = 1; hlos_call_id <= CRI_CSVT_MAX_CALLS; hlos_call_id++)
    {
        for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
        {
            if(TRUE == csvt_calls[iter_call_object].is_valid &&
               hlos_call_id == csvt_calls[iter_call_object].hlos_call_id)
            {
                break;
            }
        }

        if(CRI_CSVT_MAX_CALLS == iter_call_object)
        {
            break;
        }
    }

    UTIL_LOG_MSG("hlos call id allocated %d",
                 hlos_call_id);

    return hlos_call_id;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_find_hlos_call_id_in_csvt_call_state

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_find_hlos_call_id_in_csvt_call_state(csvt_event_type_enum_v01 csvt_call_state)
{
    int iter_call_object;
    int hlos_call_id;

    iter_call_object = NIL;
    hlos_call_id = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           csvt_call_state == csvt_calls[iter_call_object].csvt_info.event_type)
        {
            hlos_call_id = csvt_calls[iter_call_object].hlos_call_id;
            break;
        }
    }

    UTIL_LOG_MSG("call state %d, hlos call id %d",
                 csvt_call_state,
                 hlos_call_id);

    return hlos_call_id;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_is_csvt_call_with_hlos_call_id_in_csvt_call_state

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_is_csvt_call_with_hlos_call_id_in_csvt_call_state(
    int hlos_call_id,
    csvt_event_type_enum_v01 csvt_call_state
)
{
    int iter_call_object;
    int ret_code;

    iter_call_object = NIL;
    ret_code = FALSE;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           hlos_call_id == csvt_calls[iter_call_object].hlos_call_id &&
           csvt_call_state == csvt_calls[iter_call_object].csvt_info.event_type)
        {
            ret_code = TRUE;
            break;
        }
    }

    UTIL_LOG_MSG("call state %d, hlos call id %d, match %d",
                 csvt_call_state,
                 hlos_call_id,
                 ret_code);

    return ret_code;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_find_hlos_id_based_on_csvt_call_object_id

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_find_hlos_id_based_on_csvt_call_object_id(int csvt_call_object_id)
{
    int hlos_call_id;

    hlos_call_id = NIL;

    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        hlos_call_id = csvt_calls[csvt_call_object_id].hlos_call_id;
    }

    UTIL_LOG_MSG("csvt call object id %d, hlos call id %d",
                 csvt_call_object_id,
                 hlos_call_id);

    return hlos_call_id;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_find_csvt_call_object_id_based_on_hlos_call_id

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_find_csvt_call_object_id_based_on_hlos_call_id(int hlos_call_id)
{
    int iter_call_object;

    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(TRUE == csvt_calls[iter_call_object].is_valid &&
           hlos_call_id == csvt_calls[iter_call_object].hlos_call_id)
        {
            break;
        }
    }

    UTIL_LOG_MSG("hlos call id %d, csvt call object id %d",
                 hlos_call_id,
                 iter_call_object);

    return iter_call_object;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_update_csvt_call_object_with_csvt_info

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_update_csvt_call_object_with_csvt_info(
    int csvt_call_object_id,
    csvt_event_report_ind_msg_v01 *csvt_info
)
{
    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id) && csvt_info)
    {
        csvt_calls[csvt_call_object_id].is_valid = TRUE;
        memcpy(&csvt_calls[csvt_call_object_id].csvt_info,
               csvt_info,
               sizeof(csvt_calls[csvt_call_object_id].csvt_info));

        UTIL_LOG_MSG("csvt call object id updated %d",
                     csvt_call_object_id);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_utils_update_csvt_call_object_with_qmi_id

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_update_csvt_call_object_with_qmi_id(
    int csvt_call_object_id,
    uint32_t qmi_id
)
{
    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        csvt_calls[csvt_call_object_id].is_valid = TRUE;
        csvt_calls[csvt_call_object_id].csvt_info.instance_id = qmi_id;

        UTIL_LOG_MSG("csvt call object id updated %d with qmi id %u",
                     csvt_call_object_id,
                     qmi_id);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_utils_is_valid_csvt_call_object_id

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_is_valid_csvt_call_object_id(int csvt_call_object_id)
{
    return (csvt_call_object_id >= NIL && csvt_call_object_id < CRI_CSVT_MAX_CALLS);
}

/***************************************************************************************************
    @function
    cri_csvt_utils_confirm_call_based_on_qmi_id

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_confirm_call_based_on_qmi_id(uint32_t qmi_id)
{
    csvt_confirm_call_req_msg_v01 csvt_confirm_call_req_msg;

    memset(&csvt_confirm_call_req_msg,
           NIL,
           sizeof(csvt_confirm_call_req_msg));

    csvt_confirm_call_req_msg.instance_id = qmi_id;
    csvt_confirm_call_req_msg.confirm_call = TRUE;
    cri_csvt_core_confirm_request_handler(NIL,
                                          &csvt_confirm_call_req_msg,
                                          NULL,
                                          NULL);
}

/***************************************************************************************************
    @function
    cri_csvt_utils_setup_timer_to_invalidate_csvt_call_object

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_setup_timer_to_invalidate_csvt_call_object(int csvt_call_object_id)
{
    struct timeval csvt_call_object_invalidate_timeval;

    memset(&csvt_call_object_invalidate_timeval,
           NIL,
           sizeof(csvt_call_object_invalidate_timeval));

    csvt_call_object_invalidate_timeval.tv_sec = CRI_CSVT_CALL_OBJECT_INVALIDATE_TIMEOUT;

    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        csvt_calls[csvt_call_object_id].csvt_call_object_invalidate_timer_id =
            util_timer_add(&csvt_call_object_invalidate_timeval,
                           cri_csvt_utils_invalidate_csvt_call_object_timer_expiry_handler,
                           (void*)(intptr_t) csvt_call_object_id,
                           sizeof(csvt_call_object_id));
        UTIL_LOG_MSG("invalidate timer id %d, csvt call object id %d",
                     csvt_calls[csvt_call_object_id].csvt_call_object_invalidate_timer_id,
                     csvt_call_object_id);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_utils_invalidate_csvt_call_object_timer_expiry_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_invalidate_csvt_call_object_timer_expiry_handler(
    void *invalidate_csvt_call_object_cb_data,
    size_t invalidate_csvt_call_object_cb_data_len
)
{
    int csvt_call_object_id;

    csvt_call_object_id = NIL;

    UTIL_LOG_MSG("csvt call object id to be invalidated %d",
                 csvt_call_object_id);
    csvt_call_object_id = (intptr_t) invalidate_csvt_call_object_cb_data;
    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        csvt_calls[csvt_call_object_id].csvt_call_object_invalidate_timer_id = NIL;
        cri_csvt_utils_invalidate_csvt_call_object(csvt_call_object_id);
    }
}

/***************************************************************************************************
    @function
    cri_csvt_utils_allocate_csvt_call_object

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_utils_allocate_csvt_call_object(char *remote_party_number,
                                             int is_mt)
{
    int iter_call_object;

    iter_call_object = NIL;

    for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
    {
        if(FALSE == csvt_calls[iter_call_object].is_valid)
        {
            memset(&csvt_calls[iter_call_object], NIL, sizeof(csvt_calls[iter_call_object]));
            csvt_calls[iter_call_object].is_valid = TRUE;
            csvt_calls[iter_call_object].hlos_call_id = cri_csvt_utils_allocate_hlos_call_id();
            csvt_calls[iter_call_object].is_mt = is_mt;
            if(remote_party_number)
            {
                strlcpy(csvt_calls[iter_call_object].remote_party_number,
                        remote_party_number,
                        sizeof(csvt_calls[iter_call_object].remote_party_number));
            }
            UTIL_LOG_MSG("csvt call object id allocated %d, rp number %s, is mt %d",
                         iter_call_object,
                         csvt_calls[iter_call_object].remote_party_number,
                         csvt_calls[iter_call_object].is_mt);
            break;
        }
    }

    return iter_call_object;
}

/***************************************************************************************************
    @function
    cri_csvt_utils_invalidate_csvt_call_object

    @implementation detail
    None.
***************************************************************************************************/
void cri_csvt_utils_invalidate_csvt_call_object(int csvt_call_object_id)
{
    if(TRUE == cri_csvt_utils_is_valid_csvt_call_object_id(csvt_call_object_id))
    {
        if(NIL != csvt_calls[csvt_call_object_id].csvt_call_object_invalidate_timer_id)
        {
            util_timer_cancel(csvt_calls[csvt_call_object_id].csvt_call_object_invalidate_timer_id);
        }
        memset(&csvt_calls[csvt_call_object_id],
               NIL,
               sizeof(csvt_calls[csvt_call_object_id]));
        UTIL_LOG_MSG("csvt call object id invalidated %d",
                     csvt_call_object_id);
    }
}

