/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        R A D I O _ F R E Q U E N C Y _ R A D I A T E D _ P E R F O R M A N C E _ E N H A N C E M E N T _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the rfrpe service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/rfrpe/main/latest/src/radio_frequency_radiated_performance_enhancement_v01.c#1 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5 
   It was generated on: Fri Aug 31 2012
   From IDL File: radio_frequency_radiated_performance_enhancement_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "radio_frequency_radiated_performance_enhancement_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t rfrpe_set_scenario_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfrpe_set_scenario_req_v01, scenarios),
  RFRPE_CONCURRENT_SCENARIOS_MAX_V01,
  QMI_IDL_OFFSET8(rfrpe_set_scenario_req_v01, scenarios) - QMI_IDL_OFFSET8(rfrpe_set_scenario_req_v01, scenarios_len)
};

static const uint8_t rfrpe_set_scenario_resp_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfrpe_set_scenario_resp_v01, resp),
  0, 1
};

/* 
 * rfrpe_get_rfm_scenarios_req is empty
 * static const uint8_t rfrpe_get_rfm_scenarios_req_data_v01[] = {
 * };
 */
  
static const uint8_t rfrpe_get_rfm_scenarios_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfrpe_get_rfm_scenarios_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rfrpe_get_rfm_scenarios_resp_v01, active_scenarios) - QMI_IDL_OFFSET8(rfrpe_get_rfm_scenarios_resp_v01, active_scenarios_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfrpe_get_rfm_scenarios_resp_v01, active_scenarios),
  RFRPE_CONCURRENT_SCENARIOS_MAX_V01,
  QMI_IDL_OFFSET8(rfrpe_get_rfm_scenarios_resp_v01, active_scenarios) - QMI_IDL_OFFSET8(rfrpe_get_rfm_scenarios_resp_v01, active_scenarios_len)
};

/* 
 * rfrpe_get_provisioned_table_revision_req is empty
 * static const uint8_t rfrpe_get_provisioned_table_revision_req_data_v01[] = {
 * };
 */
  
static const uint8_t rfrpe_get_provisioned_table_revision_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_revision) - QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_revision_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_revision),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_OEM) - QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_OEM_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_OEM),
  RFRPE_OEM_STR_LENGTH_V01,
  QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_OEM) - QMI_IDL_OFFSET8(rfrpe_get_provisioned_table_revision_resp_v01, provisioned_table_OEM_len)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry rfrpe_message_table_v01[] = {
  {sizeof(rfrpe_set_scenario_req_v01), rfrpe_set_scenario_req_data_v01},
  {sizeof(rfrpe_set_scenario_resp_v01), rfrpe_set_scenario_resp_data_v01},
  {0, 0},
  {sizeof(rfrpe_get_rfm_scenarios_resp_v01), rfrpe_get_rfm_scenarios_resp_data_v01},
  {0, 0},
  {sizeof(rfrpe_get_provisioned_table_revision_resp_v01), rfrpe_get_provisioned_table_revision_resp_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object rfrpe_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *rfrpe_qmi_idl_type_table_object_referenced_tables_v01[] =
{&rfrpe_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object rfrpe_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(rfrpe_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  rfrpe_message_table_v01,
  rfrpe_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry rfrpe_service_command_messages_v01[] = {
  {QMI_RFRPE_SET_RFM_SCENARIO_REQ_V01, TYPE16(0, 0), 132},
  {QMI_RFRPE_GET_RFM_SCENARIO_REQ_V01, TYPE16(0, 2), 0},
  {QMI_RFRPE_GET_PROVISIONED_TABLE_REVISION_REQ_V01, TYPE16(0, 4), 0}
};

static const qmi_idl_service_message_table_entry rfrpe_service_response_messages_v01[] = {
  {QMI_RFRPE_SET_RFM_SCENARIO_RESP_V01, TYPE16(0, 1), 7},
  {QMI_RFRPE_GET_RFM_SCENARIO_RESP_V01, TYPE16(0, 3), 139},
  {QMI_RFRPE_GET_PROVISIONED_TABLE_REVISION_RESP_V01, TYPE16(0, 5), 146}
};

/*Service Object*/
struct qmi_idl_service_object rfrpe_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x29,
  146,
  { sizeof(rfrpe_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(rfrpe_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { rfrpe_service_command_messages_v01, rfrpe_service_response_messages_v01, NULL},
  &rfrpe_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type rfrpe_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( RFRPE_V01_IDL_MAJOR_VERS != idl_maj_version || RFRPE_V01_IDL_MINOR_VERS != idl_min_version 
       || RFRPE_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&rfrpe_qmi_idl_service_object_v01;
}

