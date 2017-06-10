/***************************************************************************************************
    @file
    hlos_core.h

    @brief
    Supports generic functions for supporting operations for a specific HLOS.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef HLOS_CORE
#define HLOS_CORE

#include "utils_common.h"
#include "cri_core.h"

typedef struct hlos_core_hlos_request_data_type
{
    unsigned long event_id;
    void *token_id;
    void *data;
    size_t data_len;
}hlos_core_hlos_request_data_type;

typedef enum hlos_core_response_pipe_category_type
{
    RILD_PIPE = 1,
    IMS_PIPE
}hlos_core_response_pipe_category_type;





/***************************************************************************************************
    @function
    hlos_core_send_response

    @brief
    Sends the response to HLOS for a received request.

    @param[in]
        hlos_core_response_pipe_category
            HLOS pipe that needs to be used for sending response
        cri_core_error
            calculated error code for the response
        hlos_core_hlos_request_data
            HLOS related information used for sending the response
        payload
            payload of the response that needs to be sent
        payload_len
            length of the payload

    @param[out]
        none

    @retval
        none
***************************************************************************************************/
void hlos_core_send_response(hlos_core_response_pipe_category_type hlos_core_response_pipe_category,
                             cri_core_error_type cri_core_error,
                             hlos_core_hlos_request_data_type *hlos_core_hlos_request_data,
                             void *payload,
                             size_t payload_len);




/***************************************************************************************************
    @function
    hlos_core_get_token_id_value

    @brief
    Retrieves the token id value from the token id a specific HLOS.

    @param[in]
        token_id
            HLOS specific token id

    @param[out]
        none

    @retval
        value of the HLOS specific token id
***************************************************************************************************/
cri_core_hlos_token_id_type hlos_core_get_token_id_value(void *token_id);

#endif
