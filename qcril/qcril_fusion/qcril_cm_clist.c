/*!
  @file
  qcril_cm_clist.c

  @brief
  Encapsulates information related to active calls.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_clist.c#19 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
11/05/09   sb      Added support to replace call list entry info.
10/01/09   pg      Do not assign connection index for data calls.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
05/07/09   fc      Fixed the issue of RIL_UNSOLICITED_CALL_STATE_CHANGED not
                   being reported correctly for CM_CALL_EVENT_MNG_CALLS_CONF.
04/28/09   fc      Standardize xxx_enum_type to xxx_e_type.
04/05/09   fc      Cleanup log macros and mutex macros.
03/17/09   fc      Cleanup unreferenced header filed inclusion.
12/16/08   fc      Change to store call mode used in mobile originated or
                   mobile terminated call.
12/08/08   pg      Added support to pass CDMA voice privacy mode to RILD.
11/14/08   pg      Corrected a debug message.
10/21/08   pg      Accepting all voice calls including TEST and OTAPA.
05/28/08   pg      Moved QCRIL_CM_CLIST_BUF_MAX define to qcril_cmi.h
05/21/08   jar     Featurized for off target platform compilation
05/08/08   fc      First cut implementation.
05/05/08   da      Initial framework.


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include <cutils/memory.h>
#include "IxErrno.h"
#include "comdef.h"
#include "cm.h"
#include "qcrili.h"
#include "qcril_cmi.h"
#include "qcril_cm_clist.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

typedef struct qcril_cm_clist_buf_tag
{    
  qcril_cm_clist_public_type    pub;
  struct qcril_cm_clist_buf_tag *prev_ptr;
  struct qcril_cm_clist_buf_tag *next_ptr;
} qcril_cm_clist_buf_type;


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


/*! @brief Typedef variables internal to module qcril_cm_clist.c
*/
typedef struct
{
  qcril_cm_clist_buf_type call[ CM_CALL_ID_MAX ];
  qcril_cm_clist_buf_type *head_ptr; 
  qcril_cm_clist_buf_type *tail_ptr; 
} qcril_cm_clist_struct_type;

/*! @brief Variables internal to module qcril_cm_clist.c
*/
static qcril_cm_clist_struct_type qcril_cm_clist[ QCRIL_MAX_INSTANCE_ID ];

/* Mutex used to control simultaneous update/access to CList */
pthread_mutex_t qcril_cm_clist_mutex;


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                                FUNCTIONS

===========================================================================*/


/*===========================================================================

  FUNCTION:  qcril_cm_lookup_state_name

===========================================================================*/
/*!
    @brief
    Lookup state name.
 
    @return
    A pointer to the state name.
*/
/*=========================================================================*/
static char *qcril_cm_lookup_state_name
( 
  qcril_cm_clist_state_e_type state
)
{
  switch( state )
  {
    case QCRIL_CM_CLIST_STATE_IDLE:
      return "Idle";
    case QCRIL_CM_CLIST_STATE_ACTIVE:
      return "Active";
    case QCRIL_CM_CLIST_STATE_HOLDING:
      return "Holding";
    case QCRIL_CM_CLIST_STATE_DIALING:
      return "Dialing";
    case QCRIL_CM_CLIST_STATE_ALERTING:
      return "Alerting";
    case QCRIL_CM_CLIST_STATE_INCOMING:
      return "Incoming";
    case QCRIL_CM_CLIST_STATE_WAITING:
      return "Waiting";
    case QCRIL_CM_CLIST_STATE_SETUP:
      return "Setup";
    default:
      return "Unknown";
  } /* end switch */

} /* qcril_cm_lookup_state_name */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_find

