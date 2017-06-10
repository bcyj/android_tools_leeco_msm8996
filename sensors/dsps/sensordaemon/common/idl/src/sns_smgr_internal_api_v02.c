/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S M G R _ I N T E R N A L _ A P I _ V 0 2  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SMGR_INTERNAL_SVC service Data structures.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.10 
   It was generated on: Mon Jul 21 2014 (Spin 0)
   From IDL File: sns_smgr_internal_api_v02.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_smgr_internal_api_v02.h"
#include "sns_common_v01.h"
#include "sns_smgr_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/* 
 * sns_smgr_sensor_events_query_req_msg is empty
 * static const uint8_t sns_smgr_sensor_events_query_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t sns_smgr_sensor_events_query_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_events_query_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_events_query_resp_msg_v02, sensor_events) - QMI_IDL_OFFSET8(sns_smgr_sensor_events_query_resp_msg_v02, sensor_events_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_events_query_resp_msg_v02, sensor_events),
  SNS_MAX_NUM_SENSOR_EVENTS_V02,
  QMI_IDL_OFFSET8(sns_smgr_sensor_events_query_resp_msg_v02, sensor_events) - QMI_IDL_OFFSET8(sns_smgr_sensor_events_query_resp_msg_v02, sensor_events_len)
};

static const uint8_t sns_smgr_sensor_event_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_req_msg_v02, sensor_event),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_req_msg_v02, registering),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_req_msg_v02, shortest_interval)
};

static const uint8_t sns_smgr_sensor_event_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sns_smgr_sensor_event_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, sensor_event),

  0x02,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, timestamp),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, event_info) - QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, event_info_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, event_info),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, event_status) - QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, event_status_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_event_ind_msg_v02, event_status)
};

static const uint8_t sns_smgr_sensor_status_monitor_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_req_msg_v02, sensor_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_req_msg_v02, registering)
};

static const uint8_t sns_smgr_sensor_status_monitor_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_resp_msg_v02, sensor_id) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_resp_msg_v02, sensor_id_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_resp_msg_v02, sensor_id)
};

static const uint8_t sns_smgr_sensor_status_monitor_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_ind_msg_v02, sensor_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_ind_msg_v02, num_clients),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_ind_msg_v02, sampling_rate),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_ind_msg_v02, wakeup_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_monitor_ind_msg_v02, timestamp)
};

static const uint8_t sns_smgr_event_gated_buffering_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, ReportId),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, Action),

  0x03,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, SensorEvent),

  0x04,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, EventDisabledConfig),
  QMI_IDL_TYPE88(2, 9),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, EventDisabledReportRate),

  0x06,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, EventEnabledConfig),
  QMI_IDL_TYPE88(2, 9),

  0x07,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, EventEnabledReportRate),

  0x08,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, EventOccurredConfig),
  QMI_IDL_TYPE88(2, 9),

  0x09,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, EventOccurredReportRate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x0A,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_req_msg_v02, NotifySuspend),
  QMI_IDL_TYPE88(1, 1)
};

