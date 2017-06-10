/*!
  @file
  qcril_arb.c

  @brief
  Manage RIL instance data and service arbitration

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

when       who     what, where, why
--------   ---     ---------------------------------------------------------- 
04/08/10   fc      First cut.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <errno.h>
#include <cutils/memory.h>
#include <pthread.h>
#include <cutils/properties.h>
#include <string.h>
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_cm.h"
#include "qcril_cmi.h"
#include "qcril_cm_util.h"
#include "qcril_log.h"
#include "qcril_arb.h"


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*! @brief Structure used to cache QCRIL instance data 
*/
typedef struct
{
  pthread_mutex_t                 mutex;               /* Mutex used to control simultaneous update/access to arb data */
  qcril_arb_ma_e_type             ma;                  /* Modem architecture indicator */ 
  uint8                           num_of_modems;       /* Number of modems in architecture */
  uint8                           num_of_slots;        /* Number of card slots */
  qcril_instance_id_e_type        num_of_instances;    /* Number of active instances */
  boolean                         sma_voice_pref_3gpp; /* Indicates the voice preference for Fusion architecture */
  boolean                         voip_enabled;        /* Indicates whether VoIP is supported */
  boolean                         net_pref_restored[ QCRIL_MAX_INSTANCE_ID ];
  qcril_cm_net_pref_e_type        net_pref[ QCRIL_MAX_INSTANCE_ID ];           /* Preferred network setting */
  qcril_arb_pref_data_tech_e_type pref_data_tech[ QCRIL_MAX_INSTANCE_ID ];     /* Preferred data tech for Fusion architecture*/
  qcril_arb_state_struct_type     state;                                       /* State data */
  #ifdef FEATURE_QCRIL_DSDS
  qcril_arb_subs_struct_type      subs;                                        /* Subscription data */
  #endif /* FEATURE_QCRIL_DSDS */
  qcril_cm_struct_type            cm_info[ QCRIL_MAX_INSTANCE_ID ];            /* QCRIL(CM) data */
  qcril_sms_struct_type           sms_info[ QCRIL_MAX_INSTANCE_ID ];           /* QCRIL(SMS) data */
  qcril_pbm_struct_type           pbm_info[ QCRIL_MAX_INSTANCE_ID ];           /* QCRIL(PBM) data */
  qcril_other_struct_type         other_info[ QCRIL_MAX_INSTANCE_ID ];         /* QCRIL(OTHER) data */
} qcril_arb_struct_type;


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* Cache QCRIL instances' data */
static qcril_arb_struct_type qcril_arb;

static char *qcril_arb_ma_name[] = { "Multimode", "Fusion (QCS)", "Fusion (TPS)", "DSDS" };

static char *qcril_arb_net_pref_name[] =
  { "GSM WCDMA preferred", "GSM Only", "WCDMA only", "GSM WCDMA Auto", "CDMA EVDO", "CDMA Only", "EVDO only", 
    "GSM WCDMA CDMA EVDO", "LTE CDMA EVDO", "LTE GSM WCDMA", "LTE CDMA EVDO GSM WCDMA", "LTE only" };

static char *qcril_arb_pref_data_tech_name[] =
  { "Unknown", "CDMA", "EVDO", "GSM", "UMTS", "EHRPD", "LTE" };

#ifdef FEATURE_QCRIL_DSDS
static char *qcril_arb_subs_state_name[] = { "Not provisioned", "Apps selected", "Provisioned", 
                                             "Apps not selected" };
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


 
/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_arb_init

===========================================================================*/
/*!
    @brief
    Initialize the split modem architecture property.

    @return
    none
*/
/*=========================================================================*/
void qcril_arb_init
( 
  void 
)
{
  qcril_instance_id_e_type instance_id;
  char args[ PROPERTY_VALUE_MAX ];     
  char property_name[ 40 ];
  int len;
  char *end_ptr;
  unsigned long ret_val;

  /*-----------------------------------------------------------------------*/
                    
  QCRIL_LOG_DEBUG ( "%s\n", "qcril_arb_init()" );

  /* Initialize internal data cache */
  memset( &qcril_arb, 0, sizeof( qcril_arb ) );

  /* Initialize mutex */
  pthread_mutex_init( &qcril_arb.mutex, NULL );  

  /* Default preferred data technology */
  for ( instance_id = 0; instance_id < QCRIL_MAX_INSTANCE_ID ; instance_id++ )
  {
    qcril_arb.pref_data_tech[ instance_id ] = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
  }

  /* Default MA indicator */
  #if defined( FEATURE_QCRIL_FUSION )
  qcril_arb.ma = QCRIL_ARB_MA_FUSION_QCS;
  #elif defined( FEATURE_QCRIL_DSDS )
  qcril_arb.ma = QCRIL_ARB_MA_DSDS;
  #else
  qcril_arb.ma = QCRIL_ARB_MA_MULTIMODE;
  #endif /* FEATURE_QCRIL_FUSION */

  /* Retrieve SMA setting from property */
  if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
  {
    property_get( QCRIL_ARB_SMA, args, "" );
    len = strlen( args );
    if ( len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 ); 
      if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) ) 
      {
        QCRIL_LOG_ERROR( "Fail to convert SMA setting %s\n", args );
      }
      else if ( ( ret_val != QCRIL_ARB_MA_FUSION_QCS ) && ( ret_val != QCRIL_ARB_MA_FUSION_TPS ) ) 
      {
        QCRIL_LOG_ERROR( "Invalid saved SMA setting %ld, use default\n", ret_val );
      }
      else
      {
        qcril_arb.ma = ( qcril_arb_ma_e_type ) ret_val;
      }
    }

    /* Save SMA setting to system property */
    QCRIL_SNPRINTF( args, sizeof( args ), "%d", qcril_arb.ma );
    if ( property_set( QCRIL_ARB_SMA, args ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Fail to save %s to system property\n", QCRIL_ARB_SMA );
    }                                                                                

    property_get( QCRIL_ARB_VOIP, args, "" );
    len = strlen( args );
    if ( ( len > 0 ) && ( strcmp( args, "true" ) == 0 ) ) 
    {
      QCRIL_LOG_DEBUG( "%s\n", "voip_enabled = true" );
      qcril_arb.voip_enabled = TRUE;
    }
    else
    {
      QCRIL_LOG_DEBUG( "%s\n", "voip_enabled = false" );
      qcril_arb.voip_enabled = FALSE;
    }

    /* Save SVLTE setting to system property */
    strlcpy(args,"true",sizeof(args));
    if ( property_set( QCRIL_ARB_SVLTE, args ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Fail to save %s to system property\n", QCRIL_ARB_SVLTE );
    }                                                                                

  }

  /* Set the maximum number of modems */
  if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
  {
    qcril_arb.num_of_modems = 2;
  }
  else
  {
    qcril_arb.num_of_modems = 1;
  }

  /* Default Split modem's global mode voice preference setting */
  qcril_arb.sma_voice_pref_3gpp = FALSE;

  /* Retrieve SMA Voice Preference setting from property */
  if ( ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS ) || ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS ) )
  {
    property_get( QCRIL_ARB_SMA_VOICE_PREF_3GPP, args, 0 );
    len = strlen( args );
    if ( len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 ); 
      if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) ) 
      {
        QCRIL_LOG_ERROR( "Fail to convert sma voice preference setting %s\n", args );
      }
      else if ( ret_val > 1 )
      {
        QCRIL_LOG_ERROR( "Invalid saved sma voice preference setting %ld, use default\n", ret_val );
      }
      else
      {
        qcril_arb.sma_voice_pref_3gpp = ret_val;
      }
    }
  }

  /* Save SMA Voice Preference setting to system property */
  QCRIL_SNPRINTF( args, sizeof( args ), "%d", qcril_arb.sma_voice_pref_3gpp );
  if ( property_set( QCRIL_ARB_SMA_VOICE_PREF_3GPP, args ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "Fail to save %s to system property\n", QCRIL_ARB_SMA_VOICE_PREF_3GPP );
  }                                                                                

  /* Retrieve DSDS enabled setting from property */
  if ( qcril_arb.ma == QCRIL_ARB_MA_DSDS )
  {
    property_get( QCRIL_ARB_DSDS, args, "" );
    len = strlen( args );
    if ( ( len > 0 ) && ( strncmp( args, "dsds", QCRIL_ARB_DSDS_PROP_LENGTH ) == 0 ) )
    {
      QCRIL_LOG_DEBUG( "%s\n", "dsds_enabled = true, in multimode" );
    }
    else
    {
      QCRIL_LOG_DEBUG( "%s\n", "dsds_enabled = false, in multimode" );
      qcril_arb.ma = QCRIL_ARB_MA_MULTIMODE;
    }
  }

  /* Set the maximum number of slots */
  #ifdef FEATURE_QCRIL_DSDS
  if ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) 
  {
    qcril_arb.num_of_slots = 2;
    qcril_arb.num_of_instances = (qcril_instance_id_e_type) 2;
  }
  else
  #endif /* FEATURE_QCRIL_DSDS */
  {
    qcril_arb.num_of_slots = 1;
    qcril_arb.num_of_instances = (qcril_instance_id_e_type) 1;
  }

  /* Retrieve Network Preference setting from property */
  for ( instance_id = 0; instance_id < QCRIL_MAX_INSTANCE_ID ; instance_id++ )
  {
    qcril_arb.net_pref_restored[ instance_id ] = FALSE;
    /* Default Network Preference Setting ito global */
    #ifdef FEATURE_QCRIL_FUSION
    qcril_arb.net_pref[ instance_id ] = QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
    #else
    qcril_arb.net_pref[ instance_id ] = QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO;
    #endif /* FEATURE_QCRIL_FUSION */

    QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s_%d", QCRIL_ARB_NET_PREF, instance_id );
    property_get( property_name, args, "" );
    len = strlen( args );
    if ( len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 ); 
      if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) ) 
      {
        QCRIL_LOG_ERROR( "RID %d Fail to convert net_pref setting %s\n", instance_id, args );
      }
      else if ( ret_val > QCRIL_CM_NET_PREF_LTE_ONLY )
      {
        QCRIL_LOG_ERROR( "RID %d Invalid saved net_pref setting %ld, use default\n", instance_id, ret_val );
      }
      else if ( ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS ) && 
                ( qcril_arb.net_pref[ instance_id ] == QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO ) )
      {
        QCRIL_LOG_ERROR( "RID %d Invalid saved net_pref setting %ld, use default\n", instance_id, ret_val );
      }
      else if ( ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) ) && 
                ( qcril_arb.net_pref[ instance_id ] == QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA ) )
      {
        QCRIL_LOG_ERROR( "RID %d Invalid saved net_pref setting %ld, use default\n", instance_id, ret_val );
      }
      else
      {
        qcril_arb.net_pref_restored[ instance_id ] = TRUE;
        qcril_arb.net_pref[ instance_id ] = ( qcril_cm_net_pref_e_type ) ret_val;
      }
    }

    /* Save Network Preference setting to system property */
    QCRIL_SNPRINTF( args, sizeof( args ), "%d", qcril_arb.net_pref[ instance_id ] );
    if ( property_set( property_name, args ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "RID %d Fail to save %s to system property\n", instance_id, QCRIL_ARB_NET_PREF );
    }                                                                                

    QCRIL_LOG_DEBUG( "RID %d, ma=%s(%d), sma_voice_pref_3gpp=%d, restored=%d, net_pref=%s(%d)\n", 
                     instance_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma, qcril_arb.sma_voice_pref_3gpp,
                     qcril_arb.net_pref_restored[ instance_id ], qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], 
                     qcril_arb.net_pref[ instance_id ] );
  }

  #ifdef FEATURE_QCRIL_DSDS
  /* Initialize UICC subscription */
  for ( instance_id = 0; instance_id < QCRIL_MAX_INSTANCE_ID ; instance_id++ )
  {
    qcril_arb.subs.info[ instance_id ].uicc_sub.act_status = RIL_UICC_SUBSCRIPTION_DEACTIVATE;
    qcril_arb.subs.info[ instance_id ].uicc_sub.slot = -1;
    qcril_arb.subs.info[ instance_id ].uicc_sub.app_index = -1;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_LOG_DEBUG( "ma=%s(%d)\n", qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma );

} /* qcril_arb_init */


