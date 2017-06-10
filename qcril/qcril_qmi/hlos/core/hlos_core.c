/***************************************************************************************************
    @file
    hlos_core.c

    @brief
    Implements functions supported in hlos_core.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "hlos_core.h"
#include "core_handler.h"
#include "qcril_qmi_ims_misc.h"
#include "qcril_qmi_ims_socket.h"






/***************************************************************************************************
    @function
    hlos_core_send_response

    @implementation detail
    None.
***************************************************************************************************/
void hlos_core_send_response(hlos_core_response_pipe_category_type hlos_core_response_pipe_category,
                             cri_core_error_type cri_core_error,
                             hlos_core_hlos_request_data_type *hlos_core_hlos_request_data,
                             void *payload,
                             size_t payload_len)
{
    RIL_Errno ril_error;

    UTIL_LOG_MSG("entry");

    ril_error = RIL_E_GENERIC_FAILURE;

    switch(cri_core_error)
    {
        case QMI_ERR_NONE_V01:
            ril_error = RIL_E_SUCCESS;
            break;

        default:
            ril_error = RIL_E_GENERIC_FAILURE;
            break;
    }

    switch(hlos_core_response_pipe_category)
    {
        case IMS_PIPE:
            qcril_qmi_ims_socket_send(hlos_core_hlos_request_data->token_id,
                                      IMS__MSG_TYPE__RESPONSE,
                                      hlos_core_hlos_request_data->event_id,
                                      qcril_qmi_ims_map_ril_error_to_ims_error(ril_error),
                                      payload,
                                      payload_len);
            break;

        case RILD_PIPE:
            //to be implemented
            break;

        default:
            UTIL_LOG_MSG("Invalid pipe being used %d",
                         hlos_core_response_pipe_category);
            break;
    }


    core_handler_remove_event(hlos_core_hlos_request_data);
    util_memory_free((void**) &hlos_core_hlos_request_data);

    UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    hlos_core_get_token_id_value

    @implementation detail
    None.
***************************************************************************************************/
cri_core_hlos_token_id_type hlos_core_get_token_id_value(void *token_id)
{
    cri_core_hlos_token_id_type token_id_value;

    token_id_value = NIL;

    if(token_id)
    {
        token_id_value = (cri_core_hlos_token_id_type) (*((int *) token_id));
    }

    return token_id_value;
}

