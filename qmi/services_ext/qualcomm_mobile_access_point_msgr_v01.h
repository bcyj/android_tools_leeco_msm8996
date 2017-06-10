#ifndef QCMAP_MSGR_SERVICE_01_H
#define QCMAP_MSGR_SERVICE_01_H
/**
  @file qualcomm_mobile_access_point_msgr_v01.h

  @brief This is the public header file which defines the qcmap_msgr service Data structures.

  This header file defines the types and structures that were defined in
  qcmap_msgr. It contains the constant values defined, enums, structures,
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


  $Header: //source/qcom/qct/interfaces/qmi/qcmap_msgr/main/latest/api/qualcomm_mobile_access_point_msgr_v01.h#23 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6 
   It requires encode/decode library version 5 or later
   It was generated on: Tue Dec  3 2013 (Spin 0)
   From IDL File: qualcomm_mobile_access_point_msgr_v01.idl */

/** @defgroup qcmap_msgr_qmi_consts Constant values defined in the IDL */
/** @defgroup qcmap_msgr_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qcmap_msgr_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qcmap_msgr_qmi_messages Structures sent as QMI messages */
/** @defgroup qcmap_msgr_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qcmap_msgr_qmi_accessor Accessor for QMI service object */
/** @defgroup qcmap_msgr_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qcmap_msgr_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QCMAP_MSGR_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QCMAP_MSGR_V01_IDL_MINOR_VERS 0x14
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QCMAP_MSGR_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QCMAP_MSGR_V01_MAX_MESSAGE_ID 0x007B
/**
    @}
  */


/** @addtogroup qcmap_msgr_qmi_consts 
    @{ 
  */
#define QCMAP_MSGR_MAX_SNAT_ENTRIES_V01 50
#define QCMAP_MSGR_MAX_FIREWALL_ENTRIES_V01 50
#define QCMAP_MSGR_IPV6_ADDR_LEN_V01 16
#define QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01 500
#define QCMAP_MSGR_MAC_ADDR_LEN_V01 6
#define QCMAP_MSGR_DEVICE_NAME_MAX_V01 100
#define QCMAP_MSGR_MAX_DHCP_RESERVATION_ENTRIES_V01 20
#define QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01 18
#define QCMAP_MSGR_PCSCF_FQDN_MAX_LENGTH_V01 100
#define QCMAP_MSGR_MAX_SIP_SERVER_ENTRIES_V01 25
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_IP_FAMILY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_IP_FAMILY_V4_V01 = 0x04, /**<  IPv4 version  */
  QCMAP_MSGR_IP_FAMILY_V6_V01 = 0x06, /**<  IPv6 version  */
  QCMAP_MSGR_IP_FAMILY_V4V6_V01 = 0x0A, /**<  Dual mode version  */
  QCMAP_MSGR_IP_FAMILY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_ip_family_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_WWAN_CALL_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 = 0x01, /**<  IPv4 call  */
  QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 = 0x02, /**<  IPv6 call  */
  QCMAP_MSGR_WWAN_CALL_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_wwan_call_type_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_WWAN_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_WWAN_STATUS_CONNECTING_V01 = 0x01, /**<  IPv4 WWAN is in connecting state  */
  QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01 = 0x02, /**<  IPv4 connection to WWAN failed  */
  QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01 = 0x03, /**<  IPv4 WWAN is in connected state  */
  QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_V01 = 0x04, /**<  IPv4 WWAN is disconnecting  */
  QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01 = 0x05, /**<  IPv4 WWAN failed to disconnect  */
  QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01 = 0x06, /**<  IPv4 WWAN is disconnected  */
  QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_V01 = 0x07, /**<  IPv6 WWAN is in connecting state  */
  QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01 = 0x08, /**<  IPv6 connection to WWAN failed  */
  QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01 = 0x09, /**<  IPv6 WWAN is in connected state  */
  QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_V01 = 0x0A, /**<  IPv6 WWAN is disconnecting  */
  QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL_V01 = 0x0B, /**<  IPv6 WWAN failed to disconnect  */
  QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01 = 0x0C, /**<  IPv6 WWAN is disconnected  */
  QCMAP_MSGR_WWAN_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_wwan_status_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_MOBILE_AP_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED_V01 = 0x01, /**<  Mobile AP is connected  */
  QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED_V01 = 0x02, /**<  Mobile AP is disconnected  */
  QCMAP_MSGR_MOBILE_AP_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_mobile_ap_status_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_STATION_MODE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_STATION_MODE_CONNECTED_V01 = 0x01, /**<  Station mode is connected  */
  QCMAP_MSGR_STATION_MODE_DISCONNECTED_V01 = 0x02, /**<  Station mode is disconnected  */
  QCMAP_MSGR_STATION_MODE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_station_mode_status_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_NAT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_NAT_SYMMETRIC_NAT_V01 = 0, /**<  NAT type is symmetric  */
  QCMAP_MSGR_NAT_PORT_RESTRICTED_CONE_NAT_V01 = 1, /**<  NAT type is Port-Restricted Cone NAT  */
  QCMAP_MSGR_NAT_FULL_CONE_NAT_V01 = 2, /**<  NAT type is Full Cone NAT  */
  QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT_V01 = 3, /**<  NAT type is Address-Restricted NAT  */
  QCMAP_MSGR_NAT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_nat_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_NAT_TIMEOUT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01 = 0x01, /**<   Generic NAT timeout  */
  QCMAP_MSGR_NAT_TIMEOUT_ICMP_V01 = 0x02, /**<   NAT timeout for ICMP  */
  QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED_V01 = 0x03, /**<   NAT timeout for the TCP established  */
  QCMAP_MSGR_NAT_TIMEOUT_UDP_V01 = 0x04, /**<   NAT timeout for UDP  */
  QCMAP_MSGR_NAT_TIMEOUT_UDP_STREAM_V01 = 0x05, /**<   NAT timeout for UDP stream  */
  QCMAP_MSGR_NAT_TIMEOUT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_nat_timeout_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_WLAN_MODE_AP_V01 = 0x01, /**<  WLAN is in AP mode  */
  QCMAP_MSGR_WLAN_MODE_AP_AP_V01 = 0x02, /**<  WLAN is in AP/AP Mode  */
  QCMAP_MSGR_WLAN_MODE_AP_STA_V01 = 0x03, /**<  WLAN is in AP/STA Mode  */
  QCMAP_MSGR_WLAN_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_wlan_mode_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_ACCESS_PROFILE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_PROFILE_FULL_ACCESS_V01 = 0, /**<  AP Can Access complete LAN Including Internet  */
  QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01 = 1, /**<  AP Can Access Only Internet               */
  QCMAP_MSGR_ACCESS_PROFILE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_access_profile_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_BOOTUP_FLAG_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_DISABLE_ON_BOOT_V01 = 0, /**<  Disable QCMAP Component on bootup  */
  QCMAP_MSGR_ENABLE_ON_BOOT_V01 = 1, /**<  Enable QCMAP Component on bootup             */
  QCMAP_MSGR_BOOTUP_FLAG_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_bootup_flag_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_USB_LINK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_USB_LINK_RNDIS_V01 = 0x01, /**<  USB tethered link is RNDIS  */
  QCMAP_MSGR_USB_LINK_ECM_V01 = 0x02, /**<  USB tethered link is ECM  */
  QCMAP_MSGR_USB_LINK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_usb_link_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_WWAN_CALL_END_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_WWAN_CALL_END_TYPE_INVALID_V01 = 0xFF, /**<  Unknown  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_MOBILE_IP_V01 = 0x01, /**<  Mobile IP  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_INTERNAL_V01 = 0x02, /**<  Internal  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_CALL_MANAGER_DEFINED_V01 = 0x03, /**<  Call manager defined  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_3GPP_SPEC_DEFINED_V01 = 0x06, /**<  3GPP specification defined  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_PPP_V01 = 0x07, /**<  PPP  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_EHRPD_V01 = 0x08, /**<  EHRPD  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_IPV6_V01 = 0x09, /**<  IPv6  */
  QCMAP_MSGR_WWAN_CALL_END_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_wwan_call_end_type_enum_v01;
/**
    @}
  */

typedef uint64_t qcmap_msgr_tech_pref_mask_v01;
#define QCMAP_MSGR_MASK_TECH_PREF_3GPP_V01 ((qcmap_msgr_tech_pref_mask_v01)0x01ull) /**<  3GPP  */
#define QCMAP_MSGR_MASK_TECH_PREF_3GPP2_V01 ((qcmap_msgr_tech_pref_mask_v01)0x02ull) /**<  3GPP2  */
typedef uint64_t qcmap_msgr_addr_type_mask_v01;
#define QCMAP_MSGR_MASK_V4_ADDR_V01 ((qcmap_msgr_addr_type_mask_v01)0x00000001ull) /**<  IPv4 address  */
#define QCMAP_MSGR_MASK_V6_ADDR_V01 ((qcmap_msgr_addr_type_mask_v01)0x00000002ull) /**<  IPv6 address  */
#define QCMAP_MSGR_MASK_V4_DNS_ADDR_V01 ((qcmap_msgr_addr_type_mask_v01)0x00000004ull) /**<  IPv4 DNS address   */
#define QCMAP_MSGR_MASK_V6_DNS_ADDR_V01 ((qcmap_msgr_addr_type_mask_v01)0x00000008ull) /**<  IPv6 DNS address  */
typedef uint64_t qcmap_msgr_alg_type_mask_v01;
#define QCMAP_MSGR_MASK_RTSP_ALG_V01 ((qcmap_msgr_alg_type_mask_v01)0x01ull) /**<  RTSP ALG   */
#define QCMAP_MSGR_MASK_SIP_ALG_V01 ((qcmap_msgr_alg_type_mask_v01)0x02ull) /**<  SIP ALG  
 All other bits are reserved and will be ignored by the service
   ADD more MASK depending on new ALG modules added
   */
