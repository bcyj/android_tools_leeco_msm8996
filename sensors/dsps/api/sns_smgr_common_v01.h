#ifndef SNS_SMGR_COMMON_SERVICE_01_H
#define SNS_SMGR_COMMON_SERVICE_01_H
/**
  @file sns_smgr_common_v01.h

  @brief This is the public header file which defines the sns_smgr_common service Data structures.

  This header file defines the types and structures that were defined in
  sns_smgr_common. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.10 
   It was generated on: Mon Jul 21 2014 (Spin 0)
   From IDL File: sns_smgr_common_v01.idl */

/** @defgroup sns_smgr_common_qmi_consts Constant values defined in the IDL */
/** @defgroup sns_smgr_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sns_smgr_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sns_smgr_common_qmi_messages Structures sent as QMI messages */
/** @defgroup sns_smgr_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sns_smgr_common_qmi_accessor Accessor for QMI service object */
/** @defgroup sns_smgr_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sns_smgr_common_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SMGR_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SMGR_COMMON_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SMGR_COMMON_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */


/** @addtogroup sns_smgr_common_qmi_consts 
    @{ 
  */

/**  Response message ACK/NAK codes
  

 The enum are 32 bits for reference only, in actual message, we use 8 bits
       only to save space  */
#define SNS_SMGR_RESPONSE_ACK_SUCCESS_V01 0

/**  See reason codes  */
#define SNS_SMGR_RESPONSE_ACK_MODIFIED_V01 1

/**  Lack table space  */
#define SNS_SMGR_RESPONSE_NAK_RESOURCES_V01 2

/**  Can't find report for delete  */
#define SNS_SMGR_RESPONSE_NAK_REPORT_ID_V01 3

/**  None supplied or modified away  */
#define SNS_SMGR_RESPONSE_NAK_NO_ITEMS_V01 4

/**  Invalid action field  */
#define SNS_SMGR_RESPONSE_NAK_UNK_ACTION_V01 5

/**  Report rate is unsupportable  */
#define SNS_SMGR_RESPONSE_NAK_REPORT_RATE_V01 6

/**  Time period in Query request is unsupportable  */
#define SNS_SMGR_RESPONSE_NAK_TIME_PERIOD_V01 7

/**  Unspecified internal errors  */
#define SNS_SMGR_RESPONSE_NAK_INTERNAL_ERR_V01 8

/**  Query request with given QueryID already received  */
#define SNS_SMGR_RESPONSE_NAK_QUERY_ID_V01 9

/**  Sensor specified in Query request was not in Buffering request  */
#define SNS_SMGR_RESPONSE_NAK_SENSOR_ID_V01 10

/**  Reason codes for substituting a default or deleting an item.  */
#define SNS_SMGR_REASON_NULL_V01 0

/**  Rate set to 20 Hz or maximum supported rate, whichever is lower  */
#define SNS_SMGR_REASON_DEFAULT_RATE_V01 10

/**  Type set to engineering units  */
#define SNS_SMGR_REASON_DEFAULT_TYPE_V01 11

/**  Decimation set to latest sample  */
#define SNS_SMGR_REASON_DEFAULT_DECIM_V01 12

/**  Sensitivity code set to 0  */
#define SNS_SMGR_REASON_DEFAULT_STIVTY_V01 13
#define SNS_SMGR_REASON_DEFAULT_FINAL_V01 14

/**  Item deleted  */
#define SNS_SMGR_REASON_UNKNOWN_SENSOR_V01 15

/**  Item deleted  */
#define SNS_SMGR_REASON_FAILED_SENSOR_V01 16

/**  Item deleted  */
#define SNS_SMGR_REASON_OTHER_FAILURE_V01 17

/**  Sampling rate is unsupportable. Item deleted  */
#define SNS_SMGR_REASON_SAMPLING_RATE_V01 18

/**  Item modified  */
#define SNS_SMGR_REASON_SAMPLE_QUALITY_NORMAL_V01 19

/**  Status of the report, if OK, or Canceled
   */
#define SNS_SMGR_REPORT_OK_V01 0

/**  Lack table space  */
#define SNS_SMGR_REPORT_CANCEL_RESOURCE_V01 1

/**  All req sensors have failed  */
#define SNS_SMGR_REPORT_CANCEL_FAILURE_V01 2

/**  Server shut down  */
#define SNS_SMGR_REPORT_CANCEL_SHUT_DOWN_V01 3

/**  Sensor Request Activity. Delete or add. Add may act as replace if the same
    client ID and report ID are found in the SOL
   */
#define SNS_SMGR_REPORT_ACTION_ADD_V01 1
#define SNS_SMGR_REPORT_ACTION_DELETE_V01 2

/**  The Decimation word specifies how to reduce oversampled data.
    Report most recent sample (Default)
    Average samples since previous report
    Filter at half the reporting rate or next lower available frequency
    sns_smgr_decimation_t
  

 Unfiltered, possibly interpolated and/or calibrated  */
#define SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01 1
#define SNS_SMGR_DECIMATION_AVERAGE_V01 2
#define SNS_SMGR_DECIMATION_FILTER_V01 3

/**  =============== for sensor report message ===============
 Flag bit values in the Flag word associated with each data item in a sensor
    report.  The first 3 values correspond to the first 3 data words which are
    related with XYZ axes of those sensors that have 3 axis measurements.  For
    sensors that are not axis oriented, these flags correspond to the first
    three data words.  If any of the first 3 flags is non-zero, it indicates
    that the corresponding data word was found at the extreme edge of its valid
    range; this indicates that the sensor was railed.
    The 4th flag, when non-zero, indicates that the data item is invalid. The
    Quality word indicates why the item is invalid.
    sns_smgr_item_flags_t
   */
#define SNS_SMGR_ITEM_FLAG_X_RAIL_V01 1
#define SNS_SMGR_ITEM_FLAG_Y_RAIL_V01 2
#define SNS_SMGR_ITEM_FLAG_Z_RAIL_V01 4
#define SNS_SMGR_ITEM_FLAG_INVALID_V01 8
#define SNS_SMGR_ITEM_FLAG_FAC_CAL_V01 16
#define SNS_SMGR_ITEM_FLAG_AUTO_CAL_V01 32

