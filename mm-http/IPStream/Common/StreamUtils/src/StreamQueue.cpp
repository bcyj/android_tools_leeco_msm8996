/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q U E U E    S E R V I C E S

GENERAL DESCRIPTION

  A queue is a simple data structure used for logically storing and re-
  trieving data blocks, in a first in - first out (FIFO) order without
  physically copying them. Software tasks and interrupt handlers can use
  queues for cleanly transferring data blocks to one another.

                        +-------+     +-------+     +-------+
                        | DATA  |     | DATA  |     | DATA  |
                        | BLOCK |     | BLOCK |     | BLOCK |
          +-------+     +-------+     +-------+     +-------+
          | QUEUE |---->| LINK  |---->| LINK  |---->| LINK  |---- |
   |----->+-------+     +-------+     +-------+     +-------+     |
   |      |       |     |       |     |       |     |       |     |
   |      +-------+     |       |     |       |     |       |     |
   |                    +-------+     +-------+     +-------+     |
   |                                                              |
   ---------------------------------------------------------------|

  The Queue Services provide a small set of declarations and functions for
  defining and initializing a queue, defining and initializing a links with-
  in a data block, placing a data block at the tail of a queue, removing a
  data block from the head of a queue, and removing a data block from any
  position in a queue.

  Aside from requiring each data block to contain a link, the Queue Services
  impose no restrictions on the size of structure of data blocks used with
  with queues. This allows software to pass virtually any type of data on
  queues. Notice that a data block may contain multiple links allowing it to
  be placed simultaneously on multiple queues.


EXTERNALIZED FUNCTIONS

  StreamQ_init
    This function initializes a queue. It should be called on behalf of a
    queue prior to using the queue.

  StreamQ_link
    This function initializes a link field. It should be called on behalf
    of a link field prior to using the link filed with the other Queue
    Services.

  StreamQ_get
    This function removes the data block at head of a queue and returns a
    pointer to the data block. If the queue is empty, then a NULL pointer
    is returned.

  StreamQ_check
    This function returns a pointer to the data block at the head of a queue.
    The data block is not removed from the queue. If the queue is empty, then
    a NULL pointer is returned.

  StreamQ_last_get
    This function removes the data block at the tail of a queue and returns a
    pointer to the data block. If the queue is empty, then a NULL pointer
    is returned.

  StreamQ_put
    This function places a data block at the tail of a queue.

  StreamQ_cnt
    This function returns the number of items currently on a queue.

  StreamQ_linear_search
    Performs a linear search of the queue calling a users callback on each
    nodal visit to determine whether or not the search should be terminated.

  StreamQ_linear_delete
    Performs a linear traversal of the queue calling a compare callback on
    each node, should the compare callback indicate that the item should be
    deleted the item is removed from the queue and the users action callback
    is called passing to the callback the pointer to the now deleted item

INITIALIZATION AND SEQUENCING REQUIREMENTS

  Prior to use, a queue must be initialized by calling StreamQ_init. Similarly,
  a link must be initialized prior to use by calling StreamQ_link.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================
                              Edit History

$PVCSPath: O:/src/asw/COMMON/vcs/queue.c_v   1.4   02 Oct 2001 10:46:56   rajeevg  $
$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/StreamQueue.cpp#8 $ $DateTime: 2012/03/20 07:46:30 $ $Author: kamit $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/17/06    ck     Fixed a compilation issue with a hanging brace for LTK builds
01/10/06    ck     Added function StreamQ_delete_ext that returns a boolean indicating
                   if the node was found and removed
05/20/05    ck     Fixed an issue with the function StreamQ_delete which was stuck in
                   an infinite loop when the node to be deleted was not in the
                   queue.
09/28/01    gr     Merged in changes from the MSP archive. Includes clean-up
                   of source code and comments, queue-specific mutual
                   exclusion for Win32 targets and the new function
                   StreamQ_linear_delete. Removed support for UTF_ASP.
01/25/01   day     Merged from MSM5105_COMMON.00.00.05.
                   Added support for MSM5000_IRAM_FWD.
                   Added support for UTF_ASP.
