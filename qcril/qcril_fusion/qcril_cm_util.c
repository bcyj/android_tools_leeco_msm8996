/*!
  @file
  qcril_cm_util.c

  @brief
  Utilities functions to support QCRIL_CM processing.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_util.c#24 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
11/12/09   sb      Added sanity timer to info rec processing.
11/02/09   sb      Fixed parsing of blank and skip tags in Ext Disp info rec.
09/21/09   pg      Report UNSOL_NETWORK_STATE_CHANGE when HDR active protocol
                   is updated.
                   Fixed Registration State info when phone is in Hybrid mode 
                   and only HDR is available.
                   Fixed data type mismatched issue in Rgistration State info
                   response.
07/23/09   sb      Cleaned up log messages in info rec processing.
07/22/09   sb      Added support for latest ril.h under FEATURE_NEW_RIL_API.
07/17/09   pg      Added default support for CDMA registration reject cause.
06/26/09   pg      Added support to read from modem whether current system is PRL matched.
06/15/09   nd      Added support to check the CDMA emergency flash number in ECC property file.
06/15/09   nd      Added support for CDMA Time of Day.
06/16/09   fc      Changes on debug messages.
06/06/09   nrn     Adding support for Authentication and Registration Reject
05/30/09   pg      Fixed Hybrid mode registration informations.
                   Verify set_preferred_network_mode request for 1XEVDO without
                   checking the returned acq_ord_pref field.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
                   Mailined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros.
03/17/09   fc      Move all ONS support to qcril_cm_ons.c.
                   Cleanup unreferenced header filed inclusion.
                   Mainlined FEATURE_CM_UTIL_RPC_AVAIL.
02/04/09   pg      Call cmutil RPC APIs when FEATURE_CM_UTIL_RPC_AVAIL is
                   defined.
01/26/08   fc      Logged assertion info.
01/14/06   fc      Changes to report "Limited Service" as "No Service" in
                   (GPRS) Registration State payload.
12/24/08   fc      Constrained the reported CID value to be in the range of 
                   0x00000000 - 0x7fffffff since Android choke on any value
                   greater than this range though ril.h claimed that it 
                   supports 0x00000000 - 0xffffffff.
10/23/08   pg      Added EVDO Rev.0 as a separate value from EVDO Rev.A in
                   RIL_REQUEST_REGISTRATION_STATE.
05/22/08   tml     Fixed compilation issue with LTK
05/08/08   fc      First cut implementation.


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <math.h>
#include <cutils/memory.h>
#include <string.h>
#include <cutils/properties.h>
#include <errno.h>
#include "comdef.h"
#include "nv.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_cm.h"
#include "qcril_cmi.h"
#include "qcril_cm_util.h"
#include "qcril_cm_clist.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/




/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/



/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_cm_util_map_nv_to_cm_rtre

===========================================================================*/
/*!
    @brief
    Map NV RTRE enum to CM RTRE enum.

    @return
    None.
*/
/*=========================================================================*/
cm_rtre_config_e_type qcril_cm_util_map_nv_to_cm_rtre
(
  uint32 nv_rtre
)
{
  cm_rtre_config_e_type cm_rtre;

  /*-----------------------------------------------------------------------*/

  switch (nv_rtre)
  {
    case NV_RTRE_CONFIG_SIM_ACCESS:
      cm_rtre = CM_RTRE_CONFIG_SIM_ACCESS;
      break;

    case NV_RTRE_CONFIG_RUIM_ONLY:
      cm_rtre = CM_RTRE_CONFIG_RUIM_ONLY;
      break;

    case NV_RTRE_CONFIG_NV_ONLY:
      cm_rtre = CM_RTRE_CONFIG_NV_ONLY;
      break;

    case NV_RTRE_CONFIG_RUIM_OR_DROP_BACK:
    default:
       cm_rtre = CM_RTRE_CONFIG_RUIM_OR_DROP_BACK;
       break;
   }

   return cm_rtre;

} /* qcril_cm_util_map_nv_to_cm_rtre */


/*=========================================================================
  FUNCTION:  qcril_cm_util_rssi_to_gw_sigal_strength

===========================================================================*/
/*!
    @brief
    Convert the RSSI to signal strength info.

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_rssi_to_gw_signal_strength
(
  uint16 rssi,
  int *signal_strength_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( signal_strength_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* RSSI dbm is actually a negative value

     RSSI                              Signal Strength
     ------                            ---------------
     -113 or less                      0
     -111                              1
     -109 ... -53                      2 ... 30 
     -51 or greater                    31
     not known or not detected         99
  */
  if ( ( QCRIL_CM_RSSI_MIN < rssi ) && ( QCRIL_CM_RSSI_MAX > rssi ) )
  {
    *signal_strength_ptr = (int) ( floor( ( ( rssi * QCRIL_CM_RSSI_SLOPE + QCRIL_CM_RSSI_OFFSET ) *
                                            QCRIL_CM_RSSI_TOOHI_CODE ) / 100 + 0.5 ) );
  }
  else if ( ( QCRIL_CM_RSSI_MAX <= rssi ) && ( QCRIL_CM_RSSI_NO_SIGNAL != rssi ) )
  {
    *signal_strength_ptr = QCRIL_CM_RSSI_TOOLO_CODE;  
  }
  else if ( QCRIL_CM_RSSI_MIN >= rssi )
  { 
    *signal_strength_ptr = QCRIL_CM_RSSI_TOOHI_CODE;  
  }
  else 
  { 
    *signal_strength_ptr = QCRIL_CM_GW_SIGNAL_STRENGTH_UNKNOWN;  
  }

} /* qcril_cm_util_rssi_to_gw_signal_strength */ 