/**  The Quality word is associated with each data item in a sensor report. It
    is a code defining the quality of the measurement.
  

 Unfiltered sample; available when client requests
       SNS_SMGR_CAL_SEL_RAW, and
       SNS_SMGR_DECIMATION_RECENT_SAMPLE, and
       SNS_SMGR_SAMPLE_QUALITY_ACCURATE_TIMESTAMP  */
#define SNS_SMGR_ITEM_QUALITY_CURRENT_SAMPLE_V01 0

/**  Sensor missed sampling schedule  */
#define SNS_SMGR_ITEM_QUALITY_PRIOR_VALUE_LATE_V01 1

/**  Client specified this for no motion  */
#define SNS_SMGR_ITEM_QUALITY_PRIOR_VALUE_SUSPENDED_V01 2

/**  Client specified averaging  */
#define SNS_SMGR_ITEM_QUALITY_AVERAGED_SPECIFIED_V01 3

/**  Average substituted while filter starting  */
#define SNS_SMGR_ITEM_QUALITY_AVERAGED_FILTER_START_V01 4

/**  This value is used if SNS_SMGR_DECIMATION_FILTER option was used in the
       report request  */
#define SNS_SMGR_ITEM_QUALITY_FILTERED_V01 5

/**  Sample not available due to failed sensor  */
#define SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01 10

/**  Sensor starting  */
#define SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01 11

/**  Not in motion  */
#define SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01 12

/**  Sample is the result of interpolation  */
#define SNS_SMGR_ITEM_QUALITY_INTERPOLATED_V01 13

/**  Sample is the result of interpolation and CIC filtering  */
#define SNS_SMGR_ITEM_QUALITY_INTERPOLATED_FILTERED_V01 14

/**  Define sensor identifier.

    The following table illustrates what the data types correspond to.
    For example, primary sensor data type of SNS_SMGR_ID_PROX_LIGHT is
    proximity sensor and the secondary one is ambient light sensor.

    ID        PRIMARY         SECONDARY
    --------  --------------  --------------
    0         Accelerometer   Temperature
    10        Gyro            Temperature
    20        Magnetometer    Temperature
    30        Pressure        Temperature
    40        Proximity       Ambient light
    50        Humidity        Temperature
    60        RGB             Color Temperature/Clear
    70        SAR             Specific Absorption Rate
    80        HallEffect      none
    90        HeartRate       Raw data(Optional)
    220       Step            none
    222       Step Count      none
    224       SMD             none
    226       GameRV          none
    228       IR Gesture      Proximity
    230       Double-tap      Single-tap
    240-249   OEM-defined     OEM-defined


 This is the primary acceleration sensor ID , namely this is
       the acceleration ID if there is only one acceleration sensor or the first
       acceleration sensor ID if there are multiple acceleration sensors  */
#define SNS_SMGR_ID_ACCEL_V01 0
#define SNS_SMGR_ID_ACCEL_2_V01 1
#define SNS_SMGR_ID_ACCEL_3_V01 2
#define SNS_SMGR_ID_ACCEL_4_V01 3
#define SNS_SMGR_ID_ACCEL_5_V01 4

/**  This is primary gyro sensor ID  */
#define SNS_SMGR_ID_GYRO_V01 10
#define SNS_SMGR_ID_GYRO_2_V01 11
#define SNS_SMGR_ID_GYRO_3_V01 12
#define SNS_SMGR_ID_GYRO_4_V01 13
#define SNS_SMGR_ID_GYRO_5_V01 14

/**  This is primary mag sensor ID  */
#define SNS_SMGR_ID_MAG_V01 20

/**  This is primary pressure sensor ID  */
#define SNS_SMGR_ID_PRESSURE_V01 30

/**  This is primary proxy light sensor ID */
#define SNS_SMGR_ID_PROX_LIGHT_V01 40

/**  This is primary humidity sensor ID */
#define SNS_SMGR_ID_HUMIDITY_V01 50

/**  Primary = RGB, Secondary = Color temperature and clear component of RGB */
#define SNS_SMGR_ID_RGB_V01 60
#define SNS_SMGR_ID_RGB_2_V01 61

/**  Primary = SAR, Secondary = none */
#define SNS_SMGR_ID_SAR_V01 70
#define SNS_SMGR_ID_SAR_2_V01 71

/**  Primary = HallEffect, Secondary = none */
#define SNS_SMGR_ID_HALL_EFFECT_V01 80

/**  Primary = HeartRate, Secondary = Raw Data(Optional) */
#define SNS_SMGR_ID_HEART_RATE_V01 90

/**  Embedded sensor: Primary = Step Detection, Secondary = (none)  */
#define SNS_SMGR_ID_STEP_EVENT_V01 220

/**  Embedded sensor: Primary = Step Count, Secondary = (none)  */
#define SNS_SMGR_ID_STEP_COUNT_V01 222

/**  Embedded sensor: Primary = SMD, Secondary = (none)  */
#define SNS_SMGR_ID_SMD_V01 224

/**  Embedded sensor: Primary = Game Rotation Vector, Secondary = (none)  */
#define SNS_SMGR_ID_GAME_ROTATION_VECTOR_V01 226

/**  Embedded sensor: Primary = IR_GESTURE, Secondary = PROXIMITY   */
#define SNS_SMGR_ID_IR_GESTURE_V01 228

/**  Embedded sensor: Primary = Double-tap, Secondary = Single-tap  */
#define SNS_SMGR_ID_TAP_V01 230

/**  Sensor IDs for custom sensor types  */
#define SNS_SMGR_ID_OEM_SENSOR_01_V01 240
#define SNS_SMGR_ID_OEM_SENSOR_02_V01 241
#define SNS_SMGR_ID_OEM_SENSOR_03_V01 242
#define SNS_SMGR_ID_OEM_SENSOR_04_V01 243
#define SNS_SMGR_ID_OEM_SENSOR_05_V01 244
#define SNS_SMGR_ID_OEM_SENSOR_06_V01 245
#define SNS_SMGR_ID_OEM_SENSOR_07_V01 246
#define SNS_SMGR_ID_OEM_SENSOR_08_V01 247
#define SNS_SMGR_ID_OEM_SENSOR_09_V01 248
#define SNS_SMGR_ID_OEM_SENSOR_10_V01 249

