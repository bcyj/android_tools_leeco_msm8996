/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S A M _ G Y R O _ T A P _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SAM_GYRO_TAP_SVC service Data structures.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_gyro_tap_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_sam_gyro_tap_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_sam_gyro_tap_enable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, report_period),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, sample_rate) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, sample_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, sample_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_time_win) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_time_win_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_time_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_time_sleep) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_time_sleep_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_time_sleep),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_dir_win) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_dir_win_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tap_dir_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, history_win) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, history_win_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, history_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, orientation_change_win) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, orientation_change_win_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, orientation_change_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, jerk_win) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, jerk_win_valid)),
  0x16,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, jerk_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, accel_tap_thresh) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, accel_tap_thresh_valid)),
  0x17,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, accel_tap_thresh),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, mild_accel_tap_thresh) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, mild_accel_tap_thresh_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, mild_accel_tap_thresh),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, gyro_tap_thresh) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, gyro_tap_thresh_valid)),
  0x19,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, gyro_tap_thresh),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_min_accel_jerk_thresh_min) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_min_accel_jerk_thresh_min_valid)),
  0x1A,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_min_accel_jerk_thresh_min),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_min_gyro_jerk_thresh_min) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_min_gyro_jerk_thresh_min_valid)),
  0x1B,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_min_gyro_jerk_thresh_min),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_max_accel_jerk_thresh_min) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_max_accel_jerk_thresh_min_valid)),
  0x1C,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_max_accel_jerk_thresh_min),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_max_gyro_jerk_thresh_min) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_max_gyro_jerk_thresh_min_valid)),
  0x1D,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_max_gyro_jerk_thresh_min),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_jerk_min_thresh) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_jerk_min_thresh_valid)),
  0x1E,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_jerk_min_thresh),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_jerk_min_thresh) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_jerk_min_thresh_valid)),
  0x1F,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_jerk_min_thresh),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_yx) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_yx_valid)),
  0x20,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_yx),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_yz) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_yz_valid)),
  0x21,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_yz),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_gyro_rat_jerk_zy) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_gyro_rat_jerk_zy_valid)),
  0x22,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_gyro_rat_jerk_zy),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_gyro_rat_jerk_zx) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_gyro_rat_jerk_zx_valid)),
  0x23,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_gyro_rat_jerk_zx),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_rat_jerk_xy) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_rat_jerk_xy_valid)),
  0x24,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_rat_jerk_xy),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_xz) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_xz_valid)),
  0x25,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, lr_accel_rat_jerk_xz),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_rat_jerk_yx) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_rat_jerk_yx_valid)),
  0x26,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_rat_jerk_yx),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_rat_jerk_yz) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_rat_jerk_yz_valid)),
  0x27,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_gyro_rat_jerk_yz),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_thresh) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_thresh_valid)),
  0x28,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_thresh),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_rat_zx) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_rat_zx_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_rat_zx),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_rat_zy) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_rat_zy_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, tb_accel_z_rat_zy),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_reject_mode) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_reject_mode_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_reject_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_check_win) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_check_win_valid)),
  0x2C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, ori_check_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_win) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_win_valid)),
  0x2D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_win),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_thr) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_thr_valid)),
  0x2E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, ori_change_thr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, z_axis_inc) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, z_axis_inc_valid)),
  0x2F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, z_axis_inc),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, loaded_z_axis_anamoly) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, loaded_z_axis_anamoly_valid)),
  0x30,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, loaded_z_axis_anamoly),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, loaded_axis_3_valid) - QMI_IDL_OFFSET16RELATIVE(sns_sam_gyro_tap_enable_req_msg_v01, loaded_axis_3_valid_valid)),
  0x31,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_gyro_tap_enable_req_msg_v01, loaded_axis_3_valid)
};

static const uint8_t sns_sam_gyro_tap_enable_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_resp_msg_v01, instance_id) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_resp_msg_v01, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_enable_resp_msg_v01, instance_id)
};

