#ifndef SNS_COMMON_SERVICE_01_H
#define SNS_COMMON_SERVICE_01_H
/**
  @file sns_common_v01.h

  @brief This is the public header file which defines the sns_common service Data structures.

  This header file defines the types and structures that were defined in
  sns_common. It contains the constant values defined, enums, structures,
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
   It was generated on: Thu Sep  4 2014 (Spin 0)
   From IDL File: sns_common_v01.idl */

/** @defgroup sns_common_qmi_consts Constant values defined in the IDL */
/** @defgroup sns_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sns_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sns_common_qmi_messages Structures sent as QMI messages */
/** @defgroup sns_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sns_common_qmi_accessor Accessor for QMI service object */
/** @defgroup sns_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sns_common_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_COMMON_V01_IDL_MINOR_VERS 0x0B
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_COMMON_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */


/** @addtogroup sns_common_qmi_consts 
    @{ 
  */

/** 
============================================================================
 Sensor Service IDs
============================================================================

 Sensor manager.  */
#define SNS_SMGR_SVC_ID_V01 0

/**  Power manager.  */
#define SNS_PM_SVC_ID_V01 1

/**  Sensor message router on the DSPS.  */
#define SNS_SMR_DSPS_SVC_ID_V01 2

/**  Sensor registry -- legacy.  */
#define SNS_REG_SVC_ID_V01 3

/**  Algorithm: Absolute motion detection.  */
#define SNS_SAM_AMD_SVC_ID_V01 4

/**  Algorithm: Relative motion detection.  */
#define SNS_SAM_RMD_SVC_ID_V01 5

/**  Algorithm: Vehicle motion detection.  */
#define SNS_SAM_VMD_SVC_ID_V01 6

/**  Sensors debug interface on the DSPS.  */
#define SNS_DEBUG_SVC_ID_V01 7

/**  Sensors diagnostics interface on the DSPS.  */
#define SNS_DIAG_DSPS_SVC_ID_V01 8

/**  Face and Shake service on the applications processor.  */
#define SNS_SAM_FNS_SVC_ID_V01 9

/**  Algorithm: Bring to ear.  */
#define SNS_SAM_BTE_SVC_ID_V01 10

/**  Algorithm: Quaternion.  */
#define SNS_SAM_QUAT_SVC_ID_V01 11

/**  Algorithm: Gravity.  */
#define SNS_SAM_GRAVITY_SVC_ID_V01 12

/**  SMGR internal service.  */
#define SNS_SMGR_INTERNAL_SVC_ID_V01 13

/**  Debug internal service.  */
#define SNS_DEBUG_INTERNAL_SVC_ID_V01 14

/**  Sensor registry V02.  */
#define SNS_REG2_SVC_ID_V01 15

/**  Algorithm: Mag Calibration  */
#define SNS_SAM_MAG_CAL_SVC_ID_V01 16

/**  Algorithm: Filtered magnetic vector.  */
#define SNS_SAM_FILTERED_MAG_SVC_ID_V01 17

/**  Algorithm: Rotation vector.  */
#define SNS_SAM_ROTATION_VECTOR_SVC_ID_V01 18

/**  Algorithm: Gyro-based quaternion.  */
#define SNS_SAM_QUATERNION_SVC_ID_V01 19

/**  Algorithm: Gravity vector.  */
#define SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01 20

/**  Algorithm: Sensor threshold.  */
#define SNS_SAM_SENSOR_THRESH_SVC_ID_V01 21

/**  Sensors time service.  */
#define SNS_TIME_SVC_ID_V01 22

/**  Algorithm: Orientation.  */
#define SNS_SAM_ORIENTATION_SVC_ID_V01 23

/**  Sensors time service V02.  */
#define SNS_TIME2_SVC_ID_V01 24

/**  Algorithm: Basic gestures.  */
#define SNS_SAM_BASIC_GESTURES_SVC_ID_V01 25

/**  Algorithm: Tap.  */
#define SNS_SAM_TAP_SVC_ID_V01 26

/**  Algorithm: Facing.  */
#define SNS_SAM_FACING_SVC_ID_V01 27

/**  Algorithm: Integrated angle.  */
#define SNS_SAM_INTEG_ANGLE_SVC_ID_V01 28

/**  Algorithm: Gyro assisted Tap.  */
#define SNS_SAM_GYRO_TAP_SVC_ID_V01 29

/**  Algorithm: Gyro assisted Tap V2.  */
#define SNS_SAM_GYRO_TAP2_SVC_ID_V01 30

/**  Reserved for OEM use.  */
#define SNS_OEM_1_SVC_ID_V01 31

/**  Reserved for OEM use.  */
#define SNS_OEM_2_SVC_ID_V01 32

