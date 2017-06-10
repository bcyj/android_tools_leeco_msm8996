/*!
  @file
  qcril_pbm.c

  @brief
  PBM event handling, implementation to get the emergency numbers
  from PBM cache, property file update with the ecc list is done here.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_pbm.c# $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
06/15/09   nd      Added support to check the CDMA emergency flash number in ECC property file.
06/01/09   sk      Initial framework


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <string.h>
#include <cutils/properties.h>
#include "qcril_log.h"
#include "qcril_pbm_api_map.h"
#include "qcril_arb.h"
#include "qcril_pbm.h"


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

static qcril_pbm_struct_type *qcril_pbm;


/*===========================================================================

                                FUNCTIONS

===========================================================================*/


/*==========================================================================
  FUNCTION:  qcril_pbm_process_event_callback
===========================================================================*/
/*!
    @brief
    Process PBM event callback.

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_process_event_callback
(
  void *pUserData,
  pbm_notify_data_s_type *pNotifyData
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_evt_e_type event_id;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( pUserData != NULL );
  QCRIL_ASSERT( pNotifyData != NULL );
  user_data = (uint32) pUserData;
  instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( user_data );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "RID %d MID %d Received PBM event : %d\n", instance_id, modem_id, pNotifyData->event );

  /* No mechanism to register for only specific events from PBM. Filter the events that are not handled here */
  if ( 
       #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
       ( pNotifyData->event == PBM_EVENT_SESSION_INIT_DONE ) ||
       #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */
       ( pNotifyData->event == PBM_EVENT_SIM_INIT_DONE ) ||
       ( pNotifyData->event == PBM_EVENT_PB_REFRESH_START ) ||
       ( pNotifyData->event == PBM_EVENT_PB_REFRESH_DONE ) ||
       ( ( pNotifyData->event == PBM_EVENT_PB_READY ) && 
         ( ( pNotifyData->data.pb_id == PBM_ECC ) || ( pNotifyData->data.pb_id == PBM_FDN )  
     ) ) )
  {
    /* Figure out qcril Event ID */
    event_id = QCRIL_EVT_PBM_BASE + pNotifyData->event;

    /* Broadcast to all instances for processing */
    for ( i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
    {
      QCRIL_LOG_INFO( "Route RID %d MID %d for %s(%d)\n", i, modem_id, qcril_log_lookup_event_name(event_id), event_id );

      /* Call event handler for pbm event */
      qcril_event_queue( i, modem_id, QCRIL_DATA_ON_STACK,
                         event_id, (void *) pNotifyData, sizeof( pbm_notify_data_s_type ), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
  }

} /* qcril_pbm_process_event_callback */


/*==========================================================================
  FUNCTION:  qcril_pbm_event_callback
===========================================================================*/
/*!
    @brief
    Callback invoked by PBM to post unsolicited events to qcril

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_event_callback
(
  void *pUserData,
  pbm_notify_data_s_type *pNotifyData
)
{
  QCRIL_LOG_ADB_MAIN("Enter %s\n", __func__);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( pUserData != NULL );
  QCRIL_ASSERT( pNotifyData != NULL );

  /*-----------------------------------------------------------------------*/

    qcril_pbm_process_event_callback( pUserData, pNotifyData );
  QCRIL_LOG_ADB_MAIN("Exit %s\n", __func__);

} /* qcril_pbm_event_callback */


/*=========================================================================
  FUNCTION:  qcril_pbm_clear_ecc_list

===========================================================================*/
/*!
    @brief
    Clear ECC list depending on the field id.

    @return
    None
*/
/*=========================================================================*/
void qcril_pbm_clear_ecc_list
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  pbm_field_id_e_type pbm_field_id
)
{
  qcril_pbm_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/


  switch ( pbm_field_id )
  {
    case PBM_FIELD_HARDCODED_ECC:
      /* In the absence of card, only hardcoded, NV ECC are applicable as per 3gpp 22.101 */
      QCRIL_LOG_DEBUG( "RID %d MID %d %s\n", instance_id, modem_id, "Clear the hardcoded ecc List" );
      i_ptr->ecc_info[ modem_id ].num_card_hcoded_ecc_entries = 0;
      i_ptr->ecc_info[ modem_id ].card_ecc_is_present = FALSE;
      break;

    case PBM_FIELD_NETWORK_ECC:
      /* Clear the Over the Air emergency number list as it is no longer valid */
      QCRIL_LOG_DEBUG( "RID %d MID %d %s\n", instance_id, modem_id, "Clear the network ecc List" );
      i_ptr->ecc_info[ modem_id ].num_ota_ecc_entries = 0;
      break;

    case PBM_FIELD_NONE :
      /* Clear the sim, hardcoded, NV ecc list so that the lists are populated afresh from PBM */
      QCRIL_LOG_DEBUG( "RID %d MID %d %s\n", instance_id, modem_id, "Clear the hardcode, network and NV ecc List" );
      i_ptr->ecc_info[ modem_id ].num_card_hcoded_ecc_entries = 0;
      i_ptr->ecc_info[ modem_id ].num_nv_ecc_entries = 0;
      i_ptr->ecc_info[ modem_id ].card_ecc_is_present = FALSE;
      break;

    default:
      QCRIL_LOG_ERROR( "RID %d MID %d Unsupported clear ecc list request %d\n", instance_id, modem_id, pbm_field_id );
      break;
  }

} /* qcril_pbm_clear_ecc_list */