/**  Sensor Unique IDs 

 This is the primary acceleration sensor SUID , namely this is
       the acceleration sensor SUID if there is only one acceleration sensor or the first
       acceleration sensor SUID if there are multiple acceleration sensors  */
#define SNS_SMGR_SUID_ACCEL_1_V01 0x4b118d0883777081
#define SNS_SMGR_SUID_ACCEL_2_V01 0x3588d0b2ffffff28
#define SNS_SMGR_SUID_ACCEL_3_V01 0x02fd528Cfe45fff8
#define SNS_SMGR_SUID_ACCEL_4_V01 0x5e2b5fa5ffffffD0
#define SNS_SMGR_SUID_ACCEL_5_V01 0x7090c43580000110

/**  This is primary Gyro sensor SUID  */
#define SNS_SMGR_SUID_GYRO_1_V01 0x35bcdb5f5574ab55
#define SNS_SMGR_SUID_GYRO_2_V01 0xfb40c7220c002008
#define SNS_SMGR_SUID_GYRO_3_V01 0x0a0852e2f582cbe0
#define SNS_SMGR_SUID_GYRO_4_V01 0x27651f7000000020
#define SNS_SMGR_SUID_GYRO_5_V01 0x16c113b169b8397c
#define SNS_SMGR_SUID_MAG_1_V01 0x3ff4eb5fde2790a3
#define SNS_SMGR_SUID_PRESSURE_1_V01 0x4bb62314f99bc876

/**  TEMP Sensor at ACCEL_1. Below cases are similar */
#define SNS_SMGR_SUID_ACCEL_TEMP_1_V01 0x1f29b4835607d1c2
#define SNS_SMGR_SUID_GYRO_TEMP_1_V01 0xdd7ed2528a66bb1c
#define SNS_SMGR_SUID_MAG_TEMP_1_V01 0xd42b24247c1e0f8f
#define SNS_SMGR_SUID_PRESSURE_TEMP_1_V01 0x66384f88e1f7a7a6
#define SNS_SMGR_SUID_AMBIENT_TEMP_1_V01 0xa2e3a4313bbb1ea6
#define SNS_SMGR_SUID_PROX_1_V01 0x1653b115d84ecf43
#define SNS_SMGR_SUID_LIGH_1_V01 0x05699579954d05da
#define SNS_SMGR_SUID_HUMIDITY_1_V01 0x568f71c1a8b35b74

/**  This is primary RGB sensor SUID  */
#define SNS_SMGR_SUID_RGB_1_V01 0xf27304ad2058987c
#define SNS_SMGR_SUID_RGB_2_V01 0x54cd45be0090c070

/**  This is primary SAR sensor SUID  */
#define SNS_SMGR_SUID_SAR_1_V01 0xf8bcccca581e74d3
#define SNS_SMGR_SUID_SAR_2_V01 0x26b5860985e2f908
#define SNS_SMGR_SUID_HALL_EFFECT_1_V01 0x544bc2f51529a489
#define SNS_SMGR_SUID_STEP_EVENT_1_V01 0x42940f9fca6ca718
#define SNS_SMGR_SUID_STEP_COUNT_1_V01 0xf6209794bc3c95a4
#define SNS_SMGR_SUID_SMD_1_V01 0xc0881938994b7e4c
#define SNS_SMGR_SUID_GRV_1_V01 0x615fed6621d30a57
#define SNS_SMGR_SUID_IR_GESTURE_1_V01 0xee10aeb33f4f1b95
#define SNS_SMGR_SUID_DOUBLE_TAP_1_V01 0x81c55ae5c11cf11d
#define SNS_SMGR_SUID_SINGLE_TAP_1_V01 0xe7153e8eff55dc10
#define SNS_SMGR_SUID_COLORTEMP_CLEAR_1_V01 0x852532f403c8bb30
#define SNS_SMGR_SUID_HEART_RATE_1_V01 0x8c1ab1a53989e04b

/**  Select the sensor data type that should be reported from the sensor.
    Primary data type for that sensor.
    Some sensors can also report secondary data Type, this could be expanded.
   */
#define SNS_SMGR_DATA_TYPE_PRIMARY_V01 0

/**  Sensor data types, described in subsequent comments  */
#define SNS_SMGR_DATA_TYPE_SECONDARY_V01 1

/**  Identify the sensor test type.
  
 Status of sensor test, used in sns_smgr_single_sensor_test_resp_msg
  
 Result of sensor test, used in sns_smgr_single_sensor_test_ind_msg
  
 Identify sensor's operating mode
  
 Select the option for how reports will be generated when the unit is
   stationary (not moving). This is a power saving feature. The goal is to
   suspend sampling and/or reporting while the unit is at rest.

   If all items of a report vote for NO_REPORT, that report is suspended
   until motion resumes. However, if another item of this report votes to
   continue reporting, this item is effectively promoted to REPORT_PRIOR.

   REPORT_PRIOR votes to continue generating the report, but suspend sampling
   the sensor named by this item, that is, keep reporting the last available
   sample. However, if some other report votes to keep this sensor sampling,
   then the prior sample continues to be updated.

   REPORT_FULL votes to continue sampling this sensor and generating this
   report.

   REPORT_INTERIM votes to sample and report when hardware indicates motion
   but the motion detection algorithm has not yet confirmed it.
   */
#define SNS_SMGR_REST_OPTION_NO_REPORT_V01 0
#define SNS_SMGR_REST_OPTION_REPORT_PRIOR_V01 1
#define SNS_SMGR_REST_OPTION_REPORT_FULL_V01 2
#define SNS_SMGR_REST_OPTION_REPORT_INTERIM_V01 3

/**  The selection of calibration for sampled data in sensor report request message.
  

 Raw sensor data + factory calibration(if available) + auto calibration
       (if available)  */
#define SNS_SMGR_CAL_SEL_FULL_CAL_V01 0

