#ifndef SNS_SMGR_INTERNAL_SVC_SERVICE_02_H
#define SNS_SMGR_INTERNAL_SVC_SERVICE_02_H
/**
  @file sns_smgr_internal_api_v02.h

  @brief This is the public header file which defines the SNS_SMGR_INTERNAL_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SMGR_INTERNAL_SVC. It contains the constant values defined, enums, structures,
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

/* This file was generated with Tool version 6.10 
   It was generated on: Mon Jul 21 2014 (Spin 0)
   From IDL File: sns_smgr_internal_api_v02.idl */

/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"
#include "sns_smgr_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SMGR_INTERNAL_SVC_V02_IDL_MAJOR_VERS 0x02
/** Revision Number of the IDL used to generate this file */
#define SNS_SMGR_INTERNAL_SVC_V02_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SMGR_INTERNAL_SVC_V02_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SMGR_INTERNAL_SVC_V02_MAX_MESSAGE_ID 0x0026
/**
    @}
  */


/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_consts 
    @{ 
  */

/**  SMGR Internal Service QMI ID */
#define SNS_SMGR_INTERNAL_SVC_ID_V02 SNS_QMI_SVC_ID_13_V01

/** 
 maximum number of sensor events supported by SMGR */
#define SNS_MAX_NUM_SENSOR_EVENTS_V02 200

/**  Sensor Event IDs */
#define SNS_SMGR_SENSOR_EVENT_ID_MOTION_DETECT_1_V02 0xd324dc568eb58fb4
#define SNS_SMGR_SENSOR_EVENT_ID_PROXIMITY_1_V02 0xbdee7f234f56769e
#define SNS_SMGR_SENSOR_EVENT_ID_PROXIMITY_FAR_1_V02 0xb3e0ec7c65d43769
#define SNS_SMGR_SENSOR_EVENT_ID_PROXIMITY_NEAR_1_V02 0x9ee4971672d356fe

/**  SMGR Internal Service QMI ID */
#define SNS_SMGR_INTERNAL_SVC_ID_V02 SNS_QMI_SVC_ID_13_V01
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_SENSOR_EVENT_PROXIMITY_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_SENSOR_EVENT_PROXIMITY_FAR_V02 = 0, 
  SNS_SMGR_SENSOR_EVENT_PROXIMITY_NEAR_V02 = 1, 
  SNS_SMGR_SENSOR_EVENT_PROXIMITY_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_sensor_event_proximity_e_v02;
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_SENSOR_EVENT_STATUS_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_SENSOR_EVENT_STATUS_ENABLED_V02 = 0, 
  SNS_SMGR_SENSOR_EVENT_STATUS_DISABLED_V02 = 1, 
  SNS_SMGR_SENSOR_EVENT_STATUS_OCCURRED_V02 = 2, 
  SNS_SMGR_SENSOR_EVENT_STATUS_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_sensor_event_status_e_v02;
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SMGR_EVENT_GATED_BUFFERING_ACTION_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SMGR_EVENT_GATED_BUFFERING_ACTION_ADD_V02 = 1, 
  SNS_SMGR_EVENT_GATED_BUFFERING_ACTION_DELETE_V02 = 2, 
  SNS_SMGR_EVENT_GATED_BUFFERING_ACTION_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_smgr_event_gated_buffering_action_e_v02;
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sns_smgr_sensor_events_query_req_msg_v02;

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command sends sensor events query request and
           gets a list of all supported sensor events */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t sensor_events_valid;  /**< Must be set to true if sensor_events is being passed */
  uint32_t sensor_events_len;  /**< Must be set to # of elements in sensor_events */
  uint64_t sensor_events[SNS_MAX_NUM_SENSOR_EVENTS_V02];
}sns_smgr_sensor_events_query_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command sends sensor event register request and
           gets sensor event indication whenever the event occurs */
