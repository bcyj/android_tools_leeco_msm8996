#ifndef _QCMAP_WEB_CLIENT_H
#define _QCMAP_WEB_CLIENT_H
/*===========================================================================

                         QCMAP_WEB_CLIENT. H

DESCRIPTION

  The Qualcomm Mobile Access Point web client file.

EXTERNALIZED FUNCTIONS

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/*===========================================================================

when        who    what, where, why
--------    ---    ----------------------------------------------------------
03/17/14    rk     token generate command change.
01/08/14    rk     Webserver paswd encryption.
11/22/13    rk     Webserver security flaws fixed.
21/02/13    rk     Created module
===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <math.h>
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "common_v01.h"
#include "comdef.h"

using namespace std;
/* mask the relevant request received from CGI for execution */
typedef enum
{
  FIELD_MASK_1 = 0x01,
  FIELD_MASK_2 = 0x02,
  FIELD_MASK_3 = 0x04,
  FIELD_MASK_4 = 0x08,
  FIELD_MASK_5 = 0x10,
  FIELD_MASK_6 = 0x20,
  FIELD_MASK_7 = 0x40,
  FIELD_MASK_8 = 0x80,
  FIELD_MASK_9 = 0x100,
  FIELD_MASK_10 = 0x200,
  FIELD_MASK_11 = 0x400,
  FIELD_MASK_12 = 0x800,
  FIELD_MASK_13 = 0x1000,
  FIELD_MASK_14 = 0x2000,
  FIELD_MASK_15 = 0x4000,
  FIELD_MASK_16 = 0x8000,
  FIELD_MASK_17 = 0x10000,
  FIELD_MASK_18 = 0x20000,
  FIELD_MASK_19 = 0x40000,
  FIELD_MASK_20 = 0x80000,
  FIELD_MASK_MAX
}field_mask;

typedef enum
{
  MOBILE_AP_PAGE_PAGE = 0,
  MOBILE_AP_PAGE_MASK,
  MOBILE_AP_PAGE_AUTO_CONNECT,
  MOBILE_AP_PAGE_AUTO_CONNECT_RESULT,
  MOBILE_AP_PAGE_ROAMING ,
  MOBILE_AP_PAGE_ROAMING_RESULT ,
  MOBILE_AP_PAGE_MAX //never used
}mobile_ap_field;
typedef enum
{
  GET_MEDIA_DIR_PAGE_PAGE = 0,
  GET_MEDIA_DIR_PAGE_RESULT,
  GET_MEDIA_DIR_PAGE_DIR_LIST,
  GET_MEDIA_DIR_PAGE_MAX //never used
}get_media_dir_field;
typedef enum
{
  SET_MEDIA_DIR_PAGE_PAGE = 0,
  SET_MEDIA_DIR_PAGE_ADD_MODIFY,
  SET_MEDIA_DIR_PAGE_DIR_LIST,
  SET_MEDIA_DIR_PAGE_RESULT,
  SET_MEDIA_DIR_PAGE_MAX //never used
}set_media_dir_field;

typedef enum
{
  UPNP_DLNA_PAGE_PAGE = 0,
  UPNP_DLNA_PAGE_MASK,
  UPNP_DLNA_PAGE_UPNP_RESULT,
  UPNP_DLNA_PAGE_UPNP_ENABLE,
  UPNP_DLNA_PAGE_DLNA_RESULT ,
  UPNP_DLNA_PAGE_DLNA_ENABLE ,
  UPNP_DLNA_PAGE_MDNS_RESULT ,
  UPNP_DLNA_PAGE_MDNS_ENABLE ,
  UPNP_DLNA_PAGE_MAX //never used
}upnp_dlna_field;


typedef enum
{
  UPNP_DLNA_NOTIFY_PAGE_PAGE = 0,
  UPNP_DLNA_NOTIFY_PAGE_MASK,
  UPNP_DLNA_NOTIFY_PAGE_UPNP_RESULT,
  UPNP_DLNA_NOTIFY_PAGE_UPNP,
  UPNP_DLNA_NOTIFY_PAGE_DLNA_RESULT ,
  UPNP_DLNA_NOTIFY_PAGE_DLNA,
  UPNP_DLNA_NOTIFY_PAGE_MAX //never used
}upnp_dlna_notify_field;

