/***************************************************************************************************
    @file
    hlos_event_lookup.c

    @brief
    Implements functions supported in hlos_event_lookup.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "hlos_event_lookup.h"
#include "core_event_lookup.h"
#include "hlos_csvt_core.h"

core_event_lookup_map_type hlos_request_map[] =
{
    {IMS__MSG_ID__REQUEST_DIAL, hlos_csvt_dial_request_handler},
    {IMS__MSG_ID__REQUEST_HANGUP, hlos_csvt_hangup_request_handler},
    {IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE, hlos_csvt_last_call_failure_cause_request_handler},
    {IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS, hlos_csvt_get_current_calls_request_handler},
    {IMS__MSG_ID__REQUEST_ANSWER, hlos_csvt_answer_request_handler}
};

core_event_lookup_map_type hlos_control_map[] =
{



};





/***************************************************************************************************
    @function
    hlos_event_lookup_request_handler

    @implementation detail
    None.
***************************************************************************************************/
void* hlos_event_lookup_request_handler(hlos_core_hlos_request_data_type
                                        *hlos_core_hlos_request_data)
{
    void *event_handler;
    unsigned long event_id;
    core_event_lookup_map_type *event_map;
    int event_map_len;

    event_handler = NULL;
    event_id = NIL;
    event_map = NULL;
    event_map_len = NIL;

    if(hlos_core_hlos_request_data)
    {
        event_id = hlos_core_hlos_request_data->event_id;
        event_map = hlos_request_map;
        event_map_len = UTIL_ARR_SIZE(hlos_request_map);
        event_handler = core_event_lookup_map_checker(event_id,
                                                      event_map,
                                                      event_map_len);
    }

    return event_handler;
}

/***************************************************************************************************
    @function
    hlos_event_lookup_control_handler

    @implementation detail
    None.
***************************************************************************************************/
void* hlos_event_lookup_control_handler(control_core_control_event_data_type
                                        *control_core_control_event_data)
{
    void *event_handler;
    unsigned long event_id;
    core_event_lookup_map_type *event_map;
    int event_map_len;

    event_handler = NULL;
    event_id = NIL;
    event_map = NULL;
    event_map_len = NIL;

    if(control_core_control_event_data)
    {
        event_id = control_core_control_event_data->event_id;
        event_map = hlos_control_map;
        event_map_len = UTIL_ARR_SIZE(hlos_control_map);
        event_handler = core_event_lookup_map_checker(event_id,
                                                      event_map,
                                                      event_map_len);
    }

    return event_handler;
}

