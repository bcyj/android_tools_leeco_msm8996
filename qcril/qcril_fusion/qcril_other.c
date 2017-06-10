/*!
  @file
  qcril_other.c

  @brief
  Handles RIL requests for common software functions an any other
  RIL function that doesn't fall in a different (more specific) category

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_other.c#26 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
12/11/09   pg      Added support for read only NV items through OEM_HOOK.
07/22/09   pg      Added support for PRL version.
                   Get MDN value from NV_DIR_NUMBER_PCS_I instead of NV_DIR_NUMBER_I.
                   Added support to return the whole home SID/NID list under FEATURE_NEW_RIL_API. 
06/15/09   nrn     Adding support for NAM programming
05/30/09   pg      Fixed get MDN implementation.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros.
02/09/09   fc      Changed debug messages.
01/26/08   fc      Logged assertion info.
12/04/08   fc      Featurize SND remote API.
                   Fixed the NV READ RPC issue on mismatch data structure.
11/05/08   fc      Added support for RIL_REQUEST_GET_MUTE.
10/06/08   pg      Added support for RIL_REQUEST_CDMA_SUBSCRIPTION.
09/30/08   pg      Check if CDMA subscription is available before reading
                   ESN/MEID.
09/22/08   pg      Do not cache ESN and MEID any more.  They should be read
                   from NV each time since their values are different with
                   different CDMA scriptions.
08/29/08   adb     Added support for SET_MUTE 
08/18/08   pg      Added support for GET BASEBAND_VERSION.
08/04/08   pg      Added support for GET IMEI, IMEISV.
05/22/08   tml     Fixed compilation issue with LTK

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <stdio.h>
#include <string.h>
#include "comdef.h"
#include "auth_rpc.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_reqlist.h"
#include "qcril_cm_clist.h"
#include "qcril_otheri.h"
#include "qcril_other.h"
#include "qcril_other_api_map.h"
#include "qcrilhook_oem.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_OTHER_NV_SO( item )       FSIZ( nv_item_type, item ), FPOS( nv_item_type, item )
#define QCRIL_OTHER_NUM_OF_NV_ITEMS     sizeof( qcril_other_nv_table ) / sizeof( qcril_other_nv_table_entry_type )


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/*! @brief Typedef variables internal to module qcril_other.c
*/
static qcril_other_struct_type *qcril_other;

static qcril_other_nv_table_entry_type qcril_other_nv_table[] =
{
  { NV_MIN1_I,                  QCRIL_OTHER_NV_SO( min1 ),                "NV_MIN1_I" },
  { NV_MIN2_I,                  QCRIL_OTHER_NV_SO( min2 ),                "NV_MIN2_I" },
  { NV_IMSI_11_12_I,            QCRIL_OTHER_NV_SO( imsi_11_12 ),          "NV_IMSI_11_12_I" },
  { NV_DIR_NUMBER_I,            QCRIL_OTHER_NV_SO( dir_number ),          "NV_DIR_NUMBER_I" },
  { NV_SID_NID_I,               QCRIL_OTHER_NV_SO( sid_nid ),             "NV_SID_NID_I" },
  { NV_IMSI_T_S1_I,             QCRIL_OTHER_NV_SO( imsi_t_s1 ),           "NV_IMSI_T_S1_I" },
  { NV_IMSI_T_S2_I,             QCRIL_OTHER_NV_SO( imsi_t_s2 ),           "NV_IMSI_T_S2_I" },
  { NV_IMSI_T_MCC_I,            QCRIL_OTHER_NV_SO( imsi_t_mcc ),          "NV_IMSI_T_MCC_I" },
  { NV_IMSI_T_11_12_I,          QCRIL_OTHER_NV_SO( imsi_t_11_12 ),        "NV_IMSI_T_11_12_I" },
  { NV_IMSI_T_ADDR_NUM_I,       QCRIL_OTHER_NV_SO( imsi_t_addr_num ),     "NV_IMSI_T_ADDR_NUM_I" },
  { NV_IMSI_MCC_I,              QCRIL_OTHER_NV_SO( imsi_mcc ),            "NV_IMSI_MCC_I" },
  { NV_PCDMACH_I,               QCRIL_OTHER_NV_SO( pcdmach ),             "NV_PCDMACH_I" },
  { NV_SCDMACH_I,               QCRIL_OTHER_NV_SO( scdmach ),             "NV_SCDMACH_I" },
  { NV_ECC_LIST_I,              QCRIL_OTHER_NV_SO( ecc_list ),            "NV_ECC_LIST_I" },
  { NV_SEC_CODE_I,              QCRIL_OTHER_NV_SO( sec_code ),            "NV_SEC_CODE_I" },
  { NV_LOCK_CODE_I,             QCRIL_OTHER_NV_SO( lock_code ),           "NV_LOCK_CODE_I" },
  { NV_DS_DEFAULT_BAUDRATE_I,   QCRIL_OTHER_NV_SO( ds_default_baudrate ), "NV_DS_DEFAULT_BAUDRATE_I" },
  { NV_ANALOG_HOME_SID_I,       QCRIL_OTHER_NV_SO( analog_home_sid ),     "NV_ANALOG_HOME_SID_I" },
  { NV_HOME_SID_NID_I,          QCRIL_OTHER_NV_SO( home_sid_nid ),        "NV_HOME_SID_NID_I" },
  { NV_GPS1_PDE_ADDRESS_I,      QCRIL_OTHER_NV_SO( gps1_pde_address ),    "NV_GPS1_PDE_ADDRESS_I" },
  { NV_GPS1_PDE_PORT_I,         QCRIL_OTHER_NV_SO( gps1_pde_port ),       "NV_GPS1_PDE_PORT_I" },
  { NV_PRIMARY_DNS_I,           QCRIL_OTHER_NV_SO( primary_dns ),         "NV_PRIMARY_DNS_I" },
  { NV_SECONDARY_DNS_I,         QCRIL_OTHER_NV_SO( secondary_dns ),       "NV_SECONDARY_DNS_I" },
  { NV_IPV6_PRIMARY_DNS_I,      QCRIL_OTHER_NV_SO( ipv6_primary_dns ),    "NV_IPV6_PRIMARY_DNS_I" },
  { NV_IPV6_SECONDARY_DNS_I,    QCRIL_OTHER_NV_SO( ipv6_secondary_dns ),  "NV_IPV6_SECONDARY_DNS_I" },
  { NV_AIR_CNT_I,               QCRIL_OTHER_NV_SO( air_cnt ),             "NV_AIR_CNT_I" },
  { NV_ROAM_CNT_I,              QCRIL_OTHER_NV_SO( roam_cnt ),            "NV_ROAM_CNT_I" },
  { NV_AUTO_ANSWER_I,           QCRIL_OTHER_NV_SO( auto_answer ),         "NV_AUTO_ANSWER_I" },
  { NV_PREF_VOICE_SO_I,         QCRIL_OTHER_NV_SO( pref_voice_so ),       "NV_PREF_VOICE_SO_I" },
  { NV_SPC_CHANGE_ENABLED_I,    QCRIL_OTHER_NV_SO( spc_change_enabled ),  "NV_SPC_CHANGE_ENABLED_I" },
  { NV_MOB_CAI_REV_I,           QCRIL_OTHER_NV_SO( mob_cai_rev ),         "NV_MOB_CAI_REV_I" }
};


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_other_init

===========================================================================*/
/*!
    @brief
    Initialize the Other subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_init( void )
{
  uint8 i;
  qcril_other_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  /* Allow cache */
  qcril_other = (qcril_other_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_OTHER );
  QCRIL_ASSERT( qcril_other != NULL );

  /* Initialize internal data */
  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_other[ i ];

    /* Initialize uplink mute setting */
    i_ptr->uplink_mute_setting = QCRIL_OTHER_MUTE_ENABLED;

    /* Initialize status of NV items */
    i_ptr->curr_nam = 0;
  }

} /* qcril_other_init() */


/*===========================================================================

  FUNCTION:  qcril_other_mute

===========================================================================*/
/*!
    @brief
    Wrapper function for SND RPC call to set mute levels. 

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_mute
( 
  qcril_instance_id_e_type instance_id,
  boolean mic_mute, 
  boolean ear_mute 
)
{
  qcril_other_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_other[ instance_id ];

  /*-----------------------------------------------------------------------*/

  /* Update uplink mute state */
  if ( mic_mute )
  {
    i_ptr->uplink_mute_setting = QCRIL_OTHER_MUTE_ENABLED;
  }
  else
  {
    i_ptr->uplink_mute_setting = QCRIL_OTHER_MUTE_DISABLED;
  }

} /* qcril_other_mute() */


