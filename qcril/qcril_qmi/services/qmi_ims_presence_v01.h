#ifndef QMI_IMS_PRESENCE_V01_SERVICE_H //QMI_IMS_PRESENCE_SERVICE_H
#define QMI_IMS_PRESENCE_V01_SERVICE_H //QMI_IMS_PRESENCE_SERVICE_H
/**
  @file qmi_ims_presence_v01.h
  
  @brief This is the public header file which defines the qmi_ims_presence service Data structures.

  This header file defines the types and structures that were defined in 
  qmi_ims_presence. It contains the constant values defined, enums, structures,
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
   It was generated on: Mon Oct 10 2011
   From IDL File: qmi_ims_presence_v01.idl */

/** @defgroup qmi_ims_presence_qmi_consts Constant values defined in the IDL */
/** @defgroup qmi_ims_presence_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qmi_ims_presence_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qmi_ims_presence_qmi_messages Structures sent as QMI messages */
/** @defgroup qmi_ims_presence_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qmi_ims_presence_qmi_accessor Accessor for QMI service object */
/** @defgroup qmi_ims_presence_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qmi_ims_presence_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define QMI_IMS_PRESENCE_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QMI_IMS_PRESENCE_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QMI_IMS_PRESENCE_V01_IDL_TOOL_VERS 0x02
/** Maximum Defined Message ID */
#define QMI_IMS_PRESENCE_V01_MAX_MESSAGE_ID 0x0010;
/** 
    @} 
  */


/** @addtogroup qmi_ims_presence_qmi_consts 
    @{ 
  */
#define RESPONSE_DETAILS_MAX_V01 512
#define PRESENCE_XML_MAX_V01 1024

/** 			  */
#define SIMPLE_XML_MAX_V01 16
#define PEER_URI_MAX_V01 255
#define SUBS_EVENT_MAX_V01 255
#define PAR_RULE_ID_MAX_V01 64
#define PAR_RULE_MAX_V01 64
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_enums
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; This is a request from AP to Modem side for the enabler. */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** EnablerStateIndication Message; This is a modem initiated indication of an enabler state when registered */
typedef struct {

  /* Mandatory */
  enabler_state_enum_v01 enabler_state;
}ims_enabler_state_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** SimplePublish Message; Simple Publish Request */
typedef struct {

  /* Mandatory */
  char simplePublish[SIMPLE_XML_MAX_V01 + 1];
}ims_send_simple_publish_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Simple Publish Request */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  uint8_t response_details_valid;  /**< Must be set to true if response_details is being passed */
  char response_details[RESPONSE_DETAILS_MAX_V01 + 1];
}ims_send_simple_publish_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** SendPublishRequestforRichPresence Message; Publish Request */
typedef struct {

  /* Mandatory */
  /*  Publish Request XML */
  char publish_xml[PRESENCE_XML_MAX_V01 + 1];
}ims_send_publish_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Publish Request */
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
 * ims_send_unpublish_req is empty
 * typedef struct {
 * }ims_send_unpublish_req_v01;
 */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; UnPublish Request (Create and Send) */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** (CreateandSend)SubscribeRequest Message; (Create and Send) Subscribe Request  */
typedef struct {

  /* Mandatory */
  uint8_t subscription_type;

  /* Mandatory */
  char peerURI[PEER_URI_MAX_V01 + 1];

  /* Mandatory */
  uint8_t isRLSSubscription;

  /* Optional */
  uint8_t userList_valid;  /**< Must be set to true if userList is being passed */
  char userList[PRESENCE_XML_MAX_V01 + 1];
}ims_send_subscribe_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; (Create and Send) Subscribe Request  */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** UnSubscribeRequest(CreateandSend) Message; UnSubscribe Request (Create and Send) */
typedef struct {

  /* Mandatory */
  char peerURI[PEER_URI_MAX_V01 + 1];
}ims_send_unsubscribe_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; UnSubscribe Request (Create and Send) */
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

