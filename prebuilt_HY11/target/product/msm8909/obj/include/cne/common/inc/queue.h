#ifndef QUEUE_H
#define QUEUE_H
/*===========================================================================

            Q U E U E    S E R V I C E S    H E A D E R    F I L E

DESCRIPTION
 This file contains types and declarations associated with the Queue
 Services.

Copyright (c) 1990-2013 by Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$PVCSPath: O:/src/asw/COMMON/vcs/queue.h_v   1.3   16 May 2002 15:43:00   rajeevg  $
$Header: //depot/asic/msmshared/services/common/queue.h#21 $ $DateTime: 2009/07/12 23:57:00 $ $Author: gsrikant $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/03/09   sri     Feature FEATURE_Q_SINGLE_LINK cleanup
03/31/09   sri     Implemented q_destroy(api)
10/30/07   hal     Fixed errors in q_linear_delete
12/11/06   sgh     Enabled mutex for WinMobile
01/10/06    ck     Added prototype for q_delete_ext function that returns a
                   boolean
05/16/02    gr     Added macro Q_ALREADY_QUEUED.
09/28/01    gr     Merged in changes from the MSP archive. Includes
                   queue-specific mutual exclusion for Win32 targets and the
                   new function q_linear_delete. Removed support for UTF_ASP.
01/25/01   day     Merged from MSM5105_COMMON.00.00.05.
                   Added support for MSM5000_IRAM_FWD.
                   Added support for UTF_ASP.
01/13/00   jct     Add q_linear_search
12/10/99   gr      Merged from the MSM3000 Rel 4.0c.
                   Re-arranged data in link structures to fix bugs that were
                   occurring when FEATURE_Q_SINGLE_LINK was defined but
                   FEATURE_Q_NO_SELF_QPTR was not.
                   Introduced q_last_get().
                   Added extern "C" to allow this file to export the queue
                   routines to a C++ program.  Changed use of word to int to
                   count the number of elements in a queue.  Upgraded comments
                   to include function headers.  Removed the inclusion of
                   comdef.h since it is no longer needed.
                   Added macros to support renaming of queue functions and
                   to intercept calls to queue functions.
04/09/99    sk     Comments changed.
04/06/99    sk     Introduced FEATURE_Q_NO_SELF_QPTR and FEATURE_Q_SINGLE_LINK
12/16/98   jct     Removed 80186 support
09/24/98   jct     Updated copyright
12/20/96   rdh     Changed to Pascal call convention.
02/08/96   jca     Added missing #include.
04/22/92   ip      Initial porting of file from Brassboard to DMSS.
02/20/92   arh     Added new function q_check.

===========================================================================*/


/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#ifdef CUST_H
  #include "customer.h"
#endif

#include "comdef.h"
#ifdef FEATURE_WINCE
   #include "KxMutex.h"
#include "windows.h"
#elif (defined (FEATURE_QUBE))
#include "qube.h"
#endif

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
typedef struct q_link_struct
{
  struct q_link_struct  *next_ptr;
    /* Ptr to next link in list. If NULL, there is no next link. */

#ifndef FEATURE_Q_NO_SELF_QPTR
  void                  *self_ptr;
    /* Ptr to item which contains this link. */

  struct q_struct       *q_ptr;
    /* Ptr to the queue on which this item is enqueued. NULL if the
       item is not on a queue. While not strictly necessary, this field
       is helpful for debugging.*/
#endif /* FEATURE_Q_NO_SELF_QPTR */

  struct q_link_struct  *prev_ptr;
    /* Ptr to prev link in list. If NULL, there is no prev link. */
} q_link_type;
#ifdef FEATURE_HTORPC_METACOMMENTS
  /*~ FIELD q_link_struct.next_ptr VOID */
  #ifndef FEATURE_Q_NO_SELF_QPTR
    /*~ FIELD q_link_struct.q_ptr VOID */
  #endif
    /*~ FIELD q_link_struct.prev_ptr VOID */
#endif /* FEATURE_HTORPC_METACOMMENTS */

#define q_head_link_type q_link_type

/* ------------------------------------------------------------------------
** Queue Structure
**
** The following structure format is used by the Queue Services to represent
** a queue.
**
** Do NOT access the fields of a queue directly. Only the Queue Services should
** do this.
** ------------------------------------------------------------------------ */
typedef struct q_struct
{
  q_link_type  link;
    /* Used for linking items into queue. */

  int          cnt;
    /* Keeps track of number of items enqueued. Though not necessary, having
       this field is tremendously helpful for debugging. */

#ifdef FEATURE_Q_WIN32_LOCKS
  void        *mutex;
#endif

#ifdef FEATURE_QUBE
  qmutex_t     mutex;
#endif /* FEATURE_QUBE */

#ifdef FEATURE_WINCE
		KxMutex pkcsQueueCriticalSection;
HANDLE mutex;
#endif /* FEATURE_WINCE */
} q_type;