/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_STA_CONNECTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_STA_CONNECTION_DYNAMIC_V01 = 0x01, /**<  Dynamic  */
  QCMAP_MSGR_STA_CONNECTION_STATIC_V01 = 0x02, /**<  Static  */
  QCMAP_MSGR_STA_CONNECTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_sta_connection_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_UPNP_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_UPNP_MODE_UP_V01 = 0x01, /**<  UPNP is in AP mode  */
  QCMAP_MSGR_UPNP_MODE_DOWN_V01 = 0x02, /**<  UPNP is in AP/AP Mode  */
  QCMAP_MSGR_UPNP_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_upnp_mode_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_DLNA_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_DLNA_MODE_UP_V01 = 0x01, /**<  DLNA is in AP mode  */
  QCMAP_MSGR_DLNA_MODE_DOWN_V01 = 0x02, /**<  DLNA is in AP/AP Mode  */
  QCMAP_MSGR_DLNA_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_dlna_mode_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_MDNS_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_MDNS_MODE_UP_V01 = 0x01, /**<  MDNS is in AP mode  */
  QCMAP_MSGR_MDNS_MODE_DOWN_V01 = 0x02, /**<  MDNS is in AP/AP Mode  */
  QCMAP_MSGR_MDNS_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_mdns_mode_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_ACTIVATE_HOSTAPD_AP_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_PRIMARY_AP_V01 = 0x01, /**<  Primary AP.  */
  QCMAP_MSGR_GUEST_AP_V01 = 0x02, /**<  Guest AP.  */
  QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01 = 0x03, /**<  Both the AP's.  */
  QCMAP_MSGR_ACTIVATE_HOSTAPD_AP_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_activate_hostapd_ap_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_ACTIVATE_HOSTAPD_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_HOSTAPD_START_V01 = 0x01, /**<  Start hostapd.  */
  QCMAP_MSGR_HOSTAPD_STOP_V01 = 0x02, /**<  Stop hostapd.  */
  QCMAP_MSGR_HOSTAPD_RESTART_V01 = 0x03, /**<  Restart hostapd.  */
  QCMAP_MSGR_ACTIVATE_HOSTAPD_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_activate_hostapd_action_enum_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_enums
    @{
  */
typedef enum {
  QCMAP_MSGR_PCSCF_INFO_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QCMAP_MSGR_PCSCF_IP_ADDRESS_V01 = 0x01, /**<  PCSCF IP Address  */
  QCMAP_MSGR_PCSCF_FQDN_V01 = 0x02, /**<  PCSCF Fully Qualified Domain Name */
  QCMAP_MSGR_PCSCF_INFO_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qcmap_msgr_pcscf_info_type_v01;
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
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
}qcmap_msgr_snat_entry_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  qcmap_msgr_tech_pref_mask_v01 tech_pref;
  /**<   Bitmap indicating the technology preference. A single connection
 is attempted using the following specified technology preferences.
 Values: \n
      - QCMAP_MSGR_MASK_TECH_PREF_3GPP (0x01) --  3GPP 
      - QCMAP_MSGR_MASK_TECH_PREF_3GPP2 (0x02) --  3GPP2 

 \vspace{3pt}
 All other bits are reserved and ignored even if they are set
 in the request. If a single value of the technology preference 
 bitmask is set, the device attempts to use that technology. If two
 or more bits in the technology preference bitmask are set, the
 device determines the technology to be used from those specified.
 */

  qcmap_msgr_ip_family_enum_v01 ip_family;
  /**<   Identifies the IP version to be used. Values: \n
      - QCMAP_MSGR_IP_FAMILY_V4 (0x04) --  IPv4 version 
      - QCMAP_MSGR_IP_FAMILY_V6 (0x06) --  IPv6 version 
      - QCMAP_MSGR_IP_FAMILY_V4V6 (0x0A) --  Dual mode version 
 */

  uint8_t v4_profile_id_3gpp2;
  /**<   CDMA profile ID for IPv4. */

  uint8_t v4_profile_id_3gpp;
  /**<   UMTS profile ID for IPv4. */

  uint8_t v6_profile_id_3gpp2;
  /**<   CDMA profile ID for IPv6. */

  uint8_t v6_profile_id_3gpp;
  /**<   UMTS profile ID for IPv6. */
}qcmap_msgr_net_policy_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint64_t bytes_rx;
  /**<   Bytes received on WWAN. */

  uint64_t bytes_tx;
  /**<   Bytes transmitted on WWAN. */

  uint32_t pkts_rx;
  /**<   Packets received on WWAN. */

  uint32_t pkts_tx;
  /**<   Packets transmitted on WWAN. */

  uint32_t pkts_dropped_rx;
  /**<   Packets dropped on Rx WWAN. */

  uint32_t pkts_dropped_tx;
  /**<   Packets dropped on Tx WWAN. */
}qcmap_msgr_wwan_statistics_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t addr;
  /**<   IPv4 address as specified in the IPv4 protocol
       specification (RFC 791 \hyperref[S2]{[S2]}).
  */

  uint32_t subnet_mask;
  /**<   IPv4 subnet mask as specified in the IPv4
       protocol specification (RFC 791 \hyperref[S2]{[S2]}).
  */
}qcmap_ip4_addr_subnet_mask_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t addr[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address as specified in the IPv6
       protocol specification (RFC 2460 \hyperref[S5]{[S5]}).
  */

  uint8_t prefix_len;
  /**<   IPv6 prefix length as specified in the IPv6
       protocol specification (RFC 3513 \hyperref[S6]{[S6]}).
  */
}qcmap_ip6_addr_prefix_len_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t port;
  /**<   TCP/UDP port as specified in the
       TCP and UDP protocols (RFC 793 \hyperref[S4]{[S4]},
       RFC 768 \hyperref[S1]{[S1]}).
  */

  uint16_t range;
  /**<   TCP/UDP port range as specified in the
       TCP and UDP protocols (RFC 793 \hyperref[S4]{[S4]},
       RFC 768 \hyperref[S1]{[S1]}).
  */
}qcmap_tcp_udp_port_range_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t value;
  /**<   TOS value as specified in the IPv4 protocol
       (RFC 791 \hyperref[S2]{[S2]}). */

  uint8_t mask;
  /**<   IPv4 TOS mask */
}qcmap_ip4_tos_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t value;
  /**<   IPv6 traffic class value as specified in the IPv6 protocol
       (RFC 2460 \hyperref[S5]{[S5]}). */

  uint8_t mask;
  /**<   IPv6 traffic class mask */
}qcmap_ip6_traffic_class_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t dhcp_start_ip;
  /**<   DHCP start IP address. */

  uint32_t dhcp_end_ip;
  /**<   DHCP end IP address. */

  uint32_t lease_time;
  /**<   DHCP lease time, in seconds.*/
}qcmap_msgr_dhcp_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t gw_ip;
  /**<   IP address of the gateway. */

  uint32_t netmask;
  /**<   Subnet mask. */

  uint8_t enable_dhcp;
  /**<   Whether to enable DHCP; boolean value. */

  qcmap_msgr_dhcp_config_v01 dhcp_config;
  /**<   DHCP configuration. Used only when DHCP is enabled. */
}qcmap_msgr_lan_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t gw_ip;
  /**<   IP address of the WLAN gateway. */

  uint32_t netmask;
  /**<   Subnet mask for WLAN. */

  qcmap_msgr_dhcp_config_v01 dhcp_config;
  /**<   WLAN DHCP configuration. */
}qcmap_msgr_wlan_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t gw_ip;
  /**<   USB gateway IP address. */

  uint32_t netmask;
  /**<   Subnet mask for USB TE. */

  uint32_t usb_ip;
  /**<   USB TE IP address. */

  uint32_t lease_time;
  /**<   DHCP lease time, in seconds.*/
}qcmap_msgr_usb_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  qcmap_msgr_wlan_config_v01 wlan_config;
  /**<   WLAN configuration */

  qcmap_msgr_usb_config_v01 usb_config;
  /**<   USB TE configuration */
}qcmap_msgr_ap_mode_wlan_usb_ip_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ip_addr;
  /**<   IP address to be assigned. */

  uint32_t gw_ip;
  /**<   IP address of the gateway. */

  uint32_t netmask;
  /**<   Subnet mask. */

  uint32_t dns_addr;
  /**<   DNS address. */
}qcmap_msgr_sta_static_ip_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  qcmap_msgr_sta_connection_enum_v01 conn_type;
  /**<   Connection type. */

  qcmap_msgr_sta_static_ip_config_v01 static_ip_config;
  /**<   Static IP configuration. Used only when the connection type is static. */
}qcmap_msgr_station_mode_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  qcmap_msgr_wwan_call_end_type_enum_v01 wwan_call_end_reason_type;
  /**<   WWAN call end type.  */

  int32_t wwan_call_end_reason_code;
  /**<   WWAN call end reason code. */
}qcmap_msgr_wwan_call_end_reason_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint64_t tx_rate;
  /**<   current tx rate. */

  uint64_t rx_rate;
  /**<   current rx rate. */

  uint64_t max_tx_rate;
  /**<   max tx rate. */

  uint64_t max_rx_rate;
  /**<   max rx rate. */
}qcmap_msgr_data_bitrate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t client_mac_addr[QCMAP_MSGR_MAC_ADDR_LEN_V01];
  /**<   MAC address of the device. */

  uint32_t client_reserved_ip;
  /**<   Reserved IP for the AP client. */

  char client_device_name[QCMAP_MSGR_DEVICE_NAME_MAX_V01];
  /**<   Device name. */

  uint8_t enable_reservation;
  /**<   To enable/disable DHCP reservation; boolean value. */
}qcmap_msgr_dhcp_reservation_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_aggregates
    @{
  */
typedef struct {

  qcmap_msgr_pcscf_info_type_v01 pcscf_info_type;
  /**<   Identifies the type of PCSCF information contained*/

  uint32_t pcscf_ip_addr;
  /**<   Identifies the PCSCF IP address  */

  char pcscf_fqdn[QCMAP_MSGR_PCSCF_FQDN_MAX_LENGTH_V01];
  /**<   Identifies the default PCSCF FQDN */
}qcmap_msgr_sip_server_info_v01;  /* Type */
/**
    @}
  */

/*
 * qcmap_msgr_mobile_ap_enable_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_mobile_ap_enable_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables the mobile AP functionality via a single mobile AP instance 
    on the ARM\reg Cortex\tmns-A5 processor. */
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
       specified in all mobile AP-specific QCMAP_MSGR messages. For example,
       QMI_QCMAP_MSGR_ MOBILE_AP_ DISABLE, QMI_ QCMAP_MSGR_BRING_UP_WWAN, etc.
       The mobile_ap_handle can be a handle from a previous successful
       mobile AP call activation. */
}qcmap_msgr_mobile_ap_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Disables the mobile AP functionality for a mobile AP instance on 
           the Cortex-A5 processor. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_mobile_ap_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables the mobile AP functionality for a mobile AP instance on 
           the Cortex-A5 processor. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_mobile_ap_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Enables IPv4 and brings up an IPv4 WWAN call if autoconnect is enabled. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_enable_ipv4_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables IPv4 and brings up an IPv4 WWAN call if autoconnect is enabled. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_ipv4_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Disables IPv4 and brings down an IPv4 WWAN call if connected. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_disable_ipv4_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables IPv4 and brings down an IPv4 WWAN call if connected. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_ipv4_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_ipv4_state_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_ipv4_state_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries the IPv4 state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IPv6 State Value */
  uint8_t ipv4_state_valid;  /**< Must be set to true if ipv4_state is being passed */
  uint8_t ipv4_state;
  /**<   Indicates whether IPv4 is enabled; boolean value.
  */
}qcmap_msgr_get_ipv4_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Enables IPv6 and brings up an IPv6 WWAN call if autoconnect is enabled. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_enable_ipv6_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables IPv6 and brings up an IPv6 WWAN call if autoconnect is enabled. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_ipv6_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Disables IPv6 and brings down an IPv6 WWAN call if connected. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_disable_ipv6_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables IPv6 and brings down an IPv6 WWAN call if connected. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_ipv6_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_ipv6_state_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_ipv6_state_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries the IPv6 state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IPv6 State Value */
  uint8_t ipv6_state_valid;  /**< Must be set to true if ipv6_state is being passed */
  uint8_t ipv6_state;
  /**<   Indicates whether IPv6 is enabled; boolean value.
  */
}qcmap_msgr_get_ipv6_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Invokes bringing up the WWAN from the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  WWAN Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  qcmap_msgr_wwan_call_type_v01 call_type;
  /**<   Type of WWAN call to be brought up. Values: \n
      - QCMAP_MSGR_WWAN_CALL_TYPE_V4 (0x01) --  IPv4 call 
      - QCMAP_MSGR_WWAN_CALL_TYPE_V6 (0x02) --  IPv6 call 
 */
}qcmap_msgr_bring_up_wwan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Invokes bringing up the WWAN from the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WWAN Connection Status */
  uint8_t conn_status_valid;  /**< Must be set to true if conn_status is being passed */
  qcmap_msgr_wwan_status_enum_v01 conn_status;
  /**<   WWAN connection status. Values: \n
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING (0x01) --  IPv4 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL (0x02) --  IPv4 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTED (0x03) --  IPv4 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING (0x04) --  IPv4 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL (0x05) --  IPv4 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTED (0x06) --  IPv4 WWAN is disconnected 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING (0x07) --  IPv6 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL (0x08) --  IPv6 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED (0x09) --  IPv6 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING (0x0A) --  IPv6 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL (0x0B) --  IPv6 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED (0x0C) --  IPv6 WWAN is disconnected  
 */
}qcmap_msgr_bring_up_wwan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Indication Message; Indicates the completion of processing a 
           QMI_QCMAP_MSGR_BRING_UP_WWAN_REQ request. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  WWAN Connection Status */
  qcmap_msgr_wwan_status_enum_v01 conn_status;
  /**<   WWAN connection status. Values: \n
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING (0x01) --  IPv4 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL (0x02) --  IPv4 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTED (0x03) --  IPv4 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING (0x04) --  IPv4 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL (0x05) --  IPv4 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTED (0x06) --  IPv4 WWAN is disconnected 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING (0x07) --  IPv6 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL (0x08) --  IPv6 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED (0x09) --  IPv6 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING (0x0A) --  IPv6 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL (0x0B) --  IPv6 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED (0x0C) --  IPv6 WWAN is disconnected 
 */

  /* Optional */
  /*  WWAN Call End Reason */
  uint8_t wwan_call_end_reason_valid;  /**< Must be set to true if wwan_call_end_reason is being passed */
  qcmap_msgr_wwan_call_end_reason_v01 wwan_call_end_reason;
  /**<   \n For values, refer to \hyperref[Q3]{[Q3]}.
   */
}qcmap_msgr_bring_up_wwan_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Tears down the WWAN. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  WWAN Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  qcmap_msgr_wwan_call_type_v01 call_type;
  /**<   Type of WWAN call to be brought down. Values: \n
      - QCMAP_MSGR_WWAN_CALL_TYPE_V4 (0x01) --  IPv4 call 
      - QCMAP_MSGR_WWAN_CALL_TYPE_V6 (0x02) --  IPv6 call 
 */
}qcmap_msgr_tear_down_wwan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Tears down the WWAN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WWAN Connection Status */
  uint8_t conn_status_valid;  /**< Must be set to true if conn_status is being passed */
  qcmap_msgr_wwan_status_enum_v01 conn_status;
  /**<   WWAN connection status. Values: \n
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING (0x01) --  IPv4 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL (0x02) --  IPv4 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTED (0x03) --  IPv4 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING (0x04) --  IPv4 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL (0x05) --  IPv4 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTED (0x06) --  IPv4 WWAN is disconnected 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING (0x07) --  IPv6 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL (0x08) --  IPv6 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED (0x09) --  IPv6 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING (0x0A) --  IPv6 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL (0x0B) --  IPv6 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED (0x0C) --  IPv6 WWAN is disconnected 
 */
}qcmap_msgr_tear_down_wwan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Indication Message; Indicates the completion of processing a 
           QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_REQ request. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  WWAN Connection Status */
  qcmap_msgr_wwan_status_enum_v01 conn_status;
  /**<   WWAN connection status. Values: \n
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING (0x01) --  IPv4 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL (0x02) --  IPv4 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTED (0x03) --  IPv4 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING (0x04) --  IPv4 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL (0x05) --  IPv4 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTED (0x06) --  IPv4 WWAN is disconnected 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING (0x07) --  IPv6 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL (0x08) --  IPv6 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED (0x09) --  IPv6 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING (0x0A) --  IPv6 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL (0x0B) --  IPv6 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED (0x0C) --  IPv6 WWAN is disconnected 
 */

  /* Optional */
  /*  WWAN Call End Reason */
  uint8_t wwan_call_end_reason_valid;  /**< Must be set to true if wwan_call_end_reason is being passed */
  qcmap_msgr_wwan_call_end_reason_v01 wwan_call_end_reason;
  /**<   \n For possible values, refer to \hyperref[Q3]{[Q3]}.
   */
}qcmap_msgr_tear_down_wwan_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Registers or deregisters the control point to receive
           QMI_QCMAP_MSGR_ WWAN_STATUS_IND. \label{idl:WWANStatusIndReg} */