/*=========================================================================
  FUNCTION:  qcril_arb_allocate_cache

===========================================================================*/
/*!
    @brief
    Allocate cache for service manager's internal data.

    @return
    Pointer to allocated cache.
*/
/*=========================================================================*/
void *qcril_arb_allocate_cache
( 
  qcril_arb_cache_e_type cache_type 
)
{    
  void *cache_ptr = NULL;

  switch ( cache_type )
  {
    case QCRIL_ARB_CACHE_STATE:
      cache_ptr = (void *) &qcril_arb.state;
      break;

    case QCRIL_ARB_CACHE_CM:
      cache_ptr = (void *) &qcril_arb.cm_info;
      break;

    case QCRIL_ARB_CACHE_SMS:
      cache_ptr = (void *) &qcril_arb.sms_info;
      break;

    case QCRIL_ARB_CACHE_PBM:
      cache_ptr = (void *) &qcril_arb.pbm_info;
      break;

    case QCRIL_ARB_CACHE_OTHER:
      cache_ptr = (void *) &qcril_arb.other_info;
      break;

    default:
      break;
  }

  return cache_ptr;

} /* qcril_arb_allocate_cache */


/*=========================================================================
  FUNCTION:  qcril_arb_query_max_num_of_modems

===========================================================================*/
/*!
    @brief
    Query the number of modems.

    @return
    None
*/
/*=========================================================================*/
uint8 qcril_arb_query_max_num_of_modems
( 
  void
)
{
  /*-----------------------------------------------------------------------*/

  return qcril_arb.num_of_modems;

} /* qcril_arb_query_max_num_of_modems */


/*=========================================================================
  FUNCTION:  qcril_arb_query_max_num_of_slots

===========================================================================*/
/*!
    @brief
    Query the number of cards.

    @return
    None
*/
/*=========================================================================*/
uint8 qcril_arb_query_max_num_of_slots
( 
  void
)
{
  /*-----------------------------------------------------------------------*/

  return qcril_arb.num_of_slots;

} /* qcril_arb_query_max_num_of_slots */


/*=========================================================================
  FUNCTION:  qcril_arb_query_max_num_of_instances

===========================================================================*/
/*!
    @brief
    Query the maximum number of instances

    @return
    None
*/
/*=========================================================================*/
qcril_instance_id_e_type qcril_arb_query_max_num_of_instances
( 
  void
)
{
  /*-----------------------------------------------------------------------*/

  return qcril_arb.num_of_instances;

} /* qcril_arb_query_max_num_of_instances */


/*=========================================================================
  FUNCTION:  qcril_arb_query_net_pref

===========================================================================*/
/*!
    @brief
    Query network preference for specified instance.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_query_net_pref
( 
  qcril_instance_id_e_type instance_id,
  boolean *net_pref_restored_ptr,
  qcril_cm_net_pref_e_type *net_pref_ptr,
  qcril_arb_ma_e_type *ma_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( net_pref_restored_ptr != NULL );
  QCRIL_ASSERT( net_pref_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  *net_pref_restored_ptr = qcril_arb.net_pref_restored[ instance_id ];

  *net_pref_ptr = qcril_arb.net_pref[ instance_id ];

  *ma_ptr = qcril_arb.ma;

  QCRIL_LOG_DEBUG( "RID %d, ma=%s(%d), restored=%d, query net_pref=%s(%d)\n", 
                   instance_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma, qcril_arb.net_pref_restored[ instance_id ],
                   qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_net_pref */


