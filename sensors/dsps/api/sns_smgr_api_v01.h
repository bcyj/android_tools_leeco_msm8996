#ifndef SNS_SMGR_SVC_SERVICE_01_H
#define SNS_SMGR_SVC_SERVICE_01_H
/**
  @file sns_smgr_api_v01.h

  @brief This is the public header file which defines the SNS_SMGR_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SMGR_SVC. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set 
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
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

/** @defgroup SNS_SMGR_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SMGR_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SMGR_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SMGR_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SMGR_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SMGR_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SMGR_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"
#include "sns_smgr_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SMGR_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SMGR_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SMGR_SVC_V01_IDL_MINOR_VERS 0x17
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SMGR_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SMGR_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SMGR_SVC_qmi_consts 
    @{ 
  */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command requests sensor periodic report for sensor sampling */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   The report ID assigned by a client to distinguish this report among its
       reports */

  /* Mandatory */
  uint8_t Action;
  /**<   Defines if this report is to be added or deleted.
    - 01 - SNS_SMGR_REPORT_ACTION_ADD
    - 02 - SNS_SMGR_REPORT_ACTION_DELETE
    - All other values defined as SNS_SMGR_REPORT_ACTION_XXXX style are reserved
      for future use
    When SNS_SMGR_REPORT_ACTION_ADD is used and the same report ID is already added,
    the old one will be replaced by the new report request.
    */

  /* Mandatory */
  uint16_t ReportRate;
  /**<   Defines reporting rate. This value shall be within the sensor capacity which
    can be identified by using SNS_SMGR_SINGLE_SENSOR_INFO message. When this parameter
    is 0, 20Hz will be used as the default.
    If a value greater than SNS_SMGR_SAMPLING_RATE_INVERSION_POINT is given, then this
    parameter will be treated as a report period, in milliseconds. This allows a client
    to sample the sensor at a rate less than 1 Hz. It is not recommended to sample any
    sensor but environmental sensors at a rate less than 1 Hz. */

  /* Mandatory */
  uint8_t BufferFactor;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0 */

  /* Mandatory */
  uint32_t Item_len;  /**< Must be set to # of elements in Item */
  sns_smgr_periodic_report_item_s_v01 Item[SNS_SMGR_MAX_ITEMS_PER_REPORT_V01];

  /* Optional */
  uint8_t cal_sel_valid;  /**< Must be set to true if cal_sel is being passed */
  uint32_t cal_sel_len;  /**< Must be set to # of elements in cal_sel */
  uint8_t cal_sel[SNS_SMGR_MAX_ITEMS_PER_REPORT_V01];
  /**<   Defines the calibration option to be used. The index of the cal sel should match
    to the index of the Item parameter.
    - 00 - SNS_SMGR_CAL_SEL_FULL_CAL which refers applying factory calibration factors
           (if available) and auto calibration factors(if available) on to the raw data
    - 01 - SNS_SMGR_CAL_SEL_FACTORY_CAL which refers applying factory calibration factors
          (if available) on to the raw data
    - 02 - SNS_SMGR_CAL_SEL_RAW
    - All other values defined as SNS_SMGR_CAL_SEL_XXXX style are reserved for future use
    */

  /* Optional */
  uint8_t SrcModule_valid;  /**< Must be set to true if SrcModule is being passed */
  uint8_t SrcModule;
  /**<   For sensor internal use only.
       Defines the source module that is sending this message.
    */

  /* Optional */
  uint8_t SampleQuality_valid;  /**< Must be set to true if SampleQuality is being passed */
  uint32_t SampleQuality_len;  /**< Must be set to # of elements in SampleQuality */
  uint16_t SampleQuality[SNS_SMGR_MAX_ITEMS_PER_REPORT_V01];
  /**<   Specifies the desired quality of sensor data
    - SNS_SMGR_SAMPLE_QUALITY_ACCURATE_TIMESTAMP - High accuracy for sample timestamp.
      Delivery sampling rate may be up to twice the requested sampling rate,
      and may also result in higher report rate.
      Clients are recommended to specify 50, 100, or 200Hz sampling rates to
      minimize the chance of increase in sampling rate.
    */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       proc_type                  = SNS_PROC_APPS
       send_indications_during_suspend  = FALSE
    */
}sns_smgr_periodic_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests sensor periodic report for sensor sampling */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  uint8_t ReportId;
  /**<   The report ID assigned by a client to distinguish this report among its reports */

  /* Mandatory */
  uint8_t AckNak;
  /**<   Defines whether this response is Acknowledgement or Negative Acknowledgement
    - 00 - SNS_SMGR_RESPONSE_ACK_SUCCESS
    - 01 - SNS_SMGR_RESPONSE_ACK_MODIFIED some parameters in the request are modified
    - 02 - SNS_SMGR_RESPONSE_NAK_RESOURCES
    - 03 - SNS_SMGR_RESPONSE_NAK_REPORT_ID Can't find report to delete
    - 04 - SNS_SMGR_RESPONSE_NAK_NO_ITEMS no item is supplied or the item is
           deleted by SMGR because of wrong parameters
    - 05 - SNS_SMGR_RESPONSE_NAK_UNK_ACTION when the action value is other than
           add or delete
    - All other values defined as SNS_SMGR_RESPONSE_ACK/NAK_XXXX style are reserved
      for future use
  */

  /* Mandatory */
  uint32_t ReasonPair_len;  /**< Must be set to # of elements in ReasonPair */
  sns_smgr_reason_pair_s_v01 ReasonPair[SNS_SMGR_MAX_NUM_REASONS_V01];
}sns_smgr_periodic_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Indication Message; This command requests sensor periodic report for sensor sampling */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   The report ID assigned by a client to distinguish this report among its
       reports */

  /* Mandatory */
  uint8_t status;
  /**<   Defines the status. Non-zero code notifies that this report is canceled
      - 00 - SNS_SMGR_REPORT_OK
      - 01 - SNS_SMGR_REPORT_CANCEL_RESOURCE
      - 02 - SNS_SMGR_REPORT_CANCEL_FAILURE
      - All other values defined as SNS_SMGR_REPORT_XXXX style are reserved for
        future use
       */

  /* Mandatory */
  uint16_t CurrentRate;
  /**<   The current reporting rate that is the sampling rate of the first item.
       If this value is greater than SNS_SMGR_SAMPLING_RATE_INVERSION_POINT,
       then it should be interpreted as the current period, in milliseconds. */

  /* Mandatory */
  uint32_t Item_len;  /**< Must be set to # of elements in Item */
  sns_smgr_data_item_s_v01 Item[SNS_SMGR_MAX_ITEMS_PER_REPORT_V01];

  /* Optional */
  uint8_t SamplingRate_valid;  /**< Must be set to true if SamplingRate is being passed */
  uint32_t SamplingRate_len;  /**< Must be set to # of elements in SamplingRate */
  uint32_t SamplingRate[SNS_SMGR_MAX_ITEMS_PER_REPORT_V01];
  /**<   Specifies the frequency at which sensor is actually sampled. This value
       is expressed in Q16 format and in unit of Hz.
     */
}sns_smgr_periodic_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command sets dynamic or auto generated calibrate factors of a sensor. */
typedef struct {

  /* Mandatory */
  uint8_t usage;
  /**<   Defines the usage of the calibration data in this request message.
    - 00 - SNS_SMGR_CAL_DYNAMIC
    - 02 - SNS_SMGR_CAL_FACTORY
    - All other values defined as SNS_SMGR_CAL_XXX style are reserved for
      future use
  */

  /* Mandatory */
  uint8_t SensorId;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */

  /* Mandatory */
  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary.
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
    - All other values defined as SNS_SMGR_DATA_TYPE_XXXX style are reserved
      for future use
      This parameter identifies the sensor data type.
   */

  /* Mandatory */
  uint32_t ZeroBias_len;  /**< Must be set to # of elements in ZeroBias */
  int32_t ZeroBias[SNS_SMGR_SENSOR_DIMENSION_V01];
  /**<  
    The value must be Q16 format (16 bits for integer part, 16 bits for decimal
    part), indicating the zero bias that is to be added (in nominal engineering
    units)
  */

  /* Mandatory */
  uint32_t ScaleFactor_len;  /**< Must be set to # of elements in ScaleFactor */
  uint32_t ScaleFactor[SNS_SMGR_SENSOR_DIMENSION_V01];
  /**<  
    The value must be Q16 format, a multiplier that indicates scale factor need
    to be multiplied to current data .
    For example, enter 1.01 if the scaling is 1% less aggressive or 0.95 if it
    is 5% more aggressive.
  */

  /* Optional */
  uint8_t CompensationMatrix_valid;  /**< Must be set to true if CompensationMatrix is being passed */
  uint32_t CompensationMatrix_len;  /**< Must be set to # of elements in CompensationMatrix */
  int32_t CompensationMatrix[SNS_SMGR_COMPENSATION_MATRIX_SIZE_V01];
  /**<  
    The Compensation Matrix, if present to calibrate sensor data for.
    If the Compensation Matrix is supplied, the ScaleFactor above are ignored.
    The calibrated sample (Sc) is computed as
    Sc = (Sr - Bias)*CM
    where :
        Sc = Calibrated sensor sample
        Sr = Read sensor sample
        CM = Compensation Matrix (from this message)
        Bias = Zero Bias (from this message)

    Matrix elements are in Q16 format in row major order ie:
    CM =  CM0  CM1  CM2
          CM3  CM4  CM5
          CM6  CM7  CM8
  */

  /* Optional */
  uint8_t CalibrationAccuracy_valid;  /**< Must be set to true if CalibrationAccuracy is being passed */
  int32_t CalibrationAccuracy;
  /**<   Calibration Accuracy. The interpretation of this field is
       implementation dependant. A guiding rule though, is that higher
       accuracies are better with 0 meaning complete unreliability.
  */
}sns_smgr_sensor_cal_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command sets dynamic or auto generated calibrate factors of a sensor. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;
}sns_smgr_sensor_cal_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sns_smgr_all_sensor_info_req_msg_v01;

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command sends all sensor info request and get all sensor IDs and
           short sensor names. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  uint32_t SensorInfo_len;  /**< Must be set to # of elements in SensorInfo */
  sns_smgr_sensor_id_info_s_v01 SensorInfo[SNS_SMGR_MAX_SENSOR_NUM_V01];
}sns_smgr_all_sensor_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command sends single sensor info request and gets all the detailed
           information of the sensor */
