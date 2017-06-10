/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q T U N E R _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the Qtuner service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5
   It was generated on: Fri Sep 28 2012
   From IDL File: qtuner_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qtuner_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t Qtuner_set_scenario_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(Qtuner_set_scenario_req_v01, scenarios),
  Qtuner_CONCURRENT_SCENARIOS_MAX_V01,
  QMI_IDL_OFFSET8(Qtuner_set_scenario_req_v01, scenarios) - QMI_IDL_OFFSET8(Qtuner_set_scenario_req_v01, scenarios_len)
};

/*
 * Qtuner_set_scenario_resp is empty
 * static const uint8_t Qtuner_set_scenario_resp_data_v01[] = {
 * };
 */

/*
 * Qtuner_get_rfm_scenarios_req is empty
 * static const uint8_t Qtuner_get_rfm_scenarios_req_data_v01[] = {
 * };
 */

static const uint8_t Qtuner_get_rfm_scenarios_resp_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(Qtuner_get_rfm_scenarios_resp_v01, active_scenarios) - QMI_IDL_OFFSET8(Qtuner_get_rfm_scenarios_resp_v01, active_scenarios_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(Qtuner_get_rfm_scenarios_resp_v01, active_scenarios),
  Qtuner_CONCURRENT_SCENARIOS_MAX_V01,
  QMI_IDL_OFFSET8(Qtuner_get_rfm_scenarios_resp_v01, active_scenarios) - QMI_IDL_OFFSET8(Qtuner_get_rfm_scenarios_resp_v01, active_scenarios_len)
};

/*
 * Qtuner_get_provisioned_table_revision_req is empty
 * static const uint8_t Qtuner_get_provisioned_table_revision_req_data_v01[] = {
 * };
 */

static const uint8_t Qtuner_get_provisioned_table_revision_resp_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_revision) - QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_revision_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_revision),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_OEM) - QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_OEM_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_OEM),
  Qtuner_OEM_STR_LENGTH_V01,
  QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_OEM) - QMI_IDL_OFFSET8(Qtuner_get_provisioned_table_revision_resp_v01, provisioned_table_OEM_len)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry Qtuner_message_table_v01[] = {
  {sizeof(Qtuner_set_scenario_req_v01), Qtuner_set_scenario_req_data_v01},
  {0, 0},
  {0, 0},
  {sizeof(Qtuner_get_rfm_scenarios_resp_v01), Qtuner_get_rfm_scenarios_resp_data_v01},
  {0, 0},
  {sizeof(Qtuner_get_provisioned_table_revision_resp_v01), Qtuner_get_provisioned_table_revision_resp_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object Qtuner_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *Qtuner_qmi_idl_type_table_object_referenced_tables_v01[] =
{&Qtuner_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object Qtuner_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(Qtuner_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  Qtuner_message_table_v01,
  Qtuner_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry Qtuner_service_command_messages_v01[] = {
  {QMI_Qtuner_SET_RFM_SCENARIO_REQ_V01, TYPE16(0, 0), 132},
  {QMI_Qtuner_GET_RFM_SCENARIO_REQ_V01, TYPE16(0, 2), 0},
  {QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_REQ_V01, TYPE16(0, 4), 0}
};

static const qmi_idl_service_message_table_entry Qtuner_service_response_messages_v01[] = {
  {QMI_Qtuner_SET_RFM_SCENARIO_RESP_V01, TYPE16(0, 1), 0},
  {QMI_Qtuner_GET_RFM_SCENARIO_RESP_V01, TYPE16(0, 3), 132},
  {QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_RESP_V01, TYPE16(0, 5), 139}
};

/*Service Object*/
struct qmi_idl_service_object Qtuner_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x04,
  139,
  { sizeof(Qtuner_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(Qtuner_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { Qtuner_service_command_messages_v01, Qtuner_service_response_messages_v01, NULL},
  &Qtuner_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type Qtuner_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( QTUNER_V01_IDL_MAJOR_VERS != idl_maj_version || QTUNER_V01_IDL_MINOR_VERS != idl_min_version
       || QTUNER_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&Qtuner_qmi_idl_service_object_v01;
}

