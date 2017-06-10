#ifndef IMSP_SERVICE_H
#define IMSP_SERVICE_H
/**
  @file ip_multimedia_subsystem_presence_v01.h
  
  @brief This is the public header file which defines the imsp service Data structures.

  This header file defines the types and structures that were defined in 
  imsp. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/imsp/main/latest/api/ip_multimedia_subsystem_presence_v01.h#3 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.0
   It requires encode/decode library version 4 or later
   It was generated on: Fri Mar  2 2012
   From IDL File: ip_multimedia_subsystem_presence_v01.idl */

/** @defgroup imsp_qmi_consts Constant values defined in the IDL */
/** @defgroup imsp_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsp_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsp_qmi_messages Structures sent as QMI messages */
/** @defgroup imsp_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsp_qmi_accessor Accessor for QMI service object */
/** @defgroup imsp_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsp_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define IMSP_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSP_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSP_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define IMSP_V01_MAX_MESSAGE_ID 0x002E;
/** 
    @} 
  */


/** @addtogroup imsp_qmi_consts 
    @{ 
  */
#define IMSP_MAX_PRESENCE_XML_STR_LEN_V01 4095
#define IMSP_MAX_URI_STR_LEN_V01 255
#define IMSP_MAX_SERVICE_DESC_STR_LEN_V01 31
#define IMSP_MAX_VERSION_STR_LEN_V01 7
#define IMSP_MAX_SERVICE_ID_STR_LEN_V01 63
#define IMSP_MAX_TIMESTAMP_LEN_V01 31
#define IMSP_MAX_RESOURCE_INSTANCE_ID_STR_LEN_V01 15
#define IMSP_MAX_RESOURCE_INSTANCE_REASON_STR_LEN_V01 31
#define IMSP_MAX_RESOURCE_INSTANCE_CID_STR_LEN_V01 287
#define IMSP_MAX_RESOURCE_INSTANCE_STATE_STR_LEN_V01 31
#define IMSP_MAX_LIST_NAME_STR_LEN_V01 63
#define IMSP_MAX_SUBSCRIBE_USER_COUNT_V01 16
#define IMSP_MAX_RECORD_IN_NOTIFY_V01 3
/**
    @}
  */

/** @addtogroup imsp_qmi_enums
    @{
  */