01/13/00   jct     Add StreamQ_linear_search
12/10/99   gr      Merged from MSM3000 Rel 4.0c.
                   Fixes to StreamQ_insert, StreamQ_delete.
                   Introduced StreamQ_last_get().
                   Changed type of queue count from word to int.
                   Added calls to Q_XCEPT macros in all queue functions.
04/09/99    sk     Comments changed.
04/06/99    sk     Introduced FEATURE_Q_NO_SELF_QPTR and FEATURE_Q_SINGLE_LINK
12/16/98   jct     Removed 80186 support
10/08/98   jct     Moved INTLOCK in StreamQ_put to beginning of function, added
                   setting link pointer to NULL in StreamQ_delete
09/24/98   jct     Update copyright
12/20/96   rdh     Removed from Boot Block, changed to Pascal call convention.
04/08/96   dna     Added comment to each func. saying it is in the Boot Block
02/28/96   dna     Prepended bb_ to all function names and put module in the
                   Boot Block.
04/23/92   ip      Initial porting of file from Brassboard to DMSS.
02/20/92   arh     Added StreamQ_check routine

===========================================================================*/

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */

#include "StreamQueue.h"
#include "qtv_msg.h"

/* ==========================================================================
FUNCTION Q_LOCK
DESCRIPTION
   Enter critical section for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
#define StreamQ_lock( q ) if (q->lock) MM_CriticalSection_Enter(q->lock)

/* ==========================================================================
FUNCTION Q_FREE
DESCRIPTION
   Leave critical section for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
#define StreamQ_free( q ) if ( q->lock) MM_CriticalSection_Leave(q->lock)

/*==========================================================================
FUNCTION Q_INIT

DESCRIPTION
  This function initializes a specified queue. It should be called for each
  queue prior to using the queue with the other Queue Services.

DEPENDENCIES
  None.

RETURN VALUE
  A pointer to the initialized queue.

SIDE EFFECTS
  The specified queue is initialized for use with Queue Services.
===========================================================================*/

StreamQ_type* StreamQ_init(
   StreamQ_type    *StreamQ_ptr,     /* Ptr to queue to be initialized. */
   MM_HANDLE critSect /* Ptr to critical section. */
)
{
   StreamQ_ptr->link.next_ptr = (StreamQ_link_type *)(&StreamQ_ptr->link);  /* Points to q link. */
   StreamQ_ptr->link.prev_ptr = (StreamQ_link_type *)(&StreamQ_ptr->link);  /* Points to q link. */

   #ifndef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_ptr->link.self_ptr = NULL;
      StreamQ_ptr->link.StreamQ_ptr    = NULL;
   #endif

   StreamQ_ptr->cnt = 0;

   if ( StreamQ_ptr->lock )
   {
     MM_CriticalSection_Release(StreamQ_ptr->lock);
   }

   if ( critSect )
   {
     // user supplied critical section cache it
     StreamQ_ptr->lock = critSect;
   }
   else if( MM_CriticalSection_Create(&StreamQ_ptr->lock) != 0)
   {
     QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_FATAL,
                   "StreamQ_init: Unable to create a critical section: %p",
                   (void *)StreamQ_ptr->lock);
   }

   return StreamQ_ptr;
} /* END StreamQ_init */


/*===========================================================================
FUNCTION Q_LINK

DESCRIPTION
  This function initializes a specified link. It should be called for each
  link prior to using the link with the other Queue Services.

DEPENDENCIES
  None.

RETURN VALUE
  A pointer to the initialized link.

SIDE EFFECTS
  The specified link is initialized for use with the Queue Services.
===========================================================================*/
StreamQ_link_type* StreamQ_link(
   void         *item_ptr,  /* Ptr to item or variable containing link. */
   StreamQ_link_type  *link_ptr   /* Ptr to link field within variable. */
)
{
   link_ptr->next_ptr = NULL;

   #ifndef FEATURE_QTVQUEUE_SINGLE_LINK
      link_ptr->prev_ptr = NULL;
   #endif

   #ifndef FEATURE_QTVQUEUE_NO_SELF_QPTR
      link_ptr->self_ptr = item_ptr;
      link_ptr->StreamQ_ptr    = NULL;
   #endif

   return link_ptr;
} /* END StreamQ_link */

