#ifndef SNS_SAM_PAM_SVC_SERVICE_01_H
#define SNS_SAM_PAM_SVC_SERVICE_01_H
/**
  @file sns_sam_pam_v01.h

  @brief This is the public header file which defines the SNS_SAM_PAM_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_PAM_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Thu Aug 21 2014 (Spin 0)
   From IDL File: sns_sam_pam_v01.idl */

/** @defgroup SNS_SAM_PAM_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_PAM_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_PAM_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_PAM_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_PAM_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_PAM_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_PAM_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_PAM_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_PAM_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_PAM_SVC_V01_IDL_MINOR_VERS 0x05
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_PAM_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_PAM_SVC_V01_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_PAM_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_PAM_SUID_V01 0x866319e2cab14f1f
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_PAM_DEV_STATE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_PAM_DEV_STATE_UNKNOWN_V01 = -2, /**<  Initial state.  */
  SNS_SAM_PAM_DEV_STATE_STATIONARY_V01 = -1, /**<  Device is in a Stationary state.  */
  SNS_SAM_PAM_DEV_STATE_MOTION_V01 = 0, /**<  Device is in motion with 0 step count.  */
  SNS_SAM_PAM_DEV_STATE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_pam_dev_state_e_v01;
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint32_t measurement_period;
  /**<   Measurement period in seconds over which the pedometer counts steps.
  */

  /* Mandatory */
  uint32_t step_count_threshold;
  /**<   Step count threshold.
       Send a new report to the client if the difference between the measured
       step count and the last reported step count is more than this threshold.
       The client can request a periodic report every measurement period by
       setting this threshold to 0.
  */

  /* Optional */
  uint8_t sample_rate_valid;  /**< Must be set to true if sample_rate is being passed */
  uint32_t sample_rate;
  /**<   Sample rate in Hz; Q16 format. */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies whether indications for this request are to be sent
       when the processor is in the Suspend state.

       If this field is not specified, the default value is set to:
       notify_suspend->proc_type                  = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend  = FALSE

       This field does not have any bearing on error indication
       messages, which are sent even during Suspend.
    */
}sns_sam_pam_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance ID, which is assigned by the SAM.
       The client must use this instance ID for future messages associated with
       the current algorithm instance.
  */
}sns_sam_pam_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_pam_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_pam_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Indication Message; Output report from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of the last sensor sample used to compute this result,
       in SSC ticks.
  */

  /* Mandatory */
  int32_t step_count;
  /**<   Step count measured in the latest measurement period, or
       the current device state (sns_sam_pam_dev_state_e).
  */
}sns_sam_pam_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_pam_get_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp of the input that caused this indication, in SSC ticks. */

  /* Optional */
  uint8_t step_count_valid;  /**< Must be set to true if step_count is being passed */
  int32_t step_count;
  /**<   Step count measured in the latest measurement period, or
       the current device state (sns_sam_pam_dev_state_e).
  */
}sns_sam_pam_get_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_PAM_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error report from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   Sensors error code. */

  /* Mandatory */
  uint8_t instance_id;
  /**<   Identifies the algorithm instance. */
}sns_sam_pam_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V01 
//#define REMOVE_SNS_SAM_PAM_CANCEL_V01 
//#define REMOVE_SNS_SAM_PAM_DISABLE_V01 
//#define REMOVE_SNS_SAM_PAM_ENABLE_V01 
//#define REMOVE_SNS_SAM_PAM_ERROR_V01 
//#define REMOVE_SNS_SAM_PAM_GET_REPORT_V01 
//#define REMOVE_SNS_SAM_PAM_REPORT_V01 
//#define REMOVE_SNS_SAM_PAM_VERSION_V01 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_PAM_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_PAM_CANCEL_REQ_V01 0x0000
#define SNS_SAM_PAM_CANCEL_RESP_V01 0x0000
#define SNS_SAM_PAM_VERSION_REQ_V01 0x0001
#define SNS_SAM_PAM_VERSION_RESP_V01 0x0001
#define SNS_SAM_PAM_ENABLE_REQ_V01 0x0002
#define SNS_SAM_PAM_ENABLE_RESP_V01 0x0002
#define SNS_SAM_PAM_DISABLE_REQ_V01 0x0003
#define SNS_SAM_PAM_DISABLE_RESP_V01 0x0003
#define SNS_SAM_PAM_GET_REPORT_REQ_V01 0x0004
#define SNS_SAM_PAM_GET_REPORT_RESP_V01 0x0004
#define SNS_SAM_PAM_REPORT_IND_V01 0x0005
#define SNS_SAM_PAM_ERROR_IND_V01 0x0006
#define SNS_SAM_PAM_GET_ATTRIBUTES_REQ_V01 0x0024
#define SNS_SAM_PAM_GET_ATTRIBUTES_RESP_V01 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_PAM_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_PAM_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_PAM_SVC_get_service_object_v01( ) \
          SNS_SAM_PAM_SVC_get_service_object_internal_v01( \
            SNS_SAM_PAM_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_PAM_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_PAM_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