/*===========================================================================

  FUNCTION:  qcril_other_request_set_mute

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_SET_MUTE. If not in call, we mute all channels.
    If in call, we leave earpiece unmuted, and mute or unmute mic as 
    requested. 

    @return
    None.
*/

/*=========================================================================*/
void qcril_other_request_set_mute
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  int mute_cmd;
  qcril_cm_clist_call_info_list_type call_info_list;
  char *mute_state_str = NULL;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  mute_cmd = * ( (int *) params_ptr->data );

  QCRIL_LOG_INFO( "Handling %s (%d) Token ID (%d) - Mute value %d", 
                  qcril_log_lookup_event_name( params_ptr->event_id ), params_ptr->event_id, 
                  qcril_log_get_token_id( params_ptr->t ),
                  mute_cmd ); 

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  qcril_cm_clist_query_call_info_list( instance_id, &call_info_list );

  if ( call_info_list.num_of_calls == 0 )
  {
    QCRIL_LOG_INFO( "%s", "Not in call - Muting channels" );
    mute_state_str = "Mic (Off), Ear (Off)";
    qcril_other_mute( instance_id, TRUE, TRUE ); 
  }
  else if ( mute_cmd )
  {
    QCRIL_LOG_INFO( "%s", "In call - Muting mic" );
    mute_state_str = "Mic (Off), Ear (On)";
    qcril_other_mute( instance_id, TRUE, FALSE );
  }
  else
  {
    QCRIL_LOG_INFO( "%s", "In call - Unmuting mic" );
    mute_state_str = "Mic (On), Ear (On)";
    qcril_other_mute( instance_id, FALSE, FALSE );
  }

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  qcril_send_request_response( &resp );

} /* qcril_other_request_set_mute() */


/*===========================================================================

  FUNCTION:  qcril_other_request_get_mute

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_MUTE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_get_mute
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  qcril_other_struct_type *i_ptr;
  int uplink_mute_setting;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_other[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "Handling %s (%d) Token ID (%d)", 
                  qcril_log_lookup_event_name( params_ptr->event_id ), params_ptr->event_id, 
                  qcril_log_get_token_id( params_ptr->t ) );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  uplink_mute_setting = i_ptr->uplink_mute_setting;

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  resp.resp_pkt = (void *) &uplink_mute_setting;
  resp.resp_len = sizeof( uplink_mute_setting );
  qcril_send_request_response( &resp );

} /* qcril_other_request_get_mute() */


/*=========================================================================

  FUNCTION:  qcril_other_read_imei_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_UE_IMEI_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.
*/
/*=========================================================================*/
static boolean qcril_other_read_imei_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *imei_rpt_ptr
)
{
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  uint8 i, digit, imei_bcd_len = 0;
  char imei_ascii[ ( NV_UE_IMEI_SIZE - 1 ) * 2 + 1 ];
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( imei_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Default IMEI */
  *imei_rpt_ptr = '0';
  *( imei_rpt_ptr + 1 ) = '\0';

  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_UE_IMEI_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_UE_IMEI_I, ( nv_item_type * ) &nv_item 
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch( nv_status )
  {
    case NV_DONE_S: 
      /* Convert it to ASCII representation of the IMEI in hexadecimal format */
      imei_bcd_len = nv_item.ue_imei.ue_imei[ 0 ];

      /* IMEI not programmed */
      if ( imei_bcd_len == 0 )
      {
        QCRIL_LOG_INFO( "IMEI value 0x%s read from NV\n", imei_rpt_ptr );
      }
      /* Invalid IMEI */
      else if ( imei_bcd_len > ( NV_UE_IMEI_SIZE - 1 ) )
      {
        QCRIL_LOG_INFO( "%s", "Invalid IMEI value from NV\n" );
      }
      /* Valid IMEI */
      else
      {
        memset( imei_ascii, 0, QCRIL_OTHER_IMEI_ASCII_MAX_LEN );

        for( i = 1; i <= imei_bcd_len; i++ )
        {
          QCRIL_LOG_INFO( "IMEI[ i ] = %d read from NV\n", nv_item.ue_imei.ue_imei[ i ] );

          digit = nv_item.ue_imei.ue_imei[ i ] & 0x0F;
          if( ( digit <= 9 ) || ( i <= 1 ) )
          {
            imei_ascii[ ( i - 1 ) * 2 ] = digit + '0';
          }
          else
          {
            imei_ascii[ ( i - 1 ) * 2 ] = '\0';
            break;
          }

          digit = nv_item.ue_imei.ue_imei[ i ] >> 4;
          if ( ( digit <= 9 ) || ( i <= 1 ) )
          {
            imei_ascii[ ( ( i - 1 ) * 2 ) + 1 ] = digit + '0';
            /* Make sure NULL terminated */
            imei_ascii[ ( ( i - 1 ) * 2 ) + 2 ] = '\0';
          }
          else
          {
            imei_ascii[ ( ( i - 1 ) * 2 ) + 1 ] = '\0';
            break;
          }
        } /* end for */

        /* Drop the first byte because it is just the ID */
        memcpy( imei_rpt_ptr, imei_ascii + 1, ( ( NV_UE_IMEI_SIZE - 1 ) * 2 ) );
        QCRIL_LOG_INFO( "IMEI value 0x%s read from NV\n", imei_rpt_ptr );
      }
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "%s", "IMEI not programmed in NV\n" );
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "Problem reading IMEI from NV, status %d\n", nv_status );
      break;
  }

  return status;

} /* qcril_other_read_imei_from_nv() */


/*===========================================================================

  FUNCTION:  qcril_other_request_get_imei

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_IMEI.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_get_imei
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_list;
  qcril_other_imei_type *payload_ptr;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Lookup the modem that should be requested for NV service */ 
  if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp ); 
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem support 3GPP */
  modem_id = modem_ids_list.modem_id[ 0 ];

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if ( ( payload_ptr = ( qcril_other_imei_type * ) qcril_malloc( sizeof( qcril_other_imei_type ) ) ) != NULL )
  {
    if ( qcril_other_read_imei_from_nv( instance_id, modem_id, payload_ptr->imei ) )
    {
      QCRIL_LOG_DEBUG( "Reply to RIL --> IMEI %s\n", payload_ptr->imei ); 
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp ); 
      resp.resp_pkt = (void *)&payload_ptr->imei;
      resp.resp_len = sizeof( payload_ptr->imei );
      qcril_send_request_response( &resp );
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp ); 
      qcril_send_request_response( &resp );
    }

    qcril_free( payload_ptr );
  }

} /* qcril_other_request_get_imei() */


/*=========================================================================

  FUNCTION:  qcril_other_read_imeisv_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_UE_IMEISV_SVN_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.

*/
/*=========================================================================*/
static boolean qcril_other_read_imeisv_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char* imeisv_rpt_ptr
)
{
  int len;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */
 
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( imeisv_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Default IMEISV */
  len = QCRIL_SNPRINTF( imeisv_rpt_ptr, QCRIL_OTHER_IMEISV_ASCII_MAX_LEN, "%02d", 0 );   
  QCRIL_ASSERT( len <= QCRIL_OTHER_IMEISV_ASCII_MAX_LEN );

  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_UE_IMEISV_SVN_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_UE_IMEISV_SVN_I, ( nv_item_type * ) &nv_item 
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch ( nv_status )
  {
    case NV_DONE_S:
      /* IMEISV_SVN cannot be > 98 */    
      if ( nv_item.ue_imeisv_svn < 98 ) 
      {
        /* Convert to ASCII representation */
        len = QCRIL_SNPRINTF( imeisv_rpt_ptr, QCRIL_OTHER_IMEISV_ASCII_MAX_LEN, "%02d", nv_item.ue_imeisv_svn );   
        QCRIL_ASSERT( len <= QCRIL_OTHER_IMEISV_ASCII_MAX_LEN );
        QCRIL_LOG_INFO( "IMEISV value %s read from NV\n", imeisv_rpt_ptr );
      }
      else
      {
        QCRIL_LOG_INFO( "%s", "Invalid IMEISV value from NV\n" );

        /* Default IMEISV value */
        len = QCRIL_SNPRINTF( imeisv_rpt_ptr, QCRIL_OTHER_IMEISV_ASCII_MAX_LEN, "%02d", 0 );   
        QCRIL_ASSERT( len <= QCRIL_OTHER_IMEISV_ASCII_MAX_LEN );
      }
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "%s", "IMEISV not programmed in NV\n" );
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "Problem reading IMEISV from NV, status %d\n", nv_status );
      break;
  }

  return status;

} /* qcril_other_read_imeisv_from_nv() */