/*===========================================================================
FUNCTION Q_PUT

DESCRIPTION
  This function enqueues an item onto a specified queue using a specified
  link.

DEPENDENCIES
  The specified queue should have been previously initialized via a call
  to StreamQ_init. The specified link field of the item should have been prev-
  iously initialized via a call to StreamQ_init_link.

RETURN VALUE
  None.

SIDE EFFECTS
  The specified item is placed at the tail of the specified queue.
===========================================================================*/
void StreamQ_put(
   StreamQ_type       *StreamQ_ptr,    /* Ptr to queue. */
   StreamQ_link_type  *link_ptr  /* Ptr to item link to use for queueing. */
)
{
   StreamQ_lock( StreamQ_ptr );

   link_ptr->next_ptr = (StreamQ_link_type *)&StreamQ_ptr->link;

   #ifndef FEATURE_QTVQUEUE_NO_SELF_QPTR
      link_ptr->StreamQ_ptr    = StreamQ_ptr;
   #endif

   #ifndef FEATURE_QTVQUEUE_SINGLE_LINK
      link_ptr->prev_ptr = StreamQ_ptr->link.prev_ptr;

   #endif

   StreamQ_ptr->link.prev_ptr->next_ptr = link_ptr;
   StreamQ_ptr->link.prev_ptr           = link_ptr;
   StreamQ_ptr->cnt++;

   StreamQ_free( StreamQ_ptr );
   return;
} /* END StreamQ_put */

/*===========================================================================
FUNCTION Q_GET

DESCRIPTION
  This function removes an item from the head of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.
===========================================================================*/
void* StreamQ_get(
  StreamQ_type  *StreamQ_ptr  /* Ptr to queue. */
)
{
   StreamQ_link_type  *link_ptr;

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_link_type  *ret_ptr = NULL;
   #endif

   StreamQ_lock( StreamQ_ptr );

   /* Get ptr to 1st queue item.
   */
   link_ptr = StreamQ_ptr->link.next_ptr;

   /* Can only get an item if the queue is non empty
   */
   if( StreamQ_ptr->cnt > 0 )
   {
      StreamQ_ptr->link.next_ptr = link_ptr->next_ptr;

      #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
         if (link_ptr->next_ptr == (StreamQ_link_type *)StreamQ_ptr)
         {
            StreamQ_ptr->link.prev_ptr = (StreamQ_link_type *)(&StreamQ_ptr->link);
         }
      #else
         link_ptr->next_ptr->prev_ptr = &StreamQ_ptr->link;
      #endif

      StreamQ_ptr->cnt--;

      /* Mark item as no longer in a queue.
      */
      #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
         link_ptr->next_ptr = NULL;
         ret_ptr = link_ptr;
      #else
         link_ptr->StreamQ_ptr = NULL;
      #endif
   }

   StreamQ_free( StreamQ_ptr );

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      return (void *)ret_ptr;
   #else
      return link_ptr->self_ptr;
   #endif
} /* END StreamQ_get */

