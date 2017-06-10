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

  q_init
    This function initializes a queue. It should be called on behalf of a
    queue prior to using the queue.

  q_link
    This function initializes a link field. It should be called on behalf
    of a link field prior to using the link filed with the other Queue
    Services.

  q_get
    This function removes the data block at head of a queue and returns a
    pointer to the data block. If the queue is empty, then a NULL pointer
    is returned.

  q_check
    This function returns a pointer to the data block at the head of a queue.
    The data block is not removed from the queue. If the queue is empty, then
    a NULL pointer is returned.

  q_last_get
    This function removes the data block at the tail of a queue and returns a
    pointer to the data block. If the queue is empty, then a NULL pointer
    is returned.

  q_put
    This function places a data block at the tail of a queue.

  q_cnt
    This function returns the number of items currently on a queue.

  q_linear_search
    Performs a linear search of the queue calling a users callback on each
    nodal visit to determine whether or not the search should be terminated.

  q_linear_delete
    Performs a linear traversal of the queue calling a compare callback on
    each node, should the compare callback indicate that the item should be
    deleted the item is removed from the queue and the users action callback
    is called passing to the callback the pointer to the now deleted item

INITIALIZATION AND SEQUENCING REQUIREMENTS

  Prior to use, a queue must be initialized by calling q_init. Similarly,
  a link must be initialized prior to use by calling q_link.

Copyright (c) 1990-2010 by Qualcomm Technologies, Inc.  All Rights Reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================
                              Edit History

$PVCSPath: O:/src/asw/COMMON/vcs/queue.c_v   1.4   02 Oct 2001 10:46:56   rajeevg  $
$Header: //source/qcom/qct/core/pkg/2H09/halcyon_modem/main/latest/AMSS/products/7x30/core/services/utils/src/queue.c#2 $ $DateTime: 2010/07/21 19:03:32 $ $Author: coresvc $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/24/09   sri     Removed NULL check in q_lock_init() and updated header
05/12/09   sri     q_delete() assign NULL to the q_delete_ptr->q_ptr only
                   after q_unlock() is done
05/04/09   sri     q_m_put() compilation error fix
04/27/09   sri     Changing the locally accessed functions to static, changing
                   the name of q_free() to q_unlock() and q_m_put modified to 
                   lock the source queue
03/31/09   sri     Implemented q_lock_delete(internal func) and q_destroy(api)
11/21/07   ejv     Fixed compiler warning.
10/30/07   hal     Fixed errors in q_linear_delete
06/11/07   enj     Fixing lint errors
12/11/06   sgh     Enabled mutex for WinMobile
01/17/06    ck     Fixed a compilation issue with a hanging brace for LTK builds
01/10/06    ck     Added function q_delete_ext that returns a boolean indicating
                   if the node was found and removed
05/20/05    ck     Fixed an issue with the function q_delete which was stuck in
                   an infinite loop when the node to be deleted was not in the
                   queue.
09/28/01    gr     Merged in changes from the MSP archive. Includes clean-up
                   of source code and comments, queue-specific mutual
                   exclusion for Win32 targets and the new function
                   q_linear_delete. Removed support for UTF_ASP.
01/25/01   day     Merged from MSM5105_COMMON.00.00.05.
                   Added support for MSM5000_IRAM_FWD.
                   Added support for UTF_ASP.
01/13/00   jct     Add q_linear_search
12/10/99   gr      Merged from MSM3000 Rel 4.0c.
                   Fixes to q_insert, q_delete.
                   Introduced q_last_get().
                   Changed type of queue count from word to int.
                   Added calls to Q_XCEPT macros in all queue functions.
04/09/99    sk     Comments changed.
04/06/99    sk     Introduced FEATURE_Q_NO_SELF_QPTR and FEATURE_Q_SINGLE_LINK
12/16/98   jct     Removed 80186 support
10/08/98   jct     Moved INTLOCK in q_put to beginning of function, added
                   setting link pointer to NULL in q_delete
09/24/98   jct     Update copyright
12/20/96   rdh     Removed from Boot Block, changed to Pascal call convention.
04/08/96   dna     Added comment to each func. saying it is in the Boot Block
02/28/96   dna     Prepended bb_ to all function names and put module in the
                   Boot Block.
04/23/92   ip      Initial porting of file from Brassboard to DMSS.
02/20/92   arh     Added q_check routine

===========================================================================*/

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */

#include "queue.h"
#include "assert.h"

#define ASSERT(X) assert(X)

#ifdef CUST_H
   #include "customer.h"
#endif

/* NOTUSED */
#define NOTUSED(i) if(i){}

/* ------------------------------------------------------------------------
** For Win32 Systems, users may want to enable mutual exclusion for the
** queue operations, this provides a mutex per queue along with locking
** and unlocking primitives.
** ------------------------------------------------------------------------ */

