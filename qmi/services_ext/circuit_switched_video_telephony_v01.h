#ifndef CSVT_SERVICE_01_H
#define CSVT_SERVICE_01_H
/**
  @file circuit_switched_video_telephony_v01.h
  
  @brief This is the public header file which defines the csvt service Data structures.

  This header file defines the types and structures that were defined in 
  csvt. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Sat Aug 24 2013 (Spin 0)
   From IDL File: circuit_switched_video_telephony_v01.idl */

/** @defgroup csvt_qmi_consts Constant values defined in the IDL */
/** @defgroup csvt_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup csvt_qmi_enums Enumerated types used in QMI messages */
/** @defgroup csvt_qmi_messages Structures sent as QMI messages */
/** @defgroup csvt_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup csvt_qmi_accessor Accessor for QMI service object */
/** @defgroup csvt_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup csvt_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define CSVT_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define CSVT_V01_IDL_MINOR_VERS 0x06
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define CSVT_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define CSVT_V01_MAX_MESSAGE_ID 0x002E;
/** 
    @} 
  */


/** @addtogroup csvt_qmi_consts 
    @{ 
  */
#define CSVT_MAX_DIAL_STRING_LEN_V01 80
#define CSVT_MAX_UUS_ID_LEN_V01 32
#define CSVT_MAX_INCOM_NUM_LEN_V01 16
#define CSVT_MAX_ACTIVE_CALL_V01 2
/**
    @}
  */

typedef uint64_t csvt_data_call_type_mask_v01;
#define CSVT_MASK_ASYNC_CSD_CALL_V01 ((csvt_data_call_type_mask_v01)0x01ull) 
#define CSVT_MASK_SYNC_CSD_CALL_V01 ((csvt_data_call_type_mask_v01)0x02ull) 
#define CSVT_MASK_VIDEO_TELEPHONY_CALL_V01 ((csvt_data_call_type_mask_v01)0x08ull) 
/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_DATA_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_TYPE_ASYNC_CSD_CALL_V01 = 0x01, 
  CSVT_TYPE_SYNC_CSD_CALL_V01 = 0x02, 
  CSVT_TYPE_VIDEO_TELEPHONY_CALL_V01 = 0x08, 
  CSVT_DATA_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_data_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_CALL_MODE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_CALL_MODE_DATA_V01 = 0x01, 
  CSVT_CALL_MODE_DATA_VOICE_V01 = 0x02, 
  CSVT_CALL_MODE_VOICE_DATA_V01 = 0x04, 
  CSVT_CALL_MODE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_call_mode_type_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_UUS_ID_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_UUS_TYPE_EMAIL_V01 = 0x01, 
  CSVT_UUS_TYPE_URL_V01 = 0x02, 
  CSVT_UUS_TYPE_H323_V01 = 0x03, 
  CSVT_UUS_ID_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_uus_id_type_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_EVENT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_EVENT_TYPE_CONFIRM_V01 = 0x1, 
  CSVT_EVENT_TYPE_PROGRESS_V01 = 0x2, 
  CSVT_EVENT_TYPE_CONNECT_V01 = 0x3, 
  CSVT_EVENT_TYPE_SETUP_V01 = 0x4, 
  CSVT_EVENT_TYPE_INCOMING_V01 = 0x5, 
  CSVT_EVENT_TYPE_END_V01 = 0x6, 
  CSVT_EVENT_TYPE_MODIFY_V01 = 0x7, 
  CSVT_EVENT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_event_type_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_TRANSPARENT_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_TRANSPARENT_CALL_V01 = 0x00, 
  CSVT_NON_TRANSPARENT_CALL_V01 = 0x01, 
  CSVT_TRANSPARENT_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_transparent_call_type_enum_v01;
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}csvt_reset_req_msg_v01;

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Resets the CSVT service state variables of the requesting control
           point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Sets the event report preference of the requesting control point. */