/*===========================================================================

  FUNCTION:  qcril_pbm_init

===========================================================================*/
/*!
    @brief
    Initialize the interface with PBM and register for PBM unsolicited 
    Notifications.

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_init
(
  void
)
{
  uint8 i, j;
  qcril_pbm_struct_type *i_ptr;
  boolean registered = FALSE;
  uint32 user_data;
  pbm_return_type ret = PBM_ERROR;

  /*-----------------------------------------------------------------------*/

  /* Allow cache */
  qcril_pbm = (qcril_pbm_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_PBM );
  QCRIL_ASSERT( qcril_pbm != NULL );

  /* Initialize internal data */
  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_pbm[ i ];

    i_ptr->ecc_modem_id = QCRIL_MAX_MODEM_ID;

    for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
    {
      i_ptr->client_info[ j ].registered = FALSE;

      if ( QCRIL_PRIMARY_INSTANCE( i ) )
      {
        i_ptr->client_info[ j ].client_is_primary = TRUE;
      }
      else
      {
        i_ptr->client_info[ j ].client_is_primary = FALSE;
      }

      /* Initializes the NV, CARD ECC lists. */
      qcril_pbm_clear_ecc_list( i, j, PBM_FIELD_NONE );

      /* Initializes the Network ECC list */
      qcril_pbm_clear_ecc_list( i, j, PBM_FIELD_NETWORK_ECC );
    }
  }

  for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
  {
    /* Register with PBM to get the unsolicited events */
    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, i, QCRIL_REQ_ID_INTERNAL );
    QCRIL_LOG_RPC2( i, "pbm_notify_register()", "Register callback for PBM events" );
    ret = qcril_pbm_api_funcs[ i ].pbm_notify_register_func( qcril_pbm_api_event_callback[ i ], (void *) user_data );
    if ( ret == PBM_SUCCESS )
    {
      QCRIL_LOG_DEBUG( "MID %d, pbm_notify_register()\n", i );
      registered = TRUE;
    }
    else
    {
      /* Try unregistering and re-register*/
      QCRIL_LOG_RPC2( i, "pbm_notify_unregister()", "Unregister with PBM" );
      ret = qcril_pbm_api_funcs[ i ].pbm_notify_unregister_func( qcril_pbm_api_event_callback[ i ], (void *) user_data );
      if ( ret == PBM_SUCCESS )
      {
        QCRIL_LOG_DEBUG( "MID %d, pbm_notify_unregister()\n", i );
        /* Re-Register with PBM now. This case will arise when arm11 crashed but arm9 is up */
        QCRIL_LOG_RPC2( i, "pbm_notify_register()", "Re-Register for PBM events" );
        ret = qcril_pbm_api_funcs[ i ].pbm_notify_register_func( qcril_pbm_api_event_callback[ i ], (void *) user_data );
        if ( ret == PBM_SUCCESS )
        {
          QCRIL_LOG_DEBUG( "MID %d, pbm_notify_register() Re-Register for PBM events\n", i );
          registered = TRUE;
        }
      }
    }

    if ( !registered )
    {
      QCRIL_LOG_ERROR( "MID %d Registration with PBM Failed with ret = %d\n", i, ret );
    }
    else
    {
      /* Record the client registration */
      for ( j = 0; j < QCRIL_MAX_INSTANCE_ID; j++ )
      {
        i_ptr = &qcril_pbm[ j ];
        i_ptr->client_info[ i ].registered = TRUE;
      } /* for ( j = 0; j < QCRIL_ARB_MAX_INSTANCES; j++ ) */
      
    }

  }

} /* qcril_pbm_init */


/*===========================================================================
  FUNCTION:  qcril_pbm_release
===========================================================================*/
/*!
    @brief
    Unregister qcril from getting PBM notifications

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_release
(
  void
)
{
  uint8 i, j;
  qcril_pbm_struct_type *i_ptr;
  uint32 user_data;

  /*-----------------------------------------------------------------------*/

  /* Unregister only if already registered */
  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_pbm[ i ];

    for ( j = 0; j < QCRIL_ARB_MAX_MODEMS; j++ )
    {
      if ( i_ptr->client_info[ j ].registered )
      {
        i_ptr->client_info[ j ].registered = FALSE;

        if ( i_ptr->client_info[ j ].client_is_primary )
        {
          user_data = QCRIL_COMPOSE_USER_DATA( i, j, QCRIL_REQ_ID_INTERNAL );
          QCRIL_LOG_RPC2( j, "pbm_notify_unregister()", "Unregister with PBM" );
          (void) qcril_pbm_api_funcs[ j ].pbm_notify_unregister_func( qcril_pbm_api_event_callback[ j ], (void *) user_data );
        }
      }
    }
  }

} /* qcril_pbm_release */


/*=========================================================================

  FUNCTION:  qcril_pbm_enable_oprt_mode_check

===========================================================================*/
/*!
    @brief
    Set PBM cache initialized flag such that PBM events will be processed
    when phone enter Airplane mode.
 
    This should be called when the IL_REQUEST_RADIO_POWER request is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_pbm_enable_oprt_mode_check
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  qcril_pbm_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  if ( !i_ptr->oprt_mode_check_enabled  )
  {
    i_ptr->oprt_mode_check_enabled = TRUE;
    QCRIL_LOG_DEBUG( "%s\n", "PBM oprt mode check enabled" );
  }

} /* qcril_pbm_oprt_mode_check_enabled */


#ifdef FEATURE_QCRIL_FUSION
/*=========================================================================

  FUNCTION:  qcril_pbm_set_pb_id_for_svlte_1

===========================================================================*/
/*!
    @brief
    Set pd_id for SVLTE Type 1. 

    @return
    errno_enum_type
*/
/*=========================================================================*/
void qcril_pbm_set_pb_id_for_svlte_1
(
  qcril_instance_id_e_type instance_id,
  pbm_phonebook_type *pb_id_ptr,
  qcril_pbm_device_type device_type 
)
{
  /*-----------------------------------------------------------------------*/

  pb_id_ptr->pb_category = PBM_LPB;
  /* For SVLTE Type 1, only CSIM ECC numbers need to be reported */
  pb_id_ptr->prov_type = PBM_PROVISION_1X_PRIMARY;
  pb_id_ptr->slot_id = 1;
  if ( device_type == QCRIL_PBM_DEVICE_TYPE_ECC ) 
  {
    pb_id_ptr->device_type = PBM_ECC;
  }
  else
  {
    pb_id_ptr->device_type = PBM_FDN;
  }

} /* qcril_pbm_set_pd_id_for_svlte_1 */
#endif /* FEATURE_QCRIL_FUSION */


