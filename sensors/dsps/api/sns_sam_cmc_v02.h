#ifndef SNS_SAM_CMC_SVC_SERVICE_02_H
#define SNS_SAM_CMC_SVC_SERVICE_02_H
/**
  @file sns_sam_cmc_v02.h

  @brief This is the public header file which defines the SNS_SAM_CMC_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_CMC_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_cmc_v02.idl */

/** @defgroup SNS_SAM_CMC_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_CMC_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_CMC_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_CMC_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_CMC_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_CMC_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_CMC_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_CMC_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_CMC_SVC_V02_IDL_MAJOR_VERS 0x02
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_CMC_SVC_V02_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_CMC_SVC_V02_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_CMC_SVC_V02_MAX_MESSAGE_ID 0x0024
/**
    @}
  */


/** @addtogroup SNS_SAM_CMC_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_CMC_SUID_V02 0xb3cbd1d7df90cfa0

/**  Max number of reports in a batch indication -
     set based on max payload size supported by transport framework */
#define SNS_SAM_CMC_MAX_ITEMS_IN_BATCH_V02 123

/**  Max number of supported motion states */
#define SNS_SAM_CMC_MS_NUM_V02 9
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_CMC_MS_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_CMC_MS_UNKNOWN_V02 = 0, 
  SNS_SAM_CMC_MS_STATIONARY_V02 = 1, 
  SNS_SAM_CMC_MS_MOVE_V02 = 2, 
  SNS_SAM_CMC_MS_FIDDLE_V02 = 3, 
  SNS_SAM_CMC_MS_PEDESTRIAN_V02 = 4, 
  SNS_SAM_CMC_MS_VEHICLE_V02 = 5, 
  SNS_SAM_CMC_MS_WALK_V02 = 6, 
  SNS_SAM_CMC_MS_RUN_V02 = 7, 
  SNS_SAM_CMC_MS_BIKE_V02 = 8, 
  SNS_SAM_CMC_MS_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_cmc_ms_e_v02;
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_CMC_MS_EVENT_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_CMC_MS_EVENT_EXIT_V02 = 0, 
  SNS_SAM_CMC_MS_EVENT_ENTER_V02 = 1, 
  SNS_SAM_CMC_MS_EVENT_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_cmc_ms_event_e_v02;
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_CMC_MS_REPORT_TYPE_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_CMC_MS_REPORT_TYPE_ALL_V02 = 0, 
  SNS_SAM_CMC_MS_REPORT_TYPE_SINGLE_V02 = 1, 
  SNS_SAM_CMC_MS_REPORT_TYPE_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_cmc_ms_report_type_e_v02;
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_CMC_MS_EVENT_REPORT_TYPE_E_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_SAM_CMC_MS_EVENT_REPORT_TYPE_ENTER_V02 = 0, 
  SNS_SAM_CMC_MS_EVENT_REPORT_TYPE_EXIT_V02 = 1, 
  SNS_SAM_CMC_MS_EVENT_REPORT_TYPE_E_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_cmc_ms_event_report_type_e_v02;
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_aggregates
    @{
  */
typedef struct {

  sns_sam_cmc_ms_e_v02 motion_state;
  /**<  
    Detected motion state.
  */

  float motion_state_probability;
  /**<  
    Likelihood of the reported motion state in the range from 0 to 1.
  */
}sns_sam_cmc_report_data_s_v02;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables the cmc algorithm. */
typedef struct {

  /* Optional */
  uint8_t notify_suspend_valid;  /**< Must be set to true if notify_suspend is being passed */
  sns_suspend_notification_s_v01 notify_suspend;
  /**<   Identifies if indications for this request should be sent
       when the processor is in suspend state.

       If this field is not specified, default value will be set to
       notify_suspend->proc_type                  = SNS_PROC_MODEM
       notify_suspend->send_indications_during_suspend  = TRUE

       This field does not have any bearing on error indication
       messages, which will be sent even during suspend.
    */
}sns_sam_cmc_enable_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables the cmc algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<  
    Algorithm instance ID maintained/assigned by SAM.
    The client shall use this instance ID for future messages associated with
    current algorithm instance.
    */
}sns_sam_cmc_enable_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables the cmc algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   To identify the algorithm instance to be disabled.  */
}sns_sam_cmc_disable_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables the cmc algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_cmc_disable_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Request Message; This command fetches latest report output of cmc algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_cmc_get_report_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Response Message; This command fetches latest report output of cmc algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp_len;  /**< Must be set to # of elements in timestamp */
  uint32_t timestamp[SNS_SAM_CMC_MS_NUM_V02];
  /**<   variable length array of the same length as report_data, when defined.
  Represents the time instance in SSC clock ticks when SNS_SAM_CMC_MS_EVENT_ENTER
  was detected for the corresponding motion state in the report_data array.
  Unit is in SSC clock ticks*/

  /* Optional */
  uint8_t report_data_valid;  /**< Must be set to true if report_data is being passed */
  uint32_t report_data_len;  /**< Must be set to # of elements in report_data */
  sns_sam_cmc_report_data_s_v02 report_data[SNS_SAM_CMC_MS_NUM_V02];
  /**<   variable length array containing the list of active motion states, i.e, algos
  for which the last detected event is SNS_SAM_CMC_MS_EVENT_ENTER*/
}sns_sam_cmc_get_report_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Indication Message; Output report from the cmc algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of last sensor input sample with which motion state was computed.
  Unit is in SSC clock ticks*/

  /* Mandatory */
  sns_sam_cmc_report_data_s_v02 report_data;
  /**<   cmc algorithm output report  */

  /* Optional */
  uint8_t ms_event_valid;  /**< Must be set to true if ms_event is being passed */
  sns_sam_cmc_ms_event_e_v02 ms_event;
  /**<   cmc motion state event type.
  */
}sns_sam_cmc_report_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error report from the cmc algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint32_t timestamp;
  /**<   Timestamp of when the error was detected; in SSC clock ticks */

  /* Mandatory */
  uint8_t error;
  /**<   sensors error code */
}sns_sam_cmc_error_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Request Message; This command updates the reporting configuration of specified motion
    states */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  uint8_t enable;
  /**<   Whether the requested report type should be enabled or disabled.  */

  /* Mandatory */
  sns_sam_cmc_ms_report_type_e_v02 report_ms_type;
  /**<   Identifies if the reporting configuration is being altered for a
  specific motion state or all supported motion states.
  */

  /* Mandatory */
  sns_sam_cmc_ms_event_report_type_e_v02 report_event_type;
  /**<   Identifies the event types that must be reported
  */

  /* Mandatory */
  sns_sam_cmc_ms_e_v02 report_motion_state;
  /**<   The specific motion state for which reporting configuration is being
  altered in the case where report_ms_type is set to SNS_SAM_CMC_MS_REPORT_TYPE_SINGLE.
  */
}sns_sam_cmc_update_reporting_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Response Message; This command updates the reporting configuration of specified motion
    states */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */
}sns_sam_cmc_update_reporting_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Request Message; This command handles batch mode for the cmc algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance.  */

  /* Mandatory */
  int32_t batch_period;
  /**<   Specifies the interval over which reports are to be batched, in seconds;
       Q16 format. If AP is in suspend and notify_suspend is FALSE, undelivered
       reports will be batched in circular FIFO and delivered upon wakeup.
       P = 0 to disable batching.
       P > 0 to enable batching.
    */

  /* Optional */
  uint8_t req_type_valid;  /**< Must be set to true if req_type is being passed */
  sns_batch_req_type_e_v01 req_type;
  /**<   Field no longer supported in version 2 and later.  Functionality can be
       approximated using the notify_suspend flag in the ENABLE_REQ message.  Max
       buffer depth is returned in GET_ATTRIBUTES_RESP.

       Optional request type:
       0 – Do not wake client from suspend when buffer is full.
       1 – Wake client from suspend when buffer is full.
       2 – Use to get max buffer depth. Does not enable/disable batching.
           instance_id and batch_period are ignored for this request type.
       Defaults to 0.
    */
}sns_sam_cmc_batch_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Response Message; This command handles batch mode for the cmc algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Algorithm instance ID maintained/assigned by SAM */

  /* Optional */
  uint8_t max_batch_size_valid;  /**< Must be set to true if max_batch_size is being passed */
  uint32_t max_batch_size;
  /**<   Max supported batch size */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp when the batch request was processed in SSC ticks */
}sns_sam_cmc_batch_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t timestamp;
  /**<   Timestamp of input with which latest step in report was detected; in SSC ticks */

  sns_sam_cmc_report_data_s_v02 report;
  /**<   cmc algorithm output report */

  sns_sam_cmc_ms_event_e_v02 ms_event;
  /**<   cmc motion state event type. */
}sns_sam_cmc_batch_item_s_v02;  /* Type */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Indication Message; Report containing a batch of algorithm outputs */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance id identifies the algorithm instance */

  /* Mandatory */
  uint32_t items_len;  /**< Must be set to # of elements in items */
  sns_sam_cmc_batch_item_s_v02 items[SNS_SAM_CMC_MAX_ITEMS_IN_BATCH_V02];
  /**<   cmc algorithm output reports */

  /* Optional */
  uint8_t ind_type_valid;  /**< Must be set to true if ind_type is being passed */
  uint8_t ind_type;
  /**<   Optional batch indication type
       SNS_BATCH_ONLY_IND - Standalone batch indication. Not part of a back to back indication stream
       SNS_BATCH_FIRST_IND - First indication in stream of back to back indications
       SNS_BATCH_INTERMEDIATE_IND - Intermediate indication in stream of back to back indications
       SNS_BATCH_LAST_IND - Last indication in stream of back to back indications
    */
}sns_sam_cmc_batch_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Request Message; This command updates active batch period for a cmc algorithm
           when batching is ongoing. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   Instance ID identifies the algorithm instance.  */

  /* Mandatory */
  int32_t active_batch_period;
  /**<   Specifies new interval (in seconds, Q16) over which reports are to be
       batched when the client processor is awake. Only takes effect if
       batching is ongoing.
       P > 0 to to override active batch period set in batch enable request.
       P = 0 to switch to active batch period set in batch enable request.
    */
}sns_sam_cmc_update_batch_period_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_CMC_SVC_qmi_messages
    @{
  */
/** Response Message; This command updates active batch period for a cmc algorithm
           when batching is ongoing. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<   Algorithm instance ID maintained/assigned by SAM */

  /* Optional */
  uint8_t timestamp_valid;  /**< Must be set to true if timestamp is being passed */
  uint32_t timestamp;
  /**<   Timestamp when the batch request was processed in SSC ticks */
}sns_sam_cmc_update_batch_period_resp_msg_v02;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_SNS_SAM_CMC_BATCH_V02 
//#define REMOVE_SNS_SAM_CMC_BATCH_REPORT_V02 
//#define REMOVE_SNS_SAM_CMC_CANCEL_V02 
//#define REMOVE_SNS_SAM_CMC_DISABLE_V02 
//#define REMOVE_SNS_SAM_CMC_ENABLE_V02 
//#define REMOVE_SNS_SAM_CMC_ERROR_V02 
//#define REMOVE_SNS_SAM_CMC_GET_REPORT_V02 
//#define REMOVE_SNS_SAM_CMC_REPORT_V02 
//#define REMOVE_SNS_SAM_CMC_UPDATE_BATCH_PERIOD_V02 
//#define REMOVE_SNS_SAM_CMC_UPDATE_REPORTING_V02 
//#define REMOVE_SNS_SAM_CMC_VERSION_V02 
//#define REMOVE_SNS_SAM_GET_ALGO_ATTRIBUTES_V02 

