/*!
  @file
  qcril_sms.c

  @brief
  Handles RIL requests for SMS (WMS)
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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_sms.c#25 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/11/10   sb      Return retry if WMS is busy.
03/01/10   fc      Re-architecture to support split modem.
02/25/10   sb      Drop MT SMS if mobile is in Emergency Callback state.
02/02/10   sb      Return retry if the network is not ready.
12/17/09   sb      Return an error code of retry if sending a message fails
                   because WMS has no resources.
12/08/09   sb      Store message_ref per message, in the reqlist.
11/12/09   sb      Moved setting of retry period to after route configuration.
10/08/09   sb      Increased the retry period to 240 seconds.
10/02/09   sb      Added support for getting and setting the SMSC address.
08/10/09   sb      Added support for RUIM SMS.
08/07/09   sb      Added support for reporting memory status.
07/24/09   sb      Added cause codes in the case of SMS failure.
07/22/09   sb      Added support for latest ril.h under FEATURE_NEW_RIL_API.
06/04/09   sb      Drop CDMA SMS ack if WMS has already acked the message.
05/19/09   sb      Adjusted SIM indexes to account for the fact that Android
                   uses GSDI indexes, which are 1 greater than WMS indexes.
05/18/09   sb      Removed the reference of wmsts_verify()
05/14/09   pg      Mainlined FEATURE_MULTIMODE_ANDROID.
05/07/09   fc      Renamed variable to fix the overloading of system variable
                   errno.
04/05/09   fc      Cleanup log macros and mutex macros.
12/16/08   fc      Added API to support the release of AMSS WMS object
                   for ONCRPC.
07/17/08   sb      Initial implementation.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <cutils/properties.h>
#include <string.h>
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_other_api_map.h"
#include "qcril_reqlist.h"
#include "qcril_smsi.h"
#include "qcril_sms_api_map.h"
#include "qcril_sms_util.h"
#include "qcril_cm.h"


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

static qcril_sms_struct_type *qcril_sms;


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_sms_cmd_callback

===========================================================================*/
/*!
    @brief
    SMS command callback function.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_sms_cmd_callback
( 
  wms_cmd_id_e_type  cmd,
  void               *user_data,
  wms_cmd_err_e_type cmd_err 
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 temp;
  uint16 req_id;
  qcril_reqlist_public_type req_info;
  qcril_sms_command_callback_params_type params;

  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( user_data != NULL );
  temp = ( uint32 ) user_data;
  instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( temp );
  modem_id = QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( temp );
  req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( temp );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "RID %d MID %d Received SMS cmd callback : Cmd %s, Req ID %d\n", instance_id, modem_id,
                   qcril_sms_lookup_cmd_name(cmd), req_id );

  /* Lookup the Instance ID */
  if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
  {
    /* Call event handler for SMS command callback */
    params.command = cmd;
    params.error = cmd_err;
    qcril_event_queue( instance_id, modem_id, QCRIL_DATA_ON_STACK, QCRIL_EVT_SMS_COMMAND_CALLBACK, 
                       (void *) &params, sizeof( params ), req_info.t );
  }
  else
  {
    QCRIL_LOG_ERROR( "Req ID: %d not found for SMS cmd callback\n", req_id );
  }
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_cmd_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_process_msg_event_callback

===========================================================================*/
/*!
    @brief
    Process SMS message event callback function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_process_msg_event_callback
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  wms_msg_event_e_type event,
  wms_msg_event_info_s_type *event_ptr,
  boolean *shared 
)
{
  qcril_instance_id_e_type route_instance_id = instance_id;
  qcril_evt_e_type event_id;
  RIL_Token token;
  uint32 temp;
  uint16 req_id;
  boolean ignore_event = TRUE;
  qcril_reqlist_public_type req_info;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( event_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  switch ( event )
  {
    #ifdef FEATURE_QCRIL_IMS
    case WMS_MSG_EVENT_TRANSPORT_REG:
      ignore_event = FALSE;
      break;
    #endif /* FEATURE_QCRIL_IMS */

    case WMS_MSG_EVENT_SUBMIT_REPORT:
      /* We only care about them if they were triggered by QCRIL */
      if ( event_ptr->submit_report_info.client_id == WMS_CLIENT_TYPE_RIL )  
      {
        ignore_event = FALSE;

        #ifdef FEATURE_QCRIL_DSDS
        /* Lookup instance id from as_id */
        (void) qcril_arb_lookup_instance_id_from_as_id( event_ptr->submit_report_info.as_id, &route_instance_id );
        #endif /* FEATURE_QCRIL_DSDS */
      }
      break;

    case WMS_MSG_EVENT_WRITE:
    case WMS_MSG_EVENT_DELETE:
    case WMS_MSG_EVENT_SEND:
    case WMS_MSG_EVENT_READ_TEMPLATE:
    case WMS_MSG_EVENT_WRITE_TEMPLATE:
      /* We only care about them if they were triggered by QCRIL */
      if ( event_ptr->status_info.client_id == WMS_CLIENT_TYPE_RIL ) 
      {
        ignore_event = FALSE;

        #ifdef FEATURE_QCRIL_DSDS
        /* Lookup instance id from as_id */
        (void) qcril_arb_lookup_instance_id_from_as_id( event_ptr->status_info.as_id, &route_instance_id );
        #endif /* FEATURE_QCRIL_DSDS */
      }
      break;

    case WMS_MSG_EVENT_RECEIVED_MESSAGE:
    case WMS_MSG_EVENT_STATUS_REPORT:
      /* Received messages and Status Reports may go to all clients, in which case
         the client id is WMS_CLIENT_TYPE_MAX; or they may go directly to QCRIL. */ 
      if ( ( event_ptr->mt_message_info.client_id == WMS_CLIENT_TYPE_RIL ) ||
           ( event_ptr->mt_message_info.client_id == WMS_CLIENT_TYPE_MAX ) )
      {
        ignore_event = FALSE;

        #ifdef FEATURE_QCRIL_DSDS
        /* Lookup instance id from as_id */
        (void) qcril_arb_lookup_instance_id_from_as_id( event_ptr->mt_message_info.as_id, &route_instance_id );
        #endif /* FEATURE_QCRIL_DSDS */
      }
      break;

    #ifdef FEATURE_QCRIL_WMS_ETWS
    case WMS_MSG_EVENT_ETWS_NOTIFICATION:
        ignore_event = FALSE;

        #ifdef FEATURE_QCRIL_DSDS
        /* Lookup instance id from as_id */
        (void) qcril_arb_lookup_instance_id_from_as_id( event_ptr->mt_message_info.as_id, &route_instance_id );
        #endif /* FEATURE_QCRIL_DSDS */
        break;
    #endif /* FEATURE_QCRIL_WMS_ETWS */

    default:
      /* Ignore all other events */
      break;
  }

  if ( ignore_event )
  {
    return;
  }

  /*-----------------------------------------------------------------------*/

  /* Figure out Event ID */
  event_id = QCRIL_EVT_SMS_MSG_BASE + event;

  QCRIL_LOG_DEBUG( "RID %d MID %d Received SMS msg event : %s (%d)\n", instance_id, modem_id, 
                   qcril_log_lookup_event_name( event_id ), event_id );

  /* Determine the token id.  For events which were triggered by RIL, the
     token is in the user data.  For unsolicited events, there is no token,
     so set it to unknown. */
  if ( ( event == WMS_MSG_EVENT_WRITE ) ||
       ( event == WMS_MSG_EVENT_DELETE ) ||
       ( event == WMS_MSG_EVENT_SEND ) ||
       ( event == WMS_MSG_EVENT_READ_TEMPLATE ) ||
       ( event == WMS_MSG_EVENT_WRITE_TEMPLATE ) )
  {
    temp = (uint32) event_ptr->status_info.user_data;
    req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( temp );

    /* Lookup the Token ID */
    if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
    {
      token = req_info.t;
    }
    else
    {
      QCRIL_LOG_ERROR( "Req ID %d not found\n", req_id );
      return;
    }
  }
  else if ( event == WMS_MSG_EVENT_SUBMIT_REPORT )
  {
    temp = (uint32) event_ptr->submit_report_info.user_data;
    req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( temp );

    /* Lookup the Token ID */
    if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
    {
      token = req_info.t;
    }
    else
    {
      QCRIL_LOG_ERROR( "Req ID %d not found\n", req_id );
      return;
    }
  }
  else 
  {
    token = ( RIL_Token ) QCRIL_TOKEN_ID_INTERNAL;
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "Route RID %d MID %d for %s (%d)\n", route_instance_id, modem_id, 
                   qcril_log_lookup_event_name( event_id ), event_id );

  /* Call event handler for SMS Message event */
  qcril_event_queue( route_instance_id, modem_id, QCRIL_DATA_ON_STACK, event_id, (void *) event_ptr, 
                     sizeof( wms_msg_event_info_s_type ), token );

} /* qcril_sms_process_msg_event_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_msg_event_callback

===========================================================================*/
/*!
    @brief
    SMS message event callback function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_callback
( 
  wms_msg_event_e_type event,
  wms_msg_event_info_s_type *event_ptr,
  boolean *shared 
)
{
  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( event_ptr != NULL );
  QCRIL_ASSERT( shared != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_sms_process_msg_event_callback ( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, event, event_ptr, shared );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_msg_event_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_msg_event_callback_fusion

===========================================================================*/
/*!
    @brief
    SMS message event callback function for MDM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_callback_fusion
( 
  wms_msg_event_e_type event,
  wms_msg_event_info_s_type *event_ptr,
  boolean *shared 
)
{

  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( event_ptr != NULL );
  QCRIL_ASSERT( shared != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_sms_process_msg_event_callback ( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_SECOND_MODEM_ID, event, event_ptr, shared );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_msg_event_callback_fusion */


/*===========================================================================

  FUNCTION:  qcril_sms_process_cfg_event_callback

===========================================================================*/
/*!
    @brief
    Process SMS configuration event callback function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_process_cfg_event_callback
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  wms_cfg_event_e_type event,
  wms_cfg_event_info_s_type *event_ptr
)
{
  qcril_instance_id_e_type route_instance_id = instance_id;
  qcril_evt_e_type event_id;
  boolean ignore_event = TRUE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( event_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  switch ( event )
  {
    #ifdef FEATURE_QCRIL_DSDS
    case WMS_CFG_EVENT_MS_MEMORY_FULL:
    case WMS_CFG_EVENT_MS_MEMORY_STATUS_SET:
      ignore_event = FALSE;

      /* Lookup instance id from as_id */
      (void) qcril_arb_lookup_instance_id_from_as_id( event_ptr->ms_memory_full.as_id, &route_instance_id );
      break;
    #else
    case WMS_CFG_EVENT_MEMORY_FULL:
    case WMS_CFG_EVENT_MEMORY_STATUS_SET:
      ignore_event = FALSE;
      break;
    #endif /* FEATURE_QCRIL_DSDS */

    case WMS_CFG_EVENT_MESSAGE_LIST:
      ignore_event = FALSE;

      #ifdef FEATURE_QCRIL_DSDS
      /* Lookup instance id from as_id */
      (void) qcril_arb_lookup_instance_id_from_as_id( event_ptr->message_list.as_id, &route_instance_id );
      #endif /* FEATURE_QCRIL_DSDS */
      break;

    default:
      break;
  }

  if ( ignore_event )
  {
    return;
  }

  /*-----------------------------------------------------------------------*/

  /* Figure out Event ID */
  event_id = QCRIL_EVT_SMS_CFG_BASE + event;

  QCRIL_LOG_DEBUG( "RID %d MID %d Received SMS cfg event : %s (%d)\n", instance_id, modem_id, qcril_log_lookup_event_name( event_id ), event_id );

  QCRIL_LOG_DEBUG( "Route RID %d MID %d for %s (%d)\n", route_instance_id, modem_id, qcril_log_lookup_event_name( event_id ), event_id );

  /* Call event handler for SMS configuration event */
  qcril_event_queue( route_instance_id, modem_id, QCRIL_DATA_ON_STACK, event_id, (void *) event_ptr, 
                     sizeof( wms_cfg_event_info_s_type ), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

} /* qcril_sms_process_cfg_event_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_cfg_event_callback

===========================================================================*/
/*!
    @brief
    SMS configuration event callback function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_cfg_event_callback
( 
  wms_cfg_event_e_type event,
  wms_cfg_event_info_s_type *event_ptr 
)
{
  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( event_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_sms_process_cfg_event_callback ( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, event, event_ptr );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_cfg_event_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_cfg_event_callback_fusion

===========================================================================*/
/*!
    @brief
    SMS configuration event callback function for MDM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_cfg_event_callback_fusion
( 
  wms_cfg_event_e_type event,
  wms_cfg_event_info_s_type *event_ptr 
)
{
  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( event_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_sms_process_cfg_event_callback ( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_SECOND_MODEM_ID, event, event_ptr );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_cfg_event_callback_fusion */


/*===========================================================================

  FUNCTION:  qcril_sms_process_bc_mm_event_callback

===========================================================================*/
/*!
    @brief
    Process SMS message event callback function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_process_bc_mm_event_callback
(
  qcril_instance_id_e_type    instance_id,
  qcril_modem_id_e_type       modem_id,
  wms_bc_mm_event_e_type      event,
  wms_bc_mm_event_info_s_type *info_ptr
)
{
  qcril_sms_struct_type *i_ptr;
  qcril_evt_e_type event_id;
  qcril_instance_id_e_type route_instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  if ( qcril_arb_lookup_instance_id_from_as_id( info_ptr->as_id, &route_instance_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "RID %d MID %d Ignoring Received SMS BC MM event : (%d) on as_id=%d\n", instance_id, modem_id,
                     event,  info_ptr->as_id);
    return;
  }
 #endif

  i_ptr = &qcril_sms[ route_instance_id ];

  /* Ignore all events except the following */
  if ( ( event != WMS_BC_MM_EVENT_PREF ) &&
       ( event != WMS_BC_MM_EVENT_TABLE ) &&
       ( event != WMS_BC_MM_EVENT_ADD_SRVS )
     )
  {
    return;
  }

  /*-----------------------------------------------------------------------*/

  /* Figure out Event ID */
  event_id = QCRIL_EVT_SMS_BC_MM_BASE + event;

  QCRIL_LOG_DEBUG( "RID %d MID %d Received SMS BC MM event : %s (%d)\n", route_instance_id, modem_id,
                   qcril_log_lookup_event_name( event_id ), event_id );

  if ( ( event == WMS_BC_MM_EVENT_PREF ) ||
       ( event == WMS_BC_MM_EVENT_ADD_SRVS ) )
  {
    /* Call event handler.  QCRIL SMS does not check the payload for these events. */
    qcril_event_queue( route_instance_id, modem_id, QCRIL_DATA_ON_STACK, 
                       event_id, NULL, 0, ( RIL_Token ) QCRIL_TOKEN_ID_INTERNAL);
  }
  else if (event == WMS_BC_MM_EVENT_TABLE)
  {
    /* Call event handler.  Pass in the BC SMS config table. */
    qcril_event_queue( route_instance_id, modem_id, QCRIL_DATA_ON_STACK, event_id, (void *) info_ptr->u.bc_mm_table.entries, 
                       info_ptr->u.bc_mm_table.size * sizeof( wms_bc_mm_service_info_s_type ), 
                       ( RIL_Token ) QCRIL_TOKEN_ID_INTERNAL );
  }

} /* qcril_sms_process_bc_mm_event_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_bc_mm_event_callback

===========================================================================*/
/*!
    @brief
    BC SMS message event callback function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_bc_mm_event_callback
(
  wms_bc_mm_event_e_type      event,
  wms_bc_mm_event_info_s_type *info_ptr
)
{
  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_sms_process_bc_mm_event_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, event, info_ptr );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_bc_mm_event_callback */


/*===========================================================================

  FUNCTION:  qcril_sms_bc_mm_event_callback_fusion

===========================================================================*/
/*!
    @brief
    BC SMS message event callback function for MDM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_bc_mm_event_callback_fusion
(
  wms_bc_mm_event_e_type      event,
  wms_bc_mm_event_info_s_type *info_ptr
)
{
  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( info_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_sms_process_bc_mm_event_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_SECOND_MODEM_ID, event, info_ptr );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_bc_mm_event_callback_fusion */