#if ((defined FEATURE_QUBE) || ((defined FEATURE_Q_WIN32_LOCKS) || (defined FEATURE_WINCE)) && !defined(FEATURE_WINCE_BOOTLOADER))

#if (((defined FEATURE_Q_WIN32_LOCKS) || (defined FEATURE_WINCE)) && !defined(FEATURE_WINCE_BOOTLOADER))
#include <windows.h>
#define USE_MUTEX
#endif

#ifdef FEATURE_QUBE
#include "qube.h"
#include "err.h"
#endif /* FEATURE_QUBE */


/*==========================================================================
FUNCTION Q_LOCK_INIT
DESCRIPTION
   Initializes the mutex for the given queue. It is expected that this call 
   happens only once for every queue. If calling multiple times for a given 
   queue, ensure that the mutex is deleted/freed (q_lock_delete), prior to it.
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
void q_lock_init( q_type *q_ptr )
{
  #ifdef USE_MUTEX
    q_ptr->mutex = CreateMutex( NULL, FALSE, NULL );
  #elif defined FEATURE_QUBE
   int status;
   qmutex_attr_t  qmutexattr;
   qmutexattr.type = QMUTEX_LOCAL;
   
   ASSERT( q_ptr != NULL );
   status = qmutex_create(&(q_ptr->mutex), &qmutexattr);
   ASSERT( status == EOK );
  #else
   /* Allocate space for the Critical Section object */
   //q_ptr->pkcsQueueCriticalSection = (KxCriticalSection *)malloc(sizeof(KxCriticalSection));

   /* Now initialize it */
   if( !( KxMutex_Init( &(q_ptr->pkcsQueueCriticalSection), OBJ_NAME) ) ){

           /* Huge issue -- log it, but continue */
           //ERR("Kx Init returned false!\n",0,0,0);

   }
   sectionCounter++;
#endif //USE_MUTEX

   return;
} /* END q_lock_init */

/* ==========================================================================
FUNCTION Q_LOCK_DELETE
DESCRIPTION
   Deletes the mutex for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
void q_lock_delete( q_type *q_ptr )
{
#ifdef FEATURE_QUBE
   int status;
#endif

   ASSERT( q_ptr != NULL );
   if(NULL == q_ptr->mutex)
   {
      /* error case : can the value of q_ptr->mutex be "0" ? */
      return; /* or ASSERT(0); */
   }
  #ifdef USE_MUTEX
   ASSERT ( (ReleaseMutex(q_ptr->mutex)) );
  #elif defined FEATURE_QUBE
   status = qmutex_delete (q_ptr->mutex);
   ASSERT ( status == EOK );
  #else
   /* q_ptr->pkcsQueueCriticalSection - NULL Check ? */
   KxMutex_Free ( &(q_ptr->pkcsQueueCriticalSection));
   sectionCounter--;
#endif //USE_MUTEX

   return;
}  /* END q_lock_delete */


/* ==========================================================================
FUNCTION Q_LOCK
DESCRIPTION
   Locks the mutex for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
static void q_lock( q_type *q_ptr )
{
  #ifdef USE_MUTEX
   WaitForSingleObject( q_ptr->mutex, INFINITE );
  #elif defined FEATURE_QUBE
   int status;
   ASSERT( q_ptr != NULL );
   status = qmutex_lock((qmutex_t)q_ptr->mutex);
   ASSERT( status == EOK );
  #else 
   (void)KxMutex_Lock( &(q_ptr->pkcsQueueCriticalSection) );
  #endif //USE_MUTEX
   return;
} /* END q_lock */

/* ==========================================================================
FUNCTION Q_UNLOCK
DESCRIPTION
   Frees the mutex for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
static void q_unlock( q_type *q_ptr )
{
  #ifdef USE_MUTEX
   ReleaseMutex( q_ptr->mutex );
  #elif defined FEATURE_QUBE
   int status;
   ASSERT( q_ptr != NULL );
   status = qmutex_unlock((qmutex_t)q_ptr->mutex);
   ASSERT( status == EOK );
  #else 
   KxMutex_Unlock( &(q_ptr->pkcsQueueCriticalSection) );
#endif //USE_MUTEX
  return;
} /* END q_unlock */

#else  
#ifdef FEATURE_DATA_LINUX
#define q_lock( q )         q_pthread_lock( q )
#define q_unlock( q )       q_pthread_unlock( q )
#define q_lock_init( q )    q_pthread_lock_init( q )
#define q_lock_delete( q )  q_pthread_lock_delete( q )

/*==========================================================================
FUNCTION Q_PTHREAD_LOCK_INIT

DESCRIPTION
  Pthread implementation of mutex initialize

DEPENDENCIES
  None.

RETURN VALUE
  N/A

SIDE EFFECTS
  Initialize Mutex
===========================================================================*/
static void q_pthread_lock_init(
   q_type  *q_ptr  /* Ptr to queue to be initialized. */
)
{
  pthread_mutex_init(&(q_ptr->mutex),NULL);
}

