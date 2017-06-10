/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q M I _ I M S _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the ims_qmi service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.7
   It was generated on: Thu Sep  1 2011
   From IDL File: qmi_ims_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_ims_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t vt_call_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_call_info_type_v01, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(vt_call_info_type_v01, call_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(vt_call_info_type_v01, call_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(vt_call_info_type_v01, direction),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t vt_remote_party_number_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_remote_party_number_type_v01, number),
  QMI_IMS_VT_NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(vt_remote_party_number_type_v01, number) - QMI_IDL_OFFSET8(vt_remote_party_number_type_v01, number_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t vt_remote_party_name_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_remote_party_name_type_v01, caller_name),
  QMI_IMS_VT_CALLER_NAME_MAX_V01,
  QMI_IDL_OFFSET8(vt_remote_party_name_type_v01, caller_name) - QMI_IDL_OFFSET8(vt_remote_party_name_type_v01, caller_name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t vt_num_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_num_info_type_v01, num),
  QMI_IMS_VT_CALLER_ID_MAX_V01,
  QMI_IDL_OFFSET8(vt_num_info_type_v01, num) - QMI_IDL_OFFSET8(vt_num_info_type_v01, num_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t vt_call_end_reason_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_call_end_reason_type_v01, call_id),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(vt_call_end_reason_type_v01, call_end_reason),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t MEDIA_PACKET_INFO_RX_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(MEDIA_PACKET_INFO_RX_v01, iTimeStamp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(MEDIA_PACKET_INFO_RX_v01, iSeqNum),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(MEDIA_PACKET_INFO_RX_v01, bMarkerBit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(MEDIA_PACKET_INFO_RX_v01, bSilence),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(MEDIA_PACKET_INFO_RX_v01, iPayloadType),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t MEDIA_PACKET_INFO_data_v01[] = {
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(MEDIA_PACKET_INFO_v01, sMediaPktInfoRx),
 5, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t QP_MULTIMEDIA_FRAME_PLAY_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(QP_MULTIMEDIA_FRAME_PLAY_v01, pData),
  ((VIDEO_DATA_MAX_V01) & 0xFF), ((VIDEO_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(QP_MULTIMEDIA_FRAME_PLAY_v01, pData) - QMI_IDL_OFFSET8(QP_MULTIMEDIA_FRAME_PLAY_v01, pData_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(QP_MULTIMEDIA_FRAME_PLAY_v01, iDataLen),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(QP_MULTIMEDIA_FRAME_PLAY_v01, iMaxBuffLen),

  QMI_IDL_FLAGS_OFFSET_IS_16 |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(QP_MULTIMEDIA_FRAME_PLAY_v01, sMediaPacketInfo),
 6, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t QP_MULTIMEDIA_FRAME_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(QP_MULTIMEDIA_FRAME_v01, pData),
  ((VIDEO_DATA_MAX_V01) & 0xFF), ((VIDEO_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(QP_MULTIMEDIA_FRAME_v01, pData) - QMI_IDL_OFFSET8(QP_MULTIMEDIA_FRAME_v01, pData_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(QP_MULTIMEDIA_FRAME_v01, iDataLen),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(QP_MULTIMEDIA_FRAME_v01, iMaxBuffLen),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t QpVideoConfig_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(QpVideoConfig_v01, Codec),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(QpVideoConfig_v01, Width),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(QpVideoConfig_v01, Height),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(QpVideoConfig_v01, BitRate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(QpVideoConfig_v01, FrameRate),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(QpVideoConfig_v01, VolHeader),
  VOL_HEADER_MAX_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(QpVideoConfig_v01, VolHeaderLen),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(QpVideoConfig_v01, NALHeader),
  NAL_HEADER_MAX_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(QpVideoConfig_v01, NALHeaderLen),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(QpVideoConfig_v01, ClockRate),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(QpVideoConfig_v01, LipSyncEnable),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t vt_dial_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(vt_dial_call_req_msg_v01, calling_number),
  QMI_IMS_VT_NUMBER_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_dial_call_req_msg_v01, call_type) - QMI_IDL_OFFSET8(vt_dial_call_req_msg_v01, call_type_valid)),
  0x10,
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(vt_dial_call_req_msg_v01, call_type)
};

static const uint8_t vt_dial_call_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_dial_call_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_dial_call_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(vt_dial_call_resp_msg_v01, call_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_dial_call_resp_msg_v01, call_id)
};

static const uint8_t vt_end_call_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_end_call_req_msg_v01, call_id)
};

static const uint8_t vt_end_call_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_end_call_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_end_call_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(vt_end_call_resp_msg_v01, call_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_end_call_resp_msg_v01, call_id)
};

static const uint8_t vt_answer_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_answer_call_req_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(vt_answer_call_req_msg_v01, answer)
};

static const uint8_t vt_answer_call_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_answer_call_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_answer_call_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(vt_answer_call_resp_msg_v01, call_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_answer_call_resp_msg_v01, call_id)
};

static const uint8_t vt_get_call_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(vt_get_call_info_req_msg_v01, call_id)
};

static const uint8_t vt_get_call_info_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, call_info) - QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, call_info_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, call_info),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, remote_party_number) - QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, remote_party_number_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, remote_party_number),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, remote_party_name) - QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, remote_party_name_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_get_call_info_resp_msg_v01, remote_party_name),
  2, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(vt_get_call_info_resp_msg_v01, conn_num_info) - QMI_IDL_OFFSET16RELATIVE(vt_get_call_info_resp_msg_v01, conn_num_info_valid)),
  0x13,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(vt_get_call_info_resp_msg_v01, conn_num_info),
  3, 0
};

