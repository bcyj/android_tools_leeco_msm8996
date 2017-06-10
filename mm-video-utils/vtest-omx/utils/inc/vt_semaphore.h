/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VT_SEMAPHORE_H
#define _VT_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/

/**
 * @brief Constructor
 */
int vt_semaphore_create(void** handle, int init_count, int max_count);

/**
 * @brief Destructor
 */
int vt_semaphore_destroy(void* handle);

/**
 * @brief Waits for the semaphore
 */
int vt_semaphore_wait(void* handle, int timeout);

/**
 * @brief Posts the semaphore
 */
int vt_semaphore_post(void* handle);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VT_SEMAPHORE_H
