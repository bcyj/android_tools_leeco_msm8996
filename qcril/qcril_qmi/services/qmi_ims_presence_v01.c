/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q M I _ I M S _ P R E S E N C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the qmi_ims_presence service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.7
   It was generated on: Mon Oct 10 2011
   From IDL File: qmi_ims_presence_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_ims_presence_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/* 
 * ims_enabler_state_req is empty
 * static const uint8_t ims_enabler_state_req_data_v01[] = {
 * };
 */
  
static const uint8_t ims_enabler_state_resp_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_enabler_state_resp_v01, enabler_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_enabler_state_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_enabler_state_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_enabler_state_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_enabler_state_ind_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_enabler_state_ind_v01, enabler_state)
};

static const uint8_t ims_send_simple_publish_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_simple_publish_req_v01, simplePublish),
  SIMPLE_XML_MAX_V01
};

static const uint8_t ims_send_simple_publish_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_send_simple_publish_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_simple_publish_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_send_simple_publish_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_simple_publish_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_send_publish_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_publish_req_v01, publish_xml),
  ((PRESENCE_XML_MAX_V01) & 0xFF), ((PRESENCE_XML_MAX_V01) >> 8)
};

static const uint8_t ims_send_publish_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_send_publish_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_publish_resp_v01, sipMessageID) - QMI_IDL_OFFSET8(ims_send_publish_resp_v01, sipMessageID_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_send_publish_resp_v01, sipMessageID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_publish_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_send_publish_resp_v01, response_details_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_publish_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

/* 
 * ims_send_unpublish_req is empty
 * static const uint8_t ims_send_unpublish_req_data_v01[] = {
 * };
 */
  
static const uint8_t ims_send_unpublish_resp_data_v01[] = {
  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, sipMessageID) - QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, sipMessageID_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, sipMessageID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, response_details_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_unpublish_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_send_subscribe_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_send_subscribe_req_v01, subscription_type),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_subscribe_req_v01, peerURI),
  PEER_URI_MAX_V01,

  0x03,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(ims_send_subscribe_req_v01, isRLSSubscription),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_send_subscribe_req_v01, userList) - QMI_IDL_OFFSET16RELATIVE(ims_send_subscribe_req_v01, userList_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_send_subscribe_req_v01, userList),
  ((PRESENCE_XML_MAX_V01) & 0xFF), ((PRESENCE_XML_MAX_V01) >> 8)
};

static const uint8_t ims_send_subscribe_resp_data_v01[] = {
  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, sipMessageID) - QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, sipMessageID_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, sipMessageID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, response_details_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_subscribe_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_send_unsubscribe_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_unsubscribe_req_v01, peerURI),
  PEER_URI_MAX_V01
};

static const uint8_t ims_send_unsubscribe_resp_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_send_unsubscribe_resp_v01, sipMessageID),

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_send_unsubscribe_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_send_unsubscribe_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_send_unsubscribe_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_unsubscribe_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_sip_msg_status_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_sip_msg_status_ind_v01, status_response),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_sip_msg_status_ind_v01, notifyDetails) - QMI_IDL_OFFSET8(ims_sip_msg_status_ind_v01, notifyDetails_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_sip_msg_status_ind_v01, notifyDetails),
  ((PRESENCE_XML_MAX_V01) & 0xFF), ((PRESENCE_XML_MAX_V01) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_sip_msg_status_ind_v01, response_details) - QMI_IDL_OFFSET16RELATIVE(ims_sip_msg_status_ind_v01, response_details_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_sip_msg_status_ind_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_http_msg_status_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_http_msg_status_ind_v01, status_response),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_http_msg_status_ind_v01, httpResponse) - QMI_IDL_OFFSET8(ims_http_msg_status_ind_v01, httpResponse_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_http_msg_status_ind_v01, httpResponse),
  ((PRESENCE_XML_MAX_V01) & 0xFF), ((PRESENCE_XML_MAX_V01) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_http_msg_status_ind_v01, response_details) - QMI_IDL_OFFSET16RELATIVE(ims_http_msg_status_ind_v01, response_details_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_http_msg_status_ind_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_add_par_rule_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_add_par_rule_req_v01, parID),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_add_par_rule_req_v01, parRuleID),
  PAR_RULE_ID_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_add_par_rule_req_v01, parRule),
  PAR_RULE_MAX_V01
};

static const uint8_t ims_add_par_rule_resp_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_add_par_rule_resp_v01, parID),

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_add_par_rule_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_add_par_rule_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_add_par_rule_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_add_par_rule_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_get_par_rule_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_get_par_rule_req_v01, parID)
};

