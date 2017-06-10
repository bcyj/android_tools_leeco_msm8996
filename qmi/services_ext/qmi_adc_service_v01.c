/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q M I _ A D C _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the adc service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.5
   It was generated on: Fri Apr 29 2011
   From IDL File: qmi_adc_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_adc_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t AdcInputPropertiesType_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcInputPropertiesType_v01, nDeviceIdx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcInputPropertiesType_v01, nChannelIdx),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t AdcRequestParametersType_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcRequestParametersType_v01, nDeviceIdx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcRequestParametersType_v01, nChannelIdx),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t AdcBatchParametersType_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, nDeviceIdx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, nChannelIdx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, eFormat),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, nNumBatches),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, nDurationUs),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, nNumConversions),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, nPeriodUs),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(AdcBatchParametersType_v01, isTimeStampRqd),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t AdcBatchStatusType_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchStatusType_v01, nToken),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t AdcResultType_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, eStatus),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nToken),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nDeviceIdx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nChannelIdx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nPhysical),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nPercent),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nMicrovolts),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcResultType_v01, nReserved),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t AdcBatchResultType_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchResultType_v01, eStatus),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchResultType_v01, nToken),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchResultType_v01, nSamples),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(AdcBatchResultType_v01, aSamples),
  ((8192) & 0xFF), ((8192) >> 8),
  QMI_IDL_OFFSET8(AdcBatchResultType_v01, aSamples) - QMI_IDL_OFFSET8(AdcBatchResultType_v01, aSamples_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t adc_get_input_properties_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(adc_get_input_properties_req_msg_v01, adc_channel_name),
  ADC_CH_NAME_LENGTH_V01
};

static const uint8_t adc_get_input_properties_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_get_input_properties_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(adc_get_input_properties_resp_msg_v01, resp_value) - QMI_IDL_OFFSET8(adc_get_input_properties_resp_msg_v01, resp_value_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_get_input_properties_resp_msg_v01, resp_value),
  0, 0
};

static const uint8_t adc_request_conversion_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_conversion_req_msg_v01, adc_param),
  1, 0
};

static const uint8_t adc_request_conversion_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_conversion_resp_msg_v01, resp),
  0, 1
};

static const uint8_t adc_request_recalibration_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_recalibration_req_msg_v01, adc_param),
  1, 0
};

static const uint8_t adc_request_recalibration_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_recalibration_resp_msg_v01, resp),
  0, 1
};

static const uint8_t adc_request_batch_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_batch_req_msg_v01, adc_param),
  2, 0
};

static const uint8_t adc_request_batch_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_batch_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(adc_request_batch_resp_msg_v01, adcBatchStatus) - QMI_IDL_OFFSET8(adc_request_batch_resp_msg_v01, adcBatchStatus_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_request_batch_resp_msg_v01, adcBatchStatus),
  3, 0
};

static const uint8_t adc_stop_batch_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(adc_stop_batch_req_msg_v01, nToken)
};

static const uint8_t adc_stop_batch_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_stop_batch_resp_msg_v01, resp),
  0, 1
};

static const uint8_t adc_event_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(adc_event_report_req_msg_v01, req) - QMI_IDL_OFFSET8(adc_event_report_req_msg_v01, req_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(adc_event_report_req_msg_v01, req)
};

static const uint8_t adc_event_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_event_report_resp_msg_v01, resp),
  0, 1
};

static const uint8_t adc_conversion_complete_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_conversion_complete_ind_msg_v01, adcResult),
  4, 0
};

static const uint8_t adc_batch_data_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(adc_batch_data_ind_msg_v01, adcBatchPayload),
  5, 0
};