/**  Raw sensor data + factory calibration(if available)  */
#define SNS_SMGR_CAL_SEL_FACTORY_CAL_V01 1

/**  Un-calibrated, possibly interpolated, sensor data.  */
#define SNS_SMGR_CAL_SEL_RAW_V01 2

/**  =============== Other Constants ===============

 Maximum number of sensor values in one report  */
#define SNS_SMGR_MAX_ITEMS_PER_REPORT_V01 10

/**  Limit number of reason codes in a response message 
 Valid report rate request and default. Rate request may be expressed as Hz
     or msec interval  */
#define SNS_SMGR_MAX_NUM_REASONS_V01 10
#define SNS_SMGR_REPORT_RATE_MIN_HZ_V01 1
#define SNS_SMGR_REPORT_RATE_MAX_HZ_V01 500

/**  Equivalent to 0.5 Hz  */
#define SNS_SMGR_REPORT_RATE_MIN_MSEC_V01 2000

/**  1 minute interval  */
#define SNS_SMGR_REPORT_RATE_MAX_MSEC_V01 60000

/**  Use default if in neither range  */
#define SNS_SMGR_REPORT_RATE_DEFAULT_V01 20

/**  X,y,z axis for primary datatype
       temperature for secondary datatype
       others are reserved fields  */
#define SNS_SMGR_SENSOR_DIMENSION_V01 3

/**  Size of the compensation matrix (in this case a 3x3 matrix)  */
#define SNS_SMGR_COMPENSATION_MATRIX_SIZE_V01 9

/**  Maximum sensor numbers  */
#define SNS_SMGR_MAX_SENSOR_NUM_V01 20

/**  Maximum number of bytes to store a sensor name  */
#define SNS_SMGR_MAX_SENSOR_NAME_SIZE_V01 80

/**  Maximum number of bytes to store a sensor name  */
#define SNS_SMGR_MAX_VENDOR_NAME_SIZE_V01 20

/**  Number of bytes to store a short sensor name  */
#define SNS_SMGR_SHORT_SENSOR_NAME_SIZE_V01 16

/**  Maximum data type per sensor, depending on the sensor  */
#define SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01 3

/**  Apply the dynamic calibration data for this report  */
#define SNS_SMGR_CAL_APPLY_V01 0

/**  Apply the dynamic calibration data for this report  */
#define SNS_SMGR_CAL_DYNAMIC_V01 SNS_SMGR_CAL_APPLY_V01

/**  This definition is defined for future use, and shall not be used until
       announce to be used  */
#define SNS_SMGR_CAL_SAVE_V01 1

/**  Apply the factory calibration data for this report  */
#define SNS_SMGR_CAL_FACTORY_V01 2

/**  To add report in sns_smgr_sensor_power_status_req_msg  */
#define SNS_SMGR_POWER_STATUS_ADD_V01 0

/**  To delete report in sns_smgr_sensor_power_status_req_msg  */
#define SNS_SMGR_POWER_STATUS_DEL_V01 1

/**  No change since last status report sns_smgr_sensor_power_status_s  */
#define SNS_SMGR_POWER_STATE_NO_CHANGE_V01 0

/**  Sensor went active since last report in sns_smgr_sensor_power_status_s  */
#define SNS_SMGR_POWER_STATE_GO_ACTIVE_V01 1

/**  Sensor went low power since last report in sns_smgr_sensor_power_status_s  */
#define SNS_SMGR_POWER_STATE_GO_LOW_POWER_V01 2

/**  Sensor cycled through active and low power since last report in
       sns_smgr_sensor_power_status_s  */
#define SNS_SMGR_POWER_STATE_CYCLE_ACTIVE_AND_LOW_V01 3

/**  Automatic control (default) in sns_smgr_sensor_power_control_req_msg  */
#define SNS_SMGR_POWER_CTRL_AUTO_V01 0

/**  Active state - command the max power state in
       sns_smgr_sensor_power_control_req_msg  */
#define SNS_SMGR_POWER_CTRL_ACTIVE_V01 1

/**  Idle state - command the low power state in
       sns_smgr_sensor_power_control_req_msg  */
#define SNS_SMGR_POWER_CTRL_IDLE_V01 2

/**  Off state - not possible in 8660 DSPS in sns_smgr_sensor_power_control_req_msg  */
#define SNS_SMGR_POWER_CTRL_OFF_V01 3

/**  To add report in sns_smgr_sensor_status_req_msg  */
#define SNS_SMGR_SENSOR_STATUS_ADD_V01 0

/**  To delete report in sns_smgr_sensor_status_req_msg  */
#define SNS_SMGR_SENSOR_STATUS_DEL_V01 1

/**  Define sensor state used in sns_smgr_sensor_status_ind_msg  */
#define SNS_SMGR_SENSOR_STATUS_UNKNOWN_V01 0
#define SNS_SMGR_SENSOR_STATUS_IDLE_V01 1
#define SNS_SMGR_SENSOR_STATUS_ACTIVE_V01 2
#define SNS_SMGR_SENSOR_STATUS_ONE_CLIENT_V01 3

/**  Maximum num of ODRs sensor could support  */
#define SNS_SMGR_MAX_SUPPORTED_ODR_NUM_V01 100

/**  Bit values for sample quality field in sensor report requests  */
#define SNS_SMGR_SAMPLE_QUALITY_ACCURATE_TIMESTAMP_V01 1

/**  =============== Buffering Specific Constants ===============
 Defines the actions used in buffering requests  */
#define SNS_SMGR_BUFFERING_ACTION_ADD_V01 1
#define SNS_SMGR_BUFFERING_ACTION_DELETE_V01 2

/**  Maximum number of sensor/data type pairs in one Buffering request  */
#define SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01 5

/**  Maximum number of samples in one Buffering report  */
#define SNS_SMGR_BUFFERING_REPORT_MAX_SAMPLES_V01 100

/**  Buffering reports sent only when queried  */
#define SNS_SMGR_BUFFERING_REPORT_RATE_NONE_V01 0

