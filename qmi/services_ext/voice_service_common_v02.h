#ifndef VOICE_SERVICE_COMMON_SERVICE_02_H
#define VOICE_SERVICE_COMMON_SERVICE_02_H
/**
  @file voice_service_common_v02.h
  
  @brief This is the public header file which defines the voice_service_common service Data structures.

  This header file defines the types and structures that were defined in 
  voice_service_common. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/voice/main/latest/api/voice_service_common_v02.h#1 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.5 
   It was generated on: Fri Oct 25 2013 (Spin 0)
   From IDL File: voice_service_common_v02.idl */

/** @defgroup voice_service_common_qmi_consts Constant values defined in the IDL */
/** @defgroup voice_service_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup voice_service_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup voice_service_common_qmi_messages Structures sent as QMI messages */
/** @defgroup voice_service_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup voice_service_common_qmi_accessor Accessor for QMI service object */
/** @defgroup voice_service_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup voice_service_common_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define VOICE_SERVICE_COMMON_V02_IDL_MAJOR_VERS 0x02
/** Revision Number of the IDL used to generate this file */
#define VOICE_SERVICE_COMMON_V02_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define VOICE_SERVICE_COMMON_V02_IDL_TOOL_VERS 0x06

/** 
    @} 
  */

/** @addtogroup voice_service_common_qmi_enums
    @{
  */
typedef enum {
  CALL_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_TYPE_VOICE_V02 = 0x00, /**<  Voice \n  */
  CALL_TYPE_VOICE_FORCED_V02 = 0x01, /**<  Avoid modem call classification \n  */
  CALL_TYPE_VOICE_IP_V02 = 0x02, /**<  Voice over IP  \n  */
  CALL_TYPE_VT_V02 = 0x03, /**<  Videotelephony call over IP \n  */
  CALL_TYPE_VIDEOSHARE_V02 = 0x04, /**<  Videoshare \n  */
  CALL_TYPE_TEST_V02 = 0x05, /**<  Test call type \n  */
  CALL_TYPE_OTAPA_V02 = 0x06, /**<  OTAPA \n  */
  CALL_TYPE_STD_OTASP_V02 = 0x07, /**<  Standard OTASP \n  */
  CALL_TYPE_NON_STD_OTASP_V02 = 0x08, /**<  Nonstandard OTASP \n  */
  CALL_TYPE_EMERGENCY_V02 = 0x09, /**<  Emergency \n  */
  CALL_TYPE_SUPS_V02 = 0x0A, /**<  Supplementary service \n  */
  CALL_TYPE_EMERGENCY_IP_V02 = 0x0B, /**<  Emergency VoIP \n  */
  CALL_TYPE_ECALL_V02 = 0x0C, /**<  eCall \n  */
  CALL_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_type_enum_v02;
/**
    @}
  */

/** @addtogroup voice_service_common_qmi_enums
    @{
  */
typedef enum {
  CALL_STATE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_STATE_ORIGINATING_V02 = 0x01, /**<  Origination \n  */
  CALL_STATE_INCOMING_V02 = 0x02, /**<  Incoming \n  */
  CALL_STATE_CONVERSATION_V02 = 0x03, /**<  Conversation \n  */
  CALL_STATE_CC_IN_PROGRESS_V02 = 0x04, /**<  Call is originating but waiting for call control to complete  \n  */
  CALL_STATE_ALERTING_V02 = 0x05, /**<  Alerting \n  */
  CALL_STATE_HOLD_V02 = 0x06, /**<  Hold \n  */
  CALL_STATE_WAITING_V02 = 0x07, /**<  Waiting \n  */
  CALL_STATE_DISCONNECTING_V02 = 0x08, /**<  Disconnecting \n  */
  CALL_STATE_END_V02 = 0x09, /**<  End \n  */
  CALL_STATE_SETUP_V02 = 0x0A, /**<  MT call is in Setup state in 3GPP \n  */
  CALL_STATE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_state_enum_v02;
/**
    @}
  */

/** @addtogroup voice_service_common_qmi_enums
    @{
  */
typedef enum {
  CALL_DIRECTION_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CALL_DIRECTION_MO_V02 = 0x01, /**<  MO call \n  */
  CALL_DIRECTION_MT_V02 = 0x02, /**<  MT call \n  */
  CALL_DIRECTION_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}call_direction_enum_v02;
/**
    @}
  */

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object voice_service_common_qmi_idl_type_table_object_v02;


#ifdef __cplusplus
}
#endif
#endif