/*===========================================================================

  FUNCTION:  qcril_other_request_get_imeisv

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_IMEISV.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_get_imeisv
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_list;
  qcril_other_imeisv_type *payload_ptr;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Lookup the modem that should be requested for NV service */ 
  if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_3GPP, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp ); 
    qcril_send_request_response( &resp );
    return;
  }

  /* Only one modem support 3GPP */
  modem_id = modem_ids_list.modem_id[ 0 ];

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if ( ( payload_ptr = ( qcril_other_imeisv_type * ) qcril_malloc( sizeof( qcril_other_imeisv_type ) ) ) != NULL )
  {
   if ( qcril_other_read_imeisv_from_nv( instance_id, modem_id, payload_ptr->imeisv ) )
   {
     qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp ); 
     resp.resp_pkt = (void *)&payload_ptr->imeisv;
     resp.resp_len = sizeof( payload_ptr->imeisv );
     qcril_send_request_response( &resp );
   }
   else
   {
     qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp ); 
     qcril_send_request_response( &resp );
   }

   qcril_free( payload_ptr );
  }

} /* qcril_other_request_get_imeisv() */


/*=========================================================================

  FUNCTION:  qcril_other_read_baseband_version_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_SW_VERSION_INFO_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.

*/
/*=========================================================================*/
static boolean qcril_other_read_baseband_version_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *baseband_version_rpt_ptr
)
{
  int len;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */
 
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( baseband_version_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Default SW version info */
  *baseband_version_rpt_ptr = '\0';

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Read data from NV */
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_SW_VERSION_INFO_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_SW_VERSION_INFO_I, ( nv_item_type * ) &nv_item 
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch( nv_status )
  {
    case NV_DONE_S:
      /* Compose the response */
      len = QCRIL_SNPRINTF( baseband_version_rpt_ptr, QCRIL_OTHER_BASEBAND_VERSION_ASCII_MAX_LEN, "%s", nv_item.sw_version_info );   
      QCRIL_ASSERT( len <= QCRIL_OTHER_BASEBAND_VERSION_ASCII_MAX_LEN );
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "%s", "BASEBAND_VERSION not programmed in NV\n" );
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "%s", "Problem reading BASEBAND_VERSION from NV\n" );
      break;
  }

  return status;

} /* qcril_other_read_baseband_version_from_nv() */


/*===========================================================================

  FUNCTION:  qcril_other_request_baseband_version

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_BASEBAND_VERSION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_baseband_version
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_list;
  qcril_other_baseband_version_type *payload_ptr;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Lookup the modem that should be requested for NV service */ 
  if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_COMMON, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( & resp );
    return;
  }

  /* In case of global device for split modem architecture, OEM is required to make sure the NV item values on both modems 
     are in sync. So read from one modem is sufficient */
  modem_id = modem_ids_list.modem_id[ 0 ];

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if ( ( payload_ptr = ( qcril_other_baseband_version_type * ) qcril_malloc( sizeof( qcril_other_baseband_version_type ) ) ) != NULL )
  {
    if ( qcril_other_read_baseband_version_from_nv( instance_id, modem_id, payload_ptr->baseband_version ) )
    {
      QCRIL_LOG_DEBUG( "Reply to RIL --> Baseband Version : %s\n", payload_ptr->baseband_version ); 
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = (void *)&payload_ptr->baseband_version; 
      resp.resp_len = sizeof( payload_ptr->baseband_version );
      qcril_send_request_response( & resp );
    }
    else
    {
      QCRIL_LOG_INFO( "%s", "Problem reading BASEBAND_VERSION from NV\n" );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( & resp );
    }

    qcril_free( payload_ptr );
  }

} /* qcril_other_request_baseband_version() */


/*=========================================================================

  FUNCTION:  qcril_other_read_esn_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_ESN_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.
*/
/*=========================================================================*/
static boolean qcril_other_read_esn_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *esn_rpt_ptr
)
{
  int len;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */
 
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( esn_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_ESN_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_ESN_I, (nv_item_type *) &nv_item 
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch ( nv_status )
  {
    case NV_DONE_S:
      /* Convert it to ASCII representation of the ESN in hexadecimal format */
      len = QCRIL_SNPRINTF( esn_rpt_ptr, QCRIL_OTHER_ESN_ASCII_MAX_LEN, "%08lx", nv_item.esn.esn  ); 
      QCRIL_ASSERT( len <= QCRIL_OTHER_ESN_ASCII_MAX_LEN );
      QCRIL_LOG_INFO( "ESN value 0x%s read from NV\n", esn_rpt_ptr );
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "%s", "ESN not programmed in NV\n" );
      memset( &nv_item.esn, 0, sizeof( nv_item.esn ) );
      len = QCRIL_SNPRINTF( esn_rpt_ptr, QCRIL_OTHER_ESN_ASCII_MAX_LEN, "%08lx", (long unsigned int) nv_item.esn.esn ); 
      QCRIL_ASSERT( len <= QCRIL_OTHER_ESN_ASCII_MAX_LEN );
      status = TRUE;
      break;
    
    default:
      QCRIL_LOG_INFO( "Problem reading ESN from NV, status %d\n", nv_status );
      break;
  }

  return status;

} /* qcril_other_get_read_esn_from_nv() */


/*=========================================================================

  FUNCTION:  qcril_other_read_meid_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_MEID_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.
*/
/*=========================================================================*/
static boolean qcril_other_read_meid_from_nv
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *meid_rpt_ptr
)
{
  int len;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( meid_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_MEID_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_MEID_I, (nv_item_type *) &nv_item
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch( nv_status )
  {
    case NV_DONE_S:
      /* Convert it to ASCII representation of the MEID in hexadecimal format */
      len = QCRIL_SNPRINTF( meid_rpt_ptr, QCRIL_OTHER_MEID_ASCII_MAX_LEN, "%06lx%08lx", nv_item.meid[ 1 ], nv_item.meid[ 0 ] ); 
      QCRIL_ASSERT( len <= QCRIL_OTHER_MEID_ASCII_MAX_LEN );
      QCRIL_LOG_INFO( "MEID value 0x%s read from NV\n", meid_rpt_ptr );
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "%s", "MEID not programmed in NV\n" );
      memset( &nv_item.meid, 0, sizeof( nv_item.meid ) );
      len = QCRIL_SNPRINTF( meid_rpt_ptr, QCRIL_OTHER_MEID_ASCII_MAX_LEN, "%06lx%08lx", nv_item.meid[ 1 ], nv_item.meid[ 0 ] ); 
      QCRIL_ASSERT( len <= QCRIL_OTHER_MEID_ASCII_MAX_LEN );
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "Problem reading MEID from NV, status\n", nv_status );
      break;
  }

  return status;

} /* qcril_other_read_meid_from_nv */


