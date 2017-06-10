/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        I P _ M U L T I M E D I A _ S U B S Y S T E M _ P R E S E N C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the imsp service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/imsp/main/latest/src/ip_multimedia_subsystem_presence_v01.c#3 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.0
   It requires encode/decode library version 4 or later
   It was generated on: Fri Mar  2 2012
   From IDL File: ip_multimedia_subsystem_presence_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "ip_multimedia_subsystem_presence_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t imsp_presence_service_description_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_service_description_v01, description),
  IMSP_MAX_SERVICE_DESC_STR_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_service_description_v01, ver),
  IMSP_MAX_VERSION_STR_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_service_description_v01, service_id),
  IMSP_MAX_SERVICE_ID_STR_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_service_capabilities_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_presence_service_capabilities_v01, is_audio_supported),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_presence_service_capabilities_v01, audio_capability),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_presence_service_capabilities_v01, is_video_supported),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_presence_service_capabilities_v01, video_capability),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_info_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_info_v01, contact_uri),
  IMSP_MAX_URI_STR_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_info_v01, service_descriptions),
 0, 0,
  QMI_IDL_FLAGS_OFFSET_IS_16 |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_info_v01, service_capabilities),
 1, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_user_info_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_user_info_v01, imsp_user_uri),
  IMSP_MAX_URI_STR_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_info_with_ts_data_v01[] = {
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_presence_info_with_ts_v01, presence_info),
 2, 0,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_info_with_ts_v01, timestamp_of_presence_info),
  IMSP_MAX_TIMESTAMP_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_list_info_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_list_info_v01, list_contact_uri),
  IMSP_MAX_URI_STR_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_list_info_v01, list_name),
  IMSP_MAX_LIST_NAME_STR_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_list_info_v01, list_version),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_list_info_v01, full_state),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_resource_instance_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_resource_instance_v01, resource_id),
  IMSP_MAX_RESOURCE_INSTANCE_ID_STR_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_resource_instance_v01, resource_state),
  IMSP_MAX_RESOURCE_INSTANCE_STATE_STR_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_resource_instance_v01, resource_reason),
  IMSP_MAX_RESOURCE_INSTANCE_REASON_STR_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_resource_instance_v01, resource_cid),
  ((IMSP_MAX_RESOURCE_INSTANCE_CID_STR_LEN_V01) & 0xFF), ((IMSP_MAX_RESOURCE_INSTANCE_CID_STR_LEN_V01) >> 8),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_user_info_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_presence_user_info_v01, resource_uri),
  IMSP_MAX_URI_STR_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_user_info_v01, is_volte_contact),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_user_info_v01, publish_status),

  QMI_IDL_FLAGS_OFFSET_IS_16 |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_user_info_v01, resource_instance),
 6, 0,
  QMI_IDL_FLAGS_OFFSET_IS_16 |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_user_info_v01, presence_user_info),
 4, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsp_presence_notify_rich_info_data_v01[] = {
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_presence_notify_rich_info_v01, list_info),
 5, 0,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsp_presence_notify_rich_info_v01, user_list_info),
  IMSP_MAX_RECORD_IN_NOTIFY_V01,
  QMI_IDL_OFFSET16RELATIVE(imsp_presence_notify_rich_info_v01, user_list_info) - QMI_IDL_OFFSET16RELATIVE(imsp_presence_notify_rich_info_v01, user_list_info_len),
 7, 0,
  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * imsp_get_enabler_state_req is empty
 * static const uint8_t imsp_get_enabler_state_req_data_v01[] = {
 * };
 */
  
static const uint8_t imsp_get_enabler_state_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_get_enabler_state_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_get_enabler_state_resp_v01, enabler_state) - QMI_IDL_OFFSET8(imsp_get_enabler_state_resp_v01, enabler_state_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_get_enabler_state_resp_v01, enabler_state)
};

static const uint8_t imsp_enabler_state_ind_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_enabler_state_ind_v01, enabler_state)
};

static const uint8_t imsp_publish_trigger_ind_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_publish_trigger_ind_v01, publish_trigger)
};

static const uint8_t imsp_send_publish_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_req_v01, publish_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_publish_req_v01, presence_info) - QMI_IDL_OFFSET8(imsp_send_publish_req_v01, presence_info_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_publish_req_v01, presence_info),
  2, 0
};

