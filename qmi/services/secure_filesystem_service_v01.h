#ifndef SFS_SERVICE_01_H
#define SFS_SERVICE_01_H
/**
  @file secure_filesystem_service_v01.h

  @brief This is the public header file which defines the sfs service Data structures.

  This header file defines the types and structures that were defined in
  sfs. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2014 Qualcomm Technologies, Inc. All rights reserved.
Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.2 
   It was generated on: Thu Nov 13 2014 (Spin 0)
   From IDL File: secure_filesystem_service_v01.idl */

/** @defgroup sfs_qmi_consts Constant values defined in the IDL */
/** @defgroup sfs_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sfs_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sfs_qmi_messages Structures sent as QMI messages */
/** @defgroup sfs_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sfs_qmi_accessor Accessor for QMI service object */
/** @defgroup sfs_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sfs_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SFS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SFS_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SFS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SFS_V01_MAX_MESSAGE_ID 0x0021
/**
    @}
  */


/** @addtogroup sfs_qmi_consts 
    @{ 
  */
#define QMI_SFS_TZ_APP_STATUS_SIZE_V01 4
#define QMI_SFS_TZ_APP_CMD_ID_SIZE_V01 4
#define QMI_SFS_TZ_APP_PARTITION_SIZE_V01 512
#define QMI_SFS_SEC_MSG_PAYLOAD_LEN_SIZE_V01 4
#define QMI_SFS_SEC_MSG_HEADER_SIZE_V01 100
#define QMI_SFS_TZ_APP_PAYLOAD_SIZE_V01 4
#define QMI_SFS_TZ_APP_WRITE_REQ_MSG_SIZE_V01 616
#define QMI_SFS_TZ_APP_WRITE_RSP_MSG_SIZE_V01 108
#define QMI_SFS_TZ_APP_READ_REQ_MSG_SIZE_V01 104
#define QMI_SFS_TZ_APP_READ_RSP_MSG_SIZE_V01 620
/**
    @}
  */

/** @addtogroup sfs_qmi_messages
    @{
  */
/** Request Message; Sends a write request to the secure file system Trustzone secure app
           given the confidential information provided by caller. */
typedef struct {

  /* Mandatory */
  /*  Confidential Payload Size */
  uint32_t payload_size;

  /* Mandatory */
  /*  Confidential Payload Containing Write Request from Trustzone Secure App */
  uint8_t sec_write_msg[QMI_SFS_TZ_APP_WRITE_REQ_MSG_SIZE_V01];
}qmi_sfs_tz_app_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sfs_qmi_messages
    @{
  */
/** Response Message; Sends a write request to the secure file system Trustzone secure app
           given the confidential information provided by caller. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Confidential Payload Size */
  uint8_t payload_size_valid;  /**< Must be set to true if payload_size is being passed */
  uint32_t payload_size;

  /* Optional */
  /*  Confidential Payload Containing Write Request from Trustzone Secure App */
  uint8_t sec_write_msg_valid;  /**< Must be set to true if sec_write_msg is being passed */
  uint8_t sec_write_msg[QMI_SFS_TZ_APP_WRITE_RSP_MSG_SIZE_V01];
}qmi_sfs_tz_app_write_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sfs_qmi_messages
    @{
  */
/** Request Message; Sends a read request to the secure file system Trustzone secure app
           given the confidential information provided by caller. */
typedef struct {

  /* Mandatory */
  /*  Confidential Payload Size */
  uint32_t payload_size;

  /* Mandatory */
  /*  Confidential Payload Containing Write Request from Trustzone Secure App */
  uint8_t sec_read_msg[QMI_SFS_TZ_APP_READ_REQ_MSG_SIZE_V01];
}qmi_sfs_tz_app_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sfs_qmi_messages
    @{
  */
/** Response Message; Sends a read request to the secure file system Trustzone secure app
           given the confidential information provided by caller. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Confidential Payload Size */
  uint8_t payload_size_valid;  /**< Must be set to true if payload_size is being passed */
  uint32_t payload_size;

  /* Optional */
  /*  Confidential Payload Containing Write Request from Trustzone Secure App */
  uint8_t sec_read_msg_valid;  /**< Must be set to true if sec_read_msg is being passed */
  uint8_t sec_read_msg[QMI_SFS_TZ_APP_READ_RSP_MSG_SIZE_V01];
}qmi_sfs_tz_app_read_rsp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_SFS_READ_CMD_TO_TZ_APP_V01 
//#define REMOVE_QMI_SFS_WRITE_CMD_TO_TZ_APP_V01 

/*Service Message Definition*/
/** @addtogroup sfs_qmi_msg_ids
    @{
  */
#define QMI_SFS_TZ_APP_WRITE_REQ_V01 0x0020
#define QMI_SFS_TZ_APP_WRITE_RSP_V01 0x0020
#define QMI_SFS_TZ_APP_READ_REQ_V01 0x0021
#define QMI_SFS_TZ_APP_READ_RSP_V01 0x0021
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sfs_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sfs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define sfs_get_service_object_v01( ) \
          sfs_get_service_object_internal_v01( \
            SFS_V01_IDL_MAJOR_VERS, SFS_V01_IDL_MINOR_VERS, \
            SFS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