#ifdef FEATURE_QCRIL_DSDS
/*=========================================================================

  FUNCTION:  qcril_pbm_set_pb_id_from_subscription

===========================================================================*/
/*!
    @brief
    Set pd_id based on subscription info. 

    @return
    errno_enum_type
*/
/*=========================================================================*/
errno_enum_type qcril_pbm_set_pb_id_from_subscription
(
  qcril_instance_id_e_type instance_id,
  pbm_phonebook_type *pb_id_ptr,
  qcril_pbm_device_type device_type,
  boolean lock_ph_mutex
)
{
  qcril_arb_subs_prov_status_e_type subs_state;
  RIL_SelectUiccSub uicc_sub;                   
  sys_modem_as_id_e_type as_id;                
  qmi_uim_session_type session_type; 
  qcril_modem_id_e_type modem_id;
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type voice_radio_tech;
  #else
  qcril_radio_tech_e_type        voice_radio_tech;
  #endif

  /*-----------------------------------------------------------------------*/

  pb_id_ptr->pb_category = PBM_LPB;
  pb_id_ptr->prov_type = PBM_PROVISION_GW_PRIMARY;
  pb_id_ptr->slot_id = 1;

  if ( device_type == QCRIL_PBM_DEVICE_TYPE_ECC )
  {
    pb_id_ptr->device_type = PBM_ECC;
  }
  else
  {
    pb_id_ptr->device_type = PBM_FDN;
  }

  if ( !qcril_arb_ma_is_dsds() )
  {
    /* Set provision type based on which radio tech UE is active on */
    if ( qcril_arb_query_voice_srv_modem_id( instance_id, &modem_id, &voice_radio_tech ) == E_SUCCESS ) 
    {
      if ( QCRIL_RADIO_TECH_IS_3GPP(voice_radio_tech)  )
      {
        pb_id_ptr->prov_type = PBM_PROVISION_GW_PRIMARY;
      }
      else if ( QCRIL_RADIO_TECH_IS_3GPP2(voice_radio_tech) && 
                !qcril_arb_rtre_control_is_nv( instance_id, modem_id, lock_ph_mutex ) )
      {
        pb_id_ptr->prov_type = PBM_PROVISION_1X_PRIMARY;
      }
      else if ( device_type == QCRIL_PBM_DEVICE_TYPE_ECC )
      {
        pb_id_ptr->device_type = PBM_ECC_OTHER;
      }
      else
      {
        return E_FAILURE;
      }
    }
    else if ( device_type == QCRIL_PBM_DEVICE_TYPE_ECC )
    {
      QCRIL_LOG_DEBUG( "%s\n", "RAT not known, access hardcoded ECC" );
      pb_id_ptr->device_type = PBM_ECC_OTHER;
    }
    else
    {
      return E_FAILURE;
    }
  }
  else
  {
    /* Lookup subscription info */
    qcril_arb_query_subs( instance_id, &subs_state, &uicc_sub, &as_id, &session_type );
    if ( subs_state == QCRIL_ARB_SUBS_NOT_PROVISIONED )
    {
      if ( device_type == QCRIL_PBM_DEVICE_TYPE_ECC )
      {
        QCRIL_LOG_DEBUG( "%s\n", "Subscription not available, access hardcoded ECC" );
        pb_id_ptr->device_type = PBM_ECC_OTHER;
      }
      else
      {
        return E_FAILURE;
      }
    }
    else 
    {
      switch ( session_type )
      {
        case QMI_UIM_SESSION_TYPE_PRI_GW_PROV:
          pb_id_ptr->prov_type = PBM_PROVISION_GW_PRIMARY;
          break;

        case QMI_UIM_SESSION_TYPE_PRI_1X_PROV:
          pb_id_ptr->prov_type = PBM_PROVISION_1X_PRIMARY;
          break;

        case QMI_UIM_SESSION_TYPE_SEC_GW_PROV:
          pb_id_ptr->prov_type = PBM_PROVISION_GW_SECONDARY;
          break;

        case QMI_UIM_SESSION_TYPE_SEC_1X_PROV:
          pb_id_ptr->prov_type = PBM_PROVISION_1X_SECONDARY;
          break;

        default:
          QCRIL_LOG_ERROR( "Invalid session type %d as subscription\n", session_type );
          return E_FAILURE;
      }

      pb_id_ptr->slot_id = uicc_sub.slot;
    }
  }

  QCRIL_LOG_DEBUG( "RID %d pb_id: category %d prov %d, slot %d, device %d\n", instance_id,  
                   pb_id_ptr->pb_category, pb_id_ptr->prov_type, pb_id_ptr->slot_id, pb_id_ptr->device_type );

  return E_SUCCESS;

} /* qcril_pbm_set_pb_id_from_subscription */
#endif /* FEATURE_QCRIL_DSDS */


