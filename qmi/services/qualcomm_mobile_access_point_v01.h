#ifndef QCMAP_SERVICE_H
#define QCMAP_SERVICE_H
/**
  @file qualcomm_mobile_access_point_v01.h
  
  @brief This is the public header file which defines the qcmap service Data structures.

  This header file defines the types and structures that were defined in 
  qcmap. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //components/rel/qmimsgs.mpss/2.3/qcmap/api/qualcomm_mobile_access_point_v01.h#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.1 
   It requires encode/decode library version 5 or later
   It was generated on: Wed Mar 27 2013 (Spin 0)
   From IDL File: qualcomm_mobile_access_point_v01.idl */

/** @defgroup qcmap_qmi_consts Constant values defined in the IDL */
/** @defgroup qcmap_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qcmap_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qcmap_qmi_messages Structures sent as QMI messages */
/** @defgroup qcmap_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qcmap_qmi_accessor Accessor for QMI service object */
/** @defgroup qcmap_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qcmap_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define QCMAP_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QCMAP_V01_IDL_MINOR_VERS 0x06
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QCMAP_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QCMAP_V01_MAX_MESSAGE_ID 0x0043;
/** 
    @} 
  */


/** @addtogroup qcmap_qmi_consts 
    @{ 
  */
#define QCMAP_MAX_SNAT_ENTRIES_V01 50
#define QCMAP_MAX_FIREWALL_ENTRIES_V01 50
#define QCMAP_IPV6_ADDR_LEN_V01 16
/**
    @}
  */

/** @addtogroup qcmap_qmi_enums
    @{
  */
