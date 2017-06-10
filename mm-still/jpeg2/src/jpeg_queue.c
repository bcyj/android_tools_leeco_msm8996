/*============================================================================

   Copyright (c) 2011, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_queue.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_common_private.h"
#include "os_thread.h"
#include "os_timer.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
//The maximum queue entry is 16
#define QUEUE_MASK   0xf
#define QUEUE_MOD(a) ((a) & QUEUE_MASK)
#define MAX_QUEUE_NUM  (QUEUE_MASK + 1)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct
{
    void        *p_data;

} jpeg_queue_pool_t;

typedef enum
{
    QUEUE_STATE_IDLE = 0,
	QUEUE_STATE_TIMEWAIT,   // queue state during dequeue but queue is empty,
	                        // and waiting for entries
    QUEUE_STATE_ABORT,      // queue is in the state of being aborted
    QUEUE_STATE_ABORTED     // queue is aborted, will not respond to dequeue/enqueue,
                            //   need reset
} jpeg_queue_state_t;

/* The structure defining the queue. */
typedef struct jpeg_q_t
{
   jpeg_queue_pool_t  queue_pool[MAX_QUEUE_NUM];
   uint32_t        queue_head;     // The head
   uint32_t        queue_tail;     // The tail
   int32_t         queue_cnt;      // The number of entry in the queue
   uint8_t         abort_flag;     // The flag to abort queue

   os_mutex_t      mutex;          // The mutex for locking use
   os_cond_t       get_cond;       // The condition variable signaling the
                                   // end of receipt on an entry into queue
   os_cond_t       abort_cond;     // The condition variable signaling the
                                   // readiness of whether any abort can
                                   // be unblocked
   jpeg_queue_state_t state;

}jpeg_q_t;

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations of helper functions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Extern variables
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
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
int jpeg_queue_init(jpeg_queue_t *p_queue)
{
    jpeg_q_t *p_q;

    // Queue validation
    if (!p_queue)
    {
        JPEG_DBG_ERROR("jpeg_queue_init: failed with empty queue pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // Allocate the jpeg_q_t structure
    p_q = (jpeg_q_t *)JPEG_MALLOC(sizeof(jpeg_q_t));
    *p_queue= (jpeg_queue_t)p_q;
    if (!p_q)
    {
        JPEG_DBG_ERROR("jpeg_queue_init: failed with allocation queue\n");
        return JPEGERR_EMALLOC;
    }

    // Zero out all fields
    // Initialize queue head, tail and counts all to zero
    STD_MEMSET(p_q, 0, sizeof(jpeg_q_t));

    // Initialize thread condition variable and mutex
    (void)os_mutex_init(&(p_q->mutex));
    (void)os_cond_init(&(p_q->get_cond));
    (void)os_cond_init(&(p_q->abort_cond));

    return JPEGERR_SUCCESS;
}

/******************************************************************************
* Function: jpege_queue_enqueue
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
    uint32_t        enqueue_entry_cnt)
{
    uint32_t        i, q_index;
    jpeg_q_t        *p_q;

    // Input parameter validation
    if ((!pp_enqueue_entry) || (!enqueue_entry_cnt))
    {
        JPEG_DBG_ERROR("jpeg_queue_enqueue: failed with input parameter check\n");
        return JPEGERR_EBADPARM;
    }
    p_q = (jpeg_q_t *)queue;
    if (!p_q)
    {
        JPEG_DBG_ERROR("jpeg_queue_enqueue: failed with empty queue pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // Enqueue entry(s) to the queue
    os_mutex_lock(&(p_q->mutex));

    // Reject if the enqueued entry(s) are greater than valid slots left:
    // queue_cnt is current number of entries in queue,
    // it plus the number of entry to be enqueued can not exceed the
    // number of entries allowed inside queue.
    if ((p_q->queue_cnt + enqueue_entry_cnt) > MAX_QUEUE_NUM)
    {
        JPEG_DBG_ERROR("jpeg_queue_enqueue: enqueuing more entries than valid queue slots\n");
        os_mutex_unlock(&(p_q->mutex));
        return JPEGERR_EFAILED;
    }

    // Enqueue the entry
    for (i = 0; i < enqueue_entry_cnt; i++)
    {
        // Appends the entries sequentially to the tail of queue.
        q_index = QUEUE_MOD(p_q->queue_tail + i);
        p_q->queue_pool[q_index].p_data  = *(pp_enqueue_entry + i);
    }

    // Update the tail of queue and entries number
    p_q->queue_tail = QUEUE_MOD(p_q->queue_tail + enqueue_entry_cnt);
    p_q->queue_cnt  += enqueue_entry_cnt;

    // Signal that enqueuing entries is done
    os_cond_signal(&(p_q->get_cond));
    os_mutex_unlock(&(p_q->mutex));
    return JPEGERR_SUCCESS;
}

/******************************************************************************
* Function: jpeg_queue_dequeue
* Description: Dequeue a entry from the queue.  It returns the double pointer
*              to the entry to be denqueued from the head of queue,
*              and accepts the number of time in mini-seconds that it conditional  .
*              waits if the queue if empty.
* Input parameters:
*   queue              - The queue object.
*   pp_dequeue_buffer  - The double pointer to dequeued entry.
*   ms                 - The time out in minisecond
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
    uint32_t       time_out_ms)
{
    uint32_t   q_index;
    jpeg_q_t   *p_q;
    int        rc;

    // Input parameter validation
    if (!pp_dequeue_entry)
    {
        JPEG_DBG_ERROR("jpeg_queue_dequeue: failed with input parameter check\n");
        return JPEGERR_EBADPARM;
    }
    p_q = (jpeg_q_t *)queue;
    if (!p_q)
    {
        JPEG_DBG_ERROR("jpeg_queue_dequeue: failed with empty queue pointer\n");
        return JPEGERR_ENULLPTR;
    }

    os_mutex_lock(&(p_q->mutex));

    if (p_q->state == QUEUE_STATE_ABORTED)
    {
        os_mutex_unlock(&(p_q->mutex));
        JPEG_DBG_ERROR("jpeg_queue_dequeue: Aborted \n");
        return JPEGERR_EFAILED;
    }

    // Check if queue is empty:
    // queue_cnt is current number of entries in queue.
    while (p_q->queue_cnt <= 0 && QUEUE_STATE_ABORT != p_q->state)
    {
        p_q->state = QUEUE_STATE_TIMEWAIT;
        // fails after conditional waiting time_out_ms in milli-seconds
        rc = os_cond_timedwait(&(p_q->get_cond), &(p_q->mutex), time_out_ms);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpeg_queue_dequeue: failed with empty queue\n");
            if (QUEUE_STATE_ABORT == p_q->state)
            {
                p_q->state = QUEUE_STATE_ABORTED;
                os_cond_signal(&(p_q->abort_cond));
            }
            p_q->state = QUEUE_STATE_IDLE;
            os_mutex_unlock(&(p_q->mutex));
            return rc;
        }
    }

    if (QUEUE_STATE_ABORT == p_q->state)
    {
        p_q->state = QUEUE_STATE_IDLE;
        // signal abort cond
        os_cond_signal(&(p_q->abort_cond));
        os_mutex_unlock(&(p_q->mutex));
        JPEG_DBG_ERROR("jpeg_queue_dequeue: failed with abort\n");
        return JPEGERR_EFAILED;
    }

    // Dequeue an entry from queue
    q_index = p_q->queue_head;

    // Dequeue the entry from the head of queue
    *pp_dequeue_entry = p_q->queue_pool[q_index].p_data;

    // Update the head of queue and entries number
    p_q->queue_head = QUEUE_MOD(p_q->queue_head + 1);
    p_q->queue_cnt  -= 1;
    p_q->state = QUEUE_STATE_IDLE;

    os_mutex_unlock(&(p_q->mutex));
    return JPEGERR_SUCCESS;
}

/******************************************************************************
* Function: jpeg_queue_destroy
* Description: Destroys the queue object and cleans up internal variables
* Input parameters:
*   p_queue            - The pointer to queue object to be destroyed.
* Return values: none
* Notes: none
 *****************************************************************************/
void  jpeg_queue_destroy(jpeg_queue_t  *p_queue)
{
    if (p_queue)
    {
        jpeg_q_t *p_q = (jpeg_q_t *)(*p_queue);
        if (p_q)
        {
            JPEG_DBG_LOW("jpeg_queue_destroy: jpeg queue destroy\n");
            jpeg_queue_abort(p_queue);
            JPEG_DBG_LOW("jpeg_queue_destroy: aborted\n");
            // queue clean up
            os_cond_destroy(&(p_q->get_cond));
            os_cond_destroy(&(p_q->abort_cond));
            os_mutex_destroy(&(p_q->mutex));
            JPEG_FREE(p_q);
        }
        *p_queue = NULL;
    }
}

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
int  jpeg_queue_reset(jpeg_queue_t  queue)
{
    jpeg_q_t *p_q = (jpeg_q_t *)queue;
    int rc = JPEGERR_SUCCESS;

    if (!p_q)
    {
        JPEG_DBG_ERROR("jpeg_queue_reset: failed with empty queue pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // Abort previous queue
    rc = jpeg_queue_abort(&queue);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpeg_queue_reset: jpeg_queue_abort failed \n");
        return rc;
    }

    os_mutex_lock(&(p_q->mutex));

    // Initialize queue head, tail and counts all to zero
    p_q->queue_head = 0;
    p_q->queue_tail = 0;
    p_q->queue_cnt = 0;
    (void)STD_MEMSET(&(p_q->queue_pool), 0, sizeof(jpeg_queue_pool_t));

    p_q->state = QUEUE_STATE_IDLE;
    os_mutex_unlock(&(p_q->mutex));
    return JPEGERR_SUCCESS;
}

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
int  jpeg_queue_abort(jpeg_queue_t  *p_queue)
{
    jpeg_q_t *p_q;
    if(!p_queue) {
        JPEG_DBG_ERROR("jpeg_queue_abort: failed with empty queue pointer\n");
        return JPEGERR_ENULLPTR;
    }
    p_q = (jpeg_q_t *)(*p_queue);
    if (!p_q) {
        JPEG_DBG_ERROR("jpeg_queue_abort: failed with empty queue\n");
        return JPEGERR_ENULLPTR;
    }

    // Abort
    os_mutex_lock(&(p_q->mutex));
    if (p_q->state == QUEUE_STATE_TIMEWAIT)
    {
		// Change queue state
        p_q->state = QUEUE_STATE_ABORT;
		// Signal the dequeue function
        os_cond_signal(&(p_q->get_cond));
        while (QUEUE_STATE_ABORT == p_q->state)
		{
			// Wait unitil abort can be unblocked
			os_cond_wait(&(p_q->abort_cond), &(p_q->mutex));
		}
	}
    p_q->state = QUEUE_STATE_ABORTED;
    os_mutex_unlock(&(p_q->mutex));

    return JPEGERR_SUCCESS;
}