/*=========================================================================

  FUNCTION:  qcril_pbm_update_ecc_cache

===========================================================================*/
/*!
    @brief
    Update the local list of emergency numbers based on the field id

    @return
    errno_enum_type
*/
/*=========================================================================*/
errno_enum_type qcril_pbm_update_ecc_cache
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  pbm_field_id_e_type pbm_field_id,
  char *ecc_val,
  uint16 ecc_len
)
{
  qcril_pbm_struct_type *i_ptr;
  errno_enum_type ret = E_SUCCESS;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/
  
  if ( ecc_val != NULL )
  {
    switch( pbm_field_id )
    {
      case PBM_FIELD_NV_ECC:
        QCRIL_LOG_DEBUG( "RID %d MID %d NV_ECC ecc_len=%d, ecc_val=%s\n", instance_id, modem_id, ecc_len, ecc_val );
        if ( ( ecc_len <= ( PBM_NUM_SIZE - 1 ) ) && ( i_ptr->ecc_info[ modem_id ].num_nv_ecc_entries < PBM_NV_EMERGENCY_NUMBERS ) )
        {
          /* Copy the ecc value in the ecc cache */
          QCRIL_SNPRINTF( i_ptr->ecc_info[ modem_id ].nv_ecc[ i_ptr->ecc_info[ modem_id ].num_nv_ecc_entries++ ], 
                          ecc_len, "%s", ecc_val );
        }
        else
        {
          QCRIL_LOG_ERROR( "%s\n", "No space left to save ecc" );
        }
        break;

      case PBM_FIELD_SIM1_ECC:
      case PBM_FIELD_SIM2_ECC:
        i_ptr->ecc_info[ modem_id ].card_ecc_is_present = TRUE;
      case PBM_FIELD_HARDCODED_ECC:
        if ( pbm_field_id == PBM_FIELD_SIM1_ECC )
        {
          QCRIL_LOG_DEBUG( "RID %d MID %d SIM1_ECC ecc_len=%d, ecc_val=%s\n", instance_id, modem_id, ecc_len, ecc_val );
        }
        else if ( pbm_field_id == PBM_FIELD_SIM2_ECC )
        {
          QCRIL_LOG_DEBUG( "RID %d MID %d SIM2_ECC ecc_len=%d, ecc_val=%s\n", instance_id, modem_id, ecc_len, ecc_val );
        }
        else
        {
          QCRIL_LOG_DEBUG( "RID %d MID %d HARDCODED_ECC ecc_len=%d, ecc_val=%s\n", instance_id, modem_id, ecc_len, ecc_val );
        }
        if ( ( ecc_len <= ( PBM_NUM_SIZE - 1 ) ) && 
             ( i_ptr->ecc_info[ modem_id ].num_card_hcoded_ecc_entries < QCRIL_PBM_MAX_CARD_HCODED_ECC_NUMBERS ) )
        {
          QCRIL_SNPRINTF( i_ptr->ecc_info[ modem_id ].card_hcoded_ecc[ i_ptr->ecc_info[ modem_id ].num_card_hcoded_ecc_entries++ ], 
                          ecc_len, "%s", ecc_val );
        }
        else
        {
          QCRIL_LOG_ERROR( "%s\n", "No space left to save ecc" );
        }
        break;
      
      case PBM_FIELD_NETWORK_ECC:
        QCRIL_LOG_DEBUG( "RID %d MID %d NETWORK_ECC ecc_len=%d, ecc_val=%s\n", instance_id, modem_id, ecc_len, ecc_val );
        if ( ( ecc_len <= CM_MAX_NUMBER_CHARS ) && ( i_ptr->ecc_info[ modem_id ].num_ota_ecc_entries < CM_MAX_EMERGENCY_NUM_COUNT ) )
        {
          QCRIL_SNPRINTF( i_ptr->ecc_info[ modem_id ].ota_ecc[ i_ptr->ecc_info[ modem_id ].num_ota_ecc_entries++ ], (ecc_len+1), 
                          "%s", ecc_val );
        }
        else
        {
          QCRIL_LOG_ERROR( "%s\n", "No space left to save ecc" );
        }
        break;

      default:
        QCRIL_LOG_DEBUG( "RID %d MID %d unsupported pbm_field_id=%d ecc_len=%d, ecc_val=%s\n", 
                         instance_id, modem_id, pbm_field_id, ecc_len, ecc_val );
        ret = E_DATA_INVALID;
        break;
    }
  }
  else
  {
    QCRIL_LOG_ERROR( "RID %d MID %d ecc_val passed is NULL", instance_id, modem_id );
    ret = E_FAILURE;
  }

  return ret;

} /* qcril_pbm_update_ecc_cache */