/**  =============== Sampling Rate Specific Constants ===============

 Requested sampling rates above this value (non-inclusive) will be
       interpreted as a desired period in milliseconds.
       IMPORTANT! This inversion point is ONLY VALID for the following fields:
       - sns_smgr_periodic_report_req_msg -> ReportRate
       - sns_smgr_periodic_report_ind_msg -> CurrentRate
       - sns_smgr_buffering_req_item_s -> SamplingRate
   */
#define SNS_SMGR_SAMPLING_RATE_INVERSION_POINT_V01 1000
#define SNS_SMGR_SAMPLE_RATE_MIN_PERIOD_MSEC_V01 60000

/**  We allow status updates for sensor status by processor, the following
    constants define the processor mappings  

 Clients on the dedicated sensors processor (DSPS/ADSP)  */
#define SNS_SMGR_DSPS_CLIENTS_V01 0

/**  Clients on the processor running the HLOS  */
#define SNS_SMGR_APPS_CLIENTS_V01 1

/**  Clients on the modem processor  */
#define SNS_SMGR_MODEM_CLIENT_V01 2
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_TEST_TYPE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_TEST_SELF_V01 = 0, 
  SNS_SMGR_TEST_IRQ_V01 = 1, 
  SNS_SMGR_TEST_CONNECTIVITY_V01 = 2, 
  SNS_SMGR_TEST_SELF_HW_V01 = 3, 
  SNS_SMGR_TEST_SELF_SW_V01 = 4, 
  SNS_SMGR_TEST_OEM_V01 = 5, 
  SNS_SMGR_TEST_TYPE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_test_type_e_v01;
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_TEST_STATUS_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_TEST_STATUS_SUCCESS_V01 = 0, 
  SNS_SMGR_TEST_STATUS_PENDING_V01 = 1, 
  SNS_SMGR_TEST_STATUS_DEVICE_BUSY_V01 = 2, /**<  Device is busy streaming  */
  SNS_SMGR_TEST_STATUS_INVALID_TEST_V01 = 3, /**<  Test case is invalid/undefined  */
  SNS_SMGR_TEST_STATUS_INVALID_PARAM_V01 = 4, /**<  Test parameter is invalid  */
  SNS_SMGR_TEST_STATUS_FAIL_V01 = 5, /**<  Unspecified error  */
  SNS_SMGR_TEST_STATUS_BUSY_TESTING_V01 = 6, /**<  Another test is running; try later  */
  SNS_SMGR_TEST_STATUS_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_test_status_e_v01;
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_TEST_RESULT_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_TEST_RESULT_PASS_V01 = 0, 
  SNS_SMGR_TEST_RESULT_FAIL_V01 = 1, 
  SNS_SMGR_TEST_RESULT_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_test_result_e_v01;
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_OP_MODE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_OP_MODE_POLLING_V01 = 0, /**<  sensor is periodically polled by SMGR */
  SNS_SMGR_OP_MODE_DRI_V01 = 1, /**<  sensor HW produces sample at one of the
                                   rates the sensor is designed to support */
  SNS_SMGR_OP_MODE_FIFO_V01 = 2, /**<  FIFO mode is DRI mode in which multiple
                                   samples are delivered by sensor HW together */
  SNS_SMGR_OP_MODE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_op_mode_e_v01;
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t SensorId;
  /**<   Defines the sensor that this configuration pertains to. The sensor can be
    one of following:
    - 00 - SNS_SMGR_ID_ACCEL
    - 10 - SNS_SMGR_ID_GYRO
    - 20 - SNS_SMGR_ID_MAG
    - 30 - SNS_SMGR_ID_PRESSURE
    - 40 - SNS_SMGR_ID_PROX_LIGHT
    - 50 - SNS_SMGR_ID_HUMIDITY
    - 60 - SNS_SMGR_ID_RGB
    - 70 - SNS_SMGR_ID_SAR
    - 80 - SNS_SMGR_ID_HALL_EFFECT
    - 90 - SNS_SMGR_ID_HEART_RATE
    - 220 - SNS_SMGR_ID_STEP_EVENT
    - 222 - SNS_SMGR_ID_STEP_COUNT
    - 224 - SNS_SMGR_ID_SMD
    - 226 - SNS_SMGR_ID_GAME_ROTATION_VECTOR
    - 228 - SNS_SMGR_ID_IR_GESTURE
    - 230 - SNS_SMGR_ID_TAP
    - 240-249 - SNS_SMGR_ID_OEM_SENSOR_XX
    - All other values defined as SNS_SMGR_ID_XXXX style are reserved for future use
  */

  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary.
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
    - All other values defined as SNS_SMGR_DATA_TYPE_XXXX style are reserved for future use
      This parameter identifies the sensor data type.
   */

  uint8_t Sensitivity;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0.
  */

  uint8_t Decimation;
  /**<   Defines decimation option for this item in this report
    - 01 - SNS_SMGR_DECIMATION_RECENT_SAMPLE
    - 03 - SNS_SMGR_DECIMATION_FILTER
    - All other values defined as SNS_SMGR_DECIMATION_XXXX style are reserved
      for future use
    The SNS_SMGR_DECIMATION_FILTER option can used only for accelerometer and
    gyro sensor type to reduce data noise.
    When SNS_SMGR_DECIMATION_FILTER option is set, multiple samples could be
    used for one report.
   */

  uint16_t MinSampleRate;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */

  uint8_t StationaryOption;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */

  uint8_t DoThresholdTest;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */

  uint8_t ThresholdOutsideMinMax;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */

  uint8_t ThresholdDelta;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */

  uint8_t ThresholdAllAxes;
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */

  int32_t ThresholdMinMax[2];
  /**<   This parameter is defined for future use and is NOT implemented.
       This value shall be set to 0. */
}sns_smgr_periodic_report_item_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ItemNum;
  /**<   The Item parameter number in the request msg */

  uint8_t Reason;
  /**<   Defines reason codes:
    - 00 - SNS_SMGR_REASON_NULL
    - 10 - SNS_SMGR_REASON_DEFAULT_RATE
    - 12 - SNS_SMGR_REASON_DEFAULT_DECIM
    - 15 - SNS_SMGR_REASON_UNKNOWN_SENSOR
    - 16 - SNS_SMGR_REASON_FAILED_SENSOR
    - 17 - SNS_SMGR_REASON_OTHER_FAILURE
    - 18 - SNS_SMGR_REASON_SAMPLING_RATE
    - 19 - SNS_SMGR_REASON_SAMPLE_QUALITY_NORMAL
    - All other values defined as SNS_SMGR_REASON_XXXX style are reserved for future use
  */
}sns_smgr_reason_pair_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
/**    Standard Data Item
    A standard data item contains a timestamp and 3 data values plus status.
    For 3-axis sensors, the data represents XYZ axis measurements. For
    single (eg temperature) or double (eg touch screen) valued sensors, only
    1 or 2 of the values is used. The structure may be used for raw
    measurements or engineering units.

 */
