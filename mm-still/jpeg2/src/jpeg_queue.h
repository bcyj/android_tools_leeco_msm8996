/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef _JPEG_QUEUE
#define _JPEG_QUEUE

#include "os_int.h"

struct jpeg_q_t;
typedef struct jpeg_q_t *jpeg_queue_t;
/******************************************************************************
* Function: jpeg_queue_init
* Description: Initialize the queue.
*              It checks the pointer to the entry array and allocate
*              jpeg_q_t structure, and initialize queue head, tail
*              and counts all to zero.
*              Also initialize thread condition variable and mutex.
* Input parameters:
*   p_queue            - The pointer to queue object.
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EMALLOC
*     JPEGERR_ENULLPTR
* (See jpegerr.h for description of error values.)
* Notes: none
*****************************************************************************/
int jpeg_queue_init(jpeg_queue_t  *p_queue);

/******************************************************************************
* Function: jpeg_queue_enqueue
* Description: Enqueue a sequence of entry. It accepts the double pointer to the
*              entry to be enqueued and the number of entries in the
*              array, appends the entries sequentially to the tail of queue.
*              The number of entries to be enqueued is checked against
*              the valid slots of the queue, and
*              return fail if it is larger than the valid size.
* Input parameters:
*   queue              - The queue object.
*   pp_enqueue_buf     - The double pointer to enqueued entry(s) .
*   enqueue_entry_cnt  - The number of enqueued entry(s).
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
* (See jpegerr.h for description of error values.)
* Notes: none
*****************************************************************************/
int jpeg_queue_enqueue(
    jpeg_queue_t    queue,
    void            **pp_enqueue_entry,
    uint32_t        enqueue_entry_cnt);

/******************************************************************************
* Function: jpeg_queue_dequeue
* Description: Dequeue a entry from the queue.  It returns the double pointer
*              to the entry to be denqueued from the head of queue,
*              and accepts the number of time in mini-seconds that it conditional  .
*              waits if the queue if empty.
* Input parameters:
*   queue              - The queue object.
*   pp_dequeue_buffer  - The double pointer to dequeued entry.
*   time_out_ms        - The time out in milli-seconds
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_ETIMEDOUT
* Notes: none
*****************************************************************************/
int jpeg_queue_dequeue(
    jpeg_queue_t   queue,
    void           **pp_dequeue_entry,
    uint32_t       time_out_ms) ;

/******************************************************************************
* Function: jpeg_queue_destroy
* Description: Destroys the queue object and cleans up internal variables
* Input parameters:
*   p_queue            - The pointer to queue object to be destroyed.
* Return values: none
* Notes: none
*****************************************************************************/
void  jpeg_queue_destroy(jpeg_queue_t  *p_queue);

/******************************************************************************
* Function: jpeg_queue_reset
* Description: Resets the queue object
* Input parameters:
*  queue              - The queue object to be reset.
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_ENULLPTR
* Notes: none
*****************************************************************************/
int jpeg_queue_reset(jpeg_queue_t  queue);

/******************************************************************************
* Function: jpeg_queue_abort
* Description: Aborts the queue object
* Input parameters:
*   p_queue            - The pointer to queue object to be aborted.
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_ENULLPTR
* Notes: none
*****************************************************************************/
int  jpeg_queue_abort(jpeg_queue_t  *p_queue);

#endif // #ifndef _JPEG_QUEUE