/*=========================================================================

FUNCTION:  qcril_pbm_get_ecc

===========================================================================*/
/*!
    @brief
    Get Emergency numbers from PBM depending on the Field ID passed.
    PBM_FIELD_NONE, PBM_FIELD_HARDCODED_ECC are passed as Field ID
    If PBM_FIELD_NONE is passed, get all the ECC (NV, SIM, Hardcoded, 
    network). PBM_FIELD_HARDCODED_ECC just gets the hardcoded ECC which
    are applicable when sim is absent.

    @return
    errno_enum_type
*/
/*=========================================================================*/
errno_enum_type qcril_pbm_get_ecc
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  pbm_field_id_e_type pbm_field_id,
  boolean lock_ph_mutex
)
{
  int num_fields = 0, offset = 0, i = 0;
  uint32 data_buf_size;
  uint16 cat = 0;
  char *num = NULL;
  pbm_field_s_type *data_buf;
  errno_enum_type ret_errno = E_SUCCESS;
  pbm_return_type pbm_status;
  
  #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
  pbm_phonebook_type pb_id;  
  pbm_record_id_type rec_id = 0;
  char *pbm_enum_rec_init_api_name = "pbm_session_enum_rec_init()";
  char *pbm_enum_next_rec_id_api_name = "pbm_session_enum_next_rec_id()";
  char *pbm_calculate_fields_size_from_id_api_name = "pbm_session_calculate_fields_size_from_id()";
  char *pbm_record_read_api_name = "pbm_session_record_read()"; 
  #else
  uint16 rec_id = 0;
  char *pbm_enum_rec_init_api_name = "pbm_enum_rec_init()";
  char *pbm_enum_next_rec_id_api_name = "pbm_enum_next_rec_id()";
  char *pbm_calculate_fields_size_from_id_api_name = "pbm_calculate_fields_size_from_id()";
  char *pbm_record_read_api_name = "pbm_record_read()"; 
  #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
    
  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_FUSION
  /* Set PB ID */
  qcril_pbm_set_pb_id_for_svlte_1( instance_id, &pb_id, QCRIL_PBM_DEVICE_TYPE_ECC );
  #endif /* FEATURE_QCRIL_FUSION */

  #ifdef FEATURE_QCRIL_DSDS
  /* Set PB ID */
  if ( qcril_pbm_set_pb_id_from_subscription( instance_id, &pb_id, QCRIL_PBM_DEVICE_TYPE_ECC, lock_ph_mutex ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s\n", "Fail to set pb_id" );
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Clear the ecc list based on field id */
  qcril_pbm_clear_ecc_list( instance_id, modem_id, pbm_field_id );

  /* Trigger the PBM record list formation the ECC records matching the cache, category, field id */
  QCRIL_LOG_RPC( modem_id, pbm_enum_rec_init_api_name, "pbm_field_id", pbm_field_id );
  pbm_status = qcril_pbm_api_funcs[ modem_id ].pbm_enum_rec_init_func( 
                                                                       #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
                                                                       pb_id, 
                                                                       #else
                                                                       PBM_ECC,
                                                                       #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */
                                                                       (uint16) PBM_CAT_ECC, pbm_field_id, NULL, 0, 0 ) ;

  if ( pbm_status == PBM_SUCCESS )
  {
    /* The record list in PBM is formed. Get the record ids one by one */
    QCRIL_LOG_RPC2( modem_id, pbm_enum_next_rec_id_api_name, "get record id" );
    while ( qcril_pbm_api_funcs[ modem_id ].pbm_enum_next_rec_id_func( &rec_id ) == PBM_SUCCESS )
    {
      /* Get the total size of all the fields in the record using the record id */
      QCRIL_LOG_RPC2( modem_id, pbm_calculate_fields_size_from_id_api_name, "get record size");
      data_buf_size = qcril_pbm_api_funcs[ modem_id ].pbm_calculate_fields_size_from_id_func( rec_id );
      data_buf = qcril_malloc( data_buf_size );
      if ( data_buf == NULL )
      {
        QCRIL_LOG_ERROR( "RID %d MID %d malloc failed. Cannot allocate memory for size = %d \n", instance_id, modem_id, 
                         data_buf_size );
        continue;
      }

      /* Read the record */
      QCRIL_LOG_RPC2( modem_id, pbm_record_read_api_name, "read ecc record" );
      if ( qcril_pbm_api_funcs[ modem_id ].pbm_record_read_func( rec_id, &cat, &num_fields, (uint8*) data_buf, 
                                                                 data_buf_size ) == PBM_SUCCESS )
      {
        for ( i = 0; i < num_fields; i++ )
        {
          /* Check if the field type is that of ECC number. Network ECC are not handled here to avoid redundancy */
          if ( ( data_buf[ i ].field_type == PBM_FT_PHONE ) && ( data_buf[ i ].field_id != PBM_FIELD_NETWORK_ECC ) )
          {
            /* Extract the number from the record field */
            offset = data_buf[ i ].buffer_offset;
            num = (char *) &( data_buf[ i ] ) + offset;

            /* Store the ECC num in list. */
            qcril_pbm_update_ecc_cache( instance_id, modem_id, data_buf[ i ].field_id, num, data_buf[ i ].data_len );
          }
        }
      }

      qcril_free( data_buf );
    }
  }
  else
  {
    QCRIL_LOG_ERROR( " %s: Error %d returned for pbm_enum_rec_init\n", __FUNCTION__, pbm_status );
    ret_errno = E_FAILURE;
  }

  return ret_errno;

} /* qcril_pbm_get_ecc */


/*=========================================================================

  FUNCTION:  qcril_pbm_update_ecc_property

===========================================================================*/
/*!
    @brief
    Put the consolidated list in property file using property_set();

    @return
    None
*/
/*=========================================================================*/
void qcril_pbm_update_ecc_property
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  boolean update_from_pbm_or_sub_event
)
{
  qcril_pbm_struct_type *i_ptr;
  char prop_val[ PROPERTY_VALUE_MAX ];
  uint8 i = 0, j = 0, k = instance_id;
  uint32 len;
  int cnt = 0;
  boolean is_prop_val_full = FALSE; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];

  /*-----------------------------------------------------------------------*/

  /* Update triggered PBM event needs unconditional update. 
     Update triggered by preferred network type change or serving system change (can be very freqent) should be honored if 
     there is a change in modem ID */
  if ( !update_from_pbm_or_sub_event && 
       ( ( modem_id == QCRIL_MAX_MODEM_ID ) || ( i_ptr->ecc_modem_id == modem_id ) ) )
  {
    QCRIL_LOG_DEBUG( "RID %d MID %d no update on ril.ecclist needed", instance_id, modem_id );
    return;
  }

  /* Remember modem id that owns the ECC property */
  i_ptr->ecc_modem_id = modem_id;

  /* initialize the property value buffer */
  memset( prop_val, '\0', PROPERTY_VALUE_MAX );

  #ifdef FEATURE_QCRIL_DSDS
  /* For DSDS, combine ECC list from both instances */
  for ( k = 0; k < QCRIL_ARB_MAX_INSTANCES; k++ )
  #endif /* FEATURE_QCRIL_DSDS */
  {
    i_ptr = &qcril_pbm[ k ];

    /* As no specified order is mentioned, we would like to format the ECC
       list as per operator instructed (OTA), operator programmed (Card) and 
       OEM programmed (NV). The final ecc list is a string of comma seperated 
       emergency numbers */

    /* Copy the OTA ecc entries to the property value buffer */
    for( i = 0; i < i_ptr->ecc_info[ modem_id ].num_ota_ecc_entries; i++ )
    {
      /* Check if we can fit one more valid emergency number in the list
         along with a comma. cnt indicates the string length copied till now
         to the buffer, 1 is for the comma */
      len = strlen( i_ptr->ecc_info[ modem_id ].ota_ecc[ i ] );
      if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
      {
        if ( prop_val[ 0 ] != '\0' )
        {
          /* If this is not the first emergency number, add a comma as a delimiter */
          prop_val[ cnt++ ] = ',';
        }

        for ( j = 0; j < len; j++ )
        {
          prop_val[ cnt++ ] = i_ptr->ecc_info[ modem_id ].ota_ecc[ i ][ j ];
        }
      }
      else
      {
        /* We cannot fit in any more emergency numbers */
        is_prop_val_full = TRUE;
        break;
      }
    }
    /* Copy the Card and hardcoded ecc entries to the property value buffer
       in the similar fashion as done for OTA numbers */
    if ( !is_prop_val_full )
    {
      for ( i = 0; i < i_ptr->ecc_info[ modem_id ].num_card_hcoded_ecc_entries; i++ )
      {
        len = strlen( i_ptr->ecc_info[ modem_id ].card_hcoded_ecc[ i ] );
        if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
        {
          if ( prop_val[ 0 ] != '\0' )
          {
            prop_val[ cnt++ ] = ',';
          }

          for ( j = 0; j < len; j++ )
          {
            prop_val[ cnt++ ] = i_ptr->ecc_info[ modem_id ].card_hcoded_ecc[ i ][ j ];
          }
        }
        else
        {
          is_prop_val_full = TRUE;
          break;
        }
      }
    }

    /* Copy the NV ecc entries to the property value buffer in the similar fashion as done for the OTA, Card numbers */
    if ( !is_prop_val_full )
    {
      for( i = 0; i < i_ptr->ecc_info[ modem_id ].num_nv_ecc_entries; i++ )
      {
        len = strlen( i_ptr->ecc_info[ modem_id ].nv_ecc[ i ] );
        if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
        {
          if ( prop_val[ 0 ] != '\0' )
          {
            prop_val[ cnt++ ] = ',';
          }

          for( j = 0; j < len; j++ )
          {
            prop_val[ cnt++ ] = i_ptr->ecc_info[ modem_id].nv_ecc[ i ][ j ];
          }
        }
        else
        {
          is_prop_val_full = TRUE;
          break;
        }
      }
    }
  }

  /* Update the property file */
  if ( property_set( QCRIL_ECC_LIST, prop_val ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "RID %d MID %d Failed to save ril.ecclist to system property\n", instance_id, modem_id );
  } 
  else if ( property_get( QCRIL_ECC_LIST, prop_val, "" ) > 0 )
  {
    QCRIL_LOG_DEBUG( "RID %d MID %d update ril.ecclist= %s\n", instance_id, modem_id, prop_val );
  }

} /* qcril_pbm_update_ecc_property */