/** @addtogroup qmi_ims_presence_qmi_enums
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** IndicationforthePresenceSIPmessagestatusfromthenetwork. Message; Indication for the Presence SIP message status from the network. */
typedef struct {

  /* Mandatory */
  sip_msg_status_enum_v01 status_response;

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

/** @addtogroup qmi_ims_presence_qmi_enums
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** IndicationforthePresenceSIPmessagestatusfromthenetwork. Message; Indication for the Presence SIP message status from the network. */
typedef struct {

  /* Mandatory */
  http_msg_status_enum_v01 status_response;

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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** AddParRule Message; Add Par Rule */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Add Par Rule */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** GetParRule Message; Get Par Rule */
typedef struct {

  /* Mandatory */
  uint32_t parID;
}ims_get_par_rule_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Get Par Rule */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** GetParRuleAtIndex Message; Get Par Rule At Index */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  uint32_t index;
}ims_get_par_rule_at_index_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Get Par Rule At Index */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** DeleteParRuleAtIndex Message; Delete Par Rule At Index */
typedef struct {

  /* Mandatory */
  uint32_t parID;

  /* Mandatory */
  uint32_t index;
}ims_del_par_rule_at_index_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Delete Par Rule At Index */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** DeleteParRule Message; Delete Par Rule  */
typedef struct {

  /* Mandatory */
  uint32_t parID;
}ims_del_par_rule_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Delete Par Rule  */
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** DeleteParDocumentRequest Message; Delete Par Document Request */
typedef struct {

  /* Mandatory */
  uint32_t parID;
}ims_del_par_document_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Delete Par Document Request */
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

/** @addtogroup qmi_ims_presence_qmi_enums
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

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/** CreatePAR(PresenceAuthorizationRules)Request Message; Create PAR (Presence Authorization Rules) Request  */
typedef struct {

  /* Mandatory */
  par_method_name_enum_v01 method_name;

  /* Mandatory */
  char requestXML[PRESENCE_XML_MAX_V01 + 1];
}ims_send_par_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qmi_ims_presence_qmi_messages
    @{
  */
/**  Message; Create PAR (Presence Authorization Rules) Request  */
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

/*Service Message Definition*/
/** @addtogroup qmi_ims_presence_qmi_msg_ids
    @{
  */
#define IMS_INCOMING_SIP_STATUS_IND_V01 0x0001
#define IMS_INCOMING_HTTP_STATUS_IND_V01 0x0002
#define IMS_ENABLER_STATE_IND_V01 0x0003
#define IMS_ENABLER_STATE_REQ_V01 0x0004
#define IMS_ENABLER_STATE_RESP_V01 0x0004
#define IMS_SEND_SIMPLE_PUBLISH_REQ_V01 0x0005
#define IMS_SEND_SIMPLE_PUBLISH_RESP_V01 0x0005
#define IMS_SEND_PUBLISH_REQ_V01 0x0006
#define IMS_SEND_PUBLISH_RESP_V01 0x0006
#define IMS_SEND_UNPUBLISH_REQ_V01 0x0007
#define IMS_SEND_UNPUBLISH_RESP_V01 0x0007
#define IMS_SEND_SUBSCRIBE_REQ_V01 0x0008
#define IMS_SEND_SUBSCRIBE_RESP_V01 0x0008
#define IMS_SEND_UNSUBSCRIBE_REQ_V01 0x0009
#define IMS_SEND_UNSUBSCRIBE_RESP_V01 0x0009
#define IMS_ADD_PAR_RULE_REQ_V01 0x000A
#define IMS_ADD_PAR_RULE_RESP_V01 0x000A
#define IMS_GET_PAR_RULE_REQ_V01 0x000B
#define IMS_GET_PAR_RULE_RESP_V01 0x000B
#define IMS_GET_PAR_RULE_AT_INDEX_REQ_V01 0x000C
#define IMS_GET_PAR_RULE_AT_INDEX_RESP_V01 0x000C
#define IMS_DEL_PAR_RULE_AT_INDEX_REQ_V01 0x000D
#define IMS_DEL_PAR_RULE_AT_INDEX_RESP_V01 0x000D
#define IMS_DEL_PAR_RULE_REQ_V01 0x000E
#define IMS_DEL_PAR_RULE_RESP_V01 0x000E
#define IMS_DEL_PAR_DOCUMENT_REQ_V01 0x000F
#define IMS_DEL_PAR_DOCUMENT_RESP_V01 0x000F
#define IMS_SEND_PAR_REQ_V01 0x0010
#define IMS_SEND_PAR_RESP_V01 0x0010
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qmi_ims_presence_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qmi_ims_presence_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qmi_ims_presence_get_service_object_v01( ) \
          qmi_ims_presence_get_service_object_internal_v01( \
            QMI_IMS_PRESENCE_V01_IDL_MAJOR_VERS, QMI_IMS_PRESENCE_V01_IDL_MINOR_VERS, \
            QMI_IMS_PRESENCE_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif // QMI_IMS_PRESENCE_V01_SERVICE_H

