/***************************************************************************************************
    @file
    util_timer.c

    @brief
    Implements functions supported in util_timer.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "util_timer.h"
#include "core_handler.h"
#include "util_log.h"

#define UTIL_TIMER_GREATER (1)
#define UTIL_TIMER_LESSER (-1)
#define UTIL_TIMER_EQUAL (0)

typedef struct util_timer_data_type
{
    int timer_id;
    struct timeval event_timeout_val;
    timer_event_data_type *timer_event_data;
}util_timer_data_type;

static pthread_t timer_thread;
static util_list_info_type* timer_queue;
static int timer_read_fd;
#ifdef QMI_RIL_UTF
int timer_write_fd;
#else
static int timer_write_fd;
#endif






/***************************************************************************************************
    @function
    util_timer_add_evaluator

    @brief
    Calculates the position in the timer queue where the event can be inserted.

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
static int util_timer_add_evaluator(util_list_node_data_type *to_be_added_data,
                                    util_list_node_data_type *to_be_evaluated_data);





/***************************************************************************************************
    @function
    util_timer_delete_evaluator

    @brief
    Frees the memory of to_be_deleted_data when it is being removed from the timer queue.

    @param[in]
        to_be_deleted_data
            data of the event that is being removed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
static void util_timer_delete_evaluator(util_list_node_data_type *to_be_deleted_data);





/***************************************************************************************************
    @function
    util_timer_thread_proc

    @brief
    Start routine for the timer thread.

    @param[in]
        util_timer_thread_proc_param
            data needed for the start routine of timer thread

    @param[out]
        none

    @retval
    NULL
***************************************************************************************************/
static void* util_timer_thread_proc(void* util_timer_thread_proc_param);





/***************************************************************************************************
    @function
    util_timer_enumerate_evaluator

    @brief
    Traverses the timer queue to log all active timers that are currently present
    in the timer queue.

    @param[in]
        to_be_evaluated_data
            data of the event that is being enumerated

    @param[out]
        none

    @retval
    FALSE always since we do not want the event to be removed after enumerating it
***************************************************************************************************/
static int util_timer_enumerate_evaluator(util_list_node_data_type *to_be_evaluated_data);







/***************************************************************************************************
    @function
    util_timer_start

    @implementation detail
    Initializes a sorted linked list.
***************************************************************************************************/
int util_timer_start()
{
    int err_code;

    err_code = ENOMEM;

    timer_queue = util_list_create(NULL,
                                   util_timer_add_evaluator,
                                   util_timer_delete_evaluator,
                                   UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP |
                                   UTIL_LIST_BIT_FIELD_SORTED);

    if(timer_queue)
    {
#ifdef QMI_RIL_UTF
        err_code = utf_pthread_create_handler(&timer_thread,
                                              NULL,
                                              util_timer_thread_proc,
                                              NULL);
#else
        err_code = pthread_create(&timer_thread,
                                  NULL,
                                  util_timer_thread_proc,
                                  NULL);
#endif
        if(err_code)
        {
            util_list_cleanup(timer_queue,
                              NULL);
        }
    }

    UTIL_LOG_MSG("error %d",
                 err_code);

    return err_code;
}