typedef struct {

  /* Mandatory */
  uint8_t SensorID;
  /**<   see #define SNS_SMGR_ID_XXX_XXX */
}sns_smgr_single_sensor_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command sends single sensor info request and gets all the detailed
           information of the sensor */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  sns_smgr_sensor_info_s_v01 SensorInfo;

  /* Optional */
  uint8_t num_buffered_reports_valid;  /**< Must be set to true if num_buffered_reports is being passed */
  uint32_t num_buffered_reports_len;  /**< Must be set to # of elements in num_buffered_reports */
  uint32_t num_buffered_reports[SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01];
  /**<   The max number of reports that can be buffered for this data type */

  /* Optional */
  uint8_t op_mode_valid;  /**< Must be set to true if op_mode is being passed */
  sns_smgr_op_mode_e_v01 op_mode;
  /**<   Sensor's current operating mode */

  /* Optional */
  uint8_t suid_valid;  /**< Must be set to true if suid is being passed */
  uint32_t suid_len;  /**< Must be set to # of elements in suid */
  uint64_t suid[SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01];
  /**<   64-bit Unique ID for this sensor */

  /* Optional */
  uint8_t supported_odr_list_valid;  /**< Must be set to true if supported_odr_list is being passed */
  uint32_t supported_odr_list_len;  /**< Must be set to # of elements in supported_odr_list */
  sns_smgr_odr_list_s_v01 supported_odr_list[SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01];
  /**<   List of supported ODR; could be empty */
}sns_smgr_single_sensor_info_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sns_smgr_sensor_test_req_msg_v01;

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests all-sensor test.
    These message are defined for future use, so the messages WON'T be supported. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  uint32_t result_len;  /**< Must be set to # of elements in result */
  sns_smgr_sensor_test_result_s_v01 result[SNS_SMGR_MAX_SENSOR_NUM_V01];
}sns_smgr_sensor_test_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command requests single sensor test.
    Currently only single sensor self-test is supported. */
