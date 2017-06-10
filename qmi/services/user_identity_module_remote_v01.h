#ifndef UIM_REMOTE_SERVICE_01_H
#define UIM_REMOTE_SERVICE_01_H
/**
  @file user_identity_module_remote_v01.h

  @brief This is the public header file which defines the uim_remote service Data structures.

  This header file defines the types and structures that were defined in
  uim_remote. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/uimrmt/main/latest/api/user_identity_module_remote_v01.h#4 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.7
   It was generated on: Wed May 21 2014 (Spin 0)
   From IDL File: user_identity_module_remote_v01.idl */

/** @defgroup uim_remote_qmi_consts Constant values defined in the IDL */
/** @defgroup uim_remote_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup uim_remote_qmi_enums Enumerated types used in QMI messages */
/** @defgroup uim_remote_qmi_messages Structures sent as QMI messages */
/** @defgroup uim_remote_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup uim_remote_qmi_accessor Accessor for QMI service object */
/** @defgroup uim_remote_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup uim_remote_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define UIM_REMOTE_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define UIM_REMOTE_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define UIM_REMOTE_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define UIM_REMOTE_V01_MAX_MESSAGE_ID 0x0027
/**
    @}
  */


/** @addtogroup uim_remote_qmi_consts
    @{
  */
#define QMI_UIM_REMOTE_MAX_ATR_LEN_V01 32
#define QMI_UIM_REMOTE_MAX_COMMAND_APDU_LEN_V01 261
#define QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01 1024
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}uim_remote_reset_req_msg_v01;

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Response Message; Resets the service state variables of the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}uim_remote_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_enums
    @{
  */
typedef enum {
  UIM_REMOTE_EVENT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  UIM_REMOTE_CONNECTION_UNAVAILABLE_V01 = 0x0, /**<  Connection is unavailable \n  */
  UIM_REMOTE_CONNECTION_AVAILABLE_V01 = 0x1, /**<  Connection is available \n  */
  UIM_REMOTE_CARD_INSERTED_V01 = 0x2, /**<  Card is inserted \n  */
  UIM_REMOTE_CARD_REMOVED_V01 = 0x3, /**<  Card was removed  \n  */
  UIM_REMOTE_CARD_ERROR_V01 = 0x4, /**<  Card error \n  */
  UIM_REMOTE_CARD_RESET_V01 = 0x5, /**<  Card reset \n  */
  UIM_REMOTE_CARD_WAKEUP_V01 = 0x6, /**<  Card wake-up  */
  UIM_REMOTE_EVENT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}uim_remote_event_type_enum_v01;
/**
    @}
  */

/** @addtogroup uim_remote_qmi_enums
    @{
  */
typedef enum {
  UIM_REMOTE_SLOT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  UIM_REMOTE_SLOT_NOT_APPLICABLE_V01 = 0x0, /**<  Not applicable \n  */
  UIM_REMOTE_SLOT_1_V01 = 0x1, /**<  Slot 1 \n  */
  UIM_REMOTE_SLOT_2_V01 = 0x2, /**<  Slot 2 \n  */
  UIM_REMOTE_SLOT_3_V01 = 0x3, /**<  Slot 3  */
  UIM_REMOTE_SLOT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}uim_remote_slot_type_enum_v01;
/**
    @}
  */

/** @addtogroup uim_remote_qmi_enums
    @{
  */
typedef enum {
  UIM_REMOTE_CARD_ERROR_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01 = 0x0, /**<  Unknown error \n  */
  UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01 = 0x1, /**<  No link was established \n  */
  UIM_REMOTE_CARD_ERROR_COMMAND_TIMEOUT_V01 = 0x2, /**<  Command timeout \n  */
  UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_V01 = 0x3, /**<  Error due to card power-down  */
  UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_TELECOM_V01 = 0x4, /**<  Error due to telecom power-down  */
  UIM_REMOTE_CARD_ERROR_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}uim_remote_card_error_type_enum_v01;
/**
    @}
  */

/** @addtogroup uim_remote_qmi_aggregates
    @{
  */
typedef struct {

  uim_remote_event_type_enum_v01 event;
  /**<   Event type received from the card. Values: \n
      - UIM_REMOTE_CONNECTION_UNAVAILABLE (0x0) --  Connection is unavailable \n
      - UIM_REMOTE_CONNECTION_AVAILABLE (0x1) --  Connection is available \n
      - UIM_REMOTE_CARD_INSERTED (0x2) --  Card is inserted \n
      - UIM_REMOTE_CARD_REMOVED (0x3) --  Card was removed  \n
      - UIM_REMOTE_CARD_ERROR (0x4) --  Card error \n
      - UIM_REMOTE_CARD_RESET (0x5) --  Card reset \n
      - UIM_REMOTE_CARD_WAKEUP (0x6) --  Card wake-up  */

  uim_remote_slot_type_enum_v01 slot;
  /**<   Card slot for the event type received. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */
}uim_remote_event_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Request Message; Notifies the service of remote UIM events. */
typedef struct {

  /* Mandatory */
  /*  UIM Remote Event Information */
  uim_remote_event_info_type_v01 event_info;

  /* Optional */
  /*  UIM Remote Answer to Reset Bytes */
  uint8_t atr_valid;  /**< Must be set to true if atr is being passed */
  uint32_t atr_len;  /**< Must be set to # of elements in atr */
  uint8_t atr[QMI_UIM_REMOTE_MAX_ATR_LEN_V01];
  /**<   Answer to reset
  */

  /* Optional */
  /*  UIM Remote Wakeup Support */
  uint8_t wakeup_support_valid;  /**< Must be set to true if wakeup_support is being passed */
  uint8_t wakeup_support;
  /**<   Indicates whether the UIM Remote supports the wake-up property.
  */

  /* Optional */
  /*  Error Cause for Card Error Event */
  uint8_t error_cause_valid;  /**< Must be set to true if error_cause is being passed */
  uim_remote_card_error_type_enum_v01 error_cause;
  /**<   Indicates the cause of error for a card error event. \n
      - UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR (0x0) --  Unknown error \n
      - UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED (0x1) --  No link was established \n
      - UIM_REMOTE_CARD_ERROR_COMMAND_TIMEOUT (0x2) --  Command timeout \n
      - UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN (0x3) --  Error due to card power-down
      - UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_TELECOM (0x4) --  Error due to telecom power-down  */
}uim_remote_event_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Response Message; Notifies the service of remote UIM events. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described
                          in the error codes section of each message definition.
  */
}uim_remote_event_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t total_response_apdu_size;
  /**<   Total response APDU size for the transaction.
  */

  uint32_t response_apdu_segment_offset;
  /**<   Offset of the APDU segment in the message.
  */
}uim_remote_response_apdu_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Request Message; Exchanges the APDU with the remote card. */
typedef struct {

  /* Mandatory */
  /*  Status of APDU Transaction */
  qmi_result_type_v01 apdu_status;
  /**<   APDU status will be either QMI_RESULT_SUCCESS or -QMI_RESULT_FAILURE.
  */

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */

  /* Mandatory */
  /*  APDU ID */
  uint32_t apdu_id;
  /**<   Identifier for a command and response APDU pair.
  */

  /* Optional */
  /*  Response APDU Information */
  uint8_t response_apdu_info_valid;  /**< Must be set to true if response_apdu_info is being passed */
  uim_remote_response_apdu_info_type_v01 response_apdu_info;

  /* Optional */
  /*  Response APDU */
  uint8_t response_apdu_segment_valid;  /**< Must be set to true if response_apdu_segment is being passed */
  uint32_t response_apdu_segment_len;  /**< Must be set to # of elements in response_apdu_segment */
  uint8_t response_apdu_segment[QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01];
  /**<   APDU returned from the control point.
  */
}uim_remote_apdu_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Response Message; Exchanges the APDU with the remote card. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described
                          in the error codes section of each message definition.
  */
}uim_remote_apdu_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Indication Message; Indication to the control point to transmit an APDU to the card. */
typedef struct {

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */

  /* Mandatory */
  /*  APDU ID */
  uint32_t apdu_id;
  /**<   Identifier for a command and response APDU pair.
  */

  /* Mandatory */
  /*  Command APDU */
  uint32_t command_apdu_len;  /**< Must be set to # of elements in command_apdu */
  uint8_t command_apdu[QMI_UIM_REMOTE_MAX_COMMAND_APDU_LEN_V01];
  /**<   APDU request sent to a control point.
  */
}uim_remote_apdu_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Indication Message; Indication to the control point to establish a connection with the card. */
typedef struct {

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */
}uim_remote_connect_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Indication Message; Indication to the control point to tear down the connection with the card. */
typedef struct {

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */
}uim_remote_disconnect_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_enums
    @{
  */
typedef enum {
  UIM_REMOTE_POWER_DOWN_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  UIM_REMOTE_POWER_DOWN_TELECOM_INTERFACE_V01 = 0x0, /**<  Power down telecom only \n  */
  UIM_REMOTE_POWER_DOWN_CARD_V01 = 0x1, /**<  Power down card \n  */
  UIM_REMOTE_POWER_DOWN_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}uim_remote_power_down_mode_enum_v01;
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Indication Message; Indication to the control point to power down the card. */
typedef struct {

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */

  /* Optional */
  /*  Power Down Mode */
  uint8_t mode_valid;  /**< Must be set to true if mode is being passed */
  uim_remote_power_down_mode_enum_v01 mode;
  /**<   Power down mode. Values: \n
      - UIM_REMOTE_POWER_DOWN_TELECOM_INTERFACE (0x0) --  Power down telecom only \n
      - UIM_REMOTE_POWER_DOWN_CARD (0x1) --  Power down card \n  \n
 All other values are reserved for future use.
 */
}uim_remote_card_power_down_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_enums
    @{
  */
typedef enum {
  UIM_REMOTE_VOLTAGE_CLASS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  UIM_REMOTE_VOLTAGE_CLASS_C_LOW_V01 = 0x0, /**<  VOLTAGE_CLASS_C_LOW \n  */
  UIM_REMOTE_VOLTAGE_CLASS_C_V01 = 0x1, /**<  VOLTAGE_CLASS_C \n  */
  UIM_REMOTE_VOLTAGE_CLASS_C_HIGH_V01 = 0x2, /**<  VOLTAGE_CLASS_C_HIGH  \n  */
  UIM_REMOTE_VOLTAGE_CLASS_B_LOW_V01 = 0x3, /**<  VOLTAGE_CLASS_B_LOW \n  */
  UIM_REMOTE_VOLTAGE_CLASS_B_V01 = 0x4, /**<  VOLTAGE_CLASS_B \n  */
  UIM_REMOTE_VOLTAGE_CLASS_B_HIGH_V01 = 0x5, /**<  VOLTAGE_CLASS_B_HIGH  */
  UIM_REMOTE_VOLTAGE_CLASS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}uim_remote_voltage_class_enum_v01;
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Indication Message; Indication to the control point to power up the card. */
typedef struct {

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */

  /* Optional */
  /*  Response Timeout */
  uint8_t response_timeout_valid;  /**< Must be set to true if response_timeout is being passed */
  uint32_t response_timeout;
  /**<   Response timeout in ms */

  /* Optional */
  /*  Voltage Class */
  uint8_t voltage_class_valid;  /**< Must be set to true if voltage_class is being passed */
  uim_remote_voltage_class_enum_v01 voltage_class;
  /**<   Voltage class. Values: \n
      - UIM_REMOTE_VOLTAGE_CLASS_C_LOW (0x0) --  VOLTAGE_CLASS_C_LOW \n
      - UIM_REMOTE_VOLTAGE_CLASS_C (0x1) --  VOLTAGE_CLASS_C \n
      - UIM_REMOTE_VOLTAGE_CLASS_C_HIGH (0x2) --  VOLTAGE_CLASS_C_HIGH  \n
      - UIM_REMOTE_VOLTAGE_CLASS_B_LOW (0x3) --  VOLTAGE_CLASS_B_LOW \n
      - UIM_REMOTE_VOLTAGE_CLASS_B (0x4) --  VOLTAGE_CLASS_B \n
      - UIM_REMOTE_VOLTAGE_CLASS_B_HIGH (0x5) --  VOLTAGE_CLASS_B_HIGH  \n
 All other values are reserved for future use. */
}uim_remote_card_power_up_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup uim_remote_qmi_messages
    @{
  */
/** Indication Message; Indication to the control point to reset the card. */
typedef struct {

  /* Mandatory */
  /*  Card Slot */
  uim_remote_slot_type_enum_v01 slot;
  /**<   Slot type. Values: \n
      - UIM_REMOTE_SLOT_NOT_APPLICABLE (0x0) --  Not applicable \n
      - UIM_REMOTE_SLOT_1 (0x1) --  Slot 1 \n
      - UIM_REMOTE_SLOT_2 (0x2) --  Slot 2 \n
      - UIM_REMOTE_SLOT_3 (0x3) --  Slot 3  */
}uim_remote_card_reset_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup uim_remote_qmi_msg_ids
    @{
  */
#define QMI_UIM_REMOTE_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_UIM_REMOTE_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_UIM_REMOTE_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_UIM_REMOTE_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_UIM_REMOTE_RESET_REQ_V01 0x0020
#define QMI_UIM_REMOTE_RESET_RESP_V01 0x0020
#define QMI_UIM_REMOTE_EVENT_REQ_V01 0x0021
#define QMI_UIM_REMOTE_EVENT_RESP_V01 0x0021
#define QMI_UIM_REMOTE_APDU_REQ_V01 0x0022
#define QMI_UIM_REMOTE_APDU_RESP_V01 0x0022
#define QMI_UIM_REMOTE_APDU_IND_V01 0x0022
#define QMI_UIM_REMOTE_CONNECT_IND_V01 0x0023
#define QMI_UIM_REMOTE_DISCONNECT_IND_V01 0x0024
#define QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01 0x0025
#define QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01 0x0026
#define QMI_UIM_REMOTE_CARD_RESET_IND_V01 0x0027
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro uim_remote_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type uim_remote_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define uim_remote_get_service_object_v01( ) \
          uim_remote_get_service_object_internal_v01( \
            UIM_REMOTE_V01_IDL_MAJOR_VERS, UIM_REMOTE_V01_IDL_MINOR_VERS, \
            UIM_REMOTE_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

