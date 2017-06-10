/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        I P _ M U L T I M E D I A _ S U B S Y S T E M _ A P P L I C A T I O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the imsa service Data structures.

  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header: //source/qcom/qct/interfaces/qmi/imsa/main/latest/src/ip_multimedia_subsystem_application_v01.c#32 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.5 
   It was generated on: Thu Apr 30 2015 (Spin 0)
   From IDL File: ip_multimedia_subsystem_application_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t imsa_rat_handover_status_info_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_rat_handover_status_info_v01, rat_ho_status),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_rat_handover_status_info_v01, source_rat),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_rat_handover_status_info_v01, target_rat),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsa_rat_handover_status_info_v01, cause_code),
  IMSA_HO_FAILURE_CAUSE_CODE_STR_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

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

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registration_failure_error_code) - QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registration_failure_error_code_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_registration_failure_error_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_reg_status) - QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_reg_status_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, ims_reg_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, registration_error_string) - QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, registration_error_string_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsa_get_registration_status_resp_msg_v01, registration_error_string),
  IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsa_get_registration_status_resp_msg_v01, registration_network) - QMI_IDL_OFFSET16RELATIVE(imsa_get_registration_status_resp_msg_v01, registration_network_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsa_get_registration_status_resp_msg_v01, registration_network)
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

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_rat) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_rat_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vt_service_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, ut_service_status) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, ut_service_status_valid)),
  0x16,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, ut_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, ut_service_rat) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, ut_service_rat_valid)),
  0x17,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, ut_service_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vs_service_status) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vs_service_status_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vs_service_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vs_service_rat) - QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vs_service_rat_valid)),
  0x19,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_service_status_resp_msg_v01, vs_service_rat)
};

static const uint8_t imsa_ind_reg_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, reg_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, reg_status_config_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, reg_status_config),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, service_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, service_status_config_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, service_status_config),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, rat_handover_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, rat_handover_config_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, rat_handover_config),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, pdp_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, pdp_status_config_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, pdp_status_config),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, acs_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, acs_status_config_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, acs_status_config),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, lte_attach_params_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, lte_attach_params_config_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, lte_attach_params_config),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, subscription_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, subscription_status_config_valid)),
  0x16,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, subscription_status_config),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, network_provisioning_status_config) - QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, network_provisioning_status_config_valid)),
  0x17,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_ind_reg_req_msg_v01, network_provisioning_status_config)
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

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registration_failure_error_code) - QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registration_failure_error_code_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_registration_failure_error_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_reg_status) - QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_reg_status_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, ims_reg_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, registration_error_string) - QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, registration_error_string_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsa_registration_status_ind_msg_v01, registration_error_string),
  IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsa_registration_status_ind_msg_v01, registration_network) - QMI_IDL_OFFSET16RELATIVE(imsa_registration_status_ind_msg_v01, registration_network_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsa_registration_status_ind_msg_v01, registration_network)
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

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_rat) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_rat_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vt_service_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, ut_service_status) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, ut_service_status_valid)),
  0x16,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, ut_service_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, ut_service_rat) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, ut_service_rat_valid)),
  0x17,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, ut_service_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vs_service_status) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vs_service_status_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vs_service_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vs_service_rat) - QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vs_service_rat_valid)),
  0x19,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_service_status_ind_msg_v01, vs_service_rat)
};

static const uint8_t imsa_rat_handover_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_rat_handover_status_ind_msg_v01, rat_ho_status_info) - QMI_IDL_OFFSET8(imsa_rat_handover_status_ind_msg_v01, rat_ho_status_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_rat_handover_status_ind_msg_v01, rat_ho_status_info),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t imsa_pdp_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_pdp_status_ind_msg_v01, is_ims_pdp_connected),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_pdp_status_ind_msg_v01, ims_pdp_failure_error_code) - QMI_IDL_OFFSET8(imsa_pdp_status_ind_msg_v01, ims_pdp_failure_error_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_pdp_status_ind_msg_v01, ims_pdp_failure_error_code)
};

