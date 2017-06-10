/* =========================================================================

DESCRIPTION
   Linked list and ordered link list routines and definitions

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
============================================================================ */

/* =========================================================================

                             Edit History

$PVCSPath: O:/src/asw/COMMON/vcs/list.c_v   1.0   24 May 2001 09:34:22   karthick  $
$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/Streamlist.cpp#11 $ $DateTime: 2013/08/02 06:28:55 $ $Author: sujitd $

when       who     what, where, why
--------   ---     ---------------------------------------------------------
09/01/00    gr     Added merge and split functions for lists and ordered
                   lists.
08/23/00    gr     Improved implementation of ordered_StreamList_push.
08/22/00    gr     Implemented ordered list API.
08/09/00   jct     Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include "Streamlist.h"
#include "IPStreamSourceUtils.h"
#include "qtv_msg.h"

/* ------------------------------------------------------------------------
**
** Unordered Lists
**
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
**
** Ordered Lists
** - These support all list operations except for push_before and
** - push_after, which would disrupt the order of a list.
** ------------------------------------------------------------------------ */

/* ==================================================================
FUNCTION ORDERED_LIST_INIT
DESCRIPTION
   Initializes an ordered list.
===================================================================== */
ordered_StreamList_type       * ordered_StreamList_init(
   ordered_StreamList_type       *StreamList_ptr,
   ordered_StreamList_config_type sort_order,
   ordered_StreamList_config_type compare_type,
   MM_HANDLE critSect /* Ptr to critical section. */
)
{
  StreamList_ptr->link.back_ptr  = NULL;
  StreamList_ptr->link.front_ptr = NULL;
  StreamList_ptr->link.size      = 0;
  StreamList_ptr->link.type      = sort_order | compare_type;

  if (StreamList_ptr->lock)
  {
    MM_CriticalSection_Release(StreamList_ptr->lock);
  }

  if (critSect)
  {
    // user supplied critical section cache it
    StreamList_ptr->lock = critSect;
  }
  else if(MM_CriticalSection_Create(&StreamList_ptr->lock) != 0)
  {
    QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_FATAL,
           "ordered_StreamList_init: Unable to create a critical section: %p",
           (void *)StreamList_ptr->lock);
  }
  return StreamList_ptr ;
} /* END */


