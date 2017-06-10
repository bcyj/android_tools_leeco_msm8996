#ifndef STREAMQUEUE_H
#define STREAMQUEUE_H
/*===========================================================================

            Q U E U E    S E R V I C E S    H E A D E R    F I L E

DESCRIPTION
 This file contains types and declarations associated with the Queue
 Services.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$PVCSPath: O:/src/asw/COMMON/vcs/queue.h_v   1.3   16 May 2002 15:43:00   rajeevg  $
$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/StreamQueue.h#7 $ $DateTime: 2012/03/20 07:46:30 $ $Author: kamit $

===========================================================================*/


/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include "AEEStdDef.h"
#include "MMCriticalSection.h"


/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

class StreamQ_type;
/* -------------------------------------------------------------------------
** Queue Link Structure
**
** The following link structure is really the heart of the Queue Services.
** It is used as a link field with variables which allows them to be moved
** on and off queues. It is also used in the definition of queues themselves.
**
** Do NOT directly access the fields of a link! Only the Queue Services should
** do this.
** ------------------------------------------------------------------------- */
typedef struct StreamQ_link_struct
{
  struct StreamQ_link_struct  *next_ptr;
    /* Ptr to next link in list. If NULL, there is no next link. */


#ifndef FEATURE_QTVQUEUE_NO_SELF_QPTR

  void                  *self_ptr;
    /* Ptr to item which contains this link. */

  StreamQ_type                *StreamQ_ptr;
    /* Ptr to the queue on which this item is enqueued. NULL if the
       item is not on a queue. While not strictly necessary, this field
       is helpful for debugging.*/
#endif

#ifndef FEATURE_QTVQUEUE_SINGLE_LINK

  struct StreamQ_link_struct  *prev_ptr;

    /* Ptr to prev link in list. If NULL, there is no prev link. */
#endif

} StreamQ_link_type;

/* ------------------------------------------------------------------------
** Queue Head Link Structure
**
** When queue items are linked in a singly link list, StreamQ_head_link_type is
** used instead of StreamQ_link_type. This avoids the overhead of traversing
** the whole of link list when queueing at the end of the link list.
** This structure should be accessed only by Queue services.
** ------------------------------------------------------------------------ */
typedef struct StreamQ_head_link_struct
{
  struct StreamQ_link_struct  *next_ptr;
    /* Ptr to head of the link list */

#ifndef FEATURE_QTVQUEUE_NO_SELF_QPTR

  void                  *self_ptr;
    /* Ptr to item which contains this link. */

  struct StreamQ_struct       *StreamQ_ptr;
    /* Ptr to the queue on which this item is enqueued. NULL if the
       item is not on a queue. While not strictly necessary, this field
       is helpful for debugging.*/
#endif

  struct StreamQ_link_struct  *prev_ptr;
    /* Ptr to the tail of link list */

} StreamQ_head_link_type;


/* ------------------------------------------------------------------------
** Queue Structure
**
** The following structure format is used by the Queue Services to represent
** a queue.
**
** Do NOT access the fields of a queue directly. Only the Queue Services should
** do this.
** ------------------------------------------------------------------------ */
class StreamQ_type
{
public:
#ifdef FEATURE_QTVQUEUE_SINGLE_LINK
  StreamQ_head_link_type  link;
#else
  StreamQ_link_type  link;
#endif
    /* Used for linking items into queue. */

  int          cnt;
    /* Keeps track of number of items enqueued. Though not necessary, having
       this field is tremendously helpful for debugging. */

  MM_HANDLE   lock;

  StreamQ_type() : lock(0) { }
  ~StreamQ_type() { if (lock) MM_CriticalSection_Release(lock); lock = 0; }
} ;

/* ------------------------------------------------------------------------
** Queue Generic Item
**   Generic items must have StreamQ_link_type as the first element.  This allows
**   the linear search function to traverse the list without knowing
**   anything about the elements
** ------------------------------------------------------------------------ */
typedef struct {
   StreamQ_link_type link;
} StreamQ_generic_item_type;