/*=========================================================================
  FUNCTION:  qcril_cm_util_srv_sys_info_to_reg_state

===========================================================================*/
/*!
    @brief
    Convert the serving status and roaming status to registration state info.

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_srv_sys_info_to_reg_state
(
  boolean reporting_data_reg_state,
  qcril_arb_pref_data_tech_e_type pref_data_tech,
  char **reg_state_ptr,
  const qcril_cm_ss_info_type *ssi_ptr,
  const cm_mode_pref_e_type mode_pref,
  qcril_cm_reg_reject_info_type *reg_reject_info_ptr
)
{      

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( reg_state_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Initialize reg_state to Unknown */
  *reg_state_ptr = "4";
  
  if ( ( ssi_ptr == NULL ) ||
       ( reporting_data_reg_state && qcril_arb_ma_is_fusion() && ( pref_data_tech == QCRIL_ARB_PREF_DATA_TECH_UNKNOWN ) ) )
  {
    QCRIL_LOG_DEBUG( "NO SERVICE - preferred data tech %d\n", pref_data_tech );
    *reg_state_ptr = "2";
    return;
  }

  QCRIL_LOG_DEBUG( "Current System : srv status %d, sys mode %d, roam status %d, srv domain %d, srv cap %d, IDM %d, hybrid %d, hdr srv status %d managed roaming %d, mode pref %d\n",
                   ssi_ptr->srv_status, ssi_ptr->sys_mode, ssi_ptr->roam_status, ssi_ptr->srv_domain, 
                   ssi_ptr->srv_capability, ssi_ptr->cur_idle_digital_mode, ssi_ptr->hdr_hybrid, 
                   ssi_ptr->hdr_srv_status, reg_reject_info_ptr->managed_roaming_enabled, mode_pref ); 

  /* Non-hybrid, powersave or no service */
  if ( ( ( mode_pref == CM_MODE_PREF_GSM_WCDMA_ONLY ) ||                                                  
         ( mode_pref == CM_MODE_PREF_GSM_ONLY ) ||                                                       
         ( mode_pref == CM_MODE_PREF_WCDMA_ONLY )                                                       
         #ifdef FEATURE_QCRIL_LTE
         || ( mode_pref == CM_MODE_PREF_LTE_ONLY )
         #endif /* FEATURE_QCRIL_LTE */
       ) ||
       ( mode_pref == CM_MODE_PREF_CDMA_ONLY ) ||
       ( ( ( mode_pref == CM_MODE_PREF_DIGITAL_ONLY ) || 
           ( mode_pref == CM_MODE_PREF_CDMA_HDR_ONLY ) || 
           ( mode_pref == CM_MODE_PREF_HDR_ONLY ) 
           #ifdef FEATURE_QCRIL_LTE
           || ( mode_pref == CM_MODE_PREF_HDR_LTE_ONLY )
           #endif /* FEATURE_QCRIL_LTE */
         ) && !ssi_ptr->hdr_hybrid ) ) 
  {
    if ( ssi_ptr->srv_status == SYS_SRV_STATUS_PWR_SAVE ) 
    {
      /* Not registered, MT is not currently searching a new operator to register to */
      QCRIL_LOG_DEBUG( "%s", "Power Save Mode\n" );
      *reg_state_ptr = "0";
      return;
    }

    if ( ssi_ptr->srv_status == SYS_SRV_STATUS_NO_SRV ) 
    {
      /* Not registered, but MT is currently searching a new operator to register to*/
      QCRIL_LOG_DEBUG( "%s", "NO SERVICE\n" );
      *reg_state_ptr = "2";
      return;
    }
  }

  /* Hybrid, powersave or no service */
  if ( ( ( mode_pref == CM_MODE_PREF_CDMA_HDR_ONLY ) || 
         ( mode_pref == CM_MODE_PREF_HDR_ONLY ) || 
         ( mode_pref == CM_MODE_PREF_DIGITAL_ONLY )
         #ifdef FEATURE_QCRIL_LTE
         || ( mode_pref == CM_MODE_PREF_HDR_LTE_ONLY )
         #endif /* FEATURE_QCRIL_LTE */
       ) && ssi_ptr->hdr_hybrid ) 
  {
    if ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_PWR_SAVE ) && 
         ( ssi_ptr->hdr_srv_status == SYS_SRV_STATUS_PWR_SAVE ) )
    {
      /* Not registered, MT is not currently searching a new operator to register to */
      QCRIL_LOG_DEBUG( "%s", "Power Save Mode\n" );
      *reg_state_ptr = "0";
      return;
    }

    if ( ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_NO_SRV ) &&
           ( ssi_ptr->hdr_srv_status == SYS_SRV_STATUS_NO_SRV ) ) ||
         ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_NO_SRV ) &&
           ( ssi_ptr->hdr_srv_status == SYS_SRV_STATUS_PWR_SAVE ) ) ||
         ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_PWR_SAVE ) &&
           ( ssi_ptr->hdr_srv_status == SYS_SRV_STATUS_NO_SRV ) ) )
    {
      /* Not registered, but MT is currently searching a new operator to register to*/
      QCRIL_LOG_DEBUG( "%s", "NO SERVICE\n" );
      *reg_state_ptr = "2";
      return;
    }
  }
   
  /* Full service - GSM, WCDMA or LTE */
  if ( QCRIL_CM_SRV_STATUS_INDICATES_GWL_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) )
  {
    /* Data registration reporting, not registered on PS domain */
    if ( reporting_data_reg_state && QCRIL_CM_SYS_MODE_IS_GWL( ssi_ptr->sys_mode ) &&
              ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_ONLY ) ||
              ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_PS ) &&
              !QCRIL_CM_SRV_DOMAIN_SUPPORT_PS( ssi_ptr->srv_domain ) ) ) )
    {
      /* Not registered, but MT is currently searching a new operator to register to*/
      QCRIL_LOG_DEBUG( "%s", "Not in PS domain, Searching for Service. Emergency calls allowed.\n" );
      *reg_state_ptr = REG_STATE_SEARCHING_EMERGENCY;
    }
    /* Data registration reporting, registered on PS domain */
    else if ( reporting_data_reg_state && QCRIL_CM_SYS_MODE_IS_GWL( ssi_ptr->sys_mode ) &&
              ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_PS_ONLY ) ||
              ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_PS ) &&
              QCRIL_CM_SRV_DOMAIN_SUPPORT_PS( ssi_ptr->srv_domain ) ) ) )
    {
      if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
      {
        /* Registered, home network */
        QCRIL_LOG_DEBUG( "%s", "Registered on Home network\n" );
        *reg_state_ptr = "1";
      }
      else
      {
        /* Registered, roaming */
        QCRIL_LOG_DEBUG( "%s", "Roaming\n" );
        *reg_state_ptr = "5";
      }
    }
    /* Voice registration reporting, Registered on PS domain but registration rejected 4 times in a row for CS domain */
    else if ( !reporting_data_reg_state && QCRIL_CM_SYS_MODE_IS_GWL( ssi_ptr->sys_mode ) &&
              ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_PS ) && ( ssi_ptr->srv_domain == SYS_SRV_DOMAIN_PS_ONLY ) &&
              ( ( reg_reject_info_ptr->managed_roaming_enabled ) || ( reg_reject_info_ptr->cm_reg_reject_info.reject_cause > 0 ) ) )
    {
      /* Registration Denied */
      QCRIL_LOG_DEBUG( "%s", "Registration Denied: LIMITED SERVICE MODE\n" );
      *reg_state_ptr = REG_STATE_DENIED_EMERGENCY;
    }
    /* For voice registration reporting, if not in CS domain, NO SERVICE */
    else if ( !reporting_data_reg_state && QCRIL_CM_SYS_MODE_IS_GWL( ssi_ptr->sys_mode ) &&
              ( !qcril_arb_voip_is_supported() &&
                ( !QCRIL_CM_SRV_DOMAIN_SUPPORT_CS( ssi_ptr->srv_domain ) ||
                  !QCRIL_CM_SRV_CAPABILITY_SUPPORT_CS( ssi_ptr->srv_capability ) ) ) )
    {
      /* Not registered, but MT is currently searching a new operator to register to*/
      *reg_state_ptr = REG_STATE_SEARCHING_EMERGENCY;
      QCRIL_LOG_DEBUG( "reg_state [%s] - Not in CS domain, Emergency calls are allowed", *reg_state_ptr );
    }
    /* For voice registration reporting, Registered on CS domain */
    else if ( !reporting_data_reg_state && QCRIL_CM_SYS_MODE_IS_GWL( ssi_ptr->sys_mode ) &&
              ( qcril_arb_voip_is_supported() ||
                ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_ONLY ) ||
                  ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_PS ) && ( ssi_ptr->srv_domain == SYS_SRV_DOMAIN_CS_ONLY ) ) ||
                  ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_PS ) && ( ssi_ptr->srv_domain == SYS_SRV_DOMAIN_CS_PS ) ) ) ) )
    {
      if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
      {
        /* Registered, home network */
        QCRIL_LOG_DEBUG( "%s", "Registered on Home network\n" );
        *reg_state_ptr = "1";
      }
      else
      {
        /* Registered, roaming */
        QCRIL_LOG_DEBUG( "%s", "Roaming\n" );
        *reg_state_ptr = "5";
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "%s", "Invalid combination of fields.\n" );
      *reg_state_ptr = "4";
    }

    return;
  }

  /* Full service, non-hybrid, CDMA */
  if ( !ssi_ptr->hdr_hybrid &&
       QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode )  )
  {
    if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
    {
      /* Registered, home network */
      QCRIL_LOG_DEBUG( "%s", "Registered on Home network\n" );
      *reg_state_ptr = "1";
    }
    else
    {
      /* Registered, roaming */
      QCRIL_LOG_DEBUG( "%s", "Roaming\n" );
      *reg_state_ptr = "5";
    }

    return;
  }

  /* Full service, non-hybrid DO scenario */
  /* DO applicable only for data */
  if ( !ssi_ptr->hdr_hybrid &&
       QCRIL_CM_SRV_STATUS_INDICATES_HDR_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) &&
       reporting_data_reg_state )
  {
    if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
    {
      /* Registered, home network */
      QCRIL_LOG_DEBUG( "%s", "Registered on Home network\n" );
      *reg_state_ptr = "1";
    }
    else
    {
      /* Registered, roaming */
      QCRIL_LOG_DEBUG( "%s", "Roaming\n" );
      *reg_state_ptr = "5";
    }

    return;
  }
  
  /* Full service, hybrid, CDMA or DO scenario */
  if ( ssi_ptr->hdr_hybrid )
  {
    /* Full service on HDR (both voice and data), IDM is only meaningful for SVDO */
    if ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( ssi_ptr->hdr_srv_status ) &&
         ( ( mode_pref != CM_MODE_PREF_CDMA_HDR_ONLY ) || 
           QCRIL_CM_SYS_MODE_IS_HDR( ssi_ptr->cur_idle_digital_mode ) ) &&
         reporting_data_reg_state ) 
    {
      /* Use hdr_roam_status to determine if AT is roaming */
      if ( ssi_ptr->hdr_roam_status == SYS_ROAM_STATUS_OFF )
      {
        /* Registered, home network */
        QCRIL_LOG_DEBUG( "%s", "Registered on Home network(Hybrid)\n" );
        *reg_state_ptr = "1";
      }
      else
      {
        /* Registered, roaming */
        QCRIL_LOG_DEBUG( "%s", "Roaming(Hybrid)\n" );
        *reg_state_ptr = "5";
      }

      return;
    }

    /* Full service on CDMA (both voice and data), IDM is only meaning for standalone SVDO */
    if ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) &&
         ( ( mode_pref != CM_MODE_PREF_CDMA_HDR_ONLY ) || 
           QCRIL_CM_SYS_MODE_IS_CDMA( ssi_ptr->cur_idle_digital_mode ) ) )
    {
      if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
      {
        /* Registered, home network */
        QCRIL_LOG_DEBUG( "%s", "Registered on Home network(Main)\n" );
        *reg_state_ptr = "1";
      }
      else
      {
        /* Registered, roaming */
        QCRIL_LOG_DEBUG( "%s", "Roaming(Main)\n" );
        *reg_state_ptr = "5";
      }
      
      return;
    }

    /* DO acquisition still in progress, not registered for Data Registration State */
    if ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) &&  
         ( mode_pref == CM_MODE_PREF_CDMA_HDR_ONLY ) && !reporting_data_reg_state )
    {
      if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
      {
        /* Registered, home network */
        QCRIL_LOG_DEBUG( "%s", "Registered on Home network(Main)\n" );
        *reg_state_ptr = "1";
      }
      else
      {
        /* Registered, roaming */
        QCRIL_LOG_DEBUG( "%s", "Roaming(Main)\n" );
        *reg_state_ptr = "5";
      }

      return;
    }
    else if ( !QCRIL_CM_SYS_MODE_IS_GW( ssi_ptr->sys_mode ) )
    {
      QCRIL_LOG_DEBUG( "%s", "NO SERVICE - Hybrid, DO acquisition in progress\n" );
      *reg_state_ptr = "2";
      return;
    }
  }

  /* Limited service, GSM, WCDMA or LTE */
  if ( ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_LIMITED ) ||
         ( ssi_ptr->srv_status == SYS_SRV_STATUS_LIMITED_REGIONAL ) ) &&
       QCRIL_CM_SYS_MODE_IS_GWL( ssi_ptr->sys_mode ) )
  {
    if ( reporting_data_reg_state && ( ssi_ptr->srv_capability != SYS_SRV_DOMAIN_CAMPED ) )
    {
      QCRIL_LOG_DEBUG( "%s", "LIMITED SERVICE, EMERGENCY CALLS ONLY\n" );
      *reg_state_ptr = REG_STATE_SEARCHING_EMERGENCY;
    }
    else if ( !reporting_data_reg_state && QCRIL_CM_SRV_CAPABILITY_SUPPORT_CS( ssi_ptr->srv_capability ) )
    {
      if ( ( reg_reject_info_ptr->managed_roaming_enabled ) || ( reg_reject_info_ptr->cm_reg_reject_info.reject_cause > 0 ) )
      {
        /* Registration Denied */
        QCRIL_LOG_DEBUG( "%s", "Registration Denied: LIMITED SERVICE MODE\n" );
        *reg_state_ptr = REG_STATE_DENIED_EMERGENCY;
      }
      else
      {
        /* Not registered, but MT is currently searching a new operator to register to*/
        *reg_state_ptr = REG_STATE_SEARCHING_EMERGENCY;
        QCRIL_LOG_DEBUG( "reg_state [%s] - Limited service, Emergency calls are allowed", *reg_state_ptr );
      }
    }
    else if ( !reporting_data_reg_state && ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_PS_ONLY ) )
    {
      *reg_state_ptr = REG_STATE_SEARCHING_EMERGENCY;
      QCRIL_LOG_DEBUG( "reg_state [%s] - Limited service (srv_capability=2), Emergency calls are allowed", *reg_state_ptr );
    }
    else
    {
      *reg_state_ptr = REG_STATE_UNKNOWN_EMERGENCY;
      QCRIL_LOG_DEBUG( "reg_state [%s] - Invalid srv capability with srv status limited", *reg_state_ptr );
    }

    return;
  }

  /* Limited service, CDMA or HDR scenario */
  if ( ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_LIMITED ) ||
         ( ssi_ptr->srv_status == SYS_SRV_STATUS_LIMITED_REGIONAL ) ||
         ( ssi_ptr->hdr_srv_status == SYS_SRV_STATUS_LIMITED ) ||
         ( ssi_ptr->hdr_srv_status == SYS_SRV_STATUS_LIMITED_REGIONAL ) ) &&
       QCRIL_CM_SYS_MODE_IS_1XEVDO( ssi_ptr->sys_mode ) )
  {
    /* No service */
    QCRIL_LOG_DEBUG( "%s", "NO SERVICE\n" );
    *reg_state_ptr = "2";
    return;
  }
         
  if ( strcmp( *reg_state_ptr, "4" ) == 0 )
  {
    /* Unknown */
    if ( ( ssi_ptr->srv_status == SYS_SRV_STATUS_NO_SRV ) || ( ssi_ptr->srv_status == SYS_SRV_STATUS_PWR_SAVE ) )
   {
     QCRIL_LOG_DEBUG( "reg_state [%s] - Unknown Registration State, No Service", *reg_state_ptr );
   }
   else
   {
     *reg_state_ptr = REG_STATE_UNKNOWN_EMERGENCY;
     QCRIL_LOG_DEBUG( "reg_state [%s] - Unknown Registration State, Emergency calls are allowed", *reg_state_ptr );
   }
  }
     
} /* qcril_cm_util_srv_sys_info_to_reg_state */


