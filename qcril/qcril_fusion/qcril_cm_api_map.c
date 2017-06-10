/*!
  @file
  qcril_cm_api_map.c

  @brief
  Handles the mapping to modems' (CM) APIs.

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

#include "qcril_cm_api_map.h"


/*===========================================================================

                       EXTERN DEFINITIONS 

===========================================================================*/

extern void qcril_cm_phonesvc_event_callback( cm_ph_event_e_type ph_event, const cm_ph_info_s_type *ph_info_ptr );
extern void qcril_cm_srvsys_event_callback( cm_ss_event_e_type ss_event, const cm_mm_ss_info_s_type *ss_info_ptr );
extern void qcril_cm_callsvc_event_callback( cm_call_event_e_type call_event, const cm_mm_call_info_s_type *call_info_ptr );
extern void qcril_cm_inband_event_callback( cm_inband_event_e_type inband_event, const cm_inband_info_s_type *inband_info_ptr );
extern void qcril_cm_supsvc_event_callback( cm_sups_event_e_type sups_event, const cm_sups_info_s_type *sups_info_ptr );

#ifdef FEATURE_QCRIL_NCELL
extern void qcril_cm_stats_event_callback( cm_stats_event_e_type stats_event, const sys_modem_stats_info_s_type *stats_info_ptr );
#endif /* FEATURE_QCRIL_NCELL */

#ifdef FEATURE_QCRIL_DSDS
void qcril_cm_subs_event_callback( cm_ph_event_e_type subs_event, const cm_subs_info_s_type *subs_info_ptr );
#endif /* FEATURE_QCRIL_DSDS */

extern void qcril_cm_phonesvc_event_callback_fusion( cm_ph_event_e_type ph_event, const cm_ph_info_s_type *ph_info_ptr );
extern void qcril_cm_srvsys_event_callback_fusion( cm_ss_event_e_type ss_event, const cm_mm_ss_info_s_type *ss_info_ptr );
extern void qcril_cm_callsvc_event_callback_fusion( cm_call_event_e_type call_event, const cm_mm_call_info_s_type *call_info_ptr );
extern void qcril_cm_inband_event_callback_fusion( cm_inband_event_e_type inband_event, const cm_inband_info_s_type *inband_info_ptr );
extern void qcril_cm_supsvc_event_callback_fusion( cm_sups_event_e_type sups_event, const cm_sups_info_s_type *sups_info_ptr );

#ifdef FEATURE_QCRIL_NCELL
extern void qcril_cm_stats_event_callback_fusion( cm_stats_event_e_type stats_event, const sys_modem_stats_info_s_type *stats_info_ptr );
#endif /* FEATURE_QCRIL_NCELL */


/*===========================================================================

                               API MAP

===========================================================================*/

qcril_cm_api_callbacks_type qcril_cm_api_callbacks[ QCRIL_MAX_MODEM_ID ] =
{
  {
    qcril_cm_phonesvc_event_callback,
    qcril_cm_srvsys_event_callback,
    qcril_cm_callsvc_event_callback,
    qcril_cm_inband_event_callback,
    qcril_cm_supsvc_event_callback

    #ifdef FEATURE_QCRIL_NCELL
    , qcril_cm_stats_event_callback
    #endif /* FEATURE_QCRIL_NCELL */

    #ifdef FEATURE_QCRIL_DSDS
    , qcril_cm_subs_event_callback
    #endif /* FEATURE_QCRIL_DSDS */
  }
  #ifdef FEATURE_QCRIL_FUSION
  ,
  {
    qcril_cm_phonesvc_event_callback_fusion,
    qcril_cm_srvsys_event_callback_fusion,
    qcril_cm_callsvc_event_callback_fusion,
    qcril_cm_inband_event_callback_fusion,
    qcril_cm_supsvc_event_callback_fusion

    #ifdef FEATURE_QCRIL_NCELL
    , qcril_cm_stats_event_callback_fusion
    #endif /* FEATURE_QCRIL_NCELL */

    #ifdef FEATURE_QCRIL_DSDS
    , qcril_cm_subs_event_callback
    #endif /* FEATURE_QCRIL_DSDS */
  }
  #endif /* FEATURE_QCRIL_FUSION */
};

