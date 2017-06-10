/******************************************************************************
  @file    fota.h
  @brief   Sample simple RIL, WAP SMS sub component

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) WAP SMS subsystem

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef FOTA_H
#define FOTA_H

#include "comdef.h"                         //boolean
#include "wms.h"                            //wms_address_s_type
#include "wmsts.h"                          //wms_gw_user_data_s_type
#include "wireless_messaging_service_v01.h" //wms_event_report_ind_msg_v01

#include "qmi_simple_ril_core.h"            //qmi_simple_ril_cmd_completion_info

void fota_log_append(size_t* pcur, char* str, size_t size, const char * format, ... );

void print_wms_address(const wms_address_s_type* address, boolean cdma);
void print_wms_subaddress(const wms_subaddress_s_type* subaddress);
void print_wms_gw_dcs(const wms_gw_dcs_s_type* dcs);
void print_wms_timestamp(const wms_timestamp_s_type* timestamp);
void print_wms_udh(const wms_udh_s_type* udh);
void print_wms_user_data(const wms_gw_user_data_s_type* user_data, const wms_gw_dcs_s_type *dcs);
void print_wms_gw_deliver(const wms_gw_deliver_s_type* deliver);


void print_wms_cdma_user_data(const wms_cdma_user_data_s_type* user_data);
void print_wms_message_id(const wms_message_id_s_type* message_id);
void print_wms_reply_option(const wms_reply_option_s_type* reply_option);
void print_wms_client_bd(const wms_client_bd_s_type* bd);
void print_wms_cdma_message(const wms_cdma_message_s_type* cdma_message);

void print_wms_raw_uint8(const uint8* data, uint32 data_len);

void wms_process_mt_gw_sms
(
  wms_event_report_ind_msg_v01 * event_report_ind
);
void wms_process_mt_cdma_sms
(
  wms_event_report_ind_msg_v01 * event_report_ind
);

void wms_print_sms_ind_fota
(
  wms_event_report_ind_msg_v01* event_report_ind,
  qmi_simple_ril_cmd_completion_info* uplink_message
);

void set_fota_cmd_enabled(int enable);
int is_fota_cmd_enabled();

boolean fota_check_cdma_wap_push_message(const wms_cdma_message_s_type *cdma_message);
boolean fota_check_gw_wap_push_message(const wms_gw_deliver_s_type *gw_deliver);
void    fota_send_raw_data(char const * const data, int nof_bytes);

#endif // FOTA_H
