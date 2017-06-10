/*!
  @file
  qcril_arb.h

  @brief

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

#ifndef QCRIL_ARB_H
#define QCRIL_ARB_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "qcrili.h"
#include "qcril_cmi.h"
#include "qcril_smsi.h"
#include "qcril_pbmi.h"
#include "qcril_otheri.h"
#include "qmi_wds_srvc.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Persistent System Property that indicates split modem architecture */ 
#define QCRIL_ARB_SMA                    "persist.radio.sma" 

/* Persistent System Property that indicates Voip is support */
#define QCRIL_ARB_VOIP                   "persist.radio.voip_enabled"

/* Persistent System Property that indicates the split modem's global mode voice preference */
#define QCRIL_ARB_SMA_VOICE_PREF_3GPP    "persist.radio.sma_voice_3gpp" 

/* Persistent System Property that indicates whether DSDS/DSDA is enabled */
#define QCRIL_ARB_DSDS                   "persist.radio.multisim.config"

/* Persistent DSDS/DSDA System Property length */
#define QCRIL_ARB_DSDS_PROP_LENGTH       4

/* Persistent System Property for allowing voice + data,
    telephony expects that property to be set to true for SVLTE type*/
#define QCRIL_ARB_SVLTE                  "ro.ril.svlte1x"

/* Persistent System Property that indicates the network preference */ 
#define QCRIL_ARB_NET_PREF               "persist.radio.net_pref" 

/* Persistent System Property that indicates if prl_pref should be set */
#define QCRIL_ARB_SET_PRL_PREF           "persist.radio.set_prl_pref"

#define QCRIL_ARB_MAX_MODEMS             qcril_arb_query_max_num_of_modems()

#define QCRIL_ARB_MAX_SLOTS              qcril_arb_query_max_num_of_slots()

#define QCRIL_ARB_MAX_INSTANCES          qcril_arb_query_max_num_of_instances()

/*! @brief Modem Architecture Indicator
*/
typedef enum
{
  QCRIL_ARB_MA_MULTIMODE      = 0, /* Multimode */
  QCRIL_ARB_MA_FUSION_QCS     = 1, /* Fusion (Qualcomm) */
  QCRIL_ARB_MA_FUSION_TPS     = 2, /* Fusion (Third party) */
  QCRIL_ARB_MA_DSDS           = 3, /* DSDS */
  QCRIL_ARB_MA_MAX            = 4
} qcril_arb_ma_e_type;

/*! @brief Preferred Data Technology
*/
typedef enum
{
  QCRIL_ARB_PREF_DATA_TECH_INVALID = -1,
  QCRIL_ARB_PREF_DATA_TECH_UNKNOWN = 0,
  QCRIL_ARB_PREF_DATA_TECH_CDMA    = 1,
  QCRIL_ARB_PREF_DATA_TECH_EVDO    = 2,
  QCRIL_ARB_PREF_DATA_TECH_GSM     = 3,
  QCRIL_ARB_PREF_DATA_TECH_UMTS    = 4,
  QCRIL_ARB_PREF_DATA_TECH_EHRPD   = 5,
  QCRIL_ARB_PREF_DATA_TECH_LTE     = 6,
  QCRIL_ARB_PREF_DATA_TECH_TDSCDMA = 7
} qcril_arb_pref_data_tech_e_type;

/*! @brief Cache Type
*/
typedef enum
{
  QCRIL_ARB_CACHE_STATE = 0,
  QCRIL_ARB_CACHE_CM    = 1,
  QCRIL_ARB_CACHE_SMS   = 2,
  QCRIL_ARB_CACHE_PBM   = 3,
  QCRIL_ARB_CACHE_OTHER = 4
} qcril_arb_cache_e_type;

/*! @brief Phone Service Category
*/
typedef enum
{
  QCRIL_ARB_PH_SRV_CAT_3GPP2,    /* Phone 3GPP2 Service */
  QCRIL_ARB_PH_SRV_CAT_3GPP,     /* Phone 3GPP Service */
  QCRIL_ARB_PH_SRV_CAT_COMMON,   /* Phone Common Service */  
  QCRIL_ARB_PH_SRV_CAT_MODE_PREF /* Phone Mode Preference Specific Service */  
} qcril_arb_ph_srv_cat_e_type;

/*! @brief SMS Service Category
*/
typedef enum
{
  QCRIL_ARB_SMS_SRV_CAT_3GPP2,  /* SMS 3GPP2 SMS Service */
  QCRIL_ARB_SMS_SRV_CAT_3GPP,   /* SMS 3GPP Service */
  QCRIL_ARB_SMS_SRV_CAT_COMMON, /* SMS Common Service */
  QCRIL_ARB_SMS_SRV_CAT_IMS,    /* SMS IMS Service */
  QCRIL_ARB_SMS_SRV_CAT_IMS_REG /* SMS IMS Registration Service */
} qcril_arb_sms_srv_cat_e_type;

