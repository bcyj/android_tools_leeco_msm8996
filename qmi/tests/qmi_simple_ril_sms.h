/******************************************************************************
  @file    qmi_simple_ril_sms.h
  @brief   Sample simple RIL, SMS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) SMS subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_simple_ril_core.h"

#ifndef QMI_SIMPLE_RIL_SMS_H
#define QMI_SIMPLE_RIL_SMS_H

extern int qmi_simple_ril_mt_sms_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_mt_sms_req_handler_fota(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_mt_sms_route_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_wms_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_cdma_mo_sms_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_mo_sms_req_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_gsm_mo_sms_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_cdma_activate_broadcast_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_gw_activate_broadcast_req_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);

#endif // QMI_SIMPLE_RIL_SMS_H