static const uint8_t vt_call_status_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, call_info),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, remote_party_number) - QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, remote_party_number_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, remote_party_number),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, remote_party_name) - QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, remote_party_name_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(vt_call_status_ind_msg_v01, remote_party_name),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(vt_call_status_ind_msg_v01, conn_num_info) - QMI_IDL_OFFSET16RELATIVE(vt_call_status_ind_msg_v01, conn_num_info_valid)),
  0x12,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(vt_call_status_ind_msg_v01, conn_num_info),
  3, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(vt_call_status_ind_msg_v01, call_end_reason) - QMI_IDL_OFFSET16RELATIVE(vt_call_status_ind_msg_v01, call_end_reason_valid)),
  0x13,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(vt_call_status_ind_msg_v01, call_end_reason),
  4, 0
};

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
 * ims_send__unpublish_req is empty
 * static const uint8_t ims_send__unpublish_req_data_v01[] = {
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

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_send_subscribe_req_v01, subscriptionEvent),
  SUBS_EVENT_MAX_V01,

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
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_send_unsubscribe_req_v01, peerURI),
  PEER_URI_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_send_unsubscribe_req_v01, UnSubscriptionEvent),
  SUBS_EVENT_MAX_V01
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

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_sip_msg_status_ind_v01, resp),
  0, 1,

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

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_http_msg_status_ind_v01, resp),
  0, 1,

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

static const uint8_t ims_video_initialise_ind_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_video_initialise_ind_v01, videoConfig),
  9, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(ims_video_initialise_ind_v01, video_device_type)
};

static const uint8_t ims_video_uninitialise_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_uninitialise_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_uninitialise_ind_v01, video_device_type)
};

static const uint8_t ims_video_play_start_ind_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_play_start_ind_v01, videoDescriptorID)
};

static const uint8_t ims_video_play_stop_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_play_stop_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_video_play_stop_ind_v01, bPurge) - QMI_IDL_OFFSET8(ims_video_play_stop_ind_v01, bPurge_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_video_play_stop_ind_v01, bPurge)
};

static const uint8_t ims_video_record_start_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_record_start_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_record_start_ind_v01, iFrameBundlingVal)
};

static const uint8_t ims_video_record_stop_ind_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_record_stop_ind_v01, videoDescriptorID)
};

static const uint8_t ims_video_start_prev_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_start_prev_ind_v01, videoDescriptorID),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_start_prev_ind_v01, eVPrevSize),

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_start_prev_ind_v01, iXPos),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_start_prev_ind_v01, iYPos)
};

static const uint8_t ims_video_play_frame_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_play_frame_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_video_play_frame_ind_v01, pCodecFrame),
  7, 0
};

static const uint8_t ims_video_codec_set_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_codec_set_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_video_codec_set_ind_v01, videoCodecConfig),
  9, 0
};

static const uint8_t ims_video_report_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_report_ind_v01, videoDescriptorID),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_report_ind_v01, n_iMSNtpTime),

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_report_ind_v01, n_iLSNtpTime),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_report_ind_v01, n_iRtpTimeStamp)
};