static const uint8_t ims_get_par_rule_resp_data_v01[] = {
  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, resp),
  0, 1,

  0x04,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, parID),

  0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, parRuleID),
  PAR_RULE_ID_MAX_V01,

  0x06,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, parRule),
  PAR_RULE_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_get_par_rule_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_get_par_rule_at_index_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_req_v01, parID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_req_v01, index)
};

static const uint8_t ims_get_par_rule_at_index_resp_data_v01[] = {
  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, resp),
  0, 1,

  0x04,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, parID),

  0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, parRuleID),
  PAR_RULE_ID_MAX_V01,

  0x06,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, parRule),
  PAR_RULE_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_get_par_rule_at_index_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_del_par_rule_at_index_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_rule_at_index_req_v01, parID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_rule_at_index_req_v01, index)
};

static const uint8_t ims_del_par_rule_at_index_resp_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_rule_at_index_resp_v01, parID),

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_del_par_rule_at_index_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_del_par_rule_at_index_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_del_par_rule_at_index_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_del_par_rule_at_index_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_del_par_rule_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_rule_req_v01, parID)
};

static const uint8_t ims_del_par_rule_resp_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_rule_resp_v01, parID),

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_del_par_rule_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_del_par_rule_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_del_par_rule_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_del_par_rule_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_del_par_document_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_document_req_v01, parID)
};

static const uint8_t ims_del_par_document_resp_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_del_par_document_resp_v01, parID),

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_del_par_document_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_del_par_document_resp_v01, response_details) - QMI_IDL_OFFSET8(ims_del_par_document_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_del_par_document_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

static const uint8_t ims_send_par_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_send_par_req_v01, method_name),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_par_req_v01, requestXML),
  ((PRESENCE_XML_MAX_V01) & 0xFF), ((PRESENCE_XML_MAX_V01) >> 8)
};

