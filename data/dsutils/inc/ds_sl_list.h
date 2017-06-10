#ifndef LIST_H
#define LIST_H
/* =========================================================================

DESCRIPTION
   Linked list and ordered link list routines and definitions

   Lists are maintained as a singly linked list for unordered lists
   called "lists" and as doubly linked lists for ordered lists called
   "ordered lists".  An item in a list can be of any type as long as
   the type contains a field of type list_link_type as its first field.
   An item in an ordered list must contain a field of type
   ordered_list_link_type as its first field.

   In general, a list is a linked list of items with a pointer to the
   front of the list and a pointer to the end of the list.

   +-----+
   |List |
   +-----+     +---------+     +---------+
   |Front|---->|Link     |---->|Link     |---->NULL
   +-----+     +---------+     +---------+
   |Back |--+  |User Data|     |User Data|
   +-----+  |  +---------+     +---------+
   |Size |  |                       ^
   +-----+  +-----------------------+


   For ordered lists, the "Link" field as has a pointer to the
   previous item as well.

   Ordered lists are ordered by "weight" where weight is any 32 bit
   weighting value the users chooses.  Ordered lists can be
   ascending or descending.

   Ascending lists are ordered with smallest items at the front of
   the list and larger items at the back of the list.  

   e.g. If the weights arrive as 7, 5, 9, 2, 1 an ascending list
        would be stored as:
            1, 2, 5, 7, 9

        Conversely, a descending list is kept with larger weights
        at the head of the list.  Using the values from the previous
        example the list of descending type would be stored as:
            9, 7, 5, 2, 1

   The penalty for an ordered list occurs at insertion (which takes
   time linear in the size of the list) with deletions taking
   constant time.

   Items of equal weight are inserted based on an additional
   initialization flag indicating whether to insert equal weight items
   in "strictly less than order" or "less than or equal to order".

   The difference is illustrated in this example.  Say two items of
   weight 10 are to be pushed into the list.  One must arrive before
   the other, so let one of the items be called 10[1] and the other
   10[2] indicating the item 10[1] arrives before 10[2].  Then the
   lists after the push for each case would be:

   Strictly Less Than (<)

      10[1], 10[2]

   Less Than or Equal To (<=)

      10[2], 10[1]

   For linear searches, the algorithm is the same for ordered and 
   unordered lists, always starting at the item at the front of the
   list and proceeding to the item at the end of the list.

Copyright (c) 2000,2012 by Qualcomm Technologies, Inc.  All Rights Reserved.

============================================================================ */

/* =========================================================================

                             Edit History

$PVCSPath: O:/src/asw/COMMON/vcs/list.h_v   1.0   24 May 2001 09:34:30   karthick  $
$Header: //source/qcom/qct/core/services/utils/main/latest/src/list.h#1 $ $DateTime: 2008/05/30 12:47:19 $ $Author: cgross $

when       who     what, where, why
--------   ---     ---------------------------------------------------------
08/22/00    gr     Modified some data structures and function signatures
08/09/00   jct     Created
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include "comdef.h"
#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */
#ifndef NULL
   #define NULL 0
#endif

#define LIST_DEBUG

/* Uncomment the following line to enable sanity checks.
** Note that this will slow down the list operations considerably.
*/
// #define LIST_DEBUG

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

typedef unsigned long list_size_type;
typedef unsigned long ordered_list_weight_type;

typedef struct list_link_struct {
   struct list_link_struct *next_ptr;
} list_link_type;
	
typedef struct ordered_list_link_struct {
   struct ordered_list_link_struct *next_ptr;
   struct ordered_list_link_struct *prev_ptr;
   ordered_list_weight_type         weight;
} ordered_list_link_type;

typedef struct {
   list_link_type *front_ptr;
   list_link_type *back_ptr;
   list_size_type  size;

   pthread_mutex_t list_mutex;
} list_type;

typedef struct {
   ordered_list_link_type  *front_ptr;
   ordered_list_link_type  *back_ptr;
   list_size_type           size;
   unsigned long            type;

   pthread_mutex_t list_mutex;
} ordered_list_type;

/* ------------------------------------------------------------------------
** List Compare Function
**    Used by the searching functions to determine if an item is in the list
**    Must return non zero if the element is found or the search should
**    be terminated, 0 otherwise
** ------------------------------------------------------------------------ */
typedef int (*list_compare_func_type)( void* item_ptr, void* compare_val );

typedef enum {
   ORDERED_LIST_ASCENDING  = 0x00000001,
   ORDERED_LIST_DESCENDING = 0x00000002,
   ORDERED_LIST_PUSH_SLT   = 0x00001000,
   ORDERED_LIST_PUSH_LTE   = 0x00002000,
   ORDERED_LIST_MAX = 0x7FFFFFFF
} ordered_list_config_type;