/*=========================================================================
  FUNCTION:  qcril_cm_util_srv_sys_info_to_avail_radio_tech

===========================================================================*/
/*!
    @brief
    Convert the serving status, system mode, service domain and EGPRS 
    support info to available radio technology info.

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_srv_sys_info_to_avail_radio_tech
(
  boolean reporting_data_reg_state,
  const cm_mode_pref_e_type mode_pref,
  char **reg_state_ptr,
  char **avail_radio_tech_ptr,
  const qcril_cm_ss_info_type *ssi_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( reg_state_ptr != NULL );
  QCRIL_ASSERT( avail_radio_tech_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Default to unknown available radio technology */
  *avail_radio_tech_ptr = "0";

  /* Only if registered on network, we report radio tech */
  if ( ( ssi_ptr != NULL ) && 
       ( ( strcmp( *reg_state_ptr, "1" ) == 0 ) || ( strcmp( *reg_state_ptr, "5" ) == 0 ) ) )
  {
    #ifdef FEATURE_QCRIL_LTE
    if ( QCRIL_CM_SRV_STATUS_INDICATES_LTE_FULL_SRV( ssi_ptr->srv_status,  ssi_ptr->sys_mode ) )
    {
      /* LTE is the available radio technology */
      QCRIL_LOG_DEBUG( "%s", "LTE System\n" ); 
      *avail_radio_tech_ptr = "14";
    } 
    else 
    #endif /* FEATURE_QCRIL_LTE */
    /* UMTS */
    if ( QCRIL_CM_SRV_STATUS_INDICATES_WCDMA_FULL_SRV( ssi_ptr->srv_status,  ssi_ptr->sys_mode ) )
    {                                                         
      /* UMTS is the available radio technology */
      QCRIL_LOG_DEBUG( "UMTS System, hs_ind %d, hs_call_status %d\n", ssi_ptr->cell_srv_ind.hs_ind, ssi_ptr->cell_srv_ind.hs_call_status );
      *avail_radio_tech_ptr = "3"; 

      if ( ( ssi_ptr->cell_srv_ind.hs_ind == SYS_HS_IND_HSDPA_SUPP_CELL ) || 
           ( ssi_ptr->cell_srv_ind.hs_call_status == SYS_HS_IND_HSDPA_SUPP_CELL ) )
      {
        /* UMTS is the available radio technology with HSDPA support */
        QCRIL_LOG_DEBUG( "%s", "HSDPA System\n" ); 
        *avail_radio_tech_ptr = "9";
      }
      else if ( ( ssi_ptr->cell_srv_ind.hs_ind == SYS_HS_IND_HSUPA_SUPP_CELL ) || 
                ( ssi_ptr->cell_srv_ind.hs_call_status == SYS_HS_IND_HSUPA_SUPP_CELL ) )
      {
        /* UMTS is the available radio technology with HSUPA support */
        QCRIL_LOG_DEBUG( "%s", "HSUPA System\n" ); 
        *avail_radio_tech_ptr = "10";
      }
      else if ( ( ssi_ptr->cell_srv_ind.hs_ind == SYS_HS_IND_HSDPA_HSUPA_SUPP_CELL ) || 
                ( ssi_ptr->cell_srv_ind.hs_call_status == SYS_HS_IND_HSDPA_HSUPA_SUPP_CELL ) )
      {
        /* UMTS is the available radio technology with HSPA support */
        QCRIL_LOG_DEBUG( "%s", "HSPA System\n" ); 
        *avail_radio_tech_ptr = "11";
      }
    } 
    /* GSM */
    else if ( QCRIL_CM_SRV_STATUS_INDICATES_GSM_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) )
    {
      QCRIL_LOG_DEBUG( "GSM System : srv domain %d, srv capability %d, egprs supp %d\n", 
                       ssi_ptr->srv_domain, ssi_ptr->srv_capability, ssi_ptr->cell_srv_ind.egprs_supp ); 

      if ( ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_PS_ONLY ) || ( ssi_ptr->srv_capability == SYS_SRV_DOMAIN_CS_PS ) ) 
      {
        if ( ssi_ptr->cell_srv_ind.egprs_supp == SYS_EGPRS_SUPPORT_AVAIL )
        {
          /* EDGE is the available radio technology */
          *avail_radio_tech_ptr = "2";
        }
        else 
        {
          /* GPRS is the available radio technology */
          *avail_radio_tech_ptr = "1";
        }
      }
      else
      {
        /* Srv capability is CS_ONLY */
        #ifndef FEATURE_ICS
        /* GPRS is the available radio technology, since GSM is not defined */
        *avail_radio_tech_ptr = "1";
        #else
        /* GSM is the available radio technology */
        *avail_radio_tech_ptr = "16";
        #endif /* FEATURE_ICS */
      }
    } 
    /* HDR only or HDR hybrid */
    else if ( ( !ssi_ptr->hdr_hybrid && QCRIL_CM_SRV_STATUS_INDICATES_HDR_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) ) ||  
              ( QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_FULL_SRV( ssi_ptr->hdr_hybrid, ssi_ptr->hdr_srv_status ) && 
                ( ( mode_pref != CM_MODE_PREF_CDMA_HDR_ONLY ) || QCRIL_CM_SYS_MODE_IS_HDR( ssi_ptr->cur_idle_digital_mode ) ) ) ) 
    {
      /* HDR only, or Hybrid and HDR is available */
      QCRIL_LOG_DEBUG ( "HDR System : HDR active prot %d, srv_status %d, HDR srv_status %d\n", 
                        ssi_ptr->hdr_active_prot, ssi_ptr->srv_status, ssi_ptr->hdr_srv_status );

      #ifdef FEATURE_QCRIL_EHRPD
      QCRIL_LOG_DEBUG ( "HDR System : HDR personality %d\n", ssi_ptr->hdr_personality ); 
      if ( ssi_ptr->hdr_personality == SYS_PERSONALITY_EHRPD )
      {
        /* eHRPD is the available radio technology */
        QCRIL_LOG_DEBUG( "%s", "eHRPD System\n" ); 
        *avail_radio_tech_ptr = "13";
      }
      else
      #endif /* FEATURE_QCRIL_EHRPD */

      #ifdef FEATURE_QCRIL_HDR_RELB
      /* Anything EVDO Rev. B or above */
      if ( ( ssi_ptr->hdr_active_prot >= SYS_ACTIVE_PROT_HDR_RELB ) && ( ssi_ptr->hdr_active_prot < SYS_ACTIVE_PROT_HDR_END ) )
      {
        /* EVDO Rev. B is the available radio technology */
        QCRIL_LOG_DEBUG( "%s", "EVDO Rel B System\n" ); 
        *avail_radio_tech_ptr = "12";
      } 
      else 
      #endif /* FEATURE_QCRIL_HDR_RELB */

      if ( ssi_ptr->hdr_active_prot == SYS_ACTIVE_PROT_HDR_RELA ) 
      {
        /* EVDO Rev. A is the available radio technology */
        QCRIL_LOG_DEBUG( "%s", "EVDO Rel A System\n" ); 
        *avail_radio_tech_ptr = "8";
      }
      else if ( ssi_ptr->hdr_active_prot == SYS_ACTIVE_PROT_HDR_REL0 ) 
      {
        /* Default to EVDO Rev. 0 as the available radio technology */
        QCRIL_LOG_DEBUG( "%s", "EVDO Rel 0 System\n" ); 
        *avail_radio_tech_ptr = "7";
      }
      else
      {
        QCRIL_LOG_DEBUG( "%s", "EVDO System, personality negotiation in progress\n" ); 
        *avail_radio_tech_ptr = "0";

        // change data registration state to unknown, for the time personality negotiation in progress.
        if( reporting_data_reg_state )
        {
          *reg_state_ptr =  "4";
        }
      }
    } /* end if HDR */

    /* CDMA only or CDMA hybrid */
    else if ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) )
    {
      /* CDMA only or hybrid and only CDMA is available */
      QCRIL_LOG_DEBUG ( "1X System : CDMA prev %d, srv_status %d, HDR srv_status %d\n", 
                        ssi_ptr->mode_info.cdma_info.bs_p_rev, ssi_ptr->srv_status, ssi_ptr->hdr_srv_status );

      if ( ssi_ptr->mode_info.cdma_info.bs_p_rev >= 6 )
      { 
        /* 1xRTT is the available radio technology */
        *avail_radio_tech_ptr = "6";
      }
      else if ( ssi_ptr->mode_info.cdma_info.bs_p_rev >= 4 )
      {
        /* IS95B is the available radio technology */
        *avail_radio_tech_ptr = "5";
      }
      else
      {
        /* IS95A is the available radio technology */
        *avail_radio_tech_ptr = "4";
      }
    }
  }

} /* qcril_cm_util_srv_sys_info_to_avail_radio_tech */


