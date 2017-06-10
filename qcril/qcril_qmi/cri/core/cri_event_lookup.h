/***************************************************************************************************
    @file
    cri_event_lookup.h

    @brief
    Supports functions for looking up corresponding event handler for a incoming QMI message
    or CRI control event.
    The found event handler would then be used by the core handler
    to process the incoming CRI message or CRI control event.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_EVENT_LOOKUP
#define CRI_EVENT_LOOKUP

#include "utils_common.h"
#include "core_handler.h"
#include "cri_core.h"
#include "control_core.h"




/***************************************************************************************************
    @function
    cri_event_lookup_message_handler

    @brief
    Retrieves the event handler for a incoming QMI message event.

    @param[in]
        cri_core_cri_message_data
            information related to the incoming QMI message event

    @param[out]
        none

    @retval
    event handler for the incoming QMI message event If found, NULL otherwise
***************************************************************************************************/
void* cri_event_lookup_message_handler(cri_core_cri_message_data_type *cri_core_cri_message_data);





/***************************************************************************************************
    @function
    cri_event_lookup_control_handler

    @brief
    Retrieves the event handler for a incoming CRI control event.

    @param[in]
        cri_core_cri_message_data
            information related to the incoming CRI control event

    @param[out]
        none

    @retval
    event handler for the incoming CRI control event If found, NULL otherwise
***************************************************************************************************/
void* cri_event_lookup_control_handler(control_core_control_event_data_type
                                       *control_core_control_event_data);

#endif