typedef struct {

  /* Mandatory */
  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to.
       The sensor ID is defined as SNS_SMGR_ID_XXXX style.
  */

  /* Mandatory */
  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary.
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
  */

  /* Mandatory */
  sns_smgr_test_type_e_v01 TestType;
  /**<   Defines the type of test to be executed.
    - 00 - SNS_SMGR_TEST_SELF
    - All other values defined as SNS_SMGR_TEST_XXXX style are reserved
      for future use
  */

  /* Optional */
  uint8_t SaveToRegistry_valid;  /**< Must be set to true if SaveToRegistry is being passed */
  uint8_t SaveToRegistry;
  /**<   Specifies whether calibration data generated during the test should be
       saved to sensors registry.
       This applies only to sensors which generate calibration data as part of
       factory test.
       Default behavior is TRUE (save calibration data to sensors registry).
  */

  /* Optional */
  uint8_t ApplyCalNow_valid;  /**< Must be set to true if ApplyCalNow is being passed */
  uint8_t ApplyCalNow;
  /**<   Specifies whether calibration data should take affect immediately, rather
       than after reboot.
       This applies only to sensors which generate calibration data as part of
       factory test.
       Default behavior is TRUE (apply calibration data immediately).
  */
}sns_smgr_single_sensor_test_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests single sensor test.
    Currently only single sensor self-test is supported. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to.
       The sensor ID is defined as SNS_SMGR_ID_XXXX style.
  */

  /* Mandatory */
  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary.
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
  */

  /* Mandatory */
  sns_smgr_test_type_e_v01 TestType;
  /**<   Defines the type of test to be executed.
    - 00 - SNS_SMGR_TEST_SELF
    - All other values are reserved for future use
  */

  /* Mandatory */
  sns_smgr_test_status_e_v01 TestStatus;
  /**<   Identifies test status */
}sns_smgr_single_sensor_test_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Indication Message; This command requests single sensor test.
    Currently only single sensor self-test is supported. */