/*=========================================================================
  FUNCTION:  qcril_cm_util_srv_sys_info_to_gw_sys_info

===========================================================================*/
/*!
    @brief
    Convert the LAC and CID to GW system info

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_srv_sys_info_to_gw_sys_info
(
  char **reg_state_ptr,     
  char **lac_ptr,
  char **cid_ptr,
  char **psc_ptr,
  char *buf_lac_ptr,
  char *buf_cid_ptr,
  char *buf_psc_ptr,  
  const qcril_cm_ss_info_type *ssi_ptr
)
{
  int len;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( reg_state_ptr != NULL );
  QCRIL_ASSERT( lac_ptr != NULL );
  QCRIL_ASSERT( cid_ptr != NULL );
  QCRIL_ASSERT( psc_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Only if registered on network, we report GW system info */
  if ( ( ssi_ptr != NULL ) && 
       ( ( strcmp( *reg_state_ptr, "1" ) == 0 ) || ( strcmp( *reg_state_ptr, "5" ) == 0 ) ) ) 
  {
    /* LAC and CID are only available if current operator is GSM/WCDMA */
    if ( QCRIL_CM_SRV_STATUS_INDICATES_GW_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) )
    {
      len = QCRIL_SNPRINTF( buf_lac_ptr, QCRIL_CM_LAC_ASCII_MAX_LEN, "%04x", ssi_ptr->sys_id.id.plmn_lac.lac );
      QCRIL_ASSERT( len <= QCRIL_CM_LAC_ASCII_MAX_LEN );
      *lac_ptr = buf_lac_ptr;

      /* Android only takes CID from 0x00000000 - 0x7fffffff*/
      if ( ssi_ptr->cell_info.cell_id > 0x7FFFFFFF )
      {
        len = QCRIL_SNPRINTF( buf_cid_ptr, QCRIL_CM_CID_ASCII_MAX_LEN, "%08x", 0x7FFFFFFF );
      }
      else
      {
        len = QCRIL_SNPRINTF( buf_cid_ptr, QCRIL_CM_CID_ASCII_MAX_LEN, "%08lx", ssi_ptr->cell_info.cell_id );
      }
      QCRIL_ASSERT( len <= QCRIL_CM_CID_ASCII_MAX_LEN );
      *cid_ptr = buf_cid_ptr;

      if ( QCRIL_CM_SRV_STATUS_INDICATES_WCDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) )
      {
        if ( ssi_ptr->cell_info.psc > 0 )
        {
          len = QCRIL_SNPRINTF( buf_psc_ptr, QCRIL_CM_PSC_HEX_MAX_LEN, "%03x", ssi_ptr->cell_info.psc );
          QCRIL_ASSERT( len <= QCRIL_CM_PSC_HEX_MAX_LEN );
          *psc_ptr = buf_psc_ptr;
        }
      }
    }
    #ifdef FEATURE_QCRIL_LTE
    /* TAC is only available if current operator is LTE */
    else if ( QCRIL_CM_SRV_STATUS_INDICATES_LTE_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) )
    {
      len = QCRIL_SNPRINTF( buf_lac_ptr, QCRIL_CM_LAC_ASCII_MAX_LEN, "%04x", ssi_ptr->tac );
      QCRIL_ASSERT( len <= QCRIL_CM_LAC_ASCII_MAX_LEN );
      *lac_ptr = buf_lac_ptr;
    }
    #endif /* FEATURE_QCRIL_LTE */
  }

} /* qcril_cm_util_srv_sys_info_to_gw_sys_info */


