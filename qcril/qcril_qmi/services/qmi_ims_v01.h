#ifndef IMS_QMI_SERVICE_H
#define IMS_QMI_SERVICE_H
/**
  @file qmi_ims_v01.h
  
  @brief This is the public header file which defines the ims_qmi service Data structures.

  This header file defines the types and structures that were defined in 
  ims_qmi. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.7
   It was generated on: Thu Sep  1 2011
   From IDL File: qmi_ims_v01.idl */

/** @defgroup ims_qmi_qmi_consts Constant values defined in the IDL */
/** @defgroup ims_qmi_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup ims_qmi_qmi_enums Enumerated types used in QMI messages */
/** @defgroup ims_qmi_qmi_messages Structures sent as QMI messages */
/** @defgroup ims_qmi_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup ims_qmi_qmi_accessor Accessor for QMI service object */
/** @defgroup ims_qmi_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ims_qmi_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define IMS_QMI_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMS_QMI_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMS_QMI_V01_IDL_TOOL_VERS 0x02
/** Maximum Defined Message ID */
#define IMS_QMI_V01_MAX_MESSAGE_ID 0x0034;
/** 
    @} 
  */


/** @addtogroup ims_qmi_qmi_consts 
    @{ 
  */
#define QMI_IMS_VT_NUMBER_MAX_V01 81
#define QMI_IMS_VT_CALLER_ID_MAX_V01 81
#define QMI_IMS_VT_CALLER_NAME_MAX_V01 182

/** *************************************************************************** */
#define RESPONSE_DETAILS_MAX_V01 512
#define PRESENCE_XML_MAX_V01 1024
#define PEER_URI_MAX_V01 255
#define SUBS_EVENT_MAX_V01 255
#define PAR_RULE_ID_MAX_V01 64
#define PAR_RULE_MAX_V01 64

/** *************************************************************************** */
#define VIDEO_DATA_MAX_V01 6144
#define RESPONSE_DETAILS_VIDEO_MAX_V01 512
#define VOL_HEADER_MAX_V01 255
#define NAL_HEADER_MAX_V01 255
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_TYPE_VT_V01 = 0x00, 
  CALL_TYPE_AUDIO_ONLY_V01 = 0x01, 
  CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_type_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Request Message; Originates a audio/video call (MO call). */
typedef struct {

  /* Mandatory */
  /*  Calling Number */
  char calling_number[QMI_IMS_VT_NUMBER_MAX_V01 + 1];
  /**<   Number to be dialed in ASCII string; 
       length range [1 to 81]
   */

  /* Optional */
  /*  Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  call_type_enum_v01 call_type;
  /**<   Call type \n
       - 0x00 -- CALL_TYPE_VT			 -- Audio/Video (automatic selection) \n
       - 0x01 -- CALL_TYPE_AUDIO_ONLY    -- Audio only VOIP call \n
   */
}vt_dial_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Response Message; Originates a audio/video call (MO call). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call ID */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;
  /**<   Unique call identifier for the dialed call
   */
}vt_dial_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Request Message; Ends a vt call. */
typedef struct {

  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;
  /**<   Unique call identifier for the call that must be ended
   */
}vt_end_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Response Message; Ends a vt call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call ID */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;
  /**<   Unique call identifier for the call that must be ended
   */
}vt_end_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  ANSWER_PARAM_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ACCEPT_V01 = 0x01, 
  REJECT_V01 = 0x02, 
  ANSWER_PARAM_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}answer_param_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Request Message; Answers an incoming vt call. */