/*===========================================================================

  FUNCTION:  qcril_sms_set_primary_client_and_route

===========================================================================*/
/*!
    @brief
    Set primary client and route configuration.

    @return
    None.
*/
/*=========================================================================*/
errno_enum_type qcril_sms_set_primary_client_and_route
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  qcril_sms_struct_type *i_ptr;
  wms_client_id_type client_id;
  wms_routes_s_type route_cfg;
  uint32 user_data;
  errno_enum_type return_errno = E_SUCCESS;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_cfg_set_primary_client_api_name = "wms_cfg_ms_set_primary_client()";
  char *wms_cfg_set_routes_api_name = "wms_cfg_ms_set_routes()";
  #else
  char *wms_cfg_set_primary_client_api_name = "wms_cfg_set_primary_client()";
  char *wms_cfg_set_routes_api_name = "wms_cfg_set_routes()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  client_id = i_ptr->client_info[ modem_id ].client_id;

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  (void) qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id );
  #endif /* FEATURE_QCRIL_DSDS */

  /* Set QCRIL to be the primary client of WMS */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, QCRIL_REQ_ID_INTERNAL );
  QCRIL_LOG_RPC( instance_id, wms_cfg_set_primary_client_api_name, "client_id", client_id );
  if ( qcril_sms_api_funcs[ modem_id ].wms_cfg_set_primary_client_func( client_id, 
                                                                        #ifdef FEATURE_QCRIL_DSDS
                                                                        as_id,
                                                                        #endif /* FEATURE_QCRIL_DSDS */
                                                                        qcril_sms_cmd_callback, (void *) user_data, TRUE,
                                                                        TRUE ) != WMS_OK_S )
  {
    QCRIL_LOG_ERROR( "RID %d MID %d Failed to set QCRIL as primary client of WMS\n", instance_id, modem_id );
     return_errno = E_FAILURE;
  }

  /* Configure the routes.  All classes are transfer-only except class 2, which is store to SIM. */
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_0 ].route = WMS_ROUTE_TRANSFER_ONLY;
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_0 ].mem_store = WMS_MEMORY_STORE_NONE;

  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_1 ].route = WMS_ROUTE_TRANSFER_ONLY;
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_1 ].mem_store = WMS_MEMORY_STORE_NONE;

  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_2 ].route = WMS_ROUTE_STORE_AND_NOTIFY;
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_2 ].mem_store = WMS_MEMORY_STORE_SIM;

  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_3 ].route = WMS_ROUTE_TRANSFER_ONLY;
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_3 ].mem_store = WMS_MEMORY_STORE_NONE;

  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_NONE ].route = WMS_ROUTE_TRANSFER_ONLY;
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_NONE ].mem_store = WMS_MEMORY_STORE_NONE;

  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_CDMA ].route = WMS_ROUTE_TRANSFER_ONLY;
  route_cfg.pp_routes[ WMS_MESSAGE_CLASS_CDMA ].mem_store = WMS_MEMORY_STORE_NONE;

  /* We want Status Reports to be sent to RIL, not stored */
  route_cfg.transfer_status_report = TRUE;

  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, QCRIL_REQ_ID_INTERNAL );
  QCRIL_LOG_RPC2( instance_id, wms_cfg_set_routes_api_name, "Store CLASS_2 on SIM" );
  if ( qcril_sms_api_funcs[ modem_id ].wms_cfg_set_routes_func( client_id, 
                                                                #ifdef FEATURE_QCRIL_DSDS
                                                                as_id,
                                                                #endif /* FEATURE_QCRIL_DSDS */
                                                                qcril_sms_cmd_callback,
                                                                (void *) user_data, &route_cfg ) != WMS_OK_S )
  {
    QCRIL_LOG_ERROR( "MID %d Failed to configure routes\n", modem_id );
    return_errno = E_FAILURE;
  }

  return return_errno;

} /* qcril_sms_set_primary_client_and_route */


/*===========================================================================

  FUNCTION:  qcril_sms_init

===========================================================================*/
/*!
    @brief
    Initialize the SMS subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
errno_enum_type qcril_sms_init
( 
  void 
)
{
  uint8 i, j;
  qcril_sms_struct_type *i_ptr;
  wms_client_id_type client_id;
  errno_enum_type return_errno = E_SUCCESS;
  #ifdef FEATURE_QCRIL_WMS_PM
  uint32 cfg_event_mask = 0, msg_event_mask = 0, bc_event_mask = 0, k= 0;
  #endif /* FEATURE_QCRIL_WMS_PM */
  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_WMS_PM
  wms_cfg_event_e_type wms_cfg_event_list[] = {
    WMS_CFG_EVENT_MESSAGE_LIST,
    #ifdef FEATURE_QCRIL_DSDS
    WMS_CFG_EVENT_MS_MEMORY_FULL,
    WMS_CFG_EVENT_MS_MEMORY_STATUS_SET,
    #else
    WMS_CFG_EVENT_MEMORY_FULL,
    WMS_CFG_EVENT_MEMORY_STATUS_SET
    #endif /* FEATURE_QCRIL_DSDS */
  };
  
  wms_msg_event_e_type wms_msg_event_list[] = {
    WMS_MSG_EVENT_SEND,
    WMS_MSG_EVENT_WRITE,
    WMS_MSG_EVENT_DELETE,
    WMS_MSG_EVENT_READ_TEMPLATE,
    WMS_MSG_EVENT_WRITE_TEMPLATE,
    WMS_MSG_EVENT_RECEIVED_MESSAGE,
    WMS_MSG_EVENT_SUBMIT_REPORT,
    WMS_MSG_EVENT_STATUS_REPORT
    #ifdef FEATURE_QCRIL_IMS
    ,WMS_MSG_EVENT_TRANSPORT_REG
    #endif /* FEATURE_QCRIL_IMS */
    #ifdef FEATURE_QCRIL_WMS_ETWS
    ,WMS_MSG_EVENT_ETWS_NOTIFICATION
    #endif /* FEATURE_QCRIL_WMS_ETWS */
  };

  wms_msg_event_e_type wms_bc_mm_event_list[] = {
    WMS_BC_MM_EVENT_PREF,
    WMS_BC_MM_EVENT_TABLE,
    WMS_BC_MM_EVENT_ADD_SRVS
  };

  #endif /*FEATURE_QCRIL_WMS_PM */

  /* Allow cache */
  qcril_sms = (qcril_sms_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_SMS );
  QCRIL_ASSERT( qcril_sms != NULL );

  /* Initialize internal data */
  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_sms[ i ];

    pthread_mutex_init( &i_ptr->sms_ack_info_mutex, NULL );

    #ifdef FEATURE_QCRIL_IMS
    pthread_mutex_init( &i_ptr->transport_reg_info_mutex, NULL );
    i_ptr->ims_client_msg_ref = WMS_DUMMY_MESSAGE_NUMBER;
    i_ptr->ims_wms_msg_ref = WMS_DUMMY_MESSAGE_NUMBER;
    #endif /* FEATURE_QCRIL_IMS */

    for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
    {
      i_ptr->sms_ack_info[ j ].gw_ack_is_needed = TRUE;
      i_ptr->sms_ack_info[ j ].cdma_ack_is_needed = TRUE;

      #ifdef FEATURE_QCRIL_IMS
      i_ptr->transport_reg_info[ j ].is_registered = FALSE;
      #endif /* FEATURE_QCRIL_IMS */

      /* Default client data */
      i_ptr->client_info[ j ].client_id_is_valid = FALSE;
      if ( QCRIL_PRIMARY_INSTANCE( i ) )
      {
        i_ptr->client_info[ i ].client_is_primary = TRUE;
      }
      else
      {
        i_ptr->client_info[ j ].client_is_primary = FALSE;
      }
    }
  }

  /* Initializes QCRIL as the clients of WMS. */
  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    QCRIL_LOG_RPC( i, "wms_client_init()", "client_id", client_id );
    if ( qcril_sms_api_funcs[ i ].wms_client_init_func( WMS_CLIENT_TYPE_RIL,
                                                        &client_id
                                                        #ifdef FEATURE_QCRIL_WMS_PM
                                                        ,WMS_CLIENT_PROCESSOR_APP1
                                                        #endif
                                                         ) != WMS_CLIENT_ERR_NONE )
    {
      QCRIL_LOG_ERROR( "MID %d Failed to register as SMS client\n", i );
      return_errno = E_FAILURE;
    }
    else
    {
      /* Record client ID */
      for ( j = 0; j < QCRIL_MAX_INSTANCE_ID; j++ )
      {
        i_ptr = &qcril_sms[ j ];
        i_ptr->client_info[ i ].client_id_is_valid = TRUE;
        i_ptr->client_info[ i ].client_id = client_id;
      }

      #ifdef FEATURE_QCRIL_WMS_PM
      for ( k = 0; k < QCRIL_ARR_SIZE( wms_cfg_event_list ); k++ )
      {
        cfg_event_mask = cfg_event_mask + ( 1 << wms_cfg_event_list[k] );
      }

      for ( k = 0; k < QCRIL_ARR_SIZE( wms_msg_event_list ); k++ )
      {
        msg_event_mask = msg_event_mask + ( 1 << wms_msg_event_list[k] );
      }

      for ( k = 0; k < QCRIL_ARR_SIZE( wms_bc_mm_event_list ); k++ )
      {
        bc_event_mask = bc_event_mask + ( 1 << wms_bc_mm_event_list[k] );
      }
      QCRIL_LOG_DEBUG( "the bit mask value for cfg_event = %d, msg_event = %d, bc_event = %d\n",
                       cfg_event_mask, msg_event_mask, bc_event_mask);
      #endif /* FEATURE_QCRIL_WMS_PM */

      /* Register the configuration call back function */
      QCRIL_LOG_RPC( i, "wms_client_reg_cfg_cb()", "client_id", client_id );
      if ( qcril_sms_api_funcs[ i ].wms_client_reg_cfg_cb_func( client_id,
                                                                #ifdef FEATURE_QCRIL_WMS_PM
                                                                WMS_PM_NOTIFY_CUSTOM_FILTER,
                                                                cfg_event_mask,
                                                                #endif
                                                                qcril_sms_api_callbacks[ i ].cfg_event_cb_func ) != WMS_CLIENT_ERR_NONE )
      {
        QCRIL_LOG_ERROR( "MID %d Failed to register as SMS client\n", i );
        return_errno = E_FAILURE;
      }

      /* Register the message call back function */
      QCRIL_LOG_RPC( i, "wms_client_reg_msg_cb()", "client_id", client_id );
      if ( qcril_sms_api_funcs[ i ].wms_client_reg_msg_cb_func( client_id,
                                                                #ifdef FEATURE_QCRIL_WMS_PM
                                                                WMS_PM_NOTIFY_CUSTOM_FILTER,
                                                                msg_event_mask,
                                                                #endif
                                                                qcril_sms_api_callbacks[ i ].msg_event_cb_func ) != WMS_CLIENT_ERR_NONE )
      {
        QCRIL_LOG_ERROR( "MID %d Failed to register message callback function\n", i );
        return_errno = E_FAILURE;
      }

      /* Register the broadcast call back function */
      QCRIL_LOG_RPC( i, "wms_client_reg_bc_mm_cb()", "client_id", client_id );
      if ( qcril_sms_api_funcs[ i ].wms_client_reg_bc_mm_cb_func( client_id,
                                                                  #ifdef FEATURE_QCRIL_WMS_PM
                                                                  WMS_PM_NOTIFY_CUSTOM_FILTER,
                                                                  bc_event_mask,
                                                                  #endif
                                                                  qcril_sms_api_callbacks[ i ].bc_mm_event_cb_func ) != WMS_CLIENT_ERR_NONE )
      {
        QCRIL_LOG_ERROR( "MID %d Failed to register broadcast callback function\n", i );
        return_errno = E_FAILURE;
      }

      /* Activate QCRIL */
      QCRIL_LOG_RPC( i, "wms_client_activate()", "client_id", client_id );
      if ( qcril_sms_api_funcs[ i ].wms_client_activate_func( client_id ) != WMS_CLIENT_ERR_NONE )
      {
        QCRIL_LOG_ERROR( "MID %d Failed to activate QCRIL as WMS client\n", i );
        return_errno = E_FAILURE;
      }

      /* For non-DSDS, configure primary client and route here. Otherwise wait for subscription info before doing any config */
      if ( !qcril_arb_ma_is_dsds() )
      {
        return_errno = qcril_sms_set_primary_client_and_route( QCRIL_DEFAULT_INSTANCE_ID, i );
      }
    }
  }

  /* Read NV_SMS_UTC_I */
  qcril_sms_read_nv_sms_utc();

  return return_errno;

} /* qcril_sms_init() */


/*=========================================================================
  FUNCTION:  qcril_sms_release

===========================================================================*/
/*!
    @brief
    Release the WMS client object.

    @return
    none
*/
/*=========================================================================*/
void qcril_sms_release
( 
  void 
)
{
  uint8 i, j;
  qcril_sms_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_sms[ i ];

    for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
    {
      if ( i_ptr->client_info[ j ].client_id_is_valid )
      {
        i_ptr->client_info[ j ].client_id_is_valid = FALSE;

        if ( i_ptr->client_info[ j ].client_is_primary )
        {
          QCRIL_LOG_RPC( j, "wms_client_release()", "client_id", i_ptr->client[ j ].client_id );
          (void) qcril_sms_api_funcs[ j ].wms_client_release_func( i_ptr->client_info[ j ].client_id ); 
        }
      }
    }
  }

} /* qcril_sms_release() */


/*=========================================================================
  FUNCTION:  qcril_sms_force_state_transition_to_wait_amss_event

===========================================================================*/
/*!
    @brief
    Change the state of the entry in the ReqList to QCRIL_REQ_AWAITING_AMSS_EVENTS.

    @return
    None
*/
/*=========================================================================*/
static void qcril_sms_force_state_transition_to_wait_amss_event
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  RIL_Token t,
  int request,
  qcril_evt_e_type pending_event_id
)
{
  IxErrnoType err_no;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  /* Change the state of the entry in ReqList to QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS */
  err_no = qcril_reqlist_update_state( instance_id, modem_id, t, QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS );

  /* State transition failure */
  if ( err_no == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "Changed the state of ReqList entry to AWAITING_MORE_AMSS_EVENTS state : %s (%d), Token ID %d, pending %s (%d)\n", 
                     qcril_log_lookup_event_name( request ), request, qcril_log_get_token_id( t ),  
                     qcril_log_lookup_event_name( pending_event_id ), pending_event_id ); 
  }
  else if ( err_no == E_NOT_ALLOWED )
  {
    QCRIL_LOG_ERROR( "Failed to change the state of ReqList entry to AWAITING_MORE_AMSS_EVENTS state : %s (%d), Token ID %d, pending %s (%d)\n", 
                     qcril_log_lookup_event_name( request ), request, qcril_log_get_token_id( t ),  
                     qcril_log_lookup_event_name( pending_event_id ), pending_event_id ); 

    /* Send RIL_E_GENERIC_FAILURE response*/
    qcril_default_request_resp_params( instance_id, t, request, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_ERROR( "ReqList entry not found : %s (%d), Token ID %d\n", 
                     qcril_log_lookup_event_name( request ), request, qcril_log_get_token_id( t ) );  
  }

} /* qcril_sms_force_state_transition_to_wait_amss_event */


/*===========================================================================

  FUNCTION:  qcril_mo_sms_error_check

===========================================================================*/
/*!
    @brief
    Does error checking on mobile-originated SMS.  This checking should be
    done before sending an SMS, or writing an SMS to SIM.

    @return
    TRUE if SMS passes error-checking; FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_mo_sms_error_check
( 
  qcril_instance_id_e_type instance_id,
  const char *pdu,
  RIL_Token t,
  int event_id
)
{   
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  /* Check if the PDU is null */
  if ( pdu == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "PDU in SMS is NULL!\n");
    qcril_default_request_resp_params( instance_id, t, event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return FALSE;
  }

  /* Check if the length exceeds the maximum allowed by WMS */
  if ( ( strlen( pdu ) / 2 ) > WMS_MAX_LEN )
  {
    QCRIL_LOG_ERROR( "%s", "PDU in SMS exceeds maximum allowable length!\n");
    qcril_default_request_resp_params( instance_id, t, event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return FALSE; 
  }

  /* All error checks passed */
  return TRUE;

} /* qcril_mo_sms_error_check */


/*===========================================================================

  FUNCTION:  qcril_sms_fill_mo_wms_payload

===========================================================================*/
/*!
    @brief
    Fill MO SMS payload per WMS API requirement.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_fill_mo_wms_payload
(
  boolean sms_over_ims,
  boolean payload_in_cdma_format,
  RIL_CDMA_SMS_Message *cdma_sms_msg,
  const char *gw_smsc_address,
  const char *gw_pdu,   
  wms_client_message_s_type *buf_ptr
)
{
  cm_called_party_bcd_no_T BCD_address;
  uint32 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( buf_ptr != NULL );

  if ( payload_in_cdma_format )
  {
    QCRIL_ASSERT( cdma_sms_msg != NULL );
  }
  else
  {
    QCRIL_ASSERT( gw_pdu != NULL );
  }

  /*-----------------------------------------------------------------------*/

  /* 3GPP2 format */
  if ( payload_in_cdma_format )
  {
    buf_ptr->msg_hdr.message_mode = WMS_MESSAGE_MODE_CDMA;
    buf_ptr->msg_hdr.tag = WMS_TAG_MO_NOT_SENT;
    buf_ptr->msg_hdr.mem_store = WMS_MEMORY_STORE_NONE;
    buf_ptr->msg_hdr.index = WMS_DUMMY_MESSAGE_INDEX;

    /* Fill in the message */
    buf_ptr->u.cdma_message.is_mo = TRUE;
    buf_ptr->u.cdma_message.teleservice = cdma_sms_msg->uTeleserviceID;

    buf_ptr->u.cdma_message.address.digit_mode = cdma_sms_msg->sAddress.digit_mode;
    buf_ptr->u.cdma_message.address.number_mode = cdma_sms_msg->sAddress.number_mode;
    buf_ptr->u.cdma_message.address.number_type = cdma_sms_msg->sAddress.number_type;
    buf_ptr->u.cdma_message.address.number_plan = cdma_sms_msg->sAddress.number_plan;
    buf_ptr->u.cdma_message.address.number_of_digits = cdma_sms_msg->sAddress.number_of_digits;

    for ( i = 0; i < buf_ptr->u.cdma_message.address.number_of_digits; i++)
    {
      buf_ptr->u.cdma_message.address.digits[ i ] = cdma_sms_msg->sAddress.digits[i];
    }

    buf_ptr->u.cdma_message.subaddress.type = cdma_sms_msg->sSubAddress.subaddressType;
    buf_ptr->u.cdma_message.subaddress.odd = cdma_sms_msg->sSubAddress.odd;
    buf_ptr->u.cdma_message.subaddress.number_of_digits = cdma_sms_msg->sSubAddress.number_of_digits;

    for ( i = 0; i < buf_ptr->u.cdma_message.subaddress.number_of_digits; i++)
    {
      buf_ptr->u.cdma_message.subaddress.digits[ i ] = cdma_sms_msg->sSubAddress.digits[i];
    }

    if ( sms_over_ims )
    {
      buf_ptr->u.cdma_message.is_tl_ack_requested = FALSE;
    }
    else
    {
      buf_ptr->u.cdma_message.is_tl_ack_requested = TRUE;
    }
    buf_ptr->u.cdma_message.is_service_present = cdma_sms_msg->bIsServicePresent;
    buf_ptr->u.cdma_message.service = cdma_sms_msg->uServicecategory;

    buf_ptr->u.cdma_message.raw_ts.format = WMS_FORMAT_CDMA;
    buf_ptr->u.cdma_message.raw_ts.len = cdma_sms_msg->uBearerDataLen;

    (void) memcpy( buf_ptr->u.cdma_message.raw_ts.data, cdma_sms_msg->aBearerData, buf_ptr->u.cdma_message.raw_ts.len );
  }
  /* 3GPP format */
  else
  {
    buf_ptr->msg_hdr.message_mode = WMS_MESSAGE_MODE_GW;
    buf_ptr->msg_hdr.tag = WMS_TAG_MO_NOT_SENT;
    buf_ptr->msg_hdr.mem_store = WMS_MEMORY_STORE_NONE;
    buf_ptr->msg_hdr.index = WMS_DUMMY_MESSAGE_INDEX;

    buf_ptr->u.gw_message.is_broadcast = FALSE;

    if ( gw_smsc_address == NULL )
    {
      /* The input SMSC address is NULL.  Set the number of digits to 0, so WMS will
         populate the SMSC address from the SIM card. */
      buf_ptr->u.gw_message.sc_address.number_of_digits = 0; 
    }
    else
    {
      /* Fill in the SMSC address */
      BCD_address.present = TRUE;
      BCD_address.length = 0;

      /* The first two ASCII hex chars of the SMSC address are the length. */ 
      qcril_sms_hex_to_byte( gw_smsc_address, &BCD_address.length, 2 );

      /* Convert the SMSC address from ASCII hex format to a byte array. */
      qcril_sms_hex_to_byte( &gw_smsc_address[ 2 ], &BCD_address.data[ 0 ], BCD_address.length * 2 );

      /* Call a WMS conversion function to translate from a byte array to WMS format. */
      qcril_sms_ts_convert_from_bcd_address( &BCD_address, &buf_ptr->u.gw_message.sc_address );
    }

    buf_ptr->u.gw_message.raw_ts_data.format = WMS_FORMAT_GW_PP;
    buf_ptr->u.gw_message.raw_ts_data.tpdu_type = WMS_TPDU_SUBMIT;
    /* The input is in ASCII hex format.  We are going to pack it into a byte array.  Two
       ASCII hex characters fit in each byte, so divide the length by two. */
    buf_ptr->u.gw_message.raw_ts_data.len = strlen( gw_pdu ) / 2;

    /* Translate the PDU from ASCII hex to bytes */
    qcril_sms_hex_to_byte( gw_pdu, &buf_ptr->u.gw_message.raw_ts_data.data[ 0 ], strlen( gw_pdu ) );
  }

} /* qcril_sms_fill_mo_wms_payload */


