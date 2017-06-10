/*!
  @file
  qcril_cm_api_map.h

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
04/06/10   pg      First cut.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#ifndef QCRIL_CM_API_MAP_H
#define QCRIL_CM_API_MAP_H

#include "comdef.h"
#include "cm.h"
#ifdef FEATURE_QCRIL_FUSION
#include "cm_fusion.h"
#endif /* FEATURE_QCRIL_FUSION */
#include "qcrili.h"


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

typedef cm_client_status_e_type (cm_client_ph_reg_f_type) (

  cm_client_id_type               client_id,
    /**< Requesting client */

  cm_ph_event_f_type             *ph_event_func,
    /**< Pointer to a callback function to notify the client of phone
    ** events */

  cm_client_event_reg_e_type      event_reg_type,
    /**< register or deregister */

  cm_ph_event_e_type              from_ph_event,
    /**< register from this event (including) */

  cm_ph_event_e_type              to_ph_event,
    /**< to this event (including) */

  cm_ph_cmd_err_f_type           *ph_cmd_err_func
    /**< pointer to a callback function to notify the client of phone
    ** command errors of OTHER clients */

);

typedef cm_client_status_e_type (cm_mm_client_ss_reg_f_type) (

  cm_client_id_type               client_id,
    /**< Requesting client */

  cm_mm_ss_event_f_type          *ss_event_func,
    /**< Pointer to a callback function to notify the client of serving system
    ** events */

  cm_client_event_reg_e_type      event_reg_type,
    /**< register or deregister */

  cm_ss_event_e_type              from_ss_event,
    /**< register from this event (including) */

  cm_ss_event_e_type              to_ss_event,
    /**< to this event (including) */

  cm_ss_cmd_err_f_type           *ss_cmd_err_func
    /**< pointer to a callback function to notify the client of serving system
    ** command errors of OTHER clients */

);

typedef cm_client_status_e_type  (cm_mm_client_call_reg_f_type) (

  cm_client_id_type           client_id,
    /**< Requesting client */

  cm_mm_call_event_f_type     *call_event_func,
    /**< Pointer to a callback function to notify the client of call
    ** events */

  cm_client_event_reg_e_type  event_reg_type,
    /**< register or deregister */

  cm_call_event_e_type        from_call_event,
    /**< register from this event (including) */

  cm_call_event_e_type        to_call_event,
    /**< to this event (including) */

  cm_mm_call_cmd_err_f_type   *call_cmd_err_func
    /**< pointer to a callback function to notify the client of call
    ** command errors of OTHER clients */

);

typedef cm_client_status_e_type  (cm_mm_client_inband_reg_f_type) (

  cm_client_id_type               client_id,
    /**< Requesting client */

  cm_inband_event_f_type          *inband_event_func,
    /**< Pointer to a callback function to notify the client of inband
    ** events */

  cm_client_event_reg_e_type      event_reg_type,
    /**< register or deregister */

  cm_inband_event_e_type          from_inband_event,
    /**< register from this event (including) */

  cm_inband_event_e_type          to_inband_event,
    /**< to this event (including) */

  cm_mm_inband_cmd_err_f_type     *inband_cmd_err_func
    /**< pointer to a callback function to notify the client of inband
    ** command errors of OTHER clients */

);

typedef cm_client_status_e_type (cm_client_sups_reg_f_type) (

  cm_client_id_type             client_id,
    /**< Requesting client */

  cm_sups_event_f_type          *sups_event_func,
    /**< Pointer to a callback function to notify the client of sups
    ** events */

  cm_client_event_reg_e_type    event_reg_type,
    /**< register or deregister */

  cm_sups_event_e_type          from_event,
    /**< register from this event (including) */

  cm_sups_event_e_type          to_event,
    /**< to this event (including) */

  cm_sups_cmd_err_f_type        *sups_cmd_err_func
    /**< pointer to a callback function to notify the client of sups
    ** command errors of OTHER clients */

);

#ifdef FEATURE_QCRIL_DSDS
typedef cm_client_status_e_type (cm_client_subs_reg_f_type) (

  cm_client_id_type               client_id,
    /**< Requesting client. */

  cm_subs_event_f_type            *subs_event_func
    /**< Pointer to a callback function to notify the client of phone
         events. */
);
#endif /* FEATURE_QCRIL_DSDS */

typedef cm_client_status_e_type (cm_client_act_f_type) (

  cm_client_id_type               client_id
    /**< Requesting client */

);

typedef cm_client_status_e_type (cm_client_init_f_type) (

  cm_client_type_e_type           client_type,
    /**< Type of client */
  cm_client_id_type               *client_id_ptr
    /**< Client ID pointer*/
);

typedef cm_client_status_e_type (cm_client_release_f_type) (

  cm_client_id_type               client_id
    /**< Client ID pointer*/

);

typedef boolean (cm_ph_cmd_oprt_mode_f_type) (

  cm_ph_cmd_cb_f_type    cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client_id,
    /**< Requesting client */

  sys_oprt_mode_e_type   oprt_mode
    /**< Switch to this operating mode */

);

typedef boolean (cm_ph_cmd_change_rtre_config_f_type) (

  cm_ph_cmd_cb_f_type    cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client_id,
    /**< Requesting client */

  cm_rtre_config_e_type  rtre_config
    /**< new RTRE configuration */

);

