/******************************************************************************
  @file    qcril_reqlist.c
  @brief   qcril qmi - request list

  DESCRIPTION
    Maintains a list of outstanding RIL_REQUESTs and pairs them with the
    response. Maintains a timer so requests don't get stuck forever.

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


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

/*! Variables internal to module qcril_reqlist.c
*/

void qcril_reqlist_print_all(qcril_instance_id_e_type instance_id);

static qcril_reqlist_buf_type reseverd_req_buf[ QCRIL_MAX_INSTANCE_ID ][QCRIL_REQLIST_RESERVERED_REQ_BUF];

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

static void qcril_reqlist_clear_reqlist_public_type(qcril_reqlist_public_type* req_pub)
{
  QCRIL_LOG_FUNC_ENTRY();
  int i;

  if( req_pub )
  {
    for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
    {
        req_pub->state[ i ] = QCRIL_REQ_FREE;
    }
    if ( req_pub->req_data )
    {
      QCRIL_LOG_INFO("Will free req_data now.");
      qcril_free(req_pub->req_data);
      req_pub->req_data = NULL;
      QCRIL_LOG_INFO("freed req_data for request: %d, token id: %d", req_pub->request, qcril_log_get_token_id(req_pub->t));
    }
  }
  else
  {
    QCRIL_LOG_FATAL("Null pointer passed for req_pub");
  }

  QCRIL_LOG_FUNC_RETURN();
}