/**  Reserved for OEM use.  */
#define SNS_OEM_3_SVC_ID_V01 33

/**  Algorithm: Gyrobuf algorithm (EIS).  */
#define SNS_SAM_GYROBUF_SVC_ID_V01 34

/**  Algorithm: Gyroint algorithm (EIS).  */
#define SNS_SAM_GYROINT_SVC_ID_V01 35

/**  Sensors file internal interface  */
#define SNS_FILE_INTERNAL_SVC_ID_V01 36

/**  Algorithm: Pedometer.  */
#define SNS_SAM_PED_SVC_ID_V01 37

/**  Algorithm: Pedestrian activity monitor.  */
#define SNS_SAM_PAM_SVC_ID_V01 38

/**  Algorithm: Detect modem scenarios.  */
#define SNS_SAM_MODEM_SCN_SVC_ID_V01 39

/**  Algorithm: Significant motion detector.  */
#define SNS_SAM_SMD_SVC_ID_V01 40

/**  Algorithm: Coarse motion classifier.  */
#define SNS_SAM_CMC_SVC_ID_V01 41

/**  Algorithm: Distance bound.  */
#define SNS_SAM_DISTANCE_BOUND_SVC_ID_V01 42

/**  Algorithm: Game rotation vector.  */
#define SNS_SAM_GAME_ROTATION_VECTOR_SVC_ID_V01 43

/**  SMGR restricted service.  */
#define SNS_SMGR_RESTRICTED_SVC_ID_V01 44

/**  Algorithm: Pedestrian alignment.  */
#define SNS_SAM_PED_ALIGNMENT_SVC_ID_V01 45

/**  Algorithm: Tilt detector.  */
#define SNS_SAM_TILT_DETECTOR_SVC_ID_V01 46

/**  Internal algorithm: Core pedometer.  */
#define SNS_SAM_INTERNAL_PED_SVC_ID_V01 47

/**  Device Position Classifier  */
#define SNS_SAM_DPC_SVC_ID_V01 48

/**  Event Gated Sensor Service  */
#define SNS_SAM_EVENT_GATED_SENSOR_SVC_ID_V01 49

/** ============================================================================
 Internal QMI Service IDs - Do not use for SMR or sensor1 requests
============================================================================ */
#define SNS_QMI_SVC_ID_0_V01 256
#define SNS_QMI_SVC_ID_1_V01 257
#define SNS_QMI_SVC_ID_2_V01 258
#define SNS_QMI_SVC_ID_3_V01 259
#define SNS_QMI_SVC_ID_4_V01 260
#define SNS_QMI_SVC_ID_5_V01 261
#define SNS_QMI_SVC_ID_6_V01 262
#define SNS_QMI_SVC_ID_7_V01 263
#define SNS_QMI_SVC_ID_8_V01 264
#define SNS_QMI_SVC_ID_9_V01 265
#define SNS_QMI_SVC_ID_10_V01 266
#define SNS_QMI_SVC_ID_11_V01 267
#define SNS_QMI_SVC_ID_12_V01 268
#define SNS_QMI_SVC_ID_13_V01 269
#define SNS_QMI_SVC_ID_14_V01 270
#define SNS_QMI_SVC_ID_15_V01 271
#define SNS_QMI_SVC_ID_16_V01 272
#define SNS_QMI_SVC_ID_17_V01 273
#define SNS_QMI_SVC_ID_18_V01 274
#define SNS_QMI_SVC_ID_19_V01 275
#define SNS_QMI_SVC_ID_20_V01 276
#define SNS_QMI_SVC_ID_21_V01 277
#define SNS_QMI_SVC_ID_22_V01 278
#define SNS_QMI_SVC_ID_23_V01 279
#define SNS_QMI_SVC_ID_24_V01 280
#define SNS_QMI_SVC_ID_25_V01 281
#define SNS_QMI_SVC_ID_26_V01 282
#define SNS_QMI_SVC_ID_27_V01 283
#define SNS_QMI_SVC_ID_28_V01 284
#define SNS_QMI_SVC_ID_29_V01 285
#define SNS_QMI_SVC_ID_30_V01 286
#define SNS_QMI_SVC_ID_31_V01 287
#define SNS_QMI_SVC_ID_32_V01 288
#define SNS_QMI_SVC_ID_33_V01 289
#define SNS_QMI_SVC_ID_34_V01 290
#define SNS_QMI_SVC_ID_35_V01 291
#define SNS_QMI_SVC_ID_36_V01 292
#define SNS_QMI_SVC_ID_37_V01 293
#define SNS_QMI_SVC_ID_38_V01 294
#define SNS_QMI_SVC_ID_39_V01 295
#define SNS_QMI_SVC_ID_40_V01 296
#define SNS_QMI_SVC_ID_41_V01 297
#define SNS_QMI_SVC_ID_42_V01 298
#define SNS_QMI_SVC_ID_43_V01 299
#define SNS_QMI_SVC_ID_44_V01 300
#define SNS_QMI_SVC_ID_45_V01 301
#define SNS_QMI_SVC_ID_46_V01 302
#define SNS_QMI_SVC_ID_47_V01 303
#define SNS_QMI_SVC_ID_48_V01 304
#define SNS_QMI_SVC_ID_49_V01 305

