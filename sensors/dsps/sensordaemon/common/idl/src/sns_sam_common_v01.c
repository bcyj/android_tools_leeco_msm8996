/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S A M _ C O M M O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sns_sam_common service Data structures.

  
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

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/* 
 * sns_sam_get_algo_attrib_req_msg is empty
 * static const uint8_t sns_sam_get_algo_attrib_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t sns_sam_get_algo_attrib_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, algorithm_revision),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, proc_type),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, supported_reporting_modes),

  0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, min_report_rate),

  0x07,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, max_report_rate),

  0x08,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, min_sample_rate),

  0x09,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, max_sample_rate),

  0x0A,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, max_batch_size),

  0x0B,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, power),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, sensorUID) - QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, sensorUID_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, sensorUID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, reserved_batch_size) - QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, reserved_batch_size_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_get_algo_attrib_resp_msg_v01, reserved_batch_size)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry sns_sam_common_message_table_v01[] = {
  {sizeof(sns_sam_get_algo_attrib_req_msg_v01), 0},
  {sizeof(sns_sam_get_algo_attrib_resp_msg_v01), sns_sam_get_algo_attrib_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
const qmi_idl_type_table_object sns_sam_common_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sns_sam_common_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sns_sam_common_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
const qmi_idl_type_table_object sns_sam_common_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(sns_sam_common_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  sns_sam_common_message_table_v01,
  sns_sam_common_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

