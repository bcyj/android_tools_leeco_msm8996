/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S M G R _ A P I _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SMGR_SVC service Data structures.

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
   From IDL File: sns_smgr_api_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_smgr_api_v01.h"
#include "sns_common_v01.h"
#include "sns_smgr_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_smgr_periodic_report_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, ReportId),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, Action),

  0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, ReportRate),

  0x04,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, BufferFactor),

  0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, Item),
  SNS_SMGR_MAX_ITEMS_PER_REPORT_V01,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, Item) - QMI_IDL_OFFSET8(sns_smgr_periodic_report_req_msg_v01, Item_len),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, cal_sel) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, cal_sel_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_periodic_report_req_msg_v01, cal_sel),
  SNS_SMGR_MAX_ITEMS_PER_REPORT_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, cal_sel) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, cal_sel_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, SrcModule) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, SrcModule_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_periodic_report_req_msg_v01, SrcModule),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, SampleQuality) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, SampleQuality_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_periodic_report_req_msg_v01, SampleQuality),
  SNS_SMGR_MAX_ITEMS_PER_REPORT_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, SampleQuality) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, SampleQuality_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, notify_suspend) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_req_msg_v01, notify_suspend_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_periodic_report_req_msg_v01, notify_suspend),
  QMI_IDL_TYPE88(1, 1)
};

static const uint8_t sns_smgr_periodic_report_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_resp_msg_v01, ReportId),

  0x04,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_resp_msg_v01, AckNak),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_resp_msg_v01, ReasonPair),
  SNS_SMGR_MAX_NUM_REASONS_V01,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_resp_msg_v01, ReasonPair) - QMI_IDL_OFFSET8(sns_smgr_periodic_report_resp_msg_v01, ReasonPair_len),
  QMI_IDL_TYPE88(2, 1)
};

static const uint8_t sns_smgr_periodic_report_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_ind_msg_v01, ReportId),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_ind_msg_v01, status),

  0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_ind_msg_v01, CurrentRate),

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_ind_msg_v01, Item),
  SNS_SMGR_MAX_ITEMS_PER_REPORT_V01,
  QMI_IDL_OFFSET8(sns_smgr_periodic_report_ind_msg_v01, Item) - QMI_IDL_OFFSET8(sns_smgr_periodic_report_ind_msg_v01, Item_len),
  QMI_IDL_TYPE88(2, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_ind_msg_v01, SamplingRate) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_ind_msg_v01, SamplingRate_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_periodic_report_ind_msg_v01, SamplingRate),
  SNS_SMGR_MAX_ITEMS_PER_REPORT_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_ind_msg_v01, SamplingRate) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_periodic_report_ind_msg_v01, SamplingRate_len)
};

static const uint8_t sns_smgr_sensor_cal_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, usage),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, SensorId),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, DataType),

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, ZeroBias),
  SNS_SMGR_SENSOR_DIMENSION_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, ZeroBias) - QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, ZeroBias_len),

  0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, ScaleFactor),
  SNS_SMGR_SENSOR_DIMENSION_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, ScaleFactor) - QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, ScaleFactor_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CompensationMatrix) - QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CompensationMatrix_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CompensationMatrix),
  SNS_SMGR_COMPENSATION_MATRIX_SIZE_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CompensationMatrix) - QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CompensationMatrix_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CalibrationAccuracy) - QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CalibrationAccuracy_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_req_msg_v01, CalibrationAccuracy)
};

static const uint8_t sns_smgr_sensor_cal_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_cal_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * sns_smgr_all_sensor_info_req_msg is empty
 * static const uint8_t sns_smgr_all_sensor_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t sns_smgr_all_sensor_info_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_all_sensor_info_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_all_sensor_info_resp_msg_v01, SensorInfo),
  SNS_SMGR_MAX_SENSOR_NUM_V01,
  QMI_IDL_OFFSET8(sns_smgr_all_sensor_info_resp_msg_v01, SensorInfo) - QMI_IDL_OFFSET8(sns_smgr_all_sensor_info_resp_msg_v01, SensorInfo_len),
  QMI_IDL_TYPE88(2, 3)
};

static const uint8_t sns_smgr_single_sensor_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_info_req_msg_v01, SensorID)
};

