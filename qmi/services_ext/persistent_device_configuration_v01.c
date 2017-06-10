/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        P E R S I S T E N T _ D E V I C E _ C O N F I G U R A T I O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the pdc service Data structures.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header: //components/rel/qmimsgs.mpss/3.4.1/pdc/src/persistent_device_configuration_v01.c#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Tue Jul 23 2013 (Spin 2)
   From IDL File: persistent_device_configuration_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "persistent_device_configuration_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t pdc_config_info_req_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_config_info_req_type_v01, config_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_config_info_req_type_v01, config_id),
  PDC_CONFIG_ID_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_config_info_req_type_v01, config_id) - QMI_IDL_OFFSET8(pdc_config_info_req_type_v01, config_id_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t pdc_config_info_resp_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_config_info_resp_type_v01, config_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_config_info_resp_type_v01, config_id),
  PDC_CONFIG_ID_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_config_info_resp_type_v01, config_id) - QMI_IDL_OFFSET8(pdc_config_info_resp_type_v01, config_id_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t pdc_load_config_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_id),
  PDC_CONFIG_ID_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_id) - QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_id_len),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, total_config_size),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_frame),
  ((PDC_CONFIG_FRAME_SIZE_MAX_V01) & 0xFF), ((PDC_CONFIG_FRAME_SIZE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_frame) - QMI_IDL_OFFSET8(pdc_load_config_info_type_v01, config_frame_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * pdc_reset_req_msg is empty
 * static const uint8_t pdc_reset_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t pdc_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_indication_register_req_msg_v01, reg_config_change) - QMI_IDL_OFFSET8(pdc_indication_register_req_msg_v01, reg_config_change_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_indication_register_req_msg_v01, reg_config_change)
};

static const uint8_t pdc_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_config_change_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_config_change_ind_msg_v01, new_config_info),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t pdc_get_selected_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_selected_config_req_msg_v01, config_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_selected_config_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_selected_config_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_selected_config_req_msg_v01, ind_token)
};

static const uint8_t pdc_get_selected_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_get_selected_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_get_selected_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, active_config_id) - QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, active_config_id_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, active_config_id),
  PDC_CONFIG_ID_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, active_config_id) - QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, active_config_id_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, pending_config_id) - QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, pending_config_id_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, pending_config_id),
  PDC_CONFIG_ID_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, pending_config_id) - QMI_IDL_OFFSET8(pdc_get_selected_config_ind_msg_v01, pending_config_id_len)
};

static const uint8_t pdc_set_selected_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_set_selected_config_req_msg_v01, new_config_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_set_selected_config_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_set_selected_config_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_set_selected_config_req_msg_v01, ind_token)
};

static const uint8_t pdc_set_selected_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_set_selected_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_set_selected_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_set_selected_config_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_set_selected_config_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_set_selected_config_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_set_selected_config_ind_msg_v01, ind_token)
};

static const uint8_t pdc_list_configs_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_list_configs_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_list_configs_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_list_configs_req_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_list_configs_req_msg_v01, config_type) - QMI_IDL_OFFSET8(pdc_list_configs_req_msg_v01, config_type_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_list_configs_req_msg_v01, config_type)
};

static const uint8_t pdc_list_configs_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_list_configs_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_list_configs_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, config_list) - QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, config_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, config_list),
  PDC_CONFIG_LIST_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, config_list) - QMI_IDL_OFFSET8(pdc_list_configs_ind_msg_v01, config_list_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t pdc_delete_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, config_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, config_id) - QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, config_id_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, config_id),
  PDC_CONFIG_ID_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, config_id) - QMI_IDL_OFFSET8(pdc_delete_config_req_msg_v01, config_id_len)
};

static const uint8_t pdc_delete_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_delete_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_delete_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_delete_config_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_delete_config_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_delete_config_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_delete_config_ind_msg_v01, ind_token)
};