/*===========================================================================
  FUNCTION:  qcril_pbm_event_handler
===========================================================================*/
/*!
    @brief
    Handle the PBM unsolicited Notifications here

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_event_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id, voice_modem_id;
  qcril_pbm_struct_type *i_ptr;
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type voice_radio_tech;
  #else
  qcril_radio_tech_e_type        voice_radio_tech;
  #endif
  const pbm_notify_data_s_type *pNotifyData;
  errno_enum_type ret = E_FAILURE;

  #ifdef FEATURE_QCRIL_DSDS
  qcril_arb_subs_prov_status_e_type subs_state;
  RIL_SelectUiccSub uicc_sub;                   
  sys_modem_as_id_e_type as_id;                
  qmi_uim_session_type session_type;
  #endif /* FEATURE_QCRIL_DSDS */
  
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  pNotifyData = ( pbm_notify_data_s_type *) params_ptr->data;
  QCRIL_ASSERT( pNotifyData != NULL );

  /*-----------------------------------------------------------------------*/

  /* Ignore PBM event when in Airplane Mode, so that ECC number on SIM will not be removed */
  if ( i_ptr->oprt_mode_check_enabled && qcril_arb_in_airplane_mode( instance_id, modem_id ) ) 
  {
    QCRIL_LOG_DEBUG( "RID %d MID %d Modem is in Airplane Mode, ignore PBM event.\n", instance_id, modem_id );
    return;
  }

  #ifdef FEATURE_QCRIL_DSDS
  if ( qcril_arb_ma_is_dsds() )
  {
    /* Lookup subscription info */
    qcril_arb_query_subs( instance_id, &subs_state, &uicc_sub, &as_id, &session_type );
    if ( subs_state == QCRIL_ARB_SUBS_NOT_PROVISIONED )
    {
      QCRIL_LOG_DEBUG( "RID %d MID %d Subscription info not available, ignore PBM event\n", instance_id, modem_id );
      return;
    }

    /* Check slot match */
    /* pbm slot_id start from 1... where as telehpony slot id start from 0.... */
    if ( ( pNotifyData->session_data.pb_id.slot_id - 1 ) != (pbm_slot_type) uicc_sub.slot ) 
    {
      QCRIL_LOG_DEBUG( "RID %d MID %d slot %d, ignore PBM event %d from slot %d\n", 
                       instance_id, modem_id, uicc_sub.slot, pNotifyData->event,  ( pNotifyData->session_data.pb_id.slot_id -1 )); 
      return;
    }
  }
  #endif /* FEATURE_QCRIL_DSDS */

  switch ( pNotifyData->event )
  {
    #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
    case PBM_EVENT_SESSION_INIT_DONE:
      /* pbm is done with initializing session. */
      QCRIL_LOG_DEBUG( "RID %d MID %d PBM_EVENT_SIM_INIT_DONE event\n", instance_id, modem_id );
      /* Get the ECC from PBM and update the property file.*/
      ret = qcril_pbm_get_ecc( instance_id, modem_id, PBM_FIELD_NONE, TRUE ); 
      break;
    #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */

    case PBM_EVENT_SIM_INIT_DONE:
      /* pbm is done with initializing card ecc. */
      QCRIL_LOG_DEBUG( "RID %d MID %d PBM_EVENT_SIM_INIT_DONE event\n", instance_id, modem_id );
      /* Get the ECC from PBM and update the property file.*/
      ret = qcril_pbm_get_ecc( instance_id, modem_id, PBM_FIELD_NONE, TRUE ); 
      break;
    
    case PBM_EVENT_PB_REFRESH_DONE:
      /* pbm finished handling refresh */
      QCRIL_LOG_DEBUG( "RID %d MID %d PBM_EVENT_PB_REFRESH_DONE event\n", instance_id, modem_id );
      if ( pNotifyData->data.pb_id == PBM_ECC )
      {
        /* The refresh is indeed for ECC. Get the ECC numbers from PBM */
        ret = qcril_pbm_get_ecc( instance_id, modem_id, PBM_FIELD_NONE, TRUE );
      }
      break;
    
    case PBM_EVENT_PB_REFRESH_START:
      /* pbm started handling refresh */
      QCRIL_LOG_DEBUG( "RID %d MID %d PBM_EVENT_PB_REFRESH_START event\n", instance_id, modem_id );
      if ( pNotifyData->data.pb_id == PBM_ECC )
      {
        /* The refresh is indeed for ECC. Reset the card ECC list. */
        qcril_pbm_clear_ecc_list( instance_id, modem_id, PBM_FIELD_HARDCODED_ECC );
        ret = E_SUCCESS;
      }
      break;
    
    case PBM_EVENT_PB_READY:
      QCRIL_LOG_DEBUG( "RID %d MID %d PBM_EVENT_PB_READY event for phone book id %d\n", instance_id, modem_id, 
                       pNotifyData->data.pb_id );
      /* Pbm ECC Phone Book Ready */
      if ( ( pNotifyData->data.pb_id == PBM_ECC ) && 
           ( !i_ptr->ecc_info[ modem_id ].card_ecc_is_present ) )
      {
        /* No ECC received from PBM till now. Get all the ECC here.*/
        ret = qcril_pbm_get_ecc( instance_id, modem_id, PBM_FIELD_NONE, TRUE );
      }
      #ifndef FEATURE_QCRIL_UIM_QMI 
      else if ( pNotifyData->data.pb_id == PBM_FDN )
      {
        /* FDN phone book ready. Respond to any waiting SIM IO requests */
        if ( qcril_process_event( instance_id, modem_id, QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE, NULL, 0, 
                                  (RIL_Token) QCRIL_TOKEN_ID_INTERNAL ) != E_SUCCESS )
        {
          QCRIL_LOG_ERROR( "RID %d MID %d Internal QCRIL MMGSDI Event processing Failed for PBM FDN record update!", 
                           instance_id, modem_id );
        }
      }
      #endif /* !FEATURE_QCRIL_UIM_QMI */
      break;

    default:
      QCRIL_LOG_DEBUG( "RID %d MID %d Unsupported PBM Event %d\n", instance_id, modem_id, pNotifyData->event );
      break;
  }

  if ( ret == E_SUCCESS )
  {
    /* Lookup the modem that should provide the voice service */
    if ( qcril_arb_query_voice_srv_modem_id( instance_id, &voice_modem_id, &voice_radio_tech ) == E_SUCCESS )
    {
      if ( voice_modem_id == modem_id )
      {
        /* Update the property file */
        qcril_pbm_update_ecc_property( instance_id, modem_id, TRUE );
      }
    }
  }

} /* qcril_pbm_event_handler */