typedef boolean (cm_ph_cmd_signal_strength_delta_f_type) (

  cm_ph_cmd_cb_f_type    cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client,
    /**< requesting client ID */

  uint8                  rssi_delta,
    /**< RSSI change delta threshold for Signal Strength reporting */

  uint8                  ecio_delta,
    /**< ECIO change delta threshold for Signal Strength  reporting */

  uint8                  io_delta
    /**< IO change delta threshold for Signal Strength  reporting */
);

#ifdef FEATURE_QCRIL_SUBS_CTRL
typedef boolean (cm_ph_cmd_subscription_available_f_type) (

  cm_ph_cmd_cb_f_type             cmd_cb_func,
    /**< client callback function */

  void                            *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type               client_id,
    /**< Requesting client */

  cm_subscription_status_e_type   cdma_status,
    /**< Subscription status of CDMA system */

  cm_subscription_status_e_type   gwl_status,
    /**< Subscription status of GSM/WCDMA/LTE system */

  cm_subscription_status_e_type   wlan_status
    /**< Subscription status of WLAN system.
    ** Clients check CM_API_WLAN for WLAN support */

);

typedef boolean (cm_ph_cmd_subscription_not_available_f_type) (

  cm_ph_cmd_cb_f_type             cmd_cb_func,
    /**< client callback function */

  void                            *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type               client_id,
    /**< Requesting client */

  cm_subscription_status_e_type   cdma_status,
    /**< Subscription status of CDMA system */

  cm_subscription_status_e_type   gwl_status,
    /**< Subscription status of GSM/WCDMA/LTE system */

  cm_subscription_status_e_type   wlan_status
    /**< Subscription status of WLAN system.
    ** Clients check CM_API_WLAN for WLAN support */

);
#endif /* FEATURE_QCRIL_SUBS_CTRL */

#ifdef FEATURE_QCRIL_DSDS
typedef boolean (cm_ph_cmd_get_subs_pref_info_f_type) (

  cm_ph_cmd_cb_f_type     cmd_cb_func,
        /**< Client callback function. */

  void                    *data_block_ptr,
        /**< Pointer to the client callback data block. */

  cm_client_id_type       client,
        /**< Requesting client ID. */

  sys_modem_as_id_e_type  asubs_id
        /**< Active subscription ID. */
);

typedef boolean (cm_ph_cmd_dual_standby_pref_f_type) (

  cm_ph_cmd_cb_f_type                 cmd_cb_func,
    /**< Client callback function. */

  void                                *data_block_ptr,
    /**< Pointer to the client callback data block. */

  cm_client_id_type                    client_id,
    /**< Requesting client. */

  sys_modem_dual_standby_pref_e_type   standby_pref,
    /**< Standby preference of the phone. */

  sys_modem_as_id_e_type               active_subs,
    /**< The active subscription in Single Standby mode.
         Only valid when standby_pref is SYS_MODEM_DS_PREF_SINGLE_STANDBY. */

  sys_modem_as_id_e_type               default_voice_subs,
    /**< The system type to avoid. */

  sys_modem_as_id_e_type               default_data_subs,
    /**< The system type to avoid. */

  sys_modem_as_id_e_type               priority_subs
    /**< The system type to avoid. */
);

typedef boolean (cm_ph_cmd_sys_sel_pref_per_subs_f_type) (

  cm_ph_cmd_cb_f_type               cmd_cb_func,
    /**< Client callback function */

  void                              *data_block_ptr,
    /**< Pointer to client callback data block */

  cm_client_id_type                 client_id,
    /**< Requesting client */

  sys_modem_as_id_e_type            asubs_id,
    /* Subscription ID to which these preferences should apply to */

  cm_mode_pref_e_type               mode_pref,
    /**< Switch to this mode preference */

  cm_pref_term_e_type               pref_term,
    /**< Switch the preference for this term */

  dword                             pref_duration,
    /**< Specify the duration in seconds for temporary term */

  cm_gw_acq_order_pref_e_type       acq_order_pref,
    /**< gw acquisition order preference */

  cm_band_pref_e_type               band_pref,
    /**< Switch to this band preference */

  cm_prl_pref_e_type                prl_pref,
    /**< Switch to this prl preference */

  cm_roam_pref_e_type               roam_pref,
    /**< Switch to this roam preference */

  cm_hybr_pref_e_type               hybr_pref,
    /**< Switch to this hybrid preference */

  cm_srv_domain_pref_e_type         srv_domain_pref,
    /**< Switch to this service domain (e.g. Circuit Switch)
    ** Set to CM_SRV_DOMAIN_PREF_NO_CHANGE if a service
    ** domain is not being changed. */

  cm_network_sel_mode_pref_e_type   network_sel_mode_pref,
    /**< Defines whether the network should be selected automatically
    ** or manually */

  const sys_plmn_id_s_type                *plmn_ptr
    /**< If network selection is set to manual, this specifies plmn id */

);

typedef boolean (cm_ph_cmd_get_ph_info_per_subs_f_type) (

  cm_ph_cmd_cb_f_type    cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client_id,
    /**< requesting client ID */

  sys_modem_as_id_e_type asubs_id

);