typedef struct {

  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;
  /**<   Unique call identifier for the call that must be answered
   */

  /* Mandatory */
  answer_param_enum_v01 answer;
  /**<   The users response to the incoming call request.
	   0 - call rejected
	   1 - call accepted
  	 */
}vt_answer_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Response Message; Answers an incoming vt call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call ID */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;
  /**<   Unique call identifier for the call that must be answered
   */
}vt_answer_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Request Message; Queries the information associated with a call. */
typedef struct {

  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;
  /**<   Call identifier for the call to be queried for information
   */
}vt_get_call_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_STATE_ORIGINATING_V01 = 0x01, 
  CALL_STATE_INCOMING_V01 = 0x02, 
  CALL_STATE_CONVERSATION_V01 = 0x03, 
  CALL_STATE_CC_IN_PROGRESS_V01 = 0x04, 
  CALL_STATE_ALERTING_V01 = 0x05, 
  CALL_STATE_HOLD_V01 = 0x06, 
  CALL_STATE_WAITING_V01 = 0x07, 
  CALL_STATE_DISCONNECTING_V01 = 0x08, 
  CALL_STATE_END_V01 = 0x09, 
  CALL_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_state_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_DIRECTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_DIRECTION_MO_V01 = 0x01, 
  CALL_DIRECTION_MT_V01 = 0x02, 
  CALL_DIRECTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_direction_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t call_id;
  /**<   Call identifier for the call queried for information
   */

  call_state_enum_v01 call_state;
  /**<   Call state \n
       - 0x01 -- CALL_STATE_ORIGINATION    -- Origination \n
       - 0x02 -- CALL_STATE_INCOMING       -- Incoming \n
       - 0x03 -- CALL_STATE_CONVERSATION   -- Conversation \n
       - 0x04 -- CALL_STATE_CC_IN_PROGRESS -- Call is originating but waiting \n
                                              for call control to complete \n
       - 0x05 -- CALL_STATE_ALERTING       -- Alerting \n
       - 0x06 -- CALL_STATE_HOLD           -- Hold \n
       - 0x07 -- CALL_STATE_WAITING        -- Waiting \n
       - 0x08 -- CALL_STATE_DISCONNECTING  -- Disconnecting \n
       - 0x0A -- CALL_STATE_END          -- MT Call is in end state 
   */

  call_type_enum_v01 call_type;
  /**<   Call type \n
       - 0x00 -- CALL_TYPE_VT	         -- VT A/V call \n
       - 0x01 -- CALL_TYPE_AUDIO_ONLY    -- VT call with audio only \n
   */

  call_direction_enum_v01 direction;
  /**<   Direction \n
       - 0x01 -- CALL_DIRECTION_MO -- MO call \n
       - 0x02 -- CALL_DIRECTION_MT -- MT call
   */
}vt_call_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t number_len;  /**< Must be set to # of elements in number */
  char number[QMI_IMS_VT_NUMBER_MAX_V01];
  /**<   Number in ASCII string
   */
}vt_remote_party_number_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t caller_name_len;  /**< Must be set to # of elements in caller_name */
  char caller_name[QMI_IMS_VT_CALLER_NAME_MAX_V01];
  /**<   Caller name per the coding scheme
   */
}vt_remote_party_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t num_len;  /**< Must be set to # of elements in num */
  char num[QMI_IMS_VT_CALLER_ID_MAX_V01];
  /**<   Caller ID in ASCII string
   */
}vt_num_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Response Message; Queries the information associated with a call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call Information */
  uint8_t call_info_valid;  /**< Must be set to true if call_info is being passed */
  vt_call_info_type_v01 call_info;

  /* Optional */
  /*  Remote Party Number */
  uint8_t remote_party_number_valid;  /**< Must be set to true if remote_party_number is being passed */
  vt_remote_party_number_type_v01 remote_party_number;

  /* Optional */
  /*  Remote Party Name** */
  uint8_t remote_party_name_valid;  /**< Must be set to true if remote_party_name is being passed */
  vt_remote_party_name_type_v01 remote_party_name;

  /* Optional */
  /*  Connected Number Information */
  uint8_t conn_num_info_valid;  /**< Must be set to true if conn_num_info is being passed */
  vt_num_info_type_v01 conn_num_info;
}vt_get_call_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_END_REASON_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_END_CAUSE_OFFLINE_V01 = 0x00, 
  CALL_END_CAUSE_NO_SRV_V01 = 0x01, 
  CALL_END_CAUSE_REL_NORMAL_V01 = 0x02, 
  CALL_END_CAUSE_CLIENT_END_V01 = 0x03, 
  CALL_END_CAUSE_INCOM_REJ_V01 = 0x04, 
  CALL_END_CAUSE_NETWORK_END_V01 = 0x05, 
  CALL_END_CAUSE_USER_BUSY_V01 = 0x06, 
  CALL_END_CAUSE_USER_ALERTING_NO_ANSWER_V01 = 0x07, 
  CALL_END_CAUSE_CALL_REJECTED_V01 = 0x08, 
  CALL_END_CAUSE_NORMAL_UNSPECIFIED_V01 = 0x09, 
  CALL_END_CAUSE_TEMPORARY_FAILURE_V01 = 0x0A, 
  CALL_END_REASON_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_end_reason_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t call_id;
  /**<   Unique call identifier for the call
   */

  call_end_reason_enum_v01 call_end_reason;
  /**<   Call end reason; 
   0	QMI_FAILURE_CAUSE_OFFLINE		Phone is offline
   1	QMI_FAILURE_CAUSE_NO_SRV		Phone has no service
   2	QMI_FAILURE_CAUSE_REL_NORMAL	Received release from base station; no reason given
   3	QMI_FAILURE_CAUSE_CLIENT_END	Client ended the call
   4	QMI_FAILURE_CAUSE_INCOM_REJ	    Client rejected incoming call
   5	QMI_FAILURE_CAUSE_NETWORK_END	Network ended the call
   6	QMI_FAILURE_CAUSE_USER_BUSY		user busy
   7	QMI_FAILURE_CAUSE_USER_ALERTING_NO_ANSWER  no response
   8	QMI_FAILURE_CAUSE_CALL_REJECTED	call explicitly rejected by user
   9	QMI_FAILURE_CAUSE_NORMAL_UNSPECIFIED Call ended by user; no reason
   10	QMI_FAILURE_CAUSE_TEMPORARY_FAILURE	call ended
    */
}vt_call_end_reason_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Call Information */
  vt_call_info_type_v01 call_info;

  /* Optional */
  /*  Remote Party Number */
  uint8_t remote_party_number_valid;  /**< Must be set to true if remote_party_number is being passed */
  vt_remote_party_number_type_v01 remote_party_number;

  /* Optional */
  /*  Remote Party Name** */
  uint8_t remote_party_name_valid;  /**< Must be set to true if remote_party_name is being passed */
  vt_remote_party_name_type_v01 remote_party_name;

  /* Optional */
  /*  Connected Number Information */
  uint8_t conn_num_info_valid;  /**< Must be set to true if conn_num_info is being passed */
  vt_num_info_type_v01 conn_num_info;

  /* Optional */
  /*  Call end reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  vt_call_end_reason_type_v01 call_end_reason;
}vt_call_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  ENABLER_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ENABLER_STATE_UNINITIALIZED_V01 = 0x01, 
  ENABLER_STATE_INITIALIZED_V01 = 0x02, 
  ENABLER_STATE_REGISTERED_V01 = 0x03, 
  ENABLER_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}enabler_state_enum_v01;
/**
    @}
  */