typedef struct {

  uint8_t SensorId;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */

  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary.
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
    - All other values defined as SNS_SMGR_DATA_TYPE_XXXX style are reserved
      for future use
      This parameter identifies the sensor data type.
   */

  int32_t ItemData[3];
  /**<   For 3-axis items such as accelerometer, gyro, and magnetometer,
     words [0] to [2] are XYZ. For other items, only the first
     word is used. The units are defined as following:
     - ACCEL       : m/s2
     - GYRO        : rad/s
     - MAG         : Gauss
     - PRESSURE    : hPa
     - PROX        : FAR=0, NEAR=1. Note: still in Q16 format
     - LIGHT       : lx
     - TEMPERATURE : Celsius
     - HUMIDITY    : percentage in Q16
     - RGB         : Raw ADC counts : X = Red, Y = Green, Z = Blue
     - SAR         : FAR=0, NEAR=non negative number indicating the sensor touched
     - HALL_EFFECT : Mag Field present=1, Mag Field not present=0
     - HEART_RATE  : X, Y, Z = Heart rate in bpm/Unitless raw data from the sensor
     - CT_C        : X = Color temperature in Q16 (deg Kelvin), Y = Raw ADC counts
                     for Clear data, Z = Reserved
     - STEP        : 1 - a step is detected
     - STEP_COUNT  : number of steps taken
     - SMD         : 1 - SMD was detected
     - GAME_ROTATION_VECTOR : quaternion values (Q16)
     - IR_GESTURE  : (revision 10: Sensor ID defined, but not used)
     - DOUBLE-TAP/PRIMARY :
     - SINGLE-TAP/SECONDARY: Dimension-less (raw) value indicating the source of
                             the tap event, relative to the device. (Consider the
                             device as a point mass located at the origin (0,0,0)
                             of the Cartesian coordinate system.)

                             0 = no tap event,      1 = tap from +X axis, 2 = tap from -X axis,
                             3 = tap from +Y axis,  4 = tap from -Y axis, 5 = tap from +Z axis,
                             6 = tap from -Z axis,  7 = tap along X axis, 8 = tap along Y axis,
                             9 = tap along Z axis, 10 = tap event (unknown axis)

     - OEM_SENSOR  : (OEM-defined)
  */

  uint32_t TimeStamp;
  /**<   The timestamp when the sample is made in ticks. */

  uint8_t ItemFlags;
  /**<   Defines the item flags. This bit flags have meanings following:
    - 00 - Normal
    - 08 - SNS_SMGR_ITEM_FLAG_INVALID
    - 16 - SNS_SMGR_ITEM_FLAG_FAC_CAL : Factory calibration data was applied
    - 32 - SNS_SMGR_ITEM_FLAG_AUTO_CAL: Auto calibration data was applied
    - All other values defined as SNS_SMGR_ITEM_FLAG_XXXX style are reserved
      for future use
  */

  uint8_t ItemQuality;
  /**<   Defines the item quality which is associated with the ItemFlags.
    - 00 - SNS_SMGR_ITEM_QUALITY_CURRENT_SAMPLE
    - 05 - SNS_SMGR_ITEM_QUALITY_FILTERED
    - 10 - SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR
    - 11 - SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY
    - 13 - SNS_SMGR_ITEM_QUALITY_INTERPOLATED
    - 14 - SNS_SMGR_ITEM_QUALITY_INTERPOLATED_FILTERED
    - All other values defined as SNS_SMGR_ITEM_QUALITY_XXXX style are reserved
      for future use
  */

  uint8_t ItemSensitivity;
  /**<   This field is defined for future use and is NOT implemented.
       Any value in this field shall not be referenced.
  */
}sns_smgr_data_item_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */

  uint32_t SensorShortName_len;  /**< Must be set to # of elements in SensorShortName */
  char SensorShortName[SNS_SMGR_SHORT_SENSOR_NAME_SIZE_V01];
  /**<   The value is a short sensor name:
  "ACCEL"
  "GYRO"
  "MAG"
  "PROX_LIGHT"
  "PRESSURE"
  "HUMIDITY"
  "RGB"
  "SAR"
  "HALL_EFFECT"
  "HEART_RATE"
  */
}sns_smgr_sensor_id_info_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t SensorID;
  /**<   Defines the sensor that this configuration pertains to. Refer to the
       Sensor ID table defined under "Define sensor identifier" .
  */

  uint8_t DataType;
  /**<   Defines sensor data type which classifies if the data type is primary
    or secondary.
    - 00 - SNS_SMGR_DATA_TYPE_PRIMARY
    - 01 - SNS_SMGR_DATA_TYPE_SECONDARY
    - All other values defined as SNS_SMGR_IDSNS_SMGR_DATA_TYPE_XXXX style are
      reserved for future use
  */

  uint32_t SensorName_len;  /**< Must be set to # of elements in SensorName */
  char SensorName[SNS_SMGR_MAX_SENSOR_NAME_SIZE_V01];
  /**<   The model name of the sensor
  */

  uint32_t VendorName_len;  /**< Must be set to # of elements in VendorName */
  char VendorName[SNS_SMGR_MAX_VENDOR_NAME_SIZE_V01];
  /**<   The vendor name of the sensor */

  uint32_t Version;
  /**<   The version of sensor module */

  uint16_t MaxSampleRate;
  /**<   The maximum freq value that the sensor can stream */

  uint16_t IdlePower;
  /**<   Power consumption in uA when the sensor is in IDLE mode */

  uint16_t MaxPower;
  /**<   Power consumption in uA when the sensor is in operation mode */

  uint32_t MaxRange;
  /**<   The maximum range that the sensor can support in nominal engineering units.
       This value is represented by Q16 format */

  uint32_t Resolution;
  /**<   The resolution that the sensor uses in nominal engineering units.
       This value is represented by Q16 format */
}sns_smgr_sensor_datatype_info_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t data_type_info_len;  /**< Must be set to # of elements in data_type_info */
  sns_smgr_sensor_datatype_info_s_v01 data_type_info[SNS_SMGR_MAX_DATA_TYPE_PER_SENSOR_V01];
}sns_smgr_sensor_info_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t odrs_len;  /**< Must be set to # of elements in odrs */
  uint16_t odrs[SNS_SMGR_MAX_SUPPORTED_ODR_NUM_V01];
}sns_smgr_odr_list_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t SensorID;
  /**<   see #define SNS_SMGR_ID_XXX_XXX */

  uint8_t BusCanAccessSensor;

  uint8_t CanCommandSensor;

  uint8_t CanReadSensorStatus;

  uint8_t CanReadSensorData;

  uint8_t DataShowsNoise;

  uint8_t CanReadFactoryCalibrationROM;

  uint8_t ValidSelfTestReport;

  uint8_t CanReceiveInterrupt;
}sns_smgr_sensor_test_result_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t SensorID;
  /**<   see #define SNS_SMGR_ID_XXX_XXX */

  uint8_t PowerAction;
  /**<   see #define SNS_SMGR_POWER_STATE_XXX */

  uint32_t ActiveTimeStamp;
  /**<   Timestamp when state changed to Active */

  uint32_t LowPowerTimeStamp;
  /**<   Timestamp when state changed to Low*/

  uint32_t CycleCount;
  /**<   Number of power state change between on and off since last report*/
}sns_smgr_sensor_power_status_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
/**  Buffering request item definition. This structure specifies information
   client must provide for each of the requested items. 
 */