/*==========================================================================
FUNCTION Q_PTHREAD_LOCK_DELETE

DESCRIPTION
  Pthread implementation of mutex delete

DEPENDENCIES
  None.

RETURN VALUE
  N/A

SIDE EFFECTS
  Initialize Mutex
===========================================================================*/
static void q_pthread_lock_delete(
   q_type  *q_ptr  /* Ptr to queue to be initialized. */
)
{
  pthread_mutex_destroy(&(q_ptr->mutex));
}

/*==========================================================================
FUNCTION Q_PTHREAD_LOCK

DESCRIPTION
  Pthread implementation of lock

DEPENDENCIES
  q_pthread_lock_init

RETURN VALUE
  N/A.

SIDE EFFECTS
  Locks Mutex
===========================================================================*/
static void q_pthread_lock(
   q_type  *q_ptr
)
{
  pthread_mutex_lock(&(q_ptr->mutex));
}

/*==========================================================================
FUNCTION Q_PTHREAD_UNLOCK

DESCRIPTION
  Pthread implementation of unlock

DEPENDENCIES
  q_pthread_lock_init

RETURN VALUE
  N/A.

SIDE EFFECTS
  Unlocks mutex
===========================================================================*/
static void q_pthread_unlock(
   q_type  *q_ptr
)
{
  pthread_mutex_unlock(&(q_ptr->mutex));
}

#else /* FEATURE_DATA_LINUX */
   #define q_lock( q ) INTLOCK( )
   #define q_unlock( q ) INTFREE( )   
   #define q_lock_init( q )
   #define q_lock_delete( q )
#endif /* FEATURE_DATA_LINUX*/
#endif /* FEATURE_Q_WIN32_LOCKS */


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
q_type* q_init(
   q_type  *q_ptr  /* Ptr to queue to be initialized. */
)
{
   ASSERT( q_ptr != NULL );
   q_ptr->link.next_ptr = (q_link_type *)(&q_ptr->link);  /* Points to q link. */
   q_ptr->link.prev_ptr = (q_link_type *)(&q_ptr->link);  /* Points to q link. */

   #ifndef FEATURE_Q_NO_SELF_QPTR
      q_ptr->link.self_ptr = NULL;
      q_ptr->link.q_ptr    = NULL;
   #endif

   q_ptr->cnt = 0;

   #ifdef FEATURE_QUBE
   // Only call qmutex_create once.
   if ( ! q_ptr->mutex ) { //Assumes zero-init of q_type.mutex
   #endif

   q_lock_init( q_ptr );

   #ifdef FEATURE_QUBE
   }
   #endif

   Q_XCEPT_Q_INIT( q_ptr );

   return q_ptr;
} /* END q_init */

/*==========================================================================
FUNCTION Q_DESTROY

DESCRIPTION
  This function destroys a specified queue. It should be called if you
  do not require this queue anymore.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  Elements in the queue will  not be accessible through this queue 
  anymore. It is user's responsibility to deallocate the memory allocated 
  for the queue and its elements to avoid leaks
===========================================================================*/
void q_destroy(
   q_type  *q_ptr  /* Ptr to queue to be destroyed. */
)
{
   ASSERT( q_ptr != NULL );

   q_lock( q_ptr );
   q_ptr->link.next_ptr = NULL;
   q_ptr->link.prev_ptr = NULL;

   #ifndef FEATURE_Q_NO_SELF_QPTR
      q_ptr->link.self_ptr = NULL;
      q_ptr->link.q_ptr    = NULL;
   #endif

   q_ptr->cnt = 0;
   q_unlock( q_ptr );

   q_lock_delete( q_ptr );

   return;
} /* END q_destroy */


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
q_link_type* q_link(
   void         *item_ptr,  /* Ptr to item or variable containing link. */
   q_link_type  *link_ptr   /* Ptr to link field within variable. */
)
{
   ASSERT( link_ptr != NULL );
   link_ptr->next_ptr = NULL;

   #ifndef FEATURE_Q_SINGLE_LINK
      link_ptr->prev_ptr = NULL;
   #endif

   #ifndef FEATURE_Q_NO_SELF_QPTR
      link_ptr->self_ptr = item_ptr;
      link_ptr->q_ptr    = NULL;
   #endif

   NOTUSED( item_ptr ); //below macro is empty... remove this line when it's not
   Q_XCEPT_Q_LINK( item_ptr, link_ptr );

   return link_ptr;
} /* END q_link */