===========================================================================*/
/*!
    @brief
    Find an entry in the list based on Call ID
 
    @return
    A pointer to the entry if found, NULL otherwise.
*/
/*=========================================================================*/
static qcril_cm_clist_buf_type *qcril_cm_clist_find
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id
)
{
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( ( buf_ptr->pub.modem_id == modem_id ) && ( buf_ptr->pub.call_id == call_id ) )
    {             
      QCRIL_LOG_INFO( "[RID %d] Found CList entry: call id %d, MID %d\n", instance_id, call_id, modem_id );
      return buf_ptr;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found CList entry : call id %d, MID %d\n", instance_id, call_id, modem_id );

  return NULL;

} /* qcril_cm_clist_find() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_find_available_conn_index

===========================================================================*/
/*!
    @brief
    Find the smallest unused Connection Index in the list.
 
    @return
    E_SUCCESS if there is Connection Index available 
    E_FAILURE if there is no Connection Index available
*/
/*=========================================================================*/
static IxErrnoType qcril_cm_clist_find_available_conn_index
( 
  qcril_instance_id_e_type instance_id,
  uint32 *conn_index_ptr 
)
{
  uint32 i;
  boolean available_conn_index[ CM_CALL_ID_MAX ];
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  memset( available_conn_index, TRUE, sizeof( available_conn_index ) );

  while ( buf_ptr != NULL )
  {
    /* Connection Index already assigned to another call */ 
    /* Check state and call type before mark it as used */
    if ( QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) && 
         ( buf_ptr->pub.state != QCRIL_CM_CLIST_STATE_IDLE ) ) 
    {
      available_conn_index[ buf_ptr->pub.conn_index - 1 ] = FALSE;
    }
    buf_ptr = buf_ptr->next_ptr;
  }

  /* Lookup the least value of available Connection Index */
  for ( i = 0; i < CM_CALL_ID_MAX; i++ )
  {
    if ( available_conn_index[ i ] )
    {
      *conn_index_ptr = i + 1;
      return E_SUCCESS;
    }
  }

  return E_FAILURE;

} /* qcril_cm_clist_find_available_conn_index() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_count_num_of_voice_calls_by_state

===========================================================================*/
/*!
    @brief
    Count the number of entries with CM_CALL_TYPE_VOICE or CM_CALL_TYPE_EMERGENCY 
    as call type with specified state in the list. 
 
    @return
*/
/*=========================================================================*/
static uint32 qcril_cm_clist_count_num_of_voice_calls_by_state
( 
  qcril_instance_id_e_type instance_id,
  uint32 state_mask
)
{
  uint32 num_of_calls = 0;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) &&
         ( buf_ptr->pub.state & state_mask ) )
    {             
      num_of_calls++;
    }

    buf_ptr = buf_ptr->next_ptr;
  }

  QCRIL_LOG_DEBUG( "[RID %d] Number of voice calls in CList (state mask %lu): %lu\n", instance_id, state_mask, num_of_calls );

  return num_of_calls;

} /* qcril_cm_clist_count_num_of_voice_calls_by_state() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_new_buf

===========================================================================*/
/*!
    @brief
    Allocate a new buffer for the list
 
    @return
    A pointer to a free buffer, or NULL if there are no free buffers 
    available.
*/
/*=========================================================================*/
static qcril_cm_clist_buf_type *qcril_cm_clist_new_buf
( 
  qcril_instance_id_e_type instance_id
)
{
  uint32 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  for ( i = 0; i < CM_CALL_ID_MAX; i++ )
  {
    if ( qcril_cm_clist[ instance_id ].call[ i ].pub.state == QCRIL_CM_CLIST_STATE_IDLE )
    {
      return &qcril_cm_clist[ instance_id ].call[ i ];
    }
  }

  return NULL;

} /* qcril_cm_clist_new_buf() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_new_all

===========================================================================*/
/*!
    @brief
    Add an entry to the list of active calls.
 
    @return
    E_SUCCESS if the entry is created properly
    E_NOT_ALLOWED if the entry was already in the list
    E_NO_MEMORY if there were no buffers
*/
/*=========================================================================*/
static IxErrnoType qcril_cm_clist_new_all
( 
  qcril_instance_id_e_type            instance_id,
  qcril_modem_id_e_type               modem_id,
  cm_call_id_type                     call_id,
  cm_call_type_e_type                 call_type,      
  cm_call_mode_info_e_type            call_mode,
  cm_call_direction_e_type            direction,
  qcril_cm_clist_state_e_type         state,
  cm_als_line_e_type                  line,           
  cm_num_s_type                       num,            
  qcril_cm_clist_public_type          *info_ptr
)
{ 
  uint32 conn_index;
  qcril_cm_clist_buf_type *buf_ptr;
  qcril_cm_clist_public_type *pub_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    /* Call ID is already in the list */
    QCRIL_LOG_ERROR( "[RID %d] Fail to add CList entry: call id=%d (duplicate)\n", instance_id, call_id );
    return E_NOT_ALLOWED;
  }

  buf_ptr = qcril_cm_clist_new_buf( instance_id );

  if ( buf_ptr == NULL )
  {
    /* No free buffers in the list */
    QCRIL_LOG_ERROR( "[RID %d] Fail to add CList entry: call id=%d (no buffer)\n", instance_id, call_id );
    return E_NO_MEMORY;
  }

  /* Initialize conn_index */
  conn_index = QCRIL_CM_CLIST_CONN_INDEX_NONE; 
  if ( QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( call_type ) )
  {
    /* Allocate the Connection Index */
    if ( qcril_cm_clist_find_available_conn_index( instance_id, &conn_index ) != E_SUCCESS )
    {
      /* No Connection Index is available */
      QCRIL_LOG_ERROR( "[RID %d] Fail to add CList entry: call id=%d (no available Conn Index)", instance_id, call_id );
      return E_NOT_ALLOWED;
    }
  }

  pub_ptr = &buf_ptr->pub;

  pub_ptr->modem_id = modem_id;
  pub_ptr->conn_index = conn_index;
  pub_ptr->prev_state = QCRIL_CM_CLIST_STATE_IDLE;
  pub_ptr->state = state;
  pub_ptr->call_id = call_id;
  pub_ptr->call_type = call_type;
  pub_ptr->call_mode = call_mode;
  pub_ptr->direction = direction;
  pub_ptr->line = line;           
  pub_ptr->num = num;
  pub_ptr->num.buf[ num.len ] = '\0';
  pub_ptr->answered = FALSE;
  pub_ptr->is_private = FALSE;
  pub_ptr->name[0] = '\0';
  pub_ptr->name_presentation = num.pi;
  pub_ptr->local_ringback = FALSE;
  pub_ptr->is_uus = FALSE;
  memset(&(pub_ptr->uus_data),0,sizeof(pub_ptr->uus_data));

  /* Sort items in the CList by the order of call being setup */
  buf_ptr->prev_ptr = qcril_cm_clist[ instance_id ].tail_ptr;
  buf_ptr->next_ptr = NULL;
  if ( qcril_cm_clist[ instance_id ].head_ptr == NULL )
  {
    qcril_cm_clist[ instance_id ].head_ptr = buf_ptr;
  }             
  if ( qcril_cm_clist[ instance_id ].tail_ptr != NULL )
  {
    qcril_cm_clist[ instance_id ].tail_ptr->next_ptr = buf_ptr;
  }
  qcril_cm_clist[ instance_id ].tail_ptr = buf_ptr;

  QCRIL_LOG_DEBUG( "[RID %d] Added CList entry : %s(%d), call id=%d, conn index=%lu, call type=%d, call mode=%d, direction=%d\n",
                   instance_id, qcril_cm_lookup_state_name( pub_ptr->state ), pub_ptr->state, pub_ptr->call_id, pub_ptr->conn_index, 
                   pub_ptr->call_type, pub_ptr->call_mode, pub_ptr->direction );

  QCRIL_LOG_DEBUG( "                           line=%d, number=%s, number presentation=%d, privacy mode=%d, "
                   "name=%s, name presentation=%d, local_ringback=%d, is_uus=%d \n",
                   pub_ptr->line, pub_ptr->num.buf, pub_ptr->num.pi, pub_ptr->is_private,
                   pub_ptr->name, pub_ptr->name_presentation, pub_ptr->local_ringback, pub_ptr->is_uus);

  if ( info_ptr != NULL )
  {
    *info_ptr = *pub_ptr;
  }

  return E_SUCCESS;

} /* qcril_cm_clist_new_all() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_log

===========================================================================*/
/*!
    @brief
    Log CList changes.
 
    @return
    none
*/
/*=========================================================================*/
void qcril_cm_clist_log
(
  qcril_instance_id_e_type instance_id,
  qcril_cm_clist_public_type *info_ptr
)
{
  char label[ 150 ];
  char *call_type[] = { "Voice", "CS Data", "PS Data", "SMS", "PD", "Test", "OTAPA", "Std OTASP", "Non-Std OTASP", 
                        "Emergency", "Sups", "VT", "VT Loopback", "VS", "PS Data IS707B" };
 
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( label, sizeof( label ), "RID %d MID %d, call id %d: %s -> %s, %s %s, %s, (%s)",
                  instance_id, info_ptr->modem_id, info_ptr->call_id, qcril_cm_lookup_state_name( info_ptr->prev_state ), 
                  qcril_cm_lookup_state_name( info_ptr->state ), 
                  ( ( info_ptr->direction == CM_CALL_DIRECTION_MO ) ? "MO" : "MT" ), 
                  ( ( info_ptr->call_type > CM_CALL_TYPE_PS_DATA_IS707B ) ? "Unknown" : call_type[ info_ptr->call_type ] ), 
                  ( info_ptr->answered ? "Answered" : "Not answered" ), 
                  ( info_ptr->is_private ? "Private" : "Public" ) );
  QCRIL_LOG_CF_PKT_RIL_ST_CHG( instance_id, label ); 

  QCRIL_LOG_DEBUG( "Changed in CList entry: %s\n", label ); 

} /* qcril_cm_clist_log */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_init

===========================================================================*/
/*!
    @brief
    Initialize CList.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_cm_clist_init
(
  void
)
{ 
  int i;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_cm_clist_init()\n" );

  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    memset( &qcril_cm_clist[ i ].call, 0, sizeof( qcril_cm_clist[ i ].call ) );
    qcril_cm_clist[ i ].head_ptr = NULL;
    qcril_cm_clist[ i ].tail_ptr = NULL;
  }

  pthread_mutex_init( &qcril_cm_clist_mutex, NULL );

} /* qcril_cm_clist_init */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_set_uus_data

