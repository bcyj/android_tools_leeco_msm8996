/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        I P _ M U L T I M E D I A _ S U B S Y S T E M _ A P P L I C A T I O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the imsa service Data structures.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/imsa/main/latest/src/ip_multimedia_subsystem_application_v01.c#11 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.5 
   It was generated on: Fri Sep 20 2013 (Spin 1)
   From IDL File: ip_multimedia_subsystem_application_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/* 
 * imsa_get_registration_status_req_msg is empty
 * static const uint8_t imsa_get_registration_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsa_get_registration_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registered) - QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registered_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registered),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registration_failure_error_code) - QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registration_failure_error_code_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registration_failure_error_code)
};

/* 
 * imsa_get_service_status_req_msg is empty
 * static const uint8_t imsa_get_service_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsa_get_service_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, sms_service_status) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, sms_service_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, sms_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, voip_service_status) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, voip_service_status_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, voip_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_status) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_status_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, sms_service_rat) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, sms_service_rat_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, sms_service_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, voip_service_rat) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, voip_service_rat_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, voip_service_rat),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_rat) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_rat_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_rat)
};

static const uint8_t imsa_ind_reg_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, reg_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, reg_status_config_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, reg_status_config),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, service_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, service_status_config_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, service_status_config)
};

static const uint8_t imsa_ind_reg_rsp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_ind_reg_rsp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t imsa_registration_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registered),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registration_failure_error_code) - QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registration_failure_error_code_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registration_failure_error_code)
};

static const uint8_t imsa_service_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, sms_service_status) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, sms_service_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, sms_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, voip_service_status) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, voip_service_status_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, voip_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_status) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_status_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, sms_service_rat) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, sms_service_rat_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, sms_service_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, voip_service_rat) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, voip_service_rat_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, voip_service_rat),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_rat) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_rat_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_rat)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry imsa_message_table_v01[] = {
  {sizeof(imsa_get_registration_status_req_msg_v01), 0},
  {sizeof(imsa_get_registration_status_resp_msg_v01), imsa_get_registration_status_resp_msg_data_v01},
  {sizeof(imsa_get_service_status_req_msg_v01), 0},
  {sizeof(imsa_get_service_status_resp_msg_v01), imsa_get_service_status_resp_msg_data_v01},
  {sizeof(imsa_ind_reg_req_msg_v01), imsa_ind_reg_req_msg_data_v01},
  {sizeof(imsa_ind_reg_rsp_msg_v01), imsa_ind_reg_rsp_msg_data_v01},
  {sizeof(imsa_registration_status_ind_msg_v01), imsa_registration_status_ind_msg_data_v01},
  {sizeof(imsa_service_status_ind_msg_v01), imsa_service_status_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object imsa_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *imsa_qmi_idl_type_table_object_referenced_tables_v01[] =
{&imsa_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object imsa_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(imsa_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  imsa_message_table_v01,
  imsa_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry imsa_service_command_messages_v01[] = {
  {QMI_IMSA_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_IMSA_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_IMSA_GET_SERVICE_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_IMSA_IND_REG_REQ_V01, QMI_IDL_TYPE16(0, 4), 8}
};

static const qmi_idl_service_message_table_entry imsa_service_response_messages_v01[] = {
  {QMI_IMSA_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_IMSA_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01, QMI_IDL_TYPE16(0, 1), 16},
  {QMI_IMSA_GET_SERVICE_STATUS_RSP_V01, QMI_IDL_TYPE16(0, 3), 49},
  {QMI_IMSA_IND_REG_RSP_V01, QMI_IDL_TYPE16(0, 5), 7}
};

static const qmi_idl_service_message_table_entry imsa_service_indication_messages_v01[] = {
  {QMI_IMSA_REGISTRATION_STATUS_IND_V01, QMI_IDL_TYPE16(0, 6), 9},
  {QMI_IMSA_SERVICE_STATUS_IND_V01, QMI_IDL_TYPE16(0, 7), 42}
};

/*Service Object*/
struct qmi_idl_service_object imsa_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x21,
  8204,
  { sizeof(imsa_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsa_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsa_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { imsa_service_command_messages_v01, imsa_service_response_messages_v01, imsa_service_indication_messages_v01},
  &imsa_qmi_idl_type_table_object_v01,
  0x05,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type imsa_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMSA_V01_IDL_MAJOR_VERS != idl_maj_version || IMSA_V01_IDL_MINOR_VERS != idl_min_version 
       || IMSA_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&imsa_qmi_idl_service_object_v01;
}