#ifndef MSM5000_IRAM_FWD
/*===========================================================================
FUNCTION Q_PUT

DESCRIPTION
  This function enqueues an item onto a specified queue using a specified
  link.

DEPENDENCIES
  The specified queue should have been previously initialized via a call
  to q_init. The specified link field of the item should have been prev-
  iously initialized via a call to q_init_link.

RETURN VALUE
  None.

SIDE EFFECTS
  The specified item is placed at the tail of the specified queue.
===========================================================================*/
void q_put(
   q_type       *q_ptr,    /* Ptr to queue. */
   q_link_type  *link_ptr  /* Ptr to item link to use for queueing. */
)
{
   q_lock( q_ptr );

   ASSERT( q_ptr != NULL );
   ASSERT( link_ptr != NULL );
   link_ptr->next_ptr = (q_link_type *)&q_ptr->link;

   #ifndef FEATURE_Q_NO_SELF_QPTR
      link_ptr->q_ptr    = q_ptr;
   #endif

   #ifndef FEATURE_Q_SINGLE_LINK
      link_ptr->prev_ptr = q_ptr->link.prev_ptr;

   #endif

   q_ptr->link.prev_ptr->next_ptr = link_ptr;
   q_ptr->link.prev_ptr           = link_ptr;
   q_ptr->cnt++;

   Q_XCEPT_Q_PUT( q_ptr, link_ptr );

   q_unlock( q_ptr );
   return;
} /* END q_put */
#endif /*MSM5000_IRAM_FWD*/

#ifndef MSM5000_IRAM_FWD
/*===========================================================================
FUNCTION Q_M_PUT

DESCRIPTION
  This function enqueues all items from a specified (source) queue into a 
  specified (target) queue.

DEPENDENCIES
  The specified queues should have been previously initialized via a call
  to q_init.

RETURN VALUE
  None.

SIDE EFFECTS
  The items in the specified (source) queue are placed at the tail of the 
  specified (target) queue.
===========================================================================*/
void q_m_put(
   q_type       *target_q_ptr,    /* Ptr to target queue. */
   q_type       *source_q_ptr    /* Ptr to source queue. */
)
{
   /* Head and Tail items of source queue */
   q_link_type *source_head_item, *source_tail_item;
   
   /* For source_head/tail_item */
   q_link_type  *link_ptr, *last_link_ptr;                         
#ifdef FEATURE_Q_NO_SELF_QPTR
   q_link_type  *ret_ptr = NULL;
#endif
#if defined(FEATURE_Q_SINGLE_LINK) || defined(FEATURE_Q_NO_SELF_QPTR)
   q_link_type  *last_ret_ptr=NULL;                     
#endif  /* FEATURE_Q_SINGLE_LINK || FEATURE_Q_NO_SELF_QPTR */

  q_lock( source_q_ptr );
  if (q_cnt( source_q_ptr ) != 0)
  {
   q_lock( target_q_ptr );

   /* Get tail item of source queue */
   /* source_tail_item = q_last_check( source_q_ptr ); */
#ifdef FEATURE_Q_SINGLE_LINK
   for( last_link_ptr = (q_link_type *)source_q_ptr;
        last_link_ptr->next_ptr != source_q_ptr->link.prev_ptr;
        last_link_ptr=last_link_ptr->next_ptr);

    if (source_q_ptr->cnt > 0)
     {
      last_ret_ptr = last_link_ptr->next_ptr;

#ifndef FEATURE_Q_NO_SELF_QPTR
      last_link_ptr = last_ret_ptr;
#endif
     }
#else
   last_link_ptr = source_q_ptr->link.prev_ptr;

   if ( source_q_ptr->cnt > 0 )
   {
#ifdef FEATURE_Q_NO_SELF_QPTR
     last_ret_ptr = last_link_ptr;
#endif
   }
#endif

   /* Q_XCEPT_Q_LAST_CHECK( source_q_ptr ); */

#ifdef FEATURE_Q_NO_SELF_QPTR
   source_tail_item = last_ret_ptr;
#else
   source_tail_item = last_link_ptr->self_ptr;
#endif

   ASSERT( source_tail_item != NULL );
   if( source_tail_item )
   {
     source_tail_item->next_ptr = (q_link_type *) &target_q_ptr->link;
   }

   /* Get head item of source queue */
   /* source_head_item = q_check( source_q_ptr ); */
   link_ptr = source_q_ptr->link.next_ptr;

#ifdef FEATURE_Q_NO_SELF_QPTR
   if( source_q_ptr->cnt > 0 )
   {
     ret_ptr = link_ptr;
   }
#endif
   /* Q_XCEPT_Q_CHECK( q_ptr ); */

#ifdef FEATURE_Q_NO_SELF_QPTR
   source_head_item = ret_ptr;
#else
   source_head_item = link_ptr->self_ptr;
#endif

   target_q_ptr->link.prev_ptr->next_ptr = source_head_item;
   target_q_ptr->link.prev_ptr           = source_tail_item;
   
   target_q_ptr->cnt += source_q_ptr->cnt;

   //Q_XCEPT_Q_PUT( q_ptr, link_ptr );

   q_init( source_q_ptr );
   q_unlock( target_q_ptr );
  }
  q_unlock( source_q_ptr );
  return;
} /* END q_m_put */