/* ------------------------------------------------------------------------
** Queue Compare Function
**    Used by the searching functions to determine if an item is in
**       the queue.
**    Returns non zero if the element should be operated upon, 0 otherwise
**    For linear searching, the operation is to return a pointer to the
**       item and terminate the search
**    For linear deleting, the operation is to remove the item from the
**       queue and continue the traversal
** ------------------------------------------------------------------------ */
typedef int (*StreamQ_compare_func_type)( void* item_ptr, void* compare_val );

/* ------------------------------------------------------------------------
** Queue Action Function
**    Used by StreamQ_linear_delete to perform an action on an item which is
**    being deleted from the list AFTER the item is deleted.  To perform
**    an action BEFORE the item is deleted, the user can call the action
**    function directly in the compare function call back.
** ------------------------------------------------------------------------ */
typedef void (*StreamQ_action_func_type)( void *item_ptr, void* param );

/*===========================================================================

                             Macro Definitions

===========================================================================*/
/* ========================================================================
MACRO Q_ALREADY_QUEUED
DESCRIPTION
   Evaluates to true if the item passed in is already in a queue and to
   false otherwise.
=========================================================================== */
#define STREAMQ_ALREADY_QUEUED( StreamQ_link_ptr ) \
   ((StreamQ_link_ptr)->next_ptr != NULL)

/*===========================================================================

                            Function Declarations

===========================================================================*/
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
StreamQ_type* StreamQ_init ( StreamQ_type *StreamQ_ptr, MM_HANDLE critSect = NULL);

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
StreamQ_link_type* StreamQ_link ( void *item_ptr, StreamQ_link_type *link_ptr );


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
void  StreamQ_put  ( StreamQ_type *StreamQ_ptr, StreamQ_link_type *link_ptr );


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
void* StreamQ_get ( StreamQ_type *StreamQ_ptr );

/*===========================================================================

FUNCTION Q_LAST_GET

DESCRIPTION
  This function removes an item from the tail of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* StreamQ_last_get ( StreamQ_type *StreamQ_ptr );



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
int StreamQ_cnt  ( StreamQ_type *StreamQ_ptr );


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
void* StreamQ_check (StreamQ_type  *StreamQ_ptr);


/*===========================================================================

FUNCTION Q_LAST_CHECK

DESCRIPTION
  This function returns a pointer to the data block at the tail of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to StreamQ_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* StreamQ_last_check ( StreamQ_type *StreamQ_ptr );


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
void* StreamQ_next  ( StreamQ_type *StreamQ_ptr, StreamQ_link_type *link_ptr );

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
void* StreamQ_prev ( StreamQ_type *StreamQ_ptr, StreamQ_link_type *link_ptr );

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
#ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
   void StreamQ_insert  ( StreamQ_type *StreamQ_ptr, StreamQ_link_type *StreamQ_insert_ptr, StreamQ_link_type *StreamQ_item_ptr );
#else
   void StreamQ_insert  ( StreamQ_link_type *StreamQ_insert_ptr, StreamQ_link_type *StreamQ_item_ptr );
#endif


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
#ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
   void StreamQ_delete  ( StreamQ_type *StreamQ_ptr, StreamQ_link_type *StreamQ_delete_ptr );
#else
   void StreamQ_delete  ( StreamQ_link_type *StreamQ_delete_ptr );
#endif

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
#ifdef FEATURE_QTVQUEUE_NO_SELF_QPTR
   boolean StreamQ_delete_ext  ( StreamQ_type *StreamQ_ptr, StreamQ_link_type *StreamQ_delete_ptr );
#else
   boolean StreamQ_delete_ext  ( StreamQ_link_type *StreamQ_delete_ptr );
#endif

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
);

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
);

#endif

#endif /* STREAMQUEUE_H */