===========================================================================*/
/*!
    @brief
    Set the uus present entry in the list with given Call ID

    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_set_uus_data
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  cm_call_event_user_data_s_type  *call_event_user_data
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( ( buf_ptr != NULL ) && ( call_event_user_data != NULL ) )
  {
    memset( &buf_ptr->pub.uus_data, 0, sizeof( cm_call_event_user_data_s_type ) );
    buf_ptr->pub.is_uus = TRUE;
    memcpy(&(buf_ptr->pub.uus_data),call_event_user_data,sizeof(cm_call_event_user_data_s_type));
    /* Log CList changes */
    QCRIL_LOG_INFO( "[RID %d] Change in call state of CList entry: call id %d, is_uus=%d, uus_len=%d, uus_data=%s, uus_type=%d\n",
                    instance_id, call_id, buf_ptr->pub.is_uus,buf_ptr->pub.uus_data.mt_user_data.user_user_data_length,
                    buf_ptr->pub.uus_data.mt_user_data.user_user_data,
                    buf_ptr->pub.uus_data.mt_user_data.user_user_type);
    QCRIL_LOG_INFO( "        uus_dcs=%d, uus_more_data_ind=%d, uus_reciver_busy=%d\n",
                    buf_ptr->pub.uus_data.mt_user_data.user_user_protocol_disc,
                    buf_ptr->pub.uus_data.mt_user_data.more_data_indicator,
                    buf_ptr->pub.uus_data.is_receiver_busy);
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  return status;

} /* qcril_cm_clist_set_uus_data() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_new

===========================================================================*/
/*!
    @brief
    Add an entry to the list of active calls.
 
    @return
    see qcril_cm_clist_new_all
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_new
( 
  qcril_instance_id_e_type             instance_id,
  qcril_modem_id_e_type                modem_id,
  const cm_mm_call_info_s_type         *call_info_ptr, 
  qcril_cm_clist_state_e_type          call_state, 
  qcril_cm_call_event_user_data_s_type *uus_ptr,
  boolean                              *unsol_call_state_changed_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  cm_num_s_type calling_num;
  cm_call_event_user_data_s_type call_event_user_data;
  qcril_cm_clist_state_e_type clist_call_state;
  qcril_cm_clist_public_type info;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  /* Set Clist entry's call state */
  clist_call_state = call_state;
  if ( call_state == QCRIL_CM_CLIST_STATE_INCOMING )
  {
    /* If in MPTY, state should be WAITING */
    if ( ( ( call_info_ptr->call_type == CM_CALL_TYPE_VOICE ) || ( call_info_ptr->call_type == CM_CALL_TYPE_EMERGENCY ) ) && 
         ( qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, QCRIL_CM_CLIST_STATE_ACTIVE ) > 0 ) )
    {
      clist_call_state = QCRIL_CM_CLIST_STATE_WAITING;
    }
  }

  /* Sanity check on Clist entry's call state */
  if ( ( clist_call_state != QCRIL_CM_CLIST_STATE_DIALING ) && ( clist_call_state != QCRIL_CM_CLIST_STATE_INCOMING ) &&
       ( clist_call_state != QCRIL_CM_CLIST_STATE_WAITING ) && ( clist_call_state != QCRIL_CM_CLIST_STATE_SETUP ) )
  {
    /* Invalid state */
    QCRIL_LOG_ERROR( "Fail to add CList entry : call id %d (invalid call state %s (%d))\n", 
                     call_info_ptr->call_id, qcril_cm_lookup_state_name( call_info_ptr->call_state ), clist_call_state );
    QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );
    return E_NOT_ALLOWED;
  }
                                
  /* CDMA MT specific, calling number not available */
  if ( ( ( call_info_ptr->mode_info.info_type == CM_CALL_MODE_INFO_CDMA ) &&
         ( call_state == QCRIL_CM_CLIST_STATE_INCOMING ) ) &&
       ( ( call_info_ptr->call_type == CM_CALL_TYPE_VOICE ) || ( call_info_ptr->call_type == CM_CALL_TYPE_TEST ) ||
         ( call_info_ptr->call_type == CM_CALL_TYPE_OTAPA ) ) )
  {
    memset( &calling_num, 0, sizeof( cm_num_s_type ) );
  }
  /* Call control modified calling number */
  else if ( call_info_ptr->result_from_cc.call_control_result == CM_CC_RESULT_ALLOWED_BUT_MODIFIED ||
            call_info_ptr->result_from_cc.call_control_result == CM_CC_RESULT_ALLOWED_BUT_MODIFIED_TO_VOICE )
  {
    calling_num = call_info_ptr->result_from_cc.num;
    QCRIL_LOG_DEBUG ("Call control modified number %s", calling_num.buf );
  }
  else
  {
    calling_num = call_info_ptr->num;
  }

  /* Set the Presentation indicator based on cause of no CLI 
     if presentation is Restricted */
  if( ( clist_call_state == QCRIL_CM_CLIST_STATE_SETUP ) &&
      ( call_info_ptr->num.pi == CM_PRESENTATION_RESTRICTED ) &&
      ( call_info_ptr->num.len == 0 ) &&
      ( call_info_ptr->mode_info.info.gw_cs_call.cause_of_no_cli.present ) )
  {
    switch (call_info_ptr->mode_info.info.gw_cs_call.cause_of_no_cli.cause_value)
    {
      case QCRIL_CM_NO_CLI_CAUSE_REJECT_BY_USER:
        calling_num.pi = QCRIL_CM_NUM_PRESENTATION_RESTRICTED;
        break;

      case QCRIL_CM_NO_CLI_CAUSE_PAYPHONE:
        calling_num.pi = QCRIL_CM_NUM_PRESENTATION_PAYPHONE;
        break;

      case QCRIL_CM_NO_CLI_CAUSE_UNAVAILABLE:
      case QCRIL_CM_NO_CLI_CAUSE_INTERACTION_WITH_OTHER_SERVICE:
      default:
        calling_num.pi = QCRIL_CM_NUM_PRESENTATION_UNKNOWN;
    }
  }

  /* Addd item to CList */
  status = qcril_cm_clist_new_all( instance_id, modem_id, call_info_ptr->call_id, call_info_ptr->call_type,
                                   call_info_ptr->mode_info.info_type, call_info_ptr->direction, clist_call_state, 
                                   call_info_ptr->line, calling_num, &info );
  if ( status == E_SUCCESS )
  {
    /* MT, add UUS data if any */
    if ( call_state == QCRIL_CM_CLIST_STATE_INCOMING )
    {
      if ( ( call_info_ptr->call_id == uus_ptr->call_id ) && uus_ptr->user_data.mt_user_data.present )
      {
        memset( &call_event_user_data, 0, sizeof( cm_call_event_user_data_s_type ) );
        memcpy( &call_event_user_data, &uus_ptr->user_data, sizeof( cm_call_event_user_data_s_type ) );
        if ( qcril_cm_clist_set_uus_data( instance_id, modem_id, info.call_id, &call_event_user_data ) == E_SUCCESS )
        {
          QCRIL_LOG_DEBUG( "CList entry : UUS Record update = %s\n","SUCCESS" );
          memset( uus_ptr, 0, sizeof( qcril_cm_call_event_user_data_s_type ) );
        }
      }
    }

    /* Log CList changes */
    qcril_cm_clist_log( instance_id, &info );

    /* Check whether call state changed idication need to be sent */
    if ( QCRIL_CM_CLIST_REPORT_CALL_STATE_CHANGED( info.call_type, info.state ) )
    {
      *unsol_call_state_changed_ptr = TRUE;
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );
                                        
  return status;

} /* qcril_cm_clist_new() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_replace

===========================================================================*/
/*!
    @brief
    Replaces an existing call with a new call.
 
    @return
    E_SUCCESS if the existing call was successfully replaced
    E_FAILURE if the existing call was not successfully replaced
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_replace
( 
  qcril_instance_id_e_type     instance_id,
  qcril_modem_id_e_type        modem_id,
  cm_call_state_e_type         call_state,          
  cm_call_id_type              call_id,
  uint32                       conn_index,
  const cm_mm_call_info_s_type *call_info_ptr, 
  boolean                      *unsol_call_state_changed_ptr
)
{
  qcril_cm_clist_buf_type *buf_ptr;
  cm_num_s_type calling_num;
  qcril_cm_clist_state_e_type clist_call_state;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  if ( call_state == CM_CALL_STATE_INCOM )
  {
    clist_call_state = QCRIL_CM_CLIST_STATE_INCOMING;
  }
  else if ( call_state == CM_CALL_STATE_CONV ) 
  {
    clist_call_state = QCRIL_CM_CLIST_STATE_ACTIVE;
  }
  else
  {
    /* Invalid state */
    QCRIL_LOG_ERROR( "[RID %d] Fail to replace CList entry : call id %d (invalid call state %s(%d))\n", 
                     instance_id, call_id, qcril_cm_lookup_state_name( call_state ), call_state );
    QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );
    return E_NOT_ALLOWED;
  }

  /* CDMA MT specific, calling number not available */
  if ( ( call_info_ptr->mode_info.info_type == CM_CALL_MODE_INFO_CDMA ) &&
       ( ( call_info_ptr->call_type == CM_CALL_TYPE_VOICE ) || ( call_info_ptr->call_type == CM_CALL_TYPE_TEST ) ||
         ( call_info_ptr->call_type == CM_CALL_TYPE_OTAPA ) ) )
  {
    memset( &calling_num, 0, sizeof( cm_num_s_type ) );
  }
  /* Call control modified calling number */
  else if ( call_info_ptr->result_from_cc.call_control_result == CM_CC_RESULT_ALLOWED_BUT_MODIFIED )
  {
    calling_num = call_info_ptr->result_from_cc.num;
  }
  else
  {
    calling_num = call_info_ptr->num;
  }

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  /* Entry was not found */
  if ( buf_ptr == NULL )
  {
    QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );
    return E_FAILURE;
  }

  QCRIL_LOG_DEBUG( "[RID %d] Found CList entry to be replaced: %s (%d), call id=%d, conn index=%lu, call type=%d, call mode=%d, direction=%d\n",
                   instance_id, qcril_cm_lookup_state_name( buf_ptr->pub.state ), buf_ptr->pub.state, buf_ptr->pub.call_id, 
                   buf_ptr->pub.conn_index, buf_ptr->pub.call_type, buf_ptr->pub.call_mode, buf_ptr->pub.direction );

  QCRIL_LOG_DEBUG( "                                         line=%d, number=%s, number presentation=%d, privacy mode=%d, "
                   "name=%s, name presentation=%d\n",
                   buf_ptr->pub.line, buf_ptr->pub.num.buf, buf_ptr->pub.num.pi, buf_ptr->pub.is_private,
                   buf_ptr->pub.name, buf_ptr->pub.name_presentation );

  buf_ptr->pub.prev_state = buf_ptr->pub.state;

  buf_ptr->pub.state = clist_call_state;
  buf_ptr->pub.conn_index = conn_index;
  buf_ptr->pub.call_type = call_info_ptr->call_type;
  buf_ptr->pub.call_mode = call_info_ptr->mode_info.info_type;
  buf_ptr->pub.direction = call_info_ptr->direction;
  buf_ptr->pub.line = call_info_ptr->line;           
  buf_ptr->pub.num = calling_num;
  buf_ptr->pub.num.buf[ calling_num.len ] = '\0';
  buf_ptr->pub.answered = FALSE;
  buf_ptr->pub.name[ 0 ] = '\0';
  buf_ptr->pub.name_presentation = calling_num.pi;

  /* Log CList changes */
  qcril_cm_clist_log( instance_id, &buf_ptr->pub );

  QCRIL_LOG_DEBUG( "[RID %d] Replaced CList entry: %s(%d), call id=%d, conn index=%lu, call type=%d, call mode=%d, direction=%d\n",
                   instance_id, qcril_cm_lookup_state_name( buf_ptr->pub.state ), buf_ptr->pub.state, buf_ptr->pub.call_id, 
                   buf_ptr->pub.conn_index, buf_ptr->pub.call_type, buf_ptr->pub.call_mode, buf_ptr->pub.direction );

  QCRIL_LOG_DEBUG( "                             line=%d, number=%s, number presentation=%d, privacy mode=%d, "
                   "name=%s, name presentation=%d\n",
                   buf_ptr->pub.line, buf_ptr->pub.num.buf, buf_ptr->pub.num.pi, buf_ptr->pub.is_private,
                   buf_ptr->pub.name, buf_ptr->pub.name_presentation );

  /* Check to see if unsolicited call state changed has to be sent after all update is completed */
  if ( QCRIL_CM_CLIST_REPORT_CALL_STATE_CHANGED( buf_ptr->pub.call_type, buf_ptr->pub.state ) )
  {
    *unsol_call_state_changed_ptr = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return E_SUCCESS;

} /* qcril_cm_clist_replace() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_incoming

===========================================================================*/
/*!
    @brief
    Change the state of the entry in the list to income.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_incoming
( 
  qcril_instance_id_e_type             instance_id,
  qcril_modem_id_e_type                modem_id,
  cm_call_id_type                      call_id,
  qcril_cm_call_event_user_data_s_type *uus_ptr,
  boolean                              *unsol_call_state_changed_ptr

)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;
  cm_call_event_user_data_s_type  call_event_user_data;

  /*-----------------------------------------------------------------------*/
                                    
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr == NULL )
  {
    QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );
    return E_FAILURE;
  }

  if ( ( ( buf_ptr->pub.call_type == CM_CALL_TYPE_VOICE ) ||
         ( buf_ptr->pub.call_type == CM_CALL_TYPE_EMERGENCY ) ) &&
         ( buf_ptr->pub.direction == CM_CALL_DIRECTION_MT ) && 
         ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_SETUP ) )
  {
    buf_ptr->pub.prev_state = buf_ptr->pub.state;

    /* If in MPTY, state should be WAITING */
    if ( qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, QCRIL_CM_CLIST_STATE_ACTIVE ) > 0 ) 
    {
      buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_WAITING;
    }
    else
    {
      buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_INCOMING;
    }

    /* Log CList changes */
    QCRIL_LOG_DEBUG( "Change in call state of CList entry : call id %d, %s -> %s\n", 
                     call_id, qcril_cm_lookup_state_name( buf_ptr->pub.prev_state ),  qcril_cm_lookup_state_name( buf_ptr->pub.state ) );

    if ( ( call_id == uus_ptr->call_id ) && uus_ptr->user_data.mt_user_data.present )
    {
      memset( &call_event_user_data, 0, sizeof( cm_call_event_user_data_s_type ) );
      memcpy( &call_event_user_data, &uus_ptr->user_data, sizeof( cm_call_event_user_data_s_type ) );
      if ( qcril_cm_clist_set_uus_data( instance_id, modem_id, call_id, &call_event_user_data ) == E_SUCCESS )
      {
        QCRIL_LOG_DEBUG( "CList entry : UUS Record update = %s\n","SUCCESS");
        memset( uus_ptr, 0, sizeof( qcril_cm_call_event_user_data_s_type ) );
      }
    }

    /* Need to send unsolicited call state changed */
    *unsol_call_state_changed_ptr = TRUE;
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_incoming() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_answer

