/***************************************************************************************************
    @file
    util_synchronization.c

    @brief
    Implements functions supported in util_synchronization.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "util_synchronization.h"
#include "util_log.h"

int qmi_shutdown_sync_thread = 0;

static const char* util_sync_bit_field_names[] =
{
    "COND_VAR_USED",
};






/***************************************************************************************************
    @function
    util_sync_data_init

    @implementation detail
    None.
***************************************************************************************************/
void util_sync_data_init(util_sync_data_type *sync_data, util_bit_field_type sync_data_bit_field)
{
    if(sync_data)
    {
        sync_data->sync_data_bit_field = sync_data_bit_field;
        pthread_mutex_init(&sync_data->sync_mutex,
                           NULL);
        if(util_bit_field_is_bits_set(sync_data_bit_field,
                                      UTIL_SYNC_BIT_FIELD_COND_VAR_USED,
                                      FALSE))
        {
            pthread_cond_init(&sync_data->sync_cond,
                              NULL);
        }
    }
}

/***************************************************************************************************
    @function
    util_sync_data_destroy

    @implementation detail
    Sync object's bit field is reset.
***************************************************************************************************/
void util_sync_data_destroy(util_sync_data_type *sync_data)
{
    if(sync_data)
    {
        pthread_mutex_destroy(&sync_data->sync_mutex);
        if(util_bit_field_is_bits_set(sync_data->sync_data_bit_field,
                                      UTIL_SYNC_BIT_FIELD_COND_VAR_USED,
                                      FALSE))
        {
            pthread_cond_destroy(&sync_data->sync_cond);
        }
        sync_data->sync_data_bit_field = NIL;
    }
}

/***************************************************************************************************
    @function
    util_sync_data_acquire_mutex

    @implementation detail
    None.
***************************************************************************************************/
int util_sync_data_acquire_mutex(util_sync_data_type *sync_data)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
        ret = pthread_mutex_lock(&sync_data->sync_mutex);
    }

    return ret;
}

/***************************************************************************************************
    @function
    util_sync_data_release_mutex

    @implementation detail
    None.
***************************************************************************************************/
int util_sync_data_release_mutex(util_sync_data_type *sync_data)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
        ret = pthread_mutex_unlock(&sync_data->sync_mutex);
    }

    return ret;
}

/***************************************************************************************************
    @function
    util_sync_data_wait_on_cond

    @implementation detail
    get_time_of_day is used to provide the absolute time to timed wait.
***************************************************************************************************/
int util_sync_data_wait_on_cond(util_sync_data_type *sync_data,
                                int wait_for_time_seconds)
{
    int ret;
    struct timespec sync_wait_time;
    struct timeval temp_tp;

    ret = ESUCCESS;
    memset(&sync_wait_time,
           0,
           sizeof(sync_wait_time));
    memset(&temp_tp,
           0,
           sizeof(temp_tp));

    if(sync_data)
    {
        if(wait_for_time_seconds)
        {
            gettimeofday(&temp_tp, NULL);
            sync_wait_time.tv_sec  = temp_tp.tv_sec;
            sync_wait_time.tv_nsec = temp_tp.tv_usec * 1000;
            sync_wait_time.tv_sec += wait_for_time_seconds;
            ret = pthread_cond_timedwait(&sync_data->sync_cond,
                                         &sync_data->sync_mutex,
                                         &sync_wait_time);
        }
        else
        {
            ret = pthread_cond_wait(&sync_data->sync_cond,
                                    &sync_data->sync_mutex);
#ifdef QMI_RIL_UTF
            if ( qmi_shutdown_sync_thread != 0 )
            {
              pthread_exit(NULL);
            }
#endif
        }
    }

    return ret;
}

/***************************************************************************************************
    @function
    util_sync_data_signal_on_cond

    @implementation detail
    None.
***************************************************************************************************/
int util_sync_data_signal_on_cond(util_sync_data_type *sync_data)
{
    int ret;

    ret = ESUCCESS;

    if(sync_data)
    {
        ret = pthread_cond_signal(&sync_data->sync_cond);
    }

    return ret;
}