typedef enum {
  IMSP_PUBLISH_STATUS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSP_PRESENCE_STATUS_CLOSED_V01 = 0x00, 
  IMSP_PRESENCE_STATUS_OPEN_V01 = 0x01, 
  IMSP_PUBLISH_STATUS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsp_publish_status_v01;
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  char description[IMSP_MAX_SERVICE_DESC_STR_LEN_V01 + 1];
  /**<   Service description.   */

  char ver[IMSP_MAX_VERSION_STR_LEN_V01 + 1];
  /**<   Service version.  */

  char service_id[IMSP_MAX_SERVICE_ID_STR_LEN_V01 + 1];
  /**<   Service ID.  */
}imsp_presence_service_description_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_enums
    @{
  */
typedef enum {
  IMSP_MEDIA_CAPABILITY_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSP_CAPABILITY_FULL_DUPLEX_V01 = 0x0, 
  IMSP_CAPABILITY_HALF_RECEIVE_ONLY_V01 = 0x1, 
  IMSP_CAPABILITY_HALF_SEND_ONLY_V01 = 0x2, 
  IMSP_MEDIA_CAPABILITY_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsp_media_capability_v01;
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_audio_supported;
  /**<   Whether or not HD audio is supported.   */

  imsp_media_capability_v01 audio_capability;
  /**<   Audio capability. Values: \n 
      - 0 -- Full duplex audio is supported \n 
      - 1 -- Receive-only audio is supported \n 
      - 2 -- Send-only audio is supported
    */

  uint8_t is_video_supported;
  /**<   Whether or not video is supported.  */

  imsp_media_capability_v01 video_capability;
  /**<   Video capability. Values: \n 
      - 0 -- Full duplex video is supported \n 
      - 1 -- Receive-only video is supported \n 
      - 2 -- Send-only video is supported
    */
}imsp_presence_service_capabilities_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  char contact_uri[IMSP_MAX_URI_STR_LEN_V01 + 1];
  /**<   Contact's URI.  */

  imsp_presence_service_description_v01 service_descriptions;
  /**<   Service descriptions.  */

  imsp_presence_service_capabilities_v01 service_capabilities;
  /**<   Service capabilities.  */
}imsp_presence_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_enums
    @{
  */
typedef enum {
  IMSP_ENABLER_STATE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSP_ENABLER_STATE_UNINITIALIZED_V01 = 0x01, 
  IMSP_ENABLER_STATE_INITIALIZED_V01 = 0x02, 
  IMSP_ENABLER_STATE_AIRPLANE_V01 = 0x03, 
  IMSP_ENABLER_STATE_REGISTERED_V01 = 0x04, 
  IMSP_ENABLER_STATE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsp_enabler_state_v01;
/**
    @}
  */

/*
 * imsp_get_enabler_state_req is empty
 * typedef struct {
 * }imsp_get_enabler_state_req_v01;
 */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; This is a request from the client to the service side to get the 
             presence enabler's state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
        qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
        qmi_error_type  - Error code. Possible error code values are 
                          described in the error codes section of 
                          each message definition.
   */

  /* Optional */
  /*  Enabler State */
  uint8_t enabler_state_valid;  /**< Must be set to true if enabler_state is being passed */
  imsp_enabler_state_v01 enabler_state;
  /**<   Values: \n
       - 1 -- Enabler is uninitialized and will not process requests. \n
       - 2 -- Enabler is initialized but the IP multimedia subsystem is not yet 
              registered with the network. In this state, the presence will return 
              an error for any requests. \n
       - 3 -- Enabler is initialized, but the device is in Airplane mode. The 
              enabler cannot process nor send requests out to the network. 

             Note: Setting the Airplane mode is not in the scope of QMI_IMSP. 
                   Clients can set Airplane mode via QMI_NAS. \n
       - 4 -- Enabler is initialized and registered with the network. The enabler 
              will process and send the requests out to the network.

   */
}imsp_get_enabler_state_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; This is a service-initiated indication of an enabler state when 
             there is a change in the enabler state. */
typedef struct {

  /* Mandatory */
  /*  Enabler State */
  imsp_enabler_state_v01 enabler_state;
  /**<   Values: \n
       - 1 -- Enabler is uninitialized and will not process requests. \n
       - 2 -- Enabler is initialized but the IP multimedia subsystem is not yet 
              registered with the network. In this state, the presence will return 
              an error for any requests. \n
       - 3 -- Enabler is initialized, but the device is in Airplane mode. The 
              enabler cannot process nor send requests out to the network. 

             Note: Setting the Airplane mode is not in the scope of QMI_IMSP. 
                   Clients can set Airplane mode via QMI_NAS. \n
       - 4 -- Enabler is initialized and registered with the network. The enabler 
              will process and send the requests out to the network.
   */
}imsp_enabler_state_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_enums
    @{
  */
typedef enum {
  IMSP_PUBLISH_TRIGGER_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSP_ETAG_EXPIRED_V01 = 0x01, 
  IMSP_RAT_CHANGE_LTE_V01 = 0x02, 
  IMSP_RAT_CHANGE_EHRPD_V01 = 0x03, 
  IMSP_AIRPLANE_MODE_V01 = 0x04, 
  IMSP_PUBLISH_TRIGGER_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsp_publish_trigger_v01;
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Notification of an event that may require the AP to publish 
             presence information.  */
typedef struct {

  /* Mandatory */
  /*  Publish Trigger */
  imsp_publish_trigger_v01 publish_trigger;
  /**<   Values: \n
    - 1 -- Presence enabler will inform the client about an expired ETAG, which
           can be used to trigger the republish. \n
    - 2 -- Presence enabler will inform the client about the LTE RAT change 
           notification to the registered client for any republish. \n
    - 3 -- Presence enabler will inform the client about the EHRPD RAT change 
           notification to the registered client for any republish. \n
    - 4 -- Presence enabler will inform the client about the Airplane mode 
           notification to the registered client to trigger any republish. 
           However, republish is not assured even if the client triggers the 
           republish because service might have already gone to Airplane mode
    */
}imsp_publish_trigger_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; Sends the Publish request with basic status or basic status with 
              optional presence information of service capabilities. */
typedef struct {

  /* Mandatory */
  /*  Publish Status */
  imsp_publish_status_v01 publish_status;
  /**<   Values: \n
      - IMSP_PUBLISH_STATUS_OPEN -- Client wants to perform a publish with the 
        basic status as open \n
      - IMSP_PUBLISH_STATUS_CLOSED -- Client wants to perform a publish with 
        the basic status as closed
    */

  /* Optional */
  /*  Publish Request */
  uint8_t presence_info_valid;  /**< Must be set to true if presence_info is being passed */
  imsp_presence_info_v01 presence_info;
}imsp_send_publish_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; Sends the Publish request with basic status or basic status with 
              optional presence information of service capabilities. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_publish_callid_valid;  /**< Must be set to true if imsp_publish_callid is being passed */
  uint32_t imsp_publish_callid;
  /**<   Unique ID for this Publish request that can be used to associate with 
      the Publish indication that will be received for this request. Also, this 
      ID can be used to unpublish.
   */
}imsp_send_publish_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Indication for the presence Publish message status from the network. */
typedef struct {

  /* Mandatory */
  /*  Status Response */
  uint8_t status_response;
  /**<   Values: \n
       - 0x00 -- Publish failure \n
       - 0x01 -- Publish success
   */

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_publish_callid;
  /**<   This is the same unique ID that was received when the Publish request 
       was initiated. Also, this ID can be used to unpublish.
   */

  /* Optional */
  /*  SIP Response Code */
  uint8_t imsp_sip_resp_code_valid;  /**< Must be set to true if imsp_sip_resp_code is being passed */
  uint32_t imsp_sip_resp_code;
  /**<   SIP response code, e.g., 200, 480, 486, etc. This is optional for a 
    Notify update as this is a request.
   */
}imsp_send_publish_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; Sends the Publish request with XML that is in compliance per 
              the PIDF RFC (RFC 3863)  */
typedef struct {

  /* Mandatory */
  /*  Publish Request */
  char imsp_rich_publish_xml[IMSP_MAX_PRESENCE_XML_STR_LEN_V01 + 1];
  /**<   Publish XML in the format specified as part of RFC 3863, RFC 4480, 
       and RFC 5196.
   */
}imsp_send_publish_xml_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; Sends the Publish request with XML that is in compliance per 
              the PIDF RFC (RFC 3863)  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_publish_callid_valid;  /**< Must be set to true if imsp_publish_callid is being passed */
  uint32_t imsp_publish_callid;
  /**<   Unique ID for this Publish XML request that can be used to associate with 
      the Publish XML indication that will be received for this request. Also, 
      this ID can be used to unpublish.
   */
}imsp_send_publish_xml_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Indication for the presence Publish XML message status sent by 
              the service on receiving the response from the presence server, 
              or for when the request timed out. */
typedef struct {

  /* Mandatory */
  /*  Status Response */
  uint8_t status_response;
  /**<   Values: \n
       - 0x00 -- Publish XML failure \n
       - 0x01 -- Publish XML success
   */

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_publish_callid;
  /**<   This is the same unique ID that was received when the Publish XML request 
       was initiated. Also, this ID can be used to unpublish.
   */

  /* Optional */
  /*  SIP Response Code */
  uint8_t imsp_sip_resp_code_valid;  /**< Must be set to true if imsp_sip_resp_code is being passed */
  uint32_t imsp_sip_resp_code;
  /**<   SIP response code, e.g., 200, 480, 486, etc. This is optional for a 
    Notify update as this is a request.
   */
}imsp_send_publish_xml_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; Sends the Unpublish request for the specified IMSP publish call ID. */
typedef struct {

  /* Optional */
  /*  Call ID */
  uint8_t imsp_publish_callid_valid;  /**< Must be set to true if imsp_publish_callid is being passed */
  uint32_t imsp_publish_callid;
  /**<   Unique ID for this Unpublish request that can be used to associate with 
      the Unpublish indication that will be received for this request. 
   */
}imsp_send_unpublish_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; Sends the Unpublish request for the specified IMSP publish call ID. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_unpublish_callid_valid;  /**< Must be set to true if imsp_unpublish_callid is being passed */
  uint32_t imsp_unpublish_callid;
  /**<   Unique ID for this Unpublish request that can be used to associate with 
      the Unpublish indication that will be received for this request. 
   */
}imsp_send_unpublish_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Indication for the presence Unpublish message status from the 
              presence server, or for when the request timed out. */
