/*!
  @file
  qcril_data_client.c

  @brief
  Implements client API for other QCRIL modules.

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/16/12   ar     Initial version

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#ifdef QCRIL_DATA_OFFTARGET
#include <netinet/in.h>
#endif
#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "qcril_data_utils.h"
#include "qcril_data_client.h"


#define QCRIL_DATA_CLIENT_HNDL_MASK       (0xFFFF0000)

#define GET_INDEX(hndl)     ((~QCRIL_DATA_CLIENT_HNDL_MASK) & hndl)
#define IS_VALID_HNDL(hndl) (QCRIL_DATA_MAX_CLIENTS_SUPPORTED > GET_INDEX(hndl))

typedef struct qcril_data_client_s
{
  qcril_data_hndl_t    hndl;
  qcril_data_net_ev_cb cb_f;
  void                *user_data;
} qcril_data_client_t;

struct {
  int                  cnt;
  qcril_data_client_t  tbl[QCRIL_DATA_MAX_CLIENTS_SUPPORTED];
  pthread_mutex_t      mutex;
} qcril_data_client_info;


/*===========================================================================

  FUNCTION:  qcril_data_client_register

===========================================================================*/
/*!
    @brief
    Used to register client callback function and cookie.  Each client
    will be allocated a unique opaque client handle.

    @return
    qcril_data_client_t if successful,
    QCRIL_DATA_CLIENT_HNDL_INVALID otherwise
*/
/*=========================================================================*/
qcril_data_hndl_t qcril_data_client_register
(
  qcril_data_net_ev_cb  cb_fn,
  void                 *user_data
)
{
  qcril_data_hndl_t  ret = QCRIL_DATA_CLIENT_HNDL_INVALID;
  int i;

  QCRIL_LOG_VERBOSE( "%s : ENTRY", __func__ );

  do {
    if( !cb_fn )
    {
      QCRIL_LOG_ERROR( "%s", "Error: NULL client callback function");
      break;
    }

    if( QCRIL_DATA_MAX_CLIENTS_SUPPORTED <= qcril_data_client_info.cnt )
    {
      QCRIL_LOG_ERROR( "%s", "Error: maximum number of clients registered");
      break;
    }

    QCRIL_DATA_MUTEX_LOCK(&qcril_data_client_info.mutex);

    /* Find free client record*/
    for( i=0; i<QCRIL_DATA_MAX_CLIENTS_SUPPORTED; i++ )
    {
      if( QCRIL_DATA_CLIENT_HNDL_INVALID == qcril_data_client_info.tbl[i].hndl )
      {
        /* Create opaque handle */
        ret = (QCRIL_DATA_CLIENT_HNDL_MASK | i);

        /* Preserve client parameters */
        qcril_data_client_info.tbl[i].hndl = ret;
        qcril_data_client_info.tbl[i].cb_f = cb_fn;
        qcril_data_client_info.tbl[i].user_data = user_data;
        QCRIL_LOG_INFO( "Allocated client[%d] using handle[0x%08x]", i, ret );

        qcril_data_client_info.cnt++;
        break;
      }
    }

    QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_client_info.mutex);

    if( QCRIL_DATA_MAX_CLIENTS_SUPPORTED < i )
    {
      QCRIL_LOG_ERROR( "%s", "Error: cannot find free client record");
      break;
    }

  } while (0);

  QCRIL_LOG_VERBOSE( "%s : EXIT", __func__ );
  return ret;
} /* qcril_data_client_register() */


/*===========================================================================

  FUNCTION:  qcril_data_client_release

===========================================================================*/
/*!
    @brief
    Used to release client registration.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_client_release
(
  qcril_data_hndl_t   hndl
)
{
  int index = 0;

  QCRIL_LOG_VERBOSE( "%s : ENTRY", __func__ );

  do {
    if( !IS_VALID_HNDL(hndl) )
    {
      QCRIL_LOG_ERROR( "Error: invalid client handle specified [0x%08x]", hndl);
      break;
    }

    QCRIL_DATA_MUTEX_LOCK(&qcril_data_client_info.mutex);

    /* Get table index from opaque handle */
    index = GET_INDEX(hndl);

    /* Clear client parameters */
    memset( &qcril_data_client_info.tbl[index], 0x0, sizeof(qcril_data_client_t) );
    qcril_data_client_info.tbl[index].hndl = QCRIL_DATA_CLIENT_HNDL_INVALID;
    QCRIL_LOG_INFO( "Cleared client[%d] using handel[0x%08x]", index, hndl );

    qcril_data_client_info.cnt--;

    QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_client_info.mutex);

  } while (0);

  QCRIL_LOG_VERBOSE( "%s : EXIT", __func__ );
  return;
} /* qcril_data_client_release() */


