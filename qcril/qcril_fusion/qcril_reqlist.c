/*!
  @file
  qcril_reqlist.c

  @brief
  Maintains a list of outstanding RIL_REQUESTs and pairs them with the
  response. Maintains a timer so requests don't get stuck forever.
*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_reqlist.c#7 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
12/08/09   sb      Added function to update subsystem info.
04/05/09   fc      Clenaup log macros and mutex macros.
09/19/08   pg      Added routine to print out all the entries on the request 
                   list.
05/29/08   tml     Fixed compilation warnings in LTK
05/22/08   tml     Fixed compilation issue with LTK
05/19/08   tml     Fixed head link list's next pointer value
05/08/08   fc      Changes to linked list implementation.
05/05/08   da      Initial framework.


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include <cutils/memory.h>
#include "IxErrno.h"
#include "ril.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_arb.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_REQLIST_BUF_MAX 50

typedef struct qcril_reqlist_buf_tag
{
  qcril_reqlist_public_type pub;
  struct qcril_reqlist_buf_tag *prev_ptr;
  struct qcril_reqlist_buf_tag *next_ptr;
} qcril_reqlist_buf_type;


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/*! Variables internal to module qcril_reqlist.c
*/
typedef struct
{
  qcril_reqlist_buf_type req[ QCRIL_REQLIST_BUF_MAX ];
  qcril_reqlist_buf_type *head_ptr; 
  qcril_reqlist_buf_type *tail_ptr;
} qcril_reqlist_struct_type;

/*! @brief Req ID */
static uint16 req_id;

/*! @brief Variables internal to module qcril_reqlist.c
*/
static qcril_reqlist_struct_type qcril_reqlist[ QCRIL_MAX_INSTANCE_ID ];

/* Mutex used to control simultaneous update/access to ReqList */
static pthread_mutex_t qcril_reqlist_mutex;      


/*===========================================================================

                                FUNCTIONS

===========================================================================*/


/*===========================================================================

  FUNCTION:  qcril_reqlist_find

===========================================================================*/
/*!
    @brief
    Find an entry in the list based on RIL_Token
 
    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find
( 
  qcril_instance_id_e_type instance_id,
  RIL_Token t
)
{                              
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( buf_ptr->pub.t == t )
    {             
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d\n", 
                      instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                      qcril_log_get_token_id( t ) );
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry : token id %d\n", instance_id, qcril_log_get_token_id( t ) );

  return NULL;

} /* qcril_reqlist_find() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_find_by_request

===========================================================================*/
/*!
    @brief
    Find the first entry in the list based on a request 
 
    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find_by_request
( 
  qcril_instance_id_e_type instance_id,
  int request
)
{
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( buf_ptr->pub.request == request ) 
    {
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d\n", 
                      instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                      qcril_log_get_token_id( buf_ptr->pub.t ) ); 
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry for %s(%d)\n", 
                  instance_id, qcril_log_lookup_event_name( request ), request );

  return NULL;

} /* qcril_reqlist_find_by_request() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_find_by_req_id

===========================================================================*/
/*!
    @brief
    Find an entry in the list based on Req ID
 
    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find_by_req_id
( 
  uint32 req_id,
  qcril_instance_id_e_type *instance_id_ptr
)
{                              
  uint8 i;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  for ( i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
  {
    buf_ptr = qcril_reqlist[ i ].head_ptr;

    while ( buf_ptr != NULL )
    {
      if ( buf_ptr->pub.req_id == req_id )
      {             
        QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d\n", 
                        i, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                        qcril_log_get_token_id( buf_ptr->pub.t ) );

        *instance_id_ptr = i;

        return buf_ptr;
      }
      else
      {
        buf_ptr = buf_ptr->next_ptr;
      }
    }

    QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry : req id %d\n", i, req_id );
  }

  return NULL;

} /* qcril_reqlist_find_by_req_id() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_find_by_event

===========================================================================*/
/*!
    @brief
    Find the first entry in the list based on an event
 
    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find_by_event
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  uint32 pending_event_id,
  uint32 state
)
{
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( ( buf_ptr->pub.state[ modem_id ] & state ) &&
         ( buf_ptr->pub.pending_event_id[ modem_id ] == pending_event_id ) )
    {
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d, pending %s (%lu)\n", 
                      instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                      qcril_log_get_token_id( buf_ptr->pub.t ), 
                      qcril_log_lookup_event_name( pending_event_id ), pending_event_id );
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry waiting for %s (%lu)\n", 
                  instance_id, qcril_log_lookup_event_name( pending_event_id ), pending_event_id );

  return NULL;

} /* qcril_reqlist_find_by_event() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_find_by_sub_id

===========================================================================*/
/*!
    @brief
    Find the first entry in the list based on
    subsystem ID (e.g. Call ID, SS Ref, Invoke ID).

    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find_by_sub_id
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  uint32 sub_id,
  uint32 state
)
{
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL  )
  {
    if ( ( buf_ptr->pub.state[ modem_id ] & state ) &&
         buf_ptr->pub.valid_sub_id && ( buf_ptr->pub.sub_id == sub_id ) )
    {
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d, sub id %lu\n",
            instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ),
            buf_ptr->pub.request, qcril_log_get_token_id( buf_ptr->pub.t ), sub_id );
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry for sub id %lu\n",
            instance_id, sub_id );

  return NULL;

} /* qcril_reqlist_find_by_sub_id() */

