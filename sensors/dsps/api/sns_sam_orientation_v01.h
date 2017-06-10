#ifndef SNS_SAM_ORIENTATION_SVC_SERVICE_01_H
#define SNS_SAM_ORIENTATION_SVC_SERVICE_01_H
/**
  @file sns_sam_orientation_v01.h

  @brief This is the public header file which defines the SNS_SAM_ORIENTATION_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_ORIENTATION_SVC. It contains the constant values defined, enums, structures,
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
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_orientation_v01.idl */

/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_ORIENTATION_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_ORIENTATION_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_ORIENTATION_SVC_V01_IDL_MINOR_VERS 0x07
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_ORIENTATION_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_ORIENTATION_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_ORIENTATION_SUID_V01 0x26f3f887f69de6ee

/**  Sensor accuracy is unreliable.  */
#define SNS_SENSOR_ACCURACY_UNRELIABLE_V01 0

/**  Sensor accuracy is low.  */
#define SNS_SENSOR_ACCURACY_LOW_V01 1

/**  Sensor accuracy is medium.  */
#define SNS_SENSOR_ACCURACY_MEDIUM_V01 2

/**  Sensor accuracy is high.  */
#define SNS_SENSOR_ACCURACY_HIGH_V01 3

/**  Maximum number of reports in a batch indication. This is set
     based on the maximum payload size supported by the transport framework. */