typedef struct {

  /* Mandatory */
  /*  Status Response */
  uint8_t status_response;
  /**<   Values: \n
       - 0x00 -- Unpublish failure \n
       - 0x01 -- Unpublish success
   */

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_unpublish_callid;
  /**<   This is the same unique ID that was received when the Unpublish request 
       was initiated. Also, this ID can only be used to match the Unpublish 
       request and indication.
   */

  /* Optional */
  /*  SIP Response Code */
  uint8_t imsp_sip_resp_code_valid;  /**< Must be set to true if imsp_sip_resp_code is being passed */
  uint32_t imsp_sip_resp_code;
  /**<   SIP response code, e.g., 200, 480, 486, etc. 
   */
}imsp_send_unpublish_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  char imsp_user_uri[IMSP_MAX_URI_STR_LEN_V01 + 1];
  /**<   User's SIP URIs that are being subscribed. 
   */
}imsp_user_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_enums
    @{
  */
typedef enum {
  IMSP_SUBSCRIPTION_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSP_SUBSCRIBE_SIMPLE_V01 = 0x01, 
  IMSP_SUBSCRIBE_POLLING_V01 = 0x02, 
  IMSP_SUBSCRIPTION_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsp_subscription_type_v01;
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; Creates and sends a Subscribe request. */
typedef struct {

  /* Mandatory */
  /*  Susbcription Type */
  imsp_subscription_type_v01 subscription_type;
  /**<   Values: \n
    - IMSP_SUBSCRIBE_SIMPLE --
      Simple subscription is used to indicate the subscription
      with an expiration > 0. Per the current requirements, simple
      is used for a single user. \n
    - IMSP_SUBSCRIBE_POLLING --
      Polling subscription is used to indicate the subscription 
      with an expiration = 0, which generates one notify. Per the current 
      requirements, polling is used for single contacts.
  */

  /* Mandatory */
  /*  User List */
  uint32_t subscribe_user_list_len;  /**< Must be set to # of elements in subscribe_user_list */
  imsp_user_info_v01 subscribe_user_list[IMSP_MAX_SUBSCRIBE_USER_COUNT_V01];
}imsp_send_subscribe_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; Creates and sends a Subscribe request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_subscribe_callid_valid;  /**< Must be set to true if imsp_subscribe_callid is being passed */
  uint32_t imsp_subscribe_callid;
  /**<   Unique ID for this Subscribe request that can be used to associate with 
      the Subscribe indication that will be received for this request. Also, this 
      ID can be used to unsubscribe.
   */
}imsp_send_subscribe_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Indication for the presence Subscribe message status from 
              the presence server, or for when the request timed out. */
typedef struct {

  /* Mandatory */
  /*  Status Response */
  uint8_t status_response;
  /**<   Values: \n
       - 0x00 -- Subscribe failure \n
       - 0x01 -- Subscribe success
   */

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_subscribe_callid;
  /**<   This is the same unique ID that was received when the Subscribe request 
       was initiated. Also, this ID can be used to unsubscribe.
   */

  /* Optional */
  /*  SIP Response Code */
  uint8_t imsp_sip_resp_code_valid;  /**< Must be set to true if imsp_sip_resp_code is being passed */
  uint32_t imsp_sip_resp_code;
  /**<   SIP response code, e.g., 200, 480, 486, etc.
   */
}imsp_send_subscribe_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; Creates and sends a Subscribe request based on the XML. */
typedef struct {

  /* Mandatory */
  /*  User List XML */
  char imsp_user_list_xml[IMSP_MAX_PRESENCE_XML_STR_LEN_V01 + 1];
  /**<   RFC compliant XML body that must be sent in the subscribe body.
   */
}imsp_send_subscribe_xml_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; Creates and sends a Subscribe request based on the XML. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_subscribe_callid_valid;  /**< Must be set to true if imsp_subscribe_callid is being passed */
  uint32_t imsp_subscribe_callid;
  /**<   Unique ID for this Subscribe XML request that can be used to associate with 
      the Subscribe XML indication that will be received for this request. Also, 
      this ID can be used to unsubscribe.
   */
}imsp_send_subscribe_xml_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Indication for the presence Subscribe XML message status from the 
              presence server, or for when the request timed out. */