/*===========================================================================
  FUNCTION:  qcril_pbm_event_card_state_changed
===========================================================================*/
/*!
    @brief
    Handles the internal event triggered because of card inserted/init 
    complete/error/removed/pin1 permanently blocked from MMGSDI.

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_event_card_state_changed
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id, voice_modem_id;
  qcril_pbm_struct_type *i_ptr;
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type voice_radio_tech;
  #else
  qcril_radio_tech_e_type        voice_radio_tech;
  #endif
  uint8 i; 
  int *slot_ptr;
  errno_enum_type ret = E_FAILURE;

  #ifdef FEATURE_QCRIL_DSDS
  qcril_arb_subs_prov_status_e_type subs_state;
  RIL_SelectUiccSub uicc_sub;                   
  sys_modem_as_id_e_type as_id;                
  qmi_uim_session_type session_type; 
  #endif /* FEATURE_QCRIL_DSDS */
  
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  slot_ptr = (int *) params_ptr->data;
  QCRIL_ASSERT( slot_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Ignore PBM event when in Airplane Mode, so that ECC number on SIM will not be removed */
  if ( i_ptr->oprt_mode_check_enabled && qcril_arb_in_airplane_mode( instance_id, modem_id ) ) 
  {
    QCRIL_LOG_INFO( "%s", "Modem is in Airplane Mode, ignore card event.\n" );
    return;
  }

  #ifdef FEATURE_QCRIL_DSDS
  if ( qcril_arb_ma_is_dsds() )
  {
    /* Lookup subscription info */
    qcril_arb_query_subs( instance_id, &subs_state, &uicc_sub, &as_id, &session_type );
    if ( subs_state == QCRIL_ARB_SUBS_NOT_PROVISIONED )
    {
      /* allowing QCRIL_EVT_PBM_CARD_ERROR event when subscription is not available for retrieving  the
           emergency call list */
      if( params_ptr->event_id != QCRIL_EVT_PBM_CARD_ERROR)
      {
        QCRIL_LOG_DEBUG( "RID %d MID %d Subscription info not available, ignore PBM event\n", instance_id, modem_id );
        return;
      }
    }
    /* Check slot match */
    else if ( *slot_ptr != uicc_sub.slot ) 
    {
      QCRIL_LOG_DEBUG( "RID %d MID %d slot %d, ignore PBM event %d from slot %d\n", 
                       instance_id, modem_id, uicc_sub.slot, params_ptr->event_id, *slot_ptr ); 
      return;
    }
  }
  #endif /* FEATURE_QCRIL_DSDS */

  switch( params_ptr->event_id )
  {
    case QCRIL_EVT_PBM_CARD_INSERTED:
      /* Upon reception of card inserted / select AID event from mmgsdi */
      for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
      {
        if ( !i_ptr->ecc_info[ i ].card_ecc_is_present )
        {
          ret = qcril_pbm_get_ecc( instance_id, i, PBM_FIELD_NONE, TRUE ); 
        }
      }
      break;

    case QCRIL_EVT_PBM_CARD_INIT_COMPLETED:
      /* If sim init is not completed yet on PBM, We will wait for PBM_EVENT_SIM_INIT_DONE from PBM */
      QCRIL_LOG_RPC2( modem_id, "pbm_is_init_completed()", "Sim init done on PBM");
      for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
      {
        QCRIL_ASSERT( i < QCRIL_MAX_MODEM_ID );
        if( qcril_pbm_api_funcs[ i ].pbm_is_init_completed_func())
        {
          ret = qcril_pbm_get_ecc( instance_id, i, PBM_FIELD_NONE, TRUE );
        }
      }
      break;

    case QCRIL_EVT_PBM_CARD_ERROR:
      for ( i = 0; i < QCRIL_ARB_MAX_MODEMS; i++ )
      {
        if ( i_ptr->ecc_info[ i ].num_card_hcoded_ecc_entries == 0 )
        {
          /* Could be the case when card is absent during power up. Get both NV, Hardcoded Ecc */
          ret = qcril_pbm_get_ecc( instance_id, i, PBM_FIELD_NONE, TRUE );
        }
        else
        {
          /* Card is present during powerup and nv ecc was read already. Just get the Hardcoded Ecc */
          ret = qcril_pbm_get_ecc( instance_id, i, PBM_FIELD_HARDCODED_ECC, TRUE );
        }

        /* OTA numbers are no longer valid. clear the OTA ECC list */
        qcril_pbm_clear_ecc_list( instance_id, i, PBM_FIELD_NETWORK_ECC );
      }
      break;

    default:
      QCRIL_LOG_ERROR( "RID %d MID %d Invalid Event 0x%x\n", instance_id, modem_id, params_ptr->event_id );
      break;
  }

  if ( ret == E_SUCCESS )
  {
    /* Lookup the modem that should provide the voice service */
    if ( qcril_arb_query_voice_srv_modem_id( instance_id, &voice_modem_id, &voice_radio_tech ) == E_SUCCESS )
    {
      if ( voice_modem_id == modem_id )
      {
        /* Update the property file */
        qcril_pbm_update_ecc_property( instance_id, modem_id, TRUE );
      }
    }
  }

} /* qcril_pbm_event_card_state_changed */