typedef enum {
  QCMAP_IP_FAMILY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_IP_V4_V01 = 0x04, 
  QCMAP_IP_V6_V01 = 0x06, 
  QCMAP_IP_FAMILY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_ip_family_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_qmi_enums
    @{
  */
typedef enum {
  QCMAP_NAT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_NAT_TYPE_SYMMETRIC_V01 = 0x00, 
  QCMAP_NAT_TYPE_PORT_RESTRICTED_CONE_V01 = 0x01, 
  QCMAP_NAT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_nat_type_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_qmi_enums
    @{
  */
typedef enum {
  QCMAP_WWAN_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_WWAN_CONNECTING_V01 = 0x01, 
  QCMAP_WWAN_CONNECTED_V01 = 0x02, 
  QCMAP_WWAN_DISCONNECTING_V01 = 0x03, 
  QCMAP_WWAN_DISCONNECTED_V01 = 0x04, 
  QCMAP_WWAN_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_wwan_status_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_qmi_enums
    @{
  */
typedef enum {
  QCMAP_EXTD_FIREWALL_PROTOCOL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_EXTD_FIREWALL_PROTO_TCP_V01 = 0x01, 
  QCMAP_EXTD_FIREWALL_PROTO_UDP_V01 = 0x02, 
  QCMAP_EXTD_FIREWALL_PROTO_ICMP_V01 = 0x03, 
  QCMAP_EXTD_FIREWALL_PROTO_ICMP6_V01 = 0x04, 
  QCMAP_EXTD_FIREWALL_PROTO_ESP_V01 = 0x05, 
  QCMAP_EXTD_FIREWALL_PROTO_TCP_UDP_V01 = 0x06, 
  QCMAP_EXTD_FIREWALL_PROTOCOL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_extd_firewall_protocol_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_qmi_enums
    @{
  */
typedef enum {
  QCMAP_CALL_END_REASON_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_CER_UNSPECIFIED_V01 = 0x0001, 
  QCMAP_CER_CLIENT_END_V01 = 0x0002, 
  QCMAP_CER_NO_SRV_V01 = 0x0003, 
  QCMAP_CER_FADE_V01 = 0x0004, 
  QCMAP_CER_REL_NORMAL_V01 = 0x0005, 
  QCMAP_CER_ACC_IN_PROG_V01 = 0x0006, 
  QCMAP_CER_ACC_FAIL_V01 = 0x0007, 
  QCMAP_CER_REDIR_OR_HANDOFF_V01 = 0x0008, 
  QCMAP_CER_CLOSE_IN_PROGRESS_V01 = 0x0009, 
  QCMAP_CER_AUTH_FAILED_V01 = 0x000A, 
  QCMAP_CER_INTERNAL_CALL_END_V01 = 0x000B, 
  QCMAP_CER_CDMA_LOCK_V01 = 0x01F4, 
  QCMAP_CER_INTERCEPT_V01 = 0x01F5, 
  QCMAP_CER_REORDER_V01 = 0x01F6, 
  QCMAP_CER_REL_SO_REJ_V01 = 0x01F7, 
  QCMAP_CER_INCOM_CALL_V01 = 0x01F8, 
  QCMAP_CER_ALERT_STOP_V01 = 0x01F9, 
  QCMAP_CER_ACTIVATION_V01 = 0x01FA, 
  QCMAP_CER_MAX_ACCESS_PROBE_V01 = 0x01FB, 
  QCMAP_CER_CCS_NOT_SUPP_BY_BS_V01 = 0x01FC, 
  QCMAP_CER_NO_RESPONSE_FROM_BS_V01 = 0x01FD, 
  QCMAP_CER_REJECTED_BY_BS_V01 = 0x01FE, 
  QCMAP_CER_INCOMPATIBLE_V01 = 0x01FF, 
  QCMAP_CER_ALREADY_IN_TC_V01 = 0x0200, 
  QCMAP_CER_USER_CALL_ORIG_DURING_GPS_V01 = 0x0201, 
  QCMAP_CER_USER_CALL_ORIG_DURING_SMS_V01 = 0x0202, 
  QCMAP_CER_NO_CDMA_SRV_V01 = 0x0203, 
  QCMAP_CER_CONF_FAILED_V01 = 0x03E8, 
  QCMAP_CER_INCOM_REJ_V01 = 0x03E9, 
  QCMAP_CER_NO_GW_SRV_V01 = 0x03EA, 
  QCMAP_CER_NETWORK_END_V01 = 0x03EB, 
  QCMAP_CER_LLC_SNDCP_FAILURE_V01 = 0x03EC, 
  QCMAP_CER_INSUFFICIENT_RESOURCES_V01 = 0x03ED, 
  QCMAP_CER_OPTION_TEMP_OOO_V01 = 0x03EE, 
  QCMAP_CER_NSAPI_ALREADY_USED_V01 = 0x03EF, 
  QCMAP_CER_REGULAR_DEACTIVATION_V01 = 0x03F0, 
  QCMAP_CER_NETWORK_FAILURE_V01 = 0x03F1, 
  QCMAP_CER_UMTS_REATTACH_REQ_V01 = 0x03F2, 
  QCMAP_CER_PROTOCOL_ERROR_V01 = 0x03F3, 
  QCMAP_CER_OPERATOR_DETERMINED_BARRING_V01 = 0x03F4, 
  QCMAP_CER_UNKNOWN_APN_V01 = 0x03F5, 
  QCMAP_CER_UNKNOWN_PDP_V01 = 0x03F6, 
  QCMAP_CER_GGSN_REJECT_V01 = 0x03F7, 
  QCMAP_CER_ACTIVATION_REJECT_V01 = 0x03F8, 
  QCMAP_CER_OPTION_NOT_SUPP_V01 = 0x03F9, 
  QCMAP_CER_OPTION_UNSUBSCRIBED_V01 = 0x03FA, 
  QCMAP_CER_QOS_NOT_ACCEPTED_V01 = 0x03FB, 
  QCMAP_CER_TFT_SEMANTIC_ERROR_V01 = 0x03FC, 
  QCMAP_CER_TFT_SYNTAX_ERROR_V01 = 0x03FD, 
  QCMAP_CER_UNKNOWN_PDP_CONTEXT_V01 = 0x03FE, 
  QCMAP_CER_FILTER_SEMANTIC_ERROR_V01 = 0x03FF, 
  QCMAP_CER_FILTER_SYNTAX_ERROR_V01 = 0x0400, 
  QCMAP_CER_PDP_WITHOUT_ACTIVE_TFT_V01 = 0x0401, 
  QCMAP_CER_INVALID_TRANSACTION_ID_V01 = 0x0402, 
  QCMAP_CER_MESSAGE_INCORRECT_SEMANTIC_V01 = 0x0403, 
  QCMAP_CER_INVALID_MANDATORY_INFO_V01 = 0x0404, 
  QCMAP_CER_MESSAGE_TYPE_UNSUPPORTED_V01 = 0x0405, 
  QCMAP_CER_MSG_TYPE_NONCOMPATIBLE_STATE_V01 = 0x0406, 
  QCMAP_CER_UNKNOWN_INFO_ELEMENT_V01 = 0x0407, 
  QCMAP_CER_CONDITIONAL_IE_ERROR_V01 = 0x0408, 
  QCMAP_CER_MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE_V01 = 0x0409, 
  QCMAP_CER_APN_TYPE_CONFLICT_V01 = 0x040A, 
  QCMAP_CER_NO_GPRS_CONTEXT_V01 = 0x040B, 
  QCMAP_CER_FEATURE_NOT_SUPPORTED_V01 = 0x040C, 
  QCMAP_CER_CD_GEN_OR_BUSY_V01 = 0x05DC, 
  QCMAP_CER_CD_BILL_OR_AUTH_V01 = 0x05DD, 
  QCMAP_CER_CHG_HDR_V01 = 0x05DE, 
  QCMAP_CER_EXIT_HDR_V01 = 0x05DF, 
  QCMAP_CER_HDR_NO_SESSION_V01 = 0x05E0, 
  QCMAP_CER_HDR_ORIG_DURING_GPS_FIX_V01 = 0x05E1, 
  QCMAP_CER_HDR_CS_TIMEOUT_V01 = 0x05E2, 
  QCMAP_CER_HDR_RELEASED_BY_CM_V01 = 0x05E3, 
  QCMAP_CALL_END_REASON_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_call_end_reason_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_qmi_enums
    @{
  */
typedef enum {
  QCMAP_VERBOSE_CALL_END_REASON_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_VCER_UNSPECIFIED_V01 = 0x00000000, 
  QCMAP_VCER_MIP_FA_REASON_UNSPECIFIED_V01 = 0x00010040, 
  QCMAP_VCER_MIP_FA_ADMIN_PROHIBITED_V01 = 0x00010041, 
  QCMAP_VCER_MIP_FA_INSUFFICIENT_RESOURCES_V01 = 0x00010042, 
  QCMAP_VCER_MIP_FA_MOBILE_NODE_AUTH_FAILURE_V01 = 0x00010043, 
  QCMAP_VCER_MIP_FA_HA_AUTH_FAILURE_V01 = 0x00010044, 
  QCMAP_VCER_MIP_FA_REQ_LIFETIME_TOO_LONG_V01 = 0x00010045, 
  QCMAP_VCER_MIP_FA_MALFORMED_REQUEST_V01 = 0x00010046, 
  QCMAP_VCER_MIP_FA_MALFOMED_REPLY_V01 = 0x00010047, 
  QCMAP_VCER_MIP_FA_ENCAPSULATION_UNAVAILABLE_V01 = 0x00010048, 
  QCMAP_VCER_MIP_FA_VJHC_UNAVAILABLE_V01 = 0x00010049, 
  QCMAP_VCER_MIP_FA_REV_TUNNEL_UNAVAILABLE_V01 = 0x0001004A, 
  QCMAP_VCER_MIP_FA_REV_TUNNEL_IS_MAND_AND_T_BIT_NOT_SET_V01 = 0x0001004B, 
  QCMAP_VCER_MIP_FA_DELIVERY_STYLE_NOT_SUPP_V01 = 0x0001004F, 
  QCMAP_VCER_MIP_FA_MISSING_NAI_V01 = 0x00010061, 
  QCMAP_VCER_MIP_FA_MISSING_HA_V01 = 0x00010062, 
  QCMAP_VCER_MIP_FA_MISSING_HOME_ADDR_V01 = 0x00010063, 
  QCMAP_VCER_MIP_FA_UNKNOWN_CHALLENGE_V01 = 0x00010068, 
  QCMAP_VCER_MIP_FA_MISSING_CHALLENGE_V01 = 0x00010069, 
  QCMAP_VCER_MIP_FA_STALE_CHALLENGE_V01 = 0x0001006A, 
  QCMAP_VCER_MIP_HA_REASON_UNSPECIFIED_V01 = 0x00010080, 
  QCMAP_VCER_MIP_HA_ADMIN_PROHIBITED_V01 = 0x00010081, 
  QCMAP_VCER_MIP_HA_INSUFFICIENT_RESOURCES_V01 = 0x00010082, 
  QCMAP_VCER_MIP_HA_MOBILE_NODE_AUTH_FAILURE_V01 = 0x00010083, 
  QCMAP_VCER_MIP_HA_FA_AUTH_FAILURE_V01 = 0x00010084, 
  QCMAP_VCER_MIP_HA_REGISTRATION_ID_MISMATCH_V01 = 0x00010085, 
  QCMAP_VCER_MIP_HA_MALFORMED_REQUEST_V01 = 0x00010086, 
  QCMAP_VCER_MIP_HA_UNKNOWN_HA_ADDR_V01 = 0x00010088, 
  QCMAP_VCER_MIP_HA_REV_TUNNEL_UNAVAILABLE_V01 = 0x00010089, 
  QCMAP_VCER_MIP_HA_REV_TUNNEL_IS_MAND_AND_T_BIT_NOT_SET_V01 = 0x0001008A, 
  QCMAP_VCER_MIP_HA_ENCAPSULATION_UNAVAILABLE_V01 = 0x0001008B, 
  QCMAP_VCER_MIP_HA_REASON_UNKNOWN_V01 = 0x0001FFFF, 
  QCMAP_VCER_INTERNAL_INTERNAL_ERROR_V01 = 0x000200C9, 
  QCMAP_VCER_INTERNAL_CALL_ENDED_V01 = 0x000200CA, 
  QCMAP_VCER_INTERNAL_INTERNAL_UNKNOWN_CAUSE_CODE_V01 = 0x000200CB, 
  QCMAP_VCER_INTERNAL_UNKNOWN_CAUSE_CODE_V01 = 0x000200CC, 
  QCMAP_VCER_INTERNAL_CLOSE_IN_PROGRESS_V01 = 0x000200CD, 
  QCMAP_VCER_INTERNAL_NW_INITIATED_TERMINATION_V01 = 0x000200CE, 
  QCMAP_VCER_INTERNAL_APP_PREEMPTED_V01 = 0x000200CF, 
  QCMAP_VCER_CM_CDMA_LOCK_V01 = 0x000301F4, 
  QCMAP_VCER_CM_INTERCEPT_V01 = 0x000301F5, 
  QCMAP_VCER_CM_REORDER_V01 = 0x000301F6, 
  QCMAP_VCER_CM_REL_SO_REJ_V01 = 0x000301F7, 
  QCMAP_VCER_CM_INCOM_CALL_V01 = 0x000301F8, 
  QCMAP_VCER_CM_ALERT_STOP_V01 = 0x000301F9, 
  QCMAP_VCER_CM_ACTIVATION_V01 = 0x000301FA, 
  QCMAP_VCER_CM_MAX_ACCESS_PROBE_V01 = 0x000301FB, 
  QCMAP_VCER_CM_CCS_NOT_SUPP_BY_BS_V01 = 0x000301FC, 
  QCMAP_VCER_CM_NO_RESPONSE_FROM_BS_V01 = 0x000301FD, 
  QCMAP_VCER_CM_REJECTED_BY_BS_V01 = 0x000301FE, 
  QCMAP_VCER_CM_INCOMPATIBLE_V01 = 0x000301FF, 
  QCMAP_VCER_CM_ALREADY_IN_TC_V01 = 0x00030200, 
  QCMAP_VCER_CM_USER_CALL_ORIG_DURING_GPS_V01 = 0x00030201, 
  QCMAP_VCER_CM_USER_CALL_ORIG_DURING_SMS_V01 = 0x00030202, 
  QCMAP_VCER_CM_NO_CDMA_SRV_V01 = 0x00030203, 
  QCMAP_VCER_CM_RETRY_ORDER_V01 = 0x00030207, 
  QCMAP_VCER_CM_CONF_FAILED_V01 = 0x000303E8, 
  QCMAP_VCER_CM_INCOM_REJ_V01 = 0x000303E9, 
  QCMAP_VCER_CM_NO_GW_SERV_V01 = 0x000303F0, 
  QCMAP_VCER_CM_NO_GPRS_CONTEXT_V01 = 0x000303F1, 
  QCMAP_VCER_CM_ILLEGAL_MS_V01 = 0x000303F2, 
  QCMAP_VCER_CM_ILLEGAL_ME_V01 = 0x000303F3, 
  QCMAP_VCER_CM_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOWED_V01 = 0x000303F4, 
  QCMAP_VCER_CM_GPRS_SERV_NOT_ALLOWED_V01 = 0x000303F5, 
  QCMAP_VCER_CM_MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK_V01 = 0x000303F6, 
  QCMAP_VCER_CM_IMPLICITLY_DETACHED_V01 = 0x000303F7, 
  QCMAP_VCER_CM_PLMN_NOT_ALLOWED_V01 = 0x000303F8, 
  QCMAP_VCER_CM_LA_NOT_ALLOWED_V01 = 0x000303F9, 
  QCMAP_VCER_CM_GPRS_SERV_NOT_ALLOWED_IN_THIS_PLMN_V01 = 0x000303FA, 
  QCMAP_VCER_CM_PDP_DUPLICATE_V01 = 0x000303FB, 
  QCMAP_VCER_CM_UE_RAT_CHANGE_V01 = 0x000303FC, 
  QCMAP_VCER_CM_CONGESTION_V01 = 0x000303FD, 
  QCMAP_VCER_CM_NO_PDP_CONTEXT_ACTIVATED_V01 = 0x000303FE, 
  QCMAP_VCER_CM_ACCESS_CLASS_DSAC_REJECTION_V01 = 0x000303FF, 
  QCMAP_VCER_CM_CD_GEN_OR_BUSY_V01 = 0x000305DC, 
  QCMAP_VCER_CM_CD_BILL_OR_AUTH_V01 = 0x000305DD, 
  QCMAP_VCER_CM_CHG_HDR_V01 = 0x000305DE, 
  QCMAP_VCER_CM_EXIT_HDR_V01 = 0x000305DF, 
  QCMAP_VCER_CM_HDR_NO_SESSION_V01 = 0x000305E0, 
  QCMAP_VCER_CM_HDR_ORIG_DURING_GPS_FIX_V01 = 0x000305E1, 
  QCMAP_VCER_CM_HDR_CS_TIMEOUT_V01 = 0x000305E2, 
  QCMAP_VCER_CM_HDR_RELEASED_BY_CM_V01 = 0x000305E3, 
  QCMAP_VCER_CM_NO_HYBR_HDR_SRV_V01 = 0x000305E6, 
  QCMAP_VCER_CM_CLIENT_END_V01 = 0x000307D0, 
  QCMAP_VCER_CM_NO_SRV_V01 = 0x000307D1, 
  QCMAP_VCER_CM_FADE_V01 = 0x000307D2, 
  QCMAP_VCER_CM_REL_NORMAL_V01 = 0x000307D3, 
  QCMAP_VCER_CM_ACC_IN_PROG_V01 = 0x000307D4, 
  QCMAP_VCER_CM_ACC_FAIL_V01 = 0x000307D5, 
  QCMAP_VCER_CM_REDIR_OR_HANDOFF_V01 = 0x000307D6, 
  QCMAP_VCER_3GPP_OPERATOR_DETERMINED_BARRING_V01 = 0x00060008, 
  QCMAP_VCER_3GPP_LLC_SNDCP_FAILURE_V01 = 0x00060019, 
  QCMAP_VCER_3GPP_INSUFFICIENT_RESOURCES_V01 = 0x0006001A, 
  QCMAP_VCER_3GPP_UNKNOWN_APN_V01 = 0x0006001B, 
  QCMAP_VCER_3GPP_UNKNOWN_PDP_V01 = 0x0006001C, 
  QCMAP_VCER_3GPP_AUTH_FAILED_V01 = 0x0006001D, 
  QCMAP_VCER_3GPP_GGSN_REJECT_V01 = 0x0006001E, 
  QCMAP_VCER_3GPP_ACTIVATION_REJECT_V01 = 0x0006001F, 
  QCMAP_VCER_3GPP_OPTION_NOT_SUPPORTED_V01 = 0x00060020, 
  QCMAP_VCER_3GPP_OPTION_UNSUBSCRIBED_V01 = 0x00060021, 
  QCMAP_VCER_3GPP_OPTION_TEMP_OOO_V01 = 0x00060022, 
  QCMAP_VCER_3GPP_NSAPI_ALREADY_USED_V01 = 0x00060023, 
  QCMAP_VCER_3GPP_REGULAR_DEACTIVATION_V01 = 0x00060024, 
  QCMAP_VCER_3GPP_QOS_NOT_ACCEPTED_V01 = 0x00060025, 
  QCMAP_VCER_3GPP_NETWORK_FAILURE_V01 = 0x00060026, 
  QCMAP_VCER_3GPP_UMTS_REACTIVATION_REQ_V01 = 0x00060027, 
  QCMAP_VCER_3GPP_FEATURE_NOT_SUPP_V01 = 0x00060028, 
  QCMAP_VCER_3GPP_TFT_SEMANTIC_ERROR_V01 = 0x00060029, 
  QCMAP_VCER_3GPP_TFT_SYTAX_ERROR_V01 = 0x0006002A, 
  QCMAP_VCER_3GPP_UNKNOWN_PDP_CONTEXT_V01 = 0x0006002B, 
  QCMAP_VCER_3GPP_FILTER_SEMANTIC_ERROR_V01 = 0x0006002C, 
  QCMAP_VCER_3GPP_FILTER_SYTAX_ERROR_V01 = 0x0006002D, 
  QCMAP_VCER_3GPP_PDP_WITHOUT_ACTIVE_TFT_V01 = 0x0006002E, 
  QCMAP_VCER_3GPP_INVALID_TRANSACTION_ID_V01 = 0x00060051, 
  QCMAP_VCER_3GPP_MESSAGE_INCORRECT_SEMANTIC_V01 = 0x0006005F, 
  QCMAP_VCER_3GPP_INVALID_MANDATORY_INFO_V01 = 0x00060060, 
  QCMAP_VCER_3GPP_MESSAGE_TYPE_UNSUPPORTED_V01 = 0x00060061, 
  QCMAP_VCER_3GPP_MSG_TYPE_NONCOMPATIBLE_STATE_V01 = 0x00060062, 
  QCMAP_VCER_3GPP_UNKNOWN_INFO_ELEMENT_V01 = 0x00060063, 
  QCMAP_VCER_3GPP_CONDITIONAL_IE_ERROR_V01 = 0x00060064, 
  QCMAP_VCER_3GPP_MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE_V01 = 0x00060065, 
  QCMAP_VCER_3GPP_PROTOCOL_ERROR_V01 = 0x0006006F, 
  QCMAP_VCER_3GPP_APN_TYPE_CONFLICT_V01 = 0x00060070, 
  QCMAP_VCER_PPP_TIMEOUT_V01 = 0x00070001, 
  QCMAP_VCER_PPP_AUTH_FAILURE_V01 = 0x00070002, 
  QCMAP_VCER_PPP_OPTION_MISMATCH_V01 = 0x00070003, 
  QCMAP_VCER_PPP_PAP_FAILURE_V01 = 0x0007001F, 
  QCMAP_VCER_PPP_CHAP_FAILURE_V01 = 0x00070020, 
  QCMAP_VCER_PPP_UNKNOWN_V01 = 0x0007FFFF, 
  QCMAP_VCER_EHRPD_SUBS_LIMITED_TO_V4_V01 = 0x00080001, 
  QCMAP_VCER_EHRPD_SUBS_LIMITED_TO_V6_V01 = 0x00080002, 
  QCMAP_VCER_EHRPD_VSNCP_TIMEOUT_V01 = 0x00080004, 
  QCMAP_VCER_EHRPD_VSNCP_FAILURE_V01 = 0x00080005, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_GEN_ERROR_V01 = 0x00080006, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_UNAUTH_APN_V01 = 0x00080007, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_PDN_LIMIT_EXCEED_V01 = 0x00080008, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_NO_PDN_GW_V01 = 0x00080009, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_PDN_GW_UNREACH_V01 = 0x0008000A, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_PDN_GW_REJ_V01 = 0x0008000B, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_INSUFF_PARAM_V01 = 0x0008000C, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_RESOURCE_UNAVAIL_V01 = 0x0008000D, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_ADMIN_PROHIBIT_V01 = 0x0008000E, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_PDN_ID_IN_USE_V01 = 0x0008000F, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_SUBSCR_LIMITATION_V01 = 0x00080010, 
  QCMAP_VCER_EHRPD_VSNCP_3GPP2I_PDN_EXISTS_FOR_THIS_APN_V01 = 0x00080011, 
  QCMAP_VCER_IPV6_PREFIX_UNAVAILABLE_V01 = 0x00090001, 
  QCMAP_VERBOSE_CALL_END_REASON_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_verbose_call_end_reason_enum_v01;
/**
    @}
  */

typedef uint64_t qcmap_tech_pref_mask_v01;
#define QCMAP_MASK_TECH_PREF_3GPP_V01 ((qcmap_tech_pref_mask_v01)0x01ull) 
#define QCMAP_MASK_TECH_PREF_3GPP2_V01 ((qcmap_tech_pref_mask_v01)0x02ull) 
typedef uint64_t qcmap_addr_type_mask_v01;
#define QCMAP_MASK_V4_ADDR_V01 ((qcmap_addr_type_mask_v01)0x00000001ull) 
#define QCMAP_MASK_V6_ADDR_V01 ((qcmap_addr_type_mask_v01)0x00000002ull) 
#define QCMAP_MASK_V4_DNS_ADDR_V01 ((qcmap_addr_type_mask_v01)0x00000004ull) 
#define QCMAP_MASK_V6_DNS_ADDR_V01 ((qcmap_addr_type_mask_v01)0x00000008ull) 
/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t firewall_handle;
  /**<   Handle identifying the firewall rule. */

  uint16_t start_dest_port;
  /**<   Start value of the destination port range. */

  uint16_t end_dest_port;
  /**<   End value of the destination port range. */

  uint8_t protocol;
  /**<   Protocol value. */
}qcmap_get_firewall_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t private_ip_addr;
  /**<   Private IP address. */

  uint16_t private_port;
  /**<   Private port. */

  uint16_t global_port;
  /**<   Global port. */

  uint8_t protocol;
  /**<   Protocol. */
}qcmap_snat_entry_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t subnet_mask;
  /**<   Subnet mask. */

  uint32_t nat_ip_addr;
  /**<   NAT IP address. */

  uint32_t nat_dns_addr;
  /**<   NAT Domain Name Service (DNS) address. */

  uint32_t usb_rmnet_ip_addr;
  /**<   RmNet USB Terminal Equipment (TE) address. */

  uint32_t usb_rmnet_gateway_addr;
  /**<   RmNet USB gateway address. */

  uint32_t apps_rmnet_ip_addr;
  /**<   RmNet applications IP address. */

  uint32_t apps_rmnet_gateway_addr;
  /**<   RmNet applications gateway address. */
}qcmap_ipv4_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  qcmap_tech_pref_mask_v01 tech_pref;
  /**<   Bitmap indicating the technology preference. A single connection
       is attempted using the following specified technology preferences: \n
       - Bit 0 -- 3GPP \n
       - Bit 1 -- 3GPP2 
       
       All other bits are reserved and ignored even if they are set
       in the request. If a single value of the technology preference 
       bitmask is set, the device attempts to use that technology. If two
       or more bits in the technology preference bitmask are set, the
       device determines the technology to be used from those specified.
  */

  uint8_t profile_id_3gpp2;
  /**<   CDMA profile ID. */

  uint8_t profile_id_3gpp;
  /**<   UMTS profile ID. */
}qcmap_net_policy_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t start_dest_port;
  /**<   Start value of the destination port range. */

  uint16_t end_dest_port;
  /**<   End value of the destination port range. */

  uint8_t protocol;
  /**<   Protocol value. */
}qcmap_firewall_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t addr;
  /**<   IPv4 address as specified in the IPv4 protocol
       specification (RFC 791 \hyperref[S2]{[S2]}).
  */

  uint32_t subnet_mask;
  /**<   IPv4 subnet mask as specified in the IPv4 protocol 
       specification (RFC 791 \hyperref[S2]{[S2]}).
  */
}qcmap_ip4_addr_subnet_mask_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t addr[QCMAP_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address as specified in the IPv6 protocol 
       specification (RFC 2460 \hyperref[S5]{[S5]}).
  */

  uint8_t prefix_len;
  /**<   IPv6 prefix length as specified in the IPv6 protocol 
       addressing architecture specification (RFC 3513 \hyperref[S6]{[S6]}).
  */
}qcmap_ip6_addr_prefix_len_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t port;
  /**<   TCP/UDP port as specified in the TCP/UDP protocol 
       (RFC 793 \hyperref[S4]{[S4]} and RFC 768 \hyperref[S1]{[S1]}).
  */

  uint16_t range;
  /**<   TCP/UDP port range as specified in the TCP/UDP protocol
       (RFC 793 \hyperref[S4]{[S4]} and RFC 768 \hyperref[S1]{[S1]}).
  */
}qcmap_tcp_udp_port_range_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t value;
  /**<   TOS value as specified in the IPv4 protocol 
       specification (RFC 791 \hyperref[S2]{[S2]}). */

  uint8_t mask;
  /**<   IPv4 TOS mask.  */
}qcmap_ip4_tos_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t value;
  /**<   IPv6 traffic class value as specified in the IPv6 protocol 
       specification (RFC 2460 \hyperref[S5]{[S5]}). */

  uint8_t mask;
  /**<   IPv6 traffic class mask.  */
}qcmap_ip6_traffic_class_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Enables the mobile AP functionality via a single mobile AP instance 
           on the modem. */
typedef struct {

  /* Mandatory */
  /*  IP Family */
  qcmap_ip_family_enum_v01 ip_family;
  /**<   Determines whether mobile AP IPv4 or IPv6 must be enabled. Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */

  /* Optional */
  /*  IP Address */
  uint8_t ip_addr_info_valid;  /**< Must be set to true if ip_addr_info is being passed */
  qcmap_ipv4_info_v01 ip_addr_info;

  /* Optional */
  /*  Network Policy */
  uint8_t net_policy_info_valid;  /**< Must be set to true if net_policy_info is being passed */
  qcmap_net_policy_info_v01 net_policy_info;

  /* Optional */
  /*  SSID2 IP Address Info */
  uint8_t ssid2_ip_addr_info_valid;  /**< Must be set to true if ssid2_ip_addr_info is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ssid2_ip_addr_info;

  /* Optional */
  /*  NAT Type Info */
  uint8_t qcmap_nat_type_info_valid;  /**< Must be set to true if qcmap_nat_type_info is being passed */
  qcmap_nat_type_enum_v01 qcmap_nat_type_info;
  /**<   NAT type specified during mobile AP enable. Values: \n
      - 0x00 -- QCMAP_NAT_TYPE_ SYMMETRIC -- 
                Symmetric NAT\n
      - 0x01 -- QCMAP_NAT_TYPE_ PORT_RESTRICTED_CONE -- 
                 Port restricted cone NAT
    */

  /* Optional */
  /*  DUN Client IPv4 Address */
  uint8_t dun_client_ip_addr_valid;  /**< Must be set to true if dun_client_ip_addr is being passed */
  uint32_t dun_client_ip_addr;
}qcmap_mobile_ap_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Enables the mobile AP functionality via a single mobile AP instance 
           on the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Mobile AP Handle */
  uint8_t mobile_ap_handle_valid;  /**< Must be set to true if mobile_ap_handle is being passed */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The mobile AP handle must be retained by the control point and
       specified in all mobile AP-specific QCMAP messages. For example,
       QMI_QCMAP_DISABLE, QMI_QCMAP_BRING_UP_WWAN, etc.
   */
}qcmap_mobile_ap_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Disables the mobile AP functionality for a mobile AP instance on 
           the modem. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_mobile_ap_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Disables the mobile AP functionality for a mobile AP instance on 
           the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_mobile_ap_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Invokes bringing up the WWAN from the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_bring_up_wwan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Invokes bringing up the WWAN from the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_bring_up_wwan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Indication Message; Indicates the completion of processing a 
           QMI_QCMAP_BRING_UP_WWAN_REQ. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
   */

  /* Mandatory */
  /*  IP Family */
  qcmap_ip_family_enum_v01 ip_family;
  /**<   Determines whether the mobile AP is IPv4 or IPv6. Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */
}qcmap_bring_up_wwan_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Tears down the WWAN. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_tear_down_wwan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Tears down the WWAN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_tear_down_wwan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Indication Message; Indicates the completion of processing a 
           QMI_QCMAP_TEAR_DOWN_WWAN_REQ. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
   */

  /* Mandatory */
  /*  IP Family */
  qcmap_ip_family_enum_v01 ip_family;
  /**<   Determines whether the mobile AP is IPv4 or IPv6. Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */
}qcmap_tear_down_wwan_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the current WWAN status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_wwan_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the current WWAN status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Call End Reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  qcmap_call_end_reason_enum_v01 call_end_reason;
  /**<   Reason the call ended; 
       see Table \ref{tbl:callEndReason} for the definition of these values.
  */

  /* Optional */
  /*  Verbose Call End Reason */
  uint8_t verbose_call_end_reason_valid;  /**< Must be set to true if verbose_call_end_reason is being passed */
  qcmap_verbose_call_end_reason_enum_v01 verbose_call_end_reason;
  /**<   Reason the call ended (verbose); 
       see Table \ref{tbl:verboseCallEndReason} for the definition of these values. 
  */

  /* Optional */
  /*  Packet Service Status */
  uint8_t wwan_status_valid;  /**< Must be set to true if wwan_status is being passed */
  qcmap_wwan_status_enum_v01 wwan_status;
  /**<   If the response is QMI_ERR_NONE, this indicates the WWAN status.
       Values: \n
       - 1 -- Connecting \n
       - 2 -- Connected \n
       - 3 -- Disconnecting \n
       - 4 -- Disconnected 
  */
}qcmap_get_wwan_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Registers/deregisters the control point to receive 
           QMI_QCMAP_WWAN_STATUS_IND. 
           \label{idl:wwanStatusIndReg} */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Register Indication */
  uint8_t register_indication;
  /**<   Specifies the registration. Values: \n
       - 0 -- Do not register or deregister if already registered \n
       - 1 -- Register for the indication; ignore if already registered
  */
}qcmap_wwan_status_ind_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Registers/deregisters the control point to receive 
           QMI_QCMAP_WWAN_STATUS_IND. 
           \label{idl:wwanStatusIndReg} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_wwan_status_ind_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current mobile AP WWAN connection status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
   */

  /* Mandatory */
  /*  IP Family */
  qcmap_ip_family_enum_v01 ip_family;
  /**<   Determines whether the mobile AP is IPv4 or IPv6. Value: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */

  /* Mandatory */
  /*  Packet Service Status */
  qcmap_wwan_status_enum_v01 wwan_status;
  /**<   Indicates the WWAN status. Values: \n
       - 1 -- Connecting \n
       - 2 -- Connected \n
       - 3 -- Disconnecting \n
       - 4 -- Disconnected 
  */

  /* Mandatory */
  /*  Reconfiguration Required */
  uint8_t reconfig_required;
  /**<   Indicates whether the IP reconfiguration is required by the control 
       point.
  */

  /* Optional */
  /*  Call End Reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  qcmap_call_end_reason_enum_v01 call_end_reason;
  /**<   Reason the call ended; 
       see Table \ref{tbl:callEndReason} for the definition of these values.
  */

  /* Optional */
  /*  Verbose Call End Reason */
  uint8_t verbose_call_end_reason_valid;  /**< Must be set to true if verbose_call_end_reason is being passed */
  qcmap_verbose_call_end_reason_enum_v01 verbose_call_end_reason;
  /**<   Reason the call ended (verbose); 
       see Table \ref{tbl:verboseCallEndReason} for the definition of these values. 
  */
}qcmap_wwan_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Configures the Internet Protocol security (IPSec) 
           Virtual Private Network (VPN) passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value.
  */
}qcmap_set_ipsec_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Configures the Internet Protocol security (IPSec) 
           Virtual Private Network (VPN) passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_set_ipsec_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the IPSec VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_ipsec_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the IPSec VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value_valid;  /**< Must be set to true if vpn_pass_through_value is being passed */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value. 
  */
}qcmap_get_ipsec_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Configures the Point-to-Point Tunneling Protocol (PPTP) VPN 
           passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value. 
  */
}qcmap_set_pptp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Configures the Point-to-Point Tunneling Protocol (PPTP) VPN 
           passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_set_pptp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the PPTP VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_pptp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the PPTP VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value_valid;  /**< Must be set to true if vpn_pass_through_value is being passed */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value. 
  */
}qcmap_get_pptp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Configures the Layer 2 Tunneling Protocol (L2TP) VPN passthrough 
           setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value. 
  */
}qcmap_set_l2tp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Configures the Layer 2 Tunneling Protocol (L2TP) VPN passthrough 
           setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_set_l2tp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the L2TP VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_l2tp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the L2TP VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value_valid;  /**< Must be set to true if vpn_pass_through_value is being passed */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value. 
  */
}qcmap_get_l2tp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Sets the Network Address Translation (NAT) entry timeout. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Timeout */
  uint16_t timeout;
  /**<   NAT entry timeout. 
  */
}qcmap_set_dynamic_nat_entry_timeout_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Sets the Network Address Translation (NAT) entry timeout. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_set_dynamic_nat_entry_timeout_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the NAT entry timeout. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_dynamic_nat_entry_timeout_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the NAT entry timeout. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Timeout */
  uint8_t timeout_valid;  /**< Must be set to true if timeout is being passed */
  uint16_t timeout;
  /**<   Dynamic NAT entry timeout. 
  */
}qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Adds a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  SNAT Entry Configuration */
  qcmap_snat_entry_config_v01 snat_entry_config;
}qcmap_add_static_nat_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Adds a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_add_static_nat_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Deletes a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  SNAT Entry Configuration */
  qcmap_snat_entry_config_v01 snat_entry_config;
}qcmap_delete_static_nat_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Deletes a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_delete_static_nat_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries all static NAT entries. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_static_nat_entries_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries all static NAT entries. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  SNAT Configuration */
  uint8_t snat_config_valid;  /**< Must be set to true if snat_config is being passed */
  uint32_t snat_config_len;  /**< Must be set to # of elements in snat_config */
  qcmap_snat_entry_config_v01 snat_config[QCMAP_MAX_SNAT_ENTRIES_V01];
}qcmap_get_static_nat_entries_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Sets the DMZ (perimeter network) IP address for the mobile AP.  */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  DMZ IP Address */
  uint32_t dmz_ip_addr;
  /**<   DMZ IP address. 
  */
}qcmap_set_dmz_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Sets the DMZ (perimeter network) IP address for the mobile AP.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_set_dmz_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the DMZ IP address on the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_dmz_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the DMZ IP address on the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  DMZ IP Address */
  uint8_t dmz_ip_addr_valid;  /**< Must be set to true if dmz_ip_addr is being passed */
  uint32_t dmz_ip_addr;
  /**<   DMZ IP address. 
  */
}qcmap_get_dmz_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Deletes the DMZ entry or DMZ IP address. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_delete_dmz_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Deletes the DMZ entry or DMZ IP address. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_delete_dmz_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the WWAN IP configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Address Type  */
  qcmap_addr_type_mask_v01 addr_type_op;
  /**<   WWAN configuration mask values: \n
       - 1 -- IPv4 address \n
       - 2 -- IPv6 address \n
       - 4 -- IPv4 DNS address \n
       - 8 -- IPv6 DNS address
  */
}qcmap_get_wwan_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the WWAN IP configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IPv4 Address */
  uint8_t v4_addr_valid;  /**< Must be set to true if v4_addr is being passed */
  uint32_t v4_addr;
  /**<   IPv4 address. */

  /* Optional */
  /*  IPv6 Address */
  uint8_t v6_addr_valid;  /**< Must be set to true if v6_addr is being passed */
  uint8_t v6_addr[QCMAP_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address. */

  /* Optional */
  /*  IPv4 Primary DNS Address */
  uint8_t v4_prim_dns_addr_valid;  /**< Must be set to true if v4_prim_dns_addr is being passed */
  uint32_t v4_prim_dns_addr;
  /**<   IPv4 primary DNS address. */

  /* Optional */
  /*  IPv4 Secondary DNS Address */
  uint8_t v4_sec_dns_addr_valid;  /**< Must be set to true if v4_sec_dns_addr is being passed */
  uint32_t v4_sec_dns_addr;
  /**<   IPv4 secondary DNS address. */

  /* Optional */
  /*  IPv6 Primary DNS Address */
  uint8_t v6_prim_dns_addr_valid;  /**< Must be set to true if v6_prim_dns_addr is being passed */
  uint8_t v6_prim_dns_addr[QCMAP_IPV6_ADDR_LEN_V01];
  /**<   IPv6 primary DNS address. */

  /* Optional */
  /*  IPv6 Secondary DNS Address */
  uint8_t v6_sec_dns_addr_valid;  /**< Must be set to true if v6_sec_dns_addr is being passed */
  uint8_t v6_sec_dns_addr[QCMAP_IPV6_ADDR_LEN_V01];
  /**<   IPv6 secondary DNS address. */
}qcmap_get_wwan_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Enables the firewall setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Packets Allowed */
  uint8_t pkts_allowed;
  /**<   Packets allowed operation. Values: \n
       - TRUE -- Packets matching the firewall rule are allowed \n
       - FALSE -- Packets matching the firewall rule are dropped
  */
}qcmap_enable_firewall_setting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Enables the firewall setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_enable_firewall_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the firewall setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_firewall_setting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the firewall setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Enabled */
  uint8_t firewall_enabled_valid;  /**< Must be set to true if firewall_enabled is being passed */
  uint8_t firewall_enabled;
  /**<   Whether the firewall is enabled; boolean value. 
  */

  /* Optional */
  /*  Packets Allowed */
  uint8_t pkts_allowed_valid;  /**< Must be set to true if pkts_allowed is being passed */
  uint8_t pkts_allowed;
  /**<   Whether packets are allowed; boolean value. 
  */
}qcmap_get_firewall_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Disables the firewall setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_disable_firewall_setting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Disables the firewall setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_disable_firewall_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Adds a firewall configuration rule. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Firewall Configuration */
  qcmap_firewall_type_v01 firewall_config;
}qcmap_add_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Adds a firewall configuration rule. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Handle */
  uint8_t firewall_handle_valid;  /**< Must be set to true if firewall_handle is being passed */
  uint32_t firewall_handle;
  /**<   Handle identifying the firewall rule. 
  */
}qcmap_add_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Deletes a firewall configuration rule. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Firewall Handle */
  uint32_t firewall_handle;
  /**<   Handle identifying the firewall entry. The value must be the handle 
       previously returned by QMI_QCMAP_ADD_ FIREWALL_CONFIG_RESP or 
       QMI_QCMAP_GET_FIREWALL_ CONFIG_RESP.
  */
}qcmap_delete_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Deletes a firewall configuration rule. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_delete_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the firewall configuration rules. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the firewall configuration rules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Configuration */
  uint8_t firewall_config_valid;  /**< Must be set to true if firewall_config is being passed */
  uint32_t firewall_config_len;  /**< Must be set to # of elements in firewall_config */
  qcmap_get_firewall_list_type_v01 firewall_config[QCMAP_MAX_FIREWALL_ENTRIES_V01];
}qcmap_get_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Enables Station (STA) mode functionality for a mobile AP instance 
           on the modem. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_station_mode_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Enables Station (STA) mode functionality for a mobile AP instance 
           on the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_station_mode_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Disables STA mode functionality for a mobile AP instance on the 
           modem. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_station_mode_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Disables STA mode functionality for a mobile AP instance on the 
           modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_station_mode_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Queries the STA mode functionality for a mobile AP instance on the 
           modem. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_station_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Queries the STA mode functionality for a mobile AP instance on the 
           modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Station Mode */
  uint8_t station_mode_valid;  /**< Must be set to true if station_mode is being passed */
  uint8_t station_mode;
  /**<   Whether STA mode has been enabled; boolean value. 
  */
}qcmap_get_station_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Adds IP filter-based firewall rules (extended firewall). */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

         The value must be the handle previously returned by
         QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
    */

  /* Mandatory */
  /*  Next Header Protocol */
  qcmap_extd_firewall_protocol_enum_v01 next_hdr_prot;
  /**<   IPv4/IPv6 next header protocol after the IP header. Values: \n

      - 0x01 -- QCMAP_EXTD_FIREWALL_ PROTO_TCP -- 
                Transmission Control Protocol \n
      - 0x02 -- QCMAP_EXTD_FIREWALL_ PROTO_UDP -- 
                User Datagram Protocol \n
      - 0x03 -- QCMAP_EXTD_FIREWALL_ PROTO_ICMP -- 
                Internet Control Message Protocol \n
      - 0x04 -- QCMAP_EXTD_FIREWALL_ PROTO_ICMP6 -- 
                Internet Control Message Protocol version 6 \n
      - 0x05 -- QCMAP_EXTD_FIREWALL_ PROTO_ESP -- 
                Encapsulating Security Payload Protocol \n
      - 0x06 -- QCMAP_EXTD_FIREWALL_ PROTO_TCP_UDP -- 
                Transmission Control Protocol/User Datagram Protocol
    */

  /* Optional */
  /*  TCP/UDP Source */
  uint8_t tcp_udp_src_valid;  /**< Must be set to true if tcp_udp_src is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_src;

  /* Optional */
  /*  TCP/UDP Destination  */
  uint8_t tcp_udp_dst_valid;  /**< Must be set to true if tcp_udp_dst is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_dst;

  /* Optional */
  /*  ICMP Type */
  uint8_t icmp_type_valid;  /**< Must be set to true if icmp_type is being passed */
  uint8_t icmp_type;
  /**<   ICMP type as specified in the ICMP specification 
         (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ICMP Code */
  uint8_t icmp_code_valid;  /**< Must be set to true if icmp_code is being passed */
  uint8_t icmp_code;
  /**<   ICMP code as specified in the ICMP specification 
         (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ESP Security Parameters Index */
  uint8_t esp_spi_valid;  /**< Must be set to true if esp_spi is being passed */
  uint32_t esp_spi;
  /**<   Security parameters index as specified in the ESP protocol 
         (RFC 4303 \hyperref[S7]{[S7]}). */

  /* Optional */
  /*  IPv4 Source Address */
  uint8_t ip4_src_addr_valid;  /**< Must be set to true if ip4_src_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_src_addr;

  /* Optional */
  /*  IPv4 Destination Address */
  uint8_t ip4_dst_addr_valid;  /**< Must be set to true if ip4_dst_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_dst_addr;

  /* Optional */
  /*  IPv4 TOS */
  uint8_t ip4_tos_valid;  /**< Must be set to true if ip4_tos is being passed */
  qcmap_ip4_tos_v01 ip4_tos;

  /* Optional */
  /*  IPv6 Source Address */
  uint8_t ip6_src_addr_valid;  /**< Must be set to true if ip6_src_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_src_addr;

  /* Optional */
  /*  IPv6 Destination Address */
  uint8_t ip6_dst_addr_valid;  /**< Must be set to true if ip6_dst_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_dst_addr;

  /* Optional */
  /*  IPv6 Traffic Class */
  uint8_t ip6_trf_cls_valid;  /**< Must be set to true if ip6_trf_cls is being passed */
  qcmap_ip6_traffic_class_v01 ip6_trf_cls;
}qcmap_add_extd_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Adds IP filter-based firewall rules (extended firewall). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall handle */
  uint8_t firewall_handle_valid;  /**< Must be set to true if firewall_handle is being passed */
  uint32_t firewall_handle;
  /**<   Handle identifying the added firewall rule. */
}qcmap_add_extd_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Gets the firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by 
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Firewall Handle */
  uint32_t firewall_handle;
  /**<   Handle identifying the firewall entry.
       
       The value must be the handle previously returned by one of the 
       following: \n
       - QMI_QCMAP_ADD_FIREWALL_ CONFIG_RESP \n
       - QMI_QCMAP_GET_FIREWALL_ CONFIG_RESP \n
       - QMI_QCMAP_ADD_EXTD_ FIREWALL_CONFIG_RESP \n
       - QMI_QCMAP_GET_FIREWALL_ CONFIG_HANDLE_LIST_RESP
  */
}qcmap_get_extd_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Gets the firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Next Header Protocol */
  uint8_t next_hdr_prot_valid;  /**< Must be set to true if next_hdr_prot is being passed */
  qcmap_extd_firewall_protocol_enum_v01 next_hdr_prot;
  /**<   IPv4/IPv6 next header protocol after the IP header. Values: \n

      - 0x01 -- QCMAP_EXTD_FIREWALL_ PROTO_TCP -- 
                Transmission Control Protocol \n
      - 0x02 -- QCMAP_EXTD_FIREWALL_ PROTO_UDP -- 
                User Datagram Protocol \n
      - 0x03 -- QCMAP_EXTD_FIREWALL_ PROTO_ICMP -- 
                Internet Control Message Protocol \n
      - 0x04 -- QCMAP_EXTD_FIREWALL_ PROTO_ICMP6 -- 
                Internet Control Message Protocol for IPv6 \n
      - 0x05 -- QCMAP_EXTD_FIREWALL_ PROTO_ESP -- 
                Encapsulating Security Payload Protocol \n
      - 0x06 -- QCMAP_EXTD_FIREWALL_ PROTO_TCP_UDP -- 
                Transmission Control Protocol/User Datagram Protocol
    */

  /* Optional */
  /*  TCP/UDP Source */
  uint8_t tcp_udp_src_valid;  /**< Must be set to true if tcp_udp_src is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_src;

  /* Optional */
  /*  TCP/UDP Destination  */
  uint8_t tcp_udp_dst_valid;  /**< Must be set to true if tcp_udp_dst is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_dst;

  /* Optional */
  /*  ICMP Type */
  uint8_t icmp_type_valid;  /**< Must be set to true if icmp_type is being passed */
  uint8_t icmp_type;
  /**<   ICMP type as specified in the ICMP specification 
         (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ICMP Code */
  uint8_t icmp_code_valid;  /**< Must be set to true if icmp_code is being passed */
  uint8_t icmp_code;
  /**<   ICMP code as specified in the ICMP specification 
         (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ESP Security Parameters Index */
  uint8_t esp_spi_valid;  /**< Must be set to true if esp_spi is being passed */
  uint32_t esp_spi;
  /**<   Security parameters index as specified in the ESP protocol 
         (RFC 4303 \hyperref[S7]{[S7]}). */

  /* Optional */
  /*  IPv4 Source Address */
  uint8_t ip4_src_addr_valid;  /**< Must be set to true if ip4_src_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_src_addr;

  /* Optional */
  /*  IPv4 Destination Address */
  uint8_t ip4_dst_addr_valid;  /**< Must be set to true if ip4_dst_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_dst_addr;

  /* Optional */
  /*  IPv4 TOS */
  uint8_t ip4_tos_valid;  /**< Must be set to true if ip4_tos is being passed */
  qcmap_ip4_tos_v01 ip4_tos;

  /* Optional */
  /*  IPv6 Source Address */
  uint8_t ip6_src_addr_valid;  /**< Must be set to true if ip6_src_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_src_addr;

  /* Optional */
  /*  IPv6 Destination Address */
  uint8_t ip6_dst_addr_valid;  /**< Must be set to true if ip6_dst_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_dst_addr;

  /* Optional */
  /*  IPv6 Traffic Class */
  uint8_t ip6_trf_cls_valid;  /**< Must be set to true if ip6_trf_cls is being passed */
  qcmap_ip6_traffic_class_v01 ip6_trf_cls;
}qcmap_get_extd_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Gets the handles of all the firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_firewall_config_handle_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Gets the handles of all the firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Handle List */
  uint8_t firewall_handle_list_valid;  /**< Must be set to true if firewall_handle_list is being passed */
  uint32_t firewall_handle_list_len;  /**< Must be set to # of elements in firewall_handle_list */
  uint32_t firewall_handle_list[QCMAP_MAX_FIREWALL_ENTRIES_V01];
  /**<   Firewall handle list. */
}qcmap_get_firewall_config_handle_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Changes the currently existing NAT type. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */

  /* Optional */
  /*  NAT Type Option */
  uint8_t nat_type_option_valid;  /**< Must be set to true if nat_type_option is being passed */
  qcmap_nat_type_enum_v01 nat_type_option;
  /**<   NAT type specified for the NAT type change. Values: \n
      - 0x00 -- QCMAP_NAT_TYPE_ SYMMETRIC -- 
                Symmetric NAT\n
      - 0x01 -- QCMAP_NAT_TYPE_ PORT_RESTRICTED_CONE -- 
                 Port restricted cone NAT
    */
}qcmap_change_nat_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Changes the currently existing NAT type. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_change_nat_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Request Message; Gets the currently enabled NAT type. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.

       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_ AP_ENABLE_REQ.
  */
}qcmap_get_nat_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_qmi_messages
    @{
  */
/** Response Message; Gets the currently enabled NAT type. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Current NAT Type */
  uint8_t nat_type_option_valid;  /**< Must be set to true if nat_type_option is being passed */
  qcmap_nat_type_enum_v01 nat_type_option;
  /**<   NAT type currently on the modem. Values: \n
      - 0x00 -- QCMAP_NAT_TYPE_ SYMMETRIC -- 
                Symmetric NAT\n
      - 0x01 -- QCMAP_NAT_TYPE_ PORT_RESTRICTED_CONE -- 
                 Port restricted cone NAT
    */
}qcmap_get_nat_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup qcmap_qmi_msg_ids
    @{
  */
#define QMI_QCMAP_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_QCMAP_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_QCMAP_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_QCMAP_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_QCMAP_MOBILE_AP_ENABLE_REQ_V01 0x0020
#define QMI_QCMAP_MOBILE_AP_ENABLE_RESP_V01 0x0020
#define QMI_QCMAP_MOBILE_AP_DISABLE_REQ_V01 0x0021
#define QMI_QCMAP_MOBILE_AP_DISABLE_RESP_V01 0x0021
#define QMI_QCMAP_BRING_UP_WWAN_REQ_V01 0x0022
#define QMI_QCMAP_BRING_UP_WWAN_RESP_V01 0x0022
#define QMI_QCMAP_BRING_UP_WWAN_IND_V01 0x0022
#define QMI_QCMAP_TEAR_DOWN_WWAN_REQ_V01 0x0023
#define QMI_QCMAP_TEAR_DOWN_WWAN_RESP_V01 0x0023
#define QMI_QCMAP_TEAR_DOWN_WWAN_IND_V01 0x0023
#define QMI_QCMAP_GET_WWAN_STATUS_REQ_V01 0x0024
#define QMI_QCMAP_GET_WWAN_STATUS_RESP_V01 0x0024
#define QMI_QCMAP_GET_IPSEC_VPN_PASS_THROUGH_REQ_V01 0x0025
#define QMI_QCMAP_GET_IPSEC_VPN_PASS_THROUGH_RESP_V01 0x0025
#define QMI_QCMAP_SET_IPSEC_VPN_PASS_THROUGH_REQ_V01 0x0026
#define QMI_QCMAP_SET_IPSEC_VPN_PASS_THROUGH_RESP_V01 0x0026
#define QMI_QCMAP_GET_PPTP_VPN_PASS_THROUGH_REQ_V01 0x0027
#define QMI_QCMAP_GET_PPTP_VPN_PASS_THROUGH_RESP_V01 0x0027
#define QMI_QCMAP_SET_PPTP_VPN_PASS_THROUGH_REQ_V01 0x0028
#define QMI_QCMAP_SET_PPTP_VPN_PASS_THROUGH_RESP_V01 0x0028
#define QMI_QCMAP_GET_L2TP_VPN_PASS_THROUGH_REQ_V01 0x0029
#define QMI_QCMAP_GET_L2TP_VPN_PASS_THROUGH_RESP_V01 0x0029
#define QMI_QCMAP_SET_L2TP_VPN_PASS_THROUGH_REQ_V01 0x002A
#define QMI_QCMAP_SET_L2TP_VPN_PASS_THROUGH_RESP_V01 0x002A
#define QMI_QCMAP_GET_DYNAMIC_NAT_ENTRY_TIMEOUT_REQ_V01 0x002B
#define QMI_QCMAP_GET_DYNAMIC_NAT_ENTRY_TIMEOUT_RESP_V01 0x002B
#define QMI_QCMAP_SET_DYNAMIC_NAT_ENTRY_TIMEOUT_REQ_V01 0x002C
#define QMI_QCMAP_SET_DYNAMIC_NAT_ENTRY_TIMEOUT_RESP_V01 0x002C
#define QMI_QCMAP_ADD_STATIC_NAT_ENTRY_REQ_V01 0x002D
#define QMI_QCMAP_ADD_STATIC_NAT_ENTRY_RESP_V01 0x002D
#define QMI_QCMAP_DELETE_STATIC_NAT_ENTRY_REQ_V01 0x002E
#define QMI_QCMAP_DELETE_STATIC_NAT_ENTRY_RESP_V01 0x002E
#define QMI_QCMAP_GET_STATIC_NAT_ENTRIES_REQ_V01 0x002F
#define QMI_QCMAP_GET_STATIC_NAT_ENTRIES_RESP_V01 0x002F
#define QMI_QCMAP_SET_DMZ_REQ_V01 0x0030
#define QMI_QCMAP_SET_DMZ_RESP_V01 0x0030
#define QMI_QCMAP_DELETE_DMZ_REQ_V01 0x0031
#define QMI_QCMAP_DELETE_DMZ_RESP_V01 0x0031
#define QMI_QCMAP_GET_DMZ_REQ_V01 0x0032
#define QMI_QCMAP_GET_DMZ_RESP_V01 0x0032
#define QMI_QCMAP_GET_WWAN_CONFIG_REQ_V01 0x0033
#define QMI_QCMAP_GET_WWAN_CONFIG_RESP_V01 0x0033
#define QMI_QCMAP_ENABLE_FIREWALL_SETTING_REQ_V01 0x0034
#define QMI_QCMAP_ENABLE_FIREWALL_SETTING_RESP_V01 0x0034
#define QMI_QCMAP_GET_FIREWALL_SETTING_REQ_V01 0x0035
#define QMI_QCMAP_GET_FIREWALL_SETTING_RESP_V01 0x0035
#define QMI_QCMAP_DISABLE_FIREWALL_SETTING_REQ_V01 0x0036
#define QMI_QCMAP_DISABLE_FIREWALL_SETTING_RESP_V01 0x0036
#define QMI_QCMAP_ADD_FIREWALL_CONFIG_REQ_V01 0x0037
#define QMI_QCMAP_ADD_FIREWALL_CONFIG_RESP_V01 0x0037
#define QMI_QCMAP_GET_FIREWALL_CONFIG_REQ_V01 0x0038
#define QMI_QCMAP_GET_FIREWALL_CONFIG_RESP_V01 0x0038
#define QMI_QCMAP_DELETE_FIREWALL_CONFIG_REQ_V01 0x0039
#define QMI_QCMAP_DELETE_FIREWALL_CONFIG_RESP_V01 0x0039
#define QMI_QCMAP_WWAN_STATUS_IND_REG_REQ_V01 0x003A
#define QMI_QCMAP_WWAN_STATUS_IND_REG_RESP_V01 0x003A
#define QMI_QCMAP_STATION_MODE_ENABLE_REQ_V01 0x003B
#define QMI_QCMAP_STATION_MODE_ENABLE_RESP_V01 0x003B
#define QMI_QCMAP_STATION_MODE_DISABLE_REQ_V01 0x003C
#define QMI_QCMAP_STATION_MODE_DISABLE_RESP_V01 0x003C
#define QMI_QCMAP_GET_STATION_MODE_REQ_V01 0x003D
#define QMI_QCMAP_GET_STATION_MODE_RESP_V01 0x003D
#define QMI_QCMAP_WWAN_STATUS_IND_V01 0x003E
#define QMI_QCMAP_ADD_EXTD_FIREWALL_CONFIG_REQ_V01 0x003F
#define QMI_QCMAP_ADD_EXTD_FIREWALL_CONFIG_RESP_V01 0x003F
#define QMI_QCMAP_GET_EXTD_FIREWALL_CONFIG_REQ_V01 0x0040
#define QMI_QCMAP_GET_EXTD_FIREWALL_CONFIG_RESP_V01 0x0040
#define QMI_QCMAP_GET_FIREWALL_CONFIG_HANDLE_LIST_REQ_V01 0x0041
#define QMI_QCMAP_GET_FIREWALL_CONFIG_HANDLE_LIST_RESP_V01 0x0041
#define QMI_QCMAP_CHANGE_NAT_TYPE_REQ_V01 0x0042
#define QMI_QCMAP_CHANGE_NAT_TYPE_RESP_V01 0x0042
#define QMI_QCMAP_GET_NAT_TYPE_REQ_V01 0x0043
#define QMI_QCMAP_GET_NAT_TYPE_RESP_V01 0x0043
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qcmap_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qcmap_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qcmap_get_service_object_v01( ) \
          qcmap_get_service_object_internal_v01( \
            QCMAP_V01_IDL_MAJOR_VERS, QCMAP_V01_IDL_MINOR_VERS, \
            QCMAP_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