typedef struct {

  /* Mandatory */
  /*  Status Response */
  uint8_t status_response;
  /**<   Values: \n
       - 0x00 -- Subscribe XML failure \n
       - 0x01 -- Subscribe XML success
   */

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_subscribe_callid;
  /**<   This is the same unique ID that was received when the Subscribe XML request 
       was initiated. Also, this ID can be used to unsubscribe.
   */

  /* Optional */
  /*  SIP Response Code */
  uint8_t imsp_sip_resp_code_valid;  /**< Must be set to true if imsp_sip_resp_code is being passed */
  uint32_t imsp_sip_resp_code;
  /**<   SIP response code, e.g., 200, 480, 486, etc.
   */
}imsp_send_subscribe_xml_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; Creates and sends an Unsubscribe request. */
typedef struct {

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_subscribe_callid;
  /**<   Unique ID that was received when the Subscribe or Subscribe XML request 
       was initiated. 
   */
}imsp_send_unsubscribe_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; Creates and sends an Unsubscribe request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_unsubscribe_callid_valid;  /**< Must be set to true if imsp_unsubscribe_callid is being passed */
  uint32_t imsp_unsubscribe_callid;
  /**<   Unique ID for this Unsubscribe request that can be used to associate with 
      the Unsubscribe indication that will be received for this request. 
   */
}imsp_send_unsubscribe_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Indication for the presence Unsubscribe message status sent by 
              the service on receiving the response from the presence server, 
              or for when the request timed out. */