static const uint8_t pdc_load_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_load_config_req_msg_v01, load_config_info),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(pdc_load_config_req_msg_v01, ind_token) - QMI_IDL_OFFSET16RELATIVE(pdc_load_config_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(pdc_load_config_req_msg_v01, ind_token)
};

static const uint8_t pdc_load_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_load_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_load_config_resp_msg_v01, frame_data_reset) - QMI_IDL_OFFSET8(pdc_load_config_resp_msg_v01, frame_data_reset_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_resp_msg_v01, frame_data_reset)
};

static const uint8_t pdc_load_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, received_config_size) - QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, received_config_size_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, received_config_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, remaining_config_size) - QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, remaining_config_size_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, remaining_config_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, frame_data_reset) - QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, frame_data_reset_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_load_config_ind_msg_v01, frame_data_reset)
};

static const uint8_t pdc_activate_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_activate_config_req_msg_v01, config_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_activate_config_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_activate_config_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_activate_config_req_msg_v01, ind_token)
};

static const uint8_t pdc_activate_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_activate_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_activate_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_activate_config_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_activate_config_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_activate_config_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_activate_config_ind_msg_v01, ind_token)
};

static const uint8_t pdc_get_config_info_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_get_config_info_req_msg_v01, new_config_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_info_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_config_info_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_info_req_msg_v01, ind_token)
};

static const uint8_t pdc_get_config_info_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_get_config_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_get_config_info_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_size) - QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_size_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_desc) - QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_desc_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_desc),
  PDC_CONFIG_DESC_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_desc) - QMI_IDL_OFFSET8(pdc_get_config_info_ind_msg_v01, config_desc_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(pdc_get_config_info_ind_msg_v01, config_version) - QMI_IDL_OFFSET16RELATIVE(pdc_get_config_info_ind_msg_v01, config_version_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(pdc_get_config_info_ind_msg_v01, config_version)
};

static const uint8_t pdc_get_config_limits_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_limits_req_msg_v01, config_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_limits_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_config_limits_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_limits_req_msg_v01, ind_token)
};

static const uint8_t pdc_get_config_limits_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_get_config_limits_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_get_config_limits_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, max_config_size) - QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, max_config_size_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, max_config_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, curr_config_size) - QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, curr_config_size_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(pdc_get_config_limits_ind_msg_v01, curr_config_size)
};

static const uint8_t pdc_get_default_config_info_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_req_msg_v01, config_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_default_config_info_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_default_config_info_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_req_msg_v01, ind_token)
};

static const uint8_t pdc_get_default_config_info_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_get_default_config_info_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_version) - QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_version_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_version),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_size) - QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_size_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_desc) - QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_desc_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_desc),
  PDC_CONFIG_DESC_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_desc) - QMI_IDL_OFFSET8(pdc_get_default_config_info_ind_msg_v01, config_desc_len)
};

static const uint8_t pdc_deactivate_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_deactivate_config_req_msg_v01, config_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_deactivate_config_req_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_deactivate_config_req_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_deactivate_config_req_msg_v01, ind_token)
};

