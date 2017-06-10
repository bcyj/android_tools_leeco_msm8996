/***************************************************************************************************
    @file
    util_synchronization.h

    @brief
    Facilitates synchronization mechanisms by providing synchronization related utilities.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef UTIL_SYNCHRONIZATION
#define UTIL_SYNCHRONIZATION

#include "utils_standard.h"
#include "util_bit_field.h"

#define UTIL_SYNC_BIT_FIELD_NONE                ((uint64_t) 0)
#define UTIL_SYNC_BIT_FIELD_COND_VAR_USED       (((uint64_t) 1) << 0)

typedef struct util_sync_data_type
{
    pthread_mutex_t sync_mutex;
    pthread_cond_t sync_cond;
    util_bit_field_type sync_data_bit_field;
}util_sync_data_type;






/***************************************************************************************************
    @function
    util_sync_data_init

    @brief
    Creates a synchronization object.

    @param[in]
        sync_data_bit_field
            bit field to indicate the specifications of the to be created sync object
            conditional variable is created if bit field contains UTIL_SYNC_BIT_FIELD_COND_VAR_USED

    @param[out]
        sync_data
            pointer to synchronization object which will
            hold the created mutex (and condition variable, If created)

    @retval
    none
***************************************************************************************************/
void util_sync_data_init(util_sync_data_type *sync_data,
                         util_bit_field_type sync_data_bit_field);




/***************************************************************************************************
    @function
    util_sync_data_destroy

    @brief
    Destroys a synchronization object.

    @param[in]
    @param[out]
        sync_data
            pointer to synchronization object which is to be destroyed

    @retval
    none
***************************************************************************************************/
void util_sync_data_destroy(util_sync_data_type *sync_data);





/***************************************************************************************************
    @function
    util_sync_data_acquire_mutex

    @brief
    Acquires the mutex contained by synchronization object.

    @param[in]
         sync_data
            pointer to synchronization object whose mutex needs to be acquired

    @param[out]
        none

    @retval
    ESUCCESS If mutex has been successfully acquired, FALSE otherwise
***************************************************************************************************/
int util_sync_data_acquire_mutex(util_sync_data_type *sync_data);





/***************************************************************************************************
    @function
    util_sync_data_release_mutex

    @brief
    Releases the mutex contained by synchronization object.

    @param[in]
         sync_data
            pointer to synchronization object whose mutex needs to be released

    @param[out]
        none

    @retval
    ESUCCESS If mutex has been successfully released, FALSE otherwise
***************************************************************************************************/
int util_sync_data_release_mutex(util_sync_data_type *sync_data);





/***************************************************************************************************
    @function
    util_sync_data_wait_on_cond

    @brief
    Waits on the conditional variable contained by synchronization object.

    @param[in]
         sync_data
            pointer to synchronization object whose conditional variable needs to be used for
            waiting
         wait_for_time_seconds
            time (in seconds) to wait on the conditional variable, NIL If wait needs
            to be indefinite

    @param[out]
        none

    @retval
    ESUCCESS If wait been performed on the conditional variable and the thread has been wokenup,
    appropriate error code otherwise
***************************************************************************************************/
int util_sync_data_wait_on_cond(util_sync_data_type *sync_data,
                                int wait_for_time_seconds);






/***************************************************************************************************
    @function
    util_sync_data_signal_on_cond

    @brief
    Signal using the conditional variable contained by synchronization object.

    @param[in]
         sync_data
            pointer to synchronization object whose conditional variable needs to be used for
            signalling

    @param[out]
        none

    @retval
    ESUCCESS If signal is successful using the conditional variable, FALSE otherwise
***************************************************************************************************/
int util_sync_data_signal_on_cond(util_sync_data_type *sync_data);


#endif