static const uint8_t sns_smgr_event_gated_buffering_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReportId) - QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReportId_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReportId),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, AckNak) - QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, AckNak_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, AckNak),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReasonPair) - QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReasonPair_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReasonPair),
  SNS_SMGR_MAX_NUM_REASONS_V01,
  QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReasonPair) - QMI_IDL_OFFSET8(sns_smgr_event_gated_buffering_resp_msg_v02, ReasonPair_len),
  QMI_IDL_TYPE88(2, 1)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_SMGR_INTERNAL_SVC_message_table_v02[] = {
  {sizeof(sns_smgr_sensor_events_query_req_msg_v02), 0},
  {sizeof(sns_smgr_sensor_events_query_resp_msg_v02), sns_smgr_sensor_events_query_resp_msg_data_v02},
  {sizeof(sns_smgr_sensor_event_req_msg_v02), sns_smgr_sensor_event_req_msg_data_v02},
  {sizeof(sns_smgr_sensor_event_resp_msg_v02), sns_smgr_sensor_event_resp_msg_data_v02},
  {sizeof(sns_smgr_sensor_event_ind_msg_v02), sns_smgr_sensor_event_ind_msg_data_v02},
  {sizeof(sns_smgr_sensor_status_monitor_req_msg_v02), sns_smgr_sensor_status_monitor_req_msg_data_v02},
  {sizeof(sns_smgr_sensor_status_monitor_resp_msg_v02), sns_smgr_sensor_status_monitor_resp_msg_data_v02},
  {sizeof(sns_smgr_sensor_status_monitor_ind_msg_v02), sns_smgr_sensor_status_monitor_ind_msg_data_v02},
  {sizeof(sns_smgr_event_gated_buffering_req_msg_v02), sns_smgr_event_gated_buffering_req_msg_data_v02},
  {sizeof(sns_smgr_event_gated_buffering_resp_msg_v02), sns_smgr_event_gated_buffering_resp_msg_data_v02}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v02;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_referenced_tables_v02[] =
{&SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v02, &sns_common_qmi_idl_type_table_object_v01, &sns_smgr_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v02 = {
  0,
  sizeof(SNS_SMGR_INTERNAL_SVC_message_table_v02)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_SMGR_INTERNAL_SVC_message_table_v02,
  SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_referenced_tables_v02,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SMGR_INTERNAL_SVC_service_command_messages_v02[] = {
  {SNS_SMGR_CANCEL_REQ_V02, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_SMGR_VERSION_REQ_V02, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_SMGR_SENSOR_EVENTS_QUERY_REQ_V02, QMI_IDL_TYPE16(0, 0), 0},
  {SNS_SMGR_SENSOR_EVENT_REQ_V02, QMI_IDL_TYPE16(0, 2), 22},
  {SNS_SMGR_SENSOR_STATUS_MONITOR_REQ_V02, QMI_IDL_TYPE16(0, 5), 15},
  {SNS_SMGR_EVENT_GATED_BUFFERING_REQ_V02, QMI_IDL_TYPE16(0, 8), 84}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_INTERNAL_SVC_service_response_messages_v02[] = {
  {SNS_SMGR_CANCEL_RESP_V02, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_SMGR_VERSION_RESP_V02, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_SMGR_SENSOR_EVENTS_QUERY_RESP_V02, QMI_IDL_TYPE16(0, 1), 1609},
  {SNS_SMGR_SENSOR_EVENT_RESP_V02, QMI_IDL_TYPE16(0, 3), 5},
  {SNS_SMGR_SENSOR_STATUS_MONITOR_RESP_V02, QMI_IDL_TYPE16(0, 6), 16},
  {SNS_SMGR_EVENT_GATED_BUFFERING_RESP_V02, QMI_IDL_TYPE16(0, 9), 37}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_INTERNAL_SVC_service_indication_messages_v02[] = {
  {SNS_SMGR_SENSOR_EVENT_IND_V02, QMI_IDL_TYPE16(0, 4), 40},
  {SNS_SMGR_SENSOR_STATUS_MONITOR_IND_V02, QMI_IDL_TYPE16(0, 7), 40},
  {SNS_SMGR_EVENT_GATED_BUFFERING_IND_V02, QMI_IDL_TYPE16(2, 0), 1676}
};

/*Service Object*/
struct qmi_idl_service_object SNS_SMGR_INTERNAL_SVC_qmi_idl_service_object_v02 = {
  0x06,
  0x02,
  SNS_SMGR_INTERNAL_SVC_ID_V02,
  1676,
  { sizeof(SNS_SMGR_INTERNAL_SVC_service_command_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_INTERNAL_SVC_service_response_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_INTERNAL_SVC_service_indication_messages_v02)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SMGR_INTERNAL_SVC_service_command_messages_v02, SNS_SMGR_INTERNAL_SVC_service_response_messages_v02, SNS_SMGR_INTERNAL_SVC_service_indication_messages_v02},
  &SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v02,
  0x03,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SMGR_INTERNAL_SVC_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SMGR_INTERNAL_SVC_V02_IDL_MAJOR_VERS != idl_maj_version || SNS_SMGR_INTERNAL_SVC_V02_IDL_MINOR_VERS != idl_min_version 
       || SNS_SMGR_INTERNAL_SVC_V02_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SMGR_INTERNAL_SVC_qmi_idl_service_object_v02;
}