/*===========================================================================

  FUNCTION:  qcril_sms_subscription_pref_info

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_SMS_SUBSCRIPTION_PREF_INFO.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_subscription_pref_info
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Configure primary client and route */
  if ( qcril_sms_set_primary_client_and_route( instance_id, modem_id ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "RID %d MID %d Fail to configure as primary client and route\n", instance_id, modem_id );
  }

} /* qcril_sms_subscripton_pref_info */


/*===========================================================================

  FUNCTION:  qcril_sms_request_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_send_sms
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  const char *smsc_address;
  const char *gw_pdu;   
  wms_client_message_s_type *buf_ptr;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_send_api_name = "wms_msg_ms_send(GW)";
  #else
  char *wms_msg_send_api_name = "wms_msg_send(GW)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( !qcril_mo_sms_error_check( instance_id, ( ( const char ** ) params_ptr->data)[ 1 ], params_ptr->t, params_ptr->event_id ) )
  {
    return;
  }

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one mode is configured for LTE/G/W SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, QCRIL_EVT_SMS_SEND, 
                               NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Fill in the message */
  buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the GW SMS message to be sent" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Fill in the message */
  smsc_address = ( (const char **) params_ptr->data )[ 0 ];
  gw_pdu = ( (const char **) params_ptr->data )[ 1 ];
  qcril_sms_fill_mo_wms_payload( FALSE, FALSE, NULL, smsc_address, gw_pdu, buf_ptr );

  /* Ask WMS to send the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_msg_send_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ) );
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_send_func( i_ptr->client_info[ modem_id ].client_id,
                                                            #ifdef FEATURE_QCRIL_DSDS
                                                            as_id,
                                                            #endif /* FEATURE_QCRIL_DSDS */
                                                            qcril_sms_cmd_callback,
                                                            (void *) user_data,
                                                            WMS_SEND_MODE_CLIENT_MESSAGE,
                                                            buf_ptr );

  qcril_free( buf_ptr );

} /* qcril_sms_request_send_sms */


/*===========================================================================

  FUNCTION:  qcril_sms_request_send_sms_expect_more

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SEND_SMS_EXPECT_MORE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_send_sms_expect_more
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_cfg_set_link_control_api_name = "wms_cfg_ms_set_link_control()";
  #else
  char *wms_cfg_set_link_control_api_name = "wms_cfg_set_link_control()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( !qcril_mo_sms_error_check( instance_id, ( (const char **) params_ptr->data)[ 1 ], params_ptr->t, params_ptr->event_id ) )
  {
    return;
  }

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one mode is configured for LTE/G/W SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Configure WMS so that it keeps the link open for 5 seconds after the SMS message is sent. */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, QCRIL_REQ_ID_INTERNAL );
  QCRIL_LOG_RPC( modem_id, wms_cfg_set_link_control_api_name, "WMS_LINK_CONTROL_ENABLED_ONE Token ID", 
                 qcril_log_get_token_id( params_ptr->t ));
  if ( qcril_sms_api_funcs[ modem_id ].wms_cfg_set_link_control_func( i_ptr->client_info[ modem_id ].client_id,
                                                                      #ifdef FEATURE_QCRIL_DSDS
                                                                      as_id,
                                                                      #endif /* FEATURE_QCRIL_DSDS */
                                                                      qcril_sms_cmd_callback,
                                                                      (void *) user_data,
                                                                      WMS_LINK_CONTROL_ENABLED_ONE,
                                                                      5 ) != WMS_OK_S )
  {
    QCRIL_LOG_ERROR( "%s", "Call to wms_cfg_set_link_control failed.\n");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Send the SMS */
  qcril_sms_request_send_sms( params_ptr, ret_ptr );

} /* qcril_sms_request_send_sms_expect_more */


/*===========================================================================

  FUNCTION:  qcril_sms_request_sms_acknowledge

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_sms_acknowledge
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint8 i;
  int tp_cause;
  wms_ack_info_s_type *buf_ptr;
  uint32 user_data;
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_ack_api_name = "wms_msg_ms_ack()";
  #else
  char *wms_msg_ack_api_name = "wms_msg_ack()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );
  
  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /* Lookup the modem where the received SMS is pending to be acked */
  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    if ( i_ptr->sms_ack_info[ i ].gw_ack_pending )
    {
      modem_id = i;
      break;
    }
  }

  if ( i >= QCRIL_MAX_MODEM_ID )
  {
    QCRIL_LOG_DEBUG( "%s", "Dropping GW Ack request since no ack is pending.\n" );

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );

    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* WMS already acked this message.  Drop the ack. */
  if ( !i_ptr->sms_ack_info[ modem_id ].gw_ack_is_needed )
  {
    QCRIL_LOG_DEBUG( "%s", "Dropping GW Ack since WMS already acked the message.\n" );

    i_ptr->sms_ack_info[ modem_id ].gw_ack_pending = FALSE;

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );

    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  /* Fill in the ack */
  buf_ptr = (wms_ack_info_s_type *) qcril_malloc( sizeof( wms_ack_info_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the GW SMS ACK to be sent" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );

    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  buf_ptr->transaction_id = i_ptr->sms_ack_info[ modem_id ].gw_transaction_id;
  buf_ptr->message_mode = WMS_MESSAGE_MODE_GW;

  buf_ptr->u.gw.success = ((int *) params_ptr->data)[0];

  if ( buf_ptr->u.gw.success )
  {
    buf_ptr->u.gw.tpdu_type = WMS_TPDU_NONE;
  }
  else
  {
    tp_cause = ( (int *) params_ptr->data)[ 1 ];

    /* Initialize the RP cause to unspecified */
    buf_ptr->u.gw.rp_cause = WMS_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;

    /* If the TP cause is memory full, set the RP cause to memory full also */
    if ( tp_cause == WMS_TP_CAUSE_MEMORY_FULL )
    {
      QCRIL_LOG_DEBUG( "%s\n", "Memory full" );
      buf_ptr->u.gw.rp_cause = WMS_RP_CAUSE_MEMORY_EXCEEDED;
    }

    /* Include the TPDU, and set the TP cause */
    buf_ptr->u.gw.tpdu_type = WMS_TPDU_DELIVER_REPORT_ERROR;
    buf_ptr->u.gw.u.error.user_data_header_present = FALSE;
    buf_ptr->u.gw.u.error.tp_cause = tp_cause;
    buf_ptr->u.gw.u.error.mask = 0;
  }

  /* Request WMS to send the ack. */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "Token ID %d, success %d", 
                  qcril_log_get_token_id( params_ptr->t ), ( (int *) params_ptr->data)[ 0 ]);
  QCRIL_LOG_RPC2( modem_id, wms_msg_ack_api_name, details );
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_ack_func( i_ptr->client_info[ modem_id ].client_id,
                                                           #ifdef FEATURE_QCRIL_DSDS
                                                           as_id,
                                                           #endif /* FEATURE_QCRIL_DSDS */
                                                           qcril_sms_cmd_callback,
                                                           (void *) user_data,
                                                           buf_ptr );

  qcril_free( buf_ptr );

  /* Reset the ack pending flag.  If the ack is not received by the network, the network
     will resend the SMS, and RIL will have another chance to ack it. */
  i_ptr->sms_ack_info[ modem_id ].gw_ack_pending = FALSE;

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

} /* qcril_sms_request_sms_acknowledge */