/*===========================================================================

  FUNCTION:  qcril_other_request_device_identity

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DEVICE_IDENTITY.

    @return
    E_SUCCESS if  
*/
/*=========================================================================*/
void qcril_other_request_device_identity
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_list;
  qcril_other_device_identity_type *payload_ptr;
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  if ( ( payload_ptr = ( qcril_other_device_identity_type * ) qcril_malloc( sizeof( qcril_other_device_identity_type ) ) ) != NULL )
  {
    payload_ptr->device_identity[ 0 ] = NULL;
    payload_ptr->device_identity[ 1 ] = NULL;
    payload_ptr->device_identity[ 2 ] = NULL;
    payload_ptr->device_identity[ 3 ] = NULL;

    /* Lookup the modem that should be requested for NV service */ 
    if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_3GPP, instance_id, &modem_ids_list ) == E_SUCCESS )
    {
      /* Only one modem is programmed to support 3GPP */
      modem_id = modem_ids_list.modem_id[ 0 ];

      if ( qcril_other_read_imei_from_nv( instance_id, modem_id, payload_ptr->imei ) )
      {
        payload_ptr->device_identity[ 0 ] = payload_ptr->imei;
      }

      if ( qcril_other_read_imeisv_from_nv( instance_id, modem_id, payload_ptr->imeisv ) )
      {
        payload_ptr->device_identity[ 1 ] = payload_ptr->imeisv;
      }
    }

    /* Lookup the modem that should be requested for NV service */ 
    if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_3GPP2, instance_id, &modem_ids_list ) == E_SUCCESS )
    {
      /* In case of global device for split modem architecture, both modems are required to sync up on NV items by OEM. 
         Only read from one modem is sufficient */
      modem_id = modem_ids_list.modem_id[ 0 ];

      if ( qcril_other_read_esn_from_nv( instance_id, modem_id, payload_ptr->esn ) )
      { 
        payload_ptr->device_identity[ 2 ] = payload_ptr->esn;
      }

      if ( qcril_other_read_meid_from_nv( instance_id, modem_id, payload_ptr->meid ) )
      {
        payload_ptr->device_identity[ 3 ] = payload_ptr->meid;
      }
    }

    QCRIL_LOG_DEBUG( "Reply to RIL -->  IMEI %s, IMEISV %s, ESN %s, MEID %s\n", 
                     payload_ptr->device_identity[ 0 ], payload_ptr->device_identity[ 1 ], payload_ptr->device_identity[ 2 ], 
                     payload_ptr->device_identity[ 3 ] ); 
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = ( void * ) payload_ptr->device_identity; 
    resp.resp_len = sizeof( payload_ptr->device_identity );
    qcril_send_request_response( &resp );

    qcril_free( payload_ptr );
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_other_request_device_identity() */


/*=========================================================================

  FUNCTION:  qcril_other_read_mdn_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_DIR_NUMBER_PCS_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_other_read_mdn_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *mdn_rpt_ptr
)
{
  qcril_other_struct_type *i_ptr;
  int i, len;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( mdn_rpt_ptr != NULL );
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_other[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Default Mobile Director Number */
  memset( mdn_rpt_ptr, 0, QCRIL_OTHER_MDN_ASCII_MAX_LEN );
  
  nv_item.mob_dir_number.nam = i_ptr->curr_nam;
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_DIR_NUMBER_PCS_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_DIR_NUMBER_PCS_I, (nv_item_type *) &nv_item 
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch( nv_status )
  {
    case NV_DONE_S:
      /* Convert it to ASCII representation of the MDN */
      len = QCRIL_SNPRINTF( mdn_rpt_ptr, nv_item.mob_dir_number.n_digits + 1, "%s", nv_item.mob_dir_number.digitn ); 

      /* Convert HEX to character */
      for( i = 0; i < nv_item.mob_dir_number.n_digits; i++ )
      {
        if ( mdn_rpt_ptr[ i ] == 10 )
        {
          mdn_rpt_ptr[ i ] = '0';
        }
        else
        {
          mdn_rpt_ptr[ i ] =  mdn_rpt_ptr[ i ] + '0';
        }
      }

      mdn_rpt_ptr[ nv_item.mob_dir_number.n_digits ] = '\0';
      QCRIL_LOG_INFO( "MDN value %s read from NV\n", mdn_rpt_ptr );
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "MDN not programmed in NV, use default MDN %s\n", mdn_rpt_ptr );
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "Problem reading MDN from NV, status %d\n", nv_status );
      break;
  }

  return status;

} /* qcril_other_read_mdn_from_nv */


/*=========================================================================

  FUNCTION:  qcril_other_read_home_sid_nid_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_HOME_SID_NID_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_other_read_home_sid_nid_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *sid_rpt_ptr,
  char *nid_rpt_ptr
)
{
  qcril_other_struct_type *i_ptr;
  int i;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_other[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( sid_rpt_ptr != NULL );
  QCRIL_ASSERT( nid_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  nv_item.home_sid_nid.nam = i_ptr->curr_nam;
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_HOME_SID_NID_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_HOME_SID_NID_I, (nv_item_type *) &nv_item
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  switch( nv_status )
  {
    case NV_DONE_S:
      status = TRUE;
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "Home SID/NID not programmed in NV, use default SID/NID values %s/%s\n", sid_rpt_ptr, nid_rpt_ptr );
      for ( i = 0; i < NV_MAX_HOME_SID_NID; i++ )
      {
        nv_item.home_sid_nid.pair[ i ].sid = 0;   /* Initialized to PRL_WILDCARD_SID (0) */
        nv_item.home_sid_nid.pair[ i ].nid = 65535;  /* Initialized to PRL_WILDCARD_NID (65535) */
      }
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "Problem reading Home SID/NID from NV, status %d\n", nv_status );
      break;
  }

  if ( status )
  {
    /* Convert it to ASCII representation of the SID and NID */
    QCRIL_SNPRINTF( sid_rpt_ptr, QCRIL_OTHER_SID_NID_LIST_ASCII_MAX_LEN, 
                    "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
                    nv_item.home_sid_nid.pair[ 0 ].sid, nv_item.home_sid_nid.pair[ 1 ].sid,
                    nv_item.home_sid_nid.pair[ 2 ].sid, nv_item.home_sid_nid.pair[ 3 ].sid,
                    nv_item.home_sid_nid.pair[ 4 ].sid, nv_item.home_sid_nid.pair[ 5 ].sid,
                    nv_item.home_sid_nid.pair[ 6 ].sid, nv_item.home_sid_nid.pair[ 7 ].sid,
                    nv_item.home_sid_nid.pair[ 8 ].sid, nv_item.home_sid_nid.pair[ 9 ].sid,
                    nv_item.home_sid_nid.pair[ 10 ].sid, nv_item.home_sid_nid.pair[ 11 ].sid,
                    nv_item.home_sid_nid.pair[ 12 ].sid, nv_item.home_sid_nid.pair[ 13 ].sid,
                    nv_item.home_sid_nid.pair[ 14 ].sid, nv_item.home_sid_nid.pair[ 15 ].sid,
                    nv_item.home_sid_nid.pair[ 16 ].sid, nv_item.home_sid_nid.pair[ 17 ].sid,
                    nv_item.home_sid_nid.pair[ 18 ].sid, nv_item.home_sid_nid.pair[ 19 ].sid );

    QCRIL_SNPRINTF( nid_rpt_ptr, QCRIL_OTHER_SID_NID_LIST_ASCII_MAX_LEN, 
                    "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
                    nv_item.home_sid_nid.pair[ 0 ].nid, nv_item.home_sid_nid.pair[ 1 ].nid,
                    nv_item.home_sid_nid.pair[ 2 ].nid, nv_item.home_sid_nid.pair[ 3 ].nid,
                    nv_item.home_sid_nid.pair[ 4 ].nid, nv_item.home_sid_nid.pair[ 5 ].nid,
                    nv_item.home_sid_nid.pair[ 6 ].nid, nv_item.home_sid_nid.pair[ 7 ].nid,
                    nv_item.home_sid_nid.pair[ 8 ].nid, nv_item.home_sid_nid.pair[ 9 ].nid,
                    nv_item.home_sid_nid.pair[ 10 ].nid, nv_item.home_sid_nid.pair[ 11 ].nid,
                    nv_item.home_sid_nid.pair[ 12 ].nid, nv_item.home_sid_nid.pair[ 13 ].nid,
                    nv_item.home_sid_nid.pair[ 14 ].nid, nv_item.home_sid_nid.pair[ 15 ].nid,
                    nv_item.home_sid_nid.pair[ 16 ].nid, nv_item.home_sid_nid.pair[ 17 ].nid,
                    nv_item.home_sid_nid.pair[ 18 ].nid, nv_item.home_sid_nid.pair[ 19 ].nid );

    QCRIL_LOG_INFO( "Home SID %s read from NV\n", sid_rpt_ptr );
    QCRIL_LOG_INFO( "Home NID %s read from NV\n", nid_rpt_ptr );
  }

  return status;

} /* qcril_other_read_home_sid_nid_from_nv */


