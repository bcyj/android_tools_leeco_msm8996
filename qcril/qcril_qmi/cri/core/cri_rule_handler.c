/***************************************************************************************************
    @file
    cri_rule_handler.c

    @brief
    Implements functions supported in cri_rule_handler.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "cri_rule_handler.h"
#include "cri_core.h"
#include "qmi_idl_lib.h"

static util_list_info_type* rule_queue;






/***************************************************************************************************
    @function
    rule_queue_add_evaluator

    @brief
    Calculates the position in the rule queue where the rule can be inserted.

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
static int rule_queue_add_evaluator(util_list_node_data_type *to_be_added_data,
                                    util_list_node_data_type *to_be_evaluated_data);





/***************************************************************************************************
    @function
    rule_queue_delete_evaluator

    @brief
    Frees the memory of to_be_deleted_data when it is being removed from the rule queue.

    @param[in]
        to_be_deleted_data
            data of the event that is being removed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void rule_queue_delete_evaluator(util_list_node_data_type *to_be_deleted_data);





/***************************************************************************************************
    @function
    rule_check_helper

    @brief
    Helper function for cri_rule_handler_rule_check().

    @param[in]
        rule_node_data
            rule queue specific data
        context
            context of the rule being checked
        cri_core_error
            calculated error of the rule being checked
        cri_resp_data
            calculated data of the rule being checked

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void rule_check_helper(util_list_node_data_type *rule_node_data,
                              cri_core_context_type context,
                              cri_core_error_type cri_core_error,
                              void *cri_resp_data);





/***************************************************************************************************
    @function
    rule_timedout_cb

    @brief
    Handles timer expiry of a rule.

    @param[in]
        rule_timedout_cb_data
            timer expiry callback data
        rule_timedout_cb_data_len
            length of the timer expiry callback data

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void rule_timedout_cb(void *rule_timedout_cb_data,
                             size_t rule_timedout_cb_data_len);



/***************************************************************************************************
    @function
    cri_rule_handler_init

    @implementation detail
    Initializes an unsorted linked list.
***************************************************************************************************/
int cri_rule_handler_init()
{
    int err_code;

    err_code = ENOMEM;

    rule_queue = util_list_create(NULL,
                                  rule_queue_add_evaluator,
                                  rule_queue_delete_evaluator,
                                  UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP);

    if(rule_queue)
    {
        err_code = ESUCCESS;
    }

    UTIL_LOG_MSG("error %d",
                 err_code);

    return err_code;
}

/***************************************************************************************************
    @function
    rule_queue_add_evaluator

    @implementation detail
    To be added node and to be evaluate node are not compared with each other since we want
    the rule queue to be just a FIFO queue.
***************************************************************************************************/
int rule_queue_add_evaluator(util_list_node_data_type *to_be_added_data,
                             util_list_node_data_type *to_be_evaluated_data)
{
    return FALSE;
}

/***************************************************************************************************
    @function
    rule_queue_delete_evaluator

    @implementation detail
    None.
***************************************************************************************************/
void rule_queue_delete_evaluator(util_list_node_data_type *to_be_deleted_data)
{
    if(to_be_deleted_data && to_be_deleted_data->user_data)
    {
        util_memory_free(&to_be_deleted_data->user_data);
    }
}

/***************************************************************************************************
    @function
    rule_timedout_cb

    @implementation detail
    None.
***************************************************************************************************/
void rule_timedout_cb(void *rule_timedout_cb_data,
                      size_t rule_timedout_cb_data_len)
{
    if(rule_timedout_cb_data && rule_timedout_cb_data_len)
    {
        cri_rule_handler_rule_check(*((cri_core_context_type*) rule_timedout_cb_data),
                                    QMI_ERR_INJECT_TIMEOUT_V01,
                                    NULL);
    }
}

/***************************************************************************************************
    @function
    cri_rule_handler_rule_add

    @implementation detail
    Creates a timer for the rule being added.
***************************************************************************************************/
int cri_rule_handler_rule_add(cri_rule_handler_rule_info_type *cri_rule_handler_rule_info,
                              struct timeval *rule_timeout)
{
    int ret_code;
    cri_rule_handler_rule_info_type *temp_rule_info;

