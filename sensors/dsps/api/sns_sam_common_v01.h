#ifndef SNS_SAM_COMMON_SERVICE_01_H
#define SNS_SAM_COMMON_SERVICE_01_H
/**
  @file sns_sam_common_v01.h

  @brief This is the public header file which defines the sns_sam_common service Data structures.

  This header file defines the types and structures that were defined in
  sns_sam_common. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_common_v01.idl */

/** @defgroup sns_sam_common_qmi_consts Constant values defined in the IDL */
/** @defgroup sns_sam_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sns_sam_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sns_sam_common_qmi_messages Structures sent as QMI messages */
/** @defgroup sns_sam_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sns_sam_common_qmi_accessor Accessor for QMI service object */
/** @defgroup sns_sam_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sns_sam_common_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_COMMON_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_COMMON_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */


/** @addtogroup sns_sam_common_qmi_consts 
    @{ 
  */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sns_sam_get_algo_attrib_req_msg_v01;

/** @addtogroup sns_sam_common_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_REPORT_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_PERIODIC_REPORT_V01 = 0x01, /**<  Can report at client specified report rate  */
  SNS_SAM_ASYNC_REPORT_V01 = 0x02, /**<  Can report on every event  */
  SNS_SAM_SYNC_REPORT_V01 = 0x04, /**<  Can report at sample rate  */
  SNS_SAM_ONE_SHOT_REPORT_V01 = 0x08, /**<  Can report at event and deactivate itself  */
  SNS_SAM_REPORT_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_report_e_v01;
/**
    @}
  */

/** @addtogroup sns_sam_common_qmi_messages
    @{
  */
/** Response Message; This command requests the attributes of the algorithm service

    This command shall only be used by algorithm services. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint32_t algorithm_revision;
  /**<   Revision number of the algorithm implemented by the service */

  /* Mandatory */
  sns_proc_type_e_v01 proc_type;
  /**<   ID of the processor on which the algorithm is running */

  /* Mandatory */
  uint32_t supported_reporting_modes;
  /**<   Bitmask of all reporting modes supported by algorithm.
       See sns_sam_report_e for reporting options */

  /* Mandatory */
  int32_t min_report_rate;
  /**<   Minimum report rate supported by algorithm (in Hz, Q16) */

  /* Mandatory */
  int32_t max_report_rate;
  /**<   Maximum report rate supported by algorithm (in Hz, Q16) */

  /* Mandatory */
  int32_t min_sample_rate;
  /**<   Minimum sample rate supported by algorithm (in Hz, Q16) */

  /* Mandatory */
  int32_t max_sample_rate;
  /**<   Maximum sample rate supported by algorithm (in Hz, Q16) */

  /* Mandatory */
  uint32_t max_batch_size;
  /**<   The maximum batch size (in reports) supported by this service,
       as if it were the only service active on the system.
       Will never return more than this many samples in one series of
       batched indications.
       Returns 0, if batching is not supported */

  /* Mandatory */
  int32_t power;
  /**<   Power estimate for algorithm (in mA, Q16) */

  /* Optional */
  uint8_t sensorUID_valid;  /**< Must be set to true if sensorUID is being passed */
  uint64_t sensorUID;
  /**<   SSC Unique identifier for this sensor */

  /* Optional */
  uint8_t reserved_batch_size_valid;  /**< Must be set to true if reserved_batch_size is being passed */
  uint32_t reserved_batch_size;
  /**<   The mimimum guaranteed batch size for this service.  Shared amongst
       all clients. If max_batch_size is '0', this field shall be ignored. */
}sns_sam_get_algo_attrib_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object sns_sam_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