===========================================================================*/
/*!
    @brief
    Record the incoming call as answered with given Call ID. 
    QCRIL_CM_CLIST_STATE_ACTIVE.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_answer
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    modem_id,
  cm_call_id_type          call_id
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/
          
  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {   
    if ( buf_ptr->pub.direction == CM_CALL_DIRECTION_MT )
    {
      buf_ptr->pub.prev_state = buf_ptr->pub.state;
      buf_ptr->pub.answered = TRUE;

      /* Log CList changes */
      qcril_cm_clist_log( instance_id, &buf_ptr->pub );
    }
    else
    {
      QCRIL_LOG_ERROR( "[RID %d] Call ID %d found in CList but not MT call\n", instance_id, call_id ); 
      status = E_FAILURE;
    }
  }
  else
  {
    /* Call not in the list */
    QCRIL_LOG_ERROR( "[RID %d] Call ID %d not found in CList\n", instance_id, call_id ); 
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_answer() */

/*===========================================================================

  FUNCTION:  qcril_cm_clist_alert

===========================================================================*/
/*!
    @brief
    Change the state of the entry in the list with given Call ID to 
    QCRIL_CM_CLIST_STATE_ALERTING.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_alert
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    modem_id,
  cm_call_id_type          call_id,
  boolean                  *unsol_call_state_changed_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( ( buf_ptr != NULL ) && ( buf_ptr->pub.direction == CM_CALL_DIRECTION_MO ) && 
       ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_DIALING ) )
  {
    buf_ptr->pub.prev_state = buf_ptr->pub.state;
    buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_ALERTING;

    /* Log CList changes */
    qcril_cm_clist_log( instance_id, &buf_ptr->pub );

    /* Need to send unsolicited call state change */
    *unsol_call_state_changed_ptr = TRUE;
  }
  else
  {
    /* Call not in the list or not an outgoing call */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_alert() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_connect

===========================================================================*/
/*!
    @brief
    Change the state of the entry in the list with given Call ID to 
    QCRIL_CM_CLIST_STATE_ACTIVE.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_connect
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    modem_id,
  cm_call_id_type          call_id,
  boolean                  *unsol_call_state_changed_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;
  uint32 current_num_of_active_voice_calls;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/
          
  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  current_num_of_active_voice_calls = qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, QCRIL_CM_CLIST_STATE_ACTIVE );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {   
    buf_ptr->pub.prev_state = buf_ptr->pub.state;
    buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_ACTIVE;

    /* Log CList changes */
    qcril_cm_clist_log( instance_id, &buf_ptr->pub );

    /* Check to see if unsolicited call state changed has to be sent after all update is completed */
    if ( QCRIL_CM_CLIST_REPORT_CALL_STATE_CHANGED( buf_ptr->pub.call_type, buf_ptr->pub.state ) )
    {
      *unsol_call_state_changed_ptr = TRUE;
    }

    /* Check whether any incoming voice call need to change state */
    if ( ( current_num_of_active_voice_calls == 0 ) &&
         ( qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, QCRIL_CM_CLIST_STATE_ACTIVE ) > 0 ) )
    {
      buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;
      while ( buf_ptr != NULL )
      {
        if ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_INCOMING ) 
        {
          buf_ptr->pub.prev_state = buf_ptr->pub.state; 
          buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_WAITING;

          /* Log CList changes */
          qcril_cm_clist_log( instance_id, &buf_ptr->pub );

          /* Need to send unsolicited call state changed */
          *unsol_call_state_changed_ptr = TRUE;
        }

        buf_ptr = buf_ptr->next_ptr;
      }
    }
  }
  else
  {
    /* Call not in the list */
    QCRIL_LOG_ERROR( "Call ID %d not found in CList\n", call_id ); 
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_connect() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_manage

===========================================================================*/
/*!
    @brief
    Updates the state of the entries in the list with the specified 
    active call IDs list information.  
 
    @return
    None
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_manage
( 
  qcril_instance_id_e_type       instance_id,
  qcril_modem_id_e_type          modem_id,
  const active_calls_list_s_type *active_calls_list_ptr, 
  boolean                        *unsol_call_state_changed_ptr
)
{
  uint32 i;
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( active_calls_list_ptr != NULL );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  /* Change all call states for all active calls to STATE_HOLDING */
  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;
  while ( buf_ptr != NULL )
  {
    if ( ( buf_ptr->pub.call_type == CM_CALL_TYPE_VOICE ) || 
         ( buf_ptr->pub.call_type == CM_CALL_TYPE_EMERGENCY ) )
    {             
      buf_ptr->pub.prev_state = buf_ptr->pub.state;
      if ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_ACTIVE )
      {
        buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_HOLDING;
        QCRIL_LOG_DEBUG( "[RID %d] call id: %d state: Holding\n", instance_id, buf_ptr->pub.call_id );
      }
    }

    buf_ptr = buf_ptr->next_ptr;
  }

  /* Update call states according to active_calls_list. */
  for ( i = 0; i < active_calls_list_ptr->size; i++ )
  {
    buf_ptr = qcril_cm_clist_find( instance_id, modem_id, active_calls_list_ptr->table[ i ] );
    if ( buf_ptr == NULL )
    {
      QCRIL_LOG_ERROR( "[RID %d] Call ID %d not found in CList\n", instance_id, active_calls_list_ptr->table[ i ] ); 
      status = E_FAILURE;
    }
    else
    {
      buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_ACTIVE;
      QCRIL_LOG_DEBUG( "[RID %d] call id: %d state: Active\n", instance_id, buf_ptr->pub.call_id );
    }
  } 

  /* Check to see if there are CList changes need to be logged */
  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;
  while ( buf_ptr != NULL )
  {
    if ( ( buf_ptr->pub.call_type == CM_CALL_TYPE_VOICE ) || 
         ( buf_ptr->pub.call_type == CM_CALL_TYPE_EMERGENCY ) )
    {             
      if ( buf_ptr->pub.prev_state != buf_ptr->pub.state )
      {
        /* Log CList changes */
        qcril_cm_clist_log( instance_id, &buf_ptr->pub );

        /* Remember to report unsolicted call state changed after all logging is done */
        *unsol_call_state_changed_ptr = TRUE;
      }
    }

    buf_ptr = buf_ptr->next_ptr;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_manage */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_free

===========================================================================*/
/*!
    @brief
    Frees an entry from the list of active calls.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_free
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    modem_id,
  cm_call_id_type          call_id,
  boolean                  *unsol_call_state_changed_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;
  qcril_cm_clist_buf_type *prev_buf_ptr, *next_buf_ptr;
  uint32 current_num_of_active_voice_calls;
  uint32 current_num_of_voice_calls;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  current_num_of_active_voice_calls = qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, QCRIL_CM_CLIST_STATE_ACTIVE );
  current_num_of_voice_calls = qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, 
                                                                                 QCRIL_CM_CLIST_STATE_ACTIVE | 
                                                                                 QCRIL_CM_CLIST_STATE_ALERTING |
                                                                                 QCRIL_CM_CLIST_STATE_DIALING | 
                                                                                 QCRIL_CM_CLIST_STATE_HOLDING |
                                                                                 QCRIL_CM_CLIST_STATE_INCOMING | 
                                                                                 QCRIL_CM_CLIST_STATE_WAITING );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  /* Entry was found */
  if ( buf_ptr != NULL )
  {
    buf_ptr->pub.prev_state = buf_ptr->pub.state;
    buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_IDLE;

    /* Log CList changes */
    qcril_cm_clist_log( instance_id, &buf_ptr->pub );

    /* Check to see if unsolicited call state changed has to be sent after all update is completed */
    if ( QCRIL_CM_CLIST_REPORT_CALL_STATE_CHANGED( buf_ptr->pub.call_type, buf_ptr->pub.state ) )
    {
      *unsol_call_state_changed_ptr = TRUE;
    }

    /* Detach the item from the CList */
    prev_buf_ptr = buf_ptr->prev_ptr;
    next_buf_ptr = buf_ptr->next_ptr;

    /* The only item in the CList is being removed */
    if ( ( prev_buf_ptr == NULL ) && ( next_buf_ptr == NULL ) )
    {
      qcril_cm_clist[ instance_id ].head_ptr = NULL;
      qcril_cm_clist[ instance_id ].tail_ptr = NULL;
    }
    /* First item in the CList is being removed */
    else if ( prev_buf_ptr == NULL )
    {
      qcril_cm_clist[ instance_id ].head_ptr = next_buf_ptr;
      next_buf_ptr->prev_ptr = NULL;
    }   
    /* Last item in the CList is being removed*/
    else if ( next_buf_ptr == NULL )
    {
      qcril_cm_clist[ instance_id ].tail_ptr = prev_buf_ptr;
      prev_buf_ptr->next_ptr = NULL;
    }
    /* Middle item in the CList is being removed */
    else
    {
      prev_buf_ptr->next_ptr = buf_ptr->next_ptr;
      next_buf_ptr->prev_ptr = buf_ptr->prev_ptr;
    }

    buf_ptr->next_ptr = buf_ptr->prev_ptr = NULL;

    /* Mute audio if the last call is being ended */
    if ( ( current_num_of_voice_calls == 1 ) &&
         ( qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id,
                                                             QCRIL_CM_CLIST_STATE_ACTIVE | QCRIL_CM_CLIST_STATE_ALERTING |
                                                             QCRIL_CM_CLIST_STATE_DIALING | QCRIL_CM_CLIST_STATE_HOLDING |
                                                             QCRIL_CM_CLIST_STATE_INCOMING | QCRIL_CM_CLIST_STATE_WAITING ) == 0 ) )
    {
      QCRIL_LOG_DEBUG( "%s", "Last call ended - muting mic and speaker");
      qcril_other_mute( instance_id, TRUE, TRUE ); 
    }

    /* Check whether any incoming voice call need to change state */
    if ( ( current_num_of_active_voice_calls > 0 ) &&
         ( qcril_cm_clist_count_num_of_voice_calls_by_state( instance_id, QCRIL_CM_CLIST_STATE_ACTIVE ) == 0 ) )
    {
      buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;
      while ( buf_ptr != NULL )
      {
        if ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_WAITING ) 
        {
          buf_ptr->pub.prev_state = buf_ptr->pub.state; 
          buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_INCOMING;

          /* Log CList changes */
          qcril_cm_clist_log( instance_id, &buf_ptr->pub );

          /* Check to see if unsolicited call state changed has to be sent after all update is completed */
          if ( QCRIL_CM_CLIST_REPORT_CALL_STATE_CHANGED( buf_ptr->pub.call_type, buf_ptr->pub.state ) )
          {
            *unsol_call_state_changed_ptr = TRUE;
          }
        }

        buf_ptr = buf_ptr->next_ptr;
      }
    }

    QCRIL_LOG_DEBUG( "[RID %d] Delete CList entry: call id %d\n", instance_id, call_id );
  }
  /* Entry was not found */
  else
  {
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_free() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_free_all

===========================================================================*/
/*!
    @brief
    Free all entries in the list of active calls.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_cm_clist_free_all
( 
  void
)
{
  int i;
  qcril_cm_clist_buf_type *buf_ptr;
  qcril_cm_clist_buf_type *next_buf_ptr;
  boolean report_call_state_changed = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_cm_clist_free_all()\n" );

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  for ( i = 1; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    buf_ptr = qcril_cm_clist[ i ].head_ptr;

    while ( buf_ptr != NULL )
    {
      /* Check to see if CALL_STATE_CHANGED need to be reported for voice call being dropped */
      if ( !report_call_state_changed && QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) )
      {                   
        report_call_state_changed = TRUE;
      }

      next_buf_ptr = buf_ptr->next_ptr;
      buf_ptr->pub.prev_state = buf_ptr->pub.state;
      buf_ptr->pub.state = QCRIL_CM_CLIST_STATE_IDLE;
      buf_ptr->next_ptr = NULL;
      buf_ptr->prev_ptr = NULL;
      buf_ptr = next_buf_ptr;
    }

    qcril_cm_clist[ i ].head_ptr = NULL;
    qcril_cm_clist[ i ].tail_ptr = NULL;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

} /* qcril_cm_clist_free_all() */