#define ORDERED_LIST_ASCENDING_PUSH_SLT  ( ORDERED_LIST_ASCENDING  | ORDERED_LIST_PUSH_SLT )
#define ORDERED_LIST_ASCENDING_PUSH_LTE  ( ORDERED_LIST_ASCENDING  | ORDERED_LIST_PUSH_LTE )
#define ORDERED_LIST_DESCENDING_PUSH_SLT ( ORDERED_LIST_DESCENDING | ORDERED_LIST_PUSH_SLT )
#define ORDERED_LIST_DESCENDING_PUSH_LTE ( ORDERED_LIST_DESCENDING | ORDERED_LIST_PUSH_LTE )

#define ORDERED_LIST_SORT_ORDER_MASK 0x0000000F
#define ORDERED_LIST_PUSH_TYPE_MASK  0x0000F000

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */

#ifdef PC_EMULATOR_H
  #ifdef LIST_RENAME
    #define PC_EMULATOR_LIST
    #include PC_EMULATOR_H
  #endif

  #ifdef LIST_INTERCEPT
    #define PC_EMULATOR_LIST_INTERCEPT
    #include PC_EMULATOR_H
  #endif
#endif

#if !defined(PC_EMULATOR_H) || !defined(LIST_INTERCEPT)
   #define LIST_INTERCEPT_LIST_INIT( list_ptr )
   #define LIST_INTERCEPT_LIST_PUSH_FRONT( list_ptr, item_link_ptr )
   #define LIST_INTERCEPT_LIST_POP_FRONT( list_ptr )
   #define LIST_INTERCEPT_LIST_PUSH_BACK( list_ptr, item_link_ptr )
   #define LIST_INTERCEPT_LIST_POP_BACK( list_ptr )
   #define LIST_INTERCEPT_LIST_SIZE( list_ptr )
   #define LIST_INTERCEPT_LIST_PEEK_FRONT( list_ptr )
   #define LIST_INTERCEPT_LIST_PEEK_BACK( list_ptr )
   #define LIST_INTERCEPT_LIST_PEEK_NEXT( list_ptr, item_after_which_to_peek )
   #define LIST_INTERCEPT_LIST_PEEK_PREV( list_ptr, item_before_which_to_peek )
   #define LIST_INTERCEPT_LIST_PUSH_BEFORE( list_ptr, item_to_push_ptr, item_to_push_before_ptr )
   #define LIST_INTERCEPT_LIST_PUSH_AFTER( list_ptr, item_to_push_ptr, item_to_push_after_ptr )
   #define LIST_INTERCEPT_LIST_POP_ITEM( list_ptr, item_to_pop_ptr )
   #define LIST_INTERCEPT_LIST_LINEAR_SEARCH( list_ptr, compare_func, compare_val )
   #define LIST_INTERCEPT_LIST_LINEAR_DELETE( list_ptr, compare_func, compare_val )

   #define LIST_INTERCEPT_ORDERED_LIST_INIT( list_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_PUSH_FRONT( list_ptr, item_link_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_POP_FRONT( list_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_PUSH_BACK( list_ptr, item_link_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_POP_BACK( list_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_SIZE( list_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_PEEK_FRONT( list_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_PEEK_BACK( list_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_PEEK_NEXT( list_ptr, item_after_which_to_peek )
   #define LIST_INTERCEPT_ORDERED_LIST_PEEK_PREV( list_ptr, item_before_which_to_peek )
   #define LIST_INTERCEPT_ORDERED_LIST_POP_ITEM( list_ptr, item_to_pop_ptr )
   #define LIST_INTERCEPT_ORDERED_LIST_LINEAR_SEARCH( list_ptr, compare_func, compare_val )
   #define LIST_INTERCEPT_ORDERED_LIST_LINEAR_DELETE( list_ptr, compare_func, compare_val )
   #define LIST_INTERCEPT_ORDERED_LIST_PUSH( list_ptr, item_link_ptr, weight )
#endif

/* ===================================================================
MACRO   LIST_LOCK
DESCRIPTION
====================================================================== */

#define list_init(list_ptr) ds_sl_list_init(list_ptr)
#define LIST_LOCK(list_mutex)  pthread_mutex_lock(list_mutex)

/* ===================================================================
MACRO     LIST_FREE
DESCRIPTION
====================================================================== */
#define LIST_FREE(list_mutex)  pthread_mutex_unlock(list_mutex)

/*===========================================================================
MACRO LIST_ASSERT

DESCRIPTION
   Causes an assertion (action depends on implementation) to happen when
   the given condition is FALSE.

  xx_condition - boolean statement to evalualte.  If the statement is
  FALSE, the assertion implementation should cause an assertion
===========================================================================*/
#if 0
#ifdef LIST_DEBUG
   #ifdef T_WINNT
      #ifdef _DEBUG
         __declspec(dllimport) void __stdcall DebugBreak( void );
         #define LIST_ASSERT( xx_condition ) \
         if( !(xx_condition) ) DebugBreak();
      #else
         #define LIST_ASSERT( xx_condition )
      #endif
   #else
      extern void exit(int);
      #define LIST_ASSERT( xx_condition ) if (!(xx_condition)) exit(xx_condition)
   #endif
