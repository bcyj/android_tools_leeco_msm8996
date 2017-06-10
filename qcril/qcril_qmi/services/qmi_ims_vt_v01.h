#ifndef IMS_QMI_SERVICE_H
#define IMS_QMI_SERVICE_H
/**
  @file qmi_ims_vt_v01.h
  
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
   It was generated on: Mon Oct 17 2011
   From IDL File: qmi_ims_vt_v01.idl */

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
#define IMS_QMI_V01_MAX_MESSAGE_ID 0x0005;
/** 
    @} 
  */


/** @addtogroup ims_qmi_qmi_consts 
    @{ 
  */
#define QMI_IMS_VT_NUMBER_MAX_V01 81
#define QMI_IMS_VT_CALLER_ID_MAX_V01 81
#define QMI_IMS_VT_NAME_MAX_V01 182
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_TYPE_VT_V01 = 0, 
  CALL_TYPE_AUDIO_ONLY_V01 = 1, 
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
}ims_vt_dial_call_req_v01;  /* Message */
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
}ims_vt_dial_call_resp_v01;  /* Message */
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
}ims_vt_end_call_req_v01;  /* Message */
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
}ims_vt_end_call_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  ANSWER_PARAM_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  REJECT_V01 = 0, 
  ACCEPT_V01 = 1, 
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
}ims_vt_answer_call_req_v01;  /* Message */
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
}ims_vt_answer_call_resp_v01;  /* Message */
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
}ims_vt_get_call_info_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_STATE_ORIGINATING_V01 = 0, 
  CALL_STATE_INCOMING_V01 = 1, 
  CALL_STATE_CONVERSATION_V01 = 2, 
  CALL_STATE_CC_IN_PROGRESS_V01 = 3, 
  CALL_STATE_ALERTING_V01 = 4, 
  CALL_STATE_HOLD_V01 = 5, 
  CALL_STATE_WAITING_V01 = 6, 
  CALL_STATE_DISCONNECTING_V01 = 7, 
  CALL_STATE_END_V01 = 8, 
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
  CALL_DIRECTION_MO_V01 = 0, 
  CALL_DIRECTION_MT_V01 = 1, 
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
}ims_vt_call_info_type_v01;  /* Type */
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
  ims_vt_call_info_type_v01 call_info;

  /* Optional */
  /*  Remote Party Name** */
  uint8_t remote_party_name_valid;  /**< Must be set to true if remote_party_name is being passed */
  char remote_party_name[QMI_IMS_VT_NAME_MAX_V01 + 1];

  /* Optional */
  /*  Connected Number Information */
  uint8_t conn_num_info_valid;  /**< Must be set to true if conn_num_info is being passed */
  char conn_num_info[QMI_IMS_VT_NUMBER_MAX_V01 + 1];
}ims_vt_get_call_info_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ims_qmi_qmi_enums
    @{
  */
typedef enum {
  CALL_END_REASON_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_END_CAUSE_OFFLINE_V01 = 0, 
  CALL_END_CAUSE_NO_SRV_V01 = 1, 
  CALL_END_CAUSE_REL_NORMAL_V01 = 2, 
  CALL_END_CAUSE_CLIENT_END_V01 = 3, 
  CALL_END_CAUSE_INCOM_REJ_V01 = 4, 
  CALL_END_CAUSE_NETWORK_END_V01 = 5, 
  CALL_END_CAUSE_USER_BUSY_V01 = 6, 
  CALL_END_CAUSE_USER_ALERTING_NO_ANSWER_V01 = 7, 
  CALL_END_CAUSE_CALL_REJECTED_V01 = 8, 
  CALL_END_CAUSE_NORMAL_UNSPECIFIED_V01 = 9, 
  CALL_END_CAUSE_TEMPORARY_FAILURE_V01 = 10, 
  CALL_END_REASON_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_end_reason_enum_v01;
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
  ims_vt_call_info_type_v01 call_info;

  /* Optional */
  /*  Remote Party Name** */
  uint8_t remote_party_name_valid;  /**< Must be set to true if remote_party_name is being passed */
  char remote_party_name[QMI_IMS_VT_NAME_MAX_V01 + 1];

  /* Optional */
  /*  Connected Number Information */
  uint8_t conn_num_info_valid;  /**< Must be set to true if conn_num_info is being passed */
  char conn_num_info[QMI_IMS_VT_NUMBER_MAX_V01 + 1];

  /* Optional */
  /*  Call end reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  call_end_reason_enum_v01 call_end_reason;
}ims_vt_call_status_ind_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup ims_qmi_qmi_msg_ids
    @{
  */
#define IMS_VT_CALL_STATUS_IND_V01 0x0001
#define IMS_VT_DIAL_CALL_REQ_V01 0x0002
#define IMS_VT_DIAL_CALL_RESP_V01 0x0002
#define IMS_VT_END_CALL_REQ_V01 0x0003
#define IMS_VT_END_CALL_RESP_V01 0x0003
#define IMS_VT_ANSWER_CALL_REQ_V01 0x0004
#define IMS_VT_ANSWER_CALL_RESP_V01 0x0004
#define IMS_VT_GET_CALL_INFO_REQ_V01 0x0005
#define IMS_VT_GET_CALL_INFO_RESP_V01 0x0005
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