/*===========================================================================
FUNCTION Q_GET

DESCRIPTION
  This function removes an item from the head of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.
===========================================================================*/
void* q_get(
  q_type  *q_ptr  /* Ptr to queue. */
)
{
   q_link_type  *link_ptr;

   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_link_type  *ret_ptr = NULL;
   #endif

   ASSERT( q_ptr != NULL );
   q_lock( q_ptr );

   /* Get ptr to 1st queue item.
   */
   link_ptr = q_ptr->link.next_ptr;

   /* Can only get an item if the queue is non empty
   */
   if( q_ptr->cnt > 0 )
   {
      q_ptr->link.next_ptr = link_ptr->next_ptr;

      #ifdef FEATURE_Q_SINGLE_LINK
         if (link_ptr->next_ptr == (q_link_type *)q_ptr)
         {
            q_ptr->link.prev_ptr = (q_link_type *)(&q_ptr->link);
         }
      #else
         link_ptr->next_ptr->prev_ptr = &q_ptr->link;
      #endif

      q_ptr->cnt--;

      /* Mark item as no longer in a queue.
      */
      #ifdef FEATURE_Q_NO_SELF_QPTR
         link_ptr->next_ptr = NULL;
         ret_ptr = link_ptr;
      #else
         link_ptr->q_ptr = NULL;
      #endif
   }

   Q_XCEPT_Q_GET( q_ptr );

   q_unlock( q_ptr );

   #ifdef FEATURE_Q_NO_SELF_QPTR
      return (void *)ret_ptr;
   #else
      return link_ptr->self_ptr;
   #endif
} /* END q_get */
#endif /*MSM5000_IRAM_FWD*/

/*===========================================================================
FUNCTION Q_LAST_GET

DESCRIPTION
  This function returns the item which was most recently enqueued in a queue.

  Note, this is different from q_get() which returns the oldest item in a
  queue.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void * q_last_get(
   q_type* q_ptr
)
{
   q_link_type  *link_ptr;
   #if defined(FEATURE_Q_SINGLE_LINK) || defined(FEATURE_Q_NO_SELF_QPTR)
      q_link_type  *ret_ptr=NULL;
   #endif  /* FEATURE_Q_SINGLE_LINK || FEATURE_Q_NO_SELF_QPTR */

   ASSERT( q_ptr != NULL );
   q_lock( q_ptr );

   #ifdef FEATURE_Q_SINGLE_LINK
      for( link_ptr           =  (q_link_type *)q_ptr;
           link_ptr->next_ptr != q_ptr->link.prev_ptr;
           link_ptr           =  link_ptr->next_ptr
         );

      if( q_ptr->cnt > 0 )
      {
         ret_ptr              = link_ptr->next_ptr;
         q_ptr->link.prev_ptr = link_ptr;
         link_ptr->next_ptr   = (q_link_type *)(&q_ptr->link);
         q_ptr->cnt--;
         #ifdef FEATURE_Q_NO_SELF_QPTR
            ret_ptr->next_ptr = NULL;
         #else
            link_ptr        = ret_ptr;
            link_ptr->q_ptr = NULL;
         #endif
      }
   #else
      link_ptr = q_ptr->link.prev_ptr;

      if ( q_ptr->cnt > 0 )
      {
         q_ptr->link.prev_ptr         = link_ptr->prev_ptr;
         link_ptr->prev_ptr->next_ptr = &q_ptr->link;
         q_ptr->cnt--;

         #ifdef FEATURE_Q_NO_SELF_QPTR
            link_ptr->next_ptr = NULL;
            ret_ptr            = link_ptr;
         #else
            link_ptr->q_ptr = NULL;
         #endif
      }
   #endif

   Q_XCEPT_Q_LAST_GET( q_ptr );

   q_unlock( q_ptr );

   #ifdef FEATURE_Q_NO_SELF_QPTR
      return (void *)ret_ptr;
   #else
      return link_ptr->self_ptr;
   #endif
}  /* q_last_get */

/*===========================================================================
FUNCTION Q_NEXT

DESCRIPTION
  This function returns a pointer to the next item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  A pointer to the next item on the queue. If the end of the queue is reached,
  then NULL is returned.

SIDE EFFECTS
  None.
===========================================================================*/
void* q_next(
   q_type       *q_ptr,
   q_link_type  *q_link_ptr
)
{
   void       *q_temp_ptr = NULL;
   ASSERT( q_link_ptr != NULL );
   q_lock( q_ptr );
   Q_XCEPT_Q_NEXT( q_ptr, q_link_ptr );
   if( (void *) q_link_ptr->next_ptr != (void *) q_ptr )
   {
     q_temp_ptr = q_link_ptr->next_ptr;
   }
   q_unlock( q_ptr );
   return q_temp_ptr;
} /* END q_next */