static const uint8_t ims_audio_report_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_audio_report_ind_v01, videoDescriptorID),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_audio_report_ind_v01, n_iMSNtpTime),

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_audio_report_ind_v01, n_iLSNtpTime),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_audio_report_ind_v01, n_iRtpTimeStamp)
};

static const uint8_t ims_last_audio_play_time_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_last_audio_play_time_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_last_audio_play_time_ind_v01, lastPlayTimeStamp)
};

static const uint8_t ims_current_audio_play_time_ind_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_current_audio_play_time_ind_v01, videoDescriptorID),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_current_audio_play_time_ind_v01, currentPlayTimeStamp)
};

static const uint8_t ims_video_status_report_request_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_video_status_report_request_v01, tVideoMsg),

  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_video_status_report_request_v01, pParam1),
  8, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(ims_video_status_report_request_v01, iParam2)
};

static const uint8_t ims_video_status_rsp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_video_status_rsp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_video_status_rsp_v01, response_details) - QMI_IDL_OFFSET8(ims_video_status_rsp_v01, response_details_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_video_status_rsp_v01, response_details),
  ((RESPONSE_DETAILS_VIDEO_MAX_V01) & 0xFF), ((RESPONSE_DETAILS_VIDEO_MAX_V01) >> 8)
};

/* Type Table */
static const qmi_idl_type_table_entry  ims_qmi_type_table_v01[] = {
  {sizeof(vt_call_info_type_v01), vt_call_info_type_data_v01},
  {sizeof(vt_remote_party_number_type_v01), vt_remote_party_number_type_data_v01},
  {sizeof(vt_remote_party_name_type_v01), vt_remote_party_name_type_data_v01},
  {sizeof(vt_num_info_type_v01), vt_num_info_type_data_v01},
  {sizeof(vt_call_end_reason_type_v01), vt_call_end_reason_type_data_v01},
  {sizeof(MEDIA_PACKET_INFO_RX_v01), MEDIA_PACKET_INFO_RX_data_v01},
  {sizeof(MEDIA_PACKET_INFO_v01), MEDIA_PACKET_INFO_data_v01},
  {sizeof(QP_MULTIMEDIA_FRAME_PLAY_v01), QP_MULTIMEDIA_FRAME_PLAY_data_v01},
  {sizeof(QP_MULTIMEDIA_FRAME_v01), QP_MULTIMEDIA_FRAME_data_v01},
  {sizeof(QpVideoConfig_v01), QpVideoConfig_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry ims_qmi_message_table_v01[] = {
  {sizeof(vt_dial_call_req_msg_v01), vt_dial_call_req_msg_data_v01},
  {sizeof(vt_dial_call_resp_msg_v01), vt_dial_call_resp_msg_data_v01},
  {sizeof(vt_end_call_req_msg_v01), vt_end_call_req_msg_data_v01},
  {sizeof(vt_end_call_resp_msg_v01), vt_end_call_resp_msg_data_v01},
  {sizeof(vt_answer_call_req_msg_v01), vt_answer_call_req_msg_data_v01},
  {sizeof(vt_answer_call_resp_msg_v01), vt_answer_call_resp_msg_data_v01},
  {sizeof(vt_get_call_info_req_msg_v01), vt_get_call_info_req_msg_data_v01},
  {sizeof(vt_get_call_info_resp_msg_v01), vt_get_call_info_resp_msg_data_v01},
  {sizeof(vt_call_status_ind_msg_v01), vt_call_status_ind_msg_data_v01},
  {0, 0},
  {sizeof(ims_enabler_state_resp_v01), ims_enabler_state_resp_data_v01},
  {sizeof(ims_enabler_state_ind_v01), ims_enabler_state_ind_data_v01},
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
  {sizeof(ims_send_par_resp_v01), ims_send_par_resp_data_v01},
  {sizeof(ims_video_initialise_ind_v01), ims_video_initialise_ind_data_v01},
  {sizeof(ims_video_uninitialise_ind_v01), ims_video_uninitialise_ind_data_v01},
  {sizeof(ims_video_play_start_ind_v01), ims_video_play_start_ind_data_v01},
  {sizeof(ims_video_play_stop_ind_v01), ims_video_play_stop_ind_data_v01},
  {sizeof(ims_video_record_start_ind_v01), ims_video_record_start_ind_data_v01},
  {sizeof(ims_video_record_stop_ind_v01), ims_video_record_stop_ind_data_v01},
  {sizeof(ims_video_start_prev_ind_v01), ims_video_start_prev_ind_data_v01},
  {sizeof(ims_video_play_frame_ind_v01), ims_video_play_frame_ind_data_v01},
  {sizeof(ims_video_codec_set_ind_v01), ims_video_codec_set_ind_data_v01},
  {sizeof(ims_video_report_ind_v01), ims_video_report_ind_data_v01},
  {sizeof(ims_audio_report_ind_v01), ims_audio_report_ind_data_v01},
  {sizeof(ims_last_audio_play_time_ind_v01), ims_last_audio_play_time_ind_data_v01},
  {sizeof(ims_current_audio_play_time_ind_v01), ims_current_audio_play_time_ind_data_v01},
  {sizeof(ims_video_status_report_request_v01), ims_video_status_report_request_data_v01},
  {sizeof(ims_video_status_rsp_v01), ims_video_status_rsp_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object ims_qmi_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *ims_qmi_qmi_idl_type_table_object_referenced_tables_v01[] =
{&ims_qmi_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object ims_qmi_qmi_idl_type_table_object_v01 = {
  sizeof(ims_qmi_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(ims_qmi_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  ims_qmi_type_table_v01,
  ims_qmi_message_table_v01,
  ims_qmi_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry ims_qmi_service_indication_messages_v01[] = {
  {QMI_IMS_VIDEO_INITIALISE_INDICATION_V01, TYPE16(0, 36), 551},
  {QMI_IMS_VIDEO_UNINITIALISE_INDICATION_V01, TYPE16(0, 37), 14},
  {QMI_IMS_VIDEO_STATUS_RESP_V01, TYPE16(0, 50), 522},
  {QMI_IMS_VIDEO_STATUS_REQUEST_V01, TYPE16(0, 49), 6167},
  {QMI_IMS_VIDEO_CODEC_SET_INDICATION_V01, TYPE16(0, 44), 551},
  {QMI_IMS_VIDEO_PLAY_FRAME_INDICATION_V01, TYPE16(0, 43), 6171},
  {QMI_IMS_VIDEO_START_PREVIEW_INDICATION_V01, TYPE16(0, 42), 28},
  {QMI_IMS_VIDEO_RECORD_STOP_INDICATION_V01, TYPE16(0, 41), 7},
  {QMI_IMS_VIDEO_RECORD_START_INDICATION_V01, TYPE16(0, 40), 14},
  {QMI_IMS_VIDEO_PLAY_STOP_INDICATION_V01, TYPE16(0, 39), 11},
  {QMI_IMS_VIDEO_PLAY_START_INDICATION_V01, TYPE16(0, 38), 7},
  {QMI_IMS_CURRENT_AUDIO_PLAY_TIME_INDICATION_V01, TYPE16(0, 48), 14},
  {QMI_IMS_LAST_AUDIO_PLAY_TIME_INDICATION_V01, TYPE16(0, 47), 14},
  {QMI_IMS_AUDIO_REPORT_INDICATION_V01, TYPE16(0, 46), 28},
  {QMI_IMS_VIDEO_REPORT_INDICATION_V01, TYPE16(0, 45), 28},
  {QMI_IMS_VT_STATUS_INDICATION_V01, TYPE16(0, 8), 369},
  {QMI_IMS_INCOMING_SIP_STATUS_IND_V01, TYPE16(0, 20), 1556},
  {QMI_IMS_INCOMING_HTTP_STATUS_IND_V01, TYPE16(0, 21), 1556},
  {QMI_IMS_ENABLER_STATE_IND_V01, TYPE16(0, 11), 7}
};

/*Service Object*/
const struct qmi_idl_service_object ims_qmi_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  0x0015,
  6171,
  { 0,
    0,
    sizeof(ims_qmi_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { NULL, NULL, ims_qmi_service_indication_messages_v01},
  &ims_qmi_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type ims_qmi_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMS_QMI_V01_IDL_MAJOR_VERS != idl_maj_version || IMS_QMI_V01_IDL_MINOR_VERS != idl_min_version 
       || IMS_QMI_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&ims_qmi_qmi_idl_service_object_v01;
}