typedef struct {

  /* Mandatory */
  /*  Status Response */
  uint8_t status_response;
  /**<   Values: \n
       - 0x00 -- Unsubscribe failure \n
       - 0x01 -- Unsubscribe success
   */

  /* Mandatory */
  /*  Call ID */
  uint32_t imsp_unsubscribe_callid;
  /**<   This is the same unique ID that was received when the Unsubscribe request 
       was initiated. Also, this ID can only be used to match the Unsubscribe 
       request and indication.
   */

  /* Optional */
  /*  SIP Response Code */
  uint8_t imsp_sip_resp_code_valid;  /**< Must be set to true if imsp_sip_resp_code is being passed */
  uint32_t imsp_sip_resp_code;
  /**<   SIP response code, e.g., 200, 480, 486, etc.
   */
}imsp_send_unsubscribe_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Notification of the Subscribe XML message. */
typedef struct {

  /* Mandatory */
  /*  Notify Details */
  char imsp_notify_details[IMSP_MAX_PRESENCE_XML_STR_LEN_V01 + 1];
  /**<   Presence status from the Subscription request; multipart/multi-MIME 
       received from the network.
   */

  /* Optional */
  /*  Call ID */
  uint8_t imsp_subscribe_callid_valid;  /**< Must be set to true if imsp_subscribe_callid is being passed */
  uint32_t imsp_subscribe_callid;
  /**<   This is same unique ID that was received when Subscribe or 
       Subscribe XML was requested. This ID can be used to match the subscribe 
       if the client wants to do so.
       
       Note: The service might also receive the notify from the presence server 
       at any time. In this case, the Call ID will not be present.
   */
}imsp_notify_xml_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  imsp_presence_info_v01 presence_info;
  /**<   User presence information such as capabilities, service information, etc.  */

  char timestamp_of_presence_info[IMSP_MAX_TIMESTAMP_LEN_V01 + 1];
  /**<   User presence information was updated at this specific timestamp.  */
}imsp_presence_info_with_ts_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  char list_contact_uri[IMSP_MAX_URI_STR_LEN_V01 + 1];
  /**<   List URI received as part of the notify.  */

  char list_name[IMSP_MAX_LIST_NAME_STR_LEN_V01 + 1];
  /**<   List name received as part of the notify.  */

  uint32_t list_version;
  /**<   List version that is the indication to Enhanced Address Book (EAB) to 
       drop the list if the latest version is already with the EAB.  */

  uint8_t full_state;
  /**<   Indicates whether or not the list is complete.   */
}imsp_presence_list_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  char resource_id[IMSP_MAX_RESOURCE_INSTANCE_ID_STR_LEN_V01 + 1];
  /**<   Resource ID in the list notify.  */

  char resource_state[IMSP_MAX_RESOURCE_INSTANCE_STATE_STR_LEN_V01 + 1];
  /**<   Resource state in the list notify.  */

  char resource_reason[IMSP_MAX_RESOURCE_INSTANCE_REASON_STR_LEN_V01 + 1];
  /**<   Resource that is part of the list because of the specified reason.  */

  char resource_cid[IMSP_MAX_RESOURCE_INSTANCE_CID_STR_LEN_V01 + 1];
  /**<   Resource CID that is the reference to the presence information.   */
}imsp_presence_resource_instance_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  char resource_uri[IMSP_MAX_URI_STR_LEN_V01 + 1];
  /**<   Resource's URI; all other information is for this specific URI.  */

  uint8_t is_volte_contact;
  /**<   Whether or not the contact is a VoLTE/RCS subscriber. 
       FALSE indicates that the contact is not a VoLTE contact because the 
       device received 404 for this contact in the notify (404 means the 
       resource is not VoLTE provisioned or not a carrier's number). 
   */

  imsp_publish_status_v01 publish_status;
  /**<   Publish status. Values: \n 
     - IMSP_PUBLISH_STATUS_OPEN -- UE is open for communication with status 
       as open \n
     - IMSP_PUBLISH_STATUS_CLOSED -- UE is closed for communication with status 
       as closed
   */

  imsp_presence_resource_instance_v01 resource_instance;
  /**<   Resource URI resource instance information.  */

  imsp_presence_info_with_ts_v01 presence_user_info;
  /**<   Resource URI's presence information.  */
}imsp_presence_user_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_aggregates
    @{
  */
typedef struct {

  imsp_presence_list_info_v01 list_info;
  /**<   Notify's list information such as list URI, list name, etc.  */

  uint32_t user_list_info_len;  /**< Must be set to # of elements in user_list_info */
  imsp_presence_user_info_v01 user_list_info[IMSP_MAX_RECORD_IN_NOTIFY_V01];
  /**<   Notify's presence information and resource instance information.  */
}imsp_presence_notify_rich_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Indication Message; Notification of the Subscribe message. */
typedef struct {

  /* Mandatory */
  /*  Notify Details */
  imsp_presence_notify_rich_info_v01 rich_info;

  /* Optional */
  /*  Call ID */
  uint8_t imsp_subscribe_callid_valid;  /**< Must be set to true if imsp_subscribe_callid is being passed */
  uint32_t imsp_subscribe_callid;
  /**<   This is same unique ID that was received when Subscribe or 
       Subscribe XML was requested. This ID can be used to match the subscribe 
       if the client wants to do so.
       
       Note: The service might also receive the notify from the presence server 
       at any time. In this case, the Call ID will not be present.
   */
}imsp_notify_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; This sets how the service notifies the client, whether to 
             parse and provide the structure or XML as it was received from 
             the presence server. */
typedef struct {

  /* Mandatory */
  /*  XML Processing */
  uint8_t update_with_struct_info;
  /**<   Notify information in the form of an structure. Values: \n
      - TRUE -- Indicates to the service that the service must update the 
             client with a defined structure by processing the incoming 
             RFC compliant Notify's XML, i.e., the service sends  
             QMI_IMSP_NOTIFY_XLM_IND. \n

      - FALSE -- Indicates to the service that the service must update the 
             client with received RFC compliant XML as is, i.e., the service 
             sends QMI_IMSP_NOTIFY_XLM_IND. \n

      By default, any time the notify is received, QMI_IMSP_NOTIFY_XLM_IND is 
      sent if the event report is enabled for the notify indication via 
      IMSP_SET_EVENT_REPORT_REQ.
      
   */
}imsp_set_notify_fmt_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; This sets how the service notifies the client, whether to 
             parse and provide the structure or XML as it was received from 
             the presence server. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */
}imsp_set_notify_fmt_resp_v01;  /* Message */
/**
    @}
  */