typedef boolean (cm_ph_cmd_get_networks_per_subs_f_type) (

  cm_ph_cmd_cb_f_type          cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type            client_id,
    /**< Requesting client */

  cm_network_list_type_e_type  list_type,
    /**< Indicates whether to retrieve the preferred or available networks */

  cm_mode_pref_e_type          network_type,

    /**< Clients check CM_API_WLAN for WLAN support */

    /**< Indicates the desired system such as WLAN or GW.
    ** Only WLAN or GW is supported, one at a time can be
    ** requested only.
    */
  sys_modem_as_id_e_type       asubs_id
    /**< Subscription ID to use to retrieve the GW network list.
             Only valid when the network_type is GW and in Dual Standby mode. 
             In Single Standby mode, a search can only be performed on active
             subscriptions. */

);

typedef boolean (cm_ph_cmd_wakeup_from_standby_per_subs_f_type) (

  cm_ph_cmd_cb_f_type        cmd_cb_func,
    /**< client callback function */

  void                      *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type          client_id,
    /**< Requesting client */

  sys_modem_as_id_e_type  asubs_id
    /**< Subscription ID of the stack to awaken. */
);
#else
typedef boolean (cm_ph_cmd_sys_sel_pref_f_type) (

  cm_ph_cmd_cb_f_type               cmd_cb_func,
    /**< Client callback function */

  void                              *data_block_ptr,
    /**< Pointer to client callback data block */

  cm_client_id_type                 client_id,
    /**< Requesting client */

  cm_mode_pref_e_type               mode_pref,
    /**< Switch to this mode preference */

  cm_pref_term_e_type               pref_term,
    /**< Switch the preference for this term */

  dword                             pref_duration,
    /**< Specify the duration in seconds for temporary term */

  cm_gw_acq_order_pref_e_type       acq_order_pref,
    /**< gw acquisition order preference */

  cm_band_pref_e_type               band_pref,
    /**< Switch to this band preference */

  cm_prl_pref_e_type                prl_pref,
    /**< Switch to this prl preference */

  cm_roam_pref_e_type               roam_pref,
    /**< Switch to this roam preference */

  cm_hybr_pref_e_type               hybr_pref,
    /**< Switch to this hybrid preference */

  cm_srv_domain_pref_e_type         srv_domain_pref,
    /**< Switch to this service domain (e.g. Circuit Switch)
    ** Set to CM_SRV_DOMAIN_PREF_NO_CHANGE if a service
    ** domain is not being changed. */

  cm_network_sel_mode_pref_e_type   network_sel_mode_pref,
    /**< Defines whether the network should be selected automatically
    ** or manually */

  const sys_plmn_id_s_type                *plmn_ptr,
    /**< If network selection is set to manual, this specifies plmn id */

  const cm_wlan_pref_s_type               *wlan_pref_ptr
    /**< WLAN Prefereces.
    ** Clients check CM_API_WLAN for WLAN support */

);

typedef boolean (cm_ph_cmd_get_ph_info_f_type) (

  cm_ph_cmd_cb_f_type    cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client_id
    /**< requesting client ID */

);

typedef boolean (cm_ph_cmd_get_networks_f_type) (

  cm_ph_cmd_cb_f_type          cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type            client_id,
    /**< Requesting client */

  cm_network_list_type_e_type  list_type,
    /**< Indicates whether to retrieve the preferred or available networks */

  cm_mode_pref_e_type          network_type

    /**< Clients check CM_API_WLAN for WLAN support */

    /**< Indicates the desired system such as WLAN or GW.
    ** Only WLAN or GW is supported, one at a time can be
    ** requested only.
    */

);

typedef boolean (cm_ph_cmd_wakeup_from_standby_f_type) (

  cm_ph_cmd_cb_f_type        cmd_cb_func,
    /**< client callback function */

  void                      *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type          client_id
    /**< Requesting client */

);
#endif /* FEATURE_QCRIL_DSDS */

typedef boolean (cm_ss_cmd_get_ss_info_f_type) (

  cm_ss_cmd_cb_f_type  cmd_cb_func,
    /**< client callback function */

  void                 *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type    client_id
    /**< requesting client ID */

);

typedef void (cm_num_init_f_type) (

  cm_num_s_type *num_ptr
    /**< The number to be initialized */

); 

typedef void (cm_num_fill_f_type) (

    cm_num_s_type          *num_ptr,
        /**< Pointer to a CM number to be initialized */
    const uint8            *num_buf,
        /**< Dialed address buffer.
        **
        ** NOTE! In the case where the digit_mode parameter is set to
        ** CM_DIGIT_MODE_4BIT_DTMF the number buffer may only contain
        ** 0-9, "#", or "*" characters.
        **
        ** In the case where the digit_mode parameter is set to
        ** CM_DIGIT_MODE_8BIT_ASCII the number buffer may contain any
        ** 8-bit ASCII characters */

    uint8                  num_len,
        /**< dialed address length */

    cm_digit_mode_e_type   digit_mode
        /**< digit mode */

);

typedef void (cm_util_default_gw_orig_params_f_type) (

  cm_gw_cs_orig_params_s_type         *gw_orig_param_ptr

);

typedef void (cm_util_set_default_voice_bearer_cap_f_type) (

  cm_bearer_capability_s_type    *bc

);

