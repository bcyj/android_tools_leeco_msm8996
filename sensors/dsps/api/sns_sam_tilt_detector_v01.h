#ifndef SNS_SAM_TILT_DETECTOR_SVC_SERVICE_01_H
#define SNS_SAM_TILT_DETECTOR_SVC_SERVICE_01_H
/**
  @file sns_sam_tilt_detector_v01.h

  @brief This is the public header file which defines the SNS_SAM_TILT_DETECTOR_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_TILT_DETECTOR_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_tilt_detector_v01.idl */

/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_TILT_DETECTOR_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_TILT_DETECTOR_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_TILT_DETECTOR_SVC_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_TILT_DETECTOR_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_TILT_DETECTOR_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_TILT_DETECTOR_SUID_V01 0xb269c2a2f74c11e3

/**  Max number of reports in a batch indication -
     set based on max payload size supported by transport framework */
#define SNS_SAM_TILT_DETECTOR_MAX_ITEMS_IN_BATCH_V01 100

/**  Maximum tilt angle threshold */
#define SNS_SAM_TILT_DETECTOR_MAX_TILT_ANGLE_THRESH_V01 90
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables the significant motion detector service. */
typedef struct {

  /* Mandatory */
  /*  Tilt Angle Threshold */
  uint32_t angle_thresh;
  /**<   The tilt-angle threshold in which a tilt event is detected, in degrees.

       The Tilt Detector service keeps track of the Current Gravity Vector
       Estimation averaged over a time window (configurable in the registry).
       Once the angle between the Current Gravity Vector Estimation and the
       Reference Gravity Vector surpasses the tilt-angle threshold, this
       service will send an indication to the requesting client.

       If this value is 0, the Tilt Detector will use the default tilt-angle
       threshold. If this value exceeds SNS_SAM_TILT_DETECTOR_MAX_TILT_ANGLE_THRESH,
       then this enable request will return the error "SENSOR1_EBAD_PARAM".
   */

  /* Optional */
  /*  Notify Suspend */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend = FALSE

       This field does not have any bearing on error indication
       messages, which will be sent even during suspend.
    */
}sns_sam_tilt_detector_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables the significant motion detector service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  sns_common_resp_s_v01 resp;

  /* Optional */
  /*  Instance ID */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<  
    Algorithm instance ID maintained/assigned by SAM.
    The client shall use this instance ID for future messages associated with
    current algorithm instance.
    */
}sns_sam_tilt_detector_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   To identify the algorithm instance to be disabled.  */
}sns_sam_tilt_detector_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  sns_common_resp_s_v01 resp;

  /* Optional */
  /*  Instance ID */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_tilt_detector_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Indication Message; Report from the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  /*  Tilt Timestamp */
  uint32_t tilt_timestamp;
  /**<   Timestamp of the last tilt event; in SSC ticks.
       If the tilt event was detected at timestamp 0, then this value must
       be set to 1. The value 0 is reserved for no tilt detected.  */
}sns_sam_tilt_detector_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error report from the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Error */
  uint8_t error;
  /**<   sensors error code */

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  /*  Timestamp */
  uint32_t timestamp;
  /**<   Timestamp of when the error was detected; in SSC ticks */
}sns_sam_tilt_detector_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command resets Reference Gravity Vector to the Current Gravity
           Vector Estimation within the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_tilt_detector_reset_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command resets Reference Gravity Vector to the Current Gravity
           Vector Estimation within the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Response */
  sns_common_resp_s_v01 resp;

  /* Optional */
  /*  Instance ID */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_tilt_detector_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command handles batch mode for the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  /*  Batch Period */
  int32_t batch_period;
  /**<   Specifies the interval over which reports are to be batched, in seconds;
       Q16 format. If AP is in suspend and notify_suspend is FALSE, undelivered
       reports will be batched in circular FIFO and delivered upon wakeup.
       P = 0 to disable batching.
       P > 0 to enable batching.
    */

  /* Optional */
  /*  Request Type */
  uint8_t req_type_valid;  /**< Must be set to true if req_type is being passed */
  sns_batch_req_type_e_v01 req_type;
  /**<   Field no longer supported in version 3 and later.  Functionality can be
       approximated using the notify_suspend flag in the ENABLE_REQ message.  Max
       buffer depth is returned in GET_ATTRIBUTES_RESP.

       Optional request type:
       0 – Do not wake client from suspend when buffer is full.
       1 – Wake client from suspend when buffer is full.
       2 – Use to get max buffer depth. Does not enable/disable batching.
           instance_id and batch_period are ignored for this request type.
       Defaults to 0.
    */
}sns_sam_tilt_detector_batch_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command handles batch mode for the Tilt Detector service. */
typedef struct {

  /* Mandatory */
  /*  Response */
  sns_common_resp_s_v01 resp;

  /* Optional */
  /*  Instance ID */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Algorithm instance ID maintained/assigned by SAM */

  /* Optional */
  /*  Max Batch Size */
  uint8_t max_batch_size_valid;  /**< Must be set to true if max_batch_size is being passed */
  uint32_t max_batch_size;
  /**<   Max supported batch size */

  /* Optional */
  /*  Timestamp */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp when the batch request was processed in SSC ticks */
}sns_sam_tilt_detector_batch_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Indication Message; Report containing a batch of Tilt Detector outputs */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance */

  /* Mandatory */
  /*  Tilt Timestamps */
  uint32_t tilt_timestamps_len;  /**< Must be set to # of elements in tilt_timestamps */
  uint32_t tilt_timestamps[SNS_SAM_TILT_DETECTOR_MAX_ITEMS_IN_BATCH_V01];
  /**<   Detected tilt timestamps */

  /* Optional */
  /*  Indication Type */
  uint8_t ind_type_valid;  /**< Must be set to true if ind_type is being passed */
  uint8_t ind_type;
  /**<   Optional batch indication type
       SNS_BATCH_ONLY_IND - Standalone batch indication. Not part of a back to back indication stream
       SNS_BATCH_FIRST_IND - First indication in stream of back to back indications
       SNS_BATCH_INTERMEDIATE_IND - Intermediate indication in stream of back to back indications
       SNS_BATCH_LAST_IND - Last indication in stream of back to back indications
    */
}sns_sam_tilt_detector_batch_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command updates active batch period for a Tilt Detector
           algorithm when batching is ongoing. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance.  */

  /* Mandatory */
  /*  Active Batch Period */
  int32_t active_batch_period;
  /**<   Specifies new interval (in seconds, Q16) over which reports are to be
       batched when the client processor is awake. Only takes effect if
       batching is ongoing.
       P > 0 to to override active batch period set in batch enable request.
       P = 0 to switch to active batch period set in batch enable request.
    */
}sns_sam_tilt_detector_update_batch_period_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command updates active batch period for a Tilt Detector
           algorithm when batching is ongoing. */