static void qcril_reqlist_copy_reqlist_public_type(qcril_reqlist_public_type* dest, const qcril_reqlist_public_type* src)
{
  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL == dest || NULL == src )
  {
    QCRIL_LOG_DEBUG("dest is null or src is null");
  }
  else
  {
    qcril_reqlist_clear_reqlist_public_type(dest);
    boolean allocated_on_heap = dest->allocated_on_heap;
    *dest = *src;
    dest->allocated_on_heap = allocated_on_heap;

    if ( NULL != src->req_data && 0 != src->req_datalen)
    {
      dest->req_data = qcril_malloc(src->req_datalen);
      if ( NULL != dest->req_data )
      {
        memcpy(dest->req_data, src->req_data, src->req_datalen);
        QCRIL_LOG_INFO("copied req_data for request: %d, token id: %d", dest->request, qcril_log_get_token_id(dest->t));
      }
      else
      {
        QCRIL_LOG_ERROR("out of memory");
        dest->req_datalen = 0;
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

static void qcril_reqlist_free_req_in_reserved_buffer(qcril_reqlist_buf_type* req_buf)
{
  QCRIL_LOG_FUNC_ENTRY();
  if ( NULL != req_buf )
  {
    qcril_reqlist_clear_reqlist_public_type(&req_buf->pub);
    QCRIL_LOG_INFO("clear pointers");
    req_buf->next_ptr = NULL;
    req_buf->prev_ptr = NULL;
    req_buf->following_ptr = NULL;
    req_buf->followed_ptr = NULL;
  }
  else
  {
    QCRIL_LOG_ERROR("free null pointer req_buf");
  }
  QCRIL_LOG_FUNC_RETURN();
}

static void qcril_reqlist_free_req_on_heap(qcril_reqlist_buf_type* req_buf)
{
  QCRIL_LOG_FUNC_ENTRY();
  if ( NULL != req_buf )
  {
    qcril_reqlist_clear_reqlist_public_type(&req_buf->pub);
    QCRIL_LOG_INFO("Will free req_buf now.");
    qcril_free(req_buf);
  }
  else
  {
    QCRIL_LOG_ERROR("free null pointer req_buf");
  }
  QCRIL_LOG_FUNC_RETURN();
}

static void qcril_reqlist_free_req(qcril_reqlist_buf_type* req_buf)
{
  QCRIL_LOG_FUNC_ENTRY();
  if ( NULL != req_buf )
  {
    if ( FALSE == req_buf->pub.allocated_on_heap )
    {
      qcril_reqlist_free_req_in_reserved_buffer(req_buf);
    }
    else
    {
      qcril_reqlist_free_req_on_heap(req_buf);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

static boolean qcril_reqlist_is_request_blocked(const qcril_reqlist_public_type *req_pub)
{
  boolean ret = FALSE;
  if ( NULL != req_pub && QCRIL_REQ_CONCURRENCY_STATE_BLOCKED == req_pub->con_state)
  {
    ret = TRUE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)ret );
  return ret;
}

static boolean qcril_reqlist_is_request_outstanding(const qcril_reqlist_public_type *req_pub)
{
  boolean ret = FALSE;
  if ( NULL != req_pub && QCRIL_REQ_CONCURRENCY_STATE_DISPATCHED == req_pub->con_state)
  {
    ret = TRUE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)ret );
  return ret;
}

static qcril_reqlist_buf_type* qcril_reqlist_find_following_blocked_request_buf(const qcril_reqlist_buf_type* buf_ptr)
{
  QCRIL_LOG_FUNC_ENTRY();
  const qcril_reqlist_buf_type* tmp = NULL;

  if ( NULL != buf_ptr )
  {
    tmp = buf_ptr->following_ptr;
  }

  while ( NULL != tmp )
  {
    if ( qcril_reqlist_is_request_blocked( &tmp->pub) )
    {
      break;
    }
    tmp = tmp->following_ptr;
  }

  if ( NULL != tmp )
  {
    QCRIL_LOG_INFO("for request %d (token id %d), find the blocked request %d (token id %d)",
                   buf_ptr->pub.request, qcril_log_get_token_id(buf_ptr->pub.t), tmp->pub.request, qcril_log_get_token_id(tmp->pub.t));
  }
  else if ( NULL != buf_ptr )
  {
    QCRIL_LOG_INFO("for request %d (token id %d), didn't find the blocked request", buf_ptr->pub.request, qcril_log_get_token_id(buf_ptr->pub.t));
  }
  else
  {
    QCRIL_LOG_DEBUG("the input buf_ptr is NULL");
  }
  QCRIL_LOG_FUNC_RETURN();

  return (qcril_reqlist_buf_type*)tmp;
}

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
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d",
                      instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                      qcril_log_get_token_id( t ) );
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry : token id %d", instance_id, qcril_log_get_token_id( t ) );

  return NULL;

} /* qcril_reqlist_find() */

/*===========================================================================

  FUNCTION:  qcril_reqlist_find_by_request

===========================================================================*/
/*!
    @brief
    For the given set of request_ids, find the first entry in the list whose request_id
    is equal to either of the one in request_ids

    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_reqlist_buf_type *qcril_reqlist_find_by_requests
(
  qcril_instance_id_e_type instance_id,
  int *requests,
  size_t num
)
{
  qcril_reqlist_buf_type *buf_ptr;
  size_t i;

  QCRIL_LOG_FUNC_ENTRY();

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_reqlist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    for ( i=0; i<num; i++ )
    {
      if ( buf_ptr->pub.request == requests[i] )
      {
        QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d",
                        instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                        qcril_log_get_token_id( buf_ptr->pub.t ) );
        return buf_ptr;
      }
    }
    buf_ptr = buf_ptr->next_ptr;
  }

  for ( i=0; i<num; i++ )
  {
    QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry for %s(%d)",
                    instance_id, qcril_log_lookup_event_name( requests[i] ), requests[i] );
  }

  return NULL;

} /* qcril_reqlist_find_by_requests() */

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
  return qcril_reqlist_find_by_requests(instance_id, &request, 1);
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
        QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d",
                        i, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                        qcril_log_get_token_id( buf_ptr->pub.t ) );

        if ( NULL != instance_id_ptr )
        {
          *instance_id_ptr = i;
        }

        return buf_ptr;
      }
      else
      {
        buf_ptr = buf_ptr->next_ptr;
      }
    }

    QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry : req id %d", i, req_id );
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
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d, pending %s (%lu)",
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

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry waiting for %s (%lu)",
                  instance_id, qcril_log_lookup_event_name( pending_event_id ), pending_event_id );

  return NULL;

} /* qcril_reqlist_find_by_event() */


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
      QCRIL_LOG_INFO( "[RID %d] Found ReqList entry : %s(%d), token id %d, pending %s (%lu), sub id %lu",
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

  QCRIL_LOG_INFO( "[RID %d] Not found ReqList entry waiting for %s (%lu) and sub id %lu",
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
  uint16 i;
  uint8 j;
  boolean found = FALSE;
  qcril_reqlist_buf_type *buf_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  for ( i = 0; i < QCRIL_REQLIST_RESERVERED_REQ_BUF && !found ; i++ )
  {
    found = TRUE;
    buf_ptr = &reseverd_req_buf[ instance_id ][ i ];
    for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
    {
      if ( buf_ptr->pub.state[ j ] != QCRIL_REQ_FREE )
      {
        found = FALSE;
        break;
      }
    }
  }

  if ( found )
  {
    buf_ptr->pub.allocated_on_heap = FALSE;
  }
  else
  {
    buf_ptr = qcril_malloc(sizeof(*buf_ptr));
    if ( NULL == buf_ptr )
    {
      QCRIL_LOG_ERROR("out of memory");
    }
    else
    {
      QCRIL_LOG_INFO("new a req_buf on heap.");
      buf_ptr->pub.allocated_on_heap = TRUE;
    }
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
  qcril_reqlist_public_type *entry_ptr,
  qcril_reqlist_buf_type *followed_req_ptr,
  qcril_reqlist_buf_type **ret_ptr
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
    QCRIL_LOG_ERROR( "[RID %d] Fail to add ReqList entry with token id %d [%p] (already exists)",
                     instance_id, qcril_log_get_token_id( buf_ptr->pub.t ), buf_ptr->pub.t );
    return E_NOT_ALLOWED;
  }

  buf_ptr = qcril_reqlist_new_buf( instance_id );
  if ( NULL != ret_ptr )
  {
    *ret_ptr = buf_ptr;
  }

  if ( buf_ptr == NULL )
  {
    /* No free buffers in the list */
    QCRIL_LOG_ERROR( "[RID %d] Fail to add ReqList entry for token id %d [%p] (no buffer)",
                     instance_id, qcril_log_get_token_id( entry_ptr->t ), entry_ptr->t );
    return E_NO_MEMORY;
  }

  pub_ptr = &buf_ptr->pub;

  qcril_reqlist_copy_reqlist_public_type(pub_ptr, entry_ptr);

  /* Sort items in the ReqList by the order of RIL command being received */
  buf_ptr->pub.req_id = req_id++;
  buf_ptr->prev_ptr = qcril_reqlist[ instance_id ].tail_ptr;
  buf_ptr->next_ptr = NULL;
  buf_ptr->followed_ptr = followed_req_ptr;

  if ( NULL != followed_req_ptr )
  {
    followed_req_ptr->following_ptr = buf_ptr;
  }

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
  entry_ptr->req_id = buf_ptr->pub.req_id;

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
  qcril_reqlist_print_all(instance_id);
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
  uint8 i, j;
  uint16 buf_idx;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  memset( &qcril_reqlist, 0, sizeof( qcril_reqlist ) );

  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    for ( buf_idx = 0; buf_idx < QCRIL_REQLIST_RESERVERED_REQ_BUF; buf_idx++ )
    {
      for ( j = 0 ; j < QCRIL_ARB_MAX_MODEMS; j++ )
      {
        reseverd_req_buf[ i ][ buf_idx ].pub.state[ j ] = QCRIL_REQ_FREE;
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
    QCRIL_LOG_DEBUG( "[RID %d] ReqList entries : Empty", instance_id );
  }
  else
  {
    QCRIL_LOG_DEBUG( "[RID %d] ReqList entries :", instance_id );
  }

  while ( buf_ptr != NULL )
  {
    QCRIL_LOG_DEBUG( "    %s (%d), token id %d followed %p following %p",
                     qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                     qcril_log_get_token_id( buf_ptr->pub.t ),
                     buf_ptr->followed_ptr, buf_ptr->following_ptr );
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
  QCRIL_LOG_FUNC_ENTRY();

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( req_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if( req_ptr )
  {
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
  }
  else
  {
    QCRIL_LOG_FATAL("Null pointer passed for req_ptr");
  }

  QCRIL_LOG_FUNC_RETURN();
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
  IxErrnoType                    status;
  qcril_request_resp_params_type resp;
  int                            must_send_failure = FALSE;
  QCRIL_LOG_FUNC_ENTRY();

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  if ( NULL != entry_ptr )
  {
    entry_ptr->con_state = QCRIL_REQ_CONCURRENCY_STATE_DISPATCHED;
    status = qcril_reqlist_new_all( instance_id, entry_ptr, NULL, NULL );
    if ( status == E_SUCCESS )
    {
      qcril_reqlist_print_all( instance_id );
    }
    else if ( ( entry_ptr->t != NULL ) && ( entry_ptr->t != ( void *) QCRIL_TOKEN_ID_INTERNAL ) )
    {
      must_send_failure = TRUE;
    }
  }
  else
  {
    status = E_INVALID_ARG;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  if ( must_send_failure )
  { // generally we do so for backwards compatibilty with Data RIL and UIM RIL as QMI RIL "new" code would send failure directly anway
    qcril_default_request_resp_params( instance_id, entry_ptr->t, entry_ptr->request, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)status );
  return status;

} /* qcril_reqlist_new() */

IxErrnoType qcril_reqlist_new_with_concurency_control
(
  qcril_instance_id_e_type instance_id,
  qcril_reqlist_public_type *entry_ptr,
  qcril_reqlist_check_concurrency_handler check_concurrency_handler,
  void *check_dispatchable_handler_data,
  size_t check_dispatchable_handler_datalen,
  qcril_reqlist_dispatch_blocked_req_handler dispatch_blocked_req_handler,
  void *req_data,
  size_t req_datalen,
  qcril_reqlist_buf_type **ret_req_buf
)
{
  IxErrnoType                    status = E_FAILURE;
  qcril_request_resp_params_type resp;
  int                            must_send_failure = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  qcril_reqlist_buf_type *followed_req = NULL;
  qcril_req_concurrency_state_e_type con_state = check_concurrency_handler(instance_id, entry_ptr, check_dispatchable_handler_data,
                                                                    check_dispatchable_handler_datalen, &followed_req);

  if ( QCRIL_REQ_CONCURRENCY_STATE_REJECTED == con_state )
  {
    must_send_failure = TRUE;
  }
  else
  {
    if ( NULL != entry_ptr )
    {
      entry_ptr->con_state = con_state;
      if ( QCRIL_REQ_CONCURRENCY_STATE_BLOCKED == con_state )
      {
        entry_ptr->handler = dispatch_blocked_req_handler;
        if ( NULL != req_data && 0 != req_datalen )
        {
          entry_ptr->req_data = qcril_malloc(req_datalen);
          if ( NULL != entry_ptr->req_data )
          {
            memcpy(entry_ptr->req_data, req_data, req_datalen);
            entry_ptr->req_datalen = req_datalen;
          }
          else
          {
            entry_ptr->req_datalen = 0;
          }
        }
      }
      status = qcril_reqlist_new_all( instance_id, entry_ptr, followed_req, ret_req_buf );

      if ( status == E_SUCCESS )
      {
        qcril_reqlist_print_all( instance_id );
        must_send_failure = FALSE;

        if ( QCRIL_REQ_CONCURRENCY_STATE_BLOCKED == con_state )
        {
          status = E_BLOCKED_BY_OUTSTANDING_REQ;
        }
      }
      else if ( ( entry_ptr->t != NULL ) && ( entry_ptr->t != ( void *) QCRIL_TOKEN_ID_INTERNAL ) )
      {
        must_send_failure = TRUE;
      }
    }
    else
    {
      status = E_INVALID_ARG;
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  if ( must_send_failure )
  { // generally we do so for backwards compatibilty with Data RIL and UIM RIL as QMI RIL "new" code would send failure directly anway
    qcril_default_request_resp_params( instance_id, entry_ptr->t, entry_ptr->request, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)status );
  return status;
}

qcril_reqlist_buf_type* qcril_reqlist_get_root_followed_req( qcril_reqlist_buf_type* req_buf )
{
  qcril_reqlist_buf_type *root_req_buf = NULL;
  qcril_reqlist_buf_type *req_buf_itr;
  if ( NULL != req_buf && req_buf->followed_ptr != NULL )
  {
    req_buf_itr = req_buf->followed_ptr;
    while ( NULL != req_buf_itr )
    {
      root_req_buf = req_buf_itr;
      req_buf_itr = req_buf_itr->followed_ptr;
    }
  }
  QCRIL_LOG_DEBUG("the root ptr is :p", root_req_buf);
  return root_req_buf;
}

static IxErrnoType qcril_reqlist_remove_request_pub_from_reqlist(qcril_reqlist_buf_type *buf_ptr, qcril_instance_id_e_type instance_id)
{
  IxErrnoType status = E_FAILURE;
  qcril_reqlist_buf_type *prev_buf_ptr, *next_buf_ptr, *following_buf_ptr, *followed_buf_ptr;

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != buf_ptr )
  {
    /* Detach the item from the ReqList */
    prev_buf_ptr = buf_ptr->prev_ptr;
    next_buf_ptr = buf_ptr->next_ptr;
    following_buf_ptr = buf_ptr->following_ptr;
    followed_buf_ptr = buf_ptr->followed_ptr;

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

    if ( NULL != followed_buf_ptr )
    {
      followed_buf_ptr->following_ptr = following_buf_ptr;
    }
    if ( NULL != following_buf_ptr )
    {
      following_buf_ptr->followed_ptr = followed_buf_ptr;
    }

    QCRIL_LOG_INFO( "[RID %d] Deleted ReqList entry : token id %d [%p]", instance_id, qcril_log_get_token_id( buf_ptr->pub.t ), buf_ptr->pub.t );

    qcril_reqlist_free_req(buf_ptr);

    qcril_reqlist_print_all( instance_id );
    status = E_SUCCESS;
  }
  return status;
}

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
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;
  QCRIL_LOG_FUNC_ENTRY();

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  status = qcril_reqlist_remove_request_pub_from_reqlist(buf_ptr, instance_id);

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) status );
  return status;
} /* qcril_reqlist_free() */

IxErrnoType qcril_reqlist_free_and_dispatch_follower_req
(
  RIL_Token t,
  qcril_instance_id_e_type instance_id,
  void *data,
  size_t datalen
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_reqlist_buf_type *buf_ptr;
  qcril_reqlist_buf_type *following_blocked_buf_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED(data);
  QCRIL_NOTUSED(datalen);
  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    following_blocked_buf_ptr = qcril_reqlist_find_following_blocked_request_buf(buf_ptr);
  }

  status = qcril_reqlist_remove_request_pub_from_reqlist(buf_ptr, instance_id);

  if ( following_blocked_buf_ptr != NULL )
  {
    if ( following_blocked_buf_ptr->pub.handler )
    {
      QCRIL_LOG_INFO( "invoking handler" );
      following_blocked_buf_ptr->pub.handler_invoked_under_mtx_lock = TRUE;
      following_blocked_buf_ptr->pub.con_state = QCRIL_REQ_CONCURRENCY_STATE_DISPATCHED;
      following_blocked_buf_ptr->pub.handler(instance_id,
                                             following_blocked_buf_ptr,
                                             following_blocked_buf_ptr->pub.req_data,
                                             following_blocked_buf_ptr->pub.req_datalen );
      following_blocked_buf_ptr->pub.handler_invoked_under_mtx_lock = FALSE;
      QCRIL_LOG_INFO( "handler returned" );
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) status );
  return status;
}

boolean qcril_reqlist_has_follower( qcril_instance_id_e_type instance_id, RIL_Token t )
{
  boolean ret = FALSE;

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  qcril_reqlist_buf_type *buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( NULL != buf_ptr && NULL != buf_ptr->following_ptr )
  {
    ret = TRUE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
}

int qcril_reqlist_under_follower_handler_exec( qcril_instance_id_e_type instance_id, RIL_Token t )
{
  int res = FALSE;

  qcril_reqlist_buf_type *buf_ptr = qcril_reqlist_find( instance_id, t );
  if ( NULL != buf_ptr && buf_ptr->pub.handler_invoked_under_mtx_lock )
  {
    res = TRUE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

RIL_Token qcril_reqlist_get_follower_token( qcril_instance_id_e_type instance_id, RIL_Token t )
{
  RIL_Token res;

  qcril_reqlist_buf_type *buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( NULL != buf_ptr && NULL != buf_ptr->following_ptr )
  {
    res = buf_ptr->following_ptr->pub.t;
  }
  else
  {
    res = QMI_RIL_ZERO;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

int qcril_reqlist_is_auto_respond_duplicate( qcril_instance_id_e_type instance_id, RIL_Token t )
{
  int res = FALSE;

  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  qcril_reqlist_buf_type *buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( NULL != buf_ptr && buf_ptr->pub.auto_respond_duplicate )
  {
    res = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

void qcril_reqlist_set_auto_respond_duplicate( qcril_instance_id_e_type instance_id, RIL_Token t )
{
  QCRIL_MUTEX_LOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  qcril_reqlist_buf_type *buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( NULL != buf_ptr )
  {
    buf_ptr->pub.auto_respond_duplicate = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_reqlist_mutex, "qcril_reqlist_mutex" );

  QCRIL_LOG_FUNC_RETURN();
}


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
      QCRIL_LOG_INFO( "[RID %d] Update ReqList entry : %s(%d), token id %d, state %s(%d)",
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
    QCRIL_LOG_INFO( "[RID %d] Update event field of ReqList entry : %s(%d), token id %d, MID %d pending %s (%d)",
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
    QCRIL_LOG_INFO( "[RID %d] Update sub id field of ReqList entry : %s(%d), token id %d, sub id %lu",
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( NULL != buf_ptr && NULL != info_ptr )
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

  if ( buf_ptr != NULL && NULL != sub_ptr )
  {
    buf_ptr->pub.sub = *sub_ptr;

    QCRIL_LOG_INFO( "[RID %d] Update sub info of ReqList entry : %s(%d), token id %d",
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

  if ( NULL != modem_ids_done_list )
  {
    modem_ids_done_list->num_of_modems = 0;
  }

  buf_ptr = qcril_reqlist_find( instance_id, t );

  if ( buf_ptr != NULL )
  {
    buf_ptr->pub.state[ modem_id ] = state;

    for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
    {
      QCRIL_LOG_DEBUG( "[RID %d] %s(%d) token id %d, MID %d, State %s(%d)",
                       instance_id, qcril_log_lookup_event_name( buf_ptr->pub.request ), buf_ptr->pub.request,
                       qcril_log_get_token_id( buf_ptr->pub.t ), i, qcril_reqlist_lookup_state_name( buf_ptr->pub.state[ i ] ),
                       buf_ptr->pub.state[ i ] );

      if ( ( buf_ptr->pub.state[ i ] == QCRIL_REQ_COMPLETED_SUCCESS ) ||
           ( buf_ptr->pub.state[ i ] == QCRIL_REQ_COMPLETED_FAILURE ) )
      {
        if ( NULL != modem_ids_done_list )
        {
          modem_ids_done_list->modem_id[ modem_ids_done_list->num_of_modems++ ] = i;
        }
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
      QCRIL_LOG_DEBUG( "[RID %d] Complete all AMSS events, ReqList entry : %s(%d), token id %d, result %d",
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

void qcril_reqlist_reply_blocked_req_with_exsting_result
(
  qcril_instance_id_e_type instance_id,
  qcril_reqlist_buf_type *req,
  void *data,
  size_t datalen
)
{
  QCRIL_NOTUSED(datalen);
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  qcril_request_resp_params_type param = *((qcril_request_resp_params_type *) data);
  param.t = req->pub.t;
  param.request_id = req->pub.request;
  qcril_send_request_response(&param);
}

qcril_req_concurrency_state_e_type qcril_reqlist_generic_check_concurrency_from_set_of_requests
(
  qcril_instance_id_e_type instance_id,
  qcril_reqlist_public_type *req,
  void *data,
  size_t datalen,
  qcril_reqlist_buf_type **followed_req
)
{
  QCRIL_NOTUSED(datalen);
  qcril_req_concurrency_state_e_type ret = QCRIL_REQ_CONCURRENCY_STATE_UNKNOWN;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(req);
  if ( NULL != data )
  {
    qcril_reqlist_generic_concurency_requests_requirement_type *requests_requirement = data;
    uint16 max_concurrency = requests_requirement->max_concurrency;
    uint16 max_pending = requests_requirement->max_pending;
    uint16 req_ids_num = requests_requirement->req_ids_num;

    int concurrent_req_count = 0;
    int pending_req_count = 0;
    qcril_reqlist_buf_type *last_req_pub = NULL;
    qcril_reqlist_buf_type *req_buf;
    *followed_req = NULL;

    req_buf = qcril_reqlist_find_by_requests(instance_id, (int*) requests_requirement->req_ids, req_ids_num);
    QCRIL_LOG_INFO("%p",req_buf);

    while ( req_buf != NULL )
    {
      last_req_pub = req_buf;
      if ( qcril_reqlist_is_request_blocked(&req_buf->pub) )
      {
        pending_req_count++;
        if ( concurrent_req_count >= max_concurrency && pending_req_count >= max_pending)
        {
          break;
        }
      }
      else if ( qcril_reqlist_is_request_outstanding(&req_buf->pub) )
      {
        concurrent_req_count++;
      }
      req_buf = req_buf->following_ptr;
    }

    if ( concurrent_req_count < max_concurrency )
    {
      *followed_req = last_req_pub;
      ret = QCRIL_REQ_CONCURRENCY_STATE_DISPATCHED;
    }
    else
    {
      if ( pending_req_count >= max_pending )
      {
        *followed_req = NULL;
        ret = QCRIL_REQ_CONCURRENCY_STATE_REJECTED;
      }
      else
      {
        *followed_req = last_req_pub;
        ret = QCRIL_REQ_CONCURRENCY_STATE_BLOCKED;
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)ret );
  return ret;
}
