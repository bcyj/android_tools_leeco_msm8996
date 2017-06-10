/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        W I R E L E S S _ L A N _ P R O X Y _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the wlps service Data structures.

  (c) 2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.7
   It was generated on: Tue Jul 29 2014 (Spin 0)
   From IDL File: wireless_lan_proxy_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "wireless_lan_proxy_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t wlps_vdev_info_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_info_s_v01, vdev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_info_s_v01, vdev_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wlps_vdev_conn_info_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, is_connected),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, rssi),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, country_code),
  QMI_WLPS_COUNTRY_CODE_LEN_V01,

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, freq),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, bssid),
  QMI_WLPS_MAX_BSSID_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, ssid),
  QMI_WLPS_MAX_SSID_LEN_V01,
  QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, ssid) - QMI_IDL_OFFSET8(wlps_vdev_conn_info_s_v01, ssid_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t wlps_update_client_version_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_version_req_msg_v01, chip_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_version_req_msg_v01, chip_name),
  QMI_WLPS_MAX_STR_LEN_V01,

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_version_req_msg_v01, chip_from),
  QMI_WLPS_MAX_STR_LEN_V01,

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_version_req_msg_v01, host_version),
  QMI_WLPS_MAX_STR_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_version_req_msg_v01, fw_version),
  QMI_WLPS_MAX_STR_LEN_V01
};

static const uint8_t wlps_update_client_version_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wlps_update_client_version_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wlps_update_client_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, fw_adsp_support),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, is_on),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, vdev_info) - QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, vdev_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, vdev_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, vdev_conn_info) - QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, vdev_conn_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, vdev_conn_info),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, channel_info) - QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, channel_info_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, channel_info),
  QMI_WLPS_MAX_NUM_CHAN_V01,
  QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, channel_info) - QMI_IDL_OFFSET8(wlps_update_client_status_req_msg_v01, channel_info_len)
};

static const uint8_t wlps_update_client_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wlps_update_client_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  wlps_type_table_v01[] = {
  {sizeof(wlps_vdev_info_s_v01), wlps_vdev_info_s_data_v01},
  {sizeof(wlps_vdev_conn_info_s_v01), wlps_vdev_conn_info_s_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry wlps_message_table_v01[] = {
  {sizeof(wlps_update_client_version_req_msg_v01), wlps_update_client_version_req_msg_data_v01},
  {sizeof(wlps_update_client_version_resp_msg_v01), wlps_update_client_version_resp_msg_data_v01},
  {sizeof(wlps_update_client_status_req_msg_v01), wlps_update_client_status_req_msg_data_v01},
  {sizeof(wlps_update_client_status_resp_msg_v01), wlps_update_client_status_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object wlps_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *wlps_qmi_idl_type_table_object_referenced_tables_v01[] =
{&wlps_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object wlps_qmi_idl_type_table_object_v01 = {
  sizeof(wlps_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(wlps_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  wlps_type_table_v01,
  wlps_message_table_v01,
  wlps_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry wlps_service_command_messages_v01[] = {
  {QMI_WLPS_UPDATE_CLIENT_VERSION_REQ_V01, QMI_IDL_TYPE16(0, 0), 83},
  {QMI_WLPS_UPDATE_CLIENT_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 2), 199}
};

static const qmi_idl_service_message_table_entry wlps_service_response_messages_v01[] = {
  {QMI_WLPS_UPDATE_CLIENT_VERSION_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_WLPS_UPDATE_CLIENT_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 3), 7}
};

/*Service Object*/
struct qmi_idl_service_object wlps_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x0039,
  199,
  { sizeof(wlps_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(wlps_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { wlps_service_command_messages_v01, wlps_service_response_messages_v01, NULL},
  &wlps_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type wlps_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( WLPS_V01_IDL_MAJOR_VERS != idl_maj_version || WLPS_V01_IDL_MINOR_VERS != idl_min_version
       || WLPS_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&wlps_qmi_idl_service_object_v01;
}