typedef struct {

  /* Mandatory */
  /*  Response */
  sns_common_resp_s_v01 resp;

  /* Optional */
  /*  Instance ID */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Algorithm instance ID maintained/assigned by SAM */

  /* Optional */
  /*  Timestamp */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp when the batch request was processed in SSC ticks */
}sns_sam_tilt_detector_update_batch_period_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_BATCH_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_BATCH_REPORT_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_CANCEL_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_DISABLE_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_ENABLE_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_ERROR_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_REPORT_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_RESET_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_UPDATE_BATCH_PERIOD_V01 
//#define REMOVE_SNS_SAM_TILT_DETECTOR_VERSION_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_TILT_DETECTOR_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_TILT_DETECTOR_CANCEL_REQ_V01 0x0000
#define SNS_SAM_TILT_DETECTOR_CANCEL_RESP_V01 0x0000
#define SNS_SAM_TILT_DETECTOR_VERSION_REQ_V01 0x0001
#define SNS_SAM_TILT_DETECTOR_VERSION_RESP_V01 0x0001
#define SNS_SAM_TILT_DETECTOR_ENABLE_REQ_V01 0x0002
#define SNS_SAM_TILT_DETECTOR_ENABLE_RESP_V01 0x0002
#define SNS_SAM_TILT_DETECTOR_DISABLE_REQ_V01 0x0003
#define SNS_SAM_TILT_DETECTOR_DISABLE_RESP_V01 0x0003
#define SNS_SAM_TILT_DETECTOR_REPORT_IND_V01 0x0005
#define SNS_SAM_TILT_DETECTOR_ERROR_IND_V01 0x0006
#define SNS_SAM_TILT_DETECTOR_RESET_REQ_V01 0x0020
#define SNS_SAM_TILT_DETECTOR_RESET_RESP_V01 0x0020
#define SNS_SAM_TILT_DETECTOR_BATCH_REQ_V01 0x0021
#define SNS_SAM_TILT_DETECTOR_BATCH_RESP_V01 0x0021
#define SNS_SAM_TILT_DETECTOR_BATCH_IND_V01 0x0022
#define SNS_SAM_TILT_DETECTOR_UPDATE_BATCH_PERIOD_REQ_V01 0x0023
#define SNS_SAM_TILT_DETECTOR_UPDATE_BATCH_PERIOD_RESP_V01 0x0023
#define SNS_SAM_TILT_DETECTOR_GET_ATTRIBUTES_REQ_V01 0x0024
#define SNS_SAM_TILT_DETECTOR_GET_ATTRIBUTES_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_TILT_DETECTOR_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_TILT_DETECTOR_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_TILT_DETECTOR_SVC_get_service_object_v01( ) \
          SNS_SAM_TILT_DETECTOR_SVC_get_service_object_internal_v01( \
            SNS_SAM_TILT_DETECTOR_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_TILT_DETECTOR_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_TILT_DETECTOR_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