typedef enum
{
  WWAN_IPV4_PAGE_PAGE = 0,
  WWAN_IPV4_PAGE_MASK,
  WWAN_IPV4_PAGE_IPV4_ENABLE,
  WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT,
  WWAN_IPV4_PAGE_BACKHAUL,
  WWAN_IPV4_PAGE_BACKHAUL_RESULT,
  WWAN_IPV4_PAGE_TECH_RESULT,
  WWAN_IPV4_PAGE_NETP_TECHNOLOGY,
  WWAN_IPV4_PAGE_NETP_TECH_IPV4,
  WWAN_IPV4_PAGE_NETP_TECH2_IPV4,
  WWAN_IPV4_PAGE_MAX //never used
}wwan_ipv4_field;

typedef enum
{
  LAN_PAGE_PAGE = 0,
  LAN_PAGE_MASK,
  LAN_PAGE_LAN_AP_GW_IP ,
  LAN_PAGE_LAN_AP_SUBNET_MASK ,
  LAN_PAGE_LAN_AP_DHCP_MODE ,
  LAN_PAGE_LAN_AP_DHCP_START_IP ,
  LAN_PAGE_LAN_AP_DHCP_END_IP ,
  LAN_PAGE_LAN_AP_DHCP_LEASE_TIME ,
  LAN_PAGE_WLAN_ENABLE_DISABLE ,
  LAN_PAGE_WLAN_MODE ,
  LAN_PAGE_WLAN_PROFILE,
  LAN_PAGE_WLAN_AP_STA_MODE ,
  LAN_PAGE_WLAN_AP_STA_STATIC_IP ,
  LAN_PAGE_WLAN_AP_STA_GW_IP ,
  LAN_PAGE_WLAN_AP_STA_STATIC_DNS_IP ,
  LAN_PAGE_WLAN_AP_STA_STATIC_SUBNET_MASK ,
  LAN_PAGE_MAX //never used
}lan_page_field;

typedef enum
{
  DHCP_RESERVATION_SETTINGS_PAGE_PAGE=0,
  DHCP_RESERVATION_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT,
  DHCP_RESERVATION_SETTINGS_PAGE_IP_ADDR,
  DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR,
  DHCP_RESERVATION_SETTINGS_PAGE_RESERVED_IP,
  DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME,
  DHCP_RESERVATION_SETTINGS_PAGE_ENABLE_RESERVATION,
  DHCP_RESERVATION_SETTINGS_PAGE_MAX //never used
}dhcp_reservation_field;

typedef enum
{
  NAT_SETTINGS_PAGE_PAGE=0,
  NAT_SETTINGS_PAGE_MASK,
  NAT_SETTINGS_PAGE_NAT_TYPE,
  NAT_SETTINGS_PAGE_NAT_TYPE_RESULT,
  NAT_SETTINGS_PAGE_DMZ_IP,
  NAT_SETTINGS_PAGE_DMZ_IP_RESULT,
  NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH,
  NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH_RESULT,
  NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH,
  NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH_RESULT,
  NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH,
  NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH_RESULT,
  NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT,
  NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT_RESULT,
  NAT_SETTINGS_PAGE_NAT_ICMP_TIMEOUT,
  NAT_SETTINGS_PAGE_NAT_ICMP_TIMEOUT_RESULT,
  NAT_SETTINGS_PAGE_NAT_TCP_TIMEOUT,
  NAT_SETTINGS_PAGE_NAT_TCP_TIMEOUT_RESULT,
  NAT_SETTINGS_PAGE_NAT_UDP_TIMEOUT,
  NAT_SETTINGS_PAGE_NAT_UDP_TIMEOUT_RESULT,
  NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS,
  NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS_RESULT,
  NAT_SETTINGS_PAGE_MAX //never used
}nat_settings_page_field;