/*! @brief NV Service Category
*/
typedef enum
{
  QCRIL_ARB_NV_SRV_CAT_3GPP2, /* 3GPP2 NV item */
  QCRIL_ARB_NV_SRV_CAT_3GPP,  /* 3GPP NV item */
  QCRIL_ARB_NV_SRV_CAT_COMMON /* NV item commonly used by all tech */
} qcril_arb_nv_srv_cat_e_type;

/*! @brief Structure used to cache QCRIL instance state
*/
typedef struct
{
  #ifdef FEATURE_QCRIL_SUBS_CTRL
  uint16 subscription_config_mask;            /*!< Indicates the subscription configuration */
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  RIL_RadioState radio_state;                 /*!< Radio State */
  qcril_modem_state_e_type modem_state;       /*!< Modem State */
  qcril_sim_state_e_type pri_gw_sim_state;    /*!< Primary GSM/WCDMA SIM State */
  qcril_sim_state_e_type pri_cdma_sim_state;  /*!< Primary CDMA SIM State */
  qcril_sim_state_e_type sec_gw_sim_state;    /*!< Secondary GSM/WCDMA SIM State */
  qcril_sim_state_e_type sec_cdma_sim_state;  /*!< Secondary CDMA SIM State */
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type voice_radio_tech;   /*!< Indicates the voice radio technology that modem camped on */
  #else
  qcril_radio_tech_e_type        voice_radio_tech;   /*!< Indicates the voice radio technology that modem camped on */
  #endif
} qcril_arb_state_info_struct_type;

typedef struct
{
  pthread_mutex_t mutex;                                          /* Mutex used to control simultaneous update/access to state info */
  qcril_arb_state_info_struct_type info[ QCRIL_MAX_INSTANCE_ID ];
} qcril_arb_state_struct_type;

#ifdef FEATURE_QCRIL_DSDS
typedef enum
{
  QCRIL_ARB_SUBS_NOT_PROVISIONED = 0,
  QCRIL_ARB_SUBS_APPS_SELECTED = 1,
  QCRIL_ARB_SUBS_PROVISIONED = 2,
  QCRIL_ARB_SUBS_APPS_NOT_SELECTED = 3
} qcril_arb_subs_prov_status_e_type;
typedef struct
{
  qcril_arb_subs_prov_status_e_type state;  /*! Subscription provisioning status */
  RIL_SelectUiccSub uicc_sub;               /*! UICC Subscription Info */
  qmi_uim_session_type session_type;        /*! UIM session type on which this subscription is currently provisioned. */
  sys_modem_as_id_e_type as_id;             /*! AMSS Subscription ID */
  boolean subs_deact_pending;               /* for pending subscription deactivation due to hotswap */
  boolean card_removed;                     /* storing the card removed info due to hotswap */
} qcril_arb_subs_info_struct_type;

typedef struct
{
  qcril_arb_subs_info_struct_type info[ QCRIL_MAX_INSTANCE_ID ];
} qcril_arb_subs_struct_type;
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_arb_init( void );

void *qcril_arb_allocate_cache( qcril_arb_cache_e_type cache_type );

void qcril_arb_query_net_pref( qcril_instance_id_e_type instance_id, boolean *net_pref_restored, 
                               qcril_cm_net_pref_e_type *net_pref_ptr, qcril_arb_ma_e_type *ma_ptr );

void qcril_arb_store_net_pref( qcril_instance_id_e_type instance_id, qcril_cm_net_pref_e_type net_pref );

uint8 qcril_arb_query_max_num_of_modems( void );

uint8 qcril_arb_query_max_num_of_slots( void );

qcril_instance_id_e_type qcril_arb_query_max_num_of_instances( void );

void qcril_arb_query_arch_modem_id( qcril_modem_id_e_type *cdma_modem_id, qcril_modem_id_e_type *evdo_modem_id, 
                                    qcril_modem_id_e_type *gwl_modem_id );

void qcril_arb_query_voice_tech_modem_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type *cdma_modem_id, 
                                          qcril_modem_id_e_type *gw_modem_id );

void qcril_arb_query_data_tech_modem_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type *cdma_modem_id,
                                         qcril_modem_id_e_type *evdo_modem_id, qcril_modem_id_e_type *gwl_modem_id,
                                         qcril_modem_id_e_type *pref_data_tech_modem_id, qcril_arb_pref_data_tech_e_type *pref_data_tech );


void qcril_arb_set_data_sys_status( qcril_instance_id_e_type     instance_id,
                                    qmi_wds_data_sys_status_type data_sys_status );