/*===========================================================================

  FUNCTION:  qcril_sms_request_write_sms_to_sim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_WRITE_SMS_TO_SIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_write_sms_to_sim
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  RIL_SMS_WriteArgs *SMS_Write_info;
  wms_gw_tpdu_type_e_type tpdu_type = WMS_TPDU_NONE;  
  cm_called_party_bcd_no_T BCD_address;
  uint32 user_data;
  wms_client_message_s_type *buf_ptr;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_write_api_name = "wms_msg_ms_write(SIM)";
  #else
  char *wms_msg_write_api_name = "wms_msg_write(SIM)";
  #endif /* FEATURE_QCRIL_DSDS */
  
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  SMS_Write_info = (RIL_SMS_WriteArgs *) params_ptr->data;
  QCRIL_ASSERT( SMS_Write_info != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( !qcril_mo_sms_error_check( instance_id, SMS_Write_info->pdu, params_ptr->t, params_ptr->event_id ) )
  {
    return;
  }

  /* Invalid status */
  if ( SMS_Write_info->status  > 3 )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one mode is configured for LTE/G/W SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_WRITE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Fill in the SMS message */
  buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the SMS message to be written to SIM" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  buf_ptr->msg_hdr.message_mode = WMS_MESSAGE_MODE_GW;

  switch ( SMS_Write_info->status )
  {
    case 0:
      buf_ptr->msg_hdr.tag = WMS_TAG_MT_NOT_READ;
      tpdu_type = WMS_TPDU_DELIVER;  
      break;

    case 1:
      buf_ptr->msg_hdr.tag = WMS_TAG_MT_READ;
      tpdu_type = WMS_TPDU_DELIVER;  
      break;

    case 2:
      buf_ptr->msg_hdr.tag = WMS_TAG_MO_NOT_SENT;
      tpdu_type = WMS_TPDU_SUBMIT;
      break;

    case 3:
      buf_ptr->msg_hdr.tag = WMS_TAG_MO_SENT;
      tpdu_type = WMS_TPDU_SUBMIT;
      break;

    default:
      break;
  }

  buf_ptr->msg_hdr.mem_store = WMS_MEMORY_STORE_SIM;
  buf_ptr->msg_hdr.index = WMS_DUMMY_MESSAGE_INDEX;

  buf_ptr->u.gw_message.is_broadcast = FALSE;

  /* Fill in the address */
  if ( SMS_Write_info->smsc == NULL )
  {
    /* The input SMSC address is NULL.  Set the number of digits to 0, so WMS will populate the SMSC address from the SIM card. */
    buf_ptr->u.gw_message.sc_address.number_of_digits = 0; 
  }
  else
  {
    /* Fill in the SMSC address */
    BCD_address.present = TRUE;
    BCD_address.length = 0;

    /* The first two ASCII hex chars of the SMSC address are the length. */ 
    qcril_sms_hex_to_byte( SMS_Write_info->smsc, &BCD_address.length, 2 );

    /* Convert the SMSC address from ASCII hex format to a byte array. */
    qcril_sms_hex_to_byte( &SMS_Write_info->smsc[ 2 ], &BCD_address.data[ 0 ], BCD_address.length * 2 );

    /* Call a WMS conversion function to translate from a byte array to WMS format. */
    qcril_sms_ts_convert_from_bcd_address( &BCD_address, &buf_ptr->u.gw_message.sc_address );
  }

  buf_ptr->u.gw_message.raw_ts_data.format = WMS_FORMAT_GW_PP;
  buf_ptr->u.gw_message.raw_ts_data.tpdu_type = tpdu_type;

  /* The input is in ASCII hex format.  We are going to pack it into a byte array.  Two
     ASCII hex characters fit in each byte, so divide the length by two. */
  buf_ptr->u.gw_message.raw_ts_data.len = strlen( SMS_Write_info->pdu ) / 2;

  /* Translate the PDU from ASCII hex to bytes */
  qcril_sms_hex_to_byte( SMS_Write_info->pdu, &buf_ptr->u.gw_message.raw_ts_data.data[ 0 ], strlen( SMS_Write_info->pdu ) );

  /* Ask WMS to write the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_msg_write_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ) );
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_write_func( i_ptr->client_info[ modem_id ].client_id,
                                                             #ifdef FEATURE_QCRIL_DSDS
                                                             as_id,
                                                             #endif /* FEATURE_QCRIL_DSDS */
                                                             qcril_sms_cmd_callback,
                                                             (void *) user_data,
                                                             WMS_WRITE_MODE_INSERT,
                                                             buf_ptr );

  qcril_free( buf_ptr );

} /* qcril_sms_request_write_sms_to_sim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_delete_sms_on_sim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DELETE_SMS_ON_SIM 

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_delete_sms_on_sim
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_delete_api_name = "wms_msg_ms_delete(SIM)";
  #else
  char *wms_msg_delete_api_name = "wms_msg_delete(SIM)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one mode is configured for LTE/G/W SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_DELETE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Ask WMS to delete the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "Token ID %d, index %d", 
                  qcril_log_get_token_id( params_ptr->t ), ( (int *) params_ptr->data)[ 0 ] );
  QCRIL_LOG_RPC2( modem_id, wms_msg_delete_api_name, details );

  /* Android uses the GSDI index, which starts at 1.  Subtract 1 to get the WMS index. */
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_delete_func( i_ptr->client_info[ modem_id ].client_id,
                                                              #ifdef FEATURE_QCRIL_DSDS
                                                              as_id,
                                                              #endif /* FEATURE_QCRIL_DSDS */
                                                              qcril_sms_cmd_callback,
                                                              (void *) user_data,
                                                              WMS_MEMORY_STORE_SIM,
                                                              ( (int *) params_ptr->data)[ 0 ] - 1 );

} /* qcril_sms_request_delete_sms_on_sim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_get_smsc_address

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_get_smsc_address
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_cfg_get_message_list_api_name = "wms_cfg_ms_get_message_list()";
  #else
  char *wms_cfg_get_message_list_api_name = "wms_cfg_get_message_list()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one mode is configured for LTE/G/W SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_CFG_MESSAGE_LIST, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /*-----------------------------------------------------------------------*/

  /* Ask WMS to retrieve all templates from the SIM card */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_cfg_get_message_list_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ) );
  (void) qcril_sms_api_funcs[ modem_id ].wms_cfg_get_message_list_func( i_ptr->client_info[ modem_id ].client_id,
                                                                        #ifdef FEATURE_QCRIL_DSDS
                                                                        as_id,
                                                                        #endif /* FEATURE_QCRIL_DSDS */
                                                                        qcril_sms_cmd_callback,
                                                                        (void *) user_data,
                                                                        WMS_MEMORY_STORE_SIM,
                                                                        WMS_TAG_MO_TEMPLATE );

} /* qcril_sms_request_get_smsc_address() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_set_smsc_address

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_set_smsc_address
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_cfg_get_message_list_api_name = "wms_cfg_ms_get_message_list()";
  #else
  char *wms_cfg_get_message_list_api_name = "wms_cfg_get_message_list()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Translate the SMSC address from ATCOP format to WMS format */
  if ( !qcril_sms_convert_smsc_address_to_wms_format( (char *) params_ptr->data, &i_ptr->smsc_address ) )
  {
    QCRIL_LOG_ERROR( "%s", "Unable to convert SMSC address to internal format");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }
 
  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one mode is configured for LTE/G/W SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_CFG_MESSAGE_LIST, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Ask WMS to retrieve all templates from the SIM card */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_cfg_get_message_list_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ) );
  (void) qcril_sms_api_funcs[ modem_id ].wms_cfg_get_message_list_func( i_ptr->client_info[ modem_id ].client_id,
                                                                        #ifdef FEATURE_QCRIL_DSDS
                                                                        as_id,
                                                                        #endif /* FEATURE_QCRIL_DSDS */
                                                                        qcril_sms_cmd_callback,
                                                                        (void *) user_data,
                                                                        WMS_MEMORY_STORE_SIM,
                                                                        WMS_TAG_MO_TEMPLATE );

} /* qcril_sms_request_set_smsc_address() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_report_sms_memory_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_REPORT_SMS_MEMORY_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_report_sms_memory_status
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  uint8 i;
  qcril_modem_ids_list_type modem_ids_list;
  boolean mem_full = FALSE;
  char *mem_full_name = "";
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_cfg_set_memory_full_api_name = "wms_cfg_ms_set_memory_full()";
  #else
  char *wms_cfg_set_memory_full_api_name = "wms_cfg_set_memory_full()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_COMMON, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_ids_list.modem_id[ 0 ], QCRIL_REQ_AWAITING_CALLBACK, 
                               #ifdef FEATURE_QCRIL_DSDS
                               QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET
                               #else
                               QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET 
                               #endif /* FEATURE_QCRIL_DSDS */
                               , NULL, &reqlist_entry ); 
  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    reqlist_entry.state[ i ] = QCRIL_REQ_AWAITING_CALLBACK;
    #ifdef FEATURE_QCRIL_DSDS
    reqlist_entry.pending_event_id[ i ] = QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET;
    #else
    reqlist_entry.pending_event_id[ i ] = QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET;
    #endif /* FEATURE_QCRIL_DSDS */
  }

  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if ( ( (int *) params_ptr->data)[ 0 ] == 0 )
  {
    mem_full = TRUE;
    mem_full_name = "Memory is full";
  }
  else if ( ( (int *) params_ptr->data)[ 0 ] == 1 )
  {
    mem_full = FALSE;
    mem_full_name = "Memory is available";
  }

  QCRIL_LOG_DEBUG( "Report Memory Status - %s\n", mem_full_name ); 

  /*-----------------------------------------------------------------------*/

  /* Request WMS to set the Memory Status */
  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
    QCRIL_SNPRINTF( details, sizeof( details ), "%s Token ID %d", mem_full_name, qcril_log_get_token_id( params_ptr->t ) );
    QCRIL_LOG_RPC2( modem_id, wms_cfg_set_memory_full_api_name, details );
    (void) qcril_sms_api_funcs[ modem_id ].wms_cfg_set_memory_full_func( i_ptr->client_info[ modem_id ].client_id,
                                                                         #ifdef FEATURE_QCRIL_DSDS
                                                                         as_id,
                                                                         #endif /* FEATURE_QCRIL_DSDS */
                                                                         qcril_sms_cmd_callback,
                                                                         (void *) user_data,
                                                                         mem_full );
  }

} /* qcril_sms_request_report_sms_memory_status() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_get_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_get_broadcast_sms_config
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_modem_ids_list_type modem_ids_list;
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem is configured to LTE/G/W */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_BC_MM_TABLE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Request WMS to get the service table for Broadcast SMS */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "GW Token ID %d", qcril_log_get_token_id( params_ptr->t ) );
  QCRIL_LOG_RPC2( modem_id, "wms_bc_mm_get_table()", details );

  (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_get_table_func( i_ptr->client_info[ modem_id ].client_id,
                                                                   #ifdef FEATURE_QCRIL_DSDS
                                                                   as_id,
                                                                   #endif
                                                                   qcril_sms_cmd_callback,
                                                                   (void *) user_data,
                                                                   WMS_MESSAGE_MODE_GW );

} /* qcril_sms_request_gsm_get_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_set_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_set_broadcast_sms_config
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 i;
  uint8 bc_table_size;
  wms_bc_mm_service_info_s_type *buf_ptr;
  char details[ 80 ];
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Check if the broadcast SMS table is too big */
  bc_table_size = params_ptr->datalen / sizeof( RIL_GSM_BroadcastSmsConfigInfo * );
  if ( bc_table_size > QCRIL_SMS_BC_MM_TABLE_SIZE )
  {
    QCRIL_LOG_ERROR( "Size of GW Broadcast SMS Config table is too big : %d\n", bc_table_size );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem is configured to LTE/G/W */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_BC_MM_ADD_SRVS, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Fill in the Broadcast SMS configuration array */
  buf_ptr = ( wms_bc_mm_service_info_s_type * ) qcril_malloc( bc_table_size * sizeof( wms_bc_mm_service_info_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the Broadcast SMS Config table" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  for ( i = 0; i < bc_table_size; i++ )
  {
    /* Fill in from the ril.h interface */
    buf_ptr[ i ].srv_id.gw_cb_range.from = ( (RIL_GSM_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->fromServiceId;
    buf_ptr[ i ].srv_id.gw_cb_range.to = ( (RIL_GSM_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->toServiceId;
    buf_ptr[ i ].selected = ( (RIL_GSM_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->selected;

    /* Set reasonable default values */
    buf_ptr[ i ].priority = WMS_PRIORITY_NORMAL;
    buf_ptr[ i ].alert = WMS_BC_ALERT_DEFAULT;
    buf_ptr[ i ].max_messages = 20;
    buf_ptr[ i ].label[ 0 ] = 0;
    buf_ptr[ i ].label_encoding = WMS_ENCODING_GSM_7_BIT_DEFAULT;
  }

  /* Request WMS to delete all services from the broadcast service table */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, "wms_bc_mm_delete_all_services()", "GW Token ID", qcril_log_get_token_id( params_ptr->t ));
  (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_delete_all_services_func( i_ptr->client_info[ modem_id ].client_id,
                                                                             #ifdef FEATURE_QCRIL_DSDS
                                                                             as_id,
                                                                             #endif
                                                                             qcril_sms_cmd_callback,
                                                                             (void *) user_data,
                                                                             WMS_MESSAGE_MODE_GW );

  /*-----------------------------------------------------------------------*/

  /* Request WMS to add services to the broadcast service table */
  QCRIL_SNPRINTF( details, sizeof( details ), "GW Token ID %d", qcril_log_get_token_id( params_ptr->t ) );
  QCRIL_LOG_RPC2( modem_id, "wms_bc_mm_add_services()", details );
  (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_add_services_func( i_ptr->client_info[ modem_id ].client_id,
                                                                      #ifdef FEATURE_QCRIL_DSDS
                                                                      as_id,
                                                                      #endif
                                                                      qcril_sms_cmd_callback,
                                                                      (void *) user_data,
                                                                      WMS_MESSAGE_MODE_GW,
                                                                      bc_table_size,
                                                                      buf_ptr );

  qcril_free( buf_ptr );

} /* qcril_sms_request_gsm_set_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_sms_broadcast_activation

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_sms_broadcast_activation
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_modem_ids_list_type modem_ids_list;
  wms_bc_pref_e_type bc_pref = WMS_BC_PREF_MAX;
  char *bc_pref_name = "";
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */
  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );
  
  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem is configured to LTE/G/W */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_BC_MM_PREF, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if ( ( (int *) params_ptr->data)[ 0 ] == 0 )
  {
    bc_pref = WMS_BC_PREF_ACTIVATE_TABLE;
    bc_pref_name = "Activate";
  }
  else if ( ( (int *) params_ptr->data)[ 0 ] == 1 )
  {
    bc_pref = WMS_BC_PREF_DEACTIVATE;
    bc_pref_name = "Deactivate";
  }

  /* Request WMS to set the BC preference */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "%s GW Token ID %d", bc_pref_name, qcril_log_get_token_id( params_ptr->t ) );
  QCRIL_LOG_RPC2( modem_id, "wms_bc_mm_set_pref()", details );
  (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_set_pref_func( i_ptr->client_info[ modem_id ].client_id,
                                                                  #ifdef FEATURE_QCRIL_DSDS
                                                                  as_id,
                                                                  #endif
                                                                  qcril_sms_cmd_callback,
                                                                  (void *) user_data,
                                                                  WMS_MESSAGE_MODE_GW,
                                                                  bc_pref );

} /* qcril_sms_request_gsm_sms_broadcast_activation() */
                                         

/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_send_sms
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  RIL_CDMA_SMS_Message *cdma_sms_msg;
  wms_client_message_s_type *buf_ptr = NULL;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;
  char details[ 80 ];

  #ifdef FEATURE_QCRIL_IMS
  wms_message_number_type client_msg_id;
  wms_message_number_type wms_msg_id;
  #endif /* FEATURE_QCRIL_IMS */
  
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_send_api_name = "wms_msg_ms_send(CDMA)";
  #elif FEATURE_QCRIL_IMS_EXT
  char *wms_msg_resend_api_name = "wms_msg_resend(CDMA)";
  #else
  char *wms_msg_send_api_name = "wms_msg_send(CDMA)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  cdma_sms_msg = (RIL_CDMA_SMS_Message *) params_ptr->data;
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem is configured to send 1X/EVDO SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList. */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_SEND, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList*/
    return;
  }

  /* Fill in the message header */
  buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the CDMA SMS message to be sent" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Fill in the message */
  qcril_sms_fill_mo_wms_payload( FALSE, TRUE, cdma_sms_msg, NULL, NULL, buf_ptr );

  #ifdef FEATURE_QCRIL_IMS_EXT
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].transport_reg_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->transport_reg_info_mutex, details );

  client_msg_id = i_ptr->ims_client_msg_ref; 
  wms_msg_id = i_ptr->ims_wms_msg_ref; 

  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );

  /* Request WMS to resend the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "Token ID %d, Client msg ID %ld, WMS msg ID %ld", 
                  qcril_log_get_token_id( params_ptr->t ), client_msg_id, wms_msg_id );
  QCRIL_LOG_RPC2( modem_id, wms_msg_resend_api_name, details );

  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_resend_func( i_ptr->client_info[ modem_id ].client_id,
                                                              qcril_sms_cmd_callback,
                                                              (void *) user_data,
                                                              WMS_SEND_MODE_CLIENT_MESSAGE,
                                                              buf_ptr,
                                                              client_msg_id,
                                                              wms_msg_id );
  #else
  /* Ask WMS to send the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_msg_send_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ));
 
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_send_func( i_ptr->client_info[ modem_id ].client_id,
                                                            #ifdef FEATURE_QCRIL_DSDS
                                                            as_id,
                                                            #endif /* FEATURE_QCRIL_DSDS */
                                                            qcril_sms_cmd_callback,
                                                            (void *) user_data,
                                                            WMS_SEND_MODE_CLIENT_MESSAGE,
                                                            buf_ptr );
  #endif /* FEATURE_QCRIL_IMS_EXT */

  qcril_free( buf_ptr );

} /* qcril_sms_request_cdma_send_sms() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_sms_acknowledge

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_sms_acknowledge
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  uint8 i;
  boolean success = FALSE;
  RIL_CDMA_SMS_Ack *ack_ptr;
  wms_ack_info_s_type *buf_ptr;
  char details[ 80 ];
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_ack_api_name = "wms_msg_ms_ack(CDMA)";
  #else
  char *wms_msg_ack_api_name = "wms_msg_ack(CDMA)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  ack_ptr = (RIL_CDMA_SMS_Ack *) params_ptr->data;
  QCRIL_ASSERT( ack_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );
  
  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /* Lookup the modem where the received SMS is pending to be acked */
  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    if ( i_ptr->sms_ack_info[ i ].cdma_ack_pending )
    {
      modem_id = i;
      break;
    }
  }

  if ( i >= QCRIL_MAX_MODEM_ID )
  {
    QCRIL_LOG_ERROR( "%s", "Unable to send ack; there are no outstanding messages to be acked.\n" );

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* WMS already acked this message or the client did not request an ack. Drop the ack. */
  if ( !i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_requested || !i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_needed)
  {
    if ( !i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_needed )
    {
      /* WMS already acked this message.  Drop the ack. */
      QCRIL_LOG_DEBUG( "%s", "Dropping CDMA Ack request since WMS already acked the message.\n" );
    }
    else if ( !i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_requested )
    {
      /* The client did not request an ack. */
      QCRIL_LOG_DEBUG( "%s", "Dropping CDMA Ack since client did not request an ack.\n" );
    }

    i_ptr->sms_ack_info[ modem_id ].cdma_ack_pending = FALSE;

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  /* Fill in the ack */
  buf_ptr = (wms_ack_info_s_type *) qcril_malloc( sizeof( wms_ack_info_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the CDMA SMS ACK to be sent" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );

    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  buf_ptr->transaction_id = i_ptr->sms_ack_info[ modem_id ].cdma_transaction_id;
  buf_ptr->message_mode = WMS_MESSAGE_MODE_CDMA;

  if ( ack_ptr->uErrorClass == RIL_CDMA_SMS_NO_ERROR )
  {
    buf_ptr->u.cdma.error_class = WMS_ERROR_NONE;
    success = TRUE;
  }
  else if ( ack_ptr->uErrorClass == RIL_CDMA_SMS_ERROR )
  {
    buf_ptr->u.cdma.error_class = WMS_ERROR_TEMP;
    success = FALSE;
  }

  buf_ptr->u.cdma.tl_status = ack_ptr->uSMSCauseCode;

  /* Request WMS to send the ack. */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "Token ID %d, success %d", qcril_log_get_token_id( params_ptr->t ), success );
  QCRIL_LOG_RPC2( modem_id, wms_msg_ack_api_name, details );
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_ack_func( i_ptr->client_info[ modem_id ].client_id,
                                                           #ifdef FEATURE_QCRIL_DSDS
                                                           as_id,
                                                           #endif /* FEATURE_QCRIL_DSDS */
                                                           qcril_sms_cmd_callback,
                                                           (void *) user_data,
                                                           buf_ptr );

  qcril_free( buf_ptr );

  /* Reset the ack pending flag.  If the ack is not received by the network, the network
     will resend the SMS, and RIL will have another chance to ack it. */
  i_ptr->sms_ack_info[ modem_id ].cdma_ack_pending = FALSE;

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

} /* qcril_sms_request_cdma_sms_acknowledge */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_write_sms_to_ruim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_write_sms_to_ruim
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  RIL_CDMA_SMS_WriteArgs *SMS_Write_info;
  uint32 i, user_data;
  wms_client_message_s_type *buf_ptr;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_write_api_name = "wms_msg_ms_write(RUIM)";
  #else
  char *wms_msg_write_api_name = "wms_msg_write(RUIM)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  SMS_Write_info = (RIL_CDMA_SMS_WriteArgs *) params_ptr->data;
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Invalid status */
  if ( SMS_Write_info->status  > 3 )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Always save and delete 1XEVDO SMS message to 1x modem */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_WRITE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Fill in the SMS message */
  buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the SMS message to be written to RUIM" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  buf_ptr->msg_hdr.message_mode = WMS_MESSAGE_MODE_CDMA;

  switch ( SMS_Write_info->status )
  {
    case 0:
      buf_ptr->msg_hdr.tag = WMS_TAG_MT_NOT_READ;
      buf_ptr->u.cdma_message.is_mo = FALSE;
      break;

    case 1:
      buf_ptr->msg_hdr.tag = WMS_TAG_MT_READ;
      buf_ptr->u.cdma_message.is_mo = FALSE;
      break;

    case 2:
      buf_ptr->msg_hdr.tag = WMS_TAG_MO_NOT_SENT;
      buf_ptr->u.cdma_message.is_mo = TRUE;
      break;

    case 3:
      buf_ptr->msg_hdr.tag = WMS_TAG_MO_SENT;
      buf_ptr->u.cdma_message.is_mo = TRUE;
      break;
  }

  buf_ptr->msg_hdr.mem_store = WMS_MEMORY_STORE_RUIM;
  buf_ptr->msg_hdr.index = WMS_DUMMY_MESSAGE_INDEX;

  buf_ptr->u.cdma_message.teleservice = SMS_Write_info->message.uTeleserviceID;
  buf_ptr->u.cdma_message.address.digit_mode = SMS_Write_info->message.sAddress.digit_mode;
  buf_ptr->u.cdma_message.address.number_mode = SMS_Write_info->message.sAddress.number_mode;
  buf_ptr->u.cdma_message.address.number_type = SMS_Write_info->message.sAddress.number_type;
  buf_ptr->u.cdma_message.address.number_plan = SMS_Write_info->message.sAddress.number_plan;
  buf_ptr->u.cdma_message.address.number_of_digits = SMS_Write_info->message.sAddress.number_of_digits;

  for ( i = 0; i < buf_ptr->u.cdma_message.address.number_of_digits; i++ )
  {
    buf_ptr->u.cdma_message.address.digits[ i ] = SMS_Write_info->message.sAddress.digits[ i ];
  }

  buf_ptr->u.cdma_message.subaddress.type = SMS_Write_info->message.sSubAddress.subaddressType;
  buf_ptr->u.cdma_message.subaddress.odd = SMS_Write_info->message.sSubAddress.odd;
  buf_ptr->u.cdma_message.subaddress.number_of_digits = SMS_Write_info->message.sSubAddress.number_of_digits;

  for ( i = 0; i < buf_ptr->u.cdma_message.subaddress.number_of_digits; i++ )
  {
    buf_ptr->u.cdma_message.subaddress.digits[ i ] = SMS_Write_info->message.sSubAddress.digits[i];
  }

  buf_ptr->u.cdma_message.is_tl_ack_requested = TRUE;
  buf_ptr->u.cdma_message.is_service_present = SMS_Write_info->message.bIsServicePresent;
  buf_ptr->u.cdma_message.service = SMS_Write_info->message.uServicecategory;

  buf_ptr->u.cdma_message.raw_ts.format = WMS_FORMAT_CDMA;
  buf_ptr->u.cdma_message.raw_ts.len = SMS_Write_info->message.uBearerDataLen;

  (void) memcpy( buf_ptr->u.cdma_message.raw_ts.data, SMS_Write_info->message.aBearerData, buf_ptr->u.cdma_message.raw_ts.len );

  /* Ask WMS to write the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_msg_write_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ) );
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_write_func( i_ptr->client_info[ modem_id ].client_id,
                                                             #ifdef FEATURE_QCRIL_DSDS
                                                             as_id,
                                                             #endif /* FEATURE_QCRIL_DSDS */
                                                             qcril_sms_cmd_callback,
                                                             (void *) user_data,
                                                             WMS_WRITE_MODE_INSERT,
                                                             buf_ptr );

  qcril_free( buf_ptr );

} /* qcril_sms_request_cdma_write_cdma_sms_to_ruim() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_delete_sms_on_ruim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_delete_sms_on_ruim
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_delete_api_name = "wms_msg_ms_delete(RUIM)";
  #else
  char *wms_msg_delete_api_name = "wms_msg_delete(RUIM)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Always save and delete 1XEVDO SMS message to 1x modem */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_DELETE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Ask WMS to delete the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "Token ID %d, index %d", 
                  qcril_log_get_token_id( params_ptr->t ), ( (int *) params_ptr->data)[ 0 ] );
  QCRIL_LOG_RPC2( modem_id, wms_msg_delete_api_name, details );

  /* Android uses the GSDI index, which starts at 1.  Subtract 1 to get the WMS index. */
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_delete_func( i_ptr->client_info[ modem_id ].client_id,
                                                              #ifdef FEATURE_QCRIL_DSDS
                                                              as_id, 
                                                              #endif /* FEATURE_QCRIL_DSDS */
                                                              qcril_sms_cmd_callback,
                                                              (void *) user_data,
                                                              WMS_MEMORY_STORE_RUIM,
                                                              ( (int *) params_ptr->data)[ 0 ] - 1 );

} /* qcril_sms_request_cdma_delete_sms_on_ruim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_get_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_get_broadcast_sms_config
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_modem_ids_list_type modem_ids_list;
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry; 
  qcril_request_resp_params_type resp;
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* OEM responsiblity to sync up 1XEVDO broadcast config for fusion. So read from one of the modem is sufficient */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_BC_MM_TABLE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Request WMS to get the service table for Broadcast SMS */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_SNPRINTF( details, sizeof( details ), "CDMA Token ID %d", qcril_log_get_token_id( params_ptr->t ) );
  QCRIL_LOG_RPC2( modem_id, "wms_bc_mm_get_table()", details );
  (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_get_table_func( i_ptr->client_info[ modem_id ].client_id,
                                                                   #ifdef FEATURE_QCRIL_DSDS
                                                                   as_id,
                                                                   #endif
                                                                   qcril_sms_cmd_callback,
                                                                   (void *) user_data,
                                                                   WMS_MESSAGE_MODE_CDMA );

} /* qcril_sms_request_cdma_get_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_set_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_set_broadcast_sms_config
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 i;
  uint8 bc_table_size;
  wms_bc_mm_service_info_s_type *buf_ptr;
  char details[ 80 ];
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */


  /* Check if the broadcast SMS table is too big */
  bc_table_size = params_ptr->datalen / sizeof( RIL_CDMA_BroadcastSmsConfigInfo * );
  if ( bc_table_size > QCRIL_SMS_BC_MM_TABLE_SIZE )
  {
    QCRIL_LOG_ERROR( "Size of CDMA Broadcast SMS Config table is too big : %d\n", bc_table_size );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_ids_list.modem_id[ 0 ], QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_BC_MM_ADD_SRVS, NULL, &reqlist_entry ); 
  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    reqlist_entry.state[ modem_id ] = QCRIL_REQ_AWAITING_CALLBACK;
    reqlist_entry.pending_event_id[ modem_id ] = QCRIL_EVT_SMS_BC_MM_ADD_SRVS;

  }

  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Fill in the Broadcast SMS configuration array */
  buf_ptr = ( wms_bc_mm_service_info_s_type * ) qcril_malloc( bc_table_size * sizeof( wms_bc_mm_service_info_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the Broadcast SMS Config table" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  for ( i = 0; i < bc_table_size; i++ )
  {
    /* Fill in from the ril.h interface */
    buf_ptr[ i ].srv_id.bc_srv_id.language = ( (RIL_CDMA_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->language;
    buf_ptr[ i ].srv_id.bc_srv_id.service = ( (RIL_CDMA_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->service_category;
    buf_ptr[ i ].selected = ( (RIL_CDMA_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->selected;

    /* Set reasonable default values */
    buf_ptr[ i ].priority = WMS_PRIORITY_NORMAL;
    buf_ptr[ i ].alert = WMS_BC_ALERT_DEFAULT;
    buf_ptr[ i ].max_messages = 20;
    buf_ptr[ i ].label[ 0 ] = 0;
    buf_ptr[ i ].label_encoding = WMS_ENCODING_GSM_7_BIT_DEFAULT;
  }

  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

    /* Request WMS to delete all services from the broadcast service table */
    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
    QCRIL_LOG_RPC( modem_id, "wms_bc_mm_delete_all_services()", "CDMA Token ID", qcril_log_get_token_id( params_ptr->t ));
    (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_delete_all_services_func( i_ptr->client_info[ modem_id ].client_id,
                                                                               #ifdef FEATURE_QCRIL_DSDS
                                                                               as_id,
                                                                               #endif
                                                                               qcril_sms_cmd_callback,
                                                                               (void *) user_data,
                                                                               WMS_MESSAGE_MODE_CDMA );


    /* Request WMS to add services to the broadcast service table */
    QCRIL_SNPRINTF( details, sizeof( details ), "CDMA Token ID %d", qcril_log_get_token_id( params_ptr->t ) );
    QCRIL_LOG_RPC2( modem_id, "wms_bc_mm_add_services()", details );
    (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_add_services_func( i_ptr->client_info[ modem_id ].client_id,
                                                                        #ifdef FEATURE_QCRIL_DSDS
                                                                        as_id,
                                                                        #endif
                                                                        qcril_sms_cmd_callback,
                                                                        (void *) user_data,
                                                                        WMS_MESSAGE_MODE_CDMA,
                                                                        bc_table_size,
                                                                        buf_ptr );
  }

  qcril_free( buf_ptr );

} /* qcril_sms_request_cdma_set_broadcast_sms_config */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_sms_broadcast_activation

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_sms_broadcast_activation
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_modem_ids_list_type modem_ids_list;
  uint8 i;
  wms_bc_pref_e_type bc_pref = WMS_BC_PREF_MAX;
  char *bc_pref_name = "";
  char details[ 80 ];
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */
  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );
  
  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_ids_list.modem_id[ 0 ], QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_BC_MM_PREF, NULL, &reqlist_entry ); 

  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    reqlist_entry.state[ modem_id ] = QCRIL_REQ_AWAITING_CALLBACK;
    reqlist_entry.pending_event_id[ modem_id ] = QCRIL_EVT_SMS_BC_MM_PREF;
  }

  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if ( ( (int *) params_ptr->data)[ 0 ] == 0 )
  {
    bc_pref = WMS_BC_PREF_ACTIVATE_TABLE;
    bc_pref_name = "Activate";
  }
  else if ( ( (int *) params_ptr->data)[ 0 ] == 1 )
  {
    bc_pref = WMS_BC_PREF_DEACTIVATE;
    bc_pref_name = "Deactivate";
  }

  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

    /* Request WMS to set the BC preference */
    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
    QCRIL_SNPRINTF( details, sizeof( details ), "CDMA %s Token ID %d", bc_pref_name, qcril_log_get_token_id( params_ptr->t ) );
    QCRIL_LOG_RPC2( modem_id, "wms_bc_mm_set_pref()", details );
    (void) qcril_sms_api_funcs[ modem_id ].wms_bc_mm_set_pref_func( i_ptr->client_info[ modem_id ].client_id,
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    as_id,
                                                                    #endif
                                                                    qcril_sms_cmd_callback,
                                                                    (void *) user_data,
                                                                    WMS_MESSAGE_MODE_CDMA,
                                                                    bc_pref );
  }

} /* qcril_sms_request_cdma_sms_broadcast_activation() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_registration_state

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_IMS_REGISTRATION_STATE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_registration_state
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  char details[ 80 ];
  int ims_registration_state[ 2 ] = { FALSE, RADIO_TECH_3GPP };
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );
  
  /*-----------------------------------------------------------------------*/

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_IMS_REG, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = &ims_registration_state;
    resp.resp_len = sizeof( ims_registration_state );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem is configured for LTE/EVDO */
  modem_id = modem_ids_list.modem_id[ 0 ];

  #ifdef FEATURE_QCRIL_IMS
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].transport_reg_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->transport_reg_info_mutex, details );

  if ( i_ptr->transport_reg_info[ modem_id ].is_registered )
  {
    ims_registration_state[ 0 ] = TRUE;
    if ( i_ptr->transport_reg_info[ modem_id ].transport_cap == WMS_MSG_TRANSPORT_CAP_3GPP_IMS )
    {
      ims_registration_state[ 1 ] = RADIO_TECH_3GPP;
    }
    else
    {
      ims_registration_state[ 1 ] = RADIO_TECH_3GPP2;
    }

    QCRIL_LOG_DEBUG( "IMS registered, transport_cap=%d\n", ims_registration_state[ 1 ] );
  }
  else
  {
    QCRIL_LOG_DEBUG( "%s\n", "IMS not registered" );
  }

  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );
  #endif /* FEATURE_QCRIL_IMS */

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  resp.resp_pkt = ims_registration_state;
  resp.resp_len = sizeof( ims_registration_state );
  qcril_send_request_response( &resp );

} /* qcril_sms_request_ims_registration_state() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_IMS_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_send_sms
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  qcril_modem_ids_list_type modem_ids_list;
  uint32 user_data;
  RIL_IMS_SMS_Message *ims_sms_msg;
  boolean sms_payload_in_cdma_format;
  RIL_CDMA_SMS_Message *cdma_sms_msg = NULL;
  const char *smsc_address = NULL;
  const char *gw_pdu = NULL;   
  wms_client_message_s_type *buf_ptr = NULL;
  wms_status_e_type decode_status;
  wms_client_ts_data_s_type *ts_data_ptr;
  qcril_reqlist_public_type reqlist_entry; 
  qcril_reqlist_u_type u_info;
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_send_api_name = "wms_msg_ms_send(IMS)";
  #else
  char *wms_msg_send_api_name = "wms_msg_send(IMS)";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  ims_sms_msg = (RIL_IMS_SMS_Message *) params_ptr->data;
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( ims_sms_msg->tech == RADIO_TECH_3GPP2 )
  {
    sms_payload_in_cdma_format = TRUE;
    cdma_sms_msg = ims_sms_msg->message.cdmaMessage;
  }
  else
  {
    sms_payload_in_cdma_format = FALSE;
    smsc_address = ( (const char **) ims_sms_msg->message.gsmMessage )[ 0 ];
    gw_pdu = ( (const char **) ims_sms_msg->message.gsmMessage )[ 1 ];   

    if ( !qcril_mo_sms_error_check( instance_id, gw_pdu, params_ptr->t, params_ptr->event_id ) )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      return;
    }
  }

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  
  #endif /* FEATURE_QCRIL_DSDS */

  /* Lookup modem to service the request */
  if ( qcril_arb_query_sms_srv_modem_id( QCRIL_ARB_SMS_SRV_CAT_IMS, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SMS_SEND_FAIL_RETRY, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem is configured to send 1X/EVDO SMS */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /* Add entry to ReqList. */
  u_info.sms.valid_client_msg_ref = FALSE;
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_SMS_SEND, &u_info, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList*/
    return;
  }

  /* Fill in the message header */
  buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );

  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the IMS SMS message to be sent" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Fill in the message */
  qcril_sms_fill_mo_wms_payload( TRUE, sms_payload_in_cdma_format, cdma_sms_msg, smsc_address, gw_pdu, buf_ptr );

  /* In case of 3GPP2 format, save the client's message reference for possible retry over 1x scenario */
  if ( ims_sms_msg->tech == RADIO_TECH_3GPP2 )
  {
    ts_data_ptr = (wms_client_ts_data_s_type *) qcril_malloc( sizeof( wms_client_ts_data_s_type ) );

    if ( ts_data_ptr == NULL )
    {
      QCRIL_LOG_ERROR( "%s\n", "No buffer to convert client ts data" ); 
    }
    else if ( ( decode_status = wms_ts_decode( &buf_ptr->u.cdma_message.raw_ts, ts_data_ptr ) ) == WMS_OK_S )
    {
      u_info.sms.valid_client_msg_ref = TRUE;
      u_info.sms.client_msg_ref = ts_data_ptr->u.cdma.message_id.id_number;
      QCRIL_LOG_DEBUG( "SMS over IMS, client msg ref %d\n", u_info.sms.client_msg_ref );

      /* Store the client message reference in the reqlist.  It will be retrieved when we get the submit report. */
      qcril_reqlist_update_sub_info( instance_id, params_ptr->t, &u_info );

      qcril_free( ts_data_ptr );
    }
  }

  /* Ask WMS to send the message */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );
  QCRIL_LOG_RPC( modem_id, wms_msg_send_api_name, "Token ID", qcril_log_get_token_id( params_ptr->t ));
  (void) qcril_sms_api_funcs[ modem_id ].wms_msg_send_func( i_ptr->client_info[ modem_id ].client_id,
                                                            #ifdef FEATURE_QCRIL_DSDS
                                                            as_id,
                                                            #endif /* FEATURE_QCRIL_DSDS */
                                                            qcril_sms_cmd_callback,
                                                            (void *) user_data,
                                                            WMS_SEND_MODE_CLIENT_MESSAGE,
                                                            buf_ptr );

  qcril_free( buf_ptr );

} /* qcril_sms_request_ims_send_sms() */


