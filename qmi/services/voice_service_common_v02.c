/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        V O I C E _ S E R V I C E _ C O M M O N _ V 0 2  . C

GENERAL DESCRIPTION
  This is the file which defines the voice_service_common service Data structures.

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/voice/main/latest/src/voice_service_common_v02.c#4 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.10 
   It was generated on: Wed Jul 16 2014 (Spin 0)
   From IDL File: voice_service_common_v02.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "voice_service_common_v02.h"


/*Type Definitions*/
static const uint8_t voice_call_end_reason_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_end_reason_type_v02, call_id),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_end_reason_type_v02, call_end_reason),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* Type Table */
static const qmi_idl_type_table_entry  voice_service_common_type_table_v02[] = {
  {sizeof(voice_call_end_reason_type_v02), voice_call_end_reason_type_data_v02}
};

/* Message Table */
/* No Messages Defined in IDL */

/* Range Table */
/* Predefine the Type Table Object */
const qmi_idl_type_table_object voice_service_common_qmi_idl_type_table_object_v02;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *voice_service_common_qmi_idl_type_table_object_referenced_tables_v02[] =
{&voice_service_common_qmi_idl_type_table_object_v02};

/*Type Table Object*/
const qmi_idl_type_table_object voice_service_common_qmi_idl_type_table_object_v02 = {
  sizeof(voice_service_common_type_table_v02)/sizeof(qmi_idl_type_table_entry ),
  0,
  1,
  voice_service_common_type_table_v02,
  NULL,
  voice_service_common_qmi_idl_type_table_object_referenced_tables_v02,
  NULL
};