void qcril_arb_set_pref_data_tech( qcril_instance_id_e_type instance_id, qcril_arb_pref_data_tech_e_type pref_data_tech );

IxErrnoType qcril_arb_query_ph_srv_modem_id( qcril_arb_ph_srv_cat_e_type ph_srv_cat, qcril_instance_id_e_type instance_id,
                                             qcril_modem_ids_list_type *modem_ids_list_ptr );

IxErrnoType qcril_arb_query_voice_srv_modem_id( qcril_instance_id_e_type instance_id,
                                                qcril_modem_id_e_type *modem_id_ptr,
                                                #ifndef FEATURE_ICS
                                                qcril_radio_tech_family_e_type *radio_tech_ptr
                                                #else
                                                qcril_radio_tech_e_type *radio_tech_ptr
                                                #endif
                                              );

IxErrnoType qcril_arb_query_sms_srv_modem_id( qcril_arb_sms_srv_cat_e_type sms_srv_cat, qcril_instance_id_e_type instance_id,
                                              qcril_modem_ids_list_type *modem_ids_list_ptr );

IxErrnoType qcril_arb_query_auth_srv_modem_id( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type *modem_id_ptr );

IxErrnoType qcril_arb_query_nv_srv_modem_id( qcril_arb_nv_srv_cat_e_type nv_srv_cat, qcril_instance_id_e_type instance_id,
                                             qcril_modem_ids_list_type *modem_ids_list_ptr );

boolean  qcril_arb_cdma_subscription_is_nv( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id );

boolean qcril_arb_ma_is_fusion( void  );

boolean qcril_arb_ma_is_dsds( void  );

#ifdef FEATURE_QCRIL_DSDS
IxErrnoType qcril_arb_lookup_instance_id_from_session_type( qmi_uim_session_type session_type,
                                                            qcril_instance_id_e_type *instance_id_ptr );

IxErrnoType qcril_arb_lookup_instance_id_from_as_id( sys_modem_as_id_e_type as_id,  qcril_instance_id_e_type *instance_id_ptr );

IxErrnoType qcril_arb_lookup_as_id_from_instance_id( qcril_instance_id_e_type instance_id,  sys_modem_as_id_e_type *as_id_ptr );

void qcril_arb_select_subs_apps( qcril_instance_id_e_type instance_id, RIL_SelectUiccSub *uicc_sub_ptr, 
                                 qmi_uim_session_type session_type );

void qcril_arb_activate_subs( qcril_instance_id_e_type instance_id, sys_modem_as_id_e_type as_id );

void qcril_arb_unselect_subs_apps( qcril_instance_id_e_type instance_id, RIL_SelectUiccSub *uicc_sub_ptr,
                                   qmi_uim_session_type session_type );

void qcril_arb_deactivate_subs( qcril_instance_id_e_type instance_id );

void qcril_arb_deactivate_all_subs( void );

void qcril_arb_query_subs( qcril_instance_id_e_type instance_id, qcril_arb_subs_prov_status_e_type *subs_state_ptr,
                           RIL_SelectUiccSub *uicc_sub_ptr, sys_modem_as_id_e_type *as_id_ptr, 
                           qmi_uim_session_type *session_type_ptr );

IxErrnoType qcril_arb_lookup_subs_from_session_type ( qmi_uim_session_type session_type, qcril_arb_subs_prov_status_e_type *subs_state_ptr,
                                                      RIL_SelectUiccSub *uicc_sub_ptr,  sys_modem_as_id_e_type *as_id_ptr,
                                                      qcril_instance_id_e_type *instance_id_ptr );

void qcril_arb_query_active_subs_list( qcril_sub_ids_list_type *sub_ids_list  );

void qcril_arb_update_subs_deactivation_pending_flag( qcril_instance_id_e_type instance_id, boolean subs_deactivated );

void qcril_arb_query_active_subs_instances_by_card ( int slot, qcril_instance_ids_list_type *sub_ids_list);

boolean qcril_arb_query_subs_deactivation_pending_flag (  qcril_instance_id_e_type instance_id );

void qcril_arb_update_card_removed_flag( qcril_instance_id_e_type instance_id, boolean is_card_removed );

boolean qcril_arb_query_card_removed_flag( qcril_instance_id_e_type instance_id );
#endif /* FEATURE_QCRIL_DSDS */

boolean  qcril_arb_rtre_control_is_nv( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, boolean lock_ph_mutex );

boolean qcril_arb_jpn_band_is_supported( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id );

boolean qcril_arb_in_airplane_mode( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id );

boolean qcril_arb_in_emerg_cb_mode ( qcril_instance_id_e_type instance_id );

boolean qcril_arb_voip_is_enabled( void );

boolean qcril_arb_voip_is_supported( void );

#endif /* QCRIL_ARB_H */
