/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S M G R _ C O M M O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sns_smgr_common service Data structures.

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.10 
   It was generated on: Mon Jul 21 2014 (Spin 0)
   From IDL File: sns_smgr_common_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_smgr_common_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
static const uint8_t sns_smgr_periodic_report_item_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, SensorId),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, DataType),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, Sensitivity),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, Decimation),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, MinSampleRate),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, StationaryOption),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, DoThresholdTest),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, ThresholdOutsideMinMax),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, ThresholdDelta),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, ThresholdAllAxes),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_item_s_v01, ThresholdMinMax),
  2,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_reason_pair_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reason_pair_s_v01, ItemNum),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reason_pair_s_v01, Reason),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_data_item_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, SensorId),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, DataType),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, ItemData),
  3,

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, TimeStamp),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, ItemFlags),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, ItemQuality),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_data_item_s_v01, ItemSensitivity),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_sensor_id_info_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_id_info_s_v01, SensorID),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_id_info_s_v01, SensorShortName),
  SNS_SMGR_SHORT_SENSOR_NAME_SIZE_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_id_info_s_v01, SensorShortName) - QMI_IDL_OFFSET8(sns_smgr_sensor_id_info_s_v01, SensorShortName_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_sensor_datatype_info_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, SensorID),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, DataType),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, SensorName),
  SNS_SMGR_MAX_SENSOR_NAME_SIZE_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, SensorName) - QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, SensorName_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, VendorName),
  SNS_SMGR_MAX_VENDOR_NAME_SIZE_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, VendorName) - QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, VendorName_len),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, Version),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, MaxSampleRate),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, IdlePower),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, MaxPower),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, MaxRange),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_datatype_info_s_v01, Resolution),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_sensor_info_s_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_info_s_v01, data_type_info),
  SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_info_s_v01, data_type_info) - QMI_IDL_OFFSET8(sns_smgr_sensor_info_s_v01, data_type_info_len),
  QMI_IDL_TYPE88(0, 4),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_odr_list_s_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_odr_list_s_v01, odrs),
  SNS_SMGR_MAX_SUPPORTED_ODR_NUM_V01,
  QMI_IDL_OFFSET8(sns_smgr_odr_list_s_v01, odrs) - QMI_IDL_OFFSET8(sns_smgr_odr_list_s_v01, odrs_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_sensor_test_result_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, SensorID),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, BusCanAccessSensor),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, CanCommandSensor),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, CanReadSensorStatus),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, CanReadSensorData),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, DataShowsNoise),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, CanReadFactoryCalibrationROM),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, ValidSelfTestReport),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_result_s_v01, CanReceiveInterrupt),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_sensor_power_status_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_s_v01, SensorID),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_s_v01, PowerAction),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_s_v01, ActiveTimeStamp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_s_v01, LowPowerTimeStamp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_s_v01, CycleCount),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_buffering_req_item_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_item_s_v01, SensorId),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_item_s_v01, DataType),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_item_s_v01, Decimation),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_item_s_v01, Calibration),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_item_s_v01, SamplingRate),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_item_s_v01, SampleQuality),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_buffering_sample_s_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_s_v01, Data),
  SNS_SMGR_SENSOR_DIMENSION_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_s_v01, TimeStampOffset),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_s_v01, Flags),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_s_v01, Quality),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_smgr_buffering_sample_index_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_index_s_v01, SensorId),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_index_s_v01, DataType),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_index_s_v01, FirstSampleIdx),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_index_s_v01, SampleCount),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_index_s_v01, FirstSampleTimestamp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_sample_index_s_v01, SamplingRate),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t sns_smgr_buffering_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, ReportId),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, Indices),
  SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01,
  QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, Indices) - QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, Indices_len),
  QMI_IDL_TYPE88(0, 11),

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, Samples),
  SNS_SMGR_BUFFERING_REPORT_MAX_SAMPLES_V01,
  QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, Samples) - QMI_IDL_OFFSET8(sns_smgr_buffering_ind_msg_v01, Samples_len),
  QMI_IDL_TYPE88(0, 10),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_buffering_ind_msg_v01, IndType) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_buffering_ind_msg_v01, IndType_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_buffering_ind_msg_v01, IndType)
};

/* Type Table */
static const qmi_idl_type_table_entry  sns_smgr_common_type_table_v01[] = {
  {sizeof(sns_smgr_periodic_report_item_s_v01), sns_smgr_periodic_report_item_s_data_v01},
  {sizeof(sns_smgr_reason_pair_s_v01), sns_smgr_reason_pair_s_data_v01},
  {sizeof(sns_smgr_data_item_s_v01), sns_smgr_data_item_s_data_v01},
  {sizeof(sns_smgr_sensor_id_info_s_v01), sns_smgr_sensor_id_info_s_data_v01},
  {sizeof(sns_smgr_sensor_datatype_info_s_v01), sns_smgr_sensor_datatype_info_s_data_v01},
  {sizeof(sns_smgr_sensor_info_s_v01), sns_smgr_sensor_info_s_data_v01},
  {sizeof(sns_smgr_odr_list_s_v01), sns_smgr_odr_list_s_data_v01},
  {sizeof(sns_smgr_sensor_test_result_s_v01), sns_smgr_sensor_test_result_s_data_v01},
  {sizeof(sns_smgr_sensor_power_status_s_v01), sns_smgr_sensor_power_status_s_data_v01},
  {sizeof(sns_smgr_buffering_req_item_s_v01), sns_smgr_buffering_req_item_s_data_v01},
  {sizeof(sns_smgr_buffering_sample_s_v01), sns_smgr_buffering_sample_s_data_v01},
  {sizeof(sns_smgr_buffering_sample_index_s_v01), sns_smgr_buffering_sample_index_s_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry sns_smgr_common_message_table_v01[] = {
  {sizeof(sns_smgr_buffering_ind_msg_v01), sns_smgr_buffering_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
const qmi_idl_type_table_object sns_smgr_common_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sns_smgr_common_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sns_smgr_common_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
const qmi_idl_type_table_object sns_smgr_common_qmi_idl_type_table_object_v01 = {
  sizeof(sns_smgr_common_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(sns_smgr_common_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  sns_smgr_common_type_table_v01,
  sns_smgr_common_message_table_v01,
  sns_smgr_common_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

