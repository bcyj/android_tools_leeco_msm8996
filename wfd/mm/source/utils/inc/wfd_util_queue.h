/* =======================================================================
                              wfd_util_queue.h
DESCRIPTION
  This header defines the wfd source item class.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/wfd_util_queue.h
 
========================================================================== */

#ifndef _VENC_QUEUE_H
#define _VENC_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/

/**
 * @brief Constructor
 *
 * @param handle The queue handle
 * @param max_queue_size Max number of items in queue (size)
 * @param max_data_size Max data size
 */
int venc_queue_create(void** handle, int max_queue_size, int max_data_size);

/**
 * @brief Destructor
 *
 * @param handle The queue handle
 */
int venc_queue_destroy(void* handle);

/**
 * @brief Pushes an item onto the queue.
 *
 * @param handle The queue handle
 * @param data Pointer to the data
 * @param data_size Size of the data in bytes
 */
int venc_queue_push(void* handle, void* data, int data_size);

/**
 * @brief Pops an item from the queue.
 *
 * @param handle The queue handle
 * @param data Pointer to the data
 * @param data_size Size of the data buffer in bytes
 */
int venc_queue_pop(void* handle, void* data, int data_size);

/**
 * @brief Peeks at the item at the head of the queue
 *
 * Nothing will be removed from the queue
 *
 * @param handle The queue handle
 * @param data Pointer to the data
 * @param data_size Size of the data in bytes
 *
 * @return 0 if there is no data
 */
int venc_queue_peek(void* handle, void* data, int data_size);

/**
 * @brief Get the size of the queue.
 *
 * @param handle The queue handle
 */
int venc_queue_size(void* handle);

/**
 * @brief Check if the queue is full
 *
 * @param handle The queue handle
 */
int venc_queue_full(void* handle);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VENC_QUEUE_H
