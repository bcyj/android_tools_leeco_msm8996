#ifndef QUEUE_H
#define QUEUE_H
/*===========================================================================

            Q U E U E    S E R V I C E S    H E A D E R    F I L E

DESCRIPTION
 This file contains types and declarations associated with the Queue
 Services.

Copyright (c) 1990,1991,1992 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 1993,1994,1995 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 1996,1997,1998 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 1999,2000,2001 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 2002-2009,2014 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/

/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

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
typedef struct sns_q_link_s
{
  struct sns_q_link_s  *next_ptr;
    /* Ptr to next link in list. If NULL, there is no next link. */

  void                  *self_ptr;
    /* Ptr to item which contains this link. */

  struct sns_q_s        *q_ptr;
    /* Ptr to the queue on which this item is enqueued. NULL if the
       item is not on a queue. While not strictly necessary, this field
       is helpful for debugging.*/

  struct sns_q_link_s  *prev_ptr;
} sns_q_link_s;

  /*These are HTORPCMETA comments, Do not delete this.*/
  /*~ FIELD q_link_struct.next_ptr VOID */
  /*~ FIELD q_link_struct.prev_ptr VOID */


/* ------------------------------------------------------------------------
** Queue Head Link Structure
**
** When queue items are linked in a singly link list, q_head_link_s is
** used instead of sns_q_link_s. This avoids the overhead of traversing
** the whole of link list when queueing at the end of the link list.
** This structure should be accessed only by Queue services.
** ------------------------------------------------------------------------ */
typedef sns_q_link_s q_head_link_s;

/* ------------------------------------------------------------------------
** Queue Structure
**
** The following structure format is used by the Queue Services to represent
** a queue.
**
** Do NOT access the fields of a queue directly. Only the Queue Services should
** do this.
** ------------------------------------------------------------------------ */
typedef struct sns_q_s
{
  q_head_link_s  link;

    /* Used for linking items into queue. */

  int          cnt;
    /* Keeps track of number of items enqueued. Though not necessary, having
       this field is tremendously helpful for debugging. */
} sns_q_s;

/* ------------------------------------------------------------------------
** Queue Generic Item
**   Generic items must have sns_q_link_s as the first element.  This allows
**   the linear search function to traverse the list without knowing
**   anything about the elements
** ------------------------------------------------------------------------ */
typedef struct {
   sns_q_link_s link;
} q_generic_item_s;

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
typedef int (*q_compare_func_t)( void* item_ptr, void* compare_val );

/* ------------------------------------------------------------------------
** Queue Action Function
**    Used by sns_q_linear_delete to perform an action on an item which is
**    being deleted from the list AFTER the item is deleted.  To perform
**    an action BEFORE the item is deleted, the user can call the action
**    function directly in the compare function call back.
** ------------------------------------------------------------------------ */
typedef void (*q_action_func_t)( void *item_ptr, void* param );

/*===========================================================================

                             Macro Definitions

===========================================================================*/


/* ========================================================================
MACRO Q_ALREADY_QUEUED
DESCRIPTION
   Evaluates to true if the item passed in is already in a queue and to
   false otherwise.
=========================================================================== */
#define Q_ALREADY_QUEUED( q_link_ptr ) \
   ((q_link_ptr)->q_ptr != NULL)