#else
   #define LIST_ASSERT( xx_condition )   if (!(xx_condition)) { \
            fprintf(stderr, "%s, %d: assertion (a) failed!",    \
                    __FILE__,                                   \
                    __LINE__);                                  \
            abort();                                            \
        }
#endif
#endif
#define LIST_ASSERT( xx_condition )   if (!(xx_condition)) { \
         fprintf(stderr, "%s, %d: assertion (a) failed!",    \
                 __FILE__,                                   \
                 __LINE__);                                  \
         abort();                                            \
	}

/* ------------------------------------------------------------------------
** Functions
** ------------------------------------------------------------------------ */

#ifdef __cplusplus
   extern "C"
   {
#endif

/* ------------------------------------------------------------------------
** 
** Unordered Lists
**
** ------------------------------------------------------------------------ */

void ds_sl_list_init(
   list_type *list_ptr 
);

void list_push_front(
   list_type      *list_ptr,
   list_link_type *item_link_ptr
);

void*
list_pop_front(
   list_type *list_ptr
);

void list_push_back(
   list_type      *list_ptr,
   list_link_type *item_link_ptr
);

void*
list_pop_back(
   list_type *list_ptr
);

list_size_type
list_size(
   list_type *list_ptr
);

void*
list_peek_front(
   list_type *list_ptr
);

void*
list_peek_back(
   list_type *list_ptr
);

void*
list_peek_next(
   list_type      *list_ptr,
   list_link_type *item_after_which_to_peek
);

void*
list_peek_prev(
   list_type      *list_ptr,
   list_link_type *item_after_which_to_peek
);

void list_push_before(
   list_type *list_ptr,
   list_link_type *item_to_push_ptr,
   list_link_type *item_to_push_before_ptr
);

void list_push_after(
   list_type *list_ptr,
   list_link_type *item_to_push_ptr,
   list_link_type *item_to_push_after_ptr
);                      

void list_pop_item(
   list_type      *list_ptr,
   list_link_type *item_to_pop_ptr
);

void*
list_linear_search(
  list_type             *list_ptr,  
  list_compare_func_type compare_func,
  void                  *compare_val
);

void list_merge(
   list_type      *list1_ptr,
   list_type      *list2_ptr
);

void list_split(
   list_type      *list1_ptr,
   list_link_type *item_at_which_to_split_ptr,
   list_type      *list2_ptr
);


/* ------------------------------------------------------------------------
**
** Ordered Lists
** - There is one function supported by ordered lists but not by
**   lists : ordered_list_push.
**   Functions such as push_before/after and push_back are not supported
**   for ordered lists as these would intefere with the ordering of the list.
** ------------------------------------------------------------------------ */

void ordered_list_init(
   ordered_list_type       *list_ptr,
   ordered_list_config_type sort_order,
   ordered_list_config_type compare_type
);

void ordered_list_push(
   ordered_list_type        *list_ptr,
   ordered_list_link_type   *item_link_ptr,
   ordered_list_weight_type  weight
);

void*
ordered_list_pop_front(
   ordered_list_type *list_ptr
);

void*
ordered_list_pop_back(
   ordered_list_type *list_ptr
);

list_size_type
ordered_list_size(
   ordered_list_type *list_ptr
);

void*
ordered_list_peek_front(
   ordered_list_type *list_ptr
);

void*
ordered_list_peek_back(
   ordered_list_type *list_ptr
);

void ordered_list_pop_item(
   ordered_list_type      *list_ptr,
   ordered_list_link_type *item_to_pop
);

void*
ordered_list_peek_next(
   ordered_list_type      *list_ptr,
   ordered_list_link_type *item_after_which_to_peek
);

void*
ordered_list_peek_prev(
   ordered_list_type      *list_ptr,
   ordered_list_link_type *item_after_which_to_peek
);

void*
ordered_list_linear_search(
  ordered_list_type      *list_ptr,  
  list_compare_func_type  compare_func,
  void                   *compare_val
);

void ordered_list_merge(
   ordered_list_type *list1_ptr,
   ordered_list_type *list2_ptr
);

void ordered_list_split(
   ordered_list_type      *list1_ptr,
   ordered_list_link_type *item_at_which_to_split_ptr,
   ordered_list_type      *list2_ptr
);


#ifdef LIST_DEBUG

unsigned long
list_is_valid(
   list_type *list_ptr
);

unsigned long
item_is_in_list(
   list_type      *list_ptr,
   list_link_type *item_ptr
);

unsigned long
ordered_list_is_valid(
   ordered_list_type *list_ptr
);

unsigned long
item_is_in_ordered_list(
   ordered_list_type      *list_ptr,
   ordered_list_link_type *item_ptr
);

#endif /* LIST_DEBUG */

#ifdef __cplusplus
   }
#endif

#endif /* LIST_H */
