/*!
  @file
  qcril_cm.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm.h#6 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/25/10   sb      Drop MT SMS if mobile is in Emergency Callback state.
11/05/09   pg      Fixed no Emergency callback mode after emergency call over
                   flash.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
03/17/09   fc      Added ONS support for NITZ.
05/28/08   pg      Added basic call services support.
05/07/08   fc      First cut implementation.


===========================================================================*/

#ifndef QCRIL_CM_H
#define QCRIL_CM_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "cm.h"
#include "qcril_cm_ss.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Determine whether the service status indicates no service  */
#define QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( srv_status ) \
  ( srv_status == SYS_SRV_STATUS_NO_SRV )

/* Determine whether the service status indicates service available  */
#define QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( srv_status ) \
  ( ( srv_status == SYS_SRV_STATUS_SRV ) || ( srv_status == SYS_SRV_STATUS_LIMITED ) || ( srv_status == SYS_SRV_STATUS_LIMITED_REGIONAL ) )
  
/* Determine whether the service status indicates full service  */
#define QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) \
  ( srv_status == SYS_SRV_STATUS_SRV )

/* Determine whether the current operator is GSM/WCDMA/LTE */
#ifdef FEATURE_QCRIL_LTE
#define QCRIL_CM_SYS_MODE_IS_GWL( sys_mode ) \
  ( ( sys_mode == SYS_SYS_MODE_GSM ) || ( sys_mode == SYS_SYS_MODE_WCDMA ) || ( sys_mode == SYS_SYS_MODE_GW ) || \
    ( sys_mode == SYS_SYS_MODE_LTE ) || ( sys_mode == SYS_SYS_MODE_GWL ) )
#else
#define QCRIL_CM_SYS_MODE_IS_GWL( sys_mode ) \
  ( ( sys_mode == SYS_SYS_MODE_GSM ) || ( sys_mode == SYS_SYS_MODE_WCDMA ) || ( sys_mode == SYS_SYS_MODE_GW ) )
#endif /* FEATURE_QCRIL_LTE */

/* Determine whether the current operator is GSM/WCDMA */
#define QCRIL_CM_SYS_MODE_IS_GW( sys_mode ) \
  ( ( sys_mode == SYS_SYS_MODE_GSM ) || ( sys_mode == SYS_SYS_MODE_WCDMA ) || ( sys_mode == SYS_SYS_MODE_GW ) )

#ifdef FEATURE_QCRIL_LTE
/* Determine whether the current operator is LTE */
#define QCRIL_CM_SYS_MODE_IS_LTE( sys_mode ) \
  ( ( sys_mode == SYS_SYS_MODE_LTE ) || ( sys_mode == SYS_SYS_MODE_GWL ) )
#endif /* FEATURE_QCRIL_LTE */

/* Determine whether the current operator is WCDMA */
#define QCRIL_CM_SYS_MODE_IS_WCDMA( sys_mode ) \
  ( ( sys_mode == SYS_SYS_MODE_WCDMA ) || ( sys_mode == SYS_SYS_MODE_GW ) )

/* Determine whether the current operator is GSM */
#define QCRIL_CM_SYS_MODE_IS_GSM( sys_mode ) \
  ( sys_mode == SYS_SYS_MODE_GSM ) 

/* Determine whether the current operator is CDMA/HDR */
#define QCRIL_CM_SYS_MODE_IS_1XEVDO( sys_mode ) \
  ( ( sys_mode == SYS_SYS_MODE_CDMA ) || ( sys_mode == SYS_SYS_MODE_HDR ) )

/* Determine whether the current operator is CDMA */
#define QCRIL_CM_SYS_MODE_IS_CDMA( sys_mode ) \
  ( sys_mode == SYS_SYS_MODE_CDMA ) 

/* Determine whether the current operator is HDR */
#define QCRIL_CM_SYS_MODE_IS_HDR( sys_mode ) \
  ( sys_mode == SYS_SYS_MODE_HDR ) 

/* Determine whether the current operator is unknown */
#define QCRIL_CM_SYS_MODE_IS_UNKNOWN( sys_mode ) \
  ( sys_mode == SYS_SYS_MODE_NO_SRV )