static const uint8_t sns_smgr_single_sensor_info_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_info_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_info_resp_msg_v01, SensorInfo),
  QMI_IDL_TYPE88(2, 5),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, num_buffered_reports) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, num_buffered_reports_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_single_sensor_info_resp_msg_v01, num_buffered_reports),
  SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, num_buffered_reports) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, num_buffered_reports_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, op_mode) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, op_mode_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_single_sensor_info_resp_msg_v01, op_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, suid) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, suid_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_single_sensor_info_resp_msg_v01, suid),
  SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, suid) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, suid_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, supported_odr_list) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, supported_odr_list_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_single_sensor_info_resp_msg_v01, supported_odr_list),
  SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, supported_odr_list) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_single_sensor_info_resp_msg_v01, supported_odr_list_len),
  QMI_IDL_TYPE88(2, 6)
};

/* 
 * sns_smgr_sensor_test_req_msg is empty
 * static const uint8_t sns_smgr_sensor_test_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t sns_smgr_sensor_test_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_resp_msg_v01, result),
  SNS_SMGR_MAX_SENSOR_NUM_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_test_resp_msg_v01, result) - QMI_IDL_OFFSET8(sns_smgr_sensor_test_resp_msg_v01, result_len),
  QMI_IDL_TYPE88(2, 7)
};

static const uint8_t sns_smgr_single_sensor_test_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, SensorID),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, DataType),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, TestType),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, SaveToRegistry) - QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, SaveToRegistry_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, SaveToRegistry),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, ApplyCalNow) - QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, ApplyCalNow_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_req_msg_v01, ApplyCalNow)
};

static const uint8_t sns_smgr_single_sensor_test_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_resp_msg_v01, SensorID),

  0x04,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_resp_msg_v01, DataType),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_resp_msg_v01, TestType),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_resp_msg_v01, TestStatus)
};

static const uint8_t sns_smgr_single_sensor_test_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, SensorID),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, DataType),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, TestType),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, TestResult),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, ErrorCode) - QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, ErrorCode_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_single_sensor_test_ind_msg_v01, ErrorCode)
};

static const uint8_t sns_smgr_sensor_power_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_req_msg_v01, ReportId),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_req_msg_v01, Action)
};

static const uint8_t sns_smgr_sensor_power_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sns_smgr_sensor_power_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_ind_msg_v01, ReportId),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_ind_msg_v01, PowerStatus),
  SNS_SMGR_MAX_SENSOR_NUM_V01,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_ind_msg_v01, PowerStatus) - QMI_IDL_OFFSET8(sns_smgr_sensor_power_status_ind_msg_v01, PowerStatus_len),
  QMI_IDL_TYPE88(2, 8)
};

static const uint8_t sns_smgr_sensor_power_control_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_control_req_msg_v01, SensorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_control_req_msg_v01, Action)
};

static const uint8_t sns_smgr_sensor_power_control_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_power_control_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sns_smgr_sensor_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_req_msg_v01, SensorID),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_req_msg_v01, ReqDataTypeNum),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_req_msg_v01, Action)
};

static const uint8_t sns_smgr_sensor_status_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_resp_msg_v01, SensorID)
};

static const uint8_t sns_smgr_sensor_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, SensorID),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, SensorState),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, TimeStamp),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, PerProcToalClients) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, PerProcToalClients_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, PerProcToalClients),
  5,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, PerProcToalClients) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, PerProcToalClients_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxFreqPerProc) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxFreqPerProc_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxFreqPerProc),
  5,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxFreqPerProc) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxFreqPerProc_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxUpdateRatePerProc) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxUpdateRatePerProc_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxUpdateRatePerProc),
  5,
  QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxUpdateRatePerProc) - QMI_IDL_OFFSET8(sns_smgr_sensor_status_ind_msg_v01, MaxUpdateRatePerProc_len)
};

static const uint8_t sns_smgr_buffering_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, ReportId),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, Action),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, ReportRate),

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, Item),
  SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, Item) - QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, Item_len),
  QMI_IDL_TYPE88(2, 9),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, notify_suspend) - QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, notify_suspend_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, notify_suspend),
  QMI_IDL_TYPE88(1, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, SrcModule) - QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, SrcModule_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_req_msg_v01, SrcModule)
};

static const uint8_t sns_smgr_buffering_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReportId) - QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReportId_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReportId),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, AckNak) - QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, AckNak_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, AckNak),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReasonPair) - QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReasonPair_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReasonPair),
  SNS_SMGR_MAX_NUM_REASONS_V01,
  QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReasonPair) - QMI_IDL_OFFSET8(sns_smgr_buffering_resp_msg_v01, ReasonPair_len),
  QMI_IDL_TYPE88(2, 1)
};

static const uint8_t sns_smgr_buffering_query_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, QueryId),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, SensorId),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, DataType),

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, TimePeriod),
  2,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, SrcModule) - QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, SrcModule_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_req_msg_v01, SrcModule)
};

static const uint8_t sns_smgr_buffering_query_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, QueryId) - QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, QueryId_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, QueryId),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, AckNak) - QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, AckNak_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_resp_msg_v01, AckNak)
};

static const uint8_t sns_smgr_buffering_query_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_ind_msg_v01, QueryId),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_ind_msg_v01, FirstSampleTimestamp),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_ind_msg_v01, SamplingRate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_ind_msg_v01, Samples),
  SNS_SMGR_BUFFERING_REPORT_MAX_SAMPLES_V01,
  QMI_IDL_OFFSET8(sns_smgr_buffering_query_ind_msg_v01, Samples) - QMI_IDL_OFFSET8(sns_smgr_buffering_query_ind_msg_v01, Samples_len),
  QMI_IDL_TYPE88(2, 10)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_SMGR_SVC_message_table_v01[] = {
  {sizeof(sns_smgr_periodic_report_req_msg_v01), sns_smgr_periodic_report_req_msg_data_v01},
  {sizeof(sns_smgr_periodic_report_resp_msg_v01), sns_smgr_periodic_report_resp_msg_data_v01},
  {sizeof(sns_smgr_periodic_report_ind_msg_v01), sns_smgr_periodic_report_ind_msg_data_v01},
  {sizeof(sns_smgr_sensor_cal_req_msg_v01), sns_smgr_sensor_cal_req_msg_data_v01},
  {sizeof(sns_smgr_sensor_cal_resp_msg_v01), sns_smgr_sensor_cal_resp_msg_data_v01},
  {sizeof(sns_smgr_all_sensor_info_req_msg_v01), 0},
  {sizeof(sns_smgr_all_sensor_info_resp_msg_v01), sns_smgr_all_sensor_info_resp_msg_data_v01},
  {sizeof(sns_smgr_single_sensor_info_req_msg_v01), sns_smgr_single_sensor_info_req_msg_data_v01},
  {sizeof(sns_smgr_single_sensor_info_resp_msg_v01), sns_smgr_single_sensor_info_resp_msg_data_v01},
  {sizeof(sns_smgr_sensor_test_req_msg_v01), 0},
  {sizeof(sns_smgr_sensor_test_resp_msg_v01), sns_smgr_sensor_test_resp_msg_data_v01},
  {sizeof(sns_smgr_single_sensor_test_req_msg_v01), sns_smgr_single_sensor_test_req_msg_data_v01},
  {sizeof(sns_smgr_single_sensor_test_resp_msg_v01), sns_smgr_single_sensor_test_resp_msg_data_v01},
  {sizeof(sns_smgr_single_sensor_test_ind_msg_v01), sns_smgr_single_sensor_test_ind_msg_data_v01},
  {sizeof(sns_smgr_sensor_power_status_req_msg_v01), sns_smgr_sensor_power_status_req_msg_data_v01},
  {sizeof(sns_smgr_sensor_power_status_resp_msg_v01), sns_smgr_sensor_power_status_resp_msg_data_v01},
  {sizeof(sns_smgr_sensor_power_status_ind_msg_v01), sns_smgr_sensor_power_status_ind_msg_data_v01},
  {sizeof(sns_smgr_sensor_power_control_req_msg_v01), sns_smgr_sensor_power_control_req_msg_data_v01},
  {sizeof(sns_smgr_sensor_power_control_resp_msg_v01), sns_smgr_sensor_power_control_resp_msg_data_v01},
  {sizeof(sns_smgr_sensor_status_req_msg_v01), sns_smgr_sensor_status_req_msg_data_v01},
  {sizeof(sns_smgr_sensor_status_resp_msg_v01), sns_smgr_sensor_status_resp_msg_data_v01},
  {sizeof(sns_smgr_sensor_status_ind_msg_v01), sns_smgr_sensor_status_ind_msg_data_v01},
  {sizeof(sns_smgr_buffering_req_msg_v01), sns_smgr_buffering_req_msg_data_v01},
  {sizeof(sns_smgr_buffering_resp_msg_v01), sns_smgr_buffering_resp_msg_data_v01},
  {sizeof(sns_smgr_buffering_query_req_msg_v01), sns_smgr_buffering_query_req_msg_data_v01},
  {sizeof(sns_smgr_buffering_query_resp_msg_v01), sns_smgr_buffering_query_resp_msg_data_v01},
  {sizeof(sns_smgr_buffering_query_ind_msg_v01), sns_smgr_buffering_query_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SMGR_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SMGR_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_SMGR_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01, &sns_smgr_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SMGR_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_SMGR_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_SMGR_SVC_message_table_v01,
  SNS_SMGR_SVC_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SMGR_SVC_service_command_messages_v01[] = {
  {SNS_SMGR_CANCEL_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_SMGR_VERSION_REQ_V01, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_SMGR_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 0), 261},
  {SNS_SMGR_CAL_REQ_V01, QMI_IDL_TYPE16(0, 3), 91},
  {SNS_SMGR_ALL_SENSOR_INFO_REQ_V01, QMI_IDL_TYPE16(0, 5), 0},
  {SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01, QMI_IDL_TYPE16(0, 7), 4},
  {SNS_SMGR_SENSOR_TEST_REQ_V01, QMI_IDL_TYPE16(0, 9), 0},
  {SNS_SMGR_SENSOR_POWER_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 14), 8},
  {SNS_SMGR_SENSOR_POWER_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 17), 8},
  {SNS_SMGR_SENSOR_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 19), 12},
  {SNS_SMGR_SINGLE_SENSOR_TEST_REQ_V01, QMI_IDL_TYPE16(0, 11), 23},
  {SNS_SMGR_BUFFERING_REQ_V01, QMI_IDL_TYPE16(0, 22), 71},
  {SNS_SMGR_BUFFERING_QUERY_REQ_V01, QMI_IDL_TYPE16(0, 24), 28}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_SVC_service_response_messages_v01[] = {
  {SNS_SMGR_CANCEL_RESP_V01, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_SMGR_VERSION_RESP_V01, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_SMGR_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 1), 37},
  {SNS_SMGR_CAL_RESP_V01, QMI_IDL_TYPE16(0, 4), 5},
  {SNS_SMGR_ALL_SENSOR_INFO_RESP_V01, QMI_IDL_TYPE16(0, 6), 369},
  {SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01, QMI_IDL_TYPE16(0, 8), 1033},
  {SNS_SMGR_SENSOR_TEST_RESP_V01, QMI_IDL_TYPE16(0, 10), 189},
  {SNS_SMGR_SENSOR_POWER_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 15), 5},
  {SNS_SMGR_SENSOR_POWER_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 18), 5},
  {SNS_SMGR_SENSOR_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 20), 9},
  {SNS_SMGR_SINGLE_SENSOR_TEST_RESP_V01, QMI_IDL_TYPE16(0, 12), 27},
  {SNS_SMGR_BUFFERING_RESP_V01, QMI_IDL_TYPE16(0, 23), 37},
  {SNS_SMGR_BUFFERING_QUERY_RESP_V01, QMI_IDL_TYPE16(0, 25), 14}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_SVC_service_indication_messages_v01[] = {
  {SNS_SMGR_REPORT_IND_V01, QMI_IDL_TYPE16(0, 2), 271},
  {SNS_SMGR_SENSOR_POWER_STATUS_IND_V01, QMI_IDL_TYPE16(0, 16), 288},
  {SNS_SMGR_SENSOR_STATUS_IND_V01, QMI_IDL_TYPE16(0, 21), 77},
  {SNS_SMGR_SINGLE_SENSOR_TEST_IND_V01, QMI_IDL_TYPE16(0, 13), 26},
  {SNS_SMGR_BUFFERING_IND_V01, QMI_IDL_TYPE16(2, 0), 1676},
  {SNS_SMGR_BUFFERING_QUERY_IND_V01, QMI_IDL_TYPE16(0, 26), 1623}
};

/*Service Object*/
struct qmi_idl_service_object SNS_SMGR_SVC_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  SNS_QMI_SVC_ID_0_V01,
  1676,
  { sizeof(SNS_SMGR_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_SVC_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SMGR_SVC_service_command_messages_v01, SNS_SMGR_SVC_service_response_messages_v01, SNS_SMGR_SVC_service_indication_messages_v01},
  &SNS_SMGR_SVC_qmi_idl_type_table_object_v01,
  0x17,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SMGR_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SMGR_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_SMGR_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_SMGR_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SMGR_SVC_qmi_idl_service_object_v01;
}