typedef void (cm_util_default_cdma_orig_params_f_type) (

  cm_cdma_orig_params_s_type         *cdma_orig_param_ptr

);

#ifdef FEATURE_QCRIL_DSDS
typedef boolean (cm_mm_call_cmd_orig_per_subs_f_type) (

  cm_call_cmd_cb_f_type       cmd_cb_func,
    /**< Client callback function */

  void                        *data_block_ptr,
    /**< Pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< Originating client */

  cm_call_type_e_type         call_type,
    /**< Call type - voice, data, etc. */

  cm_srv_type_e_type          srv_type,
    /**< Service type - automatic, etc. */

  const sys_modem_as_id_e_type       asubs_id,
    /* Subscription-id on which the call is to be originated */

  const cm_num_s_type         *calling_number,
    /**< Number of mobile that is originating the call */

  const cm_num_s_type         *called_number,
    /**< Number dialed - phone that is being called */

  const cm_orig_alpha_s_type  *alpha_buf,
    /**< Alpha tag to be displayed for the origination */

  const cm_cdma_orig_params_s_type  *cdma_orig_params_ptr,
    /**< Pointer to cdma mode specific data */

  const cm_gw_cs_orig_params_s_type *gw_cs_orig_params_ptr,
    /**< Pointer to GW CS mode specific data */

  const cm_gw_ps_orig_params_s_type *gw_ps_orig_params_ptr,
    /**< Pointer to GW PS mode specific data */

  cm_call_id_type             *return_call_id_ptr
    /**< Call ID will be allocated and returned */

);

typedef boolean (cm_mm_call_cmd_sups_per_subs_f_type) (

  cm_call_cmd_cb_f_type       cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< sending client */

  cm_call_sups_type_e_type    sups_type,
    /**< Type of supplementary service */

  const cm_call_sups_params_s_type  *sups_params_ptr,
    /**< Pointer to supplementary service specific data */

  sys_modem_as_id_e_type            asubs_id
    /**< Active subscription id for origination */
);
#else
typedef boolean (cm_mm_call_cmd_orig_f_type) (

  #ifdef FEATURE_QCRIL_CALL_ORIG_EXTEN3
  cm_call_cmd_exten_cb_f_type       cmd_cb_func,
    /**< Client callback function */
  #else
  cm_call_cmd_cb_f_type       cmd_cb_func,
    /**< Client callback function */
  #endif

  void                        *data_block_ptr,
    /**< Pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< Originating client */

  cm_call_type_e_type         call_type,
    /**< Call type - voice, data, etc. */

  cm_srv_type_e_type          srv_type,
    /**< Service type - automatic, etc. */

  const cm_num_s_type         *calling_number,
    /**< Number of mobile that is originating the call */

  const cm_num_s_type         *called_number,
    /**< Number dialed - phone that is being called */

  const cm_orig_alpha_s_type  *alpha_buf,
    /**< Alpha tag to be displayed for the origination */

  const cm_cdma_orig_params_s_type  *cdma_orig_params_ptr,
    /**< Pointer to cdma mode specific data */

  const cm_gw_cs_orig_params_s_type *gw_cs_orig_params_ptr,
    /**< Pointer to GW CS mode specific data */

  const cm_gw_ps_orig_params_s_type *gw_ps_orig_params_ptr,
    /**< Pointer to GW PS mode specific data */

  #ifdef FEATURE_QCRIL_CALL_ORIG_EXTEN
  cm_call_id_type                   switch_out_call_id,
    /**< Switch out call id */

  const cm_cc_generic_params_s_type *cc_generic_params_ptr,
    /**< Call-control generic parameters. Default val = TRUE */
  #elif defined( FEATURE_QCRIL_CALL_ORIG_EXTEN3 )
  cm_call_id_type                   switch_out_call_id,
    /**< Switch out call id */

  cm_orig_client_categ_e_type       orig_client_categ,
    /**< Initiating client category. */
  #endif

  cm_call_id_type             *return_call_id_ptr
    /**< Call ID will be allocated and returned */

);

typedef boolean (cm_mm_call_cmd_sups_f_type) (

  cm_call_cmd_cb_f_type       cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< sending client */

  cm_call_sups_type_e_type    sups_type,
    /**< Type of supplementary service */

  const cm_call_sups_params_s_type  *sups_params_ptr
    /**< Pointer to supplementary service specific data */

);
#endif /* FEATURE_QCRIL_DSDS */

typedef boolean (cm_mm_call_cmd_answer_f_type) (

  cm_call_cmd_cb_f_type  cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client_id,
    /**< answering client */

  cm_call_id_type        call_id,
    /**< call ID of the call */

  const cm_ans_params_s_type   *ans_params_ptr
    /**< pointer to mode specific data */

);

typedef boolean (cm_mm_call_cmd_end_f_type) (

  cm_call_cmd_cb_f_type  cmd_cb_func,
    /**< client callback function */

  void                   *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type      client_id,
    /**< ending client */

  uint8                  num_call_ids,
    /**< number of call IDs sent */

  const cm_end_params_s_type   *end_params_ptr
    /**< pointer to mode specific data */

);