/* Determine whether the service status indicates LTE no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_LTE_NO_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_LTE( sys_mode ) )

/* Determine whether the service status indicates GW no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_GW_NO_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_GW( sys_mode ) )

/* Determine whether the service status indicates GWL no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_GWL_NO_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_GWL( sys_mode ) )

/* Determine whether the service status indicates CDMA no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_CDMA_NO_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_CDMA( sys_mode ) )

/* Determine whether the service status indicates HDR no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_HDR_NO_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_HDR( sys_mode ) ) 

/* Determine whether the service status indicates hybrid HDR no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_NO_SRV( hdr_hybrid, hdr_srv_status, sys_mode ) \
  ( hdr_hybrid && QCRIL_CM_SRV_STATUS_INDICATES_NO_SRV( hdr_srv_status ) &&                     \
   ( QCRIL_CM_SYS_MODE_IS_CDMA( sys_mode ) || QCRIL_CM_SYS_MODE_IS_UNKNOWN( sys_mode ) ) ) 

/* Determine whether the service status indicates 1XEVO no service */
#define QCRIL_CM_SRV_STATUS_INDICATES_1XEVDO_NO_SRV( srv_status, sys_mode, hdr_hybrid, hdr_srv_status ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_NO_SRV( srv_status, sys_mode ) ||                                \
    QCRIL_CM_SRV_STATUS_INDICATES_HDR_NO_SRV( srv_status, sys_mode ) ||                                 \
    QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_NO_SRV( hdr_hybrid, hdr_srv_status, sys_mode ) )
  
/* Determine whether the service status indicates LTE service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_LTE_SRV_AVAILABLE( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( srv_status ) && QCRIL_CM_SYS_MODE_IS_LTE( sys_mode ) )

/* Determine whether the service status indicates GW service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_GW_SRV_AVAILABLE( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( srv_status ) && QCRIL_CM_SYS_MODE_IS_GW( sys_mode ) )

/* Determine whether the service status indicates GWL service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_GWL_SRV_AVAILABLE( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( srv_status ) && QCRIL_CM_SYS_MODE_IS_GWL( sys_mode ) )

/* Determine whether the service status indicates CDMA service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_CDMA_SRV_AVAILABLE( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( srv_status ) && QCRIL_CM_SYS_MODE_IS_CDMA( sys_mode ) )

/* Determine whether the service status indicates HDR service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_HDR_SRV_AVAILABLE( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( srv_status ) && QCRIL_CM_SYS_MODE_IS_HDR( sys_mode ) ) 

/* Determine whether the service status indicates hybrid HDR service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_SRV_AVAILABLE( hdr_hybrid, hdr_srv_status, sys_mode ) \
  ( hdr_hybrid && QCRIL_CM_SRV_STATUS_INDICATES_SRV_AVAILABLE( hdr_srv_status ) &&                     \
   ( QCRIL_CM_SYS_MODE_IS_CDMA( sys_mode ) || QCRIL_CM_SYS_MODE_IS_UNKNOWN( sys_mode ) ) ) 

/* Determine whether the service status indicates 1XEVO service available */
#define QCRIL_CM_SRV_STATUS_INDICATES_1XEVDO_SRV_AVAILABLE( srv_status, sys_mode, hdr_hybrid, hdr_srv_status ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_SRV_AVAILABLE( srv_status, sys_mode ) ||                                \
    QCRIL_CM_SRV_STATUS_INDICATES_HDR_SRV_AVAILABLE( srv_status, sys_mode ) ||                                 \
    QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_SRV_AVAILABLE( hdr_hybrid, hdr_srv_status, sys_mode ) )