typedef struct {

  /* Optional */
  /*  Report Call Events */
  uint8_t report_call_events_valid;  /**< Must be set to true if report_call_events is being passed */
  uint8_t report_call_events;
  /**<   Values: \n
       - 0x00 -- Do not report call events \n
       - 0x01 -- Report call events
    */

  /* Optional */
  /*  Call Type Mask */
  uint8_t call_types_valid;  /**< Must be set to true if call_types is being passed */
  csvt_data_call_type_mask_v01 call_types;
  /**<   Values: \n
       - 0x01 -- Asynchronous CSVT call \n
       - 0x02 -- Synchronous CSVT call \n
       - 0x08 -- Videotelephony call
    */
}csvt_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Sets the event report preference of the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_DATA_NETWORK_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_DATA_NETWORK_GSM_V01 = 0x03, 
  CSVT_DATA_NETWORK_WCDMA_V01 = 0x05, 
  CSVT_DATA_NETWORK_TDSCDMA_V01 = 0x0B, 
  CSVT_DATA_NETWORK_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_data_network_type_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_DATA_PORT_FAMILY_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_DATA_PORT_SMD_FAMILY_V01 = 0x04, 
  CSVT_DATA_PORT_A2_FAMILY_V01 = 0x0E, 
  CSVT_DATA_PORT_MUX_FAMILY_V01 = 0x0F, 
  CSVT_DATA_PORT_FAMILY_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_data_port_family_type_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_aggregates
    @{
  */
typedef struct {

  csvt_data_port_family_type_enum_v01 port_family;
  /**<   Values: \n
       - 0x04 -- SMD \n
       - 0x0E -- A2 \n
       - 0x0F -- MUX
    */

  uint32_t port_number;
  /**<   Port number. */
}csvt_port_id_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csvt_qmi_aggregates
    @{
  */
typedef struct {

  csvt_uus_id_type_enum_v01 uus_id_type;
  /**<   Values: \n
       - 0x01 -- E-mail \n
       - 0x02 -- URL \n
       - 0x03 -- H.323
    */

  char uus_id[CSVT_MAX_UUS_ID_LEN_V01 + 1];
  /**<   User-to-user signaling ID. */
}csvt_uus_id_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the state of CSVT. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the underlying CSD call. */

  /* Mandatory */
  /*  Event Type */
  csvt_event_type_enum_v01 event_type;
  /**<   List of CSD call events. Values: \n
       - 0x01 -- Call confirmation event \n
       - 0x02 -- Call progress notification event \n
       - 0x03 -- Call connect notification event \n
       - 0x04 -- Call setup notification event \n
       - 0x05 -- Incoming call event \n
       - 0x06 -- Call end event \n
       - 0x07 -- Call modification event
    */

  /* Optional */
  /*  Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  csvt_data_call_type_enum_v01 call_type;
  /**<   Call type. Values: \n
       - 0x01 -- Asynchronous CSVT call \n
       - 0x02 -- Synchronous CSVT call \n
       - 0x08 -- Videotelephony call
    */

  /* Optional */
  /*  Synchronous Call */
  uint8_t synchronous_call_valid;  /**< Must be set to true if synchronous_call is being passed */
  uint8_t synchronous_call;
  /**<   Synchronous call. Values: \n
       - 0x00 -- Asynchronous call \n
       - 0x01 -- Synchronous call
    */

  /* Optional */
  /*  Transparent Call */
  uint8_t transparent_call_valid;  /**< Must be set to true if transparent_call is being passed */
  csvt_transparent_call_type_enum_v01 transparent_call;
  /**<   Transparent call. Values: \n
       - 0x00 -- Transparent call \n
       - 0x01 -- Nontransparent call
    */

  /* Optional */
  /*  Network Type */
  uint8_t network_type_valid;  /**< Must be set to true if network_type is being passed */
  csvt_data_network_type_enum_v01 network_type;
  /**<   Network type. Values: \n
       - 0x03 -- GSM \n
       - 0x05 -- WCDMA \n
       - 0x0B -- TDS
    */

  /* Optional */
  /*  Network Speed */
  uint8_t network_speed_valid;  /**< Must be set to true if network_speed is being passed */
  uint16_t network_speed;
  /**<   Network speed (see \hyperref[S1]{[S1]}, Section 6.7).
  */

  /* Optional */
  /*  Maximum Frame Size */
  uint8_t max_frame_size_valid;  /**< Must be set to true if max_frame_size is being passed */
  uint8_t max_frame_size;
  /**<   Maximum frame size (in bytes).
  */

  /* Optional */
  /*  Incoming Number */
  uint8_t incoming_number_valid;  /**< Must be set to true if incoming_number is being passed */
  char incoming_number[CSVT_MAX_INCOM_NUM_LEN_V01 + 1];
  /**<   Incoming number.
  */

  /* Optional */
  /*  UUS ID (user-to-user signaling ID) */
  uint8_t uus_id_valid;  /**< Must be set to true if uus_id is being passed */
  csvt_uus_id_v01 uus_id;

  /* Optional */
  /*  Modify Allowed */
  uint8_t modify_allowed_valid;  /**< Must be set to true if modify_allowed is being passed */
  uint8_t modify_allowed;
  /**<   Subsequent call modification. Values: \n
       - 0x00 -- Modification not allowed \n
       - 0x01 -- Modification allowed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */

  /* Optional */
  /*  Port Data */
  uint8_t port_data_valid;  /**< Must be set to true if port_data is being passed */
  csvt_port_id_type_v01 port_data;

  /* Optional */
  /*  Incoming Number Length */
  uint8_t incoming_number_length_valid;  /**< Must be set to true if incoming_number_length is being passed */
  uint32_t incoming_number_length;
  /**<   Incoming number length.
  */
}csvt_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Originates a new CSVT call. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the underlying CSD call.
  */

  /* Mandatory */
  /*  Call Mode */
  csvt_call_mode_type_enum_v01 call_mode;
  /**<   Call mode. Values: \n
       - 0x01 -- Data-only mode \n
       - 0x02 -- Voice + Data mode \n
       - 0x04 -- Data + Voice mode
    */

  /* Mandatory */
  /*  Dial String */
  char dial_string[CSVT_MAX_DIAL_STRING_LEN_V01 + 1];
  /**<   CSVT dial string.
  */

  /* Optional */
  /*  Network Datarate */
  uint8_t network_datarate_valid;  /**< Must be set to true if network_datarate is being passed */
  uint8_t network_datarate;
  /**<   Network datarate (see \hyperref[S1]{[S1]}, Section 6.7). \n
       Default value is 64 kbps.
  */

  /* Optional */
  /*  Air Interface Datarate */
  uint8_t air_interface_datarate_valid;  /**< Must be set to true if air_interface_datarate is being passed */
  uint8_t air_interface_datarate;
  /**<   Air interface datarate (see \hyperref[S1]{[S1]}, Section 6.7). \n
       Default value is 64 kbps.
  */

  /* Optional */
  /*  Synchronous Call */
  uint8_t synchronous_call_valid;  /**< Must be set to true if synchronous_call is being passed */
  uint8_t synchronous_call;
  /**<   Synchronous call. Values: \n
       - 0x00 -- Asynchronous call \n
       - 0x01 -- Synchronous call \n
       Default call type is Synchronous.
  */

  /* Optional */
  /*  Transparent Call */
  uint8_t transparent_call_valid;  /**< Must be set to true if transparent_call is being passed */
  csvt_transparent_call_type_enum_v01 transparent_call;
  /**<   Transparent call. Values: \n
       - 0x00 -- Transparent call \n
       - 0x01 -- Nontransparent call \n
       Default call type is Transparent.
  */

  /* Optional */
  /*  CLI Enabled */
  uint8_t cli_enabled_valid;  /**< Must be set to true if cli_enabled is being passed */
  uint8_t cli_enabled;
  /**<   CLI enabled. Values: \n
       - 0x00 -- Disable CLI \n
       - 0x01 -- Enable CLI \n
       Note: CLI is disabled by default.
  */

  /* Optional */
  /*  CUG Enabled */
  uint8_t cug_enabled_valid;  /**< Must be set to true if cug_enabled is being passed */
  uint8_t cug_enabled;
  /**<   CUG enabled. Values: \n
       - 0x00 -- Disable CUG \n
       - 0x01 -- Enable CUG \n
       Note: CUG is disabled by default.
  */

  /* Optional */
  /*  CUG Index */
  uint8_t cug_index_valid;  /**< Must be set to true if cug_index is being passed */
  uint8_t cug_index;
  /**<   CUG index (see \hyperref[S1]{[S1]}, Section 7.10). \n
       Default value is 0.
  */

  /* Optional */
  /*  Suppress Preferred CUG */
  uint8_t supress_preferred_cug_valid;  /**< Must be set to true if supress_preferred_cug is being passed */
  uint8_t supress_preferred_cug;
  /**<   Suppress preferred CUG. Values: \n
       - 0x00 -- Do not suppress CUG \n
       - 0x01 -- Suppress CUG \n
       Note: Preferred CUG is suppressed by default.
  */

  /* Optional */
  /*  Suppress Outgoing Access */
  uint8_t supress_outgoing_access_valid;  /**< Must be set to true if supress_outgoing_access is being passed */
  uint8_t supress_outgoing_access;
  /**<   Suppress outgoing access. Values: \n
       - 0x00 -- Do not suppress outgoing access \n
       - 0x01 -- Suppress outgoing access \n
       Note: Outgoing access is suppressed by default.
  */

  /* Optional */
  /*  UUS ID */
  uint8_t uus_id_valid;  /**< Must be set to true if uus_id is being passed */
  csvt_uus_id_v01 uus_id;
}csvt_originate_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Originates a new CSVT call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_originate_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates the execution status of a CSVT call that the control point
           had originated. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID.
  */

  /* Mandatory */
  /*  Request Status */
  uint8_t request_status;
  /**<   Request status. Values: \n
       - 0x0 -- Request processing succeeded \n
       - 0x1 -- Request processing failed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */
}csvt_originate_call_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Confirms an incoming CSVT call. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the call to be confirmed.
  */

  /* Mandatory */
  /*  Confirm Call */
  uint8_t confirm_call;
  /**<   Confirm call. Values: \n
       - 0x00 -- Reject the call \n
       - 0x01 -- Confirm the call
    */

  /* Optional */
  /*  Reject Value */
  uint8_t reject_value_valid;  /**< Must be set to true if reject_value is being passed */
  uint8_t reject_value;
  /**<   Reject_value Values: \n
       Call control cause as defined 
       in GSM 04.08 Table 10.86 constants
  */
}csvt_confirm_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Confirms an incoming CSVT call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_confirm_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates the execution status of a CSVT call that the control point
           had confirmed. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID.
  */

  /* Mandatory */
  /*  Request Status */
  uint8_t request_status;
  /**<   Request status. Values: \n
       - 0x0 - Request processing succeeded \n
   - 0x1 - Request processing failed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */
}csvt_confirm_call_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Answers an incoming call. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the call to be answered.
  */

  /* Mandatory */
  /*  Answer Call */
  uint8_t answer_call;
  /**<   Answer call. Values: \n
       - 0x00 -- Reject the call \n
       - 0x01 -- Answer the call
  */

  /* Optional */
  /*  Reject Value */
  uint8_t reject_value_valid;  /**< Must be set to true if reject_value is being passed */
  uint8_t reject_value;
  /**<   Reject_value Values: \n
       Call control cause as defined 
       in GSM 04.08 Table 10.86 constants
    */
}csvt_answer_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Answers an incoming call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_answer_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates the execution status of a CSVT call that the control point
           had answered. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID.
  */

  /* Mandatory */
  /*  Request Status */
  uint8_t request_status;
  /**<   Request status. Values: \n
       - 0x0 -- Request processing succeeded \n
       - 0x1 -- Request processing failed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */
}csvt_answer_call_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Ends an ongoing CSVT call that had been confirmed earlier by this
           control point. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the call to be terminated.
  */
}csvt_end_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Ends an ongoing CSVT call that had been confirmed earlier by this
           control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_end_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates the execution status of a CSVT call that the control point
           had ended. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID.
  */

  /* Mandatory */
  /*  Request Status */
  uint8_t request_status;
  /**<   Request status. Values: \n
       - 0x0 -- Request processing succeeded \n
       - 0x1 -- Request processing failed
    */
}csvt_end_call_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Modifies an ongoing call. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the underlying CSD call.
  */

  /* Mandatory */
  /*  Call Type */
  csvt_data_call_type_enum_v01 new_call_type;
  /**<   Call type. Values: \n
       - 0x01 -- Asynchronous CSVT call \n
       - 0x02 -- Synchronous CSVT call \n
       - 0x08 -- Videotelephony call
    */
}csvt_modify_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Modifies an ongoing call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_modify_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates the execution status of a CSVT call that the control point
           had modified. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID.
  */

  /* Mandatory */
  /*  Request Status */
  uint8_t request_status;
  /**<   Request status. Values: \n
       - 0x0 -- Request processing succeeded \n
       - 0x1 -- Request processing failed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */
}csvt_modify_call_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Accepts a network-initiated call modification. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the underlying CSD call.
  */

  /* Mandatory */
  /*  Accept Request */
  uint8_t accept_request;
  /**<   Accept request. Values: \n
       - 0x00 -- Reject the modification \n
       - 0x01 -- Accept the modification
    */
}csvt_ack_call_modify_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Accepts a network-initiated call modification. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_ack_call_modify_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Indication Message; Indicates the execution status of a network-initiated CSVT call
           modification that the control point had acknowledged. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID.
  */

  /* Mandatory */
  /*  Request Status */
  uint8_t request_status;
  /**<   Request status. Values: \n
       - 0x0 -- Request processing succeeded \n
       - 0x1 -- Request processing failed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */
}csvt_ack_call_modify_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t rlp_version;
  /**<   RLP version.
  */

  uint32_t rlp_tx_window_size;
  /**<   Outgoing window size (specified as number of frames). Values: \n
       - Minimum -- 1 \n
       - Maximum -- 61 for RLP Ver 1 and Ver 2, and 488 for RLP Ver 3
    */

  uint32_t rlp_rx_window_size;
  /**<   Incoming window size (specified as number of frames). Values: \n
       - Minimum -- 1 \n
       - Maximum -- 61 for RLP Ver 1 and Ver 2, and 488 for RLP Ver 3
    */

  uint32_t rlp_ack_timer;
  /**<   RLP acknowledgement timer (in hundredths of seconds). Values: \n
       - Minimum -- 38 for RLP Ver 1 and Ver 2, and 41 for RLP Ver 3 \n
       - Maximum -- 255
    */

  uint32_t rlp_retrans_attempts;
  /**<   Number of RLP retransmission attempts. Values: \n
       - Minimum -- 1 \n
       - Maximum -- 255
    */

  uint32_t rlp_reseq_timer;
  /**<   RLP resequencing timer (in hundredths of seconds). Values: \n
       - Minimum -- 38 for RLP Ver 1 and Ver 2, and 41 for RLP Ver 3 \n
       - Maximum -- 255
    */
}rlp_params_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t v42_direction;
  /**<   Direction to enable compression. Values: \n
       - 0x01 -- For originated calls \n
       - 0x02 -- For answered calls
    */

  uint32_t v42_negotiation;
  /**<   V42 negotiation preference. Values: \n
       - 0x00 -- Compression is enabled when possible \n
       - 0x01 -- Compression must always be enabled
    */

  uint32_t v42_max_dict;
  /**<   V42.bis maximum dictionary size.
  */

  uint32_t v42_max_string;
  /**<   V42.bis maximum string size.
  */
}v42_params_v01;  /* Type */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}csvt_get_rlp_params_req_msg_v01;

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Queries the current active settings and parameters for Radio Link
           Protocol (RLP) and V42.bis compression. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  RLP1 Parameters */
  uint8_t rlp1_parameters_valid;  /**< Must be set to true if rlp1_parameters is being passed */
  rlp_params_v01 rlp1_parameters;

  /* Optional */
  /*  V42 Parameters */
  uint8_t v42_parameters_valid;  /**< Must be set to true if v42_parameters is being passed */
  v42_params_v01 v42_parameters;

  /* Optional */
  /*  RLP2 Parameters */
  uint8_t rlp2_parameters_valid;  /**< Must be set to true if rlp2_parameters is being passed */
  rlp_params_v01 rlp2_parameters;

  /* Optional */
  /*  RLP3 Parameters */
  uint8_t rlp3_parameters_valid;  /**< Must be set to true if rlp3_parameters is being passed */
  rlp_params_v01 rlp3_parameters;
}csvt_get_rlp_params_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Sets the nontransparent RLP parameters for subsequent CSVT calls
           made by the control point. */