/***************************************************************************************************
    @function
    util_timer_thread_proc

    @implementation detail
    A pipe is created for other threads to communicate with the timer thread.
    Timer queue is sorted in ascending order.
    Timer thread waits on a select() with the lowest timeout (which would correspond to the
    1st timer in the timer queue) and the pipe descriptors.
    If there are no active timers, select blocks indefinitely on the pipe descriptors.

    Once a timer expires, timer thread waits on the next smallest timeout.

    Threads which need to add or cancel timer write to the pipe.
***************************************************************************************************/
void* util_timer_thread_proc(void* util_timer_thread_proc_param)
{
    util_list_node_data_type *node_data;
    util_timer_data_type *timer_data;
    struct timeval *select_timeout_ptr;
    struct timeval select_timeout;
    struct timeval current_time;
    int file_des[2];
    fd_set read_fd_set;
    fd_set temp_read_fd_set;
    int ret_code;
    char temp_buff[16];

    node_data = NULL;
    timer_data = NULL;
    select_timeout_ptr = NULL;
    memset(&select_timeout,
           0,
           sizeof(select_timeout));
    memset(&current_time,
           0,
           sizeof(current_time));
    memset(&file_des,
           0,
           sizeof(file_des));
    FD_ZERO(&read_fd_set);
    ret_code = NIL;
    memset(&temp_buff,
           0,
           sizeof(temp_buff));



    if(pipe(file_des) < 0)
    {
        UTIL_LOG_MSG("error creating pipe - timer service not setup");
    }
    else
    {
        timer_read_fd = file_des[0];
        timer_write_fd = file_des[1];
        fcntl(timer_read_fd,
              F_SETFL,
              O_NONBLOCK);
        FD_SET(timer_read_fd,
               &read_fd_set);

        while(1)
        {
            memcpy(&temp_read_fd_set,
                   &read_fd_set,
                   sizeof(fd_set));
            select(timer_read_fd + 1,
                   &temp_read_fd_set,
                   NULL,
                   NULL,
                   select_timeout_ptr);
            do
            {
              ret_code = read(timer_read_fd,
                              &temp_buff,
                              sizeof(temp_buff));
#ifdef QMI_RIL_UTF
              if ( ret_code == 6 )
              {
                if (strncmp((char*)temp_buff, "reset", 6) == 0)
                {
                  close(file_des[0]);
                  close(file_des[1]);
                  pthread_exit(NULL);
                }
              }
#endif
            } while (ret_code > 0 || (ret_code < 0 && errno == EINTR));


            util_list_lock_list(timer_queue);
            util_timer_get_current_time(&current_time);
            UTIL_LOG_MSG("current time (%ld sec, %ld usec)",
                         current_time.tv_sec,
                         current_time.tv_usec);
            util_list_enumerate(timer_queue,
                                util_timer_enumerate_evaluator);
            node_data = util_list_retrieve_head(timer_queue);
            if(!node_data)
            {
                UTIL_LOG_MSG("no active timers");
            }

            while(node_data)
            {
                if(node_data->user_data)
                {
                    timer_data = (util_timer_data_type*) node_data->user_data;
                    util_timer_get_current_time(&current_time);
                    if(UTIL_TIMER_GREATER ==
                       util_timer_compare_times(&current_time,
                                                &timer_data->event_timeout_val))
                    {
                        UTIL_LOG_MSG("timer id %d expired, adding to core queue",
                                     timer_data->timer_id);
                        core_handler_add_event(CORE_HANDLER_TIMER_EVENT,
                                               timer_data->timer_event_data);
                        util_list_delete(timer_queue,
                                         node_data,
                                         NULL);
                        node_data = util_list_retrieve_head(timer_queue);
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    UTIL_LOG_MSG("unexpected : user_data is NULL");
                    break;
                }
            }

            node_data = util_list_retrieve_head(timer_queue);
            if(node_data && node_data->user_data)
            {
                timer_data = (util_timer_data_type*) node_data->user_data;
                util_timer_get_current_time(&current_time);
                if(UTIL_TIMER_GREATER == util_timer_compare_times(&timer_data->event_timeout_val,
                                                                  &current_time))
                {
                    util_timer_sub_times(&timer_data->event_timeout_val,
                                         &current_time,
                                         &select_timeout);
                }
                else
                {
                    memset(&select_timeout,
                           0,
                           sizeof(select_timeout));
                }
                UTIL_LOG_MSG("next timer to expire in (%ld sec, %ld usec)",
                             select_timeout.tv_sec,
                             select_timeout.tv_usec);
                select_timeout_ptr = &select_timeout;
            }
            else
            {
                select_timeout_ptr = NULL;
            }
            util_list_unlock_list(timer_queue);
        }
    }

    return NULL;
}

/***************************************************************************************************
    @function
    util_timer_enumerate_evaluator

    @implementation detail
    None.
***************************************************************************************************/
int util_timer_enumerate_evaluator(util_list_node_data_type *to_be_evaluated_data)
{
    util_timer_data_type *timer_data;

    timer_data = NULL;

    if(to_be_evaluated_data && to_be_evaluated_data->user_data)
    {
        timer_data = (util_timer_data_type*) to_be_evaluated_data->user_data;
        UTIL_LOG_MSG("timer id %d, expiry time (%ld sec, %ld usec), event data %p",
                     timer_data->timer_id,
                     timer_data->event_timeout_val.tv_sec,
                     timer_data->event_timeout_val.tv_usec,
                     timer_data->timer_event_data);
    }

    return FALSE;
}

/***************************************************************************************************
    @function
    util_timer_add_evaluator

    @implementation detail
    None.
***************************************************************************************************/
int util_timer_add_evaluator(util_list_node_data_type *to_be_added_data,
                             util_list_node_data_type *to_be_evaluated_data)
{
    int ret_code;
    struct timeval *to_be_added_timeout_val;
    struct timeval *to_be_evaluated_timeout_val;
    util_timer_data_type *util_timer_data;

    ret_code = FALSE;
    util_timer_data = NULL;

    if(to_be_added_data && to_be_added_data->user_data &&
       to_be_evaluated_data && to_be_evaluated_data->user_data)
    {
        util_timer_data = ((util_timer_data_type*)to_be_added_data->user_data);
        to_be_added_timeout_val = &(util_timer_data->event_timeout_val);
        to_be_evaluated_timeout_val = &(util_timer_data->event_timeout_val);
        if(UTIL_TIMER_GREATER == util_timer_compare_times(to_be_evaluated_timeout_val,
                                                          to_be_added_timeout_val))
        {
            ret_code = TRUE;
        }
    }

    return ret_code;
}

/***************************************************************************************************
    @function
    util_timer_delete_evaluator

    @implementation detail
    None.
***************************************************************************************************/
void util_timer_delete_evaluator(util_list_node_data_type *to_be_deleted_data)
{
    if(to_be_deleted_data && to_be_deleted_data->user_data)
    {
        util_memory_free(&to_be_deleted_data->user_data);
    }
}

/***************************************************************************************************
    @function
    util_timer_add

    @implementation detail
    Threads which need to add timer write to the pipe.
***************************************************************************************************/
int util_timer_add(struct timeval *timeout,
                   timer_expiry_cb_type timer_expiry_cb,
                   void *timer_expiry_cb_data,
                   size_t timer_expiry_cb_data_len)
{
    int ret_code;
    int ret_write;
    int temp_timer_id;
    int iter_timer_id;
    core_handler_data_type *core_handler_data;
    util_timer_data_type *timer_data;
    timer_event_data_type *timer_event_data;
    util_list_node_data_type *temp_node_data;
    util_timer_data_type *temp_timer_data;
    int is_timer_id_already_allocated;
    struct timeval current_time;

    ret_code = ENOMEM;
    ret_write = NIL;
    temp_timer_id = NIL;
    iter_timer_id = 1;
    core_handler_data = NULL;
    timer_data = NULL;
    timer_event_data = NULL;
    temp_node_data = NULL;
    temp_timer_data = NULL;
    is_timer_id_already_allocated = FALSE;
    memset(&current_time,
           0,
           sizeof(current_time));

    if(timer_expiry_cb)
    {
        timer_event_data = util_memory_alloc(sizeof(*timer_event_data));
        if(timer_event_data)
        {
            timer_event_data->timer_event_category = TIMER_EXPIRY;
            timer_event_data->data = timer_expiry_cb_data;
            timer_event_data->data_len = timer_expiry_cb_data_len;
            timer_event_data->timer_expiry_cb = timer_expiry_cb;

            timer_data = util_memory_alloc(sizeof(*timer_data));
            if(timer_data)
            {
                timer_data->timer_event_data = timer_event_data;
                util_timer_get_current_time(&current_time);
                if(timeout)
                {
                    util_timer_add_times(&current_time,
                                         timeout,
                                         &timer_data->event_timeout_val);
                }
                else
                {
                    timer_data->event_timeout_val = current_time;
                }

                util_list_lock_list(timer_queue);
                for(iter_timer_id = 1; (NIL == temp_timer_id); iter_timer_id++)
                {
                    is_timer_id_already_allocated = FALSE;
                    temp_node_data = util_list_retrieve_head(timer_queue);
                    while(temp_node_data)
                    {
                        if(temp_node_data->user_data)
                        {
                            temp_timer_data = (util_timer_data_type*) temp_node_data->user_data;
                            if(temp_timer_data->timer_id == iter_timer_id)
                            {
                                is_timer_id_already_allocated = TRUE;
                                break;
                            }
                            else
                            {
                                temp_node_data = util_list_retrieve_successor(timer_queue,
                                                                              temp_node_data);
                            }
                        }
                    }
                    if(FALSE == is_timer_id_already_allocated)
                    {
                        temp_timer_id = iter_timer_id;
                    }
                }
                timer_data->timer_id = temp_timer_id;


                ret_code = util_list_add(timer_queue,
                                         timer_data,
                                         NULL,
                                         NIL);
                if(ret_code)
                {
                    temp_timer_id = NIL;
                    util_memory_free((void**) &timer_event_data);
                    util_memory_free((void**) &timer_data);
                }
                else
                {
                    do
                    {
                      ret_write = write (timer_write_fd,
                                         " ",
                                         1);
                    } while (ret_write < 0 && errno == EINTR);
                }
                util_list_unlock_list(timer_queue);
            }
            else
            {
                util_memory_free((void**) &timer_event_data);
            }
        }
    }
    else
    {
        UTIL_LOG_MSG("Need to provide cb in case of timer expiry");
    }

    if(temp_timer_id)
    {
        if(timeout)
        {
            UTIL_LOG_MSG("timer created, timeout (%ld sec, %ld usec), timer id %d",
                         timeout->tv_sec,
                         timeout->tv_usec,
                         temp_timer_id);
        }
        else
        {
            UTIL_LOG_MSG("timer created, instant timeout, timer id %d",
                         temp_timer_id);
        }
    }
    else
    {
        UTIL_LOG_MSG("timer creation failure");
    }

    return temp_timer_id;
}

/***************************************************************************************************
    @function
    util_timer_cancel

    @implementation detail
    Threads which need to cancel timer write to the pipe.
***************************************************************************************************/
void* util_timer_cancel(int timer_id)
{
    util_list_node_data_type *temp_node_data;
    util_timer_data_type *temp_timer_data;
    void *timer_expiry_cb_data;
    int is_util_timer_cancelled;
    int ret_write;
    int is_head;

    temp_node_data = NULL;
    temp_timer_data = NULL;
    timer_expiry_cb_data = NULL;
    is_util_timer_cancelled = FALSE;
    ret_write = NIL;
    is_head = TRUE;

    util_list_lock_list(timer_queue);
    temp_node_data = util_list_retrieve_head(timer_queue);
    while(temp_node_data)
    {
        if(temp_node_data->user_data)
        {
            temp_timer_data = (util_timer_data_type*) temp_node_data->user_data;
            if(temp_timer_data->timer_id == timer_id)
            {
                timer_expiry_cb_data = temp_timer_data->timer_event_data->data;
                util_memory_free((void**) &(temp_timer_data->timer_event_data));
                util_list_delete(timer_queue,
                                 temp_node_data,
                                 NULL);
                is_util_timer_cancelled = TRUE;

                if(is_head)
                {
                    do
                    {
                      ret_write = write (timer_write_fd,
                                         " ",
                                         1);
                    } while (ret_write < 0 && errno == EINTR);
                }
                break;
            }
            else
            {
                is_head = FALSE;
                temp_node_data = util_list_retrieve_successor(timer_queue,
                                                              temp_node_data);
            }
        }
    }
    util_list_unlock_list(timer_queue);

    if(FALSE == is_util_timer_cancelled)
    {
        UTIL_LOG_MSG("no active timer with timer id %d",
                     timer_id);
    }
    else
    {
        UTIL_LOG_MSG("timer cancelled, timer id %d",
                     timer_id);
    }

    return timer_expiry_cb_data;
}

/***************************************************************************************************
    @function
    util_timer_expiry_handler

    @implementation detail
    None.
***************************************************************************************************/
void util_timer_expiry_handler(void *event_data)
{
    timer_event_data_type *timer_event_data;

    timer_event_data = NULL;

    if(event_data)
    {
        timer_event_data =  (timer_event_data_type *) event_data;
        if(timer_event_data->timer_expiry_cb)
        {
            (*(timer_event_data->timer_expiry_cb))(timer_event_data->data,
                                                   timer_event_data->data_len);
        }
        core_handler_remove_event(timer_event_data);
        util_memory_free((void**) &timer_event_data);
    }
}

/***************************************************************************************************
    @function
    util_timer_get_current_time

    @implementation detail
    Uses system time.
***************************************************************************************************/
void util_timer_get_current_time(struct timeval *current_time)
{
    if(current_time)
    {
        gettimeofday(current_time, //TODO: Need to use kernel time whenever available
                     NULL);
    }
}

/***************************************************************************************************
    @function
    util_timer_add_times

    @implementation detail
    None.
***************************************************************************************************/
void util_timer_add_times(const struct timeval *timer_param_1,
                          const struct timeval *timer_param_2,
                          struct timeval *timer_result)
{
    if(timer_param_1 && timer_param_2 && timer_result)
    {
        timer_result->tv_sec = timer_param_1->tv_sec + timer_param_2->tv_sec;
        timer_result->tv_usec = timer_param_1->tv_usec + timer_param_2->tv_usec;
        if(timer_result->tv_usec >= 1000000)
        {
            timer_result->tv_sec++;
            timer_result->tv_usec -= 1000000;
        }
    }
}

/***************************************************************************************************
    @function
    util_timer_sub_times

    @implementation detail
    None.
***************************************************************************************************/
void util_timer_sub_times(const struct timeval *timer_param_1,
                          const struct timeval *timer_param_2,
                          struct timeval *timer_result)
{
    if(timer_param_1 && timer_param_2 && timer_result)
    {
        timer_result->tv_sec = timer_param_1->tv_sec - timer_param_2->tv_sec;
        timer_result->tv_usec = timer_param_1->tv_usec - timer_param_2->tv_usec;
        if(timer_result->tv_usec < 0)
        {
            timer_result->tv_sec--;
            timer_result->tv_usec += 1000000;
        }
    }
}

/***************************************************************************************************
    @function
    util_timer_compare_times

    @implementation detail
    None.
***************************************************************************************************/
int util_timer_compare_times(const struct timeval *timer_param_1,
                             const struct timeval *timer_param_2)
{
    int ret_code;

    ret_code = 0;

    if(timer_param_1 && timer_param_2)
    {
        if(timer_param_1->tv_sec == timer_param_2->tv_sec)
        {
            if(timer_param_1->tv_usec == timer_param_2->tv_usec)
            {
                ret_code = UTIL_TIMER_EQUAL;
            }
            else if(timer_param_1->tv_usec > timer_param_2->tv_usec)
            {
                ret_code = UTIL_TIMER_GREATER;
            }
            else
            {
                ret_code = UTIL_TIMER_LESSER;
            }
        }
        else if(timer_param_1->tv_sec > timer_param_2->tv_sec)
        {
            ret_code = UTIL_TIMER_GREATER;
        }
        else
        {
            ret_code = UTIL_TIMER_LESSER;
        }
    }

    return ret_code;
}


