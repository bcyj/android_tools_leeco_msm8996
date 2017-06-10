/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S A R _ V S _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the svs service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 02.04 
   It was generated on: Thu Mar 24 2011
   From IDL File: sar_vs_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sar_vs_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sar_vs_rf_sar_state_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(sar_vs_rf_sar_state_req_msg_v01, sar_rf_state)
};

static const uint8_t sar_vs_rf_sar_state_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_vs_rf_sar_state_resp_msg_v01, resp),
  0, 1
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry svs_message_table_v01[] = {
  {sizeof(sar_vs_rf_sar_state_req_msg_v01), sar_vs_rf_sar_state_req_msg_data_v01},
  {sizeof(sar_vs_rf_sar_state_resp_msg_v01), sar_vs_rf_sar_state_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object svs_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *svs_qmi_idl_type_table_object_referenced_tables_v01[] =
{&svs_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object svs_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(svs_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  svs_message_table_v01,
  svs_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry svs_service_command_messages_v01[] = {
  {QMI_SAR_VS_RF_SAR_STATE_REQ_MSG_V01, TYPE16(0, 0), 4}
};

static const qmi_idl_service_message_table_entry svs_service_response_messages_v01[] = {
  {QMI_SAR_VS_RF_SAR_STATE_RESP_MSG_V01, TYPE16(0, 1), 7}
};

/*Service Object*/
const struct qmi_idl_service_object svs_qmi_idl_service_object_v01 = {
  02,
  01,
  0xE3,
  7,
  { sizeof(svs_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(svs_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { svs_service_command_messages_v01, svs_service_response_messages_v01, NULL},
  &svs_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type svs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SVS_V01_IDL_MAJOR_VERS != idl_maj_version || SVS_V01_IDL_MINOR_VERS != idl_min_version 
       || SVS_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&svs_qmi_idl_service_object_v01;
}

