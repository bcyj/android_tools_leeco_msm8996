#ifndef SNS_SAM_QUAT_SVC_SERVICE_01_H
#define SNS_SAM_QUAT_SVC_SERVICE_01_H
/**
  @file sns_sam_quat_v01.h

  @brief This is the public header file which defines the SNS_SAM_QUAT_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_QUAT_SVC. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6 
   It was generated on: Thu Feb 27 2014 (Spin 0)
   From IDL File: sns_sam_quat_v01.idl */

/** @defgroup SNS_SAM_QUAT_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_QUAT_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_QUAT_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_QUAT_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_QUAT_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_QUAT_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_QUAT_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_QUAT_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_QUAT_SVC_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_QUAT_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_QUAT_SVC_V01_MAX_MESSAGE_ID 0x0006
/**
    @}
  */


/** @addtogroup SNS_SAM_QUAT_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_QUAT_SUID_V01 0x7ed433a8f46f5e01
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_aggregates
    @{
  */
typedef struct {

  int32_t q[4];
  /**<   quaternion output in Q16 format */
}sns_sam_quat_result_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  uint32_t report_period;
  /**<   unit of seconds, Q16; 0 = to report at sampling rate */

  /* Optional */
  uint8_t sample_rate_valid;  /**< Must be set to true if sample_rate is being passed */
  uint32_t sample_rate;
  /**<   sample rate in Hz, Q16 ; should be greater than report rate */
}sns_sam_quat_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint8_t instance_id;
  /**<  
    Instance ID is assigned by SAM.
    The client shall use this instance ID for future messages associated with
    this algorithm instance.
  */
}sns_sam_quat_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */
}sns_sam_quat_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */
}sns_sam_quat_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Indication Message; Report containing Quaternion algorithm output */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of input used to generate the algorithm output */

  /* Mandatory */
  sns_sam_quat_result_s_v01 result;
  /**<   Output of the Quaternion algorithm instance */
}sns_sam_quat_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output of a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */
}sns_sam_quat_get_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output of a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of input used to generate the algorithm output */

  /* Mandatory */
  sns_sam_quat_result_s_v01 result;
  /**<   Output of the Quaternion algorithm instance */
}sns_sam_quat_get_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_QUAT_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error indication for a Quaternion algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   Sensor1 error code */

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance. */
}sns_sam_quat_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_SAM_QUAT_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_QUAT_CANCEL_REQ_V01 0x0000
#define SNS_SAM_QUAT_CANCEL_RESP_V01 0x0000
#define SNS_SAM_QUAT_VERSION_REQ_V01 0x0001
#define SNS_SAM_QUAT_VERSION_RESP_V01 0x0001
#define SNS_SAM_QUAT_ENABLE_REQ_V01 0x0002
#define SNS_SAM_QUAT_ENABLE_RESP_V01 0x0002
#define SNS_SAM_QUAT_DISABLE_REQ_V01 0x0003
#define SNS_SAM_QUAT_DISABLE_RESP_V01 0x0003
#define SNS_SAM_QUAT_GET_REPORT_REQ_V01 0x0004
#define SNS_SAM_QUAT_GET_REPORT_RESP_V01 0x0004
#define SNS_SAM_QUAT_REPORT_IND_V01 0x0005
#define SNS_SAM_QUAT_ERROR_IND_V01 0x0006
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_QUAT_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_QUAT_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_QUAT_SVC_get_service_object_v01( ) \
          SNS_SAM_QUAT_SVC_get_service_object_internal_v01( \
            SNS_SAM_QUAT_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_QUAT_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_QUAT_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