static const uint8_t pdc_deactivate_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(pdc_deactivate_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t pdc_deactivate_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(pdc_deactivate_config_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(pdc_deactivate_config_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(pdc_deactivate_config_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(pdc_deactivate_config_ind_msg_v01, ind_token)
};

/* Type Table */
static const qmi_idl_type_table_entry  pdc_type_table_v01[] = {
  {sizeof(pdc_config_info_req_type_v01), pdc_config_info_req_type_data_v01},
  {sizeof(pdc_config_info_resp_type_v01), pdc_config_info_resp_type_data_v01},
  {sizeof(pdc_load_config_info_type_v01), pdc_load_config_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry pdc_message_table_v01[] = {
  {sizeof(pdc_reset_req_msg_v01), 0},
  {sizeof(pdc_reset_resp_msg_v01), pdc_reset_resp_msg_data_v01},
  {sizeof(pdc_indication_register_req_msg_v01), pdc_indication_register_req_msg_data_v01},
  {sizeof(pdc_indication_register_resp_msg_v01), pdc_indication_register_resp_msg_data_v01},
  {sizeof(pdc_config_change_ind_msg_v01), pdc_config_change_ind_msg_data_v01},
  {sizeof(pdc_get_selected_config_req_msg_v01), pdc_get_selected_config_req_msg_data_v01},
  {sizeof(pdc_get_selected_config_resp_msg_v01), pdc_get_selected_config_resp_msg_data_v01},
  {sizeof(pdc_get_selected_config_ind_msg_v01), pdc_get_selected_config_ind_msg_data_v01},
  {sizeof(pdc_set_selected_config_req_msg_v01), pdc_set_selected_config_req_msg_data_v01},
  {sizeof(pdc_set_selected_config_resp_msg_v01), pdc_set_selected_config_resp_msg_data_v01},
  {sizeof(pdc_set_selected_config_ind_msg_v01), pdc_set_selected_config_ind_msg_data_v01},
  {sizeof(pdc_list_configs_req_msg_v01), pdc_list_configs_req_msg_data_v01},
  {sizeof(pdc_list_configs_resp_msg_v01), pdc_list_configs_resp_msg_data_v01},
  {sizeof(pdc_list_configs_ind_msg_v01), pdc_list_configs_ind_msg_data_v01},
  {sizeof(pdc_delete_config_req_msg_v01), pdc_delete_config_req_msg_data_v01},
  {sizeof(pdc_delete_config_resp_msg_v01), pdc_delete_config_resp_msg_data_v01},
  {sizeof(pdc_delete_config_ind_msg_v01), pdc_delete_config_ind_msg_data_v01},
  {sizeof(pdc_load_config_req_msg_v01), pdc_load_config_req_msg_data_v01},
  {sizeof(pdc_load_config_resp_msg_v01), pdc_load_config_resp_msg_data_v01},
  {sizeof(pdc_load_config_ind_msg_v01), pdc_load_config_ind_msg_data_v01},
  {sizeof(pdc_activate_config_req_msg_v01), pdc_activate_config_req_msg_data_v01},
  {sizeof(pdc_activate_config_resp_msg_v01), pdc_activate_config_resp_msg_data_v01},
  {sizeof(pdc_activate_config_ind_msg_v01), pdc_activate_config_ind_msg_data_v01},
  {sizeof(pdc_get_config_info_req_msg_v01), pdc_get_config_info_req_msg_data_v01},
  {sizeof(pdc_get_config_info_resp_msg_v01), pdc_get_config_info_resp_msg_data_v01},
  {sizeof(pdc_get_config_info_ind_msg_v01), pdc_get_config_info_ind_msg_data_v01},
  {sizeof(pdc_get_config_limits_req_msg_v01), pdc_get_config_limits_req_msg_data_v01},
  {sizeof(pdc_get_config_limits_resp_msg_v01), pdc_get_config_limits_resp_msg_data_v01},
  {sizeof(pdc_get_config_limits_ind_msg_v01), pdc_get_config_limits_ind_msg_data_v01},
  {sizeof(pdc_get_default_config_info_req_msg_v01), pdc_get_default_config_info_req_msg_data_v01},
  {sizeof(pdc_get_default_config_info_resp_msg_v01), pdc_get_default_config_info_resp_msg_data_v01},
  {sizeof(pdc_get_default_config_info_ind_msg_v01), pdc_get_default_config_info_ind_msg_data_v01},
  {sizeof(pdc_deactivate_config_req_msg_v01), pdc_deactivate_config_req_msg_data_v01},
  {sizeof(pdc_deactivate_config_resp_msg_v01), pdc_deactivate_config_resp_msg_data_v01},
  {sizeof(pdc_deactivate_config_ind_msg_v01), pdc_deactivate_config_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object pdc_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *pdc_qmi_idl_type_table_object_referenced_tables_v01[] =
{&pdc_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object pdc_qmi_idl_type_table_object_v01 = {
  sizeof(pdc_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(pdc_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  pdc_type_table_v01,
  pdc_message_table_v01,
  pdc_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry pdc_service_command_messages_v01[] = {
  {QMI_PDC_RESET_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_PDC_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_PDC_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_PDC_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 2), 4},
  {QMI_PDC_GET_SELECTED_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 5), 14},
  {QMI_PDC_SET_SELECTED_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 8), 139},
  {QMI_PDC_LIST_CONFIGS_REQ_V01, QMI_IDL_TYPE16(0, 11), 14},
  {QMI_PDC_DELETE_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 14), 142},
  {QMI_PDC_LOAD_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 17), 32913},
  {QMI_PDC_ACTIVATE_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 20), 14},
  {QMI_PDC_GET_CONFIG_INFO_REQ_V01, QMI_IDL_TYPE16(0, 23), 139},
  {QMI_PDC_GET_CONFIG_LIMITS_REQ_V01, QMI_IDL_TYPE16(0, 26), 14},
  {QMI_PDC_GET_DEFAULT_CONFIG_INFO_REQ_V01, QMI_IDL_TYPE16(0, 29), 14},
  {QMI_PDC_DEACTIVATE_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 32), 14}
};

static const qmi_idl_service_message_table_entry pdc_service_response_messages_v01[] = {
  {QMI_PDC_RESET_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_PDC_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_PDC_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_PDC_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_PDC_GET_SELECTED_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 6), 7},
  {QMI_PDC_SET_SELECTED_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 9), 7},
  {QMI_PDC_LIST_CONFIGS_RESP_V01, QMI_IDL_TYPE16(0, 12), 7},
  {QMI_PDC_DELETE_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 15), 7},
  {QMI_PDC_LOAD_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 18), 11},
  {QMI_PDC_ACTIVATE_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 21), 7},
  {QMI_PDC_GET_CONFIG_INFO_RESP_V01, QMI_IDL_TYPE16(0, 24), 7},
  {QMI_PDC_GET_CONFIG_LIMITS_RESP_V01, QMI_IDL_TYPE16(0, 27), 7},
  {QMI_PDC_GET_DEFAULT_CONFIG_INFO_RESP_V01, QMI_IDL_TYPE16(0, 30), 7},
  {QMI_PDC_DEACTIVATE_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 33), 7}
};