typedef boolean (cm_mm_call_cmd_privacy_pref_f_type) (

  cm_call_cmd_cb_f_type   cmd_cb_func,
    /**< client callback function */

  void                    *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type       client_id,
    /**< ending client */

  cm_call_id_type         call_id,
    /**< call ID of the call */

  cm_privacy_pref_e_type  privacy_pref
    /**< privacy pref setting */

);

typedef boolean (cm_mm_call_cmd_setup_res_f_type) (

  cm_call_cmd_cb_f_type         cmd_cb_func,
    /**< client callback function */

  void                          *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type             client_id,
    /**< answering client */

  cm_call_id_type               call_id,
    /**< call ID of the call */

  const cm_setup_res_params_s_type   *setup_params_ptr
    /**< pointer to mode specific data */

);

typedef boolean (cm_mm_inband_cmd_start_cont_dtmf_f_type) (

  cm_inband_cmd_cb_f_type     cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< Requesting client */

  cm_call_id_type             call_id,
    /**< call ID of the call */

  uint8                       digit
    /**< DTMF digit */

);

typedef boolean (cm_mm_inband_cmd_stop_cont_dtmf_f_type) (

  cm_inband_cmd_cb_f_type     cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< Requesting client */

  cm_call_id_type             call_id,
    /**< call ID of the call */

  uint16                      duration
    /**< DTMF duration */

);

typedef boolean (cm_mm_inband_cmd_burst_dtmf_f_type) (

  cm_inband_cmd_cb_f_type     cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< Requesting client */

  cm_call_id_type             call_id,
    /**< call ID of the call */

  uint8                       on_length,
    /**< DTMF pulse width */

  uint8                       off_length,
    /**< DTMF inter-digit interval */

  uint8                       cnt,
    /**< DTMF digit count */

  const uint8                 *dtmf_digit_ptr
    /**< Pointer to the DTMF digit buffer */

);

typedef void (cm_get_user_data_f_type) (

  const cm_mm_call_info_s_type      *call_info_ptr,
    /**< pointer to call info pointer sent to the clients */

  cm_call_event_user_data_s_type    *call_event_user_data
    /**< pointer to a client before for user data details */

);

#ifdef FEATURE_QCRIL_DSDS
typedef boolean (cm_sups_cmd_interrogate_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */

  void                         *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type            client_id,
    /**< originating client */

  const cm_interr_ss_params_s_type   *interr_cmd_params,
    /**< pointer to interrogate SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_deactivate_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type       sups_cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< originating client */

  const cm_deact_ss_params_s_type   *deact_cmd_params,
    /**< pointer to erase SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_activate_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type    sups_cmd_cb_func,
    /**< client callback function */

  void                     *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type        client_id,
    /**< originating client */

  const cm_act_ss_params_s_type  *act_cmd_params,
    /**< pointer to activate SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_register_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type     sups_cmd_cb_func,
    /**< client callback function */

  void                      *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type         client_id,
    /**< originating client */

  const cm_reg_ss_params_s_type   *reg_cmd_params,
    /**< pointer to register SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_erase_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type       sups_cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< originating client */

  const cm_erase_ss_params_s_type   *erase_cmd_params,
    /**< pointer to erase SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_reg_password_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type      sups_cmd_cb_func,
    /**< client callback function */

  void                       *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type          client_id,
    /**< originating client */

  const cm_reg_pwd_params_s_type   *reg_pwd_cmd_params,
    /**< pointer to register password SS command data */

  const cm_num_s_type              *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                           is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type            asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_process_uss_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type    sups_cmd_cb_func,
    /**< client callback function */

  void                     *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type        client_id,
    /**< originating client */

  const cm_uss_params_s_type     *process_uss_cmd_params,
    /**< pointer to Process Unstructured SS command data */

  const cm_num_s_type             *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                         is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type          asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_uss_res_per_subs_f_type) (

  cm_sups_cmd_cb_f_type         sups_cmd_cb_func,
    /**< client callback function */

  void                          *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type             client_id,
    /**< originating client */

  const cm_uss_res_params_s_type      *uss_res_params,
    /**< pointer to Unstructured SS response data */

  sys_modem_as_id_e_type              asubs_id
    /**< Active subscription ID. */

);

