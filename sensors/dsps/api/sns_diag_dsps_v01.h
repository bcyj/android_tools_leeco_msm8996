#ifndef SNS_DIAG_DSPS_SVC_SERVICE_01_H
#define SNS_DIAG_DSPS_SVC_SERVICE_01_H
/**
  @file sns_diag_dsps_v01.h

  @brief This is the public header file which defines the SNS_DIAG_DSPS_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_DIAG_DSPS_SVC. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13
   It was generated on: Wed Sep  3 2014 (Spin 0)
   From IDL File: sns_diag_dsps_v01.idl */

/** @defgroup SNS_DIAG_DSPS_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_DIAG_DSPS_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_DIAG_DSPS_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_DIAG_DSPS_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_DIAG_DSPS_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_DIAG_DSPS_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_DIAG_DSPS_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_DIAG_DSPS_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_DIAG_DSPS_SVC_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_DIAG_DSPS_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_DIAG_DSPS_SVC_V01_MAX_MESSAGE_ID 0x0004
/**
    @}
  */


/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_consts
    @{
  */

/**

 max # of options in an options msg */
#define SNS_DIAG_MAX_OPTIONS_V01 16
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_aggregates
    @{
  */
/**  structure containing log mask information.
    On DSPS QMI IDL type table should not be empty
 */
typedef struct {

  uint64_t mask;
  /**<   64 bit mask indicating which diag elements are enabled */
}sns_diag_mask_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_aggregates
    @{
  */
/**  structure defininig debug options and their values
 */
typedef struct {

  uint32_t option;
  /**<   option to set value for. Opaque;- NOT enumerated in IDL*/

  uint64_t value;
  /**<   option value to be set. Opaque;- NOT enumerated in IDL*/
}sns_diag_debug_options_v01;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_messages
    @{
  */
/** Request Message; This command sets generic debug options */
typedef struct {

  /* Mandatory */
  uint32_t options_len;  /**< Must be set to # of elements in options */
  sns_diag_debug_options_v01 options[SNS_DIAG_MAX_OPTIONS_V01];
  /**<   Options array */
}sns_diag_set_debug_options_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_messages
    @{
  */
/** Response Message; This command sets generic debug options */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   response indicating whether the request suceeded or failed */
}sns_diag_set_debug_options_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_messages
    @{
  */
/** Request Message; This command sets the log mask for sensors diag module on DSPS */
typedef struct {

  /* Mandatory */
  sns_diag_mask_s_v01 log_mask;
  /**<   log mask indicating which log packets are enabled */

  /* Optional */
  uint8_t log_mask_ext_valid;  /**< Must be set to true if log_mask_ext is being passed */
  sns_diag_mask_s_v01 log_mask_ext;
  /**<   mask for enabling logs with extended IDs (64 and beyond) */
}sns_diag_set_log_mask_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_messages
    @{
  */
/** Response Message; This command sets the log mask for sensors diag module on DSPS */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   response indicating whether the request suceeded or failed */
}sns_diag_set_log_mask_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_messages
    @{
  */
/** Request Message; This command sets the debug mask for sensors diag module on DSPS */
typedef struct {

  /* Mandatory */
  sns_diag_mask_s_v01 debug_mask;
  /**<   debug mask indicating which debug messages are enabled */
}sns_diag_set_debug_mask_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_messages
    @{
  */
/** Response Message; This command sets the debug mask for sensors diag module on DSPS */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   response indicating whether the request suceeded or failed */
}sns_diag_set_debug_mask_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_SNS_COMMON_CANCEL_V01
//#define REMOVE_SNS_COMMON_VERSION_V01
//#define REMOVE_SNS_DIAG_SET_DEBUG_MASK_V01
//#define REMOVE_SNS_DIAG_SET_DEBUG_OPTIONS_V01
//#define REMOVE_SNS_DIAG_SET_LOG_MASK_V01

/*Service Message Definition*/
/** @addtogroup SNS_DIAG_DSPS_SVC_qmi_msg_ids
    @{
  */
#define SNS_DIAG_DSPS_CANCEL_REQ_V01 0x0000
#define SNS_DIAG_DSPS_CANCEL_RESP_V01 0x0000
#define SNS_DIAG_DSPS_VERSION_REQ_V01 0x0001
#define SNS_DIAG_DSPS_VERSION_RESP_V01 0x0001
#define SNS_DIAG_SET_LOG_MASK_REQ_V01 0x0002
#define SNS_DIAG_SET_LOG_MASK_RESP_V01 0x0002
#define SNS_DIAG_SET_DEBUG_MASK_REQ_V01 0x0003
#define SNS_DIAG_SET_DEBUG_MASK_RESP_V01 0x0003
#define SNS_DIAG_SET_DEBUG_OPTIONS_REQ_V01 0x0004
#define SNS_DIAG_SET_DEBUG_OPTIONS_RESP_V01 0x0004
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_DIAG_DSPS_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_DIAG_DSPS_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define SNS_DIAG_DSPS_SVC_get_service_object_v01( ) \
          SNS_DIAG_DSPS_SVC_get_service_object_internal_v01( \
            SNS_DIAG_DSPS_SVC_V01_IDL_MAJOR_VERS, SNS_DIAG_DSPS_SVC_V01_IDL_MINOR_VERS, \
            SNS_DIAG_DSPS_SVC_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

