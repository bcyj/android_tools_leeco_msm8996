#ifndef SNS_SAM_QMD_SERVICE_01_H
#define SNS_SAM_QMD_SERVICE_01_H
/**
  @file sns_sam_qmd_v01.h

  @brief This is the public header file which defines the sns_sam_qmd service Data structures.

  This header file defines the types and structures that were defined in
  sns_sam_qmd. It contains the constant values defined, enums, structures,
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
   From IDL File: sns_sam_qmd_v01.idl */

/** @defgroup sns_sam_qmd_qmi_consts Constant values defined in the IDL */
/** @defgroup sns_sam_qmd_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sns_sam_qmd_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sns_sam_qmd_qmi_messages Structures sent as QMI messages */
/** @defgroup sns_sam_qmd_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sns_sam_qmd_qmi_accessor Accessor for QMI service object */
/** @defgroup sns_sam_qmd_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sns_sam_qmd_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_QMD_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_QMD_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_QMD_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */


/** @addtogroup sns_sam_qmd_qmi_consts 
    @{ 
  */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_MOTION_STATE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_MOTION_UNKNOWN_V01 = 0, 
  SNS_SAM_MOTION_REST_V01 = 1, 
  SNS_SAM_MOTION_MOVE_V01 = 2, 
  SNS_SAM_MOTION_STATE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_motion_state_e_v01;
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_aggregates
    @{
  */
/**  ENUMs are for reference only, uint8 is used instead 
 */
typedef struct {

  uint32_t var_thresh;
  /**<   Accel Variance thresold, we declare motion if the sum of variances on all axes exceed this , Q16, in m^2/s^4*/

  uint32_t var_win_len;
  /**<   time period over which the variance is computed, in Q16 seconds */
}sns_qmd_config_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Request Message; This command enables a QMD algorithm. */
typedef struct {

  /* Mandatory */
  uint32_t report_period;
  /**<   Unit of seconds, Q16; value of 0 means reporting on new event only  */

  /* Optional */
  uint8_t config_valid;  /**< Must be set to true if config is being passed */
  sns_qmd_config_s_v01 config;
  /**<   configuration options for this instance of the QMD algorithm, some 
  instances of QMD algorithms may not support this configuration parameters. 
  In such cases, their behaviour shall default to the behavious of the 
  algorithm is this optional parameter was omitted from the request */

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type                  = SNS_PROC_APPS
       notify_suspend->send_indications_during_suspend  = FALSE

       This field does not have any bearing on error indication
       messages, which will be sent even during suspend.
    */
}sns_sam_qmd_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Response Message; This command enables a QMD algorithm. */
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
}sns_sam_qmd_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Request Message; This command disables a QMD algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_qmd_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Response Message; This command disables a QMD algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_qmd_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Indication Message; Report containing QMD algorithm output */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of input used to generate the algorithm output  */

  /* Mandatory */
  sns_sam_motion_state_e_v01 state;
  /**<   Motion state output of QMD algorithm instance.  */
}sns_sam_qmd_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output of QMD algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_qmd_get_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output of QMD algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of input used to generate the algorithm output  */

  /* Mandatory */
  sns_sam_motion_state_e_v01 state;
  /**<   Motion state output of QMD algorithm instance.  */
}sns_sam_qmd_get_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_sam_qmd_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error indication for a QMD algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   Sensor1 error code.  */

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_qmd_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object sns_sam_qmd_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