/*===========================================================================
FUNCTION Q_LAST_GET

DESCRIPTION
  This function returns the item which was most recently enqueued in a queue.

  Note, this is different from StreamQ_get() which returns the oldest item in a
  queue.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void * StreamQ_last_get(
   StreamQ_type* StreamQ_ptr
)
{
   StreamQ_link_type  *link_ptr;
   #if defined(FEATURE_QTVQUEUE_SINGLE_LINK) || defined(FEATURE_QTVQUEUE_NO_SELF_QPTR)
      StreamQ_link_type  *ret_ptr=NULL;
   #endif  /* FEATURE_Q_SINGLE_LINK || FEATURE_Q_NO_SELF_QPTR */

   StreamQ_lock( StreamQ_ptr );

   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      for( link_ptr           =  (StreamQ_link_type *)StreamQ_ptr;
           link_ptr->next_ptr != StreamQ_ptr->link.prev_ptr;
           link_ptr           =  link_ptr->next_ptr
         );

      if( StreamQ_ptr->cnt > 0 )
      {
         ret_ptr              = link_ptr->next_ptr;
         StreamQ_ptr->link.prev_ptr = link_ptr;
         link_ptr->next_ptr   = (StreamQ_link_type *)(&StreamQ_ptr->link);
         StreamQ_ptr->cnt--;
         #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
            ret_ptr->next_ptr = NULL;
         #else
            link_ptr        = ret_ptr;
            link_ptr->StreamQ_ptr = NULL;
         #endif
      }
   #else
      link_ptr = StreamQ_ptr->link.prev_ptr;

      if ( StreamQ_ptr->cnt > 0 )
      {
         StreamQ_ptr->link.prev_ptr         = link_ptr->prev_ptr;
         link_ptr->prev_ptr->next_ptr = &StreamQ_ptr->link;
         StreamQ_ptr->cnt--;

         #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
            link_ptr->next_ptr = NULL;
            ret_ptr            = link_ptr;
         #else
            link_ptr->StreamQ_ptr = NULL;
         #endif
      }
   #endif

   StreamQ_free( StreamQ_ptr );

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      return (void *)ret_ptr;
   #else
      return link_ptr->self_ptr;
   #endif
}  /* StreamQ_last_get */

/*===========================================================================
FUNCTION Q_NEXT

DESCRIPTION
  This function returns a pointer to the next item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  A pointer to the next item on the queue. If the end of the queue is reached,
  then NULL is returned.

SIDE EFFECTS
  None.
===========================================================================*/
void* StreamQ_next(
   StreamQ_type       *StreamQ_ptr,
   StreamQ_link_type  *StreamQ_link_ptr
)
{
   if( (void *) StreamQ_link_ptr->next_ptr != (void *) StreamQ_ptr )
   {
      return StreamQ_link_ptr->next_ptr;
   }
   else
   {
      return NULL;
   }
} /* END StreamQ_next */

/*===========================================================================
FUNCTION Q_PREV

DESCRIPTION
  This function returns a pointer to the previous item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  A pointer to the previous item on the queue. If the start of the queue is reached,
  then NULL is returned.

SIDE EFFECTS
  None.
===========================================================================*/
void* StreamQ_prev(
   StreamQ_type       *StreamQ_ptr,
   StreamQ_link_type  *StreamQ_link_ptr
)
{
   if( (void *) StreamQ_link_ptr->prev_ptr != (void *) StreamQ_ptr )
   {
      return StreamQ_link_ptr->prev_ptr;
   }
   else
   {
      return NULL;
   }
} /* END StreamQ_prev */

/*===========================================================================
FUNCTION Q_INSERT

DESCRIPTION
  This function inserts an item before a specified item on a queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is inserted before input item.
===========================================================================*/
void StreamQ_insert(
   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_type    *StreamQ_ptr,          /* Ptr to the queue */
   #endif
   StreamQ_link_type  *StreamQ_insert_ptr,   /* Ptr to link of item to insert */
   StreamQ_link_type  *StreamQ_item_ptr      /* Ptr to link item to insert before */
)
{
   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      StreamQ_link_type  *link_ptr;
   #endif

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_lock( StreamQ_ptr );
   #else
      StreamQ_lock( StreamQ_item_ptr->StreamQ_ptr );
   #endif

   StreamQ_insert_ptr->next_ptr = StreamQ_item_ptr;

   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      /* Start at beginning of queue and find the item that will be before the
      ** new item
      */
      #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
         link_ptr = (StreamQ_link_type *) StreamQ_ptr;
      #else
         link_ptr = (StreamQ_link_type *) StreamQ_item_ptr->StreamQ_ptr;
      #endif

      while (link_ptr->next_ptr != StreamQ_item_ptr)
      {
         link_ptr = link_ptr->next_ptr;
      }
      link_ptr->next_ptr = StreamQ_insert_ptr;

   #else
      StreamQ_insert_ptr->prev_ptr = StreamQ_item_ptr->prev_ptr;
      StreamQ_item_ptr->prev_ptr->next_ptr = StreamQ_insert_ptr;
      StreamQ_item_ptr->prev_ptr = StreamQ_insert_ptr;
   #endif

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_ptr->cnt++;
   #else
      StreamQ_insert_ptr->StreamQ_ptr = StreamQ_item_ptr->StreamQ_ptr;
      StreamQ_item_ptr->StreamQ_ptr->cnt++;
   #endif

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_free( StreamQ_ptr );
   #else
      StreamQ_free( StreamQ_item_ptr->StreamQ_ptr );
   #endif
   return;
} /* END StreamQ_insert */