typedef enum
{
  STATIC_NAT_SETTINGS_PAGE_PAGE=0,
  STATIC_NAT_SETTINGS_PAGE_MASK,
  STATIC_NAT_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT,
  STATIC_NAT_SETTINGS_PAGE_PRIVATE_IP_ADDR,
  STATIC_NAT_SETTINGS_PAGE_PRIVATE_PORT,
  STATIC_NAT_SETTINGS_PAGE_GLOBAL_PORT,
  STATIC_NAT_SETTINGS_PAGE_PROTOCOL,
  STATIC_NAT_SETTINGS_PAGE_OLD_PRIVATE_IP_ADDR,
  STATIC_NAT_SETTINGS_PAGE_OLD_PRIVATE_PORT,
  STATIC_NAT_SETTINGS_PAGE_OLD_GLOBAL_PORT,
  STATIC_NAT_SETTINGS_PAGE_OLD_PROTOCOL,
  STATIC_NAT_SETTINGS_PAGE_MAX //never used
}static_nat_page_field;
typedef enum
{
  WWAN_STATS_PAGE_PAGE = 0,
  WWAN_STATS_PAGE_GET_RESET_MODE,
  WWAN_STATS_PAGE_IP_FAMILY,
  WWAN_STATS_PAGE_RESULT,
  WWAN_STATS_PAGE_BYTES_RX,
  WWAN_STATS_PAGE_BYTES_TX,
  WWAN_STATS_PAGE_PACKETS_RX,
  WWAN_STATS_PAGE_PACKETS_TX,
  WWAN_STATS_PAGE_PACKET_DROPPED_RX,
  WWAN_STATS_PAGE_PACKET_DROPPED_TX,
  WWAN_STATS_PAGE_CONFIG_ERROR,
  WWAN_STATS_PAGE_PUBLIC_IP,
  WWAN_STATS_PAGE_DNS_PRI,
  WWAN_STATS_PAGE_DNS_SEC,
  WWAN_STATS_PAGE_STATUS_ERROR,
  WWAN_STATS_PAGE_STATUS,
  WWAN_STATS_PAGE_STA_RESULT,
  WWAN_STATS_PAGE_STA_STATUS,
  WWAN_STATS_PAGE_MAX //never used
}wwan_stats_page;
typedef enum
{
  DATA_RATE_PAGE_PAGE = 0,
  DATA_RATE_PAGE_RESULT,
  DATA_RATE_PAGE_BYTES_TX,
  DATA_RATE_PAGE_BYTES_RX,
  DATA_RATE_PAGE_BYTES_TX_MAX,
  DATA_RATE_PAGE_BYTES_RX_MAX,
  DATA_RATE_PAGE_MAX //never used
}data_rate_page;

typedef enum
{
  FIREWALL_SETTINGS_PAGE_PAGE = 0,
  FIREWALL_SETTINGS_PAGE_MASK,
  FIREWALL_SETTINGS_PAGE_FIREWALL_STATE,
  FIREWALL_SETTINGS_PAGE_PACKETS_ALLOWED,
  FIREWALL_SETTINGS_PAGE_FIREWALL_HANDLE,
  FIREWALL_SETTINGS_PAGE_IP_FAMILY,
  FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO_VALID,
  FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO,
  FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID,
  FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT,
  FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE,
  FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID,
  FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT,
  FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE,
  FIREWALL_SETTINGS_PAGE_ICMP_TYPE_VALID,
  FIREWALL_SETTINGS_PAGE_ICMP_TYPE,
  FIREWALL_SETTINGS_PAGE_ICMP_CODE_VALID,
  FIREWALL_SETTINGS_PAGE_ICMP_CODE,
  FIREWALL_SETTINGS_PAGE_ESPSPI_VALID,
  FIREWALL_SETTINGS_PAGE_ESPSPI,
  FIREWALL_SETTINGS_PAGE_IPV4_SRC_VALID,
  FIREWALL_SETTINGS_PAGE_IPV4_SRC_ADDR,
  FIREWALL_SETTINGS_PAGE_IPV4_SRC_SUBNET,
  FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALID,
  FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALUE,
  FIREWALL_SETTINGS_PAGE_IPV4_TOS_MASK,
  FIREWALL_SETTINGS_PAGE_IPV6_SRC_VALID,
  FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR,
  FIREWALL_SETTINGS_PAGE_IPV6_SRC_PREFIX_LENGTH,
  FIREWALL_SETTINGS_PAGE_IPV6_CLASS_VALID,
  FIREWALL_SETTINGS_PAGE_IPV6_CLASS,
  FIREWALL_SETTINGS_PAGE_IPV6_MASK,
  FIREWALL_SETTINGS_PAGE_MAX //never used
}firewall_settings_page_field;

typedef enum
{
  FIREWALL_SETTINGS_GET_PAGE_PAGE = 0,
  FIREWALL_SETTINGS_GET_PAGE_IP_FAMILY
}firewall_settings_get_page_field;


