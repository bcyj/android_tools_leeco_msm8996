#ifndef SNS_SAM_DISTANCE_BOUND_SVC_SERVICE_01_H
#define SNS_SAM_DISTANCE_BOUND_SVC_SERVICE_01_H
/**
  @file sns_sam_distance_bound_v01.h

  @brief This is the public header file which defines the SNS_SAM_DISTANCE_BOUND_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_DISTANCE_BOUND_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.10 
   It was generated on: Mon Aug 18 2014 (Spin 0)
   From IDL File: sns_sam_distance_bound_v01.idl */

/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_DISTANCE_BOUND_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_DISTANCE_BOUND_SVC_V01_IDL_MINOR_VERS 0x05
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_DISTANCE_BOUND_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_DISTANCE_BOUND_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_consts 
    @{ 
  */

/**  The maximum number of motion states that can be supported  */
#define SNS_SAM_DISTANCE_BOUND_SUID_V01 0xa1fe7c7b96bb2322
#define SNS_SAM_DISTANCE_BOUND_MOTION_STATE_MAX_V01 50
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_DISTANCE_BOUND_MS_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_UNKNOWN_V01 = 0, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_STATIONARY_V01 = 1, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_INMOTION_V01 = 2, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_FIDDLE_V01 = 3, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_PEDESTRIAN_V01 = 4, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_VEHICLE_V01 = 5, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_WALK_V01 = 6, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_RUN_V01 = 7, 
  SNS_SAM_DISTANCE_BOUND_MOTION_STATE_BIKE_V01 = 8, 
  SNS_SAM_DISTANCE_BOUND_MS_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_distance_bound_ms_e_v01;
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_UNKNOWN_V01 = -1, 
  SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_FALSE_V01 = 0, 
  SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_TRUE_V01 = 1, 
  SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_distance_bound_breach_event_e_v01;
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_aggregates
    @{
  */
typedef struct {

  sns_sam_distance_bound_breach_event_e_v01 distance_bound_breach_event;
  /**<  
    Indicates if the distance bound set by client has been breached.
    Set to SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_UNKNOWN if
    no sns_sam_distance_bound_set_bound_req_msg has been received. All other
    output fields are to be ignored.
    Set to SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_TRUE if breach has occurred.
    Set to SNS_SAM_DISTANCE_BOUND_BREACH_EVENT_FALSE if breach has not occurred.
  */

  uint8_t session_key;
  /**<  
    The unique session key provided by client associated with this report.
  */
}sns_sam_distance_bound_report_data_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables the distance_bound algorithm. */
typedef struct {

  /* Optional */
  uint8_t motion_state_speed_bounds_valid;  /**< Must be set to true if motion_state_speed_bounds is being passed */
  uint32_t motion_state_speed_bounds_len;  /**< Must be set to # of elements in motion_state_speed_bounds */
  float motion_state_speed_bounds[SNS_SAM_DISTANCE_BOUND_MOTION_STATE_MAX_V01];
  /**<  
    An array that specifies a maximum speed associated with each motion state
    defined in meters per second. If not provided, the algorithm will
    use default values. The sns_sam_distance_bound_ms_e specifies the index of
    each motion state in the array motion_state_speed_bounds array.
    */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type                  = SNS_PROC_MODEM
       notify_suspend->send_indications_during_suspend  = TRUE

       This field does not have any bearing on error indication
       messages, which will be sent even during suspend.
    */
}sns_sam_distance_bound_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables the distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<  
    Algorithm instance ID maintained/assigned by SAM.
    The client shall use this instance ID for future messages associated with
    current algorithm instance.
    */
}sns_sam_distance_bound_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables the distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   To identify the algorithm instance to be disabled.  */
}sns_sam_distance_bound_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables the distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_distance_bound_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output of distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_distance_bound_get_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output of distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp of input with which latest distance bound report was computed
  in SSC ticks  */

  /* Optional */
  uint8_t report_data_valid;  /**< Must be set to true if report_data is being passed */
  sns_sam_distance_bound_report_data_s_v01 report_data;
  /**<   distance_bound algorithm output report  */

  /* Optional */
  uint8_t distance_accumulated_valid;  /**< Must be set to true if distance_accumulated is being passed */
  float distance_accumulated;
  /**<  
    Distance accumulated in meters since the last sns_sam_distance_bound_set_bound_req
  */
}sns_sam_distance_bound_get_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Indication Message; Output report from the distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of input with which latest distance bound report was computed
  in SSC ticks */

  /* Mandatory */
  sns_sam_distance_bound_report_data_s_v01 report_data;
  /**<   distance_bound algorithm output report  */

  /* Optional */
  uint8_t distance_accumulated_valid;  /**< Must be set to true if distance_accumulated is being passed */
  float distance_accumulated;
  /**<  
    Distance accumulated in meters since the last sns_sam_distance_bound_set_bound_req
  */
}sns_sam_distance_bound_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error report from the distance_bound algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of when the error was detected; in SSC ticks */

  /* Mandatory */
  uint8_t error;
  /**<   sensors error code */
}sns_sam_distance_bound_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Request Message; This COMMAND sets the distance bound. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  float distance_bound;
  /**<  
    The distance bound in meters. When the distance travelled exceeds this,
    an event is sent to the requestor, and the distance travelled accumulation
    is reset.
    */

  /* Mandatory */
  uint8_t session_key;
  /**<  
    A unique key provided by the client that identifies the session associated
    with the sns_sam_distance_bound_set_bound_req_msg. A session is defined as
    the period between the arrival of a sns_sam_distance_bound_set_bound_req_msg
    and the generation of a sns_sam_distance_bound_report_ind_msg associated with
    it. There can be only one session active at a time. The arrival of a new
    sns_sam_distance_bound_set_bound_req_msg discards the previous session. The
    session_key returned with the sns_sam_distance_bound_report_ind_msg can be used
    by a client to identify the request associated with it.
    */
}sns_sam_distance_bound_set_bound_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_messages
    @{
  */
/** Response Message; This COMMAND sets the distance bound. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_distance_bound_set_bound_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_CANCEL_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_DISABLE_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_ENABLE_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_ERROR_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_GET_REPORT_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_REPORT_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_SET_BOUND_V01 
//#define REMOVE_SNS_SAM_DISTANCE_BOUND_VERSION_V01 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_DISTANCE_BOUND_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_DISTANCE_BOUND_CANCEL_REQ_V01 0x0000
#define SNS_SAM_DISTANCE_BOUND_CANCEL_RESP_V01 0x0000
#define SNS_SAM_DISTANCE_BOUND_VERSION_REQ_V01 0x0001
#define SNS_SAM_DISTANCE_BOUND_VERSION_RESP_V01 0x0001
#define SNS_SAM_DISTANCE_BOUND_ENABLE_REQ_V01 0x0002
#define SNS_SAM_DISTANCE_BOUND_ENABLE_RESP_V01 0x0002
#define SNS_SAM_DISTANCE_BOUND_DISABLE_REQ_V01 0x0003
#define SNS_SAM_DISTANCE_BOUND_DISABLE_RESP_V01 0x0003
#define SNS_SAM_DISTANCE_BOUND_GET_REPORT_REQ_V01 0x0004
#define SNS_SAM_DISTANCE_BOUND_GET_REPORT_RESP_V01 0x0004
#define SNS_SAM_DISTANCE_BOUND_REPORT_IND_V01 0x0005
#define SNS_SAM_DISTANCE_BOUND_ERROR_IND_V01 0x0006
#define SNS_SAM_DISTANCE_BOUND_SET_BOUND_REQ_V01 0x0020
#define SNS_SAM_DISTANCE_BOUND_SET_BOUND_RESP_V01 0x0020
#define SNS_SAM_DISTANCE_BOUND_GET_ATTRIBUTES_REQ_V01 0x0024
#define SNS_SAM_DISTANCE_BOUND_GET_ATTRIBUTES_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_DISTANCE_BOUND_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_DISTANCE_BOUND_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_DISTANCE_BOUND_SVC_get_service_object_v01( ) \
          SNS_SAM_DISTANCE_BOUND_SVC_get_service_object_internal_v01( \
            SNS_SAM_DISTANCE_BOUND_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_DISTANCE_BOUND_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_DISTANCE_BOUND_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

