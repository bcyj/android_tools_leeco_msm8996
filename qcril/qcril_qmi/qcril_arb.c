/******************************************************************************
  @file    qcril_arb.c
  @brief   qcril qmi - compatibility layer for Fusion

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


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
#include "qcril_cm_util.h"
#include "qcril_log.h"
#include "qcril_arb.h"
#include "qmi_wds_srvc.h"
#include "qcril_qmi_nas.h"

#define QCRIL_ARB_NETWORK_INFO_LEN 10 /* same defintion as QCRIL_NETWORK_INFO_LEN
                                         from qcril_data_netctrl.c
                                       */
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
  qcril_arb_pref_data_tech_e_type pref_data_tech[ QCRIL_MAX_INSTANCE_ID ];     /* Preferred data tech for Fusion architecture*/
  qcril_other_struct_type         other_info[ QCRIL_MAX_INSTANCE_ID ];         /* QCRIL(OTHER) data */
  qcril_sms_struct_type           sms_info[ QCRIL_MAX_INSTANCE_ID ];           /* QCRIL(SMS) data */
  qcril_arb_state_struct_type     state;                                       /* State data */
  qmi_wds_data_sys_status_type    data_sys_status[ QCRIL_MAX_INSTANCE_ID ];
  boolean                         is_current[QCRIL_MAX_INSTANCE_ID];
  dsd_system_status_ind_msg_v01   dsd_system_status;
  boolean                         is_dsd[QCRIL_MAX_INSTANCE_ID];
} qcril_arb_struct_type;


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* Cache QCRIL instances' data */
static qcril_arb_struct_type qcril_arb;



static char *qcril_arb_pref_data_tech_name[] =
  { "Unknown", "CDMA", "EVDO", "GSM", "UMTS", "EHRPD", "LTE", "TDSCDMA", "<invalid slot>" };

/* dynamically allocate memory in qcril_arb_init could be better approach than
   static array. But there is no appropriate function memory free can be called
 */
static qmi_wds_data_sys_status_network_info_type data_sys_status_array[QCRIL_MAX_INSTANCE_ID][QCRIL_ARB_NETWORK_INFO_LEN];
static qcril_arb_pref_data_snapshot_type qcril_arb_pref_data_snapshot;
/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