typedef struct {

  /* Mandatory */
  uint64_t sensor_event;
  /**<   The sensor event for which to register*/

  /* Mandatory */
  uint8_t registering;
  /**<   TRUE to register, FALSE deregister*/

  /* Mandatory */
  uint32_t shortest_interval;
  /**<   Shortest time, in milliseconds, between two consecutive events*/
}sns_smgr_sensor_event_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command sends sensor event register request and
           gets sensor event indication whenever the event occurs */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
}sns_smgr_sensor_event_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Indication Message; This command sends sensor event register request and
           gets sensor event indication whenever the event occurs */
typedef struct {

  /* Mandatory */
  uint64_t sensor_event;
  /**<   The sensor event that has just occurred*/

  /* Mandatory */
  uint64_t timestamp;
  /**<   The timestamp, in ticks, when the event occurred*/

  /* Optional */
  uint8_t event_info_valid;  /**< Must be set to true if event_info is being passed */
  uint64_t event_info;
  /**<   
    If sensor_event is SNS_SMGR_SENSOR_EVENT_ID_MOTION_DETECT, this field is unused
 
    If sensor_event is SNS_SMGR_SENSOR_EVENT_ID_PROXIMITY, this field can be
    - SNS_SMGR_SENSOR_EVENT_PROXIMITY_FAR or
    - SNS_SMGR_SENSOR_EVENT_PROXIMITY_NEAR
  */

  /* Optional */
  uint8_t event_status_valid;  /**< Must be set to true if event_status is being passed */
  sns_smgr_sensor_event_status_e_v02 event_status;
  /**<  
    The current status for the sensor event. Possible values are
    -SNS_SMGR_SENSOR_EVENT_STATUS_ENABLED
    -SNS_SMGR_SENSOR_EVENT_STATUS_DISABLED
    -SNS_SMGR_SENSOR_EVENT_STATUS_OCCURRED
  */
}sns_smgr_sensor_event_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command sends sensor monitor request and receives indications
    when sensor sampling rate, and/or wakeup rate, and/or number of clients
    change. */
typedef struct {

  /* Mandatory */
  uint64_t sensor_id;
  /**<   SNS_SMGR_SENSOR_ID_xxx*/

  /* Mandatory */
  uint8_t registering;
  /**<   TRUE to register, FALSE deregister*/
}sns_smgr_sensor_status_monitor_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command sends sensor monitor request and receives indications
    when sensor sampling rate, and/or wakeup rate, and/or number of clients
    change. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t sensor_id_valid;  /**< Must be set to true if sensor_id is being passed */
  uint64_t sensor_id;
  /**<   SNS_SMGR_SENSOR_ID_xxx*/
}sns_smgr_sensor_status_monitor_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Indication Message; This command sends sensor monitor request and receives indications
    when sensor sampling rate, and/or wakeup rate, and/or number of clients
    change. */
