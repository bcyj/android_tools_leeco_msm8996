/***************************************************************************************************
    @file
    core_queue_util.h

    @brief
    Supports utility functions primarily for performing find operations on core queue.
    HLOS interface, Flow control and CRI can use these functions to check If a event with desired
    information is present in the core queue.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CORE_QUEUE_UTIL
#define CORE_QUEUE_UTIL

#include "utils_common.h"
#include "cri_core.h"





/***************************************************************************************************
    @function
    core_queue_util_is_event_present_with_hlos_token_id

    @brief
    Checks whether a HLOS request is present in the core queue with a specific token id.

    @param[in]
        hlos_token_id
            token id that needs to be used for checking a corresponding HLOS request is present
            or not

    @param[out]
        none

    @retval
    TRUE if the HLOS request with hlos_token_id is present in core queue, FALSE otherwise
***************************************************************************************************/
int core_queue_util_is_event_present_with_hlos_token_id(cri_core_hlos_token_id_type hlos_token_id);


#endif
