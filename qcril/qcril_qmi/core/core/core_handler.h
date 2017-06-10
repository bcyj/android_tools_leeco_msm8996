/***************************************************************************************************
    @file
    core_handler.h

    @brief
    Supports functions for performing operations on/using core queue.
    Primary use would be to add and remove incoming events to the
    core queue. The events once added would be processed by the core thread.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CORE_HANDLER
#define CORE_HANDLER

#include "utils_common.h"
#include "core_queue_util.h"
#include "qcril_qmi_client.h"

typedef enum core_handler_event_category_type
{
    CORE_HANDLER_HLOS_REQUEST = 1,
    CORE_HANDLER_HLOS_CONTROL,
    CORE_HANDLER_CRI_MESSAGE,
    CORE_HANDLER_CRI_CONTROL,
    CORE_HANDLER_TIMER_EVENT
}core_handler_event_category_type;

typedef struct core_handler_data_type
{
    int is_processed;
    core_handler_event_category_type event_category;
    void *event_data;
}core_handler_data_type;

/*
core queue is exported to be used by other core features and utilities
Ex: Flow control
*/
extern util_list_info_type* core_queue;






/***************************************************************************************************
    @function
    core_handler_start

    @brief
    Spawns the core thread and creates the core queue to facilitate the
    processing of the incoming events.

    @param[in]
        none

    @param[out]
        none

    @retval
    ESUCCESS if core thread and queue have been initialized successfully,
    appropriate error code otherwise
***************************************************************************************************/
int core_handler_start();





/***************************************************************************************************
    @function
    core_handler_add_event

    @brief
    Adds an event to the core queue.

    @param[in]
        event_category
            category of the event that is being added
        event_data
            data of the event that is being added

    @param[out]
        none

    @retval
    ESUCCESS if event has been added successfully, appropriate error code
    otherwise
***************************************************************************************************/
int core_handler_add_event(core_handler_event_category_type event_category,
                           void *event_data);




/***************************************************************************************************
    @function
    core_handler_remove_event

    @brief
    Removes an event from the core queue.

    @param[in]
        event_data_to_be_removed
            data of the event that is being removed
            event_data_to_be_removed is NOT freed as part of this function

    @param[out]
        none

    @retval
    ESUCCESS if event has been removed successfully, appropriate error code
    otherwise
***************************************************************************************************/
int core_handler_remove_event(void *event_data_to_be_removed);

int core_shutdown_for_reboot();

#endif