/*===========================================================================

  FUNCTION:  qcril_reqlist_find_by_event_and_sub_id

===========================================================================*/
/*!
    @brief
    Find the first entry in the list based on an event and 
    a subsystem ID (e.g. Call ID, SS Ref, Invoke ID).
 
    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find_by_event_and_sub_id
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  uint32 pending_event_id,
  uint32 sub_id,
  uint32 state
)
{
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL  )
  {
    if ( ( buf_ptr->pub.state[ modem_id ] & state ) &&
         ( buf_ptr->pub.pending_event_id[ modem_id ] == pending_event_id ) &&
         buf_ptr->pub.valid_sub_id && ( buf_ptr->pub.sub_id == sub_id ) )
    {
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d, pending %s (%lu), sub id %lu\n", 
                      instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                      qcril_log_get_token_id( buf_ptr->pub.t ), 
                      qcril_log_lookup_event_name( pending_event_id ), pending_event_id, sub_id );                 
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry waiting for %s (%lu) and sub id %lu\n", 
                  instance_id, qcril_log_lookup_event_name( pending_event_id ), pending_event_id, sub_id );

  return NULL;

} /* qcril_reqlist_find_by_event_and_sub_id() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_lookup_state_name

===========================================================================*/
/*!
    @brief
    Lookup the state name.
 
    @return
    None
*/
/*=========================================================================*/
const char *qcril_reqlist_lookup_state_name
(
  qcril_req_state_e_type state
)
{
  /*-----------------------------------------------------------------------*/

  switch( state )
  {
    case QCRIL_REQ_FREE:
      return "QCRIL_REQ_FREE";
    
    case QCRIL_REQ_AWAITING_CALLBACK:
      return "QCRIL_REQ_AWAITING_CALLBACK";

    case QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS:
      return "QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS";

    case QCRIL_REQ_COMPLETED_SUCCESS:
      return "QCRIL_REQ_COMPLETED_SUCCESS";

    case QCRIL_REQ_COMPLETED_FAILURE:
      return "QCRIL_REQ_COMPLETED_FAILURE";

    default:
      break;
  }

  return "Undefined";

} /* qcril_reqlist_lookup_state_name */


