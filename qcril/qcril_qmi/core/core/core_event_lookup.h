/***************************************************************************************************
    @file
    core_event_lookup.h

    @brief
    Supports functions for core_handler to lookup the corresponding
    event handler for a incoming event.
    The found event handler would then be used by the core handler
    to process the incoming event.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CORE_EVENT_LOOKUP
#define CORE_EVENT_LOOKUP

#include "utils_common.h"
#include "core_handler.h"

typedef struct core_event_lookup_map_type
{
    unsigned long event_id;
    void (*event_handler)(void *event_data);
}core_event_lookup_map_type;






/***************************************************************************************************
    @function
    core_event_lookup_map_checker

    @brief
    Retrieves the event handler from a event_map for a given event_id.

    @param[in]
        event_id
            event_id that needs to be used for retrieving the event handler
        event_map
            event_map that needs to be used for traversing
        event_map_len
            number of entries in the provided event map

    @param[out]
        none

    @retval
    event handler for a given event_id and event_map If found, NULL otherwise
***************************************************************************************************/
void* core_event_lookup_map_checker(unsigned long event_id,
                                    core_event_lookup_map_type *event_map,
                                    int event_map_len);





/***************************************************************************************************
    @function
    core_event_lookup_handler

    @brief
    Retrieves the event handler for a incoming event.

    @param[in]
        core_handler_data
            information related to the incoming event

    @param[out]
        none

    @retval
    event handler for the incoming event If found, NULL otherwise
***************************************************************************************************/
void* core_event_lookup_handler(core_handler_data_type *core_handler_data);





#endif