typedef boolean (cm_sups_cmd_release_cc_per_subs_f_type) (

  cm_sups_cmd_cb_f_type          sups_cmd_cb_func,
    /**< client callback function */

  void                           *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type              client_id,
    /**< originating client */

  const cm_release_ss_params_s_type    *release_cmd_params,
    /**< pointer to release SS command data */

  const cm_num_s_type                   *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  boolean                               is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type                asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_uss_notify_res_per_subs_f_type) (

  cm_sups_cmd_cb_f_type               sups_cmd_cb_func,
    /**< client callback function */

  void                                *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type                   client_id,
    /**< originating client */

  const cm_uss_notify_res_params_s_type     *uss_notify_res_params,
    /**< pointer to Unstructured SS Notify response data */

  sys_modem_as_id_e_type                    asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_get_password_res_per_subs_f_type) (

  cm_sups_cmd_cb_f_type             sups_cmd_cb_func,
    /**< client callback function */

  void                              *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type                 client_id,
    /**< originating client */

  const cm_get_pwd_res_params_s_type      *get_pwd_res_params,
    /**< pointer to Get Password response data */

  sys_modem_as_id_e_type                  asubs_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_interrogate_cc_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */

  void                         *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type            client_id,
    /**< originating client */

  const cm_interr_ss_params_s_type   *interr_cmd_params,
    /**< pointer to interrogate SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              subscription_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_deactivate_cc_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type       sups_cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< originating client */

  const cm_deact_ss_params_s_type   *deact_cmd_params,
    /**< pointer to erase SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              subscription_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_activate_cc_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type    sups_cmd_cb_func,
    /**< client callback function */

  void                     *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type        client_id,
    /**< originating client */

  const cm_act_ss_params_s_type  *act_cmd_params,
    /**< pointer to activate SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              subscription_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_register_cc_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type     sups_cmd_cb_func,
    /**< client callback function */

  void                      *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type         client_id,
    /**< originating client */

  const cm_reg_ss_params_s_type   *reg_cmd_params,
    /**< pointer to register SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              subscription_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_erase_cc_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type       sups_cmd_cb_func,
    /**< client callback function */

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< originating client */

  const cm_erase_ss_params_s_type   *erase_cmd_params,
    /**< pointer to erase SS command data */

  const cm_num_s_type                *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                             is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type              subscription_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_reg_password_cc_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type      sups_cmd_cb_func,
    /**< client callback function */

  void                       *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type          client_id,
    /**< originating client */

  const cm_reg_pwd_params_s_type   *reg_pwd_cmd_params,
    /**< pointer to register password SS command data */

  const cm_num_s_type              *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                           is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type            subscription_id
    /**< Active subscription ID. */
);

typedef boolean (cm_sups_cmd_process_uss_exten_per_subs_f_type) (

  cm_sups_cmd_exten_cb_f_type    sups_cmd_cb_func,
    /**< client callback function */

  void                     *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type        client_id,
    /**< originating client */

  const cm_uss_params_s_type     *process_uss_cmd_params,
    /**< pointer to Process Unstructured SS command data */

  const cm_num_s_type             *dialed_digits,
    /**< Pointer to cm_num_s_type of the digits dialed. */

  cm_orig_client_categ_e_type        orig_client_categ,
   /**< Initiating client category */

  boolean                         is_call_control_processing_complete,
    /**< Indicates if call control has been completed. */

  sys_modem_as_id_e_type          subscription_id
    /**< Active subscription ID. */
);

#else
typedef boolean (cm_sups_cmd_interrogate_f_type) (

    #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                         *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type            client_id,
    /**< originating client */

  const cm_interr_ss_params_s_type   *interr_cmd_params
    /**< pointer to interrogate SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_deactivate_f_type) (

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< originating client */

  const cm_deact_ss_params_s_type   *deact_cmd_params
    /**< pointer to erase SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_activate_f_type) (

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                     *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type        client_id,
    /**< originating client */

  const cm_act_ss_params_s_type  *act_cmd_params
    /**< pointer to activate SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_register_f_type) (

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                      *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type         client_id,
    /**< originating client */

  const cm_reg_ss_params_s_type   *reg_cmd_params
    /**< pointer to register SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_erase_f_type) (

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                        *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type           client_id,
    /**< originating client */

  const cm_erase_ss_params_s_type   *erase_cmd_params
    /**< pointer to erase SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_reg_password_f_type) (

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                       *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type          client_id,
    /**< originating client */

  const cm_reg_pwd_params_s_type   *reg_pwd_cmd_params
    /**< pointer to register password SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_process_uss_f_type) (

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_exten_cb_f_type  sups_cmd_cb_func,
  #else
  cm_sups_cmd_cb_f_type        sups_cmd_cb_func,
    /**< client callback function */
  #endif

  void                     *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type        client_id,
    /**< originating client */

  const cm_uss_params_s_type     *process_uss_cmd_params
    /**< pointer to Process Unstructured SS command data */

  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  ,
  const cm_num_s_type            *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  cm_orig_client_categ_e_type    orig_client_categ
   /**< Initiating client category */
  #elif !defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
  ,
  const cm_num_s_type          *dialed_digits,
    /**< pointer to a cm_num_s_type of the digitis dialed */

  boolean                      is_call_control_processing_complete
    /**< Indicates whether call control has already been completed */
  #endif

);

typedef boolean (cm_sups_cmd_uss_res_f_type) (

  cm_sups_cmd_cb_f_type         sups_cmd_cb_func,
    /**< client callback function */

  void                          *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type             client_id,
    /**< originating client */

  const cm_uss_res_params_s_type      *uss_res_params
    /**< pointer to Unstructured SS response data */

);

typedef boolean (cm_sups_cmd_release_f_type) (

  cm_sups_cmd_cb_f_type          sups_cmd_cb_func,
    /**< client callback function */

  void                           *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type              client_id,
    /**< originating client */

  const cm_release_ss_params_s_type    *release_cmd_params
    /**< pointer to release SS command data */

);

typedef boolean (cm_sups_cmd_uss_notify_res_f_type) (

  cm_sups_cmd_cb_f_type               sups_cmd_cb_func,
    /**< client callback function */

  void                                *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type                   client_id,
    /**< originating client */

  const cm_uss_notify_res_params_s_type     *uss_notify_res_params
    /**< pointer to Unstructured SS Notify response data */

);