/* Determine whether the service status indicates GSM full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_GSM_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_GSM( sys_mode ) )

/* Determine whether the service status indicates WCDMA full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_WCDMA_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_WCDMA( sys_mode ) )

/* Determine whether the service status indicates LTE full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_LTE_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_LTE( sys_mode ) )

/* Determine whether the service status indicates GW full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_GW_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_GW( sys_mode ) )

/* Determine whether the service status indicates GWL full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_GWL_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_GWL( sys_mode ) )

/* Determine whether the service status indicates CDMA full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_CDMA( sys_mode ) )

/* Determine whether the service status indicates HDR full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_HDR_FULL_SRV( srv_status, sys_mode ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( srv_status ) && QCRIL_CM_SYS_MODE_IS_HDR( sys_mode ) ) 

/* Determine whether the service status indicates hybrid HDR full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_FULL_SRV( hdr_hybrid, hdr_srv_status ) \
  ( hdr_hybrid && QCRIL_CM_SRV_STATUS_INDICATES_FULL_SRV( hdr_srv_status ) ) 

/* Determine whether the service status indicates 1XEVO full service */
#define QCRIL_CM_SRV_STATUS_INDICATES_1XEVDO_FULL_SRV( srv_status, sys_mode, hdr_hybrid, hdr_srv_status ) \
  ( QCRIL_CM_SRV_STATUS_INDICATES_CDMA_FULL_SRV( srv_status, sys_mode ) || \
    QCRIL_CM_SRV_STATUS_INDICATES_HDR_FULL_SRV( srv_status, sys_mode ) ||  \
    QCRIL_CM_SRV_STATUS_INDICATES_HYBRID_HDR_FULL_SRV( hdr_hybrid, hdr_srv_status ) )

/* Determine whether the network preference supports CDMA/EVDO */
#define QCRIL_CM_NET_PREF_INDICATES_1XEVDO( net_pref ) \
  ( (net_pref == QCRIL_CM_NET_PREF_CDMA_EVDO) ||(net_pref == QCRIL_CM_NET_PREF_CDMA_ONLY)  ||     \
    (net_pref == QCRIL_CM_NET_PREF_EVDO_ONLY) || (net_pref == QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO) ||\
    (net_pref == QCRIL_CM_NET_PREF_LTE_CDMA_EVDO) || (net_pref == QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA) )

/* Determine whether the network preference is CDMA/EVDO only */ 
#define QCRIL_CM_NET_PREF_INDICATES_1XEVDO_ONLY( net_pref ) \
  ( (net_pref == QCRIL_CM_NET_PREF_CDMA_EVDO) ||(net_pref == QCRIL_CM_NET_PREF_CDMA_ONLY)  ||     \
    (net_pref == QCRIL_CM_NET_PREF_EVDO_ONLY) )

#define QCRIL_CM_ALPHA_TEXT_MAX_SIZE 90

typedef enum
{
  QCRIL_CM_NET_PREF_GSM_WCDMA_PREFERRED     = 0,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_GSM_ONLY                = 1,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_WCDMA_ONLY              = 2,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_GSM_WCDMA_AUTO          = 3,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_CDMA_EVDO               = 4,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_CDMA_ONLY               = 5,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_EVDO_ONLY               = 6,  /* Option allowed for non-fusion and fusion */
  QCRIL_CM_NET_PREF_GSM_WCDMA_CDMA_EVDO     = 7,  /* Option allowed for non-fusion */
  QCRIL_CM_NET_PREF_LTE_CDMA_EVDO           = 8,  /* Option allowed for fusion */
  QCRIL_CM_NET_PREF_LTE_GSM_WCDMA           = 9,  /* Option allowed for fusion */
  QCRIL_CM_NET_PREF_LTE_CDMA_EVDO_GSM_WCDMA = 10, /* Option allowed for fusion */
  QCRIL_CM_NET_PREF_LTE_ONLY                = 11  /* Option allowed for fusion */
} qcril_cm_net_pref_e_type; 

typedef enum
{
  QCRIL_CM_SUBSCRIPTION_DEACTIVATED = 0,      /* subscription deactivated at modem */
  QCRIL_CM_SUBSCRIPTION_ACTIVATED = 1         /* subscription activated at modem */
}qcril_cm_subscription_status;

/*! @brief Radio Mode
*/
typedef struct
{
  sys_oprt_mode_e_type oprt_mode;
  boolean is_command_instance;
} qcril_cm_radio_mode_type;

/*! @brief Preferred Network Type 
*/
typedef struct
{
  qcril_cm_net_pref_e_type net_pref;
  cm_mode_pref_e_type mode_pref[ QCRIL_MAX_MODEM_ID ];
  cm_gw_acq_order_pref_e_type acq_order_pref[ QCRIL_MAX_MODEM_ID ];
  sys_oprt_mode_e_type oprt_mode[ QCRIL_MAX_MODEM_ID ];
} qcril_cm_network_pref_type;