/*===========================================================================

  FUNCTION:  qcril_data_client_notify

===========================================================================*/
/*!
    @brief
    Invoke the client's registered callback (if any) to pass the
    specified event and payload.

    @return
    QCRIL_DATA_SUCCESS on successful operation,
    QCRIL_DATA_FAILURE otherwise
*/
/*=========================================================================*/
int qcril_data_client_notify
(
  qcril_data_net_evt_t      evt,
  qcril_data_evt_payload_t *payload
)
{
  int ret = QCRIL_DATA_FAILURE;
  int i;

  QCRIL_LOG_VERBOSE( "%s : ENTRY", __func__ );

  do {
    QCRIL_DATA_MUTEX_LOCK(&qcril_data_client_info.mutex);

    /* Loop over client records */
    for( i=0; i<QCRIL_DATA_MAX_CLIENTS_SUPPORTED; i++ )
    {
      if( (QCRIL_DATA_CLIENT_HNDL_INVALID != qcril_data_client_info.tbl[i].hndl) &&
          qcril_data_client_info.tbl[i].cb_f )
      {
        qcril_data_client_info.tbl[i].cb_f( qcril_data_client_info.tbl[i].hndl,
                                            qcril_data_client_info.tbl[i].user_data,
                                            evt, payload );
        QCRIL_LOG_INFO( "Posting event[%d] payload[0x%x] to client[%08x]",
                        evt, payload, qcril_data_client_info.tbl[i].hndl );
        ret = QCRIL_DATA_SUCCESS;
      }
    }

    QCRIL_DATA_MUTEX_UNLOCK(&qcril_data_client_info.mutex);

  } while (0);

  QCRIL_LOG_VERBOSE( "%s : EXIT", __func__ );
  return ret;
}


/*===========================================================================

  FUNCTION:  qcril_data_get_active_calls

===========================================================================*/
/*!
    @brief
    Used to query the current acive calls within QCRIL-Data module.
    The caller should pass a call_list array of dimension
    QCRIL_DATA_MAX_CALL_RECORDS.  If call_list is NULL, only the
    num_calls is updated.

    @return
    QCRIL_DATA_SUCCESS on successful operation,
    QCRIL_DATA_FAILURE otherwise
*/
/*=========================================================================*/
int qcril_data_get_active_calls
(
  qcril_data_hndl_t              hndl,
  unsigned int                  *num_calls,
  qcril_data_active_call_info_t *call_list
)
{
  int i, index = 0;

  QCRIL_LOG_VERBOSE( "%s : ENTRY", __func__ );

  do {
    if( !IS_VALID_HNDL(hndl) )
    {
      QCRIL_LOG_ERROR( "Error: invalid client handle specified [0x%08x]", hndl);
      break;
    }

    QCRIL_DATA_MUTEX_LOCK( &info_tbl_mutex );

    /* Loop over call table */
    for( i=0; i<QCRIL_DATA_MAX_CALL_RECORDS; i++ )
    {
      if( CALL_ID_INVALID != info_tbl[i].cid )
      {
        /* If called passed array, populate next call record */
        if( call_list )
        {
          call_list[ index ].call_id   = info_tbl[i].cid;
          call_list[ index ].radioTech = info_tbl[i].call_info.radioTech;
          call_list[ index ].address   = info_tbl[i].call_info.address;
          call_list[ index ].active    = info_tbl[i].call_info.active;
          memcpy( &call_list[ index ].apn,
                  info_tbl[i].call_info.apn,
                  sizeof(call_list[ index ].apn) );
          memcpy( &call_list[ index ].dev_name,
                  info_tbl[i].call_info.apn,
                  sizeof(call_list[ index ].dev_name) );
        }
        index++;
      }
    } /* for */

    QCRIL_DATA_MUTEX_UNLOCK( &info_tbl_mutex );
  } while (0);

  *num_calls = index;
  QCRIL_LOG_INFO( "Query reported [%d] call records, table[%p]",
                  *num_calls, call_list );

  QCRIL_LOG_VERBOSE( "%s : EXIT", __func__ );
  return QCRIL_DATA_SUCCESS;
}




/*===========================================================================

  FUNCTION:  qcril_data_client_init

===========================================================================*/
/*!
    @brief
    Initialize the client module

    @return
    None
*/
/*=========================================================================*/
void qcril_data_client_init ( void )
{
  int i;
  pthread_mutexattr_t mutex_attr;

  QCRIL_LOG_VERBOSE( "%s : ENTRY", __func__ );

  do {
    /* Initialize the client info structure */
    memset( &qcril_data_client_info, 0x0, sizeof( qcril_data_client_info ) );

    /* Set each clinet record invalid */
    for( i=0; i<QCRIL_DATA_MAX_CLIENTS_SUPPORTED; i++ )
    {
      qcril_data_client_info.tbl[i].hndl = QCRIL_DATA_CLIENT_HNDL_INVALID;
    }

    /* Initialize mutex */
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init( &qcril_data_client_info.mutex, &mutex_attr );

  } while (0);

  QCRIL_LOG_VERBOSE( "%s : EXIT", __func__ );
  return;
}