typedef boolean (cm_sups_cmd_get_password_res_f_type) (

  cm_sups_cmd_cb_f_type             sups_cmd_cb_func,
    /**< client callback function */

  void                              *data_block_ptr,
    /**< pointer to client callback data block */

  cm_client_id_type                 client_id,
    /**< originating client */

  const cm_get_pwd_res_params_s_type      *get_pwd_res_params
    /**< pointer to Get Password response data */

); 
#endif /* FEATURE_QCRIL_DSDS */

#ifdef FEATURE_QCRIL_NCELL
typedef cm_client_status_e_type (cm_client_stats_reg_f_type) (

  cm_client_id_type             client,
    /**< client id */

  cm_stats_event_f_type         *stats_event_func,
    /**< Pointer to a callback function to notify the client of statistics
     ** events */

  cm_client_event_reg_e_type    event_reg_type,
    /**< Register/De register */

  cm_stats_event_e_type         from_stats_event,
    /**< register from this event (including) */

  cm_stats_event_e_type         to_stats_event,
    /**< to this event (including) */ 

  cm_stats_cmd_err_f_type       *stats_cmd_err_func
    /**< pointer to a callback function to notify the client of stats
     ** command errors of OTHER clients */
);

typedef boolean (cm_stats_set_modem_info_mask_f_type) (

   cm_stats_cmd_cb_f_type            cmd_cb_func,
     /**< Client callback function */

   void                              *data_block_ptr,
    /**< Pointer to client callback data block */

   cm_client_id_type                 client_id,
    /**< Requesting client */

   sys_modem_module_e_type           module_type,
    /**< Module for which mask is being set */

   uint64                            bit_mask
    /**< Bit mask value */
   
);
#endif /* FEATURE_QCRIL_NCELL */

typedef struct {

  cm_ph_event_f_type      *ph_event_callback_func;
  cm_mm_ss_event_f_type   *ss_event_callback_func;
  cm_mm_call_event_f_type *call_event_callback_func;
  cm_inband_event_f_type  *inband_event_callback_func;
  cm_sups_event_f_type    *sups_event_callback_func;

  #ifdef FEATURE_QCRIL_NCELL
  cm_stats_event_f_type   *stats_event_callback_func;
  #endif /* FEATURE_QCRIL_NCELL */

  #ifdef FEATURE_QCRIL_DSDS
  cm_subs_event_f_type    *subs_event_callback_func;
  #endif /* FEATURE_QCRIL_DSDS */

} qcril_cm_api_callbacks_type;