static const qmi_idl_service_message_table_entry pdc_service_indication_messages_v01[] = {
  {QMI_PDC_CONFIG_CHANGE_IND_V01, QMI_IDL_TYPE16(0, 4), 132},
  {QMI_PDC_GET_SELECTED_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 7), 268},
  {QMI_PDC_SET_SELECTED_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 10), 12},
  {QMI_PDC_LIST_CONFIGS_IND_V01, QMI_IDL_TYPE16(0, 13), 3241},
  {QMI_PDC_DELETE_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 16), 12},
  {QMI_PDC_LOAD_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 19), 30},
  {QMI_PDC_ACTIVATE_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 22), 12},
  {QMI_PDC_GET_CONFIG_INFO_IND_V01, QMI_IDL_TYPE16(0, 25), 285},
  {QMI_PDC_GET_CONFIG_LIMITS_IND_V01, QMI_IDL_TYPE16(0, 28), 34},
  {QMI_PDC_GET_DEFAULT_CONFIG_INFO_IND_V01, QMI_IDL_TYPE16(0, 31), 285},
  {QMI_PDC_DEACTIVATE_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 34), 12}
};

/*Service Object*/
struct qmi_idl_service_object pdc_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x24,
  32913,
  { sizeof(pdc_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(pdc_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(pdc_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { pdc_service_command_messages_v01, pdc_service_response_messages_v01, pdc_service_indication_messages_v01},
  &pdc_qmi_idl_type_table_object_v01,
  0x05,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type pdc_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( PDC_V01_IDL_MAJOR_VERS != idl_maj_version || PDC_V01_IDL_MINOR_VERS != idl_min_version 
       || PDC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&pdc_qmi_idl_service_object_v01;
}