/*=========================================================================
  FUNCTION:  qcril_sms_command_event_callback

===========================================================================*/
/*!
    @brief
    Handle an SMS command callback event.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_command_event_callback
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_command_callback_params_type *callback_params_ptr;
  qcril_reqlist_public_type req_info;
  qcril_modem_ids_list_type modem_ids_done_list;
  RIL_Errno failure_cause;
  qcril_request_resp_params_type resp;
  IxErrnoType result;

  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  callback_params_ptr = (qcril_sms_command_callback_params_type *) params_ptr->data;

  switch ( callback_params_ptr->command )
  {
    case WMS_CMD_MSG_SEND:
    case WMS_CMD_MSG_ACK:
    case WMS_CMD_MSG_WRITE:
    case WMS_CMD_MSG_DELETE:
    case WMS_CMD_BC_MM_SET_PREF:
    case WMS_CMD_BC_MM_DELETE_ALL_SERVICES:
    case WMS_CMD_BC_MM_ADD_SRV:
    case WMS_CMD_BC_MM_GET_TABLE:
    case WMS_CMD_CFG_SET_MEMORY_FULL:
    case WMS_CMD_CFG_GET_MESSAGE_LIST:
    case WMS_CMD_MSG_READ_TEMPLATE:
    case WMS_CMD_MSG_WRITE_TEMPLATE:
      /* Look for the command in the ReqList */
      if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
      {
        if ( callback_params_ptr->error != WMS_CMD_ERR_NONE )
        {
          /* The command failed */
          QCRIL_LOG_ERROR( "%s callback failed.  Token: %d, Error %d\n", qcril_sms_lookup_cmd_name( callback_params_ptr->command ),
                           qcril_log_get_token_id( params_ptr->t ), callback_params_ptr->error );

          failure_cause = RIL_E_GENERIC_FAILURE;

          /* Check if the error is due to FDN check failure on WMS */
          if ( WMS_CMD_ERR_MSG_BLOCKED == callback_params_ptr->error )
          {
            failure_cause = RIL_E_FDN_CHECK_FAILURE;
          }
          else if ( ( callback_params_ptr->command == WMS_CMD_MSG_SEND ) &&
                    ( ( callback_params_ptr->error == WMS_CMD_ERR_NO_RESOURCE ) ||
                      ( callback_params_ptr->error == WMS_CMD_ERR_BUSY ) ||
                      ( callback_params_ptr->error == WMS_CMD_ERR_MSG_NO_SC_ADDRESS ) ) )
          {
            failure_cause = RIL_E_SMS_SEND_FAIL_RETRY;
          }

          /* Send response and remove the entry from ReqList */
          qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, failure_cause, &resp );
          qcril_send_request_response( &resp );
          QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);
          return;
        }
        /* Force state transition to WaitingMoreAMSSEvents if there are pending AMSS events */
        else if ( req_info.pending_event_id[ modem_id ] != QCRIL_EVT_NONE )
        {
          qcril_sms_force_state_transition_to_wait_amss_event( instance_id, modem_id, params_ptr->t, req_info.request, 
                                                               req_info.pending_event_id[ modem_id ] );
        }
        else
        {
          /* Received all AMSS events */
          if ( qcril_reqlist_complete_all_amss_events( instance_id, modem_id, params_ptr->t, QCRIL_REQ_COMPLETED_SUCCESS,
                                                       &modem_ids_done_list, &result ) == E_SUCCESS )
          {
            if ( result == E_SUCCESS )
            {
              /* Send RIL_E_SUCCESS response */
              qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
              qcril_send_request_response( &resp );
            }
            else
            {
              /* Send response and remove the entry from ReqList */
              qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
              qcril_send_request_response( &resp );
            }
          }
        }
      }
      else
      {
        /* Entry not found in the ReqList */
        QCRIL_LOG_ERROR( "ReqList entry not found for SMS command: %s, Token ID %d\n",
                         qcril_sms_lookup_cmd_name( callback_params_ptr->command ), qcril_log_get_token_id( params_ptr->t ) ); 
      }
      break;

  default:
    if ( callback_params_ptr->error != WMS_CMD_ERR_NONE )
    {
      QCRIL_LOG_ERROR( "SMS command callback failed: %s\n", qcril_sms_lookup_cmd_name( callback_params_ptr->command ) );
    }
    break;
  } /* end switch */
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_sms_command_event_callback */