typedef struct {

  /* Mandatory */
  /*  Register Indication */
  uint8_t register_indication;
  /**<   Specifies the registration. Values: \n
       - 0 -- Do not register or deregister the control point if already registered \n
       - 1 -- Register for the indication; ignore if already registered
  */
}qcmap_msgr_wwan_status_ind_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Registers or deregisters the control point to receive
           QMI_QCMAP_MSGR_ WWAN_STATUS_IND. \label{idl:WWANStatusIndReg} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_wwan_status_ind_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current mobile AP WWAN connection status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
        QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  Packet Service Status */
  qcmap_msgr_wwan_status_enum_v01 wwan_status;
  /**<   Indicates the WWAN status. Values: \n
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING (0x01) --  IPv4 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL (0x02) --  IPv4 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTED (0x03) --  IPv4 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING (0x04) --  IPv4 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL (0x05) --  IPv4 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTED (0x06) --  IPv4 WWAN is disconnected 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING (0x07) --  IPv6 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL (0x08) --  IPv6 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED (0x09) --  IPv6 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING (0x0A) --  IPv6 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL (0x0B) --  IPv6 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED (0x0C) --  IPv6 WWAN is disconnected 
 */

  /* Optional */
  /*  WWAN Call End Reason */
  uint8_t wwan_call_end_reason_valid;  /**< Must be set to true if wwan_call_end_reason is being passed */
  qcmap_msgr_wwan_call_end_reason_v01 wwan_call_end_reason;
  /**<   \n For possible values, refer to \hyperref[Q3]{[Q3]}.
   */
}qcmap_msgr_wwan_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Registers or deregisters the control point to receive
           QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND. */
typedef struct {

  /* Mandatory */
  /*  Register Indication */
  uint8_t register_indication;
  /**<   Specifies the registration. Values: \n
       - 0 -- Do not register or deregister the control point if already registered \n
       - 1 -- Register for the indication; ignore if already registered
  */
}qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Registers or deregisters the control point to receive
           QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current mobile AP status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  Mobile AP Status */
  qcmap_msgr_mobile_ap_status_enum_v01 mobile_ap_status;
  /**<   Indicates the mobile AP status. Values: \n
      - QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED (0x01) --  Mobile AP is connected 
      - QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED (0x02) --  Mobile AP is disconnected 
 */
}qcmap_msgr_mobile_ap_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Registers or deregisters the control point to receive
           QMI_QCMAP_MSGR_STATION_MODE_STATUS_IND. */
