/***************************************************************************************************
    @file
    core_queue_util.c

    @brief
    Implements functions supported in core_queue_util.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "core_handler.h"
#include "core_queue_util.h"
#include "hlos_core.h"




/***************************************************************************************************
    @function
    core_queue_util_is_event_present_with_hlos_token_id

    @implementation detail
    Other state information (such as is processed, is flow control approved are not taken into
    consideration.
***************************************************************************************************/
int core_queue_util_is_event_present_with_hlos_token_id(cri_core_hlos_token_id_type hlos_token_id)
{
    int is_present;
    util_list_node_data_type *temp_node_data;
    core_handler_data_type *core_handler_data;
    hlos_core_hlos_request_data_type *hlos_core_hlos_request_data;
    cri_core_hlos_token_id_type token_value;

    is_present = FALSE;
    temp_node_data = NULL;
    core_handler_data = NULL;
    hlos_core_hlos_request_data = NULL;
    token_value = NIL;

    util_list_lock_list(core_queue);
    temp_node_data = util_list_retrieve_head(core_queue);
    while(temp_node_data)
    {
        if(temp_node_data->user_data)
        {
            core_handler_data = (core_handler_data_type*) temp_node_data->user_data;
            if(CORE_HANDLER_HLOS_REQUEST == core_handler_data->event_category)
            {
                hlos_core_hlos_request_data = (hlos_core_hlos_request_data_type*)
                                                core_handler_data->event_data;
                token_value = hlos_core_get_token_id_value(hlos_core_hlos_request_data->token_id);
                if(hlos_token_id == token_value)
                {
                    is_present = TRUE;
                    break;
                }
            }
            temp_node_data = util_list_retrieve_successor(core_queue,
                                                          temp_node_data);
        }
        else
        {
            UTIL_LOG_MSG("unexpected : user_data is NULL");
        }
    }
    util_list_unlock_list(core_queue);

    UTIL_LOG_MSG("hlos_token_id %u, is present %d",
                 hlos_token_id,
                 is_present);

    return is_present;
}