/*=========================================================================
  FUNCTION:  qcril_cm_util_srv_sys_info_to_1xevdo_sys_info

===========================================================================*/
/*!
    @brief
    Convert the 1XEVDO system info

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_srv_sys_info_to_1xevdo_sys_info
(
  qcril_cm_registration_state_type *resp_ptr,
  const qcril_cm_ss_info_type *ssi_ptr,
  cm_mode_pref_e_type mode_pref
)
{
  int len;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( resp_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( ( ssi_ptr != NULL ) && 
       ( ( strcmp( resp_ptr->registration_state[ 0 ], "1" ) == 0 ) || ( strcmp( resp_ptr->registration_state[ 0 ], "5" ) == 0 ) ) )
  {
    if ( QCRIL_CM_SRV_STATUS_INDICATES_1XEVDO_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode, ssi_ptr->hdr_hybrid, 
                                                        ssi_ptr->hdr_srv_status ) ) 
    {  
      /* Set BSID, BS Latitude, BS Longuitude, ccs supported, SID and NID for CDMA only */
      if ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) ) 
      {
        len = QCRIL_SNPRINTF( resp_ptr->base_id, QCRIL_CM_BASE_ID_ASCII_MAX_LEN, "%d", ssi_ptr->mode_info.cdma_info.base_id );
        QCRIL_ASSERT( len <= QCRIL_CM_BASE_ID_ASCII_MAX_LEN );
        resp_ptr->registration_state[ 4 ] = resp_ptr->base_id;

        len = QCRIL_SNPRINTF( resp_ptr->base_latitude, QCRIL_CM_BASE_LATITUDE_ASCII_MAX_LEN, "%ld", (int32)ssi_ptr->mode_info.cdma_info.base_lat );
        QCRIL_ASSERT( len <= QCRIL_CM_BASE_LATITUDE_ASCII_MAX_LEN );
        resp_ptr->registration_state[ 5 ] = resp_ptr->base_latitude;

        len = QCRIL_SNPRINTF( resp_ptr->base_longitude, QCRIL_CM_BASE_LONGITUDE_ASCII_MAX_LEN, "%ld", 
                            (int32)ssi_ptr->mode_info.cdma_info.base_long );
        QCRIL_ASSERT( len <= QCRIL_CM_BASE_LONGITUDE_ASCII_MAX_LEN );
        resp_ptr->registration_state[ 6 ] = resp_ptr->base_longitude;
        QCRIL_LOG_INFO( "Base id: %d; Base latitude: %ld; Base longitude : %ld\n", ssi_ptr->mode_info.cdma_info.base_id,
                        (int32)ssi_ptr->mode_info.cdma_info.base_lat, (int32)ssi_ptr->mode_info.cdma_info.base_long ); 

        if ( ssi_ptr->mode_info.cdma_info.ccs_supported )
        {
          resp_ptr->registration_state[ 7 ] = "1";
        }
        else
        {
          resp_ptr->registration_state[ 7 ] = "0";
        }

        len = QCRIL_SNPRINTF( resp_ptr->sid, QCRIL_CM_SID_ASCII_MAX_LEN, "%05d", ssi_ptr->sys_id.id.is95.sid );
        QCRIL_ASSERT( len <= QCRIL_CM_SID_ASCII_MAX_LEN );
        resp_ptr->registration_state[ 8 ] = resp_ptr->sid;

        len = QCRIL_SNPRINTF( resp_ptr->nid, QCRIL_CM_NID_ASCII_MAX_LEN, "%05d", ssi_ptr->sys_id.id.is95.nid );
        QCRIL_ASSERT( len <= QCRIL_CM_NID_ASCII_MAX_LEN );
        resp_ptr->registration_state[ 9 ] = resp_ptr->nid;
      }

      /* Set Roam Status for HDR only or CDMA only */
      if ( !ssi_ptr->hdr_hybrid &&
           ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) ||
             QCRIL_CM_SRV_STATUS_INDICATES_HDR_FULL_SRV( ssi_ptr->srv_status, ssi_ptr->sys_mode ) ) )
      {
        /* Always check if CDMA service is available first.  If it is, roam_status should be based on 1X. */
        if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF ) 
        {
          resp_ptr->registration_state[ 10 ] = "1";
        }
        else if ( ssi_ptr->roam_status == SYS_ROAM_STATUS_ON ) 
        {
          resp_ptr->registration_state[ 10 ] = "0";
        }
        else
        {
          len = QCRIL_SNPRINTF( resp_ptr->roam_status, QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN, "%d", ssi_ptr->roam_status );
          QCRIL_ASSERT( len <= QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN );
          resp_ptr->registration_state[ 10 ] = resp_ptr->roam_status;
        }
        QCRIL_LOG_INFO( "Roam Status : %d\n", ssi_ptr->roam_status ); 
      }

      /* Set Roam Status for Hybrid and HDR only */
      else if( ssi_ptr->hdr_hybrid )
      {
        if( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( ssi_ptr->hdr_srv_status ) && 
            ( ( mode_pref != CM_MODE_PREF_CDMA_HDR_ONLY ) || 
              QCRIL_CM_SYS_MODE_IS_HDR( ssi_ptr->cur_idle_digital_mode ) ) ) 
        {
          if ( ssi_ptr->hdr_roam_status == SYS_ROAM_STATUS_OFF ) 
          {
            resp_ptr->registration_state[ 10 ] = "1";
          }
          else if ( ssi_ptr->hdr_roam_status == SYS_ROAM_STATUS_ON ) 
          {
            resp_ptr->registration_state[ 10 ] = "0";
          }
          else
          {
            len = QCRIL_SNPRINTF( resp_ptr->roam_status, QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN, "%d", ssi_ptr->hdr_roam_status );
            QCRIL_ASSERT( len <= QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN );
            resp_ptr->registration_state[ 10 ] = resp_ptr->roam_status;
          }
          QCRIL_LOG_INFO( "HDR roam Status : %d\n", ssi_ptr->hdr_roam_status ); 
        }
        else
        {
          if( ssi_ptr->roam_status == SYS_ROAM_STATUS_OFF )
          {
            resp_ptr->registration_state[ 10 ] = "1";
          }
          else if( ssi_ptr->roam_status == SYS_ROAM_STATUS_ON )
          {
            resp_ptr->registration_state[ 10 ] = "0";
          }
          else
          {
            len = QCRIL_SNPRINTF( resp_ptr->roam_status, QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN, "%d", ssi_ptr->roam_status );
            QCRIL_ASSERT( len <= QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN );
            resp_ptr->registration_state[ 10 ] = resp_ptr->roam_status;
          }
          QCRIL_LOG_INFO( "Roam Status : %d\n", ssi_ptr->roam_status ); 
        }
      }

      /* Set PRL indicator */
      if ( ssi_ptr->is_sys_prl_match )
      {
        resp_ptr->registration_state[ 11 ] = "1";
      }
      else
      {
        resp_ptr->registration_state[ 11 ] = "0";
      }

      /* Set Default Roaming Indicator */
      len = QCRIL_SNPRINTF( resp_ptr->def_roam_ind, QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN, "%d", ssi_ptr->def_roam_ind );
      QCRIL_ASSERT( len <= QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN );
      resp_ptr->registration_state[ 12 ] = resp_ptr->def_roam_ind;
      QCRIL_LOG_INFO( "Is in PRL : %d; Default Roam Indicator : %d\n", ssi_ptr->is_sys_prl_match, ssi_ptr->def_roam_ind ); 
    }
  }

} /* qcril_cm_util_srv_sys_info_to_1xevdo_sys_info */