typedef enum
{
  IPV6_WWAN_SETTINGS_PAGE_PAGE = 0,
  IPV6_WWAN_SETTINGS_PAGE_MASK,
  IPV6_WWAN_SETTINGS_PAGE_ENABLE,
  IPV6_WWAN_SETTINGS_PAGE_ENABLE_RESULT,
  IPV6_WWAN_SETTINGS_PAGE_BACKHAUL,
  IPV6_WWAN_SETTINGS_PAGE_BACKHAUL_RESULT,
  IPV6_WWAN_SETTINGS_PAGE_POLICY_RESULT,
  IPV6_WWAN_SETTINGS_PAGE_TECH_PREF,
  IPV6_WWAN_SETTINGS_PAGE_PROFILE_ID_GPP,
  IPV6_WWAN_SETTINGS_PAGE_PROFILE_ID_GPP2,
  IPV6_WWAN_SETTINGS_PAGE_MAX
}ipv6_wwan_settings_page_field;

typedef enum
{
IPV6_GET_STATE_RESULT=1,
IPV6_GET_STATE,
IPV6_GET_WWAN_STATUS_ERROR,
IPV6_GET_WWAN_STATUS,
IPV6_GET_WWAN_POLICY_ERROR,
IPV6_GET_WWAN_POLICY_TECH,
IPV6_GET_WWAN_POLICY_3GPP,
IPV6_GET_WWAN_POLICY_3GPP2,
IPV6_GET_WWAN_POLICY_MAX
}
ipv6_get_state;


typedef enum
{
  DISABLED = 0,
  ENABLED,
  DISABLE_PnP
}enable_disable;

typedef enum
{
  ADD = 1,
  DELETE,
  MODIFY,
  ENABLE,
  NO_CHANGE
}operations;

/*===========================================================================

                            CONSTANT DEFINITIONS

===========================================================================*/
#define QCMAP_WEBCLIENT_CGI_FILE "/www/qcmap_webclient_cgi_file"
#define QCMAP_CM_SUCCESS               0         /* Successful operation   */
#define QCMAP_CM_ERROR                -1         /* Unsuccessful operation */
#define MAX_ELEMENT_LENGTH 60       //Max Size of any element value
#define MAX_ELEMENT_COUNT  40       //Max Elements can be processed
#define ENABLE 4
#define SETMOBILEAP "SetMobileAP"
#define GETMOBILEAP "GetMobileAP"
#define ADDSNATENTRY "AddSnatEntry"
#define GETSNATENTRIES "GetSnatEntries"
#define GETMEDIA_DIR_LIST "GetMediaDir"
#define SETMEDIA_DIR_LIST "SetMediaDir"
#define SETNATSETTINGS "SetNatSettings"
#define GETNATSETTINGS "GetNatSettings"
#define SETFIREWALLENTRIES "SetFirewall"
#define GETFIREWALLENTRIES "GetFirewall"
#define GETFIREWALLCONFIG "GetConfigFirewall"
#define FIREWALLPAGE "FirewallPage"
#define IPV6SETWWAN "SetWWANv6"
#define IPV6GETWWAN "GetWWANv6"
#define GETIPV6PAGE "Getipv6page"
#define WWAN_STAT_MAX 6
#define DEBUG_PRINT 1
#define SETLAN_CONFIG "SetLanConfig"
#define GETLAN_CONFIG "GetLanConfig"
#define SETWWANIPV4 "SetWWANIPV4"
#define GETWWANIPV4 "GetWWANIPV4"
#define GETUPNPDLNA "GetMediaSharingStatus"
#define SETUPNPDLNA "SetMediaSharingStatus"
#define GETDATARATE "GetDATASTATS"
#define SETNOTIFYINTERVAL "SetNotifyInterval"
#define GETNOTIFYINTERVAL "GetNotifyInterval"
#define GETDHCPRESERVATIONRECORDS "GetDHCPReservations"
#define ADDDHCPRESERVATIONRECORD "AddDHCPReservation"