/*===========================================================================

  FUNCTION:  qcril_cm_clist_update_voice_privacy_mode

===========================================================================*/
/*!
    @brief
    Change the privacy_mode of the entry in the list with given Call ID to 
    voice_privacy.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_update_voice_privacy_mode
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  boolean privacy_mode,
  boolean *unsol_call_state_changed_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/
                                    
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  *unsol_call_state_changed_ptr = FALSE;

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    if ( ( ( buf_ptr->pub.call_type == CM_CALL_TYPE_VOICE ) ||
           ( buf_ptr->pub.call_type == CM_CALL_TYPE_EMERGENCY ) ) &&
         ( buf_ptr->pub.is_private != privacy_mode ) )
    {
      /* Log CList changes */
      QCRIL_LOG_DEBUG( "Change in call state of CList entry : call id %d, %s -> %s\n", 
                       call_id, ( buf_ptr->pub.is_private ? "Private" : "Public" ), ( privacy_mode ? "Private" : "Public" ) );

      buf_ptr->pub.is_private = privacy_mode;

      /* Send unsolicited call state change if needed */
      *unsol_call_state_changed_ptr = TRUE;
    }
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_update_voice_privacy_mode() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_update_name

===========================================================================*/
/*!
    @brief
    Change the name of the entry in the list with given Call ID
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_update_name
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  char * name
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    if ( strcmp( name, buf_ptr->pub.name ) != 0 )
    {
      /* Log CList changes */
      QCRIL_LOG_INFO( "[RID %d] Change in call state of CList entry: call id %d, name %s -> %s\n", 
                      instance_id, call_id, buf_ptr->pub.name, name );
      QCRIL_SNPRINTF( buf_ptr->pub.name, CM_MAX_ALPHA_TAG_CHARS, "%s", name );
    }
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_update_name() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_update_name_presentation

