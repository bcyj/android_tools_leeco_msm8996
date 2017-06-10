#ifndef SAR_SERVICE_01_H
#define SAR_SERVICE_01_H
/**
  @file specific_absorption_rate_v01.h

  @brief This is the public header file which defines the sar service Data structures.

  This header file defines the types and structures that were defined in
  sar. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header: //components/rel/qmimsgs.mpss/3.4/sar/api/specific_absorption_rate_v01.h#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It requires encode/decode library version 5 or later
   It was generated on: Thu May 30 2013 (Spin 0)
   From IDL File: specific_absorption_rate_v01.idl */

/** @defgroup sar_qmi_consts Constant values defined in the IDL */
/** @defgroup sar_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sar_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sar_qmi_messages Structures sent as QMI messages */
/** @defgroup sar_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sar_qmi_accessor Accessor for QMI service object */
/** @defgroup sar_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sar_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SAR_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SAR_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SAR_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SAR_V01_MAX_MESSAGE_ID 0x0020;
/**
    @}
  */


/** @addtogroup sar_qmi_consts
    @{
  */
/**
    @}
  */

/** @addtogroup sar_qmi_enums
    @{
  */
typedef enum {
  QMI_SAR_RF_STATE_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_SAR_RF_STATE_DEFAULT_V01 = 0, /**<  Default RF state.  */
  QMI_SAR_RF_STATE_1_V01 = 1, /**<  RF state 1. \n  */
  QMI_SAR_RF_STATE_2_V01 = 2, /**<  RF state 2. \n  */
  QMI_SAR_RF_STATE_3_V01 = 3, /**<  RF state 3. \n  */
  QMI_SAR_RF_STATE_4_V01 = 4, /**<  RF state 4. \n  */
  QMI_SAR_RF_STATE_5_V01 = 5, /**<  RF state 5. \n  */
  QMI_SAR_RF_STATE_6_V01 = 6, /**<  RF state 6. \n  */
  QMI_SAR_RF_STATE_7_V01 = 7, /**<  RF state 7. \n  */
  QMI_SAR_RF_STATE_8_V01 = 8, /**<  RF state 8.  */
  QMI_SAR_RF_STATE_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_sar_rf_state_enum_type_v01;
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; Sets the specified SAR RF state. */
typedef struct {

  /* Mandatory */
  /*  SAR RF State */
  qmi_sar_rf_state_enum_type_v01 sar_rf_state;
  /**<
 SAR RF state must be specified:
      - QMI_SAR_RF_STATE_DEFAULT (0) --  Default RF state.
      - QMI_SAR_RF_STATE_1 (1) --  RF state 1. \n
      - QMI_SAR_RF_STATE_2 (2) --  RF state 2. \n
      - QMI_SAR_RF_STATE_3 (3) --  RF state 3. \n
      - QMI_SAR_RF_STATE_4 (4) --  RF state 4. \n
      - QMI_SAR_RF_STATE_5 (5) --  RF state 5. \n
      - QMI_SAR_RF_STATE_6 (6) --  RF state 6. \n
      - QMI_SAR_RF_STATE_7 (7) --  RF state 7. \n
      - QMI_SAR_RF_STATE_8 (8) --  RF state 8.
 */

  /* Optional */
  uint8_t compatibility_key_valid;  /**< Must be set to true if compatibility_key is being passed */
  uint32_t compatibility_key;
  /**<
     Compatibility Key that need to be verified.

  */
}sar_rf_set_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Sets the specified SAR RF state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
     qmi_error_type  - Error code. Possible error code values are
     described in the error codes section of each message definition.
    */
}sar_rf_set_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * sar_rf_get_state_req_msg is empty
 * typedef struct {
 * }sar_rf_get_state_req_msg_v01;
 */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Gets the specified SAR RF state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Vendor Specific Configuration */
  uint8_t sar_rf_state_valid;  /**< Must be set to true if sar_rf_state is being passed */
  qmi_sar_rf_state_enum_type_v01 sar_rf_state;
  /**<
 SAR RF state must be specified:
      - QMI_SAR_RF_STATE_DEFAULT (0) --  Default RF state.
      - QMI_SAR_RF_STATE_1 (1) --  RF state 1. \n
      - QMI_SAR_RF_STATE_2 (2) --  RF state 2. \n
      - QMI_SAR_RF_STATE_3 (3) --  RF state 3. \n
      - QMI_SAR_RF_STATE_4 (4) --  RF state 4. \n
      - QMI_SAR_RF_STATE_5 (5) --  RF state 5. \n
      - QMI_SAR_RF_STATE_6 (6) --  RF state 6. \n
      - QMI_SAR_RF_STATE_7 (7) --  RF state 7. \n
      - QMI_SAR_RF_STATE_8 (8) --  RF state 8.
 */
}sar_rf_get_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * sar_rf_get_compatibility_key_req_msg is empty
 * typedef struct {
 * }sar_rf_get_compatibility_key_req_msg_v01;
 */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Gets the compatibility Key info. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Vendor Specific Configuration */
  uint8_t compatibility_key_valid;  /**< Must be set to true if compatibility_key is being passed */
  uint32_t compatibility_key;
  /**<
     Compatibility Key value.
  */
}sar_rf_get_compatibility_key_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup sar_qmi_msg_ids
    @{
  */
#define QMI_SAR_RF_SET_STATE_REQ_MSG_V01 0x0001
#define QMI_SAR_RF_SET_STATE_RESP_MSG_V01 0x0001
#define QMI_SAR_RF_GET_STATE_REQ_MSG_V01 0x0002
#define QMI_SAR_RF_GET_STATE_RESP_MSG_V01 0x0002
#define QMI_SAR_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_SAR_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_SAR_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_SAR_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_SAR_GET_COMPATIBILITY_KEY_REQ_MSG_V01 0x0020
#define QMI_SAR_GET_COMPATIBILITY_KEY_RESP_MSG_V01 0x0020
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sar_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sar_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define sar_get_service_object_v01( ) \
          sar_get_service_object_internal_v01( \
            SAR_V01_IDL_MAJOR_VERS, SAR_V01_IDL_MINOR_VERS, \
            SAR_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