/*===========================================================================

                            Function Declarations

===========================================================================*/
#ifdef __cplusplus
   extern "C" {
#endif

/*==========================================================================

FUNCTION sns_q_init

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
sns_q_s* sns_q_init ( sns_q_s *q_ptr );


/*===========================================================================

FUNCTION sns_q_link

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
sns_q_link_s* sns_q_link ( void *item_ptr, sns_q_link_s *link_ptr );


/*===========================================================================

FUNCTION sns_q_put

DESCRIPTION
  This function enqueues an item onto a specified queue using a specified
  link.

DEPENDENCIES
  The specified queue should have been previously initialized via a call
  to sns_q_init. The specified link field of the item should have been prev-
  iously initialized via a call to q_init_link.

RETURN VALUE
  None.

SIDE EFFECTS
  The specified item is placed at the tail of the specified queue.

===========================================================================*/
void  sns_q_put  ( sns_q_s *q_ptr, sns_q_link_s *link_ptr );


/*===========================================================================

FUNCTION sns_q_get

DESCRIPTION
  This function removes an item from the head of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* sns_q_get ( sns_q_s *q_ptr );

/*===========================================================================

FUNCTION sns_q_last_get

DESCRIPTION
  This function removes an item from the tail of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* sns_q_last_get ( sns_q_s *q_ptr );



/*===========================================================================

FUNCTION sns_q_cnt

DESCRIPTION
  This function returns the number of items currently queued on a specified
  queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  The number of items currently queued on the specified queue.

SIDE EFFECTS
  None.

===========================================================================*/
int sns_q_cnt  ( sns_q_s const *q_ptr );


/*===========================================================================

FUNCTION sns_q_check

DESCRIPTION
  This function returns a pointer to the data block at the head of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  None

===========================================================================*/
void* sns_q_check (sns_q_s const *q_ptr);


/*===========================================================================

FUNCTION sns_q_last_check

DESCRIPTION
  This function returns a pointer to the data block at the tail of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* sns_q_last_check ( sns_q_s const *q_ptr );


/*===========================================================================

FUNCTION sns_q_next

DESCRIPTION
  This function returns a pointer to the next item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  A pointer to the next item on the queue. If the end of the queue is reached,
  then NULL is returned.

SIDE EFFECTS
  None.

===========================================================================*/
void* sns_q_next  ( sns_q_s const *q_ptr, sns_q_link_s const *link_ptr );

/*===========================================================================

FUNCTION sns_q_prev

DESCRIPTION
  This function returns a pointer to the previous item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  A pointer to the previous item on the queue. If the beginning of the queue
  is reached, then NULL is returned.

SIDE EFFECTS
  None.

===========================================================================*/
void* sns_q_prev  ( sns_q_s const *q_ptr, sns_q_link_s const *link_ptr );

/*===========================================================================

FUNCTION sns_q_insert

DESCRIPTION
  This function inserts an item before a specified item on a queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Item q_insert_ptr is inserted before item q_item_ptr.

===========================================================================*/
void sns_q_insert  ( sns_q_link_s *q_insert_ptr, sns_q_link_s *q_item_ptr );

/*===========================================================================

FUNCTION sns_q_insert_after

DESCRIPTION
  This function inserts an item after a specified item on a queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Item q_insert_ptr is inserted after q_item_ptr.

===========================================================================*/
void sns_q_insert_after  ( sns_q_link_s *q_insert_ptr, sns_q_link_s *q_item_ptr );

/*===========================================================================

FUNCTION sns_q_delete

DESCRIPTION
  This function removes an item from a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is delete from the queue.

===========================================================================*/
void sns_q_delete  ( sns_q_link_s *q_delete_ptr );

/*===========================================================================

FUNCTION sns_q_delete_ext
DESCRIPTION
  This function removes an item from a specified queue.
DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.
RETURN VALUE
  FALSE : if the item is not found in the queue.
  TRUE  : if the item is found and removed from the queue.
SIDE EFFECTS
  Input item is deleted from the queue.
===========================================================================*/

//boolean sns_q_delete_ext  ( sns_q_s *q_ptr, sns_q_link_s *q_delete_ptr );

/*===========================================================================

FUNCTION sns_q_linear_search

DESCRIPTION
  Given a comparison function, this function traverses the elements in
  a queue, calls the compare function, and returns a pointer to the
  current element being compared if the user passed compare function
  returns non zero.

  The user compare function should return 0 if the current element is
  not the element in which the compare function is interested.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

  The user's queue elements must have sns_q_link_s as the first element
  of the queued structure.

RETURN VALUE
  A pointer to the found element

SIDE EFFECTS
  None.

===========================================================================*/
void* sns_q_linear_search(
  sns_q_s const    *q_ptr,
  q_compare_func_t compare_func,
  void             *compare_val
);

/*===========================================================================

FUNCTION sns_q_linear_delete

DESCRIPTION
  Given a comparison function, this function traverses the elements in
  a queue, calls the compare function, and returns a pointer to the
  current element being compared if the user passed compare function
  returns non zero.  In addition, the item will be removed from the queue.

  The user compare function should return 0 if the current element is
  not the element in which the compare function is interested.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to sns_q_init.

  The user's queue elements must have sns_q_link_s as the first element
  of the queued structure.

  The user's compare function will be passed NULL for the compare value.

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/
void sns_q_linear_delete(
  sns_q_s          *q_ptr,
  q_compare_func_t compare_func,
  void             *param,
  q_action_func_t  action_func
);

#ifdef __cplusplus
   }
#endif

#endif /* QUEUE_H */