static const uint8_t imsp_send_publish_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_publish_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_publish_resp_v01, imsp_publish_callid) - QMI_IDL_OFFSET8(imsp_send_publish_resp_v01, imsp_publish_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_resp_v01, imsp_publish_callid)
};

static const uint8_t imsp_send_publish_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_ind_v01, status_response),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_ind_v01, imsp_publish_callid),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_publish_ind_v01, imsp_sip_resp_code) - QMI_IDL_OFFSET8(imsp_send_publish_ind_v01, imsp_sip_resp_code_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_ind_v01, imsp_sip_resp_code)
};

static const uint8_t imsp_send_publish_xml_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_send_publish_xml_req_v01, imsp_rich_publish_xml),
  ((IMSP_MAX_PRESENCE_XML_STR_LEN_V01) & 0xFF), ((IMSP_MAX_PRESENCE_XML_STR_LEN_V01) >> 8)
};

static const uint8_t imsp_send_publish_xml_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_publish_xml_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_publish_xml_resp_v01, imsp_publish_callid) - QMI_IDL_OFFSET8(imsp_send_publish_xml_resp_v01, imsp_publish_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_xml_resp_v01, imsp_publish_callid)
};

static const uint8_t imsp_send_publish_xml_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_xml_ind_v01, status_response),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_xml_ind_v01, imsp_publish_callid),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_publish_xml_ind_v01, imsp_sip_resp_code) - QMI_IDL_OFFSET8(imsp_send_publish_xml_ind_v01, imsp_sip_resp_code_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_publish_xml_ind_v01, imsp_sip_resp_code)
};

static const uint8_t imsp_send_unpublish_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_unpublish_req_v01, imsp_publish_callid) - QMI_IDL_OFFSET8(imsp_send_unpublish_req_v01, imsp_publish_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unpublish_req_v01, imsp_publish_callid)
};

static const uint8_t imsp_send_unpublish_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_unpublish_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_unpublish_resp_v01, imsp_unpublish_callid) - QMI_IDL_OFFSET8(imsp_send_unpublish_resp_v01, imsp_unpublish_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unpublish_resp_v01, imsp_unpublish_callid)
};

static const uint8_t imsp_send_unpublish_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unpublish_ind_v01, status_response),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unpublish_ind_v01, imsp_unpublish_callid),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_unpublish_ind_v01, imsp_sip_resp_code) - QMI_IDL_OFFSET8(imsp_send_unpublish_ind_v01, imsp_sip_resp_code_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unpublish_ind_v01, imsp_sip_resp_code)
};

static const uint8_t imsp_send_subscribe_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_req_v01, subscription_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_req_v01, subscribe_user_list),
  IMSP_MAX_SUBSCRIBE_USER_COUNT_V01,
  QMI_IDL_OFFSET8(imsp_send_subscribe_req_v01, subscribe_user_list) - QMI_IDL_OFFSET8(imsp_send_subscribe_req_v01, subscribe_user_list_len),
  3, 0
};

static const uint8_t imsp_send_subscribe_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_subscribe_resp_v01, imsp_subscribe_callid) - QMI_IDL_OFFSET8(imsp_send_subscribe_resp_v01, imsp_subscribe_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_resp_v01, imsp_subscribe_callid)
};

static const uint8_t imsp_send_subscribe_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_ind_v01, status_response),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_ind_v01, imsp_subscribe_callid),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_subscribe_ind_v01, imsp_sip_resp_code) - QMI_IDL_OFFSET8(imsp_send_subscribe_ind_v01, imsp_sip_resp_code_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_ind_v01, imsp_sip_resp_code)
};

static const uint8_t imsp_send_subscribe_xml_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_send_subscribe_xml_req_v01, imsp_user_list_xml),
  ((IMSP_MAX_PRESENCE_XML_STR_LEN_V01) & 0xFF), ((IMSP_MAX_PRESENCE_XML_STR_LEN_V01) >> 8)
};

static const uint8_t imsp_send_subscribe_xml_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_xml_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_subscribe_xml_resp_v01, imsp_subscribe_callid) - QMI_IDL_OFFSET8(imsp_send_subscribe_xml_resp_v01, imsp_subscribe_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_xml_resp_v01, imsp_subscribe_callid)
};

