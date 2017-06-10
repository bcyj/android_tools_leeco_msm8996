/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S M G R _ R E S T R I C T E D _ A P I _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SMGR_RESTRICTED_SVC service Data structures.

  
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6 
   It was generated on: Tue Mar  4 2014 (Spin 0)
   From IDL File: sns_smgr_restricted_api_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_smgr_restricted_api_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_smgr_driver_access_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, Uuid),
  SNS_SMGR_UUID_LENGTH_V01,

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, RequestId),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, RequestMsg) - QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, RequestMsg_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, RequestMsg),
  ((SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01) & 0xFF), ((SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, RequestMsg) - QMI_IDL_OFFSET8(sns_smgr_driver_access_req_msg_v01, RequestMsg_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_driver_access_req_msg_v01, TransactionId) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_driver_access_req_msg_v01, TransactionId_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_driver_access_req_msg_v01, TransactionId)
};

static const uint8_t sns_smgr_driver_access_resp_msg_data_v01[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, Resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseStatus) - QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseStatus_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseStatus),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseMsg) - QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseMsg_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseMsg),
  ((SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01) & 0xFF), ((SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseMsg) - QMI_IDL_OFFSET8(sns_smgr_driver_access_resp_msg_v01, ResponseMsg_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_driver_access_resp_msg_v01, TransactionId) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_driver_access_resp_msg_v01, TransactionId_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_driver_access_resp_msg_v01, TransactionId)
};

static const uint8_t sns_smgr_driver_access_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_ind_msg_v01, IndicationId),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_driver_access_ind_msg_v01, IndicationMsg) - QMI_IDL_OFFSET8(sns_smgr_driver_access_ind_msg_v01, IndicationMsg_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_driver_access_ind_msg_v01, IndicationMsg),
  ((SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01) & 0xFF), ((SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(sns_smgr_driver_access_ind_msg_v01, IndicationMsg) - QMI_IDL_OFFSET8(sns_smgr_driver_access_ind_msg_v01, IndicationMsg_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_smgr_driver_access_ind_msg_v01, TransactionId) - QMI_IDL_OFFSET16RELATIVE(sns_smgr_driver_access_ind_msg_v01, TransactionId_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_smgr_driver_access_ind_msg_v01, TransactionId)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_SMGR_RESTRICTED_SVC_message_table_v01[] = {
  {sizeof(sns_smgr_driver_access_req_msg_v01), sns_smgr_driver_access_req_msg_data_v01},
  {sizeof(sns_smgr_driver_access_resp_msg_v01), sns_smgr_driver_access_resp_msg_data_v01},
  {sizeof(sns_smgr_driver_access_ind_msg_v01), sns_smgr_driver_access_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SMGR_RESTRICTED_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SMGR_RESTRICTED_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_SMGR_RESTRICTED_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SMGR_RESTRICTED_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_SMGR_RESTRICTED_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_SMGR_RESTRICTED_SVC_message_table_v01,
  SNS_SMGR_RESTRICTED_SVC_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SMGR_RESTRICTED_SVC_service_command_messages_v01[] = {
  {SNS_SMGR_RESTRICTED_CANCEL_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_SMGR_RESTRICTED_VERSION_REQ_V01, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_SMGR_DRIVER_ACCESS_REQ_V01, QMI_IDL_TYPE16(0, 0), 291}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_RESTRICTED_SVC_service_response_messages_v01[] = {
  {SNS_SMGR_RESTRICTED_CANCEL_RESP_V01, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_SMGR_RESTRICTED_VERSION_RESP_V01, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_SMGR_DRIVER_ACCESS_RESP_V01, QMI_IDL_TYPE16(0, 1), 274}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_RESTRICTED_SVC_service_indication_messages_v01[] = {
  {SNS_SMGR_DRIVER_ACCESS_IND_V01, QMI_IDL_TYPE16(0, 2), 269}
};

/*Service Object*/
struct qmi_idl_service_object SNS_SMGR_RESTRICTED_SVC_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  SNS_QMI_SVC_ID_44_V01,
  291,
  { sizeof(SNS_SMGR_RESTRICTED_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_RESTRICTED_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_RESTRICTED_SVC_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SMGR_RESTRICTED_SVC_service_command_messages_v01, SNS_SMGR_RESTRICTED_SVC_service_response_messages_v01, SNS_SMGR_RESTRICTED_SVC_service_indication_messages_v01},
  &SNS_SMGR_RESTRICTED_SVC_qmi_idl_type_table_object_v01,
  0x03,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SMGR_RESTRICTED_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SMGR_RESTRICTED_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_SMGR_RESTRICTED_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_SMGR_RESTRICTED_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SMGR_RESTRICTED_SVC_qmi_idl_service_object_v01;
}