/* ------------------------------------------------------------------------
** Queue Generic Item
**   Generic items must have q_link_type as the first element.  This allows
**   the linear search function to traverse the list without knowing
**   anything about the elements
** ------------------------------------------------------------------------ */
typedef struct {
   q_link_type link;
} q_generic_item_type;

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
typedef int (*q_compare_func_type)( void* item_ptr, void* compare_val );

/* ------------------------------------------------------------------------
** Queue Action Function
**    Used by q_linear_delete to perform an action on an item which is
**    being deleted from the list AFTER the item is deleted.  To perform
**    an action BEFORE the item is deleted, the user can call the action
**    function directly in the compare function call back.
** ------------------------------------------------------------------------ */
typedef void (*q_action_func_type)( void *item_ptr, void* param );

/*===========================================================================

                             Macro Definitions

===========================================================================*/

#ifdef PC_EMULATOR_H
  #ifdef Q_RENAME
    #define PC_EMULATOR_QUEUE
    #include PC_EMULATOR_H
  #endif

  #ifdef Q_XCEPT
    #define PC_EMULATOR_Q_XCEPT
    #include PC_EMULATOR_H
  #endif
#endif

#if !defined(PC_EMULATOR_H) || !defined(Q_XCEPT)
  #define Q_XCEPT_Q_INIT( q_ptr )
  #define Q_XCEPT_Q_LINK( q_ptr, link_ptr )
  #define Q_XCEPT_Q_PUT( q_ptr, link_ptr )
  #define Q_XCEPT_Q_GET( q_ptr )
  #define Q_XCEPT_Q_LAST_GET( q_ptr )
  #define Q_XCEPT_Q_CNT( q_ptr )
  #define Q_XCEPT_Q_CHECK( q_ptr )
  #define Q_XCEPT_Q_LAST_CHECK( q_ptr )
  #define Q_XCEPT_Q_NEXT( q_ptr, link_ptr )
  #ifdef FEATURE_Q_NO_SELF_QPTR
    #define Q_XCEPT_Q_INSERT( q_ptr, q_insert_ptr, q_item_ptr )
  #else
  #define Q_XCEPT_Q_INSERT( q_insert_ptr, q_item_ptr )
  #endif
  #ifdef FEATURE_Q_NO_SELF_QPTR
    #define Q_XCEPT_Q_DELETE( q_ptr, q_delete_ptr )
  #else
  #define Q_XCEPT_Q_DELETE( q_delete_ptr )
#endif
#endif

#ifdef FEATURE_WINCE
	#define OBJ_NAME	"QUEUE_CRITICAL_SECTION_OBJ"
	static uint32 sectionCounter = 0;
#endif

/* ========================================================================
MACRO Q_ALREADY_QUEUED
DESCRIPTION
   Evaluates to true if the item passed in is already in a queue and to
   false otherwise.
=========================================================================== */
#define Q_ALREADY_QUEUED( q_link_ptr ) \
   ((q_link_ptr)->next_ptr != NULL)

/*===========================================================================

                            Function Declarations

===========================================================================*/
#ifdef __cplusplus
   extern "C" {
#endif

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
q_type* q_init ( q_type *q_ptr );


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
q_link_type* q_link ( void *item_ptr, q_link_type *link_ptr );


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
void  q_put  ( q_type *q_ptr, q_link_type *link_ptr );

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
);

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
void* q_get ( q_type *q_ptr );

/*===========================================================================

FUNCTION Q_LAST_GET

DESCRIPTION
  This function removes an item from the tail of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* q_last_get ( q_type *q_ptr );



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
int q_cnt  ( q_type *q_ptr );


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
void* q_check (q_type  *q_ptr);


/*===========================================================================

FUNCTION Q_LAST_CHECK

DESCRIPTION
  This function returns a pointer to the data block at the tail of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to q_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.

===========================================================================*/
void* q_last_check ( q_type *q_ptr );


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
void* q_next  ( q_type *q_ptr, q_link_type *link_ptr );


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
#ifdef FEATURE_Q_NO_SELF_QPTR
   void q_insert  ( q_type *q_ptr, q_link_type *q_insert_ptr, q_link_type *q_item_ptr );
#else
void q_insert  ( q_link_type *q_insert_ptr, q_link_type *q_item_ptr );
#endif


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
#ifdef FEATURE_Q_NO_SELF_QPTR
   void q_delete  ( q_type *q_ptr, q_link_type *q_delete_ptr );
#else
void q_delete  ( q_link_type *q_delete_ptr );
#endif

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
#ifdef FEATURE_Q_NO_SELF_QPTR
   boolean q_delete_ext  ( q_type *q_ptr, q_link_type *q_delete_ptr );
#else
boolean q_delete_ext  ( q_link_type *q_delete_ptr );
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
);

#if defined FEATURE_Q_NO_SELF_QPTR
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
  None.

===========================================================================*/
void *q_linear_delete_new(
  q_type             *q_ptr,
  q_compare_func_type compare_func,
  void               *compare_val,
  q_action_func_type  action_func,
  void               *param
);



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
);

#endif /* FEATURE_Q_NO_SELF_QPTR */

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
);


#ifdef __cplusplus
   }
#endif

#endif /* QUEUE_H */
