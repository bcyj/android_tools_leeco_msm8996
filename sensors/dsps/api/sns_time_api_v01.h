#ifndef SNS_TIME_SVC_SERVICE_H
#define SNS_TIME_SVC_SERVICE_H
/**
  @file sns_time_api_v01.h
  
  @brief This is the public header file which defines the SNS_TIME_SVC service Data structures.

  This header file defines the types and structures that were defined in 
  SNS_TIME_SVC. It contains the constant values defined, enums, structures,
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

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5 
   It requires encode/decode library version 4 or later
   It was generated on: Fri Sep  7 2012
   From IDL File: sns_time_api_v01.idl */

/** @defgroup SNS_TIME_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_TIME_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_TIME_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_TIME_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_TIME_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_TIME_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_TIME_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_TIME_SVC_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SNS_TIME_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_TIME_SVC_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_TIME_SVC_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SNS_TIME_SVC_V01_MAX_MESSAGE_ID 0x0000;
/** 
    @} 
  */


/** @addtogroup SNS_TIME_SVC_qmi_consts 
    @{ 
  */
/**
    @}
  */

/** @addtogroup SNS_TIME_SVC_qmi_enums
    @{
  */
typedef enum {
  SENSOR_TIME_RESULTS_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SENSOR_TIME_ESUCCESS_V01 = 0, 
  SENSOR_TIME_EINTERNAL_V01 = -1, 
  SENSOR_TIME_EINIT_V01 = -2, 
  SENSOR_TIME_EAPPS_V01 = -3, 
  SENSOR_TIME_EDSPS_V01 = -4, 
  SENSOR_TIME_RESULTS_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sensor_time_results_e_v01;
/**
    @}
  */

/*
 * sns_time_timestamp_req_msg is empty
 * typedef struct {
 * }sns_time_timestamp_req_msg_v01;
 */

/** @addtogroup SNS_TIME_SVC_qmi_messages
    @{
  */
/** Response Message; This command generates equivalent timestamps for multiple sources. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t timestamp_dsps_valid;  /**< Must be set to true if timestamp_dsps is being passed */
  uint32_t timestamp_dsps;
  /**<   Timestamp from the DSPS in clock ticks */

  /* Optional */
  uint8_t timestamp_apps_valid;  /**< Must be set to true if timestamp_apps is being passed */
  uint64_t timestamp_apps;
  /**<   Timestamp from the Apps processor in nanoseconds since last epoch */

  /* Optional */
  uint8_t error_code_valid;  /**< Must be set to true if error_code is being passed */
  sensor_time_results_e_v01 error_code;
}sns_time_timestamp_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_TIME_SVC_qmi_msg_ids
    @{
  */
#define SNS_TIME_TIMESTAMP_REQ_V01 0x0000
#define SNS_TIME_TIMESTAMP_RESP_V01 0x0000
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_TIME_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_TIME_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_TIME_SVC_get_service_object_v01( ) \
          SNS_TIME_SVC_get_service_object_internal_v01( \
            SNS_TIME_SVC_V01_IDL_MAJOR_VERS, SNS_TIME_SVC_V01_IDL_MINOR_VERS, \
            SNS_TIME_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

