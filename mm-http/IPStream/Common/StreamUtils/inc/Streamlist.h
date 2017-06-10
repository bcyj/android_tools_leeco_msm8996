#ifndef STREAMLIST_H
#define STREAMLIST_H
/* =========================================================================

DESCRIPTION
   Linked list and ordered link list routines and definitions

   Lists are maintained as a singly linked list for unordered lists
   called "lists" and as doubly linked lists for ordered lists called
   "ordered lists".  An item in a list can be of any type as long as
   the type contains a field of type StreamList_link_type as its first field.
   An item in an ordered list must contain a field of type
   ordered_StreamList_link_type as its first field.

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

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.

============================================================================ */

/* =========================================================================

                             Edit History

$PVCSPath: O:/src/asw/COMMON/vcs/list.h_v   1.0   24 May 2001 09:34:30   karthick  $
$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/Streamlist.h#12 $ $DateTime: 2012/03/29 21:05:18 $ $Author: sbazar $

when       who     what, where, why
--------   ---     ---------------------------------------------------------
08/22/00    gr     Modified some data structures and function signatures
08/09/00   jct     Created
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include "AEEStdDef.h"
#include "MMCriticalSection.h"

#ifdef LIST_DEBUG
#undef LIST_DEBUG
#endif
/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */
/* Uncomment the following line to enable sanity checks.
** Note that this will slow down the list operations considerably.
*/
// #define LIST_DEBUG

class ordered_StreamList_type;
/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

typedef uint64 StreamList_size_type;
typedef uint64 ordered_StreamList_weight_type;

typedef struct StreamList_link_struct {
   struct StreamList_link_struct *next_ptr;
} StreamList_link_type;

typedef struct ordered_StreamList_link_struct {
   struct ordered_StreamList_link_struct *next_ptr;
   struct ordered_StreamList_link_struct *prev_ptr;
   ordered_StreamList_weight_type         weight;
   ordered_StreamList_type               *StreamList_ptr;
} ordered_StreamList_link_type;

typedef struct {
   StreamList_link_type *front_ptr;
   StreamList_link_type *back_ptr;
   StreamList_size_type  size;
} StreamList_type;

typedef struct {
   ordered_StreamList_link_type  *front_ptr;
   ordered_StreamList_link_type  *back_ptr;
   StreamList_size_type           size;
   unsigned long            type;
 } ordered_StreamL_type;



class ordered_StreamList_type
{
public:
  ordered_StreamL_type  link;

  /* Used for linking items into list. */

  int  cnt;
  /* Keeps track of number of items enqueued.*/

  MM_HANDLE  lock;

  ordered_StreamList_type() : lock(0) { }
  ~ordered_StreamList_type() { if (lock) MM_CriticalSection_Release(lock); lock = 0; }
} ;

/* ------------------------------------------------------------------------
** List Compare Function
**    Used by the searching functions to determine if an item is in the list
**    Must return non zero if the element is found or the search should
**    be terminated, 0 otherwise
** ------------------------------------------------------------------------ */
typedef int (*StreamList_compare_func_type)( void* item_ptr, void* compare_val );

typedef enum {
   ORDERED_STREAMLIST_ASCENDING  = 0x00000001,
   ORDERED_STREAMLIST_DESCENDING = 0x00000002,
   ORDERED_STREAMLIST_PUSH_SLT   = 0x00001000,
   ORDERED_STREAMLIST_PUSH_LTE   = 0x00002000,
   ORDERED_STREAMLIST_MAX = 0x7FFFFFFF
} ordered_StreamList_config_type;

#define ORDERED_STREAMLIST_ASCENDING_PUSH_SLT  ( ORDERED_STREAMLIST_ASCENDING  | ORDERED_STREAMLIST_PUSH_SLT )
#define ORDERED_STREAMLIST_ASCENDING_PUSH_LTE  ( ORDERED_STREAMLIST_ASCENDING  | ORDERED_STREAMLIST_PUSH_LTE )
#define ORDERED_STREAMLIST_DESCENDING_PUSH_SLT ( ORDERED_STREAMLIST_DESCENDING | ORDERED_STREAMLIST_PUSH_SLT )
#define ORDERED_STREAMLIST_DESCENDING_PUSH_LTE ( ORDERED_STREAMLIST_DESCENDING | ORDERED_STREAMLIST_PUSH_LTE )