/*Service Message Definition*/
/** @addtogroup SNS_SAM_CMC_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_CMC_CANCEL_REQ_V02 0x0000
#define SNS_SAM_CMC_CANCEL_RESP_V02 0x0000
#define SNS_SAM_CMC_VERSION_REQ_V02 0x0001
#define SNS_SAM_CMC_VERSION_RESP_V02 0x0001
#define SNS_SAM_CMC_ENABLE_REQ_V02 0x0002
#define SNS_SAM_CMC_ENABLE_RESP_V02 0x0002
#define SNS_SAM_CMC_DISABLE_REQ_V02 0x0003
#define SNS_SAM_CMC_DISABLE_RESP_V02 0x0003
#define SNS_SAM_CMC_GET_REPORT_REQ_V02 0x0004
#define SNS_SAM_CMC_GET_REPORT_RESP_V02 0x0004
#define SNS_SAM_CMC_REPORT_IND_V02 0x0005
#define SNS_SAM_CMC_ERROR_IND_V02 0x0006
#define SNS_SAM_CMC_UPDATE_REPORTING_REQ_V02 0x0020
#define SNS_SAM_CMC_UPDATE_REPORTING_RESP_V02 0x0020
#define SNS_SAM_CMC_BATCH_REQ_V02 0x0021
#define SNS_SAM_CMC_BATCH_RESP_V02 0x0021
#define SNS_SAM_CMC_BATCH_IND_V02 0x0022
#define SNS_SAM_CMC_UPDATE_BATCH_PERIOD_REQ_V02 0x0023
#define SNS_SAM_CMC_UPDATE_BATCH_PERIOD_RESP_V02 0x0023
#define SNS_SAM_CMC_GET_ATTRIBUTES_REQ_V02 0x0024
#define SNS_SAM_CMC_GET_ATTRIBUTES_RESP_V02 0x0024
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_CMC_SVC_get_service_object_v02( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_CMC_SVC_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_CMC_SVC_get_service_object_v02( ) \
          SNS_SAM_CMC_SVC_get_service_object_internal_v02( \
            SNS_SAM_CMC_SVC_V02_IDL_MAJOR_VERS, SNS_SAM_CMC_SVC_V02_IDL_MINOR_VERS, \
            SNS_SAM_CMC_SVC_V02_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

