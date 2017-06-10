/******************************************************************************
  @file    ds_profile_util.c
  @brief    

  DESCRIPTION
  This file contains:
  1. Utility functions for DS PROFILE

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2008 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/src/ds_util.c#3 $ $DateTime: 2009/10/26 12:52:40 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#include "ds_util.h"
#include "ds_profile_os.h"
#include <string.h>

#define IS_ITR_HNDL_VALID( itr_hndl ) \
          ( (uint32)itr_hndl <= ITR_MAX_NUMBER_HANDLES ) 

#define MAGIC_EMPTY 0xFFFFFFFF

/*---------------------------------------------------------------------------
               Implementation of List iterator fn
---------------------------------------------------------------------------*/
void list_first( 
  void *t 
)
{
  ds_util_list_type *l  = ( ds_util_list_type *) t;   

  /* Get what is the current node */
  ds_util_list_node_type *first = (ds_util_list_node_type *)(l->lst.front_ptr);

  /* Validation */
  if ( ( first == NULL) || ( (uint32)first == MAGIC_EMPTY ) ) 
  {
    /* This cannot be handled, stop here */
    return;
  }

  l->itr.curr = ( void * )first;
  return;
}

int list_next( 
  void *t 
)
{
  ds_util_list_type *lst  = ( ds_util_list_type *) t;   

  /* Get what is the current node */
  ds_util_list_node_type *curr = lst->itr.curr;

  /* Validation */
  if ( ( curr == NULL) || ( (uint32)curr == MAGIC_EMPTY ) ) 
  {
    /* This cannot be handled, stop here */
    return DS_FAILURE;
  }

  /* Advance by 1 and return */
  lst->itr.curr           = ( void * ) ( ( ( list_link_type * ) curr )->next_ptr );
  if ( lst->itr.curr == NULL )
  {
    return DS_FAILURE;
  }
  return DS_SUCCESS;
}

/* TO DO make get_data and get_size to 1 function, (ret type error code) */
void * list_get_data( 
  void *t 
)
{
  ds_util_list_type *l  = ( ds_util_list_type *) t;   

  /* Get what is the current node */
  ds_util_list_node_type *curr = l->itr.curr;

  /* Validation */
  if ( ( curr == NULL) || ( (uint32)curr == MAGIC_EMPTY ) ) 
  {
    return NULL;
  }

  /* Return data from node */
  return ( void *) curr->data;
}

int list_get_size( 
  void *t 
)
{
  ds_util_list_type *l  = ( ds_util_list_type *) t;   

  /* Get what is the current node */
  ds_util_list_node_type *curr = l->itr.curr;

  /* Validation */
  if ( ( curr == NULL) || ( (uint32)curr == MAGIC_EMPTY ) ) 
  {
    return 0;
  }

  /* Return data from node */
  return curr->size;
}

void list_dstr( 
  void *t 
)
{
  ds_util_list_type *l  = ( ds_util_list_type *) t; 
  l->itr.f( t );
  return;
}

/*---------------------------------------------------------------------------
               Implementation of List Fn
---------------------------------------------------------------------------*/
int ds_util_list_get_hndl
(
  ds_util_list_hndl_type *hndl /* <- */
)
{
  ds_util_list_type *l = NULL;

  /* Allocate and init list blob a */

  l = ( ds_util_list_type * )DS_PROFILE_MEM_ALLOC( sizeof( ds_util_list_type ),
                                              MODEM_MEM_CLIENT_DATA );

  if ( l == NULL ) 
  {
    return DS_FAILURE;
  }
  memset( l, 0, sizeof( ds_util_list_type ) );
  list_init( &l->lst );
  l->itr.f      = list_first;
  l->itr.n      = list_next;
  l->itr.i      = list_get_data;
  l->itr.s      = list_get_size;
  l->itr.d      = list_dstr;
  l->itr.curr   = (void *)MAGIC_EMPTY;
  *hndl         = (ds_util_list_hndl_type)l;
  

  return DS_SUCCESS;
}/* ds_util_list_get_hndl() */

