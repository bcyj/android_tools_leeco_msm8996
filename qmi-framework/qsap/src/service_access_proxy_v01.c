/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S E R V I C E _ A C C E S S _ P R O X Y _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sap service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/sap/main/latest/src/service_access_proxy_v01.c#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 4.3
   It was generated on: Fri Feb 10 2012
   From IDL File: service_access_proxy_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "service_access_proxy_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t sap_service_obj_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sap_service_obj_type_v01, service_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sap_service_obj_type_v01, major_vers),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sap_service_obj_type_v01, max_msg_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t sap_register_service_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sap_register_service_req_msg_v01, service_obj),
  0, 0
};

static const uint8_t sap_register_service_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sap_register_service_resp_msg_v01, resp),
  0, 1
};

static const uint8_t sap_deregister_service_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sap_deregister_service_req_msg_v01, service_obj),
  0, 0
};

static const uint8_t sap_deregister_service_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sap_deregister_service_resp_msg_v01, resp),
  0, 1
};

static const uint8_t sap_client_connect_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sap_client_connect_ind_msg_v01, service_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sap_client_connect_ind_msg_v01, client_id)
};

static const uint8_t sap_client_disconnect_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sap_client_disconnect_ind_msg_v01, service_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sap_client_disconnect_ind_msg_v01, client_id)
};

/* Type Table */
static const qmi_idl_type_table_entry  sap_type_table_v01[] = {
  {sizeof(sap_service_obj_type_v01), sap_service_obj_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry sap_message_table_v01[] = {
  {sizeof(sap_register_service_req_msg_v01), sap_register_service_req_msg_data_v01},
  {sizeof(sap_register_service_resp_msg_v01), sap_register_service_resp_msg_data_v01},
  {sizeof(sap_deregister_service_req_msg_v01), sap_deregister_service_req_msg_data_v01},
  {sizeof(sap_deregister_service_resp_msg_v01), sap_deregister_service_resp_msg_data_v01},
  {sizeof(sap_client_connect_ind_msg_v01), sap_client_connect_ind_msg_data_v01},
  {sizeof(sap_client_disconnect_ind_msg_v01), sap_client_disconnect_ind_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object sap_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sap_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sap_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object sap_qmi_idl_type_table_object_v01 = {
  sizeof(sap_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(sap_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  sap_type_table_v01,
  sap_message_table_v01,
  sap_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry sap_service_command_messages_v01[] = {
  {QMI_SAP_REGISTER_SERVICE_REQ_V01, TYPE16(0, 0), 15},
  {QMI_SAP_DEREGISTER_SERVICE_REQ_V01, TYPE16(0, 2), 15}
};

static const qmi_idl_service_message_table_entry sap_service_response_messages_v01[] = {
  {QMI_SAP_REGISTER_SERVICE_RESP_V01, TYPE16(0, 1), 7},
  {QMI_SAP_DEREGISTER_SERVICE_RESP_V01, TYPE16(0, 3), 7}
};

static const qmi_idl_service_message_table_entry sap_service_indication_messages_v01[] = {
  {QMI_SAP_CLIENT_CONNECT_IND_V01, TYPE16(0, 4), 11},
  {QMI_SAP_CLIENT_DISCONNECT_IND_V01, TYPE16(0, 5), 11}
};

/*Service Object*/
const struct qmi_idl_service_object sap_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  0x19,
  15,
  { sizeof(sap_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(sap_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(sap_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { sap_service_command_messages_v01, sap_service_response_messages_v01, sap_service_indication_messages_v01},
  &sap_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type sap_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SAP_V01_IDL_MAJOR_VERS != idl_maj_version || SAP_V01_IDL_MINOR_VERS != idl_min_version
       || SAP_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&sap_qmi_idl_service_object_v01;
}