/* ==================================================================
FUNCTION ORDERED_LIST_PUSH_AFTER
DESCRIPTION
   Pushes an item after another specified item in an ordered list.
   The caller must check to ensure that this operation does not
   disrupt the order of the list.
   Note: This function is not exported.
===================================================================== */
void ordered_StreamList_push_after(
   ordered_StreamList_type      *StreamList_ptr,
   ordered_StreamList_link_type *item_to_push_ptr,
   ordered_StreamList_link_type *item_to_push_after_ptr
)
{
 ORDERED_STREAMLIST_LOCK(StreamList_ptr);
   item_to_push_ptr->next_ptr = item_to_push_after_ptr->next_ptr;
   item_to_push_ptr->prev_ptr = item_to_push_after_ptr;
   item_to_push_after_ptr->next_ptr = item_to_push_ptr;
   if ( StreamList_ptr->link.back_ptr == item_to_push_after_ptr )
   {
     StreamList_ptr->link.back_ptr = item_to_push_ptr;
   }
   else
   {
      item_to_push_ptr->next_ptr->prev_ptr = item_to_push_ptr;
   }
   StreamList_ptr->link.size++;
 ORDERED_STREAMLIST_FREE(StreamList_ptr);

   return;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_PUSH_BEFORE
DESCRIPTION
   Pushes an item before another specified item in an ordered list.
   The caller must check to ensure that this operation does not
   disrupt the order of the list.
   Note: This function is not exported.
===================================================================== */
void ordered_StreamList_push_before(
   ordered_StreamList_type      *StreamList_ptr,
   ordered_StreamList_link_type *item_to_push_ptr,
   ordered_StreamList_link_type *item_to_push_before_ptr
)
{
 ORDERED_STREAMLIST_LOCK(StreamList_ptr);
   item_to_push_ptr->next_ptr = item_to_push_before_ptr;
   item_to_push_ptr->prev_ptr = item_to_push_before_ptr->prev_ptr;
   item_to_push_before_ptr->prev_ptr = item_to_push_ptr;
   if ( StreamList_ptr->link.front_ptr == item_to_push_before_ptr )
   {
     StreamList_ptr->link.front_ptr = item_to_push_ptr;
   }
   else
   {
      item_to_push_ptr->prev_ptr->next_ptr = item_to_push_ptr;
   }
   StreamList_ptr->link.size++;
 ORDERED_STREAMLIST_FREE(StreamList_ptr);

   return;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_PUSH
DESCRIPTION
   Pushes an item on an ordered list. The point of insertion depends
   on the weight of the item, and on the type of list (ascending,
   descending, etc.)
===================================================================== */
void ordered_StreamList_push(
   ordered_StreamList_type        *StreamList_ptr,
   ordered_StreamList_link_type   *item_link_ptr,
   ordered_StreamList_weight_type  weight
)
{
   ordered_StreamList_link_type *temp_ptr;
   ordered_StreamList_link_type *back_ptr;

   ORDERED_STREAMLIST_LOCK(StreamList_ptr);
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      STREAMLIST_ASSERT( item_link_ptr != NULL );
      STREAMLIST_ASSERT( !item_is_in_ordered_streamlist( StreamList_ptr, item_link_ptr ) );

      item_link_ptr->weight = weight;
      item_link_ptr->StreamList_ptr = StreamList_ptr;

      if ( StreamList_ptr->link.size == 0 )
      {
         item_link_ptr->next_ptr = item_link_ptr->prev_ptr = NULL;
        StreamList_ptr->link.front_ptr = StreamList_ptr->link.back_ptr = item_link_ptr;
        StreamList_ptr->link.size++;
      }
      else
      {
        temp_ptr = StreamList_ptr->link.front_ptr;
        back_ptr = StreamList_ptr->link.back_ptr;
        switch( StreamList_ptr->link.type )
         {
            case ORDERED_STREAMLIST_ASCENDING_PUSH_SLT:
               if ( back_ptr->weight < weight )
               {
                  ordered_StreamList_push_after( StreamList_ptr, item_link_ptr, back_ptr );
               }
               else
               {
                  while ( temp_ptr->weight < weight )
                  {
                     temp_ptr = temp_ptr->next_ptr;
                  }
                  ordered_StreamList_push_before( StreamList_ptr, item_link_ptr, temp_ptr );
               }
               break;

            case ORDERED_STREAMLIST_ASCENDING_PUSH_LTE:
               if ( back_ptr->weight <= weight )
               {
                  ordered_StreamList_push_after( StreamList_ptr, item_link_ptr, back_ptr );
               }
               else
               {
                  while ( temp_ptr->weight <= weight )
                  {
                     temp_ptr = temp_ptr->next_ptr;
                  }
                  ordered_StreamList_push_before( StreamList_ptr, item_link_ptr, temp_ptr );
               }
               break;

            case ORDERED_STREAMLIST_DESCENDING_PUSH_SLT:
               if ( back_ptr->weight > weight )
               {
                  ordered_StreamList_push_after( StreamList_ptr, item_link_ptr, back_ptr );
               }
               else
               {
                  while ( temp_ptr->weight > weight )
                  {
                     temp_ptr = temp_ptr->next_ptr;
                  }
                  ordered_StreamList_push_before( StreamList_ptr, item_link_ptr, temp_ptr );
               }
               break;

            case ORDERED_STREAMLIST_DESCENDING_PUSH_LTE:
               if ( back_ptr->weight >= weight )
               {
                  ordered_StreamList_push_after( StreamList_ptr, item_link_ptr, back_ptr );
               }
               else
               {
                  while ( temp_ptr->weight >= weight )
                  {
                     temp_ptr = temp_ptr->next_ptr;
                  }
                  ordered_StreamList_push_before( StreamList_ptr, item_link_ptr, temp_ptr );
               }
               break;

            default:
               /* Unknown list type.
               ** Wailing, gnashing of teeth, etc.
               */
               break;
         }
      }
   ORDERED_STREAMLIST_FREE(StreamList_ptr);

   return;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_POP_FRONT
DESCRIPTION
   Removes an item from the front of an ordered list.
===================================================================== */
void*
ordered_StreamList_pop_front(
   ordered_StreamList_type *StreamList_ptr
)
{
   ordered_StreamList_link_type *ret_ptr = NULL;
   ORDERED_STREAMLIST_LOCK(StreamList_ptr);
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );

      if( StreamList_ptr->link.size > 0 )
      {
         ret_ptr = StreamList_ptr->link.front_ptr;
         StreamList_ptr->link.front_ptr = ret_ptr->next_ptr;
         if( StreamList_ptr->link.front_ptr == NULL )
         {
            StreamList_ptr->link.back_ptr = NULL;
         }
         else
         {
            StreamList_ptr->link.front_ptr->prev_ptr = NULL;
         }
         StreamList_ptr->link.size--;
      }
   ORDERED_STREAMLIST_FREE(StreamList_ptr);
   return ret_ptr;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_SIZE
DESCRIPTION
   Returns the number of elements in an ordered list.
===================================================================== */
StreamList_size_type
ordered_StreamList_size(
   ordered_StreamList_type *StreamList_ptr
)
{
   StreamList_size_type size;
   ORDERED_STREAMLIST_LOCK(StreamList_ptr);
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      size = StreamList_ptr->link.size;
   ORDERED_STREAMLIST_FREE(StreamList_ptr);
   return size;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_PEEK_FRONT
DESCRIPTION
   Returns a pointer to the first item in an ordered list, or NULL
   if the list is empty.
===================================================================== */
void*
ordered_StreamList_peek_front(
   ordered_StreamList_type *StreamList_ptr
)
{
   void *item_ptr;
   ORDERED_STREAMLIST_LOCK(StreamList_ptr);
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      item_ptr = StreamList_ptr->link.front_ptr;
   ORDERED_STREAMLIST_FREE(StreamList_ptr);
   return item_ptr;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_PEEK_BACK
DESCRIPTION
   Returns a pointer to the last item in an ordered list, or NULL
   if the list is empty.
===================================================================== */
void*
ordered_StreamList_peek_back(
   ordered_StreamList_type *StreamList_ptr
)
{
   void *item_ptr;
   ORDERED_STREAMLIST_LOCK(StreamList_ptr);
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      item_ptr = StreamList_ptr->link.back_ptr;
   ORDERED_STREAMLIST_FREE(StreamList_ptr);
   return item_ptr;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_POP_ITEM
DESCRIPTION
   Removes a specified item from an ordered list.
===================================================================== */
void ordered_StreamList_pop_item(
   ordered_StreamList_type      *StreamList_ptr,
   ordered_StreamList_link_type *item_to_pop_ptr
)
{
   ORDERED_STREAMLIST_LOCK(StreamList_ptr);
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      STREAMLIST_ASSERT( item_is_in_ordered_streamlist( StreamList_ptr, item_to_pop_ptr ) );
      if ( item_to_pop_ptr == StreamList_ptr->link.front_ptr )
      {
         StreamList_ptr->link.front_ptr = item_to_pop_ptr->next_ptr;
      }
      else
      {
         item_to_pop_ptr->prev_ptr->next_ptr = item_to_pop_ptr->next_ptr;
      }
      if ( item_to_pop_ptr == StreamList_ptr->link.back_ptr )
      {
         StreamList_ptr->link.back_ptr = item_to_pop_ptr->prev_ptr;
      }
      else
      {
         item_to_pop_ptr->next_ptr->prev_ptr = item_to_pop_ptr->prev_ptr;
      }
      StreamList_ptr->link.size--;
   ORDERED_STREAMLIST_FREE(StreamList_ptr);

   return;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_PEEK_NEXT
DESCRIPTION
   Returns a pointer to the item following a specified item in an
   ordered list, or NULL if the input item is the last item in the
   list.
===================================================================== */
void*
ordered_StreamList_peek_next(
#ifdef LIST_DEBUG
   ordered_StreamList_type      *StreamList_ptr,
#endif
      /*lint -esym(715,StreamList_ptr)
      ** Have lint not complain about the ignored parameter 'StreamList_ptr'.
      */
   ordered_StreamList_link_type *item_after_which_to_peek
)
{
   void *item_ptr;
   ORDERED_STREAMLIST_LOCK(item_after_which_to_peek->StreamList_ptr);

 #ifdef LIST_DEBUG
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      STREAMLIST_ASSERT( item_is_in_ordered_streamlist( StreamList_ptr, item_after_which_to_peek ) );
#endif

      item_ptr = item_after_which_to_peek->next_ptr;
  ORDERED_STREAMLIST_FREE(item_after_which_to_peek->StreamList_ptr);
   return item_ptr;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_PEEK_PREV
DESCRIPTION
   Returns a pointer to the item preceding a specified item in an
   ordered list, or NULL if the input item is the last item in the
   list.
===================================================================== */
void*
ordered_StreamList_peek_prev(
#ifdef LIST_DEBUG
   ordered_StreamList_type      *StreamList_ptr,
#endif
   ordered_StreamList_link_type *item_before_which_to_peek
)
{
   void *item_ptr;
   ORDERED_STREAMLIST_LOCK(item_before_which_to_peek->StreamList_ptr);

#ifdef LIST_DEBUG
      STREAMLIST_ASSERT( ordered_StreamList_is_valid( StreamList_ptr ) );
      STREAMLIST_ASSERT( item_is_in_ordered_streamlist( StreamList_ptr, item_before_which_to_peek ) );
#endif

      item_ptr = item_before_which_to_peek->prev_ptr;
   ORDERED_STREAMLIST_FREE(item_before_which_to_peek->StreamList_ptr);
   return item_ptr;
} /* END */

/* ==================================================================
FUNCTION ORDERED_LIST_SWAP
DESCRIPTION
   Swaps two ordered lists by swapping their head nodes.
   For internal use only.
===================================================================== */
void ordered_StreamList_swap(
   ordered_StreamList_type *list1_ptr,
   ordered_StreamList_type *list2_ptr
)
{
   ordered_StreamList_type temp_StreamList_hdr;

   temp_StreamList_hdr = *list1_ptr;
   *list1_ptr    = *list2_ptr;
   *list2_ptr    = temp_StreamList_hdr;

   return;
} /* END ordered_StreamList_swap */


#ifdef LIST_DEBUG

/* ------------------------------------------------------------------------
**
** Sanity checks that are invoked in debug mode
**
** ------------------------------------------------------------------------ */

unsigned long
ordered_StreamList_is_valid( ordered_StreamList_type *StreamList_ptr )
{
   ordered_StreamList_link_type *item_ptr;
   unsigned long   cur_size = 0;

   STREAMLIST_ASSERT( StreamList_ptr != NULL );

   item_ptr = StreamList_ptr->front_ptr;
   while( item_ptr != NULL )
   {
      cur_size++;
      if ( cur_size > StreamList_ptr->size )
      {
         return 0;
      }
      item_ptr = item_ptr->next_ptr;
   }

   if ( cur_size != StreamList_ptr->size )
   {
      return 0;
   }
   else
   {
      return 1;
   }
} /* END ordered_StreamList_is_valid */


unsigned long
item_is_in_ordered_streamlist(
   ordered_StreamList_type      *StreamList_ptr,
   ordered_StreamList_link_type *item_ptr
)
{
   ordered_StreamList_link_type *cur_item_ptr;

   STREAMLIST_ASSERT( StreamList_ptr != NULL );
   STREAMLIST_ASSERT( item_ptr != NULL );

   cur_item_ptr = StreamList_ptr->front_ptr;
   while( cur_item_ptr != NULL && cur_item_ptr != item_ptr )
   {
      cur_item_ptr = cur_item_ptr->next_ptr;
   }

   if ( cur_item_ptr == NULL )
   {
      return 0;
   }
   else
   {
      return 1;
   }
} /* END item_is_in_ordered_streamlist */


#endif