    ret_code = ENOMEM;
    temp_rule_info = NULL;

    if(cri_rule_handler_rule_info && rule_timeout)
    {
        temp_rule_info = util_memory_alloc(sizeof(*cri_rule_handler_rule_info));
        if(temp_rule_info)
        {
            memcpy(temp_rule_info,
                   cri_rule_handler_rule_info,
                   sizeof(cri_rule_handler_rule_info_type));
            temp_rule_info->core_rule_info.timer_id =
                        util_timer_add(rule_timeout,
                                       rule_timedout_cb,
                                       &temp_rule_info->core_rule_info.context,
                                       sizeof(cri_core_context_type*));
            ret_code = util_list_add(rule_queue,
                                     temp_rule_info,
                                     NULL,
                                     NIL);
            if(ret_code)
            {
                UTIL_LOG_MSG("rule creation failure");
                util_memory_free((void**) &temp_rule_info);
            }
            else
            {
                UTIL_LOG_MSG("rule creation success, %s, timeout %d seconds",
                             cri_core_create_loggable_context(
                                 temp_rule_info->core_rule_info.context
                                 ),
                             rule_timeout->tv_sec);
            }
        }
    }
    else
    {
        UTIL_LOG_MSG("need to provide rule info and timeout - rule creation failure");
    }

    return ret_code;
}