/*
 * imsp_get_notify_fmt_req is empty
 * typedef struct {
 * }imsp_get_notify_fmt_req_v01;
 */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; This gets information on the current settings of the service 
             on how the service notifies the client, whether to parse 
             and provide the structure or XML as it was received from the 
             presence server. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  XML Processing */
  uint8_t update_with_struct_info_valid;  /**< Must be set to true if update_with_struct_info is being passed */
  uint8_t update_with_struct_info;
  /**<   Notify information in the form of an structure. Values: \n
     - TRUE -- Indicates that the service's current setting is to process the 
            incoming notify and convert them to structures to send the 
            indication to the client. Thus, on getting the incoming notify, 
            the service invokes the QMI_IMSP_NOTIFY_XLM_IND if the 
            notify indication is turned on. \n

      - FALSE -- Indicates that the service updates the client with received 
            RFC compliant XML as is, i.e., the service sends 
            QMI_IMSP_NOTIFY_XLM_IND if the notify indication is turned on. \n

      By default, any time the notify is received, QMI_IMSP_NOTIFY_XLM_IND is 
      sent if the event report is enabled for the notify indication via 
      IMSP_SET_EVENT_REPORT_REQ.
   */
}imsp_get_notify_fmt_resp_v01;  /* Message */
/**
    @}
  */

