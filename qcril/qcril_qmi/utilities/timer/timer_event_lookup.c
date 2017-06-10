/***************************************************************************************************
    @file
    timer_event_lookup.c

    @brief
    Implements functions supported in timer_event_lookup.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "timer_event_lookup.h"
#include "core_event_lookup.h"
#include "util_timer.h"


core_event_lookup_map_type timer_expiry_event_map[] =
{
    {TIMER_EXPIRY, util_timer_expiry_handler}
};





/***************************************************************************************************
    @function
    timer_event_lookup_handler

    @implementation detail
    None.
***************************************************************************************************/
void* timer_event_lookup_handler(timer_event_data_type *timer_event_data)
{
    void *event_handler;
    unsigned long event_id;
    core_event_lookup_map_type *event_map;
    int event_map_len;

    event_handler = NULL;
    event_id = NIL;
    event_map = NULL;
    event_map_len = NIL;

    if(timer_event_data)
    {
        event_id = timer_event_data->timer_event_category;
        event_map = timer_expiry_event_map;
        event_map_len = UTIL_ARR_SIZE(timer_expiry_event_map);
        event_handler = core_event_lookup_map_checker(event_id,
                                                      event_map,
                                                      event_map_len);
    }

    return event_handler;
}