/***************************************************************************************************
    @function
    cri_rule_handler_rule_check

    @implementation detail
    None.
***************************************************************************************************/
void cri_rule_handler_rule_check(cri_core_context_type context,
                                 cri_core_error_type cri_core_error,
                                 void *cri_resp_data)
{
    util_list_node_data_type *temp_node_data;
    util_list_node_data_type *temp_successor_node_data;
    cri_rule_handler_rule_info_type *cri_rule_handler_rule_info;
    int is_util_rule_cancelled;

    temp_node_data = NULL;
    temp_successor_node_data = NULL;
    cri_rule_handler_rule_info = NULL;
    is_util_rule_cancelled = FALSE;

    UTIL_LOG_MSG("entry");

    UTIL_LOG_MSG("rule check, %s, cri_core_error %d",
                 cri_core_create_loggable_context(context),
                 cri_core_error);

    temp_node_data = util_list_retrieve_head(rule_queue);
    while(temp_node_data)
    {
        temp_successor_node_data = util_list_retrieve_successor(rule_queue,
                                                                temp_node_data);
        if(temp_node_data->user_data)
        {
            cri_rule_handler_rule_info = (cri_rule_handler_rule_info_type*)
                                            temp_node_data->user_data;
            if(context == NIL) //received indication
            {
                if((NULL != cri_rule_handler_rule_info->user_rule_info.rule_check_handler) &&
                   (TRUE == ((*(cri_rule_handler_rule_info->user_rule_info.rule_check_handler))
                             (cri_rule_handler_rule_info->user_rule_info.rule_data)))
                  )
                {
                    rule_check_helper(temp_node_data,
                                      context,
                                      cri_core_error,
                                      cri_resp_data);
                }
            }
            else if(context == cri_rule_handler_rule_info->core_rule_info.context)
            {
                if(NULL != cri_rule_handler_rule_info->core_rule_info.qmi_resp_data)
                {
                    if(CRI_ERR_INJECT_TIMEOUT_V01 != cri_core_error)
                    {
                        cri_rule_handler_rule_info->core_rule_info.qmi_resp_data = NULL;
                    }
                    else
                    {
                        util_memory_free(
                            (void**) &cri_rule_handler_rule_info->core_rule_info.qmi_resp_data);
                    }
                }

                if(NULL != cri_rule_handler_rule_info->core_rule_info.qmi_cb_data)
                {
                    if(CRI_ERR_INJECT_TIMEOUT_V01 != cri_core_error)
                    {
                        cri_rule_handler_rule_info->core_rule_info.qmi_cb_data = NULL;
                    }
                    else
                    {
                        util_memory_free(
                            (void**) &cri_rule_handler_rule_info->core_rule_info.qmi_cb_data);
                    }
                }

                if((CRI_ERR_NONE_V01 != cri_core_error) ||
                   (NULL == cri_rule_handler_rule_info->user_rule_info.rule_check_handler) ||
                   (TRUE == ((*(cri_rule_handler_rule_info->user_rule_info.rule_check_handler))
                             (cri_rule_handler_rule_info->user_rule_info.rule_data)))
                            //TODO : pass sub_id
                  )
                {
                    rule_check_helper(temp_node_data,
                                      context,
                                      cri_core_error,
                                      cri_resp_data);
                }
                break;
            }
        }
        temp_node_data = temp_successor_node_data;
    }

    UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    rule_check_helper

    @implementation detail
    None.
***************************************************************************************************/
void rule_check_helper(util_list_node_data_type *rule_node_data,
                       cri_core_context_type context,
                       cri_core_error_type cri_core_error,
                       void *cri_resp_data)
{
    cri_rule_handler_rule_info_type *cri_rule_info;
    void *hlos_cri_resp_data;

    UTIL_LOG_MSG("entry");

    cri_rule_info = NULL;
    hlos_cri_resp_data = NULL;

    if(rule_node_data && rule_node_data->user_data)
    {
        cri_rule_info = (cri_rule_handler_rule_info_type*) rule_node_data->user_data;

        if(CRI_ERR_INJECT_TIMEOUT_V01 != cri_core_error)
        {
            UTIL_LOG_MSG("rule met due to async resp or indication");
            if(cri_rule_info->core_rule_info.timer_id)
            {
                util_timer_cancel(cri_rule_info->core_rule_info.timer_id);
            }
        }
        else
        {
            UTIL_LOG_MSG("rule met due to expired timer");
        }

        if(cri_rule_info->user_rule_info.cri_resp_data_calculator)
        {
            hlos_cri_resp_data = ((*(cri_rule_info->user_rule_info.cri_resp_data_calculator))
                                  (cri_core_error,
                                   cri_rule_info->user_rule_info.cri_resp_util_data));
        }
        else
        {
            hlos_cri_resp_data = cri_resp_data;
        }

        if(cri_rule_info->core_rule_info.hlos_resp_cb)
        {
            (*(cri_rule_info->core_rule_info.hlos_resp_cb))(
                                        cri_rule_info->core_rule_info.context,
                                        cri_core_error,
                                        (void*) cri_rule_info->core_rule_info.hlos_resp_cb_data,
                                        hlos_cri_resp_data);
        }

        if(cri_rule_info->user_rule_info.rule_data && cri_rule_info->user_rule_info.rule_data_free_handler)
        {
            (*(cri_rule_info->user_rule_info.rule_data_free_handler))(
                                        cri_rule_info->user_rule_info.rule_data);
        }

        if(cri_rule_info->user_rule_info.cri_resp_util_data &&
           cri_rule_info->user_rule_info.cri_resp_util_data_free_handler)
        {
            (*(cri_rule_info->user_rule_info.cri_resp_util_data_free_handler))
            (cri_rule_info->user_rule_info.cri_resp_util_data);
        }

        if(hlos_cri_resp_data &&
           cri_rule_info->user_rule_info.cri_resp_data_calculator &&
           cri_rule_info->user_rule_info.cri_resp_data_free_handler)
        {
            (*(cri_rule_info->user_rule_info.cri_resp_data_free_handler))
            (hlos_cri_resp_data);
        }

        util_list_delete(rule_queue,
                         rule_node_data,
                         NULL);
    }

    UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    cri_rule_handler_rule_delete

    @implementation detail
    None.
***************************************************************************************************/
void cri_rule_handler_rule_delete(cri_core_context_type cri_core_context)
{
    util_list_node_data_type *temp_node_data;
    cri_rule_handler_rule_info_type *cri_rule_info;
    int is_util_rule_cancelled;

    UTIL_LOG_MSG("entry");

    temp_node_data = NULL;
    cri_rule_info = NULL;
    is_util_rule_cancelled = FALSE;

    temp_node_data = util_list_retrieve_head(rule_queue);
    while(temp_node_data)
    {
        if(temp_node_data->user_data)
        {
            cri_rule_info = (cri_rule_handler_rule_info_type*)
                                temp_node_data->user_data;
            if(cri_rule_info->core_rule_info.context == cri_core_context)
            {
                if(cri_rule_info->core_rule_info.timer_id)
                {
                    util_timer_cancel(cri_rule_info->core_rule_info.timer_id);
                }

                if(cri_rule_info->core_rule_info.hlos_resp_cb)
                {
                    (*(cri_rule_info->core_rule_info.hlos_resp_cb))(
                                                cri_rule_info->core_rule_info.context,
                                                QMI_ERR_INTERNAL_V01,
                                                (void*)
                                                cri_rule_info->core_rule_info.hlos_resp_cb_data,
                                                NULL);
                }

                if(cri_rule_info->user_rule_info.rule_data_free_handler)
                {
                    (*(cri_rule_info->user_rule_info.rule_data_free_handler))(
                                                cri_rule_info->user_rule_info.rule_data);
                }

                if(cri_rule_info->user_rule_info.cri_resp_util_data &&
                   cri_rule_info->user_rule_info.cri_resp_util_data_free_handler)
                {
                    (*(cri_rule_info->user_rule_info.cri_resp_util_data_free_handler))
                    (cri_rule_info->user_rule_info.cri_resp_util_data);
                }

                util_list_delete(rule_queue,
                                 temp_node_data,
                                 NULL);
                UTIL_LOG_MSG("rule deleted, %s",
                             cri_core_create_loggable_context(cri_core_context));
                is_util_rule_cancelled = TRUE;
                break;
            }
            else
            {
                temp_node_data = util_list_retrieve_successor(rule_queue,
                                                              temp_node_data);
            }
        }
    }

    if(FALSE == is_util_rule_cancelled)
    {
        UTIL_LOG_MSG("no active rule with context %lu",
                     cri_core_context);
    }

     UTIL_LOG_MSG("exit");
}

/***************************************************************************************************
    @function
    cri_rule_handler_rule_delete_all

    @implementation detail
    None.
***************************************************************************************************/
void cri_rule_handler_rule_delete_all()
{
    util_list_node_data_type *temp_node_data;
    util_list_node_data_type *temp_successor_node_data;
    cri_rule_handler_rule_info_type *cri_rule_info;
    int is_util_rule_cancelled;

    UTIL_LOG_MSG("entry");

    temp_node_data = NULL;
    temp_successor_node_data = NULL;
    cri_rule_info = NULL;
    is_util_rule_cancelled = FALSE;

    temp_node_data = util_list_retrieve_head(rule_queue);
    while(temp_node_data)
    {
        temp_successor_node_data = util_list_retrieve_successor(rule_queue,
                                                                temp_node_data);
        if(temp_node_data->user_data)
        {
            cri_rule_info = (cri_rule_handler_rule_info_type*) temp_node_data->user_data;
            if(cri_rule_info->core_rule_info.timer_id)
            {
                util_timer_cancel(cri_rule_info->core_rule_info.timer_id);
            }

            if(cri_rule_info->core_rule_info.hlos_resp_cb)
            {
                (*(cri_rule_info->core_rule_info.hlos_resp_cb))(
                                                 cri_rule_info->core_rule_info.context,
                                                 QMI_ERR_INTERNAL_V01,
                                                 (void*)
                                                 cri_rule_info->core_rule_info.hlos_resp_cb_data,
                                                 NULL);
            }

            if(cri_rule_info->user_rule_info.rule_data_free_handler)
            {
                (*(cri_rule_info->user_rule_info.rule_data_free_handler))(
                                                      cri_rule_info->user_rule_info.rule_data);
            }

            if(cri_rule_info->user_rule_info.cri_resp_util_data &&
               cri_rule_info->user_rule_info.cri_resp_util_data_free_handler)
            {
                (*(cri_rule_info->user_rule_info.cri_resp_util_data_free_handler))
                (cri_rule_info->user_rule_info.cri_resp_util_data);
            }

            util_list_delete(rule_queue,
                             temp_node_data,
                             NULL);
        }
        temp_node_data = temp_successor_node_data;
    }

    UTIL_LOG_MSG("all rules deleted");

    UTIL_LOG_MSG("exit");
}