static const uint8_t imsp_send_subscribe_xml_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_xml_ind_v01, status_response),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_xml_ind_v01, imsp_subscribe_callid),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_subscribe_xml_ind_v01, imsp_sip_resp_code) - QMI_IDL_OFFSET8(imsp_send_subscribe_xml_ind_v01, imsp_sip_resp_code_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_subscribe_xml_ind_v01, imsp_sip_resp_code)
};

static const uint8_t imsp_send_unsubscribe_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unsubscribe_req_v01, imsp_subscribe_callid)
};

static const uint8_t imsp_send_unsubscribe_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_send_unsubscribe_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_unsubscribe_resp_v01, imsp_unsubscribe_callid) - QMI_IDL_OFFSET8(imsp_send_unsubscribe_resp_v01, imsp_unsubscribe_callid_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unsubscribe_resp_v01, imsp_unsubscribe_callid)
};

static const uint8_t imsp_send_unsubscribe_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unsubscribe_ind_v01, status_response),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unsubscribe_ind_v01, imsp_unsubscribe_callid),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_send_unsubscribe_ind_v01, imsp_sip_resp_code) - QMI_IDL_OFFSET8(imsp_send_unsubscribe_ind_v01, imsp_sip_resp_code_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsp_send_unsubscribe_ind_v01, imsp_sip_resp_code)
};

static const uint8_t imsp_notify_xml_ind_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsp_notify_xml_ind_v01, imsp_notify_details),
  ((IMSP_MAX_PRESENCE_XML_STR_LEN_V01) & 0xFF), ((IMSP_MAX_PRESENCE_XML_STR_LEN_V01) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsp_notify_xml_ind_v01, imsp_subscribe_callid) - QMI_IDL_OFFSET16RELATIVE(imsp_notify_xml_ind_v01, imsp_subscribe_callid_valid)),
  0x10,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsp_notify_xml_ind_v01, imsp_subscribe_callid)
};

static const uint8_t imsp_notify_ind_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_notify_ind_v01, rich_info),
  8, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsp_notify_ind_v01, imsp_subscribe_callid) - QMI_IDL_OFFSET16RELATIVE(imsp_notify_ind_v01, imsp_subscribe_callid_valid)),
  0x10,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsp_notify_ind_v01, imsp_subscribe_callid)
};

static const uint8_t imsp_set_notify_fmt_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_set_notify_fmt_req_v01, update_with_struct_info)
};

static const uint8_t imsp_set_notify_fmt_resp_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_set_notify_fmt_resp_v01, resp),
  0, 1
};

/* 
 * imsp_get_notify_fmt_req is empty
 * static const uint8_t imsp_get_notify_fmt_req_data_v01[] = {
 * };
 */
  
static const uint8_t imsp_get_notify_fmt_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_get_notify_fmt_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_get_notify_fmt_resp_v01, update_with_struct_info) - QMI_IDL_OFFSET8(imsp_get_notify_fmt_resp_v01, update_with_struct_info_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsp_get_notify_fmt_resp_v01, update_with_struct_info)
};

static const uint8_t imsp_set_event_report_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsp_set_event_report_req_v01, event_report_bit_masks)
};

static const uint8_t imsp_set_event_report_resp_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_set_event_report_resp_v01, resp),
  0, 1
};

/* 
 * imsp_get_event_report_req is empty
 * static const uint8_t imsp_get_event_report_req_data_v01[] = {
 * };
 */
  
static const uint8_t imsp_get_event_report_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsp_get_event_report_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsp_get_event_report_resp_v01, event_report_bit_masks) - QMI_IDL_OFFSET8(imsp_get_event_report_resp_v01, event_report_bit_masks_valid)),
  0x10,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsp_get_event_report_resp_v01, event_report_bit_masks)
};

