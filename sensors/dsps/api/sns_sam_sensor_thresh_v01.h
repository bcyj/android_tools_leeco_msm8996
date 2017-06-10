#ifndef SNS_SAM_SENSOR_THRESH_SVC_SERVICE_01_H
#define SNS_SAM_SENSOR_THRESH_SVC_SERVICE_01_H
/**
  @file sns_sam_sensor_thresh_v01.h

  @brief This is the public header file which defines the SNS_SAM_SENSOR_THRESH_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_SENSOR_THRESH_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Fri Aug 22 2014 (Spin 0)
   From IDL File: sns_sam_sensor_thresh_v01.idl */

/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_SENSOR_THRESH_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_SENSOR_THRESH_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_SENSOR_THRESH_SVC_V01_IDL_MINOR_VERS 0x06
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_SENSOR_THRESH_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_SENSOR_THRESH_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_THRESHOLD_SUID_V01 0x2c4c22743f921c6b
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_enums
    @{
  */
typedef enum {
  THRESHOLD_TYPE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_SENSOR_ABSOLUTE_THRESHOLD_V01 = 0, /**<  Absolute threshold.  */
  SNS_SAM_SENSOR_RELATIVE_THRESHOLD_V01 = 1, /**<  Relative threshold.  */
  THRESHOLD_TYPE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}threshold_type_e_v01;
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables a sensor-threshold algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t sensor_id;
  /**<   Sensor ID, defined as SNS_SMGR_ID_XXXX style.
       SNS_SMGR_ID_PROX_LIGHT_V01, SNS_SMGR_ID_ACCEL_V01,SNS_SMGR_ID_GYRO_V01,
       SNS_SMGR_ID_MAG_V01 and SNS_SMGR_ID_PRESSURE_V01 are supported.
       Unsupported sensor IDs will result in SENSOR1_EBAD_PARAM error in response.
  */

  /* Mandatory */
  uint8_t data_type;
  /**<   Defines the sensor data type:
       - 00 -- SNS_SMGR_DATA_TYPE_PRIMARY
       - 01 -- SNS_SMGR_DATA_TYPE_SECONDARY
  */

  /* Mandatory */
  uint32_t sample_rate;
  /**<   Sample rate in Hz; Q16 format. */

  /* Mandatory */
  uint32_t threshold[3];
  /**<   Threshold for the sensor data output, in Q16 format.
       For 3-axis items, such as accelerometer, gyroscope, and magnetometer,
       words [0] to [2] are XYZ. For other items, fewer words may be used.

       If, for any of the valid axis, the current and last reported samples
       have an absolute difference that is bigger than or equal to the threshold,
       the current samples on all valid axis are reported to the client.

       For a proximity sensor that gives binary (NEAR or FAR) values, setting
       threshold[0] to 0x10000 will effectively only output proximity data
       on a change (from NEAR to FAR, or FAR to NEAR).

       For accelerometer, the threshold is set in units of m/s^2, in Q16 format.
       For gyroscope, the threshold is set in units of rad/s, in Q16 format.

       The meaning of this field can be changed based on the value of threshold_type, i.e.,
       when threshold_type = SNS_SAM_SENSOR_RELATIVE_THRESHOLD, the threshold's value
       is a percent value (between 0.0 and 1.0) in Q16 format.
       For example, if the relative threshold = 100% (= 1.0), the threshold = 0x00010000.
  */

  /* Optional */
  uint8_t report_period_valid;  /**< Must be set to true if report_period is being passed */
  uint32_t report_period;
  /**<   Current report interval, in milliseconds.

       If report_period is not specified OR it is specified as zero, the
       behavior described for the threshold parameter above will take effect.

       If report_period is specified, the following behavior will take effect:
       If the sensor sample value of any axis exceeds the threshold for a time
       period equal to or greater than the time threshold, report new value to client.

       Note that a single axis must be outside the range for the entire
       report period.
  */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies whether indications for this request are to be sent
       when the processor is in the Suspend state.

       If this field is not specified, the default value is set to:
       notify_suspend->proc_type                  = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend  = FALSE

       This field does not have any bearing on error indication
       messages, which are sent even during Suspend.
    */

  /* Optional */
  uint8_t threshold_type_valid;  /**< Must be set to true if threshold_type is being passed */
  uint8_t threshold_type;
  /**<   Threshold type, as specified by threshold_type_e. */
}sns_sam_sensor_thresh_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables a sensor-threshold algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance ID, which is assigned by the SAM.
       The client must use this instance ID for future messages associated with
       the current algorithm instance.
  */
}sns_sam_sensor_thresh_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables a sensor-threshold algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_sensor_thresh_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables a sensor-threshold algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_sensor_thresh_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Indication Message; Output report from a sensor threshold algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of the input used to generate the algorithm output. */

  /* Mandatory */
  uint32_t sample_value[3];
  /**<   Output of the sensor threshold algorithm:
       For 3-axis items, such as accelerometer, gyroscope, and magnetometer,
       words [0] to [2] are XYZ. For other items, less words may be used.
       This report indication is generated if any of the current samples
       exceeds the respective threshold from the last reported samples.
       The current sample values are reported in Q16 format.
  */
}sns_sam_sensor_thresh_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_sensor_thresh_get_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp of the input used to generate the algorithm output. */

  /* Optional */
  uint8_t sample_value_valid;  /**< Must be set to true if sample_value is being passed */
  uint32_t sample_value[3];
  /**<   Latest output of the sensor threshold algorithm instance.
       For 3-axis items, such as accelerometer, gyroscope, and magnetometer,
       words [0] to [2] are XYZ. For other items, less words may be used.
       These output sample values are reported in Q16 format.
  */
}sns_sam_sensor_thresh_get_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error indication from a sensor-threshold algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   Sensor1 error code. */

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_sensor_thresh_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_COMMON_CANCEL_V01 
//#define REMOVE_SNS_COMMON_VERSION_V01 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V01 
//#define REMOVE_SNS_SAM_SENSOR_THRESH_DISABLE_V01 
//#define REMOVE_SNS_SAM_SENSOR_THRESH_ENABLE_V01 
//#define REMOVE_SNS_SAM_SENSOR_THRESH_ERROR_V01 
//#define REMOVE_SNS_SAM_SENSOR_THRESH_GET_REPORT_V01 
//#define REMOVE_SNS_SAM_SENSOR_THRESH_REPORT_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_SENSOR_THRESH_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_SENSOR_THRESH_CANCEL_REQ_V01 0x0000
#define SNS_SAM_SENSOR_THRESH_CANCEL_RESP_V01 0x0000
#define SNS_SAM_SENSOR_THRESH_VERSION_REQ_V01 0x0001
#define SNS_SAM_SENSOR_THRESH_VERSION_RESP_V01 0x0001
#define SNS_SAM_SENSOR_THRESH_ENABLE_REQ_V01 0x0002
#define SNS_SAM_SENSOR_THRESH_ENABLE_RESP_V01 0x0002
#define SNS_SAM_SENSOR_THRESH_DISABLE_REQ_V01 0x0003
#define SNS_SAM_SENSOR_THRESH_DISABLE_RESP_V01 0x0003
#define SNS_SAM_SENSOR_THRESH_GET_REPORT_REQ_V01 0x0004
#define SNS_SAM_SENSOR_THRESH_GET_REPORT_RESP_V01 0x0004
#define SNS_SAM_SENSOR_THRESH_REPORT_IND_V01 0x0005
#define SNS_SAM_SENSOR_THRESH_ERROR_IND_V01 0x0006
#define SNS_SAM_SENSOR_THRESH_GET_ATTRIBUTES_REQ_V01 0x0024
#define SNS_SAM_SENSOR_THRESH_GET_ATTRIBUTES_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_SENSOR_THRESH_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_SENSOR_THRESH_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_SENSOR_THRESH_SVC_get_service_object_v01( ) \
          SNS_SAM_SENSOR_THRESH_SVC_get_service_object_internal_v01( \
            SNS_SAM_SENSOR_THRESH_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_SENSOR_THRESH_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_SENSOR_THRESH_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

