/******************************************************************************
  @file    qmi_simple_ril_ss.h
  @brief   Sample simple RIL, SS/USSD sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) SS/USSD

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_simple_ril_core.h"

#ifndef QMI_SIMPLE_RIL_SS_H
#define QMI_SIMPLE_RIL_SS_H

extern int qmi_simple_ril_show_call_waiting_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_show_call_waiting_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_show_call_barring_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_show_call_barring_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_show_call_forwarding_info_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_show_call_forwarding_info_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_ss_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);


#endif // QMI_SIMPLE_RIL_SS_H
