/***************************************************************************************************
    @file
    core_handler.c

    @brief
    Implements functions supported in core_handler.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "core_handler.h"
#include "core_event_lookup.h"
#include "hlos_core.h"
#include "cri_core.h"
#include "control_core.h"

static pthread_t core_handler_thread;
util_list_info_type* core_queue;
extern int qmi_shutdown_sync_thread;





/***************************************************************************************************
    @function
    core_queue_add_evaluator

    @brief
    Calculates the position in the core queue where the event can be inserted.

    @param[in]
        to_be_added_data
            data of the event that is being added
        to_be_evaluated_data
            data of the event that is already in the list

    @param[out]
        none

    @retval
    TRUE if to_be_added event can be inserted before to_be_evaluated event, FALSE
    otherwise
***************************************************************************************************/
static int core_queue_add_evaluator(util_list_node_data_type *to_be_added_data,
                                    util_list_node_data_type *to_be_evaluated_data);




/***************************************************************************************************
    @function
    core_queue_delete_evaluator

    @brief
    Frees the memory of to_be_deleted_data when it is being removed from the core queue.

    @param[in]
        to_be_deleted_data
            data of the event that is being removed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void core_queue_delete_evaluator(util_list_node_data_type *to_be_deleted_data);




/***************************************************************************************************
    @function
    core_handler_thread_proc

    @brief
    Start routine for the core thread.

    @param[in]
        core_handler_thread_proc_param
            data needed for the start routine of core thread

    @param[out]
        none

    @retval
    NULL
***************************************************************************************************/
static void* core_handler_thread_proc(void* core_handler_thread_proc_param);




/***************************************************************************************************
    @function
    core_queue_find_for_processing_evaluator

    @brief
    Checks whether an event can be processed or not.

    @param[in]
        to_be_found_data
            data of the event that is being checked for processing

    @param[out]
        none

    @retval
    TRUE If the event can be processed, FALSE otherwise
***************************************************************************************************/
static int core_queue_find_for_processing_evaluator(util_list_node_data_type *to_be_found_data);



/***************************************************************************************************
    @function
    core_queue_enumerate_evaluator

    @brief
    Traverses the core queue to log all events that are currently present in the core queue.

    @param[in]
        to_be_evaluated_data
            data of the event that is being enumerated

    @param[out]
        none

    @retval
    FALSE always since we do not want the event to be removed after enumerating it
***************************************************************************************************/
static int core_queue_enumerate_evaluator(util_list_node_data_type *to_be_evaluated_data);




/***************************************************************************************************
    @function
    core_queue_enumerate_helper

    @brief
    Helper function for core_queue_enumerate_evaluator().

    @param[in]
        core_handler_data
            core queue specific data of the event

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void core_queue_enumerate_helper(core_handler_data_type *core_handler_data);


/***************************************************************************************************
    @function
    core_handler_start

    @implementation detail
    Initializes an unsorted linked list.
***************************************************************************************************/
int core_handler_start()
{
    int err_code;

    err_code = ENOMEM;

    core_queue = util_list_create(NULL,
                                  core_queue_add_evaluator,
                                  core_queue_delete_evaluator,
                                  UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP |
                                  UTIL_LIST_BIT_FIELD_USE_COND_VAR);

    if(core_queue)
    {
        #ifdef QMI_RIL_UTF
        err_code = utf_pthread_create_handler(&core_handler_thread,
                                              NULL,
                                              core_handler_thread_proc,
                                              NULL);

        #else
        err_code = pthread_create(&core_handler_thread,
                                  NULL,
                                  core_handler_thread_proc,
                                  NULL);
        #endif
        if(err_code)
        {
            util_list_cleanup(core_queue,
                              NULL);
        }
    }

    UTIL_LOG_MSG("error %d",
                 err_code);

    return err_code;
}

/***************************************************************************************************
    @function
    core_handler_thread_proc

    @implementation detail
    core_queue_find_for_processing_evaluator dictates whether an event can be processed.
    Sets is_processed flag of an event to TRUE after calling the corresponding event handler.
***************************************************************************************************/
void* core_handler_thread_proc(void* core_handler_thread_proc_param)
{
    util_list_node_data_type *node_data;
    core_handler_data_type *core_handler_data;
    void (*event_handler)(void *event_data);

    node_data = NULL;
    core_handler_data = NULL;

    while(1)
    {
        util_list_lock_list(core_queue);

        core_handler_data = NULL;
        while(NULL == core_handler_data)
        {
            util_list_enumerate(core_queue,
                                core_queue_enumerate_evaluator);

            node_data = util_list_find(core_queue,
                                       core_queue_find_for_processing_evaluator);
            if(NULL == node_data)
            {
                UTIL_LOG_MSG("no unprocessed entries in core queue");
                util_list_wait_on_list(core_queue,
                                       NIL);
            }
            else
            {
                core_handler_data = (core_handler_data_type*) (node_data->user_data);
                if(core_handler_data)
                {
                    core_handler_data->is_processed = TRUE;
                }
                else
                {
                    UTIL_LOG_MSG("unexpected : core_handler_data is NULL");
                }
            }
        }
        util_list_unlock_list(core_queue);

        if(core_handler_data)
        {
            event_handler = core_event_lookup_handler(core_handler_data);
            if(event_handler)
            {
                (*event_handler) (core_handler_data->event_data);
            }
        }
    }

    return NULL;
}