/*===========================================================================
  FUNCTION:  qcril_pbm_event_update_ota_ecc_list
===========================================================================*/
/*!
    @brief
    Handle the internal event to update the ecc list property with 
    Network emergency numbers 

    @return
    None.
*/
/*=========================================================================*/
void qcril_pbm_event_update_ota_ecc_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id, voice_modem_id;
  qcril_pbm_struct_type *i_ptr;
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type voice_radio_tech;
  #else
  qcril_radio_tech_e_type        voice_radio_tech;
  #endif
  const cm_emerg_num_list_s_type *ntwk_ecc_list;
  int i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_pbm[ instance_id ];
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  ntwk_ecc_list = ( cm_emerg_num_list_s_type * ) params_ptr->data;

  /*-----------------------------------------------------------------------*/

  if ( ntwk_ecc_list == NULL )
  {
    QCRIL_LOG_ERROR( "RID %d MID %d OTA ECC list NULL\n", instance_id, modem_id );
    return;
  }

  /* clear the existing network ecc list */
  qcril_pbm_clear_ecc_list( instance_id, modem_id, PBM_FIELD_NETWORK_ECC );

  /* copy the ecc from the event payload. Also check if the list 
     length passed is invalid as the maximum numbers from network 
     can be CM_MAX_EMERGENCY_NUM_COUNT */
  for ( i = 0; ( i < ntwk_ecc_list->num_list_len ) && ( i != CM_MAX_EMERGENCY_NUM_COUNT ); i++ )
  {
    qcril_pbm_update_ecc_cache( instance_id, modem_id, PBM_FIELD_NETWORK_ECC, 
                                (char *) ntwk_ecc_list->num_list[i].num.buf, ntwk_ecc_list->num_list[i].num.len );
  }

  /* Lookup the modem that should provide the service */
  if ( qcril_arb_query_voice_srv_modem_id( instance_id, &voice_modem_id, &voice_radio_tech ) == E_SUCCESS )
  {
    if ( voice_modem_id == modem_id )
    {
      /* Update the property file */
      qcril_pbm_update_ecc_property( instance_id, modem_id, TRUE );
    }
  }

} /* qcril_pbm_event_update_ota_ecc_list */