/*===========================================================================
FUNCTION Q_DELETE

DESCRIPTION
  This function removes an item from a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is delete from the queue.
===========================================================================*/
void StreamQ_delete(
   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_type       *StreamQ_ptr,         /* Ptr to the Queue */
   #endif
      StreamQ_link_type  *StreamQ_delete_ptr   /* Ptr to link of item to delete */
)
{
   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      StreamQ_link_type *link_ptr;
      StreamQ_type *real_StreamQ_ptr;
      int qcount;
   #endif

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_lock( StreamQ_ptr );
   #else
      StreamQ_lock( StreamQ_delete_ptr->StreamQ_ptr );
   #endif

   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
         real_StreamQ_ptr = StreamQ_ptr;
      #else
         real_StreamQ_ptr = StreamQ_delete_ptr->StreamQ_ptr;
      #endif

      for( qcount = StreamQ_ptr->cnt,
           link_ptr           =  (StreamQ_link_type *) real_StreamQ_ptr;
           link_ptr->next_ptr != StreamQ_delete_ptr && qcount > 0;
           link_ptr           =  link_ptr->next_ptr, qcount--);

      if(qcount > 0)
      {
        link_ptr->next_ptr = StreamQ_delete_ptr->next_ptr;

        if(link_ptr->next_ptr == (StreamQ_link_type *) real_StreamQ_ptr)
        {
          real_StreamQ_ptr->link.prev_ptr = link_ptr;
        }
   #else
        StreamQ_delete_ptr->prev_ptr->next_ptr = StreamQ_delete_ptr->next_ptr;
        StreamQ_delete_ptr->next_ptr->prev_ptr = StreamQ_delete_ptr->prev_ptr;
   #endif

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
        StreamQ_ptr->cnt--;
        StreamQ_delete_ptr->next_ptr = NULL;
   #else
        StreamQ_delete_ptr->StreamQ_ptr->cnt--;
   #endif

   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      }
   #endif

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_free( StreamQ_ptr );
   #else
      StreamQ_free( StreamQ_delete_ptr->StreamQ_ptr );
      StreamQ_delete_ptr->StreamQ_ptr = NULL;
   #endif
   return;
} /* END StreamQ_delete */

/*===========================================================================
FUNCTION Q_DELETE_EXT

DESCRIPTION
  This function removes an item from a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  FALSE : if the item is not found in the queue.
  TRUE  : if the item is found and removed from the queue.

SIDE EFFECTS
  Input item is deleted from the queue.
===========================================================================*/
boolean StreamQ_delete_ext(
   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_type       *StreamQ_ptr,         /* Ptr to the Queue */
   #endif
      StreamQ_link_type  *StreamQ_delete_ptr   /* Ptr to link of item to delete */
)
{
   #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
      StreamQ_link_type *link_ptr;
      StreamQ_type *real_StreamQ_ptr;
      int qcount;
   #endif
      boolean item_in_q = FALSE;

      if (StreamQ_delete_ptr && StreamQ_delete_ptr->StreamQ_ptr &&
          StreamQ_delete_ptr->prev_ptr && StreamQ_delete_ptr->next_ptr
   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
          && StreamQ_ptr
   #endif
         )
      {
        #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
          StreamQ_lock( StreamQ_ptr );
        #else
          StreamQ_lock( StreamQ_delete_ptr->StreamQ_ptr );
        #endif

        #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
          #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
             real_StreamQ_ptr = StreamQ_ptr;
          #else
             real_StreamQ_ptr = StreamQ_delete_ptr->StreamQ_ptr;
          #endif

          for( qcount = StreamQ_ptr->cnt,
               link_ptr           =  (StreamQ_link_type *) real_StreamQ_ptr;
               link_ptr->next_ptr != StreamQ_delete_ptr && qcount > 0;
               link_ptr           =  link_ptr->next_ptr, qcount--);

          if(qcount > 0)
          {
            link_ptr->next_ptr = StreamQ_delete_ptr->next_ptr;

            if(link_ptr->next_ptr == (StreamQ_link_type *) real_StreamQ_ptr)
            {
              real_StreamQ_ptr->link.prev_ptr = link_ptr;
            }
        #else
            StreamQ_delete_ptr->prev_ptr->next_ptr = StreamQ_delete_ptr->next_ptr;
            StreamQ_delete_ptr->next_ptr->prev_ptr = StreamQ_delete_ptr->prev_ptr;
        #endif

        #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
            StreamQ_ptr->cnt--;
            StreamQ_delete_ptr->next_ptr = NULL;
        #else
            StreamQ_delete_ptr->StreamQ_ptr->cnt--;
        #endif

          item_in_q = TRUE;
        #ifdef FEATURE_QTVQUEUE_SINGLE_LINK
          }
        #endif

        #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
          StreamQ_free( StreamQ_ptr );
        #else
        if (StreamQ_delete_ptr->StreamQ_ptr)
        {
          StreamQ_free( StreamQ_delete_ptr->StreamQ_ptr );
        }
        #endif
     }

     return item_in_q;
} /* END StreamQ_delete_ext */

