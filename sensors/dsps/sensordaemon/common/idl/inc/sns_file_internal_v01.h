#ifndef SNS_FILE_INTERNAL_SVC_SERVICE_H
#define SNS_FILE_INTERNAL_SVC_SERVICE_H
/**
  @file sns_file_internal_v01.h
  
  @brief This is the public header file which defines the SNS_FILE_INTERNAL_SVC service Data structures.

  This header file defines the types and structures that were defined in 
  SNS_FILE_INTERNAL_SVC. It contains the constant values defined, enums, structures,
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
   From IDL File: sns_file_internal_v01.idl */

/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_FILE_INTERNAL_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SNS_FILE_INTERNAL_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_FILE_INTERNAL_SVC_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_FILE_INTERNAL_SVC_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define SNS_FILE_INTERNAL_SVC_V01_MAX_MESSAGE_ID 0x0004;
/** 
    @} 
  */


/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_consts 
    @{ 
  */

/**   maximum number of characters in the name of a file to be opened/created  */
#define SNS_FILE_MAX_FILENAME_SIZE_V01 512

/**   maximum number of characters that may be included in the 'mode' field  */
#define SNS_FILE_MAX_MODE_SIZE_V01 4

/**   maximum size (in bytes) that may be written through a single write request  */
#define SNS_FILE_MAX_BUF_SIZE_V01 512
/**
    @}
  */

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command opens a file file */
typedef struct {

  /* Mandatory */
  uint32_t path_name_len;  /**< Must be set to # of elements in path_name */
  char path_name[SNS_FILE_MAX_FILENAME_SIZE_V01];
  /**<   Path to the file to be created/opened.  See ANSI C fopen.  Only accepts
       alphanumeric characters and forward slashes.  Will not create new
       directories. */

  /* Mandatory */
  uint32_t mode_len;  /**< Must be set to # of elements in mode */
  char mode[SNS_FILE_MAX_MODE_SIZE_V01];
  /**<   Mode with which to open the file.  See ANSI C fopen. */
}sns_file_open_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command opens a file file */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t fildes_valid;  /**< Must be set to true if fildes is being passed */
  int64_t fildes;
  /**<   File descriptor of opened file */
}sns_file_open_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command writes to a file file at the current file offset,
      per ANSI C fwrite. */
typedef struct {

  /* Mandatory */
  int64_t fildes;
  /**<   File descriptor, as returned in sns_file_open_resp_msg */

  /* Mandatory */
  uint32_t buf_len;  /**< Must be set to # of elements in buf */
  uint8_t buf[SNS_FILE_MAX_BUF_SIZE_V01];
  /**<   Data to write */
}sns_file_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command writes to a file file at the current file offset,
      per ANSI C fwrite. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t bytes_written_valid;  /**< Must be set to true if bytes_written is being passed */
  uint32_t bytes_written;
  /**<   Number of bytes written by the write operation */
}sns_file_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command closes a file file, and flushes buffers to disk
      per ANSI C fclose. */
typedef struct {

  /* Mandatory */
  int64_t fildes;
  /**<   File descriptor, as returned in sns_file_open_resp_msg */
}sns_file_close_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command closes a file file, and flushes buffers to disk
      per ANSI C fclose. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
}sns_file_close_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_FILE_INTERNAL_SVC_qmi_msg_ids
    @{
  */
#define SNS_FILE_INTERNAL_CANCEL_REQ_V01 0x0000
#define SNS_FILE_INTERNAL_CANCEL_RESP_V01 0x0000
#define SNS_FILE_INTERNAL_VERSION_REQ_V01 0x0001
#define SNS_FILE_INTERNAL_VERSION_RESP_V01 0x0001
#define SNS_FILE_INTERNAL_OPEN_REQ_V01 0x0002
#define SNS_FILE_INTERNAL_OPEN_RESP_V01 0x0002
#define SNS_FILE_INTERNAL_WRITE_REQ_V01 0x0003
#define SNS_FILE_INTERNAL_WRITE_RESP_V01 0x0003
#define SNS_FILE_INTERNAL_CLOSE_REQ_V01 0x0004
#define SNS_FILE_INTERNAL_CLOSE_RESP_V01 0x0004
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_FILE_INTERNAL_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_FILE_INTERNAL_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_FILE_INTERNAL_SVC_get_service_object_v01( ) \
          SNS_FILE_INTERNAL_SVC_get_service_object_internal_v01( \
            SNS_FILE_INTERNAL_SVC_V01_IDL_MAJOR_VERS, SNS_FILE_INTERNAL_SVC_V01_IDL_MINOR_VERS, \
            SNS_FILE_INTERNAL_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

