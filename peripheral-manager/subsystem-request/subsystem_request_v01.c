/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S U B S Y S T E M _ R E Q U E S T _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the ssreq service Data structures.

  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.10
   It was generated on: Wed Jun 11 2014 (Spin 0)
   From IDL File: subsystem_request_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "subsystem_request_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t qmi_ssreq_system_shutdown_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_ssreq_system_shutdown_req_msg_v01, ss_client_id)
};

static const uint8_t qmi_ssreq_system_shutdown_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_ssreq_system_shutdown_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t qmi_ssreq_system_shutdown_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_ssreq_system_shutdown_ind_msg_v01, status)
};

static const uint8_t qmi_ssreq_system_restart_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_ssreq_system_restart_req_msg_v01, ss_client_id)
};

static const uint8_t qmi_ssreq_system_restart_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_ssreq_system_restart_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t qmi_ssreq_system_restart_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_ssreq_system_restart_ind_msg_v01, status)
};

static const uint8_t qmi_ssreq_peripheral_restart_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_ssreq_peripheral_restart_req_msg_v01, ss_client_id)
};

static const uint8_t qmi_ssreq_peripheral_restart_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_ssreq_peripheral_restart_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t qmi_ssreq_peripheral_restart_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_ssreq_peripheral_restart_ind_msg_v01, status)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry ssreq_message_table_v01[] = {
  {sizeof(qmi_ssreq_system_shutdown_req_msg_v01), qmi_ssreq_system_shutdown_req_msg_data_v01},
  {sizeof(qmi_ssreq_system_shutdown_resp_msg_v01), qmi_ssreq_system_shutdown_resp_msg_data_v01},
  {sizeof(qmi_ssreq_system_shutdown_ind_msg_v01), qmi_ssreq_system_shutdown_ind_msg_data_v01},
  {sizeof(qmi_ssreq_system_restart_req_msg_v01), qmi_ssreq_system_restart_req_msg_data_v01},
  {sizeof(qmi_ssreq_system_restart_resp_msg_v01), qmi_ssreq_system_restart_resp_msg_data_v01},
  {sizeof(qmi_ssreq_system_restart_ind_msg_v01), qmi_ssreq_system_restart_ind_msg_data_v01},
  {sizeof(qmi_ssreq_peripheral_restart_req_msg_v01), qmi_ssreq_peripheral_restart_req_msg_data_v01},
  {sizeof(qmi_ssreq_peripheral_restart_resp_msg_v01), qmi_ssreq_peripheral_restart_resp_msg_data_v01},
  {sizeof(qmi_ssreq_peripheral_restart_ind_msg_v01), qmi_ssreq_peripheral_restart_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object ssreq_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *ssreq_qmi_idl_type_table_object_referenced_tables_v01[] =
{&ssreq_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object ssreq_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(ssreq_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  ssreq_message_table_v01,
  ssreq_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry ssreq_service_command_messages_v01[] = {
  {QMI_SSREQ_SYSTEM_SHUTDOWN_REQ_V01, QMI_IDL_TYPE16(0, 0), 7},
  {QMI_SSREQ_SYSTEM_RESTART_REQ_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_SSREQ_PERIPHERAL_RESTART_REQ_V01, QMI_IDL_TYPE16(0, 6), 7}
};

static const qmi_idl_service_message_table_entry ssreq_service_response_messages_v01[] = {
  {QMI_SSREQ_SYSTEM_SHUTDOWN_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_SSREQ_SYSTEM_RESTART_RESP_V01, QMI_IDL_TYPE16(0, 4), 7},
  {QMI_SSREQ_PERIPHERAL_RESTART_RESP_V01, QMI_IDL_TYPE16(0, 7), 7}
};

static const qmi_idl_service_message_table_entry ssreq_service_indication_messages_v01[] = {
  {QMI_SSREQ_SYSTEM_SHUTDOWN_IND_V01, QMI_IDL_TYPE16(0, 2), 7},
  {QMI_SSREQ_SYSTEM_RESTART_IND_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_SSREQ_PERIPHERAL_RESTART_IND_V01, QMI_IDL_TYPE16(0, 8), 7}
};

/*Service Object*/
struct qmi_idl_service_object ssreq_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x35,
  7,
  { sizeof(ssreq_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ssreq_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ssreq_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { ssreq_service_command_messages_v01, ssreq_service_response_messages_v01, ssreq_service_indication_messages_v01},
  &ssreq_qmi_idl_type_table_object_v01,
  0x01,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type ssreq_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SSREQ_V01_IDL_MAJOR_VERS != idl_maj_version || SSREQ_V01_IDL_MINOR_VERS != idl_min_version
       || SSREQ_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&ssreq_qmi_idl_service_object_v01;
}