typedef struct {

  uint8_t SensorId;
  /**<   Identifies the sensor to be sampled for data.  The valid sensors are:
    - SNS_SMGR_ID_ACCEL
    - SNS_SMGR_ID_GYRO
    - SNS_SMGR_ID_MAG
    - SNS_SMGR_ID_PRESSURE
    - SNS_SMGR_ID_PROX_LIGHT
    - SNS_SMGR_ID_HUMIDITY
    - SNS_SMGR_ID_RGB
    - SNS_SMGR_ID_SAR
    - SNS_SMGR_ID_HALL_EFFECT
    - SNS_SMGR_ID_HEART_RATE
    - SNS_SMGR_ID_STEP_EVENT
    - SNS_SMGR_ID_STEP_COUNT
    - SNS_SMGR_ID_SMD
    - SNS_SMGR_ID_GAME_ROTATION_VECTOR
    - SNS_SMGR_ID_IR_GESTURE
    - SNS_SMGR_ID_TAP
    - SNS_SMGR_OEM_SENSOR_XX
    - All other values defined as SNS_SMGR_ID_XXXX style are reserved for future use
  */

  uint8_t DataType;
  /**<   Identifies which data type of the specified sensor is being requested.
    - SNS_SMGR_DATA_TYPE_PRIMARY
    - SNS_SMGR_DATA_TYPE_SECONDARY
    - All other values defined as SNS_SMGR_DATA_TYPE_XXXX style are reserved for
      future use
      This parameter identifies the sensor data type.
   */

  uint8_t Decimation;
  /**<   Specifies decimation option for samples belonging to this item
    - SNS_SMGR_DECIMATION_RECENT_SAMPLE
    - SNS_SMGR_DECIMATION_FILTER
    - All other values will be rejected.
    The SNS_SMGR_DECIMATION_FILTER option is only applicable for ACCEL, GYRO,
    and MAG sensor types to reduce data noise.  When SNS_SMGR_DECIMATION_FILTER
    option is specified, multiple samples could be used for one report.
   */

  uint8_t Calibration;
  /**<   Specifies how raw data is to be calibrated
    - SNS_SMGR_CAL_SEL_FULL_CAL
    - SNS_SMGR_CAL_SEL_FACTORY_CAL
    - SNS_SMGR_CAL_SEL_RAW
    - All other values will be rejected
    */

  uint16_t SamplingRate;
  /**<   Specifies the frequency at which sensor is sampled.
    This value shall be within the sensor capacity,                                       .
    expressed in integer format and in unit of Hz.
    Values outside of sensor capacity will be rejected.
    If this value is greater than SNS_SMGR_SAMPLING_RATE_INVERSION_POINT,
    then it will be interpreted as the sampling period, in milliseconds. This
    allows a client to sample the sensor at a rate less than 1 Hz. It is not
    recommended to sample any sensor but environmental sensors at a rate less
    than 1 Hz.
  */

  uint16_t SampleQuality;
  /**<   Specifies the desired quality of sensor data
    - SNS_SMGR_SAMPLE_QUALITY_ACCURATE_TIMESTAMP - High accuracy for sample timestamp.
      Delivery sampling rate may be up to twice the requested sampling rate,
      and may also result in higher report rate.
      Clients are recommended to specify 50, 100, or 200Hz sampling rates to
      minimize the chance of increase in sampling rate.
    */
}sns_smgr_buffering_req_item_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
/**  Sample structure in Buffering report
    For 3-axis sensors, the data represents XYZ axis measurements. For
    single (eg temperature) or double (eg touch screen) valued sensors, only
    1 or 2 of the values is used. The structure may be used for raw
    measurements or engineering units.

 */
