#ifndef WDS_SERVICE_01_H
#define WDS_SERVICE_01_H
/**
  @file wireless_data_service_v01.h

  @brief This is the public header file which defines the wds service Data structures.

  This header file defines the types and structures that were defined in
  wds. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2006-2015 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.2 
   It was generated on: Fri Jan 30 2015 (Spin 0)
   From IDL File: wireless_data_service_v01.idl */

/** @defgroup wds_qmi_consts Constant values defined in the IDL */
/** @defgroup wds_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup wds_qmi_enums Enumerated types used in QMI messages */
/** @defgroup wds_qmi_messages Structures sent as QMI messages */
/** @defgroup wds_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup wds_qmi_accessor Accessor for QMI service object */
/** @defgroup wds_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"
#include "data_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup wds_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define WDS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define WDS_V01_IDL_MINOR_VERS 0x5B
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define WDS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define WDS_V01_MAX_MESSAGE_ID 0xFFFE
/**
    @}
  */


/** @addtogroup wds_qmi_consts 
    @{ 
  */
#define QMI_WDS_RAT_DONT_CARE_V01 0
#define QMI_WDS_RAT_NULL_BEARER_V01 0x8000
#define QMI_WDS_SO_DONT_CARE_V01 0
#define QMI_WDS_AUTH_PREF_PAP_NEVER_PERFORMED_V01 0
#define QMI_WDS_AUTH_PREF_PAP_MAY_BE_PERFORMED_V01 1
#define QMI_WDS_AUTH_PREF_CHAP_NEVER_PERFORMED_V01 0
#define QMI_WDS_AUTH_PREF_CHAP_MAY_BE_PERFORMED_V01 1
#define QMI_WDS_APN_NAME_MAX_V01 150
#define QMI_WDS_USER_NAME_MAX_V01 127
#define QMI_WDS_PASSWORD_MAX_V01 127
#define QMI_WDS_PROFILE_NAME_MAX_V01 50
#define QMI_WDS_APN_NAME_LIST_MAX_V01 20
#define QMI_WDS_MAX_NAI_LEN_V01 72
#define QMI_WDS_MAX_MN_HA_KEY_LEN_V01 16
#define QMI_WDS_MAX_MN_AAA_KEY_LEN_V01 16
#define QMI_WDS_MAX_PHONE_NUMBER_LEN_V01 100
#define QMI_WDS_MAX_CALL_HISTORY_V01 50
#define QMI_WDS_SPC_LEN_V01 6
#define QMI_WDS_IPV6_ADDR_LEN_V01 16
#define QMI_WDS_IPV4_ADDR_LEN_V01 4
#define QMI_WDS_FQDN_MAX_V01 256
#define QMI_WDS_DOMAIN_NAME_MAX_V01 256
#define QMI_WDS_PCSCF_ADDR_LIST_MAX_V01 20
#define QMI_WDS_DOMAIN_NAME_LIST_MAX_V01 15
#define QMI_WDS_FQDN_LIST_MAX_V01 15
#define QMI_WDS_PROFILE_LIST_MAX_V01 255
#define QMI_WDS_MAX_BEARER_CONTEXT_V01 8
#define QMI_WDS_EMBMS_MAX_TMGI_V01 128
#define QMI_WDS_EMBMS_MAX_EARFCN_V01 64
#define QMI_WDS_SYS_NETWORK_MAX_V01 16
#define QMI_WDS_APNS_MAX_V01 8
#define QMI_WDS_ADDITIONAL_PDN_FILTERS_MAX_V01 50
#define QMI_WDS_APP_SPECIFIC_INFO_V01 255
#define QMI_WDS_IPSEC_SA_HASH_MAX_SIZE_V01 20
#define QMI_WDS_IPSEC_SA_CRYPTO_ALGO_KEY_MAX_SIZE_V01 32
#define QMI_WDS_IPSEC_SA_AES_BLOCK_BYTE_LEN_V01 16
#define QMI_WDS_IPSEC_SA_CFG_DATA_SIZE_MAX_V01 32
#define QMI_WDS_IPSEC_SA_MAX_TS_COUNT_V01 4
#define QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01 3
#define QMI_WDS_MAX_FAILURE_TIMER_V01 6
#define QMI_WDS_MAX_DISALLOW_TIMER_V01 6
#define QMI_WDS_ATTACH_PDN_MAX_V01 56
#define QMI_WDS_EMBMS_FREQ_SAI_MAX_V01 18
#define QMI_WDS_EMBMS_SAI_LIST_MAX_V01 64
#define QMI_WDS_MAX_RUNTIME_SETTINGS_FAILURES_V01 15
#define QMI_WDS_MAX_THROUGHPUT_INFO_V01 50
#define QMI_WDS_MAX_BEARER_RLP_MAC_ID_CONTEXT_V01 50
#define QMI_WDS_MAX_APN_INFO_ARRAY_SIZE_V01 2
#define QMI_WDS_MAX_FILE_NAME_V01 100
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IP_VERSION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IP_VERSION_IPV4_V01 = 0x04, 
  WDS_IP_VERSION_IPV6_V01 = 0x06, 
  WDS_IP_VERSION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ip_version_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DEFAULT_SUBS_V01 = 0x0000, /**<  Default data subscription \n  */
  WDS_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary \n  */
  WDS_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary \n  */
  WDS_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary \n  */
  WDS_DONT_CARE_SUBS_V01 = 0x00FF, /**<  Default value used in the absence of
       explicit binding */
  WDS_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_bind_subscription_enum_v01;
/**
    @}
  */

typedef uint32_t wds_rat_cdma_mask_v01;
#define QMI_WDS_MASK_RAT_CDMA_1X_V01 ((wds_rat_cdma_mask_v01)0x01) 
#define QMI_WDS_MASK_RAT_EVDO_REVO_V01 ((wds_rat_cdma_mask_v01)0x02) 
#define QMI_WDS_MASK_RAT_EVDO_REVA_V01 ((wds_rat_cdma_mask_v01)0x04) 
#define QMI_WDS_MASK_RAT_EVDO_REVB_V01 ((wds_rat_cdma_mask_v01)0x08) 
#define QMI_WDS_MASK_RAT_EHRPD_V01 ((wds_rat_cdma_mask_v01)0x10) 
#define QMI_WDS_MASK_RAT_FMC_V01 ((wds_rat_cdma_mask_v01)0x20) 
typedef uint32_t wds_rat_umts_mask_v01;
#define QMI_WDS_MASK_RAT_WCDMA_V01 ((wds_rat_umts_mask_v01)0x01) 
#define QMI_WDS_MASK_RAT_GPRS_V01 ((wds_rat_umts_mask_v01)0x02) 
#define QMI_WDS_MASK_RAT_HSDPA_V01 ((wds_rat_umts_mask_v01)0x04) 
#define QMI_WDS_MASK_RAT_HSUPA_V01 ((wds_rat_umts_mask_v01)0x08) 
#define QMI_WDS_MASK_RAT_EDGE_V01 ((wds_rat_umts_mask_v01)0x10) 
#define QMI_WDS_MASK_RAT_LTE_V01 ((wds_rat_umts_mask_v01)0x20) 
#define QMI_WDS_MASK_RAT_HSDPA_PLUS_V01 ((wds_rat_umts_mask_v01)0x40) 
#define QMI_WDS_MASK_RAT_DC_HSDAP_PLUS_V01 ((wds_rat_umts_mask_v01)0x80) 
#define QMI_WDS_MASK_RAT_64_QAM_V01 ((wds_rat_umts_mask_v01)0x100) 
#define QMI_WDS_MASK_RAT_TDSCDMA_V01 ((wds_rat_umts_mask_v01)0x200) 
typedef uint32_t wds_so_cdma_1x_mask_v01;
#define QMI_WDS_MASK_SO_CDMA_1X_IS95_V01 ((wds_so_cdma_1x_mask_v01)0x01) 
#define QMI_WDS_MASK_SO_CDMA_1X_IS2000_V01 ((wds_so_cdma_1x_mask_v01)0x02) 
#define QMI_WDS_MASK_SO_CDMA_1X_IS2000_REL_A_V01 ((wds_so_cdma_1x_mask_v01)0x04) 
typedef uint32_t wds_so_cdma_evdo_reva_mask_v01;
#define QMI_WDS_MASK_SO_EVDO_REVA_DPA_V01 ((wds_so_cdma_evdo_reva_mask_v01)0x01) 
#define QMI_WDS_MASK_SO_EVDO_REVA_MFPA_V01 ((wds_so_cdma_evdo_reva_mask_v01)0x02) 
#define QMI_WDS_MASK_SO_EVDO_REVA_EMPA_V01 ((wds_so_cdma_evdo_reva_mask_v01)0x04) 
#define QMI_WDS_MASK_SO_EVDO_REVA_EMPA_EHRPD_V01 ((wds_so_cdma_evdo_reva_mask_v01)0x08) 
typedef uint32_t wds_so_cdma_evdo_revb_mask_v01;
#define QMI_WDS_MASK_SO_EVDO_REVB_DPA_V01 ((wds_so_cdma_evdo_revb_mask_v01)0x01) 
#define QMI_WDS_MASK_SO_EVDO_REVB_MFPA_V01 ((wds_so_cdma_evdo_revb_mask_v01)0x02) 
#define QMI_WDS_MASK_SO_EVDO_REVB_EMPA_V01 ((wds_so_cdma_evdo_revb_mask_v01)0x04) 
#define QMI_WDS_MASK_SO_EVDO_REVB_EMPA_EHRPD_V01 ((wds_so_cdma_evdo_revb_mask_v01)0x08) 
#define QMI_WDS_MASK_SO_EVDO_REVB_MMPA_V01 ((wds_so_cdma_evdo_revb_mask_v01)0x10) 
#define QMI_WDS_MASK_SO_EVDO_REVB_MMPA_EHRPD_V01 ((wds_so_cdma_evdo_revb_mask_v01)0x20) 
typedef uint8_t wds_auth_pref_mask_v01;
#define QMI_WDS_MASK_AUTH_PREF_PAP_V01 ((wds_auth_pref_mask_v01)0x01) 
#define QMI_WDS_MASK_AUTH_PREF_CHAP_V01 ((wds_auth_pref_mask_v01)0x02) 
typedef uint8_t wds_tech_pref_mask_v01;
#define QMI_WDS_MASK_TECH_PREF_3GPP_V01 ((wds_tech_pref_mask_v01)0x01) 
#define QMI_WDS_MASK_TECH_PREF_3GPP2_V01 ((wds_tech_pref_mask_v01)0x02) 
typedef uint32_t wds_stats_mask_v01;
#define QMI_WDS_MASK_STATS_TX_PACKETS_OK_V01 ((wds_stats_mask_v01)0x00000001) 
#define QMI_WDS_MASK_STATS_RX_PACKETS_OK_V01 ((wds_stats_mask_v01)0x00000002) 
#define QMI_WDS_MASK_STATS_TX_PACKET_ERRORS_V01 ((wds_stats_mask_v01)0x00000004) 
#define QMI_WDS_MASK_STATS_RX_PACKET_ERRORS_V01 ((wds_stats_mask_v01)0x00000008) 
#define QMI_WDS_MASK_STATS_TX_OVERFLOWS_V01 ((wds_stats_mask_v01)0x00000010) 
#define QMI_WDS_MASK_STATS_RX_OVERFLOWS_V01 ((wds_stats_mask_v01)0x00000020) 
#define QMI_WDS_MASK_STATS_TX_BYTES_OK_V01 ((wds_stats_mask_v01)0x00000040) 
#define QMI_WDS_MASK_STATS_RX_BYTES_OK_V01 ((wds_stats_mask_v01)0x00000080) 
#define QMI_WDS_MASK_STATS_TX_PKTS_DROPPED_V01 ((wds_stats_mask_v01)0x00000100) 
#define QMI_WDS_MASK_STATS_RX_PKTS_DROPPED_V01 ((wds_stats_mask_v01)0x00000200) 
typedef uint32_t wds_req_settings_mask_v01;
#define QMI_WDS_MASK_REQ_SETTINGS_PROFILE_ID_V01 ((wds_req_settings_mask_v01)0x00000001) 
#define QMI_WDS_MASK_REQ_SETTINGS_PROFILE_NAME_V01 ((wds_req_settings_mask_v01)0x00000002) 
#define QMI_WDS_MASK_REQ_SETTINGS_PDP_TYPE_V01 ((wds_req_settings_mask_v01)0x00000004) 
#define QMI_WDS_MASK_REQ_SETTINGS_APN_NAME_V01 ((wds_req_settings_mask_v01)0x00000008) 
#define QMI_WDS_MASK_REQ_SETTINGS_DNS_ADDR_V01 ((wds_req_settings_mask_v01)0x00000010) 
#define QMI_WDS_MASK_REQ_SETTINGS_UMTS_GPRS_GRANTED_QOS_V01 ((wds_req_settings_mask_v01)0x00000020) 
#define QMI_WDS_MASK_REQ_SETTINGS_USERNAME_V01 ((wds_req_settings_mask_v01)0x00000040) 
#define QMI_WDS_MASK_REQ_SETTINGS_AUTH_PROTOCOL_V01 ((wds_req_settings_mask_v01)0x00000080) 
#define QMI_WDS_MASK_REQ_SETTINGS_IP_ADDR_V01 ((wds_req_settings_mask_v01)0x00000100) 
#define QMI_WDS_MASK_REQ_SETTINGS_GATEWAY_INFO_V01 ((wds_req_settings_mask_v01)0x00000200) 
#define QMI_WDS_MASK_REQ_SETTINGS_PCSCF_ADDR_USING_PCO_V01 ((wds_req_settings_mask_v01)0x00000400) 
#define QMI_WDS_MASK_REQ_SETTINGS_PCSCF_SERVER_ADDR_LIST_V01 ((wds_req_settings_mask_v01)0x00000800) 
#define QMI_WDS_MASK_REQ_SETTINGS_PCSCF_DOMAIN_NAME_LIST_V01 ((wds_req_settings_mask_v01)0x00001000) 
#define QMI_WDS_MASK_REQ_SETTINGS_MTU_V01 ((wds_req_settings_mask_v01)0x00002000) 
#define QMI_WDS_MASK_REQ_SETTINGS_DOMAIN_NAME_LIST_V01 ((wds_req_settings_mask_v01)0x00004000) 
#define QMI_WDS_MASK_REQ_SETTINGS_IP_FAMILY_V01 ((wds_req_settings_mask_v01)0x00008000) 
#define QMI_WDS_MASK_REQ_SETTINGS_IM_CN_FLAG_V01 ((wds_req_settings_mask_v01)0x00010000) 
#define QMI_WDS_MASK_REQ_SETTINGS_TECHNOLOGY_NAME_V01 ((wds_req_settings_mask_v01)0x00020000) 
#define QMI_WDS_MASK_REQ_SETTINGS_OP_RES_PCO_V01 ((wds_req_settings_mask_v01)0x00040000) 
typedef uint8_t wds_dun_ctrl_event_report_mask_v01;
#define QMI_WDS_CTRL_EVENT_REPORT_DUN_CALL_COMPLETE_V01 ((wds_dun_ctrl_event_report_mask_v01)0x01) 
#define QMI_WDS_CTRL_EVENT_REPORT_DUN_ENTITLEMENT_V01 ((wds_dun_ctrl_event_report_mask_v01)0x02) 
#define QMI_WDS_CTRL_EVENT_REPORT_DUN_SILENT_REDIAL_V01 ((wds_dun_ctrl_event_report_mask_v01)0x04) 
/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Resets the WDS service state variables of the 
           requesting control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_reset_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Resets the WDS service state variables of the 
           requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}wds_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t stats_period;
  /**<   Period between transfer statistics reports. Values: \n
       - 0 -- Do not report \n
       - Other -- Period between reports (in seconds)
  */

  wds_stats_mask_v01 stats_mask;
  /**<   Requested statistic bitmask. Values: \n
       - 0x00000001 -- Tx packets OK    \n
       - 0x00000002 -- Rx packets OK    \n
       - 0x00000004 -- Tx packet errors \n
       - 0x00000008 -- Rx packet errors \n
       - 0x00000010 -- Tx overflows     \n
       - 0x00000020 -- Rx overflows     \n
       - 0x00000040 -- Tx bytes OK      \n
       - 0x00000080 -- Rx bytes OK      \n
       - 0x00000100 -- Tx packets dropped \n
       - 0x00000200 -- Rx packets dropped      
  
       Each bit set causes the corresponding optional TLV to be
       sent in QMI_WDS_EVENT_REPORT_IND.

       All unlisted bits are reserved for future use and must be set
       to zero.
  */
}wds_statistics_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the wireless data connection state reporting conditions for 
    the requesting control point. */
typedef struct {

  /* Optional */
  /*  Current Channel Rate Indicator */
  uint8_t report_channel_rate_valid;  /**< Must be set to true if report_channel_rate is being passed */
  uint8_t report_channel_rate;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report channel rate when it changes
  */

  /* Optional */
  /*  Transfer Statistics Indicator */
  uint8_t report_stats_valid;  /**< Must be set to true if report_stats is being passed */
  wds_statistics_indicator_type_v01 report_stats;

  /* Optional */
  /*  Data Bearer Technology Indicator     */
  uint8_t report_data_bearer_tech_valid;  /**< Must be set to true if report_data_bearer_tech is being passed */
  uint8_t report_data_bearer_tech;
  /**<   Note: This TLV is deprecated in QMI WDS version 1.4 and later. \n
       Values: \n
       - 0 -- Do not report \n
       - 1 -- Report radio interface used for data transfer when it
         changes
  */

  /* Optional */
  /*  Dormancy Status indicator */
  uint8_t report_dormancy_status_valid;  /**< Must be set to true if report_dormancy_status is being passed */
  uint8_t report_dormancy_status;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report traffic channel state of interface used for data
         connection
  */

  /* Optional */
  /*  MIP Status Indicator */
  uint8_t report_mip_status_valid;  /**< Must be set to true if report_mip_status is being passed */
  uint8_t report_mip_status;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report MIP status
  */

  /* Optional */
  /*  Current Data Bearer Technology Indicator */
  uint8_t report_current_data_bearer_tech_valid;  /**< Must be set to true if report_current_data_bearer_tech is being passed */
  uint8_t report_current_data_bearer_tech;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report current data bearer technology when it changes
  */

  /* Optional */
  /*  Data Call Status Change Indicator */
  uint8_t report_data_call_status_change_valid;  /**< Must be set to true if report_data_call_status_change is being passed */
  uint8_t report_data_call_status_change;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report data call status change when it changes
  */

  /* Optional */
  /*  Current Preferred Data System Indicator */
  uint8_t report_preferred_data_system_valid;  /**< Must be set to true if report_preferred_data_system is being passed */
  uint8_t report_preferred_data_system;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report preferred data system when it changes
  */

  /* Optional */
  /*  EV-DO Page Monitor Period Change Indicator */
  uint8_t report_evdo_page_monitor_period_change_valid;  /**< Must be set to true if report_evdo_page_monitor_period_change is being passed */
  uint8_t report_evdo_page_monitor_period_change;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report EV-DO page monitor period change event
  */

  /* Optional */
  /*  Data System Status Change Indicator */
  uint8_t report_data_system_status_valid;  /**< Must be set to true if report_data_system_status is being passed */
  uint8_t report_data_system_status;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report data system status change event
  */

  /* Optional */
  /*  Uplink Flow Control Indicator */
  uint8_t report_uplink_flow_control_valid;  /**< Must be set to true if report_uplink_flow_control is being passed */
  uint8_t report_uplink_flow_control;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report uplink flow control change event
  */

  /* Optional */
  /*  Limited Data System Status Change Indicator */
  uint8_t limited_data_system_status_valid;  /**< Must be set to true if limited_data_system_status is being passed */
  uint8_t limited_data_system_status;
  /**<   Values: \n
       - 0 -- Do not report limited data system status \n
       - 1 -- Report interfamily transition of data system status \n
       
       Indications for transition between RATs belonging to two different 
       families would be reported. Control points are expected to 
       deregister from Data System Status Change Indicator reporting (using TLV 0x1A) 
       and register for the Limited Data System Status Change Indicator (to only get 
       QMI_WDS_EVENT_REPORT_IND with the Data System Status TLV (0x24) for 
       interfamily system status changes). 
       
       WCDMA family : \n
       - WCDMA \n
       - HSDPA \n
       - HSUPA \n
       - HSDPA+ \n
       - DC_HSDPA+ \n
       - 64_QAM \n

       GSM family \n  
       - GPRS \n
       - EDGE \n
      
       LTE family \n
       - LTE \n
      
       TDSCDMA family \n         
       - TDSCDMA
  */

  /* Optional */
  /*  Additional PDN Filters Removal Indicator */
  uint8_t report_additional_pdn_filters_removal_valid;  /**< Must be set to true if report_additional_pdn_filters_removal is being passed */
  uint8_t report_additional_pdn_filters_removal;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report additional PDN filters removal event       
  */

  /* Optional */
  /*  Data Bearer Technology Extended Indicator */
  uint8_t report_data_bearer_tech_ex_valid;  /**< Must be set to true if report_data_bearer_tech_ex is being passed */
  uint8_t report_data_bearer_tech_ex;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report data bearer technology extended when it changes
  */
}wds_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the wireless data connection state reporting conditions for 
    the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}wds_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t current_channel_tx_rate;
  /**<   Max channel Tx rate in bits per second.*/

  uint32_t current_channel_rx_rate;
  /**<   Max channel Rx rate in bits per second.*/
}wds_channel_rate_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_CURRENT_NW_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CURRENT_NETWORK_UNKNOWN_V01 = 0x00, /**<  Unknown\n  */
  WDS_CURRENT_NETWORK_3GPP2_V01 = 0x01, /**<  3GPP2 \n  */
  WDS_CURRENT_NETWORK_3GPP_V01 = 0x02, /**<  3GPP  */
  WDS_CURRENT_NW_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_current_nw_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_current_nw_enum_v01 current_nw;
  /**<   Current network type of data bearer. Values: \n
      - WDS_CURRENT_NETWORK_UNKNOWN (0x00) --  Unknown\n 
      - WDS_CURRENT_NETWORK_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_CURRENT_NETWORK_3GPP (0x02) --  3GPP  
 */

  uint32_t rat_mask;
  /**<   RAT mask to indicate the type of technology.
       A RAT mask value of zero indicates that this field is ignored. Values: \n
       - 0x00 -- DONT_CARE \n
       - 0x8000 -- NULL_BEARER \n

       CDMA RAT mask: \n
       - 0x01 -- CDMA_1X \n
       - 0x02 -- EVDO_REV0 \n
       - 0x04 -- EVDO_REVA \n
       - 0x08 -- EVDO_REVB \n
       - 0x10 -- EHRPD \n
       - 0x20 -- FMC \n

       UMTS RAT mask: \n
       - 0x01 -- WCDMA \n
       - 0x02 -- GPRS \n
       - 0x04 -- HSDPA \n
       - 0x08 -- HSUPA \n
       - 0x10 -- EDGE \n
       - 0x20 -- LTE \n
       - 0x40 -- HSDPA+ \n
       - 0x80 -- DC_HSDPA+ \n
       - 0x100 -- 64_QAM \n
       - 0x200 -- TDSCDMA
  */

  uint32_t so_mask;
  /**<   SO mask to indicate the service option or type of
       application.

       An SO mask value of zero indicates that this field is ignored. Values: \n
       - 0x00 -- DONT_CARE \n

       CDMA 1X SO mask: \n
       - 0x01 -- CDMA_1X_IS95 \n
       - 0x02 -- CDMA_1X_IS2000 \n
       - 0x04 -- CDMA_1X_IS2000_REL_A \n

       CDMA EV-DO Rev 0 SO mask: \n
       - 0x01 -- DPA \n

       CDMA EV-DO Rev A SO mask: \n
       - 0x01 -- DPA \n
       - 0x02 -- MFPA \n
       - 0x04 -- EMPA \n
       - 0x08 -- EMPA_EHRPD \n

       CDMA EV-DO Rev B SO mask: \n
       - 0x01 -- DPA \n
       - 0x02 -- MFPA \n
       - 0x04 -- EMPA \n
       - 0x08 -- EMPA_EHRPD \n
       - 0x10 -- MMPA \n
       - 0x20 -- MMPA_EHRPD
  */
}wds_current_bearer_tech_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DATA_BEARER_TECH_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_BEARER_TECH_1X_V01 = 0x01, 
  WDS_DATA_BEARER_TECH_1XEVDO_V01 = 0x02, 
  WDS_DATA_BEARER_TECH_GSM_V01 = 0x03, 
  WDS_DATA_BEARER_TECH_UMTS_V01 = 0x04, 
  WDS_DATA_BEARER_TECH_CDMA_EVDO_REVA_V01 = 0x05, 
  WDS_DATA_BEARER_TECH_EDGE_V01 = 0x06, 
  WDS_DATA_BEARER_TECH_HSDPA_WCDMA_V01 = 0x07, 
  WDS_DATA_BEARER_TECH_WCDMA_HSUPA_V01 = 0x08, 
  WDS_DATA_BEARER_TECH_HSDPA_HSUPA_V01 = 0x09, 
  WDS_DATA_BEARER_TECH_LTE_V01 = 0x0A, 
  WDS_DATA_BEARER_TECH_CDMA_EVDO_EHRPD_V01 = 0x0B, 
  WDS_DATA_BEARER_TECH_HSDPA_PLUS_WCDMA_V01 = 0x0C, 
  WDS_DATA_BEARER_TECH_HSDPA_PLUS_HSUPA_V01 = 0x0D, 
  WDS_DATA_BEARER_TECH_DC_HSDPA_PLUS_WCDMA_V01 = 0x0E, 
  WDS_DATA_BEARER_TECH_DC_HSDPA_PLUS_HSUPA_V01 = 0x0F, 
  WDS_DATA_BEARER_TECH_HSDPA_PLUS_64QAM_V01 = 0x10, 
  WDS_DATA_BEARER_TECH_HSDPA_PLUS_64QAM_HSUPA_V01 = 0x11, 
  WDS_DATA_BEARER_TECH_TDSCDMA_V01 = 0x12, 
  WDS_DATA_BEARER_TECH_TDSCDMA_HSDPA_V01 = 0x13, 
  WDS_DATA_BEARER_TECH_TDSCDMA_HSUPA_V01 = 0x14, 
  WDS_DATA_BEARER_TECH_IWLAN_S2B_V01 = 0x15, 
  WDS_DATA_BEARER_TECH_UNKNOWN_V01 = -1, 
  WDS_DATA_BEARER_TECH_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_data_bearer_tech_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DORMANCY_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DORMANCY_STATUS_DORMANT_V01 = 0x01, 
  WDS_DORMANCY_STATUS_ACTIVE_V01 = 0x02, 
  WDS_DORMANCY_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dormancy_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EVDO_SLOT_CYCLE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EVDO_SLOT_CYCLE_6_MSECS_V01 = 0x00, 
  WDS_EVDO_SLOT_CYCLE_13_MSECS_V01 = 0x01, 
  WDS_EVDO_SLOT_CYCLE_26_MSECS_V01 = 0x02, 
  WDS_EVDO_SLOT_CYCLE_53_MSECS_V01 = 0x03, 
  WDS_EVDO_SLOT_CYCLE_106_MSECS_V01 = 0x04, 
  WDS_EVDO_SLOT_CYCLE_213_MSECS_V01 = 0x05, 
  WDS_EVDO_SLOT_CYCLE_426_MSECS_V01 = 0x06, 
  WDS_EVDO_SLOT_CYCLE_1280_MSECS_V01 = 0x07, 
  WDS_EVDO_SLOT_CYCLE_2560_MSECS_V01 = 0x08, 
  WDS_EVDO_SLOT_CYCLE_5120_MSECS_V01 = 0x09, 
  WDS_EVDO_SLOT_CYCLE_10240_MSECS_V01 = 0x0A, 
  WDS_EVDO_SLOT_CYCLE_20480_MSECS_V01 = 0x0B, 
  WDS_EVDO_SLOT_CYCLE_40960_MSECS_V01 = 0x0C, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_6_MSECS_V01 = 0x0D, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_7_MSECS_V01 = 0x0E, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_8_MSECS_V01 = 0x0F, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_9_MSECS_V01 = 0x10, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_10_MSECS_V01 = 0x11, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_11_MSECS_V01 = 0x12, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_12_MSECS_V01 = 0x13, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_13_MSECS_V01 = 0x14, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_14_MSECS_V01 = 0x15, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_15_MSECS_V01 = 0x16, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_16_MSECS_V01 = 0x17, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_17_MSECS_V01 = 0x18, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_18_MSECS_V01 = 0x19, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_19_MSECS_V01 = 0x1A, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_20_MSECS_V01 = 0x1B, 
  WDS_EVDO_SLOT_CYCLE_1280_TIMES_21_MSECS_V01 = 0x1C, 
  WDS_EVDO_SLOT_CYCLE_DEFAULT_V01 = -1, 
  WDS_EVDO_SLOT_CYCLE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_evdo_slot_cycle_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_evdo_slot_cycle_enum_v01 evdo_page_monitor_period_change;
  /**<    EV-DO slot cycle and long sleep info.
  */

  uint8_t evdo_force_long_sleep;
  /**<   Set to 1 if EV-DO is currently forced to ignore the slot cycle setting 
       and instead sleep for long periods, potentially missing pages
  */
}wds_evdo_page_monitor_period_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DATA_CALL_STATUS_CHANGE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_CALL_UNKNOWN_V01 = 0x00, 
  WDS_DATA_CALL_ACTIVATED_V01 = 0x01, 
  WDS_DATA_CALL_TERMINATED_V01 = 0x02, 
  WDS_DATA_CALL_STATUS_CHANGE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_data_call_status_change_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_CURR_PREF_DATA_SYS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CURR_PREF_SYS_UNKNOWN_V01 = 0x00, 
  WDS_CURR_PREF_SYS_CDMA_1X_V01 = 0x01, 
  WDS_CURR_PREF_SYS_EVDO_V01 = 0x02, 
  WDS_CURR_PREF_SYS_GPRS_V01 = 0x03, 
  WDS_CURR_PREF_SYS_WCDMA_V01 = 0x04, 
  WDS_CURR_PREF_SYS_LTE_V01 = 0x05, 
  WDS_CURR_PREF_SYS_TDSCDMA_V01 = 0x06, 
  WDS_CURR_PREF_DATA_SYS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_curr_pref_data_sys_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EVENT_DATA_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_CALL_NONE_V01 = 0x00, 
  WDS_DATA_CALL_EMBEDDED_V01 = 0x01, 
  WDS_DATA_CALL_TETHERED_V01 = 0x02, 
  WDS_DATA_CALL_MODEM_EMBEDDED_V01 = 0x03, 
  WDS_EVENT_DATA_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_event_data_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EVENT_TETHERED_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TETHERED_CALL_NONE_V01 = 0x00, 
  WDS_TETHERED_CALL_RMNET_V01 = 0x01, 
  WDS_TETHERED_CALL_DUN_V01 = 0x02, 
  WDS_EVENT_TETHERED_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_event_tethered_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_event_data_call_type_enum_v01 data_call_type;
  /**<   Values: \n
       - 0x01 -- Embedded call (application) \n
       - 0x02 -- Tethered call \n
       - 0x03 -- Modem embedded call
  */

  wds_event_tethered_call_type_enum_v01 tethered_call_type;
  /**<   Values: \n
       - 0x00 -- Non-tethered call \n
       - 0x01 -- RmNet call \n
       - 0x02 -- DUN call
  */
}wds_event_data_call_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_SYS_NETWORK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_SYS_NETWORK_3GPP_V01 = 0x00, 
  WDS_SYS_NETWORK_3GPP2_V01 = 0x01, 
  WDS_SYS_NETWORK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_sys_network_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_sys_network_enum_v01 network;
  /**<   Values: \n
       - 0 -- 3GPP \n
       - 1 -- 3GPP2
  */

  uint32_t rat_mask;
  /**<   RAT mask to indicate the type of technology.

       A RAT mask value of zero indicates that this field is ignored. Values: \n
       - 0x00 -- DONT_CARE \n
       - 0x8000 -- NULL_BEARER \n

       CDMA RAT mask: \n
       - 0x01 -- CDMA_1X \n
       - 0x02 -- EVDO_REV0 \n
       - 0x04 -- EVDO_REVA \n
       - 0x08 -- EVDO_REVB \n
       - 0x10 -- EHRPD \n
       - 0x20 -- FMC \n

       UMTS RAT mask: \n
       - 0x01   -- WCDMA \n
       - 0x02   -- GPRS \n
       - 0x04   -- HSDPA \n
       - 0x08   -- HSUPA \n
       - 0x10   -- EDGE \n
       - 0x20   -- LTE \n
       - 0x40   -- HSDPA+ \n
       - 0x80   -- DC_HSDPA+ \n
       - 0x100  -- 64_QAM \n
       - 0x200  -- TDSCDMA
  */

  uint32_t so_mask;
  /**<   SO mask to indicate the service option or type of
       application.

       An SO mask value of zero indicates that this field is ignored.
       Values: \n
       - 0x00 -- DONT_CARE \n

       CDMA 1X SO mask: \n
       - 0x01 -- CDMA_1X_IS95 \n
       - 0x02 -- CDMA_1X_IS2000 \n
       - 0x04 -- CDMA_1X_IS2000_REL_A \n

       CDMA EV-DO Rev 0 SO mask: \n
       - 0x01 -- DPA \n

       CDMA EV-DO Rev A SO mask: \n
       - 0x01 -- DPA \n
       - 0x02 -- MFPA \n
       - 0x04 -- EMPA \n
       - 0x08 -- EMPA_EHRPD \n

       CDMA EV-DO Rev B SO mask: \n
       - 0x01 -- DPA \n
       - 0x02 -- MFPA \n
       - 0x04 -- EMPA \n
       - 0x08 -- EMPA_EHRPD \n
       - 0x10 -- MMPA \n
       - 0x20 -- MMPA_EHRPD
  */
}wds_sys_network_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_sys_network_enum_v01 preferred_network;
  /**<   Values: \n
       - 0 -- 3GPP \n
       - 1 -- 3GPP2
  */

  uint32_t network_info_len;  /**< Must be set to # of elements in network_info */
  wds_sys_network_info_type_v01 network_info[QMI_WDS_SYS_NETWORK_MAX_V01];
}wds_data_system_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DATA_CALL_ADDR_FAMILY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_CALL_ADDR_FAMILY_UNKNOWN_V01 = 0, 
  WDS_DATA_CALL_ADDR_FAMILY_IPV4_V01 = 4, 
  WDS_DATA_CALL_ADDR_FAMILY_IPV6_V01 = 6, 
  WDS_DATA_CALL_ADDR_FAMILY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_data_call_addr_family_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_BEARER_TECH_NETWORK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_BEARER_TECH_NETWORK_3GPP_V01 = 0, /**<  3GPP \n  */
  WDS_BEARER_TECH_NETWORK_3GPP2_V01 = 1, /**<  3GPP2  */
  WDS_BEARER_TECH_NETWORK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_bearer_tech_network_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_BEARER_TECH_RAT_EX_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_BEARER_TECH_RAT_EX_NULL_BEARER_V01 = 0x00, /**<  NULL bearer  */
  WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA_V01 = 0x01, /**<  3GPP WCDMA  */
  WDS_BEARER_TECH_RAT_EX_3GPP_GERAN_V01 = 0x02, /**<  3GPP GERAN  */
  WDS_BEARER_TECH_RAT_EX_3GPP_LTE_V01 = 0x03, /**<  3GPP LTE  */
  WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA_V01 = 0x04, /**<  3GPP TDSCDMA */
  WDS_BEARER_TECH_RAT_EX_3GPP_WLAN_V01 = 0x05, /**<  3GPP WLAN  */
  WDS_BEARER_TECH_RAT_EX_3GPP_MAX_V01 = 0x64, /**<  3GPP maximum  */
  WDS_BEARER_TECH_RAT_EX_3GPP2_1X_V01 = 0x65, /**<  3GPP2 1X  */
  WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD_V01 = 0x66, /**<  3GPP2 HRPD  */
  WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD_V01 = 0x67, /**<  3GPP2 EHRPD  */
  WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN_V01 = 0x68, /**<  3GPP2 WLAN  */
  WDS_BEARER_TECH_RAT_EX_3GPP2_MAX_V01 = 0xC8, /**<  3GPP2 maximum  */
  WDS_BEARER_TECH_RAT_EX_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_bearer_tech_rat_ex_enum_v01;
/**
    @}
  */

typedef uint64_t wds_bearer_tech_so_mask_v01;
#define QMI_WDS_SO_MASK_UNSPECIFIED_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000000ull) 
#define QMI_WDS_3GPP_SO_MASK_WCDMA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000001ull) 
#define QMI_WDS_3GPP_SO_MASK_HSDPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000002ull) 
#define QMI_WDS_3GPP_SO_MASK_HSUPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000004ull) 
#define QMI_WDS_3GPP_SO_MASK_HSDPAPLUS_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000008ull) 
#define QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000010ull) 
#define QMI_WDS_3GPP_SO_MASK_64_QAM_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000020ull) 
#define QMI_WDS_3GPP_SO_MASK_HSPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000040ull) 
#define QMI_WDS_3GPP_SO_MASK_GPRS_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000080ull) 
#define QMI_WDS_3GPP_SO_MASK_EDGE_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000100ull) 
#define QMI_WDS_3GPP_SO_MASK_GSM_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000200ull) 
#define QMI_WDS_3GPP_SO_MASK_S2B_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000400ull) 
#define QMI_WDS_3GPP_SO_MASK_LTE_LIMITED_SRVC_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000000800ull) 
#define QMI_WDS_3GPP_SO_MASK_LTE_FDD_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000001000ull) 
#define QMI_WDS_3GPP_SO_MASK_LTE_TDD_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000002000ull) 
#define QMI_WDS_3GPP_SO_MASK_TDSCDMA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000004000ull) 
#define QMI_WDS_3GPP_SO_MASK_DC_HSUPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000000008000ull) 
#define QMI_WDS_3GPP2_SO_MASK_1X_IS95_V01 ((wds_bearer_tech_so_mask_v01)0x0000000001000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_1X_IS2000_V01 ((wds_bearer_tech_so_mask_v01)0x0000000002000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_1X_IS2000_REL_A_V01 ((wds_bearer_tech_so_mask_v01)0x0000000004000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REV0_DPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000008000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVA_DPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000010000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVB_DPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000020000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVA_MPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000040000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000080000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVA_EMPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000100000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVB_EMPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000200000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MMPA_V01 ((wds_bearer_tech_so_mask_v01)0x0000000400000000ull) 
#define QMI_WDS_3GPP2_SO_MASK_HDR_EVDO_FMC_V01 ((wds_bearer_tech_so_mask_v01)0x0000000800000000ull) 
/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_bearer_tech_network_enum_v01 technology;
  /**<   Technology type. Values: \n 
      - WDS_BEARER_TECH_NETWORK_3GPP (0) --  3GPP \n 
      - WDS_BEARER_TECH_NETWORK_3GPP2 (1) --  3GPP2 
 */

  wds_bearer_tech_rat_ex_enum_v01 rat_value;
  /**<   RAT value. Values: \n
      - WDS_BEARER_TECH_RAT_EX_NULL_BEARER (0x00) --  NULL bearer 
      - WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA (0x01) --  3GPP WCDMA 
      - WDS_BEARER_TECH_RAT_EX_3GPP_GERAN (0x02) --  3GPP GERAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP_LTE (0x03) --  3GPP LTE 
      - WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA (0x04) --  3GPP TDSCDMA
      - WDS_BEARER_TECH_RAT_EX_3GPP_WLAN (0x05) --  3GPP WLAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP_MAX (0x64) --  3GPP maximum 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_1X (0x65) --  3GPP2 1X 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD (0x66) --  3GPP2 HRPD 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD (0x67) --  3GPP2 EHRPD 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN (0x68) --  3GPP2 WLAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_MAX (0xC8) --  3GPP2 maximum  
 */

  wds_bearer_tech_so_mask_v01 so_mask;
  /**<   SO mask to indicate the service option or type of
       application.

       An SO mask value of zero indicates that this field is ignored.
       Values: \n
       - 0x00 -- SO mask unspecified \n     

       3GPP SO mask: \n
       - 0x01  -- WCDMA \n
       - 0x02  -- HSDPA \n
       - 0x04  -- HSUPA \n
       - 0x08  -- HSDPAPLUS \n
       - 0x10  -- DC HSDPAPLUS \n
       - 0x20  -- 64 QAM \n
       - 0x40  -- HSPA \n
       - 0x80  -- GPRS \n
       - 0x100 -- EDGE \n
       - 0x200 -- GSM \n
       - 0x400 -- S2B \n
       - 0x800 -- LTE limited service \n
       - 0x1000 -- LTE FDD \n
       - 0x2000 -- LTE TDD \n
       - 0x4000 -- TDSCDMA \n
       - 0x8000 -- DC HSUPA \n
       
       3GPP2 SO mask: \n
       - 0x01000000   -- 1X IS95 \n
       - 0x02000000   -- 1X IS2000 \n
       - 0x04000000   -- 1X IS2000 REL A \n
       - 0x08000000   -- HDR REV0 DPA \n
       - 0x10000000   -- HDR REVA DPA \n
       - 0x20000000   -- HDR REVB DPA \n
       - 0x40000000   -- HDR REVA MPA \n
       - 0x80000000   -- HDR REVB MPA \n
       - 0x100000000  -- HDR REVA EMPA \n
       - 0x200000000  -- HDR REVB EMPA \n
       - 0x400000000  -- HDR REVB MMPA \n
       - 0x800000000  -- HDR EVDO FMC       
  */
}wds_bearer_tech_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the WDS connection related state change. */
typedef struct {

  /* Optional */
  /*  Tx Packets OK */
  uint8_t tx_ok_count_valid;  /**< Must be set to true if tx_ok_count is being passed */
  uint32_t tx_ok_count;
  /**<   Number of packets transmitted without error.*/

  /* Optional */
  /*  Rx Packets OK */
  uint8_t rx_ok_count_valid;  /**< Must be set to true if rx_ok_count is being passed */
  uint32_t rx_ok_count;
  /**<   Number of packets received without error.*/

  /* Optional */
  /*  Tx Packet Errors */
  uint8_t tx_err_count_valid;  /**< Must be set to true if tx_err_count is being passed */
  uint32_t tx_err_count;
  /**<   Number of outgoing packets with framing errors.*/

  /* Optional */
  /*  Rx Packet Errors */
  uint8_t rx_err_count_valid;  /**< Must be set to true if rx_err_count is being passed */
  uint32_t rx_err_count;
  /**<   Number of incoming packets with framing errors.*/

  /* Optional */
  /*  Tx Overflows */
  uint8_t tx_ofl_count_valid;  /**< Must be set to true if tx_ofl_count is being passed */
  uint32_t tx_ofl_count;
  /**<   Number of packets dropped because Tx buffer overflowed (out of memory).*/

  /* Optional */
  /*  Rx Overflows */
  uint8_t rx_ofl_count_valid;  /**< Must be set to true if rx_ofl_count is being passed */
  uint32_t rx_ofl_count;
  /**<   Number of packets dropped because Rx buffer overflowed (out of memory).*/

  /* Optional */
  /*  Channel Rate */
  uint8_t channel_rate_valid;  /**< Must be set to true if channel_rate is being passed */
  wds_channel_rate_type_v01 channel_rate;

  /* Optional */
  /*  Data Bearer Technology  */
  uint8_t data_bearer_tech_valid;  /**< Must be set to true if data_bearer_tech is being passed */
  wds_data_bearer_tech_enum_v01 data_bearer_tech;
  /**<   Note: This TLV is deprecated in QMI_WDS version 1.4 and later. \n
        Values: \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X             \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM                     \n
       - 0x04 -- UMTS                    \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE                    \n
       - 0x07 -- HSDPA and WCDMA         \n
       - 0x08 -- WCDMA and HSUPA         \n
       - 0x09 -- HSDPA and HSUPA         \n
       - 0x0A -- LTE                     \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD          \n
       - 0x0C -- HSDPA+ and WCDMA        \n
       - 0x0D -- HSDPA+ and HSUPA        \n
       - 0x0E -- DC_HSDPA+ and WCDMA     \n
       - 0x0F -- DC_HSDAP+ and HSUPA     \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - -1   -- Unknown                 
  */

  /* Optional */
  /*  Dormancy Status */
  uint8_t dormancy_status_valid;  /**< Must be set to true if dormancy_status is being passed */
  wds_dormancy_status_enum_v01 dormancy_status;
  /**<   Values: \n
       - 1 -- Traffic channel dormant \n
       - 2 -- Traffic channel active
  */

  /* Optional */
  /*  Tx Bytes OK */
  uint8_t tx_ok_bytes_count_valid;  /**< Must be set to true if tx_ok_bytes_count is being passed */
  uint64_t tx_ok_bytes_count;
  /**<   Number of bytes transmitted without error*/

  /* Optional */
  /*  Rx Bytes OK */
  uint8_t rx_ok_bytes_count_valid;  /**< Must be set to true if rx_ok_bytes_count is being passed */
  uint64_t rx_ok_bytes_count;
  /**<   Number of bytes received without error*/

  /* Optional */
  /*  MIP Status */
  uint8_t mip_status_valid;  /**< Must be set to true if mip_status is being passed */
  uint8_t mip_status;
  /**<   Status of the last MIP call (or attempt). Values: \n
       - 0x00 -- Success \n
       - 0 -- Error code (as defined in \hyperref[S4]{[S4]})
  */

  /* Optional */
  /*  Current Data Bearer Technology */
  uint8_t current_bearer_tech_valid;  /**< Must be set to true if current_bearer_tech is being passed */
  wds_current_bearer_tech_type_v01 current_bearer_tech;

  /* Optional */
  /*  Data Call Status Change */
  uint8_t data_call_status_valid;  /**< Must be set to true if data_call_status is being passed */
  wds_data_call_status_change_enum_v01 data_call_status;
  /**<   Values: \n
       - 0x01 -- Data call activated \n
       - 0x02 -- Data call terminated
  */

  /* Optional */
  /*  Current Preferred Data System */
  uint8_t current_sys_valid;  /**< Must be set to true if current_sys is being passed */
  wds_curr_pref_data_sys_enum_v01 current_sys;
  /**<   Values: \n
       - 0x00 -- Unknown \n
       - 0x01 -- CMDA_1X \n
       - 0x02 -- EVDO \n
       - 0x03 -- GPRS \n
       - 0x04 -- WCDMA \n
       - 0x05 -- LTE   \n
       - 0x06 -- TDSCDMA
  */

  /* Optional */
  /*  Data Call Type */
  uint8_t data_call_type_valid;  /**< Must be set to true if data_call_type is being passed */
  wds_event_data_call_type_v01 data_call_type;

  /* Optional */
  /*  EV-DO Page Monitor Period Change */
  uint8_t evdo_page_monitor_period_change_valid;  /**< Must be set to true if evdo_page_monitor_period_change is being passed */
  wds_evdo_page_monitor_period_type_v01 evdo_page_monitor_period_change;

  /* Optional */
  /*  Data System Status */
  uint8_t system_status_valid;  /**< Must be set to true if system_status is being passed */
  wds_data_system_status_type_v01 system_status;

  /* Optional */
  /*  Tx Packets Dropped */
  uint8_t tx_dropped_count_valid;  /**< Must be set to true if tx_dropped_count is being passed */
  uint32_t tx_dropped_count;
  /**<   Number of outgoing packets dropped.*/

  /* Optional */
  /*  Rx Packets Dropped */
  uint8_t rx_dropped_count_valid;  /**< Must be set to true if rx_dropped_count is being passed */
  uint32_t rx_dropped_count;
  /**<   Number of incoming packets dropped.*/

  /* Optional */
  /*  Uplink Flow Control */
  uint8_t uplink_flow_control_valid;  /**< Must be set to true if uplink_flow_control is being passed */
  uint8_t uplink_flow_control;
  /**<   Uplink flow control status. Values: \n
       - 0 -- Not flow controlled \n
       - 1 -- Flow controlled
   */

  /* Optional */
  /*  Data Call Address Family */
  uint8_t data_call_addr_family_valid;  /**< Must be set to true if data_call_addr_family is being passed */
  wds_data_call_addr_family_enum_v01 data_call_addr_family;
  /**<   Data call address family. This TLV is sent in conjunction with
       the Data Call Status Change TLV (0x1F) to indicate the IP family type
       of the call activated or terminated. \n
       - 0 -- Unknown \n
       - 4 -- IPv4 \n
       - 6 -- IPv6 \n
       Note: For legacy control points that do not bind to an IP type
       or do not specify the IP type when bringing up a call, unknown is
       returned if the call fails.
   */

  /* Optional */
  /*  Additional PDN Filters Removed */
  uint8_t removed_filter_handles_valid;  /**< Must be set to true if removed_filter_handles is being passed */
  uint32_t removed_filter_handles_len;  /**< Must be set to # of elements in removed_filter_handles */
  uint32_t removed_filter_handles[QMI_WDS_ADDITIONAL_PDN_FILTERS_MAX_V01];
  /**<   Removed filter handles. This TLV contains the list of all removed 
       filters that were set by the client on the RmNet port. Each filter is
       identified by a filter handle.
   */

  /* Optional */
  /*  Data Bearer Technology Extended */
  uint8_t bearer_tech_ex_valid;  /**< Must be set to true if bearer_tech_ex is being passed */
  wds_bearer_tech_info_type_v01 bearer_tech_ex;

  /* Optional */
  /*  Uplink Flow Control Sequence Number */
  uint8_t uplink_fc_seq_num_valid;  /**< Must be set to true if uplink_fc_seq_num is being passed */
  uint16_t uplink_fc_seq_num;
  /**<   Sequence number of each flow enable and disable event. This TLV
       is sent with the Uplink Flow Control TLV.
       Each time the flow is disabled (flow controlled), the sequence number
       is increased. It can be used in conjunction with the QMAP in-band flow
       control sequence number to determine the validity of the message 
       received by the control point.
  */
}wds_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Aborts a previously issued QMI_WDS command. */
typedef struct {

  /* Mandatory */
  /*  TX_ID */
  uint16_t tx_id;
  /**<   Transaction ID of the request to be aborted.*/
}wds_abort_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Aborts a previously issued QMI_WDS command. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_abort_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different QMI_WDS indications
           for the requesting control point. */
typedef struct {

  /* Optional */
  /*  eMBMS TMGI List */
  uint8_t report_embms_tmgi_list_valid;  /**< Must be set to true if report_embms_tmgi_list is being passed */
  uint8_t report_embms_tmgi_list;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report eMBMS TMGI list
  */

  /* Optional */
  /*  Suppress Packet Service Status Indication */
  uint8_t suppress_pkt_srvc_ind_valid;  /**< Must be set to true if suppress_pkt_srvc_ind is being passed */
  uint8_t suppress_pkt_srvc_ind;
  /**<   Whether to suppress the packet service status indication. Values:  \n
       - 0 -- Do not suppress \n
       - 1 -- Suppress QMI_WDS_PKT_SRVC_STATUS_IND
  */

  /* Optional */
  /*  Extended IP Configuration Change */
  uint8_t report_extended_ip_config_change_valid;  /**< Must be set to true if report_extended_ip_config_change is being passed */
  uint8_t report_extended_ip_config_change;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report extended IP configuration information change
  */

  /* Optional */
  /*  Changed LTE Attach PDN List */
  uint8_t report_lte_attach_pdn_list_change_valid;  /**< Must be set to true if report_lte_attach_pdn_list_change is being passed */
  uint8_t report_lte_attach_pdn_list_change;
  /**<   Whether to report a changed LTE attach PDN list. Values: \n
 	   - 0 -- Do not report \n
       - 1 -- Report changed LTE attach PDN list
   */

  /* Optional */
  /*  Report Reverse IP Transport Filter Setup */
  uint8_t report_reverse_ip_transport_filter_setup_valid;  /**< Must be set to true if report_reverse_ip_transport_filter_setup is being passed */
  uint8_t report_reverse_ip_transport_filter_setup;
  /**<   Whether to report a reverse IP transport filter setup. Values: \n
       - 0 -- Do not report \n
       - 1 -- Report reverse IP transport filter setup
  */

  /* Optional */
  /*  Report Handoff Information */
  uint8_t report_handoff_information_valid;  /**< Must be set to true if report_handoff_information is being passed */
  uint8_t report_handoff_information;
  /**<   Whether to report handoff information. Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report handoff information
  
 TLV 0x16 is reserved. 
  */

  /* Optional */
  /*  Report eMBMS SAI List Changes */
  uint8_t report_embms_sai_list_valid;  /**< Must be set to true if report_embms_sai_list is being passed */
  uint8_t report_embms_sai_list;
  /**<   Whether to report an eMBMS SAI list change. Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report 
  */

  /* Optional */
  /*  Report Uplink Throughput Information */
  uint8_t report_throughput_valid;  /**< Must be set to true if report_throughput is being passed */
  uint8_t report_throughput;
  /**<    Values: \n
       - 0 -- Do not report \n
       - 1 -- Report throughput information
  */

  /* Optional */
  /*  Report Profile Changes */
  uint8_t report_profile_changed_events_valid;  /**< Must be set to true if report_profile_changed_events is being passed */
  uint8_t report_profile_changed_events;
  /**<    Values: \n
       - 0 -- Do not report \n
       - 1 -- Report profile changed events
  */

  /* Optional */
  /*  Report APN List in Roaming */
  uint8_t report_roaming_apn_list_valid;  /**< Must be set to true if report_roaming_apn_list is being passed */
  uint8_t report_roaming_apn_list;
  /**<    Values: \n
       - 0 -- Do not report \n
       - 1 -- Report the list of APNs in Roaming mode
  */

  /* Optional */
  /*  Report Downlink throughput info */
  uint8_t report_dl_throughput_valid;  /**< Must be set to true if report_dl_throughput is being passed */
  uint8_t report_dl_throughput;
  /**<    Values: \n
       - 0 -- Do not report \n
       - 1 -- Report Downlink throughput info
  */

  /* Optional */
  /*  Report eMBMS Content Description Control */
  uint8_t report_embms_content_desc_control_valid;  /**< Must be set to true if report_embms_content_desc_control is being passed */
  uint8_t report_embms_content_desc_control;
  /**<    Values: \n
       - 0 -- Do not report \n
       - 1 -- Report eMBMS content description control
  */

  /* Optional */
  /*  Report Policy Ready */
  uint8_t report_policy_ready_valid;  /**< Must be set to true if report_policy_ready is being passed */
  uint8_t report_policy_ready;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report policy ready 
  */

  /* Optional */
  /*  Report APN Parameter Change information */
  uint8_t report_apn_param_change_info_valid;  /**< Must be set to true if report_apn_param_change_info is being passed */
  uint8_t report_apn_param_change_info;
}wds_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different QMI_WDS indications
           for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IP_FAMILY_PREFERENCE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IP_FAMILY_PREF_IPV4_V01 = 0x04, 
  WDS_IP_FAMILY_PREF_IPV6_V01 = 0x06, 
  WDS_IP_FAMILY_PREF_UNSPEC_V01 = 0x08, 
  WDS_IP_FAMILY_PREFERENCE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ip_family_preference_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EXT_TECH_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EXT_TECH_PREF_CDMA_V01 = -32767, 
  WDS_EXT_TECH_PREF_UMTS_V01 = -32764, 
  WDS_EXT_TECH_PREF_EMBMS_V01 = -30590, 
  WDS_EXT_TECH_PREF_MODEM_LINK_LOCAL_V01 = -30584, 
  WDS_EXT_TECH_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ext_tech_pref_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CALL_TYPE_LAPTOP_CALL_V01 = 0x00, 
  WDS_CALL_TYPE_EMBEDDED_CALL_V01 = 0x01, 
  WDS_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IP_SUPPORT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IP_SUPPORT_TYPE_IPV4_V01 = 0x00, /**<  IPv4 \n  */
  WDS_IP_SUPPORT_TYPE_IPV6_V01 = 0x01, /**<  IPv6 \n  */
  WDS_IP_SUPPORT_TYPE_IPV4V6_V01 = 0x02, /**<  IPv4v6  */
  WDS_IP_SUPPORT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ip_support_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ipv4_addr;
  /**<   PDN's IPv4 address.
  */

  uint8_t ipv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   PDN's IPv6 address.
  */

  wds_ip_support_type_enum_v01 bearer_ip_type;
  /**<   Type of bearer IP. Values: \n
      - WDS_IP_SUPPORT_TYPE_IPV4 (0x00) --  IPv4 \n 
      - WDS_IP_SUPPORT_TYPE_IPV6 (0x01) --  IPv6 \n 
      - WDS_IP_SUPPORT_TYPE_IPV4V6 (0x02) --  IPv4v6 
 */
}wds_handoff_context_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_APN_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_APN_TYPE_UNSPECIFIED_V01 = 0, /**<  APN type unspecified  */
  WDS_APN_TYPE_INTERNET_V01 = 1, /**<  APN type for Internet traffic  */
  WDS_APN_TYPE_IMS_V01 = 2, /**<  APN type for IMS  */
  WDS_APN_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_apn_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Activates a packet data session (if not already 
           started) on behalf of the requesting control point. */
typedef struct {

  /* Optional */
  /*  Primary DNS Address Preference */
  uint8_t primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if primary_DNS_IPv4_address_preference is being passed */
  uint32_t primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the network;
       if not specified, the wireless device attempts to obtain the DNS
       address automatically from the network. The negotiated value is provided
       to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS Address Preference */
  uint8_t secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if secondary_DNS_IPv4_address_preference is being passed */
  uint32_t secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the network;
       if not specified, the wireless device attempts to obtain the DNS
       address automatically from the network. The negotiated value is provided
       to the host via DHCP.
  */

  /* Optional */
  /*  Primary NetBIOS Name Server (NBNS) Address Preference */
  uint8_t primary_nbns_address_pref_valid;  /**< Must be set to true if primary_nbns_address_pref is being passed */
  uint32_t primary_nbns_address_pref;
  /**<   The primary NBNS address. The specified IPv4 address is requested as the
       primary NBNS server during data session establishment. If it is not
       provided, the primary NBNS server address is obtained automatically
       from the network. The result of negotiation (the assigned address) is
       provided to the host via DHCP
  */

  /* Optional */
  /*  Secondary NBNS Address Preference */
  uint8_t secondary_nbns_address_pref_valid;  /**< Must be set to true if secondary_nbns_address_pref is being passed */
  uint32_t secondary_nbns_address_pref;
  /**<   The secondary NetBIOS name server address. The specified IPv4 address is
       requested as the secondary NBNS server during data session
       establishment. If not provided, the secondary NBNS server address
       is obtained automatically from the network. The result of negotiation
       (the assigned address) is provided to the host via DHCP.
  */

  /* Optional */
  /*  Context Access Point Node (APN) Name */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String parameter that is a logical name used to
       select the GGSN and external packet data network. If the value is NULL 
	   or omitted, the subscription default value is requested.

       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.

       This TLV is ignored if the 3GPP-configured profile TLV is
       present, i.e., the APN name cannot be overridden.
  */

  /* Optional */
  /*  IP Address Preference */
  uint8_t ipv4_address_pref_valid;  /**< Must be set to true if ipv4_address_pref is being passed */
  uint32_t ipv4_address_pref;
  /**<   The preferred IPv4 address to be assigned to the TE. The actual assigned
       address is negotiated with the network and may differ from this value.
       If not specified, the IPv4 Address is obtained automatically from the
       network. The assigned value is provided to the host via DHCP.
  */

  /* Optional */
  /*  Authentication Preference */
  uint8_t authentication_preference_valid;  /**< Must be set to true if authentication_preference is being passed */
  wds_auth_pref_mask_v01 authentication_preference;
  /**<   A bit map that indicates the authentication algorithm preference. Values: \n
       Bit 0 -- PAP preference: \n
       - 0 -- PAP is never performed \n
       - 1 -- PAP may be performed \n
       Bit 1 -- CHAP preference: \n
       - 0 -- CHAP is never performed \n
       - 1 -- CHAP may be performed \n
       All other bits are reserved and ignored even if they are set
       in the request.

       If more than one bit is set, the device decides which
       authentication procedure is performed while setting up the data
       session. For example, the device may have a policy to select
       the most secure authentication mechanism.
  */

  /* Optional */
  /*  Username */
  uint8_t username_valid;  /**< Must be set to true if username is being passed */
  char username[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   The username to be used during data network authentication.  
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Password */
  uint8_t password_valid;  /**< Must be set to true if password is being passed */
  char password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  IP Family Preference */
  uint8_t ip_family_preference_valid;  /**< Must be set to true if ip_family_preference is being passed */
  wds_ip_family_preference_enum_v01 ip_family_preference;
  /**<   If this TLV is absent, the device attempts to bring up a call
       on default IP preference (currently IPv4, to maintain current
       behavioral backward compatability). Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6 \n
       - 8 -- Unspecified 
  */

  /* Optional */
  /*  Technology Preference */
  uint8_t technology_preference_valid;  /**< Must be set to true if technology_preference is being passed */
  wds_tech_pref_mask_v01 technology_preference;
  /**<   Bitmap that indicates the technology preference. A single connection
       is attempted using the following specified technology preferences: \n
       - Bit 0 -- 3GPP \n
       - Bit 1 -- 3GPP2 \n
       All other bits are reserved and ignored even if they are set
       in the request. If a single value of the technology preference bitmask 
	   is set, the device attempts to use that technology. If two
       or more bits in the technology preference bitmask are set, the
       device determines which technology to use from those specified. If
       this TLV is absent, the device assumes all supported technologies
       are acceptable.
   */

  /* Optional */
  /*  3GPP Configured Profile Identifier */
  uint8_t profile_index_valid;  /**< Must be set to true if profile_index is being passed */
  uint8_t profile_index;
  /**<   The index of the configured profile on which data call parameters
       are based (other TLVs present override the profile settings).
       If this TLV is not present, the data call parameters are based on
       device default settings for each parameter.
  */

  /* Optional */
  /*  3GPP2 Configured Profile Identifier */
  uint8_t profile_index_3gpp2_valid;  /**< Must be set to true if profile_index_3gpp2 is being passed */
  uint8_t profile_index_3gpp2;
  /**<   Index of the configured profile on which data call parameters
       are based (other TLVs present override the profile settings).
       If this TLV is not present, data call parameters are based on
       device default settings for each parameter.
  */

  /* Optional */
  /*  Enable Autoconnect */
  uint8_t enable_autoconnect_valid;  /**< Must be set to true if enable_autoconnect is being passed */
  uint8_t enable_autoconnect;
  /**<   If set to 1 (TRUE), the device attempts to bring up
       a call automatically. The default is FALSE.
       Note: When this TLV is used, the override
       parameters passed in other TLVs in this
       message are ignored by the device.
  */

  /* Optional */
  /*  Extended Technology Preference */
  uint8_t ext_technology_preference_valid;  /**< Must be set to true if ext_technology_preference is being passed */
  wds_ext_tech_pref_enum_v01 ext_technology_preference;
  /**<   The technology preference used while attempting a packet data
       connection. Values: \n
       - -32767 -- CDMA \n
       - -32764 -- UMTS \n
       - -30590 -- eMBMS \n
       - -30584 -- Modem Link Local \n
      Modem Link Local is an interface for transferring data
      between entities on the AP and modem.
  */

  /* Optional */
  /*  Call Type Identifier */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  wds_call_type_enum_v01 call_type;
  /**<   Type of call to be originated. Values: \n
       - 0 -- LAPTOP CALL \n
       - 1 -- EMBEDDED CALL \n
       If this TLV is not present, by default the call is considered
       to be a laptop call.
  */

  /* Optional */
  /*  Handoff Context */
  uint8_t handoff_context_valid;  /**< Must be set to true if handoff_context is being passed */
  wds_handoff_context_type_v01 handoff_context;
  /**<   \n
     Context information needed if the TE is handing off a call to the modem.
  */

  /* Optional */
  /*  IP Stream ID */
  uint8_t ips_id_valid;  /**< Must be set to true if ips_id is being passed */
  uint8_t ips_id;
  /**<   IP stream ID associated with the data call.
  */

  /* Optional */
  /*  APN type enum */
  uint8_t apn_type_valid;  /**< Must be set to true if apn_type is being passed */
  wds_apn_type_enum_v01 apn_type;
  /**<        - WDS_APN_TYPE_UNSPECIFIED (0) --  APN type unspecified 
      - WDS_APN_TYPE_INTERNET (1) --  APN type for Internet traffic 
      - WDS_APN_TYPE_IMS (2) --  APN type for IMS  
 */
}wds_start_network_interface_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_TYPE_UNSPECIFIED_V01 = 0x00, 
  WDS_VCER_TYPE_MOBILE_IP_V01 = 0x01, 
  WDS_VCER_TYPE_INTERNAL_V01 = 0x02, 
  WDS_VCER_TYPE_CALL_MANAGER_DEFINED_V01 = 0x03, 
  WDS_VCER_TYPE_3GPP_SPEC_DEFINED_V01 = 0x06, 
  WDS_VCER_TYPE_PPP_V01 = 0x07, 
  WDS_VCER_TYPE_EHRPD_V01 = 0x08, 
  WDS_VCER_TYPE_IPV6_V01 = 0x09, 
  WDS_VERBOSE_CALL_END_REASON_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_MIP_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_MIP_FA_REASON_UNSPECIFIED_V01 = 0x0040, 
  WDS_VCER_MIP_FA_ADMIN_PROHIBITED_V01 = 0x0041, 
  WDS_VCER_MIP_FA_INSUFFICIENT_RESOURCES_V01 = 0x0042, 
  WDS_VCER_MIP_FA_MOBILE_NODE_AUTH_FAILURE_V01 = 0x0043, 
  WDS_VCER_MIP_FA_HA_AUTH_FAILURE_V01 = 0x0044, 
  WDS_VCER_MIP_FA_REQ_LIFETIME_TOO_LONG_V01 = 0x0045, 
  WDS_VCER_MIP_FA_MALFORMED_REQUEST_V01 = 0x0046, 
  WDS_VCER_MIP_FA_MALFOMED_REPLY_V01 = 0x0047, 
  WDS_VCER_MIP_FA_ENCAPSULATION_UNAVAILABLE_V01 = 0x0048, 
  WDS_VCER_MIP_FA_VJHC_UNAVAILABLE_V01 = 0x0049, 
  WDS_VCER_MIP_FA_REV_TUNNEL_UNAVAILABLE_V01 = 0x004A, 
  WDS_VCER_MIP_FA_REV_TUNNEL_IS_MAND_AND_T_BIT_NOT_SET_V01 = 0x4B, 
  WDS_VCER_MIP_FA_DELIVERY_STYLE_NOT_SUPP_V01 = 0x4F, 
  WDS_VCER_MIP_FA_MISSING_NAI_V01 = 0x0061, 
  WDS_VCER_MIP_FA_MISSING_HA_V01 = 0x0062, 
  WDS_VCER_MIP_FA_MISSING_HOME_ADDR_V01 = 0x0063, 
  WDS_VCER_MIP_FA_UNKNOWN_CHALLENGE_V01 = 0x0068, 
  WDS_VCER_MIP_FA_MISSING_CHALLENGE_V01 = 0x0069, 
  WDS_VCER_MIP_FA_STALE_CHALLENGE_V01 = 0x006A, 
  WDS_VCER_MIP_HA_REASON_UNSPECIFIED_V01 = 0x0080, 
  WDS_VCER_MIP_HA_ADMIN_PROHIBITED_V01 = 0x0081, 
  WDS_VCER_MIP_HA_INSUFFICIENT_RESOURCES_V01 = 0x0082, 
  WDS_VCER_MIP_HA_MOBILE_NODE_AUTH_FAILURE_V01 = 0x0083, 
  WDS_VCER_MIP_HA_FA_AUTH_FAILURE_V01 = 0x0084, 
  WDS_VCER_MIP_HA_REGISTRATION_ID_MISMATCH_V01 = 0x0085, 
  WDS_VCER_MIP_HA_MALFORMED_REQUEST_V01 = 0x0086, 
  WDS_VCER_MIP_HA_UNKNOWN_HA_ADDR_V01 = 0x0088, 
  WDS_VCER_MIP_HA_REV_TUNNEL_UNAVAILABLE_V01 = 0x0089, 
  WDS_VCER_MIP_HA_REV_TUNNEL_IS_MAND_AND_T_BIT_NOT_SET_V01 = 0x008A, 
  WDS_VCER_MIP_HA_ENCAPSULATION_UNAVAILABLE_V01 = 0x008B, 
  WDS_VCER_MIP_HA_REASON_UNKNOWN_V01 = -1, 
  WDS_VERBOSE_CALL_END_REASON_MIP_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_mip_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_INTERNAL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_INTERNAL_INTERNAL_ERROR_V01 = 0xC9, 
  WDS_VCER_INTERNAL_CALL_ENDED_V01 = 0xCA, 
  WDS_VCER_INTERNAL_INTERNAL_UNKNOWN_CAUSE_CODE_V01 = 0xCB, 
  WDS_VCER_INTERNAL_UNKNOWN_CAUSE_CODE_V01 = 0xCC, 
  WDS_VCER_INTERNAL_CLOSE_IN_PROGRESS_V01 = 0xCD, 
  WDS_VCER_INTERNAL_NW_INITIATED_TERMINATION_V01 = 0xCE, 
  WDS_VCER_INTERNAL_APP_PREEMPTED_V01 = 0xCF, 
  WDS_VCER_INTERNAL_ERR_PDN_IPV4_CALL_DISALLOWED_V01 = 0xD0, 
  WDS_VCER_INTERNAL_ERR_PDN_IPV4_CALL_THROTTLED_V01 = 0xD1, 
  WDS_VCER_INTERNAL_ERR_PDN_IPV6_CALL_DISALLOWED_V01 = 0xD2, 
  WDS_VCER_INTERNAL_ERR_PDN_IPV6_CALL_THROTTLED_V01 = 0xD3, 
  WDS_VCER_INTERNAL_MODEM_RESTART_V01 = 0xD4, 
  WDS_VCER_INTERNAL_PDP_PPP_NOT_SUPPORTED_V01 = 0xD5, 
  WDS_VCER_INTERNAL_UNPREFERRED_RAT_V01 = 0xD6, 
  WDS_VCER_INTERNAL_PHYS_LINK_CLOSE_IN_PROGRESS_V01 = 0xD7, 
  WDS_VCER_INTERNAL_APN_PENDING_HANDOVER_V01 = 0xD8, 
  WDS_VCER_INTERNAL_PROFILE_BEARER_INCOMPATIBLE_V01 = 0xD9, 
  WDS_VCER_INTERNAL_MMGSDI_CARD_EVT_V01 = 0xDA, 
  WDS_VCER_INTERNAL_LPM_OR_PWR_DOWN_V01 = 0xDB, 
  WDS_VCER_INTERNAL_APN_DISABLED_V01 = 0xDC, 
  WDS_VCER_INTERNAL_MPIT_EXPIRED_V01 = 0xDD, 
  WDS_VCER_INTERNAL_IPV6_ADDR_TRANSFER_FAILED_V01 = 0xDE, 
  WDS_VCER_INTERNAL_TRAT_SWAP_FAILED_V01 = 0xDF, 
  WDS_VCER_INTERNAL_INTERNAL_EHRPD_TO_HRPD_FALLBACK_V01 = 0xE0, 
  WDS_VCER_INTERNAL_MANDATORY_APN_DISABLED_V01 = 0xE1, 
  WDS_VCER_INTERNAL_MIP_CONFIG_FAILURE_V01 = 0xE2, 
  WDS_VCER_INTERNAL_INTERNAL_PDN_INACTIVITY_TIMER_EXPIRED_V01 = 0xE3, 
  WDS_VCER_INTERNAL_MAX_V4_CONNECTIONS_V01 = 0xE4, 
  WDS_VCER_INTERNAL_MAX_V6_CONNECTIONS_V01 = 0xE5, 
  WDS_VCER_INTERNAL_APN_MISMATCH_V01 = 0xE6, 
  WDS_VCER_INTERNAL_IP_VERSION_MISMATCH_V01 = 0xE7, 
  WDS_VCER_INTERNAL_DUN_CALL_DISALLOWED_V01 = 0xE8, 
  WDS_VCER_INTERNAL_INVALID_PROFILE_V01 = 0xE9, 
  WDS_VCER_INTERNAL_INTERNAL_EPC_NONEPC_TRANSITION_V01 = 0xEA, 
  WDS_VCER_INTERNAL_INVALID_PROFILE_ID_V01 = 0xEB, 
  WDS_VCER_INTERNAL_INTERNAL_CALL_ALREADY_PRESENT_V01 = 0xEC, 
  WDS_VCER_INTERNAL_IFACE_IN_USE_V01 = 0xED, 
  WDS_VCER_INTERNAL_IP_PDP_MISMATCH_V01 = 0xEE, 
  WDS_VCER_INTERNAL_APN_DISALLOWED_ON_ROAMING_V01 = 0xEF, 
  WDS_VCER_INTERNAL_APN_PRM_CHG_V01 = 0xF0, 
  WDS_VCER_INTERNAL_IFACE_IN_USE_CFG_MATCH_V01 = 0xF1, 
  WDS_VCER_INTERNAL_NULL_APN_DISALLOWED_V01 = 0xF2, 
  WDS_VERBOSE_CALL_END_REASON_INTERNAL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_internal_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_CM_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_CM_CDMA_LOCK_V01 = 0x1F4, 
  WDS_VCER_CM_INTERCEPT_V01 = 0x1F5, 
  WDS_VCER_CM_REORDER_V01 = 0x1F6, 
  WDS_VCER_CM_REL_SO_REJ_V01 = 0x1F7, 
  WDS_VCER_CM_INCOM_CALL_V01 = 0x1F8, 
  WDS_VCER_CM_ALERT_STOP_V01 = 0x1F9, 
  WDS_VCER_CM_ACTIVATION_V01 = 0x1FA, 
  WDS_VCER_CM_MAX_ACCESS_PROBE_V01 = 0x1FB, 
  WDS_VCER_CM_CCS_NOT_SUPP_BY_BS_V01 = 0x1FC, 
  WDS_VCER_CM_NO_RESPONSE_FROM_BS_V01 = 0x1FD, 
  WDS_VCER_CM_REJECTED_BY_BS_V01 = 0x1FE, 
  WDS_VCER_CM_INCOMPATIBLE_V01 = 0x1FF, 
  WDS_VCER_CM_ALREADY_IN_TC_V01 = 0x200, 
  WDS_VCER_CM_USER_CALL_ORIG_DURING_GPS_V01 = 0x201, 
  WDS_VCER_CM_USER_CALL_ORIG_DURING_SMS_V01 = 0x202, 
  WDS_VCER_CM_NO_CDMA_SRV_V01 = 0x203, 
  WDS_VCER_CM_MC_ABORT_V01 = 0x204, 
  WDS_VCER_CM_PSIST_NG_V01 = 0x205, 
  WDS_VCER_CM_UIM_NOT_PRESENT_V01 = 0x206, 
  WDS_VCER_CM_RETRY_ORDER_V01 = 0x207, 
  WDS_VCER_CM_ACCESS_BLOCK_V01 = 0x208, 
  WDS_VCER_CM_ACCESS_BLOCK_ALL_V01 = 0x209, 
  WDS_VCER_CM_IS707B_MAX_ACC_V01 = 0x20A, 
  WDS_VCER_CM_THERMAL_EMERGENCY_V01 = 0x20B, 
  WDS_VCER_CM_CALL_ORIG_THROTTLED_V01 = 0x20C, 
  WDS_VCER_CM_USER_CALL_ORIG_DURING_VOICE_CALL_V01 = 0x20D, 
  WDS_VCER_CM_CONF_FAILED_V01 = 0x3E8, 
  WDS_VCER_CM_INCOM_REJ_V01 = 0x3E9, 
  WDS_VCER_CM_NEW_NO_GW_SERV_V01 = 0x3EA, 
  WDS_VCER_CM_NEW_NO_GPRS_CONTEXT_V01 = 0x3EB, 
  WDS_VCER_CM_NEW_ILLEGAL_MS_V01 = 0x3EC, 
  WDS_VCER_CM_NEW_ILLEGAL_ME_V01 = 0x3ED, 
  WDS_VCER_CM_NEW_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOWED_V01 = 0x3EE, 
  WDS_VCER_CM_NEW_GPRS_SERV_NOT_ALLOWED_V01 = 0x3EF, 
  WDS_VCER_CM_NO_GW_SERV_V01 = 0x3F0, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_NO_GPRS_CONTEXT_V01 = 0x3F1, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_ILLEGAL_MS_V01 = 0x3F2, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_ILLEGAL_ME_V01 = 0x3F3, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOWED_V01 = 0x3F4, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_GPRS_SERV_NOT_ALLOWED_V01 = 0x3F5, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK_V01 = 0x3F6, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_IMPLICITLY_DETACHED_V01 = 0x3F7, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_PLMN_NOT_ALLOWED_V01 = 0x3F8, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_LA_NOT_ALLOWED_V01 = 0x3F9, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_GPRS_SERV_NOT_ALLOWED_IN_THIS_PLMN_V01 = 0x3FA, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_PDP_DUPLICATE_V01 = 0x3FB, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_UE_RAT_CHANGE_V01 = 0x3FC, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_CONGESTION_V01 = 0x3FD, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_NO_PDP_CONTEXT_ACTIVATED_V01 = 0x3FE, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_ACCESS_CLASS_DSAC_REJECTION_V01 = 0x3FF, /**<  Deprecated rev 1.26  */
  WDS_VCER_CM_NEW_MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK_V01 = 0x3F0, 
  WDS_VCER_CM_NEW_IMPLICITLY_DETACHED_V01 = 0x3F1, 
  WDS_VCER_CM_NEW_PLMN_NOT_ALLOWED_V01 = 0x3F2, 
  WDS_VCER_CM_NEW_LA_NOT_ALLOWED_V01 = 0x3F3, 
  WDS_VCER_CM_NEW_GPRS_SERV_NOT_ALLOWED_IN_THIS_PLMN_V01 = 0x3F4, 
  WDS_VCER_CM_NEW_PDP_DUPLICATE_V01 = 0x3F5, 
  WDS_VCER_CM_NEW_UE_RAT_CHANGE_V01 = 0x3F6, 
  WDS_VCER_CM_NEW_CONGESTION_V01 = 0x3F7, 
  WDS_VCER_CM_NEW_NO_PDP_CONTEXT_ACTIVATED_V01 = 0x3F8, 
  WDS_VCER_CM_NEW_ACCESS_CLASS_DSAC_REJECTION_V01 = 0x3F9, 
  WDS_VCER_CM_PDP_ACTIVATE_MAX_RETRY_FAILED_V01 = 0x3FA, 
  WDS_VCER_CM_RAB_FAILURE_V01 = 0x3FB, 
  WDS_VCER_CM_EPS_SERVICES_NOT_ALLOWED_V01 = 0x3FC, 
  WDS_VCER_CM_TRACKING_AREA_NOT_ALLOWED_V01 = 0x3FD, 
  WDS_VCER_CM_ROAMING_NOT_ALLOWED_IN_THIS_TRACKING_AREA_V01 = 0x3FE, 
  WDS_VCER_CM_NO_SUITABLE_CELLS_IN_TRACKING_AREA_V01 = 0x3FF, 
  WDS_VCER_CM_NOT_AUTHORIZED_FOR_THIS_CSG_V01 = 0x400, 
  WDS_VCER_CM_ESM_UNKNOWN_EPS_BEARER_CONTEXT_V01 = 0x401, 
  WDS_VCER_CM_DRB_RELEASED_AT_RRC_V01 = 0x402, 
  WDS_VCER_CM_NAS_SIG_CONN_RELEASED_V01 = 0x403, 
  WDS_VCER_CM_EMM_DETACHED_V01 = 0x404, 
  WDS_VCER_CM_EMM_ATTACH_FAILED_V01 = 0x405, 
  WDS_VCER_CM_EMM_ATTACH_STARTED_V01 = 0x406, 
  WDS_VCER_CM_LTE_NAS_SERVICE_REQ_FAILED_V01 = 0x407, 
  WDS_VCER_CM_ESM_ACTIVE_DEDICATED_BEARER_REACTIVATED_BY_NW_V01 = 0x408, 
  WDS_VCER_CM_ESM_LOWER_LAYER_FAILURE_V01 = 0x409, 
  WDS_VCER_CM_ESM_SYNC_UP_WITH_NW_V01 = 0x40A, 
  WDS_VCER_CM_ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER_V01 = 0x40B, 
  WDS_VCER_CM_ESM_BAD_OTA_MESSAGE_V01 = 0x40C, 
  WDS_VCER_CM_ESM_DS_REJECTED_THE_CALL_V01 = 0x40D, 
  WDS_VCER_CM_ESM_CONTEXT_TRANSFERED_DUE_TO_IRAT_V01 = 0x40E, 
  WDS_VCER_CM_DS_EXPLICIT_DEACT_V01 = 0x40F, 
  WDS_VCER_CM_ESM_LOCAL_CAUSE_NONE_V01 = 0x410, 
  WDS_VCER_CM_LTE_NAS_SERVICE_REQ_FAILED_NO_THROTTLE_V01 = 0x411, 
  WDS_VCER_CM_ACL_FAILURE_V01 = 0x412, 
  WDS_VCER_CM_LTE_NAS_SERVICE_REQ_FAILED_DS_DISALLOW_V01 = 0x413, 
  WDS_VCER_CM_EMM_T3417_EXPIRED_V01 = 0x414, 
  WDS_VCER_CM_EMM_T3417_EXT_EXPIRED_V01 = 0x415, 
  WDS_VCER_CM_LRRC_UL_DATA_CNF_FAILURE_TXN_V01 = 0x416, 
  WDS_VCER_CM_LRRC_UL_DATA_CNF_FAILURE_HO_V01 = 0x417, 
  WDS_VCER_CM_LRRC_UL_DATA_CNF_FAILURE_CONN_REL_V01 = 0x418, 
  WDS_VCER_CM_LRRC_UL_DATA_CNF_FAILURE_RLF_V01 = 0x419, 
  WDS_VCER_CM_LRRC_UL_DATA_CNF_FAILURE_CTRL_NOT_CONN_V01 = 0x41A, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_V01 = 0x41B, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_ABORTED_V01 = 0x41C, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_ACCESS_BARRED_V01 = 0x41D, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_CELL_RESEL_V01 = 0x41E, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_CONFIG_FAILURE_V01 = 0x41F, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_TIMER_EXPIRED_V01 = 0x420, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_LINK_FAILURE_V01 = 0x421, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_NOT_CAMPED_V01 = 0x422, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_SI_FAILURE_V01 = 0x423, 
  WDS_VCER_CM_LRRC_CONN_EST_FAILURE_CONN_REJECT_V01 = 0x424, 
  WDS_VCER_CM_LRRC_CONN_REL_NORMAL_V01 = 0x425, 
  WDS_VCER_CM_LRRC_CONN_REL_RLF_V01 = 0x426, 
  WDS_VCER_CM_LRRC_CONN_REL_CRE_FAILURE_V01 = 0x427, 
  WDS_VCER_CM_LRRC_CONN_REL_OOS_DURING_CRE_V01 = 0x428, 
  WDS_VCER_CM_LRRC_CONN_REL_ABORTED_V01 = 0x429, 
  WDS_VCER_CM_LRRC_CONN_REL_SIB_READ_ERROR_V01 = 0x42A, 
  WDS_VCER_CM_DETACH_WITH_REATTACH_LTE_NW_DETACH_V01 = 0x42B, 
  WDS_VCER_CM_DETACH_WITH_OUT_REATTACH_LTE_NW_DETACH_V01 = 0x42C, 
  WDS_VCER_CM_ESM_PROC_TIME_OUT_V01 = 0x42D, 
  WDS_VCER_CM_INVALID_CONNECTION_ID_V01 = 0x42E, 
  WDS_VCER_CM_INVALID_NSAPI_V01 = 0x42F, 
  WDS_VCER_CM_INVALID_PRI_NSAPI_V01 = 0x430, 
  WDS_VCER_CM_INVALID_FIELD_V01 = 0x431, 
  WDS_VCER_CM_RAB_SETUP_FAILURE_V01 = 0x432, 
  WDS_VCER_CM_PDP_ESTABLISH_MAX_TIMEOUT_V01 = 0x433, 
  WDS_VCER_CM_PDP_MODIFY_MAX_TIMEOUT_V01 = 0x434, 
  WDS_VCER_CM_PDP_INACTIVE_MAX_TIMEOUT_V01 = 0x435, 
  WDS_VCER_CM_PDP_LOWERLAYER_ERROR_V01 = 0x436, 
  WDS_VCER_CM_PPD_UNKNOWN_REASON_V01 = 0x437, 
  WDS_VCER_CM_PDP_MODIFY_COLLISION_V01 = 0x438, 
  WDS_VCER_CM_PDP_MBMS_REQUEST_COLLISION_V01 = 0x439, 
  WDS_VCER_CM_MBMS_DUPLICATE_V01 = 0x43A, 
  WDS_VCER_CM_SM_PS_DETACHED_V01 = 0x43B, 
  WDS_VCER_CM_SM_NO_RADIO_AVAILABLE_V01 = 0x43C, 
  WDS_VCER_CM_SM_ABORT_SERVICE_NOT_AVAILABLE_V01 = 0x43D, 
  WDS_VCER_CM_MESSAGE_EXCEED_MAX_L2_LIMIT_V01 = 0x43E, 
  WDS_VCER_CM_SM_NAS_SRV_REQ_FAILURE_V01 = 0x43F, 
  WDS_VCER_CM_RRC_CONN_EST_FAILURE_REQ_ERROR_V01 = 0x440, 
  WDS_VCER_CM_RRC_CONN_EST_FAILURE_TAI_CHANGE_V01 = 0x441, 
  WDS_VCER_CM_RRC_CONN_EST_FAILURE_RF_UNAVAILABLE_V01 = 0x442, 
  WDS_VCER_CM_RRC_CONN_REL_ABORTED_IRAT_SUCCESS_V01 = 0x443, 
  WDS_VCER_CM_RRC_CONN_REL_RLF_SEC_NOT_ACTIVE_V01 = 0x444, 
  WDS_VCER_CM_RRC_CONN_REL_IRAT_TO_LTE_ABORTED_V01 = 0x445, 
  WDS_VCER_CM_RRC_CONN_REL_IRAT_FROM_LTE_TO_G_CCO_SUCCESS_V01 = 0x446, 
  WDS_VCER_CM_RRC_CONN_REL_IRAT_FROM_LTE_TO_G_CCO_ABORTED_V01 = 0x447, 
  WDS_VCER_CM_IMSI_UNKNOWN_IN_HSS_V01 = 0x448, 
  WDS_VCER_CM_IMEI_NOT_ACCEPTED_V01 = 0x449, 
  WDS_VCER_CM_EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED_V01 = 0x44A, 
  WDS_VCER_CM_EPS_SERVICES_NOT_ALLOWED_IN_PLMN_V01 = 0x44B, 
  WDS_VCER_CM_MSC_TEMPORARILY_NOT_REACHABLE_V01 = 0x44C, 
  WDS_VCER_CM_CS_DOMAIN_NOT_AVAILABLE_V01 = 0x44D, 
  WDS_VCER_CM_ESM_FAILURE_V01 = 0x44E, 
  WDS_VCER_CM_MAC_FAILURE_V01 = 0x44F, 
  WDS_VCER_CM_SYNCH_FAILURE_V01 = 0x450, 
  WDS_VCER_CM_UE_SECURITY_CAPABILITIES_MISMATCH_V01 = 0x451, 
  WDS_VCER_CM_SECURITY_MODE_REJ_UNSPECIFIED_V01 = 0x452, 
  WDS_VCER_CM_NON_EPS_AUTH_UNACCEPTABLE_V01 = 0x453, 
  WDS_VCER_CM_CS_FALLBACK_CALL_EST_NOT_ALLOWED_V01 = 0x454, 
  WDS_VCER_CM_NO_EPS_BEARER_CONTEXT_ACTIVATED_V01 = 0x455, 
  WDS_VCER_CM_EMM_INVALID_STATE_V01 = 0x456, 
  WDS_VCER_CM_NAS_LAYER_FAILURE_V01 = 0x457, 
  WDS_VCER_CM_CD_GEN_OR_BUSY_V01 = 0x5DC, 
  WDS_VCER_CM_CD_BILL_OR_AUTH_V01 = 0x5DD, 
  WDS_VCER_CM_CHG_HDR_V01 = 0x5DE, 
  WDS_VCER_CM_EXIT_HDR_V01 = 0x5DF, 
  WDS_VCER_CM_HDR_NO_SESSION_V01 = 0x5E0, 
  WDS_VCER_CM_HDR_ORIG_DURING_GPS_FIX_V01 = 0x5E1, 
  WDS_VCER_CM_HDR_CS_TIMEOUT_V01 = 0x5E2, 
  WDS_VCER_CM_HDR_RELEASED_BY_CM_V01 = 0x5E3, 
  WDS_VCER_CM_COLLOC_ACQ_FAIL_V01 = 0x5E4, 
  WDS_VCER_CM_OTASP_COMMIT_IN_PROG_V01 = 0x5E5, 
  WDS_VCER_CM_NO_HYBR_HDR_SRV_V01 = 0x5E6, 
  WDS_VCER_CM_HDR_NO_LOCK_GRANTED_V01 = 0x5E7, 
  WDS_VCER_CM_HOLD_OTHER_IN_PROG_V01 = 0x5E8, 
  WDS_VCER_CM_HDR_FADE_V01 = 0x5E9, 
  WDS_VCER_CM_HDR_ACC_FAIL_V01 = 0x5EA, 
  WDS_VCER_CM_CLIENT_END_V01 = 0x7D0, 
  WDS_VCER_CM_NO_SRV_V01 = 0x7D1, 
  WDS_VCER_CM_FADE_V01 = 0x7D2, 
  WDS_VCER_CM_REL_NORMAL_V01 = 0x7D3, 
  WDS_VCER_CM_ACC_IN_PROG_V01 = 0x7D4, 
  WDS_VCER_CM_ACC_FAIL_V01 = 0x7D5, 
  WDS_VCER_CM_REDIR_OR_HANDOFF_V01 = 0x7D6, 
  WDS_VCER_CM_OFFLINE_V01 = 0x9C4, 
  WDS_VCER_CM_EMERGENCY_MODE_V01 = 0x9C5, 
  WDS_VCER_CM_PHONE_IN_USE_V01 = 0x9C6, 
  WDS_VCER_CM_INVALID_MODE_V01 = 0x9C7, 
  WDS_VCER_CM_INVALID_SIM_STATE_V01 = 0x9C8, 
  WDS_VCER_CM_NO_COLLOC_HDR_V01 = 0x9C9, 
  WDS_VCER_CM_CALL_CONTROL_REJECTED_V01 = 0x9CA, 
  WDS_VERBOSE_CALL_END_REASON_CM_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_cm_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_3GPP_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_3GPP_OPERATOR_DETERMINED_BARRING_V01 = 0x08, 
  WDS_VCER_3GPP_NAS_SIGNALLING_ERROR_V01 = 0x0E, 
  WDS_VCER_3GPP_LLC_SNDCP_FAILURE_V01 = 0x19, 
  WDS_VCER_3GPP_INSUFFICIENT_RESOURCES_V01 = 0x1A, 
  WDS_VCER_3GPP_UNKNOWN_APN_V01 = 0x1B, 
  WDS_VCER_3GPP_UNKNOWN_PDP_V01 = 0x1C, 
  WDS_VCER_3GPP_AUTH_FAILED_V01 = 0x1D, 
  WDS_VCER_3GPP_GGSN_REJECT_V01 = 0x1E, 
  WDS_VCER_3GPP_ACTIVATION_REJECT_V01 = 0x1F, 
  WDS_VCER_3GPP_OPTION_NOT_SUPPORTED_V01 = 0x20, 
  WDS_VCER_3GPP_OPTION_UNSUBSCRIBED_V01 = 0x21, 
  WDS_VCER_3GPP_OPTION_TEMP_OOO_V01 = 0x22, 
  WDS_VCER_3GPP_NSAPI_ALREADY_USED_V01 = 0x23, 
  WDS_VCER_3GPP_REGULAR_DEACTIVATION_V01 = 0x24, 
  WDS_VCER_3GPP_QOS_NOT_ACCEPTED_V01 = 0x25, 
  WDS_VCER_3GPP_NETWORK_FAILURE_V01 = 0x26, 
  WDS_VCER_3GPP_UMTS_REACTIVATION_REQ_V01 = 0x27, 
  WDS_VCER_3GPP_FEATURE_NOT_SUPP_V01 = 0x28, 
  WDS_VCER_3GPP_TFT_SEMANTIC_ERROR_V01 = 0x29, 
  WDS_VCER_3GPP_TFT_SYTAX_ERROR_V01 = 0x2A, 
  WDS_VCER_3GPP_UNKNOWN_PDP_CONTEXT_V01 = 0x2B, 
  WDS_VCER_3GPP_FILTER_SEMANTIC_ERROR_V01 = 0x2C, 
  WDS_VCER_3GPP_FILTER_SYTAX_ERROR_V01 = 0x2D, 
  WDS_VCER_3GPP_PDP_WITHOUT_ACTIVE_TFT_V01 = 0x2E, 
  WDS_VCER_3GPP_IP_V4_ONLY_ALLOWED_V01 = 0x32, 
  WDS_VCER_3GPP_IP_V6_ONLY_ALLOWED_V01 = 0x33, 
  WDS_VCER_3GPP_SINGLE_ADDR_BEARER_ONLY_V01 = 0x34, 
  WDS_VCER_3GPP_ESM_INFO_NOT_RECEIVED_V01 = 0x35, 
  WDS_VCER_3GPP_PDN_CONN_DOES_NOT_EXIST_V01 = 0x36, 
  WDS_VCER_3GPP_MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED_V01 = 0x37, 
  WDS_VCER_3GPP_MAX_ACTIVE_PDP_CONTEXT_REACHED_V01 = 0x41, 
  WDS_VCER_3GPP_UNSUPPORTED_APN_IN_CURRENT_PLMN_V01 = 0x42, 
  WDS_VCER_3GPP_INVALID_TRANSACTION_ID_V01 = 0x51, 
  WDS_VCER_3GPP_MESSAGE_INCORRECT_SEMANTIC_V01 = 0x5F, 
  WDS_VCER_3GPP_INVALID_MANDATORY_INFO_V01 = 0x60, 
  WDS_VCER_3GPP_MESSAGE_TYPE_UNSUPPORTED_V01 = 0x61, 
  WDS_VCER_3GPP_MSG_TYPE_NONCOMPATIBLE_STATE_V01 = 0x62, 
  WDS_VCER_3GPP_UNKNOWN_INFO_ELEMENT_V01 = 0x63, 
  WDS_VCER_3GPP_CONDITIONAL_IE_ERROR_V01 = 0x64, 
  WDS_VCER_3GPP_MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE_V01 = 0x65, 
  WDS_VCER_3GPP_PROTOCOL_ERROR_V01 = 0x6F, 
  WDS_VCER_3GPP_APN_TYPE_CONFLICT_V01 = 0x70, 
  WDS_VCER_3GPP_INVALID_PCSCF_ADDR_V01 = 0x71, 
  WDS_VCER_3GPP_INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN_V01 = 0x72, 
  WDS_VCER_3GPP_EMM_ACCESS_BARRED_V01 = 0x73, 
  WDS_VCER_3GPP_EMERGENCY_IFACE_ONLY_V01 = 0x74, 
  WDS_VCER_3GPP_IFACE_MISMATCH_V01 = 0x75, 
  WDS_VCER_3GPP_COMPANION_IFACE_IN_USE_V01 = 0x76, 
  WDS_VCER_3GPP_IP_ADDRESS_MISMATCH_V01 = 0x77, 
  WDS_VCER_3GPP_IFACE_AND_POL_FAMILY_MISMATCH_V01 = 0x78, 
  WDS_VCER_3GPP_EMM_ACCESS_BARRED_INFINITE_RETRY_V01 = 0x79, 
  WDS_VCER_3GPP_AUTH_FAILURE_ON_EMERGENCY_CALL_V01 = 0x7A, 
  WDS_VERBOSE_CALL_END_REASON_3GPP_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_3gpp_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_PPP_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_PPP_TIMEOUT_V01 = 0x01, 
  WDS_VCER_PPP_AUTH_FAILURE_V01 = 0x02, 
  WDS_VCER_PPP_OPTION_MISMATCH_V01 = 0x03, 
  WDS_VCER_PPP_PAP_FAILURE_V01 = 0x1F, 
  WDS_VCER_PPP_CHAP_FAILURE_V01 = 0x20, 
  WDS_VCER_PPP_ERR_CLOSE_IN_PROGRESS_V01 = 0x21, 
  WDS_VCER_PPPNV_REFRESH_IN_PROGRESS_V01 = 0x22, 
  WDS_VCER_PPP_UNKNOWN_V01 = -1, 
  WDS_VERBOSE_CALL_END_REASON_PPP_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_ppp_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_EHRPD_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_EHRPD_SUBS_LIMITED_TO_V4_V01 = 0x01, 
  WDS_VCER_EHRPD_SUBS_LIMITED_TO_V6_V01 = 0x02, 
  WDS_VCER_EHRPD_VSNCP_TIMEOUT_V01 = 0x04, 
  WDS_VCER_EHRPD_VSNCP_FAILURE_V01 = 0x05, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_GEN_ERROR_V01 = 0x06, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_UNAUTH_APN_V01 = 0x07, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_PDN_LIMIT_EXCEED_V01 = 0x08, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_NO_PDN_GW_V01 = 0x09, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_PDN_GW_UNREACH_V01 = 0x0A, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_PDN_GW_REJ_V01 = 0x0B, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_INSUFF_PARAM_V01 = 0x0C, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_RESOURCE_UNAVAIL_V01 = 0x0D, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_ADMIN_PROHIBIT_V01 = 0x0E, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_PDN_ID_IN_USE_V01 = 0x0F, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_SUBSCR_LIMITATION_V01 = 0x10, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_PDN_EXISTS_FOR_THIS_APN_V01 = 0x11, 
  WDS_VCER_EHRPD_VSNCP_3GPP2I_RECONNECT_NOT_ALLOWED_V01 = 0x13, 
  WDS_VERBOSE_CALL_END_REASON_EHRPD_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_ehrpd_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_VERBOSE_CALL_END_REASON_IPV6_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_VCER_IPV6_PREFIX_UNAVAILABLE_V01 = 0x01, 
  WDS_VCER_IPV6_ERR_HRPD_IPV6_DISABLED_V01 = 0x02, 
  WDS_VCER_IPV6_DISABLED_V01 = 0x03, 
  WDS_VERBOSE_CALL_END_REASON_IPV6_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_verbose_call_end_reason_ipv6_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_CALL_END_REASON_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CER_UNSPECIFIED_V01 = 0x001, 
  WDS_CER_CLIENT_END_V01 = 0x002, 
  WDS_CER_NO_SRV_V01 = 0x003, 
  WDS_CER_FADE_V01 = 0x004, 
  WDS_CER_REL_NORMAL_V01 = 0x005, 
  WDS_CER_ACC_IN_PROG_V01 = 0x006, 
  WDS_CER_ACC_FAIL_V01 = 0x007, 
  WDS_CER_REDIR_OR_HANDOFF_V01 = 0x008, 
  WDS_CER_CLOSE_IN_PROGRESS_V01 = 0x009, 
  WDS_CER_AUTH_FAILED_V01 = 0x00A, 
  WDS_CER_INTERNAL_CALL_END_V01 = 0x00B, 
  WDS_CER_CDMA_LOCK_V01 = 0x1F4, 
  WDS_CER_INTERCEPT_V01 = 0x1F5, 
  WDS_CER_REORDER_V01 = 0x1F6, 
  WDS_CER_REL_SO_REJ_V01 = 0x1F7, 
  WDS_CER_INCOM_CALL_V01 = 0x1F8, 
  WDS_CER_ALERT_STOP_V01 = 0x1F9, 
  WDS_CER_ACTIVATION_V01 = 0x1FA, 
  WDS_CER_MAX_ACCESS_PROBE_V01 = 0x1FB, 
  WDS_CER_CCS_NOT_SUPP_BY_BS_V01 = 0x1FC, 
  WDS_CER_NO_RESPONSE_FROM_BS_V01 = 0x1FD, 
  WDS_CER_REJECTED_BY_BS_V01 = 0x1FE, 
  WDS_CER_INCOMPATIBLE_V01 = 0x1FF, 
  WDS_CER_ALREADY_IN_TC_V01 = 0x200, 
  WDS_CER_USER_CALL_ORIG_DURING_GPS_V01 = 0x201, 
  WDS_CER_USER_CALL_ORIG_DURING_SMS_V01 = 0x202, 
  WDS_CER_NO_CDMA_SRV_V01 = 0x203, 
  WDS_CER_CONF_FAILED_V01 = 0x3E8, 
  WDS_CER_INCOM_REJ_V01 = 0x3E9, 
  WDS_CER_NO_GW_SRV_V01 = 0x3EA, 
  WDS_CER_NETWORK_END_V01 = 0x3EB, 
  WDS_CER_LLC_SNDCP_FAILURE_V01 = 0x3EC, 
  WDS_CER_INSUFFICIENT_RESOURCES_V01 = 0x3ED, 
  WDS_CER_OPTION_TEMP_OOO_V01 = 0x3EE, 
  WDS_CER_NSAPI_ALREADY_USED_V01 = 0x3EF, 
  WDS_CER_REGULAR_DEACTIVATION_V01 = 0x3F0, 
  WDS_CER_NETWORK_FAILURE_V01 = 0x3F1, 
  WDS_CER_UMTS_REATTACH_REQ_V01 = 0x3F2, 
  WDS_CER_PROTOCOL_ERROR_V01 = 0x3F3, 
  WDS_CER_OPERATOR_DETERMINED_BARRING_V01 = 0x3F4, 
  WDS_CER_UNKNOWN_APN_V01 = 0x3F5, 
  WDS_CER_UNKNOWN_PDP_V01 = 0x3F6, 
  WDS_CER_GGSN_REJECT_V01 = 0x3F7, 
  WDS_CER_ACTIVATION_REJECT_V01 = 0x3F8, 
  WDS_CER_OPTION_NOT_SUPP_V01 = 0x3F9, 
  WDS_CER_OPTION_UNSUBSCRIBED_V01 = 0x3FA, 
  WDS_CER_QOS_NOT_ACCEPTED_V01 = 0x3FB, 
  WDS_CER_TFT_SEMANTIC_ERROR_V01 = 0x3FC, 
  WDS_CER_TFT_SYNTAX_ERROR_V01 = 0x3FD, 
  WDS_CER_UNKNOWN_PDP_CONTEXT_V01 = 0x3FE, 
  WDS_CER_FILTER_SEMANTIC_ERROR_V01 = 0x3FF, 
  WDS_CER_FILTER_SYNTAX_ERROR_V01 = 0x400, 
  WDS_CER_PDP_WITHOUT_ACTIVE_TFT_V01 = 0x401, 
  WDS_CER_INVALID_TRANSACTION_ID_V01 = 0x402, 
  WDS_CER_MESSAGE_INCORRECT_SEMANTIC_V01 = 0x403, 
  WDS_CER_INVALID_MANDATORY_INFO_V01 = 0x404, 
  WDS_CER_MESSAGE_TYPE_UNSUPPORTED_V01 = 0x405, 
  WDS_CER_MSG_TYPE_NONCOMPATIBLE_STATE_V01 = 0x406, 
  WDS_CER_UNKNOWN_INFO_ELEMENT_V01 = 0x407, 
  WDS_CER_CONDITIONAL_IE_ERROR_V01 = 0x408, 
  WDS_CER_MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE_V01 = 0x409, 
  WDS_CER_APN_TYPE_CONFLICT_V01 = 0x40A, 
  WDS_CER_NO_GPRS_CONTEXT_V01 = 0x40B, 
  WDS_CER_FEATURE_NOT_SUPPORTED_V01 = 0x40C, 
  WDS_CER_CD_GEN_OR_BUSY_V01 = 0x5DC, 
  WDS_CER_CD_BILL_OR_AUTH_V01 = 0x5DD, 
  WDS_CER_CHG_HDR_V01 = 0x5DE, 
  WDS_CER_EXIT_HDR_V01 = 0x5DF, 
  WDS_CER_HDR_NO_SESSION_V01 = 0x5E0, 
  WDS_CER_HDR_ORIG_DURING_GPS_FIX_V01 = 0x5E1, 
  WDS_CER_HDR_CS_TIMEOUT_V01 = 0x5E2, 
  WDS_CER_HDR_RELEASED_BY_CM_V01 = 0x5E3, 
  WDS_CALL_END_REASON_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_call_end_reason_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_verbose_call_end_reason_type_enum_v01 call_end_reason_type;
  /**<   Call end reason type. Values: \n
       - 0 -- Unspecified \n
       - 1 -- Mobile IP \n
       - 2 -- Internal \n
       - 3 -- Call Manager defined \n
       - 6 -- 3GPP Specification defined \n
       - 7 -- PPP \n
       - 8 -- EHRPD \n
       - 9 -- IPv6
  */

  uint16_t call_end_reason;
  /**<   Reason the call ended (verbose); see Appendix \ref{app:VerboseCallEndReasons} for the definition
       of these values.
  */
}wds_verbose_call_end_reason_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Activates a packet data session (if not already 
           started) on behalf of the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Packet Data Handle  */
  uint32_t pkt_data_handle;
  /**<   The handle identifying the call instance providing packet service.

       The packet data handle must be retained by the control point and
       specified in the STOP_NETWORK_INTERFACE message issued when the control
       point is finished with the packet data session.
  */

  /* Optional */
  /*  Call End Reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  wds_call_end_reason_enum_v01 call_end_reason;
  /**<   Reason the call ended; see Appendix \ref{app:callEndReasons} for the definition of these 
       values. */

  /* Optional */
  /*  Verbose Call End Reason */
  uint8_t verbose_call_end_reason_valid;  /**< Must be set to true if verbose_call_end_reason is being passed */
  wds_verbose_call_end_reason_type_v01 verbose_call_end_reason;

  /* Optional */
  /*  Peripheral End Point ID */
  uint8_t ep_id_valid;  /**< Must be set to true if ep_id is being passed */
  data_ep_id_type_v01 ep_id;
  /**<   \n The peripheral end point of the RmNet instance where a data call is already present.
  */

  /* Optional */
  /*  Mux ID */
  uint8_t mux_id_valid;  /**< Must be set to true if mux_id is being passed */
  uint8_t mux_id;
  /**<   Mux ID of the RmNet instance where a data call is already present.
  */
}wds_start_network_interface_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Deactivates a packet data session (unless in use by other control
    points) on behalf of the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Packet Data Handle */
  uint32_t pkt_data_handle;
  /**<   Handle identifying the call instance from which 
       to unbind the control point.
       The value must be the handle previously returned by
       QMI_WDS_START_NETWORK_ INTERFACE_REQ.
   */

  /* Optional */
  /*  Disable Autoconnect */
  uint8_t disable_autoconnect_valid;  /**< Must be set to true if disable_autoconnect is being passed */
  uint8_t disable_autoconnect;
  /**<   If set to 1 (TRUE), the device disables autoconnect,
       i.e., the calls need to be made manually until the
       setting is enabled again. The default is FALSE.
       Note: When this TLV is present, the client must
       use a global handle (0xFFFFFFFF) in the
       Packet Data Handle TLV above.
  */
}wds_stop_network_interface_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Deactivates a packet data session (unless in use by other control
    points) on behalf of the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_stop_network_interface_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current packet data connection status. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_pkt_srvc_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_CONNECTION_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CONNECTION_STATUS_DISCONNECTED_V01 = 0x01, 
  WDS_CONNECTION_STATUS_CONNECTED_V01 = 0x02, 
  WDS_CONNECTION_STATUS_SUSPENDED_V01 = 0x03, 
  WDS_CONNECTION_STATUS_AUTHENTICATING_V01 = 0x04, 
  WDS_CONNECTION_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_connection_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current packet data connection status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Connection status. */
  wds_connection_status_enum_v01 connection_status;
  /**<   Current link status. Values: \n
        - 1 -- DISCONNECTED \n
        - 2 -- CONNECTED \n
        - 3 -- SUSPENDED \n
        - 4 -- AUTHENTICATING
   */
}wds_get_pkt_srvc_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  /*  Connection status */
  wds_connection_status_enum_v01 connection_status;
  /**<   Current link status. Values: \n
        - 1 -- DISCONNECTED \n
        - 2 -- CONNECTED \n
        - 3 -- SUSPENDED \n
        - 4 -- AUTHENTICATING
   */

  /*  Reconfiguration required */
  uint8_t reconfiguration_required;
  /**<   Indicates whether the network interface on the host needs to be
       reconfigured. Values: \n
       - 0 -- No need to reconfigure \n
       - 1 -- Reconfiguration required
   */
}wds_packet_service_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IP_FAMILY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IP_FAMILY_IPV4_V01 = 0x04, /**<  IPv4 \n  */
  WDS_IP_FAMILY_IPV6_V01 = 0x06, /**<  IPv6   */
  WDS_IP_FAMILY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ip_family_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_TECHNOLOGY_NAME_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TECHNOLOGY_NAME_CDMA_V01 = -32767, 
  WDS_TECHNOLOGY_NAME_UMTS_V01 = -32764, 
  WDS_TECHNOLOGY_NAME_WLAN_LOCAL_BRKOUT_V01 = -32736, 
  WDS_TECHNOLOGY_NAME_IWLAN_S2B_V01 = -32735, 
  WDS_TECHNOLOGY_NAME_EPC_V01 = -30592, 
  WDS_TECHNOLOGY_NAME_EMBMS_V01 = -30590, 
  WDS_TECHNOLOGY_NAME_MODEM_LINK_LOCAL_V01 = -30584, 
  WDS_TECHNOLOGY_NAME_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_technology_name_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current packet data 
           connection status. */
typedef struct {

  /* Mandatory */
  /*  Packet Service Status */
  wds_packet_service_status_type_v01 status;

  /* Optional */
  /*  Call End Reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  wds_call_end_reason_enum_v01 call_end_reason;
  /**<   See Appendix \ref{app:callEndReasons}
       for the definition of these values.
  */

  /* Optional */
  /*  Verbose Call End Reason */
  uint8_t verbose_call_end_reason_valid;  /**< Must be set to true if verbose_call_end_reason is being passed */
  wds_verbose_call_end_reason_type_v01 verbose_call_end_reason;

  /* Optional */
  /*  IP Family */
  uint8_t ip_family_valid;  /**< Must be set to true if ip_family is being passed */
  wds_ip_family_enum_v01 ip_family;
  /**<   IP family of the packet data connection. Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */

  /* Optional */
  /*  Technology Name */
  uint8_t tech_name_valid;  /**< Must be set to true if tech_name is being passed */
  wds_technology_name_enum_v01 tech_name;
  /**<   Technology name of the packet data connection. Values: \n
       - -32767 -- CDMA \n
       - -32764 -- UMTS \n
       - -30592 -- EPC \n
       - -30590 -- EMBMS \n
       - -30584 -- Modem Link Local \n
      EPC is a logical interface to support LTE/eHRPD handoff. 
      It is returned if the device supports IP session continuity.

      Modem Link Local is an interface for transferring data
      between entities on the AP and modem.
  */

  /* Optional */
  /*  Bearer ID */
  uint8_t bearer_id_valid;  /**< Must be set to true if bearer_id is being passed */
  uint8_t bearer_id;
  /**<   Bearer ID (3GPP) or RLP ID (3GPP2) of the packet data connection.
  */

  /* Optional */
  /*  XLAT capability */
  uint8_t xlat_capable_valid;  /**< Must be set to true if xlat_capable is being passed */
  uint8_t xlat_capable;
  /**<   Indicates xlat capability of the data session.Values: \n
       - 0 -- XLAT not capable \n
       - 1 -- XLAT capable
  */
}wds_pkt_srvc_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current bit rate of the packet data connection. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_current_channel_rate_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t current_channel_tx_rate;
  /**<   Instantaneous channel Tx rate in bits per second.*/

  uint32_t current_channel_rx_rate;
  /**<   Instantaneous channel Rx rate in bits per second.*/

  uint32_t max_channel_tx_rate;
  /**<   Maximum Tx rate that can be assigned to the device by the serving
       system in bits per second.
  */

  uint32_t max_channel_rx_rate;
  /**<   Maximum Rx rate that can be assigned to the device by the serving
     system in bits per second.
  */
}wds_current_channel_rate_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current bit rate of the packet data connection. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Channel Rate */
  wds_current_channel_rate_type_v01 rates;
}wds_get_current_channel_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the packet data transfer statistics from the start of the 
    current packet data session. */
typedef struct {

  /* Mandatory */
  /*  Packet Statistics Mask */
  wds_stats_mask_v01 stats_mask;
  /**<   Values: \n
       - 0x00000001 -- Tx packets OK \n
       - 0x00000002 -- Rx packets OK \n
       - 0x00000004 -- Tx packet errors \n
       - 0x00000008 -- Rx packet errors \n
       - 0x00000010 -- Tx overflows \n
       - 0x00000020 -- Rx overflows \n
       - 0x00000040 -- Tx bytes OK \n
       - 0x00000080 -- Rx bytes OK \n
       - 0x00000100 -- Tx packets dropped \n
       - 0x00000200 -- Rx packets dropped

       All unlisted bits are reserved for future use and must be set
       to zero unless recognized by issuer.
  */
}wds_get_pkt_statistics_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the packet data transfer statistics from the start of the 
    current packet data session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Tx Packets OK */
  uint8_t tx_ok_count_valid;  /**< Must be set to true if tx_ok_count is being passed */
  uint32_t tx_ok_count;
  /**<   Number of packets transmitted without error.*/

  /* Optional */
  /*  Rx Packets OK */
  uint8_t rx_ok_count_valid;  /**< Must be set to true if rx_ok_count is being passed */
  uint32_t rx_ok_count;
  /**<   Number of packets received without error.*/

  /* Optional */
  /*  Tx Packet Errors */
  uint8_t tx_err_count_valid;  /**< Must be set to true if tx_err_count is being passed */
  uint32_t tx_err_count;
  /**<   Number of outgoing packets with framing errors.*/

  /* Optional */
  /*  Rx Packet Errors */
  uint8_t rx_err_count_valid;  /**< Must be set to true if rx_err_count is being passed */
  uint32_t rx_err_count;
  /**<   Number of incoming packets with framing errors.*/

  /* Optional */
  /*  Tx Overflows */
  uint8_t tx_ofl_count_valid;  /**< Must be set to true if tx_ofl_count is being passed */
  uint32_t tx_ofl_count;
  /**<   Number of packets dropped because Tx buffer overflowed (out of memory).*/

  /* Optional */
  /*  Rx Overflows */
  uint8_t rx_ofl_count_valid;  /**< Must be set to true if rx_ofl_count is being passed */
  uint32_t rx_ofl_count;
  /**<   Number of packets dropped because Rx buffer overflowed (out of memory).*/

  /* Optional */
  /*  Tx Bytes OK */
  uint8_t tx_ok_bytes_count_valid;  /**< Must be set to true if tx_ok_bytes_count is being passed */
  uint64_t tx_ok_bytes_count;
  /**<   Number of bytes transmitted without error.*/

  /* Optional */
  /*  Rx Bytes OK */
  uint8_t rx_ok_bytes_count_valid;  /**< Must be set to true if rx_ok_bytes_count is being passed */
  uint64_t rx_ok_bytes_count;
  /**<   Number of bytes received without error.*/

  /* Optional */
  /*  Last Call Tx Bytes OK */
  uint8_t last_call_tx_ok_bytes_count_valid;  /**< Must be set to true if last_call_tx_ok_bytes_count is being passed */
  uint64_t last_call_tx_ok_bytes_count;
  /**<   Number of bytes transmitted without error during
       the last data call (0 if no call was made earlier).
       Returned only if not in a call, and when the
       previous call was made using RmNet (for any
       devices that support
       QMI_WDS_GET_DUN_CALL_INFO).
  */

  /* Optional */
  /*  Last Call Rx Bytes OK */
  uint8_t last_call_rx_ok_bytes_count_valid;  /**< Must be set to true if last_call_rx_ok_bytes_count is being passed */
  uint64_t last_call_rx_ok_bytes_count;
  /**<   Number of bytes received without error during the
       last data call (0 if no call was made earlier).
       Returned only if not in a call, and when the
       previous call was made using RmNet (for any
       devices that support
       QMI_WDS_GET_DUN_CALL_INFO).
  */

  /* Optional */
  /*  Tx Packets Dropped */
  uint8_t tx_dropped_count_valid;  /**< Must be set to true if tx_dropped_count is being passed */
  uint32_t tx_dropped_count;
  /**<   Number of outgoing packets dropped.*/

  /* Optional */
  /*  Rx Packets Dropped */
  uint8_t rx_dropped_count_valid;  /**< Must be set to true if rx_dropped_count is being passed */
  uint32_t rx_dropped_count;
  /**<   Number of incoming packets dropped.*/
}wds_get_pkt_statistics_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Forces the device to immediately drop the traffic channel on the 
    serving radio interface. */
typedef struct {

  /* Optional */
  /*  Delay Time in Milliseconds */
  uint8_t delay_timer_valid;  /**< Must be set to true if delay_timer is being passed */
  uint32_t delay_timer;
  /**<   Delay time, in milliseconds. */
}wds_go_dormant_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Forces the device to immediately drop the traffic channel on the 
    serving radio interface. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_go_dormant_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Forces the device to immediately reestablish the 
           traffic channel on the serving radio interface. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_go_active_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Forces the device to immediately reestablish the 
           traffic channel on the serving radio interface. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_go_active_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_TRAFFIC_CLASS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TRAFFIC_CLASS_SUBSCRIBED_V01 = 0x00, 
  WDS_TRAFFIC_CLASS_CONVERSATIONAL_V01 = 0x01, 
  WDS_TRAFFIC_CLASS_STREAMING_V01 = 0x02, 
  WDS_TRAFFIC_CLASS_INTERACTIVE_V01 = 0x03, 
  WDS_TRAFFIC_CLASS_BACKGROUND_V01 = 0x04, 
  WDS_TRAFFIC_CLASS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_traffic_class_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_QOS_DELIVERY_ORDER_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_QOS_DELIVERY_ORDER_SUBSCRIBE_V01 = 0x00, 
  WDS_QOS_DELIVERY_ORDER_ON_V01 = 0x01, 
  WDS_QOS_DELIVERY_ORDER_OFF_V01 = 0x02, 
  WDS_QOS_DELIVERY_ORDER_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_qos_delivery_order_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DELIVERY_ERRONEOUS_SDUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DELIVERY_ERRONEOUS_SDUS_SUBSCRIBE_V01 = 0x00, 
  WDS_DELIVERY_ERRONEOUS_SDUS_NO_DETECTION_V01 = 0x01, 
  WDS_DELIVERY_ERRONEOUS_SDUS_YES_V01 = 0x02, 
  WDS_DELIVERY_ERRONEOUS_SDUS_NO_V01 = 0x03, 
  WDS_DELIVERY_ERRONEOUS_SDUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_delivery_erroneous_SDUs_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_SDU_ERROR_RATIO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_SDU_ERROR_RATIO_SUBSCRIBE_V01 = 0x00, 
  WDS_SDU_ERROR_RATIO_1X10_2_V01 = 0x01, 
  WDS_SDU_ERROR_RATIO_7X10_3_V01 = 0x02, 
  WDS_SDU_ERROR_RATIO_1X10_3_V01 = 0x03, 
  WDS_SDU_ERROR_RATIO_1X10_4_V01 = 0x04, 
  WDS_SDU_ERROR_RATIO_1X10_5_V01 = 0x05, 
  WDS_SDU_ERROR_RATIO_1X10_6_V01 = 0x06, 
  WDS_SDU_ERROR_RATIO_1X10_1_V01 = 0x07, 
  WDS_SDU_ERROR_RATIO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_sdu_error_ratio_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_RESIDUAL_BIT_ERROR_RATIO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_RESIDUAL_BIT_ERROR_RATIO_SUBSCRIBE_V01 = 0x00, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_5X10_2_V01 = 0x01, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_1X10_2_V01 = 0x02, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_5X10_3_V01 = 0x03, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_4X10_3_V01 = 0x04, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_1X10_3_V01 = 0x05, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_1X10_4_V01 = 0x06, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_1X10_5_V01 = 0x07, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_1X10_6_V01 = 0x08, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_6X10_8_V01 = 0x09, 
  WDS_RESIDUAL_BIT_ERROR_RATIO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_residual_bit_error_ratio_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_traffic_class_enum_v01 traffic_class;
  /**<   Traffic class. Values: \n
       - 0 -- Subscribed \n
       - 1 -- Conversational \n
       - 2 -- Streaming \n
       - 3 -- Interactive \n
       - 4 -- Background
   */

  uint32_t max_uplink_bitrate;
  /**<   Maximum uplink bit rate in bits per second.*/

  uint32_t max_downlink_bitrate;
  /**<   Maximum downlink bit rate in bits per second.*/

  uint32_t guaranteed_uplink_bitrate;
  /**<   Guaranteed uplink bit rate in bits per second.*/

  uint32_t guaranteed_downlink_bitrate;
  /**<   Guaranteed downlink bit rate in bits per second.*/

  wds_qos_delivery_order_enum_v01 qos_delivery_order;
  /**<   Values: \n
       - 0 -- Subscribe \n
       - 1 -- Delivery order on \n
       - 2 -- Delivery order off
   */

  uint32_t max_sdu_size;
  /**<   Maximum SDU size.*/

  wds_sdu_error_ratio_enum_v01 sdu_error_ratio;
  /**<   Target value for the fraction of SDUs lost or detected as erroneous. Values: \n
       - 0 -- Subscribe \n
       - 1 -- 1x@latexonly$10^2$@endlatexonly \n
       - 2 -- 7x10^3 \n
       - 3 -- 1x10^3 \n
       - 4 -- 1x10^4 \n
       - 5 -- 1x10^5 \n
       - 6 -- 1x10^6 \n
       - 7 -- 1x10^1
   */

  wds_residual_bit_error_ratio_enum_v01 residual_bit_error_ratio;
  /**<   Target value for the undetected bit error ratio in the delivered 
       SDUs. Values: \n
        - 0 -- Subscribe \n
        - 1 -- 5x@latexonly$10^2$@endlatexonly \n
        - 2 -- 1x10^2 \n
        - 3 -- 5x10^3 \n
        - 4 -- 4x10^3 \n
        - 5 -- 1x10^3 \n
        - 6 -- 1x10^4 \n
        - 7 -- 1x10^5 \n
        - 8 -- 1x10^6 \n
        - 9 -- 6x10^8
   */

  wds_delivery_erroneous_SDUs_enum_v01 delivery_erroneous_SDUs;
  /**<   Delivery of erroneous SDUs.
       Indicates whether SDUs detected as erroneous are delivered or not. Values: \n
       - 0 -- Subscribe \n
       - 1 -- No detection \n
       - 2 -- Erroneous SDU is delivered \n
       - 3 -- Erroneous SDU is not delivered
   */

  uint32_t transfer_delay;
  /**<   Transfer delay (ms).
       Indicates the targeted time between a request to transfer an SDU
       at one SAP to its delivery at the other SAP, in
       milliseconds; if the parameter is set to 0, the subscribed
       value is requested.
   */

  uint32_t traffic_handling_priority;
  /**<   Traffic handling priority.
       Specifies the relative importance for handling of SDUs that
       belong to the UMTS bearer, compared to the SDUs of other
       bearers. If the parameter is set to 0, the subscribed value
       is requested.
   */
}wds_umts_qos_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_umts_qos_type_v01 umts_qos;

  uint8_t sig_ind;
  /**<   Signaling indication flag. Values: \n
         - 0 -- Signaling indication off \n
         - 1 -- Signaling indication on
  */
}wds_umts_qos_with_sig_ind_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t precedence_class;
  /**<   Precedence class \hyperref[Q3]{[Q3]}*/

  uint32_t delay_class;
  /**<   Delay class \hyperref[Q3]{[Q3]}*/

  uint32_t reliability_class;
  /**<   Reliability class \hyperref[Q3]{[Q3]}*/

  uint32_t peak_throughput_class;
  /**<   Peak throughput class \hyperref[Q3]{[Q3]}*/

  uint32_t mean_throughput_class;
  /**<   Mean throughput class \hyperref[Q3]{[Q3]}*/
}wds_gprs_qos_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t filter_id;
  /**<   Filter identifier.*/

  uint8_t eval_id;
  /**<   Evaluation precedence index.*/

  wds_ip_version_enum_v01 ip_version;
  /**<   IP version number. Values: \n
        - 4 -- IPv4 \n
        - 6 -- IPv6
  */

  uint8_t source_ip[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Values: \n
       - IPv4 -- Fill the first 4 bytes \n
       - IPv6 -- Fill all the 16 bytes
  */

  uint8_t source_ip_mask;
  /**<   Mask value for the source address.*/

  uint8_t next_header;
  /**<   Next header/protocol value.*/

  uint16_t dest_port_range_start;
  /**<   Start value for the destination port range.*/

  uint16_t dest_port_range_end;
  /**<   End value for the destination port range.*/

  uint16_t src_port_range_start;
  /**<   Start value for the source port range.*/

  uint16_t src_port_range_end;
  /**<   End value for the source port range.*/

  uint32_t ipsec_spi;
  /**<   IPSec security parameter index.*/

  uint16_t tos_mask;
  /**<   TOS mask (traffic class for IPv6).*/

  uint32_t flow_label;
  /**<   Flow label.*/
}wds_tft_id_param_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t qci;
  /**<   For LTE, the requested QOS must be specified using the QOS Class
       Identifier (QOS). Values: \n
         - QCI value 0 -- Requests the network to assign the appropriate QCI value \n
         - QCI values 1-4 -- Associated with guaranteed bit rates \n
         - QCI values 5-9 -- Associated with nonguaranteed bit rates, the values \n
           specified as guaranteed and maximum bit rates are ignored.
  */

  uint32_t g_dl_bit_rate;
  /**<   Guaranteed DL bit rate.*/

  uint32_t max_dl_bit_rate;
  /**<   Maximum DL bit rate.*/

  uint32_t g_ul_bit_rate;
  /**<   Guaranteed UL bit rate.*/

  uint32_t max_ul_bit_rate;
  /**<   Maximum UL bit rate.*/
}wds_3gpp_lte_qos_params_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_TYPE_3GPP_V01 = 0x00, /**<  3GPP \n  */
  WDS_PROFILE_TYPE_3GPP2_V01 = 0x01, /**<  3GPP2 \n  */
  WDS_PROFILE_TYPE_EPC_V01 = 0x02, /**<  EPC  */
  WDS_PROFILE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_CHANGED_EVENT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_CREATE_PROFILE_EVENT_V01 = 0x01, /**<  Create profile event \n  */
  WDS_PROFILE_DELETE_PROFILE_EVENT_V01 = 0x02, /**<  Delete profile event \n  */
  WDS_PROFILE_MODIFY_PROFILE_EVENT_V01 = 0x03, /**<  Modify profile event \n  */
  WDS_PROFILE_SUBSCRIPTION_CHANGE_EVENT_V01 = 0x04, /**<  Subscription changed event  */
  WDS_PROFILE_CHANGED_EVENT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_changed_event_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PDP_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PDP_TYPE_PDP_IPV4_V01 = 0x00, 
  WDS_PDP_TYPE_PDP_PPP_V01 = 0x01, 
  WDS_PDP_TYPE_PDP_IPV6_V01 = 0x02, 
  WDS_PDP_TYPE_PDP_IPV4V6_V01 = 0x03, 
  WDS_PDP_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_pdp_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PDP_HDR_COMPR_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PDP_HDR_COMPR_TYPE_OFF_V01 = 0x00, 
  WDS_PDP_HDR_COMPR_TYPE_MANUFACTURER_PREF_V01 = 0x01, 
  WDS_PDP_HDR_COMPR_TYPE_RFC_1144_V01 = 0x02, 
  WDS_PDP_HDR_COMPR_TYPE_RFC_2507_V01 = 0x03, 
  WDS_PDP_HDR_COMPR_TYPE_RFC_3095_V01 = 0x04, 
  WDS_PDP_HDR_COMPR_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_pdp_hdr_compr_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PDP_DATA_COMPR_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PDP_DATA_COMPR_TYPE_OFF_V01 = 0x00, 
  WDS_PDP_DATA_COMPR_TYPE_MANUFACTURER_PREF_V01 = 0x01, 
  WDS_PDP_DATA_COMPR_TYPE_V42_V01 = 0x02, 
  WDS_PDP_DATA_COMPR_TYPE_V44_V01 = 0x03, 
  WDS_PDP_DATA_COMPR_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_pdp_data_compr_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PDP_ACCESS_CONTROL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PDP_ACCESS_CONTROL_NONE_V01 = 0x00, 
  WDS_PDP_ACCESS_CONTROL_REJECT_V01 = 0x01, 
  WDS_PDP_ACCESS_CONTROL_PERMISSION_V01 = 0x02, 
  WDS_PDP_ACCESS_CONTROL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_pdp_access_control_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_ADDR_ALLOCATION_PREFERENCE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_ADDR_ALLOC_PREF_NAS_V01 = 0x00, 
  WDS_ADDR_ALLOC_PREF_DHCP_V01 = 0x01, 
  WDS_ADDR_ALLOCATION_PREFERENCE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_addr_allocation_preference_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_AUTH_PROTOCOL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_AUTH_PROTOCOL_NONE_V01 = 0, 
  WDS_PROFILE_AUTH_PROTOCOL_PAP_V01 = 1, 
  WDS_PROFILE_AUTH_PROTOCOL_CHAP_V01 = 2, 
  WDS_PROFILE_AUTH_PROTOCOL_PAP_CHAP_V01 = 3, 
  WDS_PROFILE_AUTH_PROTOCOL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_auth_protocol_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_PDN_LVL_AUTH_PROTO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_NONE_V01 = 0, 
  WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_V01 = 1, 
  WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP_V01 = 2, 
  WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_CHAP_V01 = 3, 
  WDS_PROFILE_PDN_LVL_AUTH_PROTO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_pdn_lvl_auth_proto_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_DATA_RATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_DATA_RATE_LOW_V01 = 0, 
  WDS_PROFILE_DATA_RATE_MEDIUM_V01 = 1, 
  WDS_PROFILE_DATA_RATE_HIGH_V01 = 2, 
  WDS_PROFILE_DATA_RATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_data_rate_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_APP_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_APP_TYPE_DEFAULT_V01 = 0x00000001, 
  WDS_PROFILE_APP_TYPE_LBS_V01 = 0x00000020, 
  WDS_PROFILE_APP_TYPE_TETHERED_V01 = 0x00000040, 
  WDS_PROFILE_APP_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_app_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_DATA_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_DATA_MODE_CDMA_HDR_V01 = 0, 
  WDS_PROFILE_DATA_MODE_CDMA_V01 = 1, 
  WDS_PROFILE_DATA_MODE_HDR_V01 = 2, 
  WDS_PROFILE_DATA_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_data_mode_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_PDN_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_PDN_TYPE_IPV4_V01 = 0, 
  WDS_PROFILE_PDN_TYPE_IPV6_V01 = 1, 
  WDS_PROFILE_PDN_TYPE_IPV4_IPV6_V01 = 2, 
  WDS_PROFILE_PDN_TYPE_UNSPECIFIED_V01 = 3, 
  WDS_PROFILE_PDN_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_pdn_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_RAT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_RAT_TYPE_HRPD_V01 = 1, 
  WDS_RAT_TYPE_EHRPD_V01 = 2, 
  WDS_RAT_TYPE_HRPD_EHRPD_V01 = 3, 
  WDS_RAT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_rat_type_enum_v01;
/**
    @}
  */

typedef uint64_t wds_apn_bearer_mask_v01;
#define WDS_MASK_APN_BEARER_G_V01 ((wds_apn_bearer_mask_v01)0x0000000000000001ull) 
#define WDS_MASK_APN_BEARER_W_V01 ((wds_apn_bearer_mask_v01)0x0000000000000002ull) 
#define WDS_MASK_APN_BEARER_L_V01 ((wds_apn_bearer_mask_v01)0x0000000000000004ull) 
#define WDS_MASK_APN_BEARER_ANY_V01 ((wds_apn_bearer_mask_v01)0x8000000000000000ull) 
/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. 
       Range: 0 to 999.
  */

  /*  MNC PCS digit include status */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding
       MNC reported in the TLVs. Values: \n
       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */
}wds_mnc_pcs_digit_include_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. 
       Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. 
       Range: 0 to 999.
  */

  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding 
       MNC reported in the TLV. Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */

  uint32_t app_specific_info_len;  /**< Must be set to # of elements in app_specific_info */
  uint8_t app_specific_info[QMI_WDS_APP_SPECIFIC_INFO_V01];
  /**<   Points to the application-specific information
       from the network. The format for this field complies with 
       3GPP TS 24.008 \hyperref[S5]{[S5]}. The field is populated in this 
       format for both 3GPP and 3GPP2. 
  */

  uint16_t container_id;
  /**<   Container ID of this PCO.
  */
}wds_op_reserved_pco_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t common_allow_linger;
  /**<   Values: \n
       - 1 -- TRUE -- Allow lingering \n
       - 0 -- FALSE -- Do not allow lingering 
  */

  uint16_t common_linger_timeout;
  /**<   Value of linger timeout in milliseconds.
  */
}wds_common_linger_param_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_COMMON_PDP_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_COMMON_PDP_TYPE_PDP_IPV4_V01 = 0x00, 
  WDS_COMMON_PDP_TYPE_PDP_IPV6_V01 = 0x01, 
  WDS_COMMON_PDP_TYPE_PDP_IPV4V6_V01 = 0x02, 
  WDS_COMMON_PDP_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_common_pdp_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Creates a configured profile with specified settings. */
typedef struct {

  /* Mandatory */
  /*  Profile Type */
  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
      - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n 
      - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_PROFILE_TYPE_EPC (0x02) --  EPC 
 */

  /* Optional */
  /*  Profile Name ** */
  uint8_t profile_name_valid;  /**< Must be set to true if profile_name is being passed */
  char profile_name[QMI_WDS_PROFILE_NAME_MAX_V01 + 1];
  /**<   One or more bytes describing the profile. The description can
       be a user-defined name for the profile.
       QMI_ERR_ARG_TOO_LONG is returned if the profile_name is too long.
   */

  /* Optional */
  /*  PDP Type ** */
  uint8_t pdp_type_valid;  /**< Must be set to true if pdp_type is being passed */
  wds_pdp_type_enum_v01 pdp_type;
  /**<   Packet Data Protocol (PDP) type specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-PPP \n
       - 2 -- PDP-IPv6 \n
       - 3 -- PDP-IPv4v6
   */

  /* Optional */
  /*  PDP Header Compression Type ** */
  uint8_t pdp_hdr_compression_type_valid;  /**< Must be set to true if pdp_hdr_compression_type is being passed */
  wds_pdp_hdr_compr_type_enum_v01 pdp_hdr_compression_type;
  /**<   Values: \n
       - 0 -- PDP header compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- PDP header compression based on RFC 1144 \n
       - 3 -- PDP header compression based on RFC 2507 \n
       - 4 -- PDP header compression based on RFC 3095 
  */

  /* Optional */
  /*  PDP Data Compression Type To Use ** */
  uint8_t pdp_data_compression_type_valid;  /**< Must be set to true if pdp_data_compression_type is being passed */
  wds_pdp_data_compr_type_enum_v01 pdp_data_compression_type;
  /**<   Values: \n
       - 0 -- PDP data compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- V.42BIS data compression \n
       - 3 -- V.44 data compression
  */

  /* Optional */
  /*  Context Access Point Node (APN) Name ** */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String parameter that is a logical name
       used to select the GGSN and external packet data network.

       If the value is NULL or omitted, the subscription default
       value is requested. This value is applicable to 3GPP and EPC Profile
       types.

       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.
  */

  /* Optional */
  /*  Primary DNS IPv4 Address Preference ** */
  uint8_t primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if primary_DNS_IPv4_address_preference is being passed */
  uint32_t primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS IPv4 Address Preference ** */
  uint8_t secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if secondary_DNS_IPv4_address_preference is being passed */
  uint32_t secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  UMTS Requested QoS ** */
  uint8_t umts_requested_qos_valid;  /**< Must be set to true if umts_requested_qos is being passed */
  wds_umts_qos_type_v01 umts_requested_qos;

  /* Optional */
  /*  UMTS Minimum QoS ** */
  uint8_t umts_minimum_qos_valid;  /**< Must be set to true if umts_minimum_qos is being passed */
  wds_umts_qos_type_v01 umts_minimum_qos;

  /* Optional */
  /*  GPRS Requested QoS ** */
  uint8_t gprs_requested_qos_valid;  /**< Must be set to true if gprs_requested_qos is being passed */
  wds_gprs_qos_type_v01 gprs_requested_qos;

  /* Optional */
  /*  GRPS Minimum Qos ** */
  uint8_t gprs_minimum_qos_valid;  /**< Must be set to true if gprs_minimum_qos is being passed */
  wds_gprs_qos_type_v01 gprs_minimum_qos;

  /* Optional */
  /*  Username ** */
  uint8_t username_valid;  /**< Must be set to true if username is being passed */
  char username[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   Username used during data network authentication.

       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Password ** */
  uint8_t password_valid;  /**< Must be set to true if password is being passed */
  char password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Authentication Preference ** */
  uint8_t authentication_preference_valid;  /**< Must be set to true if authentication_preference is being passed */
  wds_auth_pref_mask_v01 authentication_preference;
  /**<   A bit map that indicates the authentication algorithm preference. Values: \n
       Bit 0 -- PAP preference: \n
       - 0 -- PAP is never performed \n
       - 1 -- PAP can be performed \n
       Bit 1 -- CHAP preference: \n
       - 0 -- CHAP is never performed \n
       - 1 -- CHAP can be performed \n
       All other bits are reserved and ignored. They must be set to zero
       by the client.

       If more than one bit is set, the device decides which
       authentication procedure is performed while setting up the data
       session, e.g., the device can have a policy to select
       the most secure authentication mechanism.
  */

  /* Optional */
  /*  IPv4 Address Preference ** */
  uint8_t ipv4_address_preference_valid;  /**< Must be set to true if ipv4_address_preference is being passed */
  uint32_t ipv4_address_preference;
  /**<   Preferred IPv4 address assigned to the TE. The actual assigned 
       address is negotiated with the network and can differ from this
       value. If not specified, the IPv4 address is obtained automatically
       from the network. The assigned value is provided to the host via DHCP.
  */

  /* Optional */
  /*  PCSCF Address Using PCO Flag ** */
  uint8_t pcscf_addr_using_pco_valid;  /**< Must be set to true if pcscf_addr_using_pco is being passed */
  uint8_t pcscf_addr_using_pco;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default the value is 0.
  */

  /* Optional */
  /*  PDP Access Control Flag ** */
  uint8_t pdp_access_control_flag_valid;  /**< Must be set to true if pdp_access_control_flag is being passed */
  wds_pdp_access_control_enum_v01 pdp_access_control_flag;
  /**<   Values: \n
       - 0 -- PDP access control none \n
       - 1 -- PDP access control reject \n
       - 2 -- PDP access control permission
  */

  /* Optional */
  /*  PCSCF Address Using DHCP ** */
  uint8_t pcscf_addr_using_dhcp_valid;  /**< Must be set to true if pcscf_addr_using_dhcp is being passed */
  uint8_t pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using DHCP \n
       - 0 -- FALSE -- Do not request \n
       By default the value is 0.
  */

  /* Optional */
  /*  IM CN flag ** */
  uint8_t im_cn_flag_valid;  /**< Must be set to true if im_cn_flag is being passed */
  uint8_t im_cn_flag;
  /**<   Values: \n
       - 1 -- TRUE -- Request IM CN flag for this profile \n
       - 0 -- FALSE -- Do not request IM CN flag for this profile
  */

  /* Optional */
  /*  Traffic Flow Template (TFT) ID1 Parameters ** */
  uint8_t tft_id1_params_valid;  /**< Must be set to true if tft_id1_params is being passed */
  wds_tft_id_param_type_v01 tft_id1_params;

  /* Optional */
  /*  TFT ID2 Parameters ** */
  uint8_t tft_id2_params_valid;  /**< Must be set to true if tft_id2_params is being passed */
  wds_tft_id_param_type_v01 tft_id2_params;

  /* Optional */
  /*  PDP Context Number ** */
  uint8_t pdp_context_valid;  /**< Must be set to true if pdp_context is being passed */
  uint8_t pdp_context;
  /**<   PDP context number*/

  /* Optional */
  /*  PDP Context Secondary Flag ** */
  uint8_t secondary_flag_valid;  /**< Must be set to true if secondary_flag is being passed */
  uint8_t secondary_flag;
  /**<   Values: \n
       - 1 -- TRUE -- This is secondary profile \n
       - 0 -- FALSE -- This is not secondary profile
  */

  /* Optional */
  /*  PDP Context Primary ID ** */
  uint8_t primary_id_valid;  /**< Must be set to true if primary_id is being passed */
  uint8_t primary_id;
  /**<   PDP context number primary ID.*/

  /* Optional */
  /*  IPv6 Address Preference ** */
  uint8_t ipv6_address_preference_valid;  /**< Must be set to true if ipv6_address_preference is being passed */
  uint8_t ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Preferred IPv6 address assigned to the TE. The actual assigned
       address is negotiated with the network and can differ from this value;
       if not specified, the IPv6 address is obtained automatically from the
       network.
  */

  /* Optional */
  /*  UMTS Requested QoS with Signaling Indication Flag ** */
  uint8_t umts_requested_qos_with_sig_ind_valid;  /**< Must be set to true if umts_requested_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_requested_qos_with_sig_ind;

  /* Optional */
  /*  UMTS Minimum QoS with Signaling Indication ** */
  uint8_t umts_minimum_qos_with_sig_ind_valid;  /**< Must be set to true if umts_minimum_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_minimum_qos_with_sig_ind;

  /* Optional */
  /*  Primary DNS IPv6 Address Preference ** */
  uint8_t primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if primary_dns_ipv6_address_preference is being passed */
  uint8_t primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The value can be used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP
  */

  /* Optional */
  /*  Secondary DNS IPv6 Address Preference ** */
  uint8_t secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if secodnary_dns_ipv6_address_preference is being passed */
  uint8_t secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The value can be used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP
  */

  /* Optional */
  /*  DHCP/NAS Preference ** */
  uint8_t addr_allocation_preference_valid;  /**< Must be set to true if addr_allocation_preference is being passed */
  wds_addr_allocation_preference_enum_v01 addr_allocation_preference;
  /**<   This enumerated value can be used to indicate the address allocation
       preference. Values: \n
         - 0 -- NAS signaling is used for address allocation \n
         - 1 -- DHCP is used for address allocation
  */

  /* Optional */
  /*  3GPP LTE QoS Parameters ** */
  uint8_t threegpp_lte_qos_params_valid;  /**< Must be set to true if threegpp_lte_qos_params is being passed */
  wds_3gpp_lte_qos_params_v01 threegpp_lte_qos_params;

  /* Optional */
  /*  APN Disabled Flag ** */
  uint8_t apn_disabled_flag_valid;  /**< Must be set to true if apn_disabled_flag is being passed */
  uint8_t apn_disabled_flag;
  /**<   Disables the use of this profile for 
       making data calls. Any data call with this profile 
       fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE 
   */

  /* Optional */
  /*  PDN Inactivity Timeout ** */
  uint8_t pdn_inactivity_timeout_valid;  /**< Must be set to true if pdn_inactivity_timeout is being passed */
  uint32_t pdn_inactivity_timeout;
  /**<   Duration of the inactivity timer in seconds. If a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, the PDP context/PDN
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class ** */
  uint8_t apn_class_valid;  /**< Must be set to true if apn_class is being passed */
  uint8_t apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. The APN class can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  APN Bearer ** */
  uint8_t apn_bearer_valid;  /**< Must be set to true if apn_bearer is being passed */
  wds_apn_bearer_mask_v01 apn_bearer;
  /**<   APN bearer mask. Specifies whether a data call is allowed on
       specific RAT types. Values: \n
       - 0x0000000000000001 -- GSM \n
       - 0x0000000000000002 -- WCDMA \n
       - 0x0000000000000004 -- LTE \n
       - 0x8000000000000000 -- Any
  */

  /* Optional */
  /*  Support Emergency Calls ** */
  uint8_t support_emergency_calls_valid;  /**< Must be set to true if support_emergency_calls is being passed */
  uint8_t support_emergency_calls;
  /**<    When this flag is set, the user can make emergency calls 
        using this profile. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE       
  */

  /* Optional */
  /*  Operator Reserved PCO ID ** */
  uint8_t op_pco_id_valid;  /**< Must be set to true if op_pco_id is being passed */
  uint16_t op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Mobile Country Code ** */
  uint8_t pco_mcc_valid;  /**< Must be set to true if pco_mcc is being passed */
  uint16_t pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code ** */
  uint8_t pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Max PDN Connections Per Time Block   ** */
  uint8_t max_pdn_conn_per_block_valid;  /**< Must be set to true if max_pdn_conn_per_block is being passed */
  uint16_t max_pdn_conn_per_block;
  /**<   Specifies the maximum number of PDN connections that the UE
    is allowed to perform with the network in a specified time block. 
    The time block size is defined by a configuration item.
    The default value is 1023. \n
    Range: 0 to 1023.
  */

  /* Optional */
  /*  Max PDN Connections Timer   ** */
  uint8_t max_pdn_conn_timer_valid;  /**< Must be set to true if max_pdn_conn_timer is being passed */
  uint16_t max_pdn_conn_timer;
  /**<   Specifies the time duration (in seconds) during which the UE 
   counts the PDN connections already made. 
   The default value is 300. \n
   Range: 0 to 3600 sec.
  */

  /* Optional */
  /*  PDN Request Wait Timer  ** */
  uint8_t pdn_req_wait_interval_valid;  /**< Must be set to true if pdn_req_wait_interval is being passed */
  uint16_t pdn_req_wait_interval;
  /**<   Specifies the minimum time interval (in seconds) between the new PDN 
   connection request and the last successful UE initiated PDN 
   disconnection.
   The default value is 0. \n
   Range: 0 to 1023 sec.
  */

  /* Optional */
  /*  3GPP Application User Data  ** */
  uint8_t app_user_data_3gpp_valid;  /**< Must be set to true if app_user_data_3gpp is being passed */
  uint32_t app_user_data_3gpp;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Roaming Disallow Flag ** */
  uint8_t roaming_disallowed_valid;  /**< Must be set to true if roaming_disallowed is being passed */
  uint8_t roaming_disallowed;
  /**<   Specifies whether the UE is allowed to connect 
       to the APN specified by the profile while roaming.
  */

  /* Optional */
  /*  PDN Disconnect Wait Timer ** */
  uint8_t pdn_discon_wait_timer_valid;  /**< Must be set to true if pdn_discon_wait_timer is being passed */
  uint8_t pdn_discon_wait_timer;
  /**<    Indicates the delay that the control point expects 
        to be available for successful deregistration with the network 
        before the modem disconnects the PDN(s). When the default value of 
        zero is specified, the modem disconnects the PDN immediately 
        upon moving to the roaming network, without waiting for the 
        control point.
        Range: 0-255 minutes.  
    */

  /* Optional */
  /*  DNS Address Using DHCP ** */
  uint8_t dns_addr_using_dhcp_valid;  /**< Must be set to true if dns_addr_using_dhcp is being passed */
  uint8_t dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common PCSCF Address Using DHCP ** * */
  uint8_t common_pcscf_addr_using_dhcp_valid;  /**< Must be set to true if common_pcscf_addr_using_dhcp is being passed */
  uint8_t common_pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)    
  */

  /* Optional */
  /*  Common DNS Address Using DHCP ** * */
  uint8_t common_dns_addr_using_dhcp_valid;  /**< Must be set to true if common_dns_addr_using_dhcp is being passed */
  uint8_t common_dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common PDP Type ** * */
  uint8_t common_pdp_type_valid;  /**< Must be set to true if common_pdp_type is being passed */
  wds_common_pdp_type_enum_v01 common_pdp_type;
  /**<   Specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-IPv6 \n
       - 2 -- PDP-IPv4v6
   */

  /* Optional */
  /*  Common Application User Data  ** */
  uint8_t common_app_user_data_valid;  /**< Must be set to true if common_app_user_data is being passed */
  uint32_t common_app_user_data;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Common Mobile Network Code *** */
  uint8_t common_pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if common_pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 common_pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Common Mobile Country Code *** */
  uint8_t common_pco_mcc_valid;  /**< Must be set to true if common_pco_mcc is being passed */
  uint16_t common_pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Common Operator Reserved PCO ID ** */
  uint8_t common_op_pco_id_valid;  /**< Must be set to true if common_op_pco_id is being passed */
  uint16_t common_op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Common Authentication Password *** */
  uint8_t common_auth_password_valid;  /**< Must be set to true if common_auth_password is being passed */
  char common_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Common User ID *** */
  uint8_t common_user_id_valid;  /**< Must be set to true if common_user_id is being passed */
  char common_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
    */

  /* Optional */
  /*  Common Authentication Protocol *** */
  uint8_t common_auth_protocol_valid;  /**< Must be set to true if common_auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 common_auth_protocol;
  /**<   Values: \n
       - 0 -- None \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  Common PCSCF Address Using PCO Flag *** */
  uint8_t common_is_pcscf_address_needed_valid;  /**< Must be set to true if common_is_pcscf_address_needed is being passed */
  uint8_t common_is_pcscf_address_needed;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default the value is 0.
  */

  /* Optional */
  /*  Common Allow/Disallow Lingering of Interface *** */
  uint8_t linger_params_valid;  /**< Must be set to true if linger_params is being passed */
  wds_common_linger_param_type_v01 linger_params;

  /* Optional */
  /*  Common Secondary DNS IPv6 Address Preference *** */
  uint8_t common_secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_secodnary_dns_ipv6_address_preference is being passed */
  uint8_t common_secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS IPv6 Address Preference *** */
  uint8_t common_primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_primary_dns_ipv6_address_preference is being passed */
  uint8_t common_primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Secondary DNS IPv4 Address Preference *** */
  uint8_t common_secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_secondary_DNS_IPv4_address_preference is being passed */
  uint32_t common_secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS Address Preference *** */
  uint8_t common_primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_primary_DNS_IPv4_address_preference is being passed */
  uint32_t common_primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the network.
       If not specified, the wireless device attempts to obtain the DNS
       address automatically from the network. The negotiated value is provided
       to the host via the DHCP.
  */

  /* Optional */
  /*  Common APN Class *** */
  uint8_t common_apn_class_valid;  /**< Must be set to true if common_apn_class is being passed */
  uint8_t common_apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. The APN class can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  Common APN Disabled Flag *** */
  uint8_t common_apn_disabled_flag_valid;  /**< Must be set to true if common_apn_disabled_flag is being passed */
  uint8_t common_apn_disabled_flag;
  /**<   Setting this flag disables the use of this profile for 
       making data calls. Any data call with this profile 
       fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE 
   */

  /* Optional */
  /*  Profile Persistence Flag * ** */
  uint8_t persistent_valid;  /**< Must be set to true if persistent is being passed */
  uint8_t persistent;
  /**<   Boolean value used to control whether the 
       profile to be created is persistent or not. 
       The default is persistent. Values: \n
       - 1 -- TRUE -- Profile is persistent \n
       - 0 -- FALSE -- Profile is not persistent        
  */

  /* Optional */
  /*  Negotiate DNS Server Preference * */
  uint8_t negotiate_dns_server_preference_valid;  /**< Must be set to true if negotiate_dns_server_preference is being passed */
  uint8_t negotiate_dns_server_preference;
  /**<   The default value is TRUE. Values: \n
       - 1 -- TRUE -- Request DNS address from the PDSN \n
       - 0 -- FALSE -- Do not request DNS address from the PDSN 
   */

  /* Optional */
  /*  PPP Session Close Timer for DO * */
  uint8_t ppp_session_close_timer_DO_valid;  /**< Must be set to true if ppp_session_close_timer_DO is being passed */
  uint32_t ppp_session_close_timer_DO;
  /**<   Timer value (in seconds) on DO indicating how long the PPP session
       must linger before closing down.
  */

  /* Optional */
  /*  PPP Session Close Timer for 1X * */
  uint8_t ppp_session_close_timer_1x_valid;  /**< Must be set to true if ppp_session_close_timer_1x is being passed */
  uint32_t ppp_session_close_timer_1x;
  /**<   Timer value (in seconds) on 1X indicating how
       long the PPP session must linger before
       closing down.
  */

  /* Optional */
  /*  Allow/Disallow Lingering of Interface * */
  uint8_t allow_linger_valid;  /**< Must be set to true if allow_linger is being passed */
  uint8_t allow_linger;
  /**<   Values: \n
       - 1 -- TRUE -- Allow lingering \n
       - 0 -- FALSE -- Do not allow lingering 
  */

  /* Optional */
  /*  LCP ACK Timeout * */
  uint8_t lcp_ack_timeout_valid;  /**< Must be set to true if lcp_ack_timeout is being passed */
  uint16_t lcp_ack_timeout;
  /**<   Value of LCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  IPCP ACK Timeout * */
  uint8_t ipcp_ack_timeout_valid;  /**< Must be set to true if ipcp_ack_timeout is being passed */
  uint16_t ipcp_ack_timeout;
  /**<   Value of IPCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  AUTH Timeout * */
  uint8_t auth_timeout_valid;  /**< Must be set to true if auth_timeout is being passed */
  uint16_t auth_timeout;
  /**<   Value of authentication timeout in milliseconds.
  */

  /* Optional */
  /*  LCP Configuration Request Retry Count Value * */
  uint8_t lcp_creq_retry_count_valid;  /**< Must be set to true if lcp_creq_retry_count is being passed */
  uint8_t lcp_creq_retry_count;
  /**<   LCP configuration request retry count value.
  */

  /* Optional */
  /*  IPCP Configuration Request Retry Count * */
  uint8_t ipcp_creq_retry_count_valid;  /**< Must be set to true if ipcp_creq_retry_count is being passed */
  uint8_t ipcp_creq_retry_count;
  /**<   IPCP configuration request retry count value.
  */

  /* Optional */
  /*  AUTH Retry * */
  uint8_t auth_retry_count_valid;  /**< Must be set to true if auth_retry_count is being passed */
  uint8_t auth_retry_count;
  /**<   Authentication retry count value.
  */

  /* Optional */
  /*  Authentication Protocol * */
  uint8_t auth_protocol_valid;  /**< Must be set to true if auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 auth_protocol;
  /**<   Values: \n
       - 0 -- NONE \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  User ID * */
  uint8_t user_id_valid;  /**< Must be set to true if user_id is being passed */
  char user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
    */

  /* Optional */
  /*  Authentication Password * */
  uint8_t auth_password_valid;  /**< Must be set to true if auth_password is being passed */
  char auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Data Rate * */
  uint8_t data_rate_valid;  /**< Must be set to true if data_rate is being passed */
  wds_profile_data_rate_enum_v01 data_rate;
  /**<   Values: \n
       - 0 -- Low (Low speed Service Options (SO15) only) \n
       - 1 -- Medium (SO33 + low R-SCH) \n
       - 2 -- High (SO33 + high R-SCH) \n
      Note: Default is 2.
  */

  /* Optional */
  /*  Application Type * */
  uint8_t app_type_valid;  /**< Must be set to true if app_type is being passed */
  wds_profile_app_type_enum_v01 app_type;
  /**<   Values: \n
       - 0x00000001 -- Default application type \n
       - 0x00000020 -- LBS application type \n
       - 0x00000040 -- Tethered application type \n
       Note: Application type value in a profile cannot be modified. 
       It can only be used to search for the profile ID numbers that have 
       the specified application type. \n
       Note: An error message is returned if this TLV is included in the request.
   */

  /* Optional */
  /*  Data Mode * */
  uint8_t data_mode_valid;  /**< Must be set to true if data_mode is being passed */
  wds_profile_data_mode_enum_v01 data_mode;
  /**<   Values: \n
       - 0 -- CDMA or HDR (Hybrid 1X/1xEV-DO) \n
       - 1 -- CDMA only (1X only) \n
       - 2 -- HDR only (1xEV-DO only) \n
       Note: Default is 0.
  */

  /* Optional */
  /*  Application Priority * */
  uint8_t app_priority_valid;  /**< Must be set to true if app_priority is being passed */
  uint8_t app_priority;
  /**<   Numerical one byte value defining the application
       priority; higher value implies higher priority. \n
       Note: Application priority value in a profile
       cannot be modified. It is currently listed for future
       extensibility of profile ID search based on
       application priority. \n
       Note: An error message is returned if this TLV is included in the request.
  */

  /* Optional */
  /*  APN String * */
  uint8_t apn_string_valid;  /**< Must be set to true if apn_string is being passed */
  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String representing the access point name;
       maximum length allowed is 100 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.
  */

  /* Optional */
  /*  PDN Type * */
  uint8_t pdn_type_valid;  /**< Must be set to true if pdn_type is being passed */
  wds_profile_pdn_type_enum_v01 pdn_type;
  /**<   Values: \n
       - 0 -- IPv4 PDN type \n
       - 1 -- IPv6 PDN type \n
       - 2 -- IPv4 or IPv6 PDN type \n
       - 3 -- Unspecified PDN type (implying no preference)
   */

  /* Optional */
  /*  Is PCSCF Address Needed * */
  uint8_t is_pcscf_address_needed_valid;  /**< Must be set to true if is_pcscf_address_needed is being passed */
  uint8_t is_pcscf_address_needed;
  /**<   Used to control whether the PCSCF
       address is requested from PDSN. Values: \n
       - 1 -- TRUE -- Request the PCSCF value from the PDSN \n
       - 0 -- FALSE -- Do not request the PCSCF value from the PDSN 
  */

  /* Optional */
  /*  IPv4 Primary DNS Address * */
  uint8_t primary_v4_dns_address_valid;  /**< Must be set to true if primary_v4_dns_address is being passed */
  uint32_t primary_v4_dns_address;
  /**<   The primary IPv4 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  IPv4 Secondary DNS Address * */
  uint8_t secondary_v4_dns_address_valid;  /**< Must be set to true if secondary_v4_dns_address is being passed */
  uint32_t secondary_v4_dns_address;
  /**<   The secondary IPv4 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  Primary IPv6 DNS Address * */
  uint8_t primary_v6_dns_address_valid;  /**< Must be set to true if primary_v6_dns_address is being passed */
  uint8_t primary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The primary IPv6 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  Secondary IPv6 DNS Address * */
  uint8_t secondary_v6_dns_address_valid;  /**< Must be set to true if secondary_v6_dns_address is being passed */
  uint8_t secondary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The secondary IPv6 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  RAT Type * */
  uint8_t rat_type_valid;  /**< Must be set to true if rat_type is being passed */
  wds_rat_type_enum_v01 rat_type;
  /**<   Values: \n
       - 1 -- HRPD \n
       - 2 -- EHRPD \n
       - 3 -- HRPD_EHRPD
  */

  /* Optional */
  /*  APN Enabled * */
  uint8_t apn_enabled_3gpp2_valid;  /**< Must be set to true if apn_enabled_3gpp2 is being passed */
  uint8_t apn_enabled_3gpp2;
  /**<    APN enabled is a flag to specify whether the APN in 
       that profile is enabled or disabled. If the APN is 
       disabled, the data call cannot be established 
       using that APN. Values: \n
       - 1 -- Enabled (default value) \n
       - 0 -- Disabled 
  */

  /* Optional */
  /*  PDN Inactivity Timeout * */
  uint8_t pdn_inactivity_timeout_3gpp2_valid;  /**< Must be set to true if pdn_inactivity_timeout_3gpp2 is being passed */
  uint32_t pdn_inactivity_timeout_3gpp2;
  /**<   The duration of the inactivity timer in minutes. If a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, the PDP context/PDN 
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class * */
  uint8_t apn_class_3gpp2_valid;  /**< Must be set to true if apn_class_3gpp2 is being passed */
  uint8_t apn_class_3gpp2;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. This can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  PDN Level Auth Protocol * */
  uint8_t pdn_level_auth_protocol_valid;  /**< Must be set to true if pdn_level_auth_protocol is being passed */
  wds_profile_pdn_lvl_auth_proto_enum_v01 pdn_level_auth_protocol;
  /**<   Authentication protocol used during PDN level authentication.
 Values: \n
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_NONE (0) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP (1) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP (2) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_CHAP (3) -- 
 */

  /* Optional */
  /*  PDN Level User ID * */
  uint8_t pdn_level_user_id_valid;  /**< Must be set to true if pdn_level_user_id is being passed */
  char pdn_level_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Level Auth Password * */
  uint8_t pdn_level_auth_password_valid;  /**< Must be set to true if pdn_level_auth_password is being passed */
  char pdn_level_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Label * */
  uint8_t pdn_label_valid;  /**< Must be set to true if pdn_label is being passed */
  char pdn_label[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   Logical name used to map the APN name for selecting the 
       packet data network. Maximum length allowed is 100 bytes. \n
       The following are the three steps of a request using the PDN label: \n
       1. Find the corresponding profile that has the indicated PDN label. \n 
       2. Get the profile's APN name. \n
       3. Use the APN name for the PDN connection.
  */

  /* Optional */
  /*  Operator Reserved PCO ID * */
  uint8_t op_pco_id_3gpp2_valid;  /**< Must be set to true if op_pco_id_3gpp2 is being passed */
  uint16_t op_pco_id_3gpp2;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Mobile Country Code * */
  uint8_t pco_mcc_3gpp2_valid;  /**< Must be set to true if pco_mcc_3gpp2 is being passed */
  uint16_t pco_mcc_3gpp2;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code * */
  uint8_t pco_mnc_includes_pcs_digit_3gpp2_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit_3gpp2 is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit_3gpp2;

  /* Optional */
  /*  PDN Throttling Timer 1-6 * */
  uint8_t failure_timer_valid;  /**< Must be set to true if failure_timer is being passed */
  uint32_t failure_timer[QMI_WDS_MAX_FAILURE_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after a PDN connection or 
  IP address assignment failure. For example, immediately following a third 
  consecutive PDN connection request failure, the UE waits failure_timer[2] 
  seconds before sending the fourth request. Following failures of six or greater,
  failure_timer[5] is used.
  */

  /* Optional */
  /*  PDN Disallow Timer 1-6 * */
  uint8_t disallow_timer_valid;  /**< Must be set to true if disallow_timer is being passed */
  uint32_t disallow_timer[QMI_WDS_MAX_DISALLOW_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after the network refuses 
  to grant the requested IP address type, such as when an IPv6 address is requested 
  from a network that only grants the IPv4 address. For example, immediately 
  after a third consecutive PDN connection request is denied, the UE waits 
  disallow_timer[2] seconds before sending the fourth request. 
  Following failures of six or greater, disallow_timer[5] is used.
  */

  /* Optional */
  /*  3GPP2 Application User Data  * */
  uint8_t app_user_data_3gpp2_valid;  /**< Must be set to true if app_user_data_3gpp2 is being passed */
  uint32_t app_user_data_3gpp2;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  PCSCF Address Using DHCP 3GPP2 * */
  uint8_t pcscf_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if pcscf_addr_using_dhcp_3gpp2 is being passed */
  uint8_t pcscf_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  DNS Address Using DHCP * */
  uint8_t dns_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if dns_addr_using_dhcp_3gpp2 is being passed */
  uint8_t dns_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  IPv6 Prefix Delegation Flag * ** */
  uint8_t ipv6_prefix_delegation_valid;  /**< Must be set to true if ipv6_prefix_delegation is being passed */
  uint8_t ipv6_prefix_delegation;
  /**<   Enables IPv6 prefix delegation.
     Values: \n
     - 0 -- FALSE (default) \n
     - 1 -- TRUE
  */
}wds_create_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the type of the profile. Values: \n
      - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n 
      - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_PROFILE_TYPE_EPC (0x02) --  EPC 
 */

  uint8_t profile_index;
  /**<   Index identifying the profile.*/
}wds_profile_identifier_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DS_EXTENDED_ERROR_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EEC_DS_PROFILE_REG_RESULT_FAIL_V01 = 0x01, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL_V01 = 0x02, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_OP_V01 = 0x03, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE_V01 = 0x04, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM_V01 = 0x05, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT_V01 = 0x06, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_V01 = 0x07, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED_V01 = 0x08, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_LEN_INVALID_V01 = 0x09, 
  WDS_EEC_DS_PROFILE_REG_RESULT_LIST_END_V01 = 0x0A, 
  WDS_EEC_DS_PROFILE_REG_RESULT_ERR_INVAL_SUBS_ID_V01 = 0x0B, 
  WDS_EEC_DS_PROFILE_REG_INVAL_PROFILE_FAMILY_V01 = 0x0C, 
  WDS_EEC_DS_PROFILE_REG_3GPP_INVAL_PROFILE_FAMILY_V01 = 0x3E9, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_REG_3GPP_ACCESS_ERR_V01 = 0x3EA, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_REG_3GPP_CONTEXT_NOT_DEFINED_V01 = 0x3EB, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET_V01 = 0x3EC, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_REG_3GPP_READ_ONLY_FLAG_SET_V01 = 0x3ED, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_REG_3GPP_ERR_OUT_OF_PROFILES_V01 = 0x3EE, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_REG_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE_V01 = 0x44D, /**<  Deprecated rev 1.25  */
  WDS_EEC_DS_PROFILE_3GPP_INVAL_PROFILE_FAMILY_V01 = 0x1001, 
  WDS_EEC_DS_PROFILE_3GPP_ACCESS_ERR_V01 = 0x1002, 
  WDS_EEC_DS_PROFILE_3GPP_CONTEXT_NOT_DEFINED_V01 = 0x1003, 
  WDS_EEC_DS_PROFILE_3GPP_VALID_FLAG_NOT_SET_V01 = 0x1004, 
  WDS_EEC_DS_PROFILE_3GPP_READ_ONLY_FLAG_SET_V01 = 0x1005, 
  WDS_EEC_DS_PROFILE_3GPP_ERR_OUT_OF_PROFILES_V01 = 0x1006, 
  WDS_EEC_DS_PROFILE_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE_V01 = 0x1101, 
  WDS_EEC_DS_PROFILE_3GPP2_ERR_OUT_OF_PROFILE_V01 = 0x1102, 
  WDS_DS_EXTENDED_ERROR_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ds_extended_error_code_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Creates a configured profile with specified settings. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Profile Identifier */
  wds_profile_identifier_type_v01 profile;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   The extended error code received from the DS Profile subsystem.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_create_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Changes the settings in a configured profile. */
typedef struct {

  /* Mandatory */
  /*  Profile Identifier */
  wds_profile_identifier_type_v01 profile;

  /* Optional */
  /*  Profile Name ** */
  uint8_t profile_name_valid;  /**< Must be set to true if profile_name is being passed */
  char profile_name[QMI_WDS_PROFILE_NAME_MAX_V01 + 1];
  /**<   One or more bytes describing the profile. The description can
       be a user-defined name for the profile.
       QMI_ERR_ARG_TOO_LONG is returned if the profile_name is too long.
   */

  /* Optional */
  /*  PDP Type ** */
  uint8_t pdp_type_valid;  /**< Must be set to true if pdp_type is being passed */
  wds_pdp_type_enum_v01 pdp_type;
  /**<   Packet Data Protocol (PDP) type specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-PPP \n
       - 2 -- PDP-IPv6 \n
       - 3 -- PDP-IPv4v6
   */

  /* Optional */
  /*  PDP Header Compression Type ** */
  uint8_t pdp_hdr_compression_type_valid;  /**< Must be set to true if pdp_hdr_compression_type is being passed */
  wds_pdp_hdr_compr_type_enum_v01 pdp_hdr_compression_type;
  /**<   Values: \n
       - 0 -- PDP header compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- PDP header compression based on RFC 1144 \n
       - 3 -- PDP header compression based on RFC 2507 \n
       - 4 -- PDP header compression based on RFC 3095
  */

  /* Optional */
  /*  PDP Data Compression Type ** */
  uint8_t pdp_data_compression_type_valid;  /**< Must be set to true if pdp_data_compression_type is being passed */
  wds_pdp_data_compr_type_enum_v01 pdp_data_compression_type;
  /**<   Values: \n
       - 0 -- PDP data compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- V.42BIS data compresion \n
       - 3 -- V.44 data compresion
  */

  /* Optional */
  /*  Context Access Point Node (APN) Name ** */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   A string parameter that is a logical name
       used to select the GGSN and external packet data network.

       If the value is NULL or omitted, the subscription default
       value is requested.

       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.
  */

  /* Optional */
  /*  Primary DNS IPv4 Address Preference ** */
  uint8_t primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if primary_DNS_IPv4_address_preference is being passed */
  uint32_t primary_DNS_IPv4_address_preference;
  /**<   This value can be used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS IPv4 Address Preference ** */
  uint8_t secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if secondary_DNS_IPv4_address_preference is being passed */
  uint32_t secondary_DNS_IPv4_address_preference;
  /**<   This value can be used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  UMTS Requested QoS ** */
  uint8_t umts_requested_qos_valid;  /**< Must be set to true if umts_requested_qos is being passed */
  wds_umts_qos_type_v01 umts_requested_qos;

  /* Optional */
  /*  UMTS Minimum QoS ** */
  uint8_t umts_minimum_qos_valid;  /**< Must be set to true if umts_minimum_qos is being passed */
  wds_umts_qos_type_v01 umts_minimum_qos;

  /* Optional */
  /*  GPRS Requested QoS ** */
  uint8_t gprs_requested_qos_valid;  /**< Must be set to true if gprs_requested_qos is being passed */
  wds_gprs_qos_type_v01 gprs_requested_qos;

  /* Optional */
  /*  GRPS Minimum Qos ** */
  uint8_t gprs_minimum_qos_valid;  /**< Must be set to true if gprs_minimum_qos is being passed */
  wds_gprs_qos_type_v01 gprs_minimum_qos;

  /* Optional */
  /*  Username ** */
  uint8_t username_valid;  /**< Must be set to true if username is being passed */
  char username[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   Username used during data network authentication.

       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Password **  */
  uint8_t password_valid;  /**< Must be set to true if password is being passed */
  char password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password to be used during data network authentication.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Authentication Preference ** */
  uint8_t authentication_preference_valid;  /**< Must be set to true if authentication_preference is being passed */
  wds_auth_pref_mask_v01 authentication_preference;
  /**<   A bit map that indicates the authentication algorithm preference. Values: \n
       Bit 0 -- PAP preference: \n
       - 0 -- PAP is never performed \n
       - 1 -- PAP may be performed \n
       Bit 1 -- CHAP preference: \n
       - 0 -- CHAP is never performed \n
       - 1 -- CHAP may be performed \n
       All other bits are reserved and ignored. They must be set to zero
       by the client.

       If more than one bit is set, the device decides which
       authentication procedure is performed while setting up the data
       session. For example, the device can have a policy to select
       the most secure authentication mechanism.
  */

  /* Optional */
  /*  IPv4 Address Preference ** */
  uint8_t ipv4_address_preference_valid;  /**< Must be set to true if ipv4_address_preference is being passed */
  uint32_t ipv4_address_preference;
  /**<   The preferred IPv4 address assigned to the TE.  
       The actual assigned address is negotiated with the network and
       may differ from this value. If not specified, the IPv4 Address
       is obtained automatically from the network. The assigned value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  PCSCF Address Using PCO Flag ** */
  uint8_t pcscf_addr_using_pco_valid;  /**< Must be set to true if pcscf_addr_using_pco is being passed */
  uint8_t pcscf_addr_using_pco;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default this value is 0.
  */

  /* Optional */
  /*  PDP Access Control Flag ** */
  uint8_t pdp_access_control_flag_valid;  /**< Must be set to true if pdp_access_control_flag is being passed */
  wds_pdp_access_control_enum_v01 pdp_access_control_flag;
  /**<   Values: \n
       - 0 -- PDP access control none \n
       - 1 -- PDP access control reject \n
       - 2 -- PDP access control permission
  */

  /* Optional */
  /*  PCSCF Address Using DHCP ** */
  uint8_t pcscf_addr_using_dhcp_valid;  /**< Must be set to true if pcscf_addr_using_dhcp is being passed */
  uint8_t pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request \n
       By default, the value is 0.
  */

  /* Optional */
  /*  IM CN flag ** */
  uint8_t im_cn_flag_valid;  /**< Must be set to true if im_cn_flag is being passed */
  uint8_t im_cn_flag;
  /**<   Values: \n
       - 1 -- TRUE -- Request the IM CN flag for this profile \n
       - 0 -- FALSE -- Do not request the IM CN flag for this profile
  */

  /* Optional */
  /*  Traffic Flow Template (TFT) ID1 Parameters ** */
  uint8_t tft_id1_params_valid;  /**< Must be set to true if tft_id1_params is being passed */
  wds_tft_id_param_type_v01 tft_id1_params;

  /* Optional */
  /*  TFT ID2 Parameters ** */
  uint8_t tft_id2_params_valid;  /**< Must be set to true if tft_id2_params is being passed */
  wds_tft_id_param_type_v01 tft_id2_params;

  /* Optional */
  /*  PDP Context Number ** */
  uint8_t pdp_context_valid;  /**< Must be set to true if pdp_context is being passed */
  uint8_t pdp_context;
  /**<   PDP context number*/

  /* Optional */
  /*  PDP Context Secondary Flag ** */
  uint8_t secondary_flag_valid;  /**< Must be set to true if secondary_flag is being passed */
  uint8_t secondary_flag;
  /**<   Values: \n
       - 1 -- TRUE -- This is the secondary profile \n
       - 0 -- FALSE -- This is not the secondary profile
  */

  /* Optional */
  /*  PDP Context Primary ID ** */
  uint8_t primary_id_valid;  /**< Must be set to true if primary_id is being passed */
  uint8_t primary_id;
  /**<   PDP context number primary ID.*/

  /* Optional */
  /*  IPv6 Address Preference ** */
  uint8_t ipv6_address_preference_valid;  /**< Must be set to true if ipv6_address_preference is being passed */
  uint8_t ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The preferred IPv6 address to be assigned to the TE. The actual assigned
       address is negotiated with the network and can differ from this value;
       if not specified, the IPv6 address is obtained automatically from the
       network.
  */

  /* Optional */
  /*  UMTS Requested QoS with Signaling Indication Flag ** */
  uint8_t umts_requested_qos_with_sig_ind_valid;  /**< Must be set to true if umts_requested_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_requested_qos_with_sig_ind;

  /* Optional */
  /*  UMTS Minimum QoS with Signaling Indication ** */
  uint8_t umts_minimum_qos_with_sig_ind_valid;  /**< Must be set to true if umts_minimum_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_minimum_qos_with_sig_ind;

  /* Optional */
  /*  Primary DNS IPv6 Address Preference ** */
  uint8_t primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if primary_dns_ipv6_address_preference is being passed */
  uint8_t primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The value can be used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS IPv6 Address Preference ** */
  uint8_t secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if secodnary_dns_ipv6_address_preference is being passed */
  uint8_t secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The value can be used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  DHCP/NAS Preference ** */
  uint8_t addr_allocation_preference_valid;  /**< Must be set to true if addr_allocation_preference is being passed */
  wds_addr_allocation_preference_enum_v01 addr_allocation_preference;
  /**<   This enumerated value can be used to indicate the address allocation
       preference. Values: \n
         - 0 -- NAS signaling is used for address allocation \n
         - 1 -- DHCP is used for address allocation
  */

  /* Optional */
  /*  3GPP LTE QoS Parameters ** */
  uint8_t threegpp_lte_qos_params_valid;  /**< Must be set to true if threegpp_lte_qos_params is being passed */
  wds_3gpp_lte_qos_params_v01 threegpp_lte_qos_params;

  /* Optional */
  /*  APN Disabled Flag ** */
  uint8_t apn_disabled_flag_valid;  /**< Must be set to true if apn_disabled_flag is being passed */
  uint8_t apn_disabled_flag;
  /**<   When this flag is set, the use of this profile for 
       making a data call is disabled. Any data call with 
       this profile fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE        
  */

  /* Optional */
  /*  PDN Inactivity Timeout ** */
  uint8_t pdn_inactivity_timeout_valid;  /**< Must be set to true if pdn_inactivity_timeout is being passed */
  uint32_t pdn_inactivity_timeout;
  /**<   The duration of the inactivity timer in seconds. When
       a PDP context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, the PDP context/PDN
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class ** */
  uint8_t apn_class_valid;  /**< Must be set to true if apn_class is being passed */
  uint8_t apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. This can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  APN Bearer ** */
  uint8_t apn_bearer_valid;  /**< Must be set to true if apn_bearer is being passed */
  wds_apn_bearer_mask_v01 apn_bearer;
  /**<   APN bearer mask. Specifies whether a data call is allowed on
       specific RAT types. Values: \n
       - 0x0000000000000001 -- GSM \n
       - 0x0000000000000002 -- WCDMA \n
       - 0x0000000000000004 -- LTE \n
       - 0x8000000000000000 -- Any
  */

  /* Optional */
  /*  Support Emergency Calls ** */
  uint8_t support_emergency_calls_valid;  /**< Must be set to true if support_emergency_calls is being passed */
  uint8_t support_emergency_calls;
  /**<    When this flag is set, the user can make emergency calls 
        using this profile. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE       
  */

  /* Optional */
  /*  Operator Reserved PCO ID ** */
  uint8_t op_pco_id_valid;  /**< Must be set to true if op_pco_id is being passed */
  uint16_t op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
   */

  /* Optional */
  /*  Mobile Country Code ** */
  uint8_t pco_mcc_valid;  /**< Must be set to true if pco_mcc is being passed */
  uint16_t pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code ** */
  uint8_t pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Max PDN Connections Per Time Block   ** */
  uint8_t max_pdn_conn_per_block_valid;  /**< Must be set to true if max_pdn_conn_per_block is being passed */
  uint16_t max_pdn_conn_per_block;
  /**<   Specifies the maximum number of PDN connections that the UE
    is allowed to perform with the network in a specified time block. 
    The time block size is defined by a configuration item.
    The default value is 1023. \n
    Range: 0 to 1023.
  */

  /* Optional */
  /*  Max PDN Connections Timer   ** */
  uint8_t max_pdn_conn_timer_valid;  /**< Must be set to true if max_pdn_conn_timer is being passed */
  uint16_t max_pdn_conn_timer;
  /**<   Specifies the time duration in seconds during which the UE 
   counts the PDN connections already made. 
   The default value is 300. \n
   Range: 0 to 3600 seconds.
  */

  /* Optional */
  /*  PDN Request Wait Timer  ** */
  uint8_t pdn_req_wait_interval_valid;  /**< Must be set to true if pdn_req_wait_interval is being passed */
  uint16_t pdn_req_wait_interval;
  /**<   Specifies the minimum time interval between the new PDN 
   connection request and the last successful UE initiated PDN 
   disconnection.
   The default value is 0. \n
   Range: 0 to 1023 sec.
  */

  /* Optional */
  /*  3GPP Application User Data  ** */
  uint8_t app_user_data_3gpp_valid;  /**< Must be set to true if app_user_data_3gpp is being passed */
  uint32_t app_user_data_3gpp;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Roaming Disallow Flag ** */
  uint8_t roaming_disallowed_valid;  /**< Must be set to true if roaming_disallowed is being passed */
  uint8_t roaming_disallowed;
  /**<   Specifies whether the UE is allowed to connect 
       to the APN specified by the profile while roaming.
  */

  /* Optional */
  /*  PDN Disconnect Wait Timer ** */
  uint8_t pdn_discon_wait_timer_valid;  /**< Must be set to true if pdn_discon_wait_timer is being passed */
  uint8_t pdn_discon_wait_timer;
  /**<    Indicates the delay that the control point expects 
        to be available for successful deregistration with the network 
        before the modem disconnects the PDN(s). When the default value of 
        zero is specified, the modem disconnects the PDN immediately 
        upon moving to the roaming network, without waiting for the 
        control point.
        Range: 0-255 minutes.  
  */

  /* Optional */
  /*  DNS Address Using DHCP ** */
  uint8_t dns_addr_using_dhcp_valid;  /**< Must be set to true if dns_addr_using_dhcp is being passed */
  uint8_t dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common PCSCF Address Using DHCP ** * */
  uint8_t common_pcscf_addr_using_dhcp_valid;  /**< Must be set to true if common_pcscf_addr_using_dhcp is being passed */
  uint8_t common_pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common DNS Address Using DHCP ** * */
  uint8_t common_dns_addr_using_dhcp_valid;  /**< Must be set to true if common_dns_addr_using_dhcp is being passed */
  uint8_t common_dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)      
  */

  /* Optional */
  /*  Common PDP Type ** * */
  uint8_t common_pdp_type_valid;  /**< Must be set to true if common_pdp_type is being passed */
  wds_common_pdp_type_enum_v01 common_pdp_type;
  /**<   Specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-IPv6 \n
       - 2 -- PDP-IPv4v6
   */

  /* Optional */
  /*  Common Application User Data  ** */
  uint8_t common_app_user_data_valid;  /**< Must be set to true if common_app_user_data is being passed */
  uint32_t common_app_user_data;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Common Mobile Network Code *** */
  uint8_t common_pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if common_pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 common_pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Common Mobile Country Code *** */
  uint8_t common_pco_mcc_valid;  /**< Must be set to true if common_pco_mcc is being passed */
  uint16_t common_pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Common Operator Reserved PCO ID ** */
  uint8_t common_op_pco_id_valid;  /**< Must be set to true if common_op_pco_id is being passed */
  uint16_t common_op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Common Authentication Password *** */
  uint8_t common_auth_password_valid;  /**< Must be set to true if common_auth_password is being passed */
  char common_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Common User ID *** */
  uint8_t common_user_id_valid;  /**< Must be set to true if common_user_id is being passed */
  char common_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
    */

  /* Optional */
  /*  Common Authentication Protocol *** */
  uint8_t common_auth_protocol_valid;  /**< Must be set to true if common_auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 common_auth_protocol;
  /**<   Values: \n
       - 0 -- None \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  Common PCSCF Address Using PCO Flag *** */
  uint8_t common_is_pcscf_address_needed_valid;  /**< Must be set to true if common_is_pcscf_address_needed is being passed */
  uint8_t common_is_pcscf_address_needed;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default the value is 0.
  */

  /* Optional */
  /*  Common Allow/Disallow Lingering of Interface *** */
  uint8_t linger_params_valid;  /**< Must be set to true if linger_params is being passed */
  wds_common_linger_param_type_v01 linger_params;

  /* Optional */
  /*  Common Secondary DNS IPv6 Address Preference *** */
  uint8_t common_secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_secodnary_dns_ipv6_address_preference is being passed */
  uint8_t common_secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS IPv6 Address Preference *** */
  uint8_t common_primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_primary_dns_ipv6_address_preference is being passed */
  uint8_t common_primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Secondary DNS IPv4 Address Preference *** */
  uint8_t common_secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_secondary_DNS_IPv4_address_preference is being passed */
  uint32_t common_secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS Address Preference *** */
  uint8_t common_primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_primary_DNS_IPv4_address_preference is being passed */
  uint32_t common_primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the network;
       if not specified, the wireless device attempts to obtain the DNS
       address automatically from the network. The negotiated value is provided
       to the host via the DHCP.
  */

  /* Optional */
  /*  Common APN Class *** */
  uint8_t common_apn_class_valid;  /**< Must be set to true if common_apn_class is being passed */
  uint8_t common_apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. The APN class can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  Common APN Disabled Flag *** */
  uint8_t common_apn_disabled_flag_valid;  /**< Must be set to true if common_apn_disabled_flag is being passed */
  uint8_t common_apn_disabled_flag;
  /**<   Setting this flag disables the use of this profile for 
       making data calls. Any data call with this profile 
       fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE 
   */

  /* Optional */
  /*  Negotiate DNS Server Preference * */
  uint8_t negotiate_dns_server_preference_valid;  /**< Must be set to true if negotiate_dns_server_preference is being passed */
  uint8_t negotiate_dns_server_preference;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address from the PDSN \n
       - 0 -- FALSE -- Do not request DNS addresses from the PDSN \n
       Note: Default value is 1 (TRUE).
  */

  /* Optional */
  /*  PPP Session Close Timer for DO * */
  uint8_t ppp_session_close_timer_DO_valid;  /**< Must be set to true if ppp_session_close_timer_DO is being passed */
  uint32_t ppp_session_close_timer_DO;
  /**<   Timer value (in seconds) on the DO indicating how long 
       the PPP session lingers before closing down.
  */

  /* Optional */
  /*  PPP Session Close Timer for 1X * */
  uint8_t ppp_session_close_timer_1x_valid;  /**< Must be set to true if ppp_session_close_timer_1x is being passed */
  uint32_t ppp_session_close_timer_1x;
  /**<   Timer value (in seconds) on 1X indicating how
       long the PPP session lingers before
       closing down.
  */

  /* Optional */
  /*  Allow/Disallow Lingering of Interface * */
  uint8_t allow_linger_valid;  /**< Must be set to true if allow_linger is being passed */
  uint8_t allow_linger;
  /**<   Values: \n
       - 1 -- TRUE -- Allow lingering \n
       - 0 -- FALSE -- Do not allow lingering 
  */

  /* Optional */
  /*  LCP ACK Timeout * */
  uint8_t lcp_ack_timeout_valid;  /**< Must be set to true if lcp_ack_timeout is being passed */
  uint16_t lcp_ack_timeout;
  /**<   Value of LCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  IPCP ACK Timeout * */
  uint8_t ipcp_ack_timeout_valid;  /**< Must be set to true if ipcp_ack_timeout is being passed */
  uint16_t ipcp_ack_timeout;
  /**<   Value of IPCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  Authentication Timeout * */
  uint8_t auth_timeout_valid;  /**< Must be set to true if auth_timeout is being passed */
  uint16_t auth_timeout;
  /**<   Value of authentication timeout in milliseconds.
  */

  /* Optional */
  /*  LCP Configuration Request Retry Count Value * */
  uint8_t lcp_creq_retry_count_valid;  /**< Must be set to true if lcp_creq_retry_count is being passed */
  uint8_t lcp_creq_retry_count;
  /**<   LCP configuration request retry count value.
  */

  /* Optional */
  /*  IPCP Configuration Request Retry Count * */
  uint8_t ipcp_creq_retry_count_valid;  /**< Must be set to true if ipcp_creq_retry_count is being passed */
  uint8_t ipcp_creq_retry_count;
  /**<   IPCP configuration request retry count value.
  */

  /* Optional */
  /*  AUTH Retry * */
  uint8_t auth_retry_count_valid;  /**< Must be set to true if auth_retry_count is being passed */
  uint8_t auth_retry_count;
  /**<   Authentication retry count value.
  */

  /* Optional */
  /*  Authentication Protocol * */
  uint8_t auth_protocol_valid;  /**< Must be set to true if auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 auth_protocol;
  /**<   Values: \n
       - 0 -- NONE \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP 
  */

  /* Optional */
  /*  User ID * */
  uint8_t user_id_valid;  /**< Must be set to true if user_id is being passed */
  char user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network
       authentication; maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the storage on the wireless device is insufficient
       in size to hold the value.
    */

  /* Optional */
  /*  Authentication Password * */
  uint8_t auth_password_valid;  /**< Must be set to true if auth_password is being passed */
  char auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network
       authentication; maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the storage on the wireless device is insufficient
       in size to hold the value.
  */

  /* Optional */
  /*  Data Rate * */
  uint8_t data_rate_valid;  /**< Must be set to true if data_rate is being passed */
  wds_profile_data_rate_enum_v01 data_rate;
  /**<   Values: \n
       - 0 -- Low (Low speed Service Options (SO15) only) \n
       - 1 -- Medium (SO33 + low R-SCH) \n
       - 2 -- High (SO33 + high R-SCH) \n
      Note: Default is 2.
  */

  /* Optional */
  /*  Application Type * */
  uint8_t app_type_valid;  /**< Must be set to true if app_type is being passed */
  wds_profile_app_type_enum_v01 app_type;
  /**<   Values: \n
       - 0x00000001 -- Default application type \n
       - 0x00000020 -- LBS application type \n
       - 0x00000040 -- tethered application type \n
       Note: Application type value in a profile cannot
       be modified. It can only be used to search for the
       profile ID numbers that have the specified
       application type. \n
       Note: An error message is returned if this TLV is included in the request.
   */

  /* Optional */
  /*  Data Mode * */
  uint8_t data_mode_valid;  /**< Must be set to true if data_mode is being passed */
  wds_profile_data_mode_enum_v01 data_mode;
  /**<   Values: \n
       - 0 -- CDMA or HDR (Hybrid 1X/1xEV-DO) \n
       - 1 -- CDMA only (1X only) \n
       - 2 -- HDR only (1xEV-DO only) \n
       Note: Default is 0.
  */

  /* Optional */
  /*  Application Priority * */
  uint8_t app_priority_valid;  /**< Must be set to true if app_priority is being passed */
  uint8_t app_priority;
  /**<   Numerical one byte value defining the application
       priority; higher value implies higher priority. \n
       Note: Application priority value in a profile
       cannot be modified. It is currently listed for future
       extensibility of profile ID search based on
       application priority. \n
       Note: An error message is returned if this TLV is included in the request.

  */

  /* Optional */
  /*  APN String * */
  uint8_t apn_string_valid;  /**< Must be set to true if apn_string is being passed */
  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String representing the access point name (APN);
       the maximum length allowed is 100 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the APN name is too long.
  */

  /* Optional */
  /*  PDN Type * */
  uint8_t pdn_type_valid;  /**< Must be set to true if pdn_type is being passed */
  wds_profile_pdn_type_enum_v01 pdn_type;
  /**<   Values: \n
       - 0 -- IPv4 PDN type \n
       - 1 -- IPv6 PDN type \n
       - 2 -- IPv4 or IPv6 PDN type \n
       - 3 -- Unspecified PDN type (no preference)
   */

  /* Optional */
  /*  Is PCSCF Address Needed * */
  uint8_t is_pcscf_address_needed_valid;  /**< Must be set to true if is_pcscf_address_needed is being passed */
  uint8_t is_pcscf_address_needed;
  /**<   Used to control whether the PCSCF
       address is requested from PDSN. Values: \n
       - 1 -- TRUE -- Request for PCSCF value from the PDSN \n
       - 0 -- FALSE -- Do not request for PCSCF value from the PDSN
  */

  /* Optional */
  /*  IPv4 Primary DNS Address * */
  uint8_t primary_v4_dns_address_valid;  /**< Must be set to true if primary_v4_dns_address is being passed */
  uint32_t primary_v4_dns_address;
  /**<   The primary IPv4 DNS address 
       statically assigned to the UE.
  */

  /* Optional */
  /*  IPv4 Secondary DNS Address * */
  uint8_t secondary_v4_dns_address_valid;  /**< Must be set to true if secondary_v4_dns_address is being passed */
  uint32_t secondary_v4_dns_address;
  /**<   The secondary IPv4 DNS address 
       statically assigned to the UE.
  */

  /* Optional */
  /*  Primary IPv6 DNS Address * */
  uint8_t primary_v6_dns_address_valid;  /**< Must be set to true if primary_v6_dns_address is being passed */
  uint8_t primary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The primary IPv6 DNS address 
       statically assigned to the UE.
  */

  /* Optional */
  /*  Secondary IPv6 DNS address * */
  uint8_t secondary_v6_dns_address_valid;  /**< Must be set to true if secondary_v6_dns_address is being passed */
  uint8_t secondary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The secondary IPv6 DNS address
       statically assigned to the UE.
  */

  /* Optional */
  /*  RAT Type * */
  uint8_t rat_type_valid;  /**< Must be set to true if rat_type is being passed */
  wds_rat_type_enum_v01 rat_type;
  /**<   Values: \n
       - 1 -- HRPD \n
       - 2 -- EHRPD \n
       - 3 -- HRPD_EHRPD
  */

  /* Optional */
  /*  APN Enabled * */
  uint8_t apn_enabled_3gpp2_valid;  /**< Must be set to true if apn_enabled_3gpp2 is being passed */
  uint8_t apn_enabled_3gpp2;
  /**<   APN enabled is a flag to specify whether the APN in 
       that profile is enabled or disabled. If the APN is 
       disabled, the data call cannot be established 
       using that APN. Values: \n
       - 1 -- Enabled (default value) \n
       - 0 -- Disabled        
  */

  /* Optional */
  /*  PDN Inactivity Timeout * */
  uint8_t pdn_inactivity_timeout_3gpp2_valid;  /**< Must be set to true if pdn_inactivity_timeout_3gpp2 is being passed */
  uint32_t pdn_inactivity_timeout_3gpp2;
  /**<   Duration of inactivity timer in minutes. If a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, the PDP context/PDN 
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class 3GPP2 * */
  uint8_t apn_class_3gpp2_valid;  /**< Must be set to true if apn_class_3gpp2 is being passed */
  uint8_t apn_class_3gpp2;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. This can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  PDN Level Auth Protocol * */
  uint8_t pdn_level_auth_protocol_valid;  /**< Must be set to true if pdn_level_auth_protocol is being passed */
  wds_profile_pdn_lvl_auth_proto_enum_v01 pdn_level_auth_protocol;
  /**<   Authentication protocol used during PDN level authentication.
 Values: \n
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_NONE (0) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP (1) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP (2) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_CHAP (3) -- 
 */

  /* Optional */
  /*  PDN Level User ID * */
  uint8_t pdn_level_user_id_valid;  /**< Must be set to true if pdn_level_user_id is being passed */
  char pdn_level_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Level Auth Password * */
  uint8_t pdn_level_auth_password_valid;  /**< Must be set to true if pdn_level_auth_password is being passed */
  char pdn_level_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Label * */
  uint8_t pdn_label_valid;  /**< Must be set to true if pdn_label is being passed */
  char pdn_label[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   Logical name used to map the APN name for selecting the 
       packet data network. Maximum length allowed is 100 bytes.
  */

  /* Optional */
  /*  Operator Reserved PCO ID * */
  uint8_t op_pco_id_3gpp2_valid;  /**< Must be set to true if op_pco_id_3gpp2 is being passed */
  uint16_t op_pco_id_3gpp2;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
   */

  /* Optional */
  /*  Mobile Country Code * */
  uint8_t pco_mcc_3gpp2_valid;  /**< Must be set to true if pco_mcc_3gpp2 is being passed */
  uint16_t pco_mcc_3gpp2;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code * */
  uint8_t pco_mnc_includes_pcs_digit_3gpp2_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit_3gpp2 is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit_3gpp2;

  /* Optional */
  /*  PDN Throttling Timer 1-6 * */
  uint8_t failure_timer_valid;  /**< Must be set to true if failure_timer is being passed */
  uint32_t failure_timer[QMI_WDS_MAX_FAILURE_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after a PDN connection or 
  IP address assignment failure. For example, immediately following a third 
  consecutive PDN connection request failure, the UE waits failure_timer[2] 
  seconds before sending the fourth request. Following failures of six or greater,
  failure_timer[5] is used.
  */

  /* Optional */
  /*  PDN Disallow Timer 1-6 * */
  uint8_t disallow_timer_valid;  /**< Must be set to true if disallow_timer is being passed */
  uint32_t disallow_timer[QMI_WDS_MAX_DISALLOW_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after the network refuses 
  to grant the requested IP address type, such as when an IPv6 address is requested 
  from a network that only grants the IPv4 address. For example, immediately 
  after a third consecutive PDN connection request is denied, the UE waits 
  disallow_timer[2] seconds before sending the fourth request. Following failures 
  of six or greater, disallow_timer[5] is used.
  */

  /* Optional */
  /*  3GPP2 Application User Data  * */
  uint8_t app_user_data_3gpp2_valid;  /**< Must be set to true if app_user_data_3gpp2 is being passed */
  uint32_t app_user_data_3gpp2;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  PCSCF Address Using DHCP 3GPP2 * */
  uint8_t pcscf_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if pcscf_addr_using_dhcp_3gpp2 is being passed */
  uint8_t pcscf_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)      
  */

  /* Optional */
  /*  DNS Address Using DHCP * */
  uint8_t dns_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if dns_addr_using_dhcp_3gpp2 is being passed */
  uint8_t dns_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)      
  */

  /* Optional */
  /*  IPv6 Prefix Delegation Flag * ** */
  uint8_t ipv6_prefix_delegation_valid;  /**< Must be set to true if ipv6_prefix_delegation is being passed */
  uint8_t ipv6_prefix_delegation;
  /**<   Enables IPv6 prefix delegation.
     Values: \n
     - 0 -- FALSE (default) \n
     - 1 -- TRUE
  */
}wds_modify_profile_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Changes the settings in a configured profile. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_modify_profile_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Deletes a configured profile. */
typedef struct {

  /* Mandatory */
  /*  Profile Identifier */
  wds_profile_identifier_type_v01 profile;
}wds_delete_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Deletes a configured profile. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_delete_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves a list of configured profiles present on the
           wireless device. */
typedef struct {

  /* Optional */
  /*  Profile Type */
  uint8_t profile_type_valid;  /**< Must be set to true if profile_type is being passed */
  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
      - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n 
      - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_PROFILE_TYPE_EPC (0x02) --  EPC 
 */
}wds_get_profile_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
      - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n 
      - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_PROFILE_TYPE_EPC (0x02) --  EPC 
 */

  uint8_t profile_index;
  /**<   Profile number identifying the profile.*/

  char profile_name[QMI_WDS_PROFILE_NAME_MAX_V01 + 1];
  /**<   One or more bytes describing the profile. The description can
       be a user-defined name for the profile.
  */
}wds_profile_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves a list of configured profiles present on the
           wireless device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Profile list  */
  uint32_t profile_list_len;  /**< Must be set to # of elements in profile_list */
  wds_profile_info_type_v01 profile_list[QMI_WDS_PROFILE_LIST_MAX_V01];

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_get_profile_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the settings from a configured profile */
typedef struct {

  /* Mandatory */
  /*  Profile Identifier */
  wds_profile_identifier_type_v01 profile;
}wds_get_profile_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the settings from a configured profile */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Profile Name ** */
  uint8_t profile_name_valid;  /**< Must be set to true if profile_name is being passed */
  char profile_name[QMI_WDS_PROFILE_NAME_MAX_V01 + 1];
  /**<   One or more bytes describing the profile. The description can
       be a user-defined name for the profile.

       QMI_ERR_ARG_TOO_LONG is returned if the profile_name is too long
   */

  /* Optional */
  /*  PDP Type ** */
  uint8_t pdp_type_valid;  /**< Must be set to true if pdp_type is being passed */
  wds_pdp_type_enum_v01 pdp_type;
  /**<   PDP type specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-PPP \n
       - 2 -- PDP-IPv6 \n
       - 3 -- PDP-IPv4v6
   */

  /* Optional */
  /*  PDP Header Compression Type ** */
  uint8_t pdp_hdr_compression_type_valid;  /**< Must be set to true if pdp_hdr_compression_type is being passed */
  wds_pdp_hdr_compr_type_enum_v01 pdp_hdr_compression_type;
  /**<   Values: \n
       - 0 -- PDP header compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- PDP header compression based on RFC 1144 \n
       - 3 -- PDP header compression based on RFC 2507 \n
       - 4 -- PDP header compression based on RFC 3095
  */

  /* Optional */
  /*  PDP Data Compression Type to Use ** */
  uint8_t pdp_data_compression_type_valid;  /**< Must be set to true if pdp_data_compression_type is being passed */
  wds_pdp_data_compr_type_enum_v01 pdp_data_compression_type;
  /**<   Values: \n
       - 0 -- PDP data compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- V.42BIS data compresion \n
       - 3 -- V.44 data compresion
  */

  /* Optional */
  /*  Context Access Point Node Name ** */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   A string parameter that is a logical name
       used to select the GGSN and external packet data network.

       If the value is NULL or omitted, the subscription default
       value is requested.

       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.
  */

  /* Optional */
  /*  Primary DNS Address Preference ** */
  uint8_t primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if primary_DNS_IPv4_address_preference is being passed */
  uint32_t primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS Address Preference ** */
  uint8_t secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if secondary_DNS_IPv4_address_preference is being passed */
  uint32_t secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  UMTS Requested QoS ** */
  uint8_t umts_requested_qos_valid;  /**< Must be set to true if umts_requested_qos is being passed */
  wds_umts_qos_type_v01 umts_requested_qos;

  /* Optional */
  /*  UMTS Minimum QoS ** */
  uint8_t umts_minimum_qos_valid;  /**< Must be set to true if umts_minimum_qos is being passed */
  wds_umts_qos_type_v01 umts_minimum_qos;

  /* Optional */
  /*  GPRS Requested QoS ** */
  uint8_t gprs_requested_qos_valid;  /**< Must be set to true if gprs_requested_qos is being passed */
  wds_gprs_qos_type_v01 gprs_requested_qos;

  /* Optional */
  /*  GRPS Minimum Qos ** */
  uint8_t gprs_minimum_qos_valid;  /**< Must be set to true if gprs_minimum_qos is being passed */
  wds_gprs_qos_type_v01 gprs_minimum_qos;

  /* Optional */
  /*  Username ** */
  uint8_t username_valid;  /**< Must be set to true if username is being passed */
  char username[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   Username used during data network authentication.

       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Password ** */
  uint8_t password_valid;  /**< Must be set to true if password is being passed */
  char password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password to be used during data network authentication.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Authentication Preference ** */
  uint8_t authentication_preference_valid;  /**< Must be set to true if authentication_preference is being passed */
  wds_auth_pref_mask_v01 authentication_preference;
  /**<   A bit map that indicates the authentication algorithm preference. Values: \n
       Bit 0 -- PAP preference: \n
       - 0 -- PAP is never performed \n
       - 1 -- PAP may be performed \n
       Bit 1 -- CHAP preference: \n
       - 0 -- CHAP is never performed \n
       - 1 -- CHAP may be performed \n
       All other bits are reserved and ignored. They must be set to zero
       by the client.

       If more than one bit is set, the device decides which
       authentication procedure is performed while setting up the data
       session. For example, the device may have a policy to select
       the most secure authentication mechanism.
  */

  /* Optional */
  /*  IPv4 Address Preference ** */
  uint8_t ipv4_address_preference_valid;  /**< Must be set to true if ipv4_address_preference is being passed */
  uint32_t ipv4_address_preference;
  /**<   Preferred IPv4 address assigned to the TE. 
       Actual assigned address is negotiated with the network and
       may differ from this value. If not specified, the IPv4 Address
       is obtained automatically from the network. The assigned value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  PCSCF Address Using PCO Flag ** */
  uint8_t pcscf_addr_using_pco_valid;  /**< Must be set to true if pcscf_addr_using_pco is being passed */
  uint8_t pcscf_addr_using_pco;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default this value is 0.
  */

  /* Optional */
  /*  PDP Access Control Flag ** */
  uint8_t pdp_access_control_flag_valid;  /**< Must be set to true if pdp_access_control_flag is being passed */
  wds_pdp_access_control_enum_v01 pdp_access_control_flag;
  /**<   PDP access control flag. Values: \n
       - 0 -- PDP access control none \n
       - 1 -- PDP access control reject \n
       - 2 -- PDP access control permission
  */

  /* Optional */
  /*  PCSCF Address Using DHCP ** */
  uint8_t pcscf_addr_using_dhcp_valid;  /**< Must be set to true if pcscf_addr_using_dhcp is being passed */
  uint8_t pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using DHCP \n
       - 0 -- FALSE -- Do not request \n
       By default, value is 0.
  */

  /* Optional */
  /*  IM CN flag ** */
  uint8_t im_cn_flag_valid;  /**< Must be set to true if im_cn_flag is being passed */
  uint8_t im_cn_flag;
  /**<   Values: \n
       - 1 -- TRUE -- Request IM CN flag for this profile \n
       - 0 -- FALSE -- Do not request IM CN flag for this profile
  */

  /* Optional */
  /*  Traffic Flow Template (TFT) ID1 Parameters ** */
  uint8_t tft_id1_params_valid;  /**< Must be set to true if tft_id1_params is being passed */
  wds_tft_id_param_type_v01 tft_id1_params;

  /* Optional */
  /*  TFT ID2 Parameters ** */
  uint8_t tft_id2_params_valid;  /**< Must be set to true if tft_id2_params is being passed */
  wds_tft_id_param_type_v01 tft_id2_params;

  /* Optional */
  /*  PDP Context Number ** */
  uint8_t pdp_context_valid;  /**< Must be set to true if pdp_context is being passed */
  uint8_t pdp_context;
  /**<   PDP context number.*/

  /* Optional */
  /*  PDP Context Secondary Flag ** */
  uint8_t secondary_flag_valid;  /**< Must be set to true if secondary_flag is being passed */
  uint8_t secondary_flag;
  /**<   Values: \n
       - 1 -- TRUE -- This is secondary profile \n
       - 0 -- FALSE -- This is not secondary profile
  */

  /* Optional */
  /*  PDP Context Primary ID ** */
  uint8_t primary_id_valid;  /**< Must be set to true if primary_id is being passed */
  uint8_t primary_id;
  /**<   PDP context number primary ID.*/

  /* Optional */
  /*  IPv6 Address Preference ** */
  uint8_t ipv6_address_preference_valid;  /**< Must be set to true if ipv6_address_preference is being passed */
  uint8_t ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Preferred IPv6 address to be assigned to the TE; actual assigned
       address is negotiated with the network and may differ from this value;
       if not specified, the IPv6 address is obtained automatically from the
       network.
  */

  /* Optional */
  /*  UMTS Requested QoS with Signaling Indication Flag ** */
  uint8_t umts_requested_qos_with_sig_ind_valid;  /**< Must be set to true if umts_requested_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_requested_qos_with_sig_ind;

  /* Optional */
  /*  UMTS Minimum QoS with Signaling Indication ** */
  uint8_t umts_minimum_qos_with_sig_ind_valid;  /**< Must be set to true if umts_minimum_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_minimum_qos_with_sig_ind;

  /* Optional */
  /*  Primary DNS IPv6 Address Preference ** */
  uint8_t primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if primary_dns_ipv6_address_preference is being passed */
  uint8_t primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Secondary DNS IPv6 Address Preference ** */
  uint8_t secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if secodnary_dns_ipv6_address_preference is being passed */
  uint8_t secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  DHCP/NAS Preference ** */
  uint8_t addr_allocation_preference_valid;  /**< Must be set to true if addr_allocation_preference is being passed */
  wds_addr_allocation_preference_enum_v01 addr_allocation_preference;
  /**<   Used to indicate the address allocation
       preference. Values: \n
         - 0 -- NAS signaling is used for address allocation \n
         - 1 -- DHCP is used for address allocation
  */

  /* Optional */
  /*  3GPP LTE QoS Parameters ** */
  uint8_t threegpp_lte_qos_params_valid;  /**< Must be set to true if threegpp_lte_qos_params is being passed */
  wds_3gpp_lte_qos_params_v01 threegpp_lte_qos_params;

  /* Optional */
  /*  APN Disabled Flag ** */
  uint8_t apn_disabled_flag_valid;  /**< Must be set to true if apn_disabled_flag is being passed */
  uint8_t apn_disabled_flag;
  /**<   If this flag is set, the use of this profile for 
       making data calls is disabled. Any data call with 
       this profile fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE        
  */

  /* Optional */
  /*  PDN Inactivity Timeout ** */
  uint8_t pdn_inactivity_timeout_valid;  /**< Must be set to true if pdn_inactivity_timeout is being passed */
  uint32_t pdn_inactivity_timeout;
  /**<   Duration of inactivity timer in seconds. If a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, PDP context/PDN
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class ** */
  uint8_t apn_class_valid;  /**< Must be set to true if apn_class is being passed */
  uint8_t apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. The APN class can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  APN Bearer ** */
  uint8_t apn_bearer_valid;  /**< Must be set to true if apn_bearer is being passed */
  wds_apn_bearer_mask_v01 apn_bearer;
  /**<   APN bearer mask. Specifies whether a data call is allowed on
       specific RAT types. Values: \n
       - 0x0000000000000001 -- GSM \n
       - 0x0000000000000002 -- WCDMA \n
       - 0x0000000000000004 -- LTE \n
       - 0x8000000000000000 -- Any
  */

  /* Optional */
  /*  Support Emergency Calls ** */
  uint8_t support_emergency_calls_valid;  /**< Must be set to true if support_emergency_calls is being passed */
  uint8_t support_emergency_calls;
  /**<    When this flag is set, the user can make emergency calls 
        using this profile. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE       
  */

  /* Optional */
  /*  Operator Reserved PCO ID ** */
  uint8_t op_pco_id_valid;  /**< Must be set to true if op_pco_id is being passed */
  uint16_t op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Mobile Country Code ** */
  uint8_t pco_mcc_valid;  /**< Must be set to true if pco_mcc is being passed */
  uint16_t pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code ** */
  uint8_t pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Max PDN Connections Per Time Block   ** */
  uint8_t max_pdn_conn_per_block_valid;  /**< Must be set to true if max_pdn_conn_per_block is being passed */
  uint16_t max_pdn_conn_per_block;
  /**<   Specifies the maximum number of PDN connections that the UE
    is allowed to perform with the network in a specified time block. 
    The time block size is defined by a configuration item.
    The default value is 1023. \n
    Range: 0 to 1023.
  */

  /* Optional */
  /*  Max PDN Connections Timer   ** */
  uint8_t max_pdn_conn_timer_valid;  /**< Must be set to true if max_pdn_conn_timer is being passed */
  uint16_t max_pdn_conn_timer;
  /**<   Specifies the time duration in seconds during which the UE 
   counts the PDN connections already made. 
   The default value is 300. \n
   Range: 0 to 3600 seconds.
  */

  /* Optional */
  /*  PDN Request Wait Timer  ** */
  uint8_t pdn_req_wait_interval_valid;  /**< Must be set to true if pdn_req_wait_interval is being passed */
  uint16_t pdn_req_wait_interval;
  /**<   Specifies the minimum time interval between the new PDN 
   connection request and the last successful UE initiated PDN 
   disconnection.
   The default value is 0. \n
   Range: 0 to 1023 sec.
  */

  /* Optional */
  /*  3GPP Application User Data  ** */
  uint8_t app_user_data_3gpp_valid;  /**< Must be set to true if app_user_data_3gpp is being passed */
  uint32_t app_user_data_3gpp;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Roaming Disallow Flag ** */
  uint8_t roaming_disallowed_valid;  /**< Must be set to true if roaming_disallowed is being passed */
  uint8_t roaming_disallowed;
  /**<   Specifies whether the UE is allowed to connect 
       to the APN specified by the profile while roaming.
  */

  /* Optional */
  /*  PDN Disconnect Wait Timer ** */
  uint8_t pdn_discon_wait_timer_valid;  /**< Must be set to true if pdn_discon_wait_timer is being passed */
  uint8_t pdn_discon_wait_timer;
  /**<    Indicates the delay that the control point expects 
        to be available for successful deregistration with the network 
        before the modem disconnects the PDN(s). When the default value of 
        zero is specified, the modem disconnects the PDN immediately 
        upon moving to the roaming network, without waiting for the 
        control point.
        Range: 0-255 minutes.  
  */

  /* Optional */
  /*  DNS Address Using DHCP ** */
  uint8_t dns_addr_using_dhcp_valid;  /**< Must be set to true if dns_addr_using_dhcp is being passed */
  uint8_t dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common PCSCF Address Using DHCP ** * */
  uint8_t common_pcscf_addr_using_dhcp_valid;  /**< Must be set to true if common_pcscf_addr_using_dhcp is being passed */
  uint8_t common_pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common DNS Address Using DHCP ** * */
  uint8_t common_dns_addr_using_dhcp_valid;  /**< Must be set to true if common_dns_addr_using_dhcp is being passed */
  uint8_t common_dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)      
  */

  /* Optional */
  /*  Common PDP Type ** * */
  uint8_t common_pdp_type_valid;  /**< Must be set to true if common_pdp_type is being passed */
  wds_common_pdp_type_enum_v01 common_pdp_type;
  /**<   Specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-IPv6 \n
       - 2 -- PDP-IPv4v6
   */

  /* Optional */
  /*  Common Application User Data  ** */
  uint8_t common_app_user_data_valid;  /**< Must be set to true if common_app_user_data is being passed */
  uint32_t common_app_user_data;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Common Mobile Network Code *** */
  uint8_t common_pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if common_pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 common_pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Common Mobile Country Code *** */
  uint8_t common_pco_mcc_valid;  /**< Must be set to true if common_pco_mcc is being passed */
  uint16_t common_pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Common Operator Reserved PCO ID ** */
  uint8_t common_op_pco_id_valid;  /**< Must be set to true if common_op_pco_id is being passed */
  uint16_t common_op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Common Authentication Password *** */
  uint8_t common_auth_password_valid;  /**< Must be set to true if common_auth_password is being passed */
  char common_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Common User ID *** */
  uint8_t common_user_id_valid;  /**< Must be set to true if common_user_id is being passed */
  char common_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
    */

  /* Optional */
  /*  Common Authentication Protocol *** */
  uint8_t common_auth_protocol_valid;  /**< Must be set to true if common_auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 common_auth_protocol;
  /**<   Values: \n
       - 0 -- None \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  Common PCSCF Address Using PCO Flag *** */
  uint8_t common_is_pcscf_address_needed_valid;  /**< Must be set to true if common_is_pcscf_address_needed is being passed */
  uint8_t common_is_pcscf_address_needed;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default the value is 0.
  */

  /* Optional */
  /*  Common Allow/Disallow Lingering of Interface *** */
  uint8_t linger_params_valid;  /**< Must be set to true if linger_params is being passed */
  wds_common_linger_param_type_v01 linger_params;

  /* Optional */
  /*  Common Secondary DNS IPv6 Address Preference *** */
  uint8_t common_secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_secodnary_dns_ipv6_address_preference is being passed */
  uint8_t common_secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS IPv6 Address Preference *** */
  uint8_t common_primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_primary_dns_ipv6_address_preference is being passed */
  uint8_t common_primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Secondary DNS IPv4 Address Preference *** */
  uint8_t common_secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_secondary_DNS_IPv4_address_preference is being passed */
  uint32_t common_secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS Address Preference *** */
  uint8_t common_primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_primary_DNS_IPv4_address_preference is being passed */
  uint32_t common_primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the network;
       if not specified, the wireless device attempts to obtain the DNS
       address automatically from the network. The negotiated value is provided
       to the host via the DHCP.
  */

  /* Optional */
  /*  Common APN Class *** */
  uint8_t common_apn_class_valid;  /**< Must be set to true if common_apn_class is being passed */
  uint8_t common_apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. The APN class can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  Common APN Disabled Flag *** */
  uint8_t common_apn_disabled_flag_valid;  /**< Must be set to true if common_apn_disabled_flag is being passed */
  uint8_t common_apn_disabled_flag;
  /**<   Setting this flag disables the use of this profile for 
       making data calls. Any data call with this profile 
       fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE 
   */

  /* Optional */
  /*  Negotiate DNS Server Preference * */
  uint8_t negotiate_dns_server_preference_valid;  /**< Must be set to true if negotiate_dns_server_preference is being passed */
  uint8_t negotiate_dns_server_preference;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address from the PDSN \n
       - 0 -- FALSE -- Do not request DNS address from the PDSN \n
       Note: Default value is 1 (TRUE).
  */

  /* Optional */
  /*  PPP Session Close Timer for DO * */
  uint8_t ppp_session_close_timer_DO_valid;  /**< Must be set to true if ppp_session_close_timer_DO is being passed */
  uint32_t ppp_session_close_timer_DO;
  /**<   Timer value (in seconds) on DO indicating how long
       the PPP session lingers before closing down.
  */

  /* Optional */
  /*  PPP Session Close Timer for 1X * */
  uint8_t ppp_session_close_timer_1x_valid;  /**< Must be set to true if ppp_session_close_timer_1x is being passed */
  uint32_t ppp_session_close_timer_1x;
  /**<   Timer value (in seconds) on 1X indicating how
       long the PPP session lingers before
       closing down.
  */

  /* Optional */
  /*  Allow/Disallow Lingering of Interface * */
  uint8_t allow_linger_valid;  /**< Must be set to true if allow_linger is being passed */
  uint8_t allow_linger;
  /**<   Values: \n
       - 1 -- TRUE -- Allow lingering \n
       - 0 -- FALSE -- Do not allow lingering
  */

  /* Optional */
  /*  LCP ACK Timeout * */
  uint8_t lcp_ack_timeout_valid;  /**< Must be set to true if lcp_ack_timeout is being passed */
  uint16_t lcp_ack_timeout;
  /**<   Value of LCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  IPCP ACK Timeout * */
  uint8_t ipcp_ack_timeout_valid;  /**< Must be set to true if ipcp_ack_timeout is being passed */
  uint16_t ipcp_ack_timeout;
  /**<   Value of IPCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  AUTH Timeout * */
  uint8_t auth_timeout_valid;  /**< Must be set to true if auth_timeout is being passed */
  uint16_t auth_timeout;
  /**<   Value of authentication timeout in milliseconds.
  */

  /* Optional */
  /*  LCP Configuration Request Retry Count Value * */
  uint8_t lcp_creq_retry_count_valid;  /**< Must be set to true if lcp_creq_retry_count is being passed */
  uint8_t lcp_creq_retry_count;
  /**<   LCP configuration request retry count value.
  */

  /* Optional */
  /*  IPCP Configuration Request Retry Count * */
  uint8_t ipcp_creq_retry_count_valid;  /**< Must be set to true if ipcp_creq_retry_count is being passed */
  uint8_t ipcp_creq_retry_count;
  /**<   IPCP configuration request retry count value.
  */

  /* Optional */
  /*  Authentication Retry * */
  uint8_t auth_retry_count_valid;  /**< Must be set to true if auth_retry_count is being passed */
  uint8_t auth_retry_count;
  /**<   Authentication retry count value.
  */

  /* Optional */
  /*  Authentication Protocol * */
  uint8_t auth_protocol_valid;  /**< Must be set to true if auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 auth_protocol;
  /**<   Values: \n
       - 0 -- NONE \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  User ID * */
  uint8_t user_id_valid;  /**< Must be set to true if user_id is being passed */
  char user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network
       authentication; maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the storage on the wireless device is insufficient
       in size to hold the value.
    */

  /* Optional */
  /*  Authentication Password * */
  uint8_t auth_password_valid;  /**< Must be set to true if auth_password is being passed */
  char auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network
       authentication; maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the storage on the wireless device is insufficient
       in size to hold the value.
  */

  /* Optional */
  /*  Data Rate * */
  uint8_t data_rate_valid;  /**< Must be set to true if data_rate is being passed */
  wds_profile_data_rate_enum_v01 data_rate;
  /**<   Values: \n
       - 0 -- Low (Low speed service options (SO15) only) \n
       - 1 -- Medium (SO33 + low R-SCH) \n
       - 2 -- High (SO33 + high R-SCH) \n
      Note: Default is 2.
  */

  /* Optional */
  /*  Application Type * */
  uint8_t app_type_valid;  /**< Must be set to true if app_type is being passed */
  wds_profile_app_type_enum_v01 app_type;
  /**<   Values: \n
       - 0x00000001 -- Default application type \n
       - 0x00000020 -- LBS application type \n
       - 0x00000040 -- Tethered application type \n
       Note: The application type value in a profile cannot
       be modified. It can only be used to search for the
       profile ID numbers that have the specified
       application type. 
   */

  /* Optional */
  /*  Data Mode * */
  uint8_t data_mode_valid;  /**< Must be set to true if data_mode is being passed */
  wds_profile_data_mode_enum_v01 data_mode;
  /**<   Values: \n
       - 0 -- CDMA or HDR (Hybrid 1X/1xEV-DO) \n
       - 1 -- CDMA only (1X only) \n
       - 2 -- HDR only (1xEV-DO only) \n
       Note: Default is 0.
  */

  /* Optional */
  /*  Application Priority * */
  uint8_t app_priority_valid;  /**< Must be set to true if app_priority is being passed */
  uint8_t app_priority;
  /**<   Numerical one byte value defining the application
       priority; higher value means higher priority. \n
       Note: Application priority value in a profile
       cannot be modified. It is listed for future
       extensibility of profile ID search based on
       application priority. 
  */

  /* Optional */
  /*  APN String * */
  uint8_t apn_string_valid;  /**< Must be set to true if apn_string is being passed */
  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String representing the access point name;
       maximum length allowed is 100 bytes.
       QMI_ERR_ARG_TOO_LONG is returned when
       the APN name is too long.
  */

  /* Optional */
  /*  PDN Type * */
  uint8_t pdn_type_valid;  /**< Must be set to true if pdn_type is being passed */
  wds_profile_pdn_type_enum_v01 pdn_type;
  /**<   Values: \n
       - 0 -- IPv4 PDN type \n
       - 1 -- IPv6 PDN type \n
       - 2 -- IPv4 or IPv6 PDN type \n
       - 3 -- Unspecified PDN type (implying no preference)
   */

  /* Optional */
  /*  Is PCSCF Address Needed * */
  uint8_t is_pcscf_address_needed_valid;  /**< Must be set to true if is_pcscf_address_needed is being passed */
  uint8_t is_pcscf_address_needed;
  /**<   Used to control whether the PCSCF
       address is requested from PDSN. Values: \n
       - 1 -- TRUE -- Request PCSCF value from the PDSN \n
       - 0 -- FALSE -- Do not request PCSCF value from the PDSN
  */

  /* Optional */
  /*  IPv4 Primary DNS Address * */
  uint8_t primary_v4_dns_address_valid;  /**< Must be set to true if primary_v4_dns_address is being passed */
  uint32_t primary_v4_dns_address;
  /**<   The primary IPv4 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  IPv4 Secondary DNS Address * */
  uint8_t secondary_v4_dns_address_valid;  /**< Must be set to true if secondary_v4_dns_address is being passed */
  uint32_t secondary_v4_dns_address;
  /**<   The secondary IPv4 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  Primary IPv6 DNS Address * */
  uint8_t primary_v6_dns_address_valid;  /**< Must be set to true if primary_v6_dns_address is being passed */
  uint8_t primary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The primary IPv6 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  Secondary IPv6 DNS Address * */
  uint8_t secondary_v6_dns_address_valid;  /**< Must be set to true if secondary_v6_dns_address is being passed */
  uint8_t secondary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The secondary IPv6 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  RAT Type * */
  uint8_t rat_type_valid;  /**< Must be set to true if rat_type is being passed */
  wds_rat_type_enum_v01 rat_type;
  /**<   Values: \n
       - 1 -- HRPD \n
       - 2 -- EHRPD \n
       - 3 -- HRPD_EHRPD
  */

  /* Optional */
  /*  APN Enabled * */
  uint8_t apn_enabled_3gpp2_valid;  /**< Must be set to true if apn_enabled_3gpp2 is being passed */
  uint8_t apn_enabled_3gpp2;
  /**<    APN enabled is a flag to specify whether the APN in 
       that profile is enabled or disabled. If the APN is 
       disabled, the data call cannot be established 
       using that APN. Values: \n
       - 1 -- Enabled (default value) \n
       - 0 -- Disabled       
  */

  /* Optional */
  /*  PDN Inactivity Timeout * */
  uint8_t pdn_inactivity_timeout_3gpp2_valid;  /**< Must be set to true if pdn_inactivity_timeout_3gpp2 is being passed */
  uint32_t pdn_inactivity_timeout_3gpp2;
  /**<   Duration of inactivity timer in minutes. If a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, the PDP context/PDN 
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class * */
  uint8_t apn_class_3gpp2_valid;  /**< Must be set to true if apn_class_3gpp2 is being passed */
  uint8_t apn_class_3gpp2;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. This can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  PDN Level Auth Protocol * */
  uint8_t pdn_level_auth_protocol_valid;  /**< Must be set to true if pdn_level_auth_protocol is being passed */
  wds_profile_pdn_lvl_auth_proto_enum_v01 pdn_level_auth_protocol;
  /**<   Authentication protocol used during PDN level authentication.
 Values: \n
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_NONE (0) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP (1) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP (2) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_CHAP (3) -- 
 */

  /* Optional */
  /*  PDN Level User ID * */
  uint8_t pdn_level_user_id_valid;  /**< Must be set to true if pdn_level_user_id is being passed */
  char pdn_level_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Level Auth Password * */
  uint8_t pdn_level_auth_password_valid;  /**< Must be set to true if pdn_level_auth_password is being passed */
  char pdn_level_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Label * */
  uint8_t pdn_label_valid;  /**< Must be set to true if pdn_label is being passed */
  char pdn_label[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   Logical name used to map the APN name for selecting the 
       packet data network. Maximum length allowed is 100 bytes.
  */

  /* Optional */
  /*  Operator Reserved PCO ID * */
  uint8_t op_pco_id_3gpp2_valid;  /**< Must be set to true if op_pco_id_3gpp2 is being passed */
  uint16_t op_pco_id_3gpp2;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
   */

  /* Optional */
  /*  Mobile Country Code * */
  uint8_t pco_mcc_3gpp2_valid;  /**< Must be set to true if pco_mcc_3gpp2 is being passed */
  uint16_t pco_mcc_3gpp2;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code * */
  uint8_t pco_mnc_includes_pcs_digit_3gpp2_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit_3gpp2 is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit_3gpp2;

  /* Optional */
  /*  PDN Throttling Timer 1-6 * */
  uint8_t failure_timer_valid;  /**< Must be set to true if failure_timer is being passed */
  uint32_t failure_timer[QMI_WDS_MAX_FAILURE_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after a PDN connection or 
  IP address assignment failure. For example, immediately following a third 
  consecutive PDN connection request failure, the UE waits failure_timer[2] 
  seconds before sending the fourth request. Following failures of six or greater,
  failure_timer[5] is used.
  */

  /* Optional */
  /*  PDN Disallow Timer 1-6 * */
  uint8_t disallow_timer_valid;  /**< Must be set to true if disallow_timer is being passed */
  uint32_t disallow_timer[QMI_WDS_MAX_DISALLOW_TIMER_V01];
  /**<   The back-off time, in seconds, to be used after the network refuses 
  to grant the requested IP address type, such as when an IPv6 address is requested 
  from a network that only grants the IPv4 address. For example, immediately 
  after a third consecutive PDN connection request is denied, the UE waits 
  disallow_timer[2] seconds before sending the fourth request. 
  Following failures of six or greater, disallow_timer[5] is used.
  */

  /* Optional */
  /*  3GPP2 Application User Data  * */
  uint8_t app_user_data_3gpp2_valid;  /**< Must be set to true if app_user_data_3gpp2 is being passed */
  uint32_t app_user_data_3gpp2;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  PCSCF Address Using DHCP 3GPP2 * */
  uint8_t pcscf_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if pcscf_addr_using_dhcp_3gpp2 is being passed */
  uint8_t pcscf_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  DNS Address Using DHCP * */
  uint8_t dns_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if dns_addr_using_dhcp_3gpp2 is being passed */
  uint8_t dns_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  IPv6 Prefix Delegation Flag * ** */
  uint8_t ipv6_prefix_delegation_valid;  /**< Must be set to true if ipv6_prefix_delegation is being passed */
  uint8_t ipv6_prefix_delegation;
  /**<   Enables IPv6 prefix delegation.
     Values: \n
     - 0 -- FALSE (default) \n
     - 1 -- TRUE
  */

  /* Optional */
  /*  Profile Extended Error Code * */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   The extended error code received from the DS profile subsystem.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_get_profile_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the default data session settings. */
typedef struct {

  /* Mandatory */
  /*  Profile Type */
  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
      - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n 
      - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_PROFILE_TYPE_EPC (0x02) --  EPC 
 */
}wds_get_default_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the default data session settings. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Profile Name */
  uint8_t profile_name_valid;  /**< Must be set to true if profile_name is being passed */
  char profile_name[QMI_WDS_PROFILE_NAME_MAX_V01 + 1];
  /**<   One or more bytes describing the profile. The description is
       a user-defined name for the profile.
       QMI_ERR_ARG_TOO_LONG is returned if the profile_name is too long.
   */

  /* Optional */
  /*  PDP Type */
  uint8_t pdp_type_valid;  /**< Must be set to true if pdp_type is being passed */
  wds_pdp_type_enum_v01 pdp_type;
  /**<   Packet Data Protocol (PDP) type specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-PPP \n
       - 2 -- PDP-IPv6 \n
       - 3 -- PDP-IPv4v6
   */

  /* Optional */
  /*  PDP Header Compression Type */
  uint8_t pdp_hdr_compression_type_valid;  /**< Must be set to true if pdp_hdr_compression_type is being passed */
  wds_pdp_hdr_compr_type_enum_v01 pdp_hdr_compression_type;
  /**<   Values: \n
       - 0 -- PDP header compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- PDP header compression based on RFC 1144 \n
       - 3 -- PDP header compression based on RFC 2507 \n
       - 4 -- PDP header compression based on RFC 3095
  */

  /* Optional */
  /*  PDP Data Compression Type  */
  uint8_t pdp_data_compression_type_valid;  /**< Must be set to true if pdp_data_compression_type is being passed */
  wds_pdp_data_compr_type_enum_v01 pdp_data_compression_type;
  /**<   Values: \n
       - 0 -- PDP data compression is off \n
       - 1 -- Manufacturer preferred compression \n
       - 2 -- V.42BIS data compresion \n
       - 3 -- V.44 data compresion
  */

  /* Optional */
  /*  Context Access Point Node (APN) Name */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   A string parameter that is a logical name
       used to select the GGSN and external packet data network.

       If the value is NULL or omitted, the subscription default
       value is requested.

       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.
  */

  /* Optional */
  /*  Primary DNS Address Preference */
  uint8_t primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if primary_DNS_IPv4_address_preference is being passed */
  uint32_t primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS Address Preference */
  uint8_t secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if secondary_DNS_IPv4_address_preference is being passed */
  uint32_t secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  UMTS Requested QoS */
  uint8_t umts_requested_qos_valid;  /**< Must be set to true if umts_requested_qos is being passed */
  wds_umts_qos_type_v01 umts_requested_qos;

  /* Optional */
  /*  UMTS Minimum QoS */
  uint8_t umts_minimum_qos_valid;  /**< Must be set to true if umts_minimum_qos is being passed */
  wds_umts_qos_type_v01 umts_minimum_qos;

  /* Optional */
  /*  GPRS Requested QoS */
  uint8_t gprs_requested_qos_valid;  /**< Must be set to true if gprs_requested_qos is being passed */
  wds_gprs_qos_type_v01 gprs_requested_qos;

  /* Optional */
  /*  GRPS Minimum Qos */
  uint8_t gprs_minimum_qos_valid;  /**< Must be set to true if gprs_minimum_qos is being passed */
  wds_gprs_qos_type_v01 gprs_minimum_qos;

  /* Optional */
  /*  Username */
  uint8_t username_valid;  /**< Must be set to true if username is being passed */
  char username[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   Username used during data network authentication.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Password */
  uint8_t password_valid;  /**< Must be set to true if password is being passed */
  char password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Authentication Preference */
  uint8_t authentication_preference_valid;  /**< Must be set to true if authentication_preference is being passed */
  wds_auth_pref_mask_v01 authentication_preference;
  /**<   A bit map that indicates the authentication algorithm preference. Values: \n
       Bit 0 -- PAP preference: \n
       - 0 -- PAP is never performed \n
       - 1 -- PAP may be performed \n
       Bit 1 -- CHAP preference: \n
       - 0 -- CHAP is never performed \n
       - 1 -- CHAP may be performed \n
       All other bits are reserved and are ignored. They must be set to zero
       by the client.

       If more than one bit is set, the device decides which
       authentication procedure is performed while setting up the data
       session. For example, the device may have a policy to select
       the most secure authentication mechanism.
  */

  /* Optional */
  /*  IPv4 Address Preference */
  uint8_t ipv4_address_preference_valid;  /**< Must be set to true if ipv4_address_preference is being passed */
  uint32_t ipv4_address_preference;
  /**<   Preferred IPv4 address assigned to the TE -- 
       The actual assigned address is negotiated with the network and
       may differ from this value. If not specified, the IPv4 Address
       is obtained automatically from the network. The assigned value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  PCSCF Address Using PCO Flag */
  uint8_t pcscf_addr_using_pco_valid;  /**< Must be set to true if pcscf_addr_using_pco is being passed */
  uint8_t pcscf_addr_using_pco;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default this value is 0.
  */

  /* Optional */
  /*  PDP Access Control Flag */
  uint8_t pdp_access_control_flag_valid;  /**< Must be set to true if pdp_access_control_flag is being passed */
  wds_pdp_access_control_enum_v01 pdp_access_control_flag;
  /**<   Values: \n
       - 0 -- PDP access control none \n
       - 1 -- PDP access control reject \n
       - 2 -- PDP access control permission
  */

  /* Optional */
  /*  PCSCF Address Using DHCP */
  uint8_t pcscf_addr_using_dhcp_valid;  /**< Must be set to true if pcscf_addr_using_dhcp is being passed */
  uint8_t pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using DHCP \n
       - 0 -- FALSE -- Do not request \n
       By default, value is 0.
  */

  /* Optional */
  /*  IM CN Flag */
  uint8_t im_cn_flag_valid;  /**< Must be set to true if im_cn_flag is being passed */
  uint8_t im_cn_flag;
  /**<   Values: \n
       - 1 -- TRUE -- Request IM CN flag for this profile \n
       - 0 -- FALSE -- Do not request IM CN flag for this profile
  */

  /* Optional */
  /*  Traffic Flow Template (TFT) ID1 Parameters */
  uint8_t tft_id1_params_valid;  /**< Must be set to true if tft_id1_params is being passed */
  wds_tft_id_param_type_v01 tft_id1_params;

  /* Optional */
  /*  TFT ID2 Parameters */
  uint8_t tft_id2_params_valid;  /**< Must be set to true if tft_id2_params is being passed */
  wds_tft_id_param_type_v01 tft_id2_params;

  /* Optional */
  /*  PDP Context Number */
  uint8_t pdp_context_valid;  /**< Must be set to true if pdp_context is being passed */
  uint8_t pdp_context;
  /**<   PDP context number.*/

  /* Optional */
  /*  PDP Context Secondary Flag */
  uint8_t secondary_flag_valid;  /**< Must be set to true if secondary_flag is being passed */
  uint8_t secondary_flag;
  /**<   Values: \n
       - 1 -- TRUE -- This is the secondary profile \n
       - 0 -- FALSE -- This is not the secondary profile
  */

  /* Optional */
  /*  PDP Context Primary ID */
  uint8_t primary_id_valid;  /**< Must be set to true if primary_id is being passed */
  uint8_t primary_id;
  /**<   PDP context number primary ID.*/

  /* Optional */
  /*  IPv6 Address Preference */
  uint8_t ipv6_address_preference_valid;  /**< Must be set to true if ipv6_address_preference is being passed */
  uint8_t ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The preferred IPv6 address to be assigned to the TE; the actual assigned
       address is negotiated with the network and may differ from this value.
       If not specified, the IPv6 address is obtained automatically from the
       network.
  */

  /* Optional */
  /*  UMTS Requested QoS With Signaling Indication Flag */
  uint8_t umts_requested_qos_with_sig_ind_valid;  /**< Must be set to true if umts_requested_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_requested_qos_with_sig_ind;

  /* Optional */
  /*  UMTS Minimum QoS With Signaling Indication */
  uint8_t umts_minimum_qos_with_sig_ind_valid;  /**< Must be set to true if umts_minimum_qos_with_sig_ind is being passed */
  wds_umts_qos_with_sig_ind_type_v01 umts_minimum_qos_with_sig_ind;

  /* Optional */
  /*  Primary DNS IPv6 Address Preference */
  uint8_t primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if primary_dns_ipv6_address_preference is being passed */
  uint8_t primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS IPv6 Address Preference */
  uint8_t secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if secodnary_dns_ipv6_address_preference is being passed */
  uint8_t secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  DHCP/NAS Preference */
  uint8_t addr_allocation_preference_valid;  /**< Must be set to true if addr_allocation_preference is being passed */
  wds_addr_allocation_preference_enum_v01 addr_allocation_preference;
  /**<   Used to indicate the address allocation
       preference. Values: \n
         - 0 -- NAS signaling is used for address allocation \n
         - 1 -- DHCP is used for address allocation
  */

  /* Optional */
  /*  3GPP LTE QoS Parameters */
  uint8_t threegpp_lte_qos_params_valid;  /**< Must be set to true if threegpp_lte_qos_params is being passed */
  wds_3gpp_lte_qos_params_v01 threegpp_lte_qos_params;

  /* Optional */
  /*  APN Disabled Flag */
  uint8_t apn_disabled_flag_valid;  /**< Must be set to true if apn_disabled_flag is being passed */
  uint8_t apn_disabled_flag;
  /**<    When this flag is set, the use of this profile for 
       making data calls is disabled. Any data call with 
       this profile fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE       
  */

  /* Optional */
  /*  PDN Inactivity Timeout */
  uint8_t pdn_inactivity_timeout_valid;  /**< Must be set to true if pdn_inactivity_timeout is being passed */
  uint32_t pdn_inactivity_timeout;
  /**<   Duration of the inactivity timer in seconds. When a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, PDP context/PDN
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class */
  uint8_t apn_class_valid;  /**< Must be set to true if apn_class is being passed */
  uint8_t apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. This can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  APN Bearer ** */
  uint8_t apn_bearer_valid;  /**< Must be set to true if apn_bearer is being passed */
  wds_apn_bearer_mask_v01 apn_bearer;
  /**<   APN bearer mask. Specifies whether a data call is allowed on
       specific RAT types. Values: \n
       - 0x0000000000000001 -- GSM \n
       - 0x0000000000000002 -- WCDMA \n
       - 0x0000000000000004 -- LTE \n
       - 0x8000000000000000 -- Any
  */

  /* Optional */
  /*  Support Emergency Calls ** */
  uint8_t support_emergency_calls_valid;  /**< Must be set to true if support_emergency_calls is being passed */
  uint8_t support_emergency_calls;
  /**<    When this flag is set, the user can make emergency calls 
        using this profile. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE       
  */

  /* Optional */
  /*  Operator Reserved PCO ID ** */
  uint8_t op_pco_id_valid;  /**< Must be set to true if op_pco_id is being passed */
  uint16_t op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
   */

  /* Optional */
  /*  Mobile Country Code ** */
  uint8_t pco_mcc_valid;  /**< Must be set to true if pco_mcc is being passed */
  uint16_t pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code ** */
  uint8_t pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Max PDN Connections Per Time Block   ** */
  uint8_t max_pdn_conn_per_block_valid;  /**< Must be set to true if max_pdn_conn_per_block is being passed */
  uint16_t max_pdn_conn_per_block;
  /**<   Specifies the maximum number of PDN connections that the UE
    is allowed to perform with the network in a specified time block. 
    The time block size is defined by a configuration item.
    The default value is 1023. \n
    Range: 0 to 1023.
  */

  /* Optional */
  /*  Max PDN Connections Timer   ** */
  uint8_t max_pdn_conn_timer_valid;  /**< Must be set to true if max_pdn_conn_timer is being passed */
  uint16_t max_pdn_conn_timer;
  /**<   Specifies the time duration (in seconds) during which the UE 
   counts the PDN connections already made. 
   The default value is 300. \n
   Range: 0 to 3600 sec.
  */

  /* Optional */
  /*  PDN Request Wait Timer  ** */
  uint8_t pdn_req_wait_interval_valid;  /**< Must be set to true if pdn_req_wait_interval is being passed */
  uint16_t pdn_req_wait_interval;
  /**<   Specifies the minimum time interval (in seconds) between the new PDN 
   connection request and the last successful UE initiated PDN 
   disconnection.
   The default value is 0. \n
   Range: 0 to 1023 sec.
  */

  /* Optional */
  /*  3GPP Application User Data  ** */
  uint8_t app_user_data_3gpp_valid;  /**< Must be set to true if app_user_data_3gpp is being passed */
  uint32_t app_user_data_3gpp;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Roaming Disallow Flag ** */
  uint8_t roaming_disallowed_valid;  /**< Must be set to true if roaming_disallowed is being passed */
  uint8_t roaming_disallowed;
  /**<   Specifies whether the UE is allowed to connect 
       to the APN specified by the profile while roaming.
  */

  /* Optional */
  /*  PDN Disconnect Wait Timer ** */
  uint8_t pdn_discon_wait_timer_valid;  /**< Must be set to true if pdn_discon_wait_timer is being passed */
  uint8_t pdn_discon_wait_timer;
  /**<    Indicates the delay that the control point expects 
        to be available for successful deregistration with the network 
        before the modem disconnects the PDN(s). When the default value of 
        zero is specified, the modem disconnects the PDN immediately 
        upon moving to the roaming network, without waiting for the 
        control point.
        Range: 0-255 minutes.  
  */

  /* Optional */
  /*  DNS Address Using DHCP ** */
  uint8_t dns_addr_using_dhcp_valid;  /**< Must be set to true if dns_addr_using_dhcp is being passed */
  uint8_t dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)      
  */

  /* Optional */
  /*  Common PCSCF Address Using DHCP ** * */
  uint8_t common_pcscf_addr_using_dhcp_valid;  /**< Must be set to true if common_pcscf_addr_using_dhcp is being passed */
  uint8_t common_pcscf_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common DNS Address Using DHCP ** * */
  uint8_t common_dns_addr_using_dhcp_valid;  /**< Must be set to true if common_dns_addr_using_dhcp is being passed */
  uint8_t common_dns_addr_using_dhcp;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  Common PDP Type ** * */
  uint8_t common_pdp_type_valid;  /**< Must be set to true if common_pdp_type is being passed */
  wds_common_pdp_type_enum_v01 common_pdp_type;
  /**<   Specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
       - 0 -- PDP-IP (IPv4) \n
       - 1 -- PDP-IPv6 \n
       - 2 -- PDP-IPv4v6
   */

  /* Optional */
  /*  Common Application User Data  ** */
  uint8_t common_app_user_data_valid;  /**< Must be set to true if common_app_user_data is being passed */
  uint32_t common_app_user_data;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  Common Mobile Network Code *** */
  uint8_t common_pco_mnc_includes_pcs_digit_valid;  /**< Must be set to true if common_pco_mnc_includes_pcs_digit is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 common_pco_mnc_includes_pcs_digit;

  /* Optional */
  /*  Common Mobile Country Code *** */
  uint8_t common_pco_mcc_valid;  /**< Must be set to true if common_pco_mcc is being passed */
  uint16_t common_pco_mcc;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Common Operator Reserved PCO ID ** */
  uint8_t common_op_pco_id_valid;  /**< Must be set to true if common_op_pco_id is being passed */
  uint16_t common_op_pco_id;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Common Authentication Password *** */
  uint8_t common_auth_password_valid;  /**< Must be set to true if common_auth_password is being passed */
  char common_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Common User ID *** */
  uint8_t common_user_id_valid;  /**< Must be set to true if common_user_id is being passed */
  char common_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network authentication; 
       maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if the storage on the 
       wireless device is insufficient in size to hold the value.
    */

  /* Optional */
  /*  Common Authentication Protocol *** */
  uint8_t common_auth_protocol_valid;  /**< Must be set to true if common_auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 common_auth_protocol;
  /**<   Values: \n
       - 0 -- None \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  Common PCSCF Address Using PCO Flag *** */
  uint8_t common_is_pcscf_address_needed_valid;  /**< Must be set to true if common_is_pcscf_address_needed is being passed */
  uint8_t common_is_pcscf_address_needed;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using PCO \n
       - 0 -- FALSE -- Do not request \n
     By default the value is 0.
  */

  /* Optional */
  /*  Common Allow/Disallow Lingering of Interface *** */
  uint8_t linger_params_valid;  /**< Must be set to true if linger_params is being passed */
  wds_common_linger_param_type_v01 linger_params;

  /* Optional */
  /*  Common Secondary DNS IPv6 Address Preference *** */
  uint8_t common_secodnary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_secodnary_dns_ipv6_address_preference is being passed */
  uint8_t common_secodnary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS IPv6 Address Preference *** */
  uint8_t common_primary_dns_ipv6_address_preference_valid;  /**< Must be set to true if common_primary_dns_ipv6_address_preference is being passed */
  uint8_t common_primary_dns_ipv6_address_preference[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Secondary DNS IPv4 Address Preference *** */
  uint8_t common_secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_secondary_DNS_IPv4_address_preference is being passed */
  uint32_t common_secondary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via the DHCP.
  */

  /* Optional */
  /*  Common Primary DNS Address Preference *** */
  uint8_t common_primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if common_primary_DNS_IPv4_address_preference is being passed */
  uint32_t common_primary_DNS_IPv4_address_preference;
  /**<   Used as a preference during negotiation with the network;
       if not specified, the wireless device attempts to obtain the DNS
       address automatically from the network. The negotiated value is provided
       to the host via the DHCP.
  */

  /* Optional */
  /*  Common APN Class *** */
  uint8_t common_apn_class_valid;  /**< Must be set to true if common_apn_class is being passed */
  uint8_t common_apn_class;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. The APN class can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  Common APN Disabled Flag *** */
  uint8_t common_apn_disabled_flag_valid;  /**< Must be set to true if common_apn_disabled_flag is being passed */
  uint8_t common_apn_disabled_flag;
  /**<   Setting this flag disables the use of this profile for 
       making data calls. Any data call with this profile 
       fails locally. Values: \n
       - 0 -- FALSE (default) \n
       - 1 -- TRUE 
   */

  /* Optional */
  /*  Negotiate DNS Server Preference */
  uint8_t negotiate_dns_server_preference_valid;  /**< Must be set to true if negotiate_dns_server_preference is being passed */
  uint8_t negotiate_dns_server_preference;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address from the PDSN \n
       - 0 -- FALSE -- Do not request DNS addresses from the PDSN \n
       Note: Default value is 1 (TRUE).
  */

  /* Optional */
  /*  PPP Session Close Timer for DO */
  uint8_t ppp_session_close_timer_DO_valid;  /**< Must be set to true if ppp_session_close_timer_DO is being passed */
  uint32_t ppp_session_close_timer_DO;
  /**<   Timer value (in seconds) on the DO indicating how long the PPP session
       lingers before closing down.
  */

  /* Optional */
  /*  PPP Session Close Timer for 1X */
  uint8_t ppp_session_close_timer_1x_valid;  /**< Must be set to true if ppp_session_close_timer_1x is being passed */
  uint32_t ppp_session_close_timer_1x;
  /**<   The timer value (in seconds) on 1X indicating how
       long the PPP session lingers before
       closing.
  */

  /* Optional */
  /*  Allow/Disallow Lingering of Interface */
  uint8_t allow_linger_valid;  /**< Must be set to true if allow_linger is being passed */
  uint8_t allow_linger;
  /**<   Values: \n
       - 1 -- TRUE -- Allow lingering \n
       - 0 -- FALSE -- Do not allow lingering
  */

  /* Optional */
  /*  LCP ACK Timeout */
  uint8_t lcp_ack_timeout_valid;  /**< Must be set to true if lcp_ack_timeout is being passed */
  uint16_t lcp_ack_timeout;
  /**<   Value of LCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  IPCP ACK Timeout */
  uint8_t ipcp_ack_timeout_valid;  /**< Must be set to true if ipcp_ack_timeout is being passed */
  uint16_t ipcp_ack_timeout;
  /**<   Value of IPCP ACK timeout in milliseconds.
  */

  /* Optional */
  /*  AUTH Timeout */
  uint8_t auth_timeout_valid;  /**< Must be set to true if auth_timeout is being passed */
  uint16_t auth_timeout;
  /**<   Value of authentication timeout in milliseconds.
  */

  /* Optional */
  /*  LCP Configuration Request Retry Count Value */
  uint8_t lcp_creq_retry_count_valid;  /**< Must be set to true if lcp_creq_retry_count is being passed */
  uint8_t lcp_creq_retry_count;
  /**<   LCP configuration request retry count value.
  */

  /* Optional */
  /*  IPCP Configuration Request Retry Count */
  uint8_t ipcp_creq_retry_count_valid;  /**< Must be set to true if ipcp_creq_retry_count is being passed */
  uint8_t ipcp_creq_retry_count;
  /**<   IPCP configuration request retry count value.
  */

  /* Optional */
  /*  AUTH Retry */
  uint8_t auth_retry_count_valid;  /**< Must be set to true if auth_retry_count is being passed */
  uint8_t auth_retry_count;
  /**<   Authentication retry count value.
  */

  /* Optional */
  /*  Authentication Protocol */
  uint8_t auth_protocol_valid;  /**< Must be set to true if auth_protocol is being passed */
  wds_profile_auth_protocol_enum_v01 auth_protocol;
  /**<   Values: \n
       - 0 -- NONE \n
       - 1 -- PAP \n
       - 2 -- CHAP \n
       - 3 -- PAP or CHAP
  */

  /* Optional */
  /*  User ID */
  uint8_t user_id_valid;  /**< Must be set to true if user_id is being passed */
  char user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during data network
       authentication; maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the storage on the wireless device is insufficient
       in size to hold the value.
    */

  /* Optional */
  /*  Authentication Password */
  uint8_t auth_password_valid;  /**< Must be set to true if auth_password is being passed */
  char auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during data network
       authentication; maximum length allowed is 127 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the storage on the wireless device is insufficient
       in size to hold the value.
  */

  /* Optional */
  /*  Data Rate */
  uint8_t data_rate_valid;  /**< Must be set to true if data_rate is being passed */
  wds_profile_data_rate_enum_v01 data_rate;
  /**<   Values: \n
       - 0 -- Low (Low speed service options (SO15) only) \n
       - 1 -- Medium (SO33 + low R-SCH) \n
       - 2 -- High (SO33 + high R-SCH) \n
      Note: Default is 2.
  */

  /* Optional */
  /*  Application Type */
  uint8_t app_type_valid;  /**< Must be set to true if app_type is being passed */
  wds_profile_app_type_enum_v01 app_type;
  /**<   Values: \n
       - 0x00000001 -- Default application type \n
       - 0x00000020 -- LBS application type \n
       - 0x00000040 -- Tethered application type \n
       Note: Application type value in a profile cannot
       be modified. It can only be used to search for the
       profile ID numbers that have the specified
       application type.
   */

  /* Optional */
  /*  Data Mode */
  uint8_t data_mode_valid;  /**< Must be set to true if data_mode is being passed */
  wds_profile_data_mode_enum_v01 data_mode;
  /**<   Values: \n
       - 0 -- CDMA or HDR (Hybrid 1X/1xEV-DO) \n
       - 1 -- CDMA only (1X only) \n
       - 2 -- HDR only (1xEV-DO only) \n
       Note: Default is 0.
  */

  /* Optional */
  /*  Application Priority */
  uint8_t app_priority_valid;  /**< Must be set to true if app_priority is being passed */
  uint8_t app_priority;
  /**<   Numerical one byte value defining the application
       priority; higher value means higher priority. \n
       Note: Application priority value in a profile
       cannot be modified. It is currently listed for future
       extensibility of profile ID search based on
       application priority.
  */

  /* Optional */
  /*  APN String */
  uint8_t apn_string_valid;  /**< Must be set to true if apn_string is being passed */
  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String representing the access point name;
       maximum length allowed is 100 bytes.
       QMI_ERR_ARG_TOO_LONG is returned if
       the APN name is too long.
  */

  /* Optional */
  /*  PDN Type */
  uint8_t pdn_type_valid;  /**< Must be set to true if pdn_type is being passed */
  wds_profile_pdn_type_enum_v01 pdn_type;
  /**<   Values: \n
       - 0 -- IPv4 PDN type \n
       - 1 -- IPv6 PDN type \n
       - 2 -- IPv4 or IPv6 PDN type \n
       - 3 -- Unspecified PDN type (implying no preference)
   */

  /* Optional */
  /*  Is PCSCF Address Needed */
  uint8_t is_pcscf_address_needed_valid;  /**< Must be set to true if is_pcscf_address_needed is being passed */
  uint8_t is_pcscf_address_needed;
  /**<   Used to control whether the PCSCF
       address is requested from PDSN. Values: \n
       - 1 -- TRUE -- Request for PCSCF value from the PDSN \n
       - 0 -- FALSE -- Do not request for PCSCF value from the PDSN
  */

  /* Optional */
  /*  IPv4 Primary DNS Address */
  uint8_t primary_v4_dns_address_valid;  /**< Must be set to true if primary_v4_dns_address is being passed */
  uint32_t primary_v4_dns_address;
  /**<   The primary IPv4 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  IPv4 Secondary DNS Address */
  uint8_t secondary_v4_dns_address_valid;  /**< Must be set to true if secondary_v4_dns_address is being passed */
  uint32_t secondary_v4_dns_address;
  /**<   The secondary IPv4 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  Primary IPv6 DNS Address */
  uint8_t primary_v6_dns_address_valid;  /**< Must be set to true if primary_v6_dns_address is being passed */
  uint8_t primary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The primary IPv6 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  Secondary IPv6 DNS Address */
  uint8_t secondary_v6_dns_address_valid;  /**< Must be set to true if secondary_v6_dns_address is being passed */
  uint8_t secondary_v6_dns_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   The secondary IPv6 DNS address that can be
       statically assigned to the UE.
  */

  /* Optional */
  /*  RAT Type */
  uint8_t rat_type_valid;  /**< Must be set to true if rat_type is being passed */
  wds_rat_type_enum_v01 rat_type;
  /**<   Values: \n
       - 1 -- HRPD \n
       - 2 -- EHRPD \n
       - 3 -- HRPD_EHRPD
  */

  /* Optional */
  /*  APN Enabled */
  uint8_t apn_enabled_3gpp2_valid;  /**< Must be set to true if apn_enabled_3gpp2 is being passed */
  uint8_t apn_enabled_3gpp2;
  /**<   APN enabled is a flag to specify whether the APN in 
       that profile is enabled or disabled. If the APN is 
       disabled, the data call cannot be established 
       using that APN. Values: \n
       - 1 -- Enabled (default value) \n
       - 0 -- Disabled         
  */

  /* Optional */
  /*  PDN Inactivity Timeout */
  uint8_t pdn_inactivity_timeout_3gpp2_valid;  /**< Must be set to true if pdn_inactivity_timeout_3gpp2 is being passed */
  uint32_t pdn_inactivity_timeout_3gpp2;
  /**<   The duration of inactivity timer in minutes. When a PDP 
       context/PDN connection is inactive (i.e., no data 
       Rx/Tx) for this duration of time, the PDP context/PDN 
       connection is disconnected.
       The default setting of zero is treated as an infinite 
       value.
  */

  /* Optional */
  /*  APN Class */
  uint8_t apn_class_3gpp2_valid;  /**< Must be set to true if apn_class_3gpp2 is being passed */
  uint8_t apn_class_3gpp2;
  /**<   An opaque, numeric identifier representing the 
       APN in the profile. This can be transparently set 
       for any profile and queried later.
  */

  /* Optional */
  /*  PDN Level Auth Protocol * */
  uint8_t pdn_level_auth_protocol_valid;  /**< Must be set to true if pdn_level_auth_protocol is being passed */
  wds_profile_pdn_lvl_auth_proto_enum_v01 pdn_level_auth_protocol;
  /**<   Authentication protocol used during PDN level authentication.
 Values: \n
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_NONE (0) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP (1) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_CHAP (2) -- 
      - WDS_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PAP_CHAP (3) -- 
 */

  /* Optional */
  /*  PDN Level User ID * */
  uint8_t pdn_level_user_id_valid;  /**< Must be set to true if pdn_level_user_id is being passed */
  char pdn_level_user_id[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   User ID used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Level Auth Password * */
  uint8_t pdn_level_auth_password_valid;  /**< Must be set to true if pdn_level_auth_password is being passed */
  char pdn_level_auth_password[QMI_WDS_PASSWORD_MAX_V01 + 1];
  /**<   Password used during PDN level authentication. 
       Maximum length allowed is 127 bytes.
  */

  /* Optional */
  /*  PDN Label * */
  uint8_t pdn_label_valid;  /**< Must be set to true if pdn_label is being passed */
  char pdn_label[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   Logical name used to map the APN name for selecting the 
       packet data network. Maximum length allowed is 100 bytes.
  */

  /* Optional */
  /*  Operator Reserved PCO ID * */
  uint8_t op_pco_id_3gpp2_valid;  /**< Must be set to true if op_pco_id_3gpp2 is being passed */
  uint16_t op_pco_id_3gpp2;
  /**<   Container ID of this PCO.
        If op_pco_id is configured, the UE sends the operator PCO
        with the container ID that is configured. Once configured,
        the profile cannot be unconfigured.
  */

  /* Optional */
  /*  Mobile Country Code * */
  uint8_t pco_mcc_3gpp2_valid;  /**< Must be set to true if pco_mcc_3gpp2 is being passed */
  uint16_t pco_mcc_3gpp2;
  /**<   A 16-bit integer representation of MCC.
       Range: 0 to 999.
  */

  /* Optional */
  /*  Mobile Network Code * */
  uint8_t pco_mnc_includes_pcs_digit_3gpp2_valid;  /**< Must be set to true if pco_mnc_includes_pcs_digit_3gpp2 is being passed */
  wds_mnc_pcs_digit_include_status_type_v01 pco_mnc_includes_pcs_digit_3gpp2;

  /* Optional */
  /*  PDN Throttling Timer 1-6 * */
  uint8_t failure_timer_valid;  /**< Must be set to true if failure_timer is being passed */
  uint32_t failure_timer[QMI_WDS_MAX_FAILURE_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after a PDN connection or 
  IP address assignment failure. For example, immediately following a third 
  consecutive PDN connection request failure, the UE waits failure_timer[2] 
  seconds before sending the fourth request. Following failures of six or 
  greater, failure_timer[5] is used.
  */

  /* Optional */
  /*  PDN Disallow Timer 1-6 * */
  uint8_t disallow_timer_valid;  /**< Must be set to true if disallow_timer is being passed */
  uint32_t disallow_timer[QMI_WDS_MAX_DISALLOW_TIMER_V01];
  /**<   The back-off time (in seconds) to be used after the network refuses 
  to grant the requested IP address type, such as when an IPv6 address is requested 
  from a network that only grants IPv4 address. For example, immediately 
  after a third consecutive PDN connection request is denied, the UE waits 
  disallow_timer[2] seconds before sending the fourth request. Following failures of 
  six or greater, disallow_timer[5] is used.
  */

  /* Optional */
  /*  3GPP2 Application User Data  * */
  uint8_t app_user_data_3gpp2_valid;  /**< Must be set to true if app_user_data_3gpp2 is being passed */
  uint32_t app_user_data_3gpp2;
  /**<   An opaque, numeric identifier representing the
       user data in the profile. This can be transparently set
       for any profile and queried later.
  */

  /* Optional */
  /*  PCSCF Address Using DHCP 3GPP2 * */
  uint8_t pcscf_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if pcscf_addr_using_dhcp_3gpp2 is being passed */
  uint8_t pcscf_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request PCSCF address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)      
  */

  /* Optional */
  /*  DNS Address Using DHCP * */
  uint8_t dns_addr_using_dhcp_3gpp2_valid;  /**< Must be set to true if dns_addr_using_dhcp_3gpp2 is being passed */
  uint8_t dns_addr_using_dhcp_3gpp2;
  /**<   Values: \n
       - 1 -- TRUE -- Request DNS address using the DHCP \n
       - 0 -- FALSE -- Do not request (default)       
  */

  /* Optional */
  /*  IPv6 Prefix Delegation Flag * ** */
  uint8_t ipv6_prefix_delegation_valid;  /**< Must be set to true if ipv6_prefix_delegation is being passed */
  uint8_t ipv6_prefix_delegation;
  /**<   Enables IPv6 prefix delegation.
     Values: \n
     - 0 -- FALSE (default) \n
     - 1 -- TRUE
  */

  /* Optional */
  /*  Profile Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Extended error code received from the DS profile subsystem.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_get_default_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the packet data session settings currently
           in use. */
typedef struct {

  /* Optional */
  /*  Requested Settings */
  uint8_t requested_settings_valid;  /**< Must be set to true if requested_settings is being passed */
  wds_req_settings_mask_v01 requested_settings;
  /**<   Set bits to 1, corresponding to requested information. 
      All other bits must be set to 0.

      If the values are not available, the corresponding TLVs are 
      not returned in the response.

      Absence of this mask TLV results in the device returning all of 
      the available information corresponding to bits 0 through 12. 
      In cases where the information from bit 13 or greater is required, 
      this TLV with all the necessary bits set must be present in the request. Values: \n
      - Bit 0  -- Profile identifier \n
      - Bit 1  -- Profile name \n
      - Bit 2  -- PDP type \n
      - Bit 3  -- APN name \n
      - Bit 4  -- DNS address \n
      - Bit 5  -- UMTS/GPRS granted QoS \n
      - Bit 6  -- Username \n
      - Bit 7  -- Authentication Protocol \n
      - Bit 8  -- IP address \n
      - Bit 9  -- Gateway information (address and subnet mask) \n
      - Bit 10 -- PCSCF address using PCO flag \n
      - Bit 11 -- PCSCF server address list \n
      - Bit 12 -- PCSCF domain name list \n
      - Bit 13 -- MTU \n
      - Bit 14 -- Domain name list \n
      - Bit 15 -- IP family \n
      - Bit 16 -- IM_CM flag \n
      - Bit 17 -- Technology name \n
      - Bit 18 -- Operator reserved PCO
   
   */
}wds_get_runtime_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Values: \n
       - 0 -- PROFILE_TYPE_3GPP -- 3GPP
   */

  uint8_t profile_index;
  /**<   Index of the profile whose settings are loaded
       prior to session parameter negotiation for
       the current call; if this TLV is not present,
       the data call parameters are based on the device
       default settings for each parameter.
  */
}wds_runtime_profile_identifier_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t pcscf_ipv4_address;
  /**<   PCSCF IPv4 server address.*/
}wds_pcscf_ipv4_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t pcscf_ipv4_addr_list_len;  /**< Must be set to # of elements in pcscf_ipv4_addr_list */
  wds_pcscf_ipv4_addr_type_v01 pcscf_ipv4_addr_list[QMI_WDS_PCSCF_ADDR_LIST_MAX_V01];
}wds_pcscf_ipv4_addr_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  char fqdn[QMI_WDS_FQDN_MAX_V01 + 1];
  /**<   FQDN string.*/
}wds_fqdn_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t fqdn_list_len;  /**< Must be set to # of elements in fqdn_list */
  wds_fqdn_type_v01 fqdn_list[QMI_WDS_FQDN_LIST_MAX_V01];
}wds_fqdn_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  char domain_name[QMI_WDS_DOMAIN_NAME_MAX_V01 + 1];
  /**<   Domain name.*/
}wds_domain_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t domain_name_list_len;  /**< Must be set to # of elements in domain_name_list */
  wds_domain_name_type_v01 domain_name_list[QMI_WDS_DOMAIN_NAME_LIST_MAX_V01];
}wds_domain_name_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t pcscf_ipv6_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   PCSCF IPv6 server address (in network
       byte order); this is an 8-element array of
       16-bit numbers, each of which is in big-endian
       format
  */
}wds_pcscf_ipv6_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t pcscf_ipv6_addr_list_len;  /**< Must be set to # of elements in pcscf_ipv6_addr_list */
  wds_pcscf_ipv6_addr_type_v01 pcscf_ipv6_addr_list[QMI_WDS_PCSCF_ADDR_LIST_MAX_V01];
}wds_pcscf_ipv6_addr_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ipv6_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address (in network byte order). The
       address is an 16-element array of 8-bit numbers, each
       of which is in big-endian format.
  */

  uint8_t ipv6_prefix_length;
  /**<   IPv6 prefix length in number of bits. Range: 0-128.
  */
}wds_runtime_ipv6_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the packet data session settings currently
           in use. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Profile Name ** */
  uint8_t profile_name_valid;  /**< Must be set to true if profile_name is being passed */
  char profile_name[QMI_WDS_PROFILE_NAME_MAX_V01 + 1];
  /**<   One or more bytes describing the profile. The description can
       be a user-defined name for the profile.
       QMI_ERR_ARG_TOO_LONG is returned when the profile_name is too long
   */

  /* Optional */
  /*  PDP Type ** */
  uint8_t pdp_type_valid;  /**< Must be set to true if pdp_type is being passed */
  wds_pdp_type_enum_v01 pdp_type;
  /**<   The PDP type specifies the type of data payload
       exchanged over the airlink when the packet data session is
       established with this profile. Values: \n
     - 0 -- PDP-IP (IPv4) \n
     - 1 -- PDP-PPP \n
     - 2 -- PDP-IPv6 \n
     - 3 -- PDP-IPv4v6 
   */

  /* Optional */
  /*  Context Access Point Node (APN) Name ** */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   Access point name -- String parameter that is a logical name
       used to select the GGSN and external packet data network.

       If the value is NULL or omitted, the subscription default
       value is requested.

       QMI_ERR_ARG_TOO_LONG is returned if the APN name is too long.
  */

  /* Optional */
  /*  Primary DNS Address Preference * ** */
  uint8_t primary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if primary_DNS_IPv4_address_preference is being passed */
  uint32_t primary_DNS_IPv4_address_preference;
  /**<   Value used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  Secondary DNS Address Preference * ** */
  uint8_t secondary_DNS_IPv4_address_preference_valid;  /**< Must be set to true if secondary_DNS_IPv4_address_preference is being passed */
  uint32_t secondary_DNS_IPv4_address_preference;
  /**<   Value used as a preference during negotiation with the
       network; if not specified, the wireless device attempts to obtain
       the DNS address automatically from the network. The negotiated value
       is provided to the host via DHCP.
  */

  /* Optional */
  /*  UMTS Requested QoS ** */
  uint8_t umts_requested_qos_valid;  /**< Must be set to true if umts_requested_qos is being passed */
  wds_umts_qos_type_v01 umts_requested_qos;

  /* Optional */
  /*  GPRS Requested QoS ** */
  uint8_t gprs_requested_qos_valid;  /**< Must be set to true if gprs_requested_qos is being passed */
  wds_gprs_qos_type_v01 gprs_requested_qos;

  /* Optional */
  /*  Username ** */
  uint8_t username_valid;  /**< Must be set to true if username is being passed */
  char username[QMI_WDS_USER_NAME_MAX_V01 + 1];
  /**<   Username used during data network authentication.

       QMI_ERR_ARG_TOO_LONG is returned if the storage on the wireless
       device is insufficient in size to hold the value.
  */

  /* Optional */
  /*  Authentication Preference ** */
  uint8_t authentication_preference_valid;  /**< Must be set to true if authentication_preference is being passed */
  wds_auth_pref_mask_v01 authentication_preference;
  /**<   A bit map that indicates the authentication algorithm preference. 
       Values: \n
       Bit 0 -- PAP preference: \n
       - 0 -- PAP is never performed \n
       - 1 -- PAP may be performed \n
       Bit 1 -- CHAP preference: \n
       - 0 -- CHAP is never performed \n
       - 1 -- CHAP may be performed \n
       All other bits are reserved and are ignored.

       If more than one bit is set, the device decides which
       authentication procedure is performed while setting up the data
       session. For example, the device may have a policy to select
       the most secure authentication mechanism.
  */

  /* Optional */
  /*  IPv4 Address Preference * ** */
  uint8_t ipv4_address_preference_valid;  /**< Must be set to true if ipv4_address_preference is being passed */
  uint32_t ipv4_address_preference;
  /**<   Preferred IPv4 address assigned to the TE. The actual assigned
       address is negotiated with the network and may differ from this
       value. If not specified, the IPv4 address is obtained automatically
       from the network. The assigned value is provided to the host via
       DHCP.
  */

  /* Optional */
  /*  Profile Identifier ** */
  uint8_t profile_valid;  /**< Must be set to true if profile is being passed */
  wds_runtime_profile_identifier_type_v01 profile;

  /* Optional */
  /*  IPv4 Gateway Address * ** */
  uint8_t ipv4_gateway_addr_valid;  /**< Must be set to true if ipv4_gateway_addr is being passed */
  uint32_t ipv4_gateway_addr;
  /**<   Gateway address.*/

  /* Optional */
  /*  IPv4 Subnet Mask * ** */
  uint8_t ipv4_subnet_mask_valid;  /**< Must be set to true if ipv4_subnet_mask is being passed */
  uint32_t ipv4_subnet_mask;
  /**<   Subnet mask.*/

  /* Optional */
  /*  PCSCF Address Using PCO Flag ** */
  uint8_t pcscf_addr_using_pco_valid;  /**< Must be set to true if pcscf_addr_using_pco is being passed */
  uint8_t pcscf_addr_using_pco;
  /**<   Values: \n
     - 1 -- TRUE -- PCSCF address is requested using PCO \n
     - 0 -- FALSE -- It is not requested
  */

  /* Optional */
  /*  PCSCF IPv4 Server Address List ** */
  uint8_t pcscf_ipv4_server_addr_list_valid;  /**< Must be set to true if pcscf_ipv4_server_addr_list is being passed */
  wds_pcscf_ipv4_addr_list_type_v01 pcscf_ipv4_server_addr_list;
  /**<   PCSCF IPv4 server address.*/

  /* Optional */
  /*  PCSCF FQDN List ** */
  uint8_t fqdn_valid;  /**< Must be set to true if fqdn is being passed */
  wds_fqdn_list_type_v01 fqdn;

  /* Optional */
  /*  IPv6 Address * ** */
  uint8_t ipv6_addr_valid;  /**< Must be set to true if ipv6_addr is being passed */
  wds_runtime_ipv6_addr_type_v01 ipv6_addr;

  /* Optional */
  /*  IPv6 Gateway Address * ** */
  uint8_t ipv6_gateway_addr_valid;  /**< Must be set to true if ipv6_gateway_addr is being passed */
  wds_runtime_ipv6_addr_type_v01 ipv6_gateway_addr;

  /* Optional */
  /*  Primary IPv6 DNS Address * ** */
  uint8_t primary_dns_IPv6_address_valid;  /**< Must be set to true if primary_dns_IPv6_address is being passed */
  uint8_t primary_dns_IPv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Primary IPv6 DNS address in network byte
       order; an 8-element array of 16-bit
       numbers, each of which is in big-endian
       format.
  */

  /* Optional */
  /*  Secondary IPv6 DNS Address * ** */
  uint8_t secondary_dns_IPv6_address_valid;  /**< Must be set to true if secondary_dns_IPv6_address is being passed */
  uint8_t secondary_dns_IPv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Secondary IPv6 DNS address in network
       byte order; an 8-element array of
       16-bit numbers, each of which is in
       big-endian format.
  */

  /* Optional */
  /*  MTU * ** */
  uint8_t mtu_valid;  /**< Must be set to true if mtu is being passed */
  uint32_t mtu;
  /**<   MTU.*/

  /* Optional */
  /*  Domain Name List * ** */
  uint8_t domain_name_list_valid;  /**< Must be set to true if domain_name_list is being passed */
  wds_domain_name_list_type_v01 domain_name_list;

  /* Optional */
  /*  IP Family * ** */
  uint8_t ip_family_valid;  /**< Must be set to true if ip_family is being passed */
  wds_ip_family_enum_v01 ip_family;
  /**<   Values: \n
       - 4 -- IPv4_ADDR \n
       - 6 -- IPv6_ADDR
  */

  /* Optional */
  /*  IM CN Flag * */
  uint8_t im_cn_flag_valid;  /**< Must be set to true if im_cn_flag is being passed */
  uint8_t im_cn_flag;
  /**<   Values: \n
       - 0 -- FALSE \n
       - 1 -- TRUE
  */

  /* Optional */
  /*  Technology Name * ** */
  uint8_t technology_name_valid;  /**< Must be set to true if technology_name is being passed */
  wds_technology_name_enum_v01 technology_name;
  /**<   Technology on which current packet data session is in progress. Values: \n
       - -32767 -- CDMA \n
       - -32764 -- UMTS \n
       - -30592 -- EPC \n
       - -30584 -- Modem Link Local \n
      EPC is a logical interface to support LTE/eHRPD handoff. 
      It is returned if the device supports IP session continuity.

      Modem Link Local is an interface for transferring data
      between entities on the AP and modem.
  */

  /* Optional */
  /*  PCSCF IPv6 Address List * ** */
  uint8_t pcscf_ipv6_server_addr_list_valid;  /**< Must be set to true if pcscf_ipv6_server_addr_list is being passed */
  wds_pcscf_ipv6_addr_list_type_v01 pcscf_ipv6_server_addr_list;
  /**<   PCSCF IPv6 server address (in network byte order); 
       An 8-element array of 16-bit numbers, each
       of which is in big endian format.
  */

  /* Optional */
  /*  Operator Reserved Protocol Information * ** */
  uint8_t operator_reserved_pco_valid;  /**< Must be set to true if operator_reserved_pco is being passed */
  wds_op_reserved_pco_info_type_v01 operator_reserved_pco;
  /**<   \n
       Operator reserved PCO information that the device
       retrieved from the network. If there is no information available,
       a value of 0 is returned.
  */
}wds_get_runtime_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_MIP_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_MOBILE_IP_OFF_V01 = 0x00, 
  WDS_MOBILE_IP_PREFERRED_V01 = 0x01, 
  WDS_MOBILE_IP_ONLY_V01 = 0x02, 
  WDS_MIP_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_mip_mode_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the current Mobile IP mode setting for the device. */
typedef struct {

  /* Mandatory */
  /*  Mobile IP Mode * */
  wds_mip_mode_enum_v01 mip_mode;
  /**<   Values: \n
     - 0 -- MIP off (simple IP only) \n
     - 1 -- MIP preferred \n
     - 2 -- MIP only
   */
}wds_set_mip_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the current Mobile IP mode setting for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_mip_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the provisioned Mobile IP mode setting from
           the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_mip_mode_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the provisioned Mobile IP mode setting from
           the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Mobile IP Mode * */
  wds_mip_mode_enum_v01 mip_mode;
  /**<   Values: \n
     - 0 -- MIP off (simple IP only) \n
     - 1 -- MIP preferred \n
     - 2 -- MIP only
   */
}wds_get_mip_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current traffic channel status. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_dormancy_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current traffic channel status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Dormancy status   */
  wds_dormancy_status_enum_v01 dormancy_status;
  /**<   Values: \n
       - 1 -- Traffic channel dormant \n
       - 2 -- Traffic channel active
  */
}wds_get_dormancy_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries autoconnect settings. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_autoconnect_setting_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_AUTOCONNECT_SETTING_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_AUTOCONNECT_DISABLED_V01 = 0x00, 
  WDS_AUTOCONNECT_ENABLED_V01 = 0x01, 
  WDS_AUTOCONNECT_PAUSED_V01 = 0x02, 
  WDS_AUTOCONNECT_SETTING_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_autoconnect_setting_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_AUTOCONNECT_ROAM_SETTING_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_AUTOCONNECT_ROAM_ALWAYS_V01 = 0x00, 
  WDS_AUTOCONNECT_ROAM_HOME_ONLY_V01 = 0x01, 
  WDS_AUTOCONNECT_ROAM_SETTING_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_autoconnect_roam_setting_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries autoconnect settings. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Autoconnect Setting   */
  wds_autoconnect_setting_enum_v01 autoconnect_setting;
  /**<   Values: \n
       - 0x00 -- Autoconnect disabled \n
       - 0x01 -- Autoconnect enabled \n
       - 0x02 -- Autoconnect paused (resume on power cycle)
  */

  /* Optional */
  /*  Autoconnect Roam Setting */
  uint8_t autoconnect_roam_setting_valid;  /**< Must be set to true if autoconnect_roam_setting is being passed */
  wds_autoconnect_roam_setting_enum_v01 autoconnect_roam_setting;
  /**<   Values: \n
       - 0x00 -- Autoconnect always allowed \n
       - 0x01 -- Autoconnect while in home service area only \n
       Note: If inactive, this TLV is not included in the response and
       the device defaults to use 0x00 -- Autoconnect always allowed. \n
       Note: autoconnect_roam_setting is only used while autoconnect
       is enabled.
  */
}wds_get_autoconnect_setting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the duration of the current call. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_call_duration_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the duration of the current call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Call Duration  */
  uint64_t call_duration;
  /**<   Call duration in milliseconds*/

  /* Optional */
  /*  Last Call Duration */
  uint8_t last_call_duration_valid;  /**< Must be set to true if last_call_duration is being passed */
  uint64_t last_call_duration;
  /**<   Call duration in milliseconds of the last data call since device was
       powered up (zero if no call was made); returned only if not in a call.
  */

  /* Optional */
  /*  Call Active Duration */
  uint8_t call_active_duration_valid;  /**< Must be set to true if call_active_duration is being passed */
  uint64_t call_active_duration;
  /**<   Duration that the current call was active, in milliseconds; returned 
       only if in a call.
  */

  /* Optional */
  /*  Last Call Active Duration */
  uint8_t last_call_active_duration_valid;  /**< Must be set to true if last_call_active_duration is being passed */
  uint64_t last_call_active_duration;
  /**<   Duration that the last data call was active, in milliseconds, since
       the device was powered up (zero if no call has been made);
       returned only if not in a call.
  */
}wds_get_call_duration_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current data bearer technology. (Deprecated) */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_data_bearer_technology_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current data bearer technology. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Data Bearer Technology   */
  wds_data_bearer_tech_enum_v01 data_bearer_tech;
  /**<   Values: \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM \n
       - 0x04 -- UMTS \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE \n
       - 0x07 -- HSDPA and WCDMA \n
       - 0x08 -- WCDMA and HSUPA \n
       - 0x09 -- HSDPA and HSUPA \n
       - 0x0A -- LTE \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD \n
       - 0x0C -- HSDPA+ and WCDMA \n
       - 0x0D -- HSDPA+ and HSUPA \n
       - 0x0E -- DC_HSDPA+ and WCDMA \n
       - 0x0F -- DC_HSDPA+ and HSUPA \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - -1   -- Unknown
  */

  /* Optional */
  /*  Last Call Data Bearer Technology */
  uint8_t last_call_data_bearer_tech_valid;  /**< Must be set to true if last_call_data_bearer_tech is being passed */
  wds_data_bearer_tech_enum_v01 last_call_data_bearer_tech;
  /**<   Returned only if not in a call and when the previous
       call was made using RmNet (for any devices that
       support QMI_WDS_GET_DUN_CALL_INFO). \n
       Values:  \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM \n
       - 0x04 -- UMTS \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE \n
       - 0x07 -- HSDPA and WCDMA \n
       - 0x08 -- WCDMA and HSUPA \n
       - 0x09 -- HSDPA and HSUPA \n
       - 0x0A -- LTE \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD \n
       - 0x0C -- HSDPA+ and WCDMA \n
       - 0x0D -- HSDPA+ and HSUPA \n
       - 0x0E -- DC_HSDPA+ and WCDMA \n
       - 0x0F -- DC_HSDPA+ and HSUPA \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - -1   -- Unknown 
       
  */
}wds_get_data_bearer_technology_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint32_t wds_dun_req_mask_v01;
#define QMI_WDS_MASK_DUN_CONNECTION_STATUS_V01 ((wds_dun_req_mask_v01)0x01) 
#define QMI_WDS_MASK_DUN_LAST_CALL_END_REASON_V01 ((wds_dun_req_mask_v01)0x02) 
#define QMI_WDS_MASK_DUN_TX_RX_BYTES_OK_V01 ((wds_dun_req_mask_v01)0x04) 
#define QMI_WDS_MASK_DUN_DORMANCY_STATUS_V01 ((wds_dun_req_mask_v01)0x08) 
#define QMI_WDS_MASK_DUN_DATA_BEARER_V01 ((wds_dun_req_mask_v01)0x10) 
#define QMI_WDS_MASK_DUN_CHANNEL_RATE_V01 ((wds_dun_req_mask_v01)0x20) 
#define QMI_WDS_MASK_DUN_CAL_ACTIVE_DURATION_V01 ((wds_dun_req_mask_v01)0x40) 
/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t stats_peroid;
  /**<   Period between transfer statistic reports. Values: \n
       - 0 -- Do not report \n
       - Other -- Period between reports (in seconds)
  */

  wds_stats_mask_v01 stats_mask;
  /**<   Requested statistic bitmask. Each bit set causes 
       the corresponding optional TLV to be sent in the IND.
       All unlisted bits are reserviced for future use and must
       be set to zero. Values: \n
       - 0x00000040 -- Tx bytes OK \n
       - 0x00000080 -- Rx bytes OK 
  */
}wds_dun_stats_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current modem connection status. */
typedef struct {

  /* Mandatory */
  /*  Request Info */
  wds_dun_req_mask_v01 mask;
  /**<   Set the bits corresponding to the information requested to 1;
       all other bits must be set to 0. If any values are not available
       or applicable, the corresponding TLVs are not returned
       in the response. Values: \n
       - Bit 0 -- Connection status \n
       - Bit 1 -- Last call end reason \n
       - Bit 2 -- Tx/Rx bytes OK \n
       - Bit 3 -- Dormancy status \n
       - Bit 4 -- Data bearer \n
       - Bit 5 -- Channel rate \n
       - Bit 6 -- Call active duration        
  */

  /* Optional */
  /*  Connect Status Indicator */
  uint8_t report_connection_status_valid;  /**< Must be set to true if report_connection_status is being passed */
  uint8_t report_connection_status;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report connection status and call end reason
  */

  /* Optional */
  /*  Transfer Statistics Indicator */
  uint8_t report_stats_valid;  /**< Must be set to true if report_stats is being passed */
  wds_dun_stats_indicator_type_v01 report_stats;

  /* Optional */
  /*  Dormancy Status Indicator */
  uint8_t report_dormancy_status_valid;  /**< Must be set to true if report_dormancy_status is being passed */
  uint8_t report_dormancy_status;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report traffic channel state of interface used for data connection
  */

  /* Optional */
  /*  Current Data Bearer Technology Indicator */
  uint8_t report_data_bearer_tech_valid;  /**< Must be set to true if report_data_bearer_tech is being passed */
  uint8_t report_data_bearer_tech;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report radio interface used for data transfer when it changes
  */

  /* Optional */
  /*  Channel Rate Indicator */
  uint8_t report_channel_rate_valid;  /**< Must be set to true if report_channel_rate is being passed */
  uint8_t report_channel_rate;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report channel rate
  */
}wds_get_dun_call_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_connection_status_enum_v01 modem_connection_status;
  /**<   Current link status. Values: \n
       - 0x01 -- DISCONNECTED \n
       - 0x02 -- CONNECTED
  */

  uint64_t modem_call_duration;
  /**<   Call duration in milliseconds.
       If the modem connection status is connected, this
       represents the duration of the current DUN call
       If the modem connection status is disconnected,
       this represents the duration of the last DUN call since
       the device was powered up (zero, if no call has been
       made or if the last call was not DUN).
  */
}wds_dun_connection_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current modem connection status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Connection Status */
  uint8_t connection_status_valid;  /**< Must be set to true if connection_status is being passed */
  wds_dun_connection_status_type_v01 connection_status;

  /* Optional */
  /*  Last Modem Call End Reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  wds_call_end_reason_enum_v01 call_end_reason;
  /**<   Reason the call ended; see Appendix \ref{app:callEndReasons} 
       for the definition of these values.
       Only valid if the last call made was DUN, else zero is
       returned.
  */

  /* Optional */
  /*  Tx Bytes OK */
  uint8_t tx_ok_bytes_count_valid;  /**< Must be set to true if tx_ok_bytes_count is being passed */
  uint64_t tx_ok_bytes_count;
  /**<   Number of bytes transmitted without error.
       Returned only if a data call is up
  */

  /* Optional */
  /*  Rx Bytes OK */
  uint8_t rx_ok_bytes_count_valid;  /**< Must be set to true if rx_ok_bytes_count is being passed */
  uint64_t rx_ok_bytes_count;
  /**<   Number of bytes received without error.
       Returned only if a data call is up
  */

  /* Optional */
  /*  Dormancy Status */
  uint8_t dormancy_status_valid;  /**< Must be set to true if dormancy_status is being passed */
  wds_dormancy_status_enum_v01 dormancy_status;
  /**<   Current traffic channel status. Returned only 
  if a data call is up. Values: \n
       - 0x01 -- Traffic channel dormant \n
       - 0x02 -- Traffic channel active        
  */

  /* Optional */
  /*  Data Bearer Technology */
  uint8_t data_bearer_tech_valid;  /**< Must be set to true if data_bearer_tech is being passed */
  wds_data_bearer_tech_enum_v01 data_bearer_tech;
  /**<   Current data bearer technology. Returned only if 
       a data call is up. Values: \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM \n
       - 0x04 -- UMTS \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE \n
       - 0x07 -- HSDPA and WCDMA \n
       - 0x08 -- WCDMA and HSUPA \n
       - 0x09 -- HSDPA and HSUPA \n
       - 0x0A -- LTE \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD \n
       - 0x0C -- HSDPA+ and WCDMA \n
       - 0x0D -- HSDPA+ and HSUPA \n
       - 0x0E -- DC_HSDPA+ and WCDMA \n
       - 0x0F -- DC_HSDPA+ and HSUPA \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - -1   -- Unknown        
  */

  /* Optional */
  /*  Channel Rate */
  uint8_t channel_rate_valid;  /**< Must be set to true if channel_rate is being passed */
  wds_current_channel_rate_type_v01 channel_rate;

  /* Optional */
  /*  Last Call Tx Bytes OK */
  uint8_t last_call_tx_ok_bytes_count_valid;  /**< Must be set to true if last_call_tx_ok_bytes_count is being passed */
  uint64_t last_call_tx_ok_bytes_count;
  /**<   Number of bytes transmitted without error during the last
       data call (0 if no call was made).
       Return only if not in a call and the previous call
       was made using DUN.
  */

  /* Optional */
  /*  Last Call Rx Bytes OK */
  uint8_t last_call_rx_ok_bytes_count_valid;  /**< Must be set to true if last_call_rx_ok_bytes_count is being passed */
  uint64_t last_call_rx_ok_bytes_count;
  /**<   Number of bytes received without error during the last
       data call (0 if no call was made).
       Returned only if not in a call and the previous
       call was made using DUN.
  */

  /* Optional */
  /*  Call Active Duration */
  uint8_t modem_call_duration_active_valid;  /**< Must be set to true if modem_call_duration_active is being passed */
  uint64_t modem_call_duration_active;
  /**<   Duration that the call is active in milliseconds.
       If the modem connection status is connected, this
       represents the active duration of the current DUN call
       If the modem connection status is disconnected,
       this represents the active duration of the last DUN call
       since the device was powered up (0 if no call has
       been made or if last call was not DUN.
  */

  /* Optional */
  /*  Last Call Data Bearer Technology */
  uint8_t last_call_data_bearer_tech_valid;  /**< Must be set to true if last_call_data_bearer_tech is being passed */
  wds_data_bearer_tech_enum_v01 last_call_data_bearer_tech;
  /**<   Returned only if not in a call and when the previous call was 
       made using DUN. \n
       Values: \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM \n
       - 0x04 -- UMTS \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE \n
       - 0x07 -- HSDPA and WCDMA \n
       - 0x08 -- WCDMA and HSUPA \n
       - 0x09 -- HSDPA and HSUPA \n
       - 0x0A -- LTE \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD \n
       - 0x0C -- HSDPA+ and WCDMA \n
       - 0x0D -- HSDPA+ and HSUPA \n
       - 0x0E -- DC_HSDPA+ and WCDMA \n
       - 0x0F -- DC_HSDPA+ and HSUPA \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - -1   -- Unknown        
  */
}wds_get_dun_call_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the DUN data connection status. */
typedef struct {

  /* Optional */
  /*  Connection Status */
  uint8_t modem_connection_status_valid;  /**< Must be set to true if modem_connection_status is being passed */
  wds_connection_status_enum_v01 modem_connection_status;
  /**<   Current link status. Values: \n
       - 0x01 -- DISCONNECTED \n
       - 0x02 -- CONNECTED
  */

  /* Optional */
  /*  Last Modem Call End Reason */
  uint8_t call_end_reason_valid;  /**< Must be set to true if call_end_reason is being passed */
  wds_call_end_reason_enum_v01 call_end_reason;
  /**<   Reason the call ended; see Appendix \ref{app:callEndReasons} for the definition
       of these values.
  */

  /* Optional */
  /*  Tx Bytes OK */
  uint8_t tx_ok_bytes_count_valid;  /**< Must be set to true if tx_ok_bytes_count is being passed */
  uint64_t tx_ok_bytes_count;
  /**<   Number of bytes transmitted without error.*/

  /* Optional */
  /*  Rx Bytes OK */
  uint8_t rx_ok_bytes_count_valid;  /**< Must be set to true if rx_ok_bytes_count is being passed */
  uint64_t rx_ok_bytes_count;
  /**<   Number of bytes received without error.*/

  /* Optional */
  /*  Dormancy Status */
  uint8_t dormancy_status_valid;  /**< Must be set to true if dormancy_status is being passed */
  wds_dormancy_status_enum_v01 dormancy_status;
  /**<   Values: \n
       - 0x01 -- Traffic channel dormant \n
       - 0x02 -- Traffic channel active
  */

  /* Optional */
  /*  Data Bearer Technology */
  uint8_t data_beare_technology_valid;  /**< Must be set to true if data_beare_technology is being passed */
  wds_data_bearer_tech_enum_v01 data_beare_technology;
  /**<   Values: \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM \n
       - 0x04 -- UMTS \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE \n
       - 0x07 -- HSDPA and WCDMA \n
       - 0x08 -- WCDMA and HSUPA \n
       - 0x09 -- HSDPA and HSUPA \n
       - 0x0A -- LTE \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD \n
       - 0x0C -- HSDPA+ and WCDMA \n
       - 0x0D -- HSDPA+ and HSUPA \n
       - 0x0E -- DC_HSDPA+ and WCDMA \n
       - 0x0F -- DC_HSDPA+ and HSUPA \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - -1   -- Unknown
  */

  /* Optional */
  /*  Channel Rate */
  uint8_t channel_rate_valid;  /**< Must be set to true if channel_rate is being passed */
  wds_channel_rate_type_v01 channel_rate;
}wds_dun_call_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current Mobile IP mode profile index from the devices. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_active_mip_profile_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current Mobile IP mode profile index from the devices. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Mobile IP Profile Identifier *  */
  uint8_t profile_index;
  /**<   Index of the active profile.*/
}wds_get_active_mip_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  char spc[QMI_WDS_SPC_LEN_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only).*/

  uint8_t profile_index;
  /**<   Index of the profile.*/
}wds_mip_profile_identifier_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the Mobile IP mode setting for the active profile of the device. */
typedef struct {

  /* Mandatory */
  /*  Mobile IP Profile Identifier * */
  wds_mip_profile_identifier_type_v01 profile_identifier;
}wds_set_active_mip_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the Mobile IP mode setting for the active profile of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_active_mip_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries a mobile IP profile from the device. */
typedef struct {

  /* Mandatory */
  /*  Mobile IP Profile Identifier * */
  uint8_t profile_index;
  /**<   Index of the profile to read.*/
}wds_read_mip_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_MIP_KEY_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_MIP_KEY_UNSET_V01 = 0x00, 
  WDS_MIP_KEY_DEFAULT_V01 = 0x01, 
  WDS_MIP_KEY_NOTDEFAULT_V01 = 0x02, 
  WDS_MIP_KEY_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_mip_key_state_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries a mobile IP profile from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Mobile IP Profile State */
  uint8_t profile_state_valid;  /**< Must be set to true if profile_state is being passed */
  uint8_t profile_state;
  /**<   Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */

  /* Optional */
  /*  Mobile IP Profile Home Address */
  uint8_t home_address_valid;  /**< Must be set to true if home_address is being passed */
  uint32_t home_address;
  /**<   Home address (IPv4 format).*/

  /* Optional */
  /*  Mobile IP Profile HA Primary */
  uint8_t home_agent_priv_valid;  /**< Must be set to true if home_agent_priv is being passed */
  uint32_t home_agent_priv;
  /**<   Primary home agent (HA) address (IPv4 format).*/

  /* Optional */
  /*  Mobile IP Profile HA Secondary */
  uint8_t home_agent_sec_valid;  /**< Must be set to true if home_agent_sec is being passed */
  uint32_t home_agent_sec;
  /**<   Secondary HA address (IPv4 format).*/

  /* Optional */
  /*  Mobile IP Profile Reverse Tunneling Pref */
  uint8_t rev_tun_pref_valid;  /**< Must be set to true if rev_tun_pref is being passed */
  uint8_t rev_tun_pref;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Mobile IP Profile NAI */
  uint8_t nai_valid;  /**< Must be set to true if nai is being passed */
  char nai[QMI_WDS_MAX_NAI_LEN_V01 + 1];
  /**<   NAI string in ASCII text.
       QMI_ERR_ARG_TOO_LONG is returned if the NAI is too long.
  */

  /* Optional */
  /*  Mobile IP Profile HA SPI */
  uint8_t mn_ha_spi_valid;  /**< Must be set to true if mn_ha_spi is being passed */
  uint32_t mn_ha_spi;
  /**<   HA security parameter index.*/

  /* Optional */
  /*  Mobile IP Profile AAA SPI */
  uint8_t mn_aaa_spi_valid;  /**< Must be set to true if mn_aaa_spi is being passed */
  uint32_t mn_aaa_spi;
  /**<   AAA server security parameter index.*/

  /* Optional */
  /*  Mobile IP Profile HA Key State * */
  uint8_t mn_ha_key_state_valid;  /**< Must be set to true if mn_ha_key_state is being passed */
  wds_mip_key_state_enum_v01 mn_ha_key_state;
  /**<   Values: \n
       - 0x00 -- Unset (empty) \n
       - 0x01 -- Set but still default value \n
       - 0x02 -- Set and modified from default value
  */

  /* Optional */
  /*  Mobile IP Profile AAA Key State * */
  uint8_t mn_aaa_key_state_valid;  /**< Must be set to true if mn_aaa_key_state is being passed */
  wds_mip_key_state_enum_v01 mn_aaa_key_state;
  /**<   Values: \n
       - 0x00 -- Unset (empty) \n
       - 0x01 -- Set but still default value \n
       - 0x02 -- Set and modified from default value
  */
}wds_read_mip_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Modifies a mobile IP profile on the device. */
typedef struct {

  /* Mandatory */
  /*  Mobile IP Profile Identifier * */
  wds_mip_profile_identifier_type_v01 profile_identifier;

  /* Optional */
  /*  Mobile IP Profile State * */
  uint8_t profile_state_valid;  /**< Must be set to true if profile_state is being passed */
  uint8_t profile_state;
  /**<   Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled 
  */

  /* Optional */
  /*  Mobile IP Profile Home Address * */
  uint8_t home_address_valid;  /**< Must be set to true if home_address is being passed */
  uint32_t home_address;
  /**<   Home address (IPv4 format).*/

  /* Optional */
  /*  Mobile IP Profile HA Primary * */
  uint8_t home_agent_priv_valid;  /**< Must be set to true if home_agent_priv is being passed */
  uint32_t home_agent_priv;
  /**<   Primary home agent address (IPv4 format).*/

  /* Optional */
  /*  Mobile IP Profile HA Secondary * */
  uint8_t home_agent_sec_valid;  /**< Must be set to true if home_agent_sec is being passed */
  uint32_t home_agent_sec;
  /**<   Secondary home agent address (IPv4 format).*/

  /* Optional */
  /*  Mobile IP Profile Reverse Tunneling Preference * */
  uint8_t rev_tun_pref_valid;  /**< Must be set to true if rev_tun_pref is being passed */
  uint8_t rev_tun_pref;
  /**<    Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Mobile IP Profile NAI * */
  uint8_t nai_valid;  /**< Must be set to true if nai is being passed */
  char nai[QMI_WDS_MAX_NAI_LEN_V01 + 1];
  /**<   NAI (network access identifier) string in ASCII text.
       QMI_ERR_ARG_TOO_LONG is returned if the NAI is too long.
  */

  /* Optional */
  /*  Mobile IP Profile HA SPI * */
  uint8_t mn_ha_spi_valid;  /**< Must be set to true if mn_ha_spi is being passed */
  uint32_t mn_ha_spi;
  /**<   HA security parameter index.*/

  /* Optional */
  /*  Mobile IP Profile AAA SPI * */
  uint8_t mn_aaa_spi_valid;  /**< Must be set to true if mn_aaa_spi is being passed */
  uint32_t mn_aaa_spi;
  /**<   AAA server security parameter index.*/

  /* Optional */
  /*  MN-HA Key * */
  uint8_t mn_ha_key_valid;  /**< Must be set to true if mn_ha_key is being passed */
  char mn_ha_key[QMI_WDS_MAX_MN_HA_KEY_LEN_V01 + 1];
  /**<   QMI_ERR_ARG_TOO_LONG is returned if the MN-HA key is too long.
  */

  /* Optional */
  /*  MN-AAA Key * */
  uint8_t mn_aaa_key_valid;  /**< Must be set to true if mn_aaa_key is being passed */
  char mn_aaa_key[QMI_WDS_MAX_MN_AAA_KEY_LEN_V01 + 1];
  /**<   String containing MN-AAA key.
       QMI_ERR_ARG_TOO_LONG is returned if the MN-AAA key is too long.
  */
}wds_modify_mip_profile_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Modifies a mobile IP profile on the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_modify_mip_profile_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the mobile IP settings from the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_mip_settings_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the mobile IP settings from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Mobile IP Mode */
  uint8_t mip_mode_valid;  /**< Must be set to true if mip_mode is being passed */
  wds_mip_mode_enum_v01 mip_mode;
  /**<   Mode: \n
       - 0x00 -- MIP off (Simple IP only) \n
       - 0x01 -- MIP preferred \n
       - 0x02 -- MIP only
  */

  /* Optional */
  /*  Mobile IP Reg Retry Count */
  uint8_t mip_reg_retry_count_valid;  /**< Must be set to true if mip_reg_retry_count is being passed */
  uint8_t mip_reg_retry_count;
  /**<   Mobile IP registration retry attempt limit.
  */

  /* Optional */
  /*  Mobile IP Reg Retry Interval */
  uint8_t mip_reg_retry_interval_valid;  /**< Must be set to true if mip_reg_retry_interval is being passed */
  uint8_t mip_reg_retry_interval;
  /**<   Mobile IP initial interval modifier used to determine
       the time between registration attempts (valid range 0-4).
  */

  /* Optional */
  /*  Mobile IP Re-Reg Period */
  uint8_t mip_re_reg_peroid_valid;  /**< Must be set to true if mip_re_reg_peroid is being passed */
  uint8_t mip_re_reg_peroid;
  /**<   Mobile IP period to attempt reregistration before
       current registration expires (in minutes).
  */

  /* Optional */
  /*  Mobile IP Re-Reg if Traffic */
  uint8_t mip_re_reg_if_traf_valid;  /**< Must be set to true if mip_re_reg_if_traf is being passed */
  uint8_t mip_re_reg_if_traf;
  /**<   Mobile IP reregistration occursonly if there is traffic since 
       the last attempt. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled 
  */

  /* Optional */
  /*  Mobile IP QC Domant Handoff */
  uint8_t mip_qc_handoff_valid;  /**< Must be set to true if mip_qc_handoff is being passed */
  uint8_t mip_qc_handoff;
  /**<   Mobile IP MN-HA authenticator calculator. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */

  /* Optional */
  /*  Mobile IP RFC2002 MN-HA Auth */
  uint8_t mip_rfc2002bis_valid;  /**< Must be set to true if mip_rfc2002bis is being passed */
  uint8_t mip_rfc2002bis;
  /**<   Mobile IP MN-HA authenticator calculation using
       RFC2002bis instead of RFC2002. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */
}wds_get_mip_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the current mobile IP setting for the device. */
typedef struct {

  /* Mandatory */
  /*  Service Programming Authentication * */
  char spc[QMI_WDS_SPC_LEN_V01];
  /**<   SPC in ASCII format (digits 0 to 0 only).
  */

  /* Optional */
  /*  Mobile IP Mode * */
  uint8_t mip_mode_valid;  /**< Must be set to true if mip_mode is being passed */
  wds_mip_mode_enum_v01 mip_mode;
  /**<   Values: \n
       - 0x00 -- MIP off (Simple IP only) \n
       - 0x01 -- MIP preferred \n
       - 0x02 -- MIP only
  */

  /* Optional */
  /*  Mobile IP Reg Retry Count * */
  uint8_t mip_reg_retry_count_valid;  /**< Must be set to true if mip_reg_retry_count is being passed */
  uint8_t mip_reg_retry_count;
  /**<   Mobile IP registration retry attempt limit.
  */

  /* Optional */
  /*  Mobile IP Reg Retry Interval * */
  uint8_t mip_reg_retry_interval_valid;  /**< Must be set to true if mip_reg_retry_interval is being passed */
  uint8_t mip_reg_retry_interval;
  /**<   Mobile IP initial interval modifier used to determine
       time between registration attempts (valid range 0-4).
  */

  /* Optional */
  /*  Mobile IP Re-Reg Period * */
  uint8_t mip_re_reg_peroid_valid;  /**< Must be set to true if mip_re_reg_peroid is being passed */
  uint8_t mip_re_reg_peroid;
  /**<   Mobile IP period to attempt reregistration before
       current registration expires (in minutes).
  */

  /* Optional */
  /*  Mobile IP Re-Reg if Traffic * */
  uint8_t mip_re_reg_if_traf_valid;  /**< Must be set to true if mip_re_reg_if_traf is being passed */
  uint8_t mip_re_reg_if_traf;
  /**<   Mobile IP reregistration only if traffic since the 
       last attempt. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */

  /* Optional */
  /*  Mobile IP QC Domant Handoff * */
  uint8_t mip_qc_handoff_valid;  /**< Must be set to true if mip_qc_handoff is being passed */
  uint8_t mip_qc_handoff;
  /**<   Mobile IP MN-HA authenticator calculator. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */

  /* Optional */
  /*  Mobile IP RFC2002 MN-HA Auth * */
  uint8_t mip_rfc2002bis_valid;  /**< Must be set to true if mip_rfc2002bis is being passed */
  uint8_t mip_rfc2002bis;
  /**<   Mobile IP MN-HA authenticator calculation using
       RFC2002bis instead of RFC2002. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */
}wds_set_mip_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the current mobile IP setting for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_mip_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the last mobile IP status from the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_last_mip_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the last mobile IP status from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Last MIP Status * */
  uint8_t mip_error;
  /**<   Status of the last MIP call (or attempt). Values: \n
       - 0x00 -- Success \n
       - > 0  -- Error code (as defined in \hyperref[S4]{[S4]})
  */
}wds_get_last_mip_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current data bearer technology. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_current_data_bearer_technology_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current data bearer technology. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Current Data Bearer Technology  */
  wds_current_bearer_tech_type_v01 current_bearer_tech;

  /* Optional */
  /*  Last Call Bearer Technology */
  uint8_t last_bearer_tech_valid;  /**< Must be set to true if last_bearer_tech is being passed */
  wds_current_bearer_tech_type_v01 last_bearer_tech;
}wds_get_current_data_bearer_technology_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries a list of call history records from
           the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_call_history_list_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DATA_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_CALL_TYPE_RMNET_V01 = 0x00, 
  WDS_DATA_CALL_TYPE_DUN_V01 = 0x01, 
  WDS_DATA_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_data_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_data_call_type_enum_v01 call_type;
  /**<   Call type. Values: \n
       - 0x00 -- RmNet \n
       - 0x01 -- Dial Up Network (DUN) 
  */

  wds_data_bearer_tech_enum_v01 call_data_bearer;
  /**<   Data bearer technology.
 Values: \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- GSM \n
       - 0x04 -- UMTS \n
       - 0x05 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO RevA) \n
       - 0x06 -- EDGE \n
       - 0x07 -- HSDPA and WCDMA \n
       - 0x08 -- WCDMA and HSUPA \n
       - 0x09 -- HSDPA and HSUPA \n
       - 0x0A -- LTE \n
       - 0x0B -- cdma2000\textsuperscript{\textregistered} EHRPD \n
       - 0x0C -- HSDPA+ and WCDMA \n
       - 0x0D -- HSDPA+ and HSUPA \n
       - 0x0E -- DC_HSDPA+ and WCDMA \n
       - 0x0F -- DC_HSDPA+ and HSUPA \n
       - 0x10 -- HSDPA+ and 64QAM        \n
       - 0x11 -- HSDPA+, 64QAM and HSUPA \n
       - 0x12 -- TDSCDMA                 \n
       - 0x13 -- TDSCDMA and HSDPA       \n
       - 0x14 -- TDSCDMA and HSUPA       \n
       - 0x15 -- IWLAN S2B               \n
       - -1   -- Unknown 
   */

  uint64_t call_timestamp;
  /**<   Call origination timestamp.*/

  uint32_t call_ip_addr;
  /**<   Call IP address (IPv4 format).
       Note: this value is zero if the IP address cannot be determined.
  */

  uint64_t call_duration_total;
  /**<   Total duration of the call in milliseconds.*/

  uint64_t call_duration_active;
  /**<   Duration the call is active in milliseconds.*/

  uint64_t call_rx_ok_bytes;
  /**<   Number of bytes transmitted without error.*/

  uint64_t call_tx_ok_bytes;
  /**<   Number of bytes received without error.*/

  wds_call_end_reason_enum_v01 call_end_reason;
  /**<   Reason the call ended.*/

  char call_phone_num[QMI_WDS_MAX_PHONE_NUMBER_LEN_V01 + 1];
  /**<   Phone number.*/
}wds_call_record_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t call_record_id;
  /**<   Unique record ID.*/

  wds_call_record_type_v01 call_record;
}wds_full_call_record_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t full_call_history_len;  /**< Must be set to # of elements in full_call_history */
  wds_full_call_record_type_v01 full_call_history[QMI_WDS_MAX_CALL_HISTORY_V01];
}wds_full_call_history_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t call_record_id;
  /**<   Unique record ID.*/
}wds_id_only_call_record_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t id_only_call_history_len;  /**< Must be set to # of elements in id_only_call_history */
  wds_id_only_call_record_type_v01 id_only_call_history[QMI_WDS_MAX_CALL_HISTORY_V01];
}wds_id_only_call_history_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries a list of call history records from
           the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Full Call History List */
  uint8_t full_call_history_valid;  /**< Must be set to true if full_call_history is being passed */
  wds_full_call_history_type_v01 full_call_history;

  /* Optional */
  /*  Record ID-Only Call History List */
  uint8_t id_only_call_history_valid;  /**< Must be set to true if id_only_call_history is being passed */
  wds_id_only_call_history_type_v01 id_only_call_history;
}wds_call_history_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries a call history record from the device. */
typedef struct {

  /* Mandatory */
  /*  Call History Record ID */
  uint16_t call_record_id;
  /**<   Record ID of the call history record to read.*/
}wds_call_history_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries a call history record from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Call History Record   */
  wds_call_record_type_v01 call_history_record;
}wds_call_history_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Clears the call history records from the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_call_history_delete_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Clears the call history records from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_call_history_delete_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Requests the maximum number of call history records that 
           can be stored in the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_call_history_max_size_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Requests the maximum number of call history records that 
           can be stored in the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Call History Size   */
  uint16_t max_size;
  /**<   Maximum number of call history records that can be stored.*/
}wds_call_history_max_size_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_FAMILY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_FAMILY_EMBEDDED_V01 = 0x00, 
  WDS_PROFILE_FAMILY_TETHERED_V01 = 0x01, 
  WDS_PROFILE_FAMILY_SOCKET_V01 = 0x01, 
  WDS_PROFILE_FAMILY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_family_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
       - 0x0 -- 3GPP \n
       - 0x1 -- 3GPP2
   */

  wds_profile_family_enum_v01 profile_family;
  /**<   Identifies the family of the profile. Values: \n
       - 0 -- Embedded \n
       - 1 -- Tethered \n
       - 1 -- Sockets (deprecated)
  */
}wds_profile_id_family_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the default profile number configured on the
             wireless device for the specified technology. */
typedef struct {

  /* Mandatory */
  /*  Profile Type */
  wds_profile_id_family_type_v01 profile;
}wds_get_default_profile_num_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the default profile number configured on the
             wireless device for the specified technology. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Default Profile Number   */
  uint8_t profile_index;
  /**<   Profile number identifying the default profile.*/

  /* Optional */
  /*  Extended error code. */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_get_default_profile_num_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
       - 0 -- 3GPP \n
       - 1 -- 3GPP2 
   */

  wds_profile_family_enum_v01 profile_family;
  /**<   Identifies the family of profile. Values: \n
       - 0 -- Embedded \n
       - 1 -- Tethered \n
       - 1 -- Sockets (deprecated)
  */

  uint8_t profile_index;
  /**<   Profile number to be set as default profile.*/
}wds_profile_identifier_with_family_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the default profile number on the wireless device for
             the specified technology. */
typedef struct {

  /* Mandatory */
  /*  Profile Identifier */
  wds_profile_identifier_with_family_type_v01 profile_identifier;
}wds_set_default_profile_num_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the default profile number on the wireless device for
             the specified technology. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_set_default_profile_num_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Resets all the parameters of the specified profile and
             technology to default values. */
typedef struct {

  /* Mandatory */
  /*  Profile Identifier */
  wds_profile_identifier_type_v01 profile_identifier;
}wds_reset_profile_to_default_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Resets all the parameters of the specified profile and
             technology to default values. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_reset_profile_to_default_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROFILE_PARAM_ID_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROFILE_PARAM_ID_UMTS_REQ_QOS_V01 = 0x17, 
  WDS_PROFILE_PARAM_ID_UMTS_MIN_QOS_V01 = 0x18, 
  WDS_PROFILE_PARAM_ID_GPRS_REQ_QOS_V01 = 0x19, 
  WDS_PROFILE_PARAM_ID_GPRS_MIN_QOS_V01 = 0x1A, 
  WDS_PROFILE_PARAM_ID_TFT_FILTER_1_V01 = 0x23, 
  WDS_PROFILE_PARAM_ID_TFT_FILTER_2_V01 = 0x24, 
  WDS_PROFILE_PARAM_ID_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_profile_param_id_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
       - 0 -- 3GPP \n
       - 1 -- 3GPP2
   */

  uint8_t profile_index;
  /**<   Profile number whose profile_param_id needs to be set to invalid.*/

  wds_profile_param_id_enum_v01 profile_param_id;
  /**<   Profile parameter that must be marked as invalid; only the following
       values are allowed: \n
         - 0x17 -- UMTS requested QoS \n
         - 0x18 -- UMTS minimum QoS \n
         - 0x19 -- GPRS requested QoS \n
         - 0x1A -- GPRS minimum QoS \n
         - 0x23 -- TFT filter ID 1 \n
         - 0x24 -- TFT filter ID 2
  */
}wds_profile_param_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Resets the specified profile parameter type for the specified
             technology to invalid. */
typedef struct {

  /* Mandatory */
  /*  Profile Parameter */
  wds_profile_param_type_v01 profile_param;
}wds_reset_profile_param_to_invalid_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Resets the specified profile parameter type for the specified
             technology to invalid. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_ds_extended_error_code_enum_v01 extended_error_code;
  /**<   Error code from the DS profile.
       These error codes are explained in Appendix  
  \ref{app:DSProfileExtendedErrorCodes}.
  */
}wds_reset_profile_param_to_invalid_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the control point IP preference. */
typedef struct {

  /* Mandatory */
  /*  IP Family Preference */
  wds_ip_family_enum_v01 ip_preference;
  /**<   Values: \n
         - 0x04 -- IPv4 \n
         - 0x06 -- IPv6
  */
}wds_set_client_ip_family_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the control point IP preference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_client_ip_family_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t stream_id;
  /**<   Stream ID*/

  uint8_t nat_presence_indicator;
  /**<   Indicates whether the NAT is present: \n
       - 0x00 -- Absent \n
       - 0x01 -- Present
  */

  uint16_t port_id;
  /**<   Port ID*/
}wds_tunnel_params_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the tunnel parameters for FMC. */
typedef struct {

  /* Mandatory */
  /*  Tunnel Parameters */
  wds_tunnel_params_type_v01 tunnel_params;

  /* Optional */
  /*  IPv4 Socket Address */
  uint8_t ipv4_sock_addr_valid;  /**< Must be set to true if ipv4_sock_addr is being passed */
  uint8_t ipv4_sock_addr[QMI_WDS_IPV4_ADDR_LEN_V01];
  /**<   Byte array containing IPv4 socket address information
       in network byte order.
  */

  /* Optional */
  /*  IPv6 Socket Address */
  uint8_t ipv6_sock_addr_valid;  /**< Must be set to true if ipv6_sock_addr is being passed */
  uint8_t ipv6_sock_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Byte array containing IPv6 socket address information
       in network byte order.
  */
}wds_fmc_set_tunnel_params_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the tunnel parameters for FMC. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_fmc_set_tunnel_params_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Clears the tunnel parameters for FMC. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_fmc_clear_tunnel_params_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Clears the tunnel parameters for FMC. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_fmc_clear_tunnel_params_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the FMC tunnel parameters from the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_fmc_get_tunnel_params_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the FMC tunnel parameters from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Tunnel Parameters */
  uint8_t tunnel_params_valid;  /**< Must be set to true if tunnel_params is being passed */
  wds_tunnel_params_type_v01 tunnel_params;

  /* Optional */
  /*  IPv4 Socket Address */
  uint8_t ipv4_sock_addr_valid;  /**< Must be set to true if ipv4_sock_addr is being passed */
  uint8_t ipv4_sock_addr[QMI_WDS_IPV4_ADDR_LEN_V01];
  /**<   Byte array containing IPv4 socket address information
       in network byte order.
  */

  /* Optional */
  /*  IPv6 Socket Address */
  uint8_t ipv6_sock_addr_valid;  /**< Must be set to true if ipv6_sock_addr is being passed */
  uint8_t ipv6_sock_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Byte array containing IPv6 socket address information
       in network byte order.
  */
}wds_fmc_get_tunnel_params_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the autoconnect settings. */
typedef struct {

  /* Mandatory */
  /*  Autoconnect Setting */
  wds_autoconnect_setting_enum_v01 autoconnect_setting;
  /**<   Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled \n
       - 0x02 -- Paused (resume on power cycle) 
  */

  /* Optional */
  /*  Autoconnect Roam Setting */
  uint8_t autoconnect_roam_setting_valid;  /**< Must be set to true if autoconnect_roam_setting is being passed */
  wds_autoconnect_roam_setting_enum_v01 autoconnect_roam_setting;
  /**<   Current autoconnect roaming status. Values: \n
       - 0x00 -- Autoconnect always allowed \n
       - 0x01 -- Autoconnect while in home service area only \n
       Note: Autoconnect roam setting is only used while
       autoconnect is enabled.
   */
}wds_set_autoconnect_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the autoconnect settings. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_autoconnect_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current DNS settings for the device. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_dns_settings_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current DNS settings for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Primary DNS IPv4 Address */
  uint8_t primary_dns_ipv4_address_valid;  /**< Must be set to true if primary_dns_ipv4_address is being passed */
  uint32_t primary_dns_ipv4_address;
  /**<   The primary DNS address reported from the device.
       Note: A value of 0.0.0.0 or the absence of this TLV
       indicates that the network values are reported.
  */

  /* Optional */
  /*  Secondary DNS IPv4 Address */
  uint8_t secondary_dns_ipv4_address_valid;  /**< Must be set to true if secondary_dns_ipv4_address is being passed */
  uint32_t secondary_dns_ipv4_address;
  /**<   Secondary DNS address reported from the device. \n
       Note: A value of 0.0.0.0 or the absence of this TLV
       indicates that the network values are reported.
  */

  /* Optional */
  /*  Primary IPv6 DNS Address */
  uint8_t primary_dns_ipv6_address_valid;  /**< Must be set to true if primary_dns_ipv6_address is being passed */
  uint8_t primary_dns_ipv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Primary IPv6 DNS address in network byte order;
       an 8-element array of 16-bit numbers, each of
       which is in big-endian format.
       Note: A value of 0 indicates that the network values are reported.
  */

  /* Optional */
  /*  Secondary IPv6 DNS Address */
  uint8_t secondary_dns_ipv6_address_valid;  /**< Must be set to true if secondary_dns_ipv6_address is being passed */
  uint8_t secondary_dns_ipv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Secondary IPv6 DNS address in network byte order;
       an 8-element array of 16-bit numbers, each of
       which is in big-endian format.
       Note: A value of 0 indicates that the network values are reported.
  */
}wds_get_dns_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the current DNS settings for the device. */
typedef struct {

  /* Optional */
  /*  Primary DNS IPv4 Address */
  uint8_t primary_dns_ipv4_address_valid;  /**< Must be set to true if primary_dns_ipv4_address is being passed */
  uint32_t primary_dns_ipv4_address;
  /**<   Primary DNS address reported from the device.
       Note: A value of 0.0.0.0 indicates that the network values are reported.
  */

  /* Optional */
  /*  Secondary DNS IPv4 Address */
  uint8_t secondary_dns_ipv4_address_valid;  /**< Must be set to true if secondary_dns_ipv4_address is being passed */
  uint32_t secondary_dns_ipv4_address;
  /**<   Secondary DNS address reported from the device.
       Note: A value of 0.0.0.0 indicates that the network values are reported.
  */

  /* Optional */
  /*  Primary IPv6 DNS Address */
  uint8_t primary_dns_ipv6_address_valid;  /**< Must be set to true if primary_dns_ipv6_address is being passed */
  uint8_t primary_dns_ipv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Primary IPv6 DNS address in network byte order;
       an 8-element array of 16-bit numbers, each of
       which is in big-endian format.
       Note: A value of 0 indicates that the network values are reported.
  */

  /* Optional */
  /*  Secondary IPv6 DNS Address */
  uint8_t secondary_dns_ipv6_address_valid;  /**< Must be set to true if secondary_dns_ipv6_address is being passed */
  uint8_t secondary_dns_ipv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Secondary IPv6 DNS address in network byte order;
       an 8-element array of 16-bit numbers, each of
       which is in big-endian format.
       Note: A value of 0 indicates that the network values are reported.
  */
}wds_set_dns_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the current DNS settings for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_dns_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the packet data session information before
           dormancy. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_pre_dormancy_cdma_settings_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_SERVICE_OPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_SO_IS_657_V01 = 0x0007, 
  WDS_SO_IS_657_RATE_SET_2_V01 = 0x000F, 
  WDS_SO_IS_707A_RATE_SET_1_FWD_REV_V01 = 0x0016, 
  WDS_SO_IS_707A_RATE_SET_2_FWD_REV_V01 = 0x0019, 
  WDS_SO_CDMA2000_PKT_V01 = 0x0021, 
  WDS_SO_IS_707_V01 = 0x1007, 
  WDS_SO_QC_RATE_SET_2_V01 = 0x8020, 
  WDS_SO_NULL_V01 = -1, 
  WDS_SERVICE_OPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_service_option_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DATA_SESS_NW_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_SESSION_NW_NO_SERVICE_V01 = 0x00, 
  WDS_DATA_SESSION_NW_CDMA_V01 = 0x02, 
  WDS_DATA_SESSION_NW_HDR_V01 = 0x04, 
  WDS_DATA_SESS_NW_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_data_sess_nw_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_service_option_enum_v01 service_option;
  /**<   Packet data call service option before dormancy.
       Values: \n
       - 0x0007 -- IS-657 \n
       - 0x000F -- IS-657 over rate set 2 \n
       - 0x0016 -- IS-707A with rate set 1 forward and reverse \n
       - 0x0019 -- IS-707A with rate set 2 forward and reverse \n
       - 0x0021 -- cdma2000\textsuperscript{\textregistered} packet service option \n
       - 0x1007 -- IS-707 \n
       - 0x8020 -- QC Proprietary, rate set 2 \n
       - -1     -- NULL service option (returned when not currently in
                  CDMA-1X data session)
  */

  wds_data_sess_nw_enum_v01 data_sess_nw;
  /**<   Data session network before dormancy. Values: \n
       - 0x00 -- No service (returned when not currently in 3GPP2 data session) \n
       - 0x02 -- CDMA \n
       - 0x04 -- HDR
  */
}wds_predormancy_cdma_settings_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the packet data session information before
           dormancy. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Predormancy Settings   */
  wds_predormancy_cdma_settings_type_v01 pre_dormancy_settings;
}wds_get_pre_dormancy_cdma_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the Chatty App Manager timer value. */
typedef struct {

  /* Mandatory */
  /*  CAM Timer */
  uint32_t cam_timer;
  /**<   CAM timer value in seconds.*/
}wds_set_cam_timer_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the Chatty App Manager timer value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_cam_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the Chatty App Manager timer value. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_cam_timer_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the Chatty App Manager timer value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  CAM Timer   */
  uint32_t cam_timer;
  /**<   Retrieves the current value of the CAM timer, in 
       seconds.
   */
}wds_get_cam_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Disables/enables the Supplemental Channel Request Message (SCRM). */
typedef struct {

  /* Mandatory */
  /*  SCRM */
  uint8_t scrm;
  /**<   Values: \n
       - 0x00 -- SCRM disabled \n
       - 0x01 -- SCRM enabled
  */
}wds_set_scrm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Disables/enables the Supplemental Channel Request Message (SCRM). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_scrm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves whether SCRM support is enabled or disabled. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_scrm_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves whether SCRM support is enabled or disabled. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  SCRM */
  uint8_t scrm;
  /**<   Values: \n
       - 0x00 -- SCRM disabled \n
       - 0x01 -- SCRM enabled
  */
}wds_get_scrm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Enables or disables reduced dormancy followed by unsolicited data. */
typedef struct {

  /* Mandatory */
  /*  RDUD */
  uint8_t rdud;
  /**<   Values: \n
       - 0x00 -- RDUD disabled \n
       - 0x01 -- RDUD enabled
  */
}wds_set_rdud_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Enables or disables reduced dormancy followed by unsolicited data. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_rdud_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves whether reduced dormancy followed by unsolicited
           data is enabled or disabled. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_rdud_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves whether reduced dormancy followed by unsolicited
           data is enabled or disabled. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  RDUD */
  uint8_t rdud;
  /**<   Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled
  */
}wds_get_rdud_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the SIP/MIP call type. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_sip_mip_call_type_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_SIP_MIP_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CALL_TYPE_SIP_MIP_NOT_UP_V01 = 0x00, 
  WDS_CALL_TYPE_SIP_UP_V01 = 0x01, 
  WDS_CALL_TYPE_MIP_UP_V01 = 0x02, 
  WDS_SIP_MIP_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_sip_mip_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the SIP/MIP call type. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Call Type  */
  wds_sip_mip_call_type_enum_v01 call_type;
  /**<   Values: \n
       - 0x00 -- SIP_MIP not up \n
       - 0x01 -- SIP up \n
       - 0x02 -- MIP up
  */
}wds_get_sip_mip_call_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the EV-DO slot cycle index.  */
typedef struct {

  /* Mandatory */
  /*  EV-DO Page Monitor Period */
  wds_evdo_slot_cycle_enum_v01 evdo_page_monitor_period;
  /**<   If the service receives any value that can not be set, it returns
       QMI_ERR_INVALID_ARG and does not modify the EV-DO page monitor period.
       Using -1 sets the SCI to the default value.
  */
}wds_set_evdo_page_monitor_period_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the EV-DO slot cycle index.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_evdo_page_monitor_period_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_SUCCESS_V01 = 0x00, 
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_FAIL_REQUEST_REJECTED_V01 = 0x01, 
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_FAIL_REQUEST_FAILED_TX_V01 = 0x02, 
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_FAIL_NOT_SUPPORTED_V01 = 0x03, 
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_FAIL_REQUEST_NO_NET_V01 = 0x04, 
  WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_evdo_page_monitor_period_result_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the result of the attempt to change the EV-DO 
           slot cycle. */
typedef struct {

  /* Mandatory */
  /*  EV-DO Slot Cycle Set Result */
  wds_evdo_page_monitor_period_result_enum_v01 status;
  /**<   Values: \n
       0x00 -- SUCCESS \n
       0x01 -- FAIL REQUEST_REJECTED \n
       0x02 -- FAIL REQUEST_FAILED_TX \n
       0x03 -- FAIL NOT_SUPPORTED \n
       0x04 -- FAIL NO_NET
  */
}wds_evdo_page_monitor_period_result_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Enables or disables the EV-DO force long sleep feature. */
typedef struct {

  /* Mandatory */
  /*  Force Long Sleep Setting */
  uint8_t force_evdo_long_sleep;
  /**<   Values: \n
       0 -- Do not force EV-DO long sleep \n
       1 -- Force EV-DO long sleep
  */
}wds_set_evdo_force_long_sleep_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Enables or disables the EV-DO force long sleep feature. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_evdo_force_long_sleep_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves details about the EV-DO page monitoring period. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_evdo_page_monitor_period_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves details about the EV-DO page monitoring period. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  EV-DO Page Monitor Period Details */
  wds_evdo_page_monitor_period_type_v01 evdo_page_monitor_period;
}wds_get_evdo_page_monitor_period_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries whether the system is call throttled and returns
           the remaining throttled delay. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_call_throttle_info_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t hdr_call_throttled_delay;
  /**<   HDR throttled delay; specifies the remaining call 
       throttled delay in seconds. Set to 0 if the system is not call
       throttled.
  */

  uint32_t cdma_call_throttled_delay;
  /**<   CDMA-1X throttled delay; specifies the remaining 
       call throttled delay in seconds. Set to 0 if the system is not 
       call throttled.
  */
}wds_call_throttle_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries whether the system is call throttled and returns
           the remaining throttled delay. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Call Throttled  */
  wds_call_throttle_info_type_v01 call_throttled;
}wds_get_call_throttle_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the Network Service Access Point
           Identifier (NSAPI), based on the access point name. */
typedef struct {

  /* Mandatory */
  /*  APN */
  char apn[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   Access point name.*/
}wds_get_nsapi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the Network Service Access Point
           Identifier (NSAPI), based on the access point name. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  NSAPI   */
  uint32_t nsapi_len;  /**< Must be set to # of elements in nsapi */
  uint8_t nsapi[QMI_WDS_MAX_BEARER_CONTEXT_V01];
  /**<   NSAPI.*/
}wds_get_nsapi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DUN_CTRL_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DUN_CTRL_PREF_DISABLE_V01 = 0x00, 
  WDS_DUN_CTRL_PREF_ENABLE_V01 = 0x01, 
  WDS_DUN_CTRL_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dun_ctrl_pref_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DUN_ALLOW_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DUN_CTRL_ACTION_DENY_V01 = 0x00, 
  WDS_DUN_CTRL_ACTION_ALLOW_V01 = 0x01, 
  WDS_DUN_ALLOW_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dun_allow_pref_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the control point's preference to control the Dial-Up Networking
           (DUN) call requests received by the modem. */
typedef struct {

  /* Mandatory */
  /*  DUN Control Preference  */
  wds_dun_ctrl_pref_enum_v01 dun_control_preference;
  /**<   Values: \n
       - 0x00 -- Relinquish control of DUN calls \n
       - 0x01 -- Exercise control over DUN calls
  */

  /* Optional */
  /*  Allow DUN Calls */
  uint8_t dun_allow_preference_valid;  /**< Must be set to true if dun_allow_preference is being passed */
  wds_dun_allow_pref_enum_v01 dun_allow_preference;
  /**<   Values: \n
       - 0x00 -- Deny subsequent DUN calls by default \n
       - 0x01 -- Allow subsequent DUN calls by default
  */
}wds_set_dun_ctrl_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the control point's preference to control the Dial-Up Networking
           (DUN) call requests received by the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_dun_ctrl_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the status of the DUN call control on the modem. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_dun_ctrl_info_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DUN_CTRL_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DUN_CTRL_STATUS_NOT_SET_V01 = 0x00, 
  WDS_DUN_CTRL_STATUS_SET_V01 = 0x01, 
  WDS_DUN_CTRL_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dun_ctrl_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DUN_CTRL_CONTROL_POINT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DUN_CTRL_CONTROL_POINT_ANOTHER_V01 = 0x00, 
  WDS_DUN_CTRL_CONTROL_POINT_CURRENT_V01 = 0x01, 
  WDS_DUN_CTRL_CONTROL_POINT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dun_ctrl_control_point_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the status of the DUN call control on the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  DUN Control Status   */
  wds_dun_ctrl_status_enum_v01 dun_control_status;
  /**<   Values: \n
       - 0x00 -- DUN control is not enabled by any control point \n
       - 0x01 -- DUN control is enabled
  */

  /* Optional */
  /*  Allow DUN Calls */
  uint8_t allow_preference_valid;  /**< Must be set to true if allow_preference is being passed */
  wds_dun_allow_pref_enum_v01 allow_preference;
  /**<   Values: \n
       - 0x00 -- Deny subsequent DUN calls by default \n
       - 0x01 -- Allow subsequent DUN calls by default
  */

  /* Optional */
  /*  Current Control Point */
  uint8_t current_control_point_valid;  /**< Must be set to true if current_control_point is being passed */
  wds_dun_ctrl_control_point_enum_v01 current_control_point;
  /**<   Set by current control point. Values: \n
       - 0x00 -- Preference is set by another control point \n
       - 0x01 -- Preference is set by current control point
  */

  /* Optional */
  /*  Event Report Mask */
  uint8_t event_report_mask_valid;  /**< Must be set to true if event_report_mask is being passed */
  wds_dun_ctrl_event_report_mask_v01 event_report_mask;
  /**<   Values: \n
       - 0x01 -- Send DUN call completion notifications \n
       - 0x02 -- Send DUN entitlement notifications \n
       - 0x04 -- Send DUN silent redial notifications
  */
}wds_get_dun_ctrl_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the DUN control event report preference for the
           control point. */
typedef struct {

  /* Mandatory */
  /*  Enable DUN Call Notifications */
  uint8_t notify_dun_call;
  /**<   Values: \n
       - 0x00 -- Disable DUN call notifications \n
       - 0x01 -- Enable DUN call notifications
  */

  /* Optional */
  /*  Entitlement Notifications */
  uint8_t notify_entitlement_valid;  /**< Must be set to true if notify_entitlement is being passed */
  uint8_t notify_entitlement;
  /**<   Values: \n
       - 0x00 -- Disable entitlement notifications \n
       - 0x01 -- Enable entitlement notifications
  */

  /* Optional */
  /*  Silent Redial Notifications */
  uint8_t notify_silent_redial_valid;  /**< Must be set to true if notify_silent_redial is being passed */
  uint8_t notify_silent_redial;
  /**<   Values: \n
       - 0x00 -- Disable silent redial notifications \n
       - 0x01 -- Enable silent redial notifications
  */
}wds_set_dun_ctrl_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the DUN control event report preference for the
           control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  Accepted Event Report Mask */
  wds_dun_ctrl_event_report_mask_v01 accepted_event_report_mask;
  /**<   Values: \n
       - 0x01 -- Send DUN call completion notifications \n
       - 0x02 -- Send DUN entitlement notifications \n
       - 0x04 -- Send DUN silent redial notifications
  */
}wds_set_dun_ctrl_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DUN_CTRL_EVENT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DUN_CTRL_EVENT_CALL_NOTIFICATION_V01 = 0x01, 
  WDS_DUN_CTRL_EVENT_CALL_ENTITLEMENT_V01 = 0x02, 
  WDS_DUN_CTRL_EVENT_CALL_SILENT_REDIAL_V01 = 0x03, 
  WDS_DUN_CTRL_EVENT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dun_ctrl_event_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates an event related to a pending DUN call
           request on the modem. */
typedef struct {

  /* Mandatory */
  /*  DUN Control Event */
  wds_dun_ctrl_event_enum_v01 dun_ctrl_event;
  /**<   Values: \n
       - 0x01 -- DUN call notification \n
       - 0x02 -- Entitlement notification \n
       - 0x03 -- Silent redial notification
  */

  /* Optional */
  /*  DUN Call Notification */
  uint8_t dun_call_notification_valid;  /**< Must be set to true if dun_call_notification is being passed */
  wds_dun_allow_pref_enum_v01 dun_call_notification;
  /**<   Values: \n
       - 0x00 -- DUN call denied \n
       - 0x01 -- DUN call allowed
  */

  /* Optional */
  /*  DUN Call Identifier */
  uint8_t dun_call_id_valid;  /**< Must be set to true if dun_call_id is being passed */
  uint8_t dun_call_id;
  /**<   DUN call identifier. */

  /* Optional */
  /*  Previous DUN Attempt Failure Reason */
  uint8_t previous_dun_failure_reason_valid;  /**< Must be set to true if previous_dun_failure_reason is being passed */
  wds_verbose_call_end_reason_type_v01 previous_dun_failure_reason;
}wds_dun_ctrl_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Allows or disallows a pending DUN call request. */
typedef struct {

  /* Mandatory */
  /*  DUN Call Action */
  wds_dun_allow_pref_enum_v01 dun_call_action;
  /**<   Allow DUN calls. Values: \n
       - 0x00 -- Deny DUN call \n
       - 0x01 -- Allow DUN call
  */

  /* Mandatory */
  /*  DUN Call Identifier */
  uint8_t dun_call_id;
  /**<   DUN call identifier. */
}wds_control_pending_dun_call_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Allows or disallows a pending DUN call request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_control_pending_dun_call_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t tmgi[6];
  /**<   TMGI
  */

  uint8_t session_id_valid;
  /**<   Session ID valid flag. Values: \n
       - 0 -- Not valid \n
       - 1 -- Valid
  */

  uint8_t session_id;
  /**<   Session ID. \n
       Note: Valid if the session_id_valid flag is one.
  */
}wds_embms_tmgi_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t earfcn;
  /**<   Frequency (EARFCN) on which the TMGI to be activated is available.
   */
}wds_embms_earfcn_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_PRIORITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_PRIORITY_0_V01 = 0, 
  WDS_EMBMS_PRIORITY_1_V01 = 1, 
  WDS_EMBMS_PRIORITY_2_V01 = 2, 
  WDS_EMBMS_PRIORITY_3_V01 = 3, 
  WDS_EMBMS_PRIORITY_4_V01 = 4, 
  WDS_EMBMS_PRIORITY_5_V01 = 5, 
  WDS_EMBMS_PRIORITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_priority_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_EXTENDED_ERROR_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_EEC_TMGI_NOT_SUPPORTED_V01 = 108, 
  WDS_EMBMS_ECC_TMGI_ACTIVATION_IN_PROGRESS_V01 = 111, 
  WDS_EMBMS_EEC_TMGI_INVALID_V01 = 124, 
  WDS_EMBMS_EEC_TMGI_DEACTIVATION_IN_PROGRESS_V01 = 203, 
  WDS_EMBMS_EXTENDED_ERROR_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_extended_error_code_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Activates the eMBMS Temporary Mobile Group Identity (TMGI). */
typedef struct {

  /* Mandatory */
  /*  Temporary Mobile Group Identity */
  wds_embms_tmgi_type_v01 tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request (default is -1).
  */

  /* Optional */
  /*  Preemption Priority */
  uint8_t preempt_priority_valid;  /**< Must be set to true if preempt_priority is being passed */
  wds_embms_priority_enum_v01 preempt_priority;
  /**<   Preemption priority of the TMGI to be activated: \n
       - 0 -- priority 0 (default) \n
       - 1 -- priority 1 \n
       - 2 -- priority 2 \n
       - 3 -- priority 3 \n
       - 4 -- priority 4 \n
       - 5 -- priority 5 (highest)
  */

  /* Optional */
  /*  Frequencies List */
  uint8_t earfcn_list_valid;  /**< Must be set to true if earfcn_list is being passed */
  uint32_t earfcn_list_len;  /**< Must be set to # of elements in earfcn_list */
  wds_embms_earfcn_type_v01 earfcn_list[QMI_WDS_EMBMS_MAX_EARFCN_V01];
  /**<   \n Default is an empty list. */

  /* Optional */
  /*  SAI List */
  uint8_t sai_list_valid;  /**< Must be set to true if sai_list is being passed */
  uint32_t sai_list_len;  /**< Must be set to # of elements in sai_list */
  uint32_t sai_list[QMI_WDS_EMBMS_SAI_LIST_MAX_V01];
  /**<   Service area identity list. */

  /* Optional */
  /*  Extended Frequencies List */
  uint8_t earfcn_list_ex_valid;  /**< Must be set to true if earfcn_list_ex is being passed */
  uint32_t earfcn_list_ex_len;  /**< Must be set to # of elements in earfcn_list_ex */
  uint32_t earfcn_list_ex[QMI_WDS_EMBMS_MAX_EARFCN_V01];
  /**<   Default is an empty list. */

  /* Optional */
  /*  Multicast Address IP Family Type */
  uint8_t multicast_ip_type_valid;  /**< Must be set to true if multicast_ip_type is being passed */
  wds_ip_family_enum_v01 multicast_ip_type;
  /**<   IP address family type of the multicast IP 
 corresponding to the requested TMGI. Values: \n
      - WDS_IP_FAMILY_IPV4 (0x04) --  IPv4 \n 
      - WDS_IP_FAMILY_IPV6 (0x06) --  IPv6  
 */

  /* Optional */
  /*  Multicast IPv4 Address */
  uint8_t multicast_ipv4_addr_valid;  /**< Must be set to true if multicast_ipv4_addr is being passed */
  uint32_t multicast_ipv4_addr;
  /**<   Multicast IPv4 address corresponding to the requested TMGI. */

  /* Optional */
  /*  Multicast IPv6 Address */
  uint8_t multicast_ipv6_addr_valid;  /**< Must be set to true if multicast_ipv6_addr is being passed */
  uint8_t multicast_ipv6_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Multicast IPv6 address corresponding to the requested TMGI. */

  /* Optional */
  /*  Multicast Port */
  uint8_t multicast_port_valid;  /**< Must be set to true if multicast_port is being passed */
  uint16_t multicast_port;
  /**<   Multicast port corresponding to the requested TMGI. */
}wds_embms_tmgi_activate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Activates the eMBMS Temporary Mobile Group Identity (TMGI). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_embms_extended_error_code_enum_v01 extended_error_code;
  /**<   Values: \n
       - 111 -- Activation is in progress \n
       - 203 -- Deactivation is in progress
  */
}wds_embms_tmgi_activate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_TMGI_ACTIVATE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TMGI_ACTIVATE_SUCCESS_V01 = 0x00000000, 
  WDS_TMGI_ACTIVATE_SUCCESS_DUPLICATE_V01 = 0x00000001, 
  WDS_TMGI_ACTIVATE_SUCCESS_IDLE_RADIO_TUNE_V01 = 0x00000002, 
  WDS_TMGI_ACTIVATE_SUCCESS_CONN_RADIO_TUNE_V01 = 0x00000003, 
  WDS_TMGI_ACTIVATE_FAILURE_UNKNOWN_V01 = 0x00010004, 
  WDS_TMGI_ACTIVATE_FAILURE_NOT_ALLOWED_V01 = 0x00010005, 
  WDS_TMGI_ACTIVATE_FAILURE_MISSING_CONTROL_INFO_V01 = 0x00010006, 
  WDS_TMGI_ACTIVATE_FAILURE_MISSING_TMGI_V01 = 0x00010007, 
  WDS_TMGI_ACTIVATE_FAILURE_MCAST_OOS_V01 = 0x00010008, 
  WDS_TMGI_ACTIVATE_FAILURE_UCAST_OOS_V01 = 0x00010009, 
  WDS_TMGI_ACTIVATE_FAILURE_CAMPED_ON_OTHER_FREQ_V01 = 0x0001000A, 
  WDS_TMGI_ACTIVATE_FAILURE_SAI_MISMATCH_V01 = 0x00010107, 
  WDS_TMGI_ACTIVATION_FAILURE_MAX_TMGI_ALREADY_ACTIVE_V01 = 0x00010108, 
  WDS_EMBMS_TMGI_ACTIVATE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_tmgi_activate_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the result of the TMGI activate request. */
typedef struct {

  /* Mandatory */
  /*  TMGI Activation Status */
  wds_embms_tmgi_activate_status_enum_v01 activate_status;
  /**<   Values: \n
       - 0x00000000 -- Success \n
       - 0x00000001 -- Success -- Duplicate activate \n
       - 0x00010000 -- Failure -- Radio configuration \n
       - 0x00010001 -- Failure -- Channel is unavailable \n
       - 0x00010002 -- Failure -- eMBMS is not enabled \n
       - 0x00010003 -- Failure -- Out of coverage \n
       - 0x00010004 -- Failure -- Unknown \n
       - 0x00010005 -- Failure -- Not allowed \n
       - 0x00010006 -- Failure -- Missing control information \n
       - 0x00010007 -- Failure -- Missing TMGI \n
       - 0x00010008 -- Failure -- Multicast OOS \n
       - 0x00010009 -- Failure -- Unicast OOS \n
       - 0x0001000A -- Failure -- Camped on another frequency
  */

  /* Mandatory */
  /*  TMGI */
  wds_embms_tmgi_type_v01 tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request.
  */
}wds_embms_tmgi_activate_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Deactivates an eMBMS TMGI. */
typedef struct {

  /* Mandatory */
  /*  Temporary Mobile Group Identity */
  wds_embms_tmgi_type_v01 tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request (default is -1).
  */
}wds_embms_tmgi_deactivate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Deactivates an eMBMS TMGI. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_embms_extended_error_code_enum_v01 extended_error_code;
  /**<   Values: \n
       - 108 -- Not supported; the TMGI is not activated by this control point \n
       - 124 -- Invalid; the TMGI is not activated \n
       - 203 -- Duplicate request, deactivation is in progress
  */
}wds_embms_tmgi_deactivate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_TMGI_DEACTIVATE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TMGI_DEACTIVATE_SUCCESS_V01 = 0x00000000, 
  WDS_EMBMS_TMGI_DEACTIVATE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_tmgi_deactivate_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the result of the TMGI deactivate request. */
typedef struct {

  /* Mandatory */
  /*  TMGI Deactivation Status */
  wds_embms_tmgi_deactivate_status_enum_v01 deactivate_status;
  /**<   Value: \n
       - 0x00000000 -- Success
  */

  /* Mandatory */
  /*  TMGI */
  wds_embms_tmgi_type_v01 tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request.
  */
}wds_embms_tmgi_deactivate_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_TMGI_LIST_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_TMGI_LIST_ACTIVE_V01 = 0x00, 
  WDS_EMBMS_TMGI_LIST_AVAILABLE_V01 = 0x01, 
  WDS_EMBMS_TMGI_LIST_OOS_WARNING_V01 = 0x02, 
  WDS_EMBMS_TMGI_LIST_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_tmgi_list_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_OOS_WARNING_REASON_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_WARN_REASON_UCAST_OOS_V01 = 0, 
  WDS_EMBMS_WARN_REASON_MCAST_OOS_V01 = 1, 
  WDS_EMBMS_WARN_REASON_OOS_CLEARED_V01 = 2, 
  WDS_EMBMS_OOS_WARNING_REASON_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_oos_warning_reason_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the TMGI list. */
typedef struct {

  /* Mandatory */
  /*  TMGI List Type */
  wds_embms_tmgi_list_type_enum_v01 list_type;
  /**<   Values: \n
       - 0x00 -- Active TMGI list \n
       - 0x01 -- Available TMGI list \n
       - 0x02 -- OOS warning TMGI list
  */

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request (default is -1).
  */
}wds_embms_tmgi_list_query_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_embms_tmgi_list_type_enum_v01 list_type;
  /**<   TMGI list type. Values: \n
       - 0x00 -- Active TMGI list \n
       - 0x01 -- Available TMGI list \n
       - 0x02 -- OOS warning TMGI list
  */

  uint32_t tmgi_list_len;  /**< Must be set to # of elements in tmgi_list */
  wds_embms_tmgi_type_v01 tmgi_list[QMI_WDS_EMBMS_MAX_TMGI_V01];
}wds_embms_tmgi_list_struct_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the TMGI list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  TMGI List */
  uint8_t tmgi_list_valid;  /**< Must be set to true if tmgi_list is being passed */
  wds_embms_tmgi_list_struct_type_v01 tmgi_list;

  /* Optional */
  /*  OOS Warning Reason */
  uint8_t warn_reason_valid;  /**< Must be set to true if warn_reason is being passed */
  wds_embms_oos_warning_reason_enum_v01 warn_reason;
  /**<   Values: \n
       - 0x00 -- Warning for unicast OOS \n
       - 0x01 -- Warning for multicast OOS \n
       - 0x02 -- Warning cleared
  */
}wds_embms_tmgi_list_query_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the currently active or available TMGI list. */
typedef struct {

  /* Optional */
  /*  TMGI List */
  uint8_t tmgi_list_valid;  /**< Must be set to true if tmgi_list is being passed */
  wds_embms_tmgi_list_struct_type_v01 tmgi_list;

  /* Optional */
  /*  OOS Warning Reason */
  uint8_t warn_reason_valid;  /**< Must be set to true if warn_reason is being passed */
  wds_embms_oos_warning_reason_enum_v01 warn_reason;
  /**<   Values: \n
       - 0x00 -- Warning for unicast OOS \n
       - 0x01 -- Warning for multicast OOS \n
       - 0x02 -- Warning cleared
  */

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the indication.
  */
}wds_embms_tmgi_list_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the preferred data system. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_preferred_data_system_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the preferred data system. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Current Preferred Data System */
  uint8_t current_sys_valid;  /**< Must be set to true if current_sys is being passed */
  wds_curr_pref_data_sys_enum_v01 current_sys;
  /**<   Values: \n
       - 0x00 -- Unknown \n
       - 0x01 -- CMDA_1X \n
       - 0x02 -- EVDO \n
       - 0x03 -- GPRS \n
       - 0x04 -- WCDMA \n
       - 0x05 -- LTE \n
       - 0x06 -- TDSCDMA
  */
}wds_get_preferred_data_system_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the last reported data call status. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_last_data_call_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_event_data_call_type_enum_v01 data_call_type;
  /**<   Values: \n
       - 0x00 -- Unknown \n
       - 0x01 -- Embedded call (application) \n
       - 0x02 -- Tethered call \n
       - 0x03 -- Modem embedded call
  */

  wds_event_tethered_call_type_enum_v01 tethered_call_type;
  /**<   Values: \n
       - 0x00 -- Non-tethered call \n
       - 0x01 -- RmNet call \n
       - 0x02 -- DUN call
  */
}wds_get_data_call_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the last reported data call status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Data Call Status */
  uint8_t data_call_status_valid;  /**< Must be set to true if data_call_status is being passed */
  wds_data_call_status_change_enum_v01 data_call_status;
  /**<   Values: \n
       - 0x00 -- Unknown \n
       - 0x01 -- Activated \n
       - 0x02 -- Terminated
  */

  /* Optional */
  /*  Data Call Type */
  uint8_t data_call_type_valid;  /**< Must be set to true if data_call_type is being passed */
  wds_get_data_call_type_v01 data_call_type;

  /* Optional */
  /*  Data Call Address Family */
  uint8_t data_call_addr_family_valid;  /**< Must be set to true if data_call_addr_family is being passed */
  wds_data_call_addr_family_enum_v01 data_call_addr_family;
  /**<   Data call address family. Values: \n
       - 0 -- Unknown \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */
}wds_get_last_data_call_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current data system status. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_current_data_system_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current data system status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Data Sytem Status */
  uint8_t system_status_valid;  /**< Must be set to true if system_status is being passed */
  wds_data_system_status_type_v01 system_status;
}wds_get_current_data_system_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_TECH_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TECH_TYPE_3GPP_V01 = 0x00, 
  WDS_TECH_TYEP_3GPP2_V01 = 0x01, 
  WDS_TECH_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_tech_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the PDN throttle information. */
typedef struct {

  /* Mandatory */
  /*  Technology Type */
  wds_tech_type_enum_v01 tech_type;
  /**<   Values: \n
       - 0 -- 3GPP \n
       - 1 -- 3GPP2
  */
}wds_get_pdn_throttle_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_ipv4_throttled;
  /**<   Values: \n
       - 0 -- IPv4 not throttled \n
       - 1 -- IPv4 throttled
  */

  uint8_t is_ipv6_throttled;
  /**<   Values: \n
  - 0 -- IPv6 not throttled \n
       - 1 -- IPv6 throttled
  */

  uint32_t remaining_ipv4_throttled_time;
  /**<   Remaining IPv4 throttled time in milliseconds.
  */

  uint32_t remaining_ipv6_throttled_time;
  /**<   Remaining IPv6 throttled time in milliseconds.
  */

  uint32_t apn_string_len;  /**< Must be set to # of elements in apn_string */
  char apn_string[QMI_WDS_APN_NAME_MAX_V01];
  /**<   APN name.
  */
}wds_pdn_throttle_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the PDN throttle information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  PDN Throttle Info */
  uint8_t throttle_info_valid;  /**< Must be set to true if throttle_info is being passed */
  uint32_t throttle_info_len;  /**< Must be set to # of elements in throttle_info */
  wds_pdn_throttle_info_type_v01 throttle_info[QMI_WDS_APNS_MAX_V01];
}wds_get_pdn_throttle_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the maximum number of attached PDNs supported. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_lte_max_attach_pdn_num_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the maximum number of attached PDNs supported. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Maximum Number of Attach PDNs Supported */
  uint8_t max_attach_pdn_num_valid;  /**< Must be set to true if max_attach_pdn_num is being passed */
  uint8_t max_attach_pdn_num;
  /**<   Maximum number of attached PDNs supported by the device. */
}wds_get_lte_max_attach_pdn_num_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the LTE attach PDN list. */
typedef struct {

  /* Mandatory */
  /*  Attach PDN List */
  uint32_t attach_pdn_list_len;  /**< Must be set to # of elements in attach_pdn_list */
  uint16_t attach_pdn_list[QMI_WDS_ATTACH_PDN_MAX_V01];
  /**<   PDN profile IDs to attach to, listed in order
       of decreasing priority. 
  */
}wds_set_lte_attach_pdn_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the LTE attach PDN list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_lte_attach_pdn_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the attach PDN list. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_lte_attach_pdn_list_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the attach PDN list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Attach PDN List */
  uint8_t attach_pdn_list_valid;  /**< Must be set to true if attach_pdn_list is being passed */
  uint32_t attach_pdn_list_len;  /**< Must be set to # of elements in attach_pdn_list */
  uint16_t attach_pdn_list[QMI_WDS_ATTACH_PDN_MAX_V01];
  /**<   PDN profile IDs to attach to, listed in order
       of decreasing priority. 
    */
}wds_get_lte_attach_pdn_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the list of LTE attach PDNs. */
typedef struct {

  /* Optional */
  /*  Changed LTE Attach PDN List */
  uint8_t attach_pdn_list_valid;  /**< Must be set to true if attach_pdn_list is being passed */
  uint32_t attach_pdn_list_len;  /**< Must be set to # of elements in attach_pdn_list */
  uint16_t attach_pdn_list[QMI_WDS_ATTACH_PDN_MAX_V01];
  /**<   PDN profile IDs to attach to, listed in order
       of decreasing priority.
    */
}wds_lte_attach_pdn_list_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries LTE attach PDN parameters. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_lte_attach_params_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries LTE attach PDN parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  APN String */
  uint8_t apn_string_valid;  /**< Must be set to true if apn_string is being passed */
  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String representing the APN.
       Maximum length is 100 bytes.
  */

  /* Optional */
  /*  IP Support Type */
  uint8_t ip_type_valid;  /**< Must be set to true if ip_type is being passed */
  wds_ip_support_type_enum_v01 ip_type;
  /**<   Values: \n
       - 0 -- IPv4 \n
       - 1 -- IPv6 \n
       - 2 -- IPv4v6
  */

  /* Optional */
  /*  Over the Air Attach Performed */
  uint8_t ota_attach_performed_valid;  /**< Must be set to true if ota_attach_performed is being passed */
  uint8_t ota_attach_performed;
  /**<   Values: \n
       - 0 -- Over-the-air attach not performed \n
       - 1 -- Over-the-air attach performed
  */
}wds_get_lte_attach_params_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Resets the packet data transfer statistics. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_reset_pkt_statistics_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Resets the packet data transfer statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_reset_pkt_statistics_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current data call flow control status */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_flow_control_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current data call flow control status */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Uplink Flow Control */
  uint8_t uplink_flow_control_valid;  /**< Must be set to true if uplink_flow_control is being passed */
  uint8_t uplink_flow_control;
  /**<   Uplink flow control status. Values: \n
       - 0 -- Not flow controlled \n
       - 1 -- Flow controlled
   */
}wds_get_flow_control_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Activates and deactivates TMGIs. */
typedef struct {

  /* Mandatory */
  /*  TMGI To Activate */
  wds_embms_tmgi_type_v01 act_tmgi;

  /* Mandatory */
  /*  TMGI To Deactivate */
  wds_embms_tmgi_type_v01 deact_tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request (default is -1).
  */

  /* Optional */
  /*  Preemption Priority */
  uint8_t preempt_priority_valid;  /**< Must be set to true if preempt_priority is being passed */
  wds_embms_priority_enum_v01 preempt_priority;
  /**<   Preemption priority of the TMGI to be activated: \n
       - 0 -- priority 0 (default) \n
       - 1 -- priority 1 \n
       - 2 -- priority 2 \n
       - 3 -- priority 3 \n
       - 4 -- priority 4 \n
       - 5 -- priority 5 (highest)
  */

  /* Optional */
  /*  Frequencies List */
  uint8_t earfcn_list_valid;  /**< Must be set to true if earfcn_list is being passed */
  uint32_t earfcn_list_len;  /**< Must be set to # of elements in earfcn_list */
  wds_embms_earfcn_type_v01 earfcn_list[QMI_WDS_EMBMS_MAX_EARFCN_V01];
  /**<   \n Default is an empty list.
  */

  /* Optional */
  /*  SAI List */
  uint8_t sai_list_valid;  /**< Must be set to true if sai_list is being passed */
  uint32_t sai_list_len;  /**< Must be set to # of elements in sai_list */
  uint32_t sai_list[QMI_WDS_EMBMS_SAI_LIST_MAX_V01];
  /**<   Default is an empty list. */

  /* Optional */
  /*  Extended Frequencies List */
  uint8_t earfcn_list_ex_valid;  /**< Must be set to true if earfcn_list_ex is being passed */
  uint32_t earfcn_list_ex_len;  /**< Must be set to # of elements in earfcn_list_ex */
  uint32_t earfcn_list_ex[QMI_WDS_EMBMS_MAX_EARFCN_V01];
  /**<   Default is an empty list.
  */

  /* Optional */
  /*  Multicast Address IP Family Type */
  uint8_t multicast_ip_type_valid;  /**< Must be set to true if multicast_ip_type is being passed */
  wds_ip_family_enum_v01 multicast_ip_type;
  /**<   IP address family type of the multicast IP 
 corresponding to the requested TMGI. Values: \n
      - WDS_IP_FAMILY_IPV4 (0x04) --  IPv4 \n 
      - WDS_IP_FAMILY_IPV6 (0x06) --  IPv6  
 */

  /* Optional */
  /*  Multicast IPv4 Address */
  uint8_t multicast_ipv4_addr_valid;  /**< Must be set to true if multicast_ipv4_addr is being passed */
  uint32_t multicast_ipv4_addr;
  /**<   Multicast IPv4 address corresponding to the requested TMGI. */

  /* Optional */
  /*  Multicast IPv6 Address */
  uint8_t multicast_ipv6_addr_valid;  /**< Must be set to true if multicast_ipv6_addr is being passed */
  uint8_t multicast_ipv6_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Multicast IPv6 address corresponding to the requested TMGI. */

  /* Optional */
  /*  Multicast Port */
  uint8_t multicast_port_valid;  /**< Must be set to true if multicast_port is being passed */
  uint16_t multicast_port;
  /**<   Multicast port corresponding to the requested TMGI. */
}wds_embms_tmgi_act_deact_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Activates and deactivates TMGIs. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Extended Error Code */
  uint8_t extended_error_code_valid;  /**< Must be set to true if extended_error_code is being passed */
  wds_embms_extended_error_code_enum_v01 extended_error_code;
  /**<   Values: \n
       - 108 -- Not supported; the TMGI to be deactivated is not activated
                by this control point \n
       - 111 -- Activation is in progress \n
       - 124 -- Invalid; the TMGI to be deactivated is not activated \n
       - 203 -- Deactivation is in progress
  */
}wds_embms_tmgi_act_deact_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the result of the TMGI activation and deactivation request. */
typedef struct {

  /* Mandatory */
  /*  TMGI Activation Status */
  wds_embms_tmgi_activate_status_enum_v01 act_status;
  /**<   Values: \n
       - 0x00000000 -- Success \n
       - 0x00000001 -- Success -- Duplicate activate \n
       - 0x00010000 -- Failure -- Radio configuration \n
       - 0x00010001 -- Failure -- Channel is unavailable \n
       - 0x00010002 -- Failure -- eMBMS is not enabled \n
       - 0x00010003 -- Failure -- Out of coverage \n
       - 0x00010004 -- Failure -- Unknown \n
       - 0x00010005 -- Failure -- Not allowed \n
       - 0x00010006 -- Failure -- Missing control information \n
       - 0x00010007 -- Failure -- Missing TMGI \n
       - 0x00010008 -- Failure -- Multicast OOS \n
       - 0x00010009 -- Failure -- Unicast OOS \n
       - 0x0001000A -- Failure -- Camped on another frequency
  */

  /* Mandatory */
  /*  Activation TMGI */
  wds_embms_tmgi_type_v01 act_tmgi;

  /* Mandatory */
  /*  TMGI Deactivation Status */
  wds_embms_tmgi_deactivate_status_enum_v01 deact_status;
  /**<   Values: \n
       - 0x00000000 -- Success
  */

  /* Mandatory */
  /*  Deactivation TMGI */
  wds_embms_tmgi_type_v01 deact_tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request.
  */
}wds_embms_tmgi_act_deact_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Binds a control point to an SIO data port. */
typedef struct {

  /* Mandatory */
  /*  Binding Data Port */
  uint16_t data_port;
  /**<   SIO data port to which the client binds.
  */
}wds_bind_data_port_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Binds a control point to an SIO data port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_bind_data_port_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_tech_pref_mask_v01 tech_pref;
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
}wds_reqd_net_policy_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PROTOCOL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_PROTO_TCP_V01 = 0x01, /**<  Transmission Control Protocol  */
  WDS_PROTO_UDP_V01 = 0x02, /**<  User Datagram Protocol  */
  WDS_PROTO_TCP_UDP_V01 = 0x03, /**<  Transmission Control Protocol/User Datagram Protocol  */
  WDS_PROTOCOL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_protocol_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t port;
  /**<   TCP, UDP, and TCP_UDP source port. */

  uint16_t port_range;
  /**<   Port range. 
  */
}wds_tcp_udp_port_range_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the filter to allow multiple PDNs to be shared on the same 
           data port. */
typedef struct {

  /* Mandatory */
  /*  IP Version */
  wds_ip_version_enum_v01 ip_version;
  /**<   IP version number. Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6
  */

  /* Mandatory */
  /*  Network Policy */
  wds_reqd_net_policy_info_v01 net_policy_info;
  /**<   \n
       Information that is required to identify the NAT interface, which is 
       required to support the sharing of PDNs on a single RmNet port.  
  */

  /* Optional */
  /*  Next Header Protocol */
  uint8_t next_hdr_prot_valid;  /**< Must be set to true if next_hdr_prot is being passed */
  wds_protocol_enum_v01 next_hdr_prot;
  /**<   IPv4/IPv6 next header protocol after the IP header. Values: \n
      - WDS_PROTO_TCP (0x01) --  Transmission Control Protocol 
      - WDS_PROTO_UDP (0x02) --  User Datagram Protocol 
      - WDS_PROTO_TCP_UDP (0x03) --  Transmission Control Protocol/User Datagram Protocol  
 */

  /* Optional */
  /*  TCP/UDP Source */
  uint8_t tcp_udp_src_valid;  /**< Must be set to true if tcp_udp_src is being passed */
  wds_tcp_udp_port_range_v01 tcp_udp_src;
  /**<   \n
       Contains the starting port number and a range value, which 
       indicates the ending port number. */
}wds_set_additional_pdn_filter_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the filter to allow multiple PDNs to be shared on the same 
           data port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Filter Handle  */
  uint8_t filter_handle_valid;  /**< Must be set to true if filter_handle is being passed */
  uint32_t filter_handle;
  /**<   Filter handle. 
  */
}wds_set_additional_pdn_filter_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Removes the filter that was set to allow additional PDNs to be 
           shared on a single port. */
typedef struct {

  /* Mandatory */
  /*  Filter Handle */
  uint32_t filter_handle;
  /**<   Filter handle. 
  */
}wds_remove_additional_pdn_filter_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Removes the filter that was set to allow additional PDNs to be 
           shared on a single port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_remove_additional_pdn_filter_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in any of the IP configuration of the data session. */
typedef struct {

  /* Optional */
  /*  Changed IP Configuration */
  uint8_t changed_ip_config_valid;  /**< Must be set to true if changed_ip_config is being passed */
  wds_req_settings_mask_v01 changed_ip_config;
  /**<   Set bits to 1, corresponding to configuration changed. Values: \n
      - Bit 4  -- DNS address \n
      - Bit 9  -- Gateway information (address and subnet mask) \n
      - Bit 10 -- PCSCF address using PCO flag \n
      - Bit 11 -- PCSCF server address list \n
      - Bit 12 -- PCSCF domain name list \n
      - Bit 13 -- MTU \n
      - Bit 14 -- Domain name list \n
      - Bit 18 -- Operator reserved PCO 
   */
}wds_extended_ip_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Registration mechanism for indications relevant to reverse IP  
           transport connections. */
typedef struct {

  /* Mandatory */
  /*  Register for Indication */
  uint8_t register_for_ind;
  /**<   Values: \n
       - 0 -- Deregister for the indication \n
       - 1 -- Register for the indication
  */
}wds_reverse_ip_transport_connection_ind_registration_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Registration mechanism for indications relevant to reverse IP  
           transport connections. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_reverse_ip_transport_connection_ind_registration_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_REVERSE_IP_TRANSPORT_CONNECTION_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_REVERSE_IP_TRANSPORT_DISCONNECTED_V01 = 0x00, /**<  Disconnected  */
  WDS_REVERSE_IP_TRANSPORT_CONNECTED_V01 = 0x01, /**<  Connected  */
  WDS_REVERSE_IP_TRANSPORT_CONNECTION_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_reverse_ip_transport_connection_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current reverse IP transport connection 
           status. */
typedef struct {

  /* Mandatory */
  /*  Reverse IP Transport Connection Status */
  wds_reverse_ip_transport_connection_status_enum_v01 rev_ip_transport_conn_status;
  /**<   Values: \n
      - WDS_REVERSE_IP_TRANSPORT_DISCONNECTED (0x00) --  Disconnected 
      - WDS_REVERSE_IP_TRANSPORT_CONNECTED (0x01) --  Connected 
 */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Transaction ID for this specific indication.
   */

  /* Optional */
  /*  Technology Name */
  uint8_t tech_name_valid;  /**< Must be set to true if tech_name is being passed */
  wds_technology_name_enum_v01 tech_name;
  /**<   Technology name of the reverse IP transport data connection. Values: \n
       - -32736 -- WLAN_LOCAL_BRKOUT \n
       - -32735 -- IWLAN_S2B \n
      WLAN_LOCAL_BRKOUT is an interface for transferring data between entities
      on the AP and modem. It can be used either for local breakout 
      calls, or for IPSec signaling for ePDG calls.

      IWLAN_S2B is an interface for transferring data between entities
      on the AP and modem for ePDG calls. 
  */

  /* Optional */
  /*  Is Shared */
  uint8_t is_shared_valid;  /**< Must be set to true if is_shared is being passed */
  uint8_t is_shared;
  /**<   Indicates whether the static SAs are shared between IPv4 and IPv6. Values: \n
       - 0 -- Not shared \n
       - 1 -- Shared
  */

  /* Optional */
  /*  IPv4 Address */
  uint8_t ipv4_addr_valid;  /**< Must be set to true if ipv4_addr is being passed */
  uint32_t ipv4_addr;
  /**<   Provides the IPv4 address for the WLAN local breakout or ePDG connection. 
   */

  /* Optional */
  /*  IPv4 Subnet Mask */
  uint8_t ipv4_subnet_mask_valid;  /**< Must be set to true if ipv4_subnet_mask is being passed */
  uint32_t ipv4_subnet_mask;
  /**<   IPv4 subnet mask. */

  /* Optional */
  /*  IPv6 Address */
  uint8_t ipv6_addr_valid;  /**< Must be set to true if ipv6_addr is being passed */
  wds_runtime_ipv6_addr_type_v01 ipv6_addr;
  /**<   \n Provides the IPv6 address for the WLAN local breakout or ePDG connection. 
  */

  /* Optional */
  /*  MTU  */
  uint8_t mtu_valid;  /**< Must be set to true if mtu is being passed */
  uint32_t mtu;
  /**<   MTU size. */
}wds_reverse_ip_transport_connection_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves IPSec static Security Associations (SA) for the ePDG call. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_ipsec_static_sa_config_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IPSEC_SA_PROTOCOL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IPSEC_SA_PROTOCOL_UNDEFINED_V01 = 0x00, /**<  Undefined \n  */
  WDS_IPSEC_SA_PROTOCOL_ISAKMP_V01 = 0x01, /**<  Internet Security Association and Key Management Protocol \n  */
  WDS_IPSEC_SA_PROTOCOL_IPSEC_AH_V01 = 0x02, /**<  Authentication header \n  */
  WDS_IPSEC_SA_PROTOCOL_IPSEC_ESP_V01 = 0x03, /**<  Encapsulating security payload  */
  WDS_IPSEC_SA_PROTOCOL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ipsec_sa_protocol_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IPSEC_SA_ENCAPSULATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IPSEC_SA_ENCAPSULATE_UNDEFINED_V01 = 0x00, /**<  Undefined \n  */
  WDS_IPSEC_SA_ENCAPSULATE_TUNNEL_V01 = 0x01, /**<  Encapsulation mode tunnel \n  */
  WDS_IPSEC_SA_ENCAPSULATE_TRANSPORT_V01 = 0x02, /**<  Encapsulation mode transport   */
  WDS_IPSEC_SA_ENCAPSULATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ipsec_sa_encapsulate_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_data_call_addr_family_enum_v01 addr_family;
  /**<   Values: \n
       - 4 -- IPv4 \n
       - 6 -- IPv6 
   */

  uint32_t ip_addr_len;  /**< Must be set to # of elements in ip_addr */
  uint8_t ip_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Contains either the IPv4 or IPv6 address based on the value of 
       addr_family.
   */
}wds_ip_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IPSEC_CRYPTO_ALGO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IPSEC_CRYPTO_ALGO_NULL_KEY_X_V01 = 0x00, /**<  No key exchange protocol used \n  */
  WDS_IPSEC_CRYPTO_ALGO_MODEXP_V01 = 0x01, /**<  Generic modulo exponentiation \n  */
  WDS_IPSEC_CRYPTO_ALGO_DIFFIE_HELLMAN_V01 = 0x02, /**<  Diffie-Hellman key exchange protocol \n  */
  WDS_IPSEC_CRYPTO_ALGO_RSA_V01 = 0x03, /**<  RSA encryption/key exchange protocol \n  */
  WDS_IPSEC_CRYPTO_ALGO_NULL_HASH_V01 = 0x04, /**<  No simultaneous hash with encryption-decryption \n  */
  WDS_IPSEC_CRYPTO_ALGO_SHA_V01 = 0x05, /**<  SHA-1 hash function \n  */
  WDS_IPSEC_CRYPTO_ALGO_SHA256_V01 = 0x06, /**<  SHA-256 hash function \n  */
  WDS_IPSEC_CRYPTO_ALGO_MD5_V01 = 0x07, /**<  MD5 hash function \n  */
  WDS_IPSEC_CRYPTO_ALGO_NULL_CIPHER_V01 = 0x08, /**<  No encryption selected \n  */
  WDS_IPSEC_CRYPTO_ALGO_DES_V01 = 0x09, /**<  DES encryption/decryption function \n  */
  WDS_IPSEC_CRYPTO_ALGO_3DES_V01 = 0x0A, /**<  3DES encryption/decryption function \n  */
  WDS_IPSEC_CRYPTO_ALGO_ARC4_V01 = 0x0B, /**<  ARC4 encryption/decryption function \n  */
  WDS_IPSEC_CRYPTO_ALGO_AES128_V01 = 0x0C, /**<  AES cipher; 128-bit key \n  */
  WDS_IPSEC_CRYPTO_ALGO_C2_V01 = 0x0D, /**<  C2 cipher \n  */
  WDS_IPSEC_CRYPTO_ALGO_AES128_CMAC_V01 = 0x0E, /**<  128-bit AES in CMAC mode \n  */
  WDS_IPSEC_CRYPTO_ALGO_SNOW3G_UEA2_V01 = 0x0F, /**<  UEA2 SNOW 3G encryption algorithm \n  */
  WDS_IPSEC_CRYPTO_ALGO_SNOW3G_UIA2_V01 = 0x10, /**<  UIA2 SNOW 3G integrity algorithm \n  */
  WDS_IPSEC_CRYPTO_ALGO_ZUC_CIPHER_V01 = 0x11, /**<  UEA3 ZUC encryption algorithm \n  */
  WDS_IPSEC_CRYPTO_ALGO_ZUC_INTEGRITY_V01 = 0x12, /**<  UIA3 ZUC integrity algorithm \n  */
  WDS_IPSEC_CRYPTO_ALGO_AES256_V01 = 0x13, /**<  AES cipher; 256-bit key \n  */
  WDS_IPSEC_CRYPTO_ALGO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ipsec_crypto_algo_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IPSEC_AES_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IPSEC_AES_MODE_MIN_V01 = 0, /**<  No AES Mode \n  */
  WDS_IPSEC_AES_MODE_CBC_V01 = 1, /**<  AES Mode CBC \n  */
  WDS_IPSEC_AES_MODE_CTR_V01 = 2, /**<  AES Mode CTR \n  */
  WDS_IPSEC_AES_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ipsec_aes_mode_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ipv4_subnet_address;
  /**<   IPv4 subnet address. */

  uint32_t subnet_mask;
  /**<   Subnet mask. */
}wds_cfg_attr_ipv4_subnet_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ipv6_address[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address. */
}wds_cfg_attr_ipv6_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t protocol;
  /**<   Protocol.*/

  uint16_t start_port;
  /**<   Start port.*/

  uint16_t end_port;
  /**<   End port.*/

  wds_ip_addr_type_v01 start_address;
  /**<   Start address. */

  wds_ip_addr_type_v01 end_address;
  /**<   End address. */
}wds_traffic_selector_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves IPSec static Security Associations (SA) for the ePDG call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Security Parameter Index Rx */
  uint8_t spi_rx_valid;  /**< Must be set to true if spi_rx is being passed */
  uint32_t spi_rx;
  /**<   Security parameter index Rx.
    */

  /* Optional */
  /*  Security Parameter Index Tx */
  uint8_t spi_tx_valid;  /**< Must be set to true if spi_tx is being passed */
  uint32_t spi_tx;
  /**<   Security parameter index Tx.
    */

  /* Optional */
  /*  IPSec SA Protocol */
  uint8_t ipsec_sa_protocol_valid;  /**< Must be set to true if ipsec_sa_protocol is being passed */
  wds_ipsec_sa_protocol_enum_v01 ipsec_sa_protocol;
  /**<   Values: 
      - WDS_IPSEC_SA_PROTOCOL_UNDEFINED (0x00) --  Undefined \n 
      - WDS_IPSEC_SA_PROTOCOL_ISAKMP (0x01) --  Internet Security Association and Key Management Protocol \n 
      - WDS_IPSEC_SA_PROTOCOL_IPSEC_AH (0x02) --  Authentication header \n 
      - WDS_IPSEC_SA_PROTOCOL_IPSEC_ESP (0x03) --  Encapsulating security payload 
 */

  /* Optional */
  /*  Encapsulation Mode */
  uint8_t encapsulation_mode_valid;  /**< Must be set to true if encapsulation_mode is being passed */
  wds_ipsec_sa_encapsulate_enum_v01 encapsulation_mode;
  /**<   Values: 
      - WDS_IPSEC_SA_ENCAPSULATE_UNDEFINED (0x00) --  Undefined \n 
      - WDS_IPSEC_SA_ENCAPSULATE_TUNNEL (0x01) --  Encapsulation mode tunnel \n 
      - WDS_IPSEC_SA_ENCAPSULATE_TRANSPORT (0x02) --  Encapsulation mode transport  
 */

  /* Optional */
  /*  Destination Address */
  uint8_t dest_addr_valid;  /**< Must be set to true if dest_addr is being passed */
  wds_ip_addr_type_v01 dest_addr;

  /* Optional */
  /*  Local Address */
  uint8_t local_addr_valid;  /**< Must be set to true if local_addr is being passed */
  wds_ip_addr_type_v01 local_addr;

  /* Optional */
  /*  Hash Algorithm */
  uint8_t hash_algo_valid;  /**< Must be set to true if hash_algo is being passed */
  wds_ipsec_crypto_algo_enum_v01 hash_algo;
  /**<   See Appendix \ref{app:CryptoAlgo} for the definition
       of these values.
   */

  /* Optional */
  /*  Hash Key Rx */
  uint8_t hash_key_rx_valid;  /**< Must be set to true if hash_key_rx is being passed */
  uint32_t hash_key_rx_len;  /**< Must be set to # of elements in hash_key_rx */
  uint8_t hash_key_rx[QMI_WDS_IPSEC_SA_HASH_MAX_SIZE_V01];
  /**<   Hash key Rx.*/

  /* Optional */
  /*  Hash Key Tx */
  uint8_t hash_key_tx_valid;  /**< Must be set to true if hash_key_tx is being passed */
  uint32_t hash_key_tx_len;  /**< Must be set to # of elements in hash_key_tx */
  uint8_t hash_key_tx[QMI_WDS_IPSEC_SA_HASH_MAX_SIZE_V01];
  /**<   Hash key Tx.*/

  /* Optional */
  /*  Cryptography Algorithm */
  uint8_t crypto_algo_valid;  /**< Must be set to true if crypto_algo is being passed */
  wds_ipsec_crypto_algo_enum_v01 crypto_algo;
  /**<   See Appendix \ref{app:CryptoAlgo} for the definition
       of these values.
   */

  /* Optional */
  /*  Cryptography Key Rx */
  uint8_t crypto_key_rx_valid;  /**< Must be set to true if crypto_key_rx is being passed */
  uint32_t crypto_key_rx_len;  /**< Must be set to # of elements in crypto_key_rx */
  uint8_t crypto_key_rx[QMI_WDS_IPSEC_SA_CRYPTO_ALGO_KEY_MAX_SIZE_V01];
  /**<   Cryptography key Rx. */

  /* Optional */
  /*  Cryptography Key Tx */
  uint8_t crypto_key_tx_valid;  /**< Must be set to true if crypto_key_tx is being passed */
  uint32_t crypto_key_tx_len;  /**< Must be set to # of elements in crypto_key_tx */
  uint8_t crypto_key_tx[QMI_WDS_IPSEC_SA_CRYPTO_ALGO_KEY_MAX_SIZE_V01];
  /**<   Cryptography key Tx. */

  /* Optional */
  /*  Initialization Vector */
  uint8_t iv_valid;  /**< Must be set to true if iv is being passed */
  uint32_t iv_len;  /**< Must be set to # of elements in iv */
  uint8_t iv[QMI_WDS_IPSEC_SA_AES_BLOCK_BYTE_LEN_V01];
  /**<   Initialization vector. */

  /* Optional */
  /*  UDP Encapsulated */
  uint8_t is_udp_encaps_valid;  /**< Must be set to true if is_udp_encaps is being passed */
  uint8_t is_udp_encaps;
  /**<   Values: \n
       - 0 -- FALSE \n
       - 1 -- TRUE 
   */

  /* Optional */
  /*  NAT Local IP Address */
  uint8_t nat_local_ip_addr_valid;  /**< Must be set to true if nat_local_ip_addr is being passed */
  wds_ip_addr_type_v01 nat_local_ip_addr;

  /* Optional */
  /*  NAT Remote IP Address */
  uint8_t nat_remote_ip_addr_valid;  /**< Must be set to true if nat_remote_ip_addr is being passed */
  wds_ip_addr_type_v01 nat_remote_ip_addr;

  /* Optional */
  /*  Configuration Attribute Internal IPv4 Address */
  uint8_t cfg_attr_internal_ipv4_address_valid;  /**< Must be set to true if cfg_attr_internal_ipv4_address is being passed */
  uint32_t cfg_attr_internal_ipv4_address_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv4_address */
  uint32_t cfg_attr_internal_ipv4_address[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute internal IPv4 address. */

  /* Optional */
  /*  Configuration Attribute Internal IPv4 Netmask */
  uint8_t cfg_attr_internal_ipv4_netmask_valid;  /**< Must be set to true if cfg_attr_internal_ipv4_netmask is being passed */
  uint32_t cfg_attr_internal_ipv4_netmask;
  /**<   Configuration attribute internal IPv4 netmask.*/

  /* Optional */
  /*  Configuration Attribute Internal IPv4 DNS */
  uint8_t cfg_attr_internal_ipv4_dns_valid;  /**< Must be set to true if cfg_attr_internal_ipv4_dns is being passed */
  uint32_t cfg_attr_internal_ipv4_dns_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv4_dns */
  uint32_t cfg_attr_internal_ipv4_dns[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute internal IPv4 DNS. */

  /* Optional */
  /*  Configuration Attribute Internal IPv4 NBNS */
  uint8_t cfg_attr_internal_ipv4_nbns_valid;  /**< Must be set to true if cfg_attr_internal_ipv4_nbns is being passed */
  uint32_t cfg_attr_internal_ipv4_nbns_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv4_nbns */
  uint32_t cfg_attr_internal_ipv4_nbns[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute internal IPv4 NBNS. */

  /* Optional */
  /*  Configuration Attribute Internal Address Expiry */
  uint8_t cfg_attr_internal_address_expiry_valid;  /**< Must be set to true if cfg_attr_internal_address_expiry is being passed */
  uint32_t cfg_attr_internal_address_expiry;
  /**<   Configuration attribute internal address expiry. */

  /* Optional */
  /*  Configuration Attribute Internal IPv4 DHCP */
  uint8_t cfg_attr_internal_ipv4_dhcp_valid;  /**< Must be set to true if cfg_attr_internal_ipv4_dhcp is being passed */
  uint32_t cfg_attr_internal_ipv4_dhcp_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv4_dhcp */
  uint32_t cfg_attr_internal_ipv4_dhcp[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute internal IPv4 DHCP. */

  /* Optional */
  /*  Configuration Attribute Application Version */
  uint8_t cfg_attr_application_version_valid;  /**< Must be set to true if cfg_attr_application_version is being passed */
  uint32_t cfg_attr_application_version_len;  /**< Must be set to # of elements in cfg_attr_application_version */
  char cfg_attr_application_version[QMI_WDS_IPSEC_SA_CFG_DATA_SIZE_MAX_V01];
  /**<   Configuration attribute application version. */

  /* Optional */
  /*  Configuration Attribute Internal IPv6 Address */
  uint8_t cfg_attr_internal_ipv6_address_valid;  /**< Must be set to true if cfg_attr_internal_ipv6_address is being passed */
  uint32_t cfg_attr_internal_ipv6_address_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv6_address */
  wds_runtime_ipv6_addr_type_v01 cfg_attr_internal_ipv6_address[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute Internal IPv6 DNS */
  uint8_t cfg_attr_internal_ipv6_dns_valid;  /**< Must be set to true if cfg_attr_internal_ipv6_dns is being passed */
  uint32_t cfg_attr_internal_ipv6_dns_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv6_dns */
  wds_cfg_attr_ipv6_type_v01 cfg_attr_internal_ipv6_dns[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute Internal IPv6 NBNS */
  uint8_t cfg_attr_internal_ipv6_nbns_valid;  /**< Must be set to true if cfg_attr_internal_ipv6_nbns is being passed */
  uint32_t cfg_attr_internal_ipv6_nbns_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv6_nbns */
  wds_cfg_attr_ipv6_type_v01 cfg_attr_internal_ipv6_nbns[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute Internal IPv6 DHCP */
  uint8_t cfg_attr_internal_ipv6_dhcp_valid;  /**< Must be set to true if cfg_attr_internal_ipv6_dhcp is being passed */
  uint32_t cfg_attr_internal_ipv6_dhcp_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv6_dhcp */
  wds_cfg_attr_ipv6_type_v01 cfg_attr_internal_ipv6_dhcp[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute Internal IPv4 Subnet */
  uint8_t cfg_attr_internal_ipv4_subnet_valid;  /**< Must be set to true if cfg_attr_internal_ipv4_subnet is being passed */
  uint32_t cfg_attr_internal_ipv4_subnet_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv4_subnet */
  wds_cfg_attr_ipv4_subnet_type_v01 cfg_attr_internal_ipv4_subnet[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute Supported Attributes */
  uint8_t cfg_attr_supported_attributes_valid;  /**< Must be set to true if cfg_attr_supported_attributes is being passed */
  uint32_t cfg_attr_supported_attributes_len;  /**< Must be set to # of elements in cfg_attr_supported_attributes */
  char cfg_attr_supported_attributes[QMI_WDS_IPSEC_SA_CFG_DATA_SIZE_MAX_V01];
  /**<   Configuration attribute supported attributes. */

  /* Optional */
  /*  Configuration Attribute Internal IPv6 Subnet */
  uint8_t cfg_attr_internal_ipv6_subnet_valid;  /**< Must be set to true if cfg_attr_internal_ipv6_subnet is being passed */
  uint32_t cfg_attr_internal_ipv6_subnet_len;  /**< Must be set to # of elements in cfg_attr_internal_ipv6_subnet */
  wds_runtime_ipv6_addr_type_v01 cfg_attr_internal_ipv6_subnet[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute Internal PCSCF IPv4 Address */
  uint8_t cfg_attr_internal_pcscf_ipv4_address_valid;  /**< Must be set to true if cfg_attr_internal_pcscf_ipv4_address is being passed */
  uint32_t cfg_attr_internal_pcscf_ipv4_address_len;  /**< Must be set to # of elements in cfg_attr_internal_pcscf_ipv4_address */
  uint32_t cfg_attr_internal_pcscf_ipv4_address[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute internal PCSCF IPv4 address.*/

  /* Optional */
  /*  Configuration Attribute Internal PCSCF IPv6 Address */
  uint8_t cfg_attr_internal_pcscf_ipv6_address_valid;  /**< Must be set to true if cfg_attr_internal_pcscf_ipv6_address is being passed */
  uint32_t cfg_attr_internal_pcscf_ipv6_address_len;  /**< Must be set to # of elements in cfg_attr_internal_pcscf_ipv6_address */
  wds_runtime_ipv6_addr_type_v01 cfg_attr_internal_pcscf_ipv6_address[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute 3GPP2 MIP4 HA */
  uint8_t cfg_attr_3gpp2_mip4_ha_valid;  /**< Must be set to true if cfg_attr_3gpp2_mip4_ha is being passed */
  uint32_t cfg_attr_3gpp2_mip4_ha_len;  /**< Must be set to # of elements in cfg_attr_3gpp2_mip4_ha */
  uint32_t cfg_attr_3gpp2_mip4_ha[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute 3GPP2 MIP4 HA. */

  /* Optional */
  /*  Configuration Attribute 3GPP2 MIP4 HOA */
  uint8_t cfg_attr_3gpp2_mip4_hoa_valid;  /**< Must be set to true if cfg_attr_3gpp2_mip4_hoa is being passed */
  uint32_t cfg_attr_3gpp2_mip4_hoa_len;  /**< Must be set to # of elements in cfg_attr_3gpp2_mip4_hoa */
  uint32_t cfg_attr_3gpp2_mip4_hoa[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];
  /**<   Configuration attribute 3GPP2 MIP4 HOA. */

  /* Optional */
  /*  Configuration Attribute 3GPP2 MIP6 HA */
  uint8_t cfg_attr_3gpp2_mip6_ha_valid;  /**< Must be set to true if cfg_attr_3gpp2_mip6_ha is being passed */
  uint32_t cfg_attr_3gpp2_mip6_ha_len;  /**< Must be set to # of elements in cfg_attr_3gpp2_mip6_ha */
  wds_runtime_ipv6_addr_type_v01 cfg_attr_3gpp2_mip6_ha[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Configuration Attribute 3GPP2 MIP6 HOA */
  uint8_t cfg_attr_3gpp2_mip6_hoa_valid;  /**< Must be set to true if cfg_attr_3gpp2_mip6_hoa is being passed */
  uint32_t cfg_attr_3gpp2_mip6_hoa_len;  /**< Must be set to # of elements in cfg_attr_3gpp2_mip6_hoa */
  wds_runtime_ipv6_addr_type_v01 cfg_attr_3gpp2_mip6_hoa[QMI_WDS_IPSEC_SA_CFG_ATTR_MAX_V01];

  /* Optional */
  /*  Traffic Selector List */
  uint8_t traffic_selector_list_valid;  /**< Must be set to true if traffic_selector_list is being passed */
  uint32_t traffic_selector_list_len;  /**< Must be set to # of elements in traffic_selector_list */
  wds_traffic_selector_type_v01 traffic_selector_list[QMI_WDS_IPSEC_SA_MAX_TS_COUNT_V01];

  /* Optional */
  /*  Traffic Selector Responder List */
  uint8_t traffic_selector_responder_list_valid;  /**< Must be set to true if traffic_selector_responder_list is being passed */
  uint32_t traffic_selector_responder_list_len;  /**< Must be set to # of elements in traffic_selector_responder_list */
  wds_traffic_selector_type_v01 traffic_selector_responder_list[QMI_WDS_IPSEC_SA_MAX_TS_COUNT_V01];

  /* Optional */
  /*  Access Point Name */
  uint8_t apn_valid;  /**< Must be set to true if apn is being passed */
  char apn[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   APN. */

  /* Optional */
  /*  Advanced Encryption Standard Mode */
  uint8_t aes_mode_valid;  /**< Must be set to true if aes_mode is being passed */
  wds_ipsec_aes_mode_enum_v01 aes_mode;
  /**<   Advanced Encryption Standard Mode. Values: \n
      - WDS_IPSEC_AES_MODE_MIN (0) --  No AES Mode \n 
      - WDS_IPSEC_AES_MODE_CBC (1) --  AES Mode CBC \n 
      - WDS_IPSEC_AES_MODE_CTR (2) --  AES Mode CTR \n 
 */
}wds_get_ipsec_static_sa_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sends notification that reverse IP transport configuration is complete on the Application
    Processor (AP) side. */
typedef struct {

  /* Mandatory */
  /*  Configuration Result */
  uint8_t config_result;
  /**<   Values: \n
       - 0 -- Failure \n
       - 1 -- Success 
  */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Transaction ID of the indication for which the processing is complete.
   */
}wds_reverse_ip_transport_config_complete_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sends notification that reverse IP transport configuration is complete on the Application
    Processor (AP) side. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_reverse_ip_transport_config_complete_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the data bearer technology. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_data_bearer_technology_ex_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the data bearer technology. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Data Bearer Technology  */
  uint8_t bearer_tech_valid;  /**< Must be set to true if bearer_tech is being passed */
  wds_bearer_tech_info_type_v01 bearer_tech;

  /* Optional */
  /*  Last Call Bearer Technology */
  uint8_t last_bearer_tech_valid;  /**< Must be set to true if last_bearer_tech is being passed */
  wds_bearer_tech_info_type_v01 last_bearer_tech;
}wds_get_data_bearer_technology_ex_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Enables or disables retrying an LTE data attach. */
typedef struct {

  /* Mandatory */
  /*  LTE Data Retry Setting */
  uint8_t lte_data_retry;
  /**<   Whether to retry an LTE data attach on a different PDN. Values: \n
      - 0 -- Do not retry in case of failure \n
      - 1 -- Retry in case of failure
  */
}wds_set_lte_data_retry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Enables or disables retrying an LTE data attach. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_lte_data_retry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the current LTE data retry setting. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_lte_data_retry_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the current LTE data retry setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Data Retry Setting */
  uint8_t lte_data_retry_valid;  /**< Must be set to true if lte_data_retry is being passed */
  uint8_t lte_data_retry;
  /**<   Whether to retry an LTE data attach on a different PDN. Values: \n
       - 0 -- Do not retry in case of failure \n
       - 1 -- Retry in case of failure
  */
}wds_get_lte_data_retry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_LTE_ATTACH_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_LTE_ATTACH_TYPE_INITIAL_V01 = 0, /**<  LTE initial attach is to be performed \n  */
  WDS_LTE_ATTACH_TYPE_HANDOFF_V01 = 1, /**<  LTE handoff attach is to be performed  */
  WDS_LTE_ATTACH_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_lte_attach_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets whether the attach to be performed is initial or handoff. */
typedef struct {

  /* Mandatory */
  /*  LTE Attach Type */
  wds_lte_attach_type_enum_v01 lte_attach_type;
  /**<   Whether the attach is initial or handoff. Values: \n
      - WDS_LTE_ATTACH_TYPE_INITIAL (0) --  LTE initial attach is to be performed \n 
      - WDS_LTE_ATTACH_TYPE_HANDOFF (1) --  LTE handoff attach is to be performed 
 */
}wds_set_lte_attach_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets whether the attach to be performed is initial or handoff. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_lte_attach_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves the current LTE attach type. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_lte_attach_type_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves the current LTE attach type. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Attach Type */
  uint8_t lte_attach_type_valid;  /**< Must be set to true if lte_attach_type is being passed */
  wds_lte_attach_type_enum_v01 lte_attach_type;
  /**<   Whether the attach is initial or handoff. Values: \n
      - WDS_LTE_ATTACH_TYPE_INITIAL (0) --  LTE initial attach is to be performed \n 
      - WDS_LTE_ATTACH_TYPE_HANDOFF (1) --  LTE handoff attach is to be performed 
 */
}wds_get_lte_attach_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_REVERSE_IP_TRANSPORT_FILTER_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_REVERSE_IP_TRANSPORT_ESP_SPI_FILTER_V01 = 0, /**<  ESP SPI filter  */
  WDS_REVERSE_IP_TRANSPORT_FILTER_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_reverse_ip_transport_filter_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates that a reverse IP transport filter must be set up. */
typedef struct {

  /* Mandatory */
  /*  Filter Type */
  wds_reverse_ip_transport_filter_type_enum_v01 filter_type;
  /**<   Type of filter to set up. Values: \n
      - WDS_REVERSE_IP_TRANSPORT_ESP_SPI_FILTER (0) --  ESP SPI filter 
 */

  /* Optional */
  /*  Security Parameter Index */
  uint8_t spi_valid;  /**< Must be set to true if spi is being passed */
  uint32_t spi;
  /**<   Security parameter index. */
}wds_reverse_ip_transport_filter_setup_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_HANDOFF_INFORMATION_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_HANDOFF_INIT_V01 = 0, /**<  Handoff has started \n  */
  WDS_HANDOFF_SUCCESS_V01 = 1, /**<  Handoff is successful \n  */
  WDS_HANDOFF_FAILURE_V01 = 2, /**<  Handoff failed  */
  WDS_HANDOFF_INFORMATION_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_handoff_information_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_bearer_tech_rat_ex_enum_v01 srat;
  /**<   Source RAT value. Values: \n
      - WDS_BEARER_TECH_RAT_EX_NULL_BEARER (0x00) --  NULL bearer 
      - WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA (0x01) --  3GPP WCDMA 
      - WDS_BEARER_TECH_RAT_EX_3GPP_GERAN (0x02) --  3GPP GERAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP_LTE (0x03) --  3GPP LTE 
      - WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA (0x04) --  3GPP TDSCDMA
      - WDS_BEARER_TECH_RAT_EX_3GPP_WLAN (0x05) --  3GPP WLAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP_MAX (0x64) --  3GPP maximum 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_1X (0x65) --  3GPP2 1X 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD (0x66) --  3GPP2 HRPD 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD (0x67) --  3GPP2 EHRPD 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN (0x68) --  3GPP2 WLAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_MAX (0xC8) --  3GPP2 maximum  
 */

  wds_bearer_tech_rat_ex_enum_v01 trat;
  /**<   Target RAT value. Values: \n
      - WDS_BEARER_TECH_RAT_EX_NULL_BEARER (0x00) --  NULL bearer 
      - WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA (0x01) --  3GPP WCDMA 
      - WDS_BEARER_TECH_RAT_EX_3GPP_GERAN (0x02) --  3GPP GERAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP_LTE (0x03) --  3GPP LTE 
      - WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA (0x04) --  3GPP TDSCDMA
      - WDS_BEARER_TECH_RAT_EX_3GPP_WLAN (0x05) --  3GPP WLAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP_MAX (0x64) --  3GPP maximum 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_1X (0x65) --  3GPP2 1X 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD (0x66) --  3GPP2 HRPD 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD (0x67) --  3GPP2 EHRPD 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN (0x68) --  3GPP2 WLAN 
      - WDS_BEARER_TECH_RAT_EX_3GPP2_MAX (0xC8) --  3GPP2 maximum  
 */
}wds_handoff_event_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates that a handoff is in progress or has been completed. */
typedef struct {

  /* Mandatory */
  /*  Handoff Information */
  wds_handoff_information_type_enum_v01 handoff_information;
  /**<   Handoff information. Values: \n
      - WDS_HANDOFF_INIT (0) --  Handoff has started \n 
      - WDS_HANDOFF_SUCCESS (1) --  Handoff is successful \n 
      - WDS_HANDOFF_FAILURE (2) --  Handoff failed 
 */

  /* Optional */
  /*  Handoff Event Information */
  uint8_t handoff_event_info_valid;  /**< Must be set to true if handoff_event_info is being passed */
  wds_handoff_event_info_type_v01 handoff_event_info;
  /**<   Handoff event information. This TLV contains the source rat and 
       target rat information when Handoff event occurs.
   */
}wds_handoff_information_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DATA_PATH_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DATA_PATH_HW_V01 = 0, /**<  Hardware data path   */
  WDS_DATA_PATH_SW_V01 = 1, /**<  Software data path   */
  WDS_DATA_PATH_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_data_path_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the client data path. */
typedef struct {

  /* Mandatory */
  /*  Sets Data Path */
  wds_data_path_type_enum_v01 data_path;
  /**<   Values: \n
      - WDS_DATA_PATH_HW (0) --  Hardware data path  
      - WDS_DATA_PATH_SW (1) --  Software data path  
 */
}wds_set_data_path_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the client data path. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_data_path_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the current modem data path. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_data_path_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the current modem data path. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Current Data Path */
  uint8_t data_path_valid;  /**< Must be set to true if data_path is being passed */
  wds_data_path_type_enum_v01 data_path;
  /**<   Values: \n
      - WDS_DATA_PATH_HW (0) --  Hardware data path  
      - WDS_DATA_PATH_SW (1) --  Software data path  
 */
}wds_get_data_path_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Triggers the modem to update the profile parameters.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_update_lte_attach_pdn_list_profiles_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Triggers the modem to update the profile parameters.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_update_lte_attach_pdn_list_profiles_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t frequency;
  /**<   Frequency associated with sai_list.
   */

  uint8_t is_serving_frequency;
  /**<   Indicates whether this is a serving frequency.
   */

  uint32_t sai_list_len;  /**< Must be set to # of elements in sai_list */
  uint32_t sai_list[QMI_WDS_EMBMS_SAI_LIST_MAX_V01];
  /**<   Service area identity for this frequency.
   */
}wds_embms_sai_list_per_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t frequency;
  /**<    Frequency associated with sai_list.
   */

  uint8_t is_serving_frequency;
  /**<   Indicates whether this is a serving frequency.
   */

  uint32_t sai_list_len;  /**< Must be set to # of elements in sai_list */
  uint32_t sai_list[QMI_WDS_EMBMS_SAI_LIST_MAX_V01];
  /**<   Service area identity for this frequency.
   */
}wds_embms_sai_list_per_freq_ex_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the Service Area Identity (SAI) list. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_embms_sai_list_query_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the Service Area Identity (SAI) list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  SAI List */
  uint8_t freq_sai_list_valid;  /**< Must be set to true if freq_sai_list is being passed */
  uint32_t freq_sai_list_len;  /**< Must be set to # of elements in freq_sai_list */
  wds_embms_sai_list_per_freq_type_v01 freq_sai_list[QMI_WDS_EMBMS_FREQ_SAI_MAX_V01];

  /* Optional */
  /*  Extended SAI List */
  uint8_t freq_sai_list_ex_valid;  /**< Must be set to true if freq_sai_list_ex is being passed */
  uint32_t freq_sai_list_ex_len;  /**< Must be set to # of elements in freq_sai_list_ex */
  wds_embms_sai_list_per_freq_ex_type_v01 freq_sai_list_ex[QMI_WDS_EMBMS_FREQ_SAI_MAX_V01];
}wds_embms_sai_list_query_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the currently available SAI list. */
typedef struct {

  /* Optional */
  /*  SAI List */
  uint8_t freq_sai_list_valid;  /**< Must be set to true if freq_sai_list is being passed */
  uint32_t freq_sai_list_len;  /**< Must be set to # of elements in freq_sai_list */
  wds_embms_sai_list_per_freq_type_v01 freq_sai_list[QMI_WDS_EMBMS_FREQ_SAI_MAX_V01];

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   SAI transaction ID for the indication.
  */

  /* Optional */
  /*  Extended SAI List */
  uint8_t freq_sai_list_ex_valid;  /**< Must be set to true if freq_sai_list_ex is being passed */
  uint32_t freq_sai_list_ex_len;  /**< Must be set to # of elements in freq_sai_list_ex */
  wds_embms_sai_list_per_freq_ex_type_v01 freq_sai_list_ex[QMI_WDS_EMBMS_FREQ_SAI_MAX_V01];
}wds_embms_sai_list_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_CLIENT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_CLIENT_TYPE_RESERVED_V01 = 0, /**<  Reserved \n  */
  WDS_CLIENT_TYPE_TETHERED_V01 = 1, /**<  Tethered \n  */
  WDS_CLIENT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_client_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Binds a control point to a muxed data port. */
typedef struct {

  /* Optional */
  /*  Peripheral End Point ID */
  uint8_t ep_id_valid;  /**< Must be set to true if ep_id is being passed */
  data_ep_id_type_v01 ep_id;
  /**<   \n Peripheral end point (physical data channel) to which
       the client binds.
  */

  /* Optional */
  /*  Mux ID */
  uint8_t mux_id_valid;  /**< Must be set to true if mux_id is being passed */
  uint8_t mux_id;
  /**<   Mux ID of the logical data channel to which
       the client binds. The default value is 0. 
  */

  /* Optional */
  /*  Reversed RmNet Flag */
  uint8_t reversed_valid;  /**< Must be set to true if reversed is being passed */
  uint8_t reversed;
  /**<   Binds to a reversed RmNet data port. Values: \n
       - 0x00 -- FALSE (default) \n
       - 0x01 -- TRUE 
  */

  /* Optional */
  /*  Client Type */
  uint8_t client_type_valid;  /**< Must be set to true if client_type is being passed */
  wds_client_type_enum_v01 client_type;
  /**<   Type of the client that requests the binding. Values: \n
      - WDS_CLIENT_TYPE_RESERVED (0) --  Reserved \n 
      - WDS_CLIENT_TYPE_TETHERED (1) --  Tethered \n 

 All other values are reserved and are ignored by service.
 */
}wds_bind_mux_data_port_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Binds a control point to a muxed data port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_bind_mux_data_port_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Initiates an ESP rekey. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_initiate_esp_rekey_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Initiates an ESP rekey. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_initiate_esp_rekey_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint32_t wds_evt_reg_mask_v01;
#define QMI_WDS_MASK_EVT_OUTAGE_NOTIFICATION_V01 ((wds_evt_reg_mask_v01)0x01) /**<  Outage notification \n  */
#define QMI_WDS_MASK_EVT_EXT_IPCONFIG_V01 ((wds_evt_reg_mask_v01)0x02) /**<  Extended IP configuration (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n  */
#define QMI_WDS_MASK_EVT_HDR_RATE_INERTIA_SUCCESS_V01 ((wds_evt_reg_mask_v01)0x04) /**<  HDR Rev0 rate inertia success \n  */
#define QMI_WDS_MASK_EVT_HDR_RATE_INERTIA_FAILURE_V01 ((wds_evt_reg_mask_v01)0x08) /**<  HDR Rev0 rate inertia failure \n  */
#define QMI_WDS_MASK_EVT_HDR_SM_SUCCESS_V01 ((wds_evt_reg_mask_v01)0x10) /**<  HDR set EIDLE Slotted Mode success (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n  */
#define QMI_WDS_MASK_EVT_HDR_SM_FAILURE_V01 ((wds_evt_reg_mask_v01)0x20) /**<  HDR set EIDLE Slotted Mode failure (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n  */
#define QMI_WDS_MASK_EVT_HDR_SM_SESS_CHANGE_V01 ((wds_evt_reg_mask_v01)0x40) /**<  HDR set EIDLE Slotted Mode session change (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n  */
#define QMI_WDS_MASK_EVT_RF_CONDITIONS_V01 ((wds_evt_reg_mask_v01)0x80) /**<  RF conditions change \n  */
#define QMI_WDS_MASK_EVT_DOS_ACK_V01 ((wds_evt_reg_mask_v01)0x100) /**<  DOS ACK event \n  */
/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Registers for IFACE events.  */
typedef struct {

  /* Mandatory */
  /*  Event Registration Mask */
  wds_evt_reg_mask_v01 event_registration_mask;
  /**<   Requested event bitmask. Values: \n
      - QMI_WDS_MASK_EVT_OUTAGE_NOTIFICATION (0x01) --  Outage notification \n 
      - QMI_WDS_MASK_EVT_EXT_IPCONFIG (0x02) --  Extended IP configuration (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n 
      - QMI_WDS_MASK_EVT_HDR_RATE_INERTIA_SUCCESS (0x04) --  HDR Rev0 rate inertia success \n 
      - QMI_WDS_MASK_EVT_HDR_RATE_INERTIA_FAILURE (0x08) --  HDR Rev0 rate inertia failure \n 
      - QMI_WDS_MASK_EVT_HDR_SM_SUCCESS (0x10) --  HDR set EIDLE Slotted Mode success (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n 
      - QMI_WDS_MASK_EVT_HDR_SM_FAILURE (0x20) --  HDR set EIDLE Slotted Mode failure (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n 
      - QMI_WDS_MASK_EVT_HDR_SM_SESS_CHANGE (0x40) --  HDR set EIDLE Slotted Mode session change (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}) \n 
      - QMI_WDS_MASK_EVT_RF_CONDITIONS (0x80) --  RF conditions change \n 
      - QMI_WDS_MASK_EVT_DOS_ACK (0x100) --  DOS ACK event \n 

 Each bit set causes the corresponding optional TLV to be sent in 
 the QMI_WDS_INTERNAL_IFACE_EV_IND indication. All unlisted bits are reserved for 
 future use and must be set to zero.
 */
}wds_internal_iface_ev_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Registers for IFACE events.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_internal_iface_ev_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IFACE_EVENT_NAME_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IFACE_EVENT_OUTAGE_NOTIFICATION_V01 = 0x0001, /**<  Outage notification \n  */
  WDS_IFACE_EVENT_EXT_IPCONFIG_V01 = 0x0002, /**<  Extended IP configuration (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister})\n  */
  WDS_IFACE_EVENT_HDR_RATE_INERTIA_SUCCESS_V01 = 0x0003, /**<  HDR Rev0 rate inertia success \n  */
  WDS_IFACE_EVENT_HDR_RATE_INERTIA_FAILURE_V01 = 0x0004, /**<  HDR Rev0 rate inertia failure \n  */
  WDS_IFACE_EVENT_HDR_SM_SUCCESS_V01 = 0x0005, /**<  HDR Set EIDLE Slotted Mode success (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister})\n  */
  WDS_IFACE_EVENT_HDR_SM_FAILURE_V01 = 0x0006, /**<  HDR Set EIDLE Slotted Mode failure (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister})\n  */
  WDS_IFACE_EVENT_HDR_SM_SESS_CHANGE_V01 = 0x0007, /**<  HDR Set EIDLE Slotted Mode session change (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}\n  */
  WDS_IFACE_EVENT_RF_CONDITIONS_V01 = 0x0008, /**<  RF conditions change \n  */
  WDS_IFACE_EVENT_DOS_ACK_V01 = 0x0009, /**<  DOS ACK event   */
  WDS_IFACE_EVENT_NAME_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_iface_event_name_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_HDR_REV0_RATE_INERTIA_FAILURE_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_HDR_REV0_RATE_INERTIA_REQUEST_REJECTED_V01 = 0, /**<  Request rejected \n  */
  WDS_HDR_REV0_RATE_INERTIA_REQUEST_FAILED_TX_V01 = 1, /**<  Request failed Tx \n  */
  WDS_HDR_REV0_RATE_INERTIA_NOT_SUPPORTED_V01 = 2, /**<  Not supported \n  */
  WDS_HDR_REV0_RATE_INERTIA_NO_NET_V01 = 3, /**<  No net   */
  WDS_HDR_REV0_RATE_INERTIA_FAILURE_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_hdr_rev0_rate_inertia_failure_code_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_HDR_SLOTTED_MODE_FAILURE_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_HDR_SLOTTED_MODE_REQUEST_REJECTED_V01 = 0, /**<  Request rejected \n  */
  WDS_HDR_SLOTTED_MODE_REQUEST_FAILED_TX_V01 = 1, /**<  Request failed Tx \n  */
  WDS_HDR_SLOTTED_MODE_NOT_SUPPORTED_V01 = 2, /**<  Not supported \n  */
  WDS_HDR_SLOTTED_MODE_NO_NET_V01 = 3, /**<  No net  */
  WDS_HDR_SLOTTED_MODE_FAILURE_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_hdr_slotted_mode_failure_code_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_RF_CONDITIONS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_RF_CONDITIONS_INVALID_V01 = 0, /**<  Invalid \n  */
  WDS_RF_CONDITIONS_BAD_V01 = 1, /**<  Bad \n  */
  WDS_RF_CONDITIONS_GOOD_V01 = 2, /**<  Good\n  */
  WDS_RF_CONDITIONS_DONT_CARE_V01 = 3, /**<  Do not care  */
  WDS_RF_CONDITIONS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_rf_conditions_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_DOS_ACK_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_DOS_ACK_NONE_V01 = -1, /**<  None \n  */
  WDS_DOS_ACK_OK_V01 = 0x00000000, /**<  Completed successfully \n  */
  WDS_DOS_ACK_HOLD_ORIG_RETRY_TIMEOUT_V01 = 0x00000001, /**<  Hold original retry timeout \n  */
  WDS_DOS_ACK_HOLD_ORIG_V01 = 0x00000002, /**<  Cannot proceed because hold original is TRUE \n  */
  WDS_DOS_ACK_NO_SRV_V01 = 0x00000003, /**<  No service \n  */
  WDS_DOS_ACK_ABORT_V01 = 0x00000004, /**<  Abort \n  */
  WDS_DOS_ACK_NOT_ALLOWED_IN_AMPS_V01 = 0x00000005, /**<  Cannot send in Analog mode \n  */
  WDS_DOS_ACK_NOT_ALLOWED_IN_HDR_V01 = 0x00000006, /**<  Cannot send in an HDR call \n  */
  WDS_DOS_ACK_L2_ACK_FAILURE_V01 = 0x00000007, /**<  Failure receiving the L2 ACK \n  */
  WDS_DOS_ACK_OUT_OF_RESOURCES_V01 = 0x00000008, /**<  Out of resources, e.g., memory buffer is full \n  */
  WDS_DOS_ACK_ACCESS_TOO_LARGE_V01 = 0x00000009, /**<  Message is too large to be sent over ACC \n  */
  WDS_DOS_ACK_DTC_TOO_LARGE_V01 = 0x0000000A, /**<  Message is too large to be sent over DTC \n  */
  WDS_DOS_ACK_OTHER_V01 = 0x0000000B, /**<  Any status response other than above \n  */
  WDS_DOS_ACK_ACCT_BLOCK_V01 = 0x0000000C, /**<  Access is blocked based on the service option \n  */
  WDS_DOS_ACK_L3_ACK_FAILURE_V01 = 0x0000000D, /**<  Failure receiving the L3 ACK  */
  WDS_DOS_ACK_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_dos_ack_status_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t time_to_outage;
  /**<   Milliseconds in which the HDR outage starts
  */

  uint32_t duration;
  /**<   Milliseconds for which the HDR outage lasts
  */
}wds_outage_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_current_nw_enum_v01 db_current_nw;
  /**<   Current network type of data bearer. Values: \n
      - WDS_CURRENT_NETWORK_UNKNOWN (0x00) --  Unknown\n 
      - WDS_CURRENT_NETWORK_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_CURRENT_NETWORK_3GPP (0x02) --  3GPP 
 */

  uint32_t db_rat_mask;
  /**<   RAT mask to indicate the type of technology.
       A RAT mask value of zero indicates that this field is ignored. Values: \n
       - 0x00 -- DONT_CARE \n
       - 0x8000 -- NULL_BEARER \n

       CDMA RAT mask: \n
       - 0x01 -- CDMA_1X \n
       - 0x02 -- EVDO_REV0 \n
       - 0x04 -- EVDO_REVA \n
       - 0x08 -- EVDO_REVB \n
       - 0x10 -- EHRPD \n
       - 0x20 -- FMC \n

       UMTS RAT mask: \n
       - 0x01 -- WCDMA \n
       - 0x02 -- GPRS \n
       - 0x04 -- HSDPA \n
       - 0x08 -- HSUPA \n
       - 0x10 -- EDGE \n
       - 0x20 -- LTE \n
       - 0x40 -- HSDPA+ \n
       - 0x80 -- DC_HSDPA+ \n
       - 0x100 -- 64_QAM \n
       - 0x200 -- TDSCDMA
  */

  uint32_t db_so_mask;
  /**<   SO mask to indicate the service option or type of
       application.

       An SO mask value of zero indicates that this field is ignored. Values: \n
       - 0x00 -- DONT_CARE \n

       CDMA 1X SO mask: \n
       - 0x01 -- CDMA_1X_IS95 \n
       - 0x02 -- CDMA_1X_IS2000 \n
       - 0x04 -- CDMA_1X_IS2000_REL_A \n

       CDMA EV-DO Rev 0 SO mask: \n
       - 0x01 -- DPA \n

       CDMA EV-DO Rev A SO mask: \n
       - 0x01 -- DPA \n
       - 0x02 -- MFPA \n
       - 0x04 -- EMPA \n
       - 0x08 -- EMPA_EHRPD \n

       CDMA EV-DO Rev B SO mask: \n
       - 0x01 -- DPA \n
       - 0x02 -- MFPA \n
       - 0x04 -- EMPA \n
       - 0x08 -- EMPA_EHRPD \n
       - 0x10 -- MMPA \n
       - 0x20 -- MMPA_EHRPD
  */

  wds_rf_conditions_enum_v01 rf_cond;
  /**<   Values: \n
      - WDS_RF_CONDITIONS_INVALID (0) --  Invalid \n 
      - WDS_RF_CONDITIONS_BAD (1) --  Bad \n 
      - WDS_RF_CONDITIONS_GOOD (2) --  Good\n 
      - WDS_RF_CONDITIONS_DONT_CARE (3) --  Do not care 
 */
}wds_rf_conditions_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t handle;
  /**<   DOS ACK handle
  */

  uint32_t overflow;
  /**<   Set to a nonzero value when the number of outstanding SDB/DOS packets 
       (the packets for which the mobile is still waiting for an ACK) is more than 
       that the mobile can handle.
  */

  wds_dos_ack_status_enum_v01 dos_ack_status;
  /**<   DOS ACK status. Values: \n
      - WDS_DOS_ACK_NONE (-1) --  None \n 
      - WDS_DOS_ACK_OK (0x00000000) --  Completed successfully \n 
      - WDS_DOS_ACK_HOLD_ORIG_RETRY_TIMEOUT (0x00000001) --  Hold original retry timeout \n 
      - WDS_DOS_ACK_HOLD_ORIG (0x00000002) --  Cannot proceed because hold original is TRUE \n 
      - WDS_DOS_ACK_NO_SRV (0x00000003) --  No service \n 
      - WDS_DOS_ACK_ABORT (0x00000004) --  Abort \n 
      - WDS_DOS_ACK_NOT_ALLOWED_IN_AMPS (0x00000005) --  Cannot send in Analog mode \n 
      - WDS_DOS_ACK_NOT_ALLOWED_IN_HDR (0x00000006) --  Cannot send in an HDR call \n 
      - WDS_DOS_ACK_L2_ACK_FAILURE (0x00000007) --  Failure receiving the L2 ACK \n 
      - WDS_DOS_ACK_OUT_OF_RESOURCES (0x00000008) --  Out of resources, e.g., memory buffer is full \n 
      - WDS_DOS_ACK_ACCESS_TOO_LARGE (0x00000009) --  Message is too large to be sent over ACC \n 
      - WDS_DOS_ACK_DTC_TOO_LARGE (0x0000000A) --  Message is too large to be sent over DTC \n 
      - WDS_DOS_ACK_OTHER (0x0000000B) --  Any status response other than above \n 
      - WDS_DOS_ACK_ACCT_BLOCK (0x0000000C) --  Access is blocked based on the service option \n 
      - WDS_DOS_ACK_L3_ACK_FAILURE (0x0000000D) --  Failure receiving the L3 ACK 
 */
}wds_dos_ack_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the occurrence of IFACE events.  */
typedef struct {

  /* Mandatory */
  /*  IFACE Event */
  wds_iface_event_name_enum_v01 iface_event_name;
  /**<   Values: \n
      - WDS_IFACE_EVENT_OUTAGE_NOTIFICATION (0x0001) --  Outage notification \n 
      - WDS_IFACE_EVENT_EXT_IPCONFIG (0x0002) --  Extended IP configuration (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister})\n 
      - WDS_IFACE_EVENT_HDR_RATE_INERTIA_SUCCESS (0x0003) --  HDR Rev0 rate inertia success \n 
      - WDS_IFACE_EVENT_HDR_RATE_INERTIA_FAILURE (0x0004) --  HDR Rev0 rate inertia failure \n 
      - WDS_IFACE_EVENT_HDR_SM_SUCCESS (0x0005) --  HDR Set EIDLE Slotted Mode success (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister})\n 
      - WDS_IFACE_EVENT_HDR_SM_FAILURE (0x0006) --  HDR Set EIDLE Slotted Mode failure (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister})\n 
      - WDS_IFACE_EVENT_HDR_SM_SESS_CHANGE (0x0007) --  HDR Set EIDLE Slotted Mode session change (deprecated as described in 
Section \ref{hdr:InternalIFaceEvRegister}\n 
      - WDS_IFACE_EVENT_RF_CONDITIONS (0x0008) --  RF conditions change \n 
      - WDS_IFACE_EVENT_DOS_ACK (0x0009) --  DOS ACK event  
 */

  /* Optional */
  /*  Outage */
  uint8_t outage_valid;  /**< Must be set to true if outage is being passed */
  wds_outage_type_v01 outage;

  /* Optional */
  /*  Extended IP Configuration Status */
  uint8_t extended_ip_config_status_valid;  /**< Must be set to true if extended_ip_config_status is being passed */
  uint8_t extended_ip_config_status;
  /**<   Values: \n
       - 0x00 -- FAIL \n
       - 0x01 -- SUCCESS
  */

  /* Optional */
  /*  HDR Rev0 Rate Inertia Failure Code */
  uint8_t hdr_rate_intertia_fail_valid;  /**< Must be set to true if hdr_rate_intertia_fail is being passed */
  wds_hdr_rev0_rate_inertia_failure_code_enum_v01 hdr_rate_intertia_fail;
  /**<   Values: \n
      - WDS_HDR_REV0_RATE_INERTIA_REQUEST_REJECTED (0) --  Request rejected \n 
      - WDS_HDR_REV0_RATE_INERTIA_REQUEST_FAILED_TX (1) --  Request failed Tx \n 
      - WDS_HDR_REV0_RATE_INERTIA_NOT_SUPPORTED (2) --  Not supported \n 
      - WDS_HDR_REV0_RATE_INERTIA_NO_NET (3) --  No net  
 */

  /* Optional */
  /*  HDR Set EIDLE Slotted Mode Failure Code */
  uint8_t hdr_sm_fail_valid;  /**< Must be set to true if hdr_sm_fail is being passed */
  wds_hdr_slotted_mode_failure_code_enum_v01 hdr_sm_fail;
  /**<   Values: \n
      - WDS_HDR_SLOTTED_MODE_REQUEST_REJECTED (0) --  Request rejected \n 
      - WDS_HDR_SLOTTED_MODE_REQUEST_FAILED_TX (1) --  Request failed Tx \n 
      - WDS_HDR_SLOTTED_MODE_NOT_SUPPORTED (2) --  Not supported \n 
      - WDS_HDR_SLOTTED_MODE_NO_NET (3) --  No net 
 */

  /* Optional */
  /*  HDR Set EIDLE Slotted Mode Session Changed */
  uint8_t slot_cycle_changed_valid;  /**< Must be set to true if slot_cycle_changed is being passed */
  wds_evdo_slot_cycle_enum_v01 slot_cycle_changed;
  /**<   Slot cycle changed by the network.
  */

  /* Optional */
  /*  RF Conditions */
  uint8_t rf_conditions_valid;  /**< Must be set to true if rf_conditions is being passed */
  wds_rf_conditions_type_v01 rf_conditions;

  /* Optional */
  /*  DOS ACK Information */
  uint8_t dos_ack_info_valid;  /**< Must be set to true if dos_ack_info is being passed */
  wds_dos_ack_info_type_v01 dos_ack_info;
}wds_internal_iface_ev_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef uint32_t wds_req_oper_mask_v01;
#define QMI_WDS_MASK_REQ_OPER_RF_CONDITIONS_V01 ((wds_req_oper_mask_v01)0x01) /**<  RF conditions \n  */
#define QMI_WDS_MASK_REQ_OPER_1X_DORM_TIMER_V01 ((wds_req_oper_mask_v01)0x02) /**<  1X dorm timer \n  */
#define QMI_WDS_MASK_REQ_OPER_1X_SESSION_TIMER_V01 ((wds_req_oper_mask_v01)0x04) /**<  1X session timer \n  */
#define QMI_WDS_MASK_REQ_OPER_HDR_1X_HANDDOWN_OPT_V01 ((wds_req_oper_mask_v01)0x08) /**<  HDR-1X hand down option \n  */
#define QMI_WDS_MASK_REQ_OPER_HYSTERISIS_ACTIVATION_TIMER_V01 ((wds_req_oper_mask_v01)0x10) /**<  Hysteresis activation timer \n  */
#define QMI_WDS_MASK_REQ_OPER_HDR_EIDLE_SM_OPT_V01 ((wds_req_oper_mask_v01)0x20) /**<  HDR EIDLE Slotted Mode option \n  */
#define QMI_WDS_MASK_REQ_OPER_SDB_SUPPORT_V01 ((wds_req_oper_mask_v01)0x40) /**<  SDB support  */
/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_SESSION_TIMER_SELECT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_SESSION_TIMER_DO_V01 = 0x01, /**<  Session timer DO \n  */
  WDS_SESSION_TIMER_1X_V01 = 0x02, /**<  Session timer 1X \n  */
  WDS_SESSION_TIMER_1X_AND_DO_V01 = 0x03, /**<  Session timer 1X and DO   */
  WDS_SESSION_TIMER_SELECT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_session_timer_select_enum_v01;
/**
    @}
  */

typedef uint32_t wds_sdb_flags_mask_v01;
#define QMI_WDS_MASK_SDB_FLAGS_MSG_EXPEDITE_V01 ((wds_sdb_flags_mask_v01)0x01) /**<  Expedite the message \n  */
#define QMI_WDS_MASK_SDB_FLAGS_MSG_FAST_EXPEDITE_V01 ((wds_sdb_flags_mask_v01)0x02) /**<  Fast expedite the message   */
/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves internal packet data session settings currently in use.  */
typedef struct {

  /* Optional */
  /*  Requested Internal Settings */
  uint8_t requested_internal_settings_valid;  /**< Must be set to true if requested_internal_settings is being passed */
  wds_req_oper_mask_v01 requested_internal_settings;
  /**<   Requested internal settings bitmask. Values: \n
      - QMI_WDS_MASK_REQ_OPER_RF_CONDITIONS (0x01) --  RF conditions \n 
      - QMI_WDS_MASK_REQ_OPER_1X_DORM_TIMER (0x02) --  1X dorm timer \n 
      - QMI_WDS_MASK_REQ_OPER_1X_SESSION_TIMER (0x04) --  1X session timer \n 
      - QMI_WDS_MASK_REQ_OPER_HDR_1X_HANDDOWN_OPT (0x08) --  HDR-1X hand down option \n 
      - QMI_WDS_MASK_REQ_OPER_HYSTERISIS_ACTIVATION_TIMER (0x10) --  Hysteresis activation timer \n 
      - QMI_WDS_MASK_REQ_OPER_HDR_EIDLE_SM_OPT (0x20) --  HDR EIDLE Slotted Mode option \n 
      - QMI_WDS_MASK_REQ_OPER_SDB_SUPPORT (0x40) --  SDB support  \n

 Each bit set causes the corresponding optional TLV to be sent in 
 the QMI_WDS_GET_INTERNAL_RUNTIME_SETTINGS_RESP response. All unlisted bits are 
 reserved for future use and must be set to zero.
 */

  /* Optional */
  /*  Session Timer Select */
  uint8_t timer_select_valid;  /**< Must be set to true if timer_select is being passed */
  wds_session_timer_select_enum_v01 timer_select;
  /**<   Values: \n
      - WDS_SESSION_TIMER_DO (0x01) --  Session timer DO \n 
      - WDS_SESSION_TIMER_1X (0x02) --  Session timer 1X \n 
      - WDS_SESSION_TIMER_1X_AND_DO (0x03) --  Session timer 1X and DO  
 */

  /* Optional */
  /*  Requested SDB Flags */
  uint8_t flags_valid;  /**< Must be set to true if flags is being passed */
  wds_sdb_flags_mask_v01 flags;
  /**<   Requested SDB flags bitmask. Values: \n
      - QMI_WDS_MASK_SDB_FLAGS_MSG_EXPEDITE (0x01) --  Expedite the message \n 
      - QMI_WDS_MASK_SDB_FLAGS_MSG_FAST_EXPEDITE (0x02) --  Fast expedite the message  
 */
}wds_get_internal_runtime_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t bit_number;
  /**<   Bit in the requested_internal_settings mask for which the operation failed.
  */

  int16_t error_value;
  /**<   Error obtained from the operation; a data services error value returned by 
       the lower layers. Refer to the DS Error numbers published in \hyperref[Q4]{[Q4]} 
       for the possible values returned in this field.
  */
}wds_get_runtime_settings_error_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves internal packet data session settings currently in use.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  RF Conditions */
  uint8_t rf_conditions_valid;  /**< Must be set to true if rf_conditions is being passed */
  wds_rf_conditions_type_v01 rf_conditions;

  /* Optional */
  /*  1X Dorm Timer */
  uint8_t dorm_timer_valid;  /**< Must be set to true if dorm_timer is being passed */
  uint32_t dorm_timer;
  /**<   1X dorm timer. */

  /* Optional */
  /*  1X Session Timer */
  uint8_t session_timer_value_valid;  /**< Must be set to true if session_timer_value is being passed */
  int16_t session_timer_value;
  /**<   1X session timer. */

  /* Optional */
  /*  HDR-1X Hand Down Option */
  uint8_t hdr_1x_handdown_option_valid;  /**< Must be set to true if hdr_1x_handdown_option is being passed */
  uint8_t hdr_1x_handdown_option;
  /**<   Values: \n
       - 0x00 -- FALSE \n
       - 0x01 -- TRUE 
  */

  /* Optional */
  /*  Hysteresis Activation Timer */
  uint8_t hysterisis_act_timer_valid;  /**< Must be set to true if hysterisis_act_timer is being passed */
  int32_t hysterisis_act_timer;
  /**<   Hysteresis activation timer. */

  /* Optional */
  /*  HDR EIDLE Slotted Mode Option */
  uint8_t slotted_mode_option_valid;  /**< Must be set to true if slotted_mode_option is being passed */
  wds_evdo_slot_cycle_enum_v01 slotted_mode_option;
  /**<   This TLV is deprecated. Control points should use QMI_WDS_GET_EVDO_PAGE_ MONITOR_PERIOD_RESP
       instead.
  */

  /* Optional */
  /*  SDB Support */
  uint8_t sdb_support_valid;  /**< Must be set to true if sdb_support is being passed */
  uint8_t sdb_support;
  /**<   Values: \n
       - 0x00 -- FALSE \n
       - 0x01 -- TRUE 
  */

  /* Optional */
  /*  Operation Failure */
  uint8_t operation_failure_valid;  /**< Must be set to true if operation_failure is being passed */
  uint32_t operation_failure_len;  /**< Must be set to # of elements in operation_failure */
  wds_get_runtime_settings_error_v01 operation_failure[QMI_WDS_MAX_RUNTIME_SETTINGS_FAILURES_V01];
}wds_get_internal_runtime_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_session_timer_select_enum_v01 timer_select;
  /**<   Values: \n
      - WDS_SESSION_TIMER_DO (0x01) --  Session timer DO \n 
      - WDS_SESSION_TIMER_1X (0x02) --  Session timer 1X \n 
      - WDS_SESSION_TIMER_1X_AND_DO (0x03) --  Session timer 1X and DO   
 */

  int16_t timer_val;
  /**<   Timer value
  */
}wds_session_timer_1x_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets/modifies internal packet data session settings.  */
typedef struct {

  /* Optional */
  /*  Enable Hold Down */
  uint8_t enable_1x_holddown_valid;  /**< Must be set to true if enable_1x_holddown is being passed */
  uint8_t enable_1x_holddown;
  /**<   Enable hold down. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  1X Dorm Timer */
  uint8_t dorm_timer_valid;  /**< Must be set to true if dorm_timer is being passed */
  uint32_t dorm_timer;
  /**<   1X dorm timer value.
  */

  /* Optional */
  /*  1X Session Timer */
  uint8_t session_timer_valid;  /**< Must be set to true if session_timer is being passed */
  wds_session_timer_1x_v01 session_timer;
  /**<   \n 1X session timer value.
  */

  /* Optional */
  /*  HDR-1X Hand Down Option */
  uint8_t hdr_1x_handdown_option_valid;  /**< Must be set to true if hdr_1x_handdown_option is being passed */
  uint8_t hdr_1x_handdown_option;
  /**<   HDR-1X hand down option. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Hysteresis Activation Timer */
  uint8_t hysterisis_act_timer_valid;  /**< Must be set to true if hysterisis_act_timer is being passed */
  int32_t hysterisis_act_timer;
  /**<   Hysteresis activation timer.
  */

  /* Optional */
  /*  HDR Slotted Mode */
  uint8_t slotted_mode_option_valid;  /**< Must be set to true if slotted_mode_option is being passed */
  wds_evdo_slot_cycle_enum_v01 slotted_mode_option;
  /**<   Slot cycle value. This TLV is deprecated. Control points should use 
       QMI_WDS_SET_EVDO_ PAGE_MONITOR_PERIOD_REQ instead.
  */

  /* Optional */
  /*  Enable HDR HPT Mode */
  uint8_t enable_hdr_hpt_valid;  /**< Must be set to true if enable_hdr_hpt is being passed */
  uint8_t enable_hdr_hpt;
  /**<   Values: \n
       - 0x00 -- Disable (FALSE) \n
       - 0x01 -- Enable (TRUE)
  */

  /* Optional */
  /*  Enable HDR Rev0 Rate Inertia */
  uint8_t enable_hdr_rev0_rate_inertia_valid;  /**< Must be set to true if enable_hdr_rev0_rate_inertia is being passed */
  uint8_t enable_hdr_rev0_rate_inertia;
  /**<   Values: \n
       - 0x00 -- Disable (FALSE) \n
       - 0x01 -- Enable (TRUE)
  */
}wds_set_internal_runtime_settings_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t tlv_type;
  /**<   TLV type in the request that elicited the error.
  */

  int16_t error_value;
  /**<   Error obtained from the operation; a data services error value returned by 
       the lower layers. Refer to the DS Error numbers published in \hyperref[Q4]{[Q4]} 
       for the possible values returned in this field.
  */
}wds_set_runtime_settings_error_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets/modifies internal packet data session settings.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Operation Failure */
  uint8_t operation_failure_valid;  /**< Must be set to true if operation_failure is being passed */
  uint32_t operation_failure_len;  /**< Must be set to # of elements in operation_failure */
  wds_set_runtime_settings_error_v01 operation_failure[QMI_WDS_MAX_RUNTIME_SETTINGS_FAILURES_V01];
}wds_set_internal_runtime_settings_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Refreshes the DHCP configuration information.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_refresh_dhcp_config_info_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Refreshes the DHCP configuration information.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_refresh_dhcp_config_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the timer for generating a throughput information indication. */
typedef struct {

  /* Optional */
  /*  Report Interval */
  uint8_t report_interval_valid;  /**< Must be set to true if report_interval is being passed */
  uint32_t report_interval;
  /**<   Period at which throughput information 
       is generated, in milliseconds.
  */
}wds_set_throughput_info_ind_freq_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the timer for generating a throughput information indication. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_throughput_info_ind_freq_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_IP_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IP_TYPE_IPV4_V01 = 0, /**<  IPv4  */
  WDS_IP_TYPE_IPV6_V01 = 1, /**<  IPv6  */
  WDS_IP_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ip_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_PORT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_SIO_PORT_ID_V01 = 0, /**<  Data_port field is used \n  */
  WDS_END_POINT_ID_V01 = 1, /**<  Ep_type, iface_id, and mux_id fields are used  */
  WDS_PORT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_port_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_HIGHER_THROUGHPUT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_HIGHER_THROUGHPUT_UNKNOWN_V01 = 0, /**<  Throughput quality is unknown \n  */
  WDS_HIGHER_THROUGHPUT_NOT_POSSIBLE_V01 = 1, /**<  Best throughput possible \n  */
  WDS_HIGHER_THROUGHPUT_POSSIBLE_V01 = 2, /**<  Better throughput than current throughput is possible  */
  WDS_HIGHER_THROUGHPUT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_higher_throughput_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_TECHNOLOGY_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_TECHNOLOGY_TYPE_3GPP_V01 = 0, /**<  3GPP  */
  WDS_TECHNOLOGY_TYPE_3GPP2_V01 = 1, /**<  3GPP2   */
  WDS_TECHNOLOGY_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_technology_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t bearer_rlp_mac_id;
  /**<   Bearer ID representing the bearer,
       or RLP_MAC ID for which the throughput is being 
       reported. */

  uint32_t uplink_actual_rate;
  /**<    Uplink actual rate in kbits per second
        corresponding to the bearer or RLP_MAC ID. */

  uint32_t uplink_queue_size;
  /**<   Number of bytes pending in the uplink queue
       corresponding to the bearer or RLP_MAC ID. */

  uint8_t is_primary;
  /**<   Boolean value to determine if the bearer or RLP_MAC ID
       is the default. */
}wds_bearer_rlp_mac_id_throughput_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String representing the APN.
       Maximum length is 100 bytes.
  */

  wds_ip_type_enum_v01 ip_type;
  /**<   IP type. Values: \n
      - WDS_IP_TYPE_IPV4 (0) --  IPv4 
      - WDS_IP_TYPE_IPV6 (1) --  IPv6 
 */

  wds_technology_type_enum_v01 tech_type;
  /**<   Technology type.
 Values: \n
      - WDS_TECHNOLOGY_TYPE_3GPP (0) --  3GPP 
      - WDS_TECHNOLOGY_TYPE_3GPP2 (1) --  3GPP2  
 */

  wds_bind_subscription_enum_v01 subscription;
  /**<   Subscription to which the APN is bound.
 Values: \n
      - WDS_DEFAULT_SUBS (0x0000) --  Default data subscription \n 
      - WDS_PRIMARY_SUBS (0x0001) --  Primary \n 
      - WDS_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - WDS_TERTIARY_SUBS (0x0003) --  Tertiary \n 
      - WDS_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding
 */

  uint32_t uplink_actual_rate;
  /**<   Uplink actual rate in kbits per second.
  */

  uint32_t uplink_allowed_rate;
  /**<   Uplink allowed rate per UE in kbits per second.
  */

  uint32_t uplink_queue_size;
  /**<   Number of bytes pending in the uplink queue.
  */

  wds_higher_throughput_enum_v01 throughput_signal;
  /**<   Indicates whether the UE can have a better throughput 
 rate than the throughput reported currently.
      - WDS_HIGHER_THROUGHPUT_UNKNOWN (0) --  Throughput quality is unknown \n 
      - WDS_HIGHER_THROUGHPUT_NOT_POSSIBLE (1) --  Best throughput possible \n 
      - WDS_HIGHER_THROUGHPUT_POSSIBLE (2) --  Better throughput than current throughput is possible 
 */

  wds_port_type_enum_v01 valid_port;
  /**<   Indicates which of the following IDs are valid: \n
      - WDS_SIO_PORT_ID (0) --  Data_port field is used \n 
      - WDS_END_POINT_ID (1) --  Ep_type, iface_id, and mux_id fields are used 
 */

  uint16_t data_port;
  /**<   SIO data port to which the client binds.
  */

  data_ep_id_type_v01 ep_id;
  /**<   The peripheral end point of the RmNet instance 
       where the data call is already present.
  */

  uint8_t mux_id;
  /**<   Mux ID of the RmNet instance where the data call is already present.
  */

  uint32_t bearer_rlp_mac_id_throughput_info_len;  /**< Must be set to # of elements in bearer_rlp_mac_id_throughput_info */
  wds_bearer_rlp_mac_id_throughput_info_type_v01 bearer_rlp_mac_id_throughput_info[QMI_WDS_MAX_BEARER_RLP_MAC_ID_CONTEXT_V01];
  /**<   Throughput info per bearer or RLP_MAC ID.
  */
}wds_throughput_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries for the last reported throughput information. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_last_throughput_info_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries for the last reported throughput information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Throughput Information */
  uint8_t throughput_info_valid;  /**< Must be set to true if throughput_info is being passed */
  uint32_t throughput_info_len;  /**< Must be set to # of elements in throughput_info */
  wds_throughput_info_type_v01 throughput_info[QMI_WDS_MAX_THROUGHPUT_INFO_V01];
}wds_get_last_throughput_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates throughput information. */
typedef struct {

  /* Optional */
  /*  Throughput Information */
  uint8_t throughput_info_valid;  /**< Must be set to true if throughput_info is being passed */
  uint32_t throughput_info_len;  /**< Must be set to # of elements in throughput_info */
  wds_throughput_info_type_v01 throughput_info[QMI_WDS_MAX_THROUGHPUT_INFO_V01];
}wds_throughput_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
    - 0x00 -- 3GPP \n
    - 0x01 -- 3GPP2 \n
    - 0x02 -- EPC \n
    - 0xFF -- All technologies \n
              Value 0xFF is reserved; this value is used to register for 
              profile change events for all technologies 
              i.e., 3GPP, 3GPP2, and EPC.
  */

  uint8_t profile_index;
  /**<   The index of the configured profile on which data call parameters
       are based (other TLVs present override the profile settings).

       Value 0xFF is reserved; this value is used to register for profile 
       change events for all profiles tied to the technology type provided 
       in the profile_type field. If profile_type is specified as 0xFF, 
       the profile_index field is ignored.
  */
}wds_profile_event_register_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Registers for profile change events. */
typedef struct {

  /* Optional */
  /*  Profile Event Registration */
  uint8_t profile_event_register_valid;  /**< Must be set to true if profile_event_register is being passed */
  uint32_t profile_event_register_len;  /**< Must be set to # of elements in profile_event_register */
  wds_profile_event_register_type_v01 profile_event_register[QMI_WDS_PROFILE_LIST_MAX_V01];
}wds_config_profile_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Registers for profile change events. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_config_profile_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  wds_profile_type_enum_v01 profile_type;
  /**<   Identifies the technology type of the profile. Values: \n
      - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n 
      - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n 
      - WDS_PROFILE_TYPE_EPC (0x02) --  EPC 
 */

  uint8_t profile_index;
  /**<   Index of the configured profile on which data call parameters
       are based.
  */

  wds_profile_changed_event_enum_v01 profile_change_evt;
  /**<   Identifies the profile event that caused a change in the profile. Values:\n
      - WDS_PROFILE_CREATE_PROFILE_EVENT (0x01) --  Create profile event \n 
      - WDS_PROFILE_DELETE_PROFILE_EVENT (0x02) --  Delete profile event \n 
      - WDS_PROFILE_MODIFY_PROFILE_EVENT (0x03) --  Modify profile event \n 
      - WDS_PROFILE_SUBSCRIPTION_CHANGE_EVENT (0x04) --  Subscription changed event 
 */
}wds_profile_changed_ind_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the profile configured for reporting of change
           events. */
typedef struct {

  /* Optional */
  /*  Profile Event Registration Indication */
  uint8_t profile_changed_ind_valid;  /**< Must be set to true if profile_changed_ind is being passed */
  wds_profile_changed_ind_type_v01 profile_changed_ind;
}wds_profile_changed_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the modem capabilites. */
typedef struct {

  /* Optional */
  /*  eMBMS Extended EARFCN List */
  uint8_t extended_embms_frequency_valid;  /**< Must be set to true if extended_embms_frequency is being passed */
  uint8_t extended_embms_frequency;
  /**<   Supported frequency values. Values: \n
       - 0 -- 16-bit frequency \n
       - 1 -- Extended frequencies (32-bit values)
    */
}wds_get_capabilities_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the modem capabilites. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  eMBMS Extended EARFCN List */
  uint8_t extended_embms_frequency_valid;  /**< Must be set to true if extended_embms_frequency is being passed */
  uint8_t extended_embms_frequency;
  /**<   Supported frequency values. Values: \n
       - 0 -- 16-bit frequency \n
       - 1 -- Extended frequencies (32-bit values)
    */
}wds_get_capabilities_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  char apn_name[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String parameter that is a logical name used to
         select the GGSN and external packet data network.
    */
}wds_apn_name_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Retrieves APN names during roaming. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_roaming_info_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Retrieves APN names during roaming. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  APN Name List */
  uint8_t roaming_apn_name_list_valid;  /**< Must be set to true if roaming_apn_name_list is being passed */
  uint32_t roaming_apn_name_list_len;  /**< Must be set to # of elements in roaming_apn_name_list */
  wds_apn_name_list_type_v01 roaming_apn_name_list[QMI_WDS_APN_NAME_LIST_MAX_V01];
}wds_get_roaming_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates APN names during roaming. */
typedef struct {

  /* Optional */
  /*  APN Name List */
  uint8_t roaming_apn_name_list_valid;  /**< Must be set to true if roaming_apn_name_list is being passed */
  uint32_t roaming_apn_name_list_len;  /**< Must be set to # of elements in roaming_apn_name_list */
  wds_apn_name_list_type_v01 roaming_apn_name_list[QMI_WDS_APN_NAME_LIST_MAX_V01];
}wds_roaming_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries the delegated IPv6 prefix. */
typedef struct {

  /* Mandatory */
  /*  Source IPv6 Address */
  uint8_t src_ipv6_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Link local address of the client
       IPv6 address (in network byte order). The address is a 
       16-element array of 8-bit numbers, each of which is in
       big-endian format.
  */
}wds_get_delegated_ipv6_prefix_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries the delegated IPv6 prefix. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IPv6 Prefix and Prefix Length */
  uint8_t ipv6_prefix_valid;  /**< Must be set to true if ipv6_prefix is being passed */
  wds_runtime_ipv6_addr_type_v01 ipv6_prefix;
}wds_get_delegated_ipv6_prefix_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Removes the delegated IPv6 prefix. */
typedef struct {

  /* Optional */
  /*  IPv6 Prefix and Prefix Length */
  uint8_t ipv6_prefix_valid;  /**< Must be set to true if ipv6_prefix is being passed */
  wds_runtime_ipv6_addr_type_v01 ipv6_prefix;

  /* Optional */
  /*  Host Link Local IPv6 Address */
  uint8_t host_ll_addr_valid;  /**< Must be set to true if host_ll_addr is being passed */
  uint8_t host_ll_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   Link local address of the IPv6 client for 
       which the matching delegated prefix is to 
       be removed (in network byte order). The address is a
       16-element array of 8-bit numbers, each of which 
       is in big-endian format.
  */
}wds_remove_delegated_ipv6_prefix_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Removes the delegated IPv6 prefix. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_remove_delegated_ipv6_prefix_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Aborts the previously issued QMI_WDS_GO_DORMANT command. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_abort_go_dormant_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Aborts the previously issued QMI_WDS_GO_DORMANT command. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_abort_go_dormant_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Binds the control point to the specified subscription. */
typedef struct {

  /* Mandatory */
  /*  Subscription Identifier */
  wds_bind_subscription_enum_v01 subscription;
  /**<   Subscription to which the client is bound.
 Values: \n
      - WDS_DEFAULT_SUBS (0x0000) --  Default data subscription \n 
      - WDS_PRIMARY_SUBS (0x0001) --  Primary \n 
      - WDS_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - WDS_TERTIARY_SUBS (0x0003) --  Tertiary \n 
      - WDS_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding
 */
}wds_bind_subscription_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Binds the control point to the specified subscription. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries for the current subscription of the control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_bind_subscription_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries for the current subscription of the control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Subscription */
  uint8_t subscription_valid;  /**< Must be set to true if subscription is being passed */
  wds_bind_subscription_enum_v01 subscription;
  /**<   Subscription to which the client is bound.
 Values: \n
      - WDS_DEFAULT_SUBS (0x0000) --  Default data subscription \n 
      - WDS_PRIMARY_SUBS (0x0001) --  Primary \n 
      - WDS_SECONDARY_SUBS (0x0002) --  Secondary \n 
      - WDS_TERTIARY_SUBS (0x0003) --  Tertiary \n 
      - WDS_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding
 */
}wds_get_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_LTE_DATA_CALL_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_LTE_CALL_TYPE_DEFAULT_V01 = 0, /**<  Default LTE data call type \n  */
  WDS_LTE_CALL_TYPE_VOLTE_V01 = 1, /**<  Voice over LTE data call type  */
  WDS_LTE_DATA_CALL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_lte_data_call_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the data call type for an active LTE call. */
typedef struct {

  /* Mandatory */
  /*  LTE Data Call Type Identifier */
  wds_lte_data_call_type_enum_v01 call_type;
  /**<   Type of LTE Data call that must be set.
 Values: \n
      - WDS_LTE_CALL_TYPE_DEFAULT (0) --  Default LTE data call type \n 
      - WDS_LTE_CALL_TYPE_VOLTE (1) --  Voice over LTE data call type 
 */
}wds_set_lte_data_call_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the data call type for an active LTE call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_lte_data_call_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Sets the timer for generating downlink throughput information
           indication. */
typedef struct {

  /* Mandatory */
  /*  Downlink Throughput Report Interval */
  uint32_t downlink_report_interval;
  /**<   Integer multiple of the minimun reporting period at which downlink
       throughput information is generated.
  */
}wds_set_downlink_throughput_info_ind_freq_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Sets the timer for generating downlink throughput information
           indication. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_downlink_throughput_info_ind_freq_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates downlink throughput information. */
typedef struct {

  /* Optional */
  /*  Downlink allowed rate */
  uint8_t downlink_allowed_rate_valid;  /**< Must be set to true if downlink_allowed_rate is being passed */
  uint32_t downlink_allowed_rate;
  /**<   The allowed downlink rate per UE in Kbps.
  */

  /* Optional */
  /*  Confidence level */
  uint8_t confidence_level_valid;  /**< Must be set to true if confidence_level is being passed */
  uint8_t confidence_level;
  /**<   Level of accuracy at which the throughput information
       is generated.(On a scale of 0-7, 0 being the least 
       accurate and 7 indicates high accuracy of reporting)
  */
}wds_downlink_throughput_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Queries for the downlink throughout info params */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wds_get_downlink_throughput_info_params_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Queries for the downlink throughout info params */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Downlink minimum timer interval */
  uint8_t min_dl_interval_valid;  /**< Must be set to true if min_dl_interval is being passed */
  uint32_t min_dl_interval;
  /**<   Min downlink interval in milliseconds
  */
}wds_get_downlink_throughput_info_params_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_SERVICE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_SERVICE_DASH_V01 = 0, /**<  DASH streaming \n  */
  WDS_EMBMS_SERVICE_FILE_DELIVERY_V01 = 1, /**<  File delivery \n  */
  WDS_EMBMS_SERVICE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_service_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; eMBMS content description parameters update */
typedef struct {

  /* Mandatory */
  /*  Temporary Mobile Group Identity */
  wds_embms_tmgi_type_v01 tmgi;

  /* Mandatory */
  /*  Transaction ID */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request.
  */

  /* Optional */
  /*  Object Delivery Duration */
  uint8_t obj_delivery_duration_valid;  /**< Must be set to true if obj_delivery_duration is being passed */
  uint32_t obj_delivery_duration;
  /**<   Segment duration for streaming. Time in milliseconds.
  */

  /* Optional */
  /*  Session FEC Redundancy Level */
  uint8_t session_fec_redundancy_lvl_valid;  /**< Must be set to true if session_fec_redundancy_lvl is being passed */
  uint32_t session_fec_redundancy_lvl;
  /**<   Ratio of repair to source symbols.
       Percentage in hundreds of a percent.
  */

  /* Optional */
  /*  Service Type */
  uint8_t service_type_valid;  /**< Must be set to true if service_type is being passed */
  wds_embms_service_type_enum_v01 service_type;
  /**<   Service Type. Values: \n
      - WDS_EMBMS_SERVICE_DASH (0) --  DASH streaming \n 
      - WDS_EMBMS_SERVICE_FILE_DELIVERY (1) --  File delivery \n 
 */

  /* Optional */
  /*  Priority */
  uint8_t priority_valid;  /**< Must be set to true if priority is being passed */
  uint32_t priority;
  /**<   Priority level vs. Tune-away. 0 means no priority.
  */

  /* Optional */
  /*  Media Data Rate 1 */
  uint8_t media_data_rate_1_valid;  /**< Must be set to true if media_data_rate_1 is being passed */
  uint32_t media_data_rate_1;
  /**<   Data rate (in kbps) for a representation.
  */

  /* Optional */
  /*  Media Data Rate 2 */
  uint8_t media_data_rate_2_valid;  /**< Must be set to true if media_data_rate_2 is being passed */
  uint32_t media_data_rate_2;
  /**<   Data rate (in kbps) for a representation.
  */

  /* Optional */
  /*  Media Data Rate 3 */
  uint8_t media_data_rate_3_valid;  /**< Must be set to true if media_data_rate_3 is being passed */
  uint32_t media_data_rate_3;
  /**<   Data rate (in kbps) for a representation.
  */

  /* Optional */
  /*  Media Data Rate 4 */
  uint8_t media_data_rate_4_valid;  /**< Must be set to true if media_data_rate_4 is being passed */
  uint32_t media_data_rate_4;
  /**<   Data rate (in kbps) for a representation.
  */

  /* Optional */
  /*  Transport Session Identifier */
  uint8_t tsi_valid;  /**< Must be set to true if tsi is being passed */
  uint32_t tsi;
  /**<   TSI of object.
  */

  /* Optional */
  /*  Transport Object Identifier */
  uint8_t toi_valid;  /**< Must be set to true if toi is being passed */
  uint32_t toi;
  /**<   TOI of object.
  */

  /* Optional */
  /*  Object Size */
  uint8_t obj_size_valid;  /**< Must be set to true if obj_size is being passed */
  uint32_t obj_size;
  /**<   Object size in bytes.
  */

  /* Optional */
  /*  Symbol Size */
  uint8_t symbol_size_valid;  /**< Must be set to true if symbol_size is being passed */
  uint32_t symbol_size;
  /**<   Object symbol size in bytes.
  */

  /* Optional */
  /*  Object FEC Redundancy Level */
  uint8_t obj_fec_redundancy_lvl_valid;  /**< Must be set to true if obj_fec_redundancy_lvl is being passed */
  uint32_t obj_fec_redundancy_lvl;
  /**<   Ration of repair to source symbols for object.
       Overrides Session FEC Redundancy Level if present.
       Percentage in hundreds of a percent.
  */

  /* Optional */
  /*  Status Transport Session Identifier */
  uint8_t status_tsi_valid;  /**< Must be set to true if status_tsi is being passed */
  uint32_t status_tsi;
  /**<   TSI of object for which status is provided. If this parameter
       is present, it is always followed by Status Transport Object
       Identifier TLV and Status TLV.
  */

  /* Optional */
  /*  Status Transport Object Identifier */
  uint8_t status_toi_valid;  /**< Must be set to true if status_toi is being passed */
  uint32_t status_toi;
  /**<   TOI of object for which status is provided.
  */

  /* Optional */
  /*  Status */
  uint8_t status_valid;  /**< Must be set to true if status is being passed */
  uint32_t status;
  /**<   Success and Failure indication. Values: \n
       - 0   -- Success \n
       - >=1 -- Failure 
  */

  /* Optional */
  /*  Status Last Decoding ESI */
  uint8_t status_last_decoding_esi_valid;  /**< Must be set to true if status_last_decoding_esi is being passed */
  uint32_t status_last_decoding_esi;
  /**<   ESI of last symbol used for decoding the current object.
       Sent only for successfully decoded objects.
  */
}wds_embms_content_desc_update_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; eMBMS content description parameters update */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_embms_content_desc_update_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_CONTENT_CONTROL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_CC_DISABLE_V01 = 0, /**<  Disable \n  */
  WDS_EMBMS_CC_ENABLE_START_OBJ_V01 = 1, /**<  Enable at start of objects \n  */
  WDS_EMBMS_CC_ENABLE_START_VIDEO_OBJ_V01 = 2, /**<  Enable at start of video objects \n  */
  WDS_EMBMS_CONTENT_CONTROL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_content_control_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_EMBMS_STATUS_CONTROL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EMBMS_SU_DISABLE_V01 = 0, /**<  Disable \n  */
  WDS_EMBMS_SU_ENABLE_OBJ_V01 = 1, /**<  Enable for objects \n  */
  WDS_EMBMS_SU_ENABLE_VIDEO_OBJ_V01 = 2, /**<  Enable for video objects \n  */
  WDS_EMBMS_STATUS_CONTROL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_embms_status_control_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates eMBMS content description settings */
typedef struct {

  /* Mandatory */
  /*  Temporary Mobile Group Identity */
  wds_embms_tmgi_type_v01 tmgi;

  /* Optional */
  /*  Transaction ID */
  uint8_t tranx_id_valid;  /**< Must be set to true if tranx_id is being passed */
  int16_t tranx_id;
  /**<   eMBMS transaction ID for the request (default is -1).
  */

  /* Optional */
  /*  Per Object Content Control */
  uint8_t content_control_valid;  /**< Must be set to true if content_control is being passed */
  wds_embms_content_control_enum_v01 content_control;
  /**<   Per object content control. Values: \n
      - WDS_EMBMS_CC_DISABLE (0) --  Disable \n 
      - WDS_EMBMS_CC_ENABLE_START_OBJ (1) --  Enable at start of objects \n 
      - WDS_EMBMS_CC_ENABLE_START_VIDEO_OBJ (2) --  Enable at start of video objects \n 
 */

  /* Optional */
  /*  Per Object Status Control */
  uint8_t status_control_valid;  /**< Must be set to true if status_control is being passed */
  wds_embms_status_control_enum_v01 status_control;
  /**<   Per object status control. Values: \n
      - WDS_EMBMS_SU_DISABLE (0) --  Disable \n 
      - WDS_EMBMS_SU_ENABLE_OBJ (1) --  Enable for objects \n 
      - WDS_EMBMS_SU_ENABLE_VIDEO_OBJ (2) --  Enable for video objects \n 
 */
}wds_embms_content_desc_control_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_POLICY_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_POLICY_TYPE_ANDSF_V01 = 0, /**<  ANDSF  */
  WDS_POLICY_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_policy_type_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Refreshes the specified policy */
typedef struct {

  /* Mandatory */
  /*  Policy Type */
  wds_policy_type_enum_v01 policy_type;
  /**<   Policy type that needs to be refreshed. Values : \n
      - WDS_POLICY_TYPE_ANDSF (0) --  ANDSF 
 */
}wds_policy_refresh_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Refreshes the specified policy */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}wds_policy_refresh_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_enums
    @{
  */
typedef enum {
  WDS_POLICY_REFRESH_RESULT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_POLICY_REFRESH_RESULT_SUCCESS_V01 = 0, /**<  Success  */
  WDS_POLICY_REFRESH_RESULT_HTTP_FAILURE_V01 = 1, /**<  HTTP Failure  */
  WDS_POLICY_REFRESH_RESULT_NW_BRINGUP_FAILURE_V01 = 2, /**<  Network bringup failure  */
  WDS_POLICY_REFRESH_RESULT_START_FTP_SERVER_FAILURE_V01 = 3, /**<  Start FTP Server Failure  */
  WDS_POLICY_REFRESH_RESULT_LL_IFACE_DOWN_FAILURE_V01 = 4, /**<  Link Local IFACE Down  */
  WDS_POLICY_REFRESH_RESULT_OTHER_FAILURE_V01 = 5, /**<  Other Unknown Error  */
  WDS_POLICY_REFRESH_RESULT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_policy_refresh_result_enum_v01;
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates the result of the attempt to refresh the policy */
typedef struct {

  /* Mandatory */
  /*  Policy Refresh Status */
  wds_policy_refresh_result_enum_v01 status;
  /**<   Values: \n
       @ENUM
  */
}wds_policy_refresh_result_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates that a policy file is ready. */
typedef struct {

  /* Mandatory */
  /*  Policy Type */
  wds_policy_type_enum_v01 policy_type;
  /**<   Type of policy thats ready for transfer. Values: \n
      - WDS_POLICY_TYPE_ANDSF (0) --  ANDSF 
 */

  /* Optional */
  /*  FTP Server IPV6 Address */
  uint8_t ftp_ipv6_addr_valid;  /**< Must be set to true if ftp_ipv6_addr is being passed */
  uint8_t ftp_ipv6_addr[QMI_WDS_IPV6_ADDR_LEN_V01];
  /**<   IPV6 address of FTP server on the modem */

  /* Optional */
  /*  TCP Port */
  uint8_t tcp_port_valid;  /**< Must be set to true if tcp_port is being passed */
  uint16_t tcp_port;
  /**<   TCP Port of FTP Server */

  /* Optional */
  /*  Policy File Name */
  uint8_t policy_file_name_valid;  /**< Must be set to true if policy_file_name is being passed */
  char policy_file_name[QMI_WDS_MAX_FILE_NAME_V01 + 1];
  /**<   File name */
}wds_policy_ready_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_aggregates
    @{
  */
typedef struct {

  char apn_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
  /**<   String parameter to store APN string*/
}wds_apn_param_apn_string_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Indication Message; Indicates old and new APN parameter information for an active data call. */
typedef struct {

  /* Optional */
  /*  Old and New APN String Info */
  uint8_t changed_apn_string_info_valid;  /**< Must be set to true if changed_apn_string_info is being passed */
  wds_apn_param_apn_string_type_v01 changed_apn_string_info[QMI_WDS_MAX_APN_INFO_ARRAY_SIZE_V01];
  /**<   Array containing old and new APN string information.
     The first value of array points to old APN string
     and the second value of array points to new APN string*/

  /* Optional */
  /*  Old and New PDP Type Info */
  uint8_t changed_pdp_type_info_valid;  /**< Must be set to true if changed_pdp_type_info is being passed */
  wds_pdp_type_enum_v01 changed_pdp_type_info[QMI_WDS_MAX_APN_INFO_ARRAY_SIZE_V01];
  /**<   Container to store old and new PDN's IP Type
 The first value of array points to old PDN's IP Type
 and the second value of array points to new PDN's IP Type 
 Values: \n
      - WDS_PDP_TYPE_PDP_IPV4 (0x00) -- 
      - WDS_PDP_TYPE_PDP_PPP (0x01) -- 
      - WDS_PDP_TYPE_PDP_IPV6 (0x02) -- 
      - WDS_PDP_TYPE_PDP_IPV4V6 (0x03) -- 
 */
}wds_apn_param_info_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Request Message; Notifies the modem to perform silent redial */
typedef struct {

  /* Optional */
  /*  Set Silent Redial */
  uint8_t set_silent_redial_valid;  /**< Must be set to true if set_silent_redial is being passed */
  uint8_t set_silent_redial;
  /**<   Value to specify if silent redial is required or not. Values: \n
       - 0 -- Silent Redial is not required \n
       - 1 -- Silent Redial is required.
    */
}wds_set_silent_redial_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wds_qmi_messages
    @{
  */
/** Response Message; Notifies the modem to perform silent redial */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wds_set_silent_redial_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_WDS_ABORT_V01 
//#define REMOVE_QMI_WDS_ABORT_GO_DORMANT_V01 
//#define REMOVE_QMI_WDS_APN_PARAM_INFO_CHANGE_IND_V01 
//#define REMOVE_QMI_WDS_BIND_DATA_PORT_V01 
//#define REMOVE_QMI_WDS_BIND_MUX_DATA_PORT_V01 
//#define REMOVE_QMI_WDS_BIND_SUBSCRIPTION_V01 
//#define REMOVE_QMI_WDS_CALL_HISTORY_DELETE_V01 
//#define REMOVE_QMI_WDS_CALL_HISTORY_LIST_V01 
//#define REMOVE_QMI_WDS_CALL_HISTORY_MAX_SIZE_V01 
//#define REMOVE_QMI_WDS_CALL_HISTORY_READ_V01 
//#define REMOVE_QMI_WDS_CONFIGURE_PROFILE_EVENT_LIST_V01 
//#define REMOVE_QMI_WDS_CONTROL_PENDING_DUN_CALL_V01 
//#define REMOVE_QMI_WDS_CREATE_PROFILE_V01 
//#define REMOVE_QMI_WDS_DELETE_PROFILE_V01 
//#define REMOVE_QMI_WDS_DOWNLINK_THROUGHPUT_INFO_IND_V01 
//#define REMOVE_QMI_WDS_DUN_CALL_INFO_IND_V01 
//#define REMOVE_QMI_WDS_DUN_CTRL_EVENT_REPORT_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_CONTENT_DESC_UPDATE_V01 
//#define REMOVE_QMI_WDS_EMBMS_SAI_LIST_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_SAI_LIST_QUERY_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_ACTIVATE_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_ACT_DEACT_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_DEACTIVATE_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_LIST_IND_V01 
//#define REMOVE_QMI_WDS_EMBMS_TMGI_LIST_QUERY_V01 
//#define REMOVE_QMI_WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_IND_V01 
//#define REMOVE_QMI_WDS_EXTENDED_IP_CONFIG_IND_V01 
//#define REMOVE_QMI_WDS_FMC_CLEAR_TUNNEL_PARAMS_V01 
//#define REMOVE_QMI_WDS_FMC_GET_TUNNEL_PARAMS_V01 
//#define REMOVE_QMI_WDS_FMC_SET_TUNNEL_PARAMS_V01 
//#define REMOVE_QMI_WDS_GET_ACTIVE_MIP_PROFILE_V01 
//#define REMOVE_QMI_WDS_GET_AUTOCONNECT_SETTING_V01 
//#define REMOVE_QMI_WDS_GET_BIND_SUBSCRIPTION_V01 
//#define REMOVE_QMI_WDS_GET_CALL_DURATION_V01 
//#define REMOVE_QMI_WDS_GET_CALL_THROTTLE_INFO_V01 
//#define REMOVE_QMI_WDS_GET_CAM_TIMER_V01 
//#define REMOVE_QMI_WDS_GET_CAPABILITIES_V01 
//#define REMOVE_QMI_WDS_GET_CURRENT_CHANNEL_RATE_V01 
//#define REMOVE_QMI_WDS_GET_CURRENT_DATA_BEARER_TECHNOLOGY_V01 
//#define REMOVE_QMI_WDS_GET_CURRENT_DATA_SYSTEM_STATUS_V01 
//#define REMOVE_QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_V01 
//#define REMOVE_QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_EX_V01 
//#define REMOVE_QMI_WDS_GET_DATA_PATH_V01 
//#define REMOVE_QMI_WDS_GET_DEFAULT_PROFILE_NUM_V01 
//#define REMOVE_QMI_WDS_GET_DEFAULT_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_DELEGATED_IPV6_PREFIX_V01 
//#define REMOVE_QMI_WDS_GET_DNS_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_DORMANCY_STATUS_V01 
//#define REMOVE_QMI_WDS_GET_DOWNLINK_THROUGHPUT_INFO_PARAMS_V01 
//#define REMOVE_QMI_WDS_GET_DUN_CALL_INFO_V01 
//#define REMOVE_QMI_WDS_GET_DUN_CTRL_INFO_V01 
//#define REMOVE_QMI_WDS_GET_EVDO_PAGE_MONITOR_PERIOD_V01 
//#define REMOVE_QMI_WDS_GET_FLOW_CONTROL_STATUS_V01 
//#define REMOVE_QMI_WDS_GET_INTERNAL_RUNTIME_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_IPSEC_STATIC_SA_CONFIG_V01 
//#define REMOVE_QMI_WDS_GET_LAST_DATA_CALL_STATUS_V01 
//#define REMOVE_QMI_WDS_GET_LAST_MIP_STATUS_V01 
//#define REMOVE_QMI_WDS_GET_LAST_THROUGHPUT_INFO_V01 
//#define REMOVE_QMI_WDS_GET_LTE_ATTACH_PARAMS_V01 
//#define REMOVE_QMI_WDS_GET_LTE_ATTACH_PDN_LIST_V01 
//#define REMOVE_QMI_WDS_GET_LTE_ATTACH_TYPE_V01 
//#define REMOVE_QMI_WDS_GET_LTE_DATA_RETRY_V01 
//#define REMOVE_QMI_WDS_GET_LTE_MAX_ATTACH_PDN_NUM_V01 
//#define REMOVE_QMI_WDS_GET_MIP_MODE_V01 
//#define REMOVE_QMI_WDS_GET_MIP_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_NSAPI_V01 
//#define REMOVE_QMI_WDS_GET_PDN_THROTTLE_INFO_V01 
//#define REMOVE_QMI_WDS_GET_PKT_SRVC_STATUS_V01 
//#define REMOVE_QMI_WDS_GET_PKT_SRVC_STATUS_IND_V01 
//#define REMOVE_QMI_WDS_GET_PKT_STATISTICS_V01 
//#define REMOVE_QMI_WDS_GET_PREFERRED_DATA_SYSTEM_V01 
//#define REMOVE_QMI_WDS_GET_PRE_DORMANCY_CDMA_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_PROFILE_LIST_V01 
//#define REMOVE_QMI_WDS_GET_PROFILE_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_RDUD_V01 
//#define REMOVE_QMI_WDS_GET_ROAMING_INFO_V01 
//#define REMOVE_QMI_WDS_GET_RUNTIME_SETTINGS_V01 
//#define REMOVE_QMI_WDS_GET_SCRM_V01 
//#define REMOVE_QMI_WDS_GET_SIP_MIP_CALL_TYPE_V01 
//#define REMOVE_QMI_WDS_GET_SUPPORTED_FIELDS_V01 
//#define REMOVE_QMI_WDS_GET_SUPPORTED_MSGS_V01 
//#define REMOVE_QMI_WDS_GO_ACTIVE_V01 
//#define REMOVE_QMI_WDS_GO_DORMANT_V01 
//#define REMOVE_QMI_WDS_HANDOFF_INFORMATION_IND_V01 
//#define REMOVE_QMI_WDS_INDICATION_REGISTER_V01 
//#define REMOVE_QMI_WDS_INITIATE_ESP_REKEY_V01 
//#define REMOVE_QMI_WDS_INTERNAL_IFACE_EV_IND_V01 
//#define REMOVE_QMI_WDS_INTERNAL_IFACE_EV_REGISTER_V01 
//#define REMOVE_QMI_WDS_LTE_ATTACH_PDN_LIST_IND_V01 
//#define REMOVE_QMI_WDS_MODIFY_MIP_PROFILE_V01 
//#define REMOVE_QMI_WDS_MODIFY_PROFILE_SETTINGS_V01 
//#define REMOVE_QMI_WDS_POLICY_READY_IND_V01 
//#define REMOVE_QMI_WDS_POLICY_REFRESH_V01 
//#define REMOVE_QMI_WDS_POLICY_REFRESH_RESULT_IND_V01 
//#define REMOVE_QMI_WDS_PROFILE_CHANGED_IND_V01 
//#define REMOVE_QMI_WDS_READ_MIP_PROFILE_V01 
//#define REMOVE_QMI_WDS_REFRESH_DHCP_CONFIG_INFO_V01 
//#define REMOVE_QMI_WDS_REMOVE_ADDITIONAL_PDN_FILTER_V01 
//#define REMOVE_QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_V01 
//#define REMOVE_QMI_WDS_RESET_V01 
//#define REMOVE_QMI_WDS_RESET_PKT_STATISTICS_V01 
//#define REMOVE_QMI_WDS_RESET_PROFILE_PARAM_TO_INVALID_V01 
//#define REMOVE_QMI_WDS_RESET_PROFILE_TO_DEFAULT_V01 
//#define REMOVE_QMI_WDS_REVERSE_IP_TRANSPORT_CONFIG_COMPLETE_V01 
//#define REMOVE_QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_V01 
//#define REMOVE_QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_REGISTRATION_V01 
//#define REMOVE_QMI_WDS_REVERSE_IP_TRANSPORT_FILTER_SETUP_IND_V01 
//#define REMOVE_QMI_WDS_ROAMING_INFO_IND_V01 
//#define REMOVE_QMI_WDS_SET_ACTIVE_MIP_PROFILE_V01 
//#define REMOVE_QMI_WDS_SET_ADDITIONAL_PDN_FILTER_V01 
//#define REMOVE_QMI_WDS_SET_AUTOCONNECT_SETTINGS_V01 
//#define REMOVE_QMI_WDS_SET_CAM_TIMER_V01 
//#define REMOVE_QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_V01 
//#define REMOVE_QMI_WDS_SET_DATA_PATH_V01 
//#define REMOVE_QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 
//#define REMOVE_QMI_WDS_SET_DNS_SETTINGS_V01 
//#define REMOVE_QMI_WDS_SET_DOWNLINK_THROUGHPUT_INFO_IND_FREQ_V01 
//#define REMOVE_QMI_WDS_SET_DUN_CTRL_EVENT_REPORT_V01 
//#define REMOVE_QMI_WDS_SET_DUN_CTRL_PREF_V01 
//#define REMOVE_QMI_WDS_SET_EVDO_FORCE_LONG_SLEEP_V01 
//#define REMOVE_QMI_WDS_SET_EVDO_PAGE_MONITOR_PERIOD_V01 
//#define REMOVE_QMI_WDS_SET_EVENT_REPORT_V01 
//#define REMOVE_QMI_WDS_SET_EVENT_REPORT_IND_V01 
//#define REMOVE_QMI_WDS_SET_INTERNAL_RUNTIME_SETTINGS_V01 
//#define REMOVE_QMI_WDS_SET_LTE_ATTACH_PDN_LIST_V01 
//#define REMOVE_QMI_WDS_SET_LTE_ATTACH_TYPE_V01 
//#define REMOVE_QMI_WDS_SET_LTE_DATA_CALL_TYPE_V01 
//#define REMOVE_QMI_WDS_SET_LTE_DATA_RETRY_V01 
//#define REMOVE_QMI_WDS_SET_MIP_MODE_V01 
//#define REMOVE_QMI_WDS_SET_MIP_SETTINGS_V01 
//#define REMOVE_QMI_WDS_SET_RDUD_V01 
//#define REMOVE_QMI_WDS_SET_SCRM_V01 
//#define REMOVE_QMI_WDS_SET_SILENT_REDIAL_V01 
//#define REMOVE_QMI_WDS_SET_THROUGHPUT_INFO_IND_FREQ_V01 
//#define REMOVE_QMI_WDS_START_NETWORK_INTERFACE_V01 
//#define REMOVE_QMI_WDS_STOP_NETWORK_INTERFACE_V01 
//#define REMOVE_QMI_WDS_THROUGHPUT_INFO_IND_V01 
//#define REMOVE_QMI_WDS_UPDATE_LTE_ATTACH_PDN_LIST_PROFILES_V01 

/*Service Message Definition*/
/** @addtogroup wds_qmi_msg_ids
    @{
  */
#define QMI_WDS_RESET_REQ_V01 0x0000
#define QMI_WDS_RESET_RESP_V01 0x0000
#define QMI_WDS_SET_EVENT_REPORT_REQ_V01 0x0001
#define QMI_WDS_SET_EVENT_REPORT_RESP_V01 0x0001
#define QMI_WDS_EVENT_REPORT_IND_V01 0x0001
#define QMI_WDS_ABORT_REQ_V01 0x0002
#define QMI_WDS_ABORT_RESP_V01 0x0002
#define QMI_WDS_INDICATION_REGISTER_REQ_V01 0x0003
#define QMI_WDS_INDICATION_REGISTER_RESP_V01 0x0003
#define QMI_WDS_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_WDS_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_WDS_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_WDS_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_WDS_START_NETWORK_INTERFACE_REQ_V01 0x0020
#define QMI_WDS_START_NETWORK_INTERFACE_RESP_V01 0x0020
#define QMI_WDS_STOP_NETWORK_INTERFACE_REQ_V01 0x0021
#define QMI_WDS_STOP_NETWORK_INTERFACE_RESP_V01 0x0021
#define QMI_WDS_GET_PKT_SRVC_STATUS_REQ_V01 0x0022
#define QMI_WDS_GET_PKT_SRVC_STATUS_RESP_V01 0x0022
#define QMI_WDS_PKT_SRVC_STATUS_IND_V01 0x0022
#define QMI_WDS_GET_CURRENT_CHANNEL_RATE_REQ_V01 0x0023
#define QMI_WDS_GET_CURRENT_CHANNEL_RATE_RESP_V01 0x0023
#define QMI_WDS_GET_PKT_STATISTICS_REQ_V01 0x0024
#define QMI_WDS_GET_PKT_STATISTICS_RESP_V01 0x0024
#define QMI_WDS_GO_DORMANT_REQ_V01 0x0025
#define QMI_WDS_GO_DORMANT_RESP_V01 0x0025
#define QMI_WDS_GO_ACTIVE_REQ_V01 0x0026
#define QMI_WDS_GO_ACTIVE_RESP_V01 0x0026
#define QMI_WDS_CREATE_PROFILE_REQ_V01 0x0027
#define QMI_WDS_CREATE_PROFILE_RESP_V01 0x0027
#define QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01 0x0028
#define QMI_WDS_MODIFY_PROFILE_SETTINGS_RESP_V01 0x0028
#define QMI_WDS_DELETE_PROFILE_REQ_V01 0x0029
#define QMI_WDS_DELETE_PROFILE_RESP_V01 0x0029
#define QMI_WDS_GET_PROFILE_LIST_REQ_V01 0x002A
#define QMI_WDS_GET_PROFILE_LIST_RESP_V01 0x002A
#define QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01 0x002B
#define QMI_WDS_GET_PROFILE_SETTINGS_RESP_V01 0x002B
#define QMI_WDS_GET_DEFAULT_SETTINGS_REQ_V01 0x002C
#define QMI_WDS_GET_DEFAULT_SETTINGS_RESP_V01 0x002C
#define QMI_WDS_GET_RUNTIME_SETTINGS_REQ_V01 0x002D
#define QMI_WDS_GET_RUNTIME_SETTINGS_RESP_V01 0x002D
#define QMI_WDS_SET_MIP_MODE_REQ_V01 0x002E
#define QMI_WDS_SET_MIP_MODE_RESP_V01 0x002E
#define QMI_WDS_GET_MIP_MODE_REQ_V01 0x002F
#define QMI_WDS_GET_MIP_MODE_RESP_V01 0x002F
#define QMI_WDS_GET_DORMANCY_STATUS_REQ_V01 0x0030
#define QMI_WDS_GET_DORMANCY_STATUS_RESP_V01 0x0030
#define QMI_WDS_GET_AUTOCONNECT_SETTING_REQ_V01 0x0034
#define QMI_WDS_GET_AUTOCONNECT_SETTING_RESP_V01 0x0034
#define QMI_WDS_GET_CALL_DURATION_REQ_V01 0x0035
#define QMI_WDS_GET_CALL_DURATION_RESP_V01 0x0035
#define QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_REQ_V01 0x0037
#define QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_RESP_V01 0x0037
#define QMI_WDS_GET_DUN_CALL_INFO_REQ_V01 0x0038
#define QMI_WDS_GET_DUN_CALL_INFO_RESP_V01 0x0038
#define QMI_WDS_DUN_CALL_INFO_IND_V01 0x0038
#define QMI_WDS_GET_ACTIVE_MIP_PROFILE_REQ_V01 0x003C
#define QMI_WDS_GET_ACTIVE_MIP_PROFILE_RESP_V01 0x003C
#define QMI_WDS_SET_ACTIVE_MIP_PROFILE_REQ_V01 0x003D
#define QMI_WDS_SET_ACTIVE_MIP_PROFILE_RESP_V01 0x003D
#define QMI_WDS_READ_MIP_PROFILE_REQ_V01 0x003E
#define QMI_WDS_READ_MIP_PROFILE_RESP_V01 0x003E
#define QMI_WDS_MODIFY_MIP_PROFILE_REQ_V01 0x003F
#define QMI_WDS_MODIFY_MIP_PROFILE_RESP_V01 0x003F
#define QMI_WDS_GET_MIP_SETTINGS_REQ_V01 0x0040
#define QMI_WDS_GET_MIP_SETTINGS_RESP_V01 0x0040
#define QMI_WDS_SET_MIP_SETTINGS_REQ_V01 0x0041
#define QMI_WDS_SET_MIP_SETTINGS_RESP_V01 0x0041
#define QMI_WDS_GET_LAST_MIP_STATUS_REQ_V01 0x0042
#define QMI_WDS_GET_LAST_MIP_STATUS_RESP_V01 0x0042
#define QMI_WDS_GET_CURRENT_DATA_BEARER_TECHNOLOGY_REQ_V01 0x0044
#define QMI_WDS_GET_CURRENT_DATA_BEARER_TECHNOLOGY_RESP_V01 0x0044
#define QMI_WDS_CALL_HISTORY_LIST_REQ_V01 0x0045
#define QMI_WDS_CALL_HISTORY_LIST_RESP_V01 0x0045
#define QMI_WDS_CALL_HISTORY_READ_REQ_V01 0x0046
#define QMI_WDS_CALL_HISTORY_READ_RESP_V01 0x0046
#define QMI_WDS_CALL_HISTORY_DELETE_REQ_V01 0x0047
#define QMI_WDS_CALL_HISTORY_DELETE_RESP_V01 0x0047
#define QMI_WDS_CALL_HISTORY_MAX_SIZE_REQ_V01 0x0048
#define QMI_WDS_CALL_HISTORY_MAX_SIZE_RESP_V01 0x0048
#define QMI_WDS_GET_DEFAULT_PROFILE_NUM_REQ_V01 0x0049
#define QMI_WDS_GET_DEFAULT_PROFILE_NUM_RESP_V01 0x0049
#define QMI_WDS_SET_DEFAULT_PROFILE_NUM_REQ_V01 0x004A
#define QMI_WDS_SET_DEFAULT_PROFILE_NUM_RESP_V01 0x004A
#define QMI_WDS_RESET_PROFILE_TO_DEFAULT_REQ_V01 0x004B
#define QMI_WDS_RESET_PROFILE_TO_DEFAULT_RESP_V01 0x004B
#define QMI_WDS_RESET_PROFILE_PARAM_TO_INVALID_REQ_V01 0x004C
#define QMI_WDS_RESET_PROFILE_PARAM_TO_INVALID_RESP_V01 0x004C
#define QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01 0x004D
#define QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_RESP_V01 0x004D
#define QMI_WDS_FMC_SET_TUNNEL_PARAMS_REQ_V01 0x004E
#define QMI_WDS_FMC_SET_TUNNEL_PARAMS_RESP_V01 0x004E
#define QMI_WDS_FMC_CLEAR_TUNNEL_PARAMS_REQ_V01 0x004F
#define QMI_WDS_FMC_CLEAR_TUNNEL_PARAMS_RESP_V01 0x004F
#define QMI_WDS_FMC_GET_TUNNEL_PARAMS_REQ_V01 0x0050
#define QMI_WDS_FMC_GET_TUNNEL_PARAMS_RESP_V01 0x0050
#define QMI_WDS_SET_AUTOCONNECT_SETTINGS_REQ_V01 0x0051
#define QMI_WDS_SET_AUTOCONNECT_SETTINGS_RESP_V01 0x0051
#define QMI_WDS_GET_DNS_SETTINGS_REQ_V01 0x0052
#define QMI_WDS_GET_DNS_SETTINGS_RESP_V01 0x0052
#define QMI_WDS_SET_DNS_SETTINGS_REQ_V01 0x0053
#define QMI_WDS_SET_DNS_SETTINGS_RESP_V01 0x0053
#define QMI_WDS_GET_PRE_DORMANCY_CDMA_SETTINGS_REQ_V01 0x0054
#define QMI_WDS_GET_PRE_DORMANCY_CDMA_SETTINGS_RESP_V01 0x0054
#define QMI_WDS_SET_CAM_TIMER_REQ_V01 0x0055
#define QMI_WDS_SET_CAM_TIMER_RESP_V01 0x0055
#define QMI_WDS_GET_CAM_TIMER_REQ_V01 0x0056
#define QMI_WDS_GET_CAM_TIMER_RESP_V01 0x0056
#define QMI_WDS_SET_SCRM_REQ_V01 0x0057
#define QMI_WDS_SET_SCRM_RESP_V01 0x0057
#define QMI_WDS_GET_SCRM_REQ_V01 0x0058
#define QMI_WDS_GET_SCRM_RESP_V01 0x0058
#define QMI_WDS_SET_RDUD_REQ_V01 0x0059
#define QMI_WDS_SET_RDUD_RESP_V01 0x0059
#define QMI_WDS_GET_RDUD_REQ_V01 0x005A
#define QMI_WDS_GET_RDUD_RESP_V01 0x005A
#define QMI_WDS_GET_SIP_MIP_CALL_TYPE_REQ_V01 0x005B
#define QMI_WDS_GET_SIP_MIP_CALL_TYPE_RESP_V01 0x005B
#define QMI_WDS_SET_EVDO_PAGE_MONITOR_PERIOD_REQ_V01 0x005C
#define QMI_WDS_SET_EVDO_PAGE_MONITOR_PERIOD_RESP_V01 0x005C
#define QMI_WDS_EVDO_PAGE_MONITOR_PERIOD_RESULT_IND_V01 0x005C
#define QMI_WDS_SET_EVDO_FORCE_LONG_SLEEP_REQ_V01 0x005D
#define QMI_WDS_SET_EVDO_FORCE_LONG_SLEEP_RESP_V01 0x005D
#define QMI_WDS_GET_EVDO_PAGE_MONITOR_PERIOD_REQ_V01 0x005E
#define QMI_WDS_GET_EVDO_PAGE_MONITOR_PERIOD_RESP_V01 0x005E
#define QMI_WDS_GET_CALL_THROTTLE_INFO_REQ_V01 0x005F
#define QMI_WDS_GET_CALL_THROTTLE_INFO_RESP_V01 0x005F
#define QMI_WDS_GET_NSAPI_REQ_V01 0x0060
#define QMI_WDS_GET_NAAPI_RESP_V01 0x0060
#define QMI_WDS_SET_DUN_CTRL_PREF_REQ_V01 0x0061
#define QMI_WDS_SET_DUN_CTRL_PREF_RESP_V01 0x0061
#define QMI_WDS_GET_DUN_CTRL_INFO_REQ_V01 0x0062
#define QMI_WDS_GET_DUN_CTRL_INFO_RESP_V01 0x0062
#define QMI_WDS_SET_DUN_CTRL_EVENT_REPORT_REQ_V01 0x0063
#define QMI_WDS_SET_DUN_CTRL_EVENT_REPORT_RESP_V01 0x0063
#define QMI_WDS_DUN_CTRL_EVENT_REPORT_IND_V01 0x0063
#define QMI_WDS_CONTROL_PENDING_DUN_CALL_REQ_V01 0x0064
#define QMI_WDS_CONTROL_PENDING_DUN_CALL_RESP_V01 0x0064
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_REQ_V01 0x0065
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_RESP_V01 0x0065
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_V01 0x0065
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_REQ_V01 0x0066
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_RESP_V01 0x0066
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_V01 0x0066
#define QMI_WDS_EMBMS_TMGI_LIST_QUERY_REQ_V01 0x0067
#define QMI_WDS_EMBMS_TMGI_LIST_QUERY_RESP_V01 0x0067
#define QMI_WDS_EMBMS_TMGI_LIST_IND_V01 0x0068
#define QMI_WDS_GET_PREFERRED_DATA_SYSTEM_REQ_V01 0x0069
#define QMI_WDS_GET_PREFERRED_DATA_SYSTEM_RESP_V01 0x0069
#define QMI_WDS_GET_LAST_DATA_CALL_STATUS_REQ_V01 0x006A
#define QMI_WDS_GET_LAST_DATA_CALL_STATUS_RESP_V01 0x006A
#define QMI_WDS_GET_CURRENT_SYSTEM_STATUS_REQ_V01 0x006B
#define QMI_WDS_GET_CURRENT_SYSTEM_STATUS_RESP_V01 0x006B
#define QMI_WDS_GET_PDN_THROTTLE_INFO_REQ_V01 0x006C
#define QMI_WDS_GET_PDN_THROTTLE_INFO_RESP_V01 0x006C
#define QMI_WDS_GET_LTE_ATTACH_PARAMS_REQ_V01 0x0085
#define QMI_WDS_GET_LTE_ATTACH_PARAMS_RESP_V01 0x0085
#define QMI_WDS_RESET_PKT_STATISTICS_REQ_V01 0x0086
#define QMI_WDS_RESET_PKT_STATISTICS_RESP_V01 0x0086
#define QMI_WDS_GET_FLOW_CONTROL_STATUS_REQ_V01 0x0087
#define QMI_WDS_GET_FLOW_CONTROL_STATUS_RESP_V01 0x0087
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_REQ_V01 0x0088
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_RESP_V01 0x0088
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_V01 0x0088
#define QMI_WDS_BIND_DATA_PORT_REQ_V01 0x0089
#define QMI_WDS_BIND_DATA_PORT_RESP_V01 0x0089
#define QMI_WDS_SET_ADDITIONAL_PDN_FILTER_REQ_V01 0x008A
#define QMI_WDS_SET_ADDITIONAL_PDN_FILTER_RESP_V01 0x008A
#define QMI_WDS_REMOVE_ADDITIONAL_PDN_FILTER_REQ_V01 0x008B
#define QMI_WDS_REMOVE_ADDITIONAL_PDN_FILTER_RESP_V01 0x008B
#define QMI_WDS_EXTENDED_IP_CONFIG_IND_V01 0x008C
#define QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_REGISTRATION_REQ_V01 0x008D
#define QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_REGISTRATION_RESP_V01 0x008D
#define QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_V01 0x008E
#define QMI_WDS_GET_IPSEC_STATIC_SA_CONFIG_REQ_V01 0x008F
#define QMI_WDS_GET_IPSEC_STATIC_SA_CONFIG_RESP_V01 0x008F
#define QMI_WDS_REVERSE_IP_TRANSPORT_CONFIG_COMPLETE_REQ_V01 0x0090
#define QMI_WDS_REVERSE_IP_TRANSPORT_CONFIG_COMPLETE_RESP_V01 0x0090
#define QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_EX_REQ_V01 0x0091
#define QMI_WDS_GET_DATA_BEARER_TECHNOLOGY_EX_RESP_V01 0x0091
#define QMI_WDS_GET_LTE_MAX_ATTACH_PDN_NUM_REQ_V01 0x0092
#define QMI_WDS_GET_LTE_MAX_ATTACH_PDN_NUM_RESP_V01 0x0092
#define QMI_WDS_SET_LTE_ATTACH_PDN_LIST_REQ_V01 0x0093
#define QMI_WDS_SET_LTE_ATTACH_PDN_LIST_RESP_V01 0x0093
#define QMI_WDS_GET_LTE_ATTACH_PDN_LIST_REQ_V01 0x0094
#define QMI_WDS_GET_LTE_ATTACH_PDN_LIST_RESP_V01 0x0094
#define QMI_WDS_LTE_ATTACH_PDN_LIST_IND_V01 0x0095
#define QMI_WDS_SET_LTE_DATA_RETRY_REQ_V01 0x0096
#define QMI_WDS_SET_LTE_DATA_RETRY_RESP_V01 0x0096
#define QMI_WDS_GET_LTE_DATA_RETRY_REQ_V01 0x0097
#define QMI_WDS_GET_LTE_DATA_RETRY_RESP_V01 0x0097
#define QMI_WDS_SET_LTE_ATTACH_TYPE_REQ_V01 0x0098
#define QMI_WDS_SET_LTE_ATTACH_TYPE_RESP_V01 0x0098
#define QMI_WDS_GET_LTE_ATTACH_TYPE_REQ_V01 0x0099
#define QMI_WDS_GET_LTE_ATTACH_TYPE_RESP_V01 0x0099
#define QMI_WDS_REVERSE_IP_TRANSPORT_FILTER_SETUP_IND_V01 0x009A
#define QMI_WDS_HANDOFF_INFORMATION_IND_V01 0x009B
#define QMI_WDS_SET_DATA_PATH_REQ_V01 0x009C
#define QMI_WDS_SET_DATA_PATH_RESP_V01 0x009C
#define QMI_WDS_GET_DATA_PATH_REQ_V01 0x009D
#define QMI_WDS_GET_DATA_PATH_RESP_V01 0x009D
#define QMI_WDS_UPDATE_LTE_ATTACH_PDN_LIST_PROFILES_REQ_V01 0x009F
#define QMI_WDS_UPDATE_LTE_ATTACH_PDN_LIST_PROFILES_RESP_V01 0x009F
#define QMI_WDS_EMBMS_SAI_LIST_QUERY_REQ_V01 0x00A0
#define QMI_WDS_EMBMS_SAI_LIST_QUERY_RESP_V01 0x00A0
#define QMI_WDS_EMBMS_SAI_LIST_IND_V01 0x00A1
#define QMI_WDS_BIND_MUX_DATA_PORT_REQ_V01 0x00A2
#define QMI_WDS_BIND_MUX_DATA_PORT_RESP_V01 0x00A2
#define QMI_WDS_SET_THROUGHPUT_INFO_IND_FREQ_REQ_V01 0x00A3
#define QMI_WDS_SET_THROUGHPUT_INFO_IND_FREQ_RESP_V01 0x00A3
#define QMI_WDS_GET_LAST_THROUGHPUT_INFO_REQ_V01 0x00A4
#define QMI_WDS_GET_LAST_THROUGHPUT_INFO_RESP_V01 0x00A4
#define QMI_WDS_THROUGHPUT_INFO_IND_V01 0x00A5
#define QMI_WDS_INITIATE_ESP_REKEY_REQ_V01 0x00A6
#define QMI_WDS_INITIATE_ESP_REKEY_RESP_V01 0x00A6
#define QMI_WDS_CONFIGURE_PROFILE_EVENT_LIST_REQ_V01 0x00A7
#define QMI_WDS_CONFIGURE_PROFILE_EVENT_LIST_RESP_V01 0x00A7
#define QMI_WDS_PROFILE_EVENT_REGISTER_IND_V01 0x00A8
#define QMI_WDS_GET_CAPABILITIES_REQ_V01 0x00A9
#define QMI_WDS_GET_CAPABILITIES_RESP_V01 0x00A9
#define QMI_WDS_GET_ROAMING_INFO_REQ_V01 0x00AA
#define QMI_WDS_GET_ROAMING_INFO_RESP_V01 0x00AA
#define QMI_WDS_ROAMING_INFO_IND_V01 0x00AB
#define QMI_WDS_GET_DELEGATED_IPV6_PREFIX_REQ_V01 0x00AC
#define QMI_WDS_GET_DELEGATED_IPV6_PREFIX_RESP_V01 0x00AC
#define QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_REQ_V01 0x00AD
#define QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_RESP_V01 0x00AD
#define QMI_WDS_ABORT_GO_DORMANT_REQ_V01 0x00AE
#define QMI_WDS_ABORT_GO_DORMANT_RESP_V01 0x00AE
#define QMI_WDS_BIND_SUBSCRIPTION_REQ_V01 0x00AF
#define QMI_WDS_BIND_SUBSCRIPTION_RESP_V01 0x00AF
#define QMI_WDS_GET_BIND_SUBSCRIPTION_REQ_V01 0x00B0
#define QMI_WDS_GET_BIND_SUBSCRIPTION_RESP_V01 0x00B0
#define QMI_WDS_SET_LTE_DATA_CALL_TYPE_REQ_V01 0x00B1
#define QMI_WDS_SET_LTE_DATA_CALL_TYPE_RESP_V01 0x00B1
#define QMI_WDS_SET_DOWNLINK_THROUGHPUT_INFO_IND_FREQ_REQ_V01 0x00B2
#define QMI_WDS_SET_DOWNLINK_THROUGHPUT_INFO_IND_FREQ_RESP_V01 0x00B2
#define QMI_WDS_DOWNLINK_THROUGHPUT_INFO_IND_V01 0x00B3
#define QMI_WDS_GET_DOWNLINK_THROUGHPUT_INFO_PARAMS_REQ_V01 0x00B4
#define QMI_WDS_GET_DOWNLINK_THROUGHPUT_INFO_PARAMS_RESP_V01 0x00B4
#define QMI_WDS_EMBMS_CONTENT_DESC_UPDATE_REQ_V01 0x00B5
#define QMI_WDS_EMBMS_CONTENT_DESC_UPDATE_RESP_V01 0x00B5
#define QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_V01 0x00B6
#define QMI_WDS_POLICY_REFRESH_REQ_V01 0x00B7
#define QMI_WDS_POLICY_REFRESH_RESP_V01 0x00B7
#define QMI_WDS_POLICY_REFRESH_RESULT_IND_V01 0x00B7
#define QMI_WDS_POLICY_READY_IND_V01 0x00B8
#define QMI_WDS_APN_PARAM_INFO_CHANGE_IND_V01 0x00B9
#define QMI_WDS_SET_SILENT_REDIAL_REQ_V01 0x00BA
#define QMI_WDS_SET_SILENT_REDIAL_RESP_V01 0x00BA
#define QMI_WDS_REFRESH_DHCP_CONFIG_INFO_REQ_V01 0xFFFB
#define QMI_WDS_REFRESH_DHCP_CONFIG_INFO_RESP_V01 0xFFFB
#define QMI_WDS_SET_INTERNAL_RUNTIME_SETTINGS_REQ_V01 0xFFFC
#define QMI_WDS_SET_INTERNAL_RUNTIME_SETTINGS_RESP_V01 0xFFFC
#define QMI_WDS_GET_INTERNAL_RUNTIME_SETTINGS_REQ_V01 0xFFFD
#define QMI_WDS_GET_INTERNAL_RUNTIME_SETTINGS_RESP_V01 0xFFFD
#define QMI_WDS_INTERNAL_IFACE_EV_REGISTER_REQ_V01 0xFFFE
#define QMI_WDS_INTERNAL_IFACE_EV_REGISTER_RESP_V01 0xFFFE
#define QMI_WDS_INTERNAL_IFACE_EV_IND_V01 0xFFFE
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro wds_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type wds_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define wds_get_service_object_v01( ) \
          wds_get_service_object_internal_v01( \
            WDS_V01_IDL_MAJOR_VERS, WDS_V01_IDL_MINOR_VERS, \
            WDS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

