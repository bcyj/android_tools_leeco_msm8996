#ifndef SNS_SAM_EVENT_GATED_SENSOR_SVC_SERVICE_01_H
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_SERVICE_01_H
/**
  @file sns_sam_event_gated_sensor_v01.h

  @brief This is the public header file which defines the SNS_SAM_EVENT_GATED_SENSOR_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_EVENT_GATED_SENSOR_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Fri Aug 22 2014 (Spin 0)
   From IDL File: sns_sam_event_gated_sensor_v01.idl */

/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"
#include "sns_smgr_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_consts 
    @{ 
  */

/**  */
#define SNS_SAM_EVENT_GATED_SENSOR_SUID_V01 0x8a839800b76018ca
/**
    @}
  */

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables a event gated sensor service. */
typedef struct {

  /* Mandatory */
  uint64_t event_suid;
  /**<   Event Sensor SUID identifies the the gating event sensor. The following classify as event sensors -
       SNS_SAM_AMD_SUID_V01, SNS_SAM_RMD_SUID_V01, SNS_SAM_SMD_SUID_V01, SNS_SAM_FACING_SUID_V01
       Other service ID will result in SENSOR1_EBAD_PARAM response
       The event sensors will be configured with default config options
  */

  /* Mandatory */
  uint8_t gating_condition;
  /**<   Gating condition identifies the state of the event sensor during which the sensor stream is to be gated.
       When event sensor is in the state specified by the gating condition, the sensor stream will be gated. In all other
       states, the sensor stream will be allowed.
       Gating condition must be one the event states supported by the event sensor as listed below
       Sensor: SNS_SAM_AMD_SUID_V01/SNS_SAM_RMD_SUID_V01 - Gating Condition: SNS_SAM_MOTION_REST_V01 or SNS_SAM_MOTION_MOVE_V01
       Sensor: SNS_SAM_SMD_SUID_V01                      - Gating Condition: SMD_STATE_NO_MOTION/SMD_STATE_MOTION
       Sensor: SNS_SAM_FACING_SUID_V01                   - State: FACING_UP/FACING_DOWN
       Else the service will respond with a SENSOR1_EBAD_PARAM
  */

  /* Mandatory */
  uint32_t streaming_item_len;  /**< Must be set to # of elements in streaming_item */
  sns_smgr_buffering_req_item_s_v01 streaming_item[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];
  /**<   The configuration of the physical sensor streams to be gated

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
  uint8_t report_rate_valid;  /**< Must be set to true if report_rate is being passed */
  uint32_t report_rate;
  /**<   Specifies the desired reporting rate for the streaming sensor expressed in Q16 format and in unit of Hz.
    If report rate is not specified, sensor data will be reported at sampling rate
    Sensor will be streaming only when the gating condition is not true i.e. when the event sensor is in any
    state other than that specified in the gating condition.
  */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type                  = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend  = FALSE

       This field does not have any bearing on error indication
       messages, which will be sent even during suspend.
    */
}sns_sam_event_gated_sensor_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables a event gated sensor service. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<  
    Instance ID is assigned by SAM.
    The client shall use this instance ID for future messages associated with
    this algorithm instance.
  */
}sns_sam_event_gated_sensor_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables a event gated sensor service. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */
}sns_sam_event_gated_sensor_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables a event gated sensor service. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */
}sns_sam_event_gated_sensor_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages
    @{
  */
/** Indication Message; This command requests sensor data to be sampled and buffered up
This command requests sensor data to be sampled and buffered up */
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
}sns_sam_event_gated_sensor_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error indication from a event gated sensor service. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   Sensor1 error code */

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */

  /* Mandatory */
  uint32_t timestamp;
  /**<   time stamp of input which caused this indication; in SSC ticks */
}sns_sam_event_gated_sensor_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_COMMON_CANCEL_V01 
//#define REMOVE_SNS_COMMON_VERSION_V01 
//#define REMOVE_SNS_SAM_EVENT_GATED_SENSOR_DISABLE_V01 
//#define REMOVE_SNS_SAM_EVENT_GATED_SENSOR_ENABLE_V01 
//#define REMOVE_SNS_SAM_EVENT_GATED_SENSOR_ERROR_V01 
//#define REMOVE_SNS_SAM_EVENT_GATED_SENSOR_REPORT_V01 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_EVENT_GATED_SENSOR_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_EVENT_GATED_SENSOR_CANCEL_REQ_V01 0x0000
#define SNS_SAM_EVENT_GATED_SENSOR_CANCEL_RESP_V01 0x0000
#define SNS_SAM_EVENT_GATED_SENSOR_VERSION_REQ_V01 0x0001
#define SNS_SAM_EVENT_GATED_SENSOR_VERSION_RESP_V01 0x0001
#define SNS_SAM_EVENT_GATED_SENSOR_ENABLE_REQ_V01 0x0002
#define SNS_SAM_EVENT_GATED_SENSOR_ENABLE_RESP_V01 0x0002
#define SNS_SAM_EVENT_GATED_SENSOR_DISABLE_REQ_V01 0x0003
#define SNS_SAM_EVENT_GATED_SENSOR_DISABLE_RESP_V01 0x0003
#define SNS_SAM_EVENT_GATED_SENSOR_REPORT_IND_V01 0x0005
#define SNS_SAM_EVENT_GATED_SENSOR_ERROR_IND_V01 0x0006
#define SNS_SAM_EVENT_GATED_SENSOR_GET_ATTRIBUTES_REQ_V01 0x0024
#define SNS_SAM_EVENT_GATED_SENSOR_GET_ATTRIBUTES_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_EVENT_GATED_SENSOR_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_EVENT_GATED_SENSOR_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_get_service_object_v01( ) \
          SNS_SAM_EVENT_GATED_SENSOR_SVC_get_service_object_internal_v01( \
            SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_EVENT_GATED_SENSOR_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