typedef struct {

  /* Mandatory */
  /*  Register Indication */
  uint8_t register_indication;
  /**<   Specifies the registration. Values: \n
       - 0 -- Do not register or deregister the control point if already registered \n
       - 1 -- Register for the indication; ignore if already registered
  */
}qcmap_msgr_station_mode_status_ind_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Registers or deregisters the control point to receive
           QMI_QCMAP_MSGR_STATION_MODE_STATUS_IND. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_station_mode_status_ind_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current station mode status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  Station Mode Status */
  qcmap_msgr_station_mode_status_enum_v01 station_mode_status;
  /**<   Indicates the mobile AP status. Values: \n
      - QCMAP_MSGR_STATION_MODE_CONNECTED (0x01) --  Station mode is connected 
      - QCMAP_MSGR_STATION_MODE_DISCONNECTED (0x02) --  Station mode is disconnected 
 */
}qcmap_msgr_station_mode_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Enables the WLAN. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_enable_wlan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables the WLAN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_wlan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Disables the WLAN. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_disable_wlan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables the WLAN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_wlan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Adds a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  SNAT Entry Configuration */
  qcmap_msgr_snat_entry_config_v01 snat_entry_config;
}qcmap_msgr_add_static_nat_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Adds a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_add_static_nat_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Deletes a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  SNAT Entry Configuration */
  qcmap_msgr_snat_entry_config_v01 snat_entry_config;
}qcmap_msgr_delete_static_nat_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Deletes a static NAT entry. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_delete_static_nat_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries all static NAT entries. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_static_nat_entries_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
  qcmap_msgr_snat_entry_config_v01 snat_config[QCMAP_MSGR_MAX_SNAT_ENTRIES_V01];
}qcmap_msgr_get_static_nat_entries_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the DMZ (perimeter network) IP address for the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  DMZ IP Address */
  uint32_t dmz_ip_addr;
  /**<   DMZ IP address. */
}qcmap_msgr_set_dmz_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the DMZ (perimeter network) IP address for the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_dmz_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the DMZ IP address on the mobile AP. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_dmz_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
  /**<   DMZ IP address. */
}qcmap_msgr_get_dmz_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Deletes the DMZ entry or DMZ IP address. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_delete_dmz_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Deletes the DMZ entry or DMZ IP address. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_delete_dmz_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets WWAN statistics. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  IP Family */
  qcmap_msgr_ip_family_enum_v01 ip_family;
  /**<   Identifies the IP version to be used. Values: \n
      - QCMAP_MSGR_IP_FAMILY_V4 (0x04) --  IPv4 version 
      - QCMAP_MSGR_IP_FAMILY_V6 (0x06) --  IPv6 version 
      - QCMAP_MSGR_IP_FAMILY_V4V6 (0x0A) --  Dual mode version 
 */
}qcmap_msgr_get_wwan_stats_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets WWAN statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WWAN Statistics */
  uint8_t wwan_stats_valid;  /**< Must be set to true if wwan_stats is being passed */
  qcmap_msgr_wwan_statistics_type_v01 wwan_stats;
}qcmap_msgr_get_wwan_stats_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Resets WWAN statistics. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  WWAN Entry With IP Version Configured */
  qcmap_msgr_ip_family_enum_v01 ip_family;
  /**<   Identifies the IP version to be used. Values: \n
      - QCMAP_MSGR_IP_FAMILY_V4 (0x04) --  IPv4 version 
      - QCMAP_MSGR_IP_FAMILY_V6 (0x06) --  IPv6 version 
      - QCMAP_MSGR_IP_FAMILY_V4V6 (0x0A) --  Dual mode version 
 */
}qcmap_msgr_reset_wwan_stats_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Resets WWAN statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_reset_wwan_stats_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an IPSec VPN passthrough is allowed; boolean value.
  */
}qcmap_msgr_set_ipsec_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures the Internet Protocol security (IPSec) 
           Virtual Private Network (VPN) passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_ipsec_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the IPSec VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_ipsec_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
}qcmap_msgr_get_ipsec_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether a PPTP VPN passthrough is allowed; boolean value.
  */
}qcmap_msgr_set_pptp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures the Point-to-Point Tunneling Protocol (PPTP) VPN 
           passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_pptp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the PPTP VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_pptp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
  /**<   Indicates whether a PPTP VPN passthrough is allowed; boolean value.
  */
}qcmap_msgr_get_pptp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  VPN Passthrough Value */
  uint8_t vpn_pass_through_value;
  /**<   Indicates whether an L2TP VPN passthrough is allowed; boolean value.
  */
}qcmap_msgr_set_l2tp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures the Layer 2 Tunneling Protocol (L2TP) VPN passthrough 
           setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_l2tp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the L2TP VPN passthrough setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_l2tp_vpn_pass_through_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
  /**<   Indicates whether an L2TP VPN passthrough is allowed; boolean value.
  */
}qcmap_msgr_get_l2tp_vpn_pass_through_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Configures the NAT type setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_AP_ ENABLE_REQ.
   */

  /* Mandatory */
  /*  NAT Type Value */
  qcmap_msgr_nat_enum_v01 nat_type;
  /**<   Indicates the type of NAT. Values: \n
      - QCMAP_MSGR_NAT_SYMMETRIC_NAT (0) --  NAT type is symmetric 
      - QCMAP_MSGR_NAT_PORT_RESTRICTED_CONE_NAT (1) --  NAT type is Port-Restricted Cone NAT 
      - QCMAP_MSGR_NAT_FULL_CONE_NAT (2) --  NAT type is Full Cone NAT 
      - QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT (3) --  NAT type is Address-Restricted NAT 
 */
}qcmap_msgr_set_nat_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures the NAT type setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_nat_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the NAT type setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
    The value must be the handle previously returned by
    QMI_QCMAP_MOBILE_AP_ ENABLE_REQ.
   */
}qcmap_msgr_get_nat_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the NAT type setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  NAT Type Value */
  uint8_t nat_type_valid;  /**< Must be set to true if nat_type is being passed */
  qcmap_msgr_nat_enum_v01 nat_type;
  /**<   Indicates the type of NAT. Values: \n
      - QCMAP_MSGR_NAT_SYMMETRIC_NAT (0) --  NAT type is symmetric 
      - QCMAP_MSGR_NAT_PORT_RESTRICTED_CONE_NAT (1) --  NAT type is Port-Restricted Cone NAT 
      - QCMAP_MSGR_NAT_FULL_CONE_NAT (2) --  NAT type is Full Cone NAT 
      - QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT (3) --  NAT type is Address-Restricted NAT 
 */
}qcmap_msgr_get_nat_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the autoconnect flag. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Autoconnect Mode Flag */
  uint8_t enable;
  /**<   Enable/disable autoconnect. Values:\n
      - TRUE -- Enable\n
      - FALSE -- Disable
   */
}qcmap_msgr_set_auto_connect_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the autoconnect flag. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_auto_connect_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the autoconnect flag. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_auto_connect_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the autoconnect flag. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Autoconnect Mode */
  uint8_t auto_conn_flag_valid;  /**< Must be set to true if auto_conn_flag is being passed */
  uint8_t auto_conn_flag;
  /**<   Autoconnect status. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled
   */
}qcmap_msgr_get_auto_connect_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Configures whether QCMAP_MSGR initiates WWAN data calls while roaming. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Roaming Mode */
  uint8_t allow_wwan_calls_while_roaming;
  /**<   Whether QCMAP_MSGR connects a data call while roaming; boolean value.
  */
}qcmap_msgr_set_roaming_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures whether QCMAP_MSGR initiates WWAN data calls while roaming. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_roaming_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the roaming flag. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_roaming_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the roaming flag. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Roaming Mode */
  uint8_t allow_wwan_calls_while_roaming_valid;  /**< Must be set to true if allow_wwan_calls_while_roaming is being passed */
  uint8_t allow_wwan_calls_while_roaming;
  /**<   Whether QCMAP_MSGR connects a data call while roaming; boolean value.
  */
}qcmap_msgr_get_roaming_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Adds IP filter-based firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
         The value must be the handle previously returned by
         QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
    */

  /* Mandatory */
  /*  IP Version */
  qcmap_msgr_ip_family_enum_v01 ip_version;
  /**<   Identifies the firewall family version. Values: \n
      - QCMAP_MSGR_IP_FAMILY_V4 (0x04) --  IPv4 version 
      - QCMAP_MSGR_IP_FAMILY_V6 (0x06) --  IPv6 version 
      - QCMAP_MSGR_IP_FAMILY_V4V6 (0x0A) --  Dual mode version 
 */

  /* Optional */
  /*  Next Protocol Header After the IP Header */
  uint8_t next_hdr_prot_valid;  /**< Must be set to true if next_hdr_prot is being passed */
  uint8_t next_hdr_prot;
  /**<   IPv4/IPv6 next header protocol. */

  /* Optional */
  /*  TCP, UDP, and TCP_UDP Source Port */
  uint8_t tcp_udp_src_valid;  /**< Must be set to true if tcp_udp_src is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_src;

  /* Optional */
  /*  TCP, UDP, and TCP_UDP Destination Port */
  uint8_t tcp_udp_dst_valid;  /**< Must be set to true if tcp_udp_dst is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_dst;

  /* Optional */
  /*  ICMP Type */
  uint8_t icmp_type_valid;  /**< Must be set to true if icmp_type is being passed */
  uint8_t icmp_type;
  /**<   ICMP type, as specified in the ICMP protocol
         (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ICMP Code */
  uint8_t icmp_code_valid;  /**< Must be set to true if icmp_code is being passed */
  uint8_t icmp_code;
  /**<   ICMP code as specified in the ICMP protocol
         (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ESP Security Parameter Index */
  uint8_t esp_spi_valid;  /**< Must be set to true if esp_spi is being passed */
  uint32_t esp_spi;
  /**<   Security parameter index, as specified in the ESP protocol
        (RFC 4303 \hyperref[S7]{[S7]}). */

  /* Optional */
  /*  IPv4 Source Address and Subnet Mask */
  uint8_t ip4_src_addr_valid;  /**< Must be set to true if ip4_src_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_src_addr;

  /* Optional */
  /*  IPv4 Destination Address and Subnet Mask */
  uint8_t ip4_dst_addr_valid;  /**< Must be set to true if ip4_dst_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_dst_addr;

  /* Optional */
  /*  IPv4 TOS Value and Mask */
  uint8_t ip4_tos_valid;  /**< Must be set to true if ip4_tos is being passed */
  qcmap_ip4_tos_v01 ip4_tos;

  /* Optional */
  /*  IPv6 Source Address and Prefix Length */
  uint8_t ip6_src_addr_valid;  /**< Must be set to true if ip6_src_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_src_addr;

  /* Optional */
  /*  IPv6 Destination Address and Prefix Length */
  uint8_t ip6_dst_addr_valid;  /**< Must be set to true if ip6_dst_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_dst_addr;

  /* Optional */
  /*  IPv6 Traffic Class Value and Mask */
  uint8_t ip6_trf_cls_valid;  /**< Must be set to true if ip6_trf_cls is being passed */
  qcmap_ip6_traffic_class_v01 ip6_trf_cls;
}qcmap_msgr_add_firewall_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Adds IP filter-based firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Handle */
  uint8_t firewall_handle_valid;  /**< Must be set to true if firewall_handle is being passed */
  uint32_t firewall_handle;
  /**<   Identifies the handle for the added firewall rule. */
}qcmap_msgr_add_firewall_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the handles of all firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       Value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  IP Version  */
  qcmap_msgr_ip_family_enum_v01 ip_version;
  /**<   Identifies the firewall family version. Values: \n
      - QCMAP_MSGR_IP_FAMILY_V4 (0x04) --  IPv4 version 
      - QCMAP_MSGR_IP_FAMILY_V6 (0x06) --  IPv6 version 
      - QCMAP_MSGR_IP_FAMILY_V4V6 (0x0A) --  Dual mode version 
 */
}qcmap_msgr_get_firewall_entries_handle_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the handles of all firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Handle List */
  uint8_t firewall_handle_list_valid;  /**< Must be set to true if firewall_handle_list is being passed */
  uint32_t firewall_handle_list_len;  /**< Must be set to # of elements in firewall_handle_list */
  uint32_t firewall_handle_list[QCMAP_MSGR_MAX_FIREWALL_ENTRIES_V01];
  /**<   Handles identifying the firewall entry. The value must be the handle
       previously returned by QMI_QCMAP_MSGR_ ADD_FIREWALL_ENTRY_RESP.
   */
}qcmap_msgr_get_firewall_entries_handle_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */

  /* Mandatory */
  /*  Firewall Handle */
  uint32_t firewall_handle;
  /**<   Handle identifying the firewall entry. The value must be the handle
       previously returned by
       QMI_QCMAP_MSGR_ADD_ FIREWALL_ENTRY_RESP,
       QMI_QCMAP_MSGR_GET_ FIREWALL_ENTRY_RESP, or
       QMI_QCMAP_MSGR_GET_ FIREWALL_ENTRIES_HANDLE_ LIST_RESP.
   */
}qcmap_msgr_get_firewall_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets firewall rules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IP Version  */
  uint8_t ip_version_valid;  /**< Must be set to true if ip_version is being passed */
  qcmap_msgr_ip_family_enum_v01 ip_version;
  /**<   Identifies the firewall family version. Values: \n
      - QCMAP_MSGR_IP_FAMILY_V4 (0x04) --  IPv4 version 
      - QCMAP_MSGR_IP_FAMILY_V6 (0x06) --  IPv6 version 
      - QCMAP_MSGR_IP_FAMILY_V4V6 (0x0A) --  Dual mode version 
 */

  /* Optional */
  /*  Next Header Protocol */
  uint8_t next_hdr_prot_valid;  /**< Must be set to true if next_hdr_prot is being passed */
  uint8_t next_hdr_prot;
  /**<   IPv4/IPv6 next header protocol. */

  /* Optional */
  /*  TCP, UDP, and TCP_UDP Source Port */
  uint8_t tcp_udp_src_valid;  /**< Must be set to true if tcp_udp_src is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_src;

  /* Optional */
  /*  TCP, UDP, and TCP_UDP Destination Port */
  uint8_t tcp_udp_dst_valid;  /**< Must be set to true if tcp_udp_dst is being passed */
  qcmap_tcp_udp_port_range_v01 tcp_udp_dst;

  /* Optional */
  /*  ICMP Type */
  uint8_t icmp_type_valid;  /**< Must be set to true if icmp_type is being passed */
  uint8_t icmp_type;
  /**<   ICMP type, as specified in the ICMP protocol
       (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ICMP Code */
  uint8_t icmp_code_valid;  /**< Must be set to true if icmp_code is being passed */
  uint8_t icmp_code;
  /**<   ICMP code, as specified in the ICMP protocol
       (RFC 792 \hyperref[S3]{[S3]}). */

  /* Optional */
  /*  ESP Security Parameter Index */
  uint8_t esp_spi_valid;  /**< Must be set to true if esp_spi is being passed */
  uint32_t esp_spi;
  /**<   Security parameter index, as specified in the ESP protocol
       (RFC 4303 \hyperref[S7]{[S7]}). */

  /* Optional */
  /*  IPv4 Source Address and Subnet Mask */
  uint8_t ip4_src_addr_valid;  /**< Must be set to true if ip4_src_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_src_addr;

  /* Optional */
  /*  IPv4 Destination Address and Subnet Mask */
  uint8_t ip4_dst_addr_valid;  /**< Must be set to true if ip4_dst_addr is being passed */
  qcmap_ip4_addr_subnet_mask_v01 ip4_dst_addr;

  /* Optional */
  /*  IPv4 TOS Value and Mask */
  uint8_t ip4_tos_valid;  /**< Must be set to true if ip4_tos is being passed */
  qcmap_ip4_tos_v01 ip4_tos;

  /* Optional */
  /*  IPv6 Source Address and Prefix Length */
  uint8_t ip6_src_addr_valid;  /**< Must be set to true if ip6_src_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_src_addr;

  /* Optional */
  /*  IPv6 Destination Address and Prefix Length */
  uint8_t ip6_dst_addr_valid;  /**< Must be set to true if ip6_dst_addr is being passed */
  qcmap_ip6_addr_prefix_len_v01 ip6_dst_addr;

  /* Optional */
  /*  IPv6 Traffic Class Value and Mask */
  uint8_t ip6_trf_cls_valid;  /**< Must be set to true if ip6_trf_cls is being passed */
  qcmap_ip6_traffic_class_v01 ip6_trf_cls;
}qcmap_msgr_get_firewall_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Deletes a firewall rule identified by a handle. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
         Value must be the handle previously returned by
         QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
    */

  /* Mandatory */
  /*  Firewall Handle */
  uint32_t firewall_handle;
  /**<   Handle identifying the firewall entry.
    */
}qcmap_msgr_delete_firewall_entry_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Deletes a firewall rule identified by a handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_delete_firewall_entry_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the WWAN IP configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Address Type  */
  qcmap_msgr_addr_type_mask_v01 addr_type_op;
  /**<   WWAN configuration mask. Values: \n
      - QCMAP_MSGR_MASK_V4_ADDR (0x00000001) --  IPv4 address 
      - QCMAP_MSGR_MASK_V6_ADDR (0x00000002) --  IPv6 address 
      - QCMAP_MSGR_MASK_V4_DNS_ADDR (0x00000004) --  IPv4 DNS address  
      - QCMAP_MSGR_MASK_V6_DNS_ADDR (0x00000008) --  IPv6 DNS address 
 */
}qcmap_msgr_get_wwan_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
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
  uint8_t v6_addr[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
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
  uint8_t v6_prim_dns_addr[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  /**<   IPv6 primary DNS address. */

  /* Optional */
  /*  IPv6 Secondary DNS Address */
  uint8_t v6_sec_dns_addr_valid;  /**< Must be set to true if v6_sec_dns_addr is being passed */
  uint8_t v6_sec_dns_addr[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  /**<   IPv6 secondary DNS address. */
}qcmap_msgr_get_wwan_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Indicates a change in the current mobile AP status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
         The value must be the handle previously returned by
         QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
   */
}qcmap_msgr_mobile_ap_status_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Indicates a change in the current mobile AP status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Mobile AP Status */
  uint8_t mobile_ap_status_valid;  /**< Must be set to true if mobile_ap_status is being passed */
  qcmap_msgr_mobile_ap_status_enum_v01 mobile_ap_status;
  /**<   Indicates the mobile AP status. Values: \n
      - QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED (0x01) --  Mobile AP is connected 
      - QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED (0x02) --  Mobile AP is disconnected 
 */
}qcmap_msgr_mobile_ap_status_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the WWAN status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  WWAN Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  qcmap_msgr_wwan_call_type_v01 call_type;
  /**<   WWAN call type for which the status is needed. Values: \n
      - QCMAP_MSGR_WWAN_CALL_TYPE_V4 (0x01) --  IPv4 call 
      - QCMAP_MSGR_WWAN_CALL_TYPE_V6 (0x02) --  IPv6 call  
 */
}qcmap_msgr_wwan_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the WWAN status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WWAN Connection Status */
  uint8_t conn_status_valid;  /**< Must be set to true if conn_status is being passed */
  qcmap_msgr_wwan_status_enum_v01 conn_status;
  /**<   WWAN connection status. Values: \n
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING (0x01) --  IPv4 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL (0x02) --  IPv4 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_CONNECTED (0x03) --  IPv4 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING (0x04) --  IPv4 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL (0x05) --  IPv4 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_DISCONNECTED (0x06) --  IPv4 WWAN is disconnected 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING (0x07) --  IPv6 WWAN is in connecting state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL (0x08) --  IPv6 connection to WWAN failed 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED (0x09) --  IPv6 WWAN is in connected state 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING (0x0A) --  IPv6 WWAN is disconnecting 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL (0x0B) --  IPv6 WWAN failed to disconnect 
      - QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED (0x0C) --  IPv6 WWAN is disconnected  
 */
}qcmap_msgr_wwan_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_station_mode_status_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_station_mode_status_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the Station Mode status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WWAN Connection Status */
  uint8_t conn_status_valid;  /**< Must be set to true if conn_status is being passed */
  qcmap_msgr_station_mode_status_enum_v01 conn_status;
  /**<   Station Mode connection status. Values: \n
      - QCMAP_MSGR_STATION_MODE_CONNECTED (0x01) --  Station mode is connected 
      - QCMAP_MSGR_STATION_MODE_DISCONNECTED (0x02) --  Station mode is disconnected  
 */
}qcmap_msgr_get_station_mode_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Configures different types of NAT timeouts. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  NAT Timeout Type */
  qcmap_msgr_nat_timeout_enum_v01 nat_timeout_type;
  /**<   Indicates the NAT Timeout type to be used. Values: \n
      - QCMAP_MSGR_NAT_TIMEOUT_GENERIC (0x01) --   Generic NAT timeout 
      - QCMAP_MSGR_NAT_TIMEOUT_ICMP (0x02) --   NAT timeout for ICMP 
      - QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED (0x03) --   NAT timeout for the TCP established 
      - QCMAP_MSGR_NAT_TIMEOUT_UDP (0x04) --   NAT timeout for UDP 
      - QCMAP_MSGR_NAT_TIMEOUT_UDP_STREAM (0x05) --   NAT timeout for UDP stream 
 */

  /* Mandatory */
  /*  NAT Timeout Value */
  uint32_t timeout_value;
  /**<   Indicates the NAT Timeout value to be used, in seconds.
  */
}qcmap_msgr_set_nat_timeout_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures different types of NAT timeouts. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_nat_timeout_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the NAT timeout setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  NAT Timeout Type */
  qcmap_msgr_nat_timeout_enum_v01 nat_timeout_type;
  /**<   Indicates the NAT Timeout type to be used. Values:\n
      - QCMAP_MSGR_NAT_TIMEOUT_GENERIC (0x01) --   Generic NAT timeout 
      - QCMAP_MSGR_NAT_TIMEOUT_ICMP (0x02) --   NAT timeout for ICMP 
      - QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED (0x03) --   NAT timeout for the TCP established 
      - QCMAP_MSGR_NAT_TIMEOUT_UDP (0x04) --   NAT timeout for UDP 
      - QCMAP_MSGR_NAT_TIMEOUT_UDP_STREAM (0x05) --   NAT timeout for UDP stream 
 */
}qcmap_msgr_get_nat_timeout_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries the NAT timeout setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  NAT Timeout */
  uint8_t timeout_value_valid;  /**< Must be set to true if timeout_value is being passed */
  uint32_t timeout_value;
  /**<   Indicates the required timeout value, in seconds.
  */
}qcmap_msgr_get_nat_timeout_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the LAN configuration. */
typedef struct {

  /* Optional */
  /*  AP LAN Configuration */
  uint8_t lan_config_valid;  /**< Must be set to true if lan_config is being passed */
  qcmap_msgr_lan_config_v01 lan_config;
}qcmap_msgr_set_lan_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the LAN configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_lan_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the WLAN configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  WLAN Mode */
  uint8_t wlan_mode_valid;  /**< Must be set to true if wlan_mode is being passed */
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode;
  /**<   Identifies the WLAN mode. Values: \n
      - QCMAP_MSGR_WLAN_MODE_AP (0x01) --  WLAN is in AP mode 
      - QCMAP_MSGR_WLAN_MODE_AP_AP (0x02) --  WLAN is in AP/AP Mode 
      - QCMAP_MSGR_WLAN_MODE_AP_STA (0x03) --  WLAN is in AP/STA Mode 
 */

  /* Optional */
  /*  Guest AP Access profile */
  uint8_t guest_ap_acess_profile_valid;  /**< Must be set to true if guest_ap_acess_profile is being passed */
  qcmap_msgr_access_profile_v01 guest_ap_acess_profile;
  /**<   Sets guest ap access profile \n
  */

  /* Optional */
  /*  Station Configuration */
  uint8_t station_config_valid;  /**< Must be set to true if station_config is being passed */
  qcmap_msgr_station_mode_config_v01 station_config;
}qcmap_msgr_set_wlan_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the WLAN configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_wlan_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the WLAN status. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
        The value must be the handle previously returned by
        QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
   */
}qcmap_msgr_get_wlan_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the WLAN status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WLAN Mode */
  uint8_t wlan_mode_valid;  /**< Must be set to true if wlan_mode is being passed */
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode;
  /**<   Identifies the WLAN mode. Values: \n
      - QCMAP_MSGR_WLAN_MODE_AP (0x01) --  WLAN is in AP mode 
      - QCMAP_MSGR_WLAN_MODE_AP_AP (0x02) --  WLAN is in AP/AP Mode 
      - QCMAP_MSGR_WLAN_MODE_AP_STA (0x03) --  WLAN is in AP/STA Mode 
 */
}qcmap_msgr_get_wlan_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Activates the WLAN with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_activate_wlan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Activates the WLAN with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_activate_wlan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Activates the LAN with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_activate_lan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Activates the LAN with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_activate_lan_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_lan_config_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_lan_config_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the LAN configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  AP Configuration */
  uint8_t lan_config_valid;  /**< Must be set to true if lan_config is being passed */
  qcmap_msgr_lan_config_v01 lan_config;

  /* Optional */
  /*  WLAN and USB Configuration on Access Point */
  uint8_t ap_wlan_usb_ip_config_valid;  /**< Must be set to true if ap_wlan_usb_ip_config is being passed */
  qcmap_msgr_ap_mode_wlan_usb_ip_config_v01 ap_wlan_usb_ip_config;
}qcmap_msgr_get_lan_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_wlan_config_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_wlan_config_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the WLAN configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WLAN mode */
  uint8_t wlan_mode_valid;  /**< Must be set to true if wlan_mode is being passed */
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode;
  /**<   Identifies the WLAN mode. Values: \n
      - QCMAP_MSGR_WLAN_MODE_AP (0x01) --  WLAN is in AP mode 
      - QCMAP_MSGR_WLAN_MODE_AP_AP (0x02) --  WLAN is in AP/AP Mode 
      - QCMAP_MSGR_WLAN_MODE_AP_STA (0x03) --  WLAN is in AP/STA Mode 
 */

  /* Optional */
  /*  Station Configuration */
  uint8_t station_config_valid;  /**< Must be set to true if station_config is being passed */
  qcmap_msgr_station_mode_config_v01 station_config;

  /* Optional */
  /*  Guest Configuration on Access Point */
  uint8_t guest_ap_access_profile_valid;  /**< Must be set to true if guest_ap_access_profile is being passed */
  qcmap_msgr_access_profile_v01 guest_ap_access_profile;
}qcmap_msgr_get_wlan_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Activates the HOSTAPD with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  AP type  */
  uint8_t ap_type_valid;  /**< Must be set to true if ap_type is being passed */
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type;
  /**<   Identifies the AP for which settings need to be activated. Values: \n
      - QCMAP_MSGR_PRIMARY_AP (0x01) --  Primary AP. 
      - QCMAP_MSGR_GUEST_AP (0x02) --  Guest AP. 
      - QCMAP_MSGR_PRIMARY_AND_GUEST_AP (0x03) --  Both the AP's. 
 */

  /* Optional */
  /*  Action Type  */
  uint8_t action_type_valid;  /**< Must be set to true if action_type is being passed */
  qcmap_msgr_activate_hostapd_action_enum_v01 action_type;
  /**<   Identifies the hostapd action which needs to be activated. Values: \n
      - QCMAP_MSGR_HOSTAPD_START (0x01) --  Start hostapd. 
      - QCMAP_MSGR_HOSTAPD_STOP (0x02) --  Stop hostapd. 
      - QCMAP_MSGR_HOSTAPD_RESTART (0x03) --  Restart hostapd. 
 */
}qcmap_msgr_activate_hostapd_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Activates the HOSTAPD with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_activate_hostapd_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Activates the supplicant with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_activate_supplicant_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Activates the supplicant with the latest configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_activate_supplicant_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets up a USB link for RNDIS/ECM tethering. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
	   QMI_QCMAP_ MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  USB Interface Name */
  qcmap_msgr_usb_link_enum_v01 usb_link;
  /**<   Identifies the USB tethered link type. Values: \n
      - QCMAP_MSGR_USB_LINK_RNDIS (0x01) --  USB tethered link is RNDIS 
      - QCMAP_MSGR_USB_LINK_ECM (0x02) --  USB tethered link is ECM 
 */
}qcmap_msgr_usb_link_up_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets up a USB link for RNDIS/ECM tethering. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_usb_link_up_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Brings down a USB link for RNDIS/ECM tethering. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
	   QMI_QCMAP_ MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  USB Interface Name */
  qcmap_msgr_usb_link_enum_v01 usb_link;
  /**<   Identifies the USB tethered link type. Values: \n
      - QCMAP_MSGR_USB_LINK_RNDIS (0x01) --  USB tethered link is RNDIS 
      - QCMAP_MSGR_USB_LINK_ECM (0x02) --  USB tethered link is ECM 
 */
}qcmap_msgr_usb_link_down_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Brings down a USB link for RNDIS/ECM tethering. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_usb_link_down_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the WWAN policy. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
   */

  /* Mandatory */
  /*  WWAN Policy  */
  qcmap_msgr_net_policy_info_v01 wwan_policy;
}qcmap_msgr_set_wwan_policy_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the WWAN policy. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_wwan_policy_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_wwan_policy_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_wwan_policy_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries the WWAN policy. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  WWAN Policy  */
  uint8_t wwan_policy_valid;  /**< Must be set to true if wwan_policy is being passed */
  qcmap_msgr_net_policy_info_v01 wwan_policy;
}qcmap_msgr_get_wwan_policy_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_enable_upnp_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_enable_upnp_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables UPnP discovery of other networked devices. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_upnp_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_disable_upnp_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_disable_upnp_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables UPnP discovery of other networked devices. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_upnp_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_enable_dlna_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_enable_dlna_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables a DLNA media server to share content with network devices. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_dlna_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_disable_dlna_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_disable_dlna_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables a DLNA media server from sharing content with network devices. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_dlna_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the firewall configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Firewall Enabled */
  uint8_t firewall_enabled;
  /**<   Indicates whether the firewall is to be enabled or disabled; boolean value.
  */

  /* Optional */
  /*  Accept or Drop Packets */
  uint8_t pkts_allowed_valid;  /**< Must be set to true if pkts_allowed is being passed */
  uint8_t pkts_allowed;
  /**<   Indicates whether packets are to be accepted or dropped; boolean value.
  */
}qcmap_msgr_set_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the firewall configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Gets the firewall configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_firewall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the firewall configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Firewall Enabled */
  uint8_t firewall_enabled_valid;  /**< Must be set to true if firewall_enabled is being passed */
  uint8_t firewall_enabled;
  /**<   Indicates whether the firewall is enabled or disabled; boolean value.
  */

  /* Optional */
  /*  Accept or Drop Packets */
  uint8_t pkts_allowed_valid;  /**< Must be set to true if pkts_allowed is being passed */
  uint8_t pkts_allowed;
  /**<   Indicates whether packets are accepted or dropped; boolean value.
  */
}qcmap_msgr_get_firewall_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_enable_multicast_dns_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_enable_multicast_dns_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables service discovery over a local LAN using multicast DNS. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_multicast_dns_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_disable_multicast_dns_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_disable_multicast_dns_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables service discovery over a local LAN using multicast DNS. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_multicast_dns_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_upnp_status_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_upnp_status_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the UPnP status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  UPNP Mode */
  uint8_t upnp_mode_valid;  /**< Must be set to true if upnp_mode is being passed */
  qcmap_msgr_upnp_mode_enum_v01 upnp_mode;
  /**<   Identifies the UPnP mode. Values: \n
      - QCMAP_MSGR_UPNP_MODE_UP (0x01) --  UPNP is in AP mode 
      - QCMAP_MSGR_UPNP_MODE_DOWN (0x02) --  UPNP is in AP/AP Mode 
 */
}qcmap_msgr_get_upnp_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_dlna_status_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_dlna_status_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the DLNA status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  DLNA Mode */
  uint8_t dlna_mode_valid;  /**< Must be set to true if dlna_mode is being passed */
  qcmap_msgr_dlna_mode_enum_v01 dlna_mode;
  /**<   Identifies the DLNA mode. Values: \n
      - QCMAP_MSGR_DLNA_MODE_UP (0x01) --  DLNA is in AP mode 
      - QCMAP_MSGR_DLNA_MODE_DOWN (0x02) --  DLNA is in AP/AP Mode 
 */
}qcmap_msgr_get_dlna_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_multicast_dns_status_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_multicast_dns_status_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets the Multicast DNS status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  MDNS Mode */
  uint8_t mdns_mode_valid;  /**< Must be set to true if mdns_mode is being passed */
  qcmap_msgr_mdns_mode_enum_v01 mdns_mode;
  /**<   Identifies the Multicast DNS mode. Values: \n
      - QCMAP_MSGR_MDNS_MODE_UP (0x01) --  MDNS is in AP mode 
      - QCMAP_MSGR_MDNS_MODE_DOWN (0x02) --  MDNS is in AP/AP Mode 
 */
}qcmap_msgr_get_multicast_dns_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets Bootup values for MobileAP components. */
typedef struct {

  /* Optional */
  /*  Mobile AP Bootup Flag */
  uint8_t mobileap_bootup_flag_valid;  /**< Must be set to true if mobileap_bootup_flag is being passed */
  qcmap_msgr_bootup_flag_v01 mobileap_bootup_flag;
  /**<   This flag is used to start Mobile AP on bootup.
        If set to true Mobile AP would be enabled on bootup.      
  */

  /* Optional */
  /*  WLAN Bootup Flag */
  uint8_t wlan_bootup_flag_valid;  /**< Must be set to true if wlan_bootup_flag is being passed */
  qcmap_msgr_bootup_flag_v01 wlan_bootup_flag;
  /**<   This flag is used to bring up WLAN on bootup.
        If set to true, then WLAN would be enabled on bootup.
  */
}qcmap_msgr_set_qcmap_bootup_cfg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets Bootup values for MobileAP components. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_qcmap_bootup_cfg_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_qcmap_bootup_cfg_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_qcmap_bootup_cfg_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Gets Bootup values for QCMAP components 
    on the ARM\reg Cortex\tmns-A5 processor. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Mobile AP Bootup Flag */
  uint8_t mobileap_bootup_flag_valid;  /**< Must be set to true if mobileap_bootup_flag is being passed */
  qcmap_msgr_bootup_flag_v01 mobileap_bootup_flag;
  /**<   This flag will indicate the bootup behaviour of Mobile AP.
        If found to be true, Mobile AP would be enabled on bootup. 
  */

  /* Optional */
  /*  WLAN Bootup Flag */
  uint8_t wlan_bootup_flag_valid;  /**< Must be set to true if wlan_bootup_flag is being passed */
  qcmap_msgr_bootup_flag_v01 wlan_bootup_flag;
  /**<   This flag indicated the bootup behaviour of WLAN.
        If found to be true, WLAN would be enabled on bootup.  
  */
}qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the DLNA media directory. */
typedef struct {

  /* Mandatory */
  /*  Media Directory */
  uint32_t media_dir_len;  /**< Must be set to # of elements in media_dir */
  char media_dir[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01];
}qcmap_msgr_set_dlna_media_dir_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the DLNA media directory. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_dlna_media_dir_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_dlna_media_dir_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_dlna_media_dir_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Retrieves the DLNA media directory. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Media Directory */
  uint8_t media_dir_valid;  /**< Must be set to true if media_dir is being passed */
  uint32_t media_dir_len;  /**< Must be set to # of elements in media_dir */
  char media_dir[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01];
}qcmap_msgr_get_dlna_media_dir_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Retrieves the current data bitrates. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_data_bitrate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Retrieves the current data bitrates. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Media Directory */
  uint8_t data_rate_valid;  /**< Must be set to true if data_rate is being passed */
  qcmap_msgr_data_bitrate_v01 data_rate;
}qcmap_msgr_get_data_bitrate_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_upnp_notify_interval_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_upnp_notify_interval_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Retrieves UPnP notify interval. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Notify Interval */
  uint8_t notify_interval_valid;  /**< Must be set to true if notify_interval is being passed */
  int32_t notify_interval;
  /**<   Specifies the UPnP notify interval in seconds, 30-60000. \n
  */
}qcmap_msgr_get_upnp_notify_interval_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the UPnP notify interval. */
typedef struct {

  /* Optional */
  /*  Notify Interval */
  uint8_t notify_interval_valid;  /**< Must be set to true if notify_interval is being passed */
  int32_t notify_interval;
  /**<   Specifies the UPnP notify interval in seconds, 30-60000. \n
  */
}qcmap_msgr_set_upnp_notify_interval_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the UPnP notify interval. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_upnp_notify_interval_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * qcmap_msgr_get_dlna_notify_interval_req_msg is empty
 * typedef struct {
 * }qcmap_msgr_get_dlna_notify_interval_req_msg_v01;
 */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Retrieves DLNA notify interval. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Notify Interval */
  uint8_t notify_interval_valid;  /**< Must be set to true if notify_interval is being passed */
  int32_t notify_interval;
  /**<   Specifies the DLNA notify interval in seconds, 30-60000. \n
  */
}qcmap_msgr_get_dlna_notify_interval_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the DLNA notify interval. */
typedef struct {

  /* Optional */
  /*  Notify Interval */
  uint8_t notify_interval_valid;  /**< Must be set to true if notify_interval is being passed */
  int32_t notify_interval;
  /**<   Specifies the DLNA notify interval in seconds, 30-60000. \n
  */
}qcmap_msgr_set_dlna_notify_interval_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the DLNA notify interval. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_dlna_notify_interval_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Adds DHCP reservation record. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  DHCP Reservation Configuration */
  qcmap_msgr_dhcp_reservation_v01 dhcp_reservation_record;
}qcmap_msgr_add_dhcp_reservation_record_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Adds DHCP reservation record. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_add_dhcp_reservation_record_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries all DHCP records and display */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_dhcp_reservation_records_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries all DHCP records and display */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Display All DHCP Records */
  uint8_t dhcp_reservation_records_valid;  /**< Must be set to true if dhcp_reservation_records is being passed */
  uint32_t dhcp_reservation_records_len;  /**< Must be set to # of elements in dhcp_reservation_records */
  qcmap_msgr_dhcp_reservation_v01 dhcp_reservation_records[QCMAP_MSGR_MAX_DHCP_RESERVATION_ENTRIES_V01];
}qcmap_msgr_get_dhcp_reservation_records_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Edit a DHCP record */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  Client Reserved IP to Access Record */
  uint8_t client_reserved_ip_valid;  /**< Must be set to true if client_reserved_ip is being passed */
  uint32_t client_reserved_ip;

  /* Optional */
  /*  DHCP Record to Edit */
  uint8_t dhcp_reservation_record_valid;  /**< Must be set to true if dhcp_reservation_record is being passed */
  qcmap_msgr_dhcp_reservation_v01 dhcp_reservation_record;
}qcmap_msgr_edit_dhcp_reservation_record_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Edit a DHCP record */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_edit_dhcp_reservation_record_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Deletes a DHCP record */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Optional */
  /*  Client IP */
  uint8_t client_reserved_ip_valid;  /**< Must be set to true if client_reserved_ip is being passed */
  uint32_t client_reserved_ip;
}qcmap_msgr_delete_dhcp_reservation_record_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Deletes a DHCP record */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_delete_dhcp_reservation_record_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Enables ALG Module. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  ALG Type  */
  qcmap_msgr_alg_type_mask_v01 alg_type_op;
  /**<   ALG configuration mask. Values: \n
      - QCMAP_MSGR_MASK_RTSP_ALG (0x01) --  RTSP ALG  
      - QCMAP_MSGR_MASK_SIP_ALG (0x02) --  SIP ALG  
 All other bits are reserved and will be ignored by the service
   ADD more MASK depending on new ALG modules added
  
 */
}qcmap_msgr_enable_alg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Enables ALG Module. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_enable_alg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Disables  ALG and stops data transfer if in progress. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  ALG Type  */
  qcmap_msgr_alg_type_mask_v01 alg_type_op;
  /**<   ALG configuration mask. Values: \n
      - QCMAP_MSGR_MASK_RTSP_ALG (0x01) --  RTSP ALG  
      - QCMAP_MSGR_MASK_SIP_ALG (0x02) --  SIP ALG  
 All other bits are reserved and will be ignored by the service
   ADD more MASK depending on new ALG modules added
  
 */
}qcmap_msgr_disable_alg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Disables  ALG and stops data transfer if in progress. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_disable_alg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Configures the webserver access from WWAN interface. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Webserver WWAN Access Flag */
  uint8_t webserver_wwan_access;
  /**<   Indicates whether webserver can be accessed from WWAN interface or not;
       boolean value.
  */
}qcmap_msgr_set_webserver_wwan_access_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Configures the webserver access from WWAN interface. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_webserver_wwan_access_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the webserver WWAN access setting. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_ MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_webserver_wwan_access_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries the webserver WWAN access setting. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Webserver WWAN Access Value */
  uint8_t webserver_wwan_access_valid;  /**< Must be set to true if webserver_wwan_access is being passed */
  uint8_t webserver_wwan_access;
  /**<   Indicates whether webserver can be accessed from WWAN; boolean value.
  */
}qcmap_msgr_get_webserver_wwan_access_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Sets the SIP server information. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */

  /* Mandatory */
  /*  Default SIP Server Information */
  qcmap_msgr_sip_server_info_v01 default_sip_server_info;
  /**<   Identifies the default PCSCF information which will be
       provided through DHCP to the Access Point clients */
}qcmap_msgr_set_sip_server_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Sets the SIP server information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}qcmap_msgr_set_sip_server_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Queries the SIP server information. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_get_sip_server_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Queries the SIP server information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Default SIP Server Information */
  uint8_t default_sip_server_info_valid;  /**< Must be set to true if default_sip_server_info is being passed */
  qcmap_msgr_sip_server_info_v01 default_sip_server_info;
  /**<   Identifies the default PCSCF information which will be
       provided through DHCP to the Access Point clients */

  /* Optional */
  /*  Network SIP Server Information */
  uint8_t network_sip_server_info_valid;  /**< Must be set to true if network_sip_server_info is being passed */
  uint32_t network_sip_server_info_len;  /**< Must be set to # of elements in network_sip_server_info */
  qcmap_msgr_sip_server_info_v01 network_sip_server_info[QCMAP_MSGR_MAX_SIP_SERVER_ENTRIES_V01];
  /**<   Identifies the network assigned PCSCF information which will be
       provided through DHCP to the Access Point clients */
}qcmap_msgr_get_sip_server_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Request Message; Resets to factory configuration. */
typedef struct {

  /* Mandatory */
  /*  Mobile AP Handle */
  uint32_t mobile_ap_handle;
  /**<   Handle identifying the mobile AP call instance.
       The value must be the handle previously returned by
       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ.
  */
}qcmap_msgr_restore_factory_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qcmap_msgr_qmi_messages
    @{
  */
/** Response Message; Resets to factory configuration. */
typedef struct {

  /* Optional */
  /*  Result Code */
  uint8_t resp_valid;  /**< Must be set to true if resp is being passed */
  qmi_response_type_v01 resp;
}qcmap_msgr_restore_factory_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup qcmap_msgr_qmi_msg_ids
    @{
  */
#define QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ_V01 0x0020
#define QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_RESP_V01 0x0020
#define QMI_QCMAP_MSGR_MOBILE_AP_DISABLE_REQ_V01 0x0021
#define QMI_QCMAP_MSGR_MOBILE_AP_DISABLE_RESP_V01 0x0021
#define QMI_QCMAP_MSGR_BRING_UP_WWAN_REQ_V01 0x0022
#define QMI_QCMAP_MSGR_BRING_UP_WWAN_RESP_V01 0x0022
#define QMI_QCMAP_MSGR_BRING_UP_WWAN_IND_V01 0x0022
#define QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_REQ_V01 0x0023
#define QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_RESP_V01 0x0023
#define QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_IND_V01 0x0023
#define QMI_QCMAP_MSGR_WWAN_STATUS_IND_REG_REQ_V01 0x0024
#define QMI_QCMAP_MSGR_WWAN_STATUS_IND_REG_RESP_V01 0x0024
#define QMI_QCMAP_MSGR_WWAN_STATUS_IND_V01 0x0025
#define QMI_QCMAP_MSGR_ENABLE_WLAN_REQ_V01 0x0026
#define QMI_QCMAP_MSGR_ENABLE_WLAN_RESP_V01 0x0026
#define QMI_QCMAP_MSGR_DISABLE_WLAN_REQ_V01 0x0027
#define QMI_QCMAP_MSGR_DISABLE_WLAN_RESP_V01 0x0027
#define QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_REG_REQ_V01 0x0028
#define QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_REG_RESP_V01 0x0028
#define QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_V01 0x0029
#define QMI_QCMAP_MSGR_ADD_STATIC_NAT_ENTRY_REQ_V01 0x002A
#define QMI_QCMAP_MSGR_ADD_STATIC_NAT_ENTRY_RESP_V01 0x002A
#define QMI_QCMAP_MSGR_DELETE_STATIC_NAT_ENTRY_REQ_V01 0x002B
#define QMI_QCMAP_MSGR_DELETE_STATIC_NAT_ENTRY_RESP_V01 0x002B
#define QMI_QCMAP_MSGR_GET_STATIC_NAT_ENTRIES_REQ_V01 0x002C
#define QMI_QCMAP_MSGR_GET_STATIC_NAT_ENTRIES_RESP_V01 0x002C
#define QMI_QCMAP_MSGR_SET_DMZ_REQ_V01 0x002D
#define QMI_QCMAP_MSGR_SET_DMZ_RESP_V01 0x002D
#define QMI_QCMAP_MSGR_DELETE_DMZ_REQ_V01 0x002E
#define QMI_QCMAP_MSGR_DELETE_DMZ_RESP_V01 0x002E
#define QMI_QCMAP_MSGR_GET_DMZ_REQ_V01 0x002F
#define QMI_QCMAP_MSGR_GET_DMZ_RESP_V01 0x002F
#define QMI_QCMAP_MSGR_GET_WWAN_STATS_REQ_V01 0x0030
#define QMI_QCMAP_MSGR_GET_WWAN_STATS_RESP_V01 0x0030
#define QMI_QCMAP_MSGR_RESET_WWAN_STATS_REQ_V01 0x0031
#define QMI_QCMAP_MSGR_RESET_WWAN_STATS_RESP_V01 0x0031
#define QMI_QCMAP_MSGR_GET_IPSEC_VPN_PASS_THROUGH_REQ_V01 0x0032
#define QMI_QCMAP_MSGR_GET_IPSEC_VPN_PASS_THROUGH_RESP_V01 0x0032
#define QMI_QCMAP_MSGR_SET_IPSEC_VPN_PASS_THROUGH_REQ_V01 0x0033
#define QMI_QCMAP_MSGR_SET_IPSEC_VPN_PASS_THROUGH_RESP_V01 0x0033
#define QMI_QCMAP_MSGR_GET_PPTP_VPN_PASS_THROUGH_REQ_V01 0x0034
#define QMI_QCMAP_MSGR_GET_PPTP_VPN_PASS_THROUGH_RESP_V01 0x0034
#define QMI_QCMAP_MSGR_SET_PPTP_VPN_PASS_THROUGH_REQ_V01 0x0035
#define QMI_QCMAP_MSGR_SET_PPTP_VPN_PASS_THROUGH_RESP_V01 0x0035
#define QMI_QCMAP_MSGR_GET_L2TP_VPN_PASS_THROUGH_REQ_V01 0x0036
#define QMI_QCMAP_MSGR_GET_L2TP_VPN_PASS_THROUGH_RESP_V01 0x0036
#define QMI_QCMAP_MSGR_SET_L2TP_VPN_PASS_THROUGH_REQ_V01 0x0037
#define QMI_QCMAP_MSGR_SET_L2TP_VPN_PASS_THROUGH_RESP_V01 0x0037
#define QMI_QCMAP_MSGR_SET_NAT_TYPE_REQ_V01 0x0038
#define QMI_QCMAP_MSGR_SET_NAT_TYPE_RESP_V01 0x0038
#define QMI_QCMAP_MSGR_GET_NAT_TYPE_REQ_V01 0x0039
#define QMI_QCMAP_MSGR_GET_NAT_TYPE_RESP_V01 0x0039
#define QMI_QCMAP_MSGR_SET_AUTO_CONNECT_REQ_V01 0x003A
#define QMI_QCMAP_MSGR_SET_AUTO_CONNECT_RESP_V01 0x003A
#define QMI_QCMAP_MSGR_GET_AUTO_CONNECT_REQ_V01 0x003B
#define QMI_QCMAP_MSGR_GET_AUTO_CONNECT_RESP_V01 0x003B
#define QMI_QCMAP_MSGR_ADD_FIREWALL_ENTRY_REQ_V01 0x003C
#define QMI_QCMAP_MSGR_ADD_FIREWALL_ENTRY_RESP_V01 0x003C
#define QMI_QCMAP_MSGR_GET_FIREWALL_ENTRIES_HANDLE_LIST_REQ_V01 0x003D
#define QMI_QCMAP_MSGR_GET_FIREWALL_ENTRIES_HANDLE_LIST_RESP_V01 0x003D
#define QMI_QCMAP_MSGR_GET_FIREWALL_ENTRY_REQ_V01 0x003E
#define QMI_QCMAP_MSGR_GET_FIREWALL_ENTRY_RESP_V01 0x003E
#define QMI_QCMAP_MSGR_DELETE_FIREWALL_ENTRY_REQ_V01 0x003F
#define QMI_QCMAP_MSGR_DELETE_FIREWALL_ENTRY_RESP_V01 0x003F
#define QMI_QCMAP_MSGR_GET_WWAN_CONFIG_REQ_V01 0x0040
#define QMI_QCMAP_MSGR_GET_WWAN_CONFIG_RESP_V01 0x0040
#define QMI_QCMAP_MSGR_MOBILE_AP_STATUS_REQ_V01 0x0041
#define QMI_QCMAP_MSGR_MOBILE_AP_STATUS_RESP_V01 0x0041
#define QMI_QCMAP_MSGR_WWAN_STATUS_REQ_V01 0x0042
#define QMI_QCMAP_MSGR_WWAN_STATUS_RESP_V01 0x0042
#define QMI_QCMAP_MSGR_SET_NAT_TIMEOUT_REQ_V01 0x0043
#define QMI_QCMAP_MSGR_SET_NAT_TIMEOUT_RESP_V01 0x0043
#define QMI_QCMAP_MSGR_GET_NAT_TIMEOUT_REQ_V01 0x0044
#define QMI_QCMAP_MSGR_GET_NAT_TIMEOUT_RESP_V01 0x0044
#define QMI_QCMAP_MSGR_SET_LAN_CONFIG_REQ_V01 0x0045
#define QMI_QCMAP_MSGR_SET_LAN_CONFIG_RESP_V01 0x0045
#define QMI_QCMAP_MSGR_GET_WLAN_STATUS_REQ_V01 0x0046
#define QMI_QCMAP_MSGR_GET_WLAN_STATUS_RESP_V01 0x0046
#define QMI_QCMAP_MSGR_ACTIVATE_WLAN_REQ_V01 0x0047
#define QMI_QCMAP_MSGR_ACTIVATE_WLAN_RESP_V01 0x0047
#define QMI_QCMAP_MSGR_GET_LAN_CONFIG_REQ_V01 0x0048
#define QMI_QCMAP_MSGR_GET_LAN_CONFIG_RESP_V01 0x0048
#define QMI_QCMAP_MSGR_USB_LINK_UP_REQ_V01 0x0049
#define QMI_QCMAP_MSGR_USB_LINK_UP_RESP_V01 0x0049
#define QMI_QCMAP_MSGR_USB_LINK_DOWN_REQ_V01 0x004A
#define QMI_QCMAP_MSGR_USB_LINK_DOWN_RESP_V01 0x004A
#define QMI_QCMAP_MSGR_ENABLE_IPV6_REQ_V01 0x004B
#define QMI_QCMAP_MSGR_ENABLE_IPV6_RESP_V01 0x004B
#define QMI_QCMAP_MSGR_DISABLE_IPV6_REQ_V01 0x004C
#define QMI_QCMAP_MSGR_DISABLE_IPV6_RESP_V01 0x004C
#define QMI_QCMAP_MSGR_SET_ROAMING_PREF_REQ_V01 0x004D
#define QMI_QCMAP_MSGR_SET_ROAMING_PREF_RESP_V01 0x004D
#define QMI_QCMAP_MSGR_GET_ROAMING_PREF_REQ_V01 0x004E
#define QMI_QCMAP_MSGR_GET_ROAMING_PREF_RESP_V01 0x004E
#define QMI_QCMAP_MSGR_SET_WWAN_POLICY_REQ_V01 0x004F
#define QMI_QCMAP_MSGR_SET_WWAN_POLICY_RESP_V01 0x004F
#define QMI_QCMAP_MSGR_GET_WWAN_POLICY_REQ_V01 0x0050
#define QMI_QCMAP_MSGR_GET_WWAN_POLICY_RESP_V01 0x0050
#define QMI_QCMAP_MSGR_GET_IPV6_STATE_REQ_V01 0x0051
#define QMI_QCMAP_MSGR_GET_IPV6_STATE_RESP_V01 0x0051
#define QMI_QCMAP_MSGR_ENABLE_UPNP_REQ_V01 0x0052
#define QMI_QCMAP_MSGR_ENABLE_UPNP_RESP_V01 0x0052
#define QMI_QCMAP_MSGR_DISABLE_UPNP_REQ_V01 0x0053
#define QMI_QCMAP_MSGR_DISABLE_UPNP_RESP_V01 0x0053
#define QMI_QCMAP_MSGR_ENABLE_DLNA_REQ_V01 0x0054
#define QMI_QCMAP_MSGR_ENABLE_DLNA_RESP_V01 0x0054
#define QMI_QCMAP_MSGR_DISABLE_DLNA_REQ_V01 0x0055
#define QMI_QCMAP_MSGR_DISABLE_DLNA_RESP_V01 0x0055
#define QMI_QCMAP_MSGR_SET_FIREWALL_CONFIG_REQ_V01 0x0056
#define QMI_QCMAP_MSGR_SET_FIREWALL_CONFIG_RESP_V01 0x0056
#define QMI_QCMAP_MSGR_GET_FIREWALL_CONFIG_REQ_V01 0x0057
#define QMI_QCMAP_MSGR_GET_FIREWALL_CONFIG_RESP_V01 0x0057
#define QMI_QCMAP_MSGR_ENABLE_MULTICAST_DNS_RESPONDER_REQ_V01 0x0058
#define QMI_QCMAP_MSGR_ENABLE_MULTICAST_DNS_RESPONDER_RESP_V01 0x0058
#define QMI_QCMAP_MSGR_DISABLE_MULTICAST_DNS_RESPONDER_REQ_V01 0x0059
#define QMI_QCMAP_MSGR_DISABLE_MULTICAST_DNS_RESPONDER_RESP_V01 0x0059
#define QMI_QCMAP_MSGR_GET_UPNP_STATUS_REQ_V01 0x005A
#define QMI_QCMAP_MSGR_GET_UPNP_STATUS_RESP_V01 0x005A
#define QMI_QCMAP_MSGR_GET_DLNA_STATUS_REQ_V01 0x005B
#define QMI_QCMAP_MSGR_GET_DLNA_STATUS_RESP_V01 0x005B
#define QMI_QCMAP_MSGR_GET_MULTICAST_DNS_STATUS_REQ_V01 0x005C
#define QMI_QCMAP_MSGR_GET_MULTICAST_DNS_STATUS_RESP_V01 0x005C
#define QMI_QCMAP_MSGR_STATION_MODE_STATUS_IND_REG_REQ_V01 0x005D
#define QMI_QCMAP_MSGR_STATION_MODE_STATUS_IND_REG_RESP_V01 0x005D
#define QMI_QCMAP_MSGR_STATION_MODE_STATUS_IND_V01 0x005E
#define QMI_QCMAP_MSGR_GET_STATION_MODE_STATUS_REQ_V01 0x005F
#define QMI_QCMAP_MSGR_GET_STATION_MODE_STATUS_RESP_V01 0x005F
#define QMI_QCMAP_MSGR_SET_QCMAP_BOOTUP_CFG_REQ_V01 0x0060
#define QMI_QCMAP_MSGR_SET_QCMAP_BOOTUP_CFG_RESP_V01 0x0060
#define QMI_QCMAP_MSGR_GET_QCMAP_BOOTUP_CFG_REQ_V01 0x0061
#define QMI_QCMAP_MSGR_GET_QCMAP_BOOTUP_CFG_RESP_V01 0x0061
#define QMI_QCMAP_MSGR_SET_DLNA_MEDIA_DIR_REQ_V01 0x0062
#define QMI_QCMAP_MSGR_SET_DLNA_MEDIA_DIR_RESP_V01 0x0062
#define QMI_QCMAP_MSGR_GET_DLNA_MEDIA_DIR_REQ_V01 0x0063
#define QMI_QCMAP_MSGR_GET_DLNA_MEDIA_DIR_RESP_V01 0x0063
#define QMI_QCMAP_MSGR_SET_WLAN_CONFIG_REQ_V01 0x0064
#define QMI_QCMAP_MSGR_SET_WLAN_CONFIG_RESP_V01 0x0064
#define QMI_QCMAP_MSGR_ACTIVATE_LAN_REQ_V01 0x0065
#define QMI_QCMAP_MSGR_ACTIVATE_LAN_RESP_V01 0x0065
#define QMI_QCMAP_MSGR_GET_WLAN_CONFIG_REQ_V01 0x0066
#define QMI_QCMAP_MSGR_GET_WLAN_CONFIG_RESP_V01 0x0066
#define QMI_QCMAP_MSGR_ENABLE_IPV4_REQ_V01 0x0067
#define QMI_QCMAP_MSGR_ENABLE_IPV4_RESP_V01 0x0067
#define QMI_QCMAP_MSGR_DISABLE_IPV4_REQ_V01 0x0068
#define QMI_QCMAP_MSGR_DISABLE_IPV4_RESP_V01 0x0068
#define QMI_QCMAP_MSGR_GET_IPV4_STATE_REQ_V01 0x0069
#define QMI_QCMAP_MSGR_GET_IPV4_STATE_RESP_V01 0x0069
#define QMI_QCMAP_MSGR_GET_DATA_BITRATE_REQ_V01 0x006A
#define QMI_QCMAP_MSGR_GET_DATA_BITRATE_RESP_V01 0x006A
#define QMI_QCMAP_MSGR_GET_UPNP_NOTIFY_INTERVAL_REQ_V01 0x006B
#define QMI_QCMAP_MSGR_GET_UPNP_NOTIFY_INTERVAL_RESP_V01 0x006B
#define QMI_QCMAP_MSGR_SET_UPNP_NOTIFY_INTERVAL_REQ_V01 0x006C
#define QMI_QCMAP_MSGR_SET_UPNP_NOTIFY_INTERVAL_RESP_V01 0x006C
#define QMI_QCMAP_MSGR_GET_DLNA_NOTIFY_INTERVAL_REQ_V01 0x006D
#define QMI_QCMAP_MSGR_GET_DLNA_NOTIFY_INTERVAL_RESP_V01 0x006D
#define QMI_QCMAP_MSGR_SET_DLNA_NOTIFY_INTERVAL_REQ_V01 0x006E
#define QMI_QCMAP_MSGR_SET_DLNA_NOTIFY_INTERVAL_RESP_V01 0x006E
#define QMI_QCMAP_MSGR_ADD_DHCP_RESERVATION_RECORD_REQ_V01 0x006F
#define QMI_QCMAP_MSGR_ADD_DHCP_RESERVATION_RECORD_RESP_V01 0x006F
#define QMI_QCMAP_MSGR_GET_DHCP_RESERVATION_RECORDS_REQ_V01 0x0070
#define QMI_QCMAP_MSGR_GET_DHCP_RESERVATION_RECORDS_RESP_V01 0x0070
#define QMI_QCMAP_MSGR_EDIT_DHCP_RESERVATION_RECORD_REQ_V01 0x0071
#define QMI_QCMAP_MSGR_EDIT_DHCP_RESERVATION_RECORD_RESP_V01 0x0071
#define QMI_QCMAP_MSGR_DELETE_DHCP_RESERVATION_RECORD_REQ_V01 0x0072
#define QMI_QCMAP_MSGR_DELETE_DHCP_RESERVATION_RECORD_RESP_V01 0x0072
#define QMI_QCMAP_MSGR_ACTIVATE_HOSTAPD_CONFIG_REQ_V01 0x0073
#define QMI_QCMAP_MSGR_ACTIVATE_HOSTAPD_CONFIG_RESP_V01 0x0073
#define QMI_QCMAP_MSGR_ACTIVATE_SUPPLICANT_CONFIG_REQ_V01 0x0074
#define QMI_QCMAP_MSGR_ACTIVATE_SUPPLICANT_CONFIG_RESP_V01 0x0074
#define QMI_QCMAP_MSGR_ENABLE_ALG_REQ_V01 0x0075
#define QMI_QCMAP_MSGR_ENABLE_ALG_RESP_V01 0x0075
#define QMI_QCMAP_MSGR_DISABLE_ALG_REQ_V01 0x0076
#define QMI_QCMAP_MSGR_DISABLE_ALG_RESP_V01 0x0076
#define QMI_QCMAP_MSGR_GET_WEBSERVER_WWAN_ACCESS_REQ_V01 0x0077
#define QMI_QCMAP_MSGR_GET_WEBSERVER_WWAN_ACCESS_RESP_V01 0x0077
#define QMI_QCMAP_MSGR_SET_WEBSERVER_WWAN_ACCESS_REQ_V01 0x0078
#define QMI_QCMAP_MSGR_SET_WEBSERVER_WWAN_ACCESS_RESP_V01 0x0078
#define QMI_QCMAP_MSGR_SET_SIP_SERVER_INFO_REQ_V01 0x0079
#define QMI_QCMAP_MSGR_SET_SIP_SERVER_INFO_RESP_V01 0x0079
#define QMI_QCMAP_MSGR_GET_SIP_SERVER_INFO_REQ_V01 0x007A
#define QMI_QCMAP_MSGR_GET_SIP_SERVER_INFO_RESP_V01 0x007A
#define QMI_QCMAP_MSGR_RESTORE_FACTORY_CONFIG_REQ_V01 0x007B
#define QMI_QCMAP_MSGR_RESTORE_FACTORY_CONFIG_RESP_V01 0x007B
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qcmap_msgr_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qcmap_msgr_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qcmap_msgr_get_service_object_v01( ) \
          qcmap_msgr_get_service_object_internal_v01( \
            QCMAP_MSGR_V01_IDL_MAJOR_VERS, QCMAP_MSGR_V01_IDL_MINOR_VERS, \
            QCMAP_MSGR_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