#define SNS_SAM_ORIENTATION_MAX_REPORTS_IN_BATCH_V01 42
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_aggregates
    @{
  */
typedef struct {

  float rotation_vector[4];
  /**<   Rotation vector values; unitless, float.*/

  float gravity[3];
  /**<   Gravity vector along axis x/y/z; units: m/s/s; SAE coordinate frame. */

  float lin_accel[3];
  /**<   Linear acceleration along axis x/y/z; units: m/s/s; SAE coordinate frame. */

  uint8_t rotation_vector_accuracy;
  /**<   Rotation vector accuracy:
     SNS_SENSOR_ACCURACY_UNRELIABLE = 0,
     SNS_SENSOR_ACCURACY_LOW        = 1,
     SNS_SENSOR_ACCURACY_MEDIUM     = 2,
     SNS_SENSOR_ACCURACY_HIGH       = 3 */

  uint8_t gravity_accuracy;
  /**<   Gravity accuracy:
       SNS_SENSOR_ACCURACY_UNRELIABLE = 0,
       SNS_SENSOR_ACCURACY_LOW        = 1,
       SNS_SENSOR_ACCURACY_MEDIUM     = 2,
       SNS_SENSOR_ACCURACY_HIGH       = 3 */

  uint8_t coordinate_system;
  /**<   Coordinate system used in the output:
       0 = Android (East/North/Up) - default
       1 = SAE (North/East/Down) */
}sns_sam_orientation_result_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  uint32_t report_period;
  /**<   Report period in seconds; Q16 format. This determines the algorithm
       output report rate. Specify 0 to report at the sampling rate.
  */

  /* Optional */
  uint8_t sample_rate_valid;  /**< Must be set to true if sample_rate is being passed */
  uint32_t sample_rate;
  /**<   Sample rate in Hz; Q16 format.
     If the sample rate is less than the report rate, it is set to the report
       rate.
  */

  /* Optional */
  uint8_t coordinate_sys_valid;  /**< Must be set to true if coordinate_sys is being passed */
  uint8_t coordinate_sys;
  /**<   Coordinate system used in the output:
       0 = Android (East/North/Up) - default
       1 = SAE (North/East/Down)*/

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
}sns_sam_orientation_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<  
    Instance ID, which is assigned by the SAM.
    The client must use this instance ID for future messages associated with
    the current algorithm instance.
  */
}sns_sam_orientation_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */
}sns_sam_orientation_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */
}sns_sam_orientation_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Indication Message; Report containing the Orientation algorithm output. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of the input used to generate the algorithm output. */

  /* Mandatory */
  sns_sam_orientation_result_s_v01 result;
  /**<   Output of the orientation algorithm instance. */
}sns_sam_orientation_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output of a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */
}sns_sam_orientation_get_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output of a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp of the input used to generate the algorithm output. */

  /* Optional */
  uint8_t result_valid;  /**< Must be set to true if result is being passed */
  sns_sam_orientation_result_s_v01 result;
  /**<   Output of the Orientation algorithm instance. */
}sns_sam_orientation_get_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error indication for an Orientation algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   Sensor1 error code. */

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */
}sns_sam_orientation_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Request Message; This command handles batch mode for a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Mandatory */
  int32_t batch_period;
  /**<   Specifies the interval over which reports are to be batched, in seconds;
       Q16 format. If AP is in suspend and notify_suspend is FALSE, undelivered
       reports will be batched in circular FIFO and delivered upon wakeup.
       - P = 0 -- Disable batching
       - P > 0 -- Enable batching
    */

  /* Optional */
  uint8_t req_type_valid;  /**< Must be set to true if req_type is being passed */
  sns_batch_req_type_e_v01 req_type;
  /**<   Field no longer supported in version 7 and later.  Functionality can be
       approximated using the notify_suspend flag in the ENABLE_REQ message.  Max
       buffer depth is returned in GET_ATTRIBUTES_RESP.

       Optional request type:
       - 0 -– Do not wake the client from Suspend when the buffer is full (default)
       - 1 -– Wake the client from Suspend when the buffer is full
       - 2 –- Use to get the maximum buffer depth. This does not enable/disable batching.
           instance_id and batch_period are ignored for this request type.
    */
}sns_sam_orientation_batch_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Response Message; This command handles batch mode for a Orientation algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Optional */
  uint8_t max_batch_size_valid;  /**< Must be set to true if max_batch_size is being passed */
  uint32_t max_batch_size;
  /**<   Maximum supported batch size. */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp when the batch request was processed, in SSC ticks. */
}sns_sam_orientation_batch_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Indication Message; Report containing a batch of algorithm outputs. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t first_report_timestamp;
  /**<   Timestamp of the input used to generate the first algorithm output in
       the batch, in SSC ticks. */

  /* Mandatory */
  uint32_t timestamp_offsets_len;  /**< Must be set to # of elements in timestamp_offsets */
  uint16_t timestamp_offsets[SNS_SAM_ORIENTATION_MAX_REPORTS_IN_BATCH_V01];
  /**<   Offsets from the timestamp of the previous report in the batch. */

  /* Mandatory */
  uint32_t reports_len;  /**< Must be set to # of elements in reports */
  sns_sam_orientation_result_s_v01 reports[SNS_SAM_ORIENTATION_MAX_REPORTS_IN_BATCH_V01];
  /**<   Orientation algorithm output reports. */

  /* Optional */
  uint8_t ind_type_valid;  /**< Must be set to true if ind_type is being passed */
  uint8_t ind_type;
  /**<   Optional batch indication type:
       - SNS_BATCH_ONLY_IND -- Standalone batch indication, not part of a back-to-back indication stream
       - SNS_BATCH_FIRST_IND -- First indication in a stream of back-to-back indications
       - SNS_BATCH_INTERMEDIATE_IND -- Intermediate indication in a stream of back-to-back indications
       - SNS_BATCH_LAST_IND -- Last indication in a stream of back-to-back indications
    */
}sns_sam_orientation_batch_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Request Message; This command updates active batch period for a Orientation
           algorithm when batching is ongoing. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Mandatory */
  int32_t active_batch_period;
  /**<   Specifies the new interval (in seconds, Q16 format) over which reports
       are to be batched when the client processor is awake. This only takes
       effect if batching is ongoing:
       - P > 0 -- Override the active batch period set in the batch enable request
       - P = 0 -- Switch to the active batch period set in the batch enable request
    */
}sns_sam_orient_update_batch_period_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_messages
    @{
  */
/** Response Message; This command updates active batch period for a Orientation
           algorithm when batching is ongoing. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance.  */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp when the batch request was processed, in SSC ticks. */
}sns_sam_orient_update_batch_period_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_COMMON_CANCEL_V01 
//#define REMOVE_SNS_COMMON_VERSION_V01 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_BATCH_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_BATCH_REPORT_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_DISABLE_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_ENABLE_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_ERROR_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_GET_REPORT_V01 
//#define REMOVE_SNS_SAM_ORIENTATION_REPORT_V01 
//#define REMOVE_SNS_SAM_ORIENT_UPDATE_BATCH_PERIOD_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_ORIENTATION_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_ORIENTATION_CANCEL_REQ_V01 0x0000
#define SNS_SAM_ORIENTATION_CANCEL_RESP_V01 0x0000
#define SNS_SAM_ORIENTATION_VERSION_REQ_V01 0x0001
#define SNS_SAM_ORIENTATION_VERSION_RESP_V01 0x0001
#define SNS_SAM_ORIENTATION_ENABLE_REQ_V01 0x0002
#define SNS_SAM_ORIENTATION_ENABLE_RESP_V01 0x0002
#define SNS_SAM_ORIENTATION_DISABLE_REQ_V01 0x0003
#define SNS_SAM_ORIENTATION_DISABLE_RESP_V01 0x0003
#define SNS_SAM_ORIENTATION_GET_REPORT_REQ_V01 0x0004
#define SNS_SAM_ORIENTATION_GET_REPORT_RESP_V01 0x0004
#define SNS_SAM_ORIENTATION_REPORT_IND_V01 0x0005
#define SNS_SAM_ORIENTATION_ERROR_IND_V01 0x0006
#define SNS_SAM_ORIENTATION_BATCH_REQ_V01 0x0021
#define SNS_SAM_ORIENTATION_BATCH_RESP_V01 0x0021
#define SNS_SAM_ORIENTATION_BATCH_IND_V01 0x0022
#define SNS_SAM_ORIENT_UPDATE_BATCH_PERIOD_REQ_V01 0x0023
#define SNS_SAM_ORIENT_UPDATE_BATCH_PERIOD_RESP_V01 0x0023
#define SNS_SAM_ORIENT_GET_ATTRIBUTES_REQ_V01 0x0024
#define SNS_SAM_ORIENT_GET_ATTRIBUTES_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_ORIENTATION_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_ORIENTATION_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_ORIENTATION_SVC_get_service_object_v01( ) \
          SNS_SAM_ORIENTATION_SVC_get_service_object_internal_v01( \
            SNS_SAM_ORIENTATION_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_ORIENTATION_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_ORIENTATION_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

