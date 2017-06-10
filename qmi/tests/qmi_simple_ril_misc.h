/******************************************************************************
  @file    qmi_simple_ril_misc.h
  @brief   Sample simple RIL, DMS&PB&UIM sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) DMS&PB&UIM subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_simple_ril_core.h"

#ifndef QMI_SIMPLE_RIL_MISC_H
#define QMI_SIMPLE_RIL_MISC_H

extern int qmi_simple_ril_misc_dms_modem_mode_command_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_dms_modem_mode_qmi_message_handler (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_dms_get_dev_id_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_dms_get_device_id_message_handler(qmi_util_service_message_info* qmi_msg_info, qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_uim_get_imsi_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_uim_read_transp_message_handler(qmi_util_service_message_info* qmi_msg_info, qmi_simple_ril_cmd_completion_info* uplink_message);
extern char * qmi_simple_ril_misc_uim_parse_gw_imsi (const unsigned char * raw_imsi_ptr, unsigned short raw_imsi_len, int * parsed_imsi_len_ptr);
extern char * qmi_simple_ril_misc_uim_parse_1x_imsi (const unsigned char * raw_imsi_ptr, unsigned short raw_imsi_len, int * parsed_imsi_len_ptr);
extern void qmi_simple_ril_misc_uim_flip_data (uint8_t * des, const uint8_t * src, uint32_t data_len);
char qmi_simple_ril_misc_uim_bin_to_hexchar( uint8_t ch);
extern int qmi_simple_ril_misc_uim_card_status_qmi_message_handler (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_uim_set_srv_status_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_uim_get_fdn_status_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_uim_verify_pin_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                          qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_uim_send_apdu_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_uim_logical_channel_callback (qmi_util_service_message_info* qmi_msg_info, 
                                                qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_card_command_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_set_srv_status_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_get_fdn_status_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_verify_pin_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_send_apdu_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_logical_channel_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_indication_register_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_indication_register_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_pb_record_update_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_pb_ready_unsolicited_qmi_message_handler (qmi_util_service_message_info* unsolicited_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);
extern int qmi_simple_ril_misc_common_qmi_svc_versions_command_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_misc_ping_handler(qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);
extern int qmi_simple_ril_ping_misc_callback (qmi_util_service_message_info* callback_msg_info,qmi_simple_ril_cmd_completion_info* uplink_message);

#endif // QMI_SIMPLE_RIL_MISC_H