/*=========================================================================
  FUNCTION:  qcril_arb_store_net_pref

===========================================================================*/
/*!
    @brief
    Save network preference for specified instance.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_store_net_pref
( 
  qcril_instance_id_e_type instance_id,
  qcril_cm_net_pref_e_type net_pref
)
{
  char args[ PROPERTY_VALUE_MAX ];     
  char property_name[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  qcril_arb.net_pref_restored[ instance_id ] = TRUE;
  qcril_arb.net_pref[ instance_id ] = net_pref;

  /* Save Network Preference setting to system property */
  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s_%d", QCRIL_ARB_NET_PREF, instance_id );
  QCRIL_SNPRINTF( args, sizeof( args ), "%d", qcril_arb.net_pref[ instance_id ] );
  if ( property_set( property_name, args ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "RID %d Fail to save %s to system property\n", instance_id, QCRIL_ARB_NET_PREF );
  }                                                                                

  QCRIL_LOG_DEBUG( "RID %d, ma=%s(%d), saved restored=%d, net_pref=%s(%d)\n", 
                   instance_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma, qcril_arb.net_pref_restored[ instance_id ],
                   qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_store_net_pref */


/*=========================================================================
  FUNCTION:  qcril_arb_query_arch_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem ids for techologies based on the underlying modem 
    architecture. 

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_query_arch_modem_id
(
  qcril_modem_id_e_type *cdma_modem_id,
  qcril_modem_id_e_type *evdo_modem_id,
  qcril_modem_id_e_type *gwl_modem_id
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( cdma_modem_id != NULL );
  QCRIL_ASSERT( evdo_modem_id != NULL );
  QCRIL_ASSERT( gwl_modem_id != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  /* Standalone modem */
  if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
  {
    *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
    *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
    *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
  }
  /* Split modem, Third party solution */
  else if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
  {
    *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
    *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
    *gwl_modem_id = QCRIL_MAX_MODEM_ID;
  }
  /* Split modem, QC party solution */
  else
  {
    *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
    *evdo_modem_id = QCRIL_SECOND_MODEM_ID;
    *gwl_modem_id = QCRIL_SECOND_MODEM_ID;
  }

  if ( *gwl_modem_id == QCRIL_MAX_MODEM_ID )
  {
    QCRIL_LOG_DEBUG( "Arch: cdma modem id=%d, evdo modem id=%d, ma=%s(%d)\n", 
                     *cdma_modem_id, *evdo_modem_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma );
  }
  else
  {
    QCRIL_LOG_DEBUG( "Arch: cdma modem id=%d, evdo modem id=%d, gwl modem id =%d, ma=%s(%d)\n", 
                     *cdma_modem_id, *evdo_modem_id, *gwl_modem_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_arch_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_voice_tech_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem ids for voice techologies based on the underlying modem 
    architecture and network preference. 

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_query_voice_tech_modem_id
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    *cdma_modem_id,
  qcril_modem_id_e_type    *gw_modem_id
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( cdma_modem_id != NULL );
  QCRIL_ASSERT( gw_modem_id != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  *cdma_modem_id = QCRIL_MAX_MODEM_ID;
  *gw_modem_id = QCRIL_MAX_MODEM_ID;

  switch( qcril_arb.net_pref[ instance_id ] )
  {
    case QCRIL_CM_NET_PREF_CDMA_ONLY:
    case QCRIL_CM_NET_PREF_CDMA_EVDO:
      *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      break;

    case QCRIL_CM_NET_PREF_EVDO_ONLY:
      if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
      {
        *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      else
      {
        *cdma_modem_id = QCRIL_SECOND_MODEM_ID;
      }
      break;
        
    case QCRIL_CM_NET_PREF_GSM_ONLY:
    case QCRIL_CM_NET_PREF_WCDMA_ONLY:
    case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
    case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
    case QCRIL_CM_NET_PREF_LTE_ONLY:
    case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
      {
        *gw_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      else
      {
        *gw_modem_id = QCRIL_SECOND_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
      *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
      {
        *gw_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      else
      {
        *gw_modem_id = QCRIL_SECOND_MODEM_ID;
      }
      break;

    default:
      break;
  } /* end switch */

  if ( ( *cdma_modem_id != QCRIL_MAX_MODEM_ID ) && ( *gw_modem_id != QCRIL_MAX_MODEM_ID ) )
  {
    QCRIL_LOG_DEBUG( "RID %d voice tech: cdma modem id=%d, gw modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *cdma_modem_id, *gw_modem_id, 
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( *cdma_modem_id != QCRIL_MAX_MODEM_ID )
  {
    QCRIL_LOG_DEBUG( "RID %d voice tech: cdma modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *cdma_modem_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( *gw_modem_id != QCRIL_MAX_MODEM_ID )
  {
    QCRIL_LOG_DEBUG( "RID %d voice tech: gw modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *gw_modem_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else 
  {
    QCRIL_LOG_DEBUG( "RID %d voice tech: not supported, ma=%s(%d), net_pref=%s(%d)\n",
                     instance_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_voice_tech_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_data_tech_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem ids for data techologies based on the underlying modem 
    architecture, network preference and DSD info. 

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_query_data_tech_modem_id
(
  qcril_instance_id_e_type        instance_id,
  qcril_modem_id_e_type           *cdma_modem_id,
  qcril_modem_id_e_type           *evdo_modem_id,
  qcril_modem_id_e_type           *gwl_modem_id,
  qcril_modem_id_e_type           *pref_data_tech_modem_id,
  qcril_arb_pref_data_tech_e_type *pref_data_tech
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( cdma_modem_id != NULL );
  QCRIL_ASSERT( evdo_modem_id != NULL );
  QCRIL_ASSERT( gwl_modem_id != NULL );
  QCRIL_ASSERT( pref_data_tech_modem_id != NULL );
  QCRIL_ASSERT( pref_data_tech != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  *cdma_modem_id = QCRIL_MAX_MODEM_ID;
  *evdo_modem_id = QCRIL_MAX_MODEM_ID;
  *gwl_modem_id = QCRIL_MAX_MODEM_ID;
  *pref_data_tech = qcril_arb.pref_data_tech[ instance_id ];
  *pref_data_tech_modem_id = QCRIL_MAX_MODEM_ID;

  switch( qcril_arb.net_pref[ instance_id ] )
  {
    case QCRIL_CM_NET_PREF_CDMA_ONLY:
      *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_CDMA;
        *pref_data_tech_modem_id = *cdma_modem_id;
      }
      break;

    case QCRIL_CM_NET_PREF_EVDO_ONLY:
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *evdo_modem_id = QCRIL_SECOND_MODEM_ID;
        *pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_EVDO;
        *pref_data_tech_modem_id = *evdo_modem_id;
      }
      else
      {
        *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_CDMA_EVDO:
      *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *evdo_modem_id = QCRIL_SECOND_MODEM_ID;
        if ( ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_EVDO ) || ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_EHRPD ) )
        {
          *pref_data_tech_modem_id = *evdo_modem_id;
        }
        else if ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_CDMA )
        {
          *pref_data_tech_modem_id = *cdma_modem_id;
        }
      }
      else
      {
        *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_GSM_ONLY:
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *gwl_modem_id = QCRIL_SECOND_MODEM_ID;
        *pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_GSM;
        *pref_data_tech_modem_id = *gwl_modem_id;
      }
      else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
      {
        *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_WCDMA_ONLY:
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *gwl_modem_id = QCRIL_SECOND_MODEM_ID;
        *pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
        *pref_data_tech_modem_id = *gwl_modem_id;
      }
      else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
      {
        *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_LTE_ONLY: 
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *gwl_modem_id = QCRIL_SECOND_MODEM_ID;
        *pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_LTE;
        *pref_data_tech_modem_id = *gwl_modem_id;
      }
      else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
      {
        *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
    case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
    case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *gwl_modem_id = QCRIL_SECOND_MODEM_ID;

        if ( *pref_data_tech != QCRIL_ARB_PREF_DATA_TECH_UNKNOWN )
        {
          *pref_data_tech_modem_id = *gwl_modem_id;
        }
      }
      else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
      {
        *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
      *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
      {
        *evdo_modem_id = QCRIL_SECOND_MODEM_ID;
        *gwl_modem_id = QCRIL_SECOND_MODEM_ID;
        if ( ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_GSM ) || ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_UMTS ) ||
             ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_LTE ) )
        {
          *pref_data_tech_modem_id = *gwl_modem_id;
        }
        else if ( ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_EVDO ) || ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_EHRPD ) )
        {
          *pref_data_tech_modem_id = *evdo_modem_id;
        }
        else if ( *pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_CDMA )
        {
          *pref_data_tech_modem_id = *cdma_modem_id;
        }
      }
      else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
      {
        *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
        *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      else if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
      {
        *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      break;

    default:
      break;
  }

  if ( ( *cdma_modem_id != QCRIL_MAX_MODEM_ID ) && ( *evdo_modem_id != QCRIL_MAX_MODEM_ID ) && ( *gwl_modem_id != QCRIL_MAX_MODEM_ID ) )
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: cdma modem id=%d, evdo modem id=%d, gwl modem id=%d, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *cdma_modem_id, *evdo_modem_id, *gwl_modem_id, *pref_data_tech_modem_id,
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( ( *cdma_modem_id != QCRIL_MAX_MODEM_ID ) && ( *evdo_modem_id != QCRIL_MAX_MODEM_ID ) )
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: cdma modem id=%d, evdo modem id=%d, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *cdma_modem_id, *evdo_modem_id, *pref_data_tech_modem_id, 
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( ( *cdma_modem_id != QCRIL_MAX_MODEM_ID ) && ( *gwl_modem_id != QCRIL_MAX_MODEM_ID ) )
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: cdma modem id=%d, gwl modem id=%d, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *cdma_modem_id, *gwl_modem_id, *pref_data_tech_modem_id, 
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( ( *evdo_modem_id != QCRIL_MAX_MODEM_ID ) && ( *gwl_modem_id != QCRIL_MAX_MODEM_ID ) )
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: evdo modem id=%d, gwl modem id=%d, pdt modem id=%, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *evdo_modem_id, *gwl_modem_id, *pref_data_tech_modem_id,
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( *cdma_modem_id != QCRIL_MAX_MODEM_ID ) 
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: cdma modem id=%d, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *cdma_modem_id, *pref_data_tech_modem_id,
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( *evdo_modem_id != QCRIL_MAX_MODEM_ID ) 
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: evdo modem id=%d, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *evdo_modem_id, *pref_data_tech_modem_id,
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else if ( *gwl_modem_id != QCRIL_MAX_MODEM_ID ) 
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: gwl modem id=%d, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *gwl_modem_id, *pref_data_tech_modem_id,
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else
  {
    QCRIL_LOG_DEBUG( "RID %d data tech: not supported, pdt modem id=%d, pdt=%s(%d), ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *gwl_modem_id, *pref_data_tech_modem_id,
                     qcril_arb_pref_data_tech_name[ *pref_data_tech ], *pref_data_tech,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_data_tech_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_set_data_sys_status

===========================================================================*/
/*!
    @brief
    Supposed to be used for RIL-QMI, make it empty to pass compilation

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_set_data_sys_status
(
  qcril_instance_id_e_type     instance_id,
  qmi_wds_data_sys_status_type data_sys_status
)
{
}

/*=========================================================================
  FUNCTION:  qcril_arb_set_pref_data_tech

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup. 

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_set_pref_data_tech
(
  qcril_instance_id_e_type instance_id,
  qcril_arb_pref_data_tech_e_type pref_data_tech
)
{
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  qcril_arb.pref_data_tech[ instance_id ] = pref_data_tech;

  QCRIL_LOG_DEBUG( "RID %d Preferred data tech %s(%d)\n", instance_id, qcril_arb_pref_data_tech_name[ pref_data_tech ], 
                   pref_data_tech );

  /* Network information updated, send unsolicited network state changed indication */
  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, &unsol_resp );
  unsol_resp.logstr = (void *) &qcril_arb_pref_data_tech_name[ pref_data_tech ];
  qcril_send_unsol_response( &unsol_resp );

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_set_pref_data_tech */


/*=========================================================================
  FUNCTION:  qcril_arb_query_ph_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for Phone Service.

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem 
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_ph_srv_modem_id
( 
  qcril_arb_ph_srv_cat_e_type ph_srv_cat,
  qcril_instance_id_e_type instance_id,
  qcril_modem_ids_list_type *modem_ids_list_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  char *ph_srv_cat_name[] = { "3GPP2", "3GPP", "Common", "Mode Pref" };
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_ids_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( ph_srv_cat > QCRIL_ARB_PH_SRV_CAT_MODE_PREF )
  {
    QCRIL_LOG_ERROR( "ph srv category %d not supported\n", ph_srv_cat );
    status = E_NOT_SUPPORTED;
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    return status;
  }

  modem_ids_list_ptr->num_of_modems = 0;

  if ( ph_srv_cat == QCRIL_ARB_PH_SRV_CAT_3GPP2 )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_EVDO_ONLY:
        /* Standalone modem or Split modem (3rd party) */
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      case QCRIL_CM_NET_PREF_CDMA_ONLY:
      case QCRIL_CM_NET_PREF_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
        modem_ids_list_ptr->num_of_modems = 1;
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  else if ( ph_srv_cat == QCRIL_ARB_PH_SRV_CAT_3GPP )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_GSM_ONLY:
      case QCRIL_CM_NET_PREF_WCDMA_ONLY:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_ONLY:
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        /* Split modem (3rd party) */
        if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
        {
          status = E_NOT_SUPPORTED;
        }
        else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  /* Common to both 3GPP2 and 3GPP services */
  else if ( ph_srv_cat == QCRIL_ARB_PH_SRV_CAT_COMMON )
  {
    if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
    {
      modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
      modem_ids_list_ptr->num_of_modems = 1;
    }
    else
    {
      modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
      modem_ids_list_ptr->modem_id[ 1 ] = QCRIL_SECOND_MODEM_ID;
      modem_ids_list_ptr->num_of_modems = 2;
    }
  }
  /* Specifc to mode preference services */
  else if ( ph_srv_cat == QCRIL_ARB_PH_SRV_CAT_MODE_PREF )
  {
    if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
    {
      modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
      modem_ids_list_ptr->num_of_modems = 1;
    }
    else 
    {
      switch( qcril_arb.net_pref[ instance_id ] )
      {
        case QCRIL_CM_NET_PREF_CDMA_ONLY:
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
          break;

        case QCRIL_CM_NET_PREF_EVDO_ONLY:
        case QCRIL_CM_NET_PREF_LTE_ONLY:
        case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
        case QCRIL_CM_NET_PREF_GSM_ONLY:
        case QCRIL_CM_NET_PREF_WCDMA_ONLY:
        case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
        case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
          break;

        default:
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->modem_id[ 1 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 2;
          break;
      }
    }
  }

  /* Sanity check */
  if ( status == E_SUCCESS )
  {
    QCRIL_ASSERT( modem_ids_list_ptr->num_of_modems > 0 );
    for ( i = 0; i < modem_ids_list_ptr->num_of_modems; i++ )
    {
      QCRIL_ASSERT( modem_ids_list_ptr->modem_id[ i ] < QCRIL_MAX_MODEM_ID );
      QCRIL_LOG_DEBUG( "RID %d ph srv %s(%d): modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                       instance_id, ph_srv_cat_name[ ph_srv_cat ], ph_srv_cat, modem_ids_list_ptr->modem_id[ i ],
                       qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                       qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );

    }
  }
  else
  {
    QCRIL_LOG_ERROR( "RID %d ph srv %s(%d): not supported, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, ph_srv_cat_name[ ph_srv_cat ], ph_srv_cat,  
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_query_ph_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_voice_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for Voice Service.

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem 
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_voice_srv_modem_id
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type *modem_id_ptr,
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type *voice_radio_tech_ptr
  #else
  qcril_radio_tech_e_type        *voice_radio_tech_ptr
  #endif
)
{
  IxErrnoType status = E_SUCCESS;
  qcril_cm_struct_type *i_ptr;
  boolean cdma_full_service = FALSE, gw_full_service = FALSE, cdma_limited_service = FALSE, gw_limited_service = FALSE;
  qcril_modem_id_e_type cdma_modem_id, gw_modem_id;
  char details[ 40 ];
  qcril_radio_tech_family_e_type radio_tech_family;
  qcril_radio_tech_e_type radio_tech;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_arb.cm_info[ instance_id ];
  QCRIL_ASSERT( modem_id_ptr != NULL );
  QCRIL_ASSERT( voice_radio_tech_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "cm_info[%d].ss_mutex\n", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->ss_mutex, details );

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  switch( qcril_arb.net_pref[ instance_id ] )
  {
    /* 1X voice preference device */
    case QCRIL_CM_NET_PREF_CDMA_ONLY:
    case QCRIL_CM_NET_PREF_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
      *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
      radio_tech_family = QCRIL_RADIO_TECH_3GPP2;
      /* defalut value of radio technology in case of 3gpp2 family is 1xRTT*/
      radio_tech = QCRIL_RADIO_TECH_1xRTT;
      break;

    case QCRIL_CM_NET_PREF_EVDO_ONLY:
      /* Not spit modem architecture */
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS ) 
      {
        /* MDM */
        *modem_id_ptr = QCRIL_SECOND_MODEM_ID;
      }
      else
      {
        *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
      }

      radio_tech_family = QCRIL_RADIO_TECH_3GPP2;
      /* defalut value of radio technology in case of 3gpp2 family is 1xRTT*/
      radio_tech = QCRIL_RADIO_TECH_1xRTT;
      break;

    /* GSM, WCDMA voice preference device */
    case QCRIL_CM_NET_PREF_GSM_ONLY: 
    case QCRIL_CM_NET_PREF_WCDMA_ONLY:
    case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
    case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
    case QCRIL_CM_NET_PREF_LTE_ONLY:
    case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      /* Not spit modem architecture */
      if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
      {
        *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
        radio_tech_family = QCRIL_RADIO_TECH_3GPP;
      }
      /* Third party solution */
      else if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
      {
        status = E_NOT_SUPPORTED;
      }
      else
      {
        /* MDM */
        *modem_id_ptr = QCRIL_SECOND_MODEM_ID;
        radio_tech_family = QCRIL_RADIO_TECH_3GPP;
      }

      switch( qcril_arb.net_pref[ instance_id ] )
      {
         case QCRIL_CM_NET_PREF_GSM_ONLY:
            radio_tech  = QCRIL_RADIO_TECH_GPRS;
            break;

         case QCRIL_CM_NET_PREF_WCDMA_ONLY:
            radio_tech  = QCRIL_RADIO_TECH_UMTS;
            break;

         case QCRIL_CM_NET_PREF_LTE_ONLY:
            radio_tech  = QCRIL_RADIO_TECH_LTE;
            break;

         default:
            /* defalut value of radio technology in case of 3gpp family is GPRS */
            radio_tech  = QCRIL_RADIO_TECH_GPRS;
            break;
      }
      break;
 
    /* Global device */
    case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
      /* Third party solution, MSM is always the voice preference device */
      if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
      {
        /* MSM */
        *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
        radio_tech_family = QCRIL_RADIO_TECH_3GPP2;

        /* defalut value of radio technology in case of 3gpp2 family is 1xRTT*/
        radio_tech = QCRIL_RADIO_TECH_1xRTT;
      }
      else
      {
        cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
        if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
        {
          gw_modem_id = QCRIL_DEFAULT_MODEM_ID;
        }
        else
        {
          gw_modem_id = QCRIL_SECOND_MODEM_ID;
        }

        /* Check which 1X modem has service */
        cdma_full_service = QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( i_ptr->ss_info[ cdma_modem_id ].srv_status, 
                                                                         i_ptr->ss_info[ cdma_modem_id ].sys_mode ); 
        cdma_limited_service = QCRIL_CM_SRV_STATUS_INDICATES_CDMA_SRV_AVAILABLE( i_ptr->ss_info[ cdma_modem_id ].srv_status, 
                                                                                 i_ptr->ss_info[ cdma_modem_id ].sys_mode ); 

        /* Check which GW modem has service */
        gw_full_service = QCRIL_CM_SRV_STATUS_INDICATES_GW_FULL_SRV( i_ptr->ss_info[ gw_modem_id ].srv_status,
                                                                     i_ptr->ss_info[ gw_modem_id ].sys_mode ); 
        gw_limited_service = QCRIL_CM_SRV_STATUS_INDICATES_GW_SRV_AVAILABLE( i_ptr->ss_info[ gw_modem_id ].srv_status,
                                                                             i_ptr->ss_info[ gw_modem_id ].sys_mode ); 

        /* Both 1X modem and GW modem have full/limited service */
        if ( ( cdma_full_service && gw_full_service ) || ( cdma_limited_service && gw_limited_service ) )
        {
          /* Choose based on voice preference setting */
          if ( qcril_arb.sma_voice_pref_3gpp )
          {
            *modem_id_ptr = QCRIL_SECOND_MODEM_ID;
            radio_tech_family = QCRIL_RADIO_TECH_3GPP;

            /* defalut value of radio technology in case of 3gpp family is GPRS */
            radio_tech  = QCRIL_RADIO_TECH_GPRS;
          }
          else
          {
            *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
            radio_tech_family = QCRIL_RADIO_TECH_3GPP2;

            /* defalut value of radio technology in case of 3gpp2 family is 1xRTT*/
            radio_tech = QCRIL_RADIO_TECH_1xRTT;
          }
        } 
        /* Only 3GPP2 has full service */
        else if ( cdma_full_service )
        {
          *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
          radio_tech_family = QCRIL_RADIO_TECH_3GPP2;

          /* defalut value of radio technology in case of 3gpp2 family is 1xRTT*/
          radio_tech = QCRIL_RADIO_TECH_1xRTT;
        }
        /* Only 3GPP has full service */
        else if ( gw_full_service )
        {
          /* Standalone modem */
          if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
          {
            *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
            radio_tech_family = QCRIL_RADIO_TECH_3GPP;
          }
          /* Split modem */
          else
          {
            *modem_id_ptr = QCRIL_SECOND_MODEM_ID;
            radio_tech_family = QCRIL_RADIO_TECH_3GPP;
          }

          switch( i_ptr->ss_info[ gw_modem_id ].sys_mode  )
          {
             case SYS_SYS_MODE_GSM:
                radio_tech  = QCRIL_RADIO_TECH_GPRS;
                break;
      
             case SYS_SYS_MODE_WCDMA:
                radio_tech  = QCRIL_RADIO_TECH_UMTS;
                break;
      
             default:
                /* defalut value of radio technology in case of 3gpp family is GPRS */
                radio_tech  = QCRIL_RADIO_TECH_GPRS;
                break;
          }
        }
        /* Only 3GPP2 has limited service */
        else if ( cdma_limited_service )
        {
          *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
          radio_tech_family = QCRIL_RADIO_TECH_3GPP2;

          /* defalut value of radio technology in case of 3gpp2 family is 1xRTT*/
          radio_tech = QCRIL_RADIO_TECH_1xRTT;
        }
        /* Only 3GPP has limited service */
        else if ( gw_limited_service )
        {
          /* Standalone modem */
          if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
          {
            *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
            radio_tech_family = QCRIL_RADIO_TECH_3GPP;
          }
          /* Split modem */
          else
          {
            *modem_id_ptr = QCRIL_SECOND_MODEM_ID;
            radio_tech_family = QCRIL_RADIO_TECH_3GPP;
          }
          /* defalut value of radio technology in case of 3gpp family is GPRS */
          radio_tech = QCRIL_RADIO_TECH_GPRS;
        }
        /* No service */
        else
        {
          *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
          radio_tech_family = QCRIL_RADIO_TECH_GLOBAL;

          /* defalut value of radio technology in case of 3gpp family is GPRS */
          radio_tech = QCRIL_RADIO_TECH_GLOBAL;
        }
      }
      break;

    default:
      status = E_NOT_ALLOWED;
      break;
  } /* end switch */
 
  if ( status == E_SUCCESS )
  {
    QCRIL_ASSERT( *modem_id_ptr < QCRIL_MAX_MODEM_ID );
    QCRIL_LOG_DEBUG( "RID %d voice srv: modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *modem_id_ptr,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else
  {
    QCRIL_LOG_ERROR( "RID %d voice srv: not supported, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_MUTEX_UNLOCK( &i_ptr->ss_mutex, details );

  if ( ( status == E_SUCCESS ) && qcril_arb_voip_is_supported() )
  {
    radio_tech_family= QCRIL_RADIO_TECH_3GPP2;
    radio_tech = QCRIL_RADIO_TECH_1xRTT;
    QCRIL_LOG_DEBUG( "%s\n", "VoIP, 3GPP2 voice radio tech" );
  }

  #ifndef FEATURE_ICS
  *voice_radio_tech_ptr = radio_tech_family;
  #else
  *voice_radio_tech_ptr = radio_tech;
  #endif

  return status;

} /* qcril_arb_query_voice_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_sms_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for SMS Service 

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem 
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_sms_srv_modem_id
( 
  qcril_arb_sms_srv_cat_e_type sms_srv_cat,
  qcril_instance_id_e_type instance_id,
  qcril_modem_ids_list_type *modem_ids_list_ptr 
)
{
  qcril_sms_struct_type *i_ptr;
  IxErrnoType status = E_SUCCESS;
  char *sms_srv_cat_name[] = { "3GPP2", "3GPP", "Common", "IMS", "IMS Reg" };
  uint8 i;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_arb.sms_info[ instance_id ];
  QCRIL_ASSERT( modem_ids_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_IMS
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].transport_reg_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->transport_reg_info_mutex, details );
  #endif /* FEATURE_QCRIL_IMS */

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( sms_srv_cat > QCRIL_ARB_SMS_SRV_CAT_IMS_REG )
  {
    QCRIL_LOG_ERROR( "RID %d sms srv category %d not supported\n", instance_id, sms_srv_cat );
    status = E_NOT_SUPPORTED;

    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

    #ifdef FEATURE_QCRIL_IMS
    QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );
    #endif /* FEATURE_QCRIL_IMS */
    return status;
  }  

  modem_ids_list_ptr->num_of_modems = 0;

  /* 3GPP2 SMS service */
  if ( sms_srv_cat == QCRIL_ARB_SMS_SRV_CAT_3GPP2 )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_CDMA_ONLY:
      case QCRIL_CM_NET_PREF_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:        
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
        modem_ids_list_ptr->num_of_modems = 1;
        break;

      case QCRIL_CM_NET_PREF_EVDO_ONLY:
        /* Split modem (QC) */
        if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  /* 3GPP SMS service */
  else if ( sms_srv_cat == QCRIL_ARB_SMS_SRV_CAT_3GPP )
  {
    switch ( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_GSM_ONLY: 
      case QCRIL_CM_NET_PREF_WCDMA_ONLY:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_ONLY:             
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA: 
        /* Split modem (3rd party) */
        if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
        {
          status = E_NOT_SUPPORTED;
        }
        /* Standalone modem */
        else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        /* Split modem (QC solution) */
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }

  /* IMS service */
  else if ( sms_srv_cat == QCRIL_ARB_SMS_SRV_CAT_IMS )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_EVDO_ONLY:
      case QCRIL_CM_NET_PREF_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_ONLY:
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:        
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        #ifdef FEATURE_QCRIL_IMS
        /* Split modem (QC), IMS service available */
        if ( ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS ) && 
             i_ptr->transport_reg_info[ QCRIL_SECOND_MODEM_ID ].is_registered )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        /* Not split modem (QC), IMS service available */
        else if ( ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS ) && 
                  i_ptr->transport_reg_info[ QCRIL_DEFAULT_MODEM_ID ].is_registered )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        #endif /* FEATURE_QCRIL_IMS */
        {
          status = E_NOT_SUPPORTED;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  /* Common SMS service */
  else if ( sms_srv_cat == QCRIL_ARB_SMS_SRV_CAT_COMMON )
  {
    switch ( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_CDMA_ONLY:
        modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
        modem_ids_list_ptr->num_of_modems = 1;
        break;

      case QCRIL_CM_NET_PREF_EVDO_ONLY:
        /* Standalone modem ot Split modem (3rd party)*/
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      case QCRIL_CM_NET_PREF_GSM_ONLY: 
      case QCRIL_CM_NET_PREF_WCDMA_ONLY:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
      case QCRIL_CM_NET_PREF_LTE_ONLY:             
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
        /* Split modem (3rd party) */
        if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
        {
          status = E_NOT_SUPPORTED;
        }
        /* Standalone modem */
        else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        /* Split modem (QC solution) */
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      case QCRIL_CM_NET_PREF_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:        
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        /* Standalone modem or Split modem (3rd party) */
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->modem_id[ 1 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 2;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  /* IMS SMS service */
  else if ( sms_srv_cat == QCRIL_ARB_SMS_SRV_CAT_IMS_REG )
  {
    switch ( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_EVDO_ONLY:
      case QCRIL_CM_NET_PREF_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_ONLY:             
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:        
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        /* Standalone modem or Split modem (3rd party)*/
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        /* Split modem (QC solution) */
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    }
  }

  /* Sanity check */
  if ( status == E_SUCCESS )
  {
    QCRIL_ASSERT( modem_ids_list_ptr->num_of_modems > 0 );
    for ( i = 0; i < modem_ids_list_ptr->num_of_modems; i++ )
    {
      QCRIL_ASSERT( modem_ids_list_ptr->modem_id[ i ] < QCRIL_MAX_MODEM_ID );
      QCRIL_LOG_DEBUG( "RID %d sms srv %s(%d): modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                       instance_id, sms_srv_cat_name[ sms_srv_cat], sms_srv_cat, modem_ids_list_ptr->modem_id[ i ],
                       qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                       qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );

    }
  }
  else
  {
    QCRIL_LOG_ERROR( "RID %d sms srv %s(%d): not supported, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, sms_srv_cat_name[ sms_srv_cat], sms_srv_cat, 
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  #ifdef FEATURE_QCRIL_IMS
  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );
  #endif /* FEATURE_QCRIL_IMS */

  return status;

} /* qcril_arb_query_sms_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_auth_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for AUTH Service. 

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem 
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_auth_srv_modem_id
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type *modem_id_ptr 
)
{
  IxErrnoType status = E_SUCCESS;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  switch( qcril_arb.net_pref[ instance_id ] )
  {
    case QCRIL_CM_NET_PREF_CDMA_ONLY:
    case QCRIL_CM_NET_PREF_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
    case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
      *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
      break;

    default:
      status = E_NOT_SUPPORTED;
      break;
  }

  if ( status == E_SUCCESS )
  {
    QCRIL_ASSERT( *modem_id_ptr < QCRIL_MAX_MODEM_ID );
    QCRIL_LOG_DEBUG( "RID %d auth srv: modem id=%d, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, *modem_id_ptr,
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }
  else
  {
    QCRIL_LOG_ERROR( "RID %d auth srv: not supported, ma=%s(%d), net_pref=%s(%d)\n", 
                     instance_id, qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_query_auth_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_nv_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for NV Service. 

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem 
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_nv_srv_modem_id
( 
  qcril_arb_nv_srv_cat_e_type nv_srv_cat,
  qcril_instance_id_e_type instance_id,
  qcril_modem_ids_list_type *modem_ids_list_ptr 
)
{
  IxErrnoType status = E_SUCCESS;
  uint8 i;
  char *nv_srv_cat_name[] = { "3GPP2", "3GPP", "Common" };

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_ids_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( nv_srv_cat > QCRIL_ARB_NV_SRV_CAT_COMMON )
  {
    QCRIL_LOG_ERROR( "RID %d nv srv category %d not supported\n", instance_id, nv_srv_cat );
    status = E_NOT_SUPPORTED;
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    return status;
  }

  modem_ids_list_ptr->num_of_modems = 0;

  /* 3GPP2 NV items */
  if ( nv_srv_cat == QCRIL_ARB_NV_SRV_CAT_3GPP2 )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_CDMA_ONLY:
        modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
        modem_ids_list_ptr->num_of_modems = 1;
        break;

      case QCRIL_CM_NET_PREF_EVDO_ONLY:
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      case QCRIL_CM_NET_PREF_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->modem_id[ 1 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 2;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  /* 3GPP NV items */
  else if ( nv_srv_cat == QCRIL_ARB_NV_SRV_CAT_3GPP )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_GSM_ONLY:
      case QCRIL_CM_NET_PREF_WCDMA_ONLY:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_ONLY:
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        /* Split modem (3rd party) */
        if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
        {
          status = E_NOT_SUPPORTED;
        }
        /* Standalone modem */
        else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        /* Split modem (QC solution) */
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;
    } /* end switch */
  }
  /* Common NV items */
  else if ( nv_srv_cat == QCRIL_ARB_NV_SRV_CAT_COMMON  )
  {
    switch( qcril_arb.net_pref[ instance_id ] )
    {
      case QCRIL_CM_NET_PREF_CDMA_ONLY:       
        modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
        modem_ids_list_ptr->num_of_modems = 1;
        break;

      case QCRIL_CM_NET_PREF_EVDO_ONLY:            
        /* Standalone modem or Split modem (3rd party) */
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      case QCRIL_CM_NET_PREF_CDMA_EVDO:        
      case QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO: 
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO:       
      case QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        /* Split modem (QC Solution) */
        if ( qcril_arb.ma != QCRIL_ARB_MA_FUSION_QCS )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->modem_id[ 1 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 2;
        }
        break;

      case QCRIL_CM_NET_PREF_GSM_ONLY: 
      case QCRIL_CM_NET_PREF_WCDMA_ONLY:           
      case QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO:      
      case QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED:
      case QCRIL_CM_NET_PREF_LTE_ONLY:  
      case QCRIL_CM_NET_PREF_LTE_GSM_WCDMA:      
        /* Split modem (3rd party) */
        if ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS )
        {
          status = E_NOT_SUPPORTED;
        }
        /* Standalone modem */
        else if ( ( qcril_arb.ma == QCRIL_ARB_MA_MULTIMODE ) || ( qcril_arb.ma == QCRIL_ARB_MA_DSDS ) )
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        /* Split modem (QC solution) */
        else
        {
          modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_SECOND_MODEM_ID;
          modem_ids_list_ptr->num_of_modems = 1;
        }
        break;

      default:
        status = E_NOT_SUPPORTED;
        break;

    } /* end switch */
  }

  /* Since the communication pipe between APPs and MDM fro NV access is not support, so all NV access goes to MSM */
  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;
  status = E_SUCCESS;

  /* Sanity check */
  if ( status == E_SUCCESS )
  {
    QCRIL_ASSERT( modem_ids_list_ptr->num_of_modems > 0 );
    for ( i = 0; i < modem_ids_list_ptr->num_of_modems; i++ )
    {
      QCRIL_ASSERT( modem_ids_list_ptr->modem_id[ i ] < QCRIL_MAX_MODEM_ID );
      QCRIL_LOG_DEBUG( "RID %d nv srv %s(%d): modem id=%d, ma=%s(%d), net pref=%s(%d)\n", 
                       instance_id, nv_srv_cat_name[ nv_srv_cat], nv_srv_cat, modem_ids_list_ptr->modem_id[ i ],
                       qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                       qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );

    }
  }
  else
  {
    QCRIL_LOG_ERROR( "RID %d nv srv %s(%d): not supported, ma=%s(%d), net pref=%s(%d)\n", 
                     instance_id, nv_srv_cat_name[ nv_srv_cat], nv_srv_cat, 
                     qcril_arb_ma_name[ qcril_arb.ma ], qcril_arb.ma,
                     qcril_arb_net_pref_name[ qcril_arb.net_pref[ instance_id ] ], qcril_arb.net_pref[ instance_id ] );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_query_nv_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_cdma_subscription_is_nv

===========================================================================*/
/*!
    @brief
    Indicates whether CDMA subscription source is NV.

    @return
     True if NV is the CDMA subscription,
     False otherwise.
*/
/*=========================================================================*/
boolean qcril_arb_cdma_subscription_is_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  boolean cdma_sub_is_nv = FALSE;
  qcril_cm_struct_type *i_ptr;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_arb.cm_info[ instance_id ];

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "cm_info[%d].ph_mutex\n", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->ph_mutex, details );

   if ( i_ptr->ph_info[ modem_id ].rtre_control == CM_RTRE_CONTROL_NV )
   {
     QCRIL_LOG_DEBUG( "RID %d NV is the CDMA subscription source\n", instance_id );
     cdma_sub_is_nv = TRUE;
   }
   else
   {
     QCRIL_LOG_DEBUG( "RID %d NV is not the CDMA subscription source\n", instance_id );
   }

   QCRIL_MUTEX_UNLOCK( &i_ptr->ph_mutex, details );

   return cdma_sub_is_nv;

} /* qcril_arb_cdma_subscription_is_nv */


/*=========================================================================
  FUNCTION:  qcril_arb_ma_is_fusion

===========================================================================*/
/*!
    @brief
    Indicates whether the modem architecture is FUSION.

    @return
     True if FUSION is the current mode,
     False otherwise.
*/
/*=========================================================================*/
boolean qcril_arb_ma_is_fusion
(
  void
)
{
  boolean ma_is_fusion = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS ) || ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_TPS ) )
  {
    ma_is_fusion = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return ma_is_fusion;

} /* qcril_arb_ma_is_fusion */


/*=========================================================================
  FUNCTION:  qcril_arb_ma_is_dsds

===========================================================================*/
/*!
    @brief
    Indicates whether the modem architecture is DSDS.

    @return
     True if DSDS is the current modem architecture,
     False otherwise.
*/
/*=========================================================================*/
boolean qcril_arb_ma_is_dsds
(
  void
)
{
  boolean ma_is_dsds = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( qcril_arb.ma == QCRIL_ARB_MA_DSDS )
  {
    ma_is_dsds = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return ma_is_dsds;

} /* qcril_arb_ma_is_dsds */

/*=========================================================================
  FUNCTION:  ril_to_uim_is_dsds_enabled

===========================================================================*/
/*!
    @brief
    Indicates whether the modem architecture is DSDS.

    @return
     True if DSDS is the current modem architecture,
     False otherwise.
*/
/*=========================================================================*/
int ril_to_uim_is_dsds_enabled(void)
{
  return qcril_arb_ma_is_dsds();
} /* ril_to_uim_is_dsds_enabled */


/*=========================================================================
  FUNCTION:  ril_to_uim_is_tsts_enabled
===========================================================================*/
int ril_to_uim_is_tsts_enabled(void)
{
  return FALSE;
} /* ril_to_uim_is_tsts_enabled */

#ifdef FEATURE_QCRIL_DSDS
/*=========================================================================
  FUNCTION:  qcril_arb_lookup_instance_id_from_session_type

===========================================================================*/
/*!
    @brief
    Lookup Instance ID from Session Type.

    @return
    E_SUCCESS if the Session Type can be mapped to Instance ID
    E_FAILURE otherwise 
*/
/*=========================================================================*/
IxErrnoType qcril_arb_lookup_instance_id_from_session_type
(
  qmi_uim_session_type session_type,
  qcril_instance_id_e_type *instance_id_ptr
)
{
  uint8 i;
  IxErrnoType status = E_FAILURE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Work as multimode under DSDS modem architecture */
  if ( !qcril_arb_ma_is_dsds() )
  {
    status = E_SUCCESS;
    *instance_id_ptr = QCRIL_DEFAULT_INSTANCE_ID;
    QCRIL_LOG_DEBUG( "Session type %d corresponds to RID %d\n", session_type, *instance_id_ptr );
    return status;
  }

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  /* Look for active subscription with matching session type */
  for (  i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
  {
    if ( ( ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_APPS_SELECTED ) ||
           ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_APPS_NOT_SELECTED ) ||
           ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_PROVISIONED ) ) && 
         ( qcril_arb.subs.info[ i ].session_type == session_type ) )
    {
      status = E_SUCCESS;
      *instance_id_ptr = i;
      break;
    }
  }

  if ( status == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "Session type %d corresponds to RID %d\n", session_type, *instance_id_ptr );
  }
  else
  {
    QCRIL_LOG_DEBUG( "Session type %d does not correspond to any instance\n", session_type );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_lookup_instance_id_from_session_type */


/*=========================================================================
  FUNCTION:  qcril_arb_lookup_instance_id_from_as_id

===========================================================================*/
/*!
    @brief
    Lookup Instance ID from AS_ID.

    @return
    E_SUCCESS if the AS_ID can be mapped to Instance ID
    E_FAILURE otherwise 
*/
/*=========================================================================*/
IxErrnoType qcril_arb_lookup_instance_id_from_as_id
(
  sys_modem_as_id_e_type as_id,
  qcril_instance_id_e_type *instance_id_ptr
)
{
  uint8 i;
  IxErrnoType status = E_FAILURE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id_ptr != NULL );

  /*-----------------------------------------------------------------------*/


  /* Work as multimode under DSDS modem architecture */
  if ( !qcril_arb_ma_is_dsds() )
  {
    status = E_SUCCESS;
    *instance_id_ptr = QCRIL_DEFAULT_INSTANCE_ID;
    QCRIL_LOG_DEBUG( "as_id %d corresponds to RID %d\n", as_id, *instance_id_ptr );
    return status;
  }

  /* Look for active subscription with matching session type */
  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  for (  i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
  {
    if ( ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_PROVISIONED ) &&
         ( qcril_arb.subs.info[ i ].uicc_sub.act_status == RIL_UICC_SUBSCRIPTION_ACTIVATE ) &&
         ( qcril_arb.subs.info[ i ].as_id == as_id ) )
    {
      status = E_SUCCESS;
      *instance_id_ptr = i;
      break;
    }
  }

  if ( status == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "as_id %d corresponds to RID %d\n", as_id, *instance_id_ptr );
  }
  else
  {
    QCRIL_LOG_DEBUG( "as_id %d does not correspond to any instance\n", as_id );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_lookup_instance_id_from_as_id */


/*=========================================================================
  FUNCTION:  qcril_arb_lookup_as_id_from_instance_id

===========================================================================*/
/*!
    @brief
    Lookup AS_ID from Instance ID.

    @return
    E_SUCCESS if the Instance ID can be mapped to AS_ID
    E_FAILURE otherwise 
*/
/*=========================================================================*/
IxErrnoType qcril_arb_lookup_as_id_from_instance_id
(
  qcril_instance_id_e_type instance_id,
  sys_modem_as_id_e_type *as_id_ptr
)
{
  IxErrnoType status = E_FAILURE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( as_id_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( !qcril_arb_ma_is_dsds() )
  {
    status = E_SUCCESS;
    *as_id_ptr = SYS_MODEM_AS_ID_1;
    QCRIL_LOG_DEBUG( "RID %d corresponds to as_id %d\n", instance_id, *as_id_ptr );
    return status;
  }

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( ( qcril_arb.subs.info[ instance_id ].state == QCRIL_ARB_SUBS_PROVISIONED ) && 
       ( qcril_arb.subs.info[ instance_id ].uicc_sub.act_status == RIL_UICC_SUBSCRIPTION_ACTIVATE ) )
  {
    status = E_SUCCESS;
    *as_id_ptr = qcril_arb.subs.info[ instance_id ].as_id;
  }

  if ( status == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "RID %d corresponds to as_id %d\n", instance_id, *as_id_ptr );
  }
  else
  {
    QCRIL_LOG_DEBUG( "RID %d does not correspond to any as_id, default as_id %d\n", instance_id, *as_id_ptr );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_lookup_as_id_from_instance_id */


/*=========================================================================
  FUNCTION:  qcril_arb_lookup_subs_from_session_type

===========================================================================*/
/*!
    @brief
    Lookup subs information from session type.

    @return
    E_SUCCESS if the session type can be mapped to active subscription
    E_FAILURE otherwise
*/
/*=========================================================================*/
IxErrnoType qcril_arb_lookup_subs_from_session_type
(
  qmi_uim_session_type session_type,
  qcril_arb_subs_prov_status_e_type *subs_state_ptr,
  RIL_SelectUiccSub *uicc_sub_ptr,
  sys_modem_as_id_e_type *as_id_ptr,
  qcril_instance_id_e_type *instance_id_ptr
)
{
  IxErrnoType status = E_FAILURE;
  int i = 0;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( as_id_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( !qcril_arb_ma_is_dsds() )
  {
    status = E_FAILURE;
    return status;
  }

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  /* Look for active subscription with matching session type */
  for (  i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
  {
    if ( ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_PROVISIONED )  &&
         ( qcril_arb.subs.info[ i ].session_type == session_type ) )
    {
      status = E_SUCCESS;
      *subs_state_ptr = qcril_arb.subs.info[ i ].state;
      *uicc_sub_ptr = qcril_arb.subs.info[ i ].uicc_sub;
      *as_id_ptr = qcril_arb.subs.info[ i ].as_id;
      *instance_id_ptr = i;
      break;
    }
  }

  if ( status == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "Session type %d corresponds to RID %d\n", session_type, *instance_id_ptr );
  }
  else
  {
    QCRIL_LOG_DEBUG( "Session type %d does not correspond to any instance\n", session_type );
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return status;

} /* qcril_arb_lookup_subs_from_session_type */

/*=========================================================================
  FUNCTION:  qcril_arb_select_subs_apps

===========================================================================*/
/*!
    @brief
    Mark subscription application selected for specified Instance ID.

    @return
  None
*/
/*=========================================================================*/
void qcril_arb_select_subs_apps
(
  qcril_instance_id_e_type instance_id,
  RIL_SelectUiccSub *uicc_sub_ptr,
  qmi_uim_session_type session_type
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( uicc_sub_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_LOG_DEBUG( "RID %d Select subs apps\n", instance_id );
  QCRIL_LOG_DEBUG( "RID %d olds subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id,
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot, 
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  qcril_arb.subs.info[ instance_id ].state = QCRIL_ARB_SUBS_APPS_SELECTED;
  memcpy( &qcril_arb.subs.info[ instance_id ].uicc_sub, uicc_sub_ptr, sizeof( RIL_SelectUiccSub ) );
  qcril_arb.subs.info[ instance_id ].session_type = session_type;
  qcril_arb.subs.info[ instance_id ].as_id = SYS_MODEM_AS_ID_NONE;

  QCRIL_LOG_DEBUG( "RID %d new subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id,
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_select_subs_apps */


/*=========================================================================
  FUNCTION:  qcril_arb_activate_subs

===========================================================================*/
/*!
    @brief
    Mark subscription activated for the specified Instance ID.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_activate_subs
(
  qcril_instance_id_e_type instance_id,
  sys_modem_as_id_e_type as_id
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_LOG_DEBUG( "RID %d Activate subs\n", instance_id );
  QCRIL_LOG_DEBUG( "RID %d olds subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id,
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  qcril_arb.subs.info[ instance_id ].state = QCRIL_ARB_SUBS_PROVISIONED;
  qcril_arb.subs.info[ instance_id ].as_id = as_id;

  QCRIL_LOG_DEBUG( "RID %d new subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id,
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  /* clear the card absent, deactivation pending flags */
  qcril_arb.subs.info[ instance_id ].card_removed = FALSE;
  qcril_arb.subs.info[ instance_id ].subs_deact_pending = FALSE;

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_activate_subs */


/*=========================================================================
  FUNCTION:  qcril_arb_unselect_subs_apps

===========================================================================*/
/*!
    @brief
    Mark subscription application not selected for specified Instance ID.

    @return
  None
*/
/*=========================================================================*/
void qcril_arb_unselect_subs_apps
(
  qcril_instance_id_e_type instance_id,
  RIL_SelectUiccSub *uicc_sub_ptr,
  qmi_uim_session_type session_type
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( uicc_sub_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_LOG_DEBUG( "RID %d Unselect subs apps\n", instance_id );
  QCRIL_LOG_DEBUG( "RID %d olds subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id, 
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  qcril_arb.subs.info[ instance_id ].state = QCRIL_ARB_SUBS_APPS_NOT_SELECTED;
  memcpy( &qcril_arb.subs.info[ instance_id ].uicc_sub, uicc_sub_ptr, sizeof( RIL_SelectUiccSub ) );
  qcril_arb.subs.info[ instance_id ].session_type = session_type;
  qcril_arb.subs.info[ instance_id ].as_id = SYS_MODEM_AS_ID_NONE;

  QCRIL_LOG_DEBUG( "RID %d new subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id,
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot, 
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_unselect_subs_apps */


/*=========================================================================
  FUNCTION:  qcril_arb_deactivate_subs

===========================================================================*/
/*!
    @brief
    Mark subscription deactivated for specified instance id.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_deactivate_subs
(
  qcril_instance_id_e_type instance_id
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_LOG_DEBUG( "RID %d Deactivate subs\n", instance_id );
  QCRIL_LOG_DEBUG( "RID %d olds subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id,
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  qcril_arb.subs.info[ instance_id ].state = QCRIL_ARB_SUBS_NOT_PROVISIONED;

  QCRIL_LOG_DEBUG( "RID %d new subs info, state = %s(%d), act_status = %d, slot_id = %d, app_index = %d, session_type = %d, as_id = %d\n",
                   instance_id, 
                   qcril_arb_subs_state_name[ qcril_arb.subs.info[ instance_id ].state ],
                   qcril_arb.subs.info[ instance_id ].state,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.act_status,
                   qcril_arb.subs.info[ instance_id ].uicc_sub.slot, 
                   qcril_arb.subs.info[ instance_id ].uicc_sub.app_index,
                   qcril_arb.subs.info[ instance_id ].session_type, qcril_arb.subs.info[ instance_id ].as_id );

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  /* clear the deactivation pending flag */
  qcril_arb_update_subs_deactivation_pending_flag( instance_id, FALSE);

  /* clear the card removed pending flag */
  qcril_arb_update_card_removed_flag( instance_id, FALSE);  
} /* qcril_arb_deactivate_subs */


/*=========================================================================
  FUNCTION:  qcril_arb_deactivate_all_subs

===========================================================================*/
/*!
    @brief
    If not multimode, mark all subscription as deactivated.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_deactivate_all_subs
(
  void
)
{
  uint8 i;

  /*-----------------------------------------------------------------------*/

  if ( qcril_arb_ma_is_dsds() )
  {
    QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

    for (  i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
    {
      qcril_arb.subs.info[ i ].state = QCRIL_ARB_SUBS_NOT_PROVISIONED;
    }

    QCRIL_LOG_DEBUG( "%s\n", "Deactivate all active subs" );

    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
  }

} /* qcril_arb_deactivate_all_subs */


/*=========================================================================
  FUNCTION:  qcril_arb_query_subs

===========================================================================*/
/*!
    @brief
    Query uicc subscription info for specified Instance ID.

    @return
    E_SUCCESS if subscription is available,
    E_FAILURE otherwise
*/
/*=========================================================================*/
void qcril_arb_query_subs
(
  qcril_instance_id_e_type instance_id,
  qcril_arb_subs_prov_status_e_type *subs_state_ptr,
  RIL_SelectUiccSub *uicc_sub_ptr,
  sys_modem_as_id_e_type *as_id_ptr,
  qmi_uim_session_type *session_type_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( subs_state_ptr != NULL );
  QCRIL_ASSERT( uicc_sub_ptr != NULL );
  QCRIL_ASSERT( as_id_ptr != NULL );
  QCRIL_ASSERT( session_type_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  *subs_state_ptr = qcril_arb.subs.info[ instance_id ].state;
  *uicc_sub_ptr = qcril_arb.subs.info[ instance_id ].uicc_sub;
  *as_id_ptr = qcril_arb.subs.info[ instance_id ].as_id;
  *session_type_ptr = qcril_arb.subs.info[ instance_id ].session_type;

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_subs */


/*=========================================================================
  FUNCTION:  qcril_arb_query_active_subs_list

===========================================================================*/
/*!
    @brief
    Query active subscription.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_query_active_subs_list
(
  qcril_sub_ids_list_type *sub_ids_list
)
{
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( sub_ids_list != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  sub_ids_list->num_of_subs = 0;

  for (  i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
  {
    if ( ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_PROVISIONED ) &&
         ( qcril_arb.subs.info[ i ].uicc_sub.act_status == RIL_UICC_SUBSCRIPTION_ACTIVATE ) )
    {
      QCRIL_LOG_DEBUG( "for active subscription[%d],  as_id = %d \n", sub_ids_list->num_of_subs, qcril_arb.subs.info[ i ].as_id);
      sub_ids_list->sub_id[ sub_ids_list->num_of_subs ] = qcril_arb.subs.info[ i ].as_id;
      sub_ids_list->num_of_subs++;
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_active_subs_list */

/*=========================================================================
  FUNCTION:  qcril_arb_query_active_subs_instances_by_card

===========================================================================*/
/*!
    @brief
    Query list of instances associated with the given card.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_query_active_subs_instances_by_card
(
  int slot,
  qcril_instance_ids_list_type *instance_ids_list
)
{
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_ids_list != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  instance_ids_list->num_of_subs = 0;

  for (  i = 0; i < QCRIL_ARB_MAX_INSTANCES; i++ )
  {
    if ( ( ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_PROVISIONED ) ||
            ( qcril_arb.subs.info[ i ].state == QCRIL_ARB_SUBS_APPS_SELECTED ) ) &&
         ( qcril_arb.subs.info[ i ].uicc_sub.act_status == RIL_UICC_SUBSCRIPTION_ACTIVATE )  &&
         ( qcril_arb.subs.info[ i ].uicc_sub.slot == slot ) )
    {
      QCRIL_LOG_DEBUG( "for active subscription[%u],  instance_id = %u \n", instance_ids_list->num_of_subs, i);
      instance_ids_list->instance_id[ instance_ids_list->num_of_subs ] = i;
      instance_ids_list->num_of_subs++;
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_query_active_subs_instances_by_card */

/*=========================================================================
  FUNCTION:  qcril_arb_update_subs_deactivation_pending_flag

===========================================================================*/
/*!
    @brief
    udpate subscription deactivation flag.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_update_subs_deactivation_pending_flag
(
  qcril_instance_id_e_type instance_id,
  boolean is_subs_deactivated
)
{

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  qcril_arb.subs.info[ instance_id].subs_deact_pending = is_subs_deactivated;

  QCRIL_LOG_DEBUG( "RIL %d is deactivated at modem, deactivation flag status = %d\n", instance_id, is_subs_deactivated);

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_update_subs_deactivation_pending_flag */


/*=========================================================================
  FUNCTION:  qcril_arb_query_subs_deactivation_pending_flag

===========================================================================*/
/*!
    @brief
    query subscription deactivation flag.

    @return
    None
*/
/*=========================================================================*/
boolean qcril_arb_query_subs_deactivation_pending_flag
(
  qcril_instance_id_e_type instance_id
)
{
  boolean is_deact_pending;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  is_deact_pending = qcril_arb.subs.info[ instance_id].subs_deact_pending;

  QCRIL_LOG_DEBUG( "RIL %d, subscription pending flag status = %d \n", instance_id, is_deact_pending);

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return is_deact_pending;
} /* qcril_arb_query_subs_deactivation_pending_flag */


/*=========================================================================
  FUNCTION:  qcril_arb_update_card_removed_flag

===========================================================================*/
/*!
    @brief
    udpate card removed flag.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_update_card_removed_flag
(
  qcril_instance_id_e_type instance_id,
  boolean is_card_removed
)
{

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  qcril_arb.subs.info[ instance_id].card_removed = is_card_removed;

  QCRIL_LOG_DEBUG( "RIL %d, updating the card removed info, status = %d\n", instance_id, is_card_removed);

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

} /* qcril_arb_update_card_removal_flag */


/*=========================================================================
  FUNCTION:  qcril_arb_query_subs_deactivation_pending_flag

===========================================================================*/
/*!
    @brief
    query card removed flag status.

    @return
    None
*/
/*=========================================================================*/
boolean qcril_arb_query_card_removed_flag
(
  qcril_instance_id_e_type instance_id
)
{
  boolean is_card_removed;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  is_card_removed = qcril_arb.subs.info[ instance_id].card_removed;

  QCRIL_LOG_DEBUG( "RIL %d, card removed status = %d \n", instance_id, is_card_removed);

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return is_card_removed;
} /* qcril_arb_query_subs_deactivation_pending_flag */
#endif /* FEATURE_QCRIL_DSDS */


/*=========================================================================
  FUNCTION:  qcril_arb_rtre_control_is_nv

===========================================================================*/
/*!
    @brief
    Indicates whether RTRE Control is NV.

    @return
     True if RTRE Control is NV,
     False otherwise.
*/
/*=========================================================================*/
boolean  qcril_arb_rtre_control_is_nv
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  boolean lock_ph_mutex
)
{
  boolean rtre_control_is_nv = FALSE;
  qcril_cm_struct_type *i_ptr;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_arb.cm_info[ instance_id ];

  /*-----------------------------------------------------------------------*/

  if ( lock_ph_mutex )
  {
    QCRIL_SNPRINTF( details, sizeof( details ), "cm_info[%d].ph_mutex\n", instance_id );
    QCRIL_MUTEX_LOCK( &i_ptr->ph_mutex, details );
  }

  QCRIL_LOG_DEBUG( "RID %d MID %d RTRE Control %d\n", instance_id, modem_id, i_ptr->ph_info[ modem_id ].rtre_control );

  if ( i_ptr->ph_info[ modem_id ].rtre_control != CM_RTRE_CONTROL_RUIM )
  {
    rtre_control_is_nv = TRUE;
  }

  if ( lock_ph_mutex )
  {
    QCRIL_MUTEX_UNLOCK( &i_ptr->ph_mutex, details );
  }

  return rtre_control_is_nv;

} /* qcril_arb_rtre_control_is_nv */


/*=========================================================================
  FUNCTION:  qcril_arb_jpn_band_is_supported

===========================================================================*/
/*!
    @brief
    Indicates whether JCDMA is supported.

    @return
     True if JPN band is supported,
     False otherwise.
*/
/*=========================================================================*/
boolean  qcril_arb_jpn_band_is_supported
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  boolean jpn_band_is_supported = FALSE;
  qcril_cm_struct_type *i_ptr;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_arb.cm_info[ instance_id ];

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "cm_info[%d].ph_mutex\n", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->ph_mutex, details );

   if ( ( i_ptr->ph_info[ modem_id ].band_capability & QCRIL_CM_BAND_PREF_JPN ) == QCRIL_CM_BAND_PREF_JPN )
   {
     QCRIL_LOG_DEBUG( "RID %d JPN band is supported\n", instance_id );
     jpn_band_is_supported = TRUE;
   }
   else
   {
     QCRIL_LOG_DEBUG( "RID %d JPN band is not supported\n", instance_id );
   }

   QCRIL_MUTEX_UNLOCK( &i_ptr->ph_mutex, details );

   return jpn_band_is_supported;

} /* qcril_arb_jpn_band_is_supported */


/*=========================================================================
  FUNCTION:  qcril_arb_in_airplane_mode

===========================================================================*/
/*!
    @brief
    Query if the phone is in Airplane mode.

    @return
    TRUE if the phone is in airplane mode.
    FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_arb_in_airplane_mode
( 
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  qcril_cm_struct_type *i_ptr;
  boolean in_airplane_mode = FALSE;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  i_ptr = &qcril_arb.cm_info[ instance_id ];

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "cm_info[%d].ph_mutex\n", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->ph_mutex, details );

  if ( i_ptr->ph_info[ modem_id ].oprt_mode == SYS_OPRT_MODE_LPM )
  {
    QCRIL_LOG_DEBUG( "RID %d In airplane mode\n", instance_id );
    in_airplane_mode = TRUE;
  }
  else
  {
    QCRIL_LOG_DEBUG( "%s\n", "Not in airplane mode" );
  }

  QCRIL_MUTEX_UNLOCK( &i_ptr->ph_mutex, details );

  return in_airplane_mode;

} /* qcril_arb_in_airplane_mode */


/*=========================================================================
  FUNCTION:  qcril_arb_in_emerg_cb_mode

===========================================================================*/
/*!
    @brief
    Query if the phone is in ECBM (Emergency CallBack Mode).

    @return
    TRUE if the phone is in ECBM
    FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_arb_in_emerg_cb_mode
( 
  qcril_instance_id_e_type instance_id
)
{
  qcril_cm_struct_type *i_ptr;
  boolean in_ecbm_state = FALSE;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_arb.cm_info[ instance_id ];

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_cm[%d].emer_cb_state_mutex", instance_id );

  QCRIL_MUTEX_LOCK( &i_ptr->emer_cb_state_mutex, details );

  if ( i_ptr->emer_cb_state == CM_PH_STATE_EMERG_CB )
  {
    QCRIL_LOG_DEBUG( "RID %d In emergency callback mode\n", instance_id );
    in_ecbm_state = TRUE;
  }

  QCRIL_LOG_DEBUG( "RID %d Not in emergency callback mode\n", instance_id );

  QCRIL_MUTEX_UNLOCK( &i_ptr->emer_cb_state_mutex, details );

  return in_ecbm_state;

} /* qcril_arb_in_emerg_cb_mode */


/*=========================================================================
  FUNCTION:  qcril_arb_voip_is_enabled

===========================================================================*/
/*!
    @brief
    Query if VoIP is enabled.

    @return
    TRUE if VoIP is enabled.
    FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_arb_voip_is_enabled
(
  void
)
{
  boolean voip_is_enabled = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  if ( qcril_arb.voip_enabled )
  {
    voip_is_enabled = TRUE;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  return voip_is_enabled;

} /* qcril_arb_voip_is_enabled */


/*=========================================================================
  FUNCTION:  qcril_arb_voip_is_supported

===========================================================================*/
/*!
    @brief
    Query if VoIP is supported.

    @return
    TRUE if VoIP is supported.
    FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_arb_voip_is_supported
(
  void
)
{
  qcril_sms_struct_type *i_ptr = &qcril_arb.sms_info[ QCRIL_DEFAULT_INSTANCE_ID ];
  boolean voip_supported = FALSE;
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  #ifdef FEATURE_QCRIL_IMS
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].transport_reg_info_mutex", QCRIL_DEFAULT_INSTANCE_ID );
  QCRIL_MUTEX_LOCK( &i_ptr->transport_reg_info_mutex, details );

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_LOG_DEBUG( "voip_enabled = %d, net_pref = %d\n", qcril_arb.voip_enabled, qcril_arb.net_pref[ QCRIL_DEFAULT_INSTANCE_ID ] );

  if ( qcril_arb.voip_enabled )
  {
    switch( qcril_arb.net_pref[ QCRIL_DEFAULT_INSTANCE_ID ] )
    {
      case QCRIL_CM_NET_PREF_EVDO_ONLY:
      case QCRIL_CM_NET_PREF_LTE_ONLY:
        /* Split modem (QC), IMS service available */
        if ( ( qcril_arb.ma == QCRIL_ARB_MA_FUSION_QCS ) && 
             i_ptr->transport_reg_info[ QCRIL_SECOND_MODEM_ID ].is_registered )
        {
          voip_supported = TRUE;
        }
        break;

      default:
        break;
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_reg_info_mutex, details );
  #endif /* FEATURE_QCRIL_IMS */

  QCRIL_LOG_DEBUG( "voip_supported = %d\n", voip_supported );

  return voip_supported;

} /* qcril_arb_voip_is_supported */