typedef struct {

  /* Optional */
  /*  RLP Parameters */
  uint8_t rlp_parameters_valid;  /**< Must be set to true if rlp_parameters is being passed */
  rlp_params_v01 rlp_parameters;

  /* Optional */
  /*  V42 Parameters */
  uint8_t v42_parameters_valid;  /**< Must be set to true if v42_parameters is being passed */
  v42_params_v01 v42_parameters;
}csvt_set_rlp_params_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Sets the nontransparent RLP parameters for subsequent CSVT calls
           made by the control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_set_rlp_params_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}csvt_get_active_call_list_req_msg_v01;

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Queries the list of the current active CSVT calls. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Active Call List */
  uint8_t active_call_inst_id_valid;  /**< Must be set to true if active_call_inst_id is being passed */
  uint32_t active_call_inst_id_len;  /**< Must be set to # of elements in active_call_inst_id */
  uint32_t active_call_inst_id[CSVT_MAX_ACTIVE_CALL_V01];
  /**<   Instance IDs of active calls.
  */
}csvt_get_active_call_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Queries the call type for the given call instance. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the underlying CSD call.
  */
}csvt_get_call_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Queries the call type for the given call instance. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  csvt_data_call_type_enum_v01 call_type;
  /**<   Call type. Values: \n
       - 0x01 -- Asynchronous CSVT call \n
       - 0x02 -- Synchronous CSVT call \n
       - 0x08 -- Videotelephony call
    */

  /* Optional */
  /*  Synchronous Call */
  uint8_t synchronous_call_valid;  /**< Must be set to true if synchronous_call is being passed */
  uint8_t synchronous_call;
  /**<   Synchronous call. Values: \n
       - 0x00 - Asynchronous call \n
    - 0x01 - Synchronous call
    */

  /* Optional */
  /*  Transparent Call */
  uint8_t transparent_call_valid;  /**< Must be set to true if transparent_call is being passed */
  csvt_transparent_call_type_enum_v01 transparent_call;
  /**<   Transparent call. Values: \n
       - 0x00 -- Transparent call \n
       - 0x01 -- Nontransparent call
    */

  /* Optional */
  /*  Network Type */
  uint8_t network_type_valid;  /**< Must be set to true if network_type is being passed */
  csvt_data_network_type_enum_v01 network_type;
  /**<   Network type. Values: \n
       - 0x03 -- GSM \n
       - 0x05 -- WCDMA \n
       - 0x0B -- TDS
    */

  /* Optional */
  /*  Network Speed */
  uint8_t network_speed_valid;  /**< Must be set to true if network_speed is being passed */
  uint16_t network_speed;
  /**<   Guaranteed network speed (in kilobits per second).
  */

  /* Optional */
  /*  Maximum Frame Size */
  uint8_t max_frame_size_valid;  /**< Must be set to true if max_frame_size is being passed */
  uint8_t max_frame_size;
  /**<   Maximum frame size (in bytes).
  */

  /* Optional */
  /*  Incoming Number */
  uint8_t incoming_number_valid;  /**< Must be set to true if incoming_number is being passed */
  char incoming_number[CSVT_MAX_INCOM_NUM_LEN_V01 + 1];
  /**<   Incoming number.
  */

  /* Optional */
  /*  UUS ID */
  uint8_t uus_id_valid;  /**< Must be set to true if uus_id is being passed */
  csvt_uus_id_v01 uus_id;

  /* Optional */
  /*  Modify Allowed */
  uint8_t modify_allowed_valid;  /**< Must be set to true if modify_allowed is being passed */
  uint8_t modify_allowed;
  /**<   Subsequent call modification. Values: \n
       - 0x00 -- Modification not allowed \n
       - 0x01 -- Modification allowed
    */

  /* Optional */
  /*  Call End Cause */
  uint8_t call_end_cause_valid;  /**< Must be set to true if call_end_cause is being passed */
  uint32_t call_end_cause;
  /**<   Call end cause (see \hyperref[S2]{[S2]}).
  */

  /* Optional */
  /*  Port Data */
  uint8_t port_data_valid;  /**< Must be set to true if port_data is being passed */
  csvt_port_id_type_v01 port_data;
}csvt_get_call_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Queries the call statistics for the given call instance. */
typedef struct {

  /* Mandatory */
  /*  Instance ID */
  uint32_t instance_id;
  /**<   Instance ID of the underlying CSD call.
  */
}csvt_get_call_stats_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Queries the call statistics for the given call instance. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call Active */
  uint8_t call_active_valid;  /**< Must be set to true if call_active is being passed */
  uint8_t call_active;
  /**<   Call active. Values: \n
       - 0x00 -- Call is currently inactive \n
       - 0x01 -- Call is currently active
    */

  /* Optional */
  /*  Tx Counter */
  uint8_t tx_counter_valid;  /**< Must be set to true if tx_counter is being passed */
  uint32_t tx_counter;
  /**<   Number of bytes transferred.
  */

  /* Optional */
  /*  Rx Counter */
  uint8_t rx_counter_valid;  /**< Must be set to true if rx_counter is being passed */
  uint32_t rx_counter;
  /**<   Number of bytes received.
  */
}csvt_get_call_stats_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_enums
    @{
  */
typedef enum {
  CSVT_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CSVT_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary  */
  CSVT_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary  */
  CSVT_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary  */
  CSVT_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}csvt_bind_subscription_enum_v01;
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Request Message; Associates requesting control point to the subscription requested. */
typedef struct {

  /* Mandatory */
  /*  Bind subscription */
  csvt_bind_subscription_enum_v01 bind_subs;
  /**<   Subscription to bind to. Values: \n
      - CSVT_PRIMARY_SUBS (0x0001) --  Primary 
      - CSVT_SECONDARY_SUBS (0x0002) --  Secondary 
      - CSVT_TERTIARY_SUBS (0x0003) --  Tertiary 
 */
}csvt_set_subscription_binding_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Associates requesting control point to the subscription requested. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}csvt_set_subscription_binding_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}csvt_get_bind_subscription_req_msg_v01;