typedef struct {

  /* Mandatory */
  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to.
       The sensor ID is defined as SNS_SMGR_ID_XXXX style.
  */

  /* Mandatory */
  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
  */

  /* Mandatory */
  sns_smgr_test_type_e_v01 TestType;
  /**<   Defines the type of test to be executed.
    - 00 - SNS_SMGR_TEST_SELF
    - All other values are reserved for future use
  */

  /* Mandatory */
  sns_smgr_test_result_e_v01 TestResult;
  /**<   Indicates test result */

  /* Optional */
  uint8_t ErrorCode_valid;  /**< Must be set to true if ErrorCode is being passed */
  uint8_t ErrorCode;
  /**<   Test-specific error code */
}sns_smgr_single_sensor_test_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command requests sensor power status.
    These message are defined for future use, so the messages WON'T be supported. */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   ID assigned by client to distinguish client's reports */

  /* Mandatory */
  uint8_t Action;
  /**<   SNS_SMGR_POWER_STATUS_ADD =0 ,Add;
       SNS_SMGR_POWER_STATUS_DEL =1, Delete */
}sns_smgr_sensor_power_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests sensor power status.
    These message are defined for future use, so the messages WON'T be supported. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;
}sns_smgr_sensor_power_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Indication Message; This command requests sensor power status.
    These message are defined for future use, so the messages WON'T be supported. */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   ID assigned by client to distinguish client's reports */

  /* Mandatory */
  uint32_t PowerStatus_len;  /**< Must be set to # of elements in PowerStatus */
  sns_smgr_sensor_power_status_s_v01 PowerStatus[SNS_SMGR_MAX_SENSOR_NUM_V01];
}sns_smgr_sensor_power_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command requests sensor power control.
    These message are defined for future use, so the messages WON'T be supported. */