===========================================================================*/
/*!
    @brief
    Change the name presentation of the entry in the list with given Call ID

    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_update_name_presentation
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  int  name_presentation
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    if ( name_presentation != buf_ptr->pub.name_presentation )
    {
      QCRIL_LOG_INFO( "[RID %d] Change in call state of CList entry : call id %d, name_presentation %d -> %d\n",
                      instance_id, call_id, buf_ptr->pub.name_presentation, name_presentation );

      buf_ptr->pub.name_presentation = name_presentation;
    }
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_update_name_presentation() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_update_number

===========================================================================*/
/*!
    @brief
    Change the number of the entry in the list with given Call ID
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_update_number
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  cm_num_s_type * number
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/
                                    
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    /* Check if the number has changed */
    if (strcmp( (char *)number->buf, (char *)buf_ptr->pub.num.buf) != 0)
    {
      /* Log CList changes */
      QCRIL_LOG_INFO( "[RID %d] Change in call state of CList entry: call id %d, number %s -> %s\n", 
                      instance_id, call_id, buf_ptr->pub.num.buf, number->buf );
    }

    /* Check if the number presentation has changed */
    if (number->pi != buf_ptr->pub.num.pi)
    {
      /* Log CList changes */
      QCRIL_LOG_INFO( "[RID %d] Change in call state of CList entry: call id %d, number presentation %d -> %d\n", 
                      instance_id, call_id, buf_ptr->pub.num.pi, number->pi );
    }

    buf_ptr->pub.num = *number;
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_update_number() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_voice_is_emergency