/*===========================================================================
FUNCTION Q_INSERT

DESCRIPTION
  This function inserts an item before a specified item on a queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is inserted before input item.
===========================================================================*/
void q_insert(
   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_type    *q_ptr,          /* Ptr to the queue */
   #endif
   q_link_type  *q_insert_ptr,   /* Ptr to link of item to insert */
   q_link_type  *q_item_ptr      /* Ptr to link item to insert before */
)
{
   #ifdef FEATURE_Q_SINGLE_LINK
      q_link_type  *link_ptr;
   #endif

   ASSERT( q_ptr        != NULL );
   ASSERT( q_insert_ptr != NULL );
   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_lock( q_ptr );
   #else
      q_lock( q_item_ptr->q_ptr );
   #endif

   q_insert_ptr->next_ptr = q_item_ptr;

   #ifdef FEATURE_Q_SINGLE_LINK
      /* Start at beginning of queue and find the item that will be before the
      ** new item
      */
      #ifdef FEATURE_Q_NO_SELF_QPTR
         link_ptr = (q_link_type *) q_ptr;
      #else
         link_ptr = (q_link_type *) q_item_ptr->q_ptr;
      #endif

      ASSERT( link_ptr != NULL );
      while (link_ptr->next_ptr != q_item_ptr)
      {
         link_ptr = link_ptr->next_ptr;
         ASSERT( link_ptr != NULL );
      }
      link_ptr->next_ptr = q_insert_ptr;

   #else
      q_insert_ptr->prev_ptr = q_item_ptr->prev_ptr;
      q_item_ptr->prev_ptr->next_ptr = q_insert_ptr;
      q_item_ptr->prev_ptr = q_insert_ptr;
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_ptr->cnt++;
   #else
      q_insert_ptr->q_ptr = q_item_ptr->q_ptr;
      q_item_ptr->q_ptr->cnt++;
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
      Q_XCEPT_Q_INSERT( q_ptr, q_insert_ptr, q_item_ptr );
   #else
      Q_XCEPT_Q_INSERT( q_insert_ptr, q_item_ptr );
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_unlock( q_ptr );
   #else
      q_unlock( q_item_ptr->q_ptr );
   #endif
   return;
} /* END q_insert */

/*===========================================================================
FUNCTION Q_DELETE

DESCRIPTION
  This function removes an item from a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is delete from the queue.
===========================================================================*/
void q_delete(
   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_type       *q_ptr,         /* Ptr to the Queue */
   #endif
      q_link_type  *q_delete_ptr   /* Ptr to link of item to delete */
)
{
   #ifdef FEATURE_Q_SINGLE_LINK
      q_link_type *link_ptr;
      q_type *real_q_ptr;
      int qcount;
   #endif

   ASSERT( q_ptr        != NULL );
   ASSERT( q_delete_ptr != NULL );
   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_lock( q_ptr );
   #else
      q_lock( q_delete_ptr->q_ptr );
   #endif

   #ifdef FEATURE_Q_SINGLE_LINK
      #ifdef FEATURE_Q_NO_SELF_QPTR
         real_q_ptr = q_ptr;
      #else
         real_q_ptr = q_delete_ptr->q_ptr;
      #endif

      ASSERT( real_q_ptr != NULL );
      for( qcount = q_ptr->cnt,
           link_ptr           =  (q_link_type *) real_q_ptr;
           link_ptr->next_ptr != q_delete_ptr && qcount > 0;
           link_ptr           =  link_ptr->next_ptr, qcount--);

      ASSERT( link_ptr != NULL );
      if(qcount > 0)
      {
        link_ptr->next_ptr = q_delete_ptr->next_ptr;

        if(link_ptr->next_ptr == (q_link_type *) real_q_ptr)
        {
          real_q_ptr->link.prev_ptr = link_ptr;
        }
   #else
        q_delete_ptr->prev_ptr->next_ptr = q_delete_ptr->next_ptr;
        q_delete_ptr->next_ptr->prev_ptr = q_delete_ptr->prev_ptr;
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
        q_ptr->cnt--;
        q_delete_ptr->next_ptr = NULL;
   #else
        q_delete_ptr->q_ptr->cnt--;
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
        Q_XCEPT_Q_DELETE( q_ptr, q_delete_ptr );
   #else
        Q_XCEPT_Q_DELETE( q_delete_ptr );
   #endif
   #ifdef FEATURE_Q_SINGLE_LINK
      }
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_unlock( q_ptr );
   #else
      q_unlock( q_delete_ptr->q_ptr );
      /* make null after unlock */
      q_delete_ptr->q_ptr = NULL;
   #endif
   return;
} /* END q_delete */

