#ifndef EMBMS_SERVICE_01_H
#define EMBMS_SERVICE_01_H
/**
  @file qmi_embms_v01.h

  @brief This is the public header file which defines the embms service Data structures.

  This header file defines the types and structures that were defined in
  embms. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.4
   It was generated on: Thu Mar 26 2015 (Spin 0)
   From IDL File: qmi_embms_v01.idl */

/** @defgroup embms_qmi_consts Constant values defined in the IDL */
/** @defgroup embms_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup embms_qmi_enums Enumerated types used in QMI messages */
/** @defgroup embms_qmi_messages Structures sent as QMI messages */
/** @defgroup embms_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup embms_qmi_accessor Accessor for QMI service object */
/** @defgroup embms_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup embms_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define EMBMS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define EMBMS_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define EMBMS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define EMBMS_V01_MAX_MESSAGE_ID 0x0021
/**
    @}
  */


/** @addtogroup embms_qmi_consts
    @{
  */
#define NUMBER_MAX_V01 64
#define IP_ADDRESS_LENGTH_MAX_V01 16
#define TMGI_LENGTH_MAX_V01 6
#define EARFCNLIST_MAX_V01 32
#define SIG_MAX_MBSFN_AREA_V01 8
#define SIG_MAX_TMGI_V01 256
#define MCC_MNC_MAX_V01 3
#define SAI_PER_FREQ_MAX_V01 64
#define FREQ_MAX_V01 9
#define SAI_MAX_V01 576
#define LOG_PACKET_ID_MAX_V01 256
#define LOG_PACKET_SIZE_MAX_V01 2048
#define CONTENT_PARAM_NUM_MAX_V01 64
#define PLMN_LIST_MAX_V01 16
/**
    @}
  */