/** ============================================================================
 Batch indication types
============================================================================

 Standalone batch indication. Not part of a back-to-back indication stream.  */
#define SNS_BATCH_ONLY_IND_V01 0

/**  First indication in a stream of back-to-back indications.  */
#define SNS_BATCH_FIRST_IND_V01 1

/**  Intermediate indication in a stream of back-to-back indications.  */
#define SNS_BATCH_INTERMEDIATE_IND_V01 2

/**  Last indication in a stream of back-to-back indications.  */
#define SNS_BATCH_LAST_IND_V01 3

/** ============================================================================
 Common structures
============================================================================ */
#define SNS_RESULT_SUCCESS_V01 0
#define SNS_RESULT_FAILURE_V01 1
/**
    @}
  */

/** @addtogroup sns_common_qmi_aggregates
    @{
  */
typedef struct {

  /*  This structure shall be the first element of every response message  */
  uint8_t sns_result_t;
  /**<   0 == SUCCESS; 1 == FAILURE
     A result of FAILURE indicates that any data contained in the response
     should not be used (other than sns_err_t) to determine the type of error. */

  uint8_t sns_err_t;
  /**<   See sensor1_error_e in sensor1.h */
}sns_common_resp_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sns_common_qmi_enums
    @{
  */
typedef enum {
  SNS_BATCH_REQ_TYPE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_BATCH_NO_WAKE_UP_V01 = 0, /**<  Use to enable/disable batching. This
       does not awaken the client from Suspend when the buffer is full.  */
  SNS_BATCH_WAKE_UPON_FIFO_FULL_V01 = 1, /**<  Use to enable/disable batching. This
       awakens the client from Suspend when the buffer is full.  */
  SNS_BATCH_GET_MAX_FIFO_SIZE_V01 = 2, /**<  Use to get the maximum buffer depth. This
       does not enable/disable batching.  */
  SNS_BATCH_REQ_TYPE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_batch_req_type_e_v01;
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sns_common_cancel_req_msg_v01;

/** @addtogroup sns_common_qmi_messages
    @{
  */
/** Response Message; Cancels all requests from a client to any sensor service.
    This must be the first message of any sensor service. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common response message. */
}sns_common_cancel_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sns_common_version_req_msg_v01;

/** @addtogroup sns_common_qmi_messages
    @{
  */
/** Request Message; Requests the version of the sensor service.
    This must be the second message of any sensor service. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;
  /**<   Common message response. */

  /* Mandatory */
  uint32_t interface_version_number;
  /**<   Version number of the interface supported by the service. */

  /* Mandatory */
  uint16_t max_message_id;
  /**<   Maximum message ID supported by this service. */
}sns_common_version_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sns_common_qmi_enums
    @{
  */
typedef enum {
  SNS_PROC_TYPE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SNS_PROC_APPS_V01 = 0, /**<  Application processor.  */
  SNS_PROC_SSC_V01 = 1, /**<  Snapdragon sensors core processor(DSPS/ADSP).  */
  SNS_PROC_MODEM_V01 = 2, /**<  Modem processor.  */
  SNS_PROC_TYPE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_proc_type_e_v01;
/**
    @}
  */

/** @addtogroup sns_common_qmi_aggregates
    @{
  */
/** ============================================================================
 Common enums
============================================================================
 Identify the processor type.
  
 */
typedef struct {

  sns_proc_type_e_v01 proc_type;
  /**<   Defines the processor to monitor for suspend.
       This is typically the processor from which the
       request message originated.
  */

  uint8_t send_indications_during_suspend;
  /**<   Defines whether indications for this request are to be sent when the
       processor is in the Suspend state:
       - If send_indications_during_suspend=1 or TRUE, indications
         will be sent for this request when the processor is in the Suspend state
         (such indications may awaken the processor from Suspend).

      - If send_indications_during_suspend=0 or FALSE, some indications 
        will not be sent for this request when the processor is in the Suspend state.
        The list of indications not sent are defined by the message in question.
   */
}sns_suspend_notification_s_v01;  /* Type */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object sns_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