/*===========================================================================
FUNCTION Q_DELETE_EXT

DESCRIPTION
  This function removes an item from a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  FALSE : if the item is not found in the queue.
  TRUE  : if the item is found and removed from the queue.

SIDE EFFECTS
  Input item is deleted from the queue.
===========================================================================*/
boolean q_delete_ext(
   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_type       *q_ptr,         /* Ptr to the Queue */
   #endif
      q_link_type  *q_delete_ptr   /* Ptr to link of item to delete */
)
{
   #ifdef FEATURE_Q_SINGLE_LINK
      q_link_type *link_ptr;
      q_type *real_q_ptr;
      int qcount;
   #endif

   boolean item_in_q = FALSE;

   ASSERT( q_ptr        != NULL );
   ASSERT( q_delete_ptr != NULL );
   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_lock( q_ptr );
   #else
      q_lock( q_delete_ptr->q_ptr );
   #endif

   #ifdef FEATURE_Q_SINGLE_LINK
      #ifdef FEATURE_Q_NO_SELF_QPTR
         real_q_ptr = q_ptr;
      #else
         real_q_ptr = q_delete_ptr->q_ptr;
      #endif

      ASSERT( real_q_ptr != NULL );
      for( qcount = q_ptr->cnt,
           link_ptr           =  (q_link_type *) real_q_ptr;
           link_ptr->next_ptr != q_delete_ptr && qcount > 0;
           link_ptr           =  link_ptr->next_ptr, qcount--);

      ASSERT( link_ptr != NULL );
      if(qcount > 0)
      {
        link_ptr->next_ptr = q_delete_ptr->next_ptr;

        if(link_ptr->next_ptr == (q_link_type *) real_q_ptr)
        {
          real_q_ptr->link.prev_ptr = link_ptr;
        }
   #else
        q_delete_ptr->prev_ptr->next_ptr = q_delete_ptr->next_ptr;
        q_delete_ptr->next_ptr->prev_ptr = q_delete_ptr->prev_ptr;
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
        q_ptr->cnt--;
        q_delete_ptr->next_ptr = NULL;
   #else
        q_delete_ptr->q_ptr->cnt--;
        q_delete_ptr->q_ptr = NULL;
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
        Q_XCEPT_Q_DELETE( q_ptr, q_delete_ptr );
   #else
        Q_XCEPT_Q_DELETE( q_delete_ptr );
   #endif
        item_in_q = TRUE;
   #ifdef FEATURE_Q_SINGLE_LINK
      }
   #endif

   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_unlock( q_ptr );
   #else
      q_unlock( q_delete_ptr->q_ptr );
   #endif
   return item_in_q;
} /* END q_delete_ext */

/*===========================================================================
FUNCTION Q_CHECK

DESCRIPTION
  This function returns a pointer to the data block at the head of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  None
===========================================================================*/
void* q_check(
  q_type  *q_ptr
)
{
   q_link_type  *link_ptr;

   #ifdef FEATURE_Q_NO_SELF_QPTR
      q_link_type  *ret_ptr = NULL;
   #endif

   ASSERT( q_ptr != NULL );
   q_lock( q_ptr );

   link_ptr = q_ptr->link.next_ptr;

   #ifdef FEATURE_Q_NO_SELF_QPTR
      if( q_ptr->cnt > 0 )
      {
         ret_ptr = link_ptr;
      }
   #endif

   Q_XCEPT_Q_CHECK( q_ptr );

   q_unlock( q_ptr );

   #ifdef FEATURE_Q_NO_SELF_QPTR
      return (void *)ret_ptr;
   #else
      return link_ptr->self_ptr;
   #endif
} /* END q_check */


/*===========================================================================

FUNCTION Q_LAST_CHECK

DESCRIPTION
  This function returns the item which was most recently enqueued in a queue.

  Note, this is different from q_check() which returns the oldest item in a
  queue.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void * q_last_check
(
  q_type* q_ptr           /* The queue from which the item will be removed */
)
{
   q_link_type  *link_ptr;                         /* For returning value. */
#if defined(FEATURE_Q_SINGLE_LINK) || defined(FEATURE_Q_NO_SELF_QPTR)
   q_link_type  *ret_ptr=NULL;                     /* For returning value. */
#endif  /* FEATURE_Q_SINGLE_LINK || FEATURE_Q_NO_SELF_QPTR */

 /*- - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - -*/

   ASSERT( q_ptr != NULL );
   q_lock( q_ptr );

   link_ptr = q_ptr->link.prev_ptr;

   if ( q_ptr->cnt > 0 )
   {
#ifdef FEATURE_Q_NO_SELF_QPTR
     ret_ptr = link_ptr;
#endif
   }

   Q_XCEPT_Q_LAST_CHECK( q_ptr );

   q_unlock( q_ptr );

#ifdef FEATURE_Q_NO_SELF_QPTR
   return  (void *)ret_ptr;
#else
   return ( link_ptr->self_ptr );
#endif
}  /* q_last_check */


