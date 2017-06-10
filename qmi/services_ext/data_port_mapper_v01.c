/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        D A T A _ P O R T _ M A P P E R _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the dpm service Data structures.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It was generated on: Tue Sep 24 2013 (Spin 0)
   From IDL File: data_port_mapper_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "data_port_mapper_v01.h"
#include "common_v01.h"
#include "data_common_v01.h"


/*Type Definitions*/
static const uint8_t ctl_port_map_details_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ctl_port_map_details_v01, port_name),
  QMI_DPM_PORT_NAME_MAX_V01,

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ctl_port_map_details_v01, default_ep_id),
  QMI_IDL_TYPE88(2, 0),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dpm_ep_pair_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dpm_ep_pair_type_v01, consumer_pipe_num),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dpm_ep_pair_type_v01, producer_pipe_num),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t hardware_accl_port_details_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(hardware_accl_port_details_v01, ep_id),
  QMI_IDL_TYPE88(2, 0),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(hardware_accl_port_details_v01, hardware_ep_pair),
  QMI_IDL_TYPE88(0, 1),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t software_data_port_map_details_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(software_data_port_map_details_v01, ep_id),
  QMI_IDL_TYPE88(2, 0),
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(software_data_port_map_details_v01, port_name),
  QMI_DPM_PORT_NAME_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t ctl_port_name_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ctl_port_name_v01, port_name),
  QMI_DPM_PORT_NAME_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t dpm_open_port_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dpm_open_port_req_msg_v01, control_port_list) - QMI_IDL_OFFSET8(dpm_open_port_req_msg_v01, control_port_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dpm_open_port_req_msg_v01, control_port_list),
  QMI_DPM_PORT_MAX_NUM_V01,
  QMI_IDL_OFFSET8(dpm_open_port_req_msg_v01, control_port_list) - QMI_IDL_OFFSET8(dpm_open_port_req_msg_v01, control_port_list_len),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, hardware_data_port_list) - QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, hardware_data_port_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dpm_open_port_req_msg_v01, hardware_data_port_list),
  QMI_DPM_PORT_MAX_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, hardware_data_port_list) - QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, hardware_data_port_list_len),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, software_data_port_list) - QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, software_data_port_list_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dpm_open_port_req_msg_v01, software_data_port_list),
  QMI_DPM_PORT_MAX_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, software_data_port_list) - QMI_IDL_OFFSET16RELATIVE(dpm_open_port_req_msg_v01, software_data_port_list_len),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t dpm_open_port_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dpm_open_port_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dpm_close_port_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dpm_close_port_req_msg_v01, control_port_list) - QMI_IDL_OFFSET8(dpm_close_port_req_msg_v01, control_port_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dpm_close_port_req_msg_v01, control_port_list),
  QMI_DPM_PORT_MAX_NUM_V01,
  QMI_IDL_OFFSET8(dpm_close_port_req_msg_v01, control_port_list) - QMI_IDL_OFFSET8(dpm_close_port_req_msg_v01, control_port_list_len),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dpm_close_port_req_msg_v01, data_port_list) - QMI_IDL_OFFSET16RELATIVE(dpm_close_port_req_msg_v01, data_port_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dpm_close_port_req_msg_v01, data_port_list),
  QMI_DPM_PORT_MAX_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(dpm_close_port_req_msg_v01, data_port_list) - QMI_IDL_OFFSET16RELATIVE(dpm_close_port_req_msg_v01, data_port_list_len),
  QMI_IDL_TYPE88(2, 0)
};

static const uint8_t dpm_close_port_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dpm_close_port_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  dpm_type_table_v01[] = {
  {sizeof(ctl_port_map_details_v01), ctl_port_map_details_data_v01},
  {sizeof(dpm_ep_pair_type_v01), dpm_ep_pair_type_data_v01},
  {sizeof(hardware_accl_port_details_v01), hardware_accl_port_details_data_v01},
  {sizeof(software_data_port_map_details_v01), software_data_port_map_details_data_v01},
  {sizeof(ctl_port_name_v01), ctl_port_name_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry dpm_message_table_v01[] = {
  {sizeof(dpm_open_port_req_msg_v01), dpm_open_port_req_msg_data_v01},
  {sizeof(dpm_open_port_resp_msg_v01), dpm_open_port_resp_msg_data_v01},
  {sizeof(dpm_close_port_req_msg_v01), dpm_close_port_req_msg_data_v01},
  {sizeof(dpm_close_port_resp_msg_v01), dpm_close_port_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object dpm_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *dpm_qmi_idl_type_table_object_referenced_tables_v01[] =
{&dpm_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01, &data_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object dpm_qmi_idl_type_table_object_v01 = {
  sizeof(dpm_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(dpm_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  dpm_type_table_v01,
  dpm_message_table_v01,
  dpm_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry dpm_service_command_messages_v01[] = {
  {QMI_DPM_OPEN_PORT_REQ_V01, QMI_IDL_TYPE16(0, 0), 3148},
  {QMI_DPM_CLOSE_PORT_REQ_V01, QMI_IDL_TYPE16(0, 2), 1320}
};

static const qmi_idl_service_message_table_entry dpm_service_response_messages_v01[] = {
  {QMI_DPM_OPEN_PORT_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_DPM_CLOSE_PORT_RESP_V01, QMI_IDL_TYPE16(0, 3), 7}
};

/*Service Object*/
struct qmi_idl_service_object dpm_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x2F,
  3148,
  { sizeof(dpm_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dpm_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { dpm_service_command_messages_v01, dpm_service_response_messages_v01, NULL},
  &dpm_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type dpm_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( DPM_V01_IDL_MAJOR_VERS != idl_maj_version || DPM_V01_IDL_MINOR_VERS != idl_min_version
       || DPM_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&dpm_qmi_idl_service_object_v01;
}

