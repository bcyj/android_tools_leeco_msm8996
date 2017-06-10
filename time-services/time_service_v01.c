/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        T I M E _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the time service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/core/pkg/mpss/rel/1.0/modem_proc/core/services/time_qmi/src/time_service_v01.c#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.8
   It was generated on: Wed Sep 28 2011
   From IDL File: time_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "time_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t time_set_genoff_offset_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(time_set_genoff_offset_req_msg_v01, base),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(time_set_genoff_offset_req_msg_v01, generic_offset)
};

static const uint8_t time_set_genoff_offset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(time_set_genoff_offset_resp_msg_v01, resp),
  0, 1
};

static const uint8_t time_get_genoff_offset_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(time_get_genoff_offset_req_msg_v01, base)
};

static const uint8_t time_get_genoff_offset_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(time_get_genoff_offset_resp_msg_v01, resp),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(time_get_genoff_offset_resp_msg_v01, base),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(time_get_genoff_offset_resp_msg_v01, generic_offset)
};

static const uint8_t time_set_leap_sec_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(time_set_leap_sec_req_msg_v01, leap_sec_set_value)
};

static const uint8_t time_set_leap_sec_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(time_set_leap_sec_resp_msg_v01, resp),
  0, 1
};

/* 
 * time_get_leap_sec_req_msg is empty
 * static const uint8_t time_get_leap_sec_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t time_get_leap_sec_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(time_get_leap_sec_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(time_get_leap_sec_resp_msg_v01, leap_second)
};

static const uint8_t time_turn_off_ind_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(time_turn_off_ind_req_msg_v01, msg_id)
};

static const uint8_t time_turn_off_ind_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(time_turn_off_ind_resp_msg_v01, resp),
  0, 1
};

static const uint8_t time_turn_on_ind_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(time_turn_on_ind_req_msg_v01, msg_id)
};

static const uint8_t time_turn_on_ind_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(time_turn_on_ind_resp_msg_v01, resp),
  0, 1
};

static const uint8_t time_update_indication_message_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(time_update_indication_message_v01, base),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(time_update_indication_message_v01, offset)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry time_message_table_v01[] = {
  {sizeof(time_set_genoff_offset_req_msg_v01), time_set_genoff_offset_req_msg_data_v01},
  {sizeof(time_set_genoff_offset_resp_msg_v01), time_set_genoff_offset_resp_msg_data_v01},
  {sizeof(time_get_genoff_offset_req_msg_v01), time_get_genoff_offset_req_msg_data_v01},
  {sizeof(time_get_genoff_offset_resp_msg_v01), time_get_genoff_offset_resp_msg_data_v01},
  {sizeof(time_set_leap_sec_req_msg_v01), time_set_leap_sec_req_msg_data_v01},
  {sizeof(time_set_leap_sec_resp_msg_v01), time_set_leap_sec_resp_msg_data_v01},
  {0, 0},
  {sizeof(time_get_leap_sec_resp_msg_v01), time_get_leap_sec_resp_msg_data_v01},
  {sizeof(time_turn_off_ind_req_msg_v01), time_turn_off_ind_req_msg_data_v01},
  {sizeof(time_turn_off_ind_resp_msg_v01), time_turn_off_ind_resp_msg_data_v01},
  {sizeof(time_turn_on_ind_req_msg_v01), time_turn_on_ind_req_msg_data_v01},
  {sizeof(time_turn_on_ind_resp_msg_v01), time_turn_on_ind_resp_msg_data_v01},
  {sizeof(time_update_indication_message_v01), time_update_indication_message_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object time_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *time_qmi_idl_type_table_object_referenced_tables_v01[] =
{&time_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object time_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(time_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  time_message_table_v01,
  time_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry time_service_command_messages_v01[] = {
  {QMI_TIME_GENOFF_SET_REQ_MSG_V01, TYPE16(0, 0), 18},
  {QMI_TIME_GENOFF_GET_REQ_MSG_V01, TYPE16(0, 2), 7},
  {QMI_TIME_TURN_OFF_IND_REQ_MSG_V01, TYPE16(0, 8), 7},
  {QMI_TIME_TURN_ON_IND_REQ_MSG_V01, TYPE16(0, 10), 7},
  {QMI_TIME_LEAP_SEC_SET_REQ_MSG_V01, TYPE16(0, 4), 4},
  {QMI_TIME_LEAP_SEC_GET_REQ_MSG_V01, TYPE16(0, 6), 0}
};

static const qmi_idl_service_message_table_entry time_service_response_messages_v01[] = {
  {QMI_TIME_GENOFF_SET_RESP_MSG_V01, TYPE16(0, 1), 7},
  {QMI_TIME_GENOFF_GET_RESP_MSG_V01, TYPE16(0, 3), 25},
  {QMI_TIME_TURN_OFF_IND_RESP_MSG_V01, TYPE16(0, 9), 7},
  {QMI_TIME_TURN_ON_IND_RESP_MSG_V01, TYPE16(0, 11), 7},
  {QMI_TIME_LEAP_SEC_SET_RESP_MSG_V01, TYPE16(0, 5), 7},
  {QMI_TIME_LEAP_SEC_GET_RESP_MSG_V01, TYPE16(0, 7), 11}
};

static const qmi_idl_service_message_table_entry time_service_indication_messages_v01[] = {
  {QMI_TIME_ATS_RTC_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_TOD_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_USER_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_SECURE_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_DRM_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_USER_UTC_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_USER_TZ_DL_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_GPS_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_1X_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_HDR_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_WCDMA_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18},
  {QMI_TIME_ATS_BREW_UPDATE_IND_MSG_V01, TYPE16(0, 12), 18}
};

/*Service Object*/
const struct qmi_idl_service_object time_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  0x16,
  25,
  { sizeof(time_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(time_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(time_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { time_service_command_messages_v01, time_service_response_messages_v01, time_service_indication_messages_v01},
  &time_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type time_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( TIME_V01_IDL_MAJOR_VERS != idl_maj_version || TIME_V01_IDL_MINOR_VERS != idl_min_version 
       || TIME_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&time_qmi_idl_service_object_v01;
}