/*=========================================================================
  FUNCTION:  qcril_sms_msg_event_send

===========================================================================*/
/*!
    @brief
    Handle WMS_MSG_EVENT_SEND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_send
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  wms_msg_event_info_s_type *event_ptr; 
  qcril_reqlist_public_type req_info;
  wms_status_e_type decode_status = WMS_STATUS_MAX;
  wms_client_ts_data_s_type *buf_ptr;
  qcril_reqlist_u_type u_info;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
  {
    buf_ptr = (wms_client_ts_data_s_type *) qcril_malloc( sizeof( wms_client_ts_data_s_type ) );

    if ( buf_ptr == NULL )
    {
      QCRIL_LOG_ERROR( "%s\n", "No buffer to convert client ts data" ); 
      return;
    }

    if ( event_ptr->status_info.message.msg_hdr.message_mode == WMS_MESSAGE_MODE_CDMA )
    {
      /* Retrieve the message reference */
      if ( ( decode_status = wms_ts_decode( &event_ptr->status_info.message.u.cdma_message.raw_ts, buf_ptr ) ) == WMS_OK_S )
      {
        req_info.sub.sms.wms_msg_ref = buf_ptr->u.cdma.message_id.id_number;
      }
    }
    else if ( event_ptr->status_info.message.msg_hdr.message_mode == WMS_MESSAGE_MODE_GW )
    {
      if ( ( decode_status = wms_ts_decode( &event_ptr->status_info.message.u.gw_message.raw_ts_data, buf_ptr ) ) == WMS_OK_S )
      {
        req_info.sub.sms.wms_msg_ref = buf_ptr->u.gw_pp.u.submit.message_reference;
      }
    }

    qcril_free( buf_ptr );

    if ( decode_status == WMS_OK_S )
    {
      QCRIL_LOG_DEBUG( "WMS_MSG_EVENT_SEND: wms msg ref %d\n", req_info.sub.sms.wms_msg_ref );

      /* Store the message reference in the reqlist.  It will be retrieved when we
         get the submit report. */
      qcril_reqlist_update_sub_info( instance_id, params_ptr->t, &req_info.sub );

      /* When sending an SMS, we wait for a submit report.
         If sending the SMS fails, the submit report will be received at the end
         of the retry period.  Otherwise, the submit report will be received when
         the message is received. */
      qcril_reqlist_update_pending_event_id( instance_id, modem_id, params_ptr->t, QCRIL_EVT_SMS_SUBMIT_RPT );
    }
    else
    {
      /* Failed to get the message reference.  Return failure to RIL and delete
         the entry from the reqlist. */
      QCRIL_LOG_ERROR( "%s", "Failed to decode the message reference in the WMS_MSG_EVENT_SEND event.\n" );
      qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "ReqList entry not found for WMS_MSG_EVENT_SEND, Token ID %d\n", qcril_log_get_token_id( params_ptr->t ) ); 
  }

} /* qcril_sms_msg_event_send */


/*=========================================================================
  FUNCTION:  qcril_sms_msg_event_submit_report

===========================================================================*/
/*!
    @brief
    Handle WMS_MSG_EVENT_SUBMIT_REPORT.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_submit_report
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  qcril_reqlist_public_type req_info;
  wms_msg_event_info_s_type *event_ptr; 
  RIL_SMS_Response send_msg_response;
  wms_raw_ts_data_s_type raw_ts_data;
  char *buf_ptr = NULL;
  qcril_request_resp_params_type resp;
  char details[ 80 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
  {
    /* Use the message reference obtained from the WMS_MSG_EVENT_SEND event. */
    send_msg_response.messageRef = req_info.sub.sms.wms_msg_ref;
    send_msg_response.ackPDU = NULL;

    /* Initialize the errorCode to -1, which means there is no error code info */
    send_msg_response.errorCode = -1;

    QCRIL_LOG_DEBUG( "EVENT_SUBMIT_REPORT: msg mode %d rpt status %d cause value %d\n",
                     event_ptr->submit_report_info.message_mode,
                     event_ptr->submit_report_info.report_status, 
                     event_ptr->submit_report_info.cause_info.cause_value ); 

    /* Delivery of the SMS Message failed at some point */
    if ( event_ptr->submit_report_info.report_status != WMS_RPT_OK )
    {
      if ( ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_GW ) && 
           ( event_ptr->submit_report_info.report_status == WMS_RPT_RP_ERROR ) )
      {
        send_msg_response.errorCode = event_ptr->submit_report_info.cause_info.cause_value;
      }
      else if ( ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_CDMA ) && 
                ( event_ptr->submit_report_info.report_status == WMS_RPT_CDMA_TL_ERROR ) )
      {
        send_msg_response.errorCode = event_ptr->submit_report_info.cause_info.tl_status;
      }

      if ( ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_GW ) && 
           ( event_ptr->submit_report_info.report_status == WMS_RPT_RP_ERROR ) && 
           ( event_ptr->submit_report_info.cause_info.cause_value == WMS_RP_CAUSE_TEMPORARY_FAILURE ) )
      {
        /* It was a temporary error in GW.  Indicate retry and remove the entry from ReqList. */
        qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SMS_SEND_FAIL_RETRY, &resp );
        resp.resp_pkt = &send_msg_response;
        resp.resp_len = sizeof( send_msg_response );
        qcril_send_request_response( &resp );
      }
      else if ( ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_CDMA ) && 
                ( ( event_ptr->submit_report_info.report_status == WMS_RPT_NETWORK_NOT_READY ) ||
                  ( event_ptr->submit_report_info.report_status == WMS_RPT_NO_ACK ) || 
                  ( ( event_ptr->submit_report_info.report_status == WMS_RPT_CDMA_TL_ERROR ) && 
                    ( event_ptr->submit_report_info.cause_info.error_class == WMS_ERROR_TEMP ) ) ) )
      {
        /* It was a temporary error in CDMA.  Indicate retry and remove the entry from ReqList. */
        qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SMS_SEND_FAIL_RETRY, &resp );
        resp.resp_pkt = &send_msg_response;
        resp.resp_len = sizeof( send_msg_response );
        qcril_send_request_response( &resp );
      }
      #ifdef FEATURE_QCRIL_IMS
      else if ( ( req_info.request == RIL_REQUEST_IMS_SEND_SMS ) &&
                ( ( ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_CDMA ) && 
                    ( ( event_ptr->submit_report_info.report_status == WMS_RPT_SIP_PERM_ERROR ) ||  
                      ( event_ptr->submit_report_info.report_status == WMS_RPT_SIP_TEMP_ERROR ) ) ) ||
                  ( ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_GW ) && 
                    ( event_ptr->submit_report_info.report_status == WMS_RPT_LL_ERROR ) ) ) ) 
      {
        /* Update the message reference info about SMS over IMS retry (3GPP2) */
        if ( req_info.sub.sms.valid_client_msg_ref )
        {
          QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].transport_reg_info_mutex", instance_id );
          QCRIL_MUTEX_LOCK( &i_ptr->transport_reg_info_mutex, details );

          i_ptr->ims_client_msg_ref = req_info.sub.sms.client_msg_ref; 
          i_ptr->ims_wms_msg_ref = req_info.sub.sms.wms_msg_ref; 

          QCRIL_LOG_DEBUG( "Update SMS over IMS retry info, client msg id =%d, wms msg id = %d\n", 
                           i_ptr->ims_client_msg_ref, i_ptr->ims_wms_msg_ref ); 

          QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );
        }

        /* It was an error in IMS. Indicate retry and remove the entry from ReqList. */
        qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SMS_SEND_FAIL_RETRY, &resp );
        resp.resp_pkt = &send_msg_response;
        resp.resp_len = sizeof( send_msg_response );
        qcril_send_request_response( &resp );
      }
      #endif /* FEATURE_QCRIL_IMS */
      else
      {
        /* Send RIL_E_GENERIC_FAILURE response and remove the entry from ReqList */
        qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
        resp.resp_pkt = &send_msg_response;
        resp.resp_len = sizeof( send_msg_response );
        qcril_send_request_response( &resp );
      }
    }
    /* The SMS Message was successfully sent. */
    else
    {
      if ( event_ptr->submit_report_info.message_mode == WMS_MESSAGE_MODE_GW )
      {
        /* Encode the Submit Report Ack and translate to ASCII hex char */
        if ( wms_ts_encode_submit_report_ack( &event_ptr->submit_report_info.u.ack, &raw_ts_data ) == WMS_OK_S )
        {
          buf_ptr = qcril_malloc( QCRIL_SMS_BUF_MAX_SIZE );
          if ( buf_ptr == NULL )
          {
            QCRIL_LOG_ERROR( "%s\n", "Failed to allocate buffer to encode submit report ack. Sending NULL." );
          }
          else
          {
            qcril_sms_byte_to_hex( &raw_ts_data.data[ 0 ], buf_ptr, raw_ts_data.len ); 
            send_msg_response.ackPDU = buf_ptr;
          }
        }
        else
        {
          QCRIL_LOG_ERROR( "%s", "Failed to encode submit report ack.  Sending NULL.\n");
        }
      }

      /* For CDMA, the ackPDU is NULL.  The ackPDU in CDMA (defined in 3.4.2.3 of IS637-B)
         consists of the cause codes (3.4.3.6), which are sent in the errorCode field. */

      /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
      qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = &send_msg_response; 
      resp.resp_len = sizeof( send_msg_response );
      qcril_send_request_response( &resp );

      if ( buf_ptr != NULL )
      {
        qcril_free( buf_ptr );
      }
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "ReqList entry not found for WMS_MSG_EVENT_SUBMIT_REPORT, Token ID %d\n", qcril_log_get_token_id( params_ptr->t ) ); 
  }

} /* qcril_sms_msg_event_submit_report */


/*=========================================================================
  FUNCTION:  qcril_sms_msg_event_write

===========================================================================*/
/*!
    @brief
    Handle WMS_MSG_EVENT_WRITE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_write
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_reqlist_public_type req_info;
  wms_msg_event_info_s_type *event_ptr; 
  int index_on_sim;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
  {
    /* Android uses the GSDI index, since it calls RIL_REQUEST_SIM_IO.
       The GSDI index is the WMS index plus 1. */
    index_on_sim = event_ptr->status_info.message.msg_hdr.index + 1;

    /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. Return the index where the message is stored. */
    qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = &index_on_sim;
    resp.resp_len = sizeof( int );
    qcril_send_request_response( &resp );
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "ReqList entry not found for WMS_MSG_EVENT_WRITE, Token ID %d\n", qcril_log_get_token_id( params_ptr->t ) ); 
  }

} /* qcril_sms_msg_event_write */


/*=========================================================================
  FUNCTION:  qcril_sms_msg_event_delete

===========================================================================*/
/*!
    @brief
    Handle WMS_MSG_EVENT_DELETE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_delete
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
  {
    /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
    qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "ReqList entry not found for WMS_MSG_EVENT_DELETE, Token ID %d\n", qcril_log_get_token_id( params_ptr->t ) ); 
  }

}; /* qcril_sms_msg_event_delete */


/*===========================================================================

  FUNCTION:  qcril_sms_msg_event_read_template

===========================================================================*/
/*!
    @brief
    Handles WMS_MSG_EVENT_READ_TEMPLATE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_read_template
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  wms_msg_event_info_s_type *event_ptr; 
  qcril_reqlist_public_type req_info;
  char smsc_address[ WMS_ADDRESS_MAX ];
  wms_client_message_s_type *buf_ptr;
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_write_template_api_name = "wms_msg_ms_write_template()";
  #else
  char *wms_msg_write_template_api_name = "wms_msg_write_template()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr != NULL);
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
  {
    if ( ( event_ptr->status_info.message.u.gw_template.mask | WMS_GW_TEMPLATE_MASK_SC_ADDR ) != 0 )
    {
      if ( req_info.request == RIL_REQUEST_GET_SMSC_ADDRESS )
      {
        /* Convert the address from WMS's format to a char * */
        if ( qcril_sms_convert_smsc_address_to_ril_format( &event_ptr->status_info.message.u.gw_template.sc_addr, smsc_address ) )
        {
          /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
          qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
          resp.resp_pkt = smsc_address;
          resp.resp_len = strlen( smsc_address );
          qcril_send_request_response( &resp );
        }
        else
        {
          qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
        }
      }
      else if ( req_info.request == RIL_REQUEST_SET_SMSC_ADDRESS )
      {
        /* We have found a template which contains an SMSC address.  Replace it
           with the new SMSC address. */

        /*-----------------------------------------------------------------------*/

        /* Update the pending event id to wait for the read template event */
        qcril_reqlist_update_pending_event_id( instance_id, modem_id, req_info.t, QCRIL_EVT_SMS_WRITE_TEMPLATE );

        /*-----------------------------------------------------------------------*/

        buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );
        if ( buf_ptr == NULL )
        {
          QCRIL_LOG_ERROR( "%s\n", "No buffer to compose GW templatei to write");
          qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          return;
        }

        /* Copy the existing template into the new one, and then replace the SMSC Address. */
        memcpy( buf_ptr, &event_ptr->status_info.message, sizeof( wms_client_message_s_type ) );

        /* Use the SMSC address stored when RIL_REQUEST_SET_SMSC_ADDRESS was processed */
        buf_ptr->u.gw_template.sc_addr = i_ptr->smsc_address;

        #ifdef FEATURE_QCRIL_DSDS
        /* Lookup as_id */
        if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
        {
          qcril_free( buf_ptr );
          QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
          qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          return;
        }  
        #endif /* FEATURE_QCRIL_DSDS */

        /* Ask WMS to write the template */
        user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, req_info.req_id );
        QCRIL_LOG_RPC( modem_id, wms_msg_write_template_api_name, "Token ID", qcril_log_get_token_id( req_info.t ) );
        (void) qcril_sms_api_funcs[ modem_id ].wms_msg_write_template_func( i_ptr->client_info[ modem_id ].client_id,
                                                                            #ifdef FEATURE_QCRIL_DSDS
                                                                            as_id,
                                                                            #endif /* FEATURE_QCRIL_DSDS */
                                                                            qcril_sms_cmd_callback,
                                                                            (void *) user_data,
                                                                            WMS_WRITE_MODE_REPLACE,
                                                                            buf_ptr );

        qcril_free( buf_ptr );
      }
    }
    else
    {
      /* There was no SMSC address.  Return failure. */
      QCRIL_LOG_ERROR( "%s", "There are no SMSC addresses stored on the SIM.\n");
      qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "ReqList entry not found for WMS_MSG_EVENT_READ_TEMPLATE, Token ID %d\n",
                     qcril_log_get_token_id( params_ptr->t ) ); 
  }

} /* qcril_sms_msg_event_read_template() */


/*===========================================================================

  FUNCTION:  qcril_sms_msg_event_write_template

===========================================================================*/
/*!
    @brief
    Handles WMS_MSG_EVENT_WRITE_TEMPLATE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_write_template
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &req_info ) == E_SUCCESS )
  {
    /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
    qcril_default_request_resp_params( instance_id, params_ptr->t, req_info.request, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "ReqList entry not found for WMS_MSG_EVENT_WRITE_TEMPLATE, Token ID %d\n", qcril_log_get_token_id( params_ptr->t ) ); 
  }

} /* qcril_sms_msg_event_write_template() */


/*===========================================================================

  FUNCTION:  qcril_sms_msg_event_transport_reg

===========================================================================*/
/*!
    @brief
    Handles WMS_MSG_EVENT_TRANSPORT_REG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_transport_reg
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  wms_msg_event_info_s_type *event_ptr; 
  qcril_unsol_resp_params_type unsol_resp;
  char details[ 80 ];
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type voice_radio_tech;
  #else
  qcril_modem_id_e_type modem_id_temp;
  qcril_radio_tech_e_type voice_radio_tech;
  #endif /* FEATURE_ICS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  /* SMS over IMS is supported only if RIL_QCOM_VERSION is defined and only
   * for targets for which FEATURE_QCRIL_IMS is defined */
  #ifdef RIL_QCOM_VERSION
  #ifdef FEATURE_QCRIL_IMS
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].transport_reg_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->transport_reg_info_mutex, details );

  i_ptr->transport_reg_info[ modem_id ] = event_ptr->transport_reg_info; 

  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );

  /* Notify RIL we received a class 2 message (new SMS on SIM) */
  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, &unsol_resp );
  qcril_send_unsol_response( &unsol_resp );

  if ( qcril_arb_voip_is_enabled() )
  {      
    /* Notify Android for the Voice Radio Tech Change */
    qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, &unsol_resp );
    #ifdef FEATURE_ICS
    qcril_arb_query_voice_srv_modem_id( instance_id, &modem_id_temp, &voice_radio_tech );
    unsol_resp.resp_pkt = (void *) &voice_radio_tech;
    unsol_resp.resp_len = sizeof( voice_radio_tech );
    #endif
    qcril_send_unsol_response( &unsol_resp );

    /* Notify Android for the Network State Change */
    qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, &unsol_resp );
    qcril_send_unsol_response( &unsol_resp );
  }

  #endif /* FEATURE_QCRIL_IMS */
  #endif /* RIL_QCOM_VERSION */

} /* qcril_sms_msg_event_transport_reg() */