/* Type Table */
static const qmi_idl_type_table_entry  adc_type_table_v01[] = {
  {sizeof(AdcInputPropertiesType_v01), AdcInputPropertiesType_data_v01},
  {sizeof(AdcRequestParametersType_v01), AdcRequestParametersType_data_v01},
  {sizeof(AdcBatchParametersType_v01), AdcBatchParametersType_data_v01},
  {sizeof(AdcBatchStatusType_v01), AdcBatchStatusType_data_v01},
  {sizeof(AdcResultType_v01), AdcResultType_data_v01},
  {sizeof(AdcBatchResultType_v01), AdcBatchResultType_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry adc_message_table_v01[] = {
  {sizeof(adc_get_input_properties_req_msg_v01), adc_get_input_properties_req_msg_data_v01},
  {sizeof(adc_get_input_properties_resp_msg_v01), adc_get_input_properties_resp_msg_data_v01},
  {sizeof(adc_request_conversion_req_msg_v01), adc_request_conversion_req_msg_data_v01},
  {sizeof(adc_request_conversion_resp_msg_v01), adc_request_conversion_resp_msg_data_v01},
  {sizeof(adc_request_recalibration_req_msg_v01), adc_request_recalibration_req_msg_data_v01},
  {sizeof(adc_request_recalibration_resp_msg_v01), adc_request_recalibration_resp_msg_data_v01},
  {sizeof(adc_request_batch_req_msg_v01), adc_request_batch_req_msg_data_v01},
  {sizeof(adc_request_batch_resp_msg_v01), adc_request_batch_resp_msg_data_v01},
  {sizeof(adc_stop_batch_req_msg_v01), adc_stop_batch_req_msg_data_v01},
  {sizeof(adc_stop_batch_resp_msg_v01), adc_stop_batch_resp_msg_data_v01},
  {sizeof(adc_event_report_req_msg_v01), adc_event_report_req_msg_data_v01},
  {sizeof(adc_event_report_resp_msg_v01), adc_event_report_resp_msg_data_v01},
  {sizeof(adc_conversion_complete_ind_msg_v01), adc_conversion_complete_ind_msg_data_v01},
  {sizeof(adc_batch_data_ind_msg_v01), adc_batch_data_ind_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object adc_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *adc_qmi_idl_type_table_object_referenced_tables_v01[] =
{&adc_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object adc_qmi_idl_type_table_object_v01 = {
  sizeof(adc_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(adc_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  adc_type_table_v01,
  adc_message_table_v01,
  adc_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry adc_service_command_messages_v01[] = {
  {QMI_ADC_GET_ADC_INPUT_PROPERTIES_REQ_V01, TYPE16(0, 0), 43},
  {QMI_ADC_REQUEST_CONVERSION_REQ_V01, TYPE16(0, 2), 11},
  {QMI_ADC_REQUEST_RECALIBRATION_REQ_V01, TYPE16(0, 4), 11},
  {QMI_ADC_REQUEST_BATCH_REQ_V01, TYPE16(0, 6), 32},
  {QMI_ADC_STOP_BATCH_REQ_V01, TYPE16(0, 8), 7}
};

static const qmi_idl_service_message_table_entry adc_service_response_messages_v01[] = {
  {QMI_ADC_GET_ADC_INPUT_PROPERTIES_RESP_V01, TYPE16(0, 1), 18},
  {QMI_ADC_REQUEST_CONVERSION_RESP_V01, TYPE16(0, 3), 7},
  {QMI_ADC_REQUEST_RECALIBRATION_RESP_V01, TYPE16(0, 5), 7},
  {QMI_ADC_REQUEST_BATCH_RESP_V01, TYPE16(0, 7), 14},
  {QMI_ADC_STOP_BATCH_RESP_V01, TYPE16(0, 9), 7}
};

static const qmi_idl_service_message_table_entry adc_service_indication_messages_v01[] = {
  {QMI_ADC_EVENT_REPORT_REQ_V01, TYPE16(0, 10), 7},
  {QMI_ADC_EVENT_REPORT_RESP_V01, TYPE16(0, 11), 7},
  {QMI_ADC_CONVERSION_COMPLETE_IND_V01, TYPE16(0, 12), 35},
  {QMI_ADC_BATCH_DATA_IND_V01, TYPE16(0, 13), 32785}
};

/*Service Object*/
const struct qmi_idl_service_object adc_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  19,
  32785,
  { sizeof(adc_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(adc_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(adc_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { adc_service_command_messages_v01, adc_service_response_messages_v01, adc_service_indication_messages_v01},
  &adc_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type adc_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( ADC_V01_IDL_MAJOR_VERS != idl_maj_version || ADC_V01_IDL_MINOR_VERS != idl_min_version 
       || ADC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&adc_qmi_idl_service_object_v01;
}

