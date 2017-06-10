#ifndef SNS_DEBUG_SVC_SERVICE_H
#define SNS_DEBUG_SVC_SERVICE_H
/**
  @file sns_debug_interface_v01.h
  
  @brief This is the public header file which defines the SNS_DEBUG_SVC service Data structures.

  This header file defines the types and structures that were defined in 
  SNS_DEBUG_SVC. It contains the constant values defined, enums, structures,
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
   From IDL File: sns_debug_interface_v01.idl */

/** @defgroup SNS_DEBUG_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_DEBUG_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_DEBUG_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_DEBUG_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_DEBUG_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_DEBUG_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_DEBUG_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_DEBUG_SVC_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SNS_DEBUG_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_DEBUG_SVC_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_DEBUG_SVC_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SNS_DEBUG_SVC_V01_MAX_MESSAGE_ID 0x0003;
/** 
    @} 
  */


/** @addtogroup SNS_DEBUG_SVC_qmi_consts 
    @{ 
  */

/**   maximum number of characters in the name of a file  */
#define SNS_DEBUG_MAX_FILENAME_SIZE_V01 30

/**   maximum size (uint32) in a log packet, therefore max number of bytes supported is 600 bytes  */
#define SNS_DEBUG_MAX_LOG_SIZE_V01 150

/**   maximum number of parameters allowed for a debug string  */
#define SNS_DEBUG_NUM_PARAMS_ALLWD_V01 3
/**
    @}
  */

/** @addtogroup SNS_DEBUG_SVC_qmi_messages
    @{
  */
/**  Message; This command is used to print a debug string. */
typedef struct {

  /* Mandatory */
  uint16_t string_identifier;
  /**<   String ID */

  /* Mandatory */
  uint8_t module_id;
  /**<   Module Identifier */

  /* Mandatory */
  uint8_t str_priority;
  /**<   Priority */

  /* Mandatory */
  uint32_t param_values_len;  /**< Must be set to # of elements in param_values */
  intptr_t param_values[SNS_DEBUG_NUM_PARAMS_ALLWD_V01];
  /**<   Parameter values */

  /* Mandatory */
  uint32_t line_number;
  /**<   Line number */

  /* Mandatory */
  uint32_t file_name_len;  /**< Must be set to # of elements in file_name */
  char file_name[SNS_DEBUG_MAX_FILENAME_SIZE_V01];
  /**<   Filename */
}sns_debug_string_id_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_DEBUG_SVC_qmi_messages
    @{
  */
/**  Message; This command is used to send a log packet. */
typedef struct {

  /* Mandatory */
  uint16_t log_pkt_type;
  /**<   Log Packet Type */

  /* Mandatory */
  uint16_t logpkt_size;
  /**<   Size of log packet */

  /* Mandatory */
  uint32_t log_pkt_contents_len;  /**< Must be set to # of elements in log_pkt_contents */
  uint32_t log_pkt_contents[SNS_DEBUG_MAX_LOG_SIZE_V01];
  /**<   Log Packet contents */
}sns_debug_log_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_DEBUG_SVC_qmi_msg_ids
    @{
  */
#define SNS_DEBUG_CANCEL_REQ_V01 0x0000
#define SNS_DEBUG_CANCEL_RESP_V01 0x0000
#define SNS_DEBUG_VERSION_REQ_V01 0x0001
#define SNS_DEBUG_VERSION_RESP_V01 0x0001
#define SNS_DEBUG_STRING_ID_IND_V01 0x0002
#define SNS_DEBUG_LOG_IND_V01 0x0003
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_DEBUG_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_DEBUG_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_DEBUG_SVC_get_service_object_v01( ) \
          SNS_DEBUG_SVC_get_service_object_internal_v01( \
            SNS_DEBUG_SVC_V01_IDL_MAJOR_VERS, SNS_DEBUG_SVC_V01_IDL_MINOR_VERS, \
            SNS_DEBUG_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

