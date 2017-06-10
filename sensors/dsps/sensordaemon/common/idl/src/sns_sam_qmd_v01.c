/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S A M _ Q M D _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sns_sam_qmd service Data structures.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_qmd_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_sam_qmd_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
static const uint8_t sns_qmd_config_s_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_qmd_config_s_v01, var_thresh),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_qmd_config_s_v01, var_win_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t sns_sam_qmd_enable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, report_period),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, config) - QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, config_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, config),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, notify_suspend) - QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, notify_suspend_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_qmd_enable_req_msg_v01, notify_suspend),
  QMI_IDL_TYPE88(1, 1)
};

static const uint8_t sns_sam_qmd_enable_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_qmd_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_enable_resp_msg_v01, instance_id)
};

static const uint8_t sns_sam_qmd_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_disable_req_msg_v01, instance_id)
};

static const uint8_t sns_sam_qmd_disable_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_qmd_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_disable_resp_msg_v01, instance_id)
};

static const uint8_t sns_sam_qmd_report_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_report_ind_msg_v01, instance_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_report_ind_msg_v01, timestamp),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_report_ind_msg_v01, state)
};

static const uint8_t sns_sam_qmd_get_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_get_report_req_msg_v01, instance_id)
};

static const uint8_t sns_sam_qmd_get_report_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_qmd_get_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_get_report_resp_msg_v01, instance_id),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_get_report_resp_msg_v01, timestamp),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_get_report_resp_msg_v01, state)
};

static const uint8_t sns_sam_qmd_error_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_error_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_qmd_error_ind_msg_v01, instance_id)
};

/* Type Table */
static const qmi_idl_type_table_entry  sns_sam_qmd_type_table_v01[] = {
  {sizeof(sns_qmd_config_s_v01), sns_qmd_config_s_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry sns_sam_qmd_message_table_v01[] = {
  {sizeof(sns_sam_qmd_enable_req_msg_v01), sns_sam_qmd_enable_req_msg_data_v01},
  {sizeof(sns_sam_qmd_enable_resp_msg_v01), sns_sam_qmd_enable_resp_msg_data_v01},
  {sizeof(sns_sam_qmd_disable_req_msg_v01), sns_sam_qmd_disable_req_msg_data_v01},
  {sizeof(sns_sam_qmd_disable_resp_msg_v01), sns_sam_qmd_disable_resp_msg_data_v01},
  {sizeof(sns_sam_qmd_report_ind_msg_v01), sns_sam_qmd_report_ind_msg_data_v01},
  {sizeof(sns_sam_qmd_get_report_req_msg_v01), sns_sam_qmd_get_report_req_msg_data_v01},
  {sizeof(sns_sam_qmd_get_report_resp_msg_v01), sns_sam_qmd_get_report_resp_msg_data_v01},
  {sizeof(sns_sam_qmd_error_ind_msg_v01), sns_sam_qmd_error_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
const qmi_idl_type_table_object sns_sam_qmd_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sns_sam_qmd_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sns_sam_qmd_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
const qmi_idl_type_table_object sns_sam_qmd_qmi_idl_type_table_object_v01 = {
  sizeof(sns_sam_qmd_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(sns_sam_qmd_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  sns_sam_qmd_type_table_v01,
  sns_sam_qmd_message_table_v01,
  sns_sam_qmd_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