typedef struct {

  /* Mandatory */
  uint64_t sensor_id;
  /**<   SNS_SMGR_SENSOR_ID_xxx*/

  /* Mandatory */
  uint8_t num_clients;
  /**<   Number of clients this sensor currently has*/

  /* Mandatory */
  int32_t sampling_rate;
  /**<   The current sampling rate in units of Hz, Q16 format*/

  /* Mandatory */
  int32_t wakeup_rate;
  /**<   The current wake up rate in units of Hz, Q16 format*/

  /* Mandatory */
  uint64_t timestamp;
  /**<   The timestamp, in ticks, when rates and/or number of clients change*/
}sns_smgr_sensor_status_monitor_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; 
  This command requests sensor data to be sampled and buffered up to be sent together
  with a configuration determined by a sensor event. */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   The report ID assigned by client to be used for identifying corresponding
    response and indication messages. An existing report will be replaced by a new
    report of the same ID.
    */

  /* Mandatory */
  sns_smgr_event_gated_buffering_action_e_v02 Action;
  /**<   Specifies the action to be carried out for this report
    - SNS_SMGR_EVENT_GATED_BUFFERING_ACTION_ADD
    - SNS_SMGR_EVENT_GATED_BUFFERING_ACTION_DELETE
    - All other values will be rejected.
  */

  /* Mandatory */
  uint64_t SensorEvent;
  /**<     The sensor event on which to gate the sensor stream. The requested stream
    cannot prevent this event from being enabled by SMGR
  */

  /* Mandatory */
  sns_smgr_buffering_req_item_s_v01 EventDisabledConfig;
  /**<         If the Sensor event is disabled, SMGR will use this configuration.
  */

  /* Mandatory */
  uint32_t EventDisabledReportRate;
  /**<    If the Sensor event is disabled, SMGR will use this Report Rate. In Q16
     format and in uit of Hz.
  */

  /* Mandatory */
  sns_smgr_buffering_req_item_s_v01 EventEnabledConfig;
  /**<         If the sensor event is enabled, SMGR will use this configuration
  */

  /* Mandatory */
  uint32_t EventEnabledReportRate;
  /**<    If the Sensor event is enabled, SMGR will use this Report Rate. In Q16
     format and in uit of Hz.
  */

  /* Mandatory */
  sns_smgr_buffering_req_item_s_v01 EventOccurredConfig;
  /**<         If a sensor event occurs, SMGR will ignore any further event state changes
             for this request and continue to use this configuration until the request
             is cancelled or is updated with a new request message.
  */

  /* Mandatory */
  uint32_t EventOccurredReportRate;
  /**<    If a sensor event occurs, SMGR will use this Report Rate. In Q16
     format and in uit of Hz.
  */

  /* Mandatory */
  sns_suspend_notification_s_v01 NotifySuspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type                  = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend  = FALSE
    */
}sns_smgr_event_gated_buffering_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; 
  This command requests sensor data to be sampled and buffered up to be sent together
  with a configuration determined by a sensor event. */
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
}sns_smgr_event_gated_buffering_resp_msg_v02;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_COMMON_CANCEL_V02 
//#define REMOVE_SNS_COMMON_VERSION_V02 
//#define REMOVE_SNS_SMGR_BUFFERING_IND_V02 
//#define REMOVE_SNS_SMGR_EVENT_GATED_BUFFERING_V02 
//#define REMOVE_SNS_SMGR_SENSOR_EVENT_V02 
//#define REMOVE_SNS_SMGR_SENSOR_EVENTS_QUERY_V02 
//#define REMOVE_SNS_SMGR_SENSOR_STATUS_MONITOR_V02 

/*Service Message Definition*/
/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_msg_ids
    @{
  */
#define SNS_SMGR_CANCEL_REQ_V02 0x0000
#define SNS_SMGR_CANCEL_RESP_V02 0x0000
#define SNS_SMGR_VERSION_REQ_V02 0x0001
#define SNS_SMGR_VERSION_RESP_V02 0x0001
#define SNS_SMGR_SENSOR_EVENTS_QUERY_REQ_V02 0x0020
#define SNS_SMGR_SENSOR_EVENTS_QUERY_RESP_V02 0x0020
#define SNS_SMGR_SENSOR_EVENT_REQ_V02 0x0021
#define SNS_SMGR_SENSOR_EVENT_RESP_V02 0x0021
#define SNS_SMGR_SENSOR_EVENT_IND_V02 0x0022
#define SNS_SMGR_SENSOR_STATUS_MONITOR_REQ_V02 0x0023
#define SNS_SMGR_SENSOR_STATUS_MONITOR_RESP_V02 0x0023
#define SNS_SMGR_SENSOR_STATUS_MONITOR_IND_V02 0x0024
#define SNS_SMGR_EVENT_GATED_BUFFERING_REQ_V02 0x0025
#define SNS_SMGR_EVENT_GATED_BUFFERING_RESP_V02 0x0025
#define SNS_SMGR_EVENT_GATED_BUFFERING_IND_V02 0x0026
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SMGR_INTERNAL_SVC_get_service_object_v02( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SMGR_INTERNAL_SVC_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SMGR_INTERNAL_SVC_get_service_object_v02( ) \
          SNS_SMGR_INTERNAL_SVC_get_service_object_internal_v02( \
            SNS_SMGR_INTERNAL_SVC_V02_IDL_MAJOR_VERS, SNS_SMGR_INTERNAL_SVC_V02_IDL_MINOR_VERS, \
            SNS_SMGR_INTERNAL_SVC_V02_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