/*===========================================================================

  FUNCTION:  qcril_reqlist_new_buf

===========================================================================*/
/*!
    @brief
    Allocate a new buffer for the list
 
    @return
    A pointer to a free buffer, or NULL if there are no free buffers 
    available.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_new_buf
(
  qcril_instance_id_e_type instance_id
)
{
  uint8 i, j;
  boolean found = FALSE;
  qcril_reqlist_buf_type *buf_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  for ( i = 0; ( i < QCRIL_REQLIST_BUF_MAX ) && !found ; i++ )
  {
    found = TRUE;
    buf_ptr = &qcril_reqlist[ instance_id ].req[ i ];
    for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
    {
      if ( buf_ptr->pub.state[ j ] != QCRIL_REQ_FREE ) 
      {
        found = FALSE;
        break;
      }
    }
  }

  if( i == QCRIL_REQLIST_BUF_MAX )
  {
    buf_ptr = NULL;
  }

  return buf_ptr;

} /* qcril_reqlist_new_buf() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_new_all

===========================================================================*/
/*!
    @brief
    Add an entry to the list of outstanding RIL_REQUESTs.
 
    @return
    E_SUCCESS if the entry is created properly
    E_NOT_ALLOWED if the entry was already in the list
    E_NO_MEMORY if there were no buffers
    E_DATA_TOO_LARGE if the subsystem specific info exceeded the buffer size
*/
/*=========================================================================*/
static IxErrnoType qcril_reqlist_new_all
( 
  qcril_instance_id_e_type instance_id,
  qcril_reqlist_public_type *entry_ptr
)
{
  qcril_reqlist_buf_type *buf_ptr;
  qcril_reqlist_public_type *pub_ptr;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist_find( instance_id, entry_ptr->t );

  if ( buf_ptr != NULL )
  {
    /* Token is already in the list */
    QCRIL_LOG_ERROR( "[RID %d] Fail to add ReqList entry with token id %d [%p] (already exists)\n", 
                     instance_id, qcril_log_get_token_id( buf_ptr->pub.t ), buf_ptr->pub.t );
    return E_NOT_ALLOWED;
  }

  buf_ptr = qcril_reqlist_new_buf( instance_id );

  if ( buf_ptr == NULL )
  {
    /* No free buffers in the list */
    QCRIL_LOG_ERROR( "[RID %d] Fail to add ReqList entry for token id %d [%p] (no buffer)\n", 
                     instance_id, qcril_log_get_token_id( entry_ptr->t ), entry_ptr->t );
    return E_NO_MEMORY;
  }

  pub_ptr = &buf_ptr->pub;

  *pub_ptr = *entry_ptr;

  /* Sort items in the ReqList by the order of RIL command being received */
  buf_ptr->pub.req_id = req_id++;
  buf_ptr->prev_ptr = qcril_reqlist[ instance_id ].tail_ptr;
  buf_ptr->next_ptr = NULL;
  if ( qcril_reqlist[ instance_id ].head_ptr == NULL )
  {
    qcril_reqlist[ instance_id ].head_ptr = buf_ptr;
  }
  if ( qcril_reqlist[ instance_id ].tail_ptr != NULL )
  {
    qcril_reqlist[ instance_id ].tail_ptr->next_ptr = buf_ptr;
  }

  qcril_reqlist[ instance_id ].tail_ptr = buf_ptr;

  /* Return Req ID to caller */
  *entry_ptr = *pub_ptr;

  QCRIL_LOG_INFO( "[RID %d] Event %s(%d), token id %d [ %p], req id %d : ", 
                  instance_id, qcril_log_lookup_event_name( pub_ptr->request ), pub_ptr->request, 
                  qcril_log_get_token_id( pub_ptr->t ), pub_ptr->t, pub_ptr->req_id  );
  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    if ( pub_ptr->state[ i ] != QCRIL_REQ_FREE )
    {
      QCRIL_LOG_INFO( "                       MID %d, pending receipt of %s, state %s(%d)", 
                      i, qcril_log_lookup_event_name( pub_ptr->pending_event_id[ i ] ),  
                      qcril_reqlist_lookup_state_name( pub_ptr->state[ i ] ), pub_ptr->state[ i ] );
    }
  }

  return E_SUCCESS;

} /* qcril_reqlist_new_all() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_init

===========================================================================*/
/*!
    @brief
    Initialize ReqList.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_reqlist_init
( 
  void
)
{ 
  uint8 i, j, buf_idx;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_reqlist_init()\n" );

  memset( &qcril_reqlist, 0, sizeof( qcril_reqlist ) ); 

  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    for ( buf_idx = 0; buf_idx < QCRIL_REQLIST_BUF_MAX; buf_idx++ )
    {
      for ( j = 0 ; j < QCRIL_ARB_MAX_MODEMS; j++ )
      {
        qcril_reqlist[ i ].req[ buf_idx ].pub.state[ j ] = QCRIL_REQ_FREE;
      }
    }

    qcril_reqlist[ i ].head_ptr = NULL;
    qcril_reqlist[ i ].tail_ptr = NULL;
  }

  pthread_mutex_init( &qcril_reqlist_mutex, NULL );

} /* qcril_reqlist_init() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_print_all

===========================================================================*/
/*!
    @brief
    Print the all the entries on the request list.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_reqlist_print_all 
(
  qcril_instance_id_e_type instance_id
)
{
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  if ( buf_ptr == NULL ) 
  {
    QCRIL_LOG_DEBUG( "[RID %d] ReqList entries : Empty \n", instance_id );
  }
  else
  {
    QCRIL_LOG_DEBUG( "[RID %d] ReqList entries :\n", instance_id );
  }

  while ( buf_ptr != NULL )
  {
    QCRIL_LOG_DEBUG( "    %s (%d), token id %d\n", 
                     qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                     qcril_log_get_token_id( buf_ptr->pub.t ) );
    buf_ptr = buf_ptr->next_ptr;
  }

  return;

} /* qcril_reqlist_print_all */