typedef struct {

  int32_t Data[SNS_SMGR_SENSOR_DIMENSION_V01];
  /**<   Each sample can have up to SNS_SMGR_SENSOR_DIMENSION words, each word
    is in Q16 format and in the units specific to the sensor/data type pair.
    For 3-axis samples, Data[0], Data[1], and Data[2] are X, Y, and Z axis,
    respectively.  For others, only Data[0] has valid measurement.
     - ACCEL/PRIMARY        : 3 axes, each in meter/second squared (m/s2)
     - GYRO/PRIMARY         : 3 axes, each in radian/second (rad/s)
     - MAG/PRIMARY          : 3 axes, each in Gauss
     - PRESSURE/PRIMARY     : 1 axis, in hectopascal (hPa)
     - PROX/PRIMARY         : 1 axis, FAR=0, NEAR=1
     - RGB/PRIMARY          : 3 axis, X axis = raw Red counts, Y axis = raw Green counts,
                              Z axis = raw Blue counts
     - SAR/PRIMARY          : 1 axis, FAR=0, NEAR=non negative number indicating the sensor touched
     - HALL_EFFECT/PRIMARY  : 1 axis, Mag Field present=1, Mag Field not present=0
     - HEART_RATE/PRIMARY   : 3 axes, X is heart rate in bpm / each is unitless raw data from sensor
     - HEART_RATE/SECONDARY : 3 axes, each is unitless raw data from sensor
     - ACCEL/SECONDARY      : 1 axis, in Celsius
     - GYRO/SECONDARY       : 1 axis, in Celsius
     - MAG/SECONDARY        : 1 axis, in Celsius
     - PRESSURE/SECONDARY   : 1 axis, in Celsius
     - PROX/SECONDARY       : 1 axis, in Lux
     - RGB/SECONDARY        : 3 axis, X axis = Color temperature in Q16 (deg Kelvin),
                              Y axis = raw Clear counts , Z axis = Reserved
     - IR_GESTURE           : (revision 10: Sensor ID defined, but not used)
     - DOUBLE-TAP/PRIMARY   :
     - SINGLE-TAP/SECONDARY:  Dimension-less (raw) value indicating the source of the tap
                              event, relative to the device. (Consider the device as a
                              point mass located at the origin (0,0,0)
                              of the Cartesian coordinate system.)

                              0 = no tap event,      1 = tap from +X axis, 2 = tap from -X axis,
                              3 = tap from +Y axis,  4 = tap from -Y axis, 5 = tap from +Z axis,
                              6 = tap from -Z axis,  7 = tap along X axis, 8 = tap along Y axis,
                              9 = tap along Z axis, 10 = tap event (unknown axis)

     - OEM_SENSOR  : (OEM-defined)
  */

  uint16_t TimeStampOffset;
  /**<   The offset from timestamps of previous sample in report (in SSC ticks).
       Note: The maximum timestamp offset that can be portrayed with this
       field is a little less than 2 seconds. */

  uint8_t Flags;
  /**<   Status flags of this sample.
    - raw data
    - SNS_SMGR_ITEM_FLAG_INVALID
    - SNS_SMGR_ITEM_FLAG_FAC_CAL : Factory calibration data was applied
    - SNS_SMGR_ITEM_FLAG_AUTO_CAL: Auto calibration data was applied
    - All other values defined as SNS_SMGR_ITEM_FLAG_XXXX style are reserved
      for future use
  */

  uint8_t Quality;
  /**<   Quality of this sample.
    - SNS_SMGR_ITEM_QUALITY_CURRENT_SAMPLE
    - SNS_SMGR_ITEM_QUALITY_FILTERED
    - SNS_SMGR_ITEM_QUALITY_INTERPOLATED
    - SNS_SMGR_ITEM_QUALITY_INTERPOLATED_FILTERED
    - SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR
    - SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY
    - All other values defined as SNS_SMGR_ITEM_QUALITY_XXXX style are reserved
      for future use
  */
}sns_smgr_buffering_sample_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_aggregates
    @{
  */
/**    Index structure used in Buffering report 
 */
typedef struct {

  uint8_t SensorId;
  /**<   Identifies the sensor to which the samples belong.  This shall match one of
    the requested sensors.
  */

  uint8_t DataType;
  /**<   Identifies the data type of the specified sensor to which the samples belong.
   */

  uint8_t FirstSampleIdx;
  /**<   Index into Samples data of the first sample belonging to this
    SensorId/DataType pair.
   */

  uint8_t SampleCount;
  /**<   Number of samples belonging to this SensorId/DataType pair.
   */

  uint32_t FirstSampleTimestamp;
  /**<   Timestamp of first sample belonging to this SensorId/DataType pair. (in SSC ticks)
    */

  uint32_t SamplingRate;
  /**<   Specifies the frequency at which sensor is actually sampled. This value
       is expressed in Q16 format and in unit of Hz.
    */
}sns_smgr_buffering_sample_index_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_smgr_common_qmi_messages
    @{
  */
/** Indication Message; This command requests sensor data to be sampled and buffered up */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   The ID corresponding to a Buffering request */

  /* Mandatory */
  uint32_t Indices_len;  /**< Must be set to # of elements in Indices */
  sns_smgr_buffering_sample_index_s_v01 Indices[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];
  /**<   Identifies which items in Samples belong to which SensorId/DataType pair
    specified in Buffering request */

  /* Mandatory */
  uint32_t Samples_len;  /**< Must be set to # of elements in Samples */
  sns_smgr_buffering_sample_s_v01 Samples[SNS_SMGR_BUFFERING_REPORT_MAX_SAMPLES_V01];
  /**<   Samples collected since previous report
    Depending on whether Batching is in effect, this may contain samples for
    only one of the requested items, or it may contain samples for all of them.

    Note: If any overflow is expected in the TimeStampOffset fields within
          this field, the SMGR will split each sample into its own indication
          and send indications back-to-back. */

  /* Optional */
  uint8_t IndType_valid;  /**< Must be set to true if IndType is being passed */
  uint8_t IndType;
  /**<   Optional batch indication type
       SNS_BATCH_ONLY_IND - Standalone batch indication. Not part of a back to
         back indication stream
       SNS_BATCH_FIRST_IND - First indication in stream of back to back indications
       SNS_BATCH_INTERMEDIATE_IND - Intermediate indication in stream of back to
         back indications
       SNS_BATCH_LAST_IND - Last indication in stream of back to back indications
    */
}sns_smgr_buffering_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object sns_smgr_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