#define WWAN_STATS "GetWWANSTATS"
#define ENABLEAP "set_mobileapstatus"
#define GET_AP_STATUS "get_mobileapstatus"
#define SUCCESS "SUCCESS"
#define SNAT_ERROR "\"snat_error\":\"%s\","
#define SNAT_COUNT "\"snat_count\":\"%d\","
#define COUNT "\"count\":\"%d\","
#define ENTRIES "\"entries\":["
#define PRIVATE_IP "\"private_ip\":\"%s\","
#define PRIVATE_PORT "\"private_port\":\"%d\","
#define GLOBAL_PORT "\"global_port\":\"%d\","
#define PROTO "\"proto\":\"%d\""
#define DHCP_RESERVATION_COUNT "\"dhcp_count\":\"%d\","
#define MAC_ADDR "\"mac\":\"%s\","
#define RESERVED_IP "\"reserved_ip\":\"%s\","
#define DEV_NAME "\"dev_name\":\"%s\","
#define RESERVATION_ENABLE "\"reservation_enable\":\"%d\""
#define PAGE "\"Page\":\"%s\","
#define RESULT "\"Result\":\"%s\""
#define DHCP_RESULT "\"result\":\"%s\""
#define RESULT_NO_COMMA "\"result\":\"%s\""
#define GET_LAN_STATUS "\"get_lan_status\":\"%s\","
#define LAN_STATUS "\"lan_status\":\"%d\","
#define GET_LAN_CONFIG "\"get_lan_config\":\"%s\""
#define SET_LAN_CONFIG "\"lan_config_result\":\"%s\","
#define SET_WLAN_STATUS "\"wlan_status_result\":\"%s\","
#define SET_WLAN_CONFIG "\"wlan_config_result\":\"%s\","
#define SET_LAN_ACTIVATE "\"lan_activate_result\":\"%s\","
#define SET_WLAN_ACTIVATE "\"wlan_activate_result\":\"%s\""
#define FIREWALL_ENABLE "\"Firewall_enable\":\"%d\","
#define PKTS_ALLOWED "\"pkts_allowed\":\"%d\","
#define PKTS_ALLOWED_CONFIG "\"pkts_allowed\":\"%d\""
#define COUNT "\"count\":\"%d\","
#define RES_SUCCESS ",\"result\":\"SUCCESS\" "
#define CONNECTED "connected"
#define DISCONNECTED "disconnected"
#define JSN_FMT_CMA "\"%s\":\"%s\","
#define JSN_FMT_NCMA "\"%s\":\"%s\""
#define MAX_BUF_LEN 40960
#define MAX(a,b) (a > b ? a : b)
#define IPV6 6
#define IPV4 4
#define IPV4V6 10
#define ZEROS_BUFF_SIZE 3
#define PADDING 2
#define MAC_HEX_STRING "0123456789abcdefABCDEF" /*MAC hex check*/
#define MAC_NULL_STRING "00:00:00:00:00:00" /*MAC Null String*/
#define Token_size 16 /* to store 128 bit token */
//session token file.
#define SESSION_TOKEN "/www/session_token.txt"
//restrict file access to user,group only.
#define CHG_SESSION_TOKEN_FILE "chmod 770 /www/session_token.txt"
//change file owner,group name to www-data, as per security avoid root.
#define CHG_SESSION_TOKEN_FILE_OWNR "chown www-data:www-data /www/session_token.txt"
//random number generate and store in session token file.
#define TOKEN_GENERATE "od -A n -t x -N 16 /dev/urandom | tr -dc A-Z-a-z-0-9 | head -c16 \
                        > /www/session_token.txt "
#define TOKEN_MISMACTH "Token_mismatch"
//if mobileap enable fails, send failure message to cgi client.
#define MOBILEAP_BRINGUP_FAIL "Mobile AP bringup failed"
#ifdef __cplusplus
  extern "C" {
#endif
extern void create_error
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  qmi_error_type_v01  qmi_error
);
extern void create_firewall_buf
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf
);
extern void create_response_buf
(
  char res_result[][MAX_ELEMENT_LENGTH],
  char* buf,
  int max_res_cnt,
  char** page_string,
  int start_index,
  int mask_braces
);
extern void store_lanconfig_res
(
  char values[][MAX_ELEMENT_LENGTH],
  int wlan_status,
  int wlan_mode,
  int gap_profile,
  int *i,
  qcmap_msgr_lan_config_v01 *lan_config,
  qcmap_msgr_station_mode_config_v01 *station_config
);

extern  void Tokenizer
(
  char * buf,
  char fields[][MAX_ELEMENT_LENGTH],
  char values[][MAX_ELEMENT_LENGTH],
  int *token_pos
);

extern int system_ip_to_readable_ip
(
  int domain,
  uint32 *addr,
  char *str
);

extern uint32_t readable_ip_to_system_ip
(
  char* string
);
extern int unencode_string
(
  char *src,
  int last
);
extern void create_msg_to_send
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  char* result_string
);
#ifdef __cplusplus
}
#endif

#endif