typedef struct {

  cm_client_act_f_type                          *cm_client_act_func;
  cm_client_init_f_type                         *cm_client_init_func;
  cm_client_release_f_type                      *cm_client_release_func;

  /* CM event handler registration functions */
  cm_client_ph_reg_f_type                       *cm_client_ph_reg_func;
  cm_mm_client_ss_reg_f_type                    *cm_mm_client_ss_reg_func;
  cm_mm_client_call_reg_f_type                  *cm_mm_client_call_reg_func;
  cm_mm_client_inband_reg_f_type                *cm_mm_client_inband_reg_func;
  cm_client_sups_reg_f_type                     *cm_client_sups_reg_func;

  #ifdef FEATURE_QCRIL_DSDS
  cm_client_subs_reg_f_type                     *cm_client_subs_reg_func;
  #endif /* FEATURE_QCRIL_DSDS */

  /* CM ph command functions */
  cm_ph_cmd_oprt_mode_f_type                    *cm_ph_cmd_oprt_mode_func;
  cm_ph_cmd_change_rtre_config_f_type           *cm_ph_cmd_change_rtre_config_func;
  cm_ph_cmd_signal_strength_delta_f_type        *cm_ph_cmd_signal_strength_delta_func;

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  cm_ph_cmd_subscription_available_f_type       *cm_ph_cmd_subscription_available_func;
  cm_ph_cmd_subscription_not_available_f_type   *cm_ph_cmd_subscription_not_available_func;
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  #ifdef FEATURE_QCRIL_DSDS
  cm_ph_cmd_sys_sel_pref_per_subs_f_type        *cm_ph_cmd_sys_sel_pref_func;
  cm_ph_cmd_get_ph_info_per_subs_f_type         *cm_ph_cmd_get_ph_info_func;
  cm_ph_cmd_get_networks_per_subs_f_type        *cm_ph_cmd_get_networks_func;
  cm_ph_cmd_wakeup_from_standby_per_subs_f_type *cm_ph_cmd_wakeup_from_standby_func;
  cm_ph_cmd_dual_standby_pref_f_type            *cm_ph_cmd_dual_standby_pref_func;
  cm_ph_cmd_get_subs_pref_info_f_type           *cm_ph_cmd_get_subs_pref_info_func;
  #else
  cm_ph_cmd_sys_sel_pref_f_type                 *cm_ph_cmd_sys_sel_pref_func;
  cm_ph_cmd_get_ph_info_f_type                  *cm_ph_cmd_get_ph_info_func;
  cm_ph_cmd_get_networks_f_type                 *cm_ph_cmd_get_networks_func;
  cm_ph_cmd_wakeup_from_standby_f_type          *cm_ph_cmd_wakeup_from_standby_func;
  #endif /* FEATURE_QCRIL_DSDS */

  /* CM ss command functions */
  cm_ss_cmd_get_ss_info_f_type                  *cm_ss_cmd_get_ss_info_func;

  /* CM util functions */
  cm_num_init_f_type                            *cm_num_init_func;
  cm_num_fill_f_type                            *cm_num_fill_func;
  cm_util_default_gw_orig_params_f_type         *cm_util_default_gw_orig_params_func;
  cm_util_set_default_voice_bearer_cap_f_type   *cm_util_set_default_voice_bearer_cap_func;
  cm_util_default_cdma_orig_params_f_type       *cm_util_default_cdma_orig_params_func;

  #ifdef FEATURE_QCRIL_DSDS
  cm_mm_call_cmd_orig_per_subs_f_type           *cm_mm_call_cmd_orig_func;
  cm_mm_call_cmd_sups_per_subs_f_type           *cm_mm_call_cmd_sups_func;
  #else
  cm_mm_call_cmd_orig_f_type                    *cm_mm_call_cmd_orig_func;
  cm_mm_call_cmd_sups_f_type                    *cm_mm_call_cmd_sups_func;
  #endif /* FEATURE_QCRIL_DSDS */

  cm_mm_call_cmd_answer_f_type                  *cm_mm_call_cmd_answer_func;
  cm_mm_call_cmd_end_f_type                     *cm_mm_call_cmd_end_func;
  cm_mm_call_cmd_privacy_pref_f_type            *cm_mm_call_cmd_privacy_pref_func;
  cm_mm_call_cmd_setup_res_f_type               *cm_mm_call_cmd_setup_res_func;

  cm_mm_inband_cmd_start_cont_dtmf_f_type       *cm_mm_inband_cmd_start_cont_dtmf_func;
  cm_mm_inband_cmd_stop_cont_dtmf_f_type        *cm_mm_inband_cmd_stop_cont_dtmf_func;
  cm_mm_inband_cmd_burst_dtmf_f_type            *cm_mm_inband_cmd_burst_dtmf_func;
  cm_get_user_data_f_type                       *cm_get_user_data_func;

  /* CM sup command functions */
  #ifdef FEATURE_QCRIL_DSDS
  cm_sups_cmd_interrogate_cc_exten_per_subs_f_type    *cm_sups_cmd_interrogate_func;
  cm_sups_cmd_deactivate_cc_exten_per_subs_f_type     *cm_sups_cmd_deactivate_func;
  cm_sups_cmd_activate_cc_exten_per_subs_f_type       *cm_sups_cmd_activate_func;
  cm_sups_cmd_register_cc_exten_per_subs_f_type       *cm_sups_cmd_register_func;
  cm_sups_cmd_erase_cc_exten_per_subs_f_type          *cm_sups_cmd_erase_func;
  cm_sups_cmd_reg_password_cc_exten_per_subs_f_type   *cm_sups_cmd_reg_password_func;
  cm_sups_cmd_process_uss_exten_per_subs_f_type       *cm_sups_cmd_process_uss_func;
  cm_sups_cmd_uss_res_per_subs_f_type           *cm_sups_cmd_uss_res_func;
  cm_sups_cmd_release_cc_per_subs_f_type        *cm_sups_cmd_release_func;
  cm_sups_cmd_uss_notify_res_per_subs_f_type    *cm_sups_cmd_uss_notify_res_func;
  cm_sups_cmd_get_password_res_per_subs_f_type  *cm_sups_cmd_get_password_res_func;
  #else
  cm_sups_cmd_interrogate_f_type                *cm_sups_cmd_interrogate_func;
  cm_sups_cmd_deactivate_f_type                 *cm_sups_cmd_deactivate_func;
  cm_sups_cmd_activate_f_type                   *cm_sups_cmd_activate_func;
  cm_sups_cmd_register_f_type                   *cm_sups_cmd_register_func;
  cm_sups_cmd_erase_f_type                      *cm_sups_cmd_erase_func;
  cm_sups_cmd_reg_password_f_type               *cm_sups_cmd_reg_password_func;
  cm_sups_cmd_process_uss_f_type                *cm_sups_cmd_process_uss_func;
  cm_sups_cmd_uss_res_f_type                    *cm_sups_cmd_uss_res_func;
  cm_sups_cmd_release_f_type                    *cm_sups_cmd_release_func;
  cm_sups_cmd_uss_notify_res_f_type             *cm_sups_cmd_uss_notify_res_func;
  cm_sups_cmd_get_password_res_f_type           *cm_sups_cmd_get_password_res_func;
  #endif /* FEATURE_QCRIL_DSDS */

  #ifdef FEATURE_QCRIL_NCELL
  /* CM statistic functions */
  cm_client_stats_reg_f_type                    *cm_client_stats_reg_func;
  cm_stats_set_modem_info_mask_f_type           *cm_stats_set_modem_info_mask_func;
  #endif /* FEATURE_QCRIL_NCELL */

} qcril_cm_api_funcs_type;


/*===========================================================================

                          EXTERN API MAP 

===========================================================================*/

extern qcril_cm_api_callbacks_type qcril_cm_api_callbacks[ QCRIL_MAX_MODEM_ID ]; 
extern qcril_cm_api_funcs_type     qcril_cm_api_funcs[ QCRIL_MAX_MODEM_ID ];

#endif /* QCRIL_CM_API_MAP_H */