/*===========================================================================*/
/*!
    @brief
    Read NV_MIN1_I and NV_MIN2_I from NV.

    @return
    TRUE if nv_read returns NV_DONE_S or NV_NONACTIVE_S.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_other_read_min_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  char *min_rpt_ptr
)
{
  qcril_other_struct_type *i_ptr;
  nv_item_type nv_item1, nv_item2;
  nv_stat_enum_type nv_status1 = NV_FAIL_S, nv_status2 = NV_FAIL_S;
  word temp;
  dword value;
  char min_ascii[ QCRIL_OTHER_MIN_ASCII_MAX_LEN ];
  int len;     
  boolean status = FALSE;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */


  /* translate from min to digit - '0' is 10 */
  char mintable[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_other[ instance_id ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( min_rpt_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return status;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Read MIN1 from NV */
  nv_item1.min1.nam = i_ptr->curr_nam;
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_MIN1_I" );
  nv_status1 = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_MIN1_I, ( nv_item_type * ) &nv_item1 
                                                                     #ifdef FEATURE_QCRIL_DSDS
                                                                     , as_id
                                                                     #endif /* FEATURE_QCRIL_DSDS */
                                                                   );

  switch( nv_status1 )
  {
    case NV_DONE_S:
      QCRIL_LOG_INFO( "MIN1 value %lx read from NV\n", (long unsigned int) nv_item1.min1.min1[ 1 ] );

      /* Read MIN2 from NV */
      QCRIL_LOG_RPC2A( modem_id, "nv_cmd_remote()", "Read NV_MIN2_I" );
      nv_item2.min2.nam = i_ptr->curr_nam;
      nv_status2 = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_MIN2_I, ( nv_item_type * ) &nv_item2
                                                                         #ifdef FEATURE_QCRIL_DSDS
                                                                         , as_id
                                                                         #endif /* FEATURE_QCRIL_DSDS */
                                                                       );

      switch ( nv_status2 )
      {
        case NV_DONE_S:
          QCRIL_LOG_INFO( "MIN2 value %x read from NV\n", (unsigned int) nv_item2.min2.min2[ 1 ] );

          /* Convert MIN_S to ASCII form. */
          value = nv_item2.min2.min2[ 1 ];
          min_ascii[ 0 ] = mintable[ (value / 100 ) % 10 ];
          value %= 100;
          min_ascii[ 1 ] = mintable[ value / 10 ];
          min_ascii[ 2 ] = mintable[ value % 10 ];

          value = nv_item1.min1.min1[ 1 ];
          temp = (word) ( value >> 14 );
          min_ascii[ 3 ] = mintable[ ( temp / 100) % 10 ];

          temp %= 100;
          min_ascii[ 4 ] = mintable[ temp / 10 ];
          min_ascii[ 5 ] = mintable[ temp % 10 ];
          value &= 0x3FFFL; /* get bottom 14 bits */

          /* next digit is top 4 bits */
          temp = (word) (( value >> 10 ) & 0xF );
          min_ascii[ 6 ] = (char) ( ( ( temp == 10 ) ? 0 : temp ) + '0' );
          temp = (word) ( value & 0x3FF ); /* get bottom 10 bits */
          min_ascii[ 7 ] = mintable[ (temp / 100) %10];
          temp %= 100;
          min_ascii[ 8 ] = mintable[ temp / 10 ];
          min_ascii[ 9 ] = mintable[ temp % 10 ];

          /* NULL terminate the string */
          min_ascii[ 10 ] = '\0';

          len = QCRIL_SNPRINTF( min_rpt_ptr, QCRIL_OTHER_MIN_ASCII_MAX_LEN, "%s", min_ascii );
          QCRIL_ASSERT( len <= QCRIL_OTHER_MIN_ASCII_MAX_LEN );
          QCRIL_LOG_INFO( "MIN value %s read from NV\n", min_rpt_ptr );
          status = TRUE;
          break;

        default:
          QCRIL_LOG_INFO( "%s", "Invalid MIN2 value from NV\n" );
          break;

      } /* end switch */
      break;

    case NV_NOTACTIVE_S:
      QCRIL_LOG_INFO( "%s", "MIN1 not programmed in NV\n" );
      status = TRUE;
      break;

    default:
      QCRIL_LOG_INFO( "Problem reading MIN1 from NV, status %d\n", nv_status1 );
      break;
  }

  return status;

} /* qcril_other_read_min_from_nv() */


/*===========================================================================

  FUNCTION:  qcril_other_request_validate_akey

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY.

    @return
    E_SUCCESS   
*/
/*=========================================================================*/
void qcril_other_request_cdma_validate_and_write_akey
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  boolean request_is_valid = TRUE;
  char *akey_ptr;
  char akey_tmp[ QCRIL_OTHER_AKEY_ASCII_MAX_LEN + 1 ];
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry; 
  
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  akey_ptr = ( char * ) params_ptr->data;
  QCRIL_ASSERT( akey_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Verify the parameters are valid */
  if ( ( strlen( params_ptr->data ) == 0 ) || ( strlen( params_ptr->data ) != QCRIL_OTHER_AKEY_ASCII_MAX_LEN ) )
  {
    QCRIL_LOG_ERROR( "Invalid AKEY length :(%d)\n", strlen( params_ptr->data ) );
    request_is_valid = FALSE;
  }

  if ( !auth_null() )                                           
  {       
    QCRIL_LOG_ERROR( "%s", "AUTH RPC calls are not working!\n");
    request_is_valid = FALSE;
  }

  /* Convert AKey string to 64-bit numeric value */
  if ( !request_is_valid )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Lookup the modem that should be requested for NV service */ 
  if ( qcril_arb_query_auth_srv_modem_id( instance_id, &modem_id ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  memset( akey_tmp, 0, QCRIL_OTHER_AKEY_ASCII_MAX_LEN + 1 );
  memcpy( akey_tmp, akey_ptr, QCRIL_OTHER_AKEY_ASCII_MAX_LEN );
  akey_tmp[ QCRIL_OTHER_AKEY_ASCII_MAX_LEN ] = '\0';

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  QCRIL_LOG_RPC2A( modem_id, "auth_validate_a_key()", akey_tmp );
  if ( qcril_other_api_funcs[ modem_id ].auth_validate_a_key_func( (byte *) akey_ptr ) )
  {
    QCRIL_LOG_RPC2A( modem_id, "auth_send_update_a_key_cmd()", akey_tmp );
    if ( qcril_other_api_funcs[ modem_id ].auth_send_update_a_key_cmd_func( (byte *) akey_ptr, QCRIL_OTHER_AKEY_ASCII_MAX_LEN, 0 ) )
    {
      QCRIL_LOG_DEBUG( "%s", "AKEY write command successfully given to modem\n" );
    }
    else
    {
      QCRIL_LOG_ERROR( "%s", "AKEY write command could not be given to modem\n" );
      request_is_valid = FALSE;
    }
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "Provided AKEY was not valid\n" );
    request_is_valid = FALSE;
  }

  /* Check to see whether specified AKey matches the NV AKey */
  if ( request_is_valid )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_other_request_cdma_validate_and_write_akey() */


/*===========================================================================

  FUNCTION:  qcril_other_request_oem_hook_strings

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_OEM_HOOK_STRINGS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_strings
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( & resp );

} /* qcril_other_request_oem_hook_strings() */


/*=========================================================================

  FUNCTION:  qcril_other_cellular_sys_is_a_channel

===========================================================================*/
/*!
    @brief
     Check if the CDMA channel is with Cellular Band System A.

    @return
     Boolean
*/
/*=========================================================================*/
static boolean qcril_other_cellular_sys_is_a_channel
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  word cdma_channel                        /* The channel to examine */
)
{
  boolean response = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  if ( qcril_arb_jpn_band_is_supported( instance_id, modem_id ) )
  {
    /* According to T53 Section 6.1.1.1, the CDMA channel must be even, and
     * be in the range of 51-749, 851-989, or 1091-1149. */
    if ( ( ( cdma_channel & 0x0001 ) == 0 ) &&
         ( ( ( cdma_channel > 51 ) && ( cdma_channel <  749 ) )   ||
           ( ( cdma_channel > 851 )  && ( cdma_channel <  989 ) ) ||
           ( ( cdma_channel > 1091 ) && ( cdma_channel < 1149 ) ) ) )
    {
      response = TRUE;
    }
   }
   else
   {
     response = QCRIL_OTHER_PRL_IS_IN_CHAN_CELLULAR_SYS_A( cdma_channel ); /* Sys A */
   }

   return response;

} /* end of qcril_other_cellular_sys_is_a_channel */


/*=========================================================================

  FUNCTION:  qcril_other_cellular_sys_is_b_channel

===========================================================================*/
/*!
    @brief
      Check if the CDMA channel is with Cellular Band System B

    @return
      Boolean
*/
/*=========================================================================*/
static boolean qcril_other_cellular_sys_is_b_channel
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  word cdma_channel                        /* The channel to examine */
)
{
  boolean response = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  if ( qcril_arb_jpn_band_is_supported( instance_id, modem_id ) )
  {
    response = QCRIL_OTHER_PRL_IS_IN_CHAN_CELLULAR_SYS_B( cdma_channel );
  }

  return response;

} /* end of qcril_other_cellular_sys_is_b_channel */