static const uint8_t imsa_acs_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_acs_status_ind_msg_v01, acs_failure_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_acs_status_ind_msg_v01, acs_failure_error_code) - QMI_IDL_OFFSET8(imsa_acs_status_ind_msg_v01, acs_failure_error_code_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsa_acs_status_ind_msg_v01, acs_failure_error_code)
};

/* 
 * imsa_get_pdp_status_req_msg is empty
 * static const uint8_t imsa_get_pdp_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsa_get_pdp_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, is_ims_pdp_connected) - QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, is_ims_pdp_connected_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, is_ims_pdp_connected),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, ims_pdp_failure_error_code) - QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, ims_pdp_failure_error_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_get_pdp_status_resp_msg_v01, ims_pdp_failure_error_code)
};

static const uint8_t imsa_lte_attach_parameters_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_lte_attach_parameters_ind_msg_v01, operator_reserved_pco)
};

/* 
 * imsa_get_lte_attach_parameters_req_msg is empty
 * static const uint8_t imsa_get_lte_attach_parameters_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsa_get_lte_attach_parameters_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_get_lte_attach_parameters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_get_lte_attach_parameters_resp_msg_v01, operator_reserved_pco) - QMI_IDL_OFFSET8(imsa_get_lte_attach_parameters_resp_msg_v01, operator_reserved_pco_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_get_lte_attach_parameters_resp_msg_v01, operator_reserved_pco)
};

static const uint8_t imsa_subscription_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_type),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_error_string) - QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_error_string_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_error_string),
  IMSA_SUBSCRIPTION_FAILURE_ERROR_STR_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_error_code) - QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_error_code_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsa_subscription_status_ind_msg_v01, subscription_error_code)
};

static const uint8_t imsa_network_provisioning_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsa_network_provisioning_status_ind_msg_v01, network_provisioning_status)
};

/* 
 * imsa_get_rtp_statistics_req_msg is empty
 * static const uint8_t imsa_get_rtp_statistics_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsa_get_rtp_statistics_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsa_get_rtp_statistics_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t imsa_rtp_statistics_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_rtp_statistics_ind_msg_v01, total_rx_expected_rtp_pkt_count) - QMI_IDL_OFFSET8(imsa_rtp_statistics_ind_msg_v01, total_rx_expected_rtp_pkt_count_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsa_rtp_statistics_ind_msg_v01, total_rx_expected_rtp_pkt_count),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsa_rtp_statistics_ind_msg_v01, total_rx_rtp_pkt_loss_count) - QMI_IDL_OFFSET8(imsa_rtp_statistics_ind_msg_v01, total_rx_rtp_pkt_loss_count_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsa_rtp_statistics_ind_msg_v01, total_rx_rtp_pkt_loss_count)
};

/* Type Table */
static const qmi_idl_type_table_entry  imsa_type_table_v01[] = {
  {sizeof(imsa_rat_handover_status_info_v01), imsa_rat_handover_status_info_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry imsa_message_table_v01[] = {
  {sizeof(imsa_get_registration_status_req_msg_v01), 0},
  {sizeof(imsa_get_registration_status_resp_msg_v01), imsa_get_registration_status_resp_msg_data_v01},
  {sizeof(imsa_get_service_status_req_msg_v01), 0},
  {sizeof(imsa_get_service_status_resp_msg_v01), imsa_get_service_status_resp_msg_data_v01},
  {sizeof(imsa_ind_reg_req_msg_v01), imsa_ind_reg_req_msg_data_v01},
  {sizeof(imsa_ind_reg_rsp_msg_v01), imsa_ind_reg_rsp_msg_data_v01},
  {sizeof(imsa_registration_status_ind_msg_v01), imsa_registration_status_ind_msg_data_v01},
  {sizeof(imsa_service_status_ind_msg_v01), imsa_service_status_ind_msg_data_v01},
  {sizeof(imsa_rat_handover_status_ind_msg_v01), imsa_rat_handover_status_ind_msg_data_v01},
  {sizeof(imsa_pdp_status_ind_msg_v01), imsa_pdp_status_ind_msg_data_v01},
  {sizeof(imsa_acs_status_ind_msg_v01), imsa_acs_status_ind_msg_data_v01},
  {sizeof(imsa_get_pdp_status_req_msg_v01), 0},
  {sizeof(imsa_get_pdp_status_resp_msg_v01), imsa_get_pdp_status_resp_msg_data_v01},
  {sizeof(imsa_lte_attach_parameters_ind_msg_v01), imsa_lte_attach_parameters_ind_msg_data_v01},
  {sizeof(imsa_get_lte_attach_parameters_req_msg_v01), 0},
  {sizeof(imsa_get_lte_attach_parameters_resp_msg_v01), imsa_get_lte_attach_parameters_resp_msg_data_v01},
  {sizeof(imsa_subscription_status_ind_msg_v01), imsa_subscription_status_ind_msg_data_v01},
  {sizeof(imsa_network_provisioning_status_ind_msg_v01), imsa_network_provisioning_status_ind_msg_data_v01},
  {sizeof(imsa_get_rtp_statistics_req_msg_v01), 0},
  {sizeof(imsa_get_rtp_statistics_resp_msg_v01), imsa_get_rtp_statistics_resp_msg_data_v01},
  {sizeof(imsa_rtp_statistics_ind_msg_v01), imsa_rtp_statistics_ind_msg_data_v01}
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
  sizeof(imsa_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(imsa_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  imsa_type_table_v01,
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
  {QMI_IMSA_IND_REG_REQ_V01, QMI_IDL_TYPE16(0, 4), 32},
  {QMI_IMSA_GET_PDP_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 11), 0},
  {QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_REQ_V01, QMI_IDL_TYPE16(0, 14), 0},
  {QMI_IMSA_GET_RTP_STATISTICS_REQ_V01, QMI_IDL_TYPE16(0, 18), 0}
};

static const qmi_idl_service_message_table_entry imsa_service_response_messages_v01[] = {
  {QMI_IMSA_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_IMSA_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01, QMI_IDL_TYPE16(0, 1), 288},
  {QMI_IMSA_GET_SERVICE_STATUS_RSP_V01, QMI_IDL_TYPE16(0, 3), 77},
  {QMI_IMSA_IND_REG_RSP_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_IMSA_GET_PDP_STATUS_RSP_V01, QMI_IDL_TYPE16(0, 12), 18},
  {QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_RSP_V01, QMI_IDL_TYPE16(0, 15), 11},
  {QMI_IMSA_GET_RTP_STATISTICS_RSP_V01, QMI_IDL_TYPE16(0, 19), 7}
};

static const qmi_idl_service_message_table_entry imsa_service_indication_messages_v01[] = {
  {QMI_IMSA_REGISTRATION_STATUS_IND_V01, QMI_IDL_TYPE16(0, 6), 281},
  {QMI_IMSA_SERVICE_STATUS_IND_V01, QMI_IDL_TYPE16(0, 7), 70},
  {QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01, QMI_IDL_TYPE16(0, 8), 141},
  {QMI_IMSA_PDP_STATUS_IND_V01, QMI_IDL_TYPE16(0, 9), 11},
  {QMI_IMSA_ACS_STATUS_IND_V01, QMI_IDL_TYPE16(0, 10), 12},
  {QMI_IMSA_LTE_ATTACH_PARAMETERS_IND_V01, QMI_IDL_TYPE16(0, 13), 4},
  {QMI_IMSA_SUBSCRIPTION_STATUS_IND_V01, QMI_IDL_TYPE16(0, 16), 144},
  {QMI_IMSA_NETWORK_PROVISIONING_STATUS_IND_V01, QMI_IDL_TYPE16(0, 17), 7},
  {QMI_IMSA_RTP_STATISTICS_IND_V01, QMI_IDL_TYPE16(0, 20), 22}
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
  0x13,
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

