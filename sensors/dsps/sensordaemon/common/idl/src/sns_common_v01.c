/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ C O M M O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sns_common service Data structures.

  
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

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_common_v01.h"


/*Type Definitions*/
static const uint8_t sns_common_resp_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_common_resp_s_v01, sns_result_t),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_common_resp_s_v01, sns_err_t),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_suspend_notification_s_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_suspend_notification_s_v01, proc_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_suspend_notification_s_v01, send_indications_during_suspend),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * sns_common_cancel_req_msg is empty
 * static const uint8_t sns_common_cancel_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t sns_common_cancel_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_common_cancel_resp_msg_v01, resp),
  QMI_IDL_TYPE88(0, 0)
};

/* 
 * sns_common_version_req_msg is empty
 * static const uint8_t sns_common_version_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t sns_common_version_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_common_version_resp_msg_v01, resp),
  QMI_IDL_TYPE88(0, 0),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_common_version_resp_msg_v01, interface_version_number),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_common_version_resp_msg_v01, max_message_id)
};

/* Type Table */
static const qmi_idl_type_table_entry  sns_common_type_table_v01[] = {
  {sizeof(sns_common_resp_s_v01), sns_common_resp_s_data_v01},
  {sizeof(sns_suspend_notification_s_v01), sns_suspend_notification_s_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry sns_common_message_table_v01[] = {
  {sizeof(sns_common_cancel_req_msg_v01), 0},
  {sizeof(sns_common_cancel_resp_msg_v01), sns_common_cancel_resp_msg_data_v01},
  {sizeof(sns_common_version_req_msg_v01), 0},
  {sizeof(sns_common_version_resp_msg_v01), sns_common_version_resp_msg_data_v01}
};

/* Range Table */
/* Predefine the Type Table Object */
const qmi_idl_type_table_object sns_common_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sns_common_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
const qmi_idl_type_table_object sns_common_qmi_idl_type_table_object_v01 = {
  sizeof(sns_common_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(sns_common_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  sns_common_type_table_v01,
  sns_common_message_table_v01,
  sns_common_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