/*=========================================================================

  FUNCTION:  qcril_other_nv_item_data_is_valid

===========================================================================*/
/*!
    @brief
      Validates the NAM/NV data of a particular item

    @return
      Boolean
*/
/*=========================================================================*/
static boolean qcril_other_nv_item_data_is_valid
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  nv_items_enum_type nv_item,
  nv_item_type *nv_item_data
)
{
  int j = 0, sid = 0;
  boolean nv_data_is_valid = TRUE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( nv_item_data != NULL );
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  switch( nv_item )
  {
    case NV_MIN1_I:
      for ( j = 0; j < NV_MAX_MINS; j++ )
      {
        if ( ( ( ( nv_item_data->min1.min1[ j ] & 0xFF000000 ) != 0 ) ||
               ( ( ( nv_item_data->min1.min1[ j ] & 0x00FFC000 ) >> 14 ) > 999 ) ||
               ( ( ( nv_item_data->min1.min1[ j ] & 0x00003C00 ) >> 10 ) > 10 ) ||
               ( ( ( nv_item_data->min1.min1[ j ] & 0x00003C00 ) >> 10 ) == 0 ) ||
               ( ( ( nv_item_data->min1.min1[ j ] & 0x000003FF ) > 999 ) ) ) ||
             ( nv_item_data->min1.nam != 0 ) )
        {
          /* Invalid Range See IS-95A section 6.3.1 */
          nv_data_is_valid = FALSE;
          break;
        }
      }
      break;

    case NV_MIN2_I:
      if ( nv_item_data->min2.nam == 0 )
      {
        /* Loop through each MIN */
        for ( j = 0; j < NV_MAX_MINS; j++ )
        {
          if ( nv_item_data->min2.min2[ j ] > QCRIL_OTHER_IMSI_S2_0 )
          {
            /* invalid range */
            nv_data_is_valid = FALSE;
            break;
          }
        }
      }
      else
      {
        nv_data_is_valid = FALSE;
      }
      break;

    case NV_IMSI_11_12_I:
      if ( ( nv_item_data->imsi_11_12.nam != 0 ) ||
           ( nv_item_data->imsi_11_12.imsi_11_12 > QCRIL_OTHER_IMSI_11_12_0 ) )
      {
        nv_data_is_valid = FALSE;
      }
      break;

    case NV_DIR_NUMBER_I:
      if ( nv_item_data->dir_number.nam != 0 )
      {
        nv_data_is_valid = FALSE;
      }
      break;

   case NV_SID_NID_I:
     if ( ( nv_item_data->sid_nid.nam != 0 ) ||
          ( nv_item_data->sid_nid.pair[ NV_CDMA_MIN_INDEX ][ 0 ].sid & 0x8000 ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_A_KEY_I:
     if ( nv_item_data->a_key.nam != 0 )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_IMSI_T_S1_I:
     if ( ( ( ( nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_CDMAMIN ] & 0xFF000000 ) != 0 ) ||
            ( ( ( nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_CDMAMIN ] & 0x00FFC000 ) >> 14 ) > 999 ) ||
              ( ( ( nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_CDMAMIN ] & 0x00003C00 ) >> 10 ) > 10 ) ||
                ( ( ( nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_CDMAMIN ] & 0x00003C00 ) >> 10 ) == 0 ) ||
                ( ( ( nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_CDMAMIN ] & 0x000003FF ) > 999 ) ) )  ||
            ( nv_item_data->imsi_t_s1.nam != 0 ) )
     {
       /* Invalid Range See IS-95A section 6.3.1 */
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_IMSI_T_S2_I:
     if ( ( nv_item_data->imsi_t_s2.min2[ QCRIL_OTHER_CDMAMIN ] > QCRIL_OTHER_IMSI_S2_0 ) ||
          ( nv_item_data->imsi_t_s2.nam != 0 ) )
     {
       /* Since it is valid for only CDMA, only CDMA MIN is checked. FMMIN is ignored */
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_IMSI_T_MCC_I:
     if ( ( nv_item_data->imsi_t_mcc.imsi_mcc > QCRIL_OTHER_IMSI_MCC_0 ) ||
          ( nv_item_data->imsi_t_mcc.nam != 0 ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_IMSI_T_11_12_I:
     if ( ( nv_item_data->imsi_t_11_12.imsi_11_12 > QCRIL_OTHER_IMSI_11_12_0 ) ||
          ( nv_item_data->imsi_t_11_12.nam != 0 ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_IMSI_T_ADDR_NUM_I:
     if ( ( nv_item_data->imsi_t_addr_num.num > 7 ) &&
          ( nv_item_data->imsi_t_addr_num.num != 0xFF ) &&
          ( nv_item_data->imsi_t_addr_num.nam != 0 ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_IMSI_MCC_I:
     if ( ( nv_item_data->imsi_mcc.imsi_mcc > QCRIL_OTHER_IMSI_MCC_0 ) ||
          ( nv_item_data->imsi_mcc.nam != 0) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_PCDMACH_I:
     if ( ( !qcril_other_cellular_sys_is_a_channel( instance_id, modem_id, nv_item_data->pcdmach.channel_a ) ) ||
          ( !qcril_other_cellular_sys_is_b_channel( instance_id, modem_id, nv_item_data->pcdmach.channel_b ) ) ||
          ( nv_item_data->pcdmach.nam != 0 ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_SCDMACH_I:
     if ( ( !qcril_other_cellular_sys_is_a_channel( instance_id, modem_id, nv_item_data->scdmach.channel_a ) ) ||
          ( !qcril_other_cellular_sys_is_b_channel( instance_id, modem_id, nv_item_data->scdmach.channel_b ) ) ||
          ( nv_item_data->scdmach.nam != 0 ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_PREF_VOICE_SO_I:
     if ( nv_item_data->pref_voice_so.nam != 0 )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_ANALOG_HOME_SID_I:
     if ( ( nv_item_data->analog_home_sid.sid & 0x8000 ) ||
          ( nv_item_data->analog_home_sid.nam != 0 ) )
     {
       /*  invalid range ( i.e > 32767) */
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_SPC_CHANGE_ENABLED_I:
     /* nothing to do for these items */
     break;

   case NV_HOME_SID_NID_I:
     if ( nv_item_data->home_sid_nid.nam == 0 )
     {
       for ( sid = 0; sid < NV_MAX_HOME_SID_NID; sid++ )
       {
         /* Is range valid ? */
         if ( nv_item_data->home_sid_nid.pair[sid].sid & 0x8000 )
         {
           nv_data_is_valid = FALSE;
           break;
         }
       }
     }
     else
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_AUTO_ANSWER_I:
     if ( !( ( nv_item_data->auto_answer.enable == 0 ) || ( nv_item_data->auto_answer.enable == 1 ) ) )
     {
       nv_data_is_valid = FALSE;
     }
     break;

   case NV_GPS1_PDE_ADDRESS_I:
   case NV_GPS1_PDE_PORT_I:
   case NV_MOB_CAI_REV_I:
   case NV_ECC_LIST_I:
   case NV_SEC_CODE_I:
   case NV_LOCK_CODE_I:
   case NV_DS_DEFAULT_BAUDRATE_I:
   case NV_PRIMARY_DNS_I:
   case NV_SECONDARY_DNS_I:
   case NV_IPV6_PRIMARY_DNS_I:
   case NV_IPV6_SECONDARY_DNS_I:
   case NV_AIR_CNT_I:
   case NV_ROAM_CNT_I:
     /* nothing to do for these items */
     break;

   default:
     QCRIL_LOG_DEBUG( "Received invalid nv item = %lu\n",  (uint32) nv_item );

  } /* switch */

  return nv_data_is_valid;

} /* qcril_other_nv_item_data_is_valid */


/*=========================================================================

  FUNCTION:  qcril_other_request_oem_hook_nv_write

===========================================================================*/
/*!
    @brief
    Writes the requested NAM parameter to NV item after validating the data.

    @return
    Void
    NV Write status is returned back in nv_write_status parameter.
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_nv_write
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint8 i, nv;
  qcril_modem_ids_list_type modem_ids_list;
  char *data;
  nv_item_type nv_item_data, *nv_item_data_ptr = &nv_item_data;
  uint32 nv_item_length = 0, nv_item = 0, index;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  char details[ 30 ];
  qcril_request_resp_params_type resp;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  data = (char *) params_ptr->data;
  QCRIL_ASSERT( data != NULL );

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

  /* Initializing the data which is not yet initialized */
  memset( &nv_item_data, 0, sizeof( nv_item_data ) );
  index = 0;

  /* decode the NV item from the raw stream, data[0--3], 4 bytes */
  memcpy( &nv_item, &data[ index ], QCRIL_OTHER_OEM_ITEMID_LEN );
  index += (int) QCRIL_OTHER_OEM_ITEMID_LEN;

  /* Decode the NV item size from the raw stream, data[4--7], 4 bytes */
  memcpy( &nv_item_length, &data[ index ], QCRIL_OTHER_OEM_ITEMID_DATA_LEN );
  index += (int) QCRIL_OTHER_OEM_ITEMID_DATA_LEN;

  QCRIL_LOG_DEBUG( "Received request for writing nv_item = %lu\n", nv_item );

  /* Get the index of nv_item in nv data */
  nv = 0;
  while ( ( nv < ( QCRIL_OTHER_NUM_OF_NV_ITEMS ) ) && ( nv_item != qcril_other_nv_table[ nv ].nv_item ) )
  {
     nv++;
  }

  /* Check if the requested NV item was found in the nv table */
  if( nv == ( QCRIL_OTHER_NUM_OF_NV_ITEMS ) )
  {
    QCRIL_LOG_DEBUG( "Requested NV item not found = %lu\n", nv_item );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Validate the NV item length */
  if ( nv_item_length != qcril_other_nv_table[ nv ].nv_item_size )
  {
    QCRIL_LOG_DEBUG( "Mismatch in Recieved Length = %lu and Expected Length = %lu for nv item = %s\n",
                     nv_item_length, qcril_other_nv_table[nv].nv_item_size,  qcril_other_nv_table[nv].name );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Lookup the modem that should be requested for NV service */ 
  if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_COMMON, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Validate the NV item data before writing to NV */
  memcpy( ( nv_item_data_ptr + qcril_other_nv_table[ nv ].nv_item_offset ), &data[ index ], nv_item_length );
  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    if( !qcril_other_nv_item_data_is_valid( instance_id, modem_id, nv_item, nv_item_data_ptr ) )
    {
      QCRIL_LOG_DEBUG( "Invalid Data Received for NV item = %s\n", qcril_other_nv_table[nv].name );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      return;
    }
  }

  /* All checks are done before reaching this point, hence write the NV item now */

  QCRIL_SNPRINTF( details, sizeof( details ), "Write %s", qcril_other_nv_table[ nv ].name );

  /* We may need to update NV item values on both modems */
  for ( i = 0; i < modem_ids_list.num_of_modems; i++ )
  {
    modem_id = modem_ids_list.modem_id[ i ];
    QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

    QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, details );
    nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_WRITE_F, nv_item, (nv_item_type *) &nv_item_data 
                                                                      #ifdef FEATURE_QCRIL_DSDS
                                                                      , as_id
                                                                      #endif /* FEATURE_QCRIL_DSDS */
                                                                    );
    QCRIL_LOG_DEBUG( "nv_status is %d after writing %s  nv item\n",  (int)nv_status, qcril_other_nv_table[nv].name );
    if ( nv_status != NV_DONE_S )
    {
      break;
    }
  }

  if ( nv_status == NV_DONE_S )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_other_request_oem_hook_nv_write */


/*=========================================================================

  FUNCTION:  qcril_other_default_min1

===========================================================================*/
/*!
    @brief
    This function computes the default min1 value as per IS-683 section 3.1.

    @return
    This function returns the encoded value of min1 with the four least
    significant digits set to ESNp, converted directly from binary to decimal,
    modulo 10000.  The other digits are set to zero.
*/
/*=========================================================================*/
dword qcril_other_default_min1
( 
  qcril_instance_id_e_type instance_id
)
{
  word zero;     /* Encoding of three zero digits */
  word fourth;   /* Fourth from last decimal digit of the ESN */
  word last3;    /* Last three decimal digits of the ESN */
  nv_item_type nvi;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  (void) qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id );
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_ESN_I" );
  qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_ESN_I, (nv_item_type *) &nvi 
                                                        #ifdef FEATURE_QCRIL_DSDS
                                                        , as_id
                                                        #endif /* FEATURE_QCRIL_DSDS */
                                                      );

  /* Encode digits as per JSTD-008 section 2.3.1.1 */
  zero = 1110 - 111;
  last3 = (word) ( nvi.esn.esn % 1000 );
  last3 += ( ( last3 / 100 ) == 0 ) ? 1000 : 0;
  last3 += ( ( ( last3 % 100) / 10 ) == 0 ) ? 100 : 0;
  last3 += ( ( last3 % 10 ) == 0 ) ? 10 : 0;
  last3 -= 111;
  fourth = (word)( ( nvi.esn.esn % 10000 ) / 1000 ); /* In range 0-9 */
  if ( fourth == 0 )
  {
    fourth = 10;
  }

  /* Concatenate results and return 24 bit value for imsi_s1 */
  /* Example: esn = 120406
  **      imsi_s1 = 000  0  406
  **  encodes to -> 999  10 395
  **       in hex = 3e7  a  18b
  **    in binary = 1111100111 1010 0110001011
  */
  return ( ( (dword) zero << 14 ) | ( fourth << 10 ) | last3 );

} /* qcril_other_default_min1() */


/*=========================================================================

  FUNCTION:  qcril_other_default_nv_value

===========================================================================*/
/*!
    @brief
    assigning defualt values to the nv_items

    @return
    Return default values for nv_items
*/
/*=========================================================================*/
static boolean qcril_other_default_nv_value
(
  qcril_instance_id_e_type instance_id,
  uint32 nv_item,
  nv_item_type *nv_item_data
)
{
  int sid = 0, indx = 0;
  boolean status = TRUE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( nv_item_data != NULL );

  /*-----------------------------------------------------------------------*/

  switch( nv_item )
  {
    case NV_MIN1_I:
      /* initializing with the default values */
      nv_item_data->min1.min1[ QCRIL_OTHER_CDMAMIN ] = qcril_other_default_min1( instance_id );
      nv_item_data->min1.min1[ QCRIL_OTHER_FMMIN ] = QCRIL_OTHER_IMSI_S1_0;
      nv_item_data->min1.nam = 0;
      break;

   case NV_MIN2_I:
     nv_item_data->min2.min2[ QCRIL_OTHER_CDMAMIN ] = QCRIL_OTHER_IMSI_S2_0;
     nv_item_data->min2.min2[ QCRIL_OTHER_FMMIN ] = QCRIL_OTHER_IMSI_S2_0;
     break;

   case NV_IMSI_11_12_I:
     nv_item_data->imsi_11_12.imsi_11_12 = QCRIL_OTHER_IMSI_11_12_0;
     break;

   case NV_DIR_NUMBER_I:
     status = FALSE;
     break;

   case NV_SID_NID_I:
     for( sid = 0; sid < NV_MAX_SID_NID; sid++ )
     {
       /*  NID default */
       nv_item_data->sid_nid.pair[ NV_CDMA_MIN_INDEX ][ sid ].sid = 0;
       nv_item_data->sid_nid.pair[ NV_CDMA_MIN_INDEX ][ sid ].nid = QCRIL_OTHER_NID_DEFAULTS;
     }
     break;

   case NV_IMSI_T_S1_I:
     for( sid = 0; sid < NV_MAX_SID_NID; sid++ )
     {
       nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_CDMAMIN ] = qcril_other_default_min1( instance_id );
       nv_item_data->imsi_t_s1.min1[ QCRIL_OTHER_FMMIN ] = QCRIL_OTHER_IMSI_S1_0;
       nv_item_data->imsi_t_s1.nam = 0;
     }
     break;

   case NV_IMSI_T_S2_I:
     nv_item_data->imsi_t_s2.min2[ QCRIL_OTHER_CDMAMIN ] = QCRIL_OTHER_IMSI_S2_0;
     nv_item_data->imsi_t_s2.min2[ QCRIL_OTHER_FMMIN ] = QCRIL_OTHER_IMSI_S2_0;
     break;

   case NV_IMSI_T_MCC_I:
     nv_item_data->imsi_t_mcc.imsi_mcc = QCRIL_OTHER_IMSI_MCC_0;
     break;

   case NV_IMSI_T_11_12_I:
     nv_item_data->imsi_t_11_12.imsi_11_12 = QCRIL_OTHER_IMSI_11_12_0;
     break;

   case NV_IMSI_T_ADDR_NUM_I:
     nv_item_data->imsi_t_addr_num.num = QCRIL_OTHER_IMSI_CLASS0_ADDR_NUM;
     break;

   case NV_IMSI_MCC_I:
     nv_item_data->imsi_mcc.imsi_mcc = QCRIL_OTHER_IMSI_MCC_0;
     break;

   case NV_PCDMACH_I:
     nv_item_data->pcdmach.channel_a = QCRIL_OTHER_PCH_A_DEFAULT;
     nv_item_data->pcdmach.channel_b = QCRIL_OTHER_PCH_B_DEFAULT;
     break;

   case NV_SCDMACH_I:
     /* setting to default values */
     nv_item_data->scdmach.channel_a = QCRIL_OTHER_SCH_A_DEFAULT;
     nv_item_data->scdmach.channel_b = QCRIL_OTHER_SCH_B_DEFAULT;
     break;

   case NV_ECC_LIST_I:
   case NV_SEC_CODE_I:
   case NV_LOCK_CODE_I:
   case NV_PREF_VOICE_SO_I:
   case NV_DS_DEFAULT_BAUDRATE_I:
     status = FALSE;
     break;

   case NV_ANALOG_HOME_SID_I:
     nv_item_data->analog_home_sid.sid = 0;
     break;

   case NV_SPC_CHANGE_ENABLED_I:
     nv_item_data->spc_change_enabled = FALSE;
     break;

   case NV_HOME_SID_NID_I:
     for( indx = 0; indx < NV_MAX_HOME_SID_NID; indx++ )
     {
       /*  NID default */
       nv_item_data->home_sid_nid.pair[ indx ].sid = 0;
       nv_item_data->home_sid_nid.pair[ indx ].nid = QCRIL_OTHER_NID_DEFAULTS;
     }
     break;

   case NV_GPS1_PDE_ADDRESS_I:
   case NV_GPS1_PDE_PORT_I:
   case NV_MOB_CAI_REV_I:
   case NV_PRIMARY_DNS_I:
   case NV_SECONDARY_DNS_I:
   case NV_IPV6_PRIMARY_DNS_I:
   case NV_IPV6_SECONDARY_DNS_I:
     status = FALSE;
     break;

   case NV_AIR_CNT_I:
     nv_item_data->air_cnt.cnt = 0;
     break;

   case NV_ROAM_CNT_I:
     nv_item_data->roam_cnt.cnt = 0;
     break;

   default:
     QCRIL_LOG_DEBUG( "Unexpected NV item %lu\n",  nv_item );
     status = FALSE;

  } /* switch */

  return status;

} /* qcril_other_default_nv_value() */


/*=========================================================================

  FUNCTION:  qcril_other_request_oem_hook_nv_read

===========================================================================*/
/*!
    @brief
    Reads the request NAM parameter from NV.

    @return
    If NV read is success then the corresponding NV item value is returned
    void
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_nv_read
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_modem_ids_list_type modem_ids_list;
  char *data_ptr;
  uint32 nv_item = 0, index;
  nv_item_type nv_item_data;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  boolean status = TRUE;
  char  details[ 30 ];
  qcril_request_resp_params_type resp;
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  data_ptr = ( char *)params_ptr->data;
  QCRIL_ASSERT( data_ptr != NULL );

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

  /* initializing the data which is not yet initialized */
  memset( &nv_item_data, 0, sizeof( nv_item_data ) );

  /* decode the NV item from the raw stream, data[0--3], 4 bytes */
  memcpy( &nv_item, data_ptr, QCRIL_OTHER_OEM_ITEMID_DATA_LEN );

  QCRIL_LOG_DEBUG( "Received request for Reading nv_item = %lu\n", nv_item );

  /* Get the index of NV item data item */
  index = 0;
  while ( ( index < ( QCRIL_OTHER_NUM_OF_NV_ITEMS ) ) && ( nv_item != qcril_other_nv_table[ index ].nv_item ) )
  {
     index++;
  }

  /* Check if the requested NV item was found in the nv table */
  if( index == ( QCRIL_OTHER_NUM_OF_NV_ITEMS ) )
  {
    QCRIL_LOG_DEBUG( "Requested NV item not found = %lu\n", nv_item );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* Lookup the modem that should be requested for NV service */ 
  if ( qcril_arb_query_nv_srv_modem_id( QCRIL_ARB_NV_SRV_CAT_COMMON, instance_id, &modem_ids_list ) != E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  /* In case of global device for split modem architecture, OEM is required to make sure the NV item values on both modems 
     are in sync. So read from one modem is sufficient */
  modem_id = modem_ids_list.modem_id[ 0 ];
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  QCRIL_SNPRINTF( details, sizeof( details ), "Read %s", qcril_other_nv_table[ index ].name );
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, details );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F,  nv_item, &nv_item_data 
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );
  QCRIL_LOG_DEBUG( "nv_status after reading %s = %d\n", qcril_other_nv_table[index].name, (int)nv_status );

  /* Return SUCCESS only in the case where status is NV_DONE_S or NV_NOTACTIVE_S */
  if( ( nv_status == NV_DONE_S  ) || ( nv_status == NV_NOTACTIVE_S ) )
  {
    /* Set default values in case nv_status is NV_NOTACTIVE_S */
    if( nv_status == NV_NOTACTIVE_S )
    {
      status = qcril_other_default_nv_value( instance_id, nv_item, &nv_item_data );
      QCRIL_LOG_DEBUG( "qcril_other_default_nv_value() returned status as %d\n", status );
    }

    /* if status is true, it means that nv item is set to defualt values properly, return the true length */
    if ( status )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = (void *) &nv_item_data;
      resp.resp_len = qcril_other_nv_table[index].nv_item_size;
      qcril_send_request_response( &resp );
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_other_request_oem_hook_nv_read */

/*=========================================================================

  FUNCTION:  qcril_other_read_managed_roaming_from_nv

===========================================================================*/
/*!
    @brief
    Read NV_GPRS_ANITE_GCF_I  and NV_MGRF_SUPPORTED_I from NV.

    @return
    TRUE if managed roaming NV items are set properly.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_other_read_managed_roaming_from_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  boolean mgr_enabled = FALSE;
  nv_item_type nv_item;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_cmd_api_name= "nv_cmd_ext_remote()";
  #else
  char *nv_cmd_api_name= "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  #ifdef FEATURE_QCRIL_DSDS
  /* Lookup as_id */
  if ( qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id ) != E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s\n", "as_id not available, Ignore NV read request" );
    return FALSE;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  /* Read data from NV */
  QCRIL_LOG_RPC2A( modem_id, nv_cmd_api_name, "Read NV_GPRS_ANITE_GCF_I" );
  nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_GPRS_ANITE_GCF_I, ( nv_item_type * ) &nv_item
                                                                    #ifdef FEATURE_QCRIL_DSDS
                                                                    , as_id
                                                                    #endif /* FEATURE_QCRIL_DSDS */
                                                                  );

  QCRIL_LOG_DEBUG("gcf_nv_status = %d,  gcf_nv_enabled = %d", nv_status, nv_item.gprs_anite_gcf );

  if( ( ( nv_status == NV_DONE_S ) && ( nv_item.gprs_anite_gcf == FALSE) ) ||
       ( nv_status != NV_DONE_S) )
  {
     nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_MGRF_SUPPORTED_I , ( nv_item_type * ) &nv_item
                                                                       #ifdef FEATURE_QCRIL_DSDS
                                                                       , as_id
                                                                       #endif /* FEATURE_QCRIL_DSDS */
                                                                     );

     QCRIL_LOG_DEBUG("mgrf_nv_status = %d,  mgrf_nv_enabled = %d", nv_status, nv_item.mgrf_supported );

     if( ( nv_status == NV_DONE_S ) && ( nv_item.mgrf_supported == TRUE) )
     {
        mgr_enabled = TRUE;
     }
  }

  return mgr_enabled;
}
/*=========================================================================

  FUNCTION:  qcril_common_update_current_imsi

===========================================================================*/
/*!
    @brief
    No action in RPC RIL, compile time compatibility with QMI RIL

    @return
    None
*/
/*=========================================================================*/
void qcril_common_update_current_imsi( char * imsi_str, int is_gwl ) 
{
  QCRIL_NOTUSED( imsi_str );
  QCRIL_NOTUSED( is_gwl );
}
