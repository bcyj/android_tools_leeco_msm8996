/***************************************************************************************************
    @file
    util_timer.h

    @brief
    Facilitates timer related operations by providing time related utilities.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef UTIL_TIMER
#define UTIL_TIMER

#include "utils_standard.h"

typedef void (*timer_expiry_cb_type)(void *timer_expiry_cb_data,
                                     size_t timer_expiry_cb_data_len);

typedef enum timer_event_category_type
{
    TIMER_EXPIRY = 1,
}timer_event_category_type;

typedef struct timer_event_data_type
{
    timer_event_category_type timer_event_category;
    timer_expiry_cb_type timer_expiry_cb;
    void *data;
    size_t data_len;
}timer_event_data_type;

/***************************************************************************************************
    @function
    util_timer_start

    @brief
    Spawns the timer thread and creates the timer queue to perform operations using timers.

    @param[in]
        none

    @param[out]
        none

    @retval
    ESUCCESS if timer thread and queue have been initialized successfully,
    appropriate error code otherwise
***************************************************************************************************/
int util_timer_start();





/***************************************************************************************************
    @function
    util_timer_add

    @brief
    Creates a timer.

    @param[in]
        timeout
            relative time for the timer to expire
        timer_expiry_cb
            pointer to the function callback that needs to be called when timer expires
        timer_expiry_cb_data
            pointer to data that the timer_expiry_cb needs to be called with
        timer_expiry_cb_data_len
            length of the timer_expiry_cb_data

    @param[out]
        none

    @retval
    timer id (a positive integer) if the timer has been created successfully, NIL otherwise
***************************************************************************************************/
int util_timer_add(struct timeval *timeout,
                   timer_expiry_cb_type timer_expiry_cb,
                   void *timer_expiry_cb_data,
                   size_t timer_expiry_cb_data_len);





/***************************************************************************************************
    @function
    util_timer_cancel

    @brief
    Cancels a active timer.

    @param[in]
        timer_id
            timer id of the timer to be cancelled

    @param[out]
        none

    @retval
    pointer to the timer expiry call back data that was provided while creating the corresponding
    timer
***************************************************************************************************/
void* util_timer_cancel(int timer_id);





/***************************************************************************************************
    @function
    util_timer_expiry_handler

    @brief
    Handles the expired timer.
    Core thread would call this function (when timer expires) which would then further call
    the provided timer expiry callback function.

    @param[in]
        event_data
            pointer to data that comprises of the timer related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_timer_expiry_handler(void *event_data);






/***************************************************************************************************
    @function
    util_timer_get_current_time

    @brief
    Retrieves the current time of the system.

    @param[in]
        none

    @param[out]
        current_time
            pointer to the time value parameter that would be populated with the current time

    @retval
    none
***************************************************************************************************/
void util_timer_get_current_time(struct timeval *current_time);





/***************************************************************************************************
    @function
    util_timer_add_times

    @brief
    Adds the 2 provided time values.

    @param[in]
        timer_param_1
            pointer to the time value parameter that would be used as first addend
        timer_param_2
            pointer to the time value parameter that would be used as second addend

    @param[out]
        timer_result
            pointer to the time value parameter that would be populated with the sum

    @retval
    none
***************************************************************************************************/
void util_timer_add_times(const struct timeval *timer_param_1,
                          const struct timeval *timer_param_2,
                          struct timeval *timer_result);






/***************************************************************************************************
    @function
    util_timer_sub_times

    @brief
    Subtracts the provided time values one from another.

    @param[in]
        timer_param_1
            pointer to the first time value parameter that would be used as minuend
        timer_param_2
            pointer to the second time value parameter that would be used as subtrahend

    @param[out]
        timer_result
            pointer to the time value parameter that would be populated with the difference

    @retval
    none
***************************************************************************************************/
void util_timer_sub_times(const struct timeval *timer_param_1,
                          const struct timeval *timer_param_2,
                          struct timeval *timer_result);





/***************************************************************************************************
    @function
    util_timer_compare_times

    @brief
    Compares the provided time values one with each other.

    @param[in]
        timer_param_1
            pointer to the time value parameter that would be compared against
        timer_param_2
            pointer to the time value parameter that would be compared with

    @param[out]
        none

    @retval
    If timer_param_1 is greater than timer_param_2, then UTIL_TIMER_GREATER
    If timer_param_1 is lesser than timer_param_2, then UTIL_TIMER_LESSER
    If timer_param_1 is equal to timer_param_2, then UTIL_TIMER_EQUAL
***************************************************************************************************/
int util_timer_compare_times(const struct timeval *timer_param_1,
                             const struct timeval *timer_param_2);

#endif