/* Type Table */
static const qmi_idl_type_table_entry  imsp_type_table_v01[] = {
  {sizeof(imsp_presence_service_description_v01), imsp_presence_service_description_data_v01},
  {sizeof(imsp_presence_service_capabilities_v01), imsp_presence_service_capabilities_data_v01},
  {sizeof(imsp_presence_info_v01), imsp_presence_info_data_v01},
  {sizeof(imsp_user_info_v01), imsp_user_info_data_v01},
  {sizeof(imsp_presence_info_with_ts_v01), imsp_presence_info_with_ts_data_v01},
  {sizeof(imsp_presence_list_info_v01), imsp_presence_list_info_data_v01},
  {sizeof(imsp_presence_resource_instance_v01), imsp_presence_resource_instance_data_v01},
  {sizeof(imsp_presence_user_info_v01), imsp_presence_user_info_data_v01},
  {sizeof(imsp_presence_notify_rich_info_v01), imsp_presence_notify_rich_info_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry imsp_message_table_v01[] = {
  {0, 0},
  {sizeof(imsp_get_enabler_state_resp_v01), imsp_get_enabler_state_resp_data_v01},
  {sizeof(imsp_enabler_state_ind_v01), imsp_enabler_state_ind_data_v01},
  {sizeof(imsp_publish_trigger_ind_v01), imsp_publish_trigger_ind_data_v01},
  {sizeof(imsp_send_publish_req_v01), imsp_send_publish_req_data_v01},
  {sizeof(imsp_send_publish_resp_v01), imsp_send_publish_resp_data_v01},
  {sizeof(imsp_send_publish_ind_v01), imsp_send_publish_ind_data_v01},
  {sizeof(imsp_send_publish_xml_req_v01), imsp_send_publish_xml_req_data_v01},
  {sizeof(imsp_send_publish_xml_resp_v01), imsp_send_publish_xml_resp_data_v01},
  {sizeof(imsp_send_publish_xml_ind_v01), imsp_send_publish_xml_ind_data_v01},
  {sizeof(imsp_send_unpublish_req_v01), imsp_send_unpublish_req_data_v01},
  {sizeof(imsp_send_unpublish_resp_v01), imsp_send_unpublish_resp_data_v01},
  {sizeof(imsp_send_unpublish_ind_v01), imsp_send_unpublish_ind_data_v01},
  {sizeof(imsp_send_subscribe_req_v01), imsp_send_subscribe_req_data_v01},
  {sizeof(imsp_send_subscribe_resp_v01), imsp_send_subscribe_resp_data_v01},
  {sizeof(imsp_send_subscribe_ind_v01), imsp_send_subscribe_ind_data_v01},
  {sizeof(imsp_send_subscribe_xml_req_v01), imsp_send_subscribe_xml_req_data_v01},
  {sizeof(imsp_send_subscribe_xml_resp_v01), imsp_send_subscribe_xml_resp_data_v01},
  {sizeof(imsp_send_subscribe_xml_ind_v01), imsp_send_subscribe_xml_ind_data_v01},
  {sizeof(imsp_send_unsubscribe_req_v01), imsp_send_unsubscribe_req_data_v01},
  {sizeof(imsp_send_unsubscribe_resp_v01), imsp_send_unsubscribe_resp_data_v01},
  {sizeof(imsp_send_unsubscribe_ind_v01), imsp_send_unsubscribe_ind_data_v01},
  {sizeof(imsp_notify_xml_ind_v01), imsp_notify_xml_ind_data_v01},
  {sizeof(imsp_notify_ind_v01), imsp_notify_ind_data_v01},
  {sizeof(imsp_set_notify_fmt_req_v01), imsp_set_notify_fmt_req_data_v01},
  {sizeof(imsp_set_notify_fmt_resp_v01), imsp_set_notify_fmt_resp_data_v01},
  {0, 0},
  {sizeof(imsp_get_notify_fmt_resp_v01), imsp_get_notify_fmt_resp_data_v01},
  {sizeof(imsp_set_event_report_req_v01), imsp_set_event_report_req_data_v01},
  {sizeof(imsp_set_event_report_resp_v01), imsp_set_event_report_resp_data_v01},
  {0, 0},
  {sizeof(imsp_get_event_report_resp_v01), imsp_get_event_report_resp_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object imsp_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *imsp_qmi_idl_type_table_object_referenced_tables_v01[] =
{&imsp_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object imsp_qmi_idl_type_table_object_v01 = {
  sizeof(imsp_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(imsp_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  imsp_type_table_v01,
  imsp_message_table_v01,
  imsp_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry imsp_service_command_messages_v01[] = {
  {QMI_IMSP_GET_ENABLER_STATE_REQ_V01, TYPE16(0, 0), 0},
  {QMI_IMSP_SEND_PUBLISH_REQ_V01, TYPE16(0, 4), 380},
  {QMI_IMSP_SEND_PUBLISH_XML_REQ_V01, TYPE16(0, 7), 4098},
  {QMI_IMSP_SEND_UNPUBLISH_REQ_V01, TYPE16(0, 10), 7},
  {QMI_IMSP_SEND_SUBSCRIBE_REQ_V01, TYPE16(0, 13), 4107},
  {QMI_IMSP_SEND_SUBSCRIBE_XML_REQ_V01, TYPE16(0, 16), 4098},
  {QMI_IMSP_SEND_UNSUBSCRIBE_REQ_V01, TYPE16(0, 19), 7},
  {QMI_IMSP_SET_NOTIFY_FMT_REQ_V01, TYPE16(0, 24), 4},
  {QMI_IMSP_GET_NOTIFY_FMT_REQ_V01, TYPE16(0, 26), 0},
  {QMI_IMSP_SET_EVENT_REPORT_REQ_V01, TYPE16(0, 28), 11},
  {QMI_IMSP_GET_EVENT_REPORT_REQ_V01, TYPE16(0, 30), 0}
};

static const qmi_idl_service_message_table_entry imsp_service_response_messages_v01[] = {
  {QMI_IMSP_GET_ENABLER_STATE_RESP_V01, TYPE16(0, 1), 14},
  {QMI_IMSP_SEND_PUBLISH_RESP_V01, TYPE16(0, 5), 14},
  {QMI_IMSP_SEND_PUBLISH_XML_RESP_V01, TYPE16(0, 8), 14},
  {QMI_IMSP_SEND_UNPUBLISH_RESP_V01, TYPE16(0, 11), 14},
  {QMI_IMSP_SEND_SUBSCRIBE_RESP_V01, TYPE16(0, 14), 14},
  {QMI_IMSP_SEND_SUBSCRIBE_XML_RESP_V01, TYPE16(0, 17), 14},
  {QMI_IMSP_SEND_UNSUBSCRIBE_RESP_V01, TYPE16(0, 20), 14},
  {QMI_IMSP_SET_NOTIFY_FMT_RESP_V01, TYPE16(0, 25), 7},
  {QMI_IMSP_GET_NOTIFY_FMT_RESP_V01, TYPE16(0, 27), 11},
  {QMI_IMSP_SET_EVENT_REPORT_RESP_V01, TYPE16(0, 29), 7},
  {QMI_IMSP_GET_EVENT_REPORT_RESP_V01, TYPE16(0, 31), 18}
};

static const qmi_idl_service_message_table_entry imsp_service_indication_messages_v01[] = {
  {QMI_IMSP_PUBLISH_TRIGGER_IND_V01, TYPE16(0, 3), 7},
  {QMI_IMSP_NOTIFY_XML_IND_V01, TYPE16(0, 22), 4105},
  {QMI_IMSP_NOTIFY_IND_V01, TYPE16(0, 23), 3432},
  {QMI_IMSP_ENABLER_STATE_IND_V01, TYPE16(0, 2), 7},
  {QMI_IMSP_SEND_PUBLISH_IND_V01, TYPE16(0, 6), 18},
  {QMI_IMSP_SEND_PUBLISH_XML_IND_V01, TYPE16(0, 9), 18},
  {QMI_IMSP_SEND_UNPUBLISH_IND_V01, TYPE16(0, 12), 18},
  {QMI_IMSP_SEND_SUBSCRIBE_IND_V01, TYPE16(0, 15), 18},
  {QMI_IMSP_SEND_SUBSCRIBE_XML_IND_V01, TYPE16(0, 18), 18},
  {QMI_IMSP_SEND_UNSUBSCRIBE_IND_V01, TYPE16(0, 21), 18}
};

/*Service Object*/
const struct qmi_idl_service_object imsp_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  0x1F,
  4107,
  { sizeof(imsp_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsp_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsp_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { imsp_service_command_messages_v01, imsp_service_response_messages_v01, imsp_service_indication_messages_v01},
  &imsp_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type imsp_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMSP_V01_IDL_MAJOR_VERS != idl_maj_version || IMSP_V01_IDL_MINOR_VERS != idl_min_version 
       || IMSP_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&imsp_qmi_idl_service_object_v01;
}