typedef struct {

  /* Mandatory */
  uint8_t SensorID;
  /**<   See #define SNS_SMGR_ID_XXX_XXX; Defines the sensor that this configuration
       pertains to.
  */

  /* Mandatory */
  uint8_t Action;
  /**<   SNS_SMGR_POWER_CTRL_AUTO =0  automatic control (default)
       SNS_SMGR_POWER_CTRL_ACTIVE =1 active state - command the max power state
       SNS_SMGR_POWER_CTRL_IDLE =2   idle state - command to low power state
       SNS_SMGR_POWER_CTRL_OFF =3 = off - not possible in 8660 DSPS
  */
}sns_smgr_sensor_power_control_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests sensor power control.
    These message are defined for future use, so the messages WON'T be supported. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;
}sns_smgr_sensor_power_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command requests sensor status which tells if the sensor is active,
    idle, or only one client is left for the sensor. Currently SMGR only supports
    one client at a time. So This feature is limited. */
typedef struct {

  /* Mandatory */
  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */

  /* Mandatory */
  uint8_t ReqDataTypeNum;
  /**<   How many data types client monitors and requests sampling data, this
       is used by SMGR to tell if there is only one client left.  When the number
       of request items drop to this number, SMGR will send
       SNS_SMGR_SENSOR_STATUS_ONE_CLIENT indication for the sensor */

  /* Mandatory */
  uint8_t Action;
  /**<   SNS_SMGR_SENSOR_STATUS_ADD = 0, Add;
       SNS_SMGR_SENSOR_STATUS_DEL = 1, Delete
       All other values defined as SNS_SMGR_SENSOR_STATUS_XXXX style are reserved
       for future use
      */
}sns_smgr_sensor_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests sensor status which tells if the sensor is active,
    idle, or only one client is left for the sensor. Currently SMGR only supports
    one client at a time. So This feature is limited. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */
}sns_smgr_sensor_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Indication Message; This command requests sensor status which tells if the sensor is active,
    idle, or only one client is left for the sensor. Currently SMGR only supports
    one client at a time. This feature is limited. */