===========================================================================*/
/*!
    @brief
    Check if the call_type of the given Call ID is an emergency call.  
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
boolean qcril_cm_clist_voice_is_emergency
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id
)
{
  boolean is_emergency = FALSE;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {   
    if ( buf_ptr->pub.call_type == CM_CALL_TYPE_EMERGENCY  )
    {
      is_emergency = TRUE;
    }
  }
  
  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return is_emergency;

} /* qcril_cm_clist_voice_is_emergency() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_voice_call_is_mpty

===========================================================================*/
/*!
    @brief
    Check whether voice calls in HOLDING or ACTIVE state are in MPTY.
 
    @return
    TRUE if it is MPTY. Otherwise, FALSE.
*/
/*=========================================================================*/
boolean qcril_cm_clist_voice_call_is_mpty
(
  qcril_instance_id_e_type instance_id
)
{
  boolean in_mpty = FALSE;
  qcril_cm_clist_buf_type *buf_ptr;
  uint32 num_of_active_voice_calls = 0;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( ( ( buf_ptr->pub.call_type == CM_CALL_TYPE_VOICE ) ||
           ( buf_ptr->pub.call_type == CM_CALL_TYPE_EMERGENCY ) ) &&
         ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_ACTIVE ) )
    {
      num_of_active_voice_calls++;
    }

    buf_ptr = buf_ptr->next_ptr;
  }

  if ( num_of_active_voice_calls > 1 )
  {
    in_mpty = TRUE;
  }

  if ( in_mpty )
  {
    QCRIL_LOG_DEBUG( "[RID %d] Voice calls in MPTY\n", instance_id ); 
  }
  else
  {
    QCRIL_LOG_DEBUG( "[RID %d] Voice calls not in MPTY\n", instance_id ); 
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return in_mpty;

} /* qcril_cm_clist_voice_call_is_mpty */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_query_voice_call_id

===========================================================================*/
/*!
    @brief
    Finds the voice entry in the list for the given Call ID.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_query_voice_call_id
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  qcril_cm_clist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;
  qcril_cm_clist_public_type *pub_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( ( buf_ptr != NULL ) && QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) )
  {
    pub_ptr = &buf_ptr->pub;
    *info_ptr = *pub_ptr;
  }
  else
  {
    /* Call ID not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_query_voice_call_id */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_query_by_conn_index

===========================================================================*/
/*!
    @brief
    Finds the entry in the list for the given Connection Index.
 
    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_query_by_conn_index
( 
  qcril_instance_id_e_type instance_id,
  uint32 conn_index,
  qcril_cm_clist_public_type *info_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;
  qcril_cm_clist_public_type *pub_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( buf_ptr->pub.conn_index == conn_index )
    {             
      break;;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  if ( buf_ptr != NULL )
  {
    pub_ptr = &buf_ptr->pub;
    *info_ptr = *pub_ptr;
    QCRIL_LOG_INFO( "[RID %d] Found CList entry: conn index %lu, call id %d, %s (%d)\n", 
                    instance_id, conn_index, buf_ptr->pub.call_id, qcril_cm_lookup_state_name( buf_ptr->pub.state ), buf_ptr->pub.state ); 
  }
  else
  {
    /* Connection Index not in the list */
    QCRIL_LOG_INFO( "{RID %d] Not found CList entry: conn index %lu\n", instance_id, conn_index );
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_query_by_conn_index */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_query_voice_call_ids_list_by_state