typedef uint64_t imsp_event_report_bit_masks_v01;
#define IMSP_ENABLE_PUBLISH_TRIGGER_IND_V01 ((imsp_event_report_bit_masks_v01)0x01) 
#define IMSP_ENABLE_ENABLER_STATE_IND_V01 ((imsp_event_report_bit_masks_v01)0x02) 
#define IMSP_ENABLE_NOTIFY_IND_V01 ((imsp_event_report_bit_masks_v01)0x04) 
/** @addtogroup imsp_qmi_messages
    @{
  */
/** Request Message; This enables or disables various indications from the service. */
typedef struct {

  /* Mandatory */
  /*  Event Report Bitmask */
  imsp_event_report_bit_masks_v01 event_report_bit_masks;
  /**<   Bitmask setting for which indications must be sent to the client. 
       Bitmask of events registered by all control points. Values: \n
       - 1 -- IMSP_ENABLE_PUBLISH_TRIGGER_ IND -- 
              Send QMI_IMSP_PUBLISH_ TRIGGER_IND \n
       - 2 -- IMSP_ENABLE_ENABLER_STATE_ IND -- 
              Send QMI_IMSP_ENABLER_STATE_ IND \n
       - 4 -- IMSP_ENABLE_NOTIFY_IND -- 
              Send QMI_IMSP_NOTIFY_XML_IND or QMI_IMSP_NOTIFY_IND \n \vspace{-.12in}

       This sets whether to send the publish trigger, enabler state, and 
       network notify indications. The values can be any combination of the 
       masks listed above.
       
       By default, all the event indications to the client are turned off 
       unless the client specifically turns on an indication.
            
       Note: Bit 4 is used for both QMI_IMSP_NOTIFY_XML_IND and 
       QMI_IMSP_NOTIFY_IND. If this is set, the notify is sent to the client. 
       Which indication method that will be used is controlled via 
       QMI_IMSP_SET_NOTIFY_FMT_REQ. Refer to QMI_IMSP_SET_NOTIFY_FMT_REQ to 
       request receiving either QMI_IMSP_NOTIFY_XML_IND or 
       QMI_IMSP_NOTIFY_IND.
   */
}imsp_set_event_report_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; This enables or disables various indications from the service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */
}imsp_set_event_report_resp_v01;  /* Message */
/**
    @}
  */

/*
 * imsp_get_event_report_req is empty
 * typedef struct {
 * }imsp_get_event_report_req_v01;
 */

/** @addtogroup imsp_qmi_messages
    @{
  */