/***************************************************************************************************
    @function
    core_queue_enumerate_evaluator

    @implementation detail
    None.
***************************************************************************************************/
int core_queue_enumerate_evaluator(util_list_node_data_type *to_be_evaluated_data)
{
    core_handler_data_type *core_handler_data;

    core_handler_data = NULL;

    if(to_be_evaluated_data && to_be_evaluated_data->user_data)
    {
        core_handler_data = (core_handler_data_type*) to_be_evaluated_data->user_data;
        if(core_handler_data->event_data)
        {
            core_queue_enumerate_helper(core_handler_data);
        }
        else
        {
            UTIL_LOG_MSG("event data absent for category %d",
                         core_handler_data->event_category);
        }
    }

    return FALSE;
}

/***************************************************************************************************
    @function
    core_queue_enumerate_helper

    @implementation detail
    None.
***************************************************************************************************/
void core_queue_enumerate_helper(core_handler_data_type *core_handler_data)
{
    hlos_core_hlos_request_data_type *hlos_core_hlos_request_data;
    cri_core_cri_message_data_type *cri_core_cri_message_data;
    control_core_control_event_data_type *control_core_control_event_data;
    timer_event_data_type *timer_event_data;

    hlos_core_hlos_request_data = NULL;
    cri_core_cri_message_data = NULL;
    control_core_control_event_data = NULL;
    timer_event_data = NULL;

    switch(core_handler_data->event_category)
    {
        case CORE_HANDLER_HLOS_REQUEST:
            hlos_core_hlos_request_data = (hlos_core_hlos_request_data_type*)
                                           core_handler_data->event_data;
            UTIL_LOG_MSG("is_processed %d hlos request id %d, token id %u, data %p, data len %d",
                         core_handler_data->is_processed,
                         hlos_core_hlos_request_data->event_id,
                         hlos_core_get_token_id_value(hlos_core_hlos_request_data->token_id),
                         hlos_core_hlos_request_data->data,
                         hlos_core_hlos_request_data->data_len);
            break;

        case CORE_HANDLER_HLOS_CONTROL:
            control_core_control_event_data = (control_core_control_event_data_type*)
                                               core_handler_data->event_data;
            UTIL_LOG_MSG("is_processed %d hlos control event id %d, data %p, data len %d",
                         core_handler_data->is_processed,
                         control_core_control_event_data->event_id,
                         control_core_control_event_data->data,
                         control_core_control_event_data->data_len);
            break;

        case CORE_HANDLER_CRI_MESSAGE:
            cri_core_cri_message_data = (cri_core_cri_message_data_type*)
                                         core_handler_data->event_data;
            UTIL_LOG_MSG("is_processed %d cri message category %d, message id %d, user handle %p, "
                         "data %p, data len %d, cb data %p, transp err %d",
                         core_handler_data->is_processed,
                         cri_core_cri_message_data->cri_message_category,
                         cri_core_cri_message_data->event_id,
                         cri_core_cri_message_data->user_handle,
                         cri_core_cri_message_data->data,
                         cri_core_cri_message_data->data_len,
                         cri_core_cri_message_data->cb_data,
                         cri_core_cri_message_data->transport_error);
            break;

        case CORE_HANDLER_CRI_CONTROL:
            control_core_control_event_data = (control_core_control_event_data_type*)
                                               core_handler_data->event_data;
            UTIL_LOG_MSG("is_processed %d cri control event id %d, data %p, data len %d",
                         core_handler_data->is_processed,
                         control_core_control_event_data->event_id,
                         control_core_control_event_data->data,
                         control_core_control_event_data->data_len);
            break;

        case CORE_HANDLER_TIMER_EVENT:
            timer_event_data = (timer_event_data_type*)
                                core_handler_data->event_data;
            UTIL_LOG_MSG("is_processed %d timer event category id %d, cb %p, data %p, data len %d",
                         core_handler_data->is_processed,
                         timer_event_data->timer_event_category,
                         timer_event_data->timer_expiry_cb,
                         timer_event_data->data,
                         timer_event_data->data_len);
            break;

        default:
            UTIL_LOG_MSG("unknown category %d",
                         core_handler_data->event_category);
            break;
    }
}