static const uint8_t ims_send_par_resp_data_v01[] = {
  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_send_par_resp_v01, resp),
  0, 1,

  0x04,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_send_par_resp_v01, httpMessageID),

  0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_par_resp_v01, httpMessage),
  ((PRESENCE_XML_MAX_V01) & 0xFF), ((PRESENCE_XML_MAX_V01) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_send_par_resp_v01, response_details) - QMI_IDL_OFFSET16RELATIVE(ims_send_par_resp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_send_par_resp_v01, response_details),
  ((RESPONSE_DETAILS_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_MAX_V01) >> 8)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry qmi_ims_presence_message_table_v01[] = {
  {0, 0},
  {sizeof(ims_enabler_state_resp_v01), ims_enabler_state_resp_data_v01},
  {sizeof(ims_enabler_state_ind_v01), ims_enabler_state_ind_data_v01},
  {sizeof(ims_send_simple_publish_req_v01), ims_send_simple_publish_req_data_v01},
  {sizeof(ims_send_simple_publish_resp_v01), ims_send_simple_publish_resp_data_v01},
  {sizeof(ims_send_publish_req_v01), ims_send_publish_req_data_v01},
  {sizeof(ims_send_publish_resp_v01), ims_send_publish_resp_data_v01},
  {0, 0},
  {sizeof(ims_send_unpublish_resp_v01), ims_send_unpublish_resp_data_v01},
  {sizeof(ims_send_subscribe_req_v01), ims_send_subscribe_req_data_v01},
  {sizeof(ims_send_subscribe_resp_v01), ims_send_subscribe_resp_data_v01},
  {sizeof(ims_send_unsubscribe_req_v01), ims_send_unsubscribe_req_data_v01},
  {sizeof(ims_send_unsubscribe_resp_v01), ims_send_unsubscribe_resp_data_v01},
  {sizeof(ims_sip_msg_status_ind_v01), ims_sip_msg_status_ind_data_v01},
  {sizeof(ims_http_msg_status_ind_v01), ims_http_msg_status_ind_data_v01},
  {sizeof(ims_add_par_rule_req_v01), ims_add_par_rule_req_data_v01},
  {sizeof(ims_add_par_rule_resp_v01), ims_add_par_rule_resp_data_v01},
  {sizeof(ims_get_par_rule_req_v01), ims_get_par_rule_req_data_v01},
  {sizeof(ims_get_par_rule_resp_v01), ims_get_par_rule_resp_data_v01},
  {sizeof(ims_get_par_rule_at_index_req_v01), ims_get_par_rule_at_index_req_data_v01},
  {sizeof(ims_get_par_rule_at_index_resp_v01), ims_get_par_rule_at_index_resp_data_v01},
  {sizeof(ims_del_par_rule_at_index_req_v01), ims_del_par_rule_at_index_req_data_v01},
  {sizeof(ims_del_par_rule_at_index_resp_v01), ims_del_par_rule_at_index_resp_data_v01},
  {sizeof(ims_del_par_rule_req_v01), ims_del_par_rule_req_data_v01},
  {sizeof(ims_del_par_rule_resp_v01), ims_del_par_rule_resp_data_v01},
  {sizeof(ims_del_par_document_req_v01), ims_del_par_document_req_data_v01},
  {sizeof(ims_del_par_document_resp_v01), ims_del_par_document_resp_data_v01},
  {sizeof(ims_send_par_req_v01), ims_send_par_req_data_v01},
  {sizeof(ims_send_par_resp_v01), ims_send_par_resp_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object qmi_ims_presence_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *qmi_ims_presence_qmi_idl_type_table_object_referenced_tables_v01[] =
{&qmi_ims_presence_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object qmi_ims_presence_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(qmi_ims_presence_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  qmi_ims_presence_message_table_v01,
  qmi_ims_presence_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry qmi_ims_presence_service_command_messages_v01[] = {
  {IMS_ENABLER_STATE_REQ_V01, TYPE16(0, 0), 0},
  {IMS_SEND_SIMPLE_PUBLISH_REQ_V01, TYPE16(0, 3), 19},
  {IMS_SEND_PUBLISH_REQ_V01, TYPE16(0, 5), 1027},
  {IMS_SEND_UNPUBLISH_REQ_V01, TYPE16(0, 7), 0},
  {IMS_SEND_SUBSCRIBE_REQ_V01, TYPE16(0, 9), 1293},
  {IMS_SEND_UNSUBSCRIBE_REQ_V01, TYPE16(0, 11), 258},
  {IMS_ADD_PAR_RULE_REQ_V01, TYPE16(0, 15), 141},
  {IMS_GET_PAR_RULE_REQ_V01, TYPE16(0, 17), 7},
  {IMS_GET_PAR_RULE_AT_INDEX_REQ_V01, TYPE16(0, 19), 14},
  {IMS_DEL_PAR_RULE_AT_INDEX_REQ_V01, TYPE16(0, 21), 14},
  {IMS_DEL_PAR_RULE_REQ_V01, TYPE16(0, 23), 7},
  {IMS_DEL_PAR_DOCUMENT_REQ_V01, TYPE16(0, 25), 7},
  {IMS_SEND_PAR_REQ_V01, TYPE16(0, 27), 1034}
};

static const qmi_idl_service_message_table_entry qmi_ims_presence_service_response_messages_v01[] = {
  {IMS_ENABLER_STATE_RESP_V01, TYPE16(0, 1), 522},
  {IMS_SEND_SIMPLE_PUBLISH_RESP_V01, TYPE16(0, 4), 522},
  {IMS_SEND_PUBLISH_RESP_V01, TYPE16(0, 6), 529},
  {IMS_SEND_UNPUBLISH_RESP_V01, TYPE16(0, 8), 529},
  {IMS_SEND_SUBSCRIBE_RESP_V01, TYPE16(0, 10), 529},
  {IMS_SEND_UNSUBSCRIBE_RESP_V01, TYPE16(0, 12), 529},
  {IMS_ADD_PAR_RULE_RESP_V01, TYPE16(0, 16), 529},
  {IMS_GET_PAR_RULE_RESP_V01, TYPE16(0, 18), 663},
  {IMS_GET_PAR_RULE_AT_INDEX_RESP_V01, TYPE16(0, 20), 663},
  {IMS_DEL_PAR_RULE_AT_INDEX_RESP_V01, TYPE16(0, 22), 529},
  {IMS_DEL_PAR_RULE_RESP_V01, TYPE16(0, 24), 529},
  {IMS_DEL_PAR_DOCUMENT_RESP_V01, TYPE16(0, 26), 529},
  {IMS_SEND_PAR_RESP_V01, TYPE16(0, 28), 1556}
};

static const qmi_idl_service_message_table_entry qmi_ims_presence_service_indication_messages_v01[] = {
  {IMS_INCOMING_SIP_STATUS_IND_V01, TYPE16(0, 13), 1549},
  {IMS_INCOMING_HTTP_STATUS_IND_V01, TYPE16(0, 14), 1549},
  {IMS_ENABLER_STATE_IND_V01, TYPE16(0, 2), 7}
};

/*Service Object*/
const struct qmi_idl_service_object qmi_ims_presence_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  0x0014,
  1556,
  { sizeof(qmi_ims_presence_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(qmi_ims_presence_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(qmi_ims_presence_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { qmi_ims_presence_service_command_messages_v01, qmi_ims_presence_service_response_messages_v01, qmi_ims_presence_service_indication_messages_v01},
  &qmi_ims_presence_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type qmi_ims_presence_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( QMI_IMS_PRESENCE_V01_IDL_MAJOR_VERS != idl_maj_version || QMI_IMS_PRESENCE_V01_IDL_MINOR_VERS != idl_min_version 
       || QMI_IMS_PRESENCE_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&qmi_ims_presence_qmi_idl_service_object_v01;
}

