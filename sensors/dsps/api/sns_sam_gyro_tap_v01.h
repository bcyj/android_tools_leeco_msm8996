#ifndef SNS_SAM_GYRO_TAP_SVC_SERVICE_01_H
#define SNS_SAM_GYRO_TAP_SVC_SERVICE_01_H
/**
  @file sns_sam_gyro_tap_v01.h

  @brief This is the public header file which defines the SNS_SAM_GYRO_TAP_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SAM_GYRO_TAP_SVC. It contains the constant values defined, enums, structures,
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
   From IDL File: sns_sam_gyro_tap_v01.idl */

/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SAM_GYRO_TAP_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SAM_GYRO_TAP_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SAM_GYRO_TAP_SVC_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SAM_GYRO_TAP_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SAM_GYRO_TAP_SVC_V01_MAX_MESSAGE_ID 0x0005
/**
    @}
  */


/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_consts 
    @{ 
  */
#define SNS_SAM_GYRO_TAP_SUID_V01 0x226b1b3e147f0598
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_enums
    @{
  */
typedef enum {
  SNS_SAM_GYRO_TAP_EVENT_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  GYRO_TAP_LEFT_V01 = 1, /**<  Phone is tapped on the left. # */
  GYRO_TAP_RIGHT_V01 = 2, /**<  Phone is tapped on the right.  */
  GYRO_TAP_TOP_V01 = 3, /**<  Phone is tapped on the top.  */
  GYRO_TAP_BOTTOM_V01 = 4, /**<  Phone is tapped on the bottom.  */
  GYRO_TAP_FRONT_V01 = 5, /**<  Phone is tapped on the front.  */
  GYRO_TAP_BACK_V01 = 6, /**<  Phone is tapped on the back.  */
  SNS_SAM_GYRO_TAP_EVENT_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}sns_sam_gyro_tap_event_e_v01;
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_messages
    @{
  */
/** Request Message; This command enables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint32_t report_period;
  /**<   Unit of seconds, Q16; value of 0 means reporting on new event only  */

  /* Optional */
  uint8_t sample_rate_valid;  /**< Must be set to true if sample_rate is being passed */
  int32_t sample_rate;
  /**<   sample rate in Hz, Q16 */

  /* Optional */
  uint8_t tap_time_win_valid;  /**< Must be set to true if tap_time_win is being passed */
  int32_t tap_time_win;
  /**<   time over which a tap must occur, s,  Q16 */

  /* Optional */
  uint8_t tap_time_sleep_valid;  /**< Must be set to true if tap_time_sleep is being passed */
  int32_t tap_time_sleep;
  /**<   max time over which a tap can occur, s,  Q16 */

  /* Optional */
  uint8_t tap_dir_win_valid;  /**< Must be set to true if tap_dir_win is being passed */
  int32_t tap_dir_win;
  /**<   time over which a tap direction is determined, s,  Q16 */

  /* Optional */
  uint8_t history_win_valid;  /**< Must be set to true if history_win is being passed */
  int32_t history_win;
  /**<   Maintained length of history  - post tentative tap, s,  Q16 */

  /* Optional */
  uint8_t orientation_change_win_valid;  /**< Must be set to true if orientation_change_win is being passed */
  int32_t orientation_change_win;
  /**<   time over which an orientation change is determined, s,  Q16 */

  /* Optional */
  uint8_t jerk_win_valid;  /**< Must be set to true if jerk_win is being passed */
  int32_t jerk_win;
  /**<   time over which a jerk is determined, s,  Q16 */

  /* Optional */
  uint8_t accel_tap_thresh_valid;  /**< Must be set to true if accel_tap_thresh is being passed */
  int32_t accel_tap_thresh;
  /**<   accel threshold in m/s/s, Q16 */

  /* Optional */
  uint8_t mild_accel_tap_thresh_valid;  /**< Must be set to true if mild_accel_tap_thresh is being passed */
  int32_t mild_accel_tap_thresh;
  /**<   mild tap accel threshold in m/s/s, Q16 */

  /* Optional */
  uint8_t gyro_tap_thresh_valid;  /**< Must be set to true if gyro_tap_thresh is being passed */
  int32_t gyro_tap_thresh;
  /**<   gyro_tap threshold in radians/s, Q16 */

  /* Optional */
  uint8_t lr_min_accel_jerk_thresh_min_valid;  /**< Must be set to true if lr_min_accel_jerk_thresh_min is being passed */
  int32_t lr_min_accel_jerk_thresh_min;
  /**<   left/right min min-accel-jerk threshold in m/s/s, Q16 */

  /* Optional */
  uint8_t lr_min_gyro_jerk_thresh_min_valid;  /**< Must be set to true if lr_min_gyro_jerk_thresh_min is being passed */
  int32_t lr_min_gyro_jerk_thresh_min;
  /**<   left/right min min-gyro-jerk threshold in radians/s, Q16 */

  /* Optional */
  uint8_t lr_max_accel_jerk_thresh_min_valid;  /**< Must be set to true if lr_max_accel_jerk_thresh_min is being passed */
  int32_t lr_max_accel_jerk_thresh_min;
  /**<   left/right max min-accel-jerk threshold in m/s/s, Q16 */

  /* Optional */
  uint8_t lr_max_gyro_jerk_thresh_min_valid;  /**< Must be set to true if lr_max_gyro_jerk_thresh_min is being passed */
  int32_t lr_max_gyro_jerk_thresh_min;
  /**<   left/right max min-gyro-jerk threshold in radians/s, Q16 */

  /* Optional */
  uint8_t tb_accel_jerk_min_thresh_valid;  /**< Must be set to true if tb_accel_jerk_min_thresh is being passed */
  int32_t tb_accel_jerk_min_thresh;
  /**<   top/botton min jerk threshold in m/s/s, Q16 */

  /* Optional */
  uint8_t tb_gyro_jerk_min_thresh_valid;  /**< Must be set to true if tb_gyro_jerk_min_thresh is being passed */
  int32_t tb_gyro_jerk_min_thresh;
  /**<   top/bottom min gyro jerk threshold in radians/s, Q16 */

  /* Optional */
  uint8_t lr_accel_rat_jerk_yx_valid;  /**< Must be set to true if lr_accel_rat_jerk_yx is being passed */
  int32_t lr_accel_rat_jerk_yx;
  /**<   ratio of L/R accel jerk for y/x axes */

  /* Optional */
  uint8_t lr_accel_rat_jerk_yz_valid;  /**< Must be set to true if lr_accel_rat_jerk_yz is being passed */
  int32_t lr_accel_rat_jerk_yz;
  /**<   ratio of L/R accel jerk for y/z axes */

  /* Optional */
  uint8_t lr_gyro_rat_jerk_zy_valid;  /**< Must be set to true if lr_gyro_rat_jerk_zy is being passed */
  int32_t lr_gyro_rat_jerk_zy;
  /**<   ratio of L/R gyro jerk for z/y axes */

  /* Optional */
  uint8_t lr_gyro_rat_jerk_zx_valid;  /**< Must be set to true if lr_gyro_rat_jerk_zx is being passed */
  int32_t lr_gyro_rat_jerk_zx;
  /**<   ratio of L/R gyro jerk for z/x axes */

  /* Optional */
  uint8_t tb_accel_rat_jerk_xy_valid;  /**< Must be set to true if tb_accel_rat_jerk_xy is being passed */
  int32_t tb_accel_rat_jerk_xy;
  /**<   ratio of T/B accel jerk for x/y axes */

  /* Optional */
  uint8_t lr_accel_rat_jerk_xz_valid;  /**< Must be set to true if lr_accel_rat_jerk_xz is being passed */
  int32_t lr_accel_rat_jerk_xz;
  /**<   ratio of T/B accel jerk for x/z axes */

  /* Optional */
  uint8_t tb_gyro_rat_jerk_yx_valid;  /**< Must be set to true if tb_gyro_rat_jerk_yx is being passed */
  int32_t tb_gyro_rat_jerk_yx;
  /**<   ratio of T/B gyro jerk for y/x axes */

  /* Optional */
  uint8_t tb_gyro_rat_jerk_yz_valid;  /**< Must be set to true if tb_gyro_rat_jerk_yz is being passed */
  int32_t tb_gyro_rat_jerk_yz;
  /**<   ratio of T/B gyro jerk for y/z axes */

  /* Optional */
  uint8_t tb_accel_z_thresh_valid;  /**< Must be set to true if tb_accel_z_thresh is being passed */
  int32_t tb_accel_z_thresh;
  /**<   t/b accel z thresh, ms/s/s, Q16 */

  /* Optional */
  uint8_t tb_accel_z_rat_zx_valid;  /**< Must be set to true if tb_accel_z_rat_zx is being passed */
  int32_t tb_accel_z_rat_zx;
  /**<   t/b accel z versus zx ratio */

  /* Optional */
  uint8_t tb_accel_z_rat_zy_valid;  /**< Must be set to true if tb_accel_z_rat_zy is being passed */
  int32_t tb_accel_z_rat_zy;
  /**<   t/b accel z versus zy ratio */

  /* Optional */
  uint8_t ori_change_reject_mode_valid;  /**< Must be set to true if ori_change_reject_mode is being passed */
  uint8_t ori_change_reject_mode;
  /**<   orientation change rejection mode (0 == off, 1 == on)*/

  /* Optional */
  uint8_t ori_check_win_valid;  /**< Must be set to true if ori_check_win is being passed */
  int32_t ori_check_win;
  /**<   orientation check window , seconds, Q16  */

  /* Optional */
  uint8_t ori_change_win_valid;  /**< Must be set to true if ori_change_win is being passed */
  int32_t ori_change_win;
  /**<   orientation change window , seconds, Q16  */

  /* Optional */
  uint8_t ori_change_thr_valid;  /**< Must be set to true if ori_change_thr is being passed */
  int32_t ori_change_thr;
  /**<   orientation change threshold metric  */

  /* Optional */
  uint8_t z_axis_inc_valid;  /**< Must be set to true if z_axis_inc is being passed */
  uint8_t z_axis_inc;
  /**<   Z axis anamoly handling mode (0 == off, 1 == on)*/

  /* Optional */
  uint8_t loaded_z_axis_anamoly_valid;  /**< Must be set to true if loaded_z_axis_anamoly is being passed */
  int8_t loaded_z_axis_anamoly;
  /**<   Z axis anamoly loaded  */

  /* Optional */
  uint8_t loaded_axis_3_valid_valid;  /**< Must be set to true if loaded_axis_3_valid is being passed */
  uint8_t loaded_axis_3_valid;
  /**<   Z axis load valid  */
}sns_sam_gyro_tap_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_messages
    @{
  */
/** Response Message; This command enables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
  /**<  
    The instance ID is maintained/assigned by SAM.
    The client shall use this instance ID for future messages associated with
    current algorithm instance.
  */
}sns_sam_gyro_tap_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_messages
    @{
  */
/** Request Message; This command disables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   To identify an instance of an algorithm.  */
}sns_sam_gyro_tap_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_messages
    @{
  */
/** Response Message; This command disables a sensor algorithm. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 resp;

  /* Optional */
  uint8_t instance_id_valid;  /**< Must be set to true if instance_id is being passed */
  uint8_t instance_id;
}sns_sam_gyro_tap_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_messages
    @{
  */
/** Indication Message; Output report from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t instance_id;
  /**<   time stamp of input which caused this indication; in ticks */

  /* Mandatory */
  uint32_t timestamp;
  /**<   TAP direction */

  /* Mandatory */
  sns_sam_gyro_tap_event_e_v01 tap_event;
}sns_sam_gyro_tap_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_messages
    @{
  */
/** Indication Message; Asynchronous error report from a sensor algorithm. */
typedef struct {

  /* Mandatory */
  uint8_t error;
  /**<   sensors error code */

  /* Mandatory */
  uint8_t instance_id;
}sns_sam_gyro_tap_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_SAM_GYRO_TAP_SVC_qmi_msg_ids
    @{
  */
#define SNS_SAM_GYRO_TAP_CANCEL_REQ_V01 0x0000
#define SNS_SAM_GYRO_TAP_CANCEL_RESP_V01 0x0000
#define SNS_SAM_GYRO_TAP_VERSION_REQ_V01 0x0001
#define SNS_SAM_GYRO_TAP_VERSION_RESP_V01 0x0001
#define SNS_SAM_GYRO_TAP_ENABLE_REQ_V01 0x0002
#define SNS_SAM_GYRO_TAP_ENABLE_RESP_V01 0x0002
#define SNS_SAM_GYRO_TAP_DISABLE_REQ_V01 0x0003
#define SNS_SAM_GYRO_TAP_DISABLE_RESP_V01 0x0003
#define SNS_SAM_GYRO_TAP_REPORT_IND_V01 0x0004
#define SNS_SAM_GYRO_TAP_ERROR_IND_V01 0x0005
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SAM_GYRO_TAP_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SAM_GYRO_TAP_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SAM_GYRO_TAP_SVC_get_service_object_v01( ) \
          SNS_SAM_GYRO_TAP_SVC_get_service_object_internal_v01( \
            SNS_SAM_GYRO_TAP_SVC_V01_IDL_MAJOR_VERS, SNS_SAM_GYRO_TAP_SVC_V01_IDL_MINOR_VERS, \
            SNS_SAM_GYRO_TAP_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