static void qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change();
static int qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status();
static int qcril_qmi_convert_rat_mask_to_technology(qcril_arb_pref_data_type *pref_data);
static void qcril_arb_current_data_technology_helper();

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

  int iter_i = 0;
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  /* Initialize internal data cache */
  memset( &qcril_arb, 0, sizeof( qcril_arb ) );

  for( iter_i = 0; iter_i < QCRIL_MAX_INSTANCE_ID; iter_i++ )
  {
    qcril_arb.data_sys_status[iter_i].network_info = &data_sys_status_array[iter_i][0];
  }

  memset(&qcril_arb_pref_data_snapshot, 0, sizeof(qcril_arb_pref_data_snapshot));
  /* Initialize mutex */
  pthread_mutex_init( &qcril_arb.mutex, NULL );

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

    case QCRIL_ARB_CACHE_SMS:
      cache_ptr = (void *) &qcril_arb.sms_info;
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

  return 1;

} /* qcril_arb_query_max_num_of_modems */



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

  return 1;

} /* qcril_arb_query_max_num_of_instances */



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

  if( cdma_modem_id != NULL )
  {
  *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
  }
  if( evdo_modem_id != NULL )
  {
  *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
  }
  if( gwl_modem_id != NULL )
  {
  *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
  }

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

    if( instance_id < QCRIL_MAX_INSTANCE_ID )
    {
      if( cdma_modem_id != NULL )
      {
  *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      if( gw_modem_id != NULL )
      {
  *gw_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
    }

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
  if( instance_id < QCRIL_MAX_INSTANCE_ID )
  {

      if( cdma_modem_id != NULL )
      {
  *cdma_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      if( evdo_modem_id != NULL )
      {
  *evdo_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      if( gwl_modem_id != NULL )
      {
  *gwl_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      if( pref_data_tech_modem_id != NULL )
      {
  *pref_data_tech_modem_id = QCRIL_DEFAULT_MODEM_ID;
      }
      if( pref_data_tech != NULL )
      {
  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
  *pref_data_tech = qcril_arb.pref_data_tech[ instance_id ];
  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
      }
  }

} /* qcril_arb_query_data_tech_modem_id */

//===========================================================================
// qcril_arb_current_data_technology_helper
//===========================================================================
void qcril_arb_current_data_technology_helper()
{
    qcril_unsol_resp_params_type        unsol_resp;
    qmi_ril_gen_operational_status_type cur_status;

    cur_status = qmi_ril_get_operational_status();

    QCRIL_LOG_INFO( "operational status %d", (int)cur_status);

    switch( cur_status )
    {
      case QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING:     // fallthrough
      case QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED:     // fallthrough
      case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING:
          // Calculate Data rte and initiate voice radio tech calculation
          qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change();
          if( TRUE == qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status() )
          {
              qmi_ril_nw_reg_data_pref_changed_action();
          }
          // Network information updated, send unsolicited network state changed indication
          qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED, &unsol_resp );
          qcril_send_unsol_response( &unsol_resp );
          break;

      default:
          break;
    }
} //qcril_arb_current_data_technology_helper

/*=========================================================================
  FUNCTION:  qcril_arb_set_dsd_sys_status

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup. This is
    called upon receipt of
    QMI_DSD_REPORT_SYSTEM_STATUS_IND_V01/QMI_DSD_GET_SYSTEM_STATUS_RESP_V01.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_set_dsd_sys_status(dsd_system_status_ind_msg_v01 *dsd_system_status)
{
    uint32_t i;
    uint8_t is_wwan_available = FALSE;
    qcril_arb_pref_data_type old_pref_data;
    qcril_arb_pref_data_type new_pref_data;
    qcril_instance_id_e_type cur_instance = qmi_ril_get_process_instance_id();

    memset( &old_pref_data, 0, sizeof( old_pref_data ) );
    memset( &new_pref_data, 0, sizeof( new_pref_data ) );

    qcril_qmi_get_pref_data_tech(&old_pref_data);

    if( dsd_system_status )
    {
        QCRIL_LOG_INFO( "available systems info valid %d", dsd_system_status->avail_sys_valid );

        if( TRUE == dsd_system_status->avail_sys_valid )
        {
            QCRIL_LOG_DEBUG("dsd_sys_status len=%d", dsd_system_status->avail_sys_len);

            QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

            memcpy(&qcril_arb.dsd_system_status,  dsd_system_status, sizeof(qcril_arb.dsd_system_status));
            qcril_arb.is_current[ cur_instance ] = TRUE;
            qcril_arb.is_dsd[ cur_instance ] = TRUE;

            QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

            qcril_qmi_get_pref_data_tech(&new_pref_data);

            QCRIL_LOG_ESSENTIAL("old_pref_data %d - new_data_pref %d", old_pref_data, new_pref_data);

            QCRIL_LOG_ESSENTIAL("preferred dsd_sys_status nw=0x%x, rat_value=0x%x, so_mask=0x%016llx",
                        dsd_system_status->avail_sys[0].technology,
                        dsd_system_status->avail_sys[0].rat_value,
                        dsd_system_status->avail_sys[0].so_mask);

            if( 0 != memcmp( &old_pref_data, &new_pref_data, sizeof(qcril_arb_pref_data_type) ) )
            {

                for(i = 0; i < dsd_system_status->avail_sys_len; i++) //The first entry in the list will be the preferred system
                {
                    QCRIL_LOG_ESSENTIAL("preferred %d - dsd_sys_status nw=0x%x, rat_value=0x%x, so_mask=0x%016llx",
                                (QMI_RIL_ZERO == i),
                                dsd_system_status->avail_sys[i].technology,
                                dsd_system_status->avail_sys[i].rat_value,
                                dsd_system_status->avail_sys[i].so_mask);
                }

                if(dsd_system_status->avail_sys_len &&
                   DSD_SYS_RAT_EX_3GPP_WLAN_V01 == dsd_system_status->avail_sys[0].rat_value)
                {
                    for(i = 1; i < dsd_system_status->avail_sys_len; i++)
                    {
                        if((DSD_SYS_NETWORK_WLAN_V01 != dsd_system_status->avail_sys[i].technology) &&
                           (DSD_SYS_RAT_EX_NULL_BEARER_V01 != dsd_system_status->avail_sys[i].rat_value))
                        {
                            is_wwan_available = TRUE;
                            break;
                        }
                    }
                }

                QCRIL_LOG_ESSENTIAL("is_wwan_available %d",is_wwan_available);
                qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_WWAN_AVAILABLE, (char*) &is_wwan_available, sizeof(    is_wwan_available));

                // Drop sig info cache if not extrapolating in screen off state,
                if ( qcril_qmi_ril_domestic_service_is_screen_off() && !new_pref_data.is_extrapolation )
                {
                  qcril_qmi_drop_sig_info_cache();
                }
                qmi_ril_nw_reg_data_sys_update_pre_update_action();

                qcril_arb_current_data_technology_helper();
            }
        }
    }
    else
    {
        QCRIL_LOG_FATAL("Null pointer passed for dsd_system_status");
    }
}

/*=========================================================================
  FUNCTION:  qcril_arb_set_data_sys_status

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup. This is
    newer version of qcril_arb_set_pref_data_tech, we pass over all info
    returned from modem to upper layer

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_set_data_sys_status
(
  qcril_instance_id_e_type     instance_id,
  qmi_wds_data_sys_status_type* data_sys_status
)
{
    unsigned int i;
    int data_len;
    qcril_instance_id_e_type cur_instance = qmi_ril_get_process_instance_id();

    QCRIL_LOG_INFO( "qcril_arb_set_data_sys_status instance:%d", (int)instance_id);

    QCRIL_LOG_DEBUG("recevd data_sys_status pref_network=0x%x, len=%d",
                    data_sys_status->pref_network,
                    data_sys_status->network_info_len);

    for(i = 0; i < data_sys_status->network_info_len; i++)
    {
      QCRIL_LOG_ESSENTIAL("recvd data_sys_status nw=0x%x, rat_mask=0x%x, so_mask=0x%x",
                      data_sys_status->network_info[i].network,
                      data_sys_status->network_info[i].rat_mask,
                      data_sys_status->network_info[i].db_so_mask );
    }

    if ( instance_id < QCRIL_MAX_INSTANCE_ID )
    {
        qmi_ril_nw_reg_data_sys_update_pre_update_action();

        QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

        qcril_arb.data_sys_status[ QCRIL_DEFAULT_INSTANCE_ID ].pref_network = data_sys_status->pref_network;
        data_len = (data_sys_status->network_info_len > QCRIL_ARB_NETWORK_INFO_LEN )? QCRIL_ARB_NETWORK_INFO_LEN:
                    data_sys_status->network_info_len;
        qcril_arb.data_sys_status[ QCRIL_DEFAULT_INSTANCE_ID ].network_info_len = data_len;

        memcpy(qcril_arb.data_sys_status[QCRIL_DEFAULT_INSTANCE_ID].network_info, data_sys_status->network_info,
               sizeof(qmi_wds_data_sys_status_network_info_type) * data_len);

        qcril_arb.is_current[ cur_instance ] = TRUE;
        qcril_arb.is_dsd[ cur_instance ] = FALSE;

        QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
        qcril_arb_current_data_technology_helper();
    }
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
  qcril_instance_id_e_type cur_instance = qmi_ril_get_process_instance_id();

  QCRIL_LOG_ESSENTIAL( "qcril_arb_set_pref_data_tech action new tech %d for instance %d", (int) pref_data_tech, (int) instance_id);

  if ( instance_id < QCRIL_MAX_INSTANCE_ID )
  {
      qmi_ril_nw_reg_data_sys_update_pre_update_action();

      QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
      qcril_arb.pref_data_tech[ cur_instance ] = pref_data_tech;
      qcril_arb.is_current[ cur_instance ] = FALSE;
      QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
      qcril_arb_current_data_technology_helper();
  }

} // qcril_arb_set_pref_data_tech

//===========================================================================
// qcril_qmi_get_pref_data_tech
//===========================================================================
void qcril_qmi_get_pref_data_tech(qcril_arb_pref_data_type *pref_data)
{
    unsigned int iter_i=0;
    qcril_instance_id_e_type cur_instance;

    cur_instance = qmi_ril_get_process_instance_id();
    if( pref_data )
    {
        pref_data->is_extrapolation = FALSE;

        QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

        QCRIL_LOG_DEBUG( "is_current %d", qcril_arb.is_current[cur_instance]);

        if( TRUE == qcril_arb.is_current[cur_instance] )
        {
            pref_data->is_current = TRUE;
            pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
            QCRIL_LOG_DEBUG( "is_dsd %d", qcril_arb.is_dsd[cur_instance]);
            if( TRUE == qcril_arb.is_dsd[cur_instance] )
            {
                pref_data->is_dsd = TRUE;
                pref_data->dsd_sys_status_info.technology = qcril_arb.dsd_system_status.avail_sys[QMI_RIL_ZERO].technology; //The first entry in the list will be the preferred system
                pref_data->dsd_sys_status_info.rat_value = qcril_arb.dsd_system_status.avail_sys[QMI_RIL_ZERO].rat_value;
                pref_data->dsd_sys_status_info.so_mask = qcril_arb.dsd_system_status.avail_sys[QMI_RIL_ZERO].so_mask;

                QCRIL_LOG_DEBUG( "technology %d rat_value %x so_mask 0x%016llx",
                                 pref_data->dsd_sys_status_info.technology,
                                 pref_data->dsd_sys_status_info.rat_value,
                                 pref_data->dsd_sys_status_info.so_mask);
            }
            else
            {
                pref_data->is_dsd = FALSE;
                pref_data->data_sys_status_info.network_type = QMI_WDS_DATA_NETWORK_TYPE_INVALID;
                for( iter_i=0; iter_i < qcril_arb.data_sys_status[cur_instance].network_info_len; iter_i++ )
                {
                    if( qcril_arb.data_sys_status[cur_instance].pref_network ==
                        qcril_arb.data_sys_status[cur_instance].network_info[iter_i].network )
                    {
                        pref_data->data_sys_status_info.network_type = qcril_arb.data_sys_status[ cur_instance ].network_info[iter_i].network;
                        pref_data->data_sys_status_info.rat_mask = qcril_arb.data_sys_status[ cur_instance ].network_info[iter_i].rat_mask;
                        pref_data->data_sys_status_info.db_so_mask = qcril_arb.data_sys_status[ cur_instance ].network_info[iter_i].db_so_mask;
                        break;
                    }
                }
                QCRIL_LOG_DEBUG( "network_type %d rat_mask %x db_so_mask %x",
                                 pref_data->data_sys_status_info.network_type,
                                 pref_data->data_sys_status_info.rat_mask,
                                 pref_data->data_sys_status_info.db_so_mask);
            }
        }
        else
        {
            pref_data->is_current = FALSE;
            pref_data->pref_data_tech = qcril_arb.pref_data_tech[ cur_instance ];
        }


        QCRIL_LOG_DEBUG( "before translation : pref_data_tech %s",qcril_qmi_util_retrieve_pref_data_tech_name(pref_data->pref_data_tech));

        if( TRUE == pref_data->is_current )
        {
            pref_data->radio_technology = qcril_qmi_convert_rat_mask_to_technology(pref_data);
            qcril_arb_pref_data_snapshot.snapshot_radio_technology = pref_data->radio_technology;
            switch( pref_data->radio_technology )
            {
                case RADIO_TECH_HSDPA: //HSDPA, HSUPA, HSPA and HSPAP can belong to either TDSCDMA or WCDMA
                case RADIO_TECH_HSUPA: //DsD layer ORs the rat_mask with TDSCDMA or WCDMA (mutually exclusive) to discern between the two
                case RADIO_TECH_HSPA:
                case RADIO_TECH_HSPAP:
                    if ( pref_data->is_dsd ) // dsd
                    {
                        switch(pref_data->dsd_sys_status_info.rat_value)
                        {
                            case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
                                pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
                                break;
                            case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
                                pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                                break;
                            default:
                                // ideally should not come here when radio tech is HS*,
                                // just in case set it to umts
                                QCRIL_LOG_DEBUG( "rat_value is not tdscdma/umts, set pref_data_tech to umts" );
                                pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                                break;
                        }
                    }
                    else // wds
                    {
                        if( pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_TDSCDMA )
                        {
                            pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
                        }
                        else
                        {
                            pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                        }
                    }
                    break;

                case RADIO_TECH_UMTS:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                    break;

#ifndef QMI_RIL_UTF
                case RADIO_TECH_TD_SCDMA:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
                    break;
#endif

                case RADIO_TECH_GPRS:
                case RADIO_TECH_EDGE:
                case RADIO_TECH_GSM:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_GSM;
                    break;

                case RADIO_TECH_LTE:
                case RADIO_TECH_IWLAN:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_LTE;
                    break;

                case RADIO_TECH_IS95A:
                case RADIO_TECH_IS95B:
                case RADIO_TECH_1xRTT:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_CDMA;
                    break;

                case RADIO_TECH_EVDO_0:
                case RADIO_TECH_EVDO_A:
                case RADIO_TECH_EVDO_B:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_EVDO;
                    break;

                case RADIO_TECH_EHRPD:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_EHRPD;
                    break;

                case RADIO_TECH_UNKNOWN:
                default:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
                    break;
            }
        }

        QCRIL_LOG_DEBUG( "after translation : pref_data_tech %s technology %d", qcril_qmi_util_retrieve_pref_data_tech_name(pref_data->pref_data_tech),
                                                                                    pref_data->radio_technology);

        QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    }

} // qcril_qmi_get_pref_data_tech

//===========================================================================
// qcril_qmi_convert_rat_mask_to_technology
//===========================================================================
int qcril_qmi_convert_rat_mask_to_technology(qcril_arb_pref_data_type *pref_data)
{
    int technology = RADIO_TECH_UNKNOWN;

    QCRIL_LOG_FUNC_ENTRY();
    if( pref_data )
    {
        pref_data->is_extrapolation = FALSE;
        if( TRUE == pref_data->is_current )
        {
            if( TRUE == pref_data->is_dsd )
            {
                switch( pref_data->dsd_sys_status_info.rat_value )
                {
                    case DSD_SYS_RAT_EX_NULL_BEARER_V01:
                        technology = qcril_arb_pref_data_snapshot.snapshot_radio_technology;
                        pref_data->is_extrapolation = TRUE;
                        break;

                    case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
                    case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPAPLUS_V01)
                            || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                            || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_64_QAM_V01) )
                        {
                            technology = RADIO_TECH_HSPAP;
                        }
                        else if( ((pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01)
                                 && (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01))
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSPA_V01) )
                        {
                            technology = RADIO_TECH_HSPA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01) )
                        {
                            technology = RADIO_TECH_HSDPA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01) )
                        {
                            technology = RADIO_TECH_HSUPA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_WCDMA_V01) )
                        {
                            technology = RADIO_TECH_UMTS;
                        }
                        else if ( (pref_data->dsd_sys_status_info.rat_value == DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01) )
                        {
#ifndef QMI_RIL_UTF
                            technology = RADIO_TECH_TD_SCDMA;
#endif
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP_GERAN_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_EDGE_V01) )
                        {
                            technology = RADIO_TECH_EDGE;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_GPRS_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_GSM_V01) )
                        {
                            technology = RADIO_TECH_GPRS;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP_LTE_V01:
                        technology = RADIO_TECH_LTE;
                        break;

                    case DSD_SYS_RAT_EX_3GPP_WLAN_V01:
                        technology = RADIO_TECH_IWLAN;
                        break;

                    case DSD_SYS_RAT_EX_3GPP2_1X_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_REL_A_V01)
                            || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_V01) )
                        {
                            technology = RADIO_TECH_1xRTT;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS95_V01) )
                        {
                            technology = RADIO_TECH_IS95A;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP2_HRPD_V01:
                        if ( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_DPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MMPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_EMPA_V01) )
                        {
                            technology = RADIO_TECH_EVDO_B;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_DPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_MPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_EMPA_V01) )
                        {
                            technology = RADIO_TECH_EVDO_A;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REV0_DPA_V01) )
                        {
                            technology = RADIO_TECH_EVDO_0;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP2_EHRPD_V01:
                            technology = RADIO_TECH_EHRPD;
                        break;

                    default: //no action
                        break;
                }
            }
            else
            {
                switch( pref_data->data_sys_status_info.network_type )
                {
                    case QMI_WDS_DATA_NETWORK_TYPE_3GPP:
                        if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_LTE) )
                        {
                            technology = RADIO_TECH_LTE;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_HSDPA_PLUS)
                                 || (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_DC_HSDPA_PLUS)
                                 || (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_64_QAM) )
                        {
                            technology = RADIO_TECH_HSPAP;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_HSDPA)
                                 && (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_HSUPA) )
                        {
                            technology = RADIO_TECH_HSPA;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_HSDPA) )
                        {
                            technology = RADIO_TECH_HSDPA;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_HSUPA) )
                        {
                            technology = RADIO_TECH_HSUPA;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_WCDMA) )
                        {
                            technology = RADIO_TECH_UMTS;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_TDSCDMA) )
                        {
#ifndef QMI_RIL_UTF
                            technology = RADIO_TECH_TD_SCDMA;
#endif
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_EDGE) )
                        {
                            technology = RADIO_TECH_EDGE;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_GPRS) )
                        {
                            technology = RADIO_TECH_GPRS;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.umts_rat_mask & UMTS_NULL_BEARER) )
                        {
                            technology = qcril_arb_pref_data_snapshot.snapshot_radio_technology;
                            pref_data->is_extrapolation = TRUE;
                        }
                        break;

                    case QMI_WDS_DATA_NETWORK_TYPE_3GPP2:
                        if( (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_EHRPD) )
                        {
                            technology = RADIO_TECH_EHRPD;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_EVDO_REVB) )
                        {
                            technology = RADIO_TECH_EVDO_B;
                            if( (pref_data->data_sys_status_info.db_so_mask.so_mask_evdo_revb & CDMA_EVDO_REVB_EMPA_EHRPD)
                                || (pref_data->data_sys_status_info.db_so_mask.so_mask_evdo_revb & CDMA_EVDO_REVB_MMPA_EHRPD) )
                            {
                              technology = RADIO_TECH_EHRPD;
                            }
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_EVDO_REVA) )
                        {
                            technology = RADIO_TECH_EVDO_A;
                            if( (pref_data->data_sys_status_info.db_so_mask.so_mask_evdo_reva & CDMA_EVDO_REVA_EMPA_EHRPD) )
                            {
                              technology = RADIO_TECH_EHRPD;
                            }
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_EVDO_REV0) )
                        {
                            technology = RADIO_TECH_EVDO_0;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_1X)
                                 || (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_FMC) )
                        {
                            technology = RADIO_TECH_1xRTT;
                        }
                        else if( (pref_data->data_sys_status_info.rat_mask.cdma_rat_mask & CDMA_NULL_BEARER) )
                        {
                            technology = qcril_arb_pref_data_snapshot.snapshot_radio_technology;
                            pref_data->is_extrapolation = TRUE;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }
    QCRIL_LOG_INFO("technology %s", qcril_qmi_util_retrieve_technology_name(technology));


    QCRIL_LOG_FUNC_RETURN_WITH_RET(technology);
    return technology;
} // qcril_qmi_convert_rat_mask_to_technology

//===========================================================================
// qcril_qmi_arb_reset_pref_data_snapshot
//===========================================================================
void qcril_qmi_arb_reset_pref_data_snapshot()
{
    qcril_instance_id_e_type cur_instance;

    QCRIL_LOG_FUNC_ENTRY();

    cur_instance = qmi_ril_get_process_instance_id();
    QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    memset(&qcril_arb_pref_data_snapshot, 0, sizeof(qcril_arb_pref_data_snapshot));

    if ( !qcril_arb.is_current[cur_instance] && QCRIL_ARB_PREF_DATA_TECH_UNKNOWN == qcril_arb.pref_data_tech[ cur_instance ] )
    {
        qcril_arb.pref_data_tech[ cur_instance ] = QCRIL_ARB_PREF_DATA_TECH_INVALID;
    }

    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_arb_reset_pref_data_snapshot

//===========================================================================
// qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change
//===========================================================================
void qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change()
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_nas_evaluate_data_rte_on_pref_data_tech_change();
    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change

//===========================================================================
// qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status
//===========================================================================
int qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status()
{
    int ret;

    QCRIL_LOG_FUNC_ENTRY();
    ret = qcril_qmi_nas_check_power_save_and_screen_off_status();
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} //qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status

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

  if( instance_id < QCRIL_MAX_INSTANCE_ID &&  modem_ids_list_ptr != NULL)
  {
  if ( ph_srv_cat > QCRIL_ARB_PH_SRV_CAT_COMMON )
  {
    QCRIL_LOG_ERROR( "ph srv category %d not supported", ph_srv_cat );
    status = E_NOT_SUPPORTED;
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
  }
  else
  {
  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;
  }
  }
  else
  {
      status = E_NOT_ALLOWED;
  }

  return status;

} /* qcril_arb_query_ph_srv_modem_id */


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

  IxErrnoType status = E_SUCCESS;

  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;
  sms_srv_cat = sms_srv_cat;
  instance_id = instance_id;

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

  if(instance_id < QCRIL_MAX_INSTANCE_ID && modem_id_ptr != NULL)
  {
      *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
  }
  else
  {
      status = E_NOT_ALLOWED;
  }


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

  if( instance_id < QCRIL_MAX_INSTANCE_ID && modem_ids_list_ptr != NULL )
  {
  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;
  nv_srv_cat = nv_srv_cat;
  }
  else
  {
      status = E_NOT_ALLOWED;
  }

  return status;

} /* qcril_arb_query_nv_srv_modem_id */


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

  return ma_is_dsds;

} /* qcril_arb_ma_is_dsds */


/*=========================================================================
  FUNCTION:  qcril_arb_in_dsds_ssmm

===========================================================================*/
/*!
    @brief
    Indicates whether the DSDS preference is single standby multimode.

    @return
     True if Multimode is the current DSDS preference,
     False otherwise.
*/
/*=========================================================================*/
boolean qcril_arb_in_dsds_ssmm
(
  void
)
{
  boolean dsds_pref_is_mm = FALSE;

  /*-----------------------------------------------------------------------*/
  return dsds_pref_is_mm;

} /* qcril_arb_in_dsds_ssmm */




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

  instance_id = instance_id;
  modem_id = modem_id;
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
  boolean in_airplane_mode = FALSE;
  instance_id = instance_id;
  modem_id = modem_id;
  return in_airplane_mode;

} /* qcril_arb_in_airplane_mode */