/*===========================================================================

  FUNCTION:  qcril_reqlist_default_entry

===========================================================================*/
/*!
    @brief
    Default reqlist entry.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_reqlist_default_entry
(
  RIL_Token t,
  int request,
  qcril_modem_id_e_type modem_id,
  qcril_req_state_e_type state,      
  qcril_evt_e_type pending_event_id,
  qcril_reqlist_u_type *sub_ptr,
  qcril_reqlist_public_type *req_ptr
)
{
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( req_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  memset( req_ptr, 0, sizeof( qcril_reqlist_public_type ) );

  req_ptr->t = t;
  req_ptr->request = request;
  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    req_ptr->state[ i ] = QCRIL_REQ_FREE;
    req_ptr->pending_event_id[ i ] = QCRIL_EVT_NONE;
  }

  req_ptr->valid_sub_id = FALSE;
  req_ptr->sub_id = 0;

  req_ptr->state[ modem_id ] = state;
  req_ptr->pending_event_id[ modem_id ] = pending_event_id;
  if ( sub_ptr != NULL )
  {
    req_ptr->sub = *sub_ptr;
  }

} /* qcril_reqlist_default_entry */


/*===========================================================================

  FUNCTION:  qcril_reqlist_new

===========================================================================*/
/*!
    @brief
    Add an entry to the list of outstanding RIL_REQUESTs.
 
    @return
    see qcril_reqlist_new_all
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_new
( 
  qcril_instance_id_e_type instance_id,
  qcril_reqlist_public_type *entry_ptr
)
{
  IxErrnoType status;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  status = qcril_reqlist_new_all( instance_id, entry_ptr );
  if ( status == E_SUCCESS )
  {
    qcril_reqlist_print_all( instance_id );
  }
  else if ( ( entry_ptr->t != NULL ) && ( entry_ptr->t != ( void *) QCRIL_TOKEN_ID_INTERNAL )
        && ( entry_ptr->t != ( void *) QCRIL_TOKEN_ID_INTERNAL1 ))
  {
    QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );
    /* Out of ReqList buffer, send RIL_E_GENERIC_FAILURE response */
    qcril_default_request_resp_params( instance_id, entry_ptr->t, entry_ptr->request, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return status;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_new() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_free_all

===========================================================================*/
/*!
    @brief
    Free all entries in the list of outstanding RIL_REQUESTs.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_reqlist_free_all
( 
  void
)
{
  uint8 i, j;
  qcril_reqlist_buf_type *buf_ptr;
  qcril_reqlist_buf_type *next_buf_ptr;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    buf_ptr = qcril_reqlist[ i ].head_ptr;

    while ( buf_ptr != NULL )
    {
      next_buf_ptr = buf_ptr->next_ptr;

      /* Send RadioNotAvailable as the response to the RIL command */
      if ( ( buf_ptr->pub.t != NULL ) && ( buf_ptr->pub.t != ( void *) QCRIL_TOKEN_ID_INTERNAL ) )
      {
        QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );
        qcril_default_request_resp_params( i, buf_ptr->pub.t, buf_ptr->pub.request, RIL_E_RADIO_NOT_AVAILABLE, &resp );
        qcril_send_request_response( &resp );
        QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );
      }

      for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
      {
        buf_ptr->pub.state[ j ] = QCRIL_REQ_FREE;
        buf_ptr->pub.pending_event_id[ j ] = QCRIL_EVT_NONE;
      }
      buf_ptr->prev_ptr = NULL;
      buf_ptr->next_ptr = NULL;
      buf_ptr = next_buf_ptr;
    }

    qcril_reqlist[ i ].head_ptr = NULL;
    qcril_reqlist[ i ].tail_ptr = NULL;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

} /* qcril_reqlist_free_all */