#define ORDERED_STREAMLIST_SORT_ORDER_MASK 0x0000000F
#define ORDERED_STREAMLIST_PUSH_TYPE_MASK  0x0000F000

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */
/* ===================================================================
MACRO
DESCRIPTION
====================================================================== */
#define STREAMLIST_LOCK()

/* ===================================================================
MACRO
DESCRIPTION
====================================================================== */
#define STREAMLIST_FREE()

/* ===================================================================
MACRO
DESCRIPTION
====================================================================== */

#define ORDERED_STREAMLIST_LOCK(l) if (l->lock) MM_CriticalSection_Enter(l->lock)

/* ===================================================================
MACRO
DESCRIPTION
====================================================================== */
#define ORDERED_STREAMLIST_FREE(l) if (l->lock) MM_CriticalSection_Leave(l->lock)

/*===========================================================================
MACRO STREAMLIST_ASSERT

DESCRIPTION
   Causes an assertion (action depends on implementation) to happen when
   the given condition is FALSE.

  xx_condition - boolean statement to evalualte.  If the statement is
  FALSE, the assertion implementation should cause an assertion
===========================================================================*/
#ifdef LIST_DEBUG
   extern void exit(int);
   #define STREAMLIST_ASSERT( xx_condition ) if (!(xx_condition)) exit(xx_condition)
#else
   #define STREAMLIST_ASSERT( xx_condition )
#endif

/* ------------------------------------------------------------------------
** Functions
** ------------------------------------------------------------------------ */

#ifdef __cplusplus
   extern "C"
   {
#endif

/* ------------------------------------------------------------------------
**
** Ordered Lists
** - There is one function supported by ordered lists but not by
**   lists : ordered_StreamList_push.
**   Functions such as push_before/after and push_back are not supported
**   for ordered lists as these would intefere with the ordering of the list.
** ------------------------------------------------------------------------ */

ordered_StreamList_type *ordered_StreamList_init(
   ordered_StreamList_type       *StreamList_ptr,
   ordered_StreamList_config_type sort_order,
   ordered_StreamList_config_type compare_type,
   MM_HANDLE critSect = NULL
);

void ordered_StreamList_push(
   ordered_StreamList_type        *StreamList_ptr,
   ordered_StreamList_link_type   *item_link_ptr,
   ordered_StreamList_weight_type  weight
);

void*
ordered_StreamList_pop_front(
   ordered_StreamList_type *StreamList_ptr
);

StreamList_size_type
ordered_StreamList_size(
   ordered_StreamList_type *StreamList_ptr
);

void*
ordered_StreamList_peek_front(
   ordered_StreamList_type *StreamList_ptr
);

void*
ordered_StreamList_peek_back(
   ordered_StreamList_type *StreamList_ptr
);

void*
ordered_StreamList_peek_next(
#ifdef LIST_DEBUG
   ordered_StreamList_type      *StreamList_ptr,
#endif
      /*lint -esym(715,StreamList_ptr)
      ** Have lint not complain about the ignored parameter 'StreamList_ptr'.
      */
   ordered_StreamList_link_type *item_after_which_to_peek
);

void*
ordered_StreamList_peek_prev(
#ifdef LIST_DEBUG
   ordered_StreamList_type      *StreamList_ptr,
#endif
   ordered_StreamList_link_type *item_before_which_to_peek
);

void ordered_StreamList_pop_item(
   ordered_StreamList_type      *StreamList_ptr,
   ordered_StreamList_link_type *item_to_pop_ptr
);

#ifdef LIST_DEBUG


unsigned long
ordered_StreamList_is_valid(
   ordered_StreamList_type *StreamList_ptr
);

unsigned long
item_is_in_ordered_streamlist(
   ordered_StreamList_type      *StreamList_ptr,
   ordered_StreamList_link_type *item_ptr
);

#endif /* LIST_DEBUG */

#ifdef __cplusplus
   }
#endif

#endif /* STREAMLIST_H */