/*===========================================================================
FUNCTION Q_CHECK

DESCRIPTION
  This function returns a pointer to the data block at the head of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  None
===========================================================================*/
void* StreamQ_check(
  StreamQ_type  *StreamQ_ptr
)
{
   StreamQ_link_type  *link_ptr;

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      StreamQ_link_type  *ret_ptr = NULL;
   #endif

   StreamQ_lock( StreamQ_ptr );

   link_ptr = StreamQ_ptr->link.next_ptr;

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      if( StreamQ_ptr->cnt > 0 )
      {
         ret_ptr = link_ptr;
      }
   #endif

   StreamQ_free( StreamQ_ptr );

   #ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
      return (void *)ret_ptr;
   #else
      return link_ptr->self_ptr;
   #endif
} /* END StreamQ_check */


/*===========================================================================

FUNCTION Q_LAST_CHECK

DESCRIPTION
  This function returns the item which was most recently enqueued in a queue.

  Note, this is different from StreamQ_check() which returns the oldest item in a
  queue.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void * StreamQ_last_check
(
  StreamQ_type* StreamQ_ptr           /* The queue from which the item will be removed */
)
{
   StreamQ_link_type  *link_ptr;                         /* For returning value. */
#if defined(FEATURE_QTVQUEUE_SINGLE_LINK) || defined(FEATURE_QTVQUEUE_NO_SELF_QPTR)
   StreamQ_link_type  *ret_ptr=NULL;                     /* For returning value. */
#endif  /* FEATURE_Q_SINGLE_LINK || FEATURE_Q_NO_SELF_QPTR */

 /*- - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - -*/

   StreamQ_lock( StreamQ_ptr );

#ifdef FEATURE_QTVQUEUE_SINGLE_LINK
   for( link_ptr = (StreamQ_link_type *)StreamQ_ptr;link_ptr->next_ptr
        != StreamQ_ptr->link.prev_ptr;link_ptr=link_ptr->next_ptr);

    if (StreamQ_ptr->cnt > 0)
     {
      ret_ptr = link_ptr->next_ptr;

#ifndef FEATURE_QTVQUEUE_NO_SELF_QPTR
      link_ptr = ret_ptr;
#endif
     }
#else
   link_ptr = StreamQ_ptr->link.prev_ptr;

   if ( StreamQ_ptr->cnt > 0 )
   {
#ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
     ret_ptr = link_ptr;
#endif
   }
#endif

   StreamQ_free( StreamQ_ptr );

#ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
   return  (void *)ret_ptr;
#else
   return ( link_ptr->self_ptr );
#endif
}  /* StreamQ_last_check */