/*===========================================================================
FUNCTION Q_CNT

DESCRIPTION
  This function returns the number of items currently queued on a specified
  queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  The number of items currently queued on the specified queue.

SIDE EFFECTS
  None.
===========================================================================*/
int q_cnt(
  q_type  *q_ptr
)
{
   ASSERT( q_ptr != NULL );
   Q_XCEPT_Q_CNT( q_ptr );
   return q_ptr->cnt;
} /* END q_cnt */


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
  to q_init.

  The user's queue elements must have q_link_type as the first element
  of the queued structure.

RETURN VALUE
  A pointer to the found element

SIDE EFFECTS
  None.
===========================================================================*/
void* q_linear_search(
   q_type             *q_ptr,
   q_compare_func_type compare_func,
   void               *compare_val
)
{
   q_generic_item_type *item_ptr = NULL;

   ASSERT( compare_func != NULL );
   item_ptr = (q_generic_item_type*)q_check( q_ptr );

   while( item_ptr != NULL )
   {
      if( compare_func( item_ptr, compare_val ) != 0 )
      {
         return item_ptr;
      }
      item_ptr = (q_generic_item_type*)q_next( q_ptr, &item_ptr->link );
   } /* END while traversing the queue */

   return NULL;
} /* END q_linear_search */


/*===========================================================================
FUNCTION Q_LINEAR_DELETE_NEW

DESCRIPTION
  Given a comparison function, this function traverses the elements in
  a queue, calls the compare function, and returns a pointer to the
  current element being compared if the user passed compare function
  returns non zero.  In addition, the item will be removed from the queue.

  The user compare function should return 0 if the current element is
  not the element in which the compare function is interested.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

  The user's queue elements must have q_link_type as the first element
  of the queued structure.

RETURN VALUE
  A pointer to the found element or NULL otherwise

SIDE EFFECTS
  If the item is found, it will be deleted from the queue
===========================================================================*/
void *q_linear_delete_new(
   q_type             *q_ptr,
   q_compare_func_type compare_func,
   void               *compare_val,
   q_action_func_type  action_func,
   void               *param
)
{
   q_generic_item_type *item_ptr = NULL;
      /* Used in the traversal to point to the current item
      */
   q_generic_item_type *prev_ptr = NULL;
      /* Used in the traversal to point to the item previous to
      ** the current item.  This makes removing the current item
      ** a constant time operation
      */

   /* User must provide a compare function, otherwise, this is
   ** meaningless. Likewise, this function is useless if there
   ** is no q_ptr
   */
   if( compare_func == NULL || q_ptr == NULL )
   {
      return NULL;
   }

   q_lock( q_ptr );

   /* item_ptr points to the first item on the list
   */
   item_ptr = (q_generic_item_type*)q_check( q_ptr );
   prev_ptr = NULL;

   while( item_ptr != NULL )
   {
      if( compare_func( item_ptr, compare_val ) != 0 )
      {
         /* Remove the item
         */
         if( prev_ptr != NULL )
         {
            /* Remove from the middle or tail
            */
            prev_ptr->link.next_ptr = item_ptr->link.next_ptr;
            item_ptr->link.next_ptr = NULL;

            /* Reduce the count by one
            */
            q_ptr->cnt--;
         }
         else
         {
            /* Remove from the head
            */
            item_ptr = q_get( q_ptr );
         }

         /* Call the action function if there is one
         */
         if( action_func )
         {
            action_func( item_ptr, param );
         }
         break;
      }
      else
      {
      /* Move on to the next item
      */
      prev_ptr = item_ptr;
      item_ptr = (q_generic_item_type*)q_next( q_ptr, &item_ptr->link );
      }
      
   } /* END while traversing the queue */

   q_unlock( q_ptr );
   return item_ptr;

} /* END q_linear_delete_new */

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
  to q_init.

  The user's queue elements must have q_link_type as the first element
  of the queued structure.

  The user's compare function will be passed NULL for the compare value.

RETURN VALUE
  None

SIDE EFFECTS
  None.
===========================================================================*/
void q_linear_delete(
   q_type             *q_ptr,
   q_compare_func_type compare_func,
   void               *param,
   q_action_func_type  action_func
)
{
   q_generic_item_type *item_ptr = NULL;
      /* Used in the traversal to point to the current item
      */
   q_generic_item_type *prev_ptr = NULL;
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

   q_lock( q_ptr );

   /* item_ptr points to the first item on the list
   */
   item_ptr = (q_generic_item_type*)q_check( q_ptr );
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
            q_ptr->cnt--;

            if( q_ptr->cnt == 1){

               q_ptr->link.prev_ptr = q_ptr->link.next_ptr;

            }
         }
         else
         {
            /* Remove from the head
            */
            q_get( q_ptr );
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
      item_ptr = (q_generic_item_type*)q_next( q_ptr, &item_ptr->link );
   } /* END while traversing the queue */

   q_unlock( q_ptr );
   return;

} /* END q_linear_delete */