typedef struct {

  /* Mandatory */
  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */

  /* Mandatory */
  uint8_t SensorState;
  /**<   Defines the sensor status for this indication. The status can be one of
    following:
    - 00 - SNS_SMGR_SENSOR_STATUS_UNKNOWN
    - 01 - SNS_SMGR_SENSOR_STATUS_IDLE
    - 02 - SNS_SMGR_SENSOR_STATUS_ACTIVE
    - 03 - SNS_SMGR_SENSOR_STATUS_ONE_CLIENT
    - All other values defined as SNS_SMGR_SENSOR_STATUS_XXXX style are reserved
      for future use
  */

  /* Mandatory */
  uint32_t TimeStamp;
  /**<   The timestamp when state is changed */

  /* Optional */
  uint8_t PerProcToalClients_valid;  /**< Must be set to true if PerProcToalClients is being passed */
  uint32_t PerProcToalClients_len;  /**< Must be set to # of elements in PerProcToalClients */
  uint16_t PerProcToalClients[5];
  /**<   Total clients per processor indexed by the constants defined above */

  /* Optional */
  uint8_t MaxFreqPerProc_valid;  /**< Must be set to true if MaxFreqPerProc is being passed */
  uint32_t MaxFreqPerProc_len;  /**< Must be set to # of elements in MaxFreqPerProc */
  int32_t MaxFreqPerProc[5];
  /**<   Max frequency of data requested by clients on each processor
       Units of Hz, Q16 format
  */

  /* Optional */
  uint8_t MaxUpdateRatePerProc_valid;  /**< Must be set to true if MaxUpdateRatePerProc is being passed */
  uint32_t MaxUpdateRatePerProc_len;  /**< Must be set to # of elements in MaxUpdateRatePerProc */
  int32_t MaxUpdateRatePerProc[5];
  /**<   Max update rate of data requested by clients on each processor
       Units of Hz, Q16 format
  */
}sns_smgr_sensor_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; 
  This command requests sensor data to be sampled and buffered up to be sent together */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   The report ID assigned by client to be used for identifying corresponding
    response and indication messages
    */

  /* Mandatory */
  uint8_t Action;
  /**<   Specifies the action to be carried out for this report
    - SNS_SMGR_BUFFERING_ACTION_ADD
    - SNS_SMGR_BUFFERING_ACTION_DELETE
    - All other values will be rejected.
    An existing report will be replaced by a new report of the same ID.
    This includes Periodic Report.  It is advisable for clients to use different
    sets of IDs for Buffering reports and Periodic reports.
    */

  /* Mandatory */
  uint32_t ReportRate;
  /**<   Specifies the desired reporting rate expressed in Q16 format and in unit of Hz.
    This is only meaningful when paired with SNS_SMGR_BUFFERING_ACTION_ADD
    To indicate no periodic reports, use SNS_SMGR_BUFFERING_REPORT_RATE_NONE.
  */

  /* Mandatory */
  uint32_t Item_len;  /**< Must be set to # of elements in Item */
  sns_smgr_buffering_req_item_s_v01 Item[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];
  /**<   The items to be buffered.

       Note: It is NOT recommended to request a set of items with sampling
             rates less than 1 Hz and sampling rates above 1 Hz. Rather, it is
             recommended to request items with a sampling rate less than 1 Hz
             separately, in its own request. The reasoning behind this
             suggestion is to prevent large amounts of back-to-back QMI
             indications from coming in. Reports that have items with sub-hz
             (less than 1 Hz) sampling rates are forced to send each sample
             individually in back-to-back indications. (See TimeStampOffset in
             the sns_smgr_buffering_sample_s structure) (See Samples in the
             sns_smgr_buffering_ind_msg message)
  */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type                  = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend  = FALSE
    */

  /* Optional */
  uint8_t SrcModule_valid;  /**< Must be set to true if SrcModule is being passed */
  uint8_t SrcModule;
  /**<   For sensor internal use only.
       Defines the source module that is sending this message.
    */
}sns_smgr_buffering_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; 
  This command requests sensor data to be sampled and buffered up to be sent together */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Optional */
  uint8_t ReportId_valid;  /**< Must be set to true if ReportId is being passed */
  uint8_t ReportId;
  /**<   The ID corresponding to a Buffering request */

  /* Optional */
  uint8_t AckNak_valid;  /**< Must be set to true if AckNak is being passed */
  uint8_t AckNak;
  /**<   Defines whether this response is Acknowledgement or Negative Acknowledgement
    - SNS_SMGR_RESPONSE_ACK_SUCCESS - the request has been accepted
    - SNS_SMGR_RESPONSE_ACK_MODIFIED - some parameters in the request are modified
    - SNS_SMGR_RESPONSE_NAK_RESOURCES - no resources to service the request
    - SNS_SMGR_RESPONSE_NAK_REPORT_ID - no such report to be deleted
    - SNS_SMGR_RESPONSE_NAK_NO_ITEMS - no valid items were sent in request
    - SNS_SMGR_RESPONSE_NAK_UNK_ACTION - invalid Action field in request
    - SNS_SMGR_RESPONSE_NAK_INTERNAL_ERR - unspecified error
  */

  /* Optional */
  uint8_t ReasonPair_valid;  /**< Must be set to true if ReasonPair is being passed */
  uint32_t ReasonPair_len;  /**< Must be set to # of elements in ReasonPair */
  sns_smgr_reason_pair_s_v01 ReasonPair[SNS_SMGR_MAX_NUM_REASONS_V01];
}sns_smgr_buffering_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Request Message; This command allows the client to request sensor samples from
    the SMGR current buffer. Often combined with requesting a buffering
    report with the report rate set to SNS_SMGR_BUFFERING_REPORT_RATE_NONE. */