/*===========================================================================

  FUNCTION:  qcril_sms_cfg_event_message_list

===========================================================================*/
/*!
    @brief
    Handles WMS_CFG_EVENT_MESSAGE_LIST.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_cfg_event_message_list
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  wms_cfg_event_info_s_type *cfg_info_ptr; 
  wms_client_message_s_type *buf_ptr;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *wms_msg_write_template_api_name = "wms_msg_ms_write_template()";
  char *wms_msg_read_template_api_name = "wms_msg_ms_read_template()";
  #else
  char *wms_msg_write_template_api_name = "wms_msg_write_template()";
  char *wms_msg_read_template_api_name = "wms_msg_read_template()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  cfg_info_ptr = ( wms_cfg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( cfg_info_ptr != NULL);
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query_by_event( instance_id, modem_id, QCRIL_EVT_SMS_CFG_MESSAGE_LIST, &req_info ) != E_SUCCESS )
  {
    /* Entry not found in the ReqList */
    return;
  }

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore request" );
    qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }  

  #endif /* FEATURE_QCRIL_DSDS */

  if ( cfg_info_ptr->message_list.len == 0 )
  {
    /* There are no templates (and hence no SMSC Addresses) stored on the SIM. */

    if ( req_info.request == RIL_REQUEST_GET_SMSC_ADDRESS )
    {
      /* There are no templates on the SIM, so the request to get the SMSC address failed. */
      QCRIL_LOG_ERROR( "%s", "There are no SMSC addresses stored on the SIM");
      qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
    else if ( req_info.request == RIL_REQUEST_SET_SMSC_ADDRESS )
    {
      /* There are no templates on the SIM, so write the SMSC address to index 0. */

      /*-----------------------------------------------------------------------*/

      /* Update the pending event id to wait for the write template event */
      qcril_reqlist_update_pending_event_id( instance_id, modem_id, req_info.t, QCRIL_EVT_SMS_WRITE_TEMPLATE );

      /*-----------------------------------------------------------------------*/

      buf_ptr = (wms_client_message_s_type *) qcril_malloc( sizeof( wms_client_message_s_type ) );
      if ( buf_ptr == NULL )
      {
        QCRIL_LOG_ERROR( "%s\n", "No buffer to compose GW template to write");
        qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
        return;
      }

      buf_ptr->msg_hdr.message_mode = WMS_MESSAGE_MODE_GW;
      buf_ptr->msg_hdr.tag = WMS_TAG_MO_TEMPLATE;
      buf_ptr->msg_hdr.mem_store = WMS_MEMORY_STORE_SIM;
      buf_ptr->msg_hdr.index = 0;

      buf_ptr->u.gw_template.alpha_id.len = 0;
      buf_ptr->u.gw_template.mask = WMS_GW_TEMPLATE_MASK_SC_ADDR;

      /* Use the SMSC address stored when RIL_REQUEST_SET_SMSC_ADDRESS was processed */
      buf_ptr->u.gw_template.sc_addr = i_ptr->smsc_address;

      /* Ask WMS to write the template */
      user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, req_info.req_id );
      QCRIL_LOG_RPC( modem_id, wms_msg_write_template_api_name, "Token ID", qcril_log_get_token_id( req_info.t ) );
      (void) qcril_sms_api_funcs[ modem_id ].wms_msg_write_template_func( i_ptr->client_info[ modem_id ].client_id,
                                                                          #ifdef FEATURE_QCRIL_DSDS
                                                                          as_id,
                                                                          #endif /* FEATURE_QCRIL_DSDS */
                                                                          qcril_sms_cmd_callback,
                                                                          (void *) user_data,
                                                                          WMS_WRITE_MODE_INSERT,
                                                                          buf_ptr );

      qcril_free( buf_ptr );
    }
  }
  else
  {
    /* There are templates stored on the SIM.  Read the first template to see if it 
       contains an SMSC address (unfortunately there's no official way to tell 
       which template might contain an SMSC address). */

    /*-----------------------------------------------------------------------*/

    /* Update the pending event id to wait for the read template event */
    qcril_reqlist_update_pending_event_id( instance_id, modem_id, req_info.t, QCRIL_EVT_SMS_READ_TEMPLATE );

    /*-----------------------------------------------------------------------*/

    /* Ask WMS to read the template */
    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, req_info.req_id );
    QCRIL_LOG_RPC( modem_id, wms_msg_read_template_api_name, "Token ID", qcril_log_get_token_id( req_info.t ) );
    (void) qcril_sms_api_funcs[ modem_id ].wms_msg_read_template_func( i_ptr->client_info[ modem_id ].client_id,
                                                                       #ifdef FEATURE_QCRIL_DSDS
                                                                       as_id,
                                                                       #endif /* FEATURE_QCRIL_DSDS */
                                                                       qcril_sms_cmd_callback,
                                                                       (void *) user_data,
                                                                       WMS_MEMORY_STORE_SIM,
                                                                       cfg_info_ptr->message_list.indices[ 0 ] );
  }

} /* qcril_sms_cfg_event_message_list() */


/*=========================================================================
  FUNCTION:  qcril_sms_cfg_event_mem_status_set

===========================================================================*/
/*!
    @brief
    Handle WMS_CFG_EVENT_MEMORY_STATUS_SET.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_cfg_event_mem_status_set
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_done_list;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  IxErrnoType result;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query_by_event( instance_id, modem_id, 
                                     #ifdef FEATURE_QCRIL_DSDS
                                     QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET
                                     #else
                                     QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET
                                     #endif /* FEATURE_QCRIL_DSDS */
                                     , &req_info ) == E_SUCCESS )
  {
    if ( qcril_reqlist_complete_all_amss_events( instance_id, modem_id, req_info.t, QCRIL_REQ_COMPLETED_SUCCESS, &modem_ids_done_list, 
                                                 &result ) == E_SUCCESS )
    {  
      if ( result == E_SUCCESS )
      {
        qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        /* Send response and remove the entry from ReqList */
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "Waiting for more WMS_CFG_EVENT_MEMORY_STATUS, already received from modem id %d\n", modem_id ); 
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "%s", "ReqList entry not found for WMS_CFG_EVENT_MEMORY_STATUS_SET\n" ); 
  }

}; /* qcril_sms_cfg_event_mem_status_set */


/*=========================================================================
  FUNCTION:  qcril_sms_cfg_event_memory_full

===========================================================================*/
/*!
    @brief
    Handle WMS_CFG_EVENT_MEMORY_FULL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_cfg_event_memory_full
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  const wms_cfg_event_info_s_type *cfg_info_ptr; 
  qcril_unsol_resp_params_type unsol_resp;
  int response_id;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  cfg_info_ptr = ( wms_cfg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( cfg_info_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( ( cfg_info_ptr->mem_store == WMS_MEMORY_STORE_SIM ) ||
       ( cfg_info_ptr->mem_store == WMS_MEMORY_STORE_RUIM ) )
  {
    QCRIL_LOG_DEBUG( "EVENT_MEMORY_FULL: mem store %d\n", cfg_info_ptr->mem_store );

    if ( cfg_info_ptr->mem_store == WMS_MEMORY_STORE_SIM ) 
    {
      /* Inform RIL that SIM is full */
      response_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
    }
    else
    {
      /* Inform RIL that RUIM is full */
      response_id = RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL;
    }

    qcril_default_unsol_resp_params( instance_id, response_id, &unsol_resp );
    qcril_send_unsol_response( &unsol_resp );
  }

} /* qcril_sms_cfg_event_memory_full */


/*=========================================================================
  FUNCTION:  qcril_sms_bc_mm_event_table

===========================================================================*/
/*!
    @brief
    Handle WMS_BC_MM_EVENT_TABLE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_bc_mm_event_table
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_reqlist_public_type req_info;
  uint32 i;
  wms_bc_mm_service_info_s_type *table_entries_ptr;
  uint16 table_size;
  RIL_CDMA_BroadcastSmsConfigInfo *cdma_buf_ptr;
  RIL_CDMA_BroadcastSmsConfigInfo **cdma_table_ptr;
  RIL_GSM_BroadcastSmsConfigInfo *gw_buf_ptr;
  RIL_GSM_BroadcastSmsConfigInfo **gw_table_ptr;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  table_entries_ptr = (wms_bc_mm_service_info_s_type *)params_ptr->data;
  QCRIL_ASSERT( table_entries_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Calculate the size of the table */
  table_size = params_ptr->datalen / sizeof ( wms_bc_mm_service_info_s_type );

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query_by_event( instance_id, modem_id, QCRIL_EVT_SMS_BC_MM_TABLE, &req_info ) == E_SUCCESS )
  {
    if ( req_info.request == RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG )
    {
      gw_buf_ptr = (RIL_GSM_BroadcastSmsConfigInfo *) qcril_malloc( table_size * sizeof( RIL_GSM_BroadcastSmsConfigInfo ) );
      if ( gw_buf_ptr == NULL )
      {
        QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the GW Broadcast SMS Event table" );
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
        return;
      }
      else
      {
        gw_table_ptr = (RIL_GSM_BroadcastSmsConfigInfo **) qcril_malloc( table_size * sizeof( RIL_GSM_BroadcastSmsConfigInfo *) );
        if ( gw_table_ptr == NULL )
        {
          QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the GW Broadcast SMS Event table" );
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );

          qcril_free( gw_buf_ptr );
          return;
        }
      }

      for ( i = 0; i < table_size; i++ )
      {
        gw_buf_ptr[ i ].fromServiceId = table_entries_ptr[ i ].srv_id.gw_cb_range.from;
        gw_buf_ptr[ i ].toServiceId = table_entries_ptr[ i ].srv_id.gw_cb_range.to;
        /* The modem does not support getting/setting the data coding
           scheme.  The interface specifies all values outside of 
           0x00 to 0xFF will be treated as empty data coding scheme. */
        gw_buf_ptr[ i ].fromCodeScheme = 0xFFF;
        gw_buf_ptr[ i ].toCodeScheme = 0xFFF;
        gw_buf_ptr[ i ].selected = table_entries_ptr[ i ].selected;

        gw_table_ptr[ i ] = &gw_buf_ptr[ i ];
      }

      qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = gw_table_ptr;
      resp.resp_len = table_size * sizeof( RIL_GSM_BroadcastSmsConfigInfo * );
      qcril_send_request_response( &resp );

      qcril_free( gw_table_ptr );
      qcril_free( gw_buf_ptr );
    }
    else if ( req_info.request == RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG )
    {
      cdma_buf_ptr = (RIL_CDMA_BroadcastSmsConfigInfo *) qcril_malloc( table_size * sizeof( RIL_CDMA_BroadcastSmsConfigInfo ) );
      if ( cdma_buf_ptr == NULL )
      {
        QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the CDMA Broadcast SMS Event table" );
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
        return;
      }
      else
      {
        cdma_table_ptr = (RIL_CDMA_BroadcastSmsConfigInfo **) qcril_malloc( table_size * sizeof( RIL_CDMA_BroadcastSmsConfigInfo *) );
        if ( cdma_table_ptr == NULL )
        {
          QCRIL_LOG_ERROR( "%s\n", "No buffer to compose the CDMA Broadcast SMS Event table" );
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );

          qcril_free( cdma_buf_ptr );
          return;
        }
      }

      for ( i = 0; i < table_size; i++ )
      {
        cdma_buf_ptr[ i ].language = table_entries_ptr[ i ].srv_id.bc_srv_id.language;
        cdma_buf_ptr[ i ].service_category = table_entries_ptr[ i ].srv_id.bc_srv_id.service;
        cdma_buf_ptr[ i ].selected = table_entries_ptr[ i ].selected;

        cdma_table_ptr[ i ] = &cdma_buf_ptr[ i ];
      }

      qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = cdma_table_ptr;
      resp.resp_len = table_size * sizeof (RIL_CDMA_BroadcastSmsConfigInfo *);
      qcril_send_request_response( &resp );

      qcril_free( cdma_table_ptr );
      qcril_free( cdma_buf_ptr );
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "%s", "ReqList entry not found for WMS_BC_MM_EVENT_TABLE\n" ); 
  }

}; /* qcril_sms_bc_mm_event_table */

/*=========================================================================
  FUNCTION:  qcril_sms_bc_mm_event_add_services

===========================================================================*/
/*!
    @brief
    Handle WMS_BC_MM_EVENT_ADD_SRVS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_bc_mm_event_add_services
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_done_list;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  IxErrnoType result;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query_by_event( instance_id, modem_id, QCRIL_EVT_SMS_BC_MM_ADD_SRVS, &req_info ) == E_SUCCESS )
  {
    if ( qcril_reqlist_complete_all_amss_events( instance_id, modem_id, req_info.t, QCRIL_REQ_COMPLETED_SUCCESS, &modem_ids_done_list, 
                                                 &result ) == E_SUCCESS )
    {
      if ( result == E_SUCCESS )
      {
        /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
        qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        /* Send response and remove the entry from ReqList */
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
      }
    }
    else
    {
      QCRIL_LOG_DEBUG( "Waiting for more WMS_BC_MM_EVENT_ADD_SRVS, rxed on modem id %d\n", modem_id ); 
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "%s", "ReqList entry not found for WMS_BC_MM_EVENT_ADD_SRVS\n" ); 
  }

}; /* qcril_sms_bc_mm_event_add_services */


/*=========================================================================
  FUNCTION:  qcril_sms_bc_mm_event_pref

===========================================================================*/
/*!
    @brief
    Handle WMS_BC_MM_EVENT_PREF.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_bc_mm_event_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_done_list;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  IxErrnoType result;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Look for the event in the ReqList */
  if ( qcril_reqlist_query_by_event( instance_id, modem_id, QCRIL_EVT_SMS_BC_MM_PREF, &req_info ) == E_SUCCESS )
  {
    if ( qcril_reqlist_complete_all_amss_events( instance_id, modem_id, req_info.t, QCRIL_REQ_COMPLETED_SUCCESS, &modem_ids_done_list, 
                                                 &result ) == E_SUCCESS )
    {
      if ( result == E_SUCCESS )
      {
        /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
        qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        /* Send response and remove the entry from ReqList */
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
      }
    }
    else
    {
      QCRIL_LOG_DEBUG( "Waiting for more WMS_BC_MM_EVENT_PREF, rxed on modem id %d\n", modem_id ); 
    }
  }
  else
  {
    /* Entry not found in the ReqList */
    QCRIL_LOG_ERROR( "%s", "ReqList entry not found for WMS_BC_MM_EVENT_PREF\n" ); 
  }

}; /* qcril_sms_bc_mm_event_pref */


/*=========================================================================
  FUNCTION:  qcril_sms_construct_gw_msg_pdu

===========================================================================*/
/*!
    @brief
    Construct the PDU to be sent to the Android SMS Application.

    @return
    TRUE if construction of PDU succeeded; FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_construct_gw_msg_pdu
(
  wms_msg_event_info_s_type *event_ptr,
  char *buf_ptr
)
{
  cm_called_party_bcd_no_T bcd_address;
  uint32 hex_pdu_pos = 0;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( buf_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Convert the address to BCD format */
  qcril_sms_ts_convert_to_bcd_address( &event_ptr->mt_message_info.message.u.gw_message.sc_address, &bcd_address );

  if ( !bcd_address.present )
  {
    QCRIL_LOG_ERROR( "%s", "Failed to construct SMS PDU!\n");
    return FALSE;
  }

  /* Tranlsate the length of the SMSC address to an ASCII hex char. */
  qcril_sms_byte_to_hex( &bcd_address.length, &buf_ptr[ hex_pdu_pos ], 1 );
  /* The length is 1 byte long; after conversion to ASCII hex, it is 2 bytes. */
  hex_pdu_pos = 2;

  /* Translate the SMSC address to ASCII hex char format */
  qcril_sms_byte_to_hex( bcd_address.data, &buf_ptr[ hex_pdu_pos ], bcd_address.length );

  /* Advance to the end of the SMSC address in the hex pdu */
  hex_pdu_pos += bcd_address.length * 2;

  /* Translate the SMS message into ASCII hex format */
  qcril_sms_byte_to_hex( &event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.data[ 0 ],
                         &buf_ptr[ hex_pdu_pos ],
                         event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.len );

  hex_pdu_pos += event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.len * 2;

  /* Terminate the ASCII hex string with a null character */
  buf_ptr[ hex_pdu_pos ] = '\0';

  return TRUE;

}; /* qcril_sms_construct_gw_msg_pdu */


/*=========================================================================
  FUNCTION:  qcril_sms_process_rxed_gw_sms

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a GW SMS message or Status Report
    is received.

    @return
    TRUE if the error check passed; FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_process_rxed_gw_sms
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  wms_msg_event_info_s_type *event_ptr,
  char *buf_ptr
)
{
  uint8 i;
  qcril_sms_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    /* If there is an ack pending, this means the Android SMS Application has yet
       to ack a previous SMS Message or Status Report.  According to ril.h, we
       are to drop all new SMS Messages/Status Reports until the previous message
       has been acked.  Note that the network will not send any new messages until
       the previous one has been acked; so the messages we are dropping here should
       all be duplicates. */
    if ( i_ptr->sms_ack_info[ i ].gw_ack_pending )
    {
      QCRIL_LOG_DEBUG( "%s", "Ignoring GW SMS Message or Status Report, Waiting for RIL to ack a previous SMS.\n");
      return FALSE;
      break;
    }
  }

  /* Construct the PDU to be sent to the Android SMS Application. */
  if ( qcril_sms_construct_gw_msg_pdu( event_ptr, buf_ptr ) )
  {
    /* An SMS Message or a Status Report was just received.  QCRIL should drop
       all subsequent SMS Messages/Status Reports until the Android SMS application
       calls RIL_REQUEST_SMS_ACKNOWLEDGE. */
    i_ptr->sms_ack_info[ modem_id ].gw_ack_pending = TRUE;

    if ( event_ptr->mt_message_info.route == WMS_ROUTE_TRANSFER_AND_ACK )
    {
      /* WMS overwrote the route to be WMS_ROUTE_TRANSFER_AND_ACK; this means WMS already
         acked the message.  Drop the ack from the Android application. */
      i_ptr->sms_ack_info[ modem_id ].gw_ack_is_needed = FALSE;

      /* Ignore the ril.h requirement to wait for an ack before delivering new MT
       messages to Android. The segments of a multi-page BC SMS message are received
       too quickly for Android to ack them, which leads to segments being dropped. */
      i_ptr->sms_ack_info[ modem_id ].gw_ack_pending = FALSE;
    }
    else
    {
      /* In all other cases, the Android application should ack the message. */
      i_ptr->sms_ack_info[ modem_id ].gw_ack_is_needed = TRUE;
    }

    /* Also, save the transaction id, for use when the Android SMS application acks this
       message. */
    i_ptr->sms_ack_info[ modem_id ].gw_transaction_id = event_ptr->mt_message_info.transaction_id;

    return TRUE;
  }

  return FALSE;

}; /* qcril_sms_process_rxed_gw_sms */

/*=========================================================================
  FUNCTION:  qcril_sms_clearing_cdma_ack
===========================================================================*/
void qcril_sms_clearing_cdma_ack( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id )
{
    qcril_sms_struct_type *i_ptr;
    char details[ 80 ];

    i_ptr = &qcril_sms[ instance_id ];

    QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
    QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

    QCRIL_LOG_INFO( "current CDMA ack pending %d, needed %d",
                    i_ptr->sms_ack_info[modem_id].cdma_ack_pending,
                    i_ptr->sms_ack_info[modem_id].cdma_ack_is_needed  );

    i_ptr->sms_ack_info[modem_id].cdma_ack_pending = FALSE;
    i_ptr->sms_ack_info[modem_id].cdma_ack_is_needed = FALSE;
    i_ptr->sms_ack_info[modem_id].cdma_ack_is_requested = FALSE;

    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
}