/***************************************************************************************************
    @function
    core_queue_add_evaluator

    @implementation detail
    To be added node and to be evaluate node are not compared with each other since we want
    the core queue to be just a FIFO queue.
***************************************************************************************************/
int core_queue_add_evaluator(util_list_node_data_type *to_be_added_data,
                             util_list_node_data_type *to_be_evaluated_data)
{
    return FALSE;
}

/***************************************************************************************************
    @function
    core_queue_add_evaluator

    @implementation detail
    None.
***************************************************************************************************/
void core_queue_delete_evaluator(util_list_node_data_type *to_be_deleted_data)
{
    if(to_be_deleted_data && to_be_deleted_data->user_data)
    {
        util_memory_free(&to_be_deleted_data->user_data);
    }
}

/***************************************************************************************************
    @function
    core_queue_find_for_processing_evaluator

    @implementation detail
    is_processed is only checked to decide whether an event has been processed or not.
    Additional criteria can be added (such as has_been_flow_control_approved...) in the future.
***************************************************************************************************/
int core_queue_find_for_processing_evaluator(util_list_node_data_type *to_be_found_data)
{
    int ret;
    core_handler_data_type *core_handler_data;

    ret = FALSE;
    core_handler_data = NULL;

    if(to_be_found_data && to_be_found_data->user_data)
    {
        core_handler_data = (core_handler_data_type*) to_be_found_data->user_data;
        if(FALSE == core_handler_data->is_processed)
        {
            ret = TRUE;
        }
    }

    return ret;
}

/***************************************************************************************************
    @function
    core_handler_add_event

    @implementation detail
    This function can be called in the context of any thread. Event is added to the queue and
    core thread is woken up through a signal.
***************************************************************************************************/
int core_handler_add_event(core_handler_event_category_type event_category,
                           void *event_data)
{
    int ret_code;
    core_handler_data_type *core_handler_data;

    ret_code = ENOMEM;
    core_handler_data = NULL;

    core_handler_data = util_memory_alloc(sizeof(*core_handler_data));
    if(core_handler_data)
    {
        core_handler_data->is_processed = FALSE;
        core_handler_data->event_category = event_category;
        core_handler_data->event_data = event_data;

        util_list_lock_list(core_queue);
        ret_code = util_list_add(core_queue,
                                 core_handler_data,
                                 NULL,
                                 NIL);
        if(ret_code)
        {
            util_memory_free((void**) &core_handler_data);
        }
        else
        {
            util_list_signal_for_list(core_queue);
        }
        util_list_unlock_list(core_queue);
    }

    UTIL_LOG_MSG("error %d for adding category %d, event data %p",
                 ret_code,
                 event_category,
                 event_data);

    return ret_code;
}

/***************************************************************************************************
    @function
    core_handler_remove_event

    @implementation detail
    event_data_to_be_removed is not freed as part of this function.
***************************************************************************************************/
int core_handler_remove_event(void *event_data_to_be_removed)
{
    int is_match;
    int ret_code;
    util_list_node_data_type *temp_node_data;
    core_handler_data_type *core_handler_data;

    ret_code = EINVAL;
    is_match = FALSE;
    temp_node_data = NULL;
    core_handler_data = NULL;

    util_list_lock_list(core_queue);
    temp_node_data = util_list_retrieve_head(core_queue);
    while(FALSE == is_match && temp_node_data)
    {
        if(temp_node_data->user_data)
        {
            core_handler_data = (core_handler_data_type*) temp_node_data->user_data;
            if(event_data_to_be_removed == core_handler_data->event_data)
            {
                is_match = TRUE;
                util_list_delete(core_queue,
                                 temp_node_data,
                                 NULL);
                util_list_signal_for_list(core_queue);
            }
            else
            {
                temp_node_data = util_list_retrieve_successor(core_queue,
                                                              temp_node_data);
            }
        }
        else
        {
            UTIL_LOG_MSG("unexpected : user_data is NULL");
        }
    }
    util_list_unlock_list(core_queue);

    if(TRUE == is_match)
    {
        ret_code = ESUCCESS;
    }

    UTIL_LOG_MSG("error %d for removing event data %p",
                 ret_code,
                 event_data_to_be_removed);

    return ret_code;
}

/***************************************************************************************************
    @function
    core_shutdown_for_reboot

    @implementation detail
    none
***************************************************************************************************/
int core_shutdown_for_reboot()
{
  // request shutdown of sync thread
  qmi_shutdown_sync_thread = 1;
  pthread_cond_signal(&core_queue->list_sync_data.sync_cond);

  return 0;
}