typedef struct {

  /* Mandatory */
  uint16_t QueryId;
  /**<   The ID corresponding to a Buffering request
    The lower 8-bit value is the ReportId of the Buffering request initiated
    by same client
    The upper 8-bit value is the transaction ID assigned by client for each query
    Query response and indications for this request shall carry this QueryId
    */

  /* Mandatory */
  uint8_t SensorId;
  /**<   Identifies the sensor from which to collect data. */

  /* Mandatory */
  uint8_t DataType;
  /**<   Identifies the data type of the specified sensor */

  /* Mandatory */
  uint32_t TimePeriod[2];
  /**<   Specify the start and end of the time period within which to collect samples.
    - TimePeriod[0] is timestamp of the start of the time period
    - TimePeriod[1] is timestamp of the end of the time period
    */

  /* Optional */
  uint8_t SrcModule_valid;  /**< Must be set to true if SrcModule is being passed */
  uint8_t SrcModule;
  /**<   For sensor internal use only.
       Defines the source module that is sending this message.
    */
}sns_smgr_buffering_query_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Response Message; This command allows the client to request sensor samples from
    the SMGR current buffer. Often combined with requesting a buffering
    report with the report rate set to SNS_SMGR_BUFFERING_REPORT_RATE_NONE. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Optional */
  uint8_t QueryId_valid;  /**< Must be set to true if QueryId is being passed */
  uint16_t QueryId;
  /**<   The ID corresponding to a Query request */

  /* Optional */
  uint8_t AckNak_valid;  /**< Must be set to true if AckNak is being passed */
  uint8_t AckNak;
  /**<   Defines whether this response is Acknowledgement or Negative Acknowledgement
    - SNS_SMGR_RESPONSE_ACK_SUCCESS - the request has been accepted
    - SNS_SMGR_RESPONSE_NAK_RESOURCES - no resources to service the request
    - SNS_SMGR_RESPONSE_NAK_REPORT_ID - report not found for given ID
    - SNS_SMGR_RESPONSE_NAK_QUERY_ID - same request already received
    - SNS_SMGR_RESPONSE_NAK_TIME_PERIOD - the start of time period is not greater
        than end of time periodic
    - SNS_SMGR_RESPONSE_NAK_SENSOR_ID - requested sensor ID/data type is not in
        Buffering request
    - SNS_SMGR_RESPONSE_NAK_INTERNAL_ERR - unspecified error
  */
}sns_smgr_buffering_query_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_SVC_qmi_messages
    @{
  */
/** Indication Message; Sensor Buffering Report Message sent to a client at the reporting rate. */
typedef struct {

  /* Mandatory */
  uint16_t QueryId;
  /**<   The ID corresponding to a Query request */

  /* Mandatory */
  uint32_t FirstSampleTimestamp;
  /**<   Timestamp of first sample belonging to this SensorId/DataType pair.
    */

  /* Mandatory */
  uint32_t SamplingRate;
  /**<   Specifies the actual frequency at which requested sensor is sampled.
       This value is expressed in Q16 format and in unit of Hz. */

  /* Mandatory */
  uint32_t Samples_len;  /**< Must be set to # of elements in Samples */
  sns_smgr_buffering_sample_s_v01 Samples[SNS_SMGR_BUFFERING_REPORT_MAX_SAMPLES_V01];
  /**<   Samples collected within requested time period */
}sns_smgr_buffering_query_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_COMMON_CANCEL_V01 
//#define REMOVE_SNS_COMMON_VERSION_V01 
//#define REMOVE_SNS_SMGR_ALL_SENSOR_INFO_V01 
//#define REMOVE_SNS_SMGR_BUFFERING_V01 
//#define REMOVE_SNS_SMGR_BUFFERING_IND_V01 
//#define REMOVE_SNS_SMGR_BUFFERING_QUERY_V01 
//#define REMOVE_SNS_SMGR_BUFFERING_QUERY_IND_V01 
//#define REMOVE_SNS_SMGR_CAL_V01 
//#define REMOVE_SNS_SMGR_REPORT_V01 
//#define REMOVE_SNS_SMGR_REPORT_IND_V01 
//#define REMOVE_SNS_SMGR_SENSOR_POWER_CONTROL_V01 
//#define REMOVE_SNS_SMGR_SENSOR_POWER_STATUS_V01 
//#define REMOVE_SNS_SMGR_SENSOR_POWER_STATUS_IND_V01 
//#define REMOVE_SNS_SMGR_SENSOR_STATUS_V01 
//#define REMOVE_SNS_SMGR_SENSOR_STATUS_IND_V01 
//#define REMOVE_SNS_SMGR_SENSOR_TEST_V01 
//#define REMOVE_SNS_SMGR_SINGLE_SENSOR_INFO_V01 
//#define REMOVE_SNS_SMGR_SINGLE_SENSOR_TEST_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SMGR_SVC_qmi_msg_ids
    @{
  */
#define SNS_SMGR_CANCEL_REQ_V01 0x0000
#define SNS_SMGR_CANCEL_RESP_V01 0x0000
#define SNS_SMGR_VERSION_REQ_V01 0x0001
#define SNS_SMGR_VERSION_RESP_V01 0x0001
#define SNS_SMGR_REPORT_REQ_V01 0x0002
#define SNS_SMGR_REPORT_RESP_V01 0x0002
#define SNS_SMGR_REPORT_IND_V01 0x0003
#define SNS_SMGR_CAL_REQ_V01 0x0004
#define SNS_SMGR_CAL_RESP_V01 0x0004
#define SNS_SMGR_ALL_SENSOR_INFO_REQ_V01 0x0005
#define SNS_SMGR_ALL_SENSOR_INFO_RESP_V01 0x0005
#define SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01 0x0006
#define SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01 0x0006
#define SNS_SMGR_SENSOR_TEST_REQ_V01 0x0007
#define SNS_SMGR_SENSOR_TEST_RESP_V01 0x0007
#define SNS_SMGR_SENSOR_POWER_STATUS_REQ_V01 0x0008
#define SNS_SMGR_SENSOR_POWER_STATUS_RESP_V01 0x0008
#define SNS_SMGR_SENSOR_POWER_STATUS_IND_V01 0x0009
#define SNS_SMGR_SENSOR_POWER_CONTROL_REQ_V01 0x000A
#define SNS_SMGR_SENSOR_POWER_CONTROL_RESP_V01 0x000A
#define SNS_SMGR_SENSOR_STATUS_REQ_V01 0x000B
#define SNS_SMGR_SENSOR_STATUS_RESP_V01 0x000B
#define SNS_SMGR_SENSOR_STATUS_IND_V01 0x000C
#define SNS_SMGR_SINGLE_SENSOR_TEST_REQ_V01 0x000D
#define SNS_SMGR_SINGLE_SENSOR_TEST_RESP_V01 0x000D
#define SNS_SMGR_SINGLE_SENSOR_TEST_IND_V01 0x000D
#define SNS_SMGR_BUFFERING_REQ_V01 0x0021
#define SNS_SMGR_BUFFERING_RESP_V01 0x0021
#define SNS_SMGR_BUFFERING_IND_V01 0x0022
#define SNS_SMGR_BUFFERING_QUERY_REQ_V01 0x0023
#define SNS_SMGR_BUFFERING_QUERY_RESP_V01 0x0023
#define SNS_SMGR_BUFFERING_QUERY_IND_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SMGR_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SMGR_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SMGR_SVC_get_service_object_v01( ) \
          SNS_SMGR_SVC_get_service_object_internal_v01( \
            SNS_SMGR_SVC_V01_IDL_MAJOR_VERS, SNS_SMGR_SVC_V01_IDL_MINOR_VERS, \
            SNS_SMGR_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

