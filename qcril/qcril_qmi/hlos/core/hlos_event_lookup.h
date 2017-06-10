/***************************************************************************************************
    @file
    hlos_event_lookup.h

    @brief
    Supports functions for looking up corresponding event handler for a incoming HLOS request
    or HLOS control event.
    The found event handler would then be used by the core handler
    to process the incoming HLOS request or HLOS control event.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef HLOS_EVENT_LOOKUP
#define HLOS_EVENT_LOOKUP

#include "utils_common.h"
#include "core_handler.h"
#include "hlos_core.h"
#include "control_core.h"




/***************************************************************************************************
    @function
    hlos_event_lookup_request_handler

    @brief
    Retrieves the event handler for a incoming HLOS request.

    @param[in]
        hlos_core_hlos_request_data
            information related to the incoming HLOS request

    @param[out]
        none

    @retval
    event handler for the incoming HLOS request If found, NULL otherwise
***************************************************************************************************/
void* hlos_event_lookup_request_handler(hlos_core_hlos_request_data_type
                                        *hlos_core_hlos_request_data);






/***************************************************************************************************
    @function
    hlos_event_lookup_control_handler

    @brief
    Retrieves the event handler for a incoming HLOS control event.

    @param[in]
        control_core_control_event_data
            information related to the incoming HLOS control event

    @param[out]
        none

    @retval
    event handler for the incoming HLOS control event If found, NULL otherwise
***************************************************************************************************/
void* hlos_event_lookup_control_handler(control_core_control_event_data_type
                                        *control_core_control_event_data);

#endif