/** @addtogroup embms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t tmgi_len;  /**< Must be set to # of elements in tmgi */
  uint8_t tmgi[TMGI_LENGTH_MAX_V01];
}embms_tmgi_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup embms_qmi_aggregates
    @{
  */
typedef struct {

  int32_t paramCode;

  int32_t paramValue;
}embms_content_desc_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup embms_qmi_aggregates
    @{
  */
typedef struct {

  char mcc[MCC_MNC_MAX_V01 + 1];

  char mnc[MCC_MNC_MAX_V01 + 1];
}embms_plmn_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup embms_qmi_enums
    @{
  */
typedef enum {
  RADIO_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  RADIO_STATE_AVAILABLE_V01 = 0x00,
  RADIO_STATE_NOT_AVAILABLE_V01 = 0x01,
  RADIO_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}radio_state_enum_v01;
/**
    @}
  */

/** @addtogroup embms_qmi_enums
    @{
  */
typedef enum {
  EMBMS_E911_MODE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  EMBMS_E911_MODE_INACTIVE_V01 = 0x0,
  EMBMS_E911_MODE_ACTIVE_V01 = 0x1,
  EMBMS_E911_MODE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}embms_e911_mode_v01;
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Enables EMBMS */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_enable_embms_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Enables EMBMS */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;

  /* Optional */
  uint8_t interface_name_valid;  /**< Must be set to true if interface_name is being passed */
  char interface_name[NUMBER_MAX_V01 + 1];

  /* Optional */
  uint8_t if_index_valid;  /**< Must be set to true if if_index is being passed */
  uint32_t if_index;
}embms_enable_embms_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Disables EMBMS */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;
}embms_disable_embms_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Disables EMBMS */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;
}embms_disable_embms_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Activates TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;

  /* Mandatory */
  embms_tmgi_type_v01 tmgi_info;

  /* Mandatory */
  int32_t preemption_priority;

  /* Mandatory */
  uint32_t earfcnlist_len;  /**< Must be set to # of elements in earfcnlist */
  int32_t earfcnlist[EARFCNLIST_MAX_V01];

  /* Optional */
  uint8_t saiList_valid;  /**< Must be set to true if saiList is being passed */
  uint32_t saiList_len;  /**< Must be set to # of elements in saiList */
  int32_t saiList[SAI_PER_FREQ_MAX_V01];
}embms_activate_tmgi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Activates TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;

  /* Optional */
  uint8_t tmgi_info_valid;  /**< Must be set to true if tmgi_info is being passed */
  embms_tmgi_type_v01 tmgi_info;
}embms_activate_tmgi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Deactivates TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;

  /* Mandatory */
  embms_tmgi_type_v01 tmgi_info;
}embms_deactivate_tmgi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Deactivates TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;

  /* Optional */
  uint8_t tmgi_info_valid;  /**< Must be set to true if tmgi_info is being passed */
  embms_tmgi_type_v01 tmgi_info;
}embms_deactivate_tmgi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Activates and deactivates TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;

  /* Mandatory */
  embms_tmgi_type_v01 act_tmgi_info;

  /* Mandatory */
  embms_tmgi_type_v01 deact_tmgi_info;

  /* Mandatory */
  int32_t preemption_priority;

  /* Mandatory */
  uint32_t earfcnlist_len;  /**< Must be set to # of elements in earfcnlist */
  int32_t earfcnlist[EARFCNLIST_MAX_V01];

  /* Optional */
  uint8_t saiList_valid;  /**< Must be set to true if saiList is being passed */
  uint32_t saiList_len;  /**< Must be set to # of elements in saiList */
  int32_t saiList[SAI_PER_FREQ_MAX_V01];
}embms_activate_deactivate_tmgi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Activates and deactivates TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint16_t act_resp_code;

  /* Mandatory */
  uint16_t deact_resp_code;

  /* Optional */
  uint8_t call_id_valid;  /**< Must be set to true if call_id is being passed */
  uint8_t call_id;

  /* Optional */
  uint8_t act_tmgi_info_valid;  /**< Must be set to true if act_tmgi_info is being passed */
  embms_tmgi_type_v01 act_tmgi_info;

  /* Optional */
  uint8_t deact_tmgi_info_valid;  /**< Must be set to true if deact_tmgi_info is being passed */
  embms_tmgi_type_v01 deact_tmgi_info;
}embms_activate_deactivate_tmgi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get Available TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;
}embms_get_available_tmgi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get Available TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t tmgi_info_valid;  /**< Must be set to true if tmgi_info is being passed */
  uint32_t tmgi_info_len;  /**< Must be set to # of elements in tmgi_info */
  embms_tmgi_type_v01 tmgi_info[NUMBER_MAX_V01];
}embms_get_available_tmgi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get Active TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;
}embms_get_active_tmgi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get Active TMGI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t tmgi_info_valid;  /**< Must be set to true if tmgi_info is being passed */
  uint32_t tmgi_info_len;  /**< Must be set to # of elements in tmgi_info */
  embms_tmgi_type_v01 tmgi_info[NUMBER_MAX_V01];
}embms_get_active_tmgi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Enables RSSI */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_enable_rssi_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Enables RSSI */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_enable_rssi_resp_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Disables RSSI */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_disable_rssi_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Disables RSSI */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_disable_rssi_resp_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get Coverage state */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_get_coverage_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get Coverage state */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t coverage_state_valid;  /**< Must be set to true if coverage_state is being passed */
  uint32_t coverage_state;
}embms_get_coverage_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get RSSI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_get_rssi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get RSSI */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t area_id_valid;  /**< Must be set to true if area_id is being passed */
  uint32_t area_id_len;  /**< Must be set to # of elements in area_id */
  int32_t area_id[SIG_MAX_MBSFN_AREA_V01];

  /* Optional */
  uint8_t sig_noise_ratio_valid;  /**< Must be set to true if sig_noise_ratio is being passed */
  uint32_t sig_noise_ratio_len;  /**< Must be set to # of elements in sig_noise_ratio */
  float sig_noise_ratio[SIG_MAX_MBSFN_AREA_V01];

  /* Optional */
  uint8_t excess_snr_valid;  /**< Must be set to true if excess_snr is being passed */
  uint32_t excess_snr_len;  /**< Must be set to # of elements in excess_snr */
  float excess_snr[SIG_MAX_MBSFN_AREA_V01];

  /* Optional */
  uint8_t number_of_tmgi_per_mbsfn_valid;  /**< Must be set to true if number_of_tmgi_per_mbsfn is being passed */
  uint32_t number_of_tmgi_per_mbsfn_len;  /**< Must be set to # of elements in number_of_tmgi_per_mbsfn */
  int32_t number_of_tmgi_per_mbsfn[SIG_MAX_MBSFN_AREA_V01];

  /* Optional */
  uint8_t active_tgmi_valid;  /**< Must be set to true if active_tgmi is being passed */
  uint32_t active_tgmi_len;  /**< Must be set to # of elements in active_tgmi */
  embms_tmgi_type_v01 active_tgmi[SIG_MAX_TMGI_V01];
}embms_get_rssi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get EMBMS Service state */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_get_embms_service_state_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get EMBMS Service state */
typedef struct {

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t embms_service_state_valid;  /**< Must be set to true if embms_service_state is being passed */
  uint32_t embms_service_state;
}embms_get_embms_service_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in EMBMS service state */
typedef struct {

  /* Mandatory */
  uint32_t state;

  /* Mandatory */
  char interface_name[NUMBER_MAX_V01 + 1];
}embms_unsol_embms_service_state_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in Active TMGI list */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t tmgi_info_len;  /**< Must be set to # of elements in tmgi_info */
  embms_tmgi_type_v01 tmgi_info[NUMBER_MAX_V01];
}embms_unsol_active_tmgi_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in Broadcast coverage */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t broadcast_coverage;
}embms_unsol_broadcast_coverage_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in RSSI */
typedef struct {

  /* Mandatory */
  uint32_t rssi;
}embms_unsol_rssi_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in Available TMGI list */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t tmgi_info_len;  /**< Must be set to # of elements in tmgi_info */
  embms_tmgi_type_v01 tmgi_info[NUMBER_MAX_V01];
}embms_unsol_available_tmgi_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates an oos waring in EMBMS service state. */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t reason;

  /* Mandatory */
  uint32_t tmgi_info_len;  /**< Must be set to # of elements in tmgi_info */
  embms_tmgi_type_v01 tmgi_info[NUMBER_MAX_V01];
}embms_unsol_oos_warning_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in mcc, mnc, cell_id */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  char mcc[MCC_MNC_MAX_V01 + 1];

  /* Mandatory */
  char mnc[MCC_MNC_MAX_V01 + 1];

  /* Mandatory */
  int32_t cell_id;
}embms_unsol_cell_info_changed_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change of radio state */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  radio_state_enum_v01 radio_state;
}embms_unsol_radio_state_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change of SAI list */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t camped_sai_list_len;  /**< Must be set to # of elements in camped_sai_list */
  int32_t camped_sai_list[SAI_PER_FREQ_MAX_V01];

  /* Mandatory */
  uint32_t num_of_sai_per_group_len;  /**< Must be set to # of elements in num_of_sai_per_group */
  int32_t num_of_sai_per_group[FREQ_MAX_V01];

  /* Mandatory */
  uint32_t available_sai_list_len;  /**< Must be set to # of elements in available_sai_list */
  int32_t available_sai_list[SAI_MAX_V01];
}embms_unsol_sai_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get active log packet ids */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t supported_log_packet_id_list_len;  /**< Must be set to # of elements in supported_log_packet_id_list */
  int32_t supported_log_packet_id_list[LOG_PACKET_ID_MAX_V01];
}embms_get_active_log_packet_ids_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get active log packet ids */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t active_log_packet_id_list_len;  /**< Must be set to # of elements in active_log_packet_id_list */
  int32_t active_log_packet_id_list[LOG_PACKET_ID_MAX_V01];
}embms_get_active_log_packet_ids_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Deliver log packet */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t packet_id;

  /* Mandatory */
  uint32_t log_packet_len;  /**< Must be set to # of elements in log_packet */
  int8_t log_packet[LOG_PACKET_SIZE_MAX_V01];
}embms_deliver_log_packet_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Deliver log packet */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_deliver_log_packet_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Set UTC time using SNTP time */
typedef struct {

  /* Mandatory */
  uint8_t sntp_available;

  /* Optional */
  uint8_t sntp_time_milli_sec_valid;  /**< Must be set to true if sntp_time_milli_sec is being passed */
  uint64_t sntp_time_milli_sec;

  /* Optional */
  uint8_t time_stamp_milli_sec_valid;  /**< Must be set to true if time_stamp_milli_sec is being passed */
  uint64_t time_stamp_milli_sec;
}embms_set_sntp_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Set UTC time using SNTP time */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_set_sntp_time_resp_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get sib16 coverage */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}embms_get_sib16_coverage_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get sib16 coverage */
typedef struct {

  /* Mandatory */
  uint8_t in_coverage;
}embms_get_sib16_coverage_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change of sib16 coverage */
typedef struct {

  /* Mandatory */
  uint8_t in_coverage;
}embms_unsol_sib16_coverage_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get UTC time */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_get_utc_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get UTC time */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Mandatory */
  uint64_t milli_sec;

  /* Optional */
  uint8_t day_light_saving_valid;  /**< Must be set to true if day_light_saving is being passed */
  uint8_t day_light_saving;

  /* Optional */
  uint8_t leap_seconds_valid;  /**< Must be set to true if leap_seconds is being passed */
  int8_t leap_seconds;

  /* Optional */
  uint8_t local_time_offset_valid;  /**< Must be set to true if local_time_offset is being passed */
  int8_t local_time_offset;
}embms_get_utc_time_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Find whether there's active e911 calls */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_get_e911_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Find whether there's active e911 calls */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;

  /* Optional */
  uint8_t e911_state_valid;  /**< Must be set to true if e911_state is being passed */
  embms_e911_mode_v01 e911_state;
}embms_get_e911_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change of e911 state */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  embms_e911_mode_v01 e911_state;
}embms_unsol_e911_state_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Update the content description */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t call_id;

  /* Mandatory */
  embms_tmgi_type_v01 tmgi_info;

  /* Optional */
  uint8_t content_desc_valid;  /**< Must be set to true if content_desc is being passed */
  uint32_t content_desc_len;  /**< Must be set to # of elements in content_desc */
  embms_content_desc_type_v01 content_desc[CONTENT_PARAM_NUM_MAX_V01];
}embms_update_content_desc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Update the content description */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  int32_t resp_code;
}embms_update_content_desc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in content description */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  embms_tmgi_type_v01 tmgi_info;

  /* Optional */
  uint8_t per_object_content_ctrl_valid;  /**< Must be set to true if per_object_content_ctrl is being passed */
  int32_t per_object_content_ctrl;

  /* Optional */
  uint8_t per_object_status_ctrl_valid;  /**< Must be set to true if per_object_status_ctrl is being passed */
  int32_t per_object_status_ctrl;
}embms_unsol_content_desc_update_per_obj_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get sib plmn info */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_get_sib_plmn_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get sib plmn info */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint32_t plmn_len;  /**< Must be set to # of elements in plmn */
  embms_plmn_type_v01 plmn[PLMN_LIST_MAX_V01];
}embms_get_sib_plmn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change of EMBMS status */
typedef struct {

  /* Mandatory */
  uint8_t is_available;
}embms_unsol_embms_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Request Message; Get EMBMS status info */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;
}embms_get_embms_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup embms_qmi_messages
    @{
  */
/** Response Message; Get EMBMS status info */
typedef struct {

  /* Mandatory */
  int32_t dbg_trace_id;

  /* Mandatory */
  uint8_t is_available;
}embms_get_embms_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01
//#define REMOVE_QMI_EMBMS_ACTIVATE_TMGI_REQ_V01
//#define REMOVE_QMI_EMBMS_ACTIVE_TMGI_IND_V01
//#define REMOVE_QMI_EMBMS_AVAILABLE_TMGI_IND_V01
//#define REMOVE_QMI_EMBMS_CELL_INFO_CHANGED_IND_V01
//#define REMOVE_QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01
//#define REMOVE_QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01
//#define REMOVE_QMI_EMBMS_DISABLE_EMBMS_REQ_V01
//#define REMOVE_QMI_EMBMS_DISABLE_RSSI_REQ_V01
//#define REMOVE_QMI_EMBMS_ENABLE_EMBMS_REQ_V01
//#define REMOVE_QMI_EMBMS_ENABLE_RSSI_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_E911_STATE_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_EMBMS_STATUS_V01
//#define REMOVE_QMI_EMBMS_GET_RSSI_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_SIB_PLMN_REQ_V01
//#define REMOVE_QMI_EMBMS_GET_UTC_TIME_REQ_V01
//#define REMOVE_QMI_EMBMS_OOS_WARNING_IND_V01
//#define REMOVE_QMI_EMBMS_RADIO_STATE_IND_V01
//#define REMOVE_QMI_EMBMS_SAI_IND_V01
//#define REMOVE_QMI_EMBMS_SET_SNTP_TIME_REQ_V01
//#define REMOVE_QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01
//#define REMOVE_QMI_EMBMS_UNSOL_CONTENT_DESC_UPDATE_PER_OBJ_IND_V01
//#define REMOVE_QMI_EMBMS_UNSOL_E911_STATE_IND_V01
//#define REMOVE_QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01
//#define REMOVE_QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01
//#define REMOVE_QMI_EMBMS_UNSOL_RSSI_IND_V01
//#define REMOVE_QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01
//#define REMOVE_QMI_EMBMS_UPDATE_CONTENT_DESC_REQ_V01

/*Service Message Definition*/
/** @addtogroup embms_qmi_msg_ids
    @{
  */
#define QMI_EMBMS_ENABLE_EMBMS_REQ_V01 0x0000
#define QMI_EMBMS_ENABLE_EMBMS_RESP_V01 0x0000
#define QMI_EMBMS_DISABLE_EMBMS_REQ_V01 0x0001
#define QMI_EMBMS_DISABLE_EMBMS_RESP_V01 0x0001
#define QMI_EMBMS_ACTIVATE_TMGI_REQ_V01 0x0002
#define QMI_EMBMS_ACTIVATE_TMGI_RESP_V01 0x0002
#define QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01 0x0003
#define QMI_EMBMS_DEACTIVATE_TMGI_RESP_V01 0x0003
#define QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01 0x0004
#define QMI_EMBMS_GET_AVAILABLE_TMGI_RESP_V01 0x0004
#define QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01 0x0005
#define QMI_EMBMS_GET_ACTIVE_TMGI_RESP_V01 0x0005
#define QMI_EMBMS_ENABLE_RSSI_REQ_V01 0x0006
#define QMI_EMBMS_ENABLE_RSSI_RESP_V01 0x0006
#define QMI_EMBMS_DISABLE_RSSI_REQ_V01 0x0007
#define QMI_EMBMS_DISABLE_RSSI_RESP_V01 0x0007
#define QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01 0x0008
#define QMI_EMBMS_GET_COVERAGE_STATE_RESP_V01 0x0008
#define QMI_EMBMS_GET_RSSI_REQ_V01 0x0009
#define QMI_EMBMS_GET_RSSI_RESP_V01 0x0009
#define QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01 0x000A
#define QMI_EMBMS_GET_EMBMS_SERVICE_STATE_RESP_V01 0x000A
#define QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01 0x000B
#define QMI_EMBMS_ACTIVE_TMGI_IND_V01 0x000C
#define QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01 0x000D
#define QMI_EMBMS_UNSOL_RSSI_IND_V01 0x000E
#define QMI_EMBMS_AVAILABLE_TMGI_IND_V01 0x000F
#define QMI_EMBMS_OOS_WARNING_IND_V01 0x0010
#define QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01 0x0011
#define QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_RESP_V01 0x0011
#define QMI_EMBMS_CELL_INFO_CHANGED_IND_V01 0x0012
#define QMI_EMBMS_RADIO_STATE_IND_V01 0x0013
#define QMI_EMBMS_SAI_IND_V01 0x0014
#define QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01 0x0015
#define QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_RESP_V01 0x0015
#define QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01 0x0016
#define QMI_EMBMS_DELIVER_LOG_PACKET_RESP_V01 0x0016
#define QMI_EMBMS_SET_SNTP_TIME_REQ_V01 0x0017
#define QMI_EMBMS_SET_SNTP_TIME_RESP_V01 0x0017
#define QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01 0x0018
#define QMI_EMBMS_GET_SIB16_COVERAGE_RESP_V01 0x0018
#define QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01 0x0019
#define QMI_EMBMS_GET_UTC_TIME_REQ_V01 0x001A
#define QMI_EMBMS_GET_UTC_TIME_RESP_V01 0x001A
#define QMI_EMBMS_GET_E911_STATE_REQ_V01 0x001B
#define QMI_EMBMS_GET_E911_STATE_RESP_V01 0x001B
#define QMI_EMBMS_E911_STATE_IND_V01 0x001C
#define QMI_EMBMS_UPDATE_CONTENT_DESC_REQ_V01 0x001D
#define QMI_EMBMS_UPDATE_CONTENT_DESC_RESP_V01 0x001D
#define QMI_EMBMS_UNSOL_CONTENT_DESC_UPDATE_PER_OBJ_IND_V01 0x001E
#define QMI_EMBMS_GET_SIB_PLMN_REQ_V01 0x001F
#define QMI_EMBMS_GET_SIB_PLMN_RESP_V01 0x001F
#define QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01 0x0020
#define QMI_EMBMS_GET_EMBMS_STATUS_REQ_V01 0x0021
#define QMI_EMBMS_GET_EMBMS_STATUS_RESP_V01 0x0021
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro embms_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type embms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define embms_get_service_object_v01( ) \
          embms_get_service_object_internal_v01( \
            EMBMS_V01_IDL_MAJOR_VERS, EMBMS_V01_IDL_MINOR_VERS, \
            EMBMS_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