qcril_cm_api_funcs_type qcril_cm_api_funcs[ QCRIL_MAX_MODEM_ID ] =
{
  {
    cm_client_act,
    cm_client_init,
    cm_client_release,

    /* CM event handler registration functions */
    cm_client_ph_reg,
    cm_mm_client_ss_reg,
    cm_mm_client_call_reg,
    cm_mm_client_inband_reg,
    cm_client_sups_reg,

    #ifdef FEATURE_QCRIL_DSDS
    cm_client_subs_reg,
    #endif /* FEATURE_QCRIL_DSDS */

    /* CM ph command functions */
    cm_ph_cmd_oprt_mode,
    cm_ph_cmd_change_rtre_config,
    cm_ph_cmd_signal_strength_delta,

    #ifdef FEATURE_QCRIL_SUBS_CTRL
    cm_ph_cmd_subscription_available,
    cm_ph_cmd_subscription_not_available,
    #endif /* FEATURE_QCRIL_SUBS_CTRL */

    #ifdef FEATURE_QCRIL_DSDS
    cm_ph_cmd_sys_sel_pref_per_subs,
    cm_ph_cmd_get_ph_info_per_subs,
    cm_ph_cmd_get_networks_per_subs,
    cm_ph_cmd_wakeup_from_standby_per_subs,
    cm_ph_cmd_dual_standby_pref,
    cm_ph_cmd_get_subs_pref_info,
    #else
    cm_ph_cmd_sys_sel_pref,
    cm_ph_cmd_get_ph_info,
    cm_ph_cmd_get_networks,
    cm_ph_cmd_wakeup_from_standby,
    #endif /* FEATURE_QCRIL_DSDS */

    /* CM ss command functions */
    cm_ss_cmd_get_ss_info,

    /* CM util functions */
    cm_num_init,                           
    cm_num_fill,                           
    cm_util_default_gw_orig_params,        
    cm_util_set_default_voice_bearer_cap,  
    cm_util_default_cdma_orig_params,      

    /* CM call command functions */
    #ifdef FEATURE_QCRIL_DSDS
    cm_mm_call_cmd_orig_per_subs,
    cm_mm_call_cmd_sups_per_subs,
    #elif defined( FEATURE_QCRIL_CALL_ORIG_EXTEN )
    cm_mm_call_cmd_orig_exten2,
    cm_mm_call_cmd_sups,
    #elif defined( FEATURE_QCRIL_CALL_ORIG_EXTEN3 )
    cm_mm_call_cmd_orig_exten3,
    cm_mm_call_cmd_sups,
    #else
    cm_mm_call_cmd_orig,
    cm_mm_call_cmd_sups,
    #endif

    cm_mm_call_cmd_answer,
    cm_mm_call_cmd_end,
    cm_mm_call_cmd_privacy_pref,
    cm_mm_call_cmd_setup_res,

    /* CM inband command functions */
    cm_mm_inband_cmd_start_cont_dtmf,
    cm_mm_inband_cmd_stop_cont_dtmf,
    cm_mm_inband_cmd_burst_dtmf,
    cm_get_user_data,

    /* CM sup command functions */
    #ifdef FEATURE_QCRIL_DSDS
    cm_sups_cmd_interrogate_cc_exten_per_subs,
    cm_sups_cmd_deactivate_cc_exten_per_subs,
    cm_sups_cmd_activate_cc_exten_per_subs,
    cm_sups_cmd_register_cc_exten_per_subs,
    cm_sups_cmd_erase_cc_exten_per_subs,
    cm_sups_cmd_reg_password_cc_exten_per_subs,
    cm_sups_cmd_process_uss_exten_per_subs,
    cm_sups_cmd_uss_res_per_subs,
    cm_sups_cmd_release_cc_per_subs,
    cm_sups_cmd_uss_notify_res_per_subs,
    cm_sups_cmd_get_password_res_per_subs
    #elif defined( FEATURE_QCRIL_SUPS_CC_UNSUPP )
    cm_sups_cmd_interrogate,
    cm_sups_cmd_deactivate,
    cm_sups_cmd_activate,
    cm_sups_cmd_register,
    cm_sups_cmd_erase,
    cm_sups_cmd_reg_password,
    cm_sups_cmd_process_uss,
    cm_sups_cmd_uss_res,
    cm_sups_cmd_release,
    cm_sups_cmd_uss_notify_res,
    cm_sups_cmd_get_password_res
    #elif defined( FEATURE_QCRIL_SUPS_CC_EXTEN )
    cm_sups_cmd_interrogate_exten,
    cm_sups_cmd_deactivate_exten,
    cm_sups_cmd_activate_exten,
    cm_sups_cmd_register_exten,
    cm_sups_cmd_erase_exten,
    cm_sups_cmd_reg_password_exten,
    cm_sups_cmd_process_uss_exten,
    cm_sups_cmd_uss_res,
    cm_sups_cmd_release,
    cm_sups_cmd_uss_notify_res,
    cm_sups_cmd_get_password_res
    #else
    cm_sups_cmd_interrogate_cc,
    cm_sups_cmd_deactivate_cc,
    cm_sups_cmd_activate_cc,
    cm_sups_cmd_register_cc,
    cm_sups_cmd_erase_cc,
    cm_sups_cmd_reg_password_cc,
    cm_sups_cmd_process_uss_cc,
    cm_sups_cmd_uss_res,
    cm_sups_cmd_release,
    cm_sups_cmd_uss_notify_res,
    cm_sups_cmd_get_password_res
    #endif

    #ifdef FEATURE_QCRIL_NCELL
    /* CM statistics functions */
    , cm_client_stats_reg
    , cm_stats_set_modem_info_mask
    #endif /* FEATURE_QCRIL_NCELL */
  }
  #if !defined(FEATURE_QCRIL_DSDS) && defined(FEATURE_QCRIL_FUSION)
  ,
  {
    cm_client_act_fusion,
    cm_client_init_fusion,
    cm_client_release_fusion,

    /* CM event handler registration functions */
    cm_client_ph_reg_fusion,
    cm_mm_client_ss_reg_fusion,
    cm_mm_client_call_reg_fusion,
    cm_mm_client_inband_reg_fusion,
    cm_client_sups_reg_fusion,

    /* CM ph command functions */
    cm_ph_cmd_oprt_mode_fusion,
    cm_ph_cmd_change_rtre_config_fusion,
    cm_ph_cmd_signal_strength_delta_fusion,
    cm_ph_cmd_sys_sel_pref_fusion,
    cm_ph_cmd_get_ph_info_fusion,
    cm_ph_cmd_get_networks_fusion,
    cm_ph_cmd_wakeup_from_standby_fusion,

    /* CM ss command functions */
    cm_ss_cmd_get_ss_info_fusion,

    /* CM util functions */
    cm_num_init_fusion,                           
    cm_num_fill_fusion,                           
    cm_util_default_gw_orig_params_fusion,        
    cm_util_set_default_voice_bearer_cap_fusion,  
    cm_util_default_cdma_orig_params_fusion,      

    /* CM call command functions */
    #ifdef FEATURE_QCRIL_CALL_ORIG_EXTEN
    cm_mm_call_cmd_orig_exten2_fusion,
    #elif defined( FEATURE_QCRIL_CALL_ORIG_EXTEN3 )
    cm_mm_call_cmd_orig_exten3_fusion,
    #else
    cm_mm_call_cmd_orig_fusion,
    #endif
    cm_mm_call_cmd_sups_fusion,
    cm_mm_call_cmd_answer_fusion,
    cm_mm_call_cmd_end_fusion,
    cm_mm_call_cmd_privacy_pref_fusion,
    cm_mm_call_cmd_setup_res_fusion,

    /* CM inband command functions */
    cm_mm_inband_cmd_start_cont_dtmf_fusion,
    cm_mm_inband_cmd_stop_cont_dtmf_fusion,
    cm_mm_inband_cmd_burst_dtmf_fusion,
    cm_get_user_data_fusion,

    /* CM sup command functions */
    #ifdef FEATURE_QCRIL_SUPS_CC_UNSUPP
    cm_sups_cmd_interrogate_fusion,
    cm_sups_cmd_deactivate_fusion,
    cm_sups_cmd_activate_fusion,
    cm_sups_cmd_register_fusion,
    cm_sups_cmd_erase_fusion,
    cm_sups_cmd_reg_password_fusion,
    cm_sups_cmd_process_uss_fusion,
    #elif defined( FEATURE_QCRIL_SUPS_CC_EXTEN )
    cm_sups_cmd_interrogate_exten_fusion,
    cm_sups_cmd_deactivate_exten_fusion,
    cm_sups_cmd_activate_exten_fusion,
    cm_sups_cmd_register_exten_fusion,
    cm_sups_cmd_erase_exten_fusion,
    cm_sups_cmd_reg_password_exten_fusion,
    cm_sups_cmd_process_uss_exten_fusion,
    #else
    cm_sups_cmd_interrogate_cc_fusion,
    cm_sups_cmd_deactivate_cc_fusion,
    cm_sups_cmd_activate_cc_fusion,
    cm_sups_cmd_register_cc_fusion,
    cm_sups_cmd_erase_cc_fusion,
    cm_sups_cmd_reg_password_cc_fusion,
    cm_sups_cmd_process_uss_cc_fusion,
    #endif
    cm_sups_cmd_uss_res_fusion,
    cm_sups_cmd_release_fusion,
    cm_sups_cmd_uss_notify_res_fusion,
    cm_sups_cmd_get_password_res_fusion

    #ifdef FEATURE_QCRIL_NCELL
    /* CM statistics functions */
    , cm_client_stats_reg_fusion
    , cm_stats_set_modem_info_mask_fusion
    #endif /* FEATURE_QCRIL_NCELL */
  }
  #endif /* !FEATURE_QCRIL_DSDS && FEATURE_QCRIL_FUSION */
};