/*=========================================================================
  FUNCTION:  qcril_sms_convert_rxed_cdma_sms_to_RIL_format

===========================================================================*/
/*!
    @brief
    Translates a CDMA SMS message from internal WMS format to RIL format.        

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_convert_rxed_cdma_sms_to_RIL_format
(
  wms_cdma_message_s_type *wms_sms_message,
  RIL_CDMA_SMS_Message *RIL_sms_message
)
{
  uint32 i;

  RIL_sms_message->uTeleserviceID = wms_sms_message->teleservice;
  RIL_sms_message->bIsServicePresent = wms_sms_message->is_service_present;
  RIL_sms_message->uServicecategory = wms_sms_message->service;

  RIL_sms_message->sAddress.digit_mode = wms_sms_message->address.digit_mode;
  RIL_sms_message->sAddress.number_mode = wms_sms_message->address.number_mode;
  RIL_sms_message->sAddress.number_type = wms_sms_message->address.number_type;
  RIL_sms_message->sAddress.number_plan = wms_sms_message->address.number_plan;
  RIL_sms_message->sAddress.number_of_digits = wms_sms_message->address.number_of_digits;

  for ( i = 0; i < RIL_sms_message->sAddress.number_of_digits; i++ )
  {
    RIL_sms_message->sAddress.digits[ i ] = wms_sms_message->address.digits[i]; 
  }

  RIL_sms_message->sSubAddress.subaddressType = wms_sms_message->subaddress.type; 
  RIL_sms_message->sSubAddress.odd = wms_sms_message->subaddress.odd; 
  RIL_sms_message->sSubAddress.number_of_digits =  
  wms_sms_message->subaddress.number_of_digits; 

  for ( i = 0; i < RIL_sms_message->sSubAddress.number_of_digits; i++ )
  {
    RIL_sms_message->sSubAddress.digits[ i ] = wms_sms_message->subaddress.digits[i]; 
  }

  RIL_sms_message->uBearerDataLen = wms_sms_message->raw_ts.len;

  (void) memcpy( RIL_sms_message->aBearerData, wms_sms_message->raw_ts.data, RIL_sms_message->uBearerDataLen );

} /* qcril_sms_convert_rxed_cdma_sms_to_RIL_format */


/*=========================================================================
  FUNCTION:  qcril_sms_process_rxed_cdma_sms

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a CDMA SMS message or Status Report
    is received.

    @return
    TRUE if the error check passed; FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_process_rxed_cdma_sms
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  wms_msg_event_info_s_type *event_ptr,
  char *buf_ptr
)
{
  uint8 i;
  qcril_sms_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( event_ptr != NULL );
  QCRIL_ASSERT( buf_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    /* If there is an ack pending, this means the Android SMS Application has yet
       to ack a previous SMS Message or Status Report.  According to ril.h, we
       are to drop all new SMS Messages/Status Reports until the previous message
       has been acked.  Note that the network will not send any new messages until
       the previous one has been acked; so the messages we are dropping here should
       all be duplicates. */
    if ( ( i_ptr->sms_ack_info[ i ].cdma_ack_pending ) || ( CM_PH_STATE_EMERG_CB == qcril_cm_get_eme_cbm( instance_id ) ) )
    {
      QCRIL_LOG_DEBUG( "%s", "Ignoring CDMA SMS Message or Status Report. Waiting for RIL to ack a previous SMS or Waiting for RIL to Exit from ECBM mode.\n");
      return FALSE;
    }
  }

  /* Translates a CDMA SMS message from internal WMS format to RIL format. */
  qcril_sms_convert_rxed_cdma_sms_to_RIL_format( &event_ptr->mt_message_info.message.u.cdma_message, (RIL_CDMA_SMS_Message *) buf_ptr );

  /* An SMS Message or a Status Report was just received. QCRIL should drop all subsequent SMS Messages/Status Reports until the 
     Android SMS application calls RIL_REQUEST_SMS_ACKNOWLEDGE. */
  i_ptr->sms_ack_info[ modem_id ].cdma_ack_pending = TRUE;

  /* Store whether the client requested an ack or not. */
  i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_requested = event_ptr->mt_message_info.message.u.cdma_message.is_tl_ack_requested;

  if ( ( event_ptr->mt_message_info.route == WMS_ROUTE_TRANSFER_AND_ACK ) ||
       ( !i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_requested ) )
  {
    /* WMS overwrote the route to be WMS_ROUTE_TRANSFER_AND_ACK; this means WMS already
       acked the message. Or, client did not request an ack. Drop the ack from the Android
       application. */
    i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_needed = FALSE;

    /* Ignore the ril.h requirement to wait for an ack before delivering new MT
       messages to Android. The segments of a multi-page BC SMS message are received
       too quickly for Android to ack them, which leads to segments being dropped. */
    i_ptr->sms_ack_info[ modem_id ].cdma_ack_pending = FALSE;
  }
  else
  {
    /* In all other cases, the Android application should ack the message. */
    i_ptr->sms_ack_info[ modem_id ].cdma_ack_is_needed = TRUE;
  }

  /* Also, save the transaction id, for use when the Android SMS application acks this
      message. */
  i_ptr->sms_ack_info[ modem_id ].cdma_transaction_id = event_ptr->mt_message_info.transaction_id;

  return TRUE;

}; /* qcril_sms_process_rxed_cdma_sms */


/*=========================================================================
  FUNCTION:  qcril_sms_msg_event_received_message

===========================================================================*/
/*!
    @brief
    Handle WMS_MSG_EVENT_RECEIVED_MESSAGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_received_message
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  wms_msg_event_info_s_type *event_ptr; 
  uint8 *data;
  int index_on_sim;
  char *buf_ptr;
  char details[ 80 ];
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* If we are in Emergency CallBack Mode, drop all incoming messages. */
  if ( qcril_arb_in_emerg_cb_mode( instance_id ) )
  {
    QCRIL_LOG_DEBUG( "%s", "Dropping MT SMS because phone is in Emergency Callback Mode.\n" );
    return;
  }

  switch ( event_ptr->mt_message_info.message.msg_hdr.message_mode )
  {
    case WMS_MESSAGE_MODE_GW:
      QCRIL_LOG_DEBUG( "GW MT SMS: is_broadcast %d raw_ts_data_format %d route %d mem_store %d\n",
                       event_ptr->mt_message_info.message.u.gw_message.is_broadcast,
                       event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.format,
                       event_ptr->mt_message_info.route,
                       event_ptr->mt_message_info.message.msg_hdr.mem_store );
      /* Broadcast SMS */
      if ( event_ptr->mt_message_info.message.u.gw_message.is_broadcast &&
           ( event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.format == WMS_FORMAT_GW_CB ) )
      {
        /* It is a broadcast SMS.  Maximum size is 88 bytes long. */
        QCRIL_LOG_VERBOSE( "%s", "GSM Broadcast SMS Message");
        data = event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.data;
        QCRIL_LOG_DEBUG( "Bytes 0 through 9: %d %d %d %d %d %d %d %d %d %d",
                         data[0],data[1],data[2],data[3],data[4],data[5],
                         data[6],data[7],data[8],data[9]);
        QCRIL_LOG_DEBUG( "Bytes 10 through 19: %d %d %d %d %d %d %d %d %d %d",
                         data[10],data[11],data[12],data[13],data[14],
                         data[15],data[16],data[17],data[18],data[19]);
        QCRIL_LOG_DEBUG( "Bytes 20 through 29: %d %d %d %d %d %d %d %d %d %d",
                         data[20],data[21],data[22],data[23],data[24],data[25],
                         data[26],data[27],data[28],data[29]);
        QCRIL_LOG_DEBUG( "Bytes 30 through 39: %d %d %d %d %d %d %d %d %d %d",
                         data[30],data[31],data[32],data[33],data[34],
                         data[35],data[36],data[37],data[38],data[39]);
        QCRIL_LOG_DEBUG( "Bytes 40 through 49: %d %d %d %d %d %d %d %d %d %d",
                         data[40],data[41],data[42],data[43],data[44],
                         data[45],data[46],data[47],data[48],data[49]);
        QCRIL_LOG_DEBUG( "Bytes 50 through 59: %d %d %d %d %d %d %d %d %d %d",
                         data[50],data[51],data[52],data[53],data[54],
                         data[55],data[56],data[57],data[58],data[59]);
        QCRIL_LOG_DEBUG( "Bytes 60 through 69: %d %d %d %d %d %d %d %d %d %d",
                         data[60],data[61],data[62],data[63],data[64],
                         data[65],data[66],data[67],data[68],data[69]);
        QCRIL_LOG_DEBUG( "Bytes 70 through 79: %d %d %d %d %d %d %d %d %d %d",
                         data[70],data[71],data[72],data[73],data[74],
                         data[75],data[76],data[77],data[78],data[79]);
        QCRIL_LOG_DEBUG( "Bytes 80 through 88: %d %d %d %d %d %d %d %d",
                         data[80],data[81],data[82],data[83],data[84],
                         data[85],data[86],data[87]);

        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, &unsol_resp );
        unsol_resp.resp_pkt = ( void * ) event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.data;
        unsol_resp.resp_len = event_ptr->mt_message_info.message.u.gw_message.raw_ts_data.len;
        qcril_send_unsol_response( &unsol_resp );
      }

      /* Point-to-point SMS, on SIM */
      else if ( ( event_ptr->mt_message_info.route == WMS_ROUTE_STORE_AND_NOTIFY ) &&
                ( event_ptr->mt_message_info.message.msg_hdr.mem_store == WMS_MEMORY_STORE_SIM ) )
      {
        /* Android uses the GSDI index, since it calls RIL_REQUEST_SIM_IO.
           The GSDI index is the WMS index plus 1. */
        index_on_sim = event_ptr->mt_message_info.message.msg_hdr.index + 1;

        /* Notify RIL we received a class 2 message (new SMS on SIM) */
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, &unsol_resp );
        unsol_resp.resp_pkt = (void *) &index_on_sim;
        unsol_resp.resp_len = sizeof( int );
        qcril_send_unsol_response( &unsol_resp );
      }

      /* Point-to-point SMS, not on SIM */
      else if ( ( ( event_ptr->mt_message_info.route == WMS_ROUTE_TRANSFER_ONLY ) ||
                  ( event_ptr->mt_message_info.route == WMS_ROUTE_TRANSFER_AND_ACK ) ) &&
                ( event_ptr->mt_message_info.message.msg_hdr.mem_store == WMS_MEMORY_STORE_NONE ) 
              )
      {
        /* QCRIL SMS configures all non-class 2 SMS messages to be WMS_ROUTE_TRANSFER_ONLY;
           however WMS can overwrite the route to be WMS_ROUTE_TRANSFER_AND_ACK, so 
           check that as well. */
        QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
        QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

        /* Allocate buffer to decode new GW SMS */
        buf_ptr = qcril_malloc( QCRIL_SMS_BUF_MAX_SIZE ); 
        if ( buf_ptr == NULL )
        {
          QCRIL_LOG_ERROR( "%s", "Failed to allocate buffer to decode new GW SMS.\n");
          QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
          return;
        }

        /* Process received GW SMS */
        if ( !qcril_sms_process_rxed_gw_sms( instance_id, modem_id, event_ptr, buf_ptr) )
        {
          QCRIL_LOG_ERROR( "%s", "Cannot notify Android SMS application of new SMS.\n");
          qcril_free( buf_ptr );
          QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
          return;
        }

        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_SMS, &unsol_resp );
        unsol_resp.resp_pkt = ( void * ) buf_ptr;
        unsol_resp.resp_len = strlen( buf_ptr );
        qcril_send_unsol_response( &unsol_resp );

        qcril_free( buf_ptr );

        QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
      }
      break;

    case WMS_MESSAGE_MODE_CDMA:
      QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
      QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

      /* Allocate buffer to decode new CDMA SMS */
      buf_ptr = qcril_malloc( sizeof( RIL_CDMA_SMS_Message ) );
      if ( buf_ptr == NULL )
      {
        QCRIL_LOG_ERROR( "%s", "Failed to allocate buffer to decode new CDMA SMS.\n");
        QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
        return;
      }                    

      /* Process received CDMA SMS */
      if ( !qcril_sms_process_rxed_cdma_sms( instance_id, modem_id, event_ptr, buf_ptr ) )
      {
        QCRIL_LOG_ERROR( "%s", "Cannot notify Android SMS application of new CDMA SMS\n" );
        qcril_free( buf_ptr );
        QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
        return;
      }

      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) buf_ptr;
      unsol_resp.resp_len = sizeof( RIL_CDMA_SMS_Message );
      qcril_send_unsol_response( &unsol_resp );

      qcril_free( buf_ptr );

      QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
      break;

    default:
      break;

  } /* end switch*/

}; /* qcril_sms_msg_event_received_message */



#ifdef FEATURE_QCRIL_WMS_ETWS
/*===========================================================================

  FUNCTION:  qcril_sms_msg_event_etws_notification

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_SMS_ETWS_NOTIFICATION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_etws_notification
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  wms_msg_event_info_s_type *event_ptr; 
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_DEBUG("etws message length = %d", event_ptr->etws_msg_info.data_len);

  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, &unsol_resp );
  unsol_resp.resp_pkt = ( void * ) event_ptr->etws_msg_info.data;
  unsol_resp.resp_len = event_ptr->etws_msg_info.data_len;
  qcril_send_unsol_response( &unsol_resp );

} /* qcril_sms_msg_event_etws_notification */
#endif /* FEATURE_QCRIL_WMS_ETWS */



/*=========================================================================
  FUNCTION:  qcril_sms_msg_event_status_report

===========================================================================*/
/*!
    @brief
    Handle WMS_MSG_EVENT_STATUS_REPORT.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_msg_event_status_report                        
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_sms_struct_type *i_ptr;
  wms_msg_event_info_s_type *event_ptr; 
  char *buf_ptr;
  char details[ 80 ];
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_sms[ instance_id ];
  event_ptr = ( wms_msg_event_info_s_type * ) params_ptr->data; 
  QCRIL_ASSERT( event_ptr );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /* Allocate buffer to decode new GW SMS */
  buf_ptr = qcril_malloc( QCRIL_SMS_BUF_MAX_SIZE ); 
  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "Failed to allocate buffer to decode new GW SMS Status Report.\n" );
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  /* Process received GW SMS status report */
  if ( !qcril_sms_process_rxed_gw_sms( instance_id, modem_id, event_ptr, buf_ptr ) )
  {
    QCRIL_LOG_ERROR( "%s", "Cannot notify Android SMS application of GW SMS Status Report.\n" );
    qcril_free( buf_ptr );
    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    return;
  }

  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, &unsol_resp );
  unsol_resp.resp_pkt = ( void * ) buf_ptr;
  unsol_resp.resp_len = strlen( buf_ptr );
  qcril_send_unsol_response( &unsol_resp );

  qcril_free( buf_ptr );

  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

} /* qcril_sms_msg_event_status_report */

/*=========================================================================
  FUNCTION:  qcril_sms_read_nv_sms_utc.

===========================================================================*/
/*!
    @brief
    Gets the value of NV_SMS_UTC_I and updates the system property.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_read_nv_sms_utc
(
  void
)
{
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  qcril_modem_ids_list_type modem_ids_list;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  nv_item_type nv_item;
  char property_name[ 40 ];
  char args[ PROPERTY_VALUE_MAX ];

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_1;
  char *nv_cmd_api_name = "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name = "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /* Since on DSDS CDMA will be on first instance so use QCRIL_DEFAULT_INSTANCE_ID
   * as instance_id and SYS_MODEM_AS_ID_1 as as_id. Use default modem */

  QCRIL_LOG_DEBUG( "Reading NV_SMS_UTC_I on RID %d, modem_id %d\n", instance_id, modem_id);

  nv_item.sms_utc = TRUE;
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "read NV_SMS_UTC_I");
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_SMS_UTC_I, (nv_item_type *) &nv_item
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );
  QCRIL_LOG_DEBUG( "nv_status value = %d\n", nv_status);

  if ( ( nv_status != NV_DONE_S ) && ( nv_status != NV_NOTACTIVE_S ) )
  {
     QCRIL_LOG_ERROR( "%s", "Reading sms utc from NV_SMS_UTC_I failed.\n" );
     return;
  }
  QCRIL_LOG_DEBUG( "Value of NV_SMS_UTC_I = %d\n", nv_item.sms_utc);

  /* Save NV_SMS_UTC_I value to system property */
  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_NV_SMS_UTC);
  QCRIL_SNPRINTF( args, sizeof( args ), "%d", nv_item.sms_utc);
  if ( property_set( property_name, args ) != E_SUCCESS )
  {
     QCRIL_LOG_ERROR( "RID %d Fail to save %s to system property\n", instance_id, QCRIL_NV_SMS_UTC );
  }
  else
  {
     QCRIL_LOG_ERROR( "RID %d Saved %s to system property\n", instance_id, QCRIL_NV_SMS_UTC );
  }
} /* qcril_sms_read_nv_sms_utc */

/*=========================================================================
  FUNCTION:  qcril_sms_reset_sms_ack_pending_flags.

===========================================================================*/
/*!
    @brief
    Reset gw_ack_pending and cdma_ack_pending flags on radio off.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_reset_sms_ack_pending_flags
(
  qcril_instance_id_e_type instance_id
)
{
  uint8 i;
  qcril_sms_struct_type *i_ptr;

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_sms[ instance_id ];

  /* Lookup the modem where the received SMS is pending to be acked */
  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
     QCRIL_LOG_DEBUG( "In qcril_sms_reset_sms_ack_pending_flags, gw_ack_pending = %d,"
           "cdma_ack_pending = %d", i_ptr->sms_ack_info[i].gw_ack_pending,
           i_ptr->sms_ack_info[i].cdma_ack_pending);
     i_ptr->sms_ack_info[ i ].gw_ack_pending = FALSE;
     i_ptr->sms_ack_info[ i ].cdma_ack_pending = FALSE;
  }
} /* qcril_sms_reset_sms_ack_pending_flags */
