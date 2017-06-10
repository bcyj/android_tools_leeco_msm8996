/***************************************************************************************************
    @file
    cri_event_lookup.c

    @brief
    Implements functions supported in cri_event_lookup.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "cri_event_lookup.h"
#include "core_event_lookup.h"
#include "cri_core.h"

core_event_lookup_map_type cri_message_map[] =
{
    {RESP, cri_core_async_resp_handler},
    {IND, cri_core_unsol_ind_handler}
    //TODO add QMUXD indications handler for SSR
};

core_event_lookup_map_type cri_control_map[] =
{



};





/***************************************************************************************************
    @function
    cri_event_lookup_message_handler

    @implementation detail
    None.
***************************************************************************************************/
void* cri_event_lookup_message_handler(cri_core_cri_message_data_type *cri_core_cri_message_data)
{
    void *event_handler;
    unsigned long event_id;
    core_event_lookup_map_type *event_map;
    int event_map_len;

    event_handler = NULL;
    event_id = NIL;
    event_map = NULL;
    event_map_len = NIL;

    if(cri_core_cri_message_data)
    {
        event_id = cri_core_cri_message_data->cri_message_category;
        event_map = cri_message_map;
        event_map_len = UTIL_ARR_SIZE(cri_message_map);
        event_handler = core_event_lookup_map_checker(event_id,
                                                      event_map,
                                                      event_map_len);
    }

    return event_handler;
}

/***************************************************************************************************
    @function
    cri_event_lookup_control_handler

    @implementation detail
    None.
***************************************************************************************************/
void* cri_event_lookup_control_handler(control_core_control_event_data_type
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
        event_map = cri_control_map;
        event_map_len = UTIL_ARR_SIZE(cri_control_map);
        event_handler = core_event_lookup_map_checker(event_id,
                                                      event_map,
                                                      event_map_len);
    }

    return event_handler;
}

