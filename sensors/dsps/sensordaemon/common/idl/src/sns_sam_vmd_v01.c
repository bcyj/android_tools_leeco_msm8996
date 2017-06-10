/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S A M _ V M D _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SAM_VMD_SVC service Data structures.

  
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_vmd_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_sam_vmd_v01.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"
#include "sns_sam_qmd_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/* Type Table */
/* Message Table */
/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SAM_VMD_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SAM_VMD_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_SAM_VMD_SVC_qmi_idl_type_table_object_v01, &sns_sam_common_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01, &sns_sam_qmd_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SAM_VMD_SVC_qmi_idl_type_table_object_v01 = {
  0,
  0,
  1,
  NULL,
  NULL,
  SNS_SAM_VMD_SVC_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SAM_VMD_SVC_service_command_messages_v01[] = {
  {SNS_SAM_VMD_CANCEL_REQ_V01, QMI_IDL_TYPE16(2, 0), 0},
  {SNS_SAM_VMD_VERSION_REQ_V01, QMI_IDL_TYPE16(2, 2), 0},
  {SNS_SAM_VMD_ENABLE_REQ_V01, QMI_IDL_TYPE16(3, 0), 26},
  {SNS_SAM_VMD_DISABLE_REQ_V01, QMI_IDL_TYPE16(3, 2), 4},
  {SNS_SAM_VMD_GET_REPORT_REQ_V01, QMI_IDL_TYPE16(3, 5), 4},
  {SNS_SAM_VMD_GET_ATTRIBUTES_REQ_V01, QMI_IDL_TYPE16(1, 0), 0}
};

static const qmi_idl_service_message_table_entry SNS_SAM_VMD_SVC_service_response_messages_v01[] = {
  {SNS_SAM_VMD_CANCEL_RESP_V01, QMI_IDL_TYPE16(2, 1), 5},
  {SNS_SAM_VMD_VERSION_RESP_V01, QMI_IDL_TYPE16(2, 3), 17},
  {SNS_SAM_VMD_ENABLE_RESP_V01, QMI_IDL_TYPE16(3, 1), 9},
  {SNS_SAM_VMD_DISABLE_RESP_V01, QMI_IDL_TYPE16(3, 3), 9},
  {SNS_SAM_VMD_GET_REPORT_RESP_V01, QMI_IDL_TYPE16(3, 6), 23},
  {SNS_SAM_VMD_GET_ATTRIBUTES_RESP_V01, QMI_IDL_TYPE16(1, 1), 86}
};

static const qmi_idl_service_message_table_entry SNS_SAM_VMD_SVC_service_indication_messages_v01[] = {
  {SNS_SAM_VMD_REPORT_IND_V01, QMI_IDL_TYPE16(3, 4), 18},
  {SNS_SAM_VMD_ERROR_IND_V01, QMI_IDL_TYPE16(3, 7), 8}
};

/*Service Object*/
struct qmi_idl_service_object SNS_SAM_VMD_SVC_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  SNS_QMI_SVC_ID_6_V01,
  86,
  { sizeof(SNS_SAM_VMD_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SAM_VMD_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SAM_VMD_SVC_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SAM_VMD_SVC_service_command_messages_v01, SNS_SAM_VMD_SVC_service_response_messages_v01, SNS_SAM_VMD_SVC_service_indication_messages_v01},
  &SNS_SAM_VMD_SVC_qmi_idl_type_table_object_v01,
  0x04,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SAM_VMD_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SAM_VMD_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_SAM_VMD_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_SAM_VMD_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SAM_VMD_SVC_qmi_idl_service_object_v01;
}