===========================================================================*/
/*!
    @brief
    Finds the Call IDs of voice entries in the list with the given state.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_cm_clist_query_voice_call_ids_list_by_state
( 
  qcril_instance_id_e_type instance_id,
  uint32 state_mask,
  qcril_cm_clist_call_ids_list_type *call_ids_list_ptr 
)
{                                   
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( call_ids_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  call_ids_list_ptr->num_of_call_ids = 0;

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) && ( buf_ptr->pub.state & state_mask ) )
    {             
      call_ids_list_ptr->modem_id[ call_ids_list_ptr->num_of_call_ids ] = buf_ptr->pub.modem_id; 
      call_ids_list_ptr->call_mode[ call_ids_list_ptr->num_of_call_ids ] = buf_ptr->pub.call_mode; 
      call_ids_list_ptr->call_id[ call_ids_list_ptr->num_of_call_ids ] = buf_ptr->pub.call_id; 
      call_ids_list_ptr->answered[ call_ids_list_ptr->num_of_call_ids++ ] = buf_ptr->pub.answered; 
      QCRIL_LOG_INFO( "[RID %d] CList entry with call id %d , state %s (%d), mode %d, answered %d\n", 
                      instance_id, buf_ptr->pub.call_id, qcril_cm_lookup_state_name( buf_ptr->pub.state ), buf_ptr->pub.state, 
                      buf_ptr->pub.call_mode, buf_ptr->pub.answered );
    }

    buf_ptr = buf_ptr->next_ptr;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

} /* qcril_cm_clist_query_voice_call_ids_by_state */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_query_call_info_list

===========================================================================*/
/*!
    @brief
    Find all voice call entries in the list.
 
    @return
    None
*/
/*=========================================================================*/
void qcril_cm_clist_query_call_info_list
( 
  qcril_instance_id_e_type instance_id,
  qcril_cm_clist_call_info_list_type *info_list_ptr
)
{
  uint32 num_of_active_voice_calls = 0;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( info_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  info_list_ptr->num_of_calls = 0;
  info_list_ptr->in_mpty = FALSE;

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) )
    {
      info_list_ptr->info[ info_list_ptr->num_of_calls++ ] = buf_ptr->pub;

      if ( QCRIL_CM_CLIST_CALL_TYPE_IS_VOICE( buf_ptr->pub.call_type ) &&
           ( buf_ptr->pub.state == QCRIL_CM_CLIST_STATE_ACTIVE ) )
      {
        num_of_active_voice_calls++;
      }
    }

    buf_ptr = buf_ptr->next_ptr;
  }

  if ( num_of_active_voice_calls > 1 )
  {
    info_list_ptr->in_mpty = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

} /* qcril_cm_clist_query_call_info_list */



/*===========================================================================

  FUNCTION:  qcril_cm_clist_query_call_id

===========================================================================*/
/*!
    @brief
    Find an entry in the list based on Call ID

    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_query_call_id
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  qcril_cm_clist_public_type *clist_info_ptr
)
{
  IxErrnoType status = E_FAILURE;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist[ instance_id ].head_ptr;

  while ( buf_ptr != NULL )
  {
    if ( ( buf_ptr->pub.call_id == call_id ) && ( buf_ptr->pub.modem_id == modem_id ) )
    {
      QCRIL_LOG_INFO( "Found CList entry : call id %d\n", call_id );
      clist_info_ptr->modem_id = buf_ptr->pub.modem_id;
      clist_info_ptr->conn_index = buf_ptr->pub.conn_index;
      clist_info_ptr->call_id = buf_ptr->pub.call_id;

      clist_info_ptr->call_id = buf_ptr->pub.call_id;
      clist_info_ptr->call_type = buf_ptr->pub.call_type;
      clist_info_ptr->direction = buf_ptr->pub.direction;
      clist_info_ptr->prev_state = buf_ptr->pub.prev_state;
      clist_info_ptr->state = buf_ptr->pub.state;
      clist_info_ptr->call_mode = buf_ptr->pub.call_mode;
      clist_info_ptr->line = buf_ptr->pub.line;
      clist_info_ptr->num = buf_ptr->pub.num;
      clist_info_ptr->num.buf[ buf_ptr->pub.num.len ] = '\0';
      clist_info_ptr->answered = buf_ptr->pub.answered;
      clist_info_ptr->is_private = buf_ptr->pub.is_private;
      memcpy(clist_info_ptr->name, buf_ptr->pub.name, CM_MAX_ALPHA_TAG_CHARS);
      clist_info_ptr->name_presentation = buf_ptr->pub.name_presentation;
      status = E_SUCCESS;
      break;
    }
    else
    {
      buf_ptr = buf_ptr->next_ptr;
    }
  }

  QCRIL_LOG_INFO( "[RID %d] Not found CList entry: call id %d\n", instance_id, call_id );

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_query_call_id() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_update_local_ringback

===========================================================================*/
/*!
    @brief
    Change the local ringback present of the entry in the list with given Call ID

    @return
    E_SUCCESS if the entry was found
    E_FAILURE if the entry was not found
*/
/*=========================================================================*/
IxErrnoType qcril_cm_clist_update_local_ringback
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id,
  boolean  is_local_ringback
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_clist_buf_type *buf_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    if ( is_local_ringback != buf_ptr->pub.local_ringback)
    {
      /* Log CList changes */
      QCRIL_LOG_INFO( "[RID %d] Change in call state of CList entry: call id %d, is_local_ringback %d -> %d\n",
                      instance_id, call_id, buf_ptr->pub.local_ringback, is_local_ringback );

      buf_ptr->pub.local_ringback = is_local_ringback;
    }
  }
  else
  {
    /* Call not in the list */
    status = E_FAILURE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return status;

} /* qcril_cm_clist_update_local_ringback() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_is_local_ringback

===========================================================================*/
/*!
    @brief
    Check whether given Call ID has local ringback set

    @return
    TRUE if the entry was found and local ringback set
    FALSE if the entry was not found or local ringback not set
*/
/*=========================================================================*/
boolean qcril_cm_clist_is_local_ringback
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id
)
{
  qcril_cm_clist_buf_type *buf_ptr;
  boolean is_local_ringback = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    /* Log CList changes */
    QCRIL_LOG_INFO( "[RID %d] Call state of CList entry: call id %d, is_local_ringback :%d\n",
                    instance_id, call_id, buf_ptr->pub.local_ringback );
    is_local_ringback = buf_ptr->pub.local_ringback;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return is_local_ringback;

} /* qcril_cm_clist_is_local_ringback() */


/*===========================================================================

  FUNCTION:  qcril_cm_clist_is_uus_call

===========================================================================*/
/*!
    @brief
    Check whether given Call ID has UUS set

    @return
    TRUE if the entry was found and UUS set
    FALSE if the entry was not found or UUS not set
*/
/*=========================================================================*/
boolean qcril_cm_clist_is_uus_call
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  cm_call_id_type call_id
)
{
  qcril_cm_clist_buf_type *buf_ptr;
  boolean is_uus_call = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  buf_ptr = qcril_cm_clist_find( instance_id, modem_id, call_id );

  if ( buf_ptr != NULL )
  {
    /* Log CList changes */
    QCRIL_LOG_INFO( "[RID %d] Call state of CList entry: call id %d, is_UUS :%d\n", instance_id, call_id, buf_ptr->pub.is_uus );
    is_uus_call = buf_ptr->pub.is_uus;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_cm_clist_mutex, "qcril_cm_clist_mutex" );

  return is_uus_call;

} /* qcril_cm_clist_is_uus_call() */