int ds_util_list_add(
  ds_util_list_hndl_type  hndl,
  void                   *info,
  uint32                  info_size
)
{
  ds_util_list_type *l = ( ds_util_list_type * )hndl;
  ds_util_list_node_type *node;

  node = ( ds_util_list_node_type * ) DS_PROFILE_MEM_ALLOC( sizeof( ds_util_list_node_type ),
                                                       MODEM_MEM_CLIENT_DATA );
  /* TO DO check size */

  if ( node == NULL) 
  {
    return DS_FAILURE;
  }

  node->data = (void *)DS_PROFILE_MEM_ALLOC( info_size,
                                        MODEM_MEM_CLIENT_DATA );
  if ( node->data == NULL) 
  {
    DS_PROFILE_MEM_FREE( (void *) node,
			MODEM_MEM_CLIENT_DATA);
    node = NULL;
    return DS_FAILURE;
  }

  node->size = info_size; 
  memcpy( node->data, info, info_size );
  node->self = node;

  /* first node is added, subsequent updates to curr is thru (*next)(), frst is unchanged */
  if ( ( (uint32)l->itr.curr == MAGIC_EMPTY ) ) 
  {
     //DS_ASSERT( l->itr.curr == NULL );
     //DS_ASSERT( l->itr.frst == NULL );
     l->itr.curr = ( void * )node;
  }

  list_push_back( &l->lst, ( list_link_type *)node );

  return DS_SUCCESS;
}/*ds_util_list_add()*/

/*==================================================================================*/

uint32 ds_util_list_get_size(
  ds_util_list_hndl_type hndl
)
{
  list_size_type sz;
  ds_util_list_type *l = ( ds_util_list_type * )hndl;

  sz = list_size( &l->lst );

  return (uint32) sz;
}

/*==================================================================================*/

int ds_util_list_rel_hndl
(
  ds_util_list_hndl_type hndl
)
{
  ds_util_list_type *l = ( ds_util_list_type * )hndl;
  ds_util_list_node_type *node;

  while ( ( node = list_pop_front( &l->lst ) ) != NULL )
  {
    DS_PROFILE_MEM_FREE( (void *)node->data,
                    MODEM_MEM_CLIENT_DATA );
    DS_PROFILE_MEM_FREE( (void *)node,
                    MODEM_MEM_CLIENT_DATA );
  }
  DS_PROFILE_MEM_FREE( (void *)l, 
                  MODEM_MEM_CLIENT_DATA );

  return DS_SUCCESS;
}


/*---------------------------------------------------------------------------
               Implementation of Iterator
---------------------------------------------------------------------------*/
int ds_util_itr_get_hndl(
  ds_util_iterable_type *obj, /* -> */
  ds_util_itr_hndl_type *hndl /* <- */
)
{

  //PLM_LOGD( "_itr_get_hndl: ENTRY", 0 );

  if ( ( obj == NULL ) || ( hndl == NULL ) ) 
  {
    //PLM_LOGE( "invalid arg", 0 );
    return DS_FAILURE;
  }

  if ( ( obj->n == NULL ) || ( obj->d == NULL ) || 
       ( obj->s == NULL ) || ( obj->f == NULL ) )
  {
    return DS_FAILURE;
  }

  *hndl = (ds_util_itr_hndl_type *) obj;
  //PLM_LOGD( "_itr_get_hndl: EXIT", 0 );
  return DS_SUCCESS;

}/* dsi_profile_utils_get_itr() */


int ds_util_itr_next(
  ds_util_itr_hndl_type   hndl
)
{
  ds_util_iterable_type *itr_obj = ( ds_util_iterable_type * )hndl;
  return itr_obj->n( (void *) itr_obj );
}/* ds_util_itr_next() */

int ds_util_itr_first(
  ds_util_itr_hndl_type   hndl
)
{
  ds_util_iterable_type *itr_obj = ( ds_util_iterable_type * )hndl;
  itr_obj->f( (void *) itr_obj );
  return DS_SUCCESS;
}


int ds_util_itr_get_data(
  ds_util_itr_hndl_type   hndl,
  void                   *obj,  /* <- */
  uint32                 *obj_size  /* <-> */
)
{
  uint32 tmp_sz = 0;
  ds_util_iterable_type *itr_obj = ( ds_util_iterable_type * )hndl;

  /* */
  void *data_obj = itr_obj->i( (void *) hndl );
  tmp_sz         = itr_obj->s( (void *) hndl );

  if ( data_obj == NULL )
  {
    return DS_FAILURE;
  }

  if ( tmp_sz > *obj_size || tmp_sz == 0 ) 
  {
    return DS_FAILURE;
  }
  memcpy( obj, data_obj, tmp_sz );
  *obj_size = tmp_sz;
  return DS_SUCCESS;
}/* ds_util_itr_get_data() */


int ds_util_itr_rel_hndl(
   ds_util_itr_hndl_type  hndl
)
{
  ds_util_iterable_type *itr_obj = ( ds_util_iterable_type * )hndl;
  itr_obj->d( (void *) itr_obj );
  return DS_SUCCESS;
}/*ds_util_itr_destroy()*/