/** Response Message; This gets the state of the event report bitmask of the service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
   */

  /* Optional */
  /*  Event Report Bitmask */
  uint8_t event_report_bit_masks_valid;  /**< Must be set to true if event_report_bit_masks is being passed */
  imsp_event_report_bit_masks_v01 event_report_bit_masks;
  /**<   Bitmask setting for which indications are sent to the client. 
       Bitmask of events registered by all control points. Values: \n
       - 1 -- IMSP_ENABLE_PUBLISH_TRIGGER_ IND -- 
              Send QMI_IMSP_PUBLISH_ TRIGGER_IND \n
       - 2 -- IMSP_ENABLE_ENABLER_STATE_ IND -- 
              Send QMI_IMSP_ENABLER_STATE_ IND \n
       - 4 -- IMSP_ENABLE_NOTIFY_IND -- 
              Send QMI_IMSP_NOTIFY_XML_IND or QMI_IMSP_NOTIFY_IND \n \vspace{-.12in}

       This indicates whether the publish trigger, enabler state, and 
       network notify indications are sent. The values can be any combination 
       of the masks listed above.
       
       By default, all the event indications to the client are turned off 
       unless the client specifically turns on an indication.
            
       Note: Bit 4 is used for both QMI_IMSP_NOTIFY_XML_IND and 
       QMI_IMSP_NOTIFY_IND. If this is set, the notify is sent to the client. 
       Which indication method that will be used is controlled via 
       QMI_IMSP_SET_NOTIFY_FMT_REQ. Refer to QMI_IMSP_SET_NOTIFY_FMT_REQ to 
       request receiving either QMI_IMSP_NOTIFY_XML_IND or 
       QMI_IMSP_NOTIFY_IND.
   */
}imsp_get_event_report_resp_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup imsp_qmi_msg_ids
    @{
  */
#define QMI_IMSP_PUBLISH_TRIGGER_IND_V01 0x0020
#define QMI_IMSP_NOTIFY_XML_IND_V01 0x0021
#define QMI_IMSP_NOTIFY_IND_V01 0x0022
#define QMI_IMSP_ENABLER_STATE_IND_V01 0x0023
#define QMI_IMSP_GET_ENABLER_STATE_REQ_V01 0x0024
#define QMI_IMSP_GET_ENABLER_STATE_RESP_V01 0x0024
#define QMI_IMSP_SEND_PUBLISH_REQ_V01 0x0025
#define QMI_IMSP_SEND_PUBLISH_RESP_V01 0x0025
#define QMI_IMSP_SEND_PUBLISH_IND_V01 0x0025
#define QMI_IMSP_SEND_PUBLISH_XML_REQ_V01 0x0026
#define QMI_IMSP_SEND_PUBLISH_XML_RESP_V01 0x0026
#define QMI_IMSP_SEND_PUBLISH_XML_IND_V01 0x0026
#define QMI_IMSP_SEND_UNPUBLISH_REQ_V01 0x0027
#define QMI_IMSP_SEND_UNPUBLISH_RESP_V01 0x0027
#define QMI_IMSP_SEND_UNPUBLISH_IND_V01 0x0027
#define QMI_IMSP_SEND_SUBSCRIBE_REQ_V01 0x0028
#define QMI_IMSP_SEND_SUBSCRIBE_RESP_V01 0x0028
#define QMI_IMSP_SEND_SUBSCRIBE_IND_V01 0x0028
#define QMI_IMSP_SEND_SUBSCRIBE_XML_REQ_V01 0x0029
#define QMI_IMSP_SEND_SUBSCRIBE_XML_RESP_V01 0x0029
#define QMI_IMSP_SEND_SUBSCRIBE_XML_IND_V01 0x0029
#define QMI_IMSP_SEND_UNSUBSCRIBE_REQ_V01 0x002A
#define QMI_IMSP_SEND_UNSUBSCRIBE_RESP_V01 0x002A
#define QMI_IMSP_SEND_UNSUBSCRIBE_IND_V01 0x002A
#define QMI_IMSP_SET_NOTIFY_FMT_REQ_V01 0x002B
#define QMI_IMSP_SET_NOTIFY_FMT_RESP_V01 0x002B
#define QMI_IMSP_GET_NOTIFY_FMT_REQ_V01 0x002C
#define QMI_IMSP_GET_NOTIFY_FMT_RESP_V01 0x002C
#define QMI_IMSP_SET_EVENT_REPORT_REQ_V01 0x002D
#define QMI_IMSP_SET_EVENT_REPORT_RESP_V01 0x002D
#define QMI_IMSP_GET_EVENT_REPORT_REQ_V01 0x002E
#define QMI_IMSP_GET_EVENT_REPORT_RESP_V01 0x002E
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsp_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsp_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define imsp_get_service_object_v01( ) \
          imsp_get_service_object_internal_v01( \
            IMSP_V01_IDL_MAJOR_VERS, IMSP_V01_IDL_MINOR_VERS, \
            IMSP_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