/*=========================================================================
  FUNCTION:  qcril_cm_util_lookup_reg_status

===========================================================================*/
/*!
    @brief
    Return the long alpha of registration status

    @return
    Long alpha of registration status
*/
/*=========================================================================*/
const char *qcril_cm_util_lookup_reg_status
(
  char *reg_state_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( reg_state_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( strcmp( reg_state_ptr, "0" ) == 0 )
  {
    return "Not registered/Not searching";
  }
  else if ( strcmp( reg_state_ptr, "1" ) == 0 )
  {
    return "Registered (Home network)";
  }
  else if ( strcmp( reg_state_ptr, "2" ) == 0 )
  {
    return "Not registered/Actively searching";
  }
  else if ( strcmp( reg_state_ptr, REG_STATE_DENIED_EMERGENCY ) == 0 )
  {
    return "Registration denied";
  }
  else if ( strcmp( reg_state_ptr, "4" ) == 0 )
  {
    return "Unknown";
  }
  else if ( strcmp( reg_state_ptr, "5" ) == 0 )
  {
    return "Registered (Roaming)";
  }
  else
  {
    return "Registered (Roaming Affiliates)";
  }

} /* qcril_cm_util_lookup_reg_status */


/*=========================================================================
  FUNCTION:  qcril_cm_util_lookup_radio_tech

===========================================================================*/
/*!
    @brief
    Return the long alpha of radio technology

    @return
    Long alpha of radio technology
    
*/
/*=========================================================================*/
const char *qcril_cm_util_lookup_radio_tech
(
  char *radio_tech_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( radio_tech_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  if ( strcmp( radio_tech_ptr, "0" ) == 0 )
  {
    return "Unknown";
  }
  else if ( strcmp( radio_tech_ptr, "1" ) == 0 )
  {
    return "GPRS";
  }
  else if ( strcmp( radio_tech_ptr, "2" ) == 0 )
  {
    return "EDGE";
  }
  else if ( strcmp( radio_tech_ptr, "3" ) == 0 )
  {
    return "UMTS";
  }
  else if ( strcmp( radio_tech_ptr, "4" ) == 0 )
  {
    return "IS95A";
  }
  else if ( strcmp( radio_tech_ptr, "5" ) == 0 )
  {
    return "IS95B";
  }
  else if ( strcmp( radio_tech_ptr, "6" ) == 0 )
  {
    return "IxRTT";
  }
  else if ( strcmp( radio_tech_ptr, "7" ) == 0 )
  {
    return "EVDO 0";
  }
  else if ( strcmp( radio_tech_ptr, "8" ) == 0 )
  {
    return "EVDO A";
  }
  else if ( strcmp( radio_tech_ptr, "9" ) == 0 )
  {
    return "HSDPA";
  }
  else if ( strcmp( radio_tech_ptr, "10" ) == 0 )
  {
    return "HSUPA";
  }
  else if ( strcmp( radio_tech_ptr, "11" ) == 0 )
  {
    return "HSPA";
  }
  else if ( strcmp( radio_tech_ptr, "12" ) == 0 ) 
  {
    return "EVDO B";
  }
  else if ( strcmp( radio_tech_ptr, "13" ) == 0 ) 
  {
    return "EHRPD";
  }
  else if ( strcmp( radio_tech_ptr, "14" ) == 0 ) 
  {
    return "LTE";
  }

  return "Undefined";

} /* qcril_cm_util_lookup_radio_tech */


/*=========================================================================
  FUNCTION:  qcril_cm_util_process_cnap_info

===========================================================================*/
/*!
    @brief
    This function is called when the incom call arrives,
    if any CNAP information is present it is processed.

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_process_cnap_info 
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  const cm_mm_call_info_s_type *call_info_ptr,
  boolean *unsol_call_state_changed_ptr
)
{
  qcril_cm_clist_public_type call_info;
  char name[ (MAX_MT_USSD_CHAR * 2) + 1];
  IxErrnoType  status = E_SUCCESS;
  uint8 pi ; /*presentation indicator */
  int name_len =0;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( call_info_ptr != NULL );
  QCRIL_ASSERT( unsol_call_state_changed_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  *unsol_call_state_changed_ptr = FALSE;

  if ( ( call_info_ptr->enc_alpha.num_bytes > 0 )  &&
       ( qcril_cm_clist_query_voice_call_id( instance_id, modem_id, call_info_ptr->call_id, &call_info ) == E_SUCCESS ) )
  {

      name_len = qcril_cm_ss_convert_ussd_string_to_utf8( call_info_ptr->enc_alpha.coding_scheme,
                                                          call_info_ptr->enc_alpha.num_bytes,
                                                          (byte *) call_info_ptr->enc_alpha.buf,
                                                          name );

      if ( name_len > ( MAX_MT_USSD_CHAR * 2 ) )
      {
         QCRIL_LOG_ERROR ("%s", "name_len exceeds MAX_MT_CNAP_CHAR, reset to  MAX_MT_CNAP_CHAR\n" );
         name_len = (int) (MAX_MT_USSD_CHAR*2);
         name[ name_len ] = '\0';
      }

    QCRIL_LOG_DEBUG( "CNAP name after converting to ascii = %s\n", name );

    /* Update name in the call list */
    if ( ( status = qcril_cm_clist_update_name( instance_id, modem_id, call_info_ptr->call_id, name ) )!= E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Failed to update the name of CList entry for call_id %d\n", call_info_ptr->call_id );
    }

    /* Update name presentation in the call list */
    if ( status == E_SUCCESS )
    {
      pi = call_info_ptr->num.pi;
      if ( call_info_ptr->num.pi == 4 )
      {
        pi = 0 ; /* In case when PI over ride category is provisioned */
      }

      if ( ( status = qcril_cm_clist_update_name_presentation( instance_id, modem_id, call_info_ptr->call_id, pi ) ) != E_SUCCESS )
      {
        QCRIL_LOG_ERROR( "Failed to update the name presentation of CList entry for call_id %d\n", call_info_ptr->call_id );
      }
    }

    if ( status == E_SUCCESS  )
    {
      /* Need to send unsolicited call state changed indication */
      *unsol_call_state_changed_ptr = TRUE;
    }
  }

} /* qcril_cm_util_process_cnap_info */


/*=========================================================================
  FUNCTION:  qcril_cm_util_srv_sys_info_to_rej_cause

===========================================================================*/
/*!
    @brief
    Function returns the registration reject cause

    @return
    registration reject cause when in limited service 
    
*/
/*=========================================================================*/
void qcril_cm_util_srv_sys_info_to_rej_cause
(
  char **reg_state_ptr,
  char **rej_cause_ptr,
  qcril_cm_reg_reject_info_type *reg_reject_info_ptr,
  const qcril_cm_ss_info_type *ssi_ptr,
  boolean reporting_data_reg_state
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( reg_state_ptr != NULL );
  QCRIL_ASSERT( rej_cause_ptr != NULL );
  
  /*-----------------------------------------------------------------------*/

  /* Default Reject Cause */
  *rej_cause_ptr = "0"; /* Unspecified failure */

  /* only when the registration state is limited and reject cause is availabe
       send the reject cause */
  if ( ( ssi_ptr != NULL ) && QCRIL_CM_SYS_MODE_IS_GW( ssi_ptr->sys_mode ) )
  {
    QCRIL_LOG_DEBUG( "Reject cause value = %d, Registration status = %s\n", reg_reject_info_ptr->cm_reg_reject_info.reject_cause,
                     qcril_cm_util_lookup_reg_status( *reg_state_ptr ) );
     
    /* reporting reject cause only when reject cause is valid and it is not already reported even once */
    if ( strcmp( *reg_state_ptr, REG_STATE_DENIED_EMERGENCY ) == 0 )
    {
      if ( reg_reject_info_ptr->managed_roaming_enabled )
      {
        *rej_cause_ptr = "10"; /*Managed Roaming Specific Cause*/
        QCRIL_LOG_DEBUG( "Reject cause value sent to UI = %s\n", *rej_cause_ptr );
        reg_reject_info_ptr->managed_roaming_enabled = FALSE;
        return;
      }
       
      if ( reg_reject_info_ptr->cm_reg_reject_info.reject_cause > 0 )
      {
        if ( !reg_reject_info_ptr->reg_reject_reported )
        {
          if ( !reporting_data_reg_state &&
               ( ( reg_reject_info_ptr->cm_reg_reject_info.reject_srv_domain == SYS_SRV_DOMAIN_CS_ONLY ) ||
                 ( reg_reject_info_ptr->cm_reg_reject_info.reject_srv_domain == SYS_SRV_DOMAIN_CS_PS ) ) )
          {
            QCRIL_LOG_DEBUG( "Reject cause reported to UI = %d, data_reg_state = %d\n",
                                              reg_reject_info_ptr->cm_reg_reject_info.reject_cause, reporting_data_reg_state );
      
            switch ( reg_reject_info_ptr->cm_reg_reject_info.reject_cause )
            {
              case 255 : *rej_cause_ptr = "1";  /*Authentication rejected*/
                break;
              case 2 :   *rej_cause_ptr = "2";  /*IMSI unknown in HLR*/
                break;
              case 3 :   *rej_cause_ptr = "3";  /*Illegal MS*/
                break;
              case 4 :   *rej_cause_ptr = "4";  /*IMSI unknown in VLR*/
                break;
              case 5 :   *rej_cause_ptr = "5";  /*IMEI not accepted*/
                break;
              case 6:    *rej_cause_ptr = "6";  /*Illegal ME*/
                break;
              case 11:   *rej_cause_ptr = "11";  /*PLMN not allowed*/
                break;
              case 12:   *rej_cause_ptr = "12";  /*Location Area not allowed*/
                break;
              case 13:   *rej_cause_ptr = "13";  /*Roaming not allowed in this location area*/
                break;
              case 15:   *rej_cause_ptr = "15";  /*No Suitable Cells in this Location Area*/
                break;
              case 17:   *rej_cause_ptr = "17";  /*Network failure */
                break;
              case 20:   *rej_cause_ptr = "20";  /*MAC Failure*/
                break;
              case 21:   *rej_cause_ptr = "21";  /*Sync Failure */
                break;
              case 22:   *rej_cause_ptr = "22";  /*Congestion */
                break;
              case 23:   *rej_cause_ptr = "23";  /*GSM Authentication unacceptable */
                break;
              case 25:   *rej_cause_ptr = "25";  /*Not Authorized for this CSG*/
                break;
              case 32:   *rej_cause_ptr = "32";  /*Service option not supported */
                break;
              case 33:   *rej_cause_ptr = "33";  /*Requested service option not subscribed */
                break;
              case 34:   *rej_cause_ptr = "34";  /*Service option temporarily out of order */
                break;
              case 38:   *rej_cause_ptr = "38";  /*Call cannot be identified */
                break;
              case 48 ... 63:  *rej_cause_ptr = "48";  /*Retry upon entry into a new cell */
                break;
              case 95:   *rej_cause_ptr = "95";  /*Semantically incorrect message */
                break;
              case 96:   *rej_cause_ptr = "96";  /*Network failure */
                break;
              case 97:   *rej_cause_ptr = "97";  /*Network failure */
                break;
              case 98:   *rej_cause_ptr = "98";  /*Network failure */
                break;
              case 99:   *rej_cause_ptr = "99";  /*Network failure */
                break;
              case 100:  *rej_cause_ptr = "100";  /*Network failure */
                break;
              case 101:  *rej_cause_ptr = "101";  /*Message not compatible with protocol state */
                break;
              case 111:  *rej_cause_ptr = "111";  /* Protocol error, unspecified */
                break;
              default:   *rej_cause_ptr = "0";  /*Unspecified failure*/
                break;
            }

            if ( reg_reject_info_ptr->cm_reg_reject_info.reject_srv_domain == SYS_SRV_DOMAIN_CS_ONLY )
            {
              reg_reject_info_ptr->reg_reject_reported = TRUE;              
            }
          }
          else if ( reporting_data_reg_state &&
                    ( ( reg_reject_info_ptr->cm_reg_reject_info.reject_srv_domain == SYS_SRV_DOMAIN_PS_ONLY ) ||
                      ( reg_reject_info_ptr->cm_reg_reject_info.reject_srv_domain == SYS_SRV_DOMAIN_CS_PS ) ) )
          {
            switch ( reg_reject_info_ptr->cm_reg_reject_info.reject_cause )
            {
              case 7 :   *rej_cause_ptr = "7";  /*Illegal MS*/
                break;
              case 8 :   *rej_cause_ptr = "8";  /*IMSI unknown in VLR*/
                break;
              case 9 :   *rej_cause_ptr = "9";  /*IMEI not accepted*/
                break;
              case 10:    *rej_cause_ptr = "10";  /*Illegal ME*/
                break;
              case 14:   *rej_cause_ptr = "14";  /*Roaming not allowed in this location area*/
                break;
              case 16:   *rej_cause_ptr = "16";  /*No Suitable Cells in this Location Area*/
                break;
              case 40:   *rej_cause_ptr = "40";  /*Call cannot be identified */
                break;
              default:   *rej_cause_ptr = "0";  /*Unspecified failure*/
                break;
            }
            reg_reject_info_ptr->reg_reject_reported = TRUE;
          }
        }
        else
        {
          *rej_cause_ptr = "0"; /*Unspecified failure*/
        }
      }
    }/* resetting the reject cause on full service */
    else if ( ( ( strcmp( *reg_state_ptr, "1" ) == 0 ) || ( strcmp( *reg_state_ptr, "5" ) == 0 ) ) &&
              ( reg_reject_info_ptr->cm_reg_reject_info.reject_cause > 0 ) &&
              ( reg_reject_info_ptr->reg_reject_reported ) )
    {
      reg_reject_info_ptr->cm_reg_reject_info.reject_cause = 0;
      reg_reject_info_ptr->cm_reg_reject_info.reject_srv_domain = SYS_SRV_DOMAIN_NONE;
      reg_reject_info_ptr->reg_reject_reported = FALSE;
    }
  }
  
  QCRIL_LOG_DEBUG( "Reject cause value sent to UI = %s\n", *rej_cause_ptr);

}/* qcril_cm_util_srv_sys_info_to_rej_cause */


/*=========================================================================
  FUNCTION:  qcril_cm_util_convert_2s_complement_to_int

===========================================================================*/
/*!
    @brief
    convert a byte from 2's complement 6 bit number to integer

    @return
    integer

*/
/*=========================================================================*/
int qcril_cm_util_convert_2s_complement_to_int
(
  byte i
)
{
  byte j=0;
  if (i >= 0x20 )
  {
    j = ((~i + 1) << 2);
    return ( -1 * (j >> 2) );
  }
  else
  {
    return i;
  }
} /* qcril_cm_util_convert_2s_complement_to_int */


/*=========================================================================
  FUNCTION:  qcril_cm_util_is_emer_number

===========================================================================*/
/*!
    @brief
    Checks whether the given number is emergency number or not by reading
    from a property file containing list of all emergency numbers.

    @return
    E_SUCCESS (or) E_FAILURE

*/
/*=========================================================================*/
IxErrnoType qcril_cm_util_is_emer_number
(
  char *flash_num
)
{
  char property_emer_list[PROPERTY_VALUE_MAX];
  char *next_num = NULL;
  const char delimiter[] = ",";
  char *emer_read = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( flash_num != NULL );

  /*-----------------------------------------------------------------------*/

  property_get( QCRIL_ECC_LIST , property_emer_list , "" );
  emer_read = strtok_r(property_emer_list , delimiter , &next_num);
  while(emer_read != NULL)
  {
    if (strcmp(emer_read , flash_num) == 0)
    {
      QCRIL_LOG_DEBUG( "Found Emergency number: %s, from property file: %s\n", flash_num , QCRIL_ECC_LIST );
      return E_SUCCESS;
    }
    emer_read = strtok_r(NULL , delimiter , &next_num);
  }
  QCRIL_LOG_DEBUG( "Not an Emergency number: %s", flash_num );

  return E_FAILURE;

} /* qcril_cm_util_is_emer_number */


/*===========================================================================
  FUNCTION: qcril_cm_util_ussd_pack 
 
===========================================================================*/
/*! 
    @brief
    Pack 7-bit GSM characters into bytes (8-bits)
    
    @return
    packed_data length

*/
/*=========================================================================*/
byte qcril_cm_util_ussd_pack(

    byte *packed_data,
    const byte *str,
    byte num_chars
)
{
  byte stridx=0;
  byte pckidx=0;
  byte shift;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  QCRIL_ASSERT(packed_data != NULL);
  QCRIL_ASSERT(str         != NULL);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Loop through the 7-bit string till the last but one character.
  */
  while(stridx < (num_chars-1))
  {
   shift = stridx  & 0x07;

   /* A byte of packed data is always made up of only 2 7-bit characters. The
   ** shift of the two characters always depends on their index in the string.
   */
   packed_data[pckidx++] = (str[stridx] >> shift) |
                           (str[stridx+1] << (7-shift)); /*lint !e734 */

   /* If the second characters fits inside the current packed byte, then skip
   ** it for the next iteration.
   */
   if(shift==6) stridx++;
   stridx++;
  }

  /* Special case for the last 7-bit character.
  */
  if(stridx < num_chars)
  {
    shift = stridx & 0x07;
    /* The tertiary operator (?:) takes care of the special case of (8n-1)
    ** 7-bit characters which requires padding with CR (0x0D).
    */
    packed_data[pckidx++] = ((shift == 6) ? (CHAR_CR << 1) : 0) |
                          (str[stridx] >> shift);
  }

  /* Takes care of special case when there are 8n 7-bit characters and the last
  ** character is a CR (0x0D).
  */
  if((num_chars & 0x07) == 0 && str[num_chars - 1] == CHAR_CR)
  {
    packed_data[pckidx++] = CHAR_CR;
  }

  return pckidx;
} /* qcril_cm_util_ussd_pack */


/*===========================================================================
  FUNCTION: qcril_cm_util_ussd_unpack 
 
===========================================================================*/
/*! 
    @brief
    Unpack the bytes (8-bit) into 7-bit GSM characters
    
    @return
    str length

*/
/*=========================================================================*/
byte qcril_cm_util_ussd_unpack
(
    byte *str,
    const byte *packed_data,
    byte num_bytes
)
{

  byte stridx = 0;
  byte pckidx = 0;
  byte prev = 0;
  byte curr = 0;
  byte shift;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  QCRIL_ASSERT(str         != NULL);
  QCRIL_ASSERT(packed_data != NULL);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  while(pckidx < num_bytes)
  {
    shift = stridx & 0x07;
    curr = packed_data[pckidx++];

    /* A 7-bit character can be split at the most between two bytes of packed
    ** data.
    */
    str[stridx++] = ( (curr << shift) | (prev >> (8-shift)) ) & 0x7F;

    /* Special case where the whole of the next 7-bit character fits inside
    ** the current byte of packed data.
    */
    if(shift == 6)
    {
      /* If the next 7-bit character is a CR (0x0D) and it is the last
      ** character, then it indicates a padding character. Drop it.
      */

      if((curr >> 1) == CHAR_CR && pckidx == num_bytes)
      {
        break;
      }
      str[stridx++] = curr >> 1;
    }

    prev = curr;
  }

  return stridx;

} /* qcril_cm_util_ussd_unpack */


/*===========================================================================
  FUNCTION: qcril_cm_util_bcd_to_ascii 
 
===========================================================================*/
/*! 
    @brief
    Convert the phone number from BCD to ASCII
    
    @return
    None

*/
/*=========================================================================*/
void qcril_cm_util_bcd_to_ascii
(
  const byte *bcd_number, 
  byte *ascii_number
)
{
  int bcd_index = 0;
  int ascii_index = 0;
  byte bcd_length;
  uint8 asc_1 = 0;
  uint8 asc_2 = 0;
  boolean presentation_indicator_absent = TRUE;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  QCRIL_ASSERT(bcd_number   != NULL);
  QCRIL_ASSERT(ascii_number != NULL);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  bcd_length = bcd_number[bcd_index++];

  /*****************************/
  /* International call or not */
  /*****************************/
  /*lint -save -e641*/
  if ((bcd_number[bcd_index] & CM_TON_MASK) >> 4 == QCRIL_CM_NUM_TYPE_INTERNATIONAL)
  {
     ascii_number[ascii_index++] = '+';
  }
  /*lint -restore */

  /* check if there is an extra byte or not (screening indicator,
  ** presentation_indicator)
  */

  presentation_indicator_absent =
     ((boolean)(bcd_number[bcd_index] & 0x80) >> 7);

  bcd_index++;

  /**************************/
  /* presentation_indicator */
  /**************************/

  if (presentation_indicator_absent == FALSE)
  {
     bcd_index++;
  }


  /*************************/
  /* Decode BCD into ascii */
  /*************************/

  for( ; bcd_index <= bcd_length; bcd_index++)
  {

     asc_1 = (bcd_number[bcd_index] & 0x0F);
     asc_2 = (bcd_number[bcd_index] & 0xF0) >> 4;

     ascii_number[ascii_index++] = (asc_1==QCRIL_BCD_STAR)? '*' :
                                    (asc_1==QCRIL_BCD_HASH)? '#' :
                                    QCRIL_INRANGE(asc_1, 0x0C, 0x0E)? (asc_1 - 0x0C) + 'a':
                                    asc_1 + '0';

     ascii_number[ascii_index++] = (asc_2==QCRIL_BCD_STAR)? '*' :
                                    (asc_2==QCRIL_BCD_HASH)? '#' :
                                    QCRIL_INRANGE(asc_2, 0x0C, 0x0E)? (asc_2 - 0x0C) + 'a':
                                    (asc_2==0x0F)? '\0' :
                                    asc_2 + '0';
  }

  /* Null terminate the ascii string */
  if (asc_2 != 0x0f)
  {
    ascii_number[ascii_index] = '\0';
  }
} /* qcril_cm_util_bcd_to_ascii */


/*===========================================================================
  FUNCTION: qcril_cm_util_number_to_bcd 
 
===========================================================================*/
/*! 
    @brief
    Convert the phone number from ASCII to BCD
    
    @return
    None

*/
/*=========================================================================*/
void qcril_cm_util_number_to_bcd
(
  const cm_num_s_type *number, 
  byte *bcd_number
)
{
  int i, j;                        /* Control variables for loops. */
  int bcd_index      = QCRIL_BCD_NUM;    /* Index into output bcd_number */

  /* (digits + number_type) in bcd can be size CM_CALLED_PARTY_BCD_NO_LENGTH
  ** temp needs to be twice that of bcd. CM_CALLED_PARTY_BCD_NO_LENGTH is
  ** GW specific so using CM_MAX_NUMBER_CHARS instead.
  */
  uint8 temp[2 * CM_MAX_NUMBER_CHARS];

  uint8 number_type = 0;

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  QCRIL_ASSERT(number     != NULL);
  QCRIL_ASSERT(bcd_number != NULL);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Store the number type of string coming in, it may be changed later */
  number_type = number->number_type;

  if ( number->len >= CM_MAX_NUMBER_CHARS )
  {
    QCRIL_LOG_ERROR( "bcd_number length %d is greater than max val %d", 
                     number->len, CM_MAX_NUMBER_CHARS, 0);
    return;
  }

  /* Copy the phone number into its temporary home */
  for (i = 0, j = 0; i < number->len; i++)
  {
    /*
    ** If international number, '+' in begining of dialed string, ignore '+'
    * and set the number_type to INTERNATIONAL.
    */
    if (i == 0)
    {
      if (number->buf[0] == '+')
      {
        number_type = (uint8)QCRIL_CM_NUM_TYPE_INTERNATIONAL;
        continue;
      }
    }

    /* If its a digit we care about.... */
    if (QCRIL_ISDIGIT(number->buf[i]) ||
        number->buf[i] == '*' ||
        number->buf[i] == '#' ||
        QCRIL_INRANGE(number->buf[i], 'a', 'c') ||
        QCRIL_INRANGE(number->buf[i], 'A', 'C') )
    {
      /* Store the bcd digits into temp. */
      temp[j++] = (number->buf[i] == '#') ? QCRIL_BCD_HASH :
                  (number->buf[i] == '*') ? QCRIL_BCD_STAR :
                  QCRIL_INRANGE(number->buf[i], 'a', 'c') ? (0x0C + number->buf[i] - 'a') :
                  QCRIL_INRANGE(number->buf[i], 'A', 'C') ? (0x0C + number->buf[i] - 'A') :
                  (number->buf[i] & 0x0f);
    }
  }
  /* Odd number of digits must have the 0x0f at the end. */
  if (j & 0x01)
  {
    temp[j++] = 0x0f;
  }

  /* Now that temp has the bcd codes in natural order... Squish them together
   * and reverse the order per bcd coding.
   */
  for (i = 0; i < j; i+=2)
  {
    /*lint -esym(644,temp)*/
    bcd_number[bcd_index++] = (byte) (temp[i+1] << 4) | temp[i];
    /*lint +esym(644,temp)*/
  }

  /* Put length value in to first element of bcd number array and the number
   * type and number plan into the second entry.
   */
  bcd_number[QCRIL_BCD_NT_NPI] = (byte) (0x80 | (number_type << 4) | number->number_plan);
  bcd_number[QCRIL_BCD_LEN] = (byte) (bcd_index - 1);

} /* qcril_cm_util_number_to_bcd */


/*===========================================================================
  FUNCTION: qcril_cm_util_ascii_to_gsm_alphabet 
 
===========================================================================*/
/*! 
    @brief
    Convert the ASCII string to GSM default alphabets string and packs it
    into bytes.
    
    @return
    None

*/
/*=========================================================================*/
byte qcril_cm_util_ascii_to_gsm_alphabet
(
    byte          *gsm_alphabet_string,
    const byte          *ascii_string,
    byte           num_chars
)
{

   byte temp_buffer[MAX_DISPLAY_TEXT_LEN];
   byte n = 0;

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   QCRIL_ASSERT(gsm_alphabet_string    != NULL);
   QCRIL_ASSERT(ascii_string           != NULL);

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*
   ** convert from ascii coding into GSM default-alphabet coding with
   ** 1 char per byte
   */
   for( n = 0; n < num_chars; n++ )
   {
      temp_buffer[n] = qcril_ascii_to_def_alpha_table[ascii_string[n]];
   }

   /*
   ** now pack the string down to 7-bit format
   */
   return qcril_cm_util_ussd_pack( gsm_alphabet_string, temp_buffer, num_chars);

} /* qcril_cm_util_ascii_to_gsm_alphabet */


/*===========================================================================
  FUNCTION: qcril_cm_util_gsm_alphabet_to_ascii
 
===========================================================================*/
/*! 
    @brief
    Unpacks bytes of data into 7-bit GSM default alphabet string and then
    converts it to an ASCII string.
    
    @return
    Number of characters written into the output buffer.

*/
/*=========================================================================*/
byte qcril_cm_util_gsm_alphabet_to_ascii
(
    byte    *ascii_string,
    const byte    *gsm_alphabet_string,
    byte     num_bytes
)
{

   byte temp_buffer[MAX_DISPLAY_TEXT_LEN];
   byte n = 0;
   byte num_chars;

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - */

   QCRIL_ASSERT(ascii_string           != NULL);
   QCRIL_ASSERT(gsm_alphabet_string    != NULL);

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - */

   /*
   ** unpack the string from 7-bit format into 1 char per byte format
   */
   num_chars = qcril_cm_util_ussd_unpack(temp_buffer, gsm_alphabet_string, num_bytes);

   /*
   ** now convert from GSM default alphabet coding into ascii coding
   */
   for( n = 0; n < num_chars; n++ )
   {
      ascii_string[n] = qcril_def_alpha_to_ascii_table[temp_buffer[n]];
   }
   ascii_string[num_chars] = '\0';

   return num_chars;

} /* qcril_cm_util_gsm_alphabet_to_ascii */

/*===========================================================================
  FUNCTION: qcril_cm_util_subs_mode_pref

===========================================================================*/
/*!
    @brief
    Convert the mode preference read from NV item NV_PREF_MODE_I to mode preference
    as per interface between qcril_cm and qcril_uim for activation of subscription.

    @return
    mode preference as per the qcril_cm and qcril_uim interface

*/
/*=========================================================================*/
boolean  qcril_cm_util_subs_mode_pref
(
  qcril_cm_mode_pref_e_type  modem_mode_pref,
  qcril_subs_mode_pref *mode_pref
)
{
 boolean return_value = FALSE;
 QCRIL_ASSERT(mode_pref != NULL);

 QCRIL_LOG_DEBUG( "mode preference read from NV = %d\n", modem_mode_pref);

 *mode_pref = QCRIL_CM_MODE_PREF_NONE;

  switch( modem_mode_pref )
 {
   case QCRIL_CM_MODE_PREF_CDMA_ONLY:
   case QCRIL_CM_MODE_PREF_HDR_ONLY:
   case QCRIL_CM_MODE_PREF_CDMA_HDR_ONLY:
   case QCRIL_CM_MODE_PREF_CDMA_AMPS_HDR_ONLY:
            *mode_pref = QCRIL_SUBS_MODE_1X;
            return_value = TRUE;
            break;

   case QCRIL_CM_MODE_PREF_GSM_ONLY:
   case QCRIL_CM_MODE_PREF_WCDMA_ONLY:
   case QCRIL_CM_MODE_PREF_GSM_WCDMA_ONLY:
            *mode_pref = QCRIL_SUBS_MODE_GW;
            return_value = TRUE;
            break;

   default:
           QCRIL_LOG_ERROR( "received invalid mode preference after reading from NV = %d\n",
                     *mode_pref);
           break;

 }

 if( return_value)
 {
   QCRIL_LOG_DEBUG( "mode preference selected = %d\n", *mode_pref);
 }

 return return_value;
} /* qcril_cm_util_subs_mode_pref */