/*! Manual network selection 
*/
typedef struct
{
  boolean                    same_plmn; /* Manual network selection on the same PLMN */
  sys_plmn_id_s_type         plmn;      /* PLMN info */
} qcril_cm_manual_network_sel_type;

/*! Supplimentary services information maintained in QCRIL for all the
    requests sent to CM
*/
typedef struct
{
  int  ss_code;
  int  ss_ref;
  int  ss_service_class;              /* required for responding to interrogation result */
  qcril_cm_ss_password_T ss_pwd;      /* required for responding to password request from AMSS */
  boolean req_from_other_client;     /* request is from other client */
} qcril_cm_ss_type;

/*! @brief CM Request Info 
*/
typedef union
{
  qcril_cm_radio_mode_type         radio_mode;

  #ifdef FEATURE_QCRIL_DSDS
  RIL_SelectUiccSub                select_uicc_sub;
  #endif /* FEATURE_QCRIL_DSDS */

  qcril_cm_network_pref_type       network_pref;
  qcril_cm_manual_network_sel_type manual_network_sel;
  cm_band_pref_e_type              band_pref;
  cm_roam_pref_e_type              roam_pref;
  cm_rtre_config_e_type            rtre_config;
  cm_privacy_pref_e_type           privacy_pref;
  qcril_cm_ss_type                 ss_info;
} qcril_cm_req_info_type;

/*! @brief Structure used to save the registration/authentication reject cause 
*/
typedef struct
{
  boolean reg_reject_reported;
  boolean managed_roaming_enabled;
  cm_reg_reject_info_s_type cm_reg_reject_info;
} qcril_cm_reg_reject_info_type;

#ifdef FEATURE_QCRIL_DSDS
/*! @brief Structure used to save the subsription preference informaiton
*/
typedef struct
{
  boolean is_pending;
  cm_subs_info_s_type cm_subs_info;
} qcril_cm_subs_pref_info;
#endif /* FEATURE_QCRIL_DSDS */

/*! @brief CM Serving System Info 
*/
typedef struct
{ 
  uint64                     changed_fields;
  sys_srv_status_e_type      srv_status;
  sys_srv_status_e_type      true_srv_status;
  sys_srv_domain_e_type      srv_domain;
  sys_srv_domain_e_type      srv_capability;
  sys_sys_mode_e_type        sys_mode;
  sys_roam_status_e_type     roam_status;
  sys_sys_id_s_type          sys_id;
  boolean                    is_sys_prl_match;
  uint16                     rssi;
  int16                      ecio;
  byte                       sinr;
  sys_sys_mode_e_type        cur_idle_digital_mode;
  boolean                    hdr_hybrid;
  sys_srv_status_e_type      hdr_srv_status;
  sys_roam_status_e_type     hdr_roam_status;
  uint16                     hdr_rssi;
  int16                      hdr_ecio;
  byte                       hdr_sinr;
  sys_active_prot_e_type     hdr_active_prot;
  #ifdef FEATURE_QCRIL_EHRPD
  sys_personality_e_type     hdr_personality;
  #endif /* FEATURE_QCRIL_EHRPD */
  int16                      rsrp;  
  int8                       rsrq;  
  cm_ss_info_u_type          mode_info;
  cm_cell_srv_ind_s_type     cell_srv_ind;
  sys_cell_info_s_type       cell_info;
  sys_lac_type               tac;
  sys_roam_status_e_type     def_roam_ind;
  boolean gw_hybrid;
} qcril_cm_ss_info_type;

typedef enum
{
  NOT_DUP_REQUEST = 1,     /* Normal request */
  DUP_REQUEST = 2,         /* Duplicate non DSDS request or DSDS request
                              which do not have a corresponding request on other instance */
  BINDED_DUP_REQUEST = 3,  /* Duplicate DSDS request which is binded to corresponding request
                              on other instance */
} qcril_flow_control_e_type;

//To get eme_cbm status
cm_ph_state_e_type qcril_cm_get_eme_cbm(qcril_instance_id_e_type id);

#endif /* QCRIL_CM_H */