static const uint8_t sns_sam_gyro_tap_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_disable_req_msg_v01, instance_id)
};

static const uint8_t sns_sam_gyro_tap_disable_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_gyro_tap_disable_resp_msg_v01, instance_id) - QMI_IDL_OFFSET8(sns_sam_gyro_tap_disable_resp_msg_v01, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_disable_resp_msg_v01, instance_id)
};

static const uint8_t sns_sam_gyro_tap_report_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_report_ind_msg_v01, instance_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_report_ind_msg_v01, timestamp),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_report_ind_msg_v01, tap_event)
};

static const uint8_t sns_sam_gyro_tap_error_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_error_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_gyro_tap_error_ind_msg_v01, instance_id)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_SAM_GYRO_TAP_SVC_message_table_v01[] = {
  {sizeof(sns_sam_gyro_tap_enable_req_msg_v01), sns_sam_gyro_tap_enable_req_msg_data_v01},
  {sizeof(sns_sam_gyro_tap_enable_resp_msg_v01), sns_sam_gyro_tap_enable_resp_msg_data_v01},
  {sizeof(sns_sam_gyro_tap_disable_req_msg_v01), sns_sam_gyro_tap_disable_req_msg_data_v01},
  {sizeof(sns_sam_gyro_tap_disable_resp_msg_v01), sns_sam_gyro_tap_disable_resp_msg_data_v01},
  {sizeof(sns_sam_gyro_tap_report_ind_msg_v01), sns_sam_gyro_tap_report_ind_msg_data_v01},
  {sizeof(sns_sam_gyro_tap_error_ind_msg_v01), sns_sam_gyro_tap_error_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SAM_GYRO_TAP_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SAM_GYRO_TAP_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_SAM_GYRO_TAP_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SAM_GYRO_TAP_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_SAM_GYRO_TAP_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_SAM_GYRO_TAP_SVC_message_table_v01,
  SNS_SAM_GYRO_TAP_SVC_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SAM_GYRO_TAP_SVC_service_command_messages_v01[] = {
  {SNS_SAM_GYRO_TAP_CANCEL_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_SAM_GYRO_TAP_VERSION_REQ_V01, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_SAM_GYRO_TAP_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 0), 233},
  {SNS_SAM_GYRO_TAP_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 2), 4}
};

static const qmi_idl_service_message_table_entry SNS_SAM_GYRO_TAP_SVC_service_response_messages_v01[] = {
  {SNS_SAM_GYRO_TAP_CANCEL_RESP_V01, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_SAM_GYRO_TAP_VERSION_RESP_V01, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_SAM_GYRO_TAP_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 1), 9},
  {SNS_SAM_GYRO_TAP_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 3), 9}
};

static const qmi_idl_service_message_table_entry SNS_SAM_GYRO_TAP_SVC_service_indication_messages_v01[] = {
  {SNS_SAM_GYRO_TAP_REPORT_IND_V01, QMI_IDL_TYPE16(0, 4), 18},
  {SNS_SAM_GYRO_TAP_ERROR_IND_V01, QMI_IDL_TYPE16(0, 5), 8}
};

/*Service Object*/
struct qmi_idl_service_object SNS_SAM_GYRO_TAP_SVC_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  SNS_QMI_SVC_ID_29_V01,
  233,
  { sizeof(SNS_SAM_GYRO_TAP_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SAM_GYRO_TAP_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SAM_GYRO_TAP_SVC_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SAM_GYRO_TAP_SVC_service_command_messages_v01, SNS_SAM_GYRO_TAP_SVC_service_response_messages_v01, SNS_SAM_GYRO_TAP_SVC_service_indication_messages_v01},
  &SNS_SAM_GYRO_TAP_SVC_qmi_idl_type_table_object_v01,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SAM_GYRO_TAP_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SAM_GYRO_TAP_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_SAM_GYRO_TAP_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_SAM_GYRO_TAP_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SAM_GYRO_TAP_SVC_qmi_idl_service_object_v01;
}