/*===========================================================================
FUNCTION Q_CNT

DESCRIPTION
  This function returns the number of items currently queued on a specified
  queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  The number of items currently queued on the specified queue.

SIDE EFFECTS
  None.
===========================================================================*/
int StreamQ_cnt(
  StreamQ_type  *StreamQ_ptr
)
{
   return StreamQ_ptr->cnt;
} /* END StreamQ_cnt */


/*===========================================================================
FUNCTION Q_LINEAR_SEARCH

DESCRIPTION
  Given a comparison function, this function traverses the elements in
  a queue, calls the compare function, and returns a pointer to the
  current element being compared if the user passed compare function
  returns non zero.

  The user compare function should return 0 if the current element is
  not the element in which the compare function is interested.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

  The user's queue elements must have StreamQ_link_type as the first element
  of the queued structure.

RETURN VALUE
  A pointer to the found element

SIDE EFFECTS
  None.
===========================================================================*/
void* StreamQ_linear_search(
   StreamQ_type             *StreamQ_ptr,
   StreamQ_compare_func_type compare_func,
   void               *compare_val
)
{
   StreamQ_generic_item_type *item_ptr = NULL;

   item_ptr = (StreamQ_generic_item_type*)StreamQ_check( StreamQ_ptr );

   while( item_ptr != NULL )
   {
      if( compare_func( item_ptr, compare_val ) != 0 )
      {
         return item_ptr;
      }
      item_ptr = (StreamQ_generic_item_type*)StreamQ_next( StreamQ_ptr, &item_ptr->link );
   } /* END while traversing the queue */

   return NULL;
} /* END StreamQ_linear_search */


#if defined FEATURE_QTVQUEUE_NO_SELF_QPTR && defined FEATURE_QTVQUEUE_SINGLE_LINK
/*===========================================================================
FUNCTION Q_LINEAR_DELETE

DESCRIPTION
  Given a comparison function, this function traverses the elements in
  a queue, calls the compare function, and returns a pointer to the
  current element being compared if the user passed compare function
  returns non zero.  In addition, the item will be removed from the queue.

  The user compare function should return 0 if the current element is
  not the element in which the compare function is interested.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

  The user's queue elements must have StreamQ_link_type as the first element
  of the queued structure.

  The user's compare function will be passed NULL for the compare value.

RETURN VALUE
  None

SIDE EFFECTS
  None.
===========================================================================*/
void StreamQ_linear_delete(
   StreamQ_type             *StreamQ_ptr,
   StreamQ_compare_func_type compare_func,
   void               *param,
   StreamQ_action_func_type  action_func
)
{
   StreamQ_generic_item_type *item_ptr = NULL;
      /* Used in the traversal to point to the current item
      */
   StreamQ_generic_item_type *prev_ptr = NULL;
      /* Used in the traversal to point to the item previous to
      ** the current item.  This makes removing the current item
      ** a constant time operation
      */

   /* User must provide a compare function, otherwise, this is
   ** meaningless.
   */
   if( compare_func == NULL )
   {
      return;
   }

   StreamQ_lock( StreamQ_ptr );

   /* item_ptr points to the first item on the list
   */
   item_ptr = (StreamQ_generic_item_type*)StreamQ_check( StreamQ_ptr );
   prev_ptr = NULL;

   while( item_ptr != NULL )
   {
      if( compare_func( item_ptr, NULL ) != 0 )
      {
         /* Remove the item
         */
         if( prev_ptr != NULL )
         {
            /* Remove from the middle or tail
            */
            prev_ptr->link.next_ptr = item_ptr->link.next_ptr;
            item_ptr->link.next_ptr = NULL;
         }
         else
         {
            /* Remove from the head
            */
            StreamQ_get( StreamQ_ptr );
         }

         /* Call the action function if there is one
         */
         if( action_func )
         {
            action_func( item_ptr, param );
         }
         break;
      }

      /* Move on to the next item
      */
      prev_ptr = item_ptr;
      item_ptr = (StreamQ_generic_item_type*)StreamQ_next( StreamQ_ptr, &item_ptr->link );
   } /* END while traversing the queue */

   StreamQ_free( StreamQ_ptr );
   return;

} /* END StreamQ_linear_delete */

#endif /* defined FEATURE_Q_NO_SELF_QPTR && defined FEATURE_Q_SINGLE_LINK */