/** @addtogroup csvt_qmi_messages
    @{
  */
/** Response Message; Queries control point bound subscription. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Bound Subscription */
  uint8_t bind_subscription_valid;  /**< Must be set to true if bind_subscription is being passed */
  csvt_bind_subscription_enum_v01 bind_subscription;
  /**<   Values: \n
      - CSVT_PRIMARY_SUBS (0x0001) --  Primary 
      - CSVT_SECONDARY_SUBS (0x0002) --  Secondary 
      - CSVT_TERTIARY_SUBS (0x0003) --  Tertiary 
 */
}csvt_get_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup csvt_qmi_msg_ids
    @{
  */
#define QMI_CSVT_RESET_REQ_V01 0x0001
#define QMI_CSVT_RESET_RESP_V01 0x0001
#define QMI_CSVT_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_CSVT_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_CSVT_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_CSVT_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_CSVT_SET_EVENT_REPORT_REQ_V01 0x0020
#define QMI_CSVT_SET_EVENT_REPORT_RESP_V01 0x0020
#define QMI_CSVT_EVENT_REPORT_IND_V01 0x0021
#define QMI_CSVT_ORIGINATE_CALL_REQ_V01 0x0022
#define QMI_CSVT_ORIGINATE_CALL_RESP_V01 0x0022
#define QMI_CSVT_ORIGINATE_CALL_IND_V01 0x0022
#define QMI_CSVT_CONFIRM_CALL_REQ_V01 0x0023
#define QMI_CSVT_CONFIRM_CALL_RESP_V01 0x0023
#define QMI_CSVT_CONFIRM_CALL_IND_V01 0x0023
#define QMI_CSVT_ANSWER_CALL_REQ_V01 0x0024
#define QMI_CSVT_ANSWER_CALL_RESP_V01 0x0024
#define QMI_CSVT_ANSWER_CALL_IND_V01 0x0024
#define QMI_CSVT_END_CALL_REQ_V01 0x0025
#define QMI_CSVT_END_CALL_RESP_V01 0x0025
#define QMI_CSVT_END_CALL_IND_V01 0x0025
#define QMI_CSVT_MODIFY_CALL_REQ_V01 0x0026
#define QMI_CSVT_MODIFY_CALL_RESP_V01 0x0026
#define QMI_CSVT_MODIFY_CALL_IND_V01 0x0026
#define QMI_CSVT_ACK_CALL_MODIFY_REQ_V01 0x0027
#define QMI_CSVT_ACK_CALL_MODIFY_RESP_V01 0x0027
#define QMI_CSVT_ACK_CALL_MODIFY_IND_V01 0x0027
#define QMI_CSVT_GET_RLP_PARAMS_REQ_V01 0x0028
#define QMI_CSVT_GET_RLP_PARAMS_RESP_V01 0x0028
#define QMI_CSVT_SET_RLP_PARAMS_REQ_V01 0x0029
#define QMI_CSVT_SET_RLP_PARAMS_RESP_V01 0x0029
#define QMI_CSVT_GET_ACTIVE_CALL_LIST_REQ_V01 0x002A
#define QMI_CSVT_GET_ACTIVE_CALL_LIST_RESP_V01 0x002A
#define QMI_CSVT_GET_CALL_INFO_REQ_V01 0x002B
#define QMI_CSVT_GET_CALL_INFO_RESP_V01 0x002B
#define QMI_CSVT_GET_CALL_STATS_REQ_V01 0x002C
#define QMI_CSVT_GET_CALL_STATS_RESP_V01 0x002C
#define QMI_CSVT_SET_SUBSCRIPTION_BINDING_REQ_V01 0x002D
#define QMI_CSVT_SET_SUBSCRIPTION_BINDING_RESP_V01 0x002D
#define QMI_CSVT_GET_BIND_SUBSCRIPTION_REQ_V01 0x002E
#define QMI_CSVT_GET_BIND_SUBSCRIPTION_RESP_V01 0x002E
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro csvt_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type csvt_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define csvt_get_service_object_v01( ) \
          csvt_get_service_object_internal_v01( \
            CSVT_V01_IDL_MAJOR_VERS, CSVT_V01_IDL_MINOR_VERS, \
            CSVT_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