/*
 * ims_enabler_state_req is empty
 * typedef struct {
 * }ims_enabler_state_req_v01;
 */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  enabler_state_enum_v01 enabler_state;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_enabler_state_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** EnablerStateIndication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  enabler_state_enum_v01 enabler_state;
}ims_enabler_state_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** SendPublishRequestforRichPresence Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Publish Request XML */
  char publish_xml[PRESENCE_XML_MAX_V01 + 1];
}ims_send_publish_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t sipMessageID_valid;  /**< Must be set to true if sipMessageID is being passed */
  uint32_t sipMessageID;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_send_publish_resp_v01;  /* Message */
/**
    @}
  */

/*
 * ims_send__unpublish_req is empty
 * typedef struct {
 * }ims_send__unpublish_req_v01;
 */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t sipMessageID_valid;  /**< Must be set to true if sipMessageID is being passed */
  uint32_t sipMessageID;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_send_unpublish_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** (CreateandSend)SubscribeRequest Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint8_t subscription_type;

  /* Mandatory */
  char peerURI[PEER_URI_MAX_V01 + 1];

  /* Mandatory */
  uint8_t isRLSSubscription;

  /* Mandatory */
  char subscriptionEvent[SUBS_EVENT_MAX_V01 + 1];

  /* Optional */
  uint8_t userList_valid;  /**< Must be set to true if userList is being passed */
  char userList[PRESENCE_XML_MAX_V01 + 1];
}ims_send_subscribe_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t sipMessageID_valid;  /**< Must be set to true if sipMessageID is being passed */
  uint32_t sipMessageID;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_send_subscribe_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** UnSubscribeRequest(CreateandSend) Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  char peerURI[PEER_URI_MAX_V01 + 1];

  /* Mandatory */
  char UnSubscriptionEvent[SUBS_EVENT_MAX_V01 + 1];
}ims_send_unsubscribe_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t sipMessageID;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_send_unsubscribe_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  SIP_MSG_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  PUBLISH_SUCCESS_V01 = 0x1, 
  PUBLISH_FAILURE_V01 = 0x2, 
  UNPUBLISH_SUCCESS_V01 = 0x3, 
  UNPUBLISH_FAILURE_V01 = 0x4, 
  SUBSCRIBE_SUCCESS_V01 = 0x5, 
  SUBSCRIBE_FAILURE_V01 = 0x6, 
  UNSUBSCRIBE_SUCCESS_V01 = 0x7, 
  UNSUBSCRIBE_FAILURE_V01 = 0x8, 
  NOTIFY_UPDATE_V01 = 0x9, 
  SIP_MSG_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sip_msg_status_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** IndicationforthePresenceSIPmessagestatusfromthenetwork. Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  sip_msg_status_enum_v01 status_response;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t notifyDetails_valid;  /**< Must be set to true if notifyDetails is being passed */
  char notifyDetails[PRESENCE_XML_MAX_V01 + 1];

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_sip_msg_status_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  HTTP_MSG_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  PAR_ADD_SUCCESS_V01 = 0x1, 
  PAR_ADD_FAILURE_V01 = 0x2, 
  HTTP_MSG_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}http_msg_status_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** IndicationforthePresenceSIPmessagestatusfromthenetwork. Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  http_msg_status_enum_v01 status_response;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t httpResponse_valid;  /**< Must be set to true if httpResponse is being passed */
  char httpResponse[PRESENCE_XML_MAX_V01 + 1];

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_http_msg_status_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** AddParRule Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  char parRuleID[PAR_RULE_ID_MAX_V01 + 1];

  /* Mandatory */
  char parRule[PAR_RULE_MAX_V01 + 1];
}ims_add_par_rule_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_add_par_rule_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** GetParRule Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;
}ims_get_par_rule_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  char parRuleID[PAR_RULE_ID_MAX_V01 + 1];

  /* Mandatory */
  char parRule[PAR_RULE_MAX_V01 + 1];

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_get_par_rule_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** GetParRuleAtIndex Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  uint32_t index;
}ims_get_par_rule_at_index_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  char parRuleID[PAR_RULE_ID_MAX_V01 + 1];

  /* Mandatory */
  char parRule[PAR_RULE_MAX_V01 + 1];

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_get_par_rule_at_index_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** DeleteParRuleAtIndex Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  uint32_t index;
}ims_del_par_rule_at_index_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_del_par_rule_at_index_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** DeleteParRule Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;
}ims_del_par_rule_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_del_par_rule_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** DeleteParDocumentRequest Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;
}ims_del_par_document_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_del_par_document_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  PAR_METHOD_NAME_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QP_ADD_PRES_AUTH_RULE_REQ_V01 = 0x00, 
  QP_DEL_PRES_AUTH_RULE_REQ_V01 = 0x01, 
  PAR_METHOD_NAME_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}par_method_name_enum_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** CreatePAR(PresenceAuthorizationRules)Request Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  par_method_name_enum_v01 method_name;

  /* Mandatory */
  char requestXML[PRESENCE_XML_MAX_V01 + 1];
}ims_send_par_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/**  Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Return the HTTP message ID */
  uint32_t httpMessageID;

  /* Mandatory */
  /*  Return the HTTP message string */
  char httpMessage[PRESENCE_XML_MAX_V01 + 1];

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_send_par_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  EVIDEO_MSG_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  VIDEO_MSG_ERROR_V01 = 0, 
  VIDEO_MSG_RECORDED_DATA_V01 = 1, 
  VIDEO_MSG_DATA_PLAYED_V01 = 2, 
  VIDEO_MSG_RESUME_PLAYING_V01 = 3, 
  VIDEO_MSG_RECORDER_STARTED_V01 = 4, 
  VIDEO_MSG_RECORDER_STOPPED_V01 = 5, 
  VIDEO_MSG_PLAYER_STARTED_V01 = 6, 
  VIDEO_MSG_PLAYER_STOPPED_V01 = 7, 
  VIDEO_MSG_DEV_INITIALISED_V01 = 8, 
  VIDEO_MSG_DEV_UNINITIALISED_V01 = 9, 
  VIDEO_MSG_SAVING_RES_ACQUIRED_V01 = 10, 
  VIDEO_MSG_SAVING_RES_RELEASED_V01 = 11, 
  VIDEO_MSG_SAVING_STARTED_V01 = 12, 
  VIDEO_MSG_SAVING_STOPPED_BY_USER_V01 = 13, 
  VIDEO_MSG_SAVING_STOPPED_STORAGE_FULL_V01 = 14, 
  VIDEO_MSG_SAVING_STOPPED_DURATION_REACHED_V01 = 15, 
  VIDEO_MSG_SAVING_ERROR_V01 = 16, 
  VIDEO_MSG_FAR_FRAME_V01 = 17, 
  VIDEO_MSG_NEAR_FRAME_V01 = 18, 
  VIDEO_MSG_CODEC_CHANGED_V01 = 19, 
  EVIDEO_MSG_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}eVIDEO_MSG_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t iTimeStamp;

  uint32_t iSeqNum;

  uint8_t bMarkerBit;

  uint8_t bSilence;

  uint8_t iPayloadType;
}MEDIA_PACKET_INFO_RX_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  MEDIA_PACKET_INFO_RX_v01 sMediaPktInfoRx;
}MEDIA_PACKET_INFO_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  /* Encoded data or the codec frame.  */
  uint32_t pData_len;  /**< Must be set to # of elements in pData */
  uint8_t pData[VIDEO_DATA_MAX_V01];

  /*  Length of the encoded data or the codec frame.  */
  uint16_t iDataLen;

  /*  Length of the allocated data buffer.  */
  uint16_t iMaxBuffLen;

  /*  MEDIA_PACKET_INFO structure.                      */
  MEDIA_PACKET_INFO_v01 sMediaPacketInfo;
}QP_MULTIMEDIA_FRAME_PLAY_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  /* Encoded data or the codec frame.  */
  uint32_t pData_len;  /**< Must be set to # of elements in pData */
  uint8_t pData[VIDEO_DATA_MAX_V01];

  /*  Length of the encoded data or the codec frame.  */
  uint16_t iDataLen;

  /*  Length of the allocated data buffer.  */
  uint16_t iMaxBuffLen;
}QP_MULTIMEDIA_FRAME_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  EVIDEO_ERROR_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  VIDEO_ERROR_OK_V01 = 0, 
  VIDEO_ERROR_UNKNOWN_V01 = -1, 
  VIDEO_ERROR_RECORDER_DOWN_V01 = 1, 
  VIDEO_ERROR_PLAYER_DOWN_V01 = 2, 
  VIDEO_ERROR_BUFFER_OVERFLOW_V01 = 3, 
  VIDEO_ERROR_BUFFER_UNDERFLOW_V01 = 4, 
  VIDEO_ERROR_RECORDER_BUSY_V01 = 5, 
  VIDEO_ERROR_PLAYER_BUSY_V01 = 6, 
  EVIDEO_ERROR_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}eVIDEO_ERROR_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  VIDEO_CODEC_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  VIDEO_CODEC_MPEG4_XVID_V01 = 0x00, 
  VIDEO_CODEC_MPEG4_ISO_V01 = 0x01, 
  VIDEO_CODEC_H263_V01 = 0x02, 
  VIDEO_CODEC_H264_V01 = 0x03, 
  VIDEO_CODEC_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}video_codec_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_aggregates
    @{
  */
typedef struct {

  /*  Codec that needs to be used. */
  video_codec_v01 Codec;

  /*  Width of the video capture  */
  int32_t Width;

  /*  Height of the video capture  */
  int32_t Height;

  /*  Output BitRate expected from the encoder in kbps  */
  int32_t BitRate;

  /*  Frame Rate of the capture in frames/sec  */
  int32_t FrameRate;

  /*  Buffer containing VOL header  */
  char VolHeader[VOL_HEADER_MAX_V01 + 1];

  /*  Length of the VOL header  */
  uint16_t VolHeaderLen;

  /*  Buffer containing NAL header  */
  char NALHeader[NAL_HEADER_MAX_V01 + 1];

  /*  Length of the NAL header  */
  uint16_t NALHeaderLen;

  /*  Sampling rate  */
  uint32_t ClockRate;

  /*  Enable/ Disable Lip synchronization */
  uint8_t LipSyncEnable;
}QpVideoConfig_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  VIDEO_DEV_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  VIDEO_PLAYER_V01 = 0x0, 
  VIDEO_RECORDER_V01 = 0x01, 
  VIDEO_DEV_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}video_dev_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  QpVideoConfig_v01 videoConfig;

  /* Mandatory */
  video_dev_v01 video_device_type;
}ims_video_initialise_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  video_dev_v01 video_device_type;
}ims_video_uninitialise_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;
}ims_video_play_start_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Optional */
  uint8_t bPurge_valid;  /**< Must be set to true if bPurge is being passed */
  uint8_t bPurge;
}ims_video_play_stop_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  uint32_t iFrameBundlingVal;
}ims_video_record_start_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;
}ims_video_record_stop_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  EVIDEO_PREV_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  VIDEO_PREV_SMALL_V01 = 0, 
  VIDEO_PREV_LARGE_V01 = 1, 
  EVIDEO_PREV_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}eVIDEO_PREV_v01;
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  eVIDEO_PREV_v01 eVPrevSize;

  /* Mandatory */
  int32_t iXPos;

  /* Mandatory */
  int32_t iYPos;
}ims_video_start_prev_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  QP_MULTIMEDIA_FRAME_PLAY_v01 pCodecFrame;
}ims_video_play_frame_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  QpVideoConfig_v01 videoCodecConfig;
}ims_video_codec_set_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  uint32_t n_iMSNtpTime;

  /* Mandatory */
  uint32_t n_iLSNtpTime;

  /* Mandatory */
  uint32_t n_iRtpTimeStamp;
}ims_video_report_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  uint32_t n_iMSNtpTime;

  /* Mandatory */
  uint32_t n_iLSNtpTime;

  /* Mandatory */
  uint32_t n_iRtpTimeStamp;
}ims_audio_report_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  uint32_t lastPlayTimeStamp;
}ims_last_audio_play_time_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  uint32_t videoDescriptorID;

  /* Mandatory */
  uint32_t currentPlayTimeStamp;
}ims_current_audio_play_time_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Request Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  eVIDEO_MSG_v01 tVideoMsg;

  /* Mandatory */
  QP_MULTIMEDIA_FRAME_v01 pParam1;

  /* Mandatory */
  uint32_t iParam2;
}ims_video_status_report_request_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_messages
    @{
  */
/** Response Message; Indicates a change in the call information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_VIDEO_MAX_V01 + 1];
}ims_video_status_rsp_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup ims_qmi_qmi_msg_ids
    @{
  */
#define QMI_IMS_VIDEO_INITIALISE_INDICATION_V01 0x0000
#define QMI_IMS_VIDEO_UNINITIALISE_INDICATION_V01 0x0001
#define QMI_IMS_VIDEO_STATUS_RESP_V01 0x0002
#define QMI_IMS_VIDEO_STATUS_REQUEST_V01 0x0003
#define QMI_IMS_VIDEO_CODEC_SET_INDICATION_V01 0x0004
#define QMI_IMS_VIDEO_PLAY_FRAME_INDICATION_V01 0x0005
#define QMI_IMS_VIDEO_START_PREVIEW_INDICATION_V01 0x0006
#define QMI_IMS_VIDEO_RECORD_STOP_INDICATION_V01 0x0007
#define QMI_IMS_VIDEO_RECORD_START_INDICATION_V01 0x0008
#define QMI_IMS_VIDEO_PLAY_STOP_INDICATION_V01 0x0009
#define QMI_IMS_VIDEO_PLAY_START_INDICATION_V01 0x0010
#define QMI_IMS_CURRENT_AUDIO_PLAY_TIME_INDICATION_V01 0x0011
#define QMI_IMS_LAST_AUDIO_PLAY_TIME_INDICATION_V01 0x0012
#define QMI_IMS_AUDIO_REPORT_INDICATION_V01 0x0013
#define QMI_IMS_VIDEO_REPORT_INDICATION_V01 0x0014
#define QMI_IMS_VT_STATUS_INDICATION_V01 0x0021
#define QMI_IMS_INCOMING_SIP_STATUS_IND_V01 0x0032
#define QMI_IMS_INCOMING_HTTP_STATUS_IND_V01 0x0033
#define QMI_IMS_ENABLER_STATE_IND_V01 0x0034
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro ims_qmi_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type ims_qmi_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define ims_qmi_get_service_object_v01( ) \
          ims_qmi_get_service_object_internal_v01( \
            IMS_QMI_V01_IDL_MAJOR_VERS, IMS_QMI_V01_IDL_MINOR_VERS, \
            IMS_QMI_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

