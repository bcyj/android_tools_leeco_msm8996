/*!
  @file
  qcril_qmi_ims_misc.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

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

#ifndef QCRIL_QMI_IMS_MISC_H
#define QCRIL_QMI_IMS_MISC_H


#include "ril.h"
#include "qcrili.h"
#include "qcril_qmi_voice.h"
#include "qcril_qmi_imsa.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_qmi_sms.h"

Ims__MsgId qcril_qmi_ims_map_event_to_request(int event);

qcril_evt_e_type qcril_qmi_ims_map_request_to_event(Ims__MsgId msg_id);

Ims__Error qcril_qmi_ims_map_ril_error_to_ims_error(int ril_error);

Ims__Registration__RegState qcril_qmi_ims_map_ril_reg_state_to_ims_reg_state(int state);

Ims__CallType qcril_qmi_ims_map_ril_call_type_to_ims_call_type(RIL_Call_Type call_type);
RIL_Call_Type qcril_qmi_ims_map_ims_call_type_to_ril_call_type(boolean has_call_type, Ims__CallType call_type);

Ims__CallDomain qcril_qmi_ims_map_ril_call_domain_to_ims_call_domain(RIL_Call_Domain call_domain);
RIL_Call_Domain qcril_qmi_ims_map_ims_call_domain_to_ril_call_domain(boolean has_call_domain, Ims__CallDomain call_domain);

RIL_Token qcril_qmi_ims_convert_ims_token_to_ril_token(uint32_t ims_token);
uint32_t qcril_qmi_ims_free_and_convert_ril_token_to_ims_token(RIL_Token ril_token);

void qcril_qmi_ims_translate_ril_callmodify_to_ims_callmodify(const RIL_Call_Modify* ril_data, Ims__CallModify* ims_data);

void qcril_qmi_ims_translate_ril_calldetails_to_ims_calldetails(const RIL_Call_Details* ril_data, Ims__CallDetails* ims_data);
void qcril_qmi_ims_translate_ims_calldetails_to_ril_calldetails(const Ims__CallDetails *ims_data, RIL_Call_Details* ril_data);

void qcril_qmi_ims_translate_ril_callforwdinfo_to_ims_callforwdinfo
(
 const qcril_qmi_voice_callforwd_info_param_u_type* ril_data, int num,
 voice_time_type_v02 *call_fwd_start_time,
 voice_time_type_v02 *call_fwd_end_time,
 Ims__CallForwardInfoList* ims_data
);

void qcril_qmi_ims_translate_ril_service_status_class_to_ims_callwaitinginfo(int service_status, int service_class, Ims__CallWaitingInfo* ims_data);
Ims__Registration__RegState qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state(uint8_t ims_registered);

void qcril_qmi_ims_translate_ril_suppsvcnotification_to_ims_suppsvcnotification(const RIL_SuppSvcNotification* ril_data, Ims__SuppSvcNotification* ims_data);

void qcril_qmi_ims_translate_ril_callcapabilities_to_ims_srvstatusinfo
(
 const voice_ip_call_capabilities_info_type_v02* ril_data,
 Ims__SrvStatusList* ims_data,
 Ims__CallType current_call_type,
 call_mode_enum_v02 call_mode
);

Ims__Info* qcril_qmi_ims_create_ims_info(
    Ims__CallType type,
    imsa_service_status_enum_v01 status,
    boolean rat_valid,
    imsa_service_rat_enum_v01 rat
);
Ims__SrvStatusList* qcril_qmi_ims_create_ims_srvstatusinfo(const qcril_qmi_imsa_srv_status_type* qmi_data);

void qcril_qmi_ims_free_srvstatuslist(Ims__SrvStatusList* ims_srv_status_list_ptr);
void qcril_qmi_ims_free_ims_info(Ims__Info* ims_info_ptr);

Ims__Handover* qcril_qmi_ims_create_ims_handover_from_imsa_rat_info(const imsa_rat_handover_status_info_v01* qmi_data);
void qcril_qmi_ims_free_ims_handover(Ims__Handover* ims_handover_ptr);

Ims__CallFailCause qcril_qmi_ims_map_ril_failcause_to_ims_failcause(RIL_LastCallFailCause ril_failcause, int ims_extended_error_code);

boolean qcril_qmi_ims_map_qmi_call_state_to_ims_conf_call_state(call_state_enum_v02 qmi_state, Ims__ConfCallState *ims_state_ptr);

Ims__SuppSvcFacilityType qcril_qmi_voice_map_qmi_reason_to_ims_facility
(
  /* Reason code from QMI Voice Get Call Barring response message */
  voice_cc_sups_result_reason_enum_v02 reason
);

Ims__CallSubstate qcril_qmi_ims_map_ril_call_substate_to_ims_call_substate
(
 RIL_Call_Sub_State  ril_call_substate
);

Ims__MwiPriority qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority
(
 wms_mwi_priority_type_enum_v01 wms_mwi_priority
);

Ims__MwiMessageType qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type
(
 transport_mwi_wms_message_type_enum_v01 mwi_wms_msg_type
);

boolean qcril_qmi_ims_translate_ims_ttymodetype_to_qmi_tty_mode
(
 Ims__TtyModeType mode,
 tty_mode_enum_v02 *tty_mode
);

boolean qcril_qmi_ims_translate_ims_callfwdtimerinfo_to_voice_time_type
(
 const Ims__CallFwdTimerInfo *callfwdtimerinfo,
 voice_time_type_v02 *call_fwd_timer
);

boolean qcril_qmi_ims_translate_voice_time_type_to_ims_callfwdtimerinfo
(
 const voice_time_type_v02 *call_fwd_timer,
 Ims__CallFwdTimerInfo *callfwdtimerinfo
);

int32_t qcril_qmi_ims_map_ims_failcause_qmi_reject_cause
(
  Ims__CallFailCause failcause
);

uint8_t qcril_qmi_ims_map_wificallingstatus_to_ims_settings_wfc_status
(
 Ims__WifiCallingStatus            status,
 ims_settings_wfc_status_enum_v01 *wifi_call
);

boolean qcril_qmi_ims_map_ims_settings_wfc_status_to_wificallingstatus
(
 ims_settings_wfc_status_enum_v01 wifi_call,
 Ims__WifiCallingStatus          *status
);

uint8_t qcril_qmi_ims_map_wificallingpreference_to_ims_settings_wfc_preference
(
 Ims__WifiCallingPreference       preference,
 ims_settings_wfc_preference_v01 *wifi_call_preference
);

boolean qcril_qmi_ims_map_ims_settings_wfc_preference_to_wificallingpreference
(
 ims_settings_wfc_preference_v01 wifi_call_preference,
 Ims__WifiCallingPreference     *preference
);

#endif /* QCRIL_QMI_IMS_MISC_H */