/*===========================================================================

  FUNCTION:  qcril_reqlist_free

===========================================================================*/
/*!
    @brief
    Frees an entry from the list of outstanding RIL_REQUESTs.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_free
( 
  qcril_instance_id_e_type instance_id,
  RIL_Token t
)
{
  uint8 i;
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;
  qcril_reqlist_public_type *pub_ptr;
  qcril_reqlist_buf_type *prev_buf_ptr, *next_buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    /* Detach the item from the ReqList */
    prev_buf_ptr = buf_ptr->prev_ptr;
    next_buf_ptr = buf_ptr->next_ptr;

    /* The only item in the ReqList is being removed */
    if ( ( prev_buf_ptr == NULL ) && ( next_buf_ptr == NULL ) )
    {
      qcril_reqlist[ instance_id ].head_ptr = NULL;
      qcril_reqlist[ instance_id ].tail_ptr = NULL;
    }
    /* First item in the ReqList is being removed */
    else if ( prev_buf_ptr == NULL )
    {
      qcril_reqlist[ instance_id ].head_ptr = next_buf_ptr;
      next_buf_ptr->prev_ptr = NULL;
    }   
    /* Last item in the ReqList is being removed*/
    else if ( next_buf_ptr == NULL )
    {
      qcril_reqlist[ instance_id ].tail_ptr = prev_buf_ptr;
      prev_buf_ptr->next_ptr = NULL;
    }
    /* Middle item in the ReqList is being removed */
    else
    {
      prev_buf_ptr->next_ptr = buf_ptr->next_ptr;
      next_buf_ptr->prev_ptr = buf_ptr->prev_ptr;
    }

    buf_ptr->next_ptr = buf_ptr->prev_ptr = NULL;

    pub_ptr = &buf_ptr->pub;

    for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
    {
      pub_ptr->state[ i ] = QCRIL_REQ_FREE;
    }

    QCRIL_LOG_INFO( "[RID %d] Deleted ReqList entry : token id %d [%p]\n", instance_id, qcril_log_get_token_id( t ), t );

    qcril_reqlist_print_all( instance_id );
  }
  else
  {
    /* Entry was not found */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_free() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_update_state

===========================================================================*/
/*!
    @brief
    Updates the state of the entry in the list for RIL_Token t with the 
    specified information.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_update_state
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  RIL_Token t,
  qcril_req_state_e_type state
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    /* Invalid state */
    if ( ( state == QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS ) && ( buf_ptr->pub.pending_event_id[ modem_id ] == QCRIL_EVT_NONE ) )
    {
      status = E_NOT_ALLOWED;
    }
    else
    {
      buf_ptr->pub.state[ modem_id ] = state;
      QCRIL_LOG_INFO( "[RID %d] Update ReqList entry : %s(%d), token id %d, state %s(%d)\n", 
                      instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                      qcril_log_get_token_id( buf_ptr->pub.t ), qcril_reqlist_lookup_state_name( state ), state );
    }
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_update_state() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_update_pending_event_id

===========================================================================*/
/*!
    @brief
    Updates the pending event ID of the entry in the list for RIL_Token t 
    with the specified information.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_update_pending_event_id
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  RIL_Token t,
  qcril_evt_e_type pending_event_id
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    buf_ptr->pub.pending_event_id[ modem_id ] = pending_event_id;
    QCRIL_LOG_INFO( "[RID %d] Update event field of ReqList entry : %s(%d), token id %d, MID %d pending %s (%d)\n", 
                    instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                    qcril_log_get_token_id( buf_ptr->pub.t ), modem_id,
                    qcril_log_lookup_event_name( pending_event_id ), pending_event_id );
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_update_pending_event_id() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_update_sub_id

===========================================================================*/
/*!
    @brief
    Updates the subsystem ID (e.g Call ID, SS Ref, Invoke ID) of the entry 
    in the list for RIL_Token t with the specified information.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_update_sub_id
( 
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  uint32 sub_id
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if (buf_ptr != NULL)
  {
    buf_ptr->pub.valid_sub_id = TRUE;
    buf_ptr->pub.sub_id = sub_id;
    QCRIL_LOG_INFO( "[RID %d] Update sub id field of ReqList entry : %s(%d), token id %d, sub id %lu\n", 
                    instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                    qcril_log_get_token_id( buf_ptr->pub.t ), sub_id );
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_update_subs_id() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_query

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given RIL_Token and returns it to
    the requesting subsystem.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query
( 
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  qcril_reqlist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if (buf_ptr != NULL)
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_request

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given RIL request.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_request
( 
  qcril_instance_id_e_type instance_id,
  int request,
  qcril_reqlist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_request( instance_id, request );

  if ( buf_ptr != NULL )
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_request */


/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_req_id

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given Req ID and returns it to
    the requesting subsystem.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_req_id
( 
  uint16 req_id,        
  qcril_instance_id_e_type *instance_id_ptr,
  qcril_reqlist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id_ptr != NULL );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_req_id( req_id, instance_id_ptr );

  if ( buf_ptr != NULL )
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_req_id() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_event

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given a pending event ID, and 
    returns it to the requesting subsystem.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
    E_NOT_ALLOWED if the entry was not waiting for an event
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_event
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_evt_e_type pending_event_id,
  qcril_reqlist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_event( instance_id, modem_id, pending_event_id, QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS );

  if (buf_ptr != NULL)
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_event() */

/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_sub_id

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given
    subsystem ID (e.g. Call ID, SS Ref, Invoke ID etc),
    and returns it to the requesting subsystem.

    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
    E_NOT_ALLOWED if the entry was not waiting for an event
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_sub_id
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  uint32 sub_id,
  qcril_reqlist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_sub_id( instance_id, modem_id, sub_id,
                                          QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS );

  if ( buf_ptr != NULL )
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_sub_id() */
/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_event_and_sub_id

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given a pending event ID, and 
    subsystem ID (e.g. Call ID, SS Ref, Invoke ID etc), and returns it to 
    the requesting subsystem.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
    E_NOT_ALLOWED if the entry was not waiting for an event
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_event_and_sub_id
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  uint32 pending_event_id,
  uint32 sub_id,
  qcril_reqlist_public_type *info_ptr 
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_event_and_sub_id( instance_id, modem_id, pending_event_id, sub_id, 
                                                    QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS );

  if ( buf_ptr != NULL )
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_event_and_sub_id() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_event_all_states

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given a pending event ID, and 
    returns it to the requesting subsystem. Ignore state of the requests.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
    E_NOT_ALLOWED if the entry was not waiting for an event
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_event_all_states
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_evt_e_type pending_event_id,
  qcril_reqlist_public_type *info_ptr 
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_event( instance_id, modem_id, pending_event_id, 
                                         QCRIL_REQ_FREE | QCRIL_REQ_AWAITING_CALLBACK | QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS | 
                                         QCRIL_REQ_COMPLETED_SUCCESS | QCRIL_REQ_COMPLETED_FAILURE );

  if ( buf_ptr != NULL )
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_event_all_states() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_query_by_event_and_sub_id_all_states

===========================================================================*/
/*!
    @brief
    Find the first entry in the list based on a request event and 
    a subsystem ID (e.g. Call ID, SS Ref, Invoke ID). Ignore the
    state of the request.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_query_by_event_and_sub_id_all_states 
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  uint32 pending_event_id,
  uint32 sub_id,
  qcril_reqlist_public_type *info_ptr 
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find_by_event_and_sub_id( instance_id, modem_id, pending_event_id, sub_id, 
                                                    QCRIL_REQ_FREE | QCRIL_REQ_AWAITING_CALLBACK | QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS | 
                                                    QCRIL_REQ_COMPLETED_SUCCESS | QCRIL_REQ_COMPLETED_FAILURE  );

  if ( buf_ptr != NULL )
  {
    *info_ptr = buf_ptr->pub;
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_query_by_event_and_sub_id_all_states() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_update_sub_info

===========================================================================*/
/*!
    @brief
    Updates the subsystem-specific info of the entry in the list for 
    RIL_Token t with the specified information.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_update_sub_info
( 
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  qcril_reqlist_u_type *sub_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( sub_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    buf_ptr->pub.sub = *sub_ptr;

    QCRIL_LOG_INFO( "[RID %d] Update sub info of ReqList entry : %s(%d), token id %d\n", 
                    instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                    qcril_log_get_token_id( buf_ptr->pub.t ) );
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_update_subs_info() */


/*===========================================================================

  FUNCTION:  qcril_reqlist_complete_all_amss_events

===========================================================================*/
/*!
    @brief
    Cleanup the state and pending event of the entry in the list with the 
    specified information.  
 
    @return
    E_SUCCESS if the entry was found and all pending events are received.
    E_IN_PROG if the entry was found but not all pending events are received.
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_reqlist_complete_all_amss_events
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  RIL_Token t,
  qcril_req_state_e_type state,
  qcril_modem_ids_list_type *modem_ids_done_list,
  IxErrnoType *result
)
{
  uint8 i;
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( modem_ids_done_list != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );
  
  *result = E_SUCCESS;

  modem_ids_done_list->num_of_modems = 0;

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    buf_ptr->pub.state[ modem_id ] = state;

    for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
    {
      QCRIL_LOG_DEBUG( "[RID %d] %s(%d) token id %d, MID %d, State %s(%d)\n", 
                       instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,  
                       qcril_log_get_token_id( buf_ptr->pub.t ), i, qcril_reqlist_lookup_state_name( buf_ptr->pub.state[ i ] ), 
                       buf_ptr->pub.state[ i ] );

      if ( ( buf_ptr->pub.state[ i ] == QCRIL_REQ_COMPLETED_SUCCESS ) ||
           ( buf_ptr->pub.state[ i ] == QCRIL_REQ_COMPLETED_FAILURE ) )
      {
        modem_ids_done_list->modem_id[ modem_ids_done_list->num_of_modems++ ] = i;
        if ( buf_ptr->pub.state[ i ] == QCRIL_REQ_COMPLETED_FAILURE ) 
        {
          *result = E_FAILURE;
        }
      }
      else if ( ( buf_ptr->pub.state[ i ] != QCRIL_REQ_FREE ) && 
                ( buf_ptr->pub.state[ i ] != QCRIL_REQ_COMPLETED_SUCCESS ) &&
                ( buf_ptr->pub.state[ i ] != QCRIL_REQ_COMPLETED_FAILURE ) )
      {
        status = E_IN_PROGRESS;
      }
    }

    if ( status == E_SUCCESS )
    {
      QCRIL_LOG_DEBUG( "[RID %d] Complete all AMSS events, ReqList entry : %s(%d), token id %d, result %d\n", 
                       instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request, 
                       qcril_log_get_token_id( buf_ptr->pub.t ), *result );
    }
  }
  else
  {
    /* Token not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  return status;

} /* qcril_reqlist_complete_all_amss_events() */
