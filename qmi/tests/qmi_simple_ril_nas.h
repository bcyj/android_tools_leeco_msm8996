/******************************************************************************
  @file    qmi_simple_ril_nas.h
  @brief   Sample simple RIL, NAS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) NW reg

  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_simple_ril_core.h"

#ifndef QMI_SIMPLE_RIL_NAS_H
#define QMI_SIMPLE_RIL_NAS_H

typedef enum
{
    RTE_NONE,
    RTE_FIRST,
    RTE_GSM = RTE_FIRST,
    RTE_WCDMA,
    RTE_LTE,
    RTE_1X,
    RTE_HDR,
    RTE_TDSCDMA,
    RTE_MAX
} qmi_simple_ril_nas_rte;

extern int qmi_simple_ril_show_cdma_ntw_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_show_cdma_ntw_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_perform_network_scan_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_perform_network_scan_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_initiate_network_auto_register_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_initiate_network_manual_register_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_set_3gpp2_subscription_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_nas_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_set_3gpp2_subs_callback (qmi_util_service_message_info* qmi_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_qmi_sar_power_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_nas_unsolicited_sys_info_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_dms_unsolicited_event_report_ind_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_nas_initialize_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_nas_init_resp_handler (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_nas_fetch_sys_info_resp_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_set_mode_pref_callback (qmi_util_service_message_info* qmi_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_get_mode_pref_resp_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_mode_pref_handler (qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern void qmi_simple_ril_show_mode_pref();
extern void qmi_simple_ril_dms_init();


#endif // QMI_SIMPLE_RIL_NAS_H
